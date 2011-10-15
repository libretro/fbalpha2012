#include "burnint.h"
#include "m6502.h"


// Maximum number of 6502 cpus
#define MAX_CPU		8

int nM6502Count = 0;		// how many m6502 cpus are we running?
static int nActiveCPU = 0;	// which m6502 cpu is active?


static struct M6502 m6502_active_cpu_regs[MAX_CPU];
static struct M6502 *m6502_regs = &m6502_active_cpu_regs[0];

void m6502Reset()
{
	Reset6502(m6502_regs);
}

void m6502Init(int num)
{
	nActiveCPU = -1;
	nM6502Count = num % MAX_CPU;

	memset (m6502_active_cpu_regs, 0, sizeof(M6502) * MAX_CPU);

	for (int i = 0; i < num; i++) {
		CpuCheatRegister(0x0003, i);
	}
}

void m6502Exit()
{
	nM6502Count = 0;

	memset (m6502_active_cpu_regs, 0, sizeof(M6502) * MAX_CPU);
}

void m6502Open(int num)
{
	nActiveCPU = num;

	m6502_regs = &m6502_active_cpu_regs[nActiveCPU % MAX_CPU];
}

void m6502Close()
{
	m6502_regs = NULL;
	nActiveCPU = -1;
}

int m6502GetActive()
{
	return nActiveCPU;
}

void m6502SetIRQ(int vector)
{
	if (vector == M6502_NMI)
	{
		Int6502(m6502_regs,INT_NMI);
	}
	else if (vector == M6502_IRQ)
	{
		Int6502(m6502_regs,INT_IRQ);
	}
	else 
	{
		Int6502(m6502_regs,INT_NONE);
	}
}



int m6502MapMemory(unsigned char* pMemory, unsigned short nStart, unsigned short nEnd, int nType)
{
	unsigned char cStart = (nStart >> 8);
	unsigned char **pMemMap = m6502_regs->MemMap;

	for (unsigned short i = cStart; i <= (nEnd >> 8); i++) {
		if (nType & M6502_READ)
		{
			pMemMap[0     + i] = pMemory + ((i - cStart) << 8);
		}
		if (nType & M6502_WRITE)
		{
			pMemMap[0x100 + i] = pMemory + ((i - cStart) << 8);
		}
		if (nType & M6502_FETCH)
		{
			pMemMap[0x200 + i] = pMemory + ((i - cStart) << 8);
		}
	}
	return 0;

}

int m6502Run(int timerTicks)
{
	int tTicks = timerTicks;



	return tTicks - Exec6502(m6502_regs, timerTicks);
}

// set memory handlers

void m6502SetReadHandler(unsigned char (*read)(unsigned short addr))
{
	m6502_regs->m6502_read = read;
}

void m6502SetFetchHandler(unsigned char (*fetch)(unsigned short addr))
{
	m6502_regs->m6502_fetch = fetch;
}

void m6502SetWriteHandler(void (*write)(unsigned short addr, unsigned char data))
{
	m6502_regs->m6502_write = write;
}


// function to read registers
int m6502_get_reg(int reg)
{
	switch (reg)
	{
	case M6502_REG_PC:
		return m6502_regs->PC.W;

	case M6502_REG_A:
		return m6502_regs->A;

	case M6502_REG_X:
		return m6502_regs->X;

	case M6502_REG_Y:
		return m6502_regs->Y;

	case M6502_REG_P:
		return m6502_regs->P;

	case M6502_REG_S:
		return m6502_regs->S;
	}

	return 0;
}


// function to write to registers
void m6502_set_reg(int reg, int data)
{
	switch (reg)
	{
	case M6502_REG_PC:
		m6502_regs->PC.W = data & 0xffff; // word
		break;

	case M6502_REG_A:
		m6502_regs->A = data & 0xff;
		break;

	case M6502_REG_X:
		m6502_regs->X = data & 0xff;
		break;

	case M6502_REG_Y:
		m6502_regs->Y = data & 0xff;
		break;

	case M6502_REG_P:
		m6502_regs->P = data & 0xff;
		break;

	case M6502_REG_S:
		m6502_regs->S = data & 0xff;
		break;
	}
}

int m6502TotalCycles()
{
	return (int)m6502_regs->nTotalCycles;
}

void m6502NewFrame()
{
	struct M6502 *tregs;

	for (int i = 0; i < MAX_CPU; i++) {
		tregs = &m6502_active_cpu_regs[i];
		tregs->nTotalCycles = 0;
	}
}

int m6502Scan(int nAction)
{

	if ((nAction & ACB_DRIVER_DATA) == 0) {
		return 0;
	}

	for (int i = 0; i < MAX_CPU; i++) {
		m6502_regs = &m6502_active_cpu_regs[i];

		SCAN_VAR(m6502_regs->A);
		SCAN_VAR(m6502_regs->X);
		SCAN_VAR(m6502_regs->Y);
		SCAN_VAR(m6502_regs->P);
		SCAN_VAR(m6502_regs->S);
		SCAN_VAR(m6502_regs->PC);
		SCAN_VAR(m6502_regs->nTotalCycles);
	}
	
	return 0;
}

// data
unsigned char Rd6502(unsigned short address)
{
	unsigned char * pr = m6502_regs->MemMap[ address >> 8 ];

	// check memory map
	if (pr != NULL) {
		return pr[ address & 0xff ];
	}

	// check read handler
	if (m6502_regs->m6502_read != NULL) {
		return m6502_regs->m6502_read(address);
	}

	return 0;
}

// data
unsigned char OpArg6502(unsigned short address)
{
	unsigned char * pr = m6502_regs->MemMap[ address >> 8 ];

	// check memory map
	if (pr != NULL) {
		return pr[ address & 0xff ];
	}

	// check read handler
	if (m6502_regs->m6502_read != NULL) {
		return m6502_regs->m6502_read(address);
	}

	return 0;
}


// opcodes
unsigned char Op6502(unsigned short address)
{
	unsigned char * pr = m6502_regs->MemMap[0x200 | (address >> 8) ];

	// check memory map
	if (pr != NULL) {
		return pr[ address & 0xff ];
	}

	// check fetch handler
	if (m6502_regs->m6502_fetch != NULL) {
		return m6502_regs->m6502_fetch(address);
	}

	// check read handler
	return Rd6502(address);
}

// write
void Wr6502(unsigned short address, unsigned char data)
{
	unsigned char * pr = m6502_regs->MemMap[0x100 | (address >> 8) ];

	// check memory map
	if (pr != NULL) {
		pr[ address & 0xff ] = data;
		return;
	}

	// check write handler
	if (m6502_regs->m6502_write != NULL) {
		m6502_regs->m6502_write(address, data);
		return;
	}
}

byte Patch6502(register byte Op,register M6502 */*R*/)
{
	bprintf(PRINT_NORMAL, _T("Patch6502: %x:\n"),Op);

	return 0;
}

// data
unsigned char m6502_read_byte(unsigned short address)
{
	unsigned char * pr = m6502_regs->MemMap[ address >> 8 ];

	// check memory map
	if (pr != NULL) {
		return pr[ address & 0xff ];
	}

	// check read handler
	if (m6502_regs->m6502_read != NULL) {
		return m6502_regs->m6502_read(address);
	}

	return 0;
}

// opcodes
unsigned char m6502_fetch_byte(unsigned short address)
{
	unsigned char * pr = m6502_regs->MemMap[0x200 | (address >> 8) ];

	// check memory map
	if (pr != NULL) {
		return pr[ address & 0xff ];
	}

	// check fetch handler
	if (m6502_regs->m6502_fetch != NULL) {
		return m6502_regs->m6502_fetch(address);
	}

	// check read handler
	return m6502_read_byte(address);
}

// write
void m6502_write_byte(unsigned short address, unsigned char data)
{
	unsigned char * pr = m6502_regs->MemMap[0x100 | (address >> 8) ];

	// check memory map
	if (pr != NULL) {
		pr[ address & 0xff ] = data;
		return;
	}

	// check write handler
	if (m6502_regs->m6502_write != NULL) {
		m6502_regs->m6502_write(address, data);
		return;
	}
}

void m6502_write_rom(unsigned short address, unsigned char data)
{
	unsigned char * pr = m6502_regs->MemMap[0x000 | (address >> 8) ];
	unsigned char * pw = m6502_regs->MemMap[0x100 | (address >> 8) ];
	unsigned char * pf = m6502_regs->MemMap[0x200 | (address >> 8) ];

	// check memory map
	if (pr != NULL) {
		pr[ address & 0xff ] = data;
	}

	if (pw != NULL) {
		pw[ address & 0xff ] = data;
	}

	if (pf != NULL) {
		pf[ address & 0xff ] = data;
	}

	// check write handler
	if (m6502_regs->m6502_write != NULL) {
		m6502_regs->m6502_write(address, data);
		return;
	}
}
