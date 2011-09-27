// Burner Config for Game file module
#include "burner.h"

#ifdef __LIBSNES__
#include "../burn/ssnes-typedefs.h"
#endif

const int nConfigMinVersion = 0x020921;

static char* GameConfigName()
{
	// Return the path of the config file for this game
	static char szName[MAX_PATH];
#ifdef _XBOX
	sprintf(szName, "game:\\config\\games\\%S.ini", BurnDrvGetText(DRV_NAME));
#else
	sprintf(szName, "/dev_hdd0/game/FBAN00000/USRDIR/config/games/%s.ini", BurnDrvGetTextA(DRV_NAME));
#endif
	return szName;
}

// Read in the config file for the game-specific inputs
int ConfigGameLoad(bool bOverWrite)
{
	char szLine[MAX_PATH];
	int nFileVersion = 0;

	FILE* h = fopen(GameConfigName(), "rt");
	if (h == NULL) {
		return 1;
	}

	if (bOverWrite) {
		nAnalogSpeed = 0x0100;
		nBurnCPUSpeedAdjust = 0x0100;
#ifndef NO_AUTOFIRE
		autofireDelay = autofireDefaultDelay;
#endif
	}

	// Go through each line of the config file and process inputs
	while (fgets(szLine, sizeof(szLine), h)) {
		char *szValue;
		size_t nLen = strlen(szLine);

		// Get rid of the linefeed at the end
		if (szLine[nLen - 1] == 10) {
			szLine[nLen - 1] = 0;
			nLen--;
		}

		szValue = labelCheck(szLine, "version");
		if (szValue) {
			nFileVersion = strtol(szValue, NULL, 0);
		}

		if (bOverWrite) {
			szValue = labelCheck(szLine, "analog");
			if (szValue) {
				nAnalogSpeed = strtol(szValue, NULL, 0);
			}
			szValue = labelCheck(szLine, "cpu");
			if (szValue) {
				nBurnCPUSpeedAdjust = strtol(szValue, NULL, 0);
			}
#ifndef NO_AUTOFIRE
			szValue = labelCheck(szLine, _T("autofire-delay"));
			if (szValue) {
				autofireDelay = strtol(szValue, NULL, 0);
			}
#endif
#if 0
			szValue = labelCheck(szLine, _T("top"));
			if (szValue) {
				nScrnVisibleOffset[0] = strtol(szValue, NULL, 0);
			} szValue = labelCheck(szLine, _T("left")); if (szValue) {
				nScrnVisibleOffset[1] = strtol(szValue, NULL, 0);
			}
			szValue = labelCheck(szLine, _T("bottom"));
			if (szValue) {
				nScrnVisibleOffset[2] = strtol(szValue, NULL, 0);
			}
			szValue = labelCheck(szLine, _T("right"));
			if (szValue) {
				nScrnVisibleOffset[3] = strtol(szValue, NULL, 0);
			}
#endif
		}

		if (nConfigMinVersion <= nFileVersion && nFileVersion <= nBurnVer) {
			szValue = labelCheck(szLine, _T("input"));
			if (szValue) {
				GameInpRead(szValue, bOverWrite);
				continue;
			}

			szValue = labelCheck(szLine, _T("macro"));
			if (szValue) {
				GameInpMacroRead(szValue, bOverWrite);
				continue;
			}

			szValue = labelCheck(szLine, _T("custom"));
			if (szValue) {
				GameInpCustomRead(szValue, bOverWrite);
				continue;
			}
		}
	}

	fclose(h);
	return 0;
}

// Write out the config file for the game-specific inputs
int ConfigGameSave(bool bSave)
{
	if (!bSave) {
		GameInpBlank(0);
		ConfigGameLoad(false);
	}

	FILE* h = fopen(GameConfigName(), _T("w+"));
	if (h == NULL) {
		return 1;
	}

	// Write title
	#ifndef __LIBSNES__
	fprintf(h, _T("// ") _T(APP_TITLE) _T(" v%s --- Config File for %s (%hs)\n\n"),
		szAppBurnVer, BurnDrvGetText(DRV_NAME), BurnDrvGetTextA(DRV_FULLNAME));
	#endif

	fprintf(h, "// --- Miscellaneous ----------------------------------------------------------\n\n");
	// Write version number
	fprintf(h, "version 0x%06X\n\n", nBurnVer);
	// Write speed for relative analog controls
	fprintf(h, "analog  0x%04X\n", nAnalogSpeed);
	// Write CPU speed adjustment
	fprintf(h, "cpu     0x%04X\n", nBurnCPUSpeedAdjust);

	// Write autofire delay
#ifndef NO_AUTOFIRE
	fprintf(h, "\n");
	fprintf(h, "autofire-delay %d\n", autofireDelay);
#endif

#if 0
	// Write screen visible size
	fprintf(h, _T("\n"));
	fprintf(h, _T("top     %d\n"), nScrnVisibleOffset[0]);
	fprintf(h, _T("left    %d\n"), nScrnVisibleOffset[1]);
	fprintf(h, _T("bottom  %d\n"), nScrnVisibleOffset[2]);
	fprintf(h, _T("right   %d\n"), nScrnVisibleOffset[3]);
#endif

	fprintf(h, "\n\n");
	fprintf(h, "// --- Inputs -----------------------------------------------------------------\n\n");

	GameInpWrite(h);

	fclose(h);
	return 0;
}
