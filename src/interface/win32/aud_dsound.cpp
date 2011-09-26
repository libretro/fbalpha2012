// DirectSound module
//#include "burner.h"
#include "aud_dsp.h"
#include <math.h>
#define DIRECTSOUND_VERSION  0x0800			// Only need version from DirectX 3
#include <dsound.h>

#ifdef _MSC_VER
#pragma comment(lib, "dsound")
#endif

// ==> get dound device, added by regret
static unsigned int dsoundDeviceCount = 0;
static unsigned int dsoundDevIndex = 0;
static GUID dsoundGUID;

static BOOL CALLBACK DSCountEnumProc(LPGUID, LPCTSTR, LPCTSTR, LPVOID)
{
	dsoundDeviceCount++;
	return TRUE;
}

static BOOL CALLBACK DSEnumDevProc(LPGUID lpGUID, LPCTSTR lpszDesc, LPCTSTR lpszDrvName, LPVOID)
{
	if (dsoundDevIndex == dsDevice) {
		memcpy(&dsoundGUID, lpGUID, sizeof(GUID));
		return FALSE;
	}
	dsoundDevIndex++;

	return TRUE;
}

static BOOL CALLBACK DSEnumProc(LPGUID lpGUID, LPCTSTR lpszDesc, LPCTSTR lpszDrvName, LPVOID lpContext)
{
	InterfaceInfo* pInfo = (InterfaceInfo*)lpContext;
	if (!pInfo) {
		return FALSE;
	}

	pInfo->deviceName[dsoundDevIndex] = (TCHAR*)malloc(256 * sizeof(TCHAR));
	if (!pInfo->deviceName[dsoundDevIndex]) {
		return TRUE;
	}
	_tcscpy(pInfo->deviceName[dsoundDevIndex], lpszDesc);
	dsoundDevIndex++;

	return TRUE;
}
// <== get dound device

class AudioDS : public Audio {
public:
	// Sound is split into a series of 'segs', one seg for each frame
	// The Loop buffer is a multiple of this seg length.

	IDirectSound8* pDS;				// DirectSound interface
	IDirectSoundBuffer* pdsbPrim ;	// Primary buffer
	IDirectSoundBuffer* pdsbLoop ;	// (Secondary) Loop buffer

	int (*DSoundGetNextSound)(int);	// Callback used to request more sound

	int set(int (*callback)(int))
	{
		if (callback == NULL) {
			DSoundGetNextSound = AudWriteSlience;
		} else {
			DSoundGetNextSound = callback;
		}
		return 0;
	}

	int blank()
	{
		if (pdsbLoop) {
			void *pData = NULL, *pData2 = NULL;
			DWORD cbLen = 0, cbLen2 = 0;

			// Lock the Loop buffer
			if (FAILED(pdsbLoop->Lock(0, loopLen, &pData, &cbLen, &pData2, &cbLen2, 0))) {
				return 1;
			}
			memset(pData, 0, cbLen);

			// Unlock (2nd 0 is because we wrote nothing to second part)
			pdsbLoop->Unlock(pData, cbLen, pData2, 0);
		}

		// Also blank the nAudNextSound buffer
		if (pAudNextSound) {
			AudWriteSlience();
		}

		return 0;
	}

	// This function checks the DSound loop, and if necessary does a callback to update the emulation
	int check()
	{
		#define WRAP_INC(x) { x++; if (x >= nAudSegCount) x = 0; }

		static int nDSoundNextSeg = 0;			// We have filled the sound in the loop up to the beginning of 'nNextSeg'

		if (pdsbLoop == NULL) {
			return 1;
		}

		int nPlaySeg = 0, nFollowingSeg = 0;
		DWORD nPlay = 0, nWrite = 0;

		// We should do nothing until nPlay has left nDSoundNextSeg
		pdsbLoop->GetCurrentPosition(&nPlay, &nWrite);

		nPlaySeg = nPlay / nAudAllocSegLen;

		if (nPlaySeg >= nAudSegCount) {
			nPlaySeg = nAudSegCount - 1;
		}
		if (nPlaySeg < 0) {						// important to ensure nPlaySeg clipped for below
			nPlaySeg = 0;
		}

		if (nDSoundNextSeg == nPlaySeg) {
			Sleep(2);							// Don't need to do anything for a bit
			return 0;
		}

		// work out which seg we will fill next
		nFollowingSeg = nDSoundNextSeg;
		WRAP_INC(nFollowingSeg);

		while (nDSoundNextSeg != nPlaySeg) {
			void *pData = NULL, *pData2 = NULL;
			DWORD cbLen = 0, cbLen2 = 0;
			int bDraw;

			// fill nNextSeg

			// Lock the relevant seg of the loop buffer
			if (SUCCEEDED(pdsbLoop->Lock(nDSoundNextSeg * nAudAllocSegLen, nAudAllocSegLen, &pData, &cbLen, &pData2, &cbLen2, 0))) {
				// Locked the segment, so write the sound we calculated last time
				memcpy(pData, pAudNextSound, nAudAllocSegLen);

				// Unlock (2nd 0 is because we wrote nothing to second part)
				pdsbLoop->Unlock(pData, cbLen, pData2, 0);
			}

			// If this is the last seg of sound, flag bDraw (to draw the graphics)
			bDraw = (nFollowingSeg == nPlaySeg) || !autoFrameSkip;

			DSoundGetNextSound(bDraw);			// get more sound into nAudNextSound

			if (nAudDSPModule & 1) {
				if (bRunPause)
					AudWriteSlience();
				else
					DspDo(pAudNextSound, nAudSegLen);
			}

			nDSoundNextSeg = nFollowingSeg;
			WRAP_INC(nFollowingSeg);
		}

		return 0;
	}

	int exit()
	{
		DspExit();

		if (pAudNextSound) {
			free(pAudNextSound);
			pAudNextSound = NULL;
		}

		DSoundGetNextSound = NULL;

		// Release the (Secondary) Loop Sound Buffer
		RELEASE(pdsbLoop);
		// Release the Primary Sound Buffer
		RELEASE(pdsbPrim);
		// Release the DirectSound interface
		RELEASE(pDS);

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

		// check device
		dsoundDeviceCount = 0;
		if (FAILED(DirectSoundEnumerate((LPDSENUMCALLBACK)DSCountEnumProc, NULL))
			|| dsoundDeviceCount <= 0) {
			return 1;
	    }
		if (dsDevice >= dsoundDeviceCount) {
			dsDevice = 0;
		}

		dsoundDevIndex = 0;
		if (dsDevice != 0 && FAILED(DirectSoundEnumerate((LPDSENUMCALLBACK)DSEnumDevProc, NULL))) {
			return 1;
	    }

		// Create the DirectSound interface
		if (dsDevice == 0) {
			if (FAILED(DirectSoundCreate8(NULL, &pDS, NULL))) {
				return 1;
			}
		} else {
			if (FAILED(DirectSoundCreate8(&dsoundGUID, &pDS, NULL))) {
				return 1;
			}
		}

		// Set the coop level
		HRESULT hr = pDS->SetCooperativeLevel(hScrnWnd, DSSCL_PRIORITY);
		if (FAILED(hr)) {
			exit();
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

		// Make the primary sound buffer
		DSBUFFERDESC dsbd;
		memset(&dsbd, 0, sizeof(dsbd));
		dsbd.dwSize = sizeof(dsbd);
		dsbd.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_GETCURRENTPOSITION2;
		if (FAILED(pDS->CreateSoundBuffer(&dsbd, &pdsbPrim, NULL))) {
			exit();
			return 1;
		}

		{
			// Set the format of the primary sound buffer (not critical if it fails)
			if (nAudSampleRate < 44100) {
				wfx.nSamplesPerSec = 44100;
			}
			pdsbPrim->SetFormat(&wfx);

			wfx.nSamplesPerSec = nAudSampleRate;
		}

		// Make the loop sound buffer
		memset(&dsbd, 0, sizeof(dsbd));
		dsbd.dwSize = sizeof(dsbd);
		// A standard secondary buffer (accurate position, plays in the background, and can notify).
		dsbd.dwFlags = DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_CTRLVOLUME;
		dsbd.dwBufferBytes = loopLen;
		dsbd.lpwfxFormat = &wfx;								// Same format as the primary buffer
		if (FAILED(pDS->CreateSoundBuffer(&dsbd, &pdsbLoop, NULL))) {
			exit();
			return 1;
		}

		pAudNextSound = (short*)malloc(nAudAllocSegLen);		// The next sound block to put in the stream
		if (pAudNextSound == NULL) {
			exit();
			return 1;
		}

		set(NULL);

		DspInit();

		return 0;
	}

	int play()
	{
		if (pdsbLoop == NULL) {
			return 1;
		}

		blank();
		pdsbLoop->SetVolume(volume);

		// Play the looping buffer
		if (FAILED(pdsbLoop->Play(0, 0, DSBPLAY_LOOPING))) {
			return 1;
		}
		return 0;
	}

	int stop()
	{
		if (!bAudOkay || !pdsbLoop) {
			return 1;
		}

		// Stop the looping buffer
		pdsbLoop->Stop();
		return 0;
	}

	int setvolume(int vol)
	{
		if (vol == 10000) {
			volume = DSBVOLUME_MAX;
		} else {
			if (vol == 0) {
				volume = DSBVOLUME_MIN;
			} else {
				volume = DSBVOLUME_MAX - (long)(10000.0 * pow(10.0, vol / -5000.0)) + 100;
			}
		}

		if (volume < DSBVOLUME_MIN) {
			volume = DSBVOLUME_MIN;
		}

		if (!pdsbLoop || FAILED(pdsbLoop->SetVolume(volume))) {
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
		dsoundDeviceCount = 0;
		if (!pInfo || FAILED(DirectSoundEnumerate((LPDSENUMCALLBACK)DSCountEnumProc, NULL))) {
			return 1;
	    }
		if (dsoundDeviceCount <= 0) {
			return 1;
		}

		pInfo->deviceNum = dsoundDeviceCount;
		pInfo->deviceName = (TCHAR**)malloc((pInfo->deviceNum) * sizeof(TCHAR*));
		if (!pInfo->deviceName) {
			return 1;
		}
		memset(pInfo->deviceName, 0, pInfo->deviceNum * sizeof(TCHAR*));

		dsoundDevIndex = 0;
		if (FAILED(DirectSoundEnumerate((LPDSENUMCALLBACK)DSEnumProc, (VOID*)pInfo))) {
			return 1;
	    }

		return 0;
	}

	int setfps()
	{
		if (nAudSampleRate <= 0) {
			return 0;
		}

		bAudOkay = false;									// This module is no longer okay

		free(pAudNextSound);
		pAudNextSound = NULL;

		// Release the (Secondary) Loop Sound Buffer
		RELEASE(pdsbLoop);

		// Update fps
		fps = nAppVirtualFps;

		// Calculate the Seg Length and Loop length (round to nearest sample)
		nAudSegLen = (nAudSampleRate * 100 + (fps >> 1)) / fps;
		nAudAllocSegLen = nAudSegLen << 2;
		loopLen = (nAudSegLen * nAudSegCount) << 2;

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

		// Make the loop sound buffer
		DSBUFFERDESC dsbd;
		memset(&dsbd, 0, sizeof(dsbd));
		dsbd.dwSize = sizeof(dsbd);
		// A standard secondary buffer (accurate position, plays in the background, and can notify).
		dsbd.dwFlags = DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_CTRLVOLUME;
		dsbd.dwBufferBytes = loopLen;
		dsbd.lpwfxFormat = &wfx;							// Same format as the primary buffer
		if (FAILED(pDS->CreateSoundBuffer(&dsbd, &pdsbLoop, NULL))) {
			return 1;
		}

		pAudNextSound = (short*)malloc(nAudAllocSegLen);	// The next sound block to put in the stream
		if (pAudNextSound == NULL) {
			return 1;
		}

		bAudOkay = true;									// This module was initted okay

		return 0;
	}

	AudioDS() {
		pDS = NULL;
		pdsbPrim = NULL;
		pdsbLoop = NULL;

		loopLen = 0;
		fps;
		volume = 0;
	}

	~AudioDS() {
		exit();
	}
};
