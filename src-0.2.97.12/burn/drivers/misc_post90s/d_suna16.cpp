// FB Alpha SunA 16-bit hardware driver module
// Based on MAME driver by Luca Elia

#include "tiles_generic.h"
#include "burn_ym2151.h"
#include "burn_ym3526.h"
#include "driver.h"
#include "dac.h"
extern "C" {
#include "ay8910.h"
}

static unsigned char *AllMem;
static unsigned char *MemEnd;
static unsigned char *AllRam;
static unsigned char *RamEnd;
static unsigned char *Drv68KROM;
static unsigned char *DrvZ80ROM0;
static unsigned char *DrvZ80ROM1;
static unsigned char *DrvZ80ROM2;
static unsigned char *DrvGfxROM0;
static unsigned int  nGfxROM0Len;
static unsigned char *DrvGfxROM1;
static unsigned char *Drv68KRAM;
static unsigned char *DrvZ80RAM0;
static unsigned char *DrvSprRAM0;
static unsigned char *DrvSprRAM1;
static unsigned char *DrvPalRAM;
static unsigned char *DrvPalRAM2;
static unsigned int  *Palette;
static unsigned int  *DrvPalette;

static short *pFMBuffer;
static short *pAY8910Buffer[3];

static unsigned char DrvRecalc;

static unsigned char DrvJoy1[16];
static unsigned char DrvJoy2[16];
static unsigned char DrvJoy3[16];
static unsigned char DrvJoy4[16];
static unsigned char DrvJoy5[16];
static unsigned char DrvJoy6[16];

static unsigned char DrvDips[3];
static unsigned char DrvReset;

static unsigned short DrvInputs[6];

static int game_select = 0;

static unsigned char soundlatch;
static unsigned char soundlatch2;
static unsigned char soundlatch3;
static unsigned char flipscreen;
static unsigned char color_bank;

static unsigned char bestofbest_prot = 0;

static unsigned char z80bankdata[2];

static struct BurnInputInfo BestbestInputList[] = {
	{"Coin 1"       , BIT_DIGITAL  , DrvJoy1 + 15,	 "p1 coin"  },
	{"Coin 2"       , BIT_DIGITAL  , DrvJoy2 + 15,	 "p2 coin"  },

	{"P1 Start"     , BIT_DIGITAL  , DrvJoy1 + 14,	 "p1 start" },
	{"P1 Up"        , BIT_DIGITAL  , DrvJoy1 + 0,    "p1 up"    },
	{"P1 Down"      , BIT_DIGITAL  , DrvJoy1 + 1,    "p1 down"  },
	{"P1 Left"      , BIT_DIGITAL  , DrvJoy1 + 2,    "p1 left"  },
	{"P1 Right"     , BIT_DIGITAL  , DrvJoy1 + 3,    "p1 right" },
	{"P1 Button 1"  , BIT_DIGITAL  , DrvJoy1 + 4,	 "p1 fire 1"},
	{"P1 Button 2"  , BIT_DIGITAL  , DrvJoy1 + 5,	 "p1 fire 2"},
	{"P1 Button 3"  , BIT_DIGITAL  , DrvJoy1 + 6,	 "p1 fire 3"},
	{"P1 Button 4"  , BIT_DIGITAL  , DrvJoy1 + 9,	 "p1 fire 4"},
	{"P1 Button 5"  , BIT_DIGITAL  , DrvJoy1 + 10,	 "p1 fire 5"},
	{"P1 Button 6"  , BIT_DIGITAL  , DrvJoy1 + 11,	 "p1 fire 6"},

	{"P2 Start"     , BIT_DIGITAL  , DrvJoy2 + 14,	 "p2 start" },
	{"P2 Up"        , BIT_DIGITAL  , DrvJoy2 + 0,    "p2 up"    },
	{"P2 Down"      , BIT_DIGITAL  , DrvJoy2 + 1,    "p2 down"  },
	{"P2 Left"      , BIT_DIGITAL  , DrvJoy2 + 2,    "p2 left"  },
	{"P2 Right"     , BIT_DIGITAL  , DrvJoy2 + 3,    "p2 right" },
	{"P2 Button 1"  , BIT_DIGITAL  , DrvJoy2 + 4,	 "p2 fire 1"},
	{"P2 Button 2"  , BIT_DIGITAL  , DrvJoy2 + 5,	 "p2 fire 2"},
	{"P2 Button 3"  , BIT_DIGITAL  , DrvJoy2 + 6,	 "p2 fire 3"},
	{"P2 Button 4"  , BIT_DIGITAL  , DrvJoy2 + 9,	 "p2 fire 4"},
	{"P2 Button 5"  , BIT_DIGITAL  , DrvJoy2 + 10,	 "p2 fire 5"},
	{"P2 Button 6"  , BIT_DIGITAL  , DrvJoy2 + 11,	 "p2 fire 6"},

	{"Service"      , BIT_DIGITAL  , DrvJoy3 + 6,    "service"  },

	{"Reset",	  BIT_DIGITAL  , &DrvReset,	"reset"     },
	{"Dip 1",	  BIT_DIPSWITCH, DrvDips + 0,	"dip"	    },
	{"Dip 2",	  BIT_DIPSWITCH, DrvDips + 1,	"dip"	    },
};

STDINPUTINFO(Bestbest)

static struct BurnInputInfo UballoonInputList[] = {
	{"Coin 1"       , BIT_DIGITAL  , DrvJoy1 + 15,	 "p1 coin"  },
	{"Coin 2"       , BIT_DIGITAL  , DrvJoy2 + 15,	 "p2 coin"  },

	{"P1 Start"     , BIT_DIGITAL  , DrvJoy1 + 14,	 "p1 start" },
	{"P1 Up"        , BIT_DIGITAL  , DrvJoy1 + 0,    "p1 up"    },
	{"P1 Down"      , BIT_DIGITAL  , DrvJoy1 + 1,    "p1 down"  },
	{"P1 Left"      , BIT_DIGITAL  , DrvJoy1 + 2,    "p1 left"  },
	{"P1 Right"     , BIT_DIGITAL  , DrvJoy1 + 3,    "p1 right" },
	{"P1 Button 1"  , BIT_DIGITAL  , DrvJoy1 + 4,	 "p1 fire 1"},
	{"P1 Button 2"  , BIT_DIGITAL  , DrvJoy1 + 5,	 "p1 fire 2"},

	{"P2 Start"     , BIT_DIGITAL  , DrvJoy2 + 14,	 "p2 start" },
	{"P2 Up"        , BIT_DIGITAL  , DrvJoy2 + 0,    "p2 up"    },
	{"P2 Down"      , BIT_DIGITAL  , DrvJoy2 + 1,    "p2 down"  },
	{"P2 Left"      , BIT_DIGITAL  , DrvJoy2 + 2,    "p2 left"  },
	{"P2 Right"     , BIT_DIGITAL  , DrvJoy2 + 3,    "p2 right" },
	{"P2 Button 1"  , BIT_DIGITAL  , DrvJoy2 + 4,	 "p2 fire 1"},
	{"P2 Button 2"  , BIT_DIGITAL  , DrvJoy2 + 5,	 "p2 fire 2"},

	{"Service"      , BIT_DIGITAL  , DrvJoy1 + 12,   "service"  },

	{"Reset",	  BIT_DIGITAL  , &DrvReset,	"reset"     },
	{"Dip 1",	  BIT_DIPSWITCH, DrvDips + 0,	"dip"	    },
	{"Dip 2",	  BIT_DIPSWITCH, DrvDips + 1,	"dip"	    },
	{"Dip 3",	  BIT_DIPSWITCH, DrvDips + 2,	"dip"	    },
};

STDINPUTINFO(Uballoon)

static struct BurnInputInfo BssoccerInputList[] = {
	{"Coin 1"       , BIT_DIGITAL  , DrvJoy6 + 4,	 "p1 coin"  },
	{"Coin 2"       , BIT_DIGITAL  , DrvJoy6 + 5,	 "p2 coin"  },
	{"Coin 3"       , BIT_DIGITAL  , DrvJoy6 + 6,	 "p3 coin"  },
	{"Coin 4"       , BIT_DIGITAL  , DrvJoy6 + 7,	 "p4 coin"  },

	{"P1 Start"     , BIT_DIGITAL  , DrvJoy1 + 7,	 "p1 start" },
	{"P1 Up"        , BIT_DIGITAL  , DrvJoy1 + 0,    "p1 up"    },
	{"P1 Down"      , BIT_DIGITAL  , DrvJoy1 + 1,    "p1 down"  },
	{"P1 Left"      , BIT_DIGITAL  , DrvJoy1 + 2,    "p1 left"  },
	{"P1 Right"     , BIT_DIGITAL  , DrvJoy1 + 3,    "p1 right" },
	{"P1 Button 1"  , BIT_DIGITAL  , DrvJoy1 + 4,	 "p1 fire 1"},
	{"P1 Button 2"  , BIT_DIGITAL  , DrvJoy1 + 5,	 "p1 fire 2"},
	{"P1 Button 3"  , BIT_DIGITAL  , DrvJoy1 + 6,	 "p1 fire 3"},

	{"P2 Start"     , BIT_DIGITAL  , DrvJoy2 + 7,	 "p2 start" },
	{"P2 Up"        , BIT_DIGITAL  , DrvJoy2 + 0,    "p2 up"    },
	{"P2 Down"      , BIT_DIGITAL  , DrvJoy2 + 1,    "p2 down"  },
	{"P2 Left"      , BIT_DIGITAL  , DrvJoy2 + 2,    "p2 left"  },
	{"P2 Right"     , BIT_DIGITAL  , DrvJoy2 + 3,    "p2 right" },
	{"P2 Button 1"  , BIT_DIGITAL  , DrvJoy2 + 4,	 "p2 fire 1"},
	{"P2 Button 2"  , BIT_DIGITAL  , DrvJoy2 + 5,	 "p2 fire 2"},
	{"P2 Button 3"  , BIT_DIGITAL  , DrvJoy2 + 6,	 "p2 fire 3"},

	{"P3 Start"     , BIT_DIGITAL  , DrvJoy3 + 7,	 "p3 start" },
	{"P3 Up"        , BIT_DIGITAL  , DrvJoy3 + 0,    "p3 up"    },
	{"P3 Down"      , BIT_DIGITAL  , DrvJoy3 + 1,    "p3 down"  },
	{"P3 Left"      , BIT_DIGITAL  , DrvJoy3 + 2,    "p3 left"  },
	{"P3 Right"     , BIT_DIGITAL  , DrvJoy3 + 3,    "p3 right" },
	{"P3 Button 1"  , BIT_DIGITAL  , DrvJoy3 + 4,	 "p3 fire 1"},
	{"P3 Button 2"  , BIT_DIGITAL  , DrvJoy3 + 5,	 "p3 fire 2"},
	{"P3 Button 3"  , BIT_DIGITAL  , DrvJoy3 + 6,	 "p3 fire 3"},

	{"P4 Start"     , BIT_DIGITAL  , DrvJoy4 + 7,	 "p4 start" },
	{"P4 Up"        , BIT_DIGITAL  , DrvJoy4 + 0,    "p4 up"    },
	{"P4 Down"      , BIT_DIGITAL  , DrvJoy4 + 1,    "p4 down"  },
	{"P4 Left"      , BIT_DIGITAL  , DrvJoy4 + 2,    "p4 left"  },
	{"P4 Right"     , BIT_DIGITAL  , DrvJoy4 + 3,    "p4 right" },
	{"P4 Button 1"  , BIT_DIGITAL  , DrvJoy4 + 4,	 "p4 fire 1"},
	{"P4 Button 2"  , BIT_DIGITAL  , DrvJoy4 + 5,	 "p4 fire 2"},
	{"P4 Button 3"  , BIT_DIGITAL  , DrvJoy4 + 6,	 "p4 fire 3"},

	{"Service"      , BIT_DIGITAL  , DrvJoy3 + 6,    "service"  },

	{"Reset",	  BIT_DIGITAL  , &DrvReset,	"reset"     },
	{"Dip 1",	  BIT_DIPSWITCH, DrvDips + 0,	"dip"	    },
	{"Dip 2",	  BIT_DIPSWITCH, DrvDips + 1,	"dip"	    },
	{"Dip 3",	  BIT_DIPSWITCH, DrvDips + 2,	"dip"	    },
};

STDINPUTINFO(Bssoccer)

static struct BurnInputInfo SunaqInputList[] = {
	{"Coin 1"       , BIT_DIGITAL  , DrvJoy1 + 7,	 "p1 coin"  },
	{"Coin 2"       , BIT_DIGITAL  , DrvJoy2 + 7,	 "p2 coin"  },

	{"P1 Start"     , BIT_DIGITAL  , DrvJoy1 + 6,	 "p1 start" },
	{"P1 Button 1"  , BIT_DIGITAL  , DrvJoy1 + 0,	 "p1 fire 1"},
	{"P1 Button 2"  , BIT_DIGITAL  , DrvJoy1 + 1,	 "p1 fire 2"},
	{"P1 Button 3"  , BIT_DIGITAL  , DrvJoy1 + 2,	 "p1 fire 3"},
	{"P1 Button 4"  , BIT_DIGITAL  , DrvJoy1 + 3,	 "p1 fire 4"},

	{"P2 Start"     , BIT_DIGITAL  , DrvJoy2 + 6,	 "p2 start" },
	{"P2 Button 1"  , BIT_DIGITAL  , DrvJoy2 + 0,	 "p2 fire 1"},
	{"P2 Button 2"  , BIT_DIGITAL  , DrvJoy2 + 1,	 "p2 fire 2"},
	{"P2 Button 3"  , BIT_DIGITAL  , DrvJoy2 + 2,	 "p2 fire 3"},
	{"P2 Button 4"  , BIT_DIGITAL  , DrvJoy2 + 3,	 "p2 fire 4"},

	{"Service"      , BIT_DIGITAL  , DrvJoy3 + 6,    "service"  },

	{"Reset",	  BIT_DIGITAL  , &DrvReset,	"reset"	    },
	{"Dip 1",	  BIT_DIPSWITCH, DrvDips + 0,	"dip"	    },
};

STDINPUTINFO(Sunaq)

static struct BurnDIPInfo bestbestDIPList[]=
{
	{0x1a, 0xff, 0xff, 0xff, NULL			},
	{0x1b, 0xff, 0xff, 0xff, NULL			},
	
	{0x1a, 0xfe, 0,       8, "Coinage" 		},
	{0x1a, 0x01, 0x07, 0x00, "5C 1C" 		},
	{0x1a, 0x01, 0x07, 0x01, "4C 1C" 		},
	{0x1a, 0x01, 0x07, 0x02, "3C 1C" 		},
	{0x1a, 0x01, 0x07, 0x03, "2C 1C" 		},
	{0x1a, 0x01, 0x07, 0x07, "1C 1C" 		},
	{0x1a, 0x01, 0x07, 0x06, "1C 2C" 		},
	{0x1a, 0x01, 0x07, 0x05, "1C 3C" 		},
	{0x1a, 0x01, 0x07, 0x04, "1C 4C" 		},

	{0x1a, 0xfe, 0,       4, "Difficulty" 		},
	{0x1a, 0x01, 0x18, 0x18, "Easy" 		},
	{0x1a, 0x01, 0x18, 0x10, "Normal" 		},
	{0x1a, 0x01, 0x18, 0x08, "Hard" },
	{0x1a, 0x01, 0x18, 0x00, "Hardest" 		},

	{0x1a, 0xfe, 0,       2, "Display Combos"  	},
	{0x1a, 0x01, 0x20, 0x00, "Off"			},
	{0x1a, 0x01, 0x20, 0x20, "On"			},

	{0x1a, 0xfe, 0,       2, "Demo Sounds"		},
	{0x1a, 0x01, 0x80, 0x80, "Off" 			},
	{0x1a, 0x01, 0x80, 0x00, "On" 			},

	{0x1b, 0xfe, 0,       2, "Flip Screen" 		},
	{0x1b, 0x01, 0x01, 0x01, "Off" 			},
	{0x1b, 0x01, 0x01, 0x00, "On" 			},

	{0x1b, 0xfe, 0,       4, "Play Time" 		},
	{0x1b, 0x01, 0x06, 0x06, "1:10"  		},
	{0x1b, 0x01, 0x06, 0x04, "1:20"  		},
	{0x1b, 0x01, 0x06, 0x02, "1:30"  		},
	{0x1b, 0x01, 0x06, 0x00, "1:40"  		},
};

STDDIPINFO(bestbest)

static struct BurnDIPInfo bssoccerDIPList[]=
{
	{0x26, 0xff, 0xff, 0xff, NULL },
	{0x27, 0xff, 0xff, 0xff, NULL },
	{0x28, 0xff, 0xff, 0xff, NULL },

	{0x26, 0xfe, 0,       8, "Coinage" },
	{0x26, 0x01, 0x07, 0x00, "4C 1C" },
	{0x26, 0x01, 0x07, 0x01, "3C 1C" },
	{0x26, 0x01, 0x07, 0x02, "2C 1C" },
	{0x26, 0x01, 0x07, 0x07, "1C 1C" },
	{0x26, 0x01, 0x07, 0x06, "1C 2C" },
	{0x26, 0x01, 0x07, 0x05, "1C 3C" },
	{0x26, 0x01, 0x07, 0x04, "1C 4C" },
	{0x26, 0x01, 0x07, 0x03, "1C 5C" },

	{0x26, 0xfe, 0,       4, "Difficulty" },
	{0x26, 0x01, 0x18, 0x10, "Easy" },
	{0x26, 0x01, 0x18, 0x18, "Normal" },
	{0x26, 0x01, 0x18, 0x08, "Hard" },
	{0x26, 0x01, 0x18, 0x00, "Hardest?"   },

	{0x26, 0xfe, 0,       2, "Demo Sounds" },
	{0x26, 0x01, 0x20, 0x00, "Off" },
	{0x26, 0x01, 0x20, 0x20, "On" },

	{0x26, 0xfe, 0,       2, "Flip Screen" },
	{0x26, 0x01, 0x40, 0x40, "Off" },
	{0x26, 0x01, 0x40, 0x00, "On" },

	{0x27, 0xfe, 0,       4, "Play Time P1"  },
	{0x27, 0x01, 0x03, 0x03, "1:30"  },
	{0x27, 0x01, 0x03, 0x02, "1:45"  },
	{0x27, 0x01, 0x03, 0x01, "2:00"  },
	{0x27, 0x01, 0x03, 0x00, "2:15"  },

	{0x27, 0xfe, 0,       4, "Play Time P2"  },
	{0x27, 0x01, 0x0c, 0x0c, "1:30"  },
	{0x27, 0x01, 0x0c, 0x08, "1:45"  },
	{0x27, 0x01, 0x0c, 0x04, "2:00"  },
	{0x27, 0x01, 0x0c, 0x00, "2:15"  },

	{0x27, 0xfe, 0,       4, "Play Time P3"  },
	{0x27, 0x01, 0x30, 0x30, "1:30"  },
	{0x27, 0x01, 0x30, 0x20, "1:45"  },
	{0x27, 0x01, 0x30, 0x10, "2:00"  },
	{0x27, 0x01, 0x30, 0x00, "2:15"  },

	{0x27, 0xfe, 0,       4, "Play Time P4"  },
	{0x27, 0x01, 0xc0, 0xc0, "1:30"  },
	{0x27, 0x01, 0xc0, 0x80, "1:45"  },
	{0x27, 0x01, 0xc0, 0x40, "2:00"  },
	{0x27, 0x01, 0xc0, 0x00, "2:15"  },

	{0x28, 0xfe, 0,       2, "Copyright"  },
	{0x28, 0x01, 0x01, 0x01, "Distributer Unico"  },
	{0x28, 0x01, 0x01, 0x00, "All Rights Reserved"  },
};

STDDIPINFO(bssoccer)


static struct BurnDIPInfo sunaqDIPList[]=
{
	{0x0e, 0xff, 0xff, 0xff, NULL },

	{0x0e, 0xfe, 0,       8, "Coinage" },
	{0x0e, 0x01, 0x07, 0x00, "5C 1C" },
	{0x0e, 0x01, 0x07, 0x01, "4C 1C" },
	{0x0e, 0x01, 0x07, 0x02, "3C 1C" },
	{0x0e, 0x01, 0x07, 0x03, "2C 1C" },
	{0x0e, 0x01, 0x07, 0x07, "1C 1C" },
	{0x0e, 0x01, 0x07, 0x06, "1C 2C" },
	{0x0e, 0x01, 0x07, 0x05, "1C 3C" },
	{0x0e, 0x01, 0x07, 0x04, "1C 4C" },

	{0x0e, 0xfe, 0,       4, "Difficulty" },
	{0x0e, 0x01, 0x18, 0x00, "Easy" },
	{0x0e, 0x01, 0x18, 0x08, "Normal" },
	{0x0e, 0x01, 0x18, 0x10, "Hard" },
	{0x0e, 0x01, 0x18, 0x18, "Hardest" },

	{0x0e, 0xfe, 0,       2, "Demo Sounds" },
	{0x0e, 0x01, 0x20, 0x20, "Off" },
	{0x0e, 0x01, 0x20, 0x00, "On" },

	{0x0e, 0xfe, 0,       2, "Flip Screen" },
	{0x0e, 0x01, 0x40, 0x40, "Off" },
	{0x0e, 0x01, 0x40, 0x00, "On" },
};

STDDIPINFO(sunaq)


static struct BurnDIPInfo uballoonDIPList[]=
{
	{0x12, 0xff, 0xff, 0xff, NULL },
	{0x13, 0xff, 0xff, 0xff, NULL },
	{0x14, 0xff, 0xff, 0xff, NULL },

	{0x12, 0xfe, 0,       2, "Copyright"  },
	{0x12, 0x01, 0x30, 0x30, "Distributer Unico"  },
	{0x12, 0x01, 0x30, 0x20, "All Rights Reserved"  },
//	{0x12, 0x01, 0x30, 0x10, "Distributer Unico"  },
//	{0x12, 0x01, 0x30, 0x00, "All Rights Reserved"  },

	{0x13, 0xfe, 0,       8, "Coinage" },
	{0x13, 0x01, 0x07, 0x00, "5C 1C" },
	{0x13, 0x01, 0x07, 0x01, "4C 1C" },
	{0x13, 0x01, 0x07, 0x02, "3C 1C" },
	{0x13, 0x01, 0x07, 0x03, "2C 1C" },
	{0x13, 0x01, 0x07, 0x07, "1C 1C" },
	{0x13, 0x01, 0x07, 0x06, "1C 2C" },
	{0x13, 0x01, 0x07, 0x05, "1C 3C" },
	{0x13, 0x01, 0x07, 0x04, "1C 4C" },

	{0x13, 0xfe, 0,       4, "Lives" },
	{0x13, 0x01, 0x18, 0x10, "2"  },
	{0x13, 0x01, 0x18, 0x18, "3"  },
	{0x13, 0x01, 0x18, 0x08, "4"  },
	{0x13, 0x01, 0x18, 0x00, "5"  },

	{0x13, 0xfe, 0,       4, "Difficulty" },
	{0x13, 0x01, 0x60, 0x40, "Easy" },
	{0x13, 0x01, 0x60, 0x60, "Normal" },
	{0x13, 0x01, 0x60, 0x20, "Hard" },
	{0x13, 0x01, 0x60, 0x00, "Hardest" },

	{0x14, 0xfe, 0,       2, "Flip Screen" },
	{0x14, 0x01, 0x01, 0x01, "Off" },
	{0x14, 0x01, 0x01, 0x00, "On" },

	{0x14, 0xfe, 0,       2, "Cabinet" },
	{0x14, 0x01, 0x02, 0x02, "Upright" },
	{0x14, 0x01, 0x02, 0x00, "Cocktail" },

	{0x14, 0xfe, 0,       8, "Bonus Life" },
	{0x14, 0x01, 0x1c, 0x1c, "200K"  },
	{0x14, 0x01, 0x1c, 0x10, "300K, 1000K"  },
	{0x14, 0x01, 0x1c, 0x18, "400K"  },
	{0x14, 0x01, 0x1c, 0x0c, "500K, 1500K"  },
	{0x14, 0x01, 0x1c, 0x08, "500K, 2000K"  },
	{0x14, 0x01, 0x1c, 0x04, "500K, 3000K"  },
	{0x14, 0x01, 0x1c, 0x14, "600K"  },
	{0x14, 0x01, 0x1c, 0x00, "None" },

	{0x14, 0xfe, 0,       2, "Demo Sounds" },
	{0x14, 0x01, 0x80, 0x80, "Off" },
	{0x14, 0x01, 0x80, 0x00, "On" },
};

STDDIPINFO(uballoon)


//-------------------------------------------------------------------------------------------------
// Generic functions

static void suna_palette_write(int offset)
{
	unsigned char r, b, g;
	unsigned short data = *((unsigned short*)(DrvPalRAM + offset));

	r = (data >>  0) & 0x1f;
	r = (r << 3) | (r >> 2);

	g = (data >>  5) & 0x1f;
	g = (g << 3) | (g >> 2);

	b = (data >> 10) & 0x1f;
	b = (b << 3) | (b >> 2);

	Palette[offset>>1] = (r << 16) | (g << 8) | b;
	DrvPalette[offset>>1] = BurnHighCol(r, g, b, 0);

	return;
}

static void write_dac(int data)
{
	DACWrite(((data & 0x0f) * 0x11)>>1);
	return;
}

//-------------------------------------------------------------------------------------------------
// Memory handlers


//----------------------------------------------------------------
// Best of Best


//------------------
// 68k

unsigned short __fastcall bestbest_read_word(unsigned int address)
{
	switch (address & ~1)
	{
		case 0x500000:
			return DrvInputs[0];

		case 0x500002:
			return DrvInputs[1];

		case 0x500004:
			return DrvInputs[2];
	}

	return 0;
}

unsigned char __fastcall bestbest_read_byte(unsigned int address)
{
	switch (address)
	{
		case 0x500000:
		case 0x500001:
			return DrvInputs[0] >> ((~address & 1) << 3);

		case 0x500002:
		case 0x500003:
			return DrvInputs[1] >> ((~address & 1) << 3);

		case 0x500004:
		case 0x500005:
			return DrvInputs[2] >> ((~address & 1) << 3);

		case 0x500019:
			return bestofbest_prot;
	}

	return 0;
}

void __fastcall bestbest_write_word(unsigned int address, unsigned short data)
{
	if ((address & 0xfff000) == 0x540000) {
		*((unsigned short*)(DrvPalRAM + (address & 0x0fff))) = data;
		suna_palette_write(address & 0xffe);
		return;
	}

	switch (address & ~1)
	{
		case 0x500000:
			soundlatch = data;
		return;

		case 0x500002:
			flipscreen = data & 0x10;
		return;
	}

	return;
}

void __fastcall bestbest_write_byte(unsigned int address, unsigned char data)
{
	if ((address & 0xfff000) == 0x540000) {
		DrvPalRAM[address & 0xfff] = data;
		suna_palette_write(address & 0xffe);
		return;
	}

	switch (address)
	{
		case 0x500000:
		case 0x500001:
			soundlatch = data;
		return;

		case 0x500002:
		case 0x500003:
			flipscreen = data & 0x10;
		return;

		case 0x500008:
		case 0x500009:
			switch (data & 0xff) {
				case 0x00:	bestofbest_prot ^= 0x09;	break;
				case 0x08:	bestofbest_prot ^= 0x02;	break;
				case 0x0c:	bestofbest_prot ^= 0x03;	break;
			}
		return;
	}

	return;
}

//------------------
// Z80 #0

void __fastcall bestbest_sound0_write(unsigned short address, unsigned char data)
{
	switch (address)
	{
		case 0xc000:
		case 0xc001:
			BurnYM3526Write(address & 1, data);
		return;

		case 0xc002:
		case 0xc003:
			AY8910Write(0, address & 1, data);
		return;

		case 0xf000:
			soundlatch2 = data;
		return;
	}

	return;
}

unsigned char __fastcall bestbest_sound0_read(unsigned short address)
{
	switch (address)
	{
		case 0xf800:
			return soundlatch;
	}

	return 0;
}

//------------------
// Z80 #1

void __fastcall bestbest_sound1_out(unsigned short port, unsigned char data)
{
	switch (port & 0xff)
	{
		case 0x00:
		case 0x01:
			write_dac(data);;
		return;

		case 0x02:
		case 0x03:
			write_dac(data);
		return;
	}

	return;
}

unsigned char __fastcall bestbest_sound1_in(unsigned short port)
{
	switch (port & 0xff)
	{
		case 0x00:
			return soundlatch2;
	}

	return 0;
}


//----------------------------------------------------------------
// SunA Quiz


//------------------
// 68k

unsigned short __fastcall sunaq_read_word(unsigned int address)
{
	if ((address & 0xfff000) == 0x540000) {
		if (address & 0x200) {
			return *((unsigned short*)(DrvPalRAM2 + (address & 0xffe)));
		} else {
			address += color_bank << 9;
			return *((unsigned short*)(DrvPalRAM + (address & 0xffe)));
		}
	}

	switch (address & ~1)
	{
		case 0x500000:
			return DrvInputs[0];

		case 0x500002:
			return DrvInputs[1];

		case 0x500004:
			return DrvInputs[2];

		case 0x500006:
			return DrvInputs[3];
	}

	return 0;
}

unsigned char __fastcall sunaq_read_byte(unsigned int address)
{
	if ((address & 0xfff000) == 0x540000) {
		if (address & 0x200) {
			return DrvPalRAM2[address & 0xffe];
		} else {
			address += color_bank << 9;
			return DrvPalRAM[address & 0xffe];
		}
	}

	switch (address)
	{
		case 0x500000:
		case 0x500001:
			return DrvInputs[0] >> ((~address & 1) << 3);

		case 0x500002:
		case 0x500003:
			return DrvInputs[1] >> ((~address & 1) << 3);

		case 0x500004:
		case 0x500005:
			return DrvInputs[2] >> ((~address & 1) << 3); 

		case 0x500006:
		case 0x500007:
			return DrvInputs[3] >> ((~address & 1) << 3);
	}

	return 0;
}

void __fastcall sunaq_write_word(unsigned int address, unsigned short data)
{
	if ((address & 0xfff000) == 0x540000) {
		if (address & 0x200) {
			*((unsigned short*)(DrvPalRAM2 + (address & 0xffff))) = data;
		} else {
			address += color_bank << 9;
			*((unsigned short*)(DrvPalRAM + (address & 0xffff))) = data;
			suna_palette_write(address & 0xffff);
		}
		return;
	}

	switch (address & ~1)
	{
		case 0x500000:
			soundlatch = data;
		return;

		case 0x500002:
			flipscreen = data & 0x01;
			color_bank = (data >> 2) & 1;
		return;

		case 0x500004:
			// coin counter
		return;
	}
	return;
}

void __fastcall sunaq_write_byte(unsigned int address, unsigned char data)
{
	if ((address & 0xfff000) == 0x540000) {
		if (address & 0x200) {
			DrvPalRAM2[address & 0xfff] = data;
		} else {
			address += color_bank << 9;
			DrvPalRAM[address & 0xfff] = data;
			suna_palette_write(address & 0xffe);
		}

		return;
	}

	switch (address)
	{
		case 0x500000:
		case 0x500001:
			soundlatch = data;
		return;

		case 0x500002:
		case 0x500003:
			flipscreen = data & 0x01;
			color_bank = (data >> 2) & 1;
		return;

		case 0x500004:
		case 0x500005:
			// coin counter
		return;
	}
	return;
}

//------------------
// Z80 #0

void __fastcall sunaq_sound0_write(unsigned short address, unsigned char data)
{
	switch (address)
	{
		case 0xf800:
			BurnYM2151SelectRegister(data);
		return;

		case 0xf801:
			BurnYM2151WriteRegister(data);
		return;

		case 0xfc00:
			soundlatch2 = data;
		return;
	}

	return;
}


//----------------------------------------------------------------
// Ultra Balloon


//------------------
// 68k

unsigned short __fastcall uballoon_read_word(unsigned int address)
{
	if ((address & 0xfff000) == 0x200000) {
		if (address & 0x200) {
			return *((unsigned short*)(DrvPalRAM2 + (address & 0xffe)));
		} else {
			address += color_bank << 9;
			return *((unsigned short*)(DrvPalRAM + (address & 0xffe)));
		}
	}

	switch (address & ~1)
	{
		case 0x600000:
			return DrvInputs[0];

		case 0x600002:
			return DrvInputs[1];

		case 0x600004:
			return DrvInputs[2];

		case 0x600006:
			return DrvInputs[3];
	}

	return 0;
}

unsigned char __fastcall uballoon_read_byte(unsigned int address)
{
	if ((address & 0xfff000) == 0x200000) {
		if (address & 0x200) {
			return DrvPalRAM2[address & 0xffe];
		} else {
			address += color_bank << 9;
			return DrvPalRAM[address & 0xffe];
		}
	}

	switch (address)
	{
		case 0x600000:
		case 0x600001:
			return DrvInputs[0] >> ((~address & 1) << 3);

		case 0x600002:
		case 0x600003:
			return DrvInputs[1] >> ((~address & 1) << 3);

		case 0x600004:
		case 0x600005:
			return DrvInputs[2] >> ((~address & 1) << 3);

		case 0x600006:
		case 0x600007:
			return DrvInputs[3] >> ((~address & 1) << 3);
	}

	return 0;
}

void __fastcall uballoon_write_word(unsigned int address, unsigned short data)
{
	if ((address & 0xfff000) == 0x200000) {
		if (address & 0x200) {
			*((unsigned short*)(DrvPalRAM2 + (address & 0xffff))) = data;
		} else {
			address += color_bank << 9;
			*((unsigned short*)(DrvPalRAM + (address & 0xffff))) = data;
			suna_palette_write(address & 0xffff);
		}
		return;
	}

	switch (address & ~1)
	{
		case 0x600000:
			soundlatch = data;
		return;

		case 0x600004:
			flipscreen = data & 0x01;
			color_bank = (data >> 2) & 1;
		return;
	}
	return;
}

void __fastcall uballoon_write_byte(unsigned int address, unsigned char data)
{
	if ((address & 0xfff000) == 0x200000) {
		if (address & 0x200) {
			DrvPalRAM2[address & 0xfff] = data;
		} else {
			address += color_bank << 9;
			DrvPalRAM[address & 0xfff] = data;
			suna_palette_write(address & 0xffe);
		}
		return;
	}

	switch (address)
	{
		case 0x600000:
		case 0x600001:
			soundlatch = data;
		return;

		case 0x600004:
		case 0x600005:
			flipscreen = data & 0x01;
			color_bank = (data >> 2) & 1;
		return;
	}
	return;
}

//------------------
// Z80 #1

static void uballoon_bankswitch(int data)
{
	z80bankdata[0] = data;

	int bank = ((data & 1) << 16) | 0x400;

	ZetMapArea(0x0400, 0xffff, 0, DrvZ80ROM1 + bank);
	ZetMapArea(0x0400, 0xffff, 2, DrvZ80ROM1 + bank);
}

void __fastcall uballoon_sound1_out(unsigned short port, unsigned char data)
{
	switch (port & 0xff)
	{
		case 0x00:
		case 0x01:
			write_dac(data);
		return;

		case 0x03:
			uballoon_bankswitch(data);
		return;
	}

	return;
}

unsigned char __fastcall uballoon_sound1_in(unsigned short port)
{
	switch (port & 0xff)
	{
		case 0x00:
			return soundlatch2;
	}

	return 0;
}


//----------------------------------------------------------------
// Back Street Soccer


//------------------
// 68k

unsigned short __fastcall bssoccer_read_word(unsigned int address)
{
	if ((address & 0xfff000) == 0x400000) {
		if (address & 0x200) {
			return *((unsigned short*)(DrvPalRAM2 + (address & 0xffe)));
		} else {
			address += color_bank << 9;
			return *((unsigned short*)(DrvPalRAM + (address & 0xffe)));
		}
	}

	switch (address & ~1)
	{
		case 0xa00000:
			return DrvInputs[0];

		case 0xa00002:
			return DrvInputs[1];

		case 0xa00004:
			return DrvInputs[2];

		case 0xa00006:
			return DrvInputs[3];

		case 0xa00008:
			return DrvInputs[4];

		case 0xa0000a:
			return DrvInputs[5];

	}

	return 0;
}

unsigned char __fastcall bssoccer_read_byte(unsigned int address)
{
	if ((address & 0xfff000) == 0x400000) {
		if (address & 0x200) {
			return DrvPalRAM2[address & 0xffe];
		} else {
			address += color_bank << 9;
			return DrvPalRAM[address & 0xffe];
		}
	}

	switch (address)
	{
		case 0xa00000:
		case 0xa00001:
			return DrvInputs[0] >> ((~address & 1) << 3);

		case 0xa00002:
		case 0xa00003:
			return DrvInputs[1] >> ((~address & 1) << 3);

		case 0xa00004:
		case 0xa00005:
			return DrvInputs[2] >> ((~address & 1) << 3);

		case 0xa00006:
		case 0xa00007:
			return DrvInputs[3] >> ((~address & 1) << 3);

		case 0xa00008:
		case 0xa00009:
			return DrvInputs[4] >> ((~address & 1) << 3);

		case 0xa0000a:
		case 0xa0000b:
			return DrvInputs[5] >> ((~address & 1) << 3);
	}

	return 0;
}

void __fastcall bssoccer_write_word(unsigned int address, unsigned short data)
{
	if ((address & 0xfff000) == 0x400000) {
		if (address & 0x200) {
			*((unsigned short*)(DrvPalRAM2 + (address & 0xffff))) = data;
		} else {
			address += color_bank << 9;
			*((unsigned short*)(DrvPalRAM + (address & 0xffff))) = data;
			suna_palette_write(address & 0xffff);
		}
		return;
	}

	switch (address & ~1)
	{
		case 0xa00000:
			soundlatch = data;
		return;

		case 0xa00002:
			flipscreen = data & 0x01;
			color_bank = (data >> 2) & 1;
		return;
	}
	return;
}

void __fastcall bssoccer_write_byte(unsigned int address, unsigned char data)
{
	if ((address & 0xfff000) == 0x400000) {
		if (address & 0x200) {
			DrvPalRAM[address & 0xfff] = data;
		} else {
			address += color_bank << 9;
			DrvPalRAM[address & 0xfff] = data;
			suna_palette_write(address & 0xffe);
		}
		return;
	}

	switch (address)
	{
		case 0xa00000:
		case 0xa00001:
			soundlatch = data;
		return;

		case 0xa00002:
		case 0xa00003:
			flipscreen = data & 0x01;
			color_bank = (data >> 2) & 1;
		return;
	}
	return;
}

//------------------
// Z80 #0

static void bssoccer_bankswitch_w(unsigned char *z80data, int p, int data)
{
	z80bankdata[p] = data;

	int bank = ((data & 7) << 16) | 0x1000;

	ZetMapArea(0x1000, 0xffff, 0, z80data + bank);
	ZetMapArea(0x1000, 0xffff, 2, z80data + bank);
}

void __fastcall bssoccer_sound0_write(unsigned short address, unsigned char data)
{
	switch (address)
	{
		case 0xf800:
			BurnYM2151SelectRegister(data);
		return;

		case 0xf801:
			BurnYM2151WriteRegister(data);
		return;

		case 0xfd00:
			soundlatch2 = data;
		return;

		case 0xfe00:
			soundlatch3 = data;
		return;
	}

	return;
}

unsigned char __fastcall bssoccer_sound0_read(unsigned short address)
{
	switch (address)
	{
		case 0xf801:
			return BurnYM2151ReadStatus();

		case 0xfc00:
			return soundlatch;
	}

	return 0;
}

//------------------
// Z80 #1

void __fastcall bssoccer_sound1_out(unsigned short port, unsigned char data)
{
	switch (port & 0xff)
	{
		case 0x00:
		case 0x01:
			write_dac(data);
		return;

		case 0x03:
			bssoccer_bankswitch_w(DrvZ80ROM1, 0, data);
		return;
	}

	return;
}

unsigned char __fastcall bssoccer_sound1_in(unsigned short port)
{
	switch (port & 0xff)
	{
		case 0x00:
			return soundlatch2;
	}

	return 0;
}

//------------------
// Z80 #2

void __fastcall bssoccer_sound2_out(unsigned short port, unsigned char data)
{
	switch (port & 0xff)
	{
		case 0x00:
		case 0x01:
			write_dac(data);
		return;

		case 0x03:
			bssoccer_bankswitch_w(DrvZ80ROM2, 1, data);
		return;
	}

	return;
}

unsigned char __fastcall bssoccer_sound2_in(unsigned short port)
{
	switch (port & 0xff)
	{
		case 0x00:
			return soundlatch3;
	}

	return 0;
}


//-------------------------------------------------------------------------------------------------
// Initialization routines

static int DrvDoReset()
{
	DrvReset = 0;

	memset (AllRam, 0, RamEnd - AllRam);

	SekOpen(0);
	SekReset();
	SekClose();

	for (int j = 0; j < 3; j++) {
		ZetOpen(j);
		ZetReset();
		ZetClose();
	}

	soundlatch = 0;
	soundlatch2 = 0;
	soundlatch3 = 0;
	color_bank = 0;
	bestofbest_prot = 0;
	flipscreen = 0;

	z80bankdata[0] = z80bankdata[1] = 0;

	if (game_select == 3) {
		ZetOpen(1);
		bssoccer_bankswitch_w(DrvZ80ROM1, 0, z80bankdata[0]);
		ZetClose();
		ZetOpen(2);
		bssoccer_bankswitch_w(DrvZ80ROM2, 1, z80bankdata[1]);
		ZetClose();
	}

	if (game_select == 2) {
		ZetOpen(0);
		uballoon_bankswitch(z80bankdata[0]);
		ZetClose();
	}

	if (game_select) {
		BurnYM2151Reset();
	} else {
		BurnYM3526Reset();
		AY8910Reset(0);
	}

	DACReset();

	return 0;
}


static int MemIndex()
{
	unsigned char *Next; Next = AllMem;

	Drv68KROM	= Next; Next += 0x0200000;
	DrvZ80ROM0	= Next; Next += 0x0010000;
	DrvZ80ROM1	= Next; Next += 0x0080000;
	DrvZ80ROM2	= Next; Next += 0x0080000;

	DrvGfxROM0	= Next; Next += 0x0600000;
	if (game_select == 0) {
		DrvGfxROM1	= Next; Next += 0x0800000;
	}

	DrvPalette	= (unsigned int*)Next; Next += 0x01000 * sizeof(int);

	pFMBuffer	= (short*)Next; Next += nBurnSoundLen * 3 * sizeof(short);

	AllRam		= Next;

	Drv68KRAM	= Next; Next += 0x0010000;

	DrvZ80RAM0	= Next; Next += 0x0000800;

	DrvSprRAM0	= Next; Next += 0x0020000;
	DrvSprRAM1	= Next; Next += 0x0020000;

	DrvPalRAM	= Next; Next += 0x0001000;
	DrvPalRAM2	= Next; Next += 0x0010000;

	Palette		= (unsigned int*)Next; Next += 0x01000 * sizeof(int);

	RamEnd		= Next;

	MemEnd		= Next;

	return 0;
}

static int DrvGfxDecode(unsigned char *gfx_base, int len)
{
	int Plane[4] = {(len << 2) + 0, (len << 2) + 4, 0, 4 };
	int XOffs[8] = {  3,  2,  1,  0, 11, 10,  9,  8 };
	int YOffs[8] = {  0, 16, 32, 48, 64, 80, 96, 112 };

	unsigned char *tmp = (unsigned char*)malloc(len);
	if (tmp == NULL) {
		return 1;
	}

	for (int i = 0; i < len; i++) tmp[i] = gfx_base[i] ^ 0xff; // copy & invert

	GfxDecode(((len * 8) / 4) / 64, 4, 8, 8, Plane, XOffs, YOffs, 0x80, tmp, gfx_base);

	free (tmp);

	return 0;
}

static int DrvLoadRoms()
{
	char* pRomName;
	struct BurnRomInfo ri;

	unsigned char *Load68K = Drv68KROM;
	unsigned char *Loadz0  = DrvZ80ROM0;
	unsigned char *Loadz1  = DrvZ80ROM1;
	unsigned char *Loadz2  = DrvZ80ROM2;
	unsigned char *Loadg0  = DrvGfxROM0;
	unsigned char *Loadg1  = DrvGfxROM1;

	int gfx0_len = 0;
	int gfx1_len = 0;

	for (int i = 0; !BurnDrvGetRomName(&pRomName, i, 0); i++) {

		BurnDrvGetRomInfo(&ri, i);

		if ((ri.nType & 7) == 1) {

			if (BurnLoadRom(Load68K + 1, i + 0, 2)) return 1;
			if (BurnLoadRom(Load68K + 0, i + 1, 2)) return 1;

			Load68K += 0x100000;

			i++;

			continue;
		}

		if ((ri.nType & 7) == 2) {
			if (BurnLoadRom(Loadz0, i, 1)) return 1;
			continue;
		}

		if ((ri.nType & 7) == 3) {
			if (BurnLoadRom(Loadz1, i, 1)) return 1;
			continue;
		}

		if ((ri.nType & 7) == 4) {
			if (BurnLoadRom(Loadz2, i, 1)) return 1;
			continue;
		}

		if ((ri.nType & 7) == 5) {
			if (BurnLoadRom(Loadg0, i, 1)) return 1;

			Loadg0 += ri.nLen;
			gfx0_len += ri.nLen;

			continue;
		}

		if ((ri.nType & 7) == 6) {
			if (BurnLoadRom(Loadg1, i, 1)) return 1;

			Loadg1 += ri.nLen;
			gfx1_len += ri.nLen;

			continue;
		}		
	}

	nGfxROM0Len = gfx0_len >> 5;

	if (gfx0_len) DrvGfxDecode(DrvGfxROM0, gfx0_len);
	if (gfx1_len) DrvGfxDecode(DrvGfxROM1, gfx1_len);

	return 0;
}

void bestbest_ay8910_write_a(unsigned int,unsigned int)
{
}

static void bestbestFMIRQHandler(int, int nStatus)
{
	if (nStatus) {
		ZetSetIRQLine(0xFF, ZET_IRQSTATUS_ACK);
	} else {
		ZetSetIRQLine(0,    ZET_IRQSTATUS_NONE);
	}
}

static int bestbestSynchroniseStream(int nSoundRate)
{
	return (long long)ZetTotalCycles() * nSoundRate / 6000000;
}

static int BestbestInit()
{
	int nLen;

	game_select = 0;

	AllMem = NULL;
	MemIndex();
	nLen = MemEnd - (unsigned char *)0;
	if ((AllMem = (unsigned char *)malloc(nLen)) == NULL) return 1;
	memset(AllMem, 0, nLen);
	MemIndex();

	for (int i = 0; i < 3; i++) {
		pAY8910Buffer[i] = pFMBuffer + nBurnSoundLen * i;
	}

	if (DrvLoadRoms()) return 1;

	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM,		   0x000000, 0x03ffff, SM_ROM);
	SekMapMemory(Drv68KROM,		   0x040000, 0x07ffff, SM_ROM);
	SekMapMemory(Drv68KROM,		   0x080000, 0x0bffff, SM_ROM);
	SekMapMemory(Drv68KROM,		   0x0c0000, 0x0fffff, SM_ROM);
	SekMapMemory(Drv68KROM + 0x100000, 0x200000, 0x2fffff, SM_ROM);
	SekMapMemory(DrvPalRAM, 	   0x540000, 0x540fff, SM_ROM);
	SekMapMemory(DrvPalRAM2,	   0x541000, 0x54ffff, SM_RAM);
	SekMapMemory(Drv68KRAM,		   0x580000, 0x58ffff, SM_RAM);
	SekMapMemory(DrvSprRAM0,	   0x5c0000, 0x5dffff, SM_RAM);
	SekMapMemory(DrvSprRAM1,	   0x5e0000, 0x5fffff, SM_RAM);
	SekSetWriteByteHandler(0,	   bestbest_write_byte);
	SekSetWriteWordHandler(0,	   bestbest_write_word);
	SekSetReadByteHandler(0,	   bestbest_read_byte);
	SekSetReadWordHandler(0,	   bestbest_read_word);
	SekClose();

	ZetInit(3); // actually 2
	ZetOpen(0);
	ZetMapArea(0x0000, 0xbfff, 0, DrvZ80ROM0);
	ZetMapArea(0x0000, 0xbfff, 2, DrvZ80ROM0);
	ZetMapArea(0xe000, 0xe7ff, 0, DrvZ80RAM0);
	ZetMapArea(0xe000, 0xe7ff, 1, DrvZ80RAM0);
	ZetMapArea(0xe000, 0xe7ff, 2, DrvZ80RAM0);
	ZetSetReadHandler(bestbest_sound0_read);
	ZetSetWriteHandler(bestbest_sound0_write);
	ZetMemEnd();
	ZetClose();

	ZetOpen(1);
	ZetMapArea(0x0000, 0xffff, 0, DrvZ80ROM1);
	ZetMapArea(0x0000, 0xffff, 2, DrvZ80ROM1);
	ZetSetInHandler(bestbest_sound1_in);
	ZetSetOutHandler(bestbest_sound1_out);
	ZetClose();

	BurnYM3526Init(3000000, &bestbestFMIRQHandler, &bestbestSynchroniseStream, 0);
	BurnTimerAttachZetYM3526(6000000);
	
	AY8910Init(0, 1500000, nBurnSoundRate, NULL, NULL, bestbest_ay8910_write_a, NULL);

	DACInit(0, 1);

	DrvDoReset();

	GenericTilesInit();

	return 0;
}

static int SunaqInit()
{
	int nLen;

	game_select = 1;

	AllMem = NULL;
	MemIndex();
	nLen = MemEnd - (unsigned char *)0;
	if ((AllMem = (unsigned char *)malloc(nLen)) == NULL) return 1;
	memset(AllMem, 0, nLen);
	MemIndex();

	if (DrvLoadRoms()) return 1;

	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM,		   0x000000, 0x0fffff, SM_ROM);
	SekMapMemory(DrvPalRAM2,	   0x540400, 0x540fff, SM_RAM);
	SekMapMemory(Drv68KRAM,		   0x580000, 0x583fff, SM_RAM);
	SekMapMemory(DrvSprRAM0,	   0x5c0000, 0x5dffff, SM_RAM);
	SekSetWriteByteHandler(0,	   sunaq_write_byte);
	SekSetWriteWordHandler(0,	   sunaq_write_word);
	SekSetReadByteHandler(0,	   sunaq_read_byte);
	SekSetReadWordHandler(0,	   sunaq_read_word);
	SekClose();

	ZetInit(3); // actually 2
	ZetOpen(0);
	ZetMapArea(0x0000, 0xefff, 0, DrvZ80ROM0);
	ZetMapArea(0x0000, 0xefff, 2, DrvZ80ROM0);
	ZetMapArea(0xf000, 0xf7ff, 0, DrvZ80RAM0);
	ZetMapArea(0xf000, 0xf7ff, 1, DrvZ80RAM0);
	ZetMapArea(0xf000, 0xf7ff, 2, DrvZ80RAM0);
	ZetSetWriteHandler(sunaq_sound0_write);
	ZetSetReadHandler(bssoccer_sound0_read);
	ZetMemEnd();
	ZetClose();

	ZetOpen(1);
	ZetMapArea(0x0000, 0x0fff, 0, DrvZ80ROM1);
	ZetMapArea(0x0000, 0x0fff, 2, DrvZ80ROM1);
	ZetMapArea(0x1000, 0xffff, 0, DrvZ80ROM1 + 0x1000);
	ZetMapArea(0x1000, 0xffff, 2, DrvZ80ROM1 + 0x1000);
	ZetSetInHandler(bssoccer_sound1_in);
	ZetSetOutHandler(bssoccer_sound1_out);
	ZetMemEnd();
	ZetClose();

	BurnYM2151Init(3579545, 25.0);

	DACInit(0, 1);

	DrvDoReset();

	GenericTilesInit();

	return 0;
}


static int UballoonInit()
{
	int nLen;

	game_select = 2;

	AllMem = NULL;
	MemIndex();
	nLen = MemEnd - (unsigned char *)0;
	if ((AllMem = (unsigned char *)malloc(nLen)) == NULL) return 1;
	memset(AllMem, 0, nLen);
	MemIndex();

	if (DrvLoadRoms()) return 1;

	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM,		   0x000000, 0x0fffff, SM_ROM);
	SekMapMemory(DrvPalRAM2,	   0x200400, 0x200fff, SM_RAM);
	SekMapMemory(DrvSprRAM0,	   0x400000, 0x41ffff, SM_RAM);
	SekMapMemory(DrvSprRAM0,	   0x5c0000, 0x5dffff, SM_RAM);
	SekMapMemory(Drv68KRAM,	   	   0x800000, 0x803fff, SM_RAM);
	SekSetWriteByteHandler(0,	   uballoon_write_byte);
	SekSetWriteWordHandler(0,	   uballoon_write_word);
	SekSetReadByteHandler(0,	   uballoon_read_byte);
	SekSetReadWordHandler(0,	   uballoon_read_word);
	SekClose();

	ZetInit(3); // actually 2
	ZetOpen(0);
	ZetMapArea(0x0000, 0xefff, 0, DrvZ80ROM0);
	ZetMapArea(0x0000, 0xefff, 2, DrvZ80ROM0);
	ZetMapArea(0xf000, 0xf7ff, 0, DrvZ80RAM0);
	ZetMapArea(0xf000, 0xf7ff, 1, DrvZ80RAM0);
	ZetMapArea(0xf000, 0xf7ff, 2, DrvZ80RAM0);
	ZetSetWriteHandler(sunaq_sound0_write);
	ZetSetReadHandler(bssoccer_sound0_read);
	ZetMemEnd();
	ZetClose();

	ZetOpen(1);
	ZetMapArea(0x0000, 0x03ff, 0, DrvZ80ROM1);
	ZetMapArea(0x0000, 0x03ff, 2, DrvZ80ROM1);
	ZetMapArea(0x0400, 0xffff, 0, DrvZ80ROM1 + 0x400);
	ZetMapArea(0x0400, 0xffff, 2, DrvZ80ROM1 + 0x400);
	ZetSetInHandler(uballoon_sound1_in);
	ZetSetOutHandler(uballoon_sound1_out);
	ZetMemEnd();
	ZetClose();

	// Patch out the protection checks
	*((unsigned short*)(Drv68KROM + 0x0113c)) = 0x4e71;
	*((unsigned short*)(Drv68KROM + 0x0113e)) = 0x4e71;
	*((unsigned short*)(Drv68KROM + 0x01784)) = 0x600c;
	*((unsigned short*)(Drv68KROM + 0x018e2)) = 0x600c;
	*((unsigned short*)(Drv68KROM + 0x03c54)) = 0x600c;
	*((unsigned short*)(Drv68KROM + 0x126a0)) = 0x4e71;

	BurnYM2151Init(3579545, 25.0);

	DACInit(0, 1);

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static int BssoccerInit()
{
	int nLen;

	game_select = 3;

	AllMem = NULL;
	MemIndex();
	nLen = MemEnd - (unsigned char *)0;
	if ((AllMem = (unsigned char *)malloc(nLen)) == NULL) return 1;
	memset(AllMem, 0, nLen);
	MemIndex();

	if (DrvLoadRoms()) return 1;

	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM,		   0x000000, 0x1fffff, SM_ROM);
	SekMapMemory(Drv68KRAM,		   0x200000, 0x203fff, SM_RAM);
	SekMapMemory(DrvPalRAM2,	   0x400400, 0x400fff, SM_RAM);
	SekMapMemory(DrvSprRAM0,	   0x600000, 0x61ffff, SM_RAM);
	SekSetWriteByteHandler(0,	   bssoccer_write_byte);
	SekSetWriteWordHandler(0,	   bssoccer_write_word);
	SekSetReadByteHandler(0,	   bssoccer_read_byte);
	SekSetReadWordHandler(0,	   bssoccer_read_word);
	SekClose();

	ZetInit(3);
	ZetOpen(0);
	ZetMapArea(0x0000, 0x7fff, 0, DrvZ80ROM0);
	ZetMapArea(0x0000, 0x7fff, 2, DrvZ80ROM0);
	ZetMapArea(0xf000, 0xf7ff, 0, DrvZ80RAM0);
	ZetMapArea(0xf000, 0xf7ff, 1, DrvZ80RAM0);
	ZetMapArea(0xf000, 0xf7ff, 2, DrvZ80RAM0);
	ZetSetWriteHandler(bssoccer_sound0_write);
	ZetSetReadHandler(bssoccer_sound0_read);
	ZetMemEnd();
	ZetClose();

	ZetOpen(1);
	ZetMapArea(0x0000, 0x0fff, 0, DrvZ80ROM1);
	ZetMapArea(0x0000, 0x0fff, 2, DrvZ80ROM1);
	ZetMapArea(0x1000, 0xffff, 0, DrvZ80ROM1 + 0x1000);
	ZetMapArea(0x1000, 0xffff, 2, DrvZ80ROM1 + 0x1000);
	ZetSetInHandler(bssoccer_sound1_in);
	ZetSetOutHandler(bssoccer_sound1_out);
	ZetMemEnd();
	ZetClose();

	ZetOpen(2);
	ZetMapArea(0x0000, 0x0fff, 0, DrvZ80ROM2);
	ZetMapArea(0x0000, 0x0fff, 2, DrvZ80ROM2);
	ZetMapArea(0x1000, 0xffff, 0, DrvZ80ROM2 + 0x1000);
	ZetMapArea(0x1000, 0xffff, 2, DrvZ80ROM2 + 0x1000);
	ZetSetInHandler(bssoccer_sound2_in);
	ZetSetOutHandler(bssoccer_sound2_out);
	ZetMemEnd();
	ZetClose();

	BurnYM2151Init(3579545, 25.0);

	DACInit(0, 1);

	DrvDoReset();

	GenericTilesInit();

	return 0;
}

static int DrvExit()
{
	free (AllMem);
	AllMem = NULL;

	if (game_select) {
		BurnYM2151Exit();
	} else {
		AY8910Exit(0);
	}

	DACExit();

	SekExit();
	ZetExit();

	GenericTilesExit();

	return 0;
}


//-------------------------------------------------------------------------------------------------
// Drawing & CPU/sound emulation routines


static void draw_sprites(unsigned short *sprites, unsigned char *gfx_base, int max_tile)
{
	int offs;

	int max_x = (256 - 0) - 8;
	int max_y = (256 - 0) - 8;

	for (offs = 0xfc00/2; offs < 0x10000/2 ; offs += 4/2)
	{
		int srcpg, srcx,srcy, dimx,dimy;
		int tile_x, tile_xinc, tile_xstart;
		int tile_y, tile_yinc;
		int dx, dy;
		int flipx, y0;

		int y		=	sprites[ offs + 0 + 0x00000 / 2 ];
		int x		=	sprites[ offs + 1 + 0x00000 / 2 ];
		int dim 	=	sprites[ offs + 0 + 0x10000 / 2 ];

		int bank	=	(x >> 12) & 0xf;

		srcpg	=	((y & 0xf000) >> 12) + ((x & 0x0200) >> 5);
		srcx	=	((y   >> 8) & 0xf) * 2;
		srcy	=	((dim >> 0) & 0xf) * 2;

		switch ( (dim >> 4) & 0xc )
		{
			case 0x0:	dimx = 2;	dimy =	2;	y0 = 0x100; break;
			case 0x4:	dimx = 4;	dimy =	4;	y0 = 0x100; break;
			case 0x8:	dimx = 2;	dimy = 32;	y0 = 0x130; break;
			default:
			case 0xc:	dimx = 4;	dimy = 32;	y0 = 0x120; break;
		}

		if (dimx==4)	{ flipx = srcx & 2; 	srcx &= ~2; }
		else		{ flipx = 0; }

		x = (x & 0xff) - (x & 0x100);
		y = (y0 - (y & 0xff) - dimy*8 ) & 0xff;

		if (flipx)	{ tile_xstart = dimx-1; tile_xinc = -1; }
		else		{ tile_xstart = 0;		tile_xinc = +1; }

		tile_y = 0; 	tile_yinc = +1;

		for (dy = 0; dy < dimy * 8; dy += 8)
		{
			tile_x = tile_xstart;

			for (dx = 0; dx < dimx * 8; dx += 8)
			{
				int addr	=	(srcpg * 0x20 * 0x20) +
								((srcx + tile_x) & 0x1f) * 0x20 +
								((srcy + tile_y) & 0x1f);

				int tile	=	sprites[ addr + 0x00000 / 2 ];
				int color	=	sprites[ addr + 0x10000 / 2 ];

				int sx		=	x + dx;
				int sy		=	(y + dy) & 0xff;

				int tile_flipx	=	tile & 0x4000;
				int tile_flipy	=	tile & 0x8000;

				if (flipx)	tile_flipx ^= 0x4000;

				if (flipscreen)
				{
					sx = max_x - sx;
					sy = max_y - sy;
					tile_flipx ^= 0x4000;
					tile_flipy ^= 0x8000;
				}

				tile   = (tile & 0x3fff) | (bank << 14);
				color += (color_bank << 4);
				color &= 0x7f;
				tile  %= max_tile;

				sy -= 16;

				tile_x += tile_xinc;

				if (sy < -15 || sy > (nScreenHeight - 1) || sx < -15 || sx > (nScreenWidth - 1)) {
					continue; 
				}

				if (tile_flipy) {
					if (tile_flipx) {
						Render8x8Tile_Mask_FlipXY_Clip(pTransDraw, tile, sx, sy, color, 4, 0x0f, 0, gfx_base);
					} else {
						Render8x8Tile_Mask_FlipY_Clip(pTransDraw, tile, sx, sy, color, 4, 0x0f, 0, gfx_base);
					}
				} else {
					if (tile_flipx) {
						Render8x8Tile_Mask_FlipX_Clip(pTransDraw, tile, sx, sy, color, 4, 0x0f, 0, gfx_base);
					} else {
						Render8x8Tile_Mask_Clip(pTransDraw, tile, sx, sy, color, 4, 0x0f, 0, gfx_base);
					}
				}
			}

			tile_y += tile_yinc;
		}

	}

}

static int DrvDraw()
{
	if (DrvRecalc) {
		for (int i = 0; i < 0x1000; i++) {
			int rgb = Palette[i];
			DrvPalette[i] = BurnHighCol(rgb >> 16, rgb >> 8, rgb, 0);
		}
	}

	for (int i = 0; i < nScreenWidth * nScreenHeight; i++) {
		pTransDraw[i] = 0xff;
	}

	draw_sprites((unsigned short*)DrvSprRAM0, DrvGfxROM0, nGfxROM0Len);
	if (!game_select) {
		draw_sprites((unsigned short*)DrvSprRAM1, DrvGfxROM1, 0x20000);
	}

	BurnTransferCopy(DrvPalette);

	return 0;
}

static inline void AssembleInputs()
{
	memset (DrvInputs, 0xff, 6 * sizeof(short));

	for (int i = 0; i < 16; i++) {
		DrvInputs[0] ^= DrvJoy1[i] << i;
		DrvInputs[1] ^= DrvJoy2[i] << i;
		DrvInputs[2] ^= DrvJoy3[i] << i;
		DrvInputs[3] ^= DrvJoy4[i] << i;
		DrvInputs[4] ^= DrvJoy5[i] << i;
		DrvInputs[5] ^= DrvJoy6[i] << i;
	}

	switch (game_select)
	{
		case 0: // bestbest
		{
			DrvInputs[2] = (DrvDips[1] << 8) | DrvDips[0];
		}
		return;

		case 1: // sunaq
		{
			DrvInputs[2] = DrvDips[0];
		}
		return;

		case 2: // uballoon
		{
			DrvInputs[1] = (DrvInputs[1] & 0xff) | (DrvDips[0] << 8);
			DrvInputs[2] = DrvDips[1];
			DrvInputs[3] = DrvDips[2];
		}
		return;

		case 3: // bssoccer
		{
			DrvInputs[4] = (DrvDips[1] << 8) | DrvDips[0];
			DrvInputs[5] = ((DrvInputs[5] & 0xfe) | (DrvDips[2] & 0x01)) | 0xff00;
		}
		return;
	}

	return;
}

static int BestbestFrame()
{
	int nCyclesTotal[3];

	int nInterleave = 100;

	if (DrvReset) {
		DrvDoReset();
	}

	AssembleInputs();

	nCyclesTotal[0] = 6000000 / 60;
	nCyclesTotal[1] = 6000000 / 60;
	nCyclesTotal[2] = 6000000 / 60;

	SekNewFrame();
	ZetNewFrame();
	
	SekOpen(0);
	for (int i = 0; i < nInterleave; i++) {

		SekRun(nCyclesTotal[0] / 2);
		if (i == (nInterleave / 2)-1) SekSetIRQLine(1, SEK_IRQSTATUS_AUTO);
		if (i == (nInterleave    )-1) SekSetIRQLine(2, SEK_IRQSTATUS_AUTO);

		ZetOpen(0);
		BurnTimerUpdateYM3526(nCyclesTotal[1] / nInterleave);
		ZetClose();
		
		ZetOpen(1);
		ZetRun(nCyclesTotal[2] / nInterleave);
		ZetClose();
	}
	SekClose();
	
	ZetOpen(0);
	BurnTimerEndFrameYM3526(nCyclesTotal[1]);
	BurnYM3526Update(pBurnSoundOut, nBurnSoundLen);
	ZetClose();

	if (pBurnSoundOut) {
		int nSample;

		AY8910Update(0, &pAY8910Buffer[0], nBurnSoundLen);
		for (int n = 0; n < nBurnSoundLen; n++) {
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

			pBurnSoundOut[(n << 1) + 0] += nSample;
			pBurnSoundOut[(n << 1) + 1] += nSample;
		}

		DACUpdate(pBurnSoundOut, nBurnSoundLen);
	}

	if (pBurnDraw) {
		DrvDraw();
	}

	return 0;
}

static int SunaqFrame()
{
	int nInterleave = nBurnSoundLen ? nBurnSoundLen : 10;
	int nSoundBufferPos = 0;
	int nCyclesTotal[3];

	if (DrvReset) {
		DrvDoReset();
	}

	AssembleInputs();

	nCyclesTotal[0] = 6000000 / 60;
	nCyclesTotal[1] = 3579500 / 60;
	nCyclesTotal[2] = 6000000 / 60;

	SekOpen(0);

	for (int i = 0; i < nInterleave; i++)
	{
		SekRun(nCyclesTotal[0] / nInterleave);
		if (i == (nInterleave    )-1) SekSetIRQLine(1, SEK_IRQSTATUS_AUTO);

		for (int j = 0; j < 2; j++) {
			ZetOpen(j);
			ZetRun(nCyclesTotal[j+1] / nInterleave);
			ZetClose();
		}

		if (pBurnSoundOut) {
			int nSegmentLength = nBurnSoundLen / nInterleave;
			short* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
			BurnYM2151Render(pSoundBuf, nSegmentLength);
			DACUpdate(pSoundBuf, nSegmentLength);

			nSoundBufferPos += nSegmentLength;
		}
	}

	// Make sure the buffer is entirely filled.
	if (pBurnSoundOut) {
		int nSegmentLength = nBurnSoundLen - nSoundBufferPos;
		short* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);

		if (nSegmentLength) {
			BurnYM2151Render(pSoundBuf, nSegmentLength);
			DACUpdate(pSoundBuf, nSegmentLength);
		}
	}

	SekClose();

	if (pBurnDraw) {
		DrvDraw();
	}

	return 0;
}

static int UballoonFrame()
{
	int nInterleave = nBurnSoundLen ? nBurnSoundLen : 10;
	int nSoundBufferPos = 0;
	int nCyclesTotal[3];

	if (DrvReset) {
		DrvDoReset();
	}

	AssembleInputs();

	nCyclesTotal[0] = 8000000 / 60;
	nCyclesTotal[1] = 3579500 / 60;
	nCyclesTotal[2] = 5000000 / 60;

	SekOpen(0);

	for (int i = 0; i < nInterleave; i++)
	{
		SekRun(nCyclesTotal[0] / nInterleave);
		if (i == (nInterleave    )-1) SekSetIRQLine(1, SEK_IRQSTATUS_AUTO);

		for (int j = 0; j < 2; j++) {
			ZetOpen(j);
			ZetRun(nCyclesTotal[j+1] / nInterleave);
			ZetClose();
		}

		if (pBurnSoundOut) {
			int nSegmentLength = nBurnSoundLen / nInterleave;
			short* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
			BurnYM2151Render(pSoundBuf, nSegmentLength);
			DACUpdate(pSoundBuf, nSegmentLength);

			nSoundBufferPos += nSegmentLength;
		}
	}

	// Make sure the buffer is entirely filled.
	if (pBurnSoundOut) {
		int nSegmentLength = nBurnSoundLen - nSoundBufferPos;
		short* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);

		if (nSegmentLength) {
			BurnYM2151Render(pSoundBuf, nSegmentLength);
			DACUpdate(pSoundBuf, nSegmentLength);
		}
	}

	SekClose();

	if (pBurnDraw) {
		DrvDraw();
	}

	return 0;
}


static int BssoccerFrame()
{
	int nInterleave = nBurnSoundLen ? nBurnSoundLen : 10;
	int nSoundBufferPos = 0;
	int nCyclesTotal[4];

	if (DrvReset) {
		DrvDoReset();
	}

	AssembleInputs();

	nCyclesTotal[0] = 8000000 / 60;
	nCyclesTotal[1] = 3579500 / 60;
	nCyclesTotal[2] = 5000000 / 60;
	nCyclesTotal[3] = 5000000 / 60;

	SekOpen(0);

	for (int i = 0; i < nInterleave; i++)
	{
		SekRun(nCyclesTotal[0] / nInterleave);
		if (i == (nInterleave / 2)-1) SekSetIRQLine(2, SEK_IRQSTATUS_AUTO);
		if (i == (nInterleave    )-1) SekSetIRQLine(1, SEK_IRQSTATUS_AUTO);

		for (int j = 0; j < 3; j++) {
			ZetOpen(j);
			ZetRun(nCyclesTotal[j+1] / nInterleave);
			ZetClose();
		}

		if (pBurnSoundOut) {
			int nSegmentLength = nBurnSoundLen / nInterleave;
			short* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
			BurnYM2151Render(pSoundBuf, nSegmentLength);
			DACUpdate(pSoundBuf, nSegmentLength);
			nSoundBufferPos += nSegmentLength;
		}
	}

	// Make sure the buffer is entirely filled.
	if (pBurnSoundOut) {
		int nSegmentLength = nBurnSoundLen - nSoundBufferPos;
		short* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);

		if (nSegmentLength) {
			BurnYM2151Render(pSoundBuf, nSegmentLength);
			DACUpdate(pSoundBuf, nSegmentLength);
		}
	}

	SekClose();

	if (pBurnDraw) {
		DrvDraw();
	}

	return 0;
}


static int DrvScan(int nAction,int *pnMin)
{
	struct BurnArea ba;

	if (pnMin) {
		*pnMin = 0x029692;
	}

	if (nAction & ACB_VOLATILE) {		
		memset(&ba, 0, sizeof(ba));

		ba.Data	  = AllRam;
		ba.nLen	  = RamEnd - AllRam;
		ba.szName = "All Ram";
		BurnAcb(&ba);

		SekScan(nAction);
		ZetScan(nAction);

		DACScan(nAction, pnMin);

		if (game_select) {
			BurnYM2151Scan(nAction);
		} else {
			AY8910Scan(nAction, pnMin);
		}

		SCAN_VAR(soundlatch);
		SCAN_VAR(soundlatch2);
		SCAN_VAR(soundlatch3);
		SCAN_VAR(flipscreen);
		SCAN_VAR(color_bank);
		SCAN_VAR(bestofbest_prot);

		SCAN_VAR(z80bankdata[0]);
		SCAN_VAR(z80bankdata[1]);
	}

	if (game_select == 3) {
		ZetOpen(1);
		bssoccer_bankswitch_w(DrvZ80ROM1, 0, z80bankdata[0]);
		ZetClose();
		ZetOpen(2);
		bssoccer_bankswitch_w(DrvZ80ROM2, 1, z80bankdata[1]);
		ZetClose();
	}

	if (game_select == 2) {
		ZetOpen(1);
		uballoon_bankswitch(z80bankdata[0]);
		ZetClose();
	}

	return 0;
}



//-------------------------------------------------------------------------------------------------
// Drivers


// Best Of Best

static struct BurnRomInfo bestbestRomDesc[] = {
	{ "4.bin",	0x20000, 0x06741994, 1 | BRF_ESS | BRF_PRG },   //  0 - 68K Code
	{ "2.bin",	0x20000, 0x42843dec, 1 | BRF_ESS | BRF_PRG },   //  1
	{ "3.bin",	0x80000, 0xe2bb8f26, 1 | BRF_ESS | BRF_PRG },   //  2
	{ "1.bin",	0x80000, 0xd365e20a, 1 | BRF_ESS | BRF_PRG },   //  3
 
	{ "5.bin",	0x10000, 0xbb9265e6, 2 | BRF_ESS | BRF_PRG },   //  4 - Z80 #0 Code

	{ "6.bin",	0x10000, 0xdd445f6b, 3 | BRF_ESS | BRF_PRG },   //  5 - Z80 #1 Code

	{ "9.bin",	0x80000, 0xb11994ea, 5 | BRF_GRA },		//  6 - Sprites (Chip 0)
	{ "10.bin",	0x80000, 0x37b41ef5, 5 | BRF_GRA },		//  7
	{ "7.bin",	0x80000, 0x16188b73, 5 | BRF_GRA },		//  8
	{ "8.bin",	0x80000, 0x765ce06b, 5 | BRF_GRA },		//  9

	{ "16.bin",	0x80000, 0xdc46cdea, 6 | BRF_GRA },		// 10 - Sprites (Chip 1)
	{ "17.bin",	0x80000, 0xc6fadd57, 6 | BRF_GRA },		// 11
	{ "13.bin",	0x80000, 0x23283ac4, 6 | BRF_GRA },		// 12
	{ "18.bin",	0x80000, 0x674c4609, 6 | BRF_GRA },		// 13
	{ "14.bin",	0x80000, 0xc210fb53, 6 | BRF_GRA },		// 14
	{ "15.bin",	0x80000, 0x3b1166c7, 6 | BRF_GRA },		// 15
	{ "11.bin",	0x80000, 0x323eebc3, 6 | BRF_GRA },		// 16
	{ "12.bin",	0x80000, 0xca7c8176, 6 | BRF_GRA },		// 17

	{ "82s129.5",	0x00100, 0x10bfcebb, 0 | BRF_OPT },		// 18 - PROMs (not used)
	{ "82s129.6",	0x00100, 0x10bfcebb, 0 | BRF_OPT },		// 19
};

STD_ROM_PICK(bestbest)
STD_ROM_FN(bestbest)

struct BurnDriver BurnDrvBestbest = {
	"bestbest", NULL, NULL, NULL, "1994",
	"Best Of Best\0", NULL, "SunA", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_VSFIGHT, 0,
	NULL, bestbestRomInfo, bestbestRomName, NULL, NULL, BestbestInputInfo, bestbestDIPInfo,
	BestbestInit, DrvExit, BestbestFrame, DrvDraw, DrvScan, &DrvRecalc, 0x1000,
	256, 224, 4, 3
};


// SunA Quiz 6000 Academy (940620-6)

static struct BurnRomInfo sunaqRomDesc[] = {
	{ "prog2.bin", 	0x80000, 0xa92bce45, 1 | BRF_ESS | BRF_PRG },   //  0 - 68K Code
	{ "prog1.bin",	0x80000, 0xff690e7e, 1 | BRF_ESS | BRF_PRG },   //  1

	{ "audio1.bin",	0x10000, 0x3df42f82, 2 | BRF_ESS | BRF_PRG },   //  2 - Z80 #0 Code

	{ "audio2.bin",	0x80000, 0xcac85ba9, 3 | BRF_ESS | BRF_PRG },   //  3 - Z80 #1 Code

	{ "gfx1.bin",	0x80000, 0x0bde5acf, 5 | BRF_GRA },		//  4 - Sprites 
	{ "gfx2.bin",	0x80000, 0x24b74826, 5 | BRF_GRA },		//  5
};

STD_ROM_PICK(sunaq)
STD_ROM_FN(sunaq)

struct BurnDriver BurnDrvSunaq = {
	"sunaq", NULL, NULL, NULL, "1994",
	"SunA Quiz 6000 Academy (940620-6)\0", NULL, "SunA", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_QUIZ, 0,
	NULL, sunaqRomInfo, sunaqRomName, NULL, NULL, SunaqInputInfo, sunaqDIPInfo,
	SunaqInit, DrvExit, SunaqFrame, DrvDraw, DrvScan, &DrvRecalc, 0x1000,
	256, 224, 4, 3
};


// Ultra Balloon

static struct BurnRomInfo uballoonRomDesc[] = {
	{ "prg2.rom",	0x80000, 0x72ab80ea, 1 | BRF_ESS | BRF_PRG },   //  0 - 68K Code
	{ "prg1.rom",	0x80000, 0x27a04f55, 1 | BRF_ESS | BRF_PRG },   //  1

	{ "audio1.rom",	0x10000, 0xc771f2b4, 2 | BRF_ESS | BRF_PRG },   //  2 - Z80 #0 Code

	{ "audio2.rom",	0x20000, 0xc7f75347, 3 | BRF_ESS | BRF_PRG },   //  3 - Z80 #1 Code

	{ "gfx1.rom",	0x80000, 0xfd2ec297, 5 | BRF_GRA },		//  4 - Sprites
	{ "gfx2.rom",	0x80000, 0x6307aa60, 5 | BRF_GRA },		//  5
	{ "gfx3.rom",	0x80000, 0x718f3150, 5 | BRF_GRA },		//  6
	{ "gfx4.rom",	0x80000, 0xaf7e057e, 5 | BRF_GRA },		//  7
};

STD_ROM_PICK(uballoon)
STD_ROM_FN(uballoon)

struct BurnDriver BurnDrvUballoon = {
	"uballoon", NULL, NULL, NULL, "1996",
	"Ultra Balloon\0", NULL, "SunA", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_PLATFORM, 0,
	NULL, uballoonRomInfo, uballoonRomName, NULL, NULL, UballoonInputInfo, uballoonDIPInfo,
	UballoonInit, DrvExit, UballoonFrame, DrvDraw, DrvScan, &DrvRecalc, 0x1000,
	256, 224, 4, 3
};


// Back Street Soccer

static struct BurnRomInfo bssoccerRomDesc[] = {
	{ "02",		0x080000, 0x32871005, 1 | BRF_ESS | BRF_PRG },   //  0 - 68K Code
	{ "01",		0x080000, 0xace00db6, 1 | BRF_ESS | BRF_PRG },   //  1
	{ "04",		0x080000, 0x25ee404d, 1 | BRF_ESS | BRF_PRG },   //  2
	{ "03",		0x080000, 0x1a131014, 1 | BRF_ESS | BRF_PRG },   //  3

	{ "11",		0x010000, 0xdf7ae9bc, 2 | BRF_ESS | BRF_PRG },   //  4 - Z80 #0 Code

	{ "13",		0x080000, 0x2b273dca, 3 | BRF_ESS | BRF_PRG },   //  5 - Z80 #1 Code

	{ "12",		0x080000, 0x6b73b87b, 4 | BRF_ESS | BRF_PRG },   //  6 - Z80 #0 Code

	{ "05",		0x080000, 0xa5245bd4, 5 | BRF_GRA },		 //  7 - Sprites
	{ "07",		0x080000, 0xfdb765c2, 5 | BRF_GRA },		 //  8
	{ "09",		0x080000, 0x0e82277f, 5 | BRF_GRA },		 //  9
	{ "06",		0x080000, 0xd42ce84b, 5 | BRF_GRA },		 // 10
	{ "08",		0x080000, 0x96cd2136, 5 | BRF_GRA },		 // 11
	{ "10",		0x080000, 0x1ca94d21, 5 | BRF_GRA },		 // 12
};

STD_ROM_PICK(bssoccer)
STD_ROM_FN(bssoccer)

struct BurnDriver BurnDrvBssoccer = {
	"bssoccer", NULL, NULL, NULL, "1996",
	"Back Street Soccer\0", "graphics issues?", "SunA", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 4, HARDWARE_MISC_POST90S, GBF_SPORTSFOOTBALL, 0,
	NULL, bssoccerRomInfo, bssoccerRomName, NULL, NULL, BssoccerInputInfo, bssoccerDIPInfo,
	BssoccerInit, DrvExit, BssoccerFrame, DrvDraw, DrvScan, &DrvRecalc, 0x1000,
	256, 224, 4, 3
};
