 // FB Alpha - Emulator for MC68000/Z80 based arcade games
//            Refer to the "license.txt" file for more info

// Main module

#include <string.h>
#include <cell/sysmodule.h>
#include <sysutil/sysutil_sysparam.h>
#include <sysutil/sysutil_msgdialog.h>
#include <sys/paths.h>
#include "burner.h"
#include "version.h"
#include "menu.h"
#include "vid_psgl.h"
#include "cellframework2/input/pad_input.h"
#include "cellframework2/audio/stream.h"
#include "input_driver.h"
#include "audio_driver.h"
#include "string.h"
#ifdef CELL_DEBUG_CONSOLE
#include <cell/control_console.h>
#endif
#include "config_file.h"

#define NORMAL_CONTROLS 0
#define ANALOG_CONTROLS 1

char		szAppBurnVer[16] = "";
char		szSVNVer[16] = "";
char		szSVNDate[30] = "";
int		ArcadeJoystick = 0;
int		exitGame = 0;
uint32_t	bBurnFirstStartup;
int		GameStatus = MENU;
int		nAppVirtualFps = 6000;
int		is_running = 0;
bool		bShowFPS = false;
int		custom_aspect_ratio_mode = 0;
cell_audio_handle_t audio_handle;
const struct cell_audio_driver *driver;
static int	controls;
bool bInputOkay = false;
static uint64_t keybinds[1024][2] = {0}; 
bool DoReset = false;

char szAppRomPaths[DIRS_MAX][MAX_PATH] = { { ROMS_DIR }};

extern void reset_frame_counter();
 
SYS_PROCESS_PARAM(1001, 0x80000);

void sysutil_exit_callback (uint64_t status, uint64_t param, void *userdata)
{
	(void) param;
	(void) userdata;

	switch (status)
	{
		case CELL_SYSUTIL_REQUEST_EXITGAME:  
			is_running = 0;
			exitGame = 1;
			break;
		case CELL_SYSUTIL_DRAWING_BEGIN:
		case CELL_SYSUTIL_DRAWING_END:
			break;
	}
}

int nPatchLang = 0;
int bEnforceDep = 0;
int nLoadMenuShowX;
int nLoadMenuShowY;

// Read in the config file for the whole application (NOTE: Not actually XML - and we like it that way)
int configAppLoadXml()
{
	config_file_t * currentconfig = config_file_new(SYS_CONFIG_FILE);

	// video
	init_setting_int("rotate-vertical", nVidRotationAdjust, 0);
	init_setting_int("vsync", bVidVSync, 1);
	init_setting_int("triple-buffer", bVidTripleBuffer, 1);

	// render
	init_setting_uint("linear", vidFilterLinear, 1);
	init_setting_uint("linear2", vidFilterLinear2, 1);
	init_setting_char("currentshader", selectedShader[0].filename, "stock.cg");
	init_setting_char("currentshader2", selectedShader[1].filename, "stock.cg");

	init_setting_int("xoffset", nXOffset, 0);
	init_setting_int("yoffset", nYOffset, 0);
	init_setting_uint("scalingfactor", bVidScalingFactor, 1);
	init_setting_bool("fbomode", bVidFBOEnabled, false);

	init_setting_uint("resolutionid", currentAvailableResolutionId, 0);

	init_setting_uint("firststartup", bBurnFirstStartup, 1);

	// video others
	init_setting_int("aspectx", nVidScrnAspectX, 4);
	init_setting_int("aspecty", nVidScrnAspectY, 3);
	init_setting_int("aspectmode", nVidScrnAspectMode, 0);

	init_setting_uint("viewport_x", m_viewport_x, 0);
	init_setting_uint("viewport_y", m_viewport_y, 0);
	init_setting_uint("viewport_width", m_viewport_width, 0);
	init_setting_uint("viewport_height", m_viewport_height, 0);
	init_setting_uint("overscan",m_overscan, 0);
	//init_setting_uint("overscan_amount",m_overscan_amount, 0);

	// gui
	init_setting_int("lastrom", nLastRom, 0);
	init_setting_int("lastfilter", nLastFilter, 0);
	init_setting_int("hidechildren", HideChildren, 0);
	init_setting_int("showthreefourplayeronly", ThreeOrFourPlayerOnly, 0);

	init_setting_int("nloadmenushowx", nLoadMenuShowX, 0);
	init_setting_int("language", nPatchLang, 0);
	init_setting_int("dependancy", bEnforceDep, 0)

	char tempStr[64] = "";

	for (int i = 0; i < 4; i++)
	{
		sprintf(tempStr, "controlsdefault%d", i);
		init_setting_char(tempStr, szPlayerDefaultIni[i], "");
	}

	init_setting_char("rompath", szAppRomPaths[0], ROMS_DIR);

	return 0;
}

// Write out the config file for the whole application
int configAppSaveXml()
{
	config_file_t * currentconfig = config_file_new(SYS_CONFIG_FILE);

	char tempStr[64] = "";

	// title
	sprintf(tempStr, "0x%06X", nBurnVer);
	config_set_string(currentconfig,  "version", tempStr);

	// video
	config_set_int(currentconfig, "rotate-vertical", nVidRotationAdjust);
	config_set_int(currentconfig, "vsync", bVidVSync);
	config_set_int(currentconfig, "triple-buffer", bVidTripleBuffer);

	// video render
	config_set_string(currentconfig, "currentshader", selectedShader[0].filename);
	config_set_string(currentconfig, "currentshader2", selectedShader[1].filename);

	config_set_uint(currentconfig, "linear", vidFilterLinear);
	config_set_uint(currentconfig, "linear2", vidFilterLinear2);

	config_set_int(currentconfig, "xoffset", nXOffset);
	config_set_int(currentconfig, "yoffset", nYOffset);
	config_set_int(currentconfig, "scalingfactor", bVidScalingFactor);
	config_set_bool(currentconfig, "fbomode", bVidFBOEnabled);

	config_set_uint(currentconfig, "resolutionid", currentAvailableResolutionId);
	config_set_uint(currentconfig, "firststartup", bBurnFirstStartup);

	// video others
	config_set_int(currentconfig, "aspectx", nVidScrnAspectX);
	config_set_int(currentconfig, "aspecty", nVidScrnAspectY);
	config_set_int(currentconfig, "aspectmode", nVidScrnAspectMode);
	config_set_uint(currentconfig, "viewport_x", m_viewport_x);
	config_set_uint(currentconfig, "viewport_y", m_viewport_y);
	config_set_uint(currentconfig, "viewport_width", m_viewport_width);
	config_set_uint(currentconfig, "viewport_height", m_viewport_height);
	config_set_uint(currentconfig, "overscan",m_overscan);
	//config_set_uint(currentconfig, "overscan_amount",m_overscan_amount);

	// gui
	config_set_int(currentconfig, "lastrom", nLastRom);
	config_set_int(currentconfig, "lastfilter", nLastFilter);

	config_set_int(currentconfig, "hidechildren", HideChildren);
	config_set_int(currentconfig, "showthreefourplayeronly", ThreeOrFourPlayerOnly);

	// gui load game dialog
	config_set_int(currentconfig, "nloadmenushowx", nLoadMenuShowX);

	// gui ips
	config_set_int(currentconfig, "language", nPatchLang);
	config_set_int(currentconfig, "dependancy", bEnforceDep);

	// preferences

	for (int i = 0; i < 4; i++)
	{
		sprintf(tempStr, "controlsdefault%d", i);
		config_set_string(currentconfig, tempStr, szPlayerDefaultIni[i]);
	}

	// paths

	//for (int i = 0; i < DIRS_MAX; i++)
	//addTextNode(rom_path, "path", szAppRomPaths[i]);
	config_set_string(currentconfig, "rompath", szAppRomPaths[0]);

	// save file
	config_file_write(currentconfig, SYS_CONFIG_FILE);

	return 0;
}

static int AppInit()
{
	if(!fileExists(SYS_CONFIG_FILE))
	{
		FILE * f;
		f = fopen(SYS_CONFIG_FILE, "w");
		fclose(f);
	}

	config_file_t * currentconfig = config_file_new(SYS_CONFIG_FILE);

	init_setting_uint("firststartup", bBurnFirstStartup, 1);

	if (bBurnFirstStartup)
	{
		printf("Creating initial config file...\n");
		configAppSaveXml();	// Create initial config file
	}

	configAppLoadXml();		// Load config for the application

	InitRomList();
	uint32_t roms_fetched = FetchRoms();

	BurnLibInit();			// Init the Burn library

	if(roms_fetched)
	{
		getAllRomsetInfo();		// Build the ROM information
	}

	BurnExtLoadOneRom = archiveLoadOneFile; 
	InitInputList();
	InitDipList();

	// get the last filter
	CurrentFilter = nLastFilter;

	BuildRomList();	
	audio_new();
	return 0;
}

extern unsigned int nPrevGame;
extern void StretchMenu();

// This will process all PC-side inputs and optionally update the emulated game side.
static void InputMake_Analog(void)
{
	struct GameInp* pgi = GameInp;
	uint32_t controller_binds_count = nGameInpCount;

	uint64_t new_state_p1 = cell_pad_input_poll_device(0);
	uint64_t pausemenu_condition = ArcadeJoystick ? (CTRL_SELECT(new_state_p1) && CTRL_START(new_state_p1)) : (CTRL_L2(new_state_p1) && CTRL_R2(new_state_p1) && CTRL_R1(new_state_p1));


	if (pausemenu_condition)
	{
		audio_stop();
		GameStatus = PAUSE;
		is_running = 0;
	}

	// Do one frames worth of keyboard input sliders
	// Begin of InputTick()

	for (uint32_t i = 0; i < controller_binds_count; i++, pgi++)
	{
		int nAdd = 0;
		if ((pgi->nInput &  GIT_GROUP_SLIDER) == 0) // not a slider
			continue;

		if (pgi->nInput == GIT_KEYSLIDER)
		{
			uint64_t reset = DoReset;
			int mask_a = keybinds[pgi->Input.Slider.SliderAxis[0]][0];
			int player_a = keybinds[pgi->Input.Slider.SliderAxis[0]][1];
			uint64_t state_a = cell_pad_input_poll_device(player_a);

			int mask_b = keybinds[pgi->Input.Slider.SliderAxis[1]][0];
			int player_b = keybinds[pgi->Input.Slider.SliderAxis[1]][1];
			uint64_t state_b = cell_pad_input_poll_device(player_b);

			uint32_t s = mask_a & state_a | (reset);
			uint32_t s2 = mask_b & state_b | (reset);
			// Get states of the two keys
			if (s)
				nAdd -= 0x100;
			if (s2)
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
	pgi = GameInp;
	for (uint32_t i = 0; i < controller_binds_count; i++, pgi++)
	{
		switch (pgi->nInput)
		{
			case GIT_CONSTANT: // Constant value
				pgi->Input.nVal = pgi->Input.Constant;
				*(pgi->Input.pVal) = pgi->Input.nVal;
				break;
			case GIT_SWITCH:
				{
					// Digital input
					uint64_t reset = DoReset;
					uint64_t mask = keybinds[pgi->Input.Switch][0];
					int player = keybinds[pgi->Input.Switch][1];
					uint64_t state = cell_pad_input_poll_device(player);

					uint64_t s = mask & state | (reset);

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
				}
				break;
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

	DoReset = 0;
}

//returns 1 if input has analog controls, else returns 0;
int InputPrepare(void)
{
	uint32_t ret = 0;
	struct GameInp* pgi = GameInp;
	for(uint32_t i = 0; i < nGameInpCount; i++, pgi++)
	{
		if(pgi->nType == BIT_ANALOG_REL)
		{
			ret = 1;
			break;
		}
	}

	keybinds[FBK_F3		][0] = 0;
	keybinds[FBK_F3		][0] = 0;
	keybinds[P1_COIN	][0] = CTRL_SELECT_MASK;
	keybinds[P1_COIN	][1] = 0;
	keybinds[P1_START	][0] = CTRL_START_MASK;
	keybinds[P1_START	][1] = 0;
	keybinds[P1_UP		][0] = CTRL_UP_MASK | CTRL_LSTICK_UP_MASK;
	keybinds[P1_UP		][1] = 0;
	keybinds[P1_DOWN	][0] = CTRL_DOWN_MASK | CTRL_LSTICK_DOWN_MASK;
	keybinds[P1_DOWN	][1] = 0;
	keybinds[P1_LEFT	][0] = CTRL_LEFT_MASK | CTRL_LSTICK_LEFT_MASK;
	keybinds[P1_LEFT	][1] = 0;
	keybinds[P1_RIGHT	][0] = CTRL_RIGHT_MASK | CTRL_LSTICK_RIGHT_MASK;
	keybinds[P1_RIGHT	][1] = 0;
	keybinds[P1_FIRE1	][0] = CTRL_CROSS_MASK;
	keybinds[P1_FIRE1	][1] = 0;
	keybinds[P1_FIRE2	][0] = CTRL_CIRCLE_MASK;
	keybinds[P1_FIRE2	][1] = 0;
	keybinds[P1_FIRE3	][0] = CTRL_SQUARE_MASK;
	keybinds[P1_FIRE3	][1] = 0;
	keybinds[P1_FIRE4	][0] = CTRL_TRIANGLE_MASK;
	keybinds[P1_FIRE4	][1] = 0;
	keybinds[P1_FIRE5	][0] = CTRL_L1_MASK;
	keybinds[P1_FIRE5	][1] = 0;
	keybinds[P1_FIRE6	][0] = CTRL_R1_MASK;
	keybinds[P1_FIRE6	][1] = 0;
	keybinds[0x88		][0] = CTRL_L2_MASK;
	keybinds[0x88		][1] = 0;
	keybinds[0x8A		][0] = CTRL_R2_MASK;
	keybinds[0x8A		][1] = 0;
	keybinds[0x3b		][0] = CTRL_L3_MASK;
	keybinds[0x3b		][1] = 0;
	keybinds[P1_SERVICE	][0] = CTRL_R3_MASK;
	keybinds[P1_SERVICE	][1] = 0;
	keybinds[0x21		][0] = CTRL_R2_MASK;
	keybinds[0x21		][1] = 0;

	keybinds[P2_COIN	][0] = CTRL_SELECT_MASK;
	keybinds[P2_COIN	][1] = 1;
	keybinds[P2_START	][0] = CTRL_START_MASK;
	keybinds[P2_START	][1] = 1;
	keybinds[P2_UP		][0] = CTRL_UP_MASK;
	keybinds[P2_UP		][1] = 1;
	keybinds[P2_DOWN	][0] = CTRL_DOWN_MASK;
	keybinds[P2_DOWN	][1] = 1;
	keybinds[P2_LEFT	][0] = CTRL_LEFT_MASK;
	keybinds[P2_LEFT	][1] = 1;
	keybinds[P2_RIGHT	][0] = CTRL_RIGHT_MASK;
	keybinds[P2_RIGHT	][1] = 1;
	keybinds[P2_FIRE1	][0] = CTRL_CROSS_MASK;
	keybinds[P2_FIRE1	][1] = 1;
	keybinds[P2_FIRE2	][0] = CTRL_CIRCLE_MASK;
	keybinds[P2_FIRE2	][1] = 1;
	keybinds[P2_FIRE3	][0] = CTRL_SQUARE_MASK;
	keybinds[P2_FIRE3	][1] = 1;
	keybinds[P2_FIRE4	][0] = CTRL_TRIANGLE_MASK;
	keybinds[P2_FIRE4	][1] = 1;
	keybinds[P2_FIRE5	][0] = CTRL_L1_MASK;
	keybinds[P2_FIRE5	][1] = 1;
	keybinds[P2_FIRE6	][0] = CTRL_R1_MASK;
	keybinds[P2_FIRE6	][1] = 1;
	keybinds[0x4088		][0] = CTRL_L2_MASK;
	keybinds[0x4088		][1] = 1;
	keybinds[0x408A		][0] = CTRL_R2_MASK;
	keybinds[0x408A		][1] = 1;
	keybinds[0x408b		][0] = CTRL_L3_MASK;
	keybinds[0x408b		][1] = 1;
	keybinds[0x408c		][0] = CTRL_R3_MASK;
	keybinds[0x408c		][1] = 1;

	keybinds[P3_COIN	][0] = CTRL_SELECT_MASK;
	keybinds[P3_COIN	][1] = 2;
	keybinds[P3_START	][0] = CTRL_START_MASK;
	keybinds[P3_START	][1] = 2;
	keybinds[P3_UP		][0] = CTRL_UP_MASK;
	keybinds[P3_UP		][1] = 2;
	keybinds[P3_DOWN	][0] = CTRL_DOWN_MASK;
	keybinds[P3_DOWN	][1] = 2;
	keybinds[P3_LEFT	][0] = CTRL_LEFT_MASK;
	keybinds[P3_LEFT	][1] = 2;
	keybinds[P3_RIGHT	][0] = CTRL_RIGHT_MASK;
	keybinds[P3_RIGHT	][1] = 2;
	keybinds[P3_FIRE1	][0] = CTRL_CROSS_MASK;
	keybinds[P3_FIRE1	][1] = 2;
	keybinds[P3_FIRE2	][0] = CTRL_CIRCLE_MASK;
	keybinds[P3_FIRE2	][1] = 2;
	keybinds[P3_FIRE3	][0] = CTRL_SQUARE_MASK;
	keybinds[P3_FIRE3	][1] = 2;
	keybinds[P3_FIRE4	][0] = CTRL_TRIANGLE_MASK;
	keybinds[P3_FIRE4	][1] = 2;
	keybinds[P3_FIRE5	][0] = CTRL_L1_MASK;
	keybinds[P3_FIRE5	][1] = 2;
	keybinds[P3_FIRE6	][0] = CTRL_R1_MASK;
	keybinds[P3_FIRE6	][1] = 2;
	keybinds[0x4188		][0] = CTRL_L2_MASK;
	keybinds[0x4188		][1] = 2;
	keybinds[0x418A		][0] = CTRL_R2_MASK;
	keybinds[0x418A		][1] = 2;
	keybinds[0x418b		][0] = CTRL_L3_MASK;
	keybinds[0x418b		][1] = 2;
	keybinds[0x418c		][0] = CTRL_R3_MASK;
	keybinds[0x418c		][1] = 2;

	keybinds[P4_COIN	][0] = CTRL_SELECT_MASK;
	keybinds[P4_COIN	][1] = 3;
	keybinds[P4_START	][0] = CTRL_START_MASK;
	keybinds[P4_START	][1] = 3;
	keybinds[P4_UP		][0] = CTRL_UP_MASK;
	keybinds[P4_UP		][1] = 3;
	keybinds[P4_DOWN	][0] = CTRL_DOWN_MASK;
	keybinds[P4_DOWN	][1] = 3;
	keybinds[P4_LEFT	][0] = CTRL_LEFT_MASK;
	keybinds[P4_LEFT	][1] = 3;
	keybinds[P4_RIGHT	][0] = CTRL_RIGHT_MASK;
	keybinds[P4_RIGHT	][1] = 3;
	keybinds[P4_FIRE1	][0] = CTRL_CROSS_MASK;
	keybinds[P4_FIRE1	][1] = 3;
	keybinds[P4_FIRE2	][0] = CTRL_CIRCLE_MASK;
	keybinds[P4_FIRE2	][1] = 3;
	keybinds[P4_FIRE3	][0] = CTRL_SQUARE_MASK;
	keybinds[P4_FIRE3	][1] = 3;
	keybinds[P4_FIRE4	][0] = CTRL_TRIANGLE_MASK;
	keybinds[P4_FIRE4	][1] = 3;
	keybinds[P4_FIRE5	][0] = CTRL_L1_MASK;
	keybinds[P4_FIRE5	][1] = 3;
	keybinds[P4_FIRE6	][0] = CTRL_R1_MASK;
	keybinds[P4_FIRE6	][1] = 3;
	keybinds[0x4288		][0] = CTRL_L2_MASK;
	keybinds[0x4288		][1] = 3;
	keybinds[0x428A		][0] = CTRL_R2_MASK;
	keybinds[0x428A		][1] = 3;
	keybinds[0x428b		][0] = CTRL_L3_MASK;
	keybinds[0x428b		][1] = 3;
	keybinds[0x428c		][0] = CTRL_R3_MASK;
	keybinds[0x428c		][1] = 3;
	return ret;
}

// This will process all PC-side inputs and optionally update the emulated game side.
static void InputMake(void)
{
	struct GameInp* pgi = GameInp;
	uint32_t controller_binds_count = nGameInpCount;

	uint64_t new_state_p1 = cell_pad_input_poll_device(0);
	uint64_t pausemenu_condition = ArcadeJoystick ? (CTRL_SELECT(new_state_p1) && CTRL_START(new_state_p1)) : (CTRL_L2(new_state_p1) && CTRL_R2(new_state_p1) && CTRL_R1(new_state_p1));



	if (pausemenu_condition)
	{
		audio_stop();
		GameStatus = PAUSE;
		is_running = 0;
	}

	for (uint32_t i = 0; i < controller_binds_count; i++, pgi++)
	{
		switch (pgi->nInput)
		{
			case GIT_CONSTANT: // Constant value
				pgi->Input.nVal = pgi->Input.Constant;
				*(pgi->Input.pVal) = pgi->Input.nVal;
				break;
			case GIT_SWITCH:
				// Digital input
				uint64_t reset = DoReset;
				uint64_t mask = keybinds[pgi->Input.Switch][0];
				int player = keybinds[pgi->Input.Switch][1];
				uint64_t state = cell_pad_input_poll_device(player);

				uint64_t s = mask & state | (reset);

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
	}

	DoReset = 0;
}

#include <PSGL/psglu.h>

void doStretch(void)
{
	extern GLuint gl_width, gl_height;
	static uint64_t old_state;
	uint64_t new_state = cell_pad_input_poll_device(0);
	uint64_t diff_state = old_state ^ new_state;

	if(CTRL_LSTICK_LEFT(new_state) || CTRL_LEFT(new_state))
		m_viewport_x -= 1;
	else if (CTRL_LSTICK_RIGHT(new_state) || CTRL_RIGHT(new_state))
		m_viewport_x += 1;

	if (CTRL_LSTICK_UP(new_state) || CTRL_UP(new_state))
	{
		m_viewport_y += 1;
	}
	else if (CTRL_LSTICK_DOWN(new_state) || CTRL_DOWN(new_state)) 
	{
		m_viewport_y -= 1;
	}

	if (CTRL_RSTICK_LEFT(new_state) || CTRL_L1(new_state))
	{
		m_viewport_width -= 1;
	}
	else if (CTRL_RSTICK_RIGHT(new_state) || CTRL_R1(new_state))
	{
		m_viewport_width += 1;
	}		
	if (CTRL_RSTICK_UP(new_state) || CTRL_L2(new_state))
	{
		m_viewport_height += 1;
	}
	else if (CTRL_RSTICK_DOWN(new_state) || CTRL_R2(new_state))
	{
		m_viewport_height -= 1;
	}

	if (CTRL_CIRCLE(new_state))
	{
		GameStatus = PAUSE;
		is_running = 0;
	}

	if (CTRL_TRIANGLE(new_state))
	{
		m_viewport_x = m_viewport_y = 0;
		m_viewport_width = gl_width;
		m_viewport_height = gl_height;
	}

	old_state = new_state;

#define WHITE		0xffffffffu
#define RED		0xff0000ffu
#define GREEN		0xff00ff00u
#define BLUE		0xffff0000u
#define YELLOW		0xff00ffffu
#define PURPLE		0xffff00ffu
#define CYAN		0xffffff00u
#define ORANGE		0xff0063ffu
#define SILVER		0xff8c848cu
#define LIGHTBLUE	0xFFFFE0E0U
#define LIGHTORANGE	0xFFE0EEFFu
#define x_position 0.3f
#define font_size 1.1f
#define ypos 0.19f
#define ypos_increment 0.04f
	cellDbgFontPrintf	(x_position,	ypos,	font_size+0.01f,	BLUE,	"Viewport X: #%d", m_viewport_x);
	cellDbgFontPrintf	(x_position,	ypos,	font_size,	GREEN,	"Viewport X: #%d", m_viewport_x);

	cellDbgFontPrintf	(x_position,	ypos+(ypos_increment*1),	font_size+0.01f,	BLUE,	"Viewport Y: #%d", m_viewport_y);
	cellDbgFontPrintf	(x_position,	ypos+(ypos_increment*1),	font_size,	GREEN,	"Viewport Y: #%d", m_viewport_y);

	cellDbgFontPrintf	(x_position,	ypos+(ypos_increment*2),	font_size+0.01f,	BLUE,	"Viewport Width: #%d", m_viewport_width);
	cellDbgFontPrintf	(x_position,	ypos+(ypos_increment*2),	font_size,	GREEN,	"Viewport Width: #%d", m_viewport_width);

	cellDbgFontPrintf	(x_position,	ypos+(ypos_increment*3),	font_size+0.01f,	BLUE,	"Viewport Height: #%d", m_viewport_height);
	cellDbgFontPrintf	(x_position,	ypos+(ypos_increment*3),	font_size,	GREEN,	"Viewport Height: #%d", m_viewport_height);

	cellDbgFontPrintf (0.09f,   0.40f,   font_size+0.01f,      BLUE,          "CONTROLS:");
	cellDbgFontPrintf (0.09f,   0.40f,   font_size,      LIGHTBLUE,           "CONTROLS:");

	cellDbgFontPrintf (0.09f,   0.46f,   font_size+0.01f,      BLUE,          "LEFT / LSTICK UP             - Decrement Viewport X");
	cellDbgFontPrintf (0.09f,   0.46f,   font_size,      LIGHTBLUE,           "LEFT / LSTICK UP             - Decrement Viewport X");

	cellDbgFontDraw();

	cellDbgFontPrintf (0.09f,   0.48f,   font_size+0.01f,      BLUE,          "RIGHT / LSTICK RIGHT         - Increment Viewport X");
	cellDbgFontPrintf (0.09f,   0.48f,   font_size,      LIGHTBLUE,           "RIGHT / LSTICK RIGHT         - Increment Viewport X");

	cellDbgFontPrintf (0.09f,   0.50f,   font_size+0.01f,      BLUE,          "UP / LSTICK UP               - Increment Viewport Y");
	cellDbgFontPrintf (0.09f,   0.50f,   font_size,      LIGHTBLUE,           "UP / LSTICK UP               - Increment Viewport Y");

	cellDbgFontDraw();

	cellDbgFontPrintf (0.09f,   0.52f,   font_size+0.01f,      BLUE,          "DOWN / LSTICK DOWN           - Decrement Viewport Y");
	cellDbgFontPrintf (0.09f,   0.52f,   font_size,      LIGHTBLUE,           "DOWN / LSTICK DOWN           - Decrement Viewport Y");

	cellDbgFontPrintf (0.09f,   0.54f,   font_size+0.01f,      BLUE,          "L1 / RSTICK LEFT             - Decrement Viewport Width");
	cellDbgFontPrintf (0.09f,   0.54f,   font_size,      LIGHTBLUE,           "L1 / RSTICK LEFT             - Decrement Viewport Width");

	cellDbgFontDraw();

	cellDbgFontPrintf (0.09f,   0.56f,   font_size+0.01f,      BLUE,          "R1/RSTICK RIGHT              - Increment Viewport Width");
	cellDbgFontPrintf (0.09f,   0.56f,   font_size,      LIGHTBLUE,           "R1/RSTICK RIGHT              - Increment Viewport Width");

	cellDbgFontPrintf (0.09f,   0.58f,   font_size+0.01f,      BLUE,          "L2 / RSTICK UP               - Increment Viewport Height");
	cellDbgFontPrintf (0.09f,   0.58f,   font_size,      LIGHTBLUE,           "L2 / RSTICK UP               - Increment Viewport Height");

	cellDbgFontDraw();

	cellDbgFontPrintf (0.09f,   0.60f,   font_size+0.01f,      BLUE,          "R2 / RSTICK DOWN             - Decrement Viewport Height");
	cellDbgFontPrintf (0.09f,   0.60f,   font_size,      LIGHTBLUE,           "R2 / RSTICK DOWN             - Decrement Viewport Height");

	cellDbgFontPrintf (0.09f,   0.66f,   font_size+0.01f,      BLUE,          "TRIANGLE                     - Reset To Defaults");
	cellDbgFontPrintf (0.09f,   0.66f,   font_size,      LIGHTBLUE,           "TRIANGLE                     - Reset To Defaults");

	cellDbgFontPrintf (0.09f,   0.68f,   font_size+0.01f,      BLUE,          "CIRCLE                       - Return to Ingame Menu");
	cellDbgFontPrintf (0.09f,   0.68f,   font_size,      LIGHTBLUE,           "CIRCLE                       - Return to Ingame Menu");

	cellDbgFontDraw();

	cellDbgFontPrintf (0.09f,   0.80f,   0.91f+0.01f,      BLUE,           "Allows you to resize the screen by moving around the two analog sticks.\nPress TRIANGLE to reset to default values, and CIRCLE to go back to the\nin-game menu.");
	cellDbgFontPrintf (0.09f,   0.80f,   0.91f,      LIGHTBLUE,           "Allows you to resize the screen by moving around the two analog sticks.\nPress TRIANGLE to reset to default values, and CIRCLE to go back to the\nin-game menu.");
	cellDbgFontDraw();
}

static void createNeedDir()
{
	cellFsMkdir("/dev_hdd0/game/FBAN00000/USRDIR/roms", CELL_FS_DEFAULT_CREATE_MODE_1);
	cellFsMkdir("/dev_hdd0/game/FBAN00000/USRDIR/config", CELL_FS_DEFAULT_CREATE_MODE_1);
	cellFsMkdir("/dev_hdd0/game/FBAN00000/USRDIR/config/presets", CELL_FS_DEFAULT_CREATE_MODE_1);
	cellFsMkdir("/dev_hdd0/game/FBAN00000/USRDIR/config/games", CELL_FS_DEFAULT_CREATE_MODE_1);
	cellFsMkdir("/dev_hdd0/game/FBAN00000/USRDIR/previews", CELL_FS_DEFAULT_CREATE_MODE_1);
	cellFsMkdir("/dev_hdd0/game/FBAN00000/USRDIR/cheats", CELL_FS_DEFAULT_CREATE_MODE_1);
	cellFsMkdir("/dev_hdd0/game/FBAN00000/USRDIR/screenshots", CELL_FS_DEFAULT_CREATE_MODE_1);
	cellFsMkdir("/dev_hdd0/game/FBAN00000/USRDIR/savestates", CELL_FS_DEFAULT_CREATE_MODE_1);
	cellFsMkdir("/dev_hdd0/game/FBAN00000/USRDIR/recordings", CELL_FS_DEFAULT_CREATE_MODE_1);
	cellFsMkdir("/dev_hdd0/game/FBAN00000/USRDIR/skins", CELL_FS_DEFAULT_CREATE_MODE_1);
	cellFsMkdir("/dev_hdd0/game/FBAN00000/USRDIR/ips", CELL_FS_DEFAULT_CREATE_MODE_1);
	cellFsMkdir("/dev_hdd0/game/FBAN00000/USRDIR/titles", CELL_FS_DEFAULT_CREATE_MODE_1);
	cellFsMkdir("/dev_hdd0/game/FBAN00000/USRDIR/flyers", CELL_FS_DEFAULT_CREATE_MODE_1);
	cellFsMkdir("/dev_hdd0/game/FBAN00000/USRDIR/scores", CELL_FS_DEFAULT_CREATE_MODE_1);
	cellFsMkdir("/dev_hdd0/game/FBAN00000/USRDIR/selects", CELL_FS_DEFAULT_CREATE_MODE_1);
	cellFsMkdir("/dev_hdd0/game/FBAN00000/USRDIR/gameovers", CELL_FS_DEFAULT_CREATE_MODE_1);
	cellFsMkdir("/dev_hdd0/game/FBAN00000/USRDIR/bosses", CELL_FS_DEFAULT_CREATE_MODE_1);
	cellFsMkdir("/dev_hdd0/game/FBAN00000/USRDIR/icons", CELL_FS_DEFAULT_CREATE_MODE_1);
}

int directLoadGame(const char * name)
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
		char * p = getBaseName(name);			// get game name
		unsigned int i = BurnDrvGetIndexByNameA(p);	// load game

		if (i < nBurnDrvCount)
		{
			RomOK = BurnerDrvInit(i, true);
			if (!RomOK)
			{
				if(!bVidOkay)			// bring up video
				{
					VidInit();		// Reinit the video plugin

					if (bVidOkay && (!bDrvOkay))
					{
						VidFrame();
					}
				}
				if(bAudOkay)
					audio_play();
			}
		}
	}

	is_running = 1;

	setWindowAspect(true);

	return RomOK;
}

// Main program entry point
int  main(int argc, char **argv)
{
	// One raw SPU used for PSGL - two raw SPUs and one SPU thread for the application
	sys_spu_initialize(4, 3);

	cellSysmoduleLoadModule(CELL_SYSMODULE_FS);
	cellSysmoduleLoadModule(CELL_SYSMODULE_IO);     
	cellSysmoduleLoadModule(CELL_SYSMODULE_PNGDEC);
	cellSysmoduleLoadModule(CELL_SYSMODULE_JPGDEC);
	cellSysmoduleLoadModule(CELL_SYSMODULE_RTC);

	cellSysutilRegisterCallback(0, sysutil_exit_callback, NULL);

#ifdef CELL_DEBUG_CONSOLE
	cellConsoleInit();
	cellConsoleNetworkInitialize();
	cellConsoleNetworkServerInit(-1);
	cellConsoleScreenShotPluginInit();
#endif

	// Make version string
	if (nBurnVer & 0xFF)
	{
		// private version (alpha)
		sprintf(szAppBurnVer, "%x.%x.%x.%02x", nBurnVer >> 20, (nBurnVer >> 16) & 0x0F, (nBurnVer >> 8) & 0xFF, nBurnVer & 0xFF);
		sprintf(szSVNVer, "%s", SVN_VERSION);
		sprintf(szSVNDate, "%s", SVN_DATE);
	}
	else
	{
		// public version
		sprintf(szAppBurnVer, "%x.%x.%x", nBurnVer >> 20, (nBurnVer >> 16) & 0x0F, (nBurnVer >> 8) & 0xFF);
		sprintf(szSVNVer, "%s", SVN_VERSION);
		sprintf(szSVNDate, "%s", SVN_DATE);
	}

	cell_pad_input_init();

	createNeedDir();		// Make sure there are roms and cfg subdirectories

	AppInit();

	psglInitGL();
	dbgFontInit();
	reset_frame_counter();

	LoadMenuTexture(TEXTURE_MENU, DEFAULT_MENU_BORDER_FILE);

#ifdef MULTIMAN_SUPPORT
	if(argc > 1)
	{
		const char * current_game = strrchr(strdup(argv[1]), '/');
		directLoadGame(strdup(current_game));
		GameStatus = EMULATING_INIT;	
		nPrevGame = 0;
	}
#endif

	do{
		switch (GameStatus)
		{
			case MENU:	
				psglClearUI();
				RomMenu();
				FrameMove();
#ifdef CELL_DEBUG_CONSOLE
				cellConsolePoll();
#endif
				psglRenderUI();
				break;
			case CONFIG_MENU:
				psglClearUI();
				ConfigMenu();
				ConfigFrameMove();
#ifdef CELL_DEBUG_CONSOLE
				cellConsolePoll();
#endif
				psglRenderUI();
				break;
			case PAUSE:
				psglClearUI();
				CalculateViewports();
				psglRenderAlpha();
				InGameMenu();
				InGameFrameMove();		
#ifdef CELL_DEBUG_CONSOLE
				cellConsolePoll();
#endif
				psglRenderUI();
				break;
			case INPUT_MENU:						
				psglClearUI();
				CalculateViewports();
				psglRenderAlpha();
				InputMenu();
				InputFrameMove();			
#ifdef CELL_DEBUG_CONSOLE
				cellConsolePoll();
#endif
				psglRenderUI();
				break;
			case DIP_MENU:						
				psglClearUI();
				CalculateViewports();
				psglRenderAlpha();
				DipMenu();
				DipFrameMove();			
				psglRenderUI();
				break;
			case SCREEN_RESIZE:
				psglClearUI();			
				CalculateViewports();
				psglRenderAlpha();
				doStretch();
				StretchMenu();
#ifdef CELL_DEBUG_CONSOLE
				cellConsolePoll();
#endif
				psglRenderUI();
				custom_aspect_ratio_mode = 1;
				nVidScrnAspectMode = ASPECT_RATIO_CUSTOM;
				break;
			case EMULATING_INIT:
				pBurnSoundOut = pAudNextSound;
				nBurnBpp = 2;
				GameStatus = EMULATING;
				controls = InputPrepare();
				break;
			case EMULATING:
				if(!is_running)
					GameStatus = PAUSE;
				CalculateViewports();
				uint32_t audio_samples = FRAMES_TO_SAMPLES(nAudSegLen);
				if(controls)
				{
					do{
						audio_check(audio_samples);
						nCurrentFrame++;
						VidFrame();
						psglRenderUI();
						InputMake_Analog();
#ifdef CELL_DEBUG_CONSOLE
						cellConsolePoll();
#endif
					}while(is_running);
				}
				else
				{
					do{
						audio_check(audio_samples);
						nCurrentFrame++;
						VidFrame();
						psglRenderUI();
						InputMake();
#ifdef CELL_DEBUG_CONSOLE
						cellConsolePoll();
#endif
					}while(is_running);
				}
				break;
		}
	}while(!exitGame);

	audio_stop();			// Stop sound if it was playing
	BurnerDrvExit();		// Make sure any game driver is exited
	mediaExit();			// Exit media

	BurnLibExit();			// Exit the Burn library

	freeAuditState();		// Free audit state 
	auditCleanup();			// Free audit info

	cell_pad_input_deinit();

	configAppSaveXml();		// Save config for the application

	cellSysmoduleUnloadModule(CELL_SYSMODULE_FS);
	cellSysmoduleUnloadModule(CELL_SYSMODULE_IO);     
	cellSysmoduleUnloadModule(CELL_SYSMODULE_RTC);
	cellSysmoduleUnloadModule(CELL_SYSMODULE_PNGDEC);
	cellSysmoduleUnloadModule(CELL_SYSMODULE_JPGDEC);
	cellSysutilUnregisterCallback(0);

	sys_process_exit(0);
}

int ProgressUpdateBurner(const char * pszText)
{ 	 
#ifdef CELL_DEBUG_MEMORY
	sys_memory_info_t mem_info;
	sys_memory_get_user_memory_size(&mem_info);
#endif

	psglClearUI();

	cellDbgFontPrintf(0.38f, 0.5f, 0.75f, 0xFFFFFFFF, "%s",  pszText);	 
	cellDbgFontDraw();
#ifdef CELL_DEBUG_MEMORY
	cellDbgFontPrintf(0.75f, 0.90f + 0.025f, 0.75f, 0xFFFF7F7f ,"%ld free memory",mem_info.available_user_memory );  
	cellDbgFontDraw();
	cellDbgFontPrintf(0.75f, 0.92f + 0.025f, 0.75f, 0xFFFF7F7f ,"%ld total memory",mem_info.total_user_memory );     
	cellDbgFontDraw();
#endif

	psglRenderUI();

	return 0;
}

void UpdateConsole(const char * text)
{  	
	 psglClearUI();
	 cellDbgFontPuts(0.38f, 0.5f, 0.75f, 0xFFFFFFFF, text);
	 cellDbgFontDraw();
	 psglRenderUI();
}

void UpdateConsoleXY(const char * text, float X, float Y)
{  	
	 psglClearUI();
	 cellDbgFontPuts(X, Y, 0.75f, 0xFFFFFFFF, text);
	 cellDbgFontDraw();
	 psglRenderUI();
}
