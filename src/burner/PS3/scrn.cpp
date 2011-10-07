#include "burner.h"
#include "cheat.h"
#include "cartridge.h"
#include "../../interface/PS3/audio_driver.h"
 
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
void simpleReinitScrn(void)
{
	VidReinit();
}
