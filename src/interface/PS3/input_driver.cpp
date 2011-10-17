#include <cell/pad.h>
#include <sys/cdefs.h>

#include "burner.h"
#include "cellframework2/input/pad_input.h"
#include "input_driver.h"
#include "menu.h"
#include "audio_driver.h"
#include "vid_psgl.h"

bool bInputOkay = false;

#define KEYBIND_PS3BUTTON 0
#define KEYBIND_PS3PLAYER 1
static uint64_t keybinds[1024][2] = {0}; 

// This will process all PC-side inputs and optionally update the emulated game side.
void InputMake_Analog(void)
{
	uint32_t set_reset = 0;
	struct GameInp* pgi = GameInp;
	uint32_t controller_binds_count = nGameInpCount;

	uint64_t new_state_p1 = cell_pad_input_poll_device(0);
	uint64_t pausemenu_condition = ArcadeJoystick ? (CTRL_SELECT(new_state_p1) && CTRL_START(new_state_p1)) : (CTRL_L2(new_state_p1) && CTRL_R2(new_state_p1) && CTRL_R1(new_state_p1));

	if (set_reset && DoReset)
		DoReset = false;

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
				if(pgi->Input.Switch != FBK_F3)
				{ // Digital input
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
				else
					set_reset = 1;
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
void InputMake(void)
{
	uint32_t set_reset = 0;
	struct GameInp* pgi = GameInp;
	uint32_t controller_binds_count = nGameInpCount;

	uint64_t new_state_p1 = cell_pad_input_poll_device(0);
	uint64_t pausemenu_condition = ArcadeJoystick ? (CTRL_SELECT(new_state_p1) && CTRL_START(new_state_p1)) : (CTRL_L2(new_state_p1) && CTRL_R2(new_state_p1) && CTRL_R1(new_state_p1));


	if (set_reset && DoReset)
		DoReset = false;

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
				if(pgi->Input.Switch != FBK_F3)
				{ // Digital input
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
				else
					set_reset = 1;
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
