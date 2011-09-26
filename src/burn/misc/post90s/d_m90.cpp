#include "tiles_generic.h"
#include "vez.h"
#include "irem_cpu.h"

static unsigned char *Mem = NULL, *MemEnd = NULL;
static unsigned char *RamStart, *RamEnd;

static unsigned char *RomZ80;
static unsigned char *RomV30;
static unsigned char *DecrRomV30;
static unsigned char *RomSpr;
static unsigned char *RomTile;

static unsigned char *RamVideo;
static unsigned char *RamV30;
static unsigned char *RamSpr;
static unsigned char *RamSprCpy;
static unsigned char *RamPal;

static unsigned short *RamCurPal;

static unsigned char DrvButton[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static unsigned char DrvJoy1[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static unsigned char DrvJoy2[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static unsigned char DrvJoy3[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static unsigned char DrvJoy4[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static unsigned char DrvInput[8] = {0, 0, 0, 0, 0, 0, 0, 0};

static unsigned char bRecalcPalette = 0;
static unsigned char DrvReset = 0;

static INT32* m90_video_control_data;


static struct BurnInputInfo dynablstInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvButton + 2,	"p1 coin"},
	{"P1 Start",	BIT_DIGITAL,	DrvButton + 0,	"p1 start"},

	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 up"},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 down"},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 left"},
	{"P1 Right",	BIT_DIGITAL,	DrvJoy1 + 0,	"p1 right"},
	{"P1 Button 1",	BIT_DIGITAL,	DrvJoy1 + 7,	"p1 fire 1"},
	{"P1 Button 2",	BIT_DIGITAL,	DrvJoy1 + 6,	"p1 fire 2"},

	{"P2 Coin",		BIT_DIGITAL,	DrvButton + 3,	"p2 coin"},
	{"P2 Start",	BIT_DIGITAL,	DrvButton + 1,	"p2 start"},

	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 up"},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 down"},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 left"},
	{"P2 Right",	BIT_DIGITAL,	DrvJoy2 + 0,	"p2 right"},
	{"P2 Button 1",	BIT_DIGITAL,	DrvJoy2 + 7,	"p2 fire 1"},
	{"P2 Button 2",	BIT_DIGITAL,	DrvJoy2 + 6,	"p2 fire 2"},

	{"P3 Coin",		BIT_DIGITAL,	DrvJoy3 + 5,	"p3 coin"},
	{"P3 Start",	BIT_DIGITAL,	DrvJoy3 + 4,	"p3 start"},

	{"P3 Up",		BIT_DIGITAL,	DrvJoy3 + 3,	"p3 up"},
	{"P3 Down",		BIT_DIGITAL,	DrvJoy3 + 2,	"p3 down"},
	{"P3 Left",		BIT_DIGITAL,	DrvJoy3 + 1,	"p3 left"},
	{"P3 Right",	BIT_DIGITAL,	DrvJoy3 + 0,	"p3 right"},
	{"P3 Button 1",	BIT_DIGITAL,	DrvJoy3 + 7,	"p3 fire 1"},
	{"P3 Button 2",	BIT_DIGITAL,	DrvJoy3 + 6,	"p3 fire 2"},

	{"P4 Coin",		BIT_DIGITAL,	DrvJoy4 + 5,	"p4 coin"},
	{"P4 Start",	BIT_DIGITAL,	DrvJoy4 + 4,	"p4 start"},

	{"P4 Up",		BIT_DIGITAL,	DrvJoy4 + 3,	"p4 up"},
	{"P4 Down",		BIT_DIGITAL,	DrvJoy4 + 2,	"p4 down"},
	{"P4 Left",		BIT_DIGITAL,	DrvJoy4 + 1,	"p4 left"},
	{"P4 Right",	BIT_DIGITAL,	DrvJoy4 + 0,	"p4 right"},
	{"P4 Button 1",	BIT_DIGITAL,	DrvJoy4 + 7,	"p4 fire 1"},
	{"P4 Button 2",	BIT_DIGITAL,	DrvJoy4 + 6,	"p4 fire 2"},

	{"Reset",		BIT_DIGITAL,	&DrvReset,		"reset"},
	{"Dip A",		BIT_DIPSWITCH,	DrvInput + 5,	"dip"},
	{"Dip B",		BIT_DIPSWITCH,	DrvInput + 6,	"dip"},
	{"Dip C",		BIT_DIPSWITCH,	DrvInput + 7,	"dip"},
};

STDINPUTINFO(dynablst)

static struct BurnDIPInfo dynablstDIPList[] = {

	// Defaults
	{0x21,	0xFF, 0xFF,	0x00, NULL},
	{0x22,	0xFF, 0xFF,	0x00, NULL},
	{0x23,	0xFF, 0xFF,	0x00, NULL},

	{0,		0xFE, 0,	2,	  "Flip Screen"},
	{0x21,	0x01, 0x01,	0x00, "Off"},
	{0x21,	0x01, 0x01,	0x01, "On"},
	{0,		0xFE, 0,	2,	  "Cabinet"},
	{0x21,	0x01, 0x02,	0x00, "2 Players"},
	{0x21,	0x01, 0x02,	0x02, "4 Players"},
	{0,		0xFE, 0,	2,	  "Coin Slots"},
	{0x21,	0x01, 0x04,	0x00, "Common"},
	{0x21,	0x01, 0x04,	0x04, "Separate"},
	{0,		0xFE, 0,	2,	  "Coin Mode"},
	{0x21,	0x01, 0x08,	0x00, "1"},
	{0x21,	0x01, 0x08,	0x08, "2"},
	{0,		0xFE, 0,	16,	  "Coinage"},
	{0x21,	0x01, 0xf0,	0x00, "1C_1C"},
	{0x21,	0x01, 0xf0,	0x10, "2C_1C"},
	{0x21,	0x01, 0xf0,	0x20, "3C_1C"},
	{0x21,	0x01, 0xf0,	0x30, "4C_1C"},
	{0x21,	0x01, 0xf0,	0x40, "5C_1C"},
	{0x21,	0x01, 0xf0,	0x50, "6C_1C"},
	{0x21,	0x01, 0xf0,	0x60, "1C_2C"},
	{0x21,	0x01, 0xf0,	0x70, "1C_3C"},
	{0x21,	0x01, 0xf0,	0x80, "1C_4C"},
	{0x21,	0x01, 0xf0,	0x90, "1C_5C"},
	{0x21,	0x01, 0xf0,	0xa0, "1C_6C"},
	{0x21,	0x01, 0xf0,	0xb0, "2C_3C"},
	{0x21,	0x01, 0xf0,	0xc0, "3C_2C"},
	{0x21,	0x01, 0xf0,	0xd0, "4C_3C"},
	{0x21,	0x01, 0xf0,	0xe0, "2 Coins to Start, 1 to Continue"},
	{0x21,	0x01, 0xf0,	0xf0, "Free Play"},

	// DIP 2
	{0,		0xFE, 0,	4,	  "Lives"},
	{0x22,	0x01, 0x03, 0x00, "2"},
	{0x22,	0x01, 0x03, 0x01, "3"},
	{0x22,	0x01, 0x03, 0x02, "4"},
	{0x22,	0x01, 0x03, 0x03, "1"},
	{0,		0xFE, 0,	4,	  "Difficulty"},
	{0x22,	0x01, 0x0C, 0x00, "Normal"},
	{0x22,	0x01, 0x0C, 0x04, "Easy"},
	{0x22,	0x01, 0x0C, 0x08, "Hard"},
	{0x22,	0x01, 0x0C, 0x0C, "Very Easy"},
	{0,		0xFE, 0,	2,	  "Any Button to Start"},
	{0x22,	0x01, 0x20, 0x00, "Yes"},
	{0x22,	0x01, 0x20, 0x20, "No"},
	{0,		0xFE, 0,	2,	  "Demo Sounds"},
	{0x22,	0x01, 0x40, 0x00, "Off"},
	{0x22,	0x01, 0x40, 0x40, "On"},
	{0,		0xFE, 0,	2,	  "Service"},
	{0x22,	0x01, 0x80, 0x00, "Off"},
	{0x22,	0x01, 0x80, 0x80, "On"},

};

STDDIPINFO(dynablst)

// Rom information
static struct BurnRomInfo dynablstRomDesc[] = {
	{ "bbm-cp1e.62",		0x20000, 0x27667681, BRF_ESS | BRF_PRG },	// CPU 0, V30
	{ "bbm-cp0e.65",		0x20000, 0x95db7a67, BRF_ESS | BRF_PRG },

	{ "bbm-sp.23",		0x10000, 0x251090cd, BRF_ESS | BRF_PRG },	// CPU 1, Z80

	{ "bbm-c0.66",		0x40000, 0x695d2019, BRF_ESS | BRF_PRG },	//GFX
	{ "bbm-c1.67",		0x40000, 0x4c7c8bbc, BRF_ESS | BRF_PRG },
	{ "bbm-c2.68",		0x40000, 0x0700d406, BRF_ESS | BRF_PRG },
	{ "bbm-c3.69",		0x40000, 0x3c3613af, BRF_ESS | BRF_PRG },

	{ "bbm-v0.20",		0x20000, 0x0fa803fe, BRF_ESS | BRF_PRG },	// AUDIO SAMPLES
};

STD_ROM_PICK(dynablst)
STD_ROM_FN(dynablst)

static struct BurnRomInfo bombrmanRomDesc[] = {
	{ "bbm-p1.62",		0x20000, 0x982bd166, BRF_ESS | BRF_PRG },	// CPU 0, V30
	{ "bbm-p0.65",		0x20000, 0x0a20afcc, BRF_ESS | BRF_PRG },

	{ "bbm-sp.23",		0x10000, 0x251090cd, BRF_ESS | BRF_PRG },	// CPU 1, Z80

	{ "bbm-c0.66",		0x40000, 0x695d2019, BRF_ESS | BRF_PRG },	//GFX
	{ "bbm-c1.67",		0x40000, 0x4c7c8bbc, BRF_ESS | BRF_PRG },
	{ "bbm-c2.68",		0x40000, 0x0700d406, BRF_ESS | BRF_PRG },
	{ "bbm-c3.69",		0x40000, 0x3c3613af, BRF_ESS | BRF_PRG },

	{ "bbm-v0.20",		0x20000, 0x0fa803fe, BRF_ESS | BRF_PRG },	// AUDIO SAMPLES
};

STD_ROM_PICK(bombrman)
STD_ROM_FN(bombrman)

static struct BurnRomInfo atompunkRomDesc[] = {
	{ "bbm-cp0d.65",		0x20000, 0x860c0479, BRF_ESS | BRF_PRG },	// CPU 0, V30
	{ "bbm-cp1d.62",		0x20000, 0xbe57bf74, BRF_ESS | BRF_PRG },

	{ "bbm-sp.23",		0x10000, 0x251090cd, BRF_ESS | BRF_PRG },	// CPU 1, Z80

	{ "bbm-c0.66",		0x40000, 0x695d2019, BRF_ESS | BRF_PRG },	//GFX
	{ "bbm-c1.67",		0x40000, 0x4c7c8bbc, BRF_ESS | BRF_PRG },
	{ "bbm-c2.68",		0x40000, 0x0700d406, BRF_ESS | BRF_PRG },
	{ "bbm-c3.69",		0x40000, 0x3c3613af, BRF_ESS | BRF_PRG },

	{ "bbm-v0.20",		0x20000, 0x0fa803fe, BRF_ESS | BRF_PRG },	// AUDIO SAMPLES
};

STD_ROM_PICK(atompunk)
STD_ROM_FN(atompunk)

static struct BurnRomInfo dynablsbRomDesc[] = {
	{ "db2-26.bin",		0x20000, 0xa78c72f8, BRF_ESS | BRF_PRG },	// CPU 0, V30
	{ "db3-25.bin",		0x20000, 0xbf3137c3, BRF_ESS | BRF_PRG },

	{ "db1-17.bin",		0x10000, 0xe693c32f, BRF_ESS | BRF_PRG },	// CPU 1, Z80

	{ "bbm-c0.66",		0x40000, 0x695d2019, BRF_ESS | BRF_PRG },	//GFX
	{ "bbm-c1.67",		0x40000, 0x4c7c8bbc, BRF_ESS | BRF_PRG },
	{ "bbm-c2.68",		0x40000, 0x0700d406, BRF_ESS | BRF_PRG },
	{ "bbm-c3.69",		0x40000, 0x3c3613af, BRF_ESS | BRF_PRG },
};

STD_ROM_PICK(dynablsb)
STD_ROM_FN(dynablsb)


static int MemIndex()
{
	unsigned char *Next; Next = Mem;
	RomV30 		= Next; Next += 0x100000;			// V33
	DecrRomV30 = Next; Next +=	0x100000;
	RomZ80		= Next; Next += 0x10000;			// z80
	RomSpr= Next;Next += (0x2000 * 16 * 16);
	RomTile= Next;Next += (0x8000 * 8 * 8);
	//MSM6295ROM	= Next; Next += 0x20000;

	RamStart	= Next;
	RamVideo	= Next; Next += 0x010000;
	RamV30		= Next; Next += 0x010000;
	RamSpr		= Next; Next += 0x000800;
	RamSprCpy	= Next; Next += 0x000800;
	RamPal		= Next; Next += 0x000400;			// 2 bank of 0x0800

	m90_video_control_data = (INT32 *) Next; Next += 16*sizeof(INT32);

	RamEnd		= Next;

	RamCurPal	= (unsigned short *) Next; Next += 0x001000;

	MemEnd		= Next;
	return 0;
}

unsigned char __fastcall m90ReadByte(unsigned int vezAddress)
{
	// V33 Start vector
	if ((vezAddress & 0xFFFF0) == 0xFFFF0 )
	{
		unsigned char vec = RomV30[(vezAddress -0xFFFF0) +  0x3fff0];

		bprintf(1, _T("Read vec %x, %x, %x\n"), vezAddress, vec, RomV30[(vezAddress -0xFFFF0) +  0x3fff0]);
		return vec;
	}

//	switch (vezAddress) {

//		default:
//			bprintf(PRINT_NORMAL, _T("Attempt to read byte value of location %x\n"), vezAddress);
//	}
	return 0;
}

void __fastcall m90WriteByte(unsigned int /*vezAddress*/, unsigned char /*byteValue*/)
{
//	switch (vezAddress) {

//		default:
			//	if(vezAddress >0x7ffff)
//			{
//				bprintf(PRINT_NORMAL, _T("Attempt to write byte value %x to location %x\n"), byteValue, vezAddress);
//			}
//	}
}

unsigned char __fastcall m90ReadPort(unsigned int vezPort)
{
	switch (vezPort) {

		case 0x00:
			//		bprintf(1, "read input port 0\n");/* Player 1 */
			break;
		case 0x01:
			//		bprintf(1, "read input port 1\n");/* Player 2 */
			break;
		case 0x02:
			//		bprintf(1, "read input port 2\n");/* Coins */
			break;
		case 0x03:
			//	bprintf(1, "read input NOP\n");/* Unused?  High byte of above */
			break;
		case 0x04:
			//	bprintf(1, "read input port 3\n");/* Dip 1 */
			break;
		case 0x05:
			//	bprintf(1, "read input port 4\n");/* Dip 2 */
			return 0x00;
			break;
		case 0x06:
			//	bprintf(1, "read input port 5\n");/* Player 3 */
			break;
		case 0x07:
			//	bprintf(1, "read input port 6\n"); /* Player 4 */
			break;
		default:
			bprintf(PRINT_NORMAL, _T("Attempt to read byte value of port %x\n"), vezPort);
	}
	return ~0;
}

void __fastcall m90WritePort(unsigned int vezPort, unsigned char byteValue)
{
	if (vezPort <= 0x01)
	{
		bprintf(1, _T("write port m72_sound_command_w %x\n"), byteValue); /* Player 4 */
	}
	else if (vezPort >= 0x02 && vezPort <= 0x03)
	{
		//bprintf(1, "write port m90_coincounter_w %x\n",byteValue); /* Player 4 */
	}
	else if (vezPort >= 0x04 && vezPort <= 0x05)
	{
		//bprintf(1, "write port quizf1_bankswitch_w %x\n",byteValue); /* Player 4 */
	}
	else if (vezPort >= 0x80 && vezPort <= 0x8f)
	{
		//bprintf(1, "write port m90_video_control_w %x\n",byteValue); /* Player 4 */
		m90_video_control_data[vezPort- 0x80] = byteValue;
	}
	else
	{
		//	bprintf(1, "unknown write port %x\n",vezPort ); /* Player 4 */
	}
}

static int DrvDoReset()
{
	VezOpen(0);
	VezReset();
	VezClose();

	return 0;
}

static int TilePlaneOffsets[4]   = { 0x600000, 0x400000, 0x200000, 0 };
static int TileXOffsets[8]       = { 0, 1, 2, 3, 4, 5, 6, 7 };
static int TileYOffsets[8]       = { 0, 8, 16, 24, 32, 40, 48, 56 };
static int SpritePlaneOffsets[4] = { 0x600000, 0x400000, 0x200000, 0 };
static int SpriteXOffsets[16]    = { 0, 1, 2, 3, 4, 5, 6, 7, 128, 129, 130, 131, 132, 133, 134, 135 };
static int SpriteYOffsets[16]    = { 0, 8, 16, 24, 32, 40, 48, 56, 64, 72, 80, 88, 96, 104, 112, 120 };

static int dynablstInit()
{
	int nRet;

	Mem = NULL;
	MemIndex();
	int nLen = MemEnd - (unsigned char *)0;
	if ((Mem = (unsigned char *)malloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);										// blank all memory
	MemIndex();

	nRet = BurnLoadRom(RomV30 + 0x000001, 0, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(RomV30 + 0x000000, 1, 2); if (nRet != 0) return 1;

	memcpy (RomV30+ 0xffff0, RomV30+ 0x3fff0, 0x10 );	/* start vector */

	irem_cpu_decrypt(0,bomberman_decryption_table,RomV30,DecrRomV30,0x7ffff );

	nRet = BurnLoadRom(RomZ80 + 0x000000, 3, 1); if (nRet != 0) return 1;

	unsigned char *sprTmp = (unsigned char *) malloc (0x100000);
	if ( sprTmp == 0 )
	{
		return 1;
	}

	for (int i = 0; i < 4; i++)
	{
		nRet = BurnLoadRom(sprTmp + (0x40000*i), 3+i, 1);
		if (nRet != 0)
		{
			return 1;
		}
	}
	GfxDecode(0x2000, 4, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x100, sprTmp, RomSpr);
	GfxDecode(0x8000, 4, 8, 8, TilePlaneOffsets, TileXOffsets, TileYOffsets, 0x40, sprTmp, RomTile);
	free(sprTmp);

	{
		unsigned int cpu_types[] = { 8 };
		VezInit(1, &cpu_types[0]);

		VezOpen(0);

		VezMapArea(0x00000, 0x7ffff, 0, RomV30 + 0x00000);	// CPU 0 ROM
		VezMapArea(0x00000, 0x7ffff, 2, DecrRomV30,RomV30 + 0x00000 );

		VezMapArea(0xa0000, 0xa3fff, 0, RamV30);
		VezMapArea(0xa0000, 0xa3fff, 1, RamV30);

		VezMapArea(0xd0000, 0xdffff, 0, RamVideo);			//??
		VezMapArea(0xd0000, 0xdffff, 1, RamVideo);

		VezMapArea(0xe0000, 0xe03ff, 0, RamPal);			// palette
		VezMapArea(0xe0000, 0xe03ff, 1, RamPal);
		//	VezMapArea(0xffff0, 0xfffff, 0, RomV30 + 0x3fff0);	// start vector

		VezSetReadHandler(m90ReadByte);
		VezSetWriteHandler(m90WriteByte);
		VezSetReadPort(m90ReadPort);
		VezSetWritePort(m90WritePort);

		VezClose();
	}

	DrvDoReset();
	return 0;
}

static int dynablstExit()
{
	VezExit();

	free(Mem);
	Mem = NULL;

	return 0;
}


#define TILE_SPR_NORMAL(x)				\
	if (q[x]) p[x]=pal[q[x]|color];

#define TILE_SPR_FLIP_X(x)				\
	if (q[x]) p[15-x]=pal[q[x]|color];

#define TILE_SPR_NORMAL_E(x)			\
	if (q[x]&&((sx+x)>=0)&&((sx+x)<320)) p[x]=pal[q[x]|color];

#define TILE_SPR_FLIP_X_E(x)			\
	if (q[x]&&((sx+15-x)>=0)&&((sx+15-x)<320)) p[15-x]=pal[q[x]|color];

#define TILE_SPR_NORMAL_LINE			\
	TILE_SPR_NORMAL( 0)					\
	TILE_SPR_NORMAL( 1)					\
	TILE_SPR_NORMAL( 2)					\
	TILE_SPR_NORMAL( 3)					\
	TILE_SPR_NORMAL( 4)					\
	TILE_SPR_NORMAL( 5)					\
	TILE_SPR_NORMAL( 6)					\
	TILE_SPR_NORMAL( 7)					\
	TILE_SPR_NORMAL( 8)					\
	TILE_SPR_NORMAL( 9)					\
	TILE_SPR_NORMAL(10)					\
	TILE_SPR_NORMAL(11)					\
	TILE_SPR_NORMAL(12)					\
	TILE_SPR_NORMAL(13)					\
	TILE_SPR_NORMAL(14)					\
	TILE_SPR_NORMAL(15)

#define TILE_SPR_FLIP_X_LINE			\
	TILE_SPR_FLIP_X( 0)					\
	TILE_SPR_FLIP_X( 1)					\
	TILE_SPR_FLIP_X( 2)					\
	TILE_SPR_FLIP_X( 3)					\
	TILE_SPR_FLIP_X( 4)					\
	TILE_SPR_FLIP_X( 5)					\
	TILE_SPR_FLIP_X( 6)					\
	TILE_SPR_FLIP_X( 7)					\
	TILE_SPR_FLIP_X( 8)					\
	TILE_SPR_FLIP_X( 9)					\
	TILE_SPR_FLIP_X(10)					\
	TILE_SPR_FLIP_X(11)					\
	TILE_SPR_FLIP_X(12)					\
	TILE_SPR_FLIP_X(13)					\
	TILE_SPR_FLIP_X(14)					\
	TILE_SPR_FLIP_X(15)

#define TILE_SPR_NORMAL_LINE_E			\
	TILE_SPR_NORMAL_E( 0)				\
	TILE_SPR_NORMAL_E( 1)				\
	TILE_SPR_NORMAL_E( 2)				\
	TILE_SPR_NORMAL_E( 3)				\
	TILE_SPR_NORMAL_E( 4)				\
	TILE_SPR_NORMAL_E( 5)				\
	TILE_SPR_NORMAL_E( 6)				\
	TILE_SPR_NORMAL_E( 7)				\
	TILE_SPR_NORMAL_E( 8)				\
	TILE_SPR_NORMAL_E( 9)				\
	TILE_SPR_NORMAL_E(10)				\
	TILE_SPR_NORMAL_E(11)				\
	TILE_SPR_NORMAL_E(12)				\
	TILE_SPR_NORMAL_E(13)				\
	TILE_SPR_NORMAL_E(14)				\
	TILE_SPR_NORMAL_E(15)

#define TILE_SPR_FLIP_X_LINE_E			\
	TILE_SPR_FLIP_X_E( 0)				\
	TILE_SPR_FLIP_X_E( 1)				\
	TILE_SPR_FLIP_X_E( 2)				\
	TILE_SPR_FLIP_X_E( 3)				\
	TILE_SPR_FLIP_X_E( 4)				\
	TILE_SPR_FLIP_X_E( 5)				\
	TILE_SPR_FLIP_X_E( 6)				\
	TILE_SPR_FLIP_X_E( 7)				\
	TILE_SPR_FLIP_X_E( 8)				\
	TILE_SPR_FLIP_X_E( 9)				\
	TILE_SPR_FLIP_X_E(10)				\
	TILE_SPR_FLIP_X_E(11)				\
	TILE_SPR_FLIP_X_E(12)				\
	TILE_SPR_FLIP_X_E(13)				\
	TILE_SPR_FLIP_X_E(14)				\
	TILE_SPR_FLIP_X_E(15)

static void pdrawgfx(unsigned int code,unsigned int color,int flipx,int flipy,int sx,int sy,int /*pri*/)
{
	unsigned short * p	= (unsigned short *) pBurnDraw;
	//	unsigned char * pp = RamPri;
	unsigned char * q	= RomSpr + (code << 8);
	unsigned short *pal	= RamCurPal;


	sx -= 80;
	sy -= 136;

	p += sy * 320 + sx;
	//	pp += sy * 320 + sx;

	if (sx < 0 || sx >= (320-16) || sy < 0 || sy >= (240-16) ) {

		if ((sx <= -16) || (sx >= 320) || (sy <= -16) || (sy >= 240))
			return;

		if (flipy) {

			p += 320 * 15;
			//pp += 320 * 15;

			if (flipx) {

				for (int i=15;i>=0;i--) {
					if (((sy+i)>=0) && ((sy+i)<240)) {

						TILE_SPR_FLIP_X_LINE_E

					}
					p -= 320;
					//pp -= 320;
					q += 16;
				}

			} else {

				for (int i=15;i>=0;i--) {
					if (((sy+i)>=0) && ((sy+i)<240)) {

						TILE_SPR_NORMAL_LINE_E

					}
					p -= 320;
					//pp -= 320;
					q += 16;
				}
			}

		} else {

			if (flipx) {

				for (int i=0;i<16;i++) {
					if (((sy+i)>=0) && ((sy+i)<240)) {

						TILE_SPR_FLIP_X_LINE_E

					}
					p += 320;
					//pp += 320;
					q += 16;
				}

			} else {

				for (int i=0;i<16;i++) {
					if (((sy+i)>=0) && ((sy+i)<240)) {

						TILE_SPR_NORMAL_LINE_E

					}
					p += 320;
					//pp += 320;
					q += 16;
				}

			}

		}

		return;
	}

	if (flipy) {

		p += 320 * 15;
		//pp += 320 * 15;

		if (flipx) {

			for (int i=0;i<16;i++) {

				TILE_SPR_FLIP_X_LINE

					p -= 320;
				//pp -= 320;
				q += 16;
			}

		} else {

			for (int i=0;i<16;i++) {

				TILE_SPR_NORMAL_LINE

					p -= 320;
				//pp -= 320;
				q += 16;
			}
		}

	} else {

		if (flipx) {

			for (int i=0;i<16;i++) {

				TILE_SPR_FLIP_X_LINE

					p += 320;
				//pp += 320;
				q += 16;
			}

		} else {

			for (int i=0;i<16;i++) {

				TILE_SPR_NORMAL_LINE

					p += 320;
				//pp += 320;
				q += 16;
			}
		}
	}
}


static void m90_drawsprites()
{
	int offs;
	UINT8 *m90_spriteram = RamVideo+0xee00;
	for (offs = 0x1f2;offs >= 0;offs -= 6)
	{
		int x,y,sprite,colour,fx,fy,y_multi,i;
		sprite = (m90_spriteram[offs+2] | (m90_spriteram[offs+3]<<8));
		colour = ((m90_spriteram[offs+1] >> 1) & 0x1f);

		y = m90_spriteram[offs+0] | ((m90_spriteram[offs+1] & 0x01) << 8);
		x = m90_spriteram[offs+4] | ((m90_spriteram[offs+5] & 0x01) << 8);

		x = x - 16;
		y = 512 - y;

		fx = m90_spriteram[offs+5] & 0x02;
		fy = m90_spriteram[offs+1] & 0x80;

		y_multi = 1 << ((m90_spriteram[offs+1] & 0x60) >> 5);
		y -= 16 * y_multi;


		//colour = ((colour & 0x08) ? 0x00 : 0x02);
		colour = colour <<4;
		for (i = 0;i < y_multi;i++)
		{
			pdrawgfx(sprite + (fy ? y_multi-1 - i : i),colour,fx,fy,x,y+i*16,1);
			//		spritetest(sprite + (fy ? y_multi-1 - i : i), x,y+i*16, , fx,fy);
		}
	}
}


#define	TILE_LAYER_1(xx)		\
	if (d[xx]) p[xx] = pal[ d[xx] | c ];

#define	TILE_LAYER_1_E(xx)		\
	if (d[xx] && (x + xx)>=0 && (x + xx)<320 ) p[xx] = pal[ d[xx] | c ];

#define	TILE_LAYER_1_LINE		\
	TILE_LAYER_1(0)				\
	TILE_LAYER_1(1)				\
	TILE_LAYER_1(2)				\
	TILE_LAYER_1(3)				\
	TILE_LAYER_1(4)				\
	TILE_LAYER_1(5)				\
	TILE_LAYER_1(6)				\
	TILE_LAYER_1(7)

#define	TILE_LAYER_1_LINE_E		\
	if ( (y+k)>=0 && (y+k)<240 ) {	\
		TILE_LAYER_1_E(0)		\
		TILE_LAYER_1_E(1)		\
		TILE_LAYER_1_E(2)		\
		TILE_LAYER_1_E(3)		\
		TILE_LAYER_1_E(4)		\
		TILE_LAYER_1_E(5)		\
		TILE_LAYER_1_E(6)		\
		TILE_LAYER_1_E(7)		\
	}

#define	TILE_LAYER_2_LINE		\
	TILE_LAYER_1(0)				\
	TILE_LAYER_1(1)				\
	TILE_LAYER_1(2)				\
	TILE_LAYER_1(3)				\
	TILE_LAYER_1(4)				\
	TILE_LAYER_1(5)				\
	TILE_LAYER_1(6)				\
	TILE_LAYER_1(7)	\
	TILE_LAYER_1(8)				\
	TILE_LAYER_1(9)				\
	TILE_LAYER_1(10)				\
	TILE_LAYER_1(11)				\
	TILE_LAYER_1(12)				\
	TILE_LAYER_1(13)				\
	TILE_LAYER_1(14)				\
	TILE_LAYER_1(15)

#define	TILE_LAYER_2_LINE_E		\
	if ( (y+k)>=0 && (y+k)<240 ) {	\
		TILE_LAYER_1_E(0)		\
		TILE_LAYER_1_E(1)		\
		TILE_LAYER_1_E(2)		\
		TILE_LAYER_1_E(3)		\
		TILE_LAYER_1_E(4)		\
		TILE_LAYER_1_E(5)		\
		TILE_LAYER_1_E(6)		\
		TILE_LAYER_1_E(7)		\
		TILE_LAYER_1_E(8)		\
		TILE_LAYER_1_E(9)		\
		TILE_LAYER_1_E(10)		\
		TILE_LAYER_1_E(11)		\
		TILE_LAYER_1_E(12)		\
		TILE_LAYER_1_E(13)		\
		TILE_LAYER_1_E(14)		\
		TILE_LAYER_1_E(15)		\
	}


static void renderpf1(unsigned int level)
{
	/* T--- -YX-  Pccc cccc  tttt tttt tttt tttt */
	unsigned short * pal = RamCurPal;
	int offs, mx, my, x, y;

	mx = -1;
	my = 0;
	for (offs = 0; offs < 64*64*4; offs+=4) {
		mx++;
		if (mx == 64) {
			mx = 0;
			my++;
		}

		x = mx * 8 - ((100 + (short)((m90_video_control_data[3]<<8)+m90_video_control_data[2]+2)) & 0x1ff);
		if (x < -16)
			x += 512;

		y = my * 8 - ((136 + (short)(m90_video_control_data[1]<<8)+m90_video_control_data[0]) & 0x1ff);
		if (y < -16)
			y += 512;

		if ( x<=-8 || x>=320 || y<=-8 || y>= 240 )
			continue;


		int tile_index = offs + ((m90_video_control_data[0xa+2*0] & 3)*0x4000);
		unsigned int tileno = RamVideo[tile_index]+(RamVideo[tile_index+1]<<8);
		if (tileno == 0) continue;

		unsigned int colour = RamVideo[tile_index+2];
		unsigned int priority = (colour & 0x30) ? 1 : 0;
		if (priority!=level)
			continue;

		unsigned int c = (colour & 0x7F) << 4;

		unsigned short * p = (unsigned short *) pBurnDraw + y * 320 + x;
		unsigned char * d = RomTile + ((tileno & 0x1ff) * 64);

		if ( x >=0 && x <= (320-8) && y >= 0 && y <= (240-8))
		{

			//unsigned char * pp = RamPri + y * 320 + x;

			for (int k=0;k<8;k++)
			{

				TILE_LAYER_1_LINE

					d += 8;
				p += 320;
			}

		}
		else
		{
			for (int k=0;k<8;k++)
			{
				TILE_LAYER_1_LINE_E

					d += 8;
				p += 320;
			}
		}
	}
}


static void renderpf2(unsigned int level)
{	/* T--- -YX-  Pccc cccc  tttt tttt tttt tttt */
	unsigned short * pal = RamCurPal;
	int offs, mx, my, x, y;

	mx = -1;
	my = 0;
	for (offs = 0; offs < 64*128*4; offs+=8) {
		mx++;
		if (mx == 128) {
			mx = 0;
			my++;
		}

		x = mx * 16 - ((155 - (short)((m90_video_control_data[7]<<8)+m90_video_control_data[6]-2)) & 0x1ff);
		if (x < -16)
			x += 512;

		y = my * 16 - ((132 + (short)(m90_video_control_data[1]<<8)+m90_video_control_data[0]) & 0x1ff);
		if (y < -16)
			y += 512;

		if ( x<=-16 || x>=320 || y<=-16 || y>= 240 )
			continue;


		int tile_index = (offs + ((m90_video_control_data[0xc] & 1)*0x4000)) ;
		unsigned int tileno = RamVideo[tile_index]+(RamVideo[tile_index+1]<<8);
		if (tileno == 0)
			continue;
		unsigned int colour = RamVideo[tile_index+2];
		unsigned int priority = (colour & 0x30) ? 1 : 0;
		if (priority!=level)
			continue;

		unsigned int c = (colour & 0x7F) << 4;
		unsigned short * p = (unsigned short *) pBurnDraw + y * 320 + (x);
		unsigned char * d = (RomSpr + (tileno * 64))-(16*8);

		if ( x >=0 && x <= (320-16) && y >= 0 && y <= (240-16))
		{

			//unsigned char * pp = RamPri + y * 320 + x;

			for (int k=0;k<16;k++)
			{

				TILE_LAYER_2_LINE

					d += 16;
				p += 320;
			}

		}
		else
		{
			for (int k=0;k<16;k++)
			{
				TILE_LAYER_2_LINE_E

				d += 16;
				p += 320;
			}
		}
	}
}


static void DrvDraw()
{
//	int pf1_base = m90_video_control_data[0xa] & 0x3;
//	int pf2_base = m90_video_control_data[0xc] & 0x3;
	int pf1_enable,pf2_enable;

	if (m90_video_control_data[0xa]&0x10)
		pf1_enable=0;
	else
		pf1_enable=1;
	if (m90_video_control_data[0xc]&0x10)
		pf2_enable=0;
	else
		pf2_enable=1;

	/* Setup scrolling */
	if (m90_video_control_data[0xa]&0x20)
	{
		/*
		tilemap_set_scroll_rows(pf1_layer,512);
		tilemap_set_scroll_rows(pf1_wide_layer,512);
		for (i=0; i<1024; i+=2)
		tilemap_set_scrollx( pf1_layer,i/2, (m90_video_data[0xf000+i]+(m90_video_data[0xf001+i]<<8))+2);
		for (i=0; i<1024; i+=2)
		tilemap_set_scrollx( pf1_wide_layer,i/2, (m90_video_data[0xf000+i]+(m90_video_data[0xf001+i]<<8))+256+2);
		*/
	}
	else
	{
		/*
		tilemap_set_scroll_rows(pf1_layer,1);
		tilemap_set_scroll_rows(pf1_wide_layer,1);
		tilemap_set_scrollx( pf1_layer,0, (m90_video_control_data[3]<<8)+m90_video_control_data[2]+2);
		tilemap_set_scrollx( pf1_wide_layer,0, (m90_video_control_data[3]<<8)+m90_video_control_data[2]+256+2);
		*/
	}

	/* Setup scrolling */
	if (m90_video_control_data[0xc]&0x20)
	{
		/*
		tilemap_set_scroll_rows(pf2_layer,512);
		tilemap_set_scroll_rows(pf2_wide_layer,512);
		for (i=0; i<1024; i+=2)
		tilemap_set_scrollx( pf2_layer,i/2, (m90_video_data[0xf400+i]+(m90_video_data[0xf401+i]<<8))-2);
		for (i=0; i<1024; i+=2)
		tilemap_set_scrollx( pf2_wide_layer,i/2, (m90_video_data[0xf400+i]+(m90_video_data[0xf401+i]<<8))+256-2);
		*/
	}
	else
	{
		/*
		tilemap_set_scroll_rows(pf2_layer,1);
		tilemap_set_scroll_rows(pf2_wide_layer,1);
		tilemap_set_scrollx( pf2_layer,0, (m90_video_control_data[7]<<8)+m90_video_control_data[6]-2);
		tilemap_set_scrollx( pf2_wide_layer,0, (m90_video_control_data[7]<<8)+m90_video_control_data[6]+256-2 );
		*/
	}
	/*
	tilemap_set_scrolly( pf1_layer,0, (m90_video_control_data[1]<<8)+m90_video_control_data[0] );
	tilemap_set_scrolly( pf2_layer,0, (m90_video_control_data[5]<<8)+m90_video_control_data[4] );
	tilemap_set_scrolly( pf1_wide_layer,0, (m90_video_control_data[1]<<8)+m90_video_control_data[0] );
	tilemap_set_scrolly( pf2_wide_layer,0, (m90_video_control_data[5]<<8)+m90_video_control_data[4] );

	fillbitmap(priority_bitmap,0,cliprect);
	*/
	/*
	if (!pf2_enable)
		fillbitmap(bitmap,machine->pens[0],cliprect);
	*/
	if (pf2_enable)
	{
		renderpf2(0);
	}

	if (pf1_enable)
	{
		renderpf1(0);
	}

	m90_drawsprites();

	if (pf2_enable)
	{
		renderpf2(1);
	}

	if (pf1_enable)
	{
		renderpf1(1);
	}
}

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

int DrvCalcPal()
{
	int i;
	unsigned short* ps;
	unsigned short* pd;

	for (i = 0, ps = (unsigned short*)RamPal, pd = RamCurPal; i < 0x800; i++, ps++, pd++) {
		*pd = CalcCol(*ps);
	}
	return 0;
}

static int dynablstFrame()
{
	if (DrvReset)
	{
		DrvDoReset();
	}

	VezOpen(0);
	VezRun(8000000 / 60);


	VezSetIRQLine(0x60, VEZ_IRQSTATUS_ACK);

	VezClose();

	if (pBurnDraw)
	{
		DrvCalcPal();
		BurnClearScreen();
		DrvDraw();
	}
	return 0;
}

static int dynablstScan(int /*nAction*/,int *pnMin)
{
	//struct BurnArea ba;

	if (pnMin)						// Return minimum compatible version
		*pnMin =  0x029671;

	return 0;
}




struct BurnDriverD BurnDrvdynablst = {
	"dynablst", NULL, NULL, "1992",
	"Dynablaster / Bomber Man\0", "Preliminary driver", "Irem (licensed from Hudson Soft)", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_16BIT_ONLY, 4, HARDWARE_MISC_MISC, 
	NULL, dynablstRomInfo, dynablstRomName, dynablstInputInfo, dynablstDIPInfo,
	dynablstInit, dynablstExit, dynablstFrame, NULL, dynablstScan, &bRecalcPalette,
	320, 240, 4, 3
};

struct BurnDriverD BurnDrvbombrman = {
	"bombrman", "dynablst", NULL, "1992",
	"Bomber Man (Japan)\0", "Preliminary driver", "Irem (licensed from Hudson Soft)", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_CLONE | BDF_16BIT_ONLY, 4, HARDWARE_MISC_MISC, 
	NULL, bombrmanRomInfo, bombrmanRomName, dynablstInputInfo, dynablstDIPInfo,
	dynablstInit, dynablstExit, dynablstFrame, NULL, dynablstScan, &bRecalcPalette,
	320, 240, 4, 3
};

struct BurnDriverD BurnDrvatompunk = {
	"atompunk", "dynablst", NULL, "1992",
	"Atomic Punk (US)\0", "Preliminary driver", "Irem America (licensed from Hudson Soft)", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_CLONE | BDF_16BIT_ONLY, 4, HARDWARE_MISC_MISC, 
	NULL, atompunkRomInfo, atompunkRomName, dynablstInputInfo, dynablstDIPInfo,
	dynablstInit, dynablstExit, dynablstFrame, NULL, dynablstScan, &bRecalcPalette,
	320, 240, 4, 3
};

struct BurnDriverD BurnDrvdynablsb = {
	"dynablstb", "dynablst", NULL, "1992",
	"Dynablaster (bootleg)\0", "Preliminary driver", "bootleg", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_CLONE | BDF_16BIT_ONLY, 4, HARDWARE_MISC_MISC, 
	NULL, dynablsbRomInfo, dynablsbRomName, dynablstInputInfo, dynablstDIPInfo,
	dynablstInit, dynablstExit, dynablstFrame, NULL, dynablstScan, &bRecalcPalette,
	320, 240, 4, 3
};
