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

 

int FBALoadString(UINT id, LPTSTR buffer, int maxsize)
{
	int ret = 0;

 
	return ret;
}

TCHAR* FBALoadStringEx(UINT id, bool translate)
{
	static TCHAR loadstr[2048] = _T("");
 

	return loadstr;
}

// ----------------------------------------------------------------------------

HMODULE FBALocaliseInstance()
{
	return 0;
}

void FBALocaliseExit()
{
 
	bLanguageActive = false;
}

int FBALocaliseInit(TCHAR* lanaugae)
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
