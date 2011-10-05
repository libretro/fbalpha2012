#ifdef FBA_DEBUG
 #define PRINT_DEBUG_INFO
#endif

// GameInp structure
#include "gameinp.h"
// Key codes
#include "inp_keys.h"

// Interface info (used for all modules)
struct InterfaceInfo {
	const TCHAR* pszModuleName;
	TCHAR** ppszInterfaceSettings;
	TCHAR** ppszModuleSettings;
};

int IntInfoFree(InterfaceInfo* pInfo);
int IntInfoInit(InterfaceInfo* pInfo);
int IntInfoAddStringInterface(InterfaceInfo* pInfo, TCHAR* szString);
int IntInfoAddStringModule(InterfaceInfo* pInfo, TCHAR* szString);

// Input plugin:
struct InputInOut {
	int   (*Init)();
	int   (*Exit)();
	int   (*SetCooperativeLevel)(bool bExclusive, bool bForeground);
	// Setup new frame
	int   (*NewFrame)();
	// Read digital
	int   (*ReadSwitch)(int nCode);
	// Read analog
	int   (*ReadJoyAxis)(int i, int nAxis);
	int   (*ReadMouseAxis)(int i, int nAxis);
	// Find out which control is activated
	int   (*Find)(bool CreateBaseline);
	// Get the name of a control
	int   (*GetControlName)(int nCode, TCHAR* pszDeviceName, TCHAR* pszControlName);
	// Get plugin info
	int   (*GetPluginSettings)(InterfaceInfo* pInfo);
	const TCHAR* szModuleName;
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

// Profiling plugin
struct ProfileDo {
	int    (*ProfileExit)();
	int    (*ProfileInit)();
	int    (*ProfileStart)(int nSubSystem);
	int    (*ProfileEnd)(int nSubSystem);
	double (*ProfileReadLast)(int nSubSystem);
	double (*ProfileReadAverage)(int nSubSystem);
	// Get plugin info
	int    (*GetPluginSettings)(InterfaceInfo* pInfo);
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

// Audio Output plugin
struct AudOut {
	int   (*BlankSound)();
	int   (*SoundCheck)();
	int   (*SoundInit)();
	int   (*SetCallback)(int (*pCallback)(int));
	int   (*SoundPlay)();
	int   (*SoundStop)();
	int   (*SoundExit)();
	int   (*SoundSetVolume)();
	// Get plugin info
	int   (*GetPluginSettings)(InterfaceInfo* pInfo);
	const TCHAR* szModuleName;
};

int AudSelect(unsigned int nPlugIn);
int AudSoundInit();
int AudSoundExit();
int AudSetCallback(int (*pCallback)(int));
int AudSoundPlay();
int AudSoundStop();
int AudBlankSound();
int AudSoundCheck();
int AudSoundSetVolume();
InterfaceInfo* AudGetInfo();
void AudWriteSilence();

extern int nAudSampleRate[8];          // sample rate
extern int nAudVolume;				// Sound volume (% * 100)
extern int nAudSegCount;          	// Segs in the pdsbLoop buffer
extern int nAudSegLen;            	// Seg length in samples (calculated from Rate/Fps)
extern int nAudAllocSegLen;
extern short *nAudNextSound;       	// The next sound seg we will add to the sample loop
extern unsigned char bAudOkay;    	// True if DSound was initted okay
extern unsigned char bAudPlaying;	// True if the Loop buffer is playing
extern int nAudDSPModule[8];			// DSP module to use: 0 = none, 1 = low-pass filter
extern unsigned int nAudSelect;

// Video Output plugin:
struct VidOut {
	int   (*Init)();
	int   (*Exit)();
	int   (*Frame)(bool bRedraw);
	int   (*Paint)(int bValidate);
	int   (*ImageSize)(RECT* pRect, int nGameWidth, int nGameHeight);
	// Get plugin info
	int   (*GetPluginSettings)(InterfaceInfo* pInfo);
	const TCHAR* szModuleName;
};

int VidSelect(unsigned int nPlugin);
int VidInit();
int VidExit();
int VidReInitialise();
int VidFrame();
int VidRedraw();
int VidRecalcPal();
int VidPaint(int bValidate);
int VidImageSize(RECT* pRect, int nGameWidth, int nGameHeight);
const TCHAR* VidGetModuleName();
InterfaceInfo* VidGetInfo();

#ifdef BUILD_WIN32
 extern HWND hVidWnd;
#endif

extern bool bVidOkay;
extern unsigned int nVidSelect;
extern int nVidWidth, nVidHeight, nVidDepth, nVidRefresh;

extern int nVidHorWidth, nVidHorHeight;
extern int nVidVerWidth, nVidVerHeight;

extern int nVidFullscreen;
extern int bVidBilinear;
extern int bVidScanlines;
extern int bVidScanRotate;
extern int bVidScanBilinear;
extern int nVidScanIntensity;
extern int bVidScanHalf;
extern int bVidScanDelay;
extern int nVidFeedbackIntensity;
extern int nVidFeedbackOverSaturation;
extern int bVidCorrectAspect;
extern int bVidArcaderes;

extern int bVidArcaderesHor;
extern int bVidArcaderesVer;

extern int nVidRotationAdjust;
extern int bVidUseHardwareGamma;
extern int bVidAutoSwitchFull;
extern int bVidForce16bit;
extern int nVidTransferMethod;
extern float fVidScreenAngle;
extern float fVidScreenCurvature;
extern long long nVidBlitterOpt[];
extern int bVidFullStretch;
extern int bVidTripleBuffer;
extern int bVidVSync;
extern double dVidCubicB;
extern double dVidCubicC;
extern int nVidScrnWidth, nVidScrnHeight;
extern int nVidScrnDepth;

extern int nVidScrnAspectX, nVidScrnAspectY;

extern unsigned char* pVidImage;
extern int nVidImageWidth, nVidImageHeight;
extern int nVidImageLeft, nVidImageTop;
extern int nVidImagePitch, nVidImageBPP;
extern int nVidImageDepth;

extern "C" unsigned int (__cdecl *VidHighCol) (int r, int g, int b, int i);

extern bool bVidUsePlaceholder;
extern TCHAR szPlaceHolder[MAX_PATH];

// vid_directx_support.cpp

int VidSNewTinyMsg(const TCHAR* pText, int nRGB = 0, int nDuration = 0, int nPiority = 5);

int VidSNewShortMsg(const TCHAR* pText, int nRGB = 0, int nDuration = 0, int nPiority = 5);
void VidSKillShortMsg();

int VidSAddChatMsg(const TCHAR* pID, int nIDRGB, const TCHAR* pMain, int nMainRGB);

#define MAX_CHAT_SIZE (128)

extern int nVidSDisplayStatus;
extern int nMaxChatFontSize;
extern int nMinChatFontSize;
extern bool bEditActive;
extern bool bEditTextChanged;
extern TCHAR EditText[MAX_CHAT_SIZE + 1];

// osd text display for dx9
extern TCHAR OSDMsg[MAX_PATH];
extern unsigned int nOSDTimer;
void VidSKillOSDMsg();
