// Burner archive module
// 7zip support, added by regret

#include "burner.h"
 
#define LOAD_OPT_ROM

static int nBArchiveError = 0;				// non-zero if there is a problem with the opened romset
static char* szBArchiveName[BZIP_MAX] = { NULL, };	// Archive files to search through

struct ROMFIND
{
	FIND_STATE nState;
	int nArchive;
	int nPos;
};

static ROMFIND* RomFind = NULL;
static int nRomCount = 0;
static int nTotalSize = 0;

static ArcEntry* List = NULL;
static int nListCount = 0;				// List of entries for current archive file
static int nCurrentArc = -1;				// Archive which is currently open

// ---------------------------------------------------------------------------

static void BArchiveListFree()
{
	if(List)
	{
		for (int i = 0; i < nListCount; i++)
		{
			free(List[i].szName);
			List[i].szName = NULL;
		}
		free(List);
		List = NULL;
	}

	nListCount = 0;
}

static int RomDescribe(struct BurnRomInfo* pri)
{
	if (!pri)
		return 1;
 
	return 0;
}

static int GetBArchiveError(int nState)
{
	switch (nState)
	{
		case STAT_OK:			// OK
			return 0x0000;
		case STAT_NOFIND:		// Not present
			return 0x0001;
		case STAT_SMALL:		// Incomplete
			return 0x0001;
		default:			// CRC wrong or too large
			return 0x0100;
	}

	return 0x0100;
}

static int CheckRomsBoot()
{
	return 0;
}

// Check the roms to see if the code, graphics etc are complete
static int CheckRoms()
{
	BurnRomInfo ri;
	int state = STAT_NOFIND;
	int error;

	for (int i = 0; i < nRomCount; i++) {
		memset(&ri, 0, sizeof(ri));
		BurnDrvGetRomInfo(&ri, i);			// Find information about the wanted rom

		if (!(ri.nType & BRF_OPT) && (ri.nCrc != 0)) {
			state = RomFind[i].nState;	// Get the state of the rom in the archive file
			error = GetBArchiveError(state);

			if (state == STAT_NOFIND && ri.nType) {	// (A type of 0 means empty slot - no rom)
				char* szName = "Unknown";
				RomDescribe(&ri);
				BurnDrvGetRomName(&szName, i, 0);
				FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_ERR_LOAD_NOTFOUND), szName);
			}

			if (error == 0)
				nBArchiveError |= 0x2000;

			if (ri.nType & BRF_ESS)			// essential rom - without it the game may not run at all
				nBArchiveError |= error << 0;
			if (ri.nType & BRF_PRG)			// rom which contains program information
				nBArchiveError |= error << 1;
			if (ri.nType & BRF_GRA) 		// rom which contains graphics information
				nBArchiveError |= error << 2;
			if (ri.nType & BRF_SND)			// rom which contains sound information
				nBArchiveError |= error << 3;
		}
	}

	if (nBArchiveError & 0x0F0F)
		nBArchiveError |= 0x4000;

	return 0;
}

// ----------------------------------------------------------------------------

static int __cdecl BArchiveBurnLoadRom(unsigned char* Dest, int* pnWrote, int i)
{
	if (i < 0 || i >= nRomCount || !RomFind)
		return 1;

	BurnRomInfo ri;
	memset(&ri, 0, sizeof(ri));
	BurnDrvGetRomInfo(&ri, i); // Get info

	// show what we're doing
	char* pszRomName = NULL;
	BurnDrvGetRomName(&pszRomName, i, 0);
	if (pszRomName == NULL)
		pszRomName = "unknown";

	char szText[MAX_PATH];
	sprintf(szText, "Loading");

	if (ri.nType & (BRF_PRG | BRF_GRA | BRF_SND | BRF_BIOS))
	{
		if (ri.nType & BRF_BIOS)
			sprintf (szText + strlen(szText), " %s", "BIOS ");
		if (ri.nType & BRF_PRG)
			sprintf (szText + strlen(szText), " %s", "program ");
		if (ri.nType & BRF_GRA)
			sprintf (szText + strlen(szText), " %s", "graphics ");
		if (ri.nType & BRF_SND)
			sprintf (szText + strlen(szText), " %s", "sound ");
		sprintf(szText + strlen(szText), "(%hs)...", pszRomName);
	}
	else
		sprintf(szText + strlen(szText), " %hs...", pszRomName);

	ProgressUpdateBurner(ri.nLen ? 1.0 / ((double)nTotalSize / ri.nLen) : 0, szText, 0);

#ifndef LOAD_OPT_ROM
	// skip loading optional rom
	if (ri.nType & BRF_OPT)
		return 0;
#endif

	if (RomFind[i].nState == STAT_NOFIND)	// Rom not found in archive at all
		return 1;

	int nWantZip = RomFind[i].nArchive;	// Which archive file it is in
	if (nCurrentArc != nWantZip) {		// If we haven't got the right archive file currently open
		archiveClose();
		nCurrentArc = -1;

		if (archiveOpen(szBArchiveName[nWantZip]))
			return 1;

		nCurrentArc = nWantZip;
	}

	// Read in file and return how many bytes we read
	if (archiveLoadFile(Dest, ri.nLen, RomFind[i].nPos, pnWrote))
		return 1; // Error loading from the archive file

	return 0;
}

// ----------------------------------------------------------------------------

int BArchiveStatus()
{
	if (!(nBArchiveError & 0x0F0F))
		return BARC_STATUS_OK;

	if (nBArchiveError & 1)
		return BARC_STATUS_ERROR;

	return BARC_STATUS_BADDATA;
}

int BArchiveCheckRoms(const bool& bootApp)
{
	// Check the roms to see if the code, graphics etc are complete
	CheckRoms();

	if (nBArchiveError & 0x2000)
	{
		if (!(nBArchiveError & 0x0F0F))
			FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_ERR_LOAD_OK));
		else
		{
			FBAPopupAddText(PUF_TEXT_DEFAULT, "\n");
			FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_ERR_LOAD_PROBLEM));
		}

		if (nBArchiveError & 0x0101)
		{
			FBAPopupAddText(PUF_TEXT_DEFAULT, _T("\n ") _T(SEPERATOR_1));
			if (nBArchiveError & 0x0001)
				FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_ERR_LOAD_ESS_MISS));
			else
				FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_ERR_LOAD_ESS_BAD));
		}
		if (nBArchiveError & 0x0202)
		{
			FBAPopupAddText(PUF_TEXT_DEFAULT, _T("\n ") _T(SEPERATOR_1));
			FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_ERR_LOAD_DET_PRG));
			if (nBArchiveError & 0x0002)
				FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_ERR_LOAD_DATA_MISS));
			else
				FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_ERR_LOAD_DATA_BAD));
		}
		if (nBArchiveError & 0x0404)
		{
			FBAPopupAddText(PUF_TEXT_DEFAULT, _T("\n ") _T(SEPERATOR_1));
			FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_ERR_LOAD_DET_GRA));
			if (nBArchiveError & 0x0004)
				FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_ERR_LOAD_DATA_MISS));
			else
				FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_ERR_LOAD_DATA_BAD));
		}
		if (nBArchiveError & 0x0808) {
			FBAPopupAddText(PUF_TEXT_DEFAULT, _T("\n ") _T(SEPERATOR_1));
			FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_ERR_LOAD_DET_SND));
			if (nBArchiveError & 0x0008)
				FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_ERR_LOAD_DATA_MISS));
			else
				FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_ERR_LOAD_DATA_BAD));
		}

		// Catch non-categorised ROMs
		if ((nBArchiveError & 0x0F0F) == 0) {
			if (nBArchiveError & 0x0010) {
				FBAPopupAddText(PUF_TEXT_DEFAULT, _T("\n ") _T(SEPERATOR_1));
				if (nBArchiveError & 0x1000)
					FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_ERR_LOAD_DATA_MISS));
				else
					FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_ERR_LOAD_DATA_BAD));
			}
		}
	} else {
		FBAPopupAddText(PUF_TEXT_DEFAULT, _T("\n"));
		FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_ERR_LOAD_NODATA));
	}
	
	BurnExtLoadRom = BArchiveBurnLoadRom; // Okay to call our function to load each rom

	return 0;
}

int BArchiveOpen(bool bootApp)
{
	if (szBArchiveName == NULL)
		return 1;

	BArchiveClose(); // Make sure nothing is open

	// Count the number of roms needed
	for (nRomCount = 0; ; nRomCount++)
	{
		if (BurnDrvGetRomInfo(NULL, nRomCount))
			break;
	}

	if (nRomCount <= 0)
		return 1;

	// Create an array for holding lookups for each rom -> archive entries
	unsigned int nMemLen = nRomCount * sizeof(ROMFIND);
	RomFind = (ROMFIND*)malloc(nMemLen);
	if (RomFind == NULL)
		return 1;

	memset(RomFind, 0, nMemLen);

	// Locate each archive file
	bool bFound = false;
	int checkvalue = ARC_NONE;
	char szFullName[MAX_PATH] = "";
	char* szName = NULL;

	for (int y = 0, z = 0; y < BZIP_MAX && z < BZIP_MAX; y++)
	{
		// Get archive name without extension
		if (BurnDrvGetArchiveName(&szName, y, false))
			break;

		bFound = false;

		for (int d = 0; d < DIRS_MAX; d++)
		{
			if (!_tcsicmp(szAppRomPaths[d], ""))
				continue; // skip empty path

			// check the archived rom file, modified by regret
			sprintf(szFullName, "%s%hs", szAppRomPaths[d], szName);

			checkvalue = archiveCheck(szFullName,  0  );
			if (checkvalue == ARC_NONE)
				continue;

			bFound = true;

			szBArchiveName[z] = (char*)malloc(MAX_PATH * sizeof(char));
			if (!szBArchiveName[z])
				continue;

			strcpy(szBArchiveName[z], szFullName);
			if (!bootApp)
				FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_ERR_LOAD_FOUND), szName, szBArchiveName[z]);
			z++;
		}

		if (!bootApp && !bFound)
			FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_ERR_LOAD_NOTFOUND), szName);
	}

	if (!bootApp)
		FBAPopupAddText(PUF_TEXT_DEFAULT, "\n");

	// Locate the ROM data in the archive files
	int nFind = -1;
	BurnRomInfo ri;

	for (int z = 0; z < BZIP_MAX; z++)
	{
		if (!szBArchiveName[z])
			continue;

		if (archiveOpen(szBArchiveName[z]))
			continue;

		archiveGetList(&List, &nListCount);	// Get the list of entries

		nCurrentArc = z;

		for (int i = 0; i < nRomCount; i++)
		{
			if (RomFind[i].nState == STAT_OK)
				continue;				// Already found this and it's okay

			nFind = findRom(i, List, nListCount);
			if (nFind < 0)					// Couldn't find this rom at all
				continue;

			RomFind[i].nArchive = z;			// Remember which archive file it is in
			RomFind[i].nPos = nFind;
			RomFind[i].nState = STAT_OK;			// Set to found okay

			memset(&ri, 0, sizeof(ri));
			BurnDrvGetRomInfo(&ri, i);		// Get info about the rom

			// if size good & nodump, try to load the file with correct filename
			if (!(ri.nType & BRF_OPT) && (ri.nCrc != 0))
				nTotalSize += ri.nLen;

			if (List[nFind].nLen == ri.nLen)
			{
				if (ri.nCrc)
				{									// If we know the CRC
					if (List[nFind].nCrc != ri.nCrc)		// Length okay, but CRC wrong
						RomFind[i].nState = STAT_CRC;
				}
			}
			else
			{
				if (List[nFind].nLen < ri.nLen)
					RomFind[i].nState = STAT_SMALL;			// Too small
				else
					RomFind[i].nState = STAT_LARGE;			// Too big
			}

			if (!bootApp)
			{
				if (RomFind[i].nState != STAT_OK)
					RomDescribe(&ri);
			}
		}

		archiveClose();

		BArchiveListFree(); // Close the last archive file if open
		nCurrentArc = -1;
	}

	return BArchiveCheckRoms(bootApp);
}

int BArchiveClose()
{
	archiveClose();
	nCurrentArc = -1;		// Close the last archive file if open

	BurnExtLoadRom = NULL;	// Can't call our function to load each rom anymore
	nBArchiveError = 0;		// reset romset errors
	nTotalSize = 0;

	if (RomFind)
	{
		free(RomFind);
		RomFind = NULL;
	}
	nRomCount = 0;

	for (int z = 0; z < BZIP_MAX; z++)
	{
		free(szBArchiveName[z]);
		szBArchiveName[z] = NULL;
	}

	return 0;
}
