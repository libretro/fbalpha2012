/* ----------------------------------------------------------------------- \\
 Favorite games module (by CaptainCPS-X)
 -------------------------------------------------------------------------
 *************************************************************************

 Last Update: Sep 5, 2008

 TODO:
  - Just clean some code a bit, but pretty much is done! even the structure
  for Favorite games is finished and Play Counter has been fully implemented! 
  
  ^____^

 *************************************************************************
 ------------------------------------------------------------------------- */
#include "burner.h"

#ifndef		LVM_SORTITEMSEX
#define		LVM_SORTITEMSEX          (LVM_FIRST + 81)
#define		ListView_SortItemsEx(hwndLV, _pfnCompare, _lPrm) \
				(BOOL)SNDMSG((hwndLV), LVM_SORTITEMSEX, (WPARAM)(LPARAM)(_lPrm), (LPARAM)(PFNLVCOMPARE)(_pfnCompare))
#endif
#ifndef		HDF_SORTUP
#define		HDF_SORTUP					0x0400
#endif
#ifndef		HDF_SORTDOWN
#define		HDF_SORTDOWN				0x0200
#endif

#define		ASCIIONLY					(1 << 19)
extern HWND hSelDlg;

#define		LV_ROMSET_COL				0
#define		LV_TITLE_COL				1
#define		LV_HARDWARE_COL				2
#define		LV_YEAR_COL					3
#define		LV_COMPANY_COL				4
#define		LV_MAXPLAYERS_COL			5
#define		LV_PLAYCOUNTER_COL			6
#define		LV_MAX_COLS					7

#define		MAX_GAMES					9999

#define		FAV_DRV_NAME				0
#define		FAV_DRV_TITLE				1
#define		FAV_DRV_HARDWARE			2
#define		FAV_DRV_YEAR				3
#define		FAV_DRV_COMPANY				4
#define		FAV_DRV_MAXPLAYERS			5
#define		FAV_DRV_NUMBER				6
#define		FAV_DRV_PLAYCOUNTER			7

HWND		hFavListView				= NULL;		// Favorite Gamelist Control Handle
HWND		hSelDlgTabControl			= NULL;		// [IDC_TAB3] GameList & Favorites Gamelist tabs control
bool		bFavSelected				= false;
int			nListViewColumnIndex		= 0;
bool		bListViewAscendingOrder		= true;

int			nFavPlayCounter				= 0;
int			nFavDrvCount				= 0;		// Total favorites currently on the list

// Favorite games structure array
struct		FAVORITESINFO Favorites[MAX_GAMES];

void SetFavoritesIcons() 
{
	if(nFavDrvCount == 0) {
		return;
	}

	if(nIconsSize == ICON_16x16) nIconsSizeXY = 16;
	if(nIconsSize == ICON_24x24) nIconsSizeXY = 24;
	if(nIconsSize == ICON_32x32) nIconsSizeXY = 32;

	HIMAGELIST hImageList = ImageList_Create(nIconsSizeXY, nIconsSizeXY, ILC_MASK | ILC_COLORDDB, 0, 0); 
	ListView_SetImageList(hFavListView, hImageList, LVSIL_SMALL);

	for(int nIndex = 0; nIndex < nFavDrvCount; nIndex++)
	{
		TCHAR szIcon[MAX_PATH];
		_stprintf(szIcon, _T("%s%s.ico"), szAppIconsPath, Favorites[nIndex].szDrvName);

		if((HICON)LoadImage(hAppInst, szIcon, IMAGE_ICON, nIconsSizeXY, nIconsSizeXY, LR_LOADFROMFILE)){
			ImageList_AddIcon(hImageList, (HICON)LoadImage(hAppInst, szIcon, IMAGE_ICON, nIconsSizeXY, nIconsSizeXY, LR_LOADFROMFILE));
		} 

		if(!(HICON)LoadImage(hAppInst, szIcon, IMAGE_ICON, nIconsSizeXY, nIconsSizeXY, LR_LOADFROMFILE)) 
		{
			unsigned int nTemp = nBurnDrvSelect;
			nBurnDrvSelect = Favorites[nIndex].nDrvNumber;

			if(BurnDrvGetText(DRV_PARENT)){
				_stprintf(szIcon, _T("%s%s.ico"), szAppIconsPath, BurnDrvGetText(DRV_PARENT));
				ImageList_AddIcon(hImageList, (HICON)LoadImage(hAppInst, szIcon, IMAGE_ICON, nIconsSizeXY, nIconsSizeXY, LR_LOADFROMFILE));
			}
			nBurnDrvSelect = nTemp;
		}

		if(!(HICON)LoadImage(hAppInst, szIcon, IMAGE_ICON, nIconsSizeXY, nIconsSizeXY, LR_LOADFROMFILE)) {
			ImageList_AddIcon(hImageList, LoadIcon(hAppInst, MAKEINTRESOURCE(IDI_STAR)) );
		}
	}
}

void InsertTabs() {
	HIMAGELIST hImageList1 = ImageList_Create(16, 16, ILC_MASK | ILC_COLORDDB, 3, 0);
	TabCtrl_SetImageList(hSelDlgTabControl, hImageList1);

	HICON hIcon1 = LoadIcon(hAppInst, MAKEINTRESOURCE(IDI_JOY));
	HICON hIcon2 = LoadIcon(hAppInst, MAKEINTRESOURCE(IDI_STAR));
	HICON hIcon3 = LoadIcon(hAppInst, MAKEINTRESOURCE(IDI_COG));

	ImageList_AddIcon(hImageList1, hIcon1);
	ImageList_AddIcon(hImageList1, hIcon2);
	ImageList_AddIcon(hImageList1, hIcon3);

	TC_ITEMHEADER TCI;
	TCI.mask = TCIF_TEXT | TCIF_IMAGE;

	TCI.pszText = _T("Games List");
	TCI.iImage = 0;
	SendMessage(hSelDlgTabControl, TCM_INSERTITEM, (WPARAM) 0, (LPARAM) &TCI);

	TCI.pszText = _T("Favorite Games List");
	TCI.iImage = 1;
	SendMessage(hSelDlgTabControl, TCM_INSERTITEM, (WPARAM) 1, (LPARAM) &TCI);

	TCI.pszText = _T("Options");
	TCI.iImage = 2;
	SendMessage(hSelDlgTabControl, TCM_INSERTITEM, (WPARAM) 2, (LPARAM) &TCI);
}

// Favorites Games List Compare function
int CALLBACK ListView_CompareFunc(LPARAM iIndex1, LPARAM iIndex2, LPARAM param)
{
	HWND hLView = (HWND)param;	// this is the ListView calling the compare function

	TCHAR szItemText1[256];
	TCHAR szItemText2[256];

	ListView_GetItemText(hLView, iIndex1, nListViewColumnIndex,  szItemText1,  256); // 1st Row
	ListView_GetItemText(hLView, iIndex2, nListViewColumnIndex,  szItemText2,  256); // 2nd Row

	int nCmpRet1 = _tcscmp(_tcslwr(szItemText1), _tcslwr(szItemText2));
	int nCmpRet2 = _tcscmp(_tcslwr(szItemText2), _tcslwr(szItemText1));

	return bListViewAscendingOrder ? nCmpRet1 : nCmpRet2;
}

// Check the version of 'comctl32.dll' to see if is the v6
BOOL IsCommCtrlVersion6()
{
    static BOOL isCommCtrlVersion6 = -1;

	if (isCommCtrlVersion6 != -1) return isCommCtrlVersion6;
    
    isCommCtrlVersion6 = FALSE; //The default value

#if defined (_UNICODE)
    HINSTANCE commCtrlDll = LoadLibrary(_T("comctl32.dll"));
#else
	HINSTANCE commCtrlDll = LoadLibrary("comctl32.dll");
#endif

	if(commCtrlDll) {
        DLLGETVERSIONPROC pDllGetVersion;
        pDllGetVersion = (DLLGETVERSIONPROC)GetProcAddress(commCtrlDll, "DllGetVersion");
        
        if (pDllGetVersion) {
            DLLVERSIONINFO dvi;
            dvi.cbSize = sizeof(DLLVERSIONINFO);
            (*pDllGetVersion)(&dvi);
            
            isCommCtrlVersion6 = (dvi.dwMajorVersion == 6);
        }        
        FreeLibrary(commCtrlDll);
    }    
    return isCommCtrlVersion6;
}

// Set the UP / DOWN arrow image when a column header is clicked in the Favorite Games List
void ListView_SetHeaderSortImage(HWND hLView, int nColIndex, bool bListViewAscending)
{
    HWND hListViewHeader			= ListView_GetHeader(hLView);
    BOOL isCommonControlVersion6	= IsCommCtrlVersion6();    
    int nColCount					= Header_GetItemCount(hListViewHeader);

    for (int nIndex = 0; nIndex < nColCount; nIndex++)
    {
        HDITEM hi;
        
        // I only need to retrieve the format if i'm on windows xp. If not, then i need to retrieve the bitmap also.
        hi.mask = HDI_FORMAT | (isCommonControlVersion6 ? 0 : HDI_BITMAP);
        
        Header_GetItem(hListViewHeader, nIndex, &hi);
        
        // Set sort image to this column
        if(nIndex == nColIndex)
        {
            // Windows xp has a easier way to show the sort order in the header: i just have to set a flag and windows will do the drawing. No windows xp, no easy way.
            if (isCommonControlVersion6) {
                hi.fmt &= ~(HDF_SORTDOWN|HDF_SORTUP);
                hi.fmt |= bListViewAscending ? HDF_SORTUP : HDF_SORTDOWN;
            } else {
                UINT bitmapID = bListViewAscending ? IDB_UPARROW : IDB_DOWNARROW;
                
				// If there's a bitmap, let's delete it.
                if (hi.hbm) DeleteObject(hi.hbm);

                hi.fmt |= HDF_BITMAP|HDF_BITMAP_ON_RIGHT;
                hi.hbm = (HBITMAP)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(bitmapID), IMAGE_BITMAP, 0,0, LR_LOADMAP3DCOLORS);
            }
        } else {
			// Remove sort image (if exists) from other columns.
			if (isCommonControlVersion6) {
                hi.fmt &= ~(HDF_SORTDOWN|HDF_SORTUP);
			} else {
				// If there's a bitmap, let's delete it.
                if (hi.hbm) DeleteObject(hi.hbm);
                
                // Remove flags that tell windows to look for a bitmap.
                hi.mask &= ~HDI_BITMAP;
                hi.fmt &= ~(HDF_BITMAP|HDF_BITMAP_ON_RIGHT);
            }
        }        
        Header_SetItem(hListViewHeader, nIndex, &hi);
    }
}

// Set / Insert items for each row of the Favorite Games List
void SetFavListRow(int nIndex, TCHAR* szRomname, TCHAR* szTitle, TCHAR* szHardware, TCHAR* szYear, TCHAR* szCompany, TCHAR* szMaxPlayers, TCHAR* szPlayCounter) 
{
	LVITEM LvItem;
	memset(&LvItem,0,sizeof(LvItem));	// Zero struct's Members
	LvItem.mask = LVIF_TEXT | LVIF_IMAGE;
	LvItem.cchTextMax = 256;

	for(int nColumn = 0; nColumn < LV_MAX_COLS; nColumn++) 
	{		
		LvItem.iSubItem = nColumn;
		LvItem.iImage = nIndex;
		switch(nColumn) 
		{
			case LV_TITLE_COL:			LvItem.pszText = szTitle;		break;
			case LV_HARDWARE_COL:		LvItem.pszText = szHardware;	break;
			case LV_YEAR_COL:			LvItem.pszText = szYear;		break;
			case LV_COMPANY_COL:		LvItem.pszText = szCompany;		break;
			case LV_MAXPLAYERS_COL:		LvItem.pszText = szMaxPlayers;	break;
			case LV_PLAYCOUNTER_COL:	LvItem.pszText = szPlayCounter;	break;
		}
		if(nColumn == LV_ROMSET_COL) 
		{
			LvItem.iItem		= nIndex;
			LvItem.iImage		= nIndex;
			LvItem.pszText		= szRomname;
			SendMessage(hFavListView, LVM_INSERTITEM, 0, (LPARAM)&LvItem);
		} else {
			SendMessage(hFavListView, LVM_SETITEM, 0, (LPARAM)&LvItem);
		}
	}
}

void FavDrvSetContent(int nItem, int nIndex, TCHAR* pszText, int nValue) {
	switch(nItem) {
		case FAV_DRV_NAME:			_stprintf(Favorites[nIndex].szDrvName,		pszText);				break;
		case FAV_DRV_TITLE:			_stprintf(Favorites[nIndex].szDrvTitle,		pszText);				break;
		case FAV_DRV_HARDWARE:		_stprintf(Favorites[nIndex].szDrvHardware,	pszText);				break;
		case FAV_DRV_YEAR:			_stprintf(Favorites[nIndex].szDrvYear,		pszText);				break;
		case FAV_DRV_COMPANY:		_stprintf(Favorites[nIndex].szDrvCompany,	pszText);				break;
		case FAV_DRV_MAXPLAYERS:	_stprintf(Favorites[nIndex].szDrvMaxPlayers, _T("%i"), nValue);		break;
		case FAV_DRV_PLAYCOUNTER:
		{
			if(nValue != -2) {
				Favorites[nIndex].nDrvPlayCount = nValue;
			} else {
				Favorites[nIndex].nDrvPlayCount = _tcstol(pszText, NULL, 0);
			}
			break;
		}
		case FAV_DRV_NUMBER: Favorites[nIndex].nDrvNumber = nValue;	break;
	}
}

TCHAR szDrvPlayCount[5];

TCHAR* FavDrvGetContent(int nContent, int nBurnDrvNumber) 
{
	int nIndex = 0;
	for(int nDrvCheck = 0; nDrvCheck < nFavDrvCount; nDrvCheck++) {
		nIndex = nDrvCheck;	
		if(Favorites[nDrvCheck].nDrvNumber == (unsigned int)nBurnDrvNumber) break;
	}

	switch(nContent) 
	{
		case FAV_DRV_NAME:			return Favorites[nIndex].szDrvName;
		case FAV_DRV_TITLE:			return Favorites[nIndex].szDrvTitle;
		case FAV_DRV_HARDWARE:		return Favorites[nIndex].szDrvHardware;
		case FAV_DRV_YEAR:			return Favorites[nIndex].szDrvYear;
		case FAV_DRV_COMPANY:		return Favorites[nIndex].szDrvCompany;
		case FAV_DRV_MAXPLAYERS:	return Favorites[nIndex].szDrvMaxPlayers;
		case FAV_DRV_PLAYCOUNTER:
			_stprintf(szDrvPlayCount, _T("%i"),Favorites[nIndex].nDrvPlayCount);
			return szDrvPlayCount;
	}
	return _T("");
}

int FavDrvGetValue(int nContent, int nBurnDrvNumber) 
{
	int nIndex = 0;
	for(int nDrvCheck = 0; nDrvCheck < nFavDrvCount; nDrvCheck++) {
		nIndex = nDrvCheck;	
		if(Favorites[nDrvCheck].nDrvNumber == (unsigned int)nBurnDrvNumber) break;
	}
	switch(nContent) {
		case FAV_DRV_PLAYCOUNTER:	return Favorites[nIndex].nDrvPlayCount;
		case FAV_DRV_NUMBER:		return Favorites[nIndex].nDrvNumber;	
	}
	return 0;
}

void SetFavListViewContent() {
	for(int nRow = 0; nRow < nFavDrvCount; nRow++) {
		SetFavListRow(nRow, Favorites[nRow].szDrvName, Favorites[nRow].szDrvTitle, Favorites[nRow].szDrvHardware, Favorites[nRow].szDrvYear, Favorites[nRow].szDrvCompany, Favorites[nRow].szDrvMaxPlayers, FavDrvGetContent(FAV_DRV_PLAYCOUNTER, Favorites[nRow].nDrvNumber));
	}
}

// Favorite Game List parsing module
int ParseFavListDat() 
{
	FILE *fp = _tfopen(_T("config\\favorites.dat"), _T("r"));	// not 'rb' anymore, there's no need
	if(!fp) return 1;								// failed to open file

	int x					= 0;
	int nLineNum			= 0;

	TCHAR  szLine[1000];
	TCHAR* pch				= NULL;
	TCHAR* str				= NULL;
	TCHAR* pszDrvName		= NULL;
	TCHAR* pszPlayCounter	= NULL;

	while (1)
	{	
		if (!_fgetts(szLine, 1000, fp)) break;						// If there are no more lines, break loop
		if (!_tcsncmp (_T("[Favorites]"), szLine, 11)) continue;	// Not parsing '[Favorites]' line, so continue with the other lines

		// Split the current line to send each value to the proper string variables
		str = szLine;
		pch = _tcstok(str, _T(";"));

		while(pch != NULL)
		{
			if(x == 0) pszDrvName		= pch;	// Driver name (Ex. mvsc)
			if(x == 1) pszPlayCounter	= pch;	// Play Counter

			pch							= _tcstok(NULL, _T(";"));
			x++;
		}
		x = 0; // reset this to zero for next line

		// Get the favorite game info from FBA itself not from text strings, this is the proper way
		// ---------------------------------------------------------------------------------------------------------
		int nBurnDrvSelectOld = nBurnDrvSelect;

		for (unsigned int nDrvCheck = 0; nDrvCheck < nBurnDrvCount; nDrvCheck++)
		{
			nBurnDrvSelect = nDrvCheck;
			if (!_tcscmp(BurnDrvGetText(DRV_NAME), pszDrvName)) break;
		}

		FavDrvSetContent(FAV_DRV_NAME, nLineNum, BurnDrvGetText(DRV_NAME), -2);
#if defined (_UNICODE)
		FavDrvSetContent(FAV_DRV_TITLE, nLineNum, BurnDrvGetText(DRV_FULLNAME), -2);					// Unicode Game Title
#else
		FavDrvSetContent(FAV_DRV_TITLE, nLineNum, BurnDrvGetText(DRV_ASCIIONLY | DRV_FULLNAME), -2);	// ASCII Game Title
#endif
		FavDrvSetContent(FAV_DRV_HARDWARE, nLineNum, BurnDrvGetText(DRV_SYSTEM), -2);
		FavDrvSetContent(FAV_DRV_YEAR, nLineNum, BurnDrvGetText(DRV_DATE), -2);
		FavDrvSetContent(FAV_DRV_COMPANY, nLineNum, BurnDrvGetText(DRV_MANUFACTURER), -2);
		FavDrvSetContent(FAV_DRV_MAXPLAYERS, nLineNum, NULL, BurnDrvGetMaxPlayers());
		FavDrvSetContent(FAV_DRV_PLAYCOUNTER, nLineNum, pszPlayCounter, -2);
		FavDrvSetContent(FAV_DRV_NUMBER, nLineNum, NULL, nBurnDrvSelect);

		nBurnDrvSelect = nBurnDrvSelectOld;
		// ---------------------------------------------------------------------------------------------------------

		nLineNum++; // next line
	}

	nFavDrvCount = nLineNum;

	fclose(fp);
	
	return 1;
}

// Refresh the Favorite Games List
void RefreshFavGameList() 
{
	ListView_DeleteAllItems(hFavListView);											// Clean the Favorite Games List
	ParseFavListDat();																// Parse the 'favorites.dat'
	SetFavListViewContent();
	ListView_SortItemsEx(hFavListView, ListView_CompareFunc, (LPARAM)hFavListView);	// Sort Favorite Games List (macro)
}

// Initiate Favorite Games List and add columns
void InitFavGameList() 
{
    ListView_SetExtendedListViewStyle(hFavListView, LVS_EX_FULLROWSELECT);
 
	UINT columnMask = LVCF_TEXT | LVCF_FMT | LVCF_SUBITEM | LVCF_WIDTH;
    LVCOLUMN lc[] = {
        { columnMask, 0,	100,	_T("Romset"),		0, LV_ROMSET_COL		,0,0 },		// 0
		{ columnMask, 0,	350,	_T("Game Title"),	0, LV_TITLE_COL			,0,0 },		// 1
        { columnMask, 0,	130,	_T("Hardware"),		0, LV_HARDWARE_COL		,0,0 },		// 2
		{ columnMask, 0,	 80,	_T("Year"),			0, LV_YEAR_COL			,0,0 },		// 3
		{ columnMask, 0,	130,	_T("Manufacturer"),	0, LV_COMPANY_COL		,0,0 },		// 4
		{ columnMask, 0,	100,	_T("Max Players"),	0, LV_MAXPLAYERS_COL	,0,0 },		// 5
		{ columnMask, 0,	110,	_T("Play Count"),	0, LV_PLAYCOUNTER_COL	,0,0 },		// 6
    };

	// Insert all columns to the Favorite Games List (macro)
	for(int nColumn = 0; nColumn < LV_MAX_COLS; nColumn++) {
		ListView_InsertColumn(hFavListView, nColumn, &lc[nColumn]);
	}

	// Parse the 'favorites.dat' 
	ParseFavListDat();
	SetFavListViewContent();
}

// Save the favorites.dat using the structure array
int SaveFavList()
{	
	// Write to the favorites.dat 
	FILE* fp = _tfopen(_T("config\\favorites.dat"), _T("w"));
	if (!fp) return 0;	// failed
	
	_ftprintf(fp, _T("[Favorites]\n"));

	if(nFavDrvCount == 0) {
		fclose(fp);
		return 0;
	}

	for (int nIndex = 0; nIndex < nFavDrvCount; nIndex++) {
		_ftprintf(fp, _T("%s;%s;\n"), FavDrvGetContent(FAV_DRV_NAME, Favorites[nIndex].nDrvNumber), FavDrvGetContent(FAV_DRV_PLAYCOUNTER, Favorites[nIndex].nDrvNumber));
	}
	if (fp) fclose(fp);
	return 1;
}

// Save the favorites.dat using altername method when Adding / Removing favorites
int SaveFavListAlt()
{   
   int nNumFavs = ListView_GetItemCount(hFavListView);
   
   // Write to the favorites.dat
   FILE* fp = _tfopen(_T("config\\favorites.dat"), _T("w"));
   if (!fp) return 0;   // failed
   
   _ftprintf(fp, _T("[Favorites]\n"));

   if(nNumFavs == 0) {
      fclose(fp);
      return 0;
   }

   LVITEM lvi;
   lvi.cchTextMax = 32;

   for (int nIndex = 0; nIndex < nNumFavs; nIndex++) {

      TCHAR szRomset[32];
      TCHAR szPlayCounter[32];

      // DrvName
      lvi.iSubItem = LV_ROMSET_COL;
      lvi.pszText = szRomset;
      SendMessage(hFavListView, LVM_GETITEMTEXT, (WPARAM)nIndex, (LPARAM)&lvi);
      //ListView_GetItemText(hFavListView, nIndex, LV_ROMSET_COL,  szRomset, 32);

      // PlayCounter
      lvi.iSubItem = LV_PLAYCOUNTER_COL;
      lvi.pszText = szPlayCounter;
      SendMessage(hFavListView, LVM_GETITEMTEXT, (WPARAM)nIndex, (LPARAM)&lvi);
      //ListView_GetItemText(hFavListView, nIndex, LV_PLAYCOUNTER_COL, szPlayCounter, 32);

      _ftprintf(fp, _T("%s;%s;\n"), szRomset, szPlayCounter);
   }
   if (fp) fclose(fp);
   return 1;
}

void UpdatePlayCounter(int nBurnDrvNumber)
{
	int nNewValue = FavDrvGetValue(FAV_DRV_PLAYCOUNTER, nBurnDrvNumber);
	nNewValue++;

	int nIndex = 0;
	for(int nDrvCheck = 0; nDrvCheck < nFavDrvCount; nDrvCheck++) {
		nIndex = nDrvCheck;	
		if(Favorites[nDrvCheck].nDrvNumber == (unsigned int)nBurnDrvNumber) break;
	}

	FavDrvSetContent(FAV_DRV_PLAYCOUNTER, nIndex, NULL, nNewValue);

	SaveFavList();
	ParseFavListDat();
}

// Add a game to Favorites
void AddToFavorites() 
{
	int nIndex = ListView_GetItemCount(hFavListView);						// index of the new favorite listview item

	TCHAR* ItemRomname		= BurnDrvGetText(DRV_NAME);						// Romset Name
#if defined (_UNICODE)
	TCHAR* ItemTitle		= BurnDrvGetText(DRV_FULLNAME);					// Unicode Game Title
#else
	TCHAR* ItemTitle		= BurnDrvGetText(DRV_ASCIIONLY | DRV_FULLNAME);	// ASCII Game Title
#endif
	TCHAR* ItemHardware		= BurnDrvGetText(DRV_SYSTEM);					// Game Hardware / System
	TCHAR* ItemYear			= BurnDrvGetText(DRV_DATE);						// Year
	TCHAR* ItemCompany		= BurnDrvGetText(DRV_MANUFACTURER);				// Manufacturer / Company
	
	TCHAR szItemMaxPlayers[5];
	_stprintf(szItemMaxPlayers, _T("%i"), BurnDrvGetMaxPlayers());			// Max Players
	TCHAR* ItemMaxPlayers	= szItemMaxPlayers;

	TCHAR szPlayCounter[5];
	_stprintf(szPlayCounter,	_T("%i"), 0);								// Play Counter (Zero because there isn't another counter in FBA)
	TCHAR* ItemPlayCounter	= szPlayCounter;

	LVITEM LvItem;
	memset(&LvItem, 0, sizeof(LvItem));
	LvItem.mask = LVIF_TEXT;
	LvItem.cchTextMax = 256;
	LvItem.iItem = nIndex;

	// Add Romname, Title and Hardware of the selected game to the Favorites List
	for (int nColumn = 0; nColumn < LV_MAX_COLS; nColumn++) {
		LvItem.iSubItem = nColumn;
		switch (nColumn) {
			case 1: LvItem.pszText = ItemTitle;			break;
			case 2: LvItem.pszText = ItemHardware;		break;
			case 3: LvItem.pszText = ItemYear;			break;
			case 4: LvItem.pszText = ItemCompany;		break;
			case 5: LvItem.pszText = ItemMaxPlayers;	break;
			case 6: LvItem.pszText = ItemPlayCounter;	break;
		}
		if(nColumn == 0) {
			LvItem.pszText = ItemRomname; 
			SendMessage(hFavListView, LVM_INSERTITEM,0,(LPARAM)&LvItem);
			SendMessage(hFavListView, LVM_SETITEM, 0, (LPARAM)&LvItem);
		} else {
			SendMessage(hFavListView, LVM_SETITEM, 0, (LPARAM)&LvItem);
		}
	}					
	SaveFavListAlt();		// Save the Favorite Games List
	RefreshFavGameList();	// Refresh Favorite Games List	
}

// Remove a game from favorites
void RemoveFromFavorites() {
	int iSel = SendMessage(hFavListView, LVM_GETNEXTITEM, ~0U, LVNI_FOCUSED);
	ListView_DeleteItem(hFavListView, iSel);
	SaveFavListAlt();		// Save the Favorite Games List
	RefreshFavGameList();	// Refresh Favorite Games List

	if(ListView_GetItemCount(hFavListView) == 0) {
		return;
	} else {
		SetFocus(hFavListView);
		ListView_SetItemState(hFavListView, 0, LVIS_SELECTED | LVIS_FOCUSED, 0x000F);
	}
}
