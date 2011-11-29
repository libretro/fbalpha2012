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
	char szChoice[MAX_PATH];
	sprintf(szChoice, "%s%s.%d.fs", SAVESTATES_DIR, BurnDrvGetTextA(DRV_NAME), nSlot);
	ret = BurnStateLoad(szChoice, 1, &DrvInitCallback);
	return ret;
}

int StatedSave(int nSlot)
{
	int ret;
	char szChoice[MAX_PATH];
	sprintf(szChoice, "%s%s.%d.fs", SAVESTATES_DIR, BurnDrvGetTextA(DRV_NAME), nSlot);
	ret = BurnStateSave(szChoice, 1);
	return ret;
}
