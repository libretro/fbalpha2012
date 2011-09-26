// Driver Selector module
// TreeView Version by HyperYagami
// Tab control by regret

/* changelog:
 update 9: add game icon display
 update 8: improved show rom info dialog
 update 7: change tree list make method (faster)
 update 6: update hardware selection
 update 5: add filter and source cleanup
 update 4: add tab control for preview image
 update 3: add tab control for options
 update 2: add icon display
 update 1: modify options layout (ref: fba plus)
*/

#include "burner.h"
#include <shellapi.h>
#include "imageload+.h"
#include "iconload.h"
#ifndef NO_IPS
#include "patch.h"
#endif
#include "seldef.h"

// Disable selecting non-available sets
#define DISABLE_NON_AVAILABLE_SELECT 0
// Prompt user on loading non-working sets
#define NON_WORKING_PROMPT_ON_LOAD 1

// use megadrive driver list
//#define MD_DRIVER_LIST

// ==> timer, added by regret
#define IMAGE_TIMER_ID 1
#define IMAGE_TIMER 2500
#define FILTER_TIMER_ID 2
#define FILTER_TIMER 500

UINT_PTR nImageTimer = 0;
UINT_PTR nFilterTimer = 0;
// <== timer

HWND hSelDlg = NULL;
static HWND hSelList = NULL;
static int nDialogSelect = -1; // The driver which this dialog selected

// Info-box
enum INFO_CONTENT { INFO_ROMNAME = 0, INFO_ROMINFO, INFO_SYSTEM, INFO_COMMENT, INFO_NOTE, INFO_NUM };

static HWND hInfoLabel[INFO_NUM] = { NULL };
static HWND hInfoText[INFO_NUM] = { NULL };
static HBRUSH hWhiteBGBrush = CreateSolidBrush(RGB(0xFF,0xFF,0xFF));

struct InfoDefine {
	unsigned int label;
	unsigned int text;
} infoDef[] = {
	{ IDC_LABELROMNAME, IDC_TEXTROMNAME },
	{ IDC_LABELROMINFO, IDC_TEXTROMINFO },
	{ IDC_LABELSYSTEM, IDC_TEXTSYSTEM },
	{ IDC_LABELCOMMENT, IDC_TEXTCOMMENT },
	{ IDC_LABELNOTES, IDC_TEXTNOTES },
};

static HBITMAP hPreview = NULL;
static HBITMAP hDefPreview = NULL;
static HICON hExpand, hCollapse;
static HICON hNotWorking, hNotFoundEss, hNotFoundNonEss, hROMICO, hClone, hImperfect;

static BOOL bTreeBuilding = FALSE; // if 1, ignore TVN_SELCHANGED messages

// ==> System define
int nSystemSel = 0;
struct SystemDefine {
	int hardwareCode;
	int buttonCode;
	int hardware;
} systemDef[] = {
	{ IDS_SEL_HARDWARE, 	-1,				-1	 			},
	{ IDS_SEL_CAVE,			IDC_CAVE,		MASKCAVE		},
	{ IDS_SEL_CPS1,			IDC_CPS1,		MASKCPS			},
	{ IDS_SEL_CPS2,			IDC_CPS2,		MASKCPS2		},
	{ IDS_SEL_CPS3,			IDC_CPS3,		MASKCPS3		},
	{ IDS_SEL_GALAXIAN,		IDC_GALAXIAN,	MASKGALAXIAN	},
	{ IDS_SEL_KANEKO16,		IDC_KANEKO16,	MASKKANEKO16	},
	{ IDS_SEL_KONAMI,		IDC_KONAMI,		MASKKONAMI		},
	{ IDS_SEL_NEOGEO,		IDC_NEOGEO,		MASKNEOGEO		},
	{ IDS_SEL_PACMAN,		IDC_PACMAN,		MASKPACMAN		},
	{ IDS_SEL_PGM,			IDC_PGM,		MASKPGM			},
	{ IDS_SEL_PSIKYO,		IDC_PSIKYO,		MASKPSIKYO		},
	{ IDS_SEL_SEGA,			IDC_SEGA,		MASKSEGA		},
	{ IDS_SEL_TOAPLAN,		IDC_TOAPLAN,	MASKTOAPLAN		},
	{ IDS_SEL_TAITO,		IDC_TAITO,		MASKTAITO		},
	{ IDS_SEL_ATARI,		IDC_ATARI,		MASKATARI		},
	{ IDS_SEL_MISC,			IDC_DRVMISC,	MASKMISC		},
#ifdef MD_DRIVER_LIST
	{ IDS_SEL_MEGADRIVE,	IDC_MEGADRIVE,	MASKMD			},
#endif
	0
};
// <== System define

int nLoadMenuShowX = 0;
int nLoadDriverShowX = 0;

static BOOL bRClick = FALSE;

// ==> tab control, added by regret
enum eTab {
	NONE = -1,
	LIST,
	FILTER,
//	ETC,
	MAX_TAB,
};

enum eImageTab {
	IMG_NONE = -1,
	IMG_PREVIEW,
	IMG_TITLE,
	IMG_FLYER,
	IMG_SCORE,
	IMG_SELECT,
	IMG_GAMEOVER,
	IMG_BOSS,
	IMG_MAXTAB,
};

struct ImageDefine {
	int imageId;
	int pathId;
} imageDef[] = {
	{ IMG_PREVIEW, PATH_PREVIEW },
	{ IMG_TITLE, PATH_TITLE },
	{ IMG_FLYER, PATH_FLYER },
	{ IMG_SCORE, PATH_SCORE },
	{ IMG_SELECT, PATH_SELECT },
	{ IMG_GAMEOVER, PATH_GAMEOVER },
	{ IMG_BOSS, PATH_BOSS },
	-1
};

HWND hSelTab = NULL;
HWND hImgTab = NULL;
int nCurrentTab, nCurImgTab;
int nTabSel;

static int previewOrgX;
static int previewOrgY;

// for tab control theme, thanks MAME32 team
FARPROC fnIsThemed  = NULL;
HBRUSH hTabBkBrush = NULL;
BOOL bThemeActive;

// filter string
TCHAR szUserFilterStr[MAX_PATH] = _T("");
static TCHAR szFilterStr[64] = _T("");
static bool hasFilterStr = false;

static const TCHAR szShowUnAvail[16] = _T("lamer");
static BOOL bShowUnAvail = FALSE;
// <== tab control

static TCHAR ResDir[] = _T("res\\"); // custom resource dir
static TCHAR archiveName[64] = _T(""); // preview archive name

struct NODEINFO {
	bool used;
	int driverId;
	bool isParent;
	HTREEITEM hTreeHandle;
	int iconid;
};

static map<string, NODEINFO*> driverNode;
static unsigned int nodeCount;
static unsigned int cloneCount; // clone game count, added by regret

// icon
static HTREEITEM hIconEffectItem = NULL;

static void rebuildEverything();

// Check if a specified driver is working
static bool checkWorkingStatus(int nDriver)
{
	unsigned int oldDrvSelect = nBurnDrvSelect;
	nBurnDrvSelect = nDriver;
	bool bStatus = BurnDrvIsWorking();
	nBurnDrvSelect = oldDrvSelect;

	return bStatus;
}

// ==> game list filter, added by regret
static inline bool myStrStrI(const TCHAR* pszStr, const TCHAR* pszSrch)
{
	if (!pszStr || !pszSrch) {
		return NULL; // no content
	}

	TCHAR* cp = (TCHAR*)pszStr;
	TCHAR* s1;
	TCHAR* s2;

	while (*cp) {
		s1 = cp;
		s2 = (TCHAR*)pszSrch;

		while (*s1 && *s2 && !_tcsnicmp(s1, s2, 1)) {
			s1++, s2++;
		}

		if (!*s2) {
			return true;//cp;
		}

		cp++;
	}
	return false;
}

static void initFilterString()
{
	SendDlgItemMessage(hSelDlg, IDC_FILTER_KEY, CB_ADDSTRING, 0, (LPARAM)_T("bootleg"));
	SendDlgItemMessage(hSelDlg, IDC_FILTER_KEY, CB_ADDSTRING, 0, (LPARAM)_T("decrypted"));
	SendDlgItemMessage(hSelDlg, IDC_FILTER_KEY, CB_ADDSTRING, 0, (LPARAM)_T("street fighter"));
	SendDlgItemMessage(hSelDlg, IDC_FILTER_KEY, CB_ADDSTRING, 0, (LPARAM)_T("king of fighters"));

	// init user filter string
	if (_tcslen(szUserFilterStr) > 0) {
		TCHAR szToken[MAX_PATH];
		_tcscpy(szToken, szUserFilterStr);
		TCHAR* token = _tcstok(szToken, _T(";"));
		while (token != NULL) {
			SendDlgItemMessage(hSelDlg, IDC_FILTER_KEY, CB_ADDSTRING, 0, (LPARAM)token);
			token = _tcstok(NULL, _T(";"));
		}
		free(token);
	}

//	_tcscpy(szFilterStr, _T(""));
	SendDlgItemMessage(hSelDlg, IDC_FILTER_KEY, WM_SETTEXT, (WPARAM)sizearray(szFilterStr), (LPARAM)szFilterStr);
	hasFilterStr = _tcslen(szFilterStr) > 0 ? true : false;
}

static void initFilterSystem()
{
	for (int i = 0; systemDef[i].hardwareCode; i++) {
		SendDlgItemMessage(hSelDlg, IDC_FILTER_SYSTEM, CB_ADDSTRING, 0, (LPARAM)FBALoadStringEx(systemDef[i].hardwareCode));
	}
	SendDlgItemMessage(hSelDlg, IDC_FILTER_SYSTEM, CB_SETCURSEL, (WPARAM)nSystemSel, 0);
}

static BOOL gameInfoFiltered(TCHAR* pszFilter)
{
	// instead "_tcsstr"
	if (myStrStrI(BurnDrvGetText(DRV_NAME), pszFilter)
		|| myStrStrI(transGameName(BurnDrvGetText(DRV_FULLNAME)), pszFilter)
		|| myStrStrI(BurnDrvGetText(DRV_ASCIIONLY | DRV_FULLNAME), pszFilter)
		|| myStrStrI(BurnDrvGetText(DRV_MANUFACTURER), pszFilter)
		|| myStrStrI(BurnDrvGetText(DRV_DATE), pszFilter)
	) {
		// match the filter string
		return FALSE;
	}

	// unicode full name
	TCHAR* pszName = BurnDrvGetText(DRV_FULLNAME);
	if (myStrStrI(pszName, pszFilter)) {
		return FALSE;
	}
	pszName = BurnDrvGetText(DRV_NEXTNAME | DRV_FULLNAME);
	if (myStrStrI(pszName, pszFilter)) {
		return FALSE;
	}

	// comment
	if (myStrStrI(BurnDrvGetText(DRV_COMMENT), pszFilter)) {
		return FALSE;
	}

	// not found
	return TRUE;
}

static inline BOOL systemFiltered()
{
	if (nSystemSel == 0)
		return FALSE; // any

	const int nHardware = 1 << (BurnDrvGetHardwareCode() >> 24);

	if (systemDef[nSystemSel].hardware ==  nHardware) {
		return FALSE;
	}
	return TRUE;
}

static BOOL isGameFiltered(BOOL bShowUnAvail)
{
	if (bShowUnAvail) {
		// show unavailable games
		if (getAuditState(nBurnDrvSelect) != AUDIT_FAIL) {
			return TRUE;
		}
	}
	else if (systemFiltered()) {
		return TRUE;
	}
	else if (hasFilterStr && gameInfoFiltered(szFilterStr)) {
		return TRUE;
	}

	return FALSE;
}

static inline BOOL gameListFilter(BOOL isParent)
{
//	if (BurnDrvGetFlags() & BDF_BOARDROM) {
//		return TRUE;
//	}

	if (isParent) {
		// Skip clones
		if (BurnDrvGetTextA(DRV_PARENT) != NULL && (BurnDrvGetFlags() & BDF_CLONE)) {
			return TRUE;
		}
	} else {
		// Skip parents
		if (BurnDrvGetTextA(DRV_PARENT) == NULL || !(BurnDrvGetFlags() & BDF_CLONE)) {
			return TRUE;
		}
	}

	// Skip non-available games if needed
	if (avOk && (nLoadMenuShowX & AVAILONLY)
		&& getAuditState(nBurnDrvSelect) == AUDIT_FAIL
		&& !bShowUnAvail) {
		return TRUE;
	}

	int nHardware = 1 << (BurnDrvGetHardwareCode() >> 24);
	if ((nHardware & MASKALL) && (nHardware & nLoadDriverShowX)) {
		return TRUE;
	}

	if (isGameFiltered(bShowUnAvail)) {
		return TRUE;
	}

	// ==> favorite filter, added by regret
	if ((nLoadMenuShowX & SHOWFAVORITE) && filterFavorite(nBurnDrvSelect)) {
		return TRUE;
	}
	// <== favorite filter

	return FALSE;
}

// do sth after filter change
extern int createFilteredGamelistFile(int);
extern int exportNonWorkingGamelist();

static void filterChange()
{
	static const TCHAR szExportRomList[16] = _T("-romlist");	// rom name list
	static const TCHAR szExportFilterList[16] = _T("-mylist");	// filtered list
	static const TCHAR szExportNWList[16] = _T("-nwlist");		// non-working list

	hasFilterStr = _tcslen(szFilterStr) > 0 ? true : false;

	if (hasFilterStr && !_tcsicmp(szFilterStr, szExportRomList)) {
		createFilteredGamelistFile(-1);
	}
	if (hasFilterStr && !_tcsicmp(szFilterStr, szExportFilterList)) {
		createFilteredGamelistFile(systemDef[nSystemSel].hardware);
	}
	else if (hasFilterStr && !_tcsicmp(szFilterStr, szExportNWList)) {
		exportNonWorkingGamelist();
	}
	else {
		rebuildEverything();
	}
}
// <== game list filter

static BOOL checkImperfectStatus(int driver)
{
	unsigned int nOldnBurnDrvSelect = nBurnDrvSelect;
	nBurnDrvSelect = driver;
	const TCHAR* comment = BurnDrvGetText(DRV_COMMENT);
	nBurnDrvSelect = nOldnBurnDrvSelect;
	if (comment == NULL)
		return FALSE;

	if (myStrStrI(comment, _T("imperfect"))
		|| myStrStrI(comment, _T("incomplete"))
		|| myStrStrI(comment, _T("no sound"))
		|| myStrStrI(comment, _T("missing"))
		|| myStrStrI(comment, _T("preliminary"))
		|| myStrStrI(comment, _T("bad dump"))) {
		return TRUE;
	}
	return FALSE;
}

// ==> load custom resource, added by regret
struct CustomIcon {
	HICON* pIcon;
	TCHAR* pszResName;
} sCusIco[] = {
	{ &hExpand,			_T("Plus.ico")						},
	{ &hCollapse,		_T("Minus.ico")						},
	{ &hNotWorking,		_T("Not Working.ico")				},
	{ &hNotFoundEss,	_T("Not Found.ico")					},
	{ &hNotFoundNonEss,	_T("Not Found Non Essential.ico")	},
	{ &hROMICO,			_T("ROM.ico")						},
	{ &hClone,			_T("Clone.ico")						},
	{ &hImperfect,		_T("Imperfect.ico")					},
	NULL // end
};

static void loadCustomRes()
{
	TCHAR szTmpStr[MAX_PATH];

	_stprintf(szTmpStr, _T("%s%s"), ResDir, _T("preview.bmp"));
	if ((hDefPreview = (HBITMAP)LoadImage(hAppInst, szTmpStr, IMAGE_BITMAP, 304, 224, LR_LOADFROMFILE)) == NULL)
		hDefPreview = LoadBitmap(hAppInst, MAKEINTRESOURCE(BMP_PREVIEW));

	for (int i = 0; sCusIco[i].pIcon; i++) {
		_stprintf(szTmpStr, _T("%s%s"), ResDir, sCusIco[i].pszResName);
		if ((*sCusIco[i].pIcon = (HICON)LoadImage(hAppInst, szTmpStr, IMAGE_ICON, 16, 16, LR_LOADFROMFILE)) == NULL)
			*sCusIco[i].pIcon = (HICON)LoadImage(hAppInst, MAKEINTRESOURCE(IDI_TV_PLUS + i), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
	}
}

static void destroyResource()
{
	if (hPreview) {
		DeleteObject((HGDIOBJ)hPreview);
		hPreview = NULL;
	}
	if (hDefPreview) {
		DeleteObject((HGDIOBJ)hDefPreview);
		hDefPreview = NULL;
	}

	for (int i = 0; sCusIco[i].pIcon; i++) {
		if (*sCusIco[i].pIcon) {
			DestroyIcon(*sCusIco[i].pIcon);
			*sCusIco[i].pIcon = NULL;
		}
	}
}

static inline void drawIcons(TVITEMEX& TvItem, RECT& rect, HDC hdc)
{
	const NODEINFO* nodeInfo = (NODEINFO*)TvItem.lParam;
	const int drvIndex = nodeInfo->driverId;
	const bool isParent = nodeInfo->isParent;
	const bool noIcon = (nLoadMenuShowX & NOICON) ? true : false;

	// Draw plus and minus buttons
	if (isParent) {
		if (TvItem.state & TVIS_EXPANDED) {
			DrawIconEx(hdc, rect.left + 4, rect.top, hCollapse, 16, 16, 0, NULL, DI_NORMAL);
		} else {
			if (TvItem.cChildren) {
				DrawIconEx(hdc, rect.left + 4, rect.top, hExpand, 16, 16, 0, NULL, DI_NORMAL);
			}
		}
	}
	rect.left += 24;

	// Add by Sho, clone game moves right 2 char
	if (!isParent) {
		rect.left += 12;
	}

	// Display the short name if needed
	if (nLoadMenuShowX & SHOWSHORT) {
		DrawText(hdc, BurnDrvGetText(DRV_NAME), -1, &rect, DT_NOPREFIX | DT_SINGLELINE | DT_LEFT | DT_VCENTER | DT_NOCLIP);
		rect.left += 68;
	}

	// Draw game icons
	if (!noIcon) {
		if (nodeInfo->iconid >= 0) {
			if (hIconEffectItem == TvItem.hItem) {
				drawIcon(true, nodeInfo->iconid, hdc, rect.left, rect.top);
				rect.left += 16;
			} else {
				drawIcon(false, nodeInfo->iconid, hdc, rect.left, rect.top);
			}
		}
		else if (!checkWorkingStatus(drvIndex)) {
			DrawIconEx(hdc, rect.left, rect.top, hNotWorking, 16, 16, 0, NULL, DI_NORMAL);
		}
		else if (getAuditState(drvIndex) == AUDIT_FAIL) {
			DrawIconEx(hdc, rect.left, rect.top, hNotFoundEss, 16, 16, 0, NULL, DI_NORMAL);
		}
		else if (getAuditState(drvIndex) == AUDIT_PARTPASS) {
			DrawIconEx(hdc, rect.left, rect.top, hNotFoundNonEss, 16, 16, 0, NULL, DI_NORMAL);
		}
		else if (checkImperfectStatus(drvIndex)) {
			DrawIconEx(hdc, rect.left, rect.top, hImperfect, 16, 16, 0, NULL, DI_NORMAL);
		}
		else if (!isParent) {
			DrawIconEx(hdc, rect.left, rect.top, hClone, 16, 16, 0, NULL, DI_NORMAL);
		}
		else {
			DrawIconEx(hdc, rect.left, rect.top, hROMICO, 16, 16, 0, NULL, DI_NORMAL);
		}
		rect.left += 20;
	}
}
// <== load custom resource

static void killImageTimer()
{
	if (nImageTimer) {
		KillTimer(hSelDlg, nImageTimer);
		nImageTimer = 0;
	}
}

static void killFilterTimer()
{
	if (nFilterTimer) {
		KillTimer(hSelDlg, nFilterTimer);
		nFilterTimer = 0;
	}
}

static void myEndDialog()
{
	killImageTimer();
	killFilterTimer();

	SendDlgItemMessage(hSelDlg, IDC_SCREENSHOT_H, STM_SETIMAGE, IMAGE_BITMAP, 0);

	// ==> load custom resource, modified by regret
	destroyResource();
	// <== load custom resource

	// ==> favorite filter, added by regret
	saveFavorites();
	// <== favorite filter

	EndDialog(hSelDlg, 0);
}

// User clicked ok for a driver in the list
static void selOkay()
{
	// modified by emufan
	unsigned int nSelectedGame = 0;
	if (!bRClick) {
		TVITEMEX TvItem;
		HTREEITEM hSelectHandle = TreeView_GetNextItem(hSelList, ~0U, TVGN_CARET);
		if (!hSelectHandle) {		// Nothing is selected, return without closing the window
			return;
		}

		TvItem.hItem = hSelectHandle;
		TvItem.mask = TVIF_PARAM;
		TreeView_GetItem(hSelList, &TvItem);
		if ((NODEINFO*)TvItem.lParam) {
			nSelectedGame = ((NODEINFO*)TvItem.lParam)->driverId;
		}
	} else {
		nSelectedGame = nBurnDrvSelect;
		bRClick = FALSE;
	}

#if DISABLE_NON_AVAILABLE_SELECT
	// Game not available, return without closing the window
	if (getAuditState(nSelectedGame) == AUDIT_FAIL) {
		return;
	}
#endif

#if NON_WORKING_PROMPT_ON_LOAD
	if (!checkWorkingStatus(nSelectedGame)) {
		if (MessageBox(hSelDlg, FBALoadStringEx(IDS_SEL_LOADNONWORK),
				_T("Warning!"), MB_YESNO | MB_DEFBUTTON2 | MB_ICONWARNING) == IDNO) {
			return;
		}
	}
#endif

	nDialogSelect = nSelectedGame;

	myEndDialog();
}

// ==> icon load, added by regret
#include <process.h>

static DWORD iconThreadId;
static HANDLE iconThread;

bool gameInList(const char* name)
{
	if (!name) {
		return false;
	}

	map<string, NODEINFO*>::iterator iter = driverNode.find(name);
	if (iter != driverNode.end()) {
		return true;
	}
	return false;
}

static unsigned __stdcall iconThreadEntry(void* param)
{
	// create image list
	if (createIconList()) {
		clearIcons();
		return 1;
	}

	// check archive file
	TCHAR iconPath[MAX_PATH] = _T("");
	_stprintf(iconPath, _T("%s%s"), getMiscPath(PATH_ICON), getMiscArchiveName(PATH_ICON));
	if (archiveCheck(iconPath) == ARC_NONE || loadIconFromArchive(iconPath)) {
		return 0;
	}

	char* gamename = NULL;
	NODEINFO* nodeInfo = NULL;
	int id = -1;

	map<string, NODEINFO*>::iterator iter = driverNode.begin();
	for (; iter != driverNode.end(); iter++) {
		nodeInfo = iter->second;

		// load icon
		gamename = BurnDrvGetMyTextA(nodeInfo->driverId, DRV_NAME);
		id = findIconMap(gamename);
		if (id >= 0) {
			nodeInfo->iconid = id;
		} else {
			// use parent's icon
			if (BurnDrvGetMyTextA(nodeInfo->driverId, DRV_PARENT)) {
				gamename = BurnDrvGetMyTextA(nodeInfo->driverId, DRV_PARENT);
				id = findIconMap(gamename);
				if (id >= 0) {
					nodeInfo->iconid = id;
				}
			}
		}
	}

	return 0;
}

static void startLoadIcon()
{
	// start icon load thread
	size_t temp = _beginthreadex(NULL, 0, iconThreadEntry, NULL, 0, (unsigned *)&iconThreadId);
	iconThread = (HANDLE)temp;
	if (!iconThread) {
		return;
	}
	SetThreadPriority(iconThread, THREAD_PRIORITY_BELOW_NORMAL);
}
// <== icon load

// ==> node info, added by regret
static bool nodeInit = false;

static inline void initNodeInfo()
{
	if (nodeInit) {
		return;
	}

	unsigned int oldDrvSelect = nBurnDrvSelect;

	NODEINFO* nodeInfo = NULL;
	for (unsigned int i = 0; i < nBurnDrvCount; i++) {
		nBurnDrvSelect = i;
		if (BurnDrvGetFlags() & BDF_BOARDROM) {
			continue; // skip bios
		}
		nodeInfo = new NODEINFO;
		nodeInfo->driverId = i;
		nodeInfo->iconid = -1;
		driverNode[BurnDrvGetTextA(DRV_NAME)] = nodeInfo;
	}

	nBurnDrvSelect = oldDrvSelect;

	startLoadIcon(); // start icon load thread

	nodeInit = true;
}

static inline void resetNodeInfo()
{
	NODEINFO* nodeInfo = NULL;
	map<string, NODEINFO*>::iterator iter = driverNode.begin();

	for (; iter != driverNode.end(); iter++) {
		nodeInfo = iter->second;
		nodeInfo->used = false;
		nodeInfo->isParent = false;
		nodeInfo->hTreeHandle = NULL;
	}
}

void clearNodeInfo()
{
	if (iconThread) {
		CloseHandle(iconThread);
		iconThread = NULL;
	}

	map<string, NODEINFO*>::iterator iter = driverNode.begin();
	for (; iter != driverNode.end(); iter++) {
		delete iter->second;
	}
	driverNode.clear();

	clearIcons();

	nodeInit = false;
}
// <== node info, added by regret

// Make a tree-view control with all drivers
static int selListMake()
{
	nodeCount = 0;
	cloneCount = 0;

	if (hSelList == NULL) {
		return 1;
	}

	resetNodeInfo();

	// ==> game list filter, added by regret
	bShowUnAvail = FALSE;
	if (hasFilterStr && !_tcsicmp(szFilterStr, szShowUnAvail)) {
		bShowUnAvail = TRUE;
	}
	// <== game list filter

	// Add all the driver names to the list
	map<string, NODEINFO*>::iterator iter;
	NODEINFO* nodeInfo = NULL;
	map<string, NODEINFO*>::iterator iter1;
	NODEINFO* nodeInfo1 = NULL;

	int nGetTextFlags = (nLoadMenuShowX & ASCIIONLY) ? DRV_ASCIIONLY : 0;

	unsigned int nTempBurnDrvSelect = nBurnDrvSelect;

	// 1st: parents
	for (iter = driverNode.begin(); iter != driverNode.end(); iter++) {
		nodeInfo = iter->second;
		nBurnDrvSelect = nodeInfo->driverId; // Switch to driver i

		// ==> game list filter, added by regret
		if (gameListFilter(TRUE)) {
			continue;
		}
		// <== game list filter

		TVINSERTSTRUCT TvItem;
		memset(&TvItem, 0, sizeof(TvItem));
		TvItem.item.mask = TVIF_TEXT | TVIF_PARAM;
		TvItem.hInsertAfter = TVI_SORT;
		TvItem.item.pszText = transGameName(BurnDrvGetText(nGetTextFlags | DRV_FULLNAME));
		TvItem.item.lParam = (LPARAM)nodeInfo;

		nodeInfo->hTreeHandle = TreeView_InsertItem(hSelList, &TvItem);
		nodeInfo->isParent = true;
		nodeInfo->used = true;

		nodeCount++;
	}

	// 2nd: clones
	for (iter = driverNode.begin(); iter != driverNode.end(); iter++) {
		nodeInfo = iter->second;
		nBurnDrvSelect = nodeInfo->driverId; // Switch to driver i

		// ==> game list filter, added by regret
		if (gameListFilter(FALSE)) {
			continue;
		}
		// <== game list filter

		TVINSERTSTRUCT TvItem;
		memset(&TvItem, 0, sizeof(TvItem));
		TvItem.item.mask = TVIF_TEXT | TVIF_PARAM;
		TvItem.hInsertAfter = TVI_SORT;
		TvItem.item.pszText = transGameName(BurnDrvGetText(nGetTextFlags | DRV_FULLNAME));

		if (!hasFilterStr || !bShowUnAvail) {
			// Find the parent's handle
			map<string, NODEINFO*>::iterator iter = driverNode.find(BurnDrvGetTextA(DRV_PARENT));
			if (iter != driverNode.end()) {
				if (iter->second->isParent) {
					TvItem.hParent = iter->second->hTreeHandle;
				}
			}

			// Find the parent and add a branch to the tree
			if (!TvItem.hParent) {
				unsigned int i = BurnDrvGetIndexByNameA(BurnDrvGetTextA(DRV_PARENT));
				if (i < nBurnDrvCount) {
					nBurnDrvSelect = i;

					iter1 = driverNode.find(BurnDrvGetTextA(DRV_NAME));
					nodeInfo1 = iter1->second;

					TVINSERTSTRUCT TempTvItem;
					memset(&TempTvItem, 0, sizeof(TempTvItem));
					TempTvItem.item.mask = TVIF_TEXT | TVIF_PARAM;
					TempTvItem.hInsertAfter = TVI_SORT;
					TempTvItem.item.pszText = transGameName(BurnDrvGetText(nGetTextFlags | DRV_FULLNAME));
					TempTvItem.item.lParam = (LPARAM)nodeInfo1;

					nodeInfo1->hTreeHandle = TreeView_InsertItem(hSelList, &TempTvItem);
					nodeInfo1->isParent = true;
					nodeInfo1->used = true;

//					TvItem.item.lParam = (LPARAM)nodeInfo1;
					TvItem.hParent = nodeInfo1->hTreeHandle;

					nodeCount++;
				}
			}
		}

		TvItem.item.lParam = (LPARAM)nodeInfo;
		nodeInfo->hTreeHandle = TreeView_InsertItem(hSelList, &TvItem);
		nodeInfo->used = true;

		nodeCount++;
		cloneCount++;
	}

#if 1
	for (iter = driverNode.begin(); iter != driverNode.end(); iter++) {
		// See if we need to expand the branch of an unavailable or non-working parent
		nodeInfo = iter->second;
		if (!nodeInfo->used) {
			continue;
		}

		if (nodeInfo->isParent && ((nLoadMenuShowX & AUTOEXPAND)
				|| getAuditState(nodeInfo->driverId) == AUDIT_FAIL
				|| !checkWorkingStatus(nodeInfo->driverId))) {
			for (iter1 = driverNode.begin(); iter1 != driverNode.end(); iter1++) {
				// Expand the branch only if a working clone is available
				nodeInfo1 = iter1->second;
				if (!nodeInfo1->used) {
					continue;
				}

				if (getAuditState(nodeInfo->driverId) != AUDIT_FAIL) {
					nBurnDrvSelect = nodeInfo1->driverId;
					if (BurnDrvGetTextA(DRV_PARENT) && iter->first == BurnDrvGetTextA(DRV_PARENT)) {
						TreeView_Expand(hSelList, nodeInfo->hTreeHandle, TVE_EXPAND);
						break;
					}
				}
			}
		}
	}
#endif

	nBurnDrvSelect = nTempBurnDrvSelect;

	return 0;
}

// preview image position, added by regret
static inline void initPreviewPos()
{
	RECT rect;
	GetWindowRect(GetDlgItem(hSelDlg, IDC_SCREENSHOT_H), &rect);
	POINT pt;
	pt.x = rect.left; pt.y = rect.top;
	ScreenToClient(hSelDlg, &pt);

	previewOrgX = pt.x;
	previewOrgY = pt.y;
}

static inline void movePreviewPos()
{
	extern void getResizeOffset(int& x, int& y);
	int offsetx, offsety;
	getResizeOffset(offsetx, offsety);

	SetWindowPos(GetDlgItem(hSelDlg, IDC_SCREENSHOT_H), NULL,
			previewOrgX + offsetx, previewOrgY + offsety, 0, 0,
			SWP_NOSIZE | SWP_NOSENDCHANGING | SWP_NOZORDER);
}

static inline void resetPreviewPos()
{
	SetWindowPos(GetDlgItem(hSelDlg, IDC_SCREENSHOT_H), NULL,
			previewOrgX, previewOrgY, 0, 0,
			SWP_NOSIZE | SWP_NOSENDCHANGING | SWP_NOZORDER);
}

static void refreshPanel()
{
	// clear preview shot
	if (hPreview) {
		DeleteObject((HGDIOBJ)hPreview);
		hPreview = NULL;
	}
	killImageTimer();

	SendDlgItemMessage(hSelDlg, IDC_SCREENSHOT_H, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hDefPreview);
	resetPreviewPos();

	CheckDlgButton(hSelDlg, IDC_CHECK7ZROMS, nLoadMenuShowX & CHECK7ZIP ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hSelDlg, IDC_MANGLENAME, nLoadMenuShowX & MANGLENAME ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hSelDlg, IDC_CHECKAUTOEXPAND, (nLoadMenuShowX & AUTOEXPAND) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hSelDlg, IDC_CHECKAVAILABLEONLY, (nLoadMenuShowX & AVAILONLY) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hSelDlg, IDC_SEL_SHORTNAME, nLoadMenuShowX & SHOWSHORT ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hSelDlg, IDC_SEL_ASCIIONLY, nLoadMenuShowX & ASCIIONLY ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hSelDlg, IDC_SEL_TRANSLANG, nLoadMenuShowX & TRANSLANG ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hSelDlg, IDC_CHECKCRC, nLoadMenuShowX & DISABLECRC ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hSelDlg, IDC_SEL_FAVORITE, nLoadMenuShowX & SHOWFAVORITE ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hSelDlg, IDC_NOICON, nLoadMenuShowX & NOICON ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hSelDlg, IDC_IMAGEKEEPGAMEASPECT, nLoadMenuShowX & KEEPGAMEASPECT ? BST_CHECKED : BST_UNCHECKED);
};

static void rebuildEverything()
{
	refreshPanel();

	bTreeBuilding = TRUE;
	SendMessage(hSelList, WM_SETREDRAW, (WPARAM)FALSE, (LPARAM)TVI_ROOT);	// disable redraw
	TreeView_DeleteItem(hSelList, TVI_ROOT);								// Destory all nodes
	hIconEffectItem = NULL;

	selListMake();
	SendMessage(hSelList, WM_SETREDRAW, (WPARAM)TRUE, (LPARAM)TVI_ROOT);	// enable redraw
	bTreeBuilding = FALSE;

	// set list focus
	if (nDialogSelect > -1) {
		unsigned int oldDrvSelect = nBurnDrvSelect;
		nBurnDrvSelect = nDialogSelect;
		map<string, NODEINFO*>::iterator iter = driverNode.find(BurnDrvGetTextA(DRV_NAME));
		TreeView_Select(hSelList, iter->second->hTreeHandle, TVGN_CARET);
		SendMessage(hSelDlg, WM_NEXTDLGCTL, (WPARAM)hSelList, TRUE);
		nBurnDrvSelect = oldDrvSelect;
	}

	// ==> display game count, added by regret
	TCHAR szGameCount[128] = _T("");
	_stprintf(szGameCount, FBALoadStringEx(IDS_SEL_GAMECOUNT), nodeCount, cloneCount);
	SetDlgItemText(hSelDlg, IDC_GAMECOUNT, szGameCount);
	// <== display game count
}

// get preview image path, added by regret
static inline void getImagePath(TCHAR* pszImgDir)
{
	int path = PATH_PREVIEW;
	for (int i = 0; imageDef[i].imageId >= 0; i++) {
		if (nCurImgTab == imageDef[i].imageId) {
			path = imageDef[i].pathId;
			break;
		}
	}

	_tcscpy(archiveName, getMiscArchiveName(path));
	_tcscpy(pszImgDir, getMiscPath(path));

#ifdef MD_DRIVER_LIST
	// get md image path
	if ((BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_SEGA_MEGADRIVE) {
		_tcscat(pszImgDir, _T("MD\\"));
	}
#endif
}

static inline BOOL getKeepImageAspect()
{
	if (nCurImgTab == IMG_FLYER) {
		return FALSE;
	}
	return (nLoadMenuShowX & KEEPGAMEASPECT);
}

// try to load image from archive file, added by regret
static HBITMAP getArchivePreviewImage(const TCHAR* szGameName, BOOL bResize)
{
	TCHAR szImgPath[MAX_PATH];
	getImagePath(szImgPath); // get image path of current tab

	const TCHAR* szParentName = BurnDrvGetText(DRV_PARENT);

	TCHAR szImgName[MAX_PATH];
	_stprintf(szImgName, _T("%s.png"), szGameName);

	TCHAR szArcName[MAX_PATH];
	_stprintf(szArcName, _T("%s%s"), szImgPath, archiveName);

	int ret = archiveCheck(szArcName);
	if (ret == ARC_NONE) {
		return NULL; // no archive
	}

	void* buf = NULL;
	int size = 0;
	ret = archiveLoadOneFile(szArcName, szImgName, &buf, &size);
	if (ret && szParentName) {
		// try parent
		_stprintf(szImgName, _T("%s.png"), szParentName);
		ret = archiveLoadOneFile(szArcName, szImgName, &buf, &size);
	}

	if (!ret) {
		HBITMAP hImage = loadImageFromBuffer(buf, size, hSelDlg, getKeepImageAspect(), bResize);
		free(buf);
		return hImage;
	}
	return NULL;
}

static HBITMAP getPreviewImage(BOOL bReset, BOOL bResize)
{
	if (nBurnDrvSelect >= nBurnDrvCount) {
		return NULL;
	}

	static int nIndex;
	int nOldIndex;

	TCHAR szBaseName[MAX_PATH];
	TCHAR szFileName[MAX_PATH];
	TCHAR szImgPath[MAX_PATH];
	TCHAR szGameName[MAX_PATH];
	FILE* fp = NULL;

	_tcscpy(szGameName, BurnDrvGetText(DRV_NAME));

	nOldIndex = nIndex;
	nIndex++;

	if (bReset) {
		nIndex = 1;
		nOldIndex = -1;
	}

	getImagePath(szImgPath); // get image path of current tab

	// get image name
	do {
		// Try to load a .PNG preview image
		_tcscpy(szBaseName, szImgPath);
		_tcscat(szBaseName, szGameName);

		if (nIndex == 1) {
			_stprintf(szFileName, _T("%s.png"), szBaseName);
			fp = _tfopen(szFileName, _T("rb"));
		}
		if (!fp) {
			_stprintf(szFileName, _T("%s [%02i].png"), szBaseName, nIndex);
			fp = _tfopen(szFileName, _T("rb"));
		}

		// Try the parent
		if (!fp && BurnDrvGetText(DRV_PARENT)) {
			_tcscpy(szBaseName, szImgPath);
			_tcscat(szBaseName, BurnDrvGetText(DRV_PARENT));
			if (nIndex == 1) {
				_stprintf(szFileName, _T("%s.png"), szBaseName);
				fp = _tfopen(szFileName, _T("rb"));
			}
			if (!fp) {
				_stprintf(szFileName, _T("%s [%02i].png"), szBaseName, nIndex);
				fp = _tfopen(szFileName, _T("rb"));
			}
		}

		if (nIndex == 1) {
			break;
		}

		if (!fp) {
			nIndex = 1;
		}
	} while (!fp);

	// get image handle
	HBITMAP hImage = NULL;
	if (fp && nIndex != nOldIndex) {
		hImage = loadImageFromFile(szFileName, hSelDlg, getKeepImageAspect(), bResize);
		fclose(fp);
	}

	// try to get image handle from archive
	if (hImage == NULL) {
		hImage = getArchivePreviewImage(szGameName, bResize);
	}

	return hImage;
}

static inline int updatePreview(BOOL bReset)
{
	if (bReset) {
		if (hPreview) {
			DeleteObject((HGDIOBJ)hPreview);
			hPreview = NULL;
		}
		killImageTimer();
	}

	nBurnDrvSelect = nDialogSelect;

	HBITMAP hNewImage = getPreviewImage(bReset, TRUE);

	// Show the preview image
	if (hNewImage) {
		DeleteObject((HGDIOBJ)hPreview);
		hPreview = hNewImage;

		SendDlgItemMessage(hSelDlg, IDC_SCREENSHOT_H, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hPreview);
		movePreviewPos();

		nImageTimer = SetTimer(hSelDlg, IMAGE_TIMER_ID, IMAGE_TIMER, NULL);
	} else {
		// We couldn't load a new image for this game, so kill the timer (it will be restarted when a new game is selected)
		killImageTimer();

		if (!hPreview) {
			SendDlgItemMessage(hSelDlg, IDC_SCREENSHOT_H, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hDefPreview);
			resetPreviewPos();
		}
	}

	return 0;
}

// ==> tab control, added by regret
static int setSelTab(int nTab)
{
	if (MAX_TAB < nTab)
		return -1;

	return TabCtrl_SetCurSel(hSelTab, nTab);
}

static void showOptionControls(const int& tabIndex, const bool& show)
{
	int flag = show ? SW_SHOW : SW_HIDE;

	switch (tabIndex) {
		case LIST:
			ShowWindow(GetDlgItem(hSelDlg, IDC_MANGLENAME), flag);
			ShowWindow(GetDlgItem(hSelDlg, IDC_SEL_SHORTNAME), flag);
			ShowWindow(GetDlgItem(hSelDlg, IDC_SEL_ASCIIONLY), flag);
			ShowWindow(GetDlgItem(hSelDlg, IDC_SEL_TRANSLANG), flag);
			ShowWindow(GetDlgItem(hSelDlg, IDC_NOICON), flag);
			ShowWindow(GetDlgItem(hSelDlg, IDC_IMAGEKEEPGAMEASPECT), flag);
			ShowWindow(GetDlgItem(hSelDlg, IDC_CHECK7ZROMS), flag);
			ShowWindow(GetDlgItem(hSelDlg, IDC_CHECKCRC), flag);
			break;

		case FILTER:
			ShowWindow(GetDlgItem(hSelDlg, IDC_CHECKAVAILABLEONLY), flag);
			ShowWindow(GetDlgItem(hSelDlg, IDC_CHECKAUTOEXPAND), flag);
			ShowWindow(GetDlgItem(hSelDlg, IDC_SEL_FAVORITE), flag);
			ShowWindow(GetDlgItem(hSelDlg, IDC_FILTER_KEY), flag);
			ShowWindow(GetDlgItem(hSelDlg, IDC_FILTER_SYSTEM), flag);
			ShowWindow(GetDlgItem(hSelDlg, IDC_DRIVERCONFIG), flag);
			break;

//		case ETC:
//			break;

		default:
			break;
	}
}

static void setOptionTab(int nTab)
{
	// Hide all controls
	if (nTab == MAX_TAB) {
		showOptionControls(LIST, false);
		showOptionControls(FILTER, false);
		return;
	}

	if (nCurrentTab == nTab || MAX_TAB < nTab)
		return;

	// Hide controls
	showOptionControls(nCurrentTab, false);

	// Show new controls
	nCurrentTab = nTab;
	nTabSel = nCurrentTab;
	showOptionControls(nCurrentTab, true);
}

static inline void updateBackgroundBrush(HWND hWndTab)
{
	// Check if the application is themed
	if (fnIsThemed) {
		bThemeActive = fnIsThemed();
	}
	// Destroy old brush
	if (hTabBkBrush) {
		DeleteObject(hTabBkBrush);
		hTabBkBrush = NULL;
	}

	// Only do this if the theme is active
	if (bThemeActive) {
		RECT rc;
		HDC hDC, hDCMem;
		HBITMAP hBmp, hBmpOld;

		// Get tab control dimensions
		GetWindowRect(hWndTab, &rc);

		// Get the tab control DC
		hDC = GetDC(hWndTab);

		// Create a compatible DC
		hDCMem = CreateCompatibleDC(hDC);
		hBmp = CreateCompatibleBitmap(hDC, rc.right-rc.left, rc.bottom-rc.top);
		hBmpOld = (HBITMAP)(SelectObject(hDCMem, hBmp));

		// Tell the tab control to paint in our DC
		SendMessage(hWndTab, WM_PRINTCLIENT, (WPARAM)(hDCMem),
			(LPARAM)(PRF_ERASEBKGND | PRF_CLIENT | PRF_NONCLIENT));

		// Create a pattern brush from the bitmap selected in our DC
		hTabBkBrush = CreatePatternBrush(hBmp);

		// Restore the bitmap
		SelectObject(hDCMem, hBmpOld);

		// Cleanup
		DeleteObject(hBmp);
		DeleteDC(hDCMem);
		ReleaseDC(hWndTab, hDC);
	}
}

static void initOptionTab()
{
	hSelTab = GetDlgItem(hSelDlg, IDC_SEL_TAB);
	nCurrentTab = NONE;

	HMODULE hThemes = LoadLibrary(_T("uxtheme.dll"));
	if (hThemes) {
		fnIsThemed = GetProcAddress(hThemes, "IsAppThemed");
		FreeLibrary(hThemes);

		updateBackgroundBrush(hSelTab);
	}
	bThemeActive = FALSE;

	// Clear all to be sure
	TabCtrl_DeleteAllItems(hSelTab);

	// init tab items
	TCITEM tie;
	memset(&tie, 0, sizeof(tie));
	tie.mask = TCIF_TEXT;

	for (int i = 0; i < MAX_TAB; i++) {
		tie.pszText = FBALoadStringEx(IDS_SELTAB_LIST + i);
		TabCtrl_InsertItem(hSelTab, i, &tie);
	}

	setOptionTab(MAX_TAB); // hide all controls
}

static void setImgTab(int nTab)
{
	if (nTab >= IMG_MAXTAB)
		return;

	nCurImgTab = nTab;

	if (nDialogSelect >= 0) {
		updatePreview(TRUE);
	}
}

static int getNextImgTab()
{
	if (nCurImgTab + 1 >= IMG_MAXTAB) {
		return IMG_NONE + 1;
	} else {
		return nCurImgTab + 1;
	}
}

static void initImageTab()
{
	hImgTab = GetDlgItem(hSelDlg, IDC_IMAGE_TAB);

	// Clear all to be sure
	TabCtrl_DeleteAllItems(hImgTab);

	// init tab items
	TCITEM tie;
	memset(&tie, 0, sizeof(tie));
	tie.mask = TCIF_TEXT;

	for (int i = 0; i < IMG_MAXTAB; i++) {
		tie.pszText = FBALoadStringEx(IDS_IMAGETAB_PREVIEW + i);
		TabCtrl_InsertItem(hImgTab, i, &tie);
	}

	TabCtrl_SetCurSel(hImgTab, nCurImgTab);
}
// <== tab control

static void displayRomInfo()
{
	char* pRomName;
	struct BurnRomInfo ri;
	TCHAR szTemp[256] = _T("");

	FBAPopupAddText(PUF_TYPE_INFO, _T("romset info for %s : \n"), BurnDrvGetText(DRV_NAME));
	FBAPopupAddText(PUF_TYPE_INFO, _T("format: rom name, size(in bytes), crc(hex) \n\n"));

	for (int i = 0; !BurnDrvGetRomName(&pRomName, i, 0); i++) {
		BurnDrvGetRomInfo(&ri, i);

		if (ri.nLen == 0 && ri.nCrc == 0) break;

		_stprintf(szTemp, _T("%hs, %d, %x\n"), pRomName, ri.nLen, ri.nCrc);

		FBAPopupAddText(PUF_TYPE_INFO, szTemp);
	}

	FBAPopupDisplay(PUF_TYPE_INFO);
}

static inline void onNodeItemClick(HTREEITEM item, bool recover)
{
	if ((nLoadMenuShowX & NOICON) || !item) {
		return;
	}

	TVITEMEX TvItem;
	TvItem.mask = TVIF_PARAM | TVIF_CHILDREN | TVIF_INTEGRAL;

//*
	if (hIconEffectItem) {
		TvItem.hItem = hIconEffectItem;
		TreeView_GetItem(hSelList, &TvItem);
		TvItem.iIntegral = 1;
		TreeView_SetItem(hSelList, &TvItem);
	}
//*/

	if (recover) {
		hIconEffectItem = NULL;
		return; // only recover state
	}
	hIconEffectItem = item;

//*
	TvItem.hItem = item;
	TreeView_GetItem(hSelList, &TvItem);
	TvItem.iIntegral = 2;
	TreeView_SetItem(hSelList, &TvItem);
//*/
}

static BOOL selContextMenuProc()
{
	// get mouse position
	POINT pt, menuPt;
	GetCursorPos(&pt);
	ScreenToClient(hSelList, &pt);
	GetCursorPos(&menuPt);

	TVHITTESTINFO lpht;
	memset(&lpht, 0, sizeof(lpht));
	lpht.pt.x = pt.x;
	lpht.pt.y = pt.y;

	// get driver number
	HTREEITEM hSelectHandle = TreeView_HitTest(hSelList, (LPARAM)(&lpht));
	if (!hSelectHandle) {		// Nothing is selected, return without closing the window
		return FALSE;
	}
	TreeView_SelectItem(hSelList, hSelectHandle);

	map<string, NODEINFO*>::iterator iter = driverNode.begin();
	for (; iter != driverNode.end(); iter++) {
		if (iter->second->used && hSelectHandle == iter->second->hTreeHandle) {
			nBurnDrvSelect = iter->second->driverId;
			break;
		}
	}

	HMENU hContextMenu = GetSubMenu(hMenuSelPop, 0);

#ifndef NO_IPS
	// IPS config menu item
	if (getPatchCount() > 0) {
		EnableMenuItem(hContextMenu, MENU_IPSSETTING, MF_ENABLED | MF_BYCOMMAND);
	} else {
		EnableMenuItem(hContextMenu, MENU_IPSSETTING, MF_GRAYED | MF_BYCOMMAND);
	}
#endif
	if (nLoadMenuShowX & SHOWFAVORITE) {
		EnableMenuItem(hContextMenu, MENU_ADD_FAVORITE, MF_GRAYED | MF_BYCOMMAND);
		EnableMenuItem(hContextMenu, MENU_REMOVE_FAVORITE, MF_ENABLED | MF_BYCOMMAND);
	}
	else {
		EnableMenuItem(hContextMenu, MENU_ADD_FAVORITE, MF_ENABLED | MF_BYCOMMAND);
		EnableMenuItem(hContextMenu, MENU_REMOVE_FAVORITE, MF_GRAYED | MF_BYCOMMAND);
	}

#ifndef NO_IMAGEMENU
	if (menuNewStyle) {
		createImageMenu(hSelDlg, hMenuSelPop);
	}
#endif
	TrackPopupMenuEx(hContextMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON, menuPt.x, menuPt.y, hSelDlg, NULL);

	return TRUE;
}

static INT_PTR CALLBACK selDialogProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	if (Msg == WM_INITDIALOG) {

		TCHAR szOldTitle[256] = _T("");
		TCHAR szNewTitle[512] = _T("");

		hSelDlg = hDlg;

		initNodeInfo(); // init node info

		// ==> load custom resource, modified by regret
		loadCustomRes();
		// <== load custom resource

		GetWindowText(hSelDlg, szOldTitle, sizearray(szOldTitle));
		_sntprintf(szNewTitle, sizearray(szOldTitle), _T(APP_TITLE) _T(SEPERATOR_1) _T("%s"), szOldTitle);
		SetWindowText(hSelDlg, szNewTitle);

		hSelList = GetDlgItem(hSelDlg, IDC_SELTREE1);

		for (int i = 0; i < INFO_NUM; i++) {
			hInfoLabel[i] = GetDlgItem(hSelDlg, infoDef[i].label);
			hInfoText[i] = GetDlgItem(hSelDlg, infoDef[i].text);
		}

#if 0 // notify user to set rom dirs at first run
		bool bFoundROMs = false;
		for (unsigned int i = 0; i < nBurnDrvCount; i++) {
			if (getAuditState(i) != AUDIT_FAIL) {
				bFoundROMs = true;
				break;
			}
		}
		if (!bFoundROMs) {
			RomsDirCreate(hDlg);
		}
#endif

		// ==> tab control, added by regret
		initOptionTab(); // initialize tab control
		setOptionTab(nTabSel);
		setSelTab(nTabSel);

		initImageTab();
		// <== tab control

		// ==> game list filter, added by regret
		initFilterString();
		initFilterSystem();
		// <== game list filter

		// ==> favorite filter, added by regret
		initFavorites();
		// <== favorite filter

		initPreviewPos();

#ifndef NO_IPS
		patchExit();
#endif

		rebuildEverything();

		wndInMid(hDlg, hScrnWnd);

		return TRUE;
	}

	if (Msg == WM_COMMAND) {
		if (HIWORD(wParam) == BN_CLICKED) {
			int wID = LOWORD(wParam);
			switch (wID) {
				case IDOK:
					selOkay();
					break;
				case IDC_DRIVERCONFIG:
					driverConfigDialog(hDlg);
					break;
				case IDCANCEL:
					SendMessage(hDlg, WM_CLOSE, 0, 0);
					return FALSE;

				case IDC_CHECK7ZROMS:
					nLoadMenuShowX ^= CHECK7ZIP;
					break;
				case IDC_CHECKCRC:
					nLoadMenuShowX ^= DISABLECRC;
					break;

				case IDC_IMAGEKEEPGAMEASPECT:
					nLoadMenuShowX ^= KEEPGAMEASPECT;
					updatePreview(TRUE);
					break;
				case IDC_NOICON:
					nLoadMenuShowX ^= NOICON;
					rebuildEverything();
					break;
				case IDC_MANGLENAME:
					nLoadMenuShowX ^= MANGLENAME;
					rebuildEverything();
					break;
				case IDC_CHECKAVAILABLEONLY:
					nLoadMenuShowX ^= AVAILONLY;
					rebuildEverything();
					break;
				case IDC_CHECKAUTOEXPAND:
					nLoadMenuShowX ^= AUTOEXPAND;
					rebuildEverything();
					break;
				case IDC_SEL_SHORTNAME:
					nLoadMenuShowX ^= SHOWSHORT;
					rebuildEverything();
					break;
				case IDC_SEL_ASCIIONLY:
					nLoadMenuShowX ^= ASCIIONLY;
					rebuildEverything();
					break;
				case IDC_SEL_FAVORITE:
					nLoadMenuShowX ^= SHOWFAVORITE;
					rebuildEverything();
					break;
				/* modify by Sho, thanks 800 */
				case IDC_SEL_TRANSLANG:
					nLoadMenuShowX ^= TRANSLANG;
					rebuildEverything();
					break;
				/* end Sho, thanks 800 */
			}
		}

		int id = LOWORD(wParam);
		switch (id) {
			case MENU_PLAY_GAME:
				bRClick = TRUE; // play game through context menu
				selOkay();
				break;

			case MENU_MAWS_INFO: {
				TCHAR szRomURL[MAX_PATH] = _T("");
				_stprintf(szRomURL, _T("%s%s"), _T("http://maws.mameworld.info/maws/set/"), BurnDrvGetText(DRV_NAME));
				ShellExecute(NULL, _T("open"), szRomURL, NULL, NULL, SW_SHOWNORMAL);
				break;
			}

			case MENU_ROM_INFO: {
				displayRomInfo();
				break;
			}

#ifndef NO_IPS
			case MENU_IPSSETTING:
				patchSettingCreate(hDlg);
				break;
#endif

			case MENU_ADD_FAVORITE:
				addFavorite(nBurnDrvSelect);
				break;

			case MENU_REMOVE_FAVORITE:
				removeFavorite(nBurnDrvSelect);
				rebuildEverything();
				break;
		}

		// ==> tab control, added by regret
		if (id == IDC_IMGFRAME) {
			int nNextImgTab = getNextImgTab();
			TabCtrl_SetCurSel(hImgTab, nNextImgTab);
			setImgTab(nNextImgTab);
		}
		// <==> tab control

		// ==> game list filter, added by regret
		if ((HWND)lParam == GetDlgItem(hDlg, IDC_FILTER_KEY)) {
			if (HIWORD(wParam) == CBN_SELCHANGE) {
				int iSel = SendDlgItemMessage(hDlg, IDC_FILTER_KEY, CB_GETCURSEL, 0, 0);
				SendDlgItemMessage(hDlg, IDC_FILTER_KEY, CB_GETLBTEXT, (WPARAM)iSel, (LPARAM)szFilterStr);
				filterChange();
			}
			if (HIWORD(wParam) == CBN_EDITCHANGE) {
				nFilterTimer = SetTimer(hSelDlg, FILTER_TIMER_ID, FILTER_TIMER, NULL);
			}
		}
		else if ((HWND)lParam == GetDlgItem(hDlg, IDC_FILTER_SYSTEM)) {
			if (HIWORD(wParam) == CBN_SELCHANGE) {
				nSystemSel = SendDlgItemMessage(hDlg, IDC_FILTER_SYSTEM, CB_GETCURSEL, 0, 0);
				rebuildEverything();
			}
		}
		// <== game list filter
	}

	if (Msg == WM_CLOSE) {
		nDialogSelect = -1;
		myEndDialog();
		DeleteObject(hWhiteBGBrush);
		DeleteObject(hTabBkBrush);
		return FALSE;
	}

	if (Msg == WM_TIMER) {
		if ((UINT_PTR)wParam == nImageTimer) {
			updatePreview(FALSE);
		}
		// ==> game list filter, added by regret
		else if((UINT_PTR)wParam == nFilterTimer) {
			killFilterTimer();
			SendDlgItemMessage(hDlg, IDC_FILTER_KEY, WM_GETTEXT, (WPARAM)sizearray(szFilterStr), (LPARAM)szFilterStr);
			filterChange();
		}
		// <== game list filter
		return FALSE;
	}

	if (Msg == WM_CTLCOLORSTATIC) {
		for (int i = 0; i < INFO_NUM; i++) {
			if ((HWND)lParam == hInfoLabel[i]) {
				return (BOOL)hWhiteBGBrush;
			}
			if ((HWND)lParam == hInfoText[i]) {
				return (BOOL)hWhiteBGBrush;
			}
		}

		// ==> tab control, added by regret
		if ((HWND)lParam == GetDlgItem(hSelDlg, IDC_GAMECOUNT)) {
			return (BOOL)hWhiteBGBrush;
		} else {
			if (fnIsThemed && fnIsThemed()) {
				RECT rc;

				// Set the background mode to transparent
				SetBkMode((HDC)wParam, TRANSPARENT);

				// Get the controls window dimensions
				GetWindowRect((HWND)lParam, &rc);

				// Map the coordinates to coordinates with the upper left corner of dialog control as base
				MapWindowPoints(NULL, hSelTab, (LPPOINT)(&rc), 2);

				// Adjust the position of the brush for this control (else we see the top left of the brush as background)
				SetBrushOrgEx((HDC)wParam, -rc.left, -rc.top, NULL);

				// Return the brush
				return (INT_PTR)(hTabBkBrush);
			} else {
				SetBkColor((HDC)wParam, GetSysColor(COLOR_3DFACE));
			}
		}
		// <== tab control
	}

	if (Msg == WM_NOTIFY) {
		LPNMHDR lpnmh = (LPNMHDR)lParam;

		if (lpnmh->code == NM_DBLCLK && LOWORD(wParam) == IDC_SELTREE1) {
			selOkay();

			// disable double-click node-expand
			SetWindowLongPtr(hSelDlg, DWLP_MSGRESULT, 1);
			return TRUE;
		}

		// F5 to refresh romsets
		if (lpnmh->code == TVN_KEYDOWN && !bTreeBuilding && LOWORD(wParam) == IDC_SELTREE1) {
			NMTVKEYDOWN* ptvkd = (LPNMTVKEYDOWN)lParam;
			if (ptvkd->wVKey == VK_F5 && !bRescanRoms) {
				bRescanRoms = true;
				CreateROMInfo();
				rebuildEverything();
				return TRUE;
			}
			return FALSE;
		}

		// ==> tab control, added by regret
		if (lpnmh->code == TCN_SELCHANGE) {
			if (lpnmh->hwndFrom == hSelTab) {
				int iTab = TabCtrl_GetCurSel(hSelTab);
				setOptionTab(iTab);
			}
			else if(lpnmh->hwndFrom == hImgTab) {
				int iImgTab = TabCtrl_GetCurSel(hImgTab);
				setImgTab(iImgTab);
			}
		}
		// <== tab control

		if (lpnmh->code == NM_RCLICK && LOWORD(wParam) == IDC_SELTREE1) {
			return selContextMenuProc();
		}

		if (lpnmh->code == NM_CUSTOMDRAW && LOWORD(wParam) == IDC_SELTREE1) {
			LPNMLVCUSTOMDRAW lplvcd = (LPNMLVCUSTOMDRAW)lParam;

			switch (lplvcd->nmcd.dwDrawStage) {
				case CDDS_PREPAINT: {
					SetWindowLongPtr(hSelDlg, DWLP_MSGRESULT, CDRF_NOTIFYITEMDRAW);
					return TRUE;
				}

				case CDDS_ITEMPREPAINT:	{
					if (lplvcd->nmcd.rc.top == lplvcd->nmcd.rc.bottom) {
						return TRUE;
					}

					HTREEITEM hSelectHandle = (HTREEITEM)(lplvcd->nmcd.dwItemSpec);
					TVITEMEX TvItem;
					TvItem.hItem = hSelectHandle;
					TvItem.mask = TVIF_PARAM | TVIF_CHILDREN | TVIF_INTEGRAL;
					TreeView_GetItem(hSelList, &TvItem);

//					dprintf(_T("  - Item (%ixi) - (%ixi) %hs\n"),
//						lplvcd->nmcd.rc.left, lplvcd->nmcd.rc.top, lplvcd->nmcd.rc.right, lplvcd->nmcd.rc.bottom,
//						((NODEINFO*)TvItem.lParam)->pszROMName);

					static RECT rect;
					rect.left = lplvcd->nmcd.rc.left;
					rect.right = lplvcd->nmcd.rc.right;
					rect.top = lplvcd->nmcd.rc.top;
					rect.bottom = lplvcd->nmcd.rc.bottom;

					const NODEINFO* node = (NODEINFO*)TvItem.lParam;

					// Set the foreground and background colours unless the item is highlighted
					if (nLoadMenuShowX & NOICON) {
						if (!(TvItem.state & (TVIS_SELECTED | TVIS_DROPHILITED))) {
							// Set less contrasting colours for clones
							if (!node->isParent) {
//								lplvcd->clrTextBk = RGB(0xF7, 0xF7, 0xF7);
								lplvcd->clrText = RGB(0x3F, 0x3F, 0x3F);
							}
							// Miss sets
							if (getAuditState(node->driverId) == AUDIT_FAIL) {
								lplvcd->clrTextBk = RGB(0xEC, 0xEC, 0xEC);
							}
						}
					}

					// Not working games, change the text colour
					if (!checkWorkingStatus(node->driverId)) {
						lplvcd->clrText = RGB(0x80, 0x00, 0x00);
					}

					{
						// Fill background
						HBRUSH hBackBrush = CreateSolidBrush(lplvcd->clrTextBk);
						FillRect(lplvcd->nmcd.hdc, &lplvcd->nmcd.rc, hBackBrush);
						DeleteObject(hBackBrush);
					}

					{
						// save original select driver
						unsigned int nOldnBurnDrvSelect = nBurnDrvSelect;
						nBurnDrvSelect = node->driverId;

						// Draw icons if needed
						drawIcons(TvItem, rect, lplvcd->nmcd.hdc);

						// Draw text
						static TCHAR szText[512] = _T("");

						SetTextColor(lplvcd->nmcd.hdc, lplvcd->clrText);
						SetBkMode(lplvcd->nmcd.hdc, TRANSPARENT);

						int nGetTextFlags = (nLoadMenuShowX & ASCIIONLY) ? DRV_ASCIIONLY : 0;
						TCHAR* pszName = transGameName(BurnDrvGetText(nGetTextFlags | DRV_FULLNAME));

						SIZE size = { 0, 0 };
						GetTextExtentPoint32(lplvcd->nmcd.hdc, pszName, _tcslen(pszName), &size);

						DrawText(lplvcd->nmcd.hdc, pszName, -1, &rect, DT_NOPREFIX | DT_SINGLELINE | DT_LEFT | DT_VCENTER);

						// Display extra info if needed
						if ((nLoadMenuShowX & TRANSLANG) == 0) {
							szText[0] = _T('\0');
							TCHAR* pszPosition = szText;

							pszName = BurnDrvGetText(nGetTextFlags | DRV_FULLNAME);
							while ((pszName = BurnDrvGetText(nGetTextFlags | DRV_NEXTNAME | DRV_FULLNAME)) != NULL) {
								if (pszPosition + _tcslen(pszName) - sizearray(szText) > szText) {
									break;
								}
								pszPosition += _stprintf(pszPosition, _T(SEPERATOR_2) _T("%s"), pszName);
							}
							if (szText[0]) {
								szText[511] = _T('\0');

//								unsigned int r = ((lplvcd->clrText >> 16 & 255) * 2 + (lplvcd->clrTextBk >> 16 & 255)) / 3;
//								unsigned int g = ((lplvcd->clrText >>  8 & 255) * 2 + (lplvcd->clrTextBk >>  8 & 255)) / 3;
//								unsigned int b = ((lplvcd->clrText >>  0 & 255) * 2 + (lplvcd->clrTextBk >>  0 & 255)) / 3;

								rect.left += size.cx;
//								SetTextColor(lplvcd->nmcd.hdc, (r << 16) | (g <<  8) | (b <<  0));
								SetTextColor(lplvcd->nmcd.hdc, RGB(0x6F, 0x6F, 0x6F));
								DrawText(lplvcd->nmcd.hdc, szText, -1, &rect, DT_NOPREFIX | DT_SINGLELINE | DT_LEFT | DT_VCENTER);
							}
						}

						nBurnDrvSelect = nOldnBurnDrvSelect; // return original select driver
					}

					SetWindowLongPtr(hSelDlg, DWLP_MSGRESULT, CDRF_SKIPDEFAULT);
					return TRUE;
				}

				default: {
					SetWindowLongPtr(hSelDlg, DWLP_MSGRESULT, CDRF_DODEFAULT);
					return TRUE;
				}
			}
		}

		if (lpnmh->code == TVN_ITEMEXPANDING && !bTreeBuilding && LOWORD(wParam) == IDC_SELTREE1) {
			TreeView_Select(hSelList, ((LPNMTREEVIEW)lParam)->itemNew.hItem, TVGN_CARET);
			return FALSE;
		}

		if (lpnmh->code == TVN_SELCHANGED && !bTreeBuilding && LOWORD(wParam) == IDC_SELTREE1) {
			HTREEITEM hSelectHandle = TreeView_GetNextItem(hSelList, ~0U, TVGN_CARET);

			// Search through driverNode[] for the driverId according to the returned hSelectHandle
			map<string, NODEINFO*>::iterator iter = driverNode.begin();
			for (; iter != driverNode.end(); iter++) {
				if (iter->second->used && hSelectHandle == iter->second->hTreeHandle) {
					nBurnDrvSelect = iter->second->driverId;

					// icon effect
					if (iter->second->iconid >= 0) {
						onNodeItemClick(hSelectHandle, false);
					} else {
						onNodeItemClick(hSelectHandle, true);
					}
					break;
				}
			}

			nDialogSelect = nBurnDrvSelect;

			int nGetTextFlags = (nLoadMenuShowX & ASCIIONLY) ? DRV_ASCIIONLY : 0;
			TCHAR szItemText[1024] = _T("");

			// Get the text from the drivers via BurnDrvGetText()
			for (int i = 0; i < INFO_NUM; i++) {
				BOOL bUseInfo = TRUE;

				switch (i) {
					case INFO_ROMNAME: {
						BOOL bBracket = FALSE;

						_stprintf(szItemText, _T("%s"), BurnDrvGetText(DRV_NAME));
						if ((BurnDrvGetFlags() & BDF_CLONE) && BurnDrvGetTextA(DRV_PARENT)) {
							unsigned int nOldDrvSelect = nBurnDrvSelect;
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
							bBracket = TRUE;
						} else {
							if (BurnDrvGetTextA(DRV_PARENT)) {
								_stprintf(szItemText + _tcslen(szItemText), _T("%suses ROMs from %s"),
									bBracket ? _T(", ") : _T(" ("), BurnDrvGetText(DRV_PARENT));
								bBracket = TRUE;
							}
						}
						if (bBracket) {
							_stprintf(szItemText + _tcslen(szItemText), _T(")"));
						}
						break;
					}
					case INFO_ROMINFO: {
						_stprintf(szItemText, _T("")); // clear rom name
						bUseInfo = FALSE;

						if (BurnDrvGetFlags() & BDF_PROTOTYPE) {
							_stprintf(szItemText + _tcslen(szItemText), _T("prototype"));
							bUseInfo = TRUE;
						}
						if (BurnDrvGetFlags() & BDF_BOOTLEG) {
							_stprintf(szItemText + _tcslen(szItemText), _T("%sbootleg"), bUseInfo ? _T(", ") : _T(""));
							bUseInfo = TRUE;
						}

						_stprintf(szItemText + _tcslen(szItemText), _T("%s%i player%s"),
								bUseInfo ? _T(", ") : _T(""), BurnDrvGetMaxPlayers(),
								(BurnDrvGetMaxPlayers() != 1) ? _T("s max") : _T(""));
						bUseInfo = TRUE;

						if (BurnDrvGetText(DRV_BOARDROM)) {
							_stprintf(szItemText + _tcslen(szItemText), _T("%suses board-ROMs from %s"),
									bUseInfo ? _T(", ") : _T(""), BurnDrvGetText(DRV_BOARDROM));
						}
						break;
					}
					case INFO_SYSTEM: {
						int width, height, aspectx, aspecty;
						BurnDrvGetVisibleSize(&width, &height);
						BurnDrvGetAspect(&aspectx, &aspecty);

						_stprintf(szItemText, _T("%s (%s, %s, %dx%d, %d:%d)"),
							BurnDrvGetTextA(DRV_MANUFACTURER) ? BurnDrvGetText(nGetTextFlags | DRV_MANUFACTURER) : FBALoadStringEx(IDS_ERR_UNKNOWN),
							BurnDrvGetText(DRV_DATE), BurnDrvGetText(nGetTextFlags | DRV_SYSTEM),
							width, height, aspectx, aspecty);
						break;
					}
					case INFO_COMMENT: {
						TCHAR* pszPosition = szItemText;
						TCHAR* pszName = transGameName(BurnDrvGetText(nGetTextFlags | DRV_FULLNAME));

						pszPosition += _sntprintf(szItemText, sizearray(szItemText), pszName);

						if ((nLoadMenuShowX & TRANSLANG) == 0) {
							pszName = BurnDrvGetText(nGetTextFlags | DRV_FULLNAME);
							while ((pszName = BurnDrvGetText(nGetTextFlags | DRV_NEXTNAME | DRV_FULLNAME)) != NULL) {
								if (pszPosition + _tcslen(pszName) - sizearray(szItemText) > szItemText) {
									break;
								}
								pszPosition += _stprintf(pszPosition, _T(SEPERATOR_2) _T("%s"), pszName);
							}
						}
						if (!szItemText[0]) {
							bUseInfo = FALSE;
						}
						break;
					}
					case INFO_NOTE: {
						const char* pszTmp = BurnDrvGetTextA(DRV_COMMENT);
						if (!pszTmp || !pszTmp[0]) bUseInfo = FALSE;
						_stprintf(szItemText, _T("%s"), bUseInfo ? BurnDrvGetText(nGetTextFlags | DRV_COMMENT) : _T(""));
						break;
					}
				}
				// set game info text
				SendMessage(hInfoText[i], WM_SETTEXT, 0, (LPARAM)szItemText);
				EnableWindow(hInfoLabel[i], bUseInfo);
			}

			updatePreview(TRUE);
		}
	}

	return FALSE;
}

int selDialog(HWND parent)
{
	unsigned int nOldSelect = nBurnDrvSelect;

	FBADialogBox(IDD_SELNEW, parent, (DLGPROC)selDialogProc);

	hSelDlg = NULL;
	hSelList = NULL;
	hIconEffectItem = NULL;

	nBurnDrvSelect = nOldSelect;

	return nDialogSelect;
}


// driver config dialog, added by regret
HWND hDriverDlg = NULL;

static void refreshDriverConfig()
{
	if (!hDriverDlg) {
		return;
	}

	for (int i = 0; systemDef[i].buttonCode; i++) {
		if (systemDef[i].buttonCode > 0) {
			CheckDlgButton(hDriverDlg, systemDef[i].buttonCode,
				(nLoadDriverShowX & systemDef[i].hardware) ? BST_UNCHECKED : BST_CHECKED);
		}
	}
}

static void setDriverConfig(const int& id)
{
	for (int i = 0; systemDef[i].buttonCode; i++) {
		if (systemDef[i].buttonCode > 0 && systemDef[i].buttonCode == id) {
			nLoadDriverShowX ^= systemDef[i].hardware;
			break;
		}
	}
}

static INT_PTR CALLBACK driverConfigProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM)
{
	if (Msg ==  WM_INITDIALOG) {
		hDriverDlg = hDlg;
		wndInMid(hDlg, hSelDlg ? hSelDlg : hScrnWnd);
		SetFocus(hDlg); // Enable Esc=close

		refreshDriverConfig();
		return TRUE;
	}
	else if (Msg ==  WM_COMMAND) {
		if (HIWORD(wParam) == BN_CLICKED) {
			int id = LOWORD(wParam);
			switch (id) {
				case IDOK:
				case IDCANCEL:
					SendMessage(hDlg, WM_CLOSE, 0, 0);
					break;

				case IDC_DRIVERALL:
					nLoadDriverShowX = 0;
					refreshDriverConfig();
					break;
				case IDC_DRIVERNONE:
					nLoadDriverShowX = MASKALL;
					refreshDriverConfig();
					break;

				default:
					setDriverConfig(id);
					break;
			}
		}
	}
	else if (Msg ==  WM_CLOSE) {
		EndDialog(hDlg, 0);
	}

	return FALSE;
}

int driverConfigDialog(HWND parent)
{
	FBADialogBox(IDD_DRIVER, parent, (DLGPROC)driverConfigProc);
	hDriverDlg = NULL;

	rebuildEverything();

	return 0;
}
