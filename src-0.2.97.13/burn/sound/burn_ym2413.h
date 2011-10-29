// burn_ym2413.h
#include "driver.h"
extern "C" {
 #include "ym2413.h"
}

int BurnYM2413Init(int nClockFrequency, float nVolume);
void BurnYM2413Reset();
void BurnYM2413Exit();
extern void (*BurnYM2413Render)(short* pSoundBuf, int nSegmentLength);
void BurnYM2413Scan(int nAction);
void BurnYM2413IncreaseVolume(int nFactor);
void BurnYM2413DecreaseVolume(int nFactor);

#define BurnYM2413Read(a) YM2413Read(0, a)
#define BurnYM2413Write(a, d) YM2413Write(0, a, d)
//#define BurnYM2413SetUpdateHandler(h) YM2413SetUpdateHandler(0, h)

