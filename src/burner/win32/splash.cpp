// Splash screen code
#include "burner.h"

int nSplashTime = 1500;
int nDisableSplash = 0;

static HWND hSplashDlg = NULL;
static HANDLE hSplashThread = NULL;
static DWORD dwSplashThreadID = 0;

static clock_t StartTime;

static INT_PTR CALLBACK SplashProc(HWND hDlg, UINT Msg, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	if (Msg == WM_INITDIALOG) {
		hSplashDlg = hDlg;

		// ==> load custom resource, added by regret
		HBITMAP hBitmap = (HBITMAP)LoadImage(hAppInst, _T("res\\splash.bmp"), IMAGE_BITMAP, 304, 224, LR_LOADFROMFILE);
		if (!hBitmap) {
			hBitmap = LoadBitmap(hAppInst, MAKEINTRESOURCE(BMP_SPLASH));
		}
		SendDlgItemMessage(hDlg, IDC_SPLASH, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBitmap);
		// <== load custom resource

#if 0
		RECT rect;
		SystemParametersInfo(SPI_GETWORKAREA, 0, &rect, 0);
		int x = 304 + GetSystemMetrics(SM_CXDLGFRAME) * 2 + 6;
		int y = 224 + GetSystemMetrics(SM_CYDLGFRAME) * 2 + 6;
		SetWindowPos(hDlg, HWND_TOPMOST, (rect.right - rect.left) / 2 - x / 2, (rect.bottom - rect.top) / 2 - y / 2, x, y, 0);
#else
		SetWindowPos(hDlg, HWND_TOPMOST, 0, 0, 304, 224, SWP_NOMOVE | SWP_SHOWWINDOW);
#endif

		SetForegroundWindow(hDlg);
		RedrawWindow(hDlg, NULL, NULL, 0);
		ShowWindow(hDlg, SW_SHOWNORMAL);

		return TRUE;
	}

	return FALSE;
}

static DWORD WINAPI DoSplash(LPVOID)
{
	MSG msg;
	BOOL bRet;

	// Raise the thread priority for this thread
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

	FBACreateDialog(IDD_SPLASH, NULL, (DLGPROC)SplashProc);

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

	DestroyWindow(hSplashDlg);

	return 0;
}

int SplashCreate()
{
	if (nDisableSplash != 0) {
		return 1;
	}
	if (hSplashDlg || !nSplashTime || hSplashThread) {
		return 1;
	}

	hSplashThread = CreateThread(NULL, 0, DoSplash, NULL, THREAD_TERMINATE, &dwSplashThreadID);

	StartTime = clock();

	return 0;
}

void SplashDestroy(bool bForce)
{
	if (hSplashThread == NULL) {
		return;
	}

	if (!bForce && (clock() - nSplashTime < StartTime)) {
		return;
	}

	// Signal the splash thread to end
	PostThreadMessage(dwSplashThreadID, WM_APP + 0, 0, 0);

	// Wait for the thread to finish
	if (WaitForSingleObject(hSplashThread, 10000) != WAIT_OBJECT_0) {
		// If the thread doesn't finish within 10 seconds, forcibly kill it
		TerminateThread(hSplashThread, 1);
	}

	hSplashDlg = NULL;

	CloseHandle(hSplashThread);

	hSplashThread = NULL;
	dwSplashThreadID = 0;
}
