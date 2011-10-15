// FB Alpha Zero Zone driver module
// Based on MAME driver by Brad Oliver

#include "tiles_generic.h"
#include "msm6295.h"

static unsigned char *AllMem;
static unsigned char *MemEnd;
static unsigned char *AllRam;
static unsigned char *RamEnd;
static unsigned char *Drv68KROM;
static unsigned char *Drv68KRAM;
static unsigned char *DrvZ80ROM;
static unsigned char *DrvZ80RAM;
static unsigned char *DrvSndROM;
static unsigned char *DrvGfxROM;
static unsigned char *DrvPalRAM;
static unsigned char *DrvVidRAM;
static unsigned int  *DrvPalette;
static unsigned int  *Palette;

static unsigned char DrvRecalc;

static unsigned char DrvJoy1[16];
static unsigned char DrvJoy2[16];
static unsigned char DrvJoy4[16];
static unsigned char DrvDips[2];
static unsigned char DrvReset;
static unsigned short DrvInputs[4];

static unsigned char soundlatch;
static unsigned char tilebank;

static struct BurnInputInfo DrvInputList[] = {
	{"Coin 1"       	  , BIT_DIGITAL  , DrvJoy1 + 0,	 "p1 coin"  },
	{"Coin 2"       	  , BIT_DIGITAL  , DrvJoy1 + 1,	 "p2 coin"  },

	{"P1 Start"     	  , BIT_DIGITAL  , DrvJoy1 + 3,	 "p1 start" },
	{"P1 Up"        	  , BIT_DIGITAL  , DrvJoy2 + 3,  "p1 up"    },
	{"P1 Down"      	  , BIT_DIGITAL  , DrvJoy2 + 2,  "p1 down"  },
	{"P1 Left"      	  , BIT_DIGITAL  , DrvJoy2 + 1,  "p1 left"  },
	{"P1 Right"     	  , BIT_DIGITAL  , DrvJoy2 + 0,  "p1 right" },
	{"P1 Button 1"  	  , BIT_DIGITAL  , DrvJoy2 + 4,  "p1 fire 1"},
	{"P1 Button 2"  	  , BIT_DIGITAL  , DrvJoy2 + 5,  "p1 fire 2"},
	{"P1 Score Line (cheat)"  , BIT_DIGITAL  , DrvJoy2 + 6,  "p1 fire 3"},

	{"P2 Start"     	  , BIT_DIGITAL  , DrvJoy1 + 4,	 "p2 start" },
	{"P2 Up"        	  , BIT_DIGITAL  , DrvJoy2 + 11, "p2 up"    },
	{"P2 Down"      	  , BIT_DIGITAL  , DrvJoy2 + 10, "p2 down"  },
	{"P2 Left"      	  , BIT_DIGITAL  , DrvJoy2 + 9,  "p2 left"  },
	{"P2 Right"     	  , BIT_DIGITAL  , DrvJoy2 + 8,  "p2 right" },
	{"P2 Button 1"  	  , BIT_DIGITAL  , DrvJoy2 + 12, "p2 fire 1"},
	{"P2 Button 2"  	  , BIT_DIGITAL  , DrvJoy2 + 13, "p2 fire 2"},
	{"P2 Score Line (cheat)"  , BIT_DIGITAL  , DrvJoy2 + 14, "p2 fire 3"},

	{"Service"      	  , BIT_DIGITAL  , DrvJoy4 + 7,  "service"  },

	{"Reset",		    BIT_DIGITAL  , &DrvReset,	 "reset"    },
	{"Dip 1",		    BIT_DIPSWITCH, DrvDips + 0,	 "dip"	   },
	{"Dip 2",		    BIT_DIPSWITCH, DrvDips + 1,	 "dip"	   },
};

STDINPUTINFO(Drv)

static struct BurnDIPInfo DrvDIPList[]=
{
	{0x14, 0xff, 0xff, 0xdf, NULL },
	{0x15, 0xff, 0xff, 0xf7, NULL },

	{0x14, 0xfe, 0,       8, "Coinage" },
	{0x14, 0x01, 0x07, 0x00, "5C 1C" },
	{0x14, 0x01, 0x07, 0x01, "4C 1C" },
	{0x14, 0x01, 0x07, 0x02, "3C 1C" },
	{0x14, 0x01, 0x07, 0x03, "2C 1C" },
	{0x14, 0x01, 0x07, 0x07, "1C 1C" },
	{0x14, 0x01, 0x07, 0x06, "1C 2C" },
	{0x14, 0x01, 0x07, 0x05, "1C 3C" },
	{0x14, 0x01, 0x07, 0x04, "1C 4C" },

	{0x14, 0xfe, 0,       2, "Difficulty" },
	{0x14, 0x01, 0x08, 0x08, "In Game Default" },
	{0x14, 0x01, 0x08, 0x00, "Always Hard" },

	{0x14, 0xfe, 0,       2, "Speed" },
	{0x14, 0x01, 0x10, 0x10, "Normal" },
	{0x14, 0x01, 0x10, 0x00, "Fast" },

	{0x14, 0xfe, 0,       2, "Demo Sounds" },
	{0x14, 0x01, 0x20, 0x00, "Off" },
	{0x14, 0x01, 0x20, 0x20, "On" },

	{0x15, 0xfe, 0,       2, "Helps" },
	{0x15, 0x01, 0x04, 0x04, "1" },
	{0x15, 0x01, 0x04, 0x00, "2" },

	{0x15, 0xfe, 0,       2, "Bonus Help" },
	{0x15, 0x01, 0x08, 0x00, "30000" },
	{0x15, 0x01, 0x08, 0x08, "None" },

	{0x15, 0xfe, 0,       2, "Activate 'Score Line'? (Cheat)" },
	{0x15, 0x01, 0x10, 0x10, "No" },
	{0x15, 0x01, 0x10, 0x00, "Yes" },
};

STDDIPINFO(Drv)

unsigned char __fastcall zerozone_read_byte(unsigned int address)
{
	switch (address)
	{
		case 0x080000:
		case 0x080001:
			return DrvInputs[0] >> ((~address & 1) << 3);

		case 0x080002:
		case 0x080003:
			return DrvInputs[1] >> ((~address & 1) << 3);

		case 0x080008:
		case 0x080009:
			return DrvInputs[2] >> ((~address & 1) << 3);

		case 0x08000a:
		case 0x08000b:
			return DrvInputs[3] >> ((~address & 1) << 3);
	}

	return 0;
}

unsigned short __fastcall zerozone_read_word(unsigned int address)
{
	switch (address)
	{
		case 0x080000:
			return DrvInputs[0];

		case 0x080002:
			return DrvInputs[1];

		case 0x080008:
			return DrvInputs[2];

		case 0x08000a:
			return DrvInputs[3];
	}

	return 0;
}

static void palette_write(int offset)
{
	unsigned char r, b, g;
	unsigned short data = *((unsigned short*)(DrvPalRAM + offset));

	r  = (data >> 11) & 0x1e;
	r |= (data >>  3) & 0x01;
	r  = (r << 3) | (r >> 2);

	g  = (data >>  7) & 0x1e;
	g |= (data >>  2) & 0x01;
	g  = (g << 3) | (g >> 2);

	b  = (data >>  3) & 0x1e;
	b |= (data >>  1) & 0x01;
	b  = (b << 3) | (b >> 2);

	Palette[offset>>1] = (r << 16) | (g << 8) | b;
	DrvPalette[offset>>1] = BurnHighCol(r, g, b, 0);

	return;
}

void __fastcall zerozone_write_word(unsigned int address, unsigned short data)
{
	if ((address & 0xffe00) == 0x88000) {
		*((unsigned short*)(DrvPalRAM + (address & 0x1fe))) = data;
		palette_write(address & 0x1fe);
		return;
	}

	switch (address)
	{
		case 0x84000:
			soundlatch = data >> 8;
			ZetRaiseIrq(0xff);
		return;

		case 0xb4000:
			tilebank = data & 7;
		return;
	}

	return;
}

void __fastcall zerozone_write_byte(unsigned int address, unsigned char data)
{
	switch (address)
	{
		case 0x84000:
			soundlatch = data;
			ZetRaiseIrq(0xff);
		return;

		case 0xb4001:
			tilebank = data & 7;
		return;
	}

	return;
}

void __fastcall zerozone_sound_write(unsigned short address, unsigned char data)
{
	switch (address)
	{
		case 0x9800:
			MSM6295Command(0, data);
		return;
	}
}

unsigned char __fastcall zerozone_sound_read(unsigned short address)
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

	soundlatch = 0;
	tilebank = 0;

	return 0;
}

static int MemIndex()
{
	unsigned char *Next; Next = AllMem;

	Drv68KROM	= Next; Next += 0x020000;
	DrvZ80ROM	= Next; Next += 0x008000;

	DrvGfxROM	= Next; Next += 0x100000;

	MSM6295ROM	= Next;
	DrvSndROM	= Next; Next += 0x040000;

	DrvPalette	= (unsigned int*)Next; Next += 0x0100 * sizeof(int);

	AllRam		= Next;

	Drv68KRAM	= Next; Next += 0x010000;
	DrvPalRAM	= Next; Next += 0x000200;
	DrvVidRAM	= Next; Next += 0x004000;

	DrvZ80RAM	= Next; Next += 0x000800;

	Palette		= (unsigned int*)Next; Next += 0x0100 * sizeof(int);

	RamEnd		= Next;

	MemEnd		= Next;

	return 0;
}

static int DrvGfxDecode()
{
	int Plane[4] = { 0,  1,  2,  3 };
	int XOffs[8] = { 0,  4,  8, 12, 16, 20, 24, 28 };
	int YOffs[8] = { 0, 32, 64, 96, 128, 160, 192, 224 };

	unsigned char *tmp = (unsigned char*)malloc(0x80000);
	if (tmp == NULL) {
		return 1;
	}

	memcpy (tmp, DrvGfxROM, 0x80000);

	GfxDecode(0x4000, 4, 8, 8, Plane, XOffs, YOffs, 0x100, tmp, DrvGfxROM);

	free (tmp);

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
		if (BurnLoadRom(Drv68KROM + 1,		0, 2)) return 1;
		if (BurnLoadRom(Drv68KROM + 0,		1, 2)) return 1;

		if (BurnLoadRom(DrvZ80ROM,		2, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM,		3, 1)) return 1;

		if (BurnLoadRom(DrvSndROM,		4, 1)) return 1;
		if (BurnLoadRom(DrvSndROM + 0x20000,    5, 1)) return 1;

		DrvGfxDecode();
	}

	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM,		0x000000, 0x01ffff, SM_ROM);
	SekMapMemory(DrvPalRAM,		0x088000, 0x0881ff, SM_ROM);
	SekMapMemory(DrvVidRAM,		0x09ce00, 0x09ffff, SM_RAM);
	SekMapMemory(Drv68KRAM,		0x0c0000, 0x0cffff, SM_RAM);
	SekSetWriteByteHandler(0,	zerozone_write_byte);
	SekSetWriteWordHandler(0,	zerozone_write_word);
	SekSetReadByteHandler(0,	zerozone_read_byte);
	SekSetReadWordHandler(0,	zerozone_read_word);
	SekClose();

	ZetInit(1);
	ZetOpen(0);
	ZetMapArea(0x0000, 0x7fff, 0, DrvZ80ROM);
	ZetMapArea(0x0000, 0x7fff, 2, DrvZ80ROM);
	ZetMapArea(0x8000, 0x87ff, 0, DrvZ80RAM);
	ZetMapArea(0x8000, 0x87ff, 1, DrvZ80RAM);
	ZetMapArea(0x8000, 0x87ff, 2, DrvZ80RAM);
	ZetSetWriteHandler(zerozone_sound_write);
	ZetSetReadHandler(zerozone_sound_read);
	ZetMemEnd();
	ZetClose();

	MSM6295Init(0, 1056000 / 132, 100.0, 0);

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static int DrvExit()
{
	GenericTilesExit();

	MSM6295Exit(0);
	SekExit();
	ZetExit();

	free (AllMem);
	AllMem = NULL;

	MSM6295ROM = NULL;

	return 0;
}

static int DrvDraw()
{
	if (DrvRecalc) {
		for (int i = 0; i < 0x100; i++) {
			int rgb = Palette[i];
			DrvPalette[i] = BurnHighCol(rgb >> 16, rgb >> 8, rgb, 0);
		}
	}

	unsigned short *vram = (unsigned short*)DrvVidRAM;

	for (int offs = 0; offs < 0x800; offs++)
	{
		int sy = (offs & 0x1f) << 3;
		int sx = (offs >> 5) << 3;

		sy -= 16;
		sx -=  8;
		if (sy < 0 || sx < 0 || sy >= nScreenHeight || sx >= nScreenWidth) continue;

		int code  = vram[offs] & 0x7ff;
		int color = vram[offs] >> 12;

		code += tilebank * (vram[offs] & 0x800);

		Render8x8Tile(pTransDraw, code, sx, sy, color, 4, 0, DrvGfxROM);
	}

	BurnTransferCopy(DrvPalette);

	return 0;
}


static int DrvFrame()
{
	if (DrvReset) {
		DrvDoReset();
	}

	{
		DrvInputs[0] = ~0;
		DrvInputs[1] = ~0;
		DrvInputs[2] = 0x00ff | (DrvDips[1] << 8);
		DrvInputs[3] = 0xff00 | DrvDips[0];

		for (int i = 0; i < 16; i++)
		{
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
			DrvInputs[3] ^= (DrvJoy4[i] & 1) << i;
		}
	}

	int nSegment;
	int nInterleave = 10;
	int nTotalCycles[2] = { 10000000 / 60, 1000000 / 60 };
	int nCyclesDone[2] = { 0, 0 };

	SekOpen(0);
	ZetOpen(0);

	for (int i = 0; i < nInterleave; i++)
	{
		nSegment = (nTotalCycles[0] - nCyclesDone[0]) / (nInterleave - i);

		nCyclesDone[0] += SekRun(nSegment);

		nSegment = (nTotalCycles[1] - nCyclesDone[1]) / (nInterleave - i);

		nCyclesDone[1] += ZetRun(nSegment);
	}

	if (pBurnSoundOut) {
		MSM6295Render(0, pBurnSoundOut, nBurnSoundLen);
	}

	SekSetIRQLine(1, SEK_IRQSTATUS_AUTO);

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
		*pnMin = 0x029692;
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

		MSM6295Scan(0, nAction);

		SCAN_VAR(soundlatch);
		SCAN_VAR(tilebank);
	}

	return 0;
}


// Zero Zone

static struct BurnRomInfo zerozoneRomDesc[] = {
	{ "zz-4.rom",	0x10000, 0x83718b9b, 1 | BRF_PRG | BRF_ESS }, //  0 - 68k Code
	{ "zz-5.rom",	0x10000, 0x18557f41, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "zz-1.rom",	0x08000, 0x223ccce5, 2 | BRF_PRG | BRF_ESS }, //  2 - Z80 Code

	{ "zz-6.rom",	0x80000, 0xc8b906b9, 3 | BRF_GRA },	      //  3 - Tiles

	{ "zz-2.rom",	0x20000, 0xc7551e81, 4 | BRF_SND },	      //  4 - MSM6295 Samples
	{ "zz-3.rom",	0x20000, 0xe348ff5e, 4 | BRF_SND },	      //  5
};

STD_ROM_PICK(zerozone)
STD_ROM_FN(zerozone)

struct BurnDriver BurnDrvZerozone = {
	"zerozone", NULL, NULL, NULL, "1993",
	"Zero Zone\0", NULL, "Comad", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_VSFIGHT, 0,
	NULL, zerozoneRomInfo, zerozoneRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x100,
	368, 224, 4, 3
};


// Las Vegas Girl (Girl '94)

static struct BurnRomInfo lvgirl94RomDesc[] = {
	{ "rom4",	0x10000, 0xc4fb449e, 1 | BRF_PRG | BRF_ESS }, //  0 - 68k Code
	{ "rom5",	0x10000, 0x5d446a1a, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "rom1",	0x08000, 0x223ccce5, 2 | BRF_PRG | BRF_ESS }, //  2 - Z80 Code

	{ "rom6",	0x40000, 0xeeeb94ba, 3 | BRF_GRA },	      //  3 - Tiles

	{ "rom2",	0x20000, 0xc7551e81, 4 | BRF_SND },	      //  4 - MSM6295 Samples
	{ "rom3",	0x20000, 0xe348ff5e, 4 | BRF_SND },	      //  5
};

STD_ROM_PICK(lvgirl94)
STD_ROM_FN(lvgirl94)

struct BurnDriver BurnDrvLvgirl94 = {
	"lvgirl94", NULL, NULL, NULL, "1994",
	"Las Vegas Girl (Girl '94)\0", NULL, "Comad", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_VSFIGHT, 0,
	NULL, lvgirl94RomInfo, lvgirl94RomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x100,
	368, 224, 4, 3
};
