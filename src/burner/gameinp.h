#if 0
struct giConstant {
	unsigned char nConst;				// The constant value
};

struct giSwitch {
	unsigned short nCode;				// The input code (for digital)
};
#endif

#ifdef USE_JOYAXIS
struct giJoyAxis {
	unsigned char nJoy;				// The joystick number
	unsigned char nAxis;	   			// The joystick axis number
};
#endif

#ifdef USE_MOUSE
struct giMouseAxis {
	unsigned char nMouse;				// The mouse number
	unsigned char nAxis;				// The axis number
	unsigned short nOffset;				// Used for absolute axes
};
#endif

#if 0
struct giSliderAxis {
	unsigned short nSlider[2];			// Keys to use for slider
};
#endif

struct giSlider {
	#ifdef USE_JOYAXIS
	union {
		struct giJoyAxis JoyAxis;
		unsigned short SliderAxis[2];
	};
	#else
	unsigned short SliderAxis[2];
	#endif
	short nSliderSpeed;				// speed with which keys move the slider
	short nSliderCenter;				// Speed the slider should center itself (high value = slow)
	int nSliderValue;				// Current position of the slider
};

struct giInput {
	union {						// Destination for the Input Value
		unsigned char* pVal;
		unsigned short* pShortVal;
	};
	unsigned short nVal;				// The Input Value

	union {
		unsigned char Constant;
		unsigned short Switch;
		#ifdef USE_JOYAXIS
		struct giJoyAxis JoyAxis;
		#endif
		#ifdef USE_MOUSE
		struct giMouseAxis MouseAxis;
		#endif
		struct giSlider Slider;
	};
};

#ifdef USE_FORCEFEEDBACK
struct giForce {
	unsigned char nInput;				// The input to apply force feedback efects to
	unsigned char nEffect;				// The effect to use
};
#endif

#ifdef USE_MACROS
struct giMacro {
	unsigned char nMode;				// 0 = Unused, 1 = used

	unsigned char* pVal[4];				// Destination for the Input Value
	unsigned char nVal[4];				// The Input Value
	unsigned char nInput[4];			// Which inputs are mapped

	unsigned short Switch;

	char szName[17];				// Maximum name length 16 chars
};
#endif

#define GIT_CONSTANT		(0x01)
#define GIT_SWITCH		(0x02)

#define GIT_GROUP_SLIDER	(0x08)
#define GIT_KEYSLIDER		(0x08)
#define GIT_JOYSLIDER		(0x09)

#define GIT_GROUP_MOUSE		(0x10)
#define GIT_MOUSEAXIS		(0x10)

#define GIT_GROUP_JOYSTICK	(0x20)
#define GIT_JOYAXIS_FULL	(0x20)
#define GIT_JOYAXIS_NEG		(0x21)
#define GIT_JOYAXIS_POS		(0x22)

#define GIT_FORCE		(0x40)

#define GIT_GROUP_MACRO		(0x80)
#define GIT_MACRO_AUTO		(0x80)
#define GIT_MACRO_CUSTOM	(0x81)

struct GameInp {
	unsigned char nInput;				// PC side: see above
	unsigned char nType;				// game side: see burn.h

	#if defined(USE_FORCE_FEEDBACK) || defined(USE_MACROS)
	union {
		struct giInput Input;
		#ifdef USE_FORCEFEEDBACK
		struct giForce Force;
		#endif
		#ifdef USE_MACROS
		struct giMacro Macro;
		#endif
	};
	#else
	struct giInput Input;
	#endif
};

