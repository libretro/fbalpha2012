#ifndef INTERFACE_H
#define INTERFACE_H

#ifdef FBA_DEBUG
 #define PRINT_DEBUG_INFO
#endif

// GameInp structure
#include "gameinp.h"

// Interface
class Audio;

// Interface info (used for all modules)
struct InterfaceInfo {
	const TCHAR* pszModuleName;
	TCHAR** ppszInterfaceSettings;
	TCHAR** ppszModuleSettings;

	// device info, added by regret
	unsigned int deviceNum;
	TCHAR** deviceName;
};

int IntInfoFree(InterfaceInfo* pInfo);
int IntInfoInit(InterfaceInfo* pInfo);
int IntInfoAddStringInterface(InterfaceInfo* pInfo, TCHAR* szString);
int IntInfoAddStringModule(InterfaceInfo* pInfo, TCHAR* szString);

// Input plugin:
struct InputInOut {
	// Read digital
	int (*ReadSwitch)(int nCode);
	// Read analog
	int (*ReadJoyAxis)(int i, int nAxis);
};

int InputInit();
int InputExit();
void InputMake(void);

extern bool bInputOkay;
extern unsigned int nInputSelect;

// Audio Output plugin
int AudWriteSlience(int draw = 0);			// Write silence into the buffer

class AudioInterface
{
	public:
	int nAudVolume;					// Sound volume (% * 100)
	bool bAudPlaying;				// True if the Loop buffer is playing

	const TCHAR* driver_list();
	void driver(const TCHAR* driver = _T(""));

	int init();
	int exit();
	int play();
	int stop();
	int blank();
	void check();
	int setvolume(int vol);
	int set(int (*callback)(int));
	int setfps();

	void term();
	int select(const TCHAR* driver);
	InterfaceInfo* get();
	const TCHAR* getName();
	void setdevice(int device);
	int getdevice(const TCHAR* driver = NULL);

	AudioInterface() {
		bAudPlaying = false;
		nAudVolume = 10000;
	}
	~AudioInterface() {
		term();
	}

	private:
	int p;
};

extern AudioInterface audio;	// audio interface instance

extern int nAudSampleRate;		// sample rate
extern int nAudSegCount;		// Segs in the pdsbLoop buffer
extern int nAudSegLen;			// Seg length in samples (calculated from Rate/Fps)
extern int nAudAllocSegLen;		// Allocated seg length in samples
extern short* pAudNextSound;	// The next sound seg we will add to the sample loop
extern bool bAudOkay;			// True if DSound was initted okay
extern int nAudDSPModule;		// DSP module to use: 0 = none, 1 = low-pass filter, 2 = reverb
extern int audStereoUpmixing;	// Enable stereo upmixing (XAudio2 only)
extern TCHAR audSelect[16];		// Which audio plugin is selected
extern int dsDevice;			// dsound device
extern int oalDevice;			// openal device
extern int xa2Device;			// xaudio2 device
extern int cellAudioDevice;		// ps3 cell audio device

// Video Output plugin:
struct VidOut {
	int (*Init)();
	int (*Exit)();
	// Get plugin info
	int (*GetSetting)(InterfaceInfo* pInfo);
};

enum VID_OUT {
	VID_PSGL,
};

int VidSelect(unsigned int driver);
int VidInit();
int VidExit();
int VidFrame();
int VidRedraw();
int VidRecalcPal();
int VidPaint(int bValidate);
int VidReinit();
int VidResize(int nWidth, int nHeight);
int VidScale(RECT* pRect, int nGameWidth, int nGameHeight);
const TCHAR* VidGetName();
InterfaceInfo* VidGetInfo();
const TCHAR* VidDriverName(unsigned int driver);

void VidSwitchFilter(int nEffect);

extern bool bVidOkay;
extern unsigned int nVidSelect;
extern int nVidDepth;
extern int nVidRefresh;
extern int nVidFullscreen;
extern int bVidCorrectAspect;
extern int nVidRotationAdjust;
extern int bVidForce16bit;
extern int bVidFullStretch;
extern int bVidTripleBuffer;
extern int bVidVSync;
extern int nVidScrnWidth, nVidScrnHeight;
extern int nVidScrnDepth;

extern unsigned int vidFilterLinear;
extern unsigned int vidHardwareVertex;
extern unsigned int vidMotionBlur;
extern unsigned int vidUseFilter;
extern unsigned int vidForceFilterSize;
extern unsigned int nVid3DProjection;
extern float fVidScreenAngle;
extern float fVidScreenCurvature;
extern int nVidDXTextureManager;

extern int nVidScrnAspectX, nVidScrnAspectY;
extern float vidScrnAspect;
extern bool autoVidScrnAspect;

extern unsigned char* pVidImage;
extern int nVidImageWidth, nVidImageHeight;
extern int nVidImageLeft, nVidImageTop;
extern int nVidImagePitch, nVidImageBPP;
extern int nVidImageDepth;

extern int nVidFilter; // filter index
extern unsigned int nVidAdapter; // Which adapter device is selected

extern int shaderindex;

extern "C" unsigned int (__cdecl *VidHighCol) (int r, int g, int b, int i);

// vid_directx_support.cpp
#if 0
int VidSNewShortMsg(const TCHAR* pText, int nRGB = 0, int nDuration = 0, int nPiority = 5);
void VidSKillShortMsg();
int VidSNewTinyMsg(const TCHAR* pText, int nRGB = 0, int nDuration = 0, int nPiority = 5);
void VidSKillTinyMsg();
#endif

#endif
