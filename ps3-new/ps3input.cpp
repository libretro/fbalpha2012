/******************************************************************************* 
 * ps3input.c - FBA Next Slim PS3
 *
 *  Created on: Aug 18, 2011
********************************************************************************/

#include "ps3input.h"

const char * Input_PrintMappedButton(uint32_t mappedbutton)
{
	switch(mappedbutton)
	{
		case BTN_FIRE1:
			return "Button Fire 1";
		case BTN_FIRE2:
			return "Button Fire 2";
		case BTN_FIRE3:
			return "Button Fire 3";
		case BTN_FIRE4:
			return "Button Fire 4";
		case BTN_FIRE5:
			return "Button Fire 5";
		case BTN_FIRE6:
			return "Button Fire 6";
		case BTN_START:
			return "Button Start";
		case BTN_COIN:
			return "Button Coin";
		case BTN_LEFT:
			return "D-Pad Left";
		case BTN_RIGHT:
			return "D-Pad Right";
		case BTN_UP:
			return "D-Pad Up";
		case BTN_DOWN:
			return "D-Pad Down";
		case BTN_NONE:
			return "None";
		case BTN_EXITTOMENU:
			return "Exit to menu";
		case BTN_QUICKSAVE:
			return "Save State";
		case BTN_QUICKLOAD:
			return "Load State";
		case BTN_INCREMENTSAVE:
			return "Increment state position";
		case BTN_DECREMENTSAVE:
			return "Decrement state position";
		case BTN_SOFTRESET:
			return "Software Reset";
		case BTN_HARDRESET:
			return "Reset";
		case BTN_FASTFORWARD:
			return "Fast forward";
		case BTN_INGAME_MENU:
			return "Ingame Menu";
		default:
			return "Unknown";
	}
}


//bool next: true is next, false is previous
uint32_t Input_GetAdjacentButtonmap(uint32_t buttonmap, uint32_t next)
{
	switch(buttonmap)
	{
		case BTN_UP:
			return next ? BTN_DOWN : BTN_NONE;
		case BTN_DOWN:
			return next ? BTN_LEFT : BTN_UP;
		case BTN_LEFT:
			return next ? BTN_RIGHT : BTN_DOWN;
		case BTN_RIGHT:
			return next ? BTN_FIRE1 : BTN_LEFT;
		case BTN_FIRE1:
			return next ? BTN_FIRE2 : BTN_RIGHT;
		case BTN_FIRE2:
			return next ? BTN_FIRE3 : BTN_FIRE1;
		case BTN_FIRE3:
			return next ? BTN_FIRE4 : BTN_FIRE2;
		case BTN_FIRE4:
			return next ? BTN_FIRE5 : BTN_FIRE3;
		case BTN_FIRE5:
			return next ? BTN_FIRE6 : BTN_FIRE4;
		case BTN_FIRE6:
			return next ? BTN_START : BTN_FIRE5;
		case BTN_START:
			return next ? BTN_COIN : BTN_FIRE6;
		case BTN_COIN:
			return next ? BTN_FASTFORWARD : BTN_START;
		case BTN_FASTFORWARD:
			return next ? BTN_HARDRESET : BTN_COIN;
		case BTN_HARDRESET:
			return next ? BTN_SOFTRESET : BTN_FASTFORWARD;
		case BTN_SOFTRESET:
			return next ? BTN_QUICKSAVE : BTN_HARDRESET;
		case BTN_QUICKSAVE:
			return next ? BTN_QUICKLOAD : BTN_SOFTRESET;
		case BTN_QUICKLOAD:
			return next ? BTN_EXITTOMENU : BTN_QUICKSAVE;
		case BTN_DECREMENTSAVE:
			return next ? BTN_INCREMENTSAVE : BTN_QUICKLOAD;
		case BTN_INCREMENTSAVE:
			return next ? BTN_EXITTOMENU : BTN_DECREMENTSAVE;
		case BTN_EXITTOMENU:
			return next ? BTN_INGAME_MENU : BTN_INCREMENTSAVE;
		case BTN_INGAME_MENU:
			return next ? BTN_NONE : BTN_EXITTOMENU;
		case BTN_NONE:
			return next ? BTN_UP : BTN_INGAME_MENU;
		default:
			return BTN_NONE;
	}
}
