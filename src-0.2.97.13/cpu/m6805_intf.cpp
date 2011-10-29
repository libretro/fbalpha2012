
#include "burnint.h"
#include "m6805_intf.h"

static int ADDRESS_MAX;
static int ADDRESS_MASK;
static int PAGE;
static int PAGE_MASK;
static int PAGE_SHIFT;

#define READ		0
#define WRITE		1
#define FETCH		2

static unsigned char (*m6805Read)(unsigned short address);
static void (*m6805Write)(unsigned short address, unsigned char data);

static unsigned char *mem[3][0x100];

void m6805MapMemory(unsigned char *ptr, int nStart, int nEnd, int nType)
{
	for (int i = nStart / PAGE; i < (nEnd / PAGE) + 1; i++)
	{
		if (nType & (1 <<  READ)) mem[ READ][i] = ptr + ((i * PAGE) - nStart);
		if (nType & (1 << WRITE)) mem[WRITE][i] = ptr + ((i * PAGE) - nStart);
		if (nType & (1 << FETCH)) mem[FETCH][i] = ptr + ((i * PAGE) - nStart);
	}
}

void m6805SetWriteHandler(void (*write)(unsigned short, unsigned char))
{
	m6805Write = write;
}

void m6805SetReadHandler(unsigned char (*read)(unsigned short))
{
	m6805Read = read;
}

void m6805_write(unsigned short address, unsigned char data)
{
	address &= ADDRESS_MASK;

	if (mem[WRITE][address >> PAGE_SHIFT] != NULL) {
		mem[WRITE][address >> PAGE_SHIFT][address & PAGE_MASK] = data;
		return;
	}

	if (m6805Write != NULL) {
		m6805Write(address, data);
		return;
	}

	return;
}

unsigned char m6805_read(unsigned short address)
{
	address &= ADDRESS_MASK;

	if (mem[READ][address >> PAGE_SHIFT] != NULL) {
		return mem[READ][address >> PAGE_SHIFT][address & PAGE_MASK];
	}

	if (m6805Read != NULL) {
		return m6805Read(address);
	}

	return 0;
}

unsigned char m6805_fetch(unsigned short address)
{
	address &= ADDRESS_MASK;

	if (mem[FETCH][address >> PAGE_SHIFT] != NULL) {
		return mem[FETCH][address >> PAGE_SHIFT][address & PAGE_MASK];
	}

	return m6805_read(address);
}

void m6805_write_rom(unsigned short address, unsigned char data)
{
	address &= ADDRESS_MASK;

	if (mem[READ][address >> PAGE_SHIFT] != NULL) {
		mem[READ][address >> PAGE_SHIFT][address & PAGE_MASK] = data;
	}

	if (mem[WRITE][address >> PAGE_SHIFT] != NULL) {
		mem[WRITE][address >> PAGE_SHIFT][address & PAGE_MASK] = data;
	}

	if (mem[FETCH][address >> PAGE_SHIFT] != NULL) {
		mem[FETCH][address >> PAGE_SHIFT][address & PAGE_MASK] = data;
	}

	if (m6805Write != NULL) {
		m6805Write(address, data);
		return;
	}

	return;
}

void m6805Init(int num, int max)
{
	ADDRESS_MAX  = max;
	ADDRESS_MASK = ADDRESS_MAX - 1;
	PAGE	     = ADDRESS_MAX / 0x100;
	PAGE_MASK    = PAGE - 1;
	PAGE_SHIFT   = 0;
	for (PAGE_SHIFT = 0; (1 << PAGE_SHIFT) < PAGE; PAGE_SHIFT++) {}

	memset (mem[0], 0, PAGE * sizeof(char *));
	memset (mem[1], 0, PAGE * sizeof(char *));
	memset (mem[2], 0, PAGE * sizeof(char *));

	for (int i = 0; i < num; i++)
		CpuCheatRegister(0x0b, i);
}

void m6805Exit()
{
	ADDRESS_MAX	= 0;
	ADDRESS_MASK	= 0;
	PAGE		= 0;
	PAGE_MASK	= 0;
	PAGE_SHIFT	= 0;
}

void m6805Open(int )
{

}

void m6805Close()
{

}

