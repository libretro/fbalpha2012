// Media module
// Add media reinit by regret

#include "burner.h"
#include "../../interface/PS3/audio_driver.h"

//forward declarations
int VidReinit(void);

// Media init / exit
int mediaInit(void)
{
	if (!bInputOkay)
		InputInit();		// Init Input

	nAppVirtualFps = nBurnFPS;

	if (!bAudOkay)
	{
		audio_init();		// Init Sound (not critical if it fails)

		/*
		if (!bAudOkay)		// Make sure the error will be visible
		{
		}
		*/
	}

	// Assume no sound
	nBurnSoundRate = 0;					
	pBurnSoundOut = NULL;

	if (bAudOkay)
	{
		nBurnSoundRate = nAudSampleRate;
		nBurnSoundLen = nAudSegLen;
	}

	if (!bVidOkay)
	{
		VidInit();		// Reinit the video plugin

		if (bVidOkay && (!bDrvOkay))
			VidFrame();
	}

	return 0;
}

int mediaExit(void)
{
	nBurnSoundRate = 0;		// Blank sound
	pBurnSoundOut = NULL;

	audio_exit();			// Exit audio
	VidExit();
	InputExit();
	return 0;
}

int mediaReInitAudio(void)
{
	if (bAudPlaying)
		audio_stop();

	nBurnSoundRate = 0;		// Blank sound
	pBurnSoundOut = NULL;

	if (audio_exit())
		return 1;

	if (audio_init())
		return 1;

	nBurnSoundRate = nAudSampleRate;
	nBurnSoundLen = nAudSegLen;

	audio_play();

	return 0;
}

int mediaChangeFps(int scale)		// change FPS
{
	nAppVirtualFps = nBurnFPS * scale / 100;

	if (!bAudOkay)
		return 1;

	if (bAudPlaying)
		audio_stop();

	nBurnSoundRate = nAudSampleRate;
	nBurnSoundLen = nAudSegLen;
	audio_play();

	return 0;
}

// simply reinit screen, added by regret
void simpleReinitScrn(void)
{
	VidReinit();
}
