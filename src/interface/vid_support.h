#ifndef VID_SUPPORT_H
#define VID_SUPPORT_H

extern int nGameWidth, nGameHeight;		// screen size
extern int nRotateGame;

void VidSFreeVidImage();
int VidSAllocVidImage();

void VidInitInfo();
#if defined (_XBOX)
static inline uint32_t next_pow2(uint32_t v)
{
   v--;
   v |= v >> 1;
   v |= v >> 2;
   v |= v >> 4;
   v |= v >> 8;
   v |= v >> 16;
   v++;
   return v;
}

static inline int VidGetTextureSize(int size)
{
	int textureSize = 128;
	//ugly code, could be replaced by call to next_pow2
	while (textureSize < size)
		textureSize <<= 1;
	//end of ugly code
	return textureSize;
}
#endif

// Image size
#ifndef SN_TARGET_PS3
int VidSScaleImage(RECT* pRect, int nGameWidth, int nGameHeight);
#endif
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
