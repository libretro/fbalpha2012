// Burner Config for Game file module
#include "burner.h"

const int nConfigMinVersion = 0x020921;

bool bSaveInputs = true;

static TCHAR* GameConfigName()
{
	// Return the path of the config file for this game
	static TCHAR szName[32];
	sprintf(szName, "config\\games\\%s.ini"), BurnDrvGetText(DRV_NAME);
	return szName;
}

// Read in the config file for the game-specific inputs
int ConfigGameLoad(bool bOverWrite)
{
	TCHAR szLine[256];
	int nFileVersion = 0;

	FILE* h = fopen(GameConfigName(), "rt");
	if (h == NULL)
		return 1;

	if (bOverWrite) {
		nAnalogSpeed = 0x0100;
		nBurnCPUSpeedAdjust = 0x0100;
	}

	// Go through each line of the config file and process inputs
	while (fgets(szLine, sizeof(szLine), h)) {
		TCHAR *szValue;
		int nLen = strlen(szLine);

		// Get rid of the linefeed at the end
		if (szLine[nLen - 1] == 10) {
			szLine[nLen - 1] = 0;
			nLen--;
		}

		szValue = LabelCheck(szLine, "version");
		if (szValue) {
			nFileVersion = strtol(szValue, NULL, 0);
		}

		if (bOverWrite) {
			szValue = LabelCheck(szLine, "analog");
			if (szValue) {
				nAnalogSpeed = strtol(szValue, NULL, 0);
			}
			szValue = LabelCheck(szLine, "cpu");
			if (szValue) {
				nBurnCPUSpeedAdjust = strtol(szValue, NULL, 0);
			}
		}

		if (nConfigMinVersion <= nFileVersion && nFileVersion <= nBurnVer) {
			szValue = LabelCheck(szLine, "input");
			if (szValue) {
				GameInpRead(szValue, bOverWrite);
				continue;
			}

			szValue = LabelCheck(szLine, "macro");
			if (szValue) {
				GameInpMacroRead(szValue, bOverWrite);
				continue;
			}

			szValue = LabelCheck(szLine, "custom");
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
	FILE* h;

	if (!bSave) {
		GameInpBlank(0);
		ConfigGameLoad(false);
	}

	h = fopen(GameConfigName(), "wt");
	if (h == NULL)
		return 1;

	// Write title
	#ifndef __LIBSNES__
	fprintf(h, _T("// ") _T(APP_TITLE) _T(" v%s --- Config File for %s (%s)\n\n"), szAppBurnVer, BurnDrvGetText(DRV_NAME), ANSIToTCHAR(BurnDrvGetTextA(DRV_FULLNAME), NULL, 0));
	#endif

	fprintf(h, "// --- Miscellaneous ----------------------------------------------------------\n\n");
	// Write version number
	fprintf(h, "version 0x%06X\n\n", nBurnVer);
	// Write speed for relative analog controls
	fprintf(h, "analog  0x%04X\n", nAnalogSpeed);
	// Write CPU speed adjustment
	fprintf(h, "cpu     0x%04X\n", nBurnCPUSpeedAdjust);

	fprintf(h, "\n\n\n");
	fprintf(h, "// --- Inputs -----------------------------------------------------------------\n\n");

	GameInpWrite(h);

	fclose(h);
	return 0;
}

