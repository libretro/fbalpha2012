// FB Alpha Armed Formation driver module
// Based on MAME driver by Carlos A. Lozano, Phil Stroffolino, and Takahiro Nogi

#include "tiles_generic.h"
#include "burn_ym3812.h"
#include "dac.h"

static unsigned char *AllMem;
static unsigned char *MemEnd;
static unsigned char *AllRam;
static unsigned char *RamEnd;
static unsigned char *Drv68KROM;
static unsigned char *Drv68KRAM0;
static unsigned char *Drv68KRAM1;
static unsigned char *Drv68KRAM2;
static unsigned char *DrvZ80ROM;
static unsigned char *DrvZ80RAM;
static unsigned char *DrvGfxROM0;
static unsigned char *DrvGfxROM1;
static unsigned char *DrvGfxROM2;
static unsigned char *DrvGfxROM3;
static unsigned char *DrvPalRAM;
static unsigned char *DrvSprRAM;
static unsigned char *DrvSprBuf;
static unsigned char *DrvBgRAM;
static unsigned char *DrvFgRAM;
static unsigned char *DrvTxRAM;
static unsigned int  *DrvPalette;

static unsigned short*DrvMcuCmd;
static unsigned short*DrvScroll;
static unsigned char *DrvVidRegs;
static unsigned char *soundlatch;
static unsigned char *flipscreen;

static unsigned char DrvRecalc;

static unsigned char DrvJoy1[16];
static unsigned char DrvJoy2[16];
static unsigned char DrvDips[2];
static unsigned short DrvInputs[4];
static unsigned char DrvReset;

static int scroll_type;
static int sprite_offy;
static int yoffset;
static int xoffset;
static int irqline;

static struct BurnInputInfo ArmedfInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy1 + 10,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 8,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"	},
	{"P1 Button 3",		BIT_DIGITAL,	DrvJoy1 + 6,	"p1 fire 3"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy1 + 11,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy1 + 9,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"	},
	{"P2 Button 3",		BIT_DIGITAL,	DrvJoy2 + 6,	"p2 fire 3"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy2 + 8,	"service"	},
	{"Tilt",		BIT_DIGITAL,	DrvJoy2 + 10,	"tilt"		},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
};

STDINPUTINFO(Armedf)

static struct BurnInputInfo Cclimbr2InputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy1 + 10,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 8,	"p1 start"	},
	{"P1 Up 1",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 up"		},
	{"P1 Down 1",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 down"	},
	{"P1 Left 1",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 left"	},
	{"P1 Right 1",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 right"	},
	{"P1 Up 2",		BIT_DIGITAL,	DrvJoy1 + 4,	"p3 up"		},
	{"P1 Down 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p3 down"	},
	{"P1 Left 2",		BIT_DIGITAL,	DrvJoy1 + 6,	"p3 left"	},
	{"P1 Right 2",		BIT_DIGITAL,	DrvJoy1 + 7,	"p3 right"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy1 + 11,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy1 + 9,	"p2 start"	},
	{"P2 Up 1",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 up"		},
	{"P2 Down 1",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 down"	},
	{"P2 Left 1",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 left"	},
	{"P2 Right 1",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 right"	},
	{"P2 Up 2",		BIT_DIGITAL,	DrvJoy2 + 4,	"p4 up"		},
	{"P2 Down 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p4 down"	},
	{"P2 Left 2",		BIT_DIGITAL,	DrvJoy2 + 6,	"p4 left"	},
	{"P2 Right 2",		BIT_DIGITAL,	DrvJoy2 + 7,	"p4 right"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy2 + 8,	"service"	},
	{"Tilt",		BIT_DIGITAL,	DrvJoy2 + 10,	"tilt"		},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
};

STDINPUTINFO(Cclimbr2)

static struct BurnDIPInfo ArmedfDIPList[]=
{
	// Default Values
	{0x15, 0xff, 0xff, 0xdf, NULL					},
	{0x16, 0xff, 0xff, 0xcf, NULL					},

	{0   , 0xfe, 0   ,    4, "Lives"				},
	{0x15, 0x01, 0x03, 0x03, "3"					},
	{0x15, 0x01, 0x03, 0x02, "4"					},
	{0x15, 0x01, 0x03, 0x01, "5"					},
	{0x15, 0x01, 0x03, 0x00, "6"					},

	{0   , 0xfe, 0   ,    2, "1st Bonus Life"			},
	{0x15, 0x01, 0x04, 0x04, "20k"					},
	{0x15, 0x01, 0x04, 0x00, "40k"					},

	{0   , 0xfe, 0   ,    2, "2nd Bonus Life"			},
	{0x15, 0x01, 0x08, 0x08, "60k"					},
	{0x15, 0x01, 0x08, 0x00, "80k"					},

	{0   , 0xfe, 0   ,    4, "Bonus Life"				},
	{0x15, 0x01, 0x0c, 0x0c, "20k then every 60k"			},
	{0x15, 0x01, 0x0c, 0x04, "20k then every 80k"			},
	{0x15, 0x01, 0x0c, 0x08, "40k then every 60k"			},
	{0x15, 0x01, 0x0c, 0x00, "40k then every 80k"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"				},
	{0x15, 0x01, 0x10, 0x00, "Off"					},
	{0x15, 0x01, 0x10, 0x10, "On"					},

	{0   , 0xfe, 0   ,    2, "Cabinet"				},
	{0x15, 0x01, 0x20, 0x00, "Upright"				},
	{0x15, 0x01, 0x20, 0x20, "Cocktail"				},

	{0   , 0xfe, 0   ,    4, "Difficulty"				},
	{0x15, 0x01, 0xc0, 0xc0, "Easy"					},
	{0x15, 0x01, 0xc0, 0x80, "Normal"				},
	{0x15, 0x01, 0xc0, 0x40, "Hard"					},
	{0x15, 0x01, 0xc0, 0x00, "Hardest"				},

	{0   , 0xfe, 0   ,    4, "Coin A"				},
	{0x16, 0x01, 0x03, 0x01, "2 Coins 1 Credits"			},
	{0x16, 0x01, 0x03, 0x03, "1 Coin  1 Credits"			},
	{0x16, 0x01, 0x03, 0x02, "1 Coin  2 Credits"			},
	{0x16, 0x01, 0x03, 0x00, "Free Play"				},

	{0   , 0xfe, 0   ,    4, "Coin B"				},
	{0x16, 0x01, 0x0c, 0x04, "2 Coins 1 Credits"			},
	{0x16, 0x01, 0x0c, 0x0c, "1 Coin  1 Credits"			},
	{0x16, 0x01, 0x0c, 0x00, "2 Coins 3 Credits"			},
	{0x16, 0x01, 0x0c, 0x08, "1 Coin  2 Credits"			},

	{0   , 0xfe, 0   ,    4, "Allow Continue"			},
	{0x16, 0x01, 0x30, 0x30, "No"					},
	{0x16, 0x01, 0x30, 0x20, "3 Times"				},
	{0x16, 0x01, 0x30, 0x10, "5 Times"				},
	{0x16, 0x01, 0x30, 0x00, "Yes"					},

	{0   , 0xfe, 0   ,    2, "Flip Screen"				},
	{0x16, 0x01, 0x40, 0x40, "Off"					},
	{0x16, 0x01, 0x40, 0x00, "On"					},
};

STDDIPINFO(Armedf)

static struct BurnDIPInfo KozureDIPList[]=
{
	{0x15, 0xff, 0xff, 0xcf, NULL					},
	{0x16, 0xff, 0xff, 0xcf, NULL					},

	{0   , 0xfe, 0   ,    4, "Lives"				},
	{0x15, 0x01, 0x03, 0x03, "3"					},
	{0x15, 0x01, 0x03, 0x02, "4"					},
	{0x15, 0x01, 0x03, 0x01, "5"					},
	{0x15, 0x01, 0x03, 0x00, "6"					},

	{0   , 0xfe, 0   ,    2, "1st Bonus Life"			},
	{0x15, 0x01, 0x04, 0x04, "None"					},
	{0x15, 0x01, 0x04, 0x00, "50k"					},

	{0   , 0xfe, 0   ,    2, "2nd Bonus Life"			},
	{0x15, 0x01, 0x08, 0x08, "60k"					},
	{0x15, 0x01, 0x08, 0x00, "90k"					},

	{0   , 0xfe, 0   ,    4, "Bonus Life"				},
	{0x15, 0x01, 0x0c, 0x08, "50k then every 60k"			},
	{0x15, 0x01, 0x0c, 0x00, "50k then every 90k"			},
	{0x15, 0x01, 0x0c, 0x0c, "Every 60k"				},
	{0x15, 0x01, 0x0c, 0x04, "Every 90k"				},

	{0   , 0xfe, 0   ,    2, "Difficulty"				},
	{0x15, 0x01, 0x40, 0x40, "Easy"					},
	{0x15, 0x01, 0x40, 0x00, "Hard"					},

	{0   , 0xfe, 0   ,    0, "Coin A"				},
	{0x16, 0x01, 0x03, 0x01, "2 Coins 1 Credits"			},
	{0x16, 0x01, 0x03, 0x03, "1 Coin  1 Credits"			},
	{0x16, 0x01, 0x03, 0x02, "1 Coin  2 Credits"			},
	{0x16, 0x01, 0x03, 0x00, "Free Play"				},

	{0   , 0xfe, 0   ,    4, "Coin B"				},
	{0x16, 0x01, 0x0c, 0x00, "3 Coins 1 Credits"			},
	{0x16, 0x01, 0x0c, 0x04, "2 Coins 3 Credits"			},
	{0x16, 0x01, 0x0c, 0x0c, "1 Coin  3 Credits"			},
	{0x16, 0x01, 0x0c, 0x08, "1 Coin  6 Credits"			},

	{0   , 0xfe, 0   ,    4, "Allow Continue"			},
	{0x16, 0x01, 0x30, 0x30, "No"					},
	{0x16, 0x01, 0x30, 0x20, "3 Times"				},
	{0x16, 0x01, 0x30, 0x10, "5 Times"				},
	{0x16, 0x01, 0x30, 0x00, "Yes"					},

	{0   , 0xfe, 0   ,    4, "Allow Continue"			},
	{0x16, 0x01, 0x40, 0x00, "No"					},
	{0x16, 0x01, 0x40, 0x40, "Yes"					},
};

STDDIPINFO(Kozure)

static struct BurnDIPInfo Cclimbr2DIPList[]=
{
	{0x17, 0xff, 0xff, 0xcf, NULL					},
	{0x18, 0xff, 0xff, 0xff, NULL					},

	{0   , 0xfe, 0   ,    4, "Lives"				},
	{0x17, 0x01, 0x03, 0x03, "3"					},
	{0x17, 0x01, 0x03, 0x02, "4"					},
	{0x17, 0x01, 0x03, 0x01, "5"					},
	{0x17, 0x01, 0x03, 0x00, "6"					},

	{0   , 0xfe, 0   ,    2, "1st Bonus Life"			},
	{0x17, 0x01, 0x04, 0x04, "30k"					},
	{0x17, 0x01, 0x04, 0x00, "60k"					},

	{0   , 0xfe, 0   ,    2, "2nd Bonus Life"			},
	{0x17, 0x01, 0x08, 0x08, "70k"					},
	{0x17, 0x01, 0x08, 0x00, "None"					},

	{0   , 0xfe, 0   ,    4, "Bonus Life"				},
	{0x17, 0x01, 0x0c, 0x0c, "30K and 100k"				},
	{0x17, 0x01, 0x0c, 0x08, "60k and 130k"				},
	{0x17, 0x01, 0x0c, 0x04, "30k only"				},
	{0x17, 0x01, 0x0c, 0x00, "60k only"				},

	{0   , 0xfe, 0   ,    2, "Difficulty"				},
	{0x17, 0x01, 0x40, 0x40, "Easy"					},
	{0x17, 0x01, 0x40, 0x00, "Normal"				},

	{0   , 0xfe, 0   ,    0, "Coin A"				},
	{0x18, 0x01, 0x03, 0x01, "2 Coins 1 Credits"			},
	{0x18, 0x01, 0x03, 0x03, "1 Coin  1 Credits"			},
	{0x18, 0x01, 0x03, 0x02, "1 Coin  2 Credits"			},
	{0x18, 0x01, 0x03, 0x00, "Free Play"				},

	{0   , 0xfe, 0   ,    4, "Coin B"				},
	{0x18, 0x01, 0x0c, 0x04, "2 Coins 1 Credits"			},
	{0x18, 0x01, 0x0c, 0x0c, "1 Coin  1 Credits"			},
	{0x18, 0x01, 0x0c, 0x00, "2 Coins 3 Credits"			},
	{0x18, 0x01, 0x0c, 0x08, "1 Coin  2 Credits"			},

	{0   , 0xfe, 0   ,    4, "Allow Continue"			},
	{0x18, 0x01, 0x10, 0x00, "No"					},
	{0x18, 0x01, 0x10, 0x10, "3 Times"				},

	{0   , 0xfe, 0   ,    2, "Flip Screen"				},
	{0x18, 0x01, 0x20, 0x20, "Off"					},
	{0x18, 0x01, 0x20, 0x00, "On"					},

	{0   , 0xfe, 0   ,    2, "Partial Invulnerability (Cheat)"	},
	{0x18, 0x01, 0x40, 0x40, "Off"					},
	{0x18, 0x01, 0x40, 0x00, "On"					},
};

STDDIPINFO(Cclimbr2)

static struct BurnDIPInfo LegionDIPList[]=
{
	{0x15, 0xff, 0xff, 0xf7, NULL					},
	{0x16, 0xff, 0xff, 0xff, NULL					},

	{0   , 0xfe, 0   ,    4, "Lives"				},
	{0x15, 0x01, 0x03, 0x03, "3"					},
	{0x15, 0x01, 0x03, 0x02, "4"					},
	{0x15, 0x01, 0x03, 0x01, "5"					},
	{0x15, 0x01, 0x03, 0x00, "6"					},

	{0   , 0xfe, 0   ,    2, "Bonus Life"				},
	{0x15, 0x01, 0x04, 0x04, "30k Then Every 100k"			},
	{0x15, 0x01, 0x04, 0x00, "50k Only"				},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"				},
	{0x15, 0x01, 0x08, 0x08, "Off"					},
	{0x15, 0x01, 0x08, 0x00, "On"					},

	{0   , 0xfe, 0   ,    2, "Flip Screen"				},
	{0x15, 0x01, 0x10, 0x10, "Off"					},
	{0x15, 0x01, 0x10, 0x00, "On"					},

	{0   , 0xfe, 0   ,    0, "Allow Invulnerability (Cheat)"	},
	{0x15, 0x01, 0x80, 0x80, "No"					},
	{0x15, 0x01, 0x80, 0x00, "Yes"					},

	{0   , 0xfe, 0   ,    0, "Coin A"				},
	{0x16, 0x01, 0x03, 0x01, "2 Coins 1 Credits"			},
	{0x16, 0x01, 0x03, 0x03, "1 Coin  1 Credits"			},
	{0x16, 0x01, 0x03, 0x02, "1 Coin  2 Credits"			},
	{0x16, 0x01, 0x03, 0x00, "Free Play"				},

	{0   , 0xfe, 0   ,    2, "Coin B"				},
	{0x16, 0x01, 0x0c, 0x04, "2 Coins 1 Credits"			},
	{0x16, 0x01, 0x0c, 0x0c, "1 Coin  1 Credits"			},
	{0x16, 0x01, 0x0c, 0x00, "2 Coins 3 Credits"			},
	{0x16, 0x01, 0x0c, 0x08, "1 Coin  2 Credits"			},

	{0   , 0xfe, 0   ,    4, "Coin Slots"				},
	{0x16, 0x01, 0x10, 0x10, "Common"				},
	{0x16, 0x01, 0x10, 0x00, "Individual"				},

	{0   , 0xfe, 0   ,    4, "Difficulty"				},
	{0x16, 0x01, 0x20, 0x20, "Easy"					},
	{0x16, 0x01, 0x20, 0x00, "Hard"					},

	{0   , 0xfe, 0   ,    2, "P1 Invulnerability (Cheat)"		},
	{0x16, 0x01, 0x40, 0x40, "Off"					},
	{0x16, 0x01, 0x40, 0x00, "On"					},

	{0   , 0xfe, 0   ,    2, "P2 Invulnerability (Cheat)"		},
	{0x16, 0x01, 0x80, 0x80, "Off"					},
	{0x16, 0x01, 0x80, 0x00, "On"					},
};

STDDIPINFO(Legion)

static struct BurnDIPInfo TerrafDIPList[]=
{
	{0x15, 0xff, 0xff, 0x0f, NULL					},
	{0x16, 0xff, 0xff, 0x3f, NULL					},

	{0   , 0xfe, 0   ,    4, "Lives"				},
	{0x15, 0x01, 0x03, 0x03, "3"					},
	{0x15, 0x01, 0x03, 0x02, "4"					},
	{0x15, 0x01, 0x03, 0x01, "5"					},
	{0x15, 0x01, 0x03, 0x00, "6"					},

	{0   , 0xfe, 0   ,    2, "1st Bonus Life"			},
	{0x15, 0x01, 0x04, 0x04, "20k"					},
	{0x15, 0x01, 0x04, 0x00, "50k"					},

	{0   , 0xfe, 0   ,    2, "2nd Bonus Life"			},
	{0x15, 0x01, 0x08, 0x08, "60k"					},
	{0x15, 0x01, 0x08, 0x00, "90k"					},

	{0   , 0xfe, 0   ,    4, "Bonus Life"				},
	{0x15, 0x01, 0x0c, 0x0c, "20k then every 60k"			},
	{0x15, 0x01, 0x0c, 0x04, "20k then every 90k"			},
	{0x15, 0x01, 0x0c, 0x08, "50k then every 60k"			},
	{0x15, 0x01, 0x0c, 0x00, "50k then every 90k"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"				},
	{0x15, 0x01, 0x10, 0x10, "Off"					},
	{0x15, 0x01, 0x10, 0x00, "On"					},

	{0   , 0xfe, 0   ,    4, "Coin A"				},
	{0x16, 0x01, 0x03, 0x01, "2 Coins 1 Credits"			},
	{0x16, 0x01, 0x03, 0x03, "1 Coin  1 Credits"			},
	{0x16, 0x01, 0x03, 0x02, "1 Coin  2 Credits"			},
	{0x16, 0x01, 0x03, 0x00, "Free Play"				},

	{0   , 0xfe, 0   ,    4, "Coin B"				},
	{0x16, 0x01, 0x0c, 0x04, "2 Coins 1 Credits"			},
	{0x16, 0x01, 0x0c, 0x0c, "1 Coin  1 Credits"			},
	{0x16, 0x01, 0x0c, 0x00, "2 Coins 3 Credits"			},
	{0x16, 0x01, 0x0c, 0x08, "1 Coin  2 Credits"			},

	{0   , 0xfe, 0   ,    0, "Flip Screen"				},
	{0x16, 0x01, 0x20, 0x20, "Off"					},
	{0x16, 0x01, 0x20, 0x00, "On"					},

	{0   , 0xfe, 0   ,    2, "Allow Continue"			},
	{0x16, 0x01, 0xc0, 0xc0, "No"					},
	{0x16, 0x01, 0xc0, 0x80, "Only 3 Times"				},
	{0x16, 0x01, 0xc0, 0x40, "Only 5 Times"				},
	{0x16, 0x01, 0xc0, 0x00, "Yes"					},
};

STDDIPINFO(Terraf)

void __fastcall armedf_write_word(unsigned int address, unsigned short data)
{
	switch (address)
	{
		case 0x06d000:
			*DrvVidRegs = data >> 8;
			*flipscreen = (data >> 12) & 1;
		return;

		case 0x06d002:
			DrvScroll[0] = data & 0x3ff;
		return;

		case 0x06d004:
			DrvScroll[1] = data & 0x1ff;
		return;

		case 0x06d006:
			DrvScroll[2] = data & 0x3ff;
		return;

		case 0x06d008:
			DrvScroll[3] = data & 0x1ff;
		return;

		case 0x06d00a:
			*soundlatch = ((data & 0x7f) << 1) | 1;
		return;
	}
}

void __fastcall cclimbr2_write_word(unsigned int address, unsigned short data)
{
	if (scroll_type == 6 && (address & 0xffffc0) == 0x040000) {
		DrvMcuCmd[(address >> 1) & 0x1f] = data;
		return;
	}

	switch (address)
	{
		case 0x7c000:
		{
			*DrvVidRegs = data >> 8;
			*flipscreen = (data >> 12) & 1;

			if (scroll_type == 2) {
				if (~data & 0x0080) {
					memset (DrvTxRAM, 0xff, 0x2000);
				}
			}

			if (scroll_type == 0 || scroll_type == 3 || scroll_type == 5 || scroll_type == 6) { // scroll6 - hack
				if (((data & 0x4100) == 0x4000 && scroll_type != 6) ||
					((data & 0x4100) == 0x0000 && scroll_type == 6)) {
					unsigned short *ram = (unsigned short*)DrvTxRAM;
					for (int i = 0x10; i < 0x1000; i++) {
						ram[i] = 0x0020;
					}
				}
			}
		}
		return;

		case 0x7c002:
			DrvScroll[0] = data & 0x3ff;
		return;

		case 0x7c004:
			DrvScroll[1] = data & 0x1ff;
		return;

		case 0x7c00a:
			*soundlatch = ((data & 0x7f) << 1) | 1;
		return;

		case 0x7c00e: 
			if (scroll_type == 0 || scroll_type == 3 || scroll_type == 5) {
				*DrvMcuCmd = data;
			}
		return;
	}
}

void __fastcall cclimbr2_write_byte(unsigned int address, unsigned char data)
{
	if (scroll_type != 0) return;

	switch (address)
	{
		case 0x7c006:
			DrvMcuCmd[11] = data;
			DrvMcuCmd[31] = 1;
		return;

		case 0x7c008:
			if (DrvMcuCmd[31]) {
				DrvMcuCmd[14] = data >> 4;
				DrvMcuCmd[12] = data;
			} else {
				DrvMcuCmd[13] = data;
			}
		return;

		case 0xc0000:
			DrvMcuCmd[31] = 0;
		return;
	}
}

unsigned short __fastcall cclimbr2_read_word(unsigned int address)
{
	switch (address)
	{
		case 0x78000:
			return DrvInputs[0];

		case 0x78002:
			return DrvInputs[1];

		case 0x78004:
			return DrvInputs[2];

		case 0x78006:
			return DrvInputs[3];
	}

	return 0;
}

void __fastcall armedf_write_port(unsigned short port, unsigned char data)
{
//	bprintf (PRINT_NORMAL, _T("%2.2x %2.2x\n"), port & 0xff, data);

	switch (port & 0xff)
	{
		case 0x00:
			BurnYM3812Write(0, data);
		return;

		case 0x01:
			BurnYM3812Write(1, data);
		return;

		case 0x02:
			DACWrite((data & 0x7f) << 1);
		// dac_0_signed_data_w
		return;

		case 0x03:
			DACWrite((data & 0x7f) << 1);
		//dac_1_signed_data_w
		return;
	}
}

unsigned char __fastcall armedf_read_port(unsigned short port)
{
//	bprintf (PRINT_NORMAL, _T("%2.2x read\n"), port & 0xff);

	switch (port & 0xff)
	{
		case 0x04:
			*soundlatch = 0;
		return 0;

		case 0x06:
			return *soundlatch;
	}

	return 0;
}

static int DrvSynchroniseStream(int nSoundRate)
{
	return (long long)ZetTotalCycles() * nSoundRate / 3072000;
}

static int DrvDoReset()
{
	DrvReset = 0;

	memset (AllRam, 0, RamEnd - AllRam);

	SekOpen(0);
	SekReset();
	SekClose();

	ZetOpen(0);
	ZetReset();
	ZetClose();

	BurnYM3812Reset();

	return 0;
}

static int MemIndex()
{
	unsigned char *Next; Next = AllMem;

	DrvZ80ROM	= Next; Next += 0x010000;
	Drv68KROM	= Next; Next += 0x060000;

	DrvGfxROM0	= Next; Next += 0x010000;
	DrvGfxROM1	= Next; Next += 0x080000;
	DrvGfxROM2	= Next; Next += 0x080000;
	DrvGfxROM3	= Next; Next += 0x080000;

	DrvPalette	= (unsigned int*)Next; Next += 0x0800 * sizeof(int);

	AllRam		= Next;

	DrvSprRAM	= Next; Next += 0x001000;
	DrvSprBuf	= Next; Next += 0x001000;
	DrvBgRAM	= Next; Next += 0x001000;
	DrvFgRAM	= Next; Next += 0x001000;
	DrvTxRAM	= Next; Next += 0x002000;
	DrvPalRAM	= Next; Next += 0x001000;
	Drv68KRAM0	= Next; Next += 0x005000;
	Drv68KRAM1	= Next; Next += 0x001000;
	Drv68KRAM2	= Next; Next += 0x001000;

	flipscreen	= Next; Next += 0x000001;
	soundlatch	= Next; Next += 0x000001;
	DrvVidRegs	= Next; Next += 0x000001;
	DrvScroll	= (unsigned short*)Next; Next += 0x000004 * sizeof(short);
	DrvMcuCmd	= (unsigned short*)Next; Next += 0x000020 * sizeof(short); 

	DrvZ80RAM	= Next; Next += 0x004000;

	RamEnd		= Next;
	MemEnd		= Next;

	return 0;
}

static int DrvGfxDecode()
{
	int Plane[4]   = { 0x000, 0x001, 0x002, 0x003 };
	int XOffs0[16] = { 0x004, 0x000, 0x00c, 0x008, 0x014, 0x010, 0x01c, 0x018,
			   0x024, 0x020, 0x02c, 0x028, 0x034, 0x030, 0x03c, 0x038 };
	int YOffs0[16] = { 0x000, 0x020, 0x040, 0x060, 0x080, 0x0a0, 0x0c0, 0x0e0,
			   0x100, 0x120, 0x140, 0x160, 0x180, 0x1a0, 0x1c0, 0x1e0 };
	int XOffs1[16] = { 0x000004, 0x000000, 0x100004, 0x100000, 0x00000c, 0x000008, 0x10000c, 0x100008,
			   0x000014, 0x000010, 0x100014, 0x100010, 0x00001c, 0x000018, 0x10001c, 0x100018 };
	int YOffs1[16] = { 0x000, 0x040, 0x080, 0x0c0, 0x100, 0x140, 0x180, 0x1c0,
			   0x200, 0x240, 0x280, 0x2c0, 0x300, 0x340, 0x380, 0x3c0 };

	unsigned char *tmp = (unsigned char*)malloc(0x40000);
	if (tmp == NULL) {
		return 1;
	}

	memcpy (tmp, DrvGfxROM0, 0x08000);

	GfxDecode(0x0400, 4,  8,  8, Plane, XOffs0, YOffs0, 0x100, tmp, DrvGfxROM0);

	memcpy (tmp, DrvGfxROM1, 0x40000);

	GfxDecode(0x0800, 4, 16, 16, Plane, XOffs0, YOffs1, 0x400, tmp, DrvGfxROM1);

	memcpy (tmp, DrvGfxROM2, 0x20000);

	GfxDecode(0x0400, 4, 16, 16, Plane, XOffs0, YOffs1, 0x400, tmp, DrvGfxROM2);

	memcpy (tmp, DrvGfxROM3, 0x40000);

	GfxDecode(0x0800, 4, 16, 16, Plane, XOffs1, YOffs0, 0x200, tmp, DrvGfxROM3);

	free (tmp);

	return 0;
}

static void Armedf68KInit()
{
	SekMapMemory(Drv68KROM,		0x000000, 0x05ffff, SM_ROM);
	SekMapMemory(DrvSprRAM,		0x060000, 0x060fff, SM_RAM);
	SekMapMemory(Drv68KRAM0,	0x061000, 0x065fff, SM_RAM);
	SekMapMemory(DrvBgRAM,		0x066000, 0x066fff, SM_RAM);
	SekMapMemory(DrvFgRAM,		0x067000, 0x067fff, SM_RAM);
	SekMapMemory(DrvTxRAM,		0x068000, 0x069fff, SM_RAM);
	SekMapMemory(DrvPalRAM,		0x06a000, 0x06afff, SM_RAM);
	SekMapMemory(Drv68KRAM1,	0x06b000, 0x06bfff, SM_RAM);
	SekMapMemory(Drv68KRAM2,	0x06c000, 0x06c7ff, SM_RAM);
	SekSetWriteWordHandler(0,	armedf_write_word);
}

static void Cclimbr268KInit()
{
	SekMapMemory(Drv68KROM,		0x000000, 0x05ffff, SM_ROM);
	SekMapMemory(DrvSprRAM,		0x060000, 0x060fff, SM_RAM);
	SekMapMemory(Drv68KRAM0,	0x061000, 0x063fff, SM_RAM);
	SekMapMemory(DrvPalRAM,		0x064000, 0x064fff, SM_RAM);
	SekMapMemory(DrvTxRAM,		0x068000, 0x069fff, SM_RAM);
	SekMapMemory(Drv68KRAM1,	0x06a000, 0x06a9ff, SM_RAM);
	SekMapMemory(Drv68KRAM2,	0x06c000, 0x06c9ff, SM_RAM);
	SekMapMemory(DrvFgRAM,		0x070000, 0x070fff, SM_RAM);
	SekMapMemory(DrvBgRAM,		0x074000, 0x074fff, SM_RAM);
	SekSetWriteWordHandler(0,	cclimbr2_write_word);
	SekSetWriteByteHandler(0,	cclimbr2_write_byte);
	SekSetReadWordHandler(0,	cclimbr2_read_word);
}

static int DrvInit(int (*pLoadRoms)(), void (*p68KInit)(), int zLen)
{
	AllMem = NULL;
	MemIndex();
	int nLen = MemEnd - (unsigned char *)0;
	if ((AllMem = (unsigned char *)malloc(nLen)) == NULL) return 1;
	memset(AllMem, 0, nLen);
	MemIndex();

	if (pLoadRoms) {
		if (pLoadRoms()) return 1;
	}

	DrvGfxDecode();

	SekInit(0, 0x68000);
	SekOpen(0);

	if (p68KInit)
	{
		p68KInit();
	}

	SekClose();

	ZetInit(1);
	ZetOpen(0);
	ZetMapArea(0x0000, zLen-1, 0, DrvZ80ROM);
	ZetMapArea(0x0000, zLen-1, 2, DrvZ80ROM);
	ZetMapArea(zLen+0, 0xffff, 0, DrvZ80RAM);
	ZetMapArea(zLen+0, 0xffff, 1, DrvZ80RAM);
	ZetMapArea(zLen+0, 0xffff, 2, DrvZ80RAM);
	ZetSetOutHandler(armedf_write_port);
	ZetSetInHandler(armedf_read_port);
	ZetMemEnd();
	ZetClose();

	BurnYM3812Init(4000000, NULL, &DrvSynchroniseStream, 0);
	BurnTimerAttachZetYM3812(3072000);

	DACInit(0, 1);

	GenericTilesInit();

	if (nScreenWidth == 320) {
		xoffset = 96;
		yoffset = 8;
	} else {
		xoffset = 112;
		yoffset = 16;
	}

	DrvDoReset();

	return 0;
}

static int DrvExit()
{
	GenericTilesExit();

	DACExit();
	BurnYM3812Exit();
	SekExit();
	ZetExit();

	free (AllMem);
	AllMem = NULL;

	return 0;
}

static inline void DrvPaletteRecalc()
{
	unsigned char r,g,b;
	unsigned short *pal = (unsigned short*)DrvPalRAM;
	for (int i = 0; i < 0x1000 / 2; i++) {
		int d = pal[i];

		r = (d >> 4) & 0xf0;
		g = (d & 0xf0);
		b = (d & 0x0f);

		r |= r >> 4;
		g |= g >> 4;
		b |= b << 4;

		DrvPalette[i] = BurnHighCol(r, g, b, 0);
	}
}

static inline void AssembleInputs()
{
	memset (DrvInputs, 0xff, 2 * sizeof(short));
	for (int i = 0; i < 16; i++) {
		DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
		DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
	}

	DrvInputs[2] = DrvDips[0] | 0xff00;
	DrvInputs[3] = DrvDips[1] | 0xff00;

	if (scroll_type == 1) {
		unsigned short *ptr = (unsigned short*)Drv68KRAM2;
		ptr[0] = DrvInputs[0];
		ptr[1] = DrvInputs[1];
		ptr[2] = DrvInputs[2];
		ptr[3] = DrvInputs[3];
	}
}

static void draw_layer(unsigned char *ram, unsigned char *gfxbase, int scrollx, int scrolly, int coloff, int code_and)
{
	unsigned short *vram = (unsigned short*)ram;

	for (int offs = 0; offs < 64 * 32; offs++)
	{
		int sy = (offs & 0x1f) << 4;
		int sx = (offs >> 5) << 4;
		sy -= scrolly + yoffset;
		sx -= scrollx + xoffset;
		if (sy < -15) sy += 512;
		if (sx < -15) sx += 1024;

		if (sy >= nScreenHeight || sx >= nScreenWidth) continue;

		int code = vram[offs] & code_and;
		int color = vram[offs] >> 11;

		if (*flipscreen) {
			Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, code, (nScreenWidth - 16) - sx, (nScreenHeight - 16) - sy, color, 4, 15, coloff, gfxbase);
		} else {
			Render16x16Tile_Mask_Clip(pTransDraw, code, sx, sy, color, 4, 15, coloff, gfxbase);
		}
	}
}

static void draw_txt_layer(int transp)
{
	unsigned short *vram = (unsigned short*)DrvTxRAM;

	for (int offs = 0; offs < 64 * 32; offs++)
	{
		int ofst = 0;
		int ofsta = 0x400;
		int sx = offs & 0x3f;
		int sy = offs >> 6;

		if (scroll_type == 1) {
	 		ofst = (sx << 5) | sy;
			ofsta = 0x800;
		} else if (scroll_type == 3 || scroll_type == 6) {
			ofst = ((sx & 0x1f) << 5) | sy | ((sx >> 5) << 11);
		} else {
			ofst = ((sy ^ 0x1f) << 5) | (sx & 0x1f) | ((sx >> 5) << 11);
		}

		sx = (sx << 3) - xoffset;
		sy = (sy << 3) - yoffset;
		if (scroll_type != 1) sx += 128;

		if (sx < -7 || sy < -7 || sx >= nScreenWidth || sy >= nScreenHeight) continue;

		int attr = vram[ofst+ofsta] & 0xff;
		int code = (vram[ofst] & 0xff) | ((attr & 3) << 8);

		if (transp) {
			if (*flipscreen) {
				Render8x8Tile_Mask_FlipXY_Clip(pTransDraw, code, (nScreenWidth - 8) - sx, (nScreenHeight - 8) - sy, attr >> 4, 4, 15, 0, DrvGfxROM0);
			} else {
				Render8x8Tile_Mask_Clip(pTransDraw, code, sx, sy, attr >> 4, 4, 15, 0, DrvGfxROM0);
			}
		} else {
			if (*flipscreen) {
				Render8x8Tile_FlipXY_Clip(pTransDraw, code, (nScreenWidth - 8) - sx, (nScreenHeight - 8) -sy, attr >> 4, 4, 0, DrvGfxROM0);
			} else {
				Render8x8Tile_Clip(pTransDraw, code, sx, sy, attr >> 4, 4, 0, DrvGfxROM0);
			}
		}
	}
}

static void draw_sprites(int priority)
{
	unsigned short *spr = (unsigned short*)DrvSprBuf;

	int sprlen = 0x1000;
	if (scroll_type == 0 || scroll_type == 5) sprlen = 0x400;

	for (int offs = 0; offs < sprlen / 2; offs+=4)
	{
		int attr  = spr[offs + 0];
		if (((attr & 0x3000) >> 12) != priority) continue;

		int code  = spr[offs + 1];
		int flipx = code & 0x2000;
		int flipy = code & 0x1000;
		int color =(spr[offs + 2] >> 8) & 0x1f;
		int sx    = spr[offs + 3];
		int sy    = sprite_offy + 240 - (attr & 0x1ff);
		code     &= 0xfff;

		if (*flipscreen) {
			sx = 320 - sx + 176;
			sy = 240 - sy + 1;
			flipx = !flipx;
			flipy = !flipy;
		}

		sy -= yoffset;
		sx -= xoffset;

		if (sx < -15 || sy < -15 || sx >= nScreenWidth || sy >= nScreenHeight) continue;

		if (flipy) {
			if (flipx) {
				Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, code, sx, sy, color, 4, 15, 0x200, DrvGfxROM3);
			} else {
				Render16x16Tile_Mask_FlipY_Clip(pTransDraw, code, sx, sy, color, 4, 15, 0x200, DrvGfxROM3);
			}
		} else {
			if (flipx) {
				Render16x16Tile_Mask_FlipX_Clip(pTransDraw, code, sx, sy, color, 4, 15, 0x200, DrvGfxROM3);
			} else {
				Render16x16Tile_Mask_Clip(pTransDraw, code, sx, sy, color, 4, 15, 0x200, DrvGfxROM3);
			}
		}
	}
}

static int DrvDraw()
{
	if (DrvRecalc) {
		DrvPaletteRecalc();
	}

	for (int offs = 0; offs < nScreenWidth * nScreenHeight; offs++)
		pTransDraw[offs] = 0x00ff;

	int txt_transp = 1;

	if (scroll_type == 0 || scroll_type == 5) {
		if ((*DrvMcuCmd & 0x000f) == 0x000f) txt_transp = 0;
	}

	if (scroll_type != 1) {
		unsigned short *ram = (unsigned short*)DrvTxRAM;
		if (scroll_type == 0 || scroll_type == 6) ram = DrvMcuCmd;

		DrvScroll[2] = (ram[13] & 0xff) | ((ram[14] & 3) << 8);
		DrvScroll[3] = (ram[11] & 0xff) | ((ram[12] & 1) << 8);
	}

	if (*DrvVidRegs & 0x08) draw_layer(DrvBgRAM, DrvGfxROM2, DrvScroll[0], DrvScroll[1], 0x600, 0x3ff);
	if ((*DrvMcuCmd & 0x30) == 0x30 && *DrvVidRegs & 0x01) draw_txt_layer(txt_transp);
	if (*DrvVidRegs & 0x02) draw_sprites(2);
	if ((*DrvMcuCmd & 0x30) == 0x20 && *DrvVidRegs & 0x01) draw_txt_layer(txt_transp);
	if (*DrvVidRegs & 0x04) draw_layer(DrvFgRAM, DrvGfxROM1, DrvScroll[2], DrvScroll[3], 0x400, 0x7ff);
	if ((*DrvMcuCmd & 0x30) == 0x10 && *DrvVidRegs & 0x01) draw_txt_layer(txt_transp);
	if (*DrvVidRegs & 0x02) draw_sprites(1);
	if ((*DrvMcuCmd & 0x30) == 0x00 && *DrvVidRegs & 0x01) draw_txt_layer(txt_transp);
	if (*DrvVidRegs & 0x02) draw_sprites(0);

	BurnTransferCopy(DrvPalette);

	memcpy (DrvSprBuf, DrvSprRAM, 0x1000);

	return 0;
}

static int DrvFrame()
{
	if (DrvReset) {
		DrvDoReset();
	}

	SekNewFrame();
	ZetNewFrame();

	AssembleInputs();

	int nSegment;
	int nInterleave = 128;
	int nTotalCycles[2] = { 8000000 / 60, 3072000 / 60 };
	int nCyclesDone[2] = { 0, 0 };

	SekOpen(0);
	ZetOpen(0);
	
//	memset (pBurnSoundOut, 0, nBurnSoundLen * sizeof(short)); 

	for (int i = 0; i < nInterleave; i++)
	{
		nSegment = nTotalCycles[0] / nInterleave;
		nCyclesDone[0] += SekRun(nSegment);

		nSegment = nTotalCycles[1] / nInterleave;
		nCyclesDone[1] += ZetRun(nSegment);
		ZetRaiseIrq(0);
	}
	
	if (pBurnSoundOut) {
		BurnYM3812Update(pBurnSoundOut, nBurnSoundLen);
	//	DACUpdate(pBurnSoundOut, nBurnSoundLen);
	}

	SekSetIRQLine(irqline, SEK_IRQSTATUS_AUTO);

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
		*pnMin = 0x029702;
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

		BurnYM3812Scan(nAction, pnMin);
		DACScan(nAction, pnMin);
	}

	return 0;
}


// Armed Formation

static struct BurnRomInfo armedfRomDesc[] = {
	{ "06.3d",	0x10000, 0x0f9015e2, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "01.3f",	0x10000, 0x816ff7c5, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "07.5d",	0x10000, 0x5b3144a5, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "02.4f",	0x10000, 0xfa10c29d, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "af_08.rom",	0x10000, 0xd1d43600, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "af_03.rom",	0x10000, 0xbbe1fe2d, 1 | BRF_PRG | BRF_ESS }, //  5

	{ "af_10.rom",	0x10000, 0xc5eacb87, 2 | BRF_PRG | BRF_ESS }, //  6 Z80 code

	{ "09.11c",	0x08000, 0x5c6993d5, 3 | BRF_GRA },           //  7 Characters

	{ "af_04.rom",	0x10000, 0x44d3af4f, 4 | BRF_GRA },           //  8 Foreground Tiles
	{ "af_05.rom",	0x10000, 0x92076cab, 4 | BRF_GRA },           //  9

	{ "af_14.rom",	0x10000, 0x8c5dc5a7, 5 | BRF_GRA },           // 10 Background Tiles
	{ "af_13.rom",	0x10000, 0x136a58a3, 5 | BRF_GRA },           // 11

	{ "af_11.rom",	0x20000, 0xb46c473c, 6 | BRF_GRA },           // 12 Sprites
	{ "af_12.rom",	0x20000, 0x23cb6bfe, 6 | BRF_GRA },           // 13
};

STD_ROM_PICK(armedf)
STD_ROM_FN(armedf)

static struct BurnRomInfo armedffRomDesc[] = {
	{ "af_06.rom",	0x10000, 0xc5326603, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "af_01.rom",	0x10000, 0x458e9542, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "af_07.rom",	0x10000, 0xcc8517f5, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "af_02.rom",	0x10000, 0x214ef220, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "af_08.rom",	0x10000, 0xd1d43600, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "af_03.rom",	0x10000, 0xbbe1fe2d, 1 | BRF_PRG | BRF_ESS }, //  5

	{ "af_10.rom",	0x10000, 0xc5eacb87, 2 | BRF_PRG | BRF_ESS }, //  6 Z80 code

	{ "af_09.rom",	0x08000, 0x7025e92d, 3 | BRF_GRA },           //  7 Characters

	{ "af_04.rom",	0x10000, 0x44d3af4f, 4 | BRF_GRA },           //  8 Foreground Tiles
	{ "af_05.rom",	0x10000, 0x92076cab, 4 | BRF_GRA },           //  9

	{ "af_14.rom",	0x10000, 0x8c5dc5a7, 5 | BRF_GRA },           // 10 Background Tiles
	{ "af_13.rom",	0x10000, 0x136a58a3, 5 | BRF_GRA },           // 11

	{ "af_11.rom",	0x20000, 0xb46c473c, 6 | BRF_GRA },           // 12 Sprites
	{ "af_12.rom",	0x20000, 0x23cb6bfe, 6 | BRF_GRA },           // 13
};

STD_ROM_PICK(armedff)
STD_ROM_FN(armedff)

static int ArmedfLoadRoms()
{
	if (BurnLoadRom(Drv68KROM + 0x000001,	 0, 2)) return 1;
	if (BurnLoadRom(Drv68KROM + 0x000000,	 1, 2)) return 1;
	if (BurnLoadRom(Drv68KROM + 0x020001,	 2, 2)) return 1;
	if (BurnLoadRom(Drv68KROM + 0x020000,	 3, 2)) return 1;
	if (BurnLoadRom(Drv68KROM + 0x040001,	 4, 2)) return 1;
	if (BurnLoadRom(Drv68KROM + 0x040000,	 5, 2)) return 1;

	if (BurnLoadRom(DrvZ80ROM,		 6, 1)) return 1;

	if (BurnLoadRom(DrvGfxROM0,		 7, 1)) return 1;

	if (BurnLoadRom(DrvGfxROM1 + 0x000000,	 8, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM1 + 0x010000,	 9, 1)) return 1;

	if (BurnLoadRom(DrvGfxROM2 + 0x000000,	10, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM2 + 0x010000,	11, 1)) return 1;

	if (BurnLoadRom(DrvGfxROM3 + 0x000000,	12, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM3 + 0x020000,	13, 1)) return 1;

	return 0;
}

static int ArmedfInit()
{
	scroll_type = 1;
	sprite_offy = 128;
	irqline = 1;

	return DrvInit(ArmedfLoadRoms, Armedf68KInit, 0xf800);
}

struct BurnDriver BurnDrvArmedf = {
	"armedf", NULL, NULL, NULL, "1988",
	"Armed Formation\0", NULL, "Nichibutsu", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_MISC_PRE90S, GBF_VERSHOOT, 0,
	NULL, armedfRomInfo, armedfRomName, NULL, NULL, ArmedfInputInfo, ArmedfDIPInfo,
	ArmedfInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x800,
	240, 320, 3, 4
};

struct BurnDriver BurnDrvArmedff = {
	"armedff", "armedf", NULL, NULL, "1988",
	"Armed Formation (Fillmore license)\0", NULL, "Nichibutsu", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_MISC_PRE90S, GBF_VERSHOOT, 0,
	NULL, armedffRomInfo, armedffRomName, NULL, NULL, ArmedfInputInfo, ArmedfDIPInfo,
	ArmedfInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x800,
	240, 320, 3, 4
};


// missing text layer on left?


// Crazy Climber 2 (Japan)

static struct BurnRomInfo cclimbr2RomDesc[] = {
	{ "4.bin",	0x10000, 0x7922ea14, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "1.bin",	0x10000, 0x2ac7ed67, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "6.bin",	0x10000, 0x7905c992, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "5.bin",	0x10000, 0x47be6c1e, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "3.bin",	0x10000, 0x1fb110d6, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "2.bin",	0x10000, 0x0024c15b, 1 | BRF_PRG | BRF_ESS }, //  5

	{ "11.bin",	0x04000, 0xfe0175be, 2 | BRF_PRG | BRF_ESS }, //  6 Z80 code
	{ "12.bin",	0x08000, 0x5ddf18f2, 2 | BRF_PRG | BRF_ESS }, //  7

	{ "10.bin",	0x08000, 0x7f475266, 3 | BRF_GRA },           //  8 Characters

	{ "7.bin",	0x10000, 0xcbdd3906, 4 | BRF_GRA },           //  9 Foreground Tiles
	{ "8.bin",	0x10000, 0xb2a613c0, 4 | BRF_GRA },           // 10

	{ "17.bin",	0x10000, 0xe24bb2d7, 5 | BRF_GRA },           // 11 Background Tiles
	{ "18.bin",	0x10000, 0x56834554, 5 | BRF_GRA },           // 12

	{ "15.bin",	0x10000, 0x4bf838be, 6 | BRF_GRA },           // 13 Sprites
	{ "16.bin",	0x10000, 0x21a265c5, 6 | BRF_GRA },           // 14
	{ "13.bin",	0x10000, 0x6b6ec999, 6 | BRF_GRA },           // 15
	{ "14.bin",	0x10000, 0xf426a4ad, 6 | BRF_GRA },           // 16

	{ "9.bin",	0x04000, 0x740d260f, 7 | BRF_GRA | BRF_OPT }, // 17 MCU data
};

STD_ROM_PICK(cclimbr2)
STD_ROM_FN(cclimbr2)

static int Cclimbr2LoadRoms()
{
	if (BurnLoadRom(Drv68KROM + 0x000001,	 0, 2)) return 1;
	if (BurnLoadRom(Drv68KROM + 0x000000,	 1, 2)) return 1;
	if (BurnLoadRom(Drv68KROM + 0x020001,	 2, 2)) return 1;
	if (BurnLoadRom(Drv68KROM + 0x020000,	 3, 2)) return 1;
	if (BurnLoadRom(Drv68KROM + 0x040001,	 4, 2)) return 1;
	if (BurnLoadRom(Drv68KROM + 0x040000,	 5, 2)) return 1;

	if (BurnLoadRom(DrvZ80ROM + 0x000000,	 6, 1)) return 1;
	if (BurnLoadRom(DrvZ80ROM + 0x004000,	 7, 1)) return 1;

	if (BurnLoadRom(DrvGfxROM0,		 8, 1)) return 1;

	if (BurnLoadRom(DrvGfxROM1 + 0x000000,	 9, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM1 + 0x010000,	10, 1)) return 1;

	if (BurnLoadRom(DrvGfxROM2 + 0x000000,	11, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM2 + 0x010000,	12, 1)) return 1;

	if (BurnLoadRom(DrvGfxROM3 + 0x000000,	13, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM3 + 0x010000,	14, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM3 + 0x020000,	15, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM3 + 0x030000,	16, 1)) return 1;

	return 0;
}

static int Cclimbr2Init()
{
	scroll_type = 4;
	sprite_offy = 0;
	irqline = 2;

	return DrvInit(Cclimbr2LoadRoms, Cclimbr268KInit, 0xc000);
}

struct BurnDriver BurnDrvCclimbr2 = {
	"cclimbr2", NULL, NULL, NULL, "1988",
	"Crazy Climber 2 (Japan)\0", NULL, "Nichibutsu", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_PRE90S, GBF_MISC, 0,
	NULL, cclimbr2RomInfo, cclimbr2RomName, NULL, NULL, Cclimbr2InputInfo, Cclimbr2DIPInfo,
	Cclimbr2Init, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x800,
	288, 224, 4, 3
};


// Crazy Climber 2 (Japan, Harder)

static struct BurnRomInfo cclmbr2aRomDesc[] = {
	{ "4a.bin",	0x10000, 0xe1d3192c, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "1a.bin",	0x10000, 0x3ef84974, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "6.bin",	0x10000, 0x7905c992, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "5.bin",	0x10000, 0x47be6c1e, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "3.bin",	0x10000, 0x1fb110d6, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "2.bin",	0x10000, 0x0024c15b, 1 | BRF_PRG | BRF_ESS }, //  5

	{ "11.bin",	0x04000, 0xfe0175be, 2 | BRF_PRG | BRF_ESS }, //  6 Z80 code
	{ "12.bin",	0x08000, 0x5ddf18f2, 2 | BRF_PRG | BRF_ESS }, //  7

	{ "10.bin",	0x08000, 0x7f475266, 3 | BRF_GRA },           //  8 Characters

	{ "7.bin",	0x10000, 0xcbdd3906, 4 | BRF_GRA },           //  9 Foreground Tiles
	{ "8.bin",	0x10000, 0xb2a613c0, 4 | BRF_GRA },           // 10

	{ "17.bin",	0x10000, 0xe24bb2d7, 5 | BRF_GRA },           // 11 Background Tiles
	{ "18.bin",	0x10000, 0x56834554, 5 | BRF_GRA },           // 12

	{ "15.bin",	0x10000, 0x4bf838be, 6 | BRF_GRA },           // 13 Sprites
	{ "16.bin",	0x10000, 0x21a265c5, 6 | BRF_GRA },           // 14
	{ "13.bin",	0x10000, 0x6b6ec999, 6 | BRF_GRA },           // 15
	{ "14.bin",	0x10000, 0xf426a4ad, 6 | BRF_GRA },           // 16

	{ "9.bin",	0x04000, 0x740d260f, 7 | BRF_GRA | BRF_OPT }, // 17 MCU data
};

STD_ROM_PICK(cclmbr2a)
STD_ROM_FN(cclmbr2a)

struct BurnDriver BurnDrvCclmbr2a = {
	"cclimbr2a", "cclimbr2", NULL, NULL, "1988",
	"Crazy Climber 2 (Japan, Harder)\0", NULL, "Nichibutsu", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_PRE90S, GBF_MISC, 0,
	NULL, cclmbr2aRomInfo, cclmbr2aRomName, NULL, NULL, Cclimbr2InputInfo, Cclimbr2DIPInfo,
	Cclimbr2Init, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x800,
	288, 224, 4, 3
};


// Kozure Ookami (Japan)

static struct BurnRomInfo kozureRomDesc[] = {
	{ "kozure8.6e",		0x10000, 0x6bbfb1e6, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "kozure3.6h",		0x10000, 0xf9178ec8, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "kozure7.5e",		0x10000, 0xa7ee09bb, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "kozure2.5h",		0x10000, 0x236d820f, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "kozure6.3e",		0x10000, 0x9120e728, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "kozure1.3h",		0x10000, 0x345fe7a5, 1 | BRF_PRG | BRF_ESS }, //  5

	{ "kozure11.17k",	0x10000, 0xdba51e2d, 2 | BRF_PRG | BRF_ESS }, //  6 Z80 code

	{ "kozure9.11e",	0x08000, 0xe041356e, 3 | BRF_GRA },           //  7 Characters

	{ "kozure5.15h",	0x20000, 0x0b510258, 4 | BRF_GRA },           //  8 Foreground Tiles
	{ "kozure4.14h",	0x10000, 0xfb8e13e6, 4 | BRF_GRA },           //  9

	{ "kozure14.8a",	0x10000, 0x94a9c3d0, 5 | BRF_GRA },           // 10 Background Tiles

	{ "kozure12.8d",	0x20000, 0x15f4021d, 6 | BRF_GRA },           // 11 Sprites
	{ "kozure13.9d",	0x20000, 0xb3b6c753, 6 | BRF_GRA },           // 12

	{ "kozure10.11c",	0x04000, 0xf48be21d, 7 | BRF_GRA | BRF_OPT }, // 13 MCU data

	{ "n82s129an.11j",	0x00100, 0x81244757, 8 | BRF_OPT },           // 14 Proms
};

STD_ROM_PICK(kozure)
STD_ROM_FN(kozure)

static int KozureLoadRoms()
{
	if (BurnLoadRom(Drv68KROM + 0x000001,	 0, 2)) return 1;
	if (BurnLoadRom(Drv68KROM + 0x000000,	 1, 2)) return 1;
	if (BurnLoadRom(Drv68KROM + 0x020001,	 2, 2)) return 1;
	if (BurnLoadRom(Drv68KROM + 0x020000,	 3, 2)) return 1;
	if (BurnLoadRom(Drv68KROM + 0x040001,	 4, 2)) return 1;
	if (BurnLoadRom(Drv68KROM + 0x040000,	 5, 2)) return 1;

	if (BurnLoadRom(DrvZ80ROM,		 6, 1)) return 1;

	if (BurnLoadRom(DrvGfxROM0,		 7, 1)) return 1;

	if (BurnLoadRom(DrvGfxROM1 + 0x000000,	 8, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM1 + 0x020000,	 9, 1)) return 1;

	if (BurnLoadRom(DrvGfxROM2,		10, 1)) return 1;

	if (BurnLoadRom(DrvGfxROM3 + 0x000000,	11, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM3 + 0x020000,	12, 1)) return 1;

	return 0;
}

static int KozureInit()
{
	scroll_type = 2;
	sprite_offy = 128;
	irqline = 1;

	return DrvInit(KozureLoadRoms, Cclimbr268KInit, 0xf800);
}

struct BurnDriver BurnDrvKozure = {
	"kozure", NULL, NULL, NULL, "1987",
	"Kozure Ookami (Japan)\0", "imperfect graphics", "Nichibutsu", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_MISC_PRE90S, GBF_SCRFIGHT, 0,
	NULL, kozureRomInfo, kozureRomName, NULL, NULL, ArmedfInputInfo, KozureDIPInfo,
	KozureInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x800,
	288, 224, 4, 3
};


// Chouji Meikyuu Legion (ver 2.03)

static struct BurnRomInfo legionRomDesc[] = {
	{ "lg3.bin",	0x10000, 0x777e4935, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "lg1.bin",	0x10000, 0xc4aeb724, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "legion.1d",	0x10000, 0xc2e45e1e, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "legion.1b",	0x10000, 0xc306660a, 1 | BRF_PRG | BRF_ESS }, //  3

	{ "legion.1h",	0x04000, 0x2ca4f7f0, 2 | BRF_PRG | BRF_ESS }, //  4 Z80 code

	{ "lg8.bin",	0x08000, 0xe0596570, 3 | BRF_GRA },           //  5 Characters

	{ "legion.1e",	0x10000, 0xa9d70faf, 4 | BRF_GRA },           //  6 Foreground Tiles
	{ "legion.1f",	0x08000, 0xf018313b, 4 | BRF_GRA },           //  7

	{ "legion.1l",	0x10000, 0x29b8adaa, 5 | BRF_GRA },           //  8 Background Tiles

	{ "legion.1k",	0x10000, 0xff5a0db9, 6 | BRF_GRA },           //  9 Sprites
	{ "legion.1j",	0x10000, 0xbae220c8, 6 | BRF_GRA },           // 10

	{ "lg7.bin",	0x04000, 0x533e2b58, 7 | BRF_GRA | BRF_OPT }, // 11 MCU data

	{ "legion.1i",	0x08000, 0x79f4a827, 0 | BRF_OPT },           // 12 Unknown
};

STD_ROM_PICK(legion)
STD_ROM_FN(legion)

static int LegionLoadRoms()
{
	if (BurnLoadRom(Drv68KROM + 0x000001,	 0, 2)) return 1;
	if (BurnLoadRom(Drv68KROM + 0x000000,	 1, 2)) return 1;
	if (BurnLoadRom(Drv68KROM + 0x020001,	 2, 2)) return 1;
	if (BurnLoadRom(Drv68KROM + 0x020000,	 3, 2)) return 1;

	if (BurnLoadRom(DrvZ80ROM,		 4, 1)) return 1;

	if (BurnLoadRom(DrvGfxROM0,		 5, 1)) return 1;

	if (BurnLoadRom(DrvGfxROM1 + 0x000000,	 6, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM1 + 0x018000,	 7, 1)) return 1;

	if (BurnLoadRom(DrvGfxROM2,		 8, 1)) return 1;

	if (BurnLoadRom(DrvGfxROM3 + 0x000000,	 9, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM3 + 0x020000,	10, 1)) return 1;

	return 0;
}

static int LegionInit()
{
	scroll_type = 3;
	sprite_offy = 0;
	irqline = 2;

	int nRet = DrvInit(LegionLoadRoms, Cclimbr268KInit, 0xf800);

	if (nRet == 0) { // hack
		*((unsigned short*)(Drv68KROM + 0x001d6)) = 0x0001;
		*((unsigned short*)(Drv68KROM + 0x00488)) = 0x4e71;
	}

	return nRet;
}

struct BurnDriver BurnDrvLegion = {
	"legion", NULL, NULL, NULL, "1987",
	"Chouji Meikyuu Legion (ver 2.03)\0", "imperfect graphics", "Nichibutsu", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_MISC_PRE90S, GBF_VERSHOOT, 0,
	NULL, legionRomInfo, legionRomName, NULL, NULL, ArmedfInputInfo, LegionDIPInfo,
	LegionInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x800,
	224, 288, 3, 4
};


// Chouji Meikyuu Legion (ver 1.05)

static struct BurnRomInfo legionoRomDesc[] = {
	{ "legion.1c",	0x10000, 0x21226660, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "legion.1a",	0x10000, 0x8c0cda1d, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "legion.1d",	0x10000, 0xc2e45e1e, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "legion.1b",	0x10000, 0xc306660a, 1 | BRF_PRG | BRF_ESS }, //  3

	{ "legion.1h",	0x04000, 0x2ca4f7f0, 2 | BRF_PRG | BRF_ESS }, //  4 Z80 code

	{ "legion.1g",	0x08000, 0xc50b0125, 3 | BRF_GRA },           //  5 Characters

	{ "legion.1e",	0x10000, 0xa9d70faf, 4 | BRF_GRA },           //  6 Foreground Tiles
	{ "legion.1f",	0x08000, 0xf018313b, 4 | BRF_GRA },           //  7

	{ "legion.1l",	0x10000, 0x29b8adaa, 5 | BRF_GRA },           //  8 Background Tiles

	{ "legion.1k",	0x10000, 0xff5a0db9, 6 | BRF_GRA },           //  9 Sprites
	{ "legion.1j",	0x10000, 0xbae220c8, 6 | BRF_GRA },           // 10

	{ "legion.1i",	0x08000, 0x79f4a827, 0 | BRF_OPT },           // 11 Unknown
};

STD_ROM_PICK(legiono)
STD_ROM_FN(legiono)

static int LegionoInit()
{
	scroll_type = 6;
	sprite_offy = 0;
	irqline = 2;

	int nRet = DrvInit(LegionLoadRoms, Cclimbr268KInit, 0xf800);

	if (nRet == 0) { // hack
		*((unsigned short*)(Drv68KROM + 0x001d6)) = 0x0001;
	}

	return nRet;
}

struct BurnDriver BurnDrvLegiono = {
	"legiono", "legion", NULL, NULL, "1987",
	"Chouji Meikyuu Legion (ver 1.05)\0", "imperfect graphics", "Nichibutsu", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_MISC_PRE90S, GBF_VERSHOOT, 0,
	NULL, legionoRomInfo, legionoRomName, NULL, NULL, ArmedfInputInfo, LegionDIPInfo,
	LegionoInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x800,
	224, 288, 3, 4
};


// Terra Force (set 1)

static struct BurnRomInfo terrafRomDesc[] = {
	{ "8.6e",		0x10000, 0xfd58fa06, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "3.6h",		0x10000, 0x54823a7d, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "7.4e",		0x10000, 0xfde8de7e, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "2.4h",		0x10000, 0xdb987414, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "6.3e",		0x10000, 0xa5bb8c3b, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "1.3h",		0x10000, 0xd2de6d28, 1 | BRF_PRG | BRF_ESS }, //  5

	{ "11.17k",		0x10000, 0x4407d475, 2 | BRF_PRG | BRF_ESS }, //  6 Z80 code

	{ "9.11e",		0x08000, 0xbc6f7cbc, 3 | BRF_GRA },           //  7 Characters

	{ "5.15h",		0x10000, 0x25d23dfd, 4 | BRF_GRA },           //  8 Foreground Tiles
	{ "4.13h",		0x10000, 0xb9b0fe27, 4 | BRF_GRA },           //  9

	{ "15.8a",		0x10000, 0x2144d8e0, 5 | BRF_GRA },           // 10 Background Tiles
	{ "14.6a",		0x10000, 0x744f5c9e, 5 | BRF_GRA },           // 11

	{ "12.7d",		0x10000, 0x2d1f2ceb, 6 | BRF_GRA },           // 12 Sprites
	{ "13.9d",		0x10000, 0x1d2f92d6, 6 | BRF_GRA },           // 13

	{ "10.11c",		0x04000, 0xac705812, 7 | BRF_GRA | BRF_OPT }, // 14 MCU data

	{ "n82s129an.11j",	0x00100, 0x81244757, 8 | BRF_OPT },           // 15 Proms
};

STD_ROM_PICK(terraf)
STD_ROM_FN(terraf)

static int TerrafInit()
{
	scroll_type = 0;
	sprite_offy = 128;
	irqline = 1;

	return DrvInit(ArmedfLoadRoms, Cclimbr268KInit, 0xf800);
}

struct BurnDriver BurnDrvTerraf = {
	"terraf", NULL, NULL, NULL, "1987",
	"Terra Force\0", "imperfect graphics", "Nichibutsu", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_PRE90S, GBF_HORSHOOT, 0,
	NULL, terrafRomInfo, terrafRomName, NULL, NULL, ArmedfInputInfo, TerrafDIPInfo,
	TerrafInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x800,
	320, 240, 4, 3
};


// Terra Force (US)

static struct BurnRomInfo terrafuRomDesc[] = {
	{ "tf-8.6e",		0x10000, 0xfea6dd64, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "tf-3.6h",		0x10000, 0x02f9d05a, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "tf-7.4e",		0x10000, 0xfde8de7e, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "tf-2.4h",		0x10000, 0xdb987414, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "tf-6.3e",		0x08000, 0xb91e9ba3, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "tf-1.3h",		0x08000, 0xd6e22375, 1 | BRF_PRG | BRF_ESS }, //  5

	{ "tf-001.17k",		0x10000, 0xeb6b4138, 2 | BRF_PRG | BRF_ESS }, //  6 Z80 code

	{ "9.11e",		0x08000, 0xbc6f7cbc, 3 | BRF_GRA },           //  7 Characters

	{ "5.15h",		0x10000, 0x25d23dfd, 4 | BRF_GRA },           //  8 Foreground Tiles
	{ "4.13h",		0x10000, 0xb9b0fe27, 4 | BRF_GRA },           //  9

	{ "15.8a",		0x10000, 0x2144d8e0, 5 | BRF_GRA },           // 10 Background Tiles
	{ "14.6a",		0x10000, 0x744f5c9e, 5 | BRF_GRA },           // 11

	{ "tf-003.7d",		0x10000, 0xd74085a1, 6 | BRF_GRA },           // 12 Sprites
	{ "tf-002.9d",		0x10000, 0x148aa0c5, 6 | BRF_GRA },           // 13

	{ "10.11c",		0x04000, 0xac705812, 7 | BRF_GRA | BRF_OPT }, // 14 MCU data

	{ "n82s129an.11j",	0x00100, 0x81244757, 8 | BRF_OPT },           // 15 Proms
};

STD_ROM_PICK(terrafu)
STD_ROM_FN(terrafu)

static int TerrafuInit()
{
	scroll_type = 5;
	sprite_offy = 128;
	irqline = 1;

	return DrvInit(ArmedfLoadRoms, Cclimbr268KInit, 0xf800);
}

struct BurnDriver BurnDrvTerrafu = {
	"terrafu", "terraf", NULL, NULL, "1987",
	"Terra Force (US)\0", "imperfect graphics", "Nichibutsu USA", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_PRE90S, GBF_HORSHOOT, 0,
	NULL, terrafuRomInfo, terrafuRomName, NULL, NULL, ArmedfInputInfo, TerrafDIPInfo,
	TerrafuInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x800,
	320, 240, 4, 3
};

// Terra Force (bootleg with additional Z80)

static struct BurnRomInfo terrafjbRomDesc[] = {
	{ "tfj-8.bin",		0x10000, 0xb11a6fa7, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "tfj-3.bin",		0x10000, 0x6c6aa7ed, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "tfj-7.bin",		0x10000, 0xfde8de7e, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "tfj-2.bin",		0x10000, 0xdb987414, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "tfb-6.bin",		0x08000, 0x552c3c63, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "tfb-1.bin",		0x08000, 0x6a0b94c7, 1 | BRF_PRG | BRF_ESS }, //  5

	{ "tf-001.17k",		0x10000, 0xeb6b4138, 2 | BRF_PRG | BRF_ESS }, //  6 Z80 Code

	{ "tfb-10.bin",		0x04000, 0x3f9aa367, 9 | BRF_PRG | BRF_ESS }, //  7 Z80 Code (Mcu replacement)

	{ "9.11e",		0x08000, 0xbc6f7cbc, 3 | BRF_GRA },           //  8 Characters

	{ "5.15h",		0x10000, 0x25d23dfd, 4 | BRF_GRA },           //  9 Foreground Tiles
	{ "4.13h",		0x10000, 0xb9b0fe27, 4 | BRF_GRA },           // 10

	{ "15.8a",		0x10000, 0x2144d8e0, 5 | BRF_GRA },           // 11 Background Tiles
	{ "14.6a",		0x10000, 0x744f5c9e, 5 | BRF_GRA },           // 12

	{ "tfj-12.7d",		0x10000, 0xd74085a1, 6 | BRF_GRA | BRF_OPT }, // 13 Sprites
	{ "tfj-13.9d",		0x10000, 0x148aa0c5, 6 | BRF_GRA | BRF_OPT }, // 14

	{ "n82s129an.11j",	0x00100, 0x81244757, 7 | BRF_OPT },           // 15 proms
};

STD_ROM_PICK(terrafjb)
STD_ROM_FN(terrafjb)

static int TerrafjbInit()
{
	return 1;
}

struct BurnDriverD BurnDrvTerrafjb = {
	"terrafjb", "terraf", NULL, NULL, "1987",
	"Terra Force (Japan bootleg with additional Z80)\0", "imperfect graphics", "bootleg", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_CLONE, 2, HARDWARE_MISC_PRE90S, GBF_HORSHOOT, 0,
	NULL, terrafjbRomInfo, terrafjbRomName, NULL, NULL, ArmedfInputInfo, TerrafDIPInfo,
	TerrafjbInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x800,
	320, 240, 4, 3
};

// Terra Force (set 2)

static struct BurnRomInfo terrafbRomDesc[] = {
	{ "tf-014.6e",		0x10000, 0x8e5f557f, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "tf-011.6h",		0x10000, 0x5320162a, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "tf-013.4e",		0x10000, 0xa86951e0, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "tf-010.4h",		0x10000, 0x58b5f43b, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "tf-012.3e",		0x08000, 0x4f0e1d76, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "tf-009.3h",		0x08000, 0xd1014280, 1 | BRF_PRG | BRF_ESS }, //  5

	{ "tf-001.17k",		0x10000, 0xeb6b4138, 2 | BRF_PRG | BRF_ESS }, //  6 Z80 code

	{ "9.11e",		0x08000, 0xbc6f7cbc, 3 | BRF_GRA },           //  7 Characters

	{ "5.15h",		0x10000, 0x25d23dfd, 4 | BRF_GRA },           //  8 Foreground Tiles
	{ "4.13h",		0x10000, 0xb9b0fe27, 4 | BRF_GRA },           //  9

	{ "15.8a",		0x10000, 0x2144d8e0, 5 | BRF_GRA },           // 10 Background Tiles
	{ "14.6a",		0x10000, 0x744f5c9e, 5 | BRF_GRA },           // 11

	{ "tfj-12.7d",		0x10000, 0xd74085a1, 6 | BRF_GRA },           // 12 Sprites
	{ "tfj-13.9d",		0x10000, 0x148aa0c5, 6 | BRF_GRA },           // 13

	{ "tf-10.11c",		0x04000, 0xac705812, 7 | BRF_GRA | BRF_OPT }, // 14 MCU data

	{ "n82s129an.11j",	0x00100, 0x81244757, 8 | BRF_OPT },           // 15 Proms
};

STD_ROM_PICK(terrafb)
STD_ROM_FN(terrafb)

struct BurnDriver BurnDrvTerrafb = {
	"terrafb", "terraf", NULL, NULL, "1987",
	"Terra Force (bootleg)\0", "imperfect graphics", "Nichibutsu", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_MISC_PRE90S, GBF_HORSHOOT, 0,
	NULL, terrafbRomInfo, terrafbRomName, NULL, NULL, ArmedfInputInfo, TerrafDIPInfo,
	TerrafuInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x800,
	320, 240, 4, 3
};
