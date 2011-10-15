#include "toaplan.h"
// Dogyuun

static unsigned char DrvButton[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static unsigned char DrvJoy1[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static unsigned char DrvJoy2[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static unsigned char DrvInput[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

static unsigned char DrvReset = 0;
static unsigned char bDrawScreen;
static bool bVBlank;

// Rom information
static struct BurnRomInfo dogyuunRomDesc[] = {
	{ "tp022_01.r16", 0x080000, 0x79EB2429, BRF_ESS | BRF_PRG }, //  0 CPU #0 code

	{ "tp022_3.w92",  0x100000, 0x191B595F, BRF_GRA },			 //  1 GP9001 #1 Tile data
	{ "tp022_4.w93",  0x100000, 0xD58D29CA, BRF_GRA },			 //  2

	{ "tp022_5.w16",  0x200000, 0xD4C1DB45, BRF_GRA },			 //  3 GP9001 #2 Tile data
	{ "tp022_6.w17",  0x200000, 0xD48DC74F, BRF_GRA },			 //  4

//	{"tp022.mcu",	0x010000, 0x00000000, 0x10}, //  5 Sound CPU

	{ "tp022_2.w30",  0x040000, 0x043271B3, BRF_SND },			 //  6 ADPCM data
};


STD_ROM_PICK(dogyuun)
STD_ROM_FN(dogyuun)

static struct BurnRomInfo dogyuunkRomDesc[] = {
	{ "01.u64",       0x080000, 0xfe5bd7f4, BRF_ESS | BRF_PRG }, //  0 CPU #0 code

	{ "tp022_3.w92",  0x100000, 0x191B595F, BRF_GRA },			 //  1 GP9001 #1 Tile data
	{ "tp022_4.w93",  0x100000, 0xD58D29CA, BRF_GRA },			 //  2

	{ "tp022_5.w16",  0x200000, 0xD4C1DB45, BRF_GRA },			 //  3 GP9001 #2 Tile data
	{ "tp022_6.w17",  0x200000, 0xD48DC74F, BRF_GRA },			 //  4

//	{"tp022.mcu",	0x010000, 0x00000000, 0x10}, //  5 Sound CPU

	{ "tp022_2.w30",  0x040000, 0x043271B3, BRF_SND },			 //  6 ADPCM data
};


STD_ROM_PICK(dogyuunk)
STD_ROM_FN(dogyuunk)

static struct BurnRomInfo dogyuuntRomDesc[] = {
	{ "sample10.9.u64.bin",     0x080000, 0x585f5016, BRF_ESS | BRF_PRG }, //  0 CPU #0 code

	{ "tp022_3.w92",  0x100000, 0x191B595F, BRF_GRA },			 //  1 GP9001 #1 Tile data
	{ "tp022_4.w93",  0x100000, 0xD58D29CA, BRF_GRA },			 //  2

	{ "tp022_5.w16",  0x200000, 0xD4C1DB45, BRF_GRA },			 //  3 GP9001 #2 Tile data
	{ "tp022_6.w17",  0x200000, 0xD48DC74F, BRF_GRA },			 //  4

//	{"tp022.mcu",	0x010000, 0x00000000, 0x10}, //  5 Sound CPU

	{ "tp022_2.w30",  0x040000, 0x043271B3, BRF_SND },			 //  6 ADPCM data
};


STD_ROM_PICK(dogyuunt)
STD_ROM_FN(dogyuunt)

static struct BurnInputInfo dogyuunInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvButton + 3,	"p1 coin"},
	{"P1 Start",	BIT_DIGITAL,	DrvButton + 5,	"p1 start"},

	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 up"},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 down"},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 left"},
	{"P1 Right",	BIT_DIGITAL,	DrvJoy1 + 3,	"p1 right"},
	{"P1 Button 1",	BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"},
	{"P1 Button 2",	BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"},
	{"P1 Button 3",	BIT_DIGITAL,	DrvJoy1 + 6,	"p1 fire 3"},

	{"P2 Coin",		BIT_DIGITAL,	DrvButton + 4,	"p2 coin"},
	{"P2 Start",	BIT_DIGITAL,	DrvButton + 6,	"p2 start"},

	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 up"},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 down"},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 left"},
	{"P2 Right",	BIT_DIGITAL,	DrvJoy2 + 3,	"p2 right"},
	{"P2 Button 1",	BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"},
	{"P2 Button 2",	BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"},
	{"P2 Button 3",	BIT_DIGITAL,	DrvJoy2 + 6,	"p2 fire 3"},

	{"Reset",		BIT_DIGITAL,	&DrvReset,		"reset"},
	{"Diagnostics",	BIT_DIGITAL,	DrvButton + 0,	"diag"},
	{"Dip A",		BIT_DIPSWITCH,	DrvInput + 3,	"dip"},
	{"Dip B",		BIT_DIPSWITCH,	DrvInput + 4,	"dip"},
	{"Dip C",		BIT_DIPSWITCH,	DrvInput + 5,	"dip"},
};

STDINPUTINFO(dogyuun)

static struct BurnDIPInfo dogyuunDIPList[] = {
	// Defaults
	{0x14,	0xFF, 0xFF,	0x00, NULL},
	{0x15,	0xFF, 0xFF,	0x00, NULL},
	
	// DIP 1
	{0,		0xFE, 0,	2,	  NULL},
	{0x14,	0x01, 0x01,	0x00, "Coin play"},
	{0x14,	0x01, 0x01,	0x01, "Free play"},
	{0,		0xFE, 0,	2,	  NULL},
	{0x14,	0x01, 0x02,	0x00, "Normal screen"},
	{0x14,	0x01, 0x02,	0x02, "Invert screen"},
	{0,		0xFE, 0,	2,	  NULL},
	{0x14,	0x01, 0x04,	0x00, "Normal mode"},
	{0x14,	0x01, 0x04,	0x04, "Test mode"},
	{0,		0xFE, 0,	2,	  "Advertise sound"},
	{0x14,	0x01, 0x08,	0x00, "On"},
	{0x14,	0x01, 0x08,	0x08, "Off"},

	// Normal coin settings
	{0,		0xFE, 0,	4,	  "Coin A"},
	{0x14,	0x82, 0x30,	0x00, "1 coin 1 play"},
	{0x16,	0x00, 0x0F, 0x03, NULL},
	{0x14,	0x82, 0x30,	0x10, "1 coin 2 plays"},
	{0x16,	0x00, 0x0F, 0x03, NULL},
	{0x14,	0x82, 0x30,	0x20, "2 coins 1 play"},
	{0x16,	0x00, 0x0F, 0x03, NULL},
	{0x14,	0x82, 0x30,	0x30, "2 coins 3 plays"},
	{0x16,	0x00, 0x0F, 0x03, NULL},
	{0,		0xFE, 0,	4,	  "Coin B"},
	{0x14,	0x82, 0xC0,	0x00, "1 coin 1 play"},
	{0x16,	0x00, 0x0F, 0x03, NULL},
	{0x14,	0x82, 0xC0,	0x40, "1 coin 2 plays"},
	{0x16,	0x00, 0x0F, 0x03, NULL},
	{0x14,	0x82, 0xC0,	0x80, "2 coins 1 play"},
	{0x16,	0x00, 0x0F, 0x03, NULL},
	{0x14,	0x82, 0xC0,	0xC0, "2 coins 3 plays"},
	{0x16,	0x00, 0x0F, 0x03, NULL},

	// European coin settings
	{0,		0xFE, 0,	4,	  "Coin A"},
	{0x14,	0x02, 0x30,	0x00, "1 coin 1 play"},
	{0x16,	0x00, 0x0F, 0x03, NULL},
	{0x14,	0x02, 0x30,	0x10, "2 coins 1 play"},
	{0x16,	0x00, 0x0F, 0x03, NULL},
	{0x14,	0x02, 0x30,	0x20, "3 coins 1 play"},
	{0x16,	0x00, 0x0F, 0x03, NULL},
	{0x14,	0x02, 0x30,	0x30, "3 coins 1 play"},
	{0x16,	0x00, 0x0F, 0x03, NULL},
	{0,		0xFE, 0,	4,	  "Coin B"},
	{0x14,	0x02, 0xC0,	0x00, "1 coin 2 plays"},
	{0x16,	0x00, 0x0F, 0x03, NULL},
	{0x14,	0x02, 0xC0,	0x40, "1 coin 3 plays"},
	{0x16,	0x00, 0x0F, 0x03, NULL},
	{0x14,	0x02, 0xC0,	0x80, "1 coin 4 play"},
	{0x16,	0x00, 0x0F, 0x03, NULL},
	{0x14,	0x02, 0xC0,	0xC0, "1 coin 6 plays"},
	{0x16,	0x00, 0x0F, 0x03, NULL},

	// DIP 2
	{0,		0xFE, 0,	4,	  "Game difficulty"},
	{0x15,	0x01, 0x03,	0x00, "B (normal)"},
	{0x15,	0x01, 0x03,	0x01, "A (easy)"},
	{0x15,	0x01, 0x03,	0x02, "C (hard)"},
	{0x15,	0x01, 0x03,	0x03, "D (very hard)"},
	{0,		0xFE, 0,	4,	  "Extend"},
	{0x15,	0x01, 0x0C,	0x00, "200,000pts only"},
	{0x15,	0x01, 0x0C,	0x04, "200,000 every max 3"},
	{0x15,	0x01, 0x0C,	0x08, "400,000pts only"},
	{0x15,	0x01, 0x0C,	0x0C, "No extend"},
	{0,		0xFE, 0,	4,	  "Hero counts"},
	{0x15,	0x01, 0x30,	0x00, "3"},
	{0x15,	0x01, 0x30,	0x01, "5"},
	{0x15,	0x01, 0x30,	0x02, "2"},
	{0x15,	0x01, 0x30,	0x03, "1"},
	{0,		0xFE, 0,	2,	  NULL},
    {0x15,	0x01, 0x40,	0x00, "Normal game"},
    {0x15,	0x01, 0x40,	0x40, "Debug mode"},
	{0,		0xFE, 0,	2,	  "Continue play"},
    {0x15,	0x01, 0x80,	0x00, "On"},
	{0x15,	0x01, 0x80,	0x80, "Off"},
};

static struct BurnDIPInfo dogyuunRegionDIPList[] = {
	// Region
	{0x16,	0xFF, 0x0F,	0x03, NULL},
	{0,		0xFE, 0,	9,	  "Region"},
    {0x16,	0x01, 0x0F,	0x00, "Japan"},
    {0x16,	0x01, 0x0F,	0x01, "U.S.A."},
    {0x16,	0x01, 0x0F,	0x02, "U.S.A. (Atari Games Corp.)"},
    {0x16,	0x01, 0x0F,	0x03, "Europe"},
    {0x16,	0x01, 0x0F,	0x04, "Hong Kong (Charterfield)"},
    {0x16,	0x01, 0x0F,	0x05, "Korea (Unite Trading)"},
    {0x16,	0x01, 0x0F,	0x06, "Taiwan"},
//	{0x16,	0x01, 0x0F,	0x07, "U.S.A."},
    {0x16,	0x01, 0x0F,	0x08, "South East Asia (Charterfield)"},
//	{0x16,	0x01, 0x0F,	0x09, "Hong Kong (Charterfield)"},
//	{0x16,	0x01, 0x0F,	0x0A, "Korea (Unite Trading)"},
//	{0x16,	0x01, 0x0F,	0x0B, "Taiwan"},
//	{0x16,	0x01, 0x0F,	0x0C, "U.S.A. (Atari Games Corp.)"},
//	{0x16,	0x01, 0x0F,	0x0D, "South East Asia (Charterfield)"},
    {0x16,	0x01, 0x0F,	0x0F, "Japan (Taito Corp license)"},
};

STDDIPINFOEXT(dogyuun, dogyuunRegion, dogyuun)

static struct BurnDIPInfo dogyuunkRegionDIPList[] = {
	// Region
	{0x16,	0xFF, 0x0F,	0x05, NULL},
	{0,		0xFE, 0,	9,	  "Region"},
    {0x16,	0x01, 0x0F,	0x00, "Japan"},
    {0x16,	0x01, 0x0F,	0x01, "U.S.A."},
    {0x16,	0x01, 0x0F,	0x02, "U.S.A. (Atari Games Corp.)"},
    {0x16,	0x01, 0x0F,	0x03, "Europe"},
    {0x16,	0x01, 0x0F,	0x04, "Hong Kong (Charterfield)"},
    {0x16,	0x01, 0x0F,	0x05, "Korea (Unite Trading)"},
    {0x16,	0x01, 0x0F,	0x06, "Taiwan"},
//	{0x16,	0x01, 0x0F,	0x07, "U.S.A."},
    {0x16,	0x01, 0x0F,	0x08, "South East Asia (Charterfield)"},
//	{0x16,	0x01, 0x0F,	0x09, "Hong Kong (Charterfield)"},
//	{0x16,	0x01, 0x0F,	0x0A, "Korea (Unite Trading)"},
//	{0x16,	0x01, 0x0F,	0x0B, "Taiwan"},
//	{0x16,	0x01, 0x0F,	0x0C, "U.S.A. (Atari Games Corp.)"},
//	{0x16,	0x01, 0x0F,	0x0D, "South East Asia (Charterfield)"},
    {0x16,	0x01, 0x0F,	0x0F, "Japan (Taito Corp license)"},
};

STDDIPINFOEXT(dogyuunk, dogyuunkRegion, dogyuun)

static struct BurnDIPInfo dogyuuntRegionDIPList[] = {
	// Region
	{0x16,	0xFF, 0x0F,	0x00, NULL},
    {0,		0xFE, 0,	12,	  "Region"},
    {0x16,	0x01, 0x0F,	0x00, "Japan"},
    {0x16,	0x01, 0x0F,	0x01, "U.S.A."},
    {0x16,	0x01, 0x0F,	0x02, "Europe"},
    {0x16,	0x01, 0x0F,	0x03, "Hong Kong"},
    {0x16,	0x01, 0x0F,	0x04, "Korea"},
    {0x16,	0x01, 0x0F,	0x05, "Taiwan"},
    {0x16,	0x01, 0x0F,	0x06, "South East Asia (Charterfield license)"},
	{0x16,	0x01, 0x0F,	0x07, "USA (Romstar license)"},
    {0x16,	0x01, 0x0F,	0x08, "Hong Kong and China (Honest Trading Co. license)"},
    {0x16,	0x01, 0x0F,	0x09, "Korea (JC Trading Corp. license)"},
    {0x16,	0x01, 0x0F,	0x0A, "USA (Fabtek license)"},
    {0x16,	0x01, 0x0F,	0x0F, "Japan (Taito Corp license)"},
 };

STDDIPINFOEXT(dogyuunt, dogyuuntRegion, dogyuun)

static unsigned char *Mem = NULL, *MemEnd = NULL;
static unsigned char *RamStart, *RamEnd;
static unsigned char *Rom01;
static unsigned char *Ram01, *Ram02, *Ram03, *RamPal;

static int nColCount = 0x0800;

// This routine is called first to determine how much memory is needed (MemEnd-(unsigned char *)0),
// and then afterwards to set up all the pointers
static int MemIndex()
{
	unsigned char *Next; Next = Mem;
	Rom01		= Next; Next += 0x080000;		//
	GP9001ROM[0]= Next; Next += nGP9001ROMSize[0];	// GP9001 tile data
	GP9001ROM[1]= Next; Next += nGP9001ROMSize[1];	// GP9001 tile data
	RamStart	= Next;
	Ram01		= Next; Next += 0x004000;		// CPU #0 work RAM
	Ram02		= Next; Next += 0x00F000;		//
	Ram03		= Next; Next += 0x000400;		//
	RamPal		= Next; Next += 0x001000;		// palette
	GP9001RAM[0]= Next; Next += 0x004000;
	GP9001Reg[0]= (unsigned short*)Next; Next += 0x0100 * sizeof(short);
	GP9001RAM[1]= Next; Next += 0x004000;
	GP9001Reg[1]= (unsigned short*)Next; Next += 0x0100 * sizeof(short);
	RamEnd		= Next;
	ToaPalette	= (unsigned int *)Next; Next += nColCount * sizeof(unsigned int);
	MemEnd		= Next;

	return 0;
}

// Scan ram
static int DrvScan(int nAction,int *pnMin)
{
	struct BurnArea ba;

	if (pnMin) {						// Return minimum compatible version
		*pnMin = 0x020997;
	}
	if (nAction & ACB_VOLATILE) {		// Scan volatile ram
		memset(&ba, 0, sizeof(ba));
    	ba.Data		= RamStart;
		ba.nLen		= RamEnd-RamStart;
		ba.szName	= "All Ram";
		BurnAcb(&ba);

		SekScan(nAction);				// scan 68000 states

		ToaScanGP9001(nAction, pnMin);
	}

	return 0;
}

static int LoadRoms()
{
	// Load 68000 ROM
	BurnLoadRom(Rom01, 0, 1);

	// Load GP9001 tile data
	ToaLoadGP9001Tiles(GP9001ROM[0], 1, 2, nGP9001ROMSize[0], true);
	ToaLoadGP9001Tiles(GP9001ROM[1], 3, 2, nGP9001ROMSize[1], true);

	return 0;
}

static unsigned int ZX80Status()
{
	static int nValue = 0xFF;
	nValue ^= 0x55;
	return nValue;
}

unsigned char __fastcall dogyuunReadByte(unsigned int sekAddress)
{
	switch (sekAddress) {
		case 0x200011:								// Player 1 inputs
			return DrvInput[0];
		case 0x200015:								// Player 2 inputs
			return DrvInput[1];
		case 0x200019:								// Other inputs
			return DrvInput[2];

		case 0x21F001:
		 	return ZX80Status();

		case 0x21F005:								// Dipswitch A
			return DrvInput[3];
		case 0x21F007:			   					// Dipswitch B
			return DrvInput[4];
		case 0x21F009:								// Dipswitch C - Territory
			return DrvInput[5];

		case 0x30000D:
			return ToaVBlankRegister();

//		default:
//			printf("Attempt to read byte value of location %x\n", sekAddress);
	}

	return 0;

}

unsigned short __fastcall dogyuunReadWord(unsigned int sekAddress)
{
	switch (sekAddress) {

		case 0x200010:								// Player 1 inputs
			return DrvInput[0];
		case 0x200014:								// Player 2 inputs
			return DrvInput[1];
		case 0x200018:								// Other inputs
			return DrvInput[2];

		case 0x21F004:								// Dipswitch A
			return DrvInput[3];
		case 0x21F006:								// Dipswitch B
			return DrvInput[4];
		case 0x21F008:								// Dipswitch C - Territory
			return DrvInput[5];

 		case 0x21F000:
		 	return ZX80Status();

		case 0x300004:
			return ToaGP9001ReadRAM_Hi(0);
		case 0x300006:
			return ToaGP9001ReadRAM_Lo(0);

		case 0x30000C:
			return ToaVBlankRegister();

		case 0x500004:
			return ToaGP9001ReadRAM_Hi(1);
		case 0x500006:
			return ToaGP9001ReadRAM_Lo(1);

		case 0x700000: {
			static int i;
			unsigned short nStatus;

			i++;
			nStatus = 0xFFFF - (i & 0xFF);
			if (i & 1) {
				nStatus &= 0x00FF;
			}
			return nStatus;

		}

//		default:
//			printf("Attempt to read word value of location %x\n", sekAddress);
	}

	return 0;

}

void __fastcall dogyuunWriteByte(unsigned int sekAddress, unsigned char byteValue)
{
	switch (sekAddress) {
		case 0x21F001:
		case 0x21F003:
		case 0x20001D:
			break;

		default: {
//			printf("Attempt to write byte value %x to location %x\n", byteValue, sekAddress);

			if ((sekAddress & 0x00FFE000) == 0x0021E000) {
				Ram02[(sekAddress & 0x1FFF) >> 1] = byteValue;
			}
		}
	}
}

void __fastcall dogyuunWriteWord(unsigned int sekAddress, unsigned short wordValue)
{
	switch (sekAddress) {
 		case 0x20001C:
			break;

		case 0x300000:								// Set GP9001 VRAM address-pointer
			ToaGP9001SetRAMPointer(wordValue);
			break;

		case 0x300004:
			ToaGP9001WriteRAM(wordValue, 0);
			break;
		case 0x300006:
			ToaGP9001WriteRAM(wordValue, 0);
			break;

		case 0x300008:
			ToaGP9001SelectRegister(wordValue);
			break;

		case 0x30000C:
			ToaGP9001WriteRegister(wordValue);
			break;

		case 0x500000:								// Set GP9001 VRAM address-pointer
			ToaGP9001SetRAMPointer(wordValue, 1);
			break;

		case 0x500004:
			ToaGP9001WriteRAM(wordValue, 1);
			break;
		case 0x500006:
			ToaGP9001WriteRAM(wordValue, 1);
			break;

		case 0x500008:
			ToaGP9001SelectRegister(wordValue, 1);
			break;

		case 0x50000C:
			ToaGP9001WriteRegister(wordValue, 1);
			break;

		default: {
//			printf("Attempt to write word value %x to location %x\n", wordValue, sekAddress);

			if ((sekAddress & 0x00FFE000) == 0x0021E000) {
				Ram02[(sekAddress & 0x1FFF) >> 1] = wordValue & 0xFF;
			}
		}
	}
}

static int DrvDoReset()
{
	SekOpen(0);
	SekReset();
	SekClose();

	return 0;
}

static int DrvInit()
{
	int nLen;

#ifdef DRIVER_ROTATION
	bToaRotateScreen = true;
#endif

	nGP9001ROMSize[0] = 0x200000;
	nGP9001ROMSize[1] = 0x400000;

	// Find out how much memory is needed
	Mem = NULL;
	MemIndex();
	nLen = MemEnd - (unsigned char *)0;
	if ((Mem = (unsigned char *)malloc(nLen)) == NULL) {
		return 1;
	}
	memset(Mem, 0, nLen);										// blank all memory
	MemIndex();													// Index the allocated memory

	// Load the roms into memory
	if (LoadRoms()) {
		return 1;
	}

	{
		SekInit(0, 0x68000);									// Allocate 68000
	    SekOpen(0);

		// Map 68000 memory:
		SekMapMemory(Rom01,		0x000000, 0x07FFFF, SM_ROM);	// CPU 0 ROM
		SekMapMemory(Ram01,		0x100000, 0x103FFF, SM_RAM);
		SekMapMemory(Ram02,		0x21E000, 0x21EFFF, SM_ROM);	// RAM; write goes through handler
		SekMapMemory(Ram03,		0x21FC00, 0x21FFFF, SM_RAM);
		SekMapMemory(RamPal,	0x400000, 0x400FFF, SM_RAM);	// Palette RAM

		SekSetReadWordHandler(0, dogyuunReadWord);
		SekSetReadByteHandler(0, dogyuunReadByte);
		SekSetWriteWordHandler(0, dogyuunWriteWord);
		SekSetWriteByteHandler(0, dogyuunWriteByte);

		SekClose();
	}

	nSpriteXOffset = 0x0024;
	nSpriteYOffset = 0x0001;

	nLayer0XOffset = -0x01D6;
	nLayer1XOffset = -0x01D8;
	nLayer2XOffset = -0x01DA;

	ToaInitGP9001(2);

	nToaPalLen = nColCount;
	ToaPalSrc = RamPal;
	ToaPalInit();

	bDrawScreen = true;

	DrvDoReset(); 			// Reset machine
	return 0;
}

static int DrvExit()
{
	ToaPalExit();

	ToaExitGP9001();
	SekExit();				// Deallocate 68000s

	// Deallocate all used memory
	free(Mem);
	Mem = NULL;

	return 0;
}

static int DrvDraw()
{
	ToaClearScreen(0);

	if (bDrawScreen) {
		ToaGetBitmap();
		ToaRenderGP9001();					// Render GP9001 graphics
	}

	ToaPalUpdate();							// Update the palette

	return 0;
}

inline static int CheckSleep(int)
{
	return 0;
}

static int DrvFrame()
{
	int nInterleave = 4;

	if (DrvReset) {														// Reset machine
		DrvDoReset();
	}

	// Compile digital inputs
	DrvInput[0] = 0x00;													// Buttons
	DrvInput[1] = 0x00;													// Player 1
	DrvInput[2] = 0x00;													// Player 2
	for (int i = 0; i < 8; i++) {
		DrvInput[0] |= (DrvJoy1[i] & 1) << i;
		DrvInput[1] |= (DrvJoy2[i] & 1) << i;
		DrvInput[2] |= (DrvButton[i] & 1) << i;
	}
	ToaClearOpposites(&DrvInput[0]);
	ToaClearOpposites(&DrvInput[1]);

	SekNewFrame();

	nCyclesTotal[0] = (int)((long long)16000000 * nBurnCPUSpeedAdjust / (0x0100 * 60));
	nCyclesDone[0] = 0;

	SekSetCyclesScanline(nCyclesTotal[0] / 262);
	nToaCyclesDisplayStart = nCyclesTotal[0] - ((nCyclesTotal[0] * (TOA_VBLANK_LINES + 240)) / 262);
	nToaCyclesVBlankStart = nCyclesTotal[0] - ((nCyclesTotal[0] * TOA_VBLANK_LINES) / 262);
	bVBlank = false;

	SekOpen(0);
	for (int i = 0; i < nInterleave; i++) {
    	int nCurrentCPU;
		int nNext;

		// Run 68000

		nCurrentCPU = 0;
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;

		// Trigger VBlank interrupt
		if (!bVBlank && nNext > nToaCyclesVBlankStart) {
			if (nCyclesDone[nCurrentCPU] < nToaCyclesVBlankStart) {
				nCyclesSegment = nToaCyclesVBlankStart - nCyclesDone[nCurrentCPU];
				nCyclesDone[nCurrentCPU] += SekRun(nCyclesSegment);
			}

			ToaBufferGP9001Sprites();

			bVBlank = true;
			SekSetIRQLine(4, SEK_IRQSTATUS_AUTO);
		}

		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		if (bVBlank || (!CheckSleep(nCurrentCPU))) {					// See if this CPU is busywaiting
			nCyclesDone[nCurrentCPU] += SekRun(nCyclesSegment);
		} else {
			nCyclesDone[nCurrentCPU] += SekIdle(nCyclesSegment);
		}

	}
	SekClose();

	if (pBurnDraw) {
		DrvDraw();														// Draw screen if needed
	}

	return 0;
}

struct BurnDriver BurnDrvDogyuun = {
	"dogyuun", NULL, NULL, NULL, "1992",
	"Dogyuun\0", "No sound (sound MCU not dumped)", "Toaplan", "Dual Toaplan GP9001 based",
	L"Dogyuun\0Dogyuun \u30C9\u30AD\u30E5\u30FC\u30F3\uFF01\uFF01\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | TOA_ROTATE_GRAPHICS_CCW, 2, HARDWARE_TOAPLAN_68K_Zx80, GBF_VERSHOOT, 0,
	NULL, dogyuunRomInfo, dogyuunRomName, NULL, NULL, dogyuunInputInfo, dogyuunDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &ToaRecalcPalette, 0x800,
	240, 320, 3, 4
};

struct BurnDriver BurnDrvDogyuunk = {
	"dogyuuna", "dogyuun", NULL, NULL, "1992",
	"Dogyuun (Licensed to Unite Trading For Korea)\0", "No sound (sound MCU not dumped)", "Toaplan", "Dual Toaplan GP9001 based",
	L"Dogyuun (Licensed to Unite Trading For Korea)\0Dogyuun \u30C9\u30AD\u30E5\u30FC\u30F3\uFF01\uFF01\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | TOA_ROTATE_GRAPHICS_CCW, 2, HARDWARE_TOAPLAN_68K_Zx80, GBF_VERSHOOT, 0,
	NULL, dogyuunkRomInfo, dogyuunkRomName, NULL, NULL, dogyuunInputInfo, dogyuunkDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &ToaRecalcPalette, 0x800,
	240, 320, 3, 4
};

struct BurnDriver BurnDrvDogyuunt = {
	"dogyuunt", "dogyuun", NULL, NULL, "1992",
	"Dogyuun (test location version)\0", "No sound (sound MCU not dumped)", "Toaplan", "Dual Toaplan GP9001 based",
	L"Dogyuun (test location version)\0Dogyuun \u30C9\u30AD\u30E5\u30FC\u30F3\uFF01\uFF01\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | TOA_ROTATE_GRAPHICS_CCW, 2, HARDWARE_TOAPLAN_68K_Zx80, GBF_VERSHOOT, 0,
	NULL, dogyuuntRomInfo, dogyuuntRomName, NULL, NULL, dogyuunInputInfo, dogyuuntDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &ToaRecalcPalette, 0x800,
	240, 320, 3, 4
};
