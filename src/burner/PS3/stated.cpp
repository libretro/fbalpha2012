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

int StatedLoad(int nSlot)
{
	TCHAR szFilter[1024];
	int nRet;
	int bOldPause;

   #if 0
	if (bDrvOkay == 0) {
		return 1;
	}
   #endif

   #if 0
	if (nSlot)
   {
	   _stprintf(szChoice, _T("%s%s slot %02x.fs"), getMiscPath(PATH_SAVESTATE), BurnDrvGetText(DRV_NAME), nSlot);
	}
   else
   {
   #endif
      sprintf(szChoice, "/dev_hdd0/game/FBAN00000/USRDIR/savestates/%s.%d.fs", BurnDrvGetTextA(DRV_NAME), nSlot);
		//StateMakeOfn(szFilter);

		bOldPause = bRunPause;
		bRunPause = 1;
		 
		bRunPause = bOldPause;
		
	//}

	nRet = BurnStateLoad(szChoice, 1, &DrvInitCallback);
 
	return nRet;
}

int StatedSave(int nSlot)
{
	TCHAR szFilter[1024];
	int nRet;
	int bOldPause;

   #if 0
	if (bDrvOkay == 0) {
		return 1;
	}
   #endif

   #if 0
	if (nSlot)
   {
	   _stprintf(szChoice, _T("%s%s slot %02x.fs"), getMiscPath(PATH_SAVESTATE), BurnDrvGetText(DRV_NAME), nSlot);
	}
   else
   {
   #endif
      sprintf(szChoice, "/dev_hdd0/game/FBAN00000/USRDIR/savestates/%s.%d.fs", BurnDrvGetTextA(DRV_NAME), nSlot);
		//StateMakeOfn(szFilter);
		
		bOldPause = bRunPause;
		bRunPause = 1; 
		bRunPause = bOldPause;		
	//}

	nRet = BurnStateSave(szChoice, 1);
 
	return nRet;
}
