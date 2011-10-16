#include <cell/pad.h>
#include <sys/cdefs.h>

#include "burner.h"
#include "cellframework2/input/pad_input.h"
#include "input_driver.h"
#include "menu.h"
#include "audio_driver.h"
#include "vid_psgl.h"

bool bInputOkay = false;

static uint32_t CellinpState(int nCode)
{
	#if 0
	if (nCode < 0)
		return 0;
	#endif

	uint32_t numPadsConnected = cell_pad_input_pads_connected();

	uint64_t new_state_p1 = cell_pad_input_poll_device(0);
	uint64_t pausemenu_condition = ArcadeJoystick ? (CTRL_SELECT(new_state_p1) && CTRL_START(new_state_p1)) : (CTRL_L2(new_state_p1) && CTRL_R2(new_state_p1) && CTRL_R1(new_state_p1));


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
		audio_stop();
		GameStatus = PAUSE;
		is_running = 0;
		return 0;
	}

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
void InputMake_Analog(void)
{
	struct GameInp* pgi;
	unsigned int i;
	uint32_t controller_binds_count = nGameInpCount;

	// Do one frames worth of keyboard input sliders
	// Begin of InputTick()

	for (i = 0, pgi = GameInp; i < controller_binds_count; i++, pgi++)
	{
		int nAdd = 0;
		if ((pgi->nInput &  GIT_GROUP_SLIDER) == 0) // not a slider
			continue;

		if (pgi->nInput == GIT_KEYSLIDER)
		{
			// Get states of the two keys
			if (CinpState(pgi->Input.Slider.SliderAxis[0]))
				nAdd -= 0x100;
			if (CinpState(pgi->Input.Slider.SliderAxis[1]))
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
	for (i = 0; i < controller_binds_count; i++, pgi++)
	{
		switch (pgi->nInput)
		{
			case GIT_CONSTANT: // Constant value
				pgi->Input.nVal = pgi->Input.Constant;
				*(pgi->Input.pVal) = pgi->Input.nVal;
				break;
			case GIT_SWITCH:
				{ // Digital input
					int s = CinpState(pgi->Input.Switch);

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

// This will process all PC-side inputs and optionally update the emulated game side.
void InputMake(void)
{
	struct GameInp* pgi;
	uint32_t controller_binds_count = nGameInpCount;

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
				{ // Digital input
					int s = CinpState(pgi->Input.Switch);

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
	}
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
