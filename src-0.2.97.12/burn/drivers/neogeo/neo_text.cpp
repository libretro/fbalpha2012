#include "neogeo.h"

unsigned char* NeoTextROM;
int nNeoTextROMSize = -1;
bool bBIOSTextROMEnabled;

static char* NeoTextTileAttrib = NULL;
static int nBankswitch;

static int nBankLookupAddress[40];
static int nBankLookupShift[40];

static unsigned char* pTile;
static unsigned char* pTileData;
static unsigned int* pTilePalette;

typedef void (*RenderTileFunction)();
static RenderTileFunction RenderTile;

static int nLastBPP = 0;

static int nMinX, nMaxX;

#define BPP 16
 #include "neo_text_render.h"
#undef BPP

#define BPP 24
 #include "neo_text_render.h"
#undef BPP

#define BPP 32
 #include "neo_text_render.h"
#undef BPP

int NeoRenderText()
{
	int x, y;
	unsigned char* pTextROM;
	char* pTileAttrib;
	unsigned char* pCurrentRow = pBurnDraw;
	unsigned int* pTextPalette = NeoPalette;
	unsigned int nTileDown = nBurnPitch << 3;
	unsigned int nTileLeft = nBurnBpp << 3;
	unsigned short* pTileRow = (unsigned short*)(NeoGraphicsRAM + 0xE000);

	if (!(nBurnLayer & 2)) {
		return 0;
	}

	if (nLastBPP != nBurnBpp ) {
		nLastBPP = nBurnBpp;

		switch (nBurnBpp) {
			case 2:
				RenderTile = *RenderTile16;
				break;
			case 3:
				RenderTile = *RenderTile24;
				break;
			case 4:
				RenderTile = *RenderTile32;
				break;
			default:
				return 1;
		}
	}

	if (!bBIOSTextROMEnabled && nBankswitch) {
		if (nBankswitch == 1) {

			// GAROU, MSLUG3, MSLUG4, SAMSHO5, and SAMSH5SP

			int nOffset[32];
			int nBank = 0x001000 + (3 << 12);
			int z = 0;

			y = 0;
			while (y < 32) {
				if (*((unsigned short*)(NeoGraphicsRAM + 0xEA00 + z)) == 0x0200 && (*((unsigned short*)(NeoGraphicsRAM + 0xEB00 + z)) & 0xFF00) == 0xFF00) {
					nBank = ((*((unsigned short*)(NeoGraphicsRAM + 0xEB00 + z)) & 3) ^ 3) << 12;
					nBank += 0x001000;
					nOffset[y++] = nBank;
				}
				nOffset[y++] = nBank;
				z += 4;
			}

			for (y = 2, pTileRow += 2; y < 30; y++, pCurrentRow += nTileDown, pTileRow++) {
				pTextROM = NeoTextROM + (nOffset[y - 2] << 5);
				pTileAttrib = NeoTextTileAttrib + nOffset[y - 2];
				for (x = nMinX, pTile = pCurrentRow; x < nMaxX; x++, pTile += nTileLeft) {
					unsigned int nTile = pTileRow[x << 5];
					int nPalette = nTile & 0xF000;
					nTile &= 0x0FFF;
					if (pTileAttrib[nTile] == 0) {
						pTileData = pTextROM + (nTile << 5);
						pTilePalette = &pTextPalette[nPalette >> 8];
						RenderTile();
					}
				}
			}
		} else {

			// KOF2000, MATRIM, SVC, and KOF2003

			unsigned short* pBankInfo = (unsigned short*)(NeoGraphicsRAM + 0xEA00) + 1;
			pTextROM = NeoTextROM + 0x020000;
			pTileAttrib = NeoTextTileAttrib + 0x01000;

			for (y = 2, pTileRow += 2; y < 30; y++, pCurrentRow += nTileDown, pTileRow++, pBankInfo++) {
 				for (x = nMinX, pTile = pCurrentRow; x < nMaxX; x++, pTile += nTileLeft) {
					unsigned int nTile = pTileRow[x << 5];
					int nPalette = nTile & 0xF000;
					nTile &= 0x0FFF;
					nTile += (((pBankInfo[nBankLookupAddress[x]] >> nBankLookupShift[x]) & 3) ^ 3) << 12;
					if (pTileAttrib[nTile] == 0) {
						pTileData = pTextROM + (nTile << 5);
						pTilePalette = &pTextPalette[nPalette >> 8];
						RenderTile();
					}
				}
			}
		}
	} else {

		if (bBIOSTextROMEnabled) {
			pTextROM = NeoTextROM;
			pTileAttrib = NeoTextTileAttrib;
		} else {
			pTextROM = NeoTextROM + 0x020000;
			pTileAttrib = NeoTextTileAttrib + 0x1000;
		}

		for (y = 2, pTileRow += 2; y < 30; y++, pCurrentRow += nTileDown, pTileRow++) {
			for (x = nMinX, pTile = pCurrentRow; x < nMaxX; x++, pTile += nTileLeft) {
				unsigned int nTile = pTileRow[x << 5];
				int nPalette = nTile & 0xF000;
				nTile &= 0xFFF;
				if (pTileAttrib[nTile] == 0) {
					pTileData = pTextROM + (nTile << 5);
					pTilePalette = &pTextPalette[nPalette >> 8];
					RenderTile();
				}
			}
		}
	}

	return 0;
}

// kof10th
static inline void NeoUpdateTextAttribOne(const int nOffset)
{
	for (int i = nOffset; i < nOffset + 32; i += 4) {
		if (*((unsigned int*)(NeoTextROM + i))) {
			NeoTextTileAttrib[nOffset >> 5] = 0;
			break;
		}
	}
}

void NeoUpdateTextOne(int nOffset, const unsigned char byteValue)
{
	nOffset = (nOffset & ~0x1F) | (((nOffset ^ 0x10) & 0x18) >> 3) | ((nOffset & 0x07) << 2);
	if (byteValue) {
		NeoTextTileAttrib[nOffset >> 5] = 0;
	} else {
		if (NeoTextTileAttrib[nOffset >> 5] == 0 && NeoTextROM[nOffset]) {
			NeoTextTileAttrib[nOffset >> 5] = 1;
			NeoUpdateTextAttribOne(nOffset);
		}
	}
	NeoTextROM[nOffset] = byteValue;
}

void NeoExitText()
{
	free(NeoTextTileAttrib);
	NeoTextTileAttrib = NULL;
}

int NeoInitText()
{
	int nTileNum = (0x020000 + nNeoTextROMSize) >> 5;

	free(NeoTextTileAttrib);
	NeoTextTileAttrib = (char*)malloc((nTileNum < 0x2000) ? 0x2000 : nTileNum);

	if (nNeoScreenWidth == 304) {
		nMinX = 1;
		nMaxX = 39;
	} else {
		nMinX = 0;
		nMaxX = 40;
	}

	for (int i = 0; i < nTileNum; i++) {
		pTile = NeoTextROM + (i << 5);
		bool bTransparent = true;
		for (int j = 0; j < 32; j++) {
			if (pTile[j]) {
				bTransparent = false;
				break;
			}
		}

		if (bTransparent) {
			NeoTextTileAttrib[i] = 1;
		} else {
			NeoTextTileAttrib[i] = 0;
		}
	}
	for (int i = nTileNum; i < 0x2000; i++) {
		NeoTextTileAttrib[i] = 1;
	}

	nBankswitch = 0;
	if (nNeoTextROMSize > 0x040000) {
		if (BurnDrvGetHardwareCode() & HARDWARE_SNK_ALTERNATE_TEXT) {
			nBankswitch = 2;

			// Precompute lookup-tables
			for (int x = nMinX; x < nMaxX; x++) {
				nBankLookupAddress[x] = (x / 6) << 5;
				nBankLookupShift[x] = (5 - (x % 6)) << 1;
			}

		} else {
			nBankswitch = 1;
		}
	}

	return 0;
}
