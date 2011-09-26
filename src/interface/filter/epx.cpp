// epxb, epxc filter, added by regret
// source from gens-rerecording (http://code.google.com/p/gens-rerecording/)

// EPX
template<typename pixel>
static inline void RenderEPX(pixel *src, unsigned int srcpitch, pixel *dst, unsigned int dstpitch, int width, int height)
{
	dstpitch /= sizeof(pixel);
	srcpitch /= sizeof(pixel);

	for (int y = 0; y < height; y++)
	{
		pixel *SrcLine = &src[y * srcpitch];
		pixel *DstLine1 = &dst[(y * 2) * dstpitch];
		pixel *DstLine2 = &dst[((y * 2) + 1) * dstpitch];

		for (int x = 0; x < width; x++)
		{
			pixel L = *(SrcLine - 1);
			pixel C = *(SrcLine);
			pixel R = *(SrcLine + 1);
			if (L != R)
			{
				pixel U = *(SrcLine - srcpitch);
				pixel D = *(SrcLine + srcpitch);
				if (U != D)
				{
					*DstLine1++ = (U == L) ? U : C;
					*DstLine1++ = (R == U) ? R : C;
					*DstLine2++ = (L == D) ? L : C;
					*DstLine2++ = (D == R) ? D : C;
					SrcLine++;
					continue;
				}
			}
			*DstLine1++ = C;
			*DstLine1++ = C;
			*DstLine2++ = C;
			*DstLine2++ = C;
			SrcLine++;
		}
	}
}

void EPX16(unsigned char *srcPtr, unsigned int srcPitch, unsigned char * /* deltaPtr */, unsigned char *dstPtr, unsigned int dstPitch, int width, int height)
{
	RenderEPX((unsigned short*)srcPtr, srcPitch, (unsigned short*)dstPtr, dstPitch, width, height);
}

void EPX32(unsigned char *srcPtr, unsigned int srcPitch, unsigned char * /* deltaPtr */, unsigned char *dstPtr, unsigned int dstPitch, int width, int height)
{
	RenderEPX((unsigned int*)srcPtr, srcPitch, (unsigned int*)dstPtr, dstPitch, width, height);
}


// EPX Plus

// Mask constants.
#define MASK_DIV2_15		((unsigned short)(0x3DEF))
#define MASK_DIV2_16		((unsigned short)(0x7BEF))
#define MASK_DIV2_32		((unsigned int)(0x7F7F7F7F))

#define BLEND(a, b, mask) ((((a) >> 1) & mask) + (((b) >> 1) & mask))

template<typename pixel>
static inline void RenderEPXPlus(pixel *src, unsigned int srcpitch, pixel *dst, unsigned int dstpitch, int width, int height, pixel mask)
{
	dstpitch /= sizeof(pixel);
	srcpitch /= sizeof(pixel);

	for (int y = 0; y < height; y++)
	{
		pixel *SrcLine = &src[y * srcpitch];
		pixel *DstLine1 = &dst[(y * 2) * dstpitch];
		pixel *DstLine2 = &dst[((y * 2) + 1) * dstpitch];

		pixel UL = *(SrcLine - srcpitch - 1);
		pixel U  = *(SrcLine - srcpitch);
		pixel UR = *(SrcLine - srcpitch + 1);
		pixel LL = *(SrcLine - 2);
		pixel L  = *(SrcLine - 1);
		pixel C  = *(SrcLine);
		pixel R  = *(SrcLine + 1);
		pixel RR = *(SrcLine + 2);
		pixel DL = *(SrcLine + srcpitch - 1);
		pixel D  = *(SrcLine + srcpitch);
		pixel DR = *(SrcLine + srcpitch + 1);

		for (int x = 0; x < width; x++)
		{
			UL = U, U = UR;
			LL = L, L = C, C = R, R = RR;
			DL = D, D = DR;

			UR = *(SrcLine - srcpitch + 1);
			RR = *(SrcLine + 2);
			DR = *(SrcLine + srcpitch + 1);

			pixel UU, DD;
			if (L != R && U != D)
			{
				UU = *(SrcLine - (2 * srcpitch));
				DD = *(SrcLine + (2 * srcpitch));
			}

			*DstLine1++ = (L != R && U != D && U == L && (UR == R || DL == D || R != RR || D != DD))
					? ((UR == DL && (L == LL || U == UU))      ? BLEND(C, U, mask) : U)
					: ((L == U && C == UR && L != C && L == D) ? BLEND(C, L, mask) : C);
			*DstLine1++ = (R != L && U != D && U == R && (UL == L || DR == D || L != LL || D != DD))
					? ((UL == DR && (R == RR || U == UU))      ? BLEND(C, U, mask) : U)
					: ((R == U && C == UL && R != C && R == D) ? BLEND(C, R, mask) : C);

			*DstLine2++ = (L != R && D != U && D == L && (DR == R || UL == U || R != RR || U != UU))
					? ((DR == UL && (L == LL || D == DD))      ? BLEND(C, D, mask) : D)
					: ((L == D && C == DR && L != C && L == U) ? BLEND(C, L, mask) : C);
			*DstLine2++ = (R != L && D != U && D == R && (DL == L || UR == U || L != LL || U != UU))
					? ((DL == UR && (R == RR || D == DD))      ? BLEND(C, D, mask) : D)
					: ((R == D && C == DL && R != C && R == U) ? BLEND(C, R, mask) : C);

			SrcLine++;
		}
	}
}

void EPXPlus15(unsigned char *srcPtr, unsigned int srcPitch, unsigned char * /* deltaPtr */, unsigned char *dstPtr, unsigned int dstPitch, int width, int height)
{
	RenderEPXPlus((unsigned short*)srcPtr, srcPitch, (unsigned short*)dstPtr, dstPitch, width, height, MASK_DIV2_15);
}

void EPXPlus16(unsigned char *srcPtr, unsigned int srcPitch, unsigned char * /* deltaPtr */, unsigned char *dstPtr, unsigned int dstPitch, int width, int height)
{
	RenderEPXPlus((unsigned short*)srcPtr, srcPitch, (unsigned short*)dstPtr, dstPitch, width, height, MASK_DIV2_16);
}

void EPXPlus32(unsigned char *srcPtr, unsigned int srcPitch, unsigned char * /* deltaPtr */, unsigned char *dstPtr, unsigned int dstPitch, int width, int height)
{
	RenderEPXPlus((unsigned int*)srcPtr, srcPitch, (unsigned int*)dstPtr, dstPitch, width, height, MASK_DIV2_32);
}
