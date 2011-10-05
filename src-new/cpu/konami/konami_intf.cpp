
#include "burnint.h"
#include "konami_intf.h"

#define MEMORY_SPACE	0x10000
#define PAGE_SIZE	0x100
#define PAGE_MASK	0xff
#define PAGE_SHIFT	8
#define PAGE_COUNT	MEMORY_SPACE / PAGE_SIZE

#define READ		0
#define WRITE		1
#define FETCH		2

int nKonamiCpuCount = 0;
static int nKonamiCpuActive = -1;

static unsigned char *mem[3][PAGE_COUNT];

static unsigned char (*konamiRead)(unsigned short address);
static void (*konamiWrite)(unsigned short address, unsigned char data);
static int (*irqcallback)(int);

void konamiMapMemory(unsigned char *src, unsigned short start, unsigned short finish, int type)
{
	unsigned short len = (finish-start) >> PAGE_SHIFT;

	for (unsigned short i = 0; i < len+1; i++)
	{
		unsigned int offset = i + (start >> PAGE_SHIFT);
		if (type & (1 <<  READ)) mem[ READ][offset] = src + (i << PAGE_SHIFT);
		if (type & (1 << WRITE)) mem[WRITE][offset] = src + (i << PAGE_SHIFT);
		if (type & (1 << FETCH)) mem[FETCH][offset] = src + (i << PAGE_SHIFT);
	}
}

int konamiDummyIrqCallback(int)
{
	return 0;
}

void konamiSetIrqCallbackHandler(int (*callback)(int))
{
	irqcallback = callback;
}

void konamiSetWriteHandler(void (*write)(unsigned short, unsigned char))
{
	konamiWrite = write;
}

void konamiSetReadHandler(unsigned char (*read)(unsigned short))
{
	konamiRead = read;
}

void konami_write_rom(unsigned short address, unsigned char data)
{
	if (mem[READ][address >> PAGE_SHIFT] != NULL) {
		mem[READ][address >> PAGE_SHIFT][address & PAGE_MASK] = data;
	}

	if (mem[FETCH][address >> PAGE_SHIFT] != NULL) {
		mem[FETCH][address >> PAGE_SHIFT][address & PAGE_MASK] = data;
	}

	if (mem[WRITE][address >> PAGE_SHIFT] != NULL) {
		mem[WRITE][address >> PAGE_SHIFT][address & PAGE_MASK] = data;
	}

	if (konamiWrite != NULL) {
		konamiWrite(address, data);
	}
}

void konami_write(unsigned short address, unsigned char data)
{
	if (mem[WRITE][address >> PAGE_SHIFT] != NULL) {
		mem[WRITE][address >> PAGE_SHIFT][address & PAGE_MASK] = data;
		return;
	}

	if (konamiWrite != NULL) {
		konamiWrite(address, data);
		return;
	}

	return;
}

unsigned char konami_read(unsigned short address)
{
	if (mem[ READ][address >> PAGE_SHIFT] != NULL) {
		return mem[ READ][address >> PAGE_SHIFT][address & PAGE_MASK];
	}

	if (konamiRead != NULL) {
		return konamiRead(address);
	}

	return 0;
}

unsigned char konami_fetch(unsigned short address)
{
	if (mem[FETCH][address >> PAGE_SHIFT] != NULL) {
		return mem[FETCH][address >> PAGE_SHIFT][address & PAGE_MASK];
	}

	if (konamiRead != NULL) {
		return konamiRead(address);
	}

	return 0;
}

void konamiSetIrqLine(int line, int state)
{
	if (state == KONAMI_HOLD_LINE) {
		konami_set_irq_line(line, KONAMI_HOLD_LINE);
		konamiRun(0);
		konami_set_irq_line(line, KONAMI_CLEAR_LINE);
		konamiRun(0);
	} else {
		konami_set_irq_line(line, state);
	}
}

void konamiInit(int num) // only 1 cpu (No examples exist of multi-cpu konami games)
{
	nKonamiCpuCount = 1;
	konami_init(konamiDummyIrqCallback);

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < (MEMORY_SPACE / PAGE_SIZE); j++) {
			mem[i][j] = NULL;
		}
	}

	CpuCheatRegister(0x0009, num);
}

void konamiExit()
{
	nKonamiCpuCount = 0;
	konamiWrite = NULL;
	konamiRead = NULL;
}

void konamiOpen(int num)
{
	nKonamiCpuActive = num;
}

void konamiClose()
{
	nKonamiCpuActive = -1;
}

int konamiGetActive()
{
	return nKonamiCpuActive;
}
