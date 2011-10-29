
#include "burnint.h"
#include "s2650_intf.h"

#define ADDRESS_MAX	0x8000
#define ADDRESS_MASK	0x7fff
#define PAGE		0x0100
#define PAGE_MASK	0x00ff

#define READ		0
#define WRITE		1
#define FETCH		2

int s2650Count;

struct s2650_handler
{
	unsigned char (*s2650Read)(unsigned short address);
	void (*s2650Write)(unsigned short address, unsigned char data);

	unsigned char (*s2650ReadPort)(unsigned short port);
	void (*s2650WritePort)(unsigned short port, unsigned char data);
	
	unsigned char *mem[3][ADDRESS_MAX / PAGE];
};

struct s2650_handler sHandler[MAX_S2650];
struct s2650_handler *sPointer;

s2650irqcallback s2650_irqcallback[MAX_S2650];

void s2650MapMemory(unsigned char *ptr, int nStart, int nEnd, int nType)
{
	for (int i = nStart / PAGE; i < (nEnd / PAGE) + 1; i++)
	{
		if (nType & (1 <<  READ)) sPointer->mem[ READ][i] = ptr + ((i * PAGE) - nStart);
		if (nType & (1 << WRITE)) sPointer->mem[WRITE][i] = ptr + ((i * PAGE) - nStart);
		if (nType & (1 << FETCH)) sPointer->mem[FETCH][i] = ptr + ((i * PAGE) - nStart);
	}
}

void s2650SetWriteHandler(void (*write)(unsigned short, unsigned char))
{
	sPointer->s2650Write = write;
}

void s2650SetReadHandler(unsigned char (*read)(unsigned short))
{
	sPointer->s2650Read = read;
}

void s2650SetOutHandler(void (*write)(unsigned short, unsigned char))
{
	sPointer->s2650WritePort = write;
}

void s2650SetInHandler(unsigned char (*read)(unsigned short))
{
	sPointer->s2650ReadPort = read;
}

void s2650_write(unsigned short address, unsigned char data)
{
	address &= ADDRESS_MASK;

	if (sPointer->mem[WRITE][address / PAGE] != NULL) {
		sPointer->mem[WRITE][address / PAGE][address & PAGE_MASK] = data;
		return;
	}

	if (sPointer->s2650Write != NULL) {
		sPointer->s2650Write(address, data);
		return;
	}

	return;
}

unsigned char s2650_read(unsigned short address)
{
	address &= ADDRESS_MASK;

	if (sPointer->mem[READ][address / PAGE] != NULL) {
		return sPointer->mem[READ][address / PAGE][address & PAGE_MASK];
	}

	if (sPointer->s2650Read != NULL) {
		return sPointer->s2650Read(address);
	}

	return 0;
}

unsigned char s2650_fetch(unsigned short address)
{
	address &= ADDRESS_MASK;

	if (sPointer->mem[FETCH][address / PAGE] != NULL) {
		return sPointer->mem[FETCH][address / PAGE][address & PAGE_MASK];
	}

	return s2650_read(address);
}

void s2650_write_port(unsigned short port, unsigned char data)
{
	if (sPointer->s2650WritePort != NULL) {
		sPointer->s2650WritePort(port, data);
		return;
	}

	return;
}

unsigned char s2650_read_port(unsigned short port)
{
	if (sPointer->s2650ReadPort != NULL) {
		return sPointer->s2650ReadPort(port);
	}

	return 0;
}

void s2650_write_rom(unsigned short address, unsigned char data)
{
	address &= ADDRESS_MASK;

	if (sPointer->mem[READ][address / PAGE] != NULL) {
		sPointer->mem[READ][address / PAGE][address & PAGE_MASK] = data;
	}

	if (sPointer->mem[WRITE][address / PAGE] != NULL) {
		sPointer->mem[WRITE][address / PAGE][address & PAGE_MASK] = data;
	}

	if (sPointer->mem[FETCH][address / PAGE] != NULL) {
		sPointer->mem[FETCH][address / PAGE][address & PAGE_MASK] = data;
	}

	if (sPointer->s2650Write != NULL) {
		sPointer->s2650Write(address, data);
		return;
	}

	return;
}

void s2650SetIrqCallback(int (*irqcallback)(int))
{
	s2650_irqcallback[nActiveS2650] = irqcallback;
}

void s2650Init(int num)
{
	s2650Count = num;
	memset (&sHandler, 0, sizeof (s2650_handler) * (num % MAX_S2650));
	s2650_init(num);

	for (int i = 0; i < num; i++)
		CpuCheatRegister(0x0008, i);
}

void s2650Exit()
{
	memset (&sHandler, 0, sizeof (sHandler));
	s2650Count = 0;
	s2650_exit();
}

void s2650Open(int num)
{
	sPointer = &sHandler[num % MAX_S2650];
	s2650_open(num);
}

void s2650Close()
{
	s2650_close();
}

int s2650GetPc()
{
	return s2650_get_pc();
}

int s2650GetActive()
{
	return nActiveS2650;
}
