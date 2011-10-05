// Support functions for all blitters
#include "burner.h"
#include "vid_support.h"

// ---------------------------------------------------------------------------
// General

static unsigned char* pVidSFullImage = NULL;

void VidSFreeVidImage()
{
	free(pVidSFullImage);
	pVidSFullImage = NULL;
	pVidImage = NULL;
}

int VidSAllocVidImage()
{
	int nMemLen = 0;

	VidSFreeVidImage();

	// Allocate an extra line above and below the image to accomodate effects
	nVidImagePitch = nVidImageWidth * ((nVidImageDepth + 7) >> 3);
	nMemLen = (nVidImageHeight + 2) * nVidImagePitch;
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

