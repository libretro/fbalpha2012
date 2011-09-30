#ifndef SN_TARGET_PS3
// Pixel filter
// modified and cleanup by regret

#include "burner.h"
#include "vid_filter.h"

int nPreScale = 0;
int nPreScaleZoom = 0;
int nPreScaleEffect = 0;

// for filter parameters
void VidFilterParamInit()
{
	nPreScale = 3;
	nPreScaleZoom = 1;
	nPreScaleEffect = 0;

	if (vidUseFilter)
	{
		nPreScaleEffect = nVidFilter;
		nPreScaleZoom = VidFilterGetZoom(nPreScaleEffect);
	}
}

// ==> extra filters, added by regret
// filters from vba-rerecording and vba-m

#define NEED_SPEED

extern void AdMame2x(unsigned char*,unsigned int,unsigned char*,unsigned char*,unsigned int,int,int);
extern void AdMame2x32(unsigned char*,unsigned int,unsigned char*,unsigned char*,unsigned int,int,int);
extern void AdMame3x(unsigned char*,unsigned int,unsigned char*,unsigned char*,unsigned int,int,int);
extern void AdMame3x32(unsigned char*,unsigned int,unsigned char*,unsigned char*,unsigned int,int,int);

extern void hq2x_init(unsigned int);
extern void hq2x(unsigned char*,unsigned int,unsigned char*,unsigned char*,unsigned int,int,int);
extern void hq2x32(unsigned char*,unsigned int,unsigned char*,unsigned char*,unsigned int,int,int);
extern void hq2xS(unsigned char*,unsigned int,unsigned char*,unsigned char*,unsigned int,int,int);
extern void hq2xS32(unsigned char*,unsigned int,unsigned char*,unsigned char*,unsigned int,int,int);
extern void hq3x(unsigned char*,unsigned int,unsigned char*,unsigned char*,unsigned int,int,int);
extern void hq3x32(unsigned char*,unsigned int,unsigned char*,unsigned char*,unsigned int,int,int);
extern void hq3xS(unsigned char*,unsigned int,unsigned char*,unsigned char*,unsigned int,int,int);
extern void hq3xS32(unsigned char*,unsigned int,unsigned char*,unsigned char*,unsigned int,int,int);

extern int Init_2xSaI(unsigned int,unsigned int);
extern void _2xSaI(unsigned char*,unsigned int,unsigned char*,unsigned char*,unsigned int,int,int);
extern void _2xSaI32(unsigned char*,unsigned int,unsigned char*,unsigned char*,unsigned int,int,int);
extern void Super2xSaI(unsigned char*,unsigned int,unsigned char*,unsigned char*,unsigned int,int,int);
extern void Super2xSaI32(unsigned char*,unsigned int,unsigned char*,unsigned char*,unsigned int,int,int);
extern void SuperEagle(unsigned char*,unsigned int,unsigned char*,unsigned char*,unsigned int,int,int);
extern void SuperEagle32(unsigned char*,unsigned int,unsigned char*,unsigned char*,unsigned int,int,int);

extern void RenderHQ2XS(unsigned char*,unsigned int,unsigned char*,unsigned char*,unsigned int,int,int);
extern void RenderHQ2XBold(unsigned char*,unsigned int,unsigned char*,unsigned char*,unsigned int,int,int);
extern void RenderHQ3XS(unsigned char*,unsigned int,unsigned char*,unsigned char*,unsigned int,int,int);
extern void RenderHQ3XBold(unsigned char*,unsigned int,unsigned char*,unsigned char*,unsigned int,int,int);

extern void _2xpm_hq15(unsigned char*,unsigned int,unsigned char*,unsigned char*,unsigned int,int,int);
extern void _2xpm_hq16(unsigned char*,unsigned int,unsigned char*,unsigned char*,unsigned int,int,int);

extern void EPX16(unsigned char*,unsigned int,unsigned char*,unsigned char*,unsigned int,int,int);
extern void EPX32(unsigned char*,unsigned int,unsigned char*,unsigned char*,unsigned int,int,int);
extern void EPXPlus15(unsigned char*,unsigned int,unsigned char*,unsigned char*,unsigned int,int,int);
extern void EPXPlus16(unsigned char*,unsigned int,unsigned char*,unsigned char*,unsigned int,int,int);
extern void EPXPlus32(unsigned char*,unsigned int,unsigned char*,unsigned char*,unsigned int,int,int);

extern void Scanline16(unsigned char*,unsigned int,unsigned char*,unsigned char*,unsigned int,int,int);
extern void Scanline32(unsigned char*,unsigned int,unsigned char*,unsigned char*,unsigned int,int,int);
extern void Scanline50_15(unsigned char*,unsigned int,unsigned char*,unsigned char*,unsigned int,int,int);
extern void Scanline50_16(unsigned char*,unsigned int,unsigned char*,unsigned char*,unsigned int,int,int);
extern void Scanline50_32(unsigned char*,unsigned int,unsigned char*,unsigned char*,unsigned int,int,int);
extern void Scanline25_15(unsigned char*,unsigned int,unsigned char*,unsigned char*,unsigned int,int,int);
extern void Scanline25_16(unsigned char*,unsigned int,unsigned char*,unsigned char*,unsigned int,int,int);
extern void Scanline25_32(unsigned char*,unsigned int,unsigned char*,unsigned char*,unsigned int,int,int);
extern void interpolated_scanline15(unsigned char*,unsigned int,unsigned char*,unsigned char*,unsigned int,int,int);
extern void interpolated_scanline16(unsigned char*,unsigned int,unsigned char*,unsigned char*,unsigned int,int,int);
extern void interpolated_scanline32(unsigned char*,unsigned int,unsigned char*,unsigned char*,unsigned int,int,int);
extern void interpolated_scanline50_15(unsigned char*,unsigned int,unsigned char*,unsigned char*,unsigned int,int,int);
extern void interpolated_scanline50_16(unsigned char*,unsigned int,unsigned char*,unsigned char*,unsigned int,int,int);
extern void interpolated_scanline50_32(unsigned char*,unsigned int,unsigned char*,unsigned char*,unsigned int,int,int);
extern void interpolated_scanline25_15(unsigned char*,unsigned int,unsigned char*,unsigned char*,unsigned int,int,int);
extern void interpolated_scanline25_16(unsigned char*,unsigned int,unsigned char*,unsigned char*,unsigned int,int,int);
extern void interpolated_scanline25_32(unsigned char*,unsigned int,unsigned char*,unsigned char*,unsigned int,int,int);

// <== extra filters

#if 0
void SoftScale(unsigned char *srcPtr, unsigned int srcPitch, unsigned char * /* deltaPtr */,
                   unsigned char *dstPtr, unsigned int dstPitch, int width, int height)
{
	if (nVidImageBPP == 2) {						// 15/16-bit
		for (int y = 0; y < height; y++, dstPtr += (dstPitch << 1), srcPtr += srcPitch) {
			unsigned short* psEnd = (unsigned short*)(srcPtr + srcPitch);
			unsigned short* pdpc = (unsigned short*)dstPtr;
			unsigned short* pdpn = (unsigned short*)(dstPtr + dstPitch);
			unsigned short* psp = (unsigned short*)srcPtr;
			do {
				*pdpc++ = *psp;
				*pdpc++ = *psp;
				*pdpn++ = *psp;
				*pdpn++ = *psp++;
			} while (psp < psEnd);
		}
	}
	else if (nVidImageBPP == 4) {
		for (int y = 0; y < height; y++, dstPtr += (dstPitch << 1), srcPtr += srcPitch) {
			unsigned int* psEnd = (unsigned int*)(srcPtr + srcPitch);
			unsigned int* pdpc = (unsigned int*)dstPtr;
			unsigned int* pdpn = (unsigned int*)(dstPtr + dstPitch);
			unsigned int* psp = (unsigned int*)srcPtr;
			do {
				*pdpc++ = *psp;
				*pdpc++ = *psp;
				*pdpn++ = *psp;
				*pdpn++ = *psp++;
			} while (psp < psEnd);
		}
	} else {									// 24-bit
		for (int y = 0; y < height; y++, dstPtr += (dstPitch << 1), srcPtr += srcPitch) {
			unsigned char* psEnd = (unsigned char*)(srcPtr + srcPitch);
			unsigned char* pdpc = (unsigned char*)dstPtr;
			unsigned char* pdpn = (unsigned char*)(dstPtr + dstPitch);
			unsigned char* psp = (unsigned char*)srcPtr;
			do {
				pdpc[0] = psp[0];
				pdpc[3] = psp[0];
				pdpn[0] = psp[0];
				pdpn[3] = psp[0];
				pdpc[1] = psp[1];
				pdpc[4] = psp[1];
				pdpn[1] = psp[1];
				pdpn[4] = psp[1];
				pdpc[2] = psp[2];
				pdpc[5] = psp[2];
				pdpn[2] = psp[2];
				pdpn[5] = psp[2];
				psp += 3;
				pdpc += 3;
				pdpn += 3;
			} while (psp < psEnd);
		}
	}
}
#endif

// filter description, modified by regret
static struct {
	const TCHAR* pszName;
	int nZoom;
	FilterFunc func15;
	FilterFunc func16;
	FilterFunc func32;
} filterInfo[] = {
	{ _T("None"), 2, NULL, NULL, NULL },

	{ _T("EPX"), 2, EPX16, EPX16, EPX32 },
	{ _T("EPX Plus"), 2, EPXPlus15, EPXPlus16, EPXPlus32 },

	{ _T("Scale2x"), 2, 0, AdMame2x, AdMame2x32 },
	{ _T("Scale3x"), 3, 0, AdMame3x, AdMame3x32 },

	{ _T("2xSaI"), 2, 0, _2xSaI, _2xSaI32 },
	{ _T("Super 2xSaI"), 2, 0, Super2xSaI, Super2xSaI32 },
	{ _T("Super Eagle"), 2, 0, SuperEagle, SuperEagle32 },

	{ _T("2xPM"), 2, _2xpm_hq15, _2xpm_hq16, 0 },

	{ _T("HQ2x"), 2, 0, hq2x, hq2x32 },
#ifdef NEED_SPEED
	{ _T("HQ2xS"), 2, 0, RenderHQ2XS, hq2xS32 },
#else
	{ _T("HQ2xS"), 2, 0, hq2xS, hq2xS32 },
#endif
	{ _T("HQ2xBold"), 2, 0, RenderHQ2XBold, 0 },

	{ _T("HQ3x"), 3, 0, hq3x, hq3x32 },
#ifdef NEED_SPEED
	{ _T("HQ3xS"), 3, 0, RenderHQ3XS, hq3xS32 },
#else
	{ _T("HQ3xS"), 3, 0, hq3xS, hq3xS32 },
#endif
	{ _T("HQ3xBold"), 3, 0, RenderHQ3XBold, 0 },

	{ _T("Scanline"), 2, Scanline16, Scanline16, Scanline32 },
	{ _T("Scanline 50%"), 2, Scanline50_15, Scanline50_16, Scanline50_32 },
	{ _T("Scanline 25%"), 2, Scanline25_15, Scanline25_16, Scanline25_32 },
	{ _T("Interpolated scanline"), 2, interpolated_scanline15, interpolated_scanline16, interpolated_scanline32 },
	{ _T("Interpolated scanline 50%"), 2, interpolated_scanline50_15, interpolated_scanline50_16, interpolated_scanline50_32 },
	{ _T("Interpolated scanline 25%"), 2, interpolated_scanline25_15, interpolated_scanline25_16, interpolated_scanline25_32 },
}; // the sequence in this structure must be refered to filter type !

FilterFunc filterFunction = 0;
int nVidFilter = 0;

static void initFilter(const DWORD f, const int colorDepth)
{
	if (f == FILTER_NONE) {
		filterFunction = 0;
		return;
	}

	switch (colorDepth) {
		case 15:
			filterFunction = filterInfo[f].func15;
			break;
		case 16:
			filterFunction = filterInfo[f].func16;
			break;
		case 32:
			filterFunction = filterInfo[f].func32;
			break;
		default:
			filterFunction = 0;
			break;
	}

	if (filterFunction) {
		switch (f) {
			case FILTER_2XSAI:
			case FILTER_SUPER2XSAI:
			case FILTER_SUPEREAGLE:
				Init_2xSaI(565, colorDepth);
				break;
			case FILTER_HQ2X:
			case FILTER_HQ2XS:
			case FILTER_HQ2XBOLD:
			case FILTER_HQ3XS:
				hq2x_init(colorDepth);
				break;
			default:
				break;
		}
	}
}

static unsigned char* filterImage = NULL;
static int filterWidth = 0;
static int filterHeight = 0;
static int filterPitch = 0;
static int filterRotate = 0;
static int filterEffect = 0;

// switch filter for d3d7, added by regret
void VidSwitchFilter(int effect)
{
	if (effect <= FILTER_NONE) {
		nVidFilter = FILTER_NUMBER - 1;
	}
	else if (effect >= FILTER_NUMBER) {
		nVidFilter = FILTER_NONE + 1;
	}
	else {
		nVidFilter = effect;
	}

	// reinit video
	VidReinit();
	VidSNewShortMsg(VidFilterGetEffect(nVidFilter));
}

const TCHAR* VidFilterGetEffect(int effect)
{
	return filterInfo[effect].pszName;
}

int VidFilterGetZoom(int effect)
{
	return filterInfo[effect].nZoom;
}

int VidFilterCheckDepth(int effect, int depth)
{
	if (effect <= FILTER_NONE || effect >= FILTER_NUMBER) {
		return 0;
	}

	if (depth == 15 && filterInfo[effect].func15
		|| depth == 16 && filterInfo[effect].func16
		|| depth == 32 && filterInfo[effect].func32) {
		return depth;
	}

	return 0;
}

void VidFilterExit()
{
	if (filterRotate) {
		free(filterImage);
	}
	filterImage = NULL;

	filterRotate = 0;
	filterEffect = 0;

	return;
}

int VidFilterInit(int effect, int rotate)
{
	filterEffect = effect;

	if (VidFilterCheckDepth(filterEffect, nVidImageDepth) == 0) {
		VidFilterExit();
		return 1;
	}

	filterImage = pVidImage + nVidImageLeft * nVidImageBPP;
	filterWidth = nVidImageWidth; filterHeight = nVidImageHeight;

	filterRotate = 0;
	if (bDrvOkay) {
		BurnDrvGetFullSize(&filterWidth, &filterHeight);

		if ((rotate & 1) && (BurnDrvGetFlags() & BDF_ORIENTATION_VERTICAL)) {
			filterRotate |= 1;
			BurnDrvGetFullSize(&filterHeight, &filterWidth);
		}

		if ((rotate & 2) && (BurnDrvGetFlags() & BDF_ORIENTATION_FLIPPED)) {
			filterRotate |= 2;
		}

		if (filterRotate) {
			filterImage = (unsigned char*)malloc(filterWidth * filterHeight * nVidImageBPP);
			if (filterImage == NULL) {
				VidFilterExit();
				return 1;
			}
		}
	}

	filterPitch = filterWidth * nVidImageBPP;

	// Init filters
	initFilter(filterEffect, nVidImageDepth);

#ifdef PRINT_DEBUG_INFO
   	dprintf(_T("  * Filter initialised: using %s in %i-bit mode.\n"), filterInfo[filterEffect].pszName, nVidImageDepth);
#endif

	return 0;
}

int VidFilterScale(RECT* pRect, int width, int height)
{
	int nWidth = pRect->right - pRect->left;
	int nHeight = pRect->bottom - pRect->top;

	int filterEnlarge = 0;
	if ((nWidth >= (width * filterInfo[filterEffect].nZoom))
		&& (nHeight >= (height * filterInfo[filterEffect].nZoom))) {
		nWidth = width * filterInfo[filterEffect].nZoom;
		nHeight = height * filterInfo[filterEffect].nZoom;
		filterEnlarge = 1;
	}

	if (!filterEnlarge) {
		nWidth = width;
		nHeight = height;
	}

	pRect->left = (pRect->right + pRect->left) >> 1;
	pRect->left -= nWidth >> 1;
	pRect->right = pRect->left + nWidth;

	pRect->top = (pRect->bottom + pRect->top) >> 1;
	pRect->top -= nHeight >> 1;
	pRect->bottom = pRect->top + nHeight;

	return 0;
}

static inline void VidFilterRotate()
{
	if (filterRotate == 0) {
		return;
	}

	unsigned char* ps, *pdd;
	unsigned char* pd = filterImage;
	if (filterRotate & 2) {
		pd += filterHeight * filterPitch - nVidImageBPP;
	}

	switch (nVidImageBPP) {
		case 4:	{
			switch (filterRotate) {
				case 1: {
					for (int y = 0; y < filterHeight; y++) {
						ps = pVidImage + (filterHeight + nVidImageLeft - 1 - y) * nVidImageBPP;
						for (int x = 0; x < filterWidth; x++) {
							*(int*)pd = *(int*)ps;
							ps += nVidImagePitch;
							pd += nVidImageBPP;
						}
					}
					break;
				}
				case 2: {
					for (int y = 0; y < filterHeight; y++) {
						ps = pVidImage + y * nVidImagePitch + nVidImageLeft * nVidImageBPP;
						for (int x = 0; x < filterWidth; x++) {
							*(int*)pd = *(int*)ps;
							ps += nVidImageBPP;
							pd -= nVidImageBPP;
						}
					}
					break;
				}
				case 3: {
					for (int y = 0; y < filterHeight; y++) {
						ps = pVidImage + (filterHeight + nVidImageLeft - 1 - y) * nVidImageBPP;
						for (int x = 0; x < filterWidth; x++) {
							*(int*)pd = *(int*)ps;
							ps += nVidImagePitch;
							pd -= nVidImageBPP;
						}
					}
					break;
				}
			}
			break;
		}

		case 3: {
			switch (filterRotate) {
				case 1: {
					for (int y = 0; y < filterHeight; y++) {
						ps = pVidImage + (filterHeight + nVidImageLeft - 1 - y) * nVidImageBPP;
						for (int x = 0; x < filterWidth; x++) {
							pd[0] = ps[0];
							pd[1] = ps[2];
							pd[2] = ps[2];
							ps += nVidImagePitch;
							pd += nVidImageBPP;
						}
					}
					break;
				}
				case 2: {
					for (int y = 0; y < filterHeight; y++) {
						ps = pVidImage + y * nVidImagePitch + nVidImageLeft * nVidImageBPP;
						for (int x = 0; x < filterWidth; x++) {
							pd[0] = ps[0];
							pd[1] = ps[2];
							pd[2] = ps[2];
							ps += nVidImageBPP;
							pd -= nVidImageBPP;
						}
					}
					break;
				}
				case 3: {
					for (int y = 0; y < filterHeight; y++) {
						ps = pVidImage + (filterHeight + nVidImageLeft - 1 - y) * nVidImageBPP;
						for (int x = 0; x < filterWidth; x++) {
							pd[0] = ps[0];
							pd[1] = ps[2];
							pd[2] = ps[2];
							ps += nVidImagePitch;
							pd -= nVidImageBPP;
						}
					}
					break;
				}
			}
			break;
		}

		case 2:	{
			switch (filterRotate) {
				case 1: {
					for (int y = 0; y < filterHeight; y++) {
						ps = pVidImage + (filterHeight + nVidImageLeft - 1 - y) * nVidImageBPP;
						for (int x = 0; x < filterWidth; x++) {
							*(short*)pd = *(short*)ps;
							ps += nVidImagePitch;
							pd += nVidImageBPP;
						}
					}
					break;
				}
				case 2: {
					for (int y = 0; y < filterHeight; y++) {
						ps = pVidImage + y * nVidImagePitch + nVidImageLeft * nVidImageBPP;
						for (int x = 0; x < filterWidth; x++) {
							*(short*)pd = *(short*)ps;
							ps += nVidImageBPP;
							pd -= nVidImageBPP;
						}
					}
					break;
				}
				case 3: {
					for (int y = 0; y < filterHeight; y++) {
						ps = pVidImage + (filterHeight + nVidImageLeft - 1 - y) * nVidImageBPP;
						for (int x = 0; x < filterWidth; x++) {
							*(short*)pd = *(short*)ps;
							ps += nVidImagePitch;
							pd -= nVidImageBPP;
						}
					}
					break;
				}
			}
			break;
		}
	}
}

int VidFilterApplyEffect(unsigned char* pd, int pitch)
{
	if (!pd) {
		return 1;
	}

	VidFilterRotate();

	if (filterFunction) {
		filterFunction(filterImage, filterPitch, NULL, pd, pitch, filterWidth, filterHeight);
		return 0;
	}
	return 1;
}
#endif
