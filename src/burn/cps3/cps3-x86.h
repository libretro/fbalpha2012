#include "burnint.h"

#ifndef HARDWARE_CAPCOM_CPS3
	#define HARDWARE_CAPCOM_CPS3		HARDWARE_MISC_MISC
#endif

#ifndef HARDWARE_CAPCOM_CPS3_NO_CD
	#define HARDWARE_CAPCOM_CPS3_NO_CD	HARDWARE_CAPCOM_CPS3
#endif

extern unsigned char cps3_reset;
extern unsigned char cps3_palette_change;

extern unsigned short *Cps3CurPal;

extern unsigned int cps3_key1, cps3_key2, cps3_isSpecial;
extern unsigned int cps3_bios_test_hack, cps3_game_test_hack;
extern unsigned int cps3_speedup_ram_address, cps3_speedup_code_address;
extern unsigned char cps3_dip;
extern unsigned int cps3_region_address, cps3_ncd_address;

extern unsigned char Cps3But1[16];
extern unsigned char Cps3But2[16];
extern unsigned char Cps3But3[16];

int cps3Init();
int cps3Exit();
int cps3Frame();
int cps3Scan(int nAction,int *pnMin);

// sound 

unsigned char __fastcall cps3SndReadByte(unsigned int addr);
unsigned short __fastcall cps3SndReadWord(unsigned int addr);
unsigned int __fastcall cps3SndReadLong(unsigned int addr);

void __fastcall cps3SndWriteByte(unsigned int addr, unsigned char data);
void __fastcall cps3SndWriteWord(unsigned int addr, unsigned short data);
void __fastcall cps3SndWriteLong(unsigned int addr, unsigned int data);

int cps3SndInit(unsigned char *);
void cps3SndReset();
void cps3SndExit();
void cps3SndUpdate();

int cps3SndScan(int);

