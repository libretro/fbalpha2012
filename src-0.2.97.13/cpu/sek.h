// 68000 (Sixty Eight K) Interface - header file

#ifndef FASTCALL
 #undef __fastcall
 #define __fastcall
#endif

#if defined BUILD_A68K
 #define EMU_A68K								// Use A68K Assembler 68000 emulator
#endif

#define EMU_M68K								// Use Musashi 68000 emulator

#define SEK_MAX	(4)								// Maximum number of CPUs supported

#if defined EMU_M68K
 #include "m68k.h"
#endif

// Number of bits used for each page in the fast memory map
#define SEK_BITS		(10)					// 10 = 0x0400 page size
#define SEK_PAGE_COUNT  (1 << (24 - SEK_BITS))	// Number of pages
#define SEK_SHIFT		(SEK_BITS)				// Shift value = page bits
#define SEK_PAGE_SIZE	(1 << SEK_BITS)			// Page size
#define SEK_PAGEM		(SEK_PAGE_SIZE - 1)
#define SEK_WADD		(SEK_PAGE_COUNT)		// Value to add for write section = Number of pages
#define SEK_MASK		(SEK_WADD - 1)
#define SEK_MAXHANDLER	(10)						// Max. number of handlers for memory access

#if SEK_MAXHANDLER < 1
 #error At least one set of handlers for memory access must be used.
#endif

#ifdef EMU_A68K
 extern "C" void __cdecl M68000_RUN();
 extern "C" void __cdecl M68000_RESET();
#endif

#ifdef EMU_A68K
 // The format of the data in a68k.asm (at the _M68000_regs location)
 struct A68KContext {
	unsigned int d[8], a[8];
	unsigned int isp, srh, ccr, xc, pc, irq, sr;
	int (*IrqCallback) (int nIrq);
	unsigned int ppc;
	int (*ResetCallback)();
	int (*RTECallback)();
	int (*CmpCallback)(unsigned int val, int reg);
	unsigned int sfc, dfc, usp, vbr;
	unsigned int nAsmBank, nCpuVersion;
 };
 extern "C" struct A68KContext M68000_regs;
 extern     struct A68KContext* SekRegs[SEK_MAX];

 extern "C" unsigned char* OP_ROM;
 extern "C" unsigned char* OP_RAM;

 void __fastcall AsekChangePc(unsigned int pc);
#endif

#ifdef EMU_M68K
 extern "C" int nSekM68KContextSize[SEK_MAX];
 extern "C" char* SekM68KContext[SEK_MAX];
 extern "C" int m68k_ICount;
#endif

typedef unsigned char (__fastcall *pSekReadByteHandler)(unsigned int a);
typedef void (__fastcall *pSekWriteByteHandler)(unsigned int a, unsigned char d);
typedef unsigned short (__fastcall *pSekReadWordHandler)(unsigned int a);
typedef void (__fastcall *pSekWriteWordHandler)(unsigned int a, unsigned short d);
typedef unsigned int (__fastcall *pSekReadLongHandler)(unsigned int a);
typedef void (__fastcall *pSekWriteLongHandler)(unsigned int a, unsigned int d);

typedef int (__fastcall *pSekResetCallback)();
typedef int (__fastcall *pSekRTECallback)();
typedef int (__fastcall *pSekIrqCallback)(int irq);
typedef int (__fastcall *pSekCmpCallback)(unsigned int val, int reg);

extern int nSekCycles[SEK_MAX], nSekCPUType[SEK_MAX];

// Mapped memory pointers to Rom and Ram areas (Read then Write)
// These memory areas must be allocated multiples of the page size
// with a 4 byte over-run area lookup for each page (*3 for read, write and fetch)
struct SekExt {
	unsigned char* MemMap[SEK_PAGE_COUNT * 3];

	// If MemMap[i] < SEK_MAXHANDLER, use the handler functions
	pSekReadByteHandler ReadByte[SEK_MAXHANDLER];
	pSekWriteByteHandler WriteByte[SEK_MAXHANDLER];
	pSekReadWordHandler ReadWord[SEK_MAXHANDLER];
	pSekWriteWordHandler WriteWord[SEK_MAXHANDLER];
	pSekReadLongHandler ReadLong[SEK_MAXHANDLER];
	pSekWriteLongHandler WriteLong[SEK_MAXHANDLER];

	pSekResetCallback ResetCallback;
	pSekRTECallback RTECallback;
	pSekIrqCallback IrqCallback;
	pSekCmpCallback CmpCallback;
};

#define SEK_DEF_READ_WORD(i, a) { unsigned short d; d = (unsigned short)(pSekExt->ReadByte[i](a) << 8); d |= (unsigned short)(pSekExt->ReadByte[i]((a) + 1)); return d; }
#define SEK_DEF_WRITE_WORD(i, a, d) { pSekExt->WriteByte[i]((a),(unsigned char)((d) >> 8)); pSekExt->WriteByte[i]((a) + 1, (unsigned char)((d) & 0xff)); }
#define SEK_DEF_READ_LONG(i, a) { unsigned int d; d = pSekExt->ReadWord[i](a) << 16; d |= pSekExt->ReadWord[i]((a) + 2); return d; }
#define SEK_DEF_WRITE_LONG(i, a, d) { pSekExt->WriteWord[i]((a),(unsigned short)((d) >> 16)); pSekExt->WriteWord[i]((a) + 2,(unsigned short)((d) & 0xffff)); }

extern struct SekExt *SekExt[SEK_MAX], *pSekExt;
extern int nSekActive;										// The cpu which is currently being emulated
extern int nSekCyclesTotal, nSekCyclesScanline, nSekCyclesSegment, nSekCyclesDone, nSekCyclesToDo;

unsigned int SekReadByte(unsigned int a);
unsigned int SekReadWord(unsigned int a);
unsigned int SekReadLong(unsigned int a);

unsigned int SekFetchByte(unsigned int a);
unsigned int SekFetchWord(unsigned int a);
unsigned int SekFetchLong(unsigned int a);

void SekWriteByte(unsigned int a, unsigned char d);
void SekWriteWord(unsigned int a, unsigned short d);
void SekWriteLong(unsigned int a, unsigned int d);

void SekWriteByteROM(unsigned int a, unsigned char d);
void SekWriteWordROM(unsigned int a, unsigned short d);
void SekWriteLongROM(unsigned int a, unsigned int d);

int SekInit(int nCount, int nCPUType);
int SekExit();

void SekNewFrame();
void SekSetCyclesScanline(int nCycles);

void SekClose();
void SekOpen(const int i);
int SekGetActive();

#define SEK_IRQSTATUS_NONE (0x0000)
#define SEK_IRQSTATUS_AUTO (0x2000)
#define SEK_IRQSTATUS_ACK  (0x1000)

void SekSetIRQLine(const int line, const int status);
void SekReset();

void SekRunEnd();
void SekRunAdjust(const int nCycles);
int SekRun(const int nCycles);

inline static int SekIdle(int nCycles)
{
	nSekCyclesTotal += nCycles;

	return nCycles;
}

inline static int SekSegmentCycles()
{
#ifdef EMU_M68K
	return nSekCyclesDone + nSekCyclesToDo - m68k_ICount;
#else
	return nSekCyclesDone + nSekCyclesToDo;
#endif
}

inline static int SekTotalCycles()
{
#ifdef EMU_M68K
	return nSekCyclesTotal + nSekCyclesToDo - m68k_ICount;
#else
	return nSekCyclesTotal + nSekCyclesToDo;
#endif
}

inline static int SekCurrentScanline()
{
	return SekTotalCycles() / nSekCyclesScanline;
}

// SekMemory types:
#define SM_READ  (1)
#define SM_WRITE (2)
#define SM_FETCH (4)
#define SM_ROM (SM_READ | SM_FETCH)
#define SM_RAM (SM_READ | SM_WRITE | SM_FETCH)

// Map areas of memory
int SekMapMemory(unsigned char* pMemory, unsigned int nStart, unsigned int nEnd, int nType);
int SekMapHandler(unsigned int nHandler, unsigned int nStart, unsigned int nEnd, int nType);

// Set handlers
int SekSetReadByteHandler(int i, pSekReadByteHandler pHandler);
int SekSetWriteByteHandler(int i, pSekWriteByteHandler pHandler);
int SekSetReadWordHandler(int i, pSekReadWordHandler pHandler);
int SekSetWriteWordHandler(int i, pSekWriteWordHandler pHandler);
int SekSetReadLongHandler(int i, pSekReadLongHandler pHandler);
int SekSetWriteLongHandler(int i, pSekWriteLongHandler pHandler);

// Set callbacks
int SekSetResetCallback(pSekResetCallback pCallback);
int SekSetRTECallback(pSekRTECallback pCallback);
int SekSetIrqCallback(pSekIrqCallback pCallback);
int SekSetCmpCallback(pSekCmpCallback pCallback);

// Get a CPU's PC
int SekGetPC(int n);

int SekScan(int nAction);
