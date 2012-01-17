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
 
extern int custom_aspect_ratio_mode;

#define DEFAULT_INP_FILENAME "/dev_hdd0/game/FBAN00000/USRDIR/config/presets/default.ini"
#define DEFAULT_NEO_INP_FILENAME "/dev_hdd0/game/FBAN00000/USRDIR/config/presets/default_neo.ini"
#define DEFAULT_CPS_INP_FILENAME "/dev_hdd0/game/FBAN00000/USRDIR/config/presets/default_cps.ini"
#define DEFAULT_MENU_BORDER_FILE "/dev_hdd0/game/FBAN00000/USRDIR/borders/Menu/main-menu.png"
#define DEFAULT_MENU_SHADER_FILE "/dev_hdd0/game/FBAN00000/USRDIR/shaders/Borders/Menu/border-only.cg"
#define GAME_INPUT_DIR "/dev_hdd0/game/FBAN00000/USRDIR/config/games/"

#define DAT_FILE "/dev_hdd0/game/FBAN00000/USRDIR/clrmame.dat"
#define CHEATS_DIR "/dev_hdd0/game/FBAN00000/USRDIR/cheats/"

#ifndef MAX_PATH
 #define MAX_PATH (260)
#endif

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

extern int nLastFilter;
extern int nLastRom;
extern int nLastFilter;
extern int HideChildren;
extern int ThreeOrFourPlayerOnly;
extern int CurrentFilter;
extern int nVidScrnAspectMode;
extern int nVidOriginalScrnAspectX;
extern int nVidOriginalScrnAspectY;

extern char szAppBurnVer[16];
extern char szSVNVer[16];
extern char szSVNDate[30];

int dprintf(char* pszFormat, ...);	// Use instead of printf() in the UI

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
int FBAPopupAddText(int nFlags, char* pszFormat, ...);
int FBAPopupDestroyText();

// misc_win32.cpp
void pathCheck(char * path);
int directLoadGame(const char * name);
int findRom(int i, struct ArcEntry* list, int count);
extern void setWindowAspect(bool first_boot);

// drv.cpp
extern int bDrvOkay;				// 1 if the Driver has been inited okay, and it's okay to use the BurnDrv functions
int BurnerDrvInit(int nDrvNum, bool bRestore);
int DrvInitCallback();				// Used when Burn library needs to load a game. DrvInit(nBurnSelect, false)
int BurnerDrvExit();

// run.cpp
extern int nAppVirtualFps;			// virtual fps
extern bool bShowFPS;
extern uint32_t bBurnFirstStartup;

void RunIdle();
int RunMessageLoop(int argc, char ** argv);
int RunExit();

// scrn.cpp
extern int nWindowSize;
extern int nWindowPosX, nWindowPosY;

extern int nSavestateSlot;

extern int nXOffset;
extern int nYOffset;

extern void UpdateConsoleXY(const char *text, float X, float Y);

void simpleReinitScrn(void);

// sel.cpp
extern int nLoadMenuShowX;
extern int nLoadDriverShowX;
extern int nTabSel;
extern char szUserFilterStr[MAX_PATH];
extern int nSystemSel;
void clearNodeInfo();

// translist.cpp
extern char szTransGamelistFile[MAX_PATH];
int loadGamelist();
int createGamelist();
char* mangleGamename(const char* pszOldName, bool bRemoveArticle = true);
char* transGameName(const char* pszOldName, bool bRemoveArticle = true);

// config.cpp
extern int nIniVersion;
int configAppLoadXml();
int configAppSaveXml();

// conc.cpp
int configCheatLoad(const char* filename = NULL);
int configCheatReload(const char* filename = NULL);

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
extern char szMiscPaths[PATH_SUM][MAX_PATH];
const char * getMiscPath(unsigned int dirType);
const char * getMiscArchiveName(unsigned int dirType);

extern char szAppRomPaths[MAX_PATH];

// memcard.cpp
extern int nMemoryCardStatus;	// & 1 = file selected, & 2 = inserted
int MemCardCreate();
int MemCardSelect();
int MemCardInsert();
int MemCardEject();
int MemCardToggle();

// progress.cpp
int ProgressUpdateBurner(const char* pszText);
int ProgressCreate();
int ProgressDestroy();

// ----------------------------------------------------------------------------
// Audit

// State is non-zero if found. 1 = found totally okay
#define STAT_NOFIND	0
#define STAT_OK		1
#define STAT_CRC	2
#define STAT_SMALL	3
#define STAT_LARGE	4

#define AUDIT_FAIL	0
#define AUDIT_PARTPASS	1
#define AUDIT_FULLPASS	3

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
