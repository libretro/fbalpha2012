// Timers (for Yamaha FM cips and generic)
#include "burnint.h"
#include "timer.h"
#include "m6809_intf.h"
#include "hd6309_intf.h"
#include "m6800_intf.h"
#include "m6502_intf.h"
#include "sh2.h"

#define MAX_TIMER_VALUE ((1 << 30) - 65536)

double dTime;									// Time elapsed since the emulated machine was started

static int nTimerCount[2], nTimerStart[2];

// Callbacks
static int (*pTimerOverCallback)(int, int);
static double (*pTimerTimeCallback)();

static int nCPUClockspeed = 0;
static int (*pCPUTotalCycles)() = NULL;
static int (*pCPURun)(int) = NULL;
static void (*pCPURunEnd)() = NULL;

// ---------------------------------------------------------------------------
// Running time

static double BurnTimerTimeCallbackDummy()
{
	return 0.0;
}

extern "C" double BurnTimerGetTime()
{
	return dTime + pTimerTimeCallback();
}

// ---------------------------------------------------------------------------
// Update timers

static int nTicksTotal, nTicksDone, nTicksExtra;

int BurnTimerUpdate(int nCycles)
{
	int nIRQStatus = 0;

	nTicksTotal = MAKE_TIMER_TICKS(nCycles, nCPUClockspeed);

//	bprintf(PRINT_NORMAL, _T(" -- Ticks: %08X, cycles %i\n"), nTicksTotal, nCycles);

	while (nTicksDone < nTicksTotal) {
		int nTimer, nCyclesSegment, nTicksSegment;

		// Determine which timer fires first
		if (nTimerCount[0] <= nTimerCount[1]) {
			nTicksSegment = nTimerCount[0];
		} else {
			nTicksSegment = nTimerCount[1];
		}
		if (nTicksSegment > nTicksTotal) {
			nTicksSegment = nTicksTotal;
		}

		nCyclesSegment = MAKE_CPU_CYLES(nTicksSegment + nTicksExtra, nCPUClockspeed);
//		bprintf(PRINT_NORMAL, _T("  - Timer: %08X, %08X, %08X, cycles %i, %i\n"), nTicksDone, nTicksSegment, nTicksTotal, nCyclesSegment, pCPUTotalCycles());

		pCPURun(nCyclesSegment - pCPUTotalCycles());

		nTicksDone = MAKE_TIMER_TICKS(pCPUTotalCycles() + 1, nCPUClockspeed) - 1;
//		bprintf(PRINT_NORMAL, _T("  - ticks done -> %08X cycles -> %i\n"), nTicksDone, pCPUTotalCycles());

		nTimer = 0;
		if (nTicksDone >= nTimerCount[0]) {
			if (nTimerStart[0] == MAX_TIMER_VALUE) {
				nTimerCount[0] = MAX_TIMER_VALUE;
			} else {
				nTimerCount[0] += nTimerStart[0];
			}
//			bprintf(PRINT_NORMAL, _T("  - timer 0 fired\n"));
			nTimer |= 1;
		}
		if (nTicksDone >= nTimerCount[1]) {
			if (nTimerStart[1] == MAX_TIMER_VALUE) {
				nTimerCount[1] = MAX_TIMER_VALUE;
			} else {
				nTimerCount[1] += nTimerStart[1];
			}
//			bprintf(PRINT_NORMAL, _T("  - timer 1 fired\n"));
			nTimer |= 2;
		}
		if (nTimer & 1) {
			nIRQStatus |= pTimerOverCallback(0, 0);
		}
		if (nTimer & 2) {
			nIRQStatus |= pTimerOverCallback(0, 1);
		}
	}

	return nIRQStatus;
}

void BurnTimerEndFrame(int nCycles)
{
	int nTicks = MAKE_TIMER_TICKS(nCycles, nCPUClockspeed);

	BurnTimerUpdate(nCycles);

	if (nTimerCount[0] < MAX_TIMER_VALUE) {
		nTimerCount[0] -= nTicks;
	}
	if (nTimerCount[1] < MAX_TIMER_VALUE) {
		nTimerCount[1] -= nTicks;
	}

	nTicksDone -= nTicks;
	if (nTicksDone < 0) {
//		bprintf(PRINT_ERROR, _T(" -- ticks done -> %08X\n"), nTicksDone);
		nTicksDone = 0;
	}
}

void BurnTimerUpdateEnd()
{
//	bprintf(PRINT_NORMAL, _T("  - end %i\n"), pCPUTotalCycles());

	pCPURunEnd();

	nTicksTotal = 0;
}


// ---------------------------------------------------------------------------
// Callbacks for the sound cores
/*
static int BurnTimerExtraCallbackDummy()
{
	return 0;
}
*/
void BurnOPLTimerCallback(int c, double period)
{
	pCPURunEnd();

	if (period == 0.0) {
		nTimerCount[c] = MAX_TIMER_VALUE;
//		bprintf(PRINT_NORMAL, _T("  - timer %i stopped\n"), c);
		return;
	}

	nTimerCount[c]  = (int)(period * (double)TIMER_TICKS_PER_SECOND);
	nTimerCount[c] += MAKE_TIMER_TICKS(pCPUTotalCycles(), nCPUClockspeed);

//	bprintf(PRINT_NORMAL, _T("  - timer %i started, %08X ticks (fires in %lf seconds)\n"), c, nTimerCount[c], period);
}

void BurnOPMTimerCallback(int c, double period)
{
	pCPURunEnd();
	
	if (period == 0.0) {
		nTimerCount[c] = MAX_TIMER_VALUE;
		return;
	}

	nTimerCount[c]  = (int)(period * (double)TIMER_TICKS_PER_SECOND);
	nTimerCount[c] += MAKE_TIMER_TICKS(pCPUTotalCycles(), nCPUClockspeed);
}

void BurnOPNTimerCallback(int  /*n */, int c, int cnt, double stepTime)
{
	pCPURunEnd();
	
	if (cnt == 0) {
		nTimerCount[c] = MAX_TIMER_VALUE;

//		bprintf(PRINT_NORMAL, _T("  - timer %i stopped\n"), c);

		return;
	}

	nTimerCount[c]  = (int)(stepTime * cnt * (double)TIMER_TICKS_PER_SECOND);
	nTimerCount[c] += MAKE_TIMER_TICKS(pCPUTotalCycles(), nCPUClockspeed);
//	bprintf(PRINT_NORMAL, _T("  - timer %i started, %08X ticks (fires in %lf seconds)\n"), c, nTimerCount[c], stepTime * cnt);
}

void BurnYMFTimerCallback(int /* n */, int c, double period)
{
	pCPURunEnd();

	if (period == 0.0) {
		nTimerStart[c] = nTimerCount[c] = MAX_TIMER_VALUE;

//		bprintf(PRINT_NORMAL, _T("  - timer %i stopped\n"), c);

		return;
	}

	nTimerStart[c]  = nTimerCount[c] = (int)(period * (double)(TIMER_TICKS_PER_SECOND / 1000000));
	nTimerCount[c] += MAKE_TIMER_TICKS(pCPUTotalCycles(), nCPUClockspeed);

//	bprintf(PRINT_NORMAL, _T("  - timer %i started, %08X ticks (fires in %lf seconds)\n"), c, nTimerCount[c], period);
}

void BurnTimerSetRetrig(int c, double period)
{
	pCPURunEnd();

	if (period == 0.0) {
		nTimerStart[c] = nTimerCount[c] = MAX_TIMER_VALUE;

//		bprintf(PRINT_NORMAL, _T("  - timer %i stopped\n"), c);

		return;
	}

	nTimerStart[c]  = nTimerCount[c] = (int)(period * (double)(TIMER_TICKS_PER_SECOND));
	nTimerCount[c] += MAKE_TIMER_TICKS(pCPUTotalCycles(), nCPUClockspeed);

//	bprintf(PRINT_NORMAL, _T("  - timer %i started, %08X ticks (fires in %lf seconds)\n"), c, nTimerCount[c], period);
}

void BurnTimerSetOneshot(int c, double period)
{
	pCPURunEnd();

	if (period == 0.0) {
		nTimerStart[c] = nTimerCount[c] = MAX_TIMER_VALUE;

//		bprintf(PRINT_NORMAL, _T("  - timer %i stopped\n"), c);

		return;
	}

	nTimerCount[c]  = (int)(period * (double)(TIMER_TICKS_PER_SECOND));
	nTimerCount[c] += MAKE_TIMER_TICKS(pCPUTotalCycles(), nCPUClockspeed);

//	bprintf(PRINT_NORMAL, _T("  - timer %i started, %08X ticks (fires in %lf seconds)\n"), c, nTimerCount[c], period / 1000000.0);
}

// ------------------------------------ ---------------------------------------
// Initialisation etc.

void BurnTimerScan(int nAction, int* pnMin)
{
	if (pnMin && *pnMin < 0x029521) {
		*pnMin = 0x029521;
	}

	if (nAction & ACB_DRIVER_DATA) {
		SCAN_VAR(nTimerCount);
		SCAN_VAR(nTimerStart);
		SCAN_VAR(dTime);

		SCAN_VAR(nTicksDone);
	}
}

void BurnTimerExit()
{
	nCPUClockspeed = 0;
	pCPUTotalCycles = NULL;
	pCPURun = NULL;
	pCPURunEnd = NULL;

	return;
}

void BurnTimerReset()
{
	nTimerCount[0] = nTimerCount[1] = MAX_TIMER_VALUE;
	nTimerStart[0] = nTimerStart[1] = MAX_TIMER_VALUE;

	dTime = 0.0;

	nTicksDone = 0;
}

int BurnTimerInit(int (*pOverCallback)(int, int), double (*pTimeCallback)())
{
	BurnTimerExit();

	pTimerOverCallback = pOverCallback;
	pTimerTimeCallback = pTimeCallback ? pTimeCallback : BurnTimerTimeCallbackDummy;

	BurnTimerReset();

	return 0;
}

int BurnTimerAttachSek(int nClockspeed)
{
	nCPUClockspeed = nClockspeed;
	pCPUTotalCycles = SekTotalCycles;
	pCPURun = SekRun;
	pCPURunEnd = SekRunEnd;

	nTicksExtra = MAKE_TIMER_TICKS(1, nCPUClockspeed) - 1;

//	bprintf(PRINT_NORMAL, _T("--- timer cpu speed %iHz, one cycle = %i ticks.\n"), nClockspeed, MAKE_TIMER_TICKS(1, nCPUClockspeed));

	return 0;
}
int BurnTimerAttachZet(int nClockspeed)
{
	nCPUClockspeed = nClockspeed;
	pCPUTotalCycles = ZetTotalCycles;
	pCPURun = ZetRun;
	pCPURunEnd = ZetRunEnd;

	nTicksExtra = MAKE_TIMER_TICKS(1, nCPUClockspeed) - 1;

//	bprintf(PRINT_NORMAL, _T("--- timer cpu speed %iHz, one cycle = %i ticks.\n"), nClockspeed, MAKE_TIMER_TICKS(1, nCPUClockspeed));

	return 0;
}

int BurnTimerAttachM6809(int nClockspeed)
{
	nCPUClockspeed = nClockspeed;
	pCPUTotalCycles = M6809TotalCycles;
	pCPURun = M6809Run;
	pCPURunEnd = M6809RunEnd;

	nTicksExtra = MAKE_TIMER_TICKS(1, nCPUClockspeed) - 1;

//	bprintf(PRINT_NORMAL, _T("--- timer cpu speed %iHz, one cycle = %i ticks.\n"), nClockspeed, MAKE_TIMER_TICKS(1, nCPUClockspeed));

	return 0;
}

int BurnTimerAttachHD6309(int nClockspeed)
{
	nCPUClockspeed = nClockspeed;
	pCPUTotalCycles = HD6309TotalCycles;
	pCPURun = HD6309Run;
	pCPURunEnd = HD6309RunEnd;

	nTicksExtra = MAKE_TIMER_TICKS(1, nCPUClockspeed) - 1;

//	bprintf(PRINT_NORMAL, _T("--- timer cpu speed %iHz, one cycle = %i ticks.\n"), nClockspeed, MAKE_TIMER_TICKS(1, nCPUClockspeed));

	return 0;
}

int BurnTimerAttachM6800(int nClockspeed)
{
	nCPUClockspeed = nClockspeed;
	pCPUTotalCycles = M6800TotalCycles;
	pCPURun = M6800Run;
	pCPURunEnd = M6800RunEnd;

	nTicksExtra = MAKE_TIMER_TICKS(1, nCPUClockspeed) - 1;

//	bprintf(PRINT_NORMAL, _T("--- timer cpu speed %iHz, one cycle = %i ticks.\n"), nClockspeed, MAKE_TIMER_TICKS(1, nCPUClockspeed));

	return 0;
}

int BurnTimerAttachHD63701(int nClockspeed)
{
	nCPUClockspeed = nClockspeed;
	pCPUTotalCycles = M6800TotalCycles;
	pCPURun = HD63701Run;
	pCPURunEnd = HD63701RunEnd;

	nTicksExtra = MAKE_TIMER_TICKS(1, nCPUClockspeed) - 1;

//	bprintf(PRINT_NORMAL, _T("--- timer cpu speed %iHz, one cycle = %i ticks.\n"), nClockspeed, MAKE_TIMER_TICKS(1, nCPUClockspeed));

	return 0;
}

int BurnTimerAttachM6803(int nClockspeed)
{
	nCPUClockspeed = nClockspeed;
	pCPUTotalCycles = M6800TotalCycles;
	pCPURun = M6803Run;
	pCPURunEnd = M6803RunEnd;

	nTicksExtra = MAKE_TIMER_TICKS(1, nCPUClockspeed) - 1;

//	bprintf(PRINT_NORMAL, _T("--- timer cpu speed %iHz, one cycle = %i ticks.\n"), nClockspeed, MAKE_TIMER_TICKS(1, nCPUClockspeed));

	return 0;
}

int BurnTimerAttachM6502(int nClockspeed)
{
	nCPUClockspeed = nClockspeed;
	pCPUTotalCycles = m6502TotalCycles;
	pCPURun = m6502Run;
	pCPURunEnd = M6800RunEnd; // doesn't do anything...

	nTicksExtra = MAKE_TIMER_TICKS(1, nCPUClockspeed) - 1;

//	bprintf(PRINT_NORMAL, _T("--- timer cpu speed %iHz, one cycle = %i ticks.\n"), nClockspeed, MAKE_TIMER_TICKS(1, nCPUClockspeed));

	return 0;
}


int BurnTimerAttachSh2(int nClockspeed)
{
	nCPUClockspeed = nClockspeed;
	pCPUTotalCycles = Sh2TotalCycles;
	pCPURun = Sh2Run;
	pCPURunEnd = Sh2StopRun;

	nTicksExtra = MAKE_TIMER_TICKS(1, nCPUClockspeed) - 1;

//	bprintf(PRINT_NORMAL, _T("--- timer cpu speed %iHz, one cycle = %i ticks.\n"), nClockspeed, MAKE_TIMER_TICKS(1, nCPUClockspeed));

	return 0;
}
