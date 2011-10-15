// Video Output - (calls all the Vid Out plugins)
#include "burner.h"

#define DEFAULT_IMAGE_WIDTH (304)
#define DEFAULT_IMAGE_HEIGHT (224)

#define ENABLE_PREVIEW

#if defined (BUILD_WIN32)
	extern struct VidOut VidOutDDraw;
	extern struct VidOut VidOutD3D;
	extern struct VidOut VidOutDDrawFX;
	extern struct VidOut VidOutDX9;
#elif defined (BUILD_SDL)
	extern struct VidOut VidOutSDLOpenGL;
	extern struct VidOut VidOutSDLFX;
#endif

static struct VidOut *pVidOut[] = {
#if defined (BUILD_WIN32)
	&VidOutDDraw,
	&VidOutD3D,
	&VidOutDDrawFX,
	&VidOutDX9,
#elif defined (BUILD_SDL)
	&VidOutSDLOpenGL,
	&VidOutSDLFX,
#endif
};

#define VID_LEN (sizeof(pVidOut) / sizeof(pVidOut[0]))

long long nVidBlitterOpt[VID_LEN] = {0, };			// Options for the blitter module (meaning depens on module)

static InterfaceInfo VidInfo = { NULL, NULL, NULL };

unsigned int nVidSelect = 0;					// Which video output is selected
static unsigned int nVidActive = 0;

bool bVidOkay = false;

int nVidWidth		= 640, nVidHeight		= 480, nVidDepth = 16, nVidRefresh = 0;

int nVidHorWidth	= 640, nVidHorHeight	= 480;	// Default Horizontal oritated resolution
int nVidVerWidth	= 640, nVidVerHeight	= 480;	// Default Vertical oriented resoultion

int nVidFullscreen = 0;
int bVidFullStretch = 0;						// 1 = stretch to fill the entire window/screen
int bVidCorrectAspect = 1;						// 1 = stretch to fill the window/screen while maintaining the correct aspect ratio
int bVidVSync = 0;								// 1 = sync blits/pageflips/presents to the screen
int bVidTripleBuffer = 0;						// 1 = use triple buffering
int bVidBilinear = 1;							// 1 = enable bi-linear filtering (D3D blitter)
int bVidScanlines = 0;							// 1 = draw scanlines
int bVidScanRotate = 1;							// 1 = rotate scanlines and RGB effects for rotated games
int bVidScanBilinear = 1;						// 1 = use bi-linear filtering for scanlines (D3D blitter, debug variable)
int nVidScanIntensity = 0x00BFBFBF;				// The maximum colour-value for the scanlines (D3D blitter)
int bVidScanHalf = 1;							// Draw scanlines at half intensity instead of black (DDraw blitter)
int bVidScanDelay = 0;							// Blend the previous image with the current one
int nVidFeedbackIntensity = 0x40;				// Blend factor for previous frame (D3D blitter)
int nVidFeedbackOverSaturation = 0x00;			// Add this to the current frame blend factor
int bVidUseHardwareGamma = 1;					// Use the video hardware when correcting gamma
int bVidAutoSwitchFull = 0;						// 1 = auto switch to fullscreen on loading driver
int bVidArcaderes = 0;							// Use game resolution for fullscreen modes

int bVidArcaderesHor = 0;
int bVidArcaderesVer = 0;

int nVidRotationAdjust = 0;						// & 1: do not rotate the graphics for vertical games,  & 2: Reverse flipping for vertical games
int bVidForce16bit = 1;							// Emulate the game in 16-bit even when the screen is 32-bit (D3D blitter)
int nVidTransferMethod = -1;					// How to transfer the game image to video memory and/or a texture --
												//  0 = blit from system memory / use driver/DirectX texture management
												//  1 = copy to a video memory surface, then use bltfast()
												// -1 = autodetect for ddraw, equals 1 for d3d
float fVidScreenAngle = 0.174533f;				// The angle at which to tilt the screen backwards (in radians, D3D blitter)
float fVidScreenCurvature = 0.698132f;			// The angle of the maximum screen curvature (in radians, D3D blitter)
double dVidCubicB = 0.0;						// Paremeters for the cubic filter (default is the CAtmull-Rom spline, DX9 blitter)
double dVidCubicC = 0.5;						//

#ifdef BUILD_WIN32
 HWND hVidWnd = NULL;							// Actual window used for video
#endif

int nVidScrnWidth = 0, nVidScrnHeight = 0;		// Actual Screen dimensions (0 if in windowed mode)
int nVidScrnDepth = 0;							// Actual screen depth

int nVidScrnAspectX = 4, nVidScrnAspectY = 3;	// Aspect ratio of the display screen

unsigned char* pVidImage = NULL;				// Memory buffer
int nVidImageWidth = DEFAULT_IMAGE_WIDTH;		// Memory buffer size
int nVidImageHeight = DEFAULT_IMAGE_HEIGHT;		//
int nVidImageLeft = 0, nVidImageTop = 0;		// Memory buffer visible area offsets
int nVidImagePitch = 0, nVidImageBPP = 0;		// Memory buffer pitch and bytes per pixel
int nVidImageDepth = 0;							// Memory buffer bits per pixel

unsigned int (__cdecl *VidHighCol) (int r, int g, int b, int i);
static bool bVidRecalcPalette;

static unsigned char* pVidTransImage = NULL;
static unsigned int* pVidTransPalette = NULL;

static unsigned int __cdecl HighCol15(int r, int g, int b, int  /* i */)
{
	unsigned int t;

	t  = (r << 7) & 0x7C00;
	t |= (g << 2) & 0x03E0;
	t |= (b >> 3) & 0x001F;

	return t;
}

int VidSelect(unsigned int nPlugin)
{
	if (nPlugin < VID_LEN) {
		nVidSelect = nPlugin;
		return 0;
	} else {
		return 1;
	}
}

// Forward to VidOut functions
int VidInit()
{
#if defined (BUILD_WIN32) && defined (ENABLE_PREVIEW)
	HBITMAP hbitmap = NULL;
	BITMAP bitmap;
#endif

	int nRet = 1;

	VidExit();

#if defined (BUILD_WIN32) && defined (ENABLE_PREVIEW)
	if (!bDrvOkay) {
		//hbitmap = (HBITMAP)LoadImage(hAppInst, _T("BMP_SPLASH"), IMAGE_BITMAP, 304, 224, 0);
		hbitmap = (HBITMAP)LoadImage(hAppInst, MAKEINTRESOURCE(BMP_SPLASH), IMAGE_BITMAP, 304, 224, 0);
		
		GetObject(hbitmap, sizeof(BITMAP), &bitmap);

		nVidImageWidth = bitmap.bmWidth; nVidImageHeight = bitmap.bmHeight;
		nVidImageLeft = nVidImageTop = 0;
	}
#endif

#if defined (BUILD_WIN32) && defined (ENABLE_PREVIEW)
	if ((nVidSelect < VID_LEN) && (bDrvOkay || hbitmap)) {
#else
	if ((nVidSelect < VID_LEN) && bDrvOkay) {
#endif
		nVidActive = nVidSelect;
		if ((nRet = pVidOut[nVidActive]->Init()) == 0) {
			nBurnBpp = nVidImageBPP;								// Set Burn library Bytes per pixel

			bVidOkay = true;

			if (bDrvOkay && (BurnDrvGetFlags() & BDF_16BIT_ONLY) && nVidImageBPP > 2) {
				nBurnBpp = 2;

				pVidTransPalette = (unsigned int*)malloc(32768 * sizeof(int));
				pVidTransImage = (unsigned char*)malloc(nVidImageWidth * nVidImageHeight * sizeof(short));

				BurnHighCol = HighCol15;

				if (pVidTransPalette == NULL || pVidTransImage == NULL) {
					VidExit();
					nRet = 1;
				}
			}
		}
	}

#if defined (BUILD_WIN32) && defined (ENABLE_PREVIEW)
	if (bVidOkay && hbitmap) {
		BITMAPINFO bitmapinfo;
		unsigned char* pLineBuffer = (unsigned char*)malloc(bitmap.bmWidth * 4);
		HDC hDC = GetDC(hVidWnd);

		if (hDC && pLineBuffer) {

			memset(&bitmapinfo, 0, sizeof(BITMAPINFO));
			bitmapinfo.bmiHeader.biSize = sizeof(BITMAPINFO);
			bitmapinfo.bmiHeader.biWidth = bitmap.bmWidth;
			bitmapinfo.bmiHeader.biHeight = bitmap.bmHeight;
			bitmapinfo.bmiHeader.biPlanes = 1;
			bitmapinfo.bmiHeader.biBitCount = 24;
			bitmapinfo.bmiHeader.biCompression = BI_RGB;
			
			for (int y = 0; y < nVidImageHeight; y++) {
				unsigned char* pd = pVidImage + y * nVidImagePitch;
				unsigned char* ps = pLineBuffer;

				GetDIBits(hDC, hbitmap, nVidImageHeight - 1 - y, 1, ps, &bitmapinfo, DIB_RGB_COLORS);

				for (int x = 0; x < nVidImageWidth; x++, ps += 3) {
					unsigned int nColour = VidHighCol(ps[2], ps[1], ps[0], 0);
					switch (nVidImageBPP) {
						case 2:
							*((unsigned short*)pd) = (unsigned short)nColour;
							pd += 2;
							break;
						case 3:
							pd[0] = (nColour >> 16) & 0xFF;
							ps[1] = (nColour >>  8) & 0xFF;
							pd[2] = (nColour >>  0) & 0xFF;
							pd += 3;
							break;
						case 4:
							*((unsigned int*)pd) = nColour;
							pd += 4;
							break;
					}
				}
			}
		}
		if (hDC) {
			ReleaseDC(hVidWnd, hDC);
		}
		free(pLineBuffer);
	}

	if (hbitmap) {
		DeleteObject(hbitmap);
	}
#endif

	return nRet;
}

int VidExit()
{
	IntInfoFree(&VidInfo);

	if (bVidOkay) {
		int nRet = pVidOut[nVidActive]->Exit();

#if defined (BUILD_WIN32)
		hVidWnd = NULL;
#endif

		bVidOkay = false;

		nVidImageWidth = DEFAULT_IMAGE_WIDTH; nVidImageHeight = DEFAULT_IMAGE_HEIGHT;

		nVidImageBPP = nVidImageDepth = 0;
		nBurnPitch = nBurnBpp = 0;

		free(pVidTransPalette);
		pVidTransPalette = NULL;
		free(pVidTransImage);
		pVidTransImage = NULL;

		return nRet;
	} else {
		return 1;
	}
}

static int VidDoFrame(bool bRedraw)
{
	int nRet;
	
	if (pVidTransImage) {
		unsigned short* pSrc = (unsigned short*)pVidTransImage;
		unsigned char* pDest = pVidImage;

		if (bVidRecalcPalette) {
			for (int r = 0; r < 256; r += 8) {
				for (int g = 0; g < 256; g += 8) {
					for (int b = 0; b < 256; b += 8) {
						pVidTransPalette[(r << 7) | (g << 2) | (b >> 3)] = VidHighCol(r | (r >> 5), g | (g >> 5), b | (b >> 5), 0);
					}
				}
			}

			bVidRecalcPalette = false;
		}

		pBurnDraw = pVidTransImage;
		nBurnPitch = nVidImageWidth * 2;

		nRet = pVidOut[nVidActive]->Frame(bRedraw);

		pBurnDraw = NULL;
		nBurnPitch = 0;

		switch (nVidImageBPP) {
			case 3: {
				for (int y = 0; y < nVidImageHeight; y++, pSrc += nVidImageWidth, pDest += nVidImagePitch) {
					for (int x = 0; x < nVidImageWidth; x++) {
						unsigned int c = pVidTransPalette[pSrc[x]];
						*(pDest + (x * 3) + 0) = c & 0xFF;
						*(pDest + (x * 3) + 1) = (c >> 8) & 0xFF;
						*(pDest + (x * 3) + 2) = c >> 16;
					}
				}
				break;
			}
			case 4: {
				for (int y = 0; y < nVidImageHeight; y++, pSrc += nVidImageWidth, pDest += nVidImagePitch) {
					for (int x = 0; x < nVidImageWidth; x++) {
						((unsigned int*)pDest)[x] = pVidTransPalette[pSrc[x]];
					}
				}
				break;
			}
		}
	} else {
		pBurnDraw = pVidImage;
		nBurnPitch = nVidImagePitch;

		nRet = pVidOut[nVidActive]->Frame(bRedraw);

		pBurnDraw = NULL;
		nBurnPitch = 0;
	}

	return nRet;
}

int VidReInitialise()
{
	free(pVidTransImage);
	pVidTransImage = (unsigned char*)malloc(nVidImageWidth * nVidImageHeight * sizeof(short));
	
	return 0;
}

int VidFrame()
{
	if (bVidOkay && bDrvOkay) {
		return VidDoFrame(0);
	} else {
		return 1;
	}
}

int VidRedraw()
{
	if (bVidOkay /* && bDrvOkay */) {
		return VidDoFrame(1);
	} else {
		return 1;
	}
}

int VidRecalcPal()
{
	bVidRecalcPalette = true;

	return BurnRecalcPal();
}

// If bValidate & 1, the video code should use ValidateRect() to validate the rectangle it draws.
int VidPaint(int bValidate)
{
	if (bVidOkay /* && bDrvOkay */) {
		return pVidOut[nVidActive]->Paint(bValidate);
	} else {
		return 1;
	}
}

int VidImageSize(RECT* pRect, int nGameWidth, int nGameHeight)
{
 	if (bVidOkay) {
		return pVidOut[nVidActive]->ImageSize(pRect, nGameWidth, nGameHeight);
	} else {
		return pVidOut[nVidSelect]->ImageSize(pRect, nGameWidth, nGameHeight);
	}
}

const TCHAR* VidGetModuleName()
{
	const TCHAR* pszName = NULL;

	if (bVidOkay) {
		pszName = pVidOut[nVidActive]->szModuleName;
	} else {
		pszName = pVidOut[nVidSelect]->szModuleName;
	}

	if (pszName) {
		return pszName;
	}

	return FBALoadStringEx(hAppInst, IDS_ERR_UNKNOWN, true);
}

InterfaceInfo* VidGetInfo()
{
	if (IntInfoInit(&VidInfo)) {
		IntInfoFree(&VidInfo);
		return NULL;
	}

	if (bVidOkay) {
		TCHAR szString[MAX_PATH] = _T("");
		RECT rect;

		VidInfo.pszModuleName = pVidOut[nVidActive]->szModuleName;

#if defined (BUILD_WIN32)
		GetClientScreenRect(hVidWnd, &rect);
		if (nVidFullscreen == 0) {
			rect.top += nMenuHeight;
			_sntprintf(szString, MAX_PATH, _T("Running in windowed mode, $ix%i, %ibpp"), rect.right - rect.left, rect.bottom - rect.top, nVidScrnDepth);
		} else {
			_sntprintf(szString, MAX_PATH, _T("Running fullscreen, $ix$i, %ibpp"), nVidScrnWidth, nVidScrnHeight, nVidScrnDepth);
		}
#elif defined (BUILD_SDL)
		_sntprintf(szString, MAX_PATH, _T("Filler for fullscreen/windowed mode & image size"));
#endif

		IntInfoAddStringInterface(&VidInfo, szString);

		_sntprintf(szString, MAX_PATH, _T("Source image %ix%i, %ibpp"), nVidImageWidth, nVidImageHeight, nVidImageDepth);
		IntInfoAddStringInterface(&VidInfo, szString);

		if (pVidTransImage) {
			_sntprintf(szString, MAX_PATH, _T("Using generic software 15->%ibpp wrapper"), nVidImageDepth);
			IntInfoAddStringInterface(&VidInfo, szString);
		}

	 	if (pVidOut[nVidActive]->GetPluginSettings) {
			pVidOut[nVidActive]->GetPluginSettings(&VidInfo);
		}
	} else {
		IntInfoAddStringInterface(&VidInfo, _T("Video plugin not initialised"));
	}

	return &VidInfo;
}
