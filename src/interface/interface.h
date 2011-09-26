#ifndef INTERFACE_H
#define INTERFACE_H

#ifdef FBA_DEBUG
 #define PRINT_DEBUG_INFO
#endif

// GameInp structure
#include "gameinp.h"

// Interface
#ifndef SN_TARGET_PS3
#include "aud_audio.h"
#else
class Audio;
#endif

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
	int (*Init)();
	int (*Exit)();
	int (*SetCooperativeLevel)(bool bExclusive, bool bForeground);
	// Setup new frame
	int (*NewFrame)();
	// Read digital
	int (*ReadSwitch)(int nCode);
	// Read analog
	int (*ReadJoyAxis)(int i, int nAxis);
	int (*ReadMouseAxis)(int i, int nAxis);
	// Find out which control is activated
	int (*Find)(bool CreateBaseline);
	// Get the name of a control
	int (*GetControlName)(int nCode, TCHAR* pszDeviceName, TCHAR* pszControlName);
	// Get plugin info
	int (*GetSetting)(InterfaceInfo* pInfo);
};

int InputInit();
int InputExit();
int InputSetCooperativeLevel(const bool bExclusive, const bool bForeGround);
int InputMake(bool bCopy);
int InputFind(const int nFlags);
int InputGetControlName(int nCode, TCHAR* pszDeviceName, TCHAR* pszControlName);
InterfaceInfo* InputGetInfo();

extern bool bInputOkay;
extern unsigned int nInputSelect;

#ifdef ENABLE_PROFILING
// Profiling plugin
struct ProfileDo {
	int    (*ProfileExit)();
	int    (*ProfileInit)();
	int    (*ProfileStart)(int nSubSystem);
	int    (*ProfileEnd)(int nSubSystem);
	double (*ProfileReadLast)(int nSubSystem);
	double (*ProfileReadAverage)(int nSubSystem);
	// Get plugin info
	int    (*GetSetting)(InterfaceInfo* pInfo);
	const  TCHAR* szModuleName;
};

extern bool bProfileOkay;
extern unsigned int nProfileSelect;

int ProfileInit();
int ProfileExit();
int ProfileProfileStart(int nSubSystem);
int ProfileProfileEnd(int nSubSustem);
double ProfileProfileReadLast(int nSubSustem);
double ProfileProfileReadAverage(int nSubSustem);
InterfaceInfo* ProfileGetInfo();
#endif

// Audio Output plugin
int AudWriteSlience(int draw = 0);			// Write silence into the buffer

class AudioInterface {
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
	int check();
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
#ifdef SN_TARGET_PS3
   int p;
#else
   Audio* p;
#endif
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
#ifndef SN_TARGET_PS3
	int (*Frame)(bool bRedraw);
	int (*Paint)(int bValidate);
	int (*Scale)(RECT* pRect, int nWidth, int nHeight);
#endif
	// Get plugin info
	int (*GetSetting)(InterfaceInfo* pInfo);
};

enum VID_OUT {
#if defined (_WIN32)
	VID_DDRAW = 0,
	VID_D3D7,
	VID_D3D,
	#ifndef NO_OPENGL
	VID_WGL,
	#endif
#elif defined (BUILD_SDL)
	VID_SDLFX,
#elif defined (SN_TARGET_PS3)
	VID_PSGL,
#endif
	VID_LEN
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

#ifdef _WIN32
 extern HWND hVidWnd;
#endif

extern bool bVidOkay;
extern unsigned int nVidSelect;
#ifndef SN_TARGET_PS3
extern int nVidWidth;
extern int nVidHeight;
#endif
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
int VidSNewShortMsg(const TCHAR* pText, int nRGB = 0, int nDuration = 0, int nPiority = 5);
void VidSKillShortMsg();
int VidSNewTinyMsg(const TCHAR* pText, int nRGB = 0, int nDuration = 0, int nPiority = 5);
void VidSKillTinyMsg();

extern int nMaxChatFontSize;
extern int nMinChatFontSize;

#endif
