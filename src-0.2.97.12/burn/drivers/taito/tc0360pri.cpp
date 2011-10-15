// TC0360PRI

#include "burnint.h"
#include "taito_ic.h"

UINT8 TC0360PRIRegs[16];

void TC0360PRIWrite(unsigned int Offset, unsigned char Data)
{
	TC0360PRIRegs[Offset] = Data;
}

void TC0360PRIHalfWordWrite(unsigned int Offset, unsigned short Data)
{
	TC0360PRIWrite(Offset, Data & 0xff);
}

void TC0360PRIHalfWordSwapWrite(unsigned int Offset, unsigned short Data)
{
	if (Data & 0xff00) TC0360PRIWrite(Offset, (Data >> 8) & 0xff);
}

void TC0360PRIReset()
{
	memset(TC0360PRIRegs, 0, 16);
}

void TC0360PRIInit()
{
	TaitoIC_TC0360PRIInUse = 1;
}

void TC0360PRIExit()
{
	memset(TC0360PRIRegs, 0, 16);
}

void TC0360PRIScan(int nAction)
{
	if (nAction & ACB_DRIVER_DATA) {
		SCAN_VAR(TC0360PRIRegs);
	}
}
