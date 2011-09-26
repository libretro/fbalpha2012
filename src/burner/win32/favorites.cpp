// favorites module, added by regret

/* changelog:
 update 1: create
*/

#include "burner.h"

map<string, int> favoriteMap;
static bool favLoaded = false;
static bool favChanged = false;

int initFavorites()
{
	if (favLoaded) {
		return 0;
	}

	favoriteMap.clear();

	//parse ips dat and update the treewidget
	FILE* fp = fopen("config\\favorites.ini", "rt");
	if (!fp) {
		return 1;
	}

	int i = 0;
	size_t nLen = 0;
	char s[MAX_PATH];
	string line;

	while (!feof(fp))
	{
		if (fgets(s, sizeof s, fp) != NULL)
		{
			if (s[0] == ';' || (s[0] == '/' && s[1] == '/'))
				continue; // skip comment

			getLine(s);
			line = s;

			if (line == "[Favorites]" || line == "\r\n" || line == "\n")
				continue;

			favoriteMap[line] = i;
			i++;
		}
	}
	fclose(fp);

	favLoaded = true;
	return 0;
}

int saveFavorites()
{
	if (!favChanged) {
		return 0;
	}

	FILE* fp = NULL;

	fp = fopen("config\\favorites.ini", "wt");
	if (!fp) {
		return 1;
	}

	fprintf(fp, "// " APP_TITLE " v%s favorite file\n\n", WtoA(szAppBurnVer));
	fprintf(fp, "[Favorites]\n");

	string game;
	map<string, int>::iterator iter = favoriteMap.begin();
	for(; iter != favoriteMap.end(); iter++)
	{
		game = iter->first;
		if (game != "\r" && game != "\n" && game != "") {
			fprintf(fp, "%s\n", game.c_str());
		}
	}
	fclose(fp);

	favChanged = false;
	return 0;
}

void addFavorite(unsigned int index)
{
	if (index >= nBurnDrvCount)
		return;

	unsigned int nOldBurnDrvSelect = nBurnDrvSelect;

	nBurnDrvSelect = index;
	string game = BurnDrvGetTextA(DRV_NAME);

	map<string, int>::iterator iter = favoriteMap.find(game);
	if (iter == favoriteMap.end()) {
		favoriteMap[game] = index;
	}

	nBurnDrvSelect = nOldBurnDrvSelect;

	favChanged = true;
}

void removeFavorite(unsigned int index)
{
	if (index >= nBurnDrvCount)
		return;

	unsigned int nOldBurnDrvSelect = nBurnDrvSelect;

	nBurnDrvSelect = index;
	string game = BurnDrvGetTextA(DRV_NAME);

	map<string, int>::iterator iter = favoriteMap.find(game);
	if (iter != favoriteMap.end()) {
		favoriteMap.erase(iter);
	}

	nBurnDrvSelect = nOldBurnDrvSelect;

	favChanged = true;
}

bool filterFavorite(const unsigned int& index)
{
	unsigned int nOldBurnDrvSelect = nBurnDrvSelect;

	nBurnDrvSelect = index;
	string game = BurnDrvGetTextA(DRV_NAME);

	bool ret;
	map<string, int>::iterator iter = favoriteMap.find(game);
	if (iter != favoriteMap.end()) {
		ret = false;
	} else {
		ret = true;
	}

	nBurnDrvSelect = nOldBurnDrvSelect;

	return ret;
}
