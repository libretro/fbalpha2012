// Misc functions module
#include <math.h>
#include "burner.h"

// ----------------------------------------------------------------------------
// Software gamma, brightness and contrast adjust, added by regret
// ref: bsnes

#if defined (SN_TARGET_PS3) || defined(__LIBSNES__)
#define min(a,b) (((a)<(b))?(a):(b)) 
#define max(a,b) (((a)>(b))?(a):(b))
#endif

#ifndef SN_TARGET_PS3
int bcolorAdjust = 0;
int color_gamma = 0;
int color_brightness = 0;
int color_contrast = 0;
int color_grayscale = 0;
int color_invert = 0;

static unsigned char contrast_adjust(unsigned char input)
{
	signed result = input - color_contrast + (2 * color_contrast * input + 127) / 255;
	return max(0, min(255, result));
}

static unsigned char brightness_adjust(unsigned char input)
{
	signed result = input + color_brightness;
	return max(0, min(255, result));
}

static unsigned char gamma_adjust(unsigned char input)
{
	signed result = (signed)(pow(((double)input / 255.0), (double)(100 + color_gamma) / 100.0) * 255.0 + 0.5);
	return max(0, min(255, result));
}

void colorAdjust(int& r, int& g, int& b)
{
	if (!bcolorAdjust)
		return;

	double kr = 0.2126, kb = 0.0722, kg = (1.0 - kr - kb);  //luminance weights

	if (color_contrast != 0) {
		r = contrast_adjust(r);
		g = contrast_adjust(g);
		b = contrast_adjust(b);
	}

	if (color_brightness != 0) {
		r = brightness_adjust(r);
		g = brightness_adjust(g);
		b = brightness_adjust(b);
	}

	if (color_gamma != 0) {
		r = gamma_adjust(r);
		g = gamma_adjust(g);
		b = gamma_adjust(b);
	}

	if (color_grayscale) {
		signed l = (signed)((double)r * kr + (double)g * kg + (double)b * kb);
		l = max(0, min(255, l));
		r = g = b = l;
    }

    if (color_invert) {
		r ^= 0xff;
		g ^= 0xff;
		b ^= 0xff;
    }
}
#endif

// Standard callbacks for 16/24/32 bit color:
static unsigned int __cdecl HighCol15(int r, int g, int b, int  /* i */)
{
#ifndef SN_TARGET_PS3
	colorAdjust(r, g, b);
#endif

	unsigned int t;
	t  = (r << 7) & 0x7c00; // 0rrr rr00 0000 0000
	t |= (g << 2) & 0x03e0; // 0000 00gg ggg0 0000
	t |= (b >> 3) & 0x001f; // 0000 0000 000b bbbb
	return t;
}

static unsigned int __cdecl HighCol16(int r, int g, int b, int /* i */)
{
#ifndef SN_TARGET_PS3
	colorAdjust(r, g, b);
#endif

	unsigned int t;
	t  = (r << 8) & 0xf800; // rrrr r000 0000 0000
	t |= (g << 3) & 0x07e0; // 0000 0ggg ggg0 0000
	t |= (b >> 3) & 0x001f; // 0000 0000 000b bbbb
	return t;
}

// 24-bit/32-bit
static unsigned int __cdecl HighCol24(int r, int g, int b, int  /* i */)
{
#ifndef SN_TARGET_PS3
	colorAdjust(r, g, b);
#endif

	unsigned int t;
	t  = (r << 16) & 0xff0000;
	t |= (g << 8 ) & 0x00ff00;
	t |= (b      ) & 0x0000ff;
	return t;
}

#ifndef __LIBSNES__
int SetBurnHighCol(int nDepth)
{
	VidRecalcPal();

	if (nDepth == 15)
		VidHighCol = HighCol15;
	else if (nDepth == 16)
		VidHighCol = HighCol16;
	else if (nDepth > 16)
		VidHighCol = HighCol24;

	if (bDrvOkay && !(BurnDrvGetFlags() & BDF_16BIT_ONLY) || nDepth <= 16)
		BurnHighCol = VidHighCol;

	return 0;
}
#endif
