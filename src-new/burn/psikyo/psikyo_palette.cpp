#include "psikyo.h"

unsigned char* PsikyoPalSrc;
unsigned char PsikyoRecalcPalette;	// Set to 1 to force recalculation of the entire palette

unsigned int* PsikyoPalette;
static unsigned short* PsikyoPalCopy;

int PsikyoPalInit()
{
	PsikyoPalette = (unsigned int*)malloc(0x1000 * sizeof(int));
	memset(PsikyoPalette, 0, 0x1000 * sizeof(int));

	PsikyoPalCopy = (unsigned short*)malloc(0x1000 * sizeof(short));
	memset(PsikyoPalCopy, 0, 0x1000 * sizeof(short));
	
	pBurnDrvPalette = PsikyoPalette;

	return 0;
}

int PsikyoPalExit()
{
	free(PsikyoPalette);
	PsikyoPalette = NULL;
	free(PsikyoPalCopy);
	PsikyoPalCopy = NULL;

	return 0;
}

inline static unsigned int CalcCol(unsigned short nColour)
{
	int r, g, b;

	r = (nColour & 0x7C00) >> 7;  	// Red
	r |= r >> 5;
	g = (nColour & 0x03E0) >> 2;	// Green
	g |= g >> 5;
	b = (nColour & 0x001F) << 3;	// Blue
	b |= b >> 5;

	return BurnHighCol(r, g, b, 0);
}

int PsikyoPalUpdate()
{
	if (PsikyoRecalcPalette) {
		unsigned short c;

		for (int i = 0; i < 0x1000; i++) {
			c = ((unsigned short*)PsikyoPalSrc)[i];
			PsikyoPalCopy[i] = c;
			PsikyoPalette[i] = CalcCol(c);
		}

		PsikyoRecalcPalette = 0;
	}

	return 0;
}

// Update the PC copy of the palette on writes to the palette memory
void PsikyoPalWriteByte(unsigned int nAddress, unsigned char byteValue)
{
	nAddress ^= 1;
	PsikyoPalSrc[nAddress] = byteValue;							// write byte

	if (*((unsigned char*)(PsikyoPalCopy + nAddress)) != byteValue) {
		*((unsigned char*)(PsikyoPalCopy + nAddress)) = byteValue;
		PsikyoPalette[nAddress >> 1] = CalcCol(*(unsigned short*)(PsikyoPalSrc + (nAddress & ~0x01)));
	}
}

void PsikyoPalWriteWord(unsigned int nAddress, unsigned short wordValue)
{
	nAddress >>= 1;

	((unsigned short*)PsikyoPalSrc)[nAddress] = wordValue;		// write word

	if (PsikyoPalCopy[nAddress] != wordValue) {
		PsikyoPalCopy[nAddress] = wordValue;
		PsikyoPalette[nAddress] = CalcCol(wordValue);
	}
}

