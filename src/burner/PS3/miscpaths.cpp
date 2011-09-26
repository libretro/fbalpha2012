// custom misc paths, by regret
#include "burner.h"
 
#if defined (_XBOX)
TCHAR szMiscPaths[PATH_SUM][MAX_PATH] = {
	_T("game:\\previews\\"), _T("game:\\cheats\\"), _T("game:\\screenshots\\"), _T("game:\\savestates\\"), _T("game:\\recordings\\"),
	_T("game:\\skins\\"), _T("game:\\ips\\"), _T("game:\\titles\\"), _T("game:\\flyers\\"),
	_T("game:\\scores\\"), _T("game:\\selects\\"), _T("game:\\gameovers\\"), _T("game:\\bosses\\"), _T("game:\\icons\\"),
};
#else
TCHAR szMiscPaths[PATH_SUM][MAX_PATH] = {
	_T("/dev_hdd0/game/FBAN00000/USRDIR/previews/"), _T("/dev_hdd0/game/FBAN00000/USRDIR/cheats/"), _T("/dev_hdd0/game/FBAN00000/USRDIR/screenshots/"), _T("/dev_hdd0/game/FBAN00000/USRDIR/savestates/"), _T("/dev_hdd0/game/FBAN00000/USRDIR/recordings/"),
	_T("/dev_hdd0/game/FBAN00000/USRDIR/skins/"), _T("/dev_hdd0/game/FBAN00000/USRDIR/ips/"), _T("/dev_hdd0/game/FBAN00000/USRDIR/titles/"), _T("/dev_hdd0/game/FBAN00000/USRDIR/flyers/"),
	_T("/dev_hdd0/game/FBAN00000/USRDIR/scores/"), _T("/dev_hdd0/game/FBAN00000/USRDIR/selects/"), _T("/dev_hdd0/game/FBAN00000/USRDIR/gameovers/"), _T("/dev_hdd0/game/FBAN00000/USRDIR/bosses/"), _T("/dev_hdd0/game/FBAN00000/USRDIR/icons/"),
};
#endif

TCHAR szAppRomPaths[DIRS_MAX][MAX_PATH] = {
#if defined (_XBOX)
	{ _T("game:\\roms\\") }
#else
	{ _T("/dev_hdd0/game/FBAN00000/USRDIR/roms/") }
#endif
};
 
 

int RomsDirCreate(HWND parent)
{
	 
	return 0;
}

 
int miscDirCreate(HWND parent)
{
	 
	return 0;
}

const TCHAR* getMiscPath(unsigned int dirType)
{
	if (dirType < PATH_PREVIEW || dirType >= PATH_SUM) {
		return NULL;
	}
	return szMiscPaths[dirType];
}

const TCHAR* getMiscArchiveName(unsigned int dirType)
{
	if (dirType < PATH_PREVIEW || dirType >= PATH_SUM) {
		return NULL;
	}

	static TCHAR szArchiveName[64] = _T("");

	switch (dirType) {
		case PATH_PREVIEW:
			_tcscpy(szArchiveName, _T("snap"));
			break;
		case PATH_TITLE:
			_tcscpy(szArchiveName, _T("titles"));
			break;
		case PATH_FLYER:
			_tcscpy(szArchiveName, _T("flyers"));
			break;
		case PATH_SCORE:
			_tcscpy(szArchiveName, _T("score"));
			break;
		case PATH_SELECT:
			_tcscpy(szArchiveName, _T("select"));
			break;
		case PATH_GAMEOVER:
			_tcscpy(szArchiveName, _T("gameover"));
			break;
		case PATH_BOSS:
			_tcscpy(szArchiveName, _T("boss"));
			break;

		case PATH_CHEAT:
			_tcscpy(szArchiveName, _T("cheat"));
			break;
		case PATH_SKIN:
			_tcscpy(szArchiveName, _T("skin"));
			break;
		case PATH_ICON:
			_tcscpy(szArchiveName, _T("icons"));
			break;
	}

	return szArchiveName;
}
 