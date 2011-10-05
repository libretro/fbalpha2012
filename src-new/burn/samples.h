
#define SAMPLE_IGNORE		0x01 // don't ever play this sample
#define SAMPLE_AUTOLOOP		0x02 // start the looping on start
#define SAMPLE_NOLOOP		0x04 // don't allow this to loop

void BurnSamplePlay(int sample);
void BurnSamplePause(int sample);
void BurnSampleResume(int sample);
void BurnSampleStop(int sample);

void BurnSampleSetLoop(int sample, bool dothis);

int  BurnSampleGetStatus(int sample);

int  BurnSampleGetPosition(int sample);
void BurnSampleSetPosition(int sample, unsigned int position);

void BurnSampleReset();

void BurnSampleInit(int nGain, int bAdd);

int  BurnSampleScan(int nAction, int *pnMin);

void BurnSampleRender(short *pDest, unsigned int pLen);
void BurnSampleExit();
