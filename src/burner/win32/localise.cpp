// Language module, added by regret

/* changelog:
 update 2: add string cache
 update 1: create
*/

#include "burner.h"

#define LANG_CACHE 0

bool bLanguageActive = false;
TCHAR szLanguage[MAX_PATH] = _T("");
HMODULE hLanguage = NULL;

#if LANG_CACHE
map<int, tstring> stringMap;
#endif

// ----------------------------------------------------------------------------

INT_PTR FBADialogBox(int id, HWND parent, DLGPROC lpDialogFunc)
{
	return DialogBox(hLanguage ? hLanguage : hAppInst, MAKEINTRESOURCE(id), parent, lpDialogFunc);
}

HWND FBACreateDialog(int id, HWND parent, DLGPROC lpDialogFunc)
{
	return CreateDialog(hLanguage ? hLanguage : hAppInst, MAKEINTRESOURCE(id), parent, lpDialogFunc);
}

HMENU FBALoadMenu(int id)
{
	return LoadMenu(hLanguage ? hLanguage : hAppInst, MAKEINTRESOURCE(id));
}

HBITMAP FBALoadBitmap(int id)
{
	return LoadBitmap(hLanguage ? hLanguage : hAppInst, MAKEINTRESOURCE(id));
}

int FBALoadString(UINT id, LPTSTR buffer, int maxsize)
{
	int ret = 0;

#if LANG_CACHE
	map<int, tstring>::iterator iter = stringMap.find(id);
	if (iter != stringMap.end()) {
		tstring str =  iter->second;
		int size = str.size() > maxsize ? maxsize : str.size() + 1;
		_tcsncpy(buffer, str.c_str(), size);
		return size;
	}
#endif

	ret = LoadString(hLanguage ? hLanguage : hAppInst, id, buffer, maxsize);
#if LANG_CACHE
	if (ret > 0) {
		// cache string
		stringMap[id] = buffer;
	}
#endif
	return ret;
}

TCHAR* FBALoadStringEx(UINT id, bool translate)
{
	static TCHAR loadstr[2048] = _T("");

	if (translate) {
		FBALoadString(id, loadstr, sizearray(loadstr));
	} else {
		LoadString(hAppInst, id, loadstr, sizearray(loadstr));
	}

	return loadstr;
}

// ----------------------------------------------------------------------------

HMODULE FBALocaliseInstance()
{
	return hLanguage ? hLanguage : hAppInst;
}

void FBALocaliseExit()
{
	// Unload the dll
	if (hLanguage) {
		FreeLibrary(hLanguage);
		hLanguage = NULL;
	}

#if LANG_CACHE
	stringMap.clear();
#endif

	bLanguageActive = false;
}

int FBALocaliseInit(TCHAR* lanaugae)
{
	FBALocaliseExit();

	if (!lanaugae || !lanaugae[0]) {
		return 0;
	}

	hLanguage = LoadLibrary(lanaugae);
	if (!hLanguage) {
		FBAPopupAddText(PUF_TYPE_WARNING, _T("Language dll load failed!"));
		FBAPopupDisplay(PUF_TYPE_WARNING);
		return 1;
	}

	_tcsncpy(szLanguage, lanaugae, sizearray(szLanguage));
	bLanguageActive = true;
	return 0;
}

// ---------------------------------------------------------------------------
// Dialog box to load language

static void localiseMakeOfn()
{
	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hScrnWnd;
	ofn.lpstrFilter = _T("Language (*.dll)\0*.dll\0All Files (*.*)\0*.*\0\0");
	ofn.lpstrFile = szChoice;
	ofn.nMaxFile = sizearray(szChoice);
	ofn.lpstrInitialDir = _T("lang");
	ofn.Flags = OFN_NOCHANGEDIR | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	ofn.lpstrDefExt = _T("dll");
}

int FBALocaliseLoad()
{
	localiseMakeOfn();
	ofn.lpstrTitle = FBALoadStringEx(IDS_LOCAL_SELECT);

	int bOldPause = bRunPause;
	bRunPause = 1;
	int ret = GetOpenFileName(&ofn);
	bRunPause = bOldPause;

	if (ret == 0) {
		return 1;
	}

	return FBALocaliseInit(szChoice);
}
