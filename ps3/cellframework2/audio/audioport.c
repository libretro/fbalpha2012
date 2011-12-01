#include <stdint.h>
#include "stream.h"
#include <cell/audio.h>
#include <cell/sysmodule.h>
#include <sys/synchronization.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>

#define resampler_s16_to_float(out, in, samples) \
   for (uint32_t i = 0; i < samples; i++) \
      out[i] = (float)in[i]/0x8000;

typedef struct fifo_buffer fifo_buffer_t;

struct fifo_buffer
{
   char *buffer;
   uint32_t bufsize;
   uint32_t first;
   uint32_t end;
};

typedef struct resampler resampler_t;

struct resampler
{
   float *data;
   double ratio;
   uint32_t data_ptr;
   uint32_t data_size;
   void *cb_data;
   int channels;
   uint64_t sum_output_frames;
   uint64_t sum_input_frames;
};

static void init_audioport(void)
{
	static int init_count = 0;
	if (init_count == 0)
	{
		cellSysmoduleLoadModule(CELL_SYSMODULE_AUDIO);
		cellAudioInit();
		init_count++;
	}
}

typedef struct audioport
{
	volatile uint64_t quit_thread;
	uint32_t audio_port;

	uint32_t channels;

	sys_lwmutex_t lock;
	sys_lwmutex_t cond_lock;
	sys_lwcond_t cond;
	pthread_t thread;

	resampler_t *re;
	float *re_buffer;
	int16_t *re_pull_buffer;
	fifo_buffer_t *buffer;

	void *userdata;

	uint32_t is_paused;
} audioport_t;

static fifo_buffer_t* fifo_new(uint32_t size)
{
	fifo_buffer_t *buf = calloc(1, sizeof(*buf));
	if (buf == NULL)
		return NULL;

	buf->buffer = memalign(128, size + 1);
	if (buf->buffer == NULL)
	{
		free(buf);
		return NULL;
	}
	memset(buf->buffer, 0, size + 1);
	buf->bufsize = size + 1;

	return buf;
}

static void fifo_free(fifo_buffer_t* buffer)
{
   free(buffer->buffer);
   free(buffer);
}

static uint32_t fifo_read_avail(fifo_buffer_t* buffer)
{
	uint32_t first = buffer->first;
	uint32_t end = buffer->end;
	if (end < first)
		end += buffer->bufsize;
	return end - first;
}

static uint32_t fifo_write_avail(fifo_buffer_t* buffer)
{
	uint32_t first = buffer->first;
	uint32_t end = buffer->end;
	if (end < first)
		end += buffer->bufsize;

	return (buffer->bufsize - 1) - (end - first);
}

static void fifo_write(fifo_buffer_t* buffer, const void* in_buf, uint32_t size)
{
	uint32_t first_write = size;
	uint32_t rest_write = 0;
	if (buffer->end + size > buffer->bufsize)
	{
		first_write = buffer->bufsize - buffer->end;
		rest_write = size - first_write;
	}

	memcpy(buffer->buffer + buffer->end, in_buf, first_write);
	if (rest_write > 0)
		memcpy(buffer->buffer, (const char*)in_buf + first_write, rest_write);

	buffer->end = (buffer->end + size) % buffer->bufsize;
}

static void fifo_read(fifo_buffer_t* buffer, void* in_buf, uint32_t size)
{
	uint32_t first_read = size;
	uint32_t rest_read = 0;
	if (buffer->first + size > buffer->bufsize)
	{
		first_read = buffer->bufsize - buffer->first;
		rest_read = size - first_read;
	}

	memcpy(in_buf, (const char*)buffer->buffer + buffer->first, first_read);
	if (rest_read > 0)
		memcpy((char*)in_buf + first_read, buffer->buffer, rest_read);

	buffer->first = (buffer->first + size) % buffer->bufsize;
}

static uint32_t resampler_cb(void *userdata, float **data)
{
	audioport_t *port = userdata;
	uint32_t has_read = 0;

	has_read = CELL_AUDIO_BLOCK_SAMPLES_X2;
	sys_lwmutex_lock(&port->lock, SYS_NO_TIMEOUT);
	uint32_t avail = fifo_read_avail(port->buffer);
	if (avail < CELL_AUDIO_BLOCK_SAMPLES_X2 * sizeof(int16_t))
		has_read = avail / sizeof(int16_t);

	fifo_read(port->buffer, port->re_pull_buffer, has_read * sizeof(int16_t));
	sys_lwmutex_unlock(&port->lock);
	sys_lwcond_signal(&port->cond);

	if (has_read < CELL_AUDIO_BLOCK_SAMPLES_X2 * 2)
		memset(port->re_pull_buffer + has_read, 0, (CELL_AUDIO_BLOCK_SAMPLES_X2 - has_read) * sizeof(int16_t));

	resampler_s16_to_float(port->re_buffer, port->re_pull_buffer, CELL_AUDIO_BLOCK_SAMPLES_X2);

	*data = port->re_buffer;
	return CELL_AUDIO_BLOCK_SAMPLES;
}


static uint32_t resampler_get_required_frames(resampler_t* state, uint32_t frames)
{
	uint32_t after_sum = state->sum_output_frames + frames;

	uint32_t min_input_frames = (uint32_t)((after_sum / state->ratio) + 2.0);
	return min_input_frames - state->sum_input_frames;
}

static void poly_create_3(float *poly, float *y)
{
	poly[2] = (y[0] - 2*y[1] + y[2])/2;
	poly[1] = -1.5*y[0] + 2*y[1] - 0.5*y[2];
	poly[0] = y[0];
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
		for (int c = 0; c < 2; c++)
		{
			float poly[3];
			float data[3];
			float x_val;

			if ((int)pos_in == 0)
			{
				data[0] = state->data[c];
				data[1] = state->data[2 + c];
				data[2] = state->data[4 + c];
				x_val = pos_in;
			}
			else
			{
				data[0] = state->data[((int)pos_in - 1) * 2 + c];
				data[1] = state->data[((int)pos_in + 0) * 2 + c];
				data[2] = state->data[((int)pos_in + 1) * 2 + c];
				x_val = pos_in - (int)pos_in + 1.0;
			}

			poly_create_3(poly, data);

			out_data[pos_out * 2 + c] = poly[2] * x_val * x_val + poly[1] * x_val + poly[0];
		}
	}
	frames_used = (int)pos_in;
	return frames_used;
}

static uint32_t resampler_cb_read(resampler_t *state, uint32_t frames, float *data)
{
	// How many frames must we have to resample?
	uint32_t req_frames = resampler_get_required_frames(state, frames);

	// Do we need to read more data?
	if (SAMPLES_TO_FRAMES(state->data_ptr) < req_frames)
	{
		uint32_t must_read = req_frames - SAMPLES_TO_FRAMES(state->data_ptr);
		float temp_buf[FRAMES_TO_SAMPLES(must_read)];

		uint32_t has_read = 0;

		uint32_t copy_size = 0;
		uint32_t ret = 0;
		float *ptr = NULL;
		while (has_read < must_read)
		{
			ret = resampler_cb(state->cb_data, &ptr);

			if (ret == 0 || ptr == NULL) // We're done.
				return -1;

			copy_size = (ret > must_read - has_read) ? (must_read - has_read) : ret;
			memcpy(temp_buf + FRAMES_TO_SAMPLES(has_read), ptr, FRAMES_TO_SAMPLES(copy_size) * sizeof(float));

			has_read += ret;
		}

		// We might have gotten a lot of data from the callback. We should realloc our buffer if needed.
		uint32_t req_buffer_frames = SAMPLES_TO_FRAMES(state->data_ptr) + has_read;

		if (req_buffer_frames > SAMPLES_TO_FRAMES(state->data_size))
		{
			state->data = realloc(state->data, FRAMES_TO_SAMPLES(req_buffer_frames) * sizeof(float));
			if (state->data == NULL)
				return -1;

			state->data_size = FRAMES_TO_SAMPLES(req_buffer_frames);
		}

		memcpy(state->data + state->data_ptr, temp_buf, FRAMES_TO_SAMPLES(must_read) * sizeof(float));
		state->data_ptr += FRAMES_TO_SAMPLES(must_read);

		// We have some data from the callback we need to copy over as well.
		if (ret > copy_size)
		{
			memcpy(state->data + state->data_ptr, ptr + FRAMES_TO_SAMPLES(copy_size), FRAMES_TO_SAMPLES(ret - copy_size) * sizeof(float));
			state->data_ptr += FRAMES_TO_SAMPLES(ret - copy_size);
		}
	}

	// Phew. We should have enough data in our buffer now to be able to process the data we need.

	uint32_t frames_used = resampler_process(state, frames, data);
	state->sum_input_frames += frames_used;
	memmove(state->data, state->data + FRAMES_TO_SAMPLES(frames_used), (state->data_ptr - FRAMES_TO_SAMPLES(frames_used)) * sizeof(float));
	state->data_ptr -= FRAMES_TO_SAMPLES(frames_used);
	state->sum_output_frames += frames;

	return frames;
}

static void resampler_event_loop(audioport_t *port, sys_event_queue_t id)
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

static void* event_loop(void *data)
{
	audioport_t *port = data;

	sys_event_queue_t id;
	sys_ipc_key_t key;

	cellAudioCreateNotifyEventQueue(&id, &key);
	cellAudioSetNotifyEventQueue(key);

	sys_event_t event;

	if (port->re)
		resampler_event_loop(port, id);
	else
	{

		int16_t *in_buf = memalign(128, CELL_AUDIO_BLOCK_SAMPLES_X2 * sizeof(int16_t));
		float *conv_buf = memalign(128, CELL_AUDIO_BLOCK_SAMPLES_X2 * sizeof(float));
		do
		{
			uint32_t has_read = CELL_AUDIO_BLOCK_SAMPLES_X2;
			sys_lwmutex_lock(&port->lock, SYS_NO_TIMEOUT);
			uint32_t avail = fifo_read_avail(port->buffer);
			if (avail < CELL_AUDIO_BLOCK_SAMPLES_X2 * sizeof(int16_t))
				has_read = avail / sizeof(int16_t);

			fifo_read(port->buffer, in_buf, has_read * sizeof(int16_t));
			sys_lwmutex_unlock(&port->lock);

			if (has_read < CELL_AUDIO_BLOCK_SAMPLES_X2)
				memset(in_buf + has_read, 0, (CELL_AUDIO_BLOCK_SAMPLES_X2 - has_read) * sizeof(int16_t));

			resampler_s16_to_float(conv_buf, in_buf, CELL_AUDIO_BLOCK_SAMPLES_X2);

			sys_event_queue_receive(id, &event, SYS_NO_TIMEOUT);
			cellAudioAddData(port->audio_port, conv_buf, CELL_AUDIO_BLOCK_SAMPLES, 1.0);

			sys_lwcond_signal(&port->cond);
		}while(!port->quit_thread);
		free(conv_buf);
	}

	cellAudioRemoveNotifyEventQueue(key);
	pthread_exit(NULL);
	return NULL;
}

static resampler_t* resampler_new(double ratio, int channels, void* cb_data)
{
	if (channels < 1)
		return NULL;

	resampler_t* state = calloc(1, sizeof(resampler_t));
	if (state == NULL)
		return NULL;

	state->ratio = ratio;
	state->channels = channels;
	state->cb_data = cb_data;

	return state;
}

static cell_audio_handle_t audioport_init(const struct cell_audio_params *params)
{
	init_audioport();

	audioport_t *handle = calloc(1, sizeof(*handle));

	CellAudioPortParam port_params = {
		.nChannel = params->channels,
		.nBlock = 8,
		.attr = 0
	};

	handle->channels = params->channels;

	handle->userdata = params->userdata;
	handle->buffer = fifo_new(params->buffer_size ? params->buffer_size : 4096);

	if (params->samplerate != 48000)
	{
		handle->re = resampler_new(48000.0 / params->samplerate, params->channels, handle);
	}

	sys_lwmutex_attribute_t attr;
	sys_lwmutex_attribute_t attr2;
	sys_lwcond_attribute_t cond_attr;

	sys_lwmutex_attribute_initialize(attr);
	sys_lwmutex_create(&handle->lock, &attr);

	sys_lwmutex_attribute_initialize(attr2);
	sys_lwmutex_create(&handle->cond_lock, &attr2);

	sys_lwcond_attribute_initialize(cond_attr);
	sys_lwcond_create(&handle->cond, &handle->cond_lock, &cond_attr);

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

	sys_lwmutex_destroy(&port->lock);
	sys_lwmutex_destroy(&port->cond_lock);
	sys_lwcond_destroy(&port->cond);

	if (port->re)
		resampler_free(port->re);
	if (port->buffer)
		fifo_free(port->buffer);

	cellAudioPortStop(port->audio_port);
	cellAudioPortClose(port->audio_port);

	free(port);
}


static int32_t audioport_write(cell_audio_handle_t handle, const int16_t *data, uint32_t samples)
{
	int32_t ret = samples;
	uint32_t bytes = samples * sizeof(int16_t);

	audioport_t *port = handle;
	do
	{
		sys_lwmutex_lock(&port->lock, SYS_NO_TIMEOUT);
		uint32_t avail = fifo_write_avail(port->buffer);
		sys_lwmutex_unlock(&port->lock);

		uint32_t to_write = avail < bytes ? avail : bytes;
		if (to_write > 0)
		{
			sys_lwmutex_lock(&port->lock, SYS_NO_TIMEOUT);
			fifo_write(port->buffer, data, to_write);
			sys_lwmutex_unlock(&port->lock);
			bytes -= to_write;
			data += to_write >> 1;
		}
		else
		{
			sys_lwmutex_lock(&port->cond_lock, SYS_NO_TIMEOUT);
			sys_lwcond_wait(&port->cond, 0);
			sys_lwmutex_unlock(&port->cond_lock);
		}
	}while(bytes);

	return ret;
}

const cell_audio_driver_t cell_audio_audioport = {
	.init = audioport_init,
	.write = audioport_write,
	//.write_avail = audioport_write_avail,
	.pause = audioport_pause,
	.unpause = audioport_unpause,
	.is_paused = audioport_is_paused,
	.free = audioport_free
};
