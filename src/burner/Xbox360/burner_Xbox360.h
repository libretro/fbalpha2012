#ifndef BURNER_WIN32_H
#define BURNER_WIN32_H

#define _WIN32_WINNT 0x0500
#define _WIN32_IE 0x0500

#ifdef _UNICODE
 #define UNICODE
#endif

#define WIN32_LEAN_AND_MEAN
#define OEMRESOURCE
#include <xtl.h>
#include "main.h"

// Additions to the Cygwin/MinGW win32 headers
#ifdef __GNUC__
 #include "mingw_win32.h"
#endif
 
// ---------------------------------------------------------------------------
// use STL
#include <string>
#include <list>
#include <set>
#include <map>
#include <string>
#include <vector>
#include <hash_map>
#include <fstream>
#include <stdio.h>

using std::string;
using std::list;
using std::set;
using std::map;
using std::vector;
using std::multimap;
using std::ofstream;
#if 0 //ndef NO_STLPORT
using std::hash_map;
#else
using stdext::hash_map;
#endif

typedef std::basic_string<TCHAR> tstring;

#ifndef MAX_PATH
 #define MAX_PATH (260)
#endif

// ---------------------------------------------------------------------------

// Macro for releasing a COM object
#define RELEASE(x) { if ((x)) (x)->Release(); (x) = NULL; }

#define IDS_ERR_UNKNOWN			"Unknown"

#define KEY_DOWN(Code) ((GetAsyncKeyState(Code) & 0x8000) ? 1 : 0)

// Macro used for re-initialiging video/sound/input
//#define POST_INITIALISE_MESSAGE { dprintf(_T("*** (re-) initialising - %s %i\n"), _T(__FILE__), __LINE__); PostMessage(NULL, WM_APP + 0, 0, 0); }
#define POST_INITIALISE_MESSAGE PostMessage(NULL, WM_APP + 0, 0, 0)

// ---------------------------------------------------------------------------
// includes
#include "strconv.h"

// ---------------------------------------------------------------------------
// from burn
extern int bsavedecryptedcs;
extern int bsavedecryptedps;
extern int bsavedecrypteds1;
extern int bsavedecryptedvs;
extern int bsavedecryptedm1;
extern int bsavedecryptedxor;
extern int bsavedecryptedprom;

// main.cpp
extern HINSTANCE hAppInst;			// Application Instance
extern HANDLE hMainThread;			// Handle to the main thread
extern int nAppThreadPriority;
extern int nAppShowCmd;
extern int nDisableSplash;
extern int nLastFilter;
extern int nLastRom;
extern int nLastFilter;
extern int HideChildren;
extern int ThreeOrFourPlayerOnly;
extern int ArcadeJoystick;
extern CBurnApp app;
extern IDirect3DDevice9 *pDevice;
//extern HACCEL hAccel;

#define EXE_NAME_SIZE (32)
extern TCHAR szAppExeName[EXE_NAME_SIZE + 1];
extern TCHAR szAppBurnVer[16];

extern bool bCmdOptUsed;
extern bool bAlwaysProcessKey;

// Used for the load/save dialog in commdlg.h
extern TCHAR szChoice[MAX_PATH];	// File chosen by the user

int dprintf(TCHAR* pszFormat, ...);	// Use instead of printf() in the UI

void AppCleanup();
bool AppProcessKeyboardInput();

// popup_win32.cpp
enum FBAPopupType { MT_NONE = 0, MT_ERROR, MT_WARNING, MT_INFO };

#define PUF_TYPE_ERROR			(1)
#define PUF_TYPE_WARNING		(2)
#define PUF_TYPE_INFO			(3)
#define PUF_TYPE_LOGONLY		(8)
#define PUF_TEXT_TRANSLATE		(1 << 16)
#define PUF_TEXT_NO_TRANSLATE	(0)
#define PUF_TEXT_DEFAULT		(PUF_TEXT_TRANSLATE)

int FBAPopupDisplay(int nFlags);
int FBAPopupAddText(int nFlags, TCHAR* pszFormat, ...);
int FBAPopupDestroyText();

// systeminfo.cpp
LONG CALLBACK ExceptionFilter(_EXCEPTION_POINTERS* pExceptionInfo);
int SystemInfoCreate(HWND);

// splash.cpp
extern int nSplashTime;
int SplashCreate();
void SplashDestroy(bool bForce);

// about.cpp
int AboutCreate(HWND);
int FirstUsageCreate(HWND);

// media.cpp
int mediaInit();
int mediaExit();
int mediaChangeFps(int scale);
int mediaReInitAudio();
int mediaReInitScrn();

// misc_win32.cpp
void createNeedDir();
bool directoryExists(const TCHAR* dirname);
void pathCheck(TCHAR* path);
int directLoadGame(TCHAR* name);
bool createToolTip(int toolID, HWND hDlg, TCHAR* pText);
int appDirectory();
int getClientScreenRect(HWND hWnd, RECT* pRect);
int wndInMid(HWND hMid, HWND hBase);
void setWindowAspect();
char* decorateGameName(unsigned int drv);
char* decorateKailleraGameName(unsigned int drv);
int findRom(int i, struct ArcEntry* list, int count);

// drv.cpp
extern int bDrvOkay;				// 1 if the Driver has been initted okay, and it's okay to use the BurnDrv functions
int BurnerDrvInit(int nDrvNum, bool bRestore);
int DrvInitCallback();				// Used when Burn library needs to load a game. DrvInit(nBurnSelect, false)
int BurnerDrvExit();

// run.cpp
extern int nAppVirtualFps;			// virtual fps
extern int bRunPause;
extern int bAltPause;
extern int autoFrameSkip;
extern bool bShowFPS;
extern bool bAppDoFast;
extern bool bAppDoStep;
extern unsigned int nFastSpeed;
extern int nShowEffect;

int RunIdle();
int RunMessageLoop();
int RunInit();
int RunExit();
int RunReset();

// scrn.cpp
extern HWND hScrnWnd;				// Handle to the screen window
extern bool bShowOnTop;
extern bool bFullscreenOnStart;
extern HWND hVideoWnd;				// Handle to the video window

extern int bAutoPause;
extern int nWindowSize;
extern RECT SystemWorkArea;			// The full screen area
extern int nWindowPosX, nWindowPosY;

extern int nSavestateSlot;
//extern int nScrnVisibleOffset[4];

extern int nXOffset;
extern int nYOffset;
extern int nXScale;
extern int nYScale;

int scrnInit();
int scrnExit();
int scrnSize();
int scrnTitle();
int scrnSwitchFull();
int scrnFakeFullscreen();
int scrnSetFull(const bool& full);
void __cdecl scrnReinit();
void setPauseMode(bool bPause);
void setPauseModeScreen(bool bPause);

// sel.cpp
extern HWND hSelDlg;
extern int nLoadMenuShowX;
extern int nLoadDriverShowX;
extern int nTabSel;
extern TCHAR szUserFilterStr[MAX_PATH];
extern int nSystemSel;
int selDialog(HWND);				// Choose a Burn driver
void clearNodeInfo();
int driverConfigDialog(HWND);		// Driver config

// translist.cpp
extern TCHAR szTransGamelistFile[MAX_PATH];
int loadGamelist();
int createGamelist();
TCHAR* mangleGamename(const TCHAR* pszOldName, bool bRemoveArticle = true);
TCHAR* transGameName(const TCHAR* pszOldName, bool bRemoveArticle = true);

// favorites.cpp
int initFavorites();
int saveFavorites();
void addFavorite(unsigned int index);
void removeFavorite(unsigned int index);
bool filterFavorite(const unsigned int& index);

// config.cpp
extern int nIniVersion;
int configAppLoadXml();
int configAppSaveXml();

// conc.cpp
int configCheatLoad(const TCHAR* filename = NULL);
int configCheatReload(const TCHAR* filename = NULL);

// wave.cpp
extern bool soundLogStart;		// wave log start flag
int waveLogStart();
int waveLogStop();
void waveLogWrite();

// inpd.cpp
int InpdUpdate();
int InpdCreate();
int InpdListMake(int bBuild);
int loadDefaultInput();
int SaveDefaultInput();

// inpcheat.cpp
int InpCheatCreate();

#ifndef NO_CHEATSEARCH
// cheatsearch.cpp
int cheatSearchCreate();
void cheatSearchDestroy();
void updateCheatSearch();
#endif

// inpdipsw.cpp
void InpDIPSWResetDIPs();
int InpDIPSWCreate();

// inpjukebox.cpp
extern HANDLE hJukeThread;			// Handle to the jukebox window thread
extern DWORD dwJukeThreadID;		// ID of the jukebox window thread
int jukeCreate();
void jukeDestroy();

// inps.cpp
extern unsigned int nInpsInput;		// The input number we are redefining
int InpsCreate();
int InpsUpdate();

// inpc.cpp
extern unsigned int nInpcInput;		// The input number we are redefining
int InpcCreate();

// stated.cpp
extern int bDrvSaveAll;
int StatedAuto(int bSave);
int StatedLoad(int nSlot);
int StatedSave(int nSlot);

// numdial.cpp
void colorAdjustDialog(HWND);
void CPUClockDialog(HWND);
void aspectSetDialog(HWND);
void screenAngleDialog(HWND);

// sfactd.cpp
int SFactdCreate();
void ToggleLayer(unsigned char thisLayer);
void ToggleSprite(unsigned char PriNum);

// roms.cpp
extern bool avOk;
extern bool bRescanRoms;
int CreateROMInfo();

// miscpaths.cpp
enum ePath {
	PATH_PREVIEW = 0,
	PATH_CHEAT,
	PATH_SCREENSHOT,
	PATH_SAVESTATE,
	PATH_RECORDING,
	PATH_SKIN,
	PATH_IPS,
	PATH_TITLE,
	PATH_FLYER,
	PATH_SCORE,
	PATH_SELECT,
	PATH_GAMEOVER,
	PATH_BOSS,
	PATH_ICON,
	PATH_SUM
};
extern TCHAR szMiscPaths[PATH_SUM][MAX_PATH];
int miscDirCreate(HWND);
const TCHAR* getMiscPath(unsigned int dirType);
const TCHAR* getMiscArchiveName(unsigned int dirType);

extern TCHAR szAppRomPaths[DIRS_MAX][MAX_PATH];
int RomsDirCreate(HWND);
void pathSheetCreate(HWND);

// skin.cpp
extern bool bUseGdip;		// use GDI+
extern int nRandomSkin;
extern bool bVidUsePlaceholder;
extern TCHAR szPlaceHolder[MAX_PATH];
HBITMAP loadSkin(HWND hWnd);
void randomSelectSkin();
int selectSkin();
void paintSkin(HWND hWnd);

// fba_kaillera.cpp
int KailleraInitInput();
int KailleraGetInput();

extern int kNetGame;
void DoNetGame();
BOOL FBA_KailleraInit();
void FBA_KailleraEnd();
void FBA_KailleraSend();

int ActivateChat();
void DeActivateChat();
bool ChatActivated();
HWND GetChatWindow();

// replay.cpp
extern int nReplayStatus;
extern bool bReplayReadOnly;
extern bool bFrameCounterDisplay;
int RecordInput();
int ReplayInput();
int StartRecord();
int StartReplay(const TCHAR* szFileName = NULL);
void StopReplay();
int FreezeInput(unsigned char** buf, int* size);
int UnfreezeInput(const unsigned char* buf, int size);

// memcard.cpp
extern int nMemoryCardStatus;	// & 1 = file selected, & 2 = inserted
int MemCardCreate();
int MemCardSelect();
int MemCardInsert();
int MemCardEject();
int MemCardToggle();

// progress.cpp
int ProgressUpdateBurner(double dProgress, const TCHAR* pszText, bool bAbs);
int ProgressCreate();
int ProgressDestroy();

// dialogmanager.cpp
void dialogAdd(int id, HWND dialog);
HWND dialogGet(int id);
void dialogDelete(int id);
bool dialogIsEmpty();
void dialogClear();
 
// ----------------------------------------------------------------------------
// Debugger

// debugger.cpp
int DebugExit();
int DebugCreate();

// ----------------------------------------------------------------------------
// AVI recording

extern int nAviStatus;
extern int nAviIntAudio;
int AviStart();
int AviRecordFrame(int bDraw);
int AviStop();
void AviSetBuffer(unsigned char* buffer);

// ----------------------------------------------------------------------------
// Audit

// State is non-zero if found. 1 = found totally okay
enum FIND_STATE { STAT_NOFIND = 0, STAT_OK, STAT_CRC, STAT_SMALL, STAT_LARGE, };
enum AUDIT_STATE { AUDIT_FAIL = 0, AUDIT_PARTPASS = 1, AUDIT_FULLPASS = 3, };
extern char* auditState;

int getAllRomsetInfo();
void auditPrepare();
int auditRomset();
void auditCleanup();

void initAuditState();
void resetAuditState();
void freeAuditState();
char getAuditState(const unsigned int& id);
void setAuditState(const unsigned int& id, char ret);


// xbox utility functions

#define DEVICE_MEMORY_UNIT0 1
#define DEVICE_MEMORY_UNIT1 2
#define DEVICE_MEMORY_ONBOARD 3
#define DEVICE_CDROM0 4
#define DEVICE_HARDISK0_PART1 5
#define DEVICE_HARDISK0_SYSPART 6
#define DEVICE_USB0 7
#define DEVICE_USB1 8
#define DEVICE_USB2 9
#define DEVICE_TEST 10
#define DEVICE_CACHE 11

typedef struct _STRING {
    USHORT Length;
    USHORT MaximumLength;
    PCHAR Buffer;
} STRING;

extern "C" int __stdcall ObCreateSymbolicLink( STRING*, STRING*);

void DebugMsg(const char* format, ...);

#endif
