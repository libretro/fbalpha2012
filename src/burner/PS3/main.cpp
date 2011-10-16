 // FB Alpha - Emulator for MC68000/Z80 based arcade games
//            Refer to the "license.txt" file for more info

// Main module

#include <string.h>
#include <cell/sysmodule.h>
#include <sysutil/sysutil_sysparam.h>
#include <sysutil/sysutil_msgdialog.h>
#include <sys/paths.h>
#include "burner.h"
#include "version.h"
#include "menu.h"
#include "vid_psgl.h"
#include "cellframework2/input/pad_input.h"
#include "cellframework2/audio/stream.h"
#include "audio_driver.h"
#include "string.h"
#ifdef CELL_DEBUG_CONSOLE
#include <cell/control_console.h>
#endif
#include "config_file.h"

#define NORMAL_CONTROLS 0
#define ANALOG_CONTROLS 1

char		szAppBurnVer[16] = "";
char		szSVNVer[16] = "";
char		szSVNDate[30] = "";
bool		DoReset = false;
int		ArcadeJoystick = 0;
int		exitGame = 0;
uint32_t	bBurnFirstStartup;
int		GameStatus = MENU;
int		nAppVirtualFps = 6000;
int		is_running = 0;
bool		bShowFPS = false;
int		custom_aspect_ratio_mode = 0;
cell_audio_handle_t audio_handle;
const struct cell_audio_driver *driver;
static int	controls;

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

	if (bBurnFirstStartup)
		configAppSaveXml();	// Create initial config file

	configAppLoadXml();		// Load config for the application

	BurnLibInit();			// Init the Burn library

	getAllRomsetInfo();		// Build the ROM information

	BurnExtLoadOneRom = archiveLoadOneFile; 
	InitRomList();
	InitInputList();
	InitDipList();

	// get the last filter
	CurrentFilter = nLastFilter;

	BuildRomList();	
	audio_new();
	return 0;
}

extern unsigned int nPrevGame;
extern void doStretch();
extern void StretchMenu();

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

#ifdef MULTIMAN_SUPPORT
	if(argc > 1)
	{
		const char * current_game = strrchr(strdup(argv[1]), '/');
		directLoadGame(strdup(current_game));
		GameStatus = EMULATING_INIT;	
		nPrevGame = 0;
	}
#endif

	do{
		switch (GameStatus)
		{
			case MENU:	
				psglClearUI();
				RomMenu();
				FrameMove();
#ifdef CELL_DEBUG_CONSOLE
				cellConsolePoll();
#endif
				psglRenderUI();
				break;
			case CONFIG_MENU:
				psglClearUI();
				ConfigMenu();
				ConfigFrameMove();
#ifdef CELL_DEBUG_CONSOLE
				cellConsolePoll();
#endif
				psglRenderUI();
				break;
			case PAUSE:
				psglClearUI();
				CalculateViewports();
				psglRenderAlpha();
				InGameMenu();
				InGameFrameMove();		
#ifdef CELL_DEBUG_CONSOLE
				cellConsolePoll();
#endif
				psglRenderUI();
				break;
			case INPUT_MENU:						
				psglClearUI();
				CalculateViewports();
				psglRenderAlpha();
				InputMenu();
				InputFrameMove();			
#ifdef CELL_DEBUG_CONSOLE
				cellConsolePoll();
#endif
				psglRenderUI();
				break;
			case DIP_MENU:						
				psglClearUI();
				CalculateViewports();
				psglRenderAlpha();
				DipMenu();
				DipFrameMove();			
				psglRenderUI();
				break;
			case SCREEN_RESIZE:
				psglClearUI();			
				CalculateViewports();
				psglRenderAlpha();
				doStretch();
				StretchMenu();
#ifdef CELL_DEBUG_CONSOLE
				cellConsolePoll();
#endif
				psglRenderUI();
				custom_aspect_ratio_mode = 1;
				nVidScrnAspectMode = ASPECT_RATIO_CUSTOM;
				break;
			case EMULATING_INIT:
			{
				struct GameInp * pgi;
				uint32_t i = 0;
				pBurnSoundOut = pAudNextSound;
				nBurnBpp = 2;
				GameStatus = EMULATING;
				for(i = 0, pgi = GameInp; i < nGameInpCount; i++, pgi++)
				{
					if(pgi->nType == BIT_ANALOG_REL)
					{
						controls = ANALOG_CONTROLS;
						break;
					}
					else
						controls = NORMAL_CONTROLS;
				}
			}
				break;
			case EMULATING:
				if(!is_running)
					GameStatus = PAUSE;
				CalculateViewports();
				uint32_t audio_samples = FRAMES_TO_SAMPLES(nAudSegLen);
				if(controls)
				{
					do{
						audio_check(audio_samples);
						nCurrentFrame++;
						VidFrame();
						psglRenderUI();
						InputMake_Analog();
#ifdef CELL_DEBUG_CONSOLE
						cellConsolePoll();
#endif
					}while(is_running);
				}
				else
				{
					do{
						audio_check(audio_samples);
						nCurrentFrame++;
						VidFrame();
						psglRenderUI();
						InputMake();
#ifdef CELL_DEBUG_CONSOLE
						cellConsolePoll();
#endif
					}while(is_running);
				}
				break;
		}
	}while(!exitGame);

	audio_stop();			// Stop sound if it was playing
	BurnerDrvExit();		// Make sure any game driver is exited
	mediaExit();			// Exit media

	BurnLibExit();			// Exit the Burn library

	freeAuditState();		// Free audit state 
	auditCleanup();			// Free audit info

	cell_pad_input_deinit();

	configAppSaveXml();		// Save config for the application

	cellSysmoduleUnloadModule(CELL_SYSMODULE_FS);
	cellSysmoduleUnloadModule(CELL_SYSMODULE_IO);     
	cellSysmoduleUnloadModule(CELL_SYSMODULE_RTC);
	cellSysutilUnregisterCallback(0);

	sys_process_exit(0);
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

void UpdateConsole(const char * text)
{  	
	 psglClearUI();
	 cellDbgFontPuts(0.38f, 0.5f, 0.75f, 0xFFFFFFFF, text);
	 cellDbgFontDraw();
	 psglRenderUI();
}

void UpdateConsoleXY(const char * text, float X, float Y)
{  	
	 psglClearUI();
	 cellDbgFontPuts(X, Y, 0.75f, 0xFFFFFFFF, text);
	 cellDbgFontDraw();
	 psglRenderUI();
}
