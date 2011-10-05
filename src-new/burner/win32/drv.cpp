// Driver Init module
#include "burner.h"

int bDrvOkay = 0;						// 1 if the Driver has been initted okay, and it's okay to use the BurnDrv functions

TCHAR szAppRomPaths[DIRS_MAX][MAX_PATH] = { { _T("") }, { _T("") }, { _T("") }, { _T("") }, { _T("") }, { _T("") }, { _T("") }, { _T("roms\\") } };

static bool bSaveRAM = false;

static int DoLibInit()					// Do Init of Burn library driver
{
	int nRet = 0;

	BzipOpen(FALSE);

	// If there is a problem with the romset, report it
	switch (BzipStatus()) {
		case BZIP_STATUS_BADDATA: {
			FBAPopupDisplay(PUF_TYPE_WARNING);
			break;
		}
		case BZIP_STATUS_ERROR: {
			FBAPopupDisplay(PUF_TYPE_ERROR);

#if 0 || !defined FBA_DEBUG
			// Don't even bother trying to start the game if we know it won't work
			BzipClose();
			return 1;
#endif

			break;
		}
		default: {

#if 0 && defined FBA_DEBUG
			FBAPopupDisplay(PUF_TYPE_INFO);
#else
			FBAPopupDisplay(PUF_TYPE_INFO | PUF_TYPE_LOGONLY);
#endif

		}
	}

	ProgressCreate();

	if (bJukeboxInUse) {
		nRet = BurnJukeboxInit();
	} else {
		nRet = BurnDrvInit();
	}

	BzipClose();

	ProgressDestroy();

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

	BzipOpen(FALSE);

	if ((nRet = BurnExtLoadRom(Dest, pnWrote, i)) != 0) {
		char* pszFilename;

		BurnDrvGetRomName(&pszFilename, i, 0);
		FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_ERR_LOAD_REQUEST), pszFilename, BurnDrvGetText(DRV_NAME));
		FBAPopupDisplay(PUF_TYPE_ERROR);
	}

	BzipClose();

	BurnExtLoadRom = DrvLoadRom;

	ScrnTitle();

	return nRet;
}

int DrvInit(int nDrvNum, bool bRestore)
{
	int nStatus;
	
	DrvExit();						// Make sure exitted
	MediaExit();

	nBurnDrvSelect = nDrvNum;		// Set the driver number

	MediaInit();

	// Define nMaxPlayers early; GameInpInit() needs it (normally defined in DoLibInit()).
	nMaxPlayers = BurnDrvGetMaxPlayers();
	GameInpInit();					// Init game input

	if(ConfigGameLoad(true)) {
		ConfigGameLoadHardwareDefaults();
	}	
	InputMake(true);
	GameInpDefault();

	if (kNetGame) {
		nBurnCPUSpeedAdjust = 0x0100;
	}

	nStatus = DoLibInit();			// Init the Burn library's driver
	if (nStatus) {
		if (nStatus & 2) {
			BurnDrvExit();			// Exit the driver

			ScrnTitle();

			FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_ERR_BURN_INIT), BurnDrvGetText(DRV_FULLNAME));
			FBAPopupDisplay(PUF_TYPE_WARNING);
		}

		POST_INITIALISE_MESSAGE;
		return 1;
	}

	BurnExtLoadRom = DrvLoadRom;

	bDrvOkay = 1;						// Okay to use all BurnDrv functions

	if (BurnDrvGetFlags() & BDF_ORIENTATION_VERTICAL) {
		nScreenSize = nScreenSizeVer;
		bVidArcaderes = bVidArcaderesVer;
		nVidWidth	= nVidVerWidth;
		nVidHeight	= nVidVerHeight;
	} else {
		nScreenSize = nScreenSizeHor;
		bVidArcaderes = bVidArcaderesHor;
		nVidWidth	= nVidHorWidth;
		nVidHeight	= nVidHorHeight;
	}

	UpdatePlayCounter(nBurnDrvSelect);	// Update favorites play count

	bSaveRAM = false;
	if (kNetGame) {
		KailleraInitInput();
		KailleraGetInput();
	} else {
		if (bRestore) {
			StatedAuto(0);
			bSaveRAM = true;

			ConfigCheatLoad();
		}
	}

	nBurnLayer = 0xFF;				// show all layers
	
	// Reset the speed throttling code, so we don't 'jump' after the load
	RunReset();

	VidExit();
	POST_INITIALISE_MESSAGE;

	return 0;
}

int DrvInitCallback()
{
	return DrvInit(nBurnDrvSelect, false);
}

int DrvExit()
{
	if (bDrvOkay) {
		StopReplay();

		VidExit();

		InvalidateRect(hScrnWnd, NULL, 1);
		UpdateWindow(hScrnWnd);			// Blank screen window

		DestroyWindow(hInpdDlg);		// Make sure the Input Dialog is exited
		DestroyWindow(hInpDIPSWDlg);	// Make sure the DipSwitch Dialog is exited
		DestroyWindow(hInpCheatDlg);	// Make sure the Cheat Dialog is exited

		if (nBurnDrvSelect < nBurnDrvCount) {
			
			if(!bJukeboxInUse) 
			{
				MemCardEject();				// Eject memory card if present

				if (bSaveRAM) {
					StatedAuto(1);			// Save NV (or full) RAM
					bSaveRAM = false;
				}

				ConfigGameSave(bSaveInputs);
			}

			GameInpExit();				// Exit game input
			
			if (bJukeboxInUse) {
				BurnJukeboxExit();
			} else {
				BurnDrvExit();				// Exit the driver
			}
		}
	}

	BurnExtLoadRom = NULL;

	bDrvOkay = 0;					// Stop using the BurnDrv functions

	bRunPause = 0;					// Don't pause when exitted

	if (bAudOkay) {
		// Write silence into the sound buffer on exit, and for drivers which don't use pBurnSoundOut
		memset(nAudNextSound, 0, nAudSegLen << 2);
	}

	nBurnDrvSelect = ~0U;			// no driver selected

	return 0;
}
