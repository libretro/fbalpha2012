// FM timers

#define TIMER_TICKS_PER_SECOND (2048000000)
#define MAKE_TIMER_TICKS(n, m) ((long long)(n) * TIMER_TICKS_PER_SECOND / (m))
#define MAKE_CPU_CYLES(n, m) ((long long)(n) * (m) / TIMER_TICKS_PER_SECOND)

extern "C" double BurnTimerGetTime();

// Callbacks for various sound chips
void BurnOPNTimerCallback(int n, int c, int cnt, double stepTime);	// period = cnt * stepTime in s
void BurnOPLTimerCallback(int c, double period);					// period in  s
void BurnYMFTimerCallback(int n, int c, double period);				// period in us

// Start / stop a timer
void BurnTimerSetRetrig(int c, double period);						// period in  s
void BurnTimerSetOneshot(int c, double period);						// period in  s

extern double dTime;

void BurnTimerExit();
void BurnTimerReset();
int BurnTimerInit(int (*pOverCallback)(int, int), double (*pTimeCallback)());
int BurnTimerAttachSek(int nClockspeed);
int BurnTimerAttachZet(int nClockspeed);
int BurnTimerAttachM6809(int nClockspeed);
int BurnTimerAttachHD6309(int nClockspeed);
int BurnTimerAttachM6800(int nClockspeed);
int BurnTimerAttachHD63701(int nClockspeed);
int BurnTimerAttachM6803(int nClockspeed);
int BurnTimerAttachM6502(int nClockspeed);
int BurnTimerAttachSh2(int nClockspeed);
void BurnTimerScan(int nAction, int* pnMin);
int BurnTimerUpdate(int nCycles);
void BurnTimerUpdateEnd();
void BurnTimerEndFrame(int nCycles);
