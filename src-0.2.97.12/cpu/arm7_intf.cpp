
#include "burnint.h"
#include "arm7_intf.h"

//#define DEBUG_LOG

#define MAX_MEMORY	0x80000000 // more than good enough for pgm
#define PAGE_SIZE	0x00001000 // 400 would be better...
#define PAGE_COUNT	(MAX_MEMORY/PAGE_SIZE)
#define PAGE_SHIFT	12	// 0x1000 -> 12 bits
#define PAGE_BYTE_AND	0x00fff	// 0x1000 - 1 (byte align)
#define PAGE_WORD_AND	0x00ffe	// 0x1000 - 2 (word align)
#define PAGE_LONG_AND	0x00ffc // 0x1000 - 4 (ignore last 4 bytes, long align)

#define READ	0
#define WRITE	1
#define FETCH	2

static unsigned char **membase[3]; // 0 read, 1, write, 2 opcode

static void (*pWriteLongHandler)(unsigned int, unsigned int  ) = NULL;
static void (*pWriteWordHandler)(unsigned int, unsigned short) = NULL;
static void (*pWriteByteHandler)(unsigned int, unsigned char ) = NULL;

static unsigned short (*pReadWordHandler)(unsigned int) = NULL;
static unsigned int   (*pReadLongHandler)(unsigned int) = NULL;
static unsigned char  (*pReadByteHandler)(unsigned int) = NULL;

static unsigned int Arm7IdleLoop = ~0;

void Arm7Init( int num ) // only one cpu supported
{
	for (int i = 0; i < 3; i++) {
		membase[i] = (unsigned char**)malloc(PAGE_COUNT * sizeof(int));
	}

	CpuCheatRegister(0x000a, num);
}

void Arm7Exit() // only one cpu supported
{
	for (int i = 0; i < 3; i++) {
		if (membase[i]) {
			free (membase[i]);
			membase[i] = NULL;
		}
	}

	Arm7IdleLoop = ~0;
}

void Arm7MapMemory(unsigned char *src, int start, int finish, int type)
{
	unsigned int len = (finish-start) >> PAGE_SHIFT;

	for (unsigned int i = 0; i < len+1; i++)
	{
		unsigned int offset = i + (start >> PAGE_SHIFT);
		if (type & (1 <<  READ)) membase[ READ][offset] = src + (i << PAGE_SHIFT);
		if (type & (1 << WRITE)) membase[WRITE][offset] = src + (i << PAGE_SHIFT);
		if (type & (1 << FETCH)) membase[FETCH][offset] = src + (i << PAGE_SHIFT);
	}
}

void Arm7SetWriteByteHandler(void (*write)(unsigned int, unsigned char))
{
	pWriteByteHandler = write;
}

void Arm7SetWriteWordHandler(void (*write)(unsigned int, unsigned short))
{
	pWriteWordHandler = write;
}

void Arm7SetWriteLongHandler(void (*write)(unsigned int, unsigned int))
{
	pWriteLongHandler = write;
}

void Arm7SetReadByteHandler(unsigned char (*read)(unsigned int))
{
	pReadByteHandler = read;
}

void Arm7SetReadWordHandler(unsigned short (*read)(unsigned int))
{
	pReadWordHandler = read;
}

void Arm7SetReadLongHandler(unsigned int (*read)(unsigned int))
{
	pReadLongHandler = read;
}

void Arm7_program_write_byte_32le(unsigned int addr, unsigned char data)
{
#ifdef DEBUG_LOG
	bprintf (PRINT_NORMAL, _T("%5.5x, %2.2x wb\n"), addr, data);
#endif

	if (membase[WRITE][addr >> PAGE_SHIFT] != NULL) {
		membase[WRITE][addr >> PAGE_SHIFT][addr & PAGE_BYTE_AND] = data;
		return;
	}

	if (pWriteByteHandler) {
		pWriteByteHandler(addr, data);
	}
}

void Arm7_program_write_word_32le(unsigned int addr, unsigned short data)
{
#ifdef DEBUG_LOG
	bprintf (PRINT_NORMAL, _T("%5.5x, %8.8x wd\n"), addr, data);
#endif

	if (membase[WRITE][addr >> PAGE_SHIFT] != NULL) {
		*((unsigned short*)(membase[WRITE][addr >> PAGE_SHIFT] + (addr & PAGE_WORD_AND))) = data;
		return;
	}

	if (pWriteWordHandler) {
		pWriteWordHandler(addr, data);
	}
}

void Arm7_program_write_dword_32le(unsigned int addr, unsigned int data)
{
#ifdef DEBUG_LOG
	bprintf (PRINT_NORMAL, _T("%5.5x, %8.8x wd\n"), addr, data);
#endif

	if (membase[WRITE][addr >> PAGE_SHIFT] != NULL) {
		*((unsigned int*)(membase[WRITE][addr >> PAGE_SHIFT] + (addr & PAGE_LONG_AND))) = data;
		return;
	}

	if (pWriteLongHandler) {
		pWriteLongHandler(addr, data);
	}
}


unsigned char Arm7_program_read_byte_32le(unsigned int addr)
{
#ifdef DEBUG_LOG
	bprintf (PRINT_NORMAL, _T("%5.5x, rb\n"), addr);
#endif

	if (membase[ READ][addr >> PAGE_SHIFT] != NULL) {
		return membase[READ][addr >> PAGE_SHIFT][addr & PAGE_BYTE_AND];
	}

	if (pReadByteHandler) {
		return pReadByteHandler(addr);
	}

	return 0;
}

unsigned short Arm7_program_read_word_32le(unsigned int addr)
{
#ifdef DEBUG_LOG
	bprintf (PRINT_NORMAL, _T("%5.5x, rl\n"), addr);
#endif

	if (membase[ READ][addr >> PAGE_SHIFT] != NULL) {
		return *((unsigned short*)(membase[ READ][addr >> PAGE_SHIFT] + (addr & PAGE_WORD_AND)));
	}

	if (pReadWordHandler) {
		return pReadWordHandler(addr);
	}

	return 0;
}

unsigned int Arm7_program_read_dword_32le(unsigned int addr)
{
#ifdef DEBUG_LOG
	bprintf (PRINT_NORMAL, _T("%5.5x, rl\n"), addr);
#endif

	if (membase[ READ][addr >> PAGE_SHIFT] != NULL) {
		return *((unsigned int*)(membase[ READ][addr >> PAGE_SHIFT] + (addr & PAGE_LONG_AND)));
	}

	if (pReadLongHandler) {
		return pReadLongHandler(addr);
	}

	return 0;
}

unsigned short Arm7_program_opcode_word_32le(unsigned int addr)
{
#ifdef DEBUG_LOG
	bprintf (PRINT_NORMAL, _T("%5.5x, rwo\n"), addr);
#endif

	// speed hack -- skip idle loop...
	if (addr == Arm7IdleLoop) {
		Arm7RunEnd();
	}

	if (membase[FETCH][addr >> PAGE_SHIFT] != NULL) {
		return *((unsigned short*)(membase[FETCH][addr >> PAGE_SHIFT] + (addr & PAGE_WORD_AND)));
	}

	// good enough for now...
	if (pReadWordHandler) {
		return pReadWordHandler(addr);
	}

	return 0;
}

unsigned int Arm7_program_opcode_dword_32le(unsigned int addr)
{
#ifdef DEBUG_LOG
	bprintf (PRINT_NORMAL, _T("%5.5x, rlo\n"), addr);
#endif

	// speed hack - skip idle loop...
	if (addr == Arm7IdleLoop) {
		Arm7RunEnd();
	}

	if (membase[FETCH][addr >> PAGE_SHIFT] != NULL) {
		return *((unsigned int*)(membase[FETCH][addr >> PAGE_SHIFT] + (addr & PAGE_LONG_AND)));
	}

	// good enough for now...
	if (pReadLongHandler) {
		return pReadLongHandler(addr);
	}

	return 0;
}

void Arm7SetIRQLine(int line, int state)
{
	if (state == ARM7_CLEAR_LINE || state == ARM7_ASSERT_LINE) {
		arm7_set_irq_line(line, state);
	}
	else if (ARM7_HOLD_LINE) {
		arm7_set_irq_line(line, ARM7_ASSERT_LINE);
		Arm7Run(0);
		arm7_set_irq_line(line, ARM7_CLEAR_LINE);
	}
}

// Set address of idle loop start - speed hack
void Arm7SetIdleLoopAddress(unsigned int address)
{
	Arm7IdleLoop = address;
}


// For cheats/etc

void Arm7_write_rom_byte(unsigned int addr, unsigned char data)
{
	// write to rom & ram
	if (membase[WRITE][addr >> PAGE_SHIFT] != NULL) {
		membase[WRITE][addr >> PAGE_SHIFT][addr & PAGE_BYTE_AND] = data;
	}

	if (membase[READ][addr >> PAGE_SHIFT] != NULL) {
		membase[READ][addr >> PAGE_SHIFT][addr & PAGE_BYTE_AND] = data;
	}

	if (pWriteByteHandler) {
		pWriteByteHandler(addr, data);
	}
}
