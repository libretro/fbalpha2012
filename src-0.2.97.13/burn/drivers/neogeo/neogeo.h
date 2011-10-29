#include "burnint.h"

// Uncomment the following line to make the display the full 320 pixels wide
#define NEO_DISPLAY_OVERSCAN

#if defined NEO_DISPLAY_OVERSCAN
 extern int nNeoScreenWidth;// = 320;
#else
 extern int nNeoScreenWidth;// = 304;
#endif

#define MAX_SLOT		(8)

#define NEO_SYS_CD		(1 << 4)
#define NEO_SYS_PCB		(1 << 3)
#define NEO_SYS_CART	(1 << 2)
#define NEO_SYS_AES		(1 << 1)
#define NEO_SYS_MVS		(1 << 0)

struct NeoGameInfo {
	int nCodeOffset; int nCodeNum;
	int nTextOffset;
	int nSpriteOffset; int nSpriteNum;
	int nSoundOffset;
	int nADPCMOffset; int nADPCMANum; int nADPCMBNum;
	int nNeoSRAMProtection;
};

struct NEO_CALLBACK {
	void (*pInitialise)();
	void (*pInstallHandlers)();
	void (*pRemoveHandlers)();
	void (*pBankswitch)();
	int (*pScan)(int, int*);
};

extern NEO_CALLBACK  NeoCallback[MAX_SLOT];
extern NEO_CALLBACK* NeoCallbackActive;

// neogeo.cpp
void NeoClearScreen();
int NeoLoadCode(int nOffset, int nNum, unsigned char* pDest);
int NeoLoadSprites(int nOffset, int nNum, unsigned char* pDest, unsigned int nSpriteSize);
int NeoLoadADPCM(int nOffset, int nNum, unsigned char* pDest);

void NeoDecodeSprites(unsigned char* pDest, int nSize);
void NeoDecodeSpritesCD(unsigned char* pData, unsigned char* pDest, int nSize);

// neo_run.cpp
extern unsigned char* NeoGraphicsRAM;

extern unsigned char nNeoNumSlots;
extern unsigned int nNeoActiveSlot;

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

extern unsigned char* Neo68KROMActive;
extern unsigned char* NeoVectorActive;
extern unsigned char* NeoZ80ROMActive;

extern unsigned char* YM2610ADPCMAROM[MAX_SLOT];
extern unsigned char* Neo68KFix[MAX_SLOT];

extern unsigned int nNeo68KROMBank;

extern unsigned char *NeoSpriteRAM, *NeoTextRAM;

extern bool bNeoEnableGraphics;

int NeoInit();
int NeoCDInit();
int NeoExit();
int NeoScan(int nAction, int* pnMin);
int NeoRender();
int NeoFrame();

void NeoMapBank();
void NeoMap68KFix();
void NeoUpdateVector();

// neo_palette.cpp
extern unsigned char* NeoPalSrc[2];
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
extern unsigned char* NeoTextROMBIOS;
extern unsigned char* NeoTextROM[MAX_SLOT];
extern int nNeoTextROMSize[MAX_SLOT];
extern bool bBIOSTextROMEnabled;

int NeoInitText(int nSlot);
void NeoSetTextSlot(int nSlot);
void NeoExitText(int nSlot);
int NeoRenderText();

void NeoDecodeTextBIOS(int nOffset, const int nSize, unsigned char* pData);
void NeoDecodeText(int nOffset, const int nSize, unsigned char* pData, unsigned char* pDest);
void NeoUpdateTextOne(int nOffset, const unsigned char byteValue);
void NeoUpdateText(int nOffset, const int nSize, unsigned char* pData, unsigned char* pDest);

// neo_sprite.cpp
extern unsigned char* NeoSpriteROM[MAX_SLOT];
extern unsigned char* NeoZoomROM;

extern int nNeoSpriteFrame;
extern unsigned int nNeoTileMask[MAX_SLOT];
extern int nNeoMaxTile[MAX_SLOT];

extern int nSliceStart, nSliceEnd, nSliceSize;

void NeoUpdateSprites(int nOffset, int nSize);
void NeoSetSpriteSlot(int nSlot);
int NeoInitSprites(int nSlot);
void NeoExitSprites(int nSlot);
int NeoRenderSprites();

// neo_decrypt.cpp
extern unsigned char nNeoProtectionXor;

void NeoCMC42Init();
void NeoCMC50Init();
void NeoCMCDecrypt(int extra_xor, unsigned char* rom, unsigned char* buf, int offset, int block_size, int rom_size);
void NeoCMCExtractSData(unsigned char* rom, unsigned char* sdata, int rom_size, int sdata_size);

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
