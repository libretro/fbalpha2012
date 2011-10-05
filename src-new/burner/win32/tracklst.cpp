/* ------------------------------------------------------------------------------------
 FB Alpha Jukebox "Fusion" 
 --------------------------------------------------------------------------------------
 Description:
 --------------------------------------------------------------------------------------

 The FB Alpha Jukebox allows to launch games to hear their bg music or sound fx! 
 Here are the supported systems at this moment:

 - CPS-1				(Z80, YM2151 [Not QSound])
 - WWF Wrestlefest
 - NeoGeo				(Z80, YM2610) (added support on 2/17/2010) (CaptainCPS-X / IQ_132)

 These ones are working but doesn't have the jukebox core implemented yet,
 so they must be launched normally and while game is emulated you can
 change bg music and hear sound fx:

 - CPS-1 / CPS-2		(Z80, QSound)


// ----------------------------------------------------------------------------------*/

#include "burner.h"
#include "tracklst.h"

// --------------------------------------------------------------------------------------
// definitions
// --------------------------------------------------------------------------------------
#ifndef			LVS_EX_DOUBLEBUFFER
#define			LVS_EX_DOUBLEBUFFER		0x00010000
#endif

#define			LST_PLAYLIST			0
#define			DAT_PLAYLIST			1
#define			LV_TRACKNUMBERCOLUMN	0
#define			LV_TRACKCODECOLUMN		1
#define			LV_TRACKTITLECOLUMN		2
// --------------------------------------------------------------------------------------
// externs
// --------------------------------------------------------------------------------------
extern void		CpsSoundCmd(UINT16 sound_code);
extern void		QSoundCMD(unsigned short nTrack);
extern void		Cps1SoundStop();
extern int		bImageOrientation;
// --------------------------------------------------------------------------------------
// declarations
// --------------------------------------------------------------------------------------

/* GLOBALS */
HWND			hJukeboxDlg				= NULL;
bool			bDoPostInitialize		= false;

/* STATICS */
static HWND		hTrackList_LST			= NULL;
static HWND		hTrackList_DAT			= NULL;
static HWND		hTracklistTabs			= NULL;
static HBRUSH	hWhiteBGBrush			= NULL;
static HBITMAP	hBmp					= NULL;
static HBITMAP	hPreview				= NULL;
static bool		isClone_LST				= false;
static bool		isClone_DAT				= false;
static int		nActivePlaylist			= LST_PLAYLIST;
static int		nTrackCount_DAT			= 0;
static int		nTrackCount_LST			= 0;
static int		nCurrentTrack_DAT		= -1;
static int		nCurrentTrack_LST		= -1;
static HANDLE	hJukeboxThread			= NULL;
static DWORD	dwJukeboxThreadID		= 0;
static int		nAutoPauseOld			= 0;
// --------------------------------------------------------------------------------------

// Structure for each sound track
struct trackInfo {
	unsigned char nTrack;					// Sound Track Ex. "0001" (HEX value)
	TCHAR szName[256];				// Sound Track Title Ex. "Ryu's Theme"
};

struct trackInfo Tracks_LST[500];	// Soundtracks Array 
struct trackInfo Tracks_DAT[500];	// Soundtracks Array 

void SetupListViewColumns()
{
	LV_COLUMN LvCol;
	ListView_SetExtendedListViewStyle(hTrackList_LST, LVS_EX_FULLROWSELECT);
	ListView_SetExtendedListViewStyle(hTrackList_DAT, LVS_EX_FULLROWSELECT);

	memset(&LvCol, 0, sizeof(LvCol));
	LvCol.mask		= LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;

	LvCol.cx		= 35;
	LvCol.pszText	= _T("#");
	SendMessage(hTrackList_LST, LVM_INSERTCOLUMN , LV_TRACKNUMBERCOLUMN, (LPARAM)&LvCol);
	SendMessage(hTrackList_DAT, LVM_INSERTCOLUMN , LV_TRACKNUMBERCOLUMN, (LPARAM)&LvCol);

	LvCol.cx		= 50;
	LvCol.pszText	= _T("Code");
	SendMessage(hTrackList_LST, LVM_INSERTCOLUMN , LV_TRACKCODECOLUMN, (LPARAM)&LvCol);
	SendMessage(hTrackList_DAT, LVM_INSERTCOLUMN , LV_TRACKCODECOLUMN, (LPARAM)&LvCol);

	LvCol.cx		= 455;
	LvCol.pszText	= _T("Track Title");
	SendMessage(hTrackList_LST, LVM_INSERTCOLUMN , LV_TRACKTITLECOLUMN, (LPARAM)&LvCol);
	SendMessage(hTrackList_DAT, LVM_INSERTCOLUMN , LV_TRACKTITLECOLUMN, (LPARAM)&LvCol);
}

int ParseM1Tracklist() 
{
	LV_ITEM LvItem;
	memset(&LvItem, 0, sizeof(LvItem));
	LvItem.mask			= LVIF_TEXT;
	LvItem.cchTextMax	= 256;

	// Read the list file
	TCHAR szFileName[MAX_PATH];
	_stprintf(szFileName, _T("%s%s.lst"), szAppListsPath, BurnDrvGetText(DRV_NAME));

	FILE *fp = _tfopen(szFileName, _T("r"));
	if(fp) {
		isClone_LST = false;
	}

	if (!fp && BurnDrvGetText(DRV_PARENT)) {	
		// Try the parent
		_stprintf(szFileName, _T("%s%s.lst"), szAppListsPath, BurnDrvGetText(DRV_PARENT));
		fp = _tfopen(szFileName, _T("r"));
		if(fp) {
			isClone_LST = true;
		}
	}

	TCHAR szLine[1000];
	TCHAR* Temp				= NULL;
	TCHAR* TempString		= NULL;
	TCHAR* TempString2		= NULL;
	int inTrackList			= 0;
	int nDefaultTrack		= 0;
	int nListPos			= 0;
	nTrackCount_LST			= 0;

	if (fp) {
		while (!feof(fp)) {
			if(_fgetts(szLine, 1000, fp) == NULL) break;

			Temp = wstring_from_utf8(TCHARToANSI(szLine, NULL, 0)); // Correct way to convert from UTF-8 to TCHAR*

			TCHAR* Tokens = NULL;

			if (!_tcsncmp(_T("$default="), Temp, 9)) {

				Tokens = _tcstok(Temp, _T("="));
				
				while (Tokens != NULL) {
					if (_tcsncmp(_T("$default="), Tokens, 9)) {
						nDefaultTrack = _tcstol(Tokens, NULL, 10);
					}
					Tokens = _tcstok(NULL, _T("=,"));
				}
			} else {
				// ...
			}
			
			if (!_tcsncmp(_T("$main"), Temp, 5)) {
				inTrackList = 1;
				continue;
			} else {
				// ...
			}

			if (inTrackList) {
				if (!_tcsncmp(_T("-----"),	Temp, 5)) continue;
				if (!_tcsncmp(_T("//"),		Temp, 2)) continue;
				if (!_tcsncmp(_T(" "),		Temp, 1)) continue;
				if (!_tcsncmp(_T("$end"),	Temp, 4)) continue;
				if (!_tcsncmp(_T("\0"),		Temp, 1)) continue;
				if ((!_tcsncmp(_T("#"),	Temp, 1)) || (!_tcsncmp(_T("$"), Temp, 1))) 
				{
					unsigned int nDecOrHex = 0;

					if((0 == _tcsncmp(_T("#"), Temp, 1)) || (0 == _tcsncmp(_T("#0"), Temp, 2))) { 
						nDecOrHex = 0;
					}

					if(0 == _tcsncmp(_T("$"), Temp, 1)) {
						nDecOrHex = 1;
					}

					LvItem.iItem		= nListPos;															// current list row
					TCHAR szNumber[6];
					_stprintf(szNumber, _T("%i"), nListPos + 1);
					LvItem.iSubItem		= LV_TRACKNUMBERCOLUMN;
					LvItem.pszText		= szNumber;

					ListView_InsertItem(hTrackList_LST, &LvItem);											// Track number [Insert new row]

					TempString = Temp + 1;
					TCHAR szTrackCode[MAX_PATH];

					// -----------------------------
					TCHAR *pdest = NULL;
					TCHAR nCodeStr[5];

					TCHAR szTmpStr[MAX_PATH];
					unsigned int nCode		= 0;
					unsigned int nCh		= _T(' ');
					unsigned int nCodeLen	= 0;

					
					_tcscpy(szTmpStr, TempString);					
					pdest = _tcschr( szTmpStr, nCh );
					nCodeLen = (unsigned int)(pdest - szTmpStr + 1);

					_tcsncpy(nCodeStr, TempString, nCodeLen-1);		// copy the track code
					
					if(nDecOrHex == 0) {
						_stscanf(nCodeStr, L"%d", &nCode);			// convert the decimal string to unsigned int
					}

					if(nDecOrHex == 1) {
						_stscanf(nCodeStr, L"%x", &nCode);			// convert the hex string to unsigned int
					}

					//bprintf(0, L"nCodeLen: %d nCodeStr: %s nCode: %4.4X szTmpStr: %s \n", nCodeLen, nCodeStr, nCode, szTmpStr);	// debug

					_stprintf(szTrackCode, _T("%4.4X"), nCode);	// format the unsigned int to a proper hex string
					
					// -----------------------------

					ListView_SetItemText(hTrackList_LST, nListPos, LV_TRACKCODECOLUMN, szTrackCode);		// Sound Code [Set into new row]

					TCHAR* pszFinalTmpStr = NULL;
					pszFinalTmpStr = szTmpStr;
					pszFinalTmpStr = pszFinalTmpStr + nCodeLen;

					TempString2 = _tcsstr(pszFinalTmpStr, _T("<"));
					
					if (TempString2) {
						memset(TempString2, 0, _tcslen(TempString2));
					} else {
						TempString2 = _tcsstr(pszFinalTmpStr, _T("\n"));
						if (TempString2) memset(TempString2, 0, _tcslen(TempString2));
					}

					ListView_SetItemText(hTrackList_LST, nListPos, LV_TRACKTITLECOLUMN, pszFinalTmpStr);		// Track title [Set into new row]
			
					/******************************************************************************/
					// Insert information to .LST tracklist array

					unsigned int nSoundCode;
					_stscanf(szTrackCode, _T("%x"), &nSoundCode);	// convert TCHAR to UINT16

					Tracks_LST[nListPos].nTrack = nSoundCode;
					_stprintf(Tracks_LST[nListPos].szName, _T("%s"), pszFinalTmpStr);

					/******************************************************************************/
					nListPos++;
					nTrackCount_LST++;
				}
			} else {
				// ...
			}
		}
		fclose(fp);
	} else {
		return 0; // !fp
	}
	return 1;
}

int SaveTracklist_LST() 
{	
	int nTotalPlayListItems = ListView_GetItemCount(hTrackList_LST);
	
	if(nTotalPlayListItems > 0) 
	{
		FILE* fp = NULL;
		TCHAR szFileName[MAX_PATH];
		TCHAR szDrvName[9];

		if(isClone_LST == false) {
			_stprintf(szDrvName, _T("%s"), BurnDrvGetText(DRV_NAME));
		} else {
			_stprintf(szDrvName, _T("%s"), BurnDrvGetText(DRV_PARENT));
		}

		_stprintf(szFileName, _T("%s%s.lst"), szAppListsPath, szDrvName);

		fp = _tfopen(szFileName, _T("w"));

		if(!fp) {
			TCHAR szError[MAX_PATH];
			_stprintf(szError, _T("Couldn't create: %s"), szFileName);
			MessageBox(hJukeboxDlg, szError, _T(APP_TITLE), MB_OK);
			return 0; // error creating the tracklist file
		}

		TCHAR szDefault[4];
		ListView_GetItemText(hTrackList_LST, 0, LV_TRACKCODECOLUMN, szDefault, 5);

		TCHAR szFinalText[2048];
		char* utf8Buffer = NULL;
		
		UINT16 nDefSoundCode;
		_stscanf(szDefault, _T("%x"), &nDefSoundCode);	// convert TCHAR to UINT16

		_stprintf(szFinalText, _T("$name=%s\n$version=%i\n$default=%3.3X\n$fixed_volume=\n$main\n\n"), szDrvName, 2, nDefSoundCode);
		utf8Buffer = utf8_from_wstring(szFinalText);

		fwrite( utf8Buffer, sizeof(char), strlen(utf8Buffer), fp );

		utf8Buffer = NULL;

		for(int iIndex = 0; iIndex < nTotalPlayListItems; iIndex++) 
		{
			TCHAR szSoundCode[5];
			TCHAR szTrackTitle[1024];
			TCHAR szFinalText2[2048];

			ListView_GetItemText(hTrackList_LST, iIndex, LV_TRACKCODECOLUMN,  szSoundCode,  5);
			ListView_GetItemText(hTrackList_LST, iIndex, LV_TRACKTITLECOLUMN, szTrackTitle, 1024);

			UINT16 nSoundCode;
			_stscanf(szSoundCode, _T("%x"), &nSoundCode);	// convert TCHAR to UINT16

			_stprintf(szFinalText2, _T("#%3.3i %s\n"), nSoundCode, szTrackTitle);

			utf8Buffer = utf8_from_wstring(szFinalText2);

			fwrite( utf8Buffer, sizeof(char), strlen(utf8Buffer), fp);

		}

		utf8Buffer = NULL;

		_stprintf(szFinalText, _T("\n// List generated by %s\n$end\n"), _T("FB Alpha Jukebox"));
		utf8Buffer = utf8_from_wstring(szFinalText);

		fwrite( utf8Buffer, sizeof(char), strlen(utf8Buffer), fp );

		fclose(fp);

		TCHAR szResult[64];
		_stprintf(szResult, _T("Tracklist Saved: (%s)"), szFileName);
		MessageBox(hJukeboxDlg, szResult, _T(APP_TITLE), MB_OK);

	} else {
		MessageBox(hJukeboxDlg, _T("No items were found on the tracklist."), _T(APP_TITLE), MB_OK);
	}
	return 1;
}

int SaveTracklist_DAT()
{
	int nTotalPlayListItems = ListView_GetItemCount(hTrackList_DAT);
	
	if(nTotalPlayListItems > 0) 
	{
		FILE* fp = NULL;
		TCHAR szFileName[MAX_PATH];
		TCHAR szDrvName[9];

		if(isClone_DAT == false) {
			_stprintf(szDrvName, _T("%s"), BurnDrvGetText(DRV_NAME));
		} else {
			_stprintf(szDrvName, _T("%s"), BurnDrvGetText(DRV_PARENT));
		}

		_stprintf(szFileName, _T("%s%s.dat"), szAppDatListsPath, szDrvName);

		fp = _tfopen(szFileName, _T("w"));

		if(!fp) {
			TCHAR szError[MAX_PATH];
			_stprintf(szError, _T("Couldn't create: %s"), szFileName);
			MessageBox(hJukeboxDlg, szError, _T(APP_TITLE), MB_OK);
			return 0; // error creating the tracklist file
		}

		TCHAR szDefault[4];
		ListView_GetItemText(hTrackList_DAT, 0, LV_TRACKCODECOLUMN, szDefault, 5);

		TCHAR szFinalText[2048];
		char* utf8Buffer = NULL;
		
		UINT16 nDefSoundCode;
		_stscanf(szDefault, _T("%x"), &nDefSoundCode);	// convert TCHAR to UINT16

		_stprintf(szFinalText, _T("[Tracks]\n"));
		utf8Buffer = utf8_from_wstring(szFinalText);

		fwrite(utf8Buffer, sizeof(char), strlen(utf8Buffer), fp);

		utf8Buffer = NULL;

		for(int iIndex = 0; iIndex < nTotalPlayListItems; iIndex++) 
		{
			TCHAR szSoundCode[5];
			TCHAR szTrackTitle[1024];
			TCHAR szFinalText2[2048];

			ListView_GetItemText(hTrackList_DAT, iIndex, LV_TRACKCODECOLUMN,  szSoundCode,  5);
			ListView_GetItemText(hTrackList_DAT, iIndex, LV_TRACKTITLECOLUMN, szTrackTitle, 1024);

			UINT16 nSoundCode;
			_stscanf(szSoundCode, _T("%x"), &nSoundCode);	// convert TCHAR to UINT16

			_stprintf(szFinalText2, _T("%4.4X=%s\n"), nSoundCode, szTrackTitle);

			utf8Buffer = utf8_from_wstring(szFinalText2);

			fwrite( utf8Buffer, sizeof(char), strlen(utf8Buffer), fp);

		}

		utf8Buffer = NULL;
		fclose(fp);

		TCHAR szResult[64];
		_stprintf(szResult, _T("Tracklist Saved: (%s)"), szFileName);
		MessageBox(hJukeboxDlg, szResult, _T(APP_TITLE), MB_OK);

	} else {
		MessageBox(hJukeboxDlg, _T("No items were found on the tracklist."), _T(APP_TITLE), MB_OK);
	}
	return 1;
}

#define IS_CPS1			((BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK)==HARDWARE_CAPCOM_CPS1)
#define IS_CPS1_QSOUND	((BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK)==HARDWARE_CAPCOM_CPS1_QSOUND)
#define IS_CPS2			((BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK)==HARDWARE_CAPCOM_CPS2)
#define IS_NEOGEO		((BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK)==HARDWARE_SNK_NEOGEO)

int nSystem = 0;

// Parse the '.dat' file and get the Tracklist
int ParseTracklist() {

	if (IS_NEOGEO)					nSystem = 1;
	if (IS_CPS2 || IS_CPS1_QSOUND)	nSystem = 2;
	if (IS_CPS1)					nSystem = 3;

	FILE *fp;
	TCHAR szLine[1000];
	TCHAR tmp[8];
	TCHAR fname[MAX_PATH];
	nTrackCount_DAT = 0;
	unsigned int nLen = 0;
//	TCHAR szDATDir[MAX_PATH];

	_stprintf(fname, _T("%s%s.dat"), szAppDatListsPath, BurnDrvGetText(DRV_NAME));

	fp = _tfopen(fname, _T("r"));
	if(fp) {
		isClone_DAT = false;
	}

	if (!fp && BurnDrvGetText(DRV_PARENT)) {	
		// Try the parent
		_stprintf(fname, _T("%s%s.dat"), szAppDatListsPath, BurnDrvGetText(DRV_PARENT));
		fp = _tfopen(fname, _T("r"));
		if(fp) {
			isClone_DAT = true;
		}
	}

	if (fp == NULL) return 0; // fail

	struct trackInfo *szTracks = &Tracks_DAT[0];

	while (1) {

		if (_fgetts(szLine, 1000, fp) == NULL) break;

		_tcsncpy(szLine, wstring_from_utf8(TCHARToANSI(szLine, NULL, 0)), _tcslen(szLine) ); // Correct way to convert from UTF-8 to TCHAR*

		nLen = _tcslen(szLine);

		// Check for tracklist header
		if (nTrackCount_DAT == 0 && _tcsncmp(_T("[Tracks]"), szLine, 8)) {
			TCHAR szResult[64];
			_stprintf(szResult, _T("Invalid tracklist header: (%s)"), fname);
			MessageBox(hJukeboxDlg, szResult, _T(APP_TITLE), MB_OK);
			break; // not a valid tracklist file
		}

		if (_tcsncmp(szLine + 4, _T("="), 1) == 0) {

			// Get track numbers
			_tcsncpy(tmp, szLine, 4);

			tmp[4] = _T('\0');

			_stscanf(tmp, _T("%x"), &szTracks->nTrack);		// Sound code

			// Get track titles
			_tcsncpy(szTracks->szName, szLine + 5, nLen-6);

			szTracks->szName[nLen-6] = _T('\0');			// Track title

			szTracks++;

			szTracks->nTrack = 0;
		}
		nTrackCount_DAT++;
	}

	fclose(fp);
	szTracks = &Tracks_DAT[0];

	if (!szTracks->nTrack) return 0;
	else return 1; // success
}

#define JB_NEOGEO	1
#define JB_QSOUND	2
#define JB_CPS1		3

// Send the sound code to the Sound CPU
static void SendSoundCode() 
{
	unsigned char nSoundCode;

	if(nActivePlaylist == LST_PLAYLIST) {
		nSoundCode = Tracks_LST[nCurrentTrack_LST].nTrack;
	} else {
		nSoundCode = Tracks_DAT[nCurrentTrack_DAT].nTrack;
	}

	if (bJukeboxInUse == false) 
	{	
		switch(nSystem) {
			
			case 0:	return;

			case JB_NEOGEO:
				JukeboxSoundLatch	= nSoundCode;
				JukeboxSoundCommand = JUKEBOX_SOUND_PLAY;
				break;

			case JB_QSOUND:
				QSoundCMD(nSoundCode);
				break;

			case JB_CPS1:
				Cps1SoundStop();
				CpsSoundCmd(nSoundCode);
				break;
		}

	} else {
		if (IS_CPS1_QSOUND) {
			QSoundCMD(nSoundCode);
		} else {
			JukeboxSoundLatch	= nSoundCode;
			JukeboxSoundCommand = JUKEBOX_SOUND_PLAY;
		}
	}

	VidPaint(2); // ...Redraw the screen.
}

// Send the 'Custom' Sound code to the Sound CPU
static void SendCustomSoundCode(UINT16 nCustomTrack) 
{
	if (bJukeboxInUse == false) 
	{	
		switch(nSystem) {
			
			case 0:	return;

			case JB_NEOGEO:
				JukeboxSoundLatch	= nCustomTrack;
				JukeboxSoundCommand = JUKEBOX_SOUND_PLAY;
				break;

			case JB_QSOUND:
				QSoundCMD(nCustomTrack);
				break;

			case JB_CPS1:
				Cps1SoundStop();
				CpsSoundCmd(nCustomTrack);
				break;
		}

	} else {
		if (IS_CPS1_QSOUND) {
			QSoundCMD(nCustomTrack);
		} else {
			JukeboxSoundLatch	= nCustomTrack;
			JukeboxSoundCommand = JUKEBOX_SOUND_PLAY;
		}
	}
	VidPaint(2); // ...Redraw the screen.
}

void StopPlayback() 
{
	if (bJukeboxInUse == false) {	
		switch(nSystem) 
		{			
			case 0:	return;

			case JB_NEOGEO:
				JukeboxSoundCommand = JUKEBOX_SOUND_STOP;
				break;

			case JB_QSOUND:
				//JukeboxSoundCommand = JUKEBOX_SOUND_STOP;
				break;

			case JB_CPS1:
				Cps1SoundStop();
				break;
		}
	} else {
		JukeboxSoundCommand = JUKEBOX_SOUND_STOP;
	}
	VidPaint(2); // ...Redraw the screen.
}

// Displays the Title of the playing Track
static void DisplayTrackInfo() 
{
	if (bJukeboxInUse == false) {
		if(nActivePlaylist == LST_PLAYLIST) {
			VidSNewTinyMsg(Tracks_LST[nCurrentTrack_LST].szName, 0xFFCC00);
		} else {
			VidSNewTinyMsg(Tracks_DAT[nCurrentTrack_DAT].szName, 0xFFCC00);
		}
		RedrawWindow(hScrnWnd, NULL, NULL, 0);
		VidPaint(2);
	}
}

// Next Track
void PlayNextTrack() 
{
	if(nActivePlaylist == LST_PLAYLIST) {
		if(nCurrentTrack_LST <= nTrackCount_LST) {
			if(nCurrentTrack_LST >= 0) {
				if(nCurrentTrack_LST != nTrackCount_LST) nCurrentTrack_LST++;
			}
			SendSoundCode();
			DisplayTrackInfo();
		}
	} else {
		if(nCurrentTrack_DAT <= nTrackCount_DAT) {
			if(nCurrentTrack_DAT >= 0) {
				if(nCurrentTrack_DAT != nTrackCount_DAT) nCurrentTrack_DAT++;
			}
			SendSoundCode();
			DisplayTrackInfo();
		}
	}
}

// Previous Track
void PlayPreviousTrack() 
{
	if(nActivePlaylist == LST_PLAYLIST) {
		if(nCurrentTrack_LST >= 0) {
			if(nCurrentTrack_LST != 0) nCurrentTrack_LST--;
			SendSoundCode();
			DisplayTrackInfo();
		}
	} else {
		if(nCurrentTrack_DAT >= 0) {
			if(nCurrentTrack_DAT != 0) nCurrentTrack_DAT--;
			SendSoundCode();
			DisplayTrackInfo();
		}
	}
}

// Play current Track
void PlayCurrentTrack() 
{
	SendSoundCode();
	DisplayTrackInfo();
}

static void SetTitlePreview()
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
	
	HBITMAP hNewImage			= NULL;
	TCHAR	szFDriver[MAX_PATH]	= _T("");
	bool	bLoadPNG			= false;
	bool	bLoadZipPNG			= false;

	if (hBmp) {
		DeleteObject((HGDIOBJ)hBmp);
		hBmp = NULL;
	}
	
	do {
		// ==================================================================
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

		// ==================================================================
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

		extern bool bGameInfoOpen;
		bGameInfoOpen = true;		// just a temporary fake  flag

		char szTemp[2][MAX_PATH];		
		sprintf(szTemp[0], _TtoA(szAppTitlesPath));
		sprintf(szTemp[1], _TtoA(szFDriver));

		// Load PNG from ZIP if we can't get a handle from the directory file
		if(bLoadZipPNG) {
			hNewImage = PNGtoBMP(hJukeboxDlg, szTemp[0], szTemp[1], FBA_LM_ZIP_BUFF, 106, 106);
		} else {
			hNewImage = PNGtoBMP(hJukeboxDlg, szTemp[0], szTemp[1], FBA_LM_FILE, 106, 106);
		}

		bGameInfoOpen = false;		// done with the temprorary fake flag

		if (hNewImage) {
			DeleteObject((HGDIOBJ)hBmp);
			hBmp = hNewImage;
			if (bImageOrientation == 0) {
				SendDlgItemMessage(hJukeboxDlg, IDC_SMALL_TITLE_IMAGE_H, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBmp);
				SendDlgItemMessage(hJukeboxDlg, IDC_SMALL_TITLE_IMAGE_V, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)NULL);
				ShowWindow(GetDlgItem(hJukeboxDlg, IDC_SMALL_TITLE_IMAGE_V), SW_HIDE);
			} else {
				SendDlgItemMessage(hJukeboxDlg, IDC_SMALL_TITLE_IMAGE_H, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)NULL);
				ShowWindow(GetDlgItem(hJukeboxDlg, IDC_SMALL_TITLE_IMAGE_V), SW_SHOW);
				SendDlgItemMessage(hJukeboxDlg, IDC_SMALL_TITLE_IMAGE_V, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBmp);
			}
		}
	} else {
		SendDlgItemMessage(hJukeboxDlg, IDC_SMALL_TITLE_IMAGE_H, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)LoadBitmap(hAppInst, MAKEINTRESOURCE(BMP_MEDIUMPREVIEW)));
		SendDlgItemMessage(hJukeboxDlg, IDC_SMALL_TITLE_IMAGE_V, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)NULL);
	}
}

void UpdatePlaylistTotalTracks(int nActivePlayList) 
{
	if(nActivePlayList == DAT_PLAYLIST) {
		int nTotalPlayListItems = ListView_GetItemCount(hTrackList_DAT)-1;

		if(nTotalPlayListItems >= 0) {
			for(int nIndex = 0; nIndex < nTotalPlayListItems + 1; nIndex++) {
				TCHAR szText[5];
				if(nIndex <= 8) {
					_stprintf(szText, _T("0%i"), nIndex + 1);
				} else {
					_stprintf(szText, _T("%i"), nIndex + 1);
				}
				ListView_SetItemText(hTrackList_DAT, nIndex, LV_TRACKNUMBERCOLUMN, szText);
			}
		}
	} else {
		int nTotalPlayListItems = ListView_GetItemCount(hTrackList_LST)-1;
	
		if(nTotalPlayListItems >= 0) {
			for(int nIndex = 0; nIndex < nTotalPlayListItems + 1; nIndex++) {
				TCHAR szText[5];
				if(nIndex <= 8) {
					_stprintf(szText, _T("0%i"), nIndex + 1);
				} else {
					_stprintf(szText, _T("%i"), nIndex + 1);
				}
				ListView_SetItemText(hTrackList_LST, nIndex, LV_TRACKNUMBERCOLUMN, szText);
			}
		}
	}
}

void FillTracklistDAT() {
	// Fill the .DAT ListBox with the tracklist
	LV_ITEM LvItem;
	memset(&LvItem, 0, sizeof(LvItem));
	LvItem.mask			= LVIF_TEXT;
	LvItem.cchTextMax	= 256;

	for(int nIndex = 0; nIndex < ((nTrackCount_DAT)-1); nIndex++) 
	{
		LvItem.iItem		= nIndex;		// current list row

		TCHAR szNumber[6];
		_stprintf(szNumber, _T("%i"), nIndex + 1);
		LvItem.iSubItem		= LV_TRACKNUMBERCOLUMN;
		LvItem.pszText		= szNumber;
		ListView_InsertItem(hTrackList_DAT, &LvItem);													// Track number [Insert new row]

		TCHAR szTrackCode[4];
		_stprintf(szTrackCode, _T("%4.4X"), Tracks_DAT[nIndex].nTrack);
		ListView_SetItemText(hTrackList_DAT, nIndex, LV_TRACKCODECOLUMN, szTrackCode);					// Sound Code [Set into new row]

		ListView_SetItemText(hTrackList_DAT, nIndex, LV_TRACKTITLECOLUMN, Tracks_DAT[nIndex].szName);		// Track title [Set into new row]
	}
}

static void InsertJukeTabs() 
{
	TC_ITEM TCI; 
	TCI.mask = TCIF_TEXT;
	TCI.pszText = _T(".LST Tracklist");
	SendMessage(hTracklistTabs, TCM_INSERTITEM, (WPARAM) 0, (LPARAM) &TCI); 
	TCI.pszText = _T(".DAT Tracklist");
	SendMessage(hTracklistTabs, TCM_INSERTITEM, (WPARAM) 1, (LPARAM) &TCI);
}

static void DisplayControls(int TabSelected) 
{
	switch(TabSelected) {
		// .LST Tracklist
		case 0:
			ShowWindow(hTrackList_DAT, SW_HIDE);	
			AnimateWindow(hTrackList_LST, 400, AW_HOR_POSITIVE | SW_SHOW);
			//ShowWindow(hTrackList_LST, SW_SHOW);
			break;

		// .DAT Tracklist
		case 1:
			ShowWindow(hTrackList_LST, SW_HIDE);
			AnimateWindow(hTrackList_DAT, 400, AW_HOR_POSITIVE | SW_SHOW);
			//ShowWindow(hTrackList_DAT, SW_SHOW);
			break;
	}	
}

void DynamicSelectionScroll(HWND hControl, int nIndex)
{
	// Dynamic ListView scrolling
	POINT selPoint;
	RECT selRect;
	ListView_GetItemPosition(hControl, nIndex, &selPoint);
	ListView_GetItemRect(hControl, nIndex, &selRect, LVIR_BOUNDS);
	ListView_Scroll(hControl, selPoint.x, selPoint.y - (selRect.bottom - selRect.top));
}

static int JukeboxDialogInit()
{
	// Get the games full name
	TCHAR szFullName[1024];
	TCHAR szText[1024];
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
	_stprintf(szItemText, _T("%s (%s, %s hardware)"), BurnDrvGetTextA(DRV_MANUFACTURER) ? BurnDrvGetText(DRV_MANUFACTURER) : _T("unknown"), BurnDrvGetText(DRV_DATE), ((BurnDrvGetHardwareCode() & HARDWARE_SNK_MVSCARTRIDGE) == HARDWARE_SNK_MVSCARTRIDGE) ? _T("Neo Geo MVS Cartidge") : BurnDrvGetText(DRV_SYSTEM));
	SendMessage(hInfoControl, WM_SETTEXT, (WPARAM)0, (LPARAM)szItemText);
	
	// Display any comments
	szItemText[0] = _T('\0');
	hInfoControl = GetDlgItem(hJukeboxDlg, IDC_TEXTNOTES);
	_stprintf(szItemText, _T("%s"), BurnDrvGetTextA(DRV_COMMENT) ? BurnDrvGetText(DRV_COMMENT) : _T(""));
	SendMessage(hInfoControl, WM_SETTEXT, (WPARAM)0, (LPARAM)szItemText);
	
	// Display the genre
	szItemText[0] = _T('\0');
	hInfoControl = GetDlgItem(hJukeboxDlg, IDC_TEXTGENRE);
	_stprintf(szItemText, _T("%s"), DecorateGenreInfo());
	SendMessage(hInfoControl, WM_SETTEXT, (WPARAM)0, (LPARAM)szItemText);

	// Make a white brush
	hWhiteBGBrush = CreateSolidBrush(RGB(0xFF,0xFF,0xFF));

	InsertJukeTabs();								// Insert tabs
	SetupListViewColumns();							// Instert columns in ListView for .LST and .DAT tracklists

	if(ParseTracklist()) {							// Parse .DAT tracklist
		FillTracklistDAT();							// Fill .DAT ListBox with the parsed info
		UpdatePlaylistTotalTracks(DAT_PLAYLIST);	// Update .DAT ListBox with numbers of tracks at the left
	}	
	if(ParseM1Tracklist()) {						// Parse .LST tracklist
		UpdatePlaylistTotalTracks(LST_PLAYLIST);
	}

	// Set the title preview image
	SetTitlePreview();

	return 0;
}

void SetBgImage(HBITMAP hBgBmp) {
	LVBKIMAGE LvBgImg;
	memset(&LvBgImg, 0, sizeof(LvBgImg));
	LvBgImg.ulFlags = LVBKIF_SOURCE_HBITMAP | LVBKIF_STYLE_TILE;
	LvBgImg.hbm		= hBgBmp;
	ListView_SetBkImage(hTrackList_LST, &LvBgImg);
}

static BOOL CALLBACK DialogProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam) {

	if (Msg == WM_INITDIALOG) 
	{
		EnableWindow(hScrnWnd, false);

		hJukeboxDlg			= hDlg;
		hTrackList_DAT		= GetDlgItem(hDlg, IDC_JUKEBOX_TRACKLIST_DAT);
		hPreview			= LoadBitmap(hAppInst, MAKEINTRESOURCE(BMP_SMALLPREVIEW));
		hTracklistTabs		= GetDlgItem(hDlg, IDC_JUKE_TAB);													// .LST / .DAT Playlist tabs control
		hTrackList_LST		= GetDlgItem(hDlg, IDC_JUKEBOX_TRACKLIST);											// Jukebox ListView for .LST

		SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)LoadIcon(hAppInst, MAKEINTRESOURCE(IDI_MPLAYER)));	// set the jukebox dialog icon
		SendMessage(GetDlgItem(hJukeboxDlg,IDC_VOLUME),TBM_SETPOS,TRUE,nAudVolume/100);							// set Volume slider position

		WndInMid(hDlg, hScrnWnd);						// Center the Dialog
		RedrawWindow(hDlg, NULL, NULL, 0);
		ShowWindow(hDlg, SW_SHOWNORMAL);
		ShowWindow(hTrackList_DAT, SW_HIDE);			// Hide Secondary playlist for .DAT

		// Start loading button and their icons, etc...
		int idcButtonID[13]			 = {	IDC_TLPLAY,			IDC_TLSTOP,			IDC_TLPAUSE,		IDC_TLNEXT,			IDC_TLPREV,				IDC_TLFIRST,		IDC_TLLAST,			IDC_TLRELOAD,		IDC_TLADD,		IDC_TLREMOVE,		IDC_CT_PLUS,		IDC_CT_MIN		, IDC_SAVE_TRACKLIST_LST	};
		int idcButtonIconID[13]		 = {	IDI_JUKE_PLAY,		IDI_JUKE_STOP,		IDI_JUKE_PAUSE,		IDI_JUKE_NEXT,		IDI_JUKE_PREVIOUS,		IDI_JUKE_FIRST,		IDI_JUKE_LAST,		IDI_JUKE_RELOAD,	IDI_JUKE_ADD,	IDI_JUKE_REMOVE,	IDI_JUKE_PLUS,		IDI_JUKE_MIN	, IDI_JUKE_SAVE				};
		int idcButtonHoverIconID[13] = {	IDI_JUKE_PLAY_H,	IDI_JUKE_STOP_H,	IDI_JUKE_PAUSE_H,	IDI_JUKE_NEXT_H,	IDI_JUKE_PREVIOUS_H,	IDI_JUKE_FIRST_H,	IDI_JUKE_LAST_H,	IDI_JUKE_RELOAD_H,	IDI_JUKE_ADD_H,	IDI_JUKE_REMOVE_H,	IDI_JUKE_PLUS_H,	IDI_JUKE_MIN_H	, IDI_JUKE_SAVE_H			};
		
		ImageButton_EnableXPThemes();

		/* no icons for these 3 at this moment */
		//ImageButton_Create(hDlg, IDC_JUKEBOX_LOG_WAV_START);
		//ImageButton_Create(hDlg, IDC_JUKEBOX_LOG_WAV_STOP);
		//ImageButton_Create(hDlg, IDC_JUKEBOX_LOWPASSFILTER);

		EnableWindow(GetDlgItem(hDlg, IDC_JUKEBOX_LOG_WAV_STOP), false);
		
		for(int x = 0; x < 13; x++)
		{
			ImageButton_Create(hDlg, idcButtonID[x]);

			if(idcButtonID[x] == IDC_CT_PLUS || idcButtonID[x] == IDC_CT_MIN) {
				ImageButton_SetIcon(GetDlgItem(hDlg, idcButtonID[x]), idcButtonIconID[x], 0, idcButtonHoverIconID[x], 20, 18);
			} else if(idcButtonID[x] == IDC_TLADD || idcButtonID[x] == IDC_TLREMOVE) {
				ImageButton_SetIcon(GetDlgItem(hDlg, idcButtonID[x]), idcButtonIconID[x], 0, idcButtonHoverIconID[x], 18, 18);
			} else {
				ImageButton_SetIcon(GetDlgItem(hDlg, idcButtonID[x]), idcButtonIconID[x], 0, idcButtonHoverIconID[x], 36, 32);
			}
		}

		JukeboxDialogInit();

		Edit_LimitText(GetDlgItem(hJukeboxDlg, IDC_CUSTOMTRACK), 4); // Limit sound code text input

		//SetBgImage(hPreview);		// Set a background image (test feature)
		//COLORREF colorBlack		= RGB( 0x00, 0x00, 0x00 );
		//COLORREF colorWhite		= RGB( 0xFF, 0xFF, 0xFF );
		//COLORREF colorYellow		= RGB( 0xFF, 0xCC, 0x00 );
		//ListView_SetTextBkColor(hTrackList_LST, colorBlack);		// Set tracklist background color
		//ListView_SetTextColor(hTrackList_LST, colorYellow);		// Set tracklist text color

		ListView_SetExtendedListViewStyle(hTrackList_LST, LVS_EX_BORDERSELECT | LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER );		
		
		//ListView_SetOutlineColor(hTrackList_LST, colorYellow);	// Set tracklist selection border
		int nSel = nTrackCount_LST;

		if(nSel < 0) {
			SetDlgItemText(hJukeboxDlg, IDC_CUSTOMTRACK, _T("0000"));
			SetDlgItemText(hJukeboxDlg, IDC_CUSTOMTITLE, _T("- Enter track title here -"));
		} else {
			nCurrentTrack_LST = 0;
			ListView_SetItemState(hTrackList_LST, nCurrentTrack_LST, LVIS_SELECTED | LVIS_FOCUSED, 0x000F);	// Select the first item on the tracklist
			TCHAR szSoundCode[5];
			_stprintf(szSoundCode, _T("%4.4X"), Tracks_LST[nCurrentTrack_LST].nTrack);
			SetDlgItemText(hJukeboxDlg, IDC_CUSTOMTRACK, szSoundCode);
			SetDlgItemText(hJukeboxDlg, IDC_CUSTOMTITLE, Tracks_LST[nCurrentTrack_LST].szName);
		}
		return TRUE;
	}

	if(Msg == WM_CTLCOLORSTATIC) 
	{
		int idcID[12] = {IDC_LABELCOMMENT,IDC_LABELROMNAME,IDC_LABELROMINFO,IDC_LABELSYSTEM,IDC_LABELNOTES,IDC_LABELGENRE,IDC_TEXTCOMMENT,IDC_TEXTROMNAME,IDC_TEXTROMINFO,IDC_TEXTSYSTEM,IDC_TEXTNOTES, IDC_TEXTGENRE};
		for(int x = 0; x < 12; x++) {
			if ((HWND)lParam == GetDlgItem(hDlg, idcID[x]))	return (BOOL)hWhiteBGBrush;
		}
	}

	NMHDR* pNmHdr = (NMHDR*)lParam;

	if(Msg == WM_NOTIFY)
	{
		// Tabs changed
		if (pNmHdr->code == TCN_SELCHANGE) 
		{
			int nTab = SendMessage(hTracklistTabs, TCM_GETCURSEL, 0, 0);

			// .LST TAB
			if(nTab == 0) {
				DisplayControls(0);
				nActivePlaylist = LST_PLAYLIST;
			}	
			// .DAT TAB
			if(nTab == 1) {
				DisplayControls(1);
				nActivePlaylist = DAT_PLAYLIST;
				if(nCurrentTrack_DAT != -1) {
					ListView_SetItemState(hTrackList_DAT, nCurrentTrack_DAT, LVIS_SELECTED | LVIS_FOCUSED, 0x000F);
				} else {
					nCurrentTrack_DAT = 0;
					ListView_SetItemState(hTrackList_DAT, nCurrentTrack_DAT, LVIS_SELECTED | LVIS_FOCUSED, 0x000F);
				}
			}			
			return FALSE;
		}


		int Id				= LOWORD(wParam);
		NMLISTVIEW* pnmv	= (NMLISTVIEW*)lParam;
		NMHDR pnm			= pnmv->hdr;

		if (pnm.code == LVN_ITEMCHANGED) 
		{
			switch(Id)
			{
				case IDC_JUKEBOX_TRACKLIST:
				{
					int iCount		= SendMessage(hTrackList_LST, LVM_GETITEMCOUNT, 0, 0);
					int iSelCount	= SendMessage(hTrackList_LST, LVM_GETSELECTEDCOUNT, 0, 0);

					if(iCount == 0 || iSelCount == 0) return 1;

					int iItem = pnmv->iItem; // simple way of getting the 'iItem'

					nCurrentTrack_LST = iItem;

					TCHAR szSoundCode[5];
					_stprintf(szSoundCode, _T("%4.4X"), Tracks_LST[nCurrentTrack_LST].nTrack);
					SetDlgItemText(hJukeboxDlg, IDC_CUSTOMTRACK, szSoundCode);
					SetDlgItemText(hJukeboxDlg, IDC_CUSTOMTITLE, Tracks_LST[nCurrentTrack_LST].szName);

					ListView_RedrawItems(hTrackList_LST, 0, ListView_GetItemCount(hTrackList_LST));		// Redraw everything on the list, including background image
				}
				break;

				case IDC_JUKEBOX_TRACKLIST_DAT:
				{
					int iCount		= SendMessage(hTrackList_DAT, LVM_GETITEMCOUNT, 0, 0);
					int iSelCount	= SendMessage(hTrackList_DAT, LVM_GETSELECTEDCOUNT, 0, 0);

					if(iCount == 0 || iSelCount == 0) return 1;

					int iItem = pnmv->iItem; // simple way of getting the 'iItem'

					nCurrentTrack_DAT = iItem;

					TCHAR szSoundCode[5];
					_stprintf(szSoundCode, _T("%4.4X"), Tracks_DAT[nCurrentTrack_DAT].nTrack);
					SetDlgItemText(hJukeboxDlg, IDC_CUSTOMTRACK, szSoundCode);
					SetDlgItemText(hJukeboxDlg, IDC_CUSTOMTITLE, Tracks_DAT[nCurrentTrack_DAT].szName);

					ListView_RedrawItems(hTrackList_DAT, 0, ListView_GetItemCount(hTrackList_DAT));		// Redraw everything on the list, including background image
				}
				break;
			}
		}

		if (pnm.code == NM_DBLCLK || pnm.code == NM_RDBLCLK) 
		{
			switch(Id)
			{
				case IDC_JUKEBOX_TRACKLIST: 
				{
					int iCount		= SendMessage(hTrackList_LST, LVM_GETITEMCOUNT, 0, 0);
					int iSelCount	= SendMessage(hTrackList_LST, LVM_GETSELECTEDCOUNT, 0, 0);

					if(iCount == 0 || iSelCount == 0) return 1;

					int iItem = pnmv->iItem; // simple way of getting the 'iItem'

					nCurrentTrack_LST = iItem;

					PlayCurrentTrack();
					TCHAR szSoundCode[5];
					_stprintf(szSoundCode, _T("%4.4X"), Tracks_LST[nCurrentTrack_LST].nTrack);
					SetDlgItemText(hJukeboxDlg, IDC_CUSTOMTRACK, szSoundCode);
					SetDlgItemText(hJukeboxDlg, IDC_CUSTOMTITLE, Tracks_LST[nCurrentTrack_LST].szName);
				}
				break;

				case IDC_JUKEBOX_TRACKLIST_DAT: 
				{
					int iCount		= SendMessage(hTrackList_DAT, LVM_GETITEMCOUNT, 0, 0);
					int iSelCount	= SendMessage(hTrackList_DAT, LVM_GETSELECTEDCOUNT, 0, 0);

					if(iCount == 0 || iSelCount == 0) return 1;

					int iItem = pnmv->iItem; // simple way of getting the 'iItem'

					nCurrentTrack_DAT = iItem;

					PlayCurrentTrack();
					TCHAR szSoundCode[5];
					_stprintf(szSoundCode, _T("%4.4X"), Tracks_DAT[nCurrentTrack_DAT].nTrack);
					SetDlgItemText(hJukeboxDlg, IDC_CUSTOMTRACK, szSoundCode);
					SetDlgItemText(hJukeboxDlg, IDC_CUSTOMTITLE, Tracks_DAT[nCurrentTrack_DAT].szName);
				}
				break;
			}

			return 0;
		}
	}

	if(Msg == WM_HSCROLL) // VOLUME TRACKBAR
	{
		nAudVolume = (SendMessage(GetDlgItem(hJukeboxDlg,IDC_VOLUME),TBM_GETPOS,0,0)) * 100;		// (Max. Volume = 10000)
		if (AudSoundSetVolume() == 0) {
			VidSNewShortMsg(FBALoadStringEx(hAppInst, IDS_SOUND_NOVOLUME, true));
		} else {
			TCHAR buffer[15];
			_stprintf(buffer, FBALoadStringEx(hAppInst, IDS_SOUND_VOLUMESET, true), nAudVolume / 100);
			VidSNewShortMsg(buffer);
		}
		VidPaint(2); // ...Redraw the screen.
	}

	if (Msg == WM_COMMAND) 
	{
		if (HIWORD(wParam) == BN_CLICKED) 
		{
			int wID = LOWORD(wParam);

			switch (wID) 
			{
				// CHANGE SOUND CODE VALUE 'PLUS' ONE
				case IDC_CT_PLUS:
				{
					TCHAR nszCustomTrack[5];
					GetDlgItemText(hJukeboxDlg,IDC_CUSTOMTRACK,nszCustomTrack,5);

					if(nszCustomTrack) {
						UINT16 nCustomTrack;
						_stscanf(nszCustomTrack, _T("%x"), &nCustomTrack);
						nCustomTrack++;
						_stprintf(nszCustomTrack, _T("%4.4X"), nCustomTrack);
						SendDlgItemMessage(hJukeboxDlg, IDC_CUSTOMTRACK, WM_SETTEXT, 0, (LPARAM)nszCustomTrack);
						SendCustomSoundCode(nCustomTrack);
					} 
					if(nActivePlaylist == LST_PLAYLIST) SendMessage(hDlg, WM_NEXTDLGCTL, (WPARAM)hTrackList_LST, TRUE);
					if(nActivePlaylist == DAT_PLAYLIST) SendMessage(hDlg, WM_NEXTDLGCTL, (WPARAM)hTrackList_DAT, TRUE);
				}
				break;

				// CHANGE SOUND CODE VALUE 'MINUS' ONE
				case IDC_CT_MIN:
				{
					TCHAR nszCustomTrack[5];
					GetDlgItemText(hJukeboxDlg,IDC_CUSTOMTRACK,nszCustomTrack,5);

					if(nszCustomTrack) {
						UINT16 nCustomTrack;

						_stscanf(nszCustomTrack, _T("%x"), &nCustomTrack);
						nCustomTrack--;
						_stprintf(nszCustomTrack, _T("%4.4X"), nCustomTrack);
						SendDlgItemMessage(hJukeboxDlg, IDC_CUSTOMTRACK, WM_SETTEXT, 0, (LPARAM)nszCustomTrack);
						SendCustomSoundCode(nCustomTrack);
					}
					if(nActivePlaylist == LST_PLAYLIST) SendMessage(hDlg, WM_NEXTDLGCTL, (WPARAM)hTrackList_LST, TRUE);
					if(nActivePlaylist == DAT_PLAYLIST) SendMessage(hDlg, WM_NEXTDLGCTL, (WPARAM)hTrackList_DAT, TRUE);
				}
				break;

				// ADD SPECIFIED CUSTOM TRACK TO PLAYLIST
				case IDC_TLADD:
				{
					if(nActivePlaylist == LST_PLAYLIST) {
						TCHAR nszCustomTrack[5];
						TCHAR nszCustomTitle[256];
						GetDlgItemText(hJukeboxDlg,IDC_CUSTOMTRACK,nszCustomTrack, 5);
						GetDlgItemText(hJukeboxDlg,IDC_CUSTOMTITLE,nszCustomTitle, 256);

						int nNewItemPos = ListView_GetItemCount(hTrackList_LST); // new item go after everything else

						LV_ITEM LvItem;
						memset(&LvItem, 0, sizeof(LvItem));
						LvItem.mask			= LVIF_TEXT;
						LvItem.iItem		= nNewItemPos;				// new item index
						
						TCHAR szNumber[6];
						_stprintf(szNumber, _T("%i"), nNewItemPos + 1);

						LvItem.cchTextMax	= 6;
						LvItem.iSubItem		= LV_TRACKNUMBERCOLUMN;
						LvItem.pszText		= szNumber;

						ListView_InsertItem(hTrackList_LST, &LvItem);												// Track number

						LvItem.cchTextMax	= 5;
						LvItem.iSubItem		= LV_TRACKCODECOLUMN;
						LvItem.pszText		= nszCustomTrack;
						
						ListView_SetItem(hTrackList_LST, &LvItem);

						LvItem.cchTextMax	= 256;
						LvItem.iSubItem		= LV_TRACKTITLECOLUMN;
						LvItem.pszText		= nszCustomTitle;

						ListView_SetItem(hTrackList_LST, &LvItem);					
						
						UpdatePlaylistTotalTracks(LST_PLAYLIST);						
						DynamicSelectionScroll(hTrackList_LST, nNewItemPos);

						ListView_SetItemState(hTrackList_LST, nNewItemPos, LVIS_SELECTED | LVIS_FOCUSED, 0x000F);						
						
						TCHAR pszCustomTrack[5];
						TCHAR pszSelectedTitle[256];			
						ListView_GetItemText(hTrackList_LST, nNewItemPos, LV_TRACKCODECOLUMN, pszCustomTrack, 5);
						ListView_GetItemText(hTrackList_LST, nNewItemPos, LV_TRACKTITLECOLUMN, pszSelectedTitle, 256);
						SetDlgItemText(hJukeboxDlg, IDC_CUSTOMTRACK, pszCustomTrack);
						SetDlgItemText(hJukeboxDlg, IDC_CUSTOMTITLE, pszSelectedTitle);

						nTrackCount_LST = ListView_GetItemCount(hTrackList_LST)-1;							// get new total of tracks on the playlist
						_stscanf(pszCustomTrack, _T("%X"), &Tracks_LST[nTrackCount_LST].nTrack);			// insert the new sound code to a the previous sound codes array so it can be used later
						_stprintf(Tracks_LST[nTrackCount_LST].szName, _T("%s"), pszSelectedTitle);			// insert the new title as well so it can be used later

						nCurrentTrack_LST = nNewItemPos;

						SendMessage(hDlg, WM_NEXTDLGCTL, (WPARAM)hTrackList_LST, TRUE);
					} else {
						TCHAR nszCustomTrack[5];
						TCHAR nszCustomTitle[256];
						GetDlgItemText(hJukeboxDlg,IDC_CUSTOMTRACK,nszCustomTrack, 5);
						GetDlgItemText(hJukeboxDlg,IDC_CUSTOMTITLE,nszCustomTitle, 256);

						int nNewItemPos = ListView_GetItemCount(hTrackList_DAT); // new item go after everything else

						LV_ITEM LvItem;
						memset(&LvItem, 0, sizeof(LvItem));
						LvItem.mask			= LVIF_TEXT;
						LvItem.iItem		= nNewItemPos;				// new item index
						
						TCHAR szNumber[6];
						_stprintf(szNumber, _T("%i"), nNewItemPos + 1);

						LvItem.cchTextMax	= 6;
						LvItem.iSubItem		= LV_TRACKNUMBERCOLUMN;
						LvItem.pszText		= szNumber;

						ListView_InsertItem(hTrackList_DAT, &LvItem);												// Track number

						LvItem.cchTextMax	= 5;
						LvItem.iSubItem		= LV_TRACKCODECOLUMN;
						LvItem.pszText		= nszCustomTrack;
						
						ListView_SetItem(hTrackList_DAT, &LvItem);

						LvItem.cchTextMax	= 256;
						LvItem.iSubItem		= LV_TRACKTITLECOLUMN;
						LvItem.pszText		= nszCustomTitle;

						ListView_SetItem(hTrackList_DAT, &LvItem);					
						
						UpdatePlaylistTotalTracks(DAT_PLAYLIST);						
						DynamicSelectionScroll(hTrackList_DAT, nNewItemPos);

						ListView_SetItemState(hTrackList_DAT, nNewItemPos, LVIS_SELECTED | LVIS_FOCUSED, 0x000F);						
						
						TCHAR pszCustomTrack[5];
						TCHAR pszSelectedTitle[256];			
						ListView_GetItemText(hTrackList_DAT, nNewItemPos, LV_TRACKCODECOLUMN, pszCustomTrack, 5);
						ListView_GetItemText(hTrackList_DAT, nNewItemPos, LV_TRACKTITLECOLUMN, pszSelectedTitle, 256);
						SetDlgItemText(hJukeboxDlg, IDC_CUSTOMTRACK, pszCustomTrack);
						SetDlgItemText(hJukeboxDlg, IDC_CUSTOMTITLE, pszSelectedTitle);

						nTrackCount_DAT = ListView_GetItemCount(hTrackList_DAT)-1;							// get new total of tracks on the playlist
						_stscanf(pszCustomTrack, _T("%X"), &Tracks_DAT[nTrackCount_DAT].nTrack);			// insert the new sound code to a the previous sound codes array so it can be used later
						_stprintf(Tracks_DAT[nTrackCount_DAT].szName, _T("%s"), pszSelectedTitle);			// insert the new title as well so it can be used later

						nCurrentTrack_DAT = nNewItemPos;

						SendMessage(hDlg, WM_NEXTDLGCTL, (WPARAM)hTrackList_DAT, TRUE);
					}
				}
				break;

				// REMOVE SELECTED TRACK FROM LIST
				case IDC_TLREMOVE:
				{
					if(nActivePlaylist == LST_PLAYLIST) {
						int iStart				= -1;
						int nArrayShiftPointer	= ListView_GetNextItem(hTrackList_LST, iStart, LVNI_SELECTED); // this is from where I need to start the rotation
						
						// Clean the array position please since there is no delete function >___>
//						free(Tracks_LST[nArrayShiftPointer].szName);
						Tracks_LST[nArrayShiftPointer].nTrack = 0;

						// shift everything please...
						for(int nShiftRange = nArrayShiftPointer; nShiftRange < nTrackCount_LST + 1; nShiftRange++) {
							Tracks_LST[nShiftRange].nTrack = Tracks_LST[nShiftRange+1].nTrack;
							_stprintf(Tracks_LST[nShiftRange].szName, _T("%s"), Tracks_LST[nShiftRange+1].szName); 
						}

						int nSelCheck = ListView_GetItemCount(hTrackList_LST);

						if(nSelCheck <= 0) {
							SetDlgItemText(hJukeboxDlg, IDC_CUSTOMTRACK, _T("0000"));
							SetDlgItemText(hJukeboxDlg, IDC_CUSTOMTITLE, _T("- Enter track title here -"));
							break;
						}

						int nSel		= ListView_GetNextItem(hTrackList_LST, iStart, LVNI_SELECTED);	
						int nNewSel		= nSel - 1;

						ListView_DeleteItem(hTrackList_LST, nSel);
						nTrackCount_LST = ListView_GetItemCount(hTrackList_LST)-1;			// get new total of tracks on the playlist	
						UpdatePlaylistTotalTracks(LST_PLAYLIST);

						if(nNewSel < 0) nNewSel = 0;
						DynamicSelectionScroll(hTrackList_LST, nNewSel);
						ListView_SetItemState(hTrackList_LST, nNewSel, LVIS_SELECTED | LVIS_FOCUSED, 0x000F);

						int nSelSecondCheck = ListView_GetItemCount(hTrackList_LST);

						if(nSelSecondCheck <= 0) {
							SetDlgItemText(hJukeboxDlg, IDC_CUSTOMTRACK, _T("0000"));
							SetDlgItemText(hJukeboxDlg, IDC_CUSTOMTITLE, _T("- Enter track title here -"));
							break;
						}

						nCurrentTrack_LST = nNewSel;

						TCHAR szSoundCode[5];
						_stprintf(szSoundCode, _T("%4.4X"), Tracks_LST[nCurrentTrack_LST].nTrack);
						SendDlgItemMessage(hJukeboxDlg, IDC_CUSTOMTRACK, WM_SETTEXT, 0, (LPARAM)szSoundCode);
						SendDlgItemMessage(hJukeboxDlg, IDC_CUSTOMTITLE, WM_SETTEXT, 0, (LPARAM)Tracks_LST[nCurrentTrack_LST].szName);

						SendMessage(hDlg, WM_NEXTDLGCTL, (WPARAM)hTrackList_LST, TRUE);
					} else {
						int iStart				= -1;
						int nArrayShiftPointer	= ListView_GetNextItem(hTrackList_DAT, iStart, LVNI_SELECTED); // this is from where I need to start the rotation
						
						// Clean the array position please since there is no delete function >___>
//						free(Tracks_DAT[nArrayShiftPointer].szName);
						Tracks_DAT[nArrayShiftPointer].nTrack = 0;

						// shift everything please...
						for(int nShiftRange = nArrayShiftPointer; nShiftRange < nTrackCount_DAT + 1; nShiftRange++) {
							Tracks_DAT[nShiftRange].nTrack = Tracks_DAT[nShiftRange+1].nTrack;
							_stprintf(Tracks_DAT[nShiftRange].szName, _T("%s"), Tracks_DAT[nShiftRange+1].szName); 
						}

						int nSelCheck = ListView_GetItemCount(hTrackList_DAT);

						if(nSelCheck <= 0) {
							SetDlgItemText(hJukeboxDlg, IDC_CUSTOMTRACK, _T("0000"));
							SetDlgItemText(hJukeboxDlg, IDC_CUSTOMTITLE, _T("- Enter track title here -"));
							break;
						}

						int nSel		= ListView_GetNextItem(hTrackList_DAT, iStart, LVNI_SELECTED);	
						int nNewSel		= nSel - 1;

						ListView_DeleteItem(hTrackList_DAT, nSel);
						nTrackCount_DAT = ListView_GetItemCount(hTrackList_DAT)-1;			// get new total of tracks on the playlist	
						UpdatePlaylistTotalTracks(DAT_PLAYLIST);

						if(nNewSel < 0) nNewSel = 0;
						DynamicSelectionScroll(hTrackList_DAT, nNewSel);
						ListView_SetItemState(hTrackList_DAT, nNewSel, LVIS_SELECTED | LVIS_FOCUSED, 0x000F);

						int nSelSecondCheck = ListView_GetItemCount(hTrackList_DAT);

						if(nSelSecondCheck <= 0) {
							SetDlgItemText(hJukeboxDlg, IDC_CUSTOMTRACK, _T("0000"));
							SetDlgItemText(hJukeboxDlg, IDC_CUSTOMTITLE, _T("- Enter track title here -"));
							break;
						}
						nCurrentTrack_DAT = nNewSel;

						TCHAR szSoundCode[5];
						_stprintf(szSoundCode, _T("%4.4X"), Tracks_DAT[nCurrentTrack_DAT].nTrack);
						SendDlgItemMessage(hJukeboxDlg, IDC_CUSTOMTRACK, WM_SETTEXT, 0, (LPARAM)szSoundCode);
						SendDlgItemMessage(hJukeboxDlg, IDC_CUSTOMTITLE, WM_SETTEXT, 0, (LPARAM)Tracks_DAT[nCurrentTrack_DAT].szName);

						SendMessage(hDlg, WM_NEXTDLGCTL, (WPARAM)hTrackList_DAT, TRUE);
					}
				}
				break;

				// STOP PLAYBACK
				case IDC_TLSTOP:
				{
					if(bRunPause) bRunPause = 0;
					StopPlayback();
					if(nActivePlaylist == LST_PLAYLIST) SendMessage(hDlg, WM_NEXTDLGCTL, (WPARAM)hTrackList_LST, TRUE);
					if(nActivePlaylist == DAT_PLAYLIST) SendMessage(hDlg, WM_NEXTDLGCTL, (WPARAM)hTrackList_DAT, TRUE);
				}
				break;

				// PLAY SPECIFIED TRACK VALUE
				case IDC_TLPLAY:
				{					
					if(bRunPause) { 
						bRunPause = 0;
						break;
					}
					if(nActivePlaylist == LST_PLAYLIST) {
						PlayCurrentTrack();
						DynamicSelectionScroll(hTrackList_LST, nCurrentTrack_LST);
						ListView_SetItemState(hTrackList_LST, nCurrentTrack_LST, LVIS_FOCUSED | LVIS_SELECTED, 0x000f);
						TCHAR szSoundCode[5];
						_stprintf(szSoundCode, _T("%4.4X"), Tracks_LST[nCurrentTrack_LST].nTrack);
						SendDlgItemMessage(hJukeboxDlg, IDC_CUSTOMTRACK, WM_SETTEXT, 0, (LPARAM)szSoundCode);
						SendDlgItemMessage(hJukeboxDlg, IDC_CUSTOMTITLE, WM_SETTEXT, 0, (LPARAM)Tracks_LST[nCurrentTrack_LST].szName);
						SendMessage(hDlg, WM_NEXTDLGCTL, (WPARAM)hTrackList_LST, TRUE);
					} else {
						PlayCurrentTrack();
						DynamicSelectionScroll(hTrackList_DAT, nCurrentTrack_DAT);
						ListView_SetItemState(hTrackList_DAT, nCurrentTrack_DAT, LVIS_FOCUSED | LVIS_SELECTED, 0x000f);
						TCHAR szSoundCode[5];
						_stprintf(szSoundCode, _T("%4.4X"), Tracks_DAT[nCurrentTrack_DAT].nTrack);
						SendDlgItemMessage(hJukeboxDlg, IDC_CUSTOMTRACK, WM_SETTEXT, 0, (LPARAM)szSoundCode);
						SendDlgItemMessage(hJukeboxDlg, IDC_CUSTOMTITLE, WM_SETTEXT, 0, (LPARAM)Tracks_DAT[nCurrentTrack_DAT].szName);
						SendMessage(hDlg, WM_NEXTDLGCTL, (WPARAM)hTrackList_DAT, TRUE);
					}
				}
				break;

				// PAUSE PLAYBACK (EMULATION)
				case IDC_TLPAUSE:
				{
					AudBlankSound();
					bRunPause = bRunPause ? 0 : 1;

					if(nActivePlaylist == LST_PLAYLIST) SendMessage(hDlg, WM_NEXTDLGCTL, (WPARAM)hTrackList_LST, TRUE);
					if(nActivePlaylist == DAT_PLAYLIST) SendMessage(hDlg, WM_NEXTDLGCTL, (WPARAM)hTrackList_DAT, TRUE);
				}
				break;

				// PLAY NEXT TRACK
				case IDC_TLNEXT:
				{
					if(nActivePlaylist == LST_PLAYLIST) {
						PlayNextTrack();
						DynamicSelectionScroll(hTrackList_LST, nCurrentTrack_LST);
						ListView_SetItemState(hTrackList_LST, nCurrentTrack_LST, LVIS_FOCUSED | LVIS_SELECTED, 0x000f);
						TCHAR szSoundCode[5];
						_stprintf(szSoundCode, _T("%4.4X"), Tracks_LST[nCurrentTrack_LST].nTrack);
						SendDlgItemMessage(hJukeboxDlg, IDC_CUSTOMTRACK, WM_SETTEXT, 0, (LPARAM)szSoundCode);
						SendDlgItemMessage(hJukeboxDlg, IDC_CUSTOMTITLE, WM_SETTEXT, 0, (LPARAM)Tracks_LST[nCurrentTrack_LST].szName);
						SendMessage(hDlg, WM_NEXTDLGCTL, (WPARAM)hTrackList_LST, TRUE);
					} else {
						PlayNextTrack();
						DynamicSelectionScroll(hTrackList_DAT, nCurrentTrack_DAT);
						ListView_SetItemState(hTrackList_DAT, nCurrentTrack_DAT, LVIS_FOCUSED | LVIS_SELECTED, 0x000f);
						TCHAR szSoundCode[5];
						_stprintf(szSoundCode, _T("%4.4X"), Tracks_DAT[nCurrentTrack_DAT].nTrack);
						SendDlgItemMessage(hJukeboxDlg, IDC_CUSTOMTRACK, WM_SETTEXT, 0, (LPARAM)szSoundCode);
						SendDlgItemMessage(hJukeboxDlg, IDC_CUSTOMTITLE, WM_SETTEXT, 0, (LPARAM)Tracks_DAT[nCurrentTrack_DAT].szName);
						SendMessage(hDlg, WM_NEXTDLGCTL, (WPARAM)hTrackList_DAT, TRUE);
					}
				}
				break;

				// PLAY PREVIOUS TRACK
				case IDC_TLPREV:
				{
					if(nActivePlaylist == LST_PLAYLIST) {
						PlayPreviousTrack();
						DynamicSelectionScroll(hTrackList_LST, nCurrentTrack_LST);
						ListView_SetItemState(hTrackList_LST, nCurrentTrack_LST, LVIS_FOCUSED | LVIS_SELECTED, 0x000f);
						TCHAR szSoundCode[5];
						_stprintf(szSoundCode, _T("%4.4X"), Tracks_LST[nCurrentTrack_LST].nTrack);
						SendDlgItemMessage(hJukeboxDlg, IDC_CUSTOMTRACK, WM_SETTEXT, 0, (LPARAM)szSoundCode);
						SendDlgItemMessage(hJukeboxDlg, IDC_CUSTOMTITLE, WM_SETTEXT, 0, (LPARAM)Tracks_LST[nCurrentTrack_LST].szName);
						SendMessage(hDlg, WM_NEXTDLGCTL, (WPARAM)hTrackList_LST, TRUE);
					} else {						
						PlayPreviousTrack();
						DynamicSelectionScroll(hTrackList_DAT, nCurrentTrack_DAT);
						ListView_SetItemState(hTrackList_DAT, nCurrentTrack_DAT, LVIS_FOCUSED | LVIS_SELECTED, 0x000f);
						TCHAR szSoundCode[5];
						_stprintf(szSoundCode, _T("%4.4X"), Tracks_DAT[nCurrentTrack_DAT].nTrack);
						SendDlgItemMessage(hJukeboxDlg, IDC_CUSTOMTRACK, WM_SETTEXT, 0, (LPARAM)szSoundCode);
						SendDlgItemMessage(hJukeboxDlg, IDC_CUSTOMTITLE, WM_SETTEXT, 0, (LPARAM)Tracks_DAT[nCurrentTrack_DAT].szName);
						SendMessage(hDlg, WM_NEXTDLGCTL, (WPARAM)hTrackList_DAT, TRUE);
					}
				}
				break;

				// SELECT AND PLAY THE FIRST TRACK OF THE LIST
				case IDC_TLFIRST:
				{
					if(nActivePlaylist == LST_PLAYLIST) {
						int nSelCheck = ListView_GetItemCount(hTrackList_LST);
						if(nSelCheck <= 0) {
							SetDlgItemText(hJukeboxDlg, IDC_CUSTOMTRACK, _T("0000"));
							SetDlgItemText(hJukeboxDlg, IDC_CUSTOMTITLE, _T("- Enter track title here -"));
							break;
						}
						int nSel = 0;
						DynamicSelectionScroll(hTrackList_LST, nSel);
						nCurrentTrack_LST = nSel;
						PlayCurrentTrack();
						ListView_SetItemState(hTrackList_LST, nSel, LVIS_FOCUSED | LVIS_SELECTED, 0x000F);
						TCHAR szSoundCode[5];
						_stprintf(szSoundCode, _T("%4.4X"), Tracks_LST[nCurrentTrack_LST].nTrack);
						SendDlgItemMessage(hJukeboxDlg, IDC_CUSTOMTRACK, WM_SETTEXT, 0, (LPARAM)szSoundCode);
						SendDlgItemMessage(hJukeboxDlg, IDC_CUSTOMTITLE, WM_SETTEXT, 0, (LPARAM)Tracks_LST[nCurrentTrack_LST].szName);	
						SendMessage(hDlg, WM_NEXTDLGCTL, (WPARAM)hTrackList_LST, TRUE);
					} else {
						int nSelCheck = ListView_GetItemCount(hTrackList_DAT);
						if(nSelCheck <= 0) {
							SetDlgItemText(hJukeboxDlg, IDC_CUSTOMTRACK, _T("0000"));
							SetDlgItemText(hJukeboxDlg, IDC_CUSTOMTITLE, _T("- Enter track title here -"));
							break;
						}
						int nSel = 0;
						DynamicSelectionScroll(hTrackList_DAT, nSel);
						nCurrentTrack_DAT = nSel;
						PlayCurrentTrack();
						ListView_SetItemState(hTrackList_DAT, nSel, LVIS_FOCUSED | LVIS_SELECTED, 0x000F);
						TCHAR szSoundCode[5];
						_stprintf(szSoundCode, _T("%4.4X"), Tracks_DAT[nCurrentTrack_DAT].nTrack);
						SendDlgItemMessage(hJukeboxDlg, IDC_CUSTOMTRACK, WM_SETTEXT, 0, (LPARAM)szSoundCode);
						SendDlgItemMessage(hJukeboxDlg, IDC_CUSTOMTITLE, WM_SETTEXT, 0, (LPARAM)Tracks_DAT[nCurrentTrack_DAT].szName);	
						SendMessage(hDlg, WM_NEXTDLGCTL, (WPARAM)hTrackList_DAT, TRUE);
					}
				}
				break;

				// SELECT AND PLAY THE LAST TRACK ON THE LIST
				case IDC_TLLAST:
				{
					if(nActivePlaylist == LST_PLAYLIST) {
						int nSel = ListView_GetItemCount(hTrackList_LST) - 1;
						DynamicSelectionScroll(hTrackList_LST, nSel);
						ListView_SetItemState(hTrackList_LST, nSel, LVIS_FOCUSED | LVIS_SELECTED, 0x000F);
						nTrackCount_LST = nSel;
						TCHAR szSoundCode[5];
						_stprintf(szSoundCode, _T("%4.4X"), Tracks_LST[nTrackCount_LST].nTrack);
						SetDlgItemText(hJukeboxDlg, IDC_CUSTOMTRACK, szSoundCode);
						SetDlgItemText(hJukeboxDlg, IDC_CUSTOMTITLE, Tracks_LST[nTrackCount_LST].szName);
						nCurrentTrack_LST = nSel;
						PlayCurrentTrack();
						SendMessage(hDlg, WM_NEXTDLGCTL, (WPARAM)hTrackList_LST, TRUE);
					} else {
						int nSel = ListView_GetItemCount(hTrackList_DAT) - 1;
						DynamicSelectionScroll(hTrackList_DAT, nSel);
						ListView_SetItemState(hTrackList_DAT, nSel, LVIS_FOCUSED | LVIS_SELECTED, 0x000F);
						nTrackCount_DAT = nSel;
						TCHAR szSoundCode[5];
						_stprintf(szSoundCode, _T("%4.4X"), Tracks_DAT[nTrackCount_DAT].nTrack);
						SetDlgItemText(hJukeboxDlg, IDC_CUSTOMTRACK, szSoundCode);
						SetDlgItemText(hJukeboxDlg, IDC_CUSTOMTITLE, Tracks_DAT[nTrackCount_DAT].szName);
						nCurrentTrack_DAT = nSel;
						PlayCurrentTrack();
						SendMessage(hDlg, WM_NEXTDLGCTL, (WPARAM)hTrackList_DAT, TRUE);
					}
				}
				break;

				// RELOAD BOTH .LST AND .DAT TRACKLISTS
				case IDC_TLRELOAD:
				{					
					if(nActivePlaylist == LST_PLAYLIST) 
					{
						ListView_DeleteAllItems(hTrackList_LST);												// Clear .LST tracklist
						if(ParseM1Tracklist()) {																// Parse .LST tracklist						
							UpdatePlaylistTotalTracks(LST_PLAYLIST);											// Update .LST tracklist number of tracks
							ListView_SetItemState(hTrackList_LST, 0, LVIS_SELECTED | LVIS_FOCUSED, 0x000f);		// Select the first item on the .LST tracklist
						}
						SendMessage(hDlg, WM_NEXTDLGCTL, (WPARAM)hTrackList_LST, TRUE);
					} else {
						ListView_DeleteAllItems(hTrackList_DAT);												// Clear .LST tracklist
						if(ParseTracklist()) {																	// Parse .LST tracklist
							FillTracklistDAT();
							UpdatePlaylistTotalTracks(DAT_PLAYLIST);											// Update .LST tracklist number of tracks
							ListView_SetItemState(hTrackList_DAT, 0, LVIS_SELECTED | LVIS_FOCUSED, 0x000f);		// Select the first item on the .LST tracklist
						}
						SendMessage(hDlg, WM_NEXTDLGCTL, (WPARAM)hTrackList_LST, TRUE);
					}					
				}
				break;
				
				// SAVE .LST TRACKLIST
				case IDC_SAVE_TRACKLIST_LST:
				{
					if(nActivePlaylist == LST_PLAYLIST) 
					{
						SaveTracklist_LST();
					} else {
						SaveTracklist_DAT();
					}
				}
				break;

				case IDC_JUKEBOX_LOG_WAV_START:
				{
					AudBlankSound();
					if(!WaveLogStart()) {
						EnableWindow(GetDlgItem(hJukeboxDlg, IDC_JUKEBOX_LOG_WAV_START), false);
						EnableWindow(GetDlgItem(hJukeboxDlg, IDC_JUKEBOX_LOG_WAV_STOP), true);
					}
				}
				break;

				case IDC_JUKEBOX_LOG_WAV_STOP:
				{
					AudBlankSound();
					WaveLogStop();
					EnableWindow(GetDlgItem(hJukeboxDlg, IDC_JUKEBOX_LOG_WAV_START), true);
					EnableWindow(GetDlgItem(hJukeboxDlg, IDC_JUKEBOX_LOG_WAV_STOP), false);
				}
				break;

				case IDC_JUKEBOX_LOWPASSFILTER:
				{
					nAudDSPModule[0] = !nAudDSPModule[0];
				}
				break;
			}
		}
	}

	if (Msg == WM_CLOSE)
	{
		EnableWindow(hScrnWnd, true);
		if(bJukeboxInUse) bDoPostInitialize	= true;	// This will be checked at run.cpp and the Jukebox will exit properly

		DestroyWindow(hJukeboxDlg);
		DestroyWindow(hTrackList_LST);
		DestroyWindow(hTrackList_DAT);
		DestroyWindow(hTracklistTabs);

		hJukeboxDlg				= NULL;
		hTrackList_LST			= NULL;
		hTrackList_DAT			= NULL;
		hTracklistTabs			= NULL;

		isClone_LST				= false;
		isClone_DAT				= false;
		nActivePlaylist			= LST_PLAYLIST;
		nTrackCount_DAT			= 0;
		nTrackCount_LST			= 0;
		nCurrentTrack_DAT		= -1;
		nCurrentTrack_LST		= -1;
		hJukeboxThread			= NULL;
		dwJukeboxThreadID		= 0;

		if (hWhiteBGBrush) {
			DeleteObject(hWhiteBGBrush);
			hWhiteBGBrush		= NULL;
		}
		if (hBmp) {
			DeleteObject((HGDIOBJ)hBmp);
			hBmp				= NULL;
		}
		if (hPreview) {
			DeleteObject((HGDIOBJ)hPreview);
			hPreview			= NULL;
		}

		bAutoPause = nAutoPauseOld;
	}

	return 0;
}

static DWORD WINAPI DoTracklist(LPVOID) {
	MSG msg;
	CreateDialog(hAppInst, MAKEINTRESOURCE(IDD_TRACKLIST), hScrnWnd, DialogProc);
	while (GetMessage(&msg, NULL, 0, 0)) {
		if (TranslateAccelerator(hScrnWnd, hAccel, &msg) == 0) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return 0;
}


int TracklistDialog() 
{
	nAutoPauseOld = bAutoPause;
	bAutoPause = 0;
	hJukeboxThread = CreateThread(NULL, 0, DoTracklist, NULL, 0, &dwJukeboxThreadID);
	bJukeboxDisplayed = true;
	return 0;
}
