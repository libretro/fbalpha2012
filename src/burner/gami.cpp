// Burner Game Input
#include "burner.h"
// Key codes
#include "../interface/inp_keys.h"

#ifdef __LIBSNES__
#include "../interface/interface-ssnes.h"
#include "../burn/ssnes-typedefs.h"
#endif

#if defined(SN_TARGET_PS3) || defined(_XBOX)
#define GamcAnalogJoy(pgi, szi, nPlayer, nJoy, nSlide) {}
#endif

// Player Default Controls
int nPlayerDefaultControls[4] = {0, 1, 2, 3};
char szPlayerDefaultIni[4][MAX_PATH] = { "", "", "", ""};

// Mapping of PC inputs to game inputs
struct GameInp* GameInp = NULL;
unsigned int nGameInpCount = 0;
#ifndef NO_MACROS
unsigned int nMacroCount = 0;
unsigned int nMaxMacro = 0;
const int nMaxMacroPerPlayer = 10;
#endif

int nAnalogSpeed;

int nFireButtons = 0;

bool bStreetFighterLayout = false;
bool bLeftAltkeyMapped = false;

extern int ArcadeJoystick;

// ---------------------------------------------------------------------------

// Check if the left alt (menu) key is mapped
void GameInpCheckLeftAlt()
{
#if !defined (_XBOX) && !defined (SN_TARGET_PS3)
	struct GameInp* pgi;
	unsigned int i;

	bLeftAltkeyMapped = false;

	for (i = 0, pgi = GameInp; i < (nGameInpCount + nMacroCount); i++, pgi++)
	{

		if (bLeftAltkeyMapped)
			break;

		switch (pgi->nInput)
		{
			case GIT_SWITCH:
				if (pgi->Input.Switch == FBK_LALT)
					bLeftAltkeyMapped = true;
				break;
			case GIT_MACRO_AUTO:
			case GIT_MACRO_CUSTOM:
				if (pgi->Macro.nMode)
				{
					if (pgi->Macro.Switch == FBK_LALT)
						bLeftAltkeyMapped = true;
				}
				break;

			default:
				continue;
		}
	}
#endif
}

// Check if the sytem mouse is mapped and set the cooperative level apropriately
void GameInpCheckMouse()
{
#if !defined (_XBOX) && !defined (SN_TARGET_PS3) && !defined (__LIBSNES__)
	bool bMouseMapped = false;
	struct GameInp* pgi;
	unsigned int i;

	for (i = 0, pgi = GameInp; i < (nGameInpCount + nMacroCount); i++, pgi++)
	{

		if (bMouseMapped)
			break;

		switch (pgi->nInput)
		{
			case GIT_SWITCH:
				if ((pgi->Input.Switch & 0xFF00) == 0x8000)
					bMouseMapped = true;
				break;
			case GIT_MOUSEAXIS:
				if (pgi->Input.MouseAxis.nMouse == 0)
					bMouseMapped = true;
				break;
			case GIT_MACRO_AUTO:
			case GIT_MACRO_CUSTOM:
				if (pgi->Macro.nMode)
				{
					if ((pgi->Macro.Switch & 0xFF00) == 0x8000)
						bMouseMapped = true;
				}
				break;

			default:
				continue;
		}
	}

	if (bDrvOkay)
	{
		if (!bRunPause)
			InputSetCooperativeLevel(bMouseMapped, !bAlwaysProcessKey);
		else
			InputSetCooperativeLevel(false, !bAlwaysProcessKey);
	}
	else
		InputSetCooperativeLevel(false, false);
#endif
}

// ---------------------------------------------------------------------------

int GameInpBlank(int bDipSwitch)
{
	unsigned int i = 0;
	struct GameInp* pgi = NULL;

	// Reset all inputs to undefined (even dip switches, if bDipSwitch==1)
	if (GameInp == NULL)
		return 1;

	// Get the targets in the library for the Input Values
	struct BurnInputInfo bii;

	for (i = 0, pgi = GameInp; i < nGameInpCount; i++, pgi++)
	{
		memset(&bii, 0, sizeof(bii));
		BurnDrvGetInputInfo(&bii, i);
		if (bDipSwitch == 0 && (bii.nType & BIT_GROUP_CONSTANT)) // Don't blank the dip switches
			continue;

		memset(pgi, 0, sizeof(*pgi)); // Clear input

		pgi->nType = bii.nType;		// store input type
		pgi->Input.pVal = bii.pVal;	// store input pointer to value
		if (bii.nType & BIT_GROUP_CONSTANT)
		{	// Further initialisation for constants/DIPs
			pgi->nInput = GIT_CONSTANT;
			pgi->Input.Constant = *bii.pVal;
		}
	}

	#ifndef NO_MACROS
	for (i = 0; i < nMacroCount; i++, pgi++)
	{
		pgi->Macro.nMode = 0;
		if (pgi->nInput == GIT_MACRO_CUSTOM)
			pgi->nInput = 0;
	}
	#endif

	bLeftAltkeyMapped = false;

	return 0;
}

#ifndef NO_MACROS
static void GameInpInitMacros()
{
	struct GameInp* pgi;
	struct BurnInputInfo bii;

	int nPunchx3[4] = {0, 0, 0, 0};
	int nPunchInputs[4][3];
	int nKickx3[4] = {0, 0, 0, 0};
	int nKickInputs[4][3];

	int nNeogeoButtons[4][4];
	int nPgmButtons[4][4];

	bStreetFighterLayout = false;
	nMacroCount = 0;

	nFireButtons = 0;

	for (unsigned int i = 0; i < nGameInpCount; i++)
	{
		bii.szName = NULL;
		BurnDrvGetInputInfo(&bii, i);

		if (bii.szName == NULL)
			bii.szName = "";

		if (bii.szName[0] == 'P' && bii.szName[1] >= '1' && bii.szName[1] <= '4')
		{
			int nPlayer = bii.szName[1] - '1';

			if (nPlayer == 0)
			{
				if (strncmp(" fire", bii.szInfo + 2, 5) == 0)
					nFireButtons++;
			}

			if (strcasecmp(" Weak Punch", bii.szName + 2) == 0)
			{
				nPunchx3[nPlayer] |= 1;
				nPunchInputs[nPlayer][0] = i;
			}

			if (strcasecmp(" Medium Punch", bii.szName + 2) == 0)
			{
				nPunchx3[nPlayer] |= 2;
				nPunchInputs[nPlayer][1] = i;
			}

			if (strcasecmp(" Strong Punch", bii.szName + 2) == 0)
			{
				nPunchx3[nPlayer] |= 4;
				nPunchInputs[nPlayer][2] = i;
			}

			if (strcasecmp(" Weak Kick", bii.szName + 2) == 0)
			{
				nKickx3[nPlayer] |= 1;
				nKickInputs[nPlayer][0] = i;
			}

			if (strcasecmp(" Medium Kick", bii.szName + 2) == 0)
			{
				nKickx3[nPlayer] |= 2;
				nKickInputs[nPlayer][1] = i;
			}

			if (strcasecmp(" Strong Kick", bii.szName + 2) == 0)
			{
				nKickx3[nPlayer] |= 4;
				nKickInputs[nPlayer][2] = i;
			}

			if (strcasecmp(" Attack", bii.szName + 2) == 0) {
				nPunchx3[nPlayer] |= 1;
				nPunchInputs[nPlayer][0] = i;
			}
			if (strcasecmp(" Jump", bii.szName + 2) == 0) {
				nPunchx3[nPlayer] |= 2;
				nPunchInputs[nPlayer][1] = i;
			}

			if (strcasecmp(" Button A", bii.szName + 2) == 0)
				nNeogeoButtons[nPlayer][0] = i;
			if (strcasecmp(" Button B", bii.szName + 2) == 0)
				nNeogeoButtons[nPlayer][1] = i;
			if (strcasecmp(" Button C", bii.szName + 2) == 0)
				nNeogeoButtons[nPlayer][2] = i;
			if (strcasecmp(" Button D", bii.szName + 2) == 0)
				nNeogeoButtons[nPlayer][3] = i;

			if (strcasecmp(" Button 1", bii.szName + 2) == 0)
				nPgmButtons[nPlayer][0] = i;
			if (strcasecmp(" Button 2", bii.szName + 2) == 0)
				nPgmButtons[nPlayer][1] = i;
			if (strcasecmp(" Button 3", bii.szName + 2) == 0)
				nPgmButtons[nPlayer][2] = i;
			if (strcasecmp(" Button 4", bii.szName + 2) == 0)
				nPgmButtons[nPlayer][3] = i;
		}
	}

	pgi = GameInp + nGameInpCount;

	for (int nPlayer = 0; nPlayer < nMaxPlayers; nPlayer++)
	{
		if (nPunchx3[nPlayer] == 7)
		{		// Create a 3x punch maco
			pgi->nInput = GIT_MACRO_AUTO;
			pgi->nType = BIT_DIGITAL;
			pgi->Macro.nMode = 0;

			sprintf(pgi->Macro.szName, "P%i 3x Punch", nPlayer + 1);
			for (int j = 0; j < 3; j++) {
				BurnDrvGetInputInfo(&bii, nPunchInputs[nPlayer][j]);
				pgi->Macro.pVal[j] = bii.pVal;
				pgi->Macro.nVal[j] = 1;
			}

			nMacroCount++;
			pgi++;
		}

		if (nKickx3[nPlayer] == 7) {		// Create a 3x kick macro
			pgi->nInput = GIT_MACRO_AUTO;
			pgi->nType = BIT_DIGITAL;
			pgi->Macro.nMode = 0;

			sprintf(pgi->Macro.szName, "P%i 3x Kick", nPlayer + 1);
			for (int j = 0; j < 3; j++) {
				BurnDrvGetInputInfo(&bii, nKickInputs[nPlayer][j]);
				pgi->Macro.pVal[j] = bii.pVal;
				pgi->Macro.nVal[j] = 1;
			}

			nMacroCount++;
			pgi++;
		}

		if (nPunchx3[nPlayer] == 3) {		// Create a special macro
			pgi->nInput = GIT_MACRO_AUTO;
			pgi->nType = BIT_DIGITAL;
			pgi->Macro.nMode = 0;

			sprintf(pgi->Macro.szName, "P%i Special", nPlayer + 1);
			for (int j = 0; j < 2; j++) {
				BurnDrvGetInputInfo(&bii, nPunchInputs[nPlayer][j]);
				pgi->Macro.pVal[j] = bii.pVal;
				pgi->Macro.nVal[j] = 1;
			}

			nMacroCount++;
			pgi++;
		}

		if (nFireButtons == 4 && (BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_SNK_NEOGEO) {

			// A + B macro
			pgi->nInput = GIT_MACRO_AUTO;
			pgi->nType = BIT_DIGITAL;
			pgi->Macro.nMode = 0;

			sprintf(pgi->Macro.szName, "P%i Button AB", nPlayer + 1);
			BurnDrvGetInputInfo(&bii, nNeogeoButtons[nPlayer][0]);
			pgi->Macro.pVal[0] = bii.pVal;
			pgi->Macro.nVal[0] = 1;
			BurnDrvGetInputInfo(&bii, nNeogeoButtons[nPlayer][1]);
			pgi->Macro.pVal[1] = bii.pVal;
			pgi->Macro.nVal[1] = 1;

			nMacroCount++;
			pgi++;

			// A + C macro
			pgi->nInput = GIT_MACRO_AUTO;
			pgi->nType = BIT_DIGITAL;
			pgi->Macro.nMode = 0;

			sprintf(pgi->Macro.szName, "P%i Button AC", nPlayer + 1);
			BurnDrvGetInputInfo(&bii, nNeogeoButtons[nPlayer][0]);
			pgi->Macro.pVal[0] = bii.pVal;
			pgi->Macro.nVal[0] = 1;
			BurnDrvGetInputInfo(&bii, nNeogeoButtons[nPlayer][2]);
			pgi->Macro.pVal[1] = bii.pVal;
			pgi->Macro.nVal[1] = 1;

			nMacroCount++;
			pgi++;

			// A + D macro
			pgi->nInput = GIT_MACRO_AUTO;
			pgi->nType = BIT_DIGITAL;
			pgi->Macro.nMode = 0;

			sprintf(pgi->Macro.szName, "P%i Button AD", nPlayer + 1);
			BurnDrvGetInputInfo(&bii, nNeogeoButtons[nPlayer][0]);
			pgi->Macro.pVal[0] = bii.pVal;
			pgi->Macro.nVal[0] = 1;
			BurnDrvGetInputInfo(&bii, nNeogeoButtons[nPlayer][3]);
			pgi->Macro.pVal[1] = bii.pVal;
			pgi->Macro.nVal[1] = 1;

			nMacroCount++;
			pgi++;

			// B + C macro
			pgi->nInput = GIT_MACRO_AUTO;
			pgi->nType = BIT_DIGITAL;
			pgi->Macro.nMode = 0;

			sprintf(pgi->Macro.szName, "P%i Button BC", nPlayer + 1);
			BurnDrvGetInputInfo(&bii, nNeogeoButtons[nPlayer][1]);
			pgi->Macro.pVal[0] = bii.pVal;
			pgi->Macro.nVal[0] = 1;
			BurnDrvGetInputInfo(&bii, nNeogeoButtons[nPlayer][2]);
			pgi->Macro.pVal[1] = bii.pVal;
			pgi->Macro.nVal[1] = 1;

			nMacroCount++;
			pgi++;

			// B + D macro
			pgi->nInput = GIT_MACRO_AUTO;
			pgi->nType = BIT_DIGITAL;
			pgi->Macro.nMode = 0;

			sprintf(pgi->Macro.szName, "P%i Button BD", nPlayer + 1);
			BurnDrvGetInputInfo(&bii, nNeogeoButtons[nPlayer][1]);
			pgi->Macro.pVal[0] = bii.pVal;
			pgi->Macro.nVal[0] = 1;
			BurnDrvGetInputInfo(&bii, nNeogeoButtons[nPlayer][3]);
			pgi->Macro.pVal[1] = bii.pVal;
			pgi->Macro.nVal[1] = 1;

			nMacroCount++;
			pgi++;

			// C + D macro
			pgi->nInput = GIT_MACRO_AUTO;
			pgi->nType = BIT_DIGITAL;
			pgi->Macro.nMode = 0;

			sprintf(pgi->Macro.szName, "P%i Button CD", nPlayer + 1);
			BurnDrvGetInputInfo(&bii, nNeogeoButtons[nPlayer][2]);
			pgi->Macro.pVal[0] = bii.pVal;
			pgi->Macro.nVal[0] = 1;
			BurnDrvGetInputInfo(&bii, nNeogeoButtons[nPlayer][3]);
			pgi->Macro.pVal[1] = bii.pVal;
			pgi->Macro.nVal[1] = 1;

			nMacroCount++;
			pgi++;

			// A + B + C macro
			pgi->nInput = GIT_MACRO_AUTO;
			pgi->nType = BIT_DIGITAL;
			pgi->Macro.nMode = 0;

			sprintf(pgi->Macro.szName, "P%i Button ABC", nPlayer + 1);
			for (int i = 0; i < 3; i++) {
				BurnDrvGetInputInfo(&bii, nNeogeoButtons[nPlayer][i]);
				pgi->Macro.pVal[i] = bii.pVal;
				pgi->Macro.nVal[i] = 1;
			}

			nMacroCount++;
			pgi++;

			// B + C + D macro
			pgi->nInput = GIT_MACRO_AUTO;
			pgi->nType = BIT_DIGITAL;
			pgi->Macro.nMode = 0;

			sprintf(pgi->Macro.szName, "P%i Button BCD", nPlayer + 1);
			for (int i = 0; i < 3; i++) {
				BurnDrvGetInputInfo(&bii, nNeogeoButtons[nPlayer][i+1]);
				pgi->Macro.pVal[i] = bii.pVal;
				pgi->Macro.nVal[i] = 1;
			}

			nMacroCount++;
			pgi++;

			// A + B + C + D macro
			pgi->nInput = GIT_MACRO_AUTO;
			pgi->nType = BIT_DIGITAL;
			pgi->Macro.nMode = 0;

			sprintf(pgi->Macro.szName, "P%i Button ABCD", nPlayer + 1);
			for (int i = 0; i < 4; i++) {
				BurnDrvGetInputInfo(&bii, nNeogeoButtons[nPlayer][i]);
				pgi->Macro.pVal[i] = bii.pVal;
				pgi->Macro.nVal[i] = 1;
			}

			nMacroCount++;
			pgi++;
		}

		if (nFireButtons == 4 && (BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_IGS_PGM) {

			// A + B macro
			pgi->nInput = GIT_MACRO_AUTO;
			pgi->nType = BIT_DIGITAL;
			pgi->Macro.nMode = 0;

			sprintf(pgi->Macro.szName, "P%i Button AB", nPlayer + 1);
			BurnDrvGetInputInfo(&bii, nPgmButtons[nPlayer][0]);
			pgi->Macro.pVal[0] = bii.pVal;
			pgi->Macro.nVal[0] = 1;
			BurnDrvGetInputInfo(&bii, nPgmButtons[nPlayer][1]);
			pgi->Macro.pVal[1] = bii.pVal;
			pgi->Macro.nVal[1] = 1;

			nMacroCount++;
			pgi++;

			// B + C macro
			pgi->nInput = GIT_MACRO_AUTO;
			pgi->nType = BIT_DIGITAL;
			pgi->Macro.nMode = 0;

			sprintf(pgi->Macro.szName, "P%i Button BC", nPlayer + 1);
			BurnDrvGetInputInfo(&bii, nPgmButtons[nPlayer][1]);
			pgi->Macro.pVal[0] = bii.pVal;
			pgi->Macro.nVal[0] = 1;
			BurnDrvGetInputInfo(&bii, nPgmButtons[nPlayer][2]);
			pgi->Macro.pVal[1] = bii.pVal;
			pgi->Macro.nVal[1] = 1;

			nMacroCount++;
			pgi++;

			// A + C macro
			pgi->nInput = GIT_MACRO_AUTO;
			pgi->nType = BIT_DIGITAL;
			pgi->Macro.nMode = 0;

			sprintf(pgi->Macro.szName, "P%i Button AC", nPlayer + 1);
			BurnDrvGetInputInfo(&bii, nPgmButtons[nPlayer][0]);
			pgi->Macro.pVal[0] = bii.pVal;
			pgi->Macro.nVal[0] = 1;
			BurnDrvGetInputInfo(&bii, nPgmButtons[nPlayer][2]);
			pgi->Macro.pVal[1] = bii.pVal;
			pgi->Macro.nVal[1] = 1;

			nMacroCount++;
			pgi++;

			// A + B + C macro
			pgi->nInput = GIT_MACRO_AUTO;
			pgi->nType = BIT_DIGITAL;
			pgi->Macro.nMode = 0;

			sprintf(pgi->Macro.szName, "P%i Button ABC", nPlayer + 1);
			for (int i = 0; i < 3; i++) {
				BurnDrvGetInputInfo(&bii, nPgmButtons[nPlayer][i]);
				pgi->Macro.pVal[i] = bii.pVal;
				pgi->Macro.nVal[i] = 1;
			}

			nMacroCount++;
			pgi++;
		}
	}

	if (((nPunchx3[0] == 7) && (nKickx3[0] == 7)) || (nFireButtons >= 5 && (BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_CAPCOM_CPS2))
		bStreetFighterLayout = true;
}
#endif

int GameInpInit()
{
	// Count the number of inputs
	nGameInpCount = 0;
	#ifndef NO_MACROS
	nMacroCount = 0;
	nMaxMacro = nMaxPlayers * nMaxMacroPerPlayer;
	#endif

	for (unsigned int i = 0; i < 0x1000; i++)
	{
		if (BurnDrvGetInputInfo(NULL, i)){			// end of input list
			nGameInpCount = i;
			break;
		}
	}

	// Allocate space for all the inputs
	#ifndef NO_MACROS
	int nSize = (nGameInpCount + nMaxMacro) * sizeof(struct GameInp);
	#else
	int nSize = (nGameInpCount) * sizeof(struct GameInp);
	#endif
	GameInp = (struct GameInp*)malloc(nSize);
	if (GameInp == NULL)
		return 1;

	memset(GameInp, 0, nSize);

	GameInpBlank(1);

	InpDIPSWResetDIPs();

	#ifndef NO_MACROS
	GameInpInitMacros();
	#endif

	nAnalogSpeed = 0x0100;

	return 0;
}

int GameInpExit()
{
	free(GameInp);
	GameInp = NULL;

	nGameInpCount = 0;
	#ifndef NO_MACROS
	nMacroCount = 0;
	#endif

	nFireButtons = 0;

	bStreetFighterLayout = false;
	bLeftAltkeyMapped = false;

	return 0;
}

// ---------------------------------------------------------------------------
// Convert a string from a config file to an input

static char* SliderInfo(struct GameInp* pgi, char* s)
{
	pgi->Input.Slider.nSliderSpeed = 0x700;				// defaults
	pgi->Input.Slider.nSliderCenter = 0;
	pgi->Input.Slider.nSliderValue = 0x8000;

	char* szRet = labelCheck(s, "speed");
	s = szRet;

	if (s == NULL)
		return s;

	pgi->Input.Slider.nSliderSpeed = (short)strtol(s, &szRet, 0);
	s = szRet;

	if (s==NULL)
		return s;

	szRet = labelCheck(s, "center");
	s = szRet;
	if (s == NULL)
		return s;

	pgi->Input.Slider.nSliderCenter = (short)strtol(s, &szRet, 0);
	s = szRet;

	if (s == NULL)
		return s;

	return szRet;
}

static int StringToJoyAxis(struct GameInp* pgi, char* s)
{
	#ifdef USE_JOYAXIS
	char* szRet = s;

	pgi->Input.JoyAxis.nJoy = (unsigned char)strtol(s, &szRet, 0);

	if (szRet == NULL)
		return 1;

	s = szRet;
	pgi->Input.JoyAxis.nAxis = (unsigned char)strtol(s, &szRet, 0);

	if (szRet == NULL)
		return 1;
	#endif

	return 0;
}

static int StringToMouseAxis(struct GameInp* pgi, char* s)
{
	#ifdef USE_MOUSE
	char* szRet = s;

	pgi->Input.MouseAxis.nAxis = (unsigned char)strtol(s, &szRet, 0);

	if (szRet == NULL)
		return 1;
	#endif

	return 0;
}

#ifndef NO_MACROS
static int StringToMacro(struct GameInp* pgi, char* s)
{
	char* szRet = labelCheck(s, "switch");

	if (szRet)
	{
		s = szRet;
		pgi->Macro.nMode = 0x01;
		pgi->Macro.Switch = (unsigned short)strtol(s, &szRet, 0);
		return 0;
	}

	return 1;
}
#endif

static int StringToInp(struct GameInp* pgi, char* s)
{
	SKIP_WS(s);	// skip whitespace
	char* szRet = labelCheck(s, "undefined");
	if (szRet) {
		pgi->nInput = 0;
		return 0;
	}

	szRet = labelCheck(s, "constant");
	if (szRet) {
		pgi->nInput = GIT_CONSTANT;
		s = szRet;
		pgi->Input.Constant = (unsigned char)strtol(s, &szRet, 0);
		*(pgi->Input.pVal) = pgi->Input.Constant;
		return 0;
	}

	szRet = labelCheck(s, "switch");
	if (szRet) {
		pgi->nInput = GIT_SWITCH;
		s = szRet;
		pgi->Input.Switch = (unsigned short)strtol(s, &szRet, 0);
		return 0;
	}

	// Analog using mouse axis:
	szRet = labelCheck(s, "mouseaxis");
	if (szRet) {
		pgi->nInput = GIT_MOUSEAXIS;
		return StringToMouseAxis(pgi, szRet);
	}
	// Analog using joystick axis:
	szRet = labelCheck(s, "joyaxis-neg");
	if (szRet) {
		pgi->nInput = GIT_JOYAXIS_NEG;
		return StringToJoyAxis(pgi, szRet);
	}
	szRet = labelCheck(s, "joyaxis-pos");
	if (szRet) {
		pgi->nInput = GIT_JOYAXIS_POS;
		return StringToJoyAxis(pgi, szRet);
	}
	szRet = labelCheck(s, "joyaxis");
	if (szRet) {
		pgi->nInput = GIT_JOYAXIS_FULL;
		return StringToJoyAxis(pgi, szRet);
	}

	// Analog using keyboard slider
	szRet = labelCheck(s, "slider");
	if (szRet)
	{
		s = szRet;
		pgi->nInput = GIT_KEYSLIDER;
		pgi->Input.Slider.SliderAxis[0] = 0;	// defaults
		pgi->Input.Slider.SliderAxis[1] = 0;	//

		pgi->Input.Slider.SliderAxis[0] = (unsigned short)strtol(s, &szRet, 0);
		s = szRet;
		if (s == NULL)
			return 1;

		pgi->Input.Slider.SliderAxis[1] = (unsigned short)strtol(s, &szRet, 0);
		s = szRet;

		if (s == NULL)
			return 1;

		szRet = SliderInfo(pgi, s);
		s = szRet;

		if (s == NULL) // Get remaining slider info
			return 1;

		return 0;
	}

	// Analog using joystick slider
	szRet = labelCheck(s, "joyslider");
	if (szRet)
	{
		s = szRet;
		pgi->nInput = GIT_JOYSLIDER;
		#ifdef USE_JOYAXIS
		pgi->Input.Slider.JoyAxis.nJoy = 0;	// defaults
		pgi->Input.Slider.JoyAxis.nAxis = 0;

		pgi->Input.Slider.JoyAxis.nJoy = (unsigned char)strtol(s, &szRet, 0);
		s = szRet;

		if (s == NULL)
			return 1;

		pgi->Input.Slider.JoyAxis.nAxis = (unsigned char)strtol(s, &szRet, 0);
		s = szRet;

		if (s == NULL)
			return 1;
		#endif

		szRet = SliderInfo(pgi, s); // Get remaining slider info
		s = szRet;

		if (s == NULL)
			return 1;

		return 0;
	}

	return 1;
}

// ---------------------------------------------------------------------------
// Convert an input to a string for config files

static char* InpToString(struct GameInp* pgi)
{
	static char szString[80];

	switch(pgi->nInput)
	{
		case 0:
			sprintf(szString, "undefined");
			break;
		case GIT_CONSTANT:
			sprintf(szString, "constant 0x%.2X", pgi->Input.Constant);
			break;
		case GIT_SWITCH:
			sprintf(szString, "switch 0x%.2X", pgi->Input.Switch);
			break;
		case GIT_KEYSLIDER:
			sprintf(szString, "slider 0x%.2x 0x%.2x speed 0x%x center %d", pgi->Input.Slider.SliderAxis[0], pgi->Input.Slider.SliderAxis[1], pgi->Input.Slider.nSliderSpeed, pgi->Input.Slider.nSliderCenter);
			break;
		#ifdef USE_JOYAXIS
		case GIT_JOYSLIDER:
			sprintf(szString, "joyslider %d %d speed 0x%x center %d", pgi->Input.Slider.JoyAxis.nJoy, pgi->Input.Slider.JoyAxis.nAxis, pgi->Input.Slider.nSliderSpeed, pgi->Input.Slider.nSliderCenter);
			break;
		#endif
		#ifdef USE_MOUSE
		case GIT_MOUSEAXIS:
			sprintf(szString, "mouseaxis %d", pgi->Input.MouseAxis.nAxis);
			break;
		#endif
		#ifdef USE_JOYAXIS
		case GIT_JOYAXIS_FULL:
			sprintf(szString, "joyaxis %d %d", pgi->Input.JoyAxis.nJoy, pgi->Input.JoyAxis.nAxis);
			break;
		case GIT_JOYAXIS_NEG:
			sprintf(szString, "joyaxis-neg %d %d", pgi->Input.JoyAxis.nJoy, pgi->Input.JoyAxis.nAxis);
			break;
		case GIT_JOYAXIS_POS:
			sprintf(szString, "joyaxis-pos %d %d", pgi->Input.JoyAxis.nJoy, pgi->Input.JoyAxis.nAxis);
			break;
		#endif
		default:
			sprintf(szString, "unknown");
			break;
	}

	return szString;
}

#ifndef NO_MACROS
static char* InpMacroToString(struct GameInp* pgi)
{
	static char szString[MAX_PATH];

	if (pgi->nInput == GIT_MACRO_AUTO)
	{
		if (pgi->Macro.nMode)
		{
			sprintf(szString, "switch 0x%.2X", pgi->Macro.Switch);
			return szString;
		}
	}

	if (pgi->nInput == GIT_MACRO_CUSTOM)
	{
		struct BurnInputInfo bii;

		if (pgi->Macro.nMode)
			sprintf(szString, "switch 0x%.2X", pgi->Macro.Switch);
		else
			sprintf(szString, "undefined");

		for (int i = 0; i < 4; i++)
		{
			if (pgi->Macro.pVal[i])
			{
				BurnDrvGetInputInfo(&bii, pgi->Macro.nInput[i]);
				sprintf(szString + strlen(szString), " \"%hs\" 0x%02X", bii.szName, pgi->Macro.nVal[i]);
			}
		}

		return szString;
	}

	return "undefined";
}
#endif

// ---------------------------------------------------------------------------
// Generate a user-friendly name for a control (PC-side)

#define FBK_DEFNAME(k) k, _T(#k)

static struct
{
	int nCode;
	const char* szName;
} KeyNames[] = {
	{ FBK_ESCAPE,				"ESCAPE" },
	{ FBK_1,				"Start Button" },
	{ FBK_2,				"Start Button" },
	{ FBK_3,				"Start Button" },
	{ FBK_4,				"Start Button" },
#if defined (_XBOX)
	{ FBK_5,				"Back Button" },
	{ FBK_6,				"Back Button" },
	{ FBK_7,				"Back Button" },
	{ FBK_8,				"Back Button" },
#else
	{ FBK_5,				"Select Button" },
	{ FBK_6,				"Select Button" },
	{ FBK_7,				"Select Button" },
	{ FBK_8,				"Select Button" },
#endif
#if defined (_XBOX)
	{ FBK_9,				"Left Thumb" },
#else
	{ FBK_9,				"L3 Button" },
#endif
	{ FBK_0,				"0" },
	{ FBK_MINUS,				"MINUS" },
	{ FBK_EQUALS,				"EQUALS" },
	{ FBK_BACK,				"BACKSPACE" },
	{ FBK_TAB,				"TAB" },
	{ FBK_Q,				"Q" },
	{ FBK_W,				"W" },
	{ FBK_E,				"E" },
	{ FBK_R,				"R" },
	{ FBK_T,				"T" },
	{ FBK_Y,				"Y" },
	{ FBK_U,				"U" },
	{ FBK_I,				"I" },
	{ FBK_O,				"O" },
	{ FBK_P,				"P" },
	{ FBK_LBRACKET,				"OPENING BRACKET" },
	{ FBK_RBRACKET,				"CLOSING BRACKET" },
	{ FBK_RETURN,				"ENTER" },
	{ FBK_LCONTROL,				"LEFT CONTROL" },
	{ FBK_A,				"Y Button" },
#if defined (_XBOX)
	{ FBK_S,				"Left Shoulder" },
	{ FBK_D,				"Right Shoulder" },
#else
	{ FBK_S,				"L1 Button" },
	{ FBK_D,				"R1 Button" },
#endif
	{ FBK_F,				"F" },
	{ FBK_G,				"G" },
	{ FBK_H,				"H" },
	{ FBK_J,				"J" },
	{ FBK_K,				"K" },
	{ FBK_L,				"L" },
	{ FBK_SEMICOLON,			"SEMICOLON" },
	{ FBK_APOSTROPHE,			"APOSTROPHE" },
	{ FBK_GRAVE,				"ACCENT GRAVE" },
	{ FBK_LSHIFT,				"LEFT SHIFT" },
	{ FBK_BACKSLASH,			"BACKSLASH" },
#if defined (_XBOX)
	{ FBK_Z,				"A Button" },
	{ FBK_X,				"B Button" },
	{ FBK_C,				"X Button" },
	{ FBK_V,				"Y Button" },
#else
	{ FBK_Z,				"Cross Button" },
	{ FBK_X,				"Circle Button" },
	{ FBK_C,				"Square Button" },
	{ FBK_V,				"Triangle Button" },
#endif
	{ FBK_B,				"B" },
	{ FBK_N,				"N" },
	{ FBK_M,				"M" },
	{ FBK_COMMA,				"COMMA" },
	{ FBK_PERIOD,				"PERIOD" },
	{ FBK_SLASH,				"SLASH" },
	{ FBK_RSHIFT,				"RIGHT SHIFT" },
	{ FBK_MULTIPLY,				"NUMPAD MULTIPLY" },
	{ FBK_LALT,				"LEFT MENU" },
	{ FBK_SPACE,				"SPACE" },
	{ FBK_CAPITAL,				"CAPSLOCK" },
#if defined (_XBOX)
	{ FBK_F1,				"Left Thumb" },
	{ FBK_F2,				"Right Thumb" },
	{ FBK_F3,				"Right Thumb" },
#else
	{ FBK_F1,				"L3 Button" },
	{ FBK_F2,				"R3 Button" },
	{ FBK_F3,				"R3 Button" },
#endif
	{ FBK_F4,				"F4" },
	{ FBK_F5,				"F5" },
	{ FBK_F6,				"F6" },
	{ FBK_F7,				"F7" },
	{ FBK_F8,				"F8" },
	{ FBK_F9,				"F9" },
	{ FBK_F10,				"F10" },
	{ FBK_NUMLOCK,				"NUMLOCK" },
	{ FBK_SCROLL,				"SCROLLLOCK" },
	{ FBK_NUMPAD7,				"NUMPAD 7" },
	{ FBK_NUMPAD8,				"NUMPAD 8" },
	{ FBK_NUMPAD9,				"NUMPAD 9" },
	{ FBK_SUBTRACT,				"NUMPAD SUBTRACT" },
	{ FBK_NUMPAD4,				"NUMPAD 4" },
	{ FBK_NUMPAD5,				"NUMPAD 5" },
	{ FBK_NUMPAD6,				"NUMPAD 6" },
	{ FBK_ADD,				"NUMPAD ADD" },
	{ FBK_NUMPAD1,				"NUMPAD 1" },
	{ FBK_NUMPAD2,				"NUMPAD 2" },
	{ FBK_NUMPAD3,				"NUMPAD 3" },
	{ FBK_NUMPAD0,				"NUMPAD 0" },
	{ FBK_DECIMAL,				"NUMPAD DECIMAL POINT" },
	{ FBK_DEFNAME(FBK_OEM_102) },
	{ FBK_F11,				"F11" },
	{ FBK_F12,				"F12" },
	{ FBK_F13,				"F13" },
	{ FBK_F14,				"F14" },
	{ FBK_F15,				"F15" },
	{ FBK_DEFNAME(FBK_KANA) },
	{ FBK_DEFNAME(FBK_ABNT_C1) },
	{ FBK_DEFNAME(FBK_CONVERT) },
	{ FBK_DEFNAME(FBK_NOCONVERT) },
	{ FBK_DEFNAME(FBK_YEN) },
	{ FBK_DEFNAME(FBK_ABNT_C2) },
	{ FBK_NUMPADEQUALS,			"NUMPAD EQUALS" },
	{ FBK_DEFNAME(FBK_PREVTRACK) },
	{ FBK_DEFNAME(FBK_AT) },
	{ FBK_COLON,				"COLON" },
	{ FBK_UNDERLINE,			"UNDERSCORE" },
	{ FBK_DEFNAME(FBK_KANJI) },
	{ FBK_DEFNAME(FBK_STOP) },
	{ FBK_DEFNAME(FBK_AX) },
	{ FBK_DEFNAME(FBK_UNLABELED) },
	{ FBK_DEFNAME(FBK_NEXTTRACK) },
	{ FBK_NUMPADENTER,			"NUMPAD ENTER" },
	{ FBK_RCONTROL,				"RIGHT CONTROL" },
	{ FBK_DEFNAME(FBK_MUTE) },
	{ FBK_DEFNAME(FBK_CALCULATOR) },
	{ FBK_DEFNAME(FBK_PLAYPAUSE) },
	{ FBK_DEFNAME(FBK_MEDIASTOP) },
	{ FBK_DEFNAME(FBK_VOLUMEDOWN) },
	{ FBK_DEFNAME(FBK_VOLUMEUP) },
	{ FBK_DEFNAME(FBK_WEBHOME) },
	{ FBK_DEFNAME(FBK_NUMPADCOMMA) },
	{ FBK_DIVIDE,				"NUMPAD DIVIDE" },
	{ FBK_SYSRQ,				"PRINTSCREEN" },
	{ FBK_RALT,				"RIGHT MENU" },
	{ FBK_PAUSE,				"PAUSE" },
	{ FBK_HOME,				"HOME" },
	{ FBK_UPARROW,				"ARROW UP" },
	{ FBK_PRIOR,				"PAGE UP" },
	{ FBK_LEFTARROW,			"ARROW LEFT" },
	{ FBK_RIGHTARROW,			"ARROW RIGHT" },
	{ FBK_END,				"END" },
	{ FBK_DOWNARROW,			"ARROW DOWN" },
	{ FBK_NEXT,				"PAGE DOWN" },
	{ FBK_INSERT,				"INSERT" },
	{ FBK_DELETE,				"DELETE"},
	{ FBK_LWIN,				"LEFT WINDOWS" },
	{ FBK_RWIN,				"RIGHT WINDOWS" },
	{ FBK_DEFNAME(FBK_APPS) },
	{ FBK_DEFNAME(FBK_POWER) },
	{ FBK_DEFNAME(FBK_SLEEP) },
	{ FBK_DEFNAME(FBK_WAKE) },
	{ FBK_DEFNAME(FBK_WEBSEARCH) },
	{ FBK_DEFNAME(FBK_WEBFAVORITES) },
	{ FBK_DEFNAME(FBK_WEBREFRESH) },
	{ FBK_DEFNAME(FBK_WEBSTOP) },
	{ FBK_DEFNAME(FBK_WEBFORWARD) },
	{ FBK_DEFNAME(FBK_WEBBACK) },
	{ FBK_DEFNAME(FBK_MYCOMPUTER) },
	{ FBK_DEFNAME(FBK_MAIL) },
	{ FBK_DEFNAME(FBK_MEDIASELECT) },
#if defined (_XBOX)
	{ 0x88,					"Left Trigger" },
	{ 0x8A,					"Right Trigger" },
#else
	{ 0x88,					"L2 Button" },
	{ 0x8A,					"R2 Button" },
#endif
#undef FBK_DEFNAME
	{ 0,					NULL }
};

char* InputCodeDesc(int c)
{
	static char szString[64];
	char* szName = "";

	// Mouse
	if (c >= 0x8000)
	{
		int nMouse = (c >> 8) & 0x3F;
		int nCode = c & 0xFF;
		if (nCode >= 0x80)
		{
			sprintf(szString, "Mouse %d Button %d", nMouse, nCode & 0x7F);
			return szString;
		}
		if (nCode < 0x06)
		{
			char szAxis[3][3] = { "X", "Y", "Z" };
			char szDir[6][16] = { "negative", "positive", "Left", "Right", "Up", "Down" };
			if (nCode < 4)
				sprintf(szString, "Mouse %d %s (%s %s)", nMouse, szDir[nCode + 2], szAxis[nCode >> 1], szDir[nCode & 1]);
			else
				sprintf(szString, "Mouse %d %s %s", nMouse, szAxis[nCode >> 1], szDir[nCode & 1]);
			return szString;
		}
	}

	// Joystick
	if (c >= 0x4000 && c < 0x8000)
	{
		int nJoy = (c >> 8) & 0x3F;
		int nCode = c & 0xFF;
		if (nCode >= 0x80)
		{
#if defined (_XBOX)

			switch(c)
			{
				case 0x4080:
				case 0x4180:
				case 0x4280:
					sprintf(szString, "A Button");
					break;
				case 0x4081:
				case 0x4181:
				case 0x4281:
					sprintf(szString, "B Button");
					break;
				case 0x4082:
				case 0x4182:
				case 0x4282:
					sprintf(szString, "X Button");
					break;
				case 0x4083:
				case 0x4183:
				case 0x4283:
					sprintf(szString, "Y Button");
					break;
				case 0x4084:
				case 0x4184:
				case 0x4284:
					sprintf(szString, "Left Shoulder");
					break;
				case 0x4085:
				case 0x4185:
				case 0x4285:
					sprintf(szString, "Right Shoulder");
					break;

			}

#elif defined (SN_TARGET_PS3)
			switch(c)
			{
				case 0x4080:
				case 0x4180:
				case 0x4280:
					sprintf(szString, "Cross Button");
					break;
				case 0x4081:
				case 0x4181:
				case 0x4281:
					sprintf(szString, "Circle Button");
					break;
				case 0x4082:
				case 0x4182:
				case 0x4282:
					sprintf(szString, "Square Button");
					break;
				case 0x4083:
				case 0x4183:
				case 0x4283:
					sprintf(szString, "Triangle Button");
					break;
				case 0x4084:
				case 0x4184:
				case 0x4284:
					sprintf(szString, "L1 Button");
					break;
				case 0x4085:
				case 0x4185:
				case 0x4285:
					sprintf(szString, "R1 Button");
					break;
				case 0x4088:
				case 0x4188:
				case 0x4288:
					sprintf(szString, "L2 Button");
					break;
				case 0x408A:
				case 0x418A:
				case 0x428A:
					sprintf(szString, "R2 Button");
					break;
				case 0x408B:
				case 0x418B:
				case 0x428B:
					sprintf(szString, "L3 Button");
					break;
				case 0x408C:
				case 0x418C:
				case 0x428C:
					sprintf(szString, "R3 Button");
					break;

			}			
#else
			sprintf(szString, "Joy %d Button %d", nJoy, nCode & 0x7F);
#endif
			return szString;
		}
		if (nCode < 0x10)
		{
			char szAxis[8][3] = { "X", "Y", "Z", "rX", "rY", "rZ", "s0", "s1" };
			char szDir[6][16] = { "negative", "positive", "Left", "Right", "Up", "Down" };
			if (nCode < 4)
				sprintf(szString, "Joy %d %s (%s %s)", nJoy, szDir[nCode + 2], szAxis[nCode >> 1], szDir[nCode & 1]);
			else
				sprintf(szString, "Joy %d %s %s", nJoy, szAxis[nCode >> 1], szDir[nCode & 1]);
			return szString;
		}
		if (nCode < 0x20)
		{
			char szDir[4][16] = { "Left", "Right", "Up", "Down" };
			sprintf(szString, "Joy %d POV-hat %d %s", nJoy, (nCode & 0x0F) >> 2, szDir[nCode & 3]);
			return szString;
		}
	}

	for (int i = 0; KeyNames[i].nCode; i++)
	{
		if (c == KeyNames[i].nCode)
		{
			if (KeyNames[i].szName)
				szName = (char*)KeyNames[i].szName;
			break;
		}
	}

	if (szName[0])
		sprintf(szString, "%s", szName);
	else
		sprintf(szString, "code 0x%.2X", c);

	return szString;
}

const char * InpToDesc(struct GameInp* pgi)
{
	static char szInputName[64] = "";

	if (pgi->nInput == 0)
		return "";

	if (pgi->nInput == GIT_CONSTANT)
	{
		if (pgi->nType & BIT_GROUP_CONSTANT)
		{
			for (int i = 0; i < 8; i++)
				szInputName[7 - i] = pgi->Input.Constant & (1 << i) ? '1' : '0';
			szInputName[8] = 0;

			return szInputName;
		}

		if (pgi->Input.Constant == 0)
			return "-";
	}
	if (pgi->nInput == GIT_SWITCH)
		return InputCodeDesc(pgi->Input.Switch);

	#ifdef USE_MOUSE
	if (pgi->nInput == GIT_MOUSEAXIS)
	{
		char nAxis = '?';
		switch (pgi->Input.MouseAxis.nAxis)
		{
			case 0:
				nAxis = 'X';
				break;
			case 1:
				nAxis = 'Y';
				break;
			case 2:
				nAxis = 'Z';
				break;
		}
		sprintf(szInputName, "Mouse %i %c axis", pgi->Input.MouseAxis.nMouse, nAxis);
		return szInputName;
	}
	#endif
#ifdef USE_JOYAXIS
	if (pgi->nInput & GIT_GROUP_JOYSTICK)
	{
		char szAxis[8][3] = { "X", "Y", "Z", "rX", "rY", "rZ", "s0", "s1" };
		char szRange[4][16] = { "unknown", "full", "negative", "positive" };
		int nRange = 0;
		switch (pgi->nInput)
		{
			case GIT_JOYAXIS_FULL:
				nRange = 1;
				break;
			case GIT_JOYAXIS_NEG:
				nRange = 2;
				break;
			case GIT_JOYAXIS_POS:
				nRange = 3;
				break;
		}

		sprintf(szInputName, "Joy %d %s axis (%s range)", pgi->Input.JoyAxis.nJoy, szAxis[pgi->Input.JoyAxis.nAxis], szRange[nRange]);
		return szInputName;
	}
#endif

	return InpToString(pgi); // Just do the rest as they are in the config file
}

#ifndef NO_MACROS
char* InpMacroToDesc(struct GameInp* pgi)
{
	if (pgi->nInput & GIT_GROUP_MACRO)
	{
		if (pgi->Macro.nMode)
			return InputCodeDesc(pgi->Macro.Switch);
	}

	return "";
}
#endif

// ---------------------------------------------------------------------------

// Find the input number by info
static unsigned int InputInfoToNum(char* szName)
{
	struct BurnInputInfo bii;
	for (unsigned int i = 0; i < nGameInpCount; i++)
	{
		BurnDrvGetInputInfo(&bii, i);
		if (bii.pVal == NULL)
			continue;

		if (strcasecmp(szName, bii.szInfo) == 0)
			return i;
	}
	return ~0U;
}

// Find the input number by name
static unsigned int InputNameToNum(char* szName)
{
	struct BurnInputInfo bii;
	for (unsigned int i = 0; i < nGameInpCount; i++)
	{
		BurnDrvGetInputInfo(&bii, i);
		if (bii.pVal == NULL)
			continue;

		if (strcasecmp(szName, bii.szName) == 0)
			return i;
	}
	return ~0U;
}

// Get the input name by number
static char* InputNumToName(unsigned int i)
{
	struct BurnInputInfo bii;
	bii.szName = NULL;
	BurnDrvGetInputInfo(&bii, i);
	if (bii.szName == NULL)
		return "unknown";
	return (char *)bii.szName;
}

// Get the input info by number, added by regret
static char* InputNumToInfo(unsigned int i)
{
	struct BurnInputInfo bii;
	bii.szName = NULL;
	BurnDrvGetInputInfo(&bii, i);
	if (bii.szInfo == NULL)
		return "unknown";
	return (char *)bii.szInfo;
}

#ifndef NO_MACROS
static unsigned int MacroNameToNum(char* szName)
{
	struct GameInp* pgi = GameInp + nGameInpCount;
	for (unsigned int i = 0; i < nMacroCount; i++, pgi++)
	{
		if (pgi->nInput & GIT_GROUP_MACRO)
		{
			if (strcasecmp(szName, pgi->Macro.szName) == 0)
				return i;
		}
	}
	return ~0U;
}
#endif

// ---------------------------------------------------------------------------

static int GameInpAutoOne(struct GameInp* pgi, char* szi)
{

	for (int i = 0; i < nMaxPlayers; i++) {
		int nSlide = nPlayerDefaultControls[i] >> 4;
		switch (nPlayerDefaultControls[i] & 0x0F)
		{
			case 0:										// Keyboard

				if (ArcadeJoystick!=1)
				{
					GamcAnalogKey(pgi, szi, i, nSlide);
					GamcPlayer(pgi, szi, i, -1);
					GamcMisc(pgi, szi, i);
				}
				else
				{
					//GamcMisc(pgi, szi, i);
					//GamcPlayerHori(pgi, szi, i, nSlide);
					GamcAnalogKey(pgi, szi, i, nSlide);
					GamcPlayer(pgi, szi, i, -1);
					GamcMisc(pgi, szi, i);
				}
				break;
			case 1:										// Joystick 1
				GamcAnalogJoy(pgi, szi, i, 0, nSlide);
				GamcPlayer(pgi, szi, i, 0);
				GamcMisc(pgi, szi, i);
				break;
			case 2:										// Joystick 2
				GamcAnalogJoy(pgi, szi, i, 1, nSlide);
				GamcPlayer(pgi, szi, i, 1);
				GamcMisc(pgi, szi, i);
				break;
			case 3:										// Joystick 3
				GamcAnalogJoy(pgi, szi, i, 2, nSlide);
				GamcPlayer(pgi, szi, i, 2);
				GamcMisc(pgi, szi, i);
				break;
			case 4:										// X-Arcade left side
				GamcMisc(pgi, szi, i);
				GamcPlayerHotRod(pgi, szi, i, 0x10, nSlide);
				break;
			case 5:										// X-Arcade right side
				GamcMisc(pgi, szi, i);
				GamcPlayerHotRod(pgi, szi, i, 0x11, nSlide);
				break;
			case 6:										// Hot Rod left side
				GamcMisc(pgi, szi, i);
				GamcPlayerHotRod(pgi, szi, i, 0x00, nSlide);
				break;
			case 7:										// Hot Rod right side
				GamcMisc(pgi, szi, i);
				GamcPlayerHotRod(pgi, szi, i, 0x01, nSlide);
				break;
			default:
				GamcMisc(pgi, szi, i);
		}
	}

	return 0;
}

#ifndef NO_MACROS
static int AddCustomMacro(char* szValue, bool bOverWrite)
{
	char* szQuote = NULL;
	char* szEnd = NULL;
	if (quoteRead(&szQuote, &szEnd, szValue))
		return 1;

	int nMode = -1;
	int nInput = -1;
	bool bCreateNew = false;
	struct BurnInputInfo bii;

	for (unsigned int j = nGameInpCount; j < nGameInpCount + nMacroCount; j++)
	{
		if (GameInp[j].nInput == GIT_MACRO_CUSTOM)
		{
			if (labelCheck(szQuote, (char *)GameInp[j].Macro.szName))
			{
				nInput = j;
				break;
			}
		}
	}

	if (nInput == -1)
	{
		if (nMacroCount + 1 == nMaxMacro)
			return 1;
		nInput = nGameInpCount + nMacroCount;
		bCreateNew = true;
	}

	strcpy(szQuote, GameInp[nInput].Macro.szName);

	if ((szValue = labelCheck(szEnd, "undefined")) != NULL)
		nMode = 0;
	else
	{
		if ((szValue = labelCheck(szEnd, "switch")) != NULL)
		{
			if (bOverWrite || GameInp[nInput].Macro.nMode == 0)
				GameInp[nInput].Macro.Switch = (unsigned short)strtol(szValue, &szValue, 0);

			nMode = 1;
		}
	}

	if (nMode >= 0)
	{
		int nFound = 0;

		for (int i = 0; i < 4; i++)
		{
			GameInp[nInput].Macro.pVal[i] = NULL;
			GameInp[nInput].Macro.nVal[i] = 0;
			GameInp[nInput].Macro.nInput[i] = 0;

			if (szValue == NULL)
				break;

			if (quoteRead(&szQuote, &szEnd, szValue))
				break;

			for (unsigned int j = 0; j < nGameInpCount; j++)
			{
				bii.szName = NULL;
				BurnDrvGetInputInfo(&bii, j);
				if (bii.pVal == NULL)
					continue;

				char* szString = labelCheck(szQuote, (char *)bii.szName);
				if (szString && szEnd)
				{
					GameInp[nInput].Macro.pVal[i] = bii.pVal;
					GameInp[nInput].Macro.nInput[i] = j;

					GameInp[nInput].Macro.nVal[i] = (unsigned char)strtol(szEnd, &szValue, 0);

					nFound++;

					break;
				}
			}
		}

		if (nFound) {
			if (GameInp[nInput].Macro.pVal[nFound - 1])
			{
				GameInp[nInput].nInput = GIT_MACRO_CUSTOM;
				GameInp[nInput].Macro.nMode = nMode;

				if (bCreateNew)
					nMacroCount++;
				return 0;
			}
		}
	}

	return 1;
}
#endif

int GameInputAutoIni(int nPlayer, const char * lpszFile, bool bOverWrite)
{
	nAnalogSpeed = 0x0100;

	FILE* h = fopen(lpszFile, "rt");
	if (h == NULL)
		return 1;

	char szLine[1024];
	int nFileVersion = 0;
	unsigned int i;

	// Go through each line of the config file and process inputs
	while (fgets(szLine, sizeof(szLine), h))
	{
		char* szValue;
		size_t nLen = strlen(szLine);

		// Get rid of the linefeed at the end
		if (szLine[nLen - 1] == 10)
		{
			szLine[nLen - 1] = 0;
			nLen--;
		}

		szValue = labelCheck(szLine, "version");
		if (szValue)
			nFileVersion = strtol(szValue, NULL, 0);

		szValue = labelCheck(szLine, "analog");

		if (szValue)
			nAnalogSpeed = strtol(szValue, NULL, 0);

		if (nConfigMinVersion <= nFileVersion && nFileVersion <= nBurnVer)
		{
			szValue = labelCheck(szLine, "input");
			if (szValue)
			{
				char* szQuote = NULL;
				char* szEnd = NULL;
				if (quoteRead(&szQuote, &szEnd, szValue))
					continue;

				if ((szQuote[0] == 'p' || szQuote[0] == 'P') && szQuote[1] >= '1' && szQuote[1] <= '0' + nMaxPlayers && szQuote[2] == ' ')
				{
					if (szQuote[1] != '1' + nPlayer)
						continue;
				}
				else
				{
					if (nPlayer != 0)
						continue;
				}

				// Find which input number this refers to
				i = InputInfoToNum(szQuote);
				if (i == ~0U)
				{
					i = InputNameToNum(szQuote);
					if (i == ~0U)
						continue;
				}

				if (GameInp[i].nInput == 0 || bOverWrite) // Undefined - assign mapping
					StringToInp(GameInp + i, szEnd);
			}

			#ifndef NO_MACROS
			szValue = labelCheck(szLine, "macro");
			if (szValue)
			{
				char* szQuote = NULL;
				char* szEnd = NULL;
				if (quoteRead(&szQuote, &szEnd, szValue))
					continue;

				i = MacroNameToNum(szQuote);
				if (i != ~0U)
				{
					i += nGameInpCount;
					if (GameInp[i].Macro.nMode == 0 || bOverWrite) // Undefined - assign mapping
						StringToMacro(GameInp + i, szEnd);
				}
			}

			szValue = labelCheck(szLine, "custom");
			if (szValue)
				AddCustomMacro(szValue, bOverWrite);
			#endif
		}
	}

	fclose(h);

	return 0;
}

// Auto-configure any undefined inputs to defaults
int GameInpDefault()
{

	struct GameInp* pgi;
	struct BurnInputInfo bii;
	unsigned int i;

	for (int nPlayer = 0; nPlayer < nMaxPlayers; nPlayer++)
	{
		if ((nPlayerDefaultControls[nPlayer] & 0x0F) != 0x0F)
			continue;

		GameInputAutoIni(nPlayer, szPlayerDefaultIni[nPlayer], false);
	}

	// Fill all inputs still undefined
	for (i = 0, pgi = GameInp; i < nGameInpCount; i++, pgi++)
	{
		if (pgi->nInput) // Already defined - leave it alone
			continue;

		// Get the extra info about the input
		bii.szInfo = NULL;
		BurnDrvGetInputInfo(&bii, i);
		if (bii.pVal == NULL)
			continue;

		if (bii.szInfo == NULL)
			bii.szInfo = "";

		// Dip switches - set to constant
		if (bii.nType & BIT_GROUP_CONSTANT)
		{
			pgi->nInput = GIT_CONSTANT;
			continue;
		}

		GameInpAutoOne(pgi, (char*)bii.szInfo);
	}

	#ifndef NO_MACROS
	// Fill in macros still undefined
	for (i = 0; i < nMacroCount; i++, pgi++)
	{
		if (pgi->nInput != GIT_MACRO_AUTO || pgi->Macro.nMode)	// Already defined - leave it alone
			continue;

		GameInpAutoOne(pgi, pgi->Macro.szName);
	}
	#endif

	return 0;
}

// ---------------------------------------------------------------------------
// Write all the GameInps out to config file 'h'


int GameInpWrite(FILE* h, bool bWriteConst)
{
	// Write input types
	for (unsigned int i = 0; i < nGameInpCount; i++)
	{
		struct GameInp* pgi = GameInp + i;
		if (pgi->nInput == GIT_CONSTANT && !bWriteConst)
			continue;	// skip constant, added by regret

		// save input info name instead, modified by regret
		char* szName = NULL;
		if (pgi->nInput == GIT_CONSTANT)
			szName = InputNumToName(i);
		else
			szName = InputNumToInfo(i);	//InputNumToName(i);
#if defined (_XBOX)
		fprintf(h, "input  \"%S\" ", szName);
#else
		fprintf(h, "input  \"%s\" ", szName);
#endif

		int nPad = 16 - strlen(szName);
		for (int j = 0; j < nPad; j++)
			fprintf(h, " ");

		fprintf(h, "%s\n", InpToString(GameInp + i));
	}

	fprintf(h, "\n");

	#ifndef NO_MACROS
	struct GameInp* pgi = GameInp + nGameInpCount;
	for (unsigned int i = 0; i < nMacroCount; i++, pgi++)
	{
		int nPad = 0;

		if (pgi->nInput & GIT_GROUP_MACRO)
		{
			switch (pgi->nInput)
			{
				case GIT_MACRO_AUTO:	// Auto-assigned macros
					fprintf(h, "macro  \"%hs\" ", pgi->Macro.szName);
					break;
				case GIT_MACRO_CUSTOM:	// Custom macros
					fprintf(h, "custom \"%hs\" ", pgi->Macro.szName);
					break;
				default:		// Unknown -- ignore
					continue;
			}

			nPad = 16 - strlen(pgi->Macro.szName);
			for (int j = 0; j < nPad; j++)
				fprintf(h, " ");

			fprintf(h, "%s\n", InpMacroToString(pgi));
		}
	}
	#endif

	return 0;
}

// Read a GameInp in
int GameInpRead(char* szVal, bool bOverWrite)
{
	char* szQuote = NULL;
	char* szEnd = NULL;

	int nRet = quoteRead(&szQuote, &szEnd, szVal);
	if (nRet)
		return 1;

	// Find which input number this refers to
	unsigned int i = InputInfoToNum(szQuote);
	if (i == ~0U)
	{
		i = InputNameToNum(szQuote);
		if (i == ~0U)
			return 1;
	}

	if (bOverWrite || GameInp[i].nInput == 0)
		StringToInp(GameInp + i, szEnd); // Parse the input description into the GameInp structure

	return 0;
}

#ifndef NO_MACROS
int GameInpMacroRead(char* szVal, bool bOverWrite)
{
	char* szQuote = NULL;
	char* szEnd = NULL;

	int nRet = quoteRead(&szQuote, &szEnd, szVal);
	if (nRet)
		return 1;

	unsigned int i = MacroNameToNum(szQuote);
	if (i != ~0U)
	{
		i += nGameInpCount;

		if (GameInp[i].Macro.nMode == 0 || bOverWrite)
			StringToMacro(GameInp + i, szEnd);
	}

	return 0;
}
#endif

#ifndef NO_MACROS
int GameInpCustomRead(char* szVal, bool bOverWrite)
{
	return AddCustomMacro(szVal, bOverWrite);
}
#endif
