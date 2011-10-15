#include "neogeo.h"
// Neo Geo -- palette functions

unsigned char* NeoPalSrc[2];		// Pointer to input palettes
unsigned int* NeoPalette;
int nNeoPaletteBank;				// Selected palette bank

unsigned int* NeoPaletteData[2] = {NULL, NULL};
static unsigned short* NeoPaletteCopy[2] = {NULL, NULL};

unsigned char NeoRecalcPalette;

int NeoInitPalette()
{
	for (int i = 0; i < 2; i++) {
		free(NeoPaletteData[i]);
		free(NeoPaletteCopy[i]);
		NeoPaletteData[i] = (unsigned int*)malloc(4096 * sizeof(int));
		NeoPaletteCopy[i] = (unsigned short*)malloc(4096 * sizeof(short));
	}

	NeoRecalcPalette = 1;

	return 0;
}

void NeoExitPalette()
{
	for (int i = 0; i < 2; i++) {
		free(NeoPaletteData[i]);
		free(NeoPaletteCopy[i]);
		NeoPaletteData[i] = NULL;
		NeoPaletteCopy[i] = NULL;
	}
}

inline static unsigned int CalcCol(unsigned short nColour)
{
	int r = (nColour & 0x0F00) >> 4;	// Red
	r |= (nColour >> 11) & 8;
	int g = (nColour & 0x00F0);			// Green
	g |= (nColour >> 10) & 8;
	int b = (nColour & 0x000F) << 4;	// Blue
	b |= (nColour >> 9) & 8;

	r |= r >> 5;
	g |= g >> 5;
	b |= b >> 5;

	return BurnHighCol(r, g, b, 0);
}

int NeoUpdatePalette()
{
	if (NeoRecalcPalette) {
		int i;
		unsigned short* ps;
		unsigned short* pc;
		unsigned int* pd;

		// Update both palette banks
		for (int j = 0; j < 2; j++) {
			for (i = 0, ps = (unsigned short*)NeoPalSrc[j], pc = NeoPaletteCopy[j], pd = NeoPaletteData[j]; i < 4096; i++, ps++, pc++, pd++) {
				*pc = *ps;
				*pd = CalcCol(*ps);
			}
		}

		NeoRecalcPalette = 0;

	}

	return 0;
}

void NeoSetPalette()
{
	NeoPalette = NeoPaletteData[nNeoPaletteBank];
	pBurnDrvPalette = NeoPaletteData[nNeoPaletteBank];
}

// Update the PC copy of the palette on writes to the palette memory
void __fastcall NeoPalWriteByte(unsigned int nAddress, unsigned char byteValue)
{
	nAddress &= 0x1FFF;
	nAddress ^= 1;

	NeoPalSrc[nNeoPaletteBank][nAddress] = byteValue;							// write byte

	if (*((unsigned char*)(NeoPaletteCopy[nNeoPaletteBank] + nAddress)) != byteValue) {
		*((unsigned char*)(NeoPaletteCopy[nNeoPaletteBank] + nAddress)) = byteValue;
		NeoPaletteData[nNeoPaletteBank][nAddress >> 1] = CalcCol(*(unsigned short*)(NeoPalSrc[nNeoPaletteBank] + (nAddress & ~0x01)));
	}
}

void __fastcall NeoPalWriteWord(unsigned int nAddress, unsigned short wordValue)
{
	nAddress &= 0x1FFF;
	nAddress >>= 1;

	((unsigned short*)NeoPalSrc[nNeoPaletteBank])[nAddress] = wordValue;		// write word

	if (NeoPaletteCopy[nNeoPaletteBank][nAddress] != wordValue) {
		NeoPaletteCopy[nNeoPaletteBank][nAddress] = wordValue;
		NeoPaletteData[nNeoPaletteBank][nAddress] = CalcCol(wordValue);
	}
}

