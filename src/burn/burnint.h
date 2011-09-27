// Burn - Arcade emulator library - internal code
#ifndef BURNINT_H
#define BURNINT_H

// Standard headers
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

 
#include "burn.h"
#undef LSB_FIRST
// ------------------------------------------------------------------
// CPU emulation interfaces

#ifdef LSB_FIRST  // just so this burn lib is still compilable under windows, etc
#define swapByte(i)(i) 
#define swapWord(i)(i)
#define swapLong(i)(i)
#define swapLongLong (x)
#else
#define swapByte(i)(i^1)  // might be needed for addresses (e.g. see some of the drivers that ^1 the address on reads)
                                                  // can't see it being useful for data
#if defined (_XBOX)
#include <PPCINTRINSICS.h>
#define swapWord(i) _byteswap_ushort(i) // swap intrinsics are faster on Xbox 360
#elif defined (SN_TARGET_PS3)
#ifdef __SNC__
#include <ppu_intrinsics.h>
#else
#define __sthbrx(base, value) do {			\
    typedef  struct {char a[2];} halfwordsize;		\
    halfwordsize *ptrp = (halfwordsize*)(void*)(base);		\
    __asm__ ("sthbrx %1,%y0"				\
	   : "=Z" (*ptrp)				\
	   : "r" (value));				\
   } while (0)

#define __stwbrx(base, value) do {		\
    typedef  struct {char a[4];} wordsize;	\
    wordsize *ptrp = (wordsize*)(void*)(base);		\
    __asm__ ("stwbrx %1,%y0"			\
	   : "=Z" (*ptrp)			\
	   : "r" (value));			\
   } while (0)
#endif
#define swapWord(i) _byteswap_ushort(i)
static inline uint16_t _byteswap_ushort(uint16_t x)
{
        uint16_t t;
#ifdef SN_TARGET_PS3
        __sthbrx(&t, x);
#else
asm("sthbrx     %1,%y0" : "=Z"(*&t) : "r"(x));
#endif
        return t;
}
#else
#define swapWord(i)((((i) & 0xff) <<  8) | (((i) & 0xff00) >> 8))
#endif

#if defined  (_XBOX)
#define swapLong(i) _byteswap_ulong(i) // swap intrinsics are faster on Xbox 360
#elif defined (SN_TARGET_PS3)
#define swapLong(i) _byteswap_ulong(i)
static inline uint32_t _byteswap_ulong(uint32_t x)
{
		uint32_t t;
#ifdef SN_TARGET_PS3
      __stwbrx(&t, x);
#else
asm("stwbrx     %1,%y0" : "=Z"(*&t) : "r"(x));
#endif
		return t;
}
#else
#define swapLong(i)		((((i) & 0xFF000000) >> 24) | \
						(((i) & 0x00FF0000) >> 8)  | \
						(((i) & 0x0000FF00) << 8)  | \
						(((i) & 0x000000FF) << 24) )
#endif

#if defined (_XBOX)
#define swapLongLong(i) _byteswap_uint64(i) // swap intrinsics are faster on Xbox 360
#else
#define swapLongLong(x) _byteswap_uint64(x)
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
  struct { UINT8 l,h,h2,h3; } b;
  struct { UINT16 l,h; } w;
#else
  struct { UINT8 h3,h2,h,l; } b;
  struct { UINT16 h,l; } w;
#endif
  UINT32 d;
}  PAIR;

// sek.cpp
#include "sek.h"

// zet.cpp
#include "zet.h"

// ------------------------------------------------------------------
// Driver information

struct BurnDriver {
	const char * szShortName;			// The filename of the zip file (without extension)
	const char* szParent;				// The filename of the parent (without extension, NULL if not applicable)
	const char* szBoardROM;			// The filename of the board ROMs (without extension, NULL if not applicable)
	const char* szDate;

	// szFullNameA, szCommentA, szManufacturerA and szSystemA should always contain valid info
	// szFullNameW, szCommentW, szManufacturerW and szSystemW should be used only if characters or scripts are needed that ASCII can't handle
	const char*    szFullNameA;
	const char*    szCommentA;
	const char*    szManufacturerA;
	const char*    szSystemA;
	wchar_t* szFullNameW;
	wchar_t* szCommentW;
	wchar_t* szManufacturerW;
	wchar_t* szSystemW;

	int flags;			// See burn.h
	int players;		// Max number of players a game supports (so we can remove single player games from netplay)
	int hardware;		// Which type of hardware the game runs on
	int (*GetZipName)(char** pszName, unsigned int i);				// Function to get possible zip names
	int (*GetRomInfo)(struct BurnRomInfo* pri,unsigned int i);		// Function to get the length and crc of each rom
	int (*GetRomName)(char** pszName, unsigned int i, int nAka);	// Function to get the possible names for each rom
	int (*GetInputInfo)(struct BurnInputInfo* pii, unsigned int i);	// Function to get the input info for the game
	int (*GetDIPInfo)(struct BurnDIPInfo* pdi, unsigned int i);		// Function to get the input info for the game
	int (*Init)(); int (*Exit)(); int (*Frame)(); int (*Redraw)(); int (*AreaScan)(int nAction, int* pnMin);
	unsigned char* pRecalcPal;										// Set to 1 if the palette needs to be fully re-calculated
	int nWidth, nHeight; int nXAspect, nYAspect;					// Screen width, height, x/y aspect
};

#define BurnDriverD BurnDriver		// Debug status
#define BurnDriverX BurnDriver		// Exclude from build

// Standard functions for dealing with ROM and input info structures
#include "stdfunc.h"

// ------------------------------------------------------------------

// burn.cpp
int BurnSetRefreshRate(double dRefreshRate);

// Byteswaps an area of memory
static inline uint32_t BurnByteswap(UINT8* pMem, int nLen)
{
	nLen >>= 1;
	for (int32_t i = 0; i < nLen; i++, pMem += 2)
	{
		UINT8 t = pMem[0];
		pMem[0] = pMem[1];
		pMem[1] = t;
	}

	return 0;
}



int BurnClearScreen();

// load.cpp
int BurnLoadRom(UINT8* Dest,int i, int nGap);
int BurnXorRom(UINT8* Dest,int i, int nGap);
int BurnLoadBitField(UINT8* pDest, UINT8* pSrc, int nField, int nSrcLen);

// ------------------------------------------------------------------
// Colour-depth independant image transfer

extern unsigned short* pTransDraw;

void BurnTransferClear();
int BurnTransferCopy(unsigned int* pPalette);
void BurnTransferExit();
int BurnTransferInit();

// ------------------------------------------------------------------
// Plotting pixels

static inline void PutPix(UINT8* pPix, UINT32 c)
{
   #ifndef __CELLOS_LV2__
	if (nBurnBpp >= 4) {
   #endif
		*((UINT32*)pPix) = c;
   #ifndef __CELLOS_LV2__
	} else {
		if (nBurnBpp == 2) {
			*((UINT16*)pPix) = (UINT16)c;
		} else {
			pPix[0] = (UINT8)(c >>  0);
			pPix[1] = (UINT8)(c >>  8);
			pPix[2] = (UINT8)(c >> 16);
		}
	}
   #endif
}

// ------------------------------------------------------------------
// Clear opposites

static inline void DrvClearOpposites(unsigned char* joystickInputs)
{
	if ((*joystickInputs & 0x03) == 0x03)
		*joystickInputs &= ~0x03;

	if ((*joystickInputs & 0x0c) == 0x0c)
		*joystickInputs &= ~0x0c;
}

static inline void DrvClearOpposites(unsigned short* joystickInputs)
{
	if ((*joystickInputs & 0x03) == 0x03)
		*joystickInputs &= ~0x03;

	if ((*joystickInputs & 0x0c) == 0x0c)
		*joystickInputs &= ~0x0c;
}

// ------------------------------------------------------------------
// Setting up cpus for cheats

typedef enum {
	CPU_M68K, CPU_VEZ, CPU_SH2, CPU_M6502, CPU_Z80,
	CPU_M6809, CPU_M6805, CPU_HD6309, CPU_M6800, CPU_S2650,
	CPU_KONAMICUSTOM, CPU_ARM7,
} CPU_CHEATTYPE;

void CpuCheatRegister(CPU_CHEATTYPE type, int num);

#endif
