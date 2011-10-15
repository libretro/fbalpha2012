#include "toaplan.h"
// Toaplan -- palette functions

unsigned char* ToaPalSrc;			// Pointer to input palette
unsigned char* ToaPalSrc2;
unsigned int* ToaPalette;
unsigned int* ToaPalette2;
int nToaPalLen;

unsigned char ToaRecalcPalette;		// Set to 1 to force recalculation of the entire palette (not needed now)

int ToaPalInit()
{
	return 0;
}

int ToaPalExit()
{
	return 0;
}

inline static unsigned int CalcCol(unsigned short nColour)
{
	int r, g, b;

	r = (nColour & 0x001F) << 3;	// Red
	r |= r >> 5;
	g = (nColour & 0x03E0) >> 2;  	// Green
	g |= g >> 5;
	b = (nColour & 0x7C00) >> 7;	// Blue
	b |= b >> 5;

	return BurnHighCol(r, g, b, 0);
}

int ToaPalUpdate()
{
	unsigned short* ps = (unsigned short*)ToaPalSrc;
	unsigned int* pd = ToaPalette;
	
	pBurnDrvPalette = ToaPalette;

	for (int i = 0; i < nToaPalLen; i++) {
		pd[i] = CalcCol(ps[i]);
	}
	return 0;
}

int ToaPal2Update()
{
	unsigned short* ps = (unsigned short*)ToaPalSrc2;
	unsigned int* pd = ToaPalette2;
	
//	pBurnDrvPalette = ToaPalette2;

	for (int i = 0; i < nToaPalLen; i++) {
		pd[i] = CalcCol(ps[i]);
	}
	return 0;
}
