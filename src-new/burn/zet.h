// Z80 (Zed Eight-Ty) Interface

#ifndef FASTCALL
 #undef __fastcall
 #define __fastcall
#endif

#include "z80.h"

extern int nHasZet;
void ZetWriteByte(unsigned short address, unsigned char data);
unsigned char ZetReadByte(unsigned short address);
void ZetWriteRom(unsigned short address, unsigned char data);
int ZetInit(int nCount);
void ZetExit();
void ZetNewFrame();
void ZetOpen(int nCPU);
void ZetClose();
int ZetGetActive();
int ZetMemCallback(int nStart,int nEnd,int nMode);
int ZetMemEnd();
int ZetMapArea(int nStart, int nEnd, int nMode, unsigned char *Mem);
int ZetMapArea(int nStart, int nEnd, int nMode, unsigned char *Mem01, unsigned char *Mem02);
int ZetReset();
int ZetPc(int n);
int ZetBc(int n);
int ZetDe(int n);
int ZetHL(int n);
int ZetScan(int nAction);
int ZetRun(int nCycles);
void ZetRunEnd();
void ZetSetIRQLine(const int line, const int status);
void ZetSetVector(int vector);
int ZetNmi();
int ZetIdle(int nCycles);
int ZetSegmentCycles();
int ZetTotalCycles();

#define ZET_IRQSTATUS_NONE 0
#define ZET_IRQSTATUS_ACK  1
#define ZET_IRQSTATUS_AUTO 2

#define ZetRaiseIrq(n) ZetSetIRQLine(n, ZET_IRQSTATUS_AUTO)
#define ZetLowerIrq(n) ZetSetIRQLine(0, Z80_CLEAR_LINE)

void ZetSetReadHandler(unsigned char (__fastcall *pHandler)(unsigned short));
void ZetSetWriteHandler(void (__fastcall *pHandler)(unsigned short, unsigned char));
void ZetSetInHandler(unsigned char (__fastcall *pHandler)(unsigned short));
void ZetSetOutHandler(void (__fastcall *pHandler)(unsigned short, unsigned char));
