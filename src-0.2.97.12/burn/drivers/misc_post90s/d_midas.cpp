// FB Alpha Andamiro "Midas" hardware driver module
// Based on MAME driver by Luca Elia

#include "tiles_generic.h"
#include "eeprom.h"
#include "ymz280b.h"

static unsigned char *AllMem;
static unsigned char *MemEnd;
static unsigned char *AllRam;
static unsigned char *RamEnd;
static unsigned char *Drv68KROM;
static unsigned char *DrvSprROM;
static unsigned char *DrvTxtROM;
static unsigned char *Drv68KRAM0;
static unsigned char *Drv68KRAM1;
static unsigned char *DrvPalRAM;
static unsigned char *DrvGfxRAM;
static unsigned int  *DrvPalette;
static unsigned char *DrvSprTransTab;
static unsigned char *DrvTxtTransTab;
static unsigned short *DrvGfxRegs;

static unsigned char DrvJoy1[16];
static unsigned char DrvJoy2[16];
static unsigned char DrvJoy3[16];
static unsigned char DrvJoy4[16];
static unsigned char DrvJoy5[16];
static unsigned char DrvJoy6[16];
static unsigned char DrvDips[1];
static unsigned char DrvReset;
static unsigned short DrvInputs[6];

static int DrvRecalc0 = 0;

static short zoom_table[16][16];

static struct BurnInputInfo LivequizInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy3 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy2 + 8,	"p1 start"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy6 + 10,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy6 + 11,	"p1 fire 2"	},
	{"P1 Button 3",		BIT_DIGITAL,	DrvJoy6 + 8,	"p1 fire 3"	},
	{"P1 Button 4",		BIT_DIGITAL,	DrvJoy6 + 9,	"p1 fire 4"	},

	{"P2 Start",		BIT_DIGITAL,	DrvJoy2 + 10,	"p2 start"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy1 + 10,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy1 + 11,	"p2 fire 2"	},
	{"P2 Button 3",		BIT_DIGITAL,	DrvJoy1 + 8,	"p2 fire 3"	},
	{"P2 Button 4",		BIT_DIGITAL,	DrvJoy1 + 9,	"p2 fire 4"	},

	{"P3 Start",		BIT_DIGITAL,	DrvJoy5 + 0,	"p3 start"	},
	{"P3 Button 1",		BIT_DIGITAL,	DrvJoy4 + 0,	"p3 fire 1"	},
	{"P3 Button 2",		BIT_DIGITAL,	DrvJoy4 + 1,	"p3 fire 2"	},
	{"P3 Button 3",		BIT_DIGITAL,	DrvJoy4 + 2,	"p3 fire 3"	},
	{"P3 Button 4",		BIT_DIGITAL,	DrvJoy4 + 3,	"p3 fire 4"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
};

STDINPUTINFO(Livequiz)

static struct BurnDIPInfo LivequizDIPList[] =
{
	{0x11, 0xff, 0xff, 0xff, NULL		},

	{0   , 0xfe, 0   ,    2, "Freeze"	},
	{0x11, 0x01, 0x80, 0x80, "Off"		},
	{0x11, 0x01, 0x80, 0x00, "On"		},
};

STDDIPINFO(Livequiz)

inline static void palette_write(int offs)
{
	DrvPalette[offs/4] = BurnHighCol(DrvPalRAM[offs + 0], DrvPalRAM[offs + 3], DrvPalRAM[offs + 2], 0);
}

void __fastcall midas_write_byte(unsigned int address, unsigned char data)
{
	if (address >= 0xa00000 && address <= 0xa3ffff) {
		DrvPalRAM[(address & 0x3ffff) ^ 1] = data;
		palette_write(address & 0x3fffc);
		return;
	}

	switch (address)
	{
		case 0x9a0001:
			EEPROMWrite(data & 0x02, data & 0x01, data & 0x04);
		return;

		case 0xb80009:
			YMZ280BSelectRegister(data);
		return;

		case 0xb8000b:
			YMZ280BWriteRegister(data);
		return;
	}
}

void __fastcall midas_write_word(unsigned int address, unsigned short data)
{
	if (address >= 0xa00000 && address <= 0xa3ffff) {
		*((unsigned short*)(DrvPalRAM + (address & 0x3fffe))) = data;
		palette_write(address & 0x3fffc);
		return;
	}

	switch (address)
	{
		case 0x9c0000:
			DrvGfxRegs[0] = data;
		return;

		case 0x9c0002:
		{
			DrvGfxRegs[1] = data;
			*((unsigned short*)(DrvGfxRAM + (DrvGfxRegs[0] << 1))) = data;
			DrvGfxRegs[0] += DrvGfxRegs[2];
		}
		return;

		case 0x9c0004:
			DrvGfxRegs[2] = data;
		return;
	}
}

unsigned char __fastcall midas_read_byte(unsigned int address)
{
	switch (address)
	{
		case 0x900001:
			return DrvInputs[5];

		case 0x920001:
			return DrvInputs[2] | (EEPROMRead() ? 8 : 0);

		case 0x940000:
			return (DrvInputs[0] >> 8);

		case 0x980000:
			return (DrvInputs[1] >> 8);

		case 0xb8000b:
			return YMZ280BReadStatus();

		case 0xba0001:
			return DrvInputs[4];

		case 0xbc0001:
			return DrvInputs[3];
	}

	return 0;
}

unsigned short __fastcall midas_read_word(unsigned int address)
{
	switch (address)
	{
		case 0x900000:
			return DrvInputs[5];

		case 0x980000:
			return DrvInputs[1];

		case 0xb00000:
		case 0xb20000:
		case 0xb40000:
		case 0xb60000:
			return 0xffff;

		case 0xbc0000:
			return DrvInputs[3];
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

	EEPROMReset();

	YMZ280BReset();

	return 0;
}

static int MemIndex()
{
	unsigned char *Next; Next = AllMem;

	Drv68KROM	= Next; Next += 0x200000;

	DrvTxtROM	= Next; Next += 0x040000;
	DrvSprROM	= Next; Next += 0x800000;

	DrvTxtTransTab	= Next; Next += 0x040000 / 0x040;
	DrvSprTransTab	= Next; Next += 0x800000 / 0x100;

	YMZ280BROM	= Next; Next += 0x200000;

	DrvPalette	= (unsigned int*)Next; Next += 0x10000 * sizeof(int);

	AllRam		= Next;

	Drv68KRAM0	= Next; Next += 0x020000;
	Drv68KRAM1	= Next; Next += 0x040000;
	DrvPalRAM	= Next; Next += 0x080000;
	DrvGfxRAM	= Next; Next += 0x020000;

	DrvGfxRegs	= (unsigned short*)Next; Next += 3 * sizeof(short);

	RamEnd		= Next;

	MemEnd		= Next;

	return 0;
}

static void DrvInitZoomTable()
{
	zoom_table[0][0] = -1;

	for (int x = 1; x < 16; x++) {
		for (int y = 0; y < 16; y++) {
			float t = ((16.0000-1.0000) / x) * y;
			zoom_table[x][y] = (t >= 16) ? -1 : (int)t;
		}
	}
}

static void DrvFillTransTabs(unsigned char *tab, unsigned char *gfx, int gfxlen, int tilesize)
{
	for (int i = 0; i < gfxlen; i+=tilesize)
	{
		tab[i/tilesize] = 0;

		int count = 0;

		for (int j = 0; j < tilesize; j++) {
			if (gfx[i + j]) {
				tab[i/tilesize] = 1;
				count++;
			}
		}

		if (count >= (tilesize-1)) {
			tab[i/tilesize] |= 2;
		}
	}
}

static int DrvGfxDecode()
{
	int Plane0[8]  = { 0x3000008, 0x3000000, 0x1000008, 0x1000000, 0x2000008, 0x2000000, 0x0000008, 0x0000000 };
	int XOffs0[16] = { 0x107, 0x106, 0x105, 0x104, 0x103, 0x102, 0x101, 0x100, 0x007, 0x006, 0x005, 0x004, 0x003, 0x002, 0x001, 0x000 };
	int YOffs0[16] = { 0x000, 0x010, 0x020, 0x030, 0x040, 0x050, 0x060, 0x070, 0x080, 0x090, 0x0a0, 0x0b0, 0x0c0, 0x0d0, 0x0e0, 0x0f0 };

	int Plane1[8]  = { 0x008, 0x009, 0x00a, 0x00b, 0x000, 0x001, 0x002, 0x003 };
	int XOffs1[8]  = { 0x104, 0x100, 0x184, 0x180, 0x004, 0x000, 0x084, 0x080 };
	int YOffs1[8]  = { 0x000, 0x010, 0x020, 0x030, 0x040, 0x050, 0x060, 0x070 };

	unsigned char *tmp = (unsigned char*)malloc(0x800000);
	if (tmp == NULL) {
		return 1;
	}

	memcpy (tmp, DrvSprROM, 0x800000);

	GfxDecode(0x8000, 8, 16, 16, Plane0, XOffs0, YOffs0, 0x200, tmp, DrvSprROM);

	memcpy (tmp, DrvTxtROM, 0x040000);

	GfxDecode(0x1000, 8,  8,  8, Plane1, XOffs1, YOffs1, 0x200, tmp, DrvTxtROM);

	DrvFillTransTabs(DrvSprTransTab, DrvSprROM, 0x800000, 0x100);
	DrvFillTransTabs(DrvTxtTransTab, DrvTxtROM, 0x040000, 0x040);

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
		if (BurnLoadRom(Drv68KROM,		0, 1)) return 1;

		if (BurnLoadRom(DrvTxtROM,      	7, 1)) return 1;

		if (BurnLoadRom(DrvSprROM + 0x000000,	3, 1)) return 1;
		if (BurnLoadRom(DrvSprROM + 0x200000,	4, 1)) return 1;
		if (BurnLoadRom(DrvSprROM + 0x400000,	5, 1)) return 1;
		if (BurnLoadRom(DrvSprROM + 0x600000,	6, 1)) return 1;

		if (BurnLoadRom(YMZ280BROM,	        8, 1)) return 1;

		DrvGfxDecode();
	}

	*((unsigned short*)(Drv68KROM + 0x13345a)) = 0x4e75; // patch out protection

	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM,			0x000000, 0x1fffff, SM_ROM);
	SekMapMemory(DrvPalRAM,			0xa00000, 0xa3ffff, SM_ROM);
	SekMapMemory(DrvPalRAM + 0x3ffff,	0xa40000, 0xa7ffff, SM_RAM);
	SekMapMemory(Drv68KRAM0,		0xd00000, 0xd1ffff, SM_RAM);
	SekMapMemory(Drv68KRAM1,		0xe00000, 0xe3ffff, SM_RAM);
	SekMapMemory(DrvGfxRAM,			0xf90000, 0xfaffff, SM_RAM);
	SekSetWriteByteHandler(0,		midas_write_byte);
	SekSetWriteWordHandler(0,		midas_write_word);
	SekSetReadByteHandler(0,		midas_read_byte);
	SekSetReadWordHandler(0,		midas_read_word);
	SekClose();

	YMZ280BInit(16934400, NULL, 3);

	EEPROMInit(&eeprom_interface_93C46);

	DrvInitZoomTable();

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static int DrvExit()
{
	GenericTilesExit();

	YMZ280BExit();
	YMZ280BROM = NULL;

	EEPROMExit();

	SekExit();

	free (AllMem);
	AllMem = NULL;

	DrvRecalc0 = 0;

	return 0;
}

static void draw_16x16_zoom(int code, int color, int sx, int sy, int xz, int yz, int fx, int fy)
{
	if (yz == 1 || xz == 1) return;

	fx = (fx * 0x0f);
	fy = (fy >> 1) * 0x0f;

	unsigned char *src = DrvSprROM + (code << 8);

	short *xm = zoom_table[xz-1];
	short *ym = zoom_table[yz-1];

	for (int y = 0; y < 16; y++)
	{
		int yy = sy + y;

		if (ym[y ^ fy] == -1 || yy < 0 || yy >= nScreenHeight) continue;

		int yyz = (ym[y ^ fy] << 4);

		for (int x = 0; x < 16; x++)
		{
			short xxz = xm[x ^ fx];
			if (xxz == -1) continue;

			int xx = sx + x;

			int pxl = src[yyz|xxz];

			if (!pxl || xx < 0 || xx >= nScreenWidth || yy < 0 || yy >= nScreenHeight) continue;

			pTransDraw[(yy * nScreenWidth) + xx] = pxl | color;
		}
	}
}

static void draw_sprites()
{
	unsigned short *codes =	(unsigned short*)DrvGfxRAM;
	unsigned short *s     =	codes + 0x8000;

	int sx_old = 0, sy_old = 0, ynum_old = 0, xzoom_old = 0;
	int xdim, ydim, xscale, yscale, ynum;

	for (int i = 0; i < 0x180; i++, s++, codes+=0x40)
	{
		int zoom	= s[0x000];
		int sy		= s[0x200];
		int sx		= s[0x400];

		int xzoom	= ((zoom >> 8) & 0x0f) + 1;
		int yzoom	= (zoom & 0x7f) + 1;

		if (sy & 0x40) {
			ynum	= ynum_old;
			sx	= sx_old + xzoom_old;
			sy	= sy_old;
		} else {
			ynum	= sy & 0x3f;
			sx	= (sx >> 7);
			sy	= 0x200 - (sy >> 7);

			if (ynum > 0x20) ynum = 0x20;
		}

		if (sx >= 0x1f0) sx -= 0x200;

		ynum_old  = ynum;
		sx_old    = sx;
		sy_old    = sy;
		xzoom_old = xzoom;

		sy	<<= 16;

		xdim	= xzoom << 16;
		ydim	= yzoom << 13;

		xscale	= xzoom;
		yscale	= yzoom >> 3;

		for (int y = 0; y < ynum; y++)
		{
			int code = codes[y * 2];
			if (code & 0x8000) continue; // only 8mb of sprites...

			int attr = codes[y * 2 + 1];

			int yy = (((sy + y * ydim) >> 16) & 0x1ff) - 16;

			if (sx >= nScreenWidth || yy >= nScreenHeight || !DrvSprTransTab[code]) continue;

			if (yscale != 16 || xscale != 16) {
				draw_16x16_zoom(code, attr & 0xff00, sx, yy, xscale, yscale, attr & 1, attr & 2);
				continue;
			}

			if (DrvSprTransTab[code] & 2) {
				if (sx < 0 || sx > (nScreenWidth-16) || yy < 0 || yy > (nScreenHeight-16)) {
					if (attr & 2) {
						if (attr & 1) {
							Render16x16Tile_FlipXY_Clip(pTransDraw, code, sx, yy, attr >> 8, 8, 0, DrvSprROM);
						} else {
							Render16x16Tile_FlipY_Clip(pTransDraw, code, sx, yy, attr >> 8, 8, 0, DrvSprROM);
						}
					} else {
						if (attr & 1) {
							Render16x16Tile_FlipX_Clip(pTransDraw, code, sx, yy, attr >> 8, 8, 0, DrvSprROM);
						} else {
							Render16x16Tile_Clip(pTransDraw, code, sx, yy, attr >> 8, 8, 0, DrvSprROM);
						}
					}
				} else {
					if (attr & 2) {
						if (attr & 1) {
							Render16x16Tile_FlipXY(pTransDraw, code, sx, yy, attr >> 8, 8, 0, DrvSprROM);
						} else {
							Render16x16Tile_FlipY(pTransDraw, code, sx, yy, attr >> 8, 8, 0, DrvSprROM);
						}
					} else {
						if (attr & 1) {
							Render16x16Tile_FlipX(pTransDraw, code, sx, yy, attr >> 8, 8, 0, DrvSprROM);
						} else {
							Render16x16Tile(pTransDraw, code, sx, yy, attr >> 8, 8, 0, DrvSprROM);
						}
					}
				}
			} else {
				if (sx < 0 || sx > (nScreenWidth-16) || yy < 0 || yy > (nScreenHeight-16)) {
					if (attr & 2) {
						if (attr & 1) {
							Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, code, sx, yy, attr >> 8, 8, 0, 0, DrvSprROM);
						} else {
							Render16x16Tile_Mask_FlipY_Clip(pTransDraw, code, sx, yy, attr >> 8, 8, 0, 0, DrvSprROM);
						}
					} else {
						if (attr & 1) {
							Render16x16Tile_Mask_FlipX_Clip(pTransDraw, code, sx, yy, attr >> 8, 8, 0, 0, DrvSprROM);
						} else {
							Render16x16Tile_Mask_Clip(pTransDraw, code, sx, yy, attr >> 8, 8, 0, 0, DrvSprROM);
						}
					}
				} else {
					if (attr & 2) {
						if (attr & 1) {
							Render16x16Tile_Mask_FlipXY(pTransDraw, code, sx, yy, attr >> 8, 8, 0, 0, DrvSprROM);
						} else {
							Render16x16Tile_Mask_FlipY(pTransDraw, code, sx, yy, attr >> 8, 8, 0, 0, DrvSprROM);
						}
					} else {
						if (attr & 1) {
							Render16x16Tile_Mask_FlipX(pTransDraw, code, sx, yy, attr >> 8, 8, 0, 0, DrvSprROM);
						} else {
							Render16x16Tile_Mask(pTransDraw, code, sx, yy, attr >> 8, 8, 0, 0, DrvSprROM);
						}
					}
				}
			}
		}
	}
}

static void draw_layer()
{
	unsigned short *vram = (unsigned short*)(DrvGfxRAM + 0xe000);

	for (int offs = 0; offs < 0x28 * 0x20; offs++)
	{
		int sx = (offs >> 5) << 3;
		int sy = ((offs & 0x1f) << 3) - 16;

		int code = vram[offs] & 0xfff;
		int color = vram[offs] >> 12;

		if (sy < 0 || sy >= nScreenHeight || !DrvTxtTransTab[code]) continue;

		if (DrvTxtTransTab[code] & 2) {
			Render8x8Tile(pTransDraw, code, sx, sy, color, 8, 0, DrvTxtROM); 
		} else {
			Render8x8Tile_Mask(pTransDraw, code, sx, sy, color, 8, 0, 0, DrvTxtROM);
		}
	}
}

static int DrvDraw()
{
	if (DrvRecalc0 != nBurnBpp) {
		for (int i = 0; i < 0x40000; i+=4) {
			palette_write(i);
		}

		DrvRecalc0 = nBurnBpp;
	}

	for (int i = 0; i < nScreenWidth * nScreenHeight; i++)
		pTransDraw[i] = 0xfff;

	draw_sprites();

	draw_layer();

	BurnTransferCopy(DrvPalette);

	return 0;
}

static int DrvFrame()
{
	if (DrvReset) {
		DrvDoReset();
	}

	{
		memset (DrvInputs, 0xff, 12);

		for (int i = 0; i < 16; i++)
		{
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
			DrvInputs[2] ^= (DrvJoy3[i] & 1) << i;
			DrvInputs[3] ^= (DrvJoy4[i] & 1) << i;
			DrvInputs[4] ^= (DrvJoy5[i] & 1) << i;
			DrvInputs[5] ^= (DrvJoy6[i] & 1) << i;
		}

		DrvInputs[2] &= ~0x0008;
		DrvInputs[5]  = (DrvInputs[5] & 0xff00) | DrvDips[0];
	}

	SekOpen(0);
	SekRun(12000000 / 60);
	SekSetIRQLine(1, SEK_IRQSTATUS_AUTO);
	SekClose();

	if (pBurnSoundOut) {
		YMZ280BRender(pBurnSoundOut, nBurnSoundLen);
	}

	if (pBurnDraw) {
		DrvDraw();
	}

	return 0;
}

static int DrvScan(int nAction, int *pnMin)
{
	struct BurnArea ba;

	if (pnMin) {
		*pnMin = 0x029698;
	}

	EEPROMScan(nAction, pnMin);

	if (nAction & ACB_VOLATILE) {

		memset(&ba, 0, sizeof(ba));
    		ba.Data		= AllRam;
		ba.nLen		= RamEnd - AllRam;
		ba.szName	= "All RAM";
		BurnAcb(&ba);

		SekScan(nAction);

		YMZ280BScan();
	}

	return 0;
}


// Live Quiz Show

static struct BurnRomInfo livequizRomDesc[] = {
	{ "flash.u1",		0x200000, 0x8ec44493, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code

	{ "main_pic12c508a.u27",0x000400, 0xa84f0a7e, 2 | BRF_PRG | BRF_OPT }, //  1 Mcu Code (not used)
	{ "sub_pic12c508a.u4",	0x000400, 0xe52ebdc4, 2 | BRF_PRG | BRF_OPT }, //  2

	{ "flash.u15",		0x200000, 0xd6eb56f1, 3 | BRF_GRA },           //  3 Sprites
	{ "flash.u16",		0x200000, 0x4c9fd873, 3 | BRF_GRA },           //  4
	{ "flash.u19",		0x200000, 0xdaa81532, 3 | BRF_GRA },           //  5
	{ "flash.u20",		0x200000, 0xb540a8c7, 3 | BRF_GRA },           //  6

	{ "27c4096.u23",	0x080000, 0x25121de8, 4 | BRF_GRA },           //  7 Characters

	{ "flash.u5",		0x200000, 0xdc062792, 5 | BRF_SND },           //  8 YMZ280b Samples
};

STD_ROM_PICK(livequiz)
STD_ROM_FN(livequiz)

struct BurnDriver BurnDrvLivequiz = {
	"livequiz", NULL, NULL, NULL, "1999",
	"Live Quiz Show\0", NULL, "Andamiro Entertainment Co. Ltd.", "Andamiro Midas",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 3, HARDWARE_MISC_POST90S, GBF_QUIZ, 0,
	NULL, livequizRomInfo, livequizRomName, NULL, NULL, LivequizInputInfo, LivequizDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, NULL, 0x10000,
	320, 224, 4, 3
};
