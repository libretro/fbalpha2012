// FB Alpha Ambush driver module
// Based on MAME driver by Zsolt Vasvari

#include "tiles_generic.h"
#include "driver.h"
extern "C" {
#include "ay8910.h"
}

static unsigned char *AllMem;
static unsigned char *MemEnd;
static unsigned char *AllRam;
static unsigned char *RamEnd;
static unsigned char *DrvZ80ROM;
static unsigned char *DrvGfxROM0;
static unsigned char *DrvGfxROM1;
static unsigned char *DrvColPROM;
static unsigned char *DrvVidRAM;
static unsigned char *DrvSprRAM;
static unsigned char *DrvZ80RAM;
static unsigned char *DrvColRAM;
static unsigned char *DrvScrRAM;
static short         *pAY8910Buffer[6];
static unsigned int *DrvPalette;
static unsigned int *Palette;

static unsigned char DrvRecalc;

static unsigned char DrvJoy1[8];
static unsigned char DrvJoy2[8];
static unsigned char DrvDips[1];
static unsigned char DrvInputs[2];
static unsigned char DrvReset;

static unsigned char *color_bank;
static unsigned char *flipscreen;

static struct BurnInputInfo AmbushInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy1 + 7,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy2 + 0,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy2 + 1,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy2 + 2,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy2 + 3,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 fire 2"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy1 + 6,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy1 + 4,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 6,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 7,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy1 + 3,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy1 + 2,	"p2 fire 2"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
};

STDINPUTINFO(Ambush)

static struct BurnDIPInfo AmbushDIPList[]=
{
	{0x11, 0xff, 0xff, 0xc0, NULL                     },

	{0   , 0xfe, 0   , 4   , "Lives"           	  },
	{0x11, 0x01, 0x03, 0x00, "3"     		  },
	{0x11, 0x01, 0x03, 0x01, "4"    		  },
	{0x11, 0x01, 0x03, 0x02, "5"     		  },
	{0x11, 0x01, 0x03, 0x03, "6"    		  },

	{0   , 0xfe, 0   , 8   , "Coinage"	          },
	{0x11, 0x01, 0x1c, 0x14, "3 Coins 1 Credit"	  },
	{0x11, 0x01, 0x1c, 0x10, "2 Coins 1 Credit"	  },
	{0x11, 0x01, 0x1c, 0x00, "1 Coins 1 Credit"	  },
	{0x11, 0x01, 0x1c, 0x04, "1 Coin 2 Credits"	  },
	{0x11, 0x01, 0x1c, 0x18, "2 Coins 5 Credits"	  },
	{0x11, 0x01, 0x1c, 0x08, "1 Coin 3 Credits"	  },
	{0x11, 0x01, 0x1c, 0x0c, "1 Coin 4 Credits"	  },
	{0x11, 0x01, 0x1c, 0x1c, "Free Play / Service"	  },

	{0   , 0xfe, 0   , 2   , "Difficulty"             },
	{0x11, 0x01, 0x20, 0x00, "Easy"       		  },
	{0x11, 0x01, 0x20, 0x20, "Hard"       		  },

	{0   , 0xfe, 0   , 2   , "Bonus Life"             },
	{0x11, 0x01, 0x40, 0x40, "80000"     		  },
	{0x11, 0x01, 0x40, 0x00, "120000"    		  },

	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x11, 0x01, 0x80, 0x80, "Upright"     		  },
	{0x11, 0x01, 0x80, 0x00, "Cocktail"    		  },
};

STDDIPINFO(Ambush)

unsigned char __fastcall ambush_read_byte(unsigned short address)
{
	switch (address)
	{
		case 0xc800:
			return DrvDips[0];
	}

	return 0;
}

void __fastcall ambush_write_byte(unsigned short address, unsigned char data)
{
	switch (address)
	{
		case 0xcc04:
			*flipscreen = data;	
		break;

		case 0xcc05:
			*color_bank = (data & 3) << 4;
		break;
	}
}

unsigned char __fastcall ambush_in_port(unsigned short port)
{
	switch (port & 0xff)
	{
		case 0x00:
			return AY8910Read(0);

		case 0x80:
			return AY8910Read(1); 
	}

	return 0;
}

void __fastcall ambush_out_port(unsigned short port, unsigned char data)
{
	switch (port & 0xff)
	{
		case 0x00:
			AY8910Write(0, 0, data);
		break;

		case 0x01:
			AY8910Write(0, 1, data);
		break;

		case 0x80:
			AY8910Write(1, 0, data);
		break;

		case 0x81:
			AY8910Write(1, 1, data);
		break;
	}
}

unsigned char AY8910_0_port0(unsigned int)
{
	return DrvInputs[0];
}

unsigned char AY8910_1_port0(unsigned int)
{
	return DrvInputs[1];
}

static int DrvDoReset()
{
	memset(AllRam, 0, RamEnd - AllRam);

	ZetOpen(0);
	ZetReset();
	ZetClose();

	AY8910Reset(0);
	AY8910Reset(1);

	return 0;
}

static void DrvPaletteInit()
{
	for (int i = 0;i < 0x100; i++)
	{
		int bit0,bit1,bit2,r,g,b;

		bit0 = (DrvColPROM[i] >> 0) & 0x01;
		bit1 = (DrvColPROM[i] >> 1) & 0x01;
		bit2 = (DrvColPROM[i] >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		bit0 = (DrvColPROM[i] >> 3) & 0x01;
		bit1 = (DrvColPROM[i] >> 4) & 0x01;
		bit2 = (DrvColPROM[i] >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		bit1 = (DrvColPROM[i] >> 6) & 0x01;
		bit2 = (DrvColPROM[i] >> 7) & 0x01;
		b = 0x47 * bit1 + 0x97 * bit2;

		Palette[i] = (r << 16) | (g << 8) | b;
	}
}

static void DrvGfxDecode()
{
	static int Plane[2]  = { 0, 0x10000 };
	static int XOffs[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 64, 65, 66, 67, 68, 69, 70, 71 };
	static int YOffs[16] = { 0, 8, 16, 24, 32, 40, 48, 56, 128, 136, 144, 152, 160, 168, 176, 184 };

	unsigned char *tmp = (unsigned char *)malloc(0x4000);
	if (tmp == NULL) {
		return;
	}

	memcpy (tmp, DrvGfxROM0, 0x4000);

	GfxDecode(0x0400, 2,  8,  8, Plane, XOffs, YOffs, 0x040, tmp, DrvGfxROM0);

	GfxDecode(0x0100, 2, 16, 16, Plane, XOffs, YOffs, 0x100, tmp, DrvGfxROM1);

	free (tmp);
}

static int MemIndex()
{
	unsigned char *Next; Next = AllMem;

	DrvZ80ROM	= Next; Next += 0x008000;

	DrvGfxROM0	= Next; Next += 0x010000;
	DrvGfxROM1	= Next; Next += 0x010000;

	DrvColPROM	= Next; Next += 0x000100;

	DrvPalette	= (unsigned int*)Next; Next += 0x0100 * sizeof(int);
	Palette		= (unsigned int*)Next; Next += 0x0100 * sizeof(int);

	AllRam		= Next;

	DrvZ80RAM	= Next; Next += 0x000800;
	DrvVidRAM	= Next; Next += 0x000400;
	DrvSprRAM	= Next; Next += 0x000200;
	DrvColRAM	= Next; Next += 0x000100;
	DrvScrRAM	= Next; Next += 0x000100;

	flipscreen 	= Next; Next += 0x000001;
	color_bank	= Next; Next += 0x000001;

	pAY8910Buffer[0] = (short *)Next; Next += nBurnSoundLen * sizeof(short);
	pAY8910Buffer[1] = (short *)Next; Next += nBurnSoundLen * sizeof(short);
	pAY8910Buffer[2] = (short *)Next; Next += nBurnSoundLen * sizeof(short);
	pAY8910Buffer[3] = (short *)Next; Next += nBurnSoundLen * sizeof(short);
	pAY8910Buffer[4] = (short *)Next; Next += nBurnSoundLen * sizeof(short);
	pAY8910Buffer[5] = (short *)Next; Next += nBurnSoundLen * sizeof(short);

	RamEnd		= Next;

	MemEnd		= Next;

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
		if (BurnLoadRom(DrvZ80ROM  + 0x0000, 0, 1)) return 1;
		if (BurnLoadRom(DrvZ80ROM  + 0x2000, 1, 1)) return 1;
		if (BurnLoadRom(DrvZ80ROM  + 0x4000, 2, 1)) return 1;
		if (BurnLoadRom(DrvZ80ROM  + 0x6000, 3, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM0 + 0x0000, 4, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM0 + 0x2000, 5, 1)) return 1;

		if (BurnLoadRom(DrvColPROM + 0x0000, 6, 1)) return 1;

		DrvPaletteInit();
		DrvGfxDecode();
	}

	ZetInit(1);
	ZetOpen(0);
	ZetMapArea(0x0000, 0x7fff, 0, DrvZ80ROM);
	ZetMapArea(0x0000, 0x7fff, 2, DrvZ80ROM);
	ZetMapArea(0x8000, 0x87ff, 0, DrvZ80RAM);
	ZetMapArea(0x8000, 0x87ff, 1, DrvZ80RAM);
	ZetMapArea(0x8000, 0x87ff, 2, DrvZ80RAM);
	ZetMapArea(0xc000, 0xc0ff, 0, DrvScrRAM);
	ZetMapArea(0xc000, 0xc0ff, 1, DrvScrRAM);
	ZetMapArea(0xc000, 0xc0ff, 2, DrvScrRAM);
	ZetMapArea(0xc100, 0xc1ff, 0, DrvColRAM);
	ZetMapArea(0xc100, 0xc1ff, 1, DrvColRAM);
	ZetMapArea(0xc100, 0xc1ff, 2, DrvColRAM);
	ZetMapArea(0xc200, 0xc3ff, 0, DrvSprRAM);
	ZetMapArea(0xc200, 0xc3ff, 1, DrvSprRAM);
	ZetMapArea(0xc200, 0xc3ff, 2, DrvSprRAM);
	ZetMapArea(0xc400, 0xc7ff, 0, DrvVidRAM);
	ZetMapArea(0xc400, 0xc7ff, 1, DrvVidRAM);
	ZetMapArea(0xc400, 0xc7ff, 2, DrvVidRAM);
	ZetSetWriteHandler(ambush_write_byte);
	ZetSetReadHandler(ambush_read_byte);
	ZetSetOutHandler(ambush_out_port);
	ZetSetInHandler(ambush_in_port);
	ZetMemEnd();
	ZetClose();

	AY8910Init(0, 1500000, nBurnSoundRate, &AY8910_0_port0, NULL, NULL, NULL);
	AY8910Init(1, 1500000, nBurnSoundRate, &AY8910_1_port0, NULL, NULL, NULL);

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static int DrvExit()
{
	GenericTilesExit();

	ZetExit();
	AY8910Exit(0);
	AY8910Exit(1);

	free (AllMem);
	AllMem = NULL;

	return 0;
}

static void draw_layer(int priority)
{
	for (int offs = 0; offs < 32 * 32; offs++)
	{
		int sx = (offs & 0x1f) << 3;
		int sy = (offs >> 5) << 3;

		int attr = DrvColRAM[(sy & 0xe0) | (sx >> 3)];
		if ((attr & 0x10) != priority) continue;

		int code  = DrvVidRAM[offs] | ((attr & 0x60) << 3);
		int color = (attr & 0x0f) | *color_bank;

		sy = (sy + (DrvScrRAM[sx >> 3] ^ 0xff)) & 0xff;

		int flip = 0;

		if (*flipscreen) {
			flip = 1;
			sx = 248 - sx;
			sy = (248 - sy) - 16;
		}

		if (priority) {
			if (flip) {
				Render8x8Tile_Mask_FlipXY_Clip(pTransDraw, code, sx, sy, color, 2, 0, 0, DrvGfxROM0);
			} else {
				Render8x8Tile_Mask_Clip(pTransDraw, code, sx, sy, color, 2, 0, 0, DrvGfxROM0);
			}
		} else {
			if (flip) {
				Render8x8Tile_FlipXY_Clip(pTransDraw, code, sx, sy, color, 2, 0, DrvGfxROM0);
			} else {
				Render8x8Tile_Clip(pTransDraw, code, sx, sy, color, 2, 0, DrvGfxROM0);
			}
		}
	}
}

static void draw_sprites()
{
	for (int offs = 0x200 - 4; offs >= 0; offs -= 4)
	{
		int sy = DrvSprRAM[offs + 0];
		int sx = DrvSprRAM[offs + 3];

		if (sy == 0 || sy == 0xff || (sx < 0x40 && DrvSprRAM[offs + 2] & 0x10) || (sx >= 0xc0 && ~DrvSprRAM[offs + 2] & 0x10))
			continue;

		int code  = (DrvSprRAM[offs + 1] & 0x3f) | ((DrvSprRAM[offs + 2] & 0x60) << 1);

		int color =(DrvSprRAM[offs + 2] & 0x0f) | *color_bank;
		int flipx = DrvSprRAM[offs + 1] & 0x40;
		int flipy = DrvSprRAM[offs + 1] & 0x80;

		if (*flipscreen)
		{
			flipx = !flipx;
			flipy = !flipy;
			sy -= 16;
		}

		if (DrvSprRAM[offs + 2] & 0x80)
		{
			if (*flipscreen) {
				sx = 240 - sx;
			} else {
				sy = 240 - sy;
			}

			if (flipy) {
				if (flipx) {
					Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, code, sx, sy, color, 2, 0, 0, DrvGfxROM1);
				} else {
					Render16x16Tile_Mask_FlipY_Clip(pTransDraw, code, sx, sy, color, 2, 0, 0, DrvGfxROM1);
				}
			} else {
				if (flipx) {
					Render16x16Tile_Mask_FlipX_Clip(pTransDraw, code, sx, sy, color, 2, 0, 0, DrvGfxROM1);
				} else {
					Render16x16Tile_Mask_Clip(pTransDraw, code, sx, sy, color, 2, 0, 0, DrvGfxROM1);
				}
			}
		} else {
			code <<= 2;

			if (*flipscreen) {
				sx = 248 - sx;
			} else {
				sy = 248 - sy;
			}

			if (flipy) {
				if (flipx) {
					Render8x8Tile_Mask_FlipXY_Clip(pTransDraw, code, sx, sy, color, 2, 0, 0, DrvGfxROM0);
				} else {
					Render8x8Tile_Mask_FlipY_Clip(pTransDraw, code, sx, sy, color, 2, 0, 0, DrvGfxROM0);
				}
			} else {
				if (flipx) {
					Render8x8Tile_Mask_FlipX_Clip(pTransDraw, code, sx, sy, color, 2, 0, 0, DrvGfxROM0);
				} else {
					Render8x8Tile_Mask_Clip(pTransDraw, code, sx, sy, color, 2, 0, 0, DrvGfxROM0);
				}
			}
		}
	}
}

static int DrvDraw()
{
	if (DrvRecalc) {
		for (int i = 0; i < 0x100; i++) {
			int p = Palette[i];
			DrvPalette[i] =  BurnHighCol(p >> 16, (p >> 8) & 0xff, p & 0xff, 0);
		}
	}

	BurnTransferClear();

	draw_layer(0x00);
	draw_sprites();
	draw_layer(0x10);

	BurnTransferCopy(DrvPalette);

	return 0;
}

static int DrvFrame()
{
	if (DrvReset) {
		DrvDoReset();
	}

	{
		memset (DrvInputs, 0xff, 2);
		for (int i = 0; i < 8; i++) {
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
		}
	}

	ZetOpen(0);
	ZetRun(4000000 / 60);
	ZetRaiseIrq(0);
	ZetClose();

	if (pBurnSoundOut) {
		int nSample;
		AY8910Update(0, &pAY8910Buffer[0], nBurnSoundLen);
		AY8910Update(1, &pAY8910Buffer[3], nBurnSoundLen);
		for (int n = 0; n < nBurnSoundLen; n++) {
			nSample  = pAY8910Buffer[0][n] >> 2;
			nSample += pAY8910Buffer[1][n] >> 2;
			nSample += pAY8910Buffer[2][n] >> 2;
			nSample += pAY8910Buffer[3][n] >> 2;
			nSample += pAY8910Buffer[4][n] >> 2;
			nSample += pAY8910Buffer[5][n] >> 2;

			if (nSample < -32768) {
				nSample = -32768;
			} else {
				if (nSample > 32767) {
					nSample = 32767;
				}
			}

			pBurnSoundOut[(n << 1) + 0] = nSample;
			pBurnSoundOut[(n << 1) + 1] = nSample;
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

		memset(&ba, 0, sizeof(ba));
		ba.Data	  = AllRam;
		ba.nLen	  = RamEnd - AllRam;
		ba.szName = "All Ram";
		BurnAcb(&ba);

		ZetScan(nAction);

		AY8910Scan(nAction, pnMin);
	}

	return 0;
}


// Ambush

static struct BurnRomInfo ambushjRomDesc[] = {
	{ "ambush.h7",    0x2000, 0xce306563, BRF_ESS | BRF_PRG }, //  0 Z80 Code
	{ "ambush.g7",    0x2000, 0x90291409, BRF_ESS | BRF_PRG }, //  1
	{ "ambush.f7",    0x2000, 0xd023ca29, BRF_ESS | BRF_PRG }, //  2
	{ "ambush.e7",    0x2000, 0x6cc2d3ee, BRF_ESS | BRF_PRG }, //  3

	{ "ambush.n4",    0x2000, 0xecc0dc85, BRF_GRA },	   //  4 Graphics tiles
	{ "ambush.m4",    0x2000, 0xe86ca98a, BRF_GRA },	   //  5

	{ "a.bpr",        0x0100, 0x5f27f511, BRF_GRA },	   //  6 color PROMs

	{ "b.bpr",        0x0100, 0x1b03fd3b, BRF_OPT },	   //  7 Proms - Not used
	{ "13.bpr",	  0x0100, 0x547e970f, BRF_OPT },	   //  8
	{ "14.bpr",	  0x0100, 0x622a8ce7, BRF_OPT },	   //  9
};

STD_ROM_PICK(ambushj)
STD_ROM_FN(ambushj)

struct BurnDriver BurnDrvAmbushj = {
	"ambushj", "ambush", NULL, NULL, "1983",
	"Ambush (Japan)\0", NULL, "Nippon Amuse Co-Ltd", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_PRE90S, GBF_MISC, 0,
	NULL, ambushjRomInfo, ambushjRomName, NULL, NULL, AmbushInputInfo, AmbushDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x100,
	256, 224, 4, 3
};


// Ambush (Tecfri)

static struct BurnRomInfo ambushRomDesc[] = {
	{ "a1.i7",        0x2000, 0xa7cd149d, BRF_ESS | BRF_PRG }, //  0 Z80 Code
	{ "a2.g7",        0x2000, 0x8328d88a, BRF_ESS | BRF_PRG }, //  1
	{ "a3.f7",        0x2000, 0x8db57ab5, BRF_ESS | BRF_PRG }, //  2
	{ "a4.e7",        0x2000, 0x4a34d2a4, BRF_ESS | BRF_PRG }, //  3

	{ "fa2.n4",       0x2000, 0xe7f134ba, BRF_GRA },	   //  4 Graphics tiles
	{ "fa1.m4",       0x2000, 0xad10969e, BRF_GRA },	   //  5

	{ "a.bpr",        0x0100, 0x5f27f511, BRF_GRA },	   //  6 color PROMs

	{ "b.bpr",        0x0100, 0x1b03fd3b, BRF_OPT },	   //  7 Proms - Not used
	{ "13.bpr",	  0x0100, 0x547e970f, BRF_OPT },	   //  8
	{ "14.bpr",	  0x0100, 0x622a8ce7, BRF_OPT },	   //  9
};

STD_ROM_PICK(ambush)
STD_ROM_FN(ambush)

struct BurnDriver BurnDrvAmbush = {
	"ambush", NULL, NULL, NULL, "1983",
	"Ambush\0", NULL, "Tecfri", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_PRE90S, GBF_MISC, 0,
	NULL, ambushRomInfo, ambushRomName, NULL, NULL, AmbushInputInfo, AmbushDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x100,
	256, 224, 4, 3
};


// Ambush (Volt Elec co-ltd)

static struct BurnRomInfo ambushvRomDesc[] = {
	{ "n1_h7.bin",    0x2000, 0x3c0833b4, BRF_ESS | BRF_PRG }, //  0 Z80 Code
	{ "ambush.g7",    0x2000, 0x90291409, BRF_ESS | BRF_PRG }, //  1
	{ "ambush.f7",    0x2000, 0xd023ca29, BRF_ESS | BRF_PRG }, //  2
	{ "ambush.e7",    0x2000, 0x6cc2d3ee, BRF_ESS | BRF_PRG }, //  3

	{ "ambush.n4",    0x2000, 0xecc0dc85, BRF_GRA },	   //  4 Graphics tiles
	{ "ambush.m4",    0x2000, 0xe86ca98a, BRF_GRA },	   //  5

	{ "a.bpr",        0x0100, 0x5f27f511, BRF_GRA },	   //  6 color PROMs

	{ "b.bpr",        0x0100, 0x1b03fd3b, BRF_OPT },	   //  7 Proms - Not used
	{ "13.bpr",	  0x0100, 0x547e970f, BRF_OPT },	   //  8
	{ "14.bpr",	  0x0100, 0x622a8ce7, BRF_OPT },	   //  9
};

STD_ROM_PICK(ambushv)
STD_ROM_FN(ambushv)

struct BurnDriver BurnDrvAmbushv = {
	"ambushv", "ambush", NULL, NULL, "1983",
	"Ambush (Volt Elec co-ltd)\0", NULL, "Volt Elec co-ltd", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_PRE90S, GBF_MISC, 0,
	NULL, ambushvRomInfo, ambushvRomName, NULL, NULL, AmbushInputInfo, AmbushDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x100,
	256, 224, 4, 3
};


