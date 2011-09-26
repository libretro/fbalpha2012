// IPS support

/* changelog:
 update 5: separate ips and add rom buffer length
 update 4: update to ips v4
 update 3: add ips setting dialog from official fba
 update 4: update to ips v3
 update 2: add display ips description and preview image
 update 1: create, base on ips v2
*/

#ifndef NO_IPS

#include "burner.h"
#include "patch.h"
#include "ips.h"
#include "imageload+.h"
#include "seldef.h"

// ==> GCC doesn't seem to define these correctly.....
#define _TreeView_SetItemState(hwndTV, hti, data, _mask) \
{ \
	TVITEM _ms_TVi; \
	_ms_TVi.mask = TVIF_STATE; \
	_ms_TVi.hItem = hti; \
	_ms_TVi.stateMask = _mask; \
	_ms_TVi.state = data; \
	SNDMSG((hwndTV), TVM_SETITEM, 0, (LPARAM)(TV_ITEM *)&_ms_TVi); \
}

#define _TreeView_SetCheckState(hwndTV, hti, fCheck) \
	_TreeView_SetItemState(hwndTV, hti, INDEXTOSTATEIMAGEMASK((fCheck) ? 2 : 1), TVIS_STATEIMAGEMASK)

#define _TreeView_GetCheckState(hwndTV, hti) \
	((((UINT)(SNDMSG((hwndTV), TVM_GETITEMSTATE, (WPARAM)(hti), TVIS_STATEIMAGEMASK))) >> 12) - 1)
// <==

#define UTF8_SIGNATURE	"\xef\xbb\xbf"
#define IPS_EXT			".ips"
#define UPS_EXT			".ups"
//#define CRC_STAG		"CRC("
//#define CRC_ETAG		")"

#define BYTE3_TO_UINT(bp) \
     (((unsigned int)(bp)[0] << 16) & 0x00FF0000) | \
     (((unsigned int)(bp)[1] << 8) & 0x0000FF00) | \
     ((unsigned int)(bp)[2] & 0x000000FF)

#define BYTE2_TO_UINT(bp) \
    (((unsigned int)(bp)[0] << 8) & 0xFF00) | \
    ((unsigned int) (bp)[1] & 0x00FF)


// ==> IPS language, added by regret
static int ansi_codepage;
int nPatchLang = 0;

static const ui_lang_info_t ui_lang_info[] = {
	{ "en_US", "US", "English (US)",        1252, 0     },
	{ "zh_CN", "CN", "Simplified Chinese",  936,  23940 },
	{ "zh_TW", "TW", "Traditional Chinese", 950,  13973 },
	{ "ja_JP", "JP", "Japanese",            932,  11520 },
	{ "ko_KR", "KR", "Korean",              949,  22428 },
	{ "fr_FR", "FR", "French",              1252, 0 },
	{ "es_ES", "ES", "Spanish",             1252, 0 },
	{ "it_IT", "IT", "Italian",             1252, 0 },
	{ "de_DE", "DE", "German",              1252, 0 },
	{ "pt_BR", "BR", "Portuguese (BRA)",    1252, 0 },
	{ "pl_PL", "PL", "Polish",              1250, 0 },
	{ "hu_HU", "HU", "Hungarian",           1250, 0 },
	{ NULL }
};
// <== IPS language

bool bDoPatch = false;
int bEnforceDep = 0;

static void doPatchGame(const char* patch_name, char* game_name, UINT8* base, int base_len)
{
	FILE* fp = NULL;
	if ((fp = fopen(patch_name, "rb")) == NULL) {
		return;
	}

	char s[1024];
	char* p = NULL;
	char* rom_name = NULL;
	char* ips_name = NULL;
	long ips_size = 0;

	// get ips size
	ips_size = getFileSize(fp);

	while (!feof(fp)) {
		if (fgets(s, sizeof s, fp) != NULL) {
			p = s;

			// skip UTF-8 sig
			if (strncmp(p, UTF8_SIGNATURE, strlen(UTF8_SIGNATURE)) == 0)
				p += strlen(UTF8_SIGNATURE);

			if (p[0] == '[')	// '['
				break;

			rom_name = strtok(p, " \t\r\n");
			if (!rom_name)
				continue;
			if (*rom_name == '#')
				continue;
			if (_stricmp(rom_name, game_name))
				continue;

			ips_name = strtok(NULL, " \t\r\n");
			if (!ips_name)
				continue;

			// skip CRC check
			strtok(NULL, "\r\n");

			char ips_path[MAX_PATH];

			if (strchr(ips_name, '\\')) {
				// ips in parent's folder
				sprintf(ips_path, "%s\\%s%s", WtoA(getMiscPath(PATH_IPS)), ips_name, IPS_EXT);
			} else {
				sprintf(ips_path, "%s%s\\%s%s", WtoA(getMiscPath(PATH_IPS)), BurnDrvGetTextA(DRV_NAME), ips_name, IPS_EXT);
			}

			applyIPSpatch(ips_path, ips_size, base, base_len);
		}
	}

	fclose(fp);
}

int getPatchFilename(TCHAR* pszName, TCHAR* pszRet, const int& patch_index)
{
	WIN32_FIND_DATA wfd;
	HANDLE hSearch;
	TCHAR szFilePathName[MAX_PATH];

	_stprintf(szFilePathName, _T("%s%s\\"), getMiscPath(PATH_IPS), pszName);

	if (szFilePathName[_tcslen(szFilePathName) -1] != _T('\\')) {
		_tcscat(szFilePathName, _T("\\"));
	}
	_tcscat(szFilePathName, _T("*.dat"));

	hSearch = FindFirstFile(szFilePathName, &wfd);

	if (hSearch) {
		int Done = 0;
		int Count = 0;

		while (!Done) {
			if (Count == patch_index) {
				_tcscpy(pszRet, wfd.cFileName);
				pszRet[_tcslen(pszRet) - 4] = _T('\0');	// To trim the ext ".dat"
				break;
			}
			Count++;
			Done = !FindNextFile(hSearch, &wfd);
		}

		FindClose(hSearch);
		return 1;
	}
	return 0;
}

int getPatchCount()
{
	TCHAR szFilePathName[MAX_PATH];

	_stprintf(szFilePathName, _T("%s%s\\"), getMiscPath(PATH_IPS), BurnDrvGetText(DRV_NAME));
	if (szFilePathName[_tcslen(szFilePathName) -1] != _T('\\')) {
		_tcscat(szFilePathName, _T("\\"));
	}
	_tcscat(szFilePathName, _T("*.dat"));

	unsigned int count = 0;
	WIN32_FIND_DATA wfd;

	HANDLE hSearch = FindFirstFile(szFilePathName, &wfd);
	if (hSearch != INVALID_HANDLE_VALUE) {
		do {
			count++;
		} while (FindNextFile(hSearch, &wfd));

		FindClose(hSearch);
	}
	return count;
}

void patchExit()
{
	bDoPatch = false;
}

// source from MAME Plus!
LPTSTR convertAmpersandString(LPCTSTR s)
{
	/* takes a string and changes any ampersands to double ampersands,
	   for setting text of window controls that don't allow us to disable
	   the ampersand underlining.
	 */
	/* returns a static buffer--use before calling again */

	static TCHAR buf[200];
	TCHAR* ptr = buf;

	while (*s)
	{
		if (*s == _T('&'))
			*ptr++ = *s;
		*ptr++ = *s++;
	}
	*ptr = 0;

	return buf;
}

// ==> IPS preview, added by regret, thanks MAME Plus! team
int getPatchPrevName(TCHAR* pszName, TCHAR* pszGameName, TCHAR* pszPatchName)
{
	_stprintf(pszName, _T("%s%s\\%s.png"), getMiscPath(PATH_IPS), pszGameName, pszPatchName);

	FILE* fp = _tfopen(pszName, _T("rb"));
	if (fp) {
		fclose(fp);
		_stprintf(pszName, _T("%s%s\\%s"), getMiscPath(PATH_IPS), pszGameName, pszPatchName);
		return 1;
	} else {
		// if not loaded, see parent's ips folder
		_stprintf(pszName, _T("%s%s\\%s.png"), getMiscPath(PATH_IPS), BurnDrvGetText(DRV_PARENT), pszPatchName);
		fp = _tfopen(pszName, _T("rb"));
		if (fp) {
			fclose(fp);
			_stprintf(pszName, _T("%s%s\\%s"), getMiscPath(PATH_IPS), BurnDrvGetText(DRV_PARENT), pszPatchName);
			return 1;
		}
	}
	return 0;
}

static TCHAR* getPatchDescByLangcode(FILE* fp, int langcode)
{
	TCHAR* result = NULL;
	char* desc = NULL;
	char langtag[8];

	sprintf(langtag, "[%s]", ui_lang_info[langcode].name);

	fseek(fp, 0, SEEK_SET);

	while (!feof(fp))
	{
		char s[4096];

		if (fgets(s, sizeof s, fp) != NULL)
		{
			if (strncmp(langtag, s, strlen(langtag)) != 0)
				continue;

			while (fgets(s, sizeof s, fp) != NULL)
			{
				if (*s == '[')
				{
					if (desc)
					{
						result = U8toW(desc);
						free(desc);
						return result;
					}
					else
						return NULL;
				}

				char* p = s;
				getLine(p);

//				if (*s == '\0')
//					continue;

				if (desc)
				{
					size_t len = strlen(desc);
					len += strlen(s) + 2;

					char* p = (char*)malloc(len + 1);
					sprintf(p, "%s\r\n%s", desc, s);
					free(desc);
					desc = p;
				}
				else
				{
					desc = (char*)malloc(strlen(s) + 1);
					if (desc != NULL)
						strcpy(desc, s);
				}
			}
		}
	}

	if (desc)
	{
		result = U8toW(desc);
		free(desc);
		return result;
	}
	else
		return NULL;
}

TCHAR* getPatchDesc(const TCHAR* pszGameName, const TCHAR* pszPatchName)
{
	FILE* fp = NULL;
	TCHAR szFilename[MAX_PATH];
	TCHAR* desc = NULL;

	_stprintf(szFilename, _T("%s\\%s\\%s.dat"), getMiscPath(PATH_IPS), pszGameName, pszPatchName);
	if ((fp = _tfopen(szFilename, _T("r"))) != NULL) {
		/* Get localized desc */
		desc = getPatchDescByLangcode(fp, nPatchLang);

		fclose(fp);
	}

	return desc;
}
// <== IPS preview


// ==> IPS setting dialog
#define MAX_NODES 300
#define MAX_ACTIVE_PATCHES 300

static int nPatchIndex = 0;
static int nNumPatches = 0;
static HTREEITEM hItemHandles[MAX_NODES];
static HTREEITEM hPatchHandlesIndex[MAX_NODES];
static TCHAR szPatchFileNames[MAX_NODES][MAX_PATH];

TCHAR szActivePatches[MAX_ACTIVE_PATCHES][MAX_PATH];

static HBRUSH hWhiteBGBrush;
static HBITMAP hPreview = NULL;
static HBITMAP hDefPreview = NULL;

static HWND hIpsDlg = NULL;
static HWND hIpsList = NULL;

// ==> IPSv4 conflict/dependancy assistant, added by regret (Thanks to Emuman)
typedef list<string> StringList;
list<StringList> confTable;
multimap<string, StringList> depTable;
hash_map<string, int> itemStateTable;

static TCHAR szDataNames[MAX_NODES][MAX_PATH];
BOOL hasAssist = false;

// string functions
static inline void lowerString(string& str)
{
	char strs[MAX_PATH];
	strcpy(strs, str.c_str());

	int i = 0;
	while (strs[i]) {
		strs[i] = strlower(strs[i]);
		i++;
	}

	str = strs;
}

static inline void lowerTrimmed(StringList& list)
{
	StringList::iterator iter = list.begin();
	for (; iter != list.end(); iter++) {
		lowerString(*iter);
	}
}

static inline StringList split2Str(const string& str, const string& separator)
{
	static StringList strs;
	strs.clear();

	size_t pos = str.find(separator);
	if (pos >= 0) {
		strs.push_back(str.substr(0, pos));
		strs.push_back(str.substr(pos + 1, str.size() - pos - 1));
	}
	return strs;
}

static inline StringList stringSplit(const string& str, const string& separator)
{
	static StringList strList;
	strList.clear();

	string tempStr = str;
	size_t pos = string::npos;

	if (tempStr.find(separator) == string::npos) {
		strList.push_back(tempStr);
	} else {
		while ((pos = tempStr.find(separator)) != string::npos) {
			strList.push_back(tempStr.substr(0, pos));
			tempStr = tempStr.substr(pos + 1);
		}
		strList.push_back(tempStr);
	}

	return strList;
}

static inline bool stringListContain(StringList& strList, const string& str)
{
	StringList::iterator iter = strList.begin();
	for (; iter != strList.end(); iter++)
	{
		if (*iter == str) {
			return true;
		}
	}
	return false;
}

// ips relation
static inline void saveDatName(const int& id, TCHAR* fileName)
{
	_tcscpy(szDataNames[id], fileName);

	// omit ".dat"
	size_t pos = _tcslen(szDataNames[id]);
	while (pos != 0 && szDataNames[id][pos] != '.') pos--;
	szDataNames[id][pos] = '\0';
}

static inline int checkPatchState(HTREEITEM item)
{
	for (int i = 0; i < nNumPatches; i++) {
		if (item == hPatchHandlesIndex[i]) {
			return i;
		}
	}
	return -1;
}

static int parseRelations()
{
	depTable.clear();
	confTable.clear();

	char assistName[MAX_PATH];
	sprintf(assistName, "%s\\%s\\%s", WtoA(getMiscPath(PATH_IPS)), BurnDrvGetTextA(DRV_NAME), "assistant.txt");

	//parse ips dat and update the treewidget
	FILE* fp = fopen(assistName, "rt");
	if (!fp) {
		return 1;
	}

	char s[1024];
	char* p = NULL;
	string line;
	size_t nLen = 0;
	size_t pos = string::npos;

	while (!feof(fp)) {
		if (fgets(s, sizeof s, fp) != NULL) {
			p = s;
			// skip UTF-8 sig
			if (strncmp(p, UTF8_SIGNATURE, strlen(UTF8_SIGNATURE)) == 0) {
				p += strlen(UTF8_SIGNATURE);
			}

			if (p[0] == '#') {
				continue; // skip comment
			}

			getLine(p);
			line = p;

			pos = line.find(">");
			if (pos != string::npos) {
				StringList depStrs = split2Str(line, ">");
				string parent = *(depStrs.begin());
				lowerString(parent);
				StringList children = stringSplit(depStrs.back(), ",");
				lowerTrimmed(children);
				depTable.insert(make_pair(parent, children));
			} else {
				StringList conflicts = stringSplit(line, ",");
				lowerTrimmed(conflicts);
				confTable.push_back(conflicts);
			}
		}
	}

	return 0;
}

// function declare
static void validateConf(const string& datName);
static void validateDep(const string& datName);

static void applyRelations(int id)
{
	if (!bEnforceDep || !hasAssist || id < 0) {
		return;
	}

	int checked = _TreeView_GetCheckState(hIpsList, hPatchHandlesIndex[id]);
	string _datName = WtoA(szDataNames[id]);
	lowerString(_datName);
	itemStateTable[_datName] = checked ? 0 : 1;

	// validate relation
	validateConf(_datName);
	validateDep(_datName);

	// reset checkbox state
	string datName;
	for (int i = 0; i < nNumPatches; i++) {
		datName = WtoA(szDataNames[i]);
		lowerString(datName);
		checked = itemStateTable[datName];
		_TreeView_SetCheckState(hIpsList, hPatchHandlesIndex[i], checked ? TRUE : FALSE);
	}

	// set reverse state to current checkbox
	checked = itemStateTable[_datName];
	_TreeView_SetCheckState(hIpsList, hPatchHandlesIndex[id], checked ? FALSE : TRUE);
}

static void initRelations()
{
	// clear state
	itemStateTable.clear();

	// reset checkbox state
	string datName;
	int nChecked = 0;

	for (int i = 0; i < nNumPatches; i++) {
		nChecked = _TreeView_GetCheckState(hIpsList, hPatchHandlesIndex[i]);
		datName = WtoA(szDataNames[i]);
		lowerString(datName);
		itemStateTable.insert(make_pair(datName, nChecked));
	}
}

static void validateConf(const string& datName)
{
#ifdef _DEBUG
	printf("validateConf: %s\n", datName.c_str());
#endif

	if (itemStateTable[datName] == 0)
		return;

	//iterate confTable
	list<StringList>::iterator iter = confTable.begin();
	for (; iter != confTable.end(); iter++) {
		StringList confList = *iter;

		//locate the confList we're interested in
		if (stringListContain(confList, datName)) {
			//uncheck items in the confList
			string conf;
			StringList::iterator it = confList.begin();
			for (; it != confList.end(); it++) {
				conf = *it;
				if (datName == conf)
					continue;

				if (itemStateTable[conf] == 1) {
					itemStateTable[conf] = 0;
#ifdef _DEBUG
					printf("auto unchk: %s\n", conf.c_str());
#endif
					validateDep(conf);
				}
			}
		}
	}
}

static void validateDep(const string& datName)
{
	//depend case A: if parent (datName) is unchecked, uncheck all dependant children
	if (itemStateTable[datName] == 0) {
		//depTable might contain more than 1 entries for a single parent
		multimap<string, StringList>::iterator _iter = depTable.begin();
		for (; _iter != depTable.end(); _iter++) {
			if (datName != _iter->first)
				continue;

			//if dep parent is found in the depTable
			StringList depChildren = _iter->second;

			//uncheck all dependant children
			StringList::iterator iter = depChildren.begin();
			for (; iter != depChildren.end(); iter++) {
				string depChild = *iter;

				//only when a child is checked
				if (itemStateTable[depChild] == 0)
					continue;

				bool isUncheck = true;

				// if any other parent is checked, ignore the unchecking
				multimap<string, StringList>::iterator it = depTable.begin();
				for (; it != depTable.end(); it++) {
					string parent = it->first;

					//only other parents
					if (parent == datName)
						continue;
					//only when another parent is checked
					if (itemStateTable[parent] == 0)
						continue;

					StringList children = it->second;
					if (stringListContain(children, depChild)) {
						isUncheck = false;
						break;
					}
				}

				if (isUncheck) {
					itemStateTable[depChild] = 0;
#ifdef _DEBUG
					printf("auto unchk: %s\n", depChild.c_str());
#endif
					validateDep(depChild);
				}
			}
		}
	}
	//depend case B: if any child ($datName) is checked, check 1 of its parents
	else if (itemStateTable[datName] == 1) {
		multimap<string, StringList>::iterator it = depTable.begin();
		for (; it != depTable.end(); it++) {
			StringList children = it->second;
			if (stringListContain(children, datName)) {
				string parent = it->first;

				//only when another parent is unchecked
				if (itemStateTable[parent] == 1)
					continue;

				bool isCheck = true;

				// if any other parent (that is not $parent) of child ($datName) is checked, ignore the checking
				multimap<string, StringList>::iterator _it = depTable.begin();
				for (; _it != depTable.end(); _it++) {
					string _parent = _it->first;

					//if any other parent (that is not $parent)
					if (_parent == parent)
						continue;

					StringList _children = _it->second;
					//of child ($datName) is checked
					if (stringListContain(_children, datName) && itemStateTable[_parent] == 1) {
						//ignore the checking
						isCheck = false;
						break;
					}
				}

				if (isCheck) {
					itemStateTable[parent] = 1;
#ifdef _DEBUG
					printf("auto chk: %s\n", parent.c_str());
#endif
					validateConf(parent);
					validateDep(parent);
					break;
				}
			}
		}
	}
}
// <== IPSv4 conflict/dependancy assistant


// preview image position, added by regret
static int previewOrgX;
static int previewOrgY;

static inline void initPatchPreviewPos()
{
	RECT rect;
	GetWindowRect(GetDlgItem(hIpsDlg, IDC_SCREENSHOT_H), &rect);
	POINT pt;
	pt.x = rect.left; pt.y = rect.top;
	ScreenToClient(hIpsDlg, &pt);

	previewOrgX = pt.x;
	previewOrgY = pt.y;
}

static inline void movePatchPreviewPos()
{
	extern void getResizeOffset(int& x, int& y);
	int offsetx, offsety;
	getResizeOffset(offsetx, offsety);

	SetWindowPos(GetDlgItem(hIpsDlg, IDC_SCREENSHOT_H), NULL,
			previewOrgX + offsetx, previewOrgY + offsety, 0, 0,
			SWP_NOSIZE | SWP_NOSENDCHANGING | SWP_NOZORDER);
}

static inline void resetPatchPreviewPos()
{
	SetWindowPos(GetDlgItem(hIpsDlg, IDC_SCREENSHOT_H), NULL,
			previewOrgX, previewOrgY, 0, 0,
			SWP_NOSIZE | SWP_NOSENDCHANGING | SWP_NOZORDER);
}


static TCHAR* gameIpsConfigName()
{
	// Return the path of the config file for this game
	static TCHAR szName[64];
	_stprintf(szName, _T("config\\ips\\%s.ini"), BurnDrvGetText(DRV_NAME));
	return szName;
}

static void fillListBox()
{
	WIN32_FIND_DATA wfd;
	HANDLE hSearch;
	FILE* fp = NULL;

	TCHAR szFilePath[MAX_PATH];
	TCHAR szFilePathSearch[MAX_PATH];
	TCHAR szFileName[MAX_PATH];
	TCHAR PatchName[MAX_PATH];

	int nHandlePos = 0;
	int nNumNodes = 0;

	nPatchIndex = 0;
	nNumPatches = 0;

	TVINSERTSTRUCT TvItem;
	memset(&TvItem, 0, sizeof(TvItem));
	TvItem.item.mask = TVIF_TEXT | TVIF_PARAM;
	TvItem.hInsertAfter = TVI_LAST;

	_stprintf(szFilePath, _T("%s%s\\"), getMiscPath(PATH_IPS), BurnDrvGetText(DRV_NAME));
	_stprintf(szFilePathSearch, _T("%s*.dat"), szFilePath);

	hSearch = FindFirstFile(szFilePathSearch, &wfd);

	if (hSearch != INVALID_HANDLE_VALUE) {
		int Done = 0;

		while (!Done) {
			memset(szFileName, '\0', MAX_PATH);
			_stprintf(szFileName, _T("%s%s"), szFilePath, wfd.cFileName);

			fp = _tfopen(szFileName, _T("r"));
			if (fp) {
				memset(PatchName, '\0', MAX_PATH);

				TCHAR* PatchDesc = getPatchDescByLangcode(fp, nPatchLang);
				// If not available - try English first
				if (PatchDesc == NULL) {
					PatchDesc = getPatchDescByLangcode(fp, 0);
					// Simplified Chinese is the reference language
					if (PatchDesc == NULL) {
						PatchDesc = getPatchDescByLangcode(fp, 1);
					}
				}
				if (PatchDesc == NULL) {
					fclose(fp);
					break;
				}

				for (unsigned int i = 0; i < _tcslen(PatchDesc); i++) {
					if (PatchDesc[i] == '\r' || PatchDesc[i] == '\n') {
						break;
					}
					PatchName[i] = PatchDesc[i];
				}

				// Check for categories
				int nNumTokens = 0;
				TCHAR szCategory[MAX_PATH];
				TCHAR tempText[MAX_PATH];
				TCHAR itemName[MAX_PATH] = _T("");
				TVITEM Tvi;
				size_t nPatchNameLength = _tcslen(PatchName);

				TCHAR* Tokens = _tcstok(PatchName, _T("/"));
				while (Tokens != NULL) {
					if (nNumTokens == 0) {
						bool bAddItem = true;

						// Check if item already exists
						nNumNodes = TreeView_GetCount(hIpsList);
						for (int i = 0; i < nNumNodes; i++) {
							memset(&Tvi, 0, sizeof(Tvi));
							Tvi.hItem = hItemHandles[i];
							Tvi.mask = TVIF_TEXT | TVIF_HANDLE;
							Tvi.pszText = tempText;
							Tvi.cchTextMax = MAX_PATH;
							TreeView_GetItem(hIpsList, &Tvi);

							if (!_tcsicmp(Tvi.pszText, Tokens)) {
								bAddItem = false; // item exists, don't add it
								break;
							}
						}

						// add category
						if (bAddItem) {
							TvItem.hParent = TVI_ROOT;
							TvItem.item.pszText = Tokens;
							hItemHandles[nHandlePos] = TreeView_InsertItem(hIpsList, &TvItem);

							nHandlePos++;
							if (nHandlePos >= MAX_NODES) {
								nHandlePos = MAX_NODES - 1;
							}
						}

						if (_tcslen(Tokens) == nPatchNameLength) {
							saveDatName(nPatchIndex, wfd.cFileName);
							hPatchHandlesIndex[nPatchIndex] = hItemHandles[nHandlePos - 1];
							_tcscpy(szPatchFileNames[nPatchIndex], szFileName);

							nPatchIndex++;
							if (nPatchIndex >= MAX_NODES) {
								nPatchIndex = MAX_NODES - 1;
							}
						}

						_tcscpy(szCategory, Tokens);
					} else {
						HTREEITEM hNode = TVI_ROOT;
						// See which category we should be in
						nNumNodes = TreeView_GetCount(hIpsList);
						for (int i = 0; i < nNumNodes; i++) {
							memset(&Tvi, 0, sizeof(Tvi));
							Tvi.hItem = hItemHandles[i];
							Tvi.mask = TVIF_TEXT | TVIF_HANDLE;
							Tvi.pszText = tempText;
							Tvi.cchTextMax = MAX_PATH;
							TreeView_GetItem(hIpsList, &Tvi);

							if (!_tcsicmp(Tvi.pszText, szCategory)) {
								hNode = Tvi.hItem;
								break;
							}
						}

						// add ips items
						saveDatName(nPatchIndex, wfd.cFileName);
						_stprintf(itemName, _T("%s [%s]"), Tokens, wfd.cFileName);
						TvItem.hParent = hNode;
						TvItem.item.pszText = itemName;
						hItemHandles[nHandlePos] = TreeView_InsertItem(hIpsList, &TvItem);

						hPatchHandlesIndex[nPatchIndex] = hItemHandles[nHandlePos];
						_tcscpy(szPatchFileNames[nPatchIndex], szFileName);

						nHandlePos++;
						if (nHandlePos >= MAX_NODES) {
							nHandlePos = MAX_NODES - 1;
						}
						nPatchIndex++;
						if (nPatchIndex >= MAX_NODES) {
							nPatchIndex = MAX_NODES - 1;
						}
					}

					Tokens = _tcstok(NULL, _T("/"));
					nNumTokens++;
				}

				fclose(fp);
			}

			Done = !FindNextFile(hSearch, &wfd);
		}

		FindClose(hSearch);
	}

	nNumPatches = nPatchIndex;

	// Expand all branches
	nNumNodes = TreeView_GetCount(hIpsList);
	for (int i = 0; i < nNumNodes; i++) {
		TreeView_Expand(hIpsList, hItemHandles[i], TVE_EXPAND);
	}
}

int getNumActivePatches()
{
	int nActivePatches = 0;

	for (int i = 0; i < MAX_ACTIVE_PATCHES; i++) {
		if (_tcsicmp(szActivePatches[i], _T("")))
			nActivePatches++;
	}

	return nActivePatches;
}

void loadActivePatches()
{
	TCHAR szLine[MAX_PATH];
	int nActivePatches = 0;
	size_t nLen;
	FILE* fp = _tfopen(gameIpsConfigName(), _T("rt"));
	if (fp) {
		while (_fgetts(szLine, sizeof(szLine), fp)) {
			nLen = _tcslen(szLine);

			// Get rid of the linefeed at the end
			if (nLen != 0 && (szLine[nLen - 1] == '\r' || szLine[nLen - 1] == '\n')) {
				szLine[nLen - 1] = 0;
				nLen--;
			}

			if (!_tcsnicmp(szLine, _T("//"), 2)) continue;
			if (!_tcsicmp(szLine, _T(""))) continue;

			_stprintf(szActivePatches[nActivePatches], _T("%s%s\\%s"), getMiscPath(PATH_IPS), BurnDrvGetText(DRV_NAME), szLine);

			nActivePatches++;
			if (nActivePatches >= MAX_ACTIVE_PATCHES) {
				break;
			}
		}

		fclose(fp);
	}
}

static void checkActivePatches()
{
	loadActivePatches();

	const int nActivePatches = getNumActivePatches();

	for (int i = 0; i < nActivePatches; i++) {
		for (int j = 0; j < nNumPatches; j++) {
			if (!_tcsicmp(szActivePatches[i], szPatchFileNames[j])) {
				_TreeView_SetCheckState(hIpsList, hPatchHandlesIndex[j], TRUE);
			}
		}
	}

	// ips relation
	initRelations();
}

void __cdecl applyPatches(UINT8* base, char* rom_name, int len)
{
	if (!bDoPatch || !base || !rom_name) {
		return;
	}

	const int nActivePatches = getNumActivePatches();

	for (int i = 0; i < nActivePatches; i++) {
		doPatchGame(WtoA(szActivePatches[i]), rom_name, base, len);
	}
}

static void refreshPatch()
{
	SendDlgItemMessage(hIpsDlg, IDC_TEXTCOMMENT, WM_SETTEXT, 0, 0);

	HTREEITEM hSelectHandle = TreeView_GetNextItem(hIpsList, ~0U, TVGN_CARET);
	if (!hSelectHandle) {
		return;
	}

	if (hPreview) {
		DeleteObject((HGDIOBJ)hPreview);
		hPreview = NULL;
	}

	for (int i = 0; i < nNumPatches; i++) {
		if (hSelectHandle == hPatchHandlesIndex[i]) {
			FILE* fp = _tfopen(szPatchFileNames[i], _T("r"));
			if (fp) {
				TCHAR* PatchDesc = getPatchDescByLangcode(fp, nPatchLang);
				// If not available - try English first
				if (PatchDesc == NULL) {
					PatchDesc = getPatchDescByLangcode(fp, 0);
					// Simplified Chinese is the reference language
					if (PatchDesc == NULL) {
						PatchDesc = getPatchDescByLangcode(fp, 1);
					}
				}
				if (PatchDesc == NULL) {
					fclose(fp);
					break;
				}

				SendDlgItemMessage(hIpsDlg, IDC_TEXTCOMMENT, WM_SETTEXT, 0, (LPARAM)PatchDesc);
				fclose(fp);
			}

			// set preview image filename
			TCHAR szPatchDatName[MAX_PATH] = _T("");
			TCHAR szImageFileName[MAX_PATH] = _T("");

			TCHAR* szGameName = BurnDrvGetText(DRV_NAME);
			getPatchFilename(szGameName, szPatchDatName, i);
			getPatchPrevName(szImageFileName, szGameName, szPatchDatName);
			_tcscat(szImageFileName, _T(".png"));

			// load image
			HBITMAP hNewImage = loadImageFromFile(szImageFileName, hIpsDlg, nLoadMenuShowX & KEEPGAMEASPECT);
			if (hNewImage) {
				hPreview = hNewImage;
				SendDlgItemMessage(hIpsDlg, IDC_SCREENSHOT_H, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hPreview);
				movePatchPreviewPos();
			} else {
				SendDlgItemMessage(hIpsDlg, IDC_SCREENSHOT_H, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hDefPreview);
				resetPatchPreviewPos();
			}

			return;
		}
	}

	SendDlgItemMessage(hIpsDlg, IDC_SCREENSHOT_H, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hDefPreview);
	resetPatchPreviewPos();
}

static void savePatches()
{
	// clear active patch index
	for (int i = 0; i < MAX_ACTIVE_PATCHES; i++) {
		_stprintf(szActivePatches[i], _T(""));
	}

	int nChecked = 0;
	int nActivePatches = 0;
	for (int i = 0; i < nNumPatches; i++) {
		nChecked = _TreeView_GetCheckState(hIpsList, hPatchHandlesIndex[i]);
		if (nChecked) {
			_tcscpy(szActivePatches[nActivePatches], szPatchFileNames[i]);
			nActivePatches++;
			if (nActivePatches >= MAX_ACTIVE_PATCHES) {
				break;
			}
		}
	}

	FILE* fp = _tfopen(gameIpsConfigName(), _T("wt"));
	if (fp) {
		_ftprintf(fp, _T("// ") _T(APP_TITLE) _T(" v%s --- IPS Config File for %s (%hs)\n\n"),
			szAppBurnVer, BurnDrvGetText(DRV_NAME), BurnDrvGetTextA(DRV_FULLNAME));

		TCHAR szPatchName[MAX_PATH];
		TCHAR szFileName[MAX_PATH];
		TCHAR* Tokens = NULL;
		for (int i = 0; i < nActivePatches; i++) {
			_tcscpy(szPatchName, szActivePatches[i]); // make a copy, modified by regret
			Tokens = _tcstok(szPatchName, _T("\\"));
			while (Tokens != NULL) {
				szFileName[0] = _T('\0');
				_tcscpy(szFileName, Tokens);
				Tokens = _tcstok(NULL, _T("\\"));
			}

			_ftprintf(fp, _T("%s\n"), szFileName);
		}
		fclose(fp);
	}

	// if no active patch, delete config file
	if (nActivePatches == 0) {
		_tremove(gameIpsConfigName());
	}
}

static void clearPatches()
{
	int nChecked = 0;
	for (int i = 0; i < nNumPatches; i++) {
		nChecked = _TreeView_GetCheckState(hIpsList, hPatchHandlesIndex[i]);
		if (nChecked) {
			_TreeView_SetCheckState(hIpsList, hPatchHandlesIndex[i], FALSE);
		}
	}

	// clear ips relation
	itemStateTable.clear();
}

static void rebuildIpsList()
{
	fillListBox();
	checkActivePatches();

	// parse ips relations
	if (parseRelations()) {
		hasAssist = FALSE;
	} else {
		hasAssist = TRUE;
	}
	EnableWindow(GetDlgItem(hIpsDlg, IDC_IPS_ENFORCE), hasAssist);
}

int patchSettingInit()
{
	// clear active patch index
	for (int i = 0; i < MAX_ACTIVE_PATCHES; i++) {
		_stprintf(szActivePatches[i], _T(""));
	}

	// Get game full name
	TCHAR szText[1024] = _T("");
	TCHAR* pszFullName = transGameName(BurnDrvGetText(DRV_FULLNAME));
	_stprintf(szText, _T("%s") _T(SEPERATOR_1) _T("%s"), FBALoadStringEx(IDS_IPSMANAGER), pszFullName);

	// Set the window caption
	SetWindowText(hIpsDlg, szText);

	// init ips language combobox
	for (int i = 0; ui_lang_info[i].name; i++) {
		SendDlgItemMessage(hIpsDlg, IDC_IPS_LANG, CB_ADDSTRING, 0, (LPARAM)AtoW(ui_lang_info[i].name));
	}
	SendDlgItemMessage(hIpsDlg, IDC_IPS_LANG, CB_SETCURSEL, (WPARAM)nPatchLang, 0);

	CheckDlgButton(hIpsDlg, IDC_IPS_ENFORCE, bEnforceDep ? BST_CHECKED : BST_UNCHECKED);

	rebuildIpsList();

	return 0;
}

void patchSettingExit()
{
	SendDlgItemMessage(hIpsDlg, IDC_SCREENSHOT_H, STM_SETIMAGE, IMAGE_BITMAP, 0);

	memset(hItemHandles, 0, MAX_NODES);
	memset(hPatchHandlesIndex, 0, MAX_NODES);

	nPatchIndex = 0;
	nNumPatches = 0;

	for (int i = 0; i < MAX_NODES; i++) {
		szPatchFileNames[i][0] = _T('\0');
		szDataNames[i][0] = _T('\0');
	}

	// clear relations
	depTable.clear();
	confTable.clear();
	itemStateTable.clear();

	if (hPreview) {
		DeleteObject((HGDIOBJ)hPreview);
		hPreview = NULL;
	}

	if (hDefPreview) {
		DeleteObject((HGDIOBJ)hDefPreview);
		hDefPreview = NULL;
	}

	DeleteObject(hWhiteBGBrush);

	EndDialog(hIpsDlg, 0);
}

static void ipsOkay()
{
	savePatches();
	patchSettingExit();
}

static INT_PTR CALLBACK IPSDlgProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch (Msg) {
		case WM_INITDIALOG: {
			hIpsDlg = hDlg;
			hIpsList = GetDlgItem(hIpsDlg, IDC_IPSTREE1);

			hWhiteBGBrush = CreateSolidBrush(RGB(0xFF,0xFF,0xFF));
			hDefPreview = LoadBitmap(hAppInst, MAKEINTRESOURCE(BMP_PREVIEW));

			DWORD dwStyle = (DWORD) GetWindowLongPtr(hIpsList, GWL_STYLE);
			dwStyle |= TVS_CHECKBOXES;
			SetWindowLongPtr(hIpsList, GWL_STYLE, (LONG_PTR) dwStyle);

			patchSettingInit();

			initPatchPreviewPos();

			wndInMid(hDlg, hScrnWnd);
			SetFocus(hDlg);											// Enable Esc=close
			break;
		}

		case WM_COMMAND: {
			int wID = LOWORD(wParam);
			int Notify = HIWORD(wParam);

			if (Notify == BN_CLICKED) {
				switch (wID) {
					case IDOK: {
						ipsOkay();
						break;
					}

					case IDCANCEL: {
						SendMessage(hDlg, WM_CLOSE, 0, 0);
						return 0;
					}

					case IDC_IPSCLEAR: {
						clearPatches();
						break;
					}

					case IDC_IPS_ENFORCE: {
						bEnforceDep = !bEnforceDep;
						break;
					}
				}
			}

			if (wID == IDC_IPS_LANG && Notify == CBN_SELCHANGE) {
				nPatchLang = SendDlgItemMessage(hIpsDlg, IDC_IPS_LANG, CB_GETCURSEL, 0, 0);
				TreeView_DeleteAllItems(hIpsList);

				// rebuild ips list
				rebuildIpsList();
				return 0;
			}

			break;
		}

		case WM_NOTIFY: {
			NMHDR* pNmHdr = (NMHDR*)lParam;

			if (LOWORD(wParam) == IDC_IPSTREE1 && pNmHdr->code == TVN_SELCHANGED) {
				refreshPatch();
				return 1;
			}

			if (LOWORD(wParam) == IDC_IPSTREE1 && pNmHdr->code == NM_DBLCLK) {
				// disable double-click node-expand
				SetWindowLongPtr(hIpsDlg, DWLP_MSGRESULT, 1);
				return 1;
			}

			if (LOWORD(wParam) == IDC_IPSTREE1 && pNmHdr->code == NM_CLICK) {
				POINT cursorPos;
				GetCursorPos(&cursorPos);
				ScreenToClient(hIpsList, &cursorPos);

				TVHITTESTINFO thi;
				thi.pt = cursorPos;
				TreeView_HitTest(hIpsList, &thi);

				if (thi.flags == TVHT_ONITEMSTATEICON) {
					TreeView_SelectItem(hIpsList, thi.hItem);

					// check state and get patch index
					int id = checkPatchState(thi.hItem);
					// apply patch relations
					applyRelations(id);
				}
				return 1;
			}

			SetWindowLongPtr(hIpsDlg, DWLP_MSGRESULT, CDRF_DODEFAULT);
			return 1;
		}

		case WM_CTLCOLORSTATIC:
			if ((HWND)lParam == GetDlgItem(hIpsDlg, IDC_TEXTCOMMENT)) {
				return (BOOL)hWhiteBGBrush;
			}
			break;

		case WM_CLOSE:
			patchSettingExit();
			break;
	}

	return 0;
}

int patchSettingCreate(HWND parent)
{
	FBADialogBox(IDD_IPS, parent, (DLGPROC)IPSDlgProc);
	return 1;
}
// <== IPS setting

#endif
