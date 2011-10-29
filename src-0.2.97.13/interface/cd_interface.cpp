// CD/CD-ROM support
#include "burner.h"

bool bCDEmuOkay = false;
unsigned int nCDEmuSelect = 0;

CDEmuStatusValue CDEmuStatus;

static InterfaceInfo CDEmuInfo = { NULL, NULL, NULL };

#if defined BUILD_WIN32
	extern struct CDEmuDo isowavDo;
#elif defined BUILD_SDL

	// CD emulation module

#endif

static struct CDEmuDo* pCDEmuDo[] =
{
#if defined BUILD_WIN32
	&isowavDo,
#elif defined BUILD_SDL

	// CD emulation module

#endif
};

#define CDEMU_LEN (sizeof(pCDEmuDo) / sizeof(pCDEmuDo[0]))

TCHAR CDEmuImage[MAX_PATH] = _T("");


// ----------------------------------------------------------------------------

int CDEmuExit()
{
	IntInfoFree(&CDEmuInfo);

	if (!bCDEmuOkay || nCDEmuSelect >= CDEMU_LEN) {
		return 1;
	}
	bCDEmuOkay = false;

	return pCDEmuDo[nCDEmuSelect]->CDEmuExit();
}

int CDEmuInit()
{
	int nRet;

	BurnDrvFindMedium(NULL);

	if (nCDEmuSelect >= CDEMU_LEN) {
		return 1;
	}

	CDEmuStatus = idle;

	if ((nRet = pCDEmuDo[nCDEmuSelect]->CDEmuInit()) == 0) {
		bCDEmuOkay = true;
	}

	return nRet;
}

int CDEmuStop()
{
	BurnDrvFindMedium(NULL);

	if (!bCDEmuOkay || nCDEmuSelect >= CDEMU_LEN) {
		return 1;
	}

	return pCDEmuDo[nCDEmuSelect]->CDEmuStop();
}

int CDEmuPlay(unsigned char M, unsigned char S, unsigned char F)
{
	if (!bCDEmuOkay || nCDEmuSelect >= CDEMU_LEN) {
		return 1;
	}

	return pCDEmuDo[nCDEmuSelect]->CDEmuPlay(M, S, F);
}

int CDEmuLoadSector(int LBA, char* pBuffer)
{
	if (!bCDEmuOkay || nCDEmuSelect >= CDEMU_LEN) {
		return 0;
	}

	return pCDEmuDo[nCDEmuSelect]->CDEmuLoadSector(LBA, pBuffer);
}

unsigned char* CDEmuReadTOC(int track)
{
	if (!bCDEmuOkay || nCDEmuSelect >= CDEMU_LEN) {
		return NULL;
	}

	return pCDEmuDo[nCDEmuSelect]->CDEmuReadTOC(track);
}

unsigned char* CDEmuReadQChannel()
{
	if (!bCDEmuOkay || nCDEmuSelect >= CDEMU_LEN) {
		return NULL;
	}

	return pCDEmuDo[nCDEmuSelect]->CDEmuReadQChannel();
}

int CDEmuGetSoundBuffer(short* buffer, int samples)
{
	if (!bCDEmuOkay || nCDEmuSelect >= CDEMU_LEN) {
		return 1;
	}

	return pCDEmuDo[nCDEmuSelect]->CDEmuGetSoundBuffer(buffer, samples);
}

InterfaceInfo* CDEmuGetInfo()
{
	if (IntInfoInit(&CDEmuInfo)) {
		IntInfoFree(&CDEmuInfo);
		return NULL;
	}

	if (bCDEmuOkay) {

		CDEmuInfo.pszModuleName = pCDEmuDo[nCDEmuSelect]->szModuleName;

	 	if (pCDEmuDo[nCDEmuSelect]->GetPluginSettings) {
			pCDEmuDo[nCDEmuSelect]->GetPluginSettings(&CDEmuInfo);
		}
	} else {
		IntInfoAddStringInterface(&CDEmuInfo, _T("CD/CD-ROM emulation module not initialised"));
	}

	return &CDEmuInfo;

	return NULL;
}

// ----------------------------------------------------------------------------
// Support functions

void CDEmuPrintCDName()
{
	TCHAR* pszName;
	
	if ((pszName = BurnDrvGetText(DRV_MEDIUMNAME)) != NULL) {
		dprintf(_T("*** Identified CD as %s"), pszName);
		while ((pszName = BurnDrvGetText(DRV_NEXTNAME | DRV_MEDIUMNAME)) != NULL) {
			dprintf(_T(SEPERATOR_2) _T("%s"), pszName);
		}
		dprintf(_T(".\n"));
	} else {
		dprintf(_T("*** Couldn't identify CD.\n"));
	}
}
