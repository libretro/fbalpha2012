/******************************************************************************* 
 * emu-ps3.h - FBA Next Slim PS3
 *
 *  Created on: Jan 1, 2012
********************************************************************************/

#ifndef FBA_NEXT_SLIM_H_
#define FBA_NEXT_SLIM_H_

/* System includes */

#include <sysutil/sysutil_gamecontent.h>

#ifdef CELL_DEBUG_CONSOLE
#include <cell/control_console.h>
#endif

#if(CELL_SDK_VERSION > 0x340000)
#include <sysutil/sysutil_screenshot.h>
#include <sysutil/sysutil_bgmplayback.h>
#endif

/* Emulator-specific includes */

/* PS3 frontend includes */

#include "cellframework2/audio/stream.h"
#include "cellframework2/input/pad_input.h"
#include "cellframework2/utility/oskutil.h"

#include "emu-ps3-constants.h"
#include "ps3input.h"

#define MAP_BUTTONS_OPTION_SETTER	0
#define MAP_BUTTONS_OPTION_GETTER	1
#define MAP_BUTTONS_OPTION_DEFAULT	2
#define MAP_BUTTONS_OPTION_NEW		3

#define WRITE_CONTROLS			0
#define READ_CONTROLS			1
#define SET_ALL_CONTROLS_TO_DEFAULT	2

enum {
   MENU_ITEM_LOAD_STATE = 0,
   MENU_ITEM_SAVE_STATE,
   MENU_ITEM_KEEP_ASPECT_RATIO,
   MENU_ITEM_OVERSCAN_AMOUNT,
   MENU_ITEM_RESIZE_MODE,
   MENU_ITEM_FRAME_ADVANCE,
   MENU_ITEM_SCREENSHOT_MODE,
   MENU_ITEM_RESET,
   MENU_ITEM_RETURN_TO_GAME,
   MENU_ITEM_RETURN_TO_MENU,
#ifdef MULTIMAN_SUPPORT
   MENU_ITEM_RETURN_TO_MULTIMAN,
#endif
   MENU_ITEM_RETURN_TO_XMB
};

#define MENU_ITEM_LAST           MENU_ITEM_RETURN_TO_XMB+1

float Emulator_GetFontSize(void);
bool Emulator_IsROMLoaded(void);
void Emulator_StopROMRunning(void);
void Emulator_StartROMRunning(uint32_t set_is_running);
void Emulator_SetExtraCartPaths(void);
bool Emulator_ROMRunning(void);
void Emulator_RequestLoadROM(const char * rom);
void emulator_toggle_sound(uint64_t soundmode);
void emulator_implementation_set_texture(const char * fname);
void emulator_save_settings(uint64_t filetosave);
void emulator_implementation_switch_control_scheme(void);
void emulator_set_controls(const char * config_file, int mapping_enum, const char * title);
void emulator_implementation_set_shader_preset(const char * fname);
void set_text_message(const char * message, uint32_t speed);
int emulator_audio_init(int samplerate);

extern char contentInfoPath[MAX_PATH_LENGTH];
extern char usrDirPath[MAX_PATH_LENGTH];
extern char DEFAULT_PRESET_FILE[MAX_PATH_LENGTH];
extern char DEFAULT_BORDER_FILE[MAX_PATH_LENGTH];
extern char DEFAULT_MENU_BORDER_FILE_[MAX_PATH_LENGTH];
extern char GAME_AWARE_SHADER_DIR_PATH[MAX_PATH_LENGTH];
extern char INPUT_PRESETS_DIR_PATH[MAX_PATH_LENGTH];
extern char PRESETS_DIR_PATH[MAX_PATH_LENGTH];
extern char BORDERS_DIR_PATH[MAX_PATH_LENGTH];
extern char SHADERS_DIR_PATH[MAX_PATH_LENGTH];
extern char DEFAULT_SHADER_FILE[MAX_PATH_LENGTH];
extern char DEFAULT_MENU_SHADER_FILE_[MAX_PATH_LENGTH];
extern char BIOS_FILE[MAX_PATH_LENGTH];
extern oskutil_params oskutil_handle;
extern struct cell_audio_params params;
extern int mode_switch;
extern uint32_t special_action_msg_expired;

#endif /* FBA_NEXT_SLIM_H_ */
