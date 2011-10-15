// burn_ym2151.h
#include "driver.h"
extern "C" {
 #include "ym2151.h"
}

int BurnYM2151Init(int nClockFrequency, float nVolume);
void BurnYM2151Reset();
void BurnYM2151Exit();
extern void (*BurnYM2151Render)(short* pSoundBuf, int nSegmentLength);
void BurnYM2151Scan(int nAction);

static inline void BurnYM2151SelectRegister(const unsigned char nRegister)
{
	extern unsigned int nBurnCurrentYM2151Register;

	nBurnCurrentYM2151Register = nRegister;
}

static inline void BurnYM2151WriteRegister(const unsigned char nValue)
{
	extern unsigned int nBurnCurrentYM2151Register;
	extern unsigned char BurnYM2151Registers[0x0100];

	BurnYM2151Registers[nBurnCurrentYM2151Register] = nValue;
	YM2151WriteReg(0, nBurnCurrentYM2151Register, nValue);
}

#define BurnYM2151ReadStatus() YM2151ReadStatus(0)
#define BurnYM2151SetIrqHandler(h) YM2151SetIrqHandler(0, h)
#define BurnYM2151SetPortHandler(h) YM2151SetPortWriteHandler(0, h)

