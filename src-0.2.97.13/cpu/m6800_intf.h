#include "M6800.h"

typedef unsigned char (*pReadByteHandler)(unsigned short a);
typedef void (*pWriteByteHandler)(unsigned short a, unsigned char d);
typedef unsigned char (*pReadOpHandler)(unsigned short a);
typedef unsigned char (*pReadOpArgHandler)(unsigned short a);
typedef unsigned char (*pReadPortHandler)(unsigned short a);
typedef void (*pWritePortHandler)(unsigned short a, unsigned char d);

struct M6800Ext {

	m6800_Regs reg;
	
	unsigned char* pMemMap[0x100 * 3];

	pReadByteHandler ReadByte;
	pWriteByteHandler WriteByte;
	pReadOpHandler ReadOp;
	pReadOpArgHandler ReadOpArg;
	pReadPortHandler ReadPort;
	pWritePortHandler WritePort;
	
	int nCyclesTotal;
	int nCyclesSegment;
	int nCyclesLeft;
};

#define M6800_IRQSTATUS_NONE	0
#define M6800_IRQSTATUS_ACK	1
#define M6800_IRQSTATUS_AUTO	2

#define HD63701_IRQSTATUS_NONE	M6800_IRQSTATUS_NONE
#define HD63701_IRQSTATUS_ACK	M6800_IRQSTATUS_ACK
#define HD63701_IRQSTATUS_AUTO	M6800_IRQSTATUS_AUTO

#define HD63701_INPUT_LINE_NMI	M6800_INPUT_LINE_NMI
#define HD63701_IRQ_LINE	M6800_IRQ_LINE
#define HD63701_TIN_LINE	M6800_TIN_LINE

#define M6801_IRQSTATUS_NONE	M6800_IRQSTATUS_NONE
#define M6801_IRQSTATUS_ACK	M6800_IRQSTATUS_ACK
#define M6801_IRQSTATUS_AUTO	M6800_IRQSTATUS_AUTO

#define M6801_INPUT_LINE_NMI	M6800_INPUT_LINE_NMI
#define M6801_IRQ_LINE		M6800_IRQ_LINE
#define M6801_TIN_LINE		M6800_TIN_LINE

#define M6803_IRQSTATUS_NONE	M6800_IRQSTATUS_NONE
#define M6803_IRQSTATUS_ACK	M6800_IRQSTATUS_ACK
#define M6803_IRQSTATUS_AUTO	M6800_IRQSTATUS_AUTO

#define M6803_INPUT_LINE_NMI	M6800_INPUT_LINE_NMI
#define M6803_IRQ_LINE		M6800_IRQ_LINE
#define M6803_TIN_LINE		M6800_TIN_LINE

#define M6800_READ	1
#define M6800_WRITE	2
#define M6800_FETCH	4

#define HD63701_READ	1
#define HD63701_WRITE	2
#define HD63701_FETCH	4

#define M6801_READ	1
#define M6801_WRITE	2
#define M6801_FETCH	4

#define M6803_READ	1
#define M6803_WRITE	2
#define M6803_FETCH	4

#define M6800_RAM	(M6800_READ | M6800_WRITE | M6800_FETCH)
#define M6800_ROM	(M6800_READ | M6800_FETCH)

#define HD63701_RAM	(HD63701_READ | HD63701_WRITE | HD63701_FETCH)
#define HD63701_ROM	(HD63701_READ | HD63701_FETCH)

#define M6801_RAM	(M6801_READ | M6801_WRITE | M6801_FETCH)
#define M6801_ROM	(M6801_READ | M6801_FETCH)

#define M6803_RAM	(M6803_READ | M6803_WRITE | M6803_FETCH)
#define M6803_ROM	(M6803_READ | M6803_FETCH)

#define CPU_TYPE_M6800		1
#define CPU_TYPE_HD63701	2
#define CPU_TYPE_M6803		3
#define CPU_TYPE_M6801		4

extern int nM6800Count;

extern int nM6800CyclesTotal;

void M6800Reset();
#define HD63701Reset		M6800Reset
#define M6803Reset		M6800Reset
#define M6801Reset		M6800Reset

void M6800NewFrame();
#define HD63701NewFrame		M6800NewFrame
#define M6803NewFrame		M6800NewFrame
#define M6801NewFrame		M6800NewFrame

int M6800CoreInit(int num, int type);
int M6800Init(int num);
int HD63701Init(int num);
int M6803Init(int num);
int M6801Init(int num);

void M6800Exit();
#define HD63701Exit		M6800Exit
#define M6803Exit		M6800Exit
#define M6801Exit		M6800Exit

void M6800SetIRQ(int vector, int status);
void HD63701SetIRQ(int vector, int status);
void M6803SetIRQ(int vector, int status);
void M6801SetIRQ(int vector, int status);

int M6800Run(int cycles);
int HD63701Run(int cycles);
int M6803Run(int cycles);
#define M6801Run(nCycles)	M6803Run(nCycles)

void M6800RunEnd();
#define HD63701RunEnd		M6800RunEnd
#define M6803RunEnd		M6800RunEnd
#define M6801RunEnd		M6800RunEnd

int M6800GetPC();
#define HD63701GetPC		M6800GetPC
#define M6803GetPC		M6800GetPC
#define M6801GetPC		M6800GetPC

int M6800MapMemory(unsigned char* pMemory, unsigned short nStart, unsigned short nEnd, int nType);
#define HD63701MapMemory	M6800MapMemory
#define M6803MapMemory		M6800MapMemory
#define M6801MapMemory		M6800MapMemory

void M6800SetReadByteHandler(unsigned char (*pHandler)(unsigned short));
#define HD63701SetReadByteHandler	M6800SetReadByteHandler
#define M6803SetReadByteHandler		M6800SetReadByteHandler
#define M6801SetReadByteHandler		M6800SetReadByteHandler

void M6800SetWriteByteHandler(void (*pHandler)(unsigned short, unsigned char));
#define HD63701SetWriteByteHandler	M6800SetWriteByteHandler
#define M6803SetWriteByteHandler	M6800SetWriteByteHandler
#define M6801SetWriteByteHandler	M6800SetWriteByteHandler

void M6800SetReadOpHandler(unsigned char (*pHandler)(unsigned short));
#define HD63701SetReadOpHandler		M6800SetReadOpHandler
#define M6803SetReadOpHandler		M6800SetReadOpHandler
#define M6801SetReadOpHandler		M6800SetReadOpHandler

void M6800SetReadOpArgHandler(unsigned char (*pHandler)(unsigned short));
#define HD63701SetReadOpArgHandler	M6800SetReadOpArgHandler
#define M6803SetReadOpArgHandler	M6800SetReadOpArgHandler
#define M6801SetReadOpArgHandler	M6800SetReadOpArgHandler

void M6800SetReadPortHandler(unsigned char (*pHandler)(unsigned short));
#define HD63701SetReadPortHandler	M6800SetReadPortHandler
#define M6803SetReadPortHandler		M6800SetReadPortHandler
#define M6801SetReadPortHandler		M6800SetReadPortHandler

void M6800SetWritePortHandler(void (*pHandler)(unsigned short, unsigned char));
#define HD63701SetWritePortHandler	M6800SetWritePortHandler
#define M6803SetWritePortHandler	M6800SetWritePortHandler
#define M6801SetWritePortHandler	M6800SetWritePortHandler

int M6800Scan(int nAction);
#define HD63701Scan		M6800Scan
#define M6803Scan		M6800Scan
#define M6801Scan		M6800Scan

void M6800WriteRom(unsigned short Address, unsigned char Data);

inline static int M6800TotalCycles()
{
	return nM6800CyclesTotal;
}
