// FB Alpha Dottori Kun driver module
// Based on MAME driver by Takahiro Nogi

#include "tiles_generic.h"

static unsigned char *AllMem;
static unsigned char *MemEnd;
static unsigned char *AllRam;
static unsigned char *RamEnd;
static unsigned char *DrvZ80ROM;
static unsigned char *DrvZ80RAM;
static unsigned int *DrvPalette;
static unsigned char DrvRecalc;

static unsigned char DrvJoy1[8];
static unsigned char DrvReset;
static unsigned char DrvInputs[1];

static unsigned char *nColor;

static struct BurnInputInfo DrvInputList[] = {
	{"P1 Up",	BIT_DIGITAL,	DrvJoy1 + 0,    "p1 up"    },
	{"P1 Down",	BIT_DIGITAL,	DrvJoy1 + 1,    "p1 down", },
	{"P1 Left",	BIT_DIGITAL,	DrvJoy1 + 2, 	"p1 left"  },
	{"P1 Right",	BIT_DIGITAL,	DrvJoy1 + 3, 	"p1 right" },
	{"P1 Button 1",	BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"},
	{"P1 Button 2",	BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"},
	{"P1 start",    BIT_DIGITAL,	DrvJoy1 + 6,	"p1 start" },
	{"P1 Coin",	BIT_DIGITAL,	DrvJoy1 + 7,	"p1 coin"  },

	{"Reset",	BIT_DIGITAL,	&DrvReset,	"reset"    },
};

STDINPUTINFO(Drv)

unsigned char __fastcall dotrikun_in_port(unsigned short port)
{
	switch (port & 0xff)
	{
		case 0x00:
			return DrvInputs[0];
	}

	return 0;
}

void __fastcall dotrikun_out_port(unsigned short port, unsigned char data)
{
	switch (port & 0xff)
	{
		case 0x00:
			*nColor = data;
		return;
	}
}

static int DrvDoReset()
{
	DrvReset = 0;

	memset (AllRam, 0, RamEnd - AllRam);

	ZetOpen(0);
	ZetReset();
	ZetClose();

	return 0;
}

static int MemIndex()
{
	unsigned char *Next; Next = AllMem;

	DrvZ80ROM	= Next; Next += 0x010000;

	DrvPalette	= (unsigned int*)Next; Next += 0x000002 * sizeof (int);

	AllRam		= Next;

	DrvZ80RAM	= Next; Next += 0x000800;

	nColor		= Next; Next += 0x000001;

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
		if (BurnLoadRom(DrvZ80ROM, 0, 1)) return 1;
	}

	ZetInit(1);
	ZetOpen(0);
	ZetMapArea (0x0000, 0x3fff, 0, DrvZ80ROM);
	ZetMapArea (0x0000, 0x3fff, 2, DrvZ80ROM);
	ZetMapArea (0x8000, 0x87ff, 0, DrvZ80RAM);
	ZetMapArea (0x8000, 0x87ff, 1, DrvZ80RAM);
	ZetMapArea (0x8000, 0x87ff, 2, DrvZ80RAM);
	ZetSetOutHandler(dotrikun_out_port);
	ZetSetInHandler(dotrikun_in_port);
	ZetMemEnd();
	ZetClose();

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static int DrvExit()
{
	GenericTilesExit();

	ZetExit();

	free (AllMem);
	AllMem = NULL;

	return 0;
}

static int DrvDraw()
{
	if (DrvRecalc) {
		DrvPalette[0] = BurnHighCol((*nColor & 0x08) ? 0xff : 0, (*nColor & 0x10) ? 0xff : 0, (*nColor & 0x20) ? 0xff : 0, 0);
		DrvPalette[1] = BurnHighCol((*nColor & 0x01) ? 0xff : 0, (*nColor & 0x02) ? 0xff : 0, (*nColor & 0x04) ? 0xff : 0, 0);
	}

	for (int offs = 0; offs < 0x0600; offs++)
	{
		int sx = (offs & 0x0f) << 4;
		int sy = (offs >> 4) << 1;
		int px = DrvZ80RAM[offs];

		for (int i = 0; i < 8; i++, sx+=2)
		{
			int pen = (px >> (7 - i)) & 1;

			if (sx > nScreenWidth || sy >= nScreenHeight) continue;

			pTransDraw[((sy + 0) << 8) | (sx + 0)] = pen;
			pTransDraw[((sy + 0) << 8) | (sx + 1)] = pen;
			pTransDraw[((sy + 1) << 8) | (sx + 0)] = pen;
			pTransDraw[((sy + 1) << 8) | (sx + 1)] = pen;
		}
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
		DrvInputs[0] = 0xff;
		for (int i = 0; i < 8; i++) {
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
		}
	}

	ZetOpen(0);
	ZetRun(4000000 / 60);
	ZetRaiseIrq(0);
	ZetClose();

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
		ba.Data	  = AllRam;
		ba.nLen	  = RamEnd - AllRam;
		ba.szName = "All Ram";
		BurnAcb(&ba);

		ZetScan(nAction);
	}

	return 0;
}


// Dottori Kun (new version)

static struct BurnRomInfo dotrikunRomDesc[] = {
	{ "14479a.mpr",	0x4000, 0xb77a50db, BRF_ESS | BRF_PRG }, //  Z80 code
};

STD_ROM_PICK(dotrikun)
STD_ROM_FN(dotrikun)

struct BurnDriver BurnDrvdotrikun = {
	"dotrikun", NULL, NULL, NULL, "1990",
	"Dottori Kun (new version)\0", NULL, "Sega", "Test Hardware",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 1, HARDWARE_MISC_PRE90S, GBF_MAZE, 0,
	NULL, dotrikunRomInfo, dotrikunRomName, NULL, NULL, DrvInputInfo, NULL,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x02,
	256, 192, 4, 3
};


// Dottori Kun (old version)

static struct BurnRomInfo dotriku2RomDesc[] = {
	{ "14479.mpr",	0x4000, 0xa6aa7fa5, BRF_ESS | BRF_PRG }, //  Z80 code
};

STD_ROM_PICK(dotriku2)
STD_ROM_FN(dotriku2)

struct BurnDriver BurnDrvdotriku2 = {
	"dotrikun2", "dotrikun", NULL, NULL, "1990",
	"Dottori Kun (old version)\0", NULL, "Sega", "Test Hardware",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 1, HARDWARE_MISC_PRE90S, GBF_MAZE, 0,
	NULL, dotriku2RomInfo, dotriku2RomName, NULL, NULL, DrvInputInfo, NULL,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x02,
	256, 192, 4, 3
};
