// Module for Cell Input
#include <cell/pad.h>
#include <sys/cdefs.h>

#include "burner.h"
#include "cellframework2/input/pad_input.h"
// Key codes
#include "inp_keys.h"
#include "menu.h"

#define LIST_DEVICES
#define MAX_KEYBOARD	(0)
#define MAX_JOYSTICK	(4)
#define MAX_JOYAXIS		(4)
#define MAX_MOUSE		(0)
#define MAX_MOUSEAXIS	(0)

int nPlayerToGamepad[4] = {0, 1, 2, 3};
static int nJoystickCount = 4;		// We poll all 4 gamepad ps3 ports by default
extern int ArcadeJoystick;
extern bool DoReset;
extern int GameStatus;

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
// ----------------------------------------------------------------------------
// Get the state (pressed = 1, not pressed = 0) of a particular input code

int CellinpState(int nCode)
{
	uint32_t numPadsConnected = 0;

	uint64_t pads_connected = cell_pad_input_pads_connected();
	uint64_t new_state_p1 = cell_pad_input_poll_device(0);
	uint64_t pausemenu_condition = ArcadeJoystick ? (CTRL_SELECT(new_state_p1) && CTRL_START(new_state_p1)) : (CTRL_L2(new_state_p1) && CTRL_R2(new_state_p1) && CTRL_R1(new_state_p1));

	if (nCode < 0)
		return 0;

	if (DoReset)
	{

		if (nCode == FBK_F3)
		{
			DoReset = false;
			return 1;
		}

	}

	if (pausemenu_condition)
	{
		//setPauseMode(1);
		audio.stop();
		GameStatus = PAUSE;
		is_running = 0;
		return 0;
	}


	numPadsConnected = cell_pad_input_pads_connected();

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

	return 0;
}
 
void doStretch(void)
{
	static uint64_t old_state;
	uint64_t new_state = cell_pad_input_poll_device(0);
	uint64_t diff_state = old_state ^ new_state;

	if(CTRL_LSTICK_LEFT(new_state))
		nXOffset -= 1;

	else if (CTRL_LSTICK_RIGHT(new_state))
		nXOffset += 1;

	if (CTRL_LSTICK_UP(new_state))
		nYOffset += 1;

	else if (CTRL_LSTICK_DOWN(new_state)) 
		nYOffset -= 1;

	if (CTRL_RSTICK_LEFT(new_state))
		nXScale -= 1;
	else if (CTRL_RSTICK_RIGHT(new_state))
		nXScale += 1;

	if (CTRL_RSTICK_UP(new_state))
		nYScale += 1;
	else if (CTRL_RSTICK_DOWN(new_state))
		nYScale -= 1;

	if (CTRL_CIRCLE(new_state))
	{
		GameStatus = PAUSE;
		is_running = 0;
	}

	if (CTRL_TRIANGLE(new_state))
		nXScale = nYScale = nXOffset = nYOffset = 0;	// reset to default

	old_state = new_state;
}
