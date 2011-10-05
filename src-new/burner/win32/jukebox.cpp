#include "burner.h"

bool bJukeboxInUse = false;
bool bJukeboxDisplayed = false;

static HWND hJukeboxDlg = NULL;
static HWND hJukeboxList = NULL;
static HICON hPrevIcon = NULL;
static HICON hPlayIcon = NULL;
static HICON hPauseIcon = NULL;
static HICON hStopIcon = NULL;
static HICON hNextIcon = NULL;

static HBRUSH hWhiteBGBrush;

static TCHAR szFullName[1024];

static int nNumTracks = 0;

static int DialogInit()
{
	// Get the games full name
	TCHAR szText[1024] = _T("");
	TCHAR* pszPosition = szText;
	TCHAR* pszName = BurnDrvGetText(DRV_FULLNAME);

	pszPosition += _sntprintf(szText, 1024, pszName);
	
	pszName = BurnDrvGetText(DRV_FULLNAME);
	while ((pszName = BurnDrvGetText(DRV_NEXTNAME | DRV_FULLNAME)) != NULL) {
		if (pszPosition + _tcslen(pszName) - 1024 > szText) {
			break;
		}
		pszPosition += _stprintf(pszPosition, _T(SEPERATOR_2) _T("%s"), pszName);
	}
	
	_tcscpy(szFullName, szText);
	
	_stprintf(szText, _T("Jukebox") _T(SEPERATOR_1) _T("%s"), szFullName);
	
	// Set the window caption
	SetWindowText(hJukeboxDlg, szText);
	
	// Display the game title
	TCHAR szItemText[1024];
	HWND hInfoControl = GetDlgItem(hJukeboxDlg, IDC_TEXTCOMMENT);
	SendMessage(hInfoControl, WM_SETTEXT, (WPARAM)0, (LPARAM)szFullName);
	
	// Display the romname
	bool bBracket = false;
	hInfoControl = GetDlgItem(hJukeboxDlg, IDC_TEXTROMNAME);
	_stprintf(szItemText, _T("%s"), BurnDrvGetText(DRV_NAME));
	if ((BurnDrvGetFlags() & BDF_CLONE) && BurnDrvGetTextA(DRV_PARENT)) {
		int nOldDrvSelect = nBurnDrvSelect;
		pszName = BurnDrvGetText(DRV_PARENT);

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
		bBracket = true;
	} else {
		if (BurnDrvGetTextA(DRV_PARENT)) {
			_stprintf(szItemText + _tcslen(szItemText), _T("%suses ROMs from %s"), bBracket ? _T(", ") : _T(" ("), BurnDrvGetText(DRV_PARENT));
			bBracket = true;
		}
	}
	if (BurnDrvGetTextA(DRV_SAMPLENAME)) {
		_stprintf(szItemText + _tcslen(szItemText), _T("%suses samples from %s"), bBracket ? _T(", ") : _T(" ("), BurnDrvGetText(DRV_SAMPLENAME));
		bBracket = true;
	}
	if (bBracket) {
		_stprintf(szItemText + _tcslen(szItemText), _T(")"));
	}
	SendMessage(hInfoControl, WM_SETTEXT, (WPARAM)0, (LPARAM)szItemText);
	
	//Display the rom info
	bool bUseInfo = false;
	szItemText[0] = _T('\0');
	hInfoControl = GetDlgItem(hJukeboxDlg, IDC_TEXTROMINFO);
	if (BurnDrvGetFlags() & BDF_PROTOTYPE) {
		_stprintf(szItemText + _tcslen(szItemText), _T("prototype"));
		bUseInfo = true;
	}
	if (BurnDrvGetFlags() & BDF_BOOTLEG) {
		_stprintf(szItemText + _tcslen(szItemText), _T("%sbootleg"), bUseInfo ? _T(", ") : _T(""));
		bUseInfo = true;
	}
	if (BurnDrvGetFlags() & BDF_HACK) {
		_stprintf(szItemText + _tcslen(szItemText), _T("%shack"), bUseInfo ? _T(", ") : _T(""));
		bUseInfo = true;
	}
	if (BurnDrvGetFlags() & BDF_HOMEBREW) {
		_stprintf(szItemText + _tcslen(szItemText), _T("%shomebrew"), bUseInfo ? _T(", ") : _T(""));
		bUseInfo = true;
	}
	if (BurnDrvGetFlags() & BDF_DEMO) {
		_stprintf(szItemText + _tcslen(szItemText), _T("%sdemo"), bUseInfo ? _T(", ") : _T(""));
		bUseInfo = true;
	}
	_stprintf(szItemText + _tcslen(szItemText), _T("%s%i player%s"), bUseInfo ? _T(", ") : _T(""), BurnDrvGetMaxPlayers(), (BurnDrvGetMaxPlayers() != 1) ? _T("s max") : _T(""));
	bUseInfo = true;
	if (BurnDrvGetText(DRV_BOARDROM)) {
		_stprintf(szItemText + _tcslen(szItemText), _T("%suses board-ROMs from %s"), bUseInfo ? _T(", ") : _T(""), BurnDrvGetText(DRV_BOARDROM));
		SendMessage(hInfoControl, WM_SETTEXT, (WPARAM)0, (LPARAM)szItemText);
		bUseInfo = true;
	}
	SendMessage(hInfoControl, WM_SETTEXT, (WPARAM)0, (LPARAM)szItemText);
	
	// Display the release info
	szItemText[0] = _T('\0');
	hInfoControl = GetDlgItem(hJukeboxDlg, IDC_TEXTSYSTEM);
	//_stprintf(szItemText, _T("%s (%s, %s hardware)"), BurnDrvGetTextA(DRV_MANUFACTURER) ? BurnDrvGetText(DRV_MANUFACTURER) : _T("unknown"), BurnDrvGetText(DRV_DATE), BurnDrvGetText(DRV_SYSTEM));
	_stprintf(szItemText, _T("%s (%s, %s hardware)"), BurnDrvGetTextA(DRV_MANUFACTURER) ? BurnDrvGetText(DRV_MANUFACTURER) : _T("unknown"), BurnDrvGetText(DRV_DATE), ((BurnDrvGetHardwareCode() & HARDWARE_SNK_MVSCARTRIDGE) == HARDWARE_SNK_MVSCARTRIDGE) ? _T("Neo Geo MVS Cartidge") : BurnDrvGetText(DRV_SYSTEM));
	SendMessage(hInfoControl, WM_SETTEXT, (WPARAM)0, (LPARAM)szItemText);
	
	// Display any comments
	szItemText[0] = _T('\0');
	hInfoControl = GetDlgItem(hJukeboxDlg, IDC_TEXTNOTES);
	_stprintf(szItemText, _T("%s"), BurnDrvGetTextA(DRV_COMMENT) ? BurnDrvGetText(DRV_COMMENT) : _T(""));
	if (BurnDrvGetFlags() & BDF_HISCORE_SUPPORTED) {
		_stprintf(szItemText + _tcslen(szItemText), _T("%shigh scores supported"), _tcslen(szItemText) ? _T(", ") : _T(""));
	}
	SendMessage(hInfoControl, WM_SETTEXT, (WPARAM)0, (LPARAM)szItemText);
	
	// Display the genre
	szItemText[0] = _T('\0');
	hInfoControl = GetDlgItem(hJukeboxDlg, IDC_TEXTGENRE);
	_stprintf(szItemText, _T("%s"), DecorateGenreInfo());
	SendMessage(hInfoControl, WM_SETTEXT, (WPARAM)0, (LPARAM)szItemText);
	
	// Set up the list
	HWND hList = GetDlgItem(hJukeboxDlg, IDC_JUKEBOX_TRACKLIST);
	LV_COLUMN LvCol;
	LV_ITEM LvItem;
	
	ListView_SetExtendedListViewStyle(hList, LVS_EX_FULLROWSELECT);
	
	memset(&LvCol, 0, sizeof(LvCol));
	LvCol.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	LvCol.cx = 50;
	LvCol.pszText = _T("Num");	
	SendMessage(hList, LVM_INSERTCOLUMN , 0, (LPARAM)&LvCol);
	LvCol.cx = 360;
	LvCol.pszText = _T("Track Name");	
	SendMessage(hList, LVM_INSERTCOLUMN , 1, (LPARAM)&LvCol);
	
	memset(&LvItem, 0, sizeof(LvItem));
	LvItem.mask=  LVIF_TEXT;
	LvItem.cchTextMax = 256;
	
	// Read the list file
	CHAR szFileName[MAX_PATH] = "";
	sprintf(szFileName, "%s%s.lst", TCHARToANSI(szAppListsPath, NULL, 0), BurnDrvGetTextA(DRV_NAME));
	
	FILE *fp = fopen(szFileName, "rt");	
	char Temp[1000];
	int inTrackList = 0;
	int nDefaultTrack = 0;
	int nListPos = 0;
	
	if (fp) {		
		while (!feof(fp)) {
			fgets(Temp, 1000, fp);

			char *Tokens;

			if (!strncmp("$default=", Temp, 9)) {
				Tokens = strtok(Temp, "=");
				
				while (Tokens != NULL) {
					if (strncmp("$default=", Tokens, 9)) nDefaultTrack = strtol(Tokens, NULL, 10);

					Tokens = strtok(NULL, "=,");
				}
			}
			
			if (!strncmp("$main", Temp, 5)) {
				inTrackList = 1;
				continue;
			}
			
			if (inTrackList) {
				if (!strncmp("-----", Temp, 5)) continue;
				if (!strncmp("//", Temp, 2)) continue;
				if (!strncmp("$", Temp, 1)) continue;
				
				if (!strncmp("#", Temp, 1)) {
					char *TempString = Temp + 1;
					
					LvItem.iItem = nListPos;
					LvItem.iSubItem = 0;
					
					TCHAR TrackNo[4];
					_stprintf(TrackNo, _T("%i"), strtol(TempString, NULL, 10));
					
					LvItem.pszText = TrackNo;
					SendMessage(hList, LVM_INSERTITEM, 0, (LPARAM)&LvItem);
					
					LvItem.iSubItem = 1;
					TempString = Temp + 5;
					
					char *TempString2 = strstr(TempString, "<");
					if (TempString2) memset(TempString2, 0, strlen(TempString2));
					
					LvItem.pszText = ANSIToTCHAR(TempString, NULL, 0);
					SendMessage(hList, LVM_SETITEM, 0, (LPARAM)&LvItem);
					
					nListPos++;
				}
			}
		}
		fclose(fp);
	}
	
	nNumTracks = SendMessage(hJukeboxList, LVM_GETITEMCOUNT, 0, 0);
	
	// Make a white brush
	hWhiteBGBrush = CreateSolidBrush(RGB(0xFF,0xFF,0xFF));
	
	return 0;
}

static BOOL CALLBACK DefInpProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch (Msg) {
		case WM_INITDIALOG: {
			hJukeboxDlg = hDlg;
			
			hJukeboxList = GetDlgItem(hDlg, IDC_JUKEBOX_TRACKLIST);
			
			hPrevIcon = (HICON)LoadImage(hAppInst, MAKEINTRESOURCE(IDI_JB_PREVIOUS), IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR);
			SendMessage(GetDlgItem(hDlg, IDC_JUKEBOX_PREVTRACK), BM_SETIMAGE, IMAGE_ICON, (LPARAM)hPrevIcon);
			
			hPlayIcon = (HICON)LoadImage(hAppInst, MAKEINTRESOURCE(IDI_JB_PLAY), IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR);
			SendMessage(GetDlgItem(hDlg, IDC_JUKEBOX_PLAY), BM_SETIMAGE, IMAGE_ICON, (LPARAM)hPlayIcon);
			
			hPauseIcon = (HICON)LoadImage(hAppInst, MAKEINTRESOURCE(IDI_JB_PAUSE), IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR);
			SendMessage(GetDlgItem(hDlg, IDC_JUKEBOX_PAUSE), BM_SETIMAGE, IMAGE_ICON, (LPARAM)hPauseIcon);
			
			hStopIcon = (HICON)LoadImage(hAppInst, MAKEINTRESOURCE(IDI_JB_STOP), IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR);
			SendMessage(GetDlgItem(hDlg, IDC_JUKEBOX_STOP), BM_SETIMAGE, IMAGE_ICON, (LPARAM)hStopIcon);
			
			hNextIcon = (HICON)LoadImage(hAppInst, MAKEINTRESOURCE(IDI_JB_NEXT), IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR);
			SendMessage(GetDlgItem(hDlg, IDC_JUKEBOX_NEXTTRACK), BM_SETIMAGE, IMAGE_ICON, (LPARAM)hNextIcon);
			
			DialogInit();
			
			return true;
		}
		
		case WM_COMMAND: {
			if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDC_JUKEBOX_PREVTRACK) {
				int nSel = SendMessage(hJukeboxList, LVM_GETNEXTITEM, (WPARAM)-1, LVNI_SELECTED);
				
				// Deselect current item
				if (nSel >= 0) ListView_SetItemState(hJukeboxList, nSel, 0, 0x000f);
				
				nSel--;
				if (nSel < 0) {
					nSel = nNumTracks - 1;
					SendMessage(hJukeboxList, LVM_SCROLL, 0, 10000); // should get us to the bottom!
				}
				SendMessage(hJukeboxList, LVM_SCROLL, 0, -16);
				ListView_SetItemState(hJukeboxList, nSel, LVIS_FOCUSED | LVIS_SELECTED, 0x000f);
				SendMessage(hDlg, WM_NEXTDLGCTL, (WPARAM)hJukeboxList, TRUE);
			}
		
			if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDC_JUKEBOX_NEXTTRACK) {
				int nSel = SendMessage(hJukeboxList, LVM_GETNEXTITEM, (WPARAM)-1, LVNI_SELECTED);
				
				// Deselect current item
				if (nSel >= 0) ListView_SetItemState(hJukeboxList, nSel, 0, 0x000f);
				
				nSel++;
				if (nSel >= nNumTracks) {
					nSel = 0;
					SendMessage(hJukeboxList, LVM_SCROLL, 0, -10000); // should get us to the top!
				}
				if (nSel > 10) SendMessage(hJukeboxList, LVM_SCROLL, 0, 16);
				ListView_SetItemState(hJukeboxList, nSel, LVIS_FOCUSED | LVIS_SELECTED, 0x000f);
				SendMessage(hDlg, WM_NEXTDLGCTL, (WPARAM)hJukeboxList, TRUE);
			}
			
			if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDC_JUKEBOX_STOP) {
				JukeboxSoundCommand = JUKEBOX_SOUND_STOP;
				SendMessage(hDlg, WM_NEXTDLGCTL, (WPARAM)hJukeboxList, TRUE);
			}
			
			if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDC_JUKEBOX_PLAY) {
				JukeboxSoundCommand = JUKEBOX_SOUND_PLAY;
				SendMessage(hDlg, WM_NEXTDLGCTL, (WPARAM)hJukeboxList, TRUE);
			}
			
			return 0;
		}
		
		case WM_CLOSE: {
//			bJukeboxDisplayed = false;
//			bJukeboxInUse = false;
			
			hJukeboxDlg = NULL;
			hJukeboxList = NULL;
			
			nNumTracks = 0;
			
			DeleteObject(hWhiteBGBrush);

			if (hPrevIcon) {
				DestroyIcon(hPrevIcon);
				hPrevIcon = NULL;
			}
			
			if (hPlayIcon) {
				DestroyIcon(hPlayIcon);
				hPlayIcon = NULL;
			}
			
			if (hPauseIcon) {
				DestroyIcon(hPauseIcon);
				hPauseIcon = NULL;
			}
			
			if (hStopIcon) {
				DestroyIcon(hStopIcon);
				hStopIcon = NULL;
			}
			
			if (hNextIcon) {
				DestroyIcon(hNextIcon);
				hNextIcon = NULL;
			}
			
			EnableWindow(hScrnWnd, true);
			EndDialog(hDlg, 0);

			return 0;
		}
		
		case WM_CTLCOLORSTATIC: {
			if ((HWND)lParam == GetDlgItem(hDlg, IDC_LABELCOMMENT) || (HWND)lParam == GetDlgItem(hDlg, IDC_LABELROMNAME) || (HWND)lParam == GetDlgItem(hDlg, IDC_LABELROMINFO) || (HWND)lParam == GetDlgItem(hDlg, IDC_LABELSYSTEM) || (HWND)lParam == GetDlgItem(hDlg, IDC_LABELNOTES) || (HWND)lParam == GetDlgItem(hDlg, IDC_LABELGENRE) || (HWND)lParam == GetDlgItem(hDlg, IDC_TEXTCOMMENT) || (HWND)lParam == GetDlgItem(hDlg, IDC_TEXTROMNAME) || (HWND)lParam == GetDlgItem(hDlg, IDC_TEXTROMINFO) || (HWND)lParam == GetDlgItem(hDlg, IDC_TEXTSYSTEM) || (HWND)lParam == GetDlgItem(hDlg, IDC_TEXTNOTES) || (HWND)lParam == GetDlgItem(hDlg, IDC_TEXTGENRE)) {
				return (BOOL)hWhiteBGBrush;
			}
			return 0;
		}
		
		case WM_NOTIFY: {
			int Id = LOWORD(wParam);
			NMHDR *pnm = (NMHDR*)lParam;

			if (Id == IDC_JUKEBOX_TRACKLIST && (pnm->code == NM_CLICK || pnm->code == LVN_ITEMCHANGED)) {
				int nSel = SendMessage(GetDlgItem(hDlg, IDC_JUKEBOX_TRACKLIST), LVM_GETNEXTITEM, (WPARAM)-1, LVNI_SELECTED);
				
				TCHAR Value[4];
				
				LVITEM LvItem;
				memset(&LvItem, 0, sizeof(LvItem));
				LvItem.mask = LVIF_TEXT;
				LvItem.iItem = nSel;
				LvItem.iSubItem = 0;
				LvItem.cchTextMax = 5;
				LvItem.pszText = Value;
				SendMessage(GetDlgItem(hDlg, IDC_JUKEBOX_TRACKLIST), LVM_GETITEMTEXT, (WPARAM)nSel, (LPARAM)&LvItem);
				
				char *Temp;
				Temp = TCHARToANSI(Value, NULL, 0);
				
				JukeboxSoundLatch = strtol(Temp, NULL, 10);
				JukeboxSoundCommand = JUKEBOX_SOUND_PLAY;
				
				return 0;
			}
		}
	}

	return 0;
}

int JukeboxDialogCreate()
{
	HWND hJukebox = NULL;
	
	hJukebox = FBACreateDialog(hAppInst, MAKEINTRESOURCE(IDD_JUKEBOX), hScrnWnd, DefInpProc);
	
	WndInMid(hJukebox, hScrnWnd);
	ShowWindow(hJukebox, SW_NORMAL);
	
	bJukeboxDisplayed = true;

	return 0;
}
