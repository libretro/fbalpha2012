// Irem M92 system games:
//
// port from MAME by OopsWare
//

#include "burnint.h"
#include "burn_ym2151.h"
#include "vez.h"


#include "irem_cpu.h"


static unsigned char *Mem = NULL, *MemEnd = NULL;
static unsigned char *RamStart, *RamEnd;

static unsigned char *RomV33;
static unsigned char *RomV30;
static unsigned char *RomGfx01;
static unsigned char *RomGfx02;

static unsigned char *RamVideo;
static unsigned char *RamV33;
static unsigned char *RamV30;
static unsigned char *RamSpr;
static unsigned char *RamSprCpy;
static unsigned char *RamPal;

static unsigned short *RamCurPal;

static unsigned char DrvButton[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static unsigned char DrvJoy1[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static unsigned char DrvJoy2[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static unsigned char DrvJoy3[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static unsigned char DrvJoy4[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static unsigned char DrvInput[8] = {0, 0, 0, 0, 0, 0, 0, 0};

static unsigned char bRecalcPalette = 0;
static unsigned char DrvReset = 0;

static int m92_irq_vectorbase;
static unsigned int PalBank;

static unsigned char pf1_control[8],pf2_control[8],pf3_control[8],pf4_control[8];
static int pf1_enable,pf2_enable,pf3_enable;
static int pf1_shape,pf2_shape,pf3_shape;
static int pf1_rowscroll,pf2_rowscroll,pf3_rowscroll;
static int pf1_vram_ptr,pf2_vram_ptr,pf3_vram_ptr;
static int m92_sprite_list;

static unsigned char sound_status[2];
static unsigned char sound_latch[2];

static unsigned char irqvector;

static unsigned char m92_sprite_buffer_busy;
static int m92_sprite_buffer_timer;

static int m92_raster_irq_position = 0;
//static int m92_raster_enable = 1;

inline static unsigned int CalcCol(int offs)
{
	// xBBBBBGGGGGRRRRR
	int nColour = RamPal[offs + 0] | (RamPal[offs + 1] << 8);
	int r, g, b;

	r = (nColour & 0x001F) << 3;	// Red
	r |= r >> 5;
	g = (nColour & 0x03E0) >> 2;	// Green
	g |= g >> 5;
	b = (nColour & 0x7C00) >> 7;	// Blue
	b |= b >> 5;

	return BurnHighCol(r, g, b, 0);
}

static struct BurnInputInfo hookInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvButton + 2,	"p1 coin"},
	{"P1 Start",	BIT_DIGITAL,	DrvButton + 0,	"p1 start"},

	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 up"},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 down"},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 left"},
	{"P1 Right",	BIT_DIGITAL,	DrvJoy1 + 0,	"p1 right"},
	{"P1 Button 1",	BIT_DIGITAL,	DrvJoy1 + 7,	"p1 fire 1"},
	{"P1 Button 2",	BIT_DIGITAL,	DrvJoy1 + 6,	"p1 fire 2"},

	{"P2 Coin",		BIT_DIGITAL,	DrvButton + 3,	"p2 coin"},
	{"P2 Start",	BIT_DIGITAL,	DrvButton + 1,	"p2 start"},

	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 up"},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 down"},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 left"},
	{"P2 Right",	BIT_DIGITAL,	DrvJoy2 + 0,	"p2 right"},
	{"P2 Button 1",	BIT_DIGITAL,	DrvJoy2 + 7,	"p2 fire 1"},
	{"P2 Button 2",	BIT_DIGITAL,	DrvJoy2 + 6,	"p2 fire 2"},

	{"P3 Coin",		BIT_DIGITAL,	DrvJoy3 + 5,	"p3 coin"},
	{"P3 Start",	BIT_DIGITAL,	DrvJoy3 + 4,	"p3 start"},

	{"P3 Up",		BIT_DIGITAL,	DrvJoy3 + 3,	"p3 up"},
	{"P3 Down",		BIT_DIGITAL,	DrvJoy3 + 2,	"p3 down"},
	{"P3 Left",		BIT_DIGITAL,	DrvJoy3 + 1,	"p3 left"},
	{"P3 Right",	BIT_DIGITAL,	DrvJoy3 + 0,	"p3 right"},
	{"P3 Button 1",	BIT_DIGITAL,	DrvJoy3 + 7,	"p3 fire 1"},
	{"P3 Button 2",	BIT_DIGITAL,	DrvJoy3 + 6,	"p3 fire 2"},

	{"P4 Coin",		BIT_DIGITAL,	DrvJoy4 + 5,	"p4 coin"},
	{"P4 Start",	BIT_DIGITAL,	DrvJoy4 + 4,	"p4 start"},

	{"P4 Up",		BIT_DIGITAL,	DrvJoy4 + 3,	"p4 up"},
	{"P4 Down",		BIT_DIGITAL,	DrvJoy4 + 2,	"p4 down"},
	{"P4 Left",		BIT_DIGITAL,	DrvJoy4 + 1,	"p4 left"},
	{"P4 Right",	BIT_DIGITAL,	DrvJoy4 + 0,	"p4 right"},
	{"P4 Button 1",	BIT_DIGITAL,	DrvJoy4 + 7,	"p4 fire 1"},
	{"P4 Button 2",	BIT_DIGITAL,	DrvJoy4 + 6,	"p4 fire 2"},

	{"Reset",		BIT_DIGITAL,	&DrvReset,		"reset"},
	{"Dip A",		BIT_DIPSWITCH,	DrvInput + 5,	"dip"},
	{"Dip B",		BIT_DIPSWITCH,	DrvInput + 6,	"dip"},
	{"Dip C",		BIT_DIPSWITCH,	DrvInput + 7,	"dip"},
};

STDINPUTINFO(hook)

static struct BurnDIPInfo hookDIPList[] = {

	// Defaults
	{0x21,	0xFF, 0xFF,	0x00, NULL},
	{0x22,	0xFF, 0xFF,	0x00, NULL},
	{0x23,	0xFF, 0xFF,	0x00, NULL},

	{0,		0xFE, 0,	2,	  "Flip Screen"},
	{0x21,	0x01, 0x01,	0x00, "Off"},
	{0x21,	0x01, 0x01,	0x01, "On"},
	{0,		0xFE, 0,	2,	  "Cabinet"},
	{0x21,	0x01, 0x02,	0x00, "2 Players"},
	{0x21,	0x01, 0x02,	0x02, "4 Players"},
	{0,		0xFE, 0,	2,	  "Coin Slots"},
	{0x21,	0x01, 0x04,	0x00, "Common"},
	{0x21,	0x01, 0x04,	0x04, "Separate"},
	{0,		0xFE, 0,	2,	  "Coin Mode"},
	{0x21,	0x01, 0x08,	0x00, "1"},
	{0x21,	0x01, 0x08,	0x08, "2"},
	{0,		0xFE, 0,	16,	  "Coinage"},
	{0x21,	0x01, 0xf0,	0x00, "1C_1C"},
	{0x21,	0x01, 0xf0,	0x10, "2C_1C"},
	{0x21,	0x01, 0xf0,	0x20, "3C_1C"},
	{0x21,	0x01, 0xf0,	0x30, "4C_1C"},
	{0x21,	0x01, 0xf0,	0x40, "5C_1C"},
	{0x21,	0x01, 0xf0,	0x50, "6C_1C"},
	{0x21,	0x01, 0xf0,	0x60, "1C_2C"},
	{0x21,	0x01, 0xf0,	0x70, "1C_3C"},
	{0x21,	0x01, 0xf0,	0x80, "1C_4C"},
	{0x21,	0x01, 0xf0,	0x90, "1C_5C"},
	{0x21,	0x01, 0xf0,	0xa0, "1C_6C"},
	{0x21,	0x01, 0xf0,	0xb0, "2C_3C"},
	{0x21,	0x01, 0xf0,	0xc0, "3C_2C"},
	{0x21,	0x01, 0xf0,	0xd0, "4C_3C"},
	{0x21,	0x01, 0xf0,	0xe0, "2 Coins to Start, 1 to Continue"},
	{0x21,	0x01, 0xf0,	0xf0, "Free Play"},

	// DIP 2
	{0,		0xFE, 0,	4,	  "Lives"},
	{0x22,	0x01, 0x03, 0x00, "2"},
	{0x22,	0x01, 0x03, 0x01, "3"},
	{0x22,	0x01, 0x03, 0x02, "4"},
	{0x22,	0x01, 0x03, 0x03, "1"},
	{0,		0xFE, 0,	4,	  "Difficulty"},
	{0x22,	0x01, 0x0C, 0x00, "Normal"},
	{0x22,	0x01, 0x0C, 0x04, "Easy"},
	{0x22,	0x01, 0x0C, 0x08, "Hard"},
	{0x22,	0x01, 0x0C, 0x0C, "Very Easy"},
	{0,		0xFE, 0,	2,	  "Any Button to Start"},
	{0x22,	0x01, 0x20, 0x00, "Yes"},
	{0x22,	0x01, 0x20, 0x20, "No"},
	{0,		0xFE, 0,	2,	  "Demo Sounds"},
	{0x22,	0x01, 0x40, 0x00, "Off"},
	{0x22,	0x01, 0x40, 0x40, "On"},
	{0,		0xFE, 0,	2,	  "Service"},
	{0x22,	0x01, 0x80, 0x00, "Off"},
	{0x22,	0x01, 0x80, 0x80, "On"},

};

STDDIPINFO(hook)


static struct BurnInputInfo inthuntInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvButton + 2,	"p1 coin"},
	{"P1 Start",	BIT_DIGITAL,	DrvButton + 0,	"p1 start"},

	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 up"},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 down"},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 left"},
	{"P1 Right",	BIT_DIGITAL,	DrvJoy1 + 0,	"p1 right"},
	{"P1 Button 1",	BIT_DIGITAL,	DrvJoy1 + 7,	"p1 fire 1"},
	{"P1 Button 2",	BIT_DIGITAL,	DrvJoy1 + 6,	"p1 fire 2"},

	{"P2 Coin",		BIT_DIGITAL,	DrvButton + 3,	"p2 coin"},
	{"P2 Start",	BIT_DIGITAL,	DrvButton + 1,	"p2 start"},

	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 up"},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 down"},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 left"},
	{"P2 Right",	BIT_DIGITAL,	DrvJoy2 + 0,	"p2 right"},
	{"P2 Button 1",	BIT_DIGITAL,	DrvJoy2 + 7,	"p2 fire 1"},
	{"P2 Button 2",	BIT_DIGITAL,	DrvJoy2 + 6,	"p2 fire 2"},

	{"Reset",		BIT_DIGITAL,	&DrvReset,		"reset"},
	{"Dip A",		BIT_DIPSWITCH,	DrvInput + 5,	"dip"},
	{"Dip B",		BIT_DIPSWITCH,	DrvInput + 6,	"dip"},
	{"Dip C",		BIT_DIPSWITCH,	DrvInput + 7,	"dip"},
};

STDINPUTINFO(inthunt)

static struct BurnDIPInfo inthuntDIPList[] = {

	// Defaults
	{0x11,	0xFF, 0xFF,	0x00, NULL},
	{0x12,	0xFF, 0xFF,	0x00, NULL},
	{0x13,	0xFF, 0xFF,	0x00, NULL},

	{0,		0xFE, 0,	2,	  "Flip Screen"},
	{0x11,	0x01, 0x01,	0x00, "Off"},
	{0x11,	0x01, 0x01,	0x01, "On"},
//	{0,		0xFE, 0,	2,	  "Unknown"},
//	{0x11,	0x01, 0x02,	0x00, "Off"},
//	{0x11,	0x01, 0x02,	0x02, "On"},
	{0,		0xFE, 0,	2,	  "Coin Slots"},
	{0x11,	0x01, 0x04,	0x00, "Common"},
	{0x11,	0x01, 0x04,	0x04, "Separate"},
	{0,		0xFE, 0,	2,	  "Coin Mode"},
	{0x11,	0x01, 0x08,	0x00, "1"},
	{0x11,	0x01, 0x08,	0x08, "2"},
	{0,		0xFE, 0,	16,	  "Coinage"},
	{0x11,	0x01, 0xf0,	0x00, "1C_1C"},
	{0x11,	0x01, 0xf0,	0x10, "2C_1C"},
	{0x11,	0x01, 0xf0,	0x20, "3C_1C"},
	{0x11,	0x01, 0xf0,	0x30, "4C_1C"},
	{0x11,	0x01, 0xf0,	0x40, "5C_1C"},
	{0x11,	0x01, 0xf0,	0x50, "6C_1C"},
	{0x11,	0x01, 0xf0,	0x60, "1C_2C"},
	{0x11,	0x01, 0xf0,	0x70, "1C_3C"},
	{0x11,	0x01, 0xf0,	0x80, "1C_4C"},
	{0x11,	0x01, 0xf0,	0x90, "1C_5C"},
	{0x11,	0x01, 0xf0,	0xa0, "1C_6C"},
	{0x11,	0x01, 0xf0,	0xb0, "2C_3C"},
	{0x11,	0x01, 0xf0,	0xc0, "3C_2C"},
	{0x11,	0x01, 0xf0,	0xd0, "4C_3C"},
	{0x11,	0x01, 0xf0,	0xe0, "2 Coins to Start, 1 to Continue"},
	{0x11,	0x01, 0xf0,	0xf0, "Free Play"},

	// DIP 2
	{0,		0xFE, 0,	4,	  "Lives"},
	{0x12,	0x01, 0x03, 0x00, "2"},
	{0x12,	0x01, 0x03, 0x01, "3"},
	{0x12,	0x01, 0x03, 0x02, "4"},
	{0x12,	0x01, 0x03, 0x03, "1"},
	{0,		0xFE, 0,	4,	  "Difficulty"},
	{0x12,	0x01, 0x0C, 0x00, "Normal"},
	{0x12,	0x01, 0x0C, 0x04, "Easy"},
	{0x12,	0x01, 0x0C, 0x08, "Hard"},
	{0x12,	0x01, 0x0C, 0x0C, "Very Easy"},
	{0,		0xFE, 0,	2,	  "Any Button to Start"},
	{0x12,	0x01, 0x20, 0x00, "Yes"},
	{0x12,	0x01, 0x20, 0x20, "No"},
	{0,		0xFE, 0,	2,	  "Demo Sounds"},
	{0x12,	0x01, 0x40, 0x00, "Off"},
	{0x12,	0x01, 0x40, 0x40, "On"},
	{0,		0xFE, 0,	2,	  "Service"},
	{0x12,	0x01, 0x80, 0x00, "Off"},
	{0x12,	0x01, 0x80, 0x80, "On"},

};

STDDIPINFO(inthunt)

// Rom information

static struct BurnRomInfo hookRomDesc[] = {
	{ "h-h0-d.3h",		0x040000, 0x40189ff6, BRF_ESS | BRF_PRG },	// CPU 0, V33
	{ "h-l0-d.3h",		0x040000, 0x14567690, BRF_ESS | BRF_PRG },
	{ "h-h1.rom",		0x020000, 0x264ba1f0, BRF_ESS | BRF_PRG },
	{ "h-l1.rom",		0x020000, 0xf9913731, BRF_ESS | BRF_PRG },

	{ "h-sh0.rom",		0x010000, 0x86a4e56e, BRF_ESS | BRF_PRG },	// CPU 1, V30
	{ "h-sl0.rom",		0x010000, 0x10fd9676, BRF_ESS | BRF_PRG },

	{ "hook-c0.rom",	0x040000, 0xdec63dcf, BRF_GRA }, 			// Tiles
	{ "hook-c1.rom",	0x040000, 0xe4eb0b92, BRF_GRA },
	{ "hook-c2.rom",	0x040000, 0xa52b320b, BRF_GRA },
	{ "hook-c3.rom",	0x040000, 0x7ef67731, BRF_GRA },

	{ "hook-000.rom",	0x100000, 0xccceac30, BRF_GRA },        	// Sprites
	{ "hook-010.rom",	0x100000, 0x8ac8da67, BRF_GRA },
	{ "hook-020.rom",	0x100000, 0x8847af9a, BRF_GRA },
	{ "hook-030.rom",	0x100000, 0x239e877e, BRF_GRA },

	{ "hook-da.rom",	0x080000, 0x88cd0212, BRF_SND }, 			// Sound
};

STD_ROM_PICK(hook)
STD_ROM_FN(hook)

static struct BurnRomInfo hookuRomDesc[] = {
	{ "h-h0-c.3h",		0x040000, 0x84cc239e, BRF_ESS | BRF_PRG },	// CPU 0, V33
	{ "h-l0-c.5h",		0x040000, 0x45e194fe, BRF_ESS | BRF_PRG },
	{ "h-h1.rom",		0x020000, 0x264ba1f0, BRF_ESS | BRF_PRG },
	{ "h-l1.rom",		0x020000, 0xf9913731, BRF_ESS | BRF_PRG },

	{ "h-sh0.rom",		0x010000, 0x86a4e56e, BRF_ESS | BRF_PRG },	// CPU 1, V30
	{ "h-sl0.rom",		0x010000, 0x10fd9676, BRF_ESS | BRF_PRG },

	{ "hook-c0.rom",	0x040000, 0xdec63dcf, BRF_GRA }, 			// Tiles
	{ "hook-c1.rom",	0x040000, 0xe4eb0b92, BRF_GRA },
	{ "hook-c2.rom",	0x040000, 0xa52b320b, BRF_GRA },
	{ "hook-c3.rom",	0x040000, 0x7ef67731, BRF_GRA },

	{ "hook-000.rom",	0x100000, 0xccceac30, BRF_GRA },        	// Sprites
	{ "hook-010.rom",	0x100000, 0x8ac8da67, BRF_GRA },
	{ "hook-020.rom",	0x100000, 0x8847af9a, BRF_GRA },
	{ "hook-030.rom",	0x100000, 0x239e877e, BRF_GRA },

	{ "hook-da.rom",	0x080000, 0x88cd0212, BRF_SND }, 			// Sound
};

STD_ROM_PICK(hooku)
STD_ROM_FN(hooku)

static struct BurnRomInfo hookjRomDesc[] = {
	{ "h-h0-g.3h",		0x040000, 0x5964c886, BRF_ESS | BRF_PRG },	// CPU 0, V33
	{ "h-l0-g.5h",		0x040000, 0x7f7433f2, BRF_ESS | BRF_PRG },
	{ "h-h1.rom",		0x020000, 0x264ba1f0, BRF_ESS | BRF_PRG },
	{ "h-l1.rom",		0x020000, 0xf9913731, BRF_ESS | BRF_PRG },

	{ "h-sh0-a.3l",		0x010000, 0xbd3d1f61, BRF_ESS | BRF_PRG },	// CPU 1, V30
	{ "h-sl0-a.3n",		0x010000, 0x76371def, BRF_ESS | BRF_PRG },

	{ "hook-c0.rom",	0x040000, 0xdec63dcf, BRF_GRA }, 			// Tiles
	{ "hook-c1.rom",	0x040000, 0xe4eb0b92, BRF_GRA },
	{ "hook-c2.rom",	0x040000, 0xa52b320b, BRF_GRA },
	{ "hook-c3.rom",	0x040000, 0x7ef67731, BRF_GRA },

	{ "hook-000.rom",	0x100000, 0xccceac30, BRF_GRA },        	// Sprites
	{ "hook-010.rom",	0x100000, 0x8ac8da67, BRF_GRA },
	{ "hook-020.rom",	0x100000, 0x8847af9a, BRF_GRA },
	{ "hook-030.rom",	0x100000, 0x239e877e, BRF_GRA },

	{ "hook-da.rom",	0x080000, 0x88cd0212, BRF_SND }, 			// Sound
};

STD_ROM_PICK(hookj)
STD_ROM_FN(hookj)

static struct BurnRomInfo inthuntRomDesc[] = {
	{ "ith-h0-d.bin",	0x040000, 0x52f8e7a6, BRF_ESS | BRF_PRG },	// CPU 0, V33
	{ "ith-l0-d.bin",	0x040000, 0x5db79eb7, BRF_ESS | BRF_PRG },
	{ "ith-h1-b.bin",	0x020000, 0xfc2899df, BRF_ESS | BRF_PRG },
	{ "ith-l1-b.bin",	0x020000, 0x955a605a, BRF_ESS | BRF_PRG },

	{ "ith-sh0.rom",	0x010000, 0x209c8b7f, BRF_ESS | BRF_PRG },	// CPU 1, V30
	{ "ith-sl0.rom",	0x010000, 0x18472d65, BRF_ESS | BRF_PRG },

	{ "ith_ic26.rom",	0x080000, 0x4c1818cf, BRF_GRA }, 			// Tiles
	{ "ith_ic25.rom",	0x080000, 0x91145bae, BRF_GRA },
	{ "ith_ic24.rom",	0x080000, 0xfc03fe3b, BRF_GRA },
	{ "ith_ic23.rom",	0x080000, 0xee156a0a, BRF_GRA },

	{ "ith_ic34.rom",	0x100000, 0xa019766e, BRF_GRA },        	// Sprites
	{ "ith_ic35.rom",	0x100000, 0x3fca3073, BRF_GRA },
	{ "ith_ic36.rom",	0x100000, 0x20d1b28b, BRF_GRA },
	{ "ith_ic37.rom",	0x100000, 0x90b6fd4b, BRF_GRA },

	{ "ith_ic9.rom",	0x080000, 0x318ee71a, BRF_SND }, 			// Sound
};

STD_ROM_PICK(inthunt)
STD_ROM_FN(inthunt)

static struct BurnRomInfo inthuntuRomDesc[] = {
	{ "ith-h0-c.bin",	0x040000, 0x563dcec0, BRF_ESS | BRF_PRG },	// CPU 0, V33
	{ "ith-l0-c.bin",	0x040000, 0x1638c705, BRF_ESS | BRF_PRG },
	{ "ith-h1-a.bin",	0x020000, 0x0253065f, BRF_ESS | BRF_PRG },
	{ "ith-l1-a.bin",	0x020000, 0xa57d688d, BRF_ESS | BRF_PRG },

	{ "ith-sh0.rom",	0x010000, 0x209c8b7f, BRF_ESS | BRF_PRG },	// CPU 1, V30
	{ "ith-sl0.rom",	0x010000, 0x18472d65, BRF_ESS | BRF_PRG },

	{ "ith_ic26.rom",	0x080000, 0x4c1818cf, BRF_GRA }, 			// Tiles
	{ "ith_ic25.rom",	0x080000, 0x91145bae, BRF_GRA },
	{ "ith_ic24.rom",	0x080000, 0xfc03fe3b, BRF_GRA },
	{ "ith_ic23.rom",	0x080000, 0xee156a0a, BRF_GRA },

	{ "ith_ic34.rom",	0x100000, 0xa019766e, BRF_GRA },        	// Sprites
	{ "ith_ic35.rom",	0x100000, 0x3fca3073, BRF_GRA },
	{ "ith_ic36.rom",	0x100000, 0x20d1b28b, BRF_GRA },
	{ "ith_ic37.rom",	0x100000, 0x90b6fd4b, BRF_GRA },

	{ "ith_ic9.rom",	0x080000, 0x318ee71a, BRF_SND }, 			// Sound
};

STD_ROM_PICK(inthuntu)
STD_ROM_FN(inthuntu)

static struct BurnRomInfo kaiteidsRomDesc[] = {
	{ "ith-h0j.bin",	0x040000, 0xdc1dec36, BRF_ESS | BRF_PRG },	// CPU 0, V33
	{ "ith-l0j.bin",	0x040000, 0x8835d704, BRF_ESS | BRF_PRG },
	{ "ith-h1j.bin",	0x020000, 0x5a7b212d, BRF_ESS | BRF_PRG },
	{ "ith-l1j.bin",	0x020000, 0x4c084494, BRF_ESS | BRF_PRG },

	{ "ith-sh0.rom",	0x010000, 0x209c8b7f, BRF_ESS | BRF_PRG },	// CPU 1, V30
	{ "ith-sl0.rom",	0x010000, 0x18472d65, BRF_ESS | BRF_PRG },

	{ "ith_ic26.rom",	0x080000, 0x4c1818cf, BRF_GRA }, 			// Tiles
	{ "ith_ic25.rom",	0x080000, 0x91145bae, BRF_GRA },
	{ "ith_ic24.rom",	0x080000, 0xfc03fe3b, BRF_GRA },
	{ "ith_ic23.rom",	0x080000, 0xee156a0a, BRF_GRA },

	{ "ith_ic34.rom",	0x100000, 0xa019766e, BRF_GRA },        	// Sprites
	{ "ith_ic35.rom",	0x100000, 0x3fca3073, BRF_GRA },
	{ "ith_ic36.rom",	0x100000, 0x20d1b28b, BRF_GRA },
	{ "ith_ic37.rom",	0x100000, 0x90b6fd4b, BRF_GRA },

	{ "ith_ic9.rom",	0x080000, 0x318ee71a, BRF_SND }, 			// Sound
};

STD_ROM_PICK(kaiteids)
STD_ROM_FN(kaiteids)

static struct BurnRomInfo rtypeleoRomDesc[] = {
	{ "rtl-h0-c.bin",	0x040000, 0x5fef7fa1, BRF_ESS | BRF_PRG },	// CPU 0, V33
	{ "rtl-l0-c.bin",	0x040000, 0x8156456b, BRF_ESS | BRF_PRG },
	{ "rtl-h1-d.bin",	0x020000, 0x352ff444, BRF_ESS | BRF_PRG },
	{ "rtl-l1-d.bin",	0x020000, 0xfd34ea46, BRF_ESS | BRF_PRG },

	{ "rtl-sh0a.bin",	0x010000, 0xe518b4e3, BRF_ESS | BRF_PRG },	// CPU 1, V30
	{ "rtl-sl0a.bin",	0x010000, 0x896f0d36, BRF_ESS | BRF_PRG },

	{ "rtl-c0.bin",		0x080000, 0xfb588d7c, BRF_GRA }, 			// Tiles
	{ "rtl-c1.bin",		0x080000, 0xe5541bff, BRF_GRA },
	{ "rtl-c2.bin",		0x080000, 0xfaa9ae27, BRF_GRA },
	{ "rtl-c3.bin",		0x080000, 0x3a2343f6, BRF_GRA },

	{ "rtl-000.bin",	0x100000, 0x82a06870, BRF_GRA },        	// Sprites
	{ "rtl-010.bin",	0x100000, 0x417e7a56, BRF_GRA },
	{ "rtl-020.bin",	0x100000, 0xf9a3f3a1, BRF_GRA },
	{ "rtl-030.bin",	0x100000, 0x03528d95, BRF_GRA },

	{ "rtl-da.bin",		0x080000, 0xdbebd1ff, BRF_SND }, 			// Sound
};

STD_ROM_PICK(rtypeleo)
STD_ROM_FN(rtypeleo)

static struct BurnRomInfo rtypelejRomDesc[] = {
	{ "rtl-h0-d.bin",	0x040000, 0x3dbac89f, BRF_ESS | BRF_PRG },	// CPU 0, V33
	{ "rtl-l0-d.bin",	0x040000, 0xf85a2537, BRF_ESS | BRF_PRG },
	{ "rtl-h1-d.bin",	0x020000, 0x352ff444, BRF_ESS | BRF_PRG },
	{ "rtl-l1-d.bin",	0x020000, 0xfd34ea46, BRF_ESS | BRF_PRG },

	{ "rtl-sh0a.bin",	0x010000, 0xe518b4e3, BRF_ESS | BRF_PRG },	// CPU 1, V30
	{ "rtl-sl0a.bin",	0x010000, 0x896f0d36, BRF_ESS | BRF_PRG },

	{ "rtl-c0.bin",		0x080000, 0xfb588d7c, BRF_GRA }, 			// Tiles
	{ "rtl-c1.bin",		0x080000, 0xe5541bff, BRF_GRA },
	{ "rtl-c2.bin",		0x080000, 0xfaa9ae27, BRF_GRA },
	{ "rtl-c3.bin",		0x080000, 0x3a2343f6, BRF_GRA },

	{ "rtl-000.bin",	0x100000, 0x82a06870, BRF_GRA },        	// Sprites
	{ "rtl-010.bin",	0x100000, 0x417e7a56, BRF_GRA },
	{ "rtl-020.bin",	0x100000, 0xf9a3f3a1, BRF_GRA },
	{ "rtl-030.bin",	0x100000, 0x03528d95, BRF_GRA },

	{ "rtl-da.bin",		0x080000, 0xdbebd1ff, BRF_SND }, 			// Sound
};

STD_ROM_PICK(rtypelej)
STD_ROM_FN(rtypelej)

// Irem GA20 Sound chip

#define MAX_VOL 256

static struct IremGA20_chip
{
	//const struct IremGA20_interface *intf;
	//unsigned char *rom;
	//int rom_size;
	//sound_stream * stream;
	unsigned char regs[0x40];
	struct IremGA20_channel {
		unsigned int rate;
		unsigned int size;
		unsigned int start;
		unsigned int pos;
		unsigned int frac;
		unsigned int end;
		unsigned int volume;
		unsigned int pan;
		unsigned int effect;
		unsigned int play;
	} channel[4];
} * chip = 0;

unsigned char IremGA20_r(int offset)
{
	int chl = offset >> 4;
	switch (offset & 0xf) {
		case 0xe:	// voice status.  bit 0 is 1 if active. (routine around 0xccc in rtypeleo)
			return chip->channel[chl].play ? 1 : 0;
			break;

		default:
			//logerror("GA20: read unk. register %d, channel %d\n", offset & 0xf, channel);
			break;
	}
	return 0;
}

void IremGA20_w(int offset, unsigned char data)
{
	chip->regs[offset] = data;
	int chl = offset >> 4;
	switch ( offset & 0xf ) {
		case 0x0:
			chip->channel[chl].start = ((chip->channel[chl].start)&0xff000) | (data<< 4);
			break;
		case 0x2:
			chip->channel[chl].start = ((chip->channel[chl].start)&0x00ff0) | (data<<12);
			break;
		case 0x4:
			chip->channel[chl].end = ((chip->channel[chl].end)&0xff000) | (data<< 4);
			break;
		case 0x6:
			chip->channel[chl].end = ((chip->channel[chl].end)&0x00ff0) | (data<<12);
			break;
		case 0x8:
			chip->channel[chl].rate = 0x1000000 / (256 - data);
			break;
		case 0xa:
			chip->channel[chl].volume = (data * MAX_VOL) / (data + 10);
			break;
		case 0xc:
			chip->channel[chl].play = data;
			chip->channel[chl].pos = chip->channel[chl].start;
			chip->channel[chl].frac = 0;
			break;
	}

}

int IremGA20_update(short * /*pBuffer*/, int /*len*/)
{
#if 0
	bprintf(PRINT_NORMAL, _T("GA20 %d%d%d%d\n"),
			chip->channel[0].play,
			chip->channel[1].play,
			chip->channel[2].play,
			chip->channel[3].play );
#endif
	return 0;
}

static void snd_update_irq()
{
	if (irqvector & 0x2)		/* YM2151 has precedence */
		VezSetIRQLine(0x18 * 4, VEZ_IRQSTATUS_ACK);
	else if (irqvector & 0x1)	/* V30 */
		VezSetIRQLine(0x19 * 4, VEZ_IRQSTATUS_ACK);

}

static void m92YM2151IRQHandler(int nStatus)
{
//	bprintf(PRINT_NORMAL, _T("m92YM2151IRQHandler nStatus %x\n"), nStatus);

	if (nStatus)irqvector |= 2;
	else		irqvector &= ~2;
	snd_update_irq();
}


unsigned char __fastcall m92ReadByte(unsigned int vezAddress)
{
	// Palette Read
	if ((vezAddress & 0xFF800) == 0xF8800 )
		return RamPal[ vezAddress - 0xF8800 + PalBank ];

	// V33 Start vector
	if ((vezAddress & 0xFFFF0) == 0xFFFF0 )
		return RomV33[ vezAddress - 0xFFFF0 + 0x7FFF0 ];

//	switch (vezAddress) {

//		default:
//			bprintf(PRINT_NORMAL, _T("Attempt to read byte value of location %x\n"), vezAddress);
//	}
	return 0;
}

void __fastcall m92WriteByte(unsigned int vezAddress, unsigned char byteValue)
{
	static unsigned int sprite_extent;

	if ((vezAddress & 0xFF800) == 0xF8800 ) {
		RamPal[ vezAddress - 0xF8800 + PalBank ] = byteValue;
		if (vezAddress & 1) {
			int offs = (vezAddress - 0xF8800 + PalBank) >> 1;
			RamCurPal[ offs ] = CalcCol( offs << 1 );
		}
		return;
	}

	switch (vezAddress) {
		// 0xf9000 ~ 0xf900f : m92_spritecontrol_w
		case 0xF9000:
			sprite_extent = byteValue;
			break;
		case 0xF9004:
			if (byteValue==8)
				m92_sprite_list=(((0x100 - sprite_extent)&0xff)*8);
			else
				m92_sprite_list=0x800;
			break;
		case 0xF9008:
			/* Pixel clock is 26.6666 MHz, we have 0x800 bytes, or 0x400 words
	           to copy from spriteram to the buffer.  It seems safe to assume 1
	           word can be copied per clock.  So:

	           1 MHz clock would be 1 word every 0.000,001s = 1000ns
	           26.6666MHz clock would be 1 word every 0.000,000,037 = 37 ns
	           Buffer should copy in about 37888 ns. */

			m92_sprite_buffer_busy = 0;
			m92_sprite_buffer_timer = VezCurrentCPU->reg.nec_ICount - 341;	// 9000000 * 0x400 * 0.000000037;
			VezCurrentCPU->reg.nec_ICount = 341;

			//bprintf(PRINT_NORMAL, _T("sprite buffer copy timer start %f\n"), 100.0 * VezSegmentCycles() / (9000000 / 60)  );

			//memcpy(RamSprCpy, RamSpr, 0x800);

			break;

		case 0xF9800:
			PalBank = (byteValue & 0x2 /* && m92_game_kludge!=3 */) ? 0x0800 : 0x0000;
			break;

		case 0xF9001:
		case 0xF9002:
		case 0xF9003:
		case 0xF9005:
		case 0xF9006:
		case 0xF9007:
		case 0xF9009:
		case 0xF900a:
		case 0xF900b:
		case 0xF9801:
			//if (!byteValue)
			break;
//		default:
			//if (vezAddress >= 0xA0000)
//			bprintf(PRINT_NORMAL, _T("Attempt to write byte value %x to location %x\n"), byteValue, vezAddress);
	}
}

unsigned char __fastcall m92ReadPort(unsigned int vezPort)
{
	switch (vezPort) {
		case 0x00: return ~DrvInput[0];	// player 1
		case 0x01: return ~DrvInput[1];	// player 2
		case 0x03: return ~DrvInput[7];	// dip 3
		case 0x04: return ~DrvInput[6];	// dip 2
		case 0x05: return ~DrvInput[5];	// dip 1
		case 0x06: return ~DrvInput[2];	// player 3
		case 0x07: return ~DrvInput[3];	// player 4

		case 0x08: return sound_status[0];
		case 0x09: return sound_status[1];

		case 0x88: return 0xFF;
		case 0x02: return (~DrvInput[4] & 0x7F) | m92_sprite_buffer_busy;
//		default:
//			bprintf(PRINT_NORMAL, _T("Attempt to read byte value of port %x\n"), vezPort);
	}
	return 0;
}

void __fastcall m92WritePort(unsigned int vezPort, unsigned char byteValue)
{
	switch (vezPort) {
		case 0x00:
			sound_latch[0] = byteValue;
			irqvector |= 1;
			snd_update_irq();
			break;
		case 0x01:
			sound_latch[1] = byteValue;
			break;
		case 0x02:
		case 0x03:
			//m92_coincounter_w

			break;

		case 0x20:
			// m92_bankswitch_w
//			bprintf(PRINT_NORMAL, _T("CPU 0 bank change to %02x\n"), byteValue);
			VezMapArea(0xa0000, 0xbffff, 0, RomV33 + 0x100000 + (byteValue&0x7)*0x10000);
			VezMapArea(0xa0000, 0xbffff, 2, RomV33 + 0x100000 + (byteValue&0x7)*0x10000);
			break;
		case 0x21:
			// m92_bankswitch_w
			break;

		case 0x40:
		case 0x41:
		case 0x42:
		case 0x43:
			// Interrupt controller, only written to at bootup
			break;
		// 0x80 ~ 0x87: m92_pf1_control_w
		case 0x80: pf1_control[0] = byteValue; break;
		case 0x81: pf1_control[1] = byteValue; break;
		case 0x82: pf1_control[2] = byteValue; break;
		case 0x83: pf1_control[3] = byteValue; break;
		case 0x84: pf1_control[4] = byteValue; break;
		case 0x85: pf1_control[5] = byteValue; break;
		case 0x86: pf1_control[6] = byteValue; break;
		case 0x87: pf1_control[7] = byteValue; break;
		// 0x88 ~ 0x8f: m92_pf2_control_w
		case 0x88: pf2_control[0] = byteValue; break;
		case 0x89: pf2_control[1] = byteValue; break;
		case 0x8a: pf2_control[2] = byteValue; break;
		case 0x8b: pf2_control[3] = byteValue; break;
		case 0x8c: pf2_control[4] = byteValue; break;
		case 0x8d: pf2_control[5] = byteValue; break;
		case 0x8e: pf2_control[6] = byteValue; break;
		case 0x8f: pf2_control[7] = byteValue; break;
		// 0x90 ~ 0x97: m92_pf3_control_w
		case 0x90: pf3_control[0] = byteValue; break;
		case 0x91: pf3_control[1] = byteValue; break;
		case 0x92: pf3_control[2] = byteValue; break;
		case 0x93: pf3_control[3] = byteValue; break;
		case 0x94: pf3_control[4] = byteValue; break;
		case 0x95: pf3_control[5] = byteValue; break;
		case 0x96: pf3_control[6] = byteValue; break;
		case 0x97: pf3_control[7] = byteValue; break;

		// 0x98 ~ 0x9f: m92_master_control_w
		case 0x98:
			pf4_control[0] = byteValue;
			pf1_enable = (byteValue & 0x10) ? 0 : 1;
			pf1_rowscroll = (byteValue & 0x40) >> 6;
			pf1_shape = (byteValue & 0x04) >> 2;
			pf1_vram_ptr = 0x4000 * (byteValue & 0x03);
			break;
		case 0x99: pf4_control[1] = byteValue; break;
		case 0x9a:
			pf4_control[2] = byteValue;
			pf2_enable = (byteValue & 0x10) ? 0 : 1;
			pf2_rowscroll = (byteValue & 0x40) >> 6;
			pf2_shape = (byteValue & 0x04) >> 2;
			pf2_vram_ptr = 0x4000 * (byteValue & 0x03);
			break;
		case 0x9b: pf4_control[3] = byteValue; break;
		case 0x9c:
			pf4_control[4] = byteValue;
			pf3_enable = (byteValue & 0x10) ? 0 : 1;
			pf3_rowscroll = (byteValue & 0x40) >> 6;
			pf3_shape = (byteValue & 0x04) >> 2;
			pf3_vram_ptr = 0x4000 * (byteValue & 0x03);
			break;
		case 0x9d: pf4_control[5] = byteValue; break;
		case 0x9e:
			pf4_control[6] = byteValue;
			m92_raster_irq_position = ((pf4_control[7]<<8) | pf4_control[6]) - 128;
			break;
		case 0x9f:
			pf4_control[7] = byteValue;
			m92_raster_irq_position = ((pf4_control[7]<<8) | pf4_control[6]) - 128;
			//bprintf(PRINT_NORMAL, _T("m92_raster_irq_position set to %d\n"), m92_raster_irq_position);
			break;

//		default:
//			bprintf(PRINT_NORMAL, _T("Attempt to write byte value %x to port %x\n"), byteValue, vezPort);
	}
}

unsigned char __fastcall m92SndReadByte(unsigned int vezAddress)
{
	if (vezAddress >= 0xa8000 && vezAddress <= 0xa803f )
		return  IremGA20_r( vezAddress - 0xa8000 );

	switch (vezAddress) {

		case 0xa8042:
			return BurnYM2151ReadStatus();
		//case 0xa8043:
		//	YM2151_status_port_0_r
		//	break;

		case 0xa8044:
			return sound_latch[0];
		case 0xa8045:
			return sound_latch[1];

//		default:
//			bprintf(PRINT_NORMAL, _T("V30 Attempt to read byte value of location %x\n"), vezAddress);
	}
	return 0;
}

void __fastcall m92SndWriteByte(unsigned int vezAddress, unsigned char byteValue)
{
	if (vezAddress >= 0xa8000 && vezAddress <= 0xa803f ) {
		IremGA20_w( vezAddress - 0xa8000, byteValue );
		return;
	}

	if (vezAddress >= 0x9ff00 && vezAddress <= 0x9ffff ) {
		// NOP
		return;
	}

	switch (vezAddress) {
		case 0xa8040:
			BurnYM2151SelectRegister(byteValue);
			break;
		//case 0xa8041:
		//	YM2151_register_port_0_w
		//	break;

		case 0xa8042:
			BurnYM2151WriteRegister(byteValue);
			break;
		//case 0xa8043:
		//	YM2151_data_port_0_w
		//	break;

		case 0xa8044:
			irqvector &= ~1;
			snd_update_irq();
			break;
		//case 0xa8045:
		//	m92_sound_irq_ack_w
		//	break;

		case 0xa8046:
			// m92_sound_status_w
			sound_status[0] = byteValue;

		//	VezOpen(0);
		//	VezSetIRQLine(m92_irq_vectorbase + 12, VEZ_IRQSTATUS_ACK); // IRQ 3
		//	VezRun(400);
		//	VezOpen(1);

			//bprintf(PRINT_NORMAL, _T("m92_sound_status_w %x, and set M92_IRQ_3\n"), byteValue);

			break;
		case 0xa8047:
			sound_status[1] = byteValue;
			break;

//		default:
//			bprintf(PRINT_NORMAL, _T("V30 Attempt to write byte value %x to location %x\n"), byteValue, vezAddress);
	}
}

static int DrvDoReset()
{
	VezOpen(0);
	VezReset();

	VezOpen(1);
	VezReset();

	BurnYM2151Reset();

	memset(pf1_control, 0, sizeof(pf1_control));
	memset(pf2_control, 0, sizeof(pf2_control));
	memset(pf3_control, 0, sizeof(pf3_control));
	memset(pf4_control, 0, sizeof(pf4_control));

	pf1_enable = pf2_enable = pf3_enable = 0;
	pf1_shape = pf2_shape = pf3_shape = 0;
	pf1_rowscroll = pf2_rowscroll = pf3_rowscroll = 0;
	pf1_vram_ptr = pf2_vram_ptr = pf3_vram_ptr = 0;

	m92_sprite_buffer_busy = 0x80;
	m92_sprite_buffer_timer = 0;

	irqvector = 0;

	return 0;
}

static int loadDecodeGfx01(unsigned char *tmp, int rid, int shift, int size)
{
	unsigned char * pgfx = RomGfx01;

	BurnLoadRom(tmp, rid, 1);

	for (int i=0; i<(size/8); i++) {
		for( int y=0;y<8;y++) {
			pgfx[0] |= ((tmp[0]>>7)&1)<<shift;
			pgfx[1] |= ((tmp[0]>>6)&1)<<shift;
			pgfx[2] |= ((tmp[0]>>5)&1)<<shift;
			pgfx[3] |= ((tmp[0]>>4)&1)<<shift;
			pgfx[4] |= ((tmp[0]>>3)&1)<<shift;
			pgfx[5] |= ((tmp[0]>>2)&1)<<shift;
			pgfx[6] |= ((tmp[0]>>1)&1)<<shift;
			pgfx[7] |= ((tmp[0]>>0)&1)<<shift;
			tmp ++;
			pgfx += 8;
		}
	}
	return 0;
}

static int loadDecodeGfx02(unsigned char *tmp, int rid, int shift, int size)
{
	unsigned char * pgfx = RomGfx02;

	BurnLoadRom(tmp, rid, 1);

	for (int i=0; i<(size/32); i++) {
		for( int y=0;y<16;y++) {
			pgfx[ 0] |= ((tmp[ 0]>>7)&1)<<shift;
			pgfx[ 1] |= ((tmp[ 0]>>6)&1)<<shift;
			pgfx[ 2] |= ((tmp[ 0]>>5)&1)<<shift;
			pgfx[ 3] |= ((tmp[ 0]>>4)&1)<<shift;
			pgfx[ 4] |= ((tmp[ 0]>>3)&1)<<shift;
			pgfx[ 5] |= ((tmp[ 0]>>2)&1)<<shift;
			pgfx[ 6] |= ((tmp[ 0]>>1)&1)<<shift;
			pgfx[ 7] |= ((tmp[ 0]>>0)&1)<<shift;

			pgfx[ 8] |= ((tmp[16]>>7)&1)<<shift;
			pgfx[ 9] |= ((tmp[16]>>6)&1)<<shift;
			pgfx[10] |= ((tmp[16]>>5)&1)<<shift;
			pgfx[11] |= ((tmp[16]>>4)&1)<<shift;
			pgfx[12] |= ((tmp[16]>>3)&1)<<shift;
			pgfx[13] |= ((tmp[16]>>2)&1)<<shift;
			pgfx[14] |= ((tmp[16]>>1)&1)<<shift;
			pgfx[15] |= ((tmp[16]>>0)&1)<<shift;

			tmp ++;
			pgfx += 16;
		}
		tmp += 16;
	}
	return 0;
}

static int MemIndex()
{
	unsigned char *Next; Next = Mem;
	RomV33 		= Next; Next += 0x0C0000;			// V33
	RomV30		= Next; Next += 0x020000 * 2;		// V30
	RomGfx01	= Next; Next += 0x200000;			// char
	RomGfx02	= Next; Next += 0x800000;			// spr
	//MSM6295ROM	= Next; Next += 0x080000;

	RamStart	= Next;
	RamVideo	= Next; Next += 0x010000;
	RamV33		= Next; Next += 0x010000;
	RamV30		= Next; Next += 0x004000;
	RamSpr		= Next; Next += 0x000800;
	RamSprCpy	= Next; Next += 0x000800;
	RamPal		= Next; Next += 0x001000;			// 2 bank of 0x0800

	chip 		= (struct IremGA20_chip *) Next;
						Next += sizeof( struct IremGA20_chip );

	RamEnd		= Next;

	RamCurPal	= (unsigned short *) Next; Next += 0x001000;

	MemEnd		= Next;
	return 0;
}

static int hookInit()
{
	int nRet;

	Mem = NULL;
	MemIndex();
	int nLen = MemEnd - (unsigned char *)0;
	if ((Mem = (unsigned char *)malloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);										// blank all memory
	MemIndex();

	nRet = BurnLoadRom(RomV33 + 0x000001, 0, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(RomV33 + 0x000000, 1, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(RomV33 + 0x080001, 2, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(RomV33 + 0x080000, 3, 2); if (nRet != 0) return 1;

	unsigned char *tmp = (unsigned char *) malloc (0x100000);
	if ( tmp == 0 )
	{
		return 1;
	}

	nRet = BurnLoadRom(tmp + 0x000001, 4, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(tmp + 0x000000, 5, 2); if (nRet != 0) return 1;

	//irem_cpu_decrypt(RomV30, 0x20000, hook_decryption_table );
	irem_cpu_decrypt(0,hook_decryption_table,tmp,RomV30,0x20000 );


	free(tmp);
	// load and decode tile
	tmp = (unsigned char *) malloc (0x100000);
	if ( tmp == 0 )
	{
		return 1;
	}
	loadDecodeGfx01(tmp,  6, 0, 0x040000);
	loadDecodeGfx01(tmp,  7, 1, 0x040000);
	loadDecodeGfx01(tmp,  8, 2, 0x040000);
	loadDecodeGfx01(tmp,  9, 3, 0x040000);

	loadDecodeGfx02(tmp, 10, 0, 0x100000);
	loadDecodeGfx02(tmp, 11, 1, 0x100000);
	loadDecodeGfx02(tmp, 12, 2, 0x100000);
	loadDecodeGfx02(tmp, 13, 3, 0x100000);

	free(tmp);

	{
		unsigned int cpu_types[] = { 0, 8 };
		VezInit(2, &cpu_types[0]);

	    VezOpen(0);

		VezMapArea(0x00000, 0x9ffff, 0, RomV33 + 0x00000);	// CPU 0 ROM
		VezMapArea(0x00000, 0x9ffff, 2, RomV33 + 0x00000);	// CPU 0 ROM

		VezMapArea(0xa0000, 0xbffff, 0, RomV33 + 0xa0000);	// rom bank
		VezMapArea(0xa0000, 0xbffff, 2, RomV33 + 0xa0000);	// rom bank

		VezMapArea(0xd0000, 0xdffff, 0, RamVideo);
		VezMapArea(0xd0000, 0xdffff, 1, RamVideo);

		VezMapArea(0xe0000, 0xeffff, 0, RamV33);			// system ram
		VezMapArea(0xe0000, 0xeffff, 1, RamV33);

		VezMapArea(0xf8000, 0xf87ff, 0, RamSpr);			// sprites ram
		VezMapArea(0xf8000, 0xf87ff, 1, RamSpr);

		VezSetReadHandler(m92ReadByte);
		VezSetWriteHandler(m92WriteByte);
		VezSetReadPort(m92ReadPort);
		VezSetWritePort(m92WritePort);

		VezOpen(1);

		VezMapArea(0x00000, 0x1ffff, 0, RomV30 + 0x00000);	// CPU 1 ROM
		VezMapArea(0x00000, 0x1ffff, 2, RomV30 + 0x20000, RomV30 + 0x00000);

		VezMapArea(0xa0000, 0xa3fff, 0, RamV30);			// system ram
		VezMapArea(0xa0000, 0xa3fff, 1, RamV30);

		// V30 Startup vector
		VezMapArea(0xff800, 0xfffff, 0, RomV30 + 0x1f800);
		VezMapArea(0xff800, 0xfffff, 2, RomV30 + 0x3f800, RomV30 + 0x1f800);

		VezSetReadHandler(m92SndReadByte);
		VezSetWriteHandler(m92SndWriteByte);

	}

	m92_irq_vectorbase = 0x80;
	PalBank	= 0;

	BurnYM2151Init(3579545, 80.0);		// 3.5795 MHz
	YM2151SetIrqHandler(0, &m92YM2151IRQHandler);

	DrvDoReset();
	return 0;
}

static int hookExit()
{
	BurnYM2151Exit();

	VezExit();

	free(Mem);
	Mem = NULL;

	return 0;
}

#define	TILE_LAYER_1(xx)		\
	if (d[xx]) p[xx] = pal[ d[xx] | c ];

#define	TILE_LAYER_1_E(xx)		\
	if (d[xx] && (x + xx)>=0 && (x + xx)<320 ) p[xx] = pal[ d[xx] | c ];

#define	TILE_LAYER_1_LINE		\
	TILE_LAYER_1(0)				\
	TILE_LAYER_1(1)				\
	TILE_LAYER_1(2)				\
	TILE_LAYER_1(3)				\
	TILE_LAYER_1(4)				\
	TILE_LAYER_1(5)				\
	TILE_LAYER_1(6)				\
	TILE_LAYER_1(7)

#define	TILE_LAYER_1_LINE_E		\
	if ( (y+k)>=0 && (y+k)<240 ) {	\
		TILE_LAYER_1_E(0)		\
		TILE_LAYER_1_E(1)		\
		TILE_LAYER_1_E(2)		\
		TILE_LAYER_1_E(3)		\
		TILE_LAYER_1_E(4)		\
		TILE_LAYER_1_E(5)		\
		TILE_LAYER_1_E(6)		\
		TILE_LAYER_1_E(7)		\
	}

static void tileLayer_1()
{
	/* T--- -YX-  Pccc cccc  tttt tttt tttt tttt */
	unsigned short * pal = RamCurPal;
	int offs, mx, my, x, y;

	mx = -1;
	my = 0;
	for (offs = 0; offs < 64*64*4; offs+=4) {
		mx++;
		if (mx == 64) {
			mx = 0;
			my++;
		}

		x = mx * 8 - ((80 + (short)((pf1_control[5]<<8) | pf1_control[4])) & 0x1ff);
		if (x < -16) x += 512;

		y = my * 8 - ((136 + (short)((pf1_control[1]<<8) | pf1_control[0])) & 0x1ff);
		if (y < -16) y += 512;

		if ( x<=-8 || x>=320 || y<=-8 || y>= 240 )
			continue;

		int tile_index = offs + pf1_vram_ptr;
		unsigned int tileno = RamVideo[tile_index] | (RamVideo[tile_index+1] << 8) | ((RamVideo[tile_index+3] & 0x80) << 9);
		if (tileno == 0) continue;

		unsigned int c = (RamVideo[tile_index+2] & 0x7F) << 4;
		unsigned short * p = (unsigned short *) pBurnDraw + y * 320 + x;
		unsigned char * d = RomGfx01 + (tileno * 64);

		if ( x >=0 && x <= (320-8) && y >= 0 && y <= (240-8)) {

			//unsigned char * pp = RamPri + y * 320 + x;

			for (int k=0;k<8;k++) {

 				TILE_LAYER_1_LINE

 				d += 8;
 				p += 320;
 			}

		} else {

			for (int k=0;k<8;k++) {

 				TILE_LAYER_1_LINE_E

 				d += 8;
 				p += 320;
 			}
		}
	}
}


static void tileLayer_2()
{
	/* T--- -YX-  Pccc cccc  tttt tttt tttt tttt */
	unsigned short * pal = RamCurPal;
	int offs, mx, my, x, y;

	mx = -1;
	my = 0;
	for (offs = 0; offs < 64*64*4; offs+=4) {
		mx++;
		if (mx == 64) {
			mx = 0;
			my++;
		}

		x = mx * 8 - ((80 - 2 + (short)((pf2_control[5]<<8) | pf2_control[4])) & 0x1ff);
		if (x < -16) x += 512;

		y = my * 8 - ((136 + (short)((pf2_control[1]<<8) | pf2_control[0])) & 0x1ff);
		if (y < -16) y += 512;

		if ( x<=-8 || x>=320 || y<=-8 || y>= 240 )
			continue;

		int tile_index = offs + pf2_vram_ptr;
		unsigned int tileno = RamVideo[tile_index] | (RamVideo[tile_index+1] << 8) | ((RamVideo[tile_index+3] & 0x80) << 9);
		if (tileno == 0) continue;

		//if ((RamVideo[tile_index+2] & 0x80) ^ pri) continue;

		unsigned int c = (RamVideo[tile_index+2] & 0x7F) << 4;
		unsigned short * p = (unsigned short *) pBurnDraw + y * 320 + x;
		unsigned char * d = RomGfx01 + (tileno * 64);
		//unsigned char * pp = RamPri + y * 320 + x;

		if ( x >=0 && x <= (320-8) && y >= 0 && y <= (240-8)) {

			for (int k=0;k<8;k++) {

 				TILE_LAYER_1_LINE

 				d += 8;
 				p += 320;
 			}

		} else {

			for (int k=0;k<8;k++) {

 				TILE_LAYER_1_LINE_E

 				d += 8;
 				p += 320;
 			}
		}
	}
}

#define	TILE_LAYER_3(xx)		\
	p[xx] = pal[ d[xx] | c ];

#define	TILE_LAYER_3_E(xx)		\
	if ((x + xx)>=0 && (x + xx)<320 ) p[xx] = pal[ d[xx] | c ];

#define	TILE_LAYER_3_LINE		\
	TILE_LAYER_3(0)				\
	TILE_LAYER_3(1)				\
	TILE_LAYER_3(2)				\
	TILE_LAYER_3(3)				\
	TILE_LAYER_3(4)				\
	TILE_LAYER_3(5)				\
	TILE_LAYER_3(6)				\
	TILE_LAYER_3(7)

#define	TILE_LAYER_3_LINE_E		\
	if ( (y+k)>=0 && (y+k)<240 ) {	\
		TILE_LAYER_3_E(0)		\
		TILE_LAYER_3_E(1)		\
		TILE_LAYER_3_E(2)		\
		TILE_LAYER_3_E(3)		\
		TILE_LAYER_3_E(4)		\
		TILE_LAYER_3_E(5)		\
		TILE_LAYER_3_E(6)		\
		TILE_LAYER_3_E(7)		\
	}

static void tileLayer_3()
{
	/* T--- -YX-  Pccc cccc  tttt tttt tttt tttt */
	unsigned short * pal = RamCurPal;
	int offs, mx, my, x, y;

	mx = -1;
	my = 0;
	for (offs = 0; offs < 64*64*4; offs+=4) {
		mx++;
		if (mx == 64) {
			mx = 0;
			my++;
		}

		x = mx * 8 - ((80 - 4 + (short)((pf3_control[5]<<8) | pf3_control[4])) & 0x1ff);
		if (x < -16) x += 512;

		y = my * 8 - ((136 + (short)((pf3_control[1]<<8) | pf3_control[0])) & 0x1ff);
		if (y < -16) y += 512;

		if ( x<=-8 || x>=320 || y<=-8 || y>= 240 )
			continue;

		int tile_index = offs + pf3_vram_ptr;
		unsigned int tileno = RamVideo[tile_index] | (RamVideo[tile_index+1] << 8) | ((RamVideo[tile_index+3] & 0x80) << 9);
		//if (tileno == 0) continue;

		unsigned int c = (RamVideo[tile_index+2] & 0x7F) << 4;
		unsigned short * p = (unsigned short *) pBurnDraw + y * 320 + x;
		unsigned char * d = RomGfx01 + (tileno * 64);
		//unsigned char * pp = RamPri + y * 320 + x;

		if ( x >=0 && x <= (320-8) && y >= 0 && y <= (240-8)) {

			for (int k=0;k<8;k++) {

 				TILE_LAYER_3_LINE

 				d += 8;
 				p += 320;
 			}

		} else {

			for (int k=0;k<8;k++) {

 				TILE_LAYER_3_LINE_E

 				d += 8;
 				p += 320;
 			}
		}
	}
}

#define TILE_SPR_NORMAL(x)				\
	if (q[x]) p[x]=pal[q[x]|color];

#define TILE_SPR_FLIP_X(x)				\
	if (q[x]) p[15-x]=pal[q[x]|color];

#define TILE_SPR_NORMAL_E(x)			\
	if (q[x]&&((sx+x)>=0)&&((sx+x)<320)) p[x]=pal[q[x]|color];

#define TILE_SPR_FLIP_X_E(x)			\
	if (q[x]&&((sx+15-x)>=0)&&((sx+15-x)<320)) p[15-x]=pal[q[x]|color];

#define TILE_SPR_NORMAL_LINE			\
	TILE_SPR_NORMAL( 0)					\
	TILE_SPR_NORMAL( 1)					\
	TILE_SPR_NORMAL( 2)					\
	TILE_SPR_NORMAL( 3)					\
	TILE_SPR_NORMAL( 4)					\
	TILE_SPR_NORMAL( 5)					\
	TILE_SPR_NORMAL( 6)					\
	TILE_SPR_NORMAL( 7)					\
	TILE_SPR_NORMAL( 8)					\
	TILE_SPR_NORMAL( 9)					\
	TILE_SPR_NORMAL(10)					\
	TILE_SPR_NORMAL(11)					\
	TILE_SPR_NORMAL(12)					\
	TILE_SPR_NORMAL(13)					\
	TILE_SPR_NORMAL(14)					\
	TILE_SPR_NORMAL(15)

#define TILE_SPR_FLIP_X_LINE			\
	TILE_SPR_FLIP_X( 0)					\
	TILE_SPR_FLIP_X( 1)					\
	TILE_SPR_FLIP_X( 2)					\
	TILE_SPR_FLIP_X( 3)					\
	TILE_SPR_FLIP_X( 4)					\
	TILE_SPR_FLIP_X( 5)					\
	TILE_SPR_FLIP_X( 6)					\
	TILE_SPR_FLIP_X( 7)					\
	TILE_SPR_FLIP_X( 8)					\
	TILE_SPR_FLIP_X( 9)					\
	TILE_SPR_FLIP_X(10)					\
	TILE_SPR_FLIP_X(11)					\
	TILE_SPR_FLIP_X(12)					\
	TILE_SPR_FLIP_X(13)					\
	TILE_SPR_FLIP_X(14)					\
	TILE_SPR_FLIP_X(15)

#define TILE_SPR_NORMAL_LINE_E			\
	TILE_SPR_NORMAL_E( 0)				\
	TILE_SPR_NORMAL_E( 1)				\
	TILE_SPR_NORMAL_E( 2)				\
	TILE_SPR_NORMAL_E( 3)				\
	TILE_SPR_NORMAL_E( 4)				\
	TILE_SPR_NORMAL_E( 5)				\
	TILE_SPR_NORMAL_E( 6)				\
	TILE_SPR_NORMAL_E( 7)				\
	TILE_SPR_NORMAL_E( 8)				\
	TILE_SPR_NORMAL_E( 9)				\
	TILE_SPR_NORMAL_E(10)				\
	TILE_SPR_NORMAL_E(11)				\
	TILE_SPR_NORMAL_E(12)				\
	TILE_SPR_NORMAL_E(13)				\
	TILE_SPR_NORMAL_E(14)				\
	TILE_SPR_NORMAL_E(15)

#define TILE_SPR_FLIP_X_LINE_E			\
	TILE_SPR_FLIP_X_E( 0)				\
	TILE_SPR_FLIP_X_E( 1)				\
	TILE_SPR_FLIP_X_E( 2)				\
	TILE_SPR_FLIP_X_E( 3)				\
	TILE_SPR_FLIP_X_E( 4)				\
	TILE_SPR_FLIP_X_E( 5)				\
	TILE_SPR_FLIP_X_E( 6)				\
	TILE_SPR_FLIP_X_E( 7)				\
	TILE_SPR_FLIP_X_E( 8)				\
	TILE_SPR_FLIP_X_E( 9)				\
	TILE_SPR_FLIP_X_E(10)				\
	TILE_SPR_FLIP_X_E(11)				\
	TILE_SPR_FLIP_X_E(12)				\
	TILE_SPR_FLIP_X_E(13)				\
	TILE_SPR_FLIP_X_E(14)				\
	TILE_SPR_FLIP_X_E(15)

static void pdrawgfx(unsigned int code,unsigned int color,int flipx,int flipy,int sx,int sy,int /*pri*/)
{
	unsigned short * p	= (unsigned short *) pBurnDraw;
//	unsigned char * pp = RamPri;
	unsigned char * q	= RomGfx02 + (code << 8);
	unsigned short *pal	= RamCurPal;

	sx -= 80;
	sy -= 136;

	p += sy * 320 + sx;
//	pp += sy * 320 + sx;

	if (sx < 0 || sx >= (320-16) || sy < 0 || sy >= (240-16) ) {

		if ((sx <= -16) || (sx >= 320) || (sy <= -16) || (sy >= 240))
			return;

		if (flipy) {

			p += 320 * 15;
			//pp += 320 * 15;

			if (flipx) {

				for (int i=15;i>=0;i--) {
					if (((sy+i)>=0) && ((sy+i)<240)) {

						TILE_SPR_FLIP_X_LINE_E

					}
					p -= 320;
					//pp -= 320;
					q += 16;
				}

			} else {

				for (int i=15;i>=0;i--) {
					if (((sy+i)>=0) && ((sy+i)<240)) {

						TILE_SPR_NORMAL_LINE_E

					}
					p -= 320;
					//pp -= 320;
					q += 16;
				}
			}

		} else {

			if (flipx) {

				for (int i=0;i<16;i++) {
					if (((sy+i)>=0) && ((sy+i)<240)) {

						TILE_SPR_FLIP_X_LINE_E

					}
					p += 320;
					//pp += 320;
					q += 16;
				}

			} else {

				for (int i=0;i<16;i++) {
					if (((sy+i)>=0) && ((sy+i)<240)) {

						TILE_SPR_NORMAL_LINE_E

					}
					p += 320;
					//pp += 320;
					q += 16;
				}

			}

		}

		return;
	}

	if (flipy) {

		p += 320 * 15;
		//pp += 320 * 15;

		if (flipx) {

			for (int i=0;i<16;i++) {

				TILE_SPR_FLIP_X_LINE

				p -= 320;
				//pp -= 320;
				q += 16;
			}

		} else {

			for (int i=0;i<16;i++) {

				TILE_SPR_NORMAL_LINE

				p -= 320;
				//pp -= 320;
				q += 16;
			}
		}

	} else {

		if (flipx) {

			for (int i=0;i<16;i++) {

				TILE_SPR_FLIP_X_LINE

				p += 320;
				//pp += 320;
				q += 16;
			}

		} else {

			for (int i=0;i<16;i++) {

				TILE_SPR_NORMAL_LINE

				p += 320;
				//pp += 320;
				q += 16;
			}

		}

	}

}

static void drawSprites()
{
	int offs = m92_sprite_list - 8;
	while (offs >= 0) {
		int x,y,sprite,colour,fx,fy,x_multi,y_multi,i,j,s_ptr,pri;

		y=(RamSprCpy[offs+0] | (RamSprCpy[offs+1]<<8))&0x1ff;
		x=(RamSprCpy[offs+6] | (RamSprCpy[offs+7]<<8))&0x1ff;

		if ((RamSprCpy[offs+4]&0x80)==0x80) pri=0; else pri=2;

		x = x - 16;
		y = 512 - 16 - y;

		sprite=(RamSprCpy[offs+2] | (RamSprCpy[offs+3]<<8));
		colour= (RamSprCpy[offs+4]&0x7f) << 4;

		fx=RamSprCpy[offs+5]&1;
		fy=(RamSprCpy[offs+5]&2)>>1;
		y_multi=(RamSprCpy[offs+1]>>1)&0x3;
		x_multi=(RamSprCpy[offs+1]>>3)&0x3;

		y_multi=1 << y_multi; /* 1, 2, 4 or 8 */
		x_multi=1 << x_multi; /* 1, 2, 4 or 8 */

		if (fx) x+=16 * (x_multi - 1);

		for (j=0; j<x_multi; j++) {
			s_ptr=8 * j;
			if (!fy) s_ptr+=y_multi-1;

			for (i=0; i<y_multi; i++) {

				pdrawgfx(sprite + s_ptr, colour, fx, fy, x, y-i*16 ,pri);

				if (fy) s_ptr++; else s_ptr--;
			}
			if (fx) x-=16; else x+=16;

			offs-=8;
		}
	}
}

static void DrvDraw()
{
#ifdef FBA_DEBUG
	if (nBurnLayer & 8) {
#endif

	if (pf3_enable)
		tileLayer_3();
	else
		memset(pBurnDraw, 0, 320 * 240 * 2);

#ifdef FBA_DEBUG
	} else
		memset(pBurnDraw, 0, 320 * 240 * 2);
#endif


#ifdef FBA_DEBUG
	if (nBurnLayer & 4)
#endif
	if (pf2_enable)
		tileLayer_2();

	drawSprites();

#ifdef FBA_DEBUG
	if (nBurnLayer & 2)
#endif
	if (pf1_enable)
		tileLayer_1();

#if 0
	bprintf(PRINT_NORMAL, _T("E:%d%d%d S:%d%d%d P:%d%d%d %04x %04x %04x R: %04x %04x, %04x %04x, %04x %04x\n"),
		pf1_enable, pf2_enable, pf3_enable,
		pf1_rowscroll, pf2_rowscroll, pf3_rowscroll,
		pf1_shape, pf2_shape, pf3_shape,
		pf1_vram_ptr, pf2_vram_ptr, pf3_vram_ptr,
		(pf1_control[5]<<8)+pf1_control[4], (pf1_control[1]<<8) + pf1_control[0],
		(pf2_control[5]<<8)+pf2_control[4], (pf2_control[1]<<8) + pf2_control[0],
		(pf3_control[5]<<8)+pf3_control[4], (pf3_control[1]<<8) + pf3_control[0]  );

#endif
}

static int hookFrame()
{
	if (DrvReset) DrvDoReset();

	if (bRecalcPalette) {
		for (int i=0; i<0x800;i++)
			RamCurPal[i] = CalcCol(i<<1);
		bRecalcPalette = 0;
	}

	DrvInput[0] = 0x00;
	DrvInput[1] = 0x00;
	DrvInput[2] = 0x00;
	DrvInput[3] = 0x00;
	DrvInput[4] = 0x00;

	for (int i=0; i<8; i++) {
		DrvInput[0] |= (DrvJoy1[i] & 1) << i;
		DrvInput[1] |= (DrvJoy2[i] & 1) << i;
		DrvInput[2] |= (DrvJoy3[i] & 1) << i;
		DrvInput[3] |= (DrvJoy4[i] & 1) << i;
		DrvInput[4] |= (DrvButton[i] & 1) << i;
	}

#if 0
	VezOpen(0);

	VezRun(9000000 / 60);

	if ( m92_sprite_buffer_busy == 0 ) {
		m92_sprite_buffer_busy = 0x80;

		memcpy(RamSprCpy, RamSpr, 0x800);

		VezSetIRQLine((m92_irq_vectorbase + 4), VEZ_IRQSTATUS_ACK);

		VezRun( m92_sprite_buffer_timer );
	}

	VezSetIRQLine((m92_irq_vectorbase + 0), VEZ_IRQSTATUS_ACK);

	VezOpen(1);

	VezRun(7159090 / 60);

	if (pBurnDraw) DrvDraw();

	if (pBurnSoundOut) {
		BurnYM2151Render(pBurnSoundOut, nBurnSoundLen);
		IremGA20_update(pBurnSoundOut, nBurnSoundLen);
	}

#else

	for (int i=0; i<32; i++ ) {
		VezOpen(0);
		VezRun(9000000 / 60 / 32);
		if ( m92_sprite_buffer_busy == 0 ) {
			m92_sprite_buffer_busy = 0x80;
			memcpy(RamSprCpy, RamSpr, 0x800);
			VezSetIRQLine((m92_irq_vectorbase + 4), VEZ_IRQSTATUS_ACK);
			VezRun( m92_sprite_buffer_timer );
		}

		VezOpen(1);
		VezRun(7159090 / 60 / 32);
	}

	VezOpen(0);
	VezSetIRQLine((m92_irq_vectorbase + 0), VEZ_IRQSTATUS_ACK);

	VezOpen(1);

	if (pBurnDraw) DrvDraw();

	if (pBurnSoundOut) {
		BurnYM2151Render(pBurnSoundOut, nBurnSoundLen);
		IremGA20_update(pBurnSoundOut, nBurnSoundLen);
	}


#endif

	return 0;
}

static int DrvScan(int nAction,int *pnMin)
{
	if ( pnMin ) *pnMin =  0x029671;

	struct BurnArea ba;

	if (nAction & ACB_MEMORY_RAM) {								// Scan all memory, devices & variables
		memset(&ba, 0, sizeof(ba));
		ba.Data	  = RamStart;
		ba.nLen	  = RamEnd-RamStart;
		ba.szName = "All Ram";
		BurnAcb(&ba);

		if (nAction & ACB_WRITE)
			bRecalcPalette = 1;
	}

	if (nAction & ACB_DRIVER_DATA) {

		VezScan(nAction);										// Scan 68000 state

		SCAN_VAR(DrvInput);
		SCAN_VAR(m92_irq_vectorbase);
		SCAN_VAR(PalBank);

		SCAN_VAR(pf1_control);
		SCAN_VAR(pf2_control);
		SCAN_VAR(pf3_control);
		SCAN_VAR(pf4_control);

		SCAN_VAR(pf1_enable);
		SCAN_VAR(pf2_enable);
		SCAN_VAR(pf3_enable);

		SCAN_VAR(pf1_shape);
		SCAN_VAR(pf2_shape);
		SCAN_VAR(pf3_shape);

		SCAN_VAR(pf1_rowscroll);
		SCAN_VAR(pf2_rowscroll);
		SCAN_VAR(pf3_rowscroll);

		SCAN_VAR(pf1_vram_ptr);
		SCAN_VAR(pf2_vram_ptr);
		SCAN_VAR(pf3_vram_ptr);

		SCAN_VAR(m92_sprite_list);

		//SCAN_VAR(sound_status);

		//SCAN_VAR(m92_sprite_buffer_busy);
		//SCAN_VAR(m92_sprite_buffer_timer);
	}

	return 0;
}


struct BurnDriverD BurnDrvHook = {
	"hook", NULL, NULL, "1992",
	"Hook (World)\0", "Preliminary driver", "Irem", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY, 4, HARDWARE_MISC_MISC, 
	NULL, hookRomInfo, hookRomName, hookInputInfo, hookDIPInfo,
	hookInit, hookExit, hookFrame, NULL, DrvScan, &bRecalcPalette,
	320, 240, 4, 3
};

struct BurnDriverD BurnDrvHooku = {
	"hooku", "hook", NULL, "1992",
	"Hook (US)\0", "Preliminary driver", "Irem America", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_16BIT_ONLY, 4, HARDWARE_MISC_MISC,
	NULL, hookuRomInfo, hookuRomName, hookInputInfo, hookDIPInfo,
	hookInit, hookExit, hookFrame, NULL, DrvScan, &bRecalcPalette, 
	320, 240, 4, 3
};

struct BurnDriverD BurnDrvHookj = {
	"hookj", "hook", NULL, "1992",
	"Hook (Japan)\0", "Preliminary driver", "Irem", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_16BIT_ONLY, 4, HARDWARE_MISC_MISC,
	NULL, hookjRomInfo, hookjRomName, hookInputInfo, hookDIPInfo,
	hookInit, hookExit, hookFrame, NULL, DrvScan,  &bRecalcPalette,
	320, 240, 4, 3
};

static int MemIndex2()
{
	unsigned char *Next; Next = Mem;
	RomV33 		= Next; Next += 0x0C0000;			// V33
	RomV30		= Next; Next += 0x100000;			// V30
	RomGfx01	= Next; Next += 0x400000;			// char
	RomGfx02	= Next; Next += 0x800000;			// spr
	//MSM6295ROM	= Next; Next += 0x080000;

	RamStart	= Next;
	RamVideo	= Next; Next += 0x010000;
	RamV33		= Next; Next += 0x010000;
	RamSpr		= Next; Next += 0x000800;
	RamSprCpy	= Next; Next += 0x000800;
	RamPal		= Next; Next += 0x001000;			// 2 bank of 0x0800

	chip 		= (struct IremGA20_chip *) Next;
						Next += sizeof( struct IremGA20_chip );

	RamEnd		= Next;

	RamCurPal	= (unsigned short *) Next; Next += 0x001000;

	MemEnd		= Next;
	return 0;
}

static int inthuntInit()
{
	int nRet;

	Mem = NULL;
	MemIndex2();
	int nLen = MemEnd - (unsigned char *)0;
	if ((Mem = (unsigned char *)malloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);										// blank all memory
	MemIndex2();

	nRet = BurnLoadRom(RomV33 + 0x000001, 0, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(RomV33 + 0x000000, 1, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(RomV33 + 0x080001, 2, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(RomV33 + 0x080000, 3, 2); if (nRet != 0) return 1;

	nRet = BurnLoadRom(RomV30 + 0x000001, 4, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(RomV30 + 0x000000, 5, 2); if (nRet != 0) return 1;

	// load and decode tile
	unsigned char *tmp = (unsigned char *) malloc (0x100000);
	if ( tmp == 0 ) return 1;

	loadDecodeGfx01(tmp,  6, 0, 0x080000);
	loadDecodeGfx01(tmp,  7, 1, 0x080000);
	loadDecodeGfx01(tmp,  8, 2, 0x080000);
	loadDecodeGfx01(tmp,  9, 3, 0x080000);

	loadDecodeGfx02(tmp, 10, 0, 0x100000);
	loadDecodeGfx02(tmp, 11, 1, 0x100000);
	loadDecodeGfx02(tmp, 12, 2, 0x100000);
	loadDecodeGfx02(tmp, 13, 3, 0x100000);

	free(tmp);

	{
		unsigned int cpu_types[] = { 0, 8 };
		VezInit(2, &cpu_types[0]);

	    VezOpen(0);

		VezMapArea(0x00000, 0x9ffff, 0, RomV33 + 0x00000);	// CPU 0 ROM
		VezMapArea(0x00000, 0x9ffff, 2, RomV33 + 0x00000);

		VezMapArea(0xa0000, 0xbffff, 0, RomV33 + 0xa0000);	// rom bank
		VezMapArea(0xa0000, 0xbffff, 2, RomV33 + 0xa0000);

		VezMapArea(0xc0000, 0xcffff, 0, RomV33 + 0x00000);	// Mirror, Used by In The Hunt as protection
		VezMapArea(0xc0000, 0xcffff, 2, RomV33 + 0x00000);

		VezMapArea(0xd0000, 0xdffff, 0, RamVideo);
		VezMapArea(0xd0000, 0xdffff, 1, RamVideo);

		VezMapArea(0xe0000, 0xeffff, 0, RamV33);			// system ram
		VezMapArea(0xe0000, 0xeffff, 1, RamV33);

		VezMapArea(0xf8000, 0xf87ff, 0, RamSpr);			// sprites ram
		VezMapArea(0xf8000, 0xf87ff, 1, RamSpr);

		VezSetReadHandler(m92ReadByte);
		VezSetWriteHandler(m92WriteByte);
		VezSetReadPort(m92ReadPort);
		VezSetWritePort(m92WritePort);
	}

	m92_irq_vectorbase = 0x80;
	PalBank	= 0;

	BurnYM2151Init(3579545, 80.0);		// 3.5795 MHz
	YM2151SetIrqHandler(0, &m92YM2151IRQHandler);

	DrvDoReset();
	return 0;
}

static int inthuntFrame()
{
	if (DrvReset) DrvDoReset();

	if (bRecalcPalette) {
		for (int i=0; i<0x800;i++)
			RamCurPal[i] = CalcCol(i<<1);
		bRecalcPalette = 0;
	}

	DrvInput[0] = 0x00;
	DrvInput[1] = 0x00;
	DrvInput[2] = 0x00;
	DrvInput[3] = 0x00;
	DrvInput[4] = 0x00;

	for (int i = 0; i < 8; i++) {
		DrvInput[0] |= (DrvJoy1[i] & 1) << i;
		DrvInput[1] |= (DrvJoy2[i] & 1) << i;
		DrvInput[2] |= (DrvJoy3[i] & 1) << i;
		DrvInput[3] |= (DrvJoy4[i] & 1) << i;
		DrvInput[4] |= (DrvButton[i] & 1) << i;
	}

	VezOpen(0);

//
	for (int i=255; i>=0; i--) {

		VezRun(9000000 / 60 / 256);

		if ( m92_sprite_buffer_busy == 0 ) {
			m92_sprite_buffer_busy = 0x80;
			memcpy(RamSprCpy, RamSpr, 0x800);
			VezSetIRQLine(m92_irq_vectorbase + 4, VEZ_IRQSTATUS_ACK);
			VezRun( m92_sprite_buffer_timer );
		}

		if (i == m92_raster_irq_position)
			VezSetIRQLine(m92_irq_vectorbase + 8, VEZ_IRQSTATUS_ACK); // IRQ 2
		else
		if (i == 249) {
			VezSetIRQLine(m92_irq_vectorbase + 0, VEZ_IRQSTATUS_ACK); // IRQ 0

		}
	}

	if (pBurnDraw) DrvDraw();

	return 0;
}

struct BurnDriverD BurnDrvInthunt = {
	"inthunt", NULL, NULL, "1993",
	"In The Hunt (World)\0", "Preliminary driver", "Irem", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY, 2, HARDWARE_MISC_MISC, 
	NULL, inthuntRomInfo, inthuntRomName, inthuntInputInfo, inthuntDIPInfo,
	inthuntInit, hookExit, inthuntFrame, NULL, DrvScan, &bRecalcPalette,
	320, 240, 4, 3
};

struct BurnDriverD BurnDrvInthuntu = {
	"inthuntu", "inthunt", NULL, "1993",
	"In The Hunt (US)\0", "Preliminary driver", "Irem America", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_16BIT_ONLY, 2, HARDWARE_MISC_MISC,
	NULL, inthuntuRomInfo, inthuntuRomName, inthuntInputInfo, inthuntDIPInfo,
	inthuntInit, hookExit, inthuntFrame, NULL, DrvScan, &bRecalcPalette, 
	320, 240, 4, 3
};

struct BurnDriverD BurnDrvKaiteids = {
	"kaiteids", "inthunt", NULL, "1993",
	"Kaitei Daisensou (Japan)\0", "Preliminary driver", "Irem", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_16BIT_ONLY, 2, HARDWARE_MISC_MISC, 
	NULL, kaiteidsRomInfo, kaiteidsRomName, inthuntInputInfo, inthuntDIPInfo,
	inthuntInit, hookExit, inthuntFrame, NULL, DrvScan, &bRecalcPalette,
	320, 240, 4, 3
};


static int rtypeleoExit()
{
	VezExit();

	free(Mem);
	Mem = NULL;

	return 0;
}

static int rtypeleoInit()
{
	int nRet;

	Mem = NULL;
	MemIndex2();
	int nLen = MemEnd - (unsigned char *)0;
	if ((Mem = (unsigned char *)malloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);										// blank all memory
	MemIndex2();

	nRet = BurnLoadRom(RomV33 + 0x000001, 0, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(RomV33 + 0x000000, 1, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(RomV33 + 0x080001, 2, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(RomV33 + 0x080000, 3, 2); if (nRet != 0) return 1;

	nRet = BurnLoadRom(RomV30 + 0x000001, 4, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(RomV30 + 0x000000, 5, 2); if (nRet != 0) return 1;

	// load and decode tile
	unsigned char *tmp = (unsigned char *) malloc (0x100000);
	if ( tmp == 0 ) return 1;

	loadDecodeGfx01(tmp,  6, 0, 0x080000);
	loadDecodeGfx01(tmp,  7, 1, 0x080000);
	loadDecodeGfx01(tmp,  8, 2, 0x080000);
	loadDecodeGfx01(tmp,  9, 3, 0x080000);

	loadDecodeGfx02(tmp, 10, 0, 0x100000);
	loadDecodeGfx02(tmp, 11, 1, 0x100000);
	loadDecodeGfx02(tmp, 12, 2, 0x100000);
	loadDecodeGfx02(tmp, 13, 3, 0x100000);

	free(tmp);

	{
		unsigned int cpu_types[] = { 0, 8 };
		VezInit(2, &cpu_types[0]);

		VezOpen(0);

		VezMapArea(0x00000, 0x9ffff, 0, RomV33 + 0x00000);	// CPU 0 ROM
		VezMapArea(0x00000, 0x9ffff, 2, RomV33 + 0x00000);	// CPU 0 ROM

		VezMapArea(0xa0000, 0xbffff, 0, RomV33 + 0xa0000);	// rom bank
		VezMapArea(0xa0000, 0xbffff, 2, RomV33 + 0xa0000);	// rom bank

		VezMapArea(0xd0000, 0xdffff, 0, RamVideo);
		VezMapArea(0xd0000, 0xdffff, 1, RamVideo);

		VezMapArea(0xe0000, 0xeffff, 0, RamV33);			// system ram
		VezMapArea(0xe0000, 0xeffff, 1, RamV33);

		VezMapArea(0xf8000, 0xf87ff, 0, RamSpr);			// sprites ram
		VezMapArea(0xf8000, 0xf87ff, 1, RamSpr);

		VezSetReadHandler(m92ReadByte);
		VezSetWriteHandler(m92WriteByte);
		VezSetReadPort(m92ReadPort);
		VezSetWritePort(m92WritePort);

	}

	m92_irq_vectorbase = 0x20;
	PalBank	= 0;

	DrvDoReset();
	return 0;
}


static int rtypeleoFrame()
{

	if (DrvReset) DrvDoReset();

	if (bRecalcPalette) {
		for (int i=0; i<0x800;i++)
			RamCurPal[i] = CalcCol(i<<1);
		bRecalcPalette = 0;
	}

	DrvInput[0] = 0x00;
	DrvInput[1] = 0x00;
	DrvInput[2] = 0x00;
	DrvInput[3] = 0x00;
	DrvInput[4] = 0x00;

	for (int i=0; i<8; i++) {
		DrvInput[0] |= (DrvJoy1[i] & 1) << i;
		DrvInput[1] |= (DrvJoy2[i] & 1) << i;
		DrvInput[2] |= (DrvJoy3[i] & 1) << i;
		DrvInput[3] |= (DrvJoy4[i] & 1) << i;
		DrvInput[4] |= (DrvButton[i] & 1) << i;
	}

#if 0
	VezOpen(0);

	VezRun(9000000 / 60);

	if ( m92_sprite_buffer_busy == 0 ) {
		m92_sprite_buffer_busy = 0x80;

		memcpy(RamSprCpy, RamSpr, 0x800);

		VezSetIRQLine((m92_irq_vectorbase + 4), VEZ_IRQSTATUS_ACK);

		VezRun( m92_sprite_buffer_timer );
	}

	VezSetIRQLine((m92_irq_vectorbase + 0), VEZ_IRQSTATUS_ACK);

	VezOpen(1);

	VezRun(7159090 / 60);

	if (pBurnDraw) DrvDraw();

#else

	for (int i=0; i<32; i++ ) {
		VezOpen(0);
		VezRun(9000000 / 60 / 32);
		if ( m92_sprite_buffer_busy == 0 ) {
			m92_sprite_buffer_busy = 0x80;
			memcpy(RamSprCpy, RamSpr, 0x800);
			VezSetIRQLine((m92_irq_vectorbase + 4), VEZ_IRQSTATUS_ACK);
			VezRun( m92_sprite_buffer_timer );
		}

		VezOpen(1);
		VezRun(7159090 / 60 / 32);
	}

	VezOpen(0);
	VezSetIRQLine((m92_irq_vectorbase + 0), VEZ_IRQSTATUS_ACK);

	VezOpen(1);

	if (pBurnDraw) DrvDraw();

#endif

	return 0;
}


struct BurnDriverD BurnDrvRtypeleo = {
	"rtypeleo", NULL, NULL, "1992",
	"R-Type Leo (World)\0", "Preliminary driver", "Irem", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_16BIT_ONLY, 2, HARDWARE_MISC_MISC,
	NULL, rtypeleoRomInfo, rtypeleoRomName, inthuntInputInfo, inthuntDIPInfo,
	rtypeleoInit, rtypeleoExit, rtypeleoFrame, NULL, DrvScan, &bRecalcPalette,
	320, 240, 4, 3
};

struct BurnDriverD BurnDrvRtypelej = {
	"rtypeleoj", "rtypeleo", NULL, "1992",
	"R-Type Leo (Japan)\0", "Preliminary driver", "Irem", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_CLONE | BDF_16BIT_ONLY, 2, HARDWARE_MISC_MISC,
	NULL, rtypelejRomInfo, rtypelejRomName, inthuntInputInfo, inthuntDIPInfo,
	rtypeleoInit, rtypeleoExit, rtypeleoFrame, NULL, DrvScan,  &bRecalcPalette, 
	320, 240, 4, 3
};
