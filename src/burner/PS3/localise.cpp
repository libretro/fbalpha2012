// Language module, added by regret

/* changelog:
 update 2: add string cache
 update 1: create
*/

#include "burner.h"

#define LANG_CACHE 0

bool bLanguageActive = false;
char szLanguage[MAX_PATH] = "";
 

int FBALoadString(int id, char *buffer, int maxsize)
{
	int ret = 0;
	return ret;
}

char* FBALoadStringEx(int id, bool translate)
{
	static char loadstr[2048] = "";
	return loadstr;
}

// ----------------------------------------------------------------------------

int FBALocaliseInstance()
{
	return 0;
}

void FBALocaliseExit()
{
	bLanguageActive = false;
}

int FBALocaliseInit(char* language)
{
	return 0;
}

// ---------------------------------------------------------------------------
// Dialog box to load language

static void localiseMakeOfn()
{
}

int FBALocaliseLoad()
{
	return 0;
}
