// rom audit  dialog module
#include "burner.h"

bool avOk = false;
bool bRescanRoms = false;

static DWORD dwScanThreadId = 0;
static HANDLE hScanThread = NULL;
static bool chOnce = false;

HWND hRomsDlg = NULL;
static HANDLE hEvent = NULL;

static void CreateRomDatName(TCHAR* szRomDat)
{
	_stprintf(szRomDat, _T("config\\%s.roms.dat"), szAppExeName);
	return;
}

///////////////////////////////////////////////////////////////////////////////
//Check Romsets////////////////////////////////////////////////////////////////

static int WriteGameAvb()
{
	TCHAR szRomDat[MAX_PATH];
	CreateRomDatName(szRomDat);

	FILE* h = _tfopen(szRomDat, _T("wt"));
	if (h == NULL) {
		return 1;
	}

	_ftprintf(h, _T(APP_TITLE) _T(" v%.20s ROMs"), szAppBurnVer);	// identifier
	_ftprintf(h, _T(" 0x%04X "), nBurnDrvCount);					// no of games

	for (unsigned int i = 0; i < nBurnDrvCount; i++) {
		if (getAuditState(i) == AUDIT_FULLPASS) {
			_fputtc(_T('*'), h);
		}
		else if (getAuditState(i) == AUDIT_PARTPASS) {
			_fputtc(_T('+'), h);
		}
		else {
			_fputtc(_T('-'), h);
		}
	}

	_ftprintf(h, _T(" END"));										// end marker

	fclose(h);
	return 0;
}

static int DoCheck(TCHAR* buffPos)
{
	TCHAR label[256];

	// Check identifier
	memset(label, 0, sizeof(label));
	_stprintf(label, _T(APP_TITLE) _T(" v%.20s ROMs"), szAppBurnVer);
	if ((buffPos = labelCheck(buffPos, label)) == NULL) {
		return 1;
	}

	// Check no of supported games
	memset(label, 0, sizeof(label));
	memcpy(label, buffPos, 16);
	buffPos += 8;
	unsigned int n = _tcstol(label, NULL, 0);
	if (n != nBurnDrvCount) {
		return 1;
	}

	if (chOnce) {
		avOk = true;
		return 0;
	}

	for (unsigned int i = 0; i < nBurnDrvCount; i++) {
		if (*buffPos == _T('*')) {
			setAuditState(i, AUDIT_FULLPASS);
		}
		else if (*buffPos == _T('+')) {
			setAuditState(i, AUDIT_PARTPASS);
		}
		else if (*buffPos == _T('-')) {
			setAuditState(i, AUDIT_FAIL);
		}
		else {
			return 1;
		}

		buffPos++;
	}

	memset(label, 0, sizeof(label));
	_stprintf(label, _T(" END"));
	if (labelCheck(buffPos, label) == NULL) {
		avOk = true;
		chOnce = true;
		return 0;
	}

	return 1;
}

int CheckGameAvb()
{
	int nBufferSize = nBurnDrvCount + 256;
	TCHAR* buffer = (TCHAR*)malloc(nBufferSize * sizeof(TCHAR));
	if (buffer == NULL) {
		return 1;
	}
	memset(buffer, 0, nBufferSize * sizeof(TCHAR));

	TCHAR szRomDat[MAX_PATH];
	CreateRomDatName(szRomDat);

	FILE* h = _tfopen(szRomDat, _T("r"));
	if (h == NULL) {
		free(buffer);
		return 1;
	}

	_fgetts(buffer, nBufferSize, h);
	fclose(h);

	int bOK = DoCheck(buffer);

	free(buffer);
	return bOK;
}

static int QuitRomsScan()
{
	DWORD dwExitCode;
	GetExitCodeThread(hScanThread, &dwExitCode);

	if (dwExitCode == STILL_ACTIVE) {

		// Signal the scan thread to abort
		SetEvent(hEvent);

		// Wait for the thread to finish
		if (WaitForSingleObject(hScanThread, 10000) != WAIT_OBJECT_0) {
			// If the thread doesn't finish within 10 seconds, forcibly kill it
			TerminateThread(hScanThread, 1);
		}

		CloseHandle(hScanThread);
	}

	CloseHandle(hEvent);

	hEvent = NULL;

	hScanThread = NULL;
	dwScanThreadId = 0;

	BArchiveClose();

	bRescanRoms = false;

	if (avOk) {
		WriteGameAvb();
	}

	return 1;
}

// set scan progress
void romsSetProgress()
{
	SendDlgItemMessage(hRomsDlg, IDC_WAIT_PROG, PBM_STEPIT, 0, 0);
}

void checkScanThread()
{
	// See if we need to abort
	if (WaitForSingleObject(hEvent, 0) == WAIT_OBJECT_0) {
		ExitThread(0);
	}
}

static DWORD WINAPI AnalyzingRoms(LPVOID)					// LPVOID lParam
{
//	bool oldMethod = (nLoadMenuShowX & OLDAUDIT) ? true : false;
	int ret = 0; // audit result

//	if (!oldMethod)
	{
		// prepare audit info
		SendDlgItemMessageA(hRomsDlg, IDC_WAIT_LABEL_A, WM_SETTEXT, 0, (LPARAM)"scanning...");
		auditPrepare();
		SendDlgItemMessageA(hRomsDlg, IDC_WAIT_LABEL_A, WM_SETTEXT, 0, (LPARAM)"auditing...");
	}

	unsigned int nOldSelect = nBurnDrvSelect;

	for (unsigned int z = 0; z < nBurnDrvCount; z++) {
		nBurnDrvSelect = z;

		// See if we need to abort
		checkScanThread();

//		if (oldMethod) {
			// set driver name
//			SendDlgItemMessage(hRomsDlg, IDC_WAIT_PROG, PBM_STEPIT, 0, 0);
//			SendDlgItemMessageA(hRomsDlg, IDC_WAIT_LABEL_A, WM_SETTEXT, 0, (LPARAM)BurnDrvGetTextA(DRV_NAME));

//			ret = BArchiveOpen(true);
//			BArchiveClose();
//		} else
		{
			if (z % 100 == 0) {
				SendDlgItemMessage(hRomsDlg, IDC_WAIT_PROG, PBM_STEPIT, 0, 0);
			}
			ret = auditRomset();
		}

		setAuditState(nBurnDrvSelect, ret);
	}

	nBurnDrvSelect = nOldSelect;

	avOk = true;

	PostMessage(hRomsDlg, WM_CLOSE, 0, 0);

	return 0;
}

static INT_PTR CALLBACK WaitProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM)		// LPARAM lParam
{
	switch (Msg) {
		case WM_INITDIALOG: {
			hRomsDlg = hDlg;

			// ==> load custom resource, added by regret
			HBITMAP hBitmap = (HBITMAP)LoadImage(hAppInst, _T("res\\misc.bmp"), IMAGE_BITMAP, 80, 60, LR_LOADFROMFILE);
			if (!hBitmap) {
				hBitmap = LoadBitmap(hAppInst, MAKEINTRESOURCE(BMP_MISC));
			}
			SendDlgItemMessage(hDlg, IDC_MISC, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBitmap);
			// <== load custom resource

			resetAuditState();

			// set progress bar range
//			if (nLoadMenuShowX & OLDAUDIT) {
//				SendDlgItemMessage(hDlg, IDC_WAIT_PROG, PBM_SETRANGE, 0, MAKELPARAM(0, nBurnDrvCount));
//			} else
			{
				extern int getFileInfo(bool scanonly);
				SendDlgItemMessage(hDlg, IDC_WAIT_PROG, PBM_SETRANGE, 0, MAKELPARAM(0, getFileInfo(true) + nBurnDrvCount / 100));
			}
			SendDlgItemMessage(hDlg, IDC_WAIT_PROG, PBM_SETSTEP, (WPARAM)1, 0);

			ShowWindow(GetDlgItem(hDlg, IDC_WAIT_LABEL_A), TRUE);
			ShowWindow(GetDlgItem(hDlg, IDCANCEL), TRUE);

			avOk = false;
			hScanThread = CreateThread(NULL, 0, AnalyzingRoms, NULL, THREAD_TERMINATE, &dwScanThreadId);

			hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

			break;
		}

		case WM_COMMAND:
			if (LOWORD(wParam) == IDCANCEL) {
				PostMessage(hDlg, WM_CLOSE, 0, 0);
			}
			break;

		case WM_CLOSE:
			QuitRomsScan();
			EndDialog(hDlg, 0);
			hRomsDlg = NULL;
			break;
	}

	return 0;
}

int CreateROMInfo()
{
	initAuditState();

	if (CheckGameAvb() || bRescanRoms) {
		FBADialogBox(IDD_WAIT, hSelDlg ? hSelDlg : hScrnWnd, (DLGPROC)WaitProc);
	}

	return 1;
}
