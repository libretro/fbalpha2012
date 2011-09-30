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
#include "InGameOptions.h"
#ifndef NO_AUTOFIRE
#include "autofire.h"
#endif
#include "../../interface/PS3/audio_driver.h"
 
int bAutoPause = 0;
int nSavestateSlot = 1;

void setPauseModeScreen(bool bPause)
{
	bRunPause = bPause;
	bAltPause = bPause;

	if (bPause)
		audio_blank();		
}

void setPauseMode(bool bPause)
{
	bRunPause = bPause;
	bAltPause = bPause;

	if (bPause)
		audio_blank();
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
