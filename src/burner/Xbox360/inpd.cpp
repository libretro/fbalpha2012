// Burner Input Dialog module
// added default mapping and autofire settings, by regret
#include "burner.h"
#ifndef NO_AUTOFIRE
#include "autofire.h"
#endif

HWND hInpdDlg = NULL;						// Handle to the Input Dialog
static HWND hInpdList = NULL;
static unsigned char* LastVal = NULL;		// Last input values/defined
static int bLastValDefined = 0;				//

static HWND hInpdGi = NULL, hInpdPci = NULL, hInpdAnalog = NULL;	// Combo boxes

#ifndef NO_AUTOFIRE
// autofire map
struct AutoFireInfo {
	int player;
	int button;
};
map<int, AutoFireInfo> autofireMap;

// ==> autofire setting, added by regret
static int InpdAutofireInit()
{
 

	return 0;
}

static void InpdAutofireExit()
{
 
}
// <== autofire setting
#endif

// Update which input is using which PC input
static int InpdUseUpdate()
{
 

	return 0;
}

int InpdUpdate()
{
 

	return 0;
}

static int InpdListBegin()
{
 

	return 0;
}

// Make a list view of the game inputs
int InpdListMake(int bBuild)
{
 
	InpdUseUpdate();

	return 0;
}

 
static int InpdInit()
{
 

	return 0;
}

static int InpdExit()
{
 

	return 0;
}

static void GameInpConfigOne(int nPlayer, int nPcDev, int nAnalog, struct GameInp* pgi, char* szi)
{
	switch (nPcDev) {
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

	for (i = 0, pgi = GameInp; i < nGameInpCount; i++, pgi++) {
		struct BurnInputInfo bii;

		// Get the extra info about the input
		bii.szInfo = NULL;
		BurnDrvGetInputInfo(&bii, i);
		if (bii.pVal == NULL) {
			continue;
		}
		if (bii.szInfo == NULL) {
			bii.szInfo = "";
		}
		GameInpConfigOne(nPlayer, nPcDev, nAnalog, pgi, bii.szInfo);
	}

	for (i = 0; i < nMacroCount; i++, pgi++) {
		GameInpConfigOne(nPlayer, nPcDev, nAnalog, pgi, pgi->Macro.szName);
	}

	GameInpCheckLeftAlt();

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
TCHAR* defaultInpFilename = _T("GAME:\\config\\presets\\default.ini");
TCHAR* defaultNeoInpFilename = _T("GAME:\\config\\presets\\default_neo.ini");
TCHAR* defaultCpsInpFilename = _T("GAME:\\config\\presets\\default_cps.ini");

static TCHAR* GetDefaultInputFilename()
{
	TCHAR* fileName = defaultInpFilename;

#if 1
	int flag = (BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK);

	if (flag == HARDWARE_SNK_NEOGEO) {
		fileName = defaultNeoInpFilename;
	}
	else if (flag == HARDWARE_CAPCOM_CPS1
		|| flag == HARDWARE_CAPCOM_CPS1_QSOUND
		|| flag == HARDWARE_CAPCOM_CPS1_GENERIC
		|| flag == HARDWARE_CAPCOM_CPSCHANGER
		|| flag == HARDWARE_CAPCOM_CPS2
		|| flag == HARDWARE_CAPCOM_CPS3) {
		fileName = defaultCpsInpFilename;
	}
#endif

	return fileName;
}

int SaveDefaultInput()
{
 
	TCHAR* fileName = GetDefaultInputFilename();
	FILE* h = _tfopen(fileName, _T("wt"));
	if (h == NULL) {
		return 1;
	}

	// Write version number
	_ftprintf(h, _T("version 0x%06X\n\n"), nBurnVer);
	GameInpWrite(h, false);
	if (h) {
		fclose(h);
	}

	return 0;
}

int loadDefaultInput()
{
	TCHAR* fileName = GetDefaultInputFilename();

	// Read default inputs from file
	for (int nPlayer = 0; nPlayer < nMaxPlayers; nPlayer++) {
		GameInputAutoIni(nPlayer, fileName, true);
	}
	return 0;
}
// <== default input mapping

// ==> save preset, added by regret
static int savePreset(HWND hDlg)
{
 
	return 0;
}

 

int InpdCreate()
{
 
	return 0;
}
