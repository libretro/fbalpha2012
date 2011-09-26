// Burner Config for Game file module
#include "burner.h"

const int nConfigMinVersion = 0x020921;

static TCHAR* GameConfigName()
{
	// Return the path of the config file for this game
	static TCHAR szName[MAX_PATH];
#ifdef _XBOX
	_stprintf(szName, _T("game:\\config\\games\\%S.ini"), BurnDrvGetText(DRV_NAME));
#else
	_stprintf(szName, _T("/dev_hdd0/game/FBAN00000/USRDIR/config/games/%s.ini"), BurnDrvGetTextA(DRV_NAME));
#endif
	return szName;
}

// Read in the config file for the game-specific inputs
int ConfigGameLoad(bool bOverWrite)
{
	TCHAR szLine[MAX_PATH];
	int nFileVersion = 0;

	FILE* h = _tfopen(GameConfigName(), _T("rt"));
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
	while (_fgetts(szLine, sizeof(szLine), h)) {
		TCHAR *szValue;
		size_t nLen = _tcslen(szLine);

		// Get rid of the linefeed at the end
		if (szLine[nLen - 1] == 10) {
			szLine[nLen - 1] = 0;
			nLen--;
		}

		szValue = labelCheck(szLine, _T("version"));
		if (szValue) {
			nFileVersion = _tcstol(szValue, NULL, 0);
		}

		if (bOverWrite) {
			szValue = labelCheck(szLine, _T("analog"));
			if (szValue) {
				nAnalogSpeed = _tcstol(szValue, NULL, 0);
			}
			szValue = labelCheck(szLine, _T("cpu"));
			if (szValue) {
				nBurnCPUSpeedAdjust = _tcstol(szValue, NULL, 0);
			}
#ifndef NO_AUTOFIRE
			szValue = labelCheck(szLine, _T("autofire-delay"));
			if (szValue) {
				autofireDelay = _tcstol(szValue, NULL, 0);
			}
#endif
#if 0
			szValue = labelCheck(szLine, _T("top"));
			if (szValue) {
				nScrnVisibleOffset[0] = _tcstol(szValue, NULL, 0);
			}
			szValue = labelCheck(szLine, _T("left"));
			if (szValue) {
				nScrnVisibleOffset[1] = _tcstol(szValue, NULL, 0);
			}
			szValue = labelCheck(szLine, _T("bottom"));
			if (szValue) {
				nScrnVisibleOffset[2] = _tcstol(szValue, NULL, 0);
			}
			szValue = labelCheck(szLine, _T("right"));
			if (szValue) {
				nScrnVisibleOffset[3] = _tcstol(szValue, NULL, 0);
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

	FILE* h = _tfopen(GameConfigName(), _T("w+"));
	if (h == NULL) {
		return 1;
	}

	// Write title
	_ftprintf(h, _T("// ") _T(APP_TITLE) _T(" v%s --- Config File for %s (%hs)\n\n"),
		szAppBurnVer, BurnDrvGetText(DRV_NAME), BurnDrvGetTextA(DRV_FULLNAME));

	_ftprintf(h, _T("// --- Miscellaneous ----------------------------------------------------------\n\n"));
	// Write version number
	_ftprintf(h, _T("version 0x%06X\n\n"), nBurnVer);
	// Write speed for relative analog controls
	_ftprintf(h, _T("analog  0x%04X\n"), nAnalogSpeed);
	// Write CPU speed adjustment
	_ftprintf(h, _T("cpu     0x%04X\n"), nBurnCPUSpeedAdjust);

	// Write autofire delay
#ifndef NO_AUTOFIRE
	_ftprintf(h, _T("\n"));
	_ftprintf(h, _T("autofire-delay %d\n"), autofireDelay);
#endif

#if 0
	// Write screen visible size
	_ftprintf(h, _T("\n"));
	_ftprintf(h, _T("top     %d\n"), nScrnVisibleOffset[0]);
	_ftprintf(h, _T("left    %d\n"), nScrnVisibleOffset[1]);
	_ftprintf(h, _T("bottom  %d\n"), nScrnVisibleOffset[2]);
	_ftprintf(h, _T("right   %d\n"), nScrnVisibleOffset[3]);
#endif

	_ftprintf(h, _T("\n\n"));
	_ftprintf(h, _T("// --- Inputs -----------------------------------------------------------------\n\n"));

	GameInpWrite(h);

	fclose(h);
	return 0;
}
