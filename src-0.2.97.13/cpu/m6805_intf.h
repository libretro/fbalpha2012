#include "m6805.h"

#define M6805_READ		1
#define M6805_WRITE		2
#define M6805_FETCH		4

#define M6805_ROM		(M6805_READ | M6805_FETCH)
#define M6805_RAM		(M6805_ROM | M6805_WRITE)

void m6805MapMemory(unsigned char *ptr, int nStart, int nEnd, int nType);

void m6805SetWriteHandler(void (*write)(unsigned short, unsigned char));
void m6805SetReadHandler(unsigned char (*read)(unsigned short));

void m6805_write(unsigned short address, unsigned char data);
unsigned char m6805_read(unsigned short address);
unsigned char m6805_fetch(unsigned short address);
void m6805_write_rom(unsigned short address, unsigned char data);

void m6805Init(int num, int address_range);
void m6805Exit();
void m6805Open(int );
void m6805Close();

int m6805Scan(int nAction, int */*pnMin*/);

void m6805Reset();
void m6805SetIrqLine(int , int state);
int m6805Run(int cycles);

void m6805RunEnd();

void m68705Reset();
void m68705SetIrqLine(int irqline, int state);

void hd63705Reset(void);
void hd63705SetIrqLine(int irqline, int state);

void m6805NewFrame();
int m6805TotalCycles();

