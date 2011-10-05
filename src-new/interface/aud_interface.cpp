// Audio Output
#include "burner.h"

int nAudSampleRate[8] = { 22050, 22050, 22050, 22050, 22050, 22050, 22050, 22050 };			// sample rate
int nAudVolume = 10000;				// Sound volume (% * 100)
int nAudSegCount = 6;				// Segs in the pdsbLoop buffer
int nAudSegLen = 0;					// Seg length in samples (calculated from Rate/Fps)
int nAudAllocSegLen = 0;
unsigned char bAudOkay = 0;			// True if DSound was initted okay
unsigned char bAudPlaying = 0;		// True if the Loop buffer is playing

int nAudDSPModule[8] = { 0, };				// DSP module to use: 0 = none, 1 = low-pass filter

short* nAudNextSound = NULL;		// The next sound seg we will add to the sample loop

unsigned int nAudSelect = 0;		// Which audio plugin is selected
static unsigned int nAudActive = 0;

#if defined (BUILD_WIN32)
	extern struct AudOut AudOutDx;
	extern struct AudOut AudOutXAudio2;
#elif defined (BUILD_SDL)
	extern struct AudOut AudOutSDL;
#endif

static struct AudOut *pAudOut[]=
{
#if defined (BUILD_WIN32)
	&AudOutDx,
	&AudOutXAudio2,
#elif defined (BUILD_SDL)
	&AudOutSDL,
#endif
};

#define AUD_LEN (sizeof(pAudOut)/sizeof(pAudOut[0]))

static InterfaceInfo AudInfo = { NULL, NULL, NULL };

int AudBlankSound()
{
	if (!bAudOkay || nAudActive >= AUD_LEN) {
		return 1;
	}
	return pAudOut[nAudActive]->BlankSound();
}

// This function checks the Sound loop, and if necessary gets some more sound
int AudSoundCheck()
{
	if (!bAudOkay || nAudActive >= AUD_LEN) {
		return 1;
	}
	return pAudOut[nAudActive]->SoundCheck();
}

int AudSoundInit()
{
	int nRet;

	if (nAudSelect >= AUD_LEN) {
		return 1;
	}
	
	nAudActive = nAudSelect;

	if ((nRet = pAudOut[nAudActive]->SoundInit()) == 0) {
		bAudOkay = true;
	}

	return nRet;
}

int AudSetCallback(int (*pCallback)(int))
{
	if (!bAudOkay || nAudActive >= AUD_LEN) {
		return 1;
	}
	return pAudOut[nAudActive]->SetCallback(pCallback);
}

int AudSoundPlay()
{
	if (!bAudOkay || nAudActive >= AUD_LEN) {
		return 1;
	}
	
	int nRet = pAudOut[nAudActive]->SoundPlay();
	if (!nRet) bAudPlaying = true;
	
	return nRet;
}

int AudSoundStop()
{
	if (nAudActive >= AUD_LEN) {
		return 1;
	}
	
	bAudPlaying = false;
	
	return pAudOut[nAudActive]->SoundStop();
}

int AudSoundExit()
{
	IntInfoFree(&AudInfo);
	
	if (!bAudOkay || nAudActive >= AUD_LEN) {
		return 1;
	}
	bAudOkay = false;

	int nRet = pAudOut[nAudActive]->SoundExit();
	
	nAudActive = 0;
	
	return nRet;
}

int AudSoundSetVolume()
{
	if (!bAudOkay || nAudActive >= AUD_LEN) {
		return 1;
	}
	return pAudOut[nAudActive]->SoundSetVolume();
}

InterfaceInfo* AudGetInfo()
{
	if (IntInfoInit(&AudInfo)) {
		IntInfoFree(&AudInfo);
		return NULL;
	}

	if (bAudOkay) {
		TCHAR szString[MAX_PATH] = _T("");

		AudInfo.pszModuleName = pAudOut[nAudActive]->szModuleName;

		_sntprintf(szString, MAX_PATH, _T("Playback at %iHz, %i%% volume"), nAudSampleRate[nAudActive], nAudVolume / 100);
		IntInfoAddStringInterface(&AudInfo, szString);

		if (nAudDSPModule[nAudActive]) {
			IntInfoAddStringInterface(&AudInfo, _T("Applying low-pass filter"));
		}

	 	if (pAudOut[nAudSelect]->GetPluginSettings) {
			pAudOut[nAudSelect]->GetPluginSettings(&AudInfo);
		}
	} else {
		IntInfoAddStringInterface(&AudInfo, _T("Audio plugin not initialised"));
	}

	return &AudInfo;
}

int AudSelect(unsigned int nPlugIn)
{
	if (nPlugIn < AUD_LEN) {
		nAudSelect = nPlugIn;
		return 0;
	}
	
	return 1;
}

void AudWriteSilence()
{
	if (nAudNextSound) {
		memset(nAudNextSound, 0, nAudAllocSegLen);
	}
}
