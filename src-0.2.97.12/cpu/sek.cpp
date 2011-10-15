// 680x0 (Sixty Eight K) Interface
#include "burnint.h"
#include "sekdebug.h"

#ifdef EMU_M68K
int nSekM68KContextSize[SEK_MAX];
char* SekM68KContext[SEK_MAX];
#endif

int nSekCount = -1;							// Number of allocated 68000s
struct SekExt *SekExt[SEK_MAX] = { NULL, }, *pSekExt = NULL;

int nSekActive = -1;								// The cpu which is currently being emulated
int nSekCyclesTotal, nSekCyclesScanline, nSekCyclesSegment, nSekCyclesDone, nSekCyclesToDo;

int nSekCPUType[SEK_MAX], nSekCycles[SEK_MAX], nSekIRQPending[SEK_MAX];

#if defined (FBA_DEBUG)

void (*SekDbgBreakpointHandlerRead)(unsigned int, int);
void (*SekDbgBreakpointHandlerFetch)(unsigned int, int);
void (*SekDbgBreakpointHandlerWrite)(unsigned int, int);

unsigned int (*SekDbgFetchByteDisassembler)(unsigned int);
unsigned int (*SekDbgFetchWordDisassembler)(unsigned int);
unsigned int (*SekDbgFetchLongDisassembler)(unsigned int);

static struct { unsigned int address; int id; } BreakpointDataRead[9]  = { { 0, 0 }, };
static struct { unsigned int address; int id; } BreakpointDataWrite[9] = { { 0, 0 }, };
static struct { unsigned int address; int id; } BreakpointFetch[9] = { { 0, 0 }, };

#endif

#if defined (EMU_A68K)
static void UpdateA68KContext()
{
	if (M68000_regs.srh & 20) {		// Supervisor mode
		M68000_regs.isp = M68000_regs.a[7];
	} else {						// User mode
		M68000_regs.usp = M68000_regs.a[7];
	}

	M68000_regs.sr  = (M68000_regs.srh <<  8) & 0xFF00;	// T, S, M, I
	M68000_regs.sr |= (M68000_regs.xc  <<  4) & 0x0010;	// X
	M68000_regs.sr |= (M68000_regs.ccr >>  4) & 0x0008;	// N
	M68000_regs.sr |= (M68000_regs.ccr >>  4) & 0x0004;	// Z
	M68000_regs.sr |= (M68000_regs.ccr >> 10) & 0x0002;	// V
	M68000_regs.sr |= (M68000_regs.ccr      ) & 0x0001;	// C
}

static unsigned int GetA68KSR()
{
	UpdateA68KContext();

	return M68000_regs.sr;
}

static unsigned int GetA68KISP()
{
	UpdateA68KContext();

	return M68000_regs.isp;
}

static unsigned int GetA68KUSP()
{
	UpdateA68KContext();

	return M68000_regs.usp;
}
#endif

#if defined (FBA_DEBUG)

inline static void CheckBreakpoint_R(unsigned int a, const unsigned int m)
{
	a &= m;

	for (int i = 0; BreakpointDataRead[i].address; i++) {
		if ((BreakpointDataRead[i].address & m) == a) {

#ifdef EMU_A68K
			UpdateA68KContext();
#endif

			SekDbgBreakpointHandlerRead(a, BreakpointDataRead[i].id);
			return;
		}
	}
}

inline static void CheckBreakpoint_W(unsigned int a, const unsigned int m)
{
	a &= m;

	for (int i = 0; BreakpointDataWrite[i].address; i++) {
		if ((BreakpointDataWrite[i].address & m) == a) {

#ifdef EMU_A68K
			UpdateA68KContext();
#endif

			SekDbgBreakpointHandlerWrite(a, BreakpointDataWrite[i].id);
			return;
		}
	}
}

inline static void CheckBreakpoint_PC()
{
	for (int i = 0; BreakpointFetch[i].address; i++) {
		if (BreakpointFetch[i].address == (unsigned int)SekGetPC(-1)) {

#ifdef EMU_A68K
			UpdateA68KContext();
#endif

			SekDbgBreakpointHandlerFetch(SekGetPC(-1), BreakpointFetch[i].id);
			return;
		}
	}
}

inline static void SingleStep_PC()
{
#ifdef EMU_A68K
	UpdateA68KContext();
#endif

	SekDbgBreakpointHandlerFetch(SekGetPC(-1), 0);
}

#endif

// ----------------------------------------------------------------------------
// Default memory access handlers

unsigned char __fastcall DefReadByte(unsigned int) { return 0; }
void __fastcall DefWriteByte(unsigned int, unsigned char) { }

#define DEFWORDHANDLERS(i)																				\
	unsigned short __fastcall DefReadWord##i(unsigned int a) { SEK_DEF_READ_WORD(i, a) }				\
	void __fastcall DefWriteWord##i(unsigned int a, unsigned short d) { SEK_DEF_WRITE_WORD(i, a ,d) }
#define DEFLONGHANDLERS(i)																				\
	unsigned int __fastcall DefReadLong##i(unsigned int a) { SEK_DEF_READ_LONG(i, a) }					\
	void __fastcall DefWriteLong##i(unsigned int a, unsigned int d) { SEK_DEF_WRITE_LONG(i, a , d) }

DEFWORDHANDLERS(0)
DEFLONGHANDLERS(0)

#if SEK_MAXHANDLER >= 2
 DEFWORDHANDLERS(1)
 DEFLONGHANDLERS(1)
#endif

#if SEK_MAXHANDLER >= 3
 DEFWORDHANDLERS(2)
 DEFLONGHANDLERS(2)
#endif

#if SEK_MAXHANDLER >= 4
 DEFWORDHANDLERS(3)
 DEFLONGHANDLERS(3)
#endif

#if SEK_MAXHANDLER >= 5
 DEFWORDHANDLERS(4)
 DEFLONGHANDLERS(4)
#endif

#if SEK_MAXHANDLER >= 6
 DEFWORDHANDLERS(5)
 DEFLONGHANDLERS(5)
#endif

#if SEK_MAXHANDLER >= 7
 DEFWORDHANDLERS(6)
 DEFLONGHANDLERS(6)
#endif

#if SEK_MAXHANDLER >= 8
 DEFWORDHANDLERS(7)
 DEFLONGHANDLERS(7)
#endif

#if SEK_MAXHANDLER >= 9
 DEFWORDHANDLERS(8)
 DEFLONGHANDLERS(8)
#endif

#if SEK_MAXHANDLER >= 10
 DEFWORDHANDLERS(9)
 DEFLONGHANDLERS(9)
#endif

// ----------------------------------------------------------------------------
// Memory access functions

// Mapped Memory lookup (               for read)
#define FIND_R(x) pSekExt->MemMap[ x >> SEK_SHIFT]
// Mapped Memory lookup (+ SEK_WADD     for write)
#define FIND_W(x) pSekExt->MemMap[(x >> SEK_SHIFT) + SEK_WADD]
// Mapped Memory lookup (+ SEK_WADD * 2 for fetch)
#define FIND_F(x) pSekExt->MemMap[(x >> SEK_SHIFT) + SEK_WADD * 2]

// Normal memory access functions
inline static unsigned char ReadByte(unsigned int a)
{
	unsigned char* pr;

	a &= 0xFFFFFF;

//	bprintf(PRINT_NORMAL, _T("read8 0x%08X\n"), a);

	pr = FIND_R(a);
	if ((unsigned int)pr >= SEK_MAXHANDLER) {
		a ^= 1;
		return pr[a & SEK_PAGEM];
	}
	return pSekExt->ReadByte[(unsigned int)pr](a);
}

inline static unsigned char FetchByte(unsigned int a)
{
	unsigned char* pr;

	a &= 0xFFFFFF;

//	bprintf(PRINT_NORMAL, _T("fetch8 0x%08X\n"), a);

	pr = FIND_F(a);
	if ((unsigned int)pr >= SEK_MAXHANDLER) {
		a ^= 1;
		return pr[a & SEK_PAGEM];
	}
	return pSekExt->ReadByte[(unsigned int)pr](a);
}

inline static void WriteByte(unsigned int a, unsigned char d)
{
	unsigned char* pr;

	a &= 0xFFFFFF;

//	bprintf(PRINT_NORMAL, _T("write8 0x%08X\n"), a);

	pr = FIND_W(a);
	if ((unsigned int)pr >= SEK_MAXHANDLER) {
		a ^= 1;
		pr[a & SEK_PAGEM] = (unsigned char)d;
		return;
	}
	pSekExt->WriteByte[(unsigned int)pr](a, d);
}

inline static void WriteByteROM(unsigned int a, unsigned char d)
{
	unsigned char* pr;

	a &= 0xFFFFFF;

	pr = FIND_R(a);
	if ((unsigned int)pr >= SEK_MAXHANDLER) {
		a ^= 1;
		pr[a & SEK_PAGEM] = (unsigned char)d;
		return;
	}
	pSekExt->WriteByte[(unsigned int)pr](a, d);
}

inline static unsigned short ReadWord(unsigned int a)
{
	unsigned char* pr;

	a &= 0xFFFFFF;

//	bprintf(PRINT_NORMAL, _T("read16 0x%08X\n"), a);

	pr = FIND_R(a);
	if ((unsigned int)pr >= SEK_MAXHANDLER) {
		return *((unsigned short*)(pr + (a & SEK_PAGEM)));
	}
	return pSekExt->ReadWord[(unsigned int)pr](a);
}

inline static unsigned short FetchWord(unsigned int a)
{
	unsigned char* pr;

	a &= 0xFFFFFF;

//	bprintf(PRINT_NORMAL, _T("fetch16 0x%08X\n"), a);

	pr = FIND_F(a);
	if ((unsigned int)pr >= SEK_MAXHANDLER) {
		return *((unsigned short*)(pr + (a & SEK_PAGEM)));
	}
	return pSekExt->ReadWord[(unsigned int)pr](a);
}

inline static void WriteWord(unsigned int a, unsigned short d)
{
	unsigned char* pr;

	a &= 0xFFFFFF;

//	bprintf(PRINT_NORMAL, _T("write16 0x%08X\n"), a);

	pr = FIND_W(a);
	if ((unsigned int)pr >= SEK_MAXHANDLER) {
		*((unsigned short*)(pr + (a & SEK_PAGEM))) = (unsigned short)d;
		return;
	}
	pSekExt->WriteWord[(unsigned int)pr](a, d);
}

inline static void WriteWordROM(unsigned int a, unsigned short d)
{
	unsigned char* pr;

	a &= 0xFFFFFF;

	pr = FIND_R(a);
	if ((unsigned int)pr >= SEK_MAXHANDLER) {
		*((unsigned short*)(pr + (a & SEK_PAGEM))) = (unsigned short)d;
		return;
	}
	pSekExt->WriteWord[(unsigned int)pr](a, d);
}

inline static unsigned int ReadLong(unsigned int a)
{
	unsigned char* pr;

	a &= 0xFFFFFF;

//	bprintf(PRINT_NORMAL, _T("read32 0x%08X\n"), a);

	pr = FIND_R(a);
	if ((unsigned int)pr >= SEK_MAXHANDLER) {
		unsigned int r = *((unsigned int*)(pr + (a & SEK_PAGEM)));
		r = (r >> 16) | (r << 16);
		return r;
	}
	return pSekExt->ReadLong[(unsigned int)pr](a);
}

inline static unsigned int FetchLong(unsigned int a)
{
	unsigned char* pr;

	a &= 0xFFFFFF;

//	bprintf(PRINT_NORMAL, _T("fetch32 0x%08X\n"), a);

	pr = FIND_F(a);
	if ((unsigned int)pr >= SEK_MAXHANDLER) {
		unsigned int r = *((unsigned int*)(pr + (a & SEK_PAGEM)));
		r = (r >> 16) | (r << 16);
		return r;
	}
	return pSekExt->ReadLong[(unsigned int)pr](a);
}

inline static void WriteLong(unsigned int a, unsigned int d)
{
	unsigned char* pr;

	a &= 0xFFFFFF;

//	bprintf(PRINT_NORMAL, _T("write32 0x%08X\n"), a);

	pr = FIND_W(a);
	if ((unsigned int)pr >= SEK_MAXHANDLER) {
		d = (d >> 16) | (d << 16);
		*((unsigned int*)(pr + (a & SEK_PAGEM))) = d;
		return;
	}
	pSekExt->WriteLong[(unsigned int)pr](a, d);
}

inline static void WriteLongROM(unsigned int a, unsigned int d)
{
	unsigned char* pr;

	a &= 0xFFFFFF;

	pr = FIND_R(a);
	if ((unsigned int)pr >= SEK_MAXHANDLER) {
		d = (d >> 16) | (d << 16);
		*((unsigned int*)(pr + (a & SEK_PAGEM))) = d;
		return;
	}
	pSekExt->WriteLong[(unsigned int)pr](a, d);
}

#if defined (FBA_DEBUG)

// Breakpoint checking memory access functions
unsigned char __fastcall ReadByteBP(unsigned int a)
{
	unsigned char* pr;

	a &= 0xFFFFFF;

	pr = FIND_R(a);

	CheckBreakpoint_R(a, ~0);

	if ((unsigned int)pr >= SEK_MAXHANDLER) {
		a ^= 1;
		return pr[a & SEK_PAGEM];
	}
	return pSekExt->ReadByte[(unsigned int)pr](a);
}

void __fastcall WriteByteBP(unsigned int a, unsigned char d)
{
	unsigned char* pr;

	a &= 0xFFFFFF;

	pr = FIND_W(a);

	CheckBreakpoint_W(a, ~0);

	if ((unsigned int)pr >= SEK_MAXHANDLER) {
		a ^= 1;
		pr[a & SEK_PAGEM] = (unsigned char)d;
		return;
	}
	pSekExt->WriteByte[(unsigned int)pr](a, d);
}

unsigned short __fastcall ReadWordBP(unsigned int a)
{
	unsigned char* pr;

	a &= 0xFFFFFF;

	pr = FIND_R(a);

	CheckBreakpoint_R(a, ~1);

	if ((unsigned int)pr >= SEK_MAXHANDLER) {
		return *((unsigned short*)(pr + (a & SEK_PAGEM)));
	}
	return pSekExt->ReadWord[(unsigned int)pr](a);
}

void __fastcall WriteWordBP(unsigned int a, unsigned short d)
{
	unsigned char* pr;

	a &= 0xFFFFFF;

	pr = FIND_W(a);

	CheckBreakpoint_W(a, ~1);

	if ((unsigned int)pr >= SEK_MAXHANDLER) {
		*((unsigned short*)(pr + (a & SEK_PAGEM))) = (unsigned short)d;
		return;
	}
	pSekExt->WriteWord[(unsigned int)pr](a, d);
}

unsigned int __fastcall ReadLongBP(unsigned int a)
{
	unsigned char* pr;

	a &= 0xFFFFFF;

	pr = FIND_R(a);

	CheckBreakpoint_R(a, ~1);

	if ((unsigned int)pr >= SEK_MAXHANDLER) {
		unsigned int r = *((unsigned int*)(pr + (a & SEK_PAGEM)));
		r = (r >> 16) | (r << 16);
		return r;
	}
	return pSekExt->ReadLong[(unsigned int)pr](a);
}

void __fastcall WriteLongBP(unsigned int a, unsigned int d)
{
	unsigned char* pr;

	a &= 0xFFFFFF;

	pr = FIND_W(a);

	CheckBreakpoint_W(a, ~1);

	if ((unsigned int)pr >= SEK_MAXHANDLER) {
		d = (d >> 16) | (d << 16);
		*((unsigned int*)(pr + (a & SEK_PAGEM))) = d;
		return;
	}
	pSekExt->WriteLong[(unsigned int)pr](a, d);
}

#endif

// ----------------------------------------------------------------------------
// A68K variables

#ifdef EMU_A68K
struct A68KContext* SekRegs[SEK_MAX] = { NULL, };
#endif

struct A68KInter {
	void (__fastcall *DebugCallback) ();
	unsigned char  (__fastcall *Read8) (unsigned int a);
	unsigned short (__fastcall *Read16)(unsigned int a);
	unsigned int   (__fastcall *Read32)(unsigned int a);
	void (__fastcall *Write8)  (unsigned int a, unsigned char d);
	void (__fastcall *Write16) (unsigned int a, unsigned short d);
	void (__fastcall *Write32) (unsigned int a, unsigned int d);
	void (__fastcall *ChangePc)(unsigned int a);
	unsigned char  (__fastcall *PcRel8) (unsigned int a);
	unsigned short (__fastcall *PcRel16)(unsigned int a);
	unsigned int   (__fastcall *PcRel32)(unsigned int a);
	unsigned short (__fastcall *Dir16)(unsigned int a);
	unsigned int   (__fastcall *Dir32)(unsigned int a);
};

extern "C" {

#ifdef EMU_A68K
 unsigned char* OP_ROM = NULL;
 unsigned char* OP_RAM = NULL;

#ifndef EMU_M68K
 int m68k_ICount = 0;
#endif

 unsigned int mem_amask = 0xFFFFFF;			// 24-bit bus
#endif

 unsigned int mame_debug = 0, cur_mrhard = 0, m68k_illegal_opcode = 0, illegal_op = 0, illegal_pc = 0, opcode_entry = 0;

 struct A68KInter a68k_memory_intf;
}

unsigned char  __fastcall A68KRead8 (unsigned int a) { return ReadByte(a);}
unsigned short __fastcall A68KRead16(unsigned int a) { return ReadWord(a);}
unsigned int   __fastcall A68KRead32(unsigned int a) { return ReadLong(a);}
unsigned char  __fastcall A68KFetch8 (unsigned int a) { return FetchByte(a);}
unsigned short __fastcall A68KFetch16(unsigned int a) { return FetchWord(a);}
unsigned int   __fastcall A68KFetch32(unsigned int a) { return FetchLong(a);}
void __fastcall A68KWrite8 (unsigned int a,unsigned char d)  { WriteByte(a,d);}
void __fastcall A68KWrite16(unsigned int a,unsigned short d) { WriteWord(a,d);}
void __fastcall A68KWrite32(unsigned int a,unsigned int d)   { WriteLong(a,d);}

#if defined (FBA_DEBUG)
void __fastcall A68KCheckBreakpoint() { CheckBreakpoint_PC(); }
void __fastcall A68KSingleStep() { SingleStep_PC(); }
#endif

#ifdef EMU_A68K
void __fastcall A68KChangePC(unsigned int pc)
{
	pc &= 0xFFFFFF;

	// Adjust OP_ROM to the current bank
	OP_ROM = FIND_F(pc) - (pc & ~SEK_PAGEM);

	// Set the current bank number
	M68000_regs.nAsmBank = pc >> SEK_BITS;
}
#endif

#ifdef EMU_M68K
extern "C" {
unsigned int __fastcall M68KReadByte(unsigned int a) { return (unsigned int)ReadByte(a); }
unsigned int __fastcall M68KReadWord(unsigned int a) { return (unsigned int)ReadWord(a); }
unsigned int __fastcall M68KReadLong(unsigned int a) { return               ReadLong(a); }

unsigned int __fastcall M68KFetchByte(unsigned int a) { return (unsigned int)FetchByte(a); }
unsigned int __fastcall M68KFetchWord(unsigned int a) { return (unsigned int)FetchWord(a); }
unsigned int __fastcall M68KFetchLong(unsigned int a) { return               FetchLong(a); }

#ifdef FBA_DEBUG
unsigned int __fastcall M68KReadByteBP(unsigned int a) { return (unsigned int)ReadByteBP(a); }
unsigned int __fastcall M68KReadWordBP(unsigned int a) { return (unsigned int)ReadWordBP(a); }
unsigned int __fastcall M68KReadLongBP(unsigned int a) { return               ReadLongBP(a); }

void __fastcall M68KWriteByteBP(unsigned int a, unsigned int d) { WriteByteBP(a, d); }
void __fastcall M68KWriteWordBP(unsigned int a, unsigned int d) { WriteWordBP(a, d); }
void __fastcall M68KWriteLongBP(unsigned int a, unsigned int d) { WriteLongBP(a, d); }

void M68KCheckBreakpoint() { CheckBreakpoint_PC(); }
void M68KSingleStep() { SingleStep_PC(); }

unsigned int (__fastcall *M68KReadByteDebug)(unsigned int);
unsigned int (__fastcall *M68KReadWordDebug)(unsigned int);
unsigned int (__fastcall *M68KReadLongDebug)(unsigned int);

void (__fastcall *M68KWriteByteDebug)(unsigned int, unsigned int);
void (__fastcall *M68KWriteWordDebug)(unsigned int, unsigned int);
void (__fastcall *M68KWriteLongDebug)(unsigned int, unsigned int);
#endif

void __fastcall M68KWriteByte(unsigned int a, unsigned int d) { WriteByte(a, d); }
void __fastcall M68KWriteWord(unsigned int a, unsigned int d) { WriteWord(a, d); }
void __fastcall M68KWriteLong(unsigned int a, unsigned int d) { WriteLong(a, d); }
}
#endif

#if defined EMU_A68K
struct A68KInter a68k_inter_normal = {
	NULL,
	A68KRead8,
	A68KRead16,
	A68KRead32,
	A68KWrite8,
	A68KWrite16,
	A68KWrite32,
	A68KChangePC,
	A68KFetch8,
	A68KFetch16,
	A68KFetch32,
	A68KRead16,	// unused
	A68KRead32,	// unused
};

#if defined (FBA_DEBUG)

struct A68KInter a68k_inter_breakpoint = {
	NULL,
	ReadByteBP,
	ReadWordBP,
	ReadLongBP,
	WriteByteBP,
	WriteWordBP,
	WriteLongBP,
	A68KChangePC,
	A68KFetch8,
	A68KFetch16,
	A68KFetch32,
	A68KRead16,	// unused
	A68KRead32,	// unused
};

#endif

#endif

// ----------------------------------------------------------------------------
// Memory accesses (non-emu specific)

unsigned int SekReadByte(unsigned int a) { return (unsigned int)ReadByte(a); }
unsigned int SekReadWord(unsigned int a) { return (unsigned int)ReadWord(a); }
unsigned int SekReadLong(unsigned int a) { return ReadLong(a); }

unsigned int SekFetchByte(unsigned int a) { return (unsigned int)FetchByte(a); }
unsigned int SekFetchWord(unsigned int a) { return (unsigned int)FetchWord(a); }
unsigned int SekFetchLong(unsigned int a) { return FetchLong(a); }

void SekWriteByte(unsigned int a, unsigned char d) { WriteByte(a, d); }
void SekWriteWord(unsigned int a, unsigned short d) { WriteWord(a, d); }
void SekWriteLong(unsigned int a, unsigned int d) { WriteLong(a, d); }

void SekWriteByteROM(unsigned int a, unsigned char d) { WriteByteROM(a, d); }
void SekWriteWordROM(unsigned int a, unsigned short d) { WriteWordROM(a, d); }
void SekWriteLongROM(unsigned int a, unsigned int d) { WriteLongROM(a, d); }

// ----------------------------------------------------------------------------
// Callbacks for A68K

#ifdef EMU_A68K
static int A68KIRQAcknowledge(int nIRQ)
{
	if (nSekIRQPending[nSekActive] & SEK_IRQSTATUS_AUTO) {
		M68000_regs.irq &= 0x78;
		nSekIRQPending[nSekActive] = 0;
	}

	nSekIRQPending[nSekActive] = 0;
	
	if (pSekExt->IrqCallback) {
		return pSekExt->IrqCallback(nIRQ);
	}

	return -1;
}

static int A68KResetCallback()
{
	if (pSekExt->ResetCallback == NULL) {
		return 0;
	}
	return pSekExt->ResetCallback();
}

static int A68KRTECallback()
{
	if (pSekExt->RTECallback == NULL) {
		return 0;
	}
	return pSekExt->RTECallback();
}

static int A68KCmpCallback(unsigned int val, int reg)
{
	if (pSekExt->CmpCallback == NULL) {
		return 0;
	}
	return pSekExt->CmpCallback(val, reg);
}

static int SekSetup(struct A68KContext* psr)
{
	psr->IrqCallback = A68KIRQAcknowledge;
	psr->ResetCallback = A68KResetCallback;
	psr->RTECallback = A68KRTECallback;
	psr->CmpCallback = A68KCmpCallback;

	return 0;
}
#endif

// ----------------------------------------------------------------------------
// Callbacks for Musashi

#ifdef EMU_M68K
extern "C" int M68KIRQAcknowledge(int nIRQ)
{
	if (nSekIRQPending[nSekActive] & SEK_IRQSTATUS_AUTO) {
		m68k_set_irq(0);
		nSekIRQPending[nSekActive] = 0;
	}
	
	if (pSekExt->IrqCallback) {
		return pSekExt->IrqCallback(nIRQ);
	}

	return M68K_INT_ACK_AUTOVECTOR;
}

extern "C" void M68KResetCallback()
{
	if (pSekExt->ResetCallback) {
		pSekExt->ResetCallback();
	}
}

extern "C" void M68KRTECallback()
{
	if (pSekExt->RTECallback) {
		pSekExt->RTECallback();
	}
}

extern "C" void M68KcmpildCallback(unsigned int val, int reg)
{
	if (pSekExt->CmpCallback) {
		pSekExt->CmpCallback(val, reg);
	}
}
#endif

// ----------------------------------------------------------------------------
// Initialisation/exit/reset

#ifdef EMU_A68K
static int SekInitCPUA68K(int nCount, int nCPUType)
{
	if (nCPUType != 0x68000) {
		return 1;
	}

	nSekCPUType[nCount] = 0;

	// Allocate emu-specific cpu states
	SekRegs[nCount] = (struct A68KContext*)malloc(sizeof(struct A68KContext));
	if (SekRegs[nCount] == NULL) {
		return 1;
	}

	// Setup each cpu context
	memset(SekRegs[nCount], 0, sizeof(struct A68KContext));
	SekSetup(SekRegs[nCount]);

	// Init cpu emulator
	M68000_RESET();

	return 0;
}
#endif

#ifdef EMU_M68K
static int SekInitCPUM68K(int nCount, int nCPUType)
{
	nSekCPUType[nCount] = nCPUType;

	switch (nCPUType) {
		case 0x68000:
			m68k_set_cpu_type(M68K_CPU_TYPE_68000);
			break;
		case 0x68010:
			m68k_set_cpu_type(M68K_CPU_TYPE_68010);
			break;
		case 0x68EC020:
			m68k_set_cpu_type(M68K_CPU_TYPE_68EC020);
			break;
		default:
			return 1;
	}

	nSekM68KContextSize[nCount] = m68k_context_size();
	SekM68KContext[nCount] = (char*)malloc(nSekM68KContextSize[nCount]);
	if (SekM68KContext[nCount] == NULL) {
		return 1;
	}
	memset(SekM68KContext[nCount], 0, nSekM68KContextSize[nCount]);
	m68k_get_context(SekM68KContext[nCount]);

	return 0;
}
#endif

void SekNewFrame()
{
	for (int i = 0; i <= nSekCount; i++) {
		nSekCycles[i] = 0;
	}

	nSekCyclesTotal = 0;
}

void SekSetCyclesScanline(int nCycles)
{
	nSekCyclesScanline = nCycles;
}

int SekInit(int nCount, int nCPUType)
{
	struct SekExt* ps = NULL;

#if !defined BUILD_A68K
	bBurnUseASMCPUEmulation = false;
#endif

	if (nSekActive >= 0) {
		SekClose();
		nSekActive = -1;
	}

	if (nCount > nSekCount) {
		nSekCount = nCount;
	}

	// Allocate cpu extenal data (memory map etc)
	SekExt[nCount] = (struct SekExt*)malloc(sizeof(struct SekExt));
	if (SekExt[nCount] == NULL) {
		SekExit();
		return 1;
	}
	memset(SekExt[nCount], 0, sizeof(struct SekExt));

	// Put in default memory handlers
	ps = SekExt[nCount];

	for (int j = 0; j < SEK_MAXHANDLER; j++) {
		ps->ReadByte[j]  = DefReadByte;
		ps->WriteByte[j] = DefWriteByte;
	}

	ps->ReadWord[0]  = DefReadWord0;
	ps->WriteWord[0] = DefWriteWord0;
	ps->ReadLong[0]  = DefReadLong0;
	ps->WriteLong[0] = DefWriteLong0;

#if SEK_MAXHANDLER >= 2
	ps->ReadWord[1]  = DefReadWord1;
	ps->WriteWord[1] = DefWriteWord1;
	ps->ReadLong[1]  = DefReadLong1;
	ps->WriteLong[1] = DefWriteLong1;
#endif

#if SEK_MAXHANDLER >= 3
	ps->ReadWord[2]  = DefReadWord2;
	ps->WriteWord[2] = DefWriteWord2;
	ps->ReadLong[2]  = DefReadLong2;
	ps->WriteLong[2] = DefWriteLong2;
#endif

#if SEK_MAXHANDLER >= 4
	ps->ReadWord[3]  = DefReadWord3;
	ps->WriteWord[3] = DefWriteWord3;
	ps->ReadLong[3]  = DefReadLong3;
	ps->WriteLong[3] = DefWriteLong3;
#endif

#if SEK_MAXHANDLER >= 5
	ps->ReadWord[4]  = DefReadWord4;
	ps->WriteWord[4] = DefWriteWord4;
	ps->ReadLong[4]  = DefReadLong4;
	ps->WriteLong[4] = DefWriteLong4;
#endif

#if SEK_MAXHANDLER >= 6
	ps->ReadWord[5]  = DefReadWord5;
	ps->WriteWord[5] = DefWriteWord5;
	ps->ReadLong[5]  = DefReadLong5;
	ps->WriteLong[5] = DefWriteLong5;
#endif

#if SEK_MAXHANDLER >= 7
	ps->ReadWord[6]  = DefReadWord6;
	ps->WriteWord[6] = DefWriteWord6;
	ps->ReadLong[6]  = DefReadLong6;
	ps->WriteLong[6] = DefWriteLong6;
#endif

#if SEK_MAXHANDLER >= 8
	ps->ReadWord[7]  = DefReadWord7;
	ps->WriteWord[7] = DefWriteWord7;
	ps->ReadLong[7]  = DefReadLong7;
	ps->WriteLong[7] = DefWriteLong7;
#endif

#if SEK_MAXHANDLER >= 9
	ps->ReadWord[8]  = DefReadWord8;
	ps->WriteWord[8] = DefWriteWord8;
	ps->ReadLong[8]  = DefReadLong8;
	ps->WriteLong[8] = DefWriteLong8;
#endif

#if SEK_MAXHANDLER >= 10
	ps->ReadWord[9]  = DefReadWord9;
	ps->WriteWord[9] = DefWriteWord9;
	ps->ReadLong[9]  = DefReadLong9;
	ps->WriteLong[9] = DefWriteLong9;
#endif

#if SEK_MAXHANDLER >= 11
	for (int j = 10; j < SEK_MAXHANDLER; j++) {
		ps->ReadWord[j]  = DefReadWord0;
		ps->WriteWord[j] = DefWriteWord0;
		ps->ReadLong[j]  = DefReadLong0;
		ps->WriteLong[j] = DefWriteLong0;
	}
#endif

	// Map the normal memory handlers
	SekDbgDisableBreakpoints();

#ifdef EMU_A68K
	if (bBurnUseASMCPUEmulation && nCPUType == 0x68000) {
		if (SekInitCPUA68K(nCount, nCPUType)) {
			SekExit();
			return 1;
		}
	} else {
#endif

#ifdef EMU_M68K
		m68k_init();
		if (SekInitCPUM68K(nCount, nCPUType)) {
			SekExit();
			return 1;
		}
#endif

#ifdef EMU_A68K
	}
#endif

	nSekCycles[nCount] = 0;
	nSekIRQPending[nCount] = 0;

	nSekCyclesTotal = 0;
	nSekCyclesScanline = 0;

	CpuCheatRegister(0x0000, nCount);

	return 0;
}

#ifdef EMU_A68K
static void SekCPUExitA68K(int i)
{
	free(SekRegs[i]);
	SekRegs[i] = NULL;
}
#endif

#ifdef EMU_M68K
static void SekCPUExitM68K(int i)
{
		free(SekM68KContext[i]);
		SekM68KContext[i] = NULL;
}
#endif

int SekExit()
{
	// Deallocate cpu extenal data (memory map etc)
	for (int i = 0; i <= nSekCount; i++) {

#ifdef EMU_A68K
		SekCPUExitA68K(i);
#endif

#ifdef EMU_M68K
		SekCPUExitM68K(i);
#endif

		// Deallocate other context data
		free(SekExt[i]);
		SekExt[i] = NULL;
	}

	pSekExt = NULL;

	nSekActive = -1;
	nSekCount = -1;

	return 0;
}

void SekReset()
{

#ifdef EMU_A68K
	if (nSekCPUType[nSekActive] == 0) {
		// A68K has no internal support for resetting the processor, so do what's needed ourselves
		M68000_regs.a[7] = FetchLong(0);	// Get initial stackpointer (register A7)
		M68000_regs.pc = FetchLong(4);		// Get initial PC
		M68000_regs.srh = 0x27;				// start in supervisor state
		A68KChangePC(M68000_regs.pc);
	} else {
#endif

#ifdef EMU_M68K
		m68k_pulse_reset();
#endif

#ifdef EMU_A68K
	}
#endif

}

// ----------------------------------------------------------------------------
// Control the active CPU

// Open a CPU
void SekOpen(const int i)
{
	if (i != nSekActive) {
		nSekActive = i;

		pSekExt = SekExt[nSekActive];						// Point to cpu context

#ifdef EMU_A68K
		if (nSekCPUType[nSekActive] == 0) {
			memcpy(&M68000_regs, SekRegs[nSekActive], sizeof(M68000_regs));
			A68KChangePC(M68000_regs.pc);
		} else {
#endif

#ifdef EMU_M68K
			m68k_set_context(SekM68KContext[nSekActive]);
#endif

#ifdef EMU_A68K
		}
#endif

		nSekCyclesTotal = nSekCycles[nSekActive];
	}
}

// Close the active cpu
void SekClose()
{

#ifdef EMU_A68K
	if (nSekCPUType[nSekActive] == 0) {
		memcpy(SekRegs[nSekActive], &M68000_regs, sizeof(M68000_regs));
	} else {
#endif

#ifdef EMU_M68K
		m68k_get_context(SekM68KContext[nSekActive]);
#endif

#ifdef EMU_A68K
	}
#endif

	nSekCycles[nSekActive] = nSekCyclesTotal;
}

// Get the current CPU
int SekGetActive()
{
	return nSekActive;
}

// Set the status of an IRQ line on the active CPU
void SekSetIRQLine(const int line, const int status)
{
//	bprintf(PRINT_NORMAL, _T("  - irq line %i -> %i\n"), line, status);

	if (status) {
		nSekIRQPending[nSekActive] = line | status;

#ifdef EMU_A68K
		if (nSekCPUType[nSekActive] == 0) {
			nSekCyclesTotal += (nSekCyclesToDo - nSekCyclesDone) - m68k_ICount;
			nSekCyclesDone += (nSekCyclesToDo - nSekCyclesDone) - m68k_ICount;

			M68000_regs.irq = line;
			m68k_ICount = nSekCyclesToDo = -1;					// Force A68K to exit
		} else {
#endif

#ifdef EMU_M68K
			m68k_set_irq(line);
#endif

#ifdef EMU_A68K
		}
#endif

		return;
	}

	nSekIRQPending[nSekActive] = 0;

#ifdef EMU_A68K
	if (nSekCPUType[nSekActive] == 0) {
		M68000_regs.irq &= 0x78;
	} else {
#endif

#ifdef EMU_M68K
		m68k_set_irq(0);
#endif

#ifdef EMU_A68K
	}
#endif

}

// Adjust the active CPU's timeslice
void SekRunAdjust(const int nCycles)
{
	if (nCycles < 0 && m68k_ICount < -nCycles) {
		SekRunEnd();
		return;
	}

#ifdef EMU_A68K
	if (nSekCPUType[nSekActive] == 0) {
		m68k_ICount += nCycles;
		nSekCyclesToDo += nCycles;
		nSekCyclesSegment += nCycles;
	} else {
#endif

#ifdef EMU_M68K
		nSekCyclesToDo += nCycles;
		m68k_modify_timeslice(nCycles);
#endif

#ifdef EMU_A68K
	}
#endif

}

// End the active CPU's timeslice
void SekRunEnd()
{

#ifdef EMU_A68K
	if (nSekCPUType[nSekActive] == 0) {
		nSekCyclesTotal += (nSekCyclesToDo - nSekCyclesDone) - m68k_ICount;
		nSekCyclesDone += (nSekCyclesToDo - nSekCyclesDone) - m68k_ICount;
		nSekCyclesSegment = nSekCyclesDone;
		m68k_ICount = nSekCyclesToDo = -1;						// Force A68K to exit
	} else {
#endif

#ifdef EMU_M68K
		m68k_end_timeslice();
#endif

#ifdef EMU_A68K
	}
#endif

}

// Run the active CPU
int SekRun(const int nCycles)
{

#ifdef EMU_A68K
	if (nSekCPUType[nSekActive] == 0) {
		nSekCyclesDone = 0;
		nSekCyclesSegment = nCycles;
		do {
			m68k_ICount = nSekCyclesToDo = nSekCyclesSegment - nSekCyclesDone;

			if (M68000_regs.irq == 0x80) {						// Cpu is in stopped state till interrupt
				nSekCyclesDone = nSekCyclesSegment;
				nSekCyclesTotal += nSekCyclesSegment;
			} else {
				M68000_RUN();
				nSekCyclesDone += nSekCyclesToDo - m68k_ICount;
				nSekCyclesTotal += nSekCyclesToDo - m68k_ICount;
			}
		} while (nSekCyclesDone < nSekCyclesSegment);

		nSekCyclesSegment = nSekCyclesDone;
		nSekCyclesToDo = m68k_ICount = -1;
		nSekCyclesDone = 0;

		return nSekCyclesSegment;								// Return the number of cycles actually done
	} else {
#endif

#ifdef EMU_M68K
		nSekCyclesToDo = nCycles;

		nSekCyclesSegment = m68k_execute(nCycles);

		nSekCyclesTotal += nSekCyclesSegment;
		nSekCyclesToDo = m68k_ICount = -1;

		return nSekCyclesSegment;
#else
		return 0;
#endif

#ifdef EMU_A68K
	}
#endif

}

// ----------------------------------------------------------------------------
// Breakpoint support

void SekDbgDisableBreakpoints()
{
#if defined FBA_DEBUG && defined EMU_M68K
		m68k_set_instr_hook_callback(NULL);

		M68KReadByteDebug = M68KReadByte;
		M68KReadWordDebug = M68KReadWord;
		M68KReadLongDebug = M68KReadLong;

		M68KWriteByteDebug = M68KWriteByte;
		M68KWriteWordDebug = M68KWriteWord;
		M68KWriteLongDebug = M68KWriteLong;
#endif

#ifdef EMU_A68K
	a68k_memory_intf = a68k_inter_normal;
#endif

	mame_debug = 0;
}

#if defined (FBA_DEBUG)

void SekDbgEnableBreakpoints()
{
	if (BreakpointDataRead[0].address || BreakpointDataWrite[0].address || BreakpointFetch[0].address) {
#if defined FBA_DEBUG && defined EMU_M68K
		SekDbgDisableBreakpoints();

		if (BreakpointFetch[0].address) {
			m68k_set_instr_hook_callback(M68KCheckBreakpoint);
		}

		if (BreakpointDataRead[0].address) {
			M68KReadByteDebug = M68KReadByteBP;
			M68KReadWordDebug = M68KReadWordBP;
			M68KReadLongDebug = M68KReadLongBP;
		}

		if (BreakpointDataWrite[0].address) {
			M68KWriteByteDebug = M68KWriteByteBP;
			M68KWriteWordDebug = M68KWriteWordBP;
			M68KWriteLongDebug = M68KWriteLongBP;
		}
#endif

#ifdef EMU_A68K
		a68k_memory_intf = a68k_inter_breakpoint;
		if (BreakpointFetch[0].address) {
			a68k_memory_intf.DebugCallback = A68KCheckBreakpoint;
			mame_debug = 255;
		} else {
			a68k_memory_intf.DebugCallback = NULL;
			mame_debug = 0;
		}
#endif
	} else {
		SekDbgDisableBreakpoints();
	}
}

void SekDbgEnableSingleStep()
{
#if defined FBA_DEBUG && defined EMU_M68K
	m68k_set_instr_hook_callback(M68KSingleStep);
#endif

#ifdef EMU_A68K
	a68k_memory_intf.DebugCallback = A68KSingleStep;
	mame_debug = 254;
#endif
}

int SekDbgSetBreakpointDataRead(unsigned int nAddress, int nIdentifier)
{
	for (int i = 0; i < 8; i++) {
		if (BreakpointDataRead[i].id == nIdentifier) {

			if	(nAddress) {							// Change breakpoint
				BreakpointDataRead[i].address = nAddress;
			} else {									// Delete breakpoint
				for ( ; i < 8; i++) {
					BreakpointDataRead[i] = BreakpointDataRead[i + 1];
				}
			}

			SekDbgEnableBreakpoints();
			return 0;
		}
	}

	// No breakpoints present, add it to the 1st slot
	BreakpointDataRead[0].address = nAddress;
	BreakpointDataRead[0].id = nIdentifier;

	SekDbgEnableBreakpoints();
	return 0;
}

int SekDbgSetBreakpointDataWrite(unsigned int nAddress, int nIdentifier)
{
	for (int i = 0; i < 8; i++) {
		if (BreakpointDataWrite[i].id == nIdentifier) {

			if (nAddress) {								// Change breakpoint
				BreakpointDataWrite[i].address = nAddress;
			} else {									// Delete breakpoint
				for ( ; i < 8; i++) {
					BreakpointDataWrite[i] = BreakpointDataWrite[i + 1];
				}
			}

			SekDbgEnableBreakpoints();
			return 0;
		}
	}

	// No breakpoints present, add it to the 1st slot
	BreakpointDataWrite[0].address = nAddress;
	BreakpointDataWrite[0].id = nIdentifier;

	SekDbgEnableBreakpoints();
	return 0;
}

int SekDbgSetBreakpointFetch(unsigned int nAddress, int nIdentifier)
{
	for (int i = 0; i < 8; i++) {
		if (BreakpointFetch[i].id == nIdentifier) {

			if (nAddress) {								// Change breakpoint
				BreakpointFetch[i].address = nAddress;
			} else {									// Delete breakpoint
				for ( ; i < 8; i++) {
					BreakpointFetch[i] = BreakpointFetch[i + 1];
				}
			}

			SekDbgEnableBreakpoints();
			return 0;
		}
	}

	// No breakpoints present, add it to the 1st slot
	BreakpointFetch[0].address = nAddress;
	BreakpointFetch[0].id = nIdentifier;

	SekDbgEnableBreakpoints();
	return 0;
}

#endif

// ----------------------------------------------------------------------------
// Memory map setup

// Note - each page is 1 << SEK_BITS.
int SekMapMemory(unsigned char* pMemory, unsigned int nStart, unsigned int nEnd, int nType)
{
	unsigned char* Ptr = pMemory - nStart;
	unsigned char** pMemMap = pSekExt->MemMap + (nStart >> SEK_SHIFT);

	// Special case for ROM banks
	if (nType == SM_ROM) {
		for (unsigned int i = (nStart & ~SEK_PAGEM); i <= nEnd; i += SEK_PAGE_SIZE, pMemMap++) {
			pMemMap[0]			  = Ptr + i;
			pMemMap[SEK_WADD * 2] = Ptr + i;
		}

		return 0;
	}

	for (unsigned int i = (nStart & ~SEK_PAGEM); i <= nEnd; i += SEK_PAGE_SIZE, pMemMap++) {

		if (nType & SM_READ) {					// Read
			pMemMap[0]			  = Ptr + i;
		}
		if (nType & SM_WRITE) {					// Write
			pMemMap[SEK_WADD]	  = Ptr + i;
		}
		if (nType & SM_FETCH) {					// Fetch
			pMemMap[SEK_WADD * 2] = Ptr + i;
		}
	}

	return 0;
}

int SekMapHandler(unsigned int nHandler, unsigned int nStart, unsigned int nEnd, int nType)
{
	unsigned char** pMemMap = pSekExt->MemMap + (nStart >> SEK_SHIFT);

	// Add to memory map
	for (unsigned int i = (nStart & ~SEK_PAGEM); i <= nEnd; i += SEK_PAGE_SIZE, pMemMap++) {

		if (nType & SM_READ) {					// Read
			pMemMap[0]			  = (unsigned char*)nHandler;
		}
		if (nType & SM_WRITE) {					// Write
			pMemMap[SEK_WADD]	  = (unsigned char*)nHandler;
		}
		if (nType & SM_FETCH) {					// Fetch
			pMemMap[SEK_WADD * 2] = (unsigned char*)nHandler;
		}
	}

	return 0;
}

// Set callbacks
int SekSetResetCallback(pSekResetCallback pCallback)
{
	pSekExt->ResetCallback = pCallback;

	return 0;
}

int SekSetRTECallback(pSekRTECallback pCallback)
{
	pSekExt->RTECallback = pCallback;

	return 0;
}

int SekSetIrqCallback(pSekIrqCallback pCallback)
{
	pSekExt->IrqCallback = pCallback;

	return 0;
}

int SekSetCmpCallback(pSekCmpCallback pCallback)
{
	pSekExt->CmpCallback = pCallback;

	return 0;
}

// Set handlers
int SekSetReadByteHandler(int i, pSekReadByteHandler pHandler)
{
	if (i >= SEK_MAXHANDLER) {
		return 1;
	}

	pSekExt->ReadByte[i] = pHandler;

	return 0;
}

int SekSetWriteByteHandler(int i, pSekWriteByteHandler pHandler)
{
	if (i >= SEK_MAXHANDLER) {
		return 1;
	}

	pSekExt->WriteByte[i] = pHandler;

	return 0;
}

int SekSetReadWordHandler(int i, pSekReadWordHandler pHandler)
{
	if (i >= SEK_MAXHANDLER) {
		return 1;
	}

	pSekExt->ReadWord[i] = pHandler;

	return 0;
}

int SekSetWriteWordHandler(int i, pSekWriteWordHandler pHandler)
{
	if (i >= SEK_MAXHANDLER) {
		return 1;
	}

	pSekExt->WriteWord[i] = pHandler;

	return 0;
}

int SekSetReadLongHandler(int i, pSekReadLongHandler pHandler)
{
	if (i >= SEK_MAXHANDLER) {
		return 1;
	}

	pSekExt->ReadLong[i] = pHandler;

	return 0;
}

int SekSetWriteLongHandler(int i, pSekWriteLongHandler pHandler)
{
	if (i >= SEK_MAXHANDLER) {
		return 1;
	}

	pSekExt->WriteLong[i] = pHandler;

	return 0;
}

// ----------------------------------------------------------------------------
// Query register values

#ifdef EMU_A68K
int SekGetPC(int n)
#else
int SekGetPC(int)
#endif
{

#ifdef EMU_A68K
	if (nSekCPUType[nSekActive] == 0) {
		if (n < 0) {								// Currently active CPU
		  return M68000_regs.pc;
		} else {
			return SekRegs[n]->pc;					// Any CPU
		}
	} else {
#endif

#ifdef EMU_M68K
		return m68k_get_reg(NULL, M68K_REG_PC);
#else
		return 0;
#endif

#ifdef EMU_A68K
	}
#endif

}

int SekDbgGetCPUType()
{
	switch (nSekCPUType[nSekActive]) {
		case 0:
		case 0x68000:
			return M68K_CPU_TYPE_68000;
		case 0x68010:
			return M68K_CPU_TYPE_68010;
		case 0x68EC020:
			return M68K_CPU_TYPE_68EC020;
	}

	return 0;
}

int SekDbgGetPendingIRQ()
{
	return nSekIRQPending[nSekActive] & 7;
}

unsigned int SekDbgGetRegister(SekRegister nRegister)
{
#if defined EMU_A68K
	if (nSekCPUType[nSekActive] == 0) {
		switch (nRegister) {
			case SEK_REG_D0:
				return M68000_regs.d[0];
			case SEK_REG_D1:
				return M68000_regs.d[1];
			case SEK_REG_D2:
				return M68000_regs.d[2];
			case SEK_REG_D3:
				return M68000_regs.d[3];
			case SEK_REG_D4:
				return M68000_regs.d[4];
			case SEK_REG_D5:
				return M68000_regs.d[5];
			case SEK_REG_D6:
				return M68000_regs.d[6];
			case SEK_REG_D7:
				return M68000_regs.d[7];

			case SEK_REG_A0:
				return M68000_regs.a[0];
			case SEK_REG_A1:
				return M68000_regs.a[1];
			case SEK_REG_A2:
				return M68000_regs.a[2];
			case SEK_REG_A3:
				return M68000_regs.a[3];
			case SEK_REG_A4:
				return M68000_regs.a[4];
			case SEK_REG_A5:
				return M68000_regs.a[5];
			case SEK_REG_A6:
				return M68000_regs.a[6];
			case SEK_REG_A7:
				return M68000_regs.a[7];

			case SEK_REG_PC:
				return M68000_regs.pc;

			case SEK_REG_SR:
				return GetA68KSR();

			case SEK_REG_SP:
				return M68000_regs.a[7];
			case SEK_REG_USP:
				return GetA68KUSP();
			case SEK_REG_ISP:
				return GetA68KISP();

			default:
				return 0;
		}
	}
#endif

	switch (nRegister) {
		case SEK_REG_D0:
			return m68k_get_reg(NULL, M68K_REG_D0);
		case SEK_REG_D1:
			return m68k_get_reg(NULL, M68K_REG_D1);
		case SEK_REG_D2:
			return m68k_get_reg(NULL, M68K_REG_D2);
		case SEK_REG_D3:
			return m68k_get_reg(NULL, M68K_REG_D3);
		case SEK_REG_D4:
			return m68k_get_reg(NULL, M68K_REG_D4);
		case SEK_REG_D5:
			return m68k_get_reg(NULL, M68K_REG_D5);
		case SEK_REG_D6:
			return m68k_get_reg(NULL, M68K_REG_D6);
		case SEK_REG_D7:
			return m68k_get_reg(NULL, M68K_REG_D7);

		case SEK_REG_A0:
			return m68k_get_reg(NULL, M68K_REG_A0);
		case SEK_REG_A1:
			return m68k_get_reg(NULL, M68K_REG_A1);
		case SEK_REG_A2:
			return m68k_get_reg(NULL, M68K_REG_A2);
		case SEK_REG_A3:
			return m68k_get_reg(NULL, M68K_REG_A3);
		case SEK_REG_A4:
			return m68k_get_reg(NULL, M68K_REG_A4);
		case SEK_REG_A5:
			return m68k_get_reg(NULL, M68K_REG_A5);
		case SEK_REG_A6:
			return m68k_get_reg(NULL, M68K_REG_A6);
		case SEK_REG_A7:
			return m68k_get_reg(NULL, M68K_REG_A7);

		case SEK_REG_PC:
			return m68k_get_reg(NULL, M68K_REG_PC);

		case SEK_REG_SR:
			return m68k_get_reg(NULL, M68K_REG_SR);

		case SEK_REG_SP:
			return m68k_get_reg(NULL, M68K_REG_SP);
		case SEK_REG_USP:
			return m68k_get_reg(NULL, M68K_REG_USP);
		case SEK_REG_ISP:
			return m68k_get_reg(NULL, M68K_REG_ISP);
		case SEK_REG_MSP:
			return m68k_get_reg(NULL, M68K_REG_MSP);

		case SEK_REG_VBR:
			return m68k_get_reg(NULL, M68K_REG_VBR);

		case SEK_REG_SFC:
			return m68k_get_reg(NULL, M68K_REG_SFC);
		case SEK_REG_DFC:
			return m68k_get_reg(NULL, M68K_REG_DFC);

		case SEK_REG_CACR:
			return m68k_get_reg(NULL, M68K_REG_CACR);
		case SEK_REG_CAAR:
			return m68k_get_reg(NULL, M68K_REG_CAAR);

		default:
			return 0;
	}
}

bool SekDbgSetRegister(SekRegister nRegister, unsigned int nValue)
{
	switch (nRegister) {
		case SEK_REG_D0:
		case SEK_REG_D1:
		case SEK_REG_D2:
		case SEK_REG_D3:
		case SEK_REG_D4:
		case SEK_REG_D5:
		case SEK_REG_D6:
		case SEK_REG_D7:
			break;

		case SEK_REG_A0:
		case SEK_REG_A1:
		case SEK_REG_A2:
		case SEK_REG_A3:
		case SEK_REG_A4:
		case SEK_REG_A5:
		case SEK_REG_A6:
		case SEK_REG_A7:
			break;

		case SEK_REG_PC:
			if (nSekCPUType[nSekActive] == 0) {
#if defined EMU_A68K
				M68000_regs.pc = nValue;
				A68KChangePC(M68000_regs.pc);
#endif
			} else {
				m68k_set_reg(M68K_REG_PC, nValue);
			}
			SekClose();
			return true;

		case SEK_REG_SR:
			break;

		case SEK_REG_SP:
		case SEK_REG_USP:
		case SEK_REG_ISP:
		case SEK_REG_MSP:
			break;

		case SEK_REG_VBR:
			break;

		case SEK_REG_SFC:
		case SEK_REG_DFC:
			break;

		case SEK_REG_CACR:
		case SEK_REG_CAAR:
			break;

		default:
			break;
	}

	return false;
}

// ----------------------------------------------------------------------------
// Savestate support

int SekScan(int nAction)
{
	// Scan the 68000 states
	struct BurnArea ba;

	if ((nAction & ACB_DRIVER_DATA) == 0) {
		return 1;
	}

	memset(&ba, 0, sizeof(ba));

	nSekActive = -1;

	for (int i = 0; i <= nSekCount; i++) {
		char szName[] = "MC68000 #n";
#if defined EMU_A68K && defined EMU_M68K
		int nType = nSekCPUType[i];
#endif

		szName[9] = '0' + i;

		SCAN_VAR(nSekCPUType[i]);

#if defined EMU_A68K && defined EMU_M68K
		// Switch to another core if needed
		if ((nAction & ACB_WRITE) && nType != nSekCPUType[i]) {
			if (nType != 0 && nType != 0x68000 && nSekCPUType[i] != 0 && nSekCPUType[i] != 0x68000) {
				continue;
			}

			if (nSekCPUType[i] == 0x68000) {
				SekCPUExitA68K(i);
				if (SekInitCPUM68K(i, 0x68000)) {
					return 1;
				}
			} else {
				SekCPUExitM68K(i);
				if (SekInitCPUA68K(i, 0x68000)) {
					return 1;
				}
			}
		}
#endif

#ifdef EMU_A68K
		if (nSekCPUType[i] == 0) {
			ba.Data = SekRegs[i];
			ba.nLen = sizeof(A68KContext);
			ba.szName = szName;

			if (nAction & ACB_READ) {
				// Blank pointers
				SekRegs[i]->IrqCallback = NULL;
				SekRegs[i]->ResetCallback = NULL;
			}

			BurnAcb(&ba);

			// Re-setup each cpu on read/write
			if (nAction & ACB_ACCESSMASK) {
				SekSetup(SekRegs[i]);
			}
		} else {
#endif

#ifdef EMU_M68K
			if (nSekCPUType[i] != 0) {
				ba.Data = SekM68KContext[i];
				ba.nLen = nSekM68KContextSize[i];
				ba.szName = szName;
				BurnAcb(&ba);
			}
#endif

#ifdef EMU_A68K
		}
#endif

	}

	return 0;
}
