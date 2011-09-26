// misc win32 functions
#include "burner.h"

// Set the current directory to be the application's directory
int appDirectory()
{
	TCHAR szPath[MAX_PATH] = _T("");
	int nLen = 0;
	TCHAR *pc1, *pc2;
	TCHAR* szCmd = GetCommandLine();

	// Find the end of the "c:\directory\program.exe" bit
	if (szCmd[0] == _T('\"')) {						// Filename is enclosed in quotes
		szCmd++;
		for (pc1 = szCmd; *pc1; pc1++) {
			if (*pc1 == _T('\"')) break;			// Find the last "
		}
	} else {
		for (pc1 = szCmd; *pc1; pc1++) {
			if (*pc1 == _T(' ')) break;				// Find the first space
		}
	}
	// Find the last \ or /
	for (pc2 = pc1; pc2 >= szCmd; pc2--) {
		if (*pc2 == _T('\\')) break;
		if (*pc2 == _T('/')) break;
	}

	// Copy the name of the executable into a variable
	nLen = pc1 - pc2 - 1;
	if (nLen > EXE_NAME_SIZE) {
		nLen = EXE_NAME_SIZE;
	}
	_tcsncpy(szAppExeName, pc2 + 1, nLen);
	szAppExeName[nLen] = 0;

	// strip .exe
	if ((pc1 = _tcschr(szAppExeName, _T('.'))) != 0) {
		*pc1 = 0;
	}

	nLen = pc2 - szCmd;
	if (nLen <= 0) return 1;			// No path

	// Now copy the path into a new buffer
	_tcsncpy(szPath, szCmd, nLen);
	SetCurrentDirectory(szPath);		// Finally set the current directory to be the application's directory

	dprintf(szPath);
	dprintf(_T("\n"));

	return 0;
}

// ---------------------------------------------------------------------------

// Get the position of the client area of a window on the screen
int getClientScreenRect(HWND hWnd, RECT *pRect)
{
	POINT Corner = {0, 0};

	GetClientRect(hWnd, pRect);
	if (!nVidFullscreen) {
		ClientToScreen(hWnd, &Corner);
	}

	pRect->left += Corner.x;
	pRect->right += Corner.x;
	pRect->top += Corner.y;
	pRect->bottom += Corner.y;

	return 0;
}

// Put a window in the middle of another window
int wndInMid(HWND hMid, HWND hBase)
{
	RECT MidRect = {0, 0, 0, 0};
	int mw = 0, mh = 0;
	RECT BaseRect = {0, 0, 0, 0};
	int bx = 0, by = 0;

	// Find the height and width of the Mid window
	GetWindowRect(hMid, &MidRect);
	mw = MidRect.right - MidRect.left;
	mh = MidRect.bottom - MidRect.top;

	// Find the center of the Base window
	if (hBase && IsWindowVisible(hBase)) {
		GetWindowRect(hBase, &BaseRect);
		if (hBase == hScrnWnd) {
			// For the main window, center in the client area.
			BaseRect.left += GetSystemMetrics(SM_CXSIZEFRAME);
			BaseRect.right -= GetSystemMetrics(SM_CXSIZEFRAME);
			BaseRect.top += GetSystemMetrics(SM_CYSIZEFRAME);
			if (bMenuEnabled) {
				BaseRect.top += GetSystemMetrics(SM_CYCAPTION);
			}
			BaseRect.bottom -= GetSystemMetrics(SM_CYSIZEFRAME);
		}
	} else {
		SystemParametersInfo(SPI_GETWORKAREA, 0, &BaseRect, 0);
	}

	bx = BaseRect.left + BaseRect.right;
	bx = (bx - mw) >> 1;
	by = BaseRect.top + BaseRect.bottom;
	by = (by - mh) >> 1;

	if (hBase) {
		SystemParametersInfo(SPI_GETWORKAREA, 0, &SystemWorkArea, 0);

		if (bx + mw > SystemWorkArea.right) {
			bx = SystemWorkArea.right - mw;
		}
		if (by + mh > SystemWorkArea.bottom) {
			by = SystemWorkArea.bottom - mh;
		}
		if (bx < SystemWorkArea.left) {
			bx = SystemWorkArea.left;
		}
		if (by < SystemWorkArea.top) {
			by = SystemWorkArea.top;
		}
	}

	// Center the window
	SetWindowPos(hMid, NULL, bx, by, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

	return 0;
}

// ==> other functions, added by regret
void setWindowAspect()
{
	int width = GetSystemMetrics(SM_CXSCREEN);
	int height = GetSystemMetrics(SM_CYSCREEN);

	if (width <= 0 || height <= 0) {
		return;
	}

	if (autoVidScrnAspect) {
		vidScrnAspect = (float)width / height;
	} else {
		vidScrnAspect = (float)nVidScrnAspectX / nVidScrnAspectY;
	}
}

void createNeedDir()
{
	CreateDirectory(_T("config"), NULL);
	CreateDirectory(_T("config\\games"), NULL);
	CreateDirectory(_T("config\\presets"), NULL);
	CreateDirectory(_T("config\\macros"), NULL); // macros
	CreateDirectory(_T("config\\ips"), NULL); // ips config
//	CreateDirectory(_T("config\\localisation"), NULL);
//	CreateDirectory(_T("config\\memcards"), NULL);
//	CreateDirectory(_T("savestates"), NULL);
//	CreateDirectory(_T("cheats"), NULL);
//	CreateDirectory(_T("previews"), NULL);
//	CreateDirectory(_T("recordings"), NULL);
//	CreateDirectory(_T("roms"), NULL);
//	CreateDirectory(_T("screenshots"), NULL);
}

bool directoryExists(const TCHAR* dirname)
{
	if (!dirname) {
		return false;
	}

	DWORD res = GetFileAttributes(dirname);
	return res != 0xFFFFFFFF && res & FILE_ATTRIBUTE_DIRECTORY;
}

int directLoadGame(TCHAR* name)
{
	if (!name) {
		return 1;
	}

	if (bFullscreenOnStart) {
		nVidFullscreen = 1;
	}

	if (_tcsicmp(&name[_tcslen(name) - 3], _T(".fs")) == 0) {
		if (BurnStateLoad(name, 1, &DrvInitCallback)) {
			return 1;
		}
	}
	else if (_tcsicmp(&name[_tcslen(name) - 3], _T(".fr")) == 0) {
		if (StartReplay(name)) {
			return 1;
		}
	}
	else {
		// get game name
		TCHAR* p = getBaseName(name);

		// load game
		unsigned int i = BurnDrvGetIndexByName(p);
		if (i < nBurnDrvCount) {
			BurnerDrvInit(i, true);
			bAltPause = 0;
		}
	}

	return 0;
}

// create dialog item tooltip
bool createToolTip(int toolID, HWND hDlg, TCHAR* pText)
{
	// toolID:   the resource ID of the control.
	// hDlg:     the handle of the dialog box.
	// pText:    the text that appears in the tooltip.
	// hAppInst: the global instance handle.

	if (!toolID || !hDlg || !pText) {
		return false;
	}

	// Get the window of the tool.
	HWND hwndTool = GetDlgItem(hDlg, toolID);

	// Create the tooltip.
	HWND hwndTip = CreateWindowEx(NULL, TOOLTIPS_CLASS, NULL,
								WS_POPUP | TTS_ALWAYSTIP,
								CW_USEDEFAULT, CW_USEDEFAULT,
								CW_USEDEFAULT, CW_USEDEFAULT,
								hDlg, NULL,
								hAppInst, NULL);

	if (!hwndTool || !hwndTip) {
		return false;
	}

	// Associate the tooltip with the tool.
	TOOLINFO toolInfo = { 0 };
	toolInfo.cbSize = sizeof(toolInfo);
	toolInfo.hwnd = hDlg;
	toolInfo.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
	toolInfo.uId = (UINT_PTR)hwndTool;
	toolInfo.lpszText = pText;
	SendMessage(hwndTip, TTM_ADDTOOL, 0, (LPARAM)&toolInfo);

	return true;
}
// <== other functions

// ---------------------------------------------------------------------------
// For DAT files printing and Kaillera windows

char* decorateGameName(unsigned int drv)
{
	if (drv >= nBurnDrvCount)
		return "";

	unsigned int nOldBurnDrv = nBurnDrvSelect;
	nBurnDrvSelect = drv;

	// get game full name
	static char szDecoratedName[1024] = "";
	strcpy(szDecoratedName, BurnDrvGetTextA(DRV_FULLNAME));

	// get game extra info
	char szGameInfo[256] = " [";
	bool hasInfo = false;

	if (BurnDrvGetFlags() & BDF_PROTOTYPE) {
		strcat(szGameInfo, "prototype");
		hasInfo = true;
	}
	if (BurnDrvGetFlags() & BDF_BOOTLEG) {
		strcat(szGameInfo, "bootleg");
		hasInfo = true;
	}
	if (BurnDrvGetTextA(DRV_COMMENT)) {
		strcat(szGameInfo, BurnDrvGetTextA(DRV_COMMENT));
		hasInfo = true;
	}

	if (hasInfo) {
		strcat(szGameInfo, "]");
		strcat(szDecoratedName, szGameInfo);
	}

	nBurnDrvSelect = nOldBurnDrv;

	return szDecoratedName;
}

char* decorateKailleraGameName(unsigned int drv)
{
	if (drv >= nBurnDrvCount)
		return "";

	unsigned int nOldBurnDrv = nBurnDrvSelect;
	nBurnDrvSelect = drv;

	// get game full name, modified by regret
	static char szDecoratedName[1024] = "";
	sprintf(szDecoratedName, "%s [%s]", BurnDrvGetTextA(DRV_FULLNAME), BurnDrvGetTextA(DRV_NAME));

	nBurnDrvSelect = nOldBurnDrv;

	return szDecoratedName;
}

// rom util
static inline int findRomByName(const char* name, ArcEntry* list, int count)
{
	if (!name || !list) {
		return -1;
	}

	// Find the rom named name in the List
	for (int i = 0; i < count; i++, list++) {
		if (list->szName && !_stricmp(name, getFilenameA(list->szName))) {
			return i;
		}
	}
	return -1; // couldn't find the rom
}

static inline int findRomByCrc(unsigned int crc, ArcEntry* list, int count)
{
	if (!list) {
		return -1;
	}

	// Find the rom named name in the List
	for (int i = 0; i < count; i++, list++)	{
		if (crc == list->nCrc) {
			return i;
		}
	}

	return -1; // couldn't find the rom
}

// Find rom number i from the pBzipDriver game
int findRom(int i, ArcEntry* list, int count)
{
	BurnRomInfo ri;
	memset(&ri, 0, sizeof(ri));

	int nRet = BurnDrvGetRomInfo(&ri, i);
	if (nRet != 0) {						// Failure: no such rom
		return -2;
	}

	if (ri.nCrc) {							// Search by crc first
		nRet = findRomByCrc(ri.nCrc, list, count);
		if (nRet >= 0) {
			return nRet;
		}
	}

	for (int nAka = 0; nAka < 0x10000; nAka++) {	// Failing that, search for possible names
		char* szPossibleName = NULL;
		nRet = BurnDrvGetRomName(&szPossibleName, i, nAka);
		if (nRet) {							// No more rom names
			break;
		}
		nRet = findRomByName(szPossibleName, list, count);
		if (nRet >= 0) {
			return nRet;
		}
	}

	return -1; // Couldn't find the rom
}


#if 0
bool SetNumLock(bool state)
{
	BYTE keyState[256];

	GetKeyboardState(keyState);
	if ((state && !(keyState[VK_NUMLOCK] & 1)) || (!state && (keyState[VK_NUMLOCK] & 1))) {
		keybd_event(VK_NUMLOCK, 0, KEYEVENTF_EXTENDEDKEY, 0 );

		keybd_event(VK_NUMLOCK, 0, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
	}

	return keyState[VK_NUMLOCK] & 1;
}

static void MyRegCreateKeys(int nDepth, TCHAR* pNames[], HKEY* pResult)
{
	for (int i = 0; i < nDepth; i++) {
		pResult[i] = NULL;
		RegCreateKeyEx((i ? pResult[i - 1] : HKEY_CLASSES_ROOT), pNames[i], 0, _T(""), 0, KEY_WRITE, NULL, &pResult[i], NULL);
	}
}

static void MyRegCloseKeys(int nDepth, HKEY* pKeys)
{
	for (int i = nDepth - 1; i >= 0; i--) {
		if (pKeys[i]) {
			RegCloseKey(pKeys[i]);
		}
	}
}

static void MyRegDeleteKeys(int nDepth, TCHAR* pNames[], HKEY* pKeys)
{
	for (int i = 0; i < nDepth - 1; i++) {
		pKeys[i] = NULL;
		RegOpenKeyEx((i ? pKeys[i - 1] : HKEY_CLASSES_ROOT), pNames[i], 0, 0, &pKeys[i]);
	}
	for (int i = nDepth - 1; i >= 0; i--) {
		RegDeleteKey((i ? pKeys[i - 1] : HKEY_CLASSES_ROOT), pNames[i]);
		if (i) {
			RegCloseKey(pKeys[i - 1]);
		}
	}
}

void RegisterExtensions(bool bCreateKeys)
{
	HKEY myKeys[4];

	TCHAR* myKeynames1[1] = { _T(".fr") };
	TCHAR* myKeynames2[1] = { _T(".fs") };
	TCHAR* myKeynames3[4] = { _T("FBAlpha"), _T("shell"), _T("open"), _T("command") };
	TCHAR* myKeynames4[2] = { _T("FBAlpha"), _T("DefaultIcon") };
	TCHAR myKeyValue[MAX_PATH + 32] = _T("");

	if (bCreateKeys) {
		TCHAR szExename[MAX_PATH] = _T("");
		GetModuleFileName(NULL, szExename, MAX_PATH);

		MyRegCreateKeys(1, myKeynames1, myKeys);
		_stprintf(myKeyValue, _T("FBAlpha"));
		RegSetValueEx(myKeys[0], NULL, 0, REG_SZ, (BYTE*)myKeyValue, (_tcslen(myKeyValue) + 1) * sizeof(TCHAR));
		MyRegCloseKeys(2, myKeys);

		MyRegCreateKeys(1, myKeynames2, myKeys);
		_stprintf(myKeyValue, _T("FBAlpha"));
		RegSetValueEx(myKeys[0], NULL, 0, REG_SZ, (BYTE*)myKeyValue, (_tcslen(myKeyValue) + 1) * sizeof(TCHAR));
		MyRegCloseKeys(2, myKeys);

		MyRegCreateKeys(4, myKeynames3, myKeys);
		_stprintf(myKeyValue, _T("\"%s\" \"%%1\" -w"), szExename);
		RegSetValueEx(myKeys[3], NULL, 0, REG_SZ, (BYTE*)myKeyValue, (_tcslen(myKeyValue) + 1) * sizeof(TCHAR));
		_stprintf(myKeyValue, _T("FB Alpha file"));
		RegSetValueEx(myKeys[0], NULL, 0, REG_SZ, (BYTE*)myKeyValue, (_tcslen(myKeyValue) + 1) * sizeof(TCHAR));
		MyRegCloseKeys(4, myKeys);

		MyRegCreateKeys(2, myKeynames4, myKeys);
		_stprintf(myKeyValue, _T("\"%s\", 0"), szExename);
		RegSetValueEx(myKeys[1], NULL, 0, REG_SZ, (BYTE*)myKeyValue, (_tcslen(myKeyValue) + 1) * sizeof(TCHAR));
		MyRegCloseKeys(2, myKeys);
	} else {
		MyRegDeleteKeys(2, myKeynames4, myKeys);
		MyRegDeleteKeys(4, myKeynames3, myKeys);
		MyRegDeleteKeys(1, myKeynames2, myKeys);
		MyRegDeleteKeys(1, myKeynames1, myKeys);
	}

	return;
}
#endif
