// State dialog module
#include "burner.h"

int bDrvSaveAll = 0;

static void StateMakeOfn(TCHAR* pszFilter)
{

	return;
}

// The automatic save
int StatedAuto(int bSave)
{
#if 0
	static TCHAR szName[MAX_PATH] = _T("");
	int nRet;

	_stprintf(szName, _T("GAME:\\config\\games\\%S.fs"), BurnDrvGetText(DRV_NAME));

	if (bSave == 0) {
		nRet = BurnStateLoad(szName, bDrvSaveAll, NULL);		// Load ram
		if (nRet && bDrvSaveAll)	{
			nRet = BurnStateLoad(szName, 0, NULL);				// Couldn't get all - okay just try the nvram
		}
	} else {
		nRet = BurnStateSave(szName, bDrvSaveAll);				// Save ram
	}

	return nRet;
#endif
	return 0;
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

	if (nSlot) {
		CreateStateName(nSlot);
	} else {
		if (bDrvOkay) {
			_stprintf(szChoice, _T("GAME:\\savestates\\%S.fs"), BurnDrvGetText(DRV_NAME));
		} else {
			_stprintf(szChoice, _T("savestate"));
		}
		StateMakeOfn(szFilter);

		bOldPause = bRunPause;
		bRunPause = 1;
		 
		bRunPause = bOldPause;
		
	}

	nRet = BurnStateLoad(szChoice, 1, &DrvInitCallback);
 
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
		_stprintf(szChoice, _T("GAME:\\savestates\\%S.fs"), BurnDrvGetText(DRV_NAME));
		StateMakeOfn(szFilter);
		
		bOldPause = bRunPause;
		bRunPause = 1; 
		bRunPause = bOldPause;		
	}

	nRet = BurnStateSave(szChoice, 1);
 
	return nRet;
}
