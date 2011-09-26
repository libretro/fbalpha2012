// scanline filters, added by regret
// source from gens-gs (http://www.sonicretro.org/)

#if defined (_XBOX)
#include <memory.h>
#else
#include <string.h>
#endif

// scanline
template<typename pixel>
static inline void render_scanline(pixel *dst, pixel *src, int destPitch, int srcPitch, int width, int height)
{
	// Pitch difference.
	destPitch /= sizeof(pixel);
	int nextLine = destPitch + (destPitch - (width * 2));

	srcPitch /= sizeof(pixel);

	pixel *line = dst;
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			*line++ = *src;
			*line++ = *src++;
		}

		// Next line.
		src += (srcPitch - width);
		line += nextLine;
	}

	// Zero the extra lines.
	line = dst + destPitch;
	for (unsigned short i = 0; i < height; i++)
	{
		memset(line, 0x00, width * 2 * sizeof(pixel));
		line += (destPitch * 2);
	}
}

void Scanline16(unsigned char *srcPtr, unsigned int srcPitch, unsigned char * /* deltaPtr */, unsigned char *dstPtr, unsigned int dstPitch, int width, int height)
{
	render_scanline((unsigned short*)dstPtr, (unsigned short*)srcPtr, dstPitch, srcPitch, width, height);
}

void Scanline32(unsigned char *srcPtr, unsigned int srcPitch, unsigned char * /* deltaPtr */, unsigned char *dstPtr, unsigned int dstPitch, int width, int height)
{
	render_scanline((unsigned int*)dstPtr, (unsigned int*)srcPtr, dstPitch, srcPitch, width, height);
}

// scanline 50%

// Mask constants.
#define MASK_DIV2_15		((unsigned short)(0x3DEF))
#define MASK_DIV2_16		((unsigned short)(0x7BEF))
#define MASK_DIV2_32		((unsigned int)(0x7F7F7F7F))

template<typename pixel>
static inline void render_scanline_50(pixel *dst, pixel *src, int destPitch, int srcPitch, int width, int height, pixel mask)
{
	// Pitch difference.
	destPitch /= sizeof(pixel);
	int nextLine = destPitch + (destPitch - (width * 2));

	srcPitch /= sizeof(pixel);

	pixel *line1 = dst;
	pixel *line2 = dst + destPitch;
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			pixel tmp = *src++;

			// First line.
			*line1++ = tmp;
			*line1++ = tmp;

			// Second line. (50% Scanline effect)
			tmp = (tmp >> 1) & mask;
			*line2++ = tmp;
			*line2++ = tmp;
		}

		// Next line.
		src += (srcPitch - width);
		line1 += nextLine;
		line2 += nextLine;
	}
}

void Scanline50_15(unsigned char *srcPtr, unsigned int srcPitch, unsigned char * /* deltaPtr */, unsigned char *dstPtr, unsigned int dstPitch, int width, int height)
{
	render_scanline_50((unsigned short*)dstPtr, (unsigned short*)srcPtr, dstPitch, srcPitch, width, height, MASK_DIV2_15);
}

void Scanline50_16(unsigned char *srcPtr, unsigned int srcPitch, unsigned char * /* deltaPtr */, unsigned char *dstPtr, unsigned int dstPitch, int width, int height)
{
	render_scanline_50((unsigned short*)dstPtr, (unsigned short*)srcPtr, dstPitch, srcPitch, width, height, MASK_DIV2_16);
}

void Scanline50_32(unsigned char *srcPtr, unsigned int srcPitch, unsigned char * /* deltaPtr */, unsigned char *dstPtr, unsigned int dstPitch, int width, int height)
{
	render_scanline_50((unsigned int*)dstPtr, (unsigned int*)srcPtr, dstPitch, srcPitch, width, height, MASK_DIV2_32);
}

// scanline 25%

// Mask constants.
#define MASK_DIV4_15		((unsigned short)(0x1CE7))
#define MASK_DIV4_16		((unsigned short)(0x39E7))
#define MASK_DIV4_32		((unsigned int)(0x3F3F3F3F))

template<typename pixel>
static inline void render_scanline_25(pixel *dst, pixel *src, int destPitch, int srcPitch, int width, int height, pixel mask2, pixel mask4)
{
	// Pitch difference.
	destPitch /= sizeof(pixel);
	int nextLine = destPitch + (destPitch - (width * 2));

	srcPitch /= sizeof(pixel);

	pixel *line1 = dst;
	pixel *line2 = dst + destPitch;
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			pixel tmp = *src++;

			// First line.
			*line1++ = tmp;
			*line1++ = tmp;

			// Second line. (25% Scanline effect)
			tmp = ((tmp >> 1) & mask2) + ((tmp >> 2) & mask4);
			*line2++ = tmp;
			*line2++ = tmp;
		}

		// Next line.
		src += (srcPitch - width);
		line1 += nextLine;
		line2 += nextLine;
	}
}

void Scanline25_15(unsigned char *srcPtr, unsigned int srcPitch, unsigned char * /* deltaPtr */, unsigned char *dstPtr, unsigned int dstPitch, int width, int height)
{
	render_scanline_25((unsigned short*)dstPtr, (unsigned short*)srcPtr, dstPitch, srcPitch, width, height, MASK_DIV2_15, MASK_DIV4_15);
}

void Scanline25_16(unsigned char *srcPtr, unsigned int srcPitch, unsigned char * /* deltaPtr */, unsigned char *dstPtr, unsigned int dstPitch, int width, int height)
{
	render_scanline_25((unsigned short*)dstPtr, (unsigned short*)srcPtr, dstPitch, srcPitch, width, height, MASK_DIV2_16, MASK_DIV4_16);
}

void Scanline25_32(unsigned char *srcPtr, unsigned int srcPitch, unsigned char * /* deltaPtr */, unsigned char *dstPtr, unsigned int dstPitch, int width, int height)
{
	render_scanline_25((unsigned int*)dstPtr, (unsigned int*)srcPtr, dstPitch, srcPitch, width, height, MASK_DIV2_32, MASK_DIV4_32);
}
