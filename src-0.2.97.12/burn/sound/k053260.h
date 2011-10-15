void K053260Reset(int chip);
void K053260Update(int chip, short *pBuf, int length);
void K053260Init(int chip, int clock, unsigned char *rom, int nLen);
void K053260Exit();
void K053260Write(int chip, int offset, unsigned char data);
unsigned char K053260Read(int chip, int offset);

int K053260Scan(int nAction);
