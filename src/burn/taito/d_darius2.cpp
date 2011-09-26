#include "tiles_generic.h"
#include "taito.h"
#include "taito_ic.h"
#include "burn_ym2610.h"

static int Ninjaw;

static void Darius2Draw();

static struct BurnInputInfo Darius2InputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , TC0220IOCInputPort0 + 2, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL   , TC0220IOCInputPort2 + 2, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL   , TC0220IOCInputPort0 + 3, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL   , TC0220IOCInputPort2 + 3, "p2 start"  },

	{"P1 Up"             , BIT_DIGITAL   , TC0220IOCInputPort1 + 0, "p1 up"     },
	{"P1 Down"           , BIT_DIGITAL   , TC0220IOCInputPort1 + 1, "p1 down"   },
	{"P1 Left"           , BIT_DIGITAL   , TC0220IOCInputPort1 + 3, "p1 left"   },
	{"P1 Right"          , BIT_DIGITAL   , TC0220IOCInputPort1 + 2, "p1 right"  },
	{"P1 Fire 1"         , BIT_DIGITAL   , TC0220IOCInputPort2 + 4, "p1 fire 1" },
	{"P1 Fire 2"         , BIT_DIGITAL   , TC0220IOCInputPort2 + 5, "p1 fire 2" },

	{"P2 Up"             , BIT_DIGITAL   , TC0220IOCInputPort1 + 4, "p2 up"     },
	{"P2 Down"           , BIT_DIGITAL   , TC0220IOCInputPort1 + 5, "p2 down"   },
	{"P2 Left"           , BIT_DIGITAL   , TC0220IOCInputPort1 + 7, "p2 left"   },
	{"P2 Right"          , BIT_DIGITAL   , TC0220IOCInputPort1 + 6, "p2 right"  },
	{"P2 Fire 1"         , BIT_DIGITAL   , TC0220IOCInputPort2 + 6, "p2 fire 1" },
	{"P2 Fire 2"         , BIT_DIGITAL   , TC0220IOCInputPort2 + 7, "p2 fire 2" },

	{"Reset"             , BIT_DIGITAL   , &TaitoReset            , "reset"     },
	{"Service"           , BIT_DIGITAL   , TC0220IOCInputPort2 + 0, "service"   },
	{"Tilt"              , BIT_DIGITAL   , TC0220IOCInputPort2 + 1, "tilt"      },
	{"Freeze"            , BIT_DIGITAL   , TC0220IOCInputPort0 + 4, "freeze"    },
	{"Dip 1"             , BIT_DIPSWITCH , TC0220IOCDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , TC0220IOCDip + 1       , "dip"       },
};

STDINPUTINFO(Darius2)

static void Darius2MakeInputs()
{
	TC0220IOCInput[0] = 0xe2;
	TC0220IOCInput[1] = 0xff;
	TC0220IOCInput[2] = 0xff;

	if (TC0220IOCInputPort0[0]) TC0220IOCInput[0] |= 0x01;
	if (TC0220IOCInputPort0[1]) TC0220IOCInput[0] -= 0x02;
	if (TC0220IOCInputPort0[2]) TC0220IOCInput[0] |= 0x04;
	if (TC0220IOCInputPort0[3]) TC0220IOCInput[0] |= 0x08;
	if (TC0220IOCInputPort0[4]) TC0220IOCInput[0] |= 0x10;
	if (TC0220IOCInputPort0[5]) TC0220IOCInput[0] -= 0x20;
	if (TC0220IOCInputPort0[6]) TC0220IOCInput[0] -= 0x40;
	if (TC0220IOCInputPort0[7]) TC0220IOCInput[0] -= 0x80;

	for (int i = 0; i < 8; i++) {
		TC0220IOCInput[1] -= (TC0220IOCInputPort1[i] & 1) << i;
		TC0220IOCInput[2] -= (TC0220IOCInputPort2[i] & 1) << i;
	}
}

static struct BurnDIPInfo Darius2DIPList[]=
{
	// Default Values
	{0x14, 0xff, 0xff, 0xfe, NULL                             },
	{0x15, 0xff, 0xff, 0xff, NULL                             },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Auto Fire"                      },
	{0x14, 0x01, 0x02, 0x02, "Normal"                         },
	{0x14, 0x01, 0x02, 0x00, "Fast"                           },

	{0   , 0xfe, 0   , 2   , "Service Mode"                   },
	{0x14, 0x01, 0x04, 0x04, "Off"                            },
	{0x14, 0x01, 0x04, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Demo Sounds"                    },
	{0x14, 0x01, 0x08, 0x00, "Off"                            },
	{0x14, 0x01, 0x08, 0x08, "On"                             },

	{0   , 0xfe, 0   , 4   , "Coin A"                         },
	{0x14, 0x01, 0x30, 0x10, "2 Coins 1 Credit"               },
	{0x14, 0x01, 0x30, 0x30, "1 Coin  1 Credit"               },
	{0x14, 0x01, 0x30, 0x00, "2 Coins 3 Credits"              },
	{0x14, 0x01, 0x30, 0x20, "1 Coin  2 Credits"              },

	{0   , 0xfe, 0   , 4   , "Coin B"                         },
	{0x14, 0x01, 0xc0, 0x40, "2 Coins 1 Credit"               },
	{0x14, 0x01, 0xc0, 0xc0, "1 Coin  1 Credit"               },
	{0x14, 0x01, 0xc0, 0x00, "2 Coins 3 Credits"              },
	{0x14, 0x01, 0xc0, 0x80, "1 Coin  2 Credits"              },

	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"                     },
	{0x15, 0x01, 0x03, 0x02, "Easy"                           },
	{0x15, 0x01, 0x03, 0x03, "Medium"                         },
	{0x15, 0x01, 0x03, 0x01, "Hard"                           },
	{0x15, 0x01, 0x03, 0x00, "Hardest"                        },

	{0   , 0xfe, 0   , 4   , "Bonus Life"                     },
	{0x15, 0x01, 0x0c, 0x0c, "every 700k"                     },
	{0x15, 0x01, 0x0c, 0x08, "every 800k"                     },
	{0x15, 0x01, 0x0c, 0x04, "every 900k"                     },
	{0x15, 0x01, 0x0c, 0x00, "every 1000K"                    },

	{0   , 0xfe, 0   , 4   , "Lives"                          },
	{0x15, 0x01, 0x30, 0x30, "3"                              },
	{0x15, 0x01, 0x30, 0x20, "4"                              },
	{0x15, 0x01, 0x30, 0x10, "5"                              },
	{0x15, 0x01, 0x30, 0x00, "6"                              },

	{0   , 0xfe, 0   , 2   , "Invulnerability"                },
	{0x15, 0x01, 0x40, 0x40, "Off"                            },
	{0x15, 0x01, 0x40, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Allow Continue"                 },
	{0x15, 0x01, 0x80, 0x00, "Off"                            },
	{0x15, 0x01, 0x80, 0x80, "On"                             },
};

STDDIPINFO(Darius2)

static struct BurnDIPInfo NinjawDIPList[]=
{
	// Default Values
	{0x14, 0xff, 0xff, 0xff, NULL                             },
	{0x15, 0xff, 0xff, 0xff, NULL                             },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Allow Continue"                 },
	{0x14, 0x01, 0x01, 0x00, "Off"                            },
	{0x14, 0x01, 0x01, 0x01, "On"                             },

	{0   , 0xfe, 0   , 2   , "Auto Fire"                      },
	{0x14, 0x01, 0x02, 0x02, "Normal"                         },
	{0x14, 0x01, 0x02, 0x00, "Fast"                           },

	{0   , 0xfe, 0   , 2   , "Service Mode"                   },
	{0x14, 0x01, 0x04, 0x04, "Off"                            },
	{0x14, 0x01, 0x04, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Demo Sounds"                    },
	{0x14, 0x01, 0x08, 0x00, "Off"                            },
	{0x14, 0x01, 0x08, 0x08, "On"                             },

	{0   , 0xfe, 0   , 4   , "Coin A"                         },
	{0x14, 0x01, 0x30, 0x10, "2 Coins 1 Credit"               },
	{0x14, 0x01, 0x30, 0x30, "1 Coin  1 Credit"               },
	{0x14, 0x01, 0x30, 0x00, "2 Coins 3 Credits"              },
	{0x14, 0x01, 0x30, 0x20, "1 Coin  2 Credits"              },

	{0   , 0xfe, 0   , 4   , "Coin B"                         },
	{0x14, 0x01, 0xc0, 0x40, "2 Coins 1 Credit"               },
	{0x14, 0x01, 0xc0, 0xc0, "1 Coin  1 Credit"               },
	{0x14, 0x01, 0xc0, 0x00, "2 Coins 3 Credits"              },
	{0x14, 0x01, 0xc0, 0x80, "1 Coin  2 Credits"              },

	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"                     },
	{0x15, 0x01, 0x03, 0x02, "Easy"                           },
	{0x15, 0x01, 0x03, 0x03, "Medium"                         },
	{0x15, 0x01, 0x03, 0x01, "Hard"                           },
	{0x15, 0x01, 0x03, 0x00, "Hardest"                        },
};

STDDIPINFO(Ninjaw)

static struct BurnDIPInfo NinjawjDIPList[]=
{
	// Default Values
	{0x14, 0xff, 0xff, 0xff, NULL                             },
	{0x15, 0xff, 0xff, 0xff, NULL                             },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Allow Continue"                 },
	{0x14, 0x01, 0x01, 0x00, "Off"                            },
	{0x14, 0x01, 0x01, 0x01, "On"                             },

	{0   , 0xfe, 0   , 2   , "Auto Fire"                      },
	{0x14, 0x01, 0x02, 0x02, "Normal"                         },
	{0x14, 0x01, 0x02, 0x00, "Fast"                           },

	{0   , 0xfe, 0   , 2   , "Service Mode"                   },
	{0x14, 0x01, 0x04, 0x04, "Off"                            },
	{0x14, 0x01, 0x04, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Demo Sounds"                    },
	{0x14, 0x01, 0x08, 0x00, "Off"                            },
	{0x14, 0x01, 0x08, 0x08, "On"                             },

	{0   , 0xfe, 0   , 4   , "Coin A"                         },
	{0x14, 0x01, 0x30, 0x00, "4 Coins 1 Credit"               },
	{0x14, 0x01, 0x30, 0x10, "3 Coins 1 Credit"               },
	{0x14, 0x01, 0x30, 0x20, "2 Coins 1 Credit"               },
	{0x14, 0x01, 0x30, 0x30, "1 Coin  1 Credit"               },

	{0   , 0xfe, 0   , 4   , "Coin B"                         },
	{0x14, 0x01, 0xc0, 0xc0, "1 Coin 2 Credits"               },
	{0x14, 0x01, 0xc0, 0x80, "1 Coin 3 Credits"               },
	{0x14, 0x01, 0xc0, 0x40, "1 Coin 4 Credits"               },
	{0x14, 0x01, 0xc0, 0x00, "1 Coin 6 Credits"               },

	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"                     },
	{0x15, 0x01, 0x03, 0x02, "Easy"                           },
	{0x15, 0x01, 0x03, 0x03, "Medium"                         },
	{0x15, 0x01, 0x03, 0x01, "Hard"                           },
	{0x15, 0x01, 0x03, 0x00, "Hardest"                        },
};

STDDIPINFO(Ninjawj)

static struct BurnRomInfo Darius2RomDesc[] = {
	{ "c07-32-1",      0x10000, 0x216c8f6a, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c07-29-1",      0x10000, 0x48de567f, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c07-31-1",      0x10000, 0x8279d2f8, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c07-30-1",      0x10000, 0x6122e400, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c07-27",        0x20000, 0x0a6f7b6c, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c07-25",        0x20000, 0x059f40ce, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c07-26",        0x20000, 0x1f411242, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c07-24",        0x20000, 0x486c9c20, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },

	{ "c07-35-1",      0x10000, 0xdd8c4723, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	{ "c07-38-1",      0x10000, 0x46afb85c, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	{ "c07-34-1",      0x10000, 0x296984b8, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	{ "c07-37-1",      0x10000, 0x8b7d461f, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	{ "c07-33-1",      0x10000, 0x2da03a3f, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	{ "c07-36-1",      0x10000, 0x02cf2b1c, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },

	{ "c07-28",        0x20000, 0xda304bc5, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },

	{ "c07-03.12",     0x80000, 0x189bafce, BRF_GRA | TAITO_CHARS },
	{ "c07-04.11",     0x80000, 0x50421e81, BRF_GRA | TAITO_CHARS },

	{ "c07-01",        0x80000, 0x3cf0f050, BRF_GRA | TAITO_SPRITESA },
	{ "c07-02",        0x80000, 0x75d16d4b, BRF_GRA | TAITO_SPRITESA },

	{ "c07-10.95",     0x80000, 0x4bbe0ed9, BRF_SND | TAITO_YM2610A },
	{ "c07-11.96",     0x80000, 0x3c815699, BRF_SND | TAITO_YM2610A },

	{ "c07-12.107",    0x80000, 0xe0b71258, BRF_SND | TAITO_YM2610B },
};

STD_ROM_PICK(Darius2)
STD_ROM_FN(Darius2)

static struct BurnRomInfo NinjawRomDesc[] = {
	{ "b31-45",        0x10000, 0x107902c3, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "b31-47",        0x10000, 0xbd536b1e, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "b31_29.34",     0x10000, 0xf2941a37, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "b31_27.31",     0x10000, 0x2f3ff642, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "b31_41.5",      0x20000, 0x0daef28a, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "b31_39.2",      0x20000, 0xe9197c3c, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "b31_40.6",      0x20000, 0x2ce0f24e, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "b31_38.3",      0x20000, 0xbc68cd99, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },

	{ "b31_33.87",     0x10000, 0x6ce9af44, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	{ "b31_36.97",     0x10000, 0xba20b0d4, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	{ "b31_32.86",     0x10000, 0xe6025fec, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	{ "b31_35.96",     0x10000, 0x70d9a89f, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	{ "b31_31.85",     0x10000, 0x837f47e2, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	{ "b31_34.95",     0x10000, 0xd6b5fb2a, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },

	{ "b31_37.11",     0x20000, 0x0ca5799d, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },

	{ "b31-01.23",     0x80000, 0x8e8237a7, BRF_GRA | TAITO_CHARS },
	{ "b31-02.24",     0x80000, 0x4c3b4e33, BRF_GRA | TAITO_CHARS },

	{ "b31-07.176",    0x80000, 0x33568cdb, BRF_GRA | TAITO_SPRITESA },
	{ "b31-06.175",    0x80000, 0x0d59439e, BRF_GRA | TAITO_SPRITESA },
	{ "b31-05.174",    0x80000, 0x0a1fc9fb, BRF_GRA | TAITO_SPRITESA },
	{ "b31-04.173",    0x80000, 0x2e1e4cb5, BRF_GRA | TAITO_SPRITESA },

	{ "b31-09.18",     0x80000, 0x60a73382, BRF_SND | TAITO_YM2610A },
	{ "b31-10.17",     0x80000, 0xc6434aef, BRF_SND | TAITO_YM2610A },
	{ "b31-11.16",     0x80000, 0x8da531d4, BRF_SND | TAITO_YM2610A },

	{ "b31-08.19",     0x80000, 0xa0a1f87d, BRF_SND | TAITO_YM2610B },

	{ "b31-25.38",     0x00200, 0xa0b4ba48, BRF_OPT },
	{ "b31-26.58",     0x00200, 0x13e5fe15, BRF_OPT },
};

STD_ROM_PICK(Ninjaw)
STD_ROM_FN(Ninjaw)

static struct BurnRomInfo NinjawjRomDesc[] = {
	{ "b31_30.35",     0x10000, 0x056edd9f, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "b31_28.32",     0x10000, 0xcfa7661c, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "b31_29.34",     0x10000, 0xf2941a37, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "b31_27.31",     0x10000, 0x2f3ff642, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "b31_41.5",      0x20000, 0x0daef28a, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "b31_39.2",      0x20000, 0xe9197c3c, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "b31_40.6",      0x20000, 0x2ce0f24e, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "b31_38.3",      0x20000, 0xbc68cd99, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },

	{ "b31_33.87",     0x10000, 0x6ce9af44, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	{ "b31_36.97",     0x10000, 0xba20b0d4, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	{ "b31_32.86",     0x10000, 0xe6025fec, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	{ "b31_35.96",     0x10000, 0x70d9a89f, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	{ "b31_31.85",     0x10000, 0x837f47e2, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },
	{ "b31_34.95",     0x10000, 0xd6b5fb2a, BRF_ESS | BRF_PRG | TAITO_68KROM2_BYTESWAP },

	{ "b31_37.11",     0x20000, 0x0ca5799d, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },

	{ "b31-01.23",     0x80000, 0x8e8237a7, BRF_GRA | TAITO_CHARS },
	{ "b31-02.24",     0x80000, 0x4c3b4e33, BRF_GRA | TAITO_CHARS },

	{ "b31-07.176",    0x80000, 0x33568cdb, BRF_GRA | TAITO_SPRITESA },
	{ "b31-06.175",    0x80000, 0x0d59439e, BRF_GRA | TAITO_SPRITESA },
	{ "b31-05.174",    0x80000, 0x0a1fc9fb, BRF_GRA | TAITO_SPRITESA },
	{ "b31-04.173",    0x80000, 0x2e1e4cb5, BRF_GRA | TAITO_SPRITESA },

	{ "b31-09.18",     0x80000, 0x60a73382, BRF_SND | TAITO_YM2610A },
	{ "b31-10.17",     0x80000, 0xc6434aef, BRF_SND | TAITO_YM2610A },
	{ "b31-11.16",     0x80000, 0x8da531d4, BRF_SND | TAITO_YM2610A },

	{ "b31-08.19",     0x80000, 0xa0a1f87d, BRF_SND | TAITO_YM2610B },

	{ "b31-25.38",     0x00200, 0xa0b4ba48, BRF_OPT },
	{ "b31-26.58",     0x00200, 0x13e5fe15, BRF_OPT },
};

STD_ROM_PICK(Ninjawj)
STD_ROM_FN(Ninjawj)

static int MemIndex()
{
	unsigned char *Next; Next = TaitoMem;

	Taito68KRom1                        = Next; Next += Taito68KRom1Size;
	Taito68KRom2                        = Next; Next += Taito68KRom2Size;
	TaitoZ80Rom1                        = Next; Next += TaitoZ80Rom1Size;
	TaitoYM2610ARom                     = Next; Next += TaitoYM2610ARomSize;
	TaitoYM2610BRom                     = Next; Next += TaitoYM2610BRomSize;

	TaitoRamStart                       = Next;

	Taito68KRam1                        = Next; Next += 0x010000;
	Taito68KRam2                        = Next; Next += 0x010000;
	TaitoZ80Ram1                        = Next; Next += 0x002000;
	TaitoSharedRam                      = Next; Next += 0x010000;
	TaitoSpriteRam                      = Next; Next += 0x004000;

	TaitoRamEnd                         = Next;

	TaitoChars                          = Next; Next += TaitoNumChar * TaitoCharWidth * TaitoCharHeight;
	TaitoSpritesA                       = Next; Next += TaitoNumSpriteA * TaitoSpriteAWidth * TaitoSpriteAHeight;

	TaitoMemEnd                         = Next;

	return 0;
}

static void Darius2CpuAReset(UINT16 d)
{
	TaitoCpuACtrl = d;
	if (!(TaitoCpuACtrl & 1)) {
		SekClose();
		SekOpen(1);
		SekReset();
		SekClose();
		SekOpen(0);
	}
}

unsigned char __fastcall Darius268K1ReadByte(unsigned int a)
{
//	switch (a) {
//		default: {
//			bprintf(PRINT_NORMAL, _T("68K #1 Read byte => %06X\n"), a);
//		}
//	}

	return 0;
}

void __fastcall Darius268K1WriteByte(unsigned int a, unsigned char d)
{
//	switch (a) {
//		default: {
//			bprintf(PRINT_NORMAL, _T("68K #1 Write byte => %06X, %02X\n"), a, d);
//		}
//	}
}

unsigned short __fastcall Darius268K1ReadWord(unsigned int a)
{
	switch (a) {
		case 0x200000: {
			return TC0220IOCPortRegRead();
		}

		case 0x200002: {
			return TC0220IOCHalfWordPortRead();
		}

		case 0x220002: {
			return TC0140SYTCommRead();
		}

		case 0x340002: {
			return TC0110PCRWordRead(0);
		}

		case 0x350002: {
			return TC0110PCRWordRead(1);
		}

		case 0x360002: {
			return TC0110PCRWordRead(2);
		}

		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Read word => %06X\n"), a);
		}
	}

	return 0;
}

void __fastcall Darius268K1WriteWord(unsigned int a, unsigned short d)
{
	TC0100SCNCtrlWordWrite_Map(0x2a0000)

	if (a >= 0x2e0000 && a <= 0x2e000f) return; // TC0100SCN Chip 1 Ctrl
	if (a >= 0x320000 && a <= 0x32000f) return; // TC0100SCN Chip 2 Ctrl

	if (a >= 0x280000 && a <= 0x293fff) {
		int Offset = (a - 0x280000) >> 1;
		UINT16 *Ram0 = (UINT16*)TC0100SCNRam[0];
		UINT16 *Ram1 = (UINT16*)TC0100SCNRam[1];
		UINT16 *Ram2 = (UINT16*)TC0100SCNRam[2];
		Ram0[Offset] = swapWord(d);
		Ram1[Offset] = swapWord(d);
		Ram2[Offset] = swapWord(d);
		return;
	}

	switch (a) {
		case 0x200000: {
			TC0220IOCHalfWordPortRegWrite(d);
			return;
		}

		case 0x200002: {
			TC0220IOCHalfWordPortWrite(d);
			return;
		}

		case 0x210000: {
			Darius2CpuAReset(d);
			return;
		}

		case 0x220000: {
			TC0140SYTPortWrite(d);
			return;
		}

		case 0x220002: {
			TC0140SYTCommWrite(d);
			return;
		}

		case 0x340000:
		case 0x340002: {
			TC0110PCRStep1WordWrite(0, (a - 0x340000) >> 1, d);
			return;
		}

		case 0x350000:
		case 0x350002: {
			TC0110PCRStep1WordWrite(1, (a - 0x350000) >> 1, d);
			return;
		}

		case 0x360000:
		case 0x360002: {
			TC0110PCRStep1WordWrite(2, (a - 0x360000) >> 1, d);
			return;
		}

		case 0x340004:
		case 0x350004:
		case 0x360004: {
			//nop
			return;
		}

		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write word => %06X, %04X\n"), a, d);
		}
	}
}

unsigned char __fastcall Darius268K2ReadByte(unsigned int a)
{
//	switch (a) {
//		default: {
//			bprintf(PRINT_NORMAL, _T("68K #2 Read byte => %06X\n"), a);
//		}
//	}

	return 0;
}

void __fastcall Darius268K2WriteByte(unsigned int a, unsigned char d)
{
//	switch (a) {
//		default: {
//			bprintf(PRINT_NORMAL, _T("68K #2 Write byte => %06X, %02X\n"), a, d);
//		}
//	}
}

unsigned short __fastcall Darius268K2ReadWord(unsigned int a)
{
	switch (a) {
		case 0x200000: {
			return TC0220IOCPortRegRead();
		}

		default: {
			bprintf(PRINT_NORMAL, _T("68K #2 Read word => %06X\n"), a);
		}
	}

	return 0;
}

void __fastcall Darius268K2WriteWord(unsigned int a, unsigned short d)
{
	if (a >= 0x280000 && a <= 0x293fff) {
		int Offset = (a - 0x280000) >> 1;
		UINT16 *Ram0 = (UINT16*)TC0100SCNRam[0];
		UINT16 *Ram1 = (UINT16*)TC0100SCNRam[1];
		UINT16 *Ram2 = (UINT16*)TC0100SCNRam[2];
		Ram0[Offset] = swapWord(d);
		Ram1[Offset] = swapWord(d);
		Ram2[Offset] = swapWord(d);
		return;
	}

	switch (a) {
		case 0x210000: {
			//???
			return;
		}

		case 0x340000:
		case 0x340002: {
			TC0110PCRStep1WordWrite(0, (a - 0x340000) >> 1, d);
			return;
		}

		case 0x350000:
		case 0x350002: {
			TC0110PCRStep1WordWrite(1, (a - 0x350000) >> 1, d);
			return;
		}

		case 0x360000:
		case 0x360002: {
			TC0110PCRStep1WordWrite(2, (a - 0x360000) >> 1, d);
			return;
		}

		case 0x340004:
		case 0x350004:
		case 0x360004: {
			//nop
			return;
		}

		default: {
			bprintf(PRINT_NORMAL, _T("68K #2 Write word => %06X, %04X\n"), a, d);
		}
	}
}

unsigned char __fastcall Darius2Z80Read(unsigned short a)
{
	switch (a) {
		case 0xe000: {
			return BurnYM2610Read(0);
		}

		case 0xe002: {
			return BurnYM2610Read(2);
		}

		case 0xe201: {
			return TC0140SYTSlaveCommRead();
		}

		case 0xea00: {
			// NOP
			return 0;
		}

		default: {
			bprintf(PRINT_NORMAL, _T("Z80 Read %04X\n"), a);
		}
	}

	return 0;
}

void __fastcall Darius2Z80Write(unsigned short a, unsigned char d)
{
	switch (a) {
		case 0xe000: {
			BurnYM2610Write(0, d);
			return;
		}

		case 0xe001: {
			BurnYM2610Write(1, d);
			return;
		}

		case 0xe002: {
			BurnYM2610Write(2, d);
			return;
		}

		case 0xe003: {
			BurnYM2610Write(3, d);
			return;
		}

		case 0xe200: {
			TC0140SYTSlavePortWrite(d);
			return;
		}

		case 0xe201: {
			TC0140SYTSlaveCommWrite(d);
			return;
		}

		case 0xe400:
		case 0xe401:
		case 0xe402:
		case 0xe403: {
			//pan control
			return;
		}

		case 0xe600: {
			//???
			return;
		}

		case 0xee00: {
			//nop
			return;
		}

		case 0xf000: {
			//nop
			return;
		}

		case 0xf200: {
			TaitoZ80Bank = (d - 1) & 7;
			ZetMapArea(0x4000, 0x7fff, 0, TaitoZ80Rom1 + 0x4000 + (TaitoZ80Bank * 0x4000));
			ZetMapArea(0x4000, 0x7fff, 2, TaitoZ80Rom1 + 0x4000 + (TaitoZ80Bank * 0x4000));
			return;
		}

		default: {
			bprintf(PRINT_NORMAL, _T("Z80 Write %04X, %02X\n"), a, d);
		}
	}
}

static int CharPlaneOffsets[4]          = { 0, 1, 2, 3 };
static int CharXOffsets[8]              = { 8, 12, 0, 4, 24, 28, 16, 20 };
static int CharYOffsets[8]              = { 0, 32, 64, 96, 128, 160, 192, 224 };
static int SpritePlaneOffsets[4]        = { 8, 12, 0, 4 };
static int SpriteXOffsets[16]           = { 3, 2, 1, 0, 19, 18, 17, 16, 259, 258, 257, 256, 275, 274, 273, 272 };
static int SpriteYOffsets[16]           = { 0, 32, 64, 96, 128, 160, 192, 224, 512, 544, 576, 608, 640, 672, 704, 736 };

static void Darius2FMIRQHandler(int, int nStatus)
{
	if (nStatus & 1) {
		ZetSetIRQLine(0xFF, ZET_IRQSTATUS_ACK);
	} else {
		ZetSetIRQLine(0,    ZET_IRQSTATUS_NONE);
	}
}

static int Darius2SynchroniseStream(int nSoundRate)
{
	return (long long)ZetTotalCycles() * nSoundRate / (16000000 / 4);
}

static double Darius2GetTime()
{
	return (double)ZetTotalCycles() / (16000000 / 4);
}

static int Darius2Init()
{
	int nLen;

	TaitoCharModulo = 0x100;
	TaitoCharNumPlanes = 4;
	TaitoCharWidth = 8;
	TaitoCharHeight = 8;
	TaitoCharPlaneOffsets = CharPlaneOffsets;
	TaitoCharXOffsets = CharXOffsets;
	TaitoCharYOffsets = CharYOffsets;
	TaitoNumChar = 0x8000;

	TaitoSpriteAModulo = 0x400;
	TaitoSpriteANumPlanes = 4;
	TaitoSpriteAWidth = 16;
	TaitoSpriteAHeight = 16;
	TaitoSpriteAPlaneOffsets = SpritePlaneOffsets;
	TaitoSpriteAXOffsets = SpriteXOffsets;
	TaitoSpriteAYOffsets = SpriteYOffsets;
	TaitoNumSpriteA = 0x2000;

	if (Ninjaw) TaitoNumSpriteA = 0x4000;

	TaitoNum68Ks = 2;
	TaitoNumZ80s = 1;
	TaitoNumYM2610 = 1;

	TaitoLoadRoms(0);

	// Allocate and Blank all required memory
	TaitoMem = NULL;
	MemIndex();
	nLen = TaitoMemEnd - (unsigned char *)0;
	if ((TaitoMem = (unsigned char *)malloc(nLen)) == NULL) return 1;
	memset(TaitoMem, 0, nLen);
	MemIndex();

	if (TaitoLoadRoms(1)) return 1;

	TC0100SCNInit(3, TaitoNumChar, 22, 16, 0, NULL);
	TC0110PCRInit(3, 0x3000);
	TC0140SYTInit();
	TC0220IOCInit();

	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Taito68KRom1           , 0x000000, 0x0bffff, SM_ROM);
	SekMapMemory(Taito68KRam1           , 0x0c0000, 0x0cffff, SM_RAM);
	SekMapMemory(TaitoSharedRam         , 0x240000, 0x24ffff, SM_RAM);
	SekMapMemory(TaitoSpriteRam         , 0x260000, 0x263fff, SM_RAM);
	SekMapMemory(TC0100SCNRam[0]        , 0x280000, 0x293fff, SM_READ);
	SekMapMemory(TC0100SCNRam[1]        , 0x2c0000, 0x2d3fff, SM_RAM);
	SekMapMemory(TC0100SCNRam[2]        , 0x300000, 0x313fff, SM_RAM);
	SekSetReadByteHandler(0, Darius268K1ReadByte);
	SekSetWriteByteHandler(0, Darius268K1WriteByte);
	SekSetReadWordHandler(0, Darius268K1ReadWord);
	SekSetWriteWordHandler(0, Darius268K1WriteWord);
	SekClose();

	SekInit(1, 0x68000);
	SekOpen(1);
	SekMapMemory(Taito68KRom2           , 0x000000, 0x05ffff, SM_ROM);
	SekMapMemory(Taito68KRam2           , 0x080000, 0x08ffff, SM_RAM);
	SekMapMemory(TaitoSharedRam         , 0x240000, 0x24ffff, SM_RAM);
	SekMapMemory(TaitoSpriteRam         , 0x260000, 0x263fff, SM_RAM);
	SekMapMemory(TC0100SCNRam[0]        , 0x280000, 0x293fff, SM_READ);
	SekSetReadByteHandler(0, Darius268K2ReadByte);
	SekSetWriteByteHandler(0, Darius268K2WriteByte);
	SekSetReadWordHandler(0, Darius268K2ReadWord);
	SekSetWriteWordHandler(0, Darius268K2WriteWord);
	SekClose();

	ZetInit(1);
	ZetOpen(0);
	ZetSetReadHandler(Darius2Z80Read);
	ZetSetWriteHandler(Darius2Z80Write);
	ZetMapArea(0x0000, 0x3fff, 0, TaitoZ80Rom1               );
	ZetMapArea(0x0000, 0x3fff, 2, TaitoZ80Rom1               );
	ZetMapArea(0x4000, 0x7fff, 0, TaitoZ80Rom1 + 0x4000      );
	ZetMapArea(0x4000, 0x7fff, 2, TaitoZ80Rom1 + 0x4000      );
	ZetMapArea(0xc000, 0xdfff, 0, TaitoZ80Ram1               );
	ZetMapArea(0xc000, 0xdfff, 1, TaitoZ80Ram1               );
	ZetMapArea(0xc000, 0xdfff, 2, TaitoZ80Ram1               );
	ZetMemEnd();
	ZetClose();

	BurnYM2610Init(16000000 / 2, TaitoYM2610ARom, (int*)&TaitoYM2610ARomSize, TaitoYM2610BRom, (int*)&TaitoYM2610BRomSize, &Darius2FMIRQHandler, Darius2SynchroniseStream, Darius2GetTime, 0);
	BurnTimerAttachZet(16000000 / 4);

	GenericTilesInit();

	TaitoDrawFunction = Darius2Draw;
	TaitoMakeInputsFunction = Darius2MakeInputs;
	TaitoIrqLine = 4;

	nTaitoCyclesTotal[0] = (16000000 / 2) / 60;
	nTaitoCyclesTotal[1] = (16000000 / 2) / 60;
	nTaitoCyclesTotal[2] = (16000000 / 4) / 60;

	TaitoResetFunction = TaitoDoReset;
	TaitoResetFunction();

	return 0;
}

static int NinjawInit()
{
	Ninjaw = 1;
	return Darius2Init();
}

static int Darius2Exit()
{
	Ninjaw = 0;

	return TaitoExit();
}

static void Darius2RenderSprites(int PriorityDraw)
{
	int Offset, Data, Code, Colour, xFlip, yFlip;
	int x, y, Priority;

	UINT16 *SpriteRam = (UINT16*)TaitoSpriteRam;

	for (Offset = 0x2000 - 4; Offset >=0; Offset -= 4) {
		Data = swapWord(SpriteRam[Offset + 2]);
		Code = Data & (TaitoNumSpriteA - 1);

		if (!Code) continue;

		Data = swapWord(SpriteRam[Offset + 0]);
		x = (Data - 32) & 0x3ff;

		Data = swapWord(SpriteRam[Offset + 1]);
		y = (Data - 0) & 0x1ff;

		Data = swapWord(SpriteRam[Offset + 3]);
		xFlip = (Data & 0x1);
		yFlip = (Data & 0x2) >> 1;
		Priority = (Data & 0x4) >> 2;

		if (Priority != PriorityDraw) continue;

		Colour = (Data & 0x7f00) >> 8;

		y -= 16;

		if (x > 0x3c0) x -= 0x400;
		if (y > 0x180) y -= 0x200;

		if (x > 16 && x < (nScreenWidth - 16) && y > 16 && y < (nScreenHeight - 16)) {
			if (xFlip) {
				if (yFlip) {
					Render16x16Tile_Mask_FlipXY(pTransDraw, Code, x, y, Colour, 4, 0, 0, TaitoSpritesA);
				} else {
					Render16x16Tile_Mask_FlipX(pTransDraw, Code, x, y, Colour, 4, 0, 0, TaitoSpritesA);
				}
			} else {
				if (yFlip) {
					Render16x16Tile_Mask_FlipY(pTransDraw, Code, x, y, Colour, 4, 0, 0, TaitoSpritesA);
				} else {
					Render16x16Tile_Mask(pTransDraw, Code, x, y, Colour, 4, 0, 0, TaitoSpritesA);
				}
			}
		} else {
			if (xFlip) {
				if (yFlip) {
					Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, Code, x, y, Colour, 4, 0, 0, TaitoSpritesA);
				} else {
					Render16x16Tile_Mask_FlipX_Clip(pTransDraw, Code, x, y, Colour, 4, 0, 0, TaitoSpritesA);
				}
			} else {
				if (yFlip) {
					Render16x16Tile_Mask_FlipY_Clip(pTransDraw, Code, x, y, Colour, 4, 0, 0, TaitoSpritesA);
				} else {
					Render16x16Tile_Mask_Clip(pTransDraw, Code, x, y, Colour, 4, 0, 0, TaitoSpritesA);
				}
			}
		}
	}
}

static void Darius2Draw()
{
	int Disable = TC0100SCNCtrl[0][6] & 0xf7;

	BurnTransferClear();

	if (TC0100SCNBottomLayer()) {
		if (!(Disable & 0x02)) TC0100SCNRenderFgLayer(0, TaitoChars);
		Darius2RenderSprites(1);
		if (!(Disable & 0x01)) TC0100SCNRenderBgLayer(0, TaitoChars);
	} else {
		if (!(Disable & 0x01)) TC0100SCNRenderBgLayer(0, TaitoChars);
		Darius2RenderSprites(1);
		if (!(Disable & 0x02)) TC0100SCNRenderFgLayer(0, TaitoChars);
	}

	Darius2RenderSprites(0);

	if (!(Disable & 0x04)) TC0100SCNRenderCharLayer();
	BurnTransferCopy(TC0110PCRPalette);
}

static int Darius2Frame()
{
	int nInterleave = 100;

	if (TaitoReset) TaitoResetFunction();

	TaitoMakeInputsFunction();

	nTaitoCyclesDone[0] = nTaitoCyclesDone[1] = nTaitoCyclesDone[2] = 0;

	SekNewFrame();
	ZetNewFrame();

	for (int i = 0; i < nInterleave; i++) {
		int nCurrentCPU, nNext;

		// Run 68000 # 1
		nCurrentCPU = 0;
		SekOpen(0);
		nNext = (i + 1) * nTaitoCyclesTotal[nCurrentCPU] / nInterleave;
		nTaitoCyclesSegment = nNext - nTaitoCyclesDone[nCurrentCPU];
		nTaitoCyclesDone[nCurrentCPU] += SekRun(nTaitoCyclesSegment);
		if (i == nInterleave - 1) SekSetIRQLine(TaitoIrqLine, SEK_IRQSTATUS_AUTO);
		SekClose();

		// Run 68000 # 2
		if ((TaitoCpuACtrl & 0x01)) {
			nCurrentCPU = 1;
			SekOpen(1);
			nNext = (i + 1) * nTaitoCyclesTotal[nCurrentCPU] / nInterleave;
			nTaitoCyclesSegment = nNext - nTaitoCyclesDone[nCurrentCPU];
			nTaitoCyclesDone[nCurrentCPU] += SekRun(nTaitoCyclesSegment);
			if (i == nInterleave - 1) SekSetIRQLine(TaitoIrqLine, SEK_IRQSTATUS_AUTO);
			SekClose();
		}
	}

	ZetOpen(0);
	BurnTimerEndFrame(nTaitoCyclesTotal[2] - nTaitoCyclesDone[2]);
	BurnYM2610Update(pBurnSoundOut, nBurnSoundLen);
	ZetClose();

	if (pBurnDraw) TaitoDrawFunction();

	return 0;
}

static int Darius2Scan(int nAction, int *pnMin)
{
	struct BurnArea ba;

	if (pnMin != NULL) {			// Return minimum compatible version
		*pnMin = 0x029684;
	}

	if (nAction & ACB_MEMORY_RAM) {
		memset(&ba, 0, sizeof(ba));
		ba.Data	  = TaitoRamStart;
		ba.nLen	  = TaitoRamEnd-TaitoRamStart;
		ba.szName = "All Ram";
		BurnAcb(&ba);
	}

	TaitoICScan(nAction);

	if (nAction & ACB_DRIVER_DATA) {
		SekScan(nAction);
		ZetScan(nAction);

		if (TaitoNumYM2610) BurnYM2610Scan(nAction, pnMin);

		SCAN_VAR(TaitoInput);
		SCAN_VAR(TaitoZ80Bank);
		SCAN_VAR(nTaitoCyclesDone);
		SCAN_VAR(nTaitoCyclesSegment);
	}

	if (nAction & ACB_WRITE && TaitoZ80Bank) {
		ZetOpen(0);
		ZetMapArea(0x4000, 0x7fff, 0, TaitoZ80Rom1 + 0x4000 + (TaitoZ80Bank * 0x4000));
		ZetMapArea(0x4000, 0x7fff, 2, TaitoZ80Rom1 + 0x4000 + (TaitoZ80Bank * 0x4000));
		ZetClose();
	}

	return 0;
}

struct BurnDriver BurnDrvDarius2 = {
	"darius2", NULL, NULL, "1989",
	"Darius II (Japan)\0", NULL, "Taito Corporation", "Taito-F2",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_TAITO_TAITOF2,
	NULL, Darius2RomInfo, Darius2RomName, Darius2InputInfo, Darius2DIPInfo,
	Darius2Init, Darius2Exit, Darius2Frame, NULL, Darius2Scan,
	NULL, 864, 224, 12, 3
};

struct BurnDriver BurnDrvNinjaw = {
	"ninjaw", NULL, NULL, "1987",
	"The Ninja Warriors (World)\0", NULL, "Taito Corporation Japan", "Taito-F2",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_TAITO_TAITOF2,
	NULL, NinjawRomInfo, NinjawRomName, Darius2InputInfo, NinjawDIPInfo,
	NinjawInit, Darius2Exit, Darius2Frame, NULL, Darius2Scan,
	NULL, 864, 224, 12, 3
};

struct BurnDriver BurnDrvNinjawj = {
	"ninjawj", "ninjaw", NULL, "1987",
	"The Ninja Warriors (Japan)\0", NULL, "Taito Corporation", "Taito-F2",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_TAITO_TAITOF2,
	NULL, NinjawjRomInfo, NinjawjRomName, Darius2InputInfo, NinjawjDIPInfo,
	NinjawInit, Darius2Exit, Darius2Frame, NULL, Darius2Scan,
	NULL, 864, 224, 12, 3
};
