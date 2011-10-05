/* -----------------------------------------------------------------------------------\\
 Advanced Module with various functions to import MAME support packages for use in FBA
 (by CaptainCPS-X)
 -----------------------------------------------------------------------------------

 Description:

	This module will allow the users to import the files FBA needs from
	any MAME packages. For example it can import the icons from a MAME icons
	package, good news is that this will get what FBA needs only, nothing else!.

 TODO:

	!!!	- Display file size on the log
	!!!	- Display processed data size and total data size in a label
	!!!	- Calculate progress percent using data size info

	X	- If the Icons feature is enabled when importing icons, FBA should load them after closing the import dlg, so 
		  they can be seen without any problem, and without restarting the emulator.

		- Add all displayed strings to 'string.rc' and 'resource_string.h' to allow them to be translated later

	X	- Add module to check for required space for import.
	X	- Add module to check for available space in directories.
	X	- Remove the 'Big ass' macro and do a nice and proffesional function
	X	- Let the user select the MAME [snaps, titles, cabinets, etc] directory
	X	- Let the user select the import directory

	X	- Add module to check what disk size format should be displayed to the user...
			- Bytes ranges:			
				[0					-> 1,023				]	Bytes
				[1,024				-> 1,048,575			]	KiloBytes
				[1,048,576			-> 1,073,741,823		]	MegaBytes
				[1,073,741,824		-> 1,099,511,627,775	]	GigaBytes
				[1,099,511,627,776	-> 1,125,899,906,843,647]	TerraBytes
		
		- Add the possibility to specify other network locations manually (this should disable the browse buttons)

	X	- When MAME import directory is verified , if nothing is found prompt the user to select another directory.

	X	- Let the user select options to make TXT logs or not.
	X	- Let the user select options to make HTML logs or not. (Have to make the module too >__>)

		- Add a new module to import Genesis support files.
	???	- Remember import directories (Have to check how many variables it needs)

	X	- Edit the UI and Add a label for to show the % of the progress bar.

	WIP	- Add a module to import ROMs or move them to a specified directory
	WIP	- Add a module to import Tracklist files

 TODO!: - CLEAN EVERYTHING! xD

 TODO LATER:

		- add ZIP support for imports
		- add module to import all support files 
		(this will require a new dialog so the user can specify origin dirs and import dirs)
		(regular browse buttons will be disabled when this option is selected)

//------------------------------------------------------------------------------------*/
#include "burner.h"
#include <shellapi.h>
#include <shlobj.h>
//#include <winable.h>

static HANDLE			hProcessThread			= NULL;
static DWORD			dwProcessThreadID		= 0;

static HWND				hImportProcessDlg		= NULL;
static HWND				hImportDlg				= NULL;
//static HWND				hImportLogListBox		= NULL;
static HWND				hImportLabel			= NULL;
static HWND				hImportEdit1			= NULL;
static HWND				hImportEdit2			= NULL;
static HWND				hImportBrowse1			= NULL;
static HWND				hImportBrowse2			= NULL;
static HWND				hImportCheckboxes[6];
static HWND				hImportListView			= NULL;
static HWND				hImportProgressPercent	= NULL;
static HMENU			hSysMenu				= NULL;

static TCHAR			szSupportType[MAX_PATH];

static bool				bMAMESupportDir			= false;
static bool				bImportDir				= false;

static int				nImportDrvCount			= 0;		// Loop related [ needed for final scrolling ]
static int				nImportLogCount			= 0;		// OK
static int				nImportLogCountFail		= 0;		// Copy Fail
static int				nImportLogCountNotFound = 0;		// Not found
static int				nImportLogCountExcluded	= 0;		// Not Imported
static unsigned int		nBarCount				= 0;
static int				nImportOption			= 0;

static bool				bIsMegaDrive			= false;
static bool				bImportFinished			= false;
static bool				bProcessing				= false;

bool					bWriteTXTLog			= false;
bool					bWriteHTMLLog			= false;
bool					bAutoCloseDlg			= false;
bool					bAutoOpenLogs			= false;

/* Not used now ----------------------------------------*/
bool					bRememberDirs			= false;	// To remeber directories (this require extra code at dialog init to check space and stuff)
bool					bNetworkPaths			= false;	// To allow the user to manually write the path of the network directory
/* -----------------------------------------------------*/

bool					bEditPath[2]			= {false, false};

int						ImporterDlgCreate(int);

#define					IMPORT_PREVIEWS			0
#define					IMPORT_TITLES			1
#define					IMPORT_FLYERS			2
#define					IMPORT_CABINETS			3
#define					IMPORT_MARQUEES			4
#define					IMPORT_CPANELS			5
#define					IMPORT_PCBS				6
#define					IMPORT_ICONS			7

#define					IMPORT_ROMS				8
#define					IMPORT_TRACKLISTS		9

#define					IS_CLONE				BurnDrvGetText(DRV_PARENT)
#define					IS_MEGADRIVE			((BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK)==HARDWARE_SEGA_MEGADRIVE)

#define CALCKILOBYTE(lpByteSize)		\
	lpByteSize / 1024LL												// lpByteSize / 1,024

#define CALCMEGABYTE(lpByteSize)		\
	(float)(lpByteSize) / (1024.0F * 1024.0F)						// lpByteSize / 1,048,576

#define CALCGIGABYTE(lpByteSize)		\
	(float)(lpByteSize) / (1024.0F * 1024.0F * 1024.0F)				// lpByteSize / 1,073,741,824

#define CALCTERABYTE(lpByteSize)		\
	(float)(lpByteSize) / (1024.0F * 1024.0F * 1024.0F * 1024.0F)	// lpByteSize / 1,099,511,627,776

__int64 lpRequiredDiskSpace					= 0LL;
__int64 lpAvailableDiskSpace				= 0LL;
__int64 lpRemainingDiskSpace				= 0LL;
unsigned __int64 lpTotalDiskSpace			= 0LL; 
unsigned __int64 lpUsedDiskSpace			= 0LL;

static BOOL CALLBACK DummyProcStart(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam);
static DWORD WINAPI DoProcess(LPVOID);
int ImportProcessStart();

static void DynamicSelectionScroll(HWND hControl, int nIndex)
{
	// Dynamic ListView scrolling
	POINT selPoint;
	RECT selRect;
	ListView_GetItemPosition(hControl, nIndex, &selPoint);
	ListView_GetItemRect(hControl, nIndex, &selRect, LVIR_BOUNDS);
	ListView_Scroll(hControl, selPoint.x, selPoint.y - (selRect.bottom - selRect.top));
}

static void SetListViewColumns()
{
	LV_COLUMN LvCol;
	ListView_SetExtendedListViewStyle(hImportListView, LVS_EX_FULLROWSELECT);

	memset(&LvCol, 0, sizeof(LvCol));
	LvCol.mask		= LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;

	LvCol.cx		= 60;
	LvCol.pszText	= _T("Status");
	SendMessage(hImportListView, LVM_INSERTCOLUMN , 0, (LPARAM)&LvCol);

	LvCol.cx		= 400;
	LvCol.pszText	= _T("File");
	SendMessage(hImportListView, LVM_INSERTCOLUMN , 1, (LPARAM)&LvCol);

	LvCol.cx		= 130;
	LvCol.pszText	= _T("Size");
	SendMessage(hImportListView, LVM_INSERTCOLUMN , 2, (LPARAM)&LvCol);

	// TODO: Add 'Parent or Clone' column

}

static void InsertLVItem(int nFileStatus, TCHAR szFile[MAX_PATH], TCHAR szSize[1024], int /*nParentOrClone*/) {
	
	int nIndex = ListView_GetItemCount(hImportListView);

	TCHAR szStatus[MAX_PATH];
	switch(nFileStatus) {
		// OK
		case 0: 
			_stprintf(szStatus, _T("OK"));
			break;
		// Copy error
		case 1:
			_stprintf(szStatus, _T("Copy error"));
			break;
		// Not found
		case 2:
			_stprintf(szStatus, _T("Not found"));
			break;
	}

	LV_ITEM LvItem;
	memset(&LvItem, 0, sizeof(LvItem));
	LvItem.mask			= LVIF_TEXT | LVIF_IMAGE;
	LvItem.cchTextMax	= 256;

	LvItem.iItem		= nIndex;										// row / item index
	LvItem.iSubItem		= 0;
	LvItem.pszText		= szStatus;
	LvItem.iImage		= nFileStatus;									// Status Icon

	ListView_InsertItem(hImportListView, &LvItem);						// COL - Status [Insert new row]
	ListView_SetItemText(hImportListView, nIndex, 1, szFile);			// COL - File [Set into new row]
	ListView_SetItemText(hImportListView, nIndex, 2, szSize);			// COL - Size [Set into new row]
	// COL - Parent or Clone
}

int CheckByteSizeFormat(__int64 lpByteSize) 
{
	int nSizeFormat		= 0;
	bool bNegative		= false;

	if(lpByteSize < 0) {
		bNegative		= true;
	}

	if(bNegative == false) {
		// Handle positive values here
		unsigned __int64 nLimit		= 1024LL;
		unsigned __int64 nKiloByte	= nLimit;
		unsigned __int64 nMegaByte	= nKiloByte * nLimit;
		unsigned __int64 nGigaByte	= nMegaByte * nLimit;
		unsigned __int64 nTeraByte	= nGigaByte * nLimit;
		unsigned __int64 nPetaByte	= nTeraByte * nLimit;

		if(
		  ((unsigned __int64)lpByteSize == 0LL) || 
		  ((unsigned __int64)lpByteSize >= 1LL			&& (unsigned __int64)lpByteSize <= (nKiloByte  - 1LL)))	nSizeFormat = 0;	// Bytes range
		if((unsigned __int64)lpByteSize >= nKiloByte	&& (unsigned __int64)lpByteSize <= (nMegaByte  - 1LL))	nSizeFormat = 1;	// KB range
		if((unsigned __int64)lpByteSize >= nMegaByte	&& (unsigned __int64)lpByteSize <= (nGigaByte  - 1LL))	nSizeFormat = 2;	// MB range
		if((unsigned __int64)lpByteSize >= nGigaByte	&& (unsigned __int64)lpByteSize <= (nTeraByte  - 1LL))	nSizeFormat = 3;	// GB range
		if((unsigned __int64)lpByteSize >= nTeraByte	&& (unsigned __int64)lpByteSize <= (nPetaByte  - 1LL))	nSizeFormat = 4;	// TB range
	} else {

		// Handle negative values 
		// (Tested & Works OK!)
		__int64 nLimit		=  1024LL;
		__int64 nKiloByte	= -1024LL;						//				= (-) 
		__int64 nMegaByte	= nKiloByte * nLimit;			// (-) * (+)	= (-)
		__int64 nGigaByte	= nMegaByte * nLimit;			// (-) * (+)	= (-)
		__int64 nTeraByte	= nGigaByte * nLimit;			// (-) * (+)	= (-)
		__int64 nPetaByte	= nTeraByte * nLimit;			// (-) * (+)	= (-)

		if(lpByteSize <= -1			&& lpByteSize >= (nKiloByte  + 1LL))	nSizeFormat = 0;	// Bytes range (-)
		if(lpByteSize <= nKiloByte	&& lpByteSize >= (nMegaByte  + 1LL))	nSizeFormat = 1;	// KB range (-)
		if(lpByteSize <= nMegaByte	&& lpByteSize >= (nGigaByte	 + 1LL))	nSizeFormat = 2;	// MB range (-)
		if(lpByteSize <= nGigaByte	&& lpByteSize >= (nTeraByte  + 1LL))	nSizeFormat = 3;	// GB range (-)
		if(lpByteSize <= nTeraByte	&& lpByteSize >= (nPetaByte  + 1LL))	nSizeFormat = 4;	// TB range (-)
	}

	return nSizeFormat;
}

TCHAR* GetProperSizeFormat(int nSizeFormat, __int64 lpByteSize) {
	
	TCHAR szSizeInfo[MAX_PATH];

	if(nSizeFormat == 0) {
		_uInt64ToCommaFormattedTCHAR(szSizeInfo, lpByteSize);
		_stprintf(szSizeInfo, _T("%s Bytes"), szSizeInfo);
	}
	if(nSizeFormat == 1) {
		_uInt64ToCommaFormattedTCHAR(szSizeInfo, CALCKILOBYTE(lpByteSize));
		_stprintf(szSizeInfo, _T("%s KB"), szSizeInfo);
	}
	if(nSizeFormat == 2) _stprintf(szSizeInfo, _T("%3.1f MB"), CALCMEGABYTE(lpByteSize));
	if(nSizeFormat == 3) _stprintf(szSizeInfo, _T("%3.1f GB"), CALCGIGABYTE(lpByteSize));
	if(nSizeFormat == 4) _stprintf(szSizeInfo, _T("%3.1f TB"), CALCTERABYTE(lpByteSize));

	TCHAR* pszBuffer = szSizeInfo;
	
	return pszBuffer;
}

#define _GetProperSizeText(szOUT, nSizeFormat, lpByteSize)	\
	_stprintf(szOUT, _T("%s"), GetProperSizeFormat(nSizeFormat, lpByteSize));

char* WriteToLog(char* utf8Buffer, TCHAR szText[MAX_PATH], FILE* fpLog) {

#if defined (UNICODE)
	utf8Buffer = utf8_from_wstring(szText);
#else
	utf8Buffer = utf8_from_astring(szText);
#endif

	if(bWriteTXTLog) {
		fwrite(utf8Buffer, sizeof(char), strlen(utf8Buffer), fpLog);
	}
	return utf8Buffer;
}

__int64 CheckAvailableDirSpace() 
{	
	unsigned __int64 lpFreeBytesAvailable		= 0LL;
	unsigned __int64 lpTotalNumberOfBytes		= 0LL; 
	unsigned __int64 lpTotalNumberOfFreeBytes	= 0LL;
	unsigned __int64 lpUsedSpace				= 0LL;
	TCHAR szImportDirectory[MAX_PATH];

	GetDlgItemText(hImportDlg, IDC_IMPORT_DIR_EDIT2, szImportDirectory,	sizeof(szImportDirectory));
	GetDiskFreeSpaceEx(szImportDirectory, (PULARGE_INTEGER)&lpFreeBytesAvailable, (PULARGE_INTEGER)&lpTotalNumberOfBytes, (PULARGE_INTEGER)&lpTotalNumberOfFreeBytes);
	lpUsedSpace = (lpTotalNumberOfBytes - lpTotalNumberOfFreeBytes);

	lpTotalDiskSpace	= lpTotalNumberOfBytes; 
	lpUsedDiskSpace		= lpUsedSpace;

	return lpTotalNumberOfFreeBytes;
}

__int64 BurnerImporterSpaceCheck() {
	
	OFSTRUCT ofs;
	int nRes							= 0;
	unsigned __int64 lpFileSize			= 0LL;
	unsigned __int64 lpTotalFileSize	= 0LL;
	TCHAR szFile[MAX_PATH];
	TCHAR szOriginalDir[MAX_PATH];
	TCHAR szExtension[6];

	GetDlgItemText(hImportDlg, IDC_IMPORT_DIR_EDIT1, szOriginalDir,	sizeof(szOriginalDir));
	
	if(nImportOption == IMPORT_ICONS || nImportOption == IMPORT_ROMS || nImportOption == IMPORT_TRACKLISTS) {
		if(nImportOption	== IMPORT_ICONS)		_stprintf(szExtension, _T("%s"), _T(".ico"));
		if(nImportOption	== IMPORT_ROMS)			_stprintf(szExtension, _T("%s"), _T(".zip"));
		if(nImportOption	== IMPORT_TRACKLISTS)	_stprintf(szExtension, _T("%s"), _T(".lst"));
	} else {
		_stprintf(szExtension, _T("%s"), _T(".png"));		
	}

	// Let's put this here temp...
	TCHAR szPercent[1024];
	_stprintf(szPercent, _T(" Checking space requirements...(please wait)"));
	SetDlgItemText(hImportDlg, IDC_STATIC1, szPercent);

	unsigned int nOldDrvSel = nBurnDrvSelect;

	TCHAR szPercentAlt[1024];

	for(unsigned int nDrvIndex = 0; nDrvIndex < nBurnDrvCount; nDrvIndex++)
	{
		nBarCount++;
		if(nBarCount == (nBurnDrvCount / 50)) {
			SendDlgItemMessage(hImportDlg, IDC_WAIT_PROG, PBM_STEPIT, 0, 0);
			nBarCount = 0;
		}

		_stprintf(szPercentAlt, _T("%3.0f%%"), (float)((float)nDrvIndex / (float)nBurnDrvCount) * 100.0f);	// Update progress percent display

		SetDlgItemText(hImportDlg, IDC_IMPORT_STATIC5, szPercentAlt);

		nBurnDrvSelect = nDrvIndex;

		if(IS_MEGADRIVE) {
			nImportDrvCount++;
			nImportLogCountExcluded++;									// File excluded
		} else {
			_stprintf(szFile, _T("%s%s%s"), szOriginalDir, BurnDrvGetText(DRV_NAME), szExtension);
			FILE* fp = _tfopen(szFile, _T("r"));
			if(fp) {
				fclose(fp);												// no need to keep open right now
				nRes = GetFileSizeEx((HANDLE)OpenFile(_TtoA(szFile), &ofs, OF_READ), (PLARGE_INTEGER)&lpFileSize);
				if(nRes) {
					nImportDrvCount++;
					nImportLogCount++;									// Check OK
					lpTotalFileSize = lpTotalFileSize + lpFileSize;
				} else {
					nImportDrvCount++;
					nImportLogCountFail++;								// Check failed
				}				
			} else {
				nImportDrvCount++;
				nImportLogCountNotFound++;								// File not found
			}
		}
	}
	nBurnDrvSelect = nOldDrvSel;

	TCHAR szCount[4][256];
	TCHAR szTotFileSize[3][256];

	_uInt64ToCommaFormattedTCHAR( szTotFileSize[0], (__int64)lpTotalFileSize);
	_uInt64ToCommaFormattedTCHAR( szTotFileSize[1], (__int64)CALCKILOBYTE(lpTotalFileSize));
	_stprintf(szTotFileSize[2], _T("%1.1f"), CALCMEGABYTE(lpTotalFileSize));
	_uInt64ToCommaFormattedTCHAR( szCount[0],		nImportLogCount);
	_uInt64ToCommaFormattedTCHAR( szCount[1],		nImportLogCountNotFound);
	_uInt64ToCommaFormattedTCHAR( szCount[2],		nImportLogCountExcluded);
	_uInt64ToCommaFormattedTCHAR( szCount[3],		nImportLogCountFail);

	TCHAR szResult[MAX_PATH];
	_stprintf(szResult, _T("Total space needed for import: \n\n\t%s bytes\n\t%s KB\n\t%s MB\n\nFound: %s files\nNot found: %s files\nExcluded: %s files\nFile size check failed: %s files\n"), szTotFileSize[0], szTotFileSize[1], szTotFileSize[2], szCount[0], szCount[1], szCount[2], szCount[3]);
	MessageBox(hImportDlg, szResult, _T(APP_TITLE), MB_OK);

	nImportDrvCount			= 0;
	nImportLogCount			= 0;
	nImportLogCountFail		= 0;
	nImportLogCountNotFound = 0;
	nImportLogCountExcluded	= 0;

	SetDlgItemText(hImportDlg, IDC_STATIC1, _T(" To continue, please select the directory for imported files."));

	return lpTotalFileSize;
}

void BurnerImporter(char* utf8Buffer, FILE* fpLog, FILE* fpLogNF, TCHAR szOriginalDir[MAX_PATH], TCHAR szTargetDir[MAX_PATH], TCHAR szExtension[MAX_PATH]) 
{
	TCHAR lpExistingFileName[MAX_PATH]; 
	TCHAR lpNewFileName[MAX_PATH];

	nImportDrvCount			= 0;
	nImportLogCount			= 0;
	nImportLogCountFail		= 0;
	nImportLogCountNotFound = 0;
	nImportLogCountExcluded	= 0;

	// TODO: Clear the progress bar to 0% before starting the loop
	// ...

	unsigned int nOldDrvSel = nBurnDrvSelect;

	for(unsigned int nDrvIndex = 0; nDrvIndex < nBurnDrvCount; nDrvIndex++)
	{
		nBarCount++;
		if(nBarCount == (nBurnDrvCount / 50)) {
			SendDlgItemMessage(hImportDlg, IDC_WAIT_PROG, PBM_STEPIT, 0, 0);
			nBarCount = 0;
		}

		nBurnDrvSelect = nDrvIndex;

		if(IS_MEGADRIVE) {

			nImportDrvCount++;
			nImportLogCountExcluded++;

			// STATUS
			TCHAR szFinalStatusText[1024];
			_stprintf(szFinalStatusText, _T(" [ %s%s ] MegaDrive file, not imported"), BurnDrvGetText(DRV_NAME), szExtension);
			SetDlgItemText(hImportDlg, IDC_STATIC1, szFinalStatusText);
			
			// LOG 
			/*
			TCHAR szLogData[1024];
			_stprintf(szLogData, _T("[ %s%s ] MegaDrive file, not imported <br>"), BurnDrvGetText(DRV_NAME), szExtension);
			
			WriteToLog(utf8Buffer, szLogData, fpLog);
			*/

			//char* Text = WriteToLog(utf8Buffer, szLogData, fpLog);

			//SendDlgItemMessageA(hImportDlg, IDC_IMPORT_LOG_LISTBOX, LB_ADDSTRING, iIndex, (LPARAM)Text);

			_stprintf(szFinalStatusText, _T(" Excluded: \t%i"), nImportLogCountExcluded);
			SetDlgItemText(hImportDlg, IDC_IMPORT_STATIC4, szFinalStatusText);
		} else {
			TCHAR szFile[MAX_PATH];
			_stprintf(szFile, _T("%s%s%s"), szOriginalDir, BurnDrvGetText(DRV_NAME), szExtension);

			FILE* fp	= _tfopen(szFile, _T("r"));
			if(fp) {
				fclose(fp); // no need to keep open right now

				_stprintf(lpExistingFileName, _T("%s"), szFile);
				_stprintf(lpNewFileName, _T("%s%s%s"), szTargetDir, BurnDrvGetText(DRV_NAME), szExtension);

				if(CopyFile(lpExistingFileName, lpNewFileName, FALSE))
				{
					nImportDrvCount++;
					nImportLogCount++;	// Import OK
					
					TCHAR szFinalStatusText[MAX_PATH];
					TCHAR szLogData[MAX_PATH];
					if(IS_CLONE) {
						_stprintf(szFinalStatusText, _T(" [ %s ] - Import OK! [ Clone ]"), lpNewFileName);	// STATUS
						_stprintf(szLogData, _T("<tr><td>%s</td><td>Import OK!</td><td>Clone</td></tr>\n"), lpNewFileName);			// LOG
					} else {
						_stprintf(szFinalStatusText, _T(" [ %s ] - Import OK! [ Parent ]"), lpNewFileName);	// STATUS
						_stprintf(szLogData, _T("<tr><td>%s</td><td>Import OK!</td><td>Parent</td></tr>\n"), lpNewFileName);		// LOG
					}
					SetDlgItemText(hImportDlg, IDC_STATIC1, szFinalStatusText);

					// LOG
					WriteToLog(utf8Buffer, szLogData, fpLog);
					//char* Text = WriteToLog(utf8Buffer, szLogData, fpLog);

					OFSTRUCT ofs;
					unsigned __int64 lpFSize = 0LL;
					TCHAR szFSize[MAX_PATH];
					GetFileSizeEx((HANDLE)OpenFile(_TtoA(lpNewFileName), &ofs, OF_READ), (PLARGE_INTEGER)&lpFSize);
					int nSFormat = CheckByteSizeFormat(lpFSize);
					_GetProperSizeText(szFSize, nSFormat, lpFSize);
			
					InsertLVItem(0, lpNewFileName, szFSize, 0);
					
					//SendDlgItemMessageA(hImportDlg, IDC_IMPORT_LOG_LISTBOX, LB_ADDSTRING, iIndex, (LPARAM)Text);

					_stprintf(szFinalStatusText, _T(" Imported: \t%i of %i"), nImportLogCount, (nImportLogCount + nImportLogCountFail + nImportLogCountNotFound));
					SetDlgItemText(hImportDlg, IDC_IMPORT_STATIC1, szFinalStatusText);
				} else {
					nImportDrvCount++;
					nImportLogCountFail++; // Copy failed

					TCHAR szFinalStatusText[MAX_PATH];
					TCHAR szLogData[MAX_PATH];
					if(IS_CLONE) {
						_stprintf(szFinalStatusText, _T(" [ %s ] - Import Failed [!][ Couldn't copy the file ] [ Clone ]"), lpNewFileName);	// STATUS
						_stprintf(szLogData, _T("<tr><td>%s</td><td>Copy Error!</td><td>Clone</td></tr>\n"), lpNewFileName);			// LOG
					} else {
						_stprintf(szFinalStatusText, _T(" [ %s ] - Import Failed [!][ Couldn't copy the file ] [ Parent ]"), lpNewFileName);	// STATUS
						_stprintf(szLogData, _T("<tr><td>%s</td><td>Copy Error!</td><td>Parent</td></tr>\n"), lpNewFileName);			// LOG
					}
					SetDlgItemText(hImportDlg, IDC_STATIC1, szFinalStatusText);

					// LOG
					WriteToLog(utf8Buffer, szLogData, fpLog);

					//char* Text = WriteToLog(utf8Buffer, szLogData, fpLog);

					//SendDlgItemMessageA(hImportDlg, IDC_IMPORT_LOG_LISTBOX, LB_ADDSTRING, iIndex, (LPARAM)Text);

					OFSTRUCT ofs;
					unsigned __int64 lpFSize = 0LL;
					TCHAR szFSize[MAX_PATH];
					GetFileSizeEx((HANDLE)OpenFile(_TtoA(lpNewFileName), &ofs, OF_READ), (PLARGE_INTEGER)&lpFSize);
					int nSFormat = CheckByteSizeFormat(lpFSize);
					_GetProperSizeText(szFSize, nSFormat, lpFSize);

					InsertLVItem(1, lpNewFileName, szFSize, 0);

					_stprintf(szFinalStatusText, _T(" Copy error: \t%i"), nImportLogCountFail);
					SetDlgItemText(hImportDlg, IDC_IMPORT_STATIC2, szFinalStatusText);
				}
			} else {
				nImportDrvCount++;
				nImportLogCountNotFound++; // File not found

				TCHAR szFinalFile[MAX_PATH];
				TCHAR szFinalStatusText[MAX_PATH];
				TCHAR szLogData[MAX_PATH];
				
				_stprintf(szFinalFile, _T("%s%s%s"), szOriginalDir, BurnDrvGetText(DRV_NAME), szExtension);		// LOG

				if(IS_CLONE) {
					_stprintf(szFinalStatusText, _T(" [ %s ] - Import Failed [!][ File not found ][ Clone ]"), szFinalFile);		// STATUS
					_stprintf(szLogData, _T("<tr><td>%s</td><td>File not found</td><td>Clone</td></tr>\n"), szFinalFile);		// LOG
				} else {
					_stprintf(szFinalStatusText, _T(" [ %s ] - Import Failed [!][ File not found ][ Parent ]"), szFinalFile);	// STATUS
					_stprintf(szLogData, _T("<tr><td>%s</td><td>File not found</td><td>Parent</td></tr>\n"), szFinalFile);		// LOG
				}
				SetDlgItemText(hImportDlg, IDC_STATIC1, szFinalStatusText);

				// LOG
				WriteToLog(utf8Buffer, szLogData, fpLog);
				WriteToLog(utf8Buffer, szLogData, fpLogNF);

				InsertLVItem(2, szFinalFile, _T("Unknown"), 0);

				//char* Text = WriteToLog(utf8Buffer, szLogData, fpLogNF);

				//SendDlgItemMessageA(hImportDlg, IDC_IMPORT_LOG_LISTBOX, LB_ADDSTRING, iIndex, (LPARAM)Text);

				_stprintf(szFinalStatusText, _T(" Not found: \t%i"), nImportLogCountNotFound);
				SetDlgItemText(hImportDlg, IDC_IMPORT_STATIC3, szFinalStatusText);
			}

			// Select last item on list
			int nSel = ListView_GetItemCount(hImportListView) - 1;
			DynamicSelectionScroll(hImportListView, nSel);

		}
	}
	nBurnDrvSelect = nOldDrvSel;
}

void ImportPackage(int/*nPackage*/) {

	char* utf8Buffer					= NULL;
	TCHAR szMAMESupportDir[MAX_PATH];
	TCHAR szImportDir[MAX_PATH];

	GetDlgItemText(hImportDlg, IDC_IMPORT_DIR_EDIT1, szMAMESupportDir,	sizeof(szMAMESupportDir));
	GetDlgItemText(hImportDlg, IDC_IMPORT_DIR_EDIT2, szImportDir,		sizeof(szImportDir));

	CreateDirectory(szImportDir, NULL);	// If for any weird reason the selected import directory doesn't exist, create it =).

	FILE* fpLog = NULL;
	
	if(bWriteTXTLog){
		fpLog =_tfopen(_T("import_log_complete.html"), _T("w"));
		if(!fpLog){
			TCHAR szError[MAX_PATH];
			_stprintf(szError, _T("Error couldn't create: import_log_complete.html"));
			MessageBox(hImportDlg, szError, _T(APP_TITLE), MB_OK);
			EndDialog(hImportDlg, 0);
			return;
		}
	}

	FILE* fpLogNF = NULL;
	
	if(bWriteTXTLog) {
		fpLogNF = _tfopen(_T("import_log_missing.html"), _T("w"));
		if(!fpLogNF){
			TCHAR szError[MAX_PATH];
			_stprintf(szError, _T("Error couldn't create: import_log_missing.html"));
			MessageBox(hImportDlg, szError, _T(APP_TITLE), MB_OK);
			EndDialog(hImportDlg, 0);
			return;
		}
	}

	SetDlgItemText(hImportDlg, IDC_STATIC1, _T(" Starting import process (please wait)..."));

	TCHAR szExtension[6];

	if(nImportOption == IMPORT_ICONS || nImportOption == IMPORT_ROMS || nImportOption == IMPORT_TRACKLISTS) {
		if(nImportOption	== IMPORT_ICONS)		_stprintf(szExtension, _T("%s"), _T(".ico"));
		if(nImportOption	== IMPORT_ROMS)			_stprintf(szExtension, _T("%s"), _T(".zip"));
		if(nImportOption	== IMPORT_TRACKLISTS)	_stprintf(szExtension, _T("%s"), _T(".lst"));
	} else {
		_stprintf(szExtension, _T("%s"), _T(".png"));		
	}

	// Start HTML log
	TCHAR szLogHeader[3500];
	_stprintf(szLogHeader, _T(" \
<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.1//EN\" \"http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd\"> \
\
<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"en\"> \
\
<head> \
<title>FB Alpha - File Importer's Log</title> \
<meta http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-1\" /> \
<style type=\"text/css\"> \
	body { \
		font-family: verdana, lucida, sans-serif; \
		font-size: 0.8em; \
		color: #333333; \
		background-color: #CE6A57; \
		margin: 0em; \
		text-align: center; \
	} \
\
	a:link { \
		color: #C03029; \
		text-decoration: none; \
	} \
\
	a:visited { \
		color: #C03029; \
		text-decoration: none; \
	} \
\
	a:hover { \
		color: #333333; \
		text-decoration: underline; \
	} \
\
	a:active { \
		color: #C03029; \
		text-decoration: underline; \
	} \
\
	a.active:link { \
		color: #C03029; \
		text-decoration: none; \
		font-weight:bold; \
		font-style: italic; \
	} \
\
	a.active:visited { \
		color: #C03029; \
		text-decoration: none; \
		font-weight:bold; \
		font-style: italic; \
	} \
\
	a.active:hover { \
		color: #333333; \
		text-decoration: underline; \
		font-weight:bold; \
		font-style: italic; \
	} \
\
	a.active:active { \
		color: #C03029; \
		text-decoration: underline; \
		font-weight:bold; \
		font-style: italic; \
	} \
\
	h2 { \
		color: #C03029; \
		font-size: 1.4em; \
		margin-top: 1em; \
	} \
\
	h3 { \
		color: #C03029; \
		font-size: 1.0em; \
	} \
\
	li { \
		padding: 0.1em; \
	} \
	td { border: 1px solid #C0C0C0; } \
	.lol { width: 20%%; } \
\
	.outer { \
		width: 78em; \
		margin: 0em auto; \
		text-align: left; \
		padding: 0.7em 0.7em 0.7em 0.7em; \
		background-color: #FFFFFF; \
		border: 0.08em solid #808080; \
		margin-top: 1em; \
		margin-bottom: 1em; \
	} \
\
	.note { \
		color: #C03029; \
		padding: 1em; \
		background-color: #DDD9D9; \
		font-style: italic; \
	} \
        .style1 \
        { \
            width: 90%%; \
            border-left-style: solid; \
            border-left-width: 1px; \
            border-right: 1px solid #C0C0C0; \
            border-top-style: solid; \
            border-top-width: 1px; \
            border-bottom: 1px solid #C0C0C0; \
        } \
</style> \
</head> \
\
<body> \
\
<div class=\"outer\"> \
\
    <table align=\"center\" cellspacing=\"1\" class=\"style1\"> \
	<tr><td> \
	<h1>FB Alpha File Importer's Log</h1><a href=\"import_log_complete.html\">import_log_complete.html</a> (full log)<br><a href=\"import_log_missing.html\">import_log_missing.html</a> (missing files log only)<br><br> \
	</td></tr> \
	</table> \
	<br> \
    <table align=\"center\" cellspacing=\"1\" class=\"style1\"> \
	<tr><td>File</td><td class=\"lol\">Status</td><td>Parent / Clone</td></tr> \
"));

	WriteToLog(utf8Buffer, szLogHeader, fpLog);
	WriteToLog(utf8Buffer, szLogHeader, fpLogNF);

	BurnerImporter( utf8Buffer, fpLog, fpLogNF, szMAMESupportDir, szImportDir, szExtension);

	SetDlgItemText(hImportDlg, IDC_STATIC1, _T(" Importing process ended, enjoy!"));

	TCHAR szStatus[1024];
	_stprintf(szStatus, _T("<br>Imported:  %i of %i files<br>Excluded: %i files<br>Copy Error(s): %i <br>Not Found: %i <br>Process count: %i <br><br>"), nImportLogCount, nImportLogCount + nImportLogCountFail + nImportLogCountNotFound, nImportLogCountExcluded, nImportLogCountFail, nImportLogCountNotFound, nImportDrvCount);
	
	// Finalize HTML log
	TCHAR szLogFooter[3500];
	_stprintf(szLogFooter, _T(" \
</table> \
<br> \
    <table align=\"center\" cellspacing=\"1\" class=\"style1\"> \
	<tr><td> \
%s \
</td></tr> \
</table> \
</div> \
\
</body> \
</html> \
"), szStatus);

	WriteToLog(utf8Buffer, szLogFooter, fpLog);
	WriteToLog(utf8Buffer, szLogFooter, fpLogNF);


	if(fpLog)				fclose(fpLog);
	if(fpLogNF)				fclose(fpLogNF);

	nImportDrvCount			= 0;
	nImportLogCount			= 0;
	nImportLogCountFail		= 0;
	nImportLogCountNotFound = 0;
	nImportLogCountExcluded	= 0;
	bIsMegaDrive			= false;
	bImportFinished			= true;
}

bool GetCheckBoxState(HWND hDlg, int nCtrl) 
{
	bool nRet		= 0;
	UINT nStatus	= IsDlgButtonChecked(hDlg, nCtrl);

	switch(nStatus) {
		case BST_CHECKED:	nRet = true;	break;	// The button is checked.
		case BST_UNCHECKED:	nRet = false;	break;	// The button is not checked.
	}
	return nRet;
}

static BOOL CALLBACK ImporterProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM /*lParam*/) 
{	
	if(Msg == WM_INITDIALOG) 
	{
		hImportDlg				= hDlg;
		hImportLabel			= GetDlgItem(hDlg, IDC_STATIC1);
		hImportEdit1			= GetDlgItem(hDlg, IDC_IMPORT_DIR_EDIT1);
		hImportEdit2			= GetDlgItem(hDlg, IDC_IMPORT_DIR_EDIT2);			
		hImportBrowse1			= GetDlgItem(hDlg, IDC_IMPORT_BROWSE1);
		hImportBrowse2			= GetDlgItem(hDlg, IDC_IMPORT_BROWSE2);
		hImportCheckboxes[0]	= GetDlgItem(hDlg, IDC_IMPORT_CHECK1);
		//hImportCheckboxes[1]	= GetDlgItem(hDlg, IDC_IMPORT_CHECK2);		
		hImportCheckboxes[2]	= GetDlgItem(hDlg, IDC_IMPORT_CHECK3);		// Open log after completion
		hImportCheckboxes[3]	= GetDlgItem(hDlg, IDC_IMPORT_CHECK4);		// Close this dialog after completion

		hImportCheckboxes[4]	= GetDlgItem(hDlg, IDC_IMPORT_CHECK5);		// Edit Package path
		hImportCheckboxes[5]	= GetDlgItem(hDlg, IDC_IMPORT_CHECK6);		// Edit Import path

		hImportListView			= GetDlgItem(hDlg, IDC_IMPORT_LISTVIEW);	// Import ListView
		hSysMenu				= GetSystemMenu(hDlg, FALSE);

		hImportProgressPercent	= GetDlgItem(hDlg, IDC_IMPORT_STATIC5);		// Import progress percent label

		switch (nImportOption) {
			case IMPORT_PREVIEWS:	_stprintf(szSupportType, _T("%s"), _T("Previews"));		break;
			case IMPORT_TITLES:		_stprintf(szSupportType, _T("%s"), _T("Titles"));		break;
			case IMPORT_FLYERS:		_stprintf(szSupportType, _T("%s"), _T("Flyers"));		break;
			case IMPORT_CABINETS:	_stprintf(szSupportType, _T("%s"), _T("Cabinets"));		break;
			case IMPORT_MARQUEES:	_stprintf(szSupportType, _T("%s"), _T("Marquees"));		break;
			case IMPORT_CPANELS:	_stprintf(szSupportType, _T("%s"), _T("C.Panels"));		break;
			case IMPORT_PCBS:		_stprintf(szSupportType, _T("%s"), _T("PCBs"));			break;
			case IMPORT_ICONS:		_stprintf(szSupportType, _T("%s"), _T("Icons"));		break;
			case IMPORT_ROMS:		_stprintf(szSupportType, _T("%s"), _T("ROMs"));			break;
			case IMPORT_TRACKLISTS:	_stprintf(szSupportType, _T("%s"), _T("Tracklists"));	break;
		}

		SendDlgItemMessage(hImportDlg, IDC_WAIT_PROG, PBM_SETSTEP, 2, 0);

		SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)LoadIcon(hAppInst, MAKEINTRESOURCE(IDI_APP))); // Set dialog icon

		TCHAR szEditText[128];
		_stprintf(szEditText, _T("Select the %s directory you wish to import"), szSupportType);
		SetDlgItemText(hDlg, IDC_IMPORT_DIR_EDIT1, szEditText);

		_stprintf(szEditText, _T("Select the %s directory for imported files"), szSupportType);
		SetDlgItemText(hDlg, IDC_IMPORT_DIR_EDIT2, szEditText);

		_stprintf(szEditText, _T("%s directory..."), szSupportType);
		SetDlgItemText(hDlg, IDC_IMPORT_BROWSE1, szEditText);

		SetDlgItemText(hDlg, IDC_STATIC1, _T(" Select the directory where MAME support files are located."));

		EnableWindow(hImportBrowse2, false);			// disable 2nd browse button
		EnableWindow(hImportCheckboxes[2], false);		// disable VIEW LOG FILES AFTER IMPORT

		// Disable WIP options at the moment
		EnableWindow(hImportCheckboxes[4], false);	// edit path 1
		EnableWindow(hImportCheckboxes[5], false);	// edit path 2
		
		//_stprintf(szEditText, _T("Imported %s directory..."), szSupportType);
		//SetDlgItemText(hDlg, IDC_IMPORT_BROWSE2, szEditText);

		SetListViewColumns();

		TCHAR szNewTitle[MAX_PATH];
		_stprintf(szNewTitle, _T("FB Alpha Importer") _T(SEPERATOR_1) _T("Import %s"), szSupportType);
		SetWindowText(hDlg, szNewTitle);

		WndInMid(hDlg, hScrnWnd);						// Center the Dialog

		return TRUE;
	}

	if(Msg == WM_COMMAND)
	{
		if (HIWORD(wParam) == BN_CLICKED) 
		{

		    LPMALLOC pMalloc		= NULL;
		    BROWSEINFO bInfo;
		    ITEMIDLIST* pItemIDList = NULL;
			TCHAR buffer[MAX_PATH];
			int var					= 0;
			int nCtrlID				= LOWORD(wParam);

			// IMPORT BUTTON
			if(nCtrlID == IDOK) {

				// Error message...
				if(bMAMESupportDir == false || bImportDir == false) {
					MessageBox(hImportDlg, _T("Please specify both directories, they are required for the import process."), _T(APP_TITLE), MB_OK | MB_ICONHAND);
					return 0;
				}

				ImportProcessStart(PROC_IMPORTSUPPORTFILES, hImportDlg);
			}

			// EDIT PACKAGE PATH
			if(nCtrlID == IDC_IMPORT_CHECK5) {
				if(bProcessing == true) return 0;
				
				// toggle the option and disable the browse button
				bEditPath[0] = GetCheckBoxState(hDlg, nCtrlID);
				
				EnableWindow(hImportBrowse1, bEditPath[0] ? false : true);			// toggle status of the 1st browse button
				EnableWindow(hImportEdit1, bEditPath[0] ? true : false);			// toggle status of the 1st edit box
			}

			// EDIT IMPORT PATH
			if(nCtrlID == IDC_IMPORT_CHECK6) {
				if(bProcessing == true)	return 0;
				if(bMAMESupportDir == false) return 0;

				bEditPath[1] = GetCheckBoxState(hDlg, nCtrlID);

				EnableWindow(hImportBrowse2, bEditPath[1] ? false : true);			// toggle status of the 2nd browse button
				EnableWindow(hImportEdit2, bEditPath[1] ? true : false);			// toggle status of the 2nd edit box
			}

			// EXIT BUTTON
			if(nCtrlID == IDC_IMPORT_EXIT) {					
				//if(bProcessing == true) return 0;
				SendMessage(hDlg, WM_CLOSE, 0, 0);
			}

			// SAVE LOGS
			if(nCtrlID == IDC_IMPORT_CHECK1) {
				if(bProcessing == true) return 0;				
				bWriteTXTLog = GetCheckBoxState(hDlg, nCtrlID);
				if(bWriteTXTLog) {
					EnableWindow(hImportCheckboxes[2], true);		// View log after import option can be used now
				} else {
					EnableWindow(hImportCheckboxes[2], false);		// View log after import option can be used now
				}
			}


			//if(nCtrlID == IDC_IMPORT_CHECK2) {
			//	if(bProcessing == true) return 0;
			// ...
			//}

			// OPEN LOGS AFTER IMPORT
			if(nCtrlID == IDC_IMPORT_CHECK3) {
				if(bProcessing == true) return 0;
				bAutoOpenLogs = GetCheckBoxState(hDlg, nCtrlID);
			}

			// AUTO CLOSE DIALOG AFTER IMPORT
			if(nCtrlID == IDC_IMPORT_CHECK4) {
				if(bProcessing == true) return 0;
				bAutoCloseDlg = GetCheckBoxState(hDlg, nCtrlID);
			}

			// BROWSE BUTTONS
			if(LOWORD(wParam) >= IDC_IMPORT_BROWSE1 && LOWORD(wParam) <= IDC_IMPORT_BROWSE2) 
            {
				TCHAR szResult[MAX_PATH];
				TCHAR szCaption[MAX_PATH];
				TCHAR szSizeInfo[5][MAX_PATH];

				if(nCtrlID == IDC_IMPORT_BROWSE1) {
					_stprintf(szCaption, _T("Select %s directory..."), szSupportType);
				}

				var = IDC_IMPORT_DIR_EDIT1 + LOWORD(wParam) - IDC_IMPORT_BROWSE1;
				
    		    SHGetMalloc(&pMalloc);
    
    			memset(&bInfo, 0, sizeof(bInfo));
    			bInfo.hwndOwner			= hImportDlg;
    			bInfo.pszDisplayName	= buffer;
    			bInfo.lpszTitle			= szCaption;
    			bInfo.ulFlags			= BIF_EDITBOX | BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
    
    			pItemIDList				= SHBrowseForFolder(&bInfo);
        
    			if (pItemIDList) {
    				if (SHGetPathFromIDList(pItemIDList, buffer)) {
    					int strLen		= _tcslen(buffer);
    					if (strLen) {
    						if (buffer[strLen - 1] != _T('\\')) {
    							buffer[strLen]		= _T('\\');
    							buffer[strLen + 1]	= _T('\0');
    						}
							SendMessage(GetDlgItem(hDlg, var), WM_SETTEXT, 0, (LPARAM)buffer);

							// The user has no choice but to follow order...
							// 1st select the MAME dir to calculate required space...
							// 2nd select destination directory and finally calculate if the required space and available space are good 
							if(nCtrlID == IDC_IMPORT_BROWSE1) {
								bMAMESupportDir			= true;											// MAME support directory selected
								ImportProcessStart(PROC_IMPORT_CHECKDISKSPACE, hDlg);					// check required disk space
								// Should display a messagebox here with information aboput the package
							}
							if(nCtrlID == IDC_IMPORT_BROWSE2) {
								bImportDir				= true;											// Import directory selected
								lpAvailableDiskSpace	= CheckAvailableDirSpace();						// check for available disk space
								lpRemainingDiskSpace	= (__int64)(lpAvailableDiskSpace - lpRequiredDiskSpace);

								// CheckByteSizeFormat() return values:
									// 0 = Bytes
									// 1 = KB
									// 2 = MB
									// 3 = GB
									// 4 = TB

								int nSizeFormat[5];
								nSizeFormat[0] = CheckByteSizeFormat(lpAvailableDiskSpace);
								nSizeFormat[1] = CheckByteSizeFormat(lpRequiredDiskSpace);
								nSizeFormat[2] = CheckByteSizeFormat(lpRemainingDiskSpace);
								nSizeFormat[3] = CheckByteSizeFormat(lpTotalDiskSpace);
								nSizeFormat[4] = CheckByteSizeFormat(lpUsedDiskSpace);

								_GetProperSizeText(szSizeInfo[0], nSizeFormat[0], lpAvailableDiskSpace);
								_GetProperSizeText(szSizeInfo[1], nSizeFormat[1], lpRequiredDiskSpace);
								_GetProperSizeText(szSizeInfo[2], nSizeFormat[2], lpRemainingDiskSpace);
								_GetProperSizeText(szSizeInfo[3], nSizeFormat[3], lpTotalDiskSpace);
								_GetProperSizeText(szSizeInfo[4], nSizeFormat[4], lpUsedDiskSpace);

								// For now we are checking exact information, if the user wanna full the disk he's gonna make it xD
								if(lpRequiredDiskSpace < lpAvailableDiskSpace) {
									// DISK SPACE OK!
									// - available disk space
									// - required disk space
									// - remaining disk space after import										
									_stprintf( szResult, _T("There is enough disk space for import!. However, you should always \nconsider checking how much space will remain after import.\n\nTotal disk capacity: %s\nUsed disk space: %s\nAvailable disk space: %s\nRequired disk space: %s\nRemaining disk space after import: %s\n"), szSizeInfo[3], szSizeInfo[4],szSizeInfo[0], szSizeInfo[1], szSizeInfo[2]);
									
									MessageBox(hImportDlg, szResult, _T("FB Alpha - Disk space is OK!"), MB_OK | MB_ICONINFORMATION);
									
									SetDlgItemText(hImportDlg, IDC_STATIC1, _T(" Ready! Click 'Start Import' button to begin!."));
								} else {
									// DISK SPACE ERROR!
									// - available disk space
									// - required disk space
									// - remaining disk space after the import (this will be negative)

									bImportDir	= false;	// make the user select another directory with enough disk space

									_stprintf( szResult, _T("There is not enough disk space for import in the selected directory. Please make sure \nyou specify a location with enough disk space.\n\nTotal disk capacity: %s\nUsed disk space: %s\nAvailable disk space: %s\nRequired disk space: %s\nRemaining disk space after import: %s\n"), szSizeInfo[3], szSizeInfo[4],szSizeInfo[0], szSizeInfo[1], szSizeInfo[2]);
									
									MessageBox(hImportDlg, szResult, _T("FB Alpha - Error: Not enough disk space for import"), MB_OK | MB_ICONHAND);
									
									SetDlgItemText(hImportDlg, IDC_STATIC1, _T(" Error: Select another directory with enough disk space."));
								}
								//lpRequiredDiskSpace		= 0LL;
								lpAvailableDiskSpace	= 0LL;
								lpRemainingDiskSpace	= 0LL;
								lpTotalDiskSpace		= 0LL;
								lpUsedDiskSpace			= 0LL;
							}
    					}
    				}
    				pMalloc->Free(pItemIDList);
    			}
        		pMalloc->Release();					
			}
		}
		//
	}

	if(Msg == WM_CLOSE)
	{
		//if(bProcessing) return 0;

		bAutoOpenLogs			= false;

		nImportDrvCount			= 0;
		nImportLogCount			= 0;
		nImportLogCountFail		= 0;
		nImportLogCountNotFound = 0;
		nImportLogCountExcluded	= 0;
		bIsMegaDrive			= false;
		bImportFinished			= false;
		bProcessing				= false;
		bMAMESupportDir			= false;
		bImportDir				= false;
		EndDialog(hDlg, 0);
		hImportDlg				= NULL;
		nBarCount				= 0;
	}

	return 0;
}

static DWORD WINAPI DoImportDlg(LPVOID) {
	MSG msg;
	FBADialogBox(hAppInst, MAKEINTRESOURCE(IDD_IMPORT_LOG_DLG), hScrnWnd, ImporterProc);
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return 0;
}

int ImporterDlgCreate(int nPackage) {
	DWORD dwThreadID = 0;
	nImportOption = nPackage;
	CreateThread(NULL, 0, DoImportDlg, NULL, THREAD_TERMINATE, &dwThreadID);
	return 0;
}

// LOOP PROCESS RELATED ------------------------------------------------------------------
int		nProcess					= 0;
HWND	hProcParent					= NULL;

static BOOL CALLBACK DummyProcStart(HWND hDlg, UINT Msg, WPARAM /*wParam*/, LPARAM /*lParam*/) 
{
	if (Msg == WM_INITDIALOG) {
		switch(nProcess) {
			case PROC_IMPORTSUPPORTFILES: {

				bProcessing = true;

				EnableWindow(hImportCheckboxes[0], false);						// 
				EnableWindow(hImportBrowse1, false);							// disable 1st browse button
				EnableWindow(hImportBrowse2, false);							// disable 2nd browse button
				
				EnableWindow(hImportCheckboxes[4], false);						// disable 1st edit path checkbox
				EnableWindow(hImportCheckboxes[5], false);						// disable 1st edit path checkbox
				
				EnableWindow(hImportBrowse2, false);							// disable 2nd browse button
				EnableWindow(hImportBrowse2, false);							// disable 2nd browse button

				EnableWindow(hImportCheckboxes[2], false);						// disable VIEW LOG FILES AFTER IMPORT
				EnableWindow(hImportCheckboxes[3], false);						// disable AUTO CLOSE DIALOG AFTER IMPORT

				EnableWindow(GetDlgItem(hImportDlg, IDOK), false);
				//EnableWindow(GetDlgItem(hImportDlg, IDC_IMPORT_EXIT), false);
				EnableMenuItem(hSysMenu, SC_CLOSE, MF_BYCOMMAND | MF_GRAYED);

				ImportPackage(nImportOption);

				// Check if we need to load icons
				if(bEnableIcons && nImportOption == IMPORT_ICONS) {
					bIconsLoaded = 0;
					// load driver icons
					LoadDrvIcons();
					bIconsLoaded = 1;
				}

				EnableMenuItem(hSysMenu, SC_CLOSE, MF_BYCOMMAND | MF_ENABLED);
				//EnableWindow(GetDlgItem(hImportDlg, IDC_IMPORT_EXIT), true);

				// Open logs after import
				if(bAutoOpenLogs) {
					ShellExecute(NULL, _T("open"), _T("import_log_complete.html"),	NULL, NULL, SW_SHOWNORMAL);
				}

				// Close after import
				if(bAutoCloseDlg) {
					EndDialog(hImportDlg, 0);
				}

				//EnableWindow(hImportCheckboxes[0], true);
				bProcessing = false;
			}	
			break;

			case PROC_IMPORT_CHECKDISKSPACE: {
				bProcessing = true;
				EnableWindow(hImportCheckboxes[0], false);

				EnableWindow(GetDlgItem(hImportDlg, IDOK), false);
				//EnableWindow(GetDlgItem(hImportDlg, IDC_IMPORT_EXIT), false);

				EnableMenuItem(hSysMenu, SC_CLOSE, MF_BYCOMMAND | MF_GRAYED);
				EnableWindow(hImportBrowse1, false);							// disable 1st browse button
				EnableWindow(hImportBrowse2, false);							// disable 2nd browse button

				EnableWindow(hImportCheckboxes[4], false);						// disable 1st edit path checkbox
				EnableWindow(hImportCheckboxes[5], false);						// disable 2nd edit path checkbox

				EnableWindow(hImportCheckboxes[2], false);						// disable VIEW LOG FILES AFTER IMPORT
				EnableWindow(hImportCheckboxes[3], false);						// disable AUTO CLOSE DIALOG AFTER IMPORT

				lpRequiredDiskSpace	= BurnerImporterSpaceCheck();				// check required disk space

				EnableMenuItem(hSysMenu, SC_CLOSE, MF_BYCOMMAND | MF_ENABLED);
				EnableWindow(GetDlgItem(hImportDlg, IDOK), true);
				//EnableWindow(GetDlgItem(hImportDlg, IDC_IMPORT_EXIT), true);
				EnableWindow(hImportCheckboxes[0], true);
				EnableWindow(hImportBrowse1, true);								// enable 1st browse button
				EnableWindow(hImportBrowse2, true);								// enable 2nd browse button
				
				//EnableWindow(hImportCheckboxes[4], true);						// enable 1st edit path checkbox
				//EnableWindow(hImportCheckboxes[5], true);						// enable 2nd edit path checkbox
				
				if(bWriteTXTLog) {
					EnableWindow(hImportCheckboxes[2], true);		// View log after import option can be used now
				}
				EnableWindow(hImportCheckboxes[3], true);						// enable AUTO CLOSE DIALOG AFTER IMPORT

				bProcessing = false;

				// If no support files are found...
				if(lpRequiredDiskSpace == 0LL) {

					// reset box messages
					TCHAR szEditText[128];
					_stprintf(szEditText, _T("Select the %s directory you wish to import"), szSupportType);
					SetDlgItemText(hImportDlg, IDC_IMPORT_DIR_EDIT1, szEditText);

					_stprintf(szEditText, _T("Select the %s directory for imported files"), szSupportType);
					SetDlgItemText(hImportDlg, IDC_IMPORT_DIR_EDIT2, szEditText);

					bMAMESupportDir		= false;								// turn this flag FALSE, so the user can select another dir
					EnableWindow(hImportBrowse2, false);						// disable 2nd browse button
					MessageBox(hImportDlg, _T("The directory you selected doesn't seem to have any \nof the files required for the import, try selecting another directory."), _T(APP_TITLE), MB_OK | MB_ICONHAND);
				}
			}
			break;
		}

		hImportProcessDlg = hDlg;
		ShowWindow(hDlg, SW_HIDE);
		SendMessage(hDlg, WM_CLOSE, 0, 0);
		return TRUE;
	}
	if (Msg == WM_CLOSE) {
		if(bProcessing) return 0;
		EndDialog(hDlg, 0);
		hImportProcessDlg = NULL;
	}
	return 0;
}

static DWORD WINAPI DoProcess(LPVOID) {
	MSG msg;
	HWND hDlg = hProcParent;
	CreateDialog(hAppInst, MAKEINTRESOURCE(IDD_DUMMY), hDlg, DummyProcStart);
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return 0;
}

int ImportProcessStart(int nProcID, HWND hProcWnd) {
	hProcParent		= hProcWnd;
	nProcess		= nProcID;
	hProcessThread	= CreateThread(NULL, 0, DoProcess, NULL, THREAD_TERMINATE, &dwProcessThreadID);
	return 0;
}
