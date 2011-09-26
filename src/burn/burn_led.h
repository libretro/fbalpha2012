
#define LED_COLOR_RED			0xff0000
#define LED_COLOR_GREEN			0x00ff00
#define LED_COLOR_BLUE			0x0000ff
#define LED_COLOR_WHITE			0xffffff
#define LED_COLOR_YELLOW		0xffff00

#define LED_SIZE_2x2			2
#define LED_SIZE_3x3			3
#define LED_SIZE_4x4			4
#define LED_SIZE_5x5			5
#define LED_SIZE_6x6			6
#define LED_SIZE_7x7			7
#define LED_SIZE_8x8			8

#define LED_POSITION_TOP_LEFT		0
#define LED_POSITION_TOP_RIGHT		1
#define LED_POSITION_BOTTOM_LEFT	2
#define LED_POSITION_BOTTOM_RIGHT	3

// transparency is a percentage 0 - 100
void BurnLEDInit(int num, int position, int size, int color, int transparency);

void BurnLEDReset();
void BurnLEDSetStatus(int led, unsigned int status);
void BurnLEDSetFlipscreen(int flip);
void BurnLEDRender();
void BurnLEDExit();

int BurnLEDScan(int nAction, int *pnMin);
