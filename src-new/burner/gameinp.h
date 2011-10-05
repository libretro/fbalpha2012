
struct giConstant {
	unsigned char nConst;				// The constant value
};

struct giSwitch {
	unsigned short nCode;				// The input code (for digital)
};

struct giJoyAxis {
	unsigned char nJoy;					// The joystick number
	unsigned char nAxis;	   			// The joystick axis number
};

struct giMouseAxis {
	unsigned char nMouse;				// The mouse number
	unsigned char nAxis;				// The axis number
	unsigned short nOffset;				// Used for absolute axes
};

struct giSliderAxis {
	unsigned short nSlider[2];			// Keys to use for slider
};

struct giSlider {
	union {
		struct giJoyAxis JoyAxis;
		struct giSliderAxis SliderAxis;
	};
	short nSliderSpeed;					// speed with which keys move the slider
	short nSliderCenter;				// Speed the slider should center itself (high value = slow)
	int nSliderValue;					// Current position of the slider
};

struct giInput {
	union {								// Destination for the Input Value
		unsigned char* pVal;
		unsigned short* pShortVal;
	};
	unsigned short nVal;				// The Input Value

	union {
		struct giConstant Constant;
		struct giSwitch Switch;
		struct giJoyAxis JoyAxis;
		struct giMouseAxis MouseAxis;
		struct giSlider Slider;
	};
};

struct giForce {
	unsigned char nInput;				// The input to apply force feedback efects to
	unsigned char nEffect;				// The effect to use
};

struct giMacro {
	unsigned char nMode;				// 0 = Unused, 1 = used

	unsigned char* pVal[4];				// Destination for the Input Value
	unsigned char nVal[4];				// The Input Value
	unsigned char nInput[4];			// Which inputs are mapped

	struct giSwitch Switch;

	char szName[17];					// Maximum name length 16 chars
};

#define GIT_CONSTANT		(0x01)
#define GIT_SWITCH			(0x02)

#define GIT_GROUP_SLIDER	(0x08)
#define GIT_KEYSLIDER		(0x08)
#define GIT_JOYSLIDER		(0x09)

#define GIT_GROUP_MOUSE		(0x10)
#define GIT_MOUSEAXIS		(0x10)

#define GIT_GROUP_JOYSTICK	(0x20)
#define GIT_JOYAXIS_FULL	(0x20)
#define GIT_JOYAXIS_NEG		(0x21)
#define GIT_JOYAXIS_POS		(0x22)

#define GIT_FORCE			(0x40)

#define GIT_GROUP_MACRO		(0x80)
#define GIT_MACRO_AUTO		(0x80)
#define GIT_MACRO_CUSTOM	(0x81)

struct GameInp {
	unsigned char nInput;				// PC side: see above
	unsigned char nType;				// game side: see burn.h

	union {
		struct giInput Input;
		struct giForce Force;
		struct giMacro Macro;
	};
};

