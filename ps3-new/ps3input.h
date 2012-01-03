/******************************************************************************* 
 * ps3input.h - FBA Next Slim PS3
 *
 *  Created on: Jan 1, 2012
********************************************************************************/

#ifndef PS3INPUT_H_
#define PS3INPUT_H_

#include <string.h>
#include "types.h"
/* Input bitmasks */
#define BTN_FIRE1			1
#define BTN_FIRE2			2
#define BTN_FIRE3			4
#define BTN_FIRE4			8
#define BTN_COIN			16
#define BTN_START			32
#define BTN_FIRE5			64
#define BTN_FIRE6			128
#define BTN_LEFT			256
#define BTN_RIGHT			512
#define BTN_UP				1024
#define BTN_DOWN			2048
#define BTN_LASTGAMEBUTTON BTN_MODE

/* Special button actions */
#define BTN_INGAME_MENU 	(0x1000)
#define BTN_SOFTRESET		(0x2000)
#define BTN_HARDRESET		(0x4000)
#define BTN_QUICKLOAD		(0x8000)
#define BTN_QUICKSAVE		(0x10000)
#define BTN_EXITTOMENU  	(0x20000)
#define BTN_NONE        	(0x40000)
#define BTN_INCREMENTSAVE	(0x80000)
#define BTN_DECREMENTSAVE	(0x100000)
#define BTN_FASTFORWARD		(0x200000)

#define BTN_FIRSTEXTRABUTTON BTN_INGAME_MENU

#define CONTROL_SCHEME_DEFAULT	0
#define CONTROL_SCHEME_CUSTOM	1

#define MAX_PADS 7

#include "inp_keys.h"		// Key codes

#define LIST_DEVICES
#define MAX_KEYBOARD	(0)
#define MAX_JOYSTICK	(4)
#define MAX_JOYAXIS	(4)
#define MAX_MOUSE	(0)
#define MAX_MOUSEAXIS	(0)

#define P1_COIN	0x06
#define P1_START 0x02
#define P1_LEFT 0xCB
#define P1_RIGHT 0xCD
#define P1_UP 0xC8
#define P1_DOWN 0xD0
#define P1_FIRE1 0x2C
#define P1_FIRE2 0x2D
#define P1_FIRE3 0x2E
#define P1_FIRE4 0x2F
#define P1_FIRE5 0x1F
#define P1_FIRE6 0x20
#define P1_SERVICE 0x3C

#define P2_COIN 0x07
#define P2_START 0x03
#define P2_LEFT 0x4000
#define P2_RIGHT 0x4001
#define P2_UP 0x4002
#define P2_DOWN 0x4003
#define P2_FIRE1 0x4080
#define P2_FIRE2 0x4081
#define P2_FIRE3 0x4082
#define P2_FIRE4 0x4083
#define P2_FIRE5 0x4084
#define P2_FIRE6 0x4085

#define P3_COIN 0x08
#define P3_START 0x04
#define P3_LEFT 0x4100
#define P3_RIGHT 0x4101
#define P3_UP 0x4102
#define P3_DOWN 0x4103
#define P3_FIRE1 0x4180
#define P3_FIRE2 0x4181
#define P3_FIRE3 0x4182
#define P3_FIRE4 0x4183
#define P3_FIRE5 0x4184
#define P3_FIRE6 0x4185

#define P4_COIN 0x09
#define P4_START 0x05
#define P4_LEFT 0x4200
#define P4_RIGHT 0x4201
#define P4_UP 0x4202
#define P4_DOWN 0x4203
#define P4_FIRE1 0x4280
#define P4_FIRE2 0x4281
#define P4_FIRE3 0x4282
#define P4_FIRE4 0x4283
#define P4_FIRE5 0x4284
#define P4_FIRE6 0x4285

enum {
	CTRL_UP_DEF,
	CTRL_DOWN_DEF,
	CTRL_LEFT_DEF,
	CTRL_RIGHT_DEF,
	CTRL_CIRCLE_DEF,
	CTRL_CROSS_DEF,
	CTRL_TRIANGLE_DEF,
	CTRL_SQUARE_DEF,
	CTRL_SELECT_DEF,
	CTRL_START_DEF,
	CTRL_L1_DEF,
	CTRL_R1_DEF,
	CTRL_L2_DEF,
	CTRL_R2_DEF,
	CTRL_L3_DEF,
	CTRL_R3_DEF,
	CTRL_L2_L3_DEF,
	CTRL_L2_R3_DEF,
	CTRL_L2_RSTICK_RIGHT_DEF,
	CTRL_L2_RSTICK_LEFT_DEF,
	CTRL_L2_RSTICK_UP_DEF,
	CTRL_L2_RSTICK_DOWN_DEF,
	CTRL_R2_RSTICK_RIGHT_DEF,
	CTRL_R2_RSTICK_LEFT_DEF,
	CTRL_R2_RSTICK_UP_DEF,
	CTRL_R2_RSTICK_DOWN_DEF,
	CTRL_R2_R3_DEF,
	CTRL_R3_L3_DEF,
	CTRL_RSTICK_UP_DEF,
	CTRL_RSTICK_DOWN_DEF,
	CTRL_RSTICK_LEFT_DEF,
	CTRL_RSTICK_RIGHT_DEF
};

#define CinpState(nCode) CellinpState(nCode)

#define BTN_DEF_MAX CTRL_RSTICK_RIGHT_DEF+1

#define CTRL_SERVICE_MASK BTN_DEF_MAX+1

#define BTN

extern uint32_t control_binds[MAX_PADS][BTN_DEF_MAX];
extern uint32_t default_control_binds[BTN_DEF_MAX];

#define Input_MapButton(buttonmap, next, defaultbutton) \
	if(defaultbutton == NULL) \
		buttonmap = Input_GetAdjacentButtonmap(buttonmap, next); \
	else \
		buttonmap = defaultbutton;

const char * Input_PrintMappedButton(uint32_t mappedbutton);
uint32_t Input_GetAdjacentButtonmap(uint32_t buttonmap, uint32_t next);

#endif
