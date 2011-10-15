#define UPD7759_STANDARD_CLOCK		640000

typedef void (*drqcallback)(int param);

extern void UPD7759Update(int chip, short *pSoundBuf, int nLength);
extern void UPD7759Reset();
extern void UPD7759Init(int chip, int clock, unsigned char* pSoundData);
extern void UPD7759SetDrqCallback(int chip, drqcallback Callback);
extern int UPD7759BusyRead(int chip);
extern void UPD7759ResetWrite(int chip, UINT8 Data);
extern void UPD7759StartWrite(int chip, UINT8 Data);
extern void UPD7759PortWrite(int chip, UINT8 Data);
extern int UPD7759Scan(int chip, int nAction,int *pnMin);
extern void UPD7759Exit();
