#ifndef MENU_H
#define MENU_H

#include <stdlib.h>
#include <string.h>
#include <sys/timer.h>
#include <sys/return_code.h>
#include <sys/process.h>
#include <cell/audio.h>
#include <cell/sysmodule.h>
#include <cell/cell_fs.h>
#include <cell/pad.h>
#include <stddef.h>
#include <math.h>
#include <sysutil/sysutil_sysparam.h>
#include <sys/spu_initialize.h>
#include <sys/paths.h>
 
enum {
	MENU,
	EMULATING,
	EMULATING_INIT,
	INGAME_MENU,
	PAUSE,
	CONFIG_MENU,
	INPUT_MENU,
	INPUT_WITH_LIST_MENU,
	SCREEN_RESIZE,
	DIP_MENU,
	DIP_WITH_LIST_MENU
};

enum {
   SETTING_DISPLAY_FRAMERATE,
   SETTING_SOUND_SAMPLERATE,
   SETTING_RESOLUTION,
   SETTING_KEEP_ASPECT,
   SETTING_ROTATE,
   SETTING_FBO_ENABLED,
   SETTING_CURRENT_SHADER,
   SETTING_CURRENT_SHADER2,
   SETTING_BILINEAR_FILTER,
   SETTING_BILINEAR_FILTER2,
   SETTING_SCALING_FACTOR,
   SETTING_OVERSCAN,
   SETTING_VSYNC,
   SETTING_HIDE_CLONES,
   SETTING_SHOW_THREE_FOUR_PLAYER_ONLY,
   SETTING_TRIPLE_BUFFER
};

#define MAX_NO_OF_SETTINGS    SETTING_TRIPLE_BUFFER+1

enum {
   INGAME_MAP_BUTTONS,
   INGAME_DIP_SWITCHES,
   INGAME_RESIZE_SCREEN,
   INGAME_FRAME_ADVANCE,
   INGAME_FBO_ENABLED,
   INGAME_CURRENT_SHADER,
   INGAME_CURRENT_SHADER2,
   INGAME_KEEP_ASPECT,
   INGAME_BILINEAR_FILTER,
   INGAME_BILINEAR_FILTER2,
   INGAME_SCALING_FACTOR,
   INGAME_OVERSCAN,
   INGAME_ROTATE,
   INGAME_SAVE_STATE,
   INGAME_LOAD_STATE,
   INGAME_RESET_GAME,
#ifdef MULTIMAN_SUPPORT
   INGAME_RETURN_TO_MULTIMAN,
#endif
   INGAME_EXIT_GAME,
   INGAME_BACK_TO_GAME
};

#define LAST_INGAME_SETTING INGAME_BACK_TO_GAME

extern void BuildRomList();
extern void RomMenu();
extern void ConfigMenu();
extern int InitRomList();
extern int InitInputList();
extern int InitDipList();
extern void InputFrameMove();
extern void InputMenu();
extern void FrameMove();
extern void ConfigFrameMove();
extern void LoadShader();
extern void InGameMenu();
extern void InGameFrameMove();
extern void DipMenu();
extern void DipFrameMove();
#endif
