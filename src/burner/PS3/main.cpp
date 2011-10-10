 // FB Alpha - Emulator for MC68000/Z80 based arcade games
//            Refer to the "license.txt" file for more info

// Main module

#include "main.h"
#include "burner.h"
#include "version.h"
#include "menu.h"
#include "vid_psgl.h"
#include "cellframework2/input/pad_input.h"
#include "audio_driver.h"
#include "string.h"
#ifdef CELL_DEBUG_CONSOLE
#include <cell/control_console.h>
#endif
#include "config_file.h"

#define init_setting_uint(charstring, setting, defaultvalue) \
	if(!(config_get_uint(currentconfig, charstring, &setting))) \
		setting = defaultvalue; 

char	szAppBurnVer[16] = "";
char	szSVNVer[16] = "";
char	szSVNDate[30] = "";
bool	DoReset = false;
int	ArcadeJoystick = 0;
int	exitGame = 0;
uint32_t bBurnFirstStartup;

extern void reset_frame_counter();
 
SYS_PROCESS_PARAM(1001, 0x80000);

void sysutil_exit_callback (uint64_t status, uint64_t param, void *userdata)
{
	(void) param;
	(void) userdata;

	switch (status)
	{
		case CELL_SYSUTIL_REQUEST_EXITGAME:  
			is_running = 0;
			exitGame = 1;
			configAppSaveXml();
			sys_process_exit(0);
			break;
		case CELL_SYSUTIL_DRAWING_BEGIN:
		case CELL_SYSUTIL_DRAWING_END:
			break;
	}
}

static int AppInit()
{
	if(!fileExists(SYS_CONFIG_FILE))
	{
		FILE * f;
		f = fopen(SYS_CONFIG_FILE, "w");
		fclose(f);
	}

	config_file_t * currentconfig = config_file_new(SYS_CONFIG_FILE);

	init_setting_uint("firststartup", bBurnFirstStartup, 1);
	printf("AppInit: bBurnFirstStartup is: %d\n", bBurnFirstStartup);

	if (bBurnFirstStartup)
		configAppSaveXml();	// Create initial config file

	configAppLoadXml();		// Load config for the application

	BurnLibInit();			// Init the Burn library

	getAllRomsetInfo();		// Build the ROM information

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
	// One raw SPU used for PSGL - two raw SPUs and one SPU thread for the application
	sys_spu_initialize(4, 3);

	cellSysmoduleLoadModule(CELL_SYSMODULE_FS);
	cellSysmoduleLoadModule(CELL_SYSMODULE_IO);     
	cellSysmoduleLoadModule(CELL_SYSMODULE_RTC);

	cellSysutilRegisterCallback(0, sysutil_exit_callback, NULL);

#ifdef CELL_DEBUG_CONSOLE
	cellConsoleInit();
	cellConsoleNetworkInitialize();
	cellConsoleNetworkServerInit(-1);
	cellConsoleScreenShotPluginInit();
#endif

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

	cell_pad_input_init();

	createNeedDir();		// Make sure there are roms and cfg subdirectories

	AppInit();


	psglInitGL();
	dbgFontInit();
	reset_frame_counter();

	mediaInit();
	RunMessageLoop(argc, argv); 

	AppExit();			// Exit the application

	cell_pad_input_deinit();

	configAppSaveXml();		// Save config for the application

	cellSysmoduleUnloadModule(CELL_SYSMODULE_FS);
	cellSysmoduleUnloadModule(CELL_SYSMODULE_IO);     
	cellSysmoduleUnloadModule(CELL_SYSMODULE_RTC);
	cellSysutilUnregisterCallback(0);

	exit(0);
}

int ProgressUpdateBurner(const char * pszText)
{ 	 
#ifdef CELL_DEBUG_MEMORY
	sys_memory_info_t mem_info;
	sys_memory_get_user_memory_size(&mem_info);
#endif

	psglClearUI();

	cellDbgFontPrintf(0.38f, 0.5f, 0.75f, 0xFFFFFFFF, "%s",  pszText);	 
	cellDbgFontDraw();
#ifdef CELL_DEBUG_MEMORY
	cellDbgFontPrintf(0.75f, 0.90f + 0.025f, 0.75f, 0xFFFF7F7f ,"%ld free memory",mem_info.available_user_memory );  
	cellDbgFontDraw();
	cellDbgFontPrintf(0.75f, 0.92f + 0.025f, 0.75f, 0xFFFF7F7f ,"%ld total memory",mem_info.total_user_memory );     
	cellDbgFontDraw();
#endif

	psglRenderUI();

	return 0;
}

void UpdateConsole(char *text)
{  	
	 psglClearUI();
	 cellDbgFontPuts(0.38f, 0.5f, 0.75f, 0xFFFFFFFF, text);
	 cellDbgFontDraw();
	 psglRenderUI();
}

void UpdateConsoleXY(char *text, float X, float Y)
{  	
	 psglClearUI();
	 cellDbgFontPuts(X, Y, 0.75f, 0xFFFFFFFF, text);
	 cellDbgFontDraw();
	 psglRenderUI();
}
