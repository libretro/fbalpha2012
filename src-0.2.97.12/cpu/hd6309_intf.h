#include "hd6309.h"

typedef unsigned char (*pReadByteHandler)(unsigned short a);
typedef void (*pWriteByteHandler)(unsigned short a, unsigned char d);
typedef unsigned char (*pReadOpHandler)(unsigned short a);
typedef unsigned char (*pReadOpArgHandler)(unsigned short a);

struct HD6309Ext {

	hd6309_Regs reg;
	
	unsigned char* pMemMap[0x100 * 3];

	pReadByteHandler ReadByte;
	pWriteByteHandler WriteByte;
	pReadOpHandler ReadOp;
	pReadOpArgHandler ReadOpArg;
	
	int nCyclesTotal;
	int nCyclesSegment;
	int nCyclesLeft;
};

#define HD6309_IRQSTATUS_NONE	0
#define HD6309_IRQSTATUS_ACK	1
#define HD6309_IRQSTATUS_AUTO	2

#define HD6309_READ	1
#define HD6309_WRITE	2
#define HD6309_FETCH	4

#define HD6309_RAM	(HD6309_READ | HD6309_WRITE | HD6309_FETCH)
#define HD6309_ROM	(HD6309_READ | HD6309_FETCH)

extern int nHD6309Count;

extern int nHD6309CyclesTotal;

void HD6309Reset();
void HD6309NewFrame();
int HD6309Init(int num);
void HD6309Exit();
void HD6309Open(int num);
void HD6309Close();
int HD6309GetActive();
void HD6309SetIRQ(int vector, int status);
int HD6309Run(int cycles);
void HD6309RunEnd();
int HD6309GetPC();
int HD6309MapMemory(unsigned char* pMemory, unsigned short nStart, unsigned short nEnd, int nType);
int HD6309MemCallback(unsigned short nStart, unsigned short nEnd, int nType);
void HD6309SetReadByteHandler(unsigned char (*pHandler)(unsigned short));
void HD6309SetWriteByteHandler(void (*pHandler)(unsigned short, unsigned char));
void HD6309SetReadOpHandler(unsigned char (*pHandler)(unsigned short));
void HD6309SetReadOpArgHandler(unsigned char (*pHandler)(unsigned short));
int HD6309Scan(int nAction);

void HD6309WriteRom(unsigned short Address, unsigned char Data);

inline static int HD6309TotalCycles()
{
	return nHD6309CyclesTotal;
}
