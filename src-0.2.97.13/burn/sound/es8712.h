
void es8712Write(int device, int offset, unsigned char data);
void es8712Play(int device);

void es8712SetBankBase(int device, int base);

void es8712Init(int device, unsigned char *rom, int sample_rate, float volume, int addSignal);
void es8712Reset(int device);
void es8712Update(int device, short *buffer, int samples);
void es8712Exit(int device);

int es8712Scan(int device, int nAction);
