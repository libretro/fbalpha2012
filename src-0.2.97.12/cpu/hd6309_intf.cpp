#include "burnint.h"
#include "hd6309_intf.h"

#define MAX_CPU		8

int nHD6309Count = 0;
static int nActiveCPU = 0;

static HD6309Ext *HD6309CPUContext;

static int nHD6309CyclesDone[MAX_CPU];
int nHD6309CyclesTotal;

static unsigned char HD6309ReadByteDummyHandler(unsigned short)
{
	return 0;
}

static void HD6309WriteByteDummyHandler(unsigned short, unsigned char)
{
}

static unsigned char HD6309ReadOpDummyHandler(unsigned short)
{
	return 0;
}

static unsigned char HD6309ReadOpArgDummyHandler(unsigned short)
{
	return 0;
}

void HD6309Reset()
{
	hd6309_reset();
}

void HD6309NewFrame()
{
	for (int i = 0; i < nHD6309Count; i++) {
		nHD6309CyclesDone[i] = 0;
	}
	nHD6309CyclesTotal = 0;
}

int HD6309Init(int num)
{
	nActiveCPU = -1;
	nHD6309Count = num % MAX_CPU;
	
	HD6309CPUContext = (HD6309Ext*)malloc(num * sizeof(HD6309Ext));
	if (HD6309CPUContext == NULL) {
		return 1;
	}

	memset(HD6309CPUContext, 0, num * sizeof(HD6309Ext));
	
	for (int i = 0; i < num; i++) {
		HD6309CPUContext[i].ReadByte = HD6309ReadByteDummyHandler;
		HD6309CPUContext[i].WriteByte = HD6309WriteByteDummyHandler;
		HD6309CPUContext[i].ReadOp = HD6309ReadOpDummyHandler;
		HD6309CPUContext[i].ReadOpArg = HD6309ReadOpArgDummyHandler;
		
		nHD6309CyclesDone[i] = 0;
	
		for (int j = 0; j < (0x0100 * 3); j++) {
			HD6309CPUContext[i].pMemMap[j] = NULL;
		}
	}
	
	nHD6309CyclesTotal = 0;
	
	hd6309_init();

	for (int i = 0; i < num; i++)
		CpuCheatRegister(0x0006, i);

	return 0;
}

void HD6309Exit()
{
	nHD6309Count = 0;

	free(HD6309CPUContext);
	HD6309CPUContext = NULL;
}

void HD6309Open(int num)
{
	nActiveCPU = num;
	
	hd6309_set_context(&HD6309CPUContext[nActiveCPU].reg);
	
	nHD6309CyclesTotal = nHD6309CyclesDone[nActiveCPU];
}

void HD6309Close()
{
	hd6309_get_context(&HD6309CPUContext[nActiveCPU].reg);
	
	nHD6309CyclesDone[nActiveCPU] = nHD6309CyclesTotal;
	
	nActiveCPU = -1;
}

int HD6309GetActive()
{
	return nActiveCPU;
}

void HD6309SetIRQ(int vector, int status)
{
	if (status == HD6309_IRQSTATUS_NONE) {
		hd6309_set_irq_line(vector, 0);
	}
	
	if (status == HD6309_IRQSTATUS_ACK) {
		hd6309_set_irq_line(vector, 1);
	}
	
	if (status == HD6309_IRQSTATUS_AUTO) {
		hd6309_set_irq_line(vector, 1);
		hd6309_execute(0);
		hd6309_set_irq_line(vector, 0);
		hd6309_execute(0);
	}
}

int HD6309Run(int cycles)
{
	cycles = hd6309_execute(cycles);
	
	nHD6309CyclesTotal += cycles;
	
	return cycles;
}

void HD6309RunEnd()
{

}

int HD6309GetPC()
{
	return hd6309_get_pc();
}

int HD6309MapMemory(unsigned char* pMemory, unsigned short nStart, unsigned short nEnd, int nType)
{
	unsigned char cStart = (nStart >> 8);
	unsigned char **pMemMap = HD6309CPUContext[nActiveCPU].pMemMap;

	for (unsigned short i = cStart; i <= (nEnd >> 8); i++) {
		if (nType & HD6309_READ)	{
			pMemMap[0     + i] = pMemory + ((i - cStart) << 8);
		}
		if (nType & HD6309_WRITE) {
			pMemMap[0x100 + i] = pMemory + ((i - cStart) << 8);
		}
		if (nType & HD6309_FETCH) {
			pMemMap[0x200 + i] = pMemory + ((i - cStart) << 8);
		}
	}
	return 0;

}

int HD6309MemCallback(unsigned short nStart, unsigned short nEnd, int nType)
{
	unsigned char cStart = (nStart >> 8);
	unsigned char **pMemMap = HD6309CPUContext[nActiveCPU].pMemMap;

	for (unsigned short i = cStart; i <= (nEnd >> 8); i++) {
		if (nType & HD6309_READ)	{
			pMemMap[0     + i] = NULL;
		}
		if (nType & HD6309_WRITE) {
			pMemMap[0x100 + i] = NULL;
		}
		if (nType & HD6309_FETCH) {
			pMemMap[0x200 + i] = NULL;
		}
	}
	return 0;

}

void HD6309SetReadByteHandler(unsigned char (*pHandler)(unsigned short))
{
	HD6309CPUContext[nActiveCPU].ReadByte = pHandler;
}

void HD6309SetWriteByteHandler(void (*pHandler)(unsigned short, unsigned char))
{
	HD6309CPUContext[nActiveCPU].WriteByte = pHandler;
}

void HD6309SetReadOpHandler(unsigned char (*pHandler)(unsigned short))
{
	HD6309CPUContext[nActiveCPU].ReadOp = pHandler;
}

void HD6309SetReadOpArgHandler(unsigned char (*pHandler)(unsigned short))
{
	HD6309CPUContext[nActiveCPU].ReadOpArg = pHandler;
}

unsigned char HD6309ReadByte(unsigned short Address)
{
	// check mem map
	unsigned char * pr = HD6309CPUContext[nActiveCPU].pMemMap[0x000 | (Address >> 8)];
	if (pr != NULL) {
		return pr[Address & 0xff];
	}
	
	// check handler
	if (HD6309CPUContext[nActiveCPU].ReadByte != NULL) {
		return HD6309CPUContext[nActiveCPU].ReadByte(Address);
	}
	
	return 0;
}

void HD6309WriteByte(unsigned short Address, unsigned char Data)
{
	// check mem map
	unsigned char * pr = HD6309CPUContext[nActiveCPU].pMemMap[0x100 | (Address >> 8)];
	if (pr != NULL) {
		pr[Address & 0xff] = Data;
		return;
	}
	
	// check handler
	if (HD6309CPUContext[nActiveCPU].WriteByte != NULL) {
		HD6309CPUContext[nActiveCPU].WriteByte(Address, Data);
		return;
	}
}

unsigned char HD6309ReadOp(unsigned short Address)
{
	// check mem map
	unsigned char * pr = HD6309CPUContext[nActiveCPU].pMemMap[0x200 | (Address >> 8)];
	if (pr != NULL) {
		return pr[Address & 0xff];
	}
	
	// check handler
	if (HD6309CPUContext[nActiveCPU].ReadOp != NULL) {
		return HD6309CPUContext[nActiveCPU].ReadOp(Address);
	}
	
	return 0;
}

unsigned char HD6309ReadOpArg(unsigned short Address)
{
	// check mem map
	unsigned char * pr = HD6309CPUContext[nActiveCPU].pMemMap[0x200 | (Address >> 8)];
	if (pr != NULL) {
		return pr[Address & 0xff];
	}
	
	// check handler
	if (HD6309CPUContext[nActiveCPU].ReadOpArg != NULL) {
		return HD6309CPUContext[nActiveCPU].ReadOpArg(Address);
	}
	
	return 0;
}

void HD6309WriteRom(unsigned short Address, unsigned char Data)
{
	// check mem map
	unsigned char * pr = HD6309CPUContext[nActiveCPU].pMemMap[0x000 | (Address >> 8)];
	unsigned char * pw = HD6309CPUContext[nActiveCPU].pMemMap[0x100 | (Address >> 8)];
	unsigned char * pf = HD6309CPUContext[nActiveCPU].pMemMap[0x200 | (Address >> 8)];

	if (pr != NULL) {
		pr[Address & 0xff] = Data;
	}
	
	if (pw != NULL) {
		pw[Address & 0xff] = Data;
	}

	if (pf != NULL) {
		pf[Address & 0xff] = Data;
	}

	// check handler
	if (HD6309CPUContext[nActiveCPU].WriteByte != NULL) {
		HD6309CPUContext[nActiveCPU].WriteByte(Address, Data);
		return;
	}
}

int HD6309Scan(int nAction)
{
	struct BurnArea ba;
	
	if ((nAction & ACB_DRIVER_DATA) == 0) {
		return 1;
	}

	for (int i = 0; i < nHD6309Count; i++) {
		char szName[] = "HD6309 #n";
		szName[7] = '0' + i;

		ba.Data = &HD6309CPUContext[i];
		ba.nLen = sizeof(HD6309CPUContext[i]);
		ba.szName = szName;
		BurnAcb(&ba);
	}
	
	return 0;
}
