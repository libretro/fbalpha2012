// Run module
#include <sys/sys_time.h>
#include <cell/rtc.h>
#include <PSGL/psgl.h>
#include "burner.h" 
#include "menu.h" 
#include "vid_psgl.h"
#include "vid_psgl_support.h"
#ifdef CELL_DEBUG_CONSOLE
#include <cell/control_console.h>
#endif
#include "cellframework2/audio/stream.h"

int GameStatus = MENU;

int nShowEffect = 0;
int bRunPause = 0;
int bAltPause = 0;
int autoFrameSkip = 0;
#define NAPP_VIRTUAL_FPS 6000
int nAppVirtualFps = 6000;				// App fps * 100
int bOnce = 0;
int is_running = 0;
bool bShowFPS = false;
int custom_aspect_ratio_mode = 0;

extern bool exitGame;
extern void doStretch();
extern void StretchMenu();
cell_audio_handle_t audio_handle;
const struct cell_audio_driver *driver;

static uint64_t inline GetTickCount()
{
	CellRtcTick ticks;
	cellRtcGetCurrentTick(&ticks);
	return ticks.tick;
}

int RunInit()
{
	// Try to run with sound
	audio.play();
	bOnce = 0;
	return 0;
}

extern void audio_check();

int RunExit()
{
	// Stop sound if it was playing
	audio.stop();
	return 0;
}

// The main message loop
int RunMessageLoop(int argc, char **argv)
{
	int bRestartVideo;
	int once = 0;
	extern unsigned int nPrevGame;
	static int bPrevPause = 0;
	static int bPrevDraw = 0;

	bRestartVideo = 0;

	RunInit();

	// get the last filter
	CurrentFilter = nLastFilter;	 
	BuildRomList();


#ifdef MULTIMAN_SUPPORT
	if(argc > 1)
	{
		const char * current_game = strrchr(strdup(argv[1]), '/');
		directLoadGame(strdup(current_game));
		mediaInit();
		RunInit();
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
				do{
					// RUN IDLE
					audio_check();

					// RUN FRAME
					nCurrentFrame++;
					VidFrame();
					// GET INPUT
					InputMake();
#ifdef CELL_DEBUG_CONSOLE
					cellConsolePoll();
#endif
				}while(is_running);
				break;
		}
	}while(!exitGame);

	RunExit();				 		 		 
	BurnerDrvExit();				// Make sure any game driver is exitted
	mediaExit();					// Exit media
	return 0;
}
 
