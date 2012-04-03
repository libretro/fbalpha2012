#include "psikyo.h"

UINT8* PsikyoPalSrc;
UINT8 PsikyoRecalcPalette;	// Set to 1 to force recalculation of the entire palette

UINT32* PsikyoPalette;
static UINT16* PsikyoPalCopy;

INT32 PsikyoPalInit()
{
	PsikyoPalette = (UINT32*)BurnMalloc(0x1000 * sizeof(UINT32));
	memset(PsikyoPalette, 0, 0x1000 * sizeof(UINT32));

	PsikyoPalCopy = (UINT16*)BurnMalloc(0x1000 * sizeof(UINT16));
	memset(PsikyoPalCopy, 0, 0x1000 * sizeof(UINT16));
	
	pBurnDrvPalette = PsikyoPalette;

	return 0;
}

INT32 PsikyoPalExit()
{
	BurnFree(PsikyoPalette);
	BurnFree(PsikyoPalCopy);

	return 0;
}

#ifndef __LIBSNES_OPTIMIZATIONS__
inline static UINT32 CalcCol(UINT16 nColour)
{
	INT32 r, g, b;

	r = (nColour & 0x7C00) >> 7;  	// Red
	r |= r >> 5;
	g = (nColour & 0x03E0) >> 2;	// Green
	g |= g >> 5;
	b = (nColour & 0x001F) << 3;	// Blue
	b |= b >> 5;

	return BurnHighCol(r, g, b, 0);
}

INT32 PsikyoPalUpdate()
{
	if (PsikyoRecalcPalette) {
		UINT16 c;

		for (INT32 i = 0; i < 0x1000; i++) {
			c = ((UINT16*)PsikyoPalSrc)[i];
			PsikyoPalCopy[i] = c;
			PsikyoPalette[i] = CalcCol(c);
		}

		PsikyoRecalcPalette = 0;
	}

	return 0;
}
#endif

// Update the PC copy of the palette on writes to the palette memory
void PsikyoPalWriteByte(UINT32 nAddress, UINT8 byteValue)
{
	nAddress ^= 1;
	PsikyoPalSrc[nAddress] = byteValue;							// write byte

	if (*((UINT8*)(PsikyoPalCopy + nAddress)) != byteValue) {
		*((UINT8*)(PsikyoPalCopy + nAddress)) = byteValue;
		UINT16 c = (*((UINT16*)(PsikyoPalSrc + (nAddress & ~0x01))));
#ifdef __LIBSNES_OPTIMIZATIONS__
		PsikyoPalette[nAddress >> 1] = c;
#else
		PsikyoPalette[nAddress >> 1] = CalcCol(c);
#endif
	}
}

void PsikyoPalWriteWord(UINT32 nAddress, UINT16 wordValue)
{
	nAddress >>= 1;

	((UINT16*)PsikyoPalSrc)[nAddress] = wordValue;		// write word

	if (PsikyoPalCopy[nAddress] != wordValue) {
		PsikyoPalCopy[nAddress] = wordValue;
#ifdef __LIBSNES_OPTIMIZATIONS__
		PsikyoPalette[nAddress] = wordValue;
#else
		PsikyoPalette[nAddress] = CalcCol(wordValue);
#endif
	}
}
