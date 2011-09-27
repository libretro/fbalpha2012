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
extern char * pBurnCartridgeFileExtFilter;
extern int (*pBurnInsertCartridgeCallBack)(const char *);

int BurnCartridgeGameInit(char * pExtFilter, int (*pInsertCartridgeCallBack)(const char *));
int BurnCartridgeGameExit();

#ifdef __cplusplus
 } // End of extern "C"
#endif

#endif //BURN_CARTRIDGE_H
