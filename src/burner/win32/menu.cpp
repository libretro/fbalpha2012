// Menu handling
// add image menu and remove rebar by regret

/* changelog:
 update 4: rewrite menu create
 update 3: disable rebar control (menu hook is useless now)
 update 2: seperate menu create and rebar create
 update 1: add image menu
*/

#include "burner.h"
#include "cheat.h"
#include "menu.h"
#include "maphkeys.h"
#include "cartridge.h"
#include "vid_filter.h"
#ifndef NO_IMAGEMENU
#include "imagemenu.h"
#endif

bool bMenuEnabled = true;
HMENU hMenuPopup = NULL;
HMENU hMainMenu = NULL;
HMENU hMenuSelPop = NULL;
int menuNewStyle = 0;
bool bModelessMenu = false;

#ifndef NO_IMAGEMENU
// ==> image menu, added by regret
static void setImageMenuTitle(HMENU hmenu)
{
	ImageMenu_SetMenuTitleProps(hmenu, _T(" "), TRUE);
	ImageMenu_SetMenuTitleBkProps(hmenu, RGB(0, 0, 0), RGB(0, 0, 255), TRUE, TRUE);

	int nMenuCnt = GetMenuItemCount(hmenu);
	for (int i = 0; i < nMenuCnt; i++) {
		HMENU hTmpMenu = GetSubMenu(hmenu, i);
		if (hTmpMenu) {
			setImageMenuTitle(hTmpMenu);
		}
	}
}

void createImageMenu(HWND hwnd, HMENU menu)
{
	if (menuNewStyle < 0) {
		menuNewStyle = 0;
		return;
	}

	// Create ownerdraw menu
	ImageMenu_CreatePopup(hwnd, menu);
//	setImageMenuTitle(hmenu);
	if (menuNewStyle >= MENUSTYLE_MAX)
		menuNewStyle = MENUSTYLE_MAX - 1;
	ImageMenu_SetStyle(menuNewStyle);
}

void removeImageMenu(HMENU menu)
{
	ImageMenu_Remove(menu);
}
// <== image menu
#endif

// ---------------------------------------------------------------------------
void onEnterMenuLoop(HWND, BOOL)
{
	if (!bModelessMenu) {
		InputSetCooperativeLevel(false, !bAlwaysProcessKey);
		audio.blank();
	} else {
		if (!kNetGame && bAutoPause) {
			bRunPause = 1;
		}
	}
}

void onExitMenuLoop(HWND, BOOL)
{
	if (!bModelessMenu) {
		GameInpCheckMouse();
	}
}

// ---------------------------------------------------------------------------
void menuReinit()
{
	menuDestroy();
	menuCreate();
}

int menuCreate()
{
	if (hMainMenu == NULL) {
		hMainMenu = FBALoadMenu(IDR_MENU);	// Main application menu

		MHkeysUpdateMenuAcc(); // update menu accel string
	}

	if (hMenuPopup == NULL) {
		hMenuPopup = CreatePopupMenu();

		TCHAR szButtonText[64];
		MENUITEMINFO menuItemInfo;
		menuItemInfo.cbSize = sizeof(MENUITEMINFO);
		menuItemInfo.fMask = MIIM_TYPE;
		menuItemInfo.dwTypeData = szButtonText;

		int nMenuCnt = GetMenuItemCount(hMainMenu);
		for (int i = 0; i < nMenuCnt; i++) {
			menuItemInfo.cch = sizearray(szButtonText);
			GetMenuItemInfo(hMainMenu, i, TRUE, &menuItemInfo);
			AppendMenu(hMenuPopup, MF_POPUP | MF_STRING, (UINT_PTR)GetSubMenu(hMainMenu, i), szButtonText);
		}

#ifndef NO_IMAGEMENU
		if (menuNewStyle) {
			createImageMenu(hScrnWnd, hMenuPopup);
		}
#endif
	}

	if (hMenuSelPop == NULL) {
		hMenuSelPop = FBALoadMenu(IDR_SEL_CONTEXT_MENU);
	}

	menuSync(MENUT_ALL);

	if (bModelessMenu) {							// Make menu modeless
		MENUINFO menu;
		memset(&menu, 0, sizeof(MENUINFO));
		menu.cbSize = sizeof(MENUINFO);
		menu.fMask = MIM_APPLYTOSUBMENUS | MIM_STYLE;
		menu.dwStyle = MNS_MODELESS | MNS_CHECKORBMP;

		SetMenuInfo(hMenuPopup, &menu);

		int nMenuCnt = GetMenuItemCount(hMainMenu);
		for (int i = 0; i < nMenuCnt; i++) {
			SetMenuInfo(GetSubMenu(hMainMenu, i), &menu);
		}

		memset(&menu, 0, sizeof(MENUINFO));
		menu.cbSize = sizeof(MENUINFO);
		menu.fMask = MIM_STYLE | MIM_MAXHEIGHT;

		GetMenuInfo(hMainMenu, &menu);
		menu.dwStyle |= MNS_MODELESS;
		SetMenuInfo(hMainMenu, &menu);
	}

	if (bMenuEnabled) {
		SetMenu(hScrnWnd, hMainMenu);
	}

	return 0;
}

void menuDestroy()
{
	MENUITEMINFO myMenuItemInfo;
	myMenuItemInfo.cbSize = sizeof(MENUITEMINFO);
	myMenuItemInfo.fMask = MIIM_SUBMENU | MIIM_STATE;
	myMenuItemInfo.fState = MFS_GRAYED;
	myMenuItemInfo.hSubMenu = NULL;
	if (hMainMenu) {
		SetMenuItemInfo(GetSubMenu(hMainMenu, 1), 1, TRUE, &myMenuItemInfo);
	}
	if (hMenuPopup) {
		SetMenuItemInfo(GetSubMenu(hMenuPopup, 1), 1, TRUE, &myMenuItemInfo);
	}

	if (hMainMenu) {
#ifndef NO_IMAGEMENU
		removeImageMenu(hMainMenu);
#endif
		DestroyMenu(hMainMenu);
		hMainMenu = NULL;
	}

	if (hMenuPopup) {
#ifndef NO_IMAGEMENU
		removeImageMenu(hMenuPopup);
#endif
		DestroyMenu(hMenuPopup);
		hMenuPopup = NULL;
	}

	if (hMenuSelPop) {
#ifndef NO_IMAGEMENU
		removeImageMenu(hMenuSelPop);
#endif
		DestroyMenu(hMenuSelPop);
		hMenuSelPop = NULL;
	}
}

// Update bullets, checkmarks, and item text
static void menuSyncFile()
{
	CheckMenuItem(hMainMenu, MENU_PAUSE, bAltPause ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hMainMenu, MENU_STATE_ALLRAM, bDrvSaveAll ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hMainMenu, MENU_AVIINTAUD, nAviIntAudio ? MF_CHECKED : MF_UNCHECKED);

	EnableMenuItem(hMainMenu, MENU_QUIT, (bDrvOkay ? MF_ENABLED : MF_GRAYED) | MF_BYCOMMAND);

	// for genesis driver
	EnableMenuItem(hMainMenu, MENU_CARTR_MOUNT, (BurnCartridgeGameLoaded ? MF_ENABLED : MF_GRAYED) | MF_BYCOMMAND);
	EnableMenuItem(hMainMenu, MENU_CARTR_UNMOUNT, (BurnCartridgeGameLoaded ? MF_ENABLED : MF_GRAYED) | MF_BYCOMMAND);

	if (bDrvOkay) {
		EnableMenuItem(hMainMenu, MENU_MEMCARD_CREATE, MF_GRAYED | MF_BYCOMMAND);
		EnableMenuItem(hMainMenu, MENU_MEMCARD_SELECT, MF_GRAYED | MF_BYCOMMAND);
		EnableMenuItem(hMainMenu, MENU_MEMCARD_INSERT, MF_GRAYED | MF_BYCOMMAND);
		EnableMenuItem(hMainMenu, MENU_MEMCARD_EJECT, MF_GRAYED | MF_BYCOMMAND);

		if ((BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_SNK_NEOGEO) {
			if (!kNetGame) {
				EnableMenuItem(hMainMenu, MENU_MEMCARD_CREATE, MF_ENABLED | MF_BYCOMMAND);
				EnableMenuItem(hMainMenu, MENU_MEMCARD_SELECT, MF_ENABLED | MF_BYCOMMAND);
				if (nMemoryCardStatus & 1) {
					if (nMemoryCardStatus & 2) {
						EnableMenuItem(hMainMenu, MENU_MEMCARD_EJECT, MF_ENABLED | MF_BYCOMMAND);
					} else {
						EnableMenuItem(hMainMenu, MENU_MEMCARD_INSERT, MF_ENABLED | MF_BYCOMMAND);
					}
				}
			}
		}

		EnableMenuItem(hMainMenu, MENU_STOPREPLAY, (nReplayStatus ? MF_ENABLED : MF_GRAYED) | MF_BYCOMMAND);
		EnableMenuItem(hMainMenu, MENU_STARTRECORD, (nReplayStatus ? MF_GRAYED : MF_ENABLED) | MF_BYCOMMAND);

		if (nReplayStatus) {
			EnableMenuItem(hMainMenu, MENU_STARTREPLAY, MF_GRAYED | MF_BYCOMMAND);
			EnableMenuItem(hMainMenu, MENU_STATE_LOAD_SLOT, MF_GRAYED | MF_BYCOMMAND);
			EnableMenuItem(hMainMenu, MENU_STATE_LOAD_DIALOG, MF_GRAYED | MF_BYCOMMAND);
		} else {
			if (kNetGame) {
				EnableMenuItem(hMainMenu, MENU_STARTREPLAY, MF_GRAYED | MF_BYCOMMAND);
				EnableMenuItem(hMainMenu, MENU_STATE_SAVE_SLOT, MF_ENABLED | MF_BYCOMMAND);
				EnableMenuItem(hMainMenu, MENU_STATE_SAVE_DIALOG, MF_ENABLED | MF_BYCOMMAND);
				EnableMenuItem(hMainMenu, MENU_STATE_LOAD_SLOT, MF_GRAYED | MF_BYCOMMAND);
				EnableMenuItem(hMainMenu, MENU_STATE_LOAD_DIALOG, MF_GRAYED | MF_BYCOMMAND);
			} else {
				EnableMenuItem(hMainMenu, MENU_STARTREPLAY, MF_ENABLED | MF_BYCOMMAND);
				EnableMenuItem(hMainMenu, MENU_STATE_SAVE_SLOT, MF_ENABLED | MF_BYCOMMAND);
				EnableMenuItem(hMainMenu, MENU_STATE_SAVE_DIALOG, MF_ENABLED | MF_BYCOMMAND);
				EnableMenuItem(hMainMenu, MENU_STATE_LOAD_SLOT, MF_ENABLED | MF_BYCOMMAND);
				EnableMenuItem(hMainMenu, MENU_STATE_LOAD_DIALOG, MF_ENABLED | MF_BYCOMMAND);
			}
		}

		EnableMenuItem(hMainMenu, MENU_AVISTART, (nAviStatus ? MF_GRAYED : MF_ENABLED) | MF_BYCOMMAND);
		EnableMenuItem(hMainMenu, MENU_AVISTOP, (nAviStatus ? MF_ENABLED : MF_GRAYED) | MF_BYCOMMAND);
		EnableMenuItem(hMainMenu, MENU_AVIINTAUD, (nAviStatus ? MF_GRAYED : MF_ENABLED)  | MF_BYCOMMAND);

		EnableMenuItem(hMainMenu, MENU_LOAD, (kNetGame ? MF_GRAYED : MF_ENABLED) | MF_BYCOMMAND);
		EnableMenuItem(hMainMenu, MENU_STARTNET, (kNetGame ? MF_GRAYED : MF_ENABLED) | MF_BYCOMMAND);
		EnableMenuItem(hMainMenu, MENU_EXIT, (kNetGame ? MF_GRAYED : MF_ENABLED) | MF_BYCOMMAND);
		EnableMenuItem(hMainMenu, MENU_PAUSE, (kNetGame ? MF_GRAYED : MF_ENABLED) | MF_BYCOMMAND);

		EnableMenuItem(hMainMenu, MENU_WLOGEND, (soundLogStart ? MF_ENABLED : MF_GRAYED) | MF_BYCOMMAND);
		EnableMenuItem(hMainMenu, MENU_WLOGSTART, (soundLogStart ? MF_GRAYED : MF_ENABLED) | MF_BYCOMMAND);
	} else {
		EnableMenuItem(hMainMenu, MENU_MEMCARD_CREATE, MF_GRAYED | MF_BYCOMMAND);
		EnableMenuItem(hMainMenu, MENU_MEMCARD_SELECT, MF_GRAYED | MF_BYCOMMAND);
		EnableMenuItem(hMainMenu, MENU_MEMCARD_INSERT, MF_GRAYED | MF_BYCOMMAND);
		EnableMenuItem(hMainMenu, MENU_MEMCARD_EJECT, MF_GRAYED | MF_BYCOMMAND);

		EnableMenuItem(hMainMenu, MENU_STATE_LOAD_SLOT, MF_GRAYED | MF_BYCOMMAND);
		EnableMenuItem(hMainMenu, MENU_STATE_LOAD_DIALOG, MF_GRAYED | MF_BYCOMMAND);
		EnableMenuItem(hMainMenu, MENU_STATE_SAVE_SLOT, MF_GRAYED | MF_BYCOMMAND);
		EnableMenuItem(hMainMenu, MENU_STATE_SAVE_DIALOG, MF_GRAYED | MF_BYCOMMAND);

		EnableMenuItem(hMainMenu, MENU_LOAD, MF_ENABLED | MF_BYCOMMAND);
		EnableMenuItem(hMainMenu, MENU_STARTNET, MF_ENABLED | MF_BYCOMMAND);
		EnableMenuItem(hMainMenu, MENU_STARTREPLAY, MF_ENABLED | MF_BYCOMMAND);
		EnableMenuItem(hMainMenu, MENU_STARTRECORD, MF_GRAYED | MF_BYCOMMAND);
		EnableMenuItem(hMainMenu, MENU_STOPREPLAY, MF_GRAYED | MF_BYCOMMAND);
		EnableMenuItem(hMainMenu, MENU_AVISTART, MF_GRAYED | MF_BYCOMMAND);
		EnableMenuItem(hMainMenu, MENU_AVISTOP, MF_GRAYED | MF_BYCOMMAND);
		EnableMenuItem(hMainMenu, MENU_AVIINTAUD, MF_GRAYED | MF_BYCOMMAND);
		EnableMenuItem(hMainMenu, MENU_EXIT, MF_ENABLED | MF_BYCOMMAND);
		EnableMenuItem(hMainMenu, MENU_PAUSE, MF_GRAYED | MF_BYCOMMAND);

		EnableMenuItem(hMainMenu, MENU_WLOGSTART, MF_GRAYED  | MF_BYCOMMAND);
		EnableMenuItem(hMainMenu, MENU_WLOGEND, MF_GRAYED  | MF_BYCOMMAND);
	}
}

static void menuSyncSetting()
{
	int var;

	if (bVidFullStretch) {
		var = MENU_STRETCH;
	} else {
		if (bVidCorrectAspect) {
			var = MENU_ASPECT;
		} else {
			var = MENU_NOSTRETCH;
		}
	}
	CheckMenuRadioItem(hMainMenu, MENU_NOSTRETCH, MENU_ASPECT, var, MF_BYCOMMAND);

	// filter
	CheckMenuItem(hMainMenu, MENU_FILTER_POINT, (vidFilterLinear == 0) ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hMainMenu, MENU_FILTER_LINEAR, vidFilterLinear ? MF_CHECKED : MF_UNCHECKED);

	CheckMenuItem(hMainMenu, MENU_FILTER, vidUseFilter ? MF_CHECKED : MF_UNCHECKED);
	var = nVidFilter + MENU_SOFT_NONE;
	CheckMenuRadioItem(hMainMenu, MENU_SOFT_NONE, MENU_FILTER_AUTOSIZE, var, MF_BYCOMMAND);
	CheckMenuItem(hMainMenu, MENU_FILTER_AUTOSIZE, vidForceFilterSize ? MF_CHECKED : MF_UNCHECKED);

	if (nWindowSize <= 4) {
		var = MENU_AUTOSIZE + nWindowSize;
	} else {
		var = MENU_WINDOWSIZEMAX;
	}
	CheckMenuRadioItem(hMainMenu, MENU_AUTOSIZE, MENU_WINDOWSIZEMAX, var, MF_BYCOMMAND);
}

static void menuSyncGame()
{
	CheckMenuItem(hMainMenu, MENU_SAVEC, bsavedecryptedcs ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hMainMenu, MENU_SAVEP, bsavedecryptedps ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hMainMenu, MENU_SAVES, bsavedecrypteds1 ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hMainMenu, MENU_SAVEV, bsavedecryptedvs ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hMainMenu, MENU_SAVEM, bsavedecryptedm1 ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hMainMenu, MENU_SAVEXOR, bsavedecryptedxor ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hMainMenu, MENU_SAVEDPROM, bsavedecryptedprom ? MF_CHECKED : MF_UNCHECKED);

	CheckMenuItem(hMainMenu, MENU_SETCPUCLOCK, nBurnCPUSpeedAdjust != 0x0100 ? MF_CHECKED : MF_UNCHECKED);

	EnableMenuItem(hMainMenu, MENU_INPUT, (bDrvOkay ? MF_ENABLED : MF_GRAYED) | MF_BYCOMMAND);
	EnableMenuItem(hMainMenu, MENU_SAVESNAP, (bDrvOkay ? MF_ENABLED : MF_GRAYED) | MF_BYCOMMAND);
	EnableMenuItem(hMainMenu, MENU_DIPSW, (bDrvOkay ? MF_ENABLED : MF_GRAYED) | MF_BYCOMMAND);

	if (bDrvOkay) {
		EnableMenuItem(hMainMenu, MENU_SETCPUCLOCK, (kNetGame ? MF_GRAYED : MF_ENABLED) | MF_BYCOMMAND);
		EnableMenuItem(hMainMenu, MENU_ENABLECHEAT, (kNetGame ? MF_GRAYED : MF_ENABLED) | MF_BYCOMMAND);
		EnableMenuItem(hMainMenu, MENU_CHEATSEARCH, (kNetGame ? MF_GRAYED : MF_ENABLED) | MF_BYCOMMAND);
		EnableMenuItem(hMainMenu, MENU_SNAPFACT, (kNetGame ? MF_GRAYED : MF_ENABLED) | MF_BYCOMMAND);
		EnableMenuItem(hMainMenu, MENU_JUKEBOX, (kNetGame ? MF_GRAYED : MF_ENABLED) | MF_BYCOMMAND);

		if (kNetGame) {
			EnableMenuItem(hMainMenu, MENU_RESETCPUCLOCK, MF_GRAYED | MF_BYCOMMAND);
		} else {
			EnableMenuItem(hMainMenu, MENU_RESETCPUCLOCK, (nBurnCPUSpeedAdjust != 0x0100 ? MF_ENABLED : MF_GRAYED) | MF_BYCOMMAND);
#if defined (FBA_DEBUG)
			EnableMenuItem(hMainMenu, MENU_DEBUG, MF_ENABLED | MF_BYCOMMAND);
#else
			EnableMenuItem(hMainMenu, MENU_DEBUG, MF_GRAYED | MF_BYCOMMAND);
#endif
		}
	} else {
		EnableMenuItem(hMainMenu, MENU_SETCPUCLOCK, MF_GRAYED | MF_BYCOMMAND);
		EnableMenuItem(hMainMenu, MENU_RESETCPUCLOCK, MF_GRAYED | MF_BYCOMMAND);
		EnableMenuItem(hMainMenu, MENU_ENABLECHEAT, MF_GRAYED | MF_BYCOMMAND);
		EnableMenuItem(hMainMenu, MENU_CHEATSEARCH, MF_GRAYED | MF_BYCOMMAND);
		EnableMenuItem(hMainMenu, MENU_DEBUG, MF_GRAYED | MF_BYCOMMAND);
		EnableMenuItem(hMainMenu, MENU_SNAPFACT, MF_GRAYED | MF_BYCOMMAND);
		EnableMenuItem(hMainMenu, MENU_JUKEBOX, MF_GRAYED | MF_BYCOMMAND);
	}
}

static void menuSyncMisc()
{
}

static void menuSyncAll()
{
	menuSyncFile();
	menuSyncSetting();
	menuSyncGame();
	menuSyncMisc();
}

void menuSync(MENU_TYPE type)
{
//	if (!bMenuEnabled) {
//		return;
//	}

	switch (type) {
		case MENUT_FILE:
			menuSyncFile();
			break;
		case MENUT_SETTING:
			menuSyncSetting();
			break;
		case MENUT_GAME:
			menuSyncGame();
			break;
		case MENUT_MISC:
			menuSyncMisc();
			break;
		case MENUT_HELP:
			break;
		default:
			menuSyncAll();
			break;
	}

	if (!bDrvOkay) {
		bAltPause = 0;
	}
}

// ---------------------------------------------------------------------------
int menuModifyAccString(unsigned int id, const TCHAR* accel)
{
	if (!accel) {
		return 1;
	}

	TCHAR text[MAX_PATH] = _T("");
	if (!GetMenuString(hMainMenu, id, text, sizearray(text), MF_BYCOMMAND)) {
		return 1;
	}

	// fixme: omit last \t
	TCHAR* p = _tcsrchr(text, '\t');
	if (p) {
		p[0] = '\0';
	}

	_stprintf(text, _T("%s\t%s"), text, accel);
	ModifyMenu(hMainMenu, id, MF_BYCOMMAND, id, text);

	return 0;
}
