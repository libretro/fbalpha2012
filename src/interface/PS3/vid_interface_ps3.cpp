#include "burnint.h"
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
int nVidScrnAspectX = 4;			// Aspect ratio (X-axis)of the display screen
int nVidScrnAspectY = 3;			// Aspect ratio (Y-axis) of the display screen
int nVidScrnAspectMode = ASPECT_RATIO_4_3;
float m_ratio = ASPECT_RATIO_4_3;		// Aspect ratio
int nVidImageWidth = DEFAULT_IMAGE_WIDTH;	// Memory buffer size
int nVidImageHeight = DEFAULT_IMAGE_HEIGHT;
int nVidImagePitch = 0;				// Memory buffer pitch
bool bVidRecalcPalette;
bool bVidFBOEnabled;				// FBO/Dual shader mode
unsigned int bVidScalingFactor = 1;		// Scaling factor for use with FBO mode
unsigned char* pVidImage = NULL;		// Memory buffer
int nXOffset = 0;
int nYOffset = 0;
uint32_t m_viewport_x, m_viewport_y, m_viewport_width, m_viewport_height;
uint32_t m_viewport_x_temp, m_viewport_y_temp, m_viewport_width_temp, m_viewport_height_temp, m_delta_temp;
GLfloat m_left, m_right, m_bottom, m_top, m_zNear, m_zFar;

extern struct BurnDriver * pDriver[];

unsigned int (__cdecl *VidHighCol) (int r, int g, int b, int i);

unsigned int __cdecl HighCol15(int r, int g, int b, int  /* i */)
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
			nBurnBpp = SCREEN_RENDER_TEXTURE_BPP; // Set Burn library Bytes per pixel
			bVidOkay = true;
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


	return nRet;
}

int VidFrame()
{
	pBurnDraw = pVidImage;
	nBurnPitch = nVidImagePitch;
	pDriver[nBurnDrvSelect]->Frame();
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

	return 0;
}
