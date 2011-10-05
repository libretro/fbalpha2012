// burn_ym2203.h
#include "driver.h"
extern "C" {
 #include "ay8910.h"
 #include "fm.h"
}
#include "timer.h"

extern "C" void BurnYM2203UpdateRequest();

int BurnYM2203Init(int num, int nClockFrequency, FM_IRQHANDLER IRQCallback, int (*StreamCallback)(int), double (*GetTimeCallback)(), int bAddSignal);
void BurnYM2203Reset();
void BurnYM2203Exit();
extern void (*BurnYM2203Update)(short* pSoundBuf, int nSegmentEnd);
void BurnYM2203Scan(int nAction, int* pnMin);

#define BurnYM2203Write(i, a, n) YM2203Write(i, a, n)
#define BurnYM2203Read(i, a) YM2203Read(i, a)

#define BurnYM2203SetPorts(c, read0, read1, write0, write1)	\
	AY8910SetPorts(c, read0, read1, write0, write1)
