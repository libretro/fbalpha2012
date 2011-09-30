#ifndef SN_TARGET_PS3
// Burner DipSwitches Dialog module
#include "burner.h"

static unsigned char nPrevDIPSettings[4];
static unsigned int nDIPGroup;
static int nDIPOffset;
static bool bOK;

static void InpDIPSWGetOffset()
{
	BurnDIPInfo bdi;
	nDIPOffset = 0;
	for (int i = 0; BurnDrvGetDIPInfo(&bdi, i) == 0; i++)
	{
		if (bdi.nFlags == 0xF0)
		{
			nDIPOffset = bdi.nInput;
			break;
		}
	}
}

void InpDIPSWResetDIPs()
{
	int i = 0;
	BurnDIPInfo bdi;
	struct GameInp* pgi = NULL;

	InpDIPSWGetOffset();

	while (BurnDrvGetDIPInfo(&bdi, i) == 0)
	{
		if (bdi.nFlags == 0xFF)
		{
			pgi = GameInp + bdi.nInput + nDIPOffset;
			if (pgi)
				pgi->Input.Constant.nConst = (pgi->Input.Constant.nConst & ~bdi.nMask) | (bdi.nSetting & bdi.nMask);
		}
		i++;
	}
}

bool CheckSetting(int i)
{
	BurnDIPInfo bdi;
	BurnDrvGetDIPInfo(&bdi, i);
	struct GameInp* pgi = GameInp + bdi.nInput + nDIPOffset;

	if (!pgi)
		return false;

	if ((pgi->Input.Constant.nConst & bdi.nMask) == bdi.nSetting)
	{
		unsigned char nFlags = bdi.nFlags;
		if ((nFlags & 0x0F) <= 1)
			return true;
		else
		{
			for (int j = 1; j < (nFlags & 0x0F); j++)
			{
				BurnDrvGetDIPInfo(&bdi, i + j);
				pgi = GameInp + bdi.nInput + nDIPOffset;
				if (nFlags & 0x80)
				{
					if ((pgi->Input.Constant.nConst & bdi.nMask) == bdi.nSetting)
						return false;
				}
				else
				{
					if ((pgi->Input.Constant.nConst & bdi.nMask) != bdi.nSetting)
						return false;
				}
			}
			return true;
		}
	}
	return false;
}


// Make a list view of the DIPswitches
// do not refresh list every time, modified by regret
static int InpDIPSWListMake(BOOL bBuild)
{
	return 0;
}

static int InpDIPSWInit()
{
	return 0;
}

static int InpDIPSWExit()
{
	hInpDIPSWList = NULL;
	hInpDIPSWDlg = NULL;

	if (!bAltPause && bRunPause)
		bRunPause = 0;
	GameInpCheckMouse();
	return 0;
}

static void InpDIPSWCancel()
{
	int i = 0, j = 0;
	BurnDIPInfo bdi;
	struct GameInp* pgi = NULL;

	while (BurnDrvGetDIPInfo(&bdi, i) == 0)
	{
		if (bdi.nInput >= 0 && bdi.nFlags == 0xFF)
		{
			pgi = GameInp + bdi.nInput + nDIPOffset;
			if (pgi)
			{
				pgi->Input.Constant.nConst = nPrevDIPSettings[j];
				j++;
			}
		}
		i++;
	}
}

// Create the list of possible values for a DIPswitch
static void InpDIPSWSelect()
{
}

static void DIPSChanged(const int& id)
{
}
 
int InpDIPSWCreate()
{
	return 0;
}
#endif
