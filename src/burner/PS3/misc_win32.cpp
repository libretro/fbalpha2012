// misc win32 functions
#include "burner.h"

// Set the current directory to be the application's directory
int appDirectory()
{
	return 0;
}

// ---------------------------------------------------------------------------

// Get the position of the client area of a window on the screen
int getClientScreenRect(HWND hWnd, RECT *pRect)
{
	POINT Corner = {0, 0};

	pRect->left = 0;
	pRect->right = nVidScrnWidth;
	pRect->top = 0;
	pRect->bottom = nVidScrnHeight;

	return 0;
}

// Put a window in the middle of another window
#if 0
int wndInMid(HWND hMid, HWND hBase)
{ 
	return 0;
}
#endif

// ==> other functions, added by regret
void setWindowAspect(bool first_boot)
{
   #if 0
	int width = 640;
	int height = 480;

	if (width <= 0 || height <= 0) {
		return;
	}
   #endif

   #if 0
	if (autoVidScrnAspect) {
		vidScrnAspect = (float)width / height;
	} else {
   #endif
   if(first_boot)
   {
      nVidOriginalScrnAspectX = nVidScrnAspectX;
      nVidOriginalScrnAspectY = nVidScrnAspectY;
   }
   
   switch(nVidScrnAspectMode)
   {
      case ASPECT_RATIO_4_3:
         nVidScrnAspectX = 4;
         nVidScrnAspectY = 3;
         bVidCorrectAspect = 1;
         bVidFullStretch = 0;
         custom_aspect_ratio_mode = 0;
         break;
      case ASPECT_RATIO_5_4:
         nVidScrnAspectX = 5;
         nVidScrnAspectY = 4;
         bVidCorrectAspect = 0;
         bVidFullStretch = 1;
         custom_aspect_ratio_mode = 0;
         break;
      case ASPECT_RATIO_7_5:
         nVidScrnAspectX = 7;
         nVidScrnAspectY = 5;
         bVidCorrectAspect = 0;
         bVidFullStretch = 1;
         custom_aspect_ratio_mode = 0;
         break;
      case ASPECT_RATIO_8_7:
         nVidScrnAspectX = 8;
         nVidScrnAspectY = 7;
         bVidCorrectAspect = 0;
         bVidFullStretch = 1;
         custom_aspect_ratio_mode = 0;
         break;
      case ASPECT_RATIO_10_7:
         nVidScrnAspectX = 10;
         nVidScrnAspectY = 7;
         bVidCorrectAspect = 0;
         bVidFullStretch = 1;
         custom_aspect_ratio_mode = 0;
         break;
      case ASPECT_RATIO_11_8:
         nVidScrnAspectX = 11;
         nVidScrnAspectY = 8;
         bVidCorrectAspect = 0;
         bVidFullStretch = 1;
         custom_aspect_ratio_mode = 0;
         break;
      case ASPECT_RATIO_12_7:
         nVidScrnAspectX = 12;
         nVidScrnAspectY = 7;
         bVidCorrectAspect = 0;
         bVidFullStretch = 1;
         custom_aspect_ratio_mode = 0;
         break;
      case ASPECT_RATIO_16_9:
         nVidScrnAspectX = 16;
         nVidScrnAspectY = 9;
         bVidCorrectAspect = 0;
         bVidFullStretch = 1;
         custom_aspect_ratio_mode = 0;
         break;
      case ASPECT_RATIO_16_10:
         nVidScrnAspectX = 16;
         nVidScrnAspectY = 10;
         bVidCorrectAspect = 0;
         bVidFullStretch = 1;
         custom_aspect_ratio_mode = 0;
         break;
      case ASPECT_RATIO_16_15:
         nVidScrnAspectX = 16;
         nVidScrnAspectY = 15;
         bVidCorrectAspect = 0;
         bVidFullStretch = 1;
         custom_aspect_ratio_mode = 0;
         break;
      case ASPECT_RATIO_19_14:
         nVidScrnAspectX = 19;
         nVidScrnAspectY = 14;
         bVidCorrectAspect = 0;
         bVidFullStretch = 1;
         custom_aspect_ratio_mode = 0;
         break;
      case ASPECT_RATIO_1_1:
         nVidScrnAspectX = 1;
         nVidScrnAspectY = 1;
         bVidCorrectAspect = 0;
         bVidFullStretch = 1;
         custom_aspect_ratio_mode = 0;
         break;
      case ASPECT_RATIO_2_1:
         nVidScrnAspectX = 2;
         nVidScrnAspectY = 1;
         bVidCorrectAspect = 0;
         bVidFullStretch = 1;
         custom_aspect_ratio_mode = 0;
         break;
      case ASPECT_RATIO_3_2:
         nVidScrnAspectX = 3;
         nVidScrnAspectY = 2;
         bVidCorrectAspect = 0;
         bVidFullStretch = 1;
         custom_aspect_ratio_mode = 0;
         break;
      case ASPECT_RATIO_3_4:
         nVidScrnAspectX = 3;
         nVidScrnAspectY = 4;
         bVidCorrectAspect = 0;
         bVidFullStretch = 1;
         custom_aspect_ratio_mode = 0;
         break;
      case ASPECT_RATIO_CUSTOM:
         bVidCorrectAspect = 0;
         bVidFullStretch = 1;
         custom_aspect_ratio_mode = 1;
         break;
      case ASPECT_RATIO_AUTO:
         if(bDrvOkay)
         {
            int width, height;
            BurnDrvGetFullSize(&width, &height);
            unsigned len = width < height ? width : height;
            unsigned highest = 1;
            for (unsigned i = 1; i < len; i++)
            {
               if ((width % i) == 0 && (height % i) == 0)
                  highest = i;
            }

            nVidScrnAspectX = width / highest;
            nVidScrnAspectY = height / highest;
            bVidCorrectAspect = 0;
            bVidFullStretch = 1;
         }
         custom_aspect_ratio_mode = 0;
         break;
      case ASPECT_RATIO_AUTO_FBA:
         if(bDrvOkay)
         {
            nVidScrnAspectX = nVidOriginalScrnAspectX;
            nVidScrnAspectY = nVidOriginalScrnAspectY;
            bVidCorrectAspect = 0;
            bVidFullStretch = 1;
         }
         custom_aspect_ratio_mode = 0;
         break; 
   }

	vidScrnAspect = (float)nVidScrnAspectX / (float)nVidScrnAspectY;
	//}

}

void createNeedDir()
{
	cellFsMkdir("/dev_hdd0/game/FBAN00000/USRDIR/roms", CELL_FS_DEFAULT_CREATE_MODE_1);
	cellFsMkdir("/dev_hdd0/game/FBAN00000/USRDIR/config", CELL_FS_DEFAULT_CREATE_MODE_1);
	cellFsMkdir("/dev_hdd0/game/FBAN00000/USRDIR/config/presets", CELL_FS_DEFAULT_CREATE_MODE_1);
	cellFsMkdir("/dev_hdd0/game/FBAN00000/USRDIR/config/games", CELL_FS_DEFAULT_CREATE_MODE_1);
	cellFsMkdir("/dev_hdd0/game/FBAN00000/USRDIR/previews", CELL_FS_DEFAULT_CREATE_MODE_1);
	cellFsMkdir("/dev_hdd0/game/FBAN00000/USRDIR/cheats", CELL_FS_DEFAULT_CREATE_MODE_1);
	cellFsMkdir("/dev_hdd0/game/FBAN00000/USRDIR/screenshots", CELL_FS_DEFAULT_CREATE_MODE_1);
	cellFsMkdir("/dev_hdd0/game/FBAN00000/USRDIR/savestates", CELL_FS_DEFAULT_CREATE_MODE_1);
	cellFsMkdir("/dev_hdd0/game/FBAN00000/USRDIR/recordings", CELL_FS_DEFAULT_CREATE_MODE_1);
	cellFsMkdir("/dev_hdd0/game/FBAN00000/USRDIR/skins", CELL_FS_DEFAULT_CREATE_MODE_1);
	cellFsMkdir("/dev_hdd0/game/FBAN00000/USRDIR/ips", CELL_FS_DEFAULT_CREATE_MODE_1);
	cellFsMkdir("/dev_hdd0/game/FBAN00000/USRDIR/titles", CELL_FS_DEFAULT_CREATE_MODE_1);
	cellFsMkdir("/dev_hdd0/game/FBAN00000/USRDIR/flyers", CELL_FS_DEFAULT_CREATE_MODE_1);
	cellFsMkdir("/dev_hdd0/game/FBAN00000/USRDIR/scores", CELL_FS_DEFAULT_CREATE_MODE_1);
	cellFsMkdir("/dev_hdd0/game/FBAN00000/USRDIR/selects", CELL_FS_DEFAULT_CREATE_MODE_1);
	cellFsMkdir("/dev_hdd0/game/FBAN00000/USRDIR/gameovers", CELL_FS_DEFAULT_CREATE_MODE_1);
	cellFsMkdir("/dev_hdd0/game/FBAN00000/USRDIR/bosses", CELL_FS_DEFAULT_CREATE_MODE_1);
	cellFsMkdir("/dev_hdd0/game/FBAN00000/USRDIR/icons", CELL_FS_DEFAULT_CREATE_MODE_1);
 
}

bool directoryExists(const TCHAR* dirname)
{
	return 1;
}


int directLoadGame(TCHAR* name)
{
	int RomOK = 1;

	if (!name)
		return 1;
 
	nVidFullscreen = 1;

	if (_tcsicmp(&name[_tcslen(name) - 3], _T(".fs")) == 0)
   {
		if (BurnStateLoad(name, 1, &DrvInitCallback))
			return 1;
	}
   #if 0
	else if (_tcsicmp(&name[_tcslen(name) - 3], _T(".fr")) == 0)
   {
		//if (StartReplay(name)) {
		//	return 1;
		//}
	}
   #endif
	else
   {
		// get game name
		TCHAR* p = getBaseName(name);

		 
		// load game
		unsigned int i = BurnDrvGetIndexByNameA(p);
 
		if (i < nBurnDrvCount)
      {
			RomOK = BurnerDrvInit(i, true);
			bAltPause = 0;
		}
	}

   is_running = 1;

   setWindowAspect(true);

	return RomOK;
}
 
// ---------------------------------------------------------------------------
// For DAT files printing and Kaillera windows

char* decorateGameName(unsigned int drv)
{
	if (drv >= nBurnDrvCount)
		return "";

	unsigned int nOldBurnDrv = nBurnDrvSelect;
	nBurnDrvSelect = drv;

	// get game full name
	static char szDecoratedName[1024] = "";
	strcpy(szDecoratedName, BurnDrvGetTextA(DRV_FULLNAME));

	// get game extra info
	char szGameInfo[256] = " [";
	bool hasInfo = false;

	if (BurnDrvGetFlags() & BDF_PROTOTYPE) {
		strcat(szGameInfo, "prototype");
		hasInfo = true;
	}
	if (BurnDrvGetFlags() & BDF_BOOTLEG) {
		strcat(szGameInfo, "bootleg");
		hasInfo = true;
	}
	if (BurnDrvGetTextA(DRV_COMMENT)) {
		strcat(szGameInfo, BurnDrvGetTextA(DRV_COMMENT));
		hasInfo = true;
	}

	if (hasInfo) {
		strcat(szGameInfo, "]");
		strcat(szDecoratedName, szGameInfo);
	}

	nBurnDrvSelect = nOldBurnDrv;

	return szDecoratedName;
}

#if 0
char* decorateKailleraGameName(unsigned int drv)
{
	if (drv >= nBurnDrvCount)
		return "";

	unsigned int nOldBurnDrv = nBurnDrvSelect;
	nBurnDrvSelect = drv;

	// get game full name, modified by regret
	static char szDecoratedName[1024] = "";
	sprintf(szDecoratedName, "%s [%s]", BurnDrvGetTextA(DRV_FULLNAME), BurnDrvGetTextA(DRV_NAME));

	nBurnDrvSelect = nOldBurnDrv;

	return szDecoratedName;
}
#endif

// rom util
static inline int findRomByName(const char* name, ArcEntry* list, int count)
{
	if (!name || !list)
		return -1;

	// Find the rom named name in the List
   int i = 0;
   do
   {
		if (list->szName && !_stricmp(name, getFilenameA(list->szName)))
			return i;
      i++;
      list++;
	}while(i < count);
	return -1; // couldn't find the rom
}

static inline int findRomByCrc(unsigned int crc, ArcEntry* list, int count)
{
	if (!list)
		return -1;

	// Find the rom named name in the List
   int i = 0;
   do
   {
		if (crc == list->nCrc)
			return i;
      i++;
      list++;
	}while(i < count);

	return -1; // couldn't find the rom
}

// Find rom number i from the pBzipDriver game
int findRom(int i, ArcEntry* list, int count)
{
	BurnRomInfo ri;
	memset(&ri, 0, sizeof(ri));

	int nRet = BurnDrvGetRomInfo(&ri, i);
	if (nRet != 0) {						// Failure: no such rom
		return -2;
	}

	if (ri.nCrc)   // Search by crc first
   {
      nRet = findRomByCrc(ri.nCrc, list, count);
		if (nRet >= 0)
			return nRet;
	}

   int nAka = 0;
   do
   {	// Failing that, search for possible names
		char* szPossibleName = NULL;
		nRet = BurnDrvGetRomName(&szPossibleName, i, nAka);
		if (nRet) // No more rom names
			break;
		nRet = findRomByName(szPossibleName, list, count);
		if (nRet >= 0)
			return nRet;
      nAka++;
	}while(nAka < 0x10000);

	return -1; // Couldn't find the rom
}


#if 0
bool SetNumLock(bool state)
{
	BYTE keyState[256];

	GetKeyboardState(keyState);
	if ((state && !(keyState[VK_NUMLOCK] & 1)) || (!state && (keyState[VK_NUMLOCK] & 1))) {
		keybd_event(VK_NUMLOCK, 0, KEYEVENTF_EXTENDEDKEY, 0 );

		keybd_event(VK_NUMLOCK, 0, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
	}

	return keyState[VK_NUMLOCK] & 1;
}

static void MyRegCreateKeys(int nDepth, TCHAR* pNames[], HKEY* pResult)
{
	for (int i = 0; i < nDepth; i++) {
		pResult[i] = NULL;
		RegCreateKeyEx((i ? pResult[i - 1] : HKEY_CLASSES_ROOT), pNames[i], 0, _T(""), 0, KEY_WRITE, NULL, &pResult[i], NULL);
	}
}

static void MyRegCloseKeys(int nDepth, HKEY* pKeys)
{
	for (int i = nDepth - 1; i >= 0; i--) {
		if (pKeys[i]) {
			RegCloseKey(pKeys[i]);
		}
	}
}

static void MyRegDeleteKeys(int nDepth, TCHAR* pNames[], HKEY* pKeys)
{
	for (int i = 0; i < nDepth - 1; i++) {
		pKeys[i] = NULL;
		RegOpenKeyEx((i ? pKeys[i - 1] : HKEY_CLASSES_ROOT), pNames[i], 0, 0, &pKeys[i]);
	}
	for (int i = nDepth - 1; i >= 0; i--) {
		RegDeleteKey((i ? pKeys[i - 1] : HKEY_CLASSES_ROOT), pNames[i]);
		if (i) {
			RegCloseKey(pKeys[i - 1]);
		}
	}
}

void RegisterExtensions(bool bCreateKeys)
{
	HKEY myKeys[4];

	TCHAR* myKeynames1[1] = { _T(".fr") };
	TCHAR* myKeynames2[1] = { _T(".fs") };
	TCHAR* myKeynames3[4] = { _T("FBAlpha"), _T("shell"), _T("open"), _T("command") };
	TCHAR* myKeynames4[2] = { _T("FBAlpha"), _T("DefaultIcon") };
	TCHAR myKeyValue[MAX_PATH + 32] = _T("");

	if (bCreateKeys) {
		TCHAR szExename[MAX_PATH] = _T("");
		GetModuleFileName(NULL, szExename, MAX_PATH);

		MyRegCreateKeys(1, myKeynames1, myKeys);
		_stprintf(myKeyValue, _T("FBAlpha"));
		RegSetValueEx(myKeys[0], NULL, 0, REG_SZ, (BYTE*)myKeyValue, (_tcslen(myKeyValue) + 1) * sizeof(TCHAR));
		MyRegCloseKeys(2, myKeys);

		MyRegCreateKeys(1, myKeynames2, myKeys);
		_stprintf(myKeyValue, _T("FBAlpha"));
		RegSetValueEx(myKeys[0], NULL, 0, REG_SZ, (BYTE*)myKeyValue, (_tcslen(myKeyValue) + 1) * sizeof(TCHAR));
		MyRegCloseKeys(2, myKeys);

		MyRegCreateKeys(4, myKeynames3, myKeys);
		_stprintf(myKeyValue, _T("\"%s\" \"%%1\" -w"), szExename);
		RegSetValueEx(myKeys[3], NULL, 0, REG_SZ, (BYTE*)myKeyValue, (_tcslen(myKeyValue) + 1) * sizeof(TCHAR));
		_stprintf(myKeyValue, _T("FB Alpha file"));
		RegSetValueEx(myKeys[0], NULL, 0, REG_SZ, (BYTE*)myKeyValue, (_tcslen(myKeyValue) + 1) * sizeof(TCHAR));
		MyRegCloseKeys(4, myKeys);

		MyRegCreateKeys(2, myKeynames4, myKeys);
		_stprintf(myKeyValue, _T("\"%s\", 0"), szExename);
		RegSetValueEx(myKeys[1], NULL, 0, REG_SZ, (BYTE*)myKeyValue, (_tcslen(myKeyValue) + 1) * sizeof(TCHAR));
		MyRegCloseKeys(2, myKeys);
	} else {
		MyRegDeleteKeys(2, myKeynames4, myKeys);
		MyRegDeleteKeys(4, myKeynames3, myKeys);
		MyRegDeleteKeys(1, myKeynames2, myKeys);
		MyRegDeleteKeys(1, myKeynames1, myKeys);
	}

	return;
}
#endif
