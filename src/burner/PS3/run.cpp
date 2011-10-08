// Run module
#include <sys/sys_time.h>
#include <cell/rtc.h>
#include <PSGL/psgl.h>
#include "burner.h" 
#include "menu.h" 
#include "vid_psgl.h"
#ifdef CELL_DEBUG_CONSOLE
#include <cell/control_console.h>
#endif
#include "cellframework2/audio/stream.h"
#include "../../interface/PS3/audio_driver.h"

extern bool exitGame;
extern void doStretch();
extern void StretchMenu();

int GameStatus = MENU;
int nAppVirtualFps = 6000;				// App fps * 100
int is_running = 0;
bool bShowFPS = false;
int custom_aspect_ratio_mode = 0;
cell_audio_handle_t audio_handle;
const struct cell_audio_driver *driver;

// The main message loop
int RunMessageLoop(int argc, char **argv)
{
	extern unsigned int nPrevGame;

	audio_play();

	// get the last filter
	CurrentFilter = nLastFilter;	 
	BuildRomList();


#ifdef MULTIMAN_SUPPORT
	if(argc > 1)
	{
		const char * current_game = strrchr(strdup(argv[1]), '/');
		directLoadGame(strdup(current_game));
		mediaInit();
		GameStatus = EMULATING;	
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
				psglRenderPaused();
				InGameMenu();
				InGameFrameMove();		
#ifdef CELL_DEBUG_CONSOLE
				cellConsolePoll();
#endif
				psglRenderUI();
				break;
			case INPUT_MENU:						
				psglClearUI();
				psglRenderPaused();
				InputMenu();
				InputFrameMove();			
#ifdef CELL_DEBUG_CONSOLE
				cellConsolePoll();
#endif
				psglRenderUI();
				break;
			case DIP_MENU:						
				psglClearUI();
				psglRenderPaused();
				DipMenu();
				DipFrameMove();			
				psglRenderUI();
				break;
			case SCREEN_RESIZE:
				psglClearUI();			
				psglRenderStretch();
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
			case EMULATING:
				if(!is_running)
					GameStatus = PAUSE;
				CalculateViewports();
				if(pVidTransImage)
				{
					if(bVidRecalcPalette)
					{
						audio_check();
						nCurrentFrame++;
						VidFrame_RecalcPalette();
					}
					do{
						audio_check();
						nCurrentFrame++;
						VidFrame_Recalc();
						InputMake();
#ifdef CELL_DEBUG_CONSOLE
						cellConsolePoll();
#endif
					}while(is_running);
				}
				else
				{
					do{
						audio_check();
						nCurrentFrame++;
						VidFrame();
						InputMake();
#ifdef CELL_DEBUG_CONSOLE
						cellConsolePoll();
#endif
					}while(is_running);
				}
				break;
		}
	}while(!exitGame);

	audio_stop();					// Stop sound if it was playing
	BurnerDrvExit();				// Make sure any game driver is exitted
	mediaExit();					// Exit media
	return 0;
}
 
