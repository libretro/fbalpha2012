// FB Alpha Dark Seal driver module
// Based on MAME driver by Bryan McPhail

// To do:
// Add support for the Huc6280 & set up the sound hardware

#include "tiles_generic.h"
#include "bitswap.h"

static unsigned char *AllMem;
static unsigned char *MemEnd;
static unsigned char *AllRam;
static unsigned char *RamEnd;
static unsigned char *Drv68KROM;
static unsigned char *DrvH6280ROM;
static unsigned char *DrvSndROM;
static unsigned char *DrvGfxROM0;
static unsigned char *DrvGfxROM1;
static unsigned char *DrvGfxROM2;
static unsigned char *DrvGfxROM3;
static unsigned char *Drv68KRAM;
static unsigned char *DrvSprRAM;
static unsigned char *DrvSprBuf;
static unsigned char *DrvPalRAM;
static unsigned char *DrvPf3RAM;
static unsigned char *DrvPf2RAM;
static unsigned char *DrvPf1RAM;
static unsigned char *DrvPf12RowRAM;
static unsigned char *DrvPf34RowRAM;
static unsigned char *DrvPfCtrlRAM0;
static unsigned char *DrvPfCtrlRAM1;
static unsigned int  *DrvPalette;

static unsigned char *soundlatch;

static unsigned char DrvJoy1[16];
static unsigned char DrvJoy2[16];
static unsigned char DrvDip[2];
static unsigned char DrvReset;
static unsigned short DrvInputs[2];

static unsigned char DrvRecalc = 0;
static int vblank = 0;

static struct BurnInputInfo DarksealInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy2 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 7,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy1 + 15,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy1 + 8,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy1 + 9,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy1 + 10,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy1 + 11,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy1 + 12,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy1 + 13,	"p2 fire 2"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Dip A",		BIT_DIPSWITCH,	DrvDip + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDip + 1,	"dip"		},
};

STDINPUTINFO(Darkseal)

static struct BurnDIPInfo DarksealDIPList[]=
{
	// Default Values
	{0x11, 0xff, 0xff, 0xff, NULL			},
	{0x12, 0xff, 0xff, 0x7f, NULL			},

	{0   , 0xfe, 0   ,    8, "Coin A"		},
	{0x11, 0x01, 0x07, 0x00, "3 Coins 1 Credit"	},
	{0x11, 0x01, 0x07, 0x01, "2 Coins 1 Credit"	},
	{0x11, 0x01, 0x07, 0x07, "1 Coin  1 Credit"	},
	{0x11, 0x01, 0x07, 0x06, "1 Coin  2 Credits"	},
	{0x11, 0x01, 0x07, 0x05, "1 Coin  3 Credits"	},
	{0x11, 0x01, 0x07, 0x04, "1 Coin  4 Credits"	},
	{0x11, 0x01, 0x07, 0x03, "1 Coin  5 Credits"	},
	{0x11, 0x01, 0x07, 0x02, "1 Coin  6 Credits"	},

	{0   , 0xfe, 0   ,    8, "Coin B"		},
	{0x11, 0x01, 0x38, 0x00, "3 Coins 1 Credit"	},
	{0x11, 0x01, 0x38, 0x08, "2 Coins 1 Credit"	},
	{0x11, 0x01, 0x38, 0x38, "1 Coin  1 Credit"	},
	{0x11, 0x01, 0x38, 0x30, "1 Coin  2 Credits"	},
	{0x11, 0x01, 0x38, 0x28, "1 Coin  3 Credits"	},
	{0x11, 0x01, 0x38, 0x20, "1 Coin  4 Credits"	},
	{0x11, 0x01, 0x38, 0x18, "1 Coin  5 Credits"	},
	{0x11, 0x01, 0x38, 0x10, "1 Coin  6 Credits"	},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x11, 0x01, 0x40, 0x40, "Off"			},
	{0x11, 0x01, 0x40, 0x00, "On"			},

	{0   , 0xfe, 0   ,    0, "Lives"		},
	{0x12, 0x01, 0x03, 0x00, "1"			},
	{0x12, 0x01, 0x03, 0x01, "2"			},
	{0x12, 0x01, 0x03, 0x03, "3"			},
	{0x12, 0x01, 0x03, 0x02, "4"			},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x12, 0x01, 0x0c, 0x08, "Easy"			},
	{0x12, 0x01, 0x0c, 0x0c, "Normal"		},
	{0x12, 0x01, 0x0c, 0x04, "Hard"			},
	{0x12, 0x01, 0x0c, 0x00, "Hardest"		},

	{0   , 0xfe, 0   ,    4, "Energy"		},
	{0x12, 0x01, 0x30, 0x00, "2"			},
	{0x12, 0x01, 0x30, 0x10, "2.5"			},
	{0x12, 0x01, 0x30, 0x30, "3"			},
	{0x12, 0x01, 0x30, 0x20, "4"			},

	{0   , 0xfe, 0   ,    4, "Allow Continue"	},
	{0x12, 0x01, 0x40, 0x00, "No"			},
	{0x12, 0x01, 0x40, 0x40, "Yes"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x12, 0x01, 0x80, 0x80, "Off"			},
	{0x12, 0x01, 0x80, 0x00, "On"			},
};

STDDIPINFO(Darkseal)

static inline void palette_write(int offset)
{
	unsigned short *data = (unsigned short*)(DrvPalRAM + offset);
	unsigned char r,g,b;

	r = (data[0x0000/2] >> 0) & 0xff;
	g = (data[0x0000/2] >> 8) & 0xff;
	b = (data[0x1000/2] >> 0) & 0xff;

	DrvPalette[offset/2] = BurnHighCol(r, g, b, 0);
}

void __fastcall darkseal_write_byte(unsigned int address, unsigned char data)
{
	if ((address & 0xfffff0) == 0x180000) {
		switch (address & 0x0e)
		{
			case 0x06:
				memcpy (DrvSprBuf, DrvSprRAM, 0x800);
			return;

			case 0x08:
				*soundlatch = data;
			return;
		}

		return;
	}
}

void __fastcall darkseal_write_word(unsigned int address, unsigned short data)
{
	if ((address & 0xfffff0) == 0x180000) {
		switch (address & 0x0f)
		{
			case 0x06:
				memcpy (DrvSprBuf, DrvSprRAM, 0x800);
			return;

			case 0x08:
				*soundlatch = data & 0xff;
			return;
		}

		return;
	}

	if ((address & 0xfffff0) == 0x240000) {
		*((unsigned short*)(DrvPfCtrlRAM0 + (address & 0x0e))) = data;
		return;
	}

	if ((address & 0xfffff0) == 0x2a0000) {
		*((unsigned short*)(DrvPfCtrlRAM1 + (address & 0x0e))) = data;
		return;
	}
}

unsigned char __fastcall darkseal_read_byte(unsigned int address)
{
	if ((address & 0xfffff0) == 0x180000) {
		switch (address & 0xf)
		{
			case 0:
				return DrvDip[1];

			case 1:
				return DrvDip[0];

			case 2:
				return DrvInputs[0] >> 8;

			case 3:
				return DrvInputs[0] & 0xff;

			case 4:
				return 0xff;

			case 5:
				return (DrvInputs[1] ^ vblank) & 0xff;
		}

		return 0xff;
	}

	return 0;
}

unsigned short __fastcall darkseal_read_word(unsigned int address)
{
	if ((address & 0xfffff0) == 0x180000) {
		switch (address & 0xe)
		{
			case 0:
				return (DrvDip[0] | (DrvDip[1]<<8));

			case 2:
				return DrvInputs[0];

			case 4:
				return DrvInputs[1] ^ vblank;
		}

		return 0xffff;
	}

	return 0;
}

static int DrvDoReset()
{
	DrvReset = 0;

	memset (AllRam, 0, RamEnd - AllRam);

	SekOpen(0);
	SekReset();
	SekClose();

	return 0;
}

static int MemIndex()
{
	unsigned char *Next; Next = AllMem;

	Drv68KROM	= Next; Next += 0x080000;

	DrvH6280ROM	= Next; Next += 0x010000;

	DrvGfxROM0	= Next; Next += 0x040000;
	DrvGfxROM1	= Next; Next += 0x100000;
	DrvGfxROM2	= Next; Next += 0x100000;
	DrvGfxROM3	= Next; Next += 0x200000;

	DrvSndROM	= Next; Next += 0x040000;

	DrvPalette	= (unsigned int*)Next; Next += 0x00800 * sizeof(int);

	AllRam		= Next;

	Drv68KRAM	= Next; Next += 0x004000;
	DrvSprRAM	= Next; Next += 0x000800;
	DrvSprBuf	= Next; Next += 0x000800;
	DrvPalRAM	= Next; Next += 0x002000;
	DrvPf3RAM	= Next; Next += 0x002000;
	DrvPf12RowRAM	= Next; Next += 0x001000;
	DrvPf34RowRAM	= Next; Next += 0x001000;
	DrvPf2RAM	= Next; Next += 0x002000;
	DrvPf1RAM	= Next; Next += 0x002000;
	DrvPfCtrlRAM0	= Next; Next += 0x000010;
	DrvPfCtrlRAM1	= Next; Next += 0x000010;

	soundlatch	= Next; Next += 0x000001;

	RamEnd		= Next;

	MemEnd		= Next;

	return 0;
}

static int DrvGfxDecode()
{
	int Plane0[4]  = { 0x00000*8, 0x10000*8, 0x8000*8, 0x18000*8 };
	int Plane1[4]  = { 8, 0, 0x40000*8+8, 0x40000*8 };
	int Plane2[4]  = { 8, 0, 0x80000*8+8, 0x80000*8 };
	int XOffs0[8]  = { 0, 1, 2, 3, 4, 5, 6, 7 };
	int YOffs0[8]  = { 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 };
	int XOffs1[16] = { 32*8+0, 32*8+1, 32*8+2, 32*8+3, 32*8+4, 32*8+5, 32*8+6, 32*8+7, 0, 1, 2, 3, 4, 5, 6, 7 };
	int YOffs1[16] = { 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16, 8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16 };

	unsigned char *tmp = (unsigned char*)malloc(0x100000);
	if (tmp == NULL) {
		return 1;
	}

	memcpy (tmp, DrvGfxROM0, 0x020000);

	GfxDecode(0x1000, 4,  8,  8, Plane0, XOffs0, YOffs0, 0x040, tmp, DrvGfxROM0);

	memcpy (tmp, DrvGfxROM1, 0x080000);

	GfxDecode(0x1000, 4, 16, 16, Plane1, XOffs1, YOffs1, 0x200, tmp, DrvGfxROM1);

	memcpy (tmp, DrvGfxROM2, 0x080000);

	GfxDecode(0x1000, 4, 16, 16, Plane1, XOffs1, YOffs1, 0x200, tmp, DrvGfxROM2);

	memcpy (tmp, DrvGfxROM3, 0x100000);

	GfxDecode(0x2000, 4, 16, 16, Plane2, XOffs1, YOffs1, 0x200, tmp, DrvGfxROM3);

	free (tmp);

	return 0;
}

static void DrvPrgDecode()
{
	for (int i = 0; i < 0x80000; i++)
		Drv68KROM[i] = BITSWAP08(Drv68KROM[i], 7, 1, 5, 4, 3, 2, 6, 0);
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
		if (BurnLoadRom(Drv68KROM + 0x00001,	0, 2)) return 1;
		if (BurnLoadRom(Drv68KROM + 0x00000,	1, 2)) return 1;
		if (BurnLoadRom(Drv68KROM + 0x40001,	2, 2)) return 1;
		if (BurnLoadRom(Drv68KROM + 0x40000,	3, 2)) return 1;

		if (BurnLoadRom(DrvH6280ROM,		4, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM0 + 0x00000,	5, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM0 + 0x10000,	6, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM1 + 0x00000,	7, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM2 + 0x00000,	8, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM3 + 0x00000,	9, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM3 + 0x80000,  10, 1)) return 1;

		if (BurnLoadRom(DrvSndROM + 0x00000,   11, 1)) return 1;
		if (BurnLoadRom(DrvSndROM + 0x20000,   12, 1)) return 1;

		DrvPrgDecode();
		DrvGfxDecode();
	}

	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM,			0x000000, 0x07ffff, SM_ROM);
	SekMapMemory(Drv68KRAM,			0x100000, 0x103fff, SM_RAM);
	SekMapMemory(DrvSprRAM,			0x120000, 0x1207ff, SM_RAM);
	SekMapMemory(DrvPalRAM,			0x140000, 0x141fff, SM_RAM); // split ram
	SekMapMemory(DrvPf3RAM + 0x001000,	0x200000, 0x200fff, SM_RAM); // mirror
	SekMapMemory(DrvPf3RAM,			0x202000, 0x203fff, SM_RAM);
	SekMapMemory(DrvPf12RowRAM,		0x220000, 0x220fff, SM_RAM);
	SekMapMemory(DrvPf34RowRAM,		0x222000, 0x222fff, SM_RAM);
	SekMapMemory(DrvPf2RAM,			0x260000, 0x261fff, SM_RAM);
	SekMapMemory(DrvPf1RAM,			0x262000, 0x263fff, SM_RAM);
	SekSetWriteWordHandler(0,		darkseal_write_word);
	SekSetWriteByteHandler(0,		darkseal_write_byte);
	SekSetReadWordHandler(0,		darkseal_read_word);
	SekSetReadByteHandler(0,		darkseal_read_byte);
	SekClose();

	// sound hardware...

	BurnSetRefreshRate(58.00);

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static int DrvExit()
{
	GenericTilesExit();

	SekExit();

	free (AllMem);
	AllMem = NULL;

	return 0;
}

static void draw_sprites()
{
	unsigned short *SprRAM = (unsigned short*)DrvSprBuf;

	for (int offs = 0; offs < 0x400; offs += 4)
	{
		int sprite = SprRAM[offs+1] & 0x1fff;
		if (!sprite) continue;

		int y = SprRAM[offs];
		int x = SprRAM[offs+2];

		int flash = ((y >> 12) & 1) & GetCurrentFrame();
		if (flash) continue;

		int color = ((x >> 9) & 0x1f) + 0x10;

		int fx = y & 0x2000;
		int fy = y & 0x4000;
		int multi = (1 << ((y & 0x0600) >> 9)) - 1;

		x &= 0x01ff;
		y &= 0x01ff;
		if (x > 255) x -= 512;
		if (y > 255) y -= 512;
		x =  240 - x;
		y = (240 - y) - 8;

		if (x > 256) continue;

		sprite &= ~multi;

		int inc = -1;

		if (!fy) {
			sprite += multi;
			inc = 1;
		}

		while (multi >= 0)
		{
			if (fy) {
				if (fx) {
					Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, sprite - multi * inc, x, y+-16*multi, color, 4, 0, 0, DrvGfxROM3);
				} else {
					Render16x16Tile_Mask_FlipY_Clip(pTransDraw, sprite - multi * inc, x, y+-16*multi, color, 4, 0, 0, DrvGfxROM3);
				}
			} else {
				if (fx) {
					Render16x16Tile_Mask_FlipX_Clip(pTransDraw, sprite - multi * inc, x, y+-16*multi, color, 4, 0, 0, DrvGfxROM3);
				} else {
					Render16x16Tile_Mask_Clip(pTransDraw, sprite - multi * inc, x, y+-16*multi, color, 4, 0, 0, DrvGfxROM3);
				}
			}

			multi--;
		}
	}
}

static void draw_pf1_layer(int scroll_x, int scroll_y)
{
	unsigned short *vram = (unsigned short*)DrvPf1RAM;

	for (int offs = 0; offs < 64 * 64; offs++)
	{
		int sx = (offs & 0x3f) << 3;
		int sy = (offs >> 6) << 3;

		sx -= scroll_x;
		if (sx < -7) sx += 0x200;
		sy -= scroll_y + 8;
		if (sy < -7) sy += 0x200;

		if (sx >= nScreenWidth || sy >= nScreenHeight) continue;

		int code = vram[offs];
		int color = code >> 12;

		code &= 0xfff;
		if (!code) continue;

		if (sy >= 0 && sx >= 0 && sy <= nScreenHeight-8 && sx <= nScreenWidth-8) {
			Render8x8Tile_Mask(pTransDraw, code, sx, sy, color, 4, 0, 0, DrvGfxROM0);
		} else {
			Render8x8Tile_Mask_Clip(pTransDraw, code, sx, sy, color, 4, 0, 0, DrvGfxROM0);
		}
	}
}

static void draw_pf23_layer_no_rowscroll(unsigned char *ram, unsigned char *gfx_base, int coloffs, int transp, int scroll_x, int scroll_y)
{
	unsigned short *vram = (unsigned short*)ram;

	for (int offs = 0; offs < 64 * 64; offs++)
	{
		int sx = (offs & 0x3f);
		int sy = (offs >> 6);

		int ofst = (sx & 0x1f) + ((sy & 0x1f) << 5) + ((sx & 0x20) << 5) + ((sy & 0x20) << 6);

		sx <<= 4, sy <<= 4;

		sx -= scroll_x;
		if (sx < -15) sx += 0x400;
		sy -= scroll_y + 8;
		if (sy < -15) sy += 0x400;

		if (sx >= nScreenWidth || sy >= nScreenHeight) continue;

		int code  = vram[ofst];
		int color = code >> 12;

		code &= 0xfff;
		if (!code && transp) continue;

		if (transp) {
			if (sy >= 0 && sx >= 0 && sy <= nScreenHeight-16 && sx <= nScreenWidth-16) {
				Render16x16Tile_Mask(pTransDraw, code, sx, sy, color, 4, 0, coloffs, gfx_base);
			} else {
				Render16x16Tile_Mask_Clip(pTransDraw, code, sx, sy, color, 4, 0, coloffs, gfx_base);
			}
		} else {
			if (sy >= 0 && sx >= 0 && sy <= nScreenHeight-16 && sx <= nScreenWidth-16) {
				Render16x16Tile(pTransDraw, code, sx, sy, color, 4, coloffs, gfx_base);
			} else {
				Render16x16Tile_Clip(pTransDraw, code, sx, sy, color, 4, coloffs, gfx_base);
			}
		}
	}
}

static void draw_pf23_layer_rowscroll(int scroll_x, int scroll_y)
{
	unsigned short *vram = (unsigned short*)DrvPf3RAM;
	unsigned short *rows = (unsigned short*)DrvPf34RowRAM;
	unsigned short *dest;

	for (int y = 8; y < 248; y++)
	{
		int row = (scroll_y + y) >> 4;
		int xscr = scroll_x + (rows[0x40+y] & 0x3ff);
		dest = pTransDraw + ((y-8) * nScreenWidth);

		for (int x = 0; x < 256+16; x+=16)
		{
			int col = ((xscr + x) & 0x3ff) >> 4;
			int sx = x - (xscr & 0x0f);
			if (sx < -15) sx += 0x400;
			if (sx < 0) sx += 16;

			int ofst = (col & 0x1f) + ((row & 0x1f) << 5) + ((col & 0x20) << 5) + ((row & 0x20) << 6);

			int code  =   vram[ofst] & 0xfff;
			int color = ((vram[ofst] >> 12) << 4) | 0x400;
			
			unsigned char *src = DrvGfxROM2 + (code << 8) + (((scroll_y + y) & 0x0f) << 4);
			for (int xx = sx; xx < ((sx+16) < nScreenWidth) ? (sx+16) : nScreenWidth; xx++) {
				if (xx >= nScreenWidth) break;
				dest[xx] = src[xx-sx] | color;
			}
		}
	}
}

static int DrvDraw()
{
	if (DrvRecalc) {
		for (int i = 0; i < 0x1000; i+=2) {
			palette_write(i);
		}
	}

	unsigned short *ctrl0 = (unsigned short*)DrvPfCtrlRAM0;
	unsigned short *ctrl1 = (unsigned short*)DrvPfCtrlRAM1;

	int flipscreen = ~ctrl0[0] & 0x80;

	if (ctrl0[6] & 0x4000) {
		draw_pf23_layer_rowscroll(ctrl0[3] & 0x3ff, ctrl0[4] & 0x3ff);
	} else {
		draw_pf23_layer_no_rowscroll(DrvPf3RAM, DrvGfxROM2, 0x400, 0, ctrl0[3] & 0x3ff, ctrl0[4] & 0x3ff);
	}

	draw_pf23_layer_no_rowscroll(DrvPf2RAM, DrvGfxROM1, 0x300, 1, ctrl1[1] & 0x3ff, ctrl1[2] & 0x3ff);

	draw_sprites();

	draw_pf1_layer(ctrl1[3] & 0x1ff, ctrl1[4] & 0x1ff);

	if (flipscreen) {
		int full = nScreenWidth * nScreenHeight;
		for (int i = 0; i < full / 2; i++) {
			int t = pTransDraw[i + 0];
			pTransDraw[i + 0] = pTransDraw[(full-1)-i];
			pTransDraw[(full-1)-i] = t;
		}
	}

	BurnTransferCopy(DrvPalette);

	return 0;
}

static int DrvFrame()
{
	if (DrvReset) {
		DrvDoReset();
	}

	{
		memset (DrvInputs, 0xff, 4);

		for (int i = 0; i < 16; i++) {
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
		}
	}

	int nSegment;
	int nInterleave = 256;
	int nTotalCycles[2] = { 12000000 / 58, 8055000 / 58 };
	int nCyclesDone[2] = { 0, 0 };

	SekOpen(0);

	vblank = 8;

	for (int i = 0; i < nInterleave; i++)
	{
		nSegment = (nTotalCycles[0] - nCyclesDone[0]) / (nInterleave - i);

		nCyclesDone[0] += SekRun(nSegment);

		if (i ==   7) vblank = 0;
		if (i == 247) vblank = 8;
	}

	SekSetIRQLine(6, SEK_IRQSTATUS_AUTO);
	SekClose();

	if (pBurnDraw) {
		DrvDraw();
	}

	return 0;
}

static int DrvScan(int nAction, int *pnMin)
{
	struct BurnArea ba;
	
	if (pnMin != NULL) {
		*pnMin = 0x029698;
	}

	if (nAction & ACB_MEMORY_RAM) {
		memset(&ba, 0, sizeof(ba));
		ba.Data	  = AllRam;
		ba.nLen	  = RamEnd-AllRam;
		ba.szName = "All Ram";
		BurnAcb(&ba);
	}

	if (nAction & ACB_DRIVER_DATA) {
		SekScan(nAction);
	}

	return 0;
}


// Dark Seal (World revision 3)

static struct BurnRomInfo darksealRomDesc[] = {
	{ "ga04-3.rom",	0x20000, 0xbafad556, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "ga01-3.rom",	0x20000, 0xf409050e, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "ga-00.rom",	0x20000, 0xfbf3ac63, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "ga-05.rom",	0x20000, 0xd5e3ae3f, 1 | BRF_PRG | BRF_ESS }, //  3

	{ "fz-06.rom",	0x10000, 0xc4828a6d, 2 | BRF_PRG | BRF_ESS }, //  4 H6280 Code

	{ "fz-02.rom",	0x10000, 0x3c9c3012, 3 | BRF_GRA },           //  5 Text Tiles
	{ "fz-03.rom",	0x10000, 0x264b90ed, 3 | BRF_GRA },           //  6

	{ "mac-03.rom",	0x80000, 0x9996f3dc, 4 | BRF_GRA },           //  7 Foreground Tiles

	{ "mac-02.rom",	0x80000, 0x49504e89, 5 | BRF_GRA },           //  8 Background Tiles

	{ "mac-00.rom",	0x80000, 0x52acf1d6, 6 | BRF_GRA },           //  9 Sprite Tiles
	{ "mac-01.rom",	0x80000, 0xb28f7584, 6 | BRF_GRA },           // 10

	{ "fz-08.rom",	0x20000, 0xc9bf68e1, 7 | BRF_SND },           // 11 Oki6295 #0 Samples

	{ "fz-07.rom",	0x20000, 0x588dd3cb, 8 | BRF_SND },           // 12 Oki6295 #1 Samples
};

STD_ROM_PICK(darkseal)
STD_ROM_FN(darkseal)

struct BurnDriver BurnDrvDarkseal = {
	"darkseal", NULL, NULL, NULL, "1990",
	"Dark Seal (World revision 3)\0", NULL, "Data East Corporation", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_MAZE | GBF_SCRFIGHT, 0,
	NULL, darksealRomInfo, darksealRomName, NULL, NULL, DarksealInputInfo, DarksealDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x800,
	256, 240, 4, 3
};


// Dark Seal (World revision 1)

static struct BurnRomInfo darksea1RomDesc[] = {
	{ "ga-04.rom",	0x20000, 0xa1a985a9, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "ga-01.rom",	0x20000, 0x98bd2940, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "ga-00.rom",	0x20000, 0xfbf3ac63, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "ga-05.rom",	0x20000, 0xd5e3ae3f, 1 | BRF_PRG | BRF_ESS }, //  3

	{ "fz-06.rom",	0x10000, 0xc4828a6d, 2 | BRF_PRG | BRF_ESS }, //  4 H6280 Code

	{ "fz-02.rom",	0x10000, 0x3c9c3012, 3 | BRF_GRA },           //  5 Text Tiles
	{ "fz-03.rom",	0x10000, 0x264b90ed, 3 | BRF_GRA },           //  6

	{ "mac-03.rom",	0x80000, 0x9996f3dc, 4 | BRF_GRA },           //  7 Foreground Tiles

	{ "mac-02.rom",	0x80000, 0x49504e89, 5 | BRF_GRA },           //  8 Background Tiles

	{ "mac-00.rom",	0x80000, 0x52acf1d6, 6 | BRF_GRA },           //  9 Sprite Tiles
	{ "mac-01.rom",	0x80000, 0xb28f7584, 6 | BRF_GRA },           // 10

	{ "fz-08.rom",	0x20000, 0xc9bf68e1, 7 | BRF_SND },           // 11 Oki6295 #0 Samples

	{ "fz-07.rom",	0x20000, 0x588dd3cb, 8 | BRF_SND },           // 12 Oki6295 #1 Samples
};

STD_ROM_PICK(darksea1)
STD_ROM_FN(darksea1)

struct BurnDriver BurnDrvDarksea1 = {
	"darkseal1", "darkseal", NULL, NULL, "1990",
	"Dark Seal (World revision 1)\0", NULL, "Data East Corporation", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_POST90S, GBF_MAZE | GBF_SCRFIGHT, 0,
	NULL, darksea1RomInfo, darksea1RomName, NULL, NULL, DarksealInputInfo, DarksealDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x800,
	256, 240, 4, 3
};


// Dark Seal (Japan)

static struct BurnRomInfo darkseajRomDesc[] = {
	{ "fz-04.bin",	0x20000, 0x817faa2c, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "fz-01.bin",	0x20000, 0x373caeee, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "fz-00.bin",	0x20000, 0x1ab99aa7, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "fz-05.bin",	0x20000, 0x3374ef8c, 1 | BRF_PRG | BRF_ESS }, //  3

	{ "fz-06.rom",	0x10000, 0xc4828a6d, 2 | BRF_PRG | BRF_ESS }, //  4 H6280 Code

	{ "fz-02.rom",	0x10000, 0x3c9c3012, 3 | BRF_GRA },           //  5 Text Tiles
	{ "fz-03.rom",	0x10000, 0x264b90ed, 3 | BRF_GRA },           //  6

	{ "mac-03.rom",	0x80000, 0x9996f3dc, 4 | BRF_GRA },           //  7 Foreground Tiles

	{ "mac-02.rom",	0x80000, 0x49504e89, 5 | BRF_GRA },           //  8 Background Tiles

	{ "mac-00.rom",	0x80000, 0x52acf1d6, 6 | BRF_GRA },           //  9 Sprite Tiles
	{ "mac-01.rom",	0x80000, 0xb28f7584, 6 | BRF_GRA },           // 10

	{ "fz-08.rom",	0x20000, 0xc9bf68e1, 7 | BRF_SND },           // 11 Oki6295 #0 Samples

	{ "fz-07.rom",	0x20000, 0x588dd3cb, 8 | BRF_SND },           // 12 Oki6295 #1 Samples
};

STD_ROM_PICK(darkseaj)
STD_ROM_FN(darkseaj)

struct BurnDriver BurnDrvDarkseaj = {
	"darksealj", "darkseal", NULL, NULL, "1990",
	"Dark Seal (Japan)\0", NULL, "Data East Corporation", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_POST90S, GBF_MAZE | GBF_SCRFIGHT, 0,
	NULL, darkseajRomInfo, darkseajRomName, NULL, NULL, DarksealInputInfo, DarksealDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x800,
	256, 240, 4, 3
};


// Gate of Doom (US revision 4)

static struct BurnRomInfo gatedoomRomDesc[] = {
	{ "gb04-4",	0x20000, 0x8e3a0bfd, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "gb01-4",	0x20000, 0x8d0fd383, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "ga-00.rom",	0x20000, 0xfbf3ac63, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "ga-05.rom",	0x20000, 0xd5e3ae3f, 1 | BRF_PRG | BRF_ESS }, //  3

	{ "fz-06.rom",	0x10000, 0xc4828a6d, 2 | BRF_PRG | BRF_ESS }, //  4 H6280 Code

	{ "fz-02.rom",	0x10000, 0x3c9c3012, 3 | BRF_GRA },           //  5 Text Tiles
	{ "fz-03.rom",	0x10000, 0x264b90ed, 3 | BRF_GRA },           //  6

	{ "mac-03.rom",	0x80000, 0x9996f3dc, 4 | BRF_GRA },           //  7 Foreground Tiles

	{ "mac-02.rom",	0x80000, 0x49504e89, 5 | BRF_GRA },           //  8 Background Tiles

	{ "mac-00.rom",	0x80000, 0x52acf1d6, 6 | BRF_GRA },           //  9 Sprite Tiles
	{ "mac-01.rom",	0x80000, 0xb28f7584, 6 | BRF_GRA },           // 10

	{ "fz-08.rom",	0x20000, 0xc9bf68e1, 7 | BRF_SND },           // 11 Oki6295 #0 Samples

	{ "fz-07.rom",	0x20000, 0x588dd3cb, 8 | BRF_SND },           // 12 Oki6295 #1 Samples
};

STD_ROM_PICK(gatedoom)
STD_ROM_FN(gatedoom)

struct BurnDriver BurnDrvGatedoom = {
	"gatedoom", "darkseal", NULL, NULL, "1990",
	"Gate of Doom (US revision 4)\0", NULL, "Data East Corporation", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_POST90S, GBF_MAZE | GBF_SCRFIGHT, 0,
	NULL, gatedoomRomInfo, gatedoomRomName, NULL, NULL, DarksealInputInfo, DarksealDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x800,
	256, 240, 4, 3
};


// Gate of Doom (US revision 1)

static struct BurnRomInfo gatedom1RomDesc[] = {
	{ "gb04.bin",	0x20000, 0x4c3bbd2b, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "gb01.bin",	0x20000, 0x59e367f4, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "gb00.bin",	0x20000, 0xa88c16a1, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "gb05.bin",	0x20000, 0x252d7e14, 1 | BRF_PRG | BRF_ESS }, //  3

	{ "fz-06.rom",	0x10000, 0xc4828a6d, 2 | BRF_PRG | BRF_ESS }, //  4 H6280 Code

	{ "fz-02.rom",	0x10000, 0x3c9c3012, 3 | BRF_GRA },           //  5 Text Tiles
	{ "fz-03.rom",	0x10000, 0x264b90ed, 3 | BRF_GRA },           //  6

	{ "mac-03.rom",	0x80000, 0x9996f3dc, 4 | BRF_GRA },           //  7 Foreground Tiles

	{ "mac-02.rom",	0x80000, 0x49504e89, 5 | BRF_GRA },           //  8 Background Tiles

	{ "mac-00.rom",	0x80000, 0x52acf1d6, 6 | BRF_GRA },           //  9 Sprite Tiles
	{ "mac-01.rom",	0x80000, 0xb28f7584, 6 | BRF_GRA },           // 10

	{ "fz-08.rom",	0x20000, 0xc9bf68e1, 7 | BRF_SND },           // 11 Oki6295 #0 Samples

	{ "fz-07.rom",	0x20000, 0x588dd3cb, 8 | BRF_SND },           // 12 Oki6295 #1 Samples
};

STD_ROM_PICK(gatedom1)
STD_ROM_FN(gatedom1)

struct BurnDriver BurnDrvGatedom1 = {
	"gatedoom1", "darkseal", NULL, NULL, "1990",
	"Gate of Doom (US revision 1)\0", NULL, "Data East Corporation", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_POST90S, GBF_MAZE | GBF_SCRFIGHT, 0,
	NULL, gatedom1RomInfo, gatedom1RomName, NULL, NULL, DarksealInputInfo, DarksealDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x800,
	256, 240, 4, 3
};
