// Media module
// Add media reinit by regret

#include "burner.h"

static int nBaseFps;

// Media init / exit
int mediaInit()
{
	if (scrnInit()) {					// Init the Scrn Window
		return 1;
	}

	if (!bInputOkay) {
		InputInit();					// Init Input
	}

	nBaseFps = nBurnFPS;
	nAppVirtualFps = nBurnFPS;
 
	if (!bAudOkay) {
		audio.init();					// Init Sound (not critical if it fails)

		if (!bAudOkay) {
			// Make sure the error will be visible
 
		}
	}

	nBurnSoundRate = 0;					// Assume no sound
	pBurnSoundOut = NULL;
	if (bAudOkay) {
		nBurnSoundRate = nAudSampleRate;
		nBurnSoundLen = nAudSegLen;
	}

	if (!bVidOkay) {
		if (!nVidFullscreen) {
			scrnSize();
		}

		// Reinit the video plugin
		VidInit();

		if (!bVidOkay && nVidFullscreen) {
			nVidFullscreen = 0;
			mediaExit();
			return (mediaInit());
		}
		

		if (bVidOkay && ((bRunPause && bAltPause) || !bDrvOkay)) {
			VidRedraw();
		}
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
#if 0
	// reinit audio if we select directsound
	int withAudio = _tcscmp(audSelect, _T("DirectSound")) ? 0 : 1;

	RunExit();

	if (withAudio) {
		nBurnSoundRate = 0;				// Blank sound
		pBurnSoundOut = NULL;

		audio.exit();					// Exit audio
	}

	VidExit();

	InputExit();

	scrnExit();							// Exit the Scrn Window

	if (scrnInit()) {					// Init the Scrn Window
		return 1;
	}

	if (!bInputOkay) {
		InputInit();					// Init Input
	}

	if (withAudio) {
		audio.init();					// Init Sound

		if (bAudOkay) {
			nBurnSoundRate = nAudSampleRate;
			nBurnSoundLen = nAudSegLen;
		}
	}

	VidReinit();

	RunInit();
	ShowWindow(hScrnWnd, SW_NORMAL);	// Show the screen window
	SetForegroundWindow(hScrnWnd);
#endif
	return 0;
}

int mediaReInitAudio()
{
	if (audio.bAudPlaying) {
		audio.stop();
	}

	nBurnSoundRate = 0;					// Blank sound
	pBurnSoundOut = NULL;

	if (audio.exit()) {
		return 1;
	}

	if (audio.init()) {

		return 1;
	}

	nBurnSoundRate = nAudSampleRate;
	nBurnSoundLen = nAudSegLen;

	RunInit();

	return 0;
}

// change FPS
int mediaChangeFps(int scale)
{
	nAppVirtualFps = nBaseFps * scale / 100;

	if (!bAudOkay) {
		return 1;
	}

	if (audio.bAudPlaying) {
		audio.stop();
	}

	if (audio.setfps()) {
		return 1;
	}

	nBurnSoundRate = nAudSampleRate;
	nBurnSoundLen = nAudSegLen;
	audio.play();

	return 0;
}
