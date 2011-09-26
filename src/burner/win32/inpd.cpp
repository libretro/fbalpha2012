// Burner Input Dialog module
// added default mapping and autofire settings, by regret
#include "burner.h"
#ifndef NO_AUTOFIRE
#include "autofire.h"
#endif

HWND hInpdDlg = NULL;						// Handle to the Input Dialog
static HWND hInpdList = NULL;
static unsigned char* LastVal = NULL;		// Last input values/defined
static int bLastValDefined = 0;				//

static HWND hInpdGi = NULL, hInpdPci = NULL, hInpdAnalog = NULL;	// Combo boxes

#ifndef NO_AUTOFIRE
// autofire map
struct AutoFireInfo {
	int player;
	int button;
};
map<int, AutoFireInfo> autofireMap;

// ==> autofire setting, added by regret
static int InpdAutofireInit()
{
	if (!hInpdList || !hInpdDlg) {
		return 1;
	}

	// init autofire checkboxes
	AutoFireInfo* autofireInfo = NULL;
	map<int, AutoFireInfo>::iterator iter = autofireMap.begin();
	for (; iter != autofireMap.end(); iter++) {
		autofireInfo = &iter->second;
		if (getAutofire(autofireInfo->player, autofireInfo->button)) {
			ListView_SetCheckState(hInpdList, iter->first, TRUE);
		}
	}

	// autofire delay control
	TCHAR delayStr[32] = _T("");
	for (unsigned int i = 1; i < 100; i++) {
		_stprintf(delayStr, _T("%d"), i);
		SendDlgItemMessage(hInpdDlg, IDC_INPD_AUTOFIRE_DELAY, CB_ADDSTRING, 0, (LPARAM)delayStr);
	}
	SendDlgItemMessage(hInpdDlg, IDC_INPD_AUTOFIRE_DELAY, CB_SETCURSEL, (WPARAM)(autofireDelay - 1), 0);

	return 0;
}

static void InpdAutofireExit()
{
	if (!hInpdDlg) {
		return;
	}

	// autofire delay
	autofireDelay = SendDlgItemMessage(hInpdDlg, IDC_INPD_AUTOFIRE_DELAY, CB_GETCURSEL, 0, 0) + 1;

	// autofire state
	AutoFireInfo* autofireInfo = NULL;
	map<int, AutoFireInfo>::iterator iter = autofireMap.begin();
	for (; iter != autofireMap.end(); iter++) {
		autofireInfo = &iter->second;
		BOOL state = ListView_GetCheckState(hInpdList, iter->first);
		setAutofire(autofireInfo->player, autofireInfo->button, state);
	}
	autofireMap.clear();
}
// <== autofire setting
#endif

// Update which input is using which PC input
static int InpdUseUpdate()
{
	if (hInpdList == NULL) {
		return 1;
	}

	unsigned int i, j = 0;
	struct GameInp* pgi = NULL;

	// Update the values of all the inputs
	for (i = 0, pgi = GameInp; i < nGameInpCount; i++, pgi++) {
		if (pgi->Input.pVal == NULL) {
			continue;
		}

		TCHAR* pszVal = InpToDesc(pgi);

		LVITEM LvItem;
		memset(&LvItem, 0, sizeof(LvItem));
		LvItem.mask = LVIF_TEXT;
		LvItem.iItem = j;
		LvItem.iSubItem = 1;
		LvItem.pszText = pszVal;
		ListView_SetItem(hInpdList, &LvItem);

		j++;
	}

	for (i = 0, pgi = GameInp + nGameInpCount; i < nMacroCount; i++, pgi++) {
		if (pgi->nInput & GIT_GROUP_MACRO) {
			TCHAR* pszVal = InpMacroToDesc(pgi);

			LVITEM LvItem;
			memset(&LvItem, 0, sizeof(LvItem));
			LvItem.mask = LVIF_TEXT;
			LvItem.iItem = j;
			LvItem.iSubItem = 1;
			LvItem.pszText = pszVal;
			ListView_SetItem(hInpdList, &LvItem);
		}

		j++;
	}

	return 0;
}

int InpdUpdate()
{
	if (hInpdList == NULL || LastVal == NULL) {
		return 1;
	}

	unsigned int i, j = 0;
	struct GameInp* pgi = NULL;
	unsigned char* plv = NULL;
	unsigned short nThisVal;

	// Update the values of all the inputs
	for (i = 0, pgi = GameInp, plv = LastVal; i < nGameInpCount; i++, pgi++, plv++) {
		TCHAR szVal[16];

		if (pgi->nType == 0) {
			continue;
		}

		if (pgi->nType & BIT_GROUP_ANALOG) {
			if (bRunPause) {									// Update LastVal
				nThisVal = pgi->Input.nVal;
			} else {
				nThisVal = *pgi->Input.pShortVal;
			}

			if (bLastValDefined && (pgi->nType != BIT_ANALOG_REL || nThisVal) && pgi->Input.nVal == *((unsigned short*)plv)) {
				j++;
				continue;
			}

			*((unsigned short*)plv) = nThisVal;
		} else {
			if (bRunPause) {									// Update LastVal
				nThisVal = pgi->Input.nVal;
			} else {
				nThisVal = *pgi->Input.pVal;
			}

			if (bLastValDefined && pgi->Input.nVal == *plv) {	// hasn't changed
				j++;
				continue;
			}

			*plv = nThisVal;
		}

		switch (pgi->nType) {
			case BIT_DIGITAL: {
				if (nThisVal == 0) {
					szVal[0] = 0;
				} else {
					if (nThisVal == 1) {
						_tcscpy(szVal, _T("ON"));
					} else {
						_stprintf(szVal, _T("0x%02X"), nThisVal);
					}
				}
				break;
			}
			case BIT_ANALOG_ABS: {
				_stprintf(szVal, _T("0x%02X"), nThisVal >> 8);
				break;
			}
			case BIT_ANALOG_REL: {
				if (nThisVal == 0) {
					szVal[0] = 0;
				}
				if ((short)nThisVal < 0) {
					_stprintf(szVal, _T("%d"), ((short)nThisVal) >> 8);
				}
				if ((short)nThisVal > 0) {
					_stprintf(szVal, _T("+%d"), ((short)nThisVal) >> 8);
				}
				break;
			}
			default: {
				_stprintf(szVal, _T("0x%02X"), nThisVal);
			}
		}

		LVITEM LvItem;
		memset(&LvItem, 0, sizeof(LvItem));
		LvItem.mask = LVIF_TEXT;
		LvItem.iItem = j;
		LvItem.iSubItem = 2;
		LvItem.pszText = szVal;
		ListView_SetItem(hInpdList, &LvItem);

		j++;
	}

	bLastValDefined = 1; // LastVal is now defined

	return 0;
}

static int InpdListBegin()
{
	if (hInpdList == NULL) {
		return 1;
	}

	// Full row select style:
#ifndef NO_AUTOFIRE
	ListView_SetExtendedListViewStyle(hInpdList, LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES);
#else
	ListView_SetExtendedListViewStyle(hInpdList, LVS_EX_FULLROWSELECT);
#endif

	// Make column headers
	LVCOLUMN LvCol;
	memset(&LvCol, 0, sizeof(LvCol));
	LvCol.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;

	LvCol.cx = 144;
	LvCol.pszText = FBALoadStringEx(IDS_INPUT_INPUT);
	ListView_InsertColumn(hInpdList, 0, &LvCol);

	LvCol.cx = 144;
	LvCol.pszText = FBALoadStringEx(IDS_INPUT_MAPPING);
	ListView_InsertColumn(hInpdList, 1, &LvCol);

	LvCol.cx = 40;
	LvCol.pszText = FBALoadStringEx(IDS_INPUT_STATE);
	ListView_InsertColumn(hInpdList, 2, &LvCol);

	return 0;
}

// Make a list view of the game inputs
int InpdListMake(int bBuild)
{
	if (hInpdList == NULL) {
		return 1;
	}

	unsigned int j = 0;
	bLastValDefined = 0;

	if (bBuild)	{
		ListView_DeleteAllItems(hInpdList);
	}

	// get button info
	int nButtons = 0; // buttons per player
	int nPlayer = 0;
	int nPlayerTemp = 0;

	// Add all the input names to the list
	for (unsigned int i = 0; i < nGameInpCount; i++) {
		// Get the name of the input
		struct BurnInputInfo bii;
		bii.szName = NULL;
		BurnDrvGetInputInfo(&bii, i);

		// skip unused inputs
		if (bii.pVal == NULL) {
			continue;
		}
		if (bii.szName == NULL)	{
			bii.szName = "";
		}

#ifndef NO_AUTOFIRE
		// init autofire info
		if ((bii.szName[0] == 'P' || bii.szName[0] == 'p')
			&& bii.szName[1] >= '1' && bii.szName[1] <= '4') {
			nPlayer = bii.szName[1] - '1';

			if (nPlayer != nPlayerTemp) {
				// if it is next player
				nButtons = 0;
			}
			if (!strncmp(" fire", bii.szInfo + 2, 5) && nButtons < MAX_AUTOFIRE_BUTTONS) {
				AutoFireInfo info;
				info.button = nButtons;
				info.player = nPlayer;
				autofireMap[j] = info;
				nButtons++;
			}

			nPlayerTemp = nPlayer;
		}
#endif

		// add to list
		LVITEM LvItem;
		memset(&LvItem, 0, sizeof(LvItem));
		LvItem.mask = LVIF_TEXT | LVIF_PARAM;
		LvItem.iItem = j;
		LvItem.iSubItem = 0;
		LvItem.pszText = AtoW(bii.szName);
		LvItem.lParam = (LPARAM)i;

		if (bBuild) {
			ListView_InsertItem(hInpdList, &LvItem);
		} else {
			ListView_SetItem(hInpdList, &LvItem);
		}

		j++;
	}

	struct GameInp* pgi = GameInp + nGameInpCount;
	for (unsigned int i = 0; i < nMacroCount; i++, pgi++) {
		if (pgi->nInput & GIT_GROUP_MACRO) {
			LVITEM LvItem;
			memset(&LvItem, 0, sizeof(LvItem));
			LvItem.mask = LVIF_TEXT | LVIF_PARAM;
			LvItem.iItem = j;
			LvItem.iSubItem = 0;
			LvItem.pszText = AtoW(pgi->Macro.szName);
			LvItem.lParam = (LPARAM)j;

			if (bBuild) {
				ListView_InsertItem(hInpdList, &LvItem);
			} else {
				ListView_SetItem(hInpdList, &LvItem);
			}
		}

		j++;
	}

	InpdUseUpdate();

	return 0;
}

static void DisablePresets()
{
	EnableWindow(hInpdPci, FALSE);
	EnableWindow(hInpdAnalog, FALSE);
	EnableWindow(GetDlgItem(hInpdDlg, IDC_INPD_DEFAULT), FALSE);
	EnableWindow(GetDlgItem(hInpdDlg, IDC_INPD_USE), FALSE);
}

static void InitComboboxes()
{
	TCHAR szLabel[1024];

	for (int i = 0; i < 4; i++) {
		_stprintf(szLabel, FBALoadStringEx(IDS_INPUT_INP_PLAYER), i + 1);
		SendMessage(hInpdGi, CB_ADDSTRING, 0, (LPARAM)szLabel);
	}

	SendMessage(hInpdPci, CB_ADDSTRING, 0, (LPARAM)FBALoadStringEx(IDS_INPUT_INP_KEYBOARD));
	for (int i = 0; i < 3; i++) {
		_stprintf(szLabel, FBALoadStringEx(IDS_INPUT_INP_JOYSTICK), i);
		SendMessage(hInpdPci, CB_ADDSTRING, 0, (LPARAM)szLabel);
	}
	SendMessage(hInpdPci, CB_ADDSTRING, 0, (LPARAM)FBALoadStringEx(IDS_INPUT_INP_XARCADEL));
	SendMessage(hInpdPci, CB_ADDSTRING, 0, (LPARAM)FBALoadStringEx(IDS_INPUT_INP_XARCADER));
	SendMessage(hInpdPci, CB_ADDSTRING, 0, (LPARAM)FBALoadStringEx(IDS_INPUT_INP_HOTRODL));
	SendMessage(hInpdPci, CB_ADDSTRING, 0, (LPARAM)FBALoadStringEx(IDS_INPUT_INP_HOTRODR));

	// Scan presets directory for .ini files and add them to the list
	WIN32_FIND_DATA findData;
	HANDLE search = FindFirstFile(_T("config\\presets\\*.ini"), &findData);
	if (search != INVALID_HANDLE_VALUE) {
		do {
			findData.cFileName[_tcslen(findData.cFileName) - 4] = 0;
			SendMessage(hInpdPci, CB_ADDSTRING, 0, (LPARAM)findData.cFileName);
		} while (FindNextFile(search, &findData) != 0);

		FindClose(search);
	}
}

static int InpdInit()
{
	hInpdList = GetDlgItem(hInpdDlg, IDC_INPD_LIST);
	if (hInpdList == NULL) {
		return 1;
	}

	// Allocate a last val array for the last input values
	int nMemLen = nGameInpCount * sizeof(char);
	LastVal = (unsigned char*)malloc(nMemLen);
	if (LastVal == NULL) {
		return 1;
	}
	memset(LastVal, 0, nMemLen);

	InpdListBegin();
	InpdListMake(1);
#ifndef NO_AUTOFIRE
	InpdAutofireInit();
#endif

	// Init the Combo boxes
	hInpdGi = GetDlgItem(hInpdDlg, IDC_INPD_GI);
	hInpdPci = GetDlgItem(hInpdDlg, IDC_INPD_PCI);
	hInpdAnalog = GetDlgItem(hInpdDlg, IDC_INPD_ANALOG);
	InitComboboxes();

	DisablePresets();

	return 0;
}

static int InpdExit()
{
#ifndef NO_AUTOFIRE
	InpdAutofireExit();
#endif
	// save what we want
	ConfigGameSave(true); // save game config

	// Exit the Combo boxes
	hInpdPci = NULL;
	hInpdGi = NULL;
	hInpdAnalog = NULL;

	if (LastVal != NULL) {
		free(LastVal);
		LastVal = NULL;
	}
	hInpdList = NULL;
	hInpdDlg = NULL;
	if (!bAltPause && bRunPause) {
		bRunPause=0;
	}
	GameInpCheckMouse();

	return 0;
}

static void GameInpConfigOne(int nPlayer, int nPcDev, int nAnalog, struct GameInp* pgi, char* szi)
{
	switch (nPcDev) {
		case  0:
			GamcPlayer(pgi, szi, nPlayer, -1);						// Keyboard
			GamcAnalogKey(pgi, szi, nPlayer, nAnalog);
			GamcMisc(pgi, szi, nPlayer);
			break;
		case  1:
			GamcPlayer(pgi, szi, nPlayer, 0);						// Joystick 1
			GamcAnalogJoy(pgi, szi, nPlayer, 0, nAnalog);
			GamcMisc(pgi, szi, nPlayer);
			break;
		case  2:
			GamcPlayer(pgi, szi, nPlayer, 1);						// Joystick 2
			GamcAnalogJoy(pgi, szi, nPlayer, 1, nAnalog);
			GamcMisc(pgi, szi, nPlayer);
			break;
		case  3:
			GamcPlayer(pgi, szi, nPlayer, 2);						// Joystick 3
			GamcAnalogJoy(pgi, szi, nPlayer, 2, nAnalog);
			GamcMisc(pgi, szi, nPlayer);
			break;
		case  4:
			GamcPlayerHotRod(pgi, szi, nPlayer, 0x10, nAnalog);		// X-Arcade left side
			GamcMisc(pgi, szi, -1);
			break;
		case  5:
			GamcPlayerHotRod(pgi, szi, nPlayer, 0x11, nAnalog);		// X-Arcade right side
			GamcMisc(pgi, szi, -1);
			break;
		case  6:
			GamcPlayerHotRod(pgi, szi, nPlayer, 0x00, nAnalog);		// HotRod left side
			GamcMisc(pgi, szi, -1);
			break;
		case  7:
			GamcPlayerHotRod(pgi, szi, nPlayer, 0x01, nAnalog);		// HotRod right size
			GamcMisc(pgi, szi, -1);
			break;
	}
}

// Configure some of the game input
static int GameInpConfig(int nPlayer, int nPcDev, int nAnalog)
{
	struct GameInp* pgi = NULL;
	unsigned int i;

	for (i = 0, pgi = GameInp; i < nGameInpCount; i++, pgi++) {
		struct BurnInputInfo bii;

		// Get the extra info about the input
		bii.szInfo = NULL;
		BurnDrvGetInputInfo(&bii, i);
		if (bii.pVal == NULL) {
			continue;
		}
		if (bii.szInfo == NULL) {
			bii.szInfo = "";
		}
		GameInpConfigOne(nPlayer, nPcDev, nAnalog, pgi, bii.szInfo);
	}

	for (i = 0; i < nMacroCount; i++, pgi++) {
		GameInpConfigOne(nPlayer, nPcDev, nAnalog, pgi, pgi->Macro.szName);
	}

	GameInpCheckLeftAlt();

	return 0;
}

// List item activated; find out which one
static int ListItemActivate()
{
	int nSel = ListView_GetNextItem(hInpdList, -1, LVNI_SELECTED);
	if (nSel < 0) {
		return 1;
	}

	// Get the corresponding input
	LVITEM LvItem;
	LvItem.mask = LVIF_PARAM;
	LvItem.iItem = nSel;
	LvItem.iSubItem = 0;
	ListView_GetItem(hInpdList, &LvItem);

	nSel = LvItem.lParam;
	if (nSel >= (int)(nGameInpCount + nMacroCount)) {	// out of range
		return 1;
	}

	struct BurnInputInfo bii;
	bii.nType = 0;
	BurnDrvGetInputInfo(&bii, nSel);
	if (bii.pVal == NULL) {
		return 1;
	}

	dialogDelete(IDD_INPS);
	dialogDelete(IDD_INPC);

	if (bii.nType & BIT_GROUP_CONSTANT) {
		// Dip switch is a constant - change it
		nInpcInput = nSel;
		InpcCreate();
	} else {
		if (GameInp[nSel].nInput == GIT_MACRO_CUSTOM) {
#if 0
			InpMacroCreate(nSel);
#endif
		} else {
			// Assign to a key
			nInpsInput = nSel;
			InpsCreate();
		}
	}

	GameInpCheckLeftAlt();

	return 0;
}

static int NewMacroButton()
{
#if 0
	LVITEM LvItem;
	int nSel;

	dialogDelete(IDD_INPS);
	dialogDelete(IDD_INPC);

	nSel = ListView_GetNextItem(hInpdList, -1, LVNI_SELECTED);
	if (nSel < 0) {
		nSel = -1;
	}

	// Get the corresponding input
	LvItem.mask = LVIF_PARAM;
	LvItem.iItem = nSel;
	LvItem.iSubItem = 0;
	ListView_GetItem(hInpdList, &LvItem);
	nSel = LvItem.lParam;

	if (nSel >= (int)nGameInpCount && nSel < (int)(nGameInpCount + nMacroCount)) {
		if (GameInp[nSel].nInput != GIT_MACRO_CUSTOM) {
			nSel = -1;
		}
	} else {
		nSel = -1;
	}

	InpMacroCreate(nSel);
#endif

	return 0;
}

static int DeleteInput(unsigned int i)
{
	if (i >= nGameInpCount) {
		if (i < nGameInpCount + nMacroCount) {	// Macro
			GameInp[i].Macro.nMode = 0;
		} else { 								// out of range
			return 1;
		}
	} else {									// "True" input
		struct BurnInputInfo bii;
		bii.nType = BIT_DIGITAL;
		BurnDrvGetInputInfo(&bii, i);
		if (bii.pVal == NULL) {
			return 1;
		}
		if (bii.nType & BIT_GROUP_CONSTANT) {	// Don't delete dip switches
			return 1;
		}

		GameInp[i].nInput = 0;
	}

	GameInpCheckLeftAlt();

	return 0;
}

// List item(s) deleted; find out which one(s)
static int ListItemDelete()
{
	int nStart = -1;
	LVITEM LvItem;
	int nRet;

	while ((nRet = ListView_GetNextItem(hInpdList, nStart, LVNI_SELECTED)) != -1) {
		nStart = nRet;

		// Get the corresponding input
		LvItem.mask = LVIF_PARAM;
		LvItem.iItem = nRet;
		LvItem.iSubItem = 0;
		ListView_GetItem(hInpdList, &LvItem);
		nRet = LvItem.lParam;

		DeleteInput(nRet);
	}

	InpdListMake(0); // refresh view
	return 0;
}

static int InitAnalogOptions(int nGi, int nPci)
{
	// init analog options dialog
	int nAnalog = -1;
	if (nPci == (nPlayerDefaultControls[nGi] & 0x0F)) {
		nAnalog = nPlayerDefaultControls[nGi] >> 4;
	}

	SendMessage(hInpdAnalog, CB_RESETCONTENT, 0, 0);
	if (nPci >= 1 && nPci <= 3) {
		// Absolute mode only for joysticks
		SendMessage(hInpdAnalog, CB_ADDSTRING, 0, (LPARAM)(LPARAM)FBALoadStringEx(IDS_INPUT_ANALOG_ABS));
	} else {
		if (nAnalog > 0) {
			nAnalog--;
		}
	}
	SendMessage(hInpdAnalog, CB_ADDSTRING, 0, (LPARAM)(LPARAM)FBALoadStringEx(IDS_INPUT_ANALOG_AUTO));
	SendMessage(hInpdAnalog, CB_ADDSTRING, 0, (LPARAM)(LPARAM)FBALoadStringEx(IDS_INPUT_ANALOG_NORMAL));

	SendMessage(hInpdAnalog, CB_SETCURSEL, (WPARAM)nAnalog, 0);

	return 0;
}

int UsePreset(bool bMakeDefault)
{
	int nGi, nPci, nAnalog = 0;
	TCHAR szFilename[MAX_PATH] = _T("config\\presets\\");

	nGi = SendMessage(hInpdGi, CB_GETCURSEL, 0, 0);
	if (nGi == CB_ERR) {
		return 1;
	}
	nPci = SendMessage(hInpdPci, CB_GETCURSEL, 0, 0);
	if (nPci == CB_ERR) {
		return 1;
	}

	if (nPci <= 7) {
		// Determine analog option
		nAnalog = SendMessage(hInpdAnalog, CB_GETCURSEL, 0, 0);
		if (nAnalog == CB_ERR) {
			return 1;
		}

		if (nPci == 0 || nPci > 3) {				// No "Absolute" option for keyboard or X-Arcade/HotRod controls
			nAnalog++;
		}

		GameInpConfig(nGi, nPci, nAnalog);			// Re-configure inputs
	} else {
		// Find out the filename of the preset ini
		SendMessage(hInpdPci, CB_GETLBTEXT, nPci, (LPARAM)(szFilename + _tcslen(szFilename)));
		_tcscat(szFilename, _T(".ini"));

		GameInputAutoIni(nGi, szFilename, true);	// Read inputs from file

		// Make sure all inputs are defined
		for (unsigned int i = 0, j = 0; i < nGameInpCount; i++) {
			if (GameInp[i].Input.pVal == NULL) {
				continue;
			}

			if (GameInp[i].nInput == 0) {
				DeleteInput(j);
			}

			j++;
		}

		nPci = 0x0F;
	}

	SendMessage(hInpdAnalog, CB_SETCURSEL, (WPARAM)-1, 0);
	SendMessage(hInpdPci, CB_SETCURSEL, (WPARAM)-1, 0);
	SendMessage(hInpdGi, CB_SETCURSEL, (WPARAM)-1, 0);

	DisablePresets();

	if (bMakeDefault) {
		nPlayerDefaultControls[nGi] = nPci | (nAnalog << 4);
		_tcscpy(szPlayerDefaultIni[nGi], szFilename);
	}

	GameInpCheckLeftAlt();

	return 0;
}

// ==> default input mapping, added by regret
TCHAR* defaultInpFilename = _T("config\\presets\\default.ini");
TCHAR* defaultNeoInpFilename = _T("config\\presets\\default_neo.ini");
TCHAR* defaultCpsInpFilename = _T("config\\presets\\default_cps.ini");

static TCHAR* GetDefaultInputFilename()
{
	TCHAR* fileName = defaultInpFilename;

#if 1
	int flag = (BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK);

	if (flag == HARDWARE_SNK_NEOGEO) {
		fileName = defaultNeoInpFilename;
	}
	else if (flag == HARDWARE_CAPCOM_CPS1
		|| flag == HARDWARE_CAPCOM_CPS1_QSOUND
		|| flag == HARDWARE_CAPCOM_CPS1_GENERIC
		|| flag == HARDWARE_CAPCOM_CPSCHANGER
		|| flag == HARDWARE_CAPCOM_CPS2
		|| flag == HARDWARE_CAPCOM_CPS3) {
		fileName = defaultCpsInpFilename;
	}
#endif

	return fileName;
}

static int SaveDefaultInput()
{
	TCHAR* fileName = GetDefaultInputFilename();
	FILE* h = _tfopen(fileName, _T("wt"));
	if (h == NULL) {
		return 1;
	}

	// Write version number
	_ftprintf(h, _T("version 0x%06X\n\n"), nBurnVer);
	GameInpWrite(h, false);
	if (h) {
		fclose(h);
	}

	return 0;
}

int loadDefaultInput()
{
	TCHAR* fileName = GetDefaultInputFilename();

	// Read default inputs from file
	for (int nPlayer = 0; nPlayer < nMaxPlayers; nPlayer++) {
		GameInputAutoIni(nPlayer, fileName, true);
	}
	return 0;
}
// <== default input mapping

// ==> save preset, added by regret
static int savePreset(HWND hDlg)
{
	// save file with your input name
	_stprintf(szChoice, _T("preset"));
	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hDlg;
	ofn.lpstrTitle = _T("Save preset file");
	ofn.lpstrFilter = _T("Preset files(*.ini)\0*.ini\0\0");
	ofn.lpstrFile = szChoice;
	ofn.nMaxFile = sizearray(szChoice);
	ofn.lpstrInitialDir = _T("config\\presets");
	ofn.Flags = OFN_NOCHANGEDIR | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	ofn.lpstrDefExt = _T("ini");

	int nRet = GetSaveFileName(&ofn);
	if (nRet == 0) {
		return 1;
	}

	FILE* h = _tfopen(szChoice, _T("wt"));
	if (h == NULL) {
		return 1;
	}

	// Write version number
	_ftprintf(h, _T("version 0x%06X\n\n"), nBurnVer);
	GameInpWrite(h, false);
	if (h) {
		fclose(h);
	}

	return 0;
}

static int onRButtonUp(HWND hwnd, int x, int y)
{
	if (hwnd == hScrnWnd || nVidFullscreen) {
		return 1;
	}

	RECT rc;
	GetWindowRect(GetDlgItem(hwnd, IDC_INPD_USE), &rc);

	POINT pt;
	pt.x = x; pt.y = y;
	ClientToScreen(hwnd, &pt);

	if (pt.x > rc.left && pt.x < rc.right && pt.y > rc.top && pt.y < rc.bottom) {
		savePreset(hwnd);
		return 0;
	}

	return 1;
}
// <== save preset

static INT_PTR CALLBACK InpDialogProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	if (Msg == WM_INITDIALOG) {
		hInpdDlg = hDlg;
		InpdInit();
		if (!kNetGame && bAutoPause) {
			bRunPause = 1;
		}

		return TRUE;
	}

	if (Msg == WM_CLOSE) {
		EnableWindow(hScrnWnd, TRUE);
		DestroyWindow(hInpdDlg);
		dialogDelete(IDD_INPD);
		return FALSE;
	}

	if (Msg == WM_DESTROY) {
		InpdExit();
		return FALSE;
	}

	if (Msg == WM_COMMAND) {
		int Id = LOWORD(wParam);
		int Notify = HIWORD(wParam);

		if (Id == IDOK && Notify == BN_CLICKED) {
			ListItemActivate();
			return FALSE;
		}
		if (Id == IDCANCEL && Notify == BN_CLICKED) {
			SendMessage(hDlg, WM_CLOSE, 0, 0);
			return FALSE;
		}

		if (Id == IDC_INPD_NEWMACRO && Notify == BN_CLICKED) {
			NewMacroButton();
			return FALSE;
		}

		if (Id == IDC_INPD_USE && Notify == BN_CLICKED) {
			UsePreset(false);
			InpdListMake(0);	// refresh view
			return FALSE;
		}

		if (Id == IDC_INPD_DEFAULT && Notify == BN_CLICKED) {
			UsePreset(true);
			InpdListMake(0);	// refresh view
			return FALSE;
		}

		// ==> default input mapping, added by regret
		if (Id == IDC_INPD_SAVEDEF && Notify == BN_CLICKED) {
			SaveDefaultInput();
			return FALSE;
		}
		// <== default input mapping

		if (Id == IDC_INPD_GI && Notify == CBN_SELCHANGE) {
			int nGi = SendMessage(hInpdGi, CB_GETCURSEL, 0, 0);
			if (nGi == CB_ERR) {
				SendMessage(hInpdPci, CB_SETCURSEL, (WPARAM)-1, 0);
				SendMessage(hInpdAnalog, CB_SETCURSEL, (WPARAM)-1, 0);
				DisablePresets();
				return FALSE;
			}

			int nPci = nPlayerDefaultControls[nGi] & 0x0F;
			SendMessage(hInpdPci, CB_SETCURSEL, nPci, 0);
			EnableWindow(hInpdPci, TRUE);

			if (nPci > 5) {
				SendMessage(hInpdAnalog, CB_SETCURSEL, (WPARAM)-1, 0);
				EnableWindow(hInpdAnalog, FALSE);
			} else {
				InitAnalogOptions(nGi, nPci);
				EnableWindow(hInpdAnalog, TRUE);
			}

			EnableWindow(GetDlgItem(hInpdDlg, IDC_INPD_DEFAULT), TRUE);
			EnableWindow(GetDlgItem(hInpdDlg, IDC_INPD_USE), TRUE);
			return FALSE;
		}

		if (Id == IDC_INPD_PCI && Notify == CBN_SELCHANGE) {
			int nGi = SendMessage(hInpdGi, CB_GETCURSEL, 0, 0);
			if (nGi == CB_ERR) {
				return FALSE;
			}
			int nPci = SendMessage(hInpdPci, CB_GETCURSEL, 0, 0);
			if (nPci == CB_ERR) {
				return FALSE;
			}

			if (nPci > 7) {
				EnableWindow(GetDlgItem(hInpdDlg, IDC_INPD_DEFAULT), TRUE);
				EnableWindow(GetDlgItem(hInpdDlg, IDC_INPD_USE), TRUE);

				SendMessage(hInpdAnalog, CB_SETCURSEL, (WPARAM)-1, 0);
				EnableWindow(hInpdAnalog, FALSE);
			} else {
				EnableWindow(hInpdAnalog, TRUE);
				InitAnalogOptions(nGi, nPci);

				if (SendMessage(hInpdAnalog, CB_GETCURSEL, 0, 0) != CB_ERR) {
					EnableWindow(GetDlgItem(hInpdDlg, IDC_INPD_DEFAULT), TRUE);
					EnableWindow(GetDlgItem(hInpdDlg, IDC_INPD_USE), TRUE);
				} else {
					EnableWindow(GetDlgItem(hInpdDlg, IDC_INPD_DEFAULT), FALSE);
					EnableWindow(GetDlgItem(hInpdDlg, IDC_INPD_USE), FALSE);
				}
			}

			return FALSE;
		}

		if (Id == IDC_INPD_ANALOG && Notify == CBN_SELCHANGE) {
			if (SendMessage(hInpdAnalog, CB_GETCURSEL, 0, 0) != CB_ERR) {
				EnableWindow(GetDlgItem(hInpdDlg, IDC_INPD_DEFAULT), TRUE);
				EnableWindow(GetDlgItem(hInpdDlg, IDC_INPD_USE), TRUE);
			}
			return FALSE;
		}
	}

	// ==> save preset if rbutton clicked, added by regret
	if (Msg == WM_RBUTTONUP) {
		return onRButtonUp(hDlg, LOWORD(lParam), HIWORD(lParam));
	}
	// <== save preset

	if (Msg == WM_NOTIFY && lParam != 0) {
		int Id = LOWORD(wParam);
		NMHDR* pnm = (NMHDR*)lParam;

		if (Id == IDC_INPD_LIST && pnm->code == LVN_ITEMACTIVATE) {
			ListItemActivate();
		}

		if (Id == IDC_INPD_LIST && pnm->code == LVN_KEYDOWN) {
			NMLVKEYDOWN *pnmkd = (NMLVKEYDOWN*)lParam;
			if (pnmkd->wVKey == VK_DELETE) {
				ListItemDelete();
			}
		}

		if (Id == IDC_INPD_LIST && pnm->code == NM_CUSTOMDRAW) {
			NMLVCUSTOMDRAW* plvcd = (NMLVCUSTOMDRAW*)lParam;

			switch (plvcd->nmcd.dwDrawStage) {
				case CDDS_PREPAINT:
                    SetWindowLongPtr(hInpdDlg, DWLP_MSGRESULT, CDRF_NOTIFYITEMDRAW);
					return 1;

				case CDDS_ITEMPREPAINT:
					if (plvcd->nmcd.dwItemSpec < (nGameInpCount - nComCount)) {  // modified by emufan
						if (GameInp[plvcd->nmcd.dwItemSpec].nType & BIT_GROUP_CONSTANT) {

							if (GameInp[plvcd->nmcd.dwItemSpec].nInput == 0) {
								plvcd->clrTextBk = RGB(0xDF, 0xDF, 0xDF);
								SetWindowLongPtr(hInpdDlg, DWLP_MSGRESULT, CDRF_NEWFONT);
								return 1;
							}

							if (GameInp[plvcd->nmcd.dwItemSpec].nType == BIT_DIPSWITCH) {
								plvcd->clrTextBk = RGB(0xFF, 0xEF, 0xD7);
								SetWindowLongPtr(hInpdDlg, DWLP_MSGRESULT, CDRF_NEWFONT);
								return 1;
							}
						}
					}

					if (plvcd->nmcd.dwItemSpec >= (nGameInpCount-nComCount)) {  // modified by emufan
						if (GameInp[plvcd->nmcd.dwItemSpec].Macro.nMode) {
							plvcd->clrTextBk = RGB(0xFF, 0xCF, 0xCF);
						} else {
							plvcd->clrTextBk = RGB(0xFF, 0xEF, 0xEF);
						}
						SetWindowLongPtr(hInpdDlg, DWLP_MSGRESULT, CDRF_NEWFONT);
						return 1;
					}
					return 1;
			}
		}

		return FALSE;
	}

	return FALSE;
}

int InpdCreate()
{
	if (bDrvOkay == 0) {
		return 1;
	}

	if (hInpdDlg) {
		// already open so just reactivate the window
		SetActiveWindow(hInpdDlg);
		return 0;
	}

	hInpdDlg = FBACreateDialog(IDD_INPD, hScrnWnd, (DLGPROC)InpDialogProc);
	if (hInpdDlg == NULL) {
		return 1;
	}
	dialogAdd(IDD_INPD, hInpdDlg);

	wndInMid(hInpdDlg, hScrnWnd);
	ShowWindow(hInpdDlg, SW_NORMAL);
	return 0;
}
