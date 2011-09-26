// State dialog module
#include "burner.h"

int bDrvSaveAll = 0;

static void StateMakeOfn(TCHAR* pszFilter)
{
	_stprintf(pszFilter, FBALoadStringEx(IDS_DISK_FILE_STATE), _T(APP_TITLE));
	memcpy(pszFilter + _tcslen(pszFilter), _T(" (*.fs, *.fr)\0*.fs;*.fr\0\0"), 25 * sizeof(TCHAR));

	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hScrnWnd;
	ofn.lpstrFilter = pszFilter;
	ofn.lpstrFile = szChoice;
	ofn.nMaxFile = sizearray(szChoice);
	ofn.lpstrInitialDir = getMiscPath(PATH_SAVESTATE);
	ofn.lpstrTitle = FBALoadStringEx(IDS_STATE_LOAD);
	ofn.Flags = OFN_NOCHANGEDIR | OFN_HIDEREADONLY;
	ofn.lpstrDefExt = _T("fs");
	return;
}

// The automatic save
int StatedAuto(int bSave)
{
	static TCHAR szName[MAX_PATH] = _T("");
	int nRet;

	_stprintf(szName, _T("config\\games\\%s.fs"), BurnDrvGetText(DRV_NAME));

	if (bSave == 0) {
		nRet = BurnStateLoad(szName, bDrvSaveAll, NULL);		// Load ram
		if (nRet && bDrvSaveAll)	{
			nRet = BurnStateLoad(szName, 0, NULL);				// Couldn't get all - okay just try the nvram
		}
	} else {
		nRet = BurnStateSave(szName, bDrvSaveAll);				// Save ram
	}

	return nRet;
}

static void CreateStateName(int nSlot)
{
	// create dir if dir doesn't exist
	if (!directoryExists(getMiscPath(PATH_SAVESTATE))) {
		CreateDirectory(getMiscPath(PATH_SAVESTATE), NULL);
	}

	_stprintf(szChoice, _T("%s%s slot %02x.fs"), getMiscPath(PATH_SAVESTATE), BurnDrvGetText(DRV_NAME), nSlot);
}

int StatedLoad(int nSlot)
{
	TCHAR szFilter[1024];
	int nRet;
	int bOldPause;

	if (bDrvOkay == 0) {
		return 1;
	}

	// if rewinding during playback, and readonly is not set,
	// then transition from decoding to encoding
	if (!bReplayReadOnly && nReplayStatus == 2)
	{
		nReplayStatus = 1;
	}
	if (bReplayReadOnly && nReplayStatus == 1)
	{
		StopReplay();
		nReplayStatus = 2;
	}

	if (nSlot) {
		CreateStateName(nSlot);
	} else {
		if (bDrvOkay) {
			_stprintf(szChoice, _T("%s*.fs"), BurnDrvGetText(DRV_NAME));
		} else {
			_stprintf(szChoice, _T("savestate"));
		}
		StateMakeOfn(szFilter);

		bOldPause = bRunPause;
		bRunPause = 1;
		nRet = GetOpenFileName(&ofn);
		bRunPause = bOldPause;

		if (nRet == 0) {		// Error
			VidSNewShortMsg(FBALoadStringEx(IDS_STATE_LOAD_ERROR), 0xFF3F3F);
			return 1;
		}
	}

	nRet = BurnStateLoad(szChoice, 1, &DrvInitCallback);

	VidSNewShortMsg(FBALoadStringEx(IDS_STATE_LOADED));

	if (nSlot) {
		return nRet;
	}

	// Describe any possible errors:
	if (nRet == 3) {
		FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_DISK_THIS_STATE));
		FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_ERR_DISK_UNAVAIL));
	} else {
		if (nRet == 4) {
			FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_DISK_THIS_STATE));
			FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_ERR_DISK_TOOOLD), _T(APP_TITLE));
		} else {
			if (nRet == 5) {
				FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_DISK_THIS_STATE));
				FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_ERR_DISK_TOONEW), _T(APP_TITLE));
			} else {
				if (nRet && !nSlot) {
					FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_ERR_DISK_LOAD));
					FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_DISK_STATE));
				}
			}
		}
	}

	if (nRet) {
		FBAPopupDisplay(PUF_TYPE_ERROR);
	}

	return nRet;
}

int StatedSave(int nSlot)
{
	TCHAR szFilter[1024];
	int nRet;
	int bOldPause;

	if (bDrvOkay == 0) {
		return 1;
	}

	if (nSlot) {
		CreateStateName(nSlot);
	} else {
		_stprintf(szChoice, _T("%s"), BurnDrvGetText(DRV_NAME));
		StateMakeOfn(szFilter);
		ofn.lpstrTitle = FBALoadStringEx(IDS_STATE_SAVE);
		ofn.Flags |= OFN_OVERWRITEPROMPT;

		bOldPause = bRunPause;
		bRunPause = 1;
		nRet = GetSaveFileName(&ofn);
		bRunPause = bOldPause;

		if (nRet == 0) {		// Error
			VidSNewShortMsg(FBALoadStringEx(IDS_STATE_SAVE_ERROR), 0xFF3F3F);
			return 1;
		}
	}

	nRet = BurnStateSave(szChoice, 1);

	if (nRet && !nSlot) {
		FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_ERR_DISK_CREATE));
		FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_DISK_STATE));
		FBAPopupDisplay(PUF_TYPE_ERROR);
	}

	VidSNewShortMsg(FBALoadStringEx(IDS_STATE_SAVED));

	return nRet;
}
