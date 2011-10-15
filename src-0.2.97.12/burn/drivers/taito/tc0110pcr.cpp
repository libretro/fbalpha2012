// TC0110PCR

#include "burnint.h"
#include "taito_ic.h"

#define MAX_TC0110PCR		3

static unsigned char *TC0110PCRRam[MAX_TC0110PCR];
unsigned int *TC0110PCRPalette = NULL;
static int TC0110PCRAddr[MAX_TC0110PCR];
int TC0110PCRTotalColours;

static inline unsigned char pal5bit(unsigned char bits)
{
	bits &= 0x1f;
	return (bits << 3) | (bits >> 2);
}

static inline unsigned char pal4bit(unsigned char bits)
{
	bits &= 0x0f;
	return (bits << 4) | (bits);
}

UINT16 TC0110PCRWordRead(int Chip)
{
	UINT16 *PalRam = (UINT16*)TC0110PCRRam[Chip];
	return PalRam[TC0110PCRAddr[Chip]];
}

void TC0110PCRWordWrite(int Chip, int Offset, UINT16 Data)
{
	int PaletteOffset = Chip * 0x1000;
	
	switch (Offset) {
		case 0: {
			TC0110PCRAddr[Chip] = (Data >> 1) & 0xfff;
			return;
		}

		case 1:	{
			UINT16 *PalRam = (UINT16*)TC0110PCRRam[Chip];
			int r, g, b;
			
			PalRam[TC0110PCRAddr[Chip]] = Data;
			
			r = pal5bit(Data >>  0);
			g = pal5bit(Data >>  5);
			b = pal5bit(Data >> 10);
			
			TC0110PCRPalette[TC0110PCRAddr[Chip] | PaletteOffset] = BurnHighCol(r, g, b, 0);
			return;
		}
	}
}

void TC0110PCRStep1WordWrite(int Chip, int Offset, UINT16 Data)
{
	int PaletteOffset = Chip * 0x1000;
	
	switch (Offset) {
		case 0: {
			TC0110PCRAddr[Chip] = Data & 0xfff;
			return;
		}
		
		case 1: {
			UINT16 *PalRam = (UINT16*)TC0110PCRRam[Chip];
			int r, g, b;
			
			PalRam[TC0110PCRAddr[Chip]] = Data;
			
			r = pal5bit(Data >>  0);
			g = pal5bit(Data >>  5);
			b = pal5bit(Data >> 10);
			
			TC0110PCRPalette[TC0110PCRAddr[Chip] | PaletteOffset] = BurnHighCol(r, g, b, 0);
			return;
		}
	}
}

void TC0110PCRStep14rbgWordWrite(int Chip, int Offset, UINT16 Data)
{
	int PaletteOffset = Chip * 0x1000;
	
	switch (Offset) {
		case 0: {
			TC0110PCRAddr[Chip] = Data & 0xfff;
			return;
		}
		
		case 1: {
			UINT16 *PalRam = (UINT16*)TC0110PCRRam[Chip];
			int r, g, b;
			
			PalRam[TC0110PCRAddr[Chip]] = Data;
			
			r = pal4bit(Data >>  0);
			g = pal4bit(Data >>  4);
			b = pal4bit(Data >>  8);
			
			TC0110PCRPalette[TC0110PCRAddr[Chip] | PaletteOffset] = BurnHighCol(r, g, b, 0);
			return;
		}
	}
}

void TC0110PCRStep1RBSwapWordWrite(int Chip, int Offset, UINT16 Data)
{
	int PaletteOffset = Chip * 0x1000;
	
	switch (Offset) {
		case 0: {
			TC0110PCRAddr[Chip] = Data & 0xfff;
			return;
		}
		
		case 1: {
			UINT16 *PalRam = (UINT16*)TC0110PCRRam[Chip];
			int r, g, b;
			
			PalRam[TC0110PCRAddr[Chip]] = Data;
			
			r = pal5bit(Data >> 10);
			g = pal5bit(Data >>  5);
			b = pal5bit(Data >>  0);
			
			TC0110PCRPalette[TC0110PCRAddr[Chip] | PaletteOffset] = BurnHighCol(r, g, b, 0);
			return;
		}
	}
}

void TC0110PCRReset()
{
	TC0110PCRAddr[0] = 0;
}

void TC0110PCRInit(int Num, int nNumColours)
{
	for (int i = 0; i < Num; i++) {
		TC0110PCRRam[i] = (unsigned char*)malloc(0x4000);
		memset(TC0110PCRRam[i], 0, 0x4000);
	}
	
	TC0110PCRPalette = (unsigned int*)malloc(nNumColours * sizeof(unsigned int));
	memset(TC0110PCRPalette, 0, nNumColours);
	
	TC0110PCRTotalColours = nNumColours;
	
	TaitoIC_TC0110PCRInUse = 1;
}

void TC0110PCRExit()
{
	for (int i = 0; i < MAX_TC0110PCR; i++) {
		free(TC0110PCRRam[i]);
		TC0110PCRRam[i] = NULL;
		TC0110PCRAddr[i] = 0;
	}
	
	free(TC0110PCRPalette);
	TC0110PCRPalette = NULL;
	
	TC0110PCRTotalColours = 0;
}

void TC0110PCRScan(int nAction)
{
	struct BurnArea ba;
	
	if (nAction & ACB_MEMORY_RAM) {
		memset(&ba, 0, sizeof(ba));
		ba.Data	  = TC0110PCRRam[0];
		ba.nLen	  = 0x4000;
		ba.szName = "TC0110PCR Ram";
		BurnAcb(&ba);
		
		memset(&ba, 0, sizeof(ba));
		ba.Data	  = TC0110PCRPalette;
		ba.nLen	  = TC0110PCRTotalColours * sizeof(unsigned int);
		ba.szName = "TC0110PCR Palette";
		BurnAcb(&ba);
	}
	
	if (nAction & ACB_DRIVER_DATA) {
		SCAN_VAR(TC0110PCRAddr[0]);
	}
}
