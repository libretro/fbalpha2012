// Run module
#include "burner.h"
#include "maphkeys.h"
#include "vid_directx_support.h"

int bRunPause = 0;
int bAltPause = 0;
int autoFrameSkip = 0;
int nAppVirtualFps = 6000;				// App fps * 100

bool bShowFPS = false;
static unsigned int nDoFPS = 0;
TCHAR fpsString[16];

static unsigned int nNormalLast = 0;	// Last value of timeGetTime()
static int nNormalFrac = 0;				// Extra fraction we did

bool bAppDoStep = 0;
bool bAppDoFast = 0;
unsigned int nFastSpeed = 2;

static inline int GetInput(bool bCopy)
{
	static int i = 0;
	InputMake(bCopy); 					// get input

	// FBA Combo - extra players input hack
	if (bInputSwap && !kNetGame && bCopy) {
		BurnDrvInputSwap();
	}

	// Update Input dialog ever 3 frames
	if (i++ == 0) {
		InpdUpdate();
	}

	if (i >= 3) {
		i = 0;
	}

	// Update Input Set dialog
	InpsUpdate();
	return 0;
}

static inline void DisplayFPS()
{
	static time_t fpsTimer;
	static unsigned int nPreviousFrames;
	float burnFps = (float)nBurnFPS / 100;

	time_t tempTime = clock();
	double fps = (double)(nFramesEmulated - nPreviousFrames) * CLOCKS_PER_SEC / (tempTime - fpsTimer);
	_sntprintf(fpsString, sizearray(fpsString), _T("%2.2lf/%2.2lf"), fps, burnFps);
	VidSNewShortMsg(fpsString, 0xDFDFFF, 480, 0);

	fpsTimer = tempTime;
	nPreviousFrames = nFramesEmulated;

	// display fps on window title
	scrnTitle();
}

// With or without sound, run one frame.
// If bDraw is true, it's the last frame before we are up to date, and so we should draw the screen
static int RunFrame(int bDraw, int bPause)
{
	static int bPrevPause = 0;
	static int bPrevDraw = 0;

	if (bPrevDraw && !bPause) {
		VidPaint(0);						// paint the screen (no need to validate)
	}

	if (!bDrvOkay && !nShowEffect) {
		return 1;
	}

	if (bPause) {
		GetInput(false);					// Update burner inputs, but not game inputs
		if (bPause != bPrevPause) {
			VidPaint(2);					// Redraw the screen (to ensure mode indicators are updated)
		}
	} else {
		nFramesEmulated++;
		nCurrentFrame++;

		if (kNetGame) {
			GetInput(true);					// Update inputs

			if (KailleraGetInput()) {		// Synchronize input with Kaillera
				return 0;
			}

			// FBA Combo - extra players input hack
			if (bInputSwap) {
				BurnDrvInputSwap();
			}
		} else {
			if (nReplayStatus == 2) {
				GetInput(false);			// Update burner inputs, but not game inputs
				if (ReplayInput()) {		// Read input from file
					setPauseMode(true);		// Auto pause after replay
					menuSync(MENUT_FILE);
					InputSetCooperativeLevel(false, false);
				}
			} else {
				GetInput(true);				// Update inputs
			}
		}

		if (nReplayStatus == 1) {
			RecordInput();					// Write input to file
		}

		if (bDraw) {
			nFramesRendered++;

			if (VidFrame()) {				// Do one frame
				audio.blank();
			}
		} else {							// frame skipping
			pBurnDraw = NULL;				// Make sure no image is drawn
			if (bDrvOkay || !nShowEffect) {
				BurnDrvFrame();
			}
		}

		if (bShowFPS) {
			if (nDoFPS < nFramesRendered) {
				DisplayFPS();
				nDoFPS = nFramesRendered + 30;
			}
		}
	}

	if (nAviStatus) {
		if (AviRecordFrame(bDraw)) {
			AviStop();
		}
	}

	bPrevPause = bPause;
	bPrevDraw = bDraw;

	return 0;
}

// Callback used when DSound needs more sound
static int RunGetNextSound(int bDraw)
{
	if (pAudNextSound == NULL) {
		return 1;
	}

	if (bRunPause) {
		if (bAppDoStep) {
			RunFrame(bDraw, 0);
			AudWriteSlience();
		} else {
			RunFrame(bDraw, 1);
		}

		bAppDoStep = 0;						// done one step
		return 0;
	}

	if (bAppDoFast) {						// do more frames
		for (unsigned int i = 0; i < nFastSpeed; i++) {
			RunFrame(0, 0);
		}
	}

	// Render frame with sound
	pBurnSoundOut = pAudNextSound;
	RunFrame(bDraw, 0);

	// Log wave file
	waveLogWrite();

	if (bAppDoStep) {
		AudWriteSlience();
	}
	bAppDoStep = 0;							// done one step

	return 0;
}

int RunIdle()
{
	if (audio.bAudPlaying) {
		// Run with sound
		audio.check();
		return 0;
	}

	// Run without sound
	int nTime = timeGetTime() - nNormalLast;
	int nCount = (nTime * nAppVirtualFps - nNormalFrac) / 100000;
	if (nCount <= 0) {						// No need to do anything for a bit
		Sleep(2);
		return 0;
	}

	nNormalFrac += nCount * 100000;
	nNormalLast += nNormalFrac / nAppVirtualFps;
	nNormalFrac %= nAppVirtualFps;

	if (bAppDoFast){						// Temporarily increase virtual fps
		nCount += nFastSpeed * 10;
	}
	if (nCount > 100) {						// Limit frame skipping
		nCount = 100;
	}

	if (bRunPause) {
		if (bAppDoStep) {					// Step one frame
			nCount = 10;
		} else {
			RunFrame(1, 1);					// Paused
			return 0;
		}
	}
	bAppDoStep = 0;

	for (int i = nCount / 10; i > 0; i--) {	// Mid-frames
		RunFrame(autoFrameSkip, 0);
	}
	RunFrame(1, 0);							// End-frame

	return 0;
}

int RunReset()
{
	// Reset the speed throttling code
	nNormalLast = 0;
	nNormalFrac = 0;
	// Reset FPS display
	nDoFPS = 0;

	if (!audio.bAudPlaying) {
		// run without sound
		nNormalLast = timeGetTime();
	}
	return 0;
}

int RunInit()
{
	// Try to run with sound
	audio.set(RunGetNextSound);
	audio.play();

	RunReset();

	return 0;
}

int RunExit()
{
	nNormalLast = 0;
	// Stop sound if it was playing
	audio.stop();
	return 0;
}

static int RunKeyMsg(const MSG& Msg)
{
	if (Msg.message == WM_SYSKEYDOWN || Msg.message == WM_KEYDOWN) {
		return MHkeysDownHandle(Msg);
	} else {
		if (Msg.message == WM_SYSKEYUP || Msg.message == WM_KEYUP) {
			return MHkeysUpHandle(Msg);
		}
	}

	return 0;
}

// The main message loop
int RunMessageLoop()
{
	int bRestartVideo;
	MSG Msg;

	do {
		bRestartVideo = 0;

		// Remove pending initialisation messages from the queue
		while (PeekMessage(&Msg, NULL, WM_APP + 0, WM_APP + 0, PM_NOREMOVE)) {
			if (Msg.message != WM_QUIT) {
				PeekMessage(&Msg, NULL, WM_APP + 0, WM_APP + 0, PM_REMOVE);
			}
		}

		RunInit();

		ShowWindow(hScrnWnd, nAppShowCmd);					// Show the screen window
		nAppShowCmd = SW_NORMAL;
		SetForegroundWindow(hScrnWnd);

		GameInpCheckLeftAlt();
		GameInpCheckMouse();								// Hide the cursor

		while (1) {
			if (PeekMessage(&Msg, NULL, 0, 0, PM_REMOVE)) {
				// A message is waiting to be processed
				if (Msg.message == WM_QUIT)	{				// Quit program
					break;
				}
				if (Msg.message == (WM_APP + 0)) {			// Restart video
					bRestartVideo = 1;
					break;
				}

				// process key message
				if (RunKeyMsg(Msg)) {
					continue;
				}

				// Check for messages for dialogs etc.
				if (!AppMessage(&Msg)) {
					continue;
				}

//				if (TranslateAccelerator(hScrnWnd, hAccel, &Msg)) {
//					continue;
//				}

				TranslateMessage(&Msg);
				DispatchMessage(&Msg);
			} else {
				// No messages are waiting
				SplashDestroy(0);
				RunIdle();
			}
		}

		RunExit();
		mediaExit();
		if (bRestartVideo) {
			mediaInit();
		}
	} while (bRestartVideo);

	return 0;
}
