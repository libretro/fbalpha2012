// Audio Output
// class version by regret (ref: bsnes)

#include "burner.h"

int nAudSampleRate = 44100;		// sample rate
int nAudSegCount = 6;			// Segs in the pdsbLoop buffer
int nAudSegLen = 0;				// Seg length in samples (calculated from Rate/Fps)
int nAudAllocSegLen = 0;		// Allocated seg length in samples
bool bAudOkay = false;			// True if DSound was initted okay
int nAudDSPModule = 0;			// DSP module to use: 0 = none, 1 = low-pass filter, 2 = reverb
int audStereoUpmixing = 0;		// Enable stereo upmixing (XAudio2 only)

TCHAR audSelect[16] = _T("");	// Which audio plugin is selected
static TCHAR audActive[16];		// Which audio plugin is actived
int dsDevice = 0;					// dsound device
int oalDevice = 0;					// openal device
int xa2Device = 0;					// xaudio2 device
int cellAudioDevice = 0;			// ps3 cell audio device


short* pAudNextSound = NULL;	// The next sound seg we will add to the sample loop

static InterfaceInfo AudInfo = { NULL, };

// Write silence into the buffer
int AudWriteSlience(int)
{
	if (pAudNextSound)
		memset(pAudNextSound, 0, nAudAllocSegLen);
	return 0;
}

// AudioInterface
#include "aud_ps3_.h"

AudioInterface audio;

void AudioInterface::driver(const TCHAR* _driver)
{
	if(p)
		term();

	if (!_tcscmp(_driver, _T("CellAudio")))
		p = audio_new();
}

void AudioInterface::term()
{
	if (p)
	{
		audio_destructor();
		p = 0;
	}
}

int AudioInterface::blank()
{
	AudWriteSlience(1);
}

// This function checks the Sound loop, and if necessary gets some more sound
void AudioInterface::check()
{
	audio_check();
}

int AudioInterface::init()
{
	int nRet;

	_tcscpy(audActive, audSelect);

	if (!p)
		driver(audActive);

	if ((nRet = audio_init()) == 0)
		bAudOkay = true;

	return nRet;
}

int AudioInterface::set(int (*callback)(int))
{
	return 0;
}

int AudioInterface::play()
{
	int ret = 0;

	if(!ret)
		bAudPlaying = true;

	return ret;
}

int AudioInterface::stop()
{
	bAudPlaying = false;
	int returnvalue = !bAudOkay ? 1 : 0;
	return p ? returnvalue : 1;
}

int AudioInterface::exit()
{
	IntInfoFree(&AudInfo);

	bAudOkay = false;

	int ret = 0;
	if(p)
	{
		ret = audio_exit();
		term();
	}
	return ret;
}

int AudioInterface::setvolume(int vol)
{
	return 0;
}

int AudioInterface::select(const TCHAR* _driver)
{
	if (!_driver || !*_driver)
		return 1;

	_tcscpy(audSelect, _driver);
	return 0;
}
