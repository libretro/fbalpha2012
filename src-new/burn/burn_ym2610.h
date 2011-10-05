// burn_ym2610.h
#include "driver.h"
extern "C" {
 #include "ay8910.h"
 #include "fm.h"
}
#include "timer.h"

extern "C" void BurnYM2610UpdateRequest();

int BurnYM2610Init(int nClockFrequency, unsigned char* YM2610ADPCMAROM, int* nYM2610ADPCMASize, unsigned char* YM2610ADPCMBROM, int* nYM2610ADPCMBSize, FM_IRQHANDLER IRQCallback, int (*StreamCallback)(int), double (*GetTimeCallback)(), int bAddSignal);
void BurnYM2610Reset();
void BurnYM2610Exit();
extern void (*BurnYM2610Update)(short* pSoundBuf, int nSegmentEnd);
void BurnYM2610Scan(int nAction, int* pnMin);

#define BurnYM2610Write(a, n) YM2610Write(0, a, n)
#define BurnYM2610Read(a) YM2610Read(0, a)

