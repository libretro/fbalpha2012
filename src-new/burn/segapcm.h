#define   BANK_256    (11)
#define   BANK_512    (12)
#define   BANK_12M    (13)
#define   BANK_MASK7    (0x70<<16)
#define   BANK_MASKF    (0xf0<<16)
#define   BANK_MASKF8   (0xf8<<16)

extern void SegaPCMUpdate(short* pSoundBuf, int nLength);
extern void SegaPCMInit(int clock, int bank, UINT8 *pPCMData, int PCMDataSize);
extern void SegaPCMExit();
extern int SegaPCMScan(int nAction,int *pnMin);
extern unsigned char SegaPCMRead(unsigned int Offset);
extern void SegaPCMWrite(unsigned int Offset, unsigned char Data);
