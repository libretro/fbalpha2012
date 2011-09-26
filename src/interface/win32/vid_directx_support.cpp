// Support functions for blitters that use DirectX
#include "burner.h"
#include "vid_directx_support.h"

#ifdef _MSC_VER
#pragma comment(lib, "ddraw")
#endif

// ---------------------------------------------------------------------------
// General

static IDirectDraw7* pDD = NULL;

void VidSExit()
{
	pDD = NULL;
}

int VidSInit(IDirectDraw7* pDD7)
{
	pDD = pDD7;
	if (pDD == NULL) {
		return 1;
	}
	return 0;
}

// ---------------------------------------------------------------------------

// Get bit-depth of a surface (15/16/24/32)
int VidSGetSurfaceDepth(IDirectDrawSurface7* pSurf)
{
	if (pSurf == NULL) {
		return 0;
	}

	// Find out the pixelformat of the screen surface
	DDPIXELFORMAT ddpf;
	memset(&ddpf, 0, sizeof(ddpf));
	ddpf.dwSize = sizeof(ddpf);

	if (SUCCEEDED(pSurf->GetPixelFormat(&ddpf))) {
		int nDepth = ddpf.dwRGBBitCount;
		if (nDepth == 16 && ddpf.dwGBitMask == 0x03E0) {
			nDepth = 15;
		}

		return nDepth;
	}
	return 0;
}

// Clear a surface to a specified colour
int VidSClearSurface(IDirectDrawSurface7* pSurf, unsigned int nColour, RECT* pRect)
{
	if (pSurf == NULL) {
		return 1;
	}

	DDBLTFX BltFx;
	memset(&BltFx, 0, sizeof(BltFx));
	BltFx.dwSize = sizeof(BltFx);
	BltFx.dwFillColor = nColour;

	if (FAILED(pSurf->Blt(NULL, NULL, pRect, DDBLT_COLORFILL, &BltFx))) {
		return 1;
	}
	return 0;
}

// Clipper
int VidSClipperInit(IDirectDrawSurface7* pSurf)
{
	if (nVidFullscreen && bDrvOkay) {
		return 0;
	}

	IDirectDrawClipper* pClipper = NULL;
	if (SUCCEEDED(DirectDrawCreateClipper(0, &pClipper, NULL))) {
		if (SUCCEEDED(pClipper->SetHWnd(0, hVidWnd))) {
			pSurf->SetClipper(pClipper);
			RELEASE(pClipper);
			return 0;
		}
	}
	return 1;
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
	// Continue if the resolution is too low
	if (pScoreInfo->nModeWidth < pScoreInfo->nRequestedWidth
		|| pScoreInfo->nModeHeight < pScoreInfo->nRequestedHeight) {
		return 0;
	}

	// Only test standard resolutions if below 512 x 384
	if ((pScoreInfo->nModeWidth == 320 && pScoreInfo->nModeHeight == 240)
		|| (pScoreInfo->nModeWidth == 400 && pScoreInfo->nModeHeight == 300)
		|| (pScoreInfo->nModeWidth >= 512 && pScoreInfo->nModeHeight >= 384)) {

		if (pScoreInfo->nRequestedWidth == 0 || pScoreInfo->nRequestedHeight == 0) {
			RECT rect = {0, 0, 0, 0};
			int nGameWidth = nVidImageWidth, nGameHeight = nVidImageHeight;
			unsigned int nScore;

			if (bDrvOkay) {
				if ((BurnDrvGetFlags() & BDF_ORIENTATION_VERTICAL) && (nVidRotationAdjust & 1)) {
					BurnDrvGetVisibleSize(&nGameHeight, &nGameWidth);
				} else {
					BurnDrvGetVisibleSize(&nGameWidth, &nGameHeight);
				}
			}

			nVidScrnWidth = rect.right = pScoreInfo->nModeWidth;
			nVidScrnHeight = rect.bottom = pScoreInfo->nModeHeight;
			VidScale(&rect, nGameWidth, nGameHeight);

			// Continue if the resolution is too low
			if (!bDrvOkay) {
				if ((unsigned int)(rect.bottom - rect.top) < pScoreInfo->nRequestedHeight) {
					return 0;
				}
			} else {
				if (BurnDrvGetFlags() & BDF_ORIENTATION_VERTICAL) {
					if ((unsigned int)(rect.right - rect.left) < pScoreInfo->nRequestedWidth) {
						return 0;
					}
				} else {
					if ((unsigned int)(rect.bottom - rect.top) < pScoreInfo->nRequestedHeight) {
						return 0;
					}
				}
			}

			// Score resolutions based on how many pixels are unused and the pixel aspect
			nScore = 65536 * (pScoreInfo->nModeWidth - (unsigned)(rect.right - rect.left)) + (pScoreInfo->nModeHeight - (unsigned)(rect.bottom - rect.top));
			nScore = (int)((double)nScore * ((double)pScoreInfo->nModeWidth / pScoreInfo->nModeHeight / vidScrnAspect));
			if (nScore < pScoreInfo->nBestScore) {
				pScoreInfo->nBestScore = nScore;
				pScoreInfo->nBestWidth = pScoreInfo->nModeWidth;
				pScoreInfo->nBestHeight = pScoreInfo->nModeHeight;
			}
		} else {
			// Select the lowest resolution that will fit the image
			if (pScoreInfo->nModeWidth < pScoreInfo->nBestWidth
				&& pScoreInfo->nModeHeight < pScoreInfo->nBestHeight) {
				pScoreInfo->nBestWidth = pScoreInfo->nModeWidth;
				pScoreInfo->nBestHeight = pScoreInfo->nModeHeight;
			}
		}
	}

	return 0;
}

int VidSInitScoreInfo(VidSDisplayScoreInfo* pScoreInfo)
{
	pScoreInfo->nBestWidth = -1U;
	pScoreInfo->nBestHeight = -1U;
	pScoreInfo->nBestScore = -1U;

	if (pScoreInfo->nRequestedZoom == 0) {
		return 0;
	}

	pScoreInfo->nRequestedWidth = 0;
	pScoreInfo->nRequestedHeight = 0;

	int nGameWidth = nVidImageWidth;
	int nGameHeight = nVidImageHeight;
	int nGameAspectX = 4, nGameAspectY = 3;

	if (bDrvOkay) {
		if ((BurnDrvGetFlags() & BDF_ORIENTATION_VERTICAL) && (nVidRotationAdjust & 1)) {
			BurnDrvGetVisibleSize(&nGameHeight, &nGameWidth);
			BurnDrvGetAspect(&nGameAspectY, &nGameAspectX);
		} else {
			BurnDrvGetVisibleSize(&nGameWidth, &nGameHeight);
			BurnDrvGetAspect(&nGameAspectX, &nGameAspectY);
		}
	}

	if (!bVidCorrectAspect || bVidFullStretch) {
		pScoreInfo->nRequestedWidth = nGameWidth * pScoreInfo->nRequestedZoom;
		pScoreInfo->nRequestedHeight = nGameHeight * pScoreInfo->nRequestedZoom;
	} else {
		if (bDrvOkay) {
			if (BurnDrvGetFlags() & BDF_ORIENTATION_VERTICAL) {
				pScoreInfo->nRequestedWidth = nGameWidth * pScoreInfo->nRequestedZoom;
			} else {
				pScoreInfo->nRequestedHeight = nGameHeight * pScoreInfo->nRequestedZoom;
			}
		}
	}

	return 0;
}

static HRESULT WINAPI myEnumModesCallback(LPDDSURFACEDESC2 pSurfaceDesc, void* lpContext)
{
	((VidSDisplayScoreInfo*)lpContext)->nModeWidth = pSurfaceDesc->dwWidth;
	((VidSDisplayScoreInfo*)lpContext)->nModeHeight = pSurfaceDesc->dwHeight;

	VidSScoreDisplayMode((VidSDisplayScoreInfo*)lpContext);

	return DDENUMRET_OK;
}

void VidSRestoreScreenMode()
{
	if (pDD == NULL) {
		return;
	}

	// Undo the changes we made to the display
	pDD->SetCooperativeLevel(NULL, DDSCL_NORMAL);

	nVidScrnWidth = 0;
	nVidScrnHeight = 0;
}

// Enter fullscreen mode, select optimal full-screen resolution
int VidSEnterFullscreenMode(int nZoom, int nDepth)
{
	if (pDD == NULL) {
		return 1;
	}

	if (nDepth == 0) {
		nDepth = nVidDepth;
	}
	if (nDepth == 15) {
		nDepth = 16;
	}

	if (FAILED(pDD->SetCooperativeLevel(hVidWnd, DDSCL_ALLOWREBOOT | DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN))) {
		return 1;
	}

	int nWidth = nVidWidth, nHeight = nVidHeight;

	if (nZoom) {
		VidSDisplayScoreInfo ScoreInfo;
		memset(&ScoreInfo, 0, sizeof(VidSDisplayScoreInfo));
		ScoreInfo.nRequestedZoom = nZoom;
		VidSInitScoreInfo(&ScoreInfo);

		DDSURFACEDESC2 ddsd;
		memset(&ddsd, 0, sizeof(ddsd));
		ddsd.dwSize = sizeof(ddsd);
		ddsd.dwFlags = DDSD_PIXELFORMAT;

		ddsd.ddpfPixelFormat.dwFlags = DDPF_RGB;
		ddsd.ddpfPixelFormat.dwRGBBitCount = nDepth;

		pDD->EnumDisplayModes(0, &ddsd, (void*)&ScoreInfo, myEnumModesCallback);

		if (ScoreInfo.nBestWidth == -1U) {
			VidSRestoreScreenMode();
			FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_ERR_UI_FULL_NOMODE));
			FBAPopupDisplay(PUF_TYPE_ERROR);
			return 1;
		}

		nWidth = ScoreInfo.nBestWidth;
		nHeight = ScoreInfo.nBestHeight;
	}

	if (!bDrvOkay && (nWidth < 640 || nHeight < 480)) {
		return 1;
	}

	if (FAILED(pDD->SetDisplayMode(nWidth, nHeight, nDepth, nVidRefresh, 0))) {
		VidSRestoreScreenMode();

		FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_ERR_UI_FULL_PROBLEM), nWidth, nHeight, nDepth, nVidRefresh);
		if (nWidth != 320 && nHeight != 240) {
			FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_ERR_UI_FULL_CUSTRES));
		}
		FBAPopupDisplay(PUF_TYPE_ERROR);

		nVidScrnWidth = 0;
		nVidScrnHeight = 0;

		return 1;
	}

	nVidScrnWidth = nWidth;
	nVidScrnHeight = nHeight;

	return 0;
}

// ---------------------------------------------------------------------------
// Text display routines

#define MESSAGE_SIZE 64
// short message
static struct {
	TCHAR pMsgText[MESSAGE_SIZE];
	COLORREF nColour;
	int nPriority;
	unsigned int nTimer;
} VidSShortMsg = { _T(""), 0, 0, 0,};
static HFONT ShortMsgFont = NULL;
const int shortMsgWidth = 240;

// tiny message
static struct {
	TCHAR pMsgText[MESSAGE_SIZE];
	COLORREF nColour;
	int nPriority;
	unsigned int nTimer;
} VidSTinyMsg = {_T(""), 0, 0, 0};
static HFONT TinyMsgFont = NULL;
const int tinyMsgWidth = 300;

// chat message
#define CHAT_SIZE 11
static struct {
	TCHAR* pIDText;
	COLORREF nIDColour;
	TCHAR* pMainText;
	COLORREF nMainColour;
} VidSChatMessage[CHAT_SIZE];

static bool bChatInitialised = false;
static HFONT ChatIDFont = NULL;
static HFONT ChatMainFont = NULL;
static unsigned int nChatTimer;
static int nChatFontSize;
static int nChatShadowOffset;
static int nChatOverFlow;
static bool bDrawChat;

static HFONT EditTextFont = NULL;
static HFONT EditCursorFont = NULL;
static DWORD nPrevStart, nPrevEnd;
static int nCursorTime;
static int nCursorState = 0;
static int nEditSize;
static int nEditShadowOffset;

// ddraw surface
static IDirectDrawSurface7* pShortMsgSurf = NULL;
static IDirectDrawSurface7* pChatSurf = NULL;
static IDirectDrawSurface7* pEditSurf = NULL;
static IDirectDrawSurface7* pTinyMsgSurf = NULL;

static unsigned int nKeyColour = 0x000001;

int nMaxChatFontSize = 20;		// Maximum size of the font used for the Kaillera chat function (1280x960 or higher)
int nMinChatFontSize = 12;		// Minimum size of the font used for the Kaillera chat function (arcade resolution)

static int nZoom;				// & 1: zoom X, & 2: zoom Y

static int nShortMsgFlags;

bool bEditActive = false;
bool bEditTextChanged = false;
TCHAR EditText[MAX_CHAT_SIZE + 1] = _T("");

TCHAR OSDMsg[128] = _T("");
unsigned int nOSDTimer = 0;
TCHAR tinyMsg[64] = _T("");
unsigned int nTinyTimer = 0;

static BOOL MyTextOut(HDC hdc, int nXStart, int nYStart, LPCTSTR lpString, int cbString, int nShadowOffset, int nColour)
{
	// Print a black shadow
	SetTextColor(hdc, 0);
	if (nShadowOffset >= 2) {
		TextOut(hdc, nXStart + nShadowOffset, nYStart + nShadowOffset, lpString, cbString);
	}
	// Print a black outline
	TextOut(hdc, nXStart - 1, nYStart - 1, lpString, cbString);
	TextOut(hdc, nXStart + 0, nYStart - 1, lpString, cbString);
	TextOut(hdc, nXStart + 1, nYStart - 1, lpString, cbString);
	TextOut(hdc, nXStart + 1, nYStart + 0, lpString, cbString);
	TextOut(hdc, nXStart + 1, nYStart + 1, lpString, cbString);
	TextOut(hdc, nXStart + 0, nYStart + 1, lpString, cbString);
	TextOut(hdc, nXStart - 1, nYStart + 1, lpString, cbString);
	TextOut(hdc, nXStart - 1, nYStart + 0, lpString, cbString);
	// Print the text on top
	SetTextColor(hdc, nColour);

	return TextOut(hdc, nXStart, nYStart, lpString, cbString);
}

static BOOL MyExtTextOut(HDC hdc, int X, int Y, UINT fuOptions, CONST RECT* lprc, LPCTSTR lpString, UINT cbCount, CONST INT* lpDx, int nShadowOffset, int nColour)
{
	// Print a black shadow
	SetTextColor(hdc, 0);
	if (nShadowOffset >= 2) {
		ExtTextOut(hdc, X + nShadowOffset, Y + nShadowOffset, fuOptions, lprc, lpString, cbCount, lpDx);
	}
	// Print a black outline
	ExtTextOut(hdc, X - 1, Y - 1, fuOptions, lprc, lpString, cbCount, lpDx);
	ExtTextOut(hdc, X + 0, Y - 1, fuOptions, lprc, lpString, cbCount, lpDx);
	ExtTextOut(hdc, X + 1, Y - 1, fuOptions, lprc, lpString, cbCount, lpDx);
	ExtTextOut(hdc, X + 1, Y + 0, fuOptions, lprc, lpString, cbCount, lpDx);
	ExtTextOut(hdc, X + 1, Y + 1, fuOptions, lprc, lpString, cbCount, lpDx);
	ExtTextOut(hdc, X + 0, Y + 1, fuOptions, lprc, lpString, cbCount, lpDx);
	ExtTextOut(hdc, X - 1, Y + 1, fuOptions, lprc, lpString, cbCount, lpDx);
	ExtTextOut(hdc, X - 1, Y + 0, fuOptions, lprc, lpString, cbCount, lpDx);
	// Print the text on top
	SetTextColor(hdc, nColour);

	return ExtTextOut(hdc, X, Y, fuOptions, lprc, lpString, cbCount, lpDx);
}

static void VidSExitTinyMsg()
{
	VidSTinyMsg.nTimer = 0;

	if (TinyMsgFont) {
		DeleteObject(TinyMsgFont);
		TinyMsgFont = NULL;
	}

	RELEASE(pTinyMsgSurf);
}

static void VidSExitShortMsg()
{
	VidSShortMsg.nTimer = 0;

	if (ShortMsgFont) {
		DeleteObject(ShortMsgFont);
		ShortMsgFont = NULL;
	}

	RELEASE(pShortMsgSurf);
}

void VidSExitChat()
{
	nChatTimer = 0;

	if (ChatIDFont) {
		DeleteObject(ChatIDFont);
		ChatIDFont = NULL;
	}
	if (ChatMainFont) {
		DeleteObject(ChatMainFont);
		ChatMainFont = NULL;
	}

	for (int i = 0; i < CHAT_SIZE; i++) {
		free(VidSChatMessage[i].pIDText);
		VidSChatMessage[i].pIDText = NULL;
		free(VidSChatMessage[i].pMainText);
		VidSChatMessage[i].pMainText = NULL;
	}

	RELEASE(pChatSurf);
}

static void VidSExitEdit()
{
	bEditActive = false;

	if (EditTextFont) {
		DeleteObject(EditTextFont);
		EditTextFont = NULL;
	}
	if (EditCursorFont) {
		DeleteObject(EditCursorFont);
		EditCursorFont = NULL;
	}

	RELEASE(pEditSurf);
}

void VidSExitOSD()
{
	VidSExitTinyMsg();
	if (kNetGame) {
		VidSExitEdit();
	}
	VidSExitChat();
	VidSExitShortMsg();
}

static int VidSInitTinyMsg(int nFlags)
{
	DDSURFACEDESC2 ddsd;

	VidSExitTinyMsg();

	TinyMsgFont = CreateFont(11 - nFlags, 0, 0, 0, FW_SEMIBOLD, 0, 0, 0, 0, 0, 0, ANTIALIASED_QUALITY, FF_DONTCARE, _T("MS Sans Serif"));
	VidSTinyMsg.nTimer = 0;

	// create surface to display the text
	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_CKSRCBLT;

	ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_VIDEOMEMORY;

	ddsd.dwWidth = tinyMsgWidth;
	ddsd.dwHeight = 20;

	ddsd.ddckCKSrcBlt.dwColorSpaceLowValue = nKeyColour;
	ddsd.ddckCKSrcBlt.dwColorSpaceHighValue = nKeyColour;

	if (FAILED(pDD->CreateSurface(&ddsd, &pTinyMsgSurf, NULL))) {
#ifdef PRINT_DEBUG_INFO
		printf("  * Error: Couldn't create OSD texture.\n");
#endif
		return 1;
	}

	VidSClearSurface(pTinyMsgSurf, nKeyColour, NULL);

	return 0;
}

static int VidSInitShortMsg(int nFlags)
{
	DDSURFACEDESC2 ddsd;

	VidSExitShortMsg();

	nShortMsgFlags = nFlags;

	ShortMsgFont = CreateFont(20 - nFlags, 0, 0, 0, FW_SEMIBOLD, 0, 0, 0, 0, 0, 0, ANTIALIASED_QUALITY, FF_DONTCARE, _T("Lucida"));
	VidSShortMsg.nTimer = 0;

	// create surface to display the text
	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_CKSRCBLT;

	ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_VIDEOMEMORY;

	ddsd.dwWidth = shortMsgWidth;
	ddsd.dwHeight = 32;

	ddsd.ddckCKSrcBlt.dwColorSpaceLowValue = nKeyColour;
	ddsd.ddckCKSrcBlt.dwColorSpaceHighValue = nKeyColour;

	if (FAILED(pDD->CreateSurface(&ddsd, &pShortMsgSurf, NULL))) {
#ifdef PRINT_DEBUG_INFO
		dprintf(_T("  * Error: Couldn't create OSD texture.\n"));
#endif
		return 1;
	}

	VidSClearSurface(pShortMsgSurf, nKeyColour, NULL);

	return 0;
}

static int VidSInitChat(int nFlags)
{
	DDSURFACEDESC2 ddsd;

	if (bChatInitialised) {
		VidSExitChat();
	} else {
//		for (int i = 0; i < CHAT_SIZE + (CHAT_SIZE >> 1); i++) {
		for (int i = 0; i < CHAT_SIZE; i++) {
			VidSChatMessage[i].pIDText = NULL;
			VidSChatMessage[i].pMainText = NULL;
		}
		bChatInitialised = true;
	}

	nChatFontSize = nMaxChatFontSize - ((nMaxChatFontSize - nMinChatFontSize) * nFlags) / 4;

	ChatIDFont = CreateFont(nChatFontSize, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, ANTIALIASED_QUALITY, FF_SWISS, _T("Lucida"));
	if (nChatFontSize > 20) {
		ChatMainFont = CreateFont(nChatFontSize, 0, 0, 0, FW_NORMAL, 0, 0, 0, 0, 0, 0, ANTIALIASED_QUALITY, FF_SWISS, _T("Lucida"));
	} else {
		ChatMainFont = CreateFont(nChatFontSize, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, ANTIALIASED_QUALITY, FF_SWISS, _T("Lucida"));
	}

	nChatShadowOffset = (nChatFontSize / 16) + 1;
	nChatOverFlow = 0;
	bDrawChat = false;

	// create surface to display the text
	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_CKSRCBLT;

	ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_VIDEOMEMORY;

	if (nVidFullscreen) {
		RECT rect;
		getClientScreenRect(hScrnWnd, &rect);
		ddsd.dwWidth = rect.right - rect.left;
	} else {
		extern RECT SystemWorkArea;
		ddsd.dwWidth = SystemWorkArea.right - SystemWorkArea.left;
	}
	ddsd.dwHeight = nChatFontSize * (CHAT_SIZE + (CHAT_SIZE >> 1));

	ddsd.ddckCKSrcBlt.dwColorSpaceLowValue = nKeyColour;
	ddsd.ddckCKSrcBlt.dwColorSpaceHighValue = nKeyColour;

	if (FAILED(pDD->CreateSurface(&ddsd, &pChatSurf, NULL))) {
#ifdef PRINT_DEBUG_INFO
		dprintf(_T("  * Error: Couldn't create Chat texture.\n"));
#endif
		return 1;
	}

	VidSClearSurface(pChatSurf, nKeyColour, NULL);

	bDrawChat = true;

	return 0;
}

static int VidSInitEdit(int nFlags)
{
	DDSURFACEDESC2 ddsd;

	VidSExitEdit();

	nEditSize = nMaxChatFontSize + 8 - ((nMaxChatFontSize - nMinChatFontSize) * nFlags) / 4;

	EditTextFont = CreateFont(nEditSize - 8, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, ANTIALIASED_QUALITY, FF_SWISS, _T("Lucida"));
	EditCursorFont = CreateFont(nEditSize - 8, 0, 0, 0, FW_BOLD, 0, TRUE, 0, 0, 0, 0, ANTIALIASED_QUALITY, FF_SWISS, _T("Lucida"));

	nEditShadowOffset = ((nEditSize - 8) / 16) + 1;

	// create surface to display the text
	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_CKSRCBLT;

	ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_VIDEOMEMORY;

	if (nVidFullscreen) {
		RECT rect;
		getClientScreenRect(hScrnWnd, &rect);
		ddsd.dwWidth = rect.right - rect.left;
	} else {
		extern RECT SystemWorkArea;
		ddsd.dwWidth = SystemWorkArea.right - SystemWorkArea.left;
	}
	ddsd.dwHeight = nEditSize;

	ddsd.ddckCKSrcBlt.dwColorSpaceLowValue = nKeyColour;
	ddsd.ddckCKSrcBlt.dwColorSpaceHighValue = nKeyColour;

	if (FAILED(pDD->CreateSurface(&ddsd, &pEditSurf, NULL))) {
#ifdef PRINT_DEBUG_INFO
		dprintf(_T("  * Error: Couldn't create OSD texture.\n"));
#endif
		return 1;
	}

	VidSClearSurface(pEditSurf, nKeyColour, NULL);

	return 0;
}

int VidSInitOSD(int nFlags)
{
#ifdef PRINT_DEBUG_INFO
//	dprintf(_T(" ** OSD initialised.\n"));
#endif

	if (pDD == NULL) {
		return 1;
	}

	if (VidSInitShortMsg(nFlags)) {
		return 1;
	}
	if (VidSInitChat(nFlags)) {
		return 1;
	}
	if (kNetGame) {
		if (VidSInitEdit(nFlags)) {
			return 1;
		}
	}
	if (VidSInitTinyMsg(nFlags)) {
		return 1;
	}

	return 0;
}

int VidSRestoreOSD()
{
	if (pShortMsgSurf) {
		if (FAILED(pShortMsgSurf->IsLost())) {
			if (FAILED(pShortMsgSurf->Restore())) {
				return 1;
			}
			VidSClearSurface(pShortMsgSurf, nKeyColour, NULL);

			VidSShortMsg.nTimer = 0;
		}
	}

	if (pTinyMsgSurf) {
		if (FAILED(pTinyMsgSurf->IsLost())) {
		if (FAILED(pTinyMsgSurf->Restore())) {
				return 1;
			}
			VidSClearSurface(pTinyMsgSurf, nKeyColour, NULL);

			VidSTinyMsg.nTimer = 0;
		}
	}

	if (pChatSurf) {
		if (FAILED(pChatSurf->IsLost())) {
			if (FAILED(pChatSurf->Restore())) {
				return 1;
			}
			VidSClearSurface(pChatSurf, nKeyColour, NULL);
		}
	}

	if (kNetGame) {
		if (pEditSurf) {
			if (FAILED(pEditSurf->IsLost())) {
				if (FAILED(pEditSurf->Restore())) {
					return 1;
				}
				VidSClearSurface(pEditSurf, nKeyColour, NULL);
			}
		}
	}

	return 0;
}

static void VidSDisplayTinyMsg(IDirectDrawSurface7* pSurf, RECT* pRect)
{
	if (VidSTinyMsg.nTimer) {
		RECT src = { 0, 0, tinyMsgWidth, 20 };
		RECT dest = { pRect->right - 320, pRect->bottom - 24, pRect->right - 8, pRect->bottom - 4 };

		// Switch off message display when the message has been displayed long enough
		if (nFramesEmulated > VidSTinyMsg.nTimer) {
			VidSTinyMsg.nTimer = 0;
		}

		if (dest.left < pRect->left) {
			src.left = pRect->left - dest.left;
			dest.left = pRect->left;
		}

		if (nZoom & 2) {
			dest.top <<= 1;
			dest.bottom <<= 1;
		}

		if (nZoom & 1) {
			dest.left <<= 1;
			dest.right <<= 1;
		}

		// Blit the message to the surface using a colourkey
		pSurf->Blt(&dest, pTinyMsgSurf, &src, DDBLT_ASYNC | DDBLT_KEYSRC, NULL);
	}
}

static void VidSDisplayShortMsg(IDirectDrawSurface7* pSurf, RECT* pRect)
{
	if (VidSShortMsg.nTimer) {
		RECT src = { 0, 0, shortMsgWidth, 32 };
		RECT dest = { 0, pRect->top + 4, pRect->right - 8, pRect->top + 36 };

		// Switch off message display when the message has been displayed long enough
		if (nFramesEmulated > VidSShortMsg.nTimer) {
			VidSShortMsg.nTimer = 0;
		}

		dest.left = dest.right - shortMsgWidth;
		if (dest.left < pRect->left) {
			src.left = pRect->left - dest.left;
			dest.left = pRect->left;
		}

		if (nZoom & 2) {
			dest.top <<= 1;
			dest.bottom <<= 1;
		}

		if (nZoom & 1) {
			dest.left <<= 1;
			dest.right <<= 1;
		}

		// Blit the message to the surface using a colourkey
		pSurf->Blt(&dest, pShortMsgSurf, &src, DDBLT_ASYNC | DDBLT_KEYSRC, NULL);
	}
}

static int VidSDrawChat(RECT* dest)
{
	if (pChatSurf == NULL) {
		return 1;
	}

	VidSClearSurface(pChatSurf, nKeyColour, NULL);

	bool bContent = false;

	for (int i = 0; i < CHAT_SIZE; i++) {
		if (VidSChatMessage[i].pIDText || VidSChatMessage[i].pMainText) {
			bContent = true;
			break;
		}
	}

	if (!bContent) {
		nChatTimer = 0;
		return 0;
	}

	nChatTimer = nFramesEmulated + 300;

	// Print the message
	HDC hDC;
	HFONT hFont;
	size_t nStringLength;
	int nFit = 0;
	SIZE sizeID, sizeMain;

	// Clear the surface first
	VidSClearSurface(pChatSurf, nKeyColour, NULL);

	pChatSurf->GetDC(&hDC);
	SetBkMode(hDC, TRANSPARENT);

	hFont = (HFONT)SelectObject(hDC, ChatIDFont);
	SetTextAlign(hDC, TA_TOP | TA_LEFT);

	nChatOverFlow = 0;

	for (int i = 0; i < CHAT_SIZE; i++) {
		// Exit when too many lines are displayed
		if (nChatOverFlow > (CHAT_SIZE >> 1)) {
			break;
		}

		if (VidSChatMessage[i].pIDText) {
			SelectObject(hDC, ChatIDFont);
			nStringLength =  _tcslen(VidSChatMessage[i].pIDText);
			MyExtTextOut(hDC, 0, (i + nChatOverFlow) * nChatFontSize, ETO_CLIPPED, dest, VidSChatMessage[i].pIDText, nStringLength, NULL, nChatShadowOffset, VidSChatMessage[i].nIDColour);
			GetTextExtentPoint32(hDC, VidSChatMessage[i].pIDText, nStringLength, &sizeID);
		} else {
			sizeID.cx = 0;
		}

		if (VidSChatMessage[i].pMainText) {
			SelectObject(hDC, ChatMainFont);
			nStringLength =  _tcslen(VidSChatMessage[i].pMainText);
			GetTextExtentExPoint(hDC, VidSChatMessage[i].pMainText, nStringLength, dest->right - sizeID.cx, &nFit, NULL, &sizeMain);

			if (nFit < nStringLength) {
				// The string doesn't fit on the screen in one line, break the line on the last space that fits
				while (nFit > 0 && (VidSChatMessage[i].pMainText[nFit] != ' ')) {
					nFit--;
				}

				nStringLength = nFit;
				MyExtTextOut(hDC, sizeID.cx, (i + nChatOverFlow) * nChatFontSize, ETO_CLIPPED, dest, VidSChatMessage[i].pMainText, nStringLength, NULL, nChatShadowOffset, VidSChatMessage[i].nMainColour);

				nChatOverFlow++;

				nStringLength =  _tcslen(&(VidSChatMessage[i].pMainText[nFit + 1]));
				MyExtTextOut(hDC, sizeID.cx, (i + nChatOverFlow) * nChatFontSize, ETO_CLIPPED, dest, &(VidSChatMessage[i].pMainText[nFit + 1]), nStringLength, NULL, nChatShadowOffset, VidSChatMessage[i].nMainColour);
			} else {
				SelectObject(hDC, ChatMainFont);
				MyTextOut(hDC, sizeID.cx, (i + nChatOverFlow) * nChatFontSize, VidSChatMessage[i].pMainText, nStringLength, nChatShadowOffset, VidSChatMessage[i].nMainColour);
			}
		}
	}

	// Clean up
	SelectObject(hDC, hFont);
	pChatSurf->ReleaseDC(hDC);

	return 0;
}

static void VidSDisplayChat(IDirectDrawSurface7* pSurf, RECT* pRect)
{
	if (nChatTimer || bDrawChat) {
		// Blit the message to the surface using a colourkey
		RECT src = { 0, 0, pRect->right - pRect->left, nChatFontSize * (CHAT_SIZE + (CHAT_SIZE >> 1)) };

		// Update the message if needed
		if (bDrawChat) {
			VidSDrawChat(&src);
			bDrawChat = false;
		}

		if (nChatTimer) {
			RECT dest = { pRect->left, pRect->bottom - nChatFontSize * (CHAT_SIZE + nChatOverFlow), pRect->right, pRect->bottom };
			src.bottom = nChatFontSize * (CHAT_SIZE + nChatOverFlow);

			if (bEditActive) {
				dest.top -= nEditSize;
				dest.bottom -= nEditSize;
			} else {
				dest.top -= 4;
				dest.bottom -= 4;
			}

			if (nZoom & 2) {
				dest.top <<= 1;
				dest.bottom <<= 1;
			}

			if (nZoom & 1) {
				dest.left <<= 1;
				dest.right <<= 1;
			}

			pSurf->Blt(&dest, pChatSurf, &src, DDBLT_ASYNC | DDBLT_KEYSRC, NULL);
		}

		// Scroll message if needed
		if (nFramesEmulated > nChatTimer) {
			nChatTimer = 0;
			VidSAddChatMsg(NULL, 0, NULL, 0);
			bDrawChat = true;
		}
	}
}

static void VidSDisplayEdit(IDirectDrawSurface7* pSurf, RECT* pRect)
{
	if (bEditActive) {
		RECT src = { 0, 0, pRect->right - pRect->left, nEditSize };
		RECT dest = { pRect->left, pRect->bottom - nEditSize, pRect->right, pRect->bottom };

		if (nZoom & 2) {
			dest.top <<= 1;
			dest.bottom <<= 1;
		}

		if (nZoom & 1) {
			dest.left <<= 1;
			dest.right <<= 1;
		}

		DWORD nStart, nEnd;
		SendMessage(GetChatWindow(), EM_GETSEL, (WPARAM)&nStart, (WPARAM)&nEnd);

		if (nStart != nPrevStart || nEnd != nPrevEnd) {
			nPrevStart = nStart;
			nPrevEnd = nEnd;
			bEditTextChanged = true;
			nCursorTime = nFramesEmulated + 30;
			nCursorState = 1;
		} else {
			if (GetCurrentFrame() > nCursorTime) {
				nCursorTime = nFramesEmulated + 30;
				nCursorState = !nCursorState;
				bEditTextChanged = true;
			}
		}

		if (bEditTextChanged) {
			if (pEditSurf == NULL) {
				return;
			}

			// Print the message
			HDC hDC;
			HFONT hFont;
			SIZE size;
			int pos;

			TCHAR Space[] = _T(" ");
			unsigned int nFit = 0;
			TCHAR* pText = EditText;

//			bool bDisplayCursor = false;
//			if (GetFocus()) {
//				bDisplayCursor = true;
//			}

			// Clear the surface first
			VidSClearSurface(pEditSurf, nKeyColour, NULL);

			pEditSurf->GetDC(&hDC);
			SetBkMode(hDC, TRANSPARENT);
			hFont = (HFONT)SelectObject(hDC, EditTextFont);
			SetTextAlign(hDC, TA_TOP | TA_LEFT);

			// If the string is wider than the screen, shift the display to the right.
			pos = 0;
			do {
				GetTextExtentExPoint(hDC, pText, _tcslen(pText), src.right - pos - nEditSize, (int*)&nFit, NULL, &size);
				if (nFit < nStart) {
					pos -= src.right * 3 / 5;
				}
			} while (nFit < nStart);

			if (nStart == _tcslen(pText)) {
				if (nStart) {
					MyExtTextOut(hDC, pos + 3, 3, ETO_CLIPPED, &src, pText, nStart, NULL, nEditShadowOffset, RGB(0xDF, 0xDF, 0xFF));
				}

				GetTextExtentPoint32(hDC, pText, nStart, &size);
				pos += size.cx;

				if (nCursorState) {
					SelectObject(hDC, EditCursorFont);
				}
				MyExtTextOut(hDC, pos + 3, 3, ETO_CLIPPED, &src, Space, 1, NULL, nEditShadowOffset, RGB(0xFF, 0xFF, 0xFF));
			} else {
				if (nStart) {
					MyExtTextOut(hDC, pos + 3, 3, ETO_CLIPPED, &src, pText, nStart, NULL, nEditShadowOffset, RGB(0xDF, 0xDF, 0xFF));
				}

				GetTextExtentPoint32(hDC, pText, nStart, &size);
				pos += size.cx;

				if (nEnd == nStart) {
					if (nCursorState) {
						SelectObject(hDC, EditCursorFont);
					}
					MyExtTextOut(hDC, pos + 3, 3, ETO_CLIPPED, &src, pText + nStart, 1, NULL, nEditShadowOffset, RGB(0xFF, 0xFF, 0xFF));

					GetTextExtentPoint32(hDC, pText + nStart, 1, &size);
					pos += size.cx;

					// We really printed one character past the end of the selection
					nEnd++;
				} else {
					// Highlight selected characters
					SelectObject(hDC, EditTextFont);
					// SetTextColor(hDC, RGB(0x7F, 0x1F, 0x1F));
					MyExtTextOut(hDC, pos + 3, 3, ETO_CLIPPED, &src, pText + nStart, nEnd - nStart, NULL, nEditShadowOffset, RGB(0xFF, 0xFF, 0xDF));

					GetTextExtentPoint32(hDC, pText + nStart, nEnd - nStart, &size);
					pos += size.cx;
				}

				if (nEnd < _tcslen(EditText)) {
					SelectObject(hDC, EditTextFont);
					MyExtTextOut(hDC, pos + 3, 3, ETO_CLIPPED, &src, pText + nEnd, _tcslen(pText) - nEnd, NULL, nEditShadowOffset, RGB(0xDF, 0xDF, 0xFF));
				}
			}

			// Clean up
			SelectObject(hDC, hFont);
			pEditSurf->ReleaseDC(hDC);

			bEditTextChanged = false;
		}

		// Blit the message to the surface using a colourkey
		pSurf->Blt(&dest, pEditSurf, &src, DDBLT_ASYNC | DDBLT_KEYSRC, NULL);
	}
}

void VidSDisplayOSD(IDirectDrawSurface7* pSurf, RECT* pRect, int nFlags)
{
	nZoom = nFlags & 3;

	VidSDisplayTinyMsg(pSurf, pRect);
	VidSDisplayShortMsg(pSurf, pRect);
	VidSDisplayChat(pSurf, pRect);
	if (kNetGame) {
		VidSDisplayEdit(pSurf, pRect);
	}
}

int VidSNewTinyMsg(const TCHAR* pText, int nRGB, int nDuration, int nPriority)	// int nRGB = 0, int nDuration = 0, int nPriority = 5
{
	// If a message with a higher priority is being displayed, exit.
	if (VidSTinyMsg.nTimer && VidSTinyMsg.nPriority > nPriority) {
		return 1;
	}

	size_t nSize = _tcslen(pText);
	if (nSize >= MESSAGE_SIZE) {
		nSize = MESSAGE_SIZE - 1;
	}
	_tcsncpy(VidSTinyMsg.pMsgText, pText, nSize);
	VidSTinyMsg.pMsgText[nSize] = 0;

	// copy tiny message
	memset(tinyMsg, '\0', sizearray(tinyMsg));
	_tcsncpy(tinyMsg, pText, nSize);

	if (nRGB) {
		// Convert RGB value to COLORREF
		VidSTinyMsg.nColour = RGB((nRGB >> 16), ((nRGB >> 8) & 0xFF), (nRGB & 0xFF));
	} else {
		// Default message colour (yellow)
		VidSTinyMsg.nColour = RGB(0xFF, 0xFF, 0x7F);
	}
	if (nDuration) {
		VidSTinyMsg.nTimer = nFramesEmulated + nDuration;
	} else {
		VidSTinyMsg.nTimer = nFramesEmulated + 120;
	}
	nTinyTimer = VidSTinyMsg.nTimer;
	VidSTinyMsg.nPriority = nPriority;

	{
		if (pTinyMsgSurf == NULL) {
			return 1;
		}

		// Print the message
		HDC hDC;
		HFONT hFont;

		// Clear the surface first
		VidSClearSurface(pTinyMsgSurf, nKeyColour, NULL);

		pTinyMsgSurf->GetDC(&hDC);
		SetBkMode(hDC, TRANSPARENT);
		hFont = (HFONT)SelectObject(hDC, TinyMsgFont);
		SetTextAlign(hDC, TA_BOTTOM | TA_RIGHT);

		// Print a black shadow
		SetTextColor(hDC, 0);
		TextOut(hDC, tinyMsgWidth, 20, VidSTinyMsg.pMsgText, _tcslen(VidSTinyMsg.pMsgText));
		// Print the text on top
		SetTextColor(hDC, VidSTinyMsg.nColour);
		TextOut(hDC, tinyMsgWidth - 1, 20 - 1, VidSTinyMsg.pMsgText, _tcslen(VidSTinyMsg.pMsgText));

		// Clean up
		SelectObject(hDC, hFont);
		pTinyMsgSurf->ReleaseDC(hDC);
	}

	return 0;
 }

int VidSNewShortMsg(const TCHAR* pText, int nRGB, int nDuration, int nPriority)	// int nRGB = 0, int nDuration = 0, int nPriority = 5
{
	// If a message with a higher priority is being displayed, exit.
	if (VidSShortMsg.nTimer && VidSShortMsg.nPriority > nPriority) {
		return 1;
	}

	size_t nSize = _tcslen(pText);
	if (nSize >= MESSAGE_SIZE) {
		nSize = MESSAGE_SIZE - 1;
	}
	_tcsncpy(VidSShortMsg.pMsgText, pText, nSize);
	VidSShortMsg.pMsgText[nSize] = 0;

	// copy osd message
	memset(OSDMsg, '\0', sizearray(OSDMsg));
	_tcsncpy(OSDMsg, pText, nSize);

	if (nRGB) {
		// Convert RGB value to COLORREF
		VidSShortMsg.nColour = RGB((nRGB >> 16), ((nRGB >> 8) & 0xFF), (nRGB & 0xFF));
	} else {
		// Default message colour (yellow)
		VidSShortMsg.nColour = RGB(0xFF, 0xFF, 0x7F);
	}
	if (nDuration) {
		VidSShortMsg.nTimer = nFramesEmulated + nDuration;
	} else {
		VidSShortMsg.nTimer = nFramesEmulated + 120;
	}
	nOSDTimer = VidSShortMsg.nTimer;
	VidSShortMsg.nPriority = nPriority;

	{
		if (pShortMsgSurf == NULL) {
			return 1;
		}

		// Print the message
		HDC hDC;
		HFONT hFont;

		// Clear the surface first
		VidSClearSurface(pShortMsgSurf, nKeyColour, NULL);

		pShortMsgSurf->GetDC(&hDC);
		SetBkMode(hDC, TRANSPARENT);
		hFont = (HFONT)SelectObject(hDC, ShortMsgFont);
		SetTextAlign(hDC, TA_TOP | TA_RIGHT);

		MyTextOut(hDC, shortMsgWidth - 2, 0, VidSShortMsg.pMsgText, _tcslen(VidSShortMsg.pMsgText), 2, VidSShortMsg.nColour);

		// Clean up
		SelectObject(hDC, hFont);
		pShortMsgSurf->ReleaseDC(hDC);
	}

	return 0;
}

void VidSKillShortMsg()
{
	VidSShortMsg.nTimer = 0;
}

void VidSKillTinyMsg()
{
	VidSTinyMsg.nTimer = 0;
}

void VidSKillOSDMsg()
{
	nOSDTimer = 0;
}

void VidSKillTinyOSDMsg()
{
	nTinyTimer = 0;
}

int VidSAddChatMsg(const TCHAR* pID, int nIDRGB, const TCHAR* pMain, int nMainRGB)
{
	if (pID || pMain) {
		// Scroll the text buffers up one entry
		free(VidSChatMessage[0].pIDText);
		VidSChatMessage[0].pIDText = NULL;
		free(VidSChatMessage[0].pMainText);
		VidSChatMessage[0].pMainText = NULL;

		for (int i = 1; i < CHAT_SIZE; i++) {
			VidSChatMessage[i - 1].pIDText = VidSChatMessage[i].pIDText;
			VidSChatMessage[i - 1].nIDColour = VidSChatMessage[i].nIDColour;
			VidSChatMessage[i - 1].pMainText = VidSChatMessage[i].pMainText;
			VidSChatMessage[i - 1].nMainColour = VidSChatMessage[i].nMainColour;
		}

		// Put the new entry in the last position
		if (pID) {
			size_t nSize = _tcslen(pID);
			VidSChatMessage[CHAT_SIZE - 1].pIDText = (TCHAR*)malloc((nSize + 1) * sizeof(TCHAR));
			 _tcscpy(VidSChatMessage[CHAT_SIZE - 1].pIDText, pID);
			VidSChatMessage[CHAT_SIZE - 1].pIDText[nSize] = 0;
			VidSChatMessage[CHAT_SIZE - 1].nIDColour = RGB((nIDRGB >> 16), ((nIDRGB >> 8) & 0xFF), (nIDRGB & 0xFF));
		} else {
			VidSChatMessage[CHAT_SIZE - 1].pIDText = NULL;
		}
		if (pMain) {
			size_t nSize = _tcslen(pMain);
			VidSChatMessage[CHAT_SIZE - 1].pMainText = (TCHAR*)malloc((nSize + 1) * sizeof(TCHAR));
			 _tcscpy(VidSChatMessage[CHAT_SIZE - 1].pMainText, pMain);
			VidSChatMessage[CHAT_SIZE - 1].pMainText[nSize] = 0;
			VidSChatMessage[CHAT_SIZE - 1].nMainColour = RGB((nMainRGB >> 16), ((nMainRGB >> 8) & 0xFF), (nMainRGB & 0xFF));
		} else {
			VidSChatMessage[CHAT_SIZE - 1].pMainText = NULL;
		}
	} else {
		for (int i = 0; i < CHAT_SIZE; i++) {
			if (VidSChatMessage[i].pIDText || VidSChatMessage[i].pMainText) {
				free(VidSChatMessage[i].pIDText);
				VidSChatMessage[i].pIDText = NULL;
				free(VidSChatMessage[i].pMainText);
				VidSChatMessage[i].pMainText = NULL;
				break;
			}
		}
	}

	bDrawChat = true;

	return 0;
}
