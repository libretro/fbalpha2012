#include "burnint.h"

// General
enum {
	PSIKYO_HW_SAMURAIA,
	PSIKYO_HW_GUNBIRD,
	PSIKYO_HW_S1945,
	PSIKYO_HW_TENGAI
};

extern int PsikyoHardwareVersion;

// palette
int PsikyoPalInit();
int PsikyoPalExit();
int PsikyoPalUpdate();

void PsikyoPalWriteByte(unsigned int nAddress, unsigned char byteValue);
void PsikyoPalWriteWord(unsigned int nAddress, unsigned short wordValue);

extern unsigned char* PsikyoPalSrc;
extern unsigned char PsikyoRecalcPalette;
extern unsigned int* PsikyoPalette;

// Tile rendering
int PsikyoTileRender();
void PsikyoSetTileBank(int nLayer, int nBank);
int PsikyoTileInit(unsigned int nROMSize);
void PsikyoTileExit();

extern unsigned char* PsikyoTileROM;
extern unsigned char* PsikyoTileRAM[3];

extern bool bPsikyoClearBackground;

// Sprite rendering
int PsikyoSpriteInit(int nROMSize);
void PsikyoSpriteExit();
int PsikyoSpriteBuffer();
int PsikyoSpriteRender(int nLowPriority, int nHighPriority);

extern unsigned char* PsikyoSpriteROM;
extern unsigned char* PsikyoSpriteRAM;
extern unsigned char* PsikyoSpriteLUT;
