#include "burnint.h"
#include "msm6295.h"
#include "burn_ym2151.h"
#include "burn_ym3812.h"

// Uncomment to have the driver perform the screen rotation (faster unless 3D hardware is used for blitting).
// #undef DRIVER_ROTATION
// #define DRIVER_ROTATION

const int TOA_68K_SPEED = 16000000;
const int TOA_Z80_SPEED = 4000000;
const int TOA_VBLANK_LINES = 22;

extern int Hellfire;
extern int Rallybik;

// toaplan.cpp
extern int nToaCyclesScanline;
extern int nToaCyclesDisplayStart;
extern int nToaCyclesVBlankStart;

int ToaLoadCode(unsigned char *Rom, int nStart, int nCount);
int ToaLoadGP9001Tiles(unsigned char* pDest, int nStart, int nNumFiles, int nROMSize, bool bSwap = false);

void ToaClearScreen(int PalOffset);

void ToaZExit();

extern unsigned char* RomZ80;
extern unsigned char* RamZ80;

extern int nCyclesDone[2], nCyclesTotal[2];
extern int nCyclesSegment;

#ifdef DRIVER_ROTATION
 extern bool bToaRotateScreen;
 #define TOA_ROTATE_GRAPHICS_CCW 0
#else
 const bool bToaRotateScreen = false;
 #define TOA_ROTATE_GRAPHICS_CCW BDF_ORIENTATION_VERTICAL
#endif

extern unsigned char* pBurnBitmap;
extern int nBurnColumn;
extern int nBurnRow;

inline void ToaGetBitmap()
{
	if (bToaRotateScreen) {
		pBurnBitmap = pBurnDraw + 319 * nBurnPitch;
		nBurnColumn = -nBurnPitch;
		nBurnRow = nBurnBpp;
	} else {
		pBurnBitmap = pBurnDraw;
		nBurnColumn = nBurnBpp;
		nBurnRow = nBurnPitch;
	}
}

inline void ToaClearOpposites(unsigned char* nJoystickInputs)
{
	if ((*nJoystickInputs & 0x03) == 0x03) {
		*nJoystickInputs &= ~0x03;
	}
	if ((*nJoystickInputs & 0x0C) == 0x0C) {
		*nJoystickInputs &= ~0x0C;
	}
}

// toa_pal.cpp
extern unsigned char *ToaPalSrc;
extern unsigned int* ToaPalette;
extern unsigned char ToaRecalcPalette;
extern int nToaPalLen;

int ToaPalInit();
int ToaPalExit();
int ToaPalUpdate();

// toa_gp9001.cpp
extern unsigned char* GP9001ROM[2];
extern unsigned int nGP9001ROMSize[2];

extern unsigned int GP9001TileBank[8];

extern unsigned char* GP9001RAM[2];
extern unsigned short* GP9001Reg[2];

extern int nSpriteXOffset, nSpriteYOffset, nSpritePriority;

extern int nLayer0XOffset, nLayer0YOffset, nLayer0Priority;
extern int nLayer1XOffset, nLayer1YOffset, nLayer1Priority;
extern int nLayer2XOffset, nLayer2YOffset, nLayer2Priority;
extern int nLayer3XOffset, nLayer3YOffset;

int ToaBufferGP9001Sprites();
int ToaRenderGP9001();
int ToaInitGP9001(int n = 1);
int ToaExitGP9001();
int ToaScanGP9001(int nAction, int* pnMin);

inline static void ToaGP9001SetRAMPointer(unsigned int wordValue, const int nController = 0)
{
	extern unsigned char* GP9001Pointer[2];

	wordValue &= 0x1FFF;
	GP9001Pointer[nController] = GP9001RAM[nController] + (wordValue << 1);
}

inline static void ToaGP9001WriteRAM(const unsigned short wordValue, const int nController)
{
	extern unsigned char* GP9001Pointer[2];

	*((unsigned short*)(GP9001Pointer[nController])) = wordValue;
	GP9001Pointer[nController] += 2;
}

inline static unsigned short ToaGP9001ReadRAM_Hi(const int nController)
{
	extern unsigned char* GP9001Pointer[2];

	return *((unsigned short*)(GP9001Pointer[nController]));
}

inline static unsigned short ToaGP9001ReadRAM_Lo(const int nController)
{
	extern unsigned char* GP9001Pointer[2];

	return *((unsigned short*)(GP9001Pointer[nController] + 2));
}

inline static void ToaGP9001SelectRegister(const unsigned short wordValue, const int nController = 0)
{
	extern int GP9001Regnum[2];

	GP9001Regnum[nController] = wordValue & 0xFF;
}

inline static void ToaGP9001WriteRegister(const unsigned short wordValue, const int nController = 0)
{
	extern int GP9001Regnum[2];

	GP9001Reg[nController][GP9001Regnum[nController]] = wordValue;
}

inline static unsigned short ToaVBlankRegister()
{
	int nCycles = SekTotalCycles();

	if (nCycles >= nToaCyclesVBlankStart) {
		return 1;
	} else {
		if (nCycles < nToaCyclesDisplayStart) {
			return 1;
		}
	}
	return 0;
}

inline static unsigned short ToaScanlineRegister()
{
	static int nPreviousScanline;
	unsigned short nFlags = 0xFE00;
	int nCurrentScanline = SekCurrentScanline();

#if 0
	// None of the games actually use this
	int nCurrentBeamPosition = SekTotalCycles() % nToaCyclesScanline;
	if (nCurrentBeamPosition < 64) {
		nFlags &= ~0x4000;
	}
#endif

	if (nCurrentScanline != nPreviousScanline) {
		nPreviousScanline = nCurrentScanline;
		nFlags &= ~0x8000;

//		bprintf(PRINT_NORMAL, _T("  - line %3i, PC 0x%08X\n"), nCurrentScanline, SekGetPC(-1));

	}

	return nFlags | nCurrentScanline;
}

// toa_extratext.cpp
extern unsigned char* ExtraTROM;
extern unsigned char* ExtraTRAM;
extern unsigned char* ExtraTScroll;
extern unsigned char* ExtraTSelect;
extern int nExtraTXOffset;

int ToaExtraTextLayer();
int ToaExtraTextInit();
void ToaExtraTextExit();

// toa_bcu2.cpp
extern int ToaOpaquePriority;
extern unsigned char* ToaPalSrc2;
extern unsigned int* ToaPalette2;

extern unsigned char* BCU2ROM;
extern unsigned char* FCU2ROM;

extern unsigned char* BCU2RAM;
extern unsigned char* FCU2RAM;
extern unsigned char* FCU2RAMSize;

extern unsigned int BCU2Pointer;
extern unsigned int FCU2Pointer;

extern unsigned int nBCU2ROMSize;
extern unsigned int nFCU2ROMSize;

extern unsigned short BCU2Reg[8];

extern int nBCU2TileXOffset;
extern int nBCU2TileYOffset;

int ToaPal2Update();
int ToaInitBCU2();
int ToaExitBCU2();
void ToaBufferFCU2Sprites();
int ToaRenderBCU2();

// toaplan1.cpp
extern int nToa1Cycles68KSync;

int ToaLoadTiles(unsigned char* pDest, int nStart, int nROMSize);
void toaplan1FMIRQHandler(int, int nStatus);
int toaplan1SynchroniseStream(int nSoundRate);
unsigned char __fastcall toaplan1ReadByteZ80RAM(unsigned int sekAddress);
unsigned short __fastcall toaplan1ReadWordZ80RAM(unsigned int sekAddress);
void __fastcall toaplan1WriteByteZ80RAM(unsigned int sekAddress, unsigned char byteValue);
void __fastcall toaplan1WriteWordZ80RAM(unsigned int sekAddress, unsigned short wordValue);

inline void ToaBCU2SetRAMPointer(unsigned int wordValue)
{
	BCU2Pointer = (wordValue & 0x3FFF) << 1;
}

inline unsigned short ToaBCU2GetRAMPointer()
{
	return (BCU2Pointer >> 1) & 0x3FFF;
}

inline void ToaBCU2WriteRAM(const unsigned short wordValue)
{
	((unsigned short*)BCU2RAM)[BCU2Pointer & 0x7FFF] = wordValue;
	BCU2Pointer++;
}

inline unsigned short ToaBCU2ReadRAM_Hi()
{
	return ((unsigned short*)BCU2RAM)[BCU2Pointer & 0x7FFF];
}

inline unsigned short ToaBCU2ReadRAM_Lo()
{
	return ((unsigned short*)BCU2RAM)[(BCU2Pointer & 0x7FFF) + 1];
}

inline void ToaFCU2SetRAMPointer(unsigned int wordValue)
{
	FCU2Pointer = wordValue & 0x03FF;
}

inline unsigned short ToaFCU2GetRAMPointer()
{
	return FCU2Pointer & 0x03FF;
}

inline void ToaFCU2WriteRAM(const unsigned short wordValue)
{
	((unsigned short*)FCU2RAM)[FCU2Pointer & 0x03FF] = wordValue;
	FCU2Pointer++;
}

inline void ToaFCU2WriteRAMSize(const unsigned short wordValue)
{
	((unsigned short*)FCU2RAMSize)[FCU2Pointer & 0x003F] = wordValue;
	FCU2Pointer++;
}

inline unsigned short ToaFCU2ReadRAM()
{
	return ((unsigned short*)FCU2RAM)[FCU2Pointer & 0x03FF];
}

inline unsigned short ToaFCU2ReadRAMSize()
{
	return ((unsigned short*)FCU2RAMSize)[FCU2Pointer & 0x003F];
}
