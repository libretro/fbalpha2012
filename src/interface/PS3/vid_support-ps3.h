#ifndef VID_SUPPORT_H
#define VID_SUPPORT_H

extern int nGameWidth, nGameHeight;		// screen size
extern int nRotateGame;

void VidSFreeVidImage();
int VidSAllocVidImage();

void VidInitInfo();

// Image size
//bool VidSGetArcaderes(int* pWidth, int* pHeight);

//for 16-bit 
//#define SCREEN_RENDER_TEXTURE_BITDEPTH 16
//#define SCREEN_RENDER_TEXTURE_BPP_SHIFT 1

//for 32-bit
#define SCREEN_RENDER_TEXTURE_BITDEPTH 32
#define SCREEN_RENDER_TEXTURE_BPP_SHIFT 2

#define VidSCopyImage(dst_ori) \
	uint8_t * ps = pVidImage + (nVidImageLeft << SCREEN_RENDER_TEXTURE_BPP_SHIFT); \
	int s = nVidImageWidth << SCREEN_RENDER_TEXTURE_BPP_SHIFT; \
	register uint16_t lineSize = nVidImageWidth << SCREEN_RENDER_TEXTURE_BPP_SHIFT; \
	uint16_t height = nVidImageHeight; \
	uint8_t * dst = (uint8_t *)dst_ori; \
	do{ \
		height--; \
		memcpy(dst, ps, lineSize); \
		ps += s; \
		dst += pitch; \
	}while(height);

#endif
