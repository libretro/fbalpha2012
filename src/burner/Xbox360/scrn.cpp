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
#include "vid_directx_support.h"
#include "InGameOptions.h"
#ifndef NO_AUTOFIRE
#include "autofire.h"
#endif
 

RECT SystemWorkArea = { 0, 0, 1280, 720 };		// Work area on the desktop
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

extern void dx9SetTargets();
extern void simpleReinitScrn(const bool& reinitVid);
extern void doResetDX9();
extern HXUIOBJ hInGameOptionsScene;
//int nScrnVisibleOffset[4] = { 0, 0, 0, 0 };		// scrn visible offset

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

		XUIMessage xuiMsg;
		InGameMenuStruct msgData;
		InGameMenuFirstFunc( &xuiMsg, &msgData, 1 );
		XuiSendMessage( hInGameOptionsScene, &xuiMsg );

		while (bRunPause)
		{			
			pDevice->Clear(
					0,
					NULL,
					D3DCLEAR_TARGET | D3DCLEAR_STENCIL | D3DCLEAR_ZBUFFER,
					D3DCOLOR_ARGB( 255, 0, 0, 0 ),
					1.0,
			0 );
 
			// Update XUI
			app.RunFrame();

			// Render XUI
			app.Render();

			// Update XUI Timers
			XuiTimersRun();

			// Present the frame.
			pDevice->Present( NULL, NULL, NULL, NULL );
		}
  
	} else {
		GameInpCheckMouse();
	}

	simpleReinitScrn(true);

}
 

// Returns true if a VidInit is needed when the window is resized
static inline bool vidInitNeeded()
{
	if (nVidSelect == VID_D3D) {
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
	scrnSize();

	// need for dx9 blitter
	if (reinitVid || vidInitNeeded()) {
		VidReinit();
	} 
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
