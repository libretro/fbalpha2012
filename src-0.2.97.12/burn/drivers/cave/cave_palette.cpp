#include "cave.h"

unsigned char* CavePalSrc;
unsigned char CaveRecalcPalette;	// Set to 1 to force recalculation of the entire palette

unsigned int* CavePalette = NULL;
static unsigned short* CavePalCopy = NULL;

int CavePalInit(int nPalSize)
{
	CavePalette = (unsigned int*)malloc(nPalSize * sizeof(int));
	memset(CavePalette, 0, nPalSize * sizeof(int));

	CavePalCopy = (unsigned short*)malloc(nPalSize * sizeof(short));
	memset(CavePalCopy, 0, nPalSize * sizeof(short));
	
	pBurnDrvPalette = CavePalette;

	return 0;
}

int CavePalExit()
{
	free(CavePalette);
	CavePalette = NULL;
	free(CavePalCopy);
	CavePalCopy = NULL;

	return 0;
}

inline static unsigned int CalcCol(unsigned short nColour)
{
	int r, g, b;

	r = (nColour & 0x03E0) >> 2;	// Red
	r |= r >> 5;
	g = (nColour & 0x7C00) >> 7;  	// Green
	g |= g >> 5;
	b = (nColour & 0x001F) << 3;	// Blue
	b |= b >> 5;

	return BurnHighCol(r, g, b, 0);
}

int CavePalUpdate4Bit(int nOffset, int nNumPalettes)
{
	int i, j;

	unsigned short* ps = (unsigned short*)CavePalSrc + nOffset;
	unsigned short* pc;
	unsigned int* pd;

	unsigned short c;

	if (CaveRecalcPalette) {

		for (i = 0; i < 0 + nNumPalettes; i++) {

			pc = CavePalCopy + (i << 8);
			pd = CavePalette + (i << 8);

			for (j = 0; j < 16; j++, ps++, pc++, pd++) {

				c = *ps;
				*pc = c;
				*pd = CalcCol(c);

			}
		}

		CaveRecalcPalette = 0;
		return 0;
	}


	for (i = 0; i < 0 + nNumPalettes; i++) {

		pc = CavePalCopy + (i << 8);
		pd = CavePalette + (i << 8);

		for (j = 0; j < 16; j++, ps++, pc++, pd++) {

			c = *ps;
			if (*pc != c) {
				*pc = c;
				*pd = CalcCol(c);
			}

		}
	}

	return 0;
}

int CavePalUpdate8Bit(int nOffset, int nNumPalettes)
{
	if (CaveRecalcPalette) {
		int i, j;

		unsigned short* ps = (unsigned short*)CavePalSrc + nOffset;
		unsigned short* pc;
		unsigned int* pd;

		unsigned short c;

		for (i = 0; i < nNumPalettes; i++) {

			pc = CavePalCopy + nOffset + (i << 8);
			pd = CavePalette + nOffset + (i << 8);

			for (j = 0; j < 256; j++, ps++, pc++, pd++) {

				c = *ps;
				*pc = c;
				*pd = CalcCol(c);

			}
		}

		CaveRecalcPalette = 0;
	}

	return 0;
}

// Update the PC copy of the palette on writes to the palette memory
void CavePalWriteByte(unsigned int nAddress, unsigned char byteValue)
{
	nAddress ^= 1;
	CavePalSrc[nAddress] = byteValue;							// write byte

	if (*((unsigned char*)(CavePalCopy + nAddress)) != byteValue) {
		*((unsigned char*)(CavePalCopy + nAddress)) = byteValue;
		CavePalette[nAddress >> 1] = CalcCol(*(unsigned short*)(CavePalSrc + (nAddress & ~0x01)));
	}
}

void CavePalWriteWord(unsigned int nAddress, unsigned short wordValue)
{
	nAddress >>= 1;

	((unsigned short*)CavePalSrc)[nAddress] = wordValue;		// write word

	if (CavePalCopy[nAddress] != wordValue) {
		CavePalCopy[nAddress] = wordValue;
		CavePalette[nAddress] = CalcCol(wordValue);
	}
}

