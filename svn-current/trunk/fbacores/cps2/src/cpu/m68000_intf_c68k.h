// 680x0 (Sixty Eight K) Interface
#include "burnint.h"
#include "m68000_intf.h"
#include "m68000_debug.h"

int nSekCpuCore = 0;  // 0 - c68k, 1 - m68k, 2 - a68k
int DebugStep = 0; // 0 - off, 1 - on

c68k_struc * SekC68KCurrentContext = NULL;
c68k_struc * SekC68KContext[SEK_MAX];

int nSekCount = -1;						// Number of allocated 68000s
struct SekExt *SekExt[SEK_MAX] = { NULL, }, *pSekExt = NULL;

int nSekActive = -1;						// The cpu which is currently being emulated
int nSekCyclesTotal, nSekCyclesScanline, nSekCyclesSegment, nSekCyclesDone, nSekCyclesToDo;

int nSekCPUType[SEK_MAX], nSekCycles[SEK_MAX], nSekIRQPending[SEK_MAX];

#if defined (FBA_DEBUG)

void (*SekDbgBreakpointHandlerRead)(unsigned int, int) = NULL;
void (*SekDbgBreakpointHandlerFetch)(unsigned int, int) = NULL;
void (*SekDbgBreakpointHandlerWrite)(unsigned int, int) = NULL;

unsigned int (*SekDbgFetchByteDisassembler)(unsigned int) = NULL;
unsigned int (*SekDbgFetchWordDisassembler)(unsigned int) = NULL;
unsigned int (*SekDbgFetchLongDisassembler)(unsigned int) = NULL;

static struct { unsigned int address; int id; } BreakpointDataRead[9]  = { { 0, 0 }, };
static struct { unsigned int address; int id; } BreakpointDataWrite[9] = { { 0, 0 }, };
static struct { unsigned int address; int id; } BreakpointFetch[9] = { { 0, 0 }, };

#endif

#if defined (FBA_DEBUG)

inline static void CheckBreakpoint_R(unsigned int a, const unsigned int m)
{
	a &= m;

	for (int i = 0; BreakpointDataRead[i].address; i++) {
		if ((BreakpointDataRead[i].address & m) == a) {

			if(SekDbgBreakpointHandlerRead)
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

			if(SekDbgBreakpointHandlerWrite)
				SekDbgBreakpointHandlerWrite(a, BreakpointDataWrite[i].id);
			return;
		}
	}
}

inline static void CheckBreakpoint_PC()
{
	for (int i = 0; BreakpointFetch[i].address; i++) {
		if (BreakpointFetch[i].address == (unsigned int)SekGetPC(-1)) {

			if(SekDbgBreakpointHandlerFetch)
				SekDbgBreakpointHandlerFetch(SekGetPC(-1), BreakpointFetch[i].id);
			return;
		}
	}
}

inline static void SingleStep_PC()
{

	if(SekDbgBreakpointHandlerFetch)
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

DEFWORDHANDLERS(0);
DEFLONGHANDLERS(0);

#if SEK_MAXHANDLER >= 2
 DEFWORDHANDLERS(1);
 DEFLONGHANDLERS(1);
#endif

#if SEK_MAXHANDLER >= 3
 DEFWORDHANDLERS(2);
 DEFLONGHANDLERS(2);
#endif

#if SEK_MAXHANDLER >= 4
 DEFWORDHANDLERS(3);
 DEFLONGHANDLERS(3);
#endif

#if SEK_MAXHANDLER >= 5
 DEFWORDHANDLERS(4);
 DEFLONGHANDLERS(4);
#endif

#if SEK_MAXHANDLER >= 6
 DEFWORDHANDLERS(5);
 DEFLONGHANDLERS(5);
#endif

#if SEK_MAXHANDLER >= 7
 DEFWORDHANDLERS(6);
 DEFLONGHANDLERS(6);
#endif

#if SEK_MAXHANDLER >= 8
 DEFWORDHANDLERS(7);
 DEFLONGHANDLERS(7);
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

	pr = FIND_R(a);
#ifdef FBA_DEBUG
	CheckBreakpoint_R(a, ~0);
#endif
	if ((uintptr_t)pr >= SEK_MAXHANDLER) {
		a ^= 1;
		return pr[a & SEK_PAGEM];
	}
	return pSekExt->ReadByte[(uintptr_t)pr](a);
}

inline static unsigned char FetchByte(unsigned int a)
{
	unsigned char* pr;

	a &= 0xFFFFFF;

//	bprintf(PRINT_NORMAL, _T("fetch8 0x%08X\n"), a);

	pr = FIND_F(a);
	if ((uintptr_t)pr >= SEK_MAXHANDLER) {
		a ^= 1;
		return pr[a & SEK_PAGEM];
	}
	return pSekExt->ReadByte[(uintptr_t)pr](a);
}

inline static void WriteByte(unsigned int a, unsigned char d)
{
	unsigned char* pr;

	a &= 0xFFFFFF;

	pr = FIND_W(a);
#ifdef FBA_DEBUG
	CheckBreakpoint_W(a, ~0);
#endif
	if ((uintptr_t)pr >= SEK_MAXHANDLER) {
		a ^= 1;
		pr[a & SEK_PAGEM] = (unsigned char)d;
		return;
	}
	pSekExt->WriteByte[(uintptr_t)pr](a, d);
}

inline static void WriteByteROM(unsigned int a, unsigned char d)
{
	unsigned char* pr;

	a &= 0xFFFFFF;

	pr = FIND_R(a);
	if ((uintptr_t)pr >= SEK_MAXHANDLER) {
		a ^= 1;
		pr[a & SEK_PAGEM] = (unsigned char)d;
		return;
	}
	pSekExt->WriteByte[(uintptr_t)pr](a, d);
}

inline static unsigned short ReadWord(unsigned int a)
{
	unsigned char* pr;

	a &= 0xFFFFFF;

	pr = FIND_R(a);
#ifdef FBA_DEBUG
	CheckBreakpoint_R(a, ~1);
#endif
	if ((uintptr_t)pr >= SEK_MAXHANDLER) {
		return *((unsigned short*)(pr + (a & SEK_PAGEM)));
	}
	return pSekExt->ReadWord[(uintptr_t)pr](a);
}

inline static unsigned short FetchWord(unsigned int a)
{
	unsigned char* pr;

	a &= 0xFFFFFF;

//	bprintf(PRINT_NORMAL, _T("fetch16 0x%08X\n"), a);

	pr = FIND_F(a);
	if ((uintptr_t)pr >= SEK_MAXHANDLER) {
		pr=(pr + (a & SEK_PAGEM));
		return (*(pr+1))<<8|(*pr);
	}
	return pSekExt->ReadWord[(uintptr_t)pr](a);
}

inline static void WriteWord(unsigned int a, unsigned short d)
{
	unsigned char* pr;

	a &= 0xFFFFFF;

	pr = FIND_W(a);
#ifdef FBA_DEBUG
	CheckBreakpoint_W(a, ~1);
#endif
	if ((uintptr_t)pr >= SEK_MAXHANDLER) {
		*((unsigned short*)(pr + (a & SEK_PAGEM))) = (unsigned short)d;
		return;
	}
	pSekExt->WriteWord[(uintptr_t)pr](a, d);
}

inline static void WriteWordROM(unsigned int a, unsigned short d)
{
	unsigned char* pr;

	a &= 0xFFFFFF;

	pr = FIND_R(a);
	if ((uintptr_t)pr >= SEK_MAXHANDLER) {
		pr=pr + (a & SEK_PAGEM);
		pr[0]=d;
		pr[1]=d>>8;
		return;
	}
	pSekExt->WriteWord[(uintptr_t)pr](a, d);
}

inline static unsigned int ReadLong(unsigned int a)
{
	unsigned char* pr;

	a &= 0xFFFFFF;

	pr = FIND_R(a);
#ifdef FBA_DEBUG
	CheckBreakpoint_R(a, ~1);
#endif
	if ((uintptr_t)pr >= SEK_MAXHANDLER) {
		unsigned int r = *((unsigned int*)(pr + (a & SEK_PAGEM)));
		r = (r >> 16) | (r << 16);
		return r;
	}
	return pSekExt->ReadLong[(uintptr_t)pr](a);
}

inline static unsigned int FetchLong(unsigned int a)
{
	unsigned char* pr;

	a &= 0xFFFFFF;

//	bprintf(PRINT_NORMAL, _T("fetch32 0x%08X\n"), a);

	pr = FIND_F(a);
	if ((uintptr_t)pr >= SEK_MAXHANDLER) {
		pr=pr + (a & SEK_PAGEM);
		return  pr[1]<<24|pr[0]<<16|pr[3]<<8|pr[2];
	}
	return pSekExt->ReadLong[(uintptr_t)pr](a);
}

inline static void WriteLong(unsigned int a, unsigned int d)
{
	unsigned char* pr;

	a &= 0xFFFFFF;

	pr = FIND_W(a);
#ifdef FBA_DEBUG
	CheckBreakpoint_W(a, ~1);
#endif
	if ((uintptr_t)pr >= SEK_MAXHANDLER) {
		d = (d >> 16) | (d << 16);
		*((uintptr_t*)(pr + (a & SEK_PAGEM))) = d;
		return;
	}
	pSekExt->WriteLong[(uintptr_t)pr](a, d);
}

inline static void WriteLongROM(unsigned int a, unsigned int d)
{
	unsigned char* pr;

	a &= 0xFFFFFF;

	pr = FIND_R(a);
	if ((uintptr_t)pr >= SEK_MAXHANDLER) {
		pr=pr + (a & SEK_PAGEM);
		pr[2] = (unsigned char)d;
		pr[3] = (unsigned char)(d>>8);
		pr[0] = (unsigned char)(d>>16);
		pr[1] = (unsigned char)(d>>24);
		return;
	}
	pSekExt->WriteLong[(uintptr_t)pr](a, d);
}

// ----------------------------------------------------------------------------
// A68K variables

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


 unsigned int mame_debug = 0, cur_mrhard = 0, m68k_illegal_opcode = 0, illegal_op = 0, illegal_pc = 0, opcode_entry = 0;

 //struct A68KInter a68k_memory_intf;
}

extern "C" struct A68KInter a68k_memory_intf;

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


extern "C" {
unsigned char C68KReadByte(unsigned int a) { return ReadByte(a); }
unsigned short C68KReadWord(unsigned int a) { return ReadWord(a); }
unsigned char C68KFetchByte(unsigned int a) { return FetchByte(a); }
unsigned short C68KFetchWord(unsigned int a) { return FetchWord(a); }
void C68KWriteByte(unsigned int a, unsigned char d) { WriteByte(a, d); }
void C68KWriteWord(unsigned int a, unsigned short d) { WriteWord(a, d); }

unsigned int C68KRebasePC(unsigned int pc) {
//	bprintf(PRINT_NORMAL, _T("C68KRebasePC 0x%08x\n"), pc);
	pc &= 0xFFFFFF;
	SekC68KCurrentContext->BasePC = (uintptr_t)FIND_F(pc) - (pc & ~SEK_PAGEM);
	return SekC68KCurrentContext->BasePC + pc;
}

int C68KInterruptCallBack(int irqline)
{
	if (nSekIRQPending[nSekActive] & SEK_IRQSTATUS_AUTO) {
		SekC68KContext[nSekActive]->IRQState = 0;	//CLEAR_LINE
		SekC68KContext[nSekActive]->IRQLine = 0;
	}
	
	nSekIRQPending[nSekActive] = 0;
	
	if (pSekExt->IrqCallback) {
		return pSekExt->IrqCallback(irqline);
	}

	return C68K_INTERRUPT_AUTOVECTOR_EX + irqline;
}

void C68KResetCallBack()
{
	if ( pSekExt->ResetCallback )
		pSekExt->ResetCallback();
}

}

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
// Initialisation/exit/reset

static int SekInitCPUC68K(int nCount, int nCPUType)
{
	if (nCPUType != 0x68000) return 1;
	nSekCPUType[nCount] = 0;

	SekC68KContext[nCount] = (c68k_struc *)malloc( sizeof( c68k_struc ) );
	if (SekC68KContext[nCount] == NULL)	return 1;

	memset(SekC68KContext[nCount], 0, sizeof( c68k_struc ));
	SekC68KCurrentContext = SekC68KContext[nCount];

	SekC68KCurrentContext->Rebase_PC = C68KRebasePC;
	
	SekC68KCurrentContext->Read_Byte = C68KReadByte;
	SekC68KCurrentContext->Read_Word = C68KReadWord;
	SekC68KCurrentContext->Read_Byte_PC_Relative = C68KFetchByte;
	SekC68KCurrentContext->Read_Word_PC_Relative = C68KFetchWord;
	SekC68KCurrentContext->Write_Byte = C68KWriteByte;
	SekC68KCurrentContext->Write_Word = C68KWriteWord;
	
	SekC68KCurrentContext->Interrupt_CallBack = C68KInterruptCallBack;
	SekC68KCurrentContext->Reset_CallBack = C68KResetCallBack;
	return 0;
}


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
	for (int j = 8; j < SEK_MAXHANDLER; j++) {
		ps->ReadWord[j]  = DefReadWord0;
		ps->WriteWord[j] = DefWriteWord0;
		ps->ReadLong[j]  = DefReadLong0;
		ps->WriteLong[j] = DefWriteLong0;
	}
#endif

	// Map the normal memory handlers
	SekDbgDisableBreakpoints();

#ifdef FBA_DEBUG
	SekDbgFetchByteDisassembler = &SekFetchByte;
	SekDbgFetchWordDisassembler = &SekFetchWord;
	SekDbgFetchLongDisassembler = &SekFetchLong;
#endif

   if(SekInitCPUC68K(nCount, nCPUType)) {
      SekExit();
      return 1;
   }
   C68k_Init( SekC68KCurrentContext );

	nSekCycles[nCount] = 0;
	nSekIRQPending[nCount] = 0;

	nSekCyclesTotal = 0;
	nSekCyclesScanline = 0;
	return 0;
}


static void SekCPUExitC68K(int i)
{
		free(SekC68KContext[i]);
		SekC68KContext[i] = NULL;
}

int SekExit()
{
	// Deallocate cpu extenal data (memory map etc)
	for (int i = 0; i <= nSekCount; i++) {

		SekCPUExitC68K(i);

		// Deallocate other context data
		free(SekExt[i]);
		SekExt[i] = NULL;
	}
	C68k_Exit();
	pSekExt = NULL;

	nSekActive = -1;
	nSekCount = -1;
	return 0;
}

void SekReset()
{

	C68k_Reset( SekC68KCurrentContext );

#ifdef FBA_DEBUG
	if(DebugStep)
		SekDbgEnableSingleStep();
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

		SekC68KCurrentContext = SekC68KContext[nSekActive];

		nSekCyclesTotal = nSekCycles[nSekActive];
	}
}

// Close the active cpu
void SekClose()
{
	// ....

	nSekCycles[nSekActive] = nSekCyclesTotal;
}

// Get the current CPU
INT32 SekGetActive()
{
#if defined FBA_DEBUG
	if (!DebugCPU_SekInitted) bprintf(PRINT_ERROR, _T("SekGetActive called without init\n"));
#endif

	return nSekActive;
}

// Set the status of an IRQ line on the active CPU
void SekSetIRQLine(const int line, const int status)
{
//	bprintf(PRINT_NORMAL, _T("  - irq line %i -> %i\n"), line, status);

	if (status) {
		nSekIRQPending[nSekActive] = line | status;

			//m68k_set_irq(line);
			SekC68KCurrentContext->IRQState = 1;	//ASSERT_LINE
			SekC68KCurrentContext->IRQLine = line;
			SekC68KCurrentContext->HaltState = 0;

		return;
	}

	nSekIRQPending[nSekActive] = 0;

		SekC68KCurrentContext->IRQState = 0;	//CLEAR_LINE
		SekC68KCurrentContext->IRQLine = 0;

}

// Adjust the active CPU's timeslice
void SekRunAdjust(const int nCycles)
{

		if (nCycles < 0 && c68k_ICount < -nCycles) {
			SekRunEnd();
			return;
		}
		nSekCyclesToDo += nCycles;
		c68k_ICount += nCycles;
		nSekCyclesSegment += nCycles;

}

// End the active CPU's timeslice
void SekRunEnd()
{

		nSekCyclesTotal += (nSekCyclesToDo - nSekCyclesDone) - c68k_ICount;
		nSekCyclesDone += (nSekCyclesToDo - nSekCyclesDone) - c68k_ICount;
		nSekCyclesSegment = nSekCyclesDone;
		nSekCyclesToDo = c68k_ICount = -1;
}

// Run the active CPU
int SekRun(const int nCycles)
{

   nSekCyclesToDo = nCycles;
   nSekCyclesSegment = C68k_Exec(SekC68KCurrentContext, nCycles);
   nSekCyclesTotal += nSekCyclesSegment;
   nSekCyclesToDo = c68k_ICount = -1;

   return nSekCyclesSegment;
}
// ----------------------------------------------------------------------------
// Breakpoint support

void SekDbgDisableBreakpoints()
{

	mame_debug = 0;
}

#if defined (FBA_DEBUG)

void SekDbgEnableBreakpoints()
{
	if (BreakpointDataRead[0].address || BreakpointDataWrite[0].address || BreakpointFetch[0].address) {

	} else {
		SekDbgDisableBreakpoints();
	}
}

void SekDbgEnableSingleStep()
{
   SekC68KCurrentContext->Dbg_CallBack = SingleStep_PC;
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

int SekMapHandler(unsigned long nHandler, unsigned int nStart, unsigned int nEnd, int nType)
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

int SekGetPC(int n)
{
	return SekC68KCurrentContext->PC - SekC68KCurrentContext->BasePC;
}

int SekDbgGetCPUType()
{
	return 0;
}

int SekDbgGetPendingIRQ()
{
	return nSekIRQPending[nSekActive] & 7;
}

unsigned int SekDbgGetRegister(SekRegister nRegister)
{
#ifdef FBA_DEBUG
	if(nSekCpuCore == SEK_CORE_A68K) {
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

			case SEK_REG_CCR:
				return M68000_regs.ccr;
			default:
				return 0;
		}
	}

		switch (nRegister) {
			case SEK_REG_D0:
				return SekC68KCurrentContext->D[0];
			case SEK_REG_D1:
				return SekC68KCurrentContext->D[1];
			case SEK_REG_D2:
				return SekC68KCurrentContext->D[2];
			case SEK_REG_D3:
				return SekC68KCurrentContext->D[3];
			case SEK_REG_D4:
				return SekC68KCurrentContext->D[4];
			case SEK_REG_D5:
				return SekC68KCurrentContext->D[5];
			case SEK_REG_D6:
				return SekC68KCurrentContext->D[6];
			case SEK_REG_D7:
				return SekC68KCurrentContext->D[7];

			case SEK_REG_A0:
				return SekC68KCurrentContext->A[0];
			case SEK_REG_A1:
				return SekC68KCurrentContext->A[1];
			case SEK_REG_A2:
				return SekC68KCurrentContext->A[2];
			case SEK_REG_A3:
				return SekC68KCurrentContext->A[3];
			case SEK_REG_A4:
				return SekC68KCurrentContext->A[4];
			case SEK_REG_A5:
				return SekC68KCurrentContext->A[5];
			case SEK_REG_A6:
				return SekC68KCurrentContext->A[6];
			case SEK_REG_A7:
				return SekC68KCurrentContext->A[7];

			case SEK_REG_PC:
				return SekC68KCurrentContext->PC - SekC68KCurrentContext->BasePC;

			case SEK_REG_SR:
				return 0;

			case SEK_REG_SP:
				return SekC68KCurrentContext->A[7];
			case SEK_REG_USP:
				return SekC68KCurrentContext->USP;
			case SEK_REG_ISP:
				return 0;

			case SEK_REG_CCR:
				return (SekC68KCurrentContext->flag_X << 4) |
					(SekC68KCurrentContext->flag_N << 3) |
					(SekC68KCurrentContext->flag_Z << 2) |
					(SekC68KCurrentContext->flag_V << 1) |
					(SekC68KCurrentContext->flag_C << 0);

			default:
				return 0;
		}
#else
	return 0;
#endif
}

bool SekDbgSetRegister(SekRegister nRegister, unsigned int nValue)
{
#if 0
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
				M68000_regs.pc = nValue;
				A68KChangePC(M68000_regs.pc);
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
#else
	return false;
#endif
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

	for (int i = 0; i <= nSekCount; i++) {
		char szName[] = "MC68000 #n";
		int nType = nSekCPUType[i];

		szName[9] = '0' + i;

		SCAN_VAR(nSekCPUType[i]);


				// PC must contain regular m68000 value
				SekC68KContext[i]->PC -= SekC68KContext[i]->BasePC;

				ba.Data = SekC68KContext[i];
				ba.nLen = (uintptr_t)&(SekC68KContext[i]->BasePC) - (uintptr_t)SekC68KContext[i];
				ba.szName = szName;
				BurnAcb(&ba);

				// restore pointer in PC
				SekC68KContext[i]->BasePC = (uintptr_t)FIND_F(SekC68KContext[i]->PC) - (SekC68KContext[i]->PC & ~SEK_PAGEM);
				SekC68KContext[i]->PC += SekC68KContext[i]->BasePC;
	}
	return 0;
}
