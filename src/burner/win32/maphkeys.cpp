// configurable hotkey module, added by regret
// based on FBA-RR (http://code.google.com/p/fbarr/)

/* changelog:
 update 6: add update menu accel string
 update 5: partial rewrite
 update 4: add key up handle and multiple modifiers support
 update 3: code cleanup
 update 2: add key string for translation
 update 1: create (ref: fba-rr)
*/

#include "burner.h"
#include "maphkeys.h"
#include "tracklst.h"
#include "cheat.h"
#include "vid_directx_support.h"
#ifndef NO_AUTOFIRE
#include "autofire.h"
#endif

enum { MODKEY_NONE = 0x00, MODKEY_CTRL = 0x01, MODKEY_ALT = 0x02, MODKEY_SHIFT = 0x04 };

// init keys
CustomKey customKeys[] = {
	{ VK_PAUSE,      MODKEY_NONE,  MENU_PAUSE,             IDS_HOTKEY_PAUSE,        "pause",         HK_pause,           0, 0 },
	{ VK_SPACE,      MODKEY_NONE,  0,                      IDS_HOTKEY_FASTFOWARD,   "fast-foward",   HK_fastFowardKeyDown, HK_fastFowardKeyUp, 0 },
	{ VK_OEM_5,      MODKEY_NONE,  0,                      IDS_HOTKEY_FRAMEADVANCE, "frame-advance", HK_frameAdvance,    0, 0 },
	{ '9',           MODKEY_SHIFT, 0,                      IDS_HOTKEY_READONLY,     "readonly",      HK_toggleReadOnly,  0, 0 },
	{ VK_OEM_MINUS,  MODKEY_SHIFT, 0,                      IDS_HOTKEY_DECSPEED,     "dec-speed",     HK_speedDec,        0, 0 },
	{ VK_OEM_PLUS,   MODKEY_SHIFT, 0,                      IDS_HOTKEY_INCSPEED,     "inc-speed",     HK_speedInc,        0, 0 },
	{ VK_OEM_PERIOD, MODKEY_NONE,  0,                      IDS_HOTKEY_FRAMECOUNTER, "frame-counter", HK_frameCounter,    0, 0 },
	{ VK_F12,        MODKEY_NONE,  MENU_SAVESNAP,          IDS_HOTKEY_SCREENSHOT,   "screenshot",    HK_screenShot,      0, 0 },

	{ 0,             MODKEY_NONE,  0,                      IDS_HOTKEY_LOADSTATE1,   "loadstate1",    HK_loadState,       0, 1 },
	{ 0,             MODKEY_NONE,  0,                      IDS_HOTKEY_LOADSTATE2,   "loadstate2",    HK_loadState,       0, 2 },
	{ 0,             MODKEY_NONE,  0,                      IDS_HOTKEY_LOADSTATE3,   "loadstate3",    HK_loadState,       0, 3 },
	{ 0,             MODKEY_NONE,  0,                      IDS_HOTKEY_LOADSTATE4,   "loadstate4",    HK_loadState,       0, 4 },
	{ 0,             MODKEY_NONE,  0,                      IDS_HOTKEY_LOADSTATE5,   "loadstate5",    HK_loadState,       0, 5 },
	{ 0,             MODKEY_NONE,  0,                      IDS_HOTKEY_LOADSTATE6,   "loadstate6",    HK_loadState,       0, 6 },
	{ 0,             MODKEY_NONE,  0,                      IDS_HOTKEY_LOADSTATE7,   "loadstate7",    HK_loadState,       0, 7 },
	{ 0,             MODKEY_NONE,  0,                      IDS_HOTKEY_LOADSTATE8,   "loadstate8",    HK_loadState,       0, 8 },
	{ 0,             MODKEY_NONE,  0,                      IDS_HOTKEY_LOADSTATE9,   "loadstate9",    HK_loadState,       0, 9 },
	{ 0,             MODKEY_NONE,  0,                      IDS_HOTKEY_SAVESTATE1,   "savestate1",    HK_saveState,       0, 1 },
	{ 0,             MODKEY_NONE,  0,                      IDS_HOTKEY_SAVESTATE2,   "savestate2",    HK_saveState,       0, 2 },
	{ 0,             MODKEY_NONE,  0,                      IDS_HOTKEY_SAVESTATE3,   "savestate3",    HK_saveState,       0, 3 },
	{ 0,             MODKEY_NONE,  0,                      IDS_HOTKEY_SAVESTATE4,   "savestate4",    HK_saveState,       0, 4 },
	{ 0,             MODKEY_NONE,  0,                      IDS_HOTKEY_SAVESTATE5,   "savestate5",    HK_saveState,       0, 5 },
	{ 0,             MODKEY_NONE,  0,                      IDS_HOTKEY_SAVESTATE6,   "savestate6",    HK_saveState,       0, 6 },
	{ 0,             MODKEY_NONE,  0,                      IDS_HOTKEY_SAVESTATE7,   "savestate7",    HK_saveState,       0, 7 },
	{ 0,             MODKEY_NONE,  0,                      IDS_HOTKEY_SAVESTATE8,   "savestate8",    HK_saveState,       0, 8 },
	{ 0,             MODKEY_NONE,  0,                      IDS_HOTKEY_SAVESTATE9,   "savestate9",    HK_saveState,       0, 9 },
	{ VK_F8,         MODKEY_NONE,  MENU_STATE_PREVSLOT,    IDS_HOTKEY_SELPREVSTAT,  "sel-prevstate", HK_prevState,       0, 0 },
	{ VK_F11,        MODKEY_NONE,  MENU_STATE_NEXTSLOT,    IDS_HOTKEY_SELNEXTSTAT,  "sel-nextstate", HK_nextState,       0, 0 },
	{ VK_F9,         MODKEY_NONE,  MENU_STATE_LOAD_SLOT,   IDS_HOTKEY_LOADCURSTAT,  "load-curstate", HK_loadCurState,    0, 0 },
	{ VK_F10,        MODKEY_NONE,  MENU_STATE_SAVE_SLOT,   IDS_HOTKEY_SAVECURSTAT,  "save-curstate", HK_saveCurState,    0, 0 },
	{ VK_F9,         MODKEY_CTRL,  MENU_STATE_LOAD_DIALOG, IDS_HOTKEY_LOADSTATDIAG, "load-dialog",   HK_loadStateDialog, 0, 0 },
	{ VK_F10,        MODKEY_CTRL,  MENU_STATE_SAVE_DIALOG, IDS_HOTKEY_SAVESTATDIAG, "save-dialog",   HK_saveStateDialog, 0, 0 },

	{ 'R',           MODKEY_ALT,   MENU_STARTRECORD,       IDS_HOTKEY_STARTRECORD,  "start-rec",     HK_startRec,        0, 0 },
	{ 'P',           MODKEY_ALT,   MENU_STARTREPLAY,       IDS_HOTKEY_PLAYRECORD,   "play-rec",      HK_playRec,         0, 0 },
	{ 'T',           MODKEY_ALT,   MENU_STOPREPLAY,        IDS_HOTKEY_ENDRECORD,    "end-rec",       HK_stopRec,         0, 0 },
	{ 'V',           MODKEY_ALT,   MENU_AVISTART,          IDS_HOTKEY_STARTAVI,     "start-avi",     HK_startAvi,        0, 0 },
	{ 'E',           MODKEY_ALT,   MENU_AVISTOP,           IDS_HOTKEY_ENDAVI,       "end-avi",       HK_stopAvi,         0, 0 },
	{ VK_F1,         MODKEY_CTRL,  MENU_ENABLECHEAT,       IDS_HOTKEY_CHEATEDIT,    "cheat",         HK_cheatEditor,     0, 0 },
	{ VK_F1,         MODKEY_ALT,   MENU_CHEATSEARCH,       IDS_HOTKEY_CHEATSEARCH,  "cheat-search",  HK_cheatSearch,     0, 0 },
	{ VK_OEM_MINUS,  MODKEY_NONE,  0,                      IDS_HOTKEY_VOLUMEDOWN,   "volume-down",   HK_volumeDec,       0, 0 },
	{ VK_OEM_PLUS,   MODKEY_NONE,  0,                      IDS_HOTKEY_VOLUMEUP,     "volume-up",     HK_volumeInc,       0, 0 },
	{ VK_BACK,       MODKEY_NONE,  0,                      IDS_HOTKEY_SHOWFPS,      "show-fps",      HK_showFps,         0, 0 },
	{ VK_F6,         MODKEY_NONE,  MENU_LOAD,              IDS_HOTKEY_OPENGAME,     "open-game",     HK_openGame,        0, 0 },
	{ VK_F6,         MODKEY_SHIFT, MENU_LOAD_QUICK,        IDS_HOTKEY_QUICKOPEN,    "quick-open",    HK_quickOpenGame,   0, 0 },
	{ VK_F4,         MODKEY_CTRL,  MENU_QUIT,              IDS_HOTKEY_EXITGAME,     "exit-game",     HK_exitGame,        0, 0 },
	{ VK_F5,         MODKEY_NONE,  MENU_INPUT,             IDS_HOTKEY_CONFIGPAD,    "config-pad",    HK_configPad,       0, 0 },
	{ VK_F5,         MODKEY_CTRL,  MENU_DIPSW,             IDS_HOTKEY_DIPSW,        "dips",          HK_setDips,         0, 0 },
	{ VK_F12,        MODKEY_CTRL,  MENU_SNAPFACT,          IDS_HOTKEY_SHOTFACT,     "shot-factory",  HK_shotFactory,     0, 0 },
	{ '1',           MODKEY_ALT,   MENU_WINDOWSIZE1X,      IDS_HOTKEY_WINSIZE1,     "win-size1",     HK_windowSize,      0, 1 },
	{ '2',           MODKEY_ALT,   MENU_WINDOWSIZE2X,      IDS_HOTKEY_WINSIZE2,     "win-size2",     HK_windowSize,      0, 2 },
	{ '3',           MODKEY_ALT,   MENU_WINDOWSIZE3X,      IDS_HOTKEY_WINSIZE3,     "win-size3",     HK_windowSize,      0, 3 },
	{ '4',           MODKEY_ALT,   MENU_WINDOWSIZE4X,      IDS_HOTKEY_WINSIZE4,     "win-size4",     HK_windowSize,      0, 4 },
	{ 'S',           MODKEY_ALT,   MENU_WINDOWSIZEMAX,     IDS_HOTKEY_WINSIZEMAX,   "win-sizemax",   HK_windowSizeMax,   0, 0 },
	{ VK_PRIOR,      MODKEY_ALT,   0,                      IDS_HOTKEY_PREVFILTER,   "prev-filter",   HK_prevFilter,      0, 0 },
	{ VK_NEXT,       MODKEY_ALT,   0,                      IDS_HOTKEY_NEXTFILTER,   "next-filter",   HK_nextFilter,      0, 0 },
	{ VK_RETURN,     MODKEY_ALT,   MENU_FULL,              IDS_HOTKEY_FULLSCREEN,   "fullscreen",    HK_fullscreen,      0, 0 },
	{ 'F',           MODKEY_CTRL,  0,                      IDS_HOTKEY_FAKEFULL,     "fake-full",     HK_fakeFullscreen,  0, 0 },
	{ 'T',           MODKEY_CTRL,  0,                      IDS_HOTKEY_CHAT,         "chat",          HK_openNetChat,     0, 0 },
	{ VK_RETURN,     MODKEY_NONE,  0,                      IDS_HOTKEY_CHATSEND,     "chat-send",     HK_netChatSend,     0, 0 },

	{ 0xffff, 0xffff, 0, IDS_ERR_UNKNOWN, "", 0, 0, 0 }, // last key
};

static int nFpsScale = 100;

HWND hMHkeysDlg = NULL;
static HWND hMHkeysList = NULL;

static HHOOK hook = 0;
static int receivingKmap;

const TCHAR* GetKeyName(int c)
{
	static TCHAR out[MAX_PATH] = _T("");
	_stprintf(out, _T("#%d"), c);

	if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z'))
	{
		_stprintf(out, _T("%c"), c);
		return out;
	}
	if (c >= VK_NUMPAD0 && c <= VK_NUMPAD9)
	{
		_stprintf(out, _T("Numpad-%c"), '0' + (c - VK_NUMPAD0));
		return out;
	}

	switch (c)
	{
		case VK_TAB:        _stprintf(out, _T("Tab")); break;
		case VK_BACK:       _stprintf(out, _T("Backspace")); break;
		case VK_CLEAR:      _stprintf(out, _T("Delete")); break;
		case VK_RETURN:     _stprintf(out, _T("Enter")); break;
		case VK_LSHIFT:     _stprintf(out, _T("LShift")); break;
		case VK_RSHIFT:     _stprintf(out, _T("RShift")); break;
		case VK_LCONTROL:   _stprintf(out, _T("LCtrl")); break;
		case VK_RCONTROL:   _stprintf(out, _T("RCtrl")); break;
		case VK_LMENU:      _stprintf(out, _T("LAlt")); break;
		case VK_RMENU:      _stprintf(out, _T("RAlt")); break;
		case VK_PAUSE:      _stprintf(out, _T("Pause")); break;
		case VK_CAPITAL:    _stprintf(out, _T("CapsLock")); break;
		case VK_ESCAPE:     _stprintf(out, _T("Escape")); break;
		case VK_SPACE:      _stprintf(out, _T("Space")); break;
		case VK_PRIOR:      _stprintf(out, _T("PgUp")); break;
		case VK_NEXT:       _stprintf(out, _T("PgDn")); break;
		case VK_HOME:       _stprintf(out, _T("Home")); break;
		case VK_END:        _stprintf(out, _T("End")); break;
		case VK_LEFT:       _stprintf(out, _T("Left") ); break;
		case VK_RIGHT:      _stprintf(out, _T("Right")); break;
		case VK_UP:         _stprintf(out, _T("Up")); break;
		case VK_DOWN:       _stprintf(out, _T("Down")); break;
		case VK_SELECT:     _stprintf(out, _T("Select")); break;
		case VK_PRINT:      _stprintf(out, _T("Print")); break;
		case VK_EXECUTE:    _stprintf(out, _T("Execute")); break;
		case VK_SNAPSHOT:   _stprintf(out, _T("SnapShot")); break;
		case VK_INSERT:     _stprintf(out, _T("Insert")); break;
		case VK_DELETE:     _stprintf(out, _T("Delete")); break;
		case VK_HELP:       _stprintf(out, _T("Help")); break;
		case VK_LWIN:       _stprintf(out, _T("LWin")); break;
		case VK_RWIN:       _stprintf(out, _T("RWin")); break;
		case VK_APPS:       _stprintf(out, _T("App")); break;
		case VK_MULTIPLY:   _stprintf(out, _T("Numpad *")); break;
		case VK_ADD:        _stprintf(out, _T("Numpad +")); break;
		case VK_SEPARATOR:  _stprintf(out, _T("\\")); break;
		case VK_OEM_7:      _stprintf(out, _T("Apostrophe")); break;
		case VK_OEM_COMMA:  _stprintf(out, _T("Comma") );break;
		case VK_OEM_PERIOD: _stprintf(out, _T("Period"));break;
		case VK_SUBTRACT:   _stprintf(out, _T("Numpad -")); break;
		case VK_DECIMAL:    _stprintf(out, _T("Numpad .")); break;
		case VK_DIVIDE:     _stprintf(out, _T("Numpad /")); break;
		case VK_NUMLOCK:    _stprintf(out, _T("NumLock")); break;
		case VK_SCROLL:     _stprintf(out, _T("ScrollLock")); break;
		case VK_OEM_MINUS:  _stprintf(out, _T("-")); break;
		case VK_OEM_PLUS:   _stprintf(out, _T("=")); break;
		case VK_SHIFT:      _stprintf(out, _T("Shift")); break;
		case VK_CONTROL:    _stprintf(out, _T("Control")); break;
		case VK_MENU:       _stprintf(out, _T("Alt")); break;
		case VK_OEM_1:      _stprintf(out, _T(";")); break;
		case VK_OEM_4:      _stprintf(out, _T("[")); break;
		case VK_OEM_6:      _stprintf(out, _T("]")); break;
		case VK_OEM_5:      _stprintf(out, _T("\\")); break;
		case VK_OEM_2:      _stprintf(out, _T("/")); break;
		case VK_OEM_3:      _stprintf(out, _T("`")); break;
		case VK_F1:         _stprintf(out, _T("F1")); break;
		case VK_F2:         _stprintf(out, _T("F2")); break;
		case VK_F3:         _stprintf(out, _T("F3")); break;
		case VK_F4:         _stprintf(out, _T("F4")); break;
		case VK_F5:         _stprintf(out, _T("F5")); break;
		case VK_F6:         _stprintf(out, _T("F6")); break;
		case VK_F7:         _stprintf(out, _T("F7")); break;
		case VK_F8:         _stprintf(out, _T("F8")); break;
		case VK_F9:         _stprintf(out, _T("F9")); break;
		case VK_F10:        _stprintf(out, _T("F10")); break;
		case VK_F11:        _stprintf(out, _T("F11")); break;
		case VK_F12:        _stprintf(out, _T("F12")); break;
		case VK_F13:        _stprintf(out, _T("F13")); break;
		case VK_F14:        _stprintf(out, _T("F14")); break;
		case VK_F15:        _stprintf(out, _T("F15")); break;
		case VK_F16:        _stprintf(out, _T("F16")); break;
	}

	return out;
}

// update menu accel string
int MHkeysUpdateMenuAcc()
{
	TCHAR accelStr[MAX_PATH] = _T("");

	CustomKey* customkey = &customKeys[0];
	while (!lastCustomKey(*customkey)) {
		if (customkey->menuid > 0) {
			accelStr[0] = '\0';

			if (customkey->keymod & MODKEY_CTRL)
				_tcscat(accelStr, _T("Ctrl+"));
			if (customkey->keymod & MODKEY_ALT)
				_tcscat(accelStr, _T("Alt+"));
			if (customkey->keymod & MODKEY_SHIFT)
				_tcscat(accelStr, _T("Shift+"));

			_tcscat(accelStr, GetKeyName(customkey->key));

			menuModifyAccString(customkey->menuid, accelStr);
		}
		customkey++;
	}

	menuSync(MENUT_ALL);

	return 0;
}

// Update which command is using which key
static int MHkeysUseUpdate()
{
	TCHAR tempTxt[MAX_PATH];
	unsigned int i;

	if (hMHkeysList == NULL) {
		return 1;
	}

	// Update the values of all the inputs
	for (i = 0; !lastCustomKey(customKeys[i]); i++) {
		CustomKey& key = customKeys[i];

		LVITEM LvItem;
		tempTxt[0] = '\0';

		if (key.keymod & MODKEY_CTRL)
			_tcscat(tempTxt, _T("Ctrl + "));
		if (key.keymod & MODKEY_ALT)
			_tcscat(tempTxt, _T("Alt + "));
		if (key.keymod & MODKEY_SHIFT)
			_tcscat(tempTxt, _T("Shift + "));

		_stprintf(tempTxt, _T("%s%s"), tempTxt, GetKeyName(key.key));

		if (!key.key)
			tempTxt[0] = '\0';

		memset(&LvItem, 0, sizeof(LvItem));
		LvItem.mask = LVIF_TEXT;
		LvItem.iItem = i;
		LvItem.iSubItem = 1;
		LvItem.pszText = tempTxt;
		ListView_SetItem(hMHkeysList, &LvItem);
	}

	return 0;
}

static int MHkeysListBegin()
{
	LVCOLUMN LvCol;
	if (hMHkeysList == NULL) {
		return 1;
	}

	// Full row select style:
	ListView_SetExtendedListViewStyle(hMHkeysList, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	// Make column headers
	memset(&LvCol, 0, sizeof(LvCol));
	LvCol.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;

	LvCol.cx = 164;
	LvCol.pszText = FBALoadStringEx(IDS_INPUT_INPUT);
	ListView_InsertColumn(hMHkeysList, 0, &LvCol);

	LvCol.cx = 160;
	LvCol.pszText = FBALoadStringEx(IDS_INPUT_MAPPING);
	ListView_InsertColumn(hMHkeysList, 1, &LvCol);

	return 0;
}

// Make a list view of the game inputs
int MHkeysListMake(int bBuild)
{
	unsigned int i;

	if (hMHkeysList == NULL) {
		return 1;
	}

	if (bBuild)	{
		ListView_DeleteAllItems(hMHkeysList);
	}

	// Add all the input names to the list
	for (i = 0; !lastCustomKey(customKeys[i]); i++) {
		CustomKey& key = customKeys[i];

		LVITEM LvItem;
		memset(&LvItem, 0, sizeof(LvItem));
		LvItem.mask = LVIF_TEXT | LVIF_PARAM;
		LvItem.iItem = i;
		LvItem.iSubItem = 0;
		LvItem.pszText = FBALoadStringEx(key.strcode);
		LvItem.lParam = (LPARAM)i;

		if (bBuild) {
			ListView_InsertItem(hMHkeysList, &LvItem);
		} else {
			ListView_SetItem(hMHkeysList, &LvItem);
		}
	}

	MHkeysUseUpdate();

	return 0;
}

static int MHkeysInit()
{
	hMHkeysList = GetDlgItem(hMHkeysDlg, IDC_MHKEYS_LIST);

	MHkeysListBegin();
	MHkeysListMake(1);

	return 0;
}

static int MHkeysExit()
{
	hMHkeysList = NULL;
	hMHkeysDlg = NULL;

	UnhookWindowsHookEx(hook);
	hook = 0;

	MHkeysUpdateMenuAcc();

	return 0;
}

static LRESULT CALLBACK KeyMappingHook(int code, WPARAM wParam, LPARAM lParam)
{
	if (code < 0) {
		return CallNextHookEx(hook, code, wParam, lParam);
	}
	if (wParam == VK_SHIFT || wParam == VK_MENU || wParam == VK_CONTROL) {
		return CallNextHookEx(hook, code, wParam, lParam);
	}

	CustomKey& key = customKeys[receivingKmap];

	key.key = wParam;
	key.keymod = 0;
	if (KEY_DOWN(VK_CONTROL))
		key.keymod |= MODKEY_CTRL;
	if (KEY_DOWN(VK_MENU))
		key.keymod |= MODKEY_ALT;
	if (KEY_DOWN(VK_SHIFT))
		key.keymod |= MODKEY_SHIFT;

	MHkeysUseUpdate();

	UnhookWindowsHookEx(hook);
	hook = 0;

	SetWindowText(GetDlgItem(hMHkeysDlg, IDC_HKEYSS_STATIC), FBALoadStringEx(IDS_MAPHOTKEY_TIP));
	return 1;
}

// List item(s) deleted; find out which one(s)
static int MHkeysItemDelete()
{
	int start = -1;
	LVITEM LvItem;
	int ret;

	while ((ret = ListView_GetNextItem(hMHkeysList, start, LVNI_SELECTED)) != -1) {
		start = ret;

		// Get the corresponding input
		LvItem.mask = LVIF_PARAM;
		LvItem.iItem = ret;
		LvItem.iSubItem = 0;
		ListView_GetItem(hMHkeysList, &LvItem);
		ret = LvItem.lParam;

		CustomKey& key = customKeys[ret];
		key.key = 0;
		key.keymod = 0;
	}

	MHkeysListMake(0);
	return 0;
}

// List item activated; find out which one
static int MHkeysItemActivate()
{
	receivingKmap = ListView_GetNextItem(hMHkeysList, -1, LVNI_SELECTED);
	if (receivingKmap < 0) {
		return 1;
	}

	CustomKey& key = customKeys[receivingKmap];
	TCHAR keystr[MAX_PATH] = _T("");
	_tcscpy(keystr, FBALoadStringEx(key.strcode));

	TCHAR str[MAX_PATH] = _T("");
	_stprintf(str, FBALoadStringEx(IDS_MAPHOTKEY_CLICKTIP), keystr);
	SetWindowText(GetDlgItem(hMHkeysDlg, IDC_HKEYSS_STATIC), str);
	hook = SetWindowsHookEx(WH_KEYBOARD, KeyMappingHook, 0, GetCurrentThreadId());

	return 0;
}

static INT_PTR CALLBACK MHkeysDialogProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	if (Msg == WM_INITDIALOG) {
		hMHkeysDlg = hDlg;
		MHkeysInit();
		wndInMid(hDlg, hScrnWnd);
		return TRUE;
	}

	if (Msg == WM_CLOSE) {
		EnableWindow(hScrnWnd, TRUE);
		DestroyWindow(hMHkeysDlg);
		return 0;
	}

	if (Msg == WM_DESTROY) {
		MHkeysExit();
		return 0;
	}

	if (Msg == WM_COMMAND) {
		int Id = LOWORD(wParam);
		int Notify = HIWORD(wParam);

		if (Id == IDOK && Notify == BN_CLICKED) {
			return 0;
		}
		else if (Id == IDCANCEL && Notify == BN_CLICKED) {
			SendMessage(hDlg, WM_CLOSE, 0, 0);
			return 0;
		}
	}

	if (Msg == WM_NOTIFY && lParam != 0) {
		int Id = LOWORD(wParam);
		NMHDR* pnm = (NMHDR*)lParam;

		if (Id == IDC_MHKEYS_LIST && pnm->code == LVN_ITEMACTIVATE) {
			MHkeysItemActivate();
		}
		else if (Id == IDC_MHKEYS_LIST && pnm->code == LVN_KEYDOWN) {
			NMLVKEYDOWN *pnmkd = (NMLVKEYDOWN*)lParam;
			if (pnmkd->wVKey == VK_DELETE) {
				MHkeysItemDelete();
			}
		}

		return 0;
	}

	return 0;
}

int MHkeysCreate(HWND parent)
{
	DestroyWindow(hMHkeysDlg); // Make sure exitted

	FBADialogBox(IDD_MHKEYS, parent, (DLGPROC)MHkeysDialogProc);
	if (hMHkeysDlg == NULL) {
		return 1;
	}

	ShowWindow(hMHkeysDlg, SW_NORMAL);

	return 0;
}

// ----------------------------------------------------------------------------

// key functions
extern bool useDialogs();
extern void simpleReinitScrn(const bool&);
extern void quickLoadFile();

void HK_pause(int)
{
	if (bDrvOkay && !kNetGame) {
		setPauseMode(!bRunPause);
	} else {
		setPauseMode(0);
	}
	menuSync(MENUT_FILE);
}

void HK_fastFowardKeyDown(int)
{
	if (!kNetGame && !soundLogStart) {
		bAppDoFast = 1;
	}
}
void HK_fastFowardKeyUp(int)
{
	bAppDoFast = 0;
}

void HK_loadState(int param)
{
	StatedLoad(param);
}
void HK_saveState(int param)
{
	StatedSave(param);
}

void HK_prevState(int)
{
	nSavestateSlot--;
	if (nSavestateSlot < 1) {
		nSavestateSlot = 1;
	}

	TCHAR szString[MAX_PATH];
	_sntprintf(szString, sizearray(szString), FBALoadStringEx(IDS_STATE_ACTIVESLOT), nSavestateSlot);
	VidSNewShortMsg(szString);
	menuSync(MENUT_FILE);
}
void HK_nextState(int)
{
	nSavestateSlot++;
	if (nSavestateSlot > 8) {
		nSavestateSlot = 8;
	}

	TCHAR szString[MAX_PATH];
	_sntprintf(szString, sizearray(szString), FBALoadStringEx(IDS_STATE_ACTIVESLOT), nSavestateSlot);
	VidSNewShortMsg(szString);
	menuSync(MENUT_FILE);
}

void HK_loadCurState(int)
{
	if (bDrvOkay && !kNetGame) {
		if (StatedLoad(nSavestateSlot) == 0) {
			if (nReplayStatus) {
				if (bReplayReadOnly) {
					VidSNewShortMsg(FBALoadStringEx(IDS_REP_REWIND));
				} else {
					VidSNewShortMsg(FBALoadStringEx(IDS_REP_UNDO));
				}
			}
		}
	}
}
void HK_saveCurState(int)
{
	if (bDrvOkay) {
		StatedSave(nSavestateSlot);
	}
}

void HK_loadStateDialog(int)
{
	if (useDialogs() && !kNetGame) {
		InputSetCooperativeLevel(false, !bAlwaysProcessKey);
		audio.stop();
		SplashDestroy(1);
		StatedLoad(0);
		GameInpCheckMouse();
		audio.play();
	}
}
void HK_saveStateDialog(int)
{
	if (useDialogs()) {
		InputSetCooperativeLevel(false, !bAlwaysProcessKey);
		audio.blank();
		StatedSave(0);
		GameInpCheckMouse();
	}
}

void HK_playRec(int)
{
	if (useDialogs()) {
		InputSetCooperativeLevel(false, !bAlwaysProcessKey);
		audio.stop();
		SplashDestroy(1);
		StopReplay();
		StartReplay();
		GameInpCheckMouse();
		audio.play();

		menuSync(MENUT_FILE);
	}
}
void HK_startRec(int)
{
	if (useDialogs()) {
		InputSetCooperativeLevel(false, !bAlwaysProcessKey);
		audio.blank();
		StopReplay();
		StartRecord();
		GameInpCheckMouse();

		menuSync(MENUT_FILE);
	}
}
void HK_stopRec(int)
{
	StopReplay();
	menuSync(MENUT_FILE);
}

void HK_startAvi(int)
{
	if (useDialogs()) {
		if (AviStart()) {
			AviStop();
		} else {
			VidSNewShortMsg(FBALoadStringEx(IDS_REC_AVI), 0x0000FF);
		}
		menuSync(MENUT_FILE);
	}
}
void HK_stopAvi(int)
{
	if (!AviStop()) {
		VidSNewShortMsg(FBALoadStringEx(IDS_STOP_AVI), 0xFF3F3F);
		menuSync(MENUT_FILE);
	}
}

void HK_frameAdvance(int)
{
	if (!kNetGame && !bRunPause) {
		bRunPause = 1;
	}
	bAppDoStep = 1;
}

void HK_toggleReadOnly(int)
{
	bReplayReadOnly = !bReplayReadOnly;
	if (bReplayReadOnly) {
		VidSNewShortMsg(_T("read-only"));
	} else {
		VidSNewShortMsg(_T("read+write"));
	}
}

void HK_frameCounter(int)
{
	bFrameCounterDisplay = !bFrameCounterDisplay;
	if (!bFrameCounterDisplay) {
		VidSKillTinyMsg();
		VidSKillTinyOSDMsg();
	}
}

void HK_speedInc(int)
{
	if (kNetGame) {
		return;
	}

	if (nFpsScale < 10) {
		nFpsScale = 10;
	} else {
		if (nFpsScale >= 100) {
			nFpsScale += 50;
		} else {
			nFpsScale += 10;
		}
	}
	if (nFpsScale > 800) {
		nFpsScale = 800;
	}

	TCHAR buffer[15];
	_stprintf(buffer, FBALoadStringEx(IDS_SPEEDSET), nFpsScale);
	VidSNewShortMsg(buffer);

	mediaChangeFps(nFpsScale);
}
void HK_speedDec(int)
{
	if (kNetGame) {
		return;
	}

	if (nFpsScale <= 10) {
		nFpsScale = 10;
	} else {
		if (nFpsScale > 100) {
			nFpsScale -= 50;
		} else {
			nFpsScale -= 10;
		}
	}
	if (nFpsScale < 10) {
		nFpsScale = 10;
	}

	TCHAR buffer[15];
	_stprintf(buffer, FBALoadStringEx(IDS_SPEEDSET), nFpsScale);
	VidSNewShortMsg(buffer);

	mediaChangeFps(nFpsScale);
}

void HK_volumeDec(int)
{
	audio.nAudVolume -= 100;
	if (audio.nAudVolume < 0) {
		audio.nAudVolume = 0;
	}
	if (audio.setvolume(audio.nAudVolume) != 0) {
		VidSNewShortMsg(FBALoadStringEx(IDS_SOUND_NOVOLUME));
	} else {
		TCHAR buffer[15];
		_stprintf(buffer, FBALoadStringEx(IDS_SOUND_VOLUMESET), audio.nAudVolume / 100);
		VidSNewShortMsg(buffer);
	}
}
void HK_volumeInc(int)
{
	audio.nAudVolume += 100;
	if (audio.nAudVolume > 10000) {
		audio.nAudVolume = 10000;
	}
	if (audio.setvolume(audio.nAudVolume) != 0) {
		VidSNewShortMsg(FBALoadStringEx(IDS_SOUND_NOVOLUME));
	} else {
		TCHAR buffer[15];
		_stprintf(buffer, FBALoadStringEx(IDS_SOUND_VOLUMESET), audio.nAudVolume / 100);
		VidSNewShortMsg(buffer);
	}
}

void HK_showFps(int)
{
	bShowFPS = !bShowFPS;
	if (!bShowFPS) {
		VidSKillShortMsg();
		VidSKillOSDMsg();
		scrnTitle();
	}
}

void HK_configPad(int)
{
	if (useDialogs()) {
		audio.blank();
		InputSetCooperativeLevel(false, false);
		InpdCreate();
	}
}

void HK_setDips(int)
{
	if (useDialogs()) {
		audio.blank();
		InputSetCooperativeLevel(false, !bAlwaysProcessKey);
		InpDIPSWCreate();
	}
}

void HK_cheatEditor(int)
{
	if (useDialogs()) {
		audio.blank();
		InputSetCooperativeLevel(false, !bAlwaysProcessKey);
		InpCheatCreate();
	}
}

void HK_cheatSearch(int)
{
#ifndef NO_CHEATSEARCH
	if (useDialogs()) {
		audio.blank();
		InputSetCooperativeLevel(false, !bAlwaysProcessKey);
		cheatSearchCreate();
	}
#endif
}

void HK_windowSize(int param)
{
	extern bool bFakeFullscreen;
	if (nWindowSize != param && !bFakeFullscreen) {
		nWindowSize = param;
		simpleReinitScrn(false);
		menuSync(MENUT_SETTING);
	}
}
void HK_windowSizeMax(int)
{
	extern bool bFakeFullscreen;
	if (nWindowSize <= 4 && !bFakeFullscreen) {
		nWindowSize = 30;
		simpleReinitScrn(false);
		menuSync(MENUT_SETTING);
	}
}

void HK_fullscreen(int)
{
	if (bDrvOkay || nVidFullscreen) {
		nVidFullscreen = !nVidFullscreen;
		scrnSwitchFull();
	}
}

void HK_fakeFullscreen(int)
{
	scrnFakeFullscreen();
}

void HK_screenShot(int)
{
	if (bDrvOkay) {
		int status = MakeScreenShot();
		if (!status) {
			VidSNewShortMsg(FBALoadStringEx(IDS_SSHOT_SAVED));
		} else {
			TCHAR tmpmsg[MAX_PATH];
			_sntprintf(tmpmsg, sizearray(tmpmsg), FBALoadStringEx(IDS_SSHOT_ERROR), status);
			VidSNewShortMsg(tmpmsg, 0xFF3F3F);
		}
	}
}
void HK_shotFactory(int)
{
	if (useDialogs()) {
		audio.blank();
		InputSetCooperativeLevel(false, !bAlwaysProcessKey);
		SFactdCreate();
	}
}

void HK_prevFilter(int)
{
	scrnSize();
	vidUseFilter = 1;
	VidSwitchFilter(nVidFilter - 1);
	menuSync(MENUT_SETTING);
}
void HK_nextFilter(int)
{
	scrnSize();
	vidUseFilter = 1;
	VidSwitchFilter(nVidFilter + 1);
	menuSync(MENUT_SETTING);
}

void HK_openGame(int)
{
	extern bool bLoading;
	if (kNetGame || !useDialogs() || bLoading) {
		return;
	}

	SplashDestroy(1);
	StopReplay();
	AviStop();

	InputSetCooperativeLevel(false, !bAlwaysProcessKey);

	bLoading = true;
	audio.stop();					// Stop while the dialog is active or we're loading ROMs

	int game = selDialog(hScrnWnd);	// Bring up select dialog to pick a driver

	bLoading = false;
	audio.play();					// Restart sound

	if (game >= 0) {
		if (bFullscreenOnStart) {
			nVidFullscreen = 1;
		}
		BurnerDrvInit(game, true);	// Init the game driver
		menuSync(MENUT_ALL);
		bAltPause = 0;
	} else {
		GameInpCheckMouse();
	}
}
void HK_quickOpenGame(int)
{
	if (!kNetGame && useDialogs()) {
		quickLoadFile();
	}
}
void HK_exitGame(int)
{
	audio.blank();
	if (nVidFullscreen) {
		nVidFullscreen = 0;
		VidExit();
	}
	if (bDrvOkay) {
		StopReplay();
		AviStop();
		BurnerDrvExit();
		if (kNetGame) {
			FBA_KailleraEnd();
			PostQuitMessage(0);
		}
		bCheatsAllowed = true; // reenable cheats netplay has ended

		POST_INITIALISE_MESSAGE;
		menuSync(MENUT_ALL);
	}
}

void HK_openNetChat(int)
{
	// fixme: message
	if (kNetGame && !ChatActivated()) {
//		if (AppMessage((MSG*)&Msg)) {
			ActivateChat();
//		}
	}
}
void HK_netChatSend(int)
{
	if (kNetGame) {
		FBA_KailleraSend();
	}
}


// key handle
static inline bool MHkeysCheckMenuState(const CustomKey* key)
{
	if (!key || (key->menuid > 0 && GetMenuState(hMainMenu, key->menuid, MF_BYCOMMAND) == MF_GRAYED)) {
		return false;
	}
	return true;
}

int MHkeysDownHandle(const MSG& Msg)
{
	static int key = 0;
	static int modifiers = 0;
	static int processed = 0;

	key = Msg.wParam;
	modifiers = 0;
	if (KEY_DOWN(VK_CONTROL)) {
		modifiers |= MODKEY_CTRL;
	}
	if (KEY_DOWN(VK_MENU)) {
		modifiers |= MODKEY_ALT;
	}
	if (KEY_DOWN(VK_SHIFT)) {
		modifiers |= MODKEY_SHIFT;
	}

	processed = 0;

	CustomKey* customkey = &customKeys[0];
	while (!lastCustomKey(*customkey)) {
		if (key == customkey->key && modifiers == customkey->keymod && customkey->handleKeyDown) {
			if (MHkeysCheckMenuState(customkey)) {
				customkey->handleKeyDown(customkey->param);
				processed = 1;
			}
		}
		customkey++;
	}

	return processed;
}

int MHkeysUpHandle(const MSG& Msg)
{
	static int key = 0;
	static int modifiers = 0;
	static int processed = 0;

	key = Msg.wParam;
	modifiers = 0;
	if (KEY_DOWN(VK_CONTROL)) {
		modifiers |= MODKEY_CTRL;
	}
	if (KEY_DOWN(VK_MENU)) {
		modifiers |= MODKEY_ALT;
	}
	if (KEY_DOWN(VK_SHIFT)) {
		modifiers |= MODKEY_SHIFT;
	}

	processed = 0;

	CustomKey* customkey = &customKeys[0];
	while (!lastCustomKey(*customkey)) {
		if (customkey->handleKeyUp && key == customkey->key && modifiers == customkey->keymod) {
			customkey->handleKeyUp(customkey->param);
			processed = 1;
		}
		customkey++;
	}

	return processed;
}
