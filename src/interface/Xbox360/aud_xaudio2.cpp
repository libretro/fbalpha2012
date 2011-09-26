// XAudio2 module, added by regret (Thanks to VBA-M team)

/* changelog:
 update 2: add init/exit voice interfaces, add reverb effect (study perpose)
 update 1: create (ref: vba-m)
*/

#ifndef NO_XAUDIO2

//#include "burner.h"
#include "aud_dsp.h"
#include <xaudio2.h>
#ifdef REVERB_EFFECT
#include <xaudio2fx.h>
#endif

class AudioXaudio2 : public Audio {
public:
	IXAudio2* pXAudio2;
	IXAudio2MasteringVoice* pMasterVoice;
	IXAudio2SourceVoice* pSourceVoice;
	XAUDIO2_BUFFER sAudioBuffer;
	XAUDIO2_VOICE_STATE vState;
	IUnknown* pXAPO;
	bool effectEnable;

	BYTE* pAudioBuffers;
	int currentBuffer;

	struct StreamingVoiceContext : public IXAudio2VoiceCallback
	{
		HANDLE hBufferEndEvent;

		StreamingVoiceContext() {
			hBufferEndEvent = NULL;
			hBufferEndEvent = CreateEvent( NULL, FALSE, FALSE, NULL);
		}
		~StreamingVoiceContext() {
			CloseHandle(hBufferEndEvent);
			hBufferEndEvent = NULL;
		}

		STDMETHOD_(void, OnBufferEnd) (void *pBufferContext) {
			SetEvent(hBufferEndEvent);
		}

		// dummies:
		STDMETHOD_(void, OnVoiceProcessingPassStart) (UINT32 BytesRequired) {}
		STDMETHOD_(void, OnVoiceProcessingPassEnd) () {}
		STDMETHOD_(void, OnStreamEnd) () {}
		STDMETHOD_(void, OnBufferStart) (void *pBufferContext) {}
		STDMETHOD_(void, OnLoopEnd) (void *pBufferContext) {}
		STDMETHOD_(void, OnVoiceError) (void *pBufferContext, HRESULT Error) {};
	};
	StreamingVoiceContext voiceContext;

	int (*XAudio2GetNextSound)(int);

	int set(int (*callback)(int))
	{
		if (callback == NULL) {
			XAudio2GetNextSound = AudWriteSlience;
		} else {
			XAudio2GetNextSound = callback;
		}

		return 0;
	}

	int blank()
	{
		if (pAudioBuffers) {
			memset(pAudioBuffers, 0, loopLen);
		}

		// Also blank the nAudNextSound buffer
		if (pAudNextSound) {
			AudWriteSlience();
		}

		return 0;
	}

	int check()
	{
		if (!pSourceVoice || !pAudioBuffers) {
			return 1;
		}

		while (true) {
			pSourceVoice->GetState(&vState);

			assert(vState.BuffersQueued < nAudSegCount);

			if (vState.BuffersQueued < nAudSegCount - 1) {
				if (vState.BuffersQueued == 0) {
					// buffers ran dry
				}
				// there is at least one free buffer
				break;
			} else {
				// the maximum number of buffers is currently queued
				if (!autoFrameSkip && !bAppDoFast) {
					// wait for one buffer to finish playing
					WaitForSingleObject(voiceContext.hBufferEndEvent, INFINITE);
				} else {
					// drop current audio frame
					Sleep(2);
					return 0;
				}
			}
		}

		XAudio2GetNextSound(true);

		// dsp update
		if (nAudDSPModule & 1) {
			if (bRunPause)
				AudWriteSlience();
			else
				DspDo(pAudNextSound, nAudSegLen);
		}
#ifdef REVERB_EFFECT
		if (nAudDSPModule & 2) {
			if (!effectEnable) {
				pSourceVoice->EnableEffect(0);
				effectEnable = true;
			}
		} else {
			if (effectEnable) {
				pSourceVoice->DisableEffect(0);
				effectEnable = false;
			}
		}
#endif

		// copy & protect the audio data in own memory area while playing it
		memcpy(&pAudioBuffers[currentBuffer * nAudAllocSegLen], pAudNextSound, nAudAllocSegLen);

		sAudioBuffer.AudioBytes = nAudAllocSegLen;
		sAudioBuffer.pAudioData = &pAudioBuffers[currentBuffer * nAudAllocSegLen];

		currentBuffer++;
		currentBuffer %= (nAudSegCount);

		HRESULT hr = pSourceVoice->SubmitSourceBuffer(&sAudioBuffer); // send buffer to queue
		assert(hr == S_OK);

		return 0;
	}

#ifdef REVERB_EFFECT
	int effectInit()
	{
		HRESULT hr = XAudio2CreateReverb(&pXAPO);
		if (FAILED(hr)) {
			return 1;
		}

		XAUDIO2_EFFECT_DESCRIPTOR descriptor;
		descriptor.InitialState = TRUE;
		descriptor.OutputChannels = 2;
		descriptor.pEffect = pXAPO;

		XAUDIO2_EFFECT_CHAIN chain;
		chain.EffectCount = 1;
		chain.pEffectDescriptors = &descriptor;

		hr = pSourceVoice->SetEffectChain(&chain);
		if (FAILED(hr)) {
			return 1;
		}

		if (nAudDSPModule & 2) {
			pSourceVoice->EnableEffect(0);
			effectEnable = true;
		} else {
			pSourceVoice->DisableEffect(0);
			effectEnable = false;
		}

		return 0;
	}

	void effectExit()
	{
		RELEASE(pXAPO);
	}
#endif

	void exitVoices()
	{
		if (pSourceVoice) {
			pSourceVoice->Stop(0);
			pSourceVoice->DestroyVoice();
			pSourceVoice = NULL;
		}
		if (pMasterVoice) {
			pMasterVoice->DestroyVoice();
			pMasterVoice = NULL;
		}
	}

	int exit()
	{
		DspExit();

		// Cleanup XAudio2

#ifdef REVERB_EFFECT
		effectExit();
#endif

		exitVoices();

		RELEASE(pXAudio2);

#if !defined (_XBOX)
		CoUninitialize();
#endif

		if (pAudNextSound) {
			free(pAudNextSound);
			pAudNextSound = NULL;
		}

		if (pAudioBuffers) {
			free(pAudioBuffers);
			pAudioBuffers = NULL;
		}

		XAudio2GetNextSound = NULL;

		return 0;
	}

	int initVoices()
	{
		unsigned int deviceCount = 0;
		if (!pXAudio2 || FAILED(pXAudio2->GetDeviceCount(&deviceCount))) {
			return 1;
		}

		if (xa2Device >= deviceCount) {
			xa2Device = 0;
		}

		// Create a mastering voice
		if (FAILED(pXAudio2->CreateMasteringVoice(&pMasterVoice, XAUDIO2_DEFAULT_CHANNELS, nAudSampleRate, 0, xa2Device, NULL))) {
			return 1;
		}

		// Make the format of the sound
		WAVEFORMATEX wfx;
		memset(&wfx, 0, sizeof(wfx));
		wfx.cbSize = sizeof(wfx);
		wfx.wFormatTag = WAVE_FORMAT_PCM;
		wfx.nChannels = 2;											// stereo
		wfx.nSamplesPerSec = nAudSampleRate;						// sample rate
		wfx.wBitsPerSample = 16;									// 16-bit
		wfx.nBlockAlign = wfx.wBitsPerSample * wfx.nChannels / 8;	// bytes per sample
		wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;

		// Create the source voice
		if (FAILED(pXAudio2->CreateSourceVoice(&pSourceVoice, &wfx, 0, XAUDIO2_DEFAULT_FREQ_RATIO, &voiceContext, NULL, NULL))) {
			return 1;
		}

		if (audStereoUpmixing) {
			// set up stereo upmixing
			XAUDIO2_DEVICE_DETAILS dd;
			memset(&dd, 0, sizeof(dd));
			pXAudio2->GetDeviceDetails(0, &dd);

			if (dd.OutputFormat.Format.nChannels >= 4) {
				float* matrix = (float*)malloc(sizeof(float) * 2 * dd.OutputFormat.Format.nChannels);
				bool matrixAvailable = true;

				switch (dd.OutputFormat.Format.nChannels) {
					case 4: // 4.0
			//Speaker \ Left Source           Right Source
			/*Front L*/	matrix[0] = 1.0000f;  matrix[1] = 0.0000f;
			/*Front R*/	matrix[2] = 0.0000f;  matrix[3] = 1.0000f;
			/*Back  L*/	matrix[4] = 1.0000f;  matrix[5] = 0.0000f;
			/*Back  R*/	matrix[6] = 0.0000f;  matrix[7] = 1.0000f;
						break;
					case 5: // 5.0
			//Speaker \ Left Source           Right Source
			/*Front L*/	matrix[0] = 1.0000f;  matrix[1] = 0.0000f;
			/*Front R*/	matrix[2] = 0.0000f;  matrix[3] = 1.0000f;
			/*Front C*/	matrix[4] = 0.7071f;  matrix[5] = 0.7071f;
			/*Side  L*/	matrix[6] = 1.0000f;  matrix[7] = 0.0000f;
			/*Side  R*/	matrix[8] = 0.0000f;  matrix[9] = 1.0000f;
						break;
					case 6: // 5.1
			//Speaker \ Left Source           Right Source
			/*Front L*/	matrix[0] = 1.0000f;  matrix[1] = 0.0000f;
			/*Front R*/	matrix[2] = 0.0000f;  matrix[3] = 1.0000f;
			/*Front C*/	matrix[4] = 0.7071f;  matrix[5] = 0.7071f;
			/*LFE    */	matrix[6] = 0.0000f;  matrix[7] = 0.0000f;
			/*Side  L*/	matrix[8] = 1.0000f;  matrix[9] = 0.0000f;
			/*Side  R*/	matrix[10] = 0.0000f;  matrix[11] = 1.0000f;
						break;
					case 7: // 6.1
			//Speaker \ Left Source           Right Source
			/*Front L*/	matrix[0] = 1.0000f;  matrix[1] = 0.0000f;
			/*Front R*/	matrix[2] = 0.0000f;  matrix[3] = 1.0000f;
			/*Front C*/	matrix[4] = 0.7071f;  matrix[5] = 0.7071f;
			/*LFE    */	matrix[6] = 0.0000f;  matrix[7] = 0.0000f;
			/*Side  L*/	matrix[8] = 1.0000f;  matrix[9] = 0.0000f;
			/*Side  R*/	matrix[10] = 0.0000f;  matrix[11] = 1.0000f;
			/*Back  C*/	matrix[12] = 0.7071f;  matrix[13] = 0.7071f;
						break;
					case 8: // 7.1
			//Speaker \ Left Source           Right Source
			/*Front L*/	matrix[0] = 1.0000f;  matrix[1] = 0.0000f;
			/*Front R*/	matrix[2] = 0.0000f;  matrix[3] = 1.0000f;
			/*Front C*/	matrix[4] = 0.7071f;  matrix[5] = 0.7071f;
			/*LFE    */	matrix[6] = 0.0000f;  matrix[7] = 0.0000f;
			/*Back  L*/	matrix[8] = 1.0000f;  matrix[9] = 0.0000f;
			/*Back  R*/	matrix[10] = 0.0000f;  matrix[11] = 1.0000f;
			/*Side  L*/	matrix[12] = 1.0000f;  matrix[13] = 0.0000f;
			/*Side  R*/	matrix[14] = 0.0000f;  matrix[15] = 1.0000f;
						break;
					default:
						matrixAvailable = false;
						break;
				}

				if (matrixAvailable) {
					pSourceVoice->SetOutputMatrix(NULL, 2, dd.OutputFormat.Format.nChannels, matrix);
				}
				free(matrix);
			}
		}

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
		loopLen = (nAudSegLen * nAudSegCount) << 2;

		// Initialize XAudio2
		// OpenAL causes trouble when COINIT_MULTITHREADED is used
#if !defined (_XBOX)
		if (FAILED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED))) {
			return 1;
		}
#endif

		HRESULT hr;
		if (FAILED(hr = XAudio2Create(&pXAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR))) {
#if !defined (_XBOX)
			CoUninitialize();
#endif
			return 1;
		}

		if (initVoices()) {
			exit();
			return 1;
		}

		memset(&sAudioBuffer, 0, sizeof(sAudioBuffer));

		pAudNextSound = (short*)malloc(nAudAllocSegLen);		// The next sound block to put in the stream
		if (pAudNextSound == NULL) {
			exit();
			return 1;
		}

		// create own buffers to store sound data because it must not be
		// manipulated while the voice plays from it
		pAudioBuffers = (BYTE *)malloc(loopLen);
		if (pAudioBuffers == NULL) {
			exit();
			return 1;
		}
		currentBuffer = 0;

		set(NULL);

		DspInit();

#ifdef REVERB_EFFECT
		effectInit();
#endif

		return 0;
	}

	int play()
	{
		if (pSourceVoice == NULL) {
			return 1;
		}

		blank();
		pSourceVoice->SetVolume(volume);

		if (FAILED(pSourceVoice->Start(0))) {
			return 1;
		}
		return 0;
	}

	int stop()
	{
		if (!bAudOkay) {
			return 1;
		}

		if (pSourceVoice) {
			pSourceVoice->Stop(0);
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

		if (!pSourceVoice || FAILED(pSourceVoice->SetVolume(volume))) {
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

		TCHAR szString[MAX_PATH] = _T("");
		_sntprintf(szString, sizearray(szString), _T("Audio is delayed by approx. %ims"), int(100000.0 / (fps / (nAudSegCount - 1.0))));
		IntInfoAddStringModule(pInfo, szString);

		// get device info
		unsigned int deviceCount = 0;
		if (!pInfo || !pXAudio2 || FAILED(pXAudio2->GetDeviceCount(&deviceCount))) {
			return 1;
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

		XAUDIO2_DEVICE_DETAILS deviceDetails;
		for (unsigned int i = 0; i < pInfo->deviceNum; i++)
		{
		    pXAudio2->GetDeviceDetails(i, &deviceDetails);
		    if (deviceDetails.OutputFormat.Format.nChannels >= 2)
		    {
				// audio device supports at least two channels
				pInfo->deviceName[i] = (TCHAR*)malloc(256 * sizeof(TCHAR));
				if (!pInfo->deviceName[i]) {
					return 1;
				}
				_tcscpy(pInfo->deviceName[i], (char *)deviceDetails.DisplayName);
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

		exitVoices();

		free(pAudNextSound);
		pAudNextSound = NULL;

		free(pAudioBuffers);
		pAudioBuffers = NULL;

		fps = nAppVirtualFps;						// Update fps

		// Calculate the Seg Length and Loop length (round to nearest sample)
		nAudSegLen = (nAudSampleRate * 100 + (fps >> 1)) / fps;
		nAudAllocSegLen = nAudSegLen << 2;
		loopLen = (nAudSegLen * nAudSegCount) << 2;

		if (initVoices()) {
			exit();
			return 1;
		}

		memset(&sAudioBuffer, 0, sizeof(sAudioBuffer));

		pAudNextSound = (short*)malloc(nAudAllocSegLen);	// The next sound block to put in the stream
		if (pAudNextSound == NULL) {
			exit();
			return 1;
		}

		// create own buffers to store sound data because it must not be
		// manipulated while the voice plays from it
		pAudioBuffers = (BYTE *)malloc(loopLen);
		if (pAudioBuffers == NULL) {
			exit();
			return 1;
		}
		currentBuffer = 0;

		bAudOkay = true;									// This module was initted okay

		return 0;
	}

	AudioXaudio2() {
		pXAudio2 = NULL;
		pMasterVoice = NULL;
		pSourceVoice = NULL;
		pXAPO = NULL;
		effectEnable = false;

		pAudioBuffers = NULL;
		currentBuffer = 0;

		loopLen = 0;
		fps = 0;
		volume = 1.0f;
	}

	~AudioXaudio2() {
		exit();
	}
};

#endif
