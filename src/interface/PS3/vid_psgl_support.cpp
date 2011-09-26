// Support functions for blitters that use PSGL
#include "burner.h"
#include "vid_psgl_support.h"

// ---------------------------------------------------------------------------
// General


void VidSExit()
{
	 
}
 
#if 0
// ----------------------------------------------------------------------------
// Gamma controls

static IDirectDrawGammaControl* pGammaControl;
static DDGAMMARAMP* pFBAGamma = NULL;
static DDGAMMARAMP* pSysGamma = NULL;

void VidSRestoreGamma()
{
	if (pGammaControl) {
		if (pSysGamma) {
			pGammaControl->SetGammaRamp(0, pSysGamma);
		}

		free(pSysGamma);
		pSysGamma = NULL;
		free(pFBAGamma);
		pFBAGamma = NULL;

		RELEASE(pGammaControl);
	}
}

int VidSUpdateGamma()
{
	if (pGammaControl) {
		if (bDoGamma) {
			for (int i = 0; i < 256; i++) {
				int nValue = (int)(65535.0 * pow((i / 255.0), nGamma));
				pFBAGamma->red[i] = nValue;
				pFBAGamma->green[i] = nValue;
				pFBAGamma->blue[i] = nValue;
			}
			pGammaControl->SetGammaRamp(0, pFBAGamma);
		} else {
			pGammaControl->SetGammaRamp(0, pSysGamma);
		}
	}

	return 0;
}

int VidSSetupGamma(IDirectDrawSurface7* pSurf)
{
	pGammaControl = NULL;

	if (!bVidUseHardwareGamma || !nVidFullscreen) {
		return 0;
	}

	if (FAILED(pSurf->QueryInterface(IID_IDirectDrawGammaControl, (void**)&pGammaControl))) {
		pGammaControl = NULL;
#ifdef PRINT_DEBUG_INFO
		dprintf(_T("  * Warning: Couldn't use hardware gamma controls.\n"));
#endif

		return 1;
	}

	pSysGamma = (DDGAMMARAMP*)malloc(sizeof(DDGAMMARAMP));
	if (pSysGamma == NULL) {
		VidSRestoreGamma();
		return 1;
	}
	pGammaControl->GetGammaRamp(0, pSysGamma);

	pFBAGamma = (DDGAMMARAMP*)malloc(sizeof(DDGAMMARAMP));
	if (pFBAGamma == NULL) {
		VidSRestoreGamma();
		return 1;
	}

	VidSUpdateGamma();

	return 0;
}
#endif

// ---------------------------------------------------------------------------
// Fullscreen mode support routines

int VidSScoreDisplayMode(VidSDisplayScoreInfo* pScoreInfo)
{
	 

	return 0;
}

int VidSInitScoreInfo(VidSDisplayScoreInfo* pScoreInfo)
{
 
	return 0;
}
 
void VidSRestoreScreenMode()
{
	 
}

// Enter fullscreen mode, select optimal full-screen resolution
int VidSEnterFullscreenMode(int nZoom, int nDepth)
{
	 

	return 0;
}
 

static void VidSExitTinyMsg()
{
 
}

static void VidSExitShortMsg()
{
 
}

void VidSExitChat()
{
	 
}

static void VidSExitEdit()
{
 
}

void VidSExitOSD()
{
 
}

static int VidSInitTinyMsg(int nFlags)
{
	 

	return 0;
}

static int VidSInitShortMsg(int nFlags)
{
	 
	 
	return 0;
}

static int VidSInitChat(int nFlags)
{
 
	return 0;
}

static int VidSInitEdit(int nFlags)
{
 

	return 0;
}

int VidSInitOSD(int nFlags)
{ 
	return 0;
}

int VidSRestoreOSD()
{
 

	return 0;
}

 

static int VidSDrawChat(RECT* dest)
{
 
	return 0;
}
 

int VidSNewTinyMsg(const TCHAR* pText, int nRGB, int nDuration, int nPriority)	// int nRGB = 0, int nDuration = 0, int nPriority = 5
{
	 
	return 0;
 }

int VidSNewShortMsg(const TCHAR* pText, int nRGB, int nDuration, int nPriority)	// int nRGB = 0, int nDuration = 0, int nPriority = 5
{
 
	return 0;
}

void VidSKillShortMsg()
{
	 
}

void VidSKillTinyMsg()
{
	 
}

void VidSKillOSDMsg()
{
	 
}

void VidSKillTinyOSDMsg()
{
	 
}

int VidSAddChatMsg(const TCHAR* pID, int nIDRGB, const TCHAR* pMain, int nMainRGB)
{
 

	return 0;
}
