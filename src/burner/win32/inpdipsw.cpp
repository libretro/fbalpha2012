// Burner DipSwitches Dialog module
#include "burner.h"

HWND hInpDIPSWDlg = NULL; // Handle to the DIPSW Dialog
static HWND hInpDIPSWList = NULL;

static unsigned char nPrevDIPSettings[4];
static unsigned int nDIPGroup;
static int nDIPOffset;
static bool bOK;

static void InpDIPSWGetOffset()
{
	BurnDIPInfo bdi;
	nDIPOffset = 0;
	for (int i = 0; BurnDrvGetDIPInfo(&bdi, i) == 0; i++) {
		if (bdi.nFlags == 0xF0) {
			nDIPOffset = bdi.nInput;
			break;
		}
	}
}

void InpDIPSWResetDIPs()
{
	int i = 0;
	BurnDIPInfo bdi;
	struct GameInp* pgi = NULL;

	InpDIPSWGetOffset();

	while (BurnDrvGetDIPInfo(&bdi, i) == 0) {
		if (bdi.nFlags == 0xFF) {
			pgi = GameInp + bdi.nInput + nDIPOffset;
			if (pgi) {
				pgi->Input.Constant.nConst = (pgi->Input.Constant.nConst & ~bdi.nMask) | (bdi.nSetting & bdi.nMask);
			}
		}
		i++;
	}
}

static int InpDIPSWListBegin()
{
	if (hInpDIPSWList == NULL) {
		return 1;
	}

	// Full row select style:
	ListView_SetExtendedListViewStyle(hInpDIPSWList, LVS_EX_FULLROWSELECT);

	// Make column headers
	LVCOLUMN LvCol;
	memset(&LvCol, 0, sizeof(LvCol));
	LvCol.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	LvCol.cx = 158;
	LvCol.pszText = FBALoadStringEx(IDS_DIP_GROUP);
	ListView_InsertColumn(hInpDIPSWList, 0, &LvCol);
	LvCol.cx = 215;
	LvCol.pszText = FBALoadStringEx(IDS_DIP_SETTING);
	ListView_InsertColumn(hInpDIPSWList, 1, &LvCol);

	return 0;
}

static bool CheckSetting(int i)
{
	BurnDIPInfo bdi;
	BurnDrvGetDIPInfo(&bdi, i);
	struct GameInp* pgi = GameInp + bdi.nInput + nDIPOffset;

	if (!pgi) {
		return false;
	}

	if ((pgi->Input.Constant.nConst & bdi.nMask) == bdi.nSetting) {
		unsigned char nFlags = bdi.nFlags;
		if ((nFlags & 0x0F) <= 1) {
			return true;
		} else {
			for (int j = 1; j < (nFlags & 0x0F); j++) {
				BurnDrvGetDIPInfo(&bdi, i + j);
				pgi = GameInp + bdi.nInput + nDIPOffset;
				if (nFlags & 0x80) {
					if ((pgi->Input.Constant.nConst & bdi.nMask) == bdi.nSetting) {
						return false;
					}
				} else {
					if ((pgi->Input.Constant.nConst & bdi.nMask) != bdi.nSetting) {
						return false;
					}
				}
			}
			return true;
		}
	}
	return false;
}

// Make a list view of the DIPswitches
// do not refresh list every time, modified by regret
static int InpDIPSWListMake(BOOL bBuild)
{
	if (hInpDIPSWList == NULL) {
		return 1;
	}

	if (bBuild) {
		ListView_DeleteAllItems(hInpDIPSWList);
	}

	BurnDIPInfo bdi;
	unsigned int i = 0, j = 0, k = 0;
	char* pDIPGroup = NULL;
	while (BurnDrvGetDIPInfo(&bdi, i) == 0) {
		if ((bdi.nFlags & 0xF0) == 0xF0) {
		   	if (bdi.nFlags == 0xFE || bdi.nFlags == 0xFD) {
				pDIPGroup = bdi.szText;
				k = i;
			}
			i++;
		} else {
			if (CheckSetting(i)) {
				LVITEM LvItem;
				memset(&LvItem, 0, sizeof(LvItem));
				LvItem.iItem = j;

				if (bBuild) {
					LvItem.mask = LVIF_TEXT | LVIF_PARAM;
					LvItem.iSubItem = 0;
					LvItem.pszText = AtoW(pDIPGroup);
					LvItem.lParam = (LPARAM)k;
					ListView_InsertItem(hInpDIPSWList, &LvItem);
				}

				LvItem.mask = LVIF_TEXT;
				LvItem.iSubItem = 1;
				LvItem.pszText = AtoW(bdi.szText);
				ListView_SetItem(hInpDIPSWList, &LvItem);
				j++;
			}
			i += (bdi.nFlags & 0x0F);
		}
	}

	return 0;
}

static int InpDIPSWInit()
{
	BurnDIPInfo bdi;
	struct GameInp* pgi = NULL;

	InpDIPSWGetOffset();

	hInpDIPSWList = GetDlgItem(hInpDIPSWDlg, IDC_INPCHEAT_LIST);
	InpDIPSWListBegin();
	InpDIPSWListMake(TRUE);

	for (int i = 0, j = 0; BurnDrvGetDIPInfo(&bdi, i) == 0; i++) {
		if (bdi.nInput >= 0  && bdi.nFlags == 0xFF) {
			pgi = GameInp + bdi.nInput + nDIPOffset;
			nPrevDIPSettings[j] = pgi->Input.Constant.nConst;
			j++;
		}
	}

	return 0;
}

static int InpDIPSWExit()
{
	hInpDIPSWList = NULL;
	hInpDIPSWDlg = NULL;

	if (!bAltPause && bRunPause) {
		bRunPause = 0;
	}
	GameInpCheckMouse();
	return 0;
}

static void InpDIPSWCancel()
{
	if (bOK) {
		return;
	}

	int i = 0, j = 0;
	BurnDIPInfo bdi;
	struct GameInp* pgi = NULL;

	while (BurnDrvGetDIPInfo(&bdi, i) == 0) {
		if (bdi.nInput >= 0 && bdi.nFlags == 0xFF) {
			pgi = GameInp + bdi.nInput + nDIPOffset;
			if (pgi) {
				pgi->Input.Constant.nConst = nPrevDIPSettings[j];
				j++;
			}
		}
		i++;
	}
}

// Create the list of possible values for a DIPswitch
static void InpDIPSWSelect()
{
	SendDlgItemMessage(hInpDIPSWDlg, IDC_INPCX1_VALUE, CB_RESETCONTENT, 0, 0);

	int nSel = ListView_GetNextItem(hInpDIPSWList, -1, LVNI_SELECTED);
	if (nSel >= 0) {
		LVITEM LvItem;
		memset(&LvItem, 0, sizeof(LvItem));
		LvItem.mask = LVIF_PARAM;
		LvItem.iItem = nSel;
		ListView_GetItem(hInpDIPSWList, &LvItem);

		nDIPGroup = LvItem.lParam;

		BurnDIPInfo bdiGroup;
		BurnDrvGetDIPInfo(&bdiGroup, nDIPGroup);

		int nCurrentSetting = 0;
		for (int i = 0, j = 0; i < bdiGroup.nSetting; i++) {
			TCHAR szText[MAX_PATH];
			BurnDIPInfo bdi;

			do {
				BurnDrvGetDIPInfo(&bdi, nDIPGroup + 1 + j++);
			} while (bdi.nFlags == 0);

			if (bdiGroup.szText) {
				_stprintf(szText, _T("%hs: %hs"), bdiGroup.szText, bdi.szText);
			} else {
				_stprintf(szText, _T("%hs"), bdi.szText);
			}
			SendDlgItemMessage(hInpDIPSWDlg, IDC_INPCX1_VALUE, CB_ADDSTRING, 0, (LPARAM)szText);

			if (CheckSetting(nDIPGroup + j)) {
				nCurrentSetting = i;
			}
		}
		SendDlgItemMessage(hInpDIPSWDlg, IDC_INPCX1_VALUE, CB_SETCURSEL, (WPARAM)nCurrentSetting, 0);
	}
}

static void DIPSChanged(const int& id)
{
	BurnDIPInfo bdi = {0, 0, 0, 0, NULL};
	int j = 0;
	for (int i = 0; i <= id; i++) {
		do {
			BurnDrvGetDIPInfo(&bdi, nDIPGroup + 1 + j++);
		} while (bdi.nFlags == 0);
	}

	struct GameInp* pgi = GameInp + bdi.nInput + nDIPOffset;
	pgi->Input.Constant.nConst = (pgi->Input.Constant.nConst & ~bdi.nMask) | (bdi.nSetting & bdi.nMask);
	if (bdi.nFlags & 0x40) {
		while (BurnDrvGetDIPInfo(&bdi, nDIPGroup + 1 + j++) == 0) {
			if (bdi.nFlags == 0) {
				pgi = GameInp + bdi.nInput + nDIPOffset;
				pgi->Input.Constant.nConst = (pgi->Input.Constant.nConst & ~bdi.nMask) | (bdi.nSetting & bdi.nMask);
			} else {
				break;
			}
		}
	}

	InpDIPSWListMake(TRUE);
}

static INT_PTR CALLBACK DIPSDialogProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	if (Msg == WM_INITDIALOG) {
//		EnableWindow(hScrnWnd, FALSE);

		hInpDIPSWDlg = hDlg;
		InpDIPSWInit();
		if (!kNetGame && bAutoPause) {
			bRunPause = 1;
		}
		return TRUE;
	}

	if (Msg == WM_CLOSE) {
		EnableWindow(hScrnWnd, TRUE);
		DestroyWindow(hInpDIPSWDlg);
		dialogDelete(IDD_INPDIP);
		return 0;
	}

	if (Msg == WM_DESTROY) {
		InpDIPSWCancel();
		InpDIPSWExit();
		return 0;
	}

	if (Msg == WM_COMMAND) {
		int Id = LOWORD(wParam);
		int Notify = HIWORD(wParam);

		if (Id == IDOK && Notify == BN_CLICKED) {			// OK button
			bOK = true;
			SendMessage(hDlg, WM_CLOSE, 0, 0);
			return 0;
		}
		if (Id == IDCANCEL && Notify == BN_CLICKED) {		// cancel = close
			SendMessage(hDlg, WM_CLOSE, 0, 0);
			return 0;
		}

		// New DIPswitch value selected
		if (Id == IDC_INPCX1_VALUE && Notify == CBN_SELCHANGE) {
			int nSel = SendDlgItemMessage(hInpDIPSWDlg, IDC_INPCX1_VALUE, CB_GETCURSEL, 0, 0);
			DIPSChanged(nSel);
			return 0;
		}

		// New DIPswitch selected
		if (Id == IDC_INPC_RESET && Notify == BN_CLICKED) {
			InpDIPSWResetDIPs();
			InpDIPSWListMake(TRUE);								// refresh view
			SendDlgItemMessage(hInpDIPSWDlg, IDC_INPCX1_VALUE, CB_RESETCONTENT, 0, 0);
			return 0;
		}
	}

	if (Msg == WM_NOTIFY && lParam) {
		int Id = LOWORD(wParam);
		NMHDR* pnm = (NMHDR*)lParam;

		if (Id == IDC_INPCHEAT_LIST && pnm->code == LVN_ITEMCHANGED) {
			if (((NM_LISTVIEW*)lParam)->uNewState & LVIS_SELECTED) {
				InpDIPSWSelect();
			}
			return 0;
		}

		// ==> double click to switch dips, added by regret
 		if (Id == IDC_INPCHEAT_LIST && ((pnm->code == NM_DBLCLK) || (pnm->code == NM_RDBLCLK))) {
			// Select the next item of the currently selected one.
			int nSel_Dbl = SendDlgItemMessage(hInpDIPSWDlg, IDC_INPCX1_VALUE, CB_GETCURSEL, 0, 0);
			int nCount = SendDlgItemMessage(hInpDIPSWDlg, IDC_INPCX1_VALUE, CB_GETCOUNT, 0, 0);
			if ((nSel_Dbl != LB_ERR) && (nCount > 1)) {
				if (pnm->code == NM_DBLCLK) {
					if (++nSel_Dbl >= nCount) nSel_Dbl = 0;
				} else {
					if (--nSel_Dbl < 0) nSel_Dbl = nCount - 1;
				}
				SendDlgItemMessage(hInpDIPSWDlg, IDC_INPCX1_VALUE, CB_SETCURSEL, nSel_Dbl, 0);
				DIPSChanged(nSel_Dbl);
			}
			return 0;
		}
		// <== double click to switch dips

		if (Id == IDC_INPCHEAT_LIST && pnm->code == NM_CUSTOMDRAW) {
			NMLVCUSTOMDRAW* plvcd = (NMLVCUSTOMDRAW*)lParam;

			switch (plvcd->nmcd.dwDrawStage) {
				case CDDS_PREPAINT: {
					SetWindowLongPtr(hInpDIPSWDlg, DWLP_MSGRESULT, CDRF_NOTIFYITEMDRAW);
					return 1;
				}
				case CDDS_ITEMPREPAINT: {
					BurnDIPInfo bdi;
					BurnDrvGetDIPInfo(&bdi, plvcd->nmcd.lItemlParam);
					if (bdi.nFlags == 0xFD) {
						plvcd->clrTextBk = RGB(0xFF, 0xDF, 0xBB);
						SetWindowLongPtr(hInpDIPSWDlg, DWLP_MSGRESULT, CDRF_NEWFONT);
					}

					return 1;
				}
			}
		}
	}

	return 0;
}

int InpDIPSWCreate()
{
	if (bDrvOkay == 0 || kNetGame) {
		return 1;
	}

	if (hInpDIPSWDlg) {
		// already open so just reactivate the window
		SetActiveWindow(hInpDIPSWDlg);
		return 0;
	}

	bOK = false;

	hInpDIPSWDlg = FBACreateDialog(IDD_INPDIP, hScrnWnd, (DLGPROC)DIPSDialogProc);
	if (hInpDIPSWDlg == NULL) {
		return 1;
	}
	dialogAdd(IDD_INPDIP, hInpDIPSWDlg);

	wndInMid(hInpDIPSWDlg, hScrnWnd);
	ShowWindow(hInpDIPSWDlg, SW_NORMAL);

	return 0;
}
