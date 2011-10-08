#include <cell/audio.h>
#include <cell/sysmodule.h>
#include "cellframework2/audio/stream.h"
#include "audio_driver.h"
#include "burner.h"

extern cell_audio_handle_t audio_handle;
extern const struct cell_audio_driver *	driver;

int		nAudSampleRate = 48010;	// Sample rate
int		nAudSegCount = 6;	// Segments in the pdsbLoop buffer
int		nAudSegLen = 0;		// Segment length in samples (calculated from Rate/Fps)
int		nAudAllocSegLen = 0;	// Allocated segment length in samples
bool		bAudOkay = false;	// True if sound was inited okay
bool		bAudPlaying = false;
int16_t *	pAudNextSound = NULL;	// The next sound seg we will add to the sample loop

int audio_new(void)
{
	pAudNextSound = (int16_t*)memalign(128, AUDIO_SEGMENT_LENGTH_TIMES_CHANNELS * sizeof(int16_t));
	driver = &cell_audio_audioport;
	audio_handle = NULL;

	return 0;
}

int audio_exit()
{ 
	bAudOkay = bAudPlaying = false;

	if (audio_handle)
	{
		driver->free(audio_handle);
		audio_handle = NULL;
	}

	if (pAudNextSound)
	{
		free(pAudNextSound);
		pAudNextSound = NULL;
	}

	return 0;
}

int audio_init(void)
{
	nAudSegLen = AUDIO_SEGMENT_LENGTH;
	nAudAllocSegLen = 12800;

	cell_audio_params params;
	memset(&params, 0, sizeof(params));
	params.channels = 2;
	params.samplerate = 48000;
	params.buffer_size = 8192;
	params.sample_cb = NULL;
	params.userdata = NULL;
	params.device = NULL;
	audio_handle = driver->init(&params);

	// The next sound block to put in the stream
	pAudNextSound = (int16_t*)memalign(128, nAudAllocSegLen);

	if (pAudNextSound == NULL)
	{
		audio_exit();
		return 1;
	}

	audio_blank();
	bAudOkay = true;

	return 0;
}

// Write silence into the buffer

int audio_blank(void)
{
	if (pAudNextSound)
	{
		memset(pAudNextSound, 0, nAudAllocSegLen);
		return 0;
	}
	else
		return 1;
}

void audio_play(void)
{
	bAudPlaying = true;
}

int audio_stop(void)
{
	bAudPlaying = false;
	return bAudOkay ? 0 : 1;
}
