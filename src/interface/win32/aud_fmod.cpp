// FMOD Ex module, added by regret

/* changelog:
 update 1: create
*/

#ifndef NO_FMODEX

#include "burner.h"
#include "aud_dsp.h"
#include "../../lib/include/fmodex/fmod.hpp"
#include "../../lib/include/fmodex/fmod_errors.h"

#ifdef _MSC_VER
#pragma comment(lib, "fmodex_vc")
#endif

#define MAX_CHANNEL 2

static BOOL ERRCHECK(FMOD_RESULT result)
{
	if (result != FMOD_OK) {
		printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
		return TRUE;
	}
	return FALSE;
}

static FMOD_RESULT F_CALLBACK pcmreadcallback(FMOD_SOUND* sound, void* data, unsigned int datalen)
{
	if (pAudNextSound) {
		memcpy(data, pAudNextSound, datalen);
	}

	return FMOD_OK;
}

static FMOD_RESULT F_CALLBACK pcmsetposcallback(FMOD_SOUND* sound, int subsound, unsigned int position, FMOD_TIMEUNIT postype)
{
	/*
	    This is useful if the user calls Channel::setPosition and you want to seek your data accordingly.
	*/
	return FMOD_OK;
}


class AudioFMODEx : public Audio {
public:
	FMOD::System* fmodSystem;
	FMOD::Sound* fmodSound;
	FMOD::Channel* fmodChannel;
	FMOD_CREATESOUNDEXINFO fomdInfo;

	int (*FMODGetNextSound)(int);

	int set(int (*callback)(int))
	{
		if (!callback) {
			FMODGetNextSound = AudWriteSlience;
		} else {
			FMODGetNextSound = callback;
		}

		return 0;
	}

	int blank()
	{
		if (pAudNextSound) {
			AudWriteSlience();
		}
		return 0;
	}

	int check()
	{
		if (!fmodSystem) {
			return 1;
		}

//		fmodSystem->update();

		FMODGetNextSound(true);

		return 0;
	}

	int exit()
	{
		DspExit();

		if (pAudNextSound) {
			free(pAudNextSound);
			pAudNextSound = NULL;
		}

		fmodSound->release();
		fmodSound = NULL;
		fmodSystem->release();
		fmodSystem = NULL;

		return 0;
	}

	int init()
	{
		if (nAudSampleRate <= 0) {
			return 0;
		}

		fps = nAppVirtualFps;

		// Calculate the Seg Length and Loop length (round to nearest sample)
		nAudSegLen = (nAudSampleRate * 100 + (fps >> 1)) / fps;
		nAudAllocSegLen = nAudSegLen << 2;
//		loopLen = (nAudSegLen * nAudSegCount) << 2;

		pAudNextSound = (short*)malloc(nAudAllocSegLen);				// The next sound block to put in the stream
		if (pAudNextSound == NULL) {
			exit();
			return 1;
		}

		FMOD_RESULT result = FMOD::System_Create(&fmodSystem);			// Create the main system object.
		if (ERRCHECK(result)) {
			return 1;
		}

		unsigned int version;
		result = fmodSystem->getVersion(&version);						// Version check
//		if (ERRCHECK(result) || version < FMOD_VERSION) {
//			return 1;
//		}

		result = fmodSystem->init(MAX_CHANNEL, FMOD_INIT_NORMAL, 0);	// Initialize FMOD.
		if (ERRCHECK(result)) {
			return 1;
		}

		FMOD_MODE mode = FMOD_OPENUSER | FMOD_OPENRAW | FMOD_LOOP_NORMAL | FMOD_SOFTWARE;

		memset(&fomdInfo, 0, sizeof(FMOD_CREATESOUNDEXINFO));
		fomdInfo.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
		fomdInfo.numchannels = 2;
		fomdInfo.defaultfrequency = nAudSampleRate;
		fomdInfo.format = FMOD_SOUND_FORMAT_PCM16;
		fomdInfo.pcmreadcallback = pcmreadcallback;
		fomdInfo.suggestedsoundtype = FMOD_SOUND_TYPE_USER;
		fomdInfo.length = nAudAllocSegLen;
		fomdInfo.decodebuffersize = nAudAllocSegLen / 4;

		result = fmodSystem->createStream(0, mode, &fomdInfo, &fmodSound);
		if (ERRCHECK(result)) {
			return 1;
		}

		result = fmodSystem->playSound(FMOD_CHANNEL_FREE, fmodSound, false, &fmodChannel);
		if (ERRCHECK(result)) {
			return 1;
		}

		set(NULL);

		DspInit();

		return 0;
	}

	int play()
	{
		blank();

		if (!fmodChannel) {
			return 0;
		}

		return 0;
	}

	int stop()
	{
		if (!bAudOkay) {
			return 1;
		}

		if (!fmodChannel) {
			return 0;
		}

		return 0;
	}

	int setvolume(int vol)
	{
		if (vol == 10000) {
			volume = 1.0f;
		} else {
			if (vol == 0) {
				volume = 0.0f;
			} else {
				volume = 1.0f * vol / 10000;
			}
		}

		if (volume < 0.0f) {
			volume = 0.0f;
		}

		if (!fmodChannel) {
			return 0;
		}

		if (ERRCHECK(fmodChannel->setVolume(volume))) {
			return 1;
		}

		return 0;
	}

	int get(void* info)
	{
		InterfaceInfo* pInfo = (InterfaceInfo*)info;
		if (!pInfo) {
			return 1;
		}

		WCHAR szString[MAX_PATH] = _T("");
		_sntprintf(szString, sizearray(szString), _T("Audio is delayed by approx. %ims"), int(100000.0 / (fps / (nAudSegCount - 1.0))));
		IntInfoAddStringModule(pInfo, szString);

		return 0;
	}

	int setfps()
	{
		if (nAudSampleRate <= 0) {
			return 0;
		}

		return 0;
	}

	AudioFMODEx() {
		fmodSystem = NULL;
		fmodSound = NULL;
		fmodChannel = NULL;

		loopLen = 0;
		fps = 0;
		volume = 1.0f;
	}

	~AudioFMODEx() {
		exit();
	}
};

#endif
