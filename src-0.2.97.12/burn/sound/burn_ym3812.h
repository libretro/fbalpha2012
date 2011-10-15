#include "driver.h"
extern "C" {
 #include "fmopl.h"
}
#include "timer.h"

int BurnTimerUpdateYM3812(int nCycles);
void BurnTimerEndFrameYM3812(int nCycles);
void BurnTimerUpdateEndYM3812();
int BurnTimerAttachSekYM3812(int nClockspeed);
int BurnTimerAttachZetYM3812(int nClockspeed);
int BurnTimerAttachM6809YM3812(int nClockspeed);
int BurnTimerAttachHD6309YM3812(int nClockspeed);
int BurnTimerAttachM6800YM3812(int nClockspeed);
int BurnTimerAttachHD63701YM3812(int nClockspeed);
int BurnTimerAttachM6803YM3812(int nClockspeed);
int BurnTimerAttachM6502YM3812(int nClockspeed);

extern "C" void BurnYM3812UpdateRequest();

int BurnYM3812Init(int nClockFrequency, OPL_IRQHANDLER IRQCallback, int (*StreamCallback)(int), int bAddSignal);
void BurnYM3812Reset();
void BurnYM3812Exit();
extern void (*BurnYM3812Update)(short* pSoundBuf, int nSegmentEnd);
void BurnYM3812Scan(int nAction, int* pnMin);

#define BurnYM3812Write(a, n) YM3812Write(0, a, n)
#define BurnYM3812Read(a) YM3812Read(0, a)
