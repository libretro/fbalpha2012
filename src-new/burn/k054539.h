#define K054539_RESET_FLAGS     0
#define K054539_REVERSE_STEREO  1
#define K054539_DISABLE_REVERB  2
#define K054539_UPDATE_AT_KEYON 4

void K054539_init_flags(int chip, int flags);
void K054539_set_gain(int chip, int channel, double gain);

void K054539Update(int chip, short *pBuf, int length);

void K054539Write(int chip, int offset, unsigned char data);
unsigned char K054539Read(int chip, int offset);

void K054539Reset(int chip);
void K054539Init(int chip, int clock, unsigned char *rom, int nLen);
void K054539Exit();

int K054539Scan(int nAction);
