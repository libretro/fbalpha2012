// FB Alpha Amazing Adventures of Mr. F. Lea driver module
// Based on MAME driver by Phil Stroffolino

#include "burnint.h"
#include "bitswap.h"
#include "driver.h"
extern "C" {
#include "ay8910.h"
}

static unsigned char *Mem, *Rom0, *Rom1, *Ram, *Gfx0, *Gfx1;
static int *Palette;
static unsigned char DrvJoy1[8], DrvJoy2[8], DrvDips[2], DrvReset;

static short *pFMBuffer, *pAY8910Buffer[9];

static int mrflea_io;
static int mrflea_main;
static int mrflea_status;
static int mrflea_select[4];
static unsigned char mrflea_gfx_bank;

static struct BurnInputInfo DrvInputList[] = {
	{"Start 1"  ,     BIT_DIGITAL  , DrvJoy1 + 3,	"p1 start" },
	{"Start 2"  ,     BIT_DIGITAL  , DrvJoy1 + 2,	"p2 start" },
	{"P1 Coin"      , BIT_DIGITAL  , DrvJoy2 + 2,	"p1 coin"  },

	{"P1 Right"     , BIT_DIGITAL  , DrvJoy1 + 4, 	"p1 right" },
	{"P1 Left"      , BIT_DIGITAL  , DrvJoy1 + 5, 	"p1 left"  },
	{"P1 Up",	  BIT_DIGITAL,   DrvJoy1 + 6,   "p1 up", },
	{"P1 Down",	  BIT_DIGITAL,   DrvJoy1 + 7,   "p1 down", },
	{"P1 Button 1"  , BIT_DIGITAL  , DrvJoy1 + 1,	"p1 fire 1"},

	{"Reset",	  BIT_DIGITAL  , &DrvReset,	"reset"    },
	{"Dip 1",	  BIT_DIPSWITCH, DrvDips+0,	"dip"	   },
	{"Dip 2",	  BIT_DIPSWITCH, DrvDips+1,	"dip"	   },
};

STDINPUTINFO(Drv)

static struct BurnDIPInfo DrvDIPList[]=
{
	// Default Values
	{0x09, 0xff, 0xff, 0xff, NULL                     },

	{0   , 0xfe, 0   , 4   , "Bonus"                  },
	{0x09, 0x01, 0x03, 0x03, "A"       		  },
	{0x09, 0x01, 0x03, 0x02, "B"       		  },
	{0x09, 0x01, 0x03, 0x01, "C"       		  },
	{0x09, 0x01, 0x03, 0x00, "D"       		  },

	// Default Values
	{0x0a, 0xff, 0xff, 0xff, NULL                     },

	{0   , 0xfe, 0   , 4   , "Coinage" 	          },
	{0x0a, 0x01, 0x03, 0x02, "2C 1C"       		  },
	{0x0a, 0x01, 0x03, 0x03, "1C 1C"       		  },
	{0x0a, 0x01, 0x03, 0x00, "2C 3C"       		  },
	{0x0a, 0x01, 0x03, 0x01, "1C 2C"       		  },

	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x0a, 0x01, 0x0c, 0x0c, "3"       		  },
	{0x0a, 0x01, 0x0c, 0x08, "4"       		  },
	{0x0a, 0x01, 0x0c, 0x04, "5"       		  },
	{0x0a, 0x01, 0x0c, 0x00, "7"       		  },

	{0   , 0xfe, 0   , 4   , "Difficulty" 	          },
	{0x0a, 0x01, 0x30, 0x30, "Easy"       		  },
	{0x0a, 0x01, 0x30, 0x20, "Medium"       	  },
	{0x0a, 0x01, 0x30, 0x10, "Hard"       		  },
	{0x0a, 0x01, 0x30, 0x00, "Hardest"     		  },
};

STDDIPINFO(Drv)

void __fastcall mrflea_write(unsigned short a, unsigned char d)
{
	if (a >= 0xe000 && a <= 0xe7ff) // video ram
	{
		Ram[0xe000 + (a & 0x3ff)] = d;
		Ram[0xe400 + (a & 0x3ff)] = (a >> 10) & 1;
		return;
	}

	if (a >= 0xe800 && a <= 0xe83f) // palette ram
	{
		Ram[a] = d;

		Palette[(a >> 1) & 0x1f]  = ((Ram[a | 1] & 0x0f) | (Ram[a | 1] << 4)) << 16;
		Palette[(a >> 1) & 0x1f] |= ((Ram[a &~1] & 0xf0) | (Ram[a &~1] >> 4)) <<  8;
		Palette[(a >> 1) & 0x1f] |= ((Ram[a &~1] & 0x0f) | (Ram[a &~1] << 4)) <<  0;

		return;
	}

	if (a >= 0xec00 && a <= 0xecff) // sprite ram
	{
		if (a & 2) { // tile number
			Ram[a | 1] = a & 1;
			a &= 0xfffe;
		}

		Ram[a] = d;

		return;
	}
}


void _fastcall mrflea_out_port(unsigned short a, unsigned char data)
{
	switch (a & 0xff)
	{
		case 0x00: // watchdog?
		case 0x43:
		break;

		case 0x40:
		{
			mrflea_status |= 0x08;
			mrflea_io = data;

			ZetClose();
			ZetOpen(1);
			ZetRaiseIrq(0);
			ZetClose();
			ZetOpen(0);
		}
		break;

		case 0x60:
			mrflea_gfx_bank = data;
		break;
	}
}

unsigned char _fastcall mrflea_in_port(unsigned short a)
{
	switch (a & 0xff)
	{
		case 0x41:
			mrflea_status &= ~0x01;
			return mrflea_main;

		break;

		case 0x42:
			return (mrflea_status ^ 0x08);
		break;
	}

	return 0;
}

void _fastcall mrflea_cpu1_out_port(unsigned short a, unsigned char data)
{
	switch (a & 0xff)
	{
		case 0x00: // watchdog
		case 0x10:
		case 0x11:
		case 0x23:
		break;

		case 0x21:
			mrflea_status |= 0x01;
			mrflea_main = data;
		break;

		case 0x40:
			AY8910Write(0, 0, mrflea_select[0]);
			AY8910Write(0, 1, data);
		break;

		case 0x42:
		break;

		case 0x44:
			AY8910Write(1, 0, mrflea_select[2]);
			AY8910Write(1, 1, data);
		break;

		case 0x46:
			AY8910Write(2, 0, mrflea_select[3]);
			AY8910Write(2, 1, data);
		break;

		case 0x41:
		case 0x43:
		case 0x45:
		case 0x47:
			mrflea_select[(a >> 1) & 3] = data;
		break;
	}
}

unsigned char __fastcall mrflea_cpu1_in_port(unsigned short a)
{
	unsigned char ret = 0;

	switch (a & 0xff)
	{
		case 0x10:
			if (mrflea_status & 0x08) return 0x00;
			return 0x01;
		break;

		case 0x20:
			mrflea_status &= ~0x08;
			return mrflea_io;

		case 0x22:
			return (mrflea_status ^ 0x01);

		case 0x40:
			if (mrflea_select[0] == 0x0f) {
				for (int i = 0; i < 8; i++) {
					ret |= DrvJoy1[i] << i;
				}
				return ~ret;
			}
			if (mrflea_select[0] == 0x0e) {
				for (int i = 0; i < 8; i++) {
					ret |= DrvJoy2[i] << i;
				}
				return ~ret;
			}
			return 0;

		case 0x44:
			if (mrflea_select[2] == 0x0f) return 0xff;
			if (mrflea_select[2] == 0x0e) return 0xff;
			return 0;

		case 0x42:
		case 0x46:
			return 0;
	}

	return 0;
}

static int DrvDoReset()
{
	memset (Ram, 0, 0x10000);

	memset (mrflea_select, 0, sizeof(int) * 4);

	mrflea_io = 0;
	mrflea_main = 0;
	mrflea_status = 0;
	mrflea_gfx_bank = 0;

	DrvReset = 0;

	for (int i = 0; i < 2; i++) {
		ZetOpen(i);
		ZetReset();
		ZetClose();
	}

	for (int i = 0; i < 3; i++) {
		AY8910Reset(i);
	}

	return 0;
}

static int convert_gfx()
{
	unsigned char *tmp = (unsigned char*)malloc(0x10000);
	if (tmp == NULL) {
		return 1;
	}

	memcpy (tmp, Gfx0, 0x10000);

	for (int i = 0; i < 0x20000; i++) // sprites
	{
		Gfx0[i^0x07]  = ((tmp[0x0000 + (i >> 3)] >> (i & 7)) & 1) << 3;
		Gfx0[i^0x07] |= ((tmp[0x4000 + (i >> 3)] >> (i & 7)) & 1) << 2;
		Gfx0[i^0x07] |= ((tmp[0x8000 + (i >> 3)] >> (i & 7)) & 1) << 1;
		Gfx0[i^0x07] |= ((tmp[0xc000 + (i >> 3)] >> (i & 7)) & 1);
	}

	memcpy (tmp, Gfx1, 0x10000);

	for (int i = 0; i < 0x20000; i+=2) // chars
	{
		Gfx1[i + 0] = (tmp[i>>1] >> 4) & 0x0f;
		Gfx1[i + 1] = (tmp[i>>1] >> 0) & 0x0f;
	}

	free (tmp);

	return 0;
}


static int DrvInit()
{
	Mem = (unsigned char*)malloc(0x70000 + 128);
	if (Mem == NULL) {
		return 1;
	}

	pFMBuffer = (short*)malloc(nBurnSoundLen * 9 * sizeof(short));
	if (pFMBuffer == NULL) {
		return 1;
	}

	memset (Mem, 0, 0x70080);
	
	Rom0 = Mem + 0x000000;
	Rom1 = Mem + 0x010000;
	Ram  = Mem + 0x020000;
	Gfx0 = Mem + 0x030000;
	Gfx1 = Mem + 0x050000;
	Palette = (int*)(Mem + 0x70000);

	{
		for (int i = 0; i < 6; i++) {
			if (BurnLoadRom(Rom0 + i * 0x2000, i,      1)) return 1;
		}

		if (BurnLoadRom(Rom1 + 0x0000, 6, 1)) return 1;
		if (BurnLoadRom(Rom1 + 0x2000, 7, 1)) return 1;
		if (BurnLoadRom(Rom1 + 0x3000, 8, 1)) return 1;

		for (int i = 0; i < 8; i++) {
			if (BurnLoadRom(Gfx0 + i * 0x2000, i +  9, 1)) return 1;
			if (BurnLoadRom(Gfx1 + i * 0x2000, i + 17, 1)) return 1;
		}

		if (convert_gfx()) return 1;
	}

	ZetInit(2);
	ZetOpen(0);
	ZetSetInHandler(mrflea_in_port);
	ZetSetOutHandler(mrflea_out_port);
	ZetSetWriteHandler(mrflea_write);
	ZetMapArea(0x0000, 0xbfff, 0, Rom0 + 0x0000);
	ZetMapArea(0x0000, 0xbfff, 2, Rom0 + 0x0000);
	ZetMapArea(0xc000, 0xcfff, 0, Ram  + 0xc000);
	ZetMapArea(0xc000, 0xcfff, 1, Ram  + 0xc000);
	ZetMapArea(0xc000, 0xcfff, 2, Ram  + 0xc000);
	ZetMemEnd();
	ZetClose();

	ZetOpen(1);
	ZetSetInHandler(mrflea_cpu1_in_port);
	ZetSetOutHandler(mrflea_cpu1_out_port);
	ZetMapArea(0x0000, 0x3fff, 0, Rom1 + 0x0000);
	ZetMapArea(0x0000, 0x3fff, 2, Rom1 + 0x0000);
	ZetMapArea(0x8000, 0x80ff, 0, Ram  + 0x8000);
	ZetMapArea(0x8000, 0x80ff, 1, Ram  + 0x8000);
	ZetMapArea(0x8000, 0x80ff, 2, Ram  + 0x8000);
	ZetMapArea(0x9000, 0x905a, 0, Ram  + 0x9000);
	ZetMapArea(0x9000, 0x905a, 1, Ram  + 0x9000);
	ZetMapArea(0x9000, 0x905a, 2, Ram  + 0x9000);
	ZetMemEnd();
	ZetClose();

	for (int i = 0; i < 9; i++)
		pAY8910Buffer[i] = pFMBuffer + nBurnSoundLen * i;

	AY8910Init(0, 2000000, nBurnSoundRate, NULL, NULL, NULL, NULL);
	AY8910Init(1, 2000000, nBurnSoundRate, NULL, NULL, NULL, NULL);
	AY8910Init(2, 2000000, nBurnSoundRate, NULL, NULL, NULL, NULL);

	DrvDoReset();

	return 0;
}

static int DrvExit()
{
	ZetExit();
	free (Mem);

	free (pFMBuffer);

	Rom0 = Rom1 = Ram = Gfx0 = Gfx1 = NULL;

	pFMBuffer = NULL;
	for (int i = 0; i < 9; i++)
		pAY8910Buffer[i] = NULL;

	Palette = NULL;

	return 0;
}


static int DrvDraw()
{
	int base = ((mrflea_gfx_bank & 0x04) << 8) | ((mrflea_gfx_bank & 0x10) << 5);

	for (int i = 0; i < 0x400; i++)
	{
		int sy = (i >> 2) & 0xf8;
		int sx = (i << 3) & 0xf8;

		if (sy >= 0xf8) continue;

		int code = base + Ram[0xe000 + i] + (Ram[0xe400 + i] << 8);

		unsigned char *src = Gfx1 + code * 64;

		for (int y = sy; y < sy + 8; y++)
		{
			for (int x = sx; x < sx + 8; x++, src++)
			{
				int pxl = Palette[*src];

				PutPix(pBurnDraw + ((y << 8) | x) * nBurnBpp, BurnHighCol(pxl >> 16, pxl >> 8, pxl, 0));
			}
		}
	}

	for (int i = 0; i < 0x100; i+=4)
	{
		int sx = Ram[0xec00 + i + 1];
		int sy = Ram[0xec00 + i + 0] - 13;

		int code = (Ram[0xec00 + i + 2] | (Ram[0xec00 + i + 3] << 8)) << 8;

		unsigned char *src = Gfx0 + code;

		for (int y = sy; y < sy + 16; y++)
		{
			for (int x = sx; x < sx + 16; x++, src++)
			{
				if (!*src || x >= 0xff || y >= 0xf8 || y < 0) continue;

				int pxl = Palette[0x10|*src];

				PutPix(pBurnDraw + ((y << 8) | x) * nBurnBpp, BurnHighCol(pxl >> 16, pxl >> 8, pxl, 0));
			}
		}
	}

	return 0;
}

static int DrvFrame()
{
	if (DrvReset) {
		DrvDoReset();
	}

	ZetNewFrame();

	int nInterleave = 200;
	int nSoundBufferPos = 0;

	int nCyclesSegment;
	int nCyclesDone[2], nCyclesTotal[2];

	nCyclesTotal[0] = 4000000 / 60;
	nCyclesTotal[1] = 6000000 / 60;
	nCyclesDone[0] = nCyclesDone[1] = 0;

	for (int i = 0; i < nInterleave; i++) {
		int nCurrentCPU, nNext;

		// Run Z80 #0
		nCurrentCPU = 0;
		ZetOpen(nCurrentCPU);
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		nCyclesDone[nCurrentCPU] += ZetRun(nCyclesSegment);
		if (i == (nInterleave - 1)) ZetRaiseIrq(0);
		ZetClose();

		// Run Z80 #1
		nCurrentCPU = 1;
		ZetOpen(nCurrentCPU);
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		nCyclesSegment = ZetRun(nCyclesSegment);
		nCyclesDone[nCurrentCPU] += nCyclesSegment;
		if ((mrflea_status&0x08) || i == (nInterleave - 1)) ZetRaiseIrq(0);
		ZetClose();

		// Render Sound Segment
		if (pBurnSoundOut) {
			int nSample;
			int nSegmentLength = nBurnSoundLen - nSoundBufferPos;
			short* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
			AY8910Update(0, &pAY8910Buffer[0], nSegmentLength);
			AY8910Update(1, &pAY8910Buffer[3], nSegmentLength);
			AY8910Update(2, &pAY8910Buffer[6], nSegmentLength);
			for (int n = 0; n < nSegmentLength; n++) {
				nSample  = pAY8910Buffer[0][n];
				nSample += pAY8910Buffer[1][n];
				nSample += pAY8910Buffer[2][n];
				nSample += pAY8910Buffer[3][n];
				nSample += pAY8910Buffer[4][n];
				nSample += pAY8910Buffer[5][n];
				nSample += pAY8910Buffer[6][n];
				nSample += pAY8910Buffer[7][n];
				nSample += pAY8910Buffer[8][n];

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
			nSoundBufferPos += nSegmentLength;
		}
	}

	// Make sure the buffer is entirely filled.
	if (pBurnSoundOut) {
		int nSample;
		int nSegmentLength = nBurnSoundLen - nSoundBufferPos;
		short* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
		if (nSegmentLength) {
			AY8910Update(0, &pAY8910Buffer[0], nSegmentLength);
			AY8910Update(1, &pAY8910Buffer[3], nSegmentLength);
			AY8910Update(2, &pAY8910Buffer[6], nSegmentLength);
			for (int n = 0; n < nSegmentLength; n++) {
				nSample  = pAY8910Buffer[0][n];
				nSample += pAY8910Buffer[1][n];
				nSample += pAY8910Buffer[2][n];
				nSample += pAY8910Buffer[3][n];
				nSample += pAY8910Buffer[4][n];
				nSample += pAY8910Buffer[5][n];
				nSample += pAY8910Buffer[6][n];
				nSample += pAY8910Buffer[7][n];
				nSample += pAY8910Buffer[8][n];

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

	if (pBurnDraw) {
		DrvDraw();
	}

	return 0;
}


static int DrvScan(int nAction,int *pnMin)
{
	struct BurnArea ba;

	if (pnMin) {						// Return minimum compatible version
		*pnMin = 0x029521;
	}

	if (nAction & ACB_VOLATILE) {		// Scan volatile ram		
		memset(&ba, 0, sizeof(ba));
		ba.Data	  = Ram;
		ba.nLen	  = 0x10000;
		ba.szName = "All Ram";
		BurnAcb(&ba);

		memset(&ba, 0, sizeof(ba));
		ba.Data	  = (unsigned char*)Palette;
		ba.nLen	  = 0x80 * sizeof(int);
		ba.szName = "Palette";
		BurnAcb(&ba);

		ZetScan(nAction);			// Scan Z80

		AY8910Scan(nAction, pnMin);		// Scan AY8910

		// Scan critical driver variables
		SCAN_VAR(mrflea_io);
		SCAN_VAR(mrflea_main);
		SCAN_VAR(mrflea_status);		
		SCAN_VAR(mrflea_gfx_bank);
		SCAN_VAR(mrflea_select[0]);
		SCAN_VAR(mrflea_select[1]);
		SCAN_VAR(mrflea_select[2]);
		SCAN_VAR(mrflea_select[3]);
	}

	return 0;
}


// The Amazing Adventures of Mr. F. Lea

static struct BurnRomInfo mrfleaRomDesc[] = {
	{ "cpu_d1",	0x2000, 0xd286217c, 1 | BRF_ESS | BRF_PRG }, //  0 Z80 0 Code
	{ "cpu_d3",	0x2000, 0x95cf94bc, 1 | BRF_ESS | BRF_PRG }, //  1
	{ "cpu_d5",	0x2000, 0x466ca77e, 1 | BRF_ESS | BRF_PRG }, //  2
	{ "cpu_b1",	0x2000, 0x721477d6, 1 | BRF_ESS | BRF_PRG }, //  3
	{ "cpu_b3",	0x2000, 0xf55b01e4, 1 | BRF_ESS | BRF_PRG }, //  4
	{ "cpu_b5",	0x2000, 0x79f560aa, 1 | BRF_ESS | BRF_PRG }, //  5

	{ "io_a11",	0x1000, 0x7a20c3ee, 2 | BRF_ESS | BRF_PRG }, //  6 Z80 1 Code
	{ "io_c11",	0x1000, 0x8d26e0c8, 2 | BRF_ESS | BRF_PRG }, //  7
	{ "io_d11",	0x1000, 0xabd9afc0, 2 | BRF_ESS | BRF_PRG }, //  8

	{ "vd_l10",	0x2000, 0x48b2adf9, 3 | BRF_GRA },	     //  9 Sprites
	{ "vd_l11",	0x2000, 0x2ff168c0, 3 | BRF_GRA },	     // 10
	{ "vd_l6",	0x2000, 0x100158ca, 3 | BRF_GRA },	     // 11
	{ "vd_l7",	0x2000, 0x34501577, 3 | BRF_GRA },	     // 12
	{ "vd_j10",	0x2000, 0x3f29b8c3, 3 | BRF_GRA },	     // 13
	{ "vd_j11",	0x2000, 0x39380bea, 3 | BRF_GRA },	     // 14
	{ "vd_j6",	0x2000, 0x2b4b110e, 3 | BRF_GRA },	     // 15
	{ "vd_j7",	0x2000, 0x3a3c8b1e, 3 | BRF_GRA },	     // 16

	{ "vd_k1",	0x2000, 0x7540e3a7, 4 | BRF_GRA },	     // 17 Characters
	{ "vd_k2",	0x2000, 0x6c688219, 4 | BRF_GRA },	     // 18
	{ "vd_k3",	0x2000, 0x15e96f3c, 4 | BRF_GRA },	     // 19
	{ "vd_k4",	0x2000, 0xfe5100df, 4 | BRF_GRA },	     // 20
	{ "vd_l1",	0x2000, 0xd1e3d056, 4 | BRF_GRA },	     // 21
	{ "vd_l2",	0x2000, 0x4d7fb925, 4 | BRF_GRA },	     // 22
	{ "vd_l3",	0x2000, 0x6d81588a, 4 | BRF_GRA },	     // 23
	{ "vd_l4",	0x2000, 0x423735a5, 4 | BRF_GRA },	     // 24
};

STD_ROM_PICK(mrflea)
STD_ROM_FN(mrflea)

struct BurnDriver BurnDrvmrflea = {
	"mrflea", NULL, NULL, NULL, "1982",
	"The Amazing Adventures of Mr. F. Lea\0", NULL, "Pacific Novelty", "misc",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 1, HARDWARE_MISC_PRE90S, GBF_PLATFORM, 0,
	NULL, mrfleaRomInfo, mrfleaRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, NULL, 0x80,
	248, 256, 3, 4
};

