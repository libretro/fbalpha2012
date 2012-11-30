#include "burnint.h"
#include "burn_sound.h"
#include "burn_ym2151.h"

void (*BurnYM2151Render)(INT16* pSoundBuf, INT32 nSegmentLength);

UINT8 BurnYM2151Registers[0x0100];
UINT32 nBurnCurrentYM2151Register;

static INT32 nBurnYM2151SoundRate;

static INT16* pBuffer;
static INT16* pYM2151Buffer[2];

static INT32 nBurnPosition;
static UINT32 nSampleSize;
static UINT32 nFractionalPosition;
static UINT32 nSamplesRendered;

static double YM2151Volumes[2];
static INT32 YM2151RouteDirs[2];

static void YM2151RenderNormal(INT16* pSoundBuf, INT32 nSegmentLength)
{
#if defined FBA_DEBUG
	if (!DebugSnd_YM2151Initted) bprintf(PRINT_ERROR, _T("YM2151RenderNormal called without init\n"));
#endif

	nBurnPosition += nSegmentLength;

	pYM2151Buffer[0] = pBuffer;
	pYM2151Buffer[1] = pBuffer + nSegmentLength;

	YM2151UpdateOne(0, pYM2151Buffer, nSegmentLength);
	
	for (INT32 n = 0; n < nSegmentLength; n++) {
		INT32 nLeftSample = 0, nRightSample = 0;
		
		if ((YM2151RouteDirs[BURN_SND_YM2151_YM2151_ROUTE_1] & BURN_SND_ROUTE_LEFT) == BURN_SND_ROUTE_LEFT) {
			nLeftSample += (INT32)(pYM2151Buffer[0][n] * YM2151Volumes[BURN_SND_YM2151_YM2151_ROUTE_1]);
		}
		if ((YM2151RouteDirs[BURN_SND_YM2151_YM2151_ROUTE_1] & BURN_SND_ROUTE_RIGHT) == BURN_SND_ROUTE_RIGHT) {
			nRightSample += (INT32)(pYM2151Buffer[0][n] * YM2151Volumes[BURN_SND_YM2151_YM2151_ROUTE_1]);
		}
		
		if ((YM2151RouteDirs[BURN_SND_YM2151_YM2151_ROUTE_2] & BURN_SND_ROUTE_LEFT) == BURN_SND_ROUTE_LEFT) {
			nLeftSample += (INT32)(pYM2151Buffer[1][n] * YM2151Volumes[BURN_SND_YM2151_YM2151_ROUTE_2]);
		}
		if ((YM2151RouteDirs[BURN_SND_YM2151_YM2151_ROUTE_2] & BURN_SND_ROUTE_RIGHT) == BURN_SND_ROUTE_RIGHT) {
			nRightSample += (INT32)(pYM2151Buffer[1][n] * YM2151Volumes[BURN_SND_YM2151_YM2151_ROUTE_2]);
		}
		
		nLeftSample = BURN_SND_CLIP(nLeftSample);
		nRightSample = BURN_SND_CLIP(nRightSample);
			
		pSoundBuf[(n << 1) + 0] = nLeftSample;
		pSoundBuf[(n << 1) + 1] = nRightSample;
	}
}

void BurnYM2151Reset()
{
#if defined FBA_DEBUG
	if (!DebugSnd_YM2151Initted) bprintf(PRINT_ERROR, _T("BurnYM2151Reset called without init\n"));
#endif

	YM2151ResetChip(0);
}

void BurnYM2151Exit()
{
#if defined FBA_DEBUG
	if (!DebugSnd_YM2151Initted) bprintf(PRINT_ERROR, _T("BurnYM2151Exit called without init\n"));
#endif

	BurnYM2151SetIrqHandler(NULL);
	BurnYM2151SetPortHandler(NULL);

	YM2151Shutdown();

	if (pBuffer) {
		free(pBuffer);
		pBuffer = NULL;
	}
	
	DebugSnd_YM2151Initted = 0;
}

INT32 BurnYM2151Init(INT32 nClockFrequency)
{
	DebugSnd_YM2151Initted = 1;
	
	if (nBurnSoundRate <= 0) {
		YM2151Init(1, nClockFrequency, 11025);
		return 0;
	}

   nBurnYM2151SoundRate = nBurnSoundRate;
   BurnYM2151Render = YM2151RenderNormal;

	YM2151Init(1, nClockFrequency, nBurnYM2151SoundRate);

	pBuffer = (INT16*)malloc(65536 * 2 * sizeof(INT16));
	memset(pBuffer, 0, 65536 * 2 * sizeof(INT16));

	nSampleSize = (UINT32)nBurnYM2151SoundRate * (1 << 16) / nBurnSoundRate;
	nFractionalPosition = 4 << 16;
	nSamplesRendered = 0;
	nBurnPosition = 0;
	
	// default routes
	YM2151Volumes[BURN_SND_YM2151_YM2151_ROUTE_1] = 1.00;
	YM2151Volumes[BURN_SND_YM2151_YM2151_ROUTE_2] = 1.00;
	YM2151RouteDirs[BURN_SND_YM2151_YM2151_ROUTE_1] = BURN_SND_ROUTE_BOTH;
	YM2151RouteDirs[BURN_SND_YM2151_YM2151_ROUTE_2] = BURN_SND_ROUTE_BOTH;

	return 0;
}

void BurnYM2151SetRoute(INT32 nIndex, double nVolume, INT32 nRouteDir)
{
#if defined FBA_DEBUG
	if (!DebugSnd_YM2151Initted) bprintf(PRINT_ERROR, _T("BurnYM2151SetRoute called without init\n"));
	if (nIndex < 0 || nIndex > 1) bprintf(PRINT_ERROR, _T("BurnYM2151SetRoute called with invalid index %i\n"), nIndex);
#endif
	
	YM2151Volumes[nIndex] = nVolume;
	YM2151RouteDirs[nIndex] = nRouteDir;
}

void BurnYM2151Scan(INT32 nAction)
{
#if defined FBA_DEBUG
	if (!DebugSnd_YM2151Initted) bprintf(PRINT_ERROR, _T("BurnYM2151Scan called without init\n"));
#endif
	
	if ((nAction & ACB_DRIVER_DATA) == 0) {
		return;
	}
	SCAN_VAR(nBurnCurrentYM2151Register);
	SCAN_VAR(BurnYM2151Registers);

	if (nAction & ACB_WRITE) {
		for (INT32 i = 0; i < 0x0100; i++) {
			YM2151WriteReg(0, i, BurnYM2151Registers[i]);
		}
	}
}
