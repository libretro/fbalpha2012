// interploated scanline filters, added by regret
// source from gens-gs (http://www.sonicretro.org/)

// interploated scanline

// Mask constants.
#define MASK_DIV2_15 		((unsigned short)(0x3DEF))
#define MASK_DIV2_16		((unsigned short)(0x7BEF))
#define MASK_DIV2_32		((unsigned int)(0x7F7F7F7F))

template<typename pixel>
static inline void render_interpolated_scanline(pixel *dst, pixel *src, int destPitch, int srcPitch, int width, int height, pixel mask)
{
	destPitch /= sizeof(pixel);
	srcPitch /= sizeof(pixel);

	for (int y = 0; y < height; y++)
	{
		pixel *SrcLine = &src[y * srcPitch];
		pixel *DstLine1 = &dst[(y * 2) * destPitch];
		pixel *DstLine2 = &dst[((y * 2) + 1) * destPitch];

		for (int x = 0; x < width; x++)
		{
			pixel C = *(SrcLine);
			pixel R = *(SrcLine + 1);

			*DstLine1++ = C;
			*DstLine1++ = ((C >> 1) & mask) + ((R >> 1) & mask);
			*DstLine2++ = 0;
			*DstLine2++ = 0;

			SrcLine++;
		}
	}
}

void interpolated_scanline15(unsigned char *srcPtr, unsigned int srcPitch, unsigned char * /* deltaPtr */, unsigned char *dstPtr, unsigned int dstPitch, int width, int height)
{
	render_interpolated_scanline((unsigned short*)dstPtr, (unsigned short*)srcPtr, dstPitch, srcPitch, width, height, MASK_DIV2_15);
}

void interpolated_scanline16(unsigned char *srcPtr, unsigned int srcPitch, unsigned char * /* deltaPtr */, unsigned char *dstPtr, unsigned int dstPitch, int width, int height)
{
	render_interpolated_scanline((unsigned short*)dstPtr, (unsigned short*)srcPtr, dstPitch, srcPitch, width, height, MASK_DIV2_16);
}

void interpolated_scanline32(unsigned char *srcPtr, unsigned int srcPitch, unsigned char * /* deltaPtr */, unsigned char *dstPtr, unsigned int dstPitch, int width, int height)
{
	render_interpolated_scanline((unsigned int*)dstPtr, (unsigned int*)srcPtr, dstPitch, srcPitch, width, height, MASK_DIV2_32);
}

// interploated scanline 50%

#define BLEND(a, b, mask) ((((a) >> 1) & mask) + (((b) >> 1) & mask))

template<typename pixel>
static inline void render_interpolated_scanline_50(pixel *dst, pixel *src, int destPitch, int srcPitch, int width, int height, pixel mask)
{
	destPitch /= sizeof(pixel);
	srcPitch /= sizeof(pixel);

	// TODO: Figure out why the interpolation function is using the line
	// below the source line instead of using the current source line.
	for (int y = 0; y < height; y++)
	{
		pixel *SrcLine = &src[y * srcPitch];
		pixel *DstLine1 = &dst[(y * 2) * destPitch];
		pixel *DstLine2 = &dst[((y * 2) + 1) * destPitch];

		for (int x = 0; x < width; x++)
		{
			pixel C = *(SrcLine);
			pixel R = *(SrcLine + 1);
			pixel D = *(SrcLine + srcPitch);
			pixel DR = *(SrcLine + srcPitch + 1);

			*DstLine1++ = C;
			*DstLine1++ = BLEND(C, R, mask);
			*DstLine2++ = (BLEND(C, D, mask) >> 1) & mask;
			*DstLine2++ = ((BLEND(BLEND(C, R, mask), BLEND(D, DR, mask), mask)) >> 1) & mask;

			SrcLine++;
		}
	}
}

void interpolated_scanline50_15(unsigned char *srcPtr, unsigned int srcPitch, unsigned char * /* deltaPtr */, unsigned char *dstPtr, unsigned int dstPitch, int width, int height)
{
	render_interpolated_scanline_50((unsigned short*)dstPtr, (unsigned short*)srcPtr, dstPitch, srcPitch, width, height, MASK_DIV2_15);
}

void interpolated_scanline50_16(unsigned char *srcPtr, unsigned int srcPitch, unsigned char * /* deltaPtr */, unsigned char *dstPtr, unsigned int dstPitch, int width, int height)
{
	render_interpolated_scanline_50((unsigned short*)dstPtr, (unsigned short*)srcPtr, dstPitch, srcPitch, width, height, MASK_DIV2_16);
}

void interpolated_scanline50_32(unsigned char *srcPtr, unsigned int srcPitch, unsigned char * /* deltaPtr */, unsigned char *dstPtr, unsigned int dstPitch, int width, int height)
{
	render_interpolated_scanline_50((unsigned int*)dstPtr, (unsigned int*)srcPtr, dstPitch, srcPitch, width, height, MASK_DIV2_32);
}

// interploated scanline 25%

// Mask constants.
#define MASK_DIV4_15		((unsigned short)(0x1CE7))
#define MASK_DIV4_16		((unsigned short)(0x39E7))
#define MASK_DIV4_32		((unsigned int)(0x3F3F3F3F))

template<typename pixel>
static inline void render_interpolated_scanline_25(pixel *dst, pixel *src, int destPitch, int srcPitch, int width, int height, pixel mask2, pixel mask4)
{
	destPitch /= sizeof(pixel);
	srcPitch /= sizeof(pixel);

	// TODO: Figure out why the interpolation function is using the line
	// below the source line instead of using the current source line.
	for (int y = 0; y < height; y++)
	{
		pixel *SrcLine = &src[y * srcPitch];
		pixel *DstLine1 = &dst[(y * 2) * destPitch];
		pixel *DstLine2 = &dst[((y * 2) + 1) * destPitch];

		for (int x = 0; x < width; x++)
		{
			pixel C = *(SrcLine);
			pixel R = *(SrcLine + 1);
			pixel D = *(SrcLine + srcPitch);
			pixel DR = *(SrcLine + srcPitch + 1);

			*DstLine1++ = C;
			*DstLine1++ = BLEND(C, R, mask2);

			pixel tmpD = BLEND(C, D, mask2);
			*DstLine2++ = ((tmpD >> 1) & mask2) + ((tmpD >> 2) & mask4);

			pixel tmpDR = BLEND(BLEND(C, R, mask2), BLEND(D, DR, mask2), mask2);
			*DstLine2++ = ((tmpDR >> 1) & mask2) + ((tmpDR >> 2) & mask4);

			SrcLine++;
		}
	}
}

void interpolated_scanline25_15(unsigned char *srcPtr, unsigned int srcPitch, unsigned char * /* deltaPtr */, unsigned char *dstPtr, unsigned int dstPitch, int width, int height)
{
	render_interpolated_scanline_25((unsigned short*)dstPtr, (unsigned short*)srcPtr, dstPitch, srcPitch, width, height, MASK_DIV2_15, MASK_DIV4_15);
}

void interpolated_scanline25_16(unsigned char *srcPtr, unsigned int srcPitch, unsigned char * /* deltaPtr */, unsigned char *dstPtr, unsigned int dstPitch, int width, int height)
{
	render_interpolated_scanline_25((unsigned short*)dstPtr, (unsigned short*)srcPtr, dstPitch, srcPitch, width, height, MASK_DIV2_16, MASK_DIV4_16);
}

void interpolated_scanline25_32(unsigned char *srcPtr, unsigned int srcPitch, unsigned char * /* deltaPtr */, unsigned char *dstPtr, unsigned int dstPitch, int width, int height)
{
	render_interpolated_scanline_25((unsigned int*)dstPtr, (unsigned int*)srcPtr, dstPitch, srcPitch, width, height, MASK_DIV2_32, MASK_DIV4_32);
}
