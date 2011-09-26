// Z80 (Zed Eight-Ty) Interface

#ifndef FASTCALL
 #undef __fastcall
 #define __fastcall
#endif

//#define EMU_DOZE					// Use Dave's 'Doze' Assembler Z80 emulator
//#define EMU_MAME_Z80
#define EMU_CZ80

#ifdef EMU_DOZE
 #include "doze.h"
#endif

#ifdef EMU_MAME_Z80
 #include "z80.h"
#endif

#ifdef EMU_CZ80
    #include "cz80.h"
    void ZetSetVector(int vector); 
	#define	CPUINFO_PTR_CPU_SPECIFIC	0x18000
	#define Z80_CLEAR_LINE		0
	#define Z80_ASSERT_LINE		1
	#define Z80_INPUT_LINE_NMI	32
	void ZetWriteRom(unsigned short address, unsigned char data);
#endif
extern int nHasZet;
void ZetWriteByte(unsigned short address, unsigned char data);
unsigned char ZetReadByte(unsigned short address);
int ZetInit(int nCount);
void ZetExit();
void ZetNewFrame();
int ZetOpen(int nCPU);
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
int ZetNmi();
int ZetIdle(int nCycles);
int ZetSegmentCycles();
int ZetTotalCycles();

#ifdef EMU_DOZE
 #define ZetRaiseIrq(n) ZetSetIRQLine(n, ZET_IRQSTATUS_AUTO)
 #define ZetLowerIrq() ZetSetIRQLine(0, ZET_IRQSTATUS_NONE)
#endif

#ifdef EMU_MAME_Z80
 #define ZetRaiseIrq(n) ZetSetIRQLine(n, Z80_ASSERT_LINE)
 #define ZetLowerIrq(n) ZetSetIRQLine(0, Z80_CLEAR_LINE)
#endif

#ifdef EMU_CZ80
 #define ZetRaiseIrq(n) ZetSetIRQLine(n, CZ80_IRQSTATUS_AUTO)
 #define ZetLowerIrq() ZetSetIRQLine(0, CZ80_IRQSTATUS_NONE)
#endif

#ifdef EMU_DOZE
 #define ZET_IRQSTATUS_NONE DOZE_IRQSTATUS_NONE
 #define ZET_IRQSTATUS_AUTO DOZE_IRQSTATUS_AUTO
 #define ZET_IRQSTATUS_ACK  DOZE_IRQSTATUS_ACK
#endif

#ifdef EMU_MAME_Z80
 #define ZET_IRQSTATUS_NONE 0
 #define ZET_IRQSTATUS_ACK  1
 #define ZET_IRQSTATUS_AUTO 2
#endif

#ifdef EMU_CZ80
 #define ZET_IRQSTATUS_NONE CZ80_IRQSTATUS_NONE
 #define ZET_IRQSTATUS_AUTO CZ80_IRQSTATUS_AUTO
 #define ZET_IRQSTATUS_ACK  CZ80_IRQSTATUS_ACK
#endif

#ifdef EMU_MAME_Z80

typedef unsigned char (__fastcall *pZetInHandler)(unsigned short a);
typedef void (__fastcall *pZetOutHandler)(unsigned short a, unsigned char d);
typedef unsigned char (__fastcall *pZetReadHandler)(unsigned short a);
typedef void (__fastcall *pZetWriteHandler)(unsigned short a, unsigned char d);

struct ZetExt {

	Z80_Regs reg;
	
	unsigned char* pZetMemMap[0x100 * 4];

	pZetInHandler ZetIn;
	pZetOutHandler ZetOut;
	pZetReadHandler ZetRead;
	pZetWriteHandler ZetWrite;
	
	int nCyclesTotal;
	int nCyclesSegment;
	int nCyclesLeft;
	
	int nEI;
	int nInterruptLatch;
};

#endif

void ZetSetReadHandler(unsigned char (__fastcall *pHandler)(unsigned short));
void ZetSetWriteHandler(void (__fastcall *pHandler)(unsigned short, unsigned char));
void ZetSetInHandler(unsigned char (__fastcall *pHandler)(unsigned short));
void ZetSetOutHandler(void (__fastcall *pHandler)(unsigned short, unsigned char));
