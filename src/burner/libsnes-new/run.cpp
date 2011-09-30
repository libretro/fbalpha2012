// Run module
#include "burner.h" 
#include "menu.h" 
#include "../../interface/libsnes-new/vid_psgl.h"				// FIXME: Your own header file here with video function forward decl.
#include "../../interface/libsnes-new/audio_driver.h" // FIXME : Your own header file here wih audio function forward decl.

int nShowEffect = 0;
int bRunPause = 0;
int bAltPause = 0;
int autoFrameSkip = 0;
#define NAPP_VIRTUAL_FPS 6000
int nAppVirtualFps = 6000;				// App fps * 100
int is_running = 0;
bool bShowFPS = false;
int custom_aspect_ratio_mode = 0;
extern bool exitGame;

// The main message loop
int RunMessageLoop(int argc, char **argv)
{
	int bRestartVideo;
	int once = 0;
	extern unsigned int nPrevGame;
	static int bPrevPause = 0;
	static int bPrevDraw = 0;

	bRestartVideo = 0;

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
		nPrevGame = 0;
	}
#endif

	do{

		CalculateViewports();
		do{
			// RUN IDLE
			audio_check();

			// RUN FRAME
			nCurrentFrame++;
			VidFrame();
			// GET INPUT
			InputMake();
		}while(is_running);
		break;
	}while(!exitGame);

	audio_stop();					// Stop sound if it was playing
	BurnerDrvExit();				// Make sure any game driver is exitted
	mediaExit();					// Exit media
	return 0;
}
