// State dialog module
#include "burner.h"

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
	int ret;

	sprintf(szChoice, "/dev_hdd0/game/FBAN00000/USRDIR/savestates/%s.%d.fs", BurnDrvGetTextA(DRV_NAME), nSlot);

	ret = BurnStateLoad(szChoice, 1, &DrvInitCallback);

	return ret;
}

int StatedSave(int nSlot)
{
	int ret;

	sprintf(szChoice, "/dev_hdd0/game/FBAN00000/USRDIR/savestates/%s.%d.fs", BurnDrvGetTextA(DRV_NAME), nSlot);

	ret = BurnStateSave(szChoice, 1);

	return ret;
}
