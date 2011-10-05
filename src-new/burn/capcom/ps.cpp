// PSound (CPS1 sound)
#include "cps.h"
#include "driver.h"
extern "C" {
 #include "ym2151.h"
}

unsigned char PsndCode, PsndFade;						// Sound code/fade sent to the z80 program

static int nSyncPeriod;
static int nSyncNext;

static int nCyclesDone;

static void drvYM2151IRQHandler(int nStatus)
{
	if (nStatus) {
		ZetSetIRQLine(0xFF, ZET_IRQSTATUS_ACK);
		ZetRun(0x0800);
	} else {
		ZetSetIRQLine(0,    ZET_IRQSTATUS_NONE);
	}
}

int PsndInit()
{
	nCpsZ80Cycles = 4000000 * 100 / nBurnFPS;
	nSyncPeriod = nCpsZ80Cycles / 32;

	// Init PSound z80
	if (PsndZInit()!= 0) {
		return 1;
	}

	// Init PSound mixing (not critical if it fails)
	PsmInit();

	YM2151SetIrqHandler(0, &drvYM2151IRQHandler);

	PsndCode = 0; PsndFade = 0;

	nCyclesDone = 0;

	return 0;
}

int PsndExit()
{
	PsmExit();
	PsndZExit();

	return 0;
}

int PsndScan(int nAction)
{
	if (nAction & ACB_DRIVER_DATA) {
		SCAN_VAR(nCyclesDone); SCAN_VAR(nSyncNext);
		PsndZScan(nAction);							// Scan Z80
		SCAN_VAR(PsndCode); SCAN_VAR(PsndFade);		// Scan sound info
	}
	return 0;
}

void PsndNewFrame()
{
	ZetNewFrame();
	PsmNewFrame();
	nSyncNext = nSyncPeriod;

	ZetIdle(nCyclesDone % nCpsZ80Cycles);
	nCyclesDone = 0;
}

int PsndSyncZ80(int nCycles)
{
	while (nSyncNext < nCycles) {
		PsmUpdate(nSyncNext * nBurnSoundLen / nCpsZ80Cycles);
		ZetRun(nSyncNext - ZetTotalCycles());
		nSyncNext += nSyncPeriod;
	}

	nCyclesDone = ZetRun(nCycles - ZetTotalCycles());

	return 0;
}
