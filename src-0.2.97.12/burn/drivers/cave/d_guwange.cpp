// Guwange
#include "cave.h"
#include "ymz280b.h"

#define CAVE_VBLANK_LINES 12

static unsigned char DrvJoy1[16] = {0, };
static unsigned char DrvJoy2[16] = {0, };
static unsigned short DrvInput[2] = {0, };

static unsigned char *Mem = NULL, *MemEnd = NULL;
static unsigned char *RamStart, *RamEnd;
static unsigned char *Rom01;
static unsigned char *Ram01;

static unsigned char DrvReset = 0;
static unsigned char bDrawScreen;
static bool bVBlank;

static char nVideoIRQ;
static char nSoundIRQ;
static char nUnknownIRQ;

static char nIRQPending;

static int nCurrentCPU;
static int nCyclesDone[2];
static int nCyclesTotal[2];
static int nCyclesSegment;

static struct BurnInputInfo guwangeInputList[] = {
	{"P1 Coin",		BIT_DIGITAL, DrvJoy2 + 0,	"p1 coin"},
	{"P1 Start",	BIT_DIGITAL, DrvJoy1 + 0,	"p1 start"},

	{"P1 Up",		BIT_DIGITAL, DrvJoy1 + 1, 	"p1 up"},
	{"P1 Down",		BIT_DIGITAL, DrvJoy1 + 2, 	"p1 down"},
	{"P1 Left",		BIT_DIGITAL, DrvJoy1 + 3, 	"p1 left"},
	{"P1 Right",	BIT_DIGITAL, DrvJoy1 + 4, 	"p1 right"},
	{"P1 Button 1",	BIT_DIGITAL, DrvJoy1 + 5,	"p1 fire 1"},
	{"P1 Button 2",	BIT_DIGITAL, DrvJoy1 + 6,	"p1 fire 2"},
	{"P1 Button 3",	BIT_DIGITAL, DrvJoy1 + 7,	"p1 fire 3"},

	{"P2 Coin",		BIT_DIGITAL, DrvJoy2 + 1,	"p2 coin"},
	{"P2 Start",	BIT_DIGITAL, DrvJoy1 + 8,	"p2 start"},

	{"P2 Up",		BIT_DIGITAL, DrvJoy1 + 9, 	"p2 up"},
	{"P2 Down",		BIT_DIGITAL, DrvJoy1 + 10, 	"p2 down"},
	{"P2 Left",		BIT_DIGITAL, DrvJoy1 + 11, 	"p2 left"},
	{"P2 Right",	BIT_DIGITAL, DrvJoy1 + 12, 	"p2 right"},
	{"P2 Button 1",	BIT_DIGITAL, DrvJoy1 + 13,	"p2 fire 1"},
	{"P2 Button 2",	BIT_DIGITAL, DrvJoy1 + 14,	"p2 fire 2"},
	{"P2 Button 3",	BIT_DIGITAL, DrvJoy1 + 15,	"p2 fire 3"},

	{"Reset",		BIT_DIGITAL, &DrvReset,		"reset"},
	{"Diagnostics",	BIT_DIGITAL, DrvJoy2 + 2,	"diag"},
	{"Service",		BIT_DIGITAL, DrvJoy2 + 3,	"service"},
};

STDINPUTINFO(guwange)

static void UpdateIRQStatus()
{
	nIRQPending = (nVideoIRQ == 0 || nSoundIRQ == 0 || nUnknownIRQ == 0);
	SekSetIRQLine(1, nIRQPending ? SEK_IRQSTATUS_ACK : SEK_IRQSTATUS_NONE);
}

unsigned char __fastcall guwangeReadByte(unsigned int sekAddress)
{
	switch (sekAddress) {
		case 0x800002:
		case 0x800003: {
			return YMZ280BReadStatus();
		}

		case 0x300000:
		case 0x300001:
		case 0x300002:
		case 0x300003: {
			unsigned char nRet = (nUnknownIRQ << 1) | nVideoIRQ;
			return nRet;
		}
		case 0x300004:
		case 0x300005: {
			unsigned char nRet = (nUnknownIRQ << 1) | nVideoIRQ;
			nVideoIRQ = 1;
			UpdateIRQStatus();
			return nRet;
		}
		case 0x300006:
		case 0x300007: {
			unsigned char nRet = (nUnknownIRQ << 1) | nVideoIRQ;
			nUnknownIRQ = 1;
			UpdateIRQStatus();
			return nRet;
		}

		case 0xD00010:
			return (DrvInput[0] >> 8) ^ 0xFF;
		case 0xD00011:
			return (DrvInput[0] & 0xFF) ^ 0xFF;
		case 0xD00012:
			return (DrvInput[1] >> 8) ^ 0xFF;
		case 0xD00013:
			return ((DrvInput[1] & 0x7F) ^ 0x7F) | (EEPROMRead() << 7);

		default: {
//			bprintf(PRINT_NORMAL, "Attempt to read byte value of location %x\n", sekAddress);
		}
	}
	return 0;
}

unsigned short __fastcall guwangeReadWord(unsigned int sekAddress)
{
	switch (sekAddress) {
		case 0x800002: {
			return YMZ280BReadStatus();
		}

		case 0x300000:
		case 0x300002: {
			unsigned short nRet = (nUnknownIRQ << 1) | nVideoIRQ;
			return nRet;
		}

		case 0x300004: {
			unsigned short nRet = (nUnknownIRQ << 1) | nVideoIRQ;
			nVideoIRQ = 1;
			UpdateIRQStatus();
			return nRet;
		}
		case 0x300006: {
			unsigned short nRet = (nUnknownIRQ << 1) | nVideoIRQ;
			nUnknownIRQ = 1;
			UpdateIRQStatus();
			return nRet;
		}

		case 0xD00010:
			return DrvInput[0] ^ 0xFFFF;
		case 0xD00012:
			return (DrvInput[1] ^ 0xFF7F) | (EEPROMRead() << 7);

		default: {
// 			bprintf(PRINT_NORMAL, "Attempt to read word value of location %x\n", sekAddress);
		}
	}
	return 0;
}

void __fastcall guwangeWriteByte(unsigned int sekAddress, unsigned char byteValue)
{
	switch (sekAddress) {
		case 0x800000:
		case 0x800001:
			YMZ280BSelectRegister(byteValue);
			break;
		case 0x800002:
		case 0x800003:
			YMZ280BWriteRegister(byteValue);
			break;

		case 0xD00011:
			EEPROMWrite(byteValue & 0x40, byteValue & 0x20, byteValue & 0x80);
			break;

		default: {
//			bprintf(PRINT_NORMAL, "Attempt to write byte value %x to location %x\n", byteValue, sekAddress);
		}
	}
}

void __fastcall guwangeWriteWord(unsigned int sekAddress, unsigned short wordValue)
{
	switch (sekAddress) {

		case 0x300000:
			nCaveXOffset = wordValue;
			return;
		case 0x300002:
			nCaveYOffset = wordValue;
			return;
		case 0x300008:
			CaveSpriteBuffer();
			nCaveSpriteBank = wordValue;
			return;

		case 0x800000:
			YMZ280BSelectRegister(wordValue & 0xFF);
			break;
		case 0x800002:
			YMZ280BWriteRegister(wordValue & 0xFF);
			break;

		case 0x900000:
			CaveTileReg[0][0] = wordValue;
			break;
		case 0x900002:
			CaveTileReg[0][1] = wordValue;
			break;
		case 0x900004:
			CaveTileReg[0][2] = wordValue;
			break;

		case 0xA00000:
			CaveTileReg[1][0] = wordValue;
			break;
		case 0xA00002:
			CaveTileReg[1][1] = wordValue;
			break;
		case 0xA00004:
			CaveTileReg[1][2] = wordValue;
			break;

		case 0xB00000:
			CaveTileReg[2][0] = wordValue;
			break;
		case 0xB00002:
			CaveTileReg[2][1] = wordValue;
			break;
		case 0xB00004:
			CaveTileReg[2][2] = wordValue;
			break;

		case 0xD00010:
			EEPROMWrite(wordValue & 0x40, wordValue & 0x20, wordValue & 0x80);
			break;

		default: {
//			bprintf(PRINT_NORMAL, "Attempt to write word value %x to location %x\n", wordValue, sekAddress);

		}
	}
}

void __fastcall guwangeWriteBytePalette(unsigned int sekAddress, unsigned char byteValue)
{
	CavePalWriteByte(sekAddress & 0xFFFF, byteValue);
}

void __fastcall guwangeWriteWordPalette(unsigned int sekAddress, unsigned short wordValue)
{
	CavePalWriteWord(sekAddress & 0xFFFF, wordValue);
}

static void TriggerSoundIRQ(int nStatus)
{
	nSoundIRQ = nStatus ^ 1;
	UpdateIRQStatus();

	if (nIRQPending && nCurrentCPU != 0) {
		nCyclesDone[0] += SekRun(0x0400);
	}
}

static int DrvExit()
{
	YMZ280BExit();

	EEPROMExit();

	CaveTileExit();
	CaveSpriteExit();
    CavePalExit();

	SekExit();				// Deallocate 68000s

	// Deallocate all used memory
	free(Mem);
	Mem = NULL;

	return 0;
}

static int DrvDoReset()
{
	SekOpen(0);
	SekReset();
	SekClose();

	EEPROMReset();

	YMZ280BReset();

	nVideoIRQ = 1;
	nSoundIRQ = 1;
	nUnknownIRQ = 1;

	nIRQPending = 0;

	YMZ280BReset();

	return 0;
}

static int DrvDraw()
{
	CavePalUpdate8Bit(0, 128);				// Update the palette
	CaveClearScreen(CavePalette[0x7F00]);

	if (bDrawScreen) {
//		CaveGetBitmap();

		CaveTileRender(1);					// Render tiles
	}

	return 0;
}

inline static void guwangeClearOpposites(unsigned char* nJoystickInputs)
{
	if ((*nJoystickInputs & 0x06) == 0x06) {
		*nJoystickInputs &= ~0x06;
	}
	if ((*nJoystickInputs & 0x18) == 0x18) {
		*nJoystickInputs &= ~0x18;
	}
}

inline static int CheckSleep(int)
{
#if 1 && defined USE_SPEEDHACKS
	int nCurrentPC = SekGetPC(-1);

	if (!nIRQPending && nCurrentPC >= 0x06D6DE && nCurrentPC <= 0x06D6F4) {
		return 1;
	}
#endif

	return 0;
}

static int DrvFrame()
{
	int nCyclesVBlank;
	int nInterleave = 8;

	if (DrvReset) {														// Reset machine
		DrvDoReset();
	}

	// Compile digital inputs
	DrvInput[0] = 0x0000;  												// Joysticks
	DrvInput[1] = 0x0000;  												// Other controls
	for (int i = 0; i < 16; i++) {
		DrvInput[0] |= (DrvJoy1[i] & 1) << i;
		DrvInput[1] |= (DrvJoy2[i] & 1) << i;
	}
	guwangeClearOpposites(((unsigned char*)DrvInput) + 0);
	guwangeClearOpposites(((unsigned char*)DrvInput) + 1);

	SekNewFrame();

	nCyclesTotal[0] = (int)((long long)16000000 * nBurnCPUSpeedAdjust / (0x0100 * CAVE_REFRESHRATE));
	nCyclesDone[0] = 0;

	nCyclesVBlank = nCyclesTotal[0] - (int)((nCyclesTotal[0] * CAVE_VBLANK_LINES) / 271.5);
	bVBlank = false;

	int nSoundBufferPos = 0;

	SekOpen(0);

	for (int i = 1; i <= nInterleave; i++) {
		int nNext;

		// Render sound segment
		if ((i & 1) == 0) {
			if (pBurnSoundOut) {
				int nSegmentEnd = nBurnSoundLen * i / nInterleave;
				short* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
				YMZ280BRender(pSoundBuf, nSegmentEnd - nSoundBufferPos);
				nSoundBufferPos = nSegmentEnd;
			}
		}

		// Run 68000
    	nCurrentCPU = 0;
		nNext = i * nCyclesTotal[nCurrentCPU] / nInterleave;

		// See if we need to trigger the VBlank interrupt
		if (!bVBlank && nNext > nCyclesVBlank) {
			if (nCyclesDone[nCurrentCPU] < nCyclesVBlank) {
				nCyclesSegment = nCyclesVBlank - nCyclesDone[nCurrentCPU];
				if (!CheckSleep(nCurrentCPU)) {							// See if this CPU is busywaiting
					nCyclesDone[nCurrentCPU] += SekRun(nCyclesSegment);
				} else {
					nCyclesDone[nCurrentCPU] += SekIdle(nCyclesSegment);
				}
			}

			if (pBurnDraw != NULL) {
				DrvDraw();												// Draw screen if needed
			}

			bVBlank = true;
			nVideoIRQ = 0;
			UpdateIRQStatus();
		}

		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		if (!CheckSleep(nCurrentCPU)) {									// See if this CPU is busywaiting
			nCyclesDone[nCurrentCPU] += SekRun(nCyclesSegment);
		} else {
			nCyclesDone[nCurrentCPU] += SekIdle(nCyclesSegment);
		}

		nCurrentCPU = -1;
	}

	// Make sure the buffer is entirely filled.
	{
		if (pBurnSoundOut) {
			int nSegmentLength = nBurnSoundLen - nSoundBufferPos;
			short* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
			if (nSegmentLength) {
				YMZ280BRender(pSoundBuf, nSegmentLength);
			}
		}
	}

	SekClose();

	return 0;
}

// This routine is called first to determine how much memory is needed (MemEnd-(unsigned char *)0),
// and then afterwards to set up all the pointers
static int MemIndex()
{
	unsigned char* Next; Next = Mem;
	Rom01			= Next; Next += 0x100000;		// 68K program
	CaveSpriteROM	= Next; Next += 0x2000000;
	CaveTileROM[0]	= Next; Next += 0x800000;		// Tile layer 0
	CaveTileROM[1]	= Next; Next += 0x400000;		// Tile layer 1
	CaveTileROM[2]	= Next; Next += 0x400000;		// Tile layer 2
	YMZ280BROM		= Next; Next += 0x400000;
	RamStart		= Next;
	Ram01			= Next; Next += 0x010000;		// CPU #0 work RAM
	CaveTileRAM[0]	= Next; Next += 0x008000;
	CaveTileRAM[1]	= Next; Next += 0x008000;
	CaveTileRAM[2]	= Next; Next += 0x008000;
	CaveSpriteRAM	= Next; Next += 0x010000;
	CavePalSrc		= Next; Next += 0x010000;		// palette
	RamEnd			= Next;
	MemEnd			= Next;

	return 0;
}

static void NibbleSwap3(unsigned char* pData, int nLen)
{
	for (int i = 0; i < nLen; i++, pData += 2) {
		unsigned char n1 = pData[0];
		unsigned char n2 = pData[1];

		pData[0] = (n1 << 4) | (n2 & 0x0F);
		pData[1] = (n1 & 0xF0) | (n2 >> 4);
	}

	return;
}

static void NibbleSwap4(unsigned char* pData, int nLen)
{
	for (int i = 0; i < nLen; i++, pData += 2) {
		unsigned char n1 = pData[0];
		unsigned char n2 = pData[1];

		pData[1] = (n2 << 4) | (n1 & 0x0F);
		pData[0] = (n2 & 0xF0) | (n1 >> 4);
	}

	return;
}


static int LoadRoms()
{
	// Load 68000 ROM
	BurnLoadRom(Rom01 + 0, 1, 2);
	BurnLoadRom(Rom01 + 1, 0, 2);

	BurnLoadRom(CaveSpriteROM + 0x0000000, 2, 2);
	BurnLoadRom(CaveSpriteROM + 0x0000001, 3, 2);
	BurnLoadRom(CaveSpriteROM + 0x1000000, 4, 2);
	BurnLoadRom(CaveSpriteROM + 0x1000001, 5, 2);
	NibbleSwap3(CaveSpriteROM, 0xC00000);

#if 1
	for (int i = 0; i < 0x100000; i++) {
		unsigned short nValue = rand() & 0x0101;
		if (nValue & 0x0001) {
			nValue |= 0x00FF;
		}
		if (nValue & 0x0100) {
			nValue |= 0xFF00;
		}
		((unsigned short*)(CaveSpriteROM + 0x1800000))[i] = nValue;
		((unsigned short*)(CaveSpriteROM + 0x1A00000))[i] = nValue;
		((unsigned short*)(CaveSpriteROM + 0x1C00000))[i] = nValue;
		((unsigned short*)(CaveSpriteROM + 0x1E00000))[i] = nValue;
	}
#else
	memcpy(CaveSpriteROM + 0x1800000, CaveSpriteROM + 0x1000000, 0x800000);
#endif

	BurnLoadRom(CaveTileROM[0] + 0x000000, 6, 1);
	NibbleSwap4(CaveTileROM[0], 0x400000);
	BurnLoadRom(CaveTileROM[1] + 0x000000, 7, 1);
	NibbleSwap4(CaveTileROM[1], 0x200000);
	BurnLoadRom(CaveTileROM[2] + 0x000000, 8, 1);
	NibbleSwap4(CaveTileROM[2], 0x200000);

	// Load YMZ280B data
	BurnLoadRom(YMZ280BROM, 9, 1);

	return 0;
}

// Scan ram
static int DrvScan(int nAction, int *pnMin)
{
	struct BurnArea ba;

	if (pnMin) {						// Return minimum compatible version
		*pnMin = 0x020902;
	}

	EEPROMScan(nAction, pnMin);			// Scan EEPROM

	if (nAction & ACB_VOLATILE) {		// Scan volatile ram

		memset(&ba, 0, sizeof(ba));
    	ba.Data		= RamStart;
		ba.nLen		= RamEnd - RamStart;
		ba.szName	= "RAM";
		BurnAcb(&ba);

		SekScan(nAction);				// scan 68000 states

		YMZ280BScan();

		SCAN_VAR(nVideoIRQ);
		SCAN_VAR(nSoundIRQ);
		SCAN_VAR(nUnknownIRQ);
		SCAN_VAR(bVBlank);

		CaveScanGraphics();

		SCAN_VAR(DrvInput);
	}

		if (nAction & ACB_WRITE) {

		CaveRecalcPalette = 1;
		}

	return 0;
}

static const UINT8 default_eeprom[16] =	{0x00,0x0C,0x11,0x0D,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x11,0x11,0xFF,0xFF,0xFF,0xFF};

static int DrvInit()
{
	int nLen;

	BurnSetRefreshRate(CAVE_REFRESHRATE);

	// Find out how much memory is needed
	Mem = NULL;
	MemIndex();
	nLen = MemEnd - (unsigned char *)0;
	if ((Mem = (unsigned char *)malloc(nLen)) == NULL) {
		return 1;
	}
	memset(Mem, 0, nLen);										// blank all memory
	MemIndex();													// Index the allocated memory

	EEPROMInit(&eeprom_interface_93C46);
	if (!EEPROMAvailable()) EEPROMFill(default_eeprom,0, sizeof(default_eeprom));

	// Load the roms into memory
	if (LoadRoms()) {
		return 1;
	}

	{
		SekInit(0, 0x68000);													// Allocate 68000
	    SekOpen(0);

		// Map 68000 memory:
		SekMapMemory(Rom01,					0x000000, 0x0FFFFF, SM_ROM);	// CPU 0 ROM
		SekMapMemory(Ram01,					0x200000, 0x20FFFF, SM_RAM);
		SekMapMemory(CaveSpriteRAM,			0x400000, 0x40FFFF, SM_RAM);
		SekMapMemory(CaveTileRAM[0],		0x500000, 0x507FFF, SM_RAM);
		SekMapMemory(CaveTileRAM[1],		0x600000, 0x607FFF, SM_RAM);
		SekMapMemory(CaveTileRAM[2],		0x700000, 0x707FFF, SM_RAM);

		SekMapMemory(CavePalSrc,			0xC00000, 0xC0FFFF, SM_ROM);	// Palette RAM (write goes through handler)
		SekMapHandler(1,					0xC00000, 0xC0FFFF, SM_WRITE);	//

		SekSetReadWordHandler(0, guwangeReadWord);
		SekSetReadByteHandler(0, guwangeReadByte);
		SekSetWriteWordHandler(0, guwangeWriteWord);
		SekSetWriteByteHandler(0, guwangeWriteByte);

		SekSetWriteWordHandler(1, guwangeWriteWordPalette);
		SekSetWriteByteHandler(1, guwangeWriteBytePalette);

		SekClose();
	}

	nCaveRowModeOffset = 2;

	CavePalInit(0x8000);
	CaveTileInit();
	CaveSpriteInit(1, 0x2000000);
	CaveTileInitLayer(0, 0x800000, 8, 0x4000);
	CaveTileInitLayer(1, 0x400000, 8, 0x4000);
	CaveTileInitLayer(2, 0x400000, 8, 0x4000);

	YMZ280BInit(16934400, &TriggerSoundIRQ, 3);

	bDrawScreen = true;

#if defined FBA_DEBUG && defined USE_SPEEDHACKS
	bprintf(PRINT_IMPORTANT, _T("  * Using speed-hacks (detecting idle loops).\n"));
#endif

	DrvDoReset(); // Reset machine

	return 0;
}

// Rom information
static struct BurnRomInfo guwangeRomDesc[] = {
	{ "gu-u0127.bin", 0x080000, 0xF86B5293, BRF_ESS | BRF_PRG }, //  0 CPU #0 code
	{ "gu-u0129.bin", 0x080000, 0x6C0E3B93, BRF_ESS | BRF_PRG }, //  1

	{ "u083.bin",     0x800000, 0xADC4B9C4, BRF_GRA },			 //  2 Sprite data
	{ "u082.bin",     0x800000, 0x3D75876C, BRF_GRA },			 //  3
	{ "u086.bin",     0x400000, 0x188E4F81, BRF_GRA },			 //  4
	{ "u085.bin",     0x400000, 0xA7D5659E, BRF_GRA },			 //  5

	{ "u101.bin",     0x800000, 0x0369491F, BRF_GRA },			 //  6 Layer 0 Tile data
	{ "u10102.bin",   0x400000, 0xE28D6855, BRF_GRA },			 //  7 Layer 1 Tile data
	{ "u10103.bin",   0x400000, 0x0FE91B8E, BRF_GRA },			 //  8 Layer 2 Tile data

	{ "u0462.bin",    0x400000, 0xB3D75691, BRF_SND },			 //  9 YMZ280B (AD)PCM data
	
	{ "atc05-1.bin",  0x000001, 0x00000000, BRF_NODUMP },
	{ "u0259.bin",    0x000001, 0x00000000, BRF_NODUMP },
	{ "u084.bin",     0x000001, 0x00000000, BRF_NODUMP },
	{ "u108.bin",     0x000001, 0x00000000, BRF_NODUMP },
	
	{ "eeprom-guwange.bin", 0x0080, 0xc3174959, BRF_OPT },
};


STD_ROM_PICK(guwange)
STD_ROM_FN(guwange)


struct BurnDriver BurnDrvGuwange = {
	"guwange", NULL, NULL, NULL, "1999",
	"Guwange (Japan, master ver. 99/06/24)\0", NULL, "Atlus / Cave", "Cave",
	L"\u3050\u308F\u3093\u3052 (Japan, master ver. 99/06/24)\0Guwange\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_16BIT_ONLY, 2, HARDWARE_CAVE_68K_ONLY, GBF_VERSHOOT, 0,
	NULL, guwangeRomInfo, guwangeRomName, NULL, NULL, guwangeInputInfo, NULL,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan,
	&CaveRecalcPalette, 0x8000, 240, 320, 3, 4
};



