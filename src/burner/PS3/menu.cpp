#include <sysutil/sysutil_msgdialog.h>
#include <sysutil/sysutil_sysparam.h>
#include <cell/cell_fs.h>
#include <vector>
#include <algorithm>
#include <map>
#include "menu.h"
#include "burner.h"
#include "../../interface/PS3/cellframework2/input/pad_input.h"
#include "../../interface/PS3/audio_driver.h"
#include "vid_psgl.h"
#include "inp_keys.h"

#define FILEBROWSER_DELAY	90000
 
#define ALL 0
#define CPS1 1
#define CPS2 2
#define CPS3 3
#define NEOGEO 4
#define TAITO 5
#define SEGA 6
#define PGM 7
#define PSIKYO 8
#define KONAMI 9
#define KANEKO 10
#define CAVE 11
#define TOAPLAN 12
#define SEGAMD 13
#define MISC 14

#define MASKMISC		(1 << (HARDWARE_PREFIX_MISC	>> 24))
#define MASKCPS			(1 << (HARDWARE_PREFIX_CPS1	>> 24))
#define MASKNEOGEO		(1 << (HARDWARE_PREFIX_SNK	>> 24))
#define MASKSEGA		(1 << (HARDWARE_PREFIX_SEGA	>> 24))
#define MASKTOAPLAN		(1 << (HARDWARE_PREFIX_TOAPLAN	>> 24))
#define MASKCAVE		(1 << (HARDWARE_PREFIX_CAVE	>> 24))
#define MASKCPS2		(1 << (HARDWARE_PREFIX_CPS2	>> 24))
#define MASKMD			(1 << (HARDWARE_PREFIX_SEGAMD	>> 24))
#define MASKPGM			(1 << (HARDWARE_PREFIX_PGM	>> 24))
#define MASKCPS3		(1 << (HARDWARE_PREFIX_CPS3	>> 24))
#define MASKTAITO		(1 << (HARDWARE_PREFIX_TAITO	>> 24))
#define MASKPSIKYO		(1 << (HARDWARE_PREFIX_PSIKYO	>> 24))
#define MASKKANEKO16		(1 << (HARDWARE_PREFIX_KANEKO16	>> 24))
#define MASKKONAMI		(1 << (HARDWARE_PREFIX_KONAMI	>> 24))
#define MASKPACMAN		(1 << (HARDWARE_PREFIX_PACMAN	>> 24))
#define MASKGALAXIAN		(1 << (HARDWARE_PREFIX_GALAXIAN >> 24))
#define MASKATARI		(1 << (HARDWARE_PREFIX_ATARI	>> 24))

#define MASKALL \
	(MASKMISC | MASKCPS | MASKNEOGEO | MASKSEGA | MASKTOAPLAN \
	| MASKCAVE | MASKCPS2 | MASKMD | MASKPGM | MASKCPS3 \
	| MASKTAITO | MASKPSIKYO | MASKKANEKO16 | MASKKONAMI | MASKPACMAN \
	| MASKGALAXIAN | MASKATARI)

#define SCALING_FACTOR_LIMIT 5
#define MAX_SHADERS 2

#define KEY(x) pgi->nInput = GIT_SWITCH; pgi->Input.Switch = (unsigned short)(x);

/****************************************************/
/* PNG SECTION                                      */
/****************************************************/
 
extern bool DoReset;
extern bool CheckSetting(int i);

unsigned int nPrevGame = ~0U;
static unsigned char nPrevDIPSettings[4];
static unsigned int nDIPGroup;
static unsigned int nDIPSel;
static unsigned int nInpSel;
static int nDIPOffset;

static uint64_t save_state_slot = 0;

std::map<std::string,std::string> m_DipList;
std::vector<std::string> m_DipListData;
std::vector<std::string> m_DipListValues;
std::vector<int> m_DipListOffsets;

std::map<std::string,std::string> m_InputList;
std::vector<std::string> m_InputListData;
std::vector<std::string> m_InputListValues;
std::vector<int> m_InputListOffsets;

std::map<int,std::string> m_InputSettingsList;
std::vector<std::string> m_InputSettingsData;
std::vector<std::string> m_InputSettingsValues;

char DipSetting[64];
char InpSetting[64];
bool dialog_is_running = false;

int CurrentFilter = 0;

int nLastRom = 0;
int nLastFilter = 0;
int HideChildren = 0;
int ThreeOrFourPlayerOnly = 0;
int _fd = 0;

// Rom List Movement 

int iGameSelect;
int iCursorPos;
int iNumGames;
int m_iMaxWindowList;
int m_iWindowMiddle;

selected_shader_t selectedShader[MAX_SHADERS];
uint32_t shaderindex = 0;
uint32_t shaderindex2 = 0;

int currentConfigIndex = 0;
int inGameIndex = 0;
int inputListSel = 0;
int dipListSel = 0;
 
// Input Movement

int iInputSelect;
int iInputCursorPos;
int iNumInput;
int m_iMaxWindowListInput;
int m_iWindowMiddleInput;

int inputList = 0;

// DIP Movement

int  iDipSelect;
int  iDipCursorPos;
int  iNumDips;
int  m_iMaxWindowListDip;
int  m_iWindowMiddleDip;

int dipList = 0;
 
#define GAMESEL_MaxWindowList 28
#define GAMESEL_WindowMiddle 14

#define COLS 0xFFFF7F7f

std::vector<std::string> m_ListData;
std::vector<std::string> m_ListShaderData;
std::vector<std::string> m_ListShader2Data;
std::vector<std::string> m_vecAvailRomIndex;
std::vector<std::string> m_vecAvailRomReleasedBy;
std::vector<std::string> m_vecAvailRomManufacturer;
std::vector<std::string> m_vecAvailRomInfo;
std::vector<std::string> m_vecAvailRomParent;
std::vector<std::int32_t> m_vecAvailRomBurnDrvIndex;

std::map<int, int> m_HardwareFilterMap;
std::map<int, std::string> m_HardwareFilterDesc;

std::vector<std::string> m_vecAvailRomList;

extern int GameStatus;

// DIP Switch Handler Code

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
				pgi->Input.Constant = (pgi->Input.Constant & ~bdi.nMask) | (bdi.nSetting & bdi.nMask);
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

	if ((pgi->Input.Constant & bdi.nMask) == bdi.nSetting)
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
					if ((pgi->Input.Constant & bdi.nMask) == bdi.nSetting)
						return false;
				}
				else
				{
					if ((pgi->Input.Constant & bdi.nMask) != bdi.nSetting)
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
static int InpDIPSWListMake(bool bBuild)
{
	return 0;
}

static int InpDIPSWInit()
{
	return 0;
}

static int InpDIPSWExit()
{
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
				pgi->Input.Constant = nPrevDIPSettings[j];
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


void LoadDIPS()
{
 
	m_DipList.clear();
	m_DipListData.clear();
	m_DipListValues.clear();
	m_DipListOffsets.clear();
 
	BurnDIPInfo bdi;
	unsigned int i = 0, j = 0, k = 0;
	char* pDIPGroup = NULL;
	while (BurnDrvGetDIPInfo(&bdi, i) == 0)
	{
		if ((bdi.nFlags & 0xF0) == 0xF0)
		{
		   	if (bdi.nFlags == 0xFE || bdi.nFlags == 0xFD)
			{
				pDIPGroup = (char*)bdi.szText;
				k = i;
			}
			i++;
		}
		else
		{
			if(CheckSetting(i))
			{ 
				if (pDIPGroup)
				{
					m_DipList[std::string(pDIPGroup)] = std::string(bdi.szText);
					m_DipListData.push_back(pDIPGroup);
					m_DipListOffsets.push_back(k);
				}
				j++;
			}
			i += (bdi.nFlags & 0x0F);
		}
	}

	iNumDips = m_DipListData.size();

	if (iNumDips <	GAMESEL_MaxWindowList)
	{
		m_iMaxWindowListDip = iNumDips;
		m_iWindowMiddleDip  = 0;
	}
	else
	{
		m_iMaxWindowListDip = GAMESEL_MaxWindowList;
		m_iWindowMiddleDip  = GAMESEL_WindowMiddle;
	}
 
}

void LoadInputs()
{
	unsigned int i, j = 0;

	m_InputList.clear();
	m_InputListData.clear();
	m_InputListOffsets.clear();

	// Add all the input names to the list
	for (unsigned int i = 0; i < nGameInpCount; i++)
	{
		// Get the name of the input
		struct BurnInputInfo bii;
		bii.szName = NULL;
		BurnDrvGetInputInfo(&bii, i);

		// skip unused inputs
		if (bii.pVal == NULL)
			continue;
		if (bii.szName == NULL)
			bii.szName = "";

		m_InputList[std::string(bii.szName)] = std::string(" ");
		m_InputListData.push_back(std::string(bii.szName));
		m_InputListOffsets.push_back(j);

		j++;
	}

	struct GameInp* pgi = GameInp + nGameInpCount;

	pgi = NULL;

	// Update the values of all the inputs
	int z = 0;
	for (i = 0, pgi = GameInp; i < nGameInpCount; i++, pgi++)
	{
		if (pgi->Input.pVal == NULL)
			continue;

		BurnInputInfo bii;
		bii.szName = NULL;
		BurnDrvGetInputInfo(&bii, i);

		// skip unused inputs
		if (bii.pVal == NULL)
			continue;
		if (bii.szName == NULL)
			bii.szName = "";


		const char* pszVal = InpToDesc(pgi);

		m_InputList[m_InputListData[z].c_str()] = std::string(pszVal);

		j++;
		z++;
	}

	iNumInput =	m_InputListData.size();

	if (iNumInput <	GAMESEL_MaxWindowList)
	{
		m_iMaxWindowListInput = iNumInput;
		m_iWindowMiddleInput  = 0;
	}
	else
	{
		m_iMaxWindowListInput = GAMESEL_MaxWindowList;
		m_iWindowMiddleInput  = GAMESEL_WindowMiddle;
	}

}

int InitDipList()
{
	iDipSelect = 0;
	iDipCursorPos = 0;

	m_DipList.clear();
	m_DipListData.clear();
	m_DipListValues.clear();
	m_DipListOffsets.clear();

}

int InitInputList()
{
	iInputSelect	= 0;
	iInputCursorPos = 0;

	m_InputSettingsList.clear();
	m_InputSettingsData.clear();

	m_InputSettingsList[CTRL_SQUARE_MASK] = std::string("Square Button");
	m_InputSettingsList[CTRL_CROSS_MASK] = std::string("Cross Button");
	m_InputSettingsList[CTRL_CIRCLE_MASK] = std::string("Circle Button");
	m_InputSettingsList[CTRL_TRIANGLE_MASK] = std::string("Triangle Button");
	m_InputSettingsList[CTRL_START_MASK] = std::string("Start Button");
	m_InputSettingsList[CTRL_SELECT_MASK] = std::string("Select Button");
	m_InputSettingsList[CTRL_L1_MASK] = std::string("L1 Button");
	m_InputSettingsList[CTRL_R1_MASK] = std::string("R1 Button");
	m_InputSettingsList[CTRL_L3_MASK] = std::string("L3 Button");
	m_InputSettingsList[CTRL_R3_MASK] = std::string("R3 Button");
	m_InputSettingsList[CTRL_L2_MASK] = std::string("L2 Button");
	m_InputSettingsList[CTRL_R2_MASK] = std::string("R2 Button");
	m_InputSettingsList[CTRL_R3_MASK | CTRL_L3_MASK] = std::string("L3 + R3");

	m_InputSettingsData.push_back(std::string("Square Button"));
	m_InputSettingsData.push_back(std::string("Cross Button"));
	m_InputSettingsData.push_back(std::string("Circle Button"));
	m_InputSettingsData.push_back(std::string("Triangle Button"));
	m_InputSettingsData.push_back(std::string("Start Button"));
	m_InputSettingsData.push_back(std::string("Select Button"));
	m_InputSettingsData.push_back(std::string("L1 Button"));
	m_InputSettingsData.push_back(std::string("R1 Button"));
	m_InputSettingsData.push_back(std::string("L3 Button"));
	m_InputSettingsData.push_back(std::string("R3 Button"));
	m_InputSettingsData.push_back(std::string("L2 Button"));
	m_InputSettingsData.push_back(std::string("R2 Button"));
	m_InputSettingsData.push_back(std::string("L3 + R3"));
}

int InitRomList()
{
 	m_ListData.clear();
	m_vecAvailRomList.clear();
	m_vecAvailRomIndex.clear();
	m_vecAvailRomBurnDrvIndex.clear();

	iGameSelect = 0;
	iCursorPos = 0;

	// build the hardware filter map
 
	m_HardwareFilterMap[ALL] = MASKALL;
	m_HardwareFilterMap[CPS1] = MASKCPS;
	m_HardwareFilterMap[CPS2] = MASKCPS2;
	m_HardwareFilterMap[CPS3] = MASKCPS3;
	m_HardwareFilterMap[NEOGEO] = MASKNEOGEO;
	m_HardwareFilterMap[TAITO] = MASKTAITO;
	m_HardwareFilterMap[SEGA] = MASKSEGA;
	m_HardwareFilterMap[PGM] = MASKPGM;
	m_HardwareFilterMap[PSIKYO] = MASKPSIKYO;
	m_HardwareFilterMap[KONAMI] = MASKKONAMI;
	m_HardwareFilterMap[KANEKO] = MASKKANEKO16;
	m_HardwareFilterMap[CAVE] = MASKCAVE;
	m_HardwareFilterMap[TOAPLAN] = MASKTOAPLAN;
	m_HardwareFilterMap[SEGAMD] = MASKMD;
	m_HardwareFilterMap[MISC] = MASKMISC;

	m_HardwareFilterDesc[ALL] = "All Hardware";
	m_HardwareFilterDesc[CPS1] = "Capcom CPS1";
	m_HardwareFilterDesc[CPS2] = "Capcom CPS2";
	m_HardwareFilterDesc[CPS3] = "Capcom CPS3";
	m_HardwareFilterDesc[NEOGEO] = "NeoGeo";
	m_HardwareFilterDesc[TAITO] = "Taito";
	m_HardwareFilterDesc[SEGA] = "Sega System 16";
	m_HardwareFilterDesc[PGM] = "PGM";
	m_HardwareFilterDesc[PSIKYO] = "Psikyo";
	m_HardwareFilterDesc[KONAMI] = "Konami";
	m_HardwareFilterDesc[KANEKO] = "Kaneko 16";
	m_HardwareFilterDesc[CAVE] = "Cave";
	m_HardwareFilterDesc[TOAPLAN] = "Toaplan";
	m_HardwareFilterDesc[SEGAMD] = "Sega Megadrive";
	m_HardwareFilterDesc[MISC] = "Misc";

	return 0;
} 

static int AvRoms()
{
	iNumGames = m_vecAvailRomList.size();

	if (iNumGames < GAMESEL_MaxWindowList)
	{
		m_iMaxWindowList = iNumGames;
		m_iWindowMiddle	 = iNumGames/2;
	}
	else
	{
		m_iMaxWindowList = GAMESEL_MaxWindowList;
		m_iWindowMiddle	 = GAMESEL_WindowMiddle;
	}

	return 0;
}

#define iterate_directory(rompath, array) \
   if (cellFsOpendir(rompath, &_fd) == CELL_FS_SUCCEEDED) \
   { \
      CellFsDirent dirent; \
      uint64_t nread = 0; \
      while (cellFsReaddir(_fd, &dirent, &nread) == CELL_FS_SUCCEEDED) \
      { \
         if (nread == 0) \
            break; \
         if (dirent.d_type == CELL_FS_TYPE_REGULAR) \
            array.push_back(dirent.d_name); \
      } \
      cellFsClosedir(_fd); \
   }	

void BuildRomList()
{
	bool IsFiltered = false;
	std::vector<std::string> vecTempRomList;
	std::vector<std::string> vecAvailRomListFileName;
	std::vector<std::string> vecAvailRomList;
	std::vector<int> vecAvailRomIndex;

	m_vecAvailRomList.clear();
	m_vecAvailRomReleasedBy.clear();
	m_vecAvailRomInfo.clear();
	m_vecAvailRomParent.clear();
	m_vecAvailRomManufacturer.clear();
	m_vecAvailRomIndex.clear();
	m_vecAvailRomBurnDrvIndex.clear();

	iGameSelect = 0;
	iCursorPos = 0;

	if (m_ListData.empty())
	{
		for (int d = 0; d < DIRS_MAX; d++)
		{
			if (!strcasecmp(szAppRomPaths[d], ""))
				continue; // skip empty path

			iterate_directory(szAppRomPaths[d], m_ListData);
		}
		std::sort(m_ListData.begin(), m_ListData.end());
	}

	//shader #1
	if (m_ListShaderData.empty())
	{
		if (cellFsOpendir(SHADER_DIRECTORY, &_fd) == CELL_FS_SUCCEEDED)
		{
			CellFsDirent dirent;
			uint64_t nread = 0;
			while (cellFsReaddir(_fd, &dirent, &nread) == CELL_FS_SUCCEEDED)
			{
				if (nread == 0)
					break;
				if (dirent.d_type == CELL_FS_TYPE_REGULAR)
				{
					if(strstr(dirent.d_name,".cg") || strstr(dirent.d_name,".CG"))
						m_ListShaderData.push_back(dirent.d_name);
				}
			}
			cellFsClosedir(_fd);
		}
		std::sort(m_ListShaderData.begin(), m_ListShaderData.end());
		for (unsigned int x = 0; x < m_ListShaderData.size(); x++)
		{
			if(strcmp(selectedShader[0].filename,m_ListShaderData[x].c_str()) == 0)
			{
				shaderindex = selectedShader[0].index = x;
				sprintf(selectedShader[0].fullpath, "%s%s", SHADER_DIRECTORY, m_ListShaderData[x].c_str());
				break;
			}
		}
	}

	//shader #2
	if (m_ListShader2Data.empty())
	{
		if (cellFsOpendir(SHADER_DIRECTORY, &_fd) == CELL_FS_SUCCEEDED)
		{
			CellFsDirent dirent;
			uint64_t nread = 0;
			while (cellFsReaddir(_fd, &dirent, &nread) == CELL_FS_SUCCEEDED)
			{
				if (nread == 0)
					break;
				if (dirent.d_type == CELL_FS_TYPE_REGULAR)
				{
					if(strstr(dirent.d_name,".cg") || strstr(dirent.d_name,".CG"))
						m_ListShader2Data.push_back(dirent.d_name);
				}
			}
			cellFsClosedir(_fd);
		}
		std::sort(m_ListShader2Data.begin(), m_ListShader2Data.end());
		for (unsigned int x = 0; x < m_ListShader2Data.size(); x++)
		{
			if(strcmp(selectedShader[1].filename,m_ListShader2Data[x].c_str()) == 0)
			{
				shaderindex2 = selectedShader[1].index = x;
				sprintf(selectedShader[1].fullpath, "%s%s", SHADER_DIRECTORY, m_ListShader2Data[x].c_str());
				break;
			}
		}
	}

	// Now build a vector of Burn Roms
	unsigned int i = 0;
	do
	{
		nBurnDrvSelect = i;	
		char *szName;

		BurnDrvGetArchiveName(&szName, 0);

		vecAvailRomListFileName.push_back(szName);
		vecAvailRomList.push_back(BurnDrvGetTextA(DRV_FULLNAME));
		vecAvailRomIndex.push_back(i);
		i++;
	}while(i < nBurnDrvCount-1);

	// For each *.zip we have, see if there is a matching burn rom
	// if so add it to the m_vec members and we are done.

	for (unsigned int x = 0; x < vecAvailRomListFileName.size(); x++)
	{
		for (unsigned int y = 0; y < m_ListData.size(); y++)
		{
			if (m_ListData[y] == vecAvailRomListFileName[x])
			{
				nBurnDrvSelect = vecAvailRomIndex[x];

				const int nHardware = 1 << (BurnDrvGetHardwareCode() >> 24);

				if (CurrentFilter > 0)
					IsFiltered = (nHardware) == m_HardwareFilterMap[CurrentFilter];
				else
					IsFiltered = true;


				if ((IsFiltered))  // skip roms marked as not working
					if (BurnDrvIsWorking() && (IsFiltered))  // skip roms marked as not working
					{

						int nNumPlayers = BurnDrvGetMaxPlayers();

						if ((HideChildren == 1 && (BurnDrvGetTextA(DRV_PARENT) == NULL && !(BurnDrvGetFlags() & BDF_CLONE))) || (HideChildren == 1 && (BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_CAPCOM_CPS3) || (HideChildren == 0))
						{
							if ((ThreeOrFourPlayerOnly == 1 && nNumPlayers > 2) || ThreeOrFourPlayerOnly == 0)
							{
								m_vecAvailRomIndex.push_back(vecAvailRomListFileName[x]);
								m_vecAvailRomBurnDrvIndex.push_back(vecAvailRomIndex[x]);
								m_vecAvailRomList.push_back(BurnDrvGetTextA(DRV_FULLNAME));
								m_vecAvailRomReleasedBy.push_back(BurnDrvGetTextA(DRV_MANUFACTURER));		

								if (BurnDrvGetTextA(DRV_SYSTEM))
									m_vecAvailRomManufacturer.push_back(BurnDrvGetTextA(DRV_SYSTEM));
								else
									m_vecAvailRomManufacturer.push_back("Unknown");

								if (BurnDrvGetTextA(DRV_COMMENT))
									m_vecAvailRomInfo.push_back(BurnDrvGetTextA(DRV_COMMENT));
								else
									m_vecAvailRomInfo.push_back("No Additional Information");

								if (BurnDrvGetTextA(DRV_PARENT))
									m_vecAvailRomParent.push_back(BurnDrvGetTextA(DRV_PARENT));
								else
									m_vecAvailRomParent.push_back("No Parent Rom");
							}
						}
					}
				break;
			}
		}
	}
	AvRoms();
	//nBurnDrvSelect = ~0U;

}


void ConfigMenu()
{
	cellDbgFontPuts(0.05f, 0.04f , 0.75f, 0xFFE0EEFF, "FBANext PS3 - Configuration Menu");             
	cellDbgFontDraw();

	if (bDrvOkay)
		cellDbgFontPrintf(0.6f, 0.90f + 0.025f, 0.75f, 0xFF805EFF ,"Circle - Return to Game");		
	else
		cellDbgFontPuts(0.6f, 0.90f + 0.025f, 0.75f, 0xFFE0EEFF ,"Circle - Back to Main Menu");

	cellDbgFontPuts(0.6f, 0.92f + 0.025f, 0.75f, 0xFFE0EEFF ,"Triangle - Generate CLRMAME.DAT");

	cellDbgFontDraw();

	int number = 0;
	cellDbgFontPrintf(0.05f, 0.08f + 0.025f * ((float)number), 0.75f, currentConfigIndex == SETTING_DISPLAY_FRAMERATE ? COLS : 0xFFFFFFFF, "Show Framerate : %s", bShowFPS ? "Yes" : "No" );     
	cellDbgFontDraw();
	number++;

	switch(psglGetCurrentResolutionId())
	{
		case CELL_VIDEO_OUT_RESOLUTION_480:
			cellDbgFontPrintf(0.05f, 0.08f + 0.025f * ((float)number), 0.75f, currentConfigIndex == SETTING_RESOLUTION ? COLS : 0xFFFFFFFF, "Resolution : 720x480 (480p)");
			break;
		case CELL_VIDEO_OUT_RESOLUTION_720:
			cellDbgFontPrintf(0.05f, 0.08f + 0.025f * ((float)number), 0.75f, currentConfigIndex == SETTING_RESOLUTION ? COLS : 0xFFFFFFFF, "Resolution : 1280x720 (720p)");
			break;
		case CELL_VIDEO_OUT_RESOLUTION_1080:
			cellDbgFontPrintf(0.05f, 0.08f + 0.025f * ((float)number), 0.75f, currentConfigIndex == SETTING_RESOLUTION ? COLS : 0xFFFFFFFF, "Resolution : 1920x1080 (1080p)");
			break;
		case CELL_VIDEO_OUT_RESOLUTION_576:
			cellDbgFontPrintf(0.05f, 0.08f + 0.025f * ((float)number), 0.75f, currentConfigIndex == SETTING_RESOLUTION ? COLS : 0xFFFFFFFF, "Resolution : 720x576 (576p)");
			break;
		case CELL_VIDEO_OUT_RESOLUTION_1600x1080:
			cellDbgFontPrintf(0.05f, 0.08f + 0.025f * ((float)number), 0.75f, currentConfigIndex == SETTING_RESOLUTION ? COLS : 0xFFFFFFFF, "Resolution : 1600x1080");
			break;
		case CELL_VIDEO_OUT_RESOLUTION_1440x1080:
			cellDbgFontPrintf(0.05f, 0.08f + 0.025f * ((float)number), 0.75f, currentConfigIndex == SETTING_RESOLUTION ? COLS : 0xFFFFFFFF, "Resolution : 1440x1080");
			break;
		case CELL_VIDEO_OUT_RESOLUTION_1280x1080:
			cellDbgFontPrintf(0.05f, 0.08f + 0.025f * ((float)number), 0.75f, currentConfigIndex == SETTING_RESOLUTION ? COLS : 0xFFFFFFFF, "Resolution : 1280x1080");
			break;
		case CELL_VIDEO_OUT_RESOLUTION_960x1080:
			cellDbgFontPrintf(0.05f, 0.08f + 0.025f * ((float)number), 0.75f, currentConfigIndex == SETTING_RESOLUTION ? COLS : 0xFFFFFFFF, "Resolution : 960x1080");
			break;
	}
	cellDbgFontDraw();
	number++;

	char msg[256];
	switch(nVidScrnAspectMode)
	{
		case ASPECT_RATIO_CUSTOM:
			strcpy(msg,"Custom (Resized)");
			break;
		case ASPECT_RATIO_AUTO:
			sprintf(msg,"Auto %d:%d", nVidScrnAspectX, nVidScrnAspectY);
			break;
		case ASPECT_RATIO_AUTO_FBA: 
			sprintf(msg,"Auto (FBA) %d:%d", nVidScrnAspectX, nVidScrnAspectY);
			break;
		default:
			sprintf(msg,"%d:%d", nVidScrnAspectX, nVidScrnAspectY);
			break;
	}
	cellDbgFontPrintf(0.05f, 0.08f + 0.025f * ((float)number), 0.75f, currentConfigIndex == SETTING_KEEP_ASPECT ? COLS : 0xFFFFFFFF, "Aspect Ratio : %s", msg);
	cellDbgFontDraw();
	number++;

	char rotatemsg[3][256] = {{"Rotate for Vertical Games"},{"Do not rotate for Vertical Games"},{"Reverse flipping for vertical games"}};
	cellDbgFontPrintf(0.05f, 0.08f + 0.025f * ((float)number), 0.75f, currentConfigIndex == SETTING_ROTATE ? COLS : 0xFFFFFFFF, "Rotation Adjust: %s", rotatemsg[nVidRotationAdjust]);     
	cellDbgFontDraw();
	number++;

	cellDbgFontPrintf(0.05f, 0.08f + 0.025f * ((float)number), 0.75f, currentConfigIndex == SETTING_FBO_ENABLED ? COLS : 0xFFFFFFFF, "Custom Scaling/Dual Shaders: %s", bVidFBOEnabled ? "Yes" : "No");
	cellDbgFontDraw();	
	number++;

	cellDbgFontPrintf(0.05f, 0.08f + 0.025f * ((float)number), 0.75f, currentConfigIndex == SETTING_CURRENT_SHADER ? COLS : 0xFFFFFFFF, "Current Shader #1: %s", m_ListShaderData[shaderindex].c_str());
	cellDbgFontDraw();	
	number++;

	cellDbgFontPrintf(0.05f, 0.08f + 0.025f * ((float)number), 0.75f, currentConfigIndex == SETTING_CURRENT_SHADER2 ? COLS : 0xFFFFFFFF, "Current Shader #2: %s", m_ListShader2Data[shaderindex2].c_str());
	cellDbgFontDraw();	
	number++;

	cellDbgFontPrintf(0.05f, 0.08f + 0.025f * ((float)number), 0.75f, currentConfigIndex == SETTING_BILINEAR_FILTER ? COLS : 0xFFFFFFFF, "Hardware Filter Shader #1: %s", vidFilterLinear ? "Linear" : "Point");
	cellDbgFontDraw();
	number++;

	cellDbgFontPrintf(0.05f, 0.08f + 0.025f * ((float)number), 0.75f, currentConfigIndex == SETTING_BILINEAR_FILTER2 ? COLS : 0xFFFFFFFF, "Hardware Filter Shader #2: %s", vidFilterLinear2 ? "Linear" : "Point");
	cellDbgFontDraw();
	number++;

	cellDbgFontPrintf(0.05f, 0.08f + 0.025f * ((float)number), 0.75f, currentConfigIndex == SETTING_SCALING_FACTOR ? COLS : 0xFFFFFFFF, "Scaling Factor: %dx", bVidScalingFactor);
	cellDbgFontDraw();
	number++;

	cellDbgFontPrintf(0.05f, 0.08f + 0.025f * ((float)number), 0.75f, currentConfigIndex == SETTING_VSYNC ? COLS : 0xFFFFFFFF, "Vertical Sync : %s", bVidVSync ? "Yes" : "No");
	cellDbgFontDraw();
	number++;

	cellDbgFontPrintf(0.05f, 0.08f + 0.025f * ((float)number), 0.75f, currentConfigIndex == SETTING_HIDE_CLONES ? COLS : 0xFFFFFFFF, "Hide Clone Roms : %s", HideChildren ? "Yes" : "No");
	cellDbgFontDraw();
	number++;

	cellDbgFontPrintf(0.05f, 0.08f + 0.025f * ((float)number), 0.75f, currentConfigIndex == SETTING_SHOW_THREE_FOUR_PLAYER_ONLY ? COLS : 0xFFFFFFFF, "Show 3 or 4 Player Roms Only : %s", ThreeOrFourPlayerOnly ? "Yes" : "No");
	cellDbgFontDraw();
	number++;

	cellDbgFontPrintf(0.05f, 0.08f + 0.025f * ((float)number), 0.75f, currentConfigIndex == SETTING_TRIPLE_BUFFER ? COLS : 0xFFFFFFFF, "Triple Buffering Enabled : %s", bVidTripleBuffer ? "Yes" : "No");
	cellDbgFontDraw();

	cellDbgFontPrintf(0.05f, 0.92f + 0.025f, 0.50f, 0xFFFFE0E0, "Core %s - r%s - %s", szAppBurnVer, szSVNVer, szSVNDate);
	cellDbgFontDraw();


}

static void cb_dialog_ok(int button_type, void *userdata)
{
	switch(button_type)
	{
		case CELL_MSGDIALOG_BUTTON_ESCAPE:
		dialog_is_running = false;
		break;
	}
}

void ConfigFrameMove()
{
	static uint64_t old_state = 0;
	uint64_t new_state = cell_pad_input_poll_device(0);
	uint64_t diff_state = old_state ^ new_state;

	if (CTRL_CIRCLE(new_state))
	{
		// switch to config
		old_state = new_state;
		if (bDrvOkay)	// theres a game loaded, return to game
		{
			audio_play();
			GameStatus = EMULATING;
			return;
		}
		else
			GameStatus = MENU;	// back to romlist
	}
	else if (CTRL_TRIANGLE(old_state & diff_state))
	{
		// switch to config

		UpdateConsoleXY("Generating clrmame.dat. Please wait...", 0.35f, 0.5f );

		if (create_datfile(DAT_FILE,0) == 0)
		{
			dialog_is_running = true;
			cellMsgDialogOpen2(CELL_MSGDIALOG_DIALOG_TYPE_NORMAL| \
					CELL_MSGDIALOG_TYPE_BG_VISIBLE| \
					CELL_MSGDIALOG_TYPE_BUTTON_TYPE_NONE|CELL_MSGDIALOG_TYPE_DISABLE_CANCEL_OFF|\
					CELL_MSGDIALOG_TYPE_DEFAULT_CURSOR_OK,\
					"clrmame.dat created in /dev_hdd0/game/FBAN00000/USRDIR/.",cb_dialog_ok,NULL,NULL);
			while(dialog_is_running)
			{
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
				psglSwap();
				cellSysutilCheckCallback();	
			}
		}
		else
		{
			dialog_is_running = true;
			cellMsgDialogOpen2(CELL_MSGDIALOG_TYPE_SE_TYPE_ERROR| \
					CELL_MSGDIALOG_TYPE_BG_VISIBLE| \
					CELL_MSGDIALOG_TYPE_BUTTON_TYPE_NONE|CELL_MSGDIALOG_TYPE_DISABLE_CANCEL_OFF|\
					CELL_MSGDIALOG_TYPE_DEFAULT_CURSOR_OK,\
					"Error generating clrmame.dat.",cb_dialog_ok,NULL,NULL);
			while(dialog_is_running)
			{
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
				psglSwap();
				cellSysutilCheckCallback();	
			}
		}
	}
	else if(CTRL_DOWN(new_state & diff_state) | CTRL_R2(new_state) | CTRL_LSTICK_DOWN(new_state))
	{
		sys_timer_usleep(FILEBROWSER_DELAY);
		currentConfigIndex++;

		if (currentConfigIndex >= MAX_NO_OF_SETTINGS)
			currentConfigIndex = MAX_NO_OF_SETTINGS-1;
	}
	else if(CTRL_UP(new_state & diff_state) | CTRL_L2(new_state) | CTRL_LSTICK_UP(new_state))
	{		
		sys_timer_usleep(FILEBROWSER_DELAY);
		currentConfigIndex--;

		if (currentConfigIndex < 0)
			currentConfigIndex = 0;
	}

	switch(currentConfigIndex)
	{
		case SETTING_DISPLAY_FRAMERATE:
			if(CTRL_LEFT(new_state & diff_state) | CTRL_RIGHT(new_state & diff_state) | CTRL_CROSS(old_state & diff_state))
			{
				bShowFPS = !bShowFPS;
			}
			break;
		case SETTING_RESOLUTION:
			if(CTRL_LEFT(new_state & diff_state) | CTRL_LSTICK_LEFT(new_state)) 
			{
				psglResolutionPrevious();
				sys_timer_usleep(FILEBROWSER_DELAY);
			}
			if(CTRL_RIGHT(new_state & diff_state) | CTRL_LSTICK_RIGHT(new_state)) 
			{
				psglResolutionNext();	
				sys_timer_usleep(FILEBROWSER_DELAY);
			}
			if(CTRL_CROSS(old_state & diff_state))
			{
				psglResolutionSwitch();	
			}
			break;
		case SETTING_KEEP_ASPECT:
			if(CTRL_LEFT(new_state & diff_state))
			{
				if(nVidScrnAspectMode > 0)
				{
					nVidScrnAspectMode--;
					setWindowAspect(0);
				}
			}
			else if(CTRL_RIGHT(new_state & diff_state) | CTRL_CROSS(old_state & diff_state))
			{
				if(nVidScrnAspectMode < LAST_ASPECT_RATIO)
				{
					nVidScrnAspectMode++;
					setWindowAspect(0);
				}
			}
			break;
		case SETTING_ROTATE:
			if(CTRL_LEFT(new_state & diff_state) | CTRL_RIGHT(new_state & diff_state) | CTRL_CROSS(old_state & diff_state))
			{
				nVidRotationAdjust++;

				if (nVidRotationAdjust > 2)
				{
					nVidRotationAdjust = 0;
				}
				//apply_rotation_settings();
			}
			break;
		case SETTING_FBO_ENABLED:
			if(CTRL_LEFT(new_state & diff_state) | CTRL_RIGHT(new_state & diff_state) | CTRL_CROSS(old_state & diff_state))
				bVidFBOEnabled = !bVidFBOEnabled;
			if(CTRL_CROSS(old_state & diff_state))
			{
				//FBO mode needs to be applied here
				//Screen reinited
			}
			break;
		case SETTING_CURRENT_SHADER:
			if(CTRL_LEFT(new_state & diff_state) | CTRL_LSTICK_LEFT(new_state))
			{
				if(shaderindex > 0)
					shaderindex--;
			}

			if(CTRL_RIGHT(new_state & diff_state) | CTRL_LSTICK_RIGHT(new_state))
			{
				if(shaderindex < m_ListShaderData.size()-1)
					shaderindex ++;
			}
			if(CTRL_CROSS(old_state & diff_state))
			{
				selectedShader[0].index = shaderindex;
				strcpy(selectedShader[0].filename, m_ListShaderData[shaderindex].c_str());
				sprintf(selectedShader[0].fullpath, "%s%s", SHADER_DIRECTORY,m_ListShaderData[shaderindex].c_str());
				psglInitShader(selectedShader[0].fullpath);
			}
			break;
		case SETTING_CURRENT_SHADER2:
			if(CTRL_LEFT(new_state & diff_state) | CTRL_LSTICK_LEFT(new_state))
			{
				if(shaderindex2 > 0)
					shaderindex2--;
			}

			if(CTRL_RIGHT(new_state & diff_state) | CTRL_LSTICK_RIGHT(new_state))
			{
				if(shaderindex2 < m_ListShader2Data.size()-1)
					shaderindex2++;
			}
			if(CTRL_CROSS(old_state & diff_state))
			{
				selectedShader[1].index = shaderindex2;
				strcpy(selectedShader[1].filename, m_ListShader2Data[shaderindex2].c_str());
				sprintf(selectedShader[1].fullpath, "%s%s", SHADER_DIRECTORY,m_ListShader2Data[shaderindex2].c_str());
				psglInitShader(selectedShader[1].fullpath);
			}
			break;
		case SETTING_BILINEAR_FILTER:
			if(CTRL_LEFT(new_state & diff_state) | CTRL_RIGHT(new_state & diff_state) | CTRL_CROSS(old_state & diff_state))
				vidFilterLinear = !vidFilterLinear;
			break;
		case SETTING_BILINEAR_FILTER2:
			if(CTRL_LEFT(new_state & diff_state) | CTRL_RIGHT(new_state & diff_state) | CTRL_CROSS(old_state & diff_state))
				vidFilterLinear2 = !vidFilterLinear2;
			break;
		case SETTING_SCALING_FACTOR:
			if(CTRL_LEFT(new_state & diff_state) | CTRL_LSTICK_LEFT(new_state))
			{
				if(bVidScalingFactor > 1)
					bVidScalingFactor--;
			}

			if(CTRL_RIGHT(new_state & diff_state) | CTRL_LSTICK_RIGHT(new_state))
			{
				if(bVidScalingFactor < SCALING_FACTOR_LIMIT)
					bVidScalingFactor++;
			}
			if(CTRL_CROSS(old_state & diff_state))
			{
				//apply scale here, and reapply FBO if activated

				//reapply screen here
				BurnReinitScrn();
				psglRedraw();
			}
			break;
		case SETTING_VSYNC:
			if(CTRL_LEFT(new_state & diff_state) | CTRL_RIGHT(new_state & diff_state) | CTRL_CROSS(old_state & diff_state))
			{
				bVidVSync = !bVidVSync;
				psglSetVSync(bVidVSync);
			}
			break;
		case SETTING_HIDE_CLONES:
			if(CTRL_LEFT(new_state & diff_state) | CTRL_RIGHT(new_state & diff_state) | CTRL_CROSS(old_state & diff_state))
				HideChildren = !HideChildren;
			break;
		case SETTING_SHOW_THREE_FOUR_PLAYER_ONLY:
			if(CTRL_LEFT(new_state & diff_state) | CTRL_RIGHT(new_state & diff_state) | CTRL_CROSS(old_state & diff_state))
				ThreeOrFourPlayerOnly = !ThreeOrFourPlayerOnly;
			break;
		case SETTING_TRIPLE_BUFFER:
			if(CTRL_LEFT(new_state & diff_state) | CTRL_RIGHT(new_state & diff_state) | CTRL_CROSS(old_state & diff_state))
			{
				bVidTripleBuffer = !bVidTripleBuffer; // we need to reset the context if we toggle tripple buffering
				psglResolutionSwitch();
			}
			break;

	}

	ConfigMenu();
	old_state = new_state;
}


void RomMenu()
{
#ifdef CELL_DEBUG_MEMORY
	sys_memory_info_t mem_info;
#endif

	int	iTempGameSel;
	int	iGameidx;

	// draw	game list entries

	iTempGameSel = iGameSelect;

#ifdef CELL_DEBUG_MEMORY
	sys_memory_get_user_memory_size(&mem_info);
#endif
	cellDbgFontPuts(0.05f, 0.04f , 0.75f, 0xFFE0EEFF, "FBANext PS3 - Main Menu");             
	cellDbgFontDraw();

	cellDbgFontPrintf(0.6f, 0.04f, 0.75f, 0xFFE0EEFF, "Current Hardware : %s", m_HardwareFilterDesc[CurrentFilter].c_str());     
	cellDbgFontDraw();
	cellDbgFontPuts(0.6f, 0.06f, 0.75f, 0xFFE0EEFF ,"L1/R1 - Previous/Next Hardware Filter");     
	cellDbgFontDraw();

	if (iNumGames == 0)
	{		
		cellDbgFontPuts(0.05f, 0.08f, 0.75f, COLS, "No Roms Found");             
		cellDbgFontDraw();
	}

	iGameidx = 0;
	do
	{
		if (iGameidx==iCursorPos)
		{	
			cellDbgFontPuts(0.05f, 0.08f + 0.025f * ((float)iGameidx ), 0.75f, COLS, m_vecAvailRomList[iTempGameSel++].c_str());             
			cellDbgFontDraw();	

			cellDbgFontPuts(0.6f, 0.80f + 0.025f, 0.75f, 0xFFE0EEFF ,"Select - Options Menu");     
			cellDbgFontDraw();
			cellDbgFontPrintf(0.6f, 0.82f + 0.025f, 0.75f, 0xFFE0EEFF ,"X - Load ROM");     
			cellDbgFontDraw();

			if (bDrvOkay)
			{
				cellDbgFontPrintf(0.6f, 0.90f + 0.025f, 0.75f, 0xFF805EFF ,"Circle - Return to Game");     
				cellDbgFontDraw();
				cellDbgFontPrintf(0.6f, 0.88f + 0.025f, 0.75f, 0xFF805EFF ,"Start - Exit Rom");     
				cellDbgFontDraw();
			}

#ifdef CELL_DEBUG_MEMORY
			cellDbgFontPrintf(0.75f, 0.90f + 0.025f, 0.75f, COLS ,"%ld free memory",mem_info.available_user_memory );     
			cellDbgFontDraw();
			cellDbgFontPrintf(0.75f, 0.92f + 0.025f, 0.75f, COLS ,"%ld total memory",mem_info.total_user_memory );     
			cellDbgFontDraw();
			cellDbgFontPrintf(0.75f, 0.95f + 0.025f, 0.75f, COLS ,"BurnDrvSelect = %ld ", m_vecAvailRomBurnDrvIndex[iGameSelect+iCursorPos]);     
			cellDbgFontDraw();
#endif


			cellDbgFontPrintf(0.05f, 0.80f + 0.025f, 0.75f, 0xFFFFE0E0, "ROM Name : %s", m_vecAvailRomIndex[iGameSelect+iCursorPos].c_str() );     
			cellDbgFontDraw();
			cellDbgFontPrintf(0.05f, 0.82f + 0.025f, 0.75f, 0xFFFFE0E0, "ROM Info : %s", m_vecAvailRomInfo[iGameSelect+iCursorPos].c_str());
			cellDbgFontDraw();
			cellDbgFontPrintf(0.05f, 0.84f + 0.025f, 0.75f, 0xFFFFE0E0, "Hardware : %s", m_vecAvailRomManufacturer[iGameSelect+iCursorPos].c_str() );     
			cellDbgFontDraw();
			cellDbgFontPrintf(0.05f, 0.86f + 0.025f, 0.75f, 0xFFFFE0E0, "Released by : %s", m_vecAvailRomReleasedBy[iGameSelect+iCursorPos].c_str());
			cellDbgFontDraw();
			cellDbgFontPrintf(0.05f, 0.88f + 0.025f, 0.75f, 0xFFFFE0E0, "Parent ROM : %s", m_vecAvailRomParent[iGameSelect+iCursorPos].c_str());
			cellDbgFontDraw();

			cellDbgFontPrintf(0.05f, 0.92f + 0.025f, 0.50f, 0xFFFFE0E0, "Core %s - r%s - %s", szAppBurnVer, szSVNVer, szSVNDate);
			cellDbgFontDraw();

			cellDbgFontPuts(0.6f, 0.84f + 0.025f  , 0.75f, 0xFFE0EEFF, "Triangle - Hide Clone Roms");
			cellDbgFontDraw();
			cellDbgFontPuts(0.6f, 0.86f + 0.025f  , 0.75f, 0xFFE0EEFF, "Square - Show 3 or 4 Player Roms Only");
			cellDbgFontDraw();
		}
		else
		{			 
			cellDbgFontPuts(0.05f, 0.08f + 0.025f * ((float)iGameidx), 0.75f, 0xFFFFFFFF, m_vecAvailRomList[iTempGameSel++].c_str());             
			cellDbgFontDraw();
		}
		iGameidx++;
	}while(iGameidx<m_iMaxWindowList);
}

void DipFrameMove()
{
	static uint64_t old_state = 0;
	uint64_t new_state = cell_pad_input_poll_device(0);
	uint64_t diff_state = old_state ^ new_state;

	if (CTRL_CIRCLE(new_state & diff_state))
	{
		// switch to dip list
		if (!dipList)
		{

			if (bDrvOkay)	// theres a game loaded, return to game
			{
				audio_play();
				is_running = 1;
				GameStatus = EMULATING;
				old_state = new_state;
				return;
			}
		}
		else
			dipList = 0;
	}
	else if (CTRL_CROSS(new_state & diff_state))
	{
		if (!dipList)
		{
			dipListSel = 0;
			dipList = 1;

			int nSel = iDipCursorPos;

			m_DipListValues.clear();

			if (nSel >= 0)
			{
				if (m_DipListOffsets.size() > 0)
				{
					nDIPGroup = m_DipListOffsets[iDipSelect	+ iDipCursorPos];

					BurnDIPInfo bdiGroup;
					BurnDrvGetDIPInfo(&bdiGroup, nDIPGroup);

					int nCurrentSetting = 0;
					for (int i = 0, j = 0; i < bdiGroup.nSetting; i++)
					{
						char szText[MAX_PATH];
						BurnDIPInfo bdi;

						do {
							BurnDrvGetDIPInfo(&bdi, nDIPGroup + 1 + j++);
						} while (bdi.nFlags == 0);

						if (bdiGroup.szText)
							sprintf(szText, "%hs: %hs", bdiGroup.szText, bdi.szText);
						else
							sprintf(szText, "%hs", bdi.szText);

						m_DipListValues.push_back(std::string(szText));

						if(CheckSetting(nDIPGroup + j))
							nCurrentSetting = i;
					}
				}

			}		

			old_state = new_state;
			return;
		}
		else
		{	
			int id = dipListSel;
			BurnDIPInfo bdi = {0, 0, 0, 0, NULL};
			int j = 0;
			for (int i = 0; i <= id; i++)
			{
				do {
					BurnDrvGetDIPInfo(&bdi, nDIPGroup + 1 + j++);
				} while (bdi.nFlags == 0);
			}


			struct GameInp* pgi = GameInp + bdi.nInput + nDIPOffset;
			pgi->Input.Constant = (pgi->Input.Constant & ~bdi.nMask) | (bdi.nSetting & bdi.nMask);
			if (bdi.nFlags & 0x40)
			{
				while (BurnDrvGetDIPInfo(&bdi, nDIPGroup + 1 + j++) == 0)
				{
					if (bdi.nFlags == 0)
					{
						pgi = GameInp + bdi.nInput + nDIPOffset;
						pgi->Input.Constant = (pgi->Input.Constant & ~bdi.nMask) | (bdi.nSetting & bdi.nMask);
					}
					else
						break;
				}
			}

			LoadDIPS();
			dipList = 0;
			old_state = new_state;
			return;
		}		
	}				 
	else if(CTRL_DOWN(new_state & diff_state) | CTRL_R2(new_state) | CTRL_LSTICK_DOWN(new_state))
	{
		if (!dipList)
		{
			// default don`t clamp cursor
			bool bClampCursor =	FALSE;

			iDipCursorPos ++;

			if(	iDipCursorPos > m_iWindowMiddleDip )
			{
				// clamp cursor	position
				bClampCursor = TRUE;

				// advance gameselect
				if(iDipSelect == 0) iDipSelect += (iDipCursorPos - m_iWindowMiddleDip);
				else iDipSelect ++;

				// clamp game window range (high)
				if((iDipSelect	+ m_iMaxWindowListDip)	> iNumDips)
				{

					// clamp to	end
					iDipSelect = iNumDips - m_iMaxWindowListDip;

					// advance cursor pos after	all!
					bClampCursor = FALSE;

					// clamp cursor	to end
					if((iDipSelect	+ iDipCursorPos) >= iNumDips)
						iDipCursorPos = m_iMaxWindowListDip-1;
				}
			}

			// check for cursor	clamp
			if(bClampCursor)
				iDipCursorPos = m_iWindowMiddleDip;	
		}
		else
		{
			dipListSel++;

			if (dipListSel > m_DipListValues.size()-1)
				dipListSel = m_DipListValues.size()-1;
		}

	}
	else if(CTRL_UP(new_state & diff_state) | CTRL_L2(new_state) | CTRL_LSTICK_UP(new_state))
	{
		if (!dipList)
		{
			// default don`t clamp cursor
			bool bClampCursor =	FALSE;

			iDipCursorPos --;
			if(	iDipCursorPos < m_iWindowMiddleDip )
			{
				// clamp cursor	position
				bClampCursor = TRUE;

				// backup window pos
				iDipSelect	--;

				// clamp game window range (low)
				if(iDipSelect < 0)
				{
					// clamp to	start
					iDipSelect	= 0;

					// backup cursor pos after all!
					bClampCursor = FALSE;

					// clamp cursor	to end
					if(	iDipCursorPos < 0 )
						iDipCursorPos = 0;
				}
			}

			// check for cursor	clamp
			if(	bClampCursor )
				iDipCursorPos = m_iWindowMiddleDip;
		}
		else
		{
			dipListSel--;

			if (dipListSel < 0)
				dipListSel = 0;
		}
	}			 			 			 	 
	else if (CTRL_SQUARE(old_state & diff_state))
	{		 
		InpDIPSWResetDIPs();
		LoadDIPS();
	}


	old_state = new_state;

}

void DipMenu()
{
	int i;
	int	iTempDipSel;
	int	iDipidx;

	// draw	input list entries

	iTempDipSel		= iDipSelect;

	cellDbgFontPuts(0.05f, 0.04f , 0.75f, 0xFFE0EEFF, "FBANext PS3 - DIP Switch Menu");             
	cellDbgFontDraw();

	for(iDipidx=0; iDipidx < m_iMaxWindowListDip; iDipidx++)
	{ 
		int val = iTempDipSel++;
		sprintf(DipSetting,"%s : %s", m_DipListData[val].c_str(), m_DipList[std::string((char *)m_DipListData[val].c_str())].c_str());

		if (iDipidx==iDipCursorPos)
			cellDbgFontPuts(0.05f, 0.08f + 0.025f * ((float)iDipidx ), 0.75f, COLS, DipSetting);
		else		 
		{
			if (dipList)
				cellDbgFontPuts(0.05f, 0.08f + 0.025f * ((float)iDipidx ), 0.75f, 0xA0A0A0A0, DipSetting);
			else
				cellDbgFontPuts(0.05f, 0.08f + 0.025f * ((float)iDipidx ), 0.75f, 0xFFFFFFFF, DipSetting);
		}
		cellDbgFontDraw();	
	}

	if (bDrvOkay)
	{
		cellDbgFontPrintf(0.7f, 0.86f + 0.025f, 0.75f, 0xFFE0EEFF ,"Cross - Change Input");     
		cellDbgFontDraw();
		cellDbgFontPrintf(0.7f, 0.88f + 0.025f, 0.75f, 0xFFE0EEFF ,"Circle - Return to Game");     
		cellDbgFontDraw();
		cellDbgFontPrintf(0.7f, 0.90f + 0.025f, 0.75f, 0xFFE0EEFF ,"Square - Reset to Defaults");     
		cellDbgFontDraw();
	}

	if (dipList)
	{
		for(i=0; i < m_DipListValues.size(); i++)
		{ 		
			if (i==dipListSel)
				cellDbgFontPuts(0.5f, 0.08f + 0.025f * ((float)i ), 0.75f, COLS, m_DipListValues[i].c_str());
			else		 
				cellDbgFontPuts(0.5f, 0.08f + 0.025f * ((float)i ), 0.75f, 0xFFFFFFFF, m_DipListValues[i].c_str());

			cellDbgFontDraw();	
		}
	}

	cellDbgFontPrintf(0.05f, 0.92f + 0.025f, 0.50f, 0xFFFFE0E0, "Core %s - r%s - %s", szAppBurnVer, szSVNVer, szSVNDate);
	cellDbgFontDraw();
}


void InputMenu()
{
	int i;
	int	iTempInputSel;
	int	iInputidx;

	// draw	input list entries

	iTempInputSel		= iInputSelect;

	cellDbgFontPuts(0.05f, 0.04f , 0.75f, 0xFFE0EEFF, "FBANext PS3 - Input Mapping Menu");             
	cellDbgFontDraw();

	for(iInputidx=0; iInputidx < m_iMaxWindowListInput; iInputidx++)
	{ 
		int val = iTempInputSel++;
		sprintf(InpSetting,"%s : %s", m_InputListData[val].c_str(), m_InputList[std::string((char *)m_InputListData[val].c_str())].c_str());

		if (iInputidx==iInputCursorPos)
			cellDbgFontPuts(0.05f, 0.08f + 0.025f * ((float)iInputidx ), 0.75f, COLS, InpSetting);
		else		 
		{
			if (inputList)
				cellDbgFontPuts(0.05f, 0.08f + 0.025f * ((float)iInputidx ), 0.75f, 0xA0A0A0A0, InpSetting);
			else
				cellDbgFontPuts(0.05f, 0.08f + 0.025f * ((float)iInputidx ), 0.75f, 0xFFFFFFFF, InpSetting);
		}

		cellDbgFontDraw();	
	}

	if (bDrvOkay)
	{
		cellDbgFontPrintf(0.7f, 0.86f + 0.025f, 0.75f, 0xFFE0EEFF ,"Cross - Change Input");     
		cellDbgFontDraw();
		cellDbgFontPrintf(0.7f, 0.88f + 0.025f, 0.75f, 0xFFE0EEFF ,"Circle - Return to Game");     
		cellDbgFontDraw();
		cellDbgFontPrintf(0.7f, 0.90f + 0.025f, 0.75f, 0xFFE0EEFF ,"Square - Reset to Defaults");     
		cellDbgFontDraw();
		cellDbgFontPrintf(0.7f, 0.92f + 0.025f, 0.75f, 0xFFE0EEFF ,"Triangle - Save as Preset");     
		cellDbgFontDraw();
	}

	if (inputList)
	{
		for(i=0; i < m_InputSettingsData.size(); i++)
		{ 		
			if (i==inputListSel)
				cellDbgFontPuts(0.5f, 0.08f + 0.025f * ((float)i ), 0.75f, COLS, m_InputSettingsData[i].c_str());
			else		 
				cellDbgFontPuts(0.5f, 0.08f + 0.025f * ((float)i ), 0.75f, 0xFFFFFFFF, m_InputSettingsData[i].c_str());
			cellDbgFontDraw();	
		}
	}

	cellDbgFontPrintf(0.05f, 0.92f + 0.025f, 0.50f, 0xFFFFE0E0, "Core %s - r%s - %s", szAppBurnVer, szSVNVer, szSVNDate);
	cellDbgFontDraw();


}

void InputFrameMove()
{
	static uint64_t old_state;
	uint64_t new_state = cell_pad_input_poll_device(0);
	uint64_t diff_state = old_state ^ new_state;

	if (CTRL_CIRCLE(new_state))
	{
		// switch to config

		if (!inputList)
		{

			if (bDrvOkay)	// theres a game loaded, return to game
			{
				old_state = new_state;
				audio_play();
				is_running = 1;
				GameStatus = EMULATING;

				return;
			}
		}
		else
		{
			inputList = 0;
			old_state = new_state;
		}
	}
	else if (CTRL_CROSS(old_state & diff_state))
	{
		if (!inputList)
		{
			inputList = 1;
			old_state = new_state;
			return;
		}
		else
		{
			struct GameInp* pgi = NULL;

			int id = inputListSel;		

			pgi = GameInp + m_InputListOffsets[iInputSelect	+ iInputCursorPos];


			if (strstr(m_InputListData[iInputSelect	+ iInputCursorPos].c_str(), "Service"))
			{
				switch (id)
				{
					case 0:
						pgi->nInput = GIT_SWITCH;
						pgi->Input.Switch = (unsigned short)FBK_C;
						break;
					case 1:
						pgi->nInput = GIT_SWITCH;
						pgi->Input.Switch = (unsigned short)FBK_Z;
						break;
					case 2:
						pgi->nInput = GIT_SWITCH;
						pgi->Input.Switch = (unsigned short)FBK_X;
						break;
					case 3:
						pgi->nInput = GIT_SWITCH;
						pgi->Input.Switch = (unsigned short)FBK_V;
						break;
					case 4:	
						pgi->nInput = GIT_SWITCH;
						pgi->Input.Switch = (unsigned short)FBK_1;
						break;
					case 5:
						pgi->nInput = GIT_SWITCH;
						pgi->Input.Switch = (unsigned short)FBK_5;
						break;
					case 6:
						pgi->nInput = GIT_SWITCH;
						pgi->Input.Switch = (unsigned short)FBK_S;
						break;
					case 7:
						pgi->nInput = GIT_SWITCH;
						pgi->Input.Switch = (unsigned short)FBK_D;
						break;
					case 8:
						pgi->nInput = GIT_SWITCH;
						pgi->Input.Switch = (unsigned short)FBK_F1;
						break;
					case 9:
						pgi->nInput = GIT_SWITCH;
						pgi->Input.Switch = (unsigned short)FBK_F2;
						break;
					case 10: 
						pgi->nInput = GIT_SWITCH;
						pgi->Input.Switch = (unsigned short)PS3_L2_BUTTON; 
						break;
					case 11:
						pgi->nInput = GIT_SWITCH;
						pgi->Input.Switch = (unsigned short)PS3_R2_BUTTON;			 
						break;
				}
			}

			else if (strstr(m_InputListData[iInputSelect+iInputCursorPos].c_str(), "P1"))
			{
				switch (id)
				{
					case 0:
						KEY(FBK_C);
						break;
					case 1:
						KEY(FBK_Z);
						break;
					case 2:
						KEY(FBK_X);
						break;
					case 3:
						KEY(FBK_V);
						break;
					case 4:	
						KEY(FBK_1);
						break;
					case 5:
						KEY(FBK_5);
						break;
					case 6:
						KEY(FBK_S);
						break;
					case 7:
						KEY(FBK_D);
						break;
					case 8:
						KEY(FBK_F1);
						break;
					case 9:
						KEY(FBK_F2);
						break;
					case 10: 
						KEY(PS3_L2_BUTTON);
						break;
					case 11:
						KEY(PS3_R2_BUTTON);
						break;
					case 12:
						KEY(PS3_L3_BUTTON);
						break;
					case 13:
						KEY(PS3_R3_BUTTON);
						break;
				}
			}

			else if (strstr(m_InputListData[iInputSelect+iInputCursorPos].c_str(), "P2"))
			{
				switch (id)
				{
					case 0:
						KEY(0x4082);
						break;
					case 1:
						KEY(0x4080);
						break;
					case 2:
						KEY(0x4081);
						break;
					case 3:
						KEY(0x4083);
						break;
					case 4:	
						KEY(0x03);
						break;
					case 5:
						KEY(0x07);
						break;
					case 6:
						KEY(0x4084);
						break;
					case 7:
						KEY(0x4085);
						break;
					case 8:
						KEY(PS3_L3_BUTTON | 0x4000);
						break;
					case 9:
						KEY(PS3_R3_BUTTON | 0x4000);
						break;
					case 10: 
						KEY(PS3_L2_BUTTON | 0x4000);
						break;
					case 11:
						KEY(PS3_R2_BUTTON | 0x4000);
						break;
				}
			}

			else if (strstr(m_InputListData[iInputSelect+iInputCursorPos].c_str(), "P3"))
			{
				switch (id)
				{
					case 0:
						KEY(0x4182);
						break;
					case 1:				 
						KEY(0x4180);
						break;
					case 2:
						KEY(0x4181);
						break;
					case 3:
						KEY(0x4183);
						break; 
					case 4:	
						KEY(0x04);
						break;
					case 5:
						KEY(0x08);
						break;
					case 6:
						KEY(0x4184);
						break;
					case 7:
						KEY(0x4185);
						break;
					case 8:
						KEY(PS3_L3_BUTTON | 0x4100);
						break;
					case 9:
						KEY(PS3_R3_BUTTON | 0x4100);
						break;
					case 10: 
						KEY(PS3_L2_BUTTON | 0x4100);
						break;
					case 11:
						KEY(PS3_R2_BUTTON | 0x4100);
						break;
				}
			}

			else if (strstr(m_InputListData[iInputSelect+iInputCursorPos].c_str(), "P4"))
			{
				switch (id)
				{
					case 0:
						KEY(0x4282);
						break;
					case 1:
						KEY(0x4280);
						break;
					case 2:
						KEY(0x4281);
						break;
					case 3:
						KEY(0x4283);
						break;
					case 4:	
						KEY(0x05);
						break;
					case 5:
						KEY(0x09);
						break;
					case 6:
						KEY(0x4284);
						break;
					case 7:
						KEY(0x4285);
						break;
					case 8:
						KEY(PS3_L3_BUTTON | 0x4200);
						break;
					case 9:
						KEY(PS3_R3_BUTTON | 0x4200);
						break;
					case 10: 
						KEY(PS3_L2_BUTTON | 0x4200);
						break;
					case 11:
						KEY(PS3_R2_BUTTON | 0x4200);
						break;
				}
			}
			else
			{
				switch (id)
				{
					case 0:
						pgi->nInput = GIT_SWITCH;
						pgi->Input.Switch = (unsigned short)FBK_C;
						break;
					case 1:
						pgi->nInput = GIT_SWITCH;
						pgi->Input.Switch = (unsigned short)FBK_Z;
						break;
					case 2:
						pgi->nInput = GIT_SWITCH;
						pgi->Input.Switch = (unsigned short)FBK_X;
						break;
					case 3:
						pgi->nInput = GIT_SWITCH;
						pgi->Input.Switch = (unsigned short)FBK_V;
						break;
					case 4:	
						pgi->nInput = GIT_SWITCH;
						pgi->Input.Switch = (unsigned short)FBK_1;
						break;
					case 5:
						pgi->nInput = GIT_SWITCH;
						pgi->Input.Switch = (unsigned short)FBK_5;
						break;
					case 6:
						pgi->nInput = GIT_SWITCH;
						pgi->Input.Switch = (unsigned short)FBK_S;
						break;
					case 7:
						pgi->nInput = GIT_SWITCH;
						pgi->Input.Switch = (unsigned short)FBK_D;
						break;
					case 8:
						pgi->nInput = GIT_SWITCH;
						pgi->Input.Switch = (unsigned short)PS3_L3_BUTTON;
						break;
					case 9:
						pgi->nInput = GIT_SWITCH;
						pgi->Input.Switch = (unsigned short)PS3_R3_BUTTON;
						break;
					case 10: 
						pgi->nInput = GIT_SWITCH;
						pgi->Input.Switch = (unsigned short)PS3_L2_BUTTON; 
						break;
					case 11:
						pgi->nInput = GIT_SWITCH;
						pgi->Input.Switch = (unsigned short)PS3_R2_BUTTON;			 
						break;
				}
			}

			LoadInputs();
			inputList = 0;
			old_state = new_state;
		}
	}				 
	else if(CTRL_DOWN(new_state & diff_state) | CTRL_R2(new_state) | CTRL_LSTICK_DOWN(new_state))
	{
		sys_timer_usleep(FILEBROWSER_DELAY);
		if (!inputList)
		{
			bool bClampCursor = FALSE;		// default don`t clamp cursor

			iInputCursorPos ++;

			if(iInputCursorPos > m_iWindowMiddleInput)
			{
				bClampCursor = TRUE;		// clamp cursor position

				// advance gameselect
				if(iInputSelect == 0)
					iInputSelect += (iInputCursorPos - m_iWindowMiddleInput);
				else
					iInputSelect ++;

				// clamp game window range (high)
				if((iInputSelect + m_iMaxWindowListInput) > iNumInput)
				{
					iInputSelect = iNumInput - m_iMaxWindowListInput;	// clamp to end

					bClampCursor = FALSE;					// advance cursor pos after all!

					// clamp cursor	to end
					if((iInputSelect + iInputCursorPos) >= iNumInput)
						iInputCursorPos = m_iMaxWindowListInput-1;
				}
			}

			// check for cursor	clamp
			if(bClampCursor)
				iInputCursorPos = m_iWindowMiddleInput;	
		}
		else
		{
			inputListSel++;

			if (inputListSel > m_InputSettingsData.size()-1)
				inputListSel = m_InputSettingsData.size()-1;
		}
		old_state = new_state;

	}
	else if(CTRL_UP(new_state & diff_state) | CTRL_L2(new_state) | CTRL_LSTICK_UP(new_state)) 
	{
		sys_timer_usleep(FILEBROWSER_DELAY);
		if (!inputList)
		{
			// default don`t clamp cursor
			bool bClampCursor =	FALSE;

			iInputCursorPos --;
			if(iInputCursorPos < m_iWindowMiddleInput)
			{
				// clamp cursor	position
				bClampCursor = TRUE;

				// backup window pos
				iInputSelect	--;

				// clamp game window range (low)
				if(iInputSelect < 0)
				{
					// clamp to	start
					iInputSelect	= 0;

					// backup cursor pos after all!
					bClampCursor = FALSE;

					// clamp cursor	to end
					if(iInputCursorPos < 0)
						iInputCursorPos = 0;
				}
			}

			// check for cursor	clamp
			if(bClampCursor)
				iInputCursorPos = m_iWindowMiddleInput;
		}
		else
		{
			inputListSel--;

			if (inputListSel < 0)
				inputListSel = 0;
		}
		old_state = new_state;
	}			 			 			 
	else if (CTRL_TRIANGLE(old_state & diff_state))
	{
		if (SaveDefaultInput()==0)
		{
			dialog_is_running = true;
			cellMsgDialogOpen2(CELL_MSGDIALOG_DIALOG_TYPE_NORMAL| \
					CELL_MSGDIALOG_TYPE_BG_VISIBLE| \
					CELL_MSGDIALOG_TYPE_BUTTON_TYPE_NONE|CELL_MSGDIALOG_TYPE_DISABLE_CANCEL_OFF|\
					CELL_MSGDIALOG_TYPE_DEFAULT_CURSOR_OK,\
					"Input Preset saved.",cb_dialog_ok,NULL,NULL);
			while(dialog_is_running)
			{
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
				psglSwap();
				cellSysutilCheckCallback();	
			}
		}
		else
		{
			dialog_is_running = true;
			cellMsgDialogOpen2(CELL_MSGDIALOG_TYPE_SE_TYPE_ERROR| \
					CELL_MSGDIALOG_TYPE_BG_VISIBLE| \
					CELL_MSGDIALOG_TYPE_BUTTON_TYPE_NONE|CELL_MSGDIALOG_TYPE_DISABLE_CANCEL_OFF|\
					CELL_MSGDIALOG_TYPE_DEFAULT_CURSOR_OK,\
					"Error saving Input Preset.",cb_dialog_ok,NULL,NULL);
			while(dialog_is_running)
			{
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
				psglSwap();
				cellSysutilCheckCallback();	
			}
		}
		old_state = new_state;

	}
	else if (CTRL_SQUARE(old_state & diff_state))
	{
		struct GameInp* pgi = NULL;
		unsigned int i;

		for (i = 0, pgi = GameInp; i < nGameInpCount; i++, pgi++)
		{
			struct BurnInputInfo bii;

			// Get the extra info about the input
			bii.szInfo = NULL;
			BurnDrvGetInputInfo(&bii, i);
			if (bii.pVal == NULL)
				continue;
			if (bii.szInfo == NULL)
				bii.szInfo = "";
			GamcPlayer(pgi, (char*)bii.szInfo, 0, -1);	// Keyboard
			GamcMisc(pgi, (char*)bii.szInfo, 0);
		}	

		old_state = new_state;
		LoadInputs();
	}

	old_state = new_state;
}


void InGameMenu()
{
	cellDbgFontPuts(0.05f, 0.04f , 0.75f, 0xFFE0EEFF, "FBANext PS3 - In Game Menu");             
	cellDbgFontDraw();

	int number = 0;
	cellDbgFontPuts(0.05f, 0.08f + 0.025f * ((float)number), 0.75f, inGameIndex == INGAME_MAP_BUTTONS ? COLS : 0xFFFFFFFF, "Map Gamepad Buttons" );     
	cellDbgFontDraw();
	number++;

	cellDbgFontPuts(0.05f, 0.08f + 0.025f * ((float)number), 0.75f, inGameIndex == INGAME_DIP_SWITCHES ? COLS : 0xFFFFFFFF, "Map Dip Switches");
	cellDbgFontDraw(); 
	number++;

	cellDbgFontPuts(0.05f, 0.08f + 0.025f * ((float)number), 0.75f, inGameIndex == INGAME_RESIZE_SCREEN ? COLS : 0xFFFFFFFF, "Resize Screen");
	cellDbgFontDraw();
	number++;

	cellDbgFontPuts(0.05f, 0.08f + 0.025f * ((float)number), 0.75f, inGameIndex == INGAME_FRAME_ADVANCE ? COLS : 0xFFFFFFFF, "Frame Advance");
	cellDbgFontDraw();
	number++;

	cellDbgFontPrintf(0.05f, 0.08f + 0.025f * ((float)number), 0.75f, inGameIndex == INGAME_FBO_ENABLED ? COLS : 0xFFFFFFFF, "Custom Scaling/Dual Shaders: %s", bVidFBOEnabled ? "Yes" : "No");
	cellDbgFontDraw();	
	number++;

	cellDbgFontPrintf(0.05f, 0.08f + 0.025f * ((float)number), 0.75f, inGameIndex == INGAME_CURRENT_SHADER ? COLS : 0xFFFFFFFF, "Current Shader #1: %s", m_ListShaderData[shaderindex].c_str());
	cellDbgFontDraw();	
	number++;

	cellDbgFontPrintf(0.05f, 0.08f + 0.025f * ((float)number), 0.75f, inGameIndex == INGAME_CURRENT_SHADER2 ? COLS : 0xFFFFFFFF, "Current Shader #2: %s", m_ListShader2Data[shaderindex2].c_str());
	cellDbgFontDraw();	
	number++;

	char msg[256];
	switch(nVidScrnAspectMode)
	{
		case ASPECT_RATIO_CUSTOM:
			sprintf(msg,"Custom (Resized)");
			break;
		case ASPECT_RATIO_AUTO:
			sprintf(msg,"Auto %d:%d", nVidScrnAspectX, nVidScrnAspectY);
			break;
		case ASPECT_RATIO_AUTO_FBA: 
			sprintf(msg,"Auto (FBA) %d:%d", nVidScrnAspectX, nVidScrnAspectY);
			break;
		default:
			sprintf(msg,"%d:%d", nVidScrnAspectX, nVidScrnAspectY);
			break;
	}
	cellDbgFontPrintf(0.05f, 0.08f + 0.025f * ((float)number), 0.75f, inGameIndex == INGAME_KEEP_ASPECT ? COLS : 0xFFFFFFFF, "Aspect Ratio : %s", msg);
	cellDbgFontDraw();
	number++;

	cellDbgFontPrintf(0.05f, 0.08f + 0.025f * ((float)number), 0.75f, inGameIndex == INGAME_BILINEAR_FILTER ? COLS : 0xFFFFFFFF, "Hardware Filter Shader #1: %s", vidFilterLinear ? "Linear" : "Point");
	cellDbgFontDraw();
	number++;

	cellDbgFontPrintf(0.05f, 0.08f + 0.025f * ((float)number), 0.75f, inGameIndex == INGAME_BILINEAR_FILTER2 ? COLS : 0xFFFFFFFF, "Hardware Filter Shader #2: %s", vidFilterLinear2 ? "Linear" : "Point");
	cellDbgFontDraw();
	number++;

	cellDbgFontPrintf(0.05f, 0.08f + 0.025f * ((float)number), 0.75f, inGameIndex == INGAME_SCALING_FACTOR ? COLS : 0xFFFFFFFF, "Scaling Factor: %dx", bVidScalingFactor);
	cellDbgFontDraw();
	number++;

	char rotatemsg[3][256] = {{"Rotate for Vertical Games"},{"Do not rotate for Vertical Games"},{"Reverse flipping for vertical games"}};
	cellDbgFontPrintf(0.05f, 0.08f + 0.025f * ((float)number), 0.75f, inGameIndex == INGAME_ROTATE ? COLS : 0xFFFFFFFF, "Rotation Adjust: %s", rotatemsg[nVidRotationAdjust]);     
	cellDbgFontDraw();
	number++;

	cellDbgFontPrintf(0.05f, 0.08f + 0.025f * ((float)number), 0.75f, inGameIndex == INGAME_SAVE_STATE ? COLS : 0xFFFFFFFF, "Save State #%d", save_state_slot);
	cellDbgFontDraw();
	number++;

	cellDbgFontPrintf(0.05f, 0.08f + 0.025f * ((float)number), 0.75f, inGameIndex == INGAME_LOAD_STATE ? COLS : 0xFFFFFFFF, "Load State #%d", save_state_slot);
	cellDbgFontDraw();
	number++;

	cellDbgFontPuts(0.05f, 0.08f + 0.025f * ((float)number), 0.75f, inGameIndex == INGAME_RESET_GAME ? COLS : 0xFFFFFFFF, "Reset Game");
	cellDbgFontDraw();
	number++;

#ifdef MULTIMAN_SUPPORT
	cellDbgFontPuts(0.05f, 0.08f + 0.025f * ((float)number), 0.75f, inGameIndex == INGAME_RETURN_TO_MULTIMAN ? COLS : 0xFFFFFFFF, "Return to multiMAN");
	cellDbgFontDraw();
	number++;
#endif

	cellDbgFontPuts(0.05f, 0.08f + 0.025f * ((float)number), 0.75f, inGameIndex == INGAME_EXIT_GAME ? COLS : 0xFFFFFFFF, "Exit Game");
	cellDbgFontDraw();
	number++;

	cellDbgFontPuts(0.05f, 0.08f + 0.025f * ((float)number), 0.75f, inGameIndex == INGAME_BACK_TO_GAME ? COLS : 0xFFFFFFFF, "Return to current Game");
	cellDbgFontDraw();

	cellDbgFontPrintf(0.05f, 0.92f + 0.025f, 0.50f, 0xFFFFE0E0, "Core %s - r%s - %s", szAppBurnVer, szSVNVer, szSVNDate);
	cellDbgFontDraw();
}


void StretchMenu()
{
	cellDbgFontPuts(0.05f, 0.04f , 0.75f, 0xFFE0EEFF, "FBANext PS3 - Resize Screen Menu");             
	cellDbgFontDraw();

	cellDbgFontPrintf(0.7f, 0.86f + 0.025f, 0.75f, 0xFFE0EEFF ,"Triangle - Reset to Default");     
	cellDbgFontDraw();
	cellDbgFontPrintf(0.7f, 0.88f + 0.025f, 0.75f, 0xFFE0EEFF ,"Circle - Return to Game");     
	cellDbgFontDraw();

	cellDbgFontPrintf(0.05f, 0.92f + 0.025f, 0.50f, 0xFFFFE0E0, "Core %s - r%s - %s", szAppBurnVer, szSVNVer, szSVNDate);
	cellDbgFontDraw();
}


void InGameFrameMove()
{
	int nRet = 0;
	static uint64_t old_state = 0;
	uint64_t new_state = cell_pad_input_poll_device(0);
	uint64_t diff_state = old_state ^ new_state;

	if (CTRL_CIRCLE(old_state & diff_state))
	{
		// switch to config

		if (bDrvOkay)	// theres a game loaded, return to game
		{
			old_state = new_state;
			audio_play();
			is_running = 1;
			GameStatus = EMULATING;
			return;
		}		
	}
	else if(CTRL_DOWN(new_state & diff_state) | CTRL_LSTICK_DOWN(new_state))
	{				 
		if(inGameIndex < LAST_INGAME_SETTING)
		{
			inGameIndex++;
			sys_timer_usleep(FILEBROWSER_DELAY);
		}
	}
	else if(CTRL_UP(new_state & diff_state) | CTRL_LSTICK_UP(new_state))
	{		 
		if(inGameIndex > 0)
		{
			inGameIndex--;
			sys_timer_usleep(FILEBROWSER_DELAY);
		}
	}

	switch(inGameIndex)
	{
		case INGAME_MAP_BUTTONS:	
			if(CTRL_CROSS(old_state & diff_state))
			{
				if (bDrvOkay)	
				{
					LoadInputs();
					GameStatus = INPUT_MENU;
				}
			}
			break;
		case INGAME_DIP_SWITCHES:
			if(CTRL_CROSS(old_state & diff_state))
			{
				if (bDrvOkay)
				{
					LoadDIPS();
					GameStatus = DIP_MENU;
				}
			}
			break;		 
		case INGAME_RESIZE_SCREEN:
			if(CTRL_CROSS(old_state & diff_state))
			{
				if (bDrvOkay)
				{
					GameStatus = SCREEN_RESIZE;
				}
			}
			break;
		case INGAME_FRAME_ADVANCE:
			if(CTRL_CROSS(old_state & diff_state) || CTRL_R2(new_state))
			{
				old_state = new_state;
				audio_play();
				is_running = 0;
				GameStatus = EMULATING;
			}
			break;
		case INGAME_FBO_ENABLED:
			if(CTRL_LEFT(new_state & diff_state) | CTRL_RIGHT(new_state & diff_state) | CTRL_CROSS(old_state & diff_state))
				bVidFBOEnabled = !bVidFBOEnabled;
			if(CTRL_CROSS(old_state & diff_state))
			{
				//FBO mode needs to be applied here
				//Screen reinited
			}
			break;
		case INGAME_CURRENT_SHADER:
			if(CTRL_LEFT(new_state & diff_state) | CTRL_LSTICK_LEFT(new_state))
			{
				if(shaderindex > 0)
					shaderindex--;
			}

			if(CTRL_RIGHT(new_state & diff_state) | CTRL_LSTICK_RIGHT(new_state))
			{
				if(shaderindex < m_ListShaderData.size()-1)
					shaderindex ++;
			}
			if(CTRL_CROSS(old_state & diff_state))
			{
				selectedShader[0].index = shaderindex;
				strcpy(selectedShader[0].filename, m_ListShaderData[shaderindex].c_str());
				sprintf(selectedShader[0].fullpath, SHADER_DIRECTORY, m_ListShaderData[shaderindex].c_str());
				psglInitShader(selectedShader[0].fullpath);
				BurnReinitScrn();
				psglRedraw();
			}
			break;
		case INGAME_CURRENT_SHADER2:
			if(CTRL_LEFT(new_state & diff_state) | CTRL_LSTICK_LEFT(new_state))
			{
				if(shaderindex2 > 0)
					shaderindex2--;
			}

			if(CTRL_RIGHT(new_state & diff_state) | CTRL_LSTICK_RIGHT(new_state))
			{
				if(shaderindex2 < m_ListShader2Data.size()-1)
					shaderindex2++;
			}
			if(CTRL_CROSS(old_state & diff_state))
			{
				selectedShader[1].index = shaderindex2;
				strcpy(selectedShader[1].filename, m_ListShader2Data[shaderindex2].c_str());
				sprintf(selectedShader[1].fullpath, SHADER_DIRECTORY, m_ListShader2Data[shaderindex2].c_str());
				psglInitShader(selectedShader[1].fullpath);
				BurnReinitScrn();
				psglRedraw();
			}
			break;
		case INGAME_KEEP_ASPECT:
			if(CTRL_LEFT(new_state & diff_state))
			{
				if(nVidScrnAspectMode > 0)
				{
					nVidScrnAspectMode--;
					setWindowAspect(0);
					BurnReinitScrn();
					psglRedraw();
				}
			}
			else if(CTRL_RIGHT(new_state & diff_state) | CTRL_CROSS(old_state & diff_state))
			{
				if(nVidScrnAspectMode < LAST_ASPECT_RATIO)
				{
					nVidScrnAspectMode++;
					setWindowAspect(0);
					BurnReinitScrn();
					psglRedraw();
				}
			}
			break;
		case INGAME_BILINEAR_FILTER:
			if(CTRL_LEFT(new_state & diff_state) | CTRL_RIGHT(new_state & diff_state) | CTRL_CROSS(old_state & diff_state))
			{
				vidFilterLinear = !vidFilterLinear;
				setlinear(vidFilterLinear);
				psglRedraw();
			}
			break;
		case INGAME_BILINEAR_FILTER2:
			if(CTRL_LEFT(new_state & diff_state) | CTRL_RIGHT(new_state & diff_state) | CTRL_CROSS(old_state & diff_state))
			{
				vidFilterLinear2 = !vidFilterLinear2;
				setlinear(vidFilterLinear2);
				psglRedraw();
			}
			break;
		case INGAME_SCALING_FACTOR:
			if(CTRL_LEFT(new_state & diff_state) | CTRL_LSTICK_LEFT(new_state))
			{
				if(bVidScalingFactor > 1)
					bVidScalingFactor--;
			}

			if(CTRL_RIGHT(new_state & diff_state) | CTRL_LSTICK_RIGHT(new_state))
			{
				if(bVidScalingFactor < SCALING_FACTOR_LIMIT)
					bVidScalingFactor++;
			}
			if(CTRL_CROSS(old_state & diff_state))
			{
				//apply scale here, and reapply FBO if activated
				//reapply screen here
				BurnReinitScrn();
				psglRedraw();
			}
			break;
		case INGAME_ROTATE:
			if(CTRL_LEFT(new_state & diff_state) | CTRL_RIGHT(new_state & diff_state) | CTRL_CROSS(old_state & diff_state))
			{
				nVidRotationAdjust++;

				if (nVidRotationAdjust > 2)
					nVidRotationAdjust = 0;
				BurnReinitScrn();	//apply_rotation_settings();
				psglRedraw();
			}
			break;
		case INGAME_SAVE_STATE:	
			if(CTRL_LEFT(new_state & diff_state) | CTRL_LSTICK_LEFT(new_state)) 
			{
				if(save_state_slot > 0)
					save_state_slot--;
			} 

			if(CTRL_RIGHT(new_state & diff_state) | CTRL_LSTICK_RIGHT(new_state)) 
				save_state_slot++;

			if(CTRL_CROSS(old_state & diff_state))
			{
				if (StatedSave(save_state_slot)==0)
				{			 
					dialog_is_running = true;
					cellMsgDialogOpen2(CELL_MSGDIALOG_DIALOG_TYPE_NORMAL| \
							CELL_MSGDIALOG_TYPE_BG_VISIBLE| \
							CELL_MSGDIALOG_TYPE_BUTTON_TYPE_NONE|CELL_MSGDIALOG_TYPE_DISABLE_CANCEL_OFF|\
							CELL_MSGDIALOG_TYPE_DEFAULT_CURSOR_OK,\
							"State saved successfully.",cb_dialog_ok,NULL,NULL);
					while(dialog_is_running)
					{
						glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
						psglSwap();
						cellSysutilCheckCallback();	
					}
				}
				else
				{
					dialog_is_running = true;
					cellMsgDialogOpen2(CELL_MSGDIALOG_TYPE_SE_TYPE_ERROR| \
							CELL_MSGDIALOG_TYPE_BG_VISIBLE| \
							CELL_MSGDIALOG_TYPE_BUTTON_TYPE_NONE|CELL_MSGDIALOG_TYPE_DISABLE_CANCEL_OFF|\
							CELL_MSGDIALOG_TYPE_DEFAULT_CURSOR_OK,\
							"Error saving state.",cb_dialog_ok,NULL,NULL);
					while(dialog_is_running)
					{
						glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
						psglSwap();
						cellSysutilCheckCallback();	
					}
				}
			}
			break;
		case INGAME_LOAD_STATE:	
			if(CTRL_LEFT(new_state & diff_state) | CTRL_LSTICK_LEFT(new_state)) 
			{
				if(save_state_slot > 0)
					save_state_slot--;
			} 

			if(CTRL_RIGHT(new_state & diff_state) | CTRL_LSTICK_RIGHT(new_state)) 
				save_state_slot++;

			if(CTRL_CROSS(old_state & diff_state))
			{
				nRet = StatedLoad(save_state_slot);	

				if (bDrvOkay)	// theres a game loaded, return to game
				{
					old_state = new_state;
					audio_play();
					GameStatus = EMULATING;				 
				}		
			}
			break;
		case INGAME_RESET_GAME:

			if(CTRL_CROSS(old_state & diff_state))
			{
				DoReset = true;
				if (bDrvOkay)	// theres a game loaded, return to game
				{
					old_state = new_state;
					audio_play();
					is_running = 1;
					GameStatus = EMULATING;				 
				}		
			}

			break;
		case INGAME_EXIT_GAME:	
			if(CTRL_CROSS(old_state & diff_state))
			{
				if (bDrvOkay)
				{
					if (nPrevGame < nBurnDrvCount) 
					{				
						old_state = new_state;
						nBurnDrvSelect = nPrevGame;
						is_running = 0;

						nPrevGame = ~0U;			 
						audio_stop();
						BurnerDrvExit();				// Make sure any game driver is exited
						mediaExit();					// Exit media

						GameStatus = MENU;	
					}
				}
			}
			break;
#ifdef MULTIMAN_SUPPORT
		case INGAME_RETURN_TO_MULTIMAN:	
			if(CTRL_CROSS(old_state & diff_state))
			{
				configAppSaveXml();
				sys_spu_initialize(6, 0);
				char multiMAN[512];
				sprintf(multiMAN, "%s", MULTIMAN_SELF);
				sys_game_process_exitspawn2((char*) multiMAN, NULL, NULL, NULL, 0, 2048, SYS_PROCESS_PRIMARY_STACK_SIZE_64K);		
				sys_process_exit(0);
			}
			break;
#endif
		case INGAME_BACK_TO_GAME:	
			if(CTRL_CROSS(old_state & diff_state))
			{			 
				if (bDrvOkay)	// theres a game loaded, return to game
				{
					old_state = new_state;
					audio_play();
					is_running = 1;
					GameStatus = EMULATING;				 
				}		
			}
			break;		 
	}

	//InGameMenu();
	old_state = new_state;

}
 
void FrameMove()
{ 
	static uint64_t old_state = 0;
	uint64_t new_state = cell_pad_input_poll_device(0);
	uint64_t diff_state = old_state ^ new_state;

	if(CTRL_R2(new_state))
	{
		// default don`t clamp cursor
		bool bClampCursor =	FALSE;

		iCursorPos ++;

		if(iCursorPos > m_iWindowMiddle)
		{
			bClampCursor = TRUE;					// clamp cursor	position

			// advance gameselect
			if(iGameSelect == 0)
				iGameSelect += (iCursorPos	- m_iWindowMiddle);
			else
				iGameSelect ++;

			// clamp game window range (high)
			if((iGameSelect	+ m_iMaxWindowList) > iNumGames)
			{

				iGameSelect = iNumGames - m_iMaxWindowList;	// clamp to end
				bClampCursor = FALSE;				// advance cursor pos after all!

				// clamp cursor	to end
				if((iGameSelect	+ iCursorPos) >= iNumGames)
					iCursorPos = m_iMaxWindowList-1;
			}
		}

		// check for cursor clamp
		if(	bClampCursor )
			iCursorPos = m_iWindowMiddle;	
	}

	if( CTRL_DOWN(new_state & diff_state) || CTRL_LSTICK_DOWN(new_state))
	{
		sys_timer_usleep(FILEBROWSER_DELAY);
		// default don`t clamp cursor
		bool bClampCursor =	FALSE;

		iCursorPos ++;

		if(iCursorPos > m_iWindowMiddle)
		{
			// clamp cursor	position
			bClampCursor = TRUE;

			// advance gameselect
			if(iGameSelect == 0)
				iGameSelect +=	(iCursorPos - m_iWindowMiddle);
			else
				iGameSelect ++;

			// clamp game window range (high)
			if((iGameSelect	+ m_iMaxWindowList) > iNumGames)
			{
				// clamp to	end
				iGameSelect = iNumGames	- m_iMaxWindowList;

				// advance cursor pos after	all!
				bClampCursor = FALSE;

				// clamp cursor	to end
				if((iGameSelect	+ iCursorPos) >= iNumGames)
					iCursorPos = m_iMaxWindowList-1;
			}
		}

		// check for cursor	clamp
		if(	bClampCursor )
			iCursorPos = m_iWindowMiddle;	

	}
	else if (CTRL_L1(old_state & diff_state))
	{
		CurrentFilter--;

		if (CurrentFilter < 0)
			CurrentFilter = 14;

		BuildRomList();

	}
	else if (CTRL_R1(old_state & diff_state))
	{
		CurrentFilter++;

		if (CurrentFilter > 14)
			CurrentFilter = 0;

		BuildRomList();

	}
	else if (CTRL_CIRCLE(new_state))
	{
		if (GameStatus == PAUSE)
		{
			// switch back to emulation
			old_state = new_state;
			audio_play();
			is_running = 1;
			GameStatus = EMULATING;
			return;
		}
	}
	else if (CTRL_SELECT(old_state & diff_state))
	{
		if (!bDrvOkay)
		{
			// switch to config
			GameStatus = CONFIG_MENU;			 
		}
	}
	else if (CTRL_START(old_state & diff_state))
	{
		if (bDrvOkay)
		{
			if (nPrevGame < nBurnDrvCount) 
			{				
				nBurnDrvSelect = nPrevGame;

				nPrevGame = ~0U;			 
				audio_stop();
				BurnerDrvExit();		// Make sure any game driver is exited
				mediaExit();			// Exit media
			}
		}
	}
	else if (CTRL_TRIANGLE(old_state & diff_state))
	{
		HideChildren = !HideChildren;
		BuildRomList();			 					 
	}
	else if (CTRL_SQUARE(old_state & diff_state))
	{
		ThreeOrFourPlayerOnly = !ThreeOrFourPlayerOnly; 
		BuildRomList();	

	}
	else if( CTRL_UP(new_state & diff_state) | CTRL_LSTICK_UP(new_state))  
	{
		// default don`t clamp cursor
		sys_timer_usleep(FILEBROWSER_DELAY);
		bool bClampCursor =	FALSE;

		iCursorPos --;
		if(iCursorPos < m_iWindowMiddle)
		{
			bClampCursor = TRUE;		// clamp cursor	position
			iGameSelect	--;		// backup window pos
			if(iGameSelect < 0)		// clamp game window range (low)
			{
				iGameSelect	= 0;		// clamp to	start
				bClampCursor = FALSE;		// backup cursor pos after all!

				// clamp cursor	to end
				if(	iCursorPos < 0 )
					iCursorPos = 0;
			}
		}

		// check for cursor	clamp
		if(	bClampCursor )
			iCursorPos = m_iWindowMiddle;
	}			 			 			 
	else if(CTRL_L2(new_state)) 
	{
		// default don`t clamp cursor
		bool bClampCursor =	FALSE;

		iCursorPos --;
		if(iCursorPos < m_iWindowMiddle)
		{
			bClampCursor = TRUE;		// clamp cursor	position
			iGameSelect	--;		// backup window pos

			// clamp game window range (low)
			if(iGameSelect < 0)
			{
				iGameSelect	= 0;			// clamp to	start
				bClampCursor = FALSE;			// backup cursor pos after all!

				// clamp cursor	to end
				if(	iCursorPos < 0 )
					iCursorPos = 0;
			}
		}

		// check for cursor clamp
		if(bClampCursor)
			iCursorPos = m_iWindowMiddle;
	}			 			 			 
	else if(CTRL_CROSS(new_state))
	{
		// initalise emulation here and set emulating to true
		int entryselected = iGameSelect	+ iCursorPos;

		if (iNumGames >	0)
		{
			nBurnDrvSelect = (unsigned int)m_vecAvailRomBurnDrvIndex[entryselected];

			if (nPrevGame == nBurnDrvSelect)
			{
				// same game, do nothing
				old_state = new_state;
				audio_play();
				GameStatus = EMULATING;	
				return;
			}	

			if (bDrvOkay)
			{
				if ( nPrevGame < nBurnDrvCount) 
				{				
					nBurnDrvSelect = nPrevGame;

					nPrevGame = ~0U;			 
					audio_stop();
					BurnerDrvExit();	// Make sure any game driver is exited
					mediaExit();		// Exit media
				}
			}

			nBurnFPS = 6000;			// Hardcoded FPS
			nFMInterpolation = 0;			// FM Interpolation hardcoded to 0

			if (directLoadGame(m_vecAvailRomIndex[entryselected].c_str()) == 0)
			{
				nPrevGame = m_vecAvailRomBurnDrvIndex[entryselected];
				//nCurrentBurnDrvSelect = nBurnDrvSelect;
				nLastRom = entryselected;
				nLastFilter = CurrentFilter;
				GameStatus = EMULATING;	
				return;

			}
			else
			{
				nBurnDrvSelect = nPrevGame;

				nPrevGame = ~0U;			 
				audio_stop();
				BurnerDrvExit();				// Make sure any game driver is exited
				mediaExit();					// Exit media
			}

			nPrevGame = nBurnDrvSelect;
		}
	}

	old_state = new_state;
}
