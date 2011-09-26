
#include "burnint.h"
#include "bitswap.h"
#include "driver.h"
extern "C" {
#include "ay8910.h"
}


static unsigned char *Mem, *Rom, *Ram, *Prom;
static unsigned char DrvJoy1[32], DrvJoy2[32], DrvJoy3[4], DrvReset, DrvDips[4];
static int suzume = 0, royalmah = 0, mjderngr = 0;
static short* pAY8910Buffer[3];
static short *pFMBuffer = NULL;
static int *palette;

static int RomBankAddr, palette_base;
static unsigned char input_port_select, dsw_select, suzume_bank;


static struct BurnInputInfo royalmahInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy1 +  0,	"p1 coin"},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 +  1,	"p1 start"},

	{"A",			BIT_DIGITAL,	DrvJoy1 +  3,	"mah a"},
	{"E",			BIT_DIGITAL,	DrvJoy1 +  4,	"mah e"},
	{"I",			BIT_DIGITAL,	DrvJoy1 +  5,	"mah i"},
	{"M",			BIT_DIGITAL,	DrvJoy1 +  6,	"mah m"},
	{"Kan",			BIT_DIGITAL,	DrvJoy1 +  7,	"mah kan"},

	{"B",			BIT_DIGITAL,	DrvJoy1 +  8,	"mah b"},
	{"F",			BIT_DIGITAL,	DrvJoy1 +  9,	"mah f"},
	{"J",			BIT_DIGITAL,	DrvJoy1 + 10,	"mah j"},
	{"N",			BIT_DIGITAL,	DrvJoy1 + 11,	"mah n"},
	{"Reach",		BIT_DIGITAL,	DrvJoy1 + 12,	"mah reach"},

	{"C",			BIT_DIGITAL,	DrvJoy1 + 13,	"mah c"},
	{"G",			BIT_DIGITAL,	DrvJoy1 + 14,	"mah g"},
	{"K",			BIT_DIGITAL,	DrvJoy1 + 15,	"mah k"},
	{"Chi",			BIT_DIGITAL,	DrvJoy1 + 16,	"mah chi"},
	{"Ron",			BIT_DIGITAL,	DrvJoy1 + 17,	"mah ron"},

	{"D",			BIT_DIGITAL,	DrvJoy1 + 18,	"mah d"},
	{"H",			BIT_DIGITAL,	DrvJoy1 + 19,	"mah h"},
	{"L",			BIT_DIGITAL,	DrvJoy1 + 20,	"mah l"},
	{"Pon",			BIT_DIGITAL,	DrvJoy1 + 21,	"mah pon"},

	{"Bet",			BIT_DIGITAL,    DrvJoy1 + 22,   "bet" },

	{"Last Chance",		BIT_DIGITAL,    DrvJoy1 + 23,   "last chance" },
	{"Double Up",		BIT_DIGITAL,	DrvJoy1 + 24,   "Double Up" },
	{"Flip Flop",		BIT_DIGITAL,	DrvJoy1 + 25,   "Flip Flop" },
	{"Big",			BIT_DIGITAL,	DrvJoy1 + 26,   "Big" },
	{"Small",		BIT_DIGITAL,	DrvJoy1 + 27,   "Small" },

	{"P1 Credit Clear",	BIT_DIGITAL,	DrvJoy1 + 28,   "Credit Clear" },

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy2 +  0,	"p1 coin"},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy2 +  1,	"p1 start"},

	{"A",			BIT_DIGITAL,	DrvJoy2 +  3,	"mah a"},
	{"E",			BIT_DIGITAL,	DrvJoy2 +  4,	"mah e"},
	{"I",			BIT_DIGITAL,	DrvJoy2 +  5,	"mah i"},
	{"M",			BIT_DIGITAL,	DrvJoy2 +  6,	"mah m"},
	{"Kan",			BIT_DIGITAL,	DrvJoy2 +  7,	"mah kan"},

	{"B",			BIT_DIGITAL,	DrvJoy2 +  8,	"mah b"},
	{"F",			BIT_DIGITAL,	DrvJoy2 +  9,	"mah f"},
	{"J",			BIT_DIGITAL,	DrvJoy2 + 10,	"mah j"},
	{"N",			BIT_DIGITAL,	DrvJoy2 + 11,	"mah n"},
	{"Reach",		BIT_DIGITAL,	DrvJoy2 + 12,	"mah reach"},

	{"C",			BIT_DIGITAL,	DrvJoy2 + 13,	"mah c"},
	{"G",			BIT_DIGITAL,	DrvJoy2 + 14,	"mah g"},
	{"K",			BIT_DIGITAL,	DrvJoy2 + 15,	"mah k"},
	{"Chi",			BIT_DIGITAL,	DrvJoy2 + 16,	"mah chi"},
	{"Ron",			BIT_DIGITAL,	DrvJoy2 + 17,	"mah ron"},

	{"D",			BIT_DIGITAL,	DrvJoy2 + 18,	"mah d"},
	{"H",			BIT_DIGITAL,	DrvJoy2 + 19,	"mah h"},
	{"L",			BIT_DIGITAL,	DrvJoy2 + 20,	"mah l"},
	{"Pon",			BIT_DIGITAL,	DrvJoy2 + 21,	"mah pon"},

	{"Bet",			BIT_DIGITAL,    DrvJoy2 + 22,   "bet" },

	{"Last Chance",		BIT_DIGITAL,    DrvJoy2 + 23,   "last chance" },
	{"Double Up",		BIT_DIGITAL,	DrvJoy2 + 24,   "Double Up" },
	{"Flip Flop",		BIT_DIGITAL,	DrvJoy2 + 25,   "Flip Flop" },
	{"Big",			BIT_DIGITAL,	DrvJoy2 + 26,   "Big" },
	{"Small",		BIT_DIGITAL,	DrvJoy2 + 27,   "Small" },

	{"P2 Credit Clear",	BIT_DIGITAL,	DrvJoy2 + 28,	"Credit Clear" },

	{"Note (Paper Money) = 10 Credits", BIT_DIGITAL, DrvJoy3 + 0, "Note = 10 creds" },
	{"Memory Reset",	BIT_DIGITAL,	DrvJoy3 + 1,	"Memory Reset" },
	{"Statistics",		BIT_DIGITAL,	DrvJoy3 + 2,	"Statistics" },

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"},
};

STDINPUTINFO(royalmah)


//-----------------------------------------------------------------------------------------------

static unsigned char input_port(int port)
{
	unsigned char ret = 0xff;
	int i = 0;

	switch (port)
	{
		case 0:
			for (i = 0; i < 5; i++)
				ret ^= DrvJoy1[3+i] << i;

			ret ^= DrvJoy1[ 1] << 5;
			ret ^= DrvJoy1[28] << 6;
			ret ^= DrvJoy2[28] << 7;
		break;

		case 1:
			for (i = 0; i < 5; i++)
				ret ^= DrvJoy1[8+i] << i;

			ret ^= DrvJoy1[22] << 5;
		break;

		case 2:
			for (i = 0; i < 5; i++)
				ret ^= DrvJoy1[13+i] << i;
		break;

		case 3:
			for (i = 0; i < 4; i++)
				ret ^= DrvJoy1[18+i] << i;
		break;

		case 4:
			for (i = 0; i < 5; i++)
				ret ^= DrvJoy1[23+i] << i;
		break;

		case 5:
			for (i = 0; i < 5; i++)
				ret ^= DrvJoy2[3+i] << i;

			ret ^= DrvJoy2[ 1] << 5;
			ret ^= DrvJoy1[ 0] << 6;
			ret ^= DrvJoy2[ 0] << 7;
		break;

		case 6:
			for (i = 0; i < 5; i++)
				ret ^= DrvJoy2[8+i] << i;

			ret ^= DrvJoy2[22] << 5;
		break;

		case 7:
			for (i = 0; i < 5; i++)
				ret ^= DrvJoy2[13+i] << i;
		break;

		case 8:
			for (i = 0; i < 4; i++)
				ret ^= DrvJoy1[18+i] << i;
		break;

		case 9:
			for (i = 0; i < 5; i++)
				ret ^= DrvJoy1[23+i] << i;
		break;

		case 10:
			for (i = 0, ret = 0; i < 3; i++)
				ret |= DrvJoy3[i] << i;
		break;

		case 11:
			ret = 0;
		break;

		case 12:
		case 13:
		case 14:
			ret = 0xff;
		break;
	}

	return ret;
}


static unsigned char royalmah_player_1_port_r(unsigned int)
{
	int ret = (input_port(0) & 0xc0) | 0x3f;

	if ((input_port_select & 0x01) == 0)  ret &= input_port(0);
	if ((input_port_select & 0x02) == 0)  ret &= input_port(1);
	if ((input_port_select & 0x04) == 0)  ret &= input_port(2);
	if ((input_port_select & 0x08) == 0)  ret &= input_port(3);
	if ((input_port_select & 0x10) == 0)  ret &= input_port(4);

	return ret;
}

static unsigned char royalmah_player_2_port_r(unsigned int)
{
	int ret = (input_port(5) & 0xc0) | 0x3f;

	if ((input_port_select & 0x01) == 0)  ret &= input_port(5);
	if ((input_port_select & 0x02) == 0)  ret &= input_port(6);
	if ((input_port_select & 0x04) == 0)  ret &= input_port(7);
	if ((input_port_select & 0x08) == 0)  ret &= input_port(8);
	if ((input_port_select & 0x10) == 0)  ret &= input_port(9);

	return ret;
}


static unsigned char suzume_dsw_r()
{
	if (suzume_bank & 0x40)
	{
		return suzume_bank;
	}
	else
	{
		switch (suzume_bank)
		{
			case 0x08: return input_port(14);	// DSW4
			case 0x10: return input_port(13);	// DSW3
			case 0x18: return input_port(12);	// DSW2
		}
		return 0;
	}
}

static unsigned char majs101b_dsw_r()
{
	switch (dsw_select)
	{
		case 0x00: return input_port(13);	// DSW3
		case 0x20: return input_port(14);	// DSW4
		case 0x40: return input_port(12);	// DSW2
	}
	return 0;
}


static void bankswitch_w(unsigned char data, unsigned char andval)
{
	RomBankAddr = 0x10000 + (data & andval) * 0x08000;

	ZetMapArea(0x8000, 0xffff, 0, Rom + RomBankAddr); // banked rom
	ZetMapArea(0x8000, 0xffff, 2, Rom + RomBankAddr);
}


void __fastcall royalmah_out_port(unsigned short a, unsigned char data)
{
	switch (a & 0xff)
	{
		case 0x00: // majs101b,
		case 0x20: // mjderngr
		case 0x87:
			dsw_select = data & 0x60;
			bankswitch_w(data, 0x1f);
		break;

		case 0x02:
		case 0x03:
			AY8910Write(0, ~a & 1, data);
		break;

		case 0x10:
			palette_base = (data >> 3) & 0x01;
		break;

		case 0x11:
			input_port_select = data;
		break;

		case 0x60: // mjderngr
			palette_base = data;
		break;

		case 0x44: // tonton
		case 0x61: // mjdiplob
			if (data) {
				bankswitch_w(data, 0x0f);
			}
		break;

		case 0x81: // suzume
			suzume_bank = data;
			bankswitch_w(data, 0x07);
		break;
	}
}

unsigned char __fastcall royalmah_in_port(unsigned short a)
{
	switch (a & 0xff)
	{
		case 0x00:
			return majs101b_dsw_r();

		case 0x01:
			return AY8910Read(0);

		case 0x10:
			return input_port(11);

		case 0x11:
			return input_port(10);

		case 0x04: // mjapinky
		case 0x12: // ippatsu
		case 0x47: // tontonb
		case 0x4c: // mjderngr
		case 0x62: // mjdiplob
		case 0x85: // dondenmj
			return input_port(12);

		case 0x13: // ippatsu
		case 0x40: // mjderngr
		case 0x46: // tontonb
		case 0x63: // mjdiplob
		case 0x86: // dondenmj
			return input_port(13);

		case 0x80:
			return suzume_dsw_r();
	}

	return 0;
}



static void Drv_palette_init()
{
	for (int i = 0; i < 0x20; i++)
	{
		unsigned char bit0, bit1, bit2, r, g, b;
		unsigned char data = Prom[i];

		bit0 = (data >> 0) & 0x01;
		bit1 = (data >> 1) & 0x01;
		bit2 = (data >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		bit0 = (data >> 3) & 0x01;
		bit1 = (data >> 4) & 0x01;
		bit2 = (data >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		bit1 = (data >> 6) & 0x01;
		bit2 = (data >> 7) & 0x01;
		b = 0x47 * bit1 + 0x97 * bit2;

		palette[i] = (r << 16) | (g << 8) | b;
	}
}


static int DrvDoReset()
{
	DrvReset = 0;

	memset (Ram, 0, 0x10000);

	RomBankAddr = palette_base = 0;
	input_port_select = dsw_select = suzume_bank = 0;

	ZetOpen(0);
	ZetReset();
	ZetClose();

	AY8910Reset(0);

	return 0;
}


static int DrvInit()
{
	// Allocate ram
	Mem = (unsigned char *)malloc( 0x100000 + 0x10000 + 0x0400 + 0x0800);
	if (Mem == NULL) {
		return 1;
	}
	Rom      = Mem + 0x000000;
	Ram	 = Mem + 0x100000;
	Prom     = Mem + 0x110000;
	palette  = (int *)(Mem + 0x110400);

	// Allocate ram for sound
	pFMBuffer = (short *)malloc (nBurnSoundLen * 3 * sizeof(short));
	if (pFMBuffer == NULL) {
		return 1;
	}

	// Load roms
	{
		int i = 0;
		struct BurnRomInfo ri;
		unsigned char *RomLoad = Rom;
		unsigned char *pRomLoad = Prom;

		while (1)
		{
			ri.nLen = 0;
			BurnDrvGetRomInfo(&ri, i);
			if (ri.nLen == 0) break;

			if ((ri.nType & 7) == 1)	// Load z80 prg roms
			{
				if (BurnLoadRom(RomLoad, i, 1)) return 1;
				RomLoad += ri.nLen;
			}

			if ((ri.nType & 7) == 2)	// Load Proms
			{
				BurnLoadRom(pRomLoad, i, 1);
				pRomLoad += ri.nLen;
			}

			i++;
		}
	}

	Drv_palette_init();

	{
		ZetInit(1);
		ZetOpen(0);
		ZetSetInHandler(royalmah_in_port);
		ZetSetOutHandler(royalmah_out_port);
		ZetMapArea(0x0000, 0x6fff, 0, Rom + 0x0000); // rom
		ZetMapArea(0x0000, 0x6fff, 2, Rom + 0x0000);
		ZetMapArea(0x7000, 0x7fff, 0, Ram + 0x7000); // nvram
		ZetMapArea(0x7000, 0x7fff, 1, Ram + 0x7000);
		ZetMapArea(0x8000, 0xffff, 0, Rom + 0x8000); // banked rom
		ZetMapArea(0x8000, 0xffff, 2, Rom + 0x8000);
		ZetMapArea(0x8000, 0xffff, 1, Ram + 0x8000); // video ram
		ZetMemEnd();
		ZetClose();
	}

	// Set sound buffers
	pAY8910Buffer[0] = pFMBuffer + nBurnSoundLen * 0;
	pAY8910Buffer[1] = pFMBuffer + nBurnSoundLen * 1;
	pAY8910Buffer[2] = pFMBuffer + nBurnSoundLen * 2;

	// Initilize sound chip
	AY8910Init(0, 1536000, nBurnSoundRate, &royalmah_player_1_port_r, &royalmah_player_2_port_r, NULL, NULL);

	// Reset the machine
	DrvDoReset();

	return 0;
}


static int DrvExit()
{
	suzume = 0;
	royalmah = 0;
	mjderngr = 0;

	AY8910Exit(0);

	ZetExit();

	free (Mem);
	Mem = NULL;

	return 0;
}



static int DrvDraw()
{
	static int i, j, color;
	static unsigned char x, y, d1, d2;

	for (i = 0; i < 0x4000; i++)
	{
		d1 = Ram[i + 0x8000];
		d2 = Ram[i + 0xc000];

		y = (i >> 6) ^ 0xff;
		x = (i << 2) ^ 0xff;

		for (j = 0; j < 4; j++)
		{
			color = ((d2 >> 1) & 8) | ((d2 << 2) & 4) | ((d1 >> 3) & 2) | (d1 & 1);
			color = palette[ color | (palette_base << 4) ];

			PutPix(pBurnDraw + ((y << 8) | x) * nBurnBpp, BurnHighCol(color >> 16, color >> 8, color, 0));

			x -= 1;
			d1 >>= 1;
			d2 >>= 1;
		}
	}

	return 0;
}


static int DrvFrame()
{
	if (DrvReset) DrvDoReset();

	int nCyclesTotal = royalmah ? 3000000 : 4000000;

	ZetOpen(0);
	ZetRun(nCyclesTotal / 60);

	if (suzume) {
		if (suzume_bank & 0x40)
			ZetNmi();
	} else {
		ZetRaiseIrq(1);
	}

	ZetClose();

	int nSoundBufferPos = 0;

	if (pBurnSoundOut) {
		int nSample;
		int nSegmentLength = nBurnSoundLen - nSoundBufferPos;
		short* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
		if (nSegmentLength) {
			AY8910Update(0, &pAY8910Buffer[0], nSegmentLength);
			for (int n = 0; n < nSegmentLength; n++) {
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

				pSoundBuf[(n << 1) + 0] = nSample;
				pSoundBuf[(n << 1) + 1] = nSample;
			}
		}
	}

	if (pBurnDraw) DrvDraw();

	return 0;
}


//-----------------------------------------------------------------------------------------------
// Game drivers


// Royal Mahjong (Japan, v1.13)

static struct BurnRomInfo royalmjRomDesc[] = {
	{ "1.p1", 	0x1000, 0x549544bb, 1 | BRF_ESS | BRF_PRG }, //  0 Z80 Code
	{ "2.p2", 	0x1000, 0xafc8a61e, 1 | BRF_ESS | BRF_PRG }, //  1
	{ "3.p3", 	0x1000, 0x5d33e54d, 1 | BRF_ESS | BRF_PRG }, //  2
	{ "4.p4", 	0x1000, 0x91339560, 1 | BRF_ESS | BRF_PRG }, //  3
	{ "5.p5", 	0x1000, 0xcc9123a3, 1 | BRF_ESS | BRF_PRG }, //  4
	{ "6.p6",       0x1000, 0x92150a0f, 1 | BRF_ESS | BRF_PRG }, //  5

	{ "18s030n.6k", 0x0020, 0xd3007282, 2 | BRF_GRA }, 	     //  6 Color PROM
};

STD_ROM_PICK(royalmj)
STD_ROM_FN(royalmj)

static int royalmjInit()
{
	royalmah = 1;

	return DrvInit();
}

struct BurnDriver BurnDrvroyalmj = {
	"royalmj", NULL, NULL, "1981",
	"Royal Mahjong (Japan, v1.13)\0", NULL, "Nichibutsu", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_MISC,
	NULL, royalmjRomInfo, royalmjRomName, royalmahInputInfo, NULL,
	royalmjInit, DrvExit, DrvFrame, DrvDraw, NULL, NULL,
	256, 256, 1, 1
};

// Royal Mahjong (Falcon bootleg, v1.01)

static struct BurnRomInfo royalmahRomDesc[] = {
	{ "rom1",       0x1000, 0x69b37a62, 1 | BRF_ESS | BRF_PRG }, //  0 Z80 Code
	{ "rom2",       0x1000, 0x0c8351b6, 1 | BRF_ESS | BRF_PRG }, //  1
	{ "rom3",       0x1000, 0xb7736596, 1 | BRF_ESS | BRF_PRG }, //  2
	{ "rom4",       0x1000, 0xe3c7c15c, 1 | BRF_ESS | BRF_PRG }, //  3
	{ "rom5",       0x1000, 0x16c09c73, 1 | BRF_ESS | BRF_PRG }, //  4
	{ "rom6",       0x1000, 0x92687327, 1 | BRF_ESS | BRF_PRG }, //  5

	{ "f-rom.bpr",  0x0020, 0xd3007282, 2 | BRF_GRA }, 	     //  6 Color PROM
};

STD_ROM_PICK(royalmah)
STD_ROM_FN(royalmah)

struct BurnDriver BurnDrvroyalmah = {
	"royalmah", "royalmj", NULL, "1982",
	"Royal Mahjong (Falcon bootleg, v1.01)\0", NULL, "bootleg", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_MISC,
	NULL, royalmahRomInfo, royalmahRomName, royalmahInputInfo, NULL,
	royalmjInit, DrvExit, DrvFrame, DrvDraw, NULL, NULL,
	256, 256, 1, 1
};

// Open Mahjong [BET] (Japan)

static struct BurnRomInfo openmjRomDesc[] = {
	{ "10",         0x2000, 0x4042920e, 1 | BRF_ESS | BRF_PRG }, //  0 Z80 Code
	{ "20",         0x2000, 0x8fa0f735, 1 | BRF_ESS | BRF_PRG }, //  1
	{ "30",         0x2000, 0x00045cd7, 1 | BRF_ESS | BRF_PRG }, //  2

	{ "82s123.prm", 0x0020, 0xd3007282, 2 | BRF_GRA }, 	     //  3 Color PROM
};

STD_ROM_PICK(openmj)
STD_ROM_FN(openmj)

struct BurnDriver BurnDrvopenmj = {
	"openmj", "royalmj", NULL, "1982",
	"Open Mahjong [BET] (Japan)\0", NULL, "Sapporo Mechanic", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_MISC,
	NULL, openmjRomInfo, openmjRomName, royalmahInputInfo, NULL,
	royalmjInit, DrvExit, DrvFrame, DrvDraw, NULL, NULL,
	256, 256, 1, 1
};


// Janyou Part II (ver 7.03, July 1 1983)

static struct BurnRomInfo janyoup2RomDesc[] = {
	{ "1.c110",       0x2000, 0x36ebb3d0, 1 | BRF_ESS | BRF_PRG }, //  0 Z80 Code
	{ "2.c109",       0x2000, 0x324426d4, 1 | BRF_ESS | BRF_PRG }, //  1
	{ "3.c108",       0x2000, 0xe98b6d34, 1 | BRF_ESS | BRF_PRG }, //  2
	{ "4.c107",       0x1000, 0x377b8ce9, 1 | BRF_ESS | BRF_PRG }, //  3

	{ "n82s123n.c98", 0x0020, 0xd3007282, 2 | BRF_GRA }, 	       //  4 Color PROM
};

STD_ROM_PICK(janyoup2)
STD_ROM_FN(janyoup2)

struct BurnDriver BurnDrvjanyoup2 = {
	"janyoup2", "royalmj", NULL, "1983",
	"Janyou Part II (ver 7.03, July 1 1983)\0", NULL, "Cosmo Denshi", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_MISC,
	NULL, janyoup2RomInfo, janyoup2RomName, royalmahInputInfo, NULL,
	DrvInit, DrvExit, DrvFrame, DrvDraw, NULL, NULL,
	256, 256, 1, 1
};


// Jan Oh

static struct BurnRomInfo janohRomDesc[] = {
	{ "rom1.p1",     0x1000, 0x8fc19963, 1 | BRF_ESS | BRF_PRG },    //  0 Z80 Code
	{ "rom2.p12",    0x1000, 0xe1141ae1, 1 | BRF_ESS | BRF_PRG },    //  1
	{ "rom3.p2",     0x1000, 0x66e6d2f4, 1 | BRF_ESS | BRF_PRG },    //  2
	{ "rom4.p3",     0x1000, 0x9186f02c, 1 | BRF_ESS | BRF_PRG },    //  3
	{ "rom5.p4",     0x1000, 0xf3c478a8, 1 | BRF_ESS | BRF_PRG },    //  4
	{ "rom6.p5",     0x1000, 0x92687327, 1 | BRF_ESS | BRF_PRG },    //  5
	{ "rom7.p6",     0x1000, 0,          1 | BRF_PRG | BRF_NODUMP }, //  6

	{ "janho.color", 0x0020, 0,          2 | BRF_GRA | BRF_NODUMP }, //  7 Color PROM
};

STD_ROM_PICK(janoh)
STD_ROM_FN(janoh)

struct BurnDriver BurnDrvjanoh = {
	"janoh", "royalmj", NULL, "1984",
	"Jan Oh\0", NULL, "Toaplan", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_CLONE, 2, HARDWARE_MISC_MISC,
	NULL, janohRomInfo, janohRomName, royalmahInputInfo, NULL,
	royalmjInit, DrvExit, DrvFrame, DrvDraw, NULL, NULL,
	256, 256, 1, 1
};

// Ippatsu Gyakuten [BET] (Japan)

static struct BurnRomInfo ippatsuRomDesc[] = {
	{ "11",       0x8000, 0x5f563be7, 1 | BRF_ESS | BRF_PRG },    //  0 Z80 Code
	{ "12",       0x4000, 0xa09a43b0, 1 | BRF_ESS | BRF_PRG },    //  1

	{ "82s123an", 0x0020, 0x3bde1bbd, 2 | BRF_GRA }, 	      //  2 Color PROM
};

STD_ROM_PICK(ippatsu)
STD_ROM_FN(ippatsu)

static int ippatsuInit()
{
	int nRet;

	nRet = DrvInit();

	memcpy (Rom + 0xc000, Rom + 0x8000, 0x4000);

	return nRet;
}

struct BurnDriver BurnDrvippatsu = {
	"ippatsu", NULL, NULL, "1986",
	"Ippatsu Gyakuten [BET] (Japan)\0", NULL, "Public Software / Paradais", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_MISC,
	NULL, ippatsuRomInfo, ippatsuRomName, royalmahInputInfo, NULL,
	ippatsuInit, DrvExit, DrvFrame, DrvDraw, NULL, NULL,
	256, 256, 1, 1
};



// Don Den Mahjong [BET] (Japan)

static struct BurnRomInfo dondenmjRomDesc[] = {
	{ "dn5.1h",     0x08000, 0x3080252e, 1 | BRF_ESS | BRF_PRG },    //  0 Z80 Code
	// bank switched ROMs follow
	{ "dn1.1e",     0x08000, 0x1cd9c48a, 1 | BRF_ESS | BRF_PRG },    //  1
	{ "dn2.1d",     0x04000, 0x7a72929d, 1 | BRF_ESS | BRF_PRG },    //  2
	{ "dn3.2h",     0x08000, 0xb09d2897, 1 | BRF_ESS | BRF_PRG },    //  3
	{ "dn4.2e",     0x08000, 0x67d7dcd6, 1 | BRF_ESS | BRF_PRG },    //  4

	{ "ic6k.bin",   0x0020, 0x97e1defe, 2 | BRF_GRA | BRF_NODUMP },  //  7 Color PROM
};

STD_ROM_PICK(dondenmj)
STD_ROM_FN(dondenmj)

static int dondenmjInit()
{
	int nRet;

	nRet = DrvInit();

	memset (Rom + 0x08000, 0, 0x20000);

	nRet += BurnLoadRom(Rom + 0x18000, 1, 1);
	nRet += BurnLoadRom(Rom + 0x20000, 2, 1);
	nRet += BurnLoadRom(Rom + 0x30000, 3, 1);
	nRet += BurnLoadRom(Rom + 0x50000, 4, 1);

	return nRet;
}

struct BurnDriver BurnDrvdondenmj = {
	"dondenmj", NULL, NULL, "1986",
	"Don Den Mahjong [BET] (Japan)\0", NULL, "Dyna Electronics", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_MISC,
	NULL, dondenmjRomInfo, dondenmjRomName, royalmahInputInfo, NULL,
	dondenmjInit, DrvExit, DrvFrame, DrvDraw, NULL, NULL,
	256, 256, 1, 1
};


// Watashiha Suzumechan (Japan)

static struct BurnRomInfo suzumeRomDesc[] = {
	{ "p1.bin",     0x1000, 0xe9706967, 1 | BRF_ESS | BRF_PRG },  //  0
	{ "p2.bin",     0x1000, 0xdd48cd62, 1 | BRF_ESS | BRF_PRG },  //  1
	{ "p3.bin",     0x1000, 0x10a05c23, 1 | BRF_ESS | BRF_PRG },  //  2
	{ "p4.bin",     0x1000, 0x267eaf52, 1 | BRF_ESS | BRF_PRG },  //  3
	{ "p5.bin",     0x1000, 0x2fde346b, 1 | BRF_ESS | BRF_PRG },  //  4
	{ "p6.bin",     0x1000, 0x57f42ac7, 1 | BRF_ESS | BRF_PRG },  //  5
	// bank switched ROMs follow
	{ "1.1a",       0x8000, 0xf670dd47, 1 | BRF_ESS | BRF_PRG },  //  6
	{ "2.1c",       0x8000, 0x140b11aa, 1 | BRF_ESS | BRF_PRG },  //  7
	{ "3.1d",       0x8000, 0x3d437b61, 1 | BRF_ESS | BRF_PRG },  //  8
	{ "4.1e",       0x8000, 0x9da8952e, 1 | BRF_ESS | BRF_PRG },  //  9
	{ "5.1h",       0x8000, 0x04a6f41a, 1 | BRF_ESS | BRF_PRG },  // 10

	{ "ic6k.bin",   0x0020, 0x97e1defe, 2 | BRF_GRA }, 	      // 11 Color PROM
};

STD_ROM_PICK(suzume)
STD_ROM_FN(suzume)

static int suzumeInit()
{
	int nRet;
	suzume = 1;

	nRet = DrvInit();

	nRet += BurnLoadRom(Rom + 0x10000,  6, 1);
	nRet += BurnLoadRom(Rom + 0x18000,  7, 1);
	nRet += BurnLoadRom(Rom + 0x20000,  8, 1);
	nRet += BurnLoadRom(Rom + 0x28000,  9, 1);
	nRet += BurnLoadRom(Rom + 0x30000, 10, 1);

	return nRet;
}

struct BurnDriver BurnDrvsuzume = {
	"suzume", NULL, NULL, "1986",
	"Watashiha Suzumechan (Japan)\0", NULL, "Dyna Electronics", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_MISC,
	NULL, suzumeRomInfo, suzumeRomName, royalmahInputInfo, NULL,
	suzumeInit, DrvExit, DrvFrame, DrvDraw, NULL, NULL,
	256, 256, 1, 1
};



// Mahjong Diplomat [BET] (Japan)

static struct BurnRomInfo mjdiplobRomDesc[] = {
	{ "071.4l",     0x10000, 0x81a6d6b0, 1 | BRF_ESS | BRF_PRG },  //  0 Z80 Code
	// bank switched ROMs follow
	{ "072.4k",     0x10000, 0xa992bb85, 1 | BRF_ESS | BRF_PRG },  //  1
	{ "073.4j",     0x10000, 0x562ed64f, 1 | BRF_ESS | BRF_PRG },  //  2
	{ "074.4h",     0x10000, 0x1eba0140, 1 | BRF_ESS | BRF_PRG },  //  3

	{ "ic6k.bin",   0x00020, 0xc1e427df, 2 | BRF_GRA }, 	       //  4 Color PROM
};

STD_ROM_PICK(mjdiplob)
STD_ROM_FN(mjdiplob)

static int mjdiplobInit()
{
	int nRet;

	nRet = DrvInit();

	memcpy (Rom + 0x10000, Rom + 0x00000, 0x50000);

	return nRet;
}

struct BurnDriver BurnDrvmjdiplob = {
	"mjdiplob", NULL, NULL, "1987",
	"Mahjong Diplomat [BET] (Japan)\0", NULL, "Dynax", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_MISC,
	NULL, mjdiplobRomInfo, mjdiplobRomName, royalmahInputInfo, NULL,
	mjdiplobInit, DrvExit, DrvFrame, DrvDraw, NULL, NULL,
	256, 256, 1, 1
};



// Tonton [BET] (Japan)

static struct BurnRomInfo tontonbRomDesc[] = {
	{ "091.5e",   	0x10000, 0xd8d67b59, 1 | BRF_ESS | BRF_PRG },  //  0 Z80 Code
	// bank switched ROMs follow
	{ "093.5b",   	0x10000, 0x24b6be55, 1 | BRF_ESS | BRF_PRG },  //  1
	{ "092.5c",   	0x10000, 0x7ff2738b, 1 | BRF_ESS | BRF_PRG },  //  2

	{ "ic6k.bin",   0x00020, 0x97e1defe, 2 | BRF_GRA }, 	       //  3 Color PROM
};

STD_ROM_PICK(tontonb)
STD_ROM_FN(tontonb)

static int tontonbInit()
{
	int nRet;

	nRet = DrvInit();

	memcpy (Rom + 0x50000, Rom + 0x20000, 0x10000);
	memcpy (Rom + 0x30000, Rom + 0x10000, 0x10000);
	memcpy (Rom + 0x10000, Rom + 0x00000, 0x10000);

	return nRet;
}

struct BurnDriver BurnDrvtontonb = {
	"tontonb", NULL, NULL, "1987",
	"Tonton [BET] (Japan)\0", NULL, "Dynax", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_MISC,
	NULL, tontonbRomInfo, tontonbRomName, royalmahInputInfo, NULL,
	tontonbInit, DrvExit, DrvFrame, DrvDraw, NULL, NULL,
	256, 256, 1, 1
};


// Mahjong Studio 101 [BET] (Japan)

static struct BurnRomInfo majs101bRomDesc[] = {
	{ "171.3e",     0x10000, 0xfa3c553b, 1 | BRF_ESS | BRF_PRG },  //  0 Z80 Code
	// bank switched ROMs follow

	{ "172.3f",     0x20000, 0x7da39a63, 1 | BRF_ESS | BRF_PRG },  //  1
	{ "173.3h",     0x20000, 0x7a9e71ae, 1 | BRF_ESS | BRF_PRG },  //  2
	{ "174.3j",     0x10000, 0x972c2cc9, 1 | BRF_ESS | BRF_PRG },  //  3

	{ "ic6k.bin",   0x00020, 0xc1e427df, 2 | BRF_GRA }, 	       //  4 Color PROM
};

STD_ROM_PICK(majs101b)
STD_ROM_FN(majs101b)

static int majs101bInit()
{
	int nRet;

	nRet = DrvInit();

	memcpy (Rom + 0x20000, Rom + 0x00000, 0x60000);

	return nRet;
}

struct BurnDriver BurnDrvmajs101b = {
	"majs101b", NULL, NULL, "1988",
	"Mahjong Studio 101 [BET] (Japan)\0", NULL, "Dynax", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_MISC,
	NULL, majs101bRomInfo, majs101bRomName, royalmahInputInfo, NULL,
	majs101bInit, DrvExit, DrvFrame, DrvDraw, NULL, NULL,
	256, 256, 1, 1
};


// Mahjong Derringer (Japan)

static struct BurnRomInfo mjderngrRomDesc[] = {
	{ "2201.1a",    0x10000, 0x54ec531d, 1 | BRF_ESS | BRF_PRG },  //  0 Z80 Code
	// bank switched ROMs follow
	{ "2202.1b",    0x10000, 0xedcf97f2, 1 | BRF_ESS | BRF_PRG },  //  1
	{ "2203.1d",    0x10000, 0xa33368c0, 1 | BRF_ESS | BRF_PRG },  //  2
	{ "2204.1e",    0x20000, 0xed5fde4b, 1 | BRF_ESS | BRF_PRG },  //  3
	{ "2205.1f",    0x20000, 0xcfb8075d, 1 | BRF_ESS | BRF_PRG },  //  4

	{ "ic3g.bin",   0x00200, 0xd43f4c7c, 2 | BRF_GRA }, 	       //  5 Color PROMs
	{ "ic4g.bin",   0x00200, 0x30cf7831, 2 | BRF_GRA }, 	       //  6
};

STD_ROM_PICK(mjderngr)
STD_ROM_FN(mjderngr)

static void mjderngr_palette_init()
{
#define pal5bit(bits) ((bits << 3) | (bits >> 2))

	for (int i = 0; i < 0x400 / 2; i++)
	{
		unsigned short data = (Prom[i] << 8) | Prom[i + 0x200];

		// the bits are in reverse order
		unsigned char r = BITSWAP08((data >>  0) & 0x1f,7,6,5,0,1,2,3,4 );
		unsigned char g = BITSWAP08((data >>  5) & 0x1f,7,6,5,0,1,2,3,4 );
		unsigned char b = BITSWAP08((data >> 10) & 0x1f,7,6,5,0,1,2,3,4 );

		palette[i] = (pal5bit(r) << 16) | (pal5bit(g) << 8) | pal5bit(b);
	}
}

static int mjderngrInit()
{
	int nRet;

	mjderngr = 1;

	nRet = DrvInit();

	memcpy (Rom + 0x70000, Rom + 0x30000, 0x40000);
	memcpy (Rom + 0x50000, Rom + 0x20000, 0x10000);
	memcpy (Rom + 0x30000, Rom + 0x10000, 0x10000);
	memcpy (Rom + 0x10000, Rom + 0x08000, 0x08000);

	mjderngr_palette_init();

	return nRet;
}

struct BurnDriver BurnDrvmjderngr = {
	"mjderngr", NULL, NULL, "1989",
	"Mahjong Derringer (Japan)\0", NULL, "Dynax", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_MISC,
	NULL, mjderngrRomInfo, mjderngrRomName, royalmahInputInfo, NULL,
	mjderngrInit, DrvExit, DrvFrame, DrvDraw, NULL, NULL,
	256, 256, 1, 1
};


// Almond Pinky [BET] (Japan)

static struct BurnRomInfo mjapinkyRomDesc[] = {
	{ "141.4d",        0x10000, 0x0c4fb83a, 1 | BRF_ESS | BRF_PRG },  //  0 Z80 Code
	// bank switched ROMs follow
	{ "142.4e",        0x10000, 0x129806f0, 1 | BRF_ESS | BRF_PRG },  //  1
	{ "143.4f",        0x10000, 0x3d0bc452, 1 | BRF_ESS | BRF_PRG },  //  2
	{ "144.4h",        0x10000, 0x24509a18, 1 | BRF_ESS | BRF_PRG },  //  3
	{ "145.4j",        0x10000, 0xfea3375a, 1 | BRF_ESS | BRF_PRG },  //  4
	{ "146.4k",        0x10000, 0xbe27a9b9, 1 | BRF_ESS | BRF_PRG },  //  5

	{ "18s030n.clr",   0x00020, 0x5736d0aa, 2 | BRF_GRA }, 	          //  6 Color PROM
};

STD_ROM_PICK(mjapinky)
STD_ROM_FN(mjapinky)

static int mjapinkyInit()
{
	int nRet;

	nRet = DrvInit();

	memcpy (Rom + 0x10000, Rom + 0x00000, 0x60000);

	return nRet;
}

struct BurnDriver BurnDrvmjapinky = {
	"mjapinky", NULL, NULL, "1988",
	"Almond Pinky [BET] (Japan)\0", NULL, "Dynax", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_MISC,
	NULL, mjapinkyRomInfo, mjapinkyRomName, royalmahInputInfo, NULL,
	mjapinkyInit, DrvExit, DrvFrame, DrvDraw, NULL, NULL,
	256, 256, 1, 1
};


//-----------------------------------------------------------------------------------------------
// These all use a tmp90841 (z80-based) cpu, so they're non-working for now
//


// Mahjong If...? [BET]

static struct BurnRomInfo mjifbRomDesc[] = {
	{ "2911.1b",    0x10000, 0x138a31a1, 1 | BRF_ESS | BRF_PRG },  //  0 tmp90841 Code
	// bank switched ROMs follow
	{ "2903.1d",    0x20000, 0x90c44965, 1 | BRF_ESS | BRF_PRG },  //  1
	{ "2906.1g",    0x20000, 0xad469345, 1 | BRF_ESS | BRF_PRG },  //  2
	{ "2904.1e",    0x20000, 0x2791abfa, 1 | BRF_ESS | BRF_PRG },  //  3
	{ "2905.1f",    0x20000, 0xb7a73cf7, 1 | BRF_ESS | BRF_PRG },  //  4
	{ "2902.1c",    0x10000, 0x0ce02a98, 1 | BRF_ESS | BRF_PRG },  //  5

	{ "d29-2.4d",   0x00200, 0x78252f6a, 2 | BRF_GRA }, 	       //  6 Color PROMs
	{ "d29-1.4c",   0x00200, 0x4aaec8cf, 2 | BRF_GRA }, 	       //  7
};

STD_ROM_PICK(mjifb)
STD_ROM_FN(mjifb)

static int tmp90841Init()
{
	return 1;
}

struct BurnDriverX BurnDrvmjifb = {
	"mjifb", NULL, NULL, "1990",
	"Mahjong If...? [BET]\0", NULL, "Dynax", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	0, 2, HARDWARE_MISC_MISC,
	NULL, mjifbRomInfo, mjifbRomName, NULL, NULL,
	tmp90841Init, DrvExit, DrvFrame, DrvDraw, NULL, NULL,
	256, 256, 1, 1
};


// Janputer '96 (Japan)

static struct BurnRomInfo janptr96RomDesc[] = {
	{ "503x-1.1h", 0x40000, 0x39914ecd, 1 | BRF_ESS | BRF_PRG },  //  0 tmp90841 Code
	// bank switched ROMs follow
	{ "503x-2.1g", 0x80000, 0xd4b1ed79, 1 | BRF_ESS | BRF_PRG },  //  1
	{ "503x-3.1f", 0x80000, 0x9ba4deb0, 1 | BRF_ESS | BRF_PRG },  //  2
	{ "503x-4.1e", 0x80000, 0xe266ca0b, 1 | BRF_ESS | BRF_PRG },  //  3

	{ "ns503b.3h", 0x00200, 0x3b2a6b12, 2 | BRF_GRA }, 	      //  4 Color PROMs
	{ "ns503a.3j", 0x00200, 0xfe49b2f0, 2 | BRF_GRA }, 	      //  5
};

STD_ROM_PICK(janptr96)
STD_ROM_FN(janptr96)

struct BurnDriverX BurnDrvjanptr96 = {
	"janptr96", NULL, NULL, "1996",
	"Janputer '96 (Japan)\0", NULL, "Dynax", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	0, 2, HARDWARE_MISC_MISC,
	NULL, janptr96RomInfo, janptr96RomName, NULL, NULL,
	tmp90841Init, DrvExit, DrvFrame, DrvDraw, NULL, NULL,
	256, 256, 1, 1
};


// Mahjong Tensinhai (Japan)

static struct BurnRomInfo mjtensinRomDesc[] = {
	{ "1001.5e",   0x80000, 0x960e1fe9, 1 | BRF_ESS | BRF_PRG },  //  0 tmp90841 Code
	// bank switched ROMs follow
	{ "1002.4e",   0x80000, 0x240eb7af, 1 | BRF_ESS | BRF_PRG },  //  1
	{ "1003.3e",   0x80000, 0x876081bf, 1 | BRF_ESS | BRF_PRG },  //  2

	{ "d100-2.7e", 0x00200, 0x6edeed23, 2 | BRF_GRA }, 	      //  4 Color Proms
	{ "d100-1.6e", 0x00200, 0x88befd59, 2 | BRF_GRA }, 	      //  5
};

STD_ROM_PICK(mjtensin)
STD_ROM_FN(mjtensin)

struct BurnDriverX BurnDrvmjtensin = {
	"mjtensin", NULL, NULL, "1995",
	"Mahjong Tensinhai (Japan)\0", NULL, "Dynax", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	0, 2, HARDWARE_MISC_MISC,
	NULL, mjtensinRomInfo, mjtensinRomName, NULL, NULL,
	tmp90841Init, DrvExit, DrvFrame, DrvDraw, NULL, NULL,
	256, 256, 1, 1
};


// Mahjong Cafe Time

static struct BurnRomInfo cafetimeRomDesc[] = {
	{ "6301.2e",  0x40000, 0x1fc10e7c, 1 | BRF_ESS | BRF_PRG },  //  0 tmp90841 Code
	// bank switched ROMs follow
	{ "6302.3e",  0x80000, 0x02bbdf78, 1 | BRF_ESS | BRF_PRG },  //  1
	{ "6303.5e",  0x80000, 0x0e71eea8, 1 | BRF_ESS | BRF_PRG },  //  2
	{ "6304.6e",  0x80000, 0x53c581d6, 1 | BRF_ESS | BRF_PRG },  //  3

	{ "d63-2.8f", 0x00200, 0xaf735b42, 2 | BRF_GRA }, 	     //  4 Color Proms
	{ "d63-1.7f", 0x00200, 0xe7410136, 2 | BRF_GRA }, 	     //  5
};

STD_ROM_PICK(cafetime)
STD_ROM_FN(cafetime)

struct BurnDriverX BurnDrvcafetime = {
	"cafetime", NULL, NULL, "1992",
	"Mahjong Cafe Time\0", NULL, "Dynax", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	0, 2, HARDWARE_MISC_MISC,
	NULL, cafetimeRomInfo, cafetimeRomName, NULL, NULL,
	tmp90841Init, DrvExit, DrvFrame, DrvDraw, NULL, NULL,
	256, 256, 1, 1
};


// Mahjong Cafe Doll (Japan)

static struct BurnRomInfo cafedollRomDesc[] = {
	{ "76xx.tmp90841",   0x02000, 0,          1 | BRF_PRG | BRF_NODUMP }, //  0 tmp90841 Code
	{ "7601",            0x80000, 0x20c80ad9, 1 | BRF_PRG | BRF_ESS },    //  1
	// bank switched ROMs follow
	{ "7602",            0x80000, 0xf472960c, 1 | BRF_PRG | BRF_ESS },    //  2
	{ "7603",            0x80000, 0xc4293019, 1 | BRF_PRG | BRF_ESS },    //  3

	{ "d76-2_82s147.9f", 0x00200, 0x9c1d0512, 2 | BRF_GRA }, 	      //  4 Color Proms
	{ "d76-1_82s147.7f", 0x00200, 0x9a75349c, 2 | BRF_GRA }, 	      //  5
};

STD_ROM_PICK(cafedoll)
STD_ROM_FN(cafedoll)

struct BurnDriverX BurnDrvcafedoll = {
	"cafedoll", NULL, NULL, "1992",
	"Mahjong Cafe Doll (Japan)\0", NULL, "Dynax", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	0, 2, HARDWARE_MISC_MISC,
	NULL, cafedollRomInfo, cafedollRomName, NULL, NULL,
	tmp90841Init, DrvExit, DrvFrame, DrvDraw, NULL, NULL,
	256, 256, 1, 1
};



// Mahjong Vegas (Japan)

static struct BurnRomInfo mjvegasRomDesc[] = {
	{ "50xx.tmp90841",   0x02000, 0,          1 | BRF_PRG | BRF_NODUMP }, //  0 tmp90841 Code
	{ "5001a.1b",        0x20000, 0x91859a47, 1 | BRF_PRG | BRF_ESS },    //  1
	// bank switched ROMs follow
	{ "5002.1d",         0x80000, 0x016c0a32, 1 | BRF_PRG | BRF_ESS },    //  2
	{ "5003.1e",         0x20000, 0x5323cc85, 1 | BRF_PRG | BRF_ESS },    //  3

	{ "d50-2_82s147.4h", 0x00200, 0x3c960ea2, 2 | BRF_GRA }, 	      //  4 Color Proms
	{ "d50-1_82s147.4g", 0x00200, 0x50c0d0ec, 2 | BRF_GRA }, 	      //  5
};

STD_ROM_PICK(mjvegas)
STD_ROM_FN(mjvegas)

struct BurnDriverX BurnDrvmjvegas = {
	"mjvegas", NULL, NULL, "1991",
	"Mahjong Vegas (Japan)\0", NULL, "Dynax", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	0, 2, HARDWARE_MISC_MISC,
	NULL, mjvegasRomInfo, mjvegasRomName, NULL, NULL,
	tmp90841Init, DrvExit, DrvFrame, DrvDraw, NULL, NULL,
	256, 256, 1, 1
};


// Mahjong Shinkirou Deja Vu (Japan)

static struct BurnRomInfo mjdejavuRomDesc[] = {
	{ "2101.1b",   0x10000, 0xb0426ea7, 1 | BRF_PRG | BRF_ESS },    //  0 tmp90841 Code
	// bank switched ROMs follow
	{ "2103.1d",   0x20000, 0xed5fde4b, 1 | BRF_PRG | BRF_ESS },    //  1
	{ "2104.1e",   0x20000, 0xcfb8075d, 1 | BRF_PRG | BRF_ESS },    //  2
	{ "2102.1c",   0x20000, 0xf461e422, 1 | BRF_PRG | BRF_ESS },    //  3

	{ "82s147.4d", 0x00200, 0xd43f4c7c, 2 | BRF_GRA }, 	        //  4 Color Proms
	{ "82s147.4c", 0x00200, 0x30cf7831, 2 | BRF_GRA }, 	        //  5
};

STD_ROM_PICK(mjdejavu)
STD_ROM_FN(mjdejavu)

struct BurnDriverX BurnDrvmjdejavu = {
	"mjdejavu", NULL, NULL, "1989",
	"Mahjong Shinkirou Deja Vu (Japan)\0", NULL, "Dynax", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	0, 2, HARDWARE_MISC_MISC,
	NULL, mjdejavuRomInfo, mjdejavuRomName, NULL, NULL,
	tmp90841Init, DrvExit, DrvFrame, DrvDraw, NULL, NULL,
	256, 256, 1, 1
};
