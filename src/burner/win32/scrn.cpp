// Screen Window
// Simplify screen reinit routine by regret

/* changelog:
 update 6: add fake fullscreen
 update 5: add scrnSwitchFull to switch fullscreen, scrnExit will not be called
 update 4: simplify enter/exit fullscreen routine
 update 3: do not delete screen in scrnInit
 update 2: add simpleReinitScrn
 update 1: source cleanup
*/

#include "burner.h"
#include <shellapi.h>
#include "cheat.h"
#include "cartridge.h"
#include "maphkeys.h"
#include "vid_directx_support.h"
#ifndef NO_AUTOFIRE
#include "autofire.h"
#endif

// Macros used for handling Window Messages
#define HANDLE_WM_ENTERMENULOOP(hwnd, wParam, lParam, fn)		\
    ((fn)((hwnd), (BOOL)(wParam)), 0L)

#define HANDLE_WM_EXITMENULOOP(hwnd, wParam, lParam, fn)		\
    ((fn)((hwnd), (BOOL)(wParam)), 0L)

#define HANDLE_WM_ENTERSIZEMOVE(hwnd, wParam, lParam, fn)		\
    ((fn)(hwnd), 0L)

#define HANDLE_WM_EXITSIZEMOVE(hwnd, wParam, lParam, fn)		\
    ((fn)(hwnd), 0L)

#define HANDLE_WM_UNINITMENUPOPUP(hwnd, wParam, lParam, fn)		\
	((fn)((hwnd), (HMENU)(wParam), (UINT)LOWORD(lParam), (BOOL)HIWORD(lParam)), 0)

#define HANDLE_WM_SIZING(hwnd, wParam, lParam, fn)				\
	((fn)((hwnd), (wParam), (lParam)))

#define HANDLE_WM_DROPFILES(hwnd, wParam, lParam, fn)			\
	((fn)((hwnd), (wParam)))

// Extra macro used for handling Window Messages
#define HANDLE_MSGB(hwnd, message, fn)							\
    case (message):												\
         HANDLE_##message((hwnd), (wParam), (lParam), (fn));	\
		 break;

RECT SystemWorkArea = { 0, 0, 640, 480 };		// Work area on the desktop
int nWindowPosX = -1, nWindowPosY = -1;			// Window position

int bAutoPause = 0;
int nSavestateSlot = 1;

bool bShowOnTop = false;
bool bFullscreenOnStart = false;
bool bFakeFullscreen = false;

static TCHAR* szClass = _T(APP_TITLE);			// Window class name
HWND hScrnWnd = NULL;							// Handle to the screen window
HWND hVideoWnd = NULL;							// Handle to the video window

static bool bMaximised;
static int nPrevWidth, nPrevHeight;

int nWindowSize = 0;

static bool bDrag = false;
static int nDragX, nDragY;
static int nOldWindowX, nOldWindowY;
static int nLeftButtonX, nLeftButtonY;

//int nScrnVisibleOffset[4] = { 0, 0, 0, 0 };		// scrn visible offset

bool useDialogs()
{
	if (/*!bDrvOkay ||*/ !nVidFullscreen) {
		return true;
	}

	return false;
}

void setPauseMode(bool bPause)
{
	bRunPause = bPause;
	bAltPause = bPause;

	if (bPause) {
		audio.blank();
		if (useDialogs()) {
			InputSetCooperativeLevel(false, !bAlwaysProcessKey);
		}
	} else {
		GameInpCheckMouse();
	}
}

int createDatfileWindows(int type)
{
	TCHAR szTitle[MAX_PATH];
	TCHAR szFilter[MAX_PATH];

	_sntprintf(szChoice, sizearray(szChoice), _T(APP_TITLE) _T(" v%.20s (%s).dat"), szAppBurnVer, _T("clrmamepro"));
	_sntprintf(szTitle, sizearray(szTitle), FBALoadStringEx(IDS_DAT_GENERATE), _T("clrmamepro"));

	_stprintf(szFilter, FBALoadStringEx(IDS_DISK_ALL_DAT), _T(APP_TITLE));
	memcpy(szFilter + _tcslen(szFilter), _T(" (*.dat)\0*.dat\0\0"), 16 * sizeof(TCHAR));

	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hScrnWnd;
	ofn.lpstrFilter = szFilter;
	ofn.lpstrFile = szChoice;
	ofn.nMaxFile = sizearray(szChoice);
	ofn.lpstrInitialDir = _T(".");
	ofn.Flags = OFN_NOCHANGEDIR | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	ofn.lpstrDefExt = _T("dat");
	ofn.lpstrTitle = szTitle;

	if (GetSaveFileName(&ofn) == 0)
		return 1;

	return create_datfile(szChoice, type);
}

// Returns true if a VidInit is needed when the window is resized
static inline bool vidInitNeeded()
{
	if (nVidSelect == VID_D3D) {
		return true;
	}
	return false;
}

// Refresh the contents of the window when re-sizing it
static inline void refreshWindow(bool bInitialise)
{
	if (nVidFullscreen) {
		return;
	}

	if (bInitialise && vidInitNeeded()) {
		VidReinit();
	}
}

// simply reinit screen, added by regret
void simpleReinitScrn(const bool& reinitVid)
{
	scrnSize();

	// need for dx9 blitter
	if (reinitVid || vidInitNeeded()) {
		VidReinit();
	}
}

static inline int onDisplayChange(HWND, UINT, UINT, UINT)
{
	if (nVidFullscreen == 0) {
		POST_INITIALISE_MESSAGE;
	}
	return 0;
}

static int onRButtonDown(HWND hwnd, BOOL bDouble, int, int, UINT)
{
	if (hwnd != hScrnWnd) {
		return 1;
	}

	if (bDouble) {
		if (bDrvOkay) {
			nVidFullscreen = !nVidFullscreen;
			scrnSwitchFull();
			return 0;
		}
	} else {
		if (!nVidFullscreen && !bFakeFullscreen) {
			bMenuEnabled = !bMenuEnabled;
			scrnSwitchFull();
			return 0;
		}
	}

	return 1;
}

/*
static int onRButtonUp(HWND, int, int, UINT)
{
	return 1;
}
*/

static int onLButtonDown(HWND hwnd, BOOL, int x, int y, UINT)
{
	if (hwnd == hScrnWnd && !nVidFullscreen) {
		RECT clientRect;

		GetWindowRect(hwnd, &clientRect);

		nOldWindowX = clientRect.left;
		nOldWindowY = clientRect.top;

		nLeftButtonX = clientRect.left + x;
		nLeftButtonY = clientRect.top + y;

		bDrag = true;
		return 0;
	}

	return 1;
}

static int onLButtonDblClk(HWND hwnd, BOOL, int, int, UINT)
{
	if (hwnd == hScrnWnd && bDrvOkay) {
		nVidFullscreen = !nVidFullscreen;
		scrnSwitchFull();
		return 0;
	}
	return 1;
}

static int onMouseMove(HWND hwnd, int x, int y, UINT keyIndicators)
{
	if (bDrag && hwnd == hScrnWnd && keyIndicators == MK_LBUTTON && !nVidFullscreen && !bFakeFullscreen) {
		RECT clientRect;
		GetWindowRect(hwnd, &clientRect);

		if ((nLeftButtonX - (clientRect.left + x)) < nDragX
			&& (nLeftButtonX - (clientRect.left + x)) > -nDragX
			&& (nLeftButtonY - (clientRect.top + y)) < nDragY
			&& (nLeftButtonY - (clientRect.top + y)) > -nDragY) {
			SetWindowPos(hwnd, NULL, nOldWindowX, nOldWindowY, 0, 0, SWP_NOREPOSITION | SWP_NOSIZE);
		} else {
			nWindowPosX = nOldWindowX - (nLeftButtonX - (clientRect.left + x));
			nWindowPosY = nOldWindowY - (nLeftButtonY - (clientRect.top + y));

			SetWindowPos(hwnd, NULL, nWindowPosX, nWindowPosY, 0, 0, SWP_NOREPOSITION | SWP_NOSIZE);
		}

		return 0;
	}

	return 1;
}

static int onLButtonUp(HWND hwnd, int x, int y, UINT)
{
	bDrag = false;

	if (nVidFullscreen) {
		if (hwnd != hScrnWnd) {
			return 1;
		}

		if (useDialogs()) {
			RECT clientRect;
			GetWindowRect(hwnd, &clientRect);
			int clientx = clientRect.left + x;
			int clienty = clientRect.top + y;

			TrackPopupMenuEx(hMenuPopup, TPM_LEFTALIGN | TPM_TOPALIGN, clientx, clienty, hwnd, NULL);
			return 0;
		}
	} else {
		if (!bMenuEnabled) {
			RECT clientRect;
			GetWindowRect(hwnd, &clientRect);
			int clientx = clientRect.left + x;
			int clienty = clientRect.top + y;

			if ((nLeftButtonX - clientx) < nDragX && (nLeftButtonX - clientx) > -nDragX
				&& (nLeftButtonY - clienty) < nDragY && (nLeftButtonY - clienty) > -nDragY) {
				TrackPopupMenuEx(hMenuPopup, TPM_LEFTALIGN | TPM_TOPALIGN, clientx, clienty, hwnd, NULL);
				return 0;
			}
		}
	}

	return 1;
}

static int onCreate(HWND, LPCREATESTRUCT)	// HWND hwnd, LPCREATESTRUCT lpCreateStruct
{
	return 1;
}

static void onActivateApp(HWND hwnd, BOOL fActivate, DWORD /* dwThreadId */)
{
	if (!kNetGame && bAutoPause && !bAltPause && dialogIsEmpty()) {
		bRunPause = fActivate? 0 : 1;
	}
	if (fActivate == false && hwnd == hScrnWnd) {
		EndMenu();
	}
	if (fActivate == false && bRunPause) {
		audio.blank();
	}

	if (fActivate) {
		if (!dialogIsEmpty()) {
			InputSetCooperativeLevel(false, !bAlwaysProcessKey);
		} else {
			GameInpCheckMouse();
		}
	}
}

static void onPaint(HWND hWnd)
{
	if (hWnd == hScrnWnd) {
		// paint and validate client area
		VidPaint(1);

		// paint skin picture
		if (!bDrvOkay && !bVidUsePlaceholder) {
			paintSkin(hWnd);
		}
	}
}

static void onClose(HWND)
{
	PostQuitMessage(0);					// Quit the program if the window is closed
}

static void onDestroy(HWND)
{
//	VidExit();							// Stop using video with the Window
	hScrnWnd = NULL;					// Make sure handle is not used again
}

void quickLoadFile()
{
	SplashDestroy(1);
	audio.stop();

	memset(&ofn, 0, sizeof(ofn));
	ofn.lpstrTitle = _T("Quick Load games");
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hScrnWnd;
	ofn.lpstrFilter = _T("All support types\0*.zip;*.7z;*.fr;*.fs\0") _T("Archive File (*.zip,*.7z)\0*.zip;*.7z\0\0");
	ofn.lpstrFile = szChoice;
	ofn.nMaxFile = sizearray(szChoice);
	ofn.Flags = OFN_NOCHANGEDIR | OFN_HIDEREADONLY;

	int bOldPause = bRunPause;
	bRunPause = 1;
	int nRet = GetOpenFileName(&ofn);
	bRunPause = bOldPause;

	audio.play();

	if (nRet == 0)
		return;

	directLoadGame(szChoice);
}

static void mountCartridge(const bool& mount)
{
	if (mount) {
		if (BurnCartridgeGameLoaded) {
			memset(&ofn, 0, sizeof(ofn));
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = hScrnWnd;
			ofn.lpstrFilter = pBurnCartridgeFileExtFilter;
			ofn.lpstrFile = szChoice;
			ofn.nMaxFile = sizearray(szChoice);
			ofn.Flags = OFN_HIDEREADONLY;

			int bOldPause = bRunPause;
			bRunPause = 1;
			int nRet = GetOpenFileName(&ofn);
			bRunPause = bOldPause;
			if (nRet && pBurnInsertCartridgeCallBack) {
				pBurnInsertCartridgeCallBack(szChoice);
			}
		}
	} else {
		if (BurnCartridgeGameLoaded && pBurnInsertCartridgeCallBack) {
			pBurnInsertCartridgeCallBack(0);
		}
	}
}

bool bLoading = false;

static void onCommand(HWND hDlg, int id, HWND /*hwndCtl*/, UINT codeNotify)
{
	if (bLoading) {
		return;
	}

	switch (id) {
		case MENU_LOAD:
			HK_openGame(0);
			break;

		case MENU_LOAD_QUICK:
			HK_quickOpenGame(0);
			break;

		case MENU_QUIT:
			HK_exitGame(0);
			break;

		case MENU_EXIT:
			StopReplay();
			AviStop();
			FBA_KailleraEnd();
			PostQuitMessage(0);

			menuSync(MENUT_ALL);
			return;

		case MENU_PAUSE:
			HK_pause(0);
			break;

		case MENU_CARTR_MOUNT:
			mountCartridge(true);
			menuSync(MENUT_FILE);
			break;

		case MENU_CARTR_UNMOUNT:
			mountCartridge(false);
			menuSync(MENUT_FILE);
			break;

		case MENU_STARTNET:
			if (!kNetGame) {
				InputSetCooperativeLevel(false, !bAlwaysProcessKey);
				audio.blank();
				SplashDestroy(1);
				StopReplay();
				AviStop();
				BurnerDrvExit();
				// load kaillera.dll
				if (FBA_KailleraInit()) {
					DoNetGame();
				}
				menuSync(MENUT_ALL);
				InputSetCooperativeLevel(false, !bAlwaysProcessKey);
			}
			break;

		case MENU_STARTREPLAY:
			HK_playRec(0);
			break;
		case MENU_STARTRECORD:
			HK_startRec(0);
			break;
		case MENU_STOPREPLAY:
			HK_stopRec(0);
			break;

		case MENU_AVISTART:
			HK_startAvi(0);
			break;
		case MENU_AVISTOP:
			HK_stopAvi(0);
			break;
		case MENU_AVIINTAUD:
			nAviIntAudio = !nAviIntAudio;
			menuSync(MENUT_FILE);
			break;

		case MENU_MEMCARD_CREATE:
			if (useDialogs() && bDrvOkay && !kNetGame
				&& (BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_SNK_NEOGEO) {
				InputSetCooperativeLevel(false, !bAlwaysProcessKey);
				audio.blank();
				MemCardEject();
				MemCardCreate();
				MemCardInsert();
				menuSync(MENUT_FILE);
				GameInpCheckMouse();
			}
			break;
		case MENU_MEMCARD_SELECT:
			if (useDialogs() && bDrvOkay && !kNetGame
				&& (BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_SNK_NEOGEO) {
				InputSetCooperativeLevel(false, !bAlwaysProcessKey);
				audio.blank();
				MemCardEject();
				MemCardSelect();
				MemCardInsert();
				menuSync(MENUT_FILE);
				GameInpCheckMouse();
			}
			break;
		case MENU_MEMCARD_INSERT:
			if (bDrvOkay && !kNetGame
				&& (BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_SNK_NEOGEO) {
				MemCardInsert();
			}
			break;
		case MENU_MEMCARD_EJECT:
			if (bDrvOkay && !kNetGame
				&& (BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_SNK_NEOGEO) {
				MemCardEject();
			}
			break;

		case MENU_MEMCARD_TOGGLE:
			if (bDrvOkay && !kNetGame
				&& (BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_SNK_NEOGEO) {
				MemCardToggle();
			}
			break;

		case MENU_STATE_LOAD_DIALOG:
			HK_loadStateDialog(0);
			break;
		case MENU_STATE_SAVE_DIALOG:
			HK_saveStateDialog(0);
			return;
		case MENU_STATE_PREVSLOT:
			HK_prevState(0);
			break;
		case MENU_STATE_NEXTSLOT:
			HK_nextState(0);
			break;
		case MENU_STATE_LOAD_SLOT:
			HK_loadCurState(0);
			break;
		case MENU_STATE_SAVE_SLOT:
			HK_saveCurState(0);
			break;

		case MENU_STATE_ALLRAM:
			bDrvSaveAll = !bDrvSaveAll;
			menuSync(MENUT_FILE);
			break;

		case MENU_WLOGSTART:
			audio.blank();
			waveLogStart();
			break;
		case MENU_WLOGEND:
			audio.blank();
			waveLogStop();
			break;

		case MENU_NOSTRETCH:
			bVidCorrectAspect = 0;
			bVidFullStretch = 0;
			simpleReinitScrn(false);
			menuSync(MENUT_SETTING);
			break;
		case MENU_STRETCH:
			bVidFullStretch = 1;
			bVidCorrectAspect = 0;
			simpleReinitScrn(false);
			menuSync(MENUT_SETTING);
			break;
		case MENU_ASPECT:
			bVidCorrectAspect = 1;
			bVidFullStretch = 0;
			simpleReinitScrn(false);
			menuSync(MENUT_SETTING);
			break;

		case MENU_FULL:
			HK_fullscreen(0);
			return;

		case MENU_AUTOSIZE:
			if (nWindowSize != 0) {
				nWindowSize = 0;
				simpleReinitScrn(false);
				menuSync(MENUT_SETTING);
			}
			break;
		case MENU_WINDOWSIZE1X:
			HK_windowSize(1);
			break;
		case MENU_WINDOWSIZE2X:
			HK_windowSize(2);
			break;
		case MENU_WINDOWSIZE3X:
			HK_windowSize(3);
			break;
		case MENU_WINDOWSIZE4X:
			HK_windowSize(4);
			break;
		case MENU_WINDOWSIZEMAX:
			HK_windowSizeMax(0);
			break;

		case MENU_HOTKEYS:
			MHkeysCreate(hDlg);
			break;

		case MENU_INPUT:
			HK_configPad(0);
			break;

		case MENU_DIPSW:
			HK_setDips(0);
			break;

		case MENU_SETCPUCLOCK:
			audio.blank();
			CPUClockDialog(hDlg);
			menuSync(MENUT_GAME);
			GameInpCheckMouse();
			break;
		case MENU_RESETCPUCLOCK:
			nBurnCPUSpeedAdjust = 0x0100;
			menuSync(MENUT_GAME);
			break;

		// ==> rom save
		case MENU_SAVEC:
			bsavedecryptedcs = !bsavedecryptedcs;
			menuSync(MENUT_GAME);
			break;
		case MENU_SAVEP:
			bsavedecryptedps = !bsavedecryptedps;
			menuSync(MENUT_GAME);
			break;
		case MENU_SAVES:
			bsavedecrypteds1 = !bsavedecrypteds1;
			menuSync(MENUT_GAME);
			break;
		case MENU_SAVEV:
			bsavedecryptedvs = !bsavedecryptedvs;
			menuSync(MENUT_GAME);
			break;
		case MENU_SAVEM:
			bsavedecryptedm1 = !bsavedecryptedm1;
			menuSync(MENUT_GAME);
			break;
		case MENU_SAVEXOR:
			bsavedecryptedxor = !bsavedecryptedxor;
			menuSync(MENUT_GAME);
			break;
		case MENU_SAVEDPROM:
			bsavedecryptedprom = !bsavedecryptedprom;
			menuSync(MENUT_GAME);
			break;
		// <== rom save

		case MENU_LANGUAGE_SELECT:
			if (useDialogs()) {
				if (!FBALocaliseLoad()) {
					menuReinit();
				}
			}
			break;
		case MENU_LANGUAGE_UNLOAD:
			_tcsncpy(szLanguage, _T(""), sizearray(szLanguage));
			FBALocaliseExit();
			menuReinit();
			break;

		case MENU_GAMELIST_SELECT:
			if (useDialogs()) {
				loadGamelist();
			}
			break;
		case MENU_GAMELIST_EXPORT:
			if (useDialogs()) {
				createGamelist();
			}
			break;

		case MENU_MISCDIR_EDIT:
			if (useDialogs()) {
				pathSheetCreate(hDlg);
			}
			break;
		case MENU_ROMDIR_EDIT:
			if (useDialogs()) {
				RomsDirCreate(hDlg);
			}
			break;

		case MENU_ENABLECHEAT:
			HK_cheatEditor(0);
			break;

#ifndef NO_CHEATSEARCH
		case MENU_CHEATSEARCH:
			HK_cheatSearch(0);
			break;
#endif

		// ==> simple jukebox, added by regret
		case MENU_JUKEBOX:
			if (useDialogs()) {
				jukeCreate();
				InputSetCooperativeLevel(false, !bAlwaysProcessKey);
			}
			break;
		// <== simple jukebox

		case MENU_DEBUG:
			if (useDialogs()) {
				InputSetCooperativeLevel(false, !bAlwaysProcessKey);
				DebugCreate();
			}
			break;

		case MENU_SAVESNAP: {
			HK_screenShot(0);
			break;
		}
		case MENU_SNAPFACT:
			HK_shotFactory(0);
			break;

		case MENU_SKIN_SELECT:
			if (useDialogs()) {
				if (selectSkin() == 0) {
					simpleReinitScrn(true);
				}
			}
			break;
		case MENU_SKIN_UNLOAD:
			_tcsncpy(szPlaceHolder, _T(""), sizearray(szPlaceHolder));
			simpleReinitScrn(true);
			break;

		case MENU_CLRMAMEPRO_XML:
			if (useDialogs()) {
				createDatfileWindows(1);
			}
			break;
		case MENU_CLRMAME_PRO:
			if (useDialogs()) {
				createDatfileWindows(0);
			}
			break;

		case MENU_SAVESET:
			configAppSaveXml();
			break;
		case MENU_LOADSET:
			configAppLoadXml();
			POST_INITIALISE_MESSAGE;
			break;

		case MENU_ABOUT:
			if (useDialogs()) {
				InputSetCooperativeLevel(false, !bAlwaysProcessKey);
				audio.blank();
				AboutCreate(hDlg);
				GameInpCheckMouse();
			}
			break;
		case MENU_SYSINFO:
			if (useDialogs()) {
				InputSetCooperativeLevel(false, !bAlwaysProcessKey);
				audio.blank();
				SystemInfoCreate(hDlg);
				GameInpCheckMouse();
			}
			break;

		case MENU_CONTENTS: {
			if (useDialogs()) {
				FILE* fp = _tfopen(_T("readme.txt"), _T("r"));
				if (fp) {
					fclose(fp);
					ShellExecute(NULL, _T("open"), _T("readme.txt"), NULL, NULL, SW_SHOWNORMAL);
				}
			}
			break;
		}

		case MENU_WWW_HOME:
			ShellExecute(NULL, _T("open"), _T("http://www.barryharris.me.uk/"), NULL, NULL, SW_SHOWNORMAL);
			break;
		case MENU_WWW_FORUM:
			ShellExecute(NULL, _T("open"), _T("http://neosource.1emulation.com/forums/"), NULL, NULL, SW_SHOWNORMAL);
			break;
		case MENU_WWW_FORUM1:
			ShellExecute(NULL, _T("open"), _T("http://www.egcg.com.cn/bbs/"), NULL, NULL, SW_SHOWNORMAL);
			break;

		// filters
		case MENU_SOFT_NONE:
			vidUseFilter = 0;
			nVidFilter = 0;
			VidReinit();
			menuSync(MENUT_SETTING);
			break;

		case MENU_SOFT_EPXB:
		case MENU_SOFT_EPXC:
		case MENU_SOFT_SCALE2X:
		case MENU_SOFT_SCALE3X:
		case MENU_SOFT_2XSAI:
		case MENU_SOFT_SUPER2XSAI:
		case MENU_SOFT_SUPEREAGLE:
		case MENU_SOFT_2XPMHQ:
		case MENU_SOFT_HQ2X:
		case MENU_SOFT_HQ2XS:
		case MENU_SOFT_HQ2XBOLD:
		case MENU_SOFT_HQ3X:
		case MENU_SOFT_HQ3XS:
		case MENU_SOFT_HQ3XBOLD:
		case MENU_SOFT_SCANLINE:
		case MENU_SOFT_SCANLINE50:
		case MENU_SOFT_SCANLINE25:
		case MENU_SOFT_INTERSCANLINE:
		case MENU_SOFT_INTERSCANLINE50:
		case MENU_SOFT_INTERSCANLINE25:
			vidUseFilter = 1;
			nVidFilter = id - MENU_SOFT_NONE;
			scrnSize();
			VidSwitchFilter(nVidFilter);
			menuSync(MENUT_SETTING);
			break;

		case MENU_FILTER_AUTOSIZE:
			vidForceFilterSize = !vidForceFilterSize;
			simpleReinitScrn(true);
			menuSync(MENUT_SETTING);
			break;

		// Options for blitters
		case MENU_FILTER_POINT:
			if (vidFilterLinear) {
				vidFilterLinear = 0;
				VidReinit();
				menuSync(MENUT_SETTING);
			}
			break;
		case MENU_FILTER_LINEAR:
			if (!vidFilterLinear) {
				vidFilterLinear = 1;
				VidReinit();
				menuSync(MENUT_SETTING);
			}
			break;

		case MENU_CONFIGALL:
			if (useDialogs()) {
				preferenceCreate(hDlg);
				setWindowAspect();
				menuReinit();
				simpleReinitScrn(true);
				mediaReInitAudio();
			}
			break;
		case MENU_VIDEOCONFIG:
			if (useDialogs()) {
				prefVideoCreate(hDlg);
				setWindowAspect();
				simpleReinitScrn(true);
			}
			break;
		case MENU_AUDIOCONFIG:
			if (useDialogs()) {
				prefAudioCreate(hDlg);
				mediaReInitAudio();
			}
			break;
		case MENU_MISCCONFIG:
			if (useDialogs()) {
				prefMiscCreate(hDlg);
				menuReinit();
				VidReinit();
			}
			break;

//		default:
//			printf("  * Command %i sent.\n");
	}

	if (ChatActivated()) {
		switch (codeNotify) {
			case EN_CHANGE: {
				bEditTextChanged = true;
				SendMessage(GetChatWindow(), WM_GETTEXT, (WPARAM)MAX_CHAT_SIZE + 1, (LPARAM)EditText);
				break;
			}
			case EN_KILLFOCUS: {
				ActivateChat();
				break;
			}
			case EN_MAXTEXT: {
				VidSNewShortMsg(FBALoadStringEx(IDS_NETPLAY_TOOMUCH), 0xFF3F3F);
				break;
			}
		}
	}
}

// Block screensaver and windows menu if needed
static int onSysCommand(HWND, UINT sysCommand, int, int)
{
	switch (sysCommand) {
		case SC_MONITORPOWER:
		case SC_SCREENSAVE: {
			if (!bRunPause && bDrvOkay) {
				return 1;
			}
			break;
		}
		case SC_KEYMENU:
		case SC_MOUSEMENU: {
			if (kNetGame && !bModelessMenu) {
				return 1;
			}
			break;
		}
	}

	return 0;
}

static void onEnterIdle(HWND /*hwnd*/, UINT /*source*/, HWND /*hwndSource*/)
{
	MSG Message;

	// Modeless dialog is idle
	while (kNetGame && !PeekMessage(&Message, NULL, 0, 0, PM_NOREMOVE)) {
		RunIdle();
	}
}

static void onSize(HWND, UINT state, int cx, int cy)
{
	if (state == SIZE_MINIMIZED) {
		bMaximised = false;
	} else {
		bool bSizeChanged = false;

		if (ChatActivated()) {
			MoveWindow(GetChatWindow(), 0, cy - 32, cx, 32, FALSE);
		}

		if (state == SIZE_MAXIMIZED) {
			if (!bMaximised) {
				bSizeChanged = true;
			}
			bMaximised = true;
		}
		if (state == SIZE_RESTORED) {
			if (bMaximised) {
				bSizeChanged = true;
			}
			bMaximised = false;
		}

		if (bSizeChanged) {
			refreshWindow(true);
		} else {
			refreshWindow(false);
		}
	}
}

static void onEnterSizeMove(HWND)
{
	audio.blank();

	RECT rect;
	GetClientRect(hScrnWnd, &rect);
	nPrevWidth = rect.right;
	nPrevHeight = rect.bottom;
}

static void onExitSizeMove(HWND)
{
	RECT rect;

	GetClientRect(hScrnWnd, &rect);
	if (rect.right != nPrevWidth || rect.bottom != nPrevHeight) {
		refreshWindow(true);
	}

	GetWindowRect(hScrnWnd, &rect);
	nWindowPosX = rect.left;
	nWindowPosY = rect.top;
}

// ==> set screen visible size, added by regret
static inline int getScrnExtraWidth()
{
	int ew = GetSystemMetrics(SM_CXSIZEFRAME) << 1;
	if (!bMenuEnabled) {
		ew += 1 << 1;
	}
	return ew;
}

static inline int getScrnExtraHeight()
{
	int eh = GetSystemMetrics(SM_CYSIZEFRAME) << 1;
	if (bMenuEnabled) {
		eh += GetSystemMetrics(SM_CYCAPTION);
		eh += GetSystemMetrics(SM_CYMENU);
	} else {
		eh += 1 << 1;
	}
	return eh;
}

static void onSizing(HWND, WPARAM wParam, LPARAM lParam)
{
	RECT* rect = (RECT *)lParam;
	int adjustment = wParam;
	int extrawidth = getScrnExtraWidth();
	int extraheight = getScrnExtraHeight();
	int adjwidth, adjheight;
	int nBmapWidth = nVidImageWidth, nBmapHeight = nVidImageHeight;

	// get game size
	if (bDrvOkay) {
		if ((BurnDrvGetFlags() & BDF_ORIENTATION_VERTICAL) && (nVidRotationAdjust & 1)) {
			BurnDrvGetVisibleSize(&nBmapHeight, &nBmapWidth);
		} else {
			BurnDrvGetVisibleSize(&nBmapWidth, &nBmapHeight);
		}

		if (nBmapWidth <= 0 || nBmapHeight <= 0) {
			return;
		}
	}

	RECT rect_image = {
		0, 0, rect->right-rect->left-extrawidth, rect->bottom-rect->top-extraheight
	};
	VidScale(&rect_image, nBmapWidth, nBmapHeight);

	// calculate screen size to fit game size
	adjwidth = rect->left + rect_image.right - rect_image.left + extrawidth - rect->right;
	adjheight = rect->top + rect_image.bottom - rect_image.top + extraheight - rect->bottom;

	switch (adjustment)
	{
		case WMSZ_BOTTOM:
		case WMSZ_BOTTOMRIGHT:
		case WMSZ_RIGHT:
			rect->right += adjwidth;
			rect->bottom += adjheight;
			break;

		case WMSZ_BOTTOMLEFT:
			rect->left -= adjwidth;
			rect->bottom += adjheight;
			break;

		case WMSZ_LEFT:
		case WMSZ_TOPLEFT:
		case WMSZ_TOP:
			rect->left -= adjwidth;
			rect->top -= adjheight;
			break;

		case WMSZ_TOPRIGHT:
			rect->right += adjwidth;
			rect->top -= adjheight;
			break;
	}
}
// <== set screen visible size

// ==> drag & drop support, added by regret
static void onDropFiles(HWND, WPARAM wParam)
{
	TCHAR filename[MAX_PATH];
	HDROP hDrop = (HDROP)wParam;
	DragQueryFile(hDrop, 0, filename, MAX_PATH); // get first filename
	DragFinish(hDrop);
	directLoadGame(filename);
}
// <== drag & drop support

// extern functions
void onEnterMenuLoop(HWND, BOOL);
void onExitMenuLoop(HWND, BOOL);

static LRESULT CALLBACK scrnProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch (Msg) {
		HANDLE_MSG(hWnd, WM_CREATE,			onCreate);
		HANDLE_MSG(hWnd, WM_ACTIVATEAPP,	onActivateApp);
		HANDLE_MSGB(hWnd,WM_PAINT,			onPaint);
		HANDLE_MSG(hWnd, WM_CLOSE,			onClose);
		HANDLE_MSG(hWnd, WM_DESTROY,		onDestroy);
		HANDLE_MSG(hWnd, WM_COMMAND,		onCommand);

		// We can't use the macro from windowsx.h macro for this one
		case WM_SYSCOMMAND: {
			if (onSysCommand(hWnd,(UINT)wParam,(int)(short)LOWORD(lParam),(int)(short)HIWORD(lParam))) {
				return 0;
			}
			break;
		}

		HANDLE_MSG(hWnd, WM_SIZE,			onSize);
		HANDLE_MSG(hWnd, WM_ENTERSIZEMOVE,	onEnterSizeMove);
		HANDLE_MSG(hWnd, WM_EXITSIZEMOVE,	onExitSizeMove);

		HANDLE_MSGB(hWnd, WM_SIZING,		onSizing);		// set screen visible size, added by regret
		HANDLE_MSGB(hWnd, WM_DROPFILES,		onDropFiles);	// drag & drop support, added by regret

		HANDLE_MSG(hWnd, WM_ENTERIDLE,		onEnterIdle);
		HANDLE_MSG(hWnd, WM_MOUSEMOVE,		onMouseMove);
		HANDLE_MSG(hWnd, WM_LBUTTONUP,		onLButtonUp);
		HANDLE_MSG(hWnd, WM_LBUTTONDOWN,	onLButtonDown);
		HANDLE_MSG(hWnd, WM_LBUTTONDBLCLK,	onLButtonDblClk);
//		HANDLE_MSG(hWnd, WM_RBUTTONUP,		onRButtonUp);
		HANDLE_MSG(hWnd, WM_RBUTTONDBLCLK,	onRButtonDown);
		HANDLE_MSG(hWnd, WM_RBUTTONDOWN,	onRButtonDown);

		HANDLE_MSG(hWnd, WM_ENTERMENULOOP,	onEnterMenuLoop);
		HANDLE_MSGB(hWnd,WM_EXITMENULOOP,	onExitMenuLoop);

		HANDLE_MSG(hWnd, WM_DISPLAYCHANGE,	onDisplayChange);
	}

	return DefWindowProc(hWnd, Msg, wParam, lParam);
}

static int scrnRegister()
{
	WNDCLASSEX WndClassEx;
	ATOM Atom = 0;

	// Register the window class
	memset(&WndClassEx, 0, sizeof(WndClassEx)); 		// Init structure to all zeros
	WndClassEx.cbSize = sizeof(WndClassEx);
	WndClassEx.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;// These cause flicker in the toolbar
	WndClassEx.lpfnWndProc = scrnProc;
	WndClassEx.hInstance = hAppInst;
	WndClassEx.hIcon = LoadIcon(hAppInst, MAKEINTRESOURCE(IDI_APP));
	WndClassEx.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClassEx.hbrBackground = CreateSolidBrush(0);
	WndClassEx.lpszClassName = szClass;

	// Register the window class with the above information:
	Atom = RegisterClassEx(&WndClassEx);
	if (!Atom) {
		return 1;
	}
	return 0;
}

int scrnTitle()
{
	TCHAR szText[1024] = _T("");

	// Create window title
	if (bDrvOkay) {
		TCHAR* pszPosition = szText;
		TCHAR* pszName = transGameName(BurnDrvGetText(DRV_FULLNAME));

		if (bShowFPS) {
			extern TCHAR fpsString[];
			pszPosition += _sntprintf(szText, sizearray(szText), _T("%s"), pszName);
			_stprintf(pszPosition, _T(" ") _T("[") _T("%s") _T("]"), fpsString);
		} else {
			_stprintf(szText, _T("%s"), pszName);
		}
	} else {
		_stprintf(szText, _T(APP_TITLE) _T( " v%.20s"), szAppBurnVer);
	}

	SetWindowText(hScrnWnd, szText);
	return 0;
}

int scrnSize()
{
	if (!hScrnWnd || nVidFullscreen || bFakeFullscreen) {
		return 1;
	}

	int nBmapWidth = nVidImageWidth, nBmapHeight = nVidImageHeight;
	int nGameAspectX = 4, nGameAspectY = 3;

	// adjust the screen size for skin, added by regret
	if (!bDrvOkay) {
		if (nBmapWidth > DEFAULT_IMAGE_WIDTH) {
			nBmapWidth = DEFAULT_IMAGE_WIDTH;
		}
		if (nBmapHeight > DEFAULT_IMAGE_HEIGHT) {
			nBmapHeight = DEFAULT_IMAGE_HEIGHT;
		}
	}

	if (bDrvOkay) {
		if ((BurnDrvGetFlags() & BDF_ORIENTATION_VERTICAL) && (nVidRotationAdjust & 1)) {
			BurnDrvGetVisibleSize(&nBmapHeight, &nBmapWidth);
			BurnDrvGetAspect(&nGameAspectY, &nGameAspectX);
		} else {
			BurnDrvGetVisibleSize(&nBmapWidth, &nBmapHeight);
			BurnDrvGetAspect(&nGameAspectX, &nGameAspectY);
		}

		if (nBmapWidth <= 0 || nBmapHeight <= 0) {
			return 1;
		}
	}

	nDragX = GetSystemMetrics(SM_CXDRAG) / 2;
	nDragY = GetSystemMetrics(SM_CYDRAG) / 2;

	// Find the size of the visible WorkArea
	SystemParametersInfo(SPI_GETWORKAREA, 0, &SystemWorkArea, 0);

	int nScrnWidth = SystemWorkArea.right - SystemWorkArea.left;
	int nScrnHeight = SystemWorkArea.bottom - SystemWorkArea.top;

	int nMaxSize = nWindowSize;

	if (nMaxSize <= 0) {
		// auto size
		if (nBmapWidth < nBmapHeight) {
			if (nScrnHeight <= 600) {
				nMaxSize = 1;
			}
			else if (nScrnHeight <= 960) {
				nMaxSize = 2;
			}
			else if (nScrnHeight <= 1280) {
				nMaxSize = 3;
			}
			else {
				nMaxSize = 4;
			}
		} else {
			if (nScrnWidth <= 640) {
				nMaxSize = 1;
			}
			else if (nScrnWidth <= 1152) {
				nMaxSize = 2;
			}
			else if (nScrnWidth <= 1600) {
				nMaxSize = 3;
			}
			else {
				nMaxSize = 4;
			}
		}
	}

	// Find the width and height
	int w = nScrnWidth;
	int h = nScrnHeight;

	// Find out how much space is taken up by the borders
	int ew = getScrnExtraWidth();
	int eh = getScrnExtraHeight();

	if (bMenuEnabled) {
		// Subtract the border space
		w -= ew;
		h -= eh;
	}

	if (bVidCorrectAspect || bVidFullStretch) {
		int ww = w;
		int hh = h;

		do {
			if (nBmapWidth < nBmapHeight) {
				if (ww > nBmapWidth * nMaxSize) {
					ww = nBmapWidth * nMaxSize;
				}
				if (hh > ww * vidScrnAspect * nGameAspectY * nScrnHeight / (nScrnWidth * nGameAspectX)) {
					hh = ww * vidScrnAspect * nGameAspectY * nScrnHeight / (nScrnWidth * nGameAspectX);
				}
			} else {
				if (hh > nBmapHeight * nMaxSize) {
					hh = nBmapHeight * nMaxSize;
				}
				if (ww > hh * nGameAspectX * nScrnWidth / (nScrnHeight * vidScrnAspect * nGameAspectY)) {
					ww = hh * nGameAspectX * nScrnWidth / (nScrnHeight * vidScrnAspect * nGameAspectY);
				}
			}
		} while ((ww > w || hh > h) && nMaxSize-- > 1);
		w =	ww;
		h = hh;
	} else {
		while ((nBmapWidth * nMaxSize > w || nBmapHeight * nMaxSize > h) && nMaxSize > 1) {
			nMaxSize--;
		}

		if (w > nBmapWidth * nMaxSize || h > nBmapHeight * nMaxSize) {
			w = nBmapWidth * nMaxSize;
			h = nBmapHeight * nMaxSize;
		}
	}

	RECT rect = { 0, 0, w, h };
	VidScale(&rect, nBmapWidth, nBmapHeight);
	w = rect.right - rect.left + ew;
	h = rect.bottom - rect.top + eh;

	int x = nWindowPosX;
	int y = nWindowPosY;
	if (x + w > SystemWorkArea.right || y + h > SystemWorkArea.bottom) {
		// Find the midpoint for the window
		x = SystemWorkArea.left + SystemWorkArea.right;
		x /= 2;
		y = SystemWorkArea.bottom + SystemWorkArea.top;
		y /= 2;

		x -= w / 2;
		y -= h / 2;
	}

	bMaximised = false;

	MoveWindow(hScrnWnd, x, y, w, h, TRUE);
//	SetWindowPos(hScrnWnd, NULL, x, y, w, h, SWP_NOREDRAW | SWP_NOACTIVATE | SWP_NOSENDCHANGING | SWP_NOZORDER);
	if (bDrvOkay && bShowOnTop) {
		SetWindowPos(hScrnWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW | SWP_NOMOVE);
	}

	nWindowPosX = x; nWindowPosY = y;

  	return 0;
}

// Init the screen window (create it)
int scrnInit()
{
	if (hScrnWnd) {
		return 0;
	}

	if (scrnRegister() != 0) {
		FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_ERR_UI_WINDOW));
		FBAPopupDisplay(PUF_TYPE_ERROR);
		return 1;
	}

	DWORD nWindowStyles, nWindowExStyles;

	if (nVidFullscreen) {
		nWindowStyles = WS_POPUP;
		nWindowExStyles = 0;
	} else {
		if (bMenuEnabled) {
			nWindowStyles = WS_OVERLAPPEDWINDOW;
			nWindowExStyles = 0;
		} else {
			nWindowStyles = WS_MINIMIZEBOX | WS_POPUP | WS_SYSMENU | WS_THICKFRAME;
			nWindowExStyles = WS_EX_CLIENTEDGE;
		}
	}

	hScrnWnd = CreateWindowEx(nWindowExStyles, szClass, _T(APP_TITLE), nWindowStyles,
		0, 0, 0, 0,
		NULL, NULL, hAppInst, NULL);

	if (hScrnWnd == NULL) {
		scrnExit();
		FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_ERR_UI_WINDOW));
		FBAPopupDisplay(PUF_TYPE_ERROR);
		return 1;
	}

	if (!nVidFullscreen) {
		// enable drag & drop
		DragAcceptFiles(hScrnWnd, TRUE);

		// Create the menu toolbar itself
		menuCreate();

		scrnTitle();
		scrnSize();
	}

	return 0;
}

// Exit the screen window (destroy it)
int scrnExit()
{
	// Ensure the window is destroyed
	DeActivateChat();

	if (hScrnWnd) {
		DestroyWindow(hScrnWnd);
		hScrnWnd = NULL;
	}

	UnregisterClass(szClass, hAppInst);		// Unregister the scrn class

	menuDestroy();

	return 0;
}

// ==> switch fullscreen, added by regret
int scrnSwitchFull()
{
	if (!hScrnWnd) {
		return 1;
	}

	DWORD styles, exStyles;

	if (nVidFullscreen) {
		styles = WS_POPUP;
		exStyles = 0;
		SetWindowLongPtr(hScrnWnd, GWL_STYLE, styles);
		SetWindowLongPtr(hScrnWnd, GWL_EXSTYLE, exStyles);

		SetMenu(hScrnWnd, NULL);
		bFakeFullscreen = false;
	} else {
		if (bMenuEnabled) {
			styles = WS_OVERLAPPEDWINDOW;
			exStyles = 0;
		} else {
			styles = WS_MINIMIZEBOX | WS_POPUP | WS_SYSMENU | WS_THICKFRAME;
			exStyles = WS_EX_CLIENTEDGE;
		}
		SetWindowLongPtr(hScrnWnd, GWL_STYLE, styles);
		SetWindowLongPtr(hScrnWnd, GWL_EXSTYLE, exStyles);

		menuCreate();
		if (bMenuEnabled) {
			SetMenu(hScrnWnd, hMainMenu);
		} else {
			SetMenu(hScrnWnd, NULL);
		}

		scrnTitle();
		scrnSize();

		ShowWindow(hScrnWnd, SW_NORMAL);
	}

	VidReinit();

	GameInpCheckMouse();

	return 0;
}

// fake fullscreen
int scrnFakeFullscreen()
{
	if (!hScrnWnd) {
		return 1;
	}
	if (nVidFullscreen) {
		return 0;
	}

	bFakeFullscreen = !bFakeFullscreen;

	DWORD styles, exStyles;

	if (bFakeFullscreen) {
		styles = WS_POPUP;
		exStyles = 0;
		SetWindowLongPtr(hScrnWnd, GWL_STYLE, styles);
		SetWindowLongPtr(hScrnWnd, GWL_EXSTYLE, exStyles);
		SetMenu(hScrnWnd, NULL);

		// set screen size
		int width = GetSystemMetrics(SM_CXSCREEN);
		int height = GetSystemMetrics(SM_CYSCREEN);
		MoveWindow(hScrnWnd, 0, 0, width, height, TRUE);
		if (bDrvOkay && bShowOnTop) {
			SetWindowPos(hScrnWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW | SWP_NOMOVE);
		}

		// hide the cursor
//		while (ShowCursor(FALSE) >= 0) {}

		ShowWindow(hScrnWnd, SW_NORMAL);
	} else {
		if (bMenuEnabled) {
			styles = WS_OVERLAPPEDWINDOW;
			exStyles = 0;
		} else {
			styles = WS_MINIMIZEBOX | WS_POPUP | WS_SYSMENU | WS_THICKFRAME;
			exStyles = WS_EX_CLIENTEDGE;
		}
		SetWindowLongPtr(hScrnWnd, GWL_STYLE, styles);
		SetWindowLongPtr(hScrnWnd, GWL_EXSTYLE, exStyles);

		menuCreate();
		if (bMenuEnabled) {
			SetMenu(hScrnWnd, hMainMenu);
		} else {
			SetMenu(hScrnWnd, NULL);
		}

		scrnTitle();
		scrnSize();

		ShowWindow(hScrnWnd, SW_MAXIMIZE); // refresh
		ShowWindow(hScrnWnd, SW_NORMAL);

		// show the cursor
//		while (ShowCursor(TRUE)  <  0) {}
	}

	VidReinit();

	return 0;
}

// set fullscreen (for opengl)
int scrnSetFull(const bool& full)
{
	if (full) {
		DISPLAY_DEVICE dev;
		memset(&dev, 0, sizeof(dev));
		dev.cb = sizeof(dev);
		EnumDisplayDevices(NULL, nVidAdapter, &dev, 0);

		DEVMODE mode;
		memset(&mode, 0, sizeof(mode));
		mode.dmSize = sizeof(mode);
		mode.dmBitsPerPel = nVidScrnDepth;
		mode.dmPelsWidth = nVidScrnWidth;
		mode.dmPelsHeight = nVidScrnHeight;
		mode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		LONG ret = ChangeDisplaySettingsEx(dev.DeviceName, &mode, NULL, CDS_FULLSCREEN, NULL);
		if (ret != DISP_CHANGE_SUCCESSFUL) {
			return 1;
		}

		SetWindowPos(hScrnWnd, NULL, 0, 0, nVidScrnWidth, nVidScrnHeight, SWP_NOZORDER | SWP_NOACTIVATE);
//		MoveWindow(hScrnWnd, 0, 0, nVidScrnWidth, nVidScrnHeight, FALSE);

		ShowWindow(hScrnWnd, SW_NORMAL);
		bFakeFullscreen = false;
	} else {
		ChangeDisplaySettingsEx(NULL, NULL, NULL, 0, NULL);
	}

	return 0;
}
// <== switch fullscreen

void __cdecl scrnReinit()
{
	simpleReinitScrn(true);
}
