// Burn - Drivers module

#include "version.h"
#include "burnint.h"
#include "burn_sound.h"
#include "driverlist.h"

// filler function, used if the application is not printing debug messages
static int __cdecl BurnbprintfFiller(int, TCHAR* , ...) { return 0; }
// pointer to burner printing function
int (__cdecl *bprintf)(int nStatus, TCHAR* szFormat, ...) = BurnbprintfFiller;

int nBurnVer = BURN_VERSION;		// Version number of the library

unsigned int nBurnDrvCount = 0;		// Count of game drivers
unsigned int nBurnDrvActive = ~0U;	// Which game driver is selected
unsigned int nBurnDrvSelect[8] = { ~0U, ~0U, ~0U, ~0U, ~0U, ~0U, ~0U, ~0U }; // Which games are selected (i.e. loaded but not necessarily active)
									
unsigned int nBurnDrvMedium = 0;	// Which medium is loaded (if recognized)

bool bBurnUseMMX;
#if defined BUILD_A68K
bool bBurnUseASMCPUEmulation = true;
#else
bool bBurnUseASMCPUEmulation = false;
#endif

#if defined (FBA_DEBUG)
 clock_t starttime = 0;
#endif

unsigned int nCurrentFrame;			// Framecount for emulated game

unsigned int nFramesEmulated;		// Counters for FPS	display
unsigned int nFramesRendered;		//
bool bForce60Hz = false;
int nBurnFPS = 6000;
int nBurnCPUSpeedAdjust = 0x0100;	// CPU speed adjustment (clock * nBurnCPUSpeedAdjust / 0x0100)

// Burn Draw:
unsigned char* pBurnDraw = NULL;	// Pointer to correctly sized bitmap
int nBurnPitch = 0;					// Pitch between each line
int nBurnBpp;						// Bytes per pixel (2, 3, or 4)

int nBurnSoundRate = 0;				// sample rate of sound or zero for no sound
int nBurnSoundLen = 0;				// length in samples per frame
short* pBurnSoundOut = NULL;		// pointer to output buffer

int nInterpolation = 1;				// Desired interpolation level for ADPCM/PCM sound
int nFMInterpolation = 0;			// Desired interpolation level for FM sound

unsigned char nBurnLayer = 0xFF;	// Can be used externally to select which layers to show
unsigned char nSpriteEnable = 0xFF;	// Can be used externally to select which layers to show

int nMaxPlayers;

bool bSaveCRoms = 0;

unsigned int *pBurnDrvPalette;

bool BurnCheckMMXSupport()
{
#if defined BUILD_X86_ASM
	unsigned int nSignatureEAX = 0, nSignatureEBX = 0, nSignatureECX = 0, nSignatureEDX = 0;

	CPUID(1, nSignatureEAX, nSignatureEBX, nSignatureECX, nSignatureEDX);

	return (nSignatureEDX >> 23) & 1;						// bit 23 of edx indicates MMX support
#else
	return 0;
#endif
}

extern "C" int BurnLibInit()
{
	BurnLibExit();
	nBurnDrvCount = sizeof(pDriver) / sizeof(pDriver[0]);	// count available drivers

	cmc_4p_Precalc();
	bBurnUseMMX = BurnCheckMMXSupport();

	return 0;
}

extern "C" int BurnLibExit()
{
	nBurnDrvCount = 0;

	return 0;
}

int BurnGetZipName(char** pszName, unsigned int i)
{
	static char szFilename[MAX_PATH];
	char* pszGameName = NULL;

	if (pszName == NULL) {
		return 1;
	}

	if (i == 0) {
		pszGameName = pDriver[nBurnDrvActive]->szShortName;
	} else {
		int nOldBurnDrvSelect = nBurnDrvActive;
		unsigned int j = pDriver[nBurnDrvActive]->szBoardROM ? 1 : 0;

		// Try BIOS/board ROMs first
		if (i == 1 && j == 1) {										// There is a BIOS/board ROM
			pszGameName = pDriver[nBurnDrvActive]->szBoardROM;
		}

		if (pszGameName == NULL) {
			// Go through the list to seek out the parent
			while (j < i) {
				char* pszParent = pDriver[nBurnDrvActive]->szParent;
				pszGameName = NULL;

				if (pszParent == NULL) {							// No parent
					break;
				}

				for (nBurnDrvActive = 0; nBurnDrvActive < nBurnDrvCount; nBurnDrvActive++) {
		            if (strcmp(pszParent, pDriver[nBurnDrvActive]->szShortName) == 0) {	// Found parent
						pszGameName = pDriver[nBurnDrvActive]->szShortName;
						break;
					}
				}

				j++;
			}
		}

		nBurnDrvActive = nOldBurnDrvSelect;
	}

	if (pszGameName == NULL) {
		*pszName = NULL;
		return 1;
	}

	strcpy(szFilename, pszGameName);
	strcat(szFilename, ".zip");

	*pszName = szFilename;

	return 0;
}

// ----------------------------------------------------------------------------
// Static functions which forward to each driver's data and functions

int BurnStateMAMEScan(int nAction, int* pnMin);
void BurnStateExit();
int BurnStateInit();

// Get the text fields for the driver in TCHARs
extern "C" TCHAR* BurnDrvGetText(unsigned int i)
{
	char* pszStringA = NULL;
	wchar_t* pszStringW = NULL;
	static char* pszCurrentNameA;
	static wchar_t* pszCurrentNameW;

#if defined (_UNICODE)

	static wchar_t szShortNameW[32];
	static wchar_t szDateW[32];
	static wchar_t szFullNameW[256];
	static wchar_t szCommentW[256];
	static wchar_t szManufacturerW[256];
	static wchar_t szSystemW[256];
	static wchar_t szParentW[32];
	static wchar_t szBoardROMW[32];
	static wchar_t szSampleNameW[32];

#else

	static char szShortNameA[32];
	static char szDateA[32];
	static char szFullNameA[256];
	static char szCommentA[256];
	static char szManufacturerA[256];
	static char szSystemA[256];
	static char szParentA[32];
	static char szBoardROMA[32];
	static char szSampleNameA[32];

#endif

	if (!(i & DRV_ASCIIONLY)) {
		switch (i & 0xFF) {
			case DRV_FULLNAME:
				case DRV_MEDIUMNAME:
				if ((i & 0xFF) == DRV_MEDIUMNAME) {
					if (pDriver[nBurnDrvActive]->media == NULL) {
						return NULL;
					}
					pszStringW = pDriver[nBurnDrvActive]->media[nBurnDrvMedium].szFullNameW;
				} else {
					pszStringW = pDriver[nBurnDrvActive]->szFullNameW;
				}
				
				if (i & DRV_NEXTNAME) {
					if (pszCurrentNameW && pDriver[nBurnDrvActive]->szFullNameW) {
						pszCurrentNameW += wcslen(pszCurrentNameW) + 1;
						if (!pszCurrentNameW[0]) {
							return NULL;
						}
						pszStringW = pszCurrentNameW;
					}
				} else {

#if !defined (_UNICODE)

					// Ensure all of the Unicode titles are printable in the current locale
					pszCurrentNameW = pDriver[nBurnDrvActive]->szFullNameW;
					if (pszCurrentNameW && pszCurrentNameW[0]) {
						int nRet;

						do {
							nRet = wcstombs(szFullNameA, pszCurrentNameW, 256);
							pszCurrentNameW += wcslen(pszCurrentNameW) + 1;
						} while	(nRet >= 0 && pszCurrentNameW[0]);

						// If all titles can be printed, we can use the Unicode versions
						if (nRet >= 0) {
							pszStringW = pszCurrentNameW = pDriver[nBurnDrvActive]->szFullNameW;
						}
					}

#else

					pszStringW = pszCurrentNameW = pDriver[nBurnDrvActive]->szFullNameW;

#endif

				}
				break;
			case DRV_COMMENT:
				pszStringW = pDriver[nBurnDrvActive]->szCommentW;
				break;
			case DRV_MANUFACTURER:
				pszStringW = pDriver[nBurnDrvActive]->szManufacturerW;
				break;
			case DRV_SYSTEM:
				pszStringW = pDriver[nBurnDrvActive]->szSystemW;
		}

#if defined (_UNICODE)

		if (pszStringW && pszStringW[0]) {
			return pszStringW;
		}

#else

		switch (i & 0xFF) {
			case DRV_NAME:
				pszStringA = szShortNameA;
				break;
			case DRV_DATE:
				pszStringA = szDateA;
				break;
			case DRV_FULLNAME:
				pszStringA = szFullNameA;
				break;
			case DRV_MEDIUMNAME:
				pszStringA = szFullNameA;
				break;
			case DRV_COMMENT:
				pszStringA = szCommentA;
				break;
			case DRV_MANUFACTURER:
				pszStringA = szManufacturerA;
				break;
			case DRV_SYSTEM:
				pszStringA = szSystemA;
				break;
			case DRV_PARENT:
				pszStringA = szParentA;
				break;
			case DRV_BOARDROM:
				pszStringA = szBoardROMA;
				break;
			case DRV_SAMPLENAME:
				pszStringA = szSampleNameA;
				break;
		}

		if (pszStringW && pszStringA && pszStringW[0]) {
			if (wcstombs(pszStringA, pszStringW, 256) != -1U) {
				return pszStringA;
			}

		}

		pszStringA = NULL;

#endif

	}

	if (i & DRV_UNICODEONLY) {
		return NULL;
	}

	switch (i & 0xFF) {
		case DRV_NAME:
			pszStringA = pDriver[nBurnDrvActive]->szShortName;
			break;
		case DRV_DATE:
			pszStringA = pDriver[nBurnDrvActive]->szDate;
			break;
		case DRV_FULLNAME:
		case DRV_MEDIUMNAME:
			if ((i & 0xFF) == DRV_MEDIUMNAME) {
				if (pDriver[nBurnDrvActive]->media == NULL) {
					return NULL;
				}
				pszStringA = pDriver[nBurnDrvActive]->media[nBurnDrvMedium].szFullNameA;
			} else {
				pszStringA = pDriver[nBurnDrvActive]->szFullNameA;
			}
			pszStringA = ((i & 0xFF) == DRV_MEDIUMNAME) ? pDriver[nBurnDrvActive]->media[nBurnDrvMedium].szFullNameA : pDriver[nBurnDrvActive]->szFullNameA;

			if (i & DRV_NEXTNAME) {
				if (!pszCurrentNameW && pDriver[nBurnDrvActive]->szFullNameA) {
					pszCurrentNameA += strlen(pszCurrentNameA) + 1;
					if (!pszCurrentNameA[0]) {
						return NULL;
					}
					pszStringA = pszCurrentNameA;
				}
			} else {
				pszStringA = pszCurrentNameA = pDriver[nBurnDrvActive]->szFullNameA;
				pszCurrentNameW = NULL;
			}
			break;
		case DRV_COMMENT:
			pszStringA = pDriver[nBurnDrvActive]->szCommentA;
			break;
		case DRV_MANUFACTURER:
			pszStringA = pDriver[nBurnDrvActive]->szManufacturerA;
			break;
		case DRV_SYSTEM:
			pszStringA = pDriver[nBurnDrvActive]->szSystemA;
			break;
		case DRV_PARENT:
			pszStringA = pDriver[nBurnDrvActive]->szParent;
			break;
		case DRV_BOARDROM:
			pszStringA = pDriver[nBurnDrvActive]->szBoardROM;
			break;
		case DRV_SAMPLENAME:
			pszStringA = pDriver[nBurnDrvActive]->szSampleName;
	}

#if defined (_UNICODE)

	switch (i & 0xFF) {
		case DRV_NAME:
			pszStringW = szShortNameW;
			break;
		case DRV_DATE:
			pszStringW = szDateW;
			break;
		case DRV_FULLNAME:
			pszStringW = szFullNameW;
			break;
		case DRV_COMMENT:
			pszStringW = szCommentW;
			break;
		case DRV_MANUFACTURER:
			pszStringW = szManufacturerW;
			break;
		case DRV_SYSTEM:
			pszStringW = szSystemW;
			break;
		case DRV_PARENT:
			pszStringW = szParentW;
			break;
		case DRV_BOARDROM:
			pszStringW = szBoardROMW;
			break;
		case DRV_SAMPLENAME:
			pszStringW = szSampleNameW;
			break;
	}

	if (pszStringW && pszStringA && pszStringA[0]) {
		if (mbstowcs(pszStringW, pszStringA, 256) != -1U) {
			return pszStringW;
		}
	}

#else

	if (pszStringA && pszStringA[0]) {
		return pszStringA;
	}

#endif

	return NULL;
}


// Get the ASCII text fields for the driver in ASCII format;
extern "C" char* BurnDrvGetTextA(unsigned int i)
{
	switch (i) {
		case DRV_NAME:
			return pDriver[nBurnDrvActive]->szShortName;
		case DRV_DATE:
			return pDriver[nBurnDrvActive]->szDate;
		case DRV_FULLNAME:
			return pDriver[nBurnDrvActive]->szFullNameA;
		case DRV_MEDIUMNAME:
			return pDriver[nBurnDrvActive]->media[nBurnDrvMedium].szFullNameA;
		case DRV_COMMENT:
			return pDriver[nBurnDrvActive]->szCommentA;
		case DRV_MANUFACTURER:
			return pDriver[nBurnDrvActive]->szManufacturerA;
		case DRV_SYSTEM:
			return pDriver[nBurnDrvActive]->szSystemA;
		case DRV_PARENT:
			return pDriver[nBurnDrvActive]->szParent;
		case DRV_BOARDROM:
			return pDriver[nBurnDrvActive]->szBoardROM;
		case DRV_SAMPLENAME:
			return pDriver[nBurnDrvActive]->szSampleName;
		default:
			return NULL;
	}
}

// Get the zip names for the driver
extern "C" int BurnDrvGetZipName(char** pszName, unsigned int i)
{
	if (pDriver[nBurnDrvActive]->GetZipName) {									// Forward to drivers function
		return pDriver[nBurnDrvActive]->GetZipName(pszName, i);
	}

	return BurnGetZipName(pszName, i);											// Forward to general function
}

extern "C" int BurnDrvGetRomInfo(struct BurnRomInfo* pri, unsigned int i)		// Forward to drivers function
{
	return pDriver[nBurnDrvActive]->GetRomInfo(pri, i);
}

extern "C" int BurnDrvGetRomName(char** pszName, unsigned int i, int nAka)		// Forward to drivers function
{
	return pDriver[nBurnDrvActive]->GetRomName(pszName, i, nAka);
}

extern "C" int BurnDrvGetInputInfo(struct BurnInputInfo* pii, unsigned int i)	// Forward to drivers function
{
	return pDriver[nBurnDrvActive]->GetInputInfo(pii, i);
}

extern "C" int BurnDrvGetDIPInfo(struct BurnDIPInfo* pdi, unsigned int i)
{
	if (pDriver[nBurnDrvActive]->GetDIPInfo) {									// Forward to drivers function
		return pDriver[nBurnDrvActive]->GetDIPInfo(pdi, i);
	}

	return 1;																	// Fail automatically
}

extern "C" int BurnDrvGetSampleInfo(struct BurnSampleInfo* pri, unsigned int i)		// Forward to drivers function
{
	return pDriver[nBurnDrvActive]->GetSampleInfo(pri, i);
}

extern "C" int BurnDrvGetSampleName(char** pszName, unsigned int i, int nAka)		// Forward to drivers function
{
	return pDriver[nBurnDrvActive]->GetSampleName(pszName, i, nAka);
}

// Get the screen size
extern "C" int BurnDrvGetVisibleSize(int* pnWidth, int* pnHeight)
{
	*pnWidth =pDriver[nBurnDrvActive]->nWidth;
	*pnHeight=pDriver[nBurnDrvActive]->nHeight;

	return 0;
}

extern "C" int BurnDrvGetVisibleOffs(int* pnLeft, int* pnTop)
{
	*pnLeft = 0;
	*pnTop = 0;

	return 0;
}

extern "C" int BurnDrvGetFullSize(int* pnWidth, int* pnHeight)
{
	if (pDriver[nBurnDrvActive]->Flags & BDF_ORIENTATION_VERTICAL) {
		*pnWidth =pDriver[nBurnDrvActive]->nHeight;
		*pnHeight=pDriver[nBurnDrvActive]->nWidth;
	} else {
		*pnWidth =pDriver[nBurnDrvActive]->nWidth;
		*pnHeight=pDriver[nBurnDrvActive]->nHeight;
	}

	return 0;
}

// Get screen aspect ratio
extern "C" int BurnDrvGetAspect(int* pnXAspect, int* pnYAspect)
{
	*pnXAspect = pDriver[nBurnDrvActive]->nXAspect;
	*pnYAspect = pDriver[nBurnDrvActive]->nYAspect;

	return 0;
}

extern "C" int BurnDrvSetVisibleSize(int pnWidth, int pnHeight)
{
	if (pDriver[nBurnDrvActive]->Flags & BDF_ORIENTATION_VERTICAL) {
		pDriver[nBurnDrvActive]->nHeight = pnWidth;
		pDriver[nBurnDrvActive]->nWidth = pnHeight;
	} else {
		pDriver[nBurnDrvActive]->nWidth = pnWidth;
		pDriver[nBurnDrvActive]->nHeight = pnHeight;
	}
	
	return 0;
}

extern "C" int BurnDrvSetAspect(int pnXAspect, int pnYAspect)
{
	pDriver[nBurnDrvActive]->nXAspect = pnXAspect;
	pDriver[nBurnDrvActive]->nYAspect = pnYAspect;

	return 0;	
}

// Get the hardware code
extern "C" int BurnDrvGetHardwareCode()
{
	return pDriver[nBurnDrvActive]->Hardware;
}

// Get flags, including BDF_GAME_WORKING flag
extern "C" int BurnDrvGetFlags()
{
	return pDriver[nBurnDrvActive]->Flags;
}

// Return BDF_WORKING flag
extern "C" bool BurnDrvIsWorking()
{
	return pDriver[nBurnDrvActive]->Flags & BDF_GAME_WORKING;
}

// Return max. number of players
extern "C" int BurnDrvGetMaxPlayers()
{
	return pDriver[nBurnDrvActive]->Players;
}

// Return genre flags
extern "C" int BurnDrvGetGenreFlags()
{
	return pDriver[nBurnDrvActive]->Genre;
}

// Return family flags
extern "C" int BurnDrvGetFamilyFlags()
{
	return pDriver[nBurnDrvActive]->Family;
}

// Init game emulation (loading any needed roms)
extern "C" int BurnDrvInit()
{
	int nReturnValue;

	if (nBurnDrvActive >= nBurnDrvCount) {
		return 1;
	}

#if defined (FBA_DEBUG)
	{
		TCHAR szText[1024] = _T("");
		TCHAR* pszPosition = szText;
		TCHAR* pszName = BurnDrvGetText(DRV_FULLNAME);
		int nName = 1;

		while ((pszName = BurnDrvGetText(DRV_NEXTNAME | DRV_FULLNAME)) != NULL) {
			nName++;
		}

		// Print the title

		bprintf(PRINT_IMPORTANT, _T("*** Starting emulation of %s - %s.\n"), BurnDrvGetText(DRV_NAME), BurnDrvGetText(DRV_FULLNAME));

		// Then print the alternative titles

		if (nName > 1) {
			bprintf(PRINT_IMPORTANT, _T("    Alternative %s "), (nName > 2) ? _T("titles are") : _T("title is"));
			pszName = BurnDrvGetText(DRV_FULLNAME);
			nName = 1;
			while ((pszName = BurnDrvGetText(DRV_NEXTNAME | DRV_FULLNAME)) != NULL) {
				if (pszPosition + _tcslen(pszName) - 1022 > szText) {
					break;
				}
				if (nName > 1) {
					bprintf(PRINT_IMPORTANT, _T(SEPERATOR_1));
				}
				bprintf(PRINT_IMPORTANT, _T("%s"), pszName);
				nName++;
			}
			bprintf(PRINT_IMPORTANT, _T(".\n"));
		}
	}
#endif

	BurnSetRefreshRate(60.0);

	CheatInit();
	HiscoreInit();
	BurnStateInit();	

	nReturnValue = pDriver[nBurnDrvActive]->Init();	// Forward to drivers function

	nMaxPlayers = pDriver[nBurnDrvActive]->Players;
	
#if defined (FBA_DEBUG)
	if (!nReturnValue) {
		starttime = clock();
		nFramesEmulated = 0;
		nFramesRendered = 0;
		nCurrentFrame = 0;
	} else {
		starttime = 0;
	}
#endif

	return nReturnValue;
}

// Exit game emulation
extern "C" int BurnDrvExit()
{
#if defined (FBA_DEBUG)
	if (starttime) {
		clock_t endtime;
		clock_t nElapsedSecs;

		endtime = clock();
		nElapsedSecs = (endtime - starttime);
		bprintf(PRINT_IMPORTANT, _T(" ** Emulation ended (running for %.2f seconds).\n"), (float)nElapsedSecs / CLOCKS_PER_SEC);
		bprintf(PRINT_IMPORTANT, _T("    %.2f%% of frames rendered (%d out of a total %d).\n"), (float)nFramesRendered / nFramesEmulated * 100, nFramesRendered, nFramesEmulated);
		bprintf(PRINT_IMPORTANT, _T("    %.2f frames per second (average).\n"), (float)nFramesRendered / nFramesEmulated * nBurnFPS / 100);
		bprintf(PRINT_NORMAL, _T("\n"));
	}
#endif

	CheatExit();
	CheatSearchExit();
	HiscoreExit();
	BurnStateExit();

	nBurnCPUSpeedAdjust = 0x0100;
	
	pBurnDrvPalette = NULL;	

	return pDriver[nBurnDrvActive]->Exit();			// Forward to drivers function
}

int (__cdecl* BurnExtCartridgeSetupCallback)(BurnCartrigeCommand nCommand) = NULL;

int BurnDrvCartridgeSetup(BurnCartrigeCommand nCommand)
{
	if (nBurnDrvActive >= nBurnDrvCount || BurnExtCartridgeSetupCallback == NULL) {
		return 1;
	}

	if (nCommand == CART_EXIT) {
		return pDriver[nBurnDrvActive]->Exit();
	}

	if (nCommand != CART_INIT_END && nCommand != CART_INIT_START) {
		return 1;
	}

	BurnExtCartridgeSetupCallback(CART_INIT_END);

#if defined FBA_DEBUG
		bprintf(PRINT_NORMAL, _T("  * Loading"));
#endif

	if (BurnExtCartridgeSetupCallback(CART_INIT_START)) {
		return 1;
	}

	if (nCommand == CART_INIT_START) {
		return pDriver[nBurnDrvActive]->Init();
	}

	return 0;
}

extern "C" int BurnDrvFindMedium(char* pszShortName)
{
	if (pszShortName == NULL) {
		nBurnDrvMedium = 0;

		return 0;
	}

	for (nBurnDrvMedium = 1; pDriver[nBurnDrvActive]->media[nBurnDrvMedium].szShortName; nBurnDrvMedium++) {
		if (strcmp(pDriver[nBurnDrvActive]->media[nBurnDrvMedium].szShortName, pszShortName) == 0) {


//bprintf(PRINT_IMPORTANT, _T("    CD loaded: %s        \n"), pDriver[nBurnDrvActive]->media[nBurnDrvMedium].szFullNameW);

			return 0;
		}
	}

	nBurnDrvMedium = 0;

	return 1;
}

// Do one frame of game emulation
extern "C" int BurnDrvFrame()
{
	CheatApply();									// Apply cheats (if any)
	HiscoreApply();
	return pDriver[nBurnDrvActive]->Frame();		// Forward to drivers function
}

// Force redraw of the screen
extern "C" int BurnDrvRedraw()
{
	if (pDriver[nBurnDrvActive]->Redraw) {
		return pDriver[nBurnDrvActive]->Redraw();	// Forward to drivers function
	}

	return 1;										// No funtion provide, so simply return
}

// Refresh Palette
extern "C" int BurnRecalcPal()
{
	if (nBurnDrvActive < nBurnDrvCount) {
		unsigned char* pr = pDriver[nBurnDrvActive]->pRecalcPal;
		if (pr == NULL) return 1;
		*pr = 1;									// Signal for the driver to refresh it's palette
	}

	return 0;
}

extern "C" int BurnDrvGetPaletteEntries()
{
	return pDriver[nBurnDrvActive]->nPaletteEntries;
}

// ----------------------------------------------------------------------------

int (__cdecl *BurnExtProgressRangeCallback)(double fProgressRange) = NULL;
int (__cdecl *BurnExtProgressUpdateCallback)(double fProgress, const TCHAR* pszText, bool bAbs) = NULL;

int BurnSetProgressRange(double fProgressRange)
{
	if (BurnExtProgressRangeCallback) {
		return BurnExtProgressRangeCallback(fProgressRange);
	}

	return 1;
}

int BurnUpdateProgress(double fProgress, const TCHAR* pszText, bool bAbs)
{
	if (BurnExtProgressUpdateCallback) {
		return BurnExtProgressUpdateCallback(fProgress, pszText, bAbs);
	}

	return 1;
}

// ----------------------------------------------------------------------------

int BurnSetRefreshRate(double dFrameRate)
{
	if (!bForce60Hz) {
		nBurnFPS = (int)(100.0 * dFrameRate);
	}

	return 0;
}

inline static int BurnClearSize(int w, int h)
{
	unsigned char *pl;
	int y;

	w *= nBurnBpp;

	// clear the screen to zero
	for (pl = pBurnDraw, y = 0; y < h; pl += nBurnPitch, y++) {
		memset(pl, 0x00, w);
	}

	return 0;
}

int BurnClearScreen()
{
	struct BurnDriver* pbd = pDriver[nBurnDrvActive];

	if (pbd->Flags & BDF_ORIENTATION_VERTICAL) {
		BurnClearSize(pbd->nHeight, pbd->nWidth);
	} else {
		BurnClearSize(pbd->nWidth, pbd->nHeight);
	}

	return 0;
}

// Byteswaps an area of memory
int BurnByteswap(UINT8* pMem, int nLen)
{
	nLen >>= 1;
	for (int i = 0; i < nLen; i++, pMem += 2) {
		UINT8 t = pMem[0];
		pMem[0] = pMem[1];
		pMem[1] = t;
	}

	return 0;
}

// Application-defined rom loading function:
int (__cdecl *BurnExtLoadRom)(unsigned char *Dest,int *pnWrote,int i) = NULL;

// Application-defined colour conversion function
static unsigned int __cdecl BurnHighColFiller(int, int, int, int) { return (unsigned int)(~0); }
unsigned int (__cdecl *BurnHighCol) (int r, int g, int b, int i) = BurnHighColFiller;

// ----------------------------------------------------------------------------
// Colour-depth independant image transfer

unsigned short* pTransDraw = NULL;

static int nTransWidth, nTransHeight;

void BurnTransferClear()
{
	memset((void*)pTransDraw, 0, nTransWidth * nTransHeight * sizeof(short));
}

int BurnTransferCopy(UINT32* pPalette)
{
	UINT16* pSrc = pTransDraw;
	UINT8* pDest = pBurnDraw;
	
	if (!nTransWidth || !nTransHeight || !pTransDraw) {
		bprintf(PRINT_NORMAL, _T("BurnTransferCopy called without BurnTransferInit!\n"));
		return 1;
	}
	
	pBurnDrvPalette = pPalette;

	switch (nBurnBpp) {
		case 2: {
			for (int y = 0; y < nTransHeight; y++, pSrc += nTransWidth, pDest += nBurnPitch) {
				for (int x = 0; x < nTransWidth; x ++) {
					((UINT16*)pDest)[x] = pPalette[pSrc[x]];
				}
			}
			break;
		}
		case 3: {
			for (int y = 0; y < nTransHeight; y++, pSrc += nTransWidth, pDest += nBurnPitch) {
				for (int x = 0; x < nTransWidth; x++) {
					UINT32 c = pPalette[pSrc[x]];
					*(pDest + (x * 3) + 0) = c & 0xFF;
					*(pDest + (x * 3) + 1) = (c >> 8) & 0xFF;
					*(pDest + (x * 3) + 2) = c >> 16;

				}
			}
			break;
		}
		case 4: {
			for (int y = 0; y < nTransHeight; y++, pSrc += nTransWidth, pDest += nBurnPitch) {
				for (int x = 0; x < nTransWidth; x++) {
					((UINT32*)pDest)[x] = pPalette[pSrc[x]];
				}
			}
			break;
		}
	}

	return 0;
}

void BurnTransferExit()
{
	if (pTransDraw) {
		free(pTransDraw);
		pTransDraw = NULL;
	}	
}

int BurnTransferInit()
{
	if (BurnDrvGetFlags() & BDF_ORIENTATION_VERTICAL) {
		BurnDrvGetVisibleSize(&nTransHeight, &nTransWidth);
	} else {
		BurnDrvGetVisibleSize(&nTransWidth, &nTransHeight);
	}

	pTransDraw = (unsigned short*)malloc(nTransWidth * nTransHeight * sizeof(short));
	if (pTransDraw == NULL) {
		return 1;
	}

	BurnTransferClear();

	return 0;
}

// ----------------------------------------------------------------------------
// Savestate support

// Application-defined callback for processing the area
static int __cdecl DefAcb (struct BurnArea* /* pba */) { return 1; }
int (__cdecl *BurnAcb) (struct BurnArea* pba) = DefAcb;

// Scan driver data
int BurnAreaScan(int nAction, int* pnMin)
{
	int nRet = 0;

	// Handle any MAME-style variables
	if (nAction & ACB_DRIVER_DATA) {
		nRet = BurnStateMAMEScan(nAction, pnMin);
	}

	// Forward to the driver
	if (pDriver[nBurnDrvActive]->AreaScan) {
		nRet |= pDriver[nBurnDrvActive]->AreaScan(nAction, pnMin);
	}

	return nRet;
}

// ----------------------------------------------------------------------------
// Wrappers for MAME-specific function calls

#include "driver.h"

// ----------------------------------------------------------------------------
// Wrapper for MAME logerror calls

#if defined (FBA_DEBUG) && defined (MAME_USE_LOGERROR)
void logerror(char* szFormat, ...)
{
	static char szLogMessage[1024];

	va_list vaFormat;
	va_start(vaFormat, szFormat);

	_vsnprintf(szLogMessage, 1024, szFormat, vaFormat);

	va_end(vaFormat);

	bprintf(PRINT_ERROR, _T("%hs"), szLogMessage);

	return;
}
#endif

// ----------------------------------------------------------------------------
// Wrapper for MAME state_save_register_* calls

struct BurnStateEntry { BurnStateEntry* pNext; BurnStateEntry* pPrev; char szName[256]; void* pValue; unsigned int nSize; };

static BurnStateEntry* pStateEntryAnchor = NULL;
typedef void (*BurnPostloadFunction)();
static BurnPostloadFunction BurnPostload[8];

static void BurnStateRegister(const char* module, int instance, const char* name, void* val, unsigned int size)
{
	// Allocate new node
	BurnStateEntry* pNewEntry = (BurnStateEntry*)malloc(sizeof(BurnStateEntry));
	if (pNewEntry == NULL) {
		return;
	}

	memset(pNewEntry, 0, sizeof(BurnStateEntry));

	// Link the new node
	pNewEntry->pNext = pStateEntryAnchor;
	if (pStateEntryAnchor) {
		pStateEntryAnchor->pPrev = pNewEntry;
	}
	pStateEntryAnchor = pNewEntry;

	sprintf(pNewEntry->szName, "%s:%s %i", module, name, instance);

	pNewEntry->pValue = val;
	pNewEntry->nSize = size;
}

void BurnStateExit()
{
	if (pStateEntryAnchor) {
		BurnStateEntry* pCurrentEntry = pStateEntryAnchor;
		BurnStateEntry* pNextEntry;

		do {
			pNextEntry = pCurrentEntry->pNext;
			if (pCurrentEntry) {
				free(pCurrentEntry);
			}
		} while ((pCurrentEntry = pNextEntry) != 0);
	}

	pStateEntryAnchor = NULL;

	for (int i = 0; i < 8; i++) {
		BurnPostload[i] = NULL;
	}
}

int BurnStateInit()
{
	BurnStateExit();

	return 0;
}

int BurnStateMAMEScan(int nAction, int* pnMin)
{
	if (nAction & ACB_VOLATILE) {

		if (pnMin && *pnMin < 0x029418) {						// Return minimum compatible version
			*pnMin = 0x029418;
		}

		if (pStateEntryAnchor) {
			struct BurnArea ba;
			BurnStateEntry* pCurrentEntry = pStateEntryAnchor;

			do {
			   	ba.Data		= pCurrentEntry->pValue;
				ba.nLen		= pCurrentEntry->nSize;
				ba.nAddress = 0;
				ba.szName	= pCurrentEntry->szName;
				BurnAcb(&ba);

			} while ((pCurrentEntry = pCurrentEntry->pNext) != 0);
		}

		if (nAction & ACB_WRITE) {
			for (int i = 0; i < 8; i++) {
				if (BurnPostload[i]) {
					BurnPostload[i]();
				}
			}
		}
	}

	return 0;
}

// wrapper functions

extern "C" void state_save_register_func_postload(void (*pFunction)())
{
	for (int i = 0; i < 8; i++) {
		if (BurnPostload[i] == NULL) {
			BurnPostload[i] = pFunction;
			break;
		}
	}
}

extern "C" void state_save_register_INT8(const char* module, int instance, const char* name, INT8* val, unsigned int size)
{
	BurnStateRegister(module, instance, name, (void*)val, size * sizeof(INT8));
}

extern "C" void state_save_register_UINT8(const char* module, int instance, const char* name, UINT8* val, unsigned int size)
{
	BurnStateRegister(module, instance, name, (void*)val, size * sizeof(UINT8));
}

extern "C" void state_save_register_INT16(const char* module, int instance, const char* name, INT16* val, unsigned int size)
{
	BurnStateRegister(module, instance, name, (void*)val, size * sizeof(INT16));
}

extern "C" void state_save_register_UINT16(const char* module, int instance, const char* name, UINT16* val, unsigned int size)
{
	BurnStateRegister(module, instance, name, (void*)val, size * sizeof(UINT16));
}

extern "C" void state_save_register_INT32(const char* module, int instance, const char* name, INT32* val, unsigned int size)
{
	BurnStateRegister(module, instance, name, (void*)val, size * sizeof(INT32));
}

extern "C" void state_save_register_UINT32(const char* module, int instance, const char* name, UINT32* val, unsigned int size)
{
	BurnStateRegister(module, instance, name, (void*)val, size * sizeof(UINT32));
}

extern "C" void state_save_register_int(const char* module, int instance, const char* name, int* val)
{
	BurnStateRegister(module, instance, name, (void*)val, sizeof(int));
}

extern "C" void state_save_register_float(const char* module, int instance, const char* name, float* val, unsigned int size)
{
	BurnStateRegister(module, instance, name, (void*)val, size * sizeof(float));
}

extern "C" void state_save_register_double(const char* module, int instance, const char* name, double* val, unsigned int size)
{
	BurnStateRegister(module, instance, name, (void*)val, size * sizeof(double));
}
