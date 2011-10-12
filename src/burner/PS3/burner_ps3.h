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

#define DEFAULT_INP_FILENAME "/dev_hdd0/game/FBAN00000/USRDIR/config/presets/default.ini"
#define DEFAULT_NEO_INP_FILENAME "/dev_hdd0/game/FBAN00000/USRDIR/config/presets/default_neo.ini"
#define DEFAULT_CPS_INP_FILENAME "/dev_hdd0/game/FBAN00000/USRDIR/config/presets/default_cps.ini"
#define GAME_INPUT_DIR "/dev_hdd0/game/FBAN00000/USRDIR/config/games/"

#define SHADER_DIRECTORY "/dev_hdd0/game/FBAN00000/USRDIR/shaders/"
#define DEFAULT_SHADER "/dev_hdd0/game/FBAN00000/USRDIR/shaders/stock.cg"
#define SYS_CONFIG_FILE "/dev_hdd0/game/FBAN00000/USRDIR/fbanext-ps3.cfg"
#define DAT_FILE "/dev_hdd0/game/FBAN00000/USRDIR/clrmame.dat"
#define MULTIMAN_SELF "/dev_hdd0/game/BLES80608/USRDIR/RELOAD.SELF"
#define PREVIEWS_DIR "/dev_hdd0/game/FBAN00000/USRDIR/previews/"
#define CHEATS_DIR "/dev_hdd0/game/FBAN00000/USRDIR/cheats/"
#define ROMS_DIR "/dev_hdd0/game/FBAN00000/USRDIR/roms/"
#define SAVESTATES_DIR "/dev_hdd0/game/FBAN00000/USRDIR/savestates/"
//redundant
#define SCREENSHOTS_DIR "/dev_hdd0/game/FBAN00000/USRDIR/screenshots/"
#define RECORDINGS_DIR "/dev_hdd0/game/FBAN00000/USRDIR/recordings/"
#define SKINS_DIR "/dev_hdd0/game/FBAN00000/USRDIR/skins/"
#define SCORES_DIR "/dev_hdd0/game/FBAN00000/USRDIR/scores/"
#define SELECTS_DIR "/dev_hdd0/game/FBAN00000/USRDIR/selects/"
#define IPS_DIR "/dev_hdd0/game/FBAN00000/USRDIR/ips/"
#define TITLES_DIR "/dev_hdd0/game/FBAN00000/USRDIR/titles/"
#define FLYERS_DIR "/dev_hdd0/game/FBAN00000/USRDIR/flyers/"
#define GAMEOVERS_DIR "/dev_hdd0/game/FBAN00000/USRDIR/gameovers/"
#define BOSSES_DIR "/dev_hdd0/game/FBAN00000/USRDIR/bosses/"
#define ICONS_DIR "/dev_hdd0/game/FBAN00000/USRDIR/icons/"
typedef std::basic_string<char> tstring;

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
extern int is_running;
extern int nVidScrnAspectMode;
extern int nVidOriginalScrnAspectX;
extern int nVidOriginalScrnAspectY;

// vid_psgl.cpp
extern uint32_t currentAvailableResolutionId;
extern uint32_t currentAvailableResolutionNo;

typedef struct {
int index;
char filename[512];
char fullpath[1024];
} selected_shader_t;

extern selected_shader_t selectedShader[2];

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

// media.cpp
int mediaInit();
int mediaExit();
int mediaReInitAudio();

// misc_win32.cpp
void createNeedDir();
bool directoryExists(const char* dirname);
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
extern int nXScale;
extern int nYScale;

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

extern char szAppRomPaths[DIRS_MAX][MAX_PATH];

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
