// FB Alpha "Got-ya" driver module
// Based on MAME driver by Nicola Salmoria

#include "tiles_generic.h"
#include "burn_ym2151.h"
#include "msm6295.h"
#include "bitswap.h"

static unsigned char *AllMem;
static unsigned char *MemEnd;
static unsigned char *AllRam;
static unsigned char *RamEnd;
static unsigned char *Drv68KROM;
static unsigned char *Drv68KRAM;
static unsigned char *DrvZ80ROM;
static unsigned char *DrvZ80RAM;
static unsigned char *DrvSndROM;
static unsigned char *DrvSndBank;
static unsigned char *DrvGfxROM0;
static unsigned char *DrvGfxROM1;
static unsigned char *DrvPalRAM;
static unsigned char *DrvFgRAM;
static unsigned char *DrvBgRAM;
static unsigned char *DrvSprRAM;
static unsigned char *DrvGfxBank;
static unsigned short *DrvScrollx;
static unsigned short *DrvScrolly;
static unsigned int  *DrvPalette;

static unsigned char DrvRecalc;

static unsigned char DrvJoy1[16];
static unsigned char DrvJoy2[16];
static unsigned char DrvDips[2];
static unsigned char DrvReset;
static unsigned short DrvInputs[3];

static unsigned char *bankselect;
static unsigned char *soundlatch;

static struct BurnInputInfo GotchaInputList[] = {
	{"Coin A",		BIT_DIGITAL,	DrvJoy2 + 8,	"p1 coin"	},
	{"Coin B",		BIT_DIGITAL,	DrvJoy2 + 9,	"p2 coin"	},

	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 start"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 fire 2"	},
	{"P1 Button 3",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 fire 3"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy1 + 7,	"p2 start"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p2 fire 2"	},
	{"P2 Button 3",		BIT_DIGITAL,	DrvJoy1 + 6,	"p2 fire 3"	},

	{"P3 Start",		BIT_DIGITAL,	DrvJoy1 + 11,	"p3 start"	},
	{"P3 Button 1",		BIT_DIGITAL,	DrvJoy1 + 8,	"p3 fire 1"	},
	{"P3 Button 2",		BIT_DIGITAL,	DrvJoy1 + 9,	"p3 fire 2"	},
	{"P3 Button 3",		BIT_DIGITAL,	DrvJoy1 + 10,	"p3 fire 3"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
};

STDINPUTINFO(Gotcha)

static struct BurnDIPInfo GotchaDIPList[]=
{
	{0x0f, 0xff, 0xff, 0x97, NULL			},
	{0x10, 0xff, 0xff, 0xff, NULL			},

	{0   , 0xfe, 0   ,    8, "Difficulty"		},
	{0x0f, 0x01, 0x07, 0x07, "1"			},
	{0x0f, 0x01, 0x07, 0x06, "2"			},
	{0x0f, 0x01, 0x07, 0x05, "3"			},
	{0x0f, 0x01, 0x07, 0x04, "4"			},
	{0x0f, 0x01, 0x07, 0x03, "5"			},
	{0x0f, 0x01, 0x07, 0x02, "6"			},
	{0x0f, 0x01, 0x07, 0x01, "7"			},
	{0x0f, 0x01, 0x07, 0x00, "8"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x0f, 0x01, 0x08, 0x08, "Off"			},
	{0x0f, 0x01, 0x08, 0x00, "On"			},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x0f, 0x01, 0x30, 0x30, "1"			},
	{0x0f, 0x01, 0x30, 0x20, "2"			},
	{0x0f, 0x01, 0x30, 0x10, "3"			},
	{0x0f, 0x01, 0x30, 0x00, "4"			},

	{0   , 0xfe, 0   ,    4, "Coinage"		},
	{0x0f, 0x01, 0xc0, 0x00, "3 Coins 1 Credits"	},
	{0x0f, 0x01, 0xc0, 0x40, "2 Coins 1 Credits"	},
	{0x0f, 0x01, 0xc0, 0x80, "1 Coin  1 Credits"	},
	{0x0f, 0x01, 0xc0, 0xc0, "1 Coin/99 Credits"	},

	{0   , 0xfe, 0   ,    2, "Info"			},
	{0x10, 0x01, 0x01, 0x01, "Off"			},
	{0x10, 0x01, 0x01, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Explane Type"		},
	{0x10, 0x01, 0x02, 0x02, "Off"			},
	{0x10, 0x01, 0x02, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Game Selection"	},
	{0x10, 0x01, 0x04, 0x00, "Off"			},
	{0x10, 0x01, 0x04, 0x04, "On"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x10, 0x01, 0x80, 0x80, "Off"			},
	{0x10, 0x01, 0x80, 0x00, "On"			},
};

STDDIPINFO(Gotcha)

static void set_okibank(int data)
{
	*DrvSndBank = data & 1;

	int nBank = (~data & 1) << 18;

	MSM6295ROM = DrvSndROM + nBank;
}

void __fastcall gotcha_write_byte(unsigned int address, unsigned char data)
{
	switch (address)
	{
		case 0x100004:
			set_okibank(data);
		return;

		case 0x300000:
			*bankselect = data & 3;
		return;

		case 0x30000e:
			DrvGfxBank[*bankselect] = data & 0x0f;
		return;
	}
}

void __fastcall gotcha_write_word(unsigned int address, unsigned short data)
{
	switch (address)
	{
		case 0x100000:
			*soundlatch = data & 0xff;
		return;

		case 0x300002:
		case 0x300006:
			DrvScrollx[(address >> 2) & 1] = data & 0x3ff;
		return;

		case 0x300004:
		case 0x300008:
			DrvScrolly[(address >> 3) & 1] = (data + 8) & 0x1ff;
		return;
	}
}

unsigned char __fastcall gotcha_read_byte(unsigned int address)
{
	switch (address)
	{
		case 0x180002:
			return DrvInputs[1] >> 8;
	}

	return 0;
}

unsigned short __fastcall gotcha_read_word(unsigned int address)
{
	switch (address)
	{
		case 0x180000:
			return DrvInputs[0];

		case 0x180004:
			return DrvInputs[2];
	}

	return 0;
}

void __fastcall gotcha_sound_write(unsigned short address, unsigned char data)
{
	switch (address)
	{
		case 0xc000:
			BurnYM2151SelectRegister(data);
		return;

		case 0xc001:
			BurnYM2151WriteRegister(data);
		return;

		case 0xc002:
		case 0xc003:
			MSM6295Command(0, data);
		return;
	}
}

unsigned char __fastcall gotcha_sound_read(unsigned short address)
{
	switch (address)
	{
		case 0xc001:
			return BurnYM2151ReadStatus();

		case 0xc006:
			return *soundlatch;
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

	MSM6295Reset(0);
	BurnYM2151Reset();

	set_okibank(1);

	return 0;
}

static int MemIndex()
{
	unsigned char *Next; Next = AllMem;

	Drv68KROM	= Next; Next += 0x080000;
	DrvZ80ROM	= Next; Next += 0x008000;

	DrvGfxROM0	= Next; Next += 0x400000;
	DrvGfxROM1	= Next; Next += 0x400000;

	MSM6295ROM	= Next;
	DrvSndROM	= Next; Next += 0x080000;

	DrvPalette	= (unsigned int*)Next; Next += 0x0300 * sizeof(int);

	AllRam		= Next;

	Drv68KRAM	= Next; Next += 0x010000;
	DrvPalRAM	= Next; Next += 0x000800;
	DrvFgRAM	= Next; Next += 0x001000;
	DrvBgRAM	= Next; Next += 0x001000;
	DrvSprRAM	= Next; Next += 0x000800;

	DrvZ80RAM	= Next; Next += 0x000800;

	DrvScrollx	= (unsigned short*)Next; Next += 0x000002 * sizeof(short);
	DrvScrolly	= (unsigned short*)Next; Next += 0x000002 * sizeof(short);

	DrvGfxBank	= Next; Next += 0x000004;

	DrvSndBank	= Next; Next += 0x000001;

	bankselect	= Next; Next += 0x000001;
	soundlatch	= Next; Next += 0x000001;

	RamEnd		= Next;

	MemEnd		= Next;

	return 0;
}

static int DrvGfxDecode()
{
	int Plane[4]  = { 0x000000,  0x400000,  0x800000,  0xc00000 };
	int XOffs[16] = { 0x080, 0x081, 0x082, 0x083, 0x084, 0x085, 0x086, 0x087,
			  0x000, 0x001, 0x002, 0x003, 0x004, 0x005, 0x006, 0x007 };
	int YOffs[16] = { 0x000, 0x008, 0x010, 0x018, 0x020, 0x028, 0x030, 0x038,
			  0x040, 0x048, 0x050, 0x058, 0x060, 0x068, 0x070, 0x078 };

	unsigned char *tmp = (unsigned char*)malloc(0x200000);
	if (tmp == NULL) {
		return 1;
	}

	memcpy (tmp, DrvGfxROM0, 0x200000);

	GfxDecode(0x4000, 4, 16, 16, Plane, XOffs, YOffs, 0x100, tmp, DrvGfxROM0);

	memcpy (tmp, DrvGfxROM1, 0x200000);

	GfxDecode(0x4000, 4, 16, 16, Plane, XOffs, YOffs, 0x100, tmp, DrvGfxROM1);

	free (tmp);

	return 0;
}

void DrvYM2151IrqHandler(int Irq)
{
	if (Irq) {
		ZetSetIRQLine(0xff, ZET_IRQSTATUS_ACK);
	} else {
		ZetSetIRQLine(0,    ZET_IRQSTATUS_NONE);
	}
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
		if (BurnLoadRom(Drv68KROM + 0x000001,	 0, 2)) return 1;
		if (BurnLoadRom(Drv68KROM + 0x000000,	 1, 2)) return 1;

		if (BurnLoadRom(DrvZ80ROM,		 2, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM1 + 0x000000,	 3, 1)) return 1; // gfx0
		if (BurnLoadRom(DrvGfxROM1 + 0x080000,	 4, 1)) return 1; // gfx0
		if (BurnLoadRom(DrvGfxROM1 + 0x100000,	 5, 1)) return 1; // gfx0
		if (BurnLoadRom(DrvGfxROM1 + 0x180000,	 6, 1)) return 1; // gfx0
		for (int i = 0; i < 0x200000; i++) {
			int j = ((i & 0x20000) << 3) | ((i & 0x1c0000) >> 1) | ((i & 0x1ffff)^0x10);
			DrvGfxROM0[j] = DrvGfxROM1[i];
		}

		if (BurnLoadRom(DrvGfxROM1 + 0x000000,	 7, 1)) return 1; // gfx0
		if (BurnLoadRom(DrvGfxROM1 + 0x080000,	 8, 1)) return 1; // gfx0
		if (BurnLoadRom(DrvGfxROM1 + 0x100000,	 9, 1)) return 1; // gfx0
		if (BurnLoadRom(DrvGfxROM1 + 0x180000,	10, 1)) return 1; // gfx0

		if (BurnLoadRom(DrvSndROM,		4, 1)) return 1;

		DrvGfxDecode();
	}

	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM,		0x000000, 0x07ffff, SM_ROM);
	SekMapMemory(Drv68KRAM,		0x120000, 0x12ffff, SM_RAM);
	SekMapMemory(DrvPalRAM,		0x140000, 0x1407ff, SM_RAM);
	SekMapMemory(DrvSprRAM,		0x160000, 0x1607ff, SM_RAM);
	SekMapMemory(DrvFgRAM,		0x320000, 0x320fff, SM_RAM);
	SekMapMemory(DrvBgRAM,		0x322000, 0x322fff, SM_RAM);
	SekSetWriteWordHandler(0,	gotcha_write_word);
	SekSetWriteByteHandler(0,	gotcha_write_byte);
	SekSetReadWordHandler(0,	gotcha_read_word);
	SekSetReadByteHandler(0,	gotcha_read_byte);
	SekClose();

	ZetInit(1);
	ZetOpen(0);
	ZetMapArea(0x0000, 0x7fff, 0, DrvZ80ROM);
	ZetMapArea(0x0000, 0x7fff, 2, DrvZ80ROM);
	ZetMapArea(0xd000, 0xd7ff, 0, DrvZ80RAM);
	ZetMapArea(0xd000, 0xd7ff, 1, DrvZ80RAM);
	ZetMapArea(0xd000, 0xd7ff, 2, DrvZ80RAM);
	ZetSetWriteHandler(gotcha_sound_write);
	ZetSetReadHandler(gotcha_sound_read);
	ZetMemEnd();
	ZetClose();

	BurnYM2151Init(3579545, 100.0);
	BurnYM2151SetIrqHandler(&DrvYM2151IrqHandler);

	MSM6295Init(0, 1000000 / 132, 100.0, 1);

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static int DrvExit()
{
	GenericTilesExit();

	BurnYM2151Exit();
	MSM6295Exit(0);
	SekExit();
	ZetExit();

	free (AllMem);
	AllMem = NULL;

	MSM6295ROM = NULL;

	return 0;
}

static void draw_layer(unsigned char *ram, int col, int scrollx, int scrolly, int transp)
{
	unsigned short *vram = (unsigned short*)ram;

	for (int offs = 0; offs < 64 * 32; offs++)
	{
		int sx = (offs & 0x3f) << 4;
		int sy = (offs >> 6) << 4;

		sx -= scrollx;
		if (sx < -15) sx += 0x400;
		sy -= scrolly;
		if (sy < -15) sy += 0x200;

		int ofst = (offs & 0x1f) | ((offs & 0x7c0)>>1) | ((offs & 0x20) << 5);

		int attr = vram[ofst];
		int code = (attr & 0x3ff) | (DrvGfxBank[(attr >> 10) & 3] << 10);
		int color = (attr >> 12) | col;

		Render16x16Tile_Mask_Clip(pTransDraw, code, sx, sy, color, 4, transp, 0, DrvGfxROM0);
	}
}

static void draw_sprites()
{
	unsigned short *vram = (unsigned short*)DrvSprRAM;

	for (int offs = 0; offs < 0x800/2; offs += 4)
	{
		int sx     = 0x13b - ((vram[offs + 2] + 0x10) & 0x1ff);
		int sy     = vram[offs + 0] + 8;
		int code   = vram[offs + 1] & 0x3fff;
		int color  = vram[offs + 2] >> 9;
		int height = 1 << ((vram[offs + 0] & 0x0600) >> 9);
		int flipx  = vram[offs + 0] & 0x2000;
		int flipy  = vram[offs + 0] & 0x4000;

		for (int y = 0; y < height; y++)
		{
			int yy = 0x101 - ((sy + 0x10 * (height - y)) & 0x1ff);

			if (flipy) {
				if (flipx) {
					Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, code + height-1 - y, sx, yy, color, 4, 0, 0, DrvGfxROM1);
				} else {
					Render16x16Tile_Mask_FlipY_Clip(pTransDraw, code + height-1 - y, sx, yy, color, 4, 0, 0, DrvGfxROM1);
				}
			} else {
				if (flipx) {
					Render16x16Tile_Mask_FlipX_Clip(pTransDraw, code + y, sx, yy, color, 4, 0, 0, DrvGfxROM1);
				} else {
					Render16x16Tile_Mask_Clip(pTransDraw, code + y, sx, yy, color, 4, 0, 0, DrvGfxROM1);
				}
			}
		}
	}
}

static int DrvDraw()
{
	if (DrvRecalc) {
		unsigned char r,g,b;
		unsigned short *pal = (unsigned short*)DrvPalRAM;

		for (int i = 0; i < 0x600/2; i++) {
			int d = pal[i];

			r = (d >> 10) & 0x1f;
			g = (d >>  5) & 0x1f;
			b = (d >>  0) & 0x1f;

			r = (r << 3) | (r >> 2);
			g = (g << 3) | (g >> 2);
			b = (b << 3) | (b >> 2);

			DrvPalette[i] = BurnHighCol(r, g, b, 0);
		}
	}

	draw_layer(DrvBgRAM, 32, DrvScrollx[1]+5, DrvScrolly[1], ~0);
	draw_layer(DrvFgRAM, 16, DrvScrollx[0]+1, DrvScrolly[0],  0);

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
		memset (DrvInputs, 0xff, 2 * sizeof(short));
		for (int i = 0; i < 16; i++) {
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
		}

		DrvInputs[2] = (DrvDips[0]) | (DrvDips[1] << 8);
	}

	int nSegment;
	int nInterleave = 10;
	int nSoundBufferPos = 0;
	int nCyclesTotal[2] = { 14318180 / 60, 6000000 / 60 };
	int nCyclesDone[2] = { 0, 0 };

	SekOpen(0);
	ZetOpen(0);

	SekNewFrame();
	ZetNewFrame();

	for (int i = 0; i < nInterleave; i++)
	{
		nSegment = (nCyclesTotal[0] - nCyclesDone[0]) / (nInterleave - i);
		nCyclesDone[0] += SekRun(nSegment);

		nSegment = (nCyclesTotal[1] - nCyclesDone[1]) / (nInterleave - i);
		nCyclesDone[1] += ZetRun(nSegment);

		if (pBurnSoundOut) {
			nSegment = nBurnSoundLen / nInterleave;

			BurnYM2151Render(pBurnSoundOut + nSoundBufferPos, nSegment);
			MSM6295Render(0, pBurnSoundOut + nSoundBufferPos, nSegment);

			nSoundBufferPos += nSegment << 1;
		}
	}

	SekSetIRQLine(6, SEK_IRQSTATUS_AUTO);

	if (pBurnSoundOut) {
		nSegment = nBurnSoundLen - nSoundBufferPos;
		if (nSegment > 0) {
			BurnYM2151Render(pBurnSoundOut + nSoundBufferPos, nSegment);
			MSM6295Render(0, pBurnSoundOut + nSoundBufferPos, nSegment);
		}
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

		BurnYM2151Scan(nAction);
		MSM6295Scan(0, nAction);

		set_okibank(*DrvSndBank);
	}

	return 0;
}


// Got-cha Mini Game Festival

static struct BurnRomInfo gotchaRomDesc[] = {
	{ "gotcha.u3",		0x40000, 0x5e5d52e0, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "gotcha.u2",		0x40000, 0x3aa8eaff, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "gotcha_u.z02",	0x10000, 0xf4f6e16b, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 Code

	{ "gotcha-u.42a",	0x80000, 0x4ea822f0, 3 | BRF_GRA },           //  3 Tiles
	{ "gotcha-u.42b",	0x80000, 0x6bb529ac, 3 | BRF_GRA },           //  4
	{ "gotcha-u.41a",	0x80000, 0x49299b7b, 3 | BRF_GRA },           //  5
	{ "gotcha-u.41b",	0x80000, 0xc093f04e, 3 | BRF_GRA },           //  6

	{ "gotcha.u56",		0x80000, 0x85f6a062, 4 | BRF_GRA },           //  7 Sprites
	{ "gotcha.u55",		0x80000, 0x426b4e48, 4 | BRF_GRA },           //  8
	{ "gotcha.u54",		0x80000, 0x903e05a4, 4 | BRF_GRA },           //  9
	{ "gotcha.u53",		0x80000, 0x3c24d51e, 4 | BRF_GRA },           // 10

	{ "gotcha-u.z11",	0x80000, 0x6111c6ae, 5 | BRF_SND },           // 11 Samples
};

STD_ROM_PICK(gotcha)
STD_ROM_FN(gotcha)

struct BurnDriver BurnDrvGotcha = {
	"gotcha", NULL, NULL, NULL, "1997",
	"Got-cha Mini Game Festival\0", NULL, "Dongsung", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 3, HARDWARE_MISC_POST90S, GBF_MINIGAMES, 0,
	NULL, gotchaRomInfo, gotchaRomName, NULL, NULL, GotchaInputInfo, GotchaDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x300,
	320, 240, 4, 3
};


// Pasha Pasha Champ Mini Game Festival

static struct BurnRomInfo ppchampRomDesc[] = {
	{ "u3",		0x40000, 0xf56c0fc2, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "u2",		0x40000, 0xa941ffdc, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "uz02",	0x10000, 0xf4f6e16b, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 Code

	{ "u42a",	0x80000, 0xf0b521d1, 3 | BRF_GRA },           //  3 Tiles
	{ "u42b",	0x80000, 0x1107918e, 3 | BRF_GRA },           //  4
	{ "u41a",	0x80000, 0x3f567d33, 3 | BRF_GRA },           //  5
	{ "u41b",	0x80000, 0x18a3497e, 3 | BRF_GRA },           //  6

	{ "u56",	0x80000, 0x160e46b3, 4 | BRF_GRA },           //  7 Sprites
	{ "u55",	0x80000, 0x7351b61c, 4 | BRF_GRA },           //  8
	{ "u54",	0x80000, 0xa3d8c5ef, 4 | BRF_GRA },           //  9
	{ "u53",	0x80000, 0x10ca65c4, 4 | BRF_GRA },           // 10

	{ "uz11",	0x80000, 0x3d96274c, 5 | BRF_SND },           // 11 Samples
};

STD_ROM_PICK(ppchamp)
STD_ROM_FN(ppchamp)

struct BurnDriver BurnDrvPpchamp = {
	"ppchamp", "gotcha", NULL, NULL, "1997",
	"Pasha Pasha Champ Mini Game Festival\0", NULL, "Dongsung", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 3, HARDWARE_MISC_POST90S, GBF_MINIGAMES, 0,
	NULL, ppchampRomInfo, ppchampRomName, NULL, NULL, GotchaInputInfo, GotchaDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x300,
	320, 240, 4, 3
};
