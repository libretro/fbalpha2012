#include "burnint.h"
#include "burn_sound.h"
#include "burn_ymf278b.h"

static int (*BurnYMF278BStreamCallback)(int nSoundRate);

static short* pBuffer;
static short* pYMF278BBuffer[2];

static int nYMF278BPosition;
static unsigned int nFractionalPosition;

// ----------------------------------------------------------------------------
// Dummy functions

static int YMF278BStreamCallbackDummy(int /* nSoundRate */)
{
	return 0;
}


// ----------------------------------------------------------------------------
// Execute YMF278B for part of a frame

static void YMF278BRender(int nSegmentLength)
{
	if (nYMF278BPosition >= nSegmentLength) {
		return;
	}

//	bprintf(PRINT_NORMAL, _T("    YMF278B render %6i -> %6i\n"), nYMF278BPosition, nSegmentLength);

	nSegmentLength -= nYMF278BPosition;

	pYMF278BBuffer[0] = pBuffer + 0 * 4096 + 4 + nYMF278BPosition;
	pYMF278BBuffer[1] = pBuffer + 1 * 4096 + 4 + nYMF278BPosition;

	ymf278b_pcm_update(0, pYMF278BBuffer, nSegmentLength);

	nYMF278BPosition += nSegmentLength;
}

// ----------------------------------------------------------------------------
// Update the sound buffer

void BurnYMF278BUpdate(int nSegmentEnd)
{
	short* pSoundBuf = pBurnSoundOut;
	int nSegmentLength = nSegmentEnd;

//	bprintf(PRINT_NORMAL, _T("    YMF278B render %6i -> %6i\n"), nYMF278BPosition, nSegmentEnd);

	if (nBurnSoundRate == 0) {
		return;
	}

	if (nSegmentEnd < nYMF278BPosition) {
		nSegmentEnd = nYMF278BPosition;
	}

	if (nSegmentLength > nBurnSoundLen) {
		nSegmentLength = nBurnSoundLen;
	}

	YMF278BRender(nSegmentEnd);

	pYMF278BBuffer[0] = pBuffer + 0 * 4096 + 4;
	pYMF278BBuffer[1] = pBuffer + 1 * 4096 + 4;

	for (int i = nFractionalPosition; i < nSegmentLength; i++) {
		pSoundBuf[(i << 1) + 0] = pYMF278BBuffer[0][i];
		pSoundBuf[(i << 1) + 1] = pYMF278BBuffer[1][i];
	}

	nFractionalPosition = nSegmentLength;

	if (nSegmentEnd >= nBurnSoundLen) {
		int nExtraSamples = nSegmentEnd - nBurnSoundLen;

		for (int i = 0; i < nExtraSamples; i++) {
			pYMF278BBuffer[0][i] = pYMF278BBuffer[0][nBurnSoundLen + i];
			pYMF278BBuffer[1][i] = pYMF278BBuffer[1][nBurnSoundLen + i];
		}

		nFractionalPosition = 0;

		nYMF278BPosition = nExtraSamples;

	}
}

// ----------------------------------------------------------------------------

void BurnYMF278BSelectRegister(int nRegister, unsigned char nValue)
{
	switch (nRegister) {
		case 0:
//			bprintf(PRINT_NORMAL, _T("    YMF278B register A -> %i\n"), nValue);
			YMF278B_control_port_0_A_w(nValue);
			break;
		case 1:
			YMF278B_control_port_0_B_w(nValue);
			break;
		case 2:
//			bprintf(PRINT_NORMAL, _T("    YMF278B register C -> %i\n"), nValue);
			YMF278B_control_port_0_C_w(nValue);
			break;
	}
}
void BurnYMF278BWriteRegister(int nRegister, unsigned char nValue)
{
	switch (nRegister) {
		case 0:
			BurnYMF278BUpdate(BurnYMF278BStreamCallback(nBurnSoundRate));
			YMF278B_data_port_0_A_w(nValue);
			break;
		case 1:
			YMF278B_data_port_0_B_w(nValue);
			break;
		case 2:
			BurnYMF278BUpdate(BurnYMF278BStreamCallback(nBurnSoundRate));
			YMF278B_data_port_0_C_w(nValue);
			break;
	}
}

unsigned char BurnYMF278BReadStatus()
{
	BurnYMF278BUpdate(BurnYMF278BStreamCallback(nBurnSoundRate));
	return YMF278B_status_port_0_r();
}

unsigned char BurnYMF278BReadData()
{
	return YMF278B_data_port_0_r();
}

// ----------------------------------------------------------------------------

void BurnYMF278BReset()
{
	BurnTimerReset();
}

void BurnYMF278BExit()
{
	YMF278B_sh_stop();

	BurnTimerExit();

	free(pBuffer);
	pBuffer = NULL;
}

int BurnYMF278BInit(int /* nClockFrequency */, unsigned char* YMF278BROM, void (*IRQCallback)(int, int), int (*StreamCallback)(int))
{
	BurnYMF278BExit();

	BurnYMF278BStreamCallback = YMF278BStreamCallbackDummy;
	if (StreamCallback) {
		BurnYMF278BStreamCallback = StreamCallback;
	}

	ymf278b_start(0, YMF278BROM, IRQCallback, BurnYMFTimerCallback, YMF278B_STD_CLOCK, nBurnSoundRate);
	BurnTimerInit(ymf278b_timer_over, NULL);

	pBuffer = (short*)malloc(4096 * 2 * sizeof(short));
	memset(pBuffer, 0, 4096 * 2 * sizeof(short));

	nYMF278BPosition = 0;

	nFractionalPosition = 0;

	return 0;
}

void BurnYMF278BScan(int nAction, int* pnMin)
{
	BurnTimerScan(nAction, pnMin);
}
