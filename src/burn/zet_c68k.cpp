// Z80 (Zed Eight-Ty) Interface
#include "burnint.h"

#ifdef EMU_CZ80
 #include "cz80.c"
#endif

#ifdef EMU_DOZE
 static DozeContext * ZetCPUContext = NULL;
#endif // EMU_DOZE

#ifdef EMU_MAME_Z80
 static struct ZetExt * ZetCPUContext = NULL;
 static struct ZetExt * lastZetCPUContext = NULL;
#endif

#ifdef EMU_CZ80
 static cz80_struc * ZetCPUContext = NULL;
 static cz80_struc * lastZetCPUContext = NULL;
#endif


static int nOpenedCPU = -1;
static int nCPUCount = 0;
int nHasZet = -1;

unsigned char __fastcall ZetDummyReadHandler(unsigned short) { return 0; }
void __fastcall ZetDummyWriteHandler(unsigned short, unsigned char) { }
unsigned char __fastcall ZetDummyInHandler(unsigned short) { return 0; }
void __fastcall ZetDummyOutHandler(unsigned short, unsigned char) { }

#ifdef EMU_MAME_Z80

unsigned char __fastcall ZetReadIO(unsigned int a)
{
	return lastZetCPUContext->ZetIn(a);
}

void __fastcall ZetWriteIO(unsigned int a, unsigned char d)
{
	lastZetCPUContext->ZetOut(a, d);
}

unsigned char __fastcall ZetReadProg(unsigned int a)
{
	unsigned char *pr = lastZetCPUContext->pZetMemMap[0x000 + (a >> 8)];
	if (pr != NULL) return pr[a & 0xff];
	
	return lastZetCPUContext->ZetRead(a);
}

void __fastcall ZetWriteProg(unsigned int a, unsigned char d)
{
	unsigned char *pr = lastZetCPUContext->pZetMemMap[0x100 + (a >> 8)];
	if (pr != NULL) {
		pr[a & 0xff] = d;
		return;
	}
	lastZetCPUContext->ZetWrite(a, d);	
//	bprintf(PRINT_NORMAL, _T("Write %x, %x\n"), a, d);
}

unsigned char __fastcall ZetReadOp(unsigned int a)
{
	unsigned char *pr = lastZetCPUContext->pZetMemMap[0x200 + (a >> 8)];
	if (pr != NULL) return pr[a & 0xff];

	bprintf(PRINT_NORMAL, _T("Op Read %x\n"), a);
	return 0;
}

unsigned char __fastcall ZetReadOpArg(unsigned int a)
{
	unsigned char *pr = lastZetCPUContext->pZetMemMap[0x300 + (a >> 8)];
	if (pr != NULL) return pr[a & 0xff];
	
	bprintf(PRINT_NORMAL, _T("Op Arg Read %x\n"), a);	
	return 0;
}
#endif

#ifdef EMU_DOZE
void ZetSetReadHandler(unsigned char (__fastcall *pHandler)(unsigned short))
{
	Doze.ReadHandler = pHandler;
}

void ZetSetWriteHandler(void (__fastcall *pHandler)(unsigned short, unsigned char))
{
	Doze.WriteHandler = pHandler;
}

void ZetSetInHandler(unsigned char (__fastcall *pHandler)(unsigned short))
{
	Doze.InHandler = pHandler;
}

void ZetSetOutHandler(void (__fastcall *pHandler)(unsigned short, unsigned char))
{
	Doze.OutHandler = pHandler;
}
#endif

#ifdef EMU_MAME_Z80
void ZetSetReadHandler(unsigned char (__fastcall *pHandler)(unsigned short))
{
	lastZetCPUContext->ZetRead = pHandler;
}

void ZetSetWriteHandler(void (__fastcall *pHandler)(unsigned short, unsigned char))
{
	lastZetCPUContext->ZetWrite = pHandler;
}

void ZetSetInHandler(unsigned char (__fastcall *pHandler)(unsigned short))
{
	lastZetCPUContext->ZetIn = pHandler;
}

void ZetSetOutHandler(void (__fastcall *pHandler)(unsigned short, unsigned char))
{
	lastZetCPUContext->ZetOut = pHandler;
}
#endif

#ifdef EMU_CZ80

void ZetSetReadHandler(unsigned char (__fastcall *pHandler)(unsigned short))
{
	lastZetCPUContext->Read_Byte = pHandler;
}

void ZetSetWriteHandler(void (__fastcall *pHandler)(unsigned short, unsigned char))
{
	lastZetCPUContext->Write_Byte = pHandler;
}

void ZetSetInHandler(unsigned char (__fastcall *pHandler)(unsigned short))
{
	lastZetCPUContext->IN_Port = pHandler;
}

void ZetSetOutHandler(void (__fastcall *pHandler)(unsigned short, unsigned char))
{
	lastZetCPUContext->OUT_Port = pHandler;
}

#endif

void ZetNewFrame()
{
#ifdef EMU_DOZE	
	for (int i = 0; i < nCPUCount; i++) {
		ZetCPUContext[i].nCyclesTotal = 0;
	}

	Doze.nCyclesTotal = 0;
#endif

#ifdef EMU_MAME_Z80
	for (int i = 0; i < nCPUCount; i++) {
		ZetCPUContext[i].nCyclesTotal = 0;
	}
#endif

#ifdef EMU_CZ80
	for (int i = 0; i < nCPUCount; i++) {
		ZetCPUContext[i].nCyclesTotal = 0;
	}
#endif

}

void ZetSetVector(int vector)
{
	int Z80Vector = 0;
	Z80Vector = vector;
}

int ZetInit(int nCount)
{
#ifdef EMU_DOZE
	ZetCPUContext = (DozeContext*)malloc(nCount * sizeof(DozeContext));
	if (ZetCPUContext == NULL) {
		return 1;
	}

	memset(ZetCPUContext, 0, nCount * sizeof(DozeContext));

	for (int i = 0; i < nCount; i++) {
		ZetCPUContext[i].nInterruptLatch = -1;

		ZetCPUContext[i].ReadHandler = ZetDummyReadHandler;
		ZetCPUContext[i].WriteHandler = ZetDummyWriteHandler;
		ZetCPUContext[i].InHandler = ZetDummyInHandler;
		ZetCPUContext[i].OutHandler = ZetDummyOutHandler;

		ZetCPUContext[i].ppMemFetch = (unsigned char**)malloc(0x0100 * sizeof(char*));
		ZetCPUContext[i].ppMemFetchData = (unsigned char**)malloc(0x0100 * sizeof(char*));
		ZetCPUContext[i].ppMemRead = (unsigned char**)malloc(0x0100 * sizeof(char*));
		ZetCPUContext[i].ppMemWrite = (unsigned char**)malloc(0x0100 * sizeof(char*));

		if (ZetCPUContext[i].ppMemFetch == NULL || ZetCPUContext[i].ppMemFetchData == NULL || ZetCPUContext[i].ppMemRead == NULL || ZetCPUContext[i].ppMemWrite == NULL) {
			ZetExit();
			return 1;
		}

		for (int j = 0; j < 0x0100; j++) {
			ZetCPUContext[i].ppMemFetch[j] = NULL;
			ZetCPUContext[i].ppMemFetchData[j] = NULL;
			ZetCPUContext[i].ppMemRead[j] = NULL;
			ZetCPUContext[i].ppMemWrite[j] = NULL;
		}
	}

	ZetOpen(0);

	nCPUCount = nCount;
#endif

#ifdef EMU_MAME_Z80
	ZetCPUContext = (struct ZetExt *) malloc(nCount * sizeof(struct ZetExt));
	if (ZetCPUContext == NULL) return 1;
	memset(ZetCPUContext, 0, nCount * sizeof(struct ZetExt));
	
	Z80Init();
	
	for (int i = 0; i < nCount; i++) {
		ZetCPUContext[i].ZetIn = ZetDummyInHandler;
		ZetCPUContext[i].ZetOut = ZetDummyOutHandler;
		ZetCPUContext[i].ZetRead = ZetDummyReadHandler;
		ZetCPUContext[i].ZetWrite = ZetDummyWriteHandler;
		// TODO: Z80Init() will set IX IY F regs with default value, so get them ...
		Z80GetContext(& (ZetCPUContext[i].reg) );
	}
	
	Z80SetIOReadHandler(ZetReadIO);
	Z80SetIOWriteHandler(ZetWriteIO);
	Z80SetProgramReadHandler(ZetReadProg);
	Z80SetProgramWriteHandler(ZetWriteProg);
	Z80SetCPUOpReadHandler(ZetReadOp);
	Z80SetCPUOpArgReadHandler(ZetReadOpArg);
	
	ZetOpen(0);
	
	nCPUCount = nCount;
#endif

#ifdef EMU_CZ80
	ZetCPUContext = (cz80_struc *) malloc(nCount * sizeof(cz80_struc));
	if (ZetCPUContext == NULL) return 1;
	Cz80_InitFlags();
	//memset(ZetCPUContext, 0, nCount * sizeof(cz80_struc));
	
	for (int i = 0; i < nCount; i++) {
		Cz80_Init( &ZetCPUContext[i] );
		ZetCPUContext[i].nInterruptLatch = -1;
		ZetCPUContext[i].IN_Port = ZetDummyInHandler;
		ZetCPUContext[i].OUT_Port = ZetDummyOutHandler;
		ZetCPUContext[i].Read_Byte = ZetDummyReadHandler;
		ZetCPUContext[i].Write_Byte = ZetDummyWriteHandler;
	}
	ZetOpen(0);
	nCPUCount = nCount;
#endif

	nHasZet = nCount;
	return 0;
}

unsigned char ZetReadByte(unsigned short address)
{
	if (nOpenedCPU < 0) return 0;

#ifdef EMU_DOZE
	if (ZetCPUContext[nOpenedCPU].ppMemRead[address>>8] != NULL) {
		return ZetCPUContext[nOpenedCPU].ppMemRead[address>>8][address];
	}

	if (ZetCPUContext[nOpenedCPU].ReadHandler != NULL) {
		return ZetCPUContext[nOpenedCPU].ReadHandler(address);
	}
#endif

	return 0;
}

void ZetWriteRom(unsigned short address, unsigned char data)
{
	if (nOpenedCPU < 0) return;

}

void ZetWriteByte(unsigned short address, unsigned char data)
{
	if (nOpenedCPU < 0) return;

#ifdef EMU_DOZE
	if (ZetCPUContext[nOpenedCPU].ppMemWrite[address>>8] != NULL) {
		ZetCPUContext[nOpenedCPU].ppMemWrite[address>>8][address] = data;
	}

	if (ZetCPUContext[nOpenedCPU].WriteHandler != NULL) {
		ZetCPUContext[nOpenedCPU].WriteHandler(address, data);
	}
#endif
}
void ZetClose()
{
#ifdef EMU_DOZE
	ZetCPUContext[nOpenedCPU] = Doze;
#endif

#ifdef EMU_MAME_Z80
	// Set handlers here too
	if (nOpenedCPU >= 0)
		Z80GetContext(&(ZetCPUContext[nOpenedCPU].reg));
#endif

#ifdef EMU_CZ80
	//
#endif

	nOpenedCPU = -1;
}

int ZetOpen(int nCPU)
{
#ifdef EMU_DOZE
	Doze = ZetCPUContext[nCPU];
#endif

#ifdef EMU_MAME_Z80
	// Set handlers here too
	Z80SetContext(&ZetCPUContext[nCPU].reg);
	lastZetCPUContext = &ZetCPUContext[nCPU];
#endif

#ifdef EMU_CZ80
	lastZetCPUContext = &ZetCPUContext[nCPU];
#endif

	nOpenedCPU = nCPU;

	return 0;
}

int ZetGetActive()
{
	return nOpenedCPU;
}
int ZetRun(int nCycles)
{
	if (nCycles <= 0||ZetCPUContext==0) return 0;
	
#ifdef EMU_DOZE
	Doze.nCyclesTotal += nCycles;
	Doze.nCyclesSegment = nCycles;
	Doze.nCyclesLeft = nCycles;

	DozeRun();
	nCycles = Doze.nCyclesSegment - Doze.nCyclesLeft;

	Doze.nCyclesTotal -= Doze.nCyclesLeft;
	Doze.nCyclesLeft = 0;
	Doze.nCyclesSegment = 0;

	return nCycles;
#endif

#ifdef EMU_MAME_Z80

	lastZetCPUContext->nCyclesTotal += nCycles;
	lastZetCPUContext->nCyclesSegment = nCycles;
	lastZetCPUContext->nCyclesLeft = nCycles;
	
	nCycles = Z80Execute(nCycles);
	
	lastZetCPUContext->nCyclesLeft = lastZetCPUContext->nCyclesLeft - nCycles;
	lastZetCPUContext->nCyclesTotal -= lastZetCPUContext->nCyclesLeft;
	lastZetCPUContext->nCyclesLeft = 0;
	lastZetCPUContext->nCyclesSegment = 0;
	
	return nCycles;
#endif


#ifdef EMU_CZ80
	lastZetCPUContext->nCyclesTotal += nCycles;
	lastZetCPUContext->nCyclesSegment = nCycles;
	lastZetCPUContext->nCyclesLeft = nCycles;

	nCycles = Cz80_Exec(lastZetCPUContext);
	//nCycles = Doze.nCyclesSegment - Doze.nCyclesLeft;

	lastZetCPUContext->nCyclesTotal -= lastZetCPUContext->nCyclesLeft;
	lastZetCPUContext->nCyclesLeft = 0;
	lastZetCPUContext->nCyclesSegment = 0;

	return nCycles;
#endif

}

void ZetRunAdjust(int nCycles)
{
#ifdef EMU_DOZE
	if (nCycles < 0 && Doze.nCyclesLeft < -nCycles) {
		nCycles = 0;
	}

	Doze.nCyclesTotal += nCycles;
	Doze.nCyclesSegment += nCycles;
	Doze.nCyclesLeft += nCycles;
#endif

#ifdef EMU_MAME_Z80
	if (nCycles < 0 && lastZetCPUContext->nCyclesLeft < -nCycles) {
		nCycles = 0;
	}

	lastZetCPUContext->nCyclesTotal += nCycles;
	lastZetCPUContext->nCyclesSegment += nCycles;
	lastZetCPUContext->nCyclesLeft += nCycles;
#endif

#ifdef EMU_CZ80
	if (nCycles < 0 && lastZetCPUContext->nCyclesLeft < -nCycles) {
		nCycles = 0;
	}

	lastZetCPUContext->nCyclesTotal += nCycles;
	lastZetCPUContext->nCyclesSegment += nCycles;
	lastZetCPUContext->nCyclesLeft += nCycles;
#endif

}

void ZetRunEnd()
{
#ifdef EMU_DOZE
	Doze.nCyclesTotal -= Doze.nCyclesLeft;
	Doze.nCyclesSegment -= Doze.nCyclesLeft;
	Doze.nCyclesLeft = 0;
#endif

#ifdef EMU_MAME_Z80
	lastZetCPUContext->nCyclesTotal -= lastZetCPUContext->nCyclesLeft;
	lastZetCPUContext->nCyclesSegment -= lastZetCPUContext->nCyclesLeft;
	lastZetCPUContext->nCyclesLeft = 0;
#endif

#ifdef EMU_CZ80
	lastZetCPUContext->nCyclesTotal -= lastZetCPUContext->nCyclesLeft;
	lastZetCPUContext->nCyclesSegment -= lastZetCPUContext->nCyclesLeft;
	lastZetCPUContext->nCyclesLeft = 0;
#endif

}

// This function will make an area callback ZetRead/ZetWrite
int ZetMemCallback(int nStart, int nEnd, int nMode)
{
#ifdef EMU_DOZE
	nStart >>= 8;
	nEnd += 0xff;
	nEnd >>= 8;

	// Leave the section out of the memory map, so the Doze* callback with be used
	for (int i = nStart; i < nEnd; i++) {
		switch (nMode) {
			case 0:
				Doze.ppMemRead[i] = NULL;
				break;
			case 1:
				Doze.ppMemWrite[i] = NULL;
				break;
			case 2:
				Doze.ppMemFetch[i] = NULL;
				break;
		}
	}
#endif

#ifdef EMU_MAME_Z80
	unsigned char cStart = (nStart >> 8);
	unsigned char **pMemMap = lastZetCPUContext->pZetMemMap;

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
				//pMemMap[0x300 + i] = NULL;
				break;
		}
	}
#endif

#ifdef EMU_CZ80
	nStart >>= CZ80_FETCH_SFT;
	nEnd += CZ80_FETCH_BANK - 1;
	nEnd >>= CZ80_FETCH_SFT;

	// Leave the section out of the memory map, so the callback with be used
	for (int i = nStart; i < nEnd; i++) {
		switch (nMode) {
			case 0:
				lastZetCPUContext->Read[i] = NULL;
				break;
			case 1:
				lastZetCPUContext->Write[i] = NULL;
				break;
			case 2:
				lastZetCPUContext->Fetch[i] = NULL;
				break;
		}
	}
#endif

	return 0;
}

int ZetMemEnd()
{
	return 0;
}

void ZetExit()
{
#ifdef EMU_DOZE
	for (int i = 0; i < nCPUCount; i++) {
		free(ZetCPUContext[i].ppMemFetch);
		ZetCPUContext[i].ppMemFetch = NULL;
		free(ZetCPUContext[i].ppMemFetchData);
		ZetCPUContext[i].ppMemFetchData = NULL;
		free(ZetCPUContext[i].ppMemRead);
		ZetCPUContext[i].ppMemRead = NULL;
		free(ZetCPUContext[i].ppMemWrite);
		ZetCPUContext[i].ppMemWrite = NULL;
	}

	free(ZetCPUContext);
	ZetCPUContext = NULL;
#endif

#ifdef EMU_MAME_Z80
	Z80Exit();
	free( ZetCPUContext );
	ZetCPUContext = NULL;
	lastZetCPUContext = NULL;
#endif

#ifdef EMU_CZ80
	free( ZetCPUContext );
	ZetCPUContext = NULL;
	lastZetCPUContext = NULL;
#endif

	nCPUCount = 0;
	nHasZet = -1;
}


int ZetMapArea(int nStart, int nEnd, int nMode, unsigned char *Mem)
{
#ifdef EMU_DOZE
	int s = nStart >> 8;
	int e = (nEnd + 0xFF) >> 8;

	// Put this section in the memory map, giving the offset from Z80 memory to PC memory
	for (int i = s; i < e; i++) {
		switch (nMode) {
			case 0:
				Doze.ppMemRead[i] = Mem - nStart;
				break;
			case 1:
				Doze.ppMemWrite[i] = Mem - nStart;
				break;
			case 2:
				Doze.ppMemFetch[i] = Mem - nStart;
				Doze.ppMemFetchData[i] = Mem - nStart;
				break;
		}
	}
#endif

#ifdef EMU_MAME_Z80
	unsigned char cStart = (nStart >> 8);
	unsigned char **pMemMap = lastZetCPUContext->pZetMemMap;

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
#endif

#ifdef EMU_CZ80
	int s = nStart >> CZ80_FETCH_SFT;
	int e = (nEnd + CZ80_FETCH_BANK - 1) >> CZ80_FETCH_SFT;

	// Put this section in the memory map, giving the offset from Z80 memory to PC memory
	for (int i = s; i < e; i++) {
		switch (nMode) {
			case 0:
				lastZetCPUContext->Read[i] = Mem - nStart;
				break;
			case 1:
				lastZetCPUContext->Write[i] = Mem - nStart;
				break;
			case 2:
				lastZetCPUContext->Fetch[i] = Mem - nStart;
				lastZetCPUContext->FetchData[i] = Mem - nStart;
				break;
		}
	}
/*
	if (nMode == 2) {
		s = lastZetCPUContext->PC - lastZetCPUContext->BasePC;
		e = s >> CZ80_FETCH_SFT;
		lastZetCPUContext->BasePC = (u32) lastZetCPUContext->Fetch[e];
		lastZetCPUContext->BasePCData = (u32) lastZetCPUContext->FetchData[e];
		lastZetCPUContext->PC = s + lastZetCPUContext->BasePC;
		lastZetCPUContext->PCData = s + lastZetCPUContext->BasePCData;
	}
*/
#endif

	return 0;
}

int ZetMapArea(int nStart, int nEnd, int nMode, unsigned char *Mem01, unsigned char *Mem02)
{
#ifdef EMU_DOZE
	int s = nStart >> 8;
	int e = (nEnd + 0xFF) >> 8;

	if (nMode != 2) {
		return 1;
	}

	// Put this section in the memory map, giving the offset from Z80 memory to PC memory
	for (int i = s; i < e; i++) {
		Doze.ppMemFetch[i] = Mem01 - nStart;
		Doze.ppMemFetchData[i] = Mem02 - nStart;
	}
#endif

#ifdef EMU_MAME_Z80
	unsigned char cStart = (nStart >> 8);
	unsigned char **pMemMap = lastZetCPUContext->pZetMemMap;
	
	if (nMode != 2) {
		return 1;
	}
	
	for (unsigned short i = cStart; i <= (nEnd >> 8); i++) {
		pMemMap[0x200 + i] = Mem01 + ((i - cStart) << 8);
		pMemMap[0x300 + i] = Mem02 + ((i - cStart) << 8);
	}
#endif

#ifdef EMU_CZ80
	int s = nStart >> CZ80_FETCH_SFT;
	int e = (nEnd + CZ80_FETCH_BANK - 1) >> CZ80_FETCH_SFT;

	if (nMode != 2) {
		return 1;
	}

	// Put this section in the memory map, giving the offset from Z80 memory to PC memory
	for (int i = s; i < e; i++) {
		lastZetCPUContext->Fetch[i] = Mem01 - nStart;
		lastZetCPUContext->FetchData[i] = Mem02 - nStart;
	}
/*	
	s = lastZetCPUContext->PC - lastZetCPUContext->BasePC;
	e = s >> CZ80_FETCH_SFT;
	lastZetCPUContext->BasePC = (u32) lastZetCPUContext->Fetch[e];
	lastZetCPUContext->BasePCData = (u32) lastZetCPUContext->FetchData[e];
	lastZetCPUContext->PC = s + lastZetCPUContext->BasePC;
	lastZetCPUContext->PCData = s + lastZetCPUContext->BasePCData;
*/	
#endif

	return 0;
}

int ZetReset()
{
#ifdef EMU_DOZE
	DozeReset();
#endif

#ifdef EMU_MAME_Z80
	Z80Reset();
#endif

#ifdef EMU_CZ80
	Cz80_Reset( lastZetCPUContext );
#endif

	return 0;
}

int ZetPc(int n)
{
#ifdef EMU_DOZE
	if (n < 0) {
		return Doze.pc;
	} else {
		return ZetCPUContext[n].pc;
	}
#endif

#ifdef EMU_MAME_Z80
	if (n < 0) {
		return lastZetCPUContext->reg.pc.w.l;
	} else {
		return ZetCPUContext[n].reg.pc.w.l;
	}
#endif

#ifdef EMU_CZ80
	if (n < 0) {
		return Cz80_Get_PC(lastZetCPUContext);
	} else {
		return Cz80_Get_PC(&ZetCPUContext[n]);
	}
#endif

	return 0;
}

int ZetBc(int n)
{
#ifdef EMU_DOZE
	if (n < 0) {
		return Doze.bc;
	} else {
		return ZetCPUContext[n].bc;
	}
#endif

#ifdef EMU_MAME_Z80
	if (n < 0) {
		return lastZetCPUContext->reg.bc.w.l;
	} else {
		return ZetCPUContext[n].reg.bc.w.l;
	}
#endif

#ifdef EMU_CZ80
	if (n < 0) {
		return Cz80_Get_BC(lastZetCPUContext);
	} else {
		return Cz80_Get_BC(&ZetCPUContext[n]);
	}
#endif

}
int ZetDe(int n)
{
#ifdef EMU_DOZE
	if (n < 0) {
		return Doze.de;
	} else {
		return ZetCPUContext[n].de;
	}
#endif

// correct?

#ifdef EMU_MAME_Z80
	if (n < 0) {
		return lastZetCPUContext->reg.de.w.l;
	} else {
		return ZetCPUContext[n].reg.de.w.l;
	}
#endif

#ifdef EMU_CZ80
	if (n < 0) {
		return Cz80_Get_DE(lastZetCPUContext);
	} else {
		return Cz80_Get_DE(&ZetCPUContext[n]);
	}
#endif
}
int ZetHL(int n)
{
#ifdef EMU_DOZE
	if (n < 0) {
		return Doze.hl;
	} else {
		return ZetCPUContext[n].hl;
	}
#endif

#ifdef EMU_MAME_Z80
	if (n < 0) {
		return lastZetCPUContext->reg.hl.w.l;
	} else {
		return ZetCPUContext[n].reg.hl.w.l;
	}
#endif

#ifdef EMU_CZ80
	if (n < 0) {
		return Cz80_Get_HL(lastZetCPUContext);
	} else {
		return Cz80_Get_HL(&ZetCPUContext[n]);
	}
#endif
}

int ZetScan(int nAction)
{
	if ((nAction & ACB_DRIVER_DATA) == 0) {
		return 0;
	}

#ifdef EMU_DOZE
	char szText[] = "Z80 #0";

	for (int i = 0; i < nCPUCount; i++) {
		szText[5] = '1' + i;

		ScanVar(&ZetCPUContext[i], 32 + 16, szText);
	}
#endif
#ifdef EMU_CZ80
	char szText[] = "Z80 #0";

	for (int i = 0; i < nCPUCount; i++) {
		szText[5] = '1' + i;

		ScanVar(&ZetCPUContext[i], (unsigned int)&(ZetCPUContext[i].Fetch)-(unsigned int)&ZetCPUContext[i], szText);
	}
#endif

	return 0;
}

void ZetSetIRQLine(const int line, const int status)
{
#ifdef EMU_DOZE
	Doze.nInterruptLatch = line | status;
#endif

#ifdef EMU_MAME_Z80
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
		break;
	}
#endif


#ifdef EMU_CZ80
	lastZetCPUContext->nInterruptLatch = line | status;
#endif
}

int ZetNmi()
{
#ifdef EMU_DOZE
	int nCycles = DozeNmi();
	// Taking an NMI requires 12 cycles
	Doze.nCyclesTotal += nCycles;
#endif

#ifdef EMU_MAME_Z80
	
	Z80SetIrqLine(Z80_INPUT_LINE_NMI, 1);
	Z80Execute(0);
	Z80SetIrqLine(Z80_INPUT_LINE_NMI, 0);
	//Z80Execute(0);
	int nCycles = 12;
	lastZetCPUContext->nCyclesTotal += nCycles;
	
#endif

#ifdef EMU_CZ80
	int nCycles = Cz80_Set_NMI(lastZetCPUContext);
	lastZetCPUContext->nCyclesTotal += nCycles;
#endif

	return nCycles;
}

int ZetIdle(int nCycles)
{
#ifdef EMU_DOZE
	Doze.nCyclesTotal += nCycles;
#endif

#ifdef EMU_MAME_Z80
	lastZetCPUContext->nCyclesTotal += nCycles;
#endif

#ifdef EMU_CZ80
	lastZetCPUContext->nCyclesTotal += nCycles;
#endif

	return nCycles;
}

int ZetSegmentCycles()
{
#ifdef EMU_DOZE
	return Doze.nCyclesSegment - Doze.nCyclesLeft;
#endif

#ifdef EMU_MAME_Z80
	return lastZetCPUContext->nCyclesSegment - lastZetCPUContext->nCyclesLeft;
#endif

#ifdef EMU_CZ80
	return lastZetCPUContext->nCyclesSegment - lastZetCPUContext->nCyclesLeft;
#endif

}

int ZetTotalCycles()
{
#ifdef EMU_DOZE
	return Doze.nCyclesTotal - Doze.nCyclesLeft;
#endif

#ifdef EMU_MAME_Z80
	return lastZetCPUContext->nCyclesTotal - lastZetCPUContext->nCyclesLeft;
#endif

#ifdef EMU_CZ80
	return lastZetCPUContext->nCyclesTotal - lastZetCPUContext->nCyclesLeft;
#endif
}
