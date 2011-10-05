// Yamaha YMZ280B module

int YMZ280BInit(int nClock, void (*IRQCallback)(int), int nChannels);
void YMZ280BReset();
int YMZ280BScan();
void YMZ280BExit();
int YMZ280BRender(short* pSoundBuf, int nSegmenLength);
void YMZ280BWriteRegister(unsigned char nValue);
unsigned int YMZ280BReadStatus();
unsigned int YMZ280BReadRAM();

extern unsigned char* YMZ280BROM;

// external memory handlers
extern void (*pYMZ280BRAMWrite)(int offset, int nValue);
extern int (*pYMZ280BRAMRead)(int offset);

extern unsigned int nYMZ280BStatus;
extern unsigned int nYMZ280BRegister;

inline static void YMZ280BSelectRegister(unsigned char nRegister)
{
	nYMZ280BRegister = nRegister;
}

inline static void YMZ280BWrite(int offset, unsigned char nValue)
{
	if (offset & 1) {
		YMZ280BWriteRegister(nValue);
	} else {
		nYMZ280BRegister = nValue;
	}
}

inline static unsigned int YMZ280BRead(int offset)
{
	if (offset & 1) {
		return YMZ280BReadStatus();
	} else {
		return YMZ280BReadRAM();
	}

	return 0;
}
