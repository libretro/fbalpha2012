// FB Alpha Welltris driver module
// Based on MAME driver David Haywood

#include "tiles_generic.h"
#include "burn_ym2610.h"

static unsigned char *AllMem;
static unsigned char *MemEnd;
static unsigned char *AllRam;
static unsigned char *RamEnd;
static unsigned char *Drv68KROM;
static unsigned char *DrvZ80ROM;
static unsigned char *DrvGfxROM0;
static unsigned char *DrvGfxROM1;
static unsigned char *DrvSndROM0;
static unsigned char *DrvSndROM1;
static unsigned char *Drv68KRAM;
static unsigned char *DrvPalRAM;
static unsigned char *DrvSprRAM;
static unsigned char *DrvVidRAM;
static unsigned char *DrvPxlRAM;
static unsigned char *DrvZ80RAM;

static unsigned int  *DrvPalette;
static unsigned char DrvRecalc;

static unsigned char *soundlatch;
static unsigned char *pending_command;

static unsigned char *flipscreen;
static unsigned char *sound_bank;
static unsigned char *gfx_bank;
static unsigned char *spritepalbank;
static unsigned char *pixelpalbank;
static unsigned char *charpalbank;

static unsigned short *scroll;

static unsigned char DrvInputs[6];
static unsigned char DrvJoy1[8];
static unsigned char DrvJoy2[8];
static unsigned char DrvJoy3[8];
static unsigned char DrvJoy4[8];
static unsigned char DrvJoy5[8];
static unsigned char DrvJoy6[8];
static unsigned char DrvDips[2];
static unsigned char DrvReset;

static int screen_y_offset;

static struct BurnInputInfo WelltrisInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy2 + 0,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy2 + 1,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy2 + 2,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy2 + 3,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p1 fire 2"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy1 + 1,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy1 + 3,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy3 + 0,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy3 + 1,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy3 + 2,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy3 + 3,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy3 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy3 + 5,	"p2 fire 2"	},

/*	{"P3 Coin",		BIT_DIGITAL,	DrvJoy6 + 0,	"p3 coin"	},
	{"P3 Start",		BIT_DIGITAL,	DrvJoy6 + 2,	"p3 start"	},
	{"P3 Up",		BIT_DIGITAL,	DrvJoy4 + 0,	"p3 up"		},
	{"P3 Down",		BIT_DIGITAL,	DrvJoy4 + 1,	"p3 down"	},
	{"P3 Left",		BIT_DIGITAL,	DrvJoy4 + 2,	"p3 left"	},
	{"P3 Right",		BIT_DIGITAL,	DrvJoy4 + 3,	"p3 right"	},
	{"P3 Button 1",		BIT_DIGITAL,	DrvJoy4 + 4,	"p3 fire 1"	},
	{"P3 Button 2",		BIT_DIGITAL,	DrvJoy4 + 5,	"p3 fire 2"	},

	{"P4 Coin",		BIT_DIGITAL,	DrvJoy6 + 1,	"p4 coin"	},
	{"P4 Start",		BIT_DIGITAL,	DrvJoy6 + 3,	"p4 start"	},
	{"P4 Up",		BIT_DIGITAL,	DrvJoy5 + 0,	"p4 up"		},
	{"P4 Down",		BIT_DIGITAL,	DrvJoy5 + 1,	"p4 down"	},
	{"P4 Left",		BIT_DIGITAL,	DrvJoy5 + 2,	"p4 left"	},
	{"P4 Right",		BIT_DIGITAL,	DrvJoy5 + 3,	"p4 right"	},
	{"P4 Button 1",		BIT_DIGITAL,	DrvJoy5 + 4,	"p4 fire 1"	},
	{"P4 Button 2",		BIT_DIGITAL,	DrvJoy5 + 5,	"p4 fire 2"	},*/

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Test",		BIT_DIGITAL,	DrvJoy1 + 4,	"diag"	},
	{"Service",		BIT_DIGITAL,	DrvJoy1 + 6,	"service"	},
//	{"Tilt",		BIT_DIGITAL,	DrvJoy1 + 5,	"tilt"		},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
};

STDINPUTINFO(Welltris)

static struct BurnInputInfo Quiz18kInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy2 + 0,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy2 + 1,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy2 + 2,	"p1 left"},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy2 + 3,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p1 fire 2"	},
	{"P1 Button 3",		BIT_DIGITAL,	DrvJoy2 + 6,	"p1 fire 3"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy1 + 1,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy1 + 3,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy3 + 0,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy3 + 1,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy3 + 2,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy3 + 3,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy3 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy3 + 5,	"p2 fire 2"	},
	{"P2 Button 3",		BIT_DIGITAL,	DrvJoy3 + 6,	"p2 fire 3"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy1 + 6,	"service"	},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
};

STDINPUTINFO(Quiz18k)

static struct BurnDIPInfo WelltrisDIPList[]=
{
	{0x13, 0xff, 0xff, 0xff, NULL				},
	{0x14, 0xff, 0xff, 0xfb, NULL				},

	{0   , 0xfe, 0   ,   16, "Coin A"			},
	{0x13, 0x01, 0x0f, 0x06, "5 Coins 1 Credit"		},
	{0x13, 0x01, 0x0f, 0x07, "4 Coins 1 Credit"		},
	{0x13, 0x01, 0x0f, 0x08, "3 Coins 1 Credit"		},
	{0x13, 0x01, 0x0f, 0x09, "2 Coins 1 Credit"		},
	{0x13, 0x01, 0x0f, 0x04, "2-1, 4-2, 5-3, 6-4"		},
	{0x13, 0x01, 0x0f, 0x03, "2-1, 4-3"			},
	{0x13, 0x01, 0x0f, 0x0f, "1 Coin  1 Credit"		},
	{0x13, 0x01, 0x0f, 0x01, "1-1, 2-2, 3-3, 4-5"		},
	{0x13, 0x01, 0x0f, 0x02, "1-1, 2-2, 3-3, 4-4, 5-6"	},
	{0x13, 0x01, 0x0f, 0x00, "1-1, 2-3"			},
	{0x13, 0x01, 0x0f, 0x05, "2 Coins 3 Credits"		},
	{0x13, 0x01, 0x0f, 0x0e, "1 Coin  2 Credits"		},
	{0x13, 0x01, 0x0f, 0x0d, "1 Coin  3 Credits"		},
	{0x13, 0x01, 0x0f, 0x0c, "1 Coin  4 Credits"		},
	{0x13, 0x01, 0x0f, 0x0b, "1 Coin  5 Credits"		},
	{0x13, 0x01, 0x0f, 0x0a, "1 Coin  6 Credits"		},

	{0   , 0xfe, 0   ,   16, "Coin B"			},
	{0x13, 0x01, 0xf0, 0x60, "5 Coins 1 Credit"		},
	{0x13, 0x01, 0xf0, 0x70, "4 Coins 1 Credit"		},
	{0x13, 0x01, 0xf0, 0x80, "3 Coins 1 Credit"		},
	{0x13, 0x01, 0xf0, 0x90, "2 Coins 1 Credit"		},
	{0x13, 0x01, 0xf0, 0x40, "2-1, 4-2, 5-3, 6-4"		},
	{0x13, 0x01, 0xf0, 0x30, "2-1, 4-3"			},
	{0x13, 0x01, 0xf0, 0xf0, "1 Coin  1 Credit"		},
	{0x13, 0x01, 0xf0, 0x10, "1-1, 2-2, 3-3, 4-5"		},
	{0x13, 0x01, 0xf0, 0x20, "1-1, 2-2, 3-3, 4-4, 5-6"	},
	{0x13, 0x01, 0xf0, 0x00, "1-1, 2-3"			},
	{0x13, 0x01, 0xf0, 0x50, "2 Coins 3 Credits"		},
	{0x13, 0x01, 0xf0, 0xe0, "1 Coin  2 Credits"		},
	{0x13, 0x01, 0xf0, 0xd0, "1 Coin  3 Credits"		},
	{0x13, 0x01, 0xf0, 0xc0, "1 Coin  4 Credits"		},
	{0x13, 0x01, 0xf0, 0xb0, "1 Coin  5 Credits"		},
	{0x13, 0x01, 0xf0, 0xa0, "1 Coin  6 Credits"		},

	{0   , 0xfe, 0   ,    4, "Difficulty"			},
	{0x14, 0x01, 0x03, 0x02, "Easy"				},
	{0x14, 0x01, 0x03, 0x03, "Normal"			},
	{0x14, 0x01, 0x03, 0x01, "Hard"				},
	{0x14, 0x01, 0x03, 0x00, "Hardest"			},

	{0   , 0xfe, 0   ,    2, "Coin Mode"			},
	{0x14, 0x01, 0x04, 0x04, "Mono Player"			},
	{0x14, 0x01, 0x04, 0x00, "Many Player"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"			},
	{0x14, 0x01, 0x08, 0x00, "Off"				},
	{0x14, 0x01, 0x08, 0x08, "On"				},

//	{0   , 0xfe, 0   ,    2, "DIPSW 2-5 (see notes)"	},
//	{0x14, 0x01, 0x10, 0x10, "Off"				},
//	{0x14, 0x01, 0x10, 0x00, "On"				},

//	{0   , 0xfe, 0   ,    2, "4 Players Mode"		},
//	{0x14, 0x01, 0x30, 0x30, "Off"				},
//	{0x14, 0x01, 0x30, 0x00, "On"				},

	{0   , 0xfe, 0   ,    2, "Flip Screen"			},
	{0x14, 0x01, 0x40, 0x40, "Off"				},
	{0x14, 0x01, 0x40, 0x00, "On"				},

	{0   , 0xfe, 0   ,    2, "Service Mode"			},
	{0x14, 0x01, 0x80, 0x80, "Off"				},
	{0x14, 0x01, 0x80, 0x00, "On"				},
};

STDDIPINFO(Welltris)

static struct BurnDIPInfo Quiz18kDIPList[]=
{
	{0x14, 0xff, 0xff, 0xff, NULL				},
	{0x15, 0xff, 0xff, 0xfd, NULL				},

	{0   , 0xfe, 0   ,    2, "Flip Screen"			},
	{0x15, 0x01, 0x01, 0x01, "Off"				},
	{0x15, 0x01, 0x01, 0x00, "On"				},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"			},
	{0x15, 0x01, 0x02, 0x02, "Off"				},
	{0x15, 0x01, 0x02, 0x00, "On"				},

	{0   , 0xfe, 0   ,    2, "Title Logo Type"		},
	{0x15, 0x01, 0x20, 0x20, "1"				},
	{0x15, 0x01, 0x20, 0x00, "2"				},

	{0   , 0xfe, 0   ,    2, "Service Mode"			},
	{0x15, 0x01, 0x80, 0x80, "Off"				},
	{0x15, 0x01, 0x80, 0x00, "On"				},
};

STDDIPINFO(Quiz18k)

inline void WelltrisClearOpposites(unsigned char* nJoystickInputs)
{
	if ((*nJoystickInputs & 0x03) == 0x03) {
		*nJoystickInputs &= ~0x03;
	}
	if ((*nJoystickInputs & 0x0c) == 0x0c) {
		*nJoystickInputs &= ~0x0c;
	}
}

static void cpu_sync()
{
	unsigned int cycles = (SekTotalCycles() * 2) - (ZetTotalCycles() * 5);

	if (cycles > 5) {
		BurnTimerUpdate(cycles / 5);
	}
}

static inline void sprite_hack(int offset)
{
	unsigned short *ram = (unsigned short*)DrvSprRAM;

	if (offset == 0x3fc) {
		if (ram[0x1fc] == 0 && ram[0x1fd] == 0 && ram[0x1ff] == 0) {
			memset (DrvSprRAM, 0, 0x3fc);
		}
	}
}

static inline void palette_write(int offset)
{
	unsigned short p = *((unsigned short*)(DrvPalRAM + offset));

	int r = (p >> 10) & 0x1f;
	int g = (p >>  5) & 0x1f;
	int b = (p >>  0) & 0x1f;

	r = (r << 3) | (r >> 2);
	g = (g << 3) | (g >> 2);
	b = (b << 3) | (b >> 2);

	DrvPalette[offset / 2] = BurnHighCol(r, g, b, 0);
}

void __fastcall welltris_main_write_byte(unsigned int address, unsigned char data)
{
	if ((address & 0xfffffc00) == 0xffc000) {
		DrvSprRAM[(address & 0x3ff) ^ 1] = data;
		sprite_hack(address & 0x3fe);
		return;
	}

	if ((address & 0xfffff000) == 0xffe000) {
		DrvPalRAM[(address & 0xfff) ^ 1] = data;
		palette_write(address & 0xffe);
		return;
	}

	switch (address)
	{
		case 0xfff001:
			*charpalbank   = data & 0x03;
			*pixelpalbank  =(data & 0x08) >> 3;
			*spritepalbank =(data & 0x20) >> 5;
			*flipscreen    = data & 0x80;
		return;

		case 0xfff003:
			gfx_bank[0] = data >> 4;
			gfx_bank[1] = data & 0x0f;
		return;

		case 0xfff009:
			cpu_sync();
			*pending_command = 0x80;
			*soundlatch = data;
			ZetNmi();
		return;
	}
}
		
void __fastcall welltris_main_write_word(unsigned int address, unsigned short data)
{
	if ((address & 0xfffffc00) == 0xffc000) {
		*((unsigned short*)(DrvSprRAM + (address & 0x3fe))) = data;
		sprite_hack(address & 0x3fe);
		return;
	}

	if ((address & 0xfffff000) == 0xffe000) {
		*((unsigned short*)(DrvPalRAM + (address & 0xffe))) = data;
		palette_write(address & 0xffe);
		return;
	}

	switch (address)
	{
		case 0xfff004:
			scroll[0] = data - 14;
		return;

		case 0xfff006:
			scroll[1] = data;
		return;
	}
}

unsigned char __fastcall welltris_main_read_byte(unsigned int address)
{
	switch (address)
	{
		case 0xfff001:
			return ~DrvInputs[1];

		case 0xfff003:
			return ~DrvInputs[2];

		case 0xfff005:
			return ~DrvInputs[3];

		case 0xfff007:
			return ~DrvInputs[4];

		case 0xfff009:
			cpu_sync();
			return (DrvInputs[0] & 0x7f) | *pending_command;

		case 0xfff00b:
			return ~DrvInputs[5];

		case 0xfff00d:
			return DrvDips[0];

		case 0xfff00f:
			return DrvDips[1];
	}

	return 0;
}

unsigned short __fastcall welltris_main_read_word(unsigned int )
{
	return 0;
}

static void bankswitch(int data)
{
	sound_bank[0] = data;

	ZetMapArea(0x8000, 0xffff, 0, DrvZ80ROM + (data & 0x03) * 0x8000);
	ZetMapArea(0x8000, 0xffff, 2, DrvZ80ROM + (data & 0x03) * 0x8000);
}

void __fastcall welltris_sound_write_port(unsigned short port, unsigned char data)
{
	switch (port & 0xff)
	{
		case 0x00:
			bankswitch(data);
		return;

		case 0x08:
		case 0x09:
		case 0x0a:
		case 0x0b:
			BurnYM2610Write(port & 3, data);
		return;

		case 0x18:
			*pending_command = 0;
		return;
	}
}

unsigned char __fastcall welltris_sound_read_port(unsigned short port)
{
	switch (port & 0xff)
	{
		case 0x08:
			return BurnYM2610Read(0);

		case 0x0a:
			return BurnYM2610Read(2);

		case 0x10:
			return *soundlatch;
	}

	return 0;
}

static void DrvFMIRQHandler(int, int nStatus)
{
	if (nStatus) {
		ZetSetIRQLine(0xFF, ZET_IRQSTATUS_ACK);
	} else {
		ZetSetIRQLine(0,    ZET_IRQSTATUS_NONE);
	}
}

static int DrvSynchroniseStream(int nSoundRate)
{
	return (long long)ZetTotalCycles() * nSoundRate / 4000000;
}

static double DrvGetTime()
{
	return (double)ZetTotalCycles() / 4000000.0;
}

static int DrvDoReset()
{
	memset (AllRam, 0, RamEnd - AllRam);

	SekOpen(0);
	SekReset();
	SekClose();

	ZetOpen(0);
	ZetReset();
	ZetClose();

	BurnYM2610Reset();

	return 0;
}

static int MemIndex()
{
	unsigned char *Next; Next = AllMem;

	Drv68KROM	= Next; Next += 0x180000;
	DrvZ80ROM	= Next; Next += 0x020000;

	DrvGfxROM0	= Next; Next += 0x300000;
	DrvGfxROM1	= Next; Next += 0x200000;

	DrvSndROM0	= Next; Next += 0x080000;
	DrvSndROM1	= Next; Next += 0x100000;

	DrvPalette	= (unsigned int*)Next; Next += 0x0800 * sizeof(int);

	AllRam		= Next;

	Drv68KRAM	= Next; Next += 0x004000;
	DrvPalRAM	= Next; Next += 0x001000;
	DrvSprRAM	= Next; Next += 0x000400;
	DrvVidRAM	= Next; Next += 0x001000;
	DrvPxlRAM	= Next; Next += 0x020000;

	DrvZ80RAM	= Next; Next += 0x000800;

	soundlatch 	= Next; Next += 0x000001;
	pending_command = Next; Next += 0x000001;
	flipscreen	= Next; Next += 0x000001;
	sound_bank	= Next; Next += 0x000001;

	gfx_bank 	= Next; Next += 0x000002;

	charpalbank	= Next; Next += 0x000001;
	spritepalbank	= Next; Next += 0x000001;
	pixelpalbank	= Next; Next += 0x000001;

	scroll		= (unsigned short*)Next; Next += 0x000002 * sizeof(short);

	RamEnd		= Next;

	MemEnd		= Next;

	return 0;
}

static void DrvGfxExpand(unsigned char *gfx, int len) // gfx0 - 0, gfx1 - 600
{
	for (int i = len - 1; i >= 0; i--) {
		gfx[i * 2 + 1] = gfx[i] >> 4;
		gfx[i * 2 + 0] = gfx[i] & 0x0f;
	}
}

static void DrvFixSprites()
{
	for (int i = 0; i < 0x100000; i+=4) {
		int t = DrvGfxROM1[i + 1];
		DrvGfxROM1[i + 1] = DrvGfxROM1[i + 2];
		DrvGfxROM1[i + 2] = t;
	}
}

static int DrvInit()
{
	AllMem = NULL;
	MemIndex();
	int nLen = MemEnd - (unsigned char *)0;
	if ((AllMem = (unsigned char *)malloc(nLen)) == NULL) return 1;
	memset(AllMem, 0, nLen);
	MemIndex();

	screen_y_offset = (strncmp(BurnDrvGetTextA(DRV_NAME), "welltris", 8) == 0) ? 8 : 0;

	{
		if (BurnLoadRom(Drv68KROM + 0x000001,  0, 2)) return 1;
		if (BurnLoadRom(Drv68KROM + 0x000000,  1, 2)) return 1;
		if (BurnLoadRom(Drv68KROM + 0x100001,  2, 2)) return 1;
		if (BurnLoadRom(Drv68KROM + 0x100000,  3, 2)) return 1;

		if (BurnLoadRom(DrvZ80ROM,	       4, 1)) return 1;

		if (BurnLoadRom(DrvSndROM0,	       5, 1)) return 1;

		if (BurnLoadRom(DrvSndROM1 + 0x00000,  6, 1)) return 1;
		if (BurnLoadRom(DrvSndROM1 + 0x80000,  7, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM1 + 0x00000,  8, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x00001,  9, 2)) return 1;

		if (BurnLoadRom(DrvGfxROM0 + 0x00000, 10, 1)) return 1;

		if (screen_y_offset == 0) {
			if (BurnLoadRom(DrvGfxROM0 + 0x080000, 11, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM0 + 0x100000, 12, 1)) return 1;
		} else {
			// welltris 4 player hack (change ori 0030 to 0000
			*((unsigned short *)(Drv68KROM + 0xB91E)) = 0x0000;
		}

		DrvFixSprites();
		DrvGfxExpand(DrvGfxROM0, 0x180000);
		DrvGfxExpand(DrvGfxROM1, 0x100000);
	}

	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM + 0x000000, 0x000000, 0x03ffff, SM_ROM);
	SekMapMemory(Drv68KROM + 0x100000, 0x100000, 0x17ffff, SM_ROM);
	SekMapMemory(DrvPxlRAM,		   0x800000, 0x81ffff, SM_RAM);
	SekMapMemory(Drv68KRAM,		   0xff8000, 0xffbfff, SM_RAM);
	SekMapMemory(DrvSprRAM,		   0xffc000, 0xffc3ff, SM_ROM);
	SekMapMemory(DrvVidRAM,		   0xffd000, 0xffdfff, SM_RAM);
	SekMapMemory(DrvPalRAM,		   0xffe000, 0xffefff, SM_ROM);
	SekSetWriteByteHandler(0,	   welltris_main_write_byte);
	SekSetWriteWordHandler(0,	   welltris_main_write_word);
	SekSetReadByteHandler(0,	   welltris_main_read_byte);
	SekSetReadWordHandler(0,           welltris_main_read_word);
	SekClose();

	ZetInit(1);
	ZetOpen(0);
	ZetMapArea(0x0000, 0x77ff, 0, DrvZ80ROM);
	ZetMapArea(0x0000, 0x77ff, 2, DrvZ80ROM);
	ZetMapArea(0x7800, 0x7fff, 0, DrvZ80RAM);
	ZetMapArea(0x7800, 0x7fff, 1, DrvZ80RAM);
	ZetMapArea(0x7800, 0x7fff, 2, DrvZ80RAM);
	ZetSetOutHandler(welltris_sound_write_port);
	ZetSetInHandler(welltris_sound_read_port);
	ZetMemEnd();
	ZetClose();

	int DrvSndROMLen1 = 0x100000;
	int DrvSndROMLen0 = 0x080000;

	BurnYM2610Init(8000000, DrvSndROM1, &DrvSndROMLen1, DrvSndROM0, &DrvSndROMLen0, &DrvFMIRQHandler, DrvSynchroniseStream, DrvGetTime, 0);
	BurnTimerAttachZet(4000000);
	BurnYM2610SetSoundMixMode(1);

	GenericTilesInit();

	DrvDoReset();	

	return 0;
}

static int DrvExit()
{
	GenericTilesExit();

	BurnYM2610Exit();
	BurnYM2610SetSoundMixMode(0);
	
	ZetExit();
	SekExit();
	
	free(AllMem);
	AllMem = NULL;

	return 0;
}

static void draw_sprites()
{
	unsigned short *ram = (unsigned short*)DrvSprRAM;
	static const unsigned char zoomtable[16] = { 0,7,14,20,25,30,34,38,42,46,49,52,54,57,59,61 };

	for (int offs = 0; offs < 0x200 - 4; offs += 4) {
		int data0 = ram[offs + 0];
		int data1 = ram[offs + 1];
		int data2 = ram[offs + 2];
		int data3 = ram[offs + 3];

		int code = data3 & 0x1fff;
		int color = (data2 & 0x0f) + (0x10 * *spritepalbank) + 0x60;
		int y = (data0 & 0x1ff) + 1 - screen_y_offset;
		int x = (data1 & 0x1ff) + 6 - 15;
		int yzoom = (data0 >> 12) & 15;
		int xzoom = (data1 >> 12) & 15;
		int zoomed = (xzoom | yzoom);
		int ytiles = ((data2 >> 12) & 7) + 1;
		int xtiles = ((data2 >>  8) & 7) + 1;
		int yflip = (data2 >> 15) & 1;
		int xflip = (data2 >> 11) & 1;
		int xt, yt;

		if (!(ram[offs + 2] & 0x0080)) continue;

		xzoom = 16 - zoomtable[xzoom] / 8;
		yzoom = 16 - zoomtable[yzoom] / 8;

		if (x >= 0x200) x -= 0x200;
		if (y >= 0x200) y -= 0x200;

		if (!xflip && !yflip) {
			for (yt = 0; yt < ytiles; yt++) {
				for (xt = 0; xt < xtiles; xt++, code++) {
					if (!zoomed)
						Render16x16Tile_Mask_Clip(pTransDraw, code, x + xt * 16, y + yt * 16, color, 4, 15, 0, DrvGfxROM1);
					else
						RenderZoomedTile(pTransDraw, DrvGfxROM1, code, color << 4, 15, x + xt * xzoom, y + yt * yzoom, 0, 0, 16, 16, 0x1000 * xzoom, 0x1000 * yzoom);
				}
				if (xtiles == 3) code += 1;
				if (xtiles == 5) code += 3;
				if (xtiles == 6) code += 2;
				if (xtiles == 7) code += 1;
			}
		}
		else if (xflip && !yflip) {
			for (yt = 0; yt < ytiles; yt++) {
				for (xt = 0; xt < xtiles; xt++, code++) {
					if (!zoomed)
						Render16x16Tile_Mask_FlipX_Clip(pTransDraw, code, x + (xtiles - 1 - xt) * 16, y + yt * 16, color, 4, 15, 0, DrvGfxROM1);
					else
						RenderZoomedTile(pTransDraw, DrvGfxROM1, code, color << 4, 15, x + (xtiles - 1 - xt) * xzoom, y + yt * yzoom, 1, 0, 16, 16, 0x1000 * xzoom, 0x1000 * yzoom);
				}
				if (xtiles == 3) code += 1;
				if (xtiles == 5) code += 3;
				if (xtiles == 6) code += 2;
				if (xtiles == 7) code += 1;
			}
		}
		else if (!xflip && yflip) {
			for (yt = 0; yt < ytiles; yt++) {
				for (xt = 0; xt < xtiles; xt++, code++) {
					if (!zoomed)
						Render16x16Tile_Mask_FlipY_Clip(pTransDraw, code, x + xt * 16, y + (ytiles - 1 - yt) * 16, color, 4, 15, 0, DrvGfxROM1);
					else
						RenderZoomedTile(pTransDraw, DrvGfxROM1, code, color << 4, 15, x + xt * xzoom, y + (ytiles - 1 - yt) * yzoom, 0, 1, 16, 16, 0x1000 * xzoom, 0x1000 * yzoom);
				}
				if (xtiles == 3) code += 1;
				if (xtiles == 5) code += 3;
				if (xtiles == 6) code += 2;
				if (xtiles == 7) code += 1;
			}
		}
		else {
			for (yt = 0; yt < ytiles; yt++) {
				for (xt = 0; xt < xtiles; xt++, code++) {
					if (!zoomed)
						Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, code, x + (xtiles - 1 - xt) * 16, y + (ytiles - 1 - yt) * 16, color, 4, 15, 0, DrvGfxROM1);
					else
						RenderZoomedTile(pTransDraw, DrvGfxROM1, code, color << 4, 15, x + (xtiles - 1 - xt) * xzoom, y + (ytiles - 1 - yt) * yzoom, 1, 1, 16, 16, 0x1000 * xzoom, 0x1000 * yzoom);
				}
				if (xtiles == 3) code += 1;
				if (xtiles == 5) code += 3;
				if (xtiles == 6) code += 2;
				if (xtiles == 7) code += 1;
			}
		}
	}
}

static void draw_foreground()
{
	int color_bank = *charpalbank << 3;
	int scrollx = scroll[0] & 0x1ff;
	int scrolly = scroll[1] & 0x0ff;

	unsigned short *vram = (unsigned short*)DrvVidRAM;

	for (int offs = 0; offs < 64 * 32; offs++)
	{
		int sx = (offs & 0x3f) << 3;
		int sy = (offs >> 6) << 3;

		sx -= scrollx;
		if (sx < -7) sx += 512;
		sy -= scrolly;
		if (sy < -7) sy += 256;

		int attr  = vram[offs];
		int code  = (attr & 0xfff) | (gfx_bank[(attr >> 12) & 1] << 12);
		int color = (attr >> 13) | color_bank;

		Render8x8Tile_Mask_Clip(pTransDraw, code, sx - 15, sy - screen_y_offset, color & 0x7f, 4, 15, 0, DrvGfxROM0);
	}
}

static void draw_background()
{
	int color_bank = 0x400 + (*pixelpalbank << 8);

	unsigned short *vram = (unsigned short*)DrvPxlRAM;

	for (int offs = (screen_y_offset * 256); offs < 256 * 256; offs++)
	{
		int sx = ((offs & 0xff) << 1) - 15;
		int sy = (offs >> 8) - screen_y_offset;

		int pxl = vram[offs];

		if (sy < nScreenHeight && sy >= 0) {
			if (sx >= 0 && sx < nScreenWidth) pTransDraw[sy * nScreenWidth + sx++] = color_bank | (pxl >> 8);
			if (sx >= 0 && sx < nScreenWidth) pTransDraw[sy * nScreenWidth + sx  ] = color_bank | (pxl & 0xff);
		}
	}
}

static int DrvDraw()
{
	if (DrvRecalc) {
		for (int i = 0; i < 0x1000; i+=2) {
			palette_write(i);
		}
		DrvRecalc = 0;
	}

	BurnTransferClear();

	draw_background();
	draw_foreground();
	draw_sprites();

	BurnTransferCopy(DrvPalette);

	return 0;
}

static int DrvFrame()
{
	if (DrvReset) {
		DrvDoReset();
	}

	SekNewFrame();
	ZetNewFrame();

	{
		DrvInputs[0] = 0xff;
		DrvInputs[1] = 0x00;
		DrvInputs[2] = 0x00;
		DrvInputs[3] = 0x00;
		DrvInputs[4] = 0x00;
		DrvInputs[5] = 0x00;

		for (int i = 0; i < 8; i++) {
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] |= (DrvJoy2[i] & 1) << i;
			DrvInputs[2] |= (DrvJoy3[i] & 1) << i;
			DrvInputs[3] |= (DrvJoy4[i] & 1) << i;
			DrvInputs[4] |= (DrvJoy5[i] & 1) << i;
			DrvInputs[5] |= (DrvJoy6[i] & 1) << i;
		}
	}

	// Clear Opposites
	WelltrisClearOpposites(&DrvInputs[1]);
	WelltrisClearOpposites(&DrvInputs[2]);
//	WelltrisClearOpposites(&DrvInputs[3]);
//	WelltrisClearOpposites(&DrvInputs[4]);

	SekOpen(0);
	ZetOpen(0);
	
	SekRun(10000000 / 60);
	SekSetIRQLine(1, SEK_IRQSTATUS_AUTO);
	
	BurnTimerEndFrame((4000000 / 60) - ZetTotalCycles());

	if (pBurnSoundOut) {
		BurnYM2610Update(pBurnSoundOut, nBurnSoundLen);
	}

	ZetClose();
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
		*pnMin = 0x029707;
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
		ZetScan(nAction);

		BurnYM2610Scan(nAction, pnMin);
	}

	if (nAction & ACB_WRITE) {
		ZetOpen(0);
		bankswitch(sound_bank[0]);
		ZetClose();
	}

	return 0;
}


// Welltris - Alexey Pajitnov's (World?, 2 players)

static struct BurnRomInfo welltrisRomDesc[] = {
	{ "j2u.8",		0x20000, 0x7488fe94, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "j1u.7",		0x20000, 0x571413ac, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "lh532j10.10",	0x40000, 0x1187c665, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "lh532j11.9",		0x40000, 0x18eda9e5, 1 | BRF_PRG | BRF_ESS }, //  3

	{ "3.144",		0x20000, 0xae8f763e, 2 | BRF_PRG | BRF_ESS }, //  4 Z80 Code

	{ "lh534j11.126",	0x80000, 0xbf85fb0d, 3 | BRF_SND },           //  5 ADPCM-B Samples

	{ "lh534j09.123",	0x80000, 0x6c2ce9a5, 4 | BRF_SND },           //  6 ADPCM-A Samples
	{ "lh534j10.124",	0x80000, 0xe3682221, 4 | BRF_SND },           //  7

	{ "046.93",		0x40000, 0x31d96d77, 5 | BRF_GRA },           //  8 Sprites
	{ "048.94",		0x40000, 0xbb4643da, 5 | BRF_GRA },           //  9

	{ "lh534j12.77",	0x80000, 0xb61a8b74, 6 | BRF_GRA },           // 10 Tiles
};

STD_ROM_PICK(welltris)
STD_ROM_FN(welltris)

struct BurnDriver BurnDrvWelltris = {
	"welltris", NULL, NULL, NULL, "1991",
	"Welltris - Alexey Pajitnov's (World?, 2 players)\0", NULL, "Video System Co.", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_PUZZLE, 0,
	NULL, welltrisRomInfo, welltrisRomName, NULL, NULL, WelltrisInputInfo, WelltrisDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x800,
	352, 240, 4, 3
};


// Welltris - Alexey Pajitnov's (Japan, 2 players)

static struct BurnRomInfo welltrisjRomDesc[] = {
	{ "j2.8",		0x20000, 0x68ec5691, 1 | BRF_PRG | BRF_ESS }, //  0 maincpu
	{ "j1.7",		0x20000, 0x1598ea2c, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "lh532j10.10",	0x40000, 0x1187c665, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "lh532j11.9",		0x40000, 0x18eda9e5, 1 | BRF_PRG | BRF_ESS }, //  3

	{ "3.144",		0x20000, 0xae8f763e, 2 | BRF_PRG | BRF_ESS }, //  4 Z80 Code

	{ "lh534j11.126",	0x80000, 0xbf85fb0d, 3 | BRF_SND },           //  5 ADPCM-B Samples

	{ "lh534j09.123",	0x80000, 0x6c2ce9a5, 4 | BRF_SND },           //  6 ADPCM-A Samples
	{ "lh534j10.124",	0x80000, 0xe3682221, 4 | BRF_SND },           //  7

	{ "046.93",		0x40000, 0x31d96d77, 5 | BRF_GRA },           //  8 Sprites
	{ "048.94",		0x40000, 0xbb4643da, 5 | BRF_GRA },           //  9

	{ "lh534j12.77",	0x80000, 0xb61a8b74, 6 | BRF_GRA },           // 10 Tiles
};

STD_ROM_PICK(welltrisj)
STD_ROM_FN(welltrisj)

struct BurnDriver BurnDrvWelltrisj = {
	"welltrisj", "welltris", NULL, NULL, "1991",
	"Welltris - Alexey Pajitnov's (Japan, 2 players)\0", NULL, "Video System Co.", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_POST90S, GBF_PUZZLE, 0,
	NULL, welltrisjRomInfo, welltrisjRomName, NULL, NULL, WelltrisInputInfo, WelltrisDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x800,
	352, 240, 4, 3
};


// Miyasu Nonki no Quiz 18-Kin

static struct BurnRomInfo quiz18kRomDesc[] = {
	{ "1-ic8.bin",		0x20000, 0x10a64336, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "2-ic7.bin",		0x20000, 0x8b21b431, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "ic10.bin",		0x40000, 0x501453a3, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "ic9.bin",		0x40000, 0x99b6840f, 1 | BRF_PRG | BRF_ESS }, //  3

	{ "3-ic144.bin",	0x20000, 0x72d372e3, 2 | BRF_PRG | BRF_ESS }, //  4 Z80 Code

	{ "ic126.bin",		0x40000, 0x7a92fbc9, 3 | BRF_SND },           //  5 ADPCM-B Samples

	{ "ic123.bin",		0x80000, 0xee4995cf, 4 | BRF_SND },           //  6 ADPCM-A Samples
	{ "ic124.bin",		0x40000, 0x076f58c3, 4 | BRF_SND },           //  7

	{ "ic93.bin",		0x80000, 0x4d387c5e, 5 | BRF_GRA },           //  8 Sprites
	{ "ic94.bin",		0x80000, 0x6be2f164, 5 | BRF_GRA },           //  9

	{ "ic77.bin",		0x80000, 0xaf3b6fd1, 6 | BRF_GRA },           // 10 Tiles
	{ "ic78.bin",		0x80000, 0x44bbdef3, 6 | BRF_GRA },           // 11
	{ "ic79.bin",		0x80000, 0xd721e169, 6 | BRF_GRA },           // 12
};

STD_ROM_PICK(quiz18k)
STD_ROM_FN(quiz18k)

struct BurnDriver BurnDrvQuiz18k = {
	"quiz18k", NULL, NULL, NULL, "1992",
	"Miyasu Nonki no Quiz 18-Kin\0", NULL, "EIM", "Miscellaneous",
	L"\u307F\u3084\u3059\u306E\u3093\u304D\u306E \u30AF\u30A4\u30BA\uFF11\uFF18\u7981 \uFF24\uFF52\uFF0E \u30A8\u30C3\u3061\u3083\u3093\u306E\u8A3A\u5BDF\u5BA4\0Miyasu Nonki no Quiz 18-Kin\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_QUIZ, 0,
	NULL, quiz18kRomInfo, quiz18kRomName, NULL, NULL, Quiz18kInputInfo, Quiz18kDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x800,
	320, 224, 4, 3
};
