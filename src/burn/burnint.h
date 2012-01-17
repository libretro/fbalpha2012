// Burn - Arcade emulator library - internal code
#ifndef BURNINT_H
#define BURNINT_H

// Standard headers
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
 
#include "burn.h"

#ifndef __LIBSNES__
#undef LSB_FIRST
#endif

// ------------------------------------------------------------------
// CPU emulation interfaces

#ifdef LSB_FIRST  // just so this burn lib is still compilable under windows, etc
#define swapByte(i) (i) 
#define swapWord(i) (i)
#define swapLong(i) (i)
#define swapLongLong(x) (x)
#else
#define swapByte(i)(i^1)  // might be needed for addresses (e.g. see some of the drivers that ^1 the address on reads)
                                                  // can't see it being useful for data
#if defined (_XBOX)
#include <PPCINTRINSICS.h>
#define swapWord(i) _byteswap_ushort(i) // swap intrinsics are faster on Xbox 360
#define swapLong(i) _byteswap_ulong(i) // swap intrinsics are faster on Xbox 360
#elif defined (SN_TARGET_PS3)
#include <ppu_intrinsics.h>
#define swapWord(i) ({uint16_t t; __sthbrx(&t, i); t;})
#define swapLong(i) ({uint32_t t; __stwbrx(&t, i); t;})
#else
#define swapWord(i)((((i) & 0xff) <<  8) | (((i) & 0xff00) >> 8))
#define swapLong(i)		((((i) & 0xFF000000) >> 24) | \
						(((i) & 0x00FF0000) >> 8)  | \
						(((i) & 0x0000FF00) << 8)  | \
						(((i) & 0x000000FF) << 24) )
#endif

#define swapLongLong(x) _byteswap_uint64(x)
#ifndef _XBOX
static inline uint64_t _byteswap_uint64(uint64_t x)
{
    union { 
        uint64_t ll;
        struct {
           uint32_t l,h;
        } l;
    } r;
    r.l.l = swapLong (x);
    r.l.h = swapLong (x>>32);
    return r.ll;
}
#endif
#endif


typedef union {
#ifdef LSB_FIRST
  struct { uint8_t l,h,h2,h3; } b;
  struct { uint16_t l,h; } w;
#else
  struct { uint8_t h3,h2,h,l; } b;
  struct { uint16_t h,l; } w;
#endif
  uint32_t d;
}  PAIR;

// sek.cpp
#include "sek.h"

// zet.cpp
#include "zet.h"

// ------------------------------------------------------------------
// Driver information

struct BurnDriver
{
	const char * szShortName;	// The filename of the zip file (without extension)
	const char* szParent;		// The filename of the parent (without extension, NULL if not applicable)
	const char* szBoardROM;		// The filename of the board ROMs (without extension, NULL if not applicable)
	const char* szDate;

	// szFullNameA, szCommentA, szManufacturerA and szSystemA should always contain valid info
	// szFullNameW, szCommentW, szManufacturerW and szSystemW should be used only if characters or scripts are needed that ASCII can't handle
	const char*    szFullNameA;
	const char*    szCommentA;
	const char*    szManufacturerA;
	const char*    szSystemA;
	const wchar_t* szFullNameW;
	const wchar_t* szCommentW;
	const wchar_t* szManufacturerW;
	const wchar_t* szSystemW;

	int flags;	// See burn.h
	int players;	// Max number of players a game supports (so we can remove single player games from netplay)
	int hardware;	// Which type of hardware the game runs on
	int (*GetZipName)(char** pszName, unsigned int i);		// Function to get possible zip names
	int (*GetRomInfo)(struct BurnRomInfo* pri,unsigned int i);	// Function to get the length and crc of each rom
	int (*GetRomName)(char** pszName, unsigned int i, int nAka);	// Function to get the possible names for each rom
	int (*GetInputInfo)(struct BurnInputInfo* pii, unsigned int i);	// Function to get the input info for the game
	int (*GetDIPInfo)(struct BurnDIPInfo* pdi, unsigned int i);	// Function to get the input info for the game
	int (*Init)(); int (*Exit)(); int (*Frame)(); int (*Redraw)(); int (*AreaScan)(int nAction, int* pnMin);
	unsigned char* pRecalcPal;				// Set to 1 if the palette needs to be fully re-calculated
	int nWidth, nHeight; int nXAspect, nYAspect;		// Screen width, height, x/y aspect
};

#define BurnDriverD BurnDriver		// Debug status
#define BurnDriverX BurnDriver		// Exclude from build

// Standard functions for dealing with ROM and input info structures
#include "stdfunc.h"

// ------------------------------------------------------------------

// burn.cpp
int BurnSetRefreshRate(double dRefreshRate);

// Byteswaps an area of memory
static inline void BurnByteswap(uint8_t *pMem, int nLen)
{
	nLen >>= 1;
	for (int32_t i = 0; i < nLen; i++, pMem += 2)
	{
		uint8_t t = pMem[0];
		pMem[0] = pMem[1];
		pMem[1] = t;
	}
}

int BurnClearScreen();

// load.cpp
int BurnLoadRom(uint8_t* Dest,int i, int nGap);
int BurnXorRom(uint8_t* Dest,int i, int nGap);
int BurnLoadBitField(uint8_t* pDest, uint8_t* pSrc, int nField, int nSrcLen);

// ------------------------------------------------------------------
// Colour-depth independant image transfer

extern unsigned short* pTransDraw;

int BurnTransferCopy(unsigned int* pPalette);
void BurnTransferExit();
int BurnTransferInit();

// ------------------------------------------------------------------
// Plotting pixels

#if USE_BPP_RENDERING == 16
#define PutPix(pPix, c) *((uint16_t*)pPix) = (uint16_t)(c);
#else
static inline void PutPix(uint8_t* pPix, uint32_t c)
{
	if (nBurnBpp >= 4)
		*((uint32_t*)pPix) = c;
	else
	{
		if (nBurnBpp == 2)
			*((uint16_t*)pPix) = (uint16_t)c;
		else
		{
			pPix[0] = (uint8_t)(c >>  0);
			pPix[1] = (uint8_t)(c >>  8);
			pPix[2] = (uint8_t)(c >> 16);
		}
	}
}
#endif

// ------------------------------------------------------------------
// Clear opposites

#define DrvClearOpposites(joystickInputs) \
	if ((*joystickInputs & 0x03) == 0x03) \
		*joystickInputs &= ~0x03; \
	if ((*joystickInputs & 0x0c) == 0x0c) \
		*joystickInputs &= ~0x0c;

// ------------------------------------------------------------------
#ifndef NO_CHEATS
// Setting up cpus for cheats

typedef enum {
	CPU_M68K, CPU_VEZ, CPU_SH2, CPU_M6502, CPU_Z80,
	CPU_M6809, CPU_M6805, CPU_HD6309, CPU_M6800, CPU_S2650,
	CPU_KONAMICUSTOM, CPU_ARM7,
} CPU_CHEATTYPE;

void CpuCheatRegister(CPU_CHEATTYPE type, int num);
#endif

#endif
