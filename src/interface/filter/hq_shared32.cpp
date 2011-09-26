// VisualBoyAdvance - Nintendo Gameboy/GameboyAdvance (TM) emulator.
// Copyright (C) 1999-2003 Forgotten
// Copyright (C) 2005 Forgotten and the VBA development team

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or(at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

#include "hq_shared32.h"

#if defined (_XBOX)
const unsigned __int64 reg_blank = 0x0000000000000000;
const unsigned __int64 const7    = 0x0000000700070007;
const unsigned __int64 treshold  = 0x0000000000300706;
#else
long long reg_blank = 0x0000000000000000LLU;
long long const7    = 0x0000000700070007LLU;
long long treshold  = 0x0000000000300706LLU;
#endif

void Interp1(unsigned char * pc, unsigned int c1, unsigned int c2)
{
	//*((int*)pc) = (c1*3+c2)/4;

	*( (unsigned int *)(pc) ) = ( (c1) == (c2) ) ? c1 :
	(
		( (
			( ( (c1) & 0x00FF00 ) * 3 ) +
			( (c2) & 0x00FF00 )
		) & 0x0003FC00 )
		+
		( (
			( ( (c1) & 0xFF00FF ) * 3 ) +
			( (c2) & 0xFF00FF )
		) & 0x03FC03FC )
	) >> 2;
}

void Interp2(unsigned char * pc, unsigned int c1, unsigned int c2, unsigned int c3)
{
	//*((int*)pc) = (c1*2+c2+c3)/4;

	*( (unsigned int *)(pc) ) =
	( ( (c1) == (c2) ) == (c3) ) ? c1 :
	(
		( (
			( ( (c1) & 0x00FF00 ) * 2 ) +
			( (c2) & 0x00FF00 ) +
			( (c3) & 0x00FF00 )
		) & 0x0003FC00 )
		+
		( (
			( ( (c1) & 0xFF00FF ) * 2 ) +
			( (c2) & 0xFF00FF ) +
			( (c3) & 0xFF00FF )
		) & 0x03FC03FC )
	) >> 2;
}

void Interp3(unsigned char * pc, unsigned int c1, unsigned int c2)
{
	//*((int*)pc) = (c1*7+c2)/8;
	//*((int*)pc) = ((((c1 & 0x00FF00)*7 + (c2 & 0x00FF00) ) & 0x0007F800) +
	//	            (((c1 & 0xFF00FF)*7 + (c2 & 0xFF00FF) ) & 0x07F807F8)) >> 3;

	*( (unsigned int *)(pc) ) =
	( (c1) == (c2) ) ? c1 :
	(
		( (
			( ( (c1) & 0x00FF00 ) * 7 ) +
			( (c2) & 0x00FF00 )
		) & 0x0007F800 )
		+
		( (
			( ( (c1) & 0xFF00FF ) * 7 ) +
			( (c2) & 0xFF00FF )
		) & 0x07F807F8 )
	) >> 3;
}

void Interp4(unsigned char * pc, unsigned int c1, unsigned int c2, unsigned int c3)
{
	//*((int*)pc) = (c1*2+(c2+c3)*7)/16;
	//*((int*)pc) = ((((c1 & 0x00FF00)*2 + ((c2 & 0x00FF00) + (c3 & 0x00FF00))*7 ) & 0x000FF000) +
	//              (((c1 & 0xFF00FF)*2 + ((c2 & 0xFF00FF) + (c3 & 0xFF00FF))*7 ) & 0x0FF00FF0)) >> 4;

	*( (unsigned int *)(pc) ) =
	( ( (c1) == (c2) ) == (c3) ) ? c1 :
	(
	( ( ( ( (c1) & 0x00FF00 ) * 2 ) + ( ( ( (c2) & 0x00FF00 ) + ( (c3) & 0x00FF00 ) ) * 7 ) ) & 0x000FF000 ) +
	( ( ( ( (c1) & 0xFF00FF ) * 2 ) + ( ( ( (c2) & 0xFF00FF ) + ( (c3) & 0xFF00FF ) ) * 7 ) ) & 0x0FF00FF0 )
	) >> 4;
}

void Interp5(unsigned char * pc, unsigned int c1, unsigned int c2)
{
	//*((int*)pc) = (c1+c2)/2;

	*( (unsigned int *)(pc) ) =
	( (c1) == (c2) ) ? c1 :
	(
		( (
			( (c1) & 0x00FF00 ) +
			( (c2) & 0x00FF00 )
		) & 0x0001FE00 )
		+
		( (
			( (c1) & 0xFF00FF ) +
			( (c2) & 0xFF00FF )
		) & 0x01FE01FE )
	) >> 1;
}


#include "interp.h"

void Interp1_16(unsigned char * pc, unsigned short c1, unsigned short c2)
{
	*((unsigned short*)pc) = interp_16_31(c1, c2);
	//*((int*)pc) = (c1*3+c2)/4;
}

void Interp2_16(unsigned char * pc, unsigned short c1, unsigned short c2, unsigned short c3)
{
	*((unsigned short*)pc) = interp_16_211(c1, c2, c3);
	//*((int*)pc) = (c1*2+c2+c3)/4;
}

void Interp3_16(unsigned char * pc, unsigned short c1, unsigned short c2)
{
	*((unsigned short*)pc) = interp_16_71(c1, c2);
//	*((unsigned short*)pc) = (c1*7+c2)/8;
//	*((unsigned short*)pc) = ((((c1 & 0x00FF00)*7 + (c2 & 0x00FF00) ) & 0x0007F800) +
//		            (((c1 & 0xFF00FF)*7 + (c2 & 0xFF00FF) ) & 0x07F807F8)) >> 3;
}

void Interp4_16(unsigned char * pc, unsigned short c1, unsigned short c2, unsigned short c3)
{
	*((unsigned short*)pc) = interp_16_772(c2, c3, c1);
//	*((unsigned short*)pc) = (c1*2+(c2+c3)*7)/16;
//	*((unsigned short*)pc) = ((((c1 & 0x00FF00)*2 + ((c2 & 0x00FF00) + (c3 & 0x00FF00))*7 ) & 0x000FF000) +
//	              (((c1 & 0xFF00FF)*2 + ((c2 & 0xFF00FF) + (c3 & 0xFF00FF))*7 ) & 0x0FF00FF0)) >> 4;
}

void Interp5_16(unsigned char * pc, unsigned short c1, unsigned short c2)
{
	*((unsigned short*)pc) = interp_16_11(c1, c2);
}


bool Diff(unsigned int c1, unsigned int c2)
{
	unsigned int
	YUV1 = RGBtoYUV(c1),
	YUV2 = RGBtoYUV(c2);

	if (YUV1 == YUV2) return false; // Save some processing power

	return
		( abs32((YUV1 & Ymask) - (YUV2 & Ymask)) > trY ) ||
		( abs32((YUV1 & Umask) - (YUV2 & Umask)) > trU ) ||
		( abs32((YUV1 & Vmask) - (YUV2 & Vmask)) > trV );
}


unsigned int RGBtoYUV(unsigned int c)
{
	// Division through 3 slows down the emulation about 10% !!!

	register unsigned char r, g, b;
	b = c & 0x0000FF;
	g = ( c & 0x00FF00 ) >> 8;
	r = c >> 16;
	return ( (r + g + b) << 14 ) +
		( ( r - b + 512 ) << 4 ) +
		( ( 2*g - r - b ) >> 3 ) + 128;

	// Extremely High Quality Code
	//unsigned char r, g, b;
	//r = c & 0xFF;
	//g = (c >> 8) & 0xFF;
	//b = (c >> 16) & 0xFF;
	//unsigned char y, u, v;
	//y = (0.256788 * r  +  0.504129 * g  +  0.097906 * b) + 16;
	//u = (-0.148223 * r  -  0.290993 * g  +  0.439216 * b) + 128;
	//v = (0.439216 * r  -  0.367788 * g  -  0.071427 * b) + 128;
	//return (y << 16) + (u << 8) + v;
}
