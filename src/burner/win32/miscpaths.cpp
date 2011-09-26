// custom misc paths, by regret
#include "burner.h"
#include <shlobj.h>

TCHAR szMiscPaths[PATH_SUM][MAX_PATH] = {
	_T("previews\\"), _T("cheats\\"), _T("screenshots\\"), _T("savestates\\"), _T("recordings\\"),
	_T("skins\\"), _T("ips\\"), _T("titles\\"), _T("flyers\\"),
	_T("scores\\"), _T("selects\\"), _T("gameovers\\"), _T("bosses\\"), _T("icons\\"),
};

TCHAR szAppRomPaths[DIRS_MAX][MAX_PATH] = {
	{ _T("") }
};

// Function for directory options
static int CALLBACK browseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	if (uMsg == BFFM_INITIALIZED && lpData) {
		SendMessage(hwnd, BFFM_SETSELECTION, (WPARAM)1, (LPARAM)lpData);
	}
	return 0;
}

// ROM paths
static INT_PTR CALLBACK ROMDirProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch (Msg) {
		case WM_INITDIALOG: {
			for (int i = 0; i < DIRS_MAX; i++) {
				SetDlgItemText(hDlg, IDC_DIR_EDIT1 + i, szAppRomPaths[i]);
			}

			wndInMid(hDlg, hSelDlg ? hSelDlg : hScrnWnd);
			SetFocus(hDlg);
			break;
		}
		case WM_COMMAND: {
			int var;
			TCHAR buffer[MAX_PATH] = _T("");

			if (LOWORD(wParam) == IDOK) {
				SendMessage(hDlg, WM_CLOSE, 0, 0);
				break;
			} else {
				if (LOWORD(wParam) >= IDC_DIR_BR1 && LOWORD(wParam) <= IDC_DIR_BR14) {
					var = IDC_DIR_EDIT1 + LOWORD(wParam) - IDC_DIR_BR1;
					GetDlgItemText(hDlg, var, buffer, sizearray(buffer));
				} else {
					if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDCANCEL) {
						SendMessage(hDlg, WM_CLOSE, 0, 0);
					}
					break;
				}
			}

			BROWSEINFO bInfo;
			memset(&bInfo, 0, sizeof(bInfo));
			bInfo.hwndOwner = hDlg;
			bInfo.lpszTitle = FBALoadStringEx(IDS_ROMS_SELECT_DIR);
			bInfo.ulFlags = BIF_EDITBOX | BIF_RETURNONLYFSDIRS;
			bInfo.lpfn = browseCallbackProc;
			if (buffer) {
				bInfo.lParam = (LPARAM)buffer;
			}

			LPITEMIDLIST pItemIDList = SHBrowseForFolder(&bInfo);
			if (pItemIDList) {
				if (SHGetPathFromIDList(pItemIDList, buffer)) {
					pathCheck(buffer);
					SetDlgItemText(hDlg, var, buffer);
				}
				LPMALLOC pMalloc = NULL;
				SHGetMalloc(&pMalloc);
				if (pMalloc) {
					pMalloc->Free(pItemIDList);
					pMalloc->Release();
				}
			}

			break;
		}

		case WM_NOTIFY: {
			switch (((NMHDR *)lParam)->code) {
				case PSN_APPLY:
					TCHAR buffer[MAX_PATH] = _T("");
					for (int i = 0; i < DIRS_MAX; i++) {
						if (GetDlgItemText(hDlg, IDC_DIR_EDIT1 + i, buffer, sizearray(buffer))) {
							pathCheck(buffer);
						}
						_tcscpy(szAppRomPaths[i], buffer);
					}
			}
		}
		break;

		case WM_CLOSE: {
			EndDialog(hDlg, 0);
//			if (chOk) {
//				bRescanRoms = true;
//				CreateROMInfo();
//			}
		}
	}

	return 0;
}


int RomsDirCreate(HWND parent)
{
	FBADialogBox(IDD_ROMSDIR, parent, (DLGPROC)ROMDirProc);
	return 0;
}

// Misc paths
static INT_PTR CALLBACK miscPathProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	int var;

	switch (Msg) {
		case WM_INITDIALOG: {
			for (int i = 0; i < PATH_SUM; i++) {
				SetDlgItemText(hDlg, IDC_DIR_EDIT1 + i, szMiscPaths[PATH_PREVIEW + i]);
			}

			wndInMid(hDlg, hSelDlg ? hSelDlg : hScrnWnd);
			SetFocus(hDlg);
			return TRUE;
		}
		case WM_COMMAND: {
			TCHAR buffer[MAX_PATH] = _T("");

			if (LOWORD(wParam) == IDOK) {
				SendMessage(hDlg, WM_CLOSE, 0, 0);
				break;
			}
			else if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == ID_DEFAULT) {
				// set default path
				SetDlgItemText(hDlg, IDC_DIR_EDIT1, _T("previews\\"));
				SetDlgItemText(hDlg, IDC_DIR_EDIT2, _T("cheats\\"));
				SetDlgItemText(hDlg, IDC_DIR_EDIT3, _T("screenshots\\"));
				SetDlgItemText(hDlg, IDC_DIR_EDIT4, _T("savestates\\"));
				SetDlgItemText(hDlg, IDC_DIR_EDIT5, _T("recordings\\"));
				SetDlgItemText(hDlg, IDC_DIR_EDIT6, _T("skins\\"));
				SetDlgItemText(hDlg, IDC_DIR_EDIT7, _T("ips\\"));

				SetDlgItemText(hDlg, IDC_DIR_EDIT8, _T("titles\\"));
				SetDlgItemText(hDlg, IDC_DIR_EDIT9, _T("flyers\\"));
				SetDlgItemText(hDlg, IDC_DIR_EDIT10, _T("scores\\"));
				SetDlgItemText(hDlg, IDC_DIR_EDIT11, _T("selects\\"));
				SetDlgItemText(hDlg, IDC_DIR_EDIT12, _T("gameovers\\"));
				SetDlgItemText(hDlg, IDC_DIR_EDIT13, _T("bosses\\"));
				SetDlgItemText(hDlg, IDC_DIR_EDIT14, _T("icons\\"));
				break;
			} else {
				if (LOWORD(wParam) >= IDC_DIR_BR1 && LOWORD(wParam) <= IDC_DIR_BR14) {
					var = IDC_DIR_EDIT1 + LOWORD(wParam) - IDC_DIR_BR1;
					GetDlgItemText(hDlg, var, buffer, sizearray(buffer));
				} else {
					if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDCANCEL) {
						SendMessage(hDlg, WM_CLOSE, 0, 0);
					}
					break;
				}
			}

			BROWSEINFO bInfo;
			memset(&bInfo, 0, sizeof(bInfo));
			bInfo.hwndOwner = hDlg;
			bInfo.lpszTitle = FBALoadStringEx(IDS_ROMS_SELECT_DIR);
			bInfo.ulFlags = BIF_EDITBOX | BIF_RETURNONLYFSDIRS;
			bInfo.lpfn = browseCallbackProc;
			if (buffer) {
				bInfo.lParam = (LPARAM)buffer;
			}

			LPITEMIDLIST pItemIDList = SHBrowseForFolder(&bInfo);
			if (pItemIDList) {
				if (SHGetPathFromIDList(pItemIDList, buffer)) {
					pathCheck(buffer);
					SetDlgItemText(hDlg, var, buffer);
				}
				LPMALLOC pMalloc = NULL;
				SHGetMalloc(&pMalloc);
				if (pMalloc) {
					pMalloc->Free(pItemIDList);
					pMalloc->Release();
				}
			}

			break;
		}

		case WM_NOTIFY: {
			switch (((NMHDR *)lParam)->code) {
				case PSN_APPLY:
					TCHAR buffer[MAX_PATH] = _T("");
					for (int i = 0; i < PATH_SUM; i++) {
						if (GetDlgItemText(hDlg, IDC_DIR_EDIT1 + i, buffer, sizearray(buffer)) && _tcscmp(szMiscPaths[i], buffer)) {
							pathCheck(buffer);
							_tcscpy(szMiscPaths[i], buffer);
						}
					}
					break;
			}
		}
		break;

		case WM_CLOSE: {
			EndDialog(hDlg, 0);
		}
	}

	return 0;
}

int miscDirCreate(HWND parent)
{
	FBADialogBox(IDD_MISCDIR, parent, (DLGPROC)miscPathProc);
	return 0;
}

const TCHAR* getMiscPath(unsigned int dirType)
{
	if (dirType < PATH_PREVIEW || dirType >= PATH_SUM) {
		return NULL;
	}
	return szMiscPaths[dirType];
}

const TCHAR* getMiscArchiveName(unsigned int dirType)
{
	if (dirType < PATH_PREVIEW || dirType >= PATH_SUM) {
		return NULL;
	}

	static TCHAR szArchiveName[64] = _T("");

	switch (dirType) {
		case PATH_PREVIEW:
			_tcscpy(szArchiveName, _T("snap"));
			break;
		case PATH_TITLE:
			_tcscpy(szArchiveName, _T("titles"));
			break;
		case PATH_FLYER:
			_tcscpy(szArchiveName, _T("flyers"));
			break;
		case PATH_SCORE:
			_tcscpy(szArchiveName, _T("score"));
			break;
		case PATH_SELECT:
			_tcscpy(szArchiveName, _T("select"));
			break;
		case PATH_GAMEOVER:
			_tcscpy(szArchiveName, _T("gameover"));
			break;
		case PATH_BOSS:
			_tcscpy(szArchiveName, _T("boss"));
			break;

		case PATH_CHEAT:
			_tcscpy(szArchiveName, _T("cheat"));
			break;
		case PATH_SKIN:
			_tcscpy(szArchiveName, _T("skin"));
			break;
		case PATH_ICON:
			_tcscpy(szArchiveName, _T("icons"));
			break;
	}

	return szArchiveName;
}

// Property sheet
void pathSheetCreate(HWND parent)
{
	HINSTANCE instance = FBALocaliseInstance();
	PROPSHEETPAGE psp[2];
	PROPSHEETHEADER psh;

	for (int i = 0; i < sizearray(psp); i++) {
		psp[i].dwSize = sizeof(PROPSHEETPAGE);
		psp[i].dwFlags = PSP_DEFAULT;
		psp[i].hInstance = instance;
		psp[i].lParam = 0;
		psp[i].pfnCallback = NULL;
	}
	psp[1].pszTemplate = MAKEINTRESOURCE(IDD_MISCDIR);
	psp[1].pfnDlgProc = (DLGPROC)miscPathProc;
	psp[0].pszTemplate = MAKEINTRESOURCE(IDD_ROMSDIR);
	psp[0].pfnDlgProc = (DLGPROC)ROMDirProc;

	psh.dwSize = sizeof(PROPSHEETHEADER);
	psh.dwFlags = PSH_PROPSHEETPAGE | PSH_NOAPPLYNOW | PSH_NOCONTEXTHELP;
	psh.hwndParent = parent;
	psh.hInstance = instance;
	psh.pszCaption = FBALoadStringEx(IDS_PREF_PATHSETTING);
	psh.nPages = sizearray(psp);
	psh.nStartPage = 0;
	psh.ppsp = (LPCPROPSHEETPAGE)&psp;
	psh.pfnCallback = NULL;
	PropertySheet(&psh);
}
