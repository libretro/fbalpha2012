// FB Alpha Minivader driver module
//Based on MAME Driver by Takahiro Nogi

#include "tiles_generic.h"

static unsigned char *AllMem;
static unsigned char *MemEnd;
static unsigned char *AllRam;
static unsigned char *RamEnd;
static unsigned char *DrvZ80ROM;
static unsigned char *DrvZ80RAM;

static unsigned char DrvJoy1[4];
static unsigned char DrvInputs[1];
static unsigned char DrvReset;

static struct BurnInputInfo DrvInputList[] = {
	{"P1 Coin"      , BIT_DIGITAL  , DrvJoy1 + 3,	"p1 coin"  },
	{"P1 Left"      , BIT_DIGITAL  , DrvJoy1 + 0, 	"p1 left"  },
	{"P1 Right"     , BIT_DIGITAL  , DrvJoy1 + 1, 	"p1 right" },
	{"P1 Button 1"  , BIT_DIGITAL  , DrvJoy1 + 2,	"p1 fire 1"},

	{"Reset"        , BIT_DIGITAL  , &DrvReset  ,	"reset"    },
};

STDINPUTINFO(Drv)

unsigned char __fastcall minivdr_read(unsigned short address)
{
	if (address == 0xe008) {
		return DrvInputs[0];
	}

	return 0;
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

	AllRam		= Next;

	DrvZ80RAM	= Next; Next += 0x002000;

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
	ZetMapArea (0x0000, 0x1fff, 0, DrvZ80ROM);
	ZetMapArea (0x0000, 0x1fff, 2, DrvZ80ROM);
	ZetMapArea (0xa000, 0xbfff, 0, DrvZ80RAM);
	ZetMapArea (0xa000, 0xbfff, 1, DrvZ80RAM);
	ZetMapArea (0xa000, 0xbfff, 2, DrvZ80RAM);
	ZetSetReadHandler(minivdr_read);
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
	unsigned int DrvPalette[2];

	DrvPalette[0] = 0;
	DrvPalette[1] = BurnHighCol(0xff, 0xff, 0xff, 0);

	for (int offs = 0x200; offs < 0x1e00; offs++)
	{
		int sx = (offs << 3) & 0xf8;
		int sy = ((offs >> 5) - 0x10) << 8;
		int d = DrvZ80RAM[offs];

		for (int i = 0; i < 8; i++, sx++)
		{
			pTransDraw[sx + sy] = (d >> (7 - i)) & 1;
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
		for (int i = 0; i < 4; i++) {
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
		}
	}

	ZetOpen(0);
	ZetRun(4000000 / 60);
	ZetSetIRQLine(0, ZET_IRQSTATUS_AUTO);
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


// Minivader

static struct BurnRomInfo minivadrRomDesc[] = {
	{ "d26-01.bin",	0x2000, 0xa96c823d, BRF_ESS | BRF_PRG }, //  Z80 code
};

STD_ROM_PICK(minivadr)
STD_ROM_FN(minivadr)

struct BurnDriver BurnDrvminivadr = {
	"minivadr", NULL, NULL, NULL, "1990",
	"Minivader\0", NULL, "Taito Corporation", "Minivader",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 1, HARDWARE_MISC_PRE90S, GBF_SHOOT, 0,
	NULL, minivadrRomInfo, minivadrRomName, NULL, NULL, DrvInputInfo, NULL,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, NULL, 0,
	256, 224, 4, 3
};
