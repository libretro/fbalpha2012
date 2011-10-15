/*
 1945K-III
 Oriental, 2000
 
 port to Finalburn Alpha by OopsWare. 2007
 
 */

#include "burnint.h"
#include "msm6295.h"

static unsigned char *Mem = NULL, *MemEnd = NULL;
static unsigned char *RamStart, *RamEnd;

static unsigned char *Rom68K;
static unsigned char *RomBg;
static unsigned char *RomSpr;

static unsigned char *Ram68K;
static unsigned short *RamPal;
static unsigned short *RamSpr0;
static unsigned short *RamSpr1;
static unsigned short *RamBg;

static unsigned short *RamCurPal;

static unsigned char bRecalcPalette = 0;
static unsigned char DrvReset = 0;
static unsigned short scrollx, scrolly;
static unsigned char m6295bank[2];

static unsigned char DrvButton[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static unsigned char DrvJoy1[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static unsigned char DrvJoy2[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static unsigned char DrvInput[8] = {0, 0, 0, 0, 0, 0, 0, 0};

inline static unsigned int CalcCol(unsigned short nColour)
{
	int r, g, b;

	r = (nColour & 0x001F) << 3;	// Red
	r |= r >> 5;
	g = (nColour & 0x03E0) >> 2;  	// Green
	g |= g >> 5;
	b = (nColour & 0x7C00) >> 7;	// Blue
	b |= b >> 5;

	return BurnHighCol(r, g, b, 0);
}

static struct BurnInputInfo _1945kiiiInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvButton + 0,	"p1 coin"},
	{"P1 Start",	BIT_DIGITAL,	DrvButton + 2,	"p1 start"},

	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 up"},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 down"},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 left"},
	{"P1 Right",	BIT_DIGITAL,	DrvJoy1 + 3,	"p1 right"},
	{"P1 Button 1",	BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"},
	{"P1 Button 2",	BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"},
	{"P1 Button 3",	BIT_DIGITAL,	DrvJoy1 + 6,	"p1 fire 3"},
	{"P1 Button 4",	BIT_DIGITAL,	DrvJoy1 + 7,	"p1 fire 4"},

	{"P2 Start",	BIT_DIGITAL,	DrvButton + 3,	"p2 start"},

	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 up"},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 down"},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 left"},
	{"P2 Right",	BIT_DIGITAL,	DrvJoy2 + 3,	"p2 right"},
	{"P2 Button 1",	BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"},
	{"P2 Button 2",	BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"},	
	{"P2 Button 3",	BIT_DIGITAL,	DrvJoy2 + 6,	"p2 fire 3"},
	{"P2 Button 4",	BIT_DIGITAL,	DrvJoy2 + 7,	"p2 fire 4"},		
	
	{"Reset",		BIT_DIGITAL,	&DrvReset,		"reset"},
	{"Dip A",		BIT_DIPSWITCH,	DrvInput + 4,	"dip"},
	{"Dip B",		BIT_DIPSWITCH,	DrvInput + 5,	"dip"},
};

STDINPUTINFO(_1945kiii)

static struct BurnDIPInfo _1945kiiiDIPList[] = {
	// Defaults
	{0x14,	0xFF, 0xFF,	0x10, NULL},
	{0x15,	0xFF, 0xFF,	0x00, NULL},

	// DIP 1
	{0,		0xFE, 0,	8,	  "Coin 1"},
	{0x14,	0x01, 0x07, 0x00, "1 coin 1 credit"},
	{0x14,	0x01, 0x07, 0x01, "2 coins 1 credit"},
	{0x14,	0x01, 0x07, 0x02, "3 coins 1 credit"},
	{0x14,	0x01, 0x07, 0x03, "1 coin 2 credits"},
	{0x14,	0x01, 0x07, 0x04, "Free Play"},
	{0x14,	0x01, 0x07, 0x05, "5 coins 1 credit"},
	{0x14,	0x01, 0x07, 0x06, "4 coins 1 credit"},
	{0x14,	0x01, 0x07, 0x07, "1 coin 3 credits"},
	{0,		0xFE, 0,	4,	  "Difficulty"},
	{0x14,	0x01, 0x18, 0x00, "Hardest"},
	{0x14,	0x01, 0x18, 0x08, "Hard"},
	{0x14,	0x01, 0x18, 0x10, "Normal"},
	{0x14,	0x01, 0x18, 0x18, "Easy"},
	{0,		0xFE, 0,	4,	  "Lives"},
	{0x14,	0x01, 0x60, 0x00, "3"},
	{0x14,	0x01, 0x60, 0x20, "2"},
	{0x14,	0x01, 0x60, 0x40, "4"},
	{0x14,	0x01, 0x60, 0x60, "5"},
	{0,		0xFE, 0,	2,	  "Service"},
	{0x14,	0x01, 0x80, 0x00, "Off"}, 
	{0x14,	0x01, 0x80, 0x80, "On"},
	
	// DIP 2
	{0,		0xFE, 0,	2,	  "Demo sound"},
	{0x15,	0x01, 0x01, 0x00, "Off"},
	{0x15,	0x01, 0x01, 0x01, "On"},
	{0,		0xFE, 0,	2,	  "Allow Continue"},
	{0x15,	0x01, 0x02, 0x00, "Yes"},
	{0x15,	0x01, 0x02, 0x02, "No"},
	
};

STDDIPINFO(_1945kiii)

static struct BurnRomInfo _1945kiiiRomDesc[] = {
	{ "prg-1.u51",	0x080000, 0x6b345f27, BRF_ESS | BRF_PRG },	// 68000 code 
	{ "prg-2.u52", 	0x080000, 0xce09b98c, BRF_ESS | BRF_PRG }, 
	
	{ "m16m-1.u62",	0x200000, 0x0b9a6474, BRF_GRA }, 			// layer 0
	{ "m16m-2.u63",	0x200000, 0x368a8c2e, BRF_GRA },
	
	{ "m16m-3.u61",	0x200000, 0x32fc80dd, BRF_GRA }, 			// layer 1
	
	{ "snd-1.su7",	0x080000, 0xbbb7f0ff, BRF_SND }, 			// sound 1

	{ "snd-2.su4",	0x080000, 0x47e3952e, BRF_SND }, 			// sound 2
};

STD_ROM_PICK(_1945kiii)
STD_ROM_FN(_1945kiii)

static void sndSetBank(unsigned char bank0, unsigned char bank1)
{
	if (bank0 != m6295bank[0])	{
		m6295bank[0] = bank0;
		for (int nChannel = 0; nChannel < 4; nChannel++) {
			MSM6295SampleInfo[0][nChannel] = MSM6295ROM + 0x000000 + 0x040000 * bank0 + (nChannel << 8);
			MSM6295SampleData[0][nChannel] = MSM6295ROM + 0x000000 + 0x040000 * bank0 + (nChannel << 16);
		}
	}
	if (bank1 != m6295bank[1])	{
		m6295bank[1] = bank1;
		for (int nChannel = 0; nChannel < 4; nChannel++) {
			MSM6295SampleInfo[1][nChannel] = MSM6295ROM + 0x080000 + 0x040000 * bank1 + (nChannel << 8);
			MSM6295SampleData[1][nChannel] = MSM6295ROM + 0x080000 + 0x040000 * bank1 + (nChannel << 16);
		}
	}
}

/*
unsigned char __fastcall k1945iiiReadByte(unsigned int sekAddress)
{
	switch (sekAddress) {

		default:
			bprintf(PRINT_NORMAL, _T("Attempt to read byte value of location %x\n"), sekAddress);
	}
	return 0;
}
*/

unsigned short __fastcall k1945iiiReadWord(unsigned int sekAddress)
{
	switch (sekAddress) {
		case 0x400000:
			return ~( DrvInput[0] + (DrvInput[1]<<8) );
		case 0x440000:
			return ~DrvInput[2];
		case 0x480000:
			return ~( DrvInput[4] + (DrvInput[5]<<8) );
			
		case 0x4C0000:
			return MSM6295ReadStatus(0);
		case 0x500000:
			return MSM6295ReadStatus(1);
		
		//default:
		//	bprintf(PRINT_NORMAL, _T("Attempt to read word value of location %x\n"), sekAddress);
	}
	return 0;
}

void __fastcall k1945iiiWriteByte(unsigned int sekAddress, unsigned char byteValue)
{
	switch (sekAddress) {
		case 0x4C0000:
			MSM6295Command(0, byteValue);
			break;
		case 0x500000:
			MSM6295Command(1, byteValue);
			break;

//		default:
//			bprintf(PRINT_NORMAL, _T("Attempt to write byte value %x to location %x\n"), byteValue, sekAddress);
	}
}

void __fastcall k1945iiiWriteWord(unsigned int sekAddress, unsigned short wordValue)
{
	switch (sekAddress) {
		case 0x340000:
			scrollx = wordValue;
			break;
		case 0x380000:
			scrolly = wordValue;
			break;
		case 0x3C0000:
			//bprintf(PRINT_NORMAL, _T("soundbanks write %d %d\n"), (wordValue & 2) >> 1, (wordValue & 4) >> 2);
			sndSetBank((wordValue & 2) >> 1, (wordValue & 4) >> 2);
			break;			

		//default:
		//	bprintf(PRINT_NORMAL, _T("Attempt to write word value %x to location %x\n"), wordValue, sekAddress);
	}
}

/*
void __fastcall k1945iiiWriteBytePalette(unsigned int sekAddress, unsigned char byteValue)
{
	bprintf(PRINT_NORMAL, _T("Palette to write byte value %x to location %x\n"), byteValue, sekAddress);
}
*/

void __fastcall k1945iiiWriteWordPalette(unsigned int sekAddress, unsigned short wordValue)
{
	sekAddress -= 0x200000;
	sekAddress >>= 1;
	RamPal[sekAddress] = wordValue;
	RamCurPal[sekAddress] = CalcCol( wordValue );
}

static int MemIndex()
{
	unsigned char *Next; Next = Mem;
	Rom68K 		= Next; Next += 0x0100000;			// 68000 ROM
	RomBg		= Next; Next += 0x0200000;
	RomSpr		= Next; Next += 0x0400000;
	MSM6295ROM	= Next; Next += 0x0100000;
	
	RamStart	= Next;
	
	Ram68K		= Next; Next += 0x020000;
	RamPal		= (unsigned short *) Next; Next += 0x001000;
	RamSpr0		= (unsigned short *) Next; Next += 0x001000;
	RamSpr1		= (unsigned short *) Next; Next += 0x001000;
	RamBg		= (unsigned short *) Next; Next += 0x001000;
	
	RamEnd		= Next;
	
	RamCurPal	= (unsigned short *) Next; Next += 0x001000;
	
	MemEnd		= Next;
	return 0;
}

static int DrvDoReset()
{
	SekOpen(0);
    SekSetIRQLine(0, SEK_IRQSTATUS_NONE);
	SekReset();
	SekClose();
	
	MSM6295Reset(0);
	MSM6295Reset(1);

	m6295bank[0] = 1;
	m6295bank[1] = 1;
	sndSetBank(0, 0);

	return 0;
}

static int DrvInit()
{
	int nRet;
	
	Mem = NULL;
	MemIndex();
	int nLen = MemEnd - (unsigned char *)0;
	if ((Mem = (unsigned char *)malloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);										// blank all memory
	MemIndex();	
	
	nRet = BurnLoadRom(Rom68K + 0x000000, 0, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Rom68K + 0x000001, 1, 2); if (nRet != 0) return 1;
	
	BurnLoadRom(RomSpr + 0, 2, 2);
	BurnLoadRom(RomSpr + 1, 3, 2);
	
	// decode sprites 
	unsigned char * tmp = RomSpr;
	for (int i=0; i<(0x400000/4); i++) {
		unsigned char c = tmp[2];
		tmp[2] = tmp[1];
		tmp[1] = c;
		tmp += 4;
	}
	
	BurnLoadRom(RomBg, 4, 1);
	
	BurnLoadRom(MSM6295ROM + 0x00000, 5, 1);
	BurnLoadRom(MSM6295ROM + 0x80000, 6, 1);
	
	{
		SekInit(0, 0x68000);										// Allocate 68000
	    SekOpen(0);

		// Map 68000 memory:
		SekMapMemory(Rom68K,		0x000000, 0x0FFFFF, SM_ROM);	// CPU 0 ROM
		SekMapMemory(Ram68K,		0x100000, 0x10FFFF, SM_RAM);	// CPU 0 RAM
		SekMapMemory((unsigned char *)RamPal,
									0x200000, 0x200FFF, SM_ROM);	// palette
		SekMapMemory((unsigned char *)RamSpr0,
									0x240000, 0x240FFF, SM_RAM);	// sprites 0
		SekMapMemory((unsigned char *)RamSpr1,
									0x280000, 0x280FFF, SM_RAM);	// sprites 1
		SekMapMemory((unsigned char *)RamBg,
									0x2C0000, 0x2C0FFF, SM_RAM);	// back ground
		SekMapMemory(Ram68K+0x10000,0x8C0000, 0x8CFFFF, SM_RAM);	// not used?
		
		SekMapHandler(1,			0x200000, 0x200FFF, SM_WRITE);
		
		SekSetReadWordHandler(0, k1945iiiReadWord);
//		SekSetReadByteHandler(0, k1945iiiReadByte);
		SekSetWriteWordHandler(0, k1945iiiWriteWord);
		SekSetWriteByteHandler(0, k1945iiiWriteByte);
		
//		SekSetWriteByteHandler(1, k1945iiiWriteBytePalette);
		SekSetWriteWordHandler(1, k1945iiiWriteWordPalette);

		SekClose();
	}
	
	MSM6295Init(0, 7500, 80, 1);
	MSM6295Init(1, 7500, 80, 1);
	
	DrvDoReset();
	return 0;
}

static int DrvExit()
{
	SekExit();
	
	MSM6295Exit(0);
	MSM6295Exit(1);
	
	free(Mem);
	Mem = NULL;
	return 0;
}

static void DrawBackground()
{
	int offs, mx, my, x, y;
	unsigned short *pal = RamCurPal;
	mx = -1;
	my = 0;
	for (offs = 0; offs < 64*32; offs++) {
		mx++;
		if (mx == 32) {
			mx = 0;
			my++;
		}

		x = mx * 16 - scrollx;
		if (x <= -192) x += 512;
		
		y = my * 16 - scrolly;
		//if (y <= (224-512)) y += 512;
		
		if ( x<=-16 || x>=320 || y<=-16 || y>= 224 ) 
			continue;
		
		unsigned char *d = RomBg + ( RamBg[offs] & 0x1fff ) * 256;
		unsigned short * p = (unsigned short *) pBurnDraw + y * 320 + x;
		
		if ( x >=0 && x <= (320-16) && y >= 0 && y <= (224-16)) {
			
			for (int k=0;k<16;k++) {
				
 				p[ 0] = pal[ d[ 0] ];
 				p[ 1] = pal[ d[ 1] ];
 				p[ 2] = pal[ d[ 2] ];
 				p[ 3] = pal[ d[ 3] ];
 				p[ 4] = pal[ d[ 4] ];
 				p[ 5] = pal[ d[ 5] ];
 				p[ 6] = pal[ d[ 6] ];
 				p[ 7] = pal[ d[ 7] ];

 				p[ 8] = pal[ d[ 8] ];
 				p[ 9] = pal[ d[ 9] ];
 				p[10] = pal[ d[10] ];
 				p[11] = pal[ d[11] ];
 				p[12] = pal[ d[12] ];
 				p[13] = pal[ d[13] ];
 				p[14] = pal[ d[14] ];
 				p[15] = pal[ d[15] ];
 				
 				d += 16;
 				p += 320;
 			}
		} else {

			for (int k=0;k<16;k++) {
				if ( (y+k)>=0 && (y+k)<224 ) {
	 				if ((x +  0) >= 0 && (x +  0)<320) p[ 0] = pal[ d[ 0] ];
	 				if ((x +  1) >= 0 && (x +  1)<320) p[ 1] = pal[ d[ 1] ];
	 				if ((x +  2) >= 0 && (x +  2)<320) p[ 2] = pal[ d[ 2] ];
	 				if ((x +  3) >= 0 && (x +  3)<320) p[ 3] = pal[ d[ 3] ];
	 				if ((x +  4) >= 0 && (x +  4)<320) p[ 4] = pal[ d[ 4] ];
	 				if ((x +  5) >= 0 && (x +  5)<320) p[ 5] = pal[ d[ 5] ];
	 				if ((x +  6) >= 0 && (x +  6)<320) p[ 6] = pal[ d[ 6] ];
	 				if ((x +  7) >= 0 && (x +  7)<320) p[ 7] = pal[ d[ 7] ];
	 				
	 				if ((x +  8) >= 0 && (x +  8)<320) p[ 8] = pal[ d[ 8] ];
	 				if ((x +  9) >= 0 && (x +  9)<320) p[ 9] = pal[ d[ 9] ];
	 				if ((x + 10) >= 0 && (x + 10)<320) p[10] = pal[ d[10] ];
	 				if ((x + 11) >= 0 && (x + 11)<320) p[11] = pal[ d[11] ];
	 				if ((x + 12) >= 0 && (x + 12)<320) p[12] = pal[ d[12] ];
	 				if ((x + 13) >= 0 && (x + 13)<320) p[13] = pal[ d[13] ];
	 				if ((x + 14) >= 0 && (x + 14)<320) p[14] = pal[ d[14] ];
	 				if ((x + 15) >= 0 && (x + 15)<320) p[15] = pal[ d[15] ];
	 			}
	 			
 				d += 16;
 				p += 320;
 			}
		}
	}
}

static void drawgfx(unsigned int code, int sx,int sy)
{
	unsigned short * p = (unsigned short *) pBurnDraw;
	unsigned char * d = RomSpr + code * 256;
	unsigned short * pal = RamCurPal + 0x100;
	
	if (sx >= (320+16)) sx -= 512;
	if (sy >= (224+16)) sy -= 256;
	
	p += sy * 320 + sx;
		
	if (sx >= 0 && sx <= (320-16) && sy > 0 && sy <= (224-16) ) {
	
		for (int k=0;k<16;k++) {
				
 				if( d[ 0] ) p[ 0] = pal[ d[ 0] ];
 				if( d[ 1] ) p[ 1] = pal[ d[ 1] ];
 				if( d[ 2] ) p[ 2] = pal[ d[ 2] ];
 				if( d[ 3] ) p[ 3] = pal[ d[ 3] ];
 				if( d[ 4] ) p[ 4] = pal[ d[ 4] ];
 				if( d[ 5] ) p[ 5] = pal[ d[ 5] ];
 				if( d[ 6] ) p[ 6] = pal[ d[ 6] ];
 				if( d[ 7] ) p[ 7] = pal[ d[ 7] ];

 				if( d[ 8] ) p[ 8] = pal[ d[ 8] ];
 				if( d[ 9] ) p[ 9] = pal[ d[ 9] ];
 				if( d[10] ) p[10] = pal[ d[10] ];
 				if( d[11] ) p[11] = pal[ d[11] ];
 				if( d[12] ) p[12] = pal[ d[12] ];
 				if( d[13] ) p[13] = pal[ d[13] ];
 				if( d[14] ) p[14] = pal[ d[14] ];
 				if( d[15] ) p[15] = pal[ d[15] ];
 				
 				d += 16;
 				p += 320;
 			}
	} else 
	if (sx >= -16 && sx < 320 && sy >= -16 && sy < 224 ) {
		
		for (int k=0;k<16;k++) {
				if ( (sy+k)>=0 && (sy+k)<224 ) {
	 				if( d[ 0] && (sx+ 0)>=0 && (sx+ 0)<320 ) p[ 0] = pal[ d[ 0] ];
	 				if( d[ 1] && (sx+ 1)>=0 && (sx+ 1)<320 ) p[ 1] = pal[ d[ 1] ];
	 				if( d[ 2] && (sx+ 2)>=0 && (sx+ 2)<320 ) p[ 2] = pal[ d[ 2] ];
	 				if( d[ 3] && (sx+ 3)>=0 && (sx+ 3)<320 ) p[ 3] = pal[ d[ 3] ];
	 				if( d[ 4] && (sx+ 4)>=0 && (sx+ 4)<320 ) p[ 4] = pal[ d[ 4] ];
	 				if( d[ 5] && (sx+ 5)>=0 && (sx+ 5)<320 ) p[ 5] = pal[ d[ 5] ];
	 				if( d[ 6] && (sx+ 6)>=0 && (sx+ 6)<320 ) p[ 6] = pal[ d[ 6] ];
	 				if( d[ 7] && (sx+ 7)>=0 && (sx+ 7)<320 ) p[ 7] = pal[ d[ 7] ];
	
	 				if( d[ 8] && (sx+ 8)>=0 && (sx+ 8)<320 ) p[ 8] = pal[ d[ 8] ];
	 				if( d[ 9] && (sx+ 9)>=0 && (sx+ 9)<320 ) p[ 9] = pal[ d[ 9] ];
	 				if( d[10] && (sx+10)>=0 && (sx+10)<320 ) p[10] = pal[ d[10] ];
	 				if( d[11] && (sx+11)>=0 && (sx+11)<320 ) p[11] = pal[ d[11] ];
	 				if( d[12] && (sx+12)>=0 && (sx+12)<320 ) p[12] = pal[ d[12] ];
	 				if( d[13] && (sx+13)>=0 && (sx+13)<320 ) p[13] = pal[ d[13] ];
	 				if( d[14] && (sx+14)>=0 && (sx+14)<320 ) p[14] = pal[ d[14] ];
	 				if( d[15] && (sx+15)>=0 && (sx+15)<320 ) p[15] = pal[ d[15] ];
 				}
 				d += 16;
 				p += 320;
 			}		
		
	}
}

static void DrawSprites()
{
	unsigned short *source = RamSpr0;
	unsigned short *source2 = RamSpr1;
	unsigned short *finish = source + 0x1000/2;

	while( source < finish ) {
		int xpos, ypos;
		int tileno;
		xpos = (source[0] & 0xff00) >> 8;
		ypos = (source[0] & 0x00ff) >> 0;
		tileno = (source2[0] & 0x7ffe) >> 1;
		xpos |=  (source2[0] & 0x0001) << 8;
		
		if (tileno)
			drawgfx(tileno, xpos, ypos);

		source++;source2++;
	}
}

static void DrvDraw()
{
	DrawBackground();
	DrawSprites();
}

static int DrvFrame()
{
	if (DrvReset) DrvDoReset();
	
	if (bRecalcPalette) {
		for (int i=0;i<(0x1000/2); i++)
			RamCurPal[i] = CalcCol( RamPal[i] );
		bRecalcPalette = 0;	
	}
	
	DrvInput[0] = 0x00;													// Joy1
	DrvInput[1] = 0x00;													// Joy2
	DrvInput[2] = 0x00;													// Buttons
	for (int i = 0; i < 8; i++) {
		DrvInput[0] |= (DrvJoy1[i] & 1) << i;
		DrvInput[1] |= (DrvJoy2[i] & 1) << i;
		DrvInput[2] |= (DrvButton[i] & 1) << i;
	}

	
	SekNewFrame();
	SekOpen(0);

#if 0	
	int nCyclesDone = 0;
	int nCyclesNext = 0;
	for(int i=0; i<10; i++) {
		nCyclesNext += (16000000 / 60 / 10);
		nCyclesDone += SekRun( nCyclesNext - nCyclesDone );
	}
#else

	SekRun(16000000 / 60);
	
#endif
	
	SekSetIRQLine(4, SEK_IRQSTATUS_AUTO);
	
	SekClose();
	
	if (pBurnDraw) DrvDraw();
	
	if (pBurnSoundOut) {
		memset(pBurnSoundOut, 0, nBurnSoundLen * 4);
		MSM6295Render(0, pBurnSoundOut, nBurnSoundLen);
		MSM6295Render(1, pBurnSoundOut, nBurnSoundLen);
	}
	return 0;
}

static int DrvScan(int nAction,int *pnMin)
{
	if (pnMin) *pnMin =  0x029671;

	struct BurnArea ba;
	
	if (nAction & ACB_MEMORY_RAM) {								// Scan all memory, devices & variables
		memset(&ba, 0, sizeof(ba));
    	ba.Data	  = RamStart;
		ba.nLen	  = RamEnd-RamStart;
		ba.szName = "All Ram";
		BurnAcb(&ba);
		
		if (nAction & ACB_WRITE)
			bRecalcPalette = 1;
	}
	
	if (nAction & ACB_DRIVER_DATA) {

		SekScan(nAction);										// Scan 68000 state

		SCAN_VAR(DrvInput);
		SCAN_VAR(scrollx);
		SCAN_VAR(scrolly);
		
		SCAN_VAR(m6295bank);
		MSM6295Scan(0, nAction);
		MSM6295Scan(1, nAction);
		
		if (nAction & ACB_WRITE) {

		}
	}
	
	return 0;
}

struct BurnDriver BurnDrv1945kiii = {
	"1945kiii", NULL, NULL, NULL, "2000",
	"1945k III\0", NULL, "Oriental", "misc",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_16BIT_ONLY, 2, HARDWARE_MISC_POST90S, GBF_VERSHOOT, 0,
	NULL, _1945kiiiRomInfo, _1945kiiiRomName, NULL, NULL, _1945kiiiInputInfo, _1945kiiiDIPInfo,
	DrvInit, DrvExit, DrvFrame, NULL, DrvScan, &bRecalcPalette, 0x1000,
	224, 320, 3, 4
};
