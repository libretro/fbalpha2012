// Burner Cheat Dialog module
#include "burner.h"
#include "cheat.h"

HWND hInpCheatDlg = NULL; // Handle to the Cheat Dialog
static HWND hInpCheatList = NULL;

static bool bOK = false;
static int nCurrentCheat = 0;
static int* nPrevCheatSettings = NULL;

static int InpCheatListBegin()
{
	if (hInpCheatList == NULL) {
		return 1;
	}

	// Full row select style:
	ListView_SetExtendedListViewStyle(hInpCheatList, LVS_EX_FULLROWSELECT);

	// Make column headers
	LVCOLUMN LvCol;
	memset(&LvCol, 0, sizeof(LvCol));
	LvCol.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	LvCol.cx = 160;
	LvCol.pszText = FBALoadStringEx(IDS_CHEAT_NAME);
	ListView_InsertColumn(hInpCheatList, 0, &LvCol);
	LvCol.cx = 140;
	LvCol.pszText = FBALoadStringEx(IDS_CHEAT_STATUS);
	ListView_InsertColumn(hInpCheatList, 1, &LvCol);

	return 0;
}

// Make a list view of the cheat list
// do not refresh list every time, modified by regret
static int InpCheatListMake(BOOL bBuild)
{
	if (hInpCheatList == NULL) {
		return 1;
	}

	if (bBuild) {
		ListView_DeleteAllItems(hInpCheatList);
	}

	int i = 0;
	CheatInfo* pCurrentCheat = pCheatInfo;

	while (pCurrentCheat) {
		LVITEM LvItem;
		memset(&LvItem, 0, sizeof(LvItem));
		LvItem.mask = LVIF_TEXT;
		LvItem.iItem = i;

		if (bBuild) {
			LvItem.iSubItem = 0;
			LvItem.pszText = pCurrentCheat->szCheatName;
			ListView_InsertItem(hInpCheatList, &LvItem);
		}
		LvItem.iSubItem = 1;
		LvItem.pszText = pCurrentCheat->pOption[pCurrentCheat->nCurrent]->szOptionName;
		ListView_SetItem(hInpCheatList, &LvItem);

		pCurrentCheat = pCurrentCheat->pNext;
		i++;
	}

	return 0;
}

static int InpCheatInit()
{
	hInpCheatList = GetDlgItem(hInpCheatDlg, IDC_INPCHEAT_LIST);
	InpCheatListBegin();
	InpCheatListMake(TRUE);

	// Save old cheat settings
	CheatInfo* pCurrentCheat = pCheatInfo;
	nCurrentCheat = 0;
	while (pCurrentCheat) {
		pCurrentCheat = pCurrentCheat->pNext;
		nCurrentCheat++;
	}

	nPrevCheatSettings = (int*)malloc(nCurrentCheat * sizeof(int));

	pCurrentCheat = pCheatInfo;
	nCurrentCheat = 0;
	while (pCurrentCheat) {
		nPrevCheatSettings[nCurrentCheat] = pCurrentCheat->nCurrent;
		pCurrentCheat = pCurrentCheat->pNext;
		nCurrentCheat++;
	}

	return 0;
}

static int InpCheatExit()
{
	free(nPrevCheatSettings);
	nPrevCheatSettings = NULL;

	hInpCheatList = NULL;
	hInpCheatDlg = NULL;
	if(!bAltPause && bRunPause) {
		bRunPause = 0;
	}
	GameInpCheckMouse();
	return 0;
}

// for cheat reload, added by regret
static void InpCheatReinit()
{
	free(nPrevCheatSettings);
	nPrevCheatSettings = NULL;

	InpCheatListMake(TRUE);

	// Save old cheat settings
	CheatInfo* pCurrentCheat = pCheatInfo;
	nCurrentCheat = 0;
	while (pCurrentCheat) {
		pCurrentCheat = pCurrentCheat->pNext;
		nCurrentCheat++;
	}

	nPrevCheatSettings = (int*)malloc(nCurrentCheat * sizeof(int));

	pCurrentCheat = pCheatInfo;
	nCurrentCheat = 0;
	while (pCurrentCheat) {
		nPrevCheatSettings[nCurrentCheat] = pCurrentCheat->nCurrent;
		pCurrentCheat = pCurrentCheat->pNext;
		nCurrentCheat++;
	}
}

static void InpCheatCancel()
{
	if (bOK) {
		return;
	}

	CheatInfo* pCurrentCheat = pCheatInfo;
	nCurrentCheat = 0;
	while (pCurrentCheat) {
		cheatEnable(nCurrentCheat, nPrevCheatSettings[nCurrentCheat]);
		pCurrentCheat = pCurrentCheat->pNext;
		nCurrentCheat++;
	}
}

static void InpCheatSelect()
{
	SendDlgItemMessage(hInpCheatDlg, IDC_INPCX1_VALUE, CB_RESETCONTENT, 0, 0);

	int nSel = ListView_GetNextItem(hInpCheatList, (WPARAM)-1, LVNI_SELECTED);
	if (nSel >= 0) {
		LVITEM LvItem;
		memset(&LvItem, 0, sizeof(LvItem));
		LvItem.mask = LVIF_PARAM;
		LvItem.iItem = nSel;
		ListView_GetItem(hInpCheatList, &LvItem);

		CheatInfo* pCurrentCheat = pCheatInfo;
		nCurrentCheat = 0;
		while (pCurrentCheat && nCurrentCheat < nSel) {
			pCurrentCheat = pCurrentCheat->pNext;
			nCurrentCheat++;
		}

		for (int i = 0; pCurrentCheat->pOption[i]; i++) {
			TCHAR szText[256];
			_stprintf(szText, _T("%s: %s"), pCurrentCheat->szCheatName, pCurrentCheat->pOption[i]->szOptionName);
			SendDlgItemMessage(hInpCheatDlg, IDC_INPCX1_VALUE, CB_ADDSTRING, 0, (LPARAM)szText);
		}

		SendDlgItemMessage(hInpCheatDlg, IDC_INPCX1_VALUE, CB_SETCURSEL, (WPARAM)pCurrentCheat->nCurrent, 0);
	}
}

static void InpCheatReset()
{
	CheatInfo* pCurrentCheat = pCheatInfo;
	nCurrentCheat = 0;
	while (pCurrentCheat) {
		cheatEnable(nCurrentCheat, -1);
		pCurrentCheat = pCurrentCheat->pNext;
		nCurrentCheat++;
	}
}

static int InpCheatPick()
{
	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hInpCheatDlg;
	ofn.lpstrFilter = _T("Cheat file (*.ini,*.dat,*.xml)\0*.ini;*.dat;*.xml\0\0");
	ofn.lpstrFile = szChoice;
	ofn.nMaxFile = sizearray(szChoice);
	ofn.lpstrInitialDir = getMiscPath(PATH_CHEAT);
	ofn.Flags = OFN_NOCHANGEDIR | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	ofn.lpstrDefExt = _T("ini");
	ofn.lpstrTitle = FBALoadStringEx(IDS_CHEAT_SELECT);

	int bOldPause = bRunPause;
	bRunPause = 1;
	int nRet = GetOpenFileName(&ofn);
	bRunPause = bOldPause;

	if (nRet == 0) {
		return 1;
	}
	return 0;
}

static INT_PTR CALLBACK CheatDialogProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	if (Msg == WM_INITDIALOG) {
//		EnableWindow(hScrnWnd, FALSE);

		hInpCheatDlg = hDlg;
		InpCheatInit();
		if (!kNetGame && bAutoPause) {
			bRunPause = 1;
		}
		return TRUE;
	}

	if (Msg == WM_CLOSE) {
		EnableWindow(hScrnWnd, TRUE);
		DestroyWindow(hInpCheatDlg);
		dialogDelete(IDD_INPCHEAT);
		return FALSE;
	}

	if (Msg == WM_DESTROY) {
		InpCheatCancel();
		InpCheatExit();
		return FALSE;
	}

	if (Msg == WM_COMMAND) {
		int Id = LOWORD(wParam);
		int Notify = HIWORD(wParam);

		if (Id == IDOK && Notify == BN_CLICKED) {		// OK button
			bOK = true;
			SendMessage(hDlg, WM_CLOSE, 0, 0);
			return FALSE;
		}
		else if (Id == IDCANCEL && Notify == BN_CLICKED) {	// cancel = close
			SendMessage(hDlg, WM_CLOSE, 0, 0);
			return FALSE;
		}
		else if (Id == IDC_INPCX1_VALUE && Notify == CBN_SELCHANGE) {
			int nSel = SendDlgItemMessage(hInpCheatDlg, IDC_INPCX1_VALUE, CB_GETCURSEL, 0, 0);
			cheatEnable(nCurrentCheat, nSel);
			InpCheatListMake(FALSE);
			return FALSE;
		}
		else if (Id == IDC_INPC_RESET && Notify == BN_CLICKED) {
			InpCheatReset();
			InpCheatListMake(TRUE); // refresh view
			SendDlgItemMessage(hInpCheatDlg, IDC_INPCX1_VALUE, CB_RESETCONTENT, 0, 0);
			return FALSE;
		}
		// cheat reload, added by regret
		else if (Id == IDC_INPC_RELOAD && Notify == BN_CLICKED) {
			InpCheatReset();
			configCheatReload();
			InpCheatReinit();
			return FALSE;
		}
		// select cheat, added by regret
		else if (Id == IDC_INPC_SELECT && Notify == BN_CLICKED) {
			if (!InpCheatPick()) {
				InpCheatReset();
				configCheatReload(szChoice);
				InpCheatReinit();
			}
			return FALSE;
		}
	}

	if (Msg == WM_NOTIFY && lParam) {
		int Id = LOWORD(wParam);
		NMHDR* pnm = (NMHDR*)lParam;

		if (Id == IDC_INPCHEAT_LIST && pnm->code == LVN_ITEMCHANGED) {
			if (((NM_LISTVIEW*)lParam)->uNewState & LVIS_SELECTED) {
				InpCheatSelect();
			}
			return FALSE;
		}

		// ==> double click to switch cheat, added by Sho
 		if (Id == IDC_INPCHEAT_LIST && ((pnm->code == NM_DBLCLK) || (pnm->code == NM_RDBLCLK))) {
			// Select the next item of the currently selected one.
			int nSel_Dbl = SendDlgItemMessage(hInpCheatDlg, IDC_INPCX1_VALUE, CB_GETCURSEL, 0, 0);
			int nCount = SendDlgItemMessage(hInpCheatDlg, IDC_INPCX1_VALUE, CB_GETCOUNT, 0, 0);
			if ((nSel_Dbl != LB_ERR) && (nCount > 1)) {
				if (pnm->code == NM_DBLCLK) {
					if (++nSel_Dbl >= nCount) nSel_Dbl = 0;
				} else {
					if (--nSel_Dbl < 0) nSel_Dbl = nCount - 1;
				}
				SendDlgItemMessage(hInpCheatDlg, IDC_INPCX1_VALUE, CB_SETCURSEL, nSel_Dbl, 0);
				cheatEnable(nCurrentCheat, nSel_Dbl);
				InpCheatListMake(FALSE);
			}
			return FALSE;
		}
		// <== double click to switch cheat
	}

	return FALSE;
}

int InpCheatCreate()
{
	if (bDrvOkay == 0 || kNetGame) {
		return 1;
	}

	if (hInpCheatDlg) {
		// already open so just reactivate the window
		SetActiveWindow(hInpCheatDlg);
		return 0;
	}

	bOK = false;

	hInpCheatDlg = FBACreateDialog(IDD_INPCHEAT, hScrnWnd, (DLGPROC)CheatDialogProc);
	if (hInpCheatDlg == NULL) {
		return 1;
	}
	dialogAdd(IDD_INPCHEAT, hInpCheatDlg);

	wndInMid(hInpCheatDlg, hScrnWnd);
	ShowWindow(hInpCheatDlg, SW_NORMAL);

	return 0;
}
