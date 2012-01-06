/******************************************************************************* 
 * emu-ps3.cpp - FBA Next Slim PS3
 *
 *  Created on: Jan 1, 2012
 ********************************************************************************/

#include <sysutil/sysutil_msgdialog.h>
#include "cellframework2/audio/stream.h"

#include <cell/dbgfont.h>
#include <cell/sysmodule.h>
#include <sys/spu_initialize.h>
#include <sys/stat.h>
#include <sysutil/sysutil_sysparam.h>
#include <string.h>
#include <stdio.h>
#include <sys/process.h>
#include "menu.h"
#include "ps3video.h"
#include "conf/config_file.h"
#include "conf/settings.h"
#include "emu-ps3.h"

// Driver Init module
#include "burner.h"
#include "burnint.h"
#include "tracklst.h"
#ifndef NO_IPS
//#include "patch.h"
#endif

#define SAMPLERATE_11KHZ 11025
#define SAMPLERATE_22KHZ 22050
#define SAMPLERATE_44KHZ 44100
#define SAMPLERATE_48KHZ 48010

SYS_PROCESS_PARAM(1001, 0x10000);

int		bDrvOkay = 0; // 1 if the Driver has been inited okay, and it's okay to use the BurnDrv functions
static bool	bSaveRAM = false;

/* PS3 frontend variables */
static uint8_t * g_fba_frame;
static int16_t * g_audio_buf;
static uint32_t audio_samples;

extern struct BurnDriver* pDriver[];
int nAudSegLen;

struct cell_audio_params params;
char contentInfoPath[MAX_PATH_LENGTH];
char usrDirPath[MAX_PATH_LENGTH];
char DEFAULT_PRESET_FILE[MAX_PATH_LENGTH];
char DEFAULT_BORDER_FILE[MAX_PATH_LENGTH];
char DEFAULT_MENU_BORDER_FILE_[MAX_PATH_LENGTH];
char GAME_AWARE_SHADER_DIR_PATH[MAX_PATH_LENGTH];
char INPUT_PRESETS_DIR_PATH[MAX_PATH_LENGTH];
char PRESETS_DIR_PATH[MAX_PATH_LENGTH];
char BORDERS_DIR_PATH[MAX_PATH_LENGTH];
char SHADERS_DIR_PATH[MAX_PATH_LENGTH];
char DEFAULT_SHADER_FILE[MAX_PATH_LENGTH];
char DEFAULT_MENU_SHADER_FILE_[MAX_PATH_LENGTH];
char SYS_CONFIG_FILE_[MAX_PATH_LENGTH];
char CONFIGURATION_FILE[MAX_PATH_LENGTH];
char BIOS_FILE[MAX_PATH_LENGTH];
#ifdef MULTIMAN_SUPPORT
char MULTIMAN_GAME_TO_BOOT[MAX_PATH_LENGTH];
#endif

cell_audio_handle_t audio_handle;
const struct cell_audio_driver *audio_driver = &cell_audio_audioport;
oskutil_params oskutil_handle;
uint32_t control_binds[MAX_PADS][BTN_DEF_MAX];

static unsigned drv_flags;
struct SSettings Settings;

int mode_switch = MODE_MENU;				/* mode the main loop is in*/

static uint32_t is_running;				/* is the ROM currently running in the emulator?*/
static bool is_ingame_menu_running;			/* is the ingame menu currently running?*/
static bool return_to_MM = false;			/* launch multiMAN on exit if ROM is passed*/
static uint32_t emulator_initialized = 0;		/* is the emulator loaded?*/
bool need_load_rom = true;				/* need to load the current rom*/
char * current_rom = NULL;				/* filename of the current rom being emulated*/
bool dialog_is_running;
char special_action_msg[256];				
uint32_t special_action_msg_expired;			/* time at which the message no longer needs to be overlaid onscreen*/
uint64_t ingame_menu_item = 0;

char		szAppBurnVer[16], szSVNVer[16], szSVNDate[30];
int		ArcadeJoystick, exitGame, custom_aspect_ratio_mode;
static uint64_t keybinds[1024][2] = {0}; 

static int width, height;

char szAppRomPaths[DIRS_MAX][MAX_PATH] = { { ROMS_DIR }};

static void emulator_load_current_save_state_slot(void)
{
	char szChoice[MAX_PATH], msg[512];

	snprintf(szChoice, sizeof(szChoice), "%s/%s.%d.fs", Settings.PS3PathSaveStates, BurnDrvGetTextA(DRV_NAME), Settings.CurrentSaveStateSlot);

	int ret = BurnStateLoad(szChoice, 1, &DrvInitCallback);

	if(ret)
		snprintf(msg, sizeof(msg), "Can't load from save state slot #%d", Settings.CurrentSaveStateSlot);
	else
		snprintf(msg, sizeof(msg), "Loaded save state slot #%d", Settings.CurrentSaveStateSlot);

	set_text_message(msg, 60);
}

static void emulator_save_current_save_state_slot(void)
{
	char msg[512], szChoice[MAX_PATH];

	snprintf(szChoice, sizeof(szChoice), "%s/%s.%d.fs", Settings.PS3PathSaveStates, BurnDrvGetTextA(DRV_NAME), Settings.CurrentSaveStateSlot);

	BurnStateSave(szChoice, 1);
	snprintf(msg, sizeof(msg), "Saved to save state slot #%d", Settings.CurrentSaveStateSlot);

	set_text_message(msg, 60);
}

int nLoadMenuShowX;

int ProgressUpdateBurner(char const * pszText)
{
	sys_memory_info_t mem_info;
	sys_memory_get_user_memory_size(&mem_info);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	ps3graphics_draw_menu(1920, 1080);

	cellDbgFontPrintf(0.38f, 0.5f, 0.75f, 0xFFFFFFFF, "%s",  pszText);	 
	cellDbgFontDraw();
	cellDbgFontPrintf(0.75f, 0.90f + 0.025f, 0.75f, 0xFFFF7F7f ,"%ld free memory",mem_info.available_user_memory );  
	cellDbgFontDraw();
	cellDbgFontPrintf(0.75f, 0.92f + 0.025f, 0.75f, 0xFFFF7F7f ,"%ld total memory",mem_info.total_user_memory );     
	cellDbgFontDraw();
	_jsPlatformSwapBuffers(psgl_device);
	cellSysutilCheckCallback();
}

void UpdateConsole(const char * text)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	ps3graphics_draw_menu(1920, 1080);

	cellDbgFontPuts(0.38f, 0.5f, 0.75f, 0xFFFFFFFF, text);
	cellDbgFontDraw();
	_jsPlatformSwapBuffers(psgl_device);
	cellSysutilCheckCallback();
}

void UpdateConsoleXY(const char * text, float X, float Y)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	ps3graphics_draw_menu(1920, 1080);

	cellDbgFontPuts(X, Y, 0.75f, 0xFFFFFFFF, text);
	cellDbgFontDraw();
	_jsPlatformSwapBuffers(psgl_device);
	cellSysutilCheckCallback();
}
void InpDIPSWResetDIPs (void) { }

static int DoLibInit() // Do Init of Burn library driver
{
	int nRet = 0;

	BArchiveOpen(false);

	// If there is a problem with the romset, report it
	switch (BArchiveStatus())
	{
		case BARC_STATUS_BADDATA:
			FBAPopupDisplay(PUF_TYPE_WARNING);
			BArchiveClose();
			return 1;
			break;
		case BARC_STATUS_ERROR:
			FBAPopupDisplay(PUF_TYPE_ERROR);
			break;
	}

	nRet = BurnDrvInit();
	BArchiveClose();

	if (nRet)
		return 3;
	else
		return 0;
}

// Catch calls to BurnLoadRom() once the emulation has started;
// Intialise the zip module before forwarding the call, and exit cleanly.
static int __cdecl DrvLoadRom(unsigned char* Dest, int* pnWrote, int i)
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

static unsigned int HighCol15(int r, int g, int b, int  /* i */)
{
	unsigned int t = 0;
	t |= (r << 7) & 0x7c00;
	t |= (g << 2) & 0x03e0;
	t |= (b >> 3) & 0x001f;
	return t;
}

void simpleReinitScrn(void)
{
	uint32_t width_tmp, height_tmp;

	ps3graphics_set_orientation(Settings.Orientation);

	BurnDrvGetVisibleSize(&width, &height);

	if (ps3graphics_calculate_aspect_ratio_before_game_load())
		ps3graphics_set_aspect_ratio(Settings.PS3KeepAspect, width, height, 1);

	VidRecalcPal();

	drv_flags = BurnDrvGetFlags();
	if (drv_flags & (BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED))
	{
		nBurnPitch = height * sizeof(uint16_t);
		width_tmp = height;
		height_tmp = width;
		width = width_tmp;
		height = height_tmp;
	}
	else
		nBurnPitch = width * sizeof(uint16_t);

	pBurnDraw = (uint8_t*)g_fba_frame;
	nBurnBpp = 2;
	BurnHighCol = HighCol15;
	nBurnLayer = 0xff;
}

//#define NEED_MEDIA_REINIT
// no need to reinit media when init a driver, modified by regret
int BurnerDrvInit(int nDrvNum, bool bRestore)
{
	BurnerDrvExit(); // Make sure exited

	nBurnDrvSelect = nDrvNum; // Set the driver number

	// Define nMaxPlayers early; GameInpInit() needs it (normally defined in DoLibInit()).
	nMaxPlayers = BurnDrvGetMaxPlayers();

	GameInpInit();					// Init game input

	if (ConfigGameLoad(true))
		loadDefaultInput();			// load default input mapping

	GameInpDefault();


	// set functions
	BurnReinitScrn = simpleReinitScrn;


#ifndef NO_IPS
	//bDoPatch = true; // play with ips
	//loadActivePatches();
	//BurnApplyPatch = applyPatches;
#endif

	int nStatus = DoLibInit();		// Init the Burn library's driver
	if (nStatus)
	{
		if (nStatus & 2)
			BurnDrvExit();			// Exit the driver

		return 1;
	}

	// ==> for changing sound track
	//parseTracklist();
	// <==

	BurnExtLoadRom = DrvLoadRom;

	bDrvOkay = 1;					// Okay to use all BurnDrv functions


	bSaveRAM = false;
	if (bRestore)
	{
		bSaveRAM = true;
	}

	// Reset the speed throttling code, so we don't 'jump' after the load
	return 0;
}

int DrvInitCallback()
{
	return BurnerDrvInit(nBurnDrvSelect, false);
}

int BurnerDrvExit()
{
	if (bDrvOkay)
	{
		//jukeDestroy();

		if (nBurnDrvSelect < nBurnDrvCount)
		{
			//MemCardEject(); // Eject memory card if present

			if (bSaveRAM)
			{
				bSaveRAM = false;
			}

			ConfigGameSave(true); // save game config

			GameInpExit(); // Exit game input
			BurnDrvExit(); // Exit the driver
		}
	}

	BurnExtLoadRom = NULL;

	bDrvOkay = 0; // Stop using the BurnDrv functions

	nBurnDrvSelect = ~0U; // no driver selected
	nBurnLayer = 0xFF; // show all layers

	return 0;
}

/* emulator-specific global variables */

/* PS3 frontend - save state/emulator SRAM related functions */

void set_text_message(const char * message, uint32_t speed)
{
	snprintf(special_action_msg, sizeof(special_action_msg), message);
	special_action_msg_expired = ps3graphics_set_text_message_speed(speed);
}

static void emulator_decrement_current_save_state_slot(void)
{
	char msg[512];

	if (Settings.CurrentSaveStateSlot != MIN_SAVE_STATE_SLOT)
		Settings.CurrentSaveStateSlot--;
	snprintf(msg, sizeof(msg), "Save state slot changed to: #%d", Settings.CurrentSaveStateSlot);

	set_text_message(msg, 60);
}

static void emulator_increment_current_save_state_slot(void)
{
	char msg[512];

	Settings.CurrentSaveStateSlot++;
	snprintf(msg, sizeof(msg), "Save state slot changed to: #%d", Settings.CurrentSaveStateSlot);
	
	set_text_message(msg, 60);
}

static void emulator_toggle_throttle(bool enable)
{
	char msg[512];

	ps3graphics_set_vsync(enable);
	if(enable)
		snprintf(msg, sizeof(msg), "Throttle mode: ON");
	else
		snprintf(msg, sizeof(msg), "Throttle mode: OFF");

	set_text_message(msg, 60);
}

#define init_setting_uint(charstring, setting, defaultvalue) \
	if(!(config_get_uint(currentconfig, charstring, &setting))) \
setting = defaultvalue; 

#define init_setting_int(charstring, setting, defaultvalue) \
	if(!(config_get_int(currentconfig, charstring, &setting))) \
setting = defaultvalue; 

#define init_setting_bool(charstring, setting, defaultvalue) \
	if(!(config_get_bool(currentconfig, charstring, &setting))) \
setting = defaultvalue; 

#define init_setting_bool(charstring, setting, defaultvalue) \
	if(!(config_get_bool(currentconfig, charstring, &setting))) \
setting =	defaultvalue;

#define init_setting_char(charstring, setting, defaultvalue) \
	if(!(config_get_char_array(currentconfig, charstring, setting, sizeof(setting)))) \
strncpy(setting,defaultvalue, sizeof(setting));

/* PS3 frontend - controls related functions */

static bool file_exists(const char * filename)
{
	CellFsStat sb;
	if(cellFsStat(filename,&sb) == CELL_FS_SUCCEEDED)
		return true;
	else
		return false;
}

void emulator_implementation_set_shader_preset(const char * fname)
{
	config_file_t * currentconfig = config_file_new(fname);

	init_setting_uint("ScaleFactor", Settings.ScaleFactor, Settings.ScaleFactor);
	init_setting_uint("Smooth", Settings.PS3Smooth, Settings.PS3Smooth);
	init_setting_uint("Smooth2", Settings.PS3Smooth2, Settings.PS3Smooth2);
	init_setting_uint("ScaleEnabled", Settings.ScaleEnabled, Settings.ScaleEnabled);
	init_setting_uint("Orientation", Settings.Orientation, Settings.Orientation);
	init_setting_char("PS3CurrentShader", Settings.PS3CurrentShader, DEFAULT_SHADER_FILE);
	init_setting_char("PS3CurrentShader2", Settings.PS3CurrentShader2, DEFAULT_SHADER_FILE);
	init_setting_char("Border", Settings.PS3CurrentBorder, DEFAULT_BORDER_FILE);

	strncpy(Settings.ShaderPresetPath, fname, sizeof(Settings.ShaderPresetPath));
	init_setting_char("ShaderPresetTitle", Settings.ShaderPresetTitle, "None");
	init_setting_uint("KeepAspect", Settings.PS3KeepAspect, Settings.PS3KeepAspect);
	init_setting_uint("OverscanEnabled", Settings.PS3OverscanEnabled, Settings.PS3OverscanEnabled);
	init_setting_int("OverscanAmount", Settings.PS3OverscanAmount, Settings.PS3OverscanAmount);
	init_setting_uint("ViewportX", Settings.ViewportX, Settings.ViewportX);
	init_setting_uint("ViewportY", Settings.ViewportY, Settings.ViewportY);
	init_setting_uint("ViewportWidth", Settings.ViewportWidth, Settings.ViewportWidth);
	init_setting_uint("ViewportHeight", Settings.ViewportHeight, Settings.ViewportHeight);
	ps3graphics_load_fragment_shader(Settings.PS3CurrentShader, 0);
	ps3graphics_load_fragment_shader(Settings.PS3CurrentShader2, 1);
	ps3graphics_set_fbo_scale(Settings.ScaleEnabled,Settings.ScaleFactor);
	ps3graphics_set_aspect_ratio(Settings.PS3KeepAspect, SCREEN_RENDER_TEXTURE_WIDTH, SCREEN_RENDER_TEXTURE_HEIGHT, 1);
	ps3graphics_set_overscan(Settings.PS3OverscanEnabled, (float)Settings.PS3OverscanAmount/100, 1);
	ps3graphics_set_smooth(Settings.PS3Smooth, 0);
	ps3graphics_set_smooth(Settings.PS3Smooth2, 1);
}

#if 0
void emulator_implementation_set_gameaware(const char * fname)
{
	ps3graphics_init_state_uniforms(fname);
	strcpy(Settings.PS3CurrentShader, ps3graphics_get_fragment_shader_path(0));
	strcpy(Settings.PS3CurrentShader2, ps3graphics_get_fragment_shader_path(1));
}
#endif

static void map_ps3_standard_controls(const char * config_file)
{
	char filetitle_tmp[512];
	char string_tmp[256];
	config_file_t * currentconfig = config_file_new(config_file);
	for(uint32_t i = 0; i < MAX_PADS; i++)
	{
		for(uint32_t j = 0; j < BTN_DEF_MAX; j++)
		{
			snprintf(string_tmp, sizeof(string_tmp), "PS3Player%d::%d", i, j);
			config_set_uint(currentconfig, string_tmp,control_binds[i][j]);
		}
	}
	config_set_string(currentconfig, "InputPresetTitle", filetitle_tmp);
	config_file_write(currentconfig, config_file);
}

static void get_ps3_standard_controls(const char * config_file)
{
	config_file_t * currentconfig = config_file_new(config_file);
	char string_tmp[256];

	for(uint32_t i = 0; i < MAX_PADS; i++)
	{
		for(uint32_t j = 0; j < BTN_DEF_MAX; j++)
		{
			snprintf(string_tmp, sizeof(string_tmp), "PS3Player%d::%d", i, j);
			init_setting_uint(string_tmp, control_binds[i][j], default_control_binds[j]);
		}
	}

	init_setting_char("InputPresetTitle", Settings.PS3CurrentInputPresetTitle, "Default");
}

uint32_t default_control_binds[] = {
	BTN_UP,				/* CTRL_UP_DEF*/
	BTN_DOWN,			/* CTRL_DOWN_DEF*/
	BTN_LEFT,			/* CTRL_LEFT_DEF*/
	BTN_RIGHT,			/* CTRL_RIGHT_DEF*/
	BTN_FIRE2,			/* CTRL_CIRCLE_DEF*/
	BTN_FIRE1,			/* CTRL_CROSS_DEF*/
	BTN_FIRE5,			/* CTRL_TRIANGLE_DEF*/
	BTN_FIRE4,			/* CTRL_SQUARE_DEF*/
	BTN_COIN,			/* CTRL_SELECT_DEF*/
	BTN_START,			/* CTRL_START_DEF*/
	BTN_FIRE3,			/* CTRL_L1_DEF*/
	BTN_FIRE6,			/* CTRL_R1_DEF*/
	BTN_NONE,			/* CTRL_L2_DEF*/
	BTN_NONE,			/* CTRL_R2_DEF*/
	BTN_NONE,			/* CTRL_L3_DEF*/
	BTN_INGAME_MENU,		/* CTRL_R3_DEF*/
	BTN_NONE,			/* CTRL_L2_L3_DEF*/
	BTN_NONE,			/* CTRL_L2_R3_DEF*/
	BTN_NONE,			/* CTRL_L2_RSTICK_RIGHT_DEF*/
	BTN_NONE,			/* CTRL_L2_RSTICK_LEFT_DEF*/
	BTN_NONE,			/* CTRL_L2_RSTICK_UP_DEF*/
	BTN_NONE,			/* CTRL_L2_RSTICK_DOWN_DEF*/
	BTN_INCREMENTSAVE,		/* CTRL_R2_RSTICK_RIGHT_DEF*/
	BTN_DECREMENTSAVE,		/* CTRL_R2_RSTICK_LEFT_DEF*/
	BTN_QUICKLOAD,			/* CTRL_R2_RSTICK_UP_DEF*/
	BTN_QUICKSAVE,			/* CTRL_R2_RSTICK_DOWN_DEF*/
	BTN_NONE,			/* CTRL_R2_R3_DEF*/
	BTN_EXITTOMENU,			/* CTRL_R3_L3_DEF*/
	BTN_NONE,			/* CTRL_RSTICK_UP_DEF*/
	BTN_FASTFORWARD,		/* CTRL_RSTICK_DOWN_DEF*/
	BTN_NONE,			/* CTRL_RSTICK_LEFT_DEF*/
	BTN_NONE			/* CTRL_RSTICK_RIGHT_DEF*/
};

static void map_ps3_button_array(void)
{
	for(int i = 0; i < MAX_PADS; i++)
		for(uint32_t j = 0; j < BTN_DEF_MAX; j++)
			Input_MapButton(control_binds[i][j],false,default_control_binds[j]);
}

void emulator_set_controls(const char * config_file, int mapping_enum, const char * title)
{
	switch(mapping_enum)
	{
		case WRITE_CONTROLS:
		{
			map_ps3_standard_controls(config_file);
			break;
		}
		case READ_CONTROLS:
		{
			get_ps3_standard_controls(config_file);
			break;
		}
		case SET_ALL_CONTROLS_TO_DEFAULT:
		{
			map_ps3_button_array();
			break;
		}
	}
}

static void ingame_menu_enable (int enable)
{
	is_running = 0;
	is_ingame_menu_running = enable;
}

static void special_actions(int specialbuttonmap)
{
	if(specialbuttonmap & BTN_SOFTRESET)
	{
	}

	if(specialbuttonmap & BTN_HARDRESET)
	{
	}

	if(frame_count < special_action_msg_expired)
	{
	}
	else
	{
		if(specialbuttonmap & BTN_EXITTOMENU)
		{
			Emulator_StopROMRunning();
			mode_switch = MODE_MENU;
			set_text_message("", 15);
		}
	}

	if(specialbuttonmap & BTN_QUICKSAVE)
	{
		emulator_save_current_save_state_slot();
	}

	if(specialbuttonmap & BTN_QUICKLOAD)
	{
		emulator_load_current_save_state_slot();
	}

	if(frame_count < special_action_msg_expired)
	{
	}
	else
	{
		if(specialbuttonmap & BTN_INGAME_MENU)
		{
			ingame_menu_enable(1);
		}
	}

	if(specialbuttonmap & BTN_DECREMENTSAVE)
	{
		emulator_decrement_current_save_state_slot();
	}

	if(specialbuttonmap & BTN_INCREMENTSAVE)
	{
		emulator_increment_current_save_state_slot();
	}

	if(specialbuttonmap & BTN_FASTFORWARD)
	{
		if(frame_count < special_action_msg_expired)
		{
		}
		else
		{
			Settings.Throttled = !Settings.Throttled;
			emulator_toggle_throttle(Settings.Throttled);
		}
	}
}

#define special_button_mappings(controllerno, specialbuttonmap, condition) \
	if(condition) \
	{ \
		if(specialbuttonmap <= BTN_LASTGAMEBUTTON) \
			input.pad[controllerno] |=  specialbuttonmap; \
		else \
			special_action_to_execute = specialbuttonmap; \
	}

//returns 1 if input has analog controls, else returns 0;
int InputPrepare(void)
{
	uint32_t ret, i;
	struct GameInp* pgi;
	
	ret = 0;
	pgi = GameInp;

	for( i = 0; i < nGameInpCount; i++, pgi++)
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
		ingame_menu_enable(true);
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
				uint64_t mask = keybinds[pgi->Input.Switch][0];
				int player = keybinds[pgi->Input.Switch][1];
				uint64_t state = cell_pad_input_poll_device(player);

				uint64_t s = mask & state;

				if (pgi->nType & BIT_GROUP_ANALOG)
				{
					// Set analog controls to full
					if (s)
						pgi->Input.nVal = 0xFFFF;
					else
						pgi->Input.nVal = 0x0001;
					*((int *)pgi->Input.pShortVal) = pgi->Input.nVal;
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
}

static void InputMake_Analog(void)
{
	struct GameInp* pgi = GameInp;
	uint32_t controller_binds_count = nGameInpCount;

	uint64_t new_state_p1 = cell_pad_input_poll_device(0);
	uint64_t pausemenu_condition = ArcadeJoystick ? (CTRL_SELECT(new_state_p1) && CTRL_START(new_state_p1)) : (CTRL_L2(new_state_p1) && CTRL_R2(new_state_p1) && CTRL_R1(new_state_p1));

	if (pausemenu_condition)
	{
		ingame_menu_enable(true);
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
			int mask_a = keybinds[pgi->Input.Slider.SliderAxis[0]][0];
			int player_a = keybinds[pgi->Input.Slider.SliderAxis[0]][1];
			uint64_t state_a = cell_pad_input_poll_device(player_a);

			int mask_b = keybinds[pgi->Input.Slider.SliderAxis[1]][0];
			int player_b = keybinds[pgi->Input.Slider.SliderAxis[1]][1];
			uint64_t state_b = cell_pad_input_poll_device(player_b);

			uint32_t s = mask_a & state_a;
			uint32_t s2 = mask_b & state_b;
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
					uint64_t mask = keybinds[pgi->Input.Switch][0];
					int player = keybinds[pgi->Input.Switch][1];
					uint64_t state = cell_pad_input_poll_device(player);

					uint64_t s = mask & state;

					if (pgi->nType & BIT_GROUP_ANALOG)
					{
						// Set analog controls to full
						if (s)
							pgi->Input.nVal = 0xFFFF;
						else
							pgi->Input.nVal = 0x0001;
						*((int *)pgi->Input.pShortVal) = pgi->Input.nVal;
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
					*((int *)pgi->Input.pShortVal) = pgi->Input.nVal;
					break;
				}
		}
	}
}

float Emulator_GetFontSize(void)
{
	return Settings.PS3FontSize/100.0;
}

bool Emulator_IsROMLoaded(void)
{
	return current_rom != NULL && need_load_rom == false;
}

void emulator_save_settings(uint64_t filetosave)
{
	config_file_t * currentconfig = config_file_new(SYS_CONFIG_FILE_);
	char filepath[MAX_PATH_LENGTH];

	switch(filetosave)
	{
		case CONFIG_FILE:
			strncpy(filepath, SYS_CONFIG_FILE_, sizeof(filepath));

			config_set_uint(currentconfig, "PS3General::ControlScheme",Settings.ControlScheme);
			config_set_uint(currentconfig, "PS3General::CurrentSaveStateSlot",Settings.CurrentSaveStateSlot);
			config_set_uint(currentconfig, "PS3General::KeepAspect",Settings.PS3KeepAspect);
			config_set_uint(currentconfig, "PS3General::ApplyShaderPresetOnStartup", Settings.ApplyShaderPresetOnStartup);
			config_set_uint(currentconfig, "PS3General::ViewportX", ps3graphics_get_viewport_x());
			config_set_uint(currentconfig, "PS3General::ViewportY", ps3graphics_get_viewport_y());
			config_set_uint(currentconfig, "PS3General::ViewportWidth", ps3graphics_get_viewport_width());
			config_set_uint(currentconfig, "PS3General::ViewportHeight", ps3graphics_get_viewport_height());
			config_set_uint(currentconfig, "PS3General::Smooth", Settings.PS3Smooth);
			config_set_uint(currentconfig, "PS3General::Smooth2", Settings.PS3Smooth2);
			config_set_uint(currentconfig, "PS3General::ScaleFactor", Settings.ScaleFactor);
			config_set_uint(currentconfig, "PS3General::OverscanEnabled", Settings.PS3OverscanEnabled);
			config_set_uint(currentconfig, "PS3General::OverscanAmount",Settings.PS3OverscanAmount);
			config_set_uint(currentconfig, "PS3General::PS3FontSize",Settings.PS3FontSize);
			config_set_uint(currentconfig, "PS3General::Throttled",Settings.Throttled);
			config_set_uint(currentconfig, "PS3General::PS3PALTemporalMode60Hz",Settings.PS3PALTemporalMode60Hz);
			config_set_uint(currentconfig, "PS3General::PS3TripleBuffering",Settings.TripleBuffering);
			config_set_uint(currentconfig, "PS3General::ScreenshotsEnabled",Settings.ScreenshotsEnabled);
			config_set_uint(currentconfig, "Sound::SoundMode",Settings.SoundMode);
			config_set_uint(currentconfig, "PS3General::PS3CurrentResolution",ps3graphics_get_current_resolution());
			config_set_string(currentconfig, "PS3General::ShaderPresetPath", Settings.ShaderPresetPath);
			config_set_string(currentconfig, "PS3General::ShaderPresetTitle", Settings.ShaderPresetTitle);
			config_set_string(currentconfig, "PS3General::PS3CurrentShader",ps3graphics_get_fragment_shader_path(0));
			config_set_string(currentconfig, "PS3General::PS3CurrentShader2", ps3graphics_get_fragment_shader_path(1));
			config_set_string(currentconfig, "PS3General::Border", Settings.PS3CurrentBorder);
			config_set_string(currentconfig, "PS3General::GameAwareShaderPath", Settings.GameAwareShaderPath);
			config_set_string(currentconfig, "PS3Paths::PathSaveStates", Settings.PS3PathSaveStates);
			config_set_string(currentconfig, "PS3Paths::PathSRAM", Settings.PS3PathSRAM);
			config_set_string(currentconfig, "PS3Paths::PathROMDirectory", Settings.PS3PathROMDirectory);
			config_set_string(currentconfig, "RSound::RSoundServerIPAddress", Settings.RSoundServerIPAddress);
			config_set_uint(currentconfig, "PS3General::ScaleEnabled", Settings.ScaleEnabled);
			config_set_uint(currentconfig, "PS3General::Orientation", Settings.Orientation);

			/* emulator-specific*/
			config_set_uint(currentconfig, "Sound::SoundMode",Settings.SoundMode);
			config_set_uint(currentconfig, "Sound::FMVolume",Settings.FMVolume);
			config_set_uint(currentconfig, "Sound::PSGVolume",Settings.PSGVolume);
			config_set_uint(currentconfig, "GenesisPlus::SixButtonPad",Settings.SixButtonPad);
			config_set_uint(currentconfig, "GenesisPlus::ExtraCart", Settings.ExtraCart);
			config_set_string(currentconfig, "GenesisPlus::BIOS",Settings.BIOS);
			config_set_string(currentconfig, "GenesisPlus::ActionReplayROMPath", Settings.ActionReplayROMPath);
			config_set_string(currentconfig, "GenesisPlus::GameGenieROMPath", Settings.GameGenieROMPath);
			config_set_string(currentconfig, "GenesisPlus::SKROMPath", Settings.SKROMPath);
			config_set_string(currentconfig, "GenesisPlus::SKupmemROMPath", Settings.SKUpmemROMPath);
			config_set_string(currentconfig, "rompath", szAppRomPaths[0]);

			config_file_write(currentconfig, filepath);
			emulator_set_controls(filepath, WRITE_CONTROLS, "Default");
			break;
		case SHADER_PRESET_FILE:
			{
				bool filename_entered = false;
				char filename_tmp[256], filetitle_tmp[256];
				oskutil_write_initial_message(&oskutil_handle, L"example");
				oskutil_write_message(&oskutil_handle, L"Enter filename for preset (with no file extension)");
				oskutil_start(&oskutil_handle);

				while(OSK_IS_RUNNING(oskutil_handle))
				{
					/* OSK Util gets updated */
					glClear(GL_COLOR_BUFFER_BIT);
					ps3graphics_draw_menu(1920, 1080);
					_jsPlatformSwapBuffers(psgl_device);
					cell_console_poll();
					cellSysutilCheckCallback();
				}

				if(oskutil_handle.text_can_be_fetched)
				{
					strncpy(filename_tmp, OUTPUT_TEXT_STRING(oskutil_handle), sizeof(filename_tmp));
					snprintf(filepath, sizeof(filepath), "%s/%s.conf", PRESETS_DIR_PATH, filename_tmp);
					filename_entered = true;
				}

				if(filename_entered)
				{

					oskutil_write_initial_message(&oskutil_handle, L"Example file title");
					oskutil_write_message(&oskutil_handle, L"Enter title for preset");
					oskutil_start(&oskutil_handle);

					while(OSK_IS_RUNNING(oskutil_handle))
					{
						/* OSK Util gets updated */
						glClear(GL_COLOR_BUFFER_BIT);
						ps3graphics_draw_menu(1920, 1080);
						_jsPlatformSwapBuffers(psgl_device);
						cell_console_poll();
						cellSysutilCheckCallback();
					}

					if(oskutil_handle.text_can_be_fetched)
						snprintf(filetitle_tmp, sizeof(filetitle_tmp), "%s", OUTPUT_TEXT_STRING(oskutil_handle));
					else
						snprintf(filetitle_tmp, sizeof(filetitle_tmp), "%s", filename_tmp);

					if(!file_exists(filepath))
					{
						FILE * f = fopen(filepath, "w");
						fclose(f);
					}

					currentconfig = config_file_new(filepath);

					config_set_string(currentconfig, "PS3CurrentShader", Settings.PS3CurrentShader);
					config_set_string(currentconfig, "PS3CurrentShader2", Settings.PS3CurrentShader2);
					config_set_string(currentconfig, "Border", Settings.PS3CurrentBorder);
					config_set_uint(currentconfig, "Smooth", Settings.PS3Smooth);
					config_set_uint(currentconfig, "Smooth2", Settings.PS3Smooth2);
					config_set_string(currentconfig, "ShaderPresetTitle", filetitle_tmp);
					config_set_uint(currentconfig, "ViewportX", ps3graphics_get_viewport_x());
					config_set_uint(currentconfig, "ViewportY", ps3graphics_get_viewport_y());
					config_set_uint(currentconfig, "ViewportWidth", ps3graphics_get_viewport_width());
					config_set_uint(currentconfig, "ViewportHeight", ps3graphics_get_viewport_height());
					config_set_uint(currentconfig, "ScaleFactor", Settings.ScaleFactor);
					config_set_uint(currentconfig, "ScaleEnabled", Settings.ScaleEnabled);
					config_set_uint(currentconfig, "Orientation", Settings.Orientation);
					config_set_uint(currentconfig, "OverscanEnabled", Settings.PS3OverscanEnabled);
					config_set_uint(currentconfig, "OverscanAmount", Settings.PS3OverscanAmount);
					config_file_write(currentconfig, filepath);
				}
			}
			break;
		case INPUT_PRESET_FILE:
			{
				bool filename_entered = false;
				char filename_tmp[256];
				oskutil_write_initial_message(&oskutil_handle, L"example");
				oskutil_write_message(&oskutil_handle, L"Enter filename for preset (with no file extension)");
				oskutil_start(&oskutil_handle);

				while(OSK_IS_RUNNING(oskutil_handle))
				{
					/* OSK Util gets updated */
					glClear(GL_COLOR_BUFFER_BIT);
					ps3graphics_draw_menu(1920, 1080);
					_jsPlatformSwapBuffers(psgl_device);
					cell_console_poll();
					cellSysutilCheckCallback();
				}

				if(oskutil_handle.text_can_be_fetched)
				{
					strncpy(filename_tmp, OUTPUT_TEXT_STRING(oskutil_handle), sizeof(filename_tmp));
					snprintf(filepath, sizeof(filepath), "%s/%s.conf", INPUT_PRESETS_DIR_PATH, filename_tmp);
					filename_entered = true;
				}

				if(filename_entered)
				{
					char filetitle_tmp[512];
					oskutil_write_initial_message(&oskutil_handle, L"Example file title");
					oskutil_write_message(&oskutil_handle, L"Enter title for preset");
					oskutil_start(&oskutil_handle);

					while(OSK_IS_RUNNING(oskutil_handle))
					{
						/* OSK Util gets updated */
						glClear(GL_COLOR_BUFFER_BIT);
						ps3graphics_draw_menu(1920, 1080);
						_jsPlatformSwapBuffers(psgl_device);
						cell_console_poll();
						cellSysutilCheckCallback();
					}

					if(oskutil_handle.text_can_be_fetched)
						snprintf(filetitle_tmp, sizeof(filetitle_tmp), "%s", OUTPUT_TEXT_STRING(oskutil_handle));
					else
						snprintf(filetitle_tmp, sizeof(filetitle_tmp), "%s", "Custom");


					if(!file_exists(filepath))
					{
						FILE * f = fopen(filepath, "w");
						fclose(f);
					}

					emulator_set_controls(filepath, WRITE_CONTROLS, filetitle_tmp);
				}
			}
			break;
	}
}

static void emulator_shutdown(void)
{
	emulator_save_settings(CONFIG_FILE);
#ifdef MULTIMAN_SUPPORT
	if(return_to_MM)
	{
		if(audio_handle)
		{
			audio_driver->free(audio_handle);
			audio_handle = NULL; 
		}
		cellSysmoduleUnloadModule(CELL_SYSMODULE_AVCONF_EXT);
		sys_spu_initialize(6, 0);
		char multiMAN[512];
		snprintf(multiMAN, sizeof(multiMAN), "%s", "/dev_hdd0/game/BLES80608/USRDIR/RELOAD.SELF");
		sys_game_process_exitspawn2((char*) multiMAN, NULL, NULL, NULL, 0, 2048, SYS_PROCESS_PRIMARY_STACK_SIZE_64K);		
		sys_process_exit(0);
	}
	else
#endif
		sys_process_exit(0);
}


static void emulator_init_settings(void)
{
	bool config_file_newly_created = false;
	memset((&Settings), 0, (sizeof(Settings)));

	if(!file_exists(SYS_CONFIG_FILE_))
	{
		FILE * f;
		f =fopen(SYS_CONFIG_FILE_, "w");
		fclose(f);
		config_file_newly_created = true;
	}

	config_file_t * currentconfig = config_file_new(SYS_CONFIG_FILE_);

	init_setting_uint("PS3General::ApplyShaderPresetOnStartup", Settings.ApplyShaderPresetOnStartup, 0);
	init_setting_uint("PS3General::KeepAspect", Settings.PS3KeepAspect, ASPECT_RATIO_4_3);
	init_setting_uint("PS3General::Smooth", Settings.PS3Smooth, 1);
	init_setting_uint("PS3General::Smooth2", Settings.PS3Smooth2, 1);
	init_setting_char("PS3General::PS3CurrentShader", Settings.PS3CurrentShader, DEFAULT_SHADER_FILE);
	init_setting_char("PS3General::PS3CurrentShader2", Settings.PS3CurrentShader2, DEFAULT_SHADER_FILE);
	init_setting_char("PS3General::Border", Settings.PS3CurrentBorder, DEFAULT_BORDER_FILE);
	init_setting_uint("PS3General::PS3TripleBuffering",Settings.TripleBuffering, 1);
	init_setting_char("PS3General::ShaderPresetPath", Settings.ShaderPresetPath, "");
	init_setting_char("PS3General::ShaderPresetTitle", Settings.ShaderPresetTitle, "None");
	init_setting_uint("PS3General::ScaleFactor", Settings.ScaleFactor, 2);
	init_setting_uint("PS3General::ViewportX", Settings.ViewportX, 0);
	init_setting_uint("PS3General::ViewportY", Settings.ViewportY, 0);
	init_setting_uint("PS3General::ViewportWidth", Settings.ViewportWidth, 0);
	init_setting_uint("PS3General::ViewportHeight", Settings.ViewportHeight, 0);
	init_setting_uint("PS3General::ScaleEnabled", Settings.ScaleEnabled, 1);
	init_setting_uint("PS3General::Orientation", Settings.Orientation, 0);
	init_setting_uint("PS3General::PS3CurrentResolution", Settings.PS3CurrentResolution, NULL);
	init_setting_uint("PS3General::OverscanEnabled", Settings.PS3OverscanEnabled, 0);
	init_setting_int("PS3General::OverscanAmount", Settings.PS3OverscanAmount, 0);
	init_setting_uint("PS3General::PS3PALTemporalMode60Hz", Settings.PS3PALTemporalMode60Hz, 0);
	init_setting_uint("Sound::SoundMode", Settings.SoundMode, SOUND_MODE_NORMAL);
	init_setting_char("RSound::RSoundServerIPAddress",  Settings.RSoundServerIPAddress, "0.0.0.0");
	init_setting_uint("PS3General::Throttled", Settings.Throttled, 1);
	init_setting_uint("PS3General::PS3FontSize", Settings.PS3FontSize, 100);
	init_setting_char("PS3Paths::PathSaveStates", Settings.PS3PathSaveStates, usrDirPath);
	init_setting_char("PS3Paths::PathSRAM", Settings.PS3PathSRAM, usrDirPath);
	init_setting_char("PS3Paths::PathROMDirectory", Settings.PS3PathROMDirectory, "/");
	init_setting_uint("PS3General::ControlScheme", Settings.ControlScheme, CONTROL_SCHEME_DEFAULT);
	init_setting_uint("PS3General::CurrentSaveStateSlot",  Settings.CurrentSaveStateSlot, 0);
	init_setting_uint("PS3General::ScreenshotsEnabled", Settings.ScreenshotsEnabled, 0);
	char tmp_str[256];
	if (config_get_char_array(currentconfig,"PS3General::GameAwareShaderPath", tmp_str, sizeof(tmp_str)))
		config_get_char_array(currentconfig, "PS3General::GameAwareShaderPath", Settings.GameAwareShaderPath, sizeof(Settings.GameAwareShaderPath));

	/* emulator-specific settings */
	init_setting_char("rompath", szAppRomPaths[0], ROMS_DIR);

	if(config_file_newly_created)
		emulator_set_controls(SYS_CONFIG_FILE_, SET_ALL_CONTROLS_TO_DEFAULT, "Default");
	else
		emulator_set_controls(SYS_CONFIG_FILE_, READ_CONTROLS, "Default");
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
		}
	}

	is_running = 1;

	return RomOK;
}

void Emulator_RequestLoadROM(const char* rom)
{
	if (current_rom == NULL || strcmp(rom, current_rom) != 0)
	{
		if (current_rom != NULL)
			free(current_rom);

		current_rom = strdup(rom);
		directLoadGame(current_rom);
	}

	need_load_rom = false;
}

void Emulator_StopROMRunning(void)
{
	is_running = 0;
}

void Emulator_StartROMRunning(uint32_t set_is_running)
{
	if(set_is_running)
		is_running = 1;
	mode_switch = MODE_EMULATION;
}

static void sysutil_exit_callback (uint64_t status, uint64_t param, void *userdata)
{
	(void) param;
	(void) userdata;

	switch (status)
	{
		case CELL_SYSUTIL_REQUEST_EXITGAME:
			mode_switch = MODE_EXIT;
			menu_is_running = 0;
			is_ingame_menu_running = 0;
			Emulator_StopROMRunning();
			return_to_MM = false;
			break;
		case CELL_SYSUTIL_DRAWING_BEGIN:
		case CELL_SYSUTIL_DRAWING_END:
		case CELL_SYSUTIL_OSKDIALOG_LOADED:
			break;
		case CELL_SYSUTIL_OSKDIALOG_FINISHED:
			oskutil_close(&oskutil_handle);
			oskutil_finished(&oskutil_handle);
			break;
		case CELL_SYSUTIL_OSKDIALOG_UNLOADED:
			oskutil_unload(&oskutil_handle);
			break;
		default:
			break;
	}
}

int emulator_audio_init(int samplerate)
{
	int nAudAllocSegLen;

	switch(samplerate)
	{
		case SAMPLERATE_11KHZ:
			nAudSegLen = 184;
			nAudAllocSegLen = 736;
			break;
		case SAMPLERATE_22KHZ:
			nAudSegLen = 368;
			nAudAllocSegLen = 1472;
			break;
		case SAMPLERATE_44KHZ:
			nAudSegLen = 735;
			nAudAllocSegLen = 2940;
			break;
		case 48000:
		case SAMPLERATE_48KHZ: 
			nAudSegLen = 801;
			nAudAllocSegLen = 12800;
			samplerate = 48000;
			break;

	}

	audio_samples = nAudSegLen << 1;

	nBurnSoundRate = samplerate;
	nBurnSoundLen = nAudSegLen;

	cell_audio_params params;
	memset(&params, 0, sizeof(params));
	params.channels = 2;
	params.samplerate = samplerate;
	params.buffer_size = 8192;
	params.userdata = NULL;
	params.device = NULL;
	audio_handle = audio_driver->init(&params);

	/* The next sound block to put in the stream*/
	g_audio_buf = (int16_t*)realloc(g_audio_buf, nAudAllocSegLen);

	if (g_audio_buf == NULL)
	{
		free(g_audio_buf);
		return 1;
	}

	if (g_audio_buf)
		memset(g_audio_buf, 0, nAudAllocSegLen);
	
	return 0;
}


int VidRecalcPal()
{
	return BurnRecalcPal();
}

static void emulator_start(void)
{
	uint32_t controls, current_selected_game_index;

	controls = InputPrepare();

	simpleReinitScrn();

	if(Settings.Throttled)
		audio_driver->unpause(audio_handle);
	
	//memset(g_fba_frame, 0, 1024*1024);
	//g_fba_frame += nBurnPitch;

	pBurnSoundOut = g_audio_buf;
	nBurnSoundRate = 48000;

	current_selected_game_index = nBurnDrvSelect;

	printf("nBurnDrvSelect: %d\n", current_selected_game_index);

	do{

		if(Settings.Throttled)
			audio_driver->write(audio_handle, pBurnSoundOut, audio_samples);

		nCurrentFrame++;
		pDriver[current_selected_game_index]->Frame();
		if(!controls)
			InputMake();
		else
			InputMake_Analog();
		ps3graphics_draw(width, height, pBurnDraw, drv_flags);
		if(frame_count < special_action_msg_expired)
		{
			cellDbgFontPrintf (0.09f, 0.90f, 1.51f, BLUE,	special_action_msg);
			cellDbgFontPrintf (0.09f, 0.90f, 1.50f, WHITE,	special_action_msg);
			cellDbgFontDraw();
		}
		else
			special_action_msg_expired = 0;
		_jsPlatformSwapBuffers(psgl_device);
		cell_console_poll();
		cellSysutilCheckCallback();
	}while(is_running);
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


void emulator_toggle_sound(uint64_t soundmode)
{
	memset(&params, 0, sizeof(params));
	params.channels=2;
	params.samplerate=48000;
	params.buffer_size=8192;
	params.sample_cb = NULL;
	params.userdata = NULL;

	switch(soundmode)
	{
		case SOUND_MODE_RSOUND:
			params.device = Settings.RSoundServerIPAddress;
			params.headset = 0;
			break;
		case SOUND_MODE_HEADSET:
			params.device = NULL;
			params.headset = 1;
			break;
		case SOUND_MODE_NORMAL: 
			params.device = NULL;
			params.headset = 0;
			break;
	}

	if(audio_handle)
	{
		audio_driver->free(audio_handle);
		audio_handle = NULL; 
	}
	if(soundmode == SOUND_MODE_RSOUND)
	{
		audio_driver = &cell_audio_rsound;

		if(!audio_handle || !(strlen(Settings.RSoundServerIPAddress) > 0))
		{
			audio_driver = &cell_audio_audioport;

			Settings.SoundMode = SOUND_MODE_NORMAL;
			dialog_is_running = true;
			cellMsgDialogOpen2(CELL_MSGDIALOG_TYPE_SE_TYPE_ERROR|CELL_MSGDIALOG_TYPE_BG_VISIBLE|\
					CELL_MSGDIALOG_TYPE_BUTTON_TYPE_NONE|CELL_MSGDIALOG_TYPE_DISABLE_CANCEL_OFF|\
					CELL_MSGDIALOG_TYPE_DEFAULT_CURSOR_OK, "Couldn't connect to RSound server at specified IP address. Falling back to\
					regular audio.",cb_dialog_ok,NULL,NULL);

			do{
				glClear(GL_COLOR_BUFFER_BIT);
				_jsPlatformSwapBuffers(psgl_device);
				cell_console_poll();
				cellSysutilCheckCallback();
			}while(dialog_is_running && is_running);
		}
	}
	else
	{
		audio_driver = &cell_audio_audioport;
	}
}


/*FIXME: Turn GREEN into WHITE and RED into LIGHTBLUE once the overlay is in*/
#define ingame_menu_reset_entry_colors(ingame_menu_item) \
{ \
	for(int i = 0; i < MENU_ITEM_LAST; i++) \
	menuitem_colors[i] = GREEN; \
	menuitem_colors[ingame_menu_item] = RED; \
}


static void ingame_menu(void)
{
	static uint64_t old_state, blocking;
	uint64_t state, button_was_pressed, button_was_held, stuck_in_loop;
	uint32_t menuitem_colors[MENU_ITEM_LAST];
	char comment[256], msg_temp[256];

	old_state = 0;
	blocking = 0;

	do
	{
		state = cell_pad_input_poll_device(0);

		stuck_in_loop = 1;
		button_was_pressed = old_state & (old_state ^ state);
		button_was_held = old_state & state;

		ps3graphics_draw(width, height, pBurnDraw, drv_flags);

		if(frame_count < special_action_msg_expired && blocking)
		{
		}
		else
		{
			if(CTRL_CIRCLE(state))
			{
				is_running = 1;
				ingame_menu_item = 0;
				is_ingame_menu_running = 0;
				Emulator_StartROMRunning(0);
			}
			switch(ingame_menu_item)
			{
				case MENU_ITEM_LOAD_STATE:
					if(CTRL_CROSS(button_was_pressed))
					{
						emulator_load_current_save_state_slot();
						is_running = 1;
						ingame_menu_item = 0;
						is_ingame_menu_running = 0;
						Emulator_StartROMRunning(0);
					}

					if(CTRL_LEFT(button_was_pressed) || CTRL_LSTICK_LEFT(button_was_pressed))
					{
						emulator_decrement_current_save_state_slot();
						blocking = 0;
					}

					if(CTRL_RIGHT(button_was_pressed) || CTRL_LSTICK_RIGHT(button_was_pressed))
					{
						emulator_increment_current_save_state_slot();
						blocking = 0;
					}

					ingame_menu_reset_entry_colors(ingame_menu_item);
					strcpy(comment,"Press LEFT or RIGHT to change the current save state slot.\nPress CROSS to load the state from the currently selected save state slot.");
					break;
				case MENU_ITEM_SAVE_STATE:
					if(CTRL_CROSS(button_was_pressed))
					{
						emulator_save_current_save_state_slot();
						is_running = 1;
						ingame_menu_item = 0;
						is_ingame_menu_running = 0;
						Emulator_StartROMRunning(0);
					}

					if(CTRL_LEFT(button_was_pressed) || CTRL_LSTICK_LEFT(button_was_pressed))
					{
						emulator_decrement_current_save_state_slot();
						blocking = 0;
					}

					if(CTRL_RIGHT(button_was_pressed) || CTRL_LSTICK_RIGHT(button_was_pressed))
					{
						emulator_increment_current_save_state_slot();
						blocking = 0;
					}

					ingame_menu_reset_entry_colors (ingame_menu_item);
					strcpy(comment,"Press LEFT or RIGHT to change the current save state slot.\nPress CROSS to save the state to the currently selected save state slot.");
					break;
				case MENU_ITEM_KEEP_ASPECT_RATIO:
					if(CTRL_LEFT(button_was_pressed) || CTRL_LSTICK_LEFT(button_was_pressed))
					{
						if(Settings.PS3KeepAspect > 0)
						{
							Settings.PS3KeepAspect--;
							ps3graphics_set_aspect_ratio(Settings.PS3KeepAspect, width, height, 1);
						}
					}
					if(CTRL_RIGHT(button_was_pressed)  || CTRL_LSTICK_RIGHT(button_was_pressed))
					{
						if(Settings.PS3KeepAspect < LAST_ASPECT_RATIO)
						{
							Settings.PS3KeepAspect++;
							ps3graphics_set_aspect_ratio(Settings.PS3KeepAspect, width, height, 1);
						}
					}
					if(CTRL_START(button_was_pressed))
					{
						Settings.PS3KeepAspect = ASPECT_RATIO_4_3;
						ps3graphics_set_aspect_ratio(Settings.PS3KeepAspect, width, height, 1);
					}
					ingame_menu_reset_entry_colors (ingame_menu_item);
					strcpy(comment,"Press LEFT or RIGHT to change the [Aspect Ratio].\nPress START to reset back to default values.");
					break;
				case MENU_ITEM_OVERSCAN_AMOUNT:
					if(CTRL_LEFT(button_was_pressed)  ||  CTRL_LSTICK_LEFT(button_was_pressed) || CTRL_CROSS(button_was_pressed) || CTRL_LSTICK_LEFT(button_was_held))
					{
						Settings.PS3OverscanAmount--;
						Settings.PS3OverscanEnabled = 1;

						if(Settings.PS3OverscanAmount == 0)
							Settings.PS3OverscanEnabled = 0;
						ps3graphics_set_overscan(Settings.PS3OverscanEnabled, (float)Settings.PS3OverscanAmount/100, 1);
					}

					if(CTRL_RIGHT(button_was_pressed) || CTRL_LSTICK_RIGHT(button_was_pressed) || CTRL_CROSS(button_was_pressed) || CTRL_LSTICK_RIGHT(button_was_held))
					{
						Settings.PS3OverscanAmount++;
						Settings.PS3OverscanEnabled = 1;

						if(Settings.PS3OverscanAmount == 0)
							Settings.PS3OverscanEnabled = 0;
						ps3graphics_set_overscan(Settings.PS3OverscanEnabled, (float)Settings.PS3OverscanAmount/100, 1);
					}

					if(CTRL_START(button_was_pressed))
					{
						Settings.PS3OverscanAmount = 0;
						Settings.PS3OverscanEnabled = 0;
						ps3graphics_set_overscan(Settings.PS3OverscanEnabled, (float)Settings.PS3OverscanAmount/100, 1);
					}
					ingame_menu_reset_entry_colors (ingame_menu_item);
					strcpy(comment,"Press LEFT or RIGHT to change the [Overscan] settings.\nPress START to reset back to default values.");
					break;
				case MENU_ITEM_ORIENTATION:
					if(CTRL_LEFT(button_was_pressed)  ||  CTRL_LSTICK_LEFT(button_was_pressed) || CTRL_CROSS(button_was_pressed) || CTRL_LSTICK_LEFT(button_was_held))
					{
						if(Settings.Orientation > 0)
						{
							Settings.Orientation--;
							ps3graphics_set_orientation(Settings.Orientation);
						}
					}

					if(CTRL_RIGHT(button_was_pressed) || CTRL_LSTICK_RIGHT(button_was_pressed) || CTRL_CROSS(button_was_pressed) || CTRL_LSTICK_RIGHT(button_was_held))
					{
						if(Settings.Orientation != MAX_ORIENTATION)
						{
							Settings.Orientation++;
							ps3graphics_set_orientation(Settings.Orientation);
						}
					}

					if(CTRL_START(button_was_pressed))
					{
						Settings.Orientation = NORMAL;
						ps3graphics_set_orientation(Settings.Orientation);
					}
					ingame_menu_reset_entry_colors (ingame_menu_item);
					strcpy(comment, "Press LEFT or RIGHT to change the [Orientation] settings.\nPress START to reset back to default values.");
					break;
				case MENU_ITEM_FRAME_ADVANCE:
					if(CTRL_CROSS(state) || CTRL_R2(state) || CTRL_L2(state))
					{
						is_running = 0;
						ingame_menu_item = MENU_ITEM_FRAME_ADVANCE;
						is_ingame_menu_running = 0;
						Emulator_StartROMRunning(0);
					}
					ingame_menu_reset_entry_colors (ingame_menu_item);
					strcpy(comment,"Press 'CROSS', 'L2' or 'R2' button to step one frame.\nNOTE: Pressing the button rapidly will advance the frame more slowly\nand prevent buttons from being input.");
					break;
				case MENU_ITEM_RESIZE_MODE:
					ingame_menu_reset_entry_colors (ingame_menu_item);
					if(CTRL_CROSS(state))
					{
						ps3graphics_set_aspect_ratio(ASPECT_RATIO_CUSTOM, width, height, 1);
						do
						{
							ps3graphics_draw(width, height, pBurnDraw, drv_flags);
							state = cell_pad_input_poll_device(0);
							ps3graphics_resize_aspect_mode_input_loop(state);
							if(CTRL_CIRCLE(state))
							{
								set_text_message("", 7);
								blocking = 1;
								stuck_in_loop = 0;
							}

							_jsPlatformSwapBuffers(psgl_device);
							cellSysutilCheckCallback();
							old_state = state;
						}while(stuck_in_loop && is_ingame_menu_running);
					}
					strcpy(comment, "Allows you to resize the screen by moving around the two analog sticks.\nPress TRIANGLE to reset to default values, and CIRCLE to go back to the\nin-game menu.");
					break;
				case MENU_ITEM_SCREENSHOT_MODE:
					if(CTRL_CROSS(state))
					{
						while(stuck_in_loop && is_ingame_menu_running)
						{
							state = cell_pad_input_poll_device(0);
							if(CTRL_CIRCLE(state))
							{
								set_text_message("", 7);
								blocking = 1;
								stuck_in_loop = 0;
							}

							ps3graphics_draw(width, height, pBurnDraw, drv_flags);
							_jsPlatformSwapBuffers(psgl_device);
							cellSysutilCheckCallback();
							old_state = state;
						}
					}

					ingame_menu_reset_entry_colors (ingame_menu_item);
					strcpy(comment, "Allows you to take a screenshot without any text clutter.\nPress CIRCLE to go back to the in-game menu while in 'Screenshot Mode'.");
					break;
				case MENU_ITEM_RETURN_TO_GAME:
					if(CTRL_CROSS(button_was_pressed))
					{
						is_running = 1;
						ingame_menu_item = 0;
						is_ingame_menu_running = 0;
						Emulator_StartROMRunning(0);
					} 
					ingame_menu_reset_entry_colors (ingame_menu_item);
					strcpy(comment,"Press 'CROSS' to return back to the game.");
					break;
				case MENU_ITEM_RESET:
					if(CTRL_CROSS(button_was_pressed))
					{
						is_running = 1;
						ingame_menu_item = 0;
						is_ingame_menu_running = 0;
						Emulator_StartROMRunning(0);
					} 
					ingame_menu_reset_entry_colors (ingame_menu_item);
					strcpy(comment,"Press 'CROSS' to reset the game.");
					break;
				case MENU_ITEM_RETURN_TO_MENU:
					if(CTRL_CROSS(button_was_pressed))
					{
						is_running = 1;
						ingame_menu_item = 0;
						is_ingame_menu_running = 0;
						mode_switch = MODE_MENU;
					}

					ingame_menu_reset_entry_colors (ingame_menu_item);
					strcpy(comment,"Press 'CROSS' to return to the ROM Browser menu.");
					break;
#ifdef MULTIMAN_SUPPORT
				case MENU_ITEM_RETURN_TO_MULTIMAN:
					if(CTRL_CROSS(button_was_pressed))
					{
						is_running = 0;
						is_ingame_menu_running = 0;
						mode_switch = MODE_EXIT;
					}

					ingame_menu_reset_entry_colors (ingame_menu_item);

					strcpy(comment,"Press 'CROSS' to quit the emulator and return to multiMAN.");
					break;
#endif
				case MENU_ITEM_RETURN_TO_XMB:
					if(CTRL_CROSS(button_was_pressed))
					{
						is_running = 0;
						is_ingame_menu_running = 0;
						return_to_MM = false;
						mode_switch = MODE_EXIT;
					}

					ingame_menu_reset_entry_colors (ingame_menu_item);

					strcpy(comment,"Press 'CROSS' to quit the emulator and return to the XMB.");
					break;
			}

			if(CTRL_UP(button_was_pressed) || CTRL_LSTICK_UP(button_was_pressed))
			{
				if(ingame_menu_item != 0)
					ingame_menu_item--;
			}

			if(CTRL_DOWN(button_was_pressed) || CTRL_LSTICK_DOWN(button_was_pressed))
			{
				if(ingame_menu_item < MENU_ITEM_LAST)
					ingame_menu_item++;
			}
		}

		float x_position = 0.3f;
		float font_size = 1.1f;
		float ypos = 0.19f;
		float ypos_increment = 0.04f;
		cellDbgFontPrintf	(x_position,	0.10f,	1.4f+0.01f,	BLUE,               "Quick Menu");
		cellDbgFontPrintf	(x_position,	0.10f,	1.4f,	WHITE,               "Quick Menu");

		cellDbgFontPrintf	(x_position,	ypos,	font_size+0.01f,	BLUE,	"Load State #%d", Settings.CurrentSaveStateSlot);
		cellDbgFontPrintf	(x_position,	ypos,	font_size,	menuitem_colors[MENU_ITEM_LOAD_STATE],	"Load State #%d", Settings.CurrentSaveStateSlot);

		cellDbgFontDraw();

		cellDbgFontPrintf	(x_position,	ypos+(ypos_increment*MENU_ITEM_SAVE_STATE),	font_size+0.01f,	BLUE,	"Save State #%d", Settings.CurrentSaveStateSlot);
		cellDbgFontPrintf	(x_position,	ypos+(ypos_increment*MENU_ITEM_SAVE_STATE),	font_size,	menuitem_colors[MENU_ITEM_SAVE_STATE],	"Save State #%d", Settings.CurrentSaveStateSlot);

		cellDbgFontPrintf	(x_position,	(ypos+(ypos_increment*MENU_ITEM_KEEP_ASPECT_RATIO)),	font_size+0.01f,	BLUE,	"Aspect Ratio: %s %s %d:%d", ps3graphics_calculate_aspect_ratio_before_game_load() ?"(Auto)" : "", Settings.PS3KeepAspect == LAST_ASPECT_RATIO ? "Custom" : "", (int)ps3graphics_get_aspect_ratio_int(0), (int)ps3graphics_get_aspect_ratio_int(1));
		cellDbgFontPrintf(x_position,	(ypos+(ypos_increment*MENU_ITEM_KEEP_ASPECT_RATIO)),	font_size,	menuitem_colors[MENU_ITEM_KEEP_ASPECT_RATIO],	"Aspect Ratio: %s %s %d:%d", ps3graphics_calculate_aspect_ratio_before_game_load() ?"(Auto)" : "", Settings.PS3KeepAspect == LAST_ASPECT_RATIO ? "Custom" : "", (int)ps3graphics_get_aspect_ratio_int(0), (int)ps3graphics_get_aspect_ratio_int(1));

		cellDbgFontPrintf	(x_position,	(ypos+(ypos_increment*MENU_ITEM_OVERSCAN_AMOUNT)),	font_size+0.01f,	BLUE,	"Overscan: %f", (float)Settings.PS3OverscanAmount/100);
		cellDbgFontPrintf	(x_position,	(ypos+(ypos_increment*MENU_ITEM_OVERSCAN_AMOUNT)),	font_size,	menuitem_colors[MENU_ITEM_OVERSCAN_AMOUNT],	"Overscan: %f", (float)Settings.PS3OverscanAmount/100);

		cellDbgFontDraw();

		cellDbgFontPrintf	(x_position,	(ypos+(ypos_increment*MENU_ITEM_RESIZE_MODE)),	font_size+0.01f,	BLUE,	"Resize Mode");
		cellDbgFontPrintf	(x_position,	(ypos+(ypos_increment*MENU_ITEM_RESIZE_MODE)),	font_size,	menuitem_colors[MENU_ITEM_RESIZE_MODE],	"Resize Mode");

		switch(ps3graphics_get_orientation_name())
		{
			case NORMAL:
				snprintf(msg_temp, sizeof(msg_temp), "Normal");
				break;
			case VERTICAL:
				snprintf(msg_temp, sizeof(msg_temp), "Vertical");
				break;
			case FLIPPED:
				snprintf(msg_temp, sizeof(msg_temp), "Flipped");
				break;
			case FLIPPED_ROTATED:
				snprintf(msg_temp, sizeof(msg_temp), "Flipped Rotated");
				break;
		}


		cellDbgFontPrintf	(x_position,	(ypos+(ypos_increment*MENU_ITEM_ORIENTATION)),	font_size+0.01f,	BLUE,	"Orientation: %s", msg_temp);
		cellDbgFontPrintf	(x_position,	(ypos+(ypos_increment*MENU_ITEM_ORIENTATION)),	font_size,	menuitem_colors[MENU_ITEM_ORIENTATION],	"Orientation: %s", msg_temp);

		cellDbgFontPuts	(x_position,	(ypos+(ypos_increment*MENU_ITEM_FRAME_ADVANCE)),	font_size+0.01f,	BLUE,	"Frame Advance");
		cellDbgFontPuts	(x_position,	(ypos+(ypos_increment*MENU_ITEM_FRAME_ADVANCE)),	font_size,	menuitem_colors[MENU_ITEM_FRAME_ADVANCE],	"Frame Advance");

		cellDbgFontPuts	(x_position,	(ypos+(ypos_increment*MENU_ITEM_SCREENSHOT_MODE)),	font_size+0.01f,	BLUE,	"Screenshot Mode");
		cellDbgFontPuts	(x_position,	(ypos+(ypos_increment*MENU_ITEM_SCREENSHOT_MODE)),	font_size,	menuitem_colors[MENU_ITEM_SCREENSHOT_MODE],	"Screenshot Mode");

		cellDbgFontPuts	(x_position,	(ypos+(ypos_increment*MENU_ITEM_RESET)),	font_size+0.01f,	BLUE,	"Reset");
		cellDbgFontPuts	(x_position,	(ypos+(ypos_increment*MENU_ITEM_RESET)),	font_size,	menuitem_colors[MENU_ITEM_RESET],	"Reset");

		cellDbgFontPuts   (x_position,   (ypos+(ypos_increment*MENU_ITEM_RETURN_TO_GAME)),   font_size+0.01f,  BLUE,  "Return to Game");
		cellDbgFontPuts   (x_position,   (ypos+(ypos_increment*MENU_ITEM_RETURN_TO_GAME)),   font_size,  menuitem_colors[MENU_ITEM_RETURN_TO_GAME],  "Return to Game");

		cellDbgFontPuts	(x_position,	(ypos+(ypos_increment*MENU_ITEM_RETURN_TO_MENU)),	font_size+0.01f,	BLUE,	"Return to Menu");
		cellDbgFontPuts	(x_position,	(ypos+(ypos_increment*MENU_ITEM_RETURN_TO_MENU)),	font_size,	menuitem_colors[MENU_ITEM_RETURN_TO_MENU],	"Return to Menu");
#ifdef MULTIMAN_SUPPORT
		cellDbgFontPuts	(x_position,	(ypos+(ypos_increment*MENU_ITEM_RETURN_TO_MULTIMAN)),	font_size+0.01f,	BLUE,	"Return to multiMAN");
		cellDbgFontPuts	(x_position,	(ypos+(ypos_increment*MENU_ITEM_RETURN_TO_MULTIMAN)),	font_size,	menuitem_colors[MENU_ITEM_RETURN_TO_MULTIMAN],	"Return to multiMAN");
#endif
		cellDbgFontPuts	(x_position,	(ypos+(ypos_increment*MENU_ITEM_RETURN_TO_XMB)),	font_size+0.01f,	BLUE,	"Return to XMB");
		cellDbgFontPuts	(x_position,	(ypos+(ypos_increment*MENU_ITEM_RETURN_TO_XMB)),	font_size,	menuitem_colors[MENU_ITEM_RETURN_TO_XMB],	"Return to XMB");

		cellDbgFontDraw();

		if(frame_count < special_action_msg_expired)
		{
			cellDbgFontPrintf (0.09f, 0.90f, 1.51f, BLUE,	special_action_msg);
			cellDbgFontPrintf (0.09f, 0.90f, 1.50f, WHITE,	special_action_msg);
		}
		else
		{
			special_action_msg_expired = 0;
			cellDbgFontPrintf (0.09f,   0.90f,   0.98f+0.01f,      BLUE,           comment);
			cellDbgFontPrintf (0.09f,   0.90f,   0.98f,      LIGHTBLUE,           comment);
		}
		cellDbgFontDraw();
		_jsPlatformSwapBuffers(psgl_device);
		old_state = state;
		cellSysutilCheckCallback();
	}while(is_ingame_menu_running);
}

void emulator_implementation_set_texture(const char * fname)
{
	strcpy(Settings.PS3CurrentBorder,fname);
	ps3graphics_load_menu_texture(TEXTURE_BACKDROP, fname);
	ps3graphics_load_menu_texture(TEXTURE_MENU, DEFAULT_MENU_BORDER_FILE_);
}

static void get_path_settings(bool multiman_support)
{
	unsigned int get_type;
	unsigned int get_attributes;
	CellGameContentSize size;
	char dirName[CELL_GAME_DIRNAME_SIZE];

	memset(&size, 0x00, sizeof(CellGameContentSize));

	int ret = cellGameBootCheck(&get_type, &get_attributes, &size, dirName); 
	if(ret < 0)
	{
		printf("cellGameBootCheck() Error: 0x%x\n", ret);
	}
	else
	{
		printf("cellGameBootCheck() OK\n");
		printf("  get_type = [%d] get_attributes = [0x%08x] dirName = [%s]\n", get_type, get_attributes, dirName);
		printf("  hddFreeSizeKB = [%d] sizeKB = [%d] sysSizeKB = [%d]\n", size.hddFreeSizeKB, size.sizeKB, size.sysSizeKB);

		ret = cellGameContentPermit(contentInfoPath, usrDirPath);

		if(multiman_support)
		{
			snprintf(contentInfoPath, sizeof(contentInfoPath), "/dev_hdd0/game/%s", EMULATOR_CONTENT_DIR);
			snprintf(usrDirPath, sizeof(usrDirPath), "/dev_hdd0/game/%s/USRDIR", EMULATOR_CONTENT_DIR);
		}

		if(ret < 0)
		{
			printf("cellGameContentPermit() Error: 0x%x\n", ret);
		}
		else
		{
			printf("cellGameContentPermit() OK\n");
			printf("contentInfoPath:[%s]\n", contentInfoPath);
			printf("usrDirPath:[%s]\n",  usrDirPath);
		}

		/* now we fill in all the variables */
		snprintf(DEFAULT_PRESET_FILE, sizeof(DEFAULT_PRESET_FILE), "%s/presets/stock.conf", usrDirPath);
		snprintf(DEFAULT_BORDER_FILE, sizeof(DEFAULT_BORDER_FILE), "%s/borders/Centered-1080p/sonic-the-hedgehog-1.png", usrDirPath);
		snprintf(DEFAULT_MENU_BORDER_FILE_, sizeof(DEFAULT_MENU_BORDER_FILE_), "%s/borders/Menu/main-menu.png", usrDirPath);
		snprintf(GAME_AWARE_SHADER_DIR_PATH, sizeof(GAME_AWARE_SHADER_DIR_PATH), "%s/gameaware", usrDirPath);
		snprintf(INPUT_PRESETS_DIR_PATH, sizeof(INPUT_PRESETS_DIR_PATH), "%s/input-presets", usrDirPath); 
		snprintf(PRESETS_DIR_PATH, sizeof(PRESETS_DIR_PATH), "%s/presets", usrDirPath); 
		snprintf(BORDERS_DIR_PATH, sizeof(BORDERS_DIR_PATH), "%s/borders", usrDirPath); 
		snprintf(SHADERS_DIR_PATH, sizeof(SHADERS_DIR_PATH), "%s/shaders", usrDirPath);
		snprintf(DEFAULT_SHADER_FILE, sizeof(DEFAULT_SHADER_FILE), "%s/shaders/stock.cg", usrDirPath);
		snprintf(DEFAULT_MENU_SHADER_FILE_, sizeof(DEFAULT_MENU_SHADER_FILE_), "%s/shaders/Borders/Menu/border-only.cg", usrDirPath);
		snprintf(SYS_CONFIG_FILE_, sizeof(SYS_CONFIG_FILE_), "%s/genesisplus.conf", usrDirPath);
		snprintf(CONFIGURATION_FILE, sizeof(CONFIGURATION_FILE), "%s/config.bin", usrDirPath);
		snprintf(BIOS_FILE, sizeof(BIOS_FILE), "%s/bios.bin", usrDirPath);
	}
}

char * decorateGameName(unsigned int drv)
{
	if (drv >= nBurnDrvCount)
		return "";

	unsigned int nOldBurnDrv = nBurnDrvSelect;
	nBurnDrvSelect = drv;

	// get game full name
	static char szDecoratedName[1024] = "";
	strcpy(szDecoratedName, BurnDrvGetTextA(DRV_FULLNAME));

	// get game extra info
	char szGameInfo[256] = " [";
	bool hasInfo = false;

	if (BurnDrvGetFlags() & BDF_PROTOTYPE)
	{
		strcat(szGameInfo, "prototype");
		hasInfo = true;
	}
	if (BurnDrvGetFlags() & BDF_BOOTLEG)
	{
		strcat(szGameInfo, "bootleg");
		hasInfo = true;
	}
	if (BurnDrvGetTextA(DRV_COMMENT))
	{
		strcat(szGameInfo, BurnDrvGetTextA(DRV_COMMENT));
		hasInfo = true;
	}

	if (hasInfo)
	{
		strcat(szGameInfo, "]");
		strcat(szDecoratedName, szGameInfo);
	}

	nBurnDrvSelect = nOldBurnDrv;

	return szDecoratedName;
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

int main(int argc, char **argv)
{
	/*struct stat st;*/
	sys_spu_initialize(6, 1); 
	cellSysmoduleLoadModule(CELL_SYSMODULE_FS);
	cellSysmoduleLoadModule(CELL_SYSMODULE_IO);
	cellSysmoduleLoadModule(CELL_SYSMODULE_PNGDEC);
	cellSysmoduleLoadModule(CELL_SYSMODULE_JPGDEC);
	cellSysmoduleLoadModule(CELL_SYSMODULE_SYSUTIL_GAME);

	cellSysutilRegisterCallback(0, sysutil_exit_callback, NULL); 

#ifdef CELL_DEBUG_CONSOLE
	cellConsoleInit();
	cellConsoleNetworkInitialize();
	cellConsoleNetworkServerInit(-1);
	cellConsoleScreenShotPluginInit();
#endif

	/* parse input roms*/
#ifdef MULTIMAN_SUPPORT
	return_to_MM = true;

	if(argc>1)
	{
		mode_switch = MODE_MULTIMAN_STARTUP;
		strncpy(MULTIMAN_GAME_TO_BOOT, argv[1], sizeof(MULTIMAN_GAME_TO_BOOT));
	}
#endif
	get_path_settings(return_to_MM);

	emulator_init_settings();

#if(CELL_SDK_VERSION > 0x340000)
	if (Settings.ScreenshotsEnabled)
	{
		cellSysmoduleLoadModule(CELL_SYSMODULE_SYSUTIL_SCREENSHOT);
		CellScreenShotSetParam  screenshot_param = {0, 0, 0, 0};

		screenshot_param.photo_title = EMULATOR_NAME;
		screenshot_param.game_title = EMULATOR_NAME;
		cellScreenShotSetParameter (&screenshot_param);
		cellScreenShotEnable();
	}
#endif

	ps3graphics_new((uint32_t)Settings.PS3CurrentResolution, Settings.PS3KeepAspect, Settings.PS3Smooth, Settings.PS3Smooth2, Settings.PS3CurrentShader, Settings.PS3CurrentShader2, DEFAULT_MENU_SHADER_FILE_, Settings.PS3OverscanEnabled, (float)Settings.PS3OverscanAmount/100, Settings.PS3PALTemporalMode60Hz, Settings.Throttled, Settings.TripleBuffering, Settings.ViewportX, Settings.ViewportY, Settings.ViewportWidth, Settings.ViewportHeight, Settings.ScaleEnabled, Settings.ScaleFactor);

	if(Settings.ApplyShaderPresetOnStartup)
		emulator_implementation_set_shader_preset(Settings.ShaderPresetPath); 

	cell_pad_input_init();

	oskutil_init(&oskutil_handle, 0);

	frame_count = 0;
	ps3graphics_init_dbgfont();

	emulator_toggle_sound(Settings.SoundMode);

	emulator_implementation_set_texture(Settings.PS3CurrentBorder);

#if(CELL_SDK_VERSION > 0x340000)
	cellSysutilEnableBgmPlayback();
#endif

	MenuInit();

	BurnLibInit();

	BurnExtLoadOneRom = archiveLoadOneFile;
	g_fba_frame = (uint8_t*)realloc(g_fba_frame, 1024*1024);

	/* char * szName; 
	   BurnDrvGetArchiveName(&szName, 0);
	   char * vendetta_hack = strstr(szName,"vendetta");
	 */
	int samples = 48000;

	/*
	   if(vendetta_hack)
	   samples = SAMPLERATE_44KHZ;
	 */

	emulator_audio_init(samples);

	do
	{
		switch(mode_switch)
		{
			case MODE_MENU:
				ps3graphics_set_orientation(NORMAL);
				MenuMainLoop();
				break;
			case MODE_EMULATION:
				if(ingame_menu_item != 0)
					is_ingame_menu_running = 1;

				emulator_start();

				if(Settings.Throttled)
					audio_driver->pause(audio_handle);

				if(is_ingame_menu_running)
					ingame_menu();
				break;
#ifdef MULTIMAN_SUPPORT
			case MODE_MULTIMAN_STARTUP:
				Emulator_StartROMRunning(1);
				Emulator_RequestLoadROM(MULTIMAN_GAME_TO_BOOT);
				break;
#endif
			case MODE_EXIT:
				emulator_shutdown();
		}
	}while(1);
}
