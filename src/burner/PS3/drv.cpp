// Driver Init module
#include "burner.h"
#include "tracklst.h"
#ifndef NO_IPS
//#include "patch.h"
#endif
#ifndef NO_AUTOFIRE
//#include "autofire.h"
#endif
#ifndef NO_COMBO
#include "combo.h"
#endif

int bDrvOkay = 0;						// 1 if the Driver has been initted okay, and it's okay to use the BurnDrv functions

static bool bSaveRAM = false;

static int DoLibInit()					// Do Init of Burn library driver
{
	int nRet = 0;

	BArchiveOpen(false);

	// If there is a problem with the romset, report it
	switch (BArchiveStatus()) {
		case BARC_STATUS_BADDATA: {
			FBAPopupDisplay(PUF_TYPE_WARNING);
			BArchiveClose();
			return 1;
			break;
		}
		case BARC_STATUS_ERROR: {
			FBAPopupDisplay(PUF_TYPE_ERROR);

#if 0 || !defined FBA_DEBUG
			// Don't even bother trying to start the game if we know it won't work
			BArchiveClose();
			return 1;
#endif

			break;
		}
		default: {

#if 0 && defined FBA_DEBUG
			//FBAPopupDisplay(PUF_TYPE_INFO);
#else
			//FBAPopupDisplay(PUF_TYPE_INFO | PUF_TYPE_LOGONLY);
#endif

		}
	}

	//ProgressCreate();

	nRet = BurnDrvInit();

	BArchiveClose();

	//ProgressDestroy();

	if (nRet) {
		return 3;
	} else {
		return 0;
	}
}

// Catch calls to BurnLoadRom() once the emulation has started;
// Intialise the zip module before forwarding the call, and exit cleanly.
static int __cdecl DrvLoadRom(unsigned char* Dest, int* pnWrote, int i)
{
	int nRet;

	BArchiveOpen(false);

	if ((nRet = BurnExtLoadRom(Dest, pnWrote, i)) != 0) {
		char* pszFilename;

		BurnDrvGetRomName(&pszFilename, i, 0);
 	}

	BArchiveClose();

	BurnExtLoadRom = DrvLoadRom;

	//scrnTitle();

	return nRet;
}

//#define NEED_MEDIA_REINIT
// no need to reinit media when init a driver, modified by regret
int BurnerDrvInit(int nDrvNum, bool bRestore)
{
	BurnerDrvExit();				// Make sure exitted

#ifdef NEED_MEDIA_REINIT
	mediaExit();
#endif

	nBurnDrvSelect = nDrvNum;		// Set the driver number

#ifdef NEED_MEDIA_REINIT
	mediaInit();
#endif
	//audio.init();
	mediaInit();
	// Define nMaxPlayers early; GameInpInit() needs it (normally defined in DoLibInit()).
	nMaxPlayers = BurnDrvGetMaxPlayers();

	GameInpInit();					// Init game input
	if (ConfigGameLoad(true)) {
		loadDefaultInput();			// load default input mapping
	}
	InputMake(true);
	GameInpDefault();

 
	// set functions
	BurnReinitScrn = scrnReinit;

#ifndef NO_IPS
//	bDoPatch = true; // play with ips
//	loadActivePatches();
//	BurnApplyPatch = applyPatches;
#endif

#ifndef NO_COMBO
	BurnInitCombo = ConstructComboList;
	BurnProcessCombo = ProcessCombo;
#endif

#ifndef NO_AUTOFIRE
//	BurnInitAutofire = initAutofire;
//	BurnDoAutofire = doAutofire;
#endif

	int nStatus = DoLibInit();		// Init the Burn library's driver
	if (nStatus) {
		if (nStatus & 2) {
			BurnDrvExit();			// Exit the driver

			//scrnTitle();
 		}

		return 1;
	}

/*	if (nInputMacroEnabled) {
		GameInpExit();
		GameInpInit();				// ReInit game input
		if (ConfigGameLoad(true)) {
			loadDefaultInput();		// load default input mapping
		}
		InputMake(true);
		GameInpDefault();
	}*/

	// ==> for changing sound track
	//parseTracklist();
	// <==

	BurnExtLoadRom = DrvLoadRom;

	bDrvOkay = 1;					// Okay to use all BurnDrv functions

	bSaveRAM = false;
 		if (bRestore) {
			StatedAuto(0);
			bSaveRAM = true;

			//configCheatLoad();
		}
 

	// Reset the speed throttling code, so we don't 'jump' after the load
	RunReset();
 
	return 0;
}

int DrvInitCallback()
{
	return BurnerDrvInit(nBurnDrvSelect, false);
}

int BurnerDrvExit()
{
	if (bDrvOkay) {
		//StopReplay();

		VidExit();

 
		//dialogClear();
#ifndef NO_CHEATSEARCH
		cheatSearchDestroy();
#endif
		//jukeDestroy();

		if (nBurnDrvSelect < nBurnDrvCount) {
			//MemCardEject();				// Eject memory card if present

			if (bSaveRAM) {
				StatedAuto(1);			// Save NV (or full) RAM
				bSaveRAM = false;
			}

			ConfigGameSave(true);		// save game config

			GameInpExit();				// Exit game input
			BurnDrvExit();				// Exit the driver
		}
	}

	BurnExtLoadRom = NULL;

	bDrvOkay = 0;					// Stop using the BurnDrv functions

	bRunPause = 0;					// Don't pause when exitted

	if (bAudOkay) {
		// Write silence into the sound buffer on exit, and for drivers which don't use pBurnSoundOut
		AudWriteSlience();
	}

	nBurnDrvSelect = ~0U;			// no driver selected
	nBurnLayer = 0xFF;				// show all layers

	return 0;
}
