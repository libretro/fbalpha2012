// Tiger Heli, Get Star / Guardian, & Slap Fight

#include "burnint.h"
#include "taito_m68705.h"
#include "driver.h"
extern "C" {
 #include "ay8910.h"
}

static int nWhichGame;

static bool bInterruptEnable;
static bool bSoundCPUEnable;
static bool bSoundNMIEnable;

static int nStatusIndex;
static int nProtectIndex;

// ---------------------------------------------------------------------------
// Inputs

static unsigned char tigerhInput[4] = {0,0,0,0};
static unsigned char tigerhInpJoy1[8];
static unsigned char tigerhInpMisc[8];
static unsigned char tigerhReset = 0;

// Dip Switch and Input Definitions
static struct BurnInputInfo tigerhInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	tigerhInpMisc + 6,	"p1 coin"},
	{"P1 Start",	BIT_DIGITAL,	tigerhInpMisc + 4,	"p1 start"},
	{"P1 Up",		BIT_DIGITAL,	tigerhInpJoy1 + 0,	"p1 up"},
	{"P1 Down",		BIT_DIGITAL,	tigerhInpJoy1 + 1,	"p1 down"},
	{"P1 Left",		BIT_DIGITAL,	tigerhInpJoy1 + 3,	"p1 left"},
	{"P1 Right",	BIT_DIGITAL,	tigerhInpJoy1 + 2,	"p1 right"},
	{"P1 Button 1",	BIT_DIGITAL,	tigerhInpMisc + 1,	"p1 fire 1"},
	{"P1 Button 2",	BIT_DIGITAL,	tigerhInpMisc + 0,	"p1 fire 2"},

	{"P2 Coin",		BIT_DIGITAL,	tigerhInpMisc + 7,	"p2 coin"},
	{"P2 Start",	BIT_DIGITAL,	tigerhInpMisc + 5,	"p2 start"},
	{"P2 Up",		BIT_DIGITAL,	tigerhInpJoy1 + 4,	"p2 up"},
	{"P2 Down",		BIT_DIGITAL,	tigerhInpJoy1 + 5,	"p2 down"},
	{"P2 Left",		BIT_DIGITAL,	tigerhInpJoy1 + 7,	"p2 left"},
	{"P2 Right",	BIT_DIGITAL,	tigerhInpJoy1 + 6,	"p2 right"},
	{"P2 Button 1",	BIT_DIGITAL,	tigerhInpMisc + 3,	"p2 fire 1"},
	{"P2 Button 2",	BIT_DIGITAL,	tigerhInpMisc + 2,	"p2 fire 2"},

	{"Reset",		BIT_DIGITAL,	&tigerhReset,		"reset"},

	{"Dip A",		BIT_DIPSWITCH,	tigerhInput + 2,	"dip"},
	{"Dip B",		BIT_DIPSWITCH,	tigerhInput + 3,	"dip"},
};

STDINPUTINFO(tigerh)

static struct BurnInputInfo gtstarbaInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	tigerhInpMisc + 6,	"p1 coin"},
	{"P1 Start",	BIT_DIGITAL,	tigerhInpMisc + 4,	"p1 start"},
	{"P1 Up",		BIT_DIGITAL,	tigerhInpJoy1 + 0,	"p1 up"},
	{"P1 Down",		BIT_DIGITAL,	tigerhInpJoy1 + 2,	"p1 down"},
	{"P1 Left",		BIT_DIGITAL,	tigerhInpJoy1 + 3,	"p1 left"},
	{"P1 Right",	BIT_DIGITAL,	tigerhInpJoy1 + 1,	"p1 right"},
	{"P1 Button 1",	BIT_DIGITAL,	tigerhInpMisc + 1,	"p1 fire 1"},
	{"P1 Button 2",	BIT_DIGITAL,	tigerhInpMisc + 0,	"p1 fire 2"},

	{"P2 Coin",		BIT_DIGITAL,	tigerhInpMisc + 7,	"p2 coin"},
	{"P2 Start",	BIT_DIGITAL,	tigerhInpMisc + 5,	"p2 start"},
	{"P2 Up",		BIT_DIGITAL,	tigerhInpJoy1 + 4,	"p2 up"},
	{"P2 Down",		BIT_DIGITAL,	tigerhInpJoy1 + 6,	"p2 down"},
	{"P2 Left",		BIT_DIGITAL,	tigerhInpJoy1 + 7,	"p2 left"},
	{"P2 Right",	BIT_DIGITAL,	tigerhInpJoy1 + 5,	"p2 right"},
	{"P2 Button 1",	BIT_DIGITAL,	tigerhInpMisc + 3,	"p2 fire 1"},
	{"P2 Button 2",	BIT_DIGITAL,	tigerhInpMisc + 2,	"p2 fire 2"},

	{"Reset",		BIT_DIGITAL,	&tigerhReset,		"reset"},

	{"Dip A",		BIT_DIPSWITCH,	tigerhInput + 2,	"dip"},
	{"Dip B",		BIT_DIPSWITCH,	tigerhInput + 3,	"dip"},
};

STDINPUTINFO(gtstarba)

static struct BurnDIPInfo tigerhDIPList[] = {
	// Defaults
	{0x11,	0xFF, 0xFF,	0x10, NULL},
	{0x12,	0xFF, 0xFF,	0x00, NULL},

	// DIP A
	{0,		0xFE, 0,	2,	  "Hero speed"},
	{0x11,	0x01, 0x80,	0x00, "Normal"},
	{0x11,	0x01, 0x80,	0x80, "Fast"},
	{0,		0xFE, 0,	2,	  NULL},
	{0x11,	0x01, 0x40,	0x00, "Normal game"},
	{0x11,	0x01, 0x40,	0x40, "DIP switch test"},
	{0,		0xFE, 0,	2,	  NULL},
	{0x11,	0x01, 0x20,	0x00, "Normal screen"},
	{0x11,	0x01, 0x20,	0x20, "Invert screen"},
	{0,		0xFE, 0,	2,	  "Cabinet"},
	{0x11,	0x01, 0x10,	0x10, "Upright"},
	{0x11,	0x01, 0x10,	0x00, "Table"},
	{0,		0xFE, 0,	2,	  "Advertise sound"},
	{0x11,	0x01, 0x08,	0x00, "On"},
	{0x11,	0x01, 0x08,	0x08, "Off"},
	{0,		0xFE, 0,	7,	  "Coinage"},
	{0x11,	0x01, 0x07,	0x00, "1 coin 1 credit"},
	{0x11,	0x01, 0x07,	0x01, "1 coin 2 credits"},
	{0x11,	0x01, 0x07,	0x02, "1 coin 3 credits"},
	{0x11,	0x01, 0x07,	0x03, "2 coins 1 credit"},
	{0x11,	0x01, 0x07,	0x04, "2 coins 3 credits"},
	{0x11,	0x01, 0x07,	0x05, "3 coins 1 credit"},
	{0x11,	0x01, 0x07,	0x07, "Free play"},
	{0x11,	0x01, 0x07,	0x06, "3 coins 1 credit"},

	// DIP B
	{0,		0xFE, 0,	2,	  "Extend"},
	{0x12,	0x01, 0x10,	0x00, "20000 & 80000"},
	{0x12,	0x01, 0x10,	0x10, "50000 & 120000"},
	{0,		0xFE, 0,	4,	  "Game difficulty"},
	{0x12,	0x01, 0x0C,	0x00, "0"},
	{0x12,	0x01, 0x0C,	0x04, "1"},
	{0x12,	0x01, 0x0C,	0x08, "2"},
	{0x12,	0x01, 0x0C,	0x0C, "3"},
	{0,		0xFE, 0,	4,	  "Number of heroes"},
	{0x12,	0x01, 0x03,	0x00, "3"},
	{0x12,	0x01, 0x03,	0x01, "5"},
	{0x12,	0x01, 0x03,	0x02, "1"},
	{0x12,	0x01, 0x03,	0x03, "2"},
};

STDDIPINFO(tigerh)

static struct BurnDIPInfo getstarDIPList[] = {
	// Defaults
	{0x11,	0xFF, 0xFF,	0x10, NULL},

	// DIP A
	{0,		0xFE, 0,	2,	  "Hero speed"}, // really work?
	{0x11,	0x01, 0x80,	0x00, "Normal"},
	{0x11,	0x01, 0x80,	0x80, "Fast"},
	{0,		0xFE, 0,	2,	  NULL},
	{0x11,	0x01, 0x40,	0x00, "Normal game"},
	{0x11,	0x01, 0x40,	0x40, "DIP switch test"},
	{0,		0xFE, 0,	2,	  NULL},
	{0x11,	0x01, 0x20,	0x00, "Normal screen"},
	{0x11,	0x01, 0x20,	0x20, "Invert screen"},
	{0,		0xFE, 0,	2,	  "Cabinet"},
	{0x11,	0x01, 0x10,	0x10, "Upright"},
	{0x11,	0x01, 0x10,	0x00, "Table"},
	{0,		0xFE, 0,	2,	  "Advertise sound"},
	{0x11,	0x01, 0x08,	0x00, "On"},
	{0x11,	0x01, 0x08,	0x08, "Off"},
	{0,		0xFE, 0,	7,	  "Coinage"},
	{0x11,	0x01, 0x07,	0x00, "1 coin 1 credit"},
	{0x11,	0x01, 0x07,	0x01, "1 coin 2 credits"},
	{0x11,	0x01, 0x07,	0x02, "1 coin 3 credits"},
	{0x11,	0x01, 0x07,	0x03, "2 coins 1 credit"},
	{0x11,	0x01, 0x07,	0x04, "2 coins 3 credits"},
	{0x11,	0x01, 0x07,	0x05, "3 coins 1 credit"},
	{0x11,	0x01, 0x07,	0x07, "Free play"},
	{0x11,	0x01, 0x07,	0x06, "3 coins 1 credit"},

	// DIP B
	{0,		0xFE, 0,	2,	  "Extend"},
	{0x12,	0x01, 0x10,	0x00, "30000 & 100000"},
	{0x12,	0x01, 0x10,	0x10, "50000 & 150000"},
	{0,		0xFE, 0,	4,	  "Game difficulty"},
	{0x12,	0x01, 0x0C,	0x00, "0"},
	{0x12,	0x01, 0x0C,	0x04, "1"},
	{0x12,	0x01, 0x0C,	0x08, "2"},
	{0x12,	0x01, 0x0C,	0x0C, "3"},
};

static struct BurnDIPInfo getstarHeroesDIPList[] =
{
	// Defaults
	{0x12,	0xFF, 0xFF,	0x01, NULL},

	// DIP B
	{0,		0xFE, 0,	4,	  "Number of heroes"},
	{0x12,	0x01, 0x03,	0x01, "3"},
	{0x12,	0x01, 0x03,	0x02, "4"},
	{0x12,	0x01, 0x03,	0x03, "5"},
	{0x12,	0x01, 0x03,	0x00, "240 (cheat)"},
};

static struct BurnDIPInfo gtstarbaHeroesDIPList[] =
{
	// Defaults
	{0x12,	0xFF, 0xFF,	0x00, NULL},

	// DIP B
	{0,		0xFE, 0,	4,	  "Number of heroes"},
	{0x12,	0x01, 0x03,	0x02, "1"},
	{0x12,	0x01, 0x03,	0x03, "2"},
	{0x12,	0x01, 0x03,	0x00, "3"},
	{0x12,	0x01, 0x03,	0x01, "5"},
};

STDDIPINFOEXT(getstar, getstar, getstarHeroes)
STDDIPINFOEXT(gtstarba, getstar, gtstarbaHeroes)

static struct BurnDIPInfo slapfighDIPList[] = {
	// Defaults
	{0x11,	0xFF, 0xFF,	0x80, NULL},
	{0x12,	0xFF, 0xFF,	0x00, NULL},

	// DIP A
	{0,		0xFE, 0,	2,	  NULL},
	{0x11,	0x01, 0x80,	0x80, "Up-right type"},
	{0x11,	0x01, 0x80,	0x00, "Table type"},
	{0,		0xFE, 0,	2,	  NULL},
	{0x11,	0x01, 0x40,	0x00, "Normal screen"},
	{0x11,	0x01, 0x40,	0x40, "Invert screen"},
	{0,		0xFE, 0,	2,	  NULL},
	{0x11,	0x01, 0x20,	0x00, "Normal game"},
	{0x11,	0x01, 0x20,	0x20, "Screen test mode"},
	{0,		0xFE, 0,	2,	  "Advertise sound"},
	{0x11,	0x01, 0x10,	0x00, "On"},
	{0x11,	0x01, 0x10,	0x10, "Off"},
	{0,		0xFE, 0,	4,	  "Coin A"},
	{0x11,	0x01, 0x0C,	0x00, "1 coin 1 play"},
	{0x11,	0x01, 0x0C,	0x08, "1 coin 2 plays"},
	{0x11,	0x01, 0x0C,	0x04, "2 coins 1 play"},
	{0x11,	0x01, 0x0C,	0x0C, "2 coins 3 plays"},
	{0,		0xFE, 0,	4,	  "Coin B"},
	{0x11,	0x01, 0x03,	0x00, "1 coin 1 play"},
	{0x11,	0x01, 0x03,	0x02, "1 coin 2 plays"},
	{0x11,	0x01, 0x03,	0x01, "2 coins 1 play"},
	{0x11,	0x01, 0x03,	0x03, "2 coins 3 plays"},

	// DIP B
	{0,		0xFE, 0,	4,	  "Game difficulty"},
	{0x12,	0x01, 0xC0,	0x00, "B"},
	{0x12,	0x01, 0xC0,	0x80, "A"},
	{0x12,	0x01, 0xC0,	0x40, "C"},
	{0x12,	0x01, 0xC0,	0xC0, "D"},
	{0,		0xFE, 0,	4,	  "Extend"},
	{0x12,	0x01, 0x30,	0x00, "30000, 100000"},
	{0x12,	0x01, 0x30,	0x20, "50000, 200000"},
	{0x12,	0x01, 0x30,	0x10, "50000 only"},
	{0x12,	0x01, 0x30,	0x30, "100000 only"},
	{0,		0xFE, 0,	4,	  "Fighter counts"},
	{0x12,	0x01, 0x0C,	0x00, "3"},
	{0x12,	0x01, 0x0C,	0x08, "5"},
	{0x12,	0x01, 0x0C,	0x04, "1"},
	{0x12,	0x01, 0x0C,	0x0C, "2"},
	{0,		0xFE, 0,	2,	  NULL},
	{0x12,	0x01, 0x02,	0x00, "Normal game"},
	{0x12,	0x01, 0x02,	0x02, "DIP-switch display"},
};

STDDIPINFO(slapfigh)

// ---------------------------------------------------------------------------

static unsigned char *Mem, *MemEnd, *RamStart, *RamEnd;

static unsigned char *Rom01, *Rom02, *Rom03;
static unsigned char *TigerHeliTileROM, *TigerHeliSpriteROM, *TigerHeliTextROM;

static unsigned char *Ram01, *RamShared, *Ram03;
static unsigned char *TigerHeliTileRAM, *TigerHeliSpriteRAM, *TigerHeliSpriteBuf, *TigerHeliTextRAM;

static unsigned char* TigerHeliPaletteROM;
static unsigned int* TigerHeliPalette;

static short* pFMBuffer;
static short* pAY8910Buffer[6];

static int use_mcu = 0;

static int MemIndex()
{
	unsigned char* Next; Next = Mem;
	Rom01				= Next; Next += 0x012000;		// Z80 main program
	Rom02				= Next; Next += 0x002000;		// Z80 sound program
	Rom03				= Next; Next += 0x000800;		// m68705 mcu program
	TigerHeliTextROM	= Next; Next += 0x010000;
	TigerHeliSpriteROM	= Next; Next += 0x040000;
	TigerHeliTileROM	= Next; Next += 0x040000;
	RamStart			= Next;
	Ram01				= Next; Next += 0x000800;		// Z80 main work RAM
	RamShared			= Next; Next += 0x000800;		// Shared RAM
	TigerHeliTextRAM	= Next; Next += 0x001000;
	TigerHeliSpriteRAM	= Next; Next += 0x000800;
	TigerHeliSpriteBuf	= Next; Next += 0x000800;
	TigerHeliTileRAM	= Next; Next += 0x001000;
	Ram03			= Next; Next += 0x000080;
	RamEnd				= Next;
	pFMBuffer			= (short*)Next; Next += nBurnSoundLen * 6 * sizeof(short);
	TigerHeliPaletteROM	= Next; Next += 0x000300;
	TigerHeliPalette	= (unsigned int*)Next; Next += 0x000100 * sizeof(int);
	MemEnd				= Next;

	return 0;
}

// ---------------------------------------------------------------------------
//	Graphics

static int nTigerHeliTileXPosLo, nTigerHeliTileXPosHi, nTigerHeliTileYPosLo;
static int nTigerHeliTileMask;

static unsigned char* pTileData;

static int nTileNumber;
static int nTilePalette;

static unsigned short* pTileRow;
static unsigned short* pTile;

static int nTileXPos, nTileYPos;

// ---------------------------------------------------------------------------
//	Palette

static unsigned char tigerhRecalcPalette = 0;

static void TigerHeliPaletteInit()
{
	for (int i = 0; i < 0x0100; i++) {
		int r, g, b;

		r = TigerHeliPaletteROM[i + 0x0000];	  // Red
		r |= r << 4;
		g = TigerHeliPaletteROM[i + 0x0100];	  // Green
		g |= g << 4;
		b = TigerHeliPaletteROM[i + 0x0200];	  // Blue
		b |= b << 4;

		TigerHeliPalette[i] = BurnHighCol(r, g, b, 0);
	}

	return;
}

static void TigerHeliPaletteUpdate()
{
	if (tigerhRecalcPalette) {
		tigerhRecalcPalette = 0;

		TigerHeliPaletteInit();
	}
}

// ---------------------------------------------------------------------------
// Text layer

static unsigned char* TigerHeliTextAttrib = NULL;

static void TigerHeliTextExit()
{
	free(TigerHeliTextAttrib);
	TigerHeliTextAttrib = NULL;
}

static int TigerHeliTextInit()
{
	if ((TigerHeliTextAttrib = (unsigned char*)malloc(0x0400)) == NULL) {
		return 1;
	}

	for (int i = 0; i < 0x0400; i++) {
		bool bEmpty = true;

		for (int j = 0; j < 64; j++) {
			if (TigerHeliTextROM[(i << 6) + j]) {
				bEmpty = false;
				break;
			}
		}

		if (bEmpty) {
			TigerHeliTextAttrib[i] = 0;
		} else {
			TigerHeliTextAttrib[i] = 1;
		}
	}

	return 0;
}

#define PLOTPIXEL(x) if (pTileData[x]) { pPixel[x] = nPalette + pTileData[x]; }

static void TigerHeliRenderTextTile()
{
	unsigned short nPalette = nTilePalette << 2;
	pTileData = TigerHeliTextROM + (nTileNumber << 6);

	unsigned short* pPixel = pTile;

	for (int y = 0; y < 8; y++, pPixel += 280, pTileData += 8) {

		if ((nTileYPos + y) >= 240) {
			break;
		}
		if ((nTileYPos + y) < 0) {
			continue;
		}

		PLOTPIXEL(0);
		PLOTPIXEL(1);
		PLOTPIXEL(2);
		PLOTPIXEL(3);
		PLOTPIXEL(4);
		PLOTPIXEL(5);
		PLOTPIXEL(6);
		PLOTPIXEL(7);
	}
}

#undef PLOTPIXEL

static void TigerHeliTextRender()
{
	unsigned char* pTextRAM;

	if ((nBurnLayer & 2) == 0) {
		return;
	}

	switch (nWhichGame) {
		case 0:										// Tiger Heli
			nTileYPos = -15;
			break;
		case 1:										// Get Star
			nTileYPos = -15;
			break;
		case 2:										// Slap Fight
			nTileYPos = -16;
			break;
	}
	pTileRow = pTransDraw + nTileYPos * 280;

	for (int y = 0; y < 32; y++, nTileYPos += 8, pTileRow += (280 << 3)) {
		if (nTileYPos <= -8 || nTileYPos >= 240 ) {
			continue;
		}
		pTextRAM = TigerHeliTextRAM + (y << 6);
		pTile = pTileRow;
		for (int x = 1; x < 36; x++, pTile += 8) {
			nTileNumber = pTextRAM[x] | (pTextRAM[0x0800 + x] << 8);
			nTilePalette = nTileNumber >> 10;
			nTileNumber &= 0x03FF;

			if (TigerHeliTextAttrib[nTileNumber]) {
				TigerHeliRenderTextTile();
			}
		}
	}

	return;
}

// ---------------------------------------------------------------------------
// Tile layer

static void TigerHeliTileExit()
{
	return;
}

static int TigerHeliTileInit()
{
	return 0;
}

#define PLOTPIXEL(x) pPixel[x] = nPalette + pTileData[x];
#define CLIPPIXEL(a,b) if ((nTileXPos + a) >= 0 && (nTileXPos + a) < 280) { b; }

static void TigerHeliRenderTileNoClip()
{
	unsigned char nPalette = nTilePalette << 4;
	pTileData = TigerHeliTileROM + (nTileNumber << 6);

	unsigned short* pPixel = pTile;

	for (int y = 0; y < 8; y++, pPixel += 280, pTileData += 8) {
		PLOTPIXEL(0);
		PLOTPIXEL(1);
		PLOTPIXEL(2);
		PLOTPIXEL(3);
		PLOTPIXEL(4);
		PLOTPIXEL(5);
		PLOTPIXEL(6);
		PLOTPIXEL(7);
	}
}

static void TigerHeliRenderTileClip()
{
	unsigned char nPalette = nTilePalette << 4;
	pTileData = TigerHeliTileROM + (nTileNumber << 6);

	unsigned short* pPixel = pTile;

	for (int y = 0; y < 8; y++, pPixel += 280, pTileData += 8) {

		if ((nTileYPos + y) >= 240) {
			break;
		}
		if ((nTileYPos + y) < 0) {
			continue;
		}

		CLIPPIXEL(0, PLOTPIXEL(0));
		CLIPPIXEL(1, PLOTPIXEL(1));
		CLIPPIXEL(2, PLOTPIXEL(2));
		CLIPPIXEL(3, PLOTPIXEL(3));
		CLIPPIXEL(4, PLOTPIXEL(4));
		CLIPPIXEL(5, PLOTPIXEL(5));
		CLIPPIXEL(6, PLOTPIXEL(6));
		CLIPPIXEL(7, PLOTPIXEL(7));
	}
}

#undef CLIPPIXEL
#undef PLOTPIXEL

static void TigerHeliTileRender()
{
	unsigned char* pTileRAM;

	if ((nBurnLayer & 3) == 0) {
		BurnTransferClear();
		return;
	}

	int nTigerHeliTileXPos = nTigerHeliTileXPosLo + (nTigerHeliTileXPosHi << 8);
	int nTigerHeliTileYPos = nTigerHeliTileYPosLo;

	switch (nWhichGame) {
		case 0:										// Tiger Heli
			nTigerHeliTileYPos -= 1;
			break;
		case 1:										// Get Star
			nTigerHeliTileYPos -= 1;
			break;
		case 2:										// Slap Fight
			nTigerHeliTileYPos += 1;
			break;
	}

	int nXPos = -nTigerHeliTileXPos & 7;
	nTileYPos = -nTigerHeliTileYPos & 7;

	if (nTigerHeliTileXPos & 7) {
		nXPos -= 8;
	}
	if (nTigerHeliTileYPos & 7) {
		nTileYPos -= 8;
	}

	pTileRow = pTransDraw;
	pTileRow -= (nTigerHeliTileXPos & 7);
	pTileRow -= (nTigerHeliTileYPos & 7) * 280;

	for (int y = 2; y < 33; y++, nTileYPos += 8, pTileRow += (280 << 3)) {
		pTileRAM = TigerHeliTileRAM + (((y + (nTigerHeliTileYPos >> 3)) << 6) & 0x07C0);
		pTile = pTileRow;
		nTileXPos = nXPos;
		for (int x = 1; x < 37; x++, nTileXPos += 8, pTile += 8) {
			int x2 = (x + ((nTigerHeliTileXPos >> 3) & 0x3F));
			nTileNumber = pTileRAM[x2] | (pTileRAM[0x0800 + x2] << 8);
			nTilePalette = nTileNumber >> 12;
			nTileNumber &= nTigerHeliTileMask;

			if (nTileXPos < 0 || nTileXPos > 272 || nTileYPos < 0 || nTileYPos > 232) {
				TigerHeliRenderTileClip();
			} else {
				TigerHeliRenderTileNoClip();
			}
		}
	}

	return;
}

// ---------------------------------------------------------------------------
// Sprites

static int nSpriteXPos, nSpriteYPos, nSpriteNumber, nSpritePalette;

static int nTigerHeliSpriteMask;

static void TigerHeliSpriteExit()
{
	return;
}

static int TigerHeliSpriteInit()
{
	return 0;
}

#define PLOTPIXEL(a) if (pSpriteData[a]) { pPixel[a] = nPalette + pSpriteData[a]; }
#define CLIPPIXEL(a,b) if ((nSpriteXPos + a) >= 0 && (nSpriteXPos + a) < 280) { b; }

static void TigerHeliRenderSpriteNoClip()
{
	unsigned char nPalette = nSpritePalette << 4;
	unsigned char* pSpriteData = TigerHeliSpriteROM + (nSpriteNumber << 8);

	unsigned short* pPixel = pTransDraw + nSpriteXPos + nSpriteYPos * 280;

	for (int y = 0; y < 16; y++, pPixel += 280, pSpriteData += 16) {
		PLOTPIXEL( 0);
		PLOTPIXEL( 1);
		PLOTPIXEL( 2);
		PLOTPIXEL( 3);
		PLOTPIXEL( 4);
		PLOTPIXEL( 5);
		PLOTPIXEL( 6);
		PLOTPIXEL( 7);
		PLOTPIXEL( 8);
		PLOTPIXEL( 9);
		PLOTPIXEL(10);
		PLOTPIXEL(11);
		PLOTPIXEL(12);
		PLOTPIXEL(13);
		PLOTPIXEL(14);
		PLOTPIXEL(15);
	}
}

static void TigerHeliRenderSpriteClip()
{
	unsigned char nPalette = nSpritePalette << 4;
	unsigned char* pSpriteData = TigerHeliSpriteROM + (nSpriteNumber << 8);

	unsigned short* pPixel = pTransDraw + nSpriteXPos + nSpriteYPos * 280;

	for (int y = 0; y < 16; y++, pPixel += 280, pSpriteData += 16) {

		if ((nSpriteYPos + y) < 0 || (nSpriteYPos + y) >= 240) {
			continue;
		}

		CLIPPIXEL( 0, PLOTPIXEL( 0));
		CLIPPIXEL( 1, PLOTPIXEL( 1));
		CLIPPIXEL( 2, PLOTPIXEL( 2));
		CLIPPIXEL( 3, PLOTPIXEL( 3));
		CLIPPIXEL( 4, PLOTPIXEL( 4));
		CLIPPIXEL( 5, PLOTPIXEL( 5));
		CLIPPIXEL( 6, PLOTPIXEL( 6));
		CLIPPIXEL( 7, PLOTPIXEL( 7));
		CLIPPIXEL( 8, PLOTPIXEL( 8));
		CLIPPIXEL( 9, PLOTPIXEL( 9));
		CLIPPIXEL(10, PLOTPIXEL(10));
		CLIPPIXEL(11, PLOTPIXEL(11));
		CLIPPIXEL(12, PLOTPIXEL(12));
		CLIPPIXEL(13, PLOTPIXEL(13));
		CLIPPIXEL(14, PLOTPIXEL(14));
		CLIPPIXEL(15, PLOTPIXEL(15));
	}
}

#undef CLIPPIXEL
#undef PLOTPIXEL

static void TigerHeliSpriteRender()
{
	unsigned char* pSpriteRAM = TigerHeliSpriteBuf;
	int nSpriteYOffset = 0;

	if ((nBurnLayer & 1) == 0) {
		return;
	}

	switch (nWhichGame) {
		case 0:										// Tiger Heli
			nSpriteYOffset = -16;
			break;
		case 1:										// Get Star
			nSpriteYOffset = -16;
			break;
		case 2:										// Slap Fight
			nSpriteYOffset = -17;
			break;
	}

	for (int i = 0; i < 0x0800; i += 4) {
		nSpriteNumber = pSpriteRAM[i + 0x00] | ((pSpriteRAM[i + 0x02] & 0xC0) << 2);
		nSpriteNumber &= nTigerHeliSpriteMask;
		nSpritePalette = (pSpriteRAM[i + 0x02] >> 1) & 0x0F;
		nSpriteXPos = (pSpriteRAM[i + 0x01] | ((pSpriteRAM[i + 0x02] & 0x01) << 8)) - 21;
		nSpriteYPos = pSpriteRAM[i + 0x03] + nSpriteYOffset;

		if (nSpriteXPos > -16 && nSpriteXPos < 280 && nSpriteYPos > -16 && nSpriteYPos < 240) {
			if (nSpriteXPos >= 0 && nSpriteXPos <= 264 && nSpriteYPos >= 0 && nSpriteYPos <= 224) {
				TigerHeliRenderSpriteNoClip();
			} else {
				TigerHeliRenderSpriteClip();
			}
		}
	}

	return;
}

static void TigerHeliBufferSprites()
{
	memcpy(TigerHeliSpriteBuf, TigerHeliSpriteRAM, 0x0800);
}

// ---------------------------------------------------------------------------


static inline void sync_mcu()
{
	int cycles = (ZetTotalCycles() / 2) - m6805TotalCycles();
	if (cycles > 0) {
	//	bprintf (0, _T("mcu %d\n"), cycles);
		m6805Run(cycles);
	}
}

unsigned char __fastcall tigerhReadCPU0(unsigned short a)
{
	if (a >= 0xc800 && a <= 0xcfff) {
		if (ZetPc(-1) == 0x6d34) return 0xff;
		return RamShared[a - 0xc800];
	}
	
	switch (a) {
		case 0xE803: {
			if (use_mcu) {
				sync_mcu();
				return standard_taito_mcu_read();
			}

			unsigned char nProtectSequence[3] = { 0, 1, (0 + 5) ^ 0x56 };

			//if (nProtectIndex == 3) {
			//	nProtectIndex = 0;
			//}

//			bprintf(PRINT_NORMAL, "Protection read (%02X) PC: %04X.\n", nProtectSequence[nProtectIndex], ZetPc(-1));
			//return nProtectSequence[nProtectIndex++];
			
			unsigned char val = nProtectSequence[nProtectIndex];
			nProtectIndex = (nProtectIndex + 1) % 3;
			return val;
		}

//		default:
//			bprintf(PRINT_NORMAL, _T("Attempt by CPU0 to read address %04X.\n"), a);
	}

	return 0;
}

unsigned char __fastcall tigerhReadCPU0_tigerhb1(unsigned short a)
{
	if (a >= 0xc800 && a <= 0xcfff) {
		if (ZetPc(-1) == 0x6d34) return 0xff;
		return RamShared[a - 0xc800];
	}
	
	switch (a) {
		case 0xE803: {
			return 0x83;
		}
	}
	
	return 0;
}


void __fastcall tigerhWriteCPU0(unsigned short a, unsigned char d)
{
	switch (a) {
		case 0xE800:
			nTigerHeliTileXPosLo = d;
			break;
		case 0xE801:
			nTigerHeliTileXPosHi = d;
			break;
		case 0xE802:
			nTigerHeliTileYPosLo = d;
			break;

		case 0xe803: 
			if (use_mcu) {
				sync_mcu();
				from_main = d;
				main_sent = 1;
				mcu_sent = 0;
				m68705SetIrqLine(0, 1 /*ASSERT_LINE*/);
			}
			break;

//		default:
//			bprintf(PRINT_NORMAL, "Attempt by CPU0 to write address %04X -> %02X.\n", a, d);
	}
}

void __fastcall tigerhWriteCPU0_slapbtuk(unsigned short a, unsigned char d)
{
	switch (a) {
		case 0xE800:
			nTigerHeliTileXPosHi = d;
			break;
		case 0xE802:
			nTigerHeliTileYPosLo = d;
			break;
		case 0xE803:
			nTigerHeliTileXPosLo = d;
			break;
//		default:
//			bprintf(PRINT_NORMAL, "Attempt by CPU0 to write address %04X -> %02X.\n", a, d);
	}
}

unsigned char __fastcall tigerhInCPU0(unsigned short a)
{
	a &= 0xFF;

	switch (a) {
		case 0x00: {

			unsigned char nStatusSequence[3] = { 0xC7, 0x55, 0x00 };

			//if (nStatusIndex == 3) {
			//	nStatusIndex = 0;
			//}

//			bprintf(PRINT_NORMAL, "Status read (%02X) PC: %04X.\n", nStatusSequence[nStatusIndex], ZetPc(-1));
			//return nStatusSequence[nStatusIndex++];
			
			unsigned char nStatus = nStatusSequence[nStatusIndex];
			nStatusIndex++;
			if (nStatusIndex > 2) nStatusIndex = 0;

			if (use_mcu) {
				sync_mcu();
				nStatus &= 0xf9;
				if (!main_sent) nStatus |= 0x02;
				if (!mcu_sent) nStatus |= 0x04;
			}

			return nStatus;
			
		}

//		default:
//			bprintf(PRINT_NORMAL, "Attempt by CPU0 to read port %02X.\n", a);
	}

	return 0;
}

unsigned char __fastcall tigerhInCPU0_gtstarba(unsigned short a)
{
	a &= 0xFF;

	switch (a) {
		case 0x00: {
			if (ZetPc(-1) == 0x6d1e)	return 0;
			if (ZetPc(-1) == 0x6d24)	return 6;
			if (ZetPc(-1) == 0x6d2c)	return 2;
			if (ZetPc(-1) == 0x6d34)	return 4;
			return 0;
		}

//		default:
//			bprintf(PRINT_NORMAL, "Attempt by CPU0 to read port %02X.\n", a);
	}

	return 0;
}

void __fastcall tigerhOutCPU0(unsigned short a, unsigned char /* d */)
{
	a &= 0xFF;

	switch (a) {
		case 0x00:					// Assert reset line on sound CPU
//			bprintf(PRINT_NORMAL, "Sound CPU disabled.\n");

			if (bSoundCPUEnable) {
				ZetClose();
				ZetOpen(1);
				ZetReset();
				ZetClose();
				ZetOpen(0);

				bSoundCPUEnable = false;
			}

			break;
		case 0x01:					// Release reset line on sound CPU
//			bprintf(PRINT_NORMAL, "Sound CPU enabled.\n");

			bSoundCPUEnable = true;
			break;

//		case 0x03:
//			break;

//		case 0x05:
//			bprintf(PRINT_NORMAL, "Sound NMI triggered.\n");
/*
			if (bSoundNMIEnable) {
				ZetClose();
				ZetOpen(1);
				ZetNmi();
				ZetClose();
				ZetOpen(0);
			}
*/
//			break;

		case 0x06:					// Disable interrupts
//			bprintf(PRINT_NORMAL, "Interrupts disabled.\n");

			bInterruptEnable = false;
			ZetLowerIrq();
			break;
		case 0x07:					// Enable interrupts
//			bprintf(PRINT_NORMAL, "Interrupts enabled.\n");

			bInterruptEnable = true;
			break;

		case 0x08:
//			bprintf(PRINT_NORMAL, "ROM bank 0 selected.\n");

			// ROM bank 0 selected
			ZetMapArea(0x8000, 0xBFFF, 0, Rom01 + 0x8000);
			ZetMapArea(0x8000, 0xBFFF, 2, Rom01 + 0x8000);
			break;
		case 0x09:
//			bprintf(PRINT_NORMAL, "ROM bank 1 selected.\n");

			// ROM bank 1 selected
			ZetMapArea(0x8000, 0xBFFF, 0, Rom01 + 0xC000);
			ZetMapArea(0x8000, 0xBFFF, 2, Rom01 + 0xC000);
			break;

//		default:
//			bprintf(PRINT_NORMAL, "Attempt by CPU0 to write port %02X -> %02X.\n", a, d);
	}
}

unsigned char __fastcall tigerhReadCPU1(unsigned short a)
{
	switch (a) {
		case 0xA081:
//			bprintf(PRINT_NORMAL, "AY8910 0 read (%02X).\n", AY8910Read(0));
			return AY8910Read(0);
		case 0xA091:
//			bprintf(PRINT_NORMAL, "AY8910 1 read (%02X).\n", AY8910Read(1));
			return AY8910Read(1);

//		default:
//			bprintf(PRINT_NORMAL, "Attempt by CPU1 to read address %04X.\n", a);
	}

	return 0;
}

void __fastcall tigerhWriteCPU1(unsigned short a, unsigned char d)
{
	switch (a) {
		case 0xA080:
//			bprintf(PRINT_NORMAL, "AY8910 0 Register select (%02X).\n", d);
			AY8910Write(0, 0, d);
			break;
		case 0xA082:
//			bprintf(PRINT_NORMAL, "AY8910 0 Register Write (%02X).\n", d);
			AY8910Write(0, 1, d);
			break;
		case 0xA090:
//			bprintf(PRINT_NORMAL, "AY8910 1 Register select (%02X).\n", d);
			AY8910Write(1, 0, d);
			break;
		case 0xA092:
//			bprintf(PRINT_NORMAL, "AY8910 1 Register Write (%02X).\n", d);
			AY8910Write(1, 1, d);
			break;

		case 0xA0E0:
//			bprintf(PRINT_NORMAL, "Sound NMI enabled.\n");
			bSoundNMIEnable = true;
			break;

//		default:
//			bprintf(PRINT_NORMAL, "Attempt by CPU1 to write address %04X -> %02X.\n", a, d);
	}
}

unsigned char __fastcall tigerhInCPU1(unsigned short /* a */)
{
//	bprintf(PRINT_NORMAL, "Attempt by CPU1 to read port %02X.\n", a);

	return 0;
}

void __fastcall tigerhOutCPU1(unsigned short /* a */, unsigned char /* d */)
{
//	bprintf(PRINT_NORMAL, "Attempt by CPU1 to write port %02X -> %02X.\n", a, d);
}

static unsigned char tigerhReadPort0(unsigned int)
{
	return ~tigerhInput[0];
}
static unsigned char tigerhReadPort1(unsigned int)
{
	return ~tigerhInput[1];
}
static unsigned char tigerhReadPort2(unsigned int)
{
	return ~tigerhInput[2];
}
static unsigned char tigerhReadPort3(unsigned int)
{
	return ~tigerhInput[3];
}

//----------------------------------------------------------------------------

void tigerh_m68705_portA_write(unsigned char *data)
{
	from_mcu = *data;
	mcu_sent = 1;
}

void tigerh_m68705_portC_read()
{
	portC_in = 0;
	if (!main_sent) portC_in |= 0x01;
	if (  mcu_sent) portC_in |= 0x02;
}

static m68705_interface tigerh_m68705_interface = {
	tigerh_m68705_portA_write, standard_m68705_portB_out, NULL,
	NULL, NULL, NULL,
	NULL, NULL, tigerh_m68705_portC_read
};

// ---------------------------------------------------------------------------

static int tigerhLoadROMs()
{
	int nRomOffset = 0;
	if (!strcmp(BurnDrvGetTextA(DRV_NAME), "slapfighb2")) nRomOffset = 1;
	if (!strcmp(BurnDrvGetTextA(DRV_NAME), "slapfighb3")) nRomOffset = 2;

	// Z80 main program
	switch (nWhichGame) {
		case 0:											// Tiger Heli
			if (BurnLoadRom(Rom01 + 0x0000, 0, 1)) {
				return 1;
			}
			if (BurnLoadRom(Rom01 + 0x4000, 1, 1)) {
				return 1;
			}
			if (BurnLoadRom(Rom01 + 0x8000, 2, 1)) {
				return 1;
			}
			break;
		case 1:											// Get Star
			if (BurnLoadRom(Rom01 + 0x0000, 0, 1)) {
				return 1;
			}
			if (BurnLoadRom(Rom01 + 0x4000, 1, 1)) {
				return 1;
			}
			if (BurnLoadRom(Rom01 + 0x8000, 2, 1)) {
				return 1;
			}
			break;
		case 2:	{										// Slap Fight
			if (!strcmp(BurnDrvGetTextA(DRV_NAME), "slapfighb2")) {
				if (BurnLoadRom(Rom01 + 0x0000, 0, 1)) {
					return 1;
				}
				if (BurnLoadRom(Rom01 + 0x4000, 1, 1)) {
					return 1;
				}
				if (BurnLoadRom(Rom01 + 0x8000, 2, 1)) {
					return 1;
				}
				break;
			} else {
				if (!strcmp(BurnDrvGetTextA(DRV_NAME), "slapfighb3")) {
					if (BurnLoadRom(Rom01 + 0x0000, 0, 1)) {
						return 1;
					}
					if (BurnLoadRom(Rom01 + 0x4000, 1, 1)) {
						return 1;
					}
					if (BurnLoadRom(Rom01 + 0x10000, 2, 1)) {
						return 1;
					}
					if (BurnLoadRom(Rom01 + 0x8000, 3, 1)) {
						return 1;
					}
					break;
				} else {
					if (BurnLoadRom(Rom01 + 0x0000, 0, 1)) {
						return 1;
					}
					if (BurnLoadRom(Rom01 + 0x8000, 1, 1)) {
						return 1;
					}
					break;
				}
			}
		}
	}

	// Sprites
	{
		int nRet = 0, nBaseROM = 0;
		switch (nWhichGame) {
			case 0:										// Tiger Heli
				nBaseROM = 3;
				break;
			case 1:										// Get Star
				nBaseROM = 3;
				break;
			case 2:										// Slap Fight
				nBaseROM = 2 + nRomOffset;
				break;
		}

		int nSize;

		{
			struct BurnRomInfo ri;

			ri.nType = 0;
			ri.nLen = 0;

			BurnDrvGetRomInfo(&ri, nBaseROM);

			nSize = ri.nLen;
		}

		unsigned char* pTemp = (unsigned char*)malloc(nSize * 4);

		for (int i = 0; i < 4; i++) {
			nRet |= BurnLoadRom(pTemp + nSize * i, nBaseROM + i, 1);
		}

		for (int i = 0; i < nSize; i++) {
			for (int j = 0; j < 8; j++) {
				TigerHeliSpriteROM[(i << 3) + j]  = ((pTemp[i + nSize * 0] >> (7 - j)) & 1) << 3;
				TigerHeliSpriteROM[(i << 3) + j] |= ((pTemp[i + nSize * 1] >> (7 - j)) & 1) << 2;
				TigerHeliSpriteROM[(i << 3) + j] |= ((pTemp[i + nSize * 2] >> (7 - j)) & 1) << 1;
				TigerHeliSpriteROM[(i << 3) + j] |= ((pTemp[i + nSize * 3] >> (7 - j)) & 1) << 0;
			}
		}

		free(pTemp);

		nTigerHeliSpriteMask = (nSize >> 5) - 1;

		if (nRet) {
			return 1;
		}
	}

	// Text layer
	{
		int nBaseROM = 0;
		switch (nWhichGame) {
			case 0:										// Tiger Heli
				nBaseROM = 7;
				break;
			case 1:										// Get Star
				nBaseROM = 7;
				break;
			case 2:										// Slap Fight
				nBaseROM = 6 + nRomOffset;
				break;
		}

		unsigned char* pTemp = (unsigned char*)malloc(0x4000);

		if (BurnLoadRom(pTemp + 0x0000, nBaseROM + 0, 1)) {
			return 1;
		}
		if (BurnLoadRom(pTemp + 0x2000, nBaseROM + 1, 1)) {
			return 1;
		}

		for (int i = 0; i < 0x02000; i++) {
			for (int j = 0; j < 8; j++) {
				TigerHeliTextROM[(i << 3) + j]  = ((pTemp[i + 0x0000] >> (7 - j)) & 1) << 1;
				TigerHeliTextROM[(i << 3) + j] |= ((pTemp[i + 0x2000] >> (7 - j)) & 1) << 0;
			}
		}

		free(pTemp);
	}

	// Tile layer
	{
		int nRet = 0, nBaseROM = 0;
		switch (nWhichGame) {
			case 0:										// Tiger Heli
				nBaseROM = 9;
				break;
			case 1:										// Get Star
				nBaseROM = 9;
				break;
			case 2:										// Slap Fight
				nBaseROM = 8 + nRomOffset;
				break;
		}

		int nSize;

		{
			struct BurnRomInfo ri;

			ri.nType = 0;
			ri.nLen = 0;

			BurnDrvGetRomInfo(&ri, nBaseROM);

			nSize = ri.nLen;
		}

		unsigned char* pTemp = (unsigned char*)malloc(nSize * 4);

		for (int i = 0; i < 4; i++) {
			nRet |= BurnLoadRom(pTemp + nSize * i, nBaseROM + i, 1);
		}

		for (int i = 0; i < nSize; i++) {
			for (int j = 0; j < 8; j++) {
				TigerHeliTileROM[(i << 3) + j]  = ((pTemp[i + nSize * 0] >> (7 - j)) & 1) << 3;
				TigerHeliTileROM[(i << 3) + j] |= ((pTemp[i + nSize * 1] >> (7 - j)) & 1) << 2;
				TigerHeliTileROM[(i << 3) + j] |= ((pTemp[i + nSize * 2] >> (7 - j)) & 1) << 1;
				TigerHeliTileROM[(i << 3) + j] |= ((pTemp[i + nSize * 3] >> (7 - j)) & 1) << 0;
			}
		}

		free(pTemp);

		nTigerHeliTileMask = (nSize >> 3) - 1;

		if (nRet) {
			return 1;
		}
	}

	// Colour PROMs
	{
		int nBaseROM = 0;
		switch (nWhichGame) {
			case 0:										// Tiger Heli
				nBaseROM = 13;
				break;
			case 1:										// Get Star
				nBaseROM = 13;
				break;
			case 2:										// Slap Fight
				nBaseROM = 12 + nRomOffset;
				break;
		}

		if (BurnLoadRom(TigerHeliPaletteROM + 0x0000, nBaseROM + 0, 1)) {
			return 1;
		}
		if (BurnLoadRom(TigerHeliPaletteROM + 0x0100, nBaseROM + 1, 1)) {
			return 1;
		}
		if (BurnLoadRom(TigerHeliPaletteROM + 0x0200, nBaseROM + 2, 1)) {
			return 1;
		}
	}

	// Z80 program
	{
		int nBaseROM = 0;
		switch (nWhichGame) {
			case 0:										// Tiger Heli
				nBaseROM = 16;
				break;
			case 1:										// Get Star
				nBaseROM = 16;
				break;
			case 2:										// Slap Fight
				nBaseROM = 15 + nRomOffset;
				break;
		}

		if (BurnLoadRom(Rom02, nBaseROM, 1)) {
			return 1;
		}
	}

	// MCU program
	{

		if (strcmp(BurnDrvGetTextA(DRV_NAME), "tigerh") == 0 || strcmp(BurnDrvGetTextA(DRV_NAME), "tigerhj") == 0) {
			if (BurnLoadRom(Rom03, 17, 1)) {
				return 1;
			}

			use_mcu = 1;
		}

	}

	return 0;
}

static int tigerhExit()
{
	BurnTransferExit();

	TigerHeliSpriteExit();
	TigerHeliTextExit();
	TigerHeliTileExit();

	ZetExit();

	if (use_mcu) {
		use_mcu = 0;
		m67805_taito_exit();
	}

	// Deallocate all used memory
	free(Mem);
	Mem = NULL;

	return 0;
}

static void tigerhDoReset()
{
	bInterruptEnable = false;
	bSoundNMIEnable = false;
	bSoundCPUEnable = true;
	
	nStatusIndex = 0;
	nProtectIndex = 0;

	ZetOpen(0);
	ZetReset();
	ZetClose();

	ZetOpen(1);
	ZetReset();
	ZetClose();

	if (use_mcu) {
		m67805_taito_reset();
	}

	return;
}

static int tigerhInit()
{
	int nLen;

	nWhichGame = -1;

	if (strcmp(BurnDrvGetTextA(DRV_NAME), "tigerh") == 0 || strcmp(BurnDrvGetTextA(DRV_NAME), "tigerhj") == 0 || strcmp(BurnDrvGetTextA(DRV_NAME), "tigerhb1") == 0 || strcmp(BurnDrvGetTextA(DRV_NAME), "tigerhb2") == 0 || strcmp(BurnDrvGetTextA(DRV_NAME), "tigerhb3") == 0) {
		nWhichGame = 0;
	}
	if (strcmp(BurnDrvGetTextA(DRV_NAME), "getstar") == 0 || strcmp(BurnDrvGetTextA(DRV_NAME), "gtstarb1") == 0 || strcmp(BurnDrvGetTextA(DRV_NAME), "gtstarb2") == 0) {
		nWhichGame = 1;
	}
	if (strcmp(BurnDrvGetTextA(DRV_NAME), "slapfigh") == 0 || strcmp(BurnDrvGetTextA(DRV_NAME), "slapfighb1") == 0 || strcmp(BurnDrvGetTextA(DRV_NAME), "slapfighb2") == 0 || strcmp(BurnDrvGetTextA(DRV_NAME), "slapfighb3") == 0) {
		nWhichGame = 2;
	}

	// Find out how much memory is needed
	Mem = NULL;
	MemIndex();
	nLen = MemEnd - (unsigned char*)0;
	if ((Mem = (unsigned char*)malloc(nLen)) == NULL) {
		return 1;
	}
	memset(Mem, 0, nLen);										   	// blank all memory
	MemIndex();													   	// Index the allocated memory

	// Load the roms into memory
	if (tigerhLoadROMs()) {
		return 1;
	}

	{
		ZetInit(2);

		// Main CPU setup
		ZetOpen(0);

		// Program ROM
		ZetMapArea(0x0000, 0x7FFF, 0, Rom01);
		ZetMapArea(0x0000, 0x7FFF, 2, Rom01);
		// Banked ROM
		ZetMapArea(0x8000, 0xBFFF, 0, Rom01 + 0x8000);
		ZetMapArea(0x8000, 0xBFFF, 2, Rom01 + 0x8000);

		// Work RAM
		ZetMapArea(0xC000, 0xC7FF, 0, Ram01);
		ZetMapArea(0xC000, 0xC7FF, 1, Ram01);
		ZetMapArea(0xC000, 0xC7FF, 2, Ram01);

		// Shared RAM
		if (strcmp(BurnDrvGetTextA(DRV_NAME), "gtstarb1")) {
			ZetMapArea(0xC800, 0xCFFF, 0, RamShared);
		}
		ZetMapArea(0xC800, 0xCFFF, 1, RamShared);
		ZetMapArea(0xC800, 0xCFFF, 2, RamShared);
		
		// Tile RAM
		ZetMapArea(0xD000, 0xDFFF, 0, TigerHeliTileRAM);
		ZetMapArea(0xD000, 0xDFFF, 1, TigerHeliTileRAM);
		ZetMapArea(0xD000, 0xDFFF, 2, TigerHeliTileRAM);
		// Sprite RAM
		ZetMapArea(0xE000, 0xE7FF, 0, TigerHeliSpriteRAM);
		ZetMapArea(0xE000, 0xE7FF, 1, TigerHeliSpriteRAM);
		ZetMapArea(0xE000, 0xE7FF, 2, TigerHeliSpriteRAM);
		
		if (!strcmp(BurnDrvGetTextA(DRV_NAME), "slapfighb2") || !strcmp(BurnDrvGetTextA(DRV_NAME), "slapfighb3")) {
			ZetMapArea(0xec00, 0xeFFF, 0, Rom01 + 0x10c00);
			ZetMapArea(0xec00, 0xeFFF, 2, Rom01 + 0x10c00);
		}
		
		// Text RAM
		ZetMapArea(0xF000, 0xFFFF, 0, TigerHeliTextRAM);
		ZetMapArea(0xF000, 0xFFFF, 1, TigerHeliTextRAM);
		ZetMapArea(0xF000, 0xFFFF, 2, TigerHeliTextRAM);

		ZetMemEnd();

		if (!strcmp(BurnDrvGetTextA(DRV_NAME), "tigerhb1")) {
			ZetSetReadHandler(tigerhReadCPU0_tigerhb1);
		} else {
			ZetSetReadHandler(tigerhReadCPU0);
		}
		
		if (!strcmp(BurnDrvGetTextA(DRV_NAME), "slapfighb2") || !strcmp(BurnDrvGetTextA(DRV_NAME), "slapfighb3")) {
			ZetSetWriteHandler(tigerhWriteCPU0_slapbtuk);
		} else {
			ZetSetWriteHandler(tigerhWriteCPU0);
		}
		
		if (!strcmp(BurnDrvGetTextA(DRV_NAME), "gtstarb1")) {
			ZetSetInHandler(tigerhInCPU0_gtstarba);
		} else {
			ZetSetInHandler(tigerhInCPU0);
		}
		
		ZetSetOutHandler(tigerhOutCPU0);

		ZetClose();

		// Sound CPU setup
		ZetOpen(1);

		// Program ROM
		ZetMapArea(0x0000, 0x1FFF, 0, Rom02);
		ZetMapArea(0x0000, 0x1FFF, 2, Rom02);

		// Work RAM
		ZetMapArea(0xC800, 0xCFFF, 0, RamShared);
		ZetMapArea(0xC800, 0xCFFF, 1, RamShared);
		ZetMapArea(0xC800, 0xCFFF, 2, RamShared);

		ZetMemEnd();

		ZetSetReadHandler(tigerhReadCPU1);
		ZetSetWriteHandler(tigerhWriteCPU1);
		ZetSetInHandler(tigerhInCPU1);
		ZetSetOutHandler(tigerhOutCPU1);

		ZetClose();

		if (use_mcu) {
			m67805_taito_init(Rom03, Ram03, &tigerh_m68705_interface);
		}
	}

	pAY8910Buffer[0] = pFMBuffer + nBurnSoundLen * 0;
	pAY8910Buffer[1] = pFMBuffer + nBurnSoundLen * 1;
	pAY8910Buffer[2] = pFMBuffer + nBurnSoundLen * 2;
	pAY8910Buffer[3] = pFMBuffer + nBurnSoundLen * 3;
	pAY8910Buffer[4] = pFMBuffer + nBurnSoundLen * 4;
	pAY8910Buffer[5] = pFMBuffer + nBurnSoundLen * 5;

	AY8910Init(0, 1500000, nBurnSoundRate, &tigerhReadPort0, &tigerhReadPort1, NULL, NULL);
	AY8910Init(1, 1500000, nBurnSoundRate, &tigerhReadPort2, &tigerhReadPort3, NULL, NULL);

	TigerHeliTileInit();
	TigerHeliTextInit();
	TigerHeliSpriteInit();
	TigerHeliPaletteInit();

	BurnTransferInit();

	tigerhDoReset();

	return 0;
}

static int tigerhScan(int nAction, int* pnMin)
{
	struct BurnArea ba;

	if (pnMin) {						// Return minimum compatible version
		*pnMin = 0x029521;
	}

	if (nAction & ACB_VOLATILE) {		// Scan volatile ram
		memset(&ba, 0, sizeof(ba));
    ba.Data	  = RamStart;
		ba.nLen	  = RamEnd-RamStart;
		ba.szName = "All Ram";
		BurnAcb(&ba);

		ZetScan(nAction);							// Scan Z80

		// Scan critical driver variables
		SCAN_VAR(bInterruptEnable);
		SCAN_VAR(bSoundCPUEnable);
		SCAN_VAR(bSoundNMIEnable);
		SCAN_VAR(nStatusIndex);
		SCAN_VAR(nProtectIndex);
		SCAN_VAR(tigerhInput);
	}

	return 0;
}

static void tigerhDraw()
{
	TigerHeliPaletteUpdate();

	TigerHeliTileRender();
	TigerHeliSpriteRender();
	TigerHeliTextRender();

	BurnTransferCopy(TigerHeliPalette);

	return;
}

static inline int CheckSleep(int)
{
	return 0;
}

static int tigerhFrame()
{
	int nCyclesTotal[3], nCyclesDone[3];

	if (tigerhReset) {													// Reset machine
		tigerhDoReset();
	}

	ZetNewFrame();
	m6805NewFrame();

	// Compile digital inputs
	tigerhInput[0] = 0x00;
	tigerhInput[1] = 0x00;
	for (int i = 0; i < 8; i++) {
		tigerhInput[0] |= (tigerhInpJoy1[i] & 1) << i;
		if (nWhichGame == 0 && i < 4) {
			tigerhInput[1] |= (tigerhInpMisc[i] & 1) << (i ^ 1);
		} else {
			tigerhInput[1] |= (tigerhInpMisc[i] & 1) << i;
		}
	}

	if ((tigerhInput[0] & 0x03) == 0x03) {
		tigerhInput[0] &= ~0x03;
	}
	if ((tigerhInput[0] & 0x0C) == 0x0C) {
		tigerhInput[0] &= ~0x0C;
	}
	if ((tigerhInput[0] & 0x30) == 0x30) {
		tigerhInput[0] &= ~0x30;
	}
	if ((tigerhInput[0] & 0xC0) == 0xC0) {
		tigerhInput[0] &= ~0xC0;
	}

	if (nWhichGame == 1) {
		tigerhInput[0] = (tigerhInput[0] & 0x99) | ((tigerhInput[0] << 1) & 0x44) | ((tigerhInput[0] >> 1) & 0x22);
	}

	nCyclesTotal[0] = nCyclesTotal[1] = 6000000 / 60;
	nCyclesDone[0] = nCyclesDone[1] = nCyclesDone[2] = 0;
	nCyclesTotal[2] = 3000000 / 60;

	const int nVBlankCycles = 248 * 6000000 / 60 / 262;
	const int nInterleave = 12;

	int nSoundBufferPos = 0;
	int nSoundNMIMask = 0;
	switch (nWhichGame) {
		case 0:
			nSoundNMIMask = 1;
			break;
		case 1:
			nSoundNMIMask = 3;
			break;
		case 2:
			nSoundNMIMask = 3;
			break;
	}

	bool bVBlank = false;

	for (int i = 0; i < nInterleave; i++) {
    	int nCurrentCPU;
		int nNext, nCyclesSegment;

		nCurrentCPU = 0;
		ZetOpen(nCurrentCPU);

		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;

		if (nNext > nVBlankCycles && !bVBlank) {
			nCyclesDone[nCurrentCPU] += ZetRun(nNext - nVBlankCycles);

			if (pBurnDraw != NULL) {
				tigerhDraw();											// Draw screen if needed
			}

			TigerHeliBufferSprites();

			bVBlank = true;

			if (bInterruptEnable) {
				ZetRaiseIrq(0xFF);
#if 0
				ZetClose();
				ZetOpen(1);
				ZetRaiseIrq(0xFF);
				ZetClose();
				ZetOpen(0);
#endif
			}
		}

		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		if (bVBlank || (!CheckSleep(nCurrentCPU))) {					// See if this CPU is busywaiting
			nCyclesDone[nCurrentCPU] += ZetRun(nCyclesSegment);
		} else {
			nCyclesDone[nCurrentCPU] += nCyclesSegment;
		}

		ZetClose();

		if (use_mcu) {
			m6805Open(0);
			nCyclesSegment = (nCyclesTotal[2] * (i + 1)) / nInterleave;
			nCyclesSegment -= m6805TotalCycles();
			if (nCyclesSegment > 0) {
				nCyclesDone[2] += m6805Run(nCyclesSegment);
			}
			m6805Close();
		}

		nCurrentCPU = 1;
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];

		if (bSoundCPUEnable) {
			ZetOpen(nCurrentCPU);

			if ((i & nSoundNMIMask) == 0) {
				if (bSoundNMIEnable) {
					ZetNmi();
				}
			}

			nCyclesDone[nCurrentCPU] += ZetRun(nCyclesSegment);
			ZetClose();
		} else {
			nCyclesDone[nCurrentCPU] += nCyclesSegment;
		}

		{
			// Render sound segment
			if (pBurnSoundOut) {
				int nSample;
				int nSegmentLength = nBurnSoundLen / nInterleave;
				short* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
				AY8910Update(0, &pAY8910Buffer[0], nSegmentLength);
				AY8910Update(1, &pAY8910Buffer[3], nSegmentLength);
   				for (int n = 0; n < nSegmentLength; n++) {
					nSample  = pAY8910Buffer[0][n] >> 2;
					nSample += pAY8910Buffer[1][n] >> 2;
					nSample += pAY8910Buffer[2][n] >> 2;
					nSample += pAY8910Buffer[3][n] >> 2;
					nSample += pAY8910Buffer[4][n] >> 2;
					nSample += pAY8910Buffer[5][n] >> 2;

					if (nSample < -32768) {
						nSample = -32768;
					} else {
						if (nSample > 32767) {
							nSample = 32767;
						}
					}

					pSoundBuf[(n << 1) + 0] = nSample;
					pSoundBuf[(n << 1) + 1] = nSample;
    			}
				nSoundBufferPos += nSegmentLength;
			}
		}
	}

	{
		// Make sure the buffer is entirely filled.
		if (pBurnSoundOut) {
			int nSample;
			int nSegmentLength = nBurnSoundLen - nSoundBufferPos;
			short* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
			if (nSegmentLength) {
				AY8910Update(0, &pAY8910Buffer[0], nSegmentLength);
				AY8910Update(1, &pAY8910Buffer[3], nSegmentLength);
   				for (int n = 0; n < nSegmentLength; n++) {
					nSample  = pAY8910Buffer[0][n] >> 2;
					nSample += pAY8910Buffer[1][n] >> 2;
					nSample += pAY8910Buffer[2][n] >> 2;
					nSample += pAY8910Buffer[3][n] >> 2;
					nSample += pAY8910Buffer[4][n] >> 2;
					nSample += pAY8910Buffer[5][n] >> 2;

					if (nSample < -32768) {
						nSample = -32768;
					} else {
						if (nSample > 32767) {
							nSample = 32767;
						}
					}

					pSoundBuf[(n << 1) + 0] = nSample;
					pSoundBuf[(n << 1) + 1] = nSample;
    			}
			}
		}
	}

	return 0;
}

// ---------------------------------------------------------------------------
// Rom information

static struct BurnRomInfo tigerhRomDesc[] = {
	{ "0.4",          0x004000, 0x4BE73246, BRF_ESS | BRF_PRG }, //  0 CPU #0 code
	{ "1.4",          0x004000, 0xAAD04867, BRF_ESS | BRF_PRG }, //  1
	{ "2.4",          0x004000, 0x4843F15C, BRF_ESS | BRF_PRG }, //  2

	{ "a47_13.8j",    0x004000, 0x739A7E7E, BRF_GRA },			 //  3 Sprite data
	{ "a47_12.6j",    0x004000, 0xC064ECDB, BRF_GRA },			 //  4
	{ "a47_11.8h",    0x004000, 0x744FAE9B, BRF_GRA },			 //  5
	{ "a47_10.6h",    0x004000, 0xE1CF844E, BRF_GRA },			 //  6

	{ "a47_05.6f",    0x002000, 0xC5325B49, BRF_GRA },			 //  7 Text layer
	{ "a47_04.6g",    0x002000, 0xCD59628E, BRF_GRA },			 //  8

	{ "a47_09.4m",    0x004000, 0x31FAE8A8, BRF_GRA },			 //  9 Background layer
	{ "a47_08.6m",    0x004000, 0xE539AF2B, BRF_GRA },			 // 10
	{ "a47_07.6n",    0x004000, 0x02FDD429, BRF_GRA },			 // 11
	{ "a47_06.6p",    0x004000, 0x11FBCC8C, BRF_GRA },			 // 12

	{ "82s129.12q",   0x000100, 0x2C69350D, BRF_GRA },			 // 13
	{ "82s129.12m",   0x000100, 0x7142E972, BRF_GRA },			 // 14
	{ "82s129.12n",   0x000100, 0x25F273F2, BRF_GRA },			 // 15

	{ "a47_03.12d",   0x002000, 0xD105260F, BRF_ESS | BRF_PRG }, // 16
	
	{ "a47_14.6a",    0x000800, 0x4042489F, BRF_ESS | BRF_PRG },		// 17 MCU

	{ "pal16r4a.2e",  260,      0x00000000, BRF_NODUMP },
};


STD_ROM_PICK(tigerh)
STD_ROM_FN(tigerh)

static struct BurnRomInfo tigerhjRomDesc[] = {
	{ "a47_00.8p",    0x004000, 0xcbdbe3cc, BRF_ESS | BRF_PRG }, //  0 CPU #0 code
	{ "a47_01.8n",    0x004000, 0x65df2152, BRF_ESS | BRF_PRG }, //  1
	{ "a47_02.8k",    0x004000, 0x633D324B, BRF_ESS | BRF_PRG }, //  2

	{ "a47_13.8j",    0x004000, 0x739A7E7E, BRF_GRA },			 //  3 Sprite data
	{ "a47_12.6j",    0x004000, 0xC064ECDB, BRF_GRA },			 //  4
	{ "a47_11.8h",    0x004000, 0x744FAE9B, BRF_GRA },			 //  5
	{ "a47_10.6h",    0x004000, 0xE1CF844E, BRF_GRA },			 //  6

	{ "a47_05.6f",    0x002000, 0xC5325B49, BRF_GRA },			 //  7 Text layer
	{ "a47_04.6g",    0x002000, 0xCD59628E, BRF_GRA },			 //  8

	{ "a47_09.4m",    0x004000, 0x31FAE8A8, BRF_GRA },			 //  9 Background layer
	{ "a47_08.6m",    0x004000, 0xE539AF2B, BRF_GRA },			 // 10
	{ "a47_07.6n",    0x004000, 0x02FDD429, BRF_GRA },			 // 11
	{ "a47_06.6p",    0x004000, 0x11FBCC8C, BRF_GRA },			 // 12

	{ "82s129.12q",   0x000100, 0x2C69350D, BRF_GRA },			 // 13
	{ "82s129.12m",   0x000100, 0x7142E972, BRF_GRA },			 // 14
	{ "82s129.12n",   0x000100, 0x25F273F2, BRF_GRA },			 // 15

	{ "a47_03.12d",   0x002000, 0xD105260F, BRF_ESS | BRF_PRG }, // 16
	
	{ "a47_14.6a",    0x000800, 0x4042489F, BRF_ESS | BRF_PRG },		// 17 MCU
};


STD_ROM_PICK(tigerhj)
STD_ROM_FN(tigerhj)

static struct BurnRomInfo tigerhb1RomDesc[] = {
	{ "b0.5",         0x004000, 0x6ae7e13c, BRF_ESS | BRF_PRG }, //  0 CPU #0 code
	{ "a47_01.8n",    0x004000, 0x65df2152, BRF_ESS | BRF_PRG }, //  1
	{ "a47_02.8k",    0x004000, 0x633D324B, BRF_ESS | BRF_PRG }, //  2

	{ "a47_13.8j",    0x004000, 0x739A7E7E, BRF_GRA },			 //  3 Sprite data
	{ "a47_12.6j",    0x004000, 0xC064ECDB, BRF_GRA },			 //  4
	{ "a47_11.8h",    0x004000, 0x744FAE9B, BRF_GRA },			 //  5
	{ "a47_10.6h",    0x004000, 0xE1CF844E, BRF_GRA },			 //  6

	{ "a47_05.6f",    0x002000, 0xC5325B49, BRF_GRA },			 //  7 Text layer
	{ "a47_04.6g",    0x002000, 0xCD59628E, BRF_GRA },			 //  8

	{ "a47_09.4m",    0x004000, 0x31FAE8A8, BRF_GRA },			 //  9 Background layer
	{ "a47_08.6m",    0x004000, 0xE539AF2B, BRF_GRA },			 // 10
	{ "a47_07.6n",    0x004000, 0x02FDD429, BRF_GRA },			 // 11
	{ "a47_06.6p",    0x004000, 0x11FBCC8C, BRF_GRA },			 // 12

	{ "82s129.12q",   0x000100, 0x2C69350D, BRF_GRA },			 // 13
	{ "82s129.12m",   0x000100, 0x7142E972, BRF_GRA },			 // 14
	{ "82s129.12n",   0x000100, 0x25F273F2, BRF_GRA },			 // 15

	{ "a47_03.12d",   0x002000, 0xD105260F, BRF_ESS | BRF_PRG }, // 16
};


STD_ROM_PICK(tigerhb1)
STD_ROM_FN(tigerhb1)

static struct BurnRomInfo tigerhb2RomDesc[] = {
	{ "rom00_09.bin", 0x004000, 0xef738c68, BRF_ESS | BRF_PRG }, //  0 CPU #0 code
	{ "a47_01.8n",    0x004000, 0x65df2152, BRF_ESS | BRF_PRG }, //  1
	{ "rom02_07.bin", 0x004000, 0x36e250b9, BRF_ESS | BRF_PRG }, //  2

	{ "a47_13.8j",    0x004000, 0x739A7E7E, BRF_GRA },			 //  3 Sprite data
	{ "a47_12.6j",    0x004000, 0xC064ECDB, BRF_GRA },			 //  4
	{ "a47_11.8h",    0x004000, 0x744FAE9B, BRF_GRA },			 //  5
	{ "a47_10.6h",    0x004000, 0xE1CF844E, BRF_GRA },			 //  6

	{ "a47_05.6f",    0x002000, 0xC5325B49, BRF_GRA },			 //  7 Text layer
	{ "a47_04.6g",    0x002000, 0xCD59628E, BRF_GRA },			 //  8

	{ "a47_09.4m",    0x004000, 0x31FAE8A8, BRF_GRA },			 //  9 Background layer
	{ "a47_08.6m",    0x004000, 0xE539AF2B, BRF_GRA },			 // 10
	{ "a47_07.6n",    0x004000, 0x02FDD429, BRF_GRA },			 // 11
	{ "a47_06.6p",    0x004000, 0x11FBCC8C, BRF_GRA },			 // 12

	{ "82s129.12q",   0x000100, 0x2C69350D, BRF_GRA },			 // 13
	{ "82s129.12m",   0x000100, 0x7142E972, BRF_GRA },			 // 14
	{ "82s129.12n",   0x000100, 0x25F273F2, BRF_GRA },			 // 15

	{ "a47_03.12d",   0x002000, 0xD105260F, BRF_ESS | BRF_PRG }, // 16
};


STD_ROM_PICK(tigerhb2)
STD_ROM_FN(tigerhb2)

static struct BurnRomInfo tigerhb3RomDesc[] = {
	{ "14",           0x004000, 0xCA59DD73, BRF_ESS | BRF_PRG }, //  0 CPU #0 code
	{ "13",           0x004000, 0x38BD54DB, BRF_ESS | BRF_PRG }, //  1
	{ "a47_02.8k",    0x004000, 0x633D324B, BRF_ESS | BRF_PRG }, //  2

	{ "a47_13.8j",    0x004000, 0x739A7E7E, BRF_GRA },			 //  3 Sprite data
	{ "a47_12.6j",    0x004000, 0xC064ECDB, BRF_GRA },			 //  4
	{ "a47_11.8h",    0x004000, 0x744FAE9B, BRF_GRA },			 //  5
	{ "a47_10.6h",    0x004000, 0xE1CF844E, BRF_GRA },			 //  6

	{ "a47_05.6f",    0x002000, 0xC5325B49, BRF_GRA },			 //  7 Text layer
	{ "a47_04.6g",    0x002000, 0xCD59628E, BRF_GRA },			 //  8

	{ "a47_09.4m",    0x004000, 0x31FAE8A8, BRF_GRA },			 //  9 Background layer
	{ "a47_08.6m",    0x004000, 0xE539AF2B, BRF_GRA },			 // 10
	{ "a47_07.6n",    0x004000, 0x02FDD429, BRF_GRA },			 // 11
	{ "a47_06.6p",    0x004000, 0x11FBCC8C, BRF_GRA },			 // 12

	{ "82s129.12q",   0x000100, 0x2C69350D, BRF_GRA },			 // 13
	{ "82s129.12m",   0x000100, 0x7142E972, BRF_GRA },			 // 14
	{ "82s129.12n",   0x000100, 0x25F273F2, BRF_GRA },			 // 15

	{ "a47_03.12d",   0x002000, 0xD105260F, BRF_ESS | BRF_PRG }, // 16
};


STD_ROM_PICK(tigerhb3)
STD_ROM_FN(tigerhb3)

static struct BurnRomInfo getstarRomDesc[] = {
	{ "a68_00-1",     0x004000, 0x6A8BDC6C, BRF_ESS | BRF_PRG }, //  0 CPU #0 code
	{ "a68_01-1",     0x004000, 0xEBE8DB3C, BRF_ESS | BRF_PRG }, //  1
	{ "a68_02-1",     0x008000, 0x343E8415, BRF_ESS | BRF_PRG }, //  2

	{ "a68-13",       0x008000, 0x643FB282, BRF_GRA },			 //  3 Sprite data
	{ "a68-12",       0x008000, 0x11F74E32, BRF_GRA },			 //  4
	{ "a68-11",       0x008000, 0xF24158CF, BRF_GRA },			 //  5
	{ "a68-10",       0x008000, 0x83161Ed0, BRF_GRA },			 //  6

	{ "a68_05-1",     0x002000, 0x06F60107, BRF_GRA },			 //  7 Text layer
	{ "a68_04-1",     0x002000, 0x1FC8F277, BRF_GRA },			 //  8

	{ "a68_09",       0x008000, 0xA293CC2E, BRF_GRA },			 //  9 Background layer
	{ "a68_08",       0x008000, 0x37662375, BRF_GRA },			 // 10
	{ "a68_07",       0x008000, 0xCF1A964C, BRF_GRA },			 // 11
	{ "a68_06",       0x008000, 0x05F9EB9A, BRF_GRA },			 // 12

	{ "rom21",        0x000100, 0xD6360B4D, BRF_GRA },			 // 13
	{ "rom20",        0x000100, 0x4CA01887, BRF_GRA },			 // 14
	{ "rom19",        0x000100, 0x513224F0, BRF_GRA },			 // 15

	{ "a68-03",       0x002000, 0x18DAA44C, BRF_ESS | BRF_PRG }, // 16

	{ "a68_14",       0x000800, 0x00000000, BRF_NODUMP | BRF_ESS | BRF_PRG }, // 17 MCU ROM
};


STD_ROM_PICK(getstar)
STD_ROM_FN(getstar)

static struct BurnRomInfo gtstarb2RomDesc[] = {
	{ "gs_14.rom",    0x004000, 0x1A57A920, BRF_ESS | BRF_PRG }, //  0 CPU #0 code
	{ "gs_13.rom",    0x004000, 0x805F8E77, BRF_ESS | BRF_PRG }, //  1
	{ "a68_02.bin",   0x008000, 0x3567DA17, BRF_ESS | BRF_PRG }, //  2

	{ "a68-13",       0x008000, 0x643FB282, BRF_GRA },			 //  3 Sprite data
	{ "a68-12",       0x008000, 0x11F74E32, BRF_GRA },			 //  4
	{ "a68-11",       0x008000, 0xF24158CF, BRF_GRA },			 //  5
	{ "a68-10",       0x008000, 0x83161Ed0, BRF_GRA },			 //  6

	{ "a68_05.bin",   0x002000, 0xE3D409E7, BRF_GRA },			 //  7 Text layer
	{ "a68_04.bin",   0x002000, 0x6E5AC9D4, BRF_GRA },			 //  8

	{ "a68_09",       0x008000, 0xA293CC2E, BRF_GRA },			 //  9 Background layer
	{ "a68_08",       0x008000, 0x37662375, BRF_GRA },			 // 10
	{ "a68_07",       0x008000, 0xCF1A964C, BRF_GRA },			 // 11
	{ "a68_06",       0x008000, 0x05F9EB9A, BRF_GRA },			 // 12

	{ "rom21",        0x000100, 0xD6360B4D, BRF_GRA },			 // 13
	{ "rom20",        0x000100, 0x4CA01887, BRF_GRA },			 // 14
	{ "rom19",        0x000100, 0x513224F0, BRF_GRA },			 // 15

	{ "a68-03",       0x002000, 0x18DAA44C, BRF_ESS | BRF_PRG }, // 16
};


STD_ROM_PICK(gtstarb2)
STD_ROM_FN(gtstarb2)

static struct BurnRomInfo gtstarb1RomDesc[] = {
	{ "gs_rb_1.bin",  0x004000, 0x9afad7e0, BRF_ESS | BRF_PRG }, //  0 CPU #0 code
	{ "gs_rb_2.bin",  0x004000, 0x5feb0a60, BRF_ESS | BRF_PRG }, //  1
	{ "gs_rb_3.bin",  0x008000, 0xe3cfb1ba, BRF_ESS | BRF_PRG }, //  2

	{ "a68-13",       0x008000, 0x643FB282, BRF_GRA },			 //  3 Sprite data
	{ "a68-12",       0x008000, 0x11F74E32, BRF_GRA },			 //  4
	{ "a68-11",       0x008000, 0xF24158CF, BRF_GRA },			 //  5
	{ "a68-10",       0x008000, 0x83161Ed0, BRF_GRA },			 //  6

	{ "a68_05.bin",   0x002000, 0xE3D409E7, BRF_GRA },			 //  7 Text layer
	{ "a68_04.bin",   0x002000, 0x6E5AC9D4, BRF_GRA },			 //  8

	{ "a68_09",       0x008000, 0xA293CC2E, BRF_GRA },			 //  9 Background layer
	{ "a68_08",       0x008000, 0x37662375, BRF_GRA },			 // 10
	{ "a68_07",       0x008000, 0xCF1A964C, BRF_GRA },			 // 11
	{ "a68_06",       0x008000, 0x05F9EB9A, BRF_GRA },			 // 12

	{ "rom21",        0x000100, 0xD6360B4D, BRF_GRA },			 // 13
	{ "rom20",        0x000100, 0x4CA01887, BRF_GRA },			 // 14
	{ "rom19",        0x000100, 0x513224F0, BRF_GRA },			 // 15

	{ "a68-03",       0x002000, 0x18DAA44C, BRF_ESS | BRF_PRG }, // 16
};


STD_ROM_PICK(gtstarb1)
STD_ROM_FN(gtstarb1)

static struct BurnRomInfo alconRomDesc[] = {
	{ "a77_00-1.8p",  0x008000, 0x2ba82d60, BRF_ESS | BRF_PRG }, //  0 CPU #0 code
	{ "a77_01-1.8n",  0x008000, 0x18bb2f12, BRF_ESS | BRF_PRG }, //  1

	{ "a77_12.8j",    0x008000, 0x8545d397, BRF_GRA },			 //  2 Sprite data
	{ "a77_11.7j",    0x008000, 0xb1b7b925, BRF_GRA },			 //  3
	{ "a77_10.8h",    0x008000, 0x422d946b, BRF_GRA },			 //  4
	{ "a77_09.7h",    0x008000, 0x587113ae, BRF_GRA },			 //  5

	{ "a77_04-1.6f",  0x002000, 0x31003483, BRF_GRA },			 //  6 Text layer
	{ "a77_03-1.6g",  0x002000, 0x404152c0, BRF_GRA },			 //  7

	{ "a77_08.6k",    0x008000, 0xB6358305, BRF_GRA },			 //  8 Background layer
	{ "a77_07.6m",    0x008000, 0xE92D9D60, BRF_GRA },			 //  9
	{ "a77_06.6n",    0x008000, 0x5FAEEEA3, BRF_GRA },			 // 10
	{ "a77_05.6p",    0x008000, 0x974E2EA9, BRF_GRA },			 // 11

	{ "21_82s129.12q",0x000100, 0xA0EFAF99, BRF_GRA },			 // 12
	{ "20_82s129.12m",0x000100, 0xA56D57E5, BRF_GRA },			 // 13
	{ "19_82s129.12n",0x000100, 0x5CBF9FBF, BRF_GRA },			 // 14

	{ "a77_02.12d",   0x002000, 0x87F4705A, BRF_ESS | BRF_PRG }, // 15

	{ "a77_13.6a",    0x000800, 0xa70c81d9, BRF_ESS | BRF_PRG }, // 16 MCU ROM
};


STD_ROM_PICK(alcon)
STD_ROM_FN(alcon)

static struct BurnRomInfo slapfighRomDesc[] = {
	{ "a77_00.8p",    0x008000, 0x674C0E0F, BRF_ESS | BRF_PRG }, //  0 CPU #0 code
	{ "a77_01.8n",    0x008000, 0x3C42E4A7, BRF_ESS | BRF_PRG }, //  1

	{ "a77_12.8j",    0x008000, 0x8545d397, BRF_GRA },			 //  2 Sprite data
	{ "a77_11.7j",    0x008000, 0xb1b7b925, BRF_GRA },			 //  3
	{ "a77_10.8h",    0x008000, 0x422d946b, BRF_GRA },			 //  4
	{ "a77_09.7h",    0x008000, 0x587113ae, BRF_GRA },			 //  5

	{ "a77_04.6f",    0x002000, 0x2AC7B943, BRF_GRA },			 //  6 Text layer
	{ "a77_03.6g",    0x002000, 0x33CADC93, BRF_GRA },			 //  7

	{ "a77_08.6k",    0x008000, 0xB6358305, BRF_GRA },			 //  8 Background layer
	{ "a77_07.6m",    0x008000, 0xE92D9D60, BRF_GRA },			 //  9
	{ "a77_06.6n",    0x008000, 0x5FAEEEA3, BRF_GRA },			 // 10
	{ "a77_05.6p",    0x008000, 0x974E2EA9, BRF_GRA },			 // 11

	{ "21_82s129.12q",0x000100, 0xA0EFAF99, BRF_GRA },			 // 12
	{ "20_82s129.12m",0x000100, 0xA56D57E5, BRF_GRA },			 // 13
	{ "19_82s129.12n",0x000100, 0x5CBF9FBF, BRF_GRA },			 // 14

	{ "a77_02.12d",   0x002000, 0x87F4705A, BRF_ESS | BRF_PRG }, // 15

	{ "a77_13.6a",    0x000800, 0xa70c81d9, BRF_ESS | BRF_PRG }, // 16 MCU ROM
};


STD_ROM_PICK(slapfigh)
STD_ROM_FN(slapfigh)

static struct BurnRomInfo slapbtjpRomDesc[] = {
	{ "sf_r19jb.bin", 0x008000, 0x9A7AC8B3, BRF_ESS | BRF_PRG }, //  0 CPU #0 code
	{ "sf_rh.bin",    0x008000, 0x3C42E4A7, BRF_ESS | BRF_PRG }, //  1

	{ "sf_r03.bin",   0x008000, 0x8545D397, BRF_GRA },			 //  2 Sprite data
	{ "sf_r01.bin",   0x008000, 0xB1B7B925, BRF_GRA },			 //  3
	{ "sf_r04.bin",   0x008000, 0x422D946B, BRF_GRA },			 //  4
	{ "sf_r02.bin",   0x008000, 0x587113AE, BRF_GRA },			 //  5

	{ "sf_r11.bin",   0x002000, 0x2AC7B943, BRF_GRA },			 //  6 Text layer
	{ "sf_r10.bin",   0x002000, 0x33CADC93, BRF_GRA },			 //  7

	{ "sf_r06.bin",   0x008000, 0xB6358305, BRF_GRA },			 //  8 Background layer
	{ "sf_r09.bin",   0x008000, 0xE92D9D60, BRF_GRA },			 //  9
	{ "sf_r08.bin",   0x008000, 0x5FAEEEA3, BRF_GRA },			 // 10
	{ "sf_r07.bin",   0x008000, 0x974E2EA9, BRF_GRA },			 // 11

	{ "sf_col21.bin", 0x000100, 0xA0EFAF99, BRF_GRA },			 // 12
	{ "sf_col20.bin", 0x000100, 0xA56D57E5, BRF_GRA },			 // 13
	{ "sf_col19.bin", 0x000100, 0x5CBF9FBF, BRF_GRA },			 // 14

	{ "sf_r05.bin",   0x002000, 0x87F4705A, BRF_ESS | BRF_PRG }, // 15
};


STD_ROM_PICK(slapbtjp)
STD_ROM_FN(slapbtjp)

static struct BurnRomInfo slapbtukRomDesc[] = {
	{ "sf_r19eb.bin", 0x004000, 0x2efe47af, BRF_ESS | BRF_PRG }, //  0 CPU #0 code
	{ "sf_r20eb.bin", 0x004000, 0xf42c7951, BRF_ESS | BRF_PRG }, //  1
	{ "sf_rh.bin",    0x008000, 0x3C42E4A7, BRF_ESS | BRF_PRG }, //  1

	{ "sf_r03.bin",   0x008000, 0x8545D397, BRF_GRA },			 //  2 Sprite data
	{ "sf_r01.bin",   0x008000, 0xB1B7B925, BRF_GRA },			 //  3
	{ "sf_r04.bin",   0x008000, 0x422D946B, BRF_GRA },			 //  4
	{ "sf_r02.bin",   0x008000, 0x587113AE, BRF_GRA },			 //  5

	{ "sf_r11.bin",   0x002000, 0x2AC7B943, BRF_GRA },			 //  6 Text layer
	{ "sf_r10.bin",   0x002000, 0x33CADC93, BRF_GRA },			 //  7

	{ "sf_r06.bin",   0x008000, 0xB6358305, BRF_GRA },			 //  8 Background layer
	{ "sf_r09.bin",   0x008000, 0xE92D9D60, BRF_GRA },			 //  9
	{ "sf_r08.bin",   0x008000, 0x5FAEEEA3, BRF_GRA },			 // 10
	{ "sf_r07.bin",   0x008000, 0x974E2EA9, BRF_GRA },			 // 11

	{ "sf_col21.bin", 0x000100, 0xA0EFAF99, BRF_GRA },			 // 12
	{ "sf_col20.bin", 0x000100, 0xA56D57E5, BRF_GRA },			 // 13
	{ "sf_col19.bin", 0x000100, 0x5CBF9FBF, BRF_GRA },			 // 14

	{ "sf_r05.bin",   0x002000, 0x87F4705A, BRF_ESS | BRF_PRG }, // 15
};


STD_ROM_PICK(slapbtuk)
STD_ROM_FN(slapbtuk)

static struct BurnRomInfo slapfgtrRomDesc[] = {
	{ "k1-10.u90",    0x004000, 0x2efe47af, BRF_ESS | BRF_PRG }, //  0 CPU #0 code
	{ "k1-09.u89",    0x004000, 0x17c187c5, BRF_ESS | BRF_PRG }, //  1
	{ "k1-08.u88",    0x002000, 0x945af97f, BRF_ESS | BRF_PRG }, //  1
	{ "k1-07.u87",    0x008000, 0x3C42E4A7, BRF_ESS | BRF_PRG }, //  1

	{ "k1-15.u60",    0x008000, 0x8545D397, BRF_GRA },			 //  2 Sprite data
	{ "k1-13.u50",    0x008000, 0xB1B7B925, BRF_GRA },			 //  3
	{ "k1-14.u59",    0x008000, 0x422D946B, BRF_GRA },			 //  4
	{ "k1-12.u49",    0x008000, 0x587113AE, BRF_GRA },			 //  5

	{ "k1-02.u57" ,   0x002000, 0x2AC7B943, BRF_GRA },			 //  6 Text layer
	{ "k1-03.u58",    0x002000, 0x33CADC93, BRF_GRA },			 //  7

	{ "k1-01.u49" ,   0x008000, 0xB6358305, BRF_GRA },			 //  8 Background layer
	{ "k1-04.u62",    0x008000, 0xE92D9D60, BRF_GRA },			 //  9
	{ "k1-05.u63",    0x008000, 0x5FAEEEA3, BRF_GRA },			 // 10
	{ "k1-06.u64",    0x008000, 0x974E2EA9, BRF_GRA },			 // 11

	{ "sf_col21.bin", 0x000100, 0xA0EFAF99, BRF_GRA },			 // 12
	{ "sf_col20.bin", 0x000100, 0xA56D57E5, BRF_GRA },			 // 13
	{ "sf_col19.bin", 0x000100, 0x5CBF9FBF, BRF_GRA },			 // 14

	{ "k1-11.u89",    0x002000, 0x87F4705A, BRF_ESS | BRF_PRG }, // 15
};


STD_ROM_PICK(slapfgtr)
STD_ROM_FN(slapfgtr)

struct BurnDriver BurnDrvTigerH = {
	"tigerh", NULL, NULL, NULL, "1985",
	"Tiger Heli (US)\0", "Protection MCU not emulated", "Taito", "Early Toaplan",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_MISC_PRE90S, GBF_VERSHOOT, 0,
	NULL, tigerhRomInfo, tigerhRomName, NULL, NULL, tigerhInputInfo, tigerhDIPInfo,
	tigerhInit, tigerhExit, tigerhFrame, NULL, tigerhScan, &tigerhRecalcPalette, 0x100,
	240, 280, 3, 4
};

struct BurnDriver BurnDrvTigerhJ = {
	"tigerhj", "tigerh", NULL, NULL, "1985",
	"Tiger Heli (Japan)\0", "Protection MCU not emulated", "Taito", "Early Toaplan",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_CLONE, 2, HARDWARE_MISC_PRE90S, GBF_VERSHOOT, 0,
	NULL, tigerhjRomInfo, tigerhjRomName, NULL, NULL, tigerhInputInfo, tigerhDIPInfo,
	tigerhInit, tigerhExit, tigerhFrame, NULL, tigerhScan, &tigerhRecalcPalette, 0x100,
	240, 280, 3, 4
};

struct BurnDriver BurnDrvTigerHB1 = {
	"tigerhb1", "tigerh", NULL, NULL, "1985",
	"Tiger Heli (bootleg, set 1)\0", NULL, "Taito", "Early Toaplan",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_MISC_PRE90S, GBF_VERSHOOT, 0,
	NULL, tigerhb1RomInfo, tigerhb1RomName, NULL, NULL, tigerhInputInfo, tigerhDIPInfo,
	tigerhInit, tigerhExit, tigerhFrame, NULL, tigerhScan, &tigerhRecalcPalette, 0x100,
	240, 280, 3, 4
};

struct BurnDriver BurnDrvTigerHB2 = {
	"tigerhb2", "tigerh", NULL, NULL, "1985",
	"Tiger Heli (bootleg, set 2)\0", NULL, "Taito", "Early Toaplan",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_MISC_PRE90S, GBF_VERSHOOT, 0,
	NULL, tigerhb2RomInfo, tigerhb2RomName, NULL, NULL, tigerhInputInfo, tigerhDIPInfo,
	tigerhInit, tigerhExit, tigerhFrame, NULL, tigerhScan, &tigerhRecalcPalette, 0x100,
	240, 280, 3, 4
};

struct BurnDriver BurnDrvTigerHB3 = {
	"tigerhb3", "tigerh", NULL, NULL, "1985",
	"Tiger Heli (bootleg, set 3)\0", NULL, "Taito", "Early Toaplan",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_MISC_PRE90S, GBF_VERSHOOT, 0,
	NULL, tigerhb3RomInfo, tigerhb3RomName, NULL, NULL, tigerhInputInfo, tigerhDIPInfo,
	tigerhInit, tigerhExit, tigerhFrame, NULL, tigerhScan, &tigerhRecalcPalette, 0x100,
	240, 280, 3, 4
};

struct BurnDriver BurnDrvGetStar = {
	"getstar", NULL, NULL, NULL, "1986",
	"Guardian\0", "Protection MCU not emulated", "Taito", "Early Toaplan",
	NULL, NULL, NULL, NULL,
	0, 2, HARDWARE_MISC_PRE90S, GBF_SCRFIGHT, 0,
	NULL, getstarRomInfo, getstarRomName, NULL, NULL, tigerhInputInfo, getstarDIPInfo,
	tigerhInit, tigerhExit, tigerhFrame, NULL, tigerhScan, &tigerhRecalcPalette, 0x100,
	280, 240, 4, 3
};

struct BurnDriver BurnDrvGetStarb2 = {
	"gtstarb2", "getstar", NULL, NULL, "1986",
	"Get Star (bootleg, set 2)\0", NULL, "Taito", "Early Toaplan",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_MISC_PRE90S, GBF_SCRFIGHT, 0,
	NULL, gtstarb2RomInfo, gtstarb2RomName, NULL, NULL, tigerhInputInfo, getstarDIPInfo,
	tigerhInit, tigerhExit, tigerhFrame, NULL, tigerhScan, &tigerhRecalcPalette, 0x100,
	280, 240, 4, 3
};

struct BurnDriver BurnDrvGetStarb1 = {
	"gtstarb1", "getstar", NULL, NULL, "1986",
	"Get Star (bootleg, set 1)\0", NULL, "Taito", "Early Toaplan",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_MISC_PRE90S, GBF_SCRFIGHT, 0,
	NULL, gtstarb1RomInfo, gtstarb1RomName, NULL, NULL, gtstarbaInputInfo, gtstarbaDIPInfo,
	tigerhInit, tigerhExit, tigerhFrame, NULL, tigerhScan, &tigerhRecalcPalette, 0x100,
	280, 240, 4, 3
};

struct BurnDriver BurnDrvAlcon = {
	"alcon", NULL, NULL, NULL, "1986",
	"Alcon (US)\0", "Protection MCU not emulated", "Taito America Corp.", "Early Toaplan",
	NULL, NULL, NULL, NULL,
	BDF_ORIENTATION_VERTICAL, 2, HARDWARE_MISC_PRE90S, GBF_VERSHOOT, 0,
	NULL, alconRomInfo, alconRomName, NULL, NULL, tigerhInputInfo, slapfighDIPInfo,
	tigerhInit, tigerhExit, tigerhFrame, NULL, tigerhScan, &tigerhRecalcPalette, 0x100,
	240, 280, 3, 4
};

struct BurnDriverD BurnDrvSlapFigh = {
	"slapfigh", "alcon", NULL, NULL, "1986",
	"Slap Fight (Japan set 1)\0", "Protection MCU not emulated", "Taito", "Early Toaplan",
	NULL, NULL, NULL, NULL,
	BDF_ORIENTATION_VERTICAL | BDF_CLONE, 2, HARDWARE_MISC_PRE90S, GBF_VERSHOOT, 0,
	NULL, slapfighRomInfo, slapfighRomName, NULL, NULL, tigerhInputInfo, slapfighDIPInfo,
	tigerhInit, tigerhExit, tigerhFrame, NULL, tigerhScan, &tigerhRecalcPalette, 0x100,
	240, 280, 3, 4
};

struct BurnDriver BurnDrvSlapBtJP = {
	"slapfighb1", "alcon", NULL, NULL, "1986",
	"Slap Fight (bootleg set 1)\0", NULL, "Taito", "Early Toaplan",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_MISC_PRE90S, GBF_VERSHOOT, 0,
	NULL, slapbtjpRomInfo, slapbtjpRomName, NULL, NULL, tigerhInputInfo, slapfighDIPInfo,
	tigerhInit, tigerhExit, tigerhFrame, NULL, tigerhScan, &tigerhRecalcPalette, 0x100,
	240, 280, 3, 4
};

struct BurnDriver BurnDrvSlapBtUK = {
	"slapfighb2", "alcon", NULL, NULL, "1986",
	"Slap Fight (bootleg set 2)\0", NULL, "Taito", "Early Toaplan",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_MISC_PRE90S, GBF_VERSHOOT, 0,
	NULL, slapbtukRomInfo, slapbtukRomName, NULL, NULL, tigerhInputInfo, slapfighDIPInfo,
	tigerhInit, tigerhExit, tigerhFrame, NULL, tigerhScan, &tigerhRecalcPalette, 0x100,
	240, 280, 3, 4
};

struct BurnDriver BurnDrvSlapFghtr = {
	"slapfighb3", "alcon", NULL, NULL, "1986",
	"Slap Fight (bootleg set 3)\0", NULL, "Taito", "Early Toaplan",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_MISC_PRE90S, GBF_VERSHOOT, 0,
	NULL, slapfgtrRomInfo, slapfgtrRomName, NULL, NULL, tigerhInputInfo, slapfighDIPInfo,
	tigerhInit, tigerhExit, tigerhFrame, NULL, tigerhScan, &tigerhRecalcPalette, 0x100,
	240, 280, 3, 4
};
