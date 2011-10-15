// FB Alpha The Speed Rumbler driver module
// Based on MAME driver by Paul Leaman

#include "tiles_generic.h"
#include "m6809_intf.h"
#include "burn_ym2203.h"

static unsigned char *AllMem;
static unsigned char *AllRam;
static unsigned char *RamEnd;
static unsigned char *MemEnd;
static unsigned char *DrvM6809ROM;
static unsigned char *DrvZ80ROM;
static unsigned char *DrvGfxROM0;
static unsigned char *DrvGfxROM1;
static unsigned char *DrvGfxROM2;
static unsigned char *DrvPROM;
static unsigned char *DrvM6809RAM;
static unsigned char *DrvSprRAM;
static unsigned char *DrvSprBuf;
static unsigned char *DrvBgRAM;
static unsigned char *DrvFgRAM;
static unsigned char *DrvPalRAM;
static unsigned char *DrvZ80RAM;

static unsigned int  *DrvPalette;
static unsigned char  DrvRecalc;

static unsigned char *DrvBank;
static unsigned char *DrvScroll;
static unsigned char *flipscreen;
static unsigned char *soundlatch;

static unsigned char DrvJoy1[8];
static unsigned char DrvJoy2[8];
static unsigned char DrvJoy3[8];
static unsigned char DrvDips[2];
static unsigned char DrvInputs[3];
static unsigned char DrvReset;

static struct BurnInputInfo SrumblerInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy1 + 6,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy2 + 3,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy2 + 2,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy2 + 1,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy2 + 0,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p1 fire 2"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy1 + 7,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy1 + 1,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy3 + 3,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy3 + 2,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy3 + 1,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy3 + 0,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy3 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy3 + 5,	"p2 fire 2"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
};

STDINPUTINFO(Srumbler)

static struct BurnDIPInfo SrumblerDIPList[]=
{
	{0x11, 0xff, 0xff, 0xff, NULL			},
	{0x12, 0xff, 0xff, 0x73, NULL			},

	{0   , 0xfe, 0   ,    8, "Coin B"		},
	{0x11, 0x01, 0x07, 0x00, "4 Coins 1 Credits"	},
	{0x11, 0x01, 0x07, 0x01, "3 Coins 1 Credits"	},
	{0x11, 0x01, 0x07, 0x02, "2 Coins 1 Credits"	},
	{0x11, 0x01, 0x07, 0x07, "1 Coin  1 Credits"	},
	{0x11, 0x01, 0x07, 0x06, "1 Coin  2 Credits"	},
	{0x11, 0x01, 0x07, 0x05, "1 Coin  3 Credits"	},
	{0x11, 0x01, 0x07, 0x04, "1 Coin  4 Credits"	},
	{0x11, 0x01, 0x07, 0x03, "1 Coin  6 Credits"	},

	{0   , 0xfe, 0   ,    8, "Coin A"		},
	{0x11, 0x01, 0x38, 0x00, "4 Coins 1 Credits"	},
	{0x11, 0x01, 0x38, 0x08, "3 Coins 1 Credits"	},
	{0x11, 0x01, 0x38, 0x10, "2 Coins 1 Credits"	},
	{0x11, 0x01, 0x38, 0x38, "1 Coin  1 Credits"	},
	{0x11, 0x01, 0x38, 0x30, "1 Coin  2 Credits"	},
	{0x11, 0x01, 0x38, 0x28, "1 Coin  3 Credits"	},
	{0x11, 0x01, 0x38, 0x20, "1 Coin  4 Credits"	},
	{0x11, 0x01, 0x38, 0x18, "1 Coin  6 Credits"	},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x11, 0x01, 0x40, 0x40, "Off"			},
	{0x11, 0x01, 0x40, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x11, 0x01, 0x80, 0x80, "Off"			},
	{0x11, 0x01, 0x80, 0x00, "On"			},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x12, 0x01, 0x03, 0x03, "3"			},
	{0x12, 0x01, 0x03, 0x02, "4"			},
	{0x12, 0x01, 0x03, 0x01, "5"			},
	{0x12, 0x01, 0x03, 0x00, "7"			},

	{0   , 0xfe, 0   ,    2, "Cabinet"		},
	{0x12, 0x01, 0x04, 0x00, "Upright"		},
	{0x12, 0x01, 0x04, 0x04, "Cocktail"		},

	{0   , 0xfe, 0   ,    4, "Bonus Life"		},
	{0x12, 0x01, 0x18, 0x18, "20k 70k and every 70k"},
	{0x12, 0x01, 0x18, 0x10, "30k 80k and every 80k"},
	{0x12, 0x01, 0x18, 0x08, "20k 80k"		},
	{0x12, 0x01, 0x18, 0x00, "30k 80k"		},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x12, 0x01, 0x60, 0x40, "Easy"			},
	{0x12, 0x01, 0x60, 0x60, "Normal"		},
	{0x12, 0x01, 0x60, 0x20, "Difficult"		},
	{0x12, 0x01, 0x60, 0x00, "Very Difficult"	},

	{0   , 0xfe, 0   ,    2, "Allow Continue"	},
	{0x12, 0x01, 0x80, 0x00, "No"			},
	{0x12, 0x01, 0x80, 0x80, "Yes"			},
};

STDDIPINFO(Srumbler)

static void bankswitch(int data)
{
	DrvBank[0] = data;

	for (int i = 0x05; i < 0x10; i++)
	{
		int bank = DrvPROM[(data & 0xf0) | i] | DrvPROM[0x100 | ((data & 0x0f) << 4) | i];

		M6809MapMemory(DrvM6809ROM + bank * 0x1000, 0x1000 * i, 0x1000 * i + 0x0fff, M6809_ROM);
	}
}

void srumbler_main_write(unsigned short address, unsigned char data)
{
	switch (address)
	{
		case 0x4008:
			bankswitch(data);
		return;

		case 0x4009:
			*flipscreen = data & 1;
		return;

		case 0x400a:
		case 0x400b:
		case 0x400c:
		case 0x400d:
			DrvScroll[address - 0x400a] = data;
		return;

		case 0x400e:
			*soundlatch = data;
		return;
	}
}

unsigned char srumbler_main_read(unsigned short address)
{
	switch (address)
	{
		case 0x4008:
			return DrvInputs[0];

		case 0x4009:
			return DrvInputs[1];

		case 0x400a:
			return DrvInputs[2];

		case 0x400b:
			return DrvDips[0];

		case 0x400c:
			return DrvDips[1];
	}

	return 0;
}

void __fastcall srumbler_sound_write(unsigned short address, unsigned char data)
{
	switch (address)
	{
		case 0x8000:
		case 0x8001:
			BurnYM2203Write(0, address & 1, data);
		return;

		case 0xa000:
		case 0xa001:
			BurnYM2203Write(1, address & 1, data);
		return;
	}
}

unsigned char __fastcall srumbler_sound_read(unsigned short address)
{
	if (address == 0xe000) {
		return *soundlatch;
	}

	return 0;
}

inline static int DrvSynchroniseStream(int nSoundRate)
{
	return (long long)(ZetTotalCycles() * nSoundRate / 3000000);
}

inline static double DrvGetTime()
{
	return (double)ZetTotalCycles() / 3000000;
}

static int DrvDoReset()
{
	DrvReset = 0;

	memset (AllRam, 0, RamEnd - AllRam);

	M6809Open(0);
	bankswitch(0);
	M6809Reset();
	M6809Close();

	ZetOpen(0);
	ZetReset();
	ZetClose();

	BurnYM2203Reset();

	return 0;
}

static int MemIndex()
{
	unsigned char *Next; Next = AllMem;

	DrvM6809ROM	= Next; Next += 0x040000;
	DrvZ80ROM	= Next; Next += 0x010000;

	DrvGfxROM0	= Next; Next += 0x020000;
	DrvGfxROM1	= Next; Next += 0x080000;
	DrvGfxROM2	= Next; Next += 0x080000;

	DrvPROM		= Next; Next += 0x000200;

	DrvPalette	= (unsigned int*)Next; Next += 0x00200 * sizeof(int);

	AllRam		= Next;

	DrvM6809RAM	= Next; Next += 0x001e00;
	DrvSprRAM	= Next; Next += 0x000200;
	DrvSprBuf	= Next; Next += 0x000200;

	DrvBgRAM	= Next; Next += 0x002000;
	DrvFgRAM	= Next; Next += 0x001000;
	DrvPalRAM	= Next; Next += 0x000400;

	DrvZ80RAM	= Next; Next += 0x000800;

	DrvBank		= Next; Next += 0x000001;
	DrvScroll	= Next; Next += 0x000004;

	flipscreen	= Next; Next += 0x000001;
	soundlatch	= Next; Next += 0x000001;

	RamEnd		= Next;

	MemEnd		= Next;

	return 0;
}

static int DrvGfxDecode()
{
	int Plane0[2]  = { 0x000004,  0x000000 };
	int Plane1[4]  = { 0x100004,  0x100000, 0x000004, 0x000000 };
	int Plane2[4]  = { 0x180000,  0x100000, 0x080000, 0x000000 };
	int XOffs0[16] = { 0x000, 0x001, 0x002, 0x003, 0x008, 0x009, 0x00a, 0x00b,
			   0x100, 0x101, 0x102, 0x103, 0x108, 0x109, 0x10a, 0x10b };
	int XOffs1[16] = { 0x000, 0x001, 0x002, 0x003, 0x004, 0x005, 0x006, 0x007,
			   0x080, 0x081, 0x082, 0x083, 0x084, 0x085, 0x086, 0x087 };
	int YOffs0[16] = { 0x000, 0x010, 0x020, 0x030, 0x040, 0x050, 0x060, 0x070,
			   0x080, 0x090, 0x0a0, 0x0b0, 0x0c0, 0x0d0, 0x0e0, 0x0f0 };
	int YOffs1[16] = { 0x000, 0x008, 0x010, 0x018, 0x020, 0x028, 0x030, 0x038,
			   0x040, 0x048, 0x050, 0x058, 0x060, 0x068, 0x070, 0x078 };

	unsigned char *tmp = (unsigned char*)malloc(0x40000);
	if (tmp == NULL) {
		return 1;
	}

	memcpy (tmp, DrvGfxROM0, 0x08000);

	GfxDecode(0x400, 2,  8,  8, Plane0, XOffs0, YOffs0, 0x080, tmp, DrvGfxROM0);

	memcpy (tmp, DrvGfxROM1, 0x40000);

	GfxDecode(0x800, 4, 16, 16, Plane1, XOffs0, YOffs0, 0x200, tmp, DrvGfxROM1);

	memcpy (tmp, DrvGfxROM2, 0x40000);

	GfxDecode(0x800, 4, 16, 16, Plane2, XOffs1, YOffs1, 0x100, tmp, DrvGfxROM2);

	free (tmp);

	return 0;
}

static int DrvInit()
{
	AllMem = NULL;
	MemIndex();
	int nLen = MemEnd - (unsigned char *)0;
	if ((AllMem = (unsigned char *)malloc(nLen)) == NULL) return 1;
	memset(AllMem, 0, nLen);
	MemIndex();

	{
		if (BurnLoadRom(DrvM6809ROM + 0x00000,  0, 1)) return 1;
		if (BurnLoadRom(DrvM6809ROM + 0x08000,  1, 1)) return 1;
		if (BurnLoadRom(DrvM6809ROM + 0x10000,  2, 1)) return 1;
		if (BurnLoadRom(DrvM6809ROM + 0x18000,  3, 1)) return 1;
		if (BurnLoadRom(DrvM6809ROM + 0x20000,  4, 1)) return 1;
		if (BurnLoadRom(DrvM6809ROM + 0x28000,  5, 1)) return 1;
		if (BurnLoadRom(DrvM6809ROM + 0x30000,  6, 1)) return 1;
		if (BurnLoadRom(DrvM6809ROM + 0x38000,  7, 1)) return 1;

		if (BurnLoadRom(DrvZ80ROM   + 0x00000,  8, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM0  + 0x00000,  9, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM1  + 0x00000, 10, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1  + 0x08000, 11, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1  + 0x10000, 12, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1  + 0x18000, 13, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1  + 0x20000, 14, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1  + 0x28000, 15, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1  + 0x30000, 16, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1  + 0x38000, 17, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM2  + 0x00000, 18, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM2  + 0x08000, 19, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM2  + 0x10000, 20, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM2  + 0x18000, 21, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM2  + 0x20000, 22, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM2  + 0x28000, 23, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM2  + 0x30000, 24, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM2  + 0x38000, 25, 1)) return 1;

		if (BurnLoadRom(DrvPROM  + 0x00000, 26, 1)) return 1;
		if (BurnLoadRom(DrvPROM  + 0x00100, 27, 1)) return 1;

		for (int i = 0; i < 0x100; i++) {
			DrvPROM[i + 0x000]  = (DrvPROM[i] & 0x03) << 4;
			DrvPROM[i + 0x100] &= 0x0f;
		}

		DrvGfxDecode();
	}

	M6809Init(1);
	M6809Open(0);
	M6809MapMemory(DrvM6809RAM,		0x0000, 0x1dff, M6809_RAM);
	M6809MapMemory(DrvSprRAM,		0x1e00, 0x1fff, M6809_RAM);
	M6809MapMemory(DrvBgRAM,		0x2000, 0x3fff, M6809_RAM);
	M6809MapMemory(DrvFgRAM,		0x5000, 0x5fff, M6809_WRITE);
	M6809MapMemory(DrvPalRAM,		0x7000, 0x73ff, M6809_WRITE);
	M6809SetReadByteHandler(srumbler_main_read);
	M6809SetWriteByteHandler(srumbler_main_write);
	M6809Close();

	ZetInit(1);
	ZetOpen(0);
	ZetMapArea(0x0000, 0x7fff, 0, DrvZ80ROM);
	ZetMapArea(0x0000, 0x7fff, 2, DrvZ80ROM);
	ZetMapArea(0xc000, 0xc7ff, 0, DrvZ80RAM);
	ZetMapArea(0xc000, 0xc7ff, 1, DrvZ80RAM);
	ZetMapArea(0xc000, 0xc7ff, 2, DrvZ80RAM);
	ZetSetWriteHandler(srumbler_sound_write);
	ZetSetReadHandler(srumbler_sound_read);
	ZetMemEnd();
	ZetClose();

	BurnYM2203Init(2, 4000000, NULL, DrvSynchroniseStream, DrvGetTime, 0);
	BurnTimerAttachZet(3000000);

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static int DrvExit()
{
	GenericTilesExit();

	M6809Exit();
	ZetExit();

	BurnYM2203Exit();

	free (AllMem);
	AllMem = NULL;

	return 0;
}

static void draw_background_layer(int type)
{
	int scrollx = ((DrvScroll[1] << 8) | DrvScroll[0]) & 0x3ff;
	int scrolly = ((DrvScroll[3] << 8) | DrvScroll[2]) & 0x3ff;

	int masks[4] = { 0xffff, 0x0000, 0x07ff, 0xf800 }; // 0, 1
	int mask = masks[type];

	for (int offs = 0; offs < 64 * 64; offs++)
	{
		int sy = (offs & 0x3f) << 4;
		int sx = (offs >> 6) << 4;

		sy -= scrolly + 8;
		if (sy < -15) sy += 1024;
		sx -= scrollx + 80;
		if (sx < -15) sx += 1024;

		if (sy >= nScreenHeight || sx >= nScreenWidth) continue;

		int attr  = DrvBgRAM[offs * 2 + 0];
		int code  = DrvBgRAM[offs * 2 + 1] | ((attr & 0x07) << 8);
		int color = attr >> 5;
		int flipy = attr & 0x08;

		int group = (attr & 0x10) >> 4;
		if ((type >> 1) != group) continue;

		{
			color = (color << 4) + 0x80;
			if (flipy) flipy = 0xf0;
			unsigned char *src = DrvGfxROM1 + (code << 8);
			unsigned short *dst;

			if (*flipscreen) {
				flipy ^= 0xff;
				sx = 336 - sx;
				sy = 224 - sy;
			}

			for (int y = 0; y < 16; y++, sy++) {
				if (sy < 0 || sy >= nScreenHeight) continue;

				dst = pTransDraw + sy * nScreenWidth;

				for (int x = 0; x < 16; x++, sx++) {
					if (sx < 0 || sx >= nScreenWidth) continue;

					int pxl = src[((y << 4) | x) ^ flipy];

					if (mask & (1 << pxl)) continue;

					dst[sx] = pxl | color;
				}

				sx -= 16;
			}
		}
	}
}

static void draw_foreground_layer(int priority)
{
	for (int offs = 0; offs < 64 * 32; offs++)
	{
		int sy = (offs & 0x1f) << 3;
		int sx = (offs >> 5) << 3;

		sx -= 80;
		sy -=  8;

		if (sx < -15 || sx >= nScreenWidth || sy >= nScreenHeight) continue;

		int attr = DrvFgRAM[offs * 2 + 0];

		int code  = DrvFgRAM[offs * 2 + 1] | ((attr & 0x03) << 8);
		int color = (attr & 0x3c) >> 2;
		int prio  = (attr & 0x40) >> 6;

		if (prio != priority) continue;

		if (*flipscreen) {
			sx = 344 - sx;
			sy = 232 - sy;

			if (priority) {
				Render8x8Tile_FlipXY_Clip(pTransDraw, code, sx, sy, color, 2, 0x1c0, DrvGfxROM0);
			} else {
				Render8x8Tile_Mask_FlipXY_Clip(pTransDraw, code, sx - 80, sy, color, 2, 3, 0x1c0, DrvGfxROM0);
			}
		} else {
			if (priority) {
				Render8x8Tile_Clip(pTransDraw, code, sx, sy, color, 2, 0x1c0, DrvGfxROM0);
			} else {
				Render8x8Tile_Mask_Clip(pTransDraw, code, sx, sy, color, 2, 3, 0x1c0, DrvGfxROM0);
			}
		}
	}
}

static void draw_sprites()
{
	for (int offs = 0x200-4; offs >= 0; offs -= 4)
	{
		int attr  = DrvSprBuf[offs + 1];
		int code  = DrvSprBuf[offs + 0] | ((attr & 0xe0) << 3);
		int sy    = DrvSprBuf[offs + 2];
		int sx    = DrvSprBuf[offs + 3] | ((attr & 0x01) << 8);
		int color = (attr & 0x1c) >> 2;
		int flipy = (attr & 0x02);
		int flipx = 0;

		if (*flipscreen)
		{
			sx = 496 - sx;
			sy = 240 - sy;
			flipy ^= 2;
			flipx = 1;
		}

		if (flipy) {
			if (flipx) {
				Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, code, sx - 80, sy - 8, color, 4, 15, 0x100, DrvGfxROM2);
			} else {
				Render16x16Tile_Mask_FlipY_Clip(pTransDraw, code, sx - 80, sy - 8, color, 4, 15, 0x100, DrvGfxROM2);
			}
		} else {
			if (flipx) {
				Render16x16Tile_Mask_FlipX_Clip(pTransDraw, code, sx - 80, sy - 8, color, 4, 15, 0x100, DrvGfxROM2);
			} else {
				Render16x16Tile_Mask_Clip(pTransDraw, code, sx - 80, sy - 8, color, 4, 15, 0x100, DrvGfxROM2);
			}
		}
	}
}

static int DrvDraw()
{
	if (DrvRecalc) {
		unsigned char r,g,b;
		for (int i = 0; i < 0x400; i+=2) {
			int d = DrvPalRAM[i + 1] | (DrvPalRAM[i + 0] << 8);

			r = (d >> 12);
			g = (d >>  8) & 0x0f;
			b = (d >>  4) & 0x0f;

			DrvPalette[i >> 1] = BurnHighCol((r << 4) | r, (g << 4) | g, (b << 4) | b, 0);
		}
	}

	if (nSpriteEnable & 1) draw_background_layer(1); // opaque
	else BurnTransferClear();

	if (nBurnLayer & 2) draw_background_layer(3);
	if (nBurnLayer & 4) draw_foreground_layer(0);

	draw_sprites();

	if (nBurnLayer & 1) draw_background_layer(2);
	if (nBurnLayer & 8) draw_foreground_layer(1);

	BurnTransferCopy(DrvPalette);

	return 0;
}

static int DrvFrame()
{
	if (DrvReset) {
		DrvDoReset();
	}

	ZetNewFrame();

	{
		memset (DrvInputs, 0xff, 3);
		for (int i = 0; i < 8; i++) {
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
			DrvInputs[2] ^= (DrvJoy3[i] & 1) << i;
		}
	}

	int nInterleave = 4;
	int nSoundBufferPos = 0;
	int nCyclesTotal[2] = { 1500000 / 60, 3000000 / 60 };
	int nCyclesDone[2] = { 0, 0 };

	M6809Open(0);
	ZetOpen(0);

	for (int i = 0; i < nInterleave; i++) {
		nCyclesDone[0] += M6809Run(nCyclesTotal[0] / nInterleave);
		if (i == (nInterleave / 2) - 1) M6809SetIRQ(1, M6809_IRQSTATUS_AUTO);
		if (i == (nInterleave / 1) - 1) M6809SetIRQ(0, M6809_IRQSTATUS_AUTO);

		BurnTimerUpdate(i * (nCyclesTotal[1] / nInterleave));
		ZetRaiseIrq(0);

		if (pBurnSoundOut) {
			int nSegmentLength = nBurnSoundLen - nSoundBufferPos;
			short* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
			BurnYM2203Update(pSoundBuf, nSegmentLength);
			nSoundBufferPos += nSegmentLength;
		}
	}
	
	BurnTimerEndFrame(nCyclesTotal[1]);
	
	if (pBurnSoundOut) {
		int nSegmentLength = nBurnSoundLen - nSoundBufferPos;
		short* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
		if (nSegmentLength) {
			BurnYM2203Update(pSoundBuf, nSegmentLength);
		}
	}

	ZetClose();
	M6809Close();

	if (pBurnDraw) {
		DrvDraw();
	}

	memcpy (DrvSprBuf, DrvSprRAM, 0x200);

	return 0;
}

static int DrvScan(int nAction,int *pnMin)
{
	struct BurnArea ba;

	if (pnMin) {
		*pnMin = 0x029706;
	}

	if (nAction & ACB_VOLATILE) {
		memset(&ba, 0, sizeof(ba));
		ba.Data	  = AllRam;
		ba.nLen	  = RamEnd - AllRam;
		ba.szName = "All RAM";
		BurnAcb(&ba);
	}

	if (nAction & ACB_DRIVER_DATA) {
		M6809Scan(nAction);
		ZetScan(nAction);

		BurnYM2203Scan(nAction, pnMin);
	}

	{
		M6809Open(0);
		bankswitch(DrvBank[0]);
		M6809Close();
	}

	return 0;
}


// The Speed Rumbler (set 1)

static struct BurnRomInfo srumblerRomDesc[] = {
	{ "14e_sr04.bin",	0x8000, 0xa68ce89c, 1 | BRF_PRG | BRF_ESS }, //  0 M6809 Code
	{ "13e_sr03.bin",	0x8000, 0x87bda812, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "12e_sr02.bin",	0x8000, 0xd8609cca, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "11e_sr01.bin",	0x8000, 0x27ec4776, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "14f_sr09.bin",	0x8000, 0x2146101d, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "13f_sr08.bin",	0x8000, 0x838369a6, 1 | BRF_PRG | BRF_ESS }, //  5
	{ "12f_sr07.bin",	0x8000, 0xde785076, 1 | BRF_PRG | BRF_ESS }, //  6
	{ "11f_sr06.bin",	0x8000, 0xa70f4fd4, 1 | BRF_PRG | BRF_ESS }, //  7

	{ "2f_sr05.bin",	0x8000, 0x0177cebe, 2 | BRF_PRG | BRF_ESS }, //  8 Z80 Code

	{ "6g_sr10.bin",	0x4000, 0xadabe271, 3 | BRF_GRA },           //  9 Characters

	{ "11a_sr11.bin",	0x8000, 0x5fa042ba, 4 | BRF_GRA },           // 10 Tiles
	{ "13a_sr12.bin",	0x8000, 0xa2db64af, 4 | BRF_GRA },           // 11
	{ "14a_sr13.bin",	0x8000, 0xf1df5499, 4 | BRF_GRA },           // 12
	{ "15a_sr14.bin",	0x8000, 0xb22b31b3, 4 | BRF_GRA },           // 13
	{ "11c_sr15.bin",	0x8000, 0xca3a3af3, 4 | BRF_GRA },           // 14
	{ "13c_sr16.bin",	0x8000, 0xc49a4a11, 4 | BRF_GRA },           // 15
	{ "14c_sr17.bin",	0x8000, 0xaa80aaab, 4 | BRF_GRA },           // 16
	{ "15c_sr18.bin",	0x8000, 0xce67868e, 4 | BRF_GRA },           // 17

	{ "15e_sr20.bin",	0x8000, 0x3924c861, 5 | BRF_GRA },           // 18 Sprites
	{ "14e_sr19.bin",	0x8000, 0xff8f9129, 5 | BRF_GRA },           // 19
	{ "15f_sr22.bin",	0x8000, 0xab64161c, 5 | BRF_GRA },           // 20
	{ "14f_sr21.bin",	0x8000, 0xfd64bcd1, 5 | BRF_GRA },           // 21
	{ "15h_sr24.bin",	0x8000, 0xc972af3e, 5 | BRF_GRA },           // 22
	{ "14h_sr23.bin",	0x8000, 0x8c9abf57, 5 | BRF_GRA },           // 23
	{ "15j_sr26.bin",	0x8000, 0xd4f1732f, 5 | BRF_GRA },           // 24
	{ "14j_sr25.bin",	0x8000, 0xd2a4ea4f, 5 | BRF_GRA },           // 25

	{ "63s141.12a",		0x0100, 0x8421786f, 6 | BRF_PRG | BRF_ESS }, // 26 Rom Bank Proms
	{ "63s141.13a",		0x0100, 0x6048583f, 6 | BRF_PRG | BRF_ESS }, // 27

	{ "63s141.8j",		0x0100, 0x1a89a7ff, 0 | BRF_OPT },           // 28 Priority Prom
};

STD_ROM_PICK(srumbler)
STD_ROM_FN(srumbler)

struct BurnDriver BurnDrvSrumbler = {
	"srumbler", NULL, NULL, NULL, "1986",
	"The Speed Rumbler (set 1)\0", NULL, "Capcom", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_MISC_PRE90S, GBF_SHOOT, 0,
	NULL, srumblerRomInfo, srumblerRomName, NULL, NULL, SrumblerInputInfo, SrumblerDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan,
	&DrvRecalc, 0x200, 240, 352, 3, 4
};


// The Speed Rumbler (set 2)

static struct BurnRomInfo srumblr2RomDesc[] = {
	{ "14e_sr04.bin",	0x8000, 0xa68ce89c, 1 | BRF_PRG | BRF_ESS }, //  0 M6809 Code
	{ "rc03.13e",		0x8000, 0xe82f78d4, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "rc02.12e",		0x8000, 0x009a62d8, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "rc01.11e",		0x8000, 0x2ac48d1d, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "rc09.14f",		0x8000, 0x64f23e72, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "rc08.13f",		0x8000, 0x74c71007, 1 | BRF_PRG | BRF_ESS }, //  5
	{ "12f_sr07.bin",	0x8000, 0xde785076, 1 | BRF_PRG | BRF_ESS }, //  6
	{ "11f_sr06.bin",	0x8000, 0xa70f4fd4, 1 | BRF_PRG | BRF_ESS }, //  7

	{ "rc05.2f",		0x8000, 0xea04fa07, 2 | BRF_PRG | BRF_ESS }, //  8 Z80 Code

	{ "6g_sr10.bin",	0x4000, 0xadabe271, 3 | BRF_GRA },           //  9 Characters

	{ "11a_sr11.bin",	0x8000, 0x5fa042ba, 4 | BRF_GRA },           // 10 Tiles
	{ "13a_sr12.bin",	0x8000, 0xa2db64af, 4 | BRF_GRA },           // 11
	{ "14a_sr13.bin",	0x8000, 0xf1df5499, 4 | BRF_GRA },           // 12
	{ "15a_sr14.bin",	0x8000, 0xb22b31b3, 4 | BRF_GRA },           // 13
	{ "11c_sr15.bin",	0x8000, 0xca3a3af3, 4 | BRF_GRA },           // 14
	{ "13c_sr16.bin",	0x8000, 0xc49a4a11, 4 | BRF_GRA },           // 15
	{ "14c_sr17.bin",	0x8000, 0xaa80aaab, 4 | BRF_GRA },           // 16
	{ "15c_sr18.bin",	0x8000, 0xce67868e, 4 | BRF_GRA },           // 17

	{ "15e_sr20.bin",	0x8000, 0x3924c861, 5 | BRF_GRA },           // 18 Sprites
	{ "14e_sr19.bin",	0x8000, 0xff8f9129, 5 | BRF_GRA },           // 19
	{ "15f_sr22.bin",	0x8000, 0xab64161c, 5 | BRF_GRA },           // 20
	{ "14f_sr21.bin",	0x8000, 0xfd64bcd1, 5 | BRF_GRA },           // 21
	{ "15h_sr24.bin",	0x8000, 0xc972af3e, 5 | BRF_GRA },           // 22
	{ "14h_sr23.bin",	0x8000, 0x8c9abf57, 5 | BRF_GRA },           // 23
	{ "15j_sr26.bin",	0x8000, 0xd4f1732f, 5 | BRF_GRA },           // 24
	{ "14j_sr25.bin",	0x8000, 0xd2a4ea4f, 5 | BRF_GRA },           // 25

	{ "63s141.12a",		0x0100, 0x8421786f, 6 | BRF_PRG | BRF_ESS }, // 26 Rom Bank Proms
	{ "63s141.13a",		0x0100, 0x6048583f, 6 | BRF_PRG | BRF_ESS }, // 27

	{ "63s141.8j",		0x0100, 0x1a89a7ff, 0 | BRF_OPT },           // 28 Priority Prom
};

STD_ROM_PICK(srumblr2)
STD_ROM_FN(srumblr2)

struct BurnDriver BurnDrvSrumblr2 = {
	"srumbler2", "srumbler", NULL, NULL, "1986",
	"The Speed Rumbler (set 2)\0", NULL, "Capcom", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_MISC_PRE90S, GBF_SHOOT, 0,
	NULL, srumblr2RomInfo, srumblr2RomName, NULL, NULL, SrumblerInputInfo, SrumblerDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan,
	&DrvRecalc, 0x200, 240, 352, 3, 4
};


// Rush & Crash (Japan)

static struct BurnRomInfo rushcrshRomDesc[] = {
	{ "14e_sr04.bin",	0x8000, 0xa68ce89c, 1 | BRF_PRG | BRF_ESS }, //  0 M6809 Code
	{ "rc03.bin",		0x8000, 0xa49c9be0, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "rc02.12e",		0x8000, 0x009a62d8, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "rc01.11e",		0x8000, 0x2ac48d1d, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "rc09.14f",		0x8000, 0x64f23e72, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "rc08.bin",		0x8000, 0x2c25874b, 1 | BRF_PRG | BRF_ESS }, //  5
	{ "12f_sr07.bin",	0x8000, 0xde785076, 1 | BRF_PRG | BRF_ESS }, //  6
	{ "11f_sr06.bin",	0x8000, 0xa70f4fd4, 1 | BRF_PRG | BRF_ESS }, //  7

	{ "rc05.2f",		0x8000, 0xea04fa07, 2 | BRF_PRG | BRF_ESS }, //  8 Z80 Code

	{ "rc10.bin",		0x4000, 0x0a3c0b0d, 3 | BRF_GRA },           //  9 Characters

	{ "11a_sr11.bin",	0x8000, 0x5fa042ba, 4 | BRF_GRA },           // 10 Tiles
	{ "13a_sr12.bin",	0x8000, 0xa2db64af, 4 | BRF_GRA },           // 11
	{ "14a_sr13.bin",	0x8000, 0xf1df5499, 4 | BRF_GRA },           // 12
	{ "15a_sr14.bin",	0x8000, 0xb22b31b3, 4 | BRF_GRA },           // 13
	{ "11c_sr15.bin",	0x8000, 0xca3a3af3, 4 | BRF_GRA },           // 14
	{ "13c_sr16.bin",	0x8000, 0xc49a4a11, 4 | BRF_GRA },           // 15
	{ "14c_sr17.bin",	0x8000, 0xaa80aaab, 4 | BRF_GRA },           // 16
	{ "15c_sr18.bin",	0x8000, 0xce67868e, 4 | BRF_GRA },           // 17

	{ "15e_sr20.bin",	0x8000, 0x3924c861, 5 | BRF_GRA },           // 18 Sprites
	{ "14e_sr19.bin",	0x8000, 0xff8f9129, 5 | BRF_GRA },           // 19
	{ "15f_sr22.bin",	0x8000, 0xab64161c, 5 | BRF_GRA },           // 20
	{ "14f_sr21.bin",	0x8000, 0xfd64bcd1, 5 | BRF_GRA },           // 21
	{ "15h_sr24.bin",	0x8000, 0xc972af3e, 5 | BRF_GRA },           // 22
	{ "14h_sr23.bin",	0x8000, 0x8c9abf57, 5 | BRF_GRA },           // 23
	{ "15j_sr26.bin",	0x8000, 0xd4f1732f, 5 | BRF_GRA },           // 24
	{ "14j_sr25.bin",	0x8000, 0xd2a4ea4f, 5 | BRF_GRA },           // 25

	{ "63s141.12a",		0x0100, 0x8421786f, 6 | BRF_PRG | BRF_ESS }, // 26 Rom Bank Proms
	{ "63s141.13a",		0x0100, 0x6048583f, 6 | BRF_PRG | BRF_ESS }, // 27

	{ "63s141.8j",		0x0100, 0x1a89a7ff, 0 | BRF_OPT },           // 28 Priority Prom
};

STD_ROM_PICK(rushcrsh)
STD_ROM_FN(rushcrsh)

struct BurnDriver BurnDrvRushcrsh = {
	"rushcrsh", "srumbler", NULL, NULL, "1986",
	"Rush & Crash (Japan)\0", NULL, "Capcom", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_MISC_PRE90S, GBF_SHOOT, 0,
	NULL, rushcrshRomInfo, rushcrshRomName, NULL, NULL, SrumblerInputInfo, SrumblerDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan,
	&DrvRecalc, 0x200, 240, 352, 3, 4
};

