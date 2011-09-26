#ifndef BURNER_PS3_H
#define BURNER_PS3_H

#ifdef _UNICODE
 #define UNICODE
#endif

#include <string.h>
#include <sys/timer.h>
#include <sys/return_code.h>
#include <sys/process.h>
#include <sys/vm.h>
#include <cell/audio.h>
#include <cell/sysmodule.h>
#include <cell/cell_fs.h>
#include <cell/pad.h>
#include <stddef.h>
#include <math.h>
#include <sysutil/sysutil_sysparam.h>
 
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

using std::string;
using std::list;
using std::set;
using std::map;
using std::vector;
using std::multimap;
using std::ofstream;
 
enum
{
   ASPECT_RATIO_4_3,
   ASPECT_RATIO_5_4,
   ASPECT_RATIO_7_5,
   ASPECT_RATIO_8_7,
   ASPECT_RATIO_10_7,
   ASPECT_RATIO_11_8,
   ASPECT_RATIO_12_7,
   ASPECT_RATIO_16_9,
   ASPECT_RATIO_16_10,
   ASPECT_RATIO_16_15,
   ASPECT_RATIO_19_14,
   ASPECT_RATIO_1_1,
   ASPECT_RATIO_2_1,
   ASPECT_RATIO_3_2,
   ASPECT_RATIO_3_4,
   ASPECT_RATIO_CUSTOM,
   ASPECT_RATIO_AUTO,
   ASPECT_RATIO_AUTO_FBA
};

#define LAST_ASPECT_RATIO ASPECT_RATIO_AUTO_FBA

extern int custom_aspect_ratio_mode;

#define SHADER_DIRECTORY "/dev_hdd0/game/FBAN00000/USRDIR/shaders/"

typedef std::basic_string<TCHAR> tstring;

#ifndef MAX_PATH
 #define MAX_PATH (260)
#endif


typedef struct tagRECT
{
    long    left;
    long    top;
    long    right;
    long    bottom;
} RECT;

typedef struct tagPOINT
{
    long  x;
    long  y;
} POINT;
 
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

extern int nAppThreadPriority;
extern int nAppShowCmd;
extern int nDisableSplash;
extern int nLastFilter;
extern int nLastRom;
extern int nLastFilter;
extern int HideChildren;
extern int ThreeOrFourPlayerOnly;
extern int CurrentFilter;
extern int CurrentFitlerType;
extern int is_running;
extern int nVidScrnAspectMode;
extern int nVidOriginalScrnAspectX;
extern int nVidOriginalScrnAspectY;

extern TCHAR szAppBurnVer[16];
extern TCHAR szSVNVer[16];
extern TCHAR szSVNDate[30];

extern bool bCmdOptUsed;
extern bool bAlwaysProcessKey;

// Used for the load/save dialog in commdlg.h
extern TCHAR szChoice[MAX_PATH];	// File chosen by the user

int dprintf(TCHAR* pszFormat, ...);	// Use instead of printf() in the UI

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

// media.cpp
int mediaInit();
int mediaExit();
int mediaChangeFps(int scale);
int mediaReInitAudio();
int mediaReInitScrn();

// misc_win32.cpp
void createNeedDir();
bool directoryExists(const TCHAR* dirname);
void pathCheck(char * path);
int directLoadGame(const char * name);
int getClientScreenRect(HWND hWnd, RECT* pRect);
int wndInMid(HWND hMid, HWND hBase);
void setWindowAspect(bool first_boot = false);
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

void RunIdle();
int RunMessageLoop(int argc, char ** argv);
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

extern void UpdateConsoleXY(char *text, float X, float Y);

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

// inpd.cpp
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

// inps.cpp
extern unsigned int nInpsInput;		// The input number we are redefining

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
const char * getMiscPath(unsigned int dirType);
const char * getMiscArchiveName(unsigned int dirType);

extern TCHAR szAppRomPaths[DIRS_MAX][MAX_PATH];
int RomsDirCreate(HWND);
void pathSheetCreate(HWND);

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

void DebugMsg(const char* format, ...);

#endif
