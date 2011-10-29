extern unsigned char* NamcoSoundProm;
void NamcoSoundUpdate(short* buffer, int length);
void NamcoSoundUpdateStereo(short* buffer, int length);
void NamcoSoundWrite(unsigned int offset, unsigned char data);
void NamcoSoundInit(int clock);
void NamcoSoundExit();
void NamcoSoundScan(int nAction,int *pnMin);

void namcos1_custom30_write(int offset, int data);
unsigned char namcos1_custom30_read(int offset);
