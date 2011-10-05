#include "burner.h"

static HWND hNgslotDlg	= NULL;
static HWND hParent		= NULL;
static HBITMAP hPreview = NULL;
static HBITMAP hBmp[6]	= { NULL, NULL, NULL, NULL, NULL, NULL};
static HBRUSH hWhiteBGBrush;
int nMultiSlotRet = 0;


void SetMVSCartPreview(int nStaticControl) 
{
	bool bParent			= false;
	bool bBoard				= false;
	TCHAR szDrvName[128]	= _T("");
	TCHAR szDrvParent[128]	= _T("");
	TCHAR szDrvBoard[128]	= _T("");

	// Check if we can get the values and proceed (FBA will crash if used incorrecly)
	if(BurnDrvGetText(DRV_NAME)) _stprintf(szDrvName, BurnDrvGetText(DRV_NAME));
	if(BurnDrvGetText(DRV_PARENT)) bParent = true;
	if(BurnDrvGetText(DRV_BOARDROM)) bBoard = true;
	if(bParent) _stprintf(szDrvParent, BurnDrvGetText(DRV_PARENT));
	if(bBoard) _stprintf(szDrvBoard, BurnDrvGetText(DRV_BOARDROM));

	int nBmpSlot = nStaticControl;

	HBITMAP hNewImage			= NULL;
	TCHAR	szFDriver[MAX_PATH]	= _T("");
	bool	bLoadPNG			= false;
	bool	bLoadZipPNG			= false;

	int idcStaticControl[6] = { IDC_NGSLOT1_IMAGE, IDC_NGSLOT2_IMAGE, IDC_NGSLOT3_IMAGE, IDC_NGSLOT4_IMAGE, IDC_NGSLOT5_IMAGE, IDC_NGSLOT6_IMAGE };

	if (hBmp[nBmpSlot]) {
		DeleteObject((HGDIOBJ)hBmp[nBmpSlot]);
		hBmp[nBmpSlot] = NULL;
	}

	do {

		// ====================================================================
		// Load PNG from directory

		if(!CheckFile(szDrvName,	szAppTitlesPath, _T(".png"))) 
		{	
			_stprintf(szFDriver, szDrvName);
			bLoadPNG = true;
			break;
		}

		if(bParent) {
			if(!CheckFile(szDrvParent,	szAppTitlesPath, _T(".png"))) 
			{	
				_stprintf(szFDriver, szDrvParent);
				bLoadPNG = true;
				break;
			}
		}

		if(bBoard) {
			if(!CheckFile(szDrvBoard,	szAppTitlesPath, _T(".png"))) 
			{ 
				_stprintf(szFDriver, szDrvBoard);
				bLoadPNG = true;
				break;
			}
		}

		// ====================================================================
		// Load PNG from ZIP

		if(!CheckZipFile(szDrvName,		szAppTitlesPath, _T(".png"))) 
		{	
			_stprintf(szFDriver, szDrvName);
			bLoadPNG = true; 
			bLoadZipPNG = true;
			break;
		}

		if(bParent) {
			if(!CheckZipFile(szDrvParent,	szAppTitlesPath, _T(".png"))) 
			{	
				_stprintf(szFDriver, szDrvParent);
				bLoadPNG = true; 
				bLoadZipPNG = true;
				break;
			}
		}

		if(bBoard) {
			if(!CheckZipFile(szDrvBoard,	szAppTitlesPath, _T(".png"))) 
			{ 
				_stprintf(szFDriver, szDrvBoard);
				bLoadPNG = true; 
				bLoadZipPNG = true;
				break;
			}
		}

		if(!bLoadPNG) break;

	} while (!bLoadPNG);

	if (bLoadPNG) {
		//extern bool bGameInfoOpen;
		//bGameInfoOpen = true;		// just a temporary fake  flag

		char szTemp[2][MAX_PATH];		
		sprintf(szTemp[0], _TtoA(szAppTitlesPath));
		sprintf(szTemp[1], _TtoA(szFDriver));

		// Load PNG from ZIP if we can't get a handle from the directory file
		if(bLoadZipPNG) {
			hNewImage = PNGtoBMP(hNgslotDlg, szTemp[0], szTemp[1], FBA_LM_ZIP_BUFF, 70, 70);
		} else {
			hNewImage = PNGtoBMP(hNgslotDlg, szTemp[0], szTemp[1], FBA_LM_FILE, 70, 70);
		}
		//bGameInfoOpen = false;		// done with the temprorary fake flag
	}
	
	if (hNewImage) {
		DeleteObject((HGDIOBJ)hBmp[nBmpSlot]);
		hBmp[nBmpSlot] = hNewImage;		
		SendDlgItemMessage(hNgslotDlg, idcStaticControl[nStaticControl], STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBmp[nBmpSlot]);
	} else {
		if (!hBmp[nBmpSlot]) {
			SendDlgItemMessage(hNgslotDlg, idcStaticControl[nStaticControl], STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hPreview);
		}
	}
}

void ScanTitlePreviews()
{
	unsigned int nOldSelection = nBurnDrvSelect;

	for (int i = 0; i < MAX_NEO_SLOTS; i++) {
		if (nNeoSlotDrvNum[i] != (unsigned int)-1){
			nBurnDrvSelect = nNeoSlotDrvNum[i];
			SetMVSCartPreview(i);			
		}
	}

	nBurnDrvSelect = nOldSelection;
}

void InitContent(HWND hDlg) 
{
	hNgslotDlg = hDlg;
			
	hPreview = LoadBitmap(hAppInst, MAKEINTRESOURCE(BMP_SMALLPREVIEW));
	SendDlgItemMessage(hDlg, IDC_NGSLOT1_IMAGE, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hPreview);
	SendDlgItemMessage(hDlg, IDC_NGSLOT2_IMAGE, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hPreview);
	SendDlgItemMessage(hDlg, IDC_NGSLOT3_IMAGE, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hPreview);
	SendDlgItemMessage(hDlg, IDC_NGSLOT4_IMAGE, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hPreview);
	SendDlgItemMessage(hDlg, IDC_NGSLOT5_IMAGE, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hPreview);
	SendDlgItemMessage(hDlg, IDC_NGSLOT6_IMAGE, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hPreview);
			
	SendMessage(GetDlgItem(hDlg, IDC_NGSLOT1_TEXT), WM_SETTEXT, (WPARAM)0, (LPARAM)_T("Slot 1"));
	SendMessage(GetDlgItem(hDlg, IDC_NGSLOT1_ROMNAME), WM_SETTEXT, (WPARAM)0, (LPARAM)_T("Empty"));
	SendMessage(GetDlgItem(hDlg, IDC_NGSLOT2_TEXT), WM_SETTEXT, (WPARAM)0, (LPARAM)_T("Slot 2"));
	SendMessage(GetDlgItem(hDlg, IDC_NGSLOT2_ROMNAME), WM_SETTEXT, (WPARAM)0, (LPARAM)_T("Empty"));
	SendMessage(GetDlgItem(hDlg, IDC_NGSLOT3_TEXT), WM_SETTEXT, (WPARAM)0, (LPARAM)_T("Slot 3"));
	SendMessage(GetDlgItem(hDlg, IDC_NGSLOT3_ROMNAME), WM_SETTEXT, (WPARAM)0, (LPARAM)_T("Empty"));
	SendMessage(GetDlgItem(hDlg, IDC_NGSLOT4_TEXT), WM_SETTEXT, (WPARAM)0, (LPARAM)_T("Slot 4"));
	SendMessage(GetDlgItem(hDlg, IDC_NGSLOT4_ROMNAME), WM_SETTEXT, (WPARAM)0, (LPARAM)_T("Empty"));
	SendMessage(GetDlgItem(hDlg, IDC_NGSLOT5_TEXT), WM_SETTEXT, (WPARAM)0, (LPARAM)_T("Slot 5"));
	SendMessage(GetDlgItem(hDlg, IDC_NGSLOT5_ROMNAME), WM_SETTEXT, (WPARAM)0, (LPARAM)_T("Empty"));
	SendMessage(GetDlgItem(hDlg, IDC_NGSLOT6_TEXT), WM_SETTEXT, (WPARAM)0, (LPARAM)_T("Slot 6"));
	SendMessage(GetDlgItem(hDlg, IDC_NGSLOT6_ROMNAME), WM_SETTEXT, (WPARAM)0, (LPARAM)_T("Empty"));
	
	for (int i = 0; i < MAX_NEO_SLOTS; i++) {
		nNeoSlotDrvNum[i] = (unsigned int)-1;
	}

	// Make a white brush
	hWhiteBGBrush = CreateSolidBrush(RGB(0xFF,0xFF,0xFF));

	WndInMid(hDlg, hParent);

	ImageButton_EnableXPThemes();
	
	ImageButton_Create(hDlg, IDCANCEL);
	ImageButton_Create(hDlg, IDOK);
	ImageButton_SetIcon(GetDlgItem(hDlg, IDCANCEL), IDI_CANCEL, 0,0,16,16);
	ImageButton_SetIcon(GetDlgItem(hDlg, IDOK), IDI_PLAY, 0,0,16,16);

	ImageButton_Create(hDlg, IDC_NGSLOT1_SELECT);
	ImageButton_Create(hDlg, IDC_NGSLOT2_SELECT);
	ImageButton_Create(hDlg, IDC_NGSLOT3_SELECT);
	ImageButton_Create(hDlg, IDC_NGSLOT4_SELECT);
	ImageButton_Create(hDlg, IDC_NGSLOT5_SELECT);
	ImageButton_Create(hDlg, IDC_NGSLOT6_SELECT);
	ImageButton_SetIcon(GetDlgItem(hDlg, IDC_NGSLOT1_SELECT), IDI_NGSLOTSELECT, 0,0,32,32);
	ImageButton_SetIcon(GetDlgItem(hDlg, IDC_NGSLOT2_SELECT), IDI_NGSLOTSELECT, 0,0,32,32);
	ImageButton_SetIcon(GetDlgItem(hDlg, IDC_NGSLOT3_SELECT), IDI_NGSLOTSELECT, 0,0,32,32);
	ImageButton_SetIcon(GetDlgItem(hDlg, IDC_NGSLOT4_SELECT), IDI_NGSLOTSELECT, 0,0,32,32);
	ImageButton_SetIcon(GetDlgItem(hDlg, IDC_NGSLOT5_SELECT), IDI_NGSLOTSELECT, 0,0,32,32);
	ImageButton_SetIcon(GetDlgItem(hDlg, IDC_NGSLOT6_SELECT), IDI_NGSLOTSELECT, 0,0,32,32);

	HICON hIcon = LoadIcon(hAppInst, MAKEINTRESOURCE(IDI_APP));
	SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);	// Set the dialog icon.

	WndInMid(hDlg, hScrnWnd);
	SetFocus(hDlg);	 // Enable Esc=close

}

static BOOL CALLBACK DefInpProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch (Msg) {
		case WM_INITDIALOG: {
			InitContent(hDlg);		
//			IpsManagerInit();
			break;
		}
		
		case WM_COMMAND: {
			int wID = LOWORD(wParam);
			int Notify = HIWORD(wParam);
						
			if (Notify == BN_CLICKED) {
				switch (wID) {
					case IDOK: {
//						IpsOkay();
						nMultiSlotRet = 1; // Selection OK
						break;
					}
				
					case IDCANCEL: {
						nMultiSlotRet = 0; // Cancel
						SendMessage(hDlg, WM_CLOSE, 0, 0);
						return 0;
					}
					
					case IDC_NGSLOT1_SELECT:
					case IDC_NGSLOT2_SELECT:
					case IDC_NGSLOT3_SELECT:
					case IDC_NGSLOT4_SELECT:
					case IDC_NGSLOT5_SELECT:
					case IDC_NGSLOT6_SELECT: {
						int nActiveSlot = wID - IDC_NGSLOT1_SELECT;

						nNeoSlotDrvNum[nActiveSlot] = SelDialog(1, hDlg);

						extern bool bDialogCancel;

						if ((nNeoSlotDrvNum[nActiveSlot] != (unsigned int)-1) && (!bDialogCancel)) 
						{
							unsigned int nOldDrvSelect = nBurnDrvSelect;
							nBurnDrvSelect = nNeoSlotDrvNum[nActiveSlot];
							TCHAR szText[1024] = _T("");
							_stprintf(szText, _T("%s%s%s%s"), BurnDrvGetText(DRV_NAME), (BurnDrvGetText(DRV_PARENT)) ? _T(" (clone of ") : _T(""), (BurnDrvGetText(DRV_PARENT)) ? BurnDrvGetText(DRV_PARENT) : _T(""), (BurnDrvGetText(DRV_PARENT)) ? _T(")") : _T(""));
							SendMessage(GetDlgItem(hDlg, IDC_NGSLOT1_ROMNAME + nActiveSlot), WM_SETTEXT, (WPARAM)0, (LPARAM)szText);
							SendMessage(GetDlgItem(hDlg, IDC_NGSLOT1_ROMTITLE + nActiveSlot), WM_SETTEXT, (WPARAM)0, (LPARAM)BurnDrvGetText(DRV_FULLNAME));
							szText[0] = _T('\0');
							_stprintf(szText, _T("%s, %s"), BurnDrvGetText(DRV_MANUFACTURER), BurnDrvGetText(DRV_DATE));
							SendMessage(GetDlgItem(hDlg, IDC_NGSLOT1_ROMINFO + nActiveSlot), WM_SETTEXT, (WPARAM)0, (LPARAM)szText);
							nBurnDrvSelect = nOldDrvSelect;
						} else {
							SendMessage(GetDlgItem(hDlg, IDC_NGSLOT1_ROMNAME + nActiveSlot), WM_SETTEXT, (WPARAM)0, (LPARAM)_T("Empty"));
							SendDlgItemMessage(hDlg, IDC_NGSLOT1_IMAGE + nActiveSlot, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hPreview);
						}
						ScanTitlePreviews();
						break;
					}
				}
			}
			break;
		}

		case WM_CTLCOLORSTATIC: 
		{
			int idcText[6]		= { IDC_NGSLOT1_TEXT, IDC_NGSLOT2_TEXT, IDC_NGSLOT3_TEXT, IDC_NGSLOT4_TEXT, IDC_NGSLOT5_TEXT, IDC_NGSLOT6_TEXT };
			int idcRomname[6]	= { IDC_NGSLOT1_ROMNAME, IDC_NGSLOT2_ROMNAME, IDC_NGSLOT3_ROMNAME, IDC_NGSLOT4_ROMNAME, IDC_NGSLOT5_ROMNAME, IDC_NGSLOT6_ROMNAME };
			int idcRomtitle[6]	= { IDC_NGSLOT1_ROMTITLE, IDC_NGSLOT2_ROMTITLE, IDC_NGSLOT3_ROMTITLE, IDC_NGSLOT4_ROMTITLE, IDC_NGSLOT5_ROMTITLE, IDC_NGSLOT6_ROMTITLE };
			int idcRominfo[6]	= { IDC_NGSLOT1_ROMINFO, IDC_NGSLOT2_ROMINFO, IDC_NGSLOT3_ROMINFO, IDC_NGSLOT4_ROMINFO, IDC_NGSLOT5_ROMINFO, IDC_NGSLOT6_ROMINFO };

			for(int i = 0; i < 6; i++) {
				if ((HWND)lParam == GetDlgItem(hDlg, idcText[i]))		return (BOOL)hWhiteBGBrush;
				if ((HWND)lParam == GetDlgItem(hDlg, idcRomname[i]))	return (BOOL)hWhiteBGBrush;
				if ((HWND)lParam == GetDlgItem(hDlg, idcRomtitle[i]))	return (BOOL)hWhiteBGBrush;
				if ((HWND)lParam == GetDlgItem(hDlg, idcRominfo[i]))	return (BOOL)hWhiteBGBrush;
			}
			return 0;
		}

		case WM_CLOSE: {
			
			for(int i = 0; i < 6; i++) {
				if (hBmp[i]) {
					DeleteObject((HGDIOBJ)hBmp[i]);
					hBmp[i] = NULL;
				}
			}
			
			if(hWhiteBGBrush) {
				DeleteObject(hWhiteBGBrush);
			}

			EndDialog(hDlg, 0);
			break;
		}
	}

	return 0;
}

int NeogeoSlotSelectCreate(HWND hParentWND)
{
	hParent = hParentWND;
	InitCommonControls();
	FBADialogBox(hAppInst, MAKEINTRESOURCE(IDD_NGSLOTSELECT), hParent, DefInpProc);
	return nMultiSlotRet;
}
