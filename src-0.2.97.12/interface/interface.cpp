#include "burner.h"

#define INT_INFO_STRINGS (8)

int IntInfoFree(InterfaceInfo* pInfo)
{
	if (pInfo->ppszInterfaceSettings) {
		for (int i = 0; i < INT_INFO_STRINGS; i++) {
			free(pInfo->ppszInterfaceSettings[i]);
		}
	}
	free(pInfo->ppszInterfaceSettings);

	if (pInfo->ppszModuleSettings) {
		for (int i = 0; i < INT_INFO_STRINGS; i++) {
			free(pInfo->ppszModuleSettings[i]);
		}
	}
	free(pInfo->ppszModuleSettings);

	memset(pInfo, 0, sizeof(InterfaceInfo));

	return 0;
}

int IntInfoInit(InterfaceInfo* pInfo)
{
	IntInfoFree(pInfo);

	pInfo->ppszInterfaceSettings = (TCHAR**)malloc((INT_INFO_STRINGS + 1) * sizeof(TCHAR*));
	if (pInfo->ppszInterfaceSettings == NULL) {
		return 1;
	}
	memset(pInfo->ppszInterfaceSettings, 0, (INT_INFO_STRINGS + 1) * sizeof(TCHAR*));

	pInfo->ppszModuleSettings = (TCHAR**)malloc((INT_INFO_STRINGS + 1) * sizeof(TCHAR*));
	if (pInfo->ppszModuleSettings == NULL) {
		return 1;
	}
	memset(pInfo->ppszModuleSettings, 0, (INT_INFO_STRINGS + 1) * sizeof(TCHAR*));

	return 0;
}

int IntInfoAddStringInterface(InterfaceInfo* pInfo, TCHAR* szString)
{
	int i;

	for (i = 0; pInfo->ppszInterfaceSettings[i] && i < INT_INFO_STRINGS; i++) { }

	if (i >= INT_INFO_STRINGS) {
		return 1;
	}

	pInfo->ppszInterfaceSettings[i] = (TCHAR*)malloc(MAX_PATH * sizeof(TCHAR));
	if (pInfo->ppszInterfaceSettings[i] == NULL) {
		return 1;
	}

	_tcsncpy(pInfo->ppszInterfaceSettings[i], szString, MAX_PATH);

	return 0;
}

int IntInfoAddStringModule(InterfaceInfo* pInfo, TCHAR* szString)
{
	int i;

	for (i = 0; pInfo->ppszModuleSettings[i] && i < INT_INFO_STRINGS; i++) { }

	if (i >= INT_INFO_STRINGS) {
		return 1;
	}

	pInfo->ppszModuleSettings[i] = (TCHAR*)malloc(MAX_PATH * sizeof(TCHAR));
	if (pInfo->ppszModuleSettings[i] == NULL) {
		return 1;
	}

	_tcsncpy(pInfo->ppszModuleSettings[i], szString, MAX_PATH);

	return 0;
}

