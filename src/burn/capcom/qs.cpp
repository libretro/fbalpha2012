#include "cps.h"
#include <math.h>
#include "burn_sound.h"
// QSound

static int nQsndCyclesExtra;

static int qsndTimerOver(int, int)
{
//	bprintf(PRINT_NORMAL, _T("  - IRQ -> 1.\n"));
	ZetSetIRQLine(0xFF, ZET_IRQSTATUS_AUTO);

	return 0;
}

int QsndInit()
{
	int nVolumeShift;
	int nRate;

	// Init QSound z80
	if (QsndZInit()) {
		return 1;
	}
	BurnTimerInit(qsndTimerOver, NULL);

//	if (Cps1Qs == 1) {
//		nCpsZ80Cycles = 6000000 * 100 / nBurnFPS;
//		BurnTimerAttachZet(6000000);
//	} else {
		nCpsZ80Cycles = 8000000 * 100 / nBurnFPS;
		BurnTimerAttachZet(8000000);
//	}

	if (nBurnSoundRate >= 0) {
		nRate = nBurnSoundRate;
	} else {
		nRate = 11025;
	}

	nVolumeShift = 0;

	// These games are too soft at normal volumes
	if (strncmp(BurnDrvGetTextA(DRV_NAME), "csclub", 6) == 0) {
		nVolumeShift = -1;
	}
#if 0
	// These games are loud at normal volumes (no clipping)
	if (strncmp(BurnDrvGetTextA(DRV_NAME), "1944",	  4) == 0 ||
		strcmp( BurnDrvGetTextA(DRV_NAME), "dimahoo"  ) == 0 ||
		strcmp( BurnDrvGetTextA(DRV_NAME), "gmahoo"   ) == 0)
	{
		nVolumeShift = 1;
	}
#endif
	// These games are too loud at normal volumes (no clipping)
	if (strncmp(BurnDrvGetTextA(DRV_NAME), "sgemf",  5) == 0 ||
//		strncmp(BurnDrvGetTextA(DRV_NAME), "pfght",  5) == 0 ||
		strncmp(BurnDrvGetTextA(DRV_NAME), "mpang",  5) == 0 ||
//		strncmp(BurnDrvGetTextA(DRV_NAME), "spf2",   4) == 0 ||
//		strncmp(BurnDrvGetTextA(DRV_NAME), "sfa2",   4) == 0 ||
		strncmp(BurnDrvGetTextA(DRV_NAME), "sfa2",   4) == 0)
	{
		nVolumeShift = 1;
	}
	// These games are too loud at normal volumes (clipping)
	if (strncmp(BurnDrvGetTextA(DRV_NAME), "19xx",   4) == 0 ||
		strncmp(BurnDrvGetTextA(DRV_NAME), "ddtod",  5) == 0)
	{
		nVolumeShift = 2;
	}

	QscInit(nRate, nVolumeShift);		// Init QSound chip

	return 0;
}

void QsndReset()
{
	BurnTimerReset();
	BurnTimerSetRetrig(0, 1.0 / 252.0);

	nQsndCyclesExtra = 0;
}

void QsndExit()
{
	QscExit();							// Exit QSound chip
	QsndZExit();
}

int QsndScan(int nAction)
{
	if (nAction & ACB_DRIVER_DATA) {
		QsndZScan(nAction);				// Scan Z80
		QscScan(nAction);				// Scan QSound Chip
	}

	return 0;
}

static int nPos;

#define QscNewFrame() \
	nPos = 0;

void QsndNewFrame()
{
	ZetNewFrame();

	ZetOpen(0);
	ZetIdle(nQsndCyclesExtra);

	QscNewFrame();
}

void QsndEndFrame()
{
	BurnTimerEndFrame(nCpsZ80Cycles);
	QscUpdate(nBurnSoundLen);

	nQsndCyclesExtra = ZetTotalCycles() - nCpsZ80Cycles;
	ZetClose();
}

void QsndSyncZ80()
{
	int nCycles = (long long)SekTotalCycles() * nCpsZ80Cycles / nCpsCycles;

	if (nCycles <= ZetTotalCycles()) {
		return;
	}

	BurnTimerUpdate(nCycles);
}

/*******************
qs_c.cpp
********************
*/
// QSound - emulator for the QSound Chip

static const int nQscClock = 4000000;
static const int nQscClockDivider = 166;

static int nQscRate = 0;
static int nQscVolumeShift;

static int Tams = -1;
static int* Qs_s = NULL;


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
      #ifndef SN_TARGET_PS3
		if (nInterpolation >= 3) {
			pc->nEndBuffer[0] = pc->PlayBank[(pc->nEnd >> 12) - 4];
			pc->nEndBuffer[1] = pc->PlayBank[(pc->nEnd >> 12) - 5];
			pc->nEndBuffer[2] = pc->PlayBank[(pc->nEnd >> 12) - 6];
			pc->nEndBuffer[3] = pc->PlayBank[(pc->nEnd >> 12) - 7];

			if (pc->nLoop) {
				for (int i = 0, j = 0; i < 4; i++, j++) {
					if (j >= (pc->nLoop >> 12)) {
						j = 0;
					}
					pc->nEndBuffer[i + 4] = pc->PlayBank[((pc->nEnd - pc->nLoop) >> 12) + j];
				}
			} else {
					pc->nEndBuffer[4] = pc->nEndBuffer[3];
					pc->nEndBuffer[5] = pc->nEndBuffer[3];
					pc->nEndBuffer[6] = pc->nEndBuffer[3];
					pc->nEndBuffer[7] = pc->nEndBuffer[3];
			}
		}
     #endif
	}
}

#ifdef SN_TARGET_PS3
#define CalcAdvance(pc) pc->nAdvance = (long long)pc->nPitch * nQscClock / nQscClockDivider / nQscRate;
#else
#define CalcAdvance(pc) \
	if (nQscRate) { \
		pc->nAdvance = (long long)pc->nPitch * nQscClock / nQscClockDivider / nQscRate; \
	}
#endif

#define QscReset() \
	memset(QChan, 0, sizeof(QChan)); \
	/* Point all to bank 0 */ \
	for (int i = 0; i < 16; i++) { \
		QChan[i].PlayBank = (char*)CpsQSam; \
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
         struct QChan * pc = QChan + i;
			CalcAdvance(pc);
		}
	}

	return 0;
}


#define QscSyncQsnd() \
	QscUpdate(ZetTotalCycles() * nBurnSoundLen / nCpsZ80Cycles);

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
#ifndef SN_TARGET_PS3
	if (pBurnSoundOut == NULL) {
		return 0;
	}
#endif

	if (nEnd > nBurnSoundLen) {
		nEnd = nBurnSoundLen;
	}

	int nLen = nEnd - nPos;
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

   #ifndef SN_TARGET_PS3
	if (nInterpolation < 3) {
   #endif

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

		BurnSoundCopyClamp_C(Qs_s, pBurnSoundOut + (nPos << 1), nLen);

		nPos = nEnd;

		return 0;
   #ifndef SN_TARGET_PS3
	}
   #endif

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

			{
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

	BurnSoundCopyClamp_C(Qs_s, pBurnSoundOut + (nPos << 1), nLen);

	nPos = nEnd;

	return 0;
}

/*************
qs_z.cpp
**************
*/

// QSound - Z80

static int nQsndZBank = 0;

// Map in the memory for the current 0x8000-0xc000 music bank
static int QsndZBankMap()
{
	unsigned int nOff;
	unsigned char* Bank;
	nOff = nQsndZBank << 14;
	nOff += 0x8000;

	if (Cps1Qs == 0) {
		if (nOff + 0x4000 > nCpsZRomLen) {			// End of bank is out of range
			nOff = 0;
		}
		Bank = CpsZRom + nOff;
	} else {
#ifdef SN_TARGET_PS3
		if (nOff + 0x4000 > (nCpsZRomLen >> 1)) {
#else
		if (nOff + 0x4000 > (nCpsZRomLen / 2)) {
#endif
			nOff = 0;
		}
#ifdef SN_TARGET_PS3
		Bank = CpsZRom - (nCpsZRomLen >> 1) + nOff;
#else
		Bank = CpsZRom - (nCpsZRomLen / 2) + nOff;
#endif
	}

	// Read and fetch the bank
	ZetMapArea(0x8000, 0xbfff, 0, Bank);
	if (Cps1Qs == 0) {
		ZetMapArea(0x8000, 0xbfff, 2, Bank, CpsZRom + nOff);
	} else {
		ZetMapArea(0x8000, 0xbfff, 2, Bank);
	}

	return 0;
}

static unsigned char QscCmd[2] = {0, 0};

void __fastcall QsndZWrite(unsigned short a, unsigned char d)
{
	if (a == 0xd000) {
		QscCmd[0] = d;
		return;
	}
	if (a == 0xd001) {
		QscCmd[1] = d;
		return;
	}
	if (a == 0xd002) {
		QscWrite(d, (QscCmd[0] << 8) | QscCmd[1]);
//		bprintf(PRINT_NORMAL, _T("QSound command %02X %04X sent.\n"), d, (QscCmd[0] << 8) | QscCmd[1]);
		return;
	}
	if (a == 0xd003) {
		int nNewBank = d & 0x0f;
		if (nQsndZBank != nNewBank) {
			nQsndZBank = nNewBank;
			QsndZBankMap();
		}
	}
}

unsigned char __fastcall QsndZRead(unsigned short a)
{
	if (a == 0xd007) {						// return ready all the time
		return 0x80;
	}
	return 0;
}

int QsndZInit()
{
	if (nCpsZRomLen < 0x8000) {				// Not enough Z80 Data
		return 1;
	}
	if (CpsZRom == NULL) {
		return 1;
	}

	ZetInit(1);

	ZetSetReadHandler(QsndZRead);
	ZetSetWriteHandler(QsndZWrite);

	// Read and fetch first 0x8000 of Rom
	if (Cps1Qs) {
		ZetMapArea(0x0000, 0x7FFF, 0, CpsZRom - (nCpsZRomLen / 2));
		ZetMapArea(0x0000, 0x7FFF, 2, CpsZRom, CpsZRom - (nCpsZRomLen / 2));	// If it tries to fetch this area
	} else {
		ZetMapArea(0x0000, 0x7FFF, 0 ,CpsZRom);
		ZetMapArea(0x0000, 0x7FFF, 2, CpsZRom);
	}

	// Map first Bank of Rom
	nQsndZBank = 0;
	QsndZBankMap();

	ZetMapArea(0xC000, 0xCFFF, 0, CpsZRamC0);
	ZetMapArea(0xC000, 0xCFFF, 1, CpsZRamC0);
	ZetMapArea(0xC000, 0xCFFF, 2, CpsZRamC0);

	ZetMemCallback(0xD000, 0xEFFF, 0);
	ZetMemCallback(0xD000, 0xEFFF, 1);

	if (Cps1Qs) {
		ZetMapArea(0xD000, 0xEFFF, 2, CpsZRom, CpsZRom - (nCpsZRomLen / 2));	// If it tries to fetch this area
	} else {
		ZetMapArea(0xD000, 0xEFFF, 2, CpsZRom);
	}

	ZetMapArea(0xF000, 0xFFFF, 0, CpsZRamF0);
	ZetMapArea(0xF000, 0xFFFF, 1, CpsZRamF0);
	ZetMapArea(0xF000, 0xFFFF, 2, CpsZRamF0);

	ZetMemEnd();
	ZetClose();

	return 0;
}

int QsndZExit()
{
	ZetExit();
	return 0;
}

// Scan the current QSound z80 state
int QsndZScan(int nAction)
{
	ZetScan(nAction);					// Scan Z80
	SCAN_VAR(nQsndZBank);

	if (nAction & ACB_WRITE) {			// If write, bank could have changed
		QsndZBankMap();
	}

	return 0;
}
