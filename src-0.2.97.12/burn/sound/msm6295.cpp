#include <math.h>
#include "burnint.h"
#include "msm6295.h"
#include "burn_sound.h"

unsigned char* MSM6295ROM;
unsigned char* MSM6295SampleInfo[MAX_MSM6295][4];
unsigned char* MSM6295SampleData[MAX_MSM6295][4];

unsigned int nMSM6295Status[MAX_MSM6295];

struct MSM6295ChannelInfo {
	int nOutput;
	int nVolume;
	int nPosition;
	int nSampleCount;
	int nSample;
	int nStep;
	int nDelta;

	int nBufPos;
};

static struct {
	int nVolume;
	int nSampleRate;
	int nSampleSize;
	int nFractionalPosition;

	// All current settings for each channel
	MSM6295ChannelInfo ChannelInfo[4];

	// Used for sending commands
	bool bIsCommand;
	int nSampleInfo;

} MSM6295[MAX_MSM6295];

static unsigned int MSM6295VolumeTable[16];
static int MSM6295DeltaTable[49 * 16];
static int MSM6295StepShift[8] = {-1, -1, -1, -1, 2, 4, 6, 8};

static int* MSM6295ChannelData[MAX_MSM6295][4];

static int* pBuffer = NULL;
static int nLastChip;

static bool bAdd;

void MSM6295Reset(int nChip)
{
	nMSM6295Status[nChip] = 0;
	MSM6295[nChip].bIsCommand = false;

	MSM6295[nChip].nFractionalPosition = 0;

	for (int nChannel = 0; nChannel < 4; nChannel++) {
		// Set initial bank information
		MSM6295SampleInfo[nChip][nChannel] = MSM6295ROM + (nChip * 0x0100000) + (nChannel << 8);
		MSM6295SampleData[nChip][nChannel] = MSM6295ROM + (nChip * 0x0100000) + (nChannel << 16);

		memset(MSM6295ChannelData[nChip][nChannel], 0, 0x1000 * sizeof(int));
		MSM6295[nChip].ChannelInfo[nChannel].nBufPos = 4;
	}
}

int MSM6295Scan(int nChip, int /*nAction*/)
{
	int nSampleSize = MSM6295[nChip].nSampleSize;
	SCAN_VAR(MSM6295[nChip]);
	MSM6295[nChip].nSampleSize = nSampleSize;

	SCAN_VAR(nMSM6295Status[nChip]);

	for (int i = 0; i < 4; i++) {
		MSM6295SampleInfo[nChip][i] -= (unsigned int)MSM6295ROM;
		SCAN_VAR(MSM6295SampleInfo[nChip][i]);
		MSM6295SampleInfo[nChip][i] += (unsigned int)MSM6295ROM;

		MSM6295SampleData[nChip][i] -= (unsigned int)MSM6295ROM;
		SCAN_VAR(MSM6295SampleData[nChip][i]);
		MSM6295SampleData[nChip][i] += (unsigned int)MSM6295ROM;
	}

	return 0;
}

static void MSM6295Render_Linear(int nChip, int* pBuf, int nSegmentLength)
{
	static int nPreviousSample[MAX_MSM6295], nCurrentSample[MAX_MSM6295];
	int nVolume = MSM6295[nChip].nVolume;
	int nFractionalPosition = MSM6295[nChip].nFractionalPosition;

	int nChannel, nDelta, nSample;
	MSM6295ChannelInfo* pChannelInfo;

	while (nSegmentLength--) {
		if (nFractionalPosition >= 0x1000) {

			nPreviousSample[nChip] = nCurrentSample[nChip];

			do {
				nCurrentSample[nChip] = 0;

				for (nChannel = 0; nChannel < 4; nChannel++) {
					if (nMSM6295Status[nChip] & (1 << nChannel)) {
						pChannelInfo = &MSM6295[nChip].ChannelInfo[nChannel];

						// Check for end of sample
						if (pChannelInfo->nSampleCount-- == 0) {
							nMSM6295Status[nChip] &= ~(1 << nChannel);
							continue;
						}

						// Get new delta from ROM
						if (pChannelInfo->nPosition & 1) {
							nDelta = pChannelInfo->nDelta & 0x0F;
						} else {
							pChannelInfo->nDelta = MSM6295SampleData[nChip][(pChannelInfo->nPosition >> 17) & 3][(pChannelInfo->nPosition >> 1) & 0xFFFF];
							nDelta = pChannelInfo->nDelta >> 4;
						}

						// Compute new sample
						nSample = pChannelInfo->nSample + MSM6295DeltaTable[(pChannelInfo->nStep << 4) + nDelta];
						if (nSample > 2047) {
							nSample = 2047;
						} else {
							if (nSample < -2048) {
								nSample = -2048;
							}
						}
						pChannelInfo->nSample = nSample;
						pChannelInfo->nOutput = (nSample * pChannelInfo->nVolume);

						// Update step value
						pChannelInfo->nStep = pChannelInfo->nStep + MSM6295StepShift[nDelta & 7];
						if (pChannelInfo->nStep > 48) {
							pChannelInfo->nStep = 48;
						} else {
							if (pChannelInfo->nStep < 0) {
								pChannelInfo->nStep = 0;
							}
						}

						nCurrentSample[nChip] += pChannelInfo->nOutput / 16;

						// Advance sample position
						pChannelInfo->nPosition++;
					}
				}

				nFractionalPosition -= 0x1000;

			} while (nFractionalPosition >= 0x1000);
		}

		// Compute linearly interpolated sample
		nSample = nPreviousSample[nChip] + (((nCurrentSample[nChip] - nPreviousSample[nChip]) * nFractionalPosition) >> 12);

		// Scale all 4 channels
		nSample *= nVolume;

		*pBuf++ += nSample;

		nFractionalPosition += MSM6295[nChip].nSampleSize;
	}

	MSM6295[nChip].nFractionalPosition = nFractionalPosition;
}

static void MSM6295Render_Cubic(int nChip, int* pBuf, int nSegmentLength)
{
	int nVolume = MSM6295[nChip].nVolume;
	int nFractionalPosition;

	int nChannel, nDelta, nSample, nOutput;
	MSM6295ChannelInfo* pChannelInfo;

	while (nSegmentLength--) {

		nOutput = 0;

		for (nChannel = 0; nChannel < 4; nChannel++) {
			pChannelInfo = &MSM6295[nChip].ChannelInfo[nChannel];
			nFractionalPosition = MSM6295[nChip].nFractionalPosition;

			if (nMSM6295Status[nChip] & (1 << nChannel)) {

				while (nFractionalPosition >= 0x1000) {

					// Check for end of sample
					if (pChannelInfo->nSampleCount-- <= 0) {
						if (pChannelInfo->nSampleCount <= -2) {
							nMSM6295Status[nChip] &= ~(1 << nChannel);
						}

						MSM6295ChannelData[nChip][nChannel][pChannelInfo->nBufPos++] = pChannelInfo->nOutput / 16;

						break;

					} else {
						// Get new delta from ROM
						if (pChannelInfo->nPosition & 1) {
							nDelta = pChannelInfo->nDelta & 0x0F;
						} else {
							pChannelInfo->nDelta = MSM6295SampleData[nChip][pChannelInfo->nPosition >> 17][(pChannelInfo->nPosition >> 1) & 0xFFFF];
							nDelta = pChannelInfo->nDelta >> 4;
						}

						// Compute new sample
						nSample = pChannelInfo->nSample + MSM6295DeltaTable[(pChannelInfo->nStep << 4) + nDelta];
						if (nSample > 2047) {
							nSample = 2047;
						} else {
							if (nSample < -2048) {
								nSample = -2048;
							}
						}
						pChannelInfo->nSample = nSample;
						pChannelInfo->nOutput = nSample * pChannelInfo->nVolume;

						// Update step value
						pChannelInfo->nStep = pChannelInfo->nStep + MSM6295StepShift[nDelta & 7];
						if (pChannelInfo->nStep > 48) {
							pChannelInfo->nStep = 48;
						} else {
							if (pChannelInfo->nStep < 0) {
								pChannelInfo->nStep = 0;
							}
						}

						// The interpolator needs a 16-bit sample, pChannelInfo->nOutput is now a 20-bit number
						MSM6295ChannelData[nChip][nChannel][pChannelInfo->nBufPos++] = pChannelInfo->nOutput / 16;

						// Advance sample position
						pChannelInfo->nPosition++;
						nFractionalPosition -= 0x1000;
					}
				}

				if (pChannelInfo->nBufPos > 0x0FF0) {
					MSM6295ChannelData[nChip][nChannel][0] = MSM6295ChannelData[nChip][nChannel][pChannelInfo->nBufPos - 4];
					MSM6295ChannelData[nChip][nChannel][1] = MSM6295ChannelData[nChip][nChannel][pChannelInfo->nBufPos - 3];
					MSM6295ChannelData[nChip][nChannel][2] = MSM6295ChannelData[nChip][nChannel][pChannelInfo->nBufPos - 2];
					MSM6295ChannelData[nChip][nChannel][3] = MSM6295ChannelData[nChip][nChannel][pChannelInfo->nBufPos - 1];
					pChannelInfo->nBufPos = 4;
				}

				nOutput += INTERPOLATE4PS_16BIT(nFractionalPosition,
												MSM6295ChannelData[nChip][nChannel][pChannelInfo->nBufPos - 4],
												MSM6295ChannelData[nChip][nChannel][pChannelInfo->nBufPos - 3],
												MSM6295ChannelData[nChip][nChannel][pChannelInfo->nBufPos - 2],
												MSM6295ChannelData[nChip][nChannel][pChannelInfo->nBufPos - 1]);
			} else {
				// Ramp channel output to 0
				if (pChannelInfo->nOutput != 0) {
					int nRamp = 2048 * 256 * 256 / nBurnSoundRate;
					if (pChannelInfo->nOutput > 0) {
						if (pChannelInfo->nOutput > nRamp) {
							pChannelInfo->nOutput -= nRamp;
						} else {
							pChannelInfo->nOutput = 0;
						}
					} else {
						if (pChannelInfo->nOutput < -nRamp) {
							pChannelInfo->nOutput += nRamp;
						} else {
							pChannelInfo->nOutput = 0;
						}
					}
					nOutput += pChannelInfo->nOutput / 16;
				}
			}
		}

		nOutput *= nVolume;

		*pBuf++ += nOutput;

		MSM6295[nChip].nFractionalPosition = (MSM6295[nChip].nFractionalPosition & 0x0FFF) + MSM6295[nChip].nSampleSize;
	}
}

int MSM6295Render(int nChip, short* pSoundBuf, int nSegmentLength)
{
	if (nChip == 0) {
		memset(pBuffer, 0, nSegmentLength * sizeof(int));
	}

	if (nInterpolation >= 3) {
		MSM6295Render_Cubic(nChip, pBuffer, nSegmentLength);
	} else {
		MSM6295Render_Linear(nChip, pBuffer, nSegmentLength);
	}

	if (nChip == nLastChip)	{
		if (bBurnUseMMX) {
#if defined BUILD_X86_ASM
			if (bAdd) {
				BurnSoundCopyClamp_Mono_Add_A(pBuffer, pSoundBuf, nSegmentLength);
			} else {
				BurnSoundCopyClamp_Mono_A(pBuffer, pSoundBuf, nSegmentLength);
			}
#endif
		} else {
			if (bAdd) {
				BurnSoundCopyClamp_Mono_Add_C(pBuffer, pSoundBuf, nSegmentLength);
			} else {
				BurnSoundCopyClamp_Mono_C(pBuffer, pSoundBuf, nSegmentLength);
			}
		}
	}

	return 0;
}

void MSM6295Command(int nChip, unsigned char nCommand)
{
	if (MSM6295[nChip].bIsCommand) {
		// Process second half of command
		int nChannel, nSampleStart, nSampleCount;
		int nVolume = nCommand & 0x0F;
		nCommand >>= 4;

		MSM6295[nChip].bIsCommand = false;

		for (nChannel = 0; nChannel < 4; nChannel++) {
			if (nCommand & (0x01 << nChannel)) {
				int nBank = (MSM6295[nChip].nSampleInfo & 0x0300) >> 8;
				MSM6295[nChip].nSampleInfo &= 0xFF;

				nSampleStart = MSM6295SampleInfo[nChip][nBank][MSM6295[nChip].nSampleInfo + 0];
				nSampleStart <<= 8;
				nSampleStart |= MSM6295SampleInfo[nChip][nBank][MSM6295[nChip].nSampleInfo + 1];
				nSampleStart <<= 8;
				nSampleStart |= MSM6295SampleInfo[nChip][nBank][MSM6295[nChip].nSampleInfo + 2];
				nSampleStart <<= 1;

				nSampleCount = MSM6295SampleInfo[nChip][nBank][MSM6295[nChip].nSampleInfo + 3];
				nSampleCount <<= 8;
				nSampleCount |= MSM6295SampleInfo[nChip][nBank][MSM6295[nChip].nSampleInfo + 4];
				nSampleCount <<= 8;
				nSampleCount |= MSM6295SampleInfo[nChip][nBank][MSM6295[nChip].nSampleInfo + 5];
				nSampleCount <<= 1;

				if (nSampleCount < 0x80000) {
					nSampleCount -= nSampleStart;

					// Start playing channel
					MSM6295[nChip].ChannelInfo[nChannel].nVolume = MSM6295VolumeTable[nVolume];
					MSM6295[nChip].ChannelInfo[nChannel].nPosition = nSampleStart;
					MSM6295[nChip].ChannelInfo[nChannel].nSampleCount = nSampleCount;
					MSM6295[nChip].ChannelInfo[nChannel].nStep = 0;
					MSM6295[nChip].ChannelInfo[nChannel].nSample = -1;

					MSM6295[nChip].ChannelInfo[nChannel].nOutput = 0;

					nMSM6295Status[nChip] |= nCommand;

					if (nInterpolation >= 3) {
						MSM6295ChannelData[nChip][nChannel][0] = 0;
						MSM6295ChannelData[nChip][nChannel][1] = 0;
						MSM6295ChannelData[nChip][nChannel][2] = 0;
						MSM6295ChannelData[nChip][nChannel][3] = 0;
						MSM6295[nChip].ChannelInfo[nChannel].nBufPos = 4;
					}
				}
			}
		}

	} else {
		// Process command
		if (nCommand & 0x80) {
			MSM6295[nChip].nSampleInfo = (nCommand & 0x7F) << 3;
			MSM6295[nChip].bIsCommand = true;
		} else {
			// Stop playing samples
			nCommand >>= 3;
			nMSM6295Status[nChip] &= ~nCommand;
		}
	}
}

void MSM6295Exit(int nChip)
{
	free(pBuffer);
	pBuffer = NULL;

	for (int nChannel = 0; nChannel < 4; nChannel++) {
		free(MSM6295ChannelData[nChip][nChannel]);
		MSM6295ChannelData[nChip][nChannel] = NULL;
	}
}

int MSM6295Init(int nChip, int nSamplerate, float fMaxVolume, bool bAddSignal)
{
	if (nBurnSoundRate > 0) {
		if (pBuffer == NULL) {
			pBuffer = (int*)malloc(nBurnSoundRate * sizeof(int));
		}
	}

	bAdd = bAddSignal;

	// Convert volume from percentage
	MSM6295[nChip].nVolume = int(fMaxVolume * 256.0 / 100.0 + 0.5);

	MSM6295[nChip].nSampleRate = nSamplerate;
	if (nBurnSoundRate > 0) {
		MSM6295[nChip].nSampleSize = (nSamplerate << 12) / nBurnSoundRate;
	} else {
		MSM6295[nChip].nSampleSize = (nSamplerate << 12) / 11025;
	}

	MSM6295[nChip].nFractionalPosition = 0;

	nMSM6295Status[nChip] = 0;
	MSM6295[nChip].bIsCommand = false;

	if (nChip == 0) {
		nLastChip = 0;
	} else {
		if (nLastChip < nChip) {
			nLastChip = nChip;
		}
	}

	// Compute sample deltas
	for (int i = 0; i < 49; i++) {
		int nStep = (int)(pow(1.1, (double)i) * 16.0);
		for (int n = 0; n < 16; n++) {
			int nDelta = nStep >> 3;
			if (n & 1) {
				nDelta += nStep >> 2;
			}
			if (n & 2) {
				nDelta += nStep >> 1;
			}
			if (n & 4) {
				nDelta += nStep;
			}
			if (n & 8) {
				nDelta = -nDelta;
			}
			MSM6295DeltaTable[(i << 4) + n] = nDelta;
		}
	}

	// Compute volume levels
	for (int i = 0; i < 16; i++) {
		double nVolume = 256.0;
		for (int n = i; n > 0; n--) {
			nVolume /= 1.412537545;
		}
		MSM6295VolumeTable[i] = (unsigned int)(nVolume + 0.5);
	}

	for (int nChannel = 0; nChannel < 4; nChannel++) {
		MSM6295ChannelData[nChip][nChannel] = (int*)malloc(0x1000 * sizeof(int));
	}

	MSM6295Reset(nChip);

	return 0;
}

