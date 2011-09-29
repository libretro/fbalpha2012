#include <sysutil/sysutil_msgdialog.h>
#include <sysutil/sysutil_sysparam.h>
#include <cell/dbgfont.h>
#include <cell/cell_fs.h>
#include <cell/codec/pngdec.h>
#include <algorithm>
#include <vector>
#include <map>
#include "menu.h"
#include "burner.h"
#include "../../interface/PS3/cellframework2/input/pad_input.h"
#include "vid_psgl.h"
#include "inp_keys.h"

#define FILEBROWSER_DELAY	180000
 
#define ALL 0
#define CPS1 1
#define CPS2 2
#define CPS3 3
#define NEOGEO 4
#define TAITO 5
#define SEGA 6
#define PGM 7
#define PSYKIO 8
#define KONAMI 9
#define KANEKO 10
#define CAVE 11
#define TOAPLAN 12
#define SEGAMD 13
#define MISC 14

#define MASKMISC		(1 << (HARDWARE_PREFIX_MISC		>> 24))
#define MASKCPS			(1 << (HARDWARE_PREFIX_CPS1		>> 24))
#define MASKNEOGEO		(1 << (HARDWARE_PREFIX_SNK		>> 24))
#define MASKSEGA		(1 << (HARDWARE_PREFIX_SEGA		>> 24))
#define MASKTOAPLAN		(1 << (HARDWARE_PREFIX_TOAPLAN	>> 24))
#define MASKCAVE		(1 << (HARDWARE_PREFIX_CAVE		>> 24))
#define MASKCPS2		(1 << (HARDWARE_PREFIX_CPS2		>> 24))
#define MASKMD			(1 << (HARDWARE_PREFIX_SEGAMD	>> 24))
#define MASKPGM			(1 << (HARDWARE_PREFIX_PGM		>> 24))
#define MASKCPS3		(1 << (HARDWARE_PREFIX_CPS3		>> 24))
#define MASKTAITO		(1 << (HARDWARE_PREFIX_TAITO	>> 24))
#define MASKPSIKYO		(1 << (HARDWARE_PREFIX_PSIKYO	>> 24))
#define MASKKANEKO16	(1 << (HARDWARE_PREFIX_KANEKO16	>> 24))
#define MASKKONAMI		(1 << (HARDWARE_PREFIX_KONAMI	>> 24))
#define MASKPACMAN		(1 << (HARDWARE_PREFIX_PACMAN	>> 24))
#define MASKGALAXIAN	(1 << (HARDWARE_PREFIX_GALAXIAN >> 24))
#define MASKATARI		(1 << (HARDWARE_PREFIX_ATARI	>> 24))

#define MASKALL \
	(MASKMISC | MASKCPS | MASKNEOGEO | MASKSEGA | MASKTOAPLAN \
	| MASKCAVE | MASKCPS2 | MASKMD | MASKPGM | MASKCPS3 \
	| MASKTAITO | MASKPSIKYO | MASKKANEKO16 | MASKKONAMI | MASKPACMAN \
	| MASKGALAXIAN | MASKATARI)

      
#define KEY(x) { pgi->nInput = GIT_SWITCH; pgi->Input.Switch.nCode = (unsigned short)(x); }
#define MACRO(x) { pgi->Macro.nMode = 1; pgi->Macro.Switch.nCode = (unsigned short)(x); }

/****************************************************/
/* PNG SECTION                                      */
/****************************************************/
 
extern bool DoReset;
extern bool CheckSetting(int i);
extern char msg[1024];

unsigned int nPrevGame = ~0U;
static unsigned char nPrevDIPSettings[4];
static unsigned int nDIPGroup;
static unsigned int nDIPSel;
static unsigned int nInpSel;
static int nDIPOffset;

static float brightness = 1.0f;
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
std::vector<int> m_InputSettingsOffsets;

char DipSetting[64];
char InpSetting[64];
char InpListSetting[64];
char SpeedAdjustString[16];
char AnalogAdjustString[16];
bool dialog_is_running = false;

typedef struct CtrlMallocArg
{
	uint32_t mallocCallCounts;

} CtrlMallocArg;


typedef struct CtrlFreeArg
{
	uint32_t freeCallCounts;

} CtrlFreeArg;


void *png_malloc(uint32_t size, void * a)
{
    CtrlMallocArg *arg;
    
	arg = (CtrlMallocArg *) a;
    
	arg->mallocCallCounts++;

	return malloc(size);
}


static int png_free(void *ptr, void * a)
{
    CtrlFreeArg *arg;
  
	arg = (CtrlFreeArg *) a;
    
	arg->freeCallCounts++;
	
	free(ptr);
	
	return 0;
}

int png_w=0, png_h=0;

int CurrentFilter = 0;

int nLastRom = 0;
int nLastFilter = 0;
int HideChildren = 0;
int ThreeOrFourPlayerOnly = 0;
int _fd = 0;

// Rom List	Movement 

float fGameSelect;
float fCursorPos;
float fMaxCount;
float m_fFrameTime;			// amount of time per frame

int	  iGameSelect;
int	  iCursorPos;
int	  iNumGames;
int	  m_iMaxWindowList;
int	  m_iWindowMiddle;

int   shaderindex;
int   currentConfigIndex = 0;
int	  inGameIndex = 0;
int	  inputListSel = 0;
int	  dipListSel = 0;
 
// Input Movement

float fInputSelect;
float fInputCursorPos;
float fInputMaxCount;
 
int	  iInputSelect;
int	  iInputCursorPos;
int	  iNumInput;
int	  m_iMaxWindowListInput;
int	  m_iWindowMiddleInput;

int inputList = 0;

// DIP Movement

float fDipSelect;
float fDipCursorPos;
float fDipMaxCount;
 
int	  iDipSelect;
int	  iDipCursorPos;
int	  iNumDips;
int	  m_iMaxWindowListDip;
int	  m_iWindowMiddleDip;

int dipList = 0;
 
 
const int GAMESEL_MaxWindowList	= 28;		 
const int GAMESEL_WindowMiddle = 14;	
const float	GAMESEL_cfDeadZone = 0.3f;
const float	GAMESEL_cfMaxThresh	= 0.93f;
const float	GAMESEL_cfMaxPossible =	1.0f;
const float	GAMESEL_cfRectifyScale = GAMESEL_cfMaxPossible/(GAMESEL_cfMaxPossible-GAMESEL_cfDeadZone);
const float	GAMESEL_cfSpeedBandFastest = 2.0f;	// seconds (don`t worry	for	PAL	NTSC dif xbapp handles that)
const float	GAMESEL_cfFastestScrollMult	= 6.0f;
const float	GAMESEL_cfSpeedBandMedium =	1.0f;	// if the pad is held at max for given seconds list	will move faster
const float	GAMESEL_cfMediumScrollMult = 5.0f;
const float	GAMESEL_cfSpeedBandLowest =	0.5f;
const float	GAMESEL_cfLowestScrollMult = 2.0f;	 

static uint32_t cols=0xFFFF7F7f;

std::vector<std::string> m_ListData;
std::vector<std::string> m_ListShaderData;
std::vector<std::string> m_vecAvailRomIndex;
std::vector<std::string> m_vecAvailRomReleasedBy;
std::vector<std::string> m_vecAvailRomManufacturer;
std::vector<std::string> m_vecAvailRomInfo;
std::vector<std::string> m_vecAvailRomParent;
std::vector<std::int32_t> m_vecAvailRomBurnDrvIndex;

std::map<int, int> m_HardwareFilterMap;
std::map<int, std::string> m_HardwareFilterDesc;
std::map<int, std::string> m_HardwareFilterBackGroundMap;

std::vector<std::string> m_vecAvailRomList;

extern int GameStatus;


// DIP Switch Handler Code
static bool bOK;

static void InpDIPSWGetOffset()
{
	BurnDIPInfo bdi;
	nDIPOffset = 0;
	for (int i = 0; BurnDrvGetDIPInfo(&bdi, i) == 0; i++) {
		if (bdi.nFlags == 0xF0) {
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
				pDIPGroup = bdi.szText;
				k = i;
			}
			i++;
		}
		else
		{
			if (CheckSetting(i))
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

	iNumDips =	m_DipListData.size();

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

	// get button info
	int nButtons = 0; // buttons per player
	int nPlayer = 0;
	int nPlayerTemp = 0;

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
	for (unsigned int i = 0; i < nMacroCount; i++, pgi++)
	{
		if (pgi->nInput & GIT_GROUP_MACRO)
		{
			m_InputList[std::string(pgi->Macro.szName)] = std::string(" ");
			m_InputListData.push_back(std::string(pgi->Macro.szName));
			m_InputListOffsets.push_back(j);
		}

		j++;
	}	 


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


		char* pszVal = InpToDesc(pgi);

		m_InputList[m_InputListData[z].c_str()] = std::string(pszVal);

		j++;
		z++;
	}

	for (i = 0, pgi = GameInp + nGameInpCount; i < nMacroCount; i++, pgi++)
	{
		if (pgi->nInput & GIT_GROUP_MACRO)
		{
			char* pszVal = InpMacroToDesc(pgi);
			m_InputList[m_InputListData[z].c_str()] = std::string(pszVal);
		}

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
	fDipSelect = 0.0f;
	iDipSelect = 0;
	fDipCursorPos = 0.0f;
	iDipCursorPos = 0;
	fDipMaxCount = 0.0f;

	m_DipList.clear();
	m_DipListData.clear();
	m_DipListValues.clear();
	m_DipListOffsets.clear();

}

int InitInputList()
{
	fInputSelect	= 0.0f;
	iInputSelect	= 0;
	fInputCursorPos = 0.0f;
	iInputCursorPos = 0;
	fInputMaxCount	= 0.0f;

	m_InputSettingsList.clear();
	m_InputSettingsData.clear();
	m_InputSettingsOffsets.clear();

/*
	m_InputSettingsList[PS3_BUTTON_SQUARE] = std::string(_T("Square Button"));
	m_InputSettingsList[PS3_BUTTON_CROSS] = std::string(_T("Cross Button"));
	m_InputSettingsList[PS3_BUTTON_CIRCLE] = std::string(_T("Circle Button"));
	m_InputSettingsList[PS3_BUTTON_TRIANGLE] = std::string(_T("Triangle Button"));
	m_InputSettingsList[PS3_BUTTON_START] = std::string(_T("Start Button"));
	m_InputSettingsList[PS3_BUTTON_SELECT] = std::string(_T("Select Button"));
	m_InputSettingsList[PS3_BUTTON_L1] = std::string(_T("L1 Button"));
	m_InputSettingsList[PS3_BUTTON_R1] = std::string(_T("R1 Button"));
	m_InputSettingsList[PS3_BUTTON_L3] = std::string(_T("L3 Button"));
	m_InputSettingsList[PS3_BUTTON_R3] = std::string(_T("R3 Button"));
	m_InputSettingsList[PS3_BUTTON_L2] = std::string(_T("L2 Button"));
	m_InputSettingsList[PS3_BUTTON_R2] = std::string(_T("R2 Button"));
	m_InputSettingsList[PS3_BUTTON_R3 | PS3_BUTTON_L3] = std::string(_T("L3 + R3"));
*/

//FIXME: Check if this is correct - compare with the original above
	m_InputSettingsList[CTRL_SQUARE_MASK] = std::string(_T("Square Button"));
	m_InputSettingsList[CTRL_CROSS_MASK] = std::string(_T("Cross Button"));
	m_InputSettingsList[CTRL_CIRCLE_MASK] = std::string(_T("Circle Button"));
	m_InputSettingsList[CTRL_TRIANGLE_MASK] = std::string(_T("Triangle Button"));
	m_InputSettingsList[CTRL_START_MASK] = std::string(_T("Start Button"));
	m_InputSettingsList[CTRL_SELECT_MASK] = std::string(_T("Select Button"));
	m_InputSettingsList[CTRL_L1_MASK] = std::string(_T("L1 Button"));
	m_InputSettingsList[CTRL_R1_MASK] = std::string(_T("R1 Button"));
	m_InputSettingsList[CTRL_L3_MASK] = std::string(_T("L3 Button"));
	m_InputSettingsList[CTRL_R3_MASK] = std::string(_T("R3 Button"));
	m_InputSettingsList[CTRL_L2_MASK] = std::string(_T("L2 Button"));
	m_InputSettingsList[CTRL_R2_MASK] = std::string(_T("R2 Button"));
	m_InputSettingsList[CTRL_R3_MASK | CTRL_L3_MASK] = std::string(_T("L3 + R3"));

	m_InputSettingsData.push_back(std::string(_T("Square Button")));
	m_InputSettingsData.push_back(std::string(_T("Cross Button")));
	m_InputSettingsData.push_back(std::string(_T("Circle Button")));
	m_InputSettingsData.push_back(std::string(_T("Triangle Button")));
	m_InputSettingsData.push_back(std::string(_T("Start Button")));
	m_InputSettingsData.push_back(std::string(_T("Select Button")));
	m_InputSettingsData.push_back(std::string(_T("L1 Button")));
	m_InputSettingsData.push_back(std::string(_T("R1 Button")));
	m_InputSettingsData.push_back(std::string(_T("L3 Button")));
	m_InputSettingsData.push_back(std::string(_T("R3 Button")));
	m_InputSettingsData.push_back(std::string(_T("L2 Button")));
	m_InputSettingsData.push_back(std::string(_T("R2 Button")));
	m_InputSettingsData.push_back(std::string(_T("L3 + R3")));

	m_InputSettingsOffsets.push_back(0);
	m_InputSettingsOffsets.push_back(1);
	m_InputSettingsOffsets.push_back(2);
	m_InputSettingsOffsets.push_back(3);
	m_InputSettingsOffsets.push_back(4);
	m_InputSettingsOffsets.push_back(5);
	m_InputSettingsOffsets.push_back(6);
	m_InputSettingsOffsets.push_back(7);
	m_InputSettingsOffsets.push_back(8);
	m_InputSettingsOffsets.push_back(9);
	m_InputSettingsOffsets.push_back(10);
	m_InputSettingsOffsets.push_back(11);
	m_InputSettingsOffsets.push_back(12);
 
}

int InitRomList()
{
 	m_ListData.clear();
	m_vecAvailRomList.clear();
	m_vecAvailRomIndex.clear();
	m_vecAvailRomBurnDrvIndex.clear();

	fGameSelect	= 0.0f;
	iGameSelect	= 0;
	fCursorPos = 0.0f;
	iCursorPos = 0;
	fMaxCount =	0.0f;

	//set frame	time
	m_fFrameTime = 1.0f	/ 60.0f;

	// build the hardware filter map
 
	m_HardwareFilterMap[ALL] = MASKALL;
	m_HardwareFilterMap[CPS1] = MASKCPS;
	m_HardwareFilterMap[CPS2] = MASKCPS2;
	m_HardwareFilterMap[CPS3] = MASKCPS3;
	m_HardwareFilterMap[NEOGEO] = MASKNEOGEO;
	m_HardwareFilterMap[TAITO] = MASKTAITO;
	m_HardwareFilterMap[SEGA] = MASKSEGA;
	m_HardwareFilterMap[PGM] = MASKPGM;
	m_HardwareFilterMap[PSYKIO] = MASKPSIKYO;
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
	m_HardwareFilterDesc[PSYKIO] = "Psikyo";
	m_HardwareFilterDesc[KONAMI] = "Konami";
	m_HardwareFilterDesc[KANEKO] = "Kaneko 16";
	m_HardwareFilterDesc[CAVE] = "Cave";
	m_HardwareFilterDesc[TOAPLAN] = "Toaplan";
	m_HardwareFilterDesc[SEGAMD] = "Sega Megadrive";
	m_HardwareFilterDesc[MISC] = "Misc";

	m_HardwareFilterBackGroundMap[ALL] = "fbanext-bg-all.png";
	m_HardwareFilterBackGroundMap[CPS1] = "fbanext-bg-cps1.png";
	m_HardwareFilterBackGroundMap[CPS2] = "fbanext-bg-cps2.png";
	m_HardwareFilterBackGroundMap[CPS3] = "fbanext-bg-cps3.png";
	m_HardwareFilterBackGroundMap[NEOGEO] = "fbanext-bg-ng.png";
	m_HardwareFilterBackGroundMap[TAITO] = "fbanext-bg-tai.png";
	m_HardwareFilterBackGroundMap[SEGA] = "fbanext-bg-s16.png";
	m_HardwareFilterBackGroundMap[PGM] = "fbanext-bg-pgm.png";
	m_HardwareFilterBackGroundMap[PSYKIO] = "fbanext-bg-psi.png";
	m_HardwareFilterBackGroundMap[KONAMI] = "fbanext-bg-kon.png";
	m_HardwareFilterBackGroundMap[KANEKO] = "fbanext-bg-kan.png";
	m_HardwareFilterBackGroundMap[CAVE] = "fbanext-bg-cave.png";
	m_HardwareFilterBackGroundMap[TOAPLAN] = "fbanext-bg-toa.png";
	m_HardwareFilterBackGroundMap[SEGAMD] = "fbanext-bg-md.png";
	m_HardwareFilterBackGroundMap[MISC] = "fbanext-bg-misc.png";

	return 0;
} 

void ResetMenuVars()
{
	fGameSelect = 0.0f;
	iGameSelect = 0;
	fCursorPos = 0.0f;
	iCursorPos = 0;
	fMaxCount = 0.0f;

	//set frame	time
	m_fFrameTime = 1.0f/60.0f;

	m_vecAvailRomList.clear();
	m_vecAvailRomIndex.clear();
	m_vecAvailRomBurnDrvIndex.clear();
}

int AvRoms()
{
	iNumGames =	m_vecAvailRomList.size();

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
	std::vector<int>		 vecAvailRomIndex;

	m_vecAvailRomList.clear();
	m_vecAvailRomReleasedBy.clear();
	m_vecAvailRomInfo.clear();
	m_vecAvailRomParent.clear();
	m_vecAvailRomManufacturer.clear();

	ResetMenuVars();

	if (m_ListData.empty())
	{
		for (int d = 0; d < DIRS_MAX; d++)
		{
			if (!_tcsicmp(szAppRomPaths[d], _T("")))
				continue; // skip empty path

			iterate_directory(szAppRomPaths[d], m_ListData);
		}
		std::sort(m_ListData.begin(), m_ListData.end());
	}

	if (m_ListShaderData.empty())
	{
		iterate_directory(SHADER_DIRECTORY, m_ListShaderData);
		std::sort(m_ListShaderData.begin(), m_ListShaderData.end());
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


#if defined (FBA_DEBUG)
				if ((IsFiltered))  // skip roms marked as not working
#else
					if (BurnDrvIsWorking() && (IsFiltered))  // skip roms marked as not working
#endif
					{

						int nNumPlayers = BurnDrvGetMaxPlayers();

						if ((HideChildren == 1 && (BurnDrvGetTextA(DRV_PARENT) == NULL && !(BurnDrvGetFlags() & BDF_CLONE))) ||
								(HideChildren == 1 && (BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_CAPCOM_CPS3) ||
								(HideChildren == 0))
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
	cellDbgFontPrintf(0.05f, 0.08f + 0.025f * ((float)number), 0.75f, currentConfigIndex == SETTING_DISPLAY_FRAMERATE ? cols : 0xFFFFFFFF, "Show Framerate : %s", bShowFPS ? "Yes" : "No" );     
	cellDbgFontDraw();
	number++;

	switch(psglGetCurrentResolutionId())
	{
		case CELL_VIDEO_OUT_RESOLUTION_480:
			cellDbgFontPrintf(0.05f, 0.08f + 0.025f * ((float)number), 0.75f, currentConfigIndex == SETTING_RESOLUTION ? cols : 0xFFFFFFFF, "Resolution : 720x480 (480p)");
			break;
		case CELL_VIDEO_OUT_RESOLUTION_720:
			cellDbgFontPrintf(0.05f, 0.08f + 0.025f * ((float)number), 0.75f, currentConfigIndex == SETTING_RESOLUTION ? cols : 0xFFFFFFFF, "Resolution : 1280x720 (720p)");
			break;
		case CELL_VIDEO_OUT_RESOLUTION_1080:
			cellDbgFontPrintf(0.05f, 0.08f + 0.025f * ((float)number), 0.75f, currentConfigIndex == SETTING_RESOLUTION ? cols : 0xFFFFFFFF, "Resolution : 1920x1080 (1080p)");
			break;
		case CELL_VIDEO_OUT_RESOLUTION_576:
			cellDbgFontPrintf(0.05f, 0.08f + 0.025f * ((float)number), 0.75f, currentConfigIndex == SETTING_RESOLUTION ? cols : 0xFFFFFFFF, "Resolution : 720x576 (576p)");
			break;
		case CELL_VIDEO_OUT_RESOLUTION_1600x1080:
			cellDbgFontPrintf(0.05f, 0.08f + 0.025f * ((float)number), 0.75f, currentConfigIndex == SETTING_RESOLUTION ? cols : 0xFFFFFFFF, "Resolution : 1600x1080");
			break;
		case CELL_VIDEO_OUT_RESOLUTION_1440x1080:
			cellDbgFontPrintf(0.05f, 0.08f + 0.025f * ((float)number), 0.75f, currentConfigIndex == SETTING_RESOLUTION ? cols : 0xFFFFFFFF, "Resolution : 1440x1080");
			break;
		case CELL_VIDEO_OUT_RESOLUTION_1280x1080:
			cellDbgFontPrintf(0.05f, 0.08f + 0.025f * ((float)number), 0.75f, currentConfigIndex == SETTING_RESOLUTION ? cols : 0xFFFFFFFF, "Resolution : 1280x1080");
			break;
		case CELL_VIDEO_OUT_RESOLUTION_960x1080:
			cellDbgFontPrintf(0.05f, 0.08f + 0.025f * ((float)number), 0.75f, currentConfigIndex == SETTING_RESOLUTION ? cols : 0xFFFFFFFF, "Resolution : 960x1080");
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
	cellDbgFontPrintf(0.05f, 0.08f + 0.025f * ((float)number), 0.75f, currentConfigIndex == SETTING_KEEP_ASPECT ? cols : 0xFFFFFFFF, "Aspect Ratio : %s", msg);
	cellDbgFontDraw();
	number++;

	char rotatemsg[3][256] = {{"Rotate for Vertical Games"},{"Do not rotate for Vertical Games"},{"Reverse flipping for vertical games"}};
	cellDbgFontPrintf(0.05f, 0.08f + 0.025f * ((float)number), 0.75f, currentConfigIndex == SETTING_ROTATE ? cols : 0xFFFFFFFF, "Rotation Adjust: %s", rotatemsg[nVidRotationAdjust]);     
	cellDbgFontDraw();
	number++;

	cellDbgFontPrintf(0.05f, 0.08f + 0.025f * ((float)number), 0.75f, currentConfigIndex == SETTING_AUTO_FRAMESKIP ? cols : 0xFFFFFFFF, "Auto Frameskip Enabled: %s", autoFrameSkip ? "Yes" : "No");
	cellDbgFontDraw();
	number++;

	cellDbgFontPrintf(0.05f, 0.08f + 0.025f * ((float)number), 0.75f, currentConfigIndex == SETTING_CURRENT_SHADER ? cols : 0xFFFFFFFF, "Current Shader : %s", m_ListShaderData[shaderindex].c_str());
	cellDbgFontDraw();	
	number++;

	cellDbgFontPrintf(0.05f, 0.08f + 0.025f * ((float)number), 0.75f, currentConfigIndex == SETTING_BILINEAR_FILTER ? cols : 0xFFFFFFFF, "Hardware Filter : %s", vidFilterLinear ? "Linear" : "Point");
	cellDbgFontDraw();
	number++;

	cellDbgFontPrintf(0.05f, 0.08f + 0.025f * ((float)number), 0.75f, currentConfigIndex == SETTING_VSYNC ? cols : 0xFFFFFFFF, "Vertical Sync : %s", bVidVSync ? "Yes" : "No");
	cellDbgFontDraw();
	number++;

	cellDbgFontPrintf(0.05f, 0.08f + 0.025f * ((float)number), 0.75f, currentConfigIndex == SETTING_HIDE_CLONES ? cols : 0xFFFFFFFF, "Hide Clone Roms : %s", HideChildren ? "Yes" : "No");
	cellDbgFontDraw();
	number++;

	cellDbgFontPrintf(0.05f, 0.08f + 0.025f * ((float)number), 0.75f, currentConfigIndex == SETTING_SHOW_THREE_FOUR_PLAYER_ONLY ? cols : 0xFFFFFFFF, "Show 3 or 4 Player Roms Only : %s", ThreeOrFourPlayerOnly ? "Yes" : "No");
	cellDbgFontDraw();
	number++;

	cellDbgFontPrintf(0.05f, 0.08f + 0.025f * ((float)number), 0.75f, currentConfigIndex == SETTING_TRIPLE_BUFFER ? cols : 0xFFFFFFFF, "Triple Buffering Enabled : %s", bVidTripleBuffer ? "Yes" : "No");
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
		if (bDrvOkay)	// theres a game loaded, return to game
		{
			old_state = new_state;
			setPauseMode(0);
			audio.play();
			GameStatus = EMULATING;
			return;
		}
		else
		{
			old_state = new_state;
			GameStatus = MENU;	// back to romlist
		}
	}
	else if (CTRL_TRIANGLE(old_state & diff_state))
	{
		// switch to config

		UpdateConsoleXY("Generating clrmame.dat. Please wait...", 0.35f, 0.5f );

		if (create_datfile("/dev_hdd0/game/FBAN00000/USRDIR/clrmame.dat",1) == 0)
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
		sys_timer_usleep(FILEBROWSER_DELAY/2);
		currentConfigIndex++;

		if (currentConfigIndex >= MAX_NO_OF_SETTINGS)
			currentConfigIndex = MAX_NO_OF_SETTINGS-1;
	}
	else if(CTRL_UP(new_state & diff_state) | CTRL_L2(new_state) | CTRL_LSTICK_UP(new_state))
	{		
		sys_timer_usleep(FILEBROWSER_DELAY/2);
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
				sys_timer_usleep(FILEBROWSER_DELAY/2);
			}
			if(CTRL_RIGHT(new_state & diff_state) | CTRL_LSTICK_RIGHT(new_state)) 
			{
				psglResolutionNext();	
				sys_timer_usleep(FILEBROWSER_DELAY/2);
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
					setWindowAspect();
				}
			}
			else if(CTRL_RIGHT(new_state & diff_state) | CTRL_CROSS(old_state & diff_state))
			{
				if(nVidScrnAspectMode < LAST_ASPECT_RATIO)
				{
					nVidScrnAspectMode++;
					setWindowAspect();
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
		case SETTING_AUTO_FRAMESKIP:
			if(CTRL_LEFT(new_state & diff_state) | CTRL_RIGHT(new_state & diff_state) | CTRL_CROSS(old_state & diff_state))
			{
				autoFrameSkip = !autoFrameSkip;
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
				vidUseFilter = 0;				

				char shaderFile[255];

				strcpy(shaderFile,SHADER_DIRECTORY);
				strcat(shaderFile,m_ListShaderData[shaderindex].c_str());
				psglInitShader(shaderFile);
			}
			break;
		case SETTING_BILINEAR_FILTER:
			if(CTRL_LEFT(new_state & diff_state) | CTRL_RIGHT(new_state & diff_state) | CTRL_CROSS(old_state & diff_state))
				vidFilterLinear = !vidFilterLinear;
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

	iGameSelect	= fGameSelect;
	iCursorPos = fCursorPos;
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
		cellDbgFontPuts(0.05f, 0.08f, 0.75f, cols, "No Roms Found");             
		cellDbgFontDraw();
	}

	iGameidx = 0;
	do
	{
		if (iGameidx==iCursorPos)
		{	
			cellDbgFontPuts(0.05f, 0.08f + 0.025f * ((float)iGameidx ), 0.75f, cols, m_vecAvailRomList[iTempGameSel++].c_str());             
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
			cellDbgFontPrintf(0.75f, 0.90f + 0.025f, 0.75f, cols ,"%ld free memory",mem_info.available_user_memory );     
			cellDbgFontDraw();
			cellDbgFontPrintf(0.75f, 0.92f + 0.025f, 0.75f, cols ,"%ld total memory",mem_info.total_user_memory );     
			cellDbgFontDraw();
			cellDbgFontPrintf(0.75f, 0.95f + 0.025f, 0.75f, cols ,"BurnDrvSelect = %ld ", m_vecAvailRomBurnDrvIndex[iGameSelect+iCursorPos]);     
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
				setPauseMode(0);
				audio.play();
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

			if (nSel >= 0) {

				if (m_DipListOffsets.size() > 0)
				{
					nDIPGroup = m_DipListOffsets[iDipSelect	+ iDipCursorPos];

					BurnDIPInfo bdiGroup;
					BurnDrvGetDIPInfo(&bdiGroup, nDIPGroup);

					int nCurrentSetting = 0;
					for (int i = 0, j = 0; i < bdiGroup.nSetting; i++) {
						char szText[MAX_PATH];
						BurnDIPInfo bdi;

						do {
							BurnDrvGetDIPInfo(&bdi, nDIPGroup + 1 + j++);
						} while (bdi.nFlags == 0);
						if (bdiGroup.szText) {
							_stprintf(szText, _T("%hs: %hs"), bdiGroup.szText, bdi.szText);
						} else {
							_stprintf(szText, _T("%hs"), bdi.szText);
						}

						m_DipListValues.push_back(std::string(szText));

						if (CheckSetting(nDIPGroup + j)) {
							nCurrentSetting = i;
						}
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
			for (int i = 0; i <= id; i++) {
				do {
					BurnDrvGetDIPInfo(&bdi, nDIPGroup + 1 + j++);
				} while (bdi.nFlags == 0);
			}


			struct GameInp* pgi = GameInp + bdi.nInput + nDIPOffset;
			pgi->Input.Constant.nConst = (pgi->Input.Constant.nConst & ~bdi.nMask) | (bdi.nSetting & bdi.nMask);
			if (bdi.nFlags & 0x40) {
				while (BurnDrvGetDIPInfo(&bdi, nDIPGroup + 1 + j++) == 0) {
					if (bdi.nFlags == 0) {
						pgi = GameInp + bdi.nInput + nDIPOffset;
						pgi->Input.Constant.nConst = (pgi->Input.Constant.nConst & ~bdi.nMask) | (bdi.nSetting & bdi.nMask);
					} else {
						break;
					}
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

			fDipCursorPos ++;

			if(	fDipCursorPos > m_iWindowMiddleDip )
			{
				// clamp cursor	position
				bClampCursor = TRUE;

				// advance gameselect
				if(fDipSelect == 0) fDipSelect +=	(fDipCursorPos - m_iWindowMiddleDip);
				else fDipSelect ++;

				// clamp game window range (high)
				if((fDipSelect	+ m_iMaxWindowListDip)	> iNumDips)
				{

					// clamp to	end
					fDipSelect	= iNumDips	- m_iMaxWindowListDip;

					// advance cursor pos after	all!
					bClampCursor = FALSE;

					// clamp cursor	to end
					if((fDipSelect	+ fDipCursorPos) >= iNumDips)
						fDipCursorPos = m_iMaxWindowListDip-1;
				}
			}

			// check for cursor	clamp
			if(	bClampCursor )
				fDipCursorPos = m_iWindowMiddleDip;	
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

			fDipCursorPos --;
			if(	fDipCursorPos < m_iWindowMiddleDip )
			{
				// clamp cursor	position
				bClampCursor = TRUE;

				// backup window pos
				fDipSelect	--;

				// clamp game window range (low)
				if(fDipSelect < 0)
				{
					// clamp to	start
					fDipSelect	= 0;

					// backup cursor pos after all!
					bClampCursor = FALSE;

					// clamp cursor	to end
					if(	fDipCursorPos < 0 )
						fDipCursorPos = 0;
				}
			}

			// check for cursor	clamp
			if(	bClampCursor )
				fDipCursorPos = m_iWindowMiddleDip;
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

	iDipSelect		= fDipSelect;
	iDipCursorPos	= fDipCursorPos;
	iTempDipSel		= iDipSelect;

	cellDbgFontPuts(0.05f, 0.04f , 0.75f, 0xFFE0EEFF, "FBANext PS3 - DIP Switch Menu");             
	cellDbgFontDraw();

	for	(iDipidx=0; iDipidx<m_iMaxWindowListDip;	iDipidx++)
	{ 
		int val = iTempDipSel++;
		sprintf(DipSetting,"%s : %s", m_DipListData[val].c_str(), m_DipList[std::string((char *)m_DipListData[val].c_str())].c_str());

		if (iDipidx==iDipCursorPos)
			cellDbgFontPuts(0.05f, 0.08f + 0.025f * ((float)iDipidx ), 0.75f, cols, DipSetting);
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
		for	(i=0; i<m_DipListValues.size();	i++)
		{ 		
			if (i==dipListSel)
				cellDbgFontPuts(0.5f, 0.08f + 0.025f * ((float)i ), 0.75f, cols, m_DipListValues[i].c_str());
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

	iInputSelect		= fInputSelect;
	iInputCursorPos		= fInputCursorPos;
	iTempInputSel		= iInputSelect;

	cellDbgFontPuts(0.05f, 0.04f , 0.75f, 0xFFE0EEFF, "FBANext PS3 - Input Mapping Menu");             
	cellDbgFontDraw();

	for	(iInputidx=0; iInputidx<m_iMaxWindowListInput;	iInputidx++)
	{ 
		int val = iTempInputSel++;
		sprintf(InpSetting,"%s : %s", m_InputListData[val].c_str(), m_InputList[std::string((char *)m_InputListData[val].c_str())].c_str());

		if (iInputidx==iInputCursorPos)
		{	 		 				
			cellDbgFontPuts(0.05f, 0.08f + 0.025f * ((float)iInputidx ), 0.75f, cols, InpSetting);
		}
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
		for	(i=0; i<m_InputSettingsData.size();	i++)
		{ 		
			if (i==inputListSel)
			{	 		 				
				cellDbgFontPuts(0.5f, 0.08f + 0.025f * ((float)i ), 0.75f, cols, m_InputSettingsData[i].c_str());
			}
			else		 
			{
				cellDbgFontPuts(0.5f, 0.08f + 0.025f * ((float)i ), 0.75f, 0xFFFFFFFF, m_InputSettingsData[i].c_str());
			}
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
				setPauseMode(0);
				audio.play();
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
						pgi->Input.Switch.nCode = (unsigned short)FBK_C;
						break;
					case 1:
						pgi->nInput = GIT_SWITCH;
						pgi->Input.Switch.nCode = (unsigned short)FBK_Z;
						break;
					case 2:
						pgi->nInput = GIT_SWITCH;
						pgi->Input.Switch.nCode = (unsigned short)FBK_X;
						break;
					case 3:
						pgi->nInput = GIT_SWITCH;
						pgi->Input.Switch.nCode = (unsigned short)FBK_V;
						break;
					case 4:	
						pgi->nInput = GIT_SWITCH;
						pgi->Input.Switch.nCode = (unsigned short)FBK_1;
						break;
					case 5:
						pgi->nInput = GIT_SWITCH;
						pgi->Input.Switch.nCode = (unsigned short)FBK_5;
						break;
					case 6:
						pgi->nInput = GIT_SWITCH;
						pgi->Input.Switch.nCode = (unsigned short)FBK_S;
						break;
					case 7:
						pgi->nInput = GIT_SWITCH;
						pgi->Input.Switch.nCode = (unsigned short)FBK_D;
						break;
					case 8:
						pgi->nInput = GIT_SWITCH;
						pgi->Input.Switch.nCode = (unsigned short)FBK_F1;
						break;
					case 9:
						pgi->nInput = GIT_SWITCH;
						pgi->Input.Switch.nCode = (unsigned short)FBK_F2;
						break;
					case 10: 
						pgi->nInput = GIT_SWITCH;
						pgi->Input.Switch.nCode = (unsigned short)PS3_L2_BUTTON; 
						break;
					case 11:
						pgi->nInput = GIT_SWITCH;
						pgi->Input.Switch.nCode = (unsigned short)PS3_R2_BUTTON;			 
						break;
				}
			}

			else if (strstr(m_InputListData[iInputSelect+iInputCursorPos].c_str(), "P1"))
			{
				switch (id)
				{
					case 0:
						if (pgi->nInput & GIT_GROUP_MACRO)
						{
							MACRO(FBK_C);
						}
						else
						{
							KEY(FBK_C);
						}			 
						break;
					case 1:
						if (pgi->nInput & GIT_GROUP_MACRO)
						{
							MACRO(FBK_Z);
						}
						else
						{
							KEY(FBK_Z);
						}	
						break;
					case 2:
						if (pgi->nInput & GIT_GROUP_MACRO)
						{
							MACRO(FBK_X);
						}
						else
						{
							KEY(FBK_X);
						}
						break;
					case 3:
						if (pgi->nInput & GIT_GROUP_MACRO)
						{
							MACRO(FBK_V);
						}
						else
						{
							KEY(FBK_V);
						}
						break;
					case 4:	
						if (pgi->nInput & GIT_GROUP_MACRO)
						{
							MACRO(FBK_1);
						}
						else
						{
							KEY(FBK_1);
						}
						break;
					case 5:
						if (pgi->nInput & GIT_GROUP_MACRO)
						{
							MACRO(FBK_5);
						}
						else
						{
							KEY(FBK_5);
						}
						break;
					case 6:
						if (pgi->nInput & GIT_GROUP_MACRO)
						{
							MACRO(FBK_S);
						}
						else
						{
							KEY(FBK_S);
						}
						break;
					case 7:
						if (pgi->nInput & GIT_GROUP_MACRO)
						{
							MACRO(FBK_D);
						}
						else
						{
							KEY(FBK_D);
						}
						break;
					case 8:
						if (pgi->nInput & GIT_GROUP_MACRO)
						{
							MACRO(FBK_F1);
						}
						else
						{
							KEY(FBK_F1);
						}
						break;
					case 9:
						if (pgi->nInput & GIT_GROUP_MACRO)
						{
							MACRO(FBK_F2);
						}
						else
						{
							KEY(FBK_F2);
						}
						break;
					case 10: 
						if (pgi->nInput & GIT_GROUP_MACRO)
						{
							MACRO(PS3_L2_BUTTON);
						}
						else
						{
							KEY(PS3_L2_BUTTON);
						}
						break;
					case 11:
						if (pgi->nInput & GIT_GROUP_MACRO)
						{
							MACRO(PS3_R2_BUTTON);
						}
						else
						{
							KEY(PS3_R2_BUTTON);
						}
						break;
					case 12:
						if (pgi->nInput & GIT_GROUP_MACRO)
						{
							MACRO(PS3_L3_BUTTON);
						}
						else
						{
							KEY(PS3_L3_BUTTON);
						}
						break;
					case 13:
						if (pgi->nInput & GIT_GROUP_MACRO)
						{
							MACRO(PS3_R3_BUTTON);
						}
						else
						{
							KEY(PS3_R3_BUTTON);
						}
						break;
				}
			}

			else if (strstr(m_InputListData[iInputSelect+iInputCursorPos].c_str(), "P2"))
			{
				switch (id)
				{
					case 0:
						if (pgi->nInput & GIT_GROUP_MACRO)
						{
							MACRO(0x4082);
						}
						else
						{
							KEY(0x4082);
						}
						break;
					case 1:
						if (pgi->nInput & GIT_GROUP_MACRO)
						{
							MACRO(0x4080);
						}
						else
						{
							KEY(0x4080);
						}
						break;
					case 2:
						if (pgi->nInput & GIT_GROUP_MACRO)
						{
							MACRO(0x4081);
						}
						else
						{
							KEY(0x4081);
						}
						break;
					case 3:
						if (pgi->nInput & GIT_GROUP_MACRO)
						{
							MACRO(0x4083);
						}
						else
						{
							KEY(0x4083);
						}
						break;
					case 4:	
						if (pgi->nInput & GIT_GROUP_MACRO)
						{
							MACRO(0x03);
						}
						else
						{
							KEY(0x03);
						}
						break;
					case 5:
						if (pgi->nInput & GIT_GROUP_MACRO)
						{
							MACRO(0x07);
						}
						else
						{
							KEY(0x07);
						}
						break;
					case 6:
						if (pgi->nInput & GIT_GROUP_MACRO)
						{
							MACRO(0x4084);
						}
						else
						{
							KEY(0x4084);
						}
						break;
					case 7:
						if (pgi->nInput & GIT_GROUP_MACRO)
						{
							MACRO(0x4085);
						}
						else
						{
							KEY(0x4085);
						}
						break;
					case 8:
						if (pgi->nInput & GIT_GROUP_MACRO)
						{
							MACRO(PS3_L3_BUTTON | 0x4000);
						}
						else
						{
							KEY(PS3_L3_BUTTON | 0x4000);
						}
						break;
					case 9:
						if (pgi->nInput & GIT_GROUP_MACRO)
						{
							MACRO(PS3_R3_BUTTON | 0x4000);
						}
						else
						{
							KEY(PS3_R3_BUTTON | 0x4000);
						}
						break;
					case 10: 
						if (pgi->nInput & GIT_GROUP_MACRO)
						{
							MACRO(PS3_L2_BUTTON | 0x4000);
						}
						else
						{
							KEY(PS3_L2_BUTTON | 0x4000);
						}
						break;
					case 11:
						if (pgi->nInput & GIT_GROUP_MACRO)
						{
							MACRO(PS3_R2_BUTTON | 0x4000);
						}
						else
						{
							KEY(PS3_R2_BUTTON | 0x4000);
						}
						break;
				}
			}

			else if (strstr(m_InputListData[iInputSelect+iInputCursorPos].c_str(), "P3"))
			{
				switch (id)
				{
					case 0:
						if (pgi->nInput & GIT_GROUP_MACRO)
						{
							MACRO(0x4182);
						}
						else
						{
							KEY(0x4182);
						} 
						break;
					case 1:				 
						if (pgi->nInput & GIT_GROUP_MACRO)
						{
							MACRO(0x4180);
						}
						else
						{
							KEY(0x4180);
						} 
						break;
					case 2:
						if (pgi->nInput & GIT_GROUP_MACRO)
						{
							MACRO(0x4181);
						}
						else
						{
							KEY(0x4181);
						} 
						break;
					case 3:
						if (pgi->nInput & GIT_GROUP_MACRO)
						{
							MACRO(0x4183);
						}
						else
						{
							KEY(0x4183);
						} 
						break; 
					case 4:	
						if (pgi->nInput & GIT_GROUP_MACRO)
						{
							MACRO(0x04);
						}
						else
						{
							KEY(0x04);
						} 
						break;
					case 5:
						if (pgi->nInput & GIT_GROUP_MACRO)
						{
							MACRO(0x08);
						}
						else
						{
							KEY(0x08);
						} 
						break;
					case 6:
						if (pgi->nInput & GIT_GROUP_MACRO)
						{
							MACRO(0x4184);
						}
						else
						{
							KEY(0x4184);
						} 
						break;
					case 7:
						if (pgi->nInput & GIT_GROUP_MACRO)
						{
							MACRO(0x4185);
						}
						else
						{
							KEY(0x4185);
						} 
						break;
					case 8:
						if (pgi->nInput & GIT_GROUP_MACRO)
						{
							MACRO(PS3_L3_BUTTON | 0x4100);
						}
						else
						{
							KEY(PS3_L3_BUTTON | 0x4100);
						} 
						break;
					case 9:
						if (pgi->nInput & GIT_GROUP_MACRO)
						{
							MACRO(PS3_R3_BUTTON | 0x4100);
						}
						else
						{
							KEY(PS3_R3_BUTTON | 0x4100);
						} 
						break;
					case 10: 
						if (pgi->nInput & GIT_GROUP_MACRO)
						{
							MACRO(PS3_L2_BUTTON | 0x4100);
						}
						else
						{
							KEY(PS3_L2_BUTTON | 0x4100);
						} 
						break;
					case 11:
						if (pgi->nInput & GIT_GROUP_MACRO)
						{
							MACRO(PS3_R2_BUTTON | 0x4100);
						}
						else
						{
							KEY(PS3_R2_BUTTON | 0x4100);
						} 
						break;
				}
			}

			else if (strstr(m_InputListData[iInputSelect+iInputCursorPos].c_str(), "P4"))
			{
				switch (id)
				{
					case 0:
						if (pgi->nInput & GIT_GROUP_MACRO)
						{
							MACRO(0x4282);
						}
						else
						{
							KEY(0x4282);
						} 
						break;
					case 1:
						if (pgi->nInput & GIT_GROUP_MACRO)
						{
							MACRO(0x4280);
						}
						else
						{
							KEY(0x4280);
						} 
						break;
					case 2:
						if (pgi->nInput & GIT_GROUP_MACRO)
						{
							MACRO(0x4281);
						}
						else
						{
							KEY(0x4281);
						}  
						break;
					case 3:
						if (pgi->nInput & GIT_GROUP_MACRO)
						{
							MACRO(0x4283);
						}
						else
						{
							KEY(0x4283);
						}  
						break;
					case 4:	
						if (pgi->nInput & GIT_GROUP_MACRO)
						{
							MACRO(0x05);
						}
						else
						{
							KEY(0x05);
						} 
						break;
					case 5:
						if (pgi->nInput & GIT_GROUP_MACRO)
						{
							MACRO(0x09);
						}
						else
						{
							KEY(0x09);
						} 
						break;
					case 6:
						if (pgi->nInput & GIT_GROUP_MACRO)
						{
							MACRO(0x4284);
						}
						else
						{
							KEY(0x4284);
						} 
						break;
					case 7:
						if (pgi->nInput & GIT_GROUP_MACRO)
						{
							MACRO(0x4285);
						}
						else
						{
							KEY(0x4285);
						} 
						break;
					case 8:
						if (pgi->nInput & GIT_GROUP_MACRO)
						{
							MACRO(PS3_L3_BUTTON | 0x4200);
						}
						else
						{
							KEY(PS3_L3_BUTTON | 0x4200);
						} 
						break;
					case 9:
						if (pgi->nInput & GIT_GROUP_MACRO)
						{
							MACRO(PS3_R3_BUTTON | 0x4200);
						}
						else
						{
							KEY(PS3_R3_BUTTON | 0x4200);
						} 
						break;
					case 10: 
						if (pgi->nInput & GIT_GROUP_MACRO)
						{
							MACRO(PS3_L2_BUTTON | 0x4200);
						}
						else
						{
							KEY(PS3_L2_BUTTON | 0x4200);
						} 
						break;
					case 11:
						if (pgi->nInput & GIT_GROUP_MACRO)
						{
							MACRO(PS3_R2_BUTTON | 0x4200);
						}
						else
						{
							KEY(PS3_R2_BUTTON | 0x4200);
						} 
						break;
				}
			}
			else
			{
				switch (id)
				{
					case 0:
						pgi->nInput = GIT_SWITCH;
						pgi->Input.Switch.nCode = (unsigned short)FBK_C;
						break;
					case 1:
						pgi->nInput = GIT_SWITCH;
						pgi->Input.Switch.nCode = (unsigned short)FBK_Z;
						break;
					case 2:
						pgi->nInput = GIT_SWITCH;
						pgi->Input.Switch.nCode = (unsigned short)FBK_X;
						break;
					case 3:
						pgi->nInput = GIT_SWITCH;
						pgi->Input.Switch.nCode = (unsigned short)FBK_V;
						break;
					case 4:	
						pgi->nInput = GIT_SWITCH;
						pgi->Input.Switch.nCode = (unsigned short)FBK_1;
						break;
					case 5:
						pgi->nInput = GIT_SWITCH;
						pgi->Input.Switch.nCode = (unsigned short)FBK_5;
						break;
					case 6:
						pgi->nInput = GIT_SWITCH;
						pgi->Input.Switch.nCode = (unsigned short)FBK_S;
						break;
					case 7:
						pgi->nInput = GIT_SWITCH;
						pgi->Input.Switch.nCode = (unsigned short)FBK_D;
						break;
					case 8:
						pgi->nInput = GIT_SWITCH;
						pgi->Input.Switch.nCode = (unsigned short)PS3_L3_BUTTON;
						break;
					case 9:
						pgi->nInput = GIT_SWITCH;
						pgi->Input.Switch.nCode = (unsigned short)PS3_R3_BUTTON;
						break;
					case 10: 
						pgi->nInput = GIT_SWITCH;
						pgi->Input.Switch.nCode = (unsigned short)PS3_L2_BUTTON; 
						break;
					case 11:
						pgi->nInput = GIT_SWITCH;
						pgi->Input.Switch.nCode = (unsigned short)PS3_R2_BUTTON;			 
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
		sys_timer_usleep(FILEBROWSER_DELAY/2);
		if (!inputList)
		{
			// default don`t clamp cursor
			bool bClampCursor =	FALSE;

			fInputCursorPos ++;

			if(fInputCursorPos > m_iWindowMiddleInput)
			{
				// clamp cursor	position
				bClampCursor = TRUE;

				// advance gameselect
				if(fInputSelect == 0)
					fInputSelect += (fInputCursorPos - m_iWindowMiddleInput);
				else
					fInputSelect ++;

				// clamp game window range (high)
				if((fInputSelect	+ m_iMaxWindowListInput)	> iNumInput)
				{

					// clamp to	end
					fInputSelect	= iNumInput	- m_iMaxWindowListInput;

					// advance cursor pos after	all!
					bClampCursor = FALSE;

					// clamp cursor	to end
					if((fInputSelect	+ fInputCursorPos) >= iNumInput)
						fInputCursorPos = m_iMaxWindowListInput-1;
				}
			}

			// check for cursor	clamp
			if(	bClampCursor )
				fInputCursorPos = m_iWindowMiddleInput;	
		}
		else
		{
			inputListSel++;

			if (inputListSel > m_InputSettingsData.size()-1)
			{
				inputListSel = m_InputSettingsData.size()-1;
			}
		}
		old_state = new_state;

	}
	else if(CTRL_UP(new_state & diff_state) | CTRL_L2(new_state) | CTRL_LSTICK_UP(new_state)) 
	{
		sys_timer_usleep(FILEBROWSER_DELAY/2);
		if (!inputList)
		{
			// default don`t clamp cursor
			bool bClampCursor =	FALSE;

			fInputCursorPos --;
			if(	fInputCursorPos < m_iWindowMiddleInput )
			{
				// clamp cursor	position
				bClampCursor = TRUE;

				// backup window pos
				fInputSelect	--;

				// clamp game window range (low)
				if(fInputSelect < 0)
				{
					// clamp to	start
					fInputSelect	= 0;

					// backup cursor pos after all!
					bClampCursor = FALSE;

					// clamp cursor	to end
					if(	fInputCursorPos < 0 )
						fInputCursorPos = 0;
				}
			}

			// check for cursor	clamp
			if(	bClampCursor )
				fInputCursorPos = m_iWindowMiddleInput;
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
			GamcPlayer(pgi, bii.szInfo, 0, -1);						// Keyboard
			GamcAnalogKey(pgi, bii.szInfo, 0, 1);
			GamcMisc(pgi, bii.szInfo, 0);
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
	cellDbgFontPuts(0.05f, 0.08f + 0.025f * ((float)number), 0.75f, inGameIndex == INGAME_MAP_BUTTONS ? cols : 0xFFFFFFFF, "Map Gamepad Buttons" );     
	cellDbgFontDraw();
	number++;

	cellDbgFontPuts(0.05f, 0.08f + 0.025f * ((float)number), 0.75f, inGameIndex == INGAME_DIP_SWITCHES ? cols : 0xFFFFFFFF, "Map Dip Switches");
	cellDbgFontDraw(); 
	number++;

	cellDbgFontPuts(0.05f, 0.08f + 0.025f * ((float)number), 0.75f, inGameIndex == INGAME_RESIZE_SCREEN ? cols : 0xFFFFFFFF, "Resize Screen");
	cellDbgFontDraw();
	number++;

	cellDbgFontPuts(0.05f, 0.08f + 0.025f * ((float)number), 0.75f, inGameIndex == INGAME_FRAME_ADVANCE ? cols : 0xFFFFFFFF, "Frame Advance");
	cellDbgFontDraw();
	number++;

	cellDbgFontPrintf(0.05f, 0.08f + 0.025f * ((float)number), 0.75f, inGameIndex == INGAME_CURRENT_SHADER ? cols : 0xFFFFFFFF, "Current Shader : %s", m_ListShaderData[shaderindex].c_str());
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
	cellDbgFontPrintf(0.05f, 0.08f + 0.025f * ((float)number), 0.75f, inGameIndex == INGAME_KEEP_ASPECT ? cols : 0xFFFFFFFF, "Aspect Ratio : %s", msg);
	cellDbgFontDraw();
	number++;

	cellDbgFontPrintf(0.05f, 0.08f + 0.025f * ((float)number), 0.75f, inGameIndex == INGAME_BILINEAR_FILTER ? cols : 0xFFFFFFFF, "Hardware Filter : %s", vidFilterLinear ? "Linear" : "Point");
	cellDbgFontDraw();
	number++;

	char rotatemsg[3][256] = {{"Rotate for Vertical Games"},{"Do not rotate for Vertical Games"},{"Reverse flipping for vertical games"}};
	cellDbgFontPrintf(0.05f, 0.08f + 0.025f * ((float)number), 0.75f, inGameIndex == INGAME_ROTATE ? cols : 0xFFFFFFFF, "Rotation Adjust: %s", rotatemsg[nVidRotationAdjust]);     
	cellDbgFontDraw();
	number++;

	cellDbgFontPrintf(0.05f, 0.08f + 0.025f * ((float)number), 0.75f, inGameIndex == INGAME_SAVE_STATE ? cols : 0xFFFFFFFF, "Save State #%d", save_state_slot);
	cellDbgFontDraw();
	number++;

	cellDbgFontPrintf(0.05f, 0.08f + 0.025f * ((float)number), 0.75f, inGameIndex == INGAME_LOAD_STATE ? cols : 0xFFFFFFFF, "Load State #%d", save_state_slot);
	cellDbgFontDraw();
	number++;

	cellDbgFontPuts(0.05f, 0.08f + 0.025f * ((float)number), 0.75f, inGameIndex == INGAME_RESET_GAME ? cols : 0xFFFFFFFF, "Reset Game");
	cellDbgFontDraw();
	number++;

#ifdef MULTIMAN_SUPPORT
	cellDbgFontPuts(0.05f, 0.08f + 0.025f * ((float)number), 0.75f, inGameIndex == INGAME_RETURN_TO_MULTIMAN ? cols : 0xFFFFFFFF, "Return to multiMAN");
	cellDbgFontDraw();
	number++;
#endif

	cellDbgFontPuts(0.05f, 0.08f + 0.025f * ((float)number), 0.75f, inGameIndex == INGAME_EXIT_GAME ? cols : 0xFFFFFFFF, "Exit Game");
	cellDbgFontDraw();
	number++;

	cellDbgFontPuts(0.05f, 0.08f + 0.025f * ((float)number), 0.75f, inGameIndex == INGAME_BACK_TO_GAME ? cols : 0xFFFFFFFF, "Return to current Game");
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
			setPauseMode(0);
			audio.play();
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
			sys_timer_usleep(FILEBROWSER_DELAY/2);
		}
	}
	else if(CTRL_UP(new_state & diff_state) | CTRL_LSTICK_UP(new_state))
	{		 
		if(inGameIndex > 0)
		{
			inGameIndex--;
			sys_timer_usleep(FILEBROWSER_DELAY/2);
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
				setPauseMode(0);
				audio.play();
				is_running = 0;
				GameStatus = EMULATING;
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
				vidUseFilter = 0;				

				char shaderFile[255];

				strcpy(shaderFile,SHADER_DIRECTORY);
				strcat(shaderFile,m_ListShaderData[shaderindex].c_str());
				psglInitShader(shaderFile);
				BurnReinitScrn();
				VidFrame();
			}
			break;
		case INGAME_KEEP_ASPECT:
			if(CTRL_LEFT(new_state & diff_state))
			{
				if(nVidScrnAspectMode > 0)
				{
					nVidScrnAspectMode--;
					setWindowAspect();
					BurnReinitScrn();
					VidFrame();
				}
			}
			else if(CTRL_RIGHT(new_state & diff_state) | CTRL_CROSS(old_state & diff_state))
			{
				if(nVidScrnAspectMode < LAST_ASPECT_RATIO)
				{
					nVidScrnAspectMode++;
					setWindowAspect();
					BurnReinitScrn();
					VidFrame();
				}
			}
			break;
		case INGAME_BILINEAR_FILTER:
			if(CTRL_LEFT(new_state & diff_state) | CTRL_RIGHT(new_state & diff_state) | CTRL_CROSS(old_state & diff_state))
			{
				vidFilterLinear = !vidFilterLinear;
				BurnReinitScrn();
				VidFrame();
			}
			break;
		case INGAME_ROTATE:
			if(CTRL_LEFT(new_state & diff_state) | CTRL_RIGHT(new_state & diff_state) | CTRL_CROSS(old_state & diff_state))
			{
				nVidRotationAdjust++;

				if (nVidRotationAdjust > 2)
				{
					nVidRotationAdjust = 0;
				}
				BurnReinitScrn();	//apply_rotation_settings();
				VidFrame();
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
					setPauseMode(0);
					audio.play();
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
					setPauseMode(0);
					audio.play();
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
					if ( nPrevGame < nBurnDrvCount ) 
					{				
						old_state = new_state;
						nBurnDrvSelect = nPrevGame;
						is_running = 0;

						nPrevGame = ~0U;			 
						RunExit();				 		 	
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
				sprintf(multiMAN, "%s", "/dev_hdd0/game/BLES80608/USRDIR/RELOAD.SELF");
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
					setPauseMode(0);
					audio.play();
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

		fCursorPos ++;

		if(	fCursorPos > m_iWindowMiddle )
		{
			// clamp cursor	position
			bClampCursor = TRUE;

			// advance gameselect
			if(fGameSelect == 0) fGameSelect +=	(fCursorPos	- m_iWindowMiddle);
			else fGameSelect ++;

			// clamp game window range (high)
			if((fGameSelect	+ m_iMaxWindowList)	> iNumGames)
			{

				// clamp to	end
				fGameSelect	= iNumGames	- m_iMaxWindowList;

				// advance cursor pos after	all!
				bClampCursor = FALSE;

				// clamp cursor	to end
				if((fGameSelect	+ fCursorPos) >= iNumGames)
					fCursorPos = m_iMaxWindowList-1;
			}
		}

		// check for cursor	clamp
		if(	bClampCursor )
			fCursorPos = m_iWindowMiddle;	
	}

	if( CTRL_DOWN(new_state & diff_state) || CTRL_LSTICK_DOWN(new_state))
	{
		sys_timer_usleep(FILEBROWSER_DELAY/2);
		// default don`t clamp cursor
		bool bClampCursor =	FALSE;

		fCursorPos ++;

		if(fCursorPos > m_iWindowMiddle)
		{
			// clamp cursor	position
			bClampCursor = TRUE;

			// advance gameselect
			if(fGameSelect == 0)
				fGameSelect +=	(fCursorPos	- m_iWindowMiddle);
			else
				fGameSelect ++;

			// clamp game window range (high)
			if((fGameSelect	+ m_iMaxWindowList)	> iNumGames)
			{

				// clamp to	end
				fGameSelect	= iNumGames	- m_iMaxWindowList;

				// advance cursor pos after	all!
				bClampCursor = FALSE;

				// clamp cursor	to end
				if((fGameSelect	+ fCursorPos) >= iNumGames)
					fCursorPos = m_iMaxWindowList-1;
			}
		}

		// check for cursor	clamp
		if(	bClampCursor )
			fCursorPos = m_iWindowMiddle;	

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
			setPauseMode(0);
			audio.play();
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
				RunExit();				 		 	
				BurnerDrvExit();				// Make sure any game driver is exited
				mediaExit();					// Exit media
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
		sys_timer_usleep(FILEBROWSER_DELAY/2);
		bool bClampCursor =	FALSE;

		fCursorPos --;
		if(fCursorPos < m_iWindowMiddle)
		{
			// clamp cursor	position
			bClampCursor = TRUE;

			// backup window pos
			fGameSelect	--;

			// clamp game window range (low)
			if(fGameSelect < 0)
			{
				// clamp to	start
				fGameSelect	= 0;

				// backup cursor pos after all!
				bClampCursor = FALSE;

				// clamp cursor	to end
				if(	fCursorPos < 0 )
					fCursorPos = 0;
			}
		}

		// check for cursor	clamp
		if(	bClampCursor )
			fCursorPos = m_iWindowMiddle;
	}			 			 			 
	else if(CTRL_L2(new_state)) 
	{
		// default don`t clamp cursor
		bool bClampCursor =	FALSE;

		fCursorPos --;
		if(fCursorPos < m_iWindowMiddle)
		{
			// clamp cursor	position
			bClampCursor = TRUE;

			// backup window pos
			fGameSelect	--;

			// clamp game window range (low)
			if(fGameSelect < 0)
			{
				// clamp to	start
				fGameSelect	= 0;

				// backup cursor pos after all!
				bClampCursor = FALSE;

				// clamp cursor	to end
				if(	fCursorPos < 0 )
					fCursorPos = 0;
			}
		}

		// check for cursor	clamp
		if(	bClampCursor )
			fCursorPos = m_iWindowMiddle;
	}			 			 			 
	else if(CTRL_CROSS(new_state))
	{
		// initalise emulation here	and	set	emulating to true
		int	entryselected =	iGameSelect	+ iCursorPos;

		if (iNumGames >	0)
		{	
			nBurnDrvSelect = (unsigned int)m_vecAvailRomBurnDrvIndex[entryselected];

			if (nPrevGame == nBurnDrvSelect)
			{
				// same game, do nothing
				old_state = new_state;
				setPauseMode(0);
				audio.play();
				GameStatus = EMULATING;	
				return;
			}	

			if (bDrvOkay)
			{
				if ( nPrevGame < nBurnDrvCount ) 
				{				
					nBurnDrvSelect = nPrevGame;

					nPrevGame = ~0U;			 
					RunExit();				 		 	
					BurnerDrvExit();				// Make sure any game driver is exited
					mediaExit();					// Exit media
				}
			}

			nBurnFPS = 6000;
			nFMInterpolation = 0;

			if (directLoadGame(m_vecAvailRomIndex[entryselected].c_str()) == 0)
			{
				nPrevGame = m_vecAvailRomBurnDrvIndex[entryselected];

				mediaInit();
				RunInit();
				//nCurrentBurnDrvSelect = nBurnDrvSelect;
				nLastRom = entryselected;
				nLastFilter = CurrentFilter;
				configAppSaveXml();
				GameStatus = EMULATING;	
				return;

			}
			else
			{
				nBurnDrvSelect = nPrevGame;

				nPrevGame = ~0U;			 
				RunExit();				 		 	
				BurnerDrvExit();				// Make sure any game driver is exited
				mediaExit();					// Exit media
			}

			nPrevGame = nBurnDrvSelect;
		}
	}

	old_state = new_state;
}
