// Screen Window
// Simplify screen reinit routine by regret

/* changelog:
 update 6: add fake fullscreen
 update 5: add scrnSwitchFull to switch fullscreen, scrnExit will not be called
 update 4: simplify enter/exit fullscreen routine
 update 3: do not delete screen in scrnInit
 update 2: add simpleReinitScrn
 update 1: source cleanup
*/

#include "burner.h"
#include "cheat.h"
#include "cartridge.h"
#include "vid_psgl_support.h"
#include "InGameOptions.h"
#ifndef NO_AUTOFIRE
#include "autofire.h"
#endif
 
//RECT SystemWorkArea = { 0, 0, 640, 480 };		// Work area on the desktop
int nWindowPosX = -1, nWindowPosY = -1;			// Window position

int bAutoPause = 0;
int nSavestateSlot = 1;

bool bShowOnTop = false;
bool bFullscreenOnStart = false;

static bool bMaximised;
static int nPrevWidth, nPrevHeight;

int nWindowSize = 0;

static bool bDrag = false;
static int nDragX, nDragY;
static int nOldWindowX, nOldWindowY;
static int nLeftButtonX, nLeftButtonY;
 
void setPauseModeScreen(bool bPause)
{
	bRunPause = bPause;
	bAltPause = bPause;

	if (bPause)
		audio.blank();		
}

void setPauseMode(bool bPause)
{
	bRunPause = bPause;
	bAltPause = bPause;

	if (bPause)
		audio.blank();
}
 
// simply reinit screen, added by regret
void simpleReinitScrn(const bool& reinitVid)
{
	VidReinit();
}

void __cdecl scrnReinit()
{
	 simpleReinitScrn(true);
}
