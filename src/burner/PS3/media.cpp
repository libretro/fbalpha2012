// Media module
// Add media reinit by regret

#include "burner.h"
#include "../../interface/PS3/audio_driver.h"
#include "../../interface/PS3/input_driver.h"

//forward declarations
int VidReinit(void);

// Media init / exit
int mediaInit(void)
{
	if (!bInputOkay)
		InputInit();		// Init Input

	nAppVirtualFps = nBurnFPS;

	char * szName;
	BurnDrvGetArchiveName(&szName, 0);
	char  * vendetta_hack = strstr(szName,"vendetta");

	if (!bAudOkay || vendetta_hack || bAudReinit)
	{
		if(vendetta_hack)
		{
			// If Vendetta is not played with sound samplerate at 44KHz, then
			// slo-mo Vendetta happens - so we set sound samplerate at 44KHz
			// and then resample to 48Khz for this game. Possibly more games 
			// are like this, so check for more
			audio_init(SAMPLERATE_44KHZ);
			bAudReinit = true;
		}
		else
		{
			audio_init(SAMPLERATE_48KHZ);
			bAudReinit = false;
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

	if (audio_init(48000))
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
