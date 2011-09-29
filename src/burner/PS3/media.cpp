// Media module
// Add media reinit by regret

#include "burner.h"

static int nBaseFps;

// Media init / exit
int mediaInit(void)
{
	if (!bInputOkay)
		InputInit();					// Init Input

	nBaseFps = nBurnFPS;
	nAppVirtualFps = nBurnFPS;

	if (!bAudOkay)
	{
		audio.init();					// Init Sound (not critical if it fails)

		if (!bAudOkay)
		{
			// Make sure the error will be visible
		}
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
		VidInit(); // Reinit the video plugin

		if (bVidOkay && ((bRunPause && bAltPause) || !bDrvOkay))
			VidFrame();
	}

	return 0;
}

int mediaExit()
{
	nBurnSoundRate = 0;					// Blank sound
	pBurnSoundOut = NULL;

	audio.exit();						// Exit audio
	VidExit();
	InputExit();
	return 0;
}

// reinit screen, audio, added by regret
int mediaReInitScrn()
{
	return 0;
}

int mediaReInitAudio()
{
	if (audio.bAudPlaying)
		audio.stop();

	nBurnSoundRate = 0;					// Blank sound
	pBurnSoundOut = NULL;

	if (audio.exit())
		return 1;

	if (audio.init())
		return 1;

	nBurnSoundRate = nAudSampleRate;
	nBurnSoundLen = nAudSegLen;

	RunInit();

	return 0;
}

// change FPS
int mediaChangeFps(int scale)
{
	nAppVirtualFps = nBaseFps * scale / 100;

	if (!bAudOkay)
		return 1;

	if (audio.bAudPlaying)
		audio.stop();

	nBurnSoundRate = nAudSampleRate;
	nBurnSoundLen = nAudSegLen;
	audio.play();

	return 0;
}
