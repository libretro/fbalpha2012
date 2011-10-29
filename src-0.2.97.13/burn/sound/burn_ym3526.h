#include "driver.h"
extern "C" {
 #include "fmopl.h"
}
#include "timer.h"

int BurnTimerUpdateYM3526(int nCycles);
void BurnTimerEndFrameYM3526(int nCycles);
int BurnTimerAttachSekYM3526(int nClockspeed);
int BurnTimerAttachZetYM3526(int nClockspeed);
int BurnTimerAttachM6809YM3526(int nClockspeed);
int BurnTimerAttachHD6309YM3526(int nClockspeed);
int BurnTimerAttachM6800YM3526(int nClockspeed);
int BurnTimerAttachHD63701YM3526(int nClockspeed);
int BurnTimerAttachM6803YM3526(int nClockspeed);
int BurnTimerAttachM6502YM3526(int nClockspeed);

extern "C" void BurnYM3526UpdateRequest();

int BurnYM3526Init(int nClockFrequency, OPL_IRQHANDLER IRQCallback, int (*StreamCallback)(int), int bAddSignal);
void BurnYM3526Reset();
void BurnYM3526Exit();
extern void (*BurnYM3526Update)(short* pSoundBuf, int nSegmentEnd);
void BurnYM3526Scan(int nAction, int* pnMin);

#define BurnYM3526Write(a, n) YM3526Write(0, a, n)
#define BurnYM3526Read(a) YM3526Read(0, a)
