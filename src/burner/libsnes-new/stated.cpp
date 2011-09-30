// State dialog module
#include "burner.h"

int bDrvSaveAll = 0;

static void StateMakeOfn(char* pszFilter)
{

	return;
}

// The automatic save
int StatedAuto(int bSave)
{
	return 0;
}

int StatedLoad(int nSlot)
{
	char szFilter[1024];
	int nRet;
	int bOldPause;

	//FIXME: Fix hardcoded path here
	sprintf(szChoice, "/dev_hdd0/game/FBAN00000/USRDIR/savestates/%s.%d.fs", BurnDrvGetTextA(DRV_NAME), nSlot);

	bOldPause = bRunPause;
	bRunPause = 1;

	bRunPause = bOldPause;


	nRet = BurnStateLoad(szChoice, 1, &DrvInitCallback);

	return nRet;
}

int StatedSave(int nSlot)
{
	char szFilter[1024];
	int nRet;
	int bOldPause;

	//FIXME: Fix hardcoded path here
	sprintf(szChoice, "/dev_hdd0/game/FBAN00000/USRDIR/savestates/%s.%d.fs", BurnDrvGetTextA(DRV_NAME), nSlot);

	bOldPause = bRunPause;
	bRunPause = 1; 
	bRunPause = bOldPause;		

	nRet = BurnStateSave(szChoice, 1);

	return nRet;
}
