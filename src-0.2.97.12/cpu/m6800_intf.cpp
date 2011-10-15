#include "burnint.h"
#include "m6800_intf.h"

#define MAX_CPU		8

int nM6800Count = 0;

static M6800Ext *M6800CPUContext;

static int nM6800CyclesDone[MAX_CPU];
int nM6800CyclesTotal;

static unsigned char M6800ReadByteDummyHandler(unsigned short)
{
	return 0;
}

static void M6800WriteByteDummyHandler(unsigned short, unsigned char)
{
}

static unsigned char M6800ReadOpDummyHandler(unsigned short)
{
	return 0;
}

static unsigned char M6800ReadOpArgDummyHandler(unsigned short)
{
	return 0;
}

static unsigned char M6800ReadPortDummyHandler(unsigned short)
{
	return 0;
}

static void M6800WritePortDummyHandler(unsigned short, unsigned char)
{
}

void M6800Reset()
{
	m6800_reset();
}

void M6800NewFrame()
{
	for (int i = 0; i < nM6800Count; i++) {
		nM6800CyclesDone[i] = 0;
	}
	nM6800CyclesTotal = 0;
}

int M6800CoreInit(int num, int type)
{
	nM6800Count = num % MAX_CPU;
	
	M6800CPUContext = (M6800Ext*)malloc(num * sizeof(M6800Ext));
	if (M6800CPUContext == NULL) {
		return 1;
	}
	
	memset(M6800CPUContext, 0, num * sizeof(M6800Ext));
	
	for (int i = 0; i < num; i++) {
		M6800CPUContext[i].ReadByte = M6800ReadByteDummyHandler;
		M6800CPUContext[i].WriteByte = M6800WriteByteDummyHandler;
		M6800CPUContext[i].ReadOp = M6800ReadOpDummyHandler;
		M6800CPUContext[i].ReadOpArg = M6800ReadOpArgDummyHandler;
		M6800CPUContext[i].ReadPort = M6800ReadPortDummyHandler;
		M6800CPUContext[i].WritePort = M6800WritePortDummyHandler;
		
		nM6800CyclesDone[i] = 0;
	
		for (int j = 0; j < (0x0100 * 3); j++) {
			M6800CPUContext[i].pMemMap[j] = NULL;
		}
	}
	
	nM6800CyclesTotal = 0;
	
	if (type == CPU_TYPE_M6800) m6800_init();
	if (type == CPU_TYPE_HD63701) hd63701_init();
	if (type == CPU_TYPE_M6803) m6803_init();
	if (type == CPU_TYPE_M6801) m6801_init();

	for (int i = 0; i < num; i++)
		CpuCheatRegister(0x0007, i);

	return 0;
}

int M6800Init(int num)
{
	return M6800CoreInit(num, CPU_TYPE_M6800);
}

int HD63701Init(int num)
{
	return M6800CoreInit(num, CPU_TYPE_HD63701);
}

int M6803Init(int num)
{
	return M6800CoreInit(num, CPU_TYPE_M6803);
}

int M6801Init(int num)
{
	return M6800CoreInit(num, CPU_TYPE_M6801);
}

void M6800Exit()
{
	nM6800Count = 0;

	free(M6800CPUContext);
	M6800CPUContext = NULL;
}

void M6800SetIRQ(int vector, int status)
{
	if (status == M6800_IRQSTATUS_NONE) {
		m6800_set_irq_line(vector, 0);
	}
	
	if (status == M6800_IRQSTATUS_ACK) {
		m6800_set_irq_line(vector, 1);
	}
	
	if (status == M6800_IRQSTATUS_AUTO) {
		m6800_set_irq_line(vector, 1);
		m6800_execute(0);
		m6800_set_irq_line(vector, 0);
		m6800_execute(0);
	}
}

void HD63701SetIRQ(int vector, int status)
{
	if (status == HD63701_IRQSTATUS_NONE) {
		m6800_set_irq_line(vector, 0);
	}
	
	if (status == HD63701_IRQSTATUS_ACK) {
		m6800_set_irq_line(vector, 1);
	}
	
	if (status == HD63701_IRQSTATUS_AUTO) {
		m6800_set_irq_line(vector, 1);
		hd63701_execute(0);
		m6800_set_irq_line(vector, 0);
		hd63701_execute(0);
	}
}

void M6803SetIRQ(int vector, int status)
{
	if (status == M6803_IRQSTATUS_NONE) {
		m6800_set_irq_line(vector, 0);
	}
	
	if (status == M6803_IRQSTATUS_ACK) {
		m6800_set_irq_line(vector, 1);
	}
	
	if (status == M6803_IRQSTATUS_AUTO) {
		m6800_set_irq_line(vector, 1);
		m6803_execute(0);
		m6800_set_irq_line(vector, 0);
		m6803_execute(0);
	}
}

void M6801SetIRQ(int vector, int status)
{
	if (status == M6801_IRQSTATUS_NONE) {
		m6800_set_irq_line(vector, 0);
	}
	
	if (status == M6801_IRQSTATUS_ACK) {
		m6800_set_irq_line(vector, 1);
	}
	
	if (status == M6801_IRQSTATUS_AUTO) {
		m6800_set_irq_line(vector, 1);
		m6803_execute(0);
		m6800_set_irq_line(vector, 0);
		m6803_execute(0);
	}
}

int M6800Run(int cycles)
{
	cycles = m6800_execute(cycles);
	
	nM6800CyclesTotal += cycles;
	
	return cycles;
}

int HD63701Run(int cycles)
{
	cycles = hd63701_execute(cycles);
	
	nM6800CyclesTotal += cycles;
	
	return cycles;
}

int M6803Run(int cycles)
{
	cycles = m6803_execute(cycles);
	
	nM6800CyclesTotal += cycles;
	
	return cycles;
}

void M6800RunEnd()
{

}

int M6800GetPC()
{
	return m6800_get_pc();
}

int M6800MapMemory(unsigned char* pMemory, unsigned short nStart, unsigned short nEnd, int nType)
{
	unsigned char cStart = (nStart >> 8);
	unsigned char **pMemMap = M6800CPUContext[0].pMemMap;

	for (unsigned short i = cStart; i <= (nEnd >> 8); i++) {
		if (nType & M6800_READ)	{
			pMemMap[0     + i] = pMemory + ((i - cStart) << 8);
		}
		if (nType & M6800_WRITE) {
			pMemMap[0x100 + i] = pMemory + ((i - cStart) << 8);
		}
		if (nType & M6800_FETCH) {
			pMemMap[0x200 + i] = pMemory + ((i - cStart) << 8);
		}
	}
	return 0;

}

void M6800SetReadByteHandler(unsigned char (*pHandler)(unsigned short))
{
	M6800CPUContext[0].ReadByte = pHandler;
}

void M6800SetWriteByteHandler(void (*pHandler)(unsigned short, unsigned char))
{
	M6800CPUContext[0].WriteByte = pHandler;
}

void M6800SetReadOpHandler(unsigned char (*pHandler)(unsigned short))
{
	M6800CPUContext[0].ReadOp = pHandler;
}

void M6800SetReadOpArgHandler(unsigned char (*pHandler)(unsigned short))
{
	M6800CPUContext[0].ReadOpArg = pHandler;
}

void M6800SetReadPortHandler(unsigned char (*pHandler)(unsigned short))
{
	M6800CPUContext[0].ReadPort = pHandler;
}

void M6800SetWritePortHandler(void (*pHandler)(unsigned short, unsigned char))
{
	M6800CPUContext[0].WritePort = pHandler;
}

unsigned char M6800ReadByte(unsigned short Address)
{
	// check mem map
	unsigned char * pr = M6800CPUContext[0].pMemMap[0x000 | (Address >> 8)];
	if (pr != NULL) {
		return pr[Address & 0xff];
	}
	
	// check handler
	if (M6800CPUContext[0].ReadByte != NULL) {
		return M6800CPUContext[0].ReadByte(Address);
	}
	
	return 0;
}

void M6800WriteByte(unsigned short Address, unsigned char Data)
{
	// check mem map
	unsigned char * pr = M6800CPUContext[0].pMemMap[0x100 | (Address >> 8)];
	if (pr != NULL) {
		pr[Address & 0xff] = Data;
		return;
	}
	
	// check handler
	if (M6800CPUContext[0].WriteByte != NULL) {
		M6800CPUContext[0].WriteByte(Address, Data);
		return;
	}
}

unsigned char M6800ReadOp(unsigned short Address)
{
	// check mem map
	unsigned char * pr = M6800CPUContext[0].pMemMap[0x200 | (Address >> 8)];
	if (pr != NULL) {
		return pr[Address & 0xff];
	}
	
	// check handler
	if (M6800CPUContext[0].ReadOp != NULL) {
		return M6800CPUContext[0].ReadOp(Address);
	}
	
	return 0;
}

unsigned char M6800ReadOpArg(unsigned short Address)
{
	// check mem map
	unsigned char * pr = M6800CPUContext[0].pMemMap[0x200 | (Address >> 8)];
	if (pr != NULL) {
		return pr[Address & 0xff];
	}
	
	// check handler
	if (M6800CPUContext[0].ReadOpArg != NULL) {
		return M6800CPUContext[0].ReadOpArg(Address);
	}
	
	return 0;
}

unsigned char M6800ReadPort(unsigned short Address)
{
	// check handler
	if (M6800CPUContext[0].ReadPort != NULL) {
		return M6800CPUContext[0].ReadPort(Address);
	}
	
	return 0;
}

void M6800WritePort(unsigned short Address, unsigned char Data)
{
	// check handler
	if (M6800CPUContext[0].WritePort != NULL) {
		M6800CPUContext[0].WritePort(Address, Data);
		return;
	}
}

void M6800WriteRom(unsigned short Address, unsigned char Data)
{
	// check mem map
	unsigned char * pr = M6800CPUContext[0].pMemMap[0x000 | (Address >> 8)];
	unsigned char * pw = M6800CPUContext[0].pMemMap[0x100 | (Address >> 8)];
	unsigned char * pf = M6800CPUContext[0].pMemMap[0x200 | (Address >> 8)];

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
	if (M6800CPUContext[0].WriteByte != NULL) {
		M6800CPUContext[0].WriteByte(Address, Data);
		return;
	}
}

int M6800Scan(int nAction)
{
	struct BurnArea ba;

	if (nAction & ACB_DRIVER_DATA) {
		m6800_Regs *tmp;
		void (* const * insn)(void);
		const UINT8 *cycles;

		for (int i = 0; i < nM6800Count; i++) {
			tmp = &M6800CPUContext[i].reg;
	
			char szName[] = "M6800 #n";
			szName[7] = '0' + i;
	
			cycles = tmp->cycles;
			insn = tmp->insn;
	
			ba.Data = &M6800CPUContext[i].reg;
			ba.nLen = sizeof(M6800CPUContext[i].reg);
			ba.szName = szName;
			BurnAcb(&ba);
	
			tmp->cycles = cycles;
			tmp->insn = insn;
		}
	}

	return 0;
}
