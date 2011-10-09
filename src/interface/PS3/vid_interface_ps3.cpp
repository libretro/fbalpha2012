#include "burner.h"
#include "highcol.h"
#include "vid_support-ps3.h"
#include "vid_psgl.h"

bool bVidOkay = false;
int bVidVSync = 1;				// 1 = sync blits/pageflips/presents to the screen
int bVidTripleBuffer = 1;			// 1 = use triple buffering
int nVidRotationAdjust = 0;			// & 1: do not rotate the graphics for vertical games,  & 2: Reverse flipping for vertical games
unsigned int vidFilterLinear = 0;		// for shader #1 - 1 = linear filter, 0 = point filter
unsigned int vidFilterLinear2 = 0;		// for shader #2 - 1 = linear filter, 0 = point filter
int nVidOriginalScrnAspectX;
int nVidOriginalScrnAspectY;
int nVidScrnAspectX = 4;			// Aspect ratio (X-axis)of the display screen
int nVidScrnAspectY = 3;			// Aspect ratio (Y-axis) of the display screen
int nVidScrnAspectMode = ASPECT_RATIO_4_3;
float vidScrnAspect = ASPECT_RATIO_4_3;		// Aspect ratio
unsigned char* pVidImage = NULL;		// Memory buffer
int nVidImageWidth = DEFAULT_IMAGE_WIDTH;	// Memory buffer size
int nVidImageHeight = DEFAULT_IMAGE_HEIGHT;
int nVidImageLeft = 0, nVidImageTop = 0;	// Memory buffer visible area offsets
int nVidImagePitch = 0;				// Memory buffer pitch
bool bVidRecalcPalette;
bool bVidFBOEnabled;				// FBO/Dual shader mode
unsigned int bVidScalingFactor = 1;		// Scaling factor for use with FBO mode
unsigned char* pVidTransImage = NULL;
static unsigned int* pVidTransPalette = NULL;
const int transPaletteSize = 65536;
int nXOffset = 0;
int nYOffset = 0;
int nXScale = 0;
int nYScale = 0;

unsigned int (__cdecl *VidHighCol) (int r, int g, int b, int i);

unsigned int __cdecl HighCol15(int r, int g, int b, int  /* i */)
{
	unsigned int t;
	t  = (r << 7) & 0x7C00;
	t |= (g << 2) & 0x03E0;
	t |= (b >> 3) & 0x001F;
	return t;
}

unsigned int __cdecl HighCol16(int r, int g, int b, int /* i */)
{
	unsigned int t;
	t  = (r << 8) & 0xf800; // rrrr r000 0000 0000
	t |= (g << 3) & 0x07e0; // 0000 0ggg ggg0 0000
	t |= (b >> 3) & 0x001f; // 0000 0000 000b bbbb
	return t;
}

unsigned int __cdecl HighCol24(int r, int g, int b, int  /* i */)
{
	unsigned int t;
	t  = (r << 16) & 0xff0000;
	t |= (g << 8 ) & 0x00ff00;
	t |= (b      ) & 0x0000ff;
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
			nBurnBpp = SCREEN_RENDER_TEXTURE_BPP; // Set Burn library Bytes per pixel
			bVidOkay = true;

			if (bDrvOkay && (BurnDrvGetFlags() & BDF_16BIT_ONLY) && nBurnBpp > 2)
			{
				nBurnBpp = BPP_16_SCREEN_RENDER_TEXTURE_BPP;

				pVidTransPalette = (unsigned int*)memalign(128, transPaletteSize * sizeof(int));
				pVidTransImage = (unsigned char*)memalign(128, nVidImageWidth * nVidImageHeight * (nBurnBpp >> 1) * sizeof(short));

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

	nBurnPitch = 0;
	nBurnBpp = 0;

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
	psglRender();

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
	psglRender();

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
	psglRender();

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

	if (!bDrvOkay)
		VidFrame();

	CalculateViewports();

	if(pVidTransImage)
	{
		nCurrentFrame++;
		VidFrame_RecalcPalette();
	}
	return 0;
}
