// Burn - Arcade emulator library - internal code

// Standard headers
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#ifdef __LIBSNES__
#include "port-typedefs.h"
#else
#include "tchar.h"
#endif
#include "burn.h"

// ---------------------------------------------------------------------------
// CPU emulation interfaces
#ifndef LSB_FIRST
#define swapByte(i)(i^1)	// might be needed for addresses (e.g. see some of the drivers that ^1
				// the address on reads) - can't see it being useful for data
#if defined (_XBOX)
#include <PPCINTRINSICS.h>
#define swapWord(i) byteswap_ushort(i)	// swap intrinsics are faster on Xbox 360
#elif defined (__CELLOS_LV2__)
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
	asm("sthbrx     %1,%y0" : "=Z"(*&t) : "r"(x));
        return t;
}
#else
#define swapWord(i)((((i) & 0xff) <<  8) | (((i) & 0xff00) >> 8))
#endif

#if defined  (_XBOX)
#define swapLong(i) _byteswap_ulong(i) // swap intrinsics are faster on Xbox 360
#elif defined (__CELLOS_LV2__)
#define swapLong(i) _byteswap_ulong(i)

static inline uint32_t _byteswap_ulong(uint32_t x)
{
	uint32_t t;
	asm("stwbrx     %1,%y0" : "=Z"(*&t) : "r"(x));
	return t;
}
#else
#define swapLong(i) ((((i) & 0xFF000000) >> 24) | (((i) & 0x00FF0000) >> 8)  | (((i) & 0x0000FF00) << 8)  | (((i) & 0x000000FF) << 24) )
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
	r.l.l = _byteswap_ulong (x);
	r.l.h = _byteswap_ulong (x>>32);
	return r.ll;
}
#endif
#endif

// sek.cpp
#include "sek.h"

// zet.cpp
#include "zet.h"

typedef union
{
#ifdef LSB_FIRST
	struct { UINT8 l,h,h2,h3; } b;
	struct { UINT16 l,h; } w;
#else
	struct { UINT8 h3, h2, h, l; } b;
	struct { UINT16 h,l; } w;
#endif
	UINT32 d;
} PAIR;

// ---------------------------------------------------------------------------
// Driver information

struct BurnMediaInfo { char* szShortName; char* szFullNameA; wchar_t* szFullNameW; };

struct BurnDriver {
	char* szShortName;			// The filename of the zip file (without extension)
	char* szParent;				// The filename of the parent (without extension, NULL if not applicable)
	char* szBoardROM;			// The filename of the board ROMs (without extension, NULL if not applicable)
	char* szSampleName;			// The filename of the samples zip file (without extension, NULL if not applicable)
	char* szDate;

	// szFullNameA, szCommentA, szManufacturerA and szSystemA should always contain valid info
	// szFullNameW, szCommentW, szManufacturerW and szSystemW should be used only if characters or scripts are needed that ASCII can't handle
	char*    szFullNameA; char*    szCommentA; char*    szManufacturerA; char*    szSystemA;
	wchar_t* szFullNameW; wchar_t* szCommentW; wchar_t* szManufacturerW; wchar_t* szSystemW;

	int Flags;			// See burn.h
	int Players;		// Max number of players a game supports (so we can remove single player games from netplay)
	int Hardware;		// Which type of hardware the game runs on
	int Genre;
	int Family;
	BurnMediaInfo* media;
	int (*GetZipName)(char** pszName, unsigned int i);				// Function to get possible zip names
	int (*GetRomInfo)(struct BurnRomInfo* pri,unsigned int i);		// Function to get the length and crc of each rom
	int (*GetRomName)(char** pszName, unsigned int i, int nAka);	// Function to get the possible names for each rom
	int (*GetSampleInfo)(struct BurnSampleInfo* pri,unsigned int i);		// Function to get the sample flags
	int (*GetSampleName)(char** pszName, unsigned int i, int nAka);	// Function to get the possible names for each sample
	int (*GetInputInfo)(struct BurnInputInfo* pii, unsigned int i);	// Function to get the input info for the game
	int (*GetDIPInfo)(struct BurnDIPInfo* pdi, unsigned int i);		// Function to get the input info for the game
	int (*Init)(); int (*Exit)(); int (*Frame)(); int (*Redraw)(); int (*AreaScan)(int nAction, int* pnMin);
	unsigned char* pRecalcPal; unsigned int nPaletteEntries;										// Set to 1 if the palette needs to be fully re-calculated
	int nWidth, nHeight; int nXAspect, nYAspect;					// Screen width, height, x/y aspect
};

#define BurnDriverD BurnDriver		// Debug status
#define BurnDriverX BurnDriver		// Exclude from build

// Standard functions for dealing with ROM and input info structures
#include "stdfunc.h"

// ---------------------------------------------------------------------------

// burn.cpp
int BurnSetRefreshRate(double dRefreshRate);
int BurnByteswap(UINT8* pm,int nLen);
int BurnClearScreen();

// load.cpp
int BurnLoadRom(UINT8* Dest,int i, int nGap);
int BurnXorRom(UINT8* Dest,int i, int nGap);
int BurnLoadBitField(UINT8* pDest, UINT8* pSrc, int nField, int nSrcLen);

// ---------------------------------------------------------------------------
// Colour-depth independant image transfer

extern unsigned short* pTransDraw;

void BurnTransferClear();
int BurnTransferCopy(unsigned int* pPalette);
void BurnTransferExit();
int BurnTransferInit();

// ---------------------------------------------------------------------------
// Plotting pixels

inline static void PutPix(UINT8* pPix, UINT32 c)
{
	if (nBurnBpp >= 4) {
		*((UINT32*)pPix) = c;
	} else {
		if (nBurnBpp == 2) {
			*((UINT16*)pPix) = (UINT16)c;
		} else {
			pPix[0] = (UINT8)(c >>  0);
			pPix[1] = (UINT8)(c >>  8);
			pPix[2] = (UINT8)(c >> 16);
		}
	}
}

// ---------------------------------------------------------------------------
// Setting up cpus for cheats

void CpuCheatRegister(int type, int num);

// burn_memory.cpp
void BurnInitMemoryManager();
unsigned char *BurnMalloc(int size);
void BurnFree(void *ptr);
void BurnExitMemoryManager();
