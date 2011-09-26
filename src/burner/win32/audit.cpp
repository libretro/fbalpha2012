// Burner new audit module, added by regret
// NO_DUMP: CRC == 0
// BAD_DUMP: CRC != 0 && flag == BRF_NODUMP

/* changelog:
 update 4: add disable crc check
 update 3: add nodump and baddump check
 update 2: add auditState interfaces
 update 1: rewrite audit method (almost same as MAMEPlus GUI)
*/

#include "burner.h"
#include "seldef.h"

#ifdef _DEBUG
// #define _CRTDBG_MAP_ALLOC
// #include <crtdbg.h>

// #define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

char* auditState = NULL;

// variable definitions
struct RomInfo
{
	string name;
	unsigned int size;
	unsigned int type;
	FIND_STATE state;
};

struct GameInfo
{
	string parent;
	string board;
	set<string> clones;
	map<unsigned int /*crc*/, RomInfo> roms;
};

static map<string, GameInfo*> allGameMap;
static bool getinfo = false;

static ArcEntry* List = NULL;
static int listCount = 0;

// string function
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

static inline void freeArchiveList()
{
	if (List) {
		for (int i = 0; i < listCount; i++) {
			free(List[i].szName);
			List[i].szName = NULL;
		}
		free(List);
		List = NULL;
	}
	listCount = 0;
}

static inline unsigned int getRomCount()
{
	unsigned int count = 0;
	for (count = 0; ; count++) {
		if (BurnDrvGetRomInfo(NULL, count)) {
			break;
		}
	}
	return count;
}

// find if gameinfo contain rom name
static inline RomInfo* getSetHasRom(GameInfo* info, string romname)
{
	if (!info || romname == "") {
		return NULL;
	}

	lowerString(romname);

	map<unsigned int, RomInfo>::iterator iter = info->roms.begin();
	for (; iter != info->roms.end(); iter++) {
		if (iter->second.name == romname) {
			return &(iter->second);
		}
	}
	return NULL;
}

static inline void deleteAllRomsetInfo()
{
	map<string, GameInfo*>::iterator iter = allGameMap.begin();
	for (; iter != allGameMap.end(); iter++) {
		if (iter->second) {
			delete iter->second;
		}
	}
	allGameMap.clear();

	getinfo = false;
}

static inline void clearAllRomsetState()
{
	GameInfo* gameInfo = NULL;
	RomInfo* romInfo = NULL;

	map<string, GameInfo*>::iterator iter = allGameMap.begin();
	for (; iter != allGameMap.end(); iter++) {
		gameInfo = iter->second;
		if (!gameInfo) {
			continue;
		}

		map<unsigned int, RomInfo>::iterator it = gameInfo->roms.begin();
		for (; it != gameInfo->roms.end(); it++) {
			romInfo = &it->second;
			if (!romInfo) {
				continue;
			}

			if (it->first == 0) {
				romInfo->state = STAT_OK; // pass no_dump rom
			} else {
				romInfo->state = STAT_NOFIND;
			}
		}
	}
}

static inline void getAllRomsetCloneInfo()
{
	GameInfo* gameInfo = NULL;
	GameInfo* parentInfo = NULL;

	map<string, GameInfo*>::iterator it;
	map<string, GameInfo*>::iterator iter = allGameMap.begin();
	for (; iter != allGameMap.end(); iter++) {
		gameInfo = iter->second;
		if (!gameInfo || (gameInfo->parent == "" && gameInfo->board == "")) {
			continue;
		}

		// add clone rom
		if (gameInfo->parent != "") {
			it = allGameMap.find(gameInfo->parent);
			if (it != allGameMap.end()) {
				parentInfo = it->second;
				if (parentInfo) {
					parentInfo->clones.insert(iter->first);
				}
			}
		}

		// add board rom
		if (gameInfo->board != "") {
			it = allGameMap.find(gameInfo->board);
			if (it != allGameMap.end()) {
				parentInfo = it->second;
				if (parentInfo) {
					parentInfo->clones.insert(iter->first);
				}
			}
		}
	}
}

// get all romsets info, only do once
int getAllRomsetInfo()
{
	if (getinfo) {
		return 0;
	}

	char* sname = NULL;
	BurnRomInfo ri;
	unsigned int romCount = 0;
	string name = "";

	unsigned int tempBurnDrvSelect = nBurnDrvSelect;

	// get all romset basic info
	for (nBurnDrvSelect = 0; nBurnDrvSelect < nBurnDrvCount; nBurnDrvSelect++) {
		// get game info
		GameInfo* gameInfo = new GameInfo;
		gameInfo->parent = BurnDrvGetTextA(DRV_PARENT) ? BurnDrvGetTextA(DRV_PARENT) : "";
		gameInfo->board = BurnDrvGetTextA(DRV_BOARDROM) ? BurnDrvGetTextA(DRV_BOARDROM) : "";
		name = BurnDrvGetTextA(DRV_NAME);

		// get rom info
		romCount = getRomCount();
		for (unsigned int i = 0; i < romCount; i++) {
			memset(&ri, 0, sizeof(ri));
			BurnDrvGetRomInfo(&ri, i); // doesn't contain rom name

			RomInfo romInfo;
			BurnDrvGetRomName(&sname, i, 0); // get rom name
			romInfo.name = sname;
			romInfo.size = ri.nLen;
			romInfo.type = ri.nType;
			if (ri.nCrc == 0) {
				romInfo.state = STAT_OK; // pass no_dump rom
			} else {
				romInfo.state = STAT_NOFIND;
			}

			gameInfo->roms[ri.nCrc] = romInfo;
		}

		// add gameinfo to list
		allGameMap[name] = gameInfo;
	}

	nBurnDrvSelect = tempBurnDrvSelect;

	getAllRomsetCloneInfo();

	getinfo = true;

	return 0;
}

int setCloneRomInfo(GameInfo* info, ArcEntry& list)
{
	if (!info) {
		return 1;
	}

	RomInfo* romInfo = NULL;
	GameInfo* cloneInfo = NULL;
	map<unsigned int, RomInfo>::iterator clone_iter;
	set<string>::iterator it = info->clones.begin();

	for (; it != info->clones.end(); it++) {
		cloneInfo = allGameMap[*it];
		clone_iter = cloneInfo->roms.find(list.nCrc);
		if (clone_iter != cloneInfo->roms.end()) {
			romInfo = &clone_iter->second;

			if (romInfo->size != list.nLen) {
				if (list.nLen < romInfo->size) {
					romInfo->state = STAT_SMALL;
				} else {
					romInfo->state = STAT_LARGE;
				}
			} else {
				romInfo->state = STAT_OK;
			}
		} else {
			// wrong CRC
			if (nLoadMenuShowX & DISABLECRC) {
				RomInfo* rom = getSetHasRom(cloneInfo, list.szName);
				if (rom) {
					rom->state = STAT_OK;
				}
			}
		}

		setCloneRomInfo(cloneInfo, list);
	}

	return 0;
}

extern void checkScanThread();
extern void romsSetProgress();

static int getArchiveInfo(const char* path, const char* name)
{
	if (!name || !path) {
		return 1;
	}

	// omit extension
	string _name = name;
	lowerString(_name); // case-insensitive
	size_t pos = _name.rfind(".");
	_name = _name.substr(0, pos);

//	if (_name == "sf2m13") {
//		int dummy = 0;
//	}

	// set progress
	checkScanThread();
	romsSetProgress();

	// find name
	map<string, GameInfo*>::iterator _it = allGameMap.find(_name);
	if (_it == allGameMap.end()) {
		return 1;
	}

	GameInfo* gameInfo = _it->second;
	if (!gameInfo) {
		return 1;
	}

	static char fileName[MAX_PATH];
	sprintf(fileName, "%s%s", path, name);

	if (archiveOpenA(fileName)) {
		return 1;
	}

	if (archiveGetList(&List, &listCount)) {
		freeArchiveList();
		return 1;
	}

	RomInfo* romInfo = NULL;
	map<unsigned int, RomInfo>::iterator iter;

	for (int i = 0; i < listCount; i++) {
		// check roms
		iter = gameInfo->roms.find(List[i].nCrc);
		if (iter != gameInfo->roms.end()) {
			romInfo = &iter->second;
			if (!romInfo) {
				continue;
			}

			if (romInfo->size != List[i].nLen) {
				if (List[i].nLen < romInfo->size) {
					romInfo->state = STAT_SMALL;
				} else {
					romInfo->state = STAT_LARGE;
				}
			} else {
				romInfo->state = STAT_OK;
			}
		} else {
			// wrong CRC
			if (nLoadMenuShowX & DISABLECRC) {
				RomInfo* rom = getSetHasRom(gameInfo, List[i].szName);
				if (rom) {
					rom->state = STAT_OK;
				}
			}
		}

		// check all clones
		setCloneRomInfo(gameInfo, List[i]);
	}

	archiveClose();

	freeArchiveList();

	return 0;
}

// get archive info in all cofigured pathes
int getFileInfo(bool scanonly)
{
	WIN32_FIND_DATAA wfd;
	HANDLE hFind = NULL;
	char fullPath[MAX_PATH];
	char szFile[MAX_PATH];
	unsigned int count = 0;

	for (int d = 0; d < DIRS_MAX; d++) {
		if (!_tcsicmp(szAppRomPaths[d], _T(""))) {
			continue; // skip empty path
		}
		sprintf(fullPath, "%s", WtoA(szAppRomPaths[d]));

		// find zip
		sprintf(szFile, "%s*.zip", fullPath);
		hFind = FindFirstFileA(szFile, &wfd);
		if (hFind != INVALID_HANDLE_VALUE) {
			do {
				if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
					continue;
				}

				if (!scanonly) {
					getArchiveInfo(fullPath, wfd.cFileName);
				}
				count++;
			} while (FindNextFileA(hFind, &wfd));

			FindClose(hFind);
		}

		// find 7z
		if (nLoadMenuShowX & CHECK7ZIP) {
			sprintf(szFile, "%s*.7z", fullPath);
			hFind = FindFirstFileA(szFile, &wfd);
			if (hFind != INVALID_HANDLE_VALUE) {
				do {
					if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
						continue;
					}

					if (!scanonly) {
						getArchiveInfo(fullPath, wfd.cFileName);
					}
					count++;
				} while (FindNextFileA(hFind, &wfd));

				FindClose(hFind);
			}
		}
	}

	return count;
}

void auditPrepare()
{
	getAllRomsetInfo();
	clearAllRomsetState();

	freeArchiveList();
	getFileInfo(false);
}

// audio all romsets
int auditRomset()
{
	string name = BurnDrvGetTextA(DRV_NAME);

//	if (name == "sfiii3an") {
//		int dummy = 2;
//	}

	// get rom info
	GameInfo* gameInfo = allGameMap[name];
	if (!gameInfo) {
		return AUDIT_FAIL;
	}

	RomInfo* romInfo = NULL;

	int ret = AUDIT_FULLPASS;
	map<unsigned int, RomInfo>::iterator iter = gameInfo->roms.begin();
	for (; iter != gameInfo->roms.end(); iter++) {
		romInfo = &iter->second;
		if (!romInfo) {
			continue;
		}

		if (romInfo->state != STAT_OK && romInfo->type && iter->first) {
			if (iter->first == 0) {
				continue; // no_dump
			}

			if (!(romInfo->type & BRF_OPT)) {
				return AUDIT_FAIL;
			}
			ret = AUDIT_PARTPASS;
		}
	}

	return ret;
}

void auditCleanup()
{
	deleteAllRomsetInfo();
}


// audit state
void initAuditState()
{
	if (auditState) {
		return;
	}

	auditState = (char*)malloc(nBurnDrvCount);
	resetAuditState();
}

void resetAuditState()
{
	if (auditState) {
		memset(auditState, 0, nBurnDrvCount);
	}
}

void freeAuditState()
{
	free(auditState);
	auditState = NULL;
}

char getAuditState(const unsigned int& id)
{
	if (id < nBurnDrvCount) {
		return auditState[id];
	}
	return AUDIT_FAIL;
}

void setAuditState(const unsigned int& id, char val)
{
	if (id >= nBurnDrvCount) {
		return;
	}
	auditState[id] = val;
}
