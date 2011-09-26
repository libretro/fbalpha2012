#include <cell/audio.h>
#include <cell/sysmodule.h>
#include "cellframework2/audio/stream.h"
 
#define AUDIO_SEGMENT_LENGTH 801
#define AUDIO_SEGMENT_LENGTH_TIMES_CHANNELS 1602

extern cell_audio_handle_t audio_handle;
extern const struct cell_audio_driver *driver;

static int audio_new()
{
	driver = &cell_audio_audioport;
	audio_handle = NULL;
	return 1;
}

static void audio_destructor()
{
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
}

int audio_exit()
{ 
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

int audio_check()
{
	pBurnSoundOut = pAudNextSound;

	int16_t * currentSound = pAudNextSound;
	driver->write(audio_handle, currentSound, AUDIO_SEGMENT_LENGTH_TIMES_CHANNELS);
	return 0;
}

static int audio_init()
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
	pAudNextSound = (int16_t*)malloc(nAudAllocSegLen);

	if (pAudNextSound == NULL)
	{
		audio_exit();
		return 1;
	}

	AudWriteSlience();

	return 0;
}
