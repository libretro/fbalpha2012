// FB Alpha Ginga NinkyouDen driver module
// Based on MAME driver by Luca Elia and Takahiro Nogi

// To do: missing Y8950 sound

#include "tiles_generic.h"
#include "m6809_intf.h"
#include "burn_y8950.h"
#include "driver.h"
extern "C" {
#include "ay8910.h"
}

static unsigned char *AllMem;
static unsigned char *MemEnd;
static unsigned char *AllRam;
static unsigned char *RamEnd;
static unsigned char *Drv68KROM;
static unsigned char *DrvM6809ROM;
static unsigned char *DrvGfxROM0;
static unsigned char *DrvGfxROM1;
static unsigned char *DrvGfxROM2;
static unsigned char *DrvGfxROM3;
static unsigned char *DrvGfxROM4;
static unsigned char *DrvSndROM;
static unsigned char *Drv68KRAM;
static unsigned char *DrvM6809RAM;
static unsigned char *DrvPalRAM;
static unsigned char *DrvFgRAM;
static unsigned char *DrvTxtRAM;
static unsigned char *DrvSprRAM;

static unsigned int  *DrvPalette;
static unsigned char  DrvRecalc;

static short *pAY8910Buffer[3];

static unsigned char *soundlatch;
static unsigned char *flipscreen;
static unsigned short *layer_control;
static unsigned short *scroll;

static unsigned char MC6840_idx0;
static unsigned char MC6840_idx1;
static unsigned char MC6840_reg0;
static unsigned char MC6840_reg1;
static unsigned char MC6840_tempo;
static unsigned char MC6840_tempo2;
static unsigned char MC6840_ctr;
static unsigned char MC6840_flag;

static unsigned char DrvJoy1[16];
static unsigned short DrvInputs[1];
static unsigned char DrvDips[2];
static unsigned char DrvReset;

static struct BurnInputInfo GinganinInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy1 + 12,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 14,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy1 + 13,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy1 + 15,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy1 + 6,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy1 + 7,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy1 + 8,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy1 + 9,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy1 + 10,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy1 + 11,	"p2 fire 2"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
};

STDINPUTINFO(Ginganin)

static struct BurnDIPInfo GinganinDIPList[]=
{
	{0x11, 0xff, 0xff, 0xff, NULL				},
	{0x12, 0xff, 0xff, 0xf7, NULL				},

	{0   , 0xfe, 0   ,    8, "Coin A"			},
	{0x11, 0x01, 0x07, 0x00, "5 Coins 1 Credits"		},
	{0x11, 0x01, 0x07, 0x04, "4 Coins 1 Credits"		},
	{0x11, 0x01, 0x07, 0x02, "3 Coins 1 Credits"		},
	{0x11, 0x01, 0x07, 0x06, "2 Coins 1 Credits"		},
	{0x11, 0x01, 0x07, 0x07, "1 Coin  1 Credits"		},
	{0x11, 0x01, 0x07, 0x03, "1 Coin  2 Credits"		},
	{0x11, 0x01, 0x07, 0x05, "1 Coin  3 Credits"		},
	{0x11, 0x01, 0x07, 0x01, "1 Coin  4 Credits"		},

	{0   , 0xfe, 0   ,    8, "Coin B"			},
	{0x11, 0x01, 0x38, 0x00, "5 Coins 1 Credits"		},
	{0x11, 0x01, 0x38, 0x20, "4 Coins 1 Credits"		},
	{0x11, 0x01, 0x38, 0x10, "3 Coins 1 Credits"		},
	{0x11, 0x01, 0x38, 0x30, "2 Coins 1 Credits"		},
	{0x11, 0x01, 0x38, 0x38, "1 Coin  1 Credits"		},
	{0x11, 0x01, 0x38, 0x18, "1 Coin  2 Credits"		},
	{0x11, 0x01, 0x38, 0x28, "1 Coin  3 Credits"		},
	{0x11, 0x01, 0x38, 0x08, "1 Coin  4 Credits"		},

	{0   , 0xfe, 0   ,    2, "Infinite Lives"		},
	{0x11, 0x01, 0x40, 0x40, "Off"				},
	{0x11, 0x01, 0x40, 0x00, "On"				},

	{0   , 0xfe, 0   ,    2, "Free Play & Invulnerability"	},
	{0x11, 0x01, 0x80, 0x80, "Off"				},
	{0x11, 0x01, 0x80, 0x00, "On"				},

	{0   , 0xfe, 0   ,    4, "Lives"			},
	{0x12, 0x01, 0x03, 0x00, "2"				},
	{0x12, 0x01, 0x03, 0x03, "3"				},
	{0x12, 0x01, 0x03, 0x01, "4"				},
	{0x12, 0x01, 0x03, 0x02, "5"				},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"			},
	{0x12, 0x01, 0x04, 0x00, "Off"				},
	{0x12, 0x01, 0x04, 0x04, "On"				},

	{0   , 0xfe, 0   ,    2, "Cabinet"			},
	{0x12, 0x01, 0x08, 0x00, "Upright"			},
	{0x12, 0x01, 0x08, 0x08, "Cocktail"			},

	{0   , 0xfe, 0   ,    2, "Flip Screen"			},
	{0x12, 0x01, 0x40, 0x40, "Off"				},
	{0x12, 0x01, 0x40, 0x00, "On"				},

	{0   , 0xfe, 0   ,    2, "Freeze (Cheat)"		},
	{0x12, 0x01, 0x80, 0x80, "Off"				},
	{0x12, 0x01, 0x80, 0x00, "On"				},
};

STDDIPINFO(Ginganin)

void __fastcall ginganin_write_word(unsigned int address, unsigned short data)
{
	if (address < 0x20000) return;

	switch (address)
	{
		case 0x060000: // scrolly fg
		case 0x060002: // scrollx fg
		case 0x060004: // scrolly bg
		case 0x060006: // scrollx bg
			scroll[(address & 6) / 2] = data;
		return;

		case 0x060008:
			*layer_control = data;
		return;

		case 0x06000c:
			*flipscreen = ~data & 1;
		return;

		case 0x06000e:
			*soundlatch = data & 0xff;
			M6809SetIRQ(0x20, M6809_IRQSTATUS_AUTO); // nmi
		return;
	}
}

unsigned short __fastcall ginganin_read_word(unsigned int address)
{
	switch (address)
	{
		case 0x070000:
			return DrvInputs[0];

		case 0x070002:
			return (DrvDips[0] | (DrvDips[1] << 8));
	}

	return 0;
}

void __fastcall ginganin_write_byte(unsigned int /*address*/, unsigned char /*data*/)
{
	return;
}

unsigned char __fastcall ginganin_read_byte(unsigned int /*address*/)
{
	return 0;
}

void ginganin_sound_write(unsigned short address, unsigned char data)
{
	switch (address)
	{
		case 0x0800:
		{
			MC6840_idx0 = data;
			MC6840_flag = MC6840_idx0 / 0x80;

			if (MC6840_flag && MC6840_reg0 != MC6840_tempo && MC6840_reg0 != 0) {
				MC6840_tempo = MC6840_reg0;
			}
		}			
		return;

		case 0x0801:
			MC6840_idx1 = data;
		return;

		case 0x0802:
			MC6840_reg0 = data;
		return;

		case 0x0803:
			MC6840_reg1 = data;
		return;

		case 0x2000:
		case 0x2001: // y8950
			BurnY8950Write(address & 1, data);
		return;

		case 0x2800:
		case 0x2801:
			AY8910Write(0, address & 1, data);
		return;
	}
}

unsigned char ginganin_sound_read(unsigned short address)
{
	if (address == 0x1800) {
		return *soundlatch;
	}
	
	bprintf(PRINT_NORMAL, _T("read %x\n"), address);

	return 0;
}

static int DrvSynchroniseStream(int nSoundRate)
{
	return (long long)M6809TotalCycles() * nSoundRate / 1000000;
}

static void DrvGfxDecode(unsigned char *src, int len, int size)
{
	int Planes[4] = { 0x000, 0x001, 0x002, 0x003 };
	int XOffs[16] = { 0x000, 0x004, 0x008, 0x00c, 0x010, 0x014, 0x018, 0x01c,
			  0x200, 0x204, 0x208, 0x20c, 0x210, 0x214, 0x218, 0x21c };
	int YOffs[16] = { 0x000, 0x020, 0x040, 0x060, 0x080, 0x0a0, 0x0c0, 0x0e0,
			  0x100, 0x120, 0x140, 0x160, 0x180, 0x1a0, 0x1c0, 0x1e0 };

	unsigned char *tmp = (unsigned char*)malloc(len);
	if (tmp == NULL) {
		return;
	}

	memcpy (tmp, src, len);

	GfxDecode((len * 2) / (size * size), 4, size, size, Planes, XOffs, YOffs, (size * size * 4), tmp, src);

	free (tmp);
}

static int DrvDoReset()
{
	memset (AllRam, 0, RamEnd - AllRam);

	SekOpen(0);
	SekReset();
	SekClose();

	M6809Open(0);
	M6809Reset();
	M6809Close();

	AY8910Reset(0);
	BurnY8950Reset();
	
	MC6840_idx0 = 0;
	MC6840_idx1 = 0;
	MC6840_reg0 = 0;
	MC6840_reg1 = 0;
	MC6840_flag = 0;
	MC6840_tempo = 0;
	MC6840_tempo2 = 0;
	MC6840_ctr = 0;

	return 0;
}

static int MemIndex()
{
	unsigned char *Next; Next = AllMem;

	Drv68KROM		= Next; Next += 0x020000;
	DrvM6809ROM		= Next; Next += 0x010000;

	DrvGfxROM0		= Next; Next += 0x040000;
	DrvGfxROM1		= Next; Next += 0x040000;
	DrvGfxROM2		= Next; Next += 0x008000;
	DrvGfxROM3		= Next; Next += 0x100000;
	DrvGfxROM4		= Next; Next += 0x008000;

	DrvSndROM		= Next; Next += 0x020000;

	DrvPalette		= (unsigned int*)Next; Next += 0x400 * sizeof(int);

	pAY8910Buffer[0]	= (short*)Next; Next += nBurnSoundLen * sizeof(short);
	pAY8910Buffer[1]	= (short*)Next; Next += nBurnSoundLen * sizeof(short);
	pAY8910Buffer[2]	= (short*)Next; Next += nBurnSoundLen * sizeof(short);

	AllRam			= Next;

	Drv68KRAM		= Next; Next += 0x004000;
	DrvM6809RAM		= Next; Next += 0x000800;
	DrvPalRAM		= Next; Next += 0x000800;
	DrvFgRAM		= Next; Next += 0x004000;
	DrvTxtRAM		= Next; Next += 0x000800;
	DrvSprRAM		= Next; Next += 0x000800;

	layer_control		= (unsigned short*)Next; Next += 0x000001 * sizeof(short);
	soundlatch		= Next; Next += 0x000001;
	flipscreen		= Next; Next += 0x000001;

	scroll			= (unsigned short*)Next; Next += 0x000004 * sizeof(short);

	RamEnd			= Next;

	MemEnd			= Next;

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
		if (BurnLoadRom(Drv68KROM   + 0x000001,  0, 2)) return 1;
		if (BurnLoadRom(Drv68KROM   + 0x000000,  1, 2)) return 1;

		if (BurnLoadRom(DrvM6809ROM + 0x000000,  2, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM0  + 0x000000,  3, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM0  + 0x010000,  4, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM1  + 0x000000,  5, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1  + 0x010000,  6, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM2  + 0x000000,  7, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM3  + 0x000000,  8, 1)) return 1;
		memcpy (DrvGfxROM3 + 0x40000, DrvGfxROM3 + 0x10000, 0x10000);
		if (BurnLoadRom(DrvGfxROM3  + 0x010000,  9, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM3  + 0x020000, 10, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM3  + 0x030000, 11, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM4  + 0x000000, 12, 1)) return 1;
		BurnByteswap(DrvGfxROM4, 0x8000);

		if (BurnLoadRom(DrvSndROM   + 0x000000, 13, 1)) return 1;
		if (BurnLoadRom(DrvSndROM   + 0x010000, 14, 1)) return 1;

		DrvGfxDecode(DrvGfxROM0, 0x20000, 16);
		DrvGfxDecode(DrvGfxROM1, 0x20000, 16);
		DrvGfxDecode(DrvGfxROM2, 0x04000,  8);
		DrvGfxDecode(DrvGfxROM3, 0x50000, 16);
	}

	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM, 		0x000000, 0x01ffff, SM_ROM);
	SekMapMemory(Drv68KRAM, 		0x020000, 0x023fff, SM_RAM);
	SekMapMemory(DrvTxtRAM, 		0x030000, 0x0307ff, SM_RAM);
	SekMapMemory(DrvSprRAM, 		0x040000, 0x0407ff, SM_RAM);
	SekMapMemory(DrvPalRAM, 		0x050000, 0x0507ff, SM_RAM);
	SekMapMemory(DrvFgRAM,  		0x068000, 0x06bfff, SM_RAM);
	SekSetWriteWordHandler(0,		ginganin_write_word);
	SekSetWriteByteHandler(0,		ginganin_write_byte);
	SekSetReadWordHandler(0,		ginganin_read_word);
	SekSetReadByteHandler(0,		ginganin_read_byte);
	SekClose();

	M6809Init(1);
	M6809Open(0);
	M6809MapMemory(DrvM6809RAM,		0x0000, 0x07ff, M6809_RAM);
	M6809MapMemory(DrvM6809ROM + 0x4000,	0x4000, 0xffff, M6809_ROM);
	M6809SetWriteByteHandler(ginganin_sound_write);
	M6809SetReadByteHandler(ginganin_sound_read);
	M6809Close();

	AY8910Init(0, 3579545 / 2, nBurnSoundRate, NULL, NULL, NULL, NULL);
	
	BurnY8950Init(3579545, DrvSndROM, 0x20000, NULL, &DrvSynchroniseStream, 1);
	BurnTimerAttachM6809Y8950(1000000);

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static int DrvExit()
{
	GenericTilesExit();

	SekExit();
	M6809Exit();

	BurnY8950Exit();
	AY8910Exit(0);

	free (AllMem);
	AllMem = NULL;

	return 0;
}

static void DrvRecalcPalette()
{
	unsigned short *pal = (unsigned short*)DrvPalRAM;
	for (int i = 0; i < 0x800 / 2; i++)
	{
		int r = (pal[i] >> 12) & 0x0f;
		int g = (pal[i] >>  8) & 0x0f;
		int b = (pal[i] >>  4) & 0x0f;

		DrvPalette[i] = BurnHighCol((r << 4) | r, (g << 4) | g, (b << 4) | b, 0);
	}
}

static void draw_layer(unsigned char *ram, unsigned char *gfx, int color_offset, int wide, int scroll_offset, int transparent)
{
	int scrollx = scroll[scroll_offset + 1] & ((wide * 16) - 1);
	int scrolly = scroll[scroll_offset + 0] & 0x01ff;

	unsigned short *vram = (unsigned short*)ram;

	for (int offs = 0; offs < 16 * 16 + 16; offs++)
	{
		int ofst  = ((((offs >> 4) + (scrollx >> 4)) & (wide-1)) << 5) + (((offs & 0xf) + (scrolly >> 4)) & 0x01f);

		int sy    = ((offs & 0xf) << 4) - (scrolly & 0x0f);
		int sx    = ((offs >> 4) << 4) - (scrollx & 0x0f);

		int attr  = vram[ofst];

		if (*flipscreen) {
			if (transparent) {
				Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, attr & 0x3ff, 240 - sx, 240 - sy - 16, attr >> 12, 4, 0x0f, color_offset, gfx);
			} else {
				Render16x16Tile_FlipXY_Clip(pTransDraw, attr & 0x3ff, 240 - sx, 240 - sy - 16, attr >> 12, 4, color_offset, gfx);
			}
		} else {
			if (transparent) {
				Render16x16Tile_Mask_Clip(pTransDraw, attr & 0x3ff, sx, sy - 16, attr >> 12, 4, 0x0f, color_offset, gfx);
			} else {
				Render16x16Tile_Clip(pTransDraw, attr & 0x3ff, sx, sy - 16, attr >> 12, 4, color_offset, gfx);
			}
		}
	}
}

static void draw_txt_layer()
{
	unsigned short *vram = (unsigned short*)DrvTxtRAM;

	for (int offs = 0; offs < 32 * 32; offs++)
	{
		int sx    = (offs & 0x1f) << 3;
		int sy    = (offs >> 5) << 3;

		if (sy < 16 || sy > 240) continue;

		int attr = vram[offs];

		if (*flipscreen) {
			Render8x8Tile_Mask_FlipXY(pTransDraw, attr & 0x1ff, sx ^ 0xf8, (sy ^ 0xf8) - 16, attr >> 12, 4, 0x0f, 0x000, DrvGfxROM2);
		} else {
			Render8x8Tile_Mask(pTransDraw, attr & 0x1ff, sx, sy - 16, attr >> 12, 4, 0x0f, 0x000, DrvGfxROM2);
		}
	}
}

static void draw_sprites()
{
	unsigned short *vram = (unsigned short*)DrvSprRAM;

	for (int offs = 0; offs < 0x800 / 2; offs += 4)
	{
		int sy    = vram[offs + 0];
		int sx    = vram[offs + 1];
		int code  = vram[offs + 2];
		int color = vram[offs + 3] >> 12;
		int flipx = code & 0x4000;
		int flipy = code & 0x8000;

		sx = (sx & 0xff) - (sx & 0x100);
		sy = (sy & 0xff) - (sy & 0x100);
		code &= 0xfff;

		if (code >= 0xa00) continue;

		if (*flipscreen)
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx ^= 0x4000;
			flipy ^= 0x8000;
		}

		if (flipy) {
			if (flipx) {
				Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, code, sx, sy - 16, color, 4, 0x0f, 0x100, DrvGfxROM3);
			} else {
				Render16x16Tile_Mask_FlipY_Clip(pTransDraw, code, sx, sy - 16, color, 4, 0x0f, 0x100, DrvGfxROM3);
			}
		} else {
			if (flipx) {
				Render16x16Tile_Mask_FlipX_Clip(pTransDraw, code, sx, sy - 16, color, 4, 0x0f, 0x100, DrvGfxROM3);
			} else {
				Render16x16Tile_Mask_Clip(pTransDraw, code, sx, sy - 16, color, 4, 0x0f, 0x100, DrvGfxROM3);
			}
		}
	}
}

static int DrvDraw()
{
	if (DrvRecalc) {
		DrvRecalcPalette();
	}

	if (*layer_control & 0x01) {
		draw_layer(DrvGfxROM4, DrvGfxROM0, 0x300, 0x200, 2, 0);
	} else {
		BurnTransferClear();
	}

	if (*layer_control & 0x02) {
		draw_layer(DrvFgRAM,   DrvGfxROM1, 0x200, 0x100, 0, 1);
	}

	if (*layer_control & 0x08) {
		draw_sprites();
	}

	if (*layer_control & 0x04) {
		draw_txt_layer();
	}

	BurnTransferCopy(DrvPalette);

	return 0;
}

static void sound_interrupt()
{
	if (MC6840_tempo2 != MC6840_tempo) {
		MC6840_tempo2 = MC6840_tempo;
		MC6840_ctr = 0;
	}

	if (MC6840_flag) {
		if (MC6840_ctr > MC6840_tempo) {
			MC6840_ctr = 0;
			M6809SetIRQ(0, M6809_IRQSTATUS_AUTO);
		} else {
			MC6840_ctr++;
		}
	}
}

static int DrvFrame()
{
	if (DrvReset) {
		DrvDoReset();
	}

	{
		DrvInputs[0] = 0xffff;

		for (int i = 0; i < 16; i++) {
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
		}
	}

	int nInterleave = 60;
	int nCyclesTotal[2] = { 6000000 / 60, 1000000 / 60 };
	int nCyclesDone[2]  = { 0, 0 };
	
	M6809NewFrame();

	SekOpen(0);
	M6809Open(0);

	for (int i = 0; i < nInterleave; i++)
	{
		int nSegment;

		nSegment = nCyclesTotal[0] / nInterleave;
		nCyclesDone[0] += SekRun(nSegment);
		if (i == (nInterleave - 1)) SekSetIRQLine(1, SEK_IRQSTATUS_AUTO);

		nSegment = nCyclesTotal[1] / nInterleave;
		BurnTimerUpdateY8950(i * (nCyclesTotal[1] / nInterleave));
		sound_interrupt();
	}

	SekClose();
	
	BurnTimerEndFrameY8950(nCyclesTotal[1]);

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

			pBurnSoundOut[(n << 1) + 0] = nSample;
			pBurnSoundOut[(n << 1) + 1] = nSample;
		}
		
		BurnY8950Update(pBurnSoundOut, nBurnSoundLen);
	}
	
	M6809Close();

	if (pBurnDraw) {
		DrvDraw();
	}

	return 0;
}

static int DrvScan(int nAction,int *pnMin)
{
	struct BurnArea ba;

	if (pnMin) {
		*pnMin = 0x029707;
	}

	if (nAction & ACB_VOLATILE) {		
		memset(&ba, 0, sizeof(ba));

		ba.Data	  = AllRam;
		ba.nLen	  = RamEnd - AllRam;
		ba.szName = "All Ram";
		BurnAcb(&ba);

		SekScan(nAction);
		ZetScan(nAction);

		AY8910Scan(nAction, pnMin);

		SCAN_VAR(MC6840_idx0);
		SCAN_VAR(MC6840_idx1);
		SCAN_VAR(MC6840_reg0);
		SCAN_VAR(MC6840_reg1);
		SCAN_VAR(MC6840_tempo);
		SCAN_VAR(MC6840_tempo2);
		SCAN_VAR(MC6840_ctr);
		SCAN_VAR(MC6840_flag);
	}

	return 0;
}


// Ginga NinkyouDen (set 1)

static struct BurnRomInfo ginganinRomDesc[] = {
	{ "gn_02.bin",	0x10000, 0x4a4e012f, 1 | BRF_PRG | BRF_ESS }, //  0 M68000 Code
	{ "gn_01.bin",	0x10000, 0x30256fcb, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "gn_05.bin",	0x10000, 0xe76e10e7, 2 | BRF_PRG | BRF_ESS }, //  2 M6809 Code

	{ "gn_15.bin",	0x10000, 0x1b8ac9fb, 3 | BRF_GRA },           //  3 Background Tiles
	{ "gn_14.bin",	0x10000, 0xe73fe668, 3 | BRF_GRA },           //  4

	{ "gn_12.bin",	0x10000, 0xc134a1e9, 4 | BRF_GRA },           //  5 Foreground Tiles
	{ "gn_13.bin",	0x10000, 0x1d3bec21, 4 | BRF_GRA },           //  6

	{ "gn_10.bin",	0x04000, 0xae371b2d, 5 | BRF_GRA },           //  7 Characters

	{ "gn_06.bin",	0x20000, 0xbdc65835, 6 | BRF_GRA },           //  8 Sprites
	{ "gn_07.bin",	0x10000, 0xc2b8eafe, 6 | BRF_GRA },           //  9
	{ "gn_08.bin",	0x10000, 0xf7c73c18, 6 | BRF_GRA },           // 10
	{ "gn_09.bin",	0x10000, 0xa5e07c3b, 6 | BRF_GRA },           // 11

	{ "gn_11.bin",	0x08000, 0xf0d0e605, 7 | BRF_GRA },           // 12 Background Tilemap

	{ "gn_04.bin",	0x10000, 0x0ed9133b, 8 | BRF_SND },           // 13 Y8950 Samples
	{ "gn_03.bin",	0x10000, 0xf1ba222c, 8 | BRF_SND },           // 14
};

STD_ROM_PICK(ginganin)
STD_ROM_FN(ginganin)

struct BurnDriver BurnDrvGinganin = {
	"ginganin", NULL, NULL, NULL, "1987",
	"Ginga NinkyouDen (set 1)\0", "Incomplete sound", "Jaleco", "Miscellaneous",
	L"\u9280\u6CB3\u4EFB\u4FA0\u4F1D (set 1)\0Ginga NinkyouDen (set 1)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_PRE90S, GBF_PLATFORM, 0,
	NULL, ginganinRomInfo, ginganinRomName, NULL, NULL, GinganinInputInfo, GinganinDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	256, 224, 4, 3
};


// Ginga NinkyouDen (set 2)

static struct BurnRomInfo ginganinaRomDesc[] = {
	{ "2.bin",	0x10000, 0x6da1d8a3, 1 | BRF_PRG | BRF_ESS }, //  0 M68000 Code
	{ "1.bin",	0x10000, 0x0bd32d59, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "gn_05.bin",	0x10000, 0xe76e10e7, 2 | BRF_PRG | BRF_ESS }, //  2 M6809 Code

	{ "gn_15.bin",	0x10000, 0x1b8ac9fb, 3 | BRF_GRA },           //  3 Background Tiles
	{ "gn_14.bin",	0x10000, 0xe73fe668, 3 | BRF_GRA },           //  4

	{ "gn_12.bin",	0x10000, 0xc134a1e9, 4 | BRF_GRA },           //  5 Foreground Tiles
	{ "gn_13.bin",	0x10000, 0x1d3bec21, 4 | BRF_GRA },           //  6

	{ "10.bin",	0x04000, 0x48a20745, 5 | BRF_GRA },           //  7 Characters

	{ "gn_06.bin",	0x20000, 0xbdc65835, 6 | BRF_GRA },           //  8 Sprites
	{ "gn_07.bin",	0x10000, 0xc2b8eafe, 6 | BRF_GRA },           //  9
	{ "gn_08.bin",	0x10000, 0xf7c73c18, 6 | BRF_GRA },           // 10
	{ "gn_09.bin",	0x10000, 0xa5e07c3b, 6 | BRF_GRA },           // 11

	{ "gn_11.bin",	0x08000, 0xf0d0e605, 7 | BRF_GRA },           // 12 Background Tilemap

	{ "gn_04.bin",	0x10000, 0x0ed9133b, 8 | BRF_SND },           // 13 Y8950 Samples
	{ "gn_03.bin",	0x10000, 0xf1ba222c, 8 | BRF_SND },           // 14
};

STD_ROM_PICK(ginganina)
STD_ROM_FN(ginganina)

struct BurnDriver BurnDrvGinganina = {
	"ginganina", "ginganin", NULL, NULL, "1987",
	"Ginga NinkyouDen (set 2)\0", "Incomplete sound", "Jaleco", "Miscellaneous",
	L"\u9280\u6CB3\u4EFB\u4FA0\u4F1D (set 2)\0Ginga NinkyouDen (set 2)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_PRE90S, GBF_PLATFORM, 0,
	NULL, ginganinaRomInfo, ginganinaRomName, NULL, NULL, GinganinInputInfo, GinganinDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	256, 224, 4, 3
};
