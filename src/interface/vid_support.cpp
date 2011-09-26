// Support functions for all blitters
#include "burner.h"
#include "vid_support.h"

// ---------------------------------------------------------------------------
// General

int nGameWidth = 0, nGameHeight = 0;
int nRotateGame = 0;

static unsigned char* pVidSFullImage = NULL;

void VidSFreeVidImage()
{
	if (pVidSFullImage) {
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

	if (pVidSFullImage) {
		memset(pVidSFullImage, 0, nMemLen);
		pVidImage = pVidSFullImage + nVidImagePitch;
		return 0;
	} else {
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

	if (bDrvOkay) {
		// Get the game screen size
		BurnDrvGetVisibleSize(&nGameWidth, &nGameHeight);

		if (BurnDrvGetFlags() & BDF_ORIENTATION_VERTICAL) {
			if (nVidRotationAdjust & 1) {
				int n = nGameWidth;
				nGameWidth = nGameHeight;
				nGameHeight = n;
				nRotateGame |= (nVidRotationAdjust & 2);
			} else {
				nRotateGame |= 1;
			}
		}
		if (BurnDrvGetFlags() & BDF_ORIENTATION_FLIPPED) {
			nRotateGame ^= 2;
		}
	}
}

// Compute the resolution needed to run a game without scaling and filling the screen
// (taking into account the aspect ratios of the game and monitor)
bool VidSGetArcaderes(int* pWidth, int* pHeight)
{
	if (!bDrvOkay) {
		return false;
	}

	int nGameWidth, nGameHeight;
	int nGameAspectX, nGameAspectY;

	if ((BurnDrvGetFlags() & BDF_ORIENTATION_VERTICAL) && (nVidRotationAdjust & 1)) {
		BurnDrvGetVisibleSize(&nGameHeight, &nGameWidth);
		BurnDrvGetAspect(&nGameAspectY, &nGameAspectX);
	} else {
		BurnDrvGetVisibleSize(&nGameWidth, &nGameHeight);
		BurnDrvGetAspect(&nGameAspectX, &nGameAspectY);
	}

	double dMonAspect = (double)vidScrnAspect;
	double dGameAspect = (double)nGameAspectX / nGameAspectY;

	if (dMonAspect > dGameAspect) {
		*pWidth = nGameHeight * nGameAspectY * nGameWidth * vidScrnAspect / (nGameHeight * nGameAspectX);
		*pHeight = nGameHeight;
	} else {
		*pWidth = nGameWidth;
		*pHeight = nGameWidth * nGameAspectX * nGameHeight / (nGameWidth * nGameAspectY * vidScrnAspect);
	}

	// Horizontal resolution must be a multiple of 8
	if (*pWidth - nGameWidth < 8) {
		*pWidth = (*pWidth + 7) & ~7;
	} else {
		*pWidth = (*pWidth + 4) & ~7;
	}

	return true;
}

// This function takes a rectangle and scales it to either:
// - The largest possible multiple of both X and Y;
// - The largest possible multiple of Y, modifying X to ensure the correct aspect ratio;
// - The window size
int VidSScaleImage(RECT* pRect, int nGameWidth, int nGameHeight)
{
   #ifndef SN_TARGET_PS3
	if (bVidFullStretch) {				// Arbitrary stretch
		return 0;
	}
   #endif

	int xm, ym;							// The multiple of nScrnWidth and nScrnHeight we can fit in
	int nScrnWidth, nScrnHeight;

	int nGameAspectX = 4, nGameAspectY = 3;
	int nWidth = pRect->right - pRect->left;
	int nHeight = pRect->bottom - pRect->top;

#ifndef SN_TARGET_PS3
	if (bDrvOkay) {
#endif
		if ((BurnDrvGetFlags() & (BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED)) && (nVidRotationAdjust & 1)) {
			BurnDrvGetAspect(&nGameAspectY, &nGameAspectX);
		} else {
			BurnDrvGetAspect(&nGameAspectX, &nGameAspectY);
		}
#ifndef SN_TARGET_PS3
	}
#endif

	xm = nWidth / nGameWidth;
	ym = nHeight / nGameHeight;

#ifdef SN_TARGET_PS3
		nScrnWidth = nVidScrnWidth;
		nScrnHeight = nVidScrnHeight;
#else
	if (nVidFullscreen) {
		nScrnWidth = nVidScrnWidth;
		nScrnHeight = nVidScrnHeight;
	} else {
		nScrnWidth = SystemWorkArea.right - SystemWorkArea.left;
		nScrnHeight = SystemWorkArea.bottom - SystemWorkArea.top;
	}
#endif

	if (bVidCorrectAspect) {					// Correct aspect ratio
		int nWidthScratch;
		nWidthScratch = nHeight * nGameAspectX * nScrnWidth / (nScrnHeight * vidScrnAspect * nGameAspectY);
		if (nWidthScratch > nWidth) {			// The image is too wide
			if (nGameWidth < nGameHeight) {		// Vertical games
				nHeight = nWidth * nGameAspectY * nScrnWidth / (nScrnHeight * vidScrnAspect * nGameAspectX);
			} else {							// Horizontal games
				nHeight = nWidth * vidScrnAspect * nGameAspectY * nScrnHeight / (nScrnWidth * nGameAspectX);
			}
		} else {
			nWidth = nWidthScratch;
		}
	} else {
		if (xm && ym) {							// Don't correct aspect ratio
			if (xm > ym) {
				xm = ym;
			} else {
				ym = xm;
			}
			nWidth = nGameWidth * xm;
			nHeight = nGameHeight * ym;
		} else {
			if (xm) {
				nWidth = nGameWidth * xm * nHeight / nGameHeight;
			} else {
				if (ym) {
					nHeight = nGameHeight * ym * nWidth / nGameWidth;
				}
			}
		}
	}

	pRect->left = (pRect->right + pRect->left) >> 1;
	pRect->left -= nWidth >> 1;
	pRect->right = pRect->left + nWidth;

	pRect->top = (pRect->top + pRect->bottom) >> 1;
	pRect->top -= nHeight >> 1;
	pRect->bottom = pRect->top + nHeight;

	return 0;
}
