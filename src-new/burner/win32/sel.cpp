// Driver Selector module
// TreeView Version by HyperYagami
#include "burner.h"

int	nIconsSize					= ICON_16x16;
int	nIconsSizeXY				= 16;
bool bEnableIcons				= 0;
bool bIconsLoaded				= 0;
int nIconsXDiff;
int nIconsYDiff;

UINT_PTR nTimer					= 0;
int nDialogSelect				= -1;										// The driver which this dialog selected
int nOldDlgSelected				= -1;
bool bDialogCancel				= false;

static int nShowMVSCartsOnly	= 0;

bool bMVSMultiSlot				= false;

bool bGameInfoOpen				= false;
bool bDrvSelected				= false;

HBITMAP hPrevBmp				= NULL;
static HBITMAP hPreview			= NULL;

HWND hSelDlg					= NULL;
static HWND hSelList			= NULL;
static HWND hParent				= NULL;
static HWND hInfoLabel[6]		= { NULL, NULL, NULL, NULL, NULL };			// 4 things in our Info-box
static HWND hInfoText[6]		= { NULL, NULL, NULL, NULL, NULL };			// 4 things in our Info-box

static HBRUSH hWhiteBGBrush;
static HICON hExpand, hCollapse;
static HICON hNotWorking, hNotFoundEss, hNotFoundNonEss;

static HICON hDrvIcon[9999];
static HICON hDrvIconMiss;

static char TreeBuilding		= 0;										// if 1, ignore TVN_SELCHANGED messages

// Filter TreeView
HWND hFilterList			= NULL;
HTREEITEM hFilterAtari			= NULL;
HTREEITEM hFilterCave			= NULL;
HTREEITEM hFilterCps1			= NULL;
HTREEITEM hFilterCps2			= NULL;
HTREEITEM hFilterCps3			= NULL;
HTREEITEM hFilterGalaxian		= NULL;
HTREEITEM hFilterKaneko16		= NULL;
HTREEITEM hFilterKonami			= NULL;
HTREEITEM hFilterNeogeo			= NULL;
HTREEITEM hFilterPacman			= NULL;
HTREEITEM hFilterPgm			= NULL;
HTREEITEM hFilterPsikyo			= NULL;
HTREEITEM hFilterSega			= NULL;
HTREEITEM hFilterTaito			= NULL;
HTREEITEM hFilterToaplan		= NULL;
HTREEITEM hFilterMegadrive		= NULL;
HTREEITEM hFilterMiscPre90s		= NULL;
HTREEITEM hFilterMiscPost90s		= NULL;
HTREEITEM hFilterBootleg		= NULL;
HTREEITEM hFilterDemo			= NULL;
HTREEITEM hFilterHack			= NULL;
HTREEITEM hFilterHomebrew		= NULL;
HTREEITEM hFilterPrototype		= NULL;
HTREEITEM hFilterGenuine		= NULL;
HTREEITEM hFilterHorshoot		= NULL;
HTREEITEM hFilterVershoot		= NULL;
HTREEITEM hFilterScrfight		= NULL;
HTREEITEM hFilterVsfight		= NULL;
HTREEITEM hFilterBios			= NULL;
HTREEITEM hFilterBreakout		= NULL;
HTREEITEM hFilterCasino			= NULL;
HTREEITEM hFilterBallpaddle		= NULL;
HTREEITEM hFilterMaze			= NULL;
HTREEITEM hFilterMinigames		= NULL;
HTREEITEM hFilterPinball		= NULL;
HTREEITEM hFilterPlatform		= NULL;
HTREEITEM hFilterPuzzle			= NULL;
HTREEITEM hFilterQuiz			= NULL;
HTREEITEM hFilterSportsmisc		= NULL;
HTREEITEM hFilterSportsfootball 	= NULL;
HTREEITEM hFilterMisc			= NULL;
HTREEITEM hFilterMahjong		= NULL;
HTREEITEM hFilterRacing			= NULL;
HTREEITEM hFilterShoot			= NULL;
HTREEITEM hFilterOtherFamily		= NULL;
HTREEITEM hFilterMslug			= NULL;
HTREEITEM hFilterSf			= NULL;
HTREEITEM hFilterKof			= NULL;
HTREEITEM hFilterDstlk			= NULL;
HTREEITEM hFilterFatfury		= NULL;
HTREEITEM hFilterSamsho			= NULL;
HTREEITEM hFilter19xx			= NULL;
HTREEITEM hFilterSonicwi		= NULL;
HTREEITEM hFilterPwrinst		= NULL;

HTREEITEM hRoot				= NULL;
HTREEITEM hBoardType			= NULL;
HTREEITEM hFamily			= NULL;
HTREEITEM hGenre			= NULL;
HTREEITEM hHardware			= NULL;

// GCC doesn't seem to define these correctly.....
#define _TreeView_SetItemState(hwndTV, hti, data, _mask) \
{ TVITEM _ms_TVi;\
  _ms_TVi.mask = TVIF_STATE; \
  _ms_TVi.hItem = hti; \
  _ms_TVi.stateMask = _mask;\
  _ms_TVi.state = data;\
  SNDMSG((hwndTV), TVM_SETITEM, 0, (LPARAM)(TV_ITEM *)&_ms_TVi);\
}

#define _TreeView_SetCheckState(hwndTV, hti, fCheck) \
  _TreeView_SetItemState(hwndTV, hti, INDEXTOSTATEIMAGEMASK((fCheck)?2:1), TVIS_STATEIMAGEMASK)

// -----------------------------------------------------------------------------------------------------------------

#define DISABLE_NON_AVAILABLE_SELECT	0						// Disable selecting non-available sets
#define NON_WORKING_PROMPT_ON_LOAD		1						// Prompt user on loading non-working sets

#define MASKCPS			(1 << (HARDWARE_PREFIX_CAPCOM			>> 24))
#define MASKCPS2		(1 << (HARDWARE_PREFIX_CPS2				>> 24))
#define MASKCPS3		(1 << (HARDWARE_PREFIX_CPS3				>> 24))
#define MASKNEOGEO		(1 << (HARDWARE_PREFIX_SNK				>> 24))
#define MASKSEGA		(1 << (HARDWARE_PREFIX_SEGA				>> 24))
#define MASKTOAPLAN 	(1 << (HARDWARE_PREFIX_TOAPLAN			>> 24))
#define MASKCAVE		(1 << (HARDWARE_PREFIX_CAVE				>> 24))
#define MASKPGM			(1 << (HARDWARE_PREFIX_IGS_PGM			>> 24))
#define MASKMEGADRIVE	(1 << (HARDWARE_PREFIX_SEGA_MEGADRIVE   >> 24))
#define MASKTAITO		(1 << (HARDWARE_PREFIX_TAITO			>> 24))
#define MASKPSIKYO		(1 << (HARDWARE_PREFIX_PSIKYO			>> 24))
#define MASKKANEKO16	(1 << (HARDWARE_PREFIX_KANEKO16			>> 24))
#define MASKKONAMI		(1 << (HARDWARE_PREFIX_KONAMI			>> 24))
#define MASKPACMAN		(1 << (HARDWARE_PREFIX_PACMAN			>> 24))
#define MASKGALAXIAN		(1 << (HARDWARE_PREFIX_GALAXIAN			>> 24))
#define MASKATARI		(0x1000 * (HARDWARE_PREFIX_ATARI			>> 24))
#define MASKMISCPRE90S	(1 << (HARDWARE_PREFIX_MISC_PRE90S		>> 24))
#define MASKMISCPOST90S	(1 << (HARDWARE_PREFIX_MISC_POST90S		>> 24))
#define MASKALL			(MASKCPS | MASKCPS2 | MASKCPS3 | MASKNEOGEO | MASKSEGA | MASKTOAPLAN | MASKCAVE | MASKPGM | MASKTAITO | MASKPSIKYO | MASKKANEKO16 | MASKKONAMI | MASKPACMAN | MASKGALAXIAN | MASKATARI | MASKMEGADRIVE | MASKMISCPRE90S | MASKMISCPOST90S)

#define AVAILONLY		(1 << 18)
#define AUTOEXPAND		(1 << 19)
#define SHOWSHORT		(1 << 20)
#define ASCIIONLY		(1 << 21)

#define MASKBOARDTYPEGENUINE	(1)
#define MASKFAMILYOTHER		0x10000000

#define MASKALLGENRE		(GBF_HORSHOOT | GBF_VERSHOOT | GBF_SCRFIGHT | GBF_VSFIGHT | GBF_BIOS | GBF_BREAKOUT | GBF_CASINO | GBF_BALLPADDLE | GBF_MAZE | GBF_MINIGAMES | GBF_PINBALL | GBF_PLATFORM | GBF_PUZZLE | GBF_QUIZ | GBF_SPORTSMISC | GBF_SPORTSFOOTBALL | GBF_MISC | GBF_MAHJONG | GBF_RACING | GBF_SHOOT)
#define MASKALLFAMILY		(MASKFAMILYOTHER | FBF_MSLUG | FBF_SF | FBF_KOF | FBF_DSTLK | FBF_FATFURY | FBF_SAMSHO | FBF_19XX | FBF_SONICWI | FBF_PWRINST)
#define MASKALLBOARD		(MASKBOARDTYPEGENUINE | BDF_BOOTLEG | BDF_DEMO | BDF_HACK | BDF_HOMEBREW | BDF_PROTOTYPE)

int nLoadMenuShowX				= 0;
int nLoadMenuBoardTypeFilter	= 0;
int nLoadMenuGenreFilter		= 0;
int nLoadMenuFamilyFilter		= 0;

struct NODEINFO {
	int nBurnDrvNo;
	bool bIsParent;
	char* pszROMName;
	HTREEITEM hTreeHandle;
};

static NODEINFO* nBurnDrv;
static unsigned int nTmpDrvCount;

// Creates a tooltip for an item in a dialog box. (CaptainCPS-X)
void CreateToolTipForRect(HWND hwndParent, PTSTR pszText)
{
	// Check if we are running Windows XP/Vista/7
    bIsWindowsXPorGreater = DetectWindowsVersion();
   
	HWND hwndTT = NULL;

	// Create a tooltip.
	if(bIsWindowsXPorGreater) {
		hwndTT = CreateWindowEx( WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL,WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP | TTS_BALLOON, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hwndParent, NULL, hAppInst, NULL);
	} else {
		hwndTT = CreateWindowEx( WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL,WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hwndParent, NULL, hAppInst, NULL);
	}

    SetWindowPos(hwndTT, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

    // Set up "tool" information.
	TOOLINFO ti;
	memset(&ti, 0, sizeof(TOOLINFO));
	ti.cbSize   = sizeof(TOOLINFO);
    ti.uFlags   = TTF_SUBCLASS;
    ti.hwnd     = hwndParent;
    ti.hinst    = hAppInst;
    ti.lpszText = pszText;
    GetClientRect (hwndParent, &ti.rect); // rect of the button, window, etc...assosiated with the tooltip

    // Associate the tooltip with the "tool" window.
    SendMessage(hwndTT, TTM_ADDTOOL, 0, (LPARAM) (LPTOOLINFO) &ti);   
}

void CheckInfoButtons() {
	UINT idcButtons[7] = {IDC_VIEWGINFO_B, IDC_ADDFAV_B, IDC_DELFAV_B, IDC_JUKE_B, IDC_MAWS_B, IDC_CAESAR_B, IDC_PROGETTOEMMA_B};

	// nothing is selected
	if(!bDrvSelected && !bFavSelected) {
		for(int x = 0; x < 7; x++) {
			EnableWindow(GetDlgItem(hSelDlg, idcButtons[x]), false);	// disable all buttons
		}
	}

	if(bDrvSelected) {
		for(int x = 0; x < 7; x++) {
			EnableWindow(GetDlgItem(hSelDlg, idcButtons[x]), true);	// enable all buttons
		}

		if (!(BurnJukeboxGetFlags() & JBF_GAME_WORKING)) {
			EnableWindow(GetDlgItem(hSelDlg, IDC_JUKE_B), false);
		}

		EnableWindow(GetDlgItem(hSelDlg, IDC_DELFAV_B), false);

		if(bFavSelected) {
			EnableWindow(GetDlgItem(hSelDlg, IDC_DELFAV_B), true);
			EnableWindow(GetDlgItem(hSelDlg, IDC_ADDFAV_B), false);
		}
	}
}

// prototype  -----------------------
static void RebuildEverything();
// ----------------------------------

void SelDlgDisplayControls(int nTabSel) {
	
	// Gamelist
	if(nTabSel == 0) {
		
		RebuildEverything();

		ShowWindow(hFavListView,								SW_HIDE);					// Hide Favorites
		ShowWindow(GetDlgItem(hSelDlg, IDC_CHECKAVAILABLEONLY),	SW_HIDE);
		ShowWindow(GetDlgItem(hSelDlg, IDC_CHECKAUTOEXPAND),	SW_HIDE);
		ShowWindow(GetDlgItem(hSelDlg, IDC_SEL_SHORTNAME),		SW_HIDE);
		ShowWindow(GetDlgItem(hSelDlg, IDC_SEL_ASCIIONLY),		SW_HIDE);
		ShowWindow(GetDlgItem(hSelDlg, IDC_OPT_STATIC),			SW_HIDE);
		AnimateWindow(GetDlgItem(hSelDlg, IDC_TREE1), 400, AW_HOR_POSITIVE | SW_SHOW);		// Show Gamelist
		return;
	} 
	// Favorites
	if(nTabSel == 1) {
		if (bEnableIcons) SetFavoritesIcons();
		ShowWindow(GetDlgItem(hSelDlg, IDC_TREE1),				SW_HIDE);					// Hide Gamelist
		ShowWindow(GetDlgItem(hSelDlg, IDC_CHECKAVAILABLEONLY), SW_HIDE);
		ShowWindow(GetDlgItem(hSelDlg, IDC_CHECKAUTOEXPAND),	SW_HIDE);
		ShowWindow(GetDlgItem(hSelDlg, IDC_SEL_SHORTNAME),		SW_HIDE);
		ShowWindow(GetDlgItem(hSelDlg, IDC_SEL_ASCIIONLY),		SW_HIDE);
		ShowWindow(GetDlgItem(hSelDlg, IDC_OPT_STATIC),			SW_HIDE);
		RebuildEverything();
		AnimateWindow(hFavListView, 400, AW_HOR_POSITIVE | SW_SHOW);						// Show Favorites
		if(ListView_GetItemCount(hFavListView) != 0) {
			ListView_SetItemState(hFavListView, 0, LVIS_SELECTED | LVIS_FOCUSED, 0x000F);
		}
	}
	if(nTabSel == 2) {
		ShowWindow(GetDlgItem(hSelDlg, IDC_TREE1),				SW_HIDE);					// Hide Gamelist
		ShowWindow(hFavListView,								SW_HIDE);					// Hide Favorites
		ShowWindow(GetDlgItem(hSelDlg, IDC_OPT_STATIC),			SW_SHOW);
		ShowWindow(GetDlgItem(hSelDlg, IDC_CHECKAVAILABLEONLY), SW_SHOW);
		ShowWindow(GetDlgItem(hSelDlg, IDC_CHECKAUTOEXPAND),	SW_SHOW);
		ShowWindow(GetDlgItem(hSelDlg, IDC_SEL_SHORTNAME),		SW_SHOW);
		ShowWindow(GetDlgItem(hSelDlg, IDC_SEL_ASCIIONLY),		SW_SHOW);
	}
}

// Check if a specified driver is working
static bool CheckWorkingStatus(int nDriver)
{
	int nOldnBurnDrvSelect = nBurnDrvSelect;
	nBurnDrvSelect = nDriver;
	bool bStatus = BurnDrvIsWorking();
	nBurnDrvSelect = nOldnBurnDrvSelect;

	return bStatus;
}

static TCHAR* MangleGamename(const TCHAR* szOldName, bool /*bRemoveArticle*/)
{
	static TCHAR szNewName[256] = _T("");

#if 0
	TCHAR* pszName = szNewName;

	if (_tcsnicmp(szOldName, _T("the "), 4) == 0) {
		int x = 0, y = 0;
		while (szOldName[x] && szOldName[x] != _T('(') && szOldName[x] != _T('-')) {
			x++;
		}
		y = x;
		while (y && szOldName[y - 1] == _T(' ')) {
			y--;
		}
		_tcsncpy(pszName, szOldName + 4, y - 4);
		pszName[y - 4] = _T('\0');
		pszName += y - 4;

		if (!bRemoveArticle) {
			pszName += _stprintf(pszName, _T(", the"));
		}
		if (szOldName[x]) {
			_stprintf(pszName, _T(" %s"), szOldName + x);
		}
	} else {
		_tcscpy(pszName, szOldName);
	}
#endif

#if 1
	_tcscpy(szNewName, szOldName);
#endif	

	return szNewName;
}

static int DoExtraFilters()
{
	if (nShowMVSCartsOnly && ((BurnDrvGetHardwareCode() & HARDWARE_SNK_MVSCARTRIDGE) != HARDWARE_SNK_MVSCARTRIDGE)) return 1;

	if ((nLoadMenuBoardTypeFilter & BDF_BOOTLEG)	&& (BurnDrvGetFlags() & BDF_BOOTLEG))				return 1;
	if ((nLoadMenuBoardTypeFilter & BDF_DEMO)		&& (BurnDrvGetFlags() & BDF_DEMO))					return 1;
	if ((nLoadMenuBoardTypeFilter & BDF_HACK)		&& (BurnDrvGetFlags() & BDF_HACK))					return 1;
	if ((nLoadMenuBoardTypeFilter & BDF_HOMEBREW)	&& (BurnDrvGetFlags() & BDF_HOMEBREW))				return 1;
	if ((nLoadMenuBoardTypeFilter & BDF_PROTOTYPE)	&& (BurnDrvGetFlags() & BDF_PROTOTYPE))				return 1;
	
	if ((nLoadMenuBoardTypeFilter & MASKBOARDTYPEGENUINE)	&& (!(BurnDrvGetFlags() & BDF_BOOTLEG)) 
															&& (!(BurnDrvGetFlags() & BDF_DEMO)) 
															&& (!(BurnDrvGetFlags() & BDF_HACK)) 
															&& (!(BurnDrvGetFlags() & BDF_HOMEBREW)) 
															&& (!(BurnDrvGetFlags() & BDF_PROTOTYPE)))	return 1;
	
	if ((nLoadMenuFamilyFilter & FBF_MSLUG)			&& (BurnDrvGetFamilyFlags() & FBF_MSLUG))			return 1;
	if ((nLoadMenuFamilyFilter & FBF_SF)			&& (BurnDrvGetFamilyFlags() & FBF_SF))				return 1;
	if ((nLoadMenuFamilyFilter & FBF_KOF)			&& (BurnDrvGetFamilyFlags() & FBF_KOF))				return 1;
	if ((nLoadMenuFamilyFilter & FBF_DSTLK)			&& (BurnDrvGetFamilyFlags() & FBF_DSTLK))			return 1;
	if ((nLoadMenuFamilyFilter & FBF_FATFURY)		&& (BurnDrvGetFamilyFlags() & FBF_FATFURY))			return 1;
	if ((nLoadMenuFamilyFilter & FBF_SAMSHO)		&& (BurnDrvGetFamilyFlags() & FBF_SAMSHO))			return 1;
	if ((nLoadMenuFamilyFilter & FBF_19XX)			&& (BurnDrvGetFamilyFlags() & FBF_19XX))			return 1;
	if ((nLoadMenuFamilyFilter & FBF_SONICWI)		&& (BurnDrvGetFamilyFlags() & FBF_SONICWI))			return 1;
	if ((nLoadMenuFamilyFilter & FBF_PWRINST)		&& (BurnDrvGetFamilyFlags() & FBF_PWRINST))			return 1;
	
	if ((nLoadMenuFamilyFilter & MASKFAMILYOTHER)	&& (!(BurnDrvGetFamilyFlags() & FBF_MSLUG)) 
													&& (!(BurnDrvGetFamilyFlags() & FBF_SF)) 
													&& (!(BurnDrvGetFamilyFlags() & FBF_KOF)) 
													&& (!(BurnDrvGetFamilyFlags() & FBF_DSTLK)) 
													&& (!(BurnDrvGetFamilyFlags() & FBF_FATFURY)) 
													&& (!(BurnDrvGetFamilyFlags() & FBF_SAMSHO)) 
													&& (!(BurnDrvGetFamilyFlags() & FBF_19XX)) 
													&& (!(BurnDrvGetFamilyFlags() & FBF_SONICWI)) 
													&& (!(BurnDrvGetFamilyFlags() & FBF_PWRINST)))		return 1;
	
	if ((nLoadMenuGenreFilter & GBF_HORSHOOT)		&& (BurnDrvGetGenreFlags() & GBF_HORSHOOT))			return 1;
	if ((nLoadMenuGenreFilter & GBF_VERSHOOT)		&& (BurnDrvGetGenreFlags() & GBF_VERSHOOT))			return 1;
	if ((nLoadMenuGenreFilter & GBF_SCRFIGHT)		&& (BurnDrvGetGenreFlags() & GBF_SCRFIGHT))			return 1;
	if ((nLoadMenuGenreFilter & GBF_VSFIGHT)		&& (BurnDrvGetGenreFlags() & GBF_VSFIGHT))			return 1;
	if ((nLoadMenuGenreFilter & GBF_BIOS)			&& (BurnDrvGetGenreFlags() & GBF_BIOS))				return 1;
	if ((nLoadMenuGenreFilter & GBF_BREAKOUT)		&& (BurnDrvGetGenreFlags() & GBF_BREAKOUT))			return 1;
	if ((nLoadMenuGenreFilter & GBF_CASINO)			&& (BurnDrvGetGenreFlags() & GBF_CASINO))			return 1;
	if ((nLoadMenuGenreFilter & GBF_BALLPADDLE)		&& (BurnDrvGetGenreFlags() & GBF_BALLPADDLE))		return 1;
	if ((nLoadMenuGenreFilter & GBF_MAZE)			&& (BurnDrvGetGenreFlags() & GBF_MAZE))				return 1;
	if ((nLoadMenuGenreFilter & GBF_MINIGAMES)		&& (BurnDrvGetGenreFlags() & GBF_MINIGAMES))		return 1;
	if ((nLoadMenuGenreFilter & GBF_PINBALL)		&& (BurnDrvGetGenreFlags() & GBF_PINBALL))			return 1;
	if ((nLoadMenuGenreFilter & GBF_PLATFORM)		&& (BurnDrvGetGenreFlags() & GBF_PLATFORM))			return 1;
	if ((nLoadMenuGenreFilter & GBF_PUZZLE)			&& (BurnDrvGetGenreFlags() & GBF_PUZZLE))			return 1;
	if ((nLoadMenuGenreFilter & GBF_QUIZ)			&& (BurnDrvGetGenreFlags() & GBF_QUIZ))				return 1;
	if ((nLoadMenuGenreFilter & GBF_SPORTSMISC)		&& (BurnDrvGetGenreFlags() & GBF_SPORTSMISC))		return 1;
	if ((nLoadMenuGenreFilter & GBF_SPORTSFOOTBALL) && (BurnDrvGetGenreFlags() & GBF_SPORTSFOOTBALL))	return 1;
	if ((nLoadMenuGenreFilter & GBF_MISC)			&& (BurnDrvGetGenreFlags() & GBF_MISC))				return 1;
	if ((nLoadMenuGenreFilter & GBF_MAHJONG)		&& (BurnDrvGetGenreFlags() & GBF_MAHJONG))			return 1;
	if ((nLoadMenuGenreFilter & GBF_RACING)			&& (BurnDrvGetGenreFlags() & GBF_RACING))			return 1;
	if ((nLoadMenuGenreFilter & GBF_SHOOT)			&& (BurnDrvGetGenreFlags() & GBF_SHOOT))			return 1;
	
	return 0;
}

// Make a tree-view control with all drivers
static int SelListMake()
{
	unsigned int i, j;
	unsigned int nMissingDrvCount = 0;

	free(nBurnDrv);
	nBurnDrv = (NODEINFO*)malloc(nBurnDrvCount * sizeof(NODEINFO));
	memset(nBurnDrv, 0, nBurnDrvCount * sizeof(NODEINFO));

	nTmpDrvCount = 0;

	if (hSelList == NULL) {
		return 1;
	}

	// Add all the driver names to the list

	// 1st: parents
	for (i = 0; i < nBurnDrvCount; i++) {
		TV_INSERTSTRUCT TvItem;

		nBurnDrvSelect = i;																// Switch to driver i

		if (BurnDrvGetFlags() & BDF_BOARDROM) {
#if defined (INCLUDE_NEOGEO_MULTISLOT)
			if (_stricmp(BurnDrvGetTextA(DRV_NAME), "neogeo")) continue;
			if (nShowMVSCartsOnly) continue;
#else
			continue;
#endif
		}
		if (BurnDrvGetText(DRV_PARENT) != NULL && (BurnDrvGetFlags() & BDF_CLONE)) {	// Skip clones
			continue;
		}
		//if (avOk && (nLoadMenuShowX & AVAILONLY) && !gameAv[i])	{						// Skip non-available games if needed
		//	continue;
		//}

		int nHardware = 1 << (BurnDrvGetHardwareCode() >> 24);
		if ((BurnDrvGetHardwareCode() >> 24) == (HARDWARE_PREFIX_ATARI >> 24)) nHardware = MASKATARI;
		if ((nHardware & MASKALL) && ((nHardware & nLoadMenuShowX) || (nHardware & MASKALL) == 0)) {
			continue;
		}
		
		if (DoExtraFilters()) continue;

		if(!gameAv[i]) nMissingDrvCount++;

		if (avOk && (nLoadMenuShowX & AVAILONLY) && !gameAv[i])	{						// Skip non-available games if needed
			continue;
		}

		memset(&TvItem, 0, sizeof(TvItem));
		TvItem.item.mask = TVIF_TEXT | TVIF_PARAM;
		TvItem.hInsertAfter = TVI_SORT;
		TvItem.item.pszText = (nLoadMenuShowX & SHOWSHORT) ? BurnDrvGetText(DRV_NAME) : MangleGamename(BurnDrvGetText(DRV_ASCIIONLY | DRV_FULLNAME), true);
		TvItem.item.lParam = (LPARAM)&nBurnDrv[nTmpDrvCount];
		nBurnDrv[nTmpDrvCount].hTreeHandle = (HTREEITEM)SendMessage(hSelList, TVM_INSERTITEM, 0, (LPARAM)&TvItem);
		nBurnDrv[nTmpDrvCount].nBurnDrvNo = i;
		nBurnDrv[nTmpDrvCount].pszROMName = BurnDrvGetTextA(DRV_NAME);
		nBurnDrv[nTmpDrvCount].bIsParent = true;
		nTmpDrvCount++;
	}

	// 2nd: clones
	for (i = 0; i < nBurnDrvCount; i++) {
		TV_INSERTSTRUCT TvItem;

		nBurnDrvSelect = i;																// Switch to driver i

		if (BurnDrvGetFlags() & BDF_BOARDROM) {
			continue;
		}

		if (BurnDrvGetTextA(DRV_PARENT) == NULL || !(BurnDrvGetFlags() & BDF_CLONE)) {	// Skip parents
			continue;
		}
		//if (avOk && (nLoadMenuShowX & AVAILONLY) && !gameAv[i])	{						// Skip non-available games if needed
		//	continue;
		//}

		int nHardware = 1 << (BurnDrvGetHardwareCode() >> 24);
		if ((BurnDrvGetHardwareCode() >> 24) == (HARDWARE_PREFIX_ATARI >> 24)) nHardware = MASKATARI;
		if ((nHardware & MASKALL) && ((nHardware & nLoadMenuShowX) || ((nHardware & MASKALL) == 0))) {
			continue;
		}
		
		if (DoExtraFilters()) continue;

		if(!gameAv[i]) nMissingDrvCount++;

		if (avOk && (nLoadMenuShowX & AVAILONLY) && !gameAv[i])	{						// Skip non-available games if needed
			continue;
		}

		memset(&TvItem, 0, sizeof(TvItem));
		TvItem.item.mask = TVIF_TEXT | TVIF_PARAM;
		TvItem.hInsertAfter = TVI_SORT;
		TvItem.item.pszText = (nLoadMenuShowX & SHOWSHORT) ? BurnDrvGetText(DRV_NAME) : MangleGamename(BurnDrvGetText(DRV_ASCIIONLY | DRV_FULLNAME), true);

		// Find the parent's handle
		for (j = 0; j < nTmpDrvCount; j++) {
			if (nBurnDrv[j].bIsParent) {
				if (!_stricmp(BurnDrvGetTextA(DRV_PARENT), nBurnDrv[j].pszROMName)) {
					TvItem.hParent = nBurnDrv[j].hTreeHandle;
					break;
				}
			}
		}

		// Find the parent and add a branch to the tree
		if (!TvItem.hParent) {
			char szTempName[9];
			strcpy(szTempName, BurnDrvGetTextA(DRV_PARENT));
			int nTempBurnDrvSelect = nBurnDrvSelect;
			for (j = 0; j < nBurnDrvCount; j++) {
				nBurnDrvSelect = j;
				if (!strcmp(szTempName, BurnDrvGetTextA(DRV_NAME))) {
					TV_INSERTSTRUCT TempTvItem;
					memset(&TempTvItem, 0, sizeof(TempTvItem));
					TempTvItem.item.mask = TVIF_TEXT | TVIF_PARAM;
					TempTvItem.hInsertAfter = TVI_SORT;
					TempTvItem.item.pszText = (nLoadMenuShowX & SHOWSHORT) ? BurnDrvGetText(DRV_NAME) : MangleGamename(BurnDrvGetText(DRV_ASCIIONLY | DRV_FULLNAME), true);
					TempTvItem.item.lParam = (LPARAM)&nBurnDrv[nTmpDrvCount];
					nBurnDrv[nTmpDrvCount].hTreeHandle = (HTREEITEM)SendMessage(hSelList, TVM_INSERTITEM, 0, (LPARAM)&TempTvItem);
					nBurnDrv[nTmpDrvCount].nBurnDrvNo = j;
					nBurnDrv[nTmpDrvCount].bIsParent = true;
					nBurnDrv[nTmpDrvCount].pszROMName = BurnDrvGetTextA(DRV_NAME);
					TvItem.item.lParam = (LPARAM)&nBurnDrv[nTmpDrvCount];
					TvItem.hParent = nBurnDrv[nTmpDrvCount].hTreeHandle;
					nTmpDrvCount++;
					break;
				}
			}
			nBurnDrvSelect = nTempBurnDrvSelect;
		}

		TvItem.item.lParam = (LPARAM)&nBurnDrv[nTmpDrvCount];
		nBurnDrv[nTmpDrvCount].hTreeHandle = (HTREEITEM)SendMessage(hSelList, TVM_INSERTITEM, 0, (LPARAM)&TvItem);
		nBurnDrv[nTmpDrvCount].pszROMName = BurnDrvGetTextA(DRV_NAME);
		nBurnDrv[nTmpDrvCount].nBurnDrvNo = i;
		nTmpDrvCount++;
	}

	for (i = 0; i < nTmpDrvCount; i++) {

		// See if we need to expand the branch of an unavailable or non-working parent
		if (nBurnDrv[i].bIsParent && ((nLoadMenuShowX & AUTOEXPAND) || !gameAv[nBurnDrv[i].nBurnDrvNo] || !CheckWorkingStatus(nBurnDrv[i].nBurnDrvNo))) {
			for (j = 0; j < nTmpDrvCount; j++) {

				// Expand the branch only if a working clone is available
				if (gameAv[nBurnDrv[j].nBurnDrvNo]) {
					nBurnDrvSelect = nBurnDrv[j].nBurnDrvNo;
					if (BurnDrvGetTextA(DRV_PARENT)) {
						if (strcmp(nBurnDrv[i].pszROMName, BurnDrvGetTextA(DRV_PARENT)) == 0) {
							SendMessage(hSelList, TVM_EXPAND,TVE_EXPAND, (LPARAM)nBurnDrv[i].hTreeHandle);
							break;
						}
					}
				}
			}
		}
	}
	
	// Update the status info
	TCHAR szRomsAvailableInfo[128] = _T("");
	
	//_stprintf(szRomsAvailableInfo, _T("Showing %i of %i sets"), nTmpDrvCount, nBurnDrvCount - 2);
	//SendDlgItemMessage(hSelDlg, IDC_DRVCOUNT, WM_SETTEXT, 0, (LPARAM)(LPCTSTR)szRomsAvailableInfo);


	// If 'Show Available Only' option is enabled
	if (avOk && (nLoadMenuShowX & AVAILONLY))	{
		// total filtered drivers + total missing
		_stprintf(szRomsAvailableInfo, _T("Showing %i of %i sets [%i Unavailable sets] [%i Total sets]"), nTmpDrvCount, nTmpDrvCount + nMissingDrvCount, nMissingDrvCount, nBurnDrvCount);
	} else {
		_stprintf(szRomsAvailableInfo, _T("Showing %i of %i sets [%i Unavailable sets] [%i Total sets]"), nTmpDrvCount, nTmpDrvCount, nMissingDrvCount, nBurnDrvCount);
	}

	SendDlgItemMessage(hSelDlg, IDC_DRVCOUNT, WM_SETTEXT, 0, (LPARAM)(LPCTSTR)szRomsAvailableInfo);

	return 0;
}

static void MyEndDialog()
{
	if (nTimer) {
		KillTimer(hSelDlg, nTimer);
		nTimer = 0;
	}

	SendDlgItemMessage(hSelDlg, IDC_SCREENSHOT_H, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)NULL);
	SendDlgItemMessage(hSelDlg, IDC_SCREENSHOT_V, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)NULL);

	if (hPrevBmp) {
		DeleteObject((HGDIOBJ)hPrevBmp);
		hPrevBmp = NULL;
	}
	if (hPreview) {
		DeleteObject((HGDIOBJ)hPreview);
		hPreview = NULL;
	}

	if (hExpand) {
		DestroyIcon(hExpand);
		hExpand = NULL;
	}
	if (hCollapse) {
		DestroyIcon(hCollapse);
		hCollapse = NULL;
	}
	if (hNotWorking) {
		DestroyIcon(hNotWorking);
		hNotWorking = NULL;
	}
	if (hNotFoundEss) {
		DestroyIcon(hNotFoundEss);
		hNotFoundEss = NULL;
	}
	if (hNotFoundNonEss) {
		DestroyIcon(hNotFoundNonEss);
		hNotFoundNonEss = NULL;
	}
	if(hDrvIconMiss) {
		DestroyIcon(hDrvIconMiss); 
		hDrvIconMiss = NULL;
	}
	
	EndDialog(hSelDlg, 0);
}

// User clicked ok for a driver in the list
static void SelOkay()
{	
	if(bFavSelected == false)
	{
		TV_ITEM TvItem;
		unsigned int nSelect = 0;
		HTREEITEM hSelectHandle = (HTREEITEM)SendMessage(hSelList, TVM_GETNEXTITEM, TVGN_CARET, ~0U);
		
		if (!hSelectHandle)	{			// Nothing is selected, return without closing the window
			return;
		}

		TvItem.hItem = hSelectHandle;
		TvItem.mask = TVIF_PARAM;
		SendMessage(hSelList, TVM_GETITEM, 0, (LPARAM)&TvItem);
		nSelect = ((NODEINFO*)TvItem.lParam)->nBurnDrvNo;

	#if DISABLE_NON_AVAILABLE_SELECT
		if (!gameAv[nSelect]) {			// Game not available, return without closing the window
			return;
		}
	#endif

	#if NON_WORKING_PROMPT_ON_LOAD
		if (!CheckWorkingStatus(nSelect)) {
			if (MessageBox(hSelDlg, _T("This game isn't working. Load it anyway?"), _T("Warning!"), MB_YESNO | MB_DEFBUTTON2 | MB_ICONWARNING) == IDNO) {
				return;
			}
		}
	#endif
		nDialogSelect = nSelect;
	}

	bJukeboxInUse = false;
	bDialogCancel = false;
	MyEndDialog();
}

static void RefreshPanel()
{
	// clear preview shot
	if (hPrevBmp) {
		DeleteObject((HGDIOBJ)hPrevBmp);
		hPrevBmp = NULL;
	}
	if (nTimer) {
		KillTimer(hSelDlg, nTimer);
		nTimer = 0;
	}

	SendDlgItemMessage(hSelDlg, IDC_SCREENSHOT_H, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hPreview);
	SendDlgItemMessage(hSelDlg, IDC_SCREENSHOT_V, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)NULL);

	// Clear the things in our Info-box
	for (int i = 0; i < 6; i++) {
		SendMessage(hInfoText[i], WM_SETTEXT, (WPARAM)0, (LPARAM)_T(""));
		EnableWindow(hInfoLabel[i], FALSE);
	}

	CheckDlgButton(hSelDlg, IDC_CHECKAUTOEXPAND, (nLoadMenuShowX & AUTOEXPAND) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hSelDlg, IDC_CHECKAVAILABLEONLY, (nLoadMenuShowX & AVAILONLY) ? BST_CHECKED : BST_UNCHECKED);

	CheckDlgButton(hSelDlg, IDC_SEL_SHORTNAME, nLoadMenuShowX & SHOWSHORT ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hSelDlg, IDC_SEL_ASCIIONLY, nLoadMenuShowX & ASCIIONLY ? BST_CHECKED : BST_UNCHECKED);
}

static void RebuildEverything()
{
	RefreshPanel();

	bDrvSelected = false;

	TreeBuilding = 1;
	SendMessage(hSelList, WM_SETREDRAW, (WPARAM)FALSE,(LPARAM)TVI_ROOT);	// disable redraw
	SendMessage(hSelList, TVM_DELETEITEM, 0, (LPARAM)TVI_ROOT);				// Destory all nodes
	SelListMake();
	SendMessage(hSelList, WM_SETREDRAW, (WPARAM)TRUE, (LPARAM)TVI_ROOT);	// enable redraw

	// Clear the things in our Info-box
	for (int i = 0; i < 6; i++) {
		SendMessage(hInfoText[i], WM_SETTEXT, (WPARAM)0, (LPARAM)_T(""));
		EnableWindow(hInfoLabel[i], FALSE);
	}

	TreeBuilding = 0;

	CheckInfoButtons();
}

#define _TVCreateFiltersA(a, b, c, d)								\
{																	\
	TvItem.hParent = a;												\
	TvItem.item.pszText = FBALoadStringEx(hAppInst, b, true);		\
	c = TreeView_InsertItem(hFilterList, &TvItem);					\
	_TreeView_SetCheckState(hFilterList, c, (d) ? FALSE : TRUE);	\
}

#define _TVCreateFiltersB(a, b, c)									\
{																	\
	TvItem.hParent = a;												\
	TvItem.item.pszText = FBALoadStringEx(hAppInst, b, true);		\
	c = TreeView_InsertItem(hFilterList, &TvItem);					\
}

static void CreateFilters()
{
	TV_INSERTSTRUCT TvItem;	
	memset(&TvItem, 0, sizeof(TvItem));

	hFilterList			= GetDlgItem(hSelDlg, IDC_TREE2);	

	TvItem.item.mask	= TVIF_TEXT | TVIF_PARAM;
	TvItem.hInsertAfter = TVI_LAST;
	
	_TVCreateFiltersB(TVI_ROOT		, IDS_SEL_FILTERS		, hRoot			);	
	_TVCreateFiltersB(hRoot			, IDS_SEL_BOARDTYPE		, hBoardType	);

	_TVCreateFiltersA(hBoardType	, IDS_SEL_GENUINE		, hFilterGenuine		, nLoadMenuBoardTypeFilter & MASKBOARDTYPEGENUINE	);
	_TVCreateFiltersA(hBoardType	, IDS_SEL_BOOTLEG		, hFilterBootleg		, nLoadMenuBoardTypeFilter & BDF_BOOTLEG			);
	_TVCreateFiltersA(hBoardType	, IDS_SEL_DEMO			, hFilterDemo			, nLoadMenuBoardTypeFilter & BDF_DEMO				);
	_TVCreateFiltersA(hBoardType	, IDS_SEL_HACK			, hFilterHack			, nLoadMenuBoardTypeFilter & BDF_HACK				);
	_TVCreateFiltersA(hBoardType	, IDS_SEL_HOMEBREW		, hFilterHomebrew		, nLoadMenuBoardTypeFilter & BDF_HOMEBREW			);
	_TVCreateFiltersA(hBoardType	, IDS_SEL_PROTOTYPE		, hFilterPrototype		, nLoadMenuBoardTypeFilter & BDF_PROTOTYPE			);
	
	_TVCreateFiltersB(hRoot			, IDS_FAMILY			, hFamily		);
	
	_TVCreateFiltersA(hFamily		, IDS_FAMILY_OTHER		, hFilterOtherFamily	, nLoadMenuFamilyFilter & MASKFAMILYOTHER			);
	_TVCreateFiltersA(hFamily		, IDS_FAMILY_19XX		, hFilter19xx			, nLoadMenuFamilyFilter & FBF_19XX					);
	_TVCreateFiltersA(hFamily		, IDS_FAMILY_SONICWI	, hFilterSonicwi		, nLoadMenuFamilyFilter & FBF_SONICWI				);
	_TVCreateFiltersA(hFamily		, IDS_FAMILY_DSTLK		, hFilterDstlk			, nLoadMenuFamilyFilter & FBF_DSTLK					);
	_TVCreateFiltersA(hFamily		, IDS_FAMILY_FATFURY	, hFilterFatfury		, nLoadMenuFamilyFilter & FBF_FATFURY				);
	_TVCreateFiltersA(hFamily		, IDS_FAMILY_KOF		, hFilterKof			, nLoadMenuFamilyFilter & FBF_KOF					);
	_TVCreateFiltersA(hFamily		, IDS_FAMILY_MSLUG		, hFilterMslug			, nLoadMenuFamilyFilter & FBF_MSLUG					);
	_TVCreateFiltersA(hFamily		, IDS_FAMILY_PWRINST	, hFilterPwrinst		, nLoadMenuFamilyFilter & FBF_PWRINST				);
	_TVCreateFiltersA(hFamily		, IDS_FAMILY_SAMSHO		, hFilterSamsho			, nLoadMenuFamilyFilter & FBF_SAMSHO				);
	_TVCreateFiltersA(hFamily		, IDS_FAMILY_SF			, hFilterSf				, nLoadMenuFamilyFilter & FBF_SF					);
	
	_TVCreateFiltersB(hRoot			, IDS_GENRE				, hGenre		);
	
	_TVCreateFiltersA(hGenre		, IDS_GENRE_BALLPADDLE	, hFilterBallpaddle		, nLoadMenuGenreFilter & GBF_BALLPADDLE				);
	_TVCreateFiltersA(hGenre		, IDS_GENRE_BIOS		, hFilterBios			, nLoadMenuGenreFilter & GBF_BIOS					);
	_TVCreateFiltersA(hGenre		, IDS_GENRE_BREAKOUT	, hFilterBreakout		, nLoadMenuGenreFilter & GBF_BREAKOUT				);
	_TVCreateFiltersA(hGenre		, IDS_GENRE_CASINO		, hFilterCasino			, nLoadMenuGenreFilter & GBF_CASINO					);
	_TVCreateFiltersA(hGenre		, IDS_GENRE_SCRFIGHT	, hFilterScrfight		, nLoadMenuGenreFilter & GBF_SCRFIGHT				);
	_TVCreateFiltersA(hGenre		, IDS_GENRE_VSFIGHT		, hFilterVsfight		, nLoadMenuGenreFilter & GBF_VSFIGHT				);
	_TVCreateFiltersA(hGenre		, IDS_GENRE_MAHJONG		, hFilterMahjong		, nLoadMenuGenreFilter & GBF_MAHJONG				);
	_TVCreateFiltersA(hGenre		, IDS_GENRE_MAZE		, hFilterMaze			, nLoadMenuGenreFilter & GBF_MAZE					);
	_TVCreateFiltersA(hGenre		, IDS_GENRE_MINIGAMES	, hFilterMinigames		, nLoadMenuGenreFilter & GBF_MINIGAMES				);
	_TVCreateFiltersA(hGenre		, IDS_GENRE_MISC		, hFilterMisc			, nLoadMenuGenreFilter & GBF_MISC					);
	_TVCreateFiltersA(hGenre		, IDS_GENRE_PINBALL		, hFilterPinball		, nLoadMenuGenreFilter & GBF_PINBALL				);
	_TVCreateFiltersA(hGenre		, IDS_GENRE_PLATFORM	, hFilterPlatform		, nLoadMenuGenreFilter & GBF_PLATFORM				);
	_TVCreateFiltersA(hGenre		, IDS_GENRE_PUZZLE		, hFilterPuzzle			, nLoadMenuGenreFilter & GBF_PUZZLE					);
	_TVCreateFiltersA(hGenre		, IDS_GENRE_QUIZ		, hFilterQuiz			, nLoadMenuGenreFilter & GBF_QUIZ					);
	_TVCreateFiltersA(hGenre		, IDS_GENRE_RACING		, hFilterRacing			, nLoadMenuGenreFilter & GBF_RACING					);
	_TVCreateFiltersA(hGenre		, IDS_GENRE_HORSHOOT	, hFilterHorshoot		, nLoadMenuGenreFilter & GBF_HORSHOOT				);
	_TVCreateFiltersA(hGenre		, IDS_GENRE_SHOOT		, hFilterShoot			, nLoadMenuGenreFilter & GBF_SHOOT					);
	_TVCreateFiltersA(hGenre		, IDS_GENRE_VERSHOOT	, hFilterVershoot		, nLoadMenuGenreFilter & GBF_VERSHOOT				);
	_TVCreateFiltersA(hGenre		, IDS_GENRE_SPORTSMISC	, hFilterSportsmisc		, nLoadMenuGenreFilter & GBF_SPORTSMISC				);
	_TVCreateFiltersA(hGenre		, IDS_GENRE_SPORTSFOOTBALL, hFilterSportsfootball, nLoadMenuGenreFilter & GBF_SPORTSFOOTBALL		);

	_TVCreateFiltersB(hRoot			, IDS_SEL_HARDWARE, hHardware			);
	
	_TVCreateFiltersA(hHardware		, IDS_SEL_ATARI			, hFilterAtari			, nLoadMenuShowX & MASKATARI						);
	_TVCreateFiltersA(hHardware		, IDS_SEL_CAVE			, hFilterCave			, nLoadMenuShowX & MASKCAVE							);
	_TVCreateFiltersA(hHardware		, IDS_SEL_CPS1			, hFilterCps1			, nLoadMenuShowX & MASKCPS							);
	_TVCreateFiltersA(hHardware		, IDS_SEL_CPS2			, hFilterCps2			, nLoadMenuShowX & MASKCPS2							);
	_TVCreateFiltersA(hHardware		, IDS_SEL_CPS3			, hFilterCps3			, nLoadMenuShowX & MASKCPS3							);
	_TVCreateFiltersA(hHardware		, IDS_SEL_GALAXIAN		, hFilterGalaxian		, nLoadMenuShowX & MASKGALAXIAN						);
	_TVCreateFiltersA(hHardware		, IDS_SEL_KANEKO16		, hFilterKaneko16		, nLoadMenuShowX & MASKKANEKO16						);
	_TVCreateFiltersA(hHardware		, IDS_SEL_KONAMI		, hFilterKonami			, nLoadMenuShowX & MASKKONAMI						);
	_TVCreateFiltersA(hHardware		, IDS_SEL_NEOGEO		, hFilterNeogeo			, nLoadMenuShowX & MASKNEOGEO						);
	_TVCreateFiltersA(hHardware		, IDS_SEL_PACMAN		, hFilterPacman			, nLoadMenuShowX & MASKPACMAN						);
	_TVCreateFiltersA(hHardware		, IDS_SEL_PGM			, hFilterPgm			, nLoadMenuShowX & MASKPGM							);
	_TVCreateFiltersA(hHardware		, IDS_SEL_PSIKYO		, hFilterPsikyo			, nLoadMenuShowX & MASKPSIKYO						);
	_TVCreateFiltersA(hHardware		, IDS_SEL_SEGA			, hFilterSega			, nLoadMenuShowX & MASKSEGA							);
	_TVCreateFiltersA(hHardware		, IDS_SEL_TAITO			, hFilterTaito			, nLoadMenuShowX & MASKTAITO						);
	_TVCreateFiltersA(hHardware		, IDS_SEL_TOAPLAN		, hFilterToaplan		, nLoadMenuShowX & MASKTOAPLAN						);
	_TVCreateFiltersA(hHardware		, IDS_SEL_MISCPRE90S	, hFilterMiscPre90s		, nLoadMenuShowX & MASKMISCPRE90S					);
	_TVCreateFiltersA(hHardware		, IDS_SEL_MISCPOST90S	, hFilterMiscPost90s	, nLoadMenuShowX & MASKMISCPOST90S					);
	_TVCreateFiltersA(hHardware		, IDS_SEL_MEGADRIVE		, hFilterMegadrive		, nLoadMenuShowX & MASKMEGADRIVE					);
	
	SendMessage(hFilterList	, TVM_EXPAND,TVE_EXPAND, (LPARAM)hRoot);
	SendMessage(hFilterList	, TVM_EXPAND,TVE_EXPAND, (LPARAM)hHardware);
}

void LoadDrvIcons() 
{
	if(nIconsSize == ICON_16x16) {
		nIconsSizeXY	= 16;
		nIconsYDiff		= 4;
	}
	if(nIconsSize == ICON_24x24) {
		nIconsSizeXY	= 24;
		nIconsYDiff		= 8;
	}
	if(nIconsSize == ICON_32x32) {
		nIconsSizeXY	= 32;
		nIconsYDiff		= 12;
	}

	unsigned int nOldDrvSel = nBurnDrvSelect;

	for(unsigned int nDrvIndex = 0; nDrvIndex < nBurnDrvCount; nDrvIndex++) 
	{		
		nBurnDrvSelect = nDrvIndex;	
		TCHAR szIcon[MAX_PATH];

		_stprintf(szIcon, _T("%s%s.ico"), szAppIconsPath, BurnDrvGetText(DRV_NAME));
		hDrvIcon[nDrvIndex] = (HICON)LoadImage(hAppInst, szIcon, IMAGE_ICON, nIconsSizeXY, nIconsSizeXY, LR_LOADFROMFILE);
		
		if(!hDrvIcon[nDrvIndex] && BurnDrvGetText(DRV_PARENT)) {
			_stprintf(szIcon, _T("%s%s.ico"), szAppIconsPath, BurnDrvGetText(DRV_PARENT));
			hDrvIcon[nDrvIndex] = (HICON)LoadImage(hAppInst, szIcon, IMAGE_ICON, nIconsSizeXY, nIconsSizeXY, LR_LOADFROMFILE);
		}
	}

	nBurnDrvSelect = nOldDrvSel;
}

void UnloadDrvIcons() {
	
	nIconsSizeXY	= 16;
	nIconsYDiff		= 4;

	for(unsigned int nDrvIndex = 0; nDrvIndex < nBurnDrvCount; nDrvIndex++) 
	{		
		DestroyIcon(hDrvIcon[nDrvIndex]);
		hDrvIcon[nDrvIndex] = NULL;
	}
}

#define UM_CHECKSTATECHANGE (WM_USER + 100)
#define UM_CLOSE			(WM_USER + 101)

#define _ToggleGameListing(nShowX, nMASK)													\
{																							\
	nShowX ^= nMASK;																		\
	_TreeView_SetCheckState(hFilterList, hItemChanged, (nShowX & nMASK) ? FALSE : TRUE);	\
}																						

static BOOL CALLBACK DialogProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	if (Msg == WM_INITDIALOG) {

		InitCommonControls();

		hSelDlg = hDlg;

		SendDlgItemMessage(hDlg, IDC_SCREENSHOT_H, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)NULL);
		SendDlgItemMessage(hDlg, IDC_SCREENSHOT_V, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)NULL);
		
		hWhiteBGBrush	= CreateSolidBrush(RGB(0xFF,0xFF,0xFF));

		if (_tcslen(szPlaceHolder)) {
			LPTSTR p = _tcsrchr(szPlaceHolder, '.');
			if (!_tcsicmp(p+1, _T("bmp"))) {
				hPreview = (HBITMAP)LoadImage(hAppInst, szPlaceHolder, IMAGE_BITMAP, 304, 228, LR_LOADFROMFILE);
			} else {
				if (!_tcsicmp(p+1, _T("png"))) {
					FILE *fp = _tfopen(szPlaceHolder, _T("rb"));
					if (fp) {
						char szTemp[MAX_PATH];
						sprintf(szTemp, _TtoA(szPlaceHolder));
						hPreview = PNGtoBMP_Simple(hScrnWnd, szTemp);
						fclose(fp);
					}
				}
			}
		} else {
			hPreview = (HBITMAP)LoadImage(hAppInst, MAKEINTRESOURCE(BMP_PREVIEW), IMAGE_BITMAP, 304, 228, 0);
		}

		hExpand			= (HICON)LoadImage(hAppInst, MAKEINTRESOURCE(IDI_TV_PLUS),			IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
		hCollapse		= (HICON)LoadImage(hAppInst, MAKEINTRESOURCE(IDI_TV_MINUS),			IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);

		hNotWorking		= (HICON)LoadImage(hAppInst, MAKEINTRESOURCE(IDI_TV_NOTWORKING),	IMAGE_ICON, nIconsSizeXY, nIconsSizeXY, LR_DEFAULTCOLOR);
		hNotFoundEss	= (HICON)LoadImage(hAppInst, MAKEINTRESOURCE(IDI_TV_NOTFOUND_ESS),	IMAGE_ICON, nIconsSizeXY, nIconsSizeXY, LR_DEFAULTCOLOR);
		hNotFoundNonEss = (HICON)LoadImage(hAppInst, MAKEINTRESOURCE(IDI_TV_NOTFOUND_NON),	IMAGE_ICON, nIconsSizeXY, nIconsSizeXY, LR_DEFAULTCOLOR);
		
		hDrvIconMiss	= (HICON)LoadImage(hAppInst, MAKEINTRESOURCE(IDI_APP),	IMAGE_ICON, nIconsSizeXY, nIconsSizeXY, LR_DEFAULTCOLOR);

		TCHAR szOldTitle[1024] = _T(""), szNewTitle[1024] = _T("");
		GetWindowText(hSelDlg, szOldTitle, 1024);		
		_sntprintf(szNewTitle, 1024, _T(APP_TITLE) _T(SEPERATOR_1) _T("%s"), szOldTitle);		
		SetWindowText(hSelDlg, szNewTitle);

		hSelList		= GetDlgItem(hSelDlg, IDC_TREE1);

		hInfoLabel[0]	= GetDlgItem(hSelDlg, IDC_LABELROMNAME);
		hInfoLabel[1]	= GetDlgItem(hSelDlg, IDC_LABELROMINFO);
		hInfoLabel[2]	= GetDlgItem(hSelDlg, IDC_LABELSYSTEM);
		hInfoLabel[3]	= GetDlgItem(hSelDlg, IDC_LABELCOMMENT);
		hInfoLabel[4]	= GetDlgItem(hSelDlg, IDC_LABELNOTES);
		hInfoLabel[5]	= GetDlgItem(hSelDlg, IDC_LABELGENRE);
		hInfoText[0]	= GetDlgItem(hSelDlg, IDC_TEXTROMNAME);
		hInfoText[1]	= GetDlgItem(hSelDlg, IDC_TEXTROMINFO);
		hInfoText[2]	= GetDlgItem(hSelDlg, IDC_TEXTSYSTEM);
		hInfoText[3]	= GetDlgItem(hSelDlg, IDC_TEXTCOMMENT);
		hInfoText[4]	= GetDlgItem(hSelDlg, IDC_TEXTNOTES);
		hInfoText[5]	= GetDlgItem(hSelDlg, IDC_TEXTGENRE);

#if !defined _UNICODE
		EnableWindow(GetDlgItem(hDlg, IDC_SEL_ASCIIONLY), FALSE);
#endif

		bool bFoundROMs = false;
		for (unsigned int i = 0; i < nBurnDrvCount; i++) {
			if (gameAv[i]) {
				bFoundROMs = true;
				break;
			}
		}
		if (!bFoundROMs) {
			RomsDirCreate(hSelDlg);
		}

		// Icons size related -----------------------------------------
		SHORT cyItem = nIconsSizeXY + 4;								// height (in pixels) of each item on the TreeView list
		TreeView_SetItemHeight(hSelList, cyItem);

		RebuildEverything();

		if (nDialogSelect > -1) {
			for (unsigned int i = 0; i < nTmpDrvCount; i++) {
				if (nBurnDrv[i].nBurnDrvNo == nDialogSelect) {
					//SendMessage(hSelList, TVM_SELECTITEM, (WPARAM)TVGN_CARET | TVGN_FIRSTVISIBLE, (LPARAM)nBurnDrv[i].hTreeHandle);
					TreeView_Select(hSelList, nBurnDrv[i].hTreeHandle, TVGN_CARET);
					TreeView_EnsureVisible(hSelList, nBurnDrv[i].hTreeHandle);
					break;
				}
			}			
			//SendMessage(hDlg, WM_NEXTDLGCTL, (WPARAM)hSelList, TRUE);
		}

		SetFocus(hSelList);
		
		DWORD dwStyle;
		dwStyle = (DWORD) GetWindowLongPtr (GetDlgItem(hSelDlg, IDC_TREE2), GWL_STYLE);
		dwStyle |= TVS_CHECKBOXES;
		SetWindowLongPtr (GetDlgItem(hSelDlg, IDC_TREE2), GWL_STYLE, (LONG_PTR) dwStyle);
		
		CreateFilters();
		
		bDoPatch = FALSE;
		PatchExit();
		
		WndInMid(hDlg, hParent);

		ImageButton_EnableXPThemes();
		
		ImageButton_Create(hSelDlg, IDCANCEL);
		ImageButton_Create(hSelDlg, IDOK);
		ImageButton_Create(hSelDlg, IDROM);
		ImageButton_Create(hSelDlg, IDRESCAN);

		ImageButton_Create(hSelDlg, IDC_VIEWGINFO_B);
		ImageButton_Create(hSelDlg, IDC_ADDFAV_B);
		ImageButton_Create(hSelDlg, IDC_DELFAV_B);
		ImageButton_Create(hSelDlg, IDC_JUKE_B);
		ImageButton_Create(hSelDlg, IDC_MAWS_B);
		ImageButton_Create(hSelDlg, IDC_CAESAR_B);
		ImageButton_Create(hSelDlg, IDC_PROGETTOEMMA_B);

		ImageButton_SetIcon(GetDlgItem(hSelDlg, IDC_VIEWGINFO_B),		IDI_VIEWGINFO,		IDI_VIEWGINFO_GRAY,		0,	28,24);
		ImageButton_SetIcon(GetDlgItem(hSelDlg, IDC_ADDFAV_B),			IDI_ADDFAV,			IDI_ADDFAV_GRAY,		0,	28,24);
		ImageButton_SetIcon(GetDlgItem(hSelDlg, IDC_DELFAV_B),			IDI_DELFAV,			IDI_DELFAV_GRAY,		0,	28,24);
		ImageButton_SetIcon(GetDlgItem(hSelDlg, IDC_JUKE_B),			IDI_JUKE,			IDI_JUKE_GRAY,			0,	28,24);
		ImageButton_SetIcon(GetDlgItem(hSelDlg, IDC_MAWS_B),			IDI_MAWS,			IDI_MAWS_GRAY,			0,	28,24);
		ImageButton_SetIcon(GetDlgItem(hSelDlg, IDC_CAESAR_B),			IDI_CAESAR,			IDI_CAESAR_GRAY,		0,	28,24);
		ImageButton_SetIcon(GetDlgItem(hSelDlg, IDC_PROGETTOEMMA_B),	IDI_PROGETTOEMMA,	IDI_PROGETTOEMMA_GRAY,	0,	28,24);

		ImageButton_SetIcon(GetDlgItem(hSelDlg, IDCANCEL),	IDI_CANCEL,		0,0,16,16);
		ImageButton_SetIcon(GetDlgItem(hSelDlg, IDOK),		IDI_PLAY,		0,0,16,16);
		ImageButton_SetIcon(GetDlgItem(hSelDlg, IDROM),		IDI_SCAN,		0,0,16,16);
		ImageButton_SetIcon(GetDlgItem(hSelDlg, IDRESCAN),	IDI_ROMDIRS,	0,0,16,16);

		HICON hIcon = LoadIcon(hAppInst, MAKEINTRESOURCE(IDI_APP));
		SendMessage(hSelDlg, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);		// Set the Game Selection dialog icon.
		
		hSelDlgTabControl	= GetDlgItem(hSelDlg, IDC_TAB3);					// GameList & Favorite games list tabs control
		hFavListView		= GetDlgItem(hSelDlg, IDC_FAVLIST);				// Favorite games 'list view' control 		
		InsertTabs();														// Insert Game list & Favorites tabs
		InitFavGameList();													// Initiate Favorites Module

		ShowWindow(hFavListView, SW_HIDE);									// Hide Favorites Gamelist	
		ShowWindow(GetDlgItem(hSelDlg, IDC_CHECKAVAILABLEONLY), SW_HIDE);
		ShowWindow(GetDlgItem(hSelDlg, IDC_CHECKAUTOEXPAND),	SW_HIDE);
		ShowWindow(GetDlgItem(hSelDlg, IDC_SEL_SHORTNAME),		SW_HIDE);
		ShowWindow(GetDlgItem(hSelDlg, IDC_SEL_ASCIIONLY),		SW_HIDE);
		ShowWindow(GetDlgItem(hSelDlg, IDC_OPT_STATIC),			SW_HIDE);

		// ------------------------------------------------------------
		// Loading / Unloading of driver icons is done in main.cpp now
		/* LoadDrvIcons(); */

		CheckInfoButtons();
		
		// Init tooltips for the Select Dialog buttons
		CreateToolTipForRect(GetDlgItem(hSelDlg, IDC_VIEWGINFO_B), FBALoadStringEx(hAppInst, IDS_VIEW_GAME_INFO, true));
      	CreateToolTipForRect(GetDlgItem(hSelDlg, IDC_ADDFAV_B), FBALoadStringEx(hAppInst, IDS_ADD_FAVORITE, true));
      	CreateToolTipForRect(GetDlgItem(hSelDlg, IDC_DELFAV_B), FBALoadStringEx(hAppInst, IDS_DEL_FAVORITE, true));
      	CreateToolTipForRect(GetDlgItem(hSelDlg, IDC_JUKE_B), FBALoadStringEx(hAppInst, IDS_JUKEBOX, true));
      	CreateToolTipForRect(GetDlgItem(hSelDlg, IDC_MAWS_B), FBALoadStringEx(hAppInst, IDS_MAWS, true));
      	CreateToolTipForRect(GetDlgItem(hSelDlg, IDC_CAESAR_B), FBALoadStringEx(hAppInst, IDS_CAESAR, true));
      	CreateToolTipForRect(GetDlgItem(hSelDlg, IDC_PROGETTOEMMA_B), FBALoadStringEx(hAppInst, IDS_PROGETTOEMMA, true));

		return TRUE;
	}

	if(Msg == UM_CHECKSTATECHANGE) {

		HTREEITEM   hItemChanged = (HTREEITEM)lParam;
		
		if (hItemChanged == hHardware) {
			if ((nLoadMenuShowX & MASKALL) == 0) {
				_TreeView_SetCheckState(hFilterList, hItemChanged, FALSE);
			
				_TreeView_SetCheckState(hFilterList, hFilterAtari, FALSE);
				_TreeView_SetCheckState(hFilterList, hFilterCave, FALSE);
				_TreeView_SetCheckState(hFilterList, hFilterCps1, FALSE);
				_TreeView_SetCheckState(hFilterList, hFilterCps2, FALSE);
				_TreeView_SetCheckState(hFilterList, hFilterCps3, FALSE);
				_TreeView_SetCheckState(hFilterList, hFilterGalaxian, FALSE);
				_TreeView_SetCheckState(hFilterList, hFilterKaneko16, FALSE);
				_TreeView_SetCheckState(hFilterList, hFilterKonami, FALSE);
				_TreeView_SetCheckState(hFilterList, hFilterNeogeo, FALSE);
				_TreeView_SetCheckState(hFilterList, hFilterPacman, FALSE);
				_TreeView_SetCheckState(hFilterList, hFilterPgm, FALSE);
				_TreeView_SetCheckState(hFilterList, hFilterPsikyo, FALSE);
				_TreeView_SetCheckState(hFilterList, hFilterSega, FALSE);
				_TreeView_SetCheckState(hFilterList, hFilterToaplan, FALSE);
				_TreeView_SetCheckState(hFilterList, hFilterTaito, FALSE);
				_TreeView_SetCheckState(hFilterList, hFilterMiscPre90s, FALSE);
				_TreeView_SetCheckState(hFilterList, hFilterMiscPost90s, FALSE);
				_TreeView_SetCheckState(hFilterList, hFilterMegadrive, FALSE);
				
				nLoadMenuShowX |= MASKALL;
			} else {
				_TreeView_SetCheckState(hFilterList, hItemChanged, TRUE);
			
				_TreeView_SetCheckState(hFilterList, hFilterAtari, TRUE);
				_TreeView_SetCheckState(hFilterList, hFilterCave, TRUE);
				_TreeView_SetCheckState(hFilterList, hFilterCps1, TRUE);
				_TreeView_SetCheckState(hFilterList, hFilterCps2, TRUE);
				_TreeView_SetCheckState(hFilterList, hFilterCps3, TRUE);
				_TreeView_SetCheckState(hFilterList, hFilterGalaxian, TRUE);
				_TreeView_SetCheckState(hFilterList, hFilterKaneko16, TRUE);
				_TreeView_SetCheckState(hFilterList, hFilterKonami, TRUE);
				_TreeView_SetCheckState(hFilterList, hFilterNeogeo, TRUE);
				_TreeView_SetCheckState(hFilterList, hFilterPacman, TRUE);
				_TreeView_SetCheckState(hFilterList, hFilterPgm, TRUE);
				_TreeView_SetCheckState(hFilterList, hFilterPsikyo, TRUE);
				_TreeView_SetCheckState(hFilterList, hFilterSega, TRUE);
				_TreeView_SetCheckState(hFilterList, hFilterToaplan, TRUE);
				_TreeView_SetCheckState(hFilterList, hFilterTaito, TRUE);
				_TreeView_SetCheckState(hFilterList, hFilterMiscPre90s, TRUE);
				_TreeView_SetCheckState(hFilterList, hFilterMiscPost90s, TRUE);
				_TreeView_SetCheckState(hFilterList, hFilterMegadrive, TRUE);
				
				nLoadMenuShowX &= 0xfffc0000;
			}
		}
		
		if (hItemChanged == hBoardType) {
			if ((nLoadMenuBoardTypeFilter & MASKALLBOARD) == 0) {
				_TreeView_SetCheckState(hFilterList, hItemChanged, FALSE);
			
				_TreeView_SetCheckState(hFilterList, hFilterBootleg, FALSE);
				_TreeView_SetCheckState(hFilterList, hFilterDemo, FALSE);
				_TreeView_SetCheckState(hFilterList, hFilterHack, FALSE);
				_TreeView_SetCheckState(hFilterList, hFilterHomebrew, FALSE);
				_TreeView_SetCheckState(hFilterList, hFilterPrototype, FALSE);
				_TreeView_SetCheckState(hFilterList, hFilterGenuine, FALSE);
				
				nLoadMenuBoardTypeFilter = MASKALLBOARD;
			} else {
				_TreeView_SetCheckState(hFilterList, hItemChanged, TRUE);
			
				_TreeView_SetCheckState(hFilterList, hFilterBootleg, TRUE);
				_TreeView_SetCheckState(hFilterList, hFilterDemo, TRUE);
				_TreeView_SetCheckState(hFilterList, hFilterHack, TRUE);
				_TreeView_SetCheckState(hFilterList, hFilterHomebrew, TRUE);
				_TreeView_SetCheckState(hFilterList, hFilterPrototype, TRUE);
				_TreeView_SetCheckState(hFilterList, hFilterGenuine, TRUE);
				
				nLoadMenuBoardTypeFilter = 0;
			}
		}
		
		if (hItemChanged == hFamily) {
			if ((nLoadMenuFamilyFilter & MASKALLFAMILY) == 0) {
				_TreeView_SetCheckState(hFilterList, hItemChanged, FALSE);
			
				_TreeView_SetCheckState(hFilterList, hFilterOtherFamily, FALSE);	
				_TreeView_SetCheckState(hFilterList, hFilterMslug, FALSE);	
				_TreeView_SetCheckState(hFilterList, hFilterSf, FALSE);	
				_TreeView_SetCheckState(hFilterList, hFilterKof, FALSE);	
				_TreeView_SetCheckState(hFilterList, hFilterDstlk, FALSE);	
				_TreeView_SetCheckState(hFilterList, hFilterFatfury, FALSE);	
				_TreeView_SetCheckState(hFilterList, hFilterSamsho, FALSE);	
				_TreeView_SetCheckState(hFilterList, hFilter19xx, FALSE);
				_TreeView_SetCheckState(hFilterList, hFilterSonicwi, FALSE);
				_TreeView_SetCheckState(hFilterList, hFilterPwrinst, FALSE);
				
				nLoadMenuFamilyFilter = MASKALLFAMILY;
			} else {
				_TreeView_SetCheckState(hFilterList, hItemChanged, TRUE);
			
				_TreeView_SetCheckState(hFilterList, hFilterOtherFamily, TRUE);	
				_TreeView_SetCheckState(hFilterList, hFilterMslug, TRUE);	
				_TreeView_SetCheckState(hFilterList, hFilterSf, TRUE);	
				_TreeView_SetCheckState(hFilterList, hFilterKof, TRUE);	
				_TreeView_SetCheckState(hFilterList, hFilterDstlk, TRUE);	
				_TreeView_SetCheckState(hFilterList, hFilterFatfury, TRUE);	
				_TreeView_SetCheckState(hFilterList, hFilterSamsho, TRUE);	
				_TreeView_SetCheckState(hFilterList, hFilter19xx, TRUE);
				_TreeView_SetCheckState(hFilterList, hFilterSonicwi, TRUE);
				_TreeView_SetCheckState(hFilterList, hFilterPwrinst, TRUE);
				
				nLoadMenuFamilyFilter = 0;
			}
		}
		
		if (hItemChanged == hGenre) {
			if ((nLoadMenuGenreFilter & MASKALLGENRE) == 0) {
				_TreeView_SetCheckState(hFilterList, hItemChanged, FALSE);
			
				_TreeView_SetCheckState(hFilterList, hFilterHorshoot, FALSE);	
				_TreeView_SetCheckState(hFilterList, hFilterVershoot, FALSE);	
				_TreeView_SetCheckState(hFilterList, hFilterScrfight, FALSE);	
				_TreeView_SetCheckState(hFilterList, hFilterVsfight, FALSE);	
				_TreeView_SetCheckState(hFilterList, hFilterBios, FALSE);	
				_TreeView_SetCheckState(hFilterList, hFilterBreakout, FALSE);	
				_TreeView_SetCheckState(hFilterList, hFilterCasino, FALSE);	
				_TreeView_SetCheckState(hFilterList, hFilterBallpaddle, FALSE);
				_TreeView_SetCheckState(hFilterList, hFilterMaze, FALSE);
				_TreeView_SetCheckState(hFilterList, hFilterMinigames, FALSE);
				_TreeView_SetCheckState(hFilterList, hFilterPinball, FALSE);
				_TreeView_SetCheckState(hFilterList, hFilterPlatform, FALSE);
				_TreeView_SetCheckState(hFilterList, hFilterPuzzle, FALSE);
				_TreeView_SetCheckState(hFilterList, hFilterQuiz, FALSE);
				_TreeView_SetCheckState(hFilterList, hFilterSportsmisc, FALSE);
				_TreeView_SetCheckState(hFilterList, hFilterSportsfootball, FALSE);
				_TreeView_SetCheckState(hFilterList, hFilterMisc, FALSE);
				_TreeView_SetCheckState(hFilterList, hFilterMahjong, FALSE);
				_TreeView_SetCheckState(hFilterList, hFilterRacing, FALSE);
				_TreeView_SetCheckState(hFilterList, hFilterShoot, FALSE);
				
				nLoadMenuGenreFilter = MASKALLGENRE;
			} else {
				_TreeView_SetCheckState(hFilterList, hItemChanged, TRUE);
			
				_TreeView_SetCheckState(hFilterList, hFilterHorshoot, TRUE);	
				_TreeView_SetCheckState(hFilterList, hFilterVershoot, TRUE);	
				_TreeView_SetCheckState(hFilterList, hFilterScrfight, TRUE);	
				_TreeView_SetCheckState(hFilterList, hFilterVsfight, TRUE);	
				_TreeView_SetCheckState(hFilterList, hFilterBios, TRUE);	
				_TreeView_SetCheckState(hFilterList, hFilterBreakout, TRUE);	
				_TreeView_SetCheckState(hFilterList, hFilterCasino, TRUE);	
				_TreeView_SetCheckState(hFilterList, hFilterBallpaddle, TRUE);
				_TreeView_SetCheckState(hFilterList, hFilterMaze, TRUE);
				_TreeView_SetCheckState(hFilterList, hFilterMinigames, TRUE);
				_TreeView_SetCheckState(hFilterList, hFilterPinball, TRUE);
				_TreeView_SetCheckState(hFilterList, hFilterPlatform, TRUE);
				_TreeView_SetCheckState(hFilterList, hFilterPuzzle, TRUE);
				_TreeView_SetCheckState(hFilterList, hFilterQuiz, TRUE);
				_TreeView_SetCheckState(hFilterList, hFilterSportsmisc, TRUE);
				_TreeView_SetCheckState(hFilterList, hFilterSportsfootball, TRUE);
				_TreeView_SetCheckState(hFilterList, hFilterMisc, TRUE);
				_TreeView_SetCheckState(hFilterList, hFilterMahjong, TRUE);
				_TreeView_SetCheckState(hFilterList, hFilterRacing, TRUE);
				_TreeView_SetCheckState(hFilterList, hFilterShoot, TRUE);
				
				nLoadMenuGenreFilter = 0;
			}
		}

		if (hItemChanged == hFilterAtari)			_ToggleGameListing(nLoadMenuShowX, MASKATARI);
		if (hItemChanged == hFilterCave)			_ToggleGameListing(nLoadMenuShowX, MASKCAVE);
		if (hItemChanged == hFilterCps1)			_ToggleGameListing(nLoadMenuShowX, MASKCPS);
		if (hItemChanged == hFilterCps2)			_ToggleGameListing(nLoadMenuShowX, MASKCPS2);
		if (hItemChanged == hFilterCps3)			_ToggleGameListing(nLoadMenuShowX, MASKCPS3);
		if (hItemChanged == hFilterGalaxian)			_ToggleGameListing(nLoadMenuShowX, MASKGALAXIAN);
		if (hItemChanged == hFilterKaneko16)			_ToggleGameListing(nLoadMenuShowX, MASKKANEKO16);
		if (hItemChanged == hFilterKonami)			_ToggleGameListing(nLoadMenuShowX, MASKKONAMI);
		if (hItemChanged == hFilterNeogeo)			_ToggleGameListing(nLoadMenuShowX, MASKNEOGEO);
		if (hItemChanged == hFilterPacman)			_ToggleGameListing(nLoadMenuShowX, MASKPACMAN);
		if (hItemChanged == hFilterPgm)				_ToggleGameListing(nLoadMenuShowX, MASKPGM);
		if (hItemChanged == hFilterPsikyo)			_ToggleGameListing(nLoadMenuShowX, MASKPSIKYO);
		if (hItemChanged == hFilterSega)			_ToggleGameListing(nLoadMenuShowX, MASKSEGA);
		if (hItemChanged == hFilterToaplan)			_ToggleGameListing(nLoadMenuShowX, MASKTOAPLAN);
		if (hItemChanged == hFilterTaito)			_ToggleGameListing(nLoadMenuShowX, MASKTAITO);
		if (hItemChanged == hFilterMiscPre90s)			_ToggleGameListing(nLoadMenuShowX, MASKMISCPRE90S);
		if (hItemChanged == hFilterMiscPost90s)			_ToggleGameListing(nLoadMenuShowX, MASKMISCPOST90S);
		if (hItemChanged == hFilterMegadrive)			_ToggleGameListing(nLoadMenuShowX, MASKMEGADRIVE);
		
		if (hItemChanged == hFilterBootleg)			_ToggleGameListing(nLoadMenuBoardTypeFilter, BDF_BOOTLEG);
		if (hItemChanged == hFilterDemo)			_ToggleGameListing(nLoadMenuBoardTypeFilter, BDF_DEMO);
		if (hItemChanged == hFilterHack)			_ToggleGameListing(nLoadMenuBoardTypeFilter, BDF_HACK);
		if (hItemChanged == hFilterHomebrew)			_ToggleGameListing(nLoadMenuBoardTypeFilter, BDF_HOMEBREW);
		if (hItemChanged == hFilterPrototype)			_ToggleGameListing(nLoadMenuBoardTypeFilter, BDF_PROTOTYPE);
		if (hItemChanged == hFilterGenuine)			_ToggleGameListing(nLoadMenuBoardTypeFilter, MASKBOARDTYPEGENUINE);
		
		if (hItemChanged == hFilterOtherFamily)			_ToggleGameListing(nLoadMenuFamilyFilter, MASKFAMILYOTHER);
		if (hItemChanged == hFilterMslug)			_ToggleGameListing(nLoadMenuFamilyFilter, FBF_MSLUG);
		if (hItemChanged == hFilterSf)				_ToggleGameListing(nLoadMenuFamilyFilter, FBF_SF);
		if (hItemChanged == hFilterKof)				_ToggleGameListing(nLoadMenuFamilyFilter, FBF_KOF);
		if (hItemChanged == hFilterDstlk)			_ToggleGameListing(nLoadMenuFamilyFilter, FBF_DSTLK);
		if (hItemChanged == hFilterFatfury)			_ToggleGameListing(nLoadMenuFamilyFilter, FBF_FATFURY);
		if (hItemChanged == hFilterSamsho)			_ToggleGameListing(nLoadMenuFamilyFilter, FBF_SAMSHO);
		if (hItemChanged == hFilter19xx)			_ToggleGameListing(nLoadMenuFamilyFilter, FBF_19XX);
		if (hItemChanged == hFilterSonicwi)			_ToggleGameListing(nLoadMenuFamilyFilter, FBF_SONICWI);
		if (hItemChanged == hFilterPwrinst)			_ToggleGameListing(nLoadMenuFamilyFilter, FBF_PWRINST);
		
		if (hItemChanged == hFilterHorshoot)			_ToggleGameListing(nLoadMenuGenreFilter, GBF_HORSHOOT);
		if (hItemChanged == hFilterVershoot)			_ToggleGameListing(nLoadMenuGenreFilter, GBF_VERSHOOT);
		if (hItemChanged == hFilterScrfight)			_ToggleGameListing(nLoadMenuGenreFilter, GBF_SCRFIGHT);
		if (hItemChanged == hFilterVsfight)			_ToggleGameListing(nLoadMenuGenreFilter, GBF_VSFIGHT);
		if (hItemChanged == hFilterBios)			_ToggleGameListing(nLoadMenuGenreFilter, GBF_BIOS);
		if (hItemChanged == hFilterBreakout)			_ToggleGameListing(nLoadMenuGenreFilter, GBF_BREAKOUT);
		if (hItemChanged == hFilterCasino)			_ToggleGameListing(nLoadMenuGenreFilter, GBF_CASINO);
		if (hItemChanged == hFilterBallpaddle)			_ToggleGameListing(nLoadMenuGenreFilter, GBF_BALLPADDLE);
		if (hItemChanged == hFilterMaze)			_ToggleGameListing(nLoadMenuGenreFilter, GBF_MAZE);
		if (hItemChanged == hFilterMinigames)			_ToggleGameListing(nLoadMenuGenreFilter, GBF_MINIGAMES);
		if (hItemChanged == hFilterPinball)			_ToggleGameListing(nLoadMenuGenreFilter, GBF_PINBALL);
		if (hItemChanged == hFilterPlatform)			_ToggleGameListing(nLoadMenuGenreFilter, GBF_PLATFORM);
		if (hItemChanged == hFilterPuzzle)			_ToggleGameListing(nLoadMenuGenreFilter, GBF_PUZZLE);
		if (hItemChanged == hFilterQuiz)			_ToggleGameListing(nLoadMenuGenreFilter, GBF_QUIZ);
		if (hItemChanged == hFilterSportsmisc)			_ToggleGameListing(nLoadMenuGenreFilter, GBF_SPORTSMISC);
		if (hItemChanged == hFilterSportsfootball)		_ToggleGameListing(nLoadMenuGenreFilter, GBF_SPORTSFOOTBALL);
		if (hItemChanged == hFilterMisc)			_ToggleGameListing(nLoadMenuGenreFilter, GBF_MISC);
		if (hItemChanged == hFilterMahjong)			_ToggleGameListing(nLoadMenuGenreFilter, GBF_MAHJONG);
		if (hItemChanged == hFilterRacing)			_ToggleGameListing(nLoadMenuGenreFilter, GBF_RACING);
		if (hItemChanged == hFilterShoot)			_ToggleGameListing(nLoadMenuGenreFilter, GBF_SHOOT);
		
		RebuildEverything();	
	}

	if (Msg == WM_COMMAND) {
		if (HIWORD(wParam) == BN_CLICKED) {
			int wID = LOWORD(wParam);
			switch (wID) {
				case IDOK:
					SelOkay();
					break;
				case IDROM:
					RomsDirCreate(hSelDlg);
					RebuildEverything();
					break;
				case IDRESCAN:
					bRescanRoms = true;
					CreateROMInfo(hSelDlg);
					RebuildEverything();
					break;
				case IDCANCEL:
					bDialogCancel = true;
					SendMessage(hDlg, WM_CLOSE, 0, 0);
					return 0;
				case IDC_CHECKAVAILABLEONLY:
					nLoadMenuShowX ^= AVAILONLY;
					RebuildEverything();
					break;
				case IDC_CHECKAUTOEXPAND:
					nLoadMenuShowX ^= AUTOEXPAND;
					RebuildEverything();
					break;
				case IDC_SEL_SHORTNAME:
					nLoadMenuShowX ^= SHOWSHORT;
					RebuildEverything();
					break;
				case IDC_SEL_ASCIIONLY:
					nLoadMenuShowX ^= ASCIIONLY;
					RebuildEverything();
					break;

				case IDC_ADDFAV_B:	{
					if(!bFavSelected) {
						AddToFavorites();
						RefreshFavGameList();
						//SelDlgDisplayControls(1);
					}
					break;
				}
				case IDC_DELFAV_B: {
					if(bFavSelected) {
						RemoveFromFavorites();
						RefreshFavGameList();
						SetFocus(hSelDlgTabControl);
						SelDlgDisplayControls(1);
					}
					break;
				}
				case IDC_VIEWGINFO_B: {
					if (nTimer) {
						KillTimer(hSelDlg, nTimer);
						nTimer = 0;
					}
					GameInfoDialogCreate(hSelDlg, nBurnDrvSelect);
					break;
				}				
				case IDC_JUKE_B: {
					if (BurnJukeboxGetFlags() & JBF_GAME_WORKING) {
						SelOkay();
						bJukeboxInUse = true;
					}
					break;
				}				
				case IDC_MAWS_B: {
					if (!nVidFullscreen) {
						TCHAR szURL[MAX_PATH];
						_stprintf(szURL, _T("http://maws.mameworld.info/maws/set/%s/"), BurnDrvGetText(DRV_NAME));
						bprintf(PRINT_NORMAL, _T("%s\n"), szURL);
						ShellExecute(NULL, _T("open"), szURL, NULL, NULL, SW_SHOWNORMAL);
						break;
					}
				}				
				case IDC_CAESAR_B: {
					if (!nVidFullscreen) {
						//YES
						TCHAR szURL[MAX_PATH];
						_stprintf(szURL, _T("http://caesar.logiqx.com/php/emulator_game.php?id=finalburnalpha&game=%s"), BurnDrvGetText(DRV_NAME));
						ShellExecute(NULL, _T("open"), szURL, NULL, NULL, SW_SHOWNORMAL);
						break;
					}
				}				
				case IDC_PROGETTOEMMA_B: {
					if (!nVidFullscreen) {
						//YES
						TCHAR szURL[MAX_PATH];
						_stprintf(szURL, _T("http://www.progettoemma.net/gioco.php?&game=%s"), BurnDrvGetText(DRV_NAME));
						ShellExecute(NULL, _T("open"), szURL, NULL, NULL, SW_SHOWNORMAL);
						break;
					}
				}
			}
		}
		
		int id = LOWORD(wParam);
			
		switch (id) {

			// ADD TO FAVORITES
			case ID_ADDFAV:	{
				AddToFavorites();
				RefreshFavGameList();
			}
			break;

			// REMOVE FROM FAVORITES
			case ID_REMOVEFAV: {
				RemoveFromFavorites();
				RefreshFavGameList();
			}
			break;

			case GAMESEL_MENU_PLAY: {
				SelOkay();
				break;
			}
				
			case GAMESEL_MENU_GAMEINFO: {
				UpdatePreview(true, hSelDlg, szAppPreviewsPath);
				if (nTimer) {
					KillTimer(hSelDlg, nTimer);
					nTimer = 0;
				}
				GameInfoDialogCreate(hSelDlg, nBurnDrvSelect);
				break;
			}
			
			case GAMESEL_MENU_IPSMANAGER: {
				IpsManagerCreate(hSelDlg);
				break;
			}
			
			case GAMESEL_MENU_IPSPLAY: {
				bDoPatch = TRUE;
				SelOkay();
				break;
			}
			
			case GAMESEL_MENU_JUKEBOX: {
				SelOkay();
				bJukeboxInUse = true;
				break;
			}
			
			case GAMESEL_MENU_VIEWMAWS: {
				if (!nVidFullscreen) {
					TCHAR szURL[MAX_PATH];
					_stprintf(szURL, _T("http://maws.mameworld.info/maws/set/%s/"), BurnDrvGetText(DRV_NAME));
					ShellExecute(NULL, _T("open"), szURL, NULL, NULL, SW_SHOWNORMAL);
				}
				break;
			}
			
			case GAMESEL_MENU_VIEWCAESAR: {
				if (!nVidFullscreen) {
					TCHAR szURL[MAX_PATH];
					_stprintf(szURL, _T("http://caesar.logiqx.com/php/emulator_game.php?id=finalburnalpha&game=%s"), BurnDrvGetText(DRV_NAME));
					ShellExecute(NULL, _T("open"), szURL, NULL, NULL, SW_SHOWNORMAL);
				}
				break;
			}
			
			case GAMESEL_MENU_VIEWEMMA: {
				if (!nVidFullscreen) {
					TCHAR szURL[MAX_PATH];
					_stprintf(szURL, _T("http://www.progettoemma.net/gioco.php?&game=%s"), BurnDrvGetText(DRV_NAME));
					ShellExecute(NULL, _T("open"), szURL, NULL, NULL, SW_SHOWNORMAL);
				}
				break;
			}
		}
	}

	if (Msg == UM_CLOSE) {
		nDialogSelect = nOldDlgSelected;
		MyEndDialog();
		DeleteObject(hWhiteBGBrush);
		return 0;
	}

	if (Msg == WM_CLOSE) {
		bDialogCancel = true;
		nDialogSelect = nOldDlgSelected;
		MyEndDialog();
		DeleteObject(hWhiteBGBrush);
		return 0;
	}

	if (Msg == WM_TIMER) {
		UpdatePreview(false, hSelDlg, szAppPreviewsPath);
		return 0;
	}
	
	if (Msg == WM_CTLCOLORSTATIC) {
		for (int i = 0; i < 6; i++) {
			if ((HWND)lParam == hInfoLabel[i])	{ return (BOOL)hWhiteBGBrush; }
			if ((HWND)lParam == hInfoText[i])	{ return (BOOL)hWhiteBGBrush; }
		}
		
		if((HWND)lParam == GetDlgItem(hSelDlg, IDC_CHECKAVAILABLEONLY)) 
		{
			SetBkColor((HDC)wParam, RGB(0xFF,0xFF,0xFF));
			SetTextColor((HDC)wParam, RGB(0x00,0x00,0x00));
			return (BOOL)CreateSolidBrush(RGB(0xFF,0xFF,0xFF));
		}

		if((HWND)lParam == GetDlgItem(hSelDlg, IDC_CHECKAUTOEXPAND)) 
		{
			SetBkColor((HDC)wParam, RGB(0xFF,0xFF,0xFF));
			SetTextColor((HDC)wParam, RGB(0x00,0x00,0x00));
			return (BOOL)CreateSolidBrush(RGB(0xFF,0xFF,0xFF));
		}

		if((HWND)lParam == GetDlgItem(hSelDlg, IDC_SEL_SHORTNAME)) 
		{
			SetBkColor((HDC)wParam, RGB(0xFF,0xFF,0xFF));
			SetTextColor((HDC)wParam, RGB(0x00,0x00,0x00));
			return (BOOL)CreateSolidBrush(RGB(0xFF,0xFF,0xFF));
		}

		if((HWND)lParam == GetDlgItem(hSelDlg, IDC_SEL_ASCIIONLY)) 
		{
			SetBkColor((HDC)wParam, RGB(0xFF,0xFF,0xFF));
			SetTextColor((HDC)wParam, RGB(0x00,0x00,0x00));
			return (BOOL)CreateSolidBrush(RGB(0xFF,0xFF,0xFF));
		}

		if((HWND)lParam == GetDlgItem(hSelDlg, IDC_OPT_STATIC)) 
		{
			SetBkMode((HDC)wParam, OPAQUE);
			SetBkColor((HDC)wParam, RGB(0xFF,0xFF,0xFF));
			SetTextColor((HDC)wParam, RGB(0x00,0x00,0x00));
			return (BOOL)CreateSolidBrush(RGB(0xFF,0xFF,0xFF));
		}
	}

	NMHDR* pNmHdr = (NMHDR*)lParam;
	if (Msg == WM_NOTIFY) 
	{
		static int lastnListViewColumnIndex = -1;

		// SORT FAV GAME LIST
		if (pNmHdr->code == LVN_COLUMNCLICK && LOWORD(wParam) == IDC_FAVLIST)
		{
			NMLISTVIEW *nmlv = (NMLISTVIEW*)lParam;
            
			hFavListView = nmlv->hdr.hwndFrom;
			nListViewColumnIndex = nmlv->iSubItem;
            
			if (lastnListViewColumnIndex == nListViewColumnIndex) {
				bListViewAscendingOrder = !bListViewAscendingOrder;
			} else {
				bListViewAscendingOrder = false;
			}

			ListView_SetHeaderSortImage(hFavListView, nListViewColumnIndex, bListViewAscendingOrder);
			lastnListViewColumnIndex = nListViewColumnIndex;
			
			// Sort Favorite Games List (macro)
			ListView_SortItemsEx(hFavListView, ListView_CompareFunc, (LPARAM)hFavListView);

			return 1;
		}

		// For ListView (favorites)
		NMLISTVIEW* pnmv	= (NMLISTVIEW*)lParam;
		NMHDR lpNMHDR		= pnmv->hdr;

		// GAME SELECTED IN FAVORITES
		if (/*(lpNMHDR.code == NM_CLICK && lpNMHDR.idFrom == IDC_FAVLIST) || */(lpNMHDR.code == LVN_ITEMCHANGED && lpNMHDR.idFrom == IDC_FAVLIST)) 
		{
			int iCount		= SendMessage(hFavListView, LVM_GETITEMCOUNT, 0, 0);
			int iSelCount	= SendMessage(hFavListView, LVM_GETSELECTEDCOUNT, 0, 0);

			if(iCount == 0 || iSelCount == 0) return 1;

			TCHAR szRomSet[9] = _T("");

			//int iSel = SendMessage(hFavListView, LVM_GETNEXTITEM, (WPARAM)-1, LVNI_FOCUSED);
			int iItem = pnmv->iItem; // simple way of getting the 'iItem'

			LVITEM LvItem;
			memset(&LvItem, 0, sizeof(LvItem));
			LvItem.iItem		= iItem;
			LvItem.mask			= LVIF_TEXT;
			LvItem.iSubItem		= 0;
			LvItem.pszText		= szRomSet;
			LvItem.cchTextMax	= 9;

			SendMessage(hFavListView, LVM_GETITEMTEXT, (WPARAM)iItem, (LPARAM)&LvItem);
			
			for (unsigned int i = 0; i < nBurnDrvCount; i++) 
			{
				nBurnDrvSelect = i;
				if (!(_tcscmp(BurnDrvGetText(DRV_NAME), szRomSet))) {
					break;
				}
			}

			nDialogSelect	= nBurnDrvSelect;				
			bDrvSelected	= true;
			bFavSelected	= true;

			UpdatePreview(true, hSelDlg, szAppPreviewsPath);
			CheckInfoButtons();

			// Get the text from the drivers via BurnDrvGetText()
			for (int i = 0; i < 6; i++) {
				int nGetTextFlags = nLoadMenuShowX & ASCIIONLY ? DRV_ASCIIONLY : 0;
				TCHAR szItemText[256];
				szItemText[0] = _T('\0');

				switch (i) {
					case 0: {
						bool bBracket = false;

						_stprintf(szItemText, _T("%s"), BurnDrvGetText(DRV_NAME));
						if ((BurnDrvGetFlags() & BDF_CLONE) && BurnDrvGetTextA(DRV_PARENT)) {
							int nOldDrvSelect = nBurnDrvSelect;
							TCHAR* pszName = BurnDrvGetText(DRV_PARENT);

							_stprintf(szItemText + _tcslen(szItemText), _T(" (clone of %s"), BurnDrvGetText(DRV_PARENT));

							for (nBurnDrvSelect = 0; nBurnDrvSelect < nBurnDrvCount; nBurnDrvSelect++) {
								if (!_tcsicmp(pszName, BurnDrvGetText(DRV_NAME))) {
									break;
								}
							}
							if (nBurnDrvSelect < nBurnDrvCount) {
								if (BurnDrvGetText(DRV_PARENT)) {
									_stprintf(szItemText + _tcslen(szItemText), _T(", uses ROMs from %s"), BurnDrvGetText(DRV_PARENT));
								}
							}
							nBurnDrvSelect = nOldDrvSelect;
							bBracket = true;
						} else {
							if (BurnDrvGetTextA(DRV_PARENT)) {
								_stprintf(szItemText + _tcslen(szItemText), _T("%suses ROMs from %s"), bBracket ? _T(", ") : _T(" ("), BurnDrvGetText(DRV_PARENT));
								bBracket = true;
							}
						}
						if (bBracket) {
							_stprintf(szItemText + _tcslen(szItemText), _T(")"));
						}
						SendMessage(hInfoText[i], WM_SETTEXT, (WPARAM)0, (LPARAM)szItemText);
						EnableWindow(hInfoLabel[i], TRUE);
						break;
					}
					case 1: {
						bool bUseInfo = false;

						if (BurnDrvGetFlags() & BDF_PROTOTYPE) {
							_stprintf(szItemText + _tcslen(szItemText), _T("prototype"));
							bUseInfo = true;
						}
						if (BurnDrvGetFlags() & BDF_BOOTLEG) {
							_stprintf(szItemText + _tcslen(szItemText), _T("%sbootleg"), bUseInfo ? _T(", ") : _T(""));
							bUseInfo = true;
						}
						if (BurnDrvGetFlags() & BDF_HACK) {
							_stprintf(szItemText + _tcslen(szItemText), _T("%shack"), bUseInfo ? _T(", ") : _T(""));
							bUseInfo = true;
						}
						if (BurnDrvGetFlags() & BDF_HOMEBREW) {
							_stprintf(szItemText + _tcslen(szItemText), _T("%shomebrew"), bUseInfo ? _T(", ") : _T(""));
							bUseInfo = true;
						}						
						if (BurnDrvGetFlags() & BDF_DEMO) {
							_stprintf(szItemText + _tcslen(szItemText), _T("%sdemo"), bUseInfo ? _T(", ") : _T(""));
							bUseInfo = true;
						}
						_stprintf(szItemText + _tcslen(szItemText), _T("%s%i player%s"), bUseInfo ? _T(", ") : _T(""), BurnDrvGetMaxPlayers(), (BurnDrvGetMaxPlayers() != 1) ? _T("s max") : _T(""));
						bUseInfo = true;
						if (BurnDrvGetText(DRV_BOARDROM)) {
							_stprintf(szItemText + _tcslen(szItemText), _T("%suses board-ROMs from %s"), bUseInfo ? _T(", ") : _T(""), BurnDrvGetText(DRV_BOARDROM));
							SendMessage(hInfoText[i], WM_SETTEXT, (WPARAM)0, (LPARAM)szItemText);
							EnableWindow(hInfoLabel[i], TRUE);
							bUseInfo = true;
						}
						SendMessage(hInfoText[i], WM_SETTEXT, (WPARAM)0, (LPARAM)szItemText);
						EnableWindow(hInfoLabel[i], bUseInfo);
						break;
					}
					case 2: {
						_stprintf(szItemText, _T("%s (%s, %s hardware)"), BurnDrvGetTextA(DRV_MANUFACTURER) ? BurnDrvGetText(nGetTextFlags | DRV_MANUFACTURER) : _T("unknown"), BurnDrvGetText(DRV_DATE), ((BurnDrvGetHardwareCode() & HARDWARE_SNK_MVSCARTRIDGE) == HARDWARE_SNK_MVSCARTRIDGE) ? _T("Neo Geo MVS Cartidge") : BurnDrvGetText(nGetTextFlags | DRV_SYSTEM));
						SendMessage(hInfoText[i], WM_SETTEXT, (WPARAM)0, (LPARAM)szItemText);
						EnableWindow(hInfoLabel[i], TRUE);
						break;
					}
					case 3: {
						TCHAR szText[1024] = _T("");
						TCHAR* pszPosition = szText;
						TCHAR* pszName = BurnDrvGetText(nGetTextFlags | DRV_FULLNAME);

						pszPosition += _sntprintf(szText, 1024, pszName);

						pszName = BurnDrvGetText(nGetTextFlags | DRV_FULLNAME);
						while ((pszName = BurnDrvGetText(nGetTextFlags | DRV_NEXTNAME | DRV_FULLNAME)) != NULL) {
							if (pszPosition + _tcslen(pszName) - 1024 > szText) {
								break;
							}
							pszPosition += _stprintf(pszPosition, _T(SEPERATOR_2) _T("%s"), pszName);
						}
						SendMessage(hInfoText[i], WM_SETTEXT, (WPARAM)0, (LPARAM)szText);
						if (szText[0]) {
							EnableWindow(hInfoLabel[i], TRUE);
						} else {
							EnableWindow(hInfoLabel[i], FALSE);
						}
						break;
					}
					case 4: {
						_stprintf(szItemText, _T("%s"), BurnDrvGetTextA(DRV_COMMENT) ? BurnDrvGetText(nGetTextFlags | DRV_COMMENT) : _T(""));
						if (BurnDrvGetFlags() & BDF_HISCORE_SUPPORTED) {
							_stprintf(szItemText + _tcslen(szItemText), _T("%shigh scores supported"), _tcslen(szItemText) ? _T(", ") : _T(""));
						}
						SendMessage(hInfoText[i], WM_SETTEXT, (WPARAM)0, (LPARAM)szItemText);
						EnableWindow(hInfoLabel[i], TRUE);
						break;
					}
					
					case 5: {
						_stprintf(szItemText, _T("%s"), DecorateGenreInfo());
						SendMessage(hInfoText[i], WM_SETTEXT, (WPARAM)0, (LPARAM)szItemText);
						EnableWindow(hInfoLabel[i], TRUE);
						break;
					}
				}
			}
		}

		// Tabs changed
		if (pNmHdr->code == TCN_SELCHANGE) 
		{
			int nTab = SendMessage(hSelDlgTabControl, TCM_GETCURSEL, 0, 0);

			switch(nTab) 
			{
				// Games list tab
				case 0:
					SelDlgDisplayControls(0);
					SetFocus(hSelList);
					break;
				// Favorite games list tab
				case 1:
					SetFocus(hFavListView);
					RefreshFavGameList();
					SelDlgDisplayControls(1);
					break;
				// Options 
				case 2:
					SelDlgDisplayControls(2);
					break;
			}
			return FALSE;
		}

		// Favorites context menu
		if (lpNMHDR.code == NM_RCLICK && lpNMHDR.idFrom == IDC_FAVLIST) 
		{
			HMENU hMenuLoad,hMenuX; // Context Menu handlers
			POINT oPoint,pt;

			// avoid popping the menu when there is no game in the list, or the user hasn't selected anything
			int iCount		= SendMessage(GetDlgItem(hSelDlg, IDC_FAVLIST), LVM_GETITEMCOUNT, 0, 0);
			int iSelCount	= SendMessage(GetDlgItem(hSelDlg, IDC_FAVLIST), LVM_GETSELECTEDCOUNT, 0, 0);			
			if(iCount == 0 || iSelCount == 0) return 1;

			GetCursorPos( &pt);
			ScreenToClient(hSelList,&pt);
			GetCursorPos( &oPoint);

			// Load Favorite Games List context menu
			hMenuLoad = FBALoadMenu(hAppInst, MAKEINTRESOURCE(IDR_MENU_FAVLIST));
			hMenuX = GetSubMenu(hMenuLoad, 0);

			TCHAR RomSet[9]	= _T("");
			
			int iItem = pnmv->iItem;

			LVITEM LvItem;
			memset(&LvItem,0,sizeof(LvItem));
			LvItem.mask			= LVIF_TEXT;
			LvItem.iSubItem		= 0;
			LvItem.pszText		= RomSet;
			LvItem.cchTextMax	= 9;
			LvItem.iItem		= iItem;

			SendMessage(GetDlgItem(hSelDlg, IDC_FAVLIST), LVM_GETITEMTEXT, iItem, (LPARAM)&LvItem); 

			for (unsigned int i = 0; i < nBurnDrvCount; i++) {
				nBurnDrvSelect = i;
				if (!_tcscmp(BurnDrvGetText(0), RomSet)) break;
			}

			nDialogSelect	= nBurnDrvSelect;					
			bDrvSelected	= true;
			bFavSelected	= true;
			UpdatePreview(true, hSelDlg, szAppPreviewsPath);

			CheckInfoButtons();
			
			if (BurnJukeboxGetFlags() & JBF_GAME_WORKING) {
				EnableMenuItem(hMenuX, GAMESEL_MENU_JUKEBOX,	MF_ENABLED	| MF_BYCOMMAND);
			} else {
				EnableMenuItem(hMenuX, GAMESEL_MENU_JUKEBOX,	MF_GRAYED	| MF_BYCOMMAND);
			}
			
			if (GetNumPatches()) {
				EnableMenuItem(hMenuX, GAMESEL_MENU_IPSMANAGER, MF_ENABLED	| MF_BYCOMMAND);
			} else {
				EnableMenuItem(hMenuX, GAMESEL_MENU_IPSMANAGER, MF_GRAYED	| MF_BYCOMMAND);
			}
			
			LoadActivePatches();
			if (GetNumActivePatches()) {
				EnableMenuItem(hMenuX, GAMESEL_MENU_IPSPLAY,	MF_ENABLED	| MF_BYCOMMAND);
			} else {
				EnableMenuItem(hMenuX, GAMESEL_MENU_IPSPLAY,	MF_GRAYED	| MF_BYCOMMAND);
			}

			TrackPopupMenu(hMenuX, TPM_LEFTALIGN | TPM_RIGHTBUTTON, oPoint.x, oPoint.y, 0, hSelDlg, NULL);			
			DestroyMenu(hMenuLoad);
		}

		// Favorites game list Double-Click event
		if (pNmHdr->code == NM_DBLCLK && LOWORD(wParam) == IDC_FAVLIST) 
		{
			// if the user hasn't selected a valid item in the listview, return
			int iCount		= SendMessage(hFavListView, LVM_GETITEMCOUNT, 0, 0);
			int iSelCount	= SendMessage(hFavListView, LVM_GETSELECTEDCOUNT, 0, 0);		
			
			if(iCount == 0 || iSelCount == 0) return 1;

			SelOkay();
		}

		if ((pNmHdr->code == NM_CLICK) && (pNmHdr->idFrom == IDC_TREE2)) 
		{
			TVHITTESTINFO thi;
			DWORD dwpos = GetMessagePos();
			thi.pt.x	= GET_X_LPARAM(dwpos);
			thi.pt.y	= GET_Y_LPARAM(dwpos);
			MapWindowPoints(HWND_DESKTOP, pNmHdr->hwndFrom, &thi.pt, 1);
			TreeView_HitTest(pNmHdr->hwndFrom, &thi);

			if(TVHT_ONITEMSTATEICON & thi.flags) {
				PostMessage(hSelDlg, UM_CHECKSTATECHANGE, 0, (LPARAM)thi.hItem);
			}

			return 1;
		}

		NMTREEVIEW* pnmtv = (NMTREEVIEW*)lParam;

		if (!TreeBuilding && pnmtv->hdr.code == NM_DBLCLK && pnmtv->hdr.idFrom == IDC_TREE1) 
		{
			DWORD dwpos = GetMessagePos();

			TVHITTESTINFO thi;
			thi.pt.x	= GET_X_LPARAM(dwpos);
			thi.pt.y	= GET_Y_LPARAM(dwpos);
			
			MapWindowPoints(HWND_DESKTOP, pNmHdr->hwndFrom, &thi.pt, 1);
			
			TreeView_HitTest(pNmHdr->hwndFrom, &thi);

			HTREEITEM hSelectHandle = thi.hItem;
         	if(hSelectHandle == NULL) return 1;

			TreeView_SelectItem(hSelList, hSelectHandle);

			// Search through nBurnDrv[] for the nBurnDrvNo according to the returned hSelectHandle
			for (unsigned int i = 0; i < nTmpDrvCount; i++) {
				if (hSelectHandle == nBurnDrv[i].hTreeHandle) {
					nBurnDrvSelect = nBurnDrv[i].nBurnDrvNo;
					break;
				}
			}
			
			nDialogSelect	= nBurnDrvSelect;
			bDrvSelected	= true;
			bFavSelected	= false;

			SelOkay();

			// disable double-click node-expand
			SetWindowLong(hSelDlg, DWL_MSGRESULT, 1);

			return 1;
		}
		
		if(!TreeBuilding && pnmtv->hdr.code == NM_RCLICK && pnmtv->hdr.idFrom == IDC_TREE1)
		{
			DWORD dwpos = GetMessagePos();

			TVHITTESTINFO thi;
			thi.pt.x	= GET_X_LPARAM(dwpos);
			thi.pt.y	= GET_Y_LPARAM(dwpos);
			
			MapWindowPoints(HWND_DESKTOP, pNmHdr->hwndFrom, &thi.pt, 1);
			
			TreeView_HitTest(pNmHdr->hwndFrom, &thi);
			
			HTREEITEM hSelectHandle = thi.hItem;
         	if(hSelectHandle == NULL) return 1;

			TreeView_SelectItem(hSelList, hSelectHandle);

			// Search through nBurnDrv[] for the nBurnDrvNo according to the returned hSelectHandle
			for (unsigned int i = 0; i < nTmpDrvCount; i++) {
				if (hSelectHandle == nBurnDrv[i].hTreeHandle) {
					nBurnDrvSelect = nBurnDrv[i].nBurnDrvNo;
					break;
				}
			}
			
			nDialogSelect	= nBurnDrvSelect;
			bDrvSelected	= true;
			bFavSelected	= false;
			UpdatePreview(true, hSelDlg, szAppPreviewsPath);
			CheckInfoButtons();

			// Menu
			POINT oPoint;
			GetCursorPos(&oPoint);

			HMENU hMenuLoad = FBALoadMenu(hAppInst, MAKEINTRESOURCE(IDR_MENU_GAMESEL));
			HMENU hMenuX = GetSubMenu(hMenuLoad, 0);

			if (BurnJukeboxGetFlags() & JBF_GAME_WORKING) {
				EnableMenuItem(hMenuX, GAMESEL_MENU_JUKEBOX,	MF_ENABLED	| MF_BYCOMMAND);
			} else {
				EnableMenuItem(hMenuX, GAMESEL_MENU_JUKEBOX,	MF_GRAYED	| MF_BYCOMMAND);
			}
			
			if (GetNumPatches()) {
				EnableMenuItem(hMenuX, GAMESEL_MENU_IPSMANAGER, MF_ENABLED	| MF_BYCOMMAND);
			} else {
				EnableMenuItem(hMenuX, GAMESEL_MENU_IPSMANAGER, MF_GRAYED	| MF_BYCOMMAND);
			}
			
			LoadActivePatches();
			if (GetNumActivePatches()) {
				EnableMenuItem(hMenuX, GAMESEL_MENU_IPSPLAY,	MF_ENABLED	| MF_BYCOMMAND);
			} else {
				EnableMenuItem(hMenuX, GAMESEL_MENU_IPSPLAY,	MF_GRAYED	| MF_BYCOMMAND);
			}

			TrackPopupMenu(hMenuX, TPM_LEFTALIGN | TPM_RIGHTBUTTON, oPoint.x, oPoint.y, 0, hSelDlg, NULL);			
			DestroyMenu(hMenuLoad);
			
			return 1;
		}

		if (pNmHdr->code == NM_CUSTOMDRAW && LOWORD(wParam) == IDC_TREE1) {
			LPNMLVCUSTOMDRAW lplvcd = (LPNMLVCUSTOMDRAW)lParam;
			int nGetTextFlags = nLoadMenuShowX & ASCIIONLY ? DRV_ASCIIONLY : 0;
			HTREEITEM hSelectHandle;
			
			switch (lplvcd->nmcd.dwDrawStage) {
				case CDDS_PREPAINT: {
					SetWindowLong(hSelDlg, DWL_MSGRESULT, CDRF_NOTIFYITEMDRAW);
					return 1;
				}

				case CDDS_ITEMPREPAINT:	{
					hSelectHandle = (HTREEITEM)(lplvcd->nmcd.dwItemSpec);
					HBRUSH hBackBrush;
					RECT rect;
					
					// TVITEM (msdn.microsoft.com) This structure is identical to the TV_ITEM structure, but it has been renamed to 
					// follow current naming conventions. New applications should use this structure.

					//TV_ITEM TvItem;
					TVITEM TvItem;
					TvItem.hItem = hSelectHandle;
					TvItem.mask = TVIF_PARAM | TVIF_STATE | TVIF_CHILDREN;
					SendMessage(hSelList, TVM_GETITEM, 0, (LPARAM)&TvItem);

//					dprintf(_T("  - Item (%i×%i) - (%i×%i) %hs\n"), lplvcd->nmcd.rc.left, lplvcd->nmcd.rc.top, lplvcd->nmcd.rc.right, lplvcd->nmcd.rc.bottom, ((NODEINFO*)TvItem.lParam)->pszROMName);

					// Set the foreground and background colours unless the item is highlighted
					if (!(TvItem.state & (TVIS_SELECTED | TVIS_DROPHILITED))) {

						// Set less contrasting colours for clones
						if (!((NODEINFO*)TvItem.lParam)->bIsParent) {
							lplvcd->clrTextBk = RGB(0xF7, 0xF7, 0xF7);
							lplvcd->clrText = RGB(0x3F, 0x3F, 0x3F);
						}

						// For parents, change the colour of the background, for clones, change only the text colour
						if (!CheckWorkingStatus(((NODEINFO*)TvItem.lParam)->nBurnDrvNo)) {
							lplvcd->clrText = RGB(0x7F, 0x7F, 0x7F);
						}
					}

					rect.left	= lplvcd->nmcd.rc.left;
					rect.right	= lplvcd->nmcd.rc.right;
					rect.top	= lplvcd->nmcd.rc.top;
					rect.bottom = lplvcd->nmcd.rc.bottom;

					hBackBrush = CreateSolidBrush(lplvcd->clrTextBk);
					
					nBurnDrvSelect = ((NODEINFO*)TvItem.lParam)->nBurnDrvNo;

					{
						// Fill background
						FillRect(lplvcd->nmcd.hdc, &lplvcd->nmcd.rc, hBackBrush);
					}

					{
						// Draw plus and minus buttons
						if (((NODEINFO*)TvItem.lParam)->bIsParent) {
							if (TvItem.state & TVIS_EXPANDED) {
								DrawIconEx(lplvcd->nmcd.hdc, rect.left + 4, rect.top + nIconsYDiff, hCollapse, 16, 16, 0, NULL, DI_NORMAL);
							} else {
								if (TvItem.cChildren) {
									DrawIconEx(lplvcd->nmcd.hdc, rect.left + 4, rect.top + nIconsYDiff, hExpand, 16, 16, 0, NULL, DI_NORMAL);
								}
							}
						}
						rect.left += 16 + 8;
					}

					rect.top += 2;

					{
						// Draw text

						TCHAR szText[1024];
						TCHAR* pszPosition = szText;
						TCHAR* pszName;
						SIZE size = { 0, 0 };

						SetTextColor(lplvcd->nmcd.hdc, lplvcd->clrText);
						SetBkMode(lplvcd->nmcd.hdc, TRANSPARENT);

						// Display the short name if needed
						if (nLoadMenuShowX & SHOWSHORT) {
							DrawText(lplvcd->nmcd.hdc, BurnDrvGetText(DRV_NAME), -1, &rect, DT_NOPREFIX | DT_SINGLELINE | DT_LEFT | DT_VCENTER);
							rect.left += 16 + 40 + 20;
						}

						{
							// Draw icons if needed
							if (!CheckWorkingStatus(((NODEINFO*)TvItem.lParam)->nBurnDrvNo)) {
								DrawIconEx(lplvcd->nmcd.hdc, rect.left, rect.top, hNotWorking, nIconsSizeXY, nIconsSizeXY, 0, NULL, DI_NORMAL);
								rect.left += nIconsSizeXY + 4;
							} else {
								if (!(gameAv[((NODEINFO*)TvItem.lParam)->nBurnDrvNo])) {
									DrawIconEx(lplvcd->nmcd.hdc, rect.left, rect.top, hNotFoundEss, nIconsSizeXY, nIconsSizeXY, 0, NULL, DI_NORMAL);
									rect.left += nIconsSizeXY + 4;
								} else {
									if (!(nLoadMenuShowX & AVAILONLY) && !(gameAv[((NODEINFO*)TvItem.lParam)->nBurnDrvNo] & 2)) {
										DrawIconEx(lplvcd->nmcd.hdc, rect.left, rect.top, hNotFoundNonEss, nIconsSizeXY, nIconsSizeXY, 0, NULL, DI_NORMAL);
										rect.left += nIconsSizeXY + 4;
									}
								}
							}
						}

						// Driver Icon drawing code...
						if(bEnableIcons && bIconsLoaded) {
							if(hDrvIcon[nBurnDrvSelect]) {
								DrawIconEx(lplvcd->nmcd.hdc, rect.left, rect.top, hDrvIcon[nBurnDrvSelect], nIconsSizeXY, nIconsSizeXY, 0, NULL, DI_NORMAL);
							}

							if(!hDrvIcon[nBurnDrvSelect]) {								
								DrawIconEx(lplvcd->nmcd.hdc, rect.left, rect.top, hDrvIconMiss, nIconsSizeXY, nIconsSizeXY, 0, NULL, DI_NORMAL);
							}
							rect.left += nIconsSizeXY + 4;
						}

						_tcsncpy(szText, MangleGamename(BurnDrvGetText(nGetTextFlags | DRV_FULLNAME), false), 1024);
						szText[1023] = _T('\0');

						GetTextExtentPoint32(lplvcd->nmcd.hdc, szText, _tcslen(szText), &size);

						DrawText(lplvcd->nmcd.hdc, szText, -1, &rect, DT_NOPREFIX | DT_SINGLELINE | DT_LEFT | DT_VCENTER);

						// Display extra info if needed
						szText[0] = _T('\0');

						pszName = BurnDrvGetText(nGetTextFlags | DRV_FULLNAME);
						while ((pszName = BurnDrvGetText(nGetTextFlags | DRV_NEXTNAME | DRV_FULLNAME)) != NULL) {
							if (pszPosition + _tcslen(pszName) - 1024 > szText) {
								break;
							}
							pszPosition += _stprintf(pszPosition, _T(SEPERATOR_2) _T("%s"), pszName);
						}
						if (szText[0]) {
							szText[255] = _T('\0');

							unsigned int r = ((lplvcd->clrText >> 16 & 255) * 2 + (lplvcd->clrTextBk >> 16 & 255)) / 3;
							unsigned int g = ((lplvcd->clrText >>  8 & 255) * 2 + (lplvcd->clrTextBk >>  8 & 255)) / 3;
							unsigned int b = ((lplvcd->clrText >>  0 & 255) * 2 + (lplvcd->clrTextBk >>  0 & 255)) / 3;

							rect.left += size.cx;
							SetTextColor(lplvcd->nmcd.hdc, (r << 16) | (g <<  8) | (b <<  0));
							DrawText(lplvcd->nmcd.hdc, szText, -1, &rect, DT_NOPREFIX | DT_SINGLELINE | DT_LEFT | DT_VCENTER);
						}
					}

					DeleteObject(hBackBrush);

					SetWindowLong(hSelDlg, DWL_MSGRESULT, CDRF_SKIPDEFAULT);
					return 1;
				}

				default: {
					SetWindowLong(hSelDlg, DWL_MSGRESULT, CDRF_DODEFAULT);
					return 1;
				}
			}
		}

		if (pNmHdr->code == TVN_ITEMEXPANDING && !TreeBuilding && LOWORD(wParam) == IDC_TREE1) {
			SendMessage(hSelList, TVM_SELECTITEM, TVGN_CARET, (LPARAM)((LPNMTREEVIEW)lParam)->itemNew.hItem);
			return FALSE;
		}

		if (pNmHdr->code == TVN_SELCHANGED && !TreeBuilding && LOWORD(wParam) == IDC_TREE1) {
			HTREEITEM hSelectHandle = (HTREEITEM)SendMessage(hSelList, TVM_GETNEXTITEM, TVGN_CARET, ~0U);

			// Search through nBurnDrv[] for the nBurnDrvNo according to the returned hSelectHandle
			for (unsigned int i = 0; i < nTmpDrvCount; i++) {
				if (hSelectHandle == nBurnDrv[i].hTreeHandle) 
				{					
					nBurnDrvSelect	= nBurnDrv[i].nBurnDrvNo;					
					nDialogSelect	= nBurnDrvSelect;					
					bDrvSelected	= true;	
					bFavSelected	= false;
					UpdatePreview(true, hSelDlg, szAppPreviewsPath);
					CheckInfoButtons();
					break;
				}
			}

			// Get the text from the drivers via BurnDrvGetText()
			for (int i = 0; i < 6; i++) {
				int nGetTextFlags = nLoadMenuShowX & ASCIIONLY ? DRV_ASCIIONLY : 0;
				TCHAR szItemText[256];
				szItemText[0] = _T('\0');

				switch (i) {
					case 0: {
						bool bBracket = false;

						_stprintf(szItemText, _T("%s"), BurnDrvGetText(DRV_NAME));
						if ((BurnDrvGetFlags() & BDF_CLONE) && BurnDrvGetTextA(DRV_PARENT)) {
							int nOldDrvSelect = nBurnDrvSelect;
							TCHAR* pszName = BurnDrvGetText(DRV_PARENT);

							_stprintf(szItemText + _tcslen(szItemText), _T(" (clone of %s"), BurnDrvGetText(DRV_PARENT));

							for (nBurnDrvSelect = 0; nBurnDrvSelect < nBurnDrvCount; nBurnDrvSelect++) {
								if (!_tcsicmp(pszName, BurnDrvGetText(DRV_NAME))) {
									break;
								}
							}
							if (nBurnDrvSelect < nBurnDrvCount) {
								if (BurnDrvGetText(DRV_PARENT)) {
									_stprintf(szItemText + _tcslen(szItemText), _T(", uses ROMs from %s"), BurnDrvGetText(DRV_PARENT));
								}
							}
							nBurnDrvSelect = nOldDrvSelect;
							bBracket = true;
						} else {
							if (BurnDrvGetTextA(DRV_PARENT)) {
								_stprintf(szItemText + _tcslen(szItemText), _T("%suses ROMs from %s"), bBracket ? _T(", ") : _T(" ("), BurnDrvGetText(DRV_PARENT));
								bBracket = true;
							}
						}
						if (BurnDrvGetTextA(DRV_SAMPLENAME)) {
							_stprintf(szItemText + _tcslen(szItemText), _T("%suses samples from %s"), bBracket ? _T(", ") : _T(" ("), BurnDrvGetText(DRV_SAMPLENAME));
							bBracket = true;
						}
						if (bBracket) {
							_stprintf(szItemText + _tcslen(szItemText), _T(")"));
						}
						SendMessage(hInfoText[i], WM_SETTEXT, (WPARAM)0, (LPARAM)szItemText);
						EnableWindow(hInfoLabel[i], TRUE);
						break;
					}
					case 1: {
						bool bUseInfo = false;

						if (BurnDrvGetFlags() & BDF_PROTOTYPE) {
							_stprintf(szItemText + _tcslen(szItemText), _T("prototype"));
							bUseInfo = true;
						}
						if (BurnDrvGetFlags() & BDF_BOOTLEG) {
							_stprintf(szItemText + _tcslen(szItemText), _T("%sbootleg"), bUseInfo ? _T(", ") : _T(""));
							bUseInfo = true;
						}
						if (BurnDrvGetFlags() & BDF_HACK) {
							_stprintf(szItemText + _tcslen(szItemText), _T("%shack"), bUseInfo ? _T(", ") : _T(""));
							bUseInfo = true;
						}
						if (BurnDrvGetFlags() & BDF_HOMEBREW) {
							_stprintf(szItemText + _tcslen(szItemText), _T("%shomebrew"), bUseInfo ? _T(", ") : _T(""));
							bUseInfo = true;
						}						
						if (BurnDrvGetFlags() & BDF_DEMO) {
							_stprintf(szItemText + _tcslen(szItemText), _T("%sdemo"), bUseInfo ? _T(", ") : _T(""));
							bUseInfo = true;
						}
						_stprintf(szItemText + _tcslen(szItemText), _T("%s%i player%s"), bUseInfo ? _T(", ") : _T(""), BurnDrvGetMaxPlayers(), (BurnDrvGetMaxPlayers() != 1) ? _T("s max") : _T(""));
						bUseInfo = true;
						if (BurnDrvGetText(DRV_BOARDROM)) {
							_stprintf(szItemText + _tcslen(szItemText), _T("%suses board-ROMs from %s"), bUseInfo ? _T(", ") : _T(""), BurnDrvGetText(DRV_BOARDROM));
							SendMessage(hInfoText[i], WM_SETTEXT, (WPARAM)0, (LPARAM)szItemText);
							EnableWindow(hInfoLabel[i], TRUE);
							bUseInfo = true;
						}
						SendMessage(hInfoText[i], WM_SETTEXT, (WPARAM)0, (LPARAM)szItemText);
						EnableWindow(hInfoLabel[i], bUseInfo);
						break;
					}
					case 2: {
						_stprintf(szItemText, _T("%s (%s, %s hardware)"), BurnDrvGetTextA(DRV_MANUFACTURER) ? BurnDrvGetText(nGetTextFlags | DRV_MANUFACTURER) : _T("unknown"), BurnDrvGetText(DRV_DATE), ((BurnDrvGetHardwareCode() & HARDWARE_SNK_MVSCARTRIDGE) == HARDWARE_SNK_MVSCARTRIDGE) ? _T("Neo Geo MVS Cartidge") : BurnDrvGetText(nGetTextFlags | DRV_SYSTEM));
						SendMessage(hInfoText[i], WM_SETTEXT, (WPARAM)0, (LPARAM)szItemText);
						EnableWindow(hInfoLabel[i], TRUE);
						break;
					}
					case 3: {
						TCHAR szText[1024] = _T("");
						TCHAR* pszPosition = szText;
						TCHAR* pszName = BurnDrvGetText(nGetTextFlags | DRV_FULLNAME);

						pszPosition += _sntprintf(szText, 1024, pszName);

						pszName = BurnDrvGetText(nGetTextFlags | DRV_FULLNAME);
						while ((pszName = BurnDrvGetText(nGetTextFlags | DRV_NEXTNAME | DRV_FULLNAME)) != NULL) {
							if (pszPosition + _tcslen(pszName) - 1024 > szText) {
								break;
							}
							pszPosition += _stprintf(pszPosition, _T(SEPERATOR_2) _T("%s"), pszName);
						}
						SendMessage(hInfoText[i], WM_SETTEXT, (WPARAM)0, (LPARAM)szText);
						if (szText[0]) {
							EnableWindow(hInfoLabel[i], TRUE);
						} else {
							EnableWindow(hInfoLabel[i], FALSE);
						}
						break;
					}
					case 4: {
						_stprintf(szItemText, _T("%s"), BurnDrvGetTextA(DRV_COMMENT) ? BurnDrvGetText(nGetTextFlags | DRV_COMMENT) : _T(""));
						if (BurnDrvGetFlags() & BDF_HISCORE_SUPPORTED) {
							_stprintf(szItemText + _tcslen(szItemText), _T("%shigh scores supported"), _tcslen(szItemText) ? _T(", ") : _T(""));
						}
						SendMessage(hInfoText[i], WM_SETTEXT, (WPARAM)0, (LPARAM)szItemText);
						EnableWindow(hInfoLabel[i], TRUE);
						break;
					}
					
					case 5: {
						_stprintf(szItemText, _T("%s"), DecorateGenreInfo());
						SendMessage(hInfoText[i], WM_SETTEXT, (WPARAM)0, (LPARAM)szItemText);
						EnableWindow(hInfoLabel[i], TRUE);
						break;
					}
				}
			}
		}
	}
	return 0;
}

int SelDialog(int nMVSCartsOnly, HWND hParentWND)
{
	int nOldSelect = nBurnDrvSelect;
	
	if(bDrvOkay) {
		nOldDlgSelected = nBurnDrvSelect;
	}

	hParent = hParentWND;
	nShowMVSCartsOnly = nMVSCartsOnly;
	
	InitCommonControls();

	FBADialogBox(hAppInst, MAKEINTRESOURCE(IDD_SELNEW), hParent, DialogProc);

	if (!_tcscmp(BurnDrvGetText(DRV_NAME), _T("neogeo"))) {
		bMVSMultiSlot = true;
	} else {
		if(!nShowMVSCartsOnly) {
			bMVSMultiSlot = false;
		}
	}

	hSelDlg = NULL;
	hSelList = NULL;

	free(nBurnDrv);
	nBurnDrv = NULL;

	nBurnDrvSelect = nOldSelect;

	return nDialogSelect;
}

