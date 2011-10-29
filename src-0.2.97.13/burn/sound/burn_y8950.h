#include "driver.h"
extern "C" {
 #include "fmopl.h"
}
#include "timer.h"

int BurnTimerUpdateY8950(int nCycles);
void BurnTimerEndFrameY8950(int nCycles);
void BurnTimerUpdateEndY8950();
int BurnTimerAttachSekY8950(int nClockspeed);
int BurnTimerAttachZetY8950(int nClockspeed);
int BurnTimerAttachM6809Y8950(int nClockspeed);
int BurnTimerAttachHD6309Y8950(int nClockspeed);
int BurnTimerAttachM6800Y8950(int nClockspeed);
int BurnTimerAttachHD63701Y8950(int nClockspeed);
int BurnTimerAttachM6803Y8950(int nClockspeed);
int BurnTimerAttachM6502Y8950(int nClockspeed);

extern "C" void BurnY8950UpdateRequest();

int BurnY8950Init(int nClockFrequency, unsigned char* Y8950ADPCMROM, int nY8950ADPCMSize, OPL_IRQHANDLER IRQCallback, int (*StreamCallback)(int), int bAddSignal);
void BurnY8950Reset();
void BurnY8950Exit();
extern void (*BurnY8950Update)(short* pSoundBuf, int nSegmentEnd);
void BurnY8950Scan(int nAction, int* pnMin);

#define BurnY8950Write(a, n) Y8950Write(0, a, n)
#define BurnY8950Read(a) Y8950Read(0, a)
