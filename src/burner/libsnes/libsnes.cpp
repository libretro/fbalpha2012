#define MAX_PATH 260

#define PUF_TYPE_ERROR			(1)
#define PUF_TYPE_WARNING		(2)
#define PUF_TYPE_INFO			(3)
#define PUF_TYPE_LOGONLY		(8)
#define PUF_TEXT_TRANSLATE		(1 << 16)
#define PUF_TEXT_NO_TRANSLATE	(0)
#define PUF_TEXT_DEFAULT		(PUF_TEXT_TRANSLATE)

#define AUDIO_SEGMENT_LENGTH 801
#define AUDIO_SEGMENT_LENGTH_TIMES_CHANNELS 1602

#include "libsnes.hpp"
#include "../burner.h"
#include "../gameinp.h"
#include "../win32/resource.h"
#include "../win32/resource_string.h"

#include <stdio.h>

static snes_video_refresh_t video_cb = NULL;
static snes_audio_sample_t audio_cb = NULL;
static snes_input_poll_t poll_cb = NULL;
static snes_input_state_t input_cb = NULL;

//global (static) variables for FBA
static int nAudSampleRate = 48000;
static int nAudSegCount = 6;
static int nAudSegLen = 0;
int bDrvOkay = 0;
static bool bInputOkay = false;
static bool bAudOkay = false;
static bool bAudPlaying = false;
static bool bVidOkay = false;
static bool bAltPause = false;
static int bRunPause = 0;
static int nAppVirtualFps = 6000;
int16_t * pAudNextSound = NULL;
int nAudAllocSegLen = 0;		// Allocated seg length in samples
static unsigned char * pVidTransImage = NULL;
static unsigned int * pVidTransPalette = NULL;
static const int transPaletteSize = 65536;
extern unsigned int nCurrentFrame;
extern bool DoReset;

unsigned char* pVidImage = NULL;				// Memory buffer
int nVidImageWidth = DEFAULT_IMAGE_WIDTH;		// Memory buffer size
int nVidImageHeight = DEFAULT_IMAGE_HEIGHT;		//
int nVidImageLeft = 0, nVidImageTop = 0;		// Memory buffer visible area offsets
int nVidImagePitch = 0, nVidImageBPP = 0;		// Memory buffer pitch and bytes per pixel
int nVidImageDepth = 0;							// Memory buffer bits per pixel
static int nGameImageWidth;
static int nGameImageHeight;
static bool bVidRecalcPalette;
static int nBaseFps;
static bool bSaveRAM = false;

static int nBArchiveError = 0;						// non-zero if there is a problem with the opened romset
static char * szBArchiveName[BZIP_MAX] = { NULL, };	// Archive files to search through
static int nRomCount = 0;

enum FIND_STATE { STAT_NOFIND = 0, STAT_OK, STAT_CRC, STAT_SMALL, STAT_LARGE};

struct ROMFIND { FIND_STATE nState; int nArchive; int nPos; };

static ROMFIND* RomFind = NULL;
static ArcEntry* List = NULL;
static int nListCount = 0;							// List of entries for current archive file
static int nTotalSize = 0;
static int nCurrentArc = -1;						// Archive which is currently open

int ArcadeJoystick = 0;
unsigned int JukeboxSoundCommand = 0;
unsigned int JukeboxSoundLatch = 0;

static char g_rom_path[1024];
static char g_rom_dir[1024];

#define ARGB(r, g, b) ((r << 16) | (g << 8) | b)

static int RomDescribe(struct BurnRomInfo* pri)
{
	if (!pri)
		return 1;
 
	return 0;
}

int BArchiveStatus()
{
   return true;
}

static int BArchiveBurnLoadRom(unsigned char* Dest, int* pnWrote, int i)
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
		sprintf(szText + strlen(szText), "(%s)...", pszRomName);
	}
	else
		sprintf(szText + strlen(szText), " %s...", pszRomName);

	//ProgressUpdateBurner(ri.nLen ? 1.0 / ((double)nTotalSize / ri.nLen) : 0, szText, 0);

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
				//FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_ERR_LOAD_NOTFOUND), szName);
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

static int CheckRomsBoot()
{
#ifdef USE_OLD_AUDIT
	int ret = AUDIT_FULLPASS;
	BurnRomInfo ri;
	int state = STAT_NOFIND;

	for (int i = 0; i < nRomCount; i++)
	{
		memset(&ri, 0, sizeof(ri));
		BurnDrvGetRomInfo(&ri, i);			// Find information about the wanted rom

		state = RomFind[i].nState;		// Get the state of the rom in the archive file
		if (state != STAT_OK && ri.nType)
		{
			if (ri.nCrc == 0)
				continue; // no_dump

			if (!(ri.nType & BRF_OPT))
				return AUDIT_FAIL;

			ret = AUDIT_PARTPASS;
		}
	}

	return ret;

#else
	return 0;
#endif
}

int BArchiveCheckRoms(const bool& bootApp)
{
#ifdef USE_OLD_AUDIT
	if (bootApp)
		return CheckRomsBoot();
#endif

	// Check the roms to see if the code, graphics etc are complete
	CheckRoms();

	#if 0
	if (nBArchiveError & 0x2000) {
		if (!(nBArchiveError & 0x0F0F)) {
			FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_ERR_LOAD_OK));
		} else {
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
		if (nBArchiveError & 0x0202) {
			FBAPopupAddText(PUF_TEXT_DEFAULT, _T("\n ") _T(SEPERATOR_1));
			FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_ERR_LOAD_DET_PRG));
			if (nBArchiveError & 0x0002)
				FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_ERR_LOAD_DATA_MISS));
			else
				FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_ERR_LOAD_DATA_BAD));
		}
		if (nBArchiveError & 0x0404) {
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
	#endif
	
	BurnExtLoadRom = BArchiveBurnLoadRom; // Okay to call our function to load each rom

	return 0;
}

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

static inline int findRomByName(const char* name, ArcEntry* list, int count)
{
	if (!name || !list)
		return -1;

	// Find the rom named name in the List
	int i = 0;
	do
	{
		if (list->szName && !strcasecmp(name, getFilenameA(list->szName)))
			return i;
		i++;
		list++;
	}while(i < count);
	return -1; // couldn't find the rom
}

static inline int findRomByCrc(unsigned int crc, ArcEntry* list, int count)
{
	if (!list)
		return -1;

	// Find the rom named name in the List
	int i = 0;
	do
	{
		if (crc == list->nCrc)
			return i;
		i++;
		list++;
	}while(i < count);

	return -1; // couldn't find the rom
}

// Find rom number i from the pBzipDriver game
int findRom(int i, ArcEntry* list, int count)
{
	BurnRomInfo ri;
	memset(&ri, 0, sizeof(ri));

	int nRet = BurnDrvGetRomInfo(&ri, i);
	if (nRet != 0) // Failure: no such rom
		return -2;

	if (ri.nCrc)   // Search by crc first
	{
		nRet = findRomByCrc(ri.nCrc, list, count);
		if (nRet >= 0)
			return nRet;
	}

	int nAka = 0;
	do
	{	// Failing that, search for possible names
		char* szPossibleName = NULL;
		nRet = BurnDrvGetRomName(&szPossibleName, i, nAka);

		if (nRet) // No more rom names
			break;

		nRet = findRomByName(szPossibleName, list, count);

		if (nRet >= 0)
			return nRet;

		nAka++;
	}while(nAka < 0x10000);

	return -1; // Couldn't find the rom
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

	for (int y = 0, z = 0; y < BZIP_MAX && z < BZIP_MAX; y++) {
		// Get archive name without extension
		if (BurnDrvGetArchiveName(&szName, y, false))
			break;

		bFound = false;

		//for (int d = 0; d < DIRS_MAX; d++)
		for (int d = 0; d < 1; d++) // Only one directory :s
		{
			//if (!strcasecmp(szAppRomPaths[d], ""))
			//	continue; // skip empty path

			// check the archived rom file, modified by regret
			//sprintf(szFullName, "%s%hs", szAppRomPaths[d], szName);
         snprintf(szFullName, sizeof(szFullName), "%s/%s", g_rom_dir, szName);

			checkvalue = archiveCheck(szFullName,  0  );
			if (checkvalue == ARC_NONE)
				continue;

			bFound = true;

			szBArchiveName[z] = (char*)malloc(MAX_PATH * sizeof(char));
			if (!szBArchiveName[z])
				continue;

			strcpy(szBArchiveName[z], szFullName);
			#if 0
			if (!bootApp)
				FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_ERR_LOAD_FOUND), szName, szBArchiveName[z]);
			#endif
			z++;
		}

		#if 0
		if (!bootApp && !bFound)
			FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_ERR_LOAD_NOTFOUND), szName);
		#endif
	}

	#if 0
	if (!bootApp)
		FBAPopupAddText(PUF_TEXT_DEFAULT, "\n");
	#endif

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

	if (RomFind) {
		free(RomFind);
		RomFind = NULL;
	}
	nRomCount = 0;

	for (int z = 0; z < BZIP_MAX; z++) {
		free(szBArchiveName[z]);
		szBArchiveName[z] = NULL;
	}

	return 0;
}

unsigned snes_library_revision_major(void)
{
   return 1;
}

unsigned snes_library_revision_minor(void)
{
   return 0;
}

const char *snes_library_id(void)
{
   return "FBANext";
}

void snes_set_video_refresh(snes_video_refresh_t cb)
{
   video_cb = cb;
}

void snes_set_audio_sample(snes_audio_sample_t cb)
{
   audio_cb = cb;
}

void snes_set_input_poll(snes_input_poll_t cb)
{
   poll_cb = cb;
}

void snes_set_input_state(snes_input_state_t cb)
{
   input_cb = cb;
}

void snes_set_controller_port_device(bool, unsigned)
{}

void snes_set_cartridge_basename(const char *path)
{
   snprintf(g_rom_path, sizeof(g_rom_path), "%s.zip", path);
   strcpy(g_rom_dir, g_rom_path);
   char *ptr = strrchr(g_rom_dir, '/');
   if (!ptr) ptr = strrchr(g_rom_dir, '\\');
   if (ptr) *ptr = '\0';
}

static uint8_t *state_buf = NULL;

void snes_init(void)
{
   state_buf = new uint8_t[2000000];
}

static unsigned serialize_size = 0;

static void configAppLoadXml()
{}

static int AudWriteSilence(void)
{
	if(pAudNextSound)
		memset(pAudNextSound, 0, nAudAllocSegLen);
	return 0;
}

static int audioInit()
{
	bAudOkay = true;
	return 0;
}

#if 0
static bool lock(unsigned int *&data, unsigned &pitch)
{
	pitch = nGameImageWidth * sizeof(unsigned int);
	return data = buffer;
}
#endif

#define VidSCopyImage32(dst_ori) \
   register uint16_t lineSize = nVidImageWidth << 2; \
   uint16_t height = nVidImageHeight; \
   uint8_t * dst = (uint8_t *)dst_ori; \
   do{ \
      height--; \
      memcpy(dst, ps, lineSize); \
      ps += s; \
      dst += pitch; \
   }while(height);

#if 0
static void VidCopyFrame(void)
{
	unsigned int* pd;
	unsigned int pitch;
	lock(pd, pitch);
	uint8_t * ps = pVidImage + (nVidImageLeft << 2);
	int s = nVidImageWidth << 2;
	VidSCopyImage32(pd);
	unsigned int inwidth = nGameImageWidth;
	unsigned int inheight = nGameImageHeight;

}
#endif

static void VidFrame(void)
{
	if (pVidTransImage)
	{
		uint16_t * pSrc = (uint16_t *)pVidTransImage;
		uint8_t * pDest = pVidImage;
		if (bVidRecalcPalette)
		{
			uint64_t r = 0;
			do{
				uint64_t g = 0;
				do{
					uint64_t b = 0;
					do{
						uint64_t r_ = r | (r >> 5);
						uint64_t g_ = g | (g >> 5);
						uint64_t b_ = b | (b >> 5);
						pVidTransPalette[(r << 7) | (g << 2) | (b >> 3)] = ARGB(r_,g_,b_);
						b += 8;
					}while(b < 256);
					g += 8;
				}while(g < 256);
				r += 8;
			}while(r < 256);
			bVidRecalcPalette = false;
		}
		pBurnDraw = pVidTransImage;
		nBurnPitch = nVidImageWidth << 1;
		BurnDrvFrame();
		//_psglRender();
		/* set avi buffer, modified by regret */
		pBurnDraw = NULL;
		nBurnPitch = 0;
		int y = 0;
		do{
			int x = 0;
			do{
				((uint32_t *)pDest)[x] = pVidTransPalette[pSrc[x]];
				x++;
			}while(x < nVidImageWidth);
			y++;
			pSrc += nVidImageWidth;
			pDest += nVidImagePitch;
		}while(y < nVidImageHeight);
	}
	else
	{
		pBurnDraw = pVidImage;
		nBurnPitch = nVidImagePitch;
		BurnDrvFrame();
		//_psglRender();
		/* set avi buffer, modified by regret */
		pBurnDraw = NULL;
		nBurnPitch = 0;
	}
}

static int VidExit()
{
	if (!bVidOkay)
		return 1;

	bAudOkay = false;
	bAudPlaying = false;

	bVidOkay = false;

	nVidImageWidth = DEFAULT_IMAGE_WIDTH;
	nVidImageHeight = DEFAULT_IMAGE_HEIGHT;

	nVidImageBPP = nVidImageDepth = 0;
	nBurnPitch = nBurnBpp = 0;

	free(pVidTransPalette);
	pVidTransPalette = NULL;
	free(pVidTransImage);
	pVidTransImage = NULL;

	return 1;
}

static unsigned int HighCol15(int r, int g, int b, int  /* i */)
{
	unsigned int t;
	t  = (r << 7) & 0x7C00;
	t |= (g << 2) & 0x03E0;
	t |= (b >> 3) & 0x001F;
	return t;
}

static int VidInit()
{
	VidExit(); 

	int nRet = 1;

	if (bDrvOkay)
	{
		nBurnBpp = nVidImageBPP; // Set Burn library Bytes per pixel
		bVidOkay = true;

		if (bDrvOkay && (BurnDrvGetFlags() & BDF_16BIT_ONLY) && nVidImageBPP > 2)
		{
			nBurnBpp = 2;

			pVidTransPalette = (unsigned int*)malloc(transPaletteSize * sizeof(int));
			pVidTransImage = (unsigned char*)malloc(nVidImageWidth * nVidImageHeight * (nVidImageBPP >> 1) * sizeof(short));

			BurnHighCol = HighCol15;

			if (pVidTransPalette == NULL || pVidTransImage == NULL)
			{
				VidExit();
				nRet = 1;
			}
		}
	}

	return nRet;
}

static int InputInit(void)
{
	bInputOkay = true;
	return bInputOkay;
}

static int mediaInit()
{
	if (!bInputOkay)
		InputInit(); // Init Input

	nBaseFps = nBurnFPS;
	nAppVirtualFps = nBurnFPS;

	if (!bAudOkay)
	{
		audioInit();					// Init Sound (not critical if it fails)

		if (!bAudOkay)
		{
			// Make sure the error will be visible
		}
	}

	// Assume no sound
	nBurnSoundRate = 0;					
	pBurnSoundOut = NULL;

	if (bAudOkay)
	{
		nBurnSoundRate = nAudSampleRate;
		nBurnSoundLen = nAudSegLen;
	}

	if (!bVidOkay)
	{
		VidInit(); // Reinit the video plugin

		#if 0
		if (bVidOkay && ((bRunPause && bAltPause) || !bDrvOkay))
			VidFrame();
		#endif
	}

	return 0;
}

static void VidReinit()
{
	VidInit();

	if (bRunPause || !bDrvOkay)
		VidFrame();
}

static void scrnReinit()
{
	VidReinit();
}

static int CinpState(int nCode)
{
	if (nCode < 0)
		return 0;

	if (DoReset)
	{

		#if 0
		if (nCode == FBK_F3)
		{
			DoReset = false;
			return 1;
		}
		#endif

	}

	#if 0
	switch (nCode)
	{
		case P1_COIN:
			return CTRL_SELECT(new_state_p1);
		case P1_START:
			return CTRL_START(new_state_p1);
		case P1_UP:
			return ((CTRL_UP(new_state_p1) | CTRL_LSTICK_UP(new_state_p1)) != 0);
		case P1_DOWN: 
			return ((CTRL_DOWN(new_state_p1) | CTRL_LSTICK_DOWN(new_state_p1)) != 0);
		case P1_LEFT:
			return ((CTRL_LEFT(new_state_p1) | CTRL_LSTICK_LEFT(new_state_p1)) != 0);
		case P1_RIGHT:
			return ((CTRL_RIGHT(new_state_p1) | CTRL_LSTICK_RIGHT(new_state_p1)) != 0);
		case P1_FIRE1:
			return CTRL_CROSS(new_state_p1);
		case P1_FIRE2:
			return CTRL_CIRCLE(new_state_p1);
		case P1_FIRE3:
			return CTRL_SQUARE(new_state_p1);
		case P1_FIRE4: 
			return CTRL_TRIANGLE(new_state_p1);
		case P1_FIRE5:
			return CTRL_L1(new_state_p1);
		case P1_FIRE6:
			return CTRL_R1(new_state_p1);
		case 0x88:
			return CTRL_L2(new_state_p1);
		case 0x8A:			 
			return CTRL_R2(new_state_p1);
		case 0x3b:
			return CTRL_L3(new_state_p1);
		case P1_SERVICE:
			return CTRL_R3(new_state_p1);
		case 0x21:
			return CTRL_R2(new_state_p1);
		default:
			break;
	}

	if (numPadsConnected > 1)
	{
		uint64_t new_state_p2 = cell_pad_input_poll_device(1);

		switch (nCode)
		{
			case P2_COIN:
				return CTRL_SELECT(new_state_p2);
			case P2_START:
				return CTRL_START(new_state_p2);
			case P2_UP:
				return ((CTRL_UP(new_state_p2) | CTRL_LSTICK_UP(new_state_p2)) != 0);
			case P2_DOWN:
				return ((CTRL_DOWN(new_state_p2) | CTRL_LSTICK_DOWN(new_state_p2)) != 0);
			case P2_LEFT:
				return ((CTRL_LEFT(new_state_p2) | CTRL_LSTICK_LEFT(new_state_p2)) != 0);
			case P2_RIGHT:
				return ((CTRL_RIGHT(new_state_p2) | CTRL_LSTICK_RIGHT(new_state_p2)) != 0);
			case P2_FIRE1:
				return CTRL_CROSS(new_state_p2);
			case P2_FIRE2:
				return CTRL_CIRCLE(new_state_p2);
			case P2_FIRE3:
				return CTRL_SQUARE(new_state_p2);
			case P2_FIRE4: 
				return CTRL_TRIANGLE(new_state_p2);
			case P2_FIRE5:
				return CTRL_L1(new_state_p2);
			case P2_FIRE6:
				return CTRL_R1(new_state_p2);
			case 0x4088:
				return CTRL_L2(new_state_p2);
			case 0x408A:			 
				return CTRL_R2(new_state_p2);
			case 0x408b:
				return CTRL_L3(new_state_p2);
			case 0x408c:
				return CTRL_R3(new_state_p2);
		}
	}


	if (numPadsConnected > 2)
	{
		uint64_t new_state_p3 = cell_pad_input_poll_device(2);

		switch (nCode)
		{ 
			case P3_COIN:
				return CTRL_SELECT(new_state_p3);
			case P3_START:
				return CTRL_START(new_state_p3);
			case P3_UP:
				return ((CTRL_UP(new_state_p3) | CTRL_LSTICK_UP(new_state_p3)) != 0);
			case P3_DOWN:
				return ((CTRL_DOWN(new_state_p3) | CTRL_LSTICK_DOWN(new_state_p3)) != 0);
			case P3_LEFT:
				return ((CTRL_LEFT(new_state_p3) | CTRL_LSTICK_LEFT(new_state_p3)) != 0);
			case P3_RIGHT:
				return ((CTRL_RIGHT(new_state_p3) | CTRL_LSTICK_RIGHT(new_state_p3)) != 0);
			case P3_FIRE1:
				return CTRL_CROSS(new_state_p3);
			case P3_FIRE2:
				return CTRL_CIRCLE(new_state_p3);
			case P3_FIRE3:
				return CTRL_SQUARE(new_state_p3);
			case P3_FIRE4:
				return CTRL_TRIANGLE(new_state_p3);
			case P3_FIRE5:
				return CTRL_L1(new_state_p3);
			case P3_FIRE6:
				return CTRL_R1(new_state_p3);
			case 0x4188:
				return CTRL_L2(new_state_p3);
			case 0x418A:			 
				return CTRL_R2(new_state_p3);
			case 0x418b:
				return CTRL_L3(new_state_p3);
			case 0x418c:
				return CTRL_R3(new_state_p3);
		}
	}

	if (numPadsConnected > 3)
	{
		uint64_t new_state_p4 = cell_pad_input_poll_device(3);

		switch (nCode)
		{
			case P4_COIN:
				return CTRL_SELECT(new_state_p4);
			case P4_START:
				return CTRL_START(new_state_p4);
			case P4_UP:
				return ((CTRL_UP(new_state_p4) | CTRL_LSTICK_UP(new_state_p4)) != 0);
			case P4_DOWN:
				return ((CTRL_DOWN(new_state_p4) | CTRL_LSTICK_DOWN(new_state_p4)) != 0);
			case P4_LEFT:
				return ((CTRL_LEFT(new_state_p4) | CTRL_LSTICK_LEFT(new_state_p4)) != 0);
			case P4_RIGHT:
				return ((CTRL_RIGHT(new_state_p4) | CTRL_LSTICK_RIGHT(new_state_p4)) != 0);
			case P4_FIRE1:
				return CTRL_CROSS(new_state_p4);
			case P4_FIRE2:
				return CTRL_CIRCLE(new_state_p4);
			case P4_FIRE3:
				return CTRL_SQUARE(new_state_p4);
			case P4_FIRE4:
				return CTRL_TRIANGLE(new_state_p4);
			case P4_FIRE5:
				return CTRL_L1(new_state_p4);
			case P4_FIRE6:
				return CTRL_R1(new_state_p4);
			case 0x4288:
				return CTRL_L2(new_state_p4);
			case 0x428A:			 
				return CTRL_R2(new_state_p4);
			case 0x428b:
				return CTRL_L3(new_state_p4);
			case 0x428c:
				return CTRL_R3(new_state_p4);

		} 
	}
	#endif

	return 0;
}

// This will process all PC-side inputs and optionally update the emulated game side.
static void InputMake(void)
{
	struct GameInp* pgi;
	unsigned int i;

	// Do one frames worth of keyboard input sliders
	// Begin of InputTick()

	for (i = 0, pgi = GameInp; i < nGameInpCount; i++, pgi++)
	{
		int nAdd = 0;
		if ((pgi->nInput &  GIT_GROUP_SLIDER) == 0) // not a slider
			continue;

		if (pgi->nInput == GIT_KEYSLIDER)
		{
			// Get states of the two keys
			if (CinpState(pgi->Input.Slider.SliderAxis.nSlider[0]))
				nAdd -= 0x100;
			if (CinpState(pgi->Input.Slider.SliderAxis.nSlider[1]))
				nAdd += 0x100;
		}

		// nAdd is now -0x100 to +0x100

		// Change to slider speed
		nAdd *= pgi->Input.Slider.nSliderSpeed;
		nAdd /= 0x100;

		if (pgi->Input.Slider.nSliderCenter)
		{ // Attact to center
			int v = pgi->Input.Slider.nSliderValue - 0x8000;
			v *= (pgi->Input.Slider.nSliderCenter - 1);
			v /= pgi->Input.Slider.nSliderCenter;
			v += 0x8000;
			pgi->Input.Slider.nSliderValue = v;
		}

		pgi->Input.Slider.nSliderValue += nAdd;
		// Limit slider
		if (pgi->Input.Slider.nSliderValue < 0x0100)
			pgi->Input.Slider.nSliderValue = 0x0100;
		if (pgi->Input.Slider.nSliderValue > 0xFF00)
			pgi->Input.Slider.nSliderValue = 0xFF00;
	}
	// End of InputTick()

	for (i = 0, pgi = GameInp; i < nGameInpCount; i++, pgi++)
	{
		if (pgi->Input.pVal == NULL)
			continue;

		switch (pgi->nInput)
		{
			case 0: // Undefined
				pgi->Input.nVal = 0;
				break;
			case GIT_CONSTANT: // Constant value
				pgi->Input.nVal = pgi->Input.Constant.nConst;
				*(pgi->Input.pVal) = pgi->Input.nVal;
				break;
			case GIT_SWITCH:
				{ // Digital input
					int s = CinpState(pgi->Input.Switch.nCode);

					if (pgi->nType & BIT_GROUP_ANALOG)
					{
						// Set analog controls to full
						if (s)
							pgi->Input.nVal = 0xFFFF;
						else
							pgi->Input.nVal = 0x0001;
#ifdef LSB_FIRST
						*(pgi->Input.pShortVal) = pgi->Input.nVal;
#else
						*((int *)pgi->Input.pShortVal) = pgi->Input.nVal;
#endif
					}
					else
					{
						// Binary controls
						if (s)
							pgi->Input.nVal = 1;
						else
							pgi->Input.nVal = 0;
						*(pgi->Input.pVal) = pgi->Input.nVal;
					}

					break;
				}
			case GIT_KEYSLIDER:						// Keyboard slider
				{
					int nSlider = pgi->Input.Slider.nSliderValue;
					if (pgi->nType == BIT_ANALOG_REL) {
						nSlider -= 0x8000;
						nSlider >>= 4;
					}

					pgi->Input.nVal = (unsigned short)nSlider;
#ifdef LSB_FIRST
					*(pgi->Input.pShortVal) = pgi->Input.nVal;
#else
					*((int *)pgi->Input.pShortVal) = pgi->Input.nVal;
#endif
					break;
				}
		}
	}
}

static int BurnerDrvExit()
{
	if (bDrvOkay)
	{

		VidExit();

		if (nBurnDrvSelect < nBurnDrvCount)
		{
			//MemCardEject();				// Eject memory card if present

			//ConfigGameSave(true);		// save game config

			GameInpExit();				// Exit game input
			BurnDrvExit();				// Exit the driver
		}
	}

	BurnExtLoadRom = NULL;

	bDrvOkay = 0;					// Stop using the BurnDrv functions

	bRunPause = 0;					// Don't pause when exitted

	if (bAudOkay)
		AudWriteSilence();	// Write silence into the sound buffer on exit, and for drivers which don't use pBurnSoundOut

	nBurnDrvSelect = ~0U;			// no driver selected
	nBurnLayer = 0xFF;				// show all layers

	return 0;
}

// get all romsets info, only do once
#if 0
static int getAllRomsetInfo()
{
	if (getinfo)
		return 0;

	char* sname = NULL;
	BurnRomInfo ri;
	unsigned int romCount = 0;
	string name = "";

	unsigned int tempBurnDrvSelect = nBurnDrvSelect;

	// get all romset basic info
	for (nBurnDrvSelect = 0; nBurnDrvSelect < nBurnDrvCount; nBurnDrvSelect++)
	{
		// get game info
		GameInfo* gameInfo = new GameInfo;
		gameInfo->parent = BurnDrvGetTextA(DRV_PARENT) ? BurnDrvGetTextA(DRV_PARENT) : "";
		gameInfo->board = BurnDrvGetTextA(DRV_BOARDROM) ? BurnDrvGetTextA(DRV_BOARDROM) : "";
		name = BurnDrvGetTextA(DRV_NAME);

		// get rom info
		romCount = getRomCount();
		for (unsigned int i = 0; i < romCount; i++)
		{
			memset(&ri, 0, sizeof(ri));
			BurnDrvGetRomInfo(&ri, i); // doesn't contain rom name

			RomInfo romInfo;
			BurnDrvGetRomName(&sname, i, 0); // get rom name
			romInfo.name = sname;
			romInfo.size = ri.nLen;
			romInfo.type = ri.nType;
			if (ri.nCrc == 0)
				romInfo.state = STAT_OK; // pass no_dump rom
			else
				romInfo.state = STAT_NOFIND;

			gameInfo->roms[ri.nCrc] = romInfo;
		}

		// add gameinfo to list
		allGameMap[name] = gameInfo;
	}

	nBurnDrvSelect = tempBurnDrvSelect;

	getAllRomsetCloneInfo();

	getinfo = true;

	return 0;
}
#endif


static int DoLibInit() // Do Init of Burn library driver
{
	int nRet = 0;

	BArchiveOpen(false);

	// If there is a problem with the romset, report it
#if 0
	switch (BArchiveStatus())
	{
		case BARC_STATUS_BADDATA:
			BArchiveClose();
			return 1;
			break;
		case BARC_STATUS_ERROR:
			break;
	}
#endif

	nRet = BurnDrvInit();
	BArchiveClose();

	if (nRet)
		return 3;
	else
		return 0;
}

// Catch calls to BurnLoadRom() once the emulation has started;
// Intialise the zip module before forwarding the call, and exit cleanly.
static int DrvLoadRom(unsigned char* Dest, int* pnWrote, int i)
{
	int nRet;

	BArchiveOpen(false);

	if ((nRet = BurnExtLoadRom(Dest, pnWrote, i)) != 0)
	{
		char* pszFilename;

		BurnDrvGetRomName(&pszFilename, i, 0);
	}

	BArchiveClose();
	BurnExtLoadRom = DrvLoadRom;

	return nRet;
}

static int BurnerDrvInit(int nDrvNum, bool bRestore)
{
	BurnerDrvExit();

	nBurnDrvSelect = nDrvNum;	// Set the driver number

	mediaInit();

	nMaxPlayers = BurnDrvGetMaxPlayers();

	GameInpInit(); // Init game input
	#if 0
	if (ConfigGameLoad(true))
		loadDefaultInput();
	#endif

	InputMake();
	GameInpDefault();

	BurnReinitScrn = scrnReinit;

	int nStatus = DoLibInit(); // Init the burn library's driver
#if 0
	if (nStatus)
	{
		if (nStatus & 2)
			BurnDrvExit(); // exit the driver

		return 1;
	}
#endif

	BurnExtLoadRom = DrvLoadRom;

	bDrvOkay = 1;

	bSaveRAM = false;

	//RunReset();

	return 0;
}

static int DrvInitCallback(void)
{
	return BurnerDrvInit(nBurnDrvSelect, false);
}

static int directLoadGame(const char * name)
{
	int RomOK = 1;

	if (!name)
		return 1;

	if (strcasecmp(&name[strlen(name) - 3], ".fs") == 0)
	{
		if (BurnStateLoad(name, 1, &DrvInitCallback))
			return 1;
	}
	else
	{
		const char * p = getBaseName(name);			// get game name
      unsigned i = BurnDrvGetIndexByNameA(p);

		if (i < nBurnDrvCount)
		{
			RomOK = BurnerDrvInit(i, true);
			bAltPause = 0;
		}
	}

	return RomOK;
}

static bool fba_init(const char *path)
{
   if (!path)
      return false;

	configAppLoadXml(); // no, this isn't actually XML at all
	BurnLibInit();
	//getAllRomsetInfo();
	//nVidFullscreen = 1;
	BurnExtLoadOneRom = archiveLoadOneFile;
	//InitRomList();
	//InitInputList();
	//InitDipList();

	//BuildRomList();

	directLoadGame(path);
	mediaInit();
	bVidOkay = bAudOkay = true;
	//serialize_size = CPUWriteState_libgba(state_buf, 2000000);
	return true;
}

void snes_term(void)
{
   delete[] state_buf;
}

void snes_power(void)
{}

void snes_reset(void)
{}

static void fba_audio(int length)
{
	pBurnSoundOut = pAudNextSound;

	int16_t * currentSound = pAudNextSound;
	for (int i = 0; i < length; i += 2)
		audio_cb(currentSound[i + 0], currentSound[i + 1]);
}

void snes_run(void)
{
	bAudPlaying = true;
	fba_audio(AUDIO_SEGMENT_LENGTH_TIMES_CHANNELS);
	nCurrentFrame++;
	VidFrame();
	InputMake();
}


unsigned snes_serialize_size(void)
{
   return 0;
}

bool snes_serialize(uint8_t *data, unsigned size)
{
   return false;
}

bool snes_unserialize(const uint8_t *data, unsigned size)
{
   return false;
}

void snes_cheat_reset(void)
{}

void snes_cheat_set(unsigned, bool, const char*)
{}

bool snes_load_cartridge_normal(const char*, const uint8_t *, unsigned)
{
	pAudNextSound = (int16_t*)malloc(AUDIO_SEGMENT_LENGTH_TIMES_CHANNELS * sizeof(int16_t));
	return fba_init(g_rom_path);
}

bool snes_load_cartridge_bsx_slotted(
  const char*, const uint8_t*, unsigned,
  const char*, const uint8_t*, unsigned
)
{ return false; }

bool snes_load_cartridge_bsx(
  const char*, const uint8_t *, unsigned,
  const char*, const uint8_t *, unsigned
)
{ return false; }

bool snes_load_cartridge_sufami_turbo(
  const char*, const uint8_t*, unsigned,
  const char*, const uint8_t*, unsigned,
  const char*, const uint8_t*, unsigned
)
{ return false; }

bool snes_load_cartridge_super_game_boy(
  const char*, const uint8_t*, unsigned,
  const char*, const uint8_t*, unsigned
)
{ return false; }

void snes_unload_cartridge(void)
{}

bool snes_get_region(void)
{
   return SNES_REGION_NTSC;
}

uint8_t *snes_get_memory_data(unsigned id)
{
   return 0;
}

unsigned snes_get_memory_size(unsigned id)
{
   return 0;
}

// Stubs

#if 0
u32 systemReadJoypad(int)
{
   poll_cb();

   u32 J = 0;

   static const unsigned binds[] = {
      SNES_DEVICE_ID_JOYPAD_A,
      SNES_DEVICE_ID_JOYPAD_B,
      SNES_DEVICE_ID_JOYPAD_SELECT,
      SNES_DEVICE_ID_JOYPAD_START,
      SNES_DEVICE_ID_JOYPAD_RIGHT,
      SNES_DEVICE_ID_JOYPAD_LEFT,
      SNES_DEVICE_ID_JOYPAD_UP,
      SNES_DEVICE_ID_JOYPAD_DOWN,
      SNES_DEVICE_ID_JOYPAD_R,
      SNES_DEVICE_ID_JOYPAD_L
   };

   for (unsigned i = 0; i < 10; i++)
      J |= input_cb(SNES_PORT_1, SNES_DEVICE_JOYPAD, 0, binds[i]) << i;

   return J;
}
#endif
