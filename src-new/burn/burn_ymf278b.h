#include "driver.h"
extern "C" {
 #include "ymf278b.h"
}

#include "timer.h"

void BurnYMF278BSelectRegister(int nRegister, unsigned char nValue);
void BurnYMF278BWriteRegister(int nRegister, unsigned char nValue);
unsigned char BurnYMF278BReadStatus();
unsigned char BurnYMF278BReadData();

int BurnYMF278BInit(int nClockFrequency, unsigned char* YMF278BROM, void (*IRQCallback)(int, int), int (*StreamCallback)(int));
void BurnYMF278BReset();
void BurnYMF278BExit();
void BurnYMF278BUpdate(int nSegmentEnd);
void BurnYMF278BScan(int nAction, int* pnMin);


