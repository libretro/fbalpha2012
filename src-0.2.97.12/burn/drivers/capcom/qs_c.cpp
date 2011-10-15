// QSound - emulator for the QSound Chip

#include <math.h>
#include "cps.h"
#include "burn_sound.h"

static const int nQscClock = 4000000;
static const int nQscClockDivider = 166;

static int nQscRate = 0;
static int nQscVolumeShift;

static int Tams = -1;
static int* Qs_s = NULL;

static int nPos;

struct QChan {
		unsigned char bKey;				// 1 if channel is playing
		char nBank;						// Bank we are currently playing a sample from

		char* PlayBank;					// Pointer to current bank

		int nPlayStart;					// Start of being played
		int nStart;						// Start of sample 16.12
		int nEnd;						// End of sample   16.12
		int nLoop;						// Loop offset from end
		int nPos;						// Current position within the bank 16.12
		int nAdvance;					// Sample size

		int nMasterVolume;				// Master volume for the channel
		int nVolume[2];					// Left & right side volumes (panning)

		int nPitch;						// Playback frequency

		char nEndBuffer[8];				// Buffer to enable correct cubic interpolation
};

static struct QChan QChan[16];

static int PanningVolumes[33];

static void MapBank(struct QChan* pc)
{
	unsigned int nBank;

	nBank = (pc->nBank & 0x7F) << 16;	// Banks are 0x10000 samples long

	// Confirm whole bank is in range:
	// If bank is out of range use bank 0 instead
	if ((nBank + 0x10000) > nCpsQSamLen) {
		nBank = 0;
	}
	pc->PlayBank = (char*)CpsQSam + nBank;
}

static void UpdateEndBuffer(struct QChan* pc)
{
	if (pc->bKey) {
		// prepare a buffer to correctly interpolate the last 4 samples
		if (nInterpolation >= 3) {
			for (int i = 0; i < 4; i++) {
				pc->nEndBuffer[i] = pc->PlayBank[(pc->nEnd >> 12) - 4 + i];
			}

			if (pc->nLoop) {
				for (int i = 0, j = 0; i < 4; i++, j++) {
					if (j >= (pc->nLoop >> 12)) {
						j = 0;
					}
					pc->nEndBuffer[i + 4] = pc->PlayBank[((pc->nEnd - pc->nLoop) >> 12) + j];
				}
			} else {
				for (int i = 0; i < 4; i++) {
					pc->nEndBuffer[i + 4] = pc->nEndBuffer[3];
				}
			}
		}
	}
}

static void CalcAdvance(struct QChan* pc)
{
	if (nQscRate) {
		pc->nAdvance = (long long)pc->nPitch * nQscClock / nQscClockDivider / nQscRate;
	}
}

void QscReset()
{
	memset(QChan, 0, sizeof(QChan));

	// Point all to bank 0
	for (int i = 0; i < 16; i++) {
		QChan[i].PlayBank = (char*)CpsQSam;
	}
}

void QscExit()
{
	nQscRate = 0;

	free(Qs_s);
	Qs_s = NULL;
	Tams = -1;
}

int QscInit(int nRate, int nVolumeShift)
{
	nQscRate = nRate;

	nQscVolumeShift = 10 + nVolumeShift;

	for (int i = 0; i < 33; i++) {
		PanningVolumes[i] = (int)((256.0 / sqrt(32.0)) * sqrt((double)i));
	}

	QscReset();

	return 0;
}

int QscScan(int nAction)
{
	SCAN_VAR(QChan);

	if (nAction & ACB_WRITE) {
		// Update bank pointers with new banks, and recalc nAdvance
		for (int i = 0; i < 16; i++) {
			MapBank(QChan + i);
			CalcAdvance(QChan + i);
		}
	}

	return 0;
}

void QscNewFrame()
{
	nPos = 0;
}

static inline void QscSyncQsnd()
{
	if (pBurnSoundOut) QscUpdate(ZetTotalCycles() * nBurnSoundLen / nCpsZ80Cycles);
}

void QscWrite(int a, int d)
{
	struct QChan* pc;
	int nChanNum, r;

	// unknown
	if (a >= 0x90) {
//		bprintf(PRINT_NORMAL, "QSound: reg 0x%02X -> 0x%02X.\n", a, d);
		return;
	}

	QscSyncQsnd();

	if (a >= 0x80) {									// Set panning for channel
		int nPan;

		nChanNum = a & 15;

		pc = QChan + nChanNum;		// Find channel
		nPan = (d - 0x10) & 0x3F;	// nPan = 0x00 to 0x20 now
		if (nPan > 0x20) {
			nPan = 0x20;
		}

//		bprintf(PRINT_NORMAL, "QSound: ch#%i pan -> 0x%04X\n", nChanNum, d);

		pc->nVolume[0] = PanningVolumes[0x20 - nPan];
		pc->nVolume[1] = PanningVolumes[0x00 + nPan];

		return;
	}

	// Get channel and register number
	nChanNum = (a >> 3) & 15;
	r = a & 7;

	// Pointer to channel info
	pc = QChan + nChanNum;

	switch (r) {
		case 0: {										// Set bank
			// Strange but true
			pc = QChan + ((nChanNum + 1) & 15);
			pc->nBank = d;
			MapBank(pc);
			UpdateEndBuffer(pc);
			break;
		}
		case 1: {										// Set sample start offset
			pc->nStart = d << 12;
			break;
		}
		case 2: {
			pc->nPitch = d;
			CalcAdvance(pc);

			if (d == 0) {								// Key off; stop playing
				pc->bKey = 0;
			}

			break;
		}
#if 0
		case 3: {
			break;
		}
#endif
		case 4: {										// Set sample loop offset
			pc->nLoop = d << 12;
			UpdateEndBuffer(pc);
			break;
		}
		case 5: {										// Set sample end offset
			pc->nEnd = d << 12;
			UpdateEndBuffer(pc);
			break;
		}
		case 6: {										// Set volume
			pc->nMasterVolume = d;

			if (d == 0) {
				pc->bKey = 0;
			} else {
				if (pc->bKey == 0) {					// Key on; play sample
					pc->nPlayStart = pc->nStart;

					pc->nPos = 0;
					pc->bKey = 3;
					UpdateEndBuffer(pc);
				}
			}
			break;
		}
#if 0
		case 7: {
			break;
		}
#endif

	}
}

int QscUpdate(int nEnd)
{
	int nLen;

	if (nEnd > nBurnSoundLen) {
		nEnd = nBurnSoundLen;
	}

	nLen = nEnd - nPos;

	if (nLen <= 0) {
		return 0;
	}

	if (Tams < nLen) {
		if (Qs_s) {
			free(Qs_s);
		}
		Tams = nLen;
		Qs_s = (int*)malloc(sizeof(int) * 2 * Tams);
	}

	memset(Qs_s, 0, nLen * 2 * sizeof(int));

	if (nInterpolation < 3) {

		// Go through all channels
		for (int c = 0; c < 16; c++) {

			// If the channel is playing, add the samples to the buffer
			if (QChan[c].bKey) {
				int VolL = (QChan[c].nMasterVolume * QChan[c].nVolume[0]) >> nQscVolumeShift;
				int VolR = (QChan[c].nMasterVolume * QChan[c].nVolume[1]) >> nQscVolumeShift;
				int* pTemp = Qs_s;
				int i = nLen;
				int s, p;

				if (QChan[c].bKey & 2) {
					QChan[c].bKey &= ~2;
					QChan[c].nPos = QChan[c].nPlayStart;
				}

				while (i--) {

					p = (QChan[c].nPos >> 12) & 0xFFFF;

					// Check for end of sample
					if (QChan[c].nPos >= (QChan[c].nEnd - 0x01000)) {
						if (QChan[c].nLoop) {						// Loop sample
							if (QChan[c].nPos < QChan[c].nEnd) {
								QChan[c].nEndBuffer[0] = QChan[c].PlayBank[(QChan[c].nEnd - QChan[c].nLoop) >> 12];
							} else {
								QChan[c].nPos = QChan[c].nEnd - QChan[c].nLoop + (QChan[c].nPos & 0x0FFF);
								p = (QChan[c].nPos >> 12) & 0xFFFF;
							}
						} else {
							if (QChan[c].nPos < QChan[c].nEnd) {
								QChan[c].nEndBuffer[0] = QChan[c].PlayBank[p];
							} else {
								QChan[c].bKey = 0;					// Quit playing
								break;
							}
						}
					} else {
						QChan[c].nEndBuffer[0] = QChan[c].PlayBank[p + 1];
					}

					// Interpolate sample
					s = QChan[c].PlayBank[p] * (1 << 6) + ((QChan[c].nPos) & ((1 << 12) - 1)) * (QChan[c].nEndBuffer[0] - QChan[c].PlayBank[p]) / (1 << 6);

					// Add to the sound currently in the buffer
					pTemp[0] += s * VolL;
					pTemp[1] += s * VolR;

					pTemp += 2;

					QChan[c].nPos += QChan[c].nAdvance;				// increment sample position based on pitch
				}
			}
		}

		if (bBurnUseMMX) {
#if defined BUILD_X86_ASM
			BurnSoundCopyClamp_A(Qs_s, pBurnSoundOut + (nPos << 1), nLen);
#endif
		} else {
			BurnSoundCopyClamp_C(Qs_s, pBurnSoundOut + (nPos << 1), nLen);
		}
		nPos = nEnd;

		return 0;
	}

	// Go through all channels
	for (int c = 0; c < 16; c++) {

		// If the channel is playing, add the samples to the buffer
		if (QChan[c].bKey) {
			int VolL = (QChan[c].nMasterVolume * QChan[c].nVolume[0]) >> nQscVolumeShift;
			int VolR = (QChan[c].nMasterVolume * QChan[c].nVolume[1]) >> nQscVolumeShift;
			int* pTemp = Qs_s;
			int i = nLen;

			// handle 1st sample
			if (QChan[c].bKey & 2) {
				while (QChan[c].nPos < 0x1000 && i) {
					int p = QChan[c].nPlayStart >> 12;
					int s = INTERPOLATE4PS_CUSTOM(QChan[c].nPos,
												  0,
												  QChan[c].PlayBank[p + 0],
												  QChan[c].PlayBank[p + 1],
												  QChan[c].PlayBank[p + 2],
												  256);

					pTemp[0] += s * VolL;
					pTemp[1] += s * VolR;

					QChan[c].nPos += QChan[c].nAdvance;				// increment sample position based on pitch

					pTemp += 2;
					i--;
				}
				if (i > 0) {
					QChan[c].bKey &= ~2;
					QChan[c].nPos = (QChan[c].nPos & 0x0FFF) + QChan[c].nPlayStart;
				}
			}

			if (bBurnUseMMX && i > 0) {
#if defined BUILD_X86_ASM
				QChan[c].bKey = (unsigned char)ChannelMix_QS_A(pTemp, i,
															   QChan[c].PlayBank,
															   QChan[c].nEnd,
															   &(QChan[c].nPos),
															   VolL,
															   VolR,
															   QChan[c].nLoop,
															   QChan[c].nAdvance,
															   QChan[c].nEndBuffer);
#endif
			} else {
				while (i > 0) {
					int s, p;

					// Check for end of sample
					if (QChan[c].nPos >= (QChan[c].nEnd - 0x3000)) {
						if (QChan[c].nPos < QChan[c].nEnd) {
							int nIndex = 4 - ((QChan[c].nEnd - QChan[c].nPos) >> 12);
							s = INTERPOLATE4PS_CUSTOM((QChan[c].nPos) & ((1 << 12) - 1),
													  QChan[c].nEndBuffer[nIndex + 0],
													  QChan[c].nEndBuffer[nIndex + 1],
													  QChan[c].nEndBuffer[nIndex + 2],
													  QChan[c].nEndBuffer[nIndex + 3],
													  256);
						} else {
							if (QChan[c].nLoop) {					// Loop sample
								if (QChan[c].nLoop <= 0x1000) {		// Don't play, but leave bKey on
									QChan[c].nPos = QChan[c].nEnd - 0x1000;
									break;
								}
								QChan[c].nPos -= QChan[c].nLoop;
								continue;
							} else {
								QChan[c].bKey = 0;					// Stop playing
								break;
							}
						}
					} else {
						p = (QChan[c].nPos >> 12) & 0xFFFF;
						s = INTERPOLATE4PS_CUSTOM((QChan[c].nPos) & ((1 << 12) - 1),
												  QChan[c].PlayBank[p + 0],
												  QChan[c].PlayBank[p + 1],
												  QChan[c].PlayBank[p + 2],
												  QChan[c].PlayBank[p + 3],
												  256);
					}

					// Add to the sound currently in the buffer
					pTemp[0] += s * VolL;
					pTemp[1] += s * VolR;

					pTemp += 2;

					QChan[c].nPos += QChan[c].nAdvance;				// increment sample position based on pitch

					i--;
				}
			}
		}
	}

	if (bBurnUseMMX) {
#if defined BUILD_X86_ASM
		BurnSoundCopyClamp_A(Qs_s, pBurnSoundOut + (nPos << 1), nLen);
#endif
	} else {
		BurnSoundCopyClamp_C(Qs_s, pBurnSoundOut + (nPos << 1), nLen);
	}
	nPos = nEnd;

	return 0;
}
