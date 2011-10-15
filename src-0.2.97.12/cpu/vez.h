// Nec V20/V30/V33 interface

#include "necintrf.h"

struct VezContext {
	nec_Regs reg;
	
	unsigned char * ppMemRead[512];
	unsigned char * ppMemWrite[512];
	unsigned char * ppMemFetch[512];
	unsigned char * ppMemFetchData[512];

	// Handlers
 #ifdef FASTCALL
	unsigned char (__fastcall *ReadHandler)(unsigned int a);
	void (__fastcall *WriteHandler)(unsigned int a, unsigned char d);
	unsigned char (__fastcall *ReadPort)(unsigned int a);
	void (__fastcall *WritePort)(unsigned int a, unsigned char d);
 #else
	unsigned char (__cdecl *ReadHandler)(unsigned int a);
	void (__cdecl *WriteHandler)(unsigned int a, unsigned char d);
	unsigned char (__cdecl *ReadPort)(unsigned int a);
	void (__cdecl *WritePort)(unsigned int a, unsigned char d);
 #endif
};

extern struct VezContext * VezCurrentCPU;

unsigned char cpu_readmem20(unsigned int a);
void cpu_writemem20(unsigned int a, unsigned char d);

extern int nVezCount;

int VezInit(int nCount, unsigned int * typelist);
void VezExit();
void VezNewFrame();
void VezOpen(int nCPU);
void VezClose();
int VezGetActive();

void VezSetDecode(unsigned char *decode); // set opcode decode

int VezMemCallback(int nStart,int nEnd,int nMode);
int VezMapArea(int nStart, int nEnd, int nMode, unsigned char *Mem);
int VezMapArea(int nStart, int nEnd, int nMode, unsigned char *Mem1, unsigned char *Mem2);

void VezSetReadHandler(unsigned char (__fastcall *)(unsigned int));
void VezSetWriteHandler(void (__fastcall *)(unsigned int, unsigned char));
void VezSetReadPort(unsigned char (__fastcall *)(unsigned int));
void VezSetWritePort(void (__fastcall *)(unsigned int, unsigned char));
void VezSetIrqCallBack(int (*cb)(int));

int VezReset();
int VezPc(int n);
int VezScan(int nAction);

int VezRun(int nCycles);

#define VEZ_IRQSTATUS_NONE 0
#define VEZ_IRQSTATUS_AUTO 1
#define VEZ_IRQSTATUS_ACK  2

void VezSetIRQLine(const int line, const int status);
