/*
 * Cartridge Support interface
 */
#include <stdio.h>
#include "cartridge.h"

char * pBurnCartridgeFileExtFilter = NULL;
int (*pBurnInsertCartridgeCallBack)(const char *) = NULL;

unsigned int BurnCartridgeGameLoaded = CARTRIDGE_NO_INIT;

int BurnCartridgeGameInit(char * pExtFilter, int (*pInsertCartridgeCallBack)(const char *))
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
