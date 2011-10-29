#include "burner.h"
#include <shlobj.h>

static HWND hTabControl = NULL;

TCHAR szAppPreviewsPath[MAX_PATH]	= _T("support\\previews\\");
TCHAR szAppTitlesPath[MAX_PATH]		= _T("support\\titles\\");
TCHAR szAppCheatsPath[MAX_PATH]		= _T("support\\cheats\\");
TCHAR szAppHiscorePath[MAX_PATH]	= _T("support\\hiscores\\");
TCHAR szAppSamplesPath[MAX_PATH]	= _T("support\\samples\\");
TCHAR szAppIpsPath[MAX_PATH]		= _T("support\\ips\\");

static INT_PTR CALLBACK DefInpProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	int var;

	switch (Msg) {
		case WM_INITDIALOG: {
			SetDlgItemText(hDlg, IDC_SUPPORTDIR_EDIT1, szAppPreviewsPath);
			SetDlgItemText(hDlg, IDC_SUPPORTDIR_EDIT2, szAppTitlesPath);
			SetDlgItemText(hDlg, IDC_SUPPORTDIR_EDIT3, szAppCheatsPath);
			SetDlgItemText(hDlg, IDC_SUPPORTDIR_EDIT4, szAppHiscorePath);
			SetDlgItemText(hDlg, IDC_SUPPORTDIR_EDIT5, szAppSamplesPath);
			SetDlgItemText(hDlg, IDC_SUPPORTDIR_EDIT6, szAppIpsPath);

			// Setup the tabs
			hTabControl = GetDlgItem(hDlg, IDC_SPATH_TAB);

			TC_ITEM tcItem; 
			tcItem.mask = TCIF_TEXT;

			UINT idsString[6] = { IDS_SPATH_PREVIEW, IDS_SPATH_TITLES, IDS_SPATH_CHEATS, IDS_SPATH_HISCORE, IDS_SPATH_SAMPLES, IDS_SPATH_IPS };
			
			for(int nIndex = 0; nIndex < 6; nIndex++) {
				tcItem.pszText = FBALoadStringEx(hAppInst, idsString[nIndex], true);
				TabCtrl_InsertItem(hTabControl, nIndex, &tcItem);
			}

			int TabPage = TabCtrl_GetCurSel(hTabControl);
			
			// hide all controls excluding the selected controls
			for(int x = 0; x < 6; x++) {
				if(x != TabPage) {
					ShowWindow(GetDlgItem(hDlg, IDC_SUPPORTDIR_BR1 + x), SW_HIDE);		// browse buttons
					ShowWindow(GetDlgItem(hDlg, IDC_SUPPORTDIR_EDIT1 + x), SW_HIDE);	// edit controls
				}
			}

			// Show the proper controls
			ShowWindow(GetDlgItem(hDlg, IDC_SUPPORTDIR_BR1 + TabPage), SW_SHOW);		// browse buttons
			ShowWindow(GetDlgItem(hDlg, IDC_SUPPORTDIR_EDIT1 + TabPage), SW_SHOW);		// edit controls

			WndInMid(hDlg, hScrnWnd);
			SetFocus(hDlg);											// Enable Esc=close
			break;
		}

		case WM_NOTIFY:
		{
			NMHDR* pNmHdr = (NMHDR*)lParam;

			if (pNmHdr->code == TCN_SELCHANGE) {

				int TabPage = TabCtrl_GetCurSel(hTabControl);
				
				// hide all controls excluding the selected controls
				for(int x = 0; x < 6; x++) {
					if(x != TabPage) {
						ShowWindow(GetDlgItem(hDlg, IDC_SUPPORTDIR_BR1 + x), SW_HIDE);		// browse buttons
						ShowWindow(GetDlgItem(hDlg, IDC_SUPPORTDIR_EDIT1 + x), SW_HIDE);	// edit controls
					}
				}

				// Show the proper controls
				ShowWindow(GetDlgItem(hDlg, IDC_SUPPORTDIR_BR1 + TabPage), SW_SHOW);		// browse buttons
				ShowWindow(GetDlgItem(hDlg, IDC_SUPPORTDIR_EDIT1 + TabPage), SW_SHOW);		// edit controls
				
				UpdateWindow(hDlg);

				return FALSE;
			}
			break;
		}

		case WM_COMMAND: {
			LPMALLOC pMalloc = NULL;
			BROWSEINFO bInfo;
			ITEMIDLIST* pItemIDList = NULL;
			TCHAR buffer[MAX_PATH];
			
			if (LOWORD(wParam) == IDOK) {
				GetDlgItemText(hDlg, IDC_SUPPORTDIR_EDIT1, szAppPreviewsPath,	sizeof(szAppPreviewsPath));
				GetDlgItemText(hDlg, IDC_SUPPORTDIR_EDIT2, szAppTitlesPath,		sizeof(szAppTitlesPath));
				GetDlgItemText(hDlg, IDC_SUPPORTDIR_EDIT3, szAppCheatsPath,	sizeof(szAppCheatsPath));
				GetDlgItemText(hDlg, IDC_SUPPORTDIR_EDIT4, szAppHiscorePath,	sizeof(szAppHiscorePath));
				GetDlgItemText(hDlg, IDC_SUPPORTDIR_EDIT5, szAppSamplesPath,	sizeof(szAppSamplesPath));
				GetDlgItemText(hDlg, IDC_SUPPORTDIR_EDIT6, szAppIpsPath,	sizeof(szAppIpsPath));

				SendMessage(hDlg, WM_CLOSE, 0, 0);
				break;
			} else {
				if (LOWORD(wParam) >= IDC_SUPPORTDIR_BR1 && LOWORD(wParam) <= IDC_SUPPORTDIR_BR6) {
					var = IDC_SUPPORTDIR_EDIT1 + LOWORD(wParam) - IDC_SUPPORTDIR_BR1;
				} else {
					if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDCANCEL) {
						SendMessage(hDlg, WM_CLOSE, 0, 0);
					}
					break;
				}
			}
			
			SHGetMalloc(&pMalloc);

			memset(&bInfo, 0, sizeof(bInfo));
			bInfo.hwndOwner = hDlg;
			bInfo.pszDisplayName = buffer;
			bInfo.lpszTitle = FBALoadStringEx(hAppInst, IDS_ROMS_SELECT_DIR, true);
			bInfo.ulFlags = BIF_EDITBOX | BIF_RETURNONLYFSDIRS;

			pItemIDList = SHBrowseForFolder(&bInfo);

			if (pItemIDList) {
				if (SHGetPathFromIDList(pItemIDList, buffer)) {
					int strLen = _tcslen(buffer);
					if (strLen) {
						if (buffer[strLen - 1] != _T('\\')) {
							buffer[strLen]		= _T('\\');
							buffer[strLen + 1]	= _T('\0');
						}
						SetDlgItemText(hDlg, var, buffer);
					}
				}
				pMalloc->Free(pItemIDList);
			}
			pMalloc->Release();
			
			break;
		}
		
		case WM_CLOSE: {
			EndDialog(hDlg, 0);
			break;
		}
	}

	return 0;
}

int SupportDirCreate()
{
	FBADialogBox(hAppInst, MAKEINTRESOURCE(IDD_SUPPORTDIR), hScrnWnd, (DLGPROC)DefInpProc);
	return 1;
}
