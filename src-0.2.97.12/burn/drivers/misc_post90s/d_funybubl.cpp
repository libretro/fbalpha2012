// FB Alpha Funny Bubble driver module
// Based on MAME driver by Pierpaolo Prazzoli and David Haywood

#include "tiles_generic.h"
#include "msm6295.h"

// To do: 
//	Fix sound after save state

static unsigned char *AllMem;
static unsigned char *MemEnd;
static unsigned char *AllRam;
static unsigned char *RamEnd;
static unsigned char *DrvZ80ROM0;
static unsigned char *DrvZ80ROM1;
static unsigned char *DrvGfxROM0;
static unsigned char *DrvGfxROM1;
static unsigned char *DrvSndROM;
static unsigned char *DrvZ80RAM0;
static unsigned char *DrvZ80RAM1;
static unsigned char *DrvPalRAM;
static unsigned char *DrvVidRAM;

static unsigned char DrvReset;
static unsigned char DrvJoy1[8];
static unsigned char DrvJoy2[8];
static unsigned char DrvJoy3[8];
static unsigned char DrvDips[1];
static unsigned char DrvInps[3];

static unsigned int  *DrvPalette;
static unsigned int  *Palette;

static unsigned char DrvRecalc;

static unsigned char soundlatch;
static int 	     nDrvRomBank;
static int	     nDrvVidRAMBank;
static int 	     nDrvOkiBank;

static struct BurnInputInfo FunybublInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy2 + 0,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy2 + 1,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy2 + 2,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy2 + 3,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p1 fire 1"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy1 + 1,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy1 + 3,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy3 + 0,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy3 + 1,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy3 + 2,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy3 + 3,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy3 + 4,	"p2 fire 1"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
};

STDINPUTINFO(Funybubl)


static struct BurnDIPInfo FunybublDIPList[]=
{
	// Default Values
	{0x0f, 0xff, 0xff, 0x3f, NULL			},

	{0   , 0xfe, 0   ,    8, "Coin_A"		},
	{0x0f, 0x01, 0x07, 0x01, "3 Coins 1 Credits "	},
	{0x0f, 0x01, 0x07, 0x02, "2 Coins 1 Credits "	},
	{0x0f, 0x01, 0x07, 0x03, "3 Coins 2 Credits "	},
	{0x0f, 0x01, 0x07, 0x07, "1 Coin 1 Credits "	},
	{0x0f, 0x01, 0x07, 0x04, "2 Coins 3 Credits "	},
	{0x0f, 0x01, 0x07, 0x06, "1 Coin 2 Credits "	},
	{0x0f, 0x01, 0x07, 0x05, "1 Coin 3 Credits "	},
	{0x0f, 0x01, 0x07, 0x00, "Free_Play"		},

	{0   , 0xfe, 0   ,    7, "Difficulty"		},
	{0x0f, 0x01, 0x38, 0x30, "Very_Easy"		},
	{0x0f, 0x01, 0x38, 0x28, "Easy"			},
	{0x0f, 0x01, 0x38, 0x38, "Normal"		},
	{0x0f, 0x01, 0x38, 0x20, "Hard 1"		},
	{0x0f, 0x01, 0x38, 0x18, "Hard 2"		},
	{0x0f, 0x01, 0x38, 0x10, "Hard 3"		},
	{0x0f, 0x01, 0x38, 0x08, "Hard 4"		},

	{0   , 0xfe, 0   ,    2, "Demo_Sounds"		},
	{0x0f, 0x01, 0x40, 0x40, "Off"			},
	{0x0f, 0x01, 0x40, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Nudity"		},
	{0x0f, 0x01, 0x80, 0x80, "Semi"			},
	{0x0f, 0x01, 0x80, 0x00, "Full"			},
};

STDDIPINFO(Funybubl)


unsigned char __fastcall funybubl_in(unsigned short port)
{
	switch (port & 0xff)
	{
		case 0x00:
		case 0x01:
		case 0x02:
			return DrvInps[port & 3];

		case 0x03:
			return DrvDips[0];

		case 0x06:
			return 0;
	}

	return 0;
}

void __fastcall funybubl_write(unsigned short address, unsigned char data)
{
	if (address >= 0xc400 && address <= 0xcfff) {

		int offset = address - 0xc400;

		DrvPalRAM[offset] = data;

		unsigned int coldat;

		offset &= 0xfffc;

		coldat = DrvPalRAM[offset] | (DrvPalRAM[offset+1] << 8) | (DrvPalRAM[offset+2] << 16) | (DrvPalRAM[offset+3] << 24);

		unsigned char r, g, b;

		r  = (coldat >> 12) << 2;
		r |= r >> 6;

		g  = (coldat >>  0) << 2;
		g |= g >> 6;

		b  = (coldat >>  6) << 2;
		b |= b >> 6;

		Palette[offset >> 2] = (r << 16) | (g << 8) | b;
		DrvPalette[offset >> 2] = BurnHighCol(r, g, b, 0);

		return;
	}
}

static void funybubl_set_vidram_bank(int data)
{
	nDrvVidRAMBank = data & 1;

	ZetMapArea(0xd000, 0xdfff, 0, DrvVidRAM + nDrvVidRAMBank * 0x1000);
	ZetMapArea(0xd000, 0xdfff, 1, DrvVidRAM + nDrvVidRAMBank * 0x1000);
	ZetMapArea(0xd000, 0xdfff, 2, DrvVidRAM + nDrvVidRAMBank * 0x1000);
}

static void funybubl_set_rom_bank(int data)
{
	nDrvRomBank = data & 0x3f;

	ZetMapArea(0x8000, 0xbfff, 0, DrvZ80ROM0 + nDrvRomBank * 0x4000);
	ZetMapArea(0x8000, 0xbfff, 1, DrvZ80ROM0 + nDrvRomBank * 0x4000);
	ZetMapArea(0x8000, 0xbfff, 2, DrvZ80ROM0 + nDrvRomBank * 0x4000);
}

static void funybubl_set_oki_bank(int data)
{
	nDrvOkiBank = data & 1;

	MSM6295ROM = DrvSndROM + nDrvOkiBank * 0x40000;
}

void __fastcall funybubl_out(unsigned short port, unsigned char data)
{
	switch (port & 0xff)
	{
		case 0x00:
			funybubl_set_vidram_bank(data);
		return;

		case 0x01:
			funybubl_set_rom_bank(data);
		return;

		case 0x03:
			soundlatch = data;

			ZetClose();
			ZetOpen(1);
			ZetRaiseIrq(0);
			ZetClose();
			ZetOpen(0);
		return;

		case 0x06:
		case 0x07:
		return;
	}
}

void __fastcall funybubl_sound_write(unsigned short address, unsigned char data)
{
	switch (address)
	{
		case 0x9000:
			funybubl_set_oki_bank(data);
		return;

		case 0x9800:
			MSM6295Command(0, data);
		return;
	}
}

unsigned char __fastcall funybubl_sound_read(unsigned short address)
{
	switch (address)
	{
		case 0x9800:
			return MSM6295ReadStatus(0);

		case 0xa000:
			return soundlatch;
	}

	return 0;
}

static int DrvGfxDecode()
{
	int Plane0[8]  = { 0x200000 * 3, 0x200000 * 2, 0x200000 * 1, 0x200000 * 0,
			   0x200000 * 7, 0x200000 * 6, 0x200000 * 5, 0x200000 * 4 };
	int XOffs0[8]  = { 0, 1, 2, 3, 4, 5, 6, 7 };
	int YOffs0[8]  = { 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 };

	int Plane1[8]  = { 0x400000 * 3 + 4, 0x400000 * 3 + 0, 0x400000 * 2 + 4, 0x400000 * 2 + 0,
			   0x400000 * 1 + 4, 0x400000 * 1 + 0, 0x400000 * 0 + 4, 0x400000 * 0 + 0 };
	int XOffs1[16] = { 0, 1,2,3, 8,9,10,11, 256,257,258,259, 264,265,266,267 };
	int YOffs1[16] = { 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
	  			8*16, 9*16,10*16,11*16,12*16,13*16,14*16,15*16 };

	unsigned char *tmp = (unsigned char*)malloc(0x200000);

	if (tmp == NULL) {
		return 1;
	}

	for (int i = 0; i < 0x200000; i++) { // invert
		DrvGfxROM0[i] ^= 0xff;
		DrvGfxROM1[i] ^= 0xff;
	}

	memcpy (tmp, DrvGfxROM0, 0x200000);

	GfxDecode(0x8000, 8,  8,  8, Plane0, XOffs0, YOffs0, 0x040, tmp, DrvGfxROM0);

	memcpy (tmp, DrvGfxROM1, 0x200000);

	GfxDecode(0x2000, 8, 16, 16, Plane1, XOffs1, YOffs1, 0x200, tmp, DrvGfxROM1);

	free (tmp);

	return 0;
}

static int MemIndex()
{
	unsigned char *Next; Next = AllMem;

	DrvZ80ROM0	= Next; Next += 0x040000;
	DrvZ80ROM1	= Next; Next += 0x008000;

	MSM6295ROM 	= Next;
	DrvSndROM	= Next; Next += 0x080000;

	DrvGfxROM0	= Next; Next += 0x200000;
	DrvGfxROM1	= Next; Next += 0x200000;

	DrvPalette	= (unsigned int*)Next; Next += 0x0300 * sizeof(int);

	AllRam		= Next;

	DrvZ80RAM0	= Next; Next += 0x002000;
	DrvPalRAM	= Next; Next += 0x000c00;
	DrvVidRAM	= Next; Next += 0x002000;

	DrvZ80RAM1	= Next; Next += 0x000800;

	Palette		= (unsigned int*)Next; Next += 0x0300 * sizeof(int);

	RamEnd		= Next;

	MemEnd		= Next;

	return 0;
}

static int DrvDoReset()
{
	DrvReset = 0;

	memset (AllRam, 0, RamEnd - AllRam);

	ZetOpen(0);
	ZetReset();
	funybubl_set_vidram_bank(0);
	funybubl_set_rom_bank(0);
	ZetClose();

	ZetOpen(1);
	ZetReset();
	ZetClose();

	funybubl_set_oki_bank(0);
	MSM6295Reset(0);

	soundlatch = 0;

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
		if (BurnLoadRom(DrvZ80ROM0, 0, 1)) return 1;

		for (int i = 0; i < 8; i++)
			if (BurnLoadRom(DrvGfxROM0 + i * 0x40000, i + 1, 1)) return 1;

		for (int i = 0; i < 4; i++)
			if (BurnLoadRom(DrvGfxROM1 + i * 0x80000, i + 9, 1)) return 1;

		if (BurnLoadRom(DrvZ80ROM1, 13, 1)) return 1;

		if (BurnLoadRom(DrvSndROM, 14, 1)) return 1;
		if (BurnLoadRom(DrvSndROM + 0x20000, 15, 1)) return 1;

		memcpy (DrvSndROM + 0x60000, DrvSndROM + 0x40000, 0x20000);
		memcpy (DrvSndROM + 0x40000, DrvSndROM + 0x00000, 0x20000);

		DrvGfxDecode();
	}

	ZetInit(2);
	ZetOpen(0);
	ZetMapArea(0x0000, 0x7fff, 0, DrvZ80ROM0);
	ZetMapArea(0x0000, 0x7fff, 2, DrvZ80ROM0);
	ZetMapArea(0xc400, 0xcfff, 0, DrvPalRAM);
	ZetMapArea(0xc400, 0xcfff, 2, DrvPalRAM);
	ZetMapArea(0xe000, 0xffff, 0, DrvZ80RAM0);
	ZetMapArea(0xe000, 0xffff, 1, DrvZ80RAM0);
	ZetMapArea(0xe000, 0xffff, 2, DrvZ80RAM0);
	ZetSetWriteHandler(funybubl_write);
	ZetSetOutHandler(funybubl_out);
	ZetSetInHandler(funybubl_in);
	ZetMemEnd();
	ZetClose();

	ZetOpen(1);
	ZetMapArea(0x0000, 0x7fff, 0, DrvZ80ROM1);
	ZetMapArea(0x0000, 0x7fff, 2, DrvZ80ROM1);
	ZetMapArea(0x8000, 0x87ff, 0, DrvZ80RAM1);
	ZetMapArea(0x8000, 0x87ff, 1, DrvZ80RAM1);
	ZetMapArea(0x8000, 0x87ff, 2, DrvZ80RAM1);
	ZetSetWriteHandler(funybubl_sound_write);
	ZetSetReadHandler(funybubl_sound_read);
	ZetMemEnd();
	ZetClose();

	MSM6295Init(0, 1056000 / 132, 100.0, 0);

	GenericTilesInit();

	return 0;
}

static int DrvExit()
{
	GenericTilesExit();

	MSM6295Exit(0);

	ZetExit();

	free (AllMem);
	AllMem = NULL;

	MSM6295ROM = NULL;

	return 0;
}

static void draw_sprites()
{
	unsigned char *source = &DrvVidRAM[0x2000-0x20];
	unsigned char *finish = source - 0x1000;

	while (source > finish)
	{
		int sx, sy, code;

		sy = source[2];
		sx = source[3];
		code =  source[0] | ( (source[1] & 0x0f) <<8);
		if (source[1] & 0x80) code += 0x1000;
		if (source[1] & 0x20) {	if (sx < 0xe0) sx += 0x100; }

		sx -= 96;
		sy -= 16;

		if ((sx >= 0 && sx < 305) || (sy >= 0 && sy < 208)) {
			Render16x16Tile_Mask_Clip(pTransDraw, code, sx, sy, 0, 8, 0xff, 0, DrvGfxROM1);
		}

		source -= 0x20;
	}
}

static void draw_background()
{
	for (int offs = 0; offs < 0x1000; offs+=2)
	{
		int sx = ((offs >> 1) & 0x3f) << 3;
		int sy = (offs >> 7) << 3;

		sx -= 96;
		sy -= 16;
		if (sx < 0 || sx > 312 || sy < 0 || sy > 216) continue;

		int code = DrvVidRAM[offs] | (DrvVidRAM[offs + 1] << 8);

		Render8x8Tile_Mask_Clip(pTransDraw, code & 0x7fff, sx, sy, 1+(code>>15), 8, 0, 0, DrvGfxROM0);
	}
}

static int DrvDraw()
{
	if (DrvRecalc) {
		for (int i = 0; i < 0x300; i++) {
			int rgb = Palette[i];
			DrvPalette[i] = BurnHighCol(rgb >> 16, rgb >> 8, rgb, 0);
		}
	}

	memset (pTransDraw, 0, nScreenWidth * nScreenHeight * 2);

	draw_background();
	draw_sprites();

	BurnTransferCopy(DrvPalette);

	return 0;
}

static int DrvFrame()
{
	if (DrvReset) {
		DrvDoReset();
	}

	{
		memset (DrvInps, 0xff, 3);
		for (int i = 0; i < 8; i++) {
			DrvInps[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInps[1] ^= (DrvJoy2[i] & 1) << i;
			DrvInps[2] ^= (DrvJoy3[i] & 1) << i;
		}
	}

	int nInterleave = 10;
	int nCyclesTotal[2] = { 6000000 / 60, 4000000 / 60 };
	int nCyclesDone[2] = { 0, 0 };

	for (int i = 0; i < nInterleave; i++)
	{
		int nSegment;

		ZetOpen(0);
		nSegment = (nCyclesTotal[0] - nCyclesDone[0]) / (nInterleave - i);
		nCyclesDone[0] += ZetRun(nSegment);
		if (i == (nInterleave - 1)) ZetRaiseIrq(0);
		ZetClose();

		ZetOpen(1);
		nSegment = (nCyclesTotal[1] - nCyclesDone[1]) / (nInterleave - i);
		nCyclesDone[1] += ZetRun(nSegment);
		ZetClose();
	}

	if (pBurnSoundOut) {
		MSM6295Render(0, pBurnSoundOut, nBurnSoundLen);
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
		*pnMin = 0x029697;
	}

	if (nAction & ACB_MEMORY_RAM) {	
		memset(&ba, 0, sizeof(ba));

		ba.Data	  = AllRam;
		ba.nLen	  = RamEnd - AllRam;
		ba.szName = "All Ram";
		BurnAcb(&ba);
	}

	if (nAction & ACB_DRIVER_DATA) {
		ZetScan(nAction);

		MSM6295Scan(0, nAction);

		SCAN_VAR(soundlatch);
		SCAN_VAR(nDrvOkiBank);
		SCAN_VAR(nDrvRomBank);
		SCAN_VAR(nDrvVidRAMBank);
	}

	if (nAction & ACB_WRITE) {
		funybubl_set_oki_bank(nDrvOkiBank);
		funybubl_set_rom_bank(nDrvRomBank);
		funybubl_set_vidram_bank(nDrvVidRAMBank);
	}

	return 0;
}

// Funny Bubble

static struct BurnRomInfo funybublRomDesc[] = {
	{ "a.ub16",	0x40000, 0x4e799cdd, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 #0 Code

	{ "f.ug13",	0x40000, 0x64d7163d, 2 | BRF_GRA },           //  1 Background Tiles
	{ "g.uh13",	0x40000, 0x6891e2b8, 2 | BRF_GRA },           //  2
	{ "h.ug15",	0x40000, 0xca7f7528, 2 | BRF_GRA },           //  3
	{ "i.uh15",	0x40000, 0x23608ec6, 2 | BRF_GRA },           //  4
	{ "l.ug16",	0x40000, 0x0acf8143, 2 | BRF_GRA },           //  5
	{ "m.uh16",	0x40000, 0x55ed8d9c, 2 | BRF_GRA },           //  6
	{ "n.ug17",	0x40000, 0x52398b68, 2 | BRF_GRA },           //  7
	{ "o.uh17",	0x40000, 0x446e31b2, 2 | BRF_GRA },           //  8

	{ "d.ug1",	0x80000, 0xb7ebbc00, 3 | BRF_GRA },           //  9 Sprites
	{ "e.ug2",	0x80000, 0x28afc396, 3 | BRF_GRA },           // 10
	{ "j.ug3",	0x80000, 0x9e8687cd, 3 | BRF_GRA },           // 11
	{ "k.ug4",	0x80000, 0x63f0e810, 3 | BRF_GRA },           // 12

	{ "p.su6",	0x08000, 0x33169d4d, 4 | BRF_PRG | BRF_ESS }, // 13 Z80 #1 Code

	{ "b.su12",	0x20000, 0xa2d780f4, 5 | BRF_SND },           // 14 Samples
	{ "c.su13",	0x40000, 0x1f7e9269, 5 | BRF_SND },           // 15
};

STD_ROM_PICK(funybubl)
STD_ROM_FN(funybubl)

struct BurnDriver BurnDrvFunybubl = {
	"funybubl", NULL, NULL, NULL, "1999",
	"Funny Bubble\0", NULL, "In Chang Electronic Co", "misc",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_PUZZLE, 0,
	NULL, funybublRomInfo, funybublRomName, NULL, NULL, FunybublInputInfo, FunybublDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x300,
	320, 224, 4, 3
};


// Funny Bubble (Comad version)

static struct BurnRomInfo funybubcRomDesc[] = {
	{ "2.ub16",	0x40000, 0xd684c13f, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 #0 Code

	{ "7.ug12",	0x40000, 0x87603d7b, 2 | BRF_GRA },           //  1 Background Tiles
	{ "8.uh13",	0x40000, 0xab6031bd, 2 | BRF_GRA },           //  2
	{ "9.ug15",	0x40000, 0x0e8352ff, 2 | BRF_GRA },           //  3
	{ "10.uh15",	0x40000, 0xdf7dd356, 2 | BRF_GRA },           //  4
	{ "13.ug16",	0x40000, 0x9f57bdd5, 2 | BRF_GRA },           //  5
	{ "14.uh16",	0x40000, 0x2ac15ea3, 2 | BRF_GRA },           //  6
	{ "15.ug17",	0x40000, 0x9a5e66a6, 2 | BRF_GRA },           //  7
	{ "16.uh17",	0x40000, 0x218060b3, 2 | BRF_GRA },           //  8

	{ "5.ug1",	0x80000, 0xb7ebbc00, 3 | BRF_GRA },           //  9 Sprites
	{ "6.ug2",	0x80000, 0x28afc396, 3 | BRF_GRA },           // 10
	{ "11.ug3",	0x80000, 0x9e8687cd, 3 | BRF_GRA },           // 11
	{ "12.ug4",	0x80000, 0x63f0e810, 3 | BRF_GRA },           // 12

	{ "1.su6",	0x08000, 0x33169d4d, 4 | BRF_PRG | BRF_ESS }, // 13 Z80 #1 Code

	{ "3.su12",	0x20000, 0xa2d780f4, 5 | BRF_SND },           // 14 Samples
	{ "4.su13",	0x40000, 0x1f7e9269, 5 | BRF_SND },           // 15
};

STD_ROM_PICK(funybubc)
STD_ROM_FN(funybubc)

struct BurnDriver BurnDrvFunybubc = {
	"funybublc", "funybubl", NULL, NULL, "1999",
	"Funny Bubble (Comad version)\0", NULL, "Comad Industry Co Ltd", "misc",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_POST90S, GBF_PUZZLE, 0,
	NULL, funybubcRomInfo, funybubcRomName, NULL, NULL, FunybublInputInfo, FunybublDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x300,
	320, 224, 4, 3
};
