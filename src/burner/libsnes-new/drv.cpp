// Driver Init module
#include "burner.h"
#include "tracklst.h"
#ifndef NO_IPS
//#include "patch.h"
#endif
#ifndef NO_COMBO
#include "combo.h"
#endif
#include "../../interface/libsnes-new/audio_driver.h"

int bDrvOkay = 0; // 1 if the Driver has been inited okay, and it's okay to use the BurnDrv functions
static bool bSaveRAM = false;

static int DoLibInit() // Do Init of Burn library driver
{
	int nRet = 0;

	BArchiveOpen(false);

	// If there is a problem with the romset, report it
	switch (BArchiveStatus())
	{
		case BARC_STATUS_BADDATA:
			FBAPopupDisplay(PUF_TYPE_WARNING);
			BArchiveClose();
			return 1;
			break;
		case BARC_STATUS_ERROR:
			FBAPopupDisplay(PUF_TYPE_ERROR);
			break;
	}

	nRet = BurnDrvInit();
	BArchiveClose();

	if (nRet)
		return 3;
	else
		return 0;
}

// Catch calls to BurnLoadRom() once the emulation has started;
// Intialise the zip module before forwarding the call, and exit cleanly.
static int __cdecl DrvLoadRom(unsigned char* Dest, int* pnWrote, int i)
{
	int nRet;

	BArchiveOpen(false);

	if ((nRet = BurnExtLoadRom(Dest, pnWrote, i)) != 0)
	{
		char* pszFilename;

		BurnDrvGetRomName(&pszFilename, i, 0);
	}

	BArchiveClose();
	BurnExtLoadRom = DrvLoadRom;

	return nRet;
}

//#define NEED_MEDIA_REINIT
// no need to reinit media when init a driver, modified by regret
int BurnerDrvInit(int nDrvNum, bool bRestore)
{
	BurnerDrvExit(); // Make sure exited

#ifdef NEED_MEDIA_REINIT
	mediaExit();
#endif

	nBurnDrvSelect = nDrvNum; // Set the driver number

#ifdef NEED_MEDIA_REINIT
	mediaInit();
#endif
	mediaInit();
	// Define nMaxPlayers early; GameInpInit() needs it (normally defined in DoLibInit()).
	nMaxPlayers = BurnDrvGetMaxPlayers();

	GameInpInit();					// Init game input

	if (ConfigGameLoad(true))
		loadDefaultInput();			// load default input mapping

	InputMake();
	GameInpDefault();


	// set functions
	BurnReinitScrn = simpleReinitScrn;

#ifndef NO_IPS
	//	bDoPatch = true; // play with ips
	//	loadActivePatches();
	//	BurnApplyPatch = applyPatches;
#endif

	int nStatus = DoLibInit();		// Init the Burn library's driver
	if (nStatus)
	{
		if (nStatus & 2)
			BurnDrvExit();			// Exit the driver

		return 1;
	}

	// ==> for changing sound track
	//parseTracklist();
	// <==

	BurnExtLoadRom = DrvLoadRom;

	bDrvOkay = 1;					// Okay to use all BurnDrv functions

	bSaveRAM = false;
	if (bRestore)
	{
		StatedAuto(0);
		bSaveRAM = true;
	}

	// Reset the speed throttling code, so we don't 'jump' after the load
	return 0;
}

int DrvInitCallback()
{
	return BurnerDrvInit(nBurnDrvSelect, false);
}

int BurnerDrvExit()
{
	if (bDrvOkay)
	{
		VidExit();
		//jukeDestroy();

		if (nBurnDrvSelect < nBurnDrvCount)
		{
			//MemCardEject(); // Eject memory card if present

			if (bSaveRAM)
			{
				StatedAuto(1); // Save NV (or full) RAM
				bSaveRAM = false;
			}

			ConfigGameSave(true); // save game config

			GameInpExit(); // Exit game input
			BurnDrvExit(); // Exit the driver
		}
	}

	BurnExtLoadRom = NULL;

	bDrvOkay = 0; // Stop using the BurnDrv functions

	bRunPause = 0; // Don't pause when exitted

	if (bAudOkay)
	{
		audio_blank();	// Write silence into the sound buffer on exit, and for drivers which don't use pBurnSoundOut
	}

	nBurnDrvSelect = ~0U; // no driver selected
	nBurnLayer = 0xFF; // show all layers

	return 0;
}
