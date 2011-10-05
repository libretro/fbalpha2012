// Burner Input module
#include <cell/pad.h>
#include <sys/cdefs.h>

#include "burner.h"
#include "cellframework2/input/pad_input.h"
#include "input_driver.h"
#include "menu.h"
#include "audio_driver.h"

bool bInputOkay = false;

#define CinpState(nCode) CellinpState(nCode)

int InputInit()
{
	bInputOkay = true;
	return bInputOkay;
}

int InputExit()
{
	bInputOkay = false;

	return bInputOkay;
}

static int CellinpState(int nCode)
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
		audio_stop();
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

// This will process all PC-side inputs and optionally update the emulated game side.
void InputMake(void)
{
	struct GameInp* pgi;
	unsigned int i;

	// Do one frames worth of keyboard input sliders
	// Begin of InputTick()

	for (i = 0, pgi = GameInp; i < nGameInpCount; i++, pgi++)
	{
		int nAdd = 0;
		if ((pgi->nInput &  GIT_GROUP_SLIDER) == 0) // not a slider
			continue;

		if (pgi->nInput == GIT_KEYSLIDER)
		{
			// Get states of the two keys
			if (CinpState(pgi->Input.Slider.SliderAxis.nSlider[0]))
				nAdd -= 0x100;
			if (CinpState(pgi->Input.Slider.SliderAxis.nSlider[1]))
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

	for (i = 0, pgi = GameInp; i < nGameInpCount; i++, pgi++)
	{
		if (pgi->Input.pVal == NULL)
			continue;

		switch (pgi->nInput)
		{
			case 0: // Undefined
				pgi->Input.nVal = 0;
				break;
			case GIT_CONSTANT: // Constant value
				pgi->Input.nVal = pgi->Input.Constant.nConst;
				*(pgi->Input.pVal) = pgi->Input.nVal;
				break;
			case GIT_SWITCH:
				{ // Digital input
					int s = CinpState(pgi->Input.Switch.nCode);

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
