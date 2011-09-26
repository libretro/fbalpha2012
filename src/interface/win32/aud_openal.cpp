// OpenAL module, added by regret (Thanks to VBA-M team)

/* changelog:
 update 1: create (ref: bsnes, vba-m)
*/

#ifndef NO_OPENAL

//#include "burner.h"
#include "aud_dsp.h"
#include "loadoal.h"

class AudioOpenAL : public Audio {
public:
	OPENALFNTABLE ALFunction;

	struct {
		ALCdevice* handle;
		ALCcontext* context;
		ALuint source;
		ALenum format;
	} device;

	bool initfinish;

	ALuint* buffer;
	ALuint tempBuffer;
	bool buffersLoaded;
	int freq;

	int (*ALGetNextSound)(int);

	int set(int (*callback)(int))
	{
		if (callback == NULL) {
			ALGetNextSound = AudWriteSlience;
		} else {
			ALGetNextSound = callback;
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
		if (!buffer) {
			return 1;
		}

		ALint sourceState = 0;
		ALint nBuffersProcessed = 0;

		if (!buffersLoaded) {
			// ==initial buffer filling==
			for (int i = 0; i < nAudSegCount; i++) {
				// Filling the buffers explicitly with silence would be cleaner,
				// but the very first sample is usually silence anyway.
				ALFunction.alBufferData(buffer[i], device.format, pAudNextSound, nAudAllocSegLen, freq);
			}

			ALFunction.alSourceQueueBuffers(device.source, nAudSegCount, buffer);

			buffersLoaded = true;
		} else {
			// ==normal buffer refreshing==
			nBuffersProcessed = 0;
			ALFunction.alGetSourcei(device.source, AL_BUFFERS_PROCESSED, &nBuffersProcessed);

			if (nBuffersProcessed == nAudSegCount) {
				// we only want to know about it when we are emulating at full speed or faster
			}

			if (!autoFrameSkip && !bAppDoFast) {
				// wait until at least one buffer has finished
				while (nBuffersProcessed == 0) {
					// wait for about half the time one buffer needs to finish
					// unoptimized: ( nAudAllocSegLen * 1000 ) / ( freq * 2 * 2 ) * 1/2
					Sleep(nAudAllocSegLen / (freq >> 7));
					ALFunction.alGetSourcei(device.source, AL_BUFFERS_PROCESSED, &nBuffersProcessed);
				}
			} else {
				if (nBuffersProcessed == 0) {
					Sleep(2);
					return 0;
				}
			}

			assert(nBuffersProcessed > 0);

			ALGetNextSound(true);

			// dsp update
			if (nAudDSPModule & 1) {
				if (bRunPause)
					AudWriteSlience();
				else
					DspDo(pAudNextSound, nAudSegLen);
			}

			// unqueue buffer
			tempBuffer = 0;
			ALFunction.alSourceUnqueueBuffers(device.source, 1, &tempBuffer);

			// refill buffer
			ALFunction.alBufferData(tempBuffer, device.format, pAudNextSound, nAudAllocSegLen, freq);

			// requeue buffer
			ALFunction.alSourceQueueBuffers(device.source, 1, &tempBuffer);
		}

		// start playing the source if necessary
		ALFunction.alGetSourcei(device.source, AL_SOURCE_STATE, &sourceState );
		if (sourceState != AL_PLAYING) {
			ALFunction.alSourcePlay(device.source);
		}

		return 0;
	}

	int exit()
	{
		if (!initfinish) {
			return 0;
		}

		DspExit();

		if (pAudNextSound) {
			free(pAudNextSound);
			pAudNextSound = NULL;
		}

		ALFunction.alSourceStop(device.source);
		ALFunction.alSourcei(device.source, AL_BUFFER, 0);
		ALFunction.alDeleteSources(1, &device.source);
		device.source = 0;

		ALFunction.alDeleteBuffers(nAudSegCount, buffer);
		free(buffer);
		buffer = NULL;

		if (device.context) {
			ALFunction.alcMakeContextCurrent(NULL);
			ALFunction.alcDestroyContext(device.context);
			device.context = 0;
		}

		if (device.handle) {
			ALFunction.alcCloseDevice(device.handle);
			device.handle = 0;
		}

		buffersLoaded = false;
		ALGetNextSound = NULL;

		UnloadOAL10Library();
		initfinish = false;

		return 0;
	}

	int init()
	{
		if (nAudSampleRate <= 0) {
			return 0;
		}
		freq = nAudSampleRate;

		// load openal32.dll
		if (!LoadOAL10Library(NULL, &ALFunction)) {
			return 1;
		}

		fps = nAppVirtualFps;

		// Calculate the Seg Length and Loop length (round to nearest sample)
		nAudSegLen = (nAudSampleRate * 100 + (fps >> 1)) / fps;
		nAudAllocSegLen = nAudSegLen << 2;
//		loopLen = (nAudSegLen * nAudSegCount) << 2;

		pAudNextSound = (short*)malloc(nAudAllocSegLen);		// The next sound block to put in the stream
		if (pAudNextSound == NULL) {
			exit();
			return 1;
		}

		buffer = (ALuint*)malloc(nAudSegCount * sizeof(ALuint));
		if (buffer == NULL) {
			exit();
			return 1;
		}
		memset(buffer, 0, nAudSegCount * sizeof(ALuint));

		// Initialize OpenAL
		ALenum error;

		device.source = 0;
		device.handle = 0;
		device.context = 0;
		device.format = AL_FORMAT_STEREO16;

		ALCchar* mydevice = NULL;
		ALCchar seldevice[256];
		if (oalDevice > 0) {
			unsigned int deviceCount = 0;
			const ALchar* devices = ALFunction.alcGetString(NULL, ALC_DEVICE_SPECIFIER);
			if (strlen(devices)) {
				while (*devices) {
					if (oalDevice == deviceCount) {
						strcpy(seldevice, devices);
						mydevice = seldevice;
						break;
					}
					devices += strlen(devices) + 1;
					deviceCount++;
				}
			}
		}

		if (!(device.handle = ALFunction.alcOpenDevice(mydevice))) {
			exit();
			return 1;
		}
		if (!(device.context = ALFunction.alcCreateContext(device.handle, NULL))) {
			exit();
			return 1;
		}
		ALFunction.alGetError();

		ALFunction.alcMakeContextCurrent(device.context);
		if ((error = ALFunction.alGetError()) != AL_NO_ERROR) {
			exit();
			return 1;
		}

		ALFunction.alGenSources(1, &device.source);
		if ((error = ALFunction.alGetError()) != AL_NO_ERROR) {
			exit();
			return 1;
		}

		ALFunction.alGenBuffers(nAudSegCount, buffer);

		set(NULL);

		DspInit();

		initfinish = true;

		return 0;
	}

	int play()
	{
		blank();

		ALFunction.alSourcePlay(device.source);

		return 0;
	}

	int stop()
	{
		if (!bAudOkay) {
			return 1;
		}

		ALFunction.alSourceStop(device.source);

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

		if (device.source) {
			ALFunction.alSourcef(device.source, AL_MAX_GAIN, volume);
		}

		return 0;
	}

	int get(void* info)
	{
		InterfaceInfo* pInfo = (InterfaceInfo*)info;
		if (!pInfo) {
			return 1;
		}

		TCHAR szString[MAX_PATH] = _T("");
		_sntprintf(szString, sizearray(szString), _T("Audio is delayed by approx. %ims"), int(100000.0 / (fps / (nAudSegCount - 1.0))));
		IntInfoAddStringModule(pInfo, szString);

		// get device info
		unsigned int deviceCount = 0;

		const ALchar* devices = ALFunction.alcGetString(NULL, ALC_DEVICE_SPECIFIER);
		if (strlen(devices)) {
			while (*devices) {
				devices += strlen(devices) + 1;
				deviceCount++;
			}
		}
		if (deviceCount <= 0) {
			return 1;
		}

		pInfo->deviceNum = deviceCount;
		pInfo->deviceName = (TCHAR**)malloc((pInfo->deviceNum) * sizeof(TCHAR*));
		if (!pInfo->deviceName) {
			return 1;
		}
		memset(pInfo->deviceName, 0, pInfo->deviceNum * sizeof(TCHAR*));

		unsigned int i = 0;
		const ALchar* devices1 = ALFunction.alcGetString(NULL, ALC_DEVICE_SPECIFIER);
		if (strlen(devices1)) {
			while (*devices1) {
				pInfo->deviceName[i] = (TCHAR*)malloc(256 * sizeof(TCHAR));
				if (!pInfo->deviceName[i]) {
					return 1;
				}
				_tcscpy(pInfo->deviceName[i], AtoW(devices1));
				devices1 += strlen(devices1) + 1;
				i++;
			}
		}

		return 0;
	}

	int setfps()
	{
		if (nAudSampleRate <= 0) {
			return 0;
		}

		bAudOkay = false;									// This module is no longer okay

		ALFunction.alDeleteBuffers(nAudSegCount, buffer);
		free(buffer);
		buffer = NULL;

		free(pAudNextSound);
		pAudNextSound = NULL;

		fps = nAppVirtualFps;

		// Calculate the Seg Length and Loop length (round to nearest sample)
		nAudSegLen = (nAudSampleRate * 100 + (fps >> 1)) / fps;
		nAudAllocSegLen = nAudSegLen << 2;
//		loopLen = (nAudSegLen * nAudSegCount) << 2;

		pAudNextSound = (short*)malloc(nAudAllocSegLen);	// The next sound block to put in the stream
		if (pAudNextSound == NULL) {
			exit();
			return 1;
		}

		buffer = (ALuint*)malloc(nAudSegCount * sizeof(ALuint));
		if (buffer == NULL) {
			exit();
			return 1;
		}
		memset(buffer, 0, nAudSegCount * sizeof(ALuint));

		ALFunction.alGenBuffers(nAudSegCount, buffer);
		buffersLoaded = false;

		bAudOkay = true;									// This module was initted okay

		return 0;
	}

	AudioOpenAL() {
		initfinish = false;

		buffer = 0;
		tempBuffer = 0;
		buffersLoaded = false;
		freq = 0;

		loopLen = 0;
		fps = 0;
		volume = 1.0f;
	}

	~AudioOpenAL() {
		exit();
	}
};

#endif
