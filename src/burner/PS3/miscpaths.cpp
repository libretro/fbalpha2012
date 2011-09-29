// custom misc paths, by regret
#include "burner.h"
 
char szMiscPaths[PATH_SUM][MAX_PATH] = {
	"/dev_hdd0/game/FBAN00000/USRDIR/previews/",
	"/dev_hdd0/game/FBAN00000/USRDIR/cheats/",
	"/dev_hdd0/game/FBAN00000/USRDIR/screenshots/",
	"/dev_hdd0/game/FBAN00000/USRDIR/savestates/",
	"/dev_hdd0/game/FBAN00000/USRDIR/recordings/",
	"/dev_hdd0/game/FBAN00000/USRDIR/skins/",
	"/dev_hdd0/game/FBAN00000/USRDIR/ips/",
	"/dev_hdd0/game/FBAN00000/USRDIR/titles/",
	"/dev_hdd0/game/FBAN00000/USRDIR/flyers/",
	"/dev_hdd0/game/FBAN00000/USRDIR/scores/",
	"/dev_hdd0/game/FBAN00000/USRDIR/selects/",
	"/dev_hdd0/game/FBAN00000/USRDIR/gameovers/",
	"/dev_hdd0/game/FBAN00000/USRDIR/bosses/",
	"/dev_hdd0/game/FBAN00000/USRDIR/icons/",
};

char szAppRomPaths[DIRS_MAX][MAX_PATH] = {
{ "/dev_hdd0/game/FBAN00000/USRDIR/roms/" }
};
 
 
const char * getMiscPath(unsigned int dirType)
{
	if (dirType < PATH_PREVIEW || dirType >= PATH_SUM)
		return NULL;

	return szMiscPaths[dirType];
}

const char * getMiscArchiveName(unsigned int dirType)
{
	if (dirType < PATH_PREVIEW || dirType >= PATH_SUM)
		return NULL;

	static char szArchiveName[64] = "";

	switch (dirType)
	{
		case PATH_PREVIEW:
			_tcscpy(szArchiveName, "snap");
			break;
		case PATH_TITLE:
			_tcscpy(szArchiveName, "titles");
			break;
		case PATH_FLYER:
			_tcscpy(szArchiveName, "flyers");
			break;
		case PATH_SCORE:
			_tcscpy(szArchiveName, "score");
			break;
		case PATH_SELECT:
			_tcscpy(szArchiveName, "select");
			break;
		case PATH_GAMEOVER:
			_tcscpy(szArchiveName, "gameover");
			break;
		case PATH_BOSS:
			_tcscpy(szArchiveName, "boss");
			break;

		case PATH_CHEAT:
			_tcscpy(szArchiveName, "cheat");
			break;
		case PATH_SKIN:
			_tcscpy(szArchiveName, "skin");
			break;
		case PATH_ICON:
			_tcscpy(szArchiveName, "icons");
			break;
	}

	return szArchiveName;
}
 
