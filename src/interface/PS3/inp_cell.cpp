// Module for Cell Input
#include <cell/pad.h>
#include <sys/cdefs.h>

#include "burner.h"
#include "cellframework2/input/pad_input.h"
#include <math.h>
// Key codes
#include "inp_keys.h"
#include "menu.h"

#define LIST_DEVICES
#define MAX_KEYBOARD	(0)
#define MAX_JOYSTICK	(4)
#define MAX_JOYAXIS		(4)
#define MAX_MOUSE		(0)
#define MAX_MOUSEAXIS	(0)

int	 nPlayerToGamepad[4] = {0, 1, 2, 3};
static int nJoystickCount = 4;		// We poll all 4 gamepad ps3 ports by default
extern int ArcadeJoystick;
extern bool DoReset;
extern int GameStatus;

// Set up a single joystick
static int CellinpJoystickInitSingle(int i)
{

	return 0;
}

// Get a DI interface to each joystick
static int CellinpJoystickInitMaster()
{

	return 0;
}

// ----------------------------------------------------------------------------

// Get a DI interface to all mice
static int CellinpMouseInitMaster()
{

	return 0;
}

// ----------------------------------------------------------------------------

int CellinpSetCooperativeLevel(bool bExclusive, bool bForeGround)
{


	return 0;
}

// ----------------------------------------------------------------------------

int CellinpExit()
{		 
	return 0;
}

int CellinpInit()
{	 
	return 0;
}

// ----------------------------------------------------------------------------

// Call before checking for Input in a frame
int CellinpStart()
{	
	return 0;
}

// ----------------------------------------------------------------------------

// Read the keyboard
static int ReadKeyboard(struct DinpKeyboardProperties* Keyboard)
{
	return 0;
}
 

// Read one of the joysticks
static int ReadJoystick(struct DinpJoyProperties* Joystick)
{
	return 0;
}

// Check a subcode (the 40xx bit in 4001, 4102 etc) for a joystick input code
static int JoystickState(struct DinpJoyProperties* Joystick, unsigned int nSubCode)
{
	return 0;
}

// ----------------------------------------------------------------------------

// Read the mouse
static int ReadMouse(struct DinpMouseProperties* Mouse)
{
	return 0;
}

// Check a subcode (the 80xx bit in 8001, 8102 etc) for a mouse input code
static int CheckMouseState(struct DinpMouseProperties* Mouse, unsigned int nSubCode)
{
	return 0;
}


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
		case 0x06:
			return CTRL_SELECT(new_state_p1);
		case 0x02:
			return CTRL_START(new_state_p1);
		case 0xC8:
			return ((CTRL_UP(new_state_p1) | CTRL_LSTICK_UP(new_state_p1)) != 0);
		case 0xD0: 
			return ((CTRL_DOWN(new_state_p1) | CTRL_LSTICK_DOWN(new_state_p1)) != 0);
		case 0xCB:
			return ((CTRL_LEFT(new_state_p1) | CTRL_LSTICK_LEFT(new_state_p1)) != 0);
		case 0xCD:
			return ((CTRL_RIGHT(new_state_p1) | CTRL_LSTICK_RIGHT(new_state_p1)) != 0);
		case 0x2C:
			return CTRL_CROSS(new_state_p1);                                        // fire 1
		case 0x2D:
			return CTRL_CIRCLE(new_state_p1);                                       // fire 2
		case 0x2E:
			return CTRL_SQUARE(new_state_p1);                                       // fire 3
		case 0x2F: 
			return CTRL_TRIANGLE(new_state_p1);                                     // fire 4
		case 0x1F:
			return CTRL_L1(new_state_p1);                                           // fire 5
		case 0x20:
			return CTRL_R1(new_state_p1);                                           // fire 6
		case 0x88:
			return CTRL_L2(new_state_p1);
		case 0x8A:			 
			return CTRL_R2(new_state_p1);
		case 0x3b:
			return CTRL_L3(new_state_p1);
		case 0x3c:
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
			case 0x07:
				return CTRL_SELECT(new_state_p2);
			case 0x03:
				return CTRL_START(new_state_p2);
			case 0x4002:
				return ((CTRL_UP(new_state_p2) | CTRL_LSTICK_UP(new_state_p2)) != 0);
			case 0x4003:
				return ((CTRL_DOWN(new_state_p2) | CTRL_LSTICK_DOWN(new_state_p2)) != 0);
			case 0x4000:
				return ((CTRL_LEFT(new_state_p2) | CTRL_LSTICK_LEFT(new_state_p2)) != 0);
			case 0x4001:
				return ((CTRL_RIGHT(new_state_p2) | CTRL_LSTICK_RIGHT(new_state_p2)) != 0);
			case 0x4080:
				return CTRL_CROSS(new_state_p2);                                        // fire 1
			case 0x4081:
				return CTRL_CIRCLE(new_state_p2);                                       // fire 2
			case 0x4082:
				return CTRL_SQUARE(new_state_p2);                                       // fire 3
			case 0x4083:
				return CTRL_TRIANGLE(new_state_p2);                                     // fire 4
			case 0x4084:
				return CTRL_L1(new_state_p2);                                           // fire 5
			case 0x4085:
				return CTRL_R1(new_state_p2);                                           // fire 6
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
			case 0x08:
				return CTRL_SELECT(new_state_p3);
			case 0x04:
				return CTRL_START(new_state_p3);
			case 0x4102:
				return ((CTRL_UP(new_state_p3) | CTRL_LSTICK_UP(new_state_p3)) != 0);
			case 0x4103:
				return ((CTRL_DOWN(new_state_p3) | CTRL_LSTICK_DOWN(new_state_p3)) != 0);
			case 0x4100:
				return ((CTRL_LEFT(new_state_p3) | CTRL_LSTICK_LEFT(new_state_p3)) != 0);
			case 0x4101:
				return ((CTRL_RIGHT(new_state_p3) | CTRL_LSTICK_RIGHT(new_state_p3)) != 0);
			case 0x4180:
				return CTRL_CROSS(new_state_p3);                                                          // fire 1
			case 0x4181:
				return CTRL_CIRCLE(new_state_p3);                                                         // fire 2
			case 0x4182:
				return CTRL_SQUARE(new_state_p3);                                                         // fire 3
			case 0x4183:
				return CTRL_TRIANGLE(new_state_p3);                                                       // fire 4
			case 0x4184:
				return CTRL_L1(new_state_p3);                                                             // fire 5
			case 0x4185:
				return CTRL_R1(new_state_p3);                                                             // fire 6
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
			case 0x09:
				return CTRL_SELECT(new_state_p4);
			case 0x05:
				return CTRL_START(new_state_p4);
			case 0x4202:
				return ((CTRL_UP(new_state_p4) | CTRL_LSTICK_UP(new_state_p4)) != 0);
			case 0x4203:
				return ((CTRL_DOWN(new_state_p4) | CTRL_LSTICK_DOWN(new_state_p4)) != 0);
			case 0x4200:
				return ((CTRL_LEFT(new_state_p4) | CTRL_LSTICK_LEFT(new_state_p4)) != 0);
			case 0x4201:
				return ((CTRL_RIGHT(new_state_p4) | CTRL_LSTICK_RIGHT(new_state_p4)) != 0);
			case 0x4280:
				return CTRL_CROSS(new_state_p4);                                              // fire 1
			case 0x4281:
				return CTRL_CIRCLE(new_state_p4);                                             // fire 2
			case 0x4282:
				return CTRL_SQUARE(new_state_p4);                                             // fire 3
			case 0x4283:
				return CTRL_TRIANGLE(new_state_p4);                                           // fire 4
			case 0x4284:
				return CTRL_L1(new_state_p4);                                                 // fire 5
			case 0x4285:
				return CTRL_R1(new_state_p4);                                                 // fire 6
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
 
// Read one joystick axis
int CellinpJoyAxis(int i, int nAxis)
{	
	return 0;
}

// Read one mouse axis
int CellinpMouseAxis(int i, int nAxis)
{

}

// This function finds which key is pressed
// To ensure analog joystick axes are handled correctly, call with CreateBaseline = true the 1st time
int CellinpFind(bool CreateBaseline)
{
	return 0;
}

int CellinpGetControlName(int nCode, TCHAR* pszDeviceName, TCHAR* pszControlName)
{
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

// ----------------------------------------------------------------------------

struct InputInOut InputInOutCellInput = { CellinpInit, CellinpExit, CellinpSetCooperativeLevel, CellinpStart, CellinpState, CellinpJoyAxis, CellinpMouseAxis, CellinpFind, CellinpGetControlName, NULL };
