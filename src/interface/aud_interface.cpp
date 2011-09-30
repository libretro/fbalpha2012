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
int AudWriteSilence(int)
{
	if (pAudNextSound)
		memset(pAudNextSound, 0, nAudAllocSegLen);
	return 0;
}

// AudioInterface
#if defined (_XBOX)
  #include "aud_xaudio2.cpp"
#elif defined (_WIN32)
  #include "aud_dsound.cpp"
 #ifndef NO_XAUDIO2
 #ifdef _MSC_VER
  #include "aud_xaudio2.cpp"
 #endif
 #endif

 #ifndef NO_OPENAL
  #include "aud_openal.cpp"
 #endif
#elif defined (BUILD_SDL)
  #include "aud_sdl.cpp"
#endif

AudioInterface audio;

const TCHAR* AudioInterface::driver_list()
{
	return
#if defined (_XBOX)
	_T("XAudio2;")
#elif defined (_WIN32)
	_T("DirectSound;")
 #ifndef NO_XAUDIO2
 #ifdef _MSC_VER
 	_T("XAudio2;")
 #endif
 #endif
 #ifndef NO_OPENAL
	_T("OpenAL;")
 #endif
#endif

#if defined (BUILD_SDL)
	_T("SDLAudio;")
#endif

  _T("None");
}

void AudioInterface::driver(const TCHAR* _driver)
{
	if (p) {
		term();
	}

	if (!_driver || !*_driver) {
		p = new Audio();
		return;
	}

#if defined (_XBOX)
	if (!_tcscmp(_driver, _T("XAudio2"))) {
		p = new AudioXaudio2();
	}
#elif defined (_WIN32)
	else if (!_tcscmp(_driver, _T("DirectSound"))) {
		p = new AudioDS();
	}

	#ifndef NO_XAUDIO2
	#ifdef _MSC_VER
	else if (!_tcscmp(_driver, _T("XAudio2"))) {
		p = new AudioXaudio2();
	}
	#endif
	#endif

	#ifndef NO_OPENAL
	else if (!_tcscmp(_driver, _T("OpenAL"))) {
		p = new AudioOpenAL();
	}
	#endif

#elif defined (BUILD_SDL)
	else if (!_tcscmp(_driver, _T("SDLAudio"))) {
		p = new AudioSDL();
	}
#endif

	else
		p = new Audio();
}

void AudioInterface::term()
{
	if (p)
	{
		delete p;
		p = 0;
	}
}

int AudioInterface::blank()
{
	if (!bAudOkay)
		return 1;

	return p ? p->blank() : 1;
}

// This function checks the Sound loop, and if necessary gets some more sound
int AudioInterface::check()
{
	return p ? p->check() : 1;
}

int AudioInterface::init()
{
	int nRet;

	_tcscpy(audActive, audSelect);

	if (!p)
		driver(audActive);

	if ((nRet = p->init()) == 0)
		bAudOkay = true;

	return nRet;
}

int AudioInterface::set(int (*callback)(int))
{
	return p ? p->set(callback) : 1;
}

int AudioInterface::play()
{
	//this will always be false with the current code - so why bother?
	if (!p) {
		return 1;
	}

	int ret = p->play();
	//no check needed here - play function always returns 0
	if (!ret)
		bAudPlaying = true;
	return ret;
}

int AudioInterface::stop()
{
	bAudPlaying = false;
	return p ? p->stop() : 1;
}

int AudioInterface::exit()
{
	IntInfoFree(&AudInfo);

	bAudOkay = false;

	int ret = 0;
	if (p)
	{
		ret = p->exit();
		term();
	}
	return ret;
}

int AudioInterface::setvolume(int vol)
{
	return p ? p->setvolume(vol) : 1;
}

int AudioInterface::setfps()
{
	return p ? p->setfps() : 1;
}

const TCHAR* AudioInterface::getName()
{
	const TCHAR* pszName = NULL;

	if (bAudOkay)
		pszName = audActive;
	else
		pszName = audSelect;

	if (pszName)
		return pszName;

	return FBALoadStringEx(1);
}

void AudioInterface::setdevice(int device)
{
#if defined (_WIN32)
	if (!strcmp(audSelect, "DirectSound")) {
		dsDevice = device;
	}

	#ifndef NO_XAUDIO2
	#ifdef _MSC_VER
	else if (!strcmp(audSelect, "XAudio2")) {
		xa2Device = device;
	}
	#endif
	#endif

	#ifndef NO_OPENAL
	else if (!strcmp(audSelect, "OpenAL"))
		oalDevice = device;
	#endif
#endif
}

int AudioInterface::getdevice(const TCHAR* _driver)
{
	TCHAR* driver = audSelect;
	if (_driver)
		driver = (TCHAR*)_driver;

#if defined (_WIN32)
	if (!strcmp(driver, "DirectSound"))
		return dsDevice;

	#ifndef NO_XAUDIO2
	#ifdef _MSC_VER
	else if (!strcmp(driver, "XAudio2"))
		return xa2Device;
	#endif
	#endif

	#ifndef NO_OPENAL
	#else if (!strcmp(driver, "OpenAL"))
		return oalDevice;
	#endif
#endif

	return 0;
}

InterfaceInfo* AudioInterface::get()
{
	if (IntInfoInit(&AudInfo)) {
		IntInfoFree(&AudInfo);
		return NULL;
	}

	if (bAudOkay) {
		char szString[MAX_PATH] = _T("");

		AudInfo.pszModuleName = getName();

		_sntprintf(szString, sizearray(szString), _T("Using audio device %d"), getdevice());
		IntInfoAddStringInterface(&AudInfo, szString);

		_sntprintf(szString, sizearray(szString), _T("Playback at %iHz, %i%% volume"), nAudSampleRate, nAudVolume / 100);
		IntInfoAddStringInterface(&AudInfo, szString);

		if (nAudDSPModule & 1)
			IntInfoAddStringInterface(&AudInfo, "Applying low-pass filter");
		if (nAudDSPModule & 2)
			IntInfoAddStringInterface(&AudInfo, "Applying reverb filter");

	 	if (p)
			p->get(&AudInfo);
	}
	else
		IntInfoAddStringInterface(&AudInfo, "Audio plugin not initialised");

	return &AudInfo;
}

int AudioInterface::select(const TCHAR* _driver)
{
	if (!_driver || !*_driver)
		return 1;

	strcpy(audSelect, _driver);
	return 0;
}
