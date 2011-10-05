#include "burnint.h"
#include "m6809_intf.h"

#define MAX_CPU		8

int nM6809Count = 0;
static int nActiveCPU = 0;

static M6809Ext *m6809CPUContext;

static int nM6809CyclesDone[MAX_CPU];
int nM6809CyclesTotal;

static unsigned char M6809ReadByteDummyHandler(unsigned short)
{
	return 0;
}

static void M6809WriteByteDummyHandler(unsigned short, unsigned char)
{
}

static unsigned char M6809ReadOpDummyHandler(unsigned short)
{
	return 0;
}

static unsigned char M6809ReadOpArgDummyHandler(unsigned short)
{
	return 0;
}

void M6809Reset()
{
	m6809_reset();
}

void M6809NewFrame()
{
	for (int i = 0; i < nM6809Count; i++) {
		nM6809CyclesDone[i] = 0;
	}
	nM6809CyclesTotal = 0;
}

int M6809Init(int num)
{
	nActiveCPU = -1;
	nM6809Count = num % MAX_CPU;
	
	m6809CPUContext = (M6809Ext*)malloc(num * sizeof(M6809Ext));
	if (m6809CPUContext == NULL) {
		return 1;
	}

	memset(m6809CPUContext, 0, num * sizeof(M6809Ext));
	
	for (int i = 0; i < num; i++) {
		m6809CPUContext[i].ReadByte = M6809ReadByteDummyHandler;
		m6809CPUContext[i].WriteByte = M6809WriteByteDummyHandler;
		m6809CPUContext[i].ReadOp = M6809ReadOpDummyHandler;
		m6809CPUContext[i].ReadOpArg = M6809ReadOpArgDummyHandler;
		
		nM6809CyclesDone[i] = 0;
	
		for (int j = 0; j < (0x0100 * 3); j++) {
			m6809CPUContext[i].pMemMap[j] = NULL;
		}
	}
	
	nM6809CyclesTotal = 0;
	
	m6809_init(NULL);

	for (int i = 0; i < num; i++)
		CpuCheatRegister(0x0005, i);

	return 0;
}

void M6809Exit()
{
	nM6809Count = 0;

	free(m6809CPUContext);
	m6809CPUContext = NULL;
}

void M6809Open(int num)
{
	nActiveCPU = num;
	
	m6809_set_context(&m6809CPUContext[nActiveCPU].reg);
	
	nM6809CyclesTotal = nM6809CyclesDone[nActiveCPU];
}

void M6809Close()
{
	m6809_get_context(&m6809CPUContext[nActiveCPU].reg);
	
	nM6809CyclesDone[nActiveCPU] = nM6809CyclesTotal;
	
	nActiveCPU = -1;
}

int M6809GetActive()
{
	return nActiveCPU;
}

void M6809SetIRQ(int vector, int status)
{
	if (status == M6809_IRQSTATUS_NONE) {
		m6809_set_irq_line(vector, 0);
	}
	
	if (status == M6809_IRQSTATUS_ACK) {
		m6809_set_irq_line(vector, 1);
	}
	
	if (status == M6809_IRQSTATUS_AUTO) {
		m6809_set_irq_line(vector, 1);
		m6809_execute(0);
		m6809_set_irq_line(vector, 0);
		m6809_execute(0);
	}
}

int M6809Run(int cycles)
{
	cycles = m6809_execute(cycles);
	
	nM6809CyclesTotal += cycles;
	
	return cycles;
}

void M6809RunEnd()
{

}

int M6809MapMemory(unsigned char* pMemory, unsigned short nStart, unsigned short nEnd, int nType)
{
	unsigned char cStart = (nStart >> 8);
	unsigned char **pMemMap = m6809CPUContext[nActiveCPU].pMemMap;

	for (unsigned short i = cStart; i <= (nEnd >> 8); i++) {
		if (nType & M6809_READ)	{
			pMemMap[0     + i] = pMemory + ((i - cStart) << 8);
		}
		if (nType & M6809_WRITE) {
			pMemMap[0x100 + i] = pMemory + ((i - cStart) << 8);
		}
		if (nType & M6809_FETCH) {
			pMemMap[0x200 + i] = pMemory + ((i - cStart) << 8);
		}
	}
	return 0;

}

void M6809SetReadByteHandler(unsigned char (*pHandler)(unsigned short))
{
	m6809CPUContext[nActiveCPU].ReadByte = pHandler;
}

void M6809SetWriteByteHandler(void (*pHandler)(unsigned short, unsigned char))
{
	m6809CPUContext[nActiveCPU].WriteByte = pHandler;
}

void M6809SetReadOpHandler(unsigned char (*pHandler)(unsigned short))
{
	m6809CPUContext[nActiveCPU].ReadOp = pHandler;
}

void M6809SetReadOpArgHandler(unsigned char (*pHandler)(unsigned short))
{
	m6809CPUContext[nActiveCPU].ReadOpArg = pHandler;
}

unsigned char M6809ReadByte(unsigned short Address)
{
	// check mem map
	unsigned char * pr = m6809CPUContext[nActiveCPU].pMemMap[0x000 | (Address >> 8)];
	if (pr != NULL) {
		return pr[Address & 0xff];
	}
	
	// check handler
	if (m6809CPUContext[nActiveCPU].ReadByte != NULL) {
		return m6809CPUContext[nActiveCPU].ReadByte(Address);
	}
	
	return 0;
}

void M6809WriteByte(unsigned short Address, unsigned char Data)
{
	// check mem map
	unsigned char * pr = m6809CPUContext[nActiveCPU].pMemMap[0x100 | (Address >> 8)];
	if (pr != NULL) {
		pr[Address & 0xff] = Data;
		return;
	}
	
	// check handler
	if (m6809CPUContext[nActiveCPU].WriteByte != NULL) {
		m6809CPUContext[nActiveCPU].WriteByte(Address, Data);
		return;
	}
}

unsigned char M6809ReadOp(unsigned short Address)
{
	// check mem map
	unsigned char * pr = m6809CPUContext[nActiveCPU].pMemMap[0x200 | (Address >> 8)];
	if (pr != NULL) {
		return pr[Address & 0xff];
	}
	
	// check handler
	if (m6809CPUContext[nActiveCPU].ReadOp != NULL) {
		return m6809CPUContext[nActiveCPU].ReadOp(Address);
	}
	
	return 0;
}

unsigned char M6809ReadOpArg(unsigned short Address)
{
	// check mem map
	unsigned char * pr = m6809CPUContext[nActiveCPU].pMemMap[0x000 | (Address >> 8)];
	if (pr != NULL) {
		return pr[Address & 0xff];
	}
	
	// check handler
	if (m6809CPUContext[nActiveCPU].ReadOpArg != NULL) {
		return m6809CPUContext[nActiveCPU].ReadOpArg(Address);
	}
	
	return 0;
}

void M6809WriteRom(unsigned short Address, unsigned char Data)
{
	unsigned char * pr = m6809CPUContext[nActiveCPU].pMemMap[0x000 | (Address >> 8)];
	unsigned char * pw = m6809CPUContext[nActiveCPU].pMemMap[0x100 | (Address >> 8)];
	unsigned char * pf = m6809CPUContext[nActiveCPU].pMemMap[0x200 | (Address >> 8)];

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
	if (m6809CPUContext[nActiveCPU].WriteByte != NULL) {
		m6809CPUContext[nActiveCPU].WriteByte(Address, Data);
		return;
	}
}

int M6809Scan(int nAction)
{
	struct BurnArea ba;
	
	if ((nAction & ACB_DRIVER_DATA) == 0) {
		return 1;
	}

	for (int i = 0; i < nM6809Count; i++) {

		M6809Ext *ptr = &m6809CPUContext[i];

		int (*Callback)(int irqline);

		Callback = ptr->reg.irq_callback;

		char szName[] = "M6809 #n";
		szName[7] = '0' + i;

		ba.Data = &m6809CPUContext[i].reg;
		ba.nLen = sizeof(m6809CPUContext[i].reg);
		ba.szName = szName;
		BurnAcb(&ba);

		// necessary?
		SCAN_VAR(ptr->nCyclesTotal);
		SCAN_VAR(ptr->nCyclesSegment);
		SCAN_VAR(ptr->nCyclesLeft);

		ptr->reg.irq_callback = Callback;
	}
	
	return 0;
}
