void K007232Update(int chip, short* pSoundBuf, int nLength);
unsigned char K007232ReadReg(int chip, int r);
void K007232WriteReg(int chip, int r, int v);
void K007232SetPortWriteHandler(int chip, void (*Handler)(int v));
void K007232Init(int chip, int clock, UINT8 *pPCMData, int PCMDataSize);
void K007232Exit();
int K007232Scan(int nAction, int *pnMin);
void K007232SetVolume(int chip, int channel,int volumeA,int volumeB);
void k007232_set_bank( int chip, int chABank, int chBBank );
