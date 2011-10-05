// Burner Input Dialog module
// added default mapping and autofire settings, by regret
#include "burner.h"

static void GameInpConfigOne(int nPlayer, int nPcDev, int nAnalog, struct GameInp* pgi, char* szi)
{
	switch (nPcDev)
	{
		case  0:
			GamcPlayer(pgi, szi, nPlayer, -1);						// Keyboard
			GamcAnalogKey(pgi, szi, nPlayer, nAnalog);
			GamcMisc(pgi, szi, nPlayer);
			break;
		case  1:
			GamcPlayer(pgi, szi, nPlayer, 0);						// Joystick 1
			GamcAnalogJoy(pgi, szi, nPlayer, 0, nAnalog);
			GamcMisc(pgi, szi, nPlayer);
			break;
		case  2:
			GamcPlayer(pgi, szi, nPlayer, 1);						// Joystick 2
			GamcAnalogJoy(pgi, szi, nPlayer, 1, nAnalog);
			GamcMisc(pgi, szi, nPlayer);
			break;
		case  3:
			GamcPlayer(pgi, szi, nPlayer, 2);						// Joystick 3
			GamcAnalogJoy(pgi, szi, nPlayer, 2, nAnalog);
			GamcMisc(pgi, szi, nPlayer);
			break;
		case  4:
			GamcPlayerHotRod(pgi, szi, nPlayer, 0x10, nAnalog);		// X-Arcade left side
			GamcMisc(pgi, szi, -1);
			break;
		case  5:
			GamcPlayerHotRod(pgi, szi, nPlayer, 0x11, nAnalog);		// X-Arcade right side
			GamcMisc(pgi, szi, -1);
			break;
		case  6:
			GamcPlayerHotRod(pgi, szi, nPlayer, 0x00, nAnalog);		// HotRod left side
			GamcMisc(pgi, szi, -1);
			break;
		case  7:
			GamcPlayerHotRod(pgi, szi, nPlayer, 0x01, nAnalog);		// HotRod right size
			GamcMisc(pgi, szi, -1);
			break;
	}
}

// Configure some of the game input
static int GameInpConfig(int nPlayer, int nPcDev, int nAnalog)
{
	struct GameInp* pgi = NULL;
	unsigned int i;

	for (i = 0, pgi = GameInp; i < nGameInpCount; i++, pgi++)
	{
		struct BurnInputInfo bii;

		// Get the extra info about the input
		bii.szInfo = NULL;
		BurnDrvGetInputInfo(&bii, i);

		if (bii.pVal == NULL)
			continue;

		if (bii.szInfo == NULL)
			bii.szInfo = "";

		GameInpConfigOne(nPlayer, nPcDev, nAnalog, pgi, bii.szInfo);
	}

	for (i = 0; i < nMacroCount; i++, pgi++)
		GameInpConfigOne(nPlayer, nPcDev, nAnalog, pgi, pgi->Macro.szName);

	return 0;
}
 
 

static int DeleteInput(unsigned int i)
{
	return 0;
}

// List item(s) deleted; find out which one(s)
static int ListItemDelete()
{
	return 0;
}

static int InitAnalogOptions(int nGi, int nPci)
{
	return 0;
}

int UsePreset(bool bMakeDefault)
{
	return 0;
}

// ==> default input mapping, added by regret
char* defaultInpFilename = "/dev_hdd0/game/FBAN00000/USRDIR/config/presets/default.ini";
char* defaultNeoInpFilename = "/dev_hdd0/game/FBAN00000/USRDIR/config/presets/default_neo.ini";
char* defaultCpsInpFilename = "/dev_hdd0/game/FBAN00000/USRDIR/config/presets/default_cps.ini";

static const char * GetDefaultInputFilename()
{
	char* fileName = defaultInpFilename;

	int flag = (BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK);

	if (flag == HARDWARE_SNK_NEOGEO)
		fileName = defaultNeoInpFilename;
	else if (flag == HARDWARE_CAPCOM_CPS1 || flag == HARDWARE_CAPCOM_CPS1_QSOUND || flag == HARDWARE_CAPCOM_CPS1_GENERIC || flag == HARDWARE_CAPCOM_CPSCHANGER || flag == HARDWARE_CAPCOM_CPS2 || flag == HARDWARE_CAPCOM_CPS3)
		fileName = defaultCpsInpFilename;

	return fileName;
}

int SaveDefaultInput()
{

	const char * fileName = GetDefaultInputFilename();
	FILE* h = fopen(fileName, "wt");

	if (h == NULL)
		return 1;

	// Write version number
	fprintf(h, "version 0x%06X\n\n", nBurnVer);
	GameInpWrite(h, false);

	if(h)
		fclose(h);

	return 0;
}

int loadDefaultInput()
{
	const char * fileName = GetDefaultInputFilename();

	// Read default inputs from file
	for (int nPlayer = 0; nPlayer < nMaxPlayers; nPlayer++)
		GameInputAutoIni(nPlayer, fileName, true);

	return 0;
}
