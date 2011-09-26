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
 

RECT SystemWorkArea = { 0, 0, 640, 480 };		// Work area on the desktop
int nWindowPosX = -1, nWindowPosY = -1;			// Window position

int bAutoPause = 0;
int nSavestateSlot = 1;

bool bShowOnTop = false;
bool bFullscreenOnStart = false;
bool bFakeFullscreen = false;

static TCHAR* szClass = _T(APP_TITLE);			// Window class name
HWND hScrnWnd = NULL;							// Handle to the screen window
HWND hVideoWnd = NULL;							// Handle to the video window

static bool bMaximised;
static int nPrevWidth, nPrevHeight;

int nWindowSize = 0;

static bool bDrag = false;
static int nDragX, nDragY;
static int nOldWindowX, nOldWindowY;
static int nLeftButtonX, nLeftButtonY;
 
bool useDialogs()
{
	return false;
}

void setPauseModeScreen(bool bPause)
{
	bRunPause = bPause;
	bAltPause = bPause;

	if (bPause) {
		audio.blank();		
	} else {
		GameInpCheckMouse();
	}
}

void setPauseMode(bool bPause)
{
	bRunPause = bPause;
	bAltPause = bPause;

	if (bPause) {

		audio.blank();
 
	} else {
		GameInpCheckMouse();
	}
 
}
 

// Returns true if a VidInit is needed when the window is resized
static inline bool vidInitNeeded()
{
	if (nVidSelect == VID_PSGL) {
		return true;
	}
	return false;
}

// Refresh the contents of the window when re-sizing it
static inline void refreshWindow(bool bInitialise)
{
 
}

// simply reinit screen, added by regret
void simpleReinitScrn(const bool& reinitVid)
{
	VidReinit();
}
 
 

bool bLoading = false;
 
static int scrnRegister()
{
	return 0;
}

int scrnTitle()
{
	return 0;
}

int scrnSize()
{
  	return 0;
}

// Init the screen window (create it)
int scrnInit()
{
	return 0;
}

// Exit the screen window (destroy it)
int scrnExit()
{
	return 0;
}

// ==> switch fullscreen, added by regret
int scrnSwitchFull()
{
	return 0;
}

// fake fullscreen
int scrnFakeFullscreen()
{
	return 0;
}

// set fullscreen (for opengl)
int scrnSetFull(const bool& full)
{ 
	return 0;
}
// <== switch fullscreen

void __cdecl scrnReinit()
{
	 simpleReinitScrn(true);
}
