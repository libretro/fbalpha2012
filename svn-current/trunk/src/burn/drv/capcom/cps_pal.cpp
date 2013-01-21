#include "cps.h"
#include "bitswap.h"

// CPS (palette)

UINT32* CpsPal = NULL;					// Hicolor version of palette
INT32 nCpsPalCtrlReg;
INT32 bCpsUpdatePalEveryFrame = 0;		// Some of the hacks need this as they don't write to CpsReg 0x0a

INT32 CpsPalInit()
{
	INT32 nLen = 0;

	nLen = 0xc00 * sizeof(UINT32);
	CpsPal = (UINT32*)BurnMalloc(nLen);
	if (CpsPal == NULL) {
		return 1;
	}

	return 0;
}

INT32 CpsPalExit()
{
	BurnFree(CpsPal);
	return 0;
}

// Update CpsPal with the new palette at pNewPal (length 0xc00 bytes)
INT32 CpsPalUpdate(UINT8* pNewPal)
{
	INT32 nCtrl = CpsReg[nCpsPalCtrlReg];
	UINT16 *PaletteRAM = (UINT16*)pNewPal;
	
	for (INT32 nPage = 0; nPage < 6; nPage++) {
		if (BIT(nCtrl, nPage)) {
			for (INT32 Offset = 0; Offset < 0x200; ++Offset) {
				INT32 Palette = BURN_ENDIAN_SWAP_INT16(*(PaletteRAM++));
				INT32 r, g, b, Bright;
				
				Bright = 0x0f + ((Palette >> 12) << 1);

				r = ((Palette >> 8) & 0x0f) * 0x11 * Bright / 0x2d;
				g = ((Palette >> 4) & 0x0f) * 0x11 * Bright / 0x2d;
				b = ((Palette >> 0) & 0x0f) * 0x11 * Bright / 0x2d;
				
				CpsPal[(0x200 * nPage) + (Offset ^ 15)] = BurnHighCol(r, g, b, 0);
			}
		}
	}

	return 0;
}
