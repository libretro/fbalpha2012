#ifndef _DAC_H_
#define _DAC_H_

void DACUpdate(short* Buffer, int Length);
void DACWrite(UINT8 Data);
void DACSignedWrite(UINT8 Data);
void DACInit(int Clock, int bAdd);
void DACSetVolShift(int nShift);
void DACReset();
void DACExit();
int DACScan(int nAction,int *pnMin);

#endif
