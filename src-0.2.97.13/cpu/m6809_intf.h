#include "m6809.h"

typedef unsigned char (*pReadByteHandler)(unsigned short a);
typedef void (*pWriteByteHandler)(unsigned short a, unsigned char d);
typedef unsigned char (*pReadOpHandler)(unsigned short a);
typedef unsigned char (*pReadOpArgHandler)(unsigned short a);

struct M6809Ext {

	m6809_Regs reg;
	
	unsigned char* pMemMap[0x100 * 3];

	pReadByteHandler ReadByte;
	pWriteByteHandler WriteByte;
	pReadOpHandler ReadOp;
	pReadOpArgHandler ReadOpArg;
	
	int nCyclesTotal;
	int nCyclesSegment;
	int nCyclesLeft;
};

#define M6809_IRQSTATUS_NONE	0
#define M6809_IRQSTATUS_ACK	1
#define M6809_IRQSTATUS_AUTO	2

#define M6809_READ	1
#define M6809_WRITE	2
#define M6809_FETCH	4

#define M6809_RAM	(M6809_READ | M6809_WRITE | M6809_FETCH)
#define M6809_ROM	(M6809_READ | M6809_FETCH)

extern int nM6809Count;

extern int nM6809CyclesTotal;

void M6809Reset();
void M6809NewFrame();
int M6809Init(int num);
void M6809Exit();
void M6809Open(int num);
void M6809Close();
int M6809GetActive();
void M6809SetIRQ(int vector, int status);
int M6809Run(int cycles);
void M6809RunEnd();
int M6809MapMemory(unsigned char* pMemory, unsigned short nStart, unsigned short nEnd, int nType);
void M6809SetReadByteHandler(unsigned char (*pHandler)(unsigned short));
void M6809SetWriteByteHandler(void (*pHandler)(unsigned short, unsigned char));
void M6809SetReadOpHandler(unsigned char (*pHandler)(unsigned short));
void M6809SetReadOpArgHandler(unsigned char (*pHandler)(unsigned short));
int M6809Scan(int nAction);

void M6809WriteRom(unsigned short Address, unsigned char Data);

inline static int M6809TotalCycles()
{
	return nM6809CyclesTotal;
}
