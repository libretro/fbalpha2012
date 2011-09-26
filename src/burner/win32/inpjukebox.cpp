// Burner Jukebox Dialog module, by regret

/* changelog:
 update 1: create
*/

#include "burner.h"
#include "tracklst.h"

HANDLE hJukeThread;
DWORD dwJukeThreadID;

HWND hInpJukeboxDlg = NULL;
static HWND hInpJukeList;

bool bShowSect = false;

static int jukeListBegin()
{
	LVCOLUMN LvCol;

	if (hInpJukeList == NULL) {
		return 1;
	}

	// Full row select style:
	ListView_SetExtendedListViewStyle(hInpJukeList, LVS_EX_FULLROWSELECT);

	// Make column headers
	memset(&LvCol, 0, sizeof(LvCol));
	LvCol.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	LvCol.cx = 60;
	LvCol.pszText = FBALoadStringEx(IDS_JUKE_CODE);
	ListView_InsertColumn(hInpJukeList, 0, &LvCol);
	LvCol.cx = 260;
	LvCol.pszText = FBALoadStringEx(IDS_JUKE_DESC);
	ListView_InsertColumn(hInpJukeList, 1, &LvCol);

	return 0;
}

// Make a list view of the jukebox list
static int jukeListMake(bool bIsSect)
{
	if (hInpJukeList == NULL) {
		return 1;
	}

	ListView_DeleteAllItems(hInpJukeList);

	TCHAR szCode[8] = _T("");
	int nItemCnt = 0;
	int i = 0;

	LVITEM LvItem;
	memset(&LvItem, 0, sizeof(LvItem));
	LvItem.mask = LVIF_TEXT;

	if (!bNoSect && !bIsSect) {
		// if no section
		LvItem.iItem = 0;
		LvItem.iSubItem = 0;
		_tcscpy(szCode, _T(" ..."));
		LvItem.pszText = szCode;
		ListView_InsertItem(hInpJukeList, &LvItem);

		LvItem.iItem = 0;
		LvItem.iSubItem = 1;
		ListView_SetItem(hInpJukeList, &LvItem);

		nItemCnt = nTrackCnt + 1;
		i++;
	}
	else if (bNoSect) {
		nItemCnt = getTrackCode(0, 0);
	}
	else if (bIsSect) {
		nItemCnt = nSectCnt;
	}

	for (int j = 0; i < nItemCnt; i++, j++) { // 'j' is for the normal counter
		// track code
		LvItem.iItem = i;
		LvItem.iSubItem = 0;
		LvItem.pszText = NULL;
		if (!bIsSect) {
			_stprintf(szCode, _T(" %.4X"), getTrackCode(nCurrentSect, j));
			LvItem.pszText = szCode;
		}
		ListView_InsertItem(hInpJukeList, &LvItem);

		// track description
		LvItem.iItem = i;
		LvItem.iSubItem = 1;
		if (bIsSect) {
			LvItem.pszText = getTrackDesc(0, j, bIsSect);
		} else {
			LvItem.pszText = getTrackDesc(nCurrentSect, j, bIsSect);
		}
		ListView_SetItem(hInpJukeList, &LvItem);
	}

	return 0;
}

static int jukeInit()
{
	bShowSect = (nSectCnt > 1);

	hInpJukeList = GetDlgItem(hInpJukeboxDlg, IDC_INPJUKE_LIST);
	jukeListBegin();
	jukeListMake(bShowSect);

	return 0;
}

static INT_PTR CALLBACK jukeboxDialogProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	if (Msg == WM_INITDIALOG) {
		hInpJukeboxDlg = hDlg;
		jukeInit();
		return TRUE;
	}

	if (Msg == WM_CLOSE) {
		DestroyWindow(hInpJukeboxDlg);
		dialogDelete(IDD_INPJUKE);
		jukeDestroy();
		return FALSE;
	}

	if (Msg == WM_COMMAND) {
		int Id = LOWORD(wParam);
		int Notify = HIWORD(wParam);

		if (Id == IDCANCEL && Notify == BN_CLICKED) { // cancel = close
			SendMessage(hDlg, WM_CLOSE, 0, 0);
			return FALSE;
		}

		if (Id == IDOK && Notify == BN_CLICKED) { // manual play
			TCHAR szValue[8];
			UINT16 nValue;
			if (GetDlgItemText(hDlg, IDC_INPJUKE_EDIT, szValue, sizearray(szValue))) {
				_stscanf(szValue, _T("%x"), &nValue);
				sendSoundCode(nValue);
			}
			return FALSE;
		}
	}

	if (Msg == WM_NOTIFY && lParam) {
		int Id = LOWORD(wParam);
		NMHDR *pnm = (NMHDR*)lParam;

		if (Id == IDC_INPJUKE_LIST && pnm->code == NM_DBLCLK) {
			int nSel = ((NM_LISTVIEW*)lParam)->iItem;

			if (bShowSect && !bNoSect) {
				// select section
				bShowSect = false;
				nCurrentSect = ListView_GetNextItem(hInpJukeList, -1, LVNI_SELECTED) + 1;
				setCurrentSect();
				jukeListMake(bShowSect);
				return FALSE;
			}

			if (!bNoSect && nSel == 0) {
				// up to section view
				bShowSect = true;
				jukeListMake(bShowSect);
				return FALSE;
			}

			// play soundtrack
			nCurrentTrack = nSel;
			if (!bShowSect && !bNoSect) {
				nCurrentTrack = nSel - 1; // skip first "..."
			}
			playCurrentTrack();
			return FALSE;
		}
	}

	return FALSE;
}

static int jukeCreateDlg()
{
	FBACreateDialog(IDD_INPJUKE, hScrnWnd, (DLGPROC)jukeboxDialogProc);
	if (hInpJukeboxDlg == NULL) {
		return 1;
	}
	dialogAdd(IDD_INPJUKE, hInpJukeboxDlg);

	wndInMid(hInpJukeboxDlg, hScrnWnd);
	ShowWindow(hInpJukeboxDlg, SW_NORMAL);

	return 0;
}

static DWORD WINAPI doJukebox(LPVOID)
{
	MSG msg;
	BOOL bRet;

	jukeCreateDlg();

	while (1) {
		bRet = GetMessage(&msg, NULL, 0, 0);

		if (bRet != 0 && bRet != -1) {
	    	// See if we need to end the thread
			if (msg.message == (WM_APP + 0)) {
				break;
			}

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	DestroyWindow(hInpJukeboxDlg);

	return 0;
}

void jukeDestroy()
{
	if (hJukeThread == NULL) {
		return;
	}

	PostThreadMessage(dwJukeThreadID, WM_APP + 0, 0, 0);

//	if (WaitForSingleObject(hJukeThread, 1000) != WAIT_OBJECT_0) {
//		TerminateThread(hJukeThread, 1);
//	}

	hInpJukeboxDlg = NULL;
	CloseHandle(hJukeThread);
	hJukeThread = NULL;
	dwJukeThreadID = 0;
}

int jukeCreate()
{
	if (bDrvOkay == 0 || kNetGame) {
		return 1;
	}

//	jukeDestroy();

	if (hInpJukeboxDlg || hJukeThread) {
		return 1;
	}

	hJukeThread = CreateThread(NULL, 0, doJukebox, NULL, THREAD_TERMINATE, &dwJukeThreadID);
	return 0;
}
