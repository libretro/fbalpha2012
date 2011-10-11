#include <cell/audio.h>
#include <cell/sysmodule.h>
#include <pthread.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "stream.h"

#include "buffer.h"

#define CELL_AUDIO_BLOCK_SAMPLES_X2 512
#define CELL_AUDIO_BLOCK_SAMPLES_X4 1024

#define SAMPLES_TO_FRAMES(x,y) ((x)/(y)->channels)
#define FRAMES_TO_SAMPLES(x,y) ((x)*(y)->channels)

#define resampler_s16_to_float(out, in, samples) \
	for (uint32_t i = 0; i < samples; i++) \
		out[i] = (float)in[i]/0x8000;

#define poly_create_3(poly, y) \
   poly[2] = (y[0] - 2 * y[1] + y[2])/2; \
   poly[1] = -1.5 * y[0] + 2 * y[1] - 0.5 * y[2]; \
   poly[0] = y[0];

typedef uint32_t (*resampler_cb_t) (void *cb_data, float **data);
typedef struct resampler resampler_t;

typedef struct audioport
{
	resampler_t *re;
	pthread_mutex_t lock;
	pthread_mutex_t cond_lock;
	pthread_cond_t cond;
	pthread_t thread;
	fifo_buffer_t *buffer;
	cell_audio_sample_cb_t sample_cb;
	float *re_buffer;
	volatile uint64_t quit_thread;
	uint32_t audio_port;
	uint32_t channels;
	uint32_t is_paused;
	int16_t *re_pull_buffer;
	void *userdata;
} audioport_t;

struct resampler
{
	resampler_cb_t func;
	void *cb_data;
	double ratio;
	float *data;
	uint64_t sum_output_frames;
	uint64_t sum_input_frames;
	uint32_t data_ptr;
	uint32_t data_size;
	int channels;
};

static uint32_t resampler_cb(void *userdata, float **data)
{
	audioport_t *port = userdata;
	uint32_t has_read = 0;

	if (port->sample_cb)
		has_read = port->sample_cb(port->re_pull_buffer, CELL_AUDIO_BLOCK_SAMPLES_X2 * sizeof(int16_t), port->userdata);
	else
	{
		has_read = CELL_AUDIO_BLOCK_SAMPLES_X2;
		pthread_mutex_lock(&port->lock);
		uint32_t avail = fifo_read_avail(port->buffer);
		if (avail < CELL_AUDIO_BLOCK_SAMPLES_X2 * sizeof(int16_t))
			has_read = avail / sizeof(int16_t);

		fifo_read(port->buffer, port->re_pull_buffer, has_read * sizeof(int16_t));
		pthread_mutex_unlock(&port->lock);
		pthread_cond_signal(&port->cond);
	}

	if (has_read < CELL_AUDIO_BLOCK_SAMPLES_X4)
		memset(port->re_pull_buffer + has_read, 0, (CELL_AUDIO_BLOCK_SAMPLES_X2 - has_read) * sizeof(int16_t));

	resampler_s16_to_float(port->re_buffer, port->re_pull_buffer, CELL_AUDIO_BLOCK_SAMPLES_X2);

	*data = port->re_buffer;
	return CELL_AUDIO_BLOCK_SAMPLES;
}

static uint32_t resampler_process(resampler_t *state, uint32_t frames, float *out_data)
{
	uint32_t frames_used = 0;
	uint64_t pos_out;
	double pos_in = 0.0;

	for (uint64_t x = state->sum_output_frames; x < state->sum_output_frames + frames; x++)
	{
		pos_out = x - state->sum_output_frames;
		pos_in  = ((double)x / state->ratio) - (double)state->sum_input_frames;
		for (int c = 0; c < state->channels; c++)
		{
			float poly[3];
			float data[3];
			float x_val;

			if ((int)pos_in == 0)
			{
				data[0] = state->data[0 * state->channels + c];
				data[1] = state->data[1 * state->channels + c];
				data[2] = state->data[2 * state->channels + c];
				x_val = pos_in;
			}
			else
			{
				data[0] = state->data[((int)pos_in - 1) * state->channels + c];
				data[1] = state->data[((int)pos_in + 0) * state->channels + c];
				data[2] = state->data[((int)pos_in + 1) * state->channels + c];
				x_val = pos_in - (int)pos_in + 1.0;
			}

			poly_create_3(poly, data);
			poly[2] = (data[0] - 2*data[1] + data[2])/2;
			poly[1] = -1.5*  data[0] + 2 * data[1] - 0.5* data[2];
			poly[0] = data[0];

			out_data[pos_out * state->channels + c] = poly[2] * x_val * x_val + poly[1] * x_val + poly[0];
		}
	}
	frames_used = (int)pos_in;
	return frames_used;
}

static uint32_t resampler_cb_read(resampler_t *state, uint32_t frames, float *data)
{
	uint32_t after_sum = state->sum_output_frames + frames;

	uint32_t min_input_frames = (uint32_t)((after_sum / state->ratio) + 2.0);
	// How many frames must we have to resample?
	uint32_t req_frames = min_input_frames - state->sum_input_frames;

	// Do we need to read more data?
	if (SAMPLES_TO_FRAMES(state->data_ptr, state) < req_frames)
	{
		uint32_t must_read = req_frames - SAMPLES_TO_FRAMES(state->data_ptr, state);
		float temp_buf[FRAMES_TO_SAMPLES(must_read, state)];

		uint32_t has_read = 0;

		uint32_t copy_size = 0;
		uint32_t ret = 0;
		float *ptr = NULL;
		while (has_read < must_read)
		{
			ret = state->func(state->cb_data, &ptr);

			if (ret == 0 || ptr == NULL) // We're done.
				return -1;

			copy_size = (ret > must_read - has_read) ? (must_read - has_read) : ret;
			memcpy(temp_buf + FRAMES_TO_SAMPLES(has_read, state), ptr, FRAMES_TO_SAMPLES(copy_size, state) * sizeof(float));

			has_read += ret;
		}

		// We might have gotten a lot of data from the callback. We should realloc our buffer if needed.
		uint32_t req_buffer_frames = SAMPLES_TO_FRAMES(state->data_ptr, state) + has_read;

		if (req_buffer_frames > SAMPLES_TO_FRAMES(state->data_size, state))
		{
			state->data = realloc(state->data, FRAMES_TO_SAMPLES(req_buffer_frames, state) * sizeof(float));
			if (state->data == NULL)
				return -1;

			state->data_size = FRAMES_TO_SAMPLES(req_buffer_frames, state);
		}

		memcpy(state->data + state->data_ptr, temp_buf, FRAMES_TO_SAMPLES(must_read, state) * sizeof(float));
		state->data_ptr += FRAMES_TO_SAMPLES(must_read, state);

		// We have some data from the callback we need to copy over as well.
		if (ret > copy_size)
		{
			memcpy(state->data + state->data_ptr, ptr + FRAMES_TO_SAMPLES(copy_size, state), FRAMES_TO_SAMPLES(ret - copy_size, state) * sizeof(float));
			state->data_ptr += FRAMES_TO_SAMPLES(ret - copy_size, state);
		}
	}

	// Phew. We should have enough data in our buffer now to be able to process the data we need.

	uint32_t frames_used = resampler_process(state, frames, data);
	state->sum_input_frames += frames_used;
	memmove(state->data, state->data + FRAMES_TO_SAMPLES(frames_used, state), (state->data_ptr - FRAMES_TO_SAMPLES(frames_used, state)) * sizeof(float));
	state->data_ptr -= FRAMES_TO_SAMPLES(frames_used, state);
	state->sum_output_frames += frames;

	return frames;
}

static void* event_loop(void *data)
{
	audioport_t *port = data;

	sys_event_queue_t id;
	sys_ipc_key_t key;

	cellAudioCreateNotifyEventQueue(&id, &key);
	cellAudioSetNotifyEventQueue(key);

	if (port->re)
	{
		sys_event_t event;
		port->re_buffer = memalign(128, CELL_AUDIO_BLOCK_SAMPLES_X2 * sizeof(float));
		port->re_pull_buffer = memalign(128, CELL_AUDIO_BLOCK_SAMPLES_X2 * sizeof(int16_t));

		float *res_buffer = memalign(128, CELL_AUDIO_BLOCK_SAMPLES_X2 * sizeof(float));

		while (!port->quit_thread)
		{
			resampler_cb_read(port->re, CELL_AUDIO_BLOCK_SAMPLES, res_buffer);
			sys_event_queue_receive(id, &event, SYS_NO_TIMEOUT);
			cellAudioAddData(port->audio_port, res_buffer, CELL_AUDIO_BLOCK_SAMPLES, 1.0);
		}
		free(res_buffer);
		free(port->re_buffer);
		free(port->re_pull_buffer);
		port->re_buffer = NULL;
		port->re_pull_buffer = NULL;
	}
	else
	{
		sys_event_t event;

		int16_t *in_buf = memalign(128, CELL_AUDIO_BLOCK_SAMPLES_X2 * sizeof(int16_t));
		float *conv_buf = memalign(128, CELL_AUDIO_BLOCK_SAMPLES_X2 * sizeof(float));
		while (!port->quit_thread)
		{
			uint32_t has_read = 0;
			if (port->sample_cb)
				has_read = port->sample_cb(in_buf, CELL_AUDIO_BLOCK_SAMPLES_X2, port->userdata);
			else
			{
				has_read = CELL_AUDIO_BLOCK_SAMPLES_X2;
				pthread_mutex_lock(&port->lock);
				uint32_t avail = fifo_read_avail(port->buffer);
				if (avail < CELL_AUDIO_BLOCK_SAMPLES_X2 * sizeof(int16_t))
					has_read = avail / sizeof(int16_t);

				fifo_read(port->buffer, in_buf, has_read * sizeof(int16_t));
				pthread_mutex_unlock(&port->lock);
			}

			if (has_read < CELL_AUDIO_BLOCK_SAMPLES * port->channels)
				memset(in_buf + has_read, 0, (CELL_AUDIO_BLOCK_SAMPLES * port->channels - has_read) * sizeof(int16_t));

			resampler_s16_to_float(conv_buf, in_buf, CELL_AUDIO_BLOCK_SAMPLES * port->channels);
			sys_event_queue_receive(id, &event, SYS_NO_TIMEOUT);
			cellAudioAddData(port->audio_port, conv_buf, CELL_AUDIO_BLOCK_SAMPLES, 1.0);

			pthread_cond_signal(&port->cond);
		}
		free(conv_buf);
	}

	cellAudioRemoveNotifyEventQueue(key);
	pthread_exit(NULL);
	return NULL;
}

static resampler_t* resampler_new(resampler_cb_t func, double ratio, int channels, void* cb_data)
{
	if (func == NULL)
		return NULL;

	if (channels < 1)
		return NULL;

	resampler_t* state = calloc(1, sizeof(resampler_t));
	if (state == NULL)
		return NULL;

	state->func = func;
	state->ratio = ratio;
	state->channels = channels;
	state->cb_data = cb_data;

	return state;
}

static cell_audio_handle_t audioport_init(const struct cell_audio_params *params)
{
	static int init_count = 0;
	if (init_count == 0)
	{
		cellSysmoduleLoadModule(CELL_SYSMODULE_AUDIO);
		cellAudioInit();
		init_count++;
	}

	audioport_t *handle = calloc(1, sizeof(*handle));

	CellAudioPortParam port_params = {
		.nChannel = params->channels,
		.nBlock = 8,
		.attr = 0
	};

	handle->channels = params->channels;

	handle->sample_cb = params->sample_cb;
	handle->userdata = params->userdata;
	handle->buffer = fifo_new(params->buffer_size ? params->buffer_size : 4096);

	if (params->samplerate != 48000)
	{
		handle->re = resampler_new(resampler_cb, 48000.0 / params->samplerate, params->channels, handle);
	}

	pthread_mutex_init(&handle->lock, NULL);
	pthread_mutex_init(&handle->cond_lock, NULL);
	pthread_cond_init(&handle->cond, NULL);

	cellAudioPortOpen(&port_params, &handle->audio_port);
	cellAudioPortStart(handle->audio_port);

	pthread_create(&handle->thread, NULL, event_loop, handle);
	return handle;
}

static void audioport_pause(cell_audio_handle_t handle)
{
	audioport_t *port = handle;
	port->is_paused = 1;
	cellAudioPortStop(port->audio_port);
}

static int32_t audioport_unpause(cell_audio_handle_t handle)
{
	audioport_t *port = handle;
	port->is_paused = 0;
	cellAudioPortStart(port->audio_port);
	return 0;
}

static uint32_t audioport_is_paused(cell_audio_handle_t handle)
{
	audioport_t *port = handle;
	return port->is_paused;
}

static void resampler_free(resampler_t* state)
{
	if (state && state->data)
		free(state->data);
	if (state)
		free(state);
}

static void audioport_free(cell_audio_handle_t handle)
{
	audioport_t *port = handle;

	port->quit_thread = 1;
	pthread_join(port->thread, NULL);

	pthread_mutex_destroy(&port->lock);
	pthread_mutex_destroy(&port->cond_lock);
	pthread_cond_destroy(&port->cond);

	if (port->re)
		resampler_free(port->re);
	if (port->buffer)
		fifo_free(port->buffer);

	cellAudioPortStop(port->audio_port);
	cellAudioPortClose(port->audio_port);

	free(port);
}

static uint32_t audioport_write_avail(cell_audio_handle_t handle)
{
	audioport_t *port = handle;

	pthread_mutex_lock(&port->lock);
	uint32_t ret = fifo_write_avail(port->buffer);
	pthread_mutex_unlock(&port->lock);
	return ret / sizeof(int16_t);
}

static int32_t audioport_write(cell_audio_handle_t handle, const int16_t *data, uint32_t samples)
{
	int32_t ret = samples;
	uint32_t bytes = samples * sizeof(int16_t);

	audioport_t *port = handle;
	do
	{
		pthread_mutex_lock(&port->lock);
		uint32_t avail = fifo_write_avail(port->buffer);
		pthread_mutex_unlock(&port->lock);

		uint32_t to_write = avail < bytes ? avail : bytes;
		if (to_write > 0)
		{
			pthread_mutex_lock(&port->lock);
			fifo_write(port->buffer, data, to_write);
			pthread_mutex_unlock(&port->lock);
			bytes -= to_write;
			data += to_write >> 1;
		}
		else
		{
			pthread_mutex_lock(&port->cond_lock);
			pthread_cond_wait(&port->cond, &port->cond_lock);
			pthread_mutex_unlock(&port->cond_lock);
		}
	}while (bytes);

	return ret;
}

static uint32_t audioport_alive(cell_audio_handle_t handle)
{
	(void)handle;
	return 1;
}



const cell_audio_driver_t cell_audio_audioport = {
	.init = audioport_init,
	.write = audioport_write,
	.write_avail = audioport_write_avail,
	.pause = audioport_pause,
	.unpause = audioport_unpause,
	.is_paused = audioport_is_paused,
	.alive = audioport_alive,
	.free = audioport_free
};
