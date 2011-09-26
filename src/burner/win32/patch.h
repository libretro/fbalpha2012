#ifndef PATCH_H
#define PATCH_H

#ifndef NO_IPS

// new IPS support, added by regret

struct ui_lang_info_t
{
	const char* name;
	const char* shortname;
	const char* description;
	int         codepage;
	int         numchars;
};

extern bool bDoPatch;
extern int nPatchLang;		// description language
extern int bEnforceDep;		// enforce dependency

int getPatchCount();
int getPatchFilename(TCHAR* pszName, TCHAR* pszRet, const int& patch_index);
int getPatchPrevName(TCHAR* pszName, TCHAR* pszGameName, TCHAR* pszPatchName);
TCHAR* getPatchDesc(const TCHAR* pszGameName, const TCHAR* pszPatchName);
void patchExit();

// IPS setting dialog
int patchSettingCreate(HWND);
int patchSettingInit();
void patchSettingExit();
void loadActivePatches();
int getNumActivePatches();
void __cdecl applyPatches(UINT8* base, char* rom_name, int len);

#endif

#endif
