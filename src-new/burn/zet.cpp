// Z80 (Zed Eight-Ty) Interface
#include "burnint.h"

#define MAX_Z80		8
static struct ZetExt * ZetCPUContext = NULL;
 
typedef unsigned char (__fastcall *pZetInHandler)(unsigned short a);
typedef void (__fastcall *pZetOutHandler)(unsigned short a, unsigned char d);
typedef unsigned char (__fastcall *pZetReadHandler)(unsigned short a);
typedef void (__fastcall *pZetWriteHandler)(unsigned short a, unsigned char d);
 
struct ZetExt {
	Z80_Regs reg;
	
	unsigned char* pZetMemMap[0x100 * 4];

	pZetInHandler ZetIn;
	pZetOutHandler ZetOut;
	pZetReadHandler ZetRead;
	pZetWriteHandler ZetWrite;
};
 
static int nZetCyclesDone[MAX_Z80];
static int nZetCyclesTotal;
static int nZ80ICount[MAX_Z80];
static UINT32 Z80EA[MAX_Z80];

static int nOpenedCPU = -1;
static int nCPUCount = 0;
int nHasZet = -1;

unsigned char __fastcall ZetDummyReadHandler(unsigned short) { return 0; }
void __fastcall ZetDummyWriteHandler(unsigned short, unsigned char) { }
unsigned char __fastcall ZetDummyInHandler(unsigned short) { return 0; }
void __fastcall ZetDummyOutHandler(unsigned short, unsigned char) { }

unsigned char __fastcall ZetReadIO(unsigned int a)
{
	return ZetCPUContext[nOpenedCPU].ZetIn(a);
}

void __fastcall ZetWriteIO(unsigned int a, unsigned char d)
{
	ZetCPUContext[nOpenedCPU].ZetOut(a, d);
}

unsigned char __fastcall ZetReadProg(unsigned int a)
{
	// check mem map
	unsigned char * pr = ZetCPUContext[nOpenedCPU].pZetMemMap[0x000 | (a >> 8)];
	if (pr != NULL) {
		return pr[a & 0xff];
	}
	
	// check handler
	if (ZetCPUContext[nOpenedCPU].ZetRead != NULL) {
		return ZetCPUContext[nOpenedCPU].ZetRead(a);
	}
	
	return 0;
}

void __fastcall ZetWriteProg(unsigned int a, unsigned char d)
{
	// check mem map
	unsigned char * pr = ZetCPUContext[nOpenedCPU].pZetMemMap[0x100 | (a >> 8)];
	if (pr != NULL) {
		pr[a & 0xff] = d;
		return;
	}
	
	// check handler
	if (ZetCPUContext[nOpenedCPU].ZetWrite != NULL) {
		ZetCPUContext[nOpenedCPU].ZetWrite(a, d);
		return;
	}
}

unsigned char __fastcall ZetReadOp(unsigned int a)
{
	// check mem map
	unsigned char * pr = ZetCPUContext[nOpenedCPU].pZetMemMap[0x200 | (a >> 8)];
	if (pr != NULL) {
		return pr[a & 0xff];
	}
	
	// check read handler
	if (ZetCPUContext[nOpenedCPU].ZetRead != NULL) {
		return ZetCPUContext[nOpenedCPU].ZetRead(a);
	}
	
	return 0;
}

unsigned char __fastcall ZetReadOpArg(unsigned int a)
{
	// check mem map
	unsigned char * pr = ZetCPUContext[nOpenedCPU].pZetMemMap[0x300 | (a >> 8)];
	if (pr != NULL) {
		return pr[a & 0xff];
	}
	
	// check read handler
	if (ZetCPUContext[nOpenedCPU].ZetRead != NULL) {
		return ZetCPUContext[nOpenedCPU].ZetRead(a);
	}
	
	return 0;
}

void ZetSetReadHandler(unsigned char (__fastcall *pHandler)(unsigned short))
{
	ZetCPUContext[nOpenedCPU].ZetRead = pHandler;
}

void ZetSetWriteHandler(void (__fastcall *pHandler)(unsigned short, unsigned char))
{
	ZetCPUContext[nOpenedCPU].ZetWrite = pHandler;
}

void ZetSetInHandler(unsigned char (__fastcall *pHandler)(unsigned short))
{
	ZetCPUContext[nOpenedCPU].ZetIn = pHandler;
}

void ZetSetOutHandler(void (__fastcall *pHandler)(unsigned short, unsigned char))
{
	ZetCPUContext[nOpenedCPU].ZetOut = pHandler;
}

void ZetNewFrame()
{
	for (int i = 0; i < nCPUCount; i++) {
		nZetCyclesDone[i] = 0;
	}
	nZetCyclesTotal = 0;
}

int ZetInit(int nCount)
{
	nOpenedCPU = -1;
	
	ZetCPUContext = (struct ZetExt *) malloc(nCount * sizeof(ZetExt));
	if (ZetCPUContext == NULL) return 1;
	memset(ZetCPUContext, 0, nCount * sizeof(ZetExt));
	
	Z80Init();
	
	for (int i = 0; i < nCount; i++) {
		ZetCPUContext[i].ZetIn = ZetDummyInHandler;
		ZetCPUContext[i].ZetOut = ZetDummyOutHandler;
		ZetCPUContext[i].ZetRead = ZetDummyReadHandler;
		ZetCPUContext[i].ZetWrite = ZetDummyWriteHandler;
		// TODO: Z80Init() will set IX IY F regs with default value, so get them ...
		Z80GetContext(&ZetCPUContext[i].reg);
		
		nZetCyclesDone[i] = 0;
		nZ80ICount[i] = 0;
		
		for (int j = 0; j < (0x0100 * 4); j++) {
			ZetCPUContext[i].pZetMemMap[j] = NULL;
		}
	}
	
	nZetCyclesTotal = 0;
	
	Z80SetIOReadHandler(ZetReadIO);
	Z80SetIOWriteHandler(ZetWriteIO);
	Z80SetProgramReadHandler(ZetReadProg);
	Z80SetProgramWriteHandler(ZetWriteProg);
	Z80SetCPUOpReadHandler(ZetReadOp);
	Z80SetCPUOpArgReadHandler(ZetReadOpArg);
	
	ZetOpen(0);
	
	nCPUCount = nCount % MAX_Z80;

	nHasZet = nCount;

	for (int i = 0; i < nCount; i++)
		CpuCheatRegister(0x0004, i);

	return 0;
}

unsigned char ZetReadByte(unsigned short address)
{
	if (nOpenedCPU < 0) return 0;

	return ZetReadProg(address);
}

void ZetWriteByte(unsigned short address, unsigned char data)
{
	if (nOpenedCPU < 0) return;

	ZetWriteProg(address, data);
}

void ZetWriteRom(unsigned short address, unsigned char data)
{
	if (nOpenedCPU < 0) return;

	if (ZetCPUContext[nOpenedCPU].pZetMemMap[0x200 | (address >> 8)] != NULL) {
		ZetCPUContext[nOpenedCPU].pZetMemMap[0x200 | (address >> 8)][address] = data;
	}
	
	if (ZetCPUContext[nOpenedCPU].pZetMemMap[0x300 | (address >> 8)] != NULL) {
		ZetCPUContext[nOpenedCPU].pZetMemMap[0x300 | (address >> 8)][address] = data;
	}
	
	ZetWriteProg(address, data);
}

void ZetClose()
{
	Z80GetContext(&ZetCPUContext[nOpenedCPU].reg);
	nZetCyclesDone[nOpenedCPU] = nZetCyclesTotal;
	nZ80ICount[nOpenedCPU] = z80_ICount;
	Z80EA[nOpenedCPU] = EA;

	nOpenedCPU = -1;
}

void ZetOpen(int nCPU)
{
	Z80SetContext(&ZetCPUContext[nCPU].reg);
	nZetCyclesTotal = nZetCyclesDone[nCPU];
	z80_ICount = nZ80ICount[nCPU];
	EA = Z80EA[nCPU];

	nOpenedCPU = nCPU;
}

int ZetGetActive()
{
	return nOpenedCPU;
}

int ZetRun(int nCycles)
{
	if (nCycles <= 0) return 0;
	
	nCycles = Z80Execute(nCycles);
	
	nZetCyclesTotal += nCycles;
	
	return nCycles;
}

void ZetRunAdjust(int /*nCycles*/)
{
}

void ZetRunEnd()
{
}

// This function will make an area callback ZetRead/ZetWrite
int ZetMemCallback(int nStart, int nEnd, int nMode)
{
	unsigned char cStart = (nStart >> 8);
	unsigned char **pMemMap = ZetCPUContext[nOpenedCPU].pZetMemMap;

	for (unsigned short i = cStart; i <= (nEnd >> 8); i++) {
		switch (nMode) {
			case 0:
				pMemMap[0     + i] = NULL;
				break;
			case 1:
				pMemMap[0x100 + i] = NULL;
				break;
			case 2:
				pMemMap[0x200 + i] = NULL;
				pMemMap[0x300 + i] = NULL;
				break;
		}
	}

	return 0;
}

int ZetMemEnd()
{
	return 0;
}

void ZetExit()
{
	Z80Exit();
	free(ZetCPUContext);
	ZetCPUContext = NULL;

	nCPUCount = 0;
	nHasZet = -1;
}


int ZetMapArea(int nStart, int nEnd, int nMode, unsigned char *Mem)
{
	unsigned char cStart = (nStart >> 8);
	unsigned char **pMemMap = ZetCPUContext[nOpenedCPU].pZetMemMap;

	for (unsigned short i = cStart; i <= (nEnd >> 8); i++) {
		switch (nMode) {
			case 0: {
				pMemMap[0     + i] = Mem + ((i - cStart) << 8);
				break;
			}
		
			case 1: {
				pMemMap[0x100 + i] = Mem + ((i - cStart) << 8);
				break;
			}
			
			case 2: {
				pMemMap[0x200 + i] = Mem + ((i - cStart) << 8);
				pMemMap[0x300 + i] = Mem + ((i - cStart) << 8);
				break;
			}
		}
	}

	return 0;
}

int ZetMapArea(int nStart, int nEnd, int nMode, unsigned char *Mem01, unsigned char *Mem02)
{
	unsigned char cStart = (nStart >> 8);
	unsigned char **pMemMap = ZetCPUContext[nOpenedCPU].pZetMemMap;
	
	if (nMode != 2) {
		return 1;
	}
	
	for (unsigned short i = cStart; i <= (nEnd >> 8); i++) {
		pMemMap[0x200 + i] = Mem01 + ((i - cStart) << 8);
		pMemMap[0x300 + i] = Mem02 + ((i - cStart) << 8);
	}

	return 0;
}

int ZetReset()
{
	Z80Reset();

	return 0;
}

int ZetPc(int n)
{
	if (n < 0) {
		return ActiveZ80GetPC();
	} else {
		return ZetCPUContext[n].reg.pc.w.l;
	}
}

int ZetBc(int n)
{
	if (n < 0) {
		return ActiveZ80GetBC();
	} else {
		return ZetCPUContext[n].reg.bc.w.l;
	}
}

int ZetDe(int n)
{
	if (n < 0) {
		return ActiveZ80GetDE();
	} else {
		return ZetCPUContext[n].reg.de.w.l;
	}
}

int ZetHL(int n)
{
	if (n < 0) {
		return ActiveZ80GetHL();
	} else {
		return ZetCPUContext[n].reg.hl.w.l;
	}
}

int ZetScan(int nAction)
{
	if ((nAction & ACB_DRIVER_DATA) == 0) {
		return 0;
	}

	char szText[] = "Z80 #0";
	
	for (int i = 0; i < nCPUCount; i++) {
		szText[5] = '1' + i;

		ScanVar(&ZetCPUContext[i].reg, sizeof(Z80_Regs), szText);
		SCAN_VAR(Z80EA[i]);
		SCAN_VAR(nZ80ICount[i]);
		SCAN_VAR(nZetCyclesDone[i]);
	}
	
	SCAN_VAR(nZetCyclesTotal);	

	return 0;
}

void ZetSetIRQLine(const int line, const int status)
{
	switch ( status ) {
		case ZET_IRQSTATUS_NONE:
			Z80SetIrqLine(0, 0);
			break;
		case ZET_IRQSTATUS_ACK: 	
			Z80SetIrqLine(line, 1);
			break;
		case ZET_IRQSTATUS_AUTO:
			Z80SetIrqLine(line, 1);
			Z80Execute(0);
			Z80SetIrqLine(0, 0);
			Z80Execute(0);
			break;
	}
}

void ZetSetVector(int vector)
{
	Z80Vector = vector;
}

int ZetNmi()
{
	Z80SetIrqLine(Z80_INPUT_LINE_NMI, 1);
	Z80Execute(0);
	Z80SetIrqLine(Z80_INPUT_LINE_NMI, 0);
	Z80Execute(0);
	int nCycles = 12;
	nZetCyclesTotal += nCycles;

	return nCycles;
}

int ZetIdle(int nCycles)
{
	nZetCyclesTotal += nCycles;

	return nCycles;
}

int ZetSegmentCycles()
{
	return 0;
}

int ZetTotalCycles()
{
	return nZetCyclesTotal;
}

#undef MAX_Z80
