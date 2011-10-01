#include "burnint.h"
#include "burn_sound.h"
#include "burn_ym2610.h"

void (*BurnYM2610Update)(short* pSoundBuf, int nSegmentEnd);

static int (*BurnYM2610StreamCallback)(int nSoundRate);

static int nBurnYM2610SoundRate;

static short* pBuffer;
static short* pYM2610Buffer[6];

static int* pAYBuffer;

static int nYM2610Position;
static int nAY8910Position;

static unsigned int nSampleSize;
static unsigned int nFractionalPosition;

static int bYM2610AddSignal;

// ----------------------------------------------------------------------------
// Dummy functions

static void YM2610UpdateDummy(short*, int /* nSegmentEnd */)
{
	return;
}

static int YM2610StreamCallbackDummy(int /* nSoundRate */)
{
	return 0;
}

// ----------------------------------------------------------------------------
// Execute YM2610 for part of a frame

static void AY8910Render(int nSegmentLength)
{

//	bprintf(PRINT_NORMAL, _T("    AY8910 render %6i -> %6i\n"), nAY8910Position, nSegmentLength);

	nSegmentLength -= nAY8910Position;

	pYM2610Buffer[2] = pBuffer + (2 << 12) + 4 + nAY8910Position;
	pYM2610Buffer[3] = pBuffer + (3 << 12) + 4 + nAY8910Position;
	pYM2610Buffer[4] = pBuffer + (4 << 12) + 4 + nAY8910Position;

	AY8910Update(0, &pYM2610Buffer[2], nSegmentLength);

	nAY8910Position += nSegmentLength;
}

static void YM2610Render(int nSegmentLength)
{
//	bprintf(PRINT_NORMAL, _T("    YM2610 render %6i -> %6i\n", nYM2610Position, nSegmentLength));

	nSegmentLength -= nYM2610Position;

	pYM2610Buffer[0] = pBuffer + 4 + nYM2610Position;
	pYM2610Buffer[1] = pBuffer + (1 << 12) + 4 + nYM2610Position;

	YM2610UpdateOne(0, &pYM2610Buffer[0], nSegmentLength);

	nYM2610Position += nSegmentLength;
}

// ----------------------------------------------------------------------------
// Update the sound buffer

static void YM2610UpdateResample(short* pSoundBuf, int nSegmentEnd)
{
	int nSegmentLength = nSegmentEnd;
	int nSamplesNeeded = nSegmentEnd * nBurnYM2610SoundRate / nBurnSoundRate + 1;

//	bprintf(PRINT_NORMAL, _T("    YM2610 update        -> %6i\n", nSegmentLength));

	if (nSamplesNeeded < nAY8910Position) {
		nSamplesNeeded = nAY8910Position;
	}
	if (nSamplesNeeded < nYM2610Position) {
		nSamplesNeeded = nYM2610Position;
	}

	if (nSegmentLength > nBurnSoundLen) {
		nSegmentLength = nBurnSoundLen;
	}
	nSegmentLength <<= 1;

	if (~(nYM2610Position >= nSamplesNeeded))
   {
	   YM2610Render(nSamplesNeeded);
   }
	if (~(nAY8910Position >= nSamplesNeeded))
   {
      AY8910Render(nSamplesNeeded);
   }

	pYM2610Buffer[0] = pBuffer + 4;
	pYM2610Buffer[1] = pBuffer + (1 << 12) + 4;
	pYM2610Buffer[2] = pBuffer + (2 << 12) + 4;
	pYM2610Buffer[3] = pBuffer + (3 << 12) + 4;
	pYM2610Buffer[4] = pBuffer + (4 << 12) + 4;
	pYM2610Buffer[5] = pBuffer + (5 << 12) + 4;

	for (int i = (nFractionalPosition >> 16) - 4; i < nSamplesNeeded; i++) {
		pYM2610Buffer[5][i] = (((int)pYM2610Buffer[2][i] + pYM2610Buffer[3][i] + pYM2610Buffer[4][i]) * (4096 * 60 / 100)) >> 12;
	}

	for (int i = (nFractionalPosition & 0xFFFF0000) >> 15; i < nSegmentLength; i += 2, nFractionalPosition += nSampleSize) {
      int pos = nFractionalPosition;
		int nSample;

      short sn, s0, s1, s2;

#define CLIP(A) ((A) < -0x8000 ? -0x8000 : (A) > 0x7fff ? 0x7fff : (A))

		// Left channel

		nSample = INTERPOLATE4PS_16BIT((nFractionalPosition >> 4) & 0x0FFF,
									   pYM2610Buffer[0][(pos >> 16) - 3] + pYM2610Buffer[5][(pos >> 16) - 3],
									   pYM2610Buffer[0][(pos >> 16) - 2] + pYM2610Buffer[5][(pos >> 16) - 2],
									   pYM2610Buffer[0][(pos >> 16) - 1] + pYM2610Buffer[5][(pos >> 16) - 1],
									   pYM2610Buffer[0][(pos >> 16) - 0] + pYM2610Buffer[5][(pos >> 16) - 0]);
		if (bYM2610AddSignal) {
			pSoundBuf[i + 0] += CLIP(nSample);
		} else {
			pSoundBuf[i + 0] = CLIP(nSample);
		}

		// Right channel
		nSample = INTERPOLATE4PS_16BIT((pos >> 4) & 0x0FFF,
									   pYM2610Buffer[1][(pos >> 16) - 3] + pYM2610Buffer[5][(pos >> 16) - 3],
									   pYM2610Buffer[1][(pos >> 16) - 2] + pYM2610Buffer[5][(pos >> 16) - 2],
									   pYM2610Buffer[1][(pos >> 16) - 1] + pYM2610Buffer[5][(pos >> 16) - 1],
									   pYM2610Buffer[1][(pos >> 16) - 0] + pYM2610Buffer[5][(pos >> 16) - 0]);
		if (bYM2610AddSignal) {
			pSoundBuf[i + 1] += CLIP(nSample);
		} else {
			pSoundBuf[i + 1] = CLIP(nSample);
		}

#undef CLIP

	}

	if (nSegmentEnd >= nBurnSoundLen) {
		int nExtraSamples = nSamplesNeeded - (nFractionalPosition >> 16);

//		bprintf(PRINT_NORMAL, _T("   %6i rendered, %i extra, %i <- %i\n"), nSamplesNeeded, nExtraSamples, nExtraSamples, (nFractionalPosition >> 16) + nExtraSamples - 1);

		for (int i = -4; i < nExtraSamples; i++) {
			pYM2610Buffer[0][i] = pYM2610Buffer[0][(nFractionalPosition >> 16) + i];
			pYM2610Buffer[1][i] = pYM2610Buffer[1][(nFractionalPosition >> 16) + i];
			pYM2610Buffer[2][i] = pYM2610Buffer[2][(nFractionalPosition >> 16) + i];
			pYM2610Buffer[3][i] = pYM2610Buffer[3][(nFractionalPosition >> 16) + i];
			pYM2610Buffer[4][i] = pYM2610Buffer[4][(nFractionalPosition >> 16) + i];
		}

		nFractionalPosition &= 0xFFFF;

		nYM2610Position = nExtraSamples;
		nAY8910Position = nExtraSamples;

		dTime += 100.0 / nBurnFPS;
	}
}

static void YM2610UpdateNormal(short* pSoundBuf, int nSegmentEnd)
{
	int nSegmentLength = nSegmentEnd;

//	bprintf(PRINT_NORMAL, _T("    YM2610 update        -> %6i\n", nSegmentLength));

	if (nSegmentEnd < nAY8910Position) {
		nSegmentEnd = nAY8910Position;
	}
	if (nSegmentEnd < nYM2610Position) {
		nSegmentEnd = nYM2610Position;
	}

	if (nSegmentLength > nBurnSoundLen) {
		nSegmentLength = nBurnSoundLen;
	}

	if (~(nYM2610Position >= nSegmentEnd))
   {
	   YM2610Render(nSegmentEnd);
   }
	if (~(nAY8910Position >= nSegmentEnd))
   {
	   AY8910Render(nSegmentEnd);
   }

	pYM2610Buffer[0] = pBuffer + 4;
	pYM2610Buffer[1] = pBuffer + 4 + (1 << 12);
	pYM2610Buffer[2] = pBuffer + 4 + (2 << 12);
	pYM2610Buffer[3] = pBuffer + 4 + (3 << 12);
	pYM2610Buffer[4] = pBuffer + 4 + (4 << 12);

	{
		for (int n = nFractionalPosition; n < nSegmentLength; n++) {
			int nAYSample, nTotalSample;

			nAYSample  = pYM2610Buffer[2][n];
			nAYSample += pYM2610Buffer[3][n];
			nAYSample += pYM2610Buffer[4][n];

			nAYSample  *= 4096 * 60 / 100;
			nAYSample >>= 12;

			nTotalSample = nAYSample + pYM2610Buffer[0][n];
			if (nTotalSample < -32768) {
				nTotalSample = -32768;
			} else {
				if (nTotalSample > 32767) {
					nTotalSample = 32767;
				}
			}

			if (bYM2610AddSignal) {
				pSoundBuf[(n << 1) + 0] += nTotalSample;
			} else {
				pSoundBuf[(n << 1) + 0] = nTotalSample;
			}

			nTotalSample = nAYSample + pYM2610Buffer[1][n];
			if (nTotalSample < -32768) {
				nTotalSample = -32768;
			} else {
				if (nTotalSample > 32767) {
					nTotalSample = 32767;
				}
			}

			if (bYM2610AddSignal) {
				pSoundBuf[(n << 1) + 1] += nTotalSample;
			} else {
				pSoundBuf[(n << 1) + 1] = nTotalSample;
			}
		}
	}

	nFractionalPosition = nSegmentLength;

	if (nSegmentEnd >= nBurnSoundLen) {
		int nExtraSamples = nSegmentEnd - nBurnSoundLen;

		for (int i = 0; i < nExtraSamples; i++) {
			pYM2610Buffer[0][i] = pYM2610Buffer[0][nBurnSoundLen + i];
			pYM2610Buffer[1][i] = pYM2610Buffer[1][nBurnSoundLen + i];
			pYM2610Buffer[2][i] = pYM2610Buffer[2][nBurnSoundLen + i];
			pYM2610Buffer[3][i] = pYM2610Buffer[3][nBurnSoundLen + i];
			pYM2610Buffer[4][i] = pYM2610Buffer[4][nBurnSoundLen + i];
		}

		nFractionalPosition = 0;

		nYM2610Position = nExtraSamples;
		nAY8910Position = nExtraSamples;

		dTime += 100.0 / nBurnFPS;
	}
}

// ----------------------------------------------------------------------------
// Callbacks for YM2610 core

void BurnYM2610UpdateRequest()
{
   int temp = BurnYM2610StreamCallback(nBurnYM2610SoundRate);
	if (~(nYM2610Position >= temp))
   {
	   YM2610Render(temp);
   }
}

static void BurnAY8910UpdateRequest()
{
   int temp = BurnYM2610StreamCallback(nBurnYM2610SoundRate);
	if (!(nAY8910Position >= temp))
	   AY8910Render(temp);
}

// ----------------------------------------------------------------------------
// Initialisation, etc.

void BurnYM2610Reset()
{
	BurnTimerReset();

	YM2610ResetChip(0);
}

void BurnYM2610Exit()
{
	YM2610Shutdown();
	AY8910Exit(0);

	BurnTimerExit();

	free(pBuffer);
	free(pAYBuffer);

	bYM2610AddSignal = 0;
}

int BurnYM2610Init(int nClockFrequency, unsigned char* YM2610ADPCMAROM, int* nYM2610ADPCMASize, unsigned char* YM2610ADPCMBROM, int* nYM2610ADPCMBSize, FM_IRQHANDLER IRQCallback, int (*StreamCallback)(int), double (*GetTimeCallback)(), int bAddSignal)
{
	BurnTimerInit(&YM2610TimerOver, GetTimeCallback);

	if (nBurnSoundRate <= 0) {
		BurnYM2610StreamCallback = YM2610StreamCallbackDummy;

		BurnYM2610Update = YM2610UpdateDummy;

		AY8910InitYM(0, nClockFrequency, 11025, NULL, NULL, NULL, NULL, BurnAY8910UpdateRequest);
		YM2610Init(1, nClockFrequency, 11025, (void**)(&YM2610ADPCMAROM), nYM2610ADPCMASize, (void**)(&YM2610ADPCMBROM), nYM2610ADPCMBSize, &BurnOPNTimerCallback, IRQCallback);
		return 0;
	}

	BurnYM2610StreamCallback = StreamCallback;

	if (nFMInterpolation == 3) {
		// Set YM2610 core samplerate to match the hardware
		nBurnYM2610SoundRate = nClockFrequency / 144;
		// Bring YM2610 core samplerate within usable range
		while (nBurnYM2610SoundRate > nBurnSoundRate * 3) {
			nBurnYM2610SoundRate >>= 1;
		}

		BurnYM2610Update = YM2610UpdateResample;

		nSampleSize = (unsigned int)nBurnYM2610SoundRate * (1 << 16) / nBurnSoundRate;
	} else {
		nBurnYM2610SoundRate = nBurnSoundRate;

		BurnYM2610Update = YM2610UpdateNormal;
	}

	AY8910InitYM(0, nClockFrequency, nBurnYM2610SoundRate, NULL, NULL, NULL, NULL, BurnAY8910UpdateRequest);
	YM2610Init(1, nClockFrequency, nBurnYM2610SoundRate, (void**)(&YM2610ADPCMAROM), nYM2610ADPCMASize, (void**)(&YM2610ADPCMBROM), nYM2610ADPCMBSize, &BurnOPNTimerCallback, IRQCallback);

	pBuffer = (short*)malloc(4096 * 6 * sizeof(short));
	memset(pBuffer, 0, 4096 * 6 * sizeof(short));

	pAYBuffer = (int*)malloc(4096 * sizeof(int));
	memset(pAYBuffer, 0, 4096 * sizeof(int));

	nYM2610Position = 0;
	nAY8910Position = 0;

	nFractionalPosition = 0;
	bYM2610AddSignal = bAddSignal;

	return 0;
}

void BurnYM2610Scan(int nAction, int* pnMin)
{
	BurnTimerScan(nAction, pnMin);
	AY8910Scan(nAction, pnMin);

	if (nAction & ACB_DRIVER_DATA) {
		SCAN_VAR(nYM2610Position);
		SCAN_VAR(nAY8910Position);
	}
}
