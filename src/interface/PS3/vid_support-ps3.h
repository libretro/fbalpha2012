#ifndef VID_SUPPORT_H
#define VID_SUPPORT_H

extern int nGameWidth, nGameHeight;		// screen size
extern int nRotateGame;

void VidSFreeVidImage();
int VidSAllocVidImage();

void VidInitInfo();

//for 16-bit 
#define BPP_16_SCREEN_RENDER_TEXTURE_BITDEPTH 16
#define BPP_16_SCREEN_RENDER_TEXTURE_BPP_SHIFT 1
#define BPP_16_SCREEN_RENDER_TEXTURE_BPP 2
#define BPP_16_SCREEN_RENDER_TEXTURE_PIXEL_FORMAT GL_RGB5_A1

//for 32-bit
#define BPP_32_SCREEN_RENDER_TEXTURE_BITDEPTH 32
#define BPP_32_SCREEN_RENDER_TEXTURE_BPP_SHIFT 2
#define BPP_32_SCREEN_RENDER_TEXTURE_BPP 4
#define BPP_32_SCREEN_RENDER_TEXTURE_PIXEL_FORMAT GL_ARGB_SCE

#if USE_BPP_RENDERING == 32
#define SCREEN_RENDER_TEXTURE_BITDEPTH		BPP_32_SCREEN_RENDER_TEXTURE_BITDEPTH
#define SCREEN_RENDER_TEXTURE_BPP		BPP_32_SCREEN_RENDER_TEXTURE_BPP
#define SCREEN_RENDER_TEXTURE_BPP_SHIFT		BPP_32_SCREEN_RENDER_TEXTURE_BPP_SHIFT
#define SCREEN_RENDER_TEXTURE_PIXEL_FORMAT	BPP_32_SCREEN_RENDER_TEXTURE_PIXEL_FORMAT
#elif USE_BPP_RENDERING == 16
#define SCREEN_RENDER_TEXTURE_BITDEPTH		BPP_16_SCREEN_RENDER_TEXTURE_BITDEPTH
#define SCREEN_RENDER_TEXTURE_BPP		BPP_16_SCREEN_RENDER_TEXTURE_BPP
#define SCREEN_RENDER_TEXTURE_BPP_SHIFT		BPP_16_SCREEN_RENDER_TEXTURE_BPP_SHIFT
#define SCREEN_RENDER_TEXTURE_PIXEL_FORMAT	BPP_16_SCREEN_RENDER_TEXTURE_PIXEL_FORMAT
#endif

#define VidSCopyImage() \
	uint8_t * dst = (uint8_t *)buffer; \
	unsigned int pitch = nVidImageWidth * sizeof(unsigned int); \
	uint8_t * ps = pVidImage + (nVidImageLeft << SCREEN_RENDER_TEXTURE_BPP_SHIFT); \
	int linesize = nVidImageWidth << SCREEN_RENDER_TEXTURE_BPP_SHIFT; \
	uint16_t height = nVidImageHeight; \
	do{ \
		memcpy(dst, ps, linesize); \
		ps += linesize; \
		dst += pitch; \
		\
		memcpy(dst, ps, linesize); \
		ps += linesize; \
		dst += pitch; \
		height -= 2; \
	}while(height);

#endif
