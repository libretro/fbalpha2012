/*
 * Cartridge Support interface
 */
#include <stdio.h>
#include "cartridge.h"

TCHAR* pBurnCartridgeFileExtFilter = NULL;
int (*pBurnInsertCartridgeCallBack)(const TCHAR *) = NULL;

unsigned int BurnCartridgeGameLoaded = CARTRIDGE_NO_INIT;

int BurnCartridgeGameInit(TCHAR* pExtFilter, int (*pInsertCartridgeCallBack)(const TCHAR *))
{
	pBurnCartridgeFileExtFilter = pExtFilter;
	pBurnInsertCartridgeCallBack = pInsertCartridgeCallBack;

	// Driver setup BurnCartridgeGameLoaded

	return 0;
}

int BurnCartridgeGameExit()
{
	BurnCartridgeGameLoaded = CARTRIDGE_NO_INIT;
	pBurnCartridgeFileExtFilter = NULL;
	pBurnInsertCartridgeCallBack = NULL;
	return 0;
}
