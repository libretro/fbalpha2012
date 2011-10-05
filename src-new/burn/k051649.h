void K051649Update(short *pBuf, int samples);
void K051649Init(int clock, float gain);
void K051649Reset();
void K051659Exit();

int K051649Scan(int nAction, int *pnMin);

void K051649WaveformWrite(int offset, int data);
unsigned char K051649WaveformRead(int offset);

void K052539WaveformWrite(int offset, int data);

void K051649VolumeWrite(int offset, int data);
void K051649FrequencyWrite(int offset, int data);
void K051649KeyonoffWrite(int data);
