// Video Output - (calls all the Vid Out plugins)
#include "burner.h"
#include "highcol.h"
#include "vid_psgl.h"

#undef ENABLE_PREVIEW
bool bVidOkay = false;

int nVidDepth = 16;
int nVidRefresh = 0;
int nVidFullscreen = 0;
int bVidFullStretch = 0;					// 1 = stretch to fill the entire window/screen
int bVidCorrectAspect = 1;					// 1 = stretch to fill the window/screen while maintaining the correct aspect ratio
int bVidVSync = 0;						// 1 = sync blits/pageflips/presents to the screen
int bVidTripleBuffer = 0;					// 1 = use triple buffering
int nVidRotationAdjust = 0;					// & 1: do not rotate the graphics for vertical games,  & 2: Reverse flipping for vertical games
int bVidForce16bit = 1;						// Emulate the game in 16-bit even when the screen is 32-bit (D3D blitter)
unsigned int vidFilterLinear = 0;				// 1 = linear filter, or point filter
int nVidOriginalScrnAspectX;
int nVidOriginalScrnAspectY;

int nVidScrnWidth = 0, nVidScrnHeight = 0;			// Actual Screen dimensions (0 if in windowed mode)
int nVidScrnDepth = 0;						// Actual screen depth

int nVidScrnAspectX = 4, nVidScrnAspectY = 3;			// Aspect ratio of the display screen
int nVidScrnAspectMode = ASPECT_RATIO_4_3;
float vidScrnAspect = (float)4 / (float)3;			// Aspect ratio
extern bool autoVidScrnAspect = true;				// Automatical Aspect ratio

unsigned char* pVidImage = NULL;				// Memory buffer
int nVidImageWidth = DEFAULT_IMAGE_WIDTH;			// Memory buffer size
int nVidImageHeight = DEFAULT_IMAGE_HEIGHT;			//
int nVidImageLeft = 0, nVidImageTop = 0;			// Memory buffer visible area offsets
int nVidImagePitch = 0, nVidImageBPP = 0;			// Memory buffer pitch and bytes per pixel
int nVidImageDepth = 0;						// Memory buffer bits per pixel

unsigned int (__cdecl *VidHighCol) (int r, int g, int b, int i);
bool bVidRecalcPalette;

unsigned char* pVidTransImage = NULL;
static unsigned int* pVidTransPalette = NULL;
const int transPaletteSize = 65536;

int nXOffset = 0;
int nYOffset = 0;
int nXScale = 0;
int nYScale = 0;

static unsigned int __cdecl HighCol15(int r, int g, int b, int  /* i */)
{
	unsigned int t;
	t  = (r << 7) & 0x7C00;
	t |= (g << 2) & 0x03E0;
	t |= (b >> 3) & 0x001F;
	return t;
}

// Forward to VidOut functions
int VidInit()
{
	VidExit();

	int nRet = 1;

	if (bDrvOkay)
	{
		if (_psglInit() == 0)
		{
			nBurnBpp = nVidImageBPP; // Set Burn library Bytes per pixel
			bVidOkay = true;

			if (bDrvOkay && (BurnDrvGetFlags() & BDF_16BIT_ONLY) && nVidImageBPP > 2)
			{
				nBurnBpp = 2;

				pVidTransPalette = (unsigned int*)malloc(transPaletteSize * sizeof(int));
				pVidTransImage = (unsigned char*)malloc(nVidImageWidth * nVidImageHeight * (nVidImageBPP >> 1) * sizeof(short));

				BurnHighCol = HighCol15;

				if (pVidTransPalette == NULL || pVidTransImage == NULL)
				{
					VidExit();
					nRet = 1;
				}
			}
		}
	}

	return nRet;
}

int VidExit()
{
	if (!bVidOkay)
		return 1;

	int nRet = _psglExit();

	bVidOkay = false;

	nVidImageWidth = DEFAULT_IMAGE_WIDTH;
	nVidImageHeight = DEFAULT_IMAGE_HEIGHT;

	nVidImageBPP = nVidImageDepth = 0;
	nBurnPitch = nBurnBpp = 0;

	free(pVidTransPalette);
	pVidTransPalette = NULL;
	free(pVidTransImage);
	pVidTransImage = NULL;

	return nRet;
}

int VidFrame_RecalcPalette()
{
	unsigned short* pSrc = (unsigned short*)pVidTransImage;
	unsigned char* pDest = pVidImage;

	uint64_t r = 0;
	do{
		uint64_t g = 0;
		do{
			uint64_t b = 0;
			do{
				uint64_t r_ = r | (r >> 5);
				uint64_t g_ = g | (g >> 5);
				uint64_t b_ = b | (b >> 5);
				pVidTransPalette[(r << 7) | (g << 2) | (b >> 3)] = ARGB(r_,g_,b_);
				b += 8;
			}while(b < 256);
			g += 8;
		}while(g < 256);
		r += 8;
	}while(r < 256);
	bVidRecalcPalette = false;

	pBurnDraw = pVidTransImage;
	nBurnPitch = nVidImageWidth << 1;
	BurnDrvFrame();
	_psglRender();

	/* set avi buffer, modified by regret */

	pBurnDraw = NULL;
	nBurnPitch = 0;
	int y = 0;
	do{
		int x = 0;

		do{
			((unsigned int*)pDest)[x] = pVidTransPalette[pSrc[x]];
			x++;
		}while(x < nVidImageWidth);

		y++;
		pSrc += nVidImageWidth;
		pDest += nVidImagePitch;
	}while(y < nVidImageHeight);

	return 0;
}

int VidFrame_Recalc()
{
	unsigned short* pSrc = (unsigned short*)pVidTransImage;
	unsigned char* pDest = pVidImage;

	pBurnDraw = pVidTransImage;
	nBurnPitch = nVidImageWidth << 1;
	BurnDrvFrame();
	_psglRender();

	/* set avi buffer, modified by regret */

	pBurnDraw = NULL;
	nBurnPitch = 0;
	int y = 0;
	do{
		int x = 0;

		do{
			((unsigned int*)pDest)[x] = pVidTransPalette[pSrc[x]];
			x++;
		}while(x < nVidImageWidth);

		y++;
		pSrc += nVidImageWidth;
		pDest += nVidImagePitch;
	}while(y < nVidImageHeight);

	return 0;
}

int VidFrame()
{
	pBurnDraw = pVidImage;
	nBurnPitch = nVidImagePitch;
	BurnDrvFrame();
	_psglRender();
	/* set avi buffer, modified by regret */
	pBurnDraw = NULL;
	nBurnPitch = 0;

	return 0;
}

int VidRecalcPal()
{
	bVidRecalcPalette = true;

	return BurnRecalcPal();
}

// reinit video, added by regret

int VidReinit()
{
	VidInit();

	if (bRunPause || !bDrvOkay)
			VidFrame();

	CalculateViewports();

	if(pVidTransImage)
	{
		nCurrentFrame++;
		VidFrame_RecalcPalette();
	}
	return 0;
}
