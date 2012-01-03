// FB Alpha key definitions

#if 0//(defined _WIN32)

// Include dinput.h here to get the DIK_* macros
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

#define FBK_ESCAPE			DIK_ESCAPE
#define FBK_1				DIK_1
#define FBK_2				DIK_2
#define FBK_3				DIK_3
#define FBK_4				DIK_4
#define FBK_5				DIK_5
#define FBK_6				DIK_6
#define FBK_7				DIK_7
#define FBK_8				DIK_8
#define FBK_9				DIK_9
#define FBK_0				DIK_0
#define FBK_MINUS			DIK_MINUS			// - on main keyboard
#define FBK_EQUALS			DIK_EQUALS
#define FBK_BACK			DIK_BACK			// backspace
#define FBK_TAB				DIK_TAB
#define FBK_Q				DIK_Q
#define FBK_W				DIK_W
#define FBK_E				DIK_E
#define FBK_R				DIK_R
#define FBK_T				DIK_T
#define FBK_Y				DIK_Y
#define FBK_U				DIK_U
#define FBK_I				DIK_I
#define FBK_O				DIK_O
#define FBK_P				DIK_P
#define FBK_LBRACKET		DIK_LBRACKET
#define FBK_RBRACKET		DIK_RBRACKET
#define FBK_RETURN			DIK_RETURN			// Enter on main keyboard
#define FBK_LCONTROL		DIK_LCONTROL
#define FBK_A				DIK_A
#define FBK_S				DIK_S
#define FBK_D				DIK_D
#define FBK_F				DIK_F
#define FBK_G				DIK_G
#define FBK_H				DIK_H
#define FBK_J				DIK_J
#define FBK_K				DIK_K
#define FBK_L				DIK_L
#define FBK_SEMICOLON		DIK_SEMICOLON
#define FBK_APOSTROPHE		DIK_APOSTROPHE
#define FBK_GRAVE			DIK_GRAVE			// accent grave
#define FBK_LSHIFT			DIK_LSHIFT
#define FBK_BACKSLASH		DIK_BACKSLASH
#define FBK_Z				DIK_Z
#define FBK_X				DIK_X
#define FBK_C				DIK_C
#define FBK_V				DIK_V
#define FBK_B				DIK_B
#define FBK_N				DIK_N
#define FBK_M				DIK_M
#define FBK_COMMA			DIK_COMMA
#define FBK_PERIOD			DIK_PERIOD			// . on main keyboard
#define FBK_SLASH			DIK_SLASH			// / on main keyboard
#define FBK_RSHIFT			DIK_RSHIFT
#define FBK_MULTIPLY		DIK_MULTIPLY		// * on numeric keypad
#define FBK_LALT			DIK_LMENU			// left Alt
#define FBK_SPACE			DIK_SPACE
#define FBK_CAPITAL			DIK_CAPITAL
#define FBK_F1				DIK_F1
#define FBK_F2				DIK_F2
#define FBK_F3				DIK_F3
#define FBK_F4				DIK_F4
#define FBK_F5				DIK_F5
#define FBK_F6				DIK_F6
#define FBK_F7				DIK_F7
#define FBK_F8				DIK_F8
#define FBK_F9				DIK_F9
#define FBK_F10				DIK_F10
#define FBK_NUMLOCK			DIK_NUMLOCK
#define FBK_SCROLL			DIK_SCROLL			// Scroll Lock
#define FBK_NUMPAD7			DIK_NUMPAD7
#define FBK_NUMPAD8			DIK_NUMPAD8
#define FBK_NUMPAD9			DIK_NUMPAD9
#define FBK_SUBTRACT		DIK_SUBTRACT		// - on numeric keypad
#define FBK_NUMPAD4			DIK_NUMPAD4
#define FBK_NUMPAD5			DIK_NUMPAD5
#define FBK_NUMPAD6			DIK_NUMPAD6
#define FBK_ADD				DIK_ADD				// + on numeric keypad
#define FBK_NUMPAD1			DIK_NUMPAD1
#define FBK_NUMPAD2			DIK_NUMPAD2
#define FBK_NUMPAD3			DIK_NUMPAD3
#define FBK_NUMPAD0			DIK_NUMPAD0
#define FBK_DECIMAL			DIK_DECIMAL			// . on numeric keypad
#define FBK_OEM_102			DIK_OEM_102			// < > | on UK/Germany keyboards
#define FBK_F11				DIK_F11
#define FBK_F12				DIK_F12

#define FBK_F13				DIK_F13				//						(NEC PC98)
#define FBK_F14				DIK_F14				//						(NEC PC98)
#define FBK_F15				DIK_F15				//						(NEC PC98)

#define FBK_KANA			DIK_KANA			// (Japanese keyboard)
#define FBK_ABNT_C1			DIK_ABNT_C1			// / ? on Portugese (Brazilian) keyboards
#define FBK_CONVERT			DIK_CONVERT			// (Japanese keyboard)
#define FBK_NOCONVERT		DIK_NOCONVERT		// (Japanese keyboard)
#define FBK_YEN				DIK_YEN				// (Japanese keyboard)
#define FBK_ABNT_C2			DIK_ABNT_C2			// Numpad . on Portugese (Brazilian) keyboards
#define FBK_NUMPADEQUALS	DIK_NUMPADEQUALS	// = on numeric keypad	(NEC PC98)
#define FBK_PREVTRACK		DIK_PREVTRACK		// Previous Track (DIK_CIRCUMFLEX on Japanese keyboard)
#define FBK_AT				DIK_AT				//						(NEC PC98)
#define FBK_COLON			DIK_COLON			//						(NEC PC98)
#define FBK_UNDERLINE		DIK_UNDERLINE		//						(NEC PC98)
#define FBK_KANJI			DIK_KANJI			// (Japanese keyboard)
#define FBK_STOP			DIK_STOP			//						(NEC PC98)
#define FBK_AX				DIK_AX				//						(Japan AX)
#define FBK_UNLABELED		DIK_UNLABELED		//						(J3100)
#define FBK_NEXTTRACK		DIK_NEXTTRACK		// Next Track
#define FBK_NUMPADENTER		DIK_NUMPADENTER		// Enter on numeric keypad
#define FBK_RCONTROL		DIK_RCONTROL
#define FBK_MUTE			DIK_MUTE			// Mute
#define FBK_CALCULATOR		DIK_CALCULATOR		// Calculator
#define FBK_PLAYPAUSE		DIK_PLAYPAUSE		// Play / Pause
#define FBK_MEDIASTOP		DIK_MEDIASTOP		// Media Stop
#define FBK_VOLUMEDOWN		DIK_VOLUMEDOWN		// Volume -
#define FBK_VOLUMEUP		DIK_VOLUMEUP		// Volume +
#define FBK_WEBHOME			DIK_WEBHOME			// Web home
#define FBK_NUMPADCOMMA		DIK_NUMPADCOMMA		// , on numeric keypad (NEC PC98)
#define FBK_DIVIDE			DIK_DIVIDE			// / on numeric keypad
#define FBK_SYSRQ			DIK_SYSRQ
#define FBK_RALT			DIK_RMENU			// right Alt
#define FBK_PAUSE			DIK_PAUSE			// Pause
#define FBK_HOME			DIK_HOME			// Home on arrow keypad
#define FBK_UPARROW			DIK_UP				// UpArrow on arrow keypad
#define FBK_PRIOR			DIK_PRIOR			// PgUp on arrow keypad
#define FBK_LEFTARROW		DIK_LEFT			// LeftArrow on arrow keypad
#define FBK_RIGHTARROW		DIK_RIGHT			// RightArrow on arrow keypad
#define FBK_END				DIK_END				// End on arrow keypad
#define FBK_DOWNARROW		DIK_DOWN			// DownArrow on arrow keypad
#define FBK_NEXT			DIK_NEXT			// PgDn on arrow keypad
#define FBK_INSERT			DIK_INSERT			// Insert on arrow keypad
#define FBK_DELETE			DIK_DELETE			// Delete on arrow keypad
#define FBK_LWIN			DIK_LWIN			// Left Windows key
#define FBK_RWIN			DIK_RWIN			// Right Windows key
#define FBK_APPS			DIK_APPS			// AppMenu key
#define FBK_POWER			DIK_POWER			// System Power
#define FBK_SLEEP			DIK_SLEEP			// System Sleep
#define FBK_WAKE			DIK_WAKE			// System Wake
#define FBK_WEBSEARCH		DIK_WEBSEARCH		// Web Search
#define FBK_WEBFAVORITES	DIK_WEBFAVORITES	// Web Favorites
#define FBK_WEBREFRESH		DIK_WEBREFRESH		// Web Refresh
#define FBK_WEBSTOP			DIK_WEBSTOP			// Web Stop
#define FBK_WEBFORWARD		DIK_WEBFORWARD		// Web Forward
#define FBK_WEBBACK			DIK_WEBBACK			// Web Back
#define FBK_MYCOMPUTER		DIK_MYCOMPUTER		// My Computer
#define FBK_MAIL			DIK_MAIL			// Mail
#define FBK_MEDIASELECT		DIK_MEDIASELECT		// Media Select

#else	// (defined _WIN32)

#define FBK_ESCAPE          0x01
#define FBK_1               0x02
#define FBK_2               0x03
#define FBK_3               0x04
#define FBK_4               0x05
#define FBK_5               0x06
#define FBK_6               0x07
#define FBK_7               0x08
#define FBK_8               0x09
#define FBK_9               0x0A
#define FBK_0               0x0B
#define FBK_MINUS           0x0C				// - on main keyboard
#define FBK_EQUALS          0x0D
#define FBK_BACK            0x0E				// backspace
#define FBK_TAB             0x0F
#define FBK_Q               0x10
#define FBK_W               0x11
#define FBK_E               0x12
#define FBK_R               0x13
#define FBK_T               0x14
#define FBK_Y               0x15
#define FBK_U               0x16
#define FBK_I               0x17
#define FBK_O               0x18
#define FBK_P               0x19
#define FBK_LBRACKET        0x1A
#define FBK_RBRACKET        0x1B
#define FBK_RETURN          0x1C				// Enter on main keyboard
#define FBK_LCONTROL        0x1D
#define FBK_A               0x1E
#define FBK_S               0x1F
#define FBK_D               0x20
#define FBK_F               0x21
#define FBK_G               0x22
#define FBK_H               0x23
#define FBK_J               0x24
#define FBK_K               0x25
#define FBK_L               0x26
#define FBK_SEMICOLON       0x27
#define FBK_APOSTROPHE      0x28
#define FBK_GRAVE           0x29				// accent grave
#define FBK_LSHIFT          0x2A
#define FBK_BACKSLASH       0x2B
#define FBK_Z               0x2C
#define FBK_X               0x2D
#define FBK_C               0x2E
#define FBK_V               0x2F
#define FBK_B               0x30
#define FBK_N               0x31
#define FBK_M               0x32
#define FBK_COMMA           0x33
#define FBK_PERIOD          0x34				// . on main keyboard
#define FBK_SLASH           0x35				// / on main keyboard
#define FBK_RSHIFT          0x36
#define FBK_MULTIPLY        0x37				// * on numeric keypad
#define FBK_LALT            0x38				// left Alt
#define FBK_SPACE           0x39
#define FBK_CAPITAL         0x3A
#define FBK_F1              0x3B
#define FBK_F2              0x3C
#define FBK_F3              0x3D
#define FBK_F4              0x3E
#define FBK_F5              0x3F
#define FBK_F6              0x40
#define FBK_F7              0x41
#define FBK_F8              0x42
#define FBK_F9              0x43
#define FBK_F10             0x44
#define FBK_NUMLOCK         0x45
#define FBK_SCROLL          0x46				// Scroll Lock
#define FBK_NUMPAD7         0x47
#define FBK_NUMPAD8         0x48
#define FBK_NUMPAD9         0x49
#define FBK_SUBTRACT        0x4A				// - on numeric keypad
#define FBK_NUMPAD4         0x4B
#define FBK_NUMPAD5         0x4C
#define FBK_NUMPAD6         0x4D
#define FBK_ADD             0x4E				// + on numeric keypad
#define FBK_NUMPAD1         0x4F
#define FBK_NUMPAD2         0x50
#define FBK_NUMPAD3         0x51
#define FBK_NUMPAD0         0x52
#define FBK_DECIMAL         0x53				// . on numeric keypad
#define FBK_OEM_102         0x56				// < > | on UK/Germany keyboards
#define FBK_F11             0x57
#define FBK_F12             0x58

#define FBK_F13             0x64				//						(NEC PC98)
#define FBK_F14             0x65				//						(NEC PC98)
#define FBK_F15             0x66				//						(NEC PC98)

#define FBK_KANA            0x70				// (Japanese keyboard)
#define FBK_ABNT_C1         0x73				// / ? on Portugese (Brazilian) keyboards
#define FBK_CONVERT         0x79				// (Japanese keyboard)
#define FBK_NOCONVERT       0x7B				// (Japanese keyboard)
#define FBK_YEN             0x7D				// (Japanese keyboard)
#define FBK_ABNT_C2         0x7E				// Numpad . on Portugese (Brazilian) keyboards
#define FBK_NUMPADEQUALS    0x8D				// = on numeric keypad  (NEC PC98)
#define FBK_PREVTRACK       0x90				// Previous Track (FBK_CIRCUMFLEX on Japanese keyboard)
#define FBK_AT              0x91				//						(NEC PC98)
#define FBK_COLON           0x92				//						(NEC PC98)
#define FBK_UNDERLINE       0x93				//						(NEC PC98)
#define FBK_KANJI           0x94				// (Japanese keyboard)
#define FBK_STOP            0x95				//						(NEC PC98)
#define FBK_AX              0x96				//						(Japan AX)
#define FBK_UNLABELED       0x97				//						(J3100)
#define FBK_NEXTTRACK       0x99				// Next Track
#define FBK_NUMPADENTER     0x9C				// Enter on numeric keypad
#define FBK_RCONTROL        0x9D
#define FBK_MUTE            0xA0				// Mute
#define FBK_CALCULATOR      0xA1				// Calculator
#define FBK_PLAYPAUSE       0xA2				// Play / Pause
#define FBK_MEDIASTOP       0xA4				// Media Stop
#define FBK_VOLUMEDOWN      0xAE				// Volume -
#define FBK_VOLUMEUP        0xB0				// Volume +
#define FBK_WEBHOME         0xB2				// Web home
#define FBK_NUMPADCOMMA     0xB3				// , on numeric keypad (NEC PC98)
#define FBK_DIVIDE          0xB5				// / on numeric keypad
#define FBK_SYSRQ           0xB7
#define FBK_RALT            0xB8				// right Alt
#define FBK_PAUSE           0xC5				// Pause
#define FBK_HOME            0xC7				// Home on arrow keypad
#define FBK_UPARROW         0xC8				// UpArrow on arrow keypad
#define FBK_PRIOR           0xC9				// PgUp on arrow keypad
#define FBK_LEFTARROW       0xCB				// LeftArrow on arrow keypad
#define FBK_RIGHTARROW      0xCD				// RightArrow on arrow keypad
#define FBK_END             0xCF				// End on arrow keypad
#define FBK_DOWNARROW       0xD0				// DownArrow on arrow keypad
#define FBK_NEXT            0xD1				// PgDn on arrow keypad
#define FBK_INSERT          0xD2				// Insert on arrow keypad
#define FBK_DELETE          0xD3				// Delete on arrow keypad
#define FBK_LWIN            0xDB				// Left Windows key
#define FBK_RWIN            0xDC				// Right Windows key
#define FBK_APPS            0xDD				// AppMenu key
#define FBK_POWER           0xDE				// System Power
#define FBK_SLEEP           0xDF				// System Sleep
#define FBK_WAKE            0xE3				// System Wake
#define FBK_WEBSEARCH       0xE5				// Web Search
#define FBK_WEBFAVORITES    0xE6				// Web Favorites
#define FBK_WEBREFRESH      0xE7				// Web Refresh
#define FBK_WEBSTOP         0xE8				// Web Stop
#define FBK_WEBFORWARD      0xE9				// Web Forward
#define FBK_WEBBACK         0xEA				// Web Back
#define FBK_MYCOMPUTER      0xEB				// My Computer
#define FBK_MAIL            0xEC				// Mail
#define FBK_MEDIASELECT     0xED				// Media Select


#define XBOX_LEFT_TRIGGER	0x88
#define XBOX_RIGHT_TRIGGER	0x8A

#define PS3_L2_BUTTON		0x88
#define PS3_R2_BUTTON		0x8A

#define PS3_L3_BUTTON		0x8B
#define PS3_R3_BUTTON		0x8C


#endif	// (defined _WIN32)
