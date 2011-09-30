 // FB Alpha - Emulator for MC68000/Z80 based arcade games
//            Refer to the "license.txt" file for more info

// Main module

#include "main.h"
#include "burner.h"
#include "version.h"
#include "menu.h"
#include "../../interface/libsnes-new/vid_psgl.h"
#include "../../interface/libsnes-new/audio_driver.h"
#include "string.h"

bool IsCurrentlyInGame = false; 
char szAppBurnVer[16] = "";
char szSVNVer[16] = "";
char szSVNDate[30] = "";

bool bCmdOptUsed = 0;
bool bAlwaysProcessKey = false;
bool DoReset = false;

int ArcadeJoystick = 0;
int exitGame = 0;

// Used for the load/save dialog in commdlg.h (savestates)
char szChoice[MAX_PATH] = "";
 
static int AppInit()
{
	if (nIniVersion < nBurnVer)
		configAppSaveXml();	// Create initial config file

	BurnLibInit();			// Init the Burn library


	getAllRomsetInfo();		// Build the ROM information
	nVidFullscreen = 1;

	BurnExtLoadOneRom = archiveLoadOneFile; 
	InitRomList();
	InitInputList();
	InitDipList();
	BuildRomList();	
	audio_new();
	return 0;
}

static int AppExit()
{
	BurnerDrvExit();				// Make sure any game driver is exited
	mediaExit();					// Exit media
	BurnLibExit();					// Exit the Burn library

	freeAuditState();				// Free audit state 
	auditCleanup();					// Free audit info
 

	return 0;
}

// Main program entry point
int  main(int argc, char **argv)
{
	// Make version string
	if (nBurnVer & 0xFF)
	{
		// private version (alpha)
		sprintf(szAppBurnVer, "%x.%x.%x.%02x", nBurnVer >> 20, (nBurnVer >> 16) & 0x0F, (nBurnVer >> 8) & 0xFF, nBurnVer & 0xFF);
		sprintf(szSVNVer, "%s", SVN_VERSION);
		sprintf(szSVNDate, "%s", SVN_DATE);
	}
	else
	{
		// public version
		sprintf(szAppBurnVer, "%x.%x.%x", nBurnVer >> 20, (nBurnVer >> 16) & 0x0F, (nBurnVer >> 8) & 0xFF);
		sprintf(szSVNVer, "%s", SVN_VERSION);
		sprintf(szSVNDate, "%s", SVN_DATE);
	}

	configAppLoadXml();				// Load config for the application

	createNeedDir();				// Make sure there are roms and cfg subdirectories

	AppInit();

	//psglInitGL();					//FIXME: Your own video init code if needed

	mediaInit();
	RunMessageLoop(argc, argv); 

	AppExit();					// Exit the application

	configAppSaveXml();				// Save config for the application

	cellSysmoduleUnloadModule(CELL_SYSMODULE_FS);
	cellSysmoduleUnloadModule(CELL_SYSMODULE_IO);     
	cellSysmoduleUnloadModule(CELL_SYSMODULE_RTC);
	cellSysutilUnregisterCallback(0);

	exit(0);
}

int ProgressUpdateBurner(double dProgress, const char * pszText, bool bAbs)
{ 	 

	 psglClearUI();

	 //cellDbgFontPrintf(0.38f, 0.5f, 0.75f, 0xFFFFFFFF, "%s",  pszText);	 
	 //cellDbgFontDraw();
	 
	 psglRenderUI();

	return 0;
}

void UpdateConsole(char *text)
{  	
	 psglClearUI();
	 //cellDbgFontPuts(0.38f, 0.5f, 0.75f, 0xFFFFFFFF, text);
	 //cellDbgFontDraw();
	 psglRenderUI();
}

void UpdateConsoleXY(char *text, float X, float Y)
{  	
	 psglClearUI();
	 //cellDbgFontPuts(X, Y, 0.75f, 0xFFFFFFFF, text);
	 //cellDbgFontDraw();
	 psglRenderUI();
}
