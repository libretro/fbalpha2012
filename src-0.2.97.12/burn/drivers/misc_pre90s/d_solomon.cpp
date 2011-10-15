#include "tiles_generic.h"

#include "driver.h"
extern "C" {
 #include "ay8910.h"
}

static unsigned char SolomonInputPort0[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static unsigned char SolomonInputPort1[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static unsigned char SolomonInputPort2[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static unsigned char SolomonDip[2]        = {0, 0};
static unsigned char SolomonInput[3]      = {0x00, 0x00, 0x00};
static unsigned char SolomonReset         = 0;

static unsigned char *Mem                 = NULL;
static unsigned char *MemEnd              = NULL;
static unsigned char *RamStart            = NULL;
static unsigned char *RamEnd              = NULL;
static unsigned char *SolomonZ80Rom1      = NULL;
static unsigned char *SolomonZ80Rom2      = NULL;
static unsigned char *SolomonZ80Ram1      = NULL;
static unsigned char *SolomonZ80Ram2      = NULL;
static unsigned char *SolomonColourRam    = NULL;
static unsigned char *SolomonVideoRam     = NULL;
static unsigned char *SolomonBgColourRam  = NULL;
static unsigned char *SolomonBgVideoRam   = NULL;
static unsigned char *SolomonSpriteRam    = NULL;
static unsigned char *SolomonPaletteRam   = NULL;
static unsigned int  *SolomonPalette      = NULL;
static unsigned char *SolomonBgTiles      = NULL;
static unsigned char *SolomonFgTiles      = NULL;
static unsigned char *SolomonSprites      = NULL;
static unsigned char *SolomonTempRom      = NULL;

static int SolomonIrqFire = 0;

static int SolomonFlipScreen = 0;

static int SolomonSoundLatch = 0;
static short* pFMBuffer;
static short* pAY8910Buffer[9];

static int nCyclesDone[2], nCyclesTotal[2];
static int nCyclesSegment;

static struct BurnInputInfo SolomonInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL  , SolomonInputPort2 + 2, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL  , SolomonInputPort2 + 0, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL  , SolomonInputPort2 + 3, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL  , SolomonInputPort2 + 1, "p2 start"  },

	{"Up"                , BIT_DIGITAL  , SolomonInputPort0 + 2, "p1 up"     },
	{"Down"              , BIT_DIGITAL  , SolomonInputPort0 + 3, "p1 down"   },
	{"Left"              , BIT_DIGITAL  , SolomonInputPort0 + 1, "p1 left"   },
	{"Right"             , BIT_DIGITAL  , SolomonInputPort0 + 0, "p1 right"  },
	{"Fire 1"            , BIT_DIGITAL  , SolomonInputPort0 + 5, "p1 fire 1" },
	{"Fire 2"            , BIT_DIGITAL  , SolomonInputPort0 + 4, "p1 fire 2" },

	{"Up (Cocktail)"     , BIT_DIGITAL  , SolomonInputPort1 + 2, "p2 up"     },
	{"Down (Cocktail)"   , BIT_DIGITAL  , SolomonInputPort1 + 3, "p2 down"   },
	{"Left (Cocktail)"   , BIT_DIGITAL  , SolomonInputPort1 + 1, "p2 left"   },
	{"Right (Cocktail)"  , BIT_DIGITAL  , SolomonInputPort1 + 0, "p2 right"  },
	{"Fire 1 (Cocktail)" , BIT_DIGITAL  , SolomonInputPort1 + 5, "p2 fire 1" },
	{"Fire 2 (Cocktail)" , BIT_DIGITAL  , SolomonInputPort1 + 4, "p2 fire 2" },

	{"Reset"             , BIT_DIGITAL  , &SolomonReset        , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH, SolomonDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH, SolomonDip + 1       , "dip"       },
};

STDINPUTINFO(Solomon)

inline void SolomonClearOpposites(unsigned char* nJoystickInputs)
{
	if ((*nJoystickInputs & 0x03) == 0x03) {
		*nJoystickInputs &= ~0x03;
	}
	if ((*nJoystickInputs & 0x0c) == 0x0c) {
		*nJoystickInputs &= ~0x0c;
	}
}

inline void SolomonMakeInputs()
{
	// Reset Inputs
	SolomonInput[0] = SolomonInput[1] = SolomonInput[2] = 0x00;

	// Compile Digital Inputs
	for (int i = 0; i < 8; i++) {
		SolomonInput[0] |= (SolomonInputPort0[i] & 1) << i;
		SolomonInput[1] |= (SolomonInputPort1[i] & 1) << i;
		SolomonInput[2] |= (SolomonInputPort2[i] & 1) << i;
	}

	// Clear Opposites
	SolomonClearOpposites(&SolomonInput[0]);
	SolomonClearOpposites(&SolomonInput[1]);
}

static struct BurnDIPInfo SolomonDIPList[]=
{
	// Default Values
	{0x11, 0xff, 0xff, 0x02, NULL                     },
	{0x12, 0xff, 0xff, 0x00, NULL                     },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Demo Sounds"            },
	{0x11, 0x01, 0x01, 0x01, "Off"                    },
	{0x11, 0x01, 0x01, 0x00, "On"                     },

	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x11, 0x01, 0x02, 0x02, "Upright"                },
	{0x11, 0x01, 0x02, 0x00, "Cocktail"               },

	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x11, 0x01, 0x0c, 0x0c, "2"                      },
	{0x11, 0x01, 0x0c, 0x00, "3"                      },
	{0x11, 0x01, 0x0c, 0x08, "4"                      },
	{0x11, 0x01, 0x0c, 0x04, "5"                      },

	{0   , 0xfe, 0   , 4   , "Coin A"                 },
	{0x11, 0x01, 0xc0, 0x80, "2 Coins 1 Play"         },
	{0x11, 0x01, 0xc0, 0x00, "1 Coin  1 Play"         },
	{0x11, 0x01, 0xc0, 0x40, "1 Coin  2 Play"         },
	{0x11, 0x01, 0xc0, 0xc0, "1 Coin  3 Play"         },

	{0   , 0xfe, 0   , 4   , "Coin B"                 },
	{0x11, 0x01, 0x30, 0x20, "2 Coins 1 Play"         },
	{0x11, 0x01, 0x30, 0x00, "1 Coin  1 Play"         },
	{0x11, 0x01, 0x30, 0x10, "1 Coin  2 Play"         },
	{0x11, 0x01, 0x30, 0x30, "1 Coin  3 Play"         },

	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"             },
	{0x12, 0x01, 0x03, 0x02, "Easy"                   },
	{0x12, 0x01, 0x03, 0x00, "Normal"                 },
	{0x12, 0x01, 0x03, 0x01, "Harder"                 },
	{0x12, 0x01, 0x03, 0x03, "Difficult"              },

	{0   , 0xfe, 0   , 4   , "Timer Speed"            },
	{0x12, 0x01, 0x0c, 0x08, "Slow"                   },
	{0x12, 0x01, 0x0c, 0x00, "Normal"                 },
	{0x12, 0x01, 0x0c, 0x04, "Faster"                 },
	{0x12, 0x01, 0x0c, 0x0c, "Fastest"                },

	{0   , 0xfe, 0   , 2   , "Extra"                  },
	{0x12, 0x01, 0x10, 0x00, "Normal"                 },
	{0x12, 0x01, 0x10, 0x10, "Difficult"              },

	{0   , 0xfe, 0   , 8   , "Bonus Life"             },
	{0x12, 0x01, 0xe0, 0x00, "30k  200k 500k"         },
	{0x12, 0x01, 0xe0, 0x80, "100k 300k 800k"         },
	{0x12, 0x01, 0xe0, 0x40, "30k  200k"              },
	{0x12, 0x01, 0xe0, 0xc0, "100k 300k"              },
	{0x12, 0x01, 0xe0, 0x20, "30k"                    },
	{0x12, 0x01, 0xe0, 0xa0, "100k"                   },
	{0x12, 0x01, 0xe0, 0x60, "200k"                   },
	{0x12, 0x01, 0xe0, 0xe0, "None"                   },
};

STDDIPINFO(Solomon)

static struct BurnRomInfo SolomonRomDesc[] = {
	{ "6.3f",          0x04000, 0x645eb0f3, BRF_ESS | BRF_PRG }, //  0	Z80 #1 Program Code
	{ "7.3h",          0x08000, 0x1bf5c482, BRF_ESS | BRF_PRG }, //  1	Z80 #1 Program Code
	{ "8.3jk",         0x08000, 0x0a6cdefc, BRF_ESS | BRF_PRG }, //  2	Z80 #1 Program Code

	{ "1.3jk",         0x04000, 0xfa6e562e, BRF_ESS | BRF_PRG }, //  3	Z80 #2 Program Code

	{ "12.3t",         0x08000, 0xb371291c, BRF_GRA },			 //  4	Characters
	{ "11.3r",         0x08000, 0x6f94d2af, BRF_GRA },			 //  5	Characters
	{ "10.3p",         0x08000, 0x8310c2a1, BRF_GRA },			 //  6	Characters
	{ "9.3m",          0x08000, 0xab7e6c42, BRF_GRA },			 //  7	Characters
	{ "2.5lm",         0x04000, 0x80fa2be3, BRF_GRA },			 //  8	Sprites
	{ "3.6lm",         0x04000, 0x236106b4, BRF_GRA },			 //  9	Sprites
	{ "4.7lm",         0x04000, 0x088fe5d9, BRF_GRA },			 //  10	Sprites
	{ "5.8lm",         0x04000, 0x8366232a, BRF_GRA },			 //  11	Sprites
};


STD_ROM_PICK(Solomon)
STD_ROM_FN(Solomon)

static struct BurnRomInfo SolomonjRomDesc[] = {
	{ "slmn_06.bin",   0x04000, 0xe4d421ff, BRF_ESS | BRF_PRG }, //  0	Z80 #1 Program Code
	{ "slmn_07.bin",   0x08000, 0xd52d7e38, BRF_ESS | BRF_PRG }, //  1	Z80 #1 Program Code
	{ "slmn_08.bin",   0x01000, 0xb924d162, BRF_ESS | BRF_PRG }, //  2	Z80 #1 Program Code

	{ "slmn_01.bin",   0x04000, 0xfa6e562e, BRF_ESS | BRF_PRG }, //  3	Z80 #2 Program Code

	{ "slmn_12.bin",   0x08000, 0xaa26dfcb, BRF_GRA },			 //  4	Characters
	{ "slmn_11.bin",   0x08000, 0x6f94d2af, BRF_GRA },			 //  5	Characters
	{ "slmn_10.bin",   0x08000, 0x8310c2a1, BRF_GRA },			 //  6	Characters
	{ "slmn_09.bin",   0x08000, 0xab7e6c42, BRF_GRA },			 //  7	Characters
	{ "slmn_02.bin",   0x04000, 0x80fa2be3, BRF_GRA },			 //  8	Sprites
	{ "slmn_03.bin",   0x04000, 0x236106b4, BRF_GRA },			 //  9	Sprites
	{ "slmn_04.bin",   0x04000, 0x088fe5d9, BRF_GRA },			 //  10	Sprites
	{ "slmn_05.bin",   0x04000, 0x8366232a, BRF_GRA },			 //  11	Sprites
};


STD_ROM_PICK(Solomonj)
STD_ROM_FN(Solomonj)

int SolomonDoReset()
{
	SolomonIrqFire = 0;
	SolomonFlipScreen = 0;
	SolomonSoundLatch = 0;

	for (int i = 0; i < 2; i++) {
		ZetOpen(i);
		ZetReset();
		ZetClose();
	}

	for (int i = 0; i < 3; i++) {
		AY8910Reset(i);
	}

	return 0;
}

unsigned char __fastcall SolomonRead1(unsigned short a)
{
	switch (a) {
		case 0xe600: {
			return SolomonInput[0];
		}

		case 0xe601: {
			return SolomonInput[1];
		}

		case 0xe602: {
			return SolomonInput[2];
		}

		case 0xe604: {
			return SolomonDip[0];
		}

		case 0xe605: {
			return SolomonDip[1];
		}
	}

	return 0;
}

void __fastcall SolomonWrite1(unsigned short a, unsigned char d)
{
	switch (a) {
		case 0xe600: {
			SolomonIrqFire = d;
			return;
		}

		case 0xe604: {
			SolomonFlipScreen = d & 1;
			return;
		}

		case 0xe800: {
			SolomonSoundLatch = d;
			ZetClose();
			ZetOpen(1);
			ZetNmi();
			ZetClose();
			ZetOpen(0);
			return;
		}
	}
}

unsigned char __fastcall SolomonRead2(unsigned short a)
{
	switch (a) {
		case 0x8000: {
			return SolomonSoundLatch;
		}
	}

	return 0;
}

void __fastcall SolomonPortWrite2(unsigned short a, unsigned char d)
{
	a &= 0xff;

	switch (a) {
		case 0x10: {
			AY8910Write(0, 0, d);
			return;
		}

		case 0x11: {
			AY8910Write(0, 1, d);
			return;
		}

		case 0x20: {
			AY8910Write(1, 0, d);
			return;
		}

		case 0x21: {
			AY8910Write(1, 1, d);
			return;
		}

		case 0x30: {
			AY8910Write(2, 0, d);
			return;
		}

		case 0x31: {
			AY8910Write(2, 1, d);
			return;
		}
	}
}

static int SolomonMemIndex()
{
	unsigned char *Next; Next = Mem;

	SolomonZ80Rom1         = Next; Next += 0x10000;
	SolomonZ80Rom2         = Next; Next += 0x04000;

	RamStart               = Next;

	SolomonZ80Ram1         = Next; Next += 0x01000;
	SolomonZ80Ram2         = Next; Next += 0x00800;
	SolomonColourRam       = Next; Next += 0x00400;
	SolomonVideoRam        = Next; Next += 0x00400;
	SolomonBgColourRam     = Next; Next += 0x00400;
	SolomonBgVideoRam      = Next; Next += 0x00400;
	SolomonSpriteRam       = Next; Next += 0x00080;
	SolomonPaletteRam      = Next; Next += 0x00200;

	RamEnd                 = Next;

	SolomonBgTiles         = Next; Next += 2048 * 8 * 8;
	SolomonFgTiles         = Next; Next += 2048 * 8 * 8;
	SolomonSprites         = Next; Next += 2048 * 8 * 8;
	pFMBuffer              = (short*)Next; Next += nBurnSoundLen * 9 * sizeof(short);
	SolomonPalette         = (unsigned int*)Next; Next += 0x00200 * sizeof(unsigned int);

	MemEnd                 = Next;

	return 0;
}

static int TilePlaneOffsets[4]   = { 0, 1, 2, 3 };
static int TileXOffsets[8]       = { 0, 4, 8, 12, 16, 20, 24, 28 };
static int TileYOffsets[8]       = { 0, 32, 64, 96, 128, 160, 192, 224 };
static int SpritePlaneOffsets[4] = { 0, 131072, 262144, 393216 };
static int SpriteXOffsets[16]    = { 0, 1, 2, 3, 4, 5, 6, 7, 64, 65, 66, 67, 68, 69, 70, 71 };
static int SpriteYOffsets[16]    = { 0, 8, 16, 24, 32, 40, 48, 56, 128, 136, 144, 152, 160, 168, 176, 184 };

int SolomonInit()
{
	int nRet = 0, nLen;

	// Allocate and Blank all required memory
	Mem = NULL;
	SolomonMemIndex();
	nLen = MemEnd - (unsigned char *)0;
	if ((Mem = (unsigned char *)malloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	SolomonMemIndex();

	SolomonTempRom = (unsigned char *)malloc(0x10000);

	// Load Z80 #1 Program Roms
	nRet = BurnLoadRom(SolomonZ80Rom1, 0, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(SolomonTempRom, 1, 1); if (nRet != 0) return 1;
	memcpy(SolomonZ80Rom1 + 0x4000, SolomonTempRom + 0x4000, 0x4000);
	memcpy(SolomonZ80Rom1 + 0x8000, SolomonTempRom + 0x0000, 0x4000);
	memset(SolomonTempRom, 0, 0x10000);
	nRet = BurnLoadRom(SolomonTempRom, 2, 1); if (nRet != 0) return 1;
	memcpy(SolomonZ80Rom1 + 0xf000, SolomonTempRom, 0x1000);
	
	// Load Z80 #2 Program Rom
	nRet = BurnLoadRom(SolomonZ80Rom2, 3, 1); if (nRet != 0) return 1;

	// Load and decode Bg Tiles
	memset(SolomonTempRom, 0, 0x10000);
	nRet = BurnLoadRom(SolomonTempRom + 0x0000, 6, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(SolomonTempRom + 0x8000, 7, 1); if (nRet != 0) return 1;
	GfxDecode(2048, 4, 8, 8, TilePlaneOffsets, TileXOffsets, TileYOffsets, 0x100, SolomonTempRom, SolomonBgTiles);

	// Load and decode Fg Tiles
	memset(SolomonTempRom, 0, 0x10000);
	nRet = BurnLoadRom(SolomonTempRom + 0x0000, 4, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(SolomonTempRom + 0x8000, 5, 1); if (nRet != 0) return 1;
	GfxDecode(2048, 4, 8, 8, TilePlaneOffsets, TileXOffsets, TileYOffsets, 0x100, SolomonTempRom, SolomonFgTiles);

	// Load and decode Sprite Tiles
	memset(SolomonTempRom, 0, 0x10000);
	nRet = BurnLoadRom(SolomonTempRom + 0x0000,  8, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(SolomonTempRom + 0x4000,  9, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(SolomonTempRom + 0x8000, 10, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(SolomonTempRom + 0xc000, 11, 1); if (nRet != 0) return 1;
//	SolomonDecodeSprites(SolomonSprites, 2048, 0x0000, 0x4000, 0x8000, 0xc000);
	GfxDecode(512, 4, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x100, SolomonTempRom, SolomonSprites);

	// Setup the Z80 emulation
	ZetInit(2);
	ZetOpen(0);
	ZetSetReadHandler(SolomonRead1);
	ZetSetWriteHandler(SolomonWrite1);
	ZetMapArea(0x0000, 0xbfff, 0, SolomonZ80Rom1         );
	ZetMapArea(0x0000, 0xbfff, 2, SolomonZ80Rom1         );
	ZetMapArea(0xc000, 0xcfff, 0, SolomonZ80Ram1         );
	ZetMapArea(0xc000, 0xcfff, 1, SolomonZ80Ram1         );
	ZetMapArea(0xc000, 0xcfff, 2, SolomonZ80Ram1         );
	ZetMapArea(0xd000, 0xd3ff, 0, SolomonColourRam       );
	ZetMapArea(0xd000, 0xd3ff, 1, SolomonColourRam       );
	ZetMapArea(0xd000, 0xd3ff, 2, SolomonColourRam       );
	ZetMapArea(0xd400, 0xd7ff, 0, SolomonVideoRam        );
	ZetMapArea(0xd400, 0xd7ff, 1, SolomonVideoRam        );
	ZetMapArea(0xd400, 0xd7ff, 2, SolomonVideoRam        );
	ZetMapArea(0xd800, 0xdbff, 0, SolomonBgColourRam     );
	ZetMapArea(0xd800, 0xdbff, 1, SolomonBgColourRam     );
	ZetMapArea(0xd800, 0xdbff, 2, SolomonBgColourRam     );
	ZetMapArea(0xdc00, 0xdfff, 0, SolomonBgVideoRam      );
	ZetMapArea(0xdc00, 0xdfff, 1, SolomonBgVideoRam      );
	ZetMapArea(0xdc00, 0xdfff, 2, SolomonBgVideoRam      );
	ZetMapArea(0xe000, 0xe07f, 0, SolomonSpriteRam       );
	ZetMapArea(0xe000, 0xe07f, 1, SolomonSpriteRam       );
	ZetMapArea(0xe000, 0xe07f, 2, SolomonSpriteRam       );
	ZetMapArea(0xe400, 0xe5ff, 0, SolomonPaletteRam      );
	ZetMapArea(0xe400, 0xe5ff, 1, SolomonPaletteRam      );
	ZetMapArea(0xe400, 0xe5ff, 2, SolomonPaletteRam      );
	ZetMapArea(0xf000, 0xffff, 0, SolomonZ80Rom1 + 0xf000);
	ZetMapArea(0xf000, 0xffff, 2, SolomonZ80Rom1 + 0xf000);
	ZetMemEnd();
	ZetClose();

	ZetOpen(1);
	ZetSetReadHandler(SolomonRead2);
	ZetSetOutHandler(SolomonPortWrite2);
	ZetMapArea(0x0000, 0x3fff, 0, SolomonZ80Rom2         );
	ZetMapArea(0x0000, 0x3fff, 2, SolomonZ80Rom2         );
	ZetMapArea(0x4000, 0x47ff, 0, SolomonZ80Ram2         );
	ZetMapArea(0x4000, 0x47ff, 1, SolomonZ80Ram2         );
	ZetMapArea(0x4000, 0x47ff, 2, SolomonZ80Ram2         );
	ZetMemEnd();
	ZetClose();

	free(SolomonTempRom);

	pAY8910Buffer[0] = pFMBuffer + nBurnSoundLen * 0;
	pAY8910Buffer[1] = pFMBuffer + nBurnSoundLen * 1;
	pAY8910Buffer[2] = pFMBuffer + nBurnSoundLen * 2;
	pAY8910Buffer[3] = pFMBuffer + nBurnSoundLen * 3;
	pAY8910Buffer[4] = pFMBuffer + nBurnSoundLen * 4;
	pAY8910Buffer[5] = pFMBuffer + nBurnSoundLen * 5;
	pAY8910Buffer[6] = pFMBuffer + nBurnSoundLen * 6;
	pAY8910Buffer[7] = pFMBuffer + nBurnSoundLen * 7;
	pAY8910Buffer[8] = pFMBuffer + nBurnSoundLen * 8;

	AY8910Init(0, 1500000, nBurnSoundRate, NULL, NULL, NULL, NULL);
	AY8910Init(1, 1500000, nBurnSoundRate, NULL, NULL, NULL, NULL);
	AY8910Init(2, 1500000, nBurnSoundRate, NULL, NULL, NULL, NULL);

	GenericTilesInit();

	// Reset the driver
	SolomonDoReset();

	return 0;
}

int SolomonExit()
{
	ZetExit();

	for (int i = 0; i < 3; i++) {
		AY8910Exit(i);
	}

	GenericTilesExit();

	free(Mem);
	Mem = NULL;

	return 0;
}

void SolomonRenderBgLayer()
{
	for (int Offs = 0; Offs < 0x400; Offs++) {
		int sx, sy, Attr, Code, Colour, FlipX, FlipY;

		sx = (Offs % 32);
		sy = (Offs / 32);
		Attr = SolomonBgColourRam[Offs];
		Code = SolomonBgVideoRam[Offs] + 256 * (Attr & 0x07);
		Colour = (Attr & 0x70) >> 4;
		FlipX = Attr & 0x80;
		FlipY = Attr & 0x08;

		if (SolomonFlipScreen) {
			sx = 31 - sx;
			sy = 31 - sy;
			FlipX = !FlipX;
			FlipY = !FlipY;
		}

		sx *= 8;
		sy *= 8;
		sy -= 16;

		if (sx >= 0 && sx < 247 && sy >= 0 && sy < 215) {
			if (!FlipY) {
				if (!FlipX) {
					Render8x8Tile_Mask(pTransDraw, Code, sx, sy, Colour, 4, 0, 128, SolomonBgTiles);
				} else {
					Render8x8Tile_Mask_FlipX(pTransDraw, Code, sx, sy, Colour, 4, 0, 128, SolomonBgTiles);
				}
			} else {
				if (!FlipX) {
					Render8x8Tile_Mask_FlipY(pTransDraw, Code, sx, sy, Colour, 4, 0, 128, SolomonBgTiles);
				} else {
					Render8x8Tile_Mask_FlipXY(pTransDraw, Code, sx, sy, Colour, 4, 0, 128, SolomonBgTiles);
				}
			}
		} else {
			if (!FlipY) {
				if (!FlipX) {
					Render8x8Tile_Mask_Clip(pTransDraw, Code, sx, sy, Colour, 4, 0, 128, SolomonBgTiles);
				} else {
					Render8x8Tile_Mask_FlipX_Clip(pTransDraw, Code, sx, sy, Colour, 4, 0, 128, SolomonBgTiles);
				}
			} else {
				if (!FlipX) {
					Render8x8Tile_Mask_FlipY_Clip(pTransDraw, Code, sx, sy, Colour, 4, 0, 128, SolomonBgTiles);
				} else {
					Render8x8Tile_Mask_FlipXY_Clip(pTransDraw, Code, sx, sy, Colour, 4, 0, 128, SolomonBgTiles);
				}
			}
		}
	}
}

void SolomonRenderFgLayer()
{
	for (int Offs = 0x400 - 1; Offs >= 0; Offs--) {
		int sx, sy, Code, Colour;

		sx = (Offs % 32);
		sy = (Offs / 32);
		Code = SolomonVideoRam[Offs] + 256 * (SolomonColourRam[Offs] & 0x07);
		Colour = (SolomonColourRam[Offs] & 0x70) >> 4;

		if (SolomonFlipScreen) {
			sx = 31 - sx;
			sy = 31 - sy;
		}

		sx *= 8;
		sy *= 8;
		sy -= 16;

		if (sx >= 0 && sx < 247 && sy >= 0 && sy < 215) {
			if (!SolomonFlipScreen) {
				Render8x8Tile_Mask(pTransDraw, Code, sx, sy, Colour, 4, 0, 0, SolomonFgTiles);
			} else {
				Render8x8Tile_Mask_FlipXY(pTransDraw, Code, sx, sy, Colour, 4, 0, 0, SolomonFgTiles);
			}
		} else {
			if (!SolomonFlipScreen) {
				Render8x8Tile_Mask_Clip(pTransDraw, Code, sx, sy, Colour, 4, 0, 0, SolomonFgTiles);
			} else {
				Render8x8Tile_Mask_FlipXY_Clip(pTransDraw, Code, sx, sy, Colour, 4, 0, 0, SolomonFgTiles);
			}
		}
	}
}

void SolomonRenderSpriteLayer()
{
	for (int Offs = 0x80 - 4; Offs >= 0; Offs -= 4) {
		int sx, sy, Attr, Code, Colour, FlipX, FlipY;

		sx = SolomonSpriteRam[Offs + 3];
		sy = 241 - SolomonSpriteRam[Offs + 2];
		Attr = SolomonSpriteRam[Offs + 1];
		Code = SolomonSpriteRam[Offs] + 16 * (Attr & 0x10);
		Colour = (Attr & 0x0e) >> 1;
		FlipX = Attr & 0x40;
		FlipY = Attr & 0x80;

		if (SolomonFlipScreen & 1) {
			sx = 240 - sx;
			sy = 240 - sy;
			FlipX = !FlipX;
			FlipY = !FlipY;
		}

		sy -= 16;

		if (sx >= 0 && sx < 239 && sy >= 0 && sy < 207) {
			if (!FlipY) {
				if (!FlipX) {
					Render16x16Tile_Mask(pTransDraw, Code, sx, sy, Colour, 4, 0, 0, SolomonSprites);
				} else {
					Render16x16Tile_Mask_FlipX(pTransDraw, Code, sx, sy, Colour, 4, 0, 0, SolomonSprites);
				}
			} else {
				if (!FlipX) {
					Render16x16Tile_Mask_FlipY(pTransDraw, Code, sx, sy, Colour, 4, 0, 0, SolomonSprites);
				} else {
					Render16x16Tile_Mask_FlipXY(pTransDraw, Code, sx, sy, Colour, 4, 0, 0, SolomonSprites);
				}
			}
		} else {
			if (!FlipY) {
				if (!FlipX) {
					Render16x16Tile_Mask_Clip(pTransDraw, Code, sx, sy, Colour, 4, 0, 0, SolomonSprites);
				} else {
					Render16x16Tile_Mask_FlipX_Clip(pTransDraw, Code, sx, sy, Colour, 4, 0, 0, SolomonSprites);
				}
			} else {
				if (!FlipX) {
					Render16x16Tile_Mask_FlipY_Clip(pTransDraw, Code, sx, sy, Colour, 4, 0, 0, SolomonSprites);
				} else {
					Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, Code, sx, sy, Colour, 4, 0, 0, SolomonSprites);
				}
			}
		}
	}
}

inline static unsigned int CalcCol(unsigned short nColour)
{
	int r, g, b;

	r = (nColour >> 0) & 0x0f;
	g = (nColour >> 4) & 0x0f;
	b = (nColour >> 8) & 0x0f;

	r = (r << 4) | r;
	g = (g << 4) | g;
	b = (b << 4) | b;

	return BurnHighCol(r, g, b, 0);
}

int SolomonCalcPalette()
{
	for (int i = 0; i < 0x200; i++) {
		SolomonPalette[i / 2] = CalcCol(SolomonPaletteRam[i & ~1] | (SolomonPaletteRam[i | 1] << 8));
	}

	return 0;
}

void SolomonDraw()
{
	BurnTransferClear();
	SolomonCalcPalette();
	SolomonRenderBgLayer();
	SolomonRenderFgLayer();
	SolomonRenderSpriteLayer();
	BurnTransferCopy(SolomonPalette);
}

int SolomonFrame()
{
	int nInterleave = 2;
	int nSoundBufferPos = 0;

	if (SolomonReset) SolomonDoReset();

	SolomonMakeInputs();

	nCyclesTotal[0] = 4000000 / 60;
	nCyclesTotal[1] = 3072000 / 60;
	nCyclesDone[0] = nCyclesDone[1] = 0;

	for (int i = 0; i < nInterleave; i++) {
		int nCurrentCPU, nNext;

		// Run Z80 #1
		nCurrentCPU = 0;
		ZetOpen(nCurrentCPU);
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		nCyclesDone[nCurrentCPU] += ZetRun(nCyclesSegment);
		if (i == 1) if(SolomonIrqFire) ZetNmi();
		ZetClose();

		// Run Z80 #2
		nCurrentCPU = 1;
		ZetOpen(nCurrentCPU);
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		nCyclesSegment = ZetRun(nCyclesSegment);
		nCyclesDone[nCurrentCPU] += nCyclesSegment;
		ZetRaiseIrq(0);
		ZetClose();

		// Render Sound Segment
		if (pBurnSoundOut) {
			int nSample;
			int nSegmentLength = nBurnSoundLen - nSoundBufferPos;
			short* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
			AY8910Update(0, &pAY8910Buffer[0], nSegmentLength);
			AY8910Update(1, &pAY8910Buffer[3], nSegmentLength);
			AY8910Update(2, &pAY8910Buffer[6], nSegmentLength);
			for (int n = 0; n < nSegmentLength; n++) {
				nSample  = pAY8910Buffer[0][n] >> 2;
				nSample += pAY8910Buffer[1][n] >> 2;
				nSample += pAY8910Buffer[2][n] >> 2;
				nSample += pAY8910Buffer[3][n] >> 2;
				nSample += pAY8910Buffer[4][n] >> 2;
				nSample += pAY8910Buffer[5][n] >> 2;
				nSample += pAY8910Buffer[6][n] >> 2;
				nSample += pAY8910Buffer[7][n] >> 2;
				nSample += pAY8910Buffer[8][n] >> 2;

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

	// Make sure the buffer is entirely filled.
	if (pBurnSoundOut) {
		int nSample;
		int nSegmentLength = nBurnSoundLen - nSoundBufferPos;
		short* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
		if (nSegmentLength) {
			AY8910Update(0, &pAY8910Buffer[0], nSegmentLength);
			AY8910Update(1, &pAY8910Buffer[3], nSegmentLength);
			AY8910Update(2, &pAY8910Buffer[6], nSegmentLength);
			for (int n = 0; n < nSegmentLength; n++) {
				nSample  = pAY8910Buffer[0][n] >> 2;
				nSample += pAY8910Buffer[1][n] >> 2;
				nSample += pAY8910Buffer[2][n] >> 2;
				nSample += pAY8910Buffer[3][n] >> 2;
				nSample += pAY8910Buffer[4][n] >> 2;
				nSample += pAY8910Buffer[5][n] >> 2;
				nSample += pAY8910Buffer[6][n] >> 2;
				nSample += pAY8910Buffer[7][n] >> 2;
				nSample += pAY8910Buffer[8][n] >> 2;

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

	if (pBurnDraw) SolomonDraw();

	return 0;
}

static int SolomonScan(int nAction,int *pnMin)
{
	struct BurnArea ba;

	if (pnMin != NULL) {			// Return minimum compatible version
		*pnMin = 0x02945;
	}

	if (nAction & ACB_MEMORY_RAM) {
		memset(&ba, 0, sizeof(ba));
		ba.Data	  = RamStart;
		ba.nLen	  = RamEnd-RamStart;
		ba.szName = "All Ram";
		BurnAcb(&ba);
	}

	if (nAction & ACB_DRIVER_DATA) {
		ZetScan(nAction);			// Scan Z80

		// Scan critical driver variables
		SCAN_VAR(SolomonIrqFire);
		SCAN_VAR(SolomonFlipScreen);
		SCAN_VAR(SolomonSoundLatch);
		SCAN_VAR(SolomonInput);
		SCAN_VAR(SolomonDip);
	}

	return 0;
}

struct BurnDriver BurnDrvSolomon = {
	"solomon", NULL, NULL, NULL, "1986",
	"Solomon's Key (US)\0", NULL, "Tecmo", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_PRE90S, GBF_PUZZLE, 0,
	NULL, SolomonRomInfo, SolomonRomName, NULL, NULL, SolomonInputInfo, SolomonDIPInfo,
	SolomonInit, SolomonExit, SolomonFrame, NULL, SolomonScan,
	NULL, 0x200, 256, 224, 4, 3
};

struct BurnDriver BurnDrvSolomonj = {
	"solomonj", "solomon", NULL, NULL, "1986",
	"Solomon's Key (Japan)\0", NULL, "Tecmo", "Miscellaneous",
	L"Solomon's Key (Japan)\0Solomon's Key \u30BD\u30ED\u30E2\u30F3\u306E\u9375\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_PRE90S, GBF_PUZZLE, 0,
	NULL, SolomonjRomInfo, SolomonjRomName, NULL, NULL, SolomonInputInfo, SolomonDIPInfo,
	SolomonInit, SolomonExit, SolomonFrame, NULL, SolomonScan,
	NULL, 0x200, 256, 224, 4, 3
};
