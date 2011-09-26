#ifndef SN_TARGET_PS3
// Sotfware effects
#ifndef VID_FILTER_H
#define VID_FILTER_H

//#include "vid_directx_support.h"

// filter type, added by regret
enum FILTER_TYPE {
	FILTER_NONE = 0,
	FILTER_EPXB,
	FILTER_EPXC,
	FILTER_SCALE2X,
	FILTER_SCALE3X,
	FILTER_2XSAI,
	FILTER_SUPER2XSAI,
	FILTER_SUPEREAGLE,
	FILTER_2XPMHQ,
	FILTER_HQ2X,
	FILTER_HQ2XS,
	FILTER_HQ2XBOLD,
	FILTER_HQ3X,
	FILTER_HQ3XS,
	FILTER_HQ3XBOLD,
	FILTER_SCANLINE,
	FILTER_SCANLINE50,
	FILTER_SCANLINE25,
	FILTER_INTERSCANLINE,
	FILTER_INTERSCANLINE50,
	FILTER_INTERSCANLINE25,

	FILTER_NUMBER,
};

extern int nPreScale;
extern int nPreScaleZoom;
extern int nPreScaleEffect;

void VidFilterParamInit();

// Function pointer type for a filter function, from vba-m
typedef void(*FilterFunc)(unsigned char*, unsigned int, unsigned char*, unsigned char*, unsigned int, int, int);

const TCHAR* VidFilterGetEffect(int effect);
int VidFilterGetZoom(int effect);
int VidFilterCheckDepth(int effect, int depth);

void VidFilterExit();
int VidFilterInit(int effect, int rotate);
int VidFilterScale(RECT* rect, int width, int height);
int VidFilterApplyEffect(unsigned char* dst, int pitch);

#endif
#endif
