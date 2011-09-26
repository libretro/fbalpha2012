// MSM6295 module header

#define MAX_MSM6295 (2)

int MSM6295Init(int nChip, int nSamplerate, float fMaxVolume, bool bAddSignal);
void MSM6295Reset(int nChip);
void MSM6295Exit(int nChip);

int MSM6295Render(int nChip, short* pSoundBuf, int nSegmenLength);
void MSM6295Command(int nChip, unsigned char nCommand);
int MSM6295Scan(int nChip, int nAction);

extern unsigned char* MSM6295ROM;
extern unsigned char* MSM6295SampleInfo[MAX_MSM6295][4];
extern unsigned char* MSM6295SampleData[MAX_MSM6295][4];

inline static unsigned int MSM6295ReadStatus(const int nChip)
{
	extern unsigned int nMSM6295Status[MAX_MSM6295];

	return nMSM6295Status[nChip];
}

