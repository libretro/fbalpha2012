#ifndef VID_SUPPORT_H
#define VID_SUPPORT_H

extern int nGameWidth, nGameHeight;		// screen size
extern int nRotateGame;

void VidSFreeVidImage();
int VidSAllocVidImage();

void VidInitInfo();
static inline int VidGetTextureSize(int size)
{
#if defined (_XBOX)
	int textureSize = 128;
	while (textureSize < size) {
		textureSize <<= 1;
	}
	return textureSize;
#else
	return size;
#endif
}

// Image size
int VidSScaleImage(RECT* pRect, int nGameWidth, int nGameHeight);
bool VidSGetArcaderes(int* pWidth, int* pHeight);

static inline void VidSCpyImg32(unsigned char* dst, unsigned int dstPitch, unsigned char *src, unsigned int srcPitch, unsigned short width, unsigned short height)
{
	// fast, iterative C version
	// copies an width*height array of visible pixels from src to dst
	// srcPitch and dstPitch are the number of garbage bytes after a scanline
	register unsigned short lineSize = width << 2;

	do{
		height--;
		memcpy(dst, src, lineSize);
		src += srcPitch;
		dst += dstPitch;
	}while (height);
}

static inline void VidSCpyImg16(unsigned char * __restrict dst, unsigned int dstPitch, unsigned char * __restrict src, unsigned int srcPitch, unsigned short width, unsigned short height)
{
	register unsigned short lineSize = width << 1;

	while (height--) {
#if defined (_XBOX)
		XMemCpy(dst, src, lineSize);
#else
		memcpy(dst, src, lineSize);
#endif
		src += srcPitch;
		dst += dstPitch;
	}
}

#endif
