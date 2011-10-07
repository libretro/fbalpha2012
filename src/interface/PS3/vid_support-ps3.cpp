// Support functions for all blitters
#include "burner.h"
#include "vid_support-ps3.h"

// ---------------------------------------------------------------------------
// General

int nGameWidth = 0;
int nGameHeight = 0;
int nRotateGame = 0;

static unsigned char* pVidSFullImage = NULL;

void VidSFreeVidImage()
{
	if (pVidSFullImage)
	{
		free(pVidSFullImage);
		pVidSFullImage = NULL;
	}
	pVidImage = NULL;
}

int VidSAllocVidImage()
{
	unsigned int nMemLen = 0;

	VidSFreeVidImage();

	// Allocate an extra line above and below the image to accomodate effects
	nVidImagePitch = nVidImageWidth * ((nVidImageDepth + 7) >> 3);
	// for using large skin with D3D7+Preview Blitter, modified by regret
	nMemLen = (nVidImageHeight + 4) * nVidImagePitch;
	pVidSFullImage = (unsigned char*)malloc(nMemLen);

	if (pVidSFullImage)
	{
		memset(pVidSFullImage, 0, nMemLen);
		pVidImage = pVidSFullImage + nVidImagePitch;
		return 0;
	}
	else
	{
		pVidImage = NULL;
		return 1;
	}
}

// init game info, added by regret
void VidInitInfo()
{
	nGameWidth = nVidImageWidth;
	nGameHeight = nVidImageHeight;
	nRotateGame = 0;

	if (bDrvOkay)
	{
		// Get the game screen size
		BurnDrvGetVisibleSize(&nGameWidth, &nGameHeight);

		if (BurnDrvGetFlags() & BDF_ORIENTATION_VERTICAL)
		{
			if (nVidRotationAdjust & 1)
			{
				int n = nGameWidth;
				nGameWidth = nGameHeight;
				nGameHeight = n;
				nRotateGame |= (nVidRotationAdjust & 2);
			}
			else
				nRotateGame |= 1;
		}

		if (BurnDrvGetFlags() & BDF_ORIENTATION_FLIPPED)
			nRotateGame ^= 2;
	}
}
