// TC0510NIO

#include "burnint.h"
#include "taito_ic.h"

unsigned char TC0510NIOInputPort0[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
unsigned char TC0510NIOInputPort1[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
unsigned char TC0510NIOInputPort2[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
unsigned char TC0510NIODip[2]        = { 0, 0 };
unsigned char TC0510NIOInput[3]      = { 0, 0, 0 };
static UINT8 TC0510NIORegs[8];

static UINT8 TC0510NIORead(int Offset)
{
	switch (Offset)	{
		case 0x00: return TC0510NIODip[0];
		case 0x01: return TC0510NIODip[1];
		case 0x02: return TC0510NIOInput[0];
		case 0x03: return TC0510NIOInput[1];
		case 0x04: return TC0510NIORegs[4];
		case 0x07: return TC0510NIOInput[2];

		default: {
			return 0xff;
		}
	}
}

static void TC0510NIOWrite(int Offset, UINT16 Data)
{
	TC0510NIORegs[Offset] = Data;
}

UINT16 TC0510NIOHalfWordRead(int Offset)
{
	return TC0510NIORead(Offset);
}

UINT16 TC0510NIOHalfWordSwapRead(int Offset)
{
	return TC0510NIOHalfWordRead(Offset ^ 1);
}

void TC0510NIOHalfWordWrite(int Offset, UINT16 Data)
{
	TC0510NIOWrite(Offset, Data & 0xff);
}

void TC0510NIOHalfWordSwapWrite(int Offset, UINT16 Data)
{
	TC0510NIOWrite(Offset ^ 1, Data);
}

void TC0510NIOReset()
{
	memset(TC0510NIORegs, 0, 8);
}

void TC0510NIOInit()
{
	TaitoIC_TC0510NIOInUse = 1;
}

void TC0510NIOExit()
{
	memset(TC0510NIORegs, 0, 8);
}

void TC0510NIOScan(int nAction)
{
	if (nAction & ACB_DRIVER_DATA) {
		SCAN_VAR(TC0510NIOInputPort0);
		SCAN_VAR(TC0510NIOInputPort1);
		SCAN_VAR(TC0510NIOInputPort2);
		SCAN_VAR(TC0510NIODip);
		SCAN_VAR(TC0510NIOInput);
		SCAN_VAR(TC0510NIORegs);
	}
}
