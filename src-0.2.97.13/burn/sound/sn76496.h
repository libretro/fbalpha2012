extern void SN76496Update(int Num, short* pSoundBuf, int Length);
extern void SN76496Write(int Num, int Data);
extern void SN76489Init(int Num, int Clock, int SignalAdd);
extern void SN76489AInit(int Num, int Clock, int SignalAdd);
extern void SN76494Init(int Num, int Clock, int SignalAdd);
extern void SN76496Init(int Num, int Clock, int SignalAdd);
extern void SN76496Exit();
extern int SN76496Scan(int nAction,int *pnMin);
