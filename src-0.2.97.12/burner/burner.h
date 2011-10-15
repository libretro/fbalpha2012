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

#ifdef __LIBSNES__
#include "port-typedefs.h"
#else
#include "tchar.h"
#endif

// Macro to make quoted strings
#define MAKE_STRING_2(s) #s
#define MAKE_STRING(s) MAKE_STRING_2(s)

#define BZIP_MAX (8)								// Maximum zip files to search through
#define DIRS_MAX (8)								// Maximum number of directories to search

#include "title.h"
#include "burn.h"

// ---------------------------------------------------------------------------
// OS dependent functionality

#if defined (BUILD_WIN32)
 #include "burner_win32.h"
#elif defined (BUILD_SDL)
 #include "burner_sdl.h"
#endif

#ifndef __LIBSNES__
#include <shellapi.h>
#include <shlwapi.h>
#include "net.h"
#include "png.h"
#include "dwmapi_core.h"
#endif

// ---------------------------------------------------------------------------
// OS independent functionality

#ifndef __LIBSNES__
#include "interface.h"
#endif

#define IMG_FREE		(1 << 0)

// Macros for parsing text
#define SKIP_WS(s) while (_istspace(*s)) { s++; }			// Skip whitespace
#define FIND_WS(s) while (*s && !_istspace(*s)) { s++; }	// Find whitespace
#define FIND_QT(s) while (*s && *s != _T('\"')) { s++; }	// Find quote

// image.cpp
#ifdef USE_IMAGE
void img_free(IMAGE* img);
int img_alloc(IMAGE* img);

bool PNGIsImage(FILE* fp);
int PNGLoad(IMAGE* img, FILE* fp, int nPreset);
#endif

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
char * InputCodeDesc(int c);
char * InpToDesc(struct GameInp* pgi);
char * InpMacroToDesc(struct GameInp* pgi);
void GameInpCheckLeftAlt();
void GameInpCheckMouse();
int GameInpBlank(int bDipSwitch);
int GameInputAutoIni(int nPlayer, char * lpszFile, bool bOverWrite);
int ConfigGameLoadHardwareDefaults();
int GameInpDefault();
int GameInpWrite(FILE* h);
int GameInpRead(char * szVal, bool bOverWrite);
int GameInpMacroRead(char * szVal, bool bOverWrite);
int GameInpCustomRead(char* szVal, bool bOverWrite);

// Player Default Controls
extern int nPlayerDefaultControls[4];
extern char szPlayerDefaultIni[4][MAX_PATH];

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
int QuoteRead(char ** ppszQuote, char ** ppszEnd, char * pszSrc);					// Read a quoted string from szSrc and point to the end
char* LabelCheck(char* s, char* pszLabel);

extern int bDoGamma;
extern int bHardwareGammaOnly;
extern double nGamma;

int SetBurnHighCol(int nDepth);
char* DecorateGameName(unsigned int nBurnDrv);
char* DecorateGenreInfo();
void ComputeGammaLUT();

// dat.cpp
int write_datfile(int bIncMegadrive, FILE* fDat);
int create_datfile(char* szFilename, int bIncMegadrive);

// sshot.cpp
int MakeScreenShot();

// state.cpp
int BurnStateLoadEmbed(FILE* fp, int nOffset, int bAll, int (*pLoadGame)());
int BurnStateLoad(char* szName, int bAll, int (*pLoadGame)());
int BurnStateSaveEmbed(FILE* fp, int nOffset, int bAll);
int BurnStateSave(char* szName, int bAll);

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
