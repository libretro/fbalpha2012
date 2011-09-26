#include "burnint.h"
#include "burn_sound.h"
#include "burn_ym3526.h"
#include "m6809_intf.h"
#include "hd6309_intf.h"
#include "m6800_intf.h"
#include "m6502_intf.h"

// Timer Related

#define MAX_TIMER_VALUE ((1 << 30) - 65536)

static double dTimeYM3526;									// Time elapsed since the emulated machine was started

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

extern "C" double BurnTimerGetTimeYM3526()
{
	return dTimeYM3526 + pTimerTimeCallback();
}

// ---------------------------------------------------------------------------
// Update timers

static int nTicksTotal, nTicksDone, nTicksExtra;

int BurnTimerUpdateYM3526(int nCycles)
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

void BurnTimerEndFrameYM3526(int nCycles)
{
	int nTicks = MAKE_TIMER_TICKS(nCycles, nCPUClockspeed);

	BurnTimerUpdateYM3526(nCycles);

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

void BurnTimerUpdateEndYM3526()
{
//	bprintf(PRINT_NORMAL, _T("  - end %i\n"), pCPUTotalCycles());

	pCPURunEnd();

	nTicksTotal = 0;
}

void BurnOPLTimerCallbackYM3526(int c, double period)
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

void BurnTimerScanYM3526(int nAction, int* pnMin)
{
	if (pnMin && *pnMin < 0x029521) {
		*pnMin = 0x029521;
	}

	if (nAction & ACB_DRIVER_DATA) {
		SCAN_VAR(nTimerCount);
		SCAN_VAR(nTimerStart);
		SCAN_VAR(dTimeYM3526);

		SCAN_VAR(nTicksDone);
	}
}

void BurnTimerExitYM3526()
{
	nCPUClockspeed = 0;
	pCPUTotalCycles = NULL;
	pCPURun = NULL;
	pCPURunEnd = NULL;

	return;
}

void BurnTimerResetYM3526()
{
	nTimerCount[0] = nTimerCount[1] = MAX_TIMER_VALUE;
	nTimerStart[0] = nTimerStart[1] = MAX_TIMER_VALUE;

	dTimeYM3526 = 0.0;

	nTicksDone = 0;
}

int BurnTimerInitYM3526(int (*pOverCallback)(int, int), double (*pTimeCallback)())
{
	BurnTimerExitYM3526();

	pTimerOverCallback = pOverCallback;
	pTimerTimeCallback = pTimeCallback ? pTimeCallback : BurnTimerTimeCallbackDummy;

	BurnTimerResetYM3526();

	return 0;
}

int BurnTimerAttachSekYM3526(int nClockspeed)
{
	nCPUClockspeed = nClockspeed;
	pCPUTotalCycles = SekTotalCycles;
	pCPURun = SekRun;
	pCPURunEnd = SekRunEnd;

	nTicksExtra = MAKE_TIMER_TICKS(1, nCPUClockspeed) - 1;

//	bprintf(PRINT_NORMAL, _T("--- timer cpu speed %iHz, one cycle = %i ticks.\n"), nClockspeed, MAKE_TIMER_TICKS(1, nCPUClockspeed));

	return 0;
}

int BurnTimerAttachZetYM3526(int nClockspeed)
{
	nCPUClockspeed = nClockspeed;
	pCPUTotalCycles = ZetTotalCycles;
	pCPURun = ZetRun;
	pCPURunEnd = ZetRunEnd;

	nTicksExtra = MAKE_TIMER_TICKS(1, nCPUClockspeed) - 1;

//	bprintf(PRINT_NORMAL, _T("--- timer cpu speed %iHz, one cycle = %i ticks.\n"), nClockspeed, MAKE_TIMER_TICKS(1, nCPUClockspeed));

	return 0;
}

int BurnTimerAttachM6809YM3526(int nClockspeed)
{
	nCPUClockspeed = nClockspeed;
	pCPUTotalCycles = M6809TotalCycles;
	pCPURun = M6809Run;
	pCPURunEnd = M6809RunEnd;

	nTicksExtra = MAKE_TIMER_TICKS(1, nCPUClockspeed) - 1;

//	bprintf(PRINT_NORMAL, _T("--- timer cpu speed %iHz, one cycle = %i ticks.\n"), nClockspeed, MAKE_TIMER_TICKS(1, nCPUClockspeed));

	return 0;
}

int BurnTimerAttachHD6309YM3526(int nClockspeed)
{
	nCPUClockspeed = nClockspeed;
	pCPUTotalCycles = HD6309TotalCycles;
	pCPURun = HD6309Run;
	pCPURunEnd = HD6309RunEnd;

	nTicksExtra = MAKE_TIMER_TICKS(1, nCPUClockspeed) - 1;

//	bprintf(PRINT_NORMAL, _T("--- timer cpu speed %iHz, one cycle = %i ticks.\n"), nClockspeed, MAKE_TIMER_TICKS(1, nCPUClockspeed));

	return 0;
}

int BurnTimerAttachM6800YM3526(int nClockspeed)
{
	nCPUClockspeed = nClockspeed;
	pCPUTotalCycles = M6800TotalCycles;
	pCPURun = M6800Run;
	pCPURunEnd = M6800RunEnd;

	nTicksExtra = MAKE_TIMER_TICKS(1, nCPUClockspeed) - 1;

//	bprintf(PRINT_NORMAL, _T("--- timer cpu speed %iHz, one cycle = %i ticks.\n"), nClockspeed, MAKE_TIMER_TICKS(1, nCPUClockspeed));

	return 0;
}

int BurnTimerAttachHD63701YM3526(int nClockspeed)
{
	nCPUClockspeed = nClockspeed;
	pCPUTotalCycles = M6800TotalCycles;
	pCPURun = HD63701Run;
	pCPURunEnd = HD63701RunEnd;

	nTicksExtra = MAKE_TIMER_TICKS(1, nCPUClockspeed) - 1;

//	bprintf(PRINT_NORMAL, _T("--- timer cpu speed %iHz, one cycle = %i ticks.\n"), nClockspeed, MAKE_TIMER_TICKS(1, nCPUClockspeed));

	return 0;
}

int BurnTimerAttachM6803YM3526(int nClockspeed)
{
	nCPUClockspeed = nClockspeed;
	pCPUTotalCycles = M6800TotalCycles;
	pCPURun = M6803Run;
	pCPURunEnd = M6803RunEnd;

	nTicksExtra = MAKE_TIMER_TICKS(1, nCPUClockspeed) - 1;

//	bprintf(PRINT_NORMAL, _T("--- timer cpu speed %iHz, one cycle = %i ticks.\n"), nClockspeed, MAKE_TIMER_TICKS(1, nCPUClockspeed));

	return 0;
}

int BurnTimerAttachM6502YM3526(int nClockspeed)
{
	nCPUClockspeed = nClockspeed;
	pCPUTotalCycles = m6502TotalCycles;
	pCPURun = m6502Run;
	pCPURunEnd = M6800RunEnd; // doesn't do anything...

	nTicksExtra = MAKE_TIMER_TICKS(1, nCPUClockspeed) - 1;

//	bprintf(PRINT_NORMAL, _T("--- timer cpu speed %iHz, one cycle = %i ticks.\n"), nClockspeed, MAKE_TIMER_TICKS(1, nCPUClockspeed));

	return 0;
}

// Sound Related

void (*BurnYM3526Update)(short* pSoundBuf, int nSegmentEnd);

static int (*BurnYM3526StreamCallback)(int nSoundRate);

static int nBurnYM3526SoundRate;

static short* pBuffer;
static short* pYM3526Buffer;

static int nYM3526Position;

static unsigned int nSampleSize;
static unsigned int nFractionalPosition;

static int bYM3526AddSignal;

// ----------------------------------------------------------------------------
// Dummy functions

static void YM3526UpdateDummy(short* , int /* nSegmentEnd */)
{
	return;
}

static int YM3526StreamCallbackDummy(int /* nSoundRate */)
{
	return 0;
}

// ----------------------------------------------------------------------------
// Execute YM3526 for part of a frame

static void YM3526Render(int nSegmentLength)
{
	if (nYM3526Position >= nSegmentLength) {
		return;
	}

//	bprintf(PRINT_NORMAL, _T("    YM3526 render %6i -> %6i\n", nYM3526Position, nSegmentLength));

	nSegmentLength -= nYM3526Position;

	YM3526UpdateOne(0, pBuffer + 0 * 4096 + 4 + nYM3526Position, nSegmentLength);

	nYM3526Position += nSegmentLength;
}

// ----------------------------------------------------------------------------
// Update the sound buffer

static void YM3526UpdateResample(short* pSoundBuf, int nSegmentEnd)
{
	int nSegmentLength = nSegmentEnd;
	int nSamplesNeeded = nSegmentEnd * nBurnYM3526SoundRate / nBurnSoundRate + 1;

//	bprintf(PRINT_NORMAL, _T("    YM3526 update        -> %6i\n", nSegmentLength));

	if (nSamplesNeeded < nYM3526Position) {
		nSamplesNeeded = nYM3526Position;
	}

	if (nSegmentLength > nBurnSoundLen) {
		nSegmentLength = nBurnSoundLen;
	}
	nSegmentLength <<= 1;

	YM3526Render(nSamplesNeeded);

	pYM3526Buffer = pBuffer + 0 * 4096 + 4;

	for (int i = (nFractionalPosition & 0xFFFF0000) >> 15; i < nSegmentLength; i += 2, nFractionalPosition += nSampleSize) {
		short nSample =  INTERPOLATE4PS_16BIT((nFractionalPosition >> 4) & 0x0FFF,
												pYM3526Buffer[(nFractionalPosition >> 16) - 3],
												pYM3526Buffer[(nFractionalPosition >> 16) - 2],
												pYM3526Buffer[(nFractionalPosition >> 16) - 1],
												pYM3526Buffer[(nFractionalPosition >> 16) - 0]);
		if (bYM3526AddSignal) {
			pSoundBuf[i + 0] += nSample;
			pSoundBuf[i + 1] += nSample;
		} else {
			pSoundBuf[i + 0] = nSample;
			pSoundBuf[i + 1] = nSample;
		}
	}

	if (nSegmentEnd >= nBurnSoundLen) {
		int nExtraSamples = nSamplesNeeded - (nFractionalPosition >> 16);

//		bprintf(PRINT_NORMAL, _T("   %6i rendered, %i extra, %i <- %i\n"), nSamplesNeeded, nExtraSamples, nExtraSamples, (nFractionalPosition >> 16) + nExtraSamples - 1);

		for (int i = -4; i < nExtraSamples; i++) {
			pYM3526Buffer[i] = pYM3526Buffer[(nFractionalPosition >> 16) + i];
		}

		nFractionalPosition &= 0xFFFF;

		nYM3526Position = nExtraSamples;
	}
}

static void YM3526UpdateNormal(short* pSoundBuf, int nSegmentEnd)
{
	int nSegmentLength = nSegmentEnd;

//	bprintf(PRINT_NORMAL, _T("    YM3526 render %6i -> %6i\n"), nYM3526Position, nSegmentEnd);

	if (nSegmentEnd < nYM3526Position) {
		nSegmentEnd = nYM3526Position;
	}

	if (nSegmentLength > nBurnSoundLen) {
		nSegmentLength = nBurnSoundLen;
	}

	YM3526Render(nSegmentEnd);

	pYM3526Buffer = pBuffer + 4 + 0 * 4096;

	for (int i = nFractionalPosition; i < nSegmentLength; i++) {
		if (bYM3526AddSignal) {
			pSoundBuf[(i << 1) + 0] += pYM3526Buffer[i];
			pSoundBuf[(i << 1) + 1] += pYM3526Buffer[i];
		} else {
			pSoundBuf[(i << 1) + 0] = pYM3526Buffer[i];
			pSoundBuf[(i << 1) + 1] = pYM3526Buffer[i];
		}
	}

	nFractionalPosition = nSegmentLength;

	if (nSegmentEnd >= nBurnSoundLen) {
		int nExtraSamples = nSegmentEnd - nBurnSoundLen;

		for (int i = 0; i < nExtraSamples; i++) {
			pYM3526Buffer[i] = pYM3526Buffer[nBurnSoundLen + i];
		}

		nFractionalPosition = 0;

		nYM3526Position = nExtraSamples;

	}
}

// ----------------------------------------------------------------------------
// Callbacks for YM3526 core

void BurnYM3526UpdateRequest(int, int)
{
	YM3526Render(BurnYM3526StreamCallback(nBurnYM3526SoundRate));
}

// ----------------------------------------------------------------------------
// Initialisation, etc.

void BurnYM3526Reset()
{
	BurnTimerResetYM3526();

	YM3526ResetChip(0);
}

void BurnYM3526Exit()
{
	YM3526Shutdown();

	BurnTimerExitYM3526();

	free(pBuffer);
	
	bYM3526AddSignal = 0;
}

int BurnYM3526Init(int nClockFrequency, OPL_IRQHANDLER IRQCallback, int (*StreamCallback)(int), int bAddSignal)
{
	BurnTimerInitYM3526(&YM3526TimerOver, NULL);

	if (nBurnSoundRate <= 0) {
		BurnYM3526StreamCallback = YM3526StreamCallbackDummy;

		BurnYM3526Update = YM3526UpdateDummy;

		YM3526Init(1, nClockFrequency, 11025);
		return 0;
	}

	BurnYM3526StreamCallback = StreamCallback;

	if (nFMInterpolation == 3) {
		// Set YM3526 core samplerate to match the hardware
		nBurnYM3526SoundRate = nClockFrequency / 72;
		// Bring YM3526 core samplerate within usable range
		while (nBurnYM3526SoundRate > nBurnSoundRate * 3) {
			nBurnYM3526SoundRate >>= 1;
		}

		BurnYM3526Update = YM3526UpdateResample;

		nSampleSize = (unsigned int)nBurnYM3526SoundRate * (1 << 16) / nBurnSoundRate;
		nFractionalPosition = 0;
	} else {
		nBurnYM3526SoundRate = nBurnSoundRate;

		BurnYM3526Update = YM3526UpdateNormal;
	}

	YM3526Init(1, nClockFrequency, nBurnYM3526SoundRate);
	YM3526SetIRQHandler(0, IRQCallback, 0);
	YM3526SetTimerHandler(0, &BurnOPLTimerCallbackYM3526, 0);
	YM3526SetUpdateHandler(0, &BurnYM3526UpdateRequest, 0);

	pBuffer = (short*)malloc(4096 * sizeof(short));
	memset(pBuffer, 0, 4096 * sizeof(short));

	nYM3526Position = 0;

	nFractionalPosition = 0;
	
	bYM3526AddSignal = bAddSignal;

	return 0;
}

void BurnYM3526Scan(int nAction, int* pnMin)
{
	BurnTimerScanYM3526(nAction, pnMin);
	
	if (nAction & ACB_DRIVER_DATA) {
		SCAN_VAR(nYM3526Position);
	}
}
