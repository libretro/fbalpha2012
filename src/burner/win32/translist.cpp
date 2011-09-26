// translation gamelist, added by regret

/* changelog:
 update 3: add export list (special use)
 update 2: speedup load translist
 update 1: create
*/

#include "burner.h"
#include "seldef.h"

TCHAR szTransGamelistFile[MAX_PATH] = _T("");

/* modify by Sho, thanks 800, MAMEPlus! Team */
// optimized by regret, ^^
static int createGamelistFile(const TCHAR* szFile)
{
	FILE* fpLst = _tfopen(_T("mamep.lst"), _T("rt"));
	if (fpLst == NULL) {
		fpLst = _tfopen(_T("config\\mamep.lst"), _T("rt"));
	}

	FILE* fpIni = _tfopen(szFile, _T("wt"));
	if (fpIni == NULL) {
		if (fpLst) fclose(fpLst);
		return 1;
	}

	// write file header
	_ftprintf(fpIni, _T("// ") _T(APP_TITLE) _T(" v%s Local Language GameList File, CodePage: %d\n"), szAppBurnVer, GetACP());
	_ftprintf(fpIni, _T("// You can edit this file manually. ^^\n"));
	_ftprintf(fpIni, _T("// Format: rom name = game name (equal sign must in two spaces)\n\n"));
	_ftprintf(fpIni, _T("[games]\n"));

	TCHAR szGamename[MAX_PATH], szTransname[MAX_PATH];
	TCHAR szLine[1024];
	TCHAR* token = NULL;

	unsigned int oldDrvSelect = nBurnDrvSelect;
	for (unsigned int i = 0; i < nBurnDrvCount; i++) {
		nBurnDrvSelect = i;
		_tcscpy(szGamename, BurnDrvGetText(DRV_NAME));
		_tcscpy(szTransname, transGameName(BurnDrvGetText(DRV_ASCIIONLY | DRV_FULLNAME)));

		if (fpLst != NULL) {
			// get translated game name from MAME Plus! *.lst file
			fseek(fpLst, 0, SEEK_SET);
			while (NULL != _fgetts(szLine, sizearray(szLine), fpLst)) {
				token = _tcstok(szLine, _T("\t"));
				if (_tcscmp(token, szGamename) == 0) {
					token = _tcstok(NULL, _T("\t"));
					_tcscpy(szTransname, token);
					break;
				}
				free(token); token = NULL;
			}
		}

		_ftprintf(fpIni, _T("%s = %s\n"), szGamename, szTransname);
	}
	nBurnDrvSelect = oldDrvSelect;

	if (fpIni) fclose(fpIni);
	if (fpLst) fclose(fpLst);
	return 0;
}

static void transMakeOfn()
{
	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hScrnWnd;
	ofn.lpstrFilter = _T("Gamelist file (*.ini)\0*.ini\0\0");
	ofn.lpstrFile = szChoice;
	ofn.nMaxFile = sizearray(szChoice);
	ofn.lpstrInitialDir = _T("lang");
	ofn.Flags = OFN_NOCHANGEDIR | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	ofn.lpstrDefExt = _T("ini");
}

map<tstring, tstring> gamelistMap;
static bool gamelistLoaded = false;

static int initGamelist()
{
	if (gamelistLoaded) {
		return 0;
	}

	gamelistMap.clear();

	// parse gamelist
	FILE* file = _tfopen(szTransGamelistFile, _T("rb"));
	if (!file) {
		gamelistLoaded = true; // if didn't find file
		return 1;
	}

	bool utf8File = false;

	// read file header
	unsigned char header[4] = { 0, };
	fread(header, 1, sizeof(header), file);

	// check if it's a UTF-16 file
	if (header[0] == 0xFF && header[1] == 0xFE) {
		fseek(file, 2, SEEK_SET);
	} else {
		// check if it's a UTF-8 file
		if (header[0] == 0xEF && header[1] == 0xBB && header[2] == 0xBF) {
			utf8File = true;
		}
		fclose(file);

		file = _tfopen(szTransGamelistFile, _T("rt"));
		if (!file) {
			return 1;
		}
		if (utf8File) {
			fseek(file, 3, SEEK_SET);
		}
	}

	DWORD pos = 0;
	size_t nLen = 0;
	TCHAR s[MAX_PATH];
	tstring line, key, value;

	while (!feof(file)) {
		if (utf8File) {
			char szTemp[MAX_PATH];
			if (fgets(szTemp, sizeof(szTemp), file) == NULL) {
				break;
			}
			_tcscpy(s, U8toW(szTemp));
		} else {
			if (_fgetts(s, sizeof s, file) == NULL) {
				break;
			}
		}

		if (s[0] == ';' || (s[0] == '/' && s[1] == '/'))
			continue; // skip comment

		// omit last \r\n
		nLen = _tcslen(s);
		if (nLen > 1 && s[nLen-2] == '\r') {
			s[nLen-2] = '\0';
		}
		else if (nLen > 1 && s[nLen-1] == '\n') {
			s[nLen-1] = '\0';
		}
		line = s;

		if (line == _T("[games]") || line == _T("\r\n") || line == _T("\n"))
			continue;

		size_t pos = line.find(_T(" = "));
		if (pos == string::npos)
			continue;

		key = line.substr(0, pos); // get key = rom name
		value = line.substr(pos + 3); // get translation name

		gamelistMap[key] = value;
	}
	fclose(file);

	gamelistLoaded = true;
	return 0;
}

int loadGamelist()
{
	_stprintf(szChoice, _T("gamelist.ini"));
	transMakeOfn();
	ofn.lpstrTitle = FBALoadStringEx(IDS_GAMELIST_SELECT);

	int bOldPause = bRunPause;
	bRunPause = 1;
	int nRet = GetOpenFileName(&ofn);
	bRunPause = bOldPause;

	if (nRet == 0) {
		return 1;
	}

	if (szChoice) {
		_tcsncpy(szTransGamelistFile, szChoice, sizearray(szTransGamelistFile));
	}

	gamelistLoaded = false;
	initGamelist();

	return 0;
}

int createGamelist()
{
	_stprintf(szChoice, _T("gamelist.ini"));
	transMakeOfn();
	ofn.lpstrTitle = FBALoadStringEx(IDS_GAMELIST_CREATE);

	int bOldPause = bRunPause;
	bRunPause = 1;
	int nRet = GetSaveFileName(&ofn);
	bRunPause = bOldPause;

	if (nRet == 0) {
		return 1;
	}

	return createGamelistFile(szChoice);
}

static TCHAR* getTranslateGameName(tstring key)
{
	static tstring name;
	map<tstring, tstring>::iterator iter = gamelistMap.find(key);
	if (iter != gamelistMap.end()) {
		name = iter->second;
		return (TCHAR*)name.c_str();
	}
	return NULL;
}

TCHAR* mangleGamename(const TCHAR* pszOldName, bool bRemoveArticle)
{
	if (!pszOldName) {
		return NULL;
	}

	static TCHAR szNewName[MAX_PATH] = _T("");
	TCHAR* pszName = szNewName;

	if ((nLoadMenuShowX & MANGLENAME) && !_tcsnicmp(pszOldName, _T("the "), 4)) {
		int x = 0, y = 0;
		while (pszOldName[x] && pszOldName[x] != _T('(') && pszOldName[x] != _T('-')) {
			x++;
		}
		y = x;
		while (y && pszOldName[y - 1] == _T(' ')) {
			y--;
		}
		_tcsncpy(pszName, pszOldName + 4, y - 4);
		pszName[y - 4] = _T('\0');
		pszName += y - 4;

		if (!bRemoveArticle) {
			pszName += _stprintf(pszName, _T(", the"));
		}
		if (pszOldName[x]) {
			_stprintf(pszName, _T(" %s"), pszOldName + x);
		}
	} else {
		_tcscpy(pszName, pszOldName);
	}

	return szNewName;
}

TCHAR* transGameName(const TCHAR* pszOldName, bool bRemoveArticle)
{
	if (!pszOldName) {
		return NULL;
	}

	if ((nLoadMenuShowX & TRANSLANG) == 0) {
		return (mangleGamename(pszOldName, bRemoveArticle));
	}

	if (!gamelistLoaded) {
		initGamelist();

#if 0
		static DWORD sss;
		sss = GetTickCount();
		static TCHAR szNewName[MAX_PATH];

		for (unsigned int n = 0; n < nBurnDrvCount; n++) {
			unsigned int nOldBurnDrvSelect = nBurnDrvSelect;
			nBurnDrvSelect = n;
//			GetPrivateProfileString(_T("games"), BurnDrvGetText(DRV_NAME), NULL, szNewName, sizearray(szNewName), szTransGamelistFile);
			TCHAR* gameName = getTranslateGameName(BurnDrvGetText(DRV_NAME));
			nBurnDrvSelect = nOldBurnDrvSelect;
		}

		static DWORD eee;
		eee = GetTickCount();

		bprintf(PRINT_IMPORTANT, _T("trans time: %d\n"), eee - sss);
#endif
	}

	TCHAR* gameName = getTranslateGameName(BurnDrvGetText(DRV_NAME));
	if (gameName) {
		return gameName;
	}
	return mangleGamename(pszOldName, bRemoveArticle);
}
/* end Sho, thanks 800 */


// ==> special use: export filtered gamelist, rom name only
int createFilteredGamelistFile(int system)
{
	FILE* file = _tfopen(_T("mylist.txt"), _T("wt"));
	if (!file) {
		return 1;
	}

	// write file header
	_ftprintf(file, _T("// Filtered gamelist, rom name only\n\n"));
	_ftprintf(file, _T("[games]\n"));

	unsigned int oldDrvSelect = nBurnDrvSelect;
	int nHardware;

	for (unsigned int i = 0; i < nBurnDrvCount; i++) {
		nBurnDrvSelect = i;

		if (system > 0) {
			nHardware = 1 << (BurnDrvGetHardwareCode() >> 24);
			if (system !=  nHardware) {
				continue;
			}
		}

		_ftprintf(file, _T("%s\n"), BurnDrvGetText(DRV_NAME));
	}
	nBurnDrvSelect = oldDrvSelect;

	if (file) {
		fclose(file);
	}
	return 0;
}

// export non-working gamelist
int exportNonWorkingGamelist()
{
	FILE* file = _tfopen(_T("nwlist.txt"), _T("wt"));
	if (!file) {
		return 1;
	}

	// write file header
	_ftprintf(file, _T("// Non-working gamelist, rom name only\n\n"));
	_ftprintf(file, _T("[games]\n"));

	unsigned int oldDrvSelect = nBurnDrvSelect;

	for (unsigned int i = 0; i < nBurnDrvCount; i++) {
		nBurnDrvSelect = i;

		if (BurnDrvIsWorking()) {
			continue;
		}

		_ftprintf(file, _T("%s\n"), BurnDrvGetText(DRV_NAME));
	}
	nBurnDrvSelect = oldDrvSelect;

	if (file) {
		fclose(file);
	}
	return 0;
}
// <== special use
