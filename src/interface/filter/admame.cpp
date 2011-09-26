/*
 * This file is part of the Advance project.
 *
 * Copyright (C) 1999-2002 Andrea Mazzoleni
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * In addition, as a special exception, Andrea Mazzoleni
 * gives permission to link the code of this program with
 * the MAME library (or with modified versions of MAME that use the
 * same license as MAME), and distribute linked combinations including
 * the two.  You must obey the GNU General Public License in all
 * respects for all of the code used other than MAME.  If you modify
 * this file, you may extend this exception to your version of the
 * file, but you are not obligated to do so.  If you do not wish to
 * do so, delete this exception statement from your version.
 */

/*
 * Alternatively at the previous license terms, you are allowed to use this
 * code in your program with these conditions:
 * - the program is not used in commercial activities.
 * - the whole source code of the program is released with the binary.
 */

static inline void internal_scale2x_16_def(unsigned short *dst, const unsigned short* src0, const unsigned short* src1, const unsigned short* src2, unsigned count)
{
	/* first pixel */
	dst[0] = src1[0];
	if (src1[1] == src0[0] && src2[0] != src0[0])
		dst[1] = src0[0];
	else
		dst[1] = src1[0];
	++src0;
	++src1;
	++src2;
	dst += 2;

	/* central pixels */
	count -= 2;
	while (count) {
		if (src0[0] != src2[0] && src1[-1] != src1[1]) {
			dst[0] = src1[-1] == src0[0] ? src0[0] : src1[0];
			dst[1] = src1[1] == src0[0] ? src0[0] : src1[0];
		} else {
			dst[0] = src1[0];
			dst[1] = src1[0];
		}

		++src0;
		++src1;
		++src2;
		dst += 2;
		--count;
	}

	/* last pixel */
	if (src1[-1] == src0[0] && src2[0] != src0[0])
		dst[0] = src0[0];
	else
		dst[0] = src1[0];
	dst[1] = src1[0];
}

static inline void internal_scale2x_32_def(unsigned int* dst, const unsigned int* src0, const unsigned int* src1, const unsigned int* src2, unsigned count)
{
	/* first pixel */
	dst[0] = src1[0];
	if (src1[1] == src0[0] && src2[0] != src0[0])
		dst[1] = src0[0];
	else
		dst[1] = src1[0];
	++src0;
	++src1;
	++src2;
	dst += 2;

	/* central pixels */
	count -= 2;
	while (count) {
		if (src0[0] != src2[0] && src1[-1] != src1[1]) {
			dst[0] = src1[-1] == src0[0] ? src0[0] : src1[0];
			dst[1] = src1[1] == src0[0] ? src0[0] : src1[0];
		} else {
			dst[0] = src1[0];
			dst[1] = src1[0];
		}

		++src0;
		++src1;
		++src2;
		dst += 2;
		--count;
	}

	/* last pixel */
	if (src1[-1] == src0[0] && src2[0] != src0[0])
		dst[0] = src0[0];
	else
		dst[0] = src1[0];
	dst[1] = src1[0];
}

void AdMame2x(unsigned char *srcPtr, unsigned int srcPitch, unsigned char * /* deltaPtr */,
				unsigned char *dstPtr, unsigned int dstPitch, int width, int height)
{
	unsigned short *dst0 = (unsigned short *)dstPtr;
	unsigned short *dst1 = dst0 + (dstPitch >> 1);

	unsigned short *src0 = (unsigned short *)srcPtr;
	unsigned short *src1 = src0 + (srcPitch >> 1);
	unsigned short *src2 = src1 + (srcPitch >> 1);

	internal_scale2x_16_def(dst0, src0, src0, src1, width);
	internal_scale2x_16_def(dst1, src1, src0, src0, width);

	int count = height;

	count -= 2;
	while (count) {
		dst0 += dstPitch;
		dst1 += dstPitch;
		internal_scale2x_16_def(dst0, src0, src1, src2, width);
		internal_scale2x_16_def(dst1, src2, src1, src0, width);
		src0 = src1;
		src1 = src2;
		src2 += srcPitch >> 1;
		--count;
	}
	dst0 += dstPitch;
	dst1 += dstPitch;
	internal_scale2x_16_def(dst0, src0, src1, src1, width);
	internal_scale2x_16_def(dst1, src1, src1, src0, width);
}

void AdMame2x32(unsigned char *srcPtr, unsigned int srcPitch, unsigned char * /* deltaPtr */,
					unsigned char *dstPtr, unsigned int dstPitch, int width, int height)
{
	unsigned int *dst0 = (unsigned int *)dstPtr;
	unsigned int *dst1 = dst0 + (dstPitch >> 2);

	unsigned int *src0 = (unsigned int *)srcPtr;
	unsigned int *src1 = src0 + (srcPitch >> 2);
	unsigned int *src2 = src1 + (srcPitch >> 2);

	internal_scale2x_32_def(dst0, src0, src0, src1, width);
	internal_scale2x_32_def(dst1, src1, src0, src0, width);

	int count = height;

	count -= 2;
	while (count) {
		dst0 += dstPitch >> 1;
		dst1 += dstPitch >> 1;
		internal_scale2x_32_def(dst0, src0, src1, src2, width);
		internal_scale2x_32_def(dst1, src2, src1, src0, width);
		src0 = src1;
		src1 = src2;
		src2 += srcPitch >> 2;
		--count;
	}
	dst0 += dstPitch >> 1;
	dst1 += dstPitch >> 1;
	internal_scale2x_32_def(dst0, src0, src1, src1, width);
	internal_scale2x_32_def(dst1, src1, src1, src0, width);
}


/***************************************************************************/
/* Scale3x C implementation */

static inline void scale3x_16_def_single(unsigned short* dst, const unsigned short* src0, const unsigned short* src1, const unsigned short* src2, unsigned count)
{
//	assert(count >= 2);

	/* first pixel */
	dst[0] = src1[0];
	dst[1] = src1[0];
	if (src1[1] == src0[0] && src2[0] != src0[0])
		dst[2] = src0[0];
	else
		dst[2] = src1[0];
	++src0;
	++src1;
	++src2;
	dst += 3;

	/* central pixels */
	count -= 2;
	while (count) {
		if (src1[-1] == src0[0] && src2[0] != src0[0] && src1[1] != src0[0])
			dst[0] = src0[0];
		else
			dst[0] = src1[0];
		dst[1] = src1[0];
		if (src1[1] == src0[0] && src2[0] != src0[0] && src1[-1] != src0[0])
			dst[2] = src0[0];
		else
			dst[2] = src1[0];

		++src0;
		++src1;
		++src2;

		dst += 3;
		--count;
	}

	/* last pixel */
	if (src1[-1] == src0[0] && src2[0] != src0[0])
		dst[0] = src0[0];
	else
		dst[0] = src1[0];
	dst[1] = src1[0];
	dst[2] = src1[0];
}

static inline void scale3x_16_def_fill(unsigned short* dst, const unsigned short* src, unsigned count)
{
	while (count) {
		dst[0] = src[0];
		dst[1] = src[0];
		dst[2] = src[0];

		++src;
		dst += 3;
		--count;
	}
}

static inline void scale3x_32_def_single(unsigned int* dst, const unsigned int* src0, const unsigned int* src1, const unsigned int* src2, unsigned count)
{
//	assert(count >= 2);

	/* first pixel */
	dst[0] = src1[0];
	dst[1] = src1[0];
	if (src1[1] == src0[0] && src2[0] != src0[0])
		dst[2] = src0[0];
	else
		dst[2] = src1[0];
	++src0;
	++src1;
	++src2;
	dst += 3;

	/* central pixels */
	count -= 2;
	while (count) {
		if (src1[-1] == src0[0] && src2[0] != src0[0] && src1[1] != src0[0])
			dst[0] = src0[0];
		else
			dst[0] = src1[0];
		dst[1] = src1[0];
		if (src1[1] == src0[0] && src2[0] != src0[0] && src1[-1] != src0[0])
			dst[2] = src0[0];
		else
			dst[2] = src1[0];

		++src0;
		++src1;
		++src2;

		dst += 3;
		--count;
	}

	/* last pixel */
	if (src1[-1] == src0[0] && src2[0] != src0[0])
		dst[0] = src0[0];
	else
		dst[0] = src1[0];
	dst[1] = src1[0];
	dst[2] = src1[0];
}

static inline void scale3x_32_def_fill(unsigned int* dst, const unsigned int* src, unsigned count)
{
	while (count) {
		dst[0] = src[0];
		dst[1] = src[0];
		dst[2] = src[0];

		++src;
		dst += 3;
		--count;
	}
}

/**
 * Scale by a factor of 3 a row of pixels of 16 bits.
 * This function operates like scale3x_8_def() but for 16 bits pixels.
 * \param src0 Pointer at the first pixel of the previous row.
 * \param src1 Pointer at the first pixel of the current row.
 * \param src2 Pointer at the first pixel of the next row.
 * \param count Length in pixels of the src0, src1 and src2 rows.
 * It must be at least 2.
 * \param dst0 First destination row, triple length in pixels.
 * \param dst1 Second destination row, triple length in pixels.
 * \param dst2 Third destination row, triple length in pixels.
 */
static void scale3x_16_def(unsigned short* dst0, unsigned short* dst1, unsigned short* dst2, const unsigned short* src0, const unsigned short* src1, const unsigned short* src2, unsigned count)
{
//	assert(count >= 2);

	scale3x_16_def_single(dst0, src0, src1, src2, count);
	scale3x_16_def_fill(dst1, src1, count);
	scale3x_16_def_single(dst2, src2, src1, src0, count);
}

/**
 * Scale by a factor of 3 a row of pixels of 32 bits.
 * This function operates like scale3x_8_def() but for 32 bits pixels.
 * \param src0 Pointer at the first pixel of the previous row.
 * \param src1 Pointer at the first pixel of the current row.
 * \param src2 Pointer at the first pixel of the next row.
 * \param count Length in pixels of the src0, src1 and src2 rows.
 * It must be at least 2.
 * \param dst0 First destination row, triple length in pixels.
 * \param dst1 Second destination row, triple length in pixels.
 * \param dst2 Third destination row, triple length in pixels.
 */
static void scale3x_32_def(unsigned int* dst0, unsigned int* dst1, unsigned int* dst2, const unsigned int* src0, const unsigned int* src1, const unsigned int* src2, unsigned count)
{
//	assert(count >= 2);

	scale3x_32_def_single(dst0, src0, src1, src2, count);
	scale3x_32_def_fill(dst1, src1, count);
	scale3x_32_def_single(dst2, src2, src1, src0, count);
}

void AdMame3x(unsigned char *srcPtr, unsigned int srcPitch, unsigned char * /* deltaPtr */,
                unsigned char *dstPtr, unsigned int dstPitch, int width, int height)
{
	int y;
	unsigned char *src_prev = srcPtr;
	unsigned char *src_curr = srcPtr;
	unsigned char *src_next = srcPtr + srcPitch;

	int depth = 16;

	scale3x_16_def((unsigned short *)dstPtr, (unsigned short *)(dstPtr + dstPitch), (unsigned short *)(dstPtr + 2 * dstPitch), (unsigned short *)src_prev, (unsigned short *)src_curr, (unsigned short *)src_next, width);

	for (y = 2; y < height; y++)
	{
		dstPtr	+= 3 * dstPitch;
		src_prev = src_curr;
		src_curr = src_next;
		src_next += srcPitch;
		scale3x_16_def((unsigned short *)dstPtr, (unsigned short *)(dstPtr + dstPitch), (unsigned short *)(dstPtr + 2 * dstPitch), (unsigned short *)src_prev, (unsigned short *)src_curr, (unsigned short *)src_next, width);
	}

	dstPtr += 3 * dstPitch;
	src_prev = src_curr;
	src_curr = src_next;
	scale3x_16_def((unsigned short *)dstPtr, (unsigned short *)(dstPtr + dstPitch), (unsigned short *)(dstPtr + 2 * dstPitch), (unsigned short *)src_prev, (unsigned short *)src_curr, (unsigned short *)src_next, width);
}

void AdMame3x32(unsigned char *srcPtr, unsigned int srcPitch, unsigned char * /* deltaPtr */,
                unsigned char *dstPtr, unsigned int dstPitch, int width, int height)
{
	int y;
	unsigned char *src_prev = srcPtr;
	unsigned char *src_curr = srcPtr;
	unsigned char *src_next = srcPtr + srcPitch;

	int depth = 32;

	scale3x_32_def((unsigned int *)dstPtr, (unsigned int *)(dstPtr + dstPitch), (unsigned int *)(dstPtr + 2 * dstPitch), (unsigned int *)src_prev, (unsigned int *)src_curr, (unsigned int *)src_next, width);

	for (y = 2; y < height; y++)
	{
		dstPtr += 3 * dstPitch;
		src_prev = src_curr;
		src_curr = src_next;
		src_next += srcPitch;
		scale3x_32_def((unsigned int *)dstPtr, (unsigned int *)(dstPtr + dstPitch), (unsigned int *)(dstPtr + 2 * dstPitch), (unsigned int *)src_prev, (unsigned int *)src_curr, (unsigned int *)src_next, width);
	}

	dstPtr += 3 * dstPitch;
	src_prev = src_curr;
	src_curr = src_next;
	scale3x_32_def((unsigned int *)dstPtr, (unsigned int *)(dstPtr + dstPitch), (unsigned int *)(dstPtr + 2 * dstPitch), (unsigned int *)src_prev, (unsigned int *)src_curr, (unsigned int *)src_next, width);
}
