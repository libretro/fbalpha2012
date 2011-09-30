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
			strcpy(szArchiveName, "snap");
			break;
		case PATH_TITLE:
			strcpy(szArchiveName, "titles");
			break;
		case PATH_FLYER:
			strcpy(szArchiveName, "flyers");
			break;
		case PATH_SCORE:
			strcpy(szArchiveName, "score");
			break;
		case PATH_SELECT:
			strcpy(szArchiveName, "select");
			break;
		case PATH_GAMEOVER:
			strcpy(szArchiveName, "gameover");
			break;
		case PATH_BOSS:
			strcpy(szArchiveName, "boss");
			break;
		case PATH_CHEAT:
			strcpy(szArchiveName, "cheat");
			break;
		case PATH_SKIN:
			strcpy(szArchiveName, "skin");
			break;
		case PATH_ICON:
			strcpy(szArchiveName, "icons");
			break;
	}

	return szArchiveName;
}
