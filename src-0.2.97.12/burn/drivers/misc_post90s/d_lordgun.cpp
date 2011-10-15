// FB Alpha Lord of Gun driver module
// Based on MAME driver by Luca Elia, XingXing, and David Haywood

#include "tiles_generic.h"
#include "burn_ymf278b.h"
#include "burn_ym3812.h"
#include "msm6295.h"
#include "8255ppi.h"
#include "eeprom.h"
#include "burn_gun.h"

static unsigned char *AllMem		= NULL;
static unsigned char *MemEnd		= NULL;
static unsigned char *AllRam		= NULL;
static unsigned char *RamEnd		= NULL;
static unsigned char *Drv68KROM		= NULL;
static unsigned char *DrvZ80ROM		= NULL;
static unsigned char *DrvGfxROM0	= NULL;
static unsigned char *DrvGfxROM1	= NULL;
static unsigned char *DrvGfxROM2	= NULL;
static unsigned char *DrvGfxROM3	= NULL;
static unsigned char *DrvSndROM0	= NULL;
static unsigned char *DrvSndROM1	= NULL;
static unsigned char *DrvSndROM2	= NULL;
static unsigned char *Drv68KRAM		= NULL;
static unsigned char *DrvPriRAM		= NULL;
static unsigned char *DrvPalRAM		= NULL;
static unsigned char *DrvVidRAM0	= NULL;
static unsigned char *DrvVidRAM1	= NULL;
static unsigned char *DrvVidRAM2	= NULL;
static unsigned char *DrvVidRAM3	= NULL;
static unsigned char *DrvScrRAM		= NULL;
static unsigned char *DrvSprRAM		= NULL;
static unsigned char *DrvProtRAM	= NULL;
static unsigned char *DrvZ80RAM		= NULL;

static unsigned int  *DrvPalette	= NULL;
static unsigned char  DrvRecalc;

static unsigned short *scrollx		= NULL;
static unsigned short *scrolly		= NULL;
static unsigned short *priority		= NULL;
static unsigned char *soundlatch	= NULL;
static char *okibank			= NULL;

static unsigned char *DrvTransTable[5]	= { NULL, NULL, NULL, NULL, NULL };
static unsigned short *draw_bitmap[5] 	= { NULL, NULL, NULL, NULL, NULL };

static unsigned char aliencha_dip_sel;
static unsigned char lordgun_whitescreen;

static int DrvAxis[4];
static unsigned short DrvAnalogInput[4];
static unsigned short DrvInputs[5];
static unsigned char DrvJoy1[16];
static unsigned char DrvJoy2[16];
static unsigned char DrvJoy3[16];
static unsigned char DrvJoy4[16];
static unsigned char DrvJoy5[16];
static unsigned char DrvDips[4];
static unsigned char DrvReset;

static int lordgun_gun_hw_x[2];
static int lordgun_gun_hw_y[2];

#define A(a, b, c, d) { a, b, (unsigned char*)(c), d }

static struct BurnInputInfo LordgunInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy4 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 start"	},
	A("P1 Right / left",	BIT_ANALOG_REL, DrvAxis + 0,	"mouse x-axis"),
	A("P1 Up / Down",	BIT_ANALOG_REL, DrvAxis + 1,	"mouse y-axis"),
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy5 + 4,	"mouse button"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy4 + 1,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 start"	},
	A("P2 Right / left",	BIT_ANALOG_REL, DrvAxis + 2,	"mouse x-axis"),
	A("P2 Up / Down",	BIT_ANALOG_REL, DrvAxis + 3,	"mouse y-axis"),
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy5 + 5,	"mouse button"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy3 + 4,	"service"	},
	{"Service",		BIT_DIGITAL,	DrvJoy1 + 7,	"service"	},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
};

STDINPUTINFO(Lordgun)

#undef A

static struct BurnInputInfo AlienchaInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy4 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 6,	"p1 fire 2"	},
	{"P1 Button 3",		BIT_DIGITAL,	DrvJoy1 + 7,	"p1 fire 3"	},
	{"P1 Button 4",		BIT_DIGITAL,	DrvJoy4 + 2,	"p1 fire 4"	},
	{"P1 Button 5",		BIT_DIGITAL,	DrvJoy4 + 3,	"p1 fire 5"	},
	{"P1 Button 6",		BIT_DIGITAL,	DrvJoy4 + 4,	"p1 fire 6"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy4 + 1,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 6,	"p2 fire 2"	},
	{"P2 Button 3",		BIT_DIGITAL,	DrvJoy2 + 7,	"p2 fire 3"	},
	{"P2 Button 4",		BIT_DIGITAL,	DrvJoy4 + 5,	"p2 fire 4"	},
	{"P2 Button 5",		BIT_DIGITAL,	DrvJoy4 + 6,	"p2 fire 5"	},
	{"P2 Button 6",		BIT_DIGITAL,	DrvJoy4 + 7,	"p2 fire 6"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy3 + 1,	"service"	},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
	{"Dip C",		BIT_DIPSWITCH,	DrvDips + 2,	"dip"		},
	{"Fake Dip",		BIT_DIPSWITCH,  DrvDips + 3,    "dip"		},
};

STDINPUTINFO(Aliencha)

static struct BurnDIPInfo LordgunDIPList[]=
{
	{0x0d, 0xff, 0xff, 0x7f, NULL			},

	{0   , 0xfe, 0   ,    2, "Stage Select"		},
	{0x0d, 0x01, 0x01, 0x01, "Off"			},
	{0x0d, 0x01, 0x01, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Guns"			},
	{0x0d, 0x01, 0x02, 0x02, "IGS"			},
	{0x0d, 0x01, 0x02, 0x00, "Konami"		},

	{0   , 0xfe, 0   ,    2, "Ranking Music"	},
	{0x0d, 0x01, 0x04, 0x04, "Exciting"		},
	{0x0d, 0x01, 0x04, 0x00, "Tender"		},

	{0   , 0xfe, 0   ,    2, "Coin Slots"		},
	{0x0d, 0x01, 0x08, 0x00, "1"			},
	{0x0d, 0x01, 0x08, 0x08, "2"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x0d, 0x01, 0x40, 0x40, "Off"			},
	{0x0d, 0x01, 0x40, 0x00, "On"			},
};

STDDIPINFO(Lordgun)

static struct BurnDIPInfo AlienchaDIPList[]=
{
	{0x1a, 0xff, 0xff, 0xff, NULL			},
	{0x1b, 0xff, 0xff, 0xff, NULL			},
	{0x1c, 0xff, 0xff, 0xff, NULL			},
	{0x1d, 0xff, 0xff, 0x00, NULL			},

	{0   , 0xfe, 0   ,    2, "Credits To Start"	},
	{0x1a, 0x01, 0x01, 0x01, "1"			},
	{0x1a, 0x01, 0x01, 0x00, "2"			},

	{0   , 0xfe, 0   ,    8, "Coin A"		},
	{0x1a, 0x01, 0x0e, 0x00, "5 Coins 1 Credits"	},
	{0x1a, 0x01, 0x0e, 0x02, "4 Coins 1 Credits"	},
	{0x1a, 0x01, 0x0e, 0x04, "3 Coins 1 Credits"	},
	{0x1a, 0x01, 0x0e, 0x06, "2 Coins 1 Credits"	},
	{0x1a, 0x01, 0x0e, 0x0e, "1 Coin  1 Credits"	},
	{0x1a, 0x01, 0x0e, 0x0c, "1 Coin  2 Credits"	},
	{0x1a, 0x01, 0x0e, 0x0a, "1 Coin  3 Credits"	},
	{0x1a, 0x01, 0x0e, 0x08, "1 Coin  4 Credits"	},

	{0   , 0xfe, 0   ,    8, "Coin B"		},
	{0x1a, 0x01, 0x70, 0x00, "5 Coins 1 Credits"	},
	{0x1a, 0x01, 0x70, 0x10, "4 Coins 1 Credits"	},
	{0x1a, 0x01, 0x70, 0x20, "3 Coins 1 Credits"	},
	{0x1a, 0x01, 0x70, 0x30, "2 Coins 1 Credits"	},
	{0x1a, 0x01, 0x70, 0x70, "1 Coin  1 Credits"	},
	{0x1a, 0x01, 0x70, 0x60, "1 Coin  2 Credits"	},
	{0x1a, 0x01, 0x70, 0x50, "1 Coin  3 Credits"	},
	{0x1a, 0x01, 0x70, 0x40, "1 Coin  4 Credits"	},

	{0   , 0xfe, 0   ,    2, "Coin Slots"		},
	{0x1a, 0x01, 0x80, 0x80, "1"			},
	{0x1a, 0x01, 0x80, 0x00, "2"			},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x1b, 0x01, 0x03, 0x03, "0"			},
	{0x1b, 0x01, 0x03, 0x02, "1"			},
	{0x1b, 0x01, 0x03, 0x01, "2"			},
	{0x1b, 0x01, 0x03, 0x00, "3"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x1b, 0x01, 0x04, 0x04, "Off"			},
	{0x1b, 0x01, 0x04, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Round Time"		},
	{0x1b, 0x01, 0x08, 0x00, "32 s"			},
	{0x1b, 0x01, 0x08, 0x08, "40 s"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x1b, 0x01, 0x10, 0x00, "Off"			},
	{0x1b, 0x01, 0x10, 0x10, "On"			},

	{0   , 0xfe, 0   ,    2, "Free Play"		},
	{0x1b, 0x01, 0x20, 0x20, "Off"			},
	{0x1b, 0x01, 0x20, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Allow Join"		},
	{0x1b, 0x01, 0x40, 0x00, "No"			},
	{0x1b, 0x01, 0x40, 0x40, "Yes"			},

	{0   , 0xfe, 0   ,    2, "Allow Continue"	},
	{0x1b, 0x01, 0x80, 0x00, "No"			},
	{0x1b, 0x01, 0x80, 0x80, "Yes"			},

	{0   , 0xfe, 0   ,    3, "Buttons"		},
	{0x1c, 0x01, 0x03, 0x01, "3"			},
	{0x1c, 0x01, 0x03, 0x02, "4"			},
	{0x1c, 0x01, 0x03, 0x03, "6"			},

	{0   , 0xfe, 0   ,    2, "Vs. Rounds"		},
	{0x1c, 0x01, 0x04, 0x00, "3"			},
	{0x1c, 0x01, 0x04, 0x04, "5"			},

	{0   , 0xfe, 0   ,    2, "Language (text)"	},
	{0x1d, 0x01, 0x01, 0x00, "English"		},
	{0x1d, 0x01, 0x01, 0x01, "Chinese"		},

	{0   , 0xfe, 0   ,    2, "Language (char names)"},
	{0x1d, 0x01, 0x02, 0x00, "English"		},
	{0x1d, 0x01, 0x02, 0x02, "Chinese"		},

	{0   , 0xfe, 0   ,    2, "Title"		},
	{0x1d, 0x01, 0x04, 0x00, "Alien Challenge"	},
	{0x1d, 0x01, 0x04, 0x04, "Round House Rumble"	},
};

STDDIPINFO(Aliencha)

static struct BurnDIPInfo AlienchacDIPList[] = {
	{0x1d, 0xff, 0xff, 0x03, NULL			},
};

STDDIPINFOEXT(Alienchac, Aliencha, Alienchac)

static const short lordgun_gun_x_table[] =
{
	-100, 0x001,0x001,0x002,0x002,0x003,0x003,0x004,0x005,0x006,0x007,0x008,0x009,0x00A,0x00B,0x00C,
	0x00D,0x00E,0x00F,0x010,0x011,0x012,0x013,0x014,0x015,0x016,0x017,0x018,0x019,0x01A,0x01B,0x01C,
	0x01D,0x01E,0x01F,0x020,0x021,0x022,0x023,0x024,0x025,0x026,0x027,0x028,0x029,0x02A,0x02B,0x02C,
	0x02D,0x02E,0x02F,0x030,0x031,0x032,0x033,0x034,0x035,0x036,0x037,0x038,0x039,0x03A,0x03B,0x03C,
	0x03D,0x03E,0x03F,0x040,0x041,0x043,0x044,0x045,0x046,0x047,0x048,0x049,0x04A,0x04B,0x04C,0x04E,
	0x04F,0x050,0x051,0x052,0x053,0x054,0x055,0x056,0x057,0x059,0x05A,0x05B,0x05C,0x05D,0x05E,0x05F,
	0x060,0x061,0x05A,0x063,0x065,0x066,0x067,0x068,0x069,0x06A,0x06B,0x06C,0x06D,0x06E,0x06F,0x071,
	0x072,0x074,0x075,0x077,0x078,0x07A,0x07B,0x07D,0x07E,0x080,0x081,0x083,0x085,0x087,0x089,0x08B,
	0x08D,0x08E,0x08F,0x090,0x092,0x093,0x095,0x097,0x098,0x099,0x09A,0x09B,0x09C,0x09D,0x09E,0x0A0,
	0x0A1,0x0A2,0x0A3,0x0A4,0x0A5,0x0A6,0x0A7,0x0A8,0x0A9,0x0AA,0x0AC,0x0AD,0x0AE,0x0AF,0x0B0,0x0B1,
	0x0B2,0x0B3,0x0B4,0x0B5,0x0B6,0x0B8,0x0B9,0x0BA,0x0BB,0x0BC,0x0BD,0x0BE,0x0BF,0x0C0,0x0C1,0x0C2,
	0x0C4,0x0C5,0x0C6,0x0C7,0x0C8,0x0C9,0x0CA,0x0CB,0x0CC,0x0CD,0x0CF,0x0D0,0x0D1,0x0D2,0x0D3,0x0D4,
	0x0D5,0x0D6,0x0D7,0x0D8,0x0D9,0x0DB,0x0DC,0x0DD,0x0DE,0x0DF,0x0E0,0x0E1,0x0E2,0x0E3,0x0E4,0x0E5,
	0x0E7,0x0E8,0x0E9,0x0EA,0x0EB,0x0EC,0x0ED,0x0EE,0x0EF,0x0F0,0x0F1,0x0F3,0x0F4,0x0F5,0x0F6,0x0F7,
	0x0F8,0x0F9,0x0FA,0x0FB,0x0FC,0x0FE,0x0FF,0x100,0x101,0x102,0x103,0x104,0x105,0x106,0x107,0x108,
	0x10A,0x10B,0x10C,0x10D,0x10E,0x10F,0x110,0x111,0x112,0x113,0x114,0x116,0x117,0x118,0x119,0x11A,
	0x11B,0x11C,0x11D,0x11E,0x11F,0x120,0x122,0x123,0x124,0x125,0x126,0x127,0x128,0x129,0x12A,0x12B,
	0x12C,0x12E,0x12F,0x130,0x131,0x132,0x133,0x134,0x135,0x136,0x137,0x139,0x13A,0x13B,0x13C,0x13D,
	0x13E,0x13F,0x140,0x141,0x142,0x143,0x145,0x146,0x147,0x148,0x149,0x14A,0x14B,0x14C,0x14D,0x14E,
	0x14F,0x151,0x152,0x153,0x154,0x155,0x156,0x157,0x158,0x159,0x15A,0x15B,0x15D,0x15E,0x15F,0x160,
	0x161,0x162,0x163,0x164,0x165,0x166,0x167,0x169,0x16A,0x16B,0x16C,0x16D,0x16E,0x16F,0x170,0x171,
	0x172,0x174,0x175,0x176,0x177,0x178,0x179,0x17A,0x17B,0x17C,0x17D,0x17E,0x17F,0x180,0x181,0x182,
	0x183,0x184,0x185,0x186,0x187,0x188,0x189,0x18A,0x18B,0x18C,0x18D,0x18E,0x18F,0x190,0x191,0x192,
	0x193,0x194,0x195,0x196,0x197,0x198,0x199,0x19A,0x19B,0x19C,0x19D,0x19E,0x19F,0x1A0,0x1A1,0x1A2,
	0x1A3,0x1A4,0x1A5,0x1A6,0x1A7,0x1A8,0x1A9,0x1AA,0x1AB,0x1AC,0x1AD,0x1AE,0x1AF,0x1B0,0x1B1,0x1B2,
	0x1B3,0x1B4,0x1B5,0x1B6,0x1B7,0x1B8,0x1B9,0x1BA,0x1BB,0x1BC,0x1BD,0x1BE,0x1BF,0x1BF
};

static void lordgun_update_gun(int i)
{
	int x = DrvAnalogInput[i] - 0x3c;

	lordgun_gun_hw_x[i] = DrvAnalogInput[i];
	lordgun_gun_hw_y[i] = DrvAnalogInput[i+2];

	if ((x < 0) || (x > (int)(sizeof(lordgun_gun_x_table)/sizeof(lordgun_gun_x_table[0]))) )
		x = 0;

	int scrx = lordgun_gun_x_table[x];
	int scry = DrvAnalogInput[i+2];

	if ((scrx < 0) || (scrx >= nScreenWidth) || (scry < 0) || (scry > nScreenHeight)) {
		lordgun_gun_hw_x[i] = lordgun_gun_hw_y[i] = 0;
	}
}

void __fastcall lordgun_write_word(unsigned int address, unsigned short data)
{
	switch (address)
	{
		case 0x502000:
			scrollx[0] = data;
		return;

		case 0x502200:
			scrollx[1] = data;
		return;

		case 0x502400:
			scrollx[2] = data;
		return;

		case 0x502600:
			scrollx[3] = data;
		return;

		case 0x502800:
			scrolly[0] = data;
		return;

		case 0x502a00:
			scrolly[1] = data;
		return;

		case 0x502c00:
			scrolly[2] = data;
		return;

		case 0x502e00:
			scrolly[3] = data;
		return;

		case 0x503000:
			priority[0] = data;
		return;

		case 0x504000:
			soundlatch[0] = data >> 8;
			soundlatch[1] = data;
			ZetNmi();
		return;

		case 0x506000:
		case 0x506002:
		case 0x506004:
		case 0x506006:
			ppi8255_w(0, (address >> 1) & 3, data);
		return;

		case 0x508000:
		case 0x508002:
		case 0x508004:
		case 0x508006:
			ppi8255_w(1, (address >> 1) & 3, data);
		return;
	}
}

void __fastcall lordgun_write_byte(unsigned int address, unsigned char data)
{
	address = data; // kill warnings...
}

unsigned short __fastcall lordgun_read_word(unsigned int address)
{
	switch (address)
	{
		case 0x503800:
			return lordgun_gun_hw_x[0];

		case 0x503a00:
			return lordgun_gun_hw_x[1];

		case 0x503c00:
			return lordgun_gun_hw_y[0];

		case 0x503e00:
			return lordgun_gun_hw_y[1];

		case 0x506000:
		case 0x506002:
		case 0x506004:
		case 0x506006:
			return ppi8255_r(0, (address >> 1) & 3);

		case 0x508000:
		case 0x508002:
		case 0x508004:
		case 0x508006:
			return ppi8255_r(1, (address >> 1) & 3);
	}

	return 0;
}

unsigned char __fastcall lordgun_read_byte(unsigned int address)
{
	switch (address)
	{
		case 0x506001:
		case 0x506003:
		case 0x506005:
		case 0x506007:
			return ppi8255_r(0, (address >> 1) & 3);

		case 0x508001:
		case 0x508003:
		case 0x508005:
		case 0x508007:
			return ppi8255_r(1, (address >> 1) & 3);
	}

	return 0;
}

static void set_oki_bank(int bank)
{
	if (*okibank != (bank & 0x02)) {
		*okibank = bank & 0x02;

		memcpy (DrvSndROM0, DrvSndROM0 + 0x40000 + *okibank * 0x40000, 0x40000);
	}
}

void __fastcall lordgun_sound_write_port(unsigned short port, unsigned char data)
{
	switch (port)
	{
		case 0x1000:	// lordgun
		case 0x1001:
			BurnYM3812Write(port & 1, data);
		return;

		case 0x2000:	// lordgun
			MSM6295Command(0, data);
		return;

		case 0x6000:	// lordgun
			set_oki_bank(data);
		return;

		case 0x7000: 	// aliencha
		case 0x7001:
		case 0x7002:
			BurnYMF278BSelectRegister(port & 3, data);
		return;

		case 0x7003:
		case 0x7004:
		case 0x7005:
			BurnYMF278BWriteRegister(port & 3, data);
		return;

		case 0x7400:	// aliencha
			MSM6295Command(0, data);
		return;

		case 0x7800:	// aliencha
			MSM6295Command(1, data);
		return;
	}
}

unsigned char __fastcall lordgun_sound_read_port(unsigned short port)
{
	switch (port)
	{
		case 0x2000:	// lordgun
			return MSM6295ReadStatus(0);

		case 0x3000:
			return soundlatch[0];

		case 0x4000:
			return soundlatch[1];

		case 0x7000:	// aliencha
			return BurnYMF278BReadStatus();

		case 0x7400:	// aliencha
			return MSM6295ReadStatus(0);

		case 0x7800:	// aliencha
			return MSM6295ReadStatus(1);
	}

	return 0;
}

// ppi8255 handlers

static void aliencha_dip_select(unsigned char data)
{
	aliencha_dip_sel = data;
}

static unsigned char lordgun_dip_read()
{
	return (DrvDips[0] & 0x4f) | (EEPROMRead() ? 0x80 : 0) | (DrvInputs[4] & 0x30);
}

static void lordgun_eeprom_write(unsigned char data)
{
	static int old;

	for (int i = 0; i < 2; i++)
		if ((data & (0x04 << i)) && !(old & (0x04 << i)))
			lordgun_update_gun(i);

	// coin counter 0x01 (0)
	
	EEPROMWrite((data & 0x20), (data & 0x10), (data & 0x40));

	lordgun_whitescreen = data & 0x80;

	old = data;
}

static void aliencha_eeprom_write(unsigned char data)
{
	lordgun_whitescreen = !(data & 0x02);

	// coin counters 0x08 (0) and 0x10 (1)

	EEPROMWrite((data & 0x40), (data & 0x20), (data & 0x80));
}

static unsigned char lordgun_start1_read() { return DrvInputs[0]; }
static unsigned char lordgun_start2_read() { return DrvInputs[1]; }
static unsigned char lordgun_service_read(){ return DrvInputs[2]; } 
static unsigned char lordgun_coin_read()   { return DrvInputs[3]; }

static unsigned char aliencha_service_read(){ return (DrvInputs[2] & 0xfe) | (EEPROMRead() ? 0x01 : 0); } // aliencha eeprom read...

static unsigned char aliencha_dip_read()
{
	switch (aliencha_dip_sel & 0x70) {
		case 0x30: return DrvDips[0];
		case 0x60: return DrvDips[1];
		case 0x50: return DrvDips[2];
	}
	return 0xff;
}

// sound irq / timing handlers

static void DrvFMIRQHandler(int, int nStatus)
{
	ZetSetIRQLine(0, nStatus ? ZET_IRQSTATUS_ACK : ZET_IRQSTATUS_NONE);
}

static int DrvSynchroniseStream(int nSoundRate)
{
	return (long long)ZetTotalCycles() * nSoundRate / 5000000;
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

	BurnYMF278BReset(); // aliencha
	BurnYM3812Reset();
	MSM6295Reset(0);
	MSM6295Reset(1); // aliencha

	*okibank = -1;
	set_oki_bank(0); // lordgun

	EEPROMReset();

	aliencha_dip_sel	= 0;
	lordgun_whitescreen	= 0;

	// rom hacks
	if (!strncmp(BurnDrvGetTextA(DRV_NAME), "aliencha", 8)) {
		*((unsigned short*)(Drv68KROM + 0x00a34)) = 0x7000 | ((DrvDips[3] >> 0) & 1); // text language
		*((unsigned short*)(Drv68KROM + 0x00a38)) = 0x7000 | ((DrvDips[3] >> 1) & 1); // character name language
		*((unsigned short*)(Drv68KROM + 0x00a3c)) = 0x7000 | ((DrvDips[3] >> 2) & 1); // title
	} else {

		unsigned char lordgun_eepromdata[48] = {
			0xFF, 0x83, 0x5F, 0xFF, 0xFF, 0xBF, 0x14, 0xB7, 0xA3, 0xA4, 0x80, 0x29, 0x37, 0xA6, 0x32, 0x39, 
			0x37, 0x90, 0x10, 0x33, 0xBA, 0xA3, 0x00, 0x37, 0x01, 0x00, 0xFF, 0xFF, 0x03, 0x42, 0xFF, 0xFF, 
			0xFF, 0x83, 0xFF, 0xFF, 0xFF, 0x87, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
		};

		if (EEPROMAvailable() == 0) {
			EEPROMFill(lordgun_eepromdata, 0, 48);
		}
	}

	return 0;
}

static int MemIndex()
{
	unsigned char *Next; Next = AllMem;

	Drv68KROM	= Next; Next += 0x200000;
	DrvZ80ROM	= Next; Next += 0x010000;

	DrvGfxROM0	= Next; Next += 0x400000;
	DrvGfxROM1	= Next; Next += 0x800000;
	DrvGfxROM2	= Next; Next += 0x800000;
	DrvGfxROM3	= Next; Next += 0x1000000;

	DrvTransTable[0]= Next; Next += (0x0400000 / ( 8 *  8));
	DrvTransTable[1]= Next; Next += (0x0800000 / (16 * 16));
	DrvTransTable[2]= Next; Next += (0x0800000 / (32 * 32));
	DrvTransTable[3]= Next; Next += (0x1000000 / (16 * 16));
	DrvTransTable[4]= Next; Next += (0x0800000 / (16 * 16)) * 16;

	MSM6295ROM	= Next;
	DrvSndROM0	= Next; Next += 0x100000;
	DrvSndROM1	= Next; Next += 0x040000;
	DrvSndROM2	= Next; Next += 0x200000;

	DrvPalette	= (unsigned int*)Next; Next += (0x0800 + 1) * sizeof(int);

	draw_bitmap[0]	= (unsigned short*)Next; Next += 448 * 240 * 2;
	draw_bitmap[1]	= (unsigned short*)Next; Next += 448 * 240 * 2;
	draw_bitmap[2]	= (unsigned short*)Next; Next += 448 * 240 * 2;
	draw_bitmap[3]	= (unsigned short*)Next; Next += 448 * 240 * 2;
	draw_bitmap[4]	= (unsigned short*)Next; Next += 448 * 240 * 2;

	AllRam		= Next;

	Drv68KRAM	= Next; Next += 0x010000;
	DrvPriRAM	= Next; Next += 0x010000;
	DrvVidRAM0	= Next; Next += 0x010000;
	DrvVidRAM1	= Next; Next += 0x004000;
	DrvVidRAM2	= Next; Next += 0x004000;
	DrvVidRAM3	= Next; Next += 0x002000;
	DrvScrRAM	= Next; Next += 0x000800;
	DrvSprRAM	= Next; Next += 0x000800;
	DrvProtRAM	= Next; Next += 0x000400;
	DrvPalRAM	= Next; Next += 0x001000;

	DrvZ80RAM	= Next; Next += 0x001000;

	scrollx		= (unsigned short*)Next; Next += 0x000004 * sizeof(short);
	scrolly		= (unsigned short*)Next; Next += 0x000004 * sizeof(short);
	priority	= (unsigned short*)Next; Next += 0x000001 * sizeof(short);

	soundlatch	= Next; Next += 0x000002;

	okibank		= (char*)Next; Next += 0x000001;

	RamEnd		= Next;

	MemEnd		= Next;

	return 0;
}

static void DrvSetTransTab(unsigned char *gfx, int tab, int len, int size)
{
	for (int i = 0; i < len; i+= (size * size))
	{
		DrvTransTable[tab][i/(size*size)] = 1; // transparent

		for (int j = 0; j < (size * size); j++) {
			if (gfx[i + j] != 0x3f) {
				DrvTransTable[tab][i/(size*size)] = 0;
				break;
			}
		}
	}
}

static void DrvGfxDecode(unsigned char *gfxsrc, unsigned char *gfxdest, int len, int size)
{
	int Planes[6] = {
		(((len * 8) / 3) * 2) + 8, (((len * 8) / 3) * 2) + 0, 
		(((len * 8) / 3) * 1) + 8, (((len * 8) / 3) * 1) + 0,
		(((len * 8) / 3) * 0) + 8, (((len * 8) / 3) * 0) + 0
	};

	int XOffs1[16] = { 0x000, 0x001, 0x002, 0x003, 0x004, 0x005, 0x006, 0x007,
			   0x100, 0x101, 0x102, 0x103, 0x104, 0x105, 0x106, 0x107 };
	int XOffs2[32] = { 0x000, 0x001, 0x002, 0x003, 0x004, 0x005, 0x006, 0x007, 
			   0x200, 0x201, 0x202, 0x203, 0x204, 0x205, 0x206, 0x207,
			   0x400, 0x401, 0x402, 0x403, 0x404, 0x405, 0x406, 0x407,
			   0x600, 0x601, 0x602, 0x603, 0x604, 0x605, 0x606, 0x607 };
	int YOffs[32]  = { 0x000, 0x010, 0x020, 0x030, 0x040, 0x050, 0x060, 0x070,
			   0x080, 0x090, 0x0a0, 0x0b0, 0x0c0, 0x0d0, 0x0e0, 0x0f0,
			   0x100, 0x110, 0x120, 0x130, 0x140, 0x150, 0x160, 0x170,
			   0x180, 0x190, 0x1a0, 0x1b0, 0x1c0, 0x1d0, 0x1e0, 0x1f0 };

	unsigned char *tmp = (unsigned char*)malloc(len);
	if (tmp == NULL) {
		return;
	}

	memcpy (tmp, gfxsrc, len);

	GfxDecode(((len * 8) / 6) / (size*size), 6, size, size, Planes, (size == 32) ? XOffs2 : XOffs1, YOffs, (size*size*2), tmp, gfxdest);

	free (tmp);
}

static int DrvInit(int (*pInitCallback)(), int lordgun)
{
	AllMem = NULL;
	MemIndex();
	int nLen = MemEnd - (unsigned char *)0;
	if ((AllMem = (unsigned char *)malloc(nLen)) == NULL) return 1;
	memset(AllMem, 0, nLen);
	MemIndex();

	if (pInitCallback) {
		if (pInitCallback()) return 1;
	}

	DrvGfxDecode(DrvGfxROM0, DrvGfxROM0, 0x300000,  8);
	DrvGfxDecode(DrvGfxROM2, DrvGfxROM1, 0x600000, 16);
	DrvGfxDecode(DrvGfxROM2, DrvGfxROM2, 0x600000, 32);
	DrvGfxDecode(DrvGfxROM3, DrvGfxROM3, 0xc00000, 16);
	DrvSetTransTab(DrvGfxROM0, 0, 0x0400000, 8);
	DrvSetTransTab(DrvGfxROM1, 1, 0x0800000, 16);
	DrvSetTransTab(DrvGfxROM2, 2, 0x0800000, 32);
	DrvSetTransTab(DrvGfxROM3, 3, 0x1000000, 16);
	DrvSetTransTab(DrvGfxROM1, 4, 0x0800000, 16/4); // for line scroll

	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM,		0x000000, 0x1fffff, SM_ROM);
	SekMapMemory(Drv68KRAM,		0x200000, 0x20ffff, SM_RAM);
	SekMapMemory(DrvPriRAM,		0x210000, 0x21ffff, SM_RAM);
	SekMapMemory(DrvVidRAM0,	0x300000, 0x30ffff, SM_RAM);
	SekMapMemory(DrvVidRAM1,	0x310000, 0x313fff, SM_RAM);
	SekMapMemory(DrvVidRAM2	,	0x314000, 0x317fff, SM_RAM);
	SekMapMemory(DrvVidRAM3,	0x318000, 0x319fff, SM_RAM);
	SekMapMemory(DrvScrRAM,		0x31c000, 0x31c7ff, SM_RAM);
	SekMapMemory(DrvSprRAM,		0x400000, 0x4007ff, SM_RAM);
	SekMapMemory(DrvPalRAM,		0x500000, 0x500fff, SM_RAM);
	SekMapMemory(DrvProtRAM,	0x50a800, 0x50abff, SM_RAM); // 900-9ff (lordgun)
	SekMapMemory(DrvProtRAM,	0x50b800, 0x50bbff, SM_RAM); // 900-9ff (aliencha)
	SekSetWriteWordHandler(0,	lordgun_write_word);
	SekSetWriteByteHandler(0,	lordgun_write_byte);
	SekSetReadWordHandler(0,	lordgun_read_word);
	SekSetReadByteHandler(0,	lordgun_read_byte);
	SekClose();

	ZetInit(1);
	ZetOpen(0);
	ZetMapArea(0x0000, 0xefff, 0, DrvZ80ROM);	
	ZetMapArea(0x0000, 0xefff, 2, DrvZ80ROM);	
	ZetMapArea(0xf000, 0xffff, 0, DrvZ80RAM);	
	ZetMapArea(0xf000, 0xffff, 1, DrvZ80RAM);
	ZetMapArea(0xf000, 0xffff, 2, DrvZ80RAM);
	ZetSetOutHandler(lordgun_sound_write_port);
	ZetSetInHandler(lordgun_sound_read_port);
	ZetMemEnd();
	ZetClose();

	// aliencha
	BurnYMF278BInit(0, DrvSndROM2, &DrvFMIRQHandler, DrvSynchroniseStream);
	BurnTimerAttachZet(5000000);

	// lordgun
	BurnYM3812Init(3579545, &DrvFMIRQHandler, &DrvSynchroniseStream, 0);
	BurnTimerAttachZetYM3812(5000000);

	MSM6295Init(0, 1000000 / 132, 100, 1);
	MSM6295Init(1, 1000000 / 132, 100, 1); // aliencha

	ppi8255_init(2);
	if (lordgun) {
		PPI0PortReadA	= lordgun_dip_read;
		PPI0PortWriteB	= lordgun_eeprom_write;
		PPI0PortReadC	= lordgun_service_read;
	} else {
		PPI0PortReadA	= aliencha_dip_read;
		PPI0PortReadC	= aliencha_service_read;
		PPI0PortWriteB	= aliencha_eeprom_write;
		PPI0PortWriteC	= aliencha_dip_select;
	}

	PPI1PortReadA	= lordgun_start1_read;
	PPI1PortReadB	= lordgun_start2_read;
	PPI1PortReadC	= lordgun_coin_read;

	EEPROMInit(&eeprom_interface_93C46);

	GenericTilesInit();

	BurnGunInit(2, true);

	DrvDoReset();

	return 0;
}

static int DrvExit()
{
	GenericTilesExit();

	BurnYMF278BExit(); // aliencha
	BurnYM3812Exit();
	MSM6295Exit(0);
	MSM6295Exit(1); // aliencha

	ppi8255_exit();
	BurnGunExit();

	SekExit();
	ZetExit();

	EEPROMExit();

	free (AllMem);
	AllMem = NULL;

	return 0;
}

static int lordgunLoadRoms()
{
	if (BurnLoadRom(Drv68KROM  + 0x000001,  0, 2)) return 1;
	if (BurnLoadRom(Drv68KROM  + 0x000000,  1, 2)) return 1;

	if (BurnLoadRom(DrvZ80ROM  + 0x000000,  2, 1)) return 1;

	if (BurnLoadRom(DrvGfxROM0 + 0x000000,  3, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM0 + 0x100000,  4, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM0 + 0x200000,  5, 1)) return 1;

	if (BurnLoadRom(DrvGfxROM2 + 0x000000,  6, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM2 + 0x200000,  7, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM2 + 0x400000,  8, 1)) return 1;

	if (BurnLoadRom(DrvGfxROM3 + 0x000000,  9, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM3 + 0x200000, 10, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM3 + 0x400000, 11, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM3 + 0x600000, 12, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM3 + 0x800000, 13, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM3 + 0xa00000, 14, 1)) return 1;

	if (BurnLoadRom(DrvSndROM0 + 0x040000, 15, 1)) return 1;

	unsigned short *rom = (unsigned short*)Drv68KROM;

	for (int i = 0; i < 0x100000/2; i++) {
		if ((i & 0x0120) == 0x0100 || (i & 0x0a00) == 0x0800)
			rom[i] ^= 0x0010;
	}

	rom[0x14832/2]	=	0x6000;		// 014832: 6700 0006  beq     $1483a (protection)
	rom[0x1587e/2]	=	0x6010;		// 01587E: 6710       beq     $15890 (rom check)

	return 0;
}

static int alienchaLoadRoms()
{
	if (BurnLoadRom(Drv68KROM  + 0x000000,  0, 1)) return 1;
	BurnByteswap(Drv68KROM, 0x200000);

	if (BurnLoadRom(DrvZ80ROM  + 0x000000,  1, 1)) return 1;

	if (BurnLoadRom(DrvGfxROM0 + 0x000000,  2, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM0 + 0x100000,  3, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM0 + 0x200000,  4, 1)) return 1;

	if (BurnLoadRom(DrvGfxROM2 + 0x000000,  5, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM2 + 0x200000,  6, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM2 + 0x400000,  7, 1)) return 1;

	if (BurnLoadRom(DrvGfxROM3 + 0x000000,  8, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM3 + 0x400000,  9, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM3 + 0x800000, 10, 1)) return 1;

	if (BurnLoadRom(DrvSndROM0 + 0x040000, 11, 1)) return 1;

	if (BurnLoadRom(DrvSndROM1 + 0x000000, 12, 1)) return 1;

	if (BurnLoadRom(DrvSndROM2 + 0x000000, 13, 1)) return 1;

	unsigned short *rom = (unsigned short*)Drv68KROM;

	rom[0x0A558/2]	=	0x6012;		// 0A558  beq.s   $A56C
	rom[0x0A8DC/2]	=	0x6012;		// 0A8DC  beq.s   $A8F0
	rom[0x0AC92/2]	=	0x6012;		// 0AC92  beq.s   $ACA6
	rom[0x124CC/2]	=	0x6012;		// 124CC  beq.s   $124E0
	rom[0x12850/2]	=	0x6012;		// 12850  beq.s   $12864
	rom[0x12C06/2]	=	0x6012;		// 12C06  beq.s   $12C1A
	rom[0x1862A/2]	=	0x6012;		// 1862A  beq.s   $1863E
	rom[0x189AE/2]	=	0x6012;		// 189AE  beq.s   $189C2
	rom[0x18D64/2]	=	0x6012;		// 18D64  beq.s   $18D78
	rom[0x230FC/2]	=	0x6012;		// 230FC  beq.s   $23110
	rom[0x23480/2]	=	0x6012;		// 23480  beq.s   $23494
	rom[0x23836/2]	=	0x6012;		// 23836  beq.s   $2384A
	rom[0x2BD0E/2]	=	0x6012;		// 2BD0E  beq.s   $2BD22
	rom[0x2C092/2]	=	0x6012;		// 2C092  beq.s   $2C0A6
	rom[0x2C448/2]	=	0x6012;		// 2C448  beq.s   $2C45C

	return 0;
}

static int alienchacLoadRoms()
{
	if (BurnLoadRom(Drv68KROM  + 0x000000,  0, 1)) return 1;
	BurnByteswap(Drv68KROM, 0x200000);
	if (BurnLoadRom(Drv68KROM  + 0x000001,  1, 2)) return 1;
	if (BurnLoadRom(Drv68KROM  + 0x000000,  2, 2)) return 1;

	if (BurnLoadRom(DrvZ80ROM  + 0x000000,  3, 1)) return 1;

	if (BurnLoadRom(DrvGfxROM0 + 0x000000,  4, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM0 + 0x100000,  5, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM0 + 0x200000,  6, 1)) return 1;

	if (BurnLoadRom(DrvGfxROM2 + 0x000000,  7, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM2 + 0x200000,  8, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM2 + 0x400000,  9, 1)) return 1;

	if (BurnLoadRom(DrvGfxROM3 + 0x000000, 10, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM3 + 0x400000, 11, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM3 + 0x800000, 12, 1)) return 1;

	if (BurnLoadRom(DrvSndROM0 + 0x040000, 13, 1)) return 1;

	if (BurnLoadRom(DrvSndROM1 + 0x000000, 14, 1)) return 1;

	if (BurnLoadRom(DrvSndROM2 + 0x000000, 15, 1)) return 1;

	unsigned short *rom = (unsigned short*)Drv68KROM;

	rom[0x0A550/2]	=	0x6012;		// 0A558  beq.s   $A564
	rom[0x0A8D4/2]	=	0x6012;		// 0A8D4  beq.s   $A8E8
	rom[0x0AC8A/2]	=	0x6012;		// 0AC8A  beq.s   $AC9E
	rom[0x124B8/2]	=	0x6012;		// 124B8  beq.s   $124CC
	rom[0x1283C/2]	=	0x6012;		// 1283C  beq.s   $12850
	rom[0x12BF2/2]	=	0x6012;		// 12BF2  beq.s   $12C06
	rom[0x18616/2]	=	0x6012;		// 18616  beq.s   $1862A
	rom[0x1899A/2]	=	0x6012;		// 1899A  beq.s   $189AE
	rom[0x18D50/2]	=	0x6012;		// 18D50  beq.s   $18D64
	rom[0x230E8/2]	=	0x6012;		// 230E8  beq.s   $230FC
	rom[0x2346C/2]	=	0x6012;		// 2346C  beq.s   $23480
	rom[0x23822/2]	=	0x6012;		// 23822  beq.s   $23822
	rom[0x2BCFA/2]	=	0x6012;		// 2BCFA  beq.s   $2BD0E
	rom[0x2C07E/2]	=	0x6012;		// 2C07E  beq.s   $2C092
	rom[0x2C434/2]	=	0x6012;		// 2C434  beq.s   $2C448

	return 0;
}

static void draw_layer_linescroll()
{
	unsigned short *vram = (unsigned short*)DrvVidRAM1;
	unsigned short *sram = (unsigned short*)DrvScrRAM;
	unsigned short *dest = draw_bitmap[1];

	for (int y = 0; y < nScreenHeight; y++, dest += nScreenWidth)
	{
		int yscroll = (scrolly[1] + y) & 0x1ff;

		for (int x = 0; x < nScreenWidth + 16; x+=16) {

			int xscroll = (scrollx[1] + sram[y * 2 + 1] + x) & 0x7ff;

			int ofst = (((yscroll >> 4) << 7) | (xscroll >> 4)) << 1;

			int attr = vram[ofst];
			int code = vram[ofst + 1] & 0x7fff;
			int pri  = (attr & 0x0e00) >> 9;

			int flipx = attr & 0x8000;
			int flipy = attr & 0x4000;

			int color = ((attr & 0x0070) << 2)+0x600+pri*0x800;

			{
				unsigned char *gfx = DrvGfxROM1 + (code << 8);

				if (flipy) gfx += ((yscroll & 0x0f)^0xf) << 4;
				else	   gfx += ((yscroll & 0x0f)    ) << 4;

				if (DrvTransTable[4][(gfx-DrvGfxROM1)/16]) continue;

				if (flipx) flipx = 0x0f;

				for (int xx = 0, sx = x - (xscroll & 0x0f); xx < 16; xx++, sx++) {
					if (sx < 0 || sx >= nScreenWidth) continue;

					int pxl = gfx[xx^flipx];

					if (pxl == 0x3f) continue;

					dest[sx] = pxl | color;
				}
			}
		}
	}
}

static void draw_layer(unsigned char *ram, unsigned char *gfx, int size, int wide, int high, int color_offset, int color_and, int layer)
{
	int twidth  = wide * size;
	int theight = high * size;

	int code_and = (size == 32) ? 0x1fff : ((size == 16) ? 0x7fff : 0xffff);

	unsigned short *vram = (unsigned short*)ram;

	for (int sy = 0, offs = 0; sy < high * size; sy+=size)
	{
		for (int sx = 0; sx < wide * size; sx+=size, offs++)
		{
			int code = vram[offs * 2 + 1] & code_and;
			if (DrvTransTable[layer][code]) continue; // transparent

			int attr = vram[offs * 2 + 0];

			int prio =  (attr >> 9) & 0x07;
			int color= ((attr >> 4) & color_and) + color_offset / 0x40 + prio * 0x800 / 0x40;

			int flipx= attr & 0x8000;
			int flipy= attr & 0x4000;

			int xx = sx - (scrollx[layer] % twidth);
			if (xx < (0 - (size - 1))) xx += twidth;

			int yy = sy - (scrolly[layer] % theight);
			if (yy < (0 - (size - 1))) yy += theight;

			if (xx >= nScreenWidth || yy >= nScreenHeight) continue;

			if (flipy) {
				if (flipx) {
					RenderCustomTile_Mask_FlipXY_Clip(draw_bitmap[layer], size, size, code, xx, yy, color, 6, 0x3f, 0, gfx);
				} else {
					RenderCustomTile_Mask_FlipY_Clip(draw_bitmap[layer], size, size, code, xx, yy, color, 6, 0x3f, 0, gfx);
				}
			} else {
				if (flipx) {
					RenderCustomTile_Mask_FlipX_Clip(draw_bitmap[layer], size, size, code, xx, yy, color, 6, 0x3f, 0, gfx);
				} else {
					RenderCustomTile_Mask_Clip(draw_bitmap[layer], size, size, code, xx, yy, color, 6, 0x3f, 0, gfx);
				}
			}
		}
	}
}

static void draw_sprites()
{
	unsigned short *sprite = (unsigned short*)DrvSprRAM;

	for (int offs = 0; offs < 0x800 / 2; offs += 4)
	{
		int x0, x1, dx, y0, y1, dy;

		int attr = sprite[offs + 1];
		if (attr & 0x0100) break;

		int sy    = sprite[offs + 0];
		int code  = sprite[offs + 2];
		int sx    = sprite[offs + 3];
		int flipx = (attr & 0x8000);
		int flipy = (attr & 0x4000);
		int pri   = (attr & 0x0e00) >> 9;
		int color = (attr & 0x00f0) >> 4;
		int nx    = (attr & 0x000f) + 1;
		int ny    = ((sy & 0xf000) >> 12) + 1;

		if (flipx) { x0 = nx - 1;	x1 = -1;	dx = -1; }
		else	   { x0 = 0;		x1 = nx;	dx = +1; }

		if (flipy) { y0 = ny - 1;	y1 = -1;	dy = -1; }
		else	   { y0 = 0;		y1 = ny;	dy = +1; }

		sx	-= 0x18;
		sy	 = (sy & 0x7ff) - (sy & 0x800);

		for (int y = y0; y != y1; y += dy, code += 1 - 0x10 * nx)
		{
			for (int x = x0; x != x1; x += dx, code += 0x10)
			{
				if (DrvTransTable[3][code]) continue;

				if (flipy) {
					if (flipx) {
						RenderCustomTile_Mask_FlipXY_Clip(draw_bitmap[4], 16, 16, code, sx + x * 0x10, sy + y * 0x10, color + pri * 0x800/0x40, 6, 0x3f, 0, DrvGfxROM3);
					} else {
						RenderCustomTile_Mask_FlipY_Clip(draw_bitmap[4], 16, 16, code, sx + x * 0x10, sy + y * 0x10, color + pri * 0x800/0x40, 6, 0x3f, 0, DrvGfxROM3);
					}
				} else {
					if (flipx) {
						RenderCustomTile_Mask_FlipX_Clip(draw_bitmap[4], 16, 16, code, sx + x * 0x10, sy + y * 0x10, color + pri * 0x800/0x40, 6, 0x3f, 0, DrvGfxROM3);
					} else {
						RenderCustomTile_Mask_Clip(draw_bitmap[4], 16, 16, code, sx + x * 0x10, sy + y * 0x10, color + pri * 0x800/0x40, 6, 0x3f, 0, DrvGfxROM3);
					}
				}
			}
		}
	}
}

static void copy_layers()
{
	const unsigned char pri2layer[8] = { 0, 0, 0, 4, 3, 0, 1, 2 };
	const unsigned char layerpri[5] = { 0, 1, 2, 4, 3 };

	unsigned short *dest	  = pTransDraw;
	unsigned short *source[5] = { draw_bitmap[0], draw_bitmap[1], draw_bitmap[2], draw_bitmap[3], draw_bitmap[4] };
	unsigned short *pri_ram = (unsigned short*)DrvPriRAM;

	for (int y = 0; y < nScreenHeight; y++)
	{
		for (int x = 0; x < nScreenWidth; x++)
		{
			unsigned short pens[5];
			int pri_addr = 0;

			for (int l = 0; l < 5; l++) {
				pens[l] = *source[l]++;
				if (pens[l] == 0x3f) pri_addr |= 1 << layerpri[l];
			}

			pri_addr |= (pens[1] >> 11) << 5;
			pri_addr |= (pens[4] >> 11) << 8;
			pri_addr |= (pens[0] >> 11) << 11;
			pri_addr |= (pens[3] >> 11) << 14;

			*dest++ = pens[pri2layer[pri_ram[pri_addr & 0x7fff] & 7]] & 0x7ff;
		}
	}
}

static void DrvPaletteRecalc()
{
	unsigned short *p = (unsigned short*)DrvPalRAM;

	for (int i = 0; i < 0x1000 / 2; i++) {
		int r = (p[i] >> 0) & 0x0f;
		int g = (p[i] >> 4) & 0x0f;
		int b = (p[i] >> 8) & 0x0f;

		r |= r << 4;
		g |= g << 4;
		b |= b << 4;

		DrvPalette[i] = BurnHighCol(r, g, b, 0);
	}

	DrvPalette[0x0800] = BurnHighCol(0xff, 0xff, 0xff, 0); // white
}

static int DrvDraw()
{
	DrvPaletteRecalc();

	if (lordgun_whitescreen) {
		for (int o = 0; o < nScreenWidth * nScreenHeight; o++) {
			pTransDraw[o] = 0x800; // white!!
		}

		BurnTransferCopy(DrvPalette);
		return 0;
	}

	for (int o = 0; o < nScreenWidth * nScreenHeight; o++) {
		draw_bitmap[0][o] = draw_bitmap[1][o] = draw_bitmap[2][o] = draw_bitmap[3][o] = draw_bitmap[4][o] = 0x003f;
	}

	int line_enable = 0;
	{
		unsigned short *rs = (unsigned short*)DrvScrRAM;
		for (int i = 0; i < nScreenHeight * 2; i+=2) {
			if ((rs[i + 1] & 0x7ff) != (rs[1] & 0x7ff)) {
				line_enable = 1;
				break;
			}
		}
	}

	draw_layer(DrvVidRAM0, DrvGfxROM0,  8, 0x100, 0x040, 0x500, 0x03, 0);
	if (line_enable == 0) {
		draw_layer(DrvVidRAM1, DrvGfxROM1, 16, 0x080, 0x020, 0x600, 0x07, 1);
	} else {
		draw_layer_linescroll();
	}
	draw_layer(DrvVidRAM2, DrvGfxROM2, 32, 0x040, 0x010, 0x700, 0x03, 2);
	draw_layer(DrvVidRAM3, DrvGfxROM0,  8, 0x040, 0x020, 0x400, 0x0f, 3);
	draw_sprites();

	copy_layers();

	BurnTransferCopy(DrvPalette);

	return 0;
}

static int lordgunDraw()
{
	DrvDraw();

	for (int i = 0; i < BurnDrvGetMaxPlayers(); i++) {
		BurnGunDrawTarget(i, BurnGunX[i] >> 8, BurnGunY[i] >> 8);
	}

	return 0;
}

static inline void compile_inputs()
{
	memset (DrvInputs, 0xff, 5 * sizeof(short));

	for (int i = 0; i < 16; i++) {
		DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
		DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
		DrvInputs[2] ^= (DrvJoy3[i] & 1) << i;
		DrvInputs[3] ^= (DrvJoy4[i] & 1) << i;
		DrvInputs[4] ^= (DrvJoy5[i] & 1) << i;
	}

	BurnGunMakeInputs(0, (short)DrvAxis[0], (short)DrvAxis[1]);
	BurnGunMakeInputs(1, (short)DrvAxis[2], (short)DrvAxis[3]);
		
	float x0 = ((float)((BurnGunX[0] >> 8) + 8)) / 448 * 412;
	float y0 = ((float)((BurnGunY[0] >> 8) + 8)) / 224 * 224;
	float x1 = ((float)((BurnGunX[1] >> 8) + 8)) / 448 * 412;
	float y1 = ((float)((BurnGunY[1] >> 8) + 8)) / 224 * 224;
	DrvAnalogInput[0] = (unsigned short)x0 + 0x3c;
	DrvAnalogInput[2] = (unsigned char)y0 + 0;
	DrvAnalogInput[1] = (unsigned short)x1 + 0x3c;
	DrvAnalogInput[3] = (unsigned char)y1 + 0;
}

static int lordgunFrame()
{
	if (DrvReset) {
		DrvDoReset();
	}

	compile_inputs();

	SekNewFrame();
	ZetNewFrame();

	int nTotalCycles[2] =  { 10000000 / 60, 5000000 / 60 };

	SekOpen(0);
	ZetOpen(0);

	SekRun(nTotalCycles[0]);
	SekSetIRQLine(4, SEK_IRQSTATUS_AUTO);

	BurnTimerEndFrameYM3812(nTotalCycles[1]);

	if (pBurnSoundOut) {
		BurnYM3812Update(pBurnSoundOut, nBurnSoundLen);
		MSM6295Render(0, pBurnSoundOut, nBurnSoundLen);
	}

	ZetClose();
	SekClose();

	if (pBurnDraw) {
		lordgunDraw();
	}

	return 0;
}

static int alienchaFrame()
{
	if (DrvReset) {
		DrvDoReset();
	}

	compile_inputs();

	SekNewFrame();
	ZetNewFrame();

	int nCyclesTotal[2] =  { 10000000 / 60, 5000000 / 60 };

	SekOpen(0);
	ZetOpen(0);

	for (int i = 0; i < 100; i++) {
		SekRun(nCyclesTotal[0] / 100);

		BurnTimerUpdate(nCyclesTotal[1] / 100);
	}

	SekSetIRQLine(4, SEK_IRQSTATUS_AUTO);

	BurnTimerEndFrame(nCyclesTotal[1]);

	if (pBurnSoundOut) {
		BurnYMF278BUpdate(nBurnSoundLen);
		MSM6295Render(0, pBurnSoundOut, nBurnSoundLen);
		MSM6295Render(1, pBurnSoundOut, nBurnSoundLen);
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

	if (pnMin) {
		*pnMin = 0x029708;
	}

	if (nAction & ACB_VOLATILE) {

		memset(&ba, 0, sizeof(ba));
    		ba.Data		= AllRam;
		ba.nLen		= RamEnd - AllRam;
		ba.szName	= "All RAM";
		BurnAcb(&ba);

		SekScan(nAction);
		ZetScan(nAction);

		BurnYMF278BScan(nAction, pnMin);

		BurnGunScan();
		EEPROMScan(nAction, pnMin);

		SCAN_VAR(aliencha_dip_sel);
		SCAN_VAR(lordgun_whitescreen);

		SCAN_VAR(lordgun_gun_hw_x[0]);
		SCAN_VAR(lordgun_gun_hw_y[0]);
		SCAN_VAR(lordgun_gun_hw_x[1]);
		SCAN_VAR(lordgun_gun_hw_y[1]);
	}

	if (nAction & ACB_WRITE) {
		int bank = *okibank;
		*okibank = -1;
		set_oki_bank(bank);
	}

	return 0;
}


// Lord of Gun (USA)

static struct BurnRomInfo lordgunRomDesc[] = {
	{ "lordgun.10",		0x080000, 0xacda77ef, 1 | BRF_PRG | BRF_ESS }, //  0 68k code
	{ "lordgun.4",		0x080000, 0xa1a61254, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "lordgun.90",		0x010000, 0xd59b5e28, 2 | BRF_PRG | BRF_ESS }, //  2 z80 code

	{ "igst001.108",	0x100000, 0x36dd96f3, 3 | BRF_GRA },           //  3 8x8 tiles
	{ "igst002.114",	0x100000, 0x816a7665, 3 | BRF_GRA },           //  4
	{ "igst003.119",	0x100000, 0xcbfee543, 3 | BRF_GRA },           //  5

	{ "igsb001.82",		0x200000, 0x3096de1c, 4 | BRF_GRA },           //  6 16x16 and 32x32 tiles
	{ "igsb002.91",		0x200000, 0x2234531e, 4 | BRF_GRA },           //  7
	{ "igsb003.97",		0x200000, 0x6cbf21ac, 4 | BRF_GRA },           //  8

	{ "igsa001.14",		0x200000, 0x400abe33, 5 | BRF_GRA },           //  9 Sprites
	{ "igsa004.13",		0x200000, 0x52687264, 5 | BRF_GRA },           // 10
	{ "igsa002.9",		0x200000, 0xa4810e38, 5 | BRF_GRA },           // 11
	{ "igsa005.8",		0x200000, 0xe32e79e3, 5 | BRF_GRA },           // 12
	{ "igsa003.3",		0x200000, 0x649e48d9, 5 | BRF_GRA },           // 13
	{ "igsa006.2",		0x200000, 0x39288eb6, 5 | BRF_GRA },           // 14

	{ "lordgun.100",	0x080000, 0xb4e0fa07, 6 | BRF_SND },           // 15 OKI #0 Samples
};

STD_ROM_PICK(lordgun)
STD_ROM_FN(lordgun)

static int lordgunInit()
{
	return DrvInit(lordgunLoadRoms, 1);
}

struct BurnDriver BurnDrvLordgun = {
	"lordgun", NULL, NULL, NULL, "1994",
	"Lord of Gun (USA)\0", "Imperfect graphics", "IGS", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_SHOOT, 0,
	NULL, lordgunRomInfo, lordgunRomName, NULL, NULL, LordgunInputInfo, LordgunDIPInfo,
	lordgunInit, DrvExit, lordgunFrame, lordgunDraw, DrvScan, &DrvRecalc, 0x800,
	448, 224, 4, 3
};

/*	"1945kiii", NULL, NULL, NULL, "2000",
	"1945k III\0", NULL, "Oriental", "misc",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_16BIT_ONLY, 2, HARDWARE_MISC_POST90S, GBF_VERSHOOT, 0,
	NULL, _1945kiiiRomInfo, _1945kiiiRomName, NULL, NULL, _1945kiiiInputInfo, _1945kiiiDIPInfo,
	DrvInit, DrvExit, DrvFrame, NULL, DrvScan, &bRecalcPalette, 0x1000,
	224, 320, 3, 4*/


// Alien Challenge (World)

static struct BurnRomInfo alienchaRomDesc[] = {
	{ "igsc0102.u81",	0x200000, 0xe3432be3, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code

	{ "hfh_s.u86",		0x010000, 0x5728a9ed, 2 | BRF_PRG | BRF_ESS }, //  1 z80 code

	{ "igst0101.u9",	0x100000, 0x2ce12d7b, 3 | BRF_GRA },           //  2 8x8 tiles
	{ "igst0102.u10",	0x100000, 0x542a76a0, 3 | BRF_GRA },           //  3
	{ "igst0103.u11",	0x100000, 0xadf5698a, 3 | BRF_GRA },           //  4

	{ "igsb0101.u8",	0x200000, 0x5c995f7e, 4 | BRF_GRA },           //  5 16x16 and 32x32 tiles
	{ "igsb0102.u7",	0x200000, 0xa2ae9baf, 4 | BRF_GRA },           //  6
	{ "igsb0103.u6",	0x200000, 0x11b927af, 4 | BRF_GRA },           //  7

	{ "igsa0101.u3",	0x400000, 0x374d07c4, 5 | BRF_GRA },           //  8 Sprites
	{ "igsa0102.u2",	0x400000, 0xdbeee7ac, 5 | BRF_GRA },           //  9
	{ "igsa0103.u1",	0x400000, 0xe5f19041, 5 | BRF_GRA },           // 10

	{ "hfh_g.u65",		0x040000, 0xec469b57, 6 | BRF_SND },           // 11 OKI #0 Samples

	{ "hfh_g.u66",		0x040000, 0x7cfcd98e, 7 | BRF_SND },           // 12 OKI #1 Samples

	{ "yrw801-m",		0x200000, 0x2a9d8d43, 8 | BRF_SND },           // 13 YMF278b Samples
};

STD_ROM_PICK(aliencha)
STD_ROM_FN(aliencha)

static int alienchaInit()
{
	return DrvInit(alienchaLoadRoms, 0);
}

struct BurnDriver BurnDrvAliencha = {
	"aliencha", NULL, NULL, NULL, "1994",
	"Alien Challenge (World)\0", NULL, "IGS", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_VSFIGHT, 0,
	NULL, alienchaRomInfo, alienchaRomName, NULL, NULL, AlienchaInputInfo, AlienchaDIPInfo,
	alienchaInit, DrvExit, alienchaFrame, DrvDraw, DrvScan, &DrvRecalc, 0x800,
	448, 224, 4, 3
};


// Alien Challenge (China)

static struct BurnRomInfo alienchacRomDesc[] = {
	{ "igsc0102.u81",	0x200000, 0xe3432be3, 1 | BRF_PRG | BRF_ESS }, //  0 68k code
	{ "hfh_p.u80",		0x080000, 0x5175ebdc, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "hfh_p.u79",		0x080000, 0x42ad978c, 1 | BRF_PRG | BRF_ESS }, //  2

	{ "hfh_s.u86",		0x010000, 0x5728a9ed, 2 | BRF_PRG | BRF_ESS }, //  3 z80 code

	{ "igst0101.u9",	0x100000, 0x2ce12d7b, 3 | BRF_GRA },           //  4 8x8 tiles
	{ "igst0102.u10",	0x100000, 0x542a76a0, 3 | BRF_GRA },           //  5
	{ "igst0103.u11",	0x100000, 0xadf5698a, 3 | BRF_GRA },           //  6

	{ "igsb0101.u8",	0x200000, 0x5c995f7e, 4 | BRF_GRA },           //  7 16x16 and 32x32 tiles
	{ "igsb0102.u7",	0x200000, 0xa2ae9baf, 4 | BRF_GRA },           //  8
	{ "igsb0103.u6",	0x200000, 0x11b927af, 4 | BRF_GRA },           //  9

	{ "igsa0101.u3",	0x400000, 0x374d07c4, 5 | BRF_GRA },           // 10 Sprites
	{ "igsa0102.u2",	0x400000, 0xdbeee7ac, 5 | BRF_GRA },           // 11
	{ "igsa0103.u1",	0x400000, 0xe5f19041, 5 | BRF_GRA },           // 12

	{ "hfh_g.u65",		0x040000, 0xec469b57, 6 | BRF_SND },           // 11 OKI #0 Samples

	{ "hfh_g.u66",		0x040000, 0x7cfcd98e, 7 | BRF_SND },           // 12 OKI #1 Samples

	{ "yrw801-m",		0x200000, 0x2a9d8d43, 8 | BRF_SND },           // 13 YMF278b Samples
};

STD_ROM_PICK(alienchac)
STD_ROM_FN(alienchac)

static int alienchacInit()
{
	return DrvInit(alienchacLoadRoms, 0);
}

struct BurnDriver BurnDrvAlienchac = {
	"alienchac", "aliencha", NULL, NULL, "1994",
	"Alien Challenge (China)\0", NULL, "IGS", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_POST90S, GBF_VSFIGHT, 0,
	NULL, alienchacRomInfo, alienchacRomName, NULL, NULL, AlienchaInputInfo, AlienchacDIPInfo,
	alienchacInit, DrvExit, alienchaFrame, DrvDraw, DrvScan, &DrvRecalc, 0x800,
	448, 224, 4, 3
};
