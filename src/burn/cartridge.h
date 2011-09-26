#ifndef BURN_CARTRIDGE_H
#define BURN_CARTRIDGE_H

#include "fbatypes.h"

#ifdef __cplusplus
 extern "C" {
#endif

#define CARTRIDGE_NO_INIT	0
#define CARTRIDGE_EMPTY		1
#define CARTRIDGE_BIOS		2
#define CARTRIDGE_GAME		3

extern unsigned int BurnCartridgeGameLoaded;
extern TCHAR* pBurnCartridgeFileExtFilter;
extern int (*pBurnInsertCartridgeCallBack)(const TCHAR*);

int BurnCartridgeGameInit(TCHAR* pExtFilter, int (*pInsertCartridgeCallBack)(const TCHAR*));
int BurnCartridgeGameExit();

#ifdef __cplusplus
 } // End of extern "C"
#endif

#endif //BURN_CARTRIDGE_H
