#include "burnint.h"
#include "burn_sound.h"
#include "burn_ym2151.h"

void (*BurnYM2151Render)(short* pSoundBuf, int nSegmentLength);

unsigned char BurnYM2151Registers[0x0100];
unsigned int nBurnCurrentYM2151Register;

static int nBurnYM2151SoundRate;

static short* pBuffer;
static short* pYM2151Buffer[2];

static int nBurnPosition;
static unsigned int nSampleSize;
static unsigned int nFractionalPosition;
static unsigned int nSamplesRendered;

static int nYM2151Volume;

static void YM2151RenderResample(short* pSoundBuf, int nSegmentLength)
{
	nBurnPosition += nSegmentLength;

	if (nBurnPosition >= nBurnSoundRate) {
		nBurnPosition = nSegmentLength;

		pYM2151Buffer[0][1] = pYM2151Buffer[0][(nFractionalPosition >> 16) - 3];
		pYM2151Buffer[0][2] = pYM2151Buffer[0][(nFractionalPosition >> 16) - 2];
		pYM2151Buffer[0][3] = pYM2151Buffer[0][(nFractionalPosition >> 16) - 1];

		pYM2151Buffer[1][1] = pYM2151Buffer[1][(nFractionalPosition >> 16) - 3];
		pYM2151Buffer[1][2] = pYM2151Buffer[1][(nFractionalPosition >> 16) - 2];
		pYM2151Buffer[1][3] = pYM2151Buffer[1][(nFractionalPosition >> 16) - 1];

		nSamplesRendered -= (nFractionalPosition >> 16) - 4;

		for (unsigned int i = 0; i <= nSamplesRendered; i++) {
			pYM2151Buffer[0][4 + i] = pYM2151Buffer[0][(nFractionalPosition >> 16) + i];
			pYM2151Buffer[1][4 + i] = pYM2151Buffer[1][(nFractionalPosition >> 16) + i];
		}

		nFractionalPosition &= 0x0000FFFF;
		nFractionalPosition |= 4 << 16;
	}

	pYM2151Buffer[0] = pBuffer + 4 + nSamplesRendered;
	pYM2151Buffer[1] = pBuffer + 4 + nSamplesRendered + 65536;

	YM2151UpdateOne(0, pYM2151Buffer, (unsigned int)(nBurnPosition + 1) * nBurnYM2151SoundRate / nBurnSoundRate - nSamplesRendered);
	nSamplesRendered += (unsigned int)(nBurnPosition + 1) * nBurnYM2151SoundRate / nBurnSoundRate - nSamplesRendered;

	pYM2151Buffer[0] = pBuffer;
	pYM2151Buffer[1] = pBuffer + 65536;

	nSegmentLength <<= 1;

	for (int i = 0; i < nSegmentLength; i += 2, nFractionalPosition += nSampleSize) {

		// Left channel
		pSoundBuf[i + 0] = INTERPOLATE4PS_CUSTOM((nFractionalPosition >> 4) & 0x0FFF,
												 pYM2151Buffer[0][(nFractionalPosition >> 16) - 3],
												 pYM2151Buffer[0][(nFractionalPosition >> 16) - 2],
												 pYM2151Buffer[0][(nFractionalPosition >> 16) - 1],
												 pYM2151Buffer[0][(nFractionalPosition >> 16) - 0],
												 nYM2151Volume);

		// Right channel
		pSoundBuf[i + 1] = INTERPOLATE4PS_CUSTOM((nFractionalPosition >> 4) & 0x0FFF,
												 pYM2151Buffer[1][(nFractionalPosition >> 16) - 3],
												 pYM2151Buffer[1][(nFractionalPosition >> 16) - 2],
												 pYM2151Buffer[1][(nFractionalPosition >> 16) - 1],
												 pYM2151Buffer[1][(nFractionalPosition >> 16) - 0],
												 nYM2151Volume);
	}
}

static void YM2151RenderNormal(short* pSoundBuf, int nSegmentLength)
{
	nBurnPosition += nSegmentLength;

	pYM2151Buffer[0] = pBuffer;
	pYM2151Buffer[1] = pBuffer + nSegmentLength;

	YM2151UpdateOne(0, pYM2151Buffer, nSegmentLength);

	if (bBurnUseMMX) {
#if defined BUILD_X86_ASM
		BurnSoundCopy_FM_A(pYM2151Buffer[0], pYM2151Buffer[1], pSoundBuf, nSegmentLength, nYM2151Volume, nYM2151Volume);
#endif
	} else {
		for (int n = 0; n < nSegmentLength; n++) {
			for (int i = 0; i < 2; i++) {
				int nSample = pYM2151Buffer[i][n] * (nYM2151Volume >> 10);
				nSample >>= 8;
				
				if (nSample < -32768) nSample = -32768;
				if (nSample > 32767) nSample = 32767;
			
				pSoundBuf[(n << 1) + i] = nSample;
			}
		}
	}
}

void BurnYM2151Reset()
{
	YM2151ResetChip(0);
}

void BurnYM2151Exit()
{
	YM2151Shutdown();

	free(pBuffer);
}

int BurnYM2151Init(int nClockFrequency, float nVolume)
{
	if (nBurnSoundRate <= 0) {
		YM2151Init(1, nClockFrequency, 11025);
		return 0;
	}

	if (nFMInterpolation == 3) {
		// Set YM2151 core samplerate to match the hardware
		nBurnYM2151SoundRate = nClockFrequency >> 6;
		// Bring YM2151 core samplerate within usable range
		while (nBurnYM2151SoundRate > nBurnSoundRate * 3) {
			nBurnYM2151SoundRate >>= 1;
		}

		BurnYM2151Render = YM2151RenderResample;

		nYM2151Volume = (int)((double)16384.0 * 100.0 / nVolume);
	} else {
		nBurnYM2151SoundRate = nBurnSoundRate;
		BurnYM2151Render = YM2151RenderNormal;

		nYM2151Volume = (int)((double)65536.0 * 100.0 / nVolume);
	}

	YM2151Init(1, nClockFrequency, nBurnYM2151SoundRate);

	pBuffer = (short*)malloc(65536 * 2 * sizeof(short));
	memset(pBuffer, 0, 65536 * 2 * sizeof(short));

	nSampleSize = (unsigned int)nBurnYM2151SoundRate * (1 << 16) / nBurnSoundRate;
	nFractionalPosition = 4 << 16;
	nSamplesRendered = 0;
	nBurnPosition = 0;

	return 0;
}

void BurnYM2151Scan(int nAction)
{
	if ((nAction & ACB_DRIVER_DATA) == 0) {
		return;
	}
	SCAN_VAR(nBurnCurrentYM2151Register);
	SCAN_VAR(BurnYM2151Registers);

	if (nAction & ACB_WRITE) {
		for (int i = 0; i < 0x0100; i++) {
			YM2151WriteReg(0, i, BurnYM2151Registers[i]);
		}
	}
}
