#include "burnint.h"
#include "eeprom.h"

#define CAVE_REFRESHRATE (15625.0 / 271.5)

inline static void CaveClearOpposites(unsigned short* nJoystickInputs)
{
	if ((*nJoystickInputs & 0x0003) == 0x0003) {
		*nJoystickInputs &= ~0x0003;
	}
	if ((*nJoystickInputs & 0x000C) == 0x000C) {
		*nJoystickInputs &= ~0x000C;
	}
}

// cave.cpp
extern int nCaveXSize, nCaveYSize;
extern int nCaveXOffset, nCaveYOffset;
extern int nCaveExtraXOffset, nCaveExtraYOffset;
extern int nCaveRowModeOffset;

int CaveScanGraphics();
void CaveClearScreen(unsigned int nColour);

// cave_palette.cpp
extern unsigned int* CavePalette;

extern unsigned char* CavePalSrc;
extern unsigned char CaveRecalcPalette;

int CavePalInit(int nPalSize);
int CavePalExit();
int CavePalUpdate4Bit(int nOffset, int nNumPalettes);
int CavePalUpdate8Bit(int nOffset, int nNumPalettes);

void CavePalWriteByte(unsigned int nAddress, unsigned char byteValue);
void CavePalWriteWord(unsigned int nAddress, unsigned short wordValue);

// cave_tiles.cpp
extern unsigned char* CaveTileROM[4];
extern unsigned char* CaveTileRAM[4];

extern unsigned int CaveTileReg[4][3];
extern int nCaveTileBank;

int CaveTileRender(int nMode);
void CaveTileExit();
int CaveTileInit();
int CaveTileInitLayer(int nLayer, int nROMSize, int nBitdepth, int nOffset);

// cave_sprite.cpp
extern int CaveSpriteVisibleXOffset;

extern unsigned char* CaveSpriteROM;
extern unsigned char* CaveSpriteRAM;

extern int nCaveSpriteBank;
extern int nCaveSpriteBankDelay;

extern int (*CaveSpriteBuffer)();
extern int CaveSpriteRender(int nLowPriority, int nHighPriority);
void CaveSpriteExit();
int CaveSpriteInit(int nType, int nROMSize);


