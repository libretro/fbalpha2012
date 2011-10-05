// FB Alpha - Emulator for MC68000/Z80 based arcade games
//            Refer to the "license.txt" file for more info

#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <ctype.h>

//#include "tchar.h"

// Macro to make quoted strings
#define MAKE_STRING_2(s) #s
#define MAKE_STRING(s) MAKE_STRING_2(s)

#define BZIP_MAX (8)								// Maximum zip files to search through
#define DIRS_MAX (8)								// Maximum number of directories to search

#include "title.h"
#include "burn.h"
#include "typedefs.h"

// ---------------------------------------------------------------------------
// OS dependent functionality

#if defined (BUILD_WIN32)
 #include "burner_win32.h"
#include <shellapi.h>
#include <shlwapi.h>
#include "net.h"
#include "menugui.h"
#include "imagebutton.h"
#include "dwmapi_core.h"
#elif defined (BUILD_SDL)
 #include "burner_sdl.h"
#endif

// ---------------------------------------------------------------------------
// OS independent functionality

#if defined(SN_TARGET_PS3)
#include "interface-ps3.h"
#elif defined (__LIBSNES__)
#else
#include "interface.h"
#endif

// Macros for parsing text
#define SKIP_WS(s) while (_istspace(*s)) { s++; }			// Skip whitespace
#define FIND_WS(s) while (*s && !_istspace(*s)) { s++; }	// Find whitespace
#define FIND_QT(s) while (*s && *s != _T('\"')) { s++; }	// Find quote

// gami.cpp
extern struct GameInp* GameInp;
extern unsigned int nGameInpCount;
extern unsigned int nMacroCount;
extern unsigned int nMaxMacro;

extern int nAnalogSpeed;

extern int nFireButtons;

extern bool bStreetFighterLayout;
extern bool bLeftAltkeyMapped;

int GameInpInit();
int GameInpExit();
TCHAR* InputCodeDesc(int c);
TCHAR* InpToDesc(struct GameInp* pgi);
TCHAR* InpMacroToDesc(struct GameInp* pgi);
void GameInpCheckLeftAlt();
void GameInpCheckMouse();
int GameInpBlank(int bDipSwitch);
int GameInputAutoIni(int nPlayer, TCHAR* lpszFile, bool bOverWrite);
int ConfigGameLoadHardwareDefaults();
int GameInpDefault();
int GameInpWrite(FILE* h);
int GameInpRead(TCHAR* szVal, bool bOverWrite);
int GameInpMacroRead(TCHAR* szVal, bool bOverWrite);
int GameInpCustomRead(TCHAR* szVal, bool bOverWrite);

// Player Default Controls
extern int nPlayerDefaultControls[4];
extern TCHAR szPlayerDefaultIni[4][MAX_PATH];

// cong.cpp
extern const int nConfigMinVersion;					// Minimum version of application for which input files are valid
extern bool bSaveInputs;
int ConfigGameLoad(bool bOverWrite);				// char* lpszName = NULL
int ConfigGameSave(bool bSave);

// conc.cpp
int ConfigCheatLoad();

// gamc.cpp
int GamcMisc(struct GameInp* pgi, char* szi, int nPlayer);
int GamcAnalogKey(struct GameInp* pgi, char* szi, int nPlayer, int nSlide);
int GamcAnalogJoy(struct GameInp* pgi, char* szi, int nPlayer, int nJoy, int nSlide);
int GamcPlayer(struct GameInp* pgi, char* szi, int nPlayer, int nDevice);
int GamcPlayerHotRod(struct GameInp* pgi, char* szi, int nPlayer, int nFlags, int nSlide);

// misc.cpp
#define QUOTE_MAX (128)															// Maximum length of "quoted strings"
int QuoteRead(TCHAR** ppszQuote, TCHAR** ppszEnd, TCHAR* pszSrc);					// Read a quoted string from szSrc and point to the end
TCHAR* LabelCheck(TCHAR* s, TCHAR* pszLabel);

extern int bDoGamma;
extern int bHardwareGammaOnly;
extern double nGamma;

int SetBurnHighCol(int nDepth);
char* DecorateGameName(unsigned int nBurnDrv);
TCHAR* DecorateGenreInfo();
void ComputeGammaLUT();

// dat.cpp
int write_datfile(int nDatType, int bIncMegadrive, FILE* fDat);
int create_datfile(TCHAR* szFilename, int nDatType, int bIncMegadrive);

// sshot.cpp
int MakeScreenShot();

// state.cpp
int BurnStateLoadEmbed(FILE* fp, int nOffset, int bAll, int (*pLoadGame)());
int BurnStateLoad(TCHAR* szName, int bAll, int (*pLoadGame)());
int BurnStateSaveEmbed(FILE* fp, int nOffset, int bAll);
int BurnStateSave(TCHAR* szName, int bAll);

// statec.cpp
int BurnStateCompress(unsigned char** pDef, int* pnDefLen, int bAll);
int BurnStateDecompress(unsigned char* Def, int nDefLen, int bAll);

// zipfn.cpp
struct ZipEntry { char* szName;	unsigned int nLen; unsigned int nCrc; };

int ZipOpen(const char* szZip);
int ZipClose();
int ZipGetList(struct ZipEntry** pList, int* pnListCount);
int ZipLoadFile(unsigned char* Dest, int nLen, int* pnWrote, int nEntry);

// bzip.cpp

#define BZIP_STATUS_OK		(0)
#define BZIP_STATUS_BADDATA	(1)
#define BZIP_STATUS_ERROR	(2)

int BzipOpen(bool);
int BzipClose();
int BzipInit();
int BzipExit();
int BzipStatus();
