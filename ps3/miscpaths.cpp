// custom misc paths, by regret
#include "burner.h"
 
char szMiscPaths[PATH_SUM][MAX_PATH] = {
	PREVIEWS_DIR,
	CHEATS_DIR,
	SCREENSHOTS_DIR,
	SAVESTATES_DIR,
	RECORDINGS_DIR,
	SKINS_DIR,
	IPS_DIR,
	TITLES_DIR,
	FLYERS_DIR,
	SCORES_DIR,
	SELECTS_DIR,
	GAMEOVERS_DIR,
	BOSSES_DIR,
	ICONS_DIR,
};

char szAppRomPaths[DIRS_MAX][MAX_PATH] = {
{ ROMS_DIR }
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
