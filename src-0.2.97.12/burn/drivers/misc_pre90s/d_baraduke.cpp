// FB Alpha Baraduke driver module
// Based on MAME driver by Manuel Abadia and various others

#include "tiles_generic.h"
#include "m6800_intf.h"
#include "m6809_intf.h"
#include "namco_snd.h"
#include "burn_led.h"

static unsigned char *AllMem;
static unsigned char *MemEnd;
static unsigned char *AllRam;
static unsigned char *RamEnd;
static unsigned char *DrvM6809ROM;
static unsigned char *DrvHD63701ROM;
static unsigned char *DrvGfxROM0;
static unsigned char *DrvGfxROM1;
static unsigned char *DrvGfxROM2;
static unsigned char *DrvColPROM;
static unsigned char *DrvHD63701RAM1;
static unsigned char *DrvHD63701RAM;
static unsigned char *DrvWavRAM;
static unsigned char *DrvVidRAM;
static unsigned char *DrvTxtRAM;
static unsigned char *DrvSprRAM;

static unsigned int  *Palette;
static unsigned int  *DrvPalette;
static unsigned char  DrvRecalc;

static unsigned char *flipscreen;
static unsigned char *buffer_sprites;
static unsigned char *coin_lockout;
static unsigned char *scroll;
static unsigned char *ip_select;

static int watchdog;

static unsigned char DrvJoy1[8];
static unsigned char DrvJoy2[8];
static unsigned char DrvJoy3[8];
static unsigned char DrvDips[3];
static unsigned char DrvReset;
static unsigned char DrvInputs[8];

static int nCyclesDone[2];

static struct BurnInputInfo BaradukeInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy2 + 3,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy2 + 2,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy2 + 0,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy2 + 1,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p1 fire 1"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy1 + 2,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy1 + 4,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy3 + 3,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy3 + 2,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy3 + 0,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy3 + 1,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy3 + 4,	"p2 fire 1"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy1 + 0,	"service"	},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
	{"Dip C",		BIT_DIPSWITCH,	DrvDips + 2,	"dip"		},
};

STDINPUTINFO(Baraduke)

static struct BurnDIPInfo BaradukeDIPList[]=
{
	{0x10, 0xff, 0xff, 0xff, NULL			},
	{0x11, 0xff, 0xff, 0xff, NULL			},
	{0x12, 0xff, 0xff, 0xff, NULL			},

	{0   , 0xfe, 0   ,    0, "Coin B"		},
	{0x10, 0x01, 0x03, 0x00, "3 Coins 1 Credits"	},
	{0x10, 0x01, 0x03, 0x01, "2 Coins 1 Credits"	},
	{0x10, 0x01, 0x03, 0x03, "1 Coin  1 Credits"	},
	{0x10, 0x01, 0x03, 0x02, "1 Coin  2 Credits"	},

	{0   , 0xfe, 0   ,    4, "Demo Sounds"		},
	{0x10, 0x01, 0x04, 0x00, "Off"			},
	{0x10, 0x01, 0x04, 0x04, "On"			},

	{0   , 0xfe, 0   ,    2, "Coin A"		},
	{0x10, 0x01, 0x18, 0x00, "3 Coins 1 Credits"	},
	{0x10, 0x01, 0x18, 0x08, "2 Coins 1 Credits"	},
	{0x10, 0x01, 0x18, 0x18, "1 Coin  1 Credits"	},
	{0x10, 0x01, 0x18, 0x10, "1 Coin  2 Credits"	},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x10, 0x01, 0x60, 0x40, "2"			},
	{0x10, 0x01, 0x60, 0x60, "3"			},
	{0x10, 0x01, 0x60, 0x20, "4"			},
	{0x10, 0x01, 0x60, 0x00, "5"			},

	{0   , 0xfe, 0   ,    4, "Service Mode"		},
	{0x10, 0x01, 0x80, 0x80, "Off"			},
	{0x10, 0x01, 0x80, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Allow Continue"	},
	{0x11, 0x01, 0x02, 0x02, "Off"			},
	{0x11, 0x01, 0x02, 0x00, "On"			},

	{0   , 0xfe, 0   ,    0, "Freeze"		},
	{0x11, 0x01, 0x04, 0x04, "Off"			},
	{0x11, 0x01, 0x04, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Round Select"		},
	{0x11, 0x01, 0x08, 0x08, "Off"			},
	{0x11, 0x01, 0x08, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Difficulty"		},
	{0x11, 0x01, 0x30, 0x20, "Easy"			},
	{0x11, 0x01, 0x30, 0x30, "Normal"		},
	{0x11, 0x01, 0x30, 0x10, "Hard"			},
	{0x11, 0x01, 0x30, 0x00, "Very Hard"		},

	{0   , 0xfe, 0   ,    2, "Bonus Life"		},
	{0x11, 0x01, 0xc0, 0x80, "Every 10k"		},
	{0x11, 0x01, 0xc0, 0xc0, "10k And Every 20k"	},
	{0x11, 0x01, 0xc0, 0x40, "Every 20k"		},
	{0x11, 0x01, 0xc0, 0x00, "None"			},

//	{0   , 0xfe, 0   ,    2, "Cabinet"		},
//	{0x12, 0x01, 0x02, 0x02, "Upright"		},
//	{0x12, 0x01, 0x02, 0x00, "Cocktail"		},
};

STDDIPINFO(Baraduke)

static struct BurnDIPInfo MetrocrsDIPList[]=
{
	{0x10, 0xff, 0xff, 0xff, NULL			},
	{0x11, 0xff, 0xff, 0xff, NULL			},
	{0x12, 0xff, 0xff, 0xff, NULL			},

	{0   , 0xfe, 0   ,    4, "Coin B"		},
	{0x10, 0x01, 0x03, 0x00, "3 Coins 1 Credits"	},
	{0x10, 0x01, 0x03, 0x01, "2 Coins 1 Credits"	},
	{0x10, 0x01, 0x03, 0x03, "1 Coin  1 Credits"	},
	{0x10, 0x01, 0x03, 0x02, "1 Coin  2 Credits"	},

	{0   , 0xfe, 0   ,    2, "Allow Continue"	},
	{0x10, 0x01, 0x04, 0x00, "No"			},
	{0x10, 0x01, 0x04, 0x04, "Yes"			},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x10, 0x01, 0x18, 0x10, "Easy"			},
	{0x10, 0x01, 0x18, 0x18, "Normal"		},
	{0x10, 0x01, 0x18, 0x08, "Hard"			},
	{0x10, 0x01, 0x18, 0x00, "Very Hard"		},

	{0   , 0xfe, 0   ,    4, "Coin A"		},
	{0x10, 0x01, 0x60, 0x00, "3 Coins 1 Credits"	},
	{0x10, 0x01, 0x60, 0x20, "2 Coins 1 Credits"	},
	{0x10, 0x01, 0x60, 0x60, "1 Coin  1 Credits"	},
	{0x10, 0x01, 0x60, 0x40, "1 Coin  2 Credits"	},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x10, 0x01, 0x80, 0x80, "Off"			},
	{0x10, 0x01, 0x80, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Freeze"		},
	{0x11, 0x01, 0x20, 0x20, "Off"			},
	{0x11, 0x01, 0x20, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Round Select"		},
	{0x11, 0x01, 0x40, 0x40, "Off"			},
	{0x11, 0x01, 0x40, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x11, 0x01, 0x80, 0x00, "Off"			},
	{0x11, 0x01, 0x80, 0x80, "On"			},

//	{0   , 0xfe, 0   ,    2, "Cabinet"		},
//	{0x12, 0x01, 0x02, 0x02, "Upright"		},
//	{0x12, 0x01, 0x02, 0x00, "Cocktail"		},
};

STDDIPINFO(Metrocrs)

void baraduke_main_write(unsigned short address, unsigned char data)
{
	if (address < 0x2000) {
		DrvSprRAM[address & 0x1fff] = data;
		if (address == 0x1ff2) buffer_sprites[0] = 1;
		return;
	}

	if ((address & 0xfc00) == 0x4000) {
		namcos1_custom30_write(address & 0x3ff, data);
		return;
	}

	switch (address)
	{
		case 0x8000:
			watchdog = 0;
		return;

		case 0x8800:
			M6809SetIRQ(0, M6809_IRQSTATUS_NONE);
		return;

		case 0xb000:
		case 0xb001:
		case 0xb002:
		case 0xb004:
		case 0xb005:
		case 0xb006:
			scroll[address & 0x07] = data;
		return;
	}
}

unsigned char baraduke_main_read(unsigned short address)
{
	if ((address & 0xfc00) == 0x4000) {
		return namcos1_custom30_read(address & 0x3ff);
	}

	return 0;
}

void baraduke_mcu_write(unsigned short address, unsigned char data)
{
	if ((address & 0xffe0) == 0x0000) {
		m6803_internal_registers_w(address & 0x1f, data);
		return;
	}

	if ((address & 0xff80) == 0x0080) {
		DrvHD63701RAM1[address & 0x7f] = data;
		return;
	}

	if ((address & 0xfc00) == 0x1000) {
		namcos1_custom30_write(address & 0x3ff, data);
		return;
	}
}

unsigned char baraduke_mcu_read(unsigned short address)
{
	if ((address & 0xffe0) == 0x0000) {
		return m6803_internal_registers_r(address & 0x1f);
	}

	if ((address & 0xff80) == 0x0080) {
		return DrvHD63701RAM1[address & 0x7f];
	}

	if (address == 0x1105) {
		static int kludge = 0;
		kludge++;
		return kludge>>4;
	}

	if ((address & 0xfc00) == 0x1000) {
		return namcos1_custom30_read(address & 0x3ff);
	}

	return 0;
}

void baraduke_mcu_write_port(unsigned short port, unsigned char data)
{
	switch (port & 0x1ff)
	{
		case HD63701_PORT1:
		{
			switch (data & 0xe0) {
				case 0x60:
					*ip_select = data & 0x07;
				return;

				case 0xc0:
					*coin_lockout = ~data & 0x01;
					// coin counters 0 -> data & 0x02, 1 -> data & 0x04
				return;
			}
		}
		return;

		case HD63701_PORT2:
			BurnLEDSetStatus(0, data & 0x08);
			BurnLEDSetStatus(1, data & 0x10);
		return;
	}
}

unsigned char baraduke_mcu_read_port(unsigned short port)
{
	switch (port & 0x1ff)
	{
		case HD63701_PORT1:
			return DrvInputs[*ip_select];

		case HD63701_PORT2:
			return 0xff;
	}

	return 0;
}

static int DrvDoReset(int ClearRAM)
{
	if (ClearRAM) {
		memset (AllRam, 0, RamEnd - AllRam);
	}

	M6809Open(0);
	M6809Reset();
	M6809Close();

//	HD63701Open(0);
	HD63701Reset();
//	HD63701Close();

	BurnLEDReset();

	BurnLEDSetFlipscreen(1);

	watchdog = 0;

	return 0;
}

static int MemIndex()
{
	unsigned char *Next; Next = AllMem;

	DrvM6809ROM		= Next; Next += 0x010000;
	DrvHD63701ROM		= Next; Next += 0x010000;

	DrvGfxROM0		= Next; Next += 0x008000;
	DrvGfxROM1		= Next; Next += 0x020000;
	DrvGfxROM2		= Next; Next += 0x020000;

	DrvColPROM		= Next; Next += 0x001000;

	Palette			= (unsigned int*)Next; Next += 0x0800 * sizeof(int);
	DrvPalette		= (unsigned int*)Next; Next += 0x0800 * sizeof(int);

	AllRam			= Next;

	DrvHD63701RAM1		= Next; Next += 0x000080;
	DrvHD63701RAM		= Next; Next += 0x000800;

	NamcoSoundProm		= Next;
	DrvWavRAM		= Next; Next += 0x000400;

	DrvVidRAM		= Next; Next += 0x002000;
	DrvTxtRAM		= Next; Next += 0x000800;
	DrvSprRAM		= Next; Next += 0x002000;

	coin_lockout		= Next; Next += 0x000001;
	ip_select		= Next; Next += 0x000001;
	buffer_sprites		= Next; Next += 0x000001;
	flipscreen		= Next; Next += 0x000001;

	scroll			= Next; Next += 0x000008;

	RamEnd			= Next;

	MemEnd			= Next;

	return 0;
}

static void DrvPaletteInit()
{
	for (int i = 0; i < 0x800; i++)
	{
		int bit0 = (DrvColPROM[0x800 + i] >> 0) & 0x01;
		int bit1 = (DrvColPROM[0x800 + i] >> 1) & 0x01;
		int bit2 = (DrvColPROM[0x800 + i] >> 2) & 0x01;
		int bit3 = (DrvColPROM[0x800 + i] >> 3) & 0x01;
		int r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		    bit0 = (DrvColPROM[0x000 + i] >> 0) & 0x01;
		    bit1 = (DrvColPROM[0x000 + i] >> 1) & 0x01;
		    bit2 = (DrvColPROM[0x000 + i] >> 2) & 0x01;
		    bit3 = (DrvColPROM[0x000 + i] >> 3) & 0x01;
		int g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		    bit0 = (DrvColPROM[0x000 + i] >> 4) & 0x01;
		    bit1 = (DrvColPROM[0x000 + i] >> 5) & 0x01;
		    bit2 = (DrvColPROM[0x000 + i] >> 6) & 0x01;
		    bit3 = (DrvColPROM[0x000 + i] >> 7) & 0x01;
		int b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		Palette[i] = (r << 16) | (g << 8) | b;
	}
	DrvRecalc = 1;
}

static void DrvGfxExpand()
{
	for (int i = 0; i < 0x2000; i++) {
		DrvGfxROM1[0xa000 + i] = DrvGfxROM1[0x8000 + i] << 4;
		DrvGfxROM1[0xc000 + i] = DrvGfxROM1[0x8000 + i];
		DrvGfxROM1[0xe000 + i] = DrvGfxROM1[0x8000 + 1] << 4;
	}

	for (int i = 0x10000 - 1; i >= 0; i--) {
		DrvGfxROM2[i * 2 + 1] = DrvGfxROM2[i] & 0x0f;
		DrvGfxROM2[i * 2 + 0] = DrvGfxROM2[i]  >> 4;
	}
}

static int DrvGfxDecode()
{
	int Plane0[3]  = { 0x8000 * 8, 0, 4 };
	int XOffs0[8]  = { 8*8, 8*8+1, 8*8+2, 8*8+3, 0, 1, 2, 3 };
	int XOffs1[8]  = { 0, 1, 2, 3, 8+0, 8+1, 8+2, 8+3 };
	int YOffs0[8]  = { 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 };
	int YOffs1[8]  = { 0*8, 2*8, 4*8, 6*8, 8*8, 10*8, 12*8, 14*8 };

	unsigned char *tmp = (unsigned char*)malloc(0x10000);
	if (tmp == NULL) {
		return 1;
	}

	memcpy (tmp, DrvGfxROM0, 0x2000);

	GfxDecode(0x0200, 2,  8,  8, Plane0 + 1, XOffs0, YOffs0, 0x080, tmp, DrvGfxROM0);

	memcpy (tmp, DrvGfxROM1, 0x10000);

	GfxDecode(0x0400, 3,  8,  8, Plane0 + 0, XOffs1, YOffs1, 0x080, tmp + 0x0000, DrvGfxROM1 + 0x00000);
	GfxDecode(0x0400, 3,  8,  8, Plane0 + 0, XOffs1, YOffs1, 0x080, tmp + 0x4000, DrvGfxROM1 + 0x10000);

	free (tmp);

	return 0;
}

static int DrvInit(int type)
{
	AllMem = NULL;
	MemIndex();
	int nLen = MemEnd - (unsigned char *)0;
	if ((AllMem = (unsigned char *)malloc(nLen)) == NULL) return 1;
	memset(AllMem, 0, nLen);
	MemIndex();

	{
		if (BurnLoadRom(DrvM6809ROM   + 0x06000,  0, 1)) return 1;
		if (BurnLoadRom(DrvM6809ROM   + 0x08000,  1, 1)) return 1;
		if (BurnLoadRom(DrvM6809ROM   + 0x0c000,  2, 1)) return 1;

		if (BurnLoadRom(DrvHD63701ROM + 0x08000,  3, 1)) return 1;
		if (BurnLoadRom(DrvHD63701ROM + 0x0f000,  4, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM0    + 0x00000,  5, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM1    + 0x00000,  6, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1    + 0x04000,  7, 1)) return 1;

		if (type == 0) {
			if (BurnLoadRom(DrvGfxROM1    + 0x08000,  8, 1)) return 1;

			if (BurnLoadRom(DrvGfxROM2    + 0x00000,  9, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM2    + 0x04000, 10, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM2    + 0x08000, 11, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM2    + 0x0c000, 12, 1)) return 1;

			if (BurnLoadRom(DrvColPROM    + 0x00000, 13, 1)) return 1;
			if (BurnLoadRom(DrvColPROM    + 0x00800, 14, 1)) return 1;
		} else {
			memset (DrvGfxROM1            + 0x08000, 0xff, 0x4000);

			if (BurnLoadRom(DrvGfxROM2    + 0x00000,  8, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM2    + 0x04000,  9, 1)) return 1;

			if (BurnLoadRom(DrvColPROM    + 0x00000, 10, 1)) return 1;
			if (BurnLoadRom(DrvColPROM    + 0x00800, 11, 1)) return 1;
		}

		DrvGfxExpand();
		DrvGfxDecode();
		DrvPaletteInit();
	}

	M6809Init(1);
	M6809Open(0);
	M6809MapMemory(DrvSprRAM,			0x0000, 0x1fff, M6809_READ | M6809_FETCH);
	M6809MapMemory(DrvSprRAM,			0x0000, 0x1eff, M6809_WRITE);
	M6809MapMemory(DrvVidRAM,			0x2000, 0x3fff, M6809_RAM);
	M6809MapMemory(DrvTxtRAM,			0x4800, 0x4fff, M6809_RAM);
	M6809MapMemory(DrvM6809ROM + 0x06000,		0x6000, 0xffff, M6809_ROM);
	M6809SetWriteByteHandler(baraduke_main_write);
	M6809SetReadByteHandler(baraduke_main_read);
	M6809Close();

	HD63701Init(1);
//	HD63701Open(0);
	HD63701MapMemory(DrvHD63701ROM + 0x8000,	0x8000, 0xbfff, HD63701_ROM);
	HD63701MapMemory(DrvHD63701RAM,			0xc000, 0xc7ff, HD63701_RAM);
	HD63701MapMemory(DrvHD63701ROM + 0xf000,	0xf000, 0xffff, HD63701_ROM);
	HD63701SetReadByteHandler(baraduke_mcu_read);
	HD63701SetWriteByteHandler(baraduke_mcu_write);
	HD63701SetReadPortHandler(baraduke_mcu_read_port);
	HD63701SetWritePortHandler(baraduke_mcu_write_port);
//	HD63701Close();

	NamcoSoundInit(49152000/2048);

	BurnLEDInit(2, LED_POSITION_BOTTOM_RIGHT, LED_SIZE_5x5, LED_COLOR_GREEN, 100);

	GenericTilesInit();

	DrvDoReset(1);

	return 0;
}

static int DrvExit()
{
	GenericTilesExit();

	BurnLEDExit();

	NamcoSoundExit();

	M6809Exit();
	HD63701Exit();

	free (AllMem);
	AllMem = NULL;

	NamcoSoundProm = NULL;

	return 0;
}

static void draw_fg_layer()
{
	for (int y = 0; y < 28; y++)
	{
		for (int x = 0; x < 36; x++)
		{
			int col = x - 2;
			int row = y + 2;

			int offs = 0;
			if (col & 0x20) {
				offs = row + ((col & 0x1f) << 5);
			} else {
				offs = col + (row << 5);
			}

			int code  = DrvTxtRAM[offs + 0x000];
			int color =(DrvTxtRAM[offs + 0x400] & 0x7f) << 2;

			if (*flipscreen) {
				Render8x8Tile_Mask_Clip(pTransDraw, code, 280 - (x * 8), 216 - (y * 8), color, 2, 3, 0, DrvGfxROM0);
			} else {
				Render8x8Tile_Mask_Clip(pTransDraw, code, x * 8, y * 8, color, 2, 3, 0, DrvGfxROM0);
			}
		}
	}
}

static void draw_bg_layer(int bank, int prio)
{
	unsigned char *src = DrvVidRAM + bank * 0x1000;
	int scrollx = (scroll[bank * 4 + 1] + ((scroll[bank * 4 + 0] & 0x01) << 8) + (bank ? 24 : 26)) & 0x1ff;
	int scrolly = (scroll[bank * 4 + 2] + 9) & 0xff;

	for (int offs = 0; offs < 64 * 32; offs++)
	{
		int sx = (offs & 0x3f) << 3;
		int sy = (offs >> 6) << 3;

		sx -= scrollx;
		if (sx < -7) sx += 512;
		sy -= scrolly;
		if (sy < -7) sy += 256;

		if (sx >= nScreenWidth || sy >= nScreenHeight) continue;

		int color = src[offs * 2 + 1];
		int code  = src[offs * 2 + 0] + ((color & 0x03) << 8) + bank * 0x400;

		if (prio) {
			if (*flipscreen) {
				Render8x8Tile_Mask_FlipXY_Clip(pTransDraw, code, 280 - sx, 216 - sy, color, 3, 7, 0, DrvGfxROM1);
			} else {
				Render8x8Tile_Mask_Clip(pTransDraw, code, sx, sy, color, 3, 7, 0, DrvGfxROM1);
			}
		} else {
			if (*flipscreen) {
				Render8x8Tile_FlipXY_Clip(pTransDraw, code, 280 - sx, 216 - sy, color, 3, 0, DrvGfxROM1);
			} else {
				Render8x8Tile_Clip(pTransDraw, code, sx, sy, color, 3, 0, DrvGfxROM1);
			}
		}
	}
}

static void draw_sprites(int prio)
{
	unsigned char *src = DrvSprRAM + 0x1800;
	int sprite_xoffs = src[0x07f5] - ((src[0x07f4] & 0x01) << 8);
	int sprite_yoffs = src[0x07f7];

	for (int offs = 0; offs < 0x800 - 16; offs += 16, src += 16)
	{
		int priority = src[10] & 0x01;

		if (priority == prio)
		{
			int attr1  = src[10];
			int sprite = src[11] << 2;
			int color  = src[12];
			int sx     = src[13] + ((color & 0x01) << 8) + sprite_xoffs;
			int attr2  = src[14];
			int sy     = (240 - src[15]) - sprite_yoffs;
			int flipx  = (attr1 & 0x20) >> 5;
			int flipy  = (attr2 & 0x01) >> 0;
			int sizex  = (attr1 & 0x80) >> 7;
			int sizey  = (attr2 & 0x04) >> 2;

			if ((attr1 & 0x10) && !sizex) sprite += 1;
			if ((attr2 & 0x10) && !sizey) sprite += 2;

			sy -= 16 * sizey;

			if (*flipscreen)
			{
				sx = 499 - 16 * sizex - sx;
				sy = 240 - 16 * sizey - sy;
				flipx ^= 1;
				flipy ^= 1;
			}

			for (int y = 0;y <= sizey;y++)
			{
				for (int x = 0;x <= sizex;x++)
				{
					int code = sprite + (y ^ (sizey * flipy)) * 2 + (x ^ (sizex * flipx));

					if (flipy) {
						if(flipx) {
							Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, code, -71 + ((sx + 16 * x) & 0x1ff), 1 + ((sy + 16 * y) & 0xff), color >> 1, 4, 0x0f, 0, DrvGfxROM2);
						} else {
							Render16x16Tile_Mask_FlipY_Clip(pTransDraw, code, -71 + ((sx + 16 * x) & 0x1ff), 1 + ((sy + 16 * y) & 0xff), color >> 1, 4, 0x0f, 0, DrvGfxROM2);
						}
					} else {
						if(flipx) {
							Render16x16Tile_Mask_FlipX_Clip(pTransDraw, code, -71 + ((sx + 16 * x) & 0x1ff), 1 + ((sy + 16 * y) & 0xff), color >> 1, 4, 0x0f, 0, DrvGfxROM2);
						} else {
							Render16x16Tile_Mask_Clip(pTransDraw, code, -71 + ((sx + 16 * x) & 0x1ff), 1 + ((sy + 16 * y) & 0xff), color >> 1, 4, 0x0f, 0, DrvGfxROM2);
						}
					}
				}
			}
		}
	}
}

static int DrvDraw()
{
	if (DrvRecalc) {
		for (int i = 0; i < 0x800; i++) {
			int p = Palette[i];
			DrvPalette[i] = BurnHighCol(p >> 16, p >> 8, p, 0);
		}
		DrvRecalc = 0;
	}

	*flipscreen = DrvSprRAM[0x1ff6] & 0x01;

	BurnLEDSetFlipscreen(*flipscreen);

	int priority = ((scroll[0] & 0x0e) == 0x0c) ? 1 : 0;

	draw_bg_layer(priority ^ 0, 0);
	draw_sprites(0);
	draw_bg_layer(priority ^ 1, 1);
	draw_sprites(1);

	draw_fg_layer();

	BurnTransferCopy(DrvPalette);

	BurnLEDRender();

	return 0;
}

static void DrvSpriteBuffer()
{
	if (buffer_sprites[0] == 0) return;

	for (int i = 0; i < 0x800; i += 16) {
		memcpy (DrvSprRAM + 0x180a + i, DrvSprRAM + 0x1804 + i, 6);
	}

	buffer_sprites[0] = 0;
}

static int DrvFrame()
{
	if (DrvReset) {
		DrvDoReset(1);
	}

	watchdog++;
	if (watchdog > 180) {
		DrvDoReset(0);
	}

	{
		memset (DrvInputs, 0xff, 8);
		for (int i = 0; i < 8; i++) {
			DrvInputs[4] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[5] ^= (DrvJoy3[i] & 1) << i;
 			DrvInputs[6] ^= (DrvJoy2[i] & 1) << i;
		}

		DrvInputs[0] = ((DrvDips[0] & 0xf8) >> 3);
		DrvInputs[1] = ((DrvDips[0] & 0x07) << 2) | ((DrvDips[1] & 0xc0) >> 6);
		DrvInputs[2] = ((DrvDips[1] & 0x3e) >> 1);
		DrvInputs[3] =  (DrvDips[2] & 0x0f) | ((DrvDips[0] & 0x01) << 4);

		if (*coin_lockout) DrvInputs[4] |= 0x06;
	}

	M6809NewFrame();
	HD63701NewFrame();

	int nInterleave = 100; // 1000?
	int nCyclesTotal[2] = { 1536000 / 60, 6144000 / 60 };
	//int nCyclesDone[2] = { 0, 0 };
	nCyclesDone[0] = nCyclesDone[1] = 0;

	for (int i = 0; i < nInterleave; i++)
	{
		int nNext;

		M6809Open(0);
		nNext = (i + 1) * nCyclesTotal[0] / nInterleave;
		nCyclesDone[0] += M6809Run(nNext - nCyclesDone[0]);
		if (i == (nInterleave - 1)) {
			M6809SetIRQ(0, M6809_IRQSTATUS_ACK);
		}
		M6809Close();

	//	HD63701Open(0);
		nNext = (i + 1) * nCyclesTotal[1] / nInterleave;
		nCyclesDone[1] += HD63701Run(nNext - nCyclesDone[1]);
		if (i == (nInterleave - 1)) {
			HD63701SetIRQ(0, M6800_IRQSTATUS_AUTO);
		}
	//	HD63701Close();
	}

	if (pBurnSoundOut) {
		NamcoSoundUpdate(pBurnSoundOut, nBurnSoundLen);
	}

	if (pBurnDraw) {
		DrvDraw();
	}

	DrvSpriteBuffer();

	return 0;
}

static int DrvScan(int nAction, int *pnMin)
{
	return 1; // Broken :(

	struct BurnArea ba;

	if (pnMin) {
		*pnMin = 0x029707;
	}

	if (nAction & ACB_VOLATILE) {		
		memset(&ba, 0, sizeof(ba));

		ba.Data	  = AllRam;
		ba.nLen	  = RamEnd - AllRam;
		ba.szName = "All Ram";
		BurnAcb(&ba);

		M6809Scan(nAction);
		HD63701Scan(nAction);

		NamcoSoundScan(nAction, pnMin);

		BurnLEDScan(nAction, pnMin);
	}

	return 0;
}


// Alien Sector

static struct BurnRomInfo aliensecRomDesc[] = {
	{ "bd1_3.9c",		0x2000, 0xea2ea790, 1 | BRF_PRG | BRF_ESS }, //  0 M6809 Code
	{ "bd2_1.9a",		0x4000, 0x9a0a9a87, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "bd2_2.9b",		0x4000, 0x383e5458, 1 | BRF_PRG | BRF_ESS }, //  2

	{ "bd1_4.3b",		0x4000, 0xabda0fe7, 2 | BRF_PRG | BRF_ESS }, //  3 HD63701 Code
	{ "cus60-60a1.mcu",	0x1000, 0x076ea82a, 2 | BRF_PRG | BRF_ESS }, //  4

	{ "bd1_5.3j",		0x2000, 0x706b7fee, 3 | BRF_GRA },           //  5 Characters

	{ "bd2_8.4p",		0x4000, 0x432bd7d9, 4 | BRF_GRA },           //  6 Background Tiles
	{ "bd1_7.4n",		0x4000, 0x0d7ebec9, 4 | BRF_GRA },           //  7
	{ "bd2_6.4m",		0x4000, 0xf4c1df60, 4 | BRF_GRA },           //  8

	{ "bd1_9.8k",		0x4000, 0x87a29acc, 5 | BRF_GRA },           //  9 Sprites
	{ "bd1_10.8l",		0x4000, 0x72b6d20c, 5 | BRF_GRA },           // 10
	{ "bd1_11.8m",		0x4000, 0x3076af9c, 5 | BRF_GRA },           // 11
	{ "bd1_12.8n",		0x4000, 0x8b4c09a3, 5 | BRF_GRA },           // 12

	{ "bd1-1.1n",		0x0800, 0x0d78ebc6, 6 | BRF_GRA },           // 13 Color PROMs
	{ "bd1-2.2m",		0x0800, 0x03f7241f, 6 | BRF_GRA },           // 14
};

STD_ROM_PICK(aliensec)
STD_ROM_FN(aliensec)

static int AlienInit()
{
	return DrvInit(0);
}

struct BurnDriver BurnDrvAliensec = {
	"aliensec", NULL, NULL, NULL, "1985",
	"Alien Sector\0", NULL, "Namco", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_PRE90S, GBF_HORSHOOT, 0,
	NULL, aliensecRomInfo, aliensecRomName, NULL, NULL, BaradukeInputInfo, BaradukeDIPInfo,
	AlienInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x800,
	288, 224, 4, 3
};


// Baraduke

static struct BurnRomInfo baradukeRomDesc[] = {
	{ "bd1_3.9c",		0x2000, 0xea2ea790, 1 | BRF_PRG | BRF_ESS }, //  0 M6809 Code
	{ "bd1_1.9a",		0x4000, 0x4e9f2bdc, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "bd1_2.9b",		0x4000, 0x40617fcd, 1 | BRF_PRG | BRF_ESS }, //  2

	{ "bd1_4b.3b",		0x4000, 0xa47ecd32, 2 | BRF_PRG | BRF_ESS }, //  3 HD63701 Code
	{ "cus60-60a1.mcu",	0x1000, 0x076ea82a, 2 | BRF_PRG | BRF_ESS }, //  4

	{ "bd1_5.3j",		0x2000, 0x706b7fee, 3 | BRF_GRA },           //  5 Characters

	{ "bd1_8.4p",		0x4000, 0xb0bb0710, 4 | BRF_GRA },           //  6 Background Tiles
	{ "bd1_7.4n",		0x4000, 0x0d7ebec9, 4 | BRF_GRA },           //  7
	{ "bd1_6.4m",		0x4000, 0xe5da0896, 4 | BRF_GRA },           //  8

	{ "bd1_9.8k",		0x4000, 0x87a29acc, 5 | BRF_GRA },           //  9 Sprites
	{ "bd1_10.8l",		0x4000, 0x72b6d20c, 5 | BRF_GRA },           // 10
	{ "bd1_11.8m",		0x4000, 0x3076af9c, 5 | BRF_GRA },           // 11
	{ "bd1_12.8n",		0x4000, 0x8b4c09a3, 5 | BRF_GRA },           // 12

	{ "bd1-1.1n",		0x0800, 0x0d78ebc6, 6 | BRF_GRA },           // 13 Color PROMs
	{ "bd1-2.2m",		0x0800, 0x03f7241f, 6 | BRF_GRA },           // 14
};

STD_ROM_PICK(baraduke)
STD_ROM_FN(baraduke)

struct BurnDriver BurnDrvBaraduke = {
	"baraduke", "aliensec", NULL, NULL, "1985",
	"Baraduke\0", NULL, "Namco", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_PRE90S, GBF_HORSHOOT, 0,
	NULL, baradukeRomInfo, baradukeRomName, NULL, NULL, BaradukeInputInfo, BaradukeDIPInfo,
	AlienInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x800,
	288, 224, 4, 3
};


// Metro-Cross (set 1)

static struct BurnRomInfo metrocrsRomDesc[] = {
	{ "mc1-3.9c",		0x2000, 0x3390b33c, 1 | BRF_PRG | BRF_ESS }, //  0 M6809 Code
	{ "mc1-1.9a",		0x4000, 0x10b0977e, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "mc1-2.9b",		0x4000, 0x5c846f35, 1 | BRF_PRG | BRF_ESS }, //  2

	{ "mc1-4.3b",		0x2000, 0x9c88f898, 2 | BRF_PRG | BRF_ESS }, //  3 HD63701 Code
	{ "cus60-60a1.mcu",	0x1000, 0x076ea82a, 2 | BRF_PRG | BRF_ESS }, //  4

	{ "mc1-5.3j",		0x2000, 0x9b5ea33a, 3 | BRF_GRA },           //  5 Characters

	{ "mc1-7.4p",		0x4000, 0xc9dfa003, 4 | BRF_GRA },           //  6 Background Tiles
	{ "mc1-6.4n",		0x4000, 0x9686dc3c, 4 | BRF_GRA },           //  7

	{ "mc1-8.8k",		0x4000, 0x265b31fa, 5 | BRF_GRA },           //  8 Sprites
	{ "mc1-9.8l",		0x4000, 0x541ec029, 5 | BRF_GRA },           //  9

	{ "mc1-1.1n",		0x0800, 0x32a78a8b, 6 | BRF_GRA },           // 10 Color PROMs
	{ "mc1-2.2m",		0x0800, 0x6f4dca7b, 6 | BRF_GRA },           // 11
};

STD_ROM_PICK(metrocrs)
STD_ROM_FN(metrocrs)

static int MetroInit()
{
	return DrvInit(1);
}

struct BurnDriver BurnDrvMetrocrs = {
	"metrocrs", NULL, NULL, NULL, "1985",
	"Metro-Cross (set 1)\0", NULL, "Namco", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_PRE90S, GBF_PLATFORM, 0,
	NULL, metrocrsRomInfo, metrocrsRomName, NULL, NULL, BaradukeInputInfo, MetrocrsDIPInfo,
	MetroInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x800,
	288, 224, 4, 3
};


// Metro-Cross (set 2)

static struct BurnRomInfo metrocrsaRomDesc[] = {
	{ "mc2-3.9b",		0x2000, 0xffe08075, 1 | BRF_PRG | BRF_ESS }, //  0 M6809 Code
	{ "mc2-1.9a",		0x4000, 0x05a239ea, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "mc2-2.9a",		0x4000, 0xdb9b0e6d, 1 | BRF_PRG | BRF_ESS }, //  2

	{ "mc1-4.3b",		0x2000, 0x9c88f898, 2 | BRF_PRG | BRF_ESS }, //  3 HD63701 Code
	{ "cus60-60a1.mcu",	0x1000, 0x076ea82a, 2 | BRF_PRG | BRF_ESS }, //  4

	{ "mc1-5.3j",		0x2000, 0x9b5ea33a, 3 | BRF_GRA },           //  5 Characters

	{ "mc1-7.4p",		0x4000, 0xc9dfa003, 4 | BRF_GRA },           //  6 Background Tiles
	{ "mc1-6.4n",		0x4000, 0x9686dc3c, 4 | BRF_GRA },           //  7

	{ "mc1-8.8k",		0x4000, 0x265b31fa, 5 | BRF_GRA },           //  8 Sprites
	{ "mc1-9.8l",		0x4000, 0x541ec029, 5 | BRF_GRA },           //  9

	{ "mc1-1.1n",		0x0800, 0x32a78a8b, 6 | BRF_GRA },           // 10 Color PROMs
	{ "mc1-2.2m",		0x0800, 0x6f4dca7b, 6 | BRF_GRA },           // 11
};

STD_ROM_PICK(metrocrsa)
STD_ROM_FN(metrocrsa)

struct BurnDriver BurnDrvMetrocrsa = {
	"metrocrsa", "metrocrs", NULL, NULL, "1985",
	"Metro-Cross (set 2)\0", NULL, "Namco", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_PRE90S, GBF_PLATFORM, 0,
	NULL, metrocrsaRomInfo, metrocrsaRomName, NULL, NULL, BaradukeInputInfo, MetrocrsDIPInfo,
	MetroInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x800,
	288, 224, 4, 3
};
