#include "burnint.h"

struct NeoGameInfo {
	int nCodeOffset; int nCodeNum;
	int nTextOffset;
	int nSpriteOffset; int nSpriteNum;
	int nSoundOffset;
	int nADPCMOffset; int nADPCMANum; int nADPCMBNum;
};

// neogeo.cpp
void NeoClearScreen();
int NeoLoadCode(int nOffset, int nNum, unsigned char* pDest);
int NeoLoadSprites(int nOffset, int nNum, unsigned char* pDest, unsigned int nSpriteSize);
int NeoLoadADPCM(int nOffset, int nNum, unsigned char* pDest);

void NeoDecodeSprites(unsigned char* pDest, int nSize);
void NeoDecodeText(unsigned char* pDest, int nSize);

// neo_run.cpp
extern unsigned char* NeoGraphicsRAM;

extern unsigned char nNeoNumSlots;

extern unsigned char NeoButton1[];
extern unsigned char NeoButton2[];
extern unsigned char NeoButton3[];
extern unsigned char NeoButton4[];
extern unsigned char NeoJoy1[];
extern unsigned char NeoJoy2[];
extern unsigned char NeoJoy3[];
extern unsigned char NeoJoy4[];
extern unsigned short NeoAxis[];
extern unsigned char NeoInput[];
extern unsigned char NeoDiag[];
extern unsigned char NeoDebugDip[];
extern unsigned char NeoReset, NeoSystem;

extern unsigned char *Neo68KROM, *NeoZ80ROM, *YM2610ADPCMAROM;
extern unsigned int nNeo68KROMBank;

extern void (*pNeoInitCallback)();
extern void (*pNeoBankswitchCallback)();
extern int (*pNeoScanCallback)(int, int*);

extern bool bNeoEnableGraphics;

int NeoInit();
int NeoExit();
int NeoScan(int nAction, int* pnMin);
int NeoRender();
int NeoFrame();

void NeoMapBank();

// neo_palette.cpp
extern unsigned char* NeoPalSrc[2];
extern unsigned int* NeoPaletteData[2];
extern int nNeoPaletteBank;
extern unsigned int* NeoPalette;

extern unsigned char NeoRecalcPalette;

int NeoInitPalette();
void NeoExitPalette();
int NeoUpdatePalette();
void NeoSetPalette();

void __fastcall NeoPalWriteByte(unsigned int nAddress, unsigned char byteValue);
void __fastcall NeoPalWriteWord(unsigned int nAddress, unsigned short wordValue);

// neo_text.cpp
extern unsigned char* NeoTextROM;
extern int nNeoTextROMSize;
extern bool bBIOSTextROMEnabled;

int NeoInitText();
void NeoExitText();
int NeoRenderText();
void NeoUpdateTextOne(int nOffset, const unsigned char byteValue);

// neo_sprite.cpp
extern int nNeoScreenWidth;

extern unsigned char* NeoSpriteROM;
extern unsigned char* NeoZoomROM;

extern int nNeoSpriteFrame;
extern unsigned int nNeoTileMask;
extern int nNeoMaxTile;

extern int nSliceStart, nSliceEnd, nSliceSize;

int NeoInitSprites();
void NeoExitSprites();
int NeoRenderSprites();

// neo_decrypt.cpp
extern unsigned char nNeoProtectionXor;

void NeoGfxDecryptCMC42Init();
void NeoGfxDecryptCMC50Init();
void NeoGfxDecryptDoBlock(int extra_xor, unsigned char* buf, int offset, int block_size, int rom_size);
void NeoExtractSData(unsigned char* rom, unsigned char* sdata, int rom_size, int sdata_size);
void pcb_gfx_crypt(unsigned char *src, int nType);
void neogeo_cmc50_m1_decrypt();

// neo_upd4990a.cpp
void uPD4990AExit();
void uPD499ASetTicks(unsigned int nTicksPerSecond);
int uPD4990AInit(unsigned int nTicksPerSecond);
void uPD4990AScan(int nAction, int* pnMin);
void uPD4990AUpdate(unsigned int nTicks);
void uPD4990AWrite(unsigned char CLK, unsigned char STB, unsigned char DATA);
unsigned char uPD4990ARead(unsigned int nTicks);

// d_neogeo.cpp
void kf2k3pcb_bios_decode();

// rom_save.cpp
void NeoSaveDecryptedCRoms();
