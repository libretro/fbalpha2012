#include <cell/audio.h>
#include <cell/sysmodule.h>
#include <sys/process.h>
#include "cellframework2/audio/stream.h"
 
#define AUDIO_CHANNELS 2
#define AUDIO_SAMPLERATE 48000
#define AUDIO_SEGMENT_LENGTH 801
#define AUDIO_SEGMENT_LENGTH_TIMES_CHANNELS 1602
#define FPS 6000

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
   #if 0
   //we don't use a DSP - low-pass filter - reverb
   DspExit();
   #endif

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

   //low-pass filter not used at the time - so just skip this if
   #if 0
   if (nAudDSPModule & 1)
   {
      if (!bRunPause)
         DspDo(pAudNextSound, nAudSegLen);
      else
         AudWriteSlience();
   }
   #endif

   int16_t *currentSound = pAudNextSound;
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

   pAudNextSound = (short*)malloc(nAudAllocSegLen);		// The next sound block to put in the stream
   if (pAudNextSound == NULL)
   {
      audio_exit();
      return 1;
   }

   AudWriteSlience();

   //we don't use a DSP - low-pass - reverb filter
   //DspInit();

   return 0;
}

#if 0
class Audio {
public:
	//int currentBuffer;

	//AudioStream::AudioPort<int16_t> *CellAudio;
   cell_audio_handle_t audio_handle;
   const struct cell_audio_driver *driver;

	//int (*CellAudioGetNextSound)(int);
	//float volume;


   Audio() {
      driver = &cell_audio_audioport;
      audio_handle = NULL;

		//CellAudio = NULL;		 
      //volume = 1.0f;
	}

	~Audio() {
		exit();	
	}

   #if 0
	int set(int (*callback)(int))
	{
		if (callback == NULL) {
			CellAudioGetNextSound = AudWriteSlience;
		} else {
			CellAudioGetNextSound = callback;
		}

		return 0;
	}
   #endif

   #if 0
	int blank()
	{
		// Also blank the nAudNextSound buffer
		if (pAudNextSound) {
			AudWriteSlience();
		}
		 
		return 0;
	}
   #endif

	int check()
	{
      pBurnSoundOut = pAudNextSound;
		
      //low-pass filter not used at the time - so just skip this if
      /*
		if (nAudDSPModule & 1) {
			if (!bRunPause)
				DspDo(pAudNextSound, nAudSegLen);
			else
				AudWriteSlience();
		}		
      */

				
      int16_t *currentSound = pAudNextSound;

      //CellAudio->write(currentSound, AUDIO_SEGMENT_LENGTH_TIMES_CHANNELS);
      driver->write(audio_handle, currentSound, AUDIO_SEGMENT_LENGTH_TIMES_CHANNELS);
	  

		return 0;
	}

	int exit()
	{ 
      //we don't use a DSP - low-pass filter - reverb
		//DspExit();

      /*
		if (CellAudio)
		{	
			//sys_timer_usleep(800000);
			delete CellAudio;
			CellAudio = NULL;
		}
      */
      if (audio_handle)
      {
         driver->free(audio_handle);
         audio_handle = NULL;
      }

		if (pAudNextSound) {
			free(pAudNextSound);
			pAudNextSound = NULL;
		}

		//CellAudioGetNextSound = NULL;       
		return 0;
	}


	int init()
	{			
		nAudSegLen = AUDIO_SEGMENT_LENGTH;
		nAudAllocSegLen = 12800;
		 
		//CellAudio = new AudioStream::AudioPort<int16_t>(2, AUDIO_SAMPLERATE);
      cell_audio_params params;
      memset(&params, 0, sizeof(params));
      params.channels = 2;
      params.samplerate = 48000;
      params.buffer_size = 8192;
      params.sample_cb = NULL;
      params.userdata = NULL;
      params.device = NULL;
      audio_handle = driver->init(&params);
		 
		pAudNextSound = (short*)malloc(nAudAllocSegLen);		// The next sound block to put in the stream
		if (pAudNextSound == NULL) {
			exit();
			return 1;
		}
  
      AudWriteSlience();

      //we don't use a DSP - low-pass - reverb filter
		//DspInit();
 
		return 0;
	}


    int play()
    {     
        return 0;
    }

    int stop()
    {
        if (!bAudOkay) {
                return 1;
        }
         
        return 0;
    }

    int setvolume(int vol)
    {            
        return 0;
    }

   #if 0
    int get(void* info)
    {             
        return 0;
    }

	int setfps()
	{
		if (AUDIO_SAMPLERATE <= 0) {
			return 0;
		}

		return 0;
	}
   #endif
};
#endif
