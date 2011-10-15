// FB Alpha Arabian driver module
// Based on MAME driver by Jarek Burczynski and various others

#include "tiles_generic.h"
#include "driver.h"
extern "C" {
#include "ay8910.h"
}

static unsigned char *AllMem;
static unsigned char *RamEnd;
static unsigned char *MemEnd;
static unsigned char *AllRam;
static unsigned char *DrvZ80ROM;
static unsigned char *DrvGfxROM;
static unsigned char *DrvZ80RAM;
static unsigned char *DrvVidRAM;
static unsigned char *DrvBlitRAM;
static unsigned char *DrvTempBmp;
static unsigned int *Palette;
static unsigned int *DrvPalette;
static unsigned char DrvRecalc;

static short *pAY8910Buffer[3];

static unsigned char *arabian_color;
static unsigned char *flipscreen;

static int custom_cpu_reset;
static int custom_cpu_busy;

static unsigned char DrvJoy1[8];
static unsigned char DrvJoy3[8];
static unsigned char DrvJoy4[8];
static unsigned char DrvJoy5[8];
static unsigned char DrvJoy6[8];
static unsigned char DrvJoy7[8];
static unsigned char DrvDips[2];
static unsigned char DrvInputs[8];
static unsigned char DrvReset;

static struct BurnInputInfo ArabianInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy3 + 1,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy4 + 2,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy4 + 3,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy4 + 1,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy4 + 0,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy5 + 0,	"p1 fire 1"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy1 + 1,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy3 + 2,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy6 + 2,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy6 + 3,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy6 + 1,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy6 + 0,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy7 + 0,	"p2 fire 1"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy3 + 3,	"service"	},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
};

STDINPUTINFO(Arabian)

static struct BurnDIPInfo ArabianDIPList[]=
{
	{0x10, 0xff, 0xff, 0x06, NULL			},
	{0x11, 0xff, 0xff, 0x0d, NULL			},

	{0   , 0xfe, 0   ,    2, "Lives"		},
	{0x10, 0x01, 0x01, 0x00, "3"			},
	{0x10, 0x01, 0x01, 0x01, "5"			},

	{0   , 0xfe, 0   ,    2, "Cabinet"		},
	{0x10, 0x01, 0x02, 0x02, "Upright"		},
	{0x10, 0x01, 0x02, 0x00, "Cocktail"		},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x10, 0x01, 0x04, 0x04, "Off"			},
	{0x10, 0x01, 0x04, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Difficulty"		},
	{0x10, 0x01, 0x08, 0x00, "Easy"			},
	{0x10, 0x01, 0x08, 0x08, "Hard"			},

	{0   , 0xfe, 0   ,    16, "Coinage"		},
	{0x10, 0x01, 0xf0, 0x10, "A 2/1 B 2/1"		},
	{0x10, 0x01, 0xf0, 0x20, "A 2/1 B 1/3"		},
	{0x10, 0x01, 0xf0, 0x00, "A 1/1 B 1/1"		},
	{0x10, 0x01, 0xf0, 0x30, "A 1/1 B 1/2"		},
	{0x10, 0x01, 0xf0, 0x40, "A 1/1 B 1/3"		},
	{0x10, 0x01, 0xf0, 0x50, "A 1/1 B 1/4"		},
	{0x10, 0x01, 0xf0, 0x60, "A 1/1 B 1/5"		},
	{0x10, 0x01, 0xf0, 0x70, "A 1/1 B 1/6"		},
	{0x10, 0x01, 0xf0, 0x80, "A 1/2 B 1/2"		},
	{0x10, 0x01, 0xf0, 0x90, "A 1/2 B 1/4"		},
	{0x10, 0x01, 0xf0, 0xa0, "A 1/2 B 1/5"		},
	{0x10, 0x01, 0xf0, 0xe0, "A 1/2 B 1/6"		},
	{0x10, 0x01, 0xf0, 0xb0, "A 1/2 B 1/10"		},
	{0x10, 0x01, 0xf0, 0xc0, "A 1/2 B 1/11"		},
	{0x10, 0x01, 0xf0, 0xd0, "A 1/2 B 1/12"		},
	{0x10, 0x01, 0xf0, 0xf0, "Free Play"		},

	{0   , 0xfe, 0   ,    2, "Coin Counters"	},
	{0x11, 0x01, 0x01, 0x01, "1"			},
	{0x11, 0x01, 0x01, 0x00, "2"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x11, 0x01, 0x02, 0x02, "Off"			},
	{0x11, 0x01, 0x02, 0x00, "On"			},

	{0   , 0xfe, 0   ,    4, "Bonus Life"		},
	{0x11, 0x01, 0x0c, 0x0c, "30k 70k 40k+"		},
	{0x11, 0x01, 0x0c, 0x04, "20k only"		},
	{0x11, 0x01, 0x0c, 0x08, "40k only"		},
	{0x11, 0x01, 0x0c, 0x00, "None"			},
};

STDDIPINFO(Arabian)

static struct BurnDIPInfo ArabianaDIPList[]=
{
	{0x10, 0xff, 0xff, 0x06, NULL			},
	{0x11, 0xff, 0xff, 0x0d, NULL			},

	{0   , 0xfe, 0   ,    2, "Lives"		},
	{0x10, 0x01, 0x01, 0x00, "3"			},
	{0x10, 0x01, 0x01, 0x01, "5"			},

	{0   , 0xfe, 0   ,    2, "Cabinet"		},
	{0x10, 0x01, 0x02, 0x02, "Upright"		},
	{0x10, 0x01, 0x02, 0x00, "Cocktail"		},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x10, 0x01, 0x04, 0x04, "Off"			},
	{0x10, 0x01, 0x04, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Difficulty"		},
	{0x10, 0x01, 0x08, 0x00, "Easy"			},
	{0x10, 0x01, 0x08, 0x08, "Hard"			},

	{0   , 0xfe, 0   ,    16, "Coinage"		},
	{0x10, 0x01, 0xf0, 0x10, "A 2/1 B 2/1"		},
	{0x10, 0x01, 0xf0, 0x20, "A 2/1 B 1/3"		},
	{0x10, 0x01, 0xf0, 0x00, "A 1/1 B 1/1"		},
	{0x10, 0x01, 0xf0, 0x30, "A 1/1 B 1/2"		},
	{0x10, 0x01, 0xf0, 0x40, "A 1/1 B 1/3"		},
	{0x10, 0x01, 0xf0, 0x50, "A 1/1 B 1/4"		},
	{0x10, 0x01, 0xf0, 0x60, "A 1/1 B 1/5"		},
	{0x10, 0x01, 0xf0, 0x70, "A 1/1 B 1/6"		},
	{0x10, 0x01, 0xf0, 0x80, "A 1/2 B 1/2"		},
	{0x10, 0x01, 0xf0, 0x90, "A 1/2 B 1/4"		},
	{0x10, 0x01, 0xf0, 0xa0, "A 1/2 B 1/5"		},
	{0x10, 0x01, 0xf0, 0xe0, "A 1/2 B 1/6"		},
	{0x10, 0x01, 0xf0, 0xb0, "A 1/2 B 1/10"		},
	{0x10, 0x01, 0xf0, 0xc0, "A 1/2 B 1/11"		},
	{0x10, 0x01, 0xf0, 0xd0, "A 1/2 B 1/12"		},
	{0x10, 0x01, 0xf0, 0xf0, "Free Play"		},

	{0   , 0xfe, 0   ,    2, "Coin Counters"	},
	{0x11, 0x01, 0x01, 0x01, "1"			},
	{0x11, 0x01, 0x01, 0x00, "2"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x11, 0x01, 0x02, 0x02, "Off"			},
	{0x11, 0x01, 0x02, 0x00, "On"			},

	{0   , 0xfe, 0   ,    4, "Bonus Life"		},
	{0x11, 0x01, 0x0c, 0x0c, "20k 50k 150k 100k+"	},
	{0x11, 0x01, 0x0c, 0x04, "20k only"		},
	{0x11, 0x01, 0x0c, 0x08, "40k only"		},
	{0x11, 0x01, 0x0c, 0x00, "None"			},
};

STDDIPINFO(Arabiana)

static unsigned char custom_cpu_r(unsigned short offset)
{
	if (custom_cpu_reset || offset < 0x7f0)
		return DrvZ80RAM[offset];

	switch (offset & 0x0f)
	{
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
			return DrvInputs[(offset & 0x0f)+2];

		case 6:
			return custom_cpu_busy ^= 1;

		case 8:
			return DrvZ80RAM[offset - 1];

		default:
			return 0;
	}
	return 0;
}

static void blit_area(unsigned char plane, unsigned short src, unsigned char x, unsigned char y, unsigned char sx, unsigned char sy)
{
	unsigned char *srcdata = DrvGfxROM + src * 4;

	for (int i = 0; i <= sx; i++, x += 4)
	{
		for (int j = 0; j <= sy; j++)
		{
			unsigned char p1 = *srcdata++;
			unsigned char p2 = *srcdata++;
			unsigned char p3 = *srcdata++;
			unsigned char p4 = *srcdata++;
			unsigned char *base = DrvTempBmp + (((y + j) & 0xff) * 256 + (x & 0xff));

			if (plane & 0x01)
			{
				if (p4 != 8) base[0] = (base[0] & ~0xf0) | (p4 << 4);
				if (p3 != 8) base[1] = (base[1] & ~0xf0) | (p3 << 4);
				if (p2 != 8) base[2] = (base[2] & ~0xf0) | (p2 << 4);
				if (p1 != 8) base[3] = (base[3] & ~0xf0) | (p1 << 4);
			}

			if (plane & 0x04)
			{
				if (p4 != 8) base[0] = (base[0] & ~0x0f) | p4;
				if (p3 != 8) base[1] = (base[1] & ~0x0f) | p3;
				if (p2 != 8) base[2] = (base[2] & ~0x0f) | p2;
				if (p1 != 8) base[3] = (base[3] & ~0x0f) | p1;
			}
		}
	}
}

static void arabian_blitter_w(int offset, unsigned char data)
{
	DrvBlitRAM[offset] = data;

	if (offset == 0x0006)
	{
		int plane = DrvBlitRAM[offset - 6];
		int src   = DrvBlitRAM[offset - 5] | (DrvBlitRAM[offset - 4] << 8);
		int x     = DrvBlitRAM[offset - 2] << 2;
		int y     = DrvBlitRAM[offset - 3];
		int sx    = DrvBlitRAM[offset - 0];
		int sy    = DrvBlitRAM[offset - 1];

		blit_area(plane, src, x, y, sx, sy);
	}
}

void __fastcall arabian_videoram_w(unsigned short offset, unsigned char data)
{
	unsigned char *base;
	unsigned char x, y;

	x = (offset >> 8) << 2;
	y = offset & 0xff;

	DrvVidRAM[offset & 0x3fff] = data;

	base = &DrvTempBmp[y * 256 + x];

	if (DrvBlitRAM[0] & 0x08)
	{
		base[0] = (base[0] & ~0x03) | ((data & 0x10) >> 3) | ((data & 0x01) >> 0);
		base[1] = (base[1] & ~0x03) | ((data & 0x20) >> 4) | ((data & 0x02) >> 1);
		base[2] = (base[2] & ~0x03) | ((data & 0x40) >> 5) | ((data & 0x04) >> 2);
		base[3] = (base[3] & ~0x03) | ((data & 0x80) >> 6) | ((data & 0x08) >> 3);
	}

	if (DrvBlitRAM[0] & 0x04)
	{
		base[0] = (base[0] & ~0x0c) | ((data & 0x10) >> 1) | ((data & 0x01) << 2);
		base[1] = (base[1] & ~0x0c) | ((data & 0x20) >> 2) | ((data & 0x02) << 1);
		base[2] = (base[2] & ~0x0c) | ((data & 0x40) >> 3) | ((data & 0x04) << 0);
		base[3] = (base[3] & ~0x0c) | ((data & 0x80) >> 4) | ((data & 0x08) >> 1);
	}

	if (DrvBlitRAM[0] & 0x02)
	{
		base[0] = (base[0] & ~0x30) | ((data & 0x10) << 1) | ((data & 0x01) << 4);
		base[1] = (base[1] & ~0x30) | ((data & 0x20) << 0) | ((data & 0x02) << 3);
		base[2] = (base[2] & ~0x30) | ((data & 0x40) >> 1) | ((data & 0x04) << 2);
		base[3] = (base[3] & ~0x30) | ((data & 0x80) >> 2) | ((data & 0x08) << 1);
	}

	if (DrvBlitRAM[0] & 0x01)
	{
		base[0] = (base[0] & ~0xc0) | ((data & 0x10) << 3) | ((data & 0x01) << 6);
		base[1] = (base[1] & ~0xc0) | ((data & 0x20) << 2) | ((data & 0x02) << 5);
		base[2] = (base[2] & ~0xc0) | ((data & 0x40) << 1) | ((data & 0x04) << 4);
		base[3] = (base[3] & ~0xc0) | ((data & 0x80) << 0) | ((data & 0x08) << 3);
	}
}

void __fastcall arabian_write(unsigned short address, unsigned char data)
{
	if ((address & 0xc000) == 0x8000) {
		arabian_videoram_w(address & 0x3fff, data);
		return;
	}

	if ((address & 0xf000) == 0xe000) {
		arabian_blitter_w(address & 0x0007, data);
		return;
	}
}

unsigned char __fastcall arabian_read(unsigned short address)
{
	if ((address & 0xfe00) == 0xc000) {
		return DrvInputs[0];
	}

	if ((address & 0xfe00) == 0xc200) {
		return DrvDips[0];
	}

	if ((address & 0xff00) == 0xd700) {
		return custom_cpu_r(address & 0x7ff);
	}

	return 0;
}

void __fastcall arabian_out(unsigned short port, unsigned char data)
{
	switch (port)
	{
		case 0xc800:
			AY8910Write(0, 0, data);
		break;

		case 0xca00:
			AY8910Write(0, 1, data);
		break;
	}
}

void ay8910_porta_w(unsigned int, unsigned int data)
{
	*arabian_color = data >> 3;
}

void ay8910_portb_w(unsigned int, unsigned int data)
{
	custom_cpu_reset = ~data & 0x10;
}

static int DrvDoReset()
{
	DrvReset = 0;

	memset (AllRam, 0, RamEnd - AllRam);

	ZetOpen(0);
	ZetReset();
	ZetClose();

	AY8910Reset(0);

	custom_cpu_reset = 0;
	custom_cpu_busy = 0;

	return 0;
}

static int MemIndex()
{
	unsigned char *Next; Next = AllMem;

	DrvZ80ROM		= Next; Next += 0x008000;

	DrvGfxROM		= Next; Next += 0x010000;

	Palette			= (unsigned int*)Next; Next += 0x2000 * sizeof(int);
	DrvPalette		= (unsigned int*)Next; Next += 0x2000 * sizeof(int);

	pAY8910Buffer[0]	= (short*)Next; Next += nBurnSoundLen * sizeof(short);
	pAY8910Buffer[1]	= (short*)Next; Next += nBurnSoundLen * sizeof(short);
	pAY8910Buffer[2]	= (short*)Next; Next += nBurnSoundLen * sizeof(short);

	AllRam			= Next;

	DrvZ80RAM		= Next; Next += 0x000800;
	DrvVidRAM		= Next; Next += 0x004000;
	DrvBlitRAM		= Next; Next += 0x001000;

	DrvTempBmp		= Next; Next += 256 * 256;

	flipscreen		= Next; Next += 0x000001;
	arabian_color		= Next; Next += 0x000001;

	RamEnd			= Next;
	MemEnd			= Next;

	return 0;
}

static void DrvPaletteInit()
{
	unsigned int tpal[0x40];

	for (int i = 0; i < 64; i++)
	{
		int r, b, g;
		r = ((i >> 5) & 1) * (153*192/255) + ((i >> 4) & 1) * (102*192/255) + ((i & 0x30) ? 63 : 0);
		g = ((i >> 3) & 1) * (156*192/255) + ((i >> 2) & 1) * (99*192/255) + ((i & 0x0c) ? 63 : 0);
		b = ((i >> 1) & 1) * 192 + ((i >> 0) & 1) * 63;
		tpal[i] = (r << 16) | (g << 8) | b;
	}

	for (int i = 0; i < (1 << 13); i++)
	{
		int ena = (i >> 12) & 1;
		int enb = (i >> 11) & 1;
		int abhf = (~i >> 10) & 1;
		int aghf = (~i >> 9) & 1;
		int arhf = (~i >> 8) & 1;
		int az = (i >> 7) & 1;
		int ar = (i >> 6) & 1;
		int ag = (i >> 5) & 1;
		int ab = (i >> 4) & 1;
		int bz = (i >> 3) & 1;
		int br = (i >> 2) & 1;
		int bg = (i >> 1) & 1;
		int bb = (i >> 0) & 1;

		int planea = (az | ar | ag | ab) & ena;

		int rhi = planea ? ar : enb ? bz : 0;
		int rlo = planea ? ((!arhf & az) ? 0 : ar) : enb ? br : 0;

		int ghi = planea ? ag : enb ? bb : 0;
		int glo = planea ? ((!aghf & az) ? 0 : ag) : enb ? bg : 0;

		int bhi = ab;
		int bbase = (!abhf & az) ? 0 : ab;

		int t = (rhi << 5) | (rlo << 4) | (ghi << 3) | (glo << 2) | (bhi << 1) | bbase;

		Palette[i] = tpal[t];
	}
}

static void DrvGfxDecode()
{
	unsigned char *tmp = (unsigned char*)malloc(0x10000);

	for (int offs = 0; offs < 0x4000 * 4; offs+=4)
	{
		int v1 = DrvGfxROM[(offs / 4) + 0x0000];
		int v2 = DrvGfxROM[(offs / 4) + 0x4000];

		for (int j = 0; j < 4; j++, v1>>=1, v2>>=1) {
			tmp[offs + 3 - j] = (v1 & 1) | ((v1 >> 3) & 2) | ((v2 << 2) & 4) | ((v2 >> 1) & 8);
		}
	}

	memcpy (DrvGfxROM, tmp, 0x10000);
	free (tmp);
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
		if (BurnLoadRom(DrvZ80ROM + 0x0000, 0, 1)) return 1;
		if (BurnLoadRom(DrvZ80ROM + 0x2000, 1, 1)) return 1;
		if (BurnLoadRom(DrvZ80ROM + 0x4000, 2, 1)) return 1;
		if (BurnLoadRom(DrvZ80ROM + 0x6000, 3, 1)) return 1;
	
		if (BurnLoadRom(DrvGfxROM + 0x0000, 4, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM + 0x2000, 5, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM + 0x4000, 6, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM + 0x6000, 7, 1)) return 1;

		DrvPaletteInit();
		DrvGfxDecode();
	}

	ZetInit(1);
	ZetOpen(0);
	ZetMapArea(0x0000, 0x7fff, 0, DrvZ80ROM);
	ZetMapArea(0x0000, 0x7fff, 2, DrvZ80ROM);
	ZetMapArea(0xd000, 0xd6ff, 0, DrvZ80RAM);
	ZetMapArea(0xd000, 0xd7ff, 1, DrvZ80RAM);
	ZetMapArea(0xd000, 0xd7ff, 2, DrvZ80RAM);
	ZetSetWriteHandler(arabian_write);
	ZetSetReadHandler(arabian_read);
	ZetSetOutHandler(arabian_out);
	ZetMemEnd();
	ZetClose();

	AY8910Init(0, 1500000, nBurnSoundRate, NULL, NULL, ay8910_porta_w, ay8910_portb_w);

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static int DrvExit()
{
	GenericTilesExit();

	ZetExit();
	AY8910Exit(0);

	free (AllMem);
	AllMem = NULL;

	return 0;
}

static inline void update_flip_state()
{
	*flipscreen = DrvZ80RAM[0x034b];

	if (DrvZ80RAM[0x0400] != 0 && !(DrvZ80RAM[0x0401] & 0x02))
		*flipscreen = !*flipscreen;
}

static int DrvDraw()
{
	if (DrvRecalc) {
		for (int i = 0; i < 0x2000; i++) {
			int d = Palette[i];
			DrvPalette[i] = BurnHighCol(d >> 16, (d >> 8) & 0xff, d & 0xff, 0);
		}
	}

	update_flip_state();

	unsigned short *pDst = pTransDraw;
	unsigned char *pSrc = DrvTempBmp + (11 * 256);

	if (*flipscreen) {
		pDst += nScreenWidth * (nScreenHeight - 1);
		for (int y = 11; y < 256-11; y++) {
			for (int x = 0; x < 256; x++) {
				pDst[x] = pSrc[x ^ 0xff];
			}
			pDst -= nScreenWidth;
			pSrc += 256;
		}
	} else {
		for (int y = 11; y < 256-11; y++) {
			for (int x = 0; x < 256; x++) {
				pDst[x] = pSrc[x];
			}
			pDst += nScreenWidth;
			pSrc += 256;
		}
	}

	BurnTransferCopy(DrvPalette + (*arabian_color << 8));

	return 0;
}

static int DrvFrame()
{
	if (DrvReset) {
		DrvDoReset();
	}

	{
		memset (DrvInputs, 0, 8);
		for (int i = 0; i < 8; i++) {
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[2] ^= (DrvJoy3[i] & 1) << i;
			DrvInputs[3] ^= (DrvJoy4[i] & 1) << i;
			DrvInputs[4] ^= (DrvJoy5[i] & 1) << i;
			DrvInputs[5] ^= (DrvJoy6[i] & 1) << i;
			DrvInputs[6] ^= (DrvJoy7[i] & 1) << i;
		}
		DrvInputs[1] = DrvDips[0];
		DrvInputs[2] |= 1;
		DrvInputs[7] = DrvDips[1];
	}		

	ZetOpen(0);
	ZetRun(3000000 / 60);
	ZetRaiseIrq(0);
	ZetClose();

	if (pBurnSoundOut) {
		int nSample;
		int nSegmentLength = nBurnSoundLen;
		short* pSoundBuf = pBurnSoundOut;
		if (nSegmentLength) {
			AY8910Update(0, &pAY8910Buffer[0], nSegmentLength);
			for (int n = 0; n < nSegmentLength; n++) {
				nSample  = pAY8910Buffer[0][n];
				nSample += pAY8910Buffer[1][n];
				nSample += pAY8910Buffer[2][n];

				nSample /= 4;

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

	if (pBurnDraw) {
		DrvDraw();
	}

	return 0;
}

static int DrvScan(int nAction,int *pnMin)
{
	struct BurnArea ba;

	if (pnMin) {
		*pnMin = 0x029702;
	}

	if (nAction & ACB_VOLATILE) {		
		memset(&ba, 0, sizeof(ba));

		ba.Data	  = AllRam;
		ba.nLen	  = RamEnd - AllRam;
		ba.szName = "All Ram";
		BurnAcb(&ba);

		ZetScan(nAction);
		AY8910Scan(nAction, pnMin);

		SCAN_VAR(custom_cpu_reset);
		SCAN_VAR(custom_cpu_busy);
	}

	return 0;
}


// Arabian

static struct BurnRomInfo arabianRomDesc[] = {
	{ "ic1rev2.87", 0x2000, 0x5e1c98b8, BRF_ESS | BRF_PRG }, //  0 Z80 Code
	{ "ic2rev2.88", 0x2000, 0x092f587e, BRF_ESS | BRF_PRG }, //  1
	{ "ic3rev2.89", 0x2000, 0x15145f23, BRF_ESS | BRF_PRG }, //  2
	{ "ic4rev2.90", 0x2000, 0x32b77b44, BRF_ESS | BRF_PRG }, //  3

	{ "tvg-91.ic84",0x2000, 0xc4637822, BRF_GRA },		 //  4 Graphics
	{ "tvg-92.ic85",0x2000, 0xf7c6866d, BRF_GRA },		 //  5
	{ "tvg-93.ic86",0x2000, 0x71acd48d, BRF_GRA },		 //  6
	{ "tvg-94.ic87",0x2000, 0x82160b9a, BRF_GRA },		 //  7
	
	{ "sun-8212.ic3",0x0800, 0x8869611e, BRF_OPT },		 //  8 MCU
};

STD_ROM_PICK(arabian)
STD_ROM_FN(arabian)

struct BurnDriver BurnDrvarabian = {
	"arabian", NULL, NULL, NULL, "1983",
	"Arabian\0", NULL, "Sun Electronics", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_MISC_PRE90S, GBF_PLATFORM, 0,
	NULL, arabianRomInfo, arabianRomName, NULL, NULL, ArabianInputInfo, ArabianDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x2000,
	234, 256, 3, 4
};

// Arabian (Atari)

static struct BurnRomInfo arabianaRomDesc[] = {
	{ "tvg-87.ic1", 0x2000, 0x51e9a6b1, BRF_ESS | BRF_PRG }, //  0 Z80 Code
	{ "tvg-88.ic2", 0x2000, 0x1cdcc1ab, BRF_ESS | BRF_PRG }, //  1
	{ "tvg-89.ic3", 0x2000, 0xb7b7faa0, BRF_ESS | BRF_PRG }, //  2
	{ "tvg-90.ic4", 0x2000, 0xdbded961, BRF_ESS | BRF_PRG }, //  3

	{ "tvg-91.ic84",0x2000, 0xc4637822, BRF_GRA },		 //  4 Graphics
	{ "tvg-92.ic85",0x2000, 0xf7c6866d, BRF_GRA },		 //  5
	{ "tvg-93.ic86",0x2000, 0x71acd48d, BRF_GRA },		 //  6
	{ "tvg-94.ic87",0x2000, 0x82160b9a, BRF_GRA },		 //  7
	
	{ "sun-8212.ic3",0x0800, 0x8869611e, BRF_OPT },		 //  8 MCU
};

STD_ROM_PICK(arabiana)
STD_ROM_FN(arabiana)

struct BurnDriver BurnDrvarabiana = {
	"arabiana", "arabian", NULL, NULL, "1983",
	"Arabian (Atari)\0", NULL, "[Sun Electronics] (Atari license)", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_MISC_PRE90S, GBF_PLATFORM, 0,
	NULL, arabianaRomInfo, arabianaRomName, NULL, NULL, ArabianInputInfo, ArabianaDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x2000,
	234, 256, 3, 4
};
