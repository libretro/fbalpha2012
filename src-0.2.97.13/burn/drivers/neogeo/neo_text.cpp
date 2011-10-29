#include "neogeo.h"

unsigned char* NeoTextROMBIOS;
unsigned char* NeoTextROM[MAX_SLOT];

int nNeoTextROMSize[MAX_SLOT] = { 0, };
bool bBIOSTextROMEnabled;

static unsigned char* NeoTextROMCurrent;

static char* NeoTextTileAttrib[MAX_SLOT] = { NULL, };
static char* NeoTextTileAttribBIOS = NULL;
static char* NeoTextTileAttribActive = NULL;
static int nBankswitch[MAX_SLOT] = { 0, };

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

	if (!bBIOSTextROMEnabled && nBankswitch[nNeoActiveSlot]) {

		if (!NeoTextROMCurrent) {
			return 0;
		}

		if (nBankswitch[nNeoActiveSlot] == 1) {

			// Garou, Metal Slug 3, Metal Slug 4

			int nOffset[32];
			int nBank = (3 << 12);
			int z = 0;

			y = 0;
			while (y < 32) {
				if (*((unsigned short*)(NeoGraphicsRAM + 0xEA00 + z)) == 0x0200 && (*((unsigned short*)(NeoGraphicsRAM + 0xEB00 + z)) & 0xFF00) == 0xFF00) {
					nBank = ((*((unsigned short*)(NeoGraphicsRAM + 0xEB00 + z)) & 3) ^ 3) << 12;
					nOffset[y++] = nBank;
				}
				nOffset[y++] = nBank;
				z += 4;
			}

			for (y = 2, pTileRow += 2; y < 30; y++, pCurrentRow += nTileDown, pTileRow++) {
				pTextROM    = NeoTextROMCurrent        + (nOffset[y - 2] << 5);
				pTileAttrib = NeoTextTileAttribActive +  nOffset[y - 2];
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

			// KOF2000

			unsigned short* pBankInfo = (unsigned short*)(NeoGraphicsRAM + 0xEA00) + 1;
			pTextROM    = NeoTextROMCurrent;
			pTileAttrib = NeoTextTileAttribActive;

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
			pTextROM    = NeoTextROMBIOS;
			pTileAttrib = NeoTextTileAttribBIOS;
		} else {
			pTextROM    = NeoTextROMCurrent;
			pTileAttrib = NeoTextTileAttribActive;
		}
		if (!pTextROM) {
			return 0;
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

void NeoExitText(int nSlot)
{
	if (NeoTextTileAttribBIOS) {
		free(NeoTextTileAttribBIOS);
		NeoTextTileAttribBIOS    = NULL;
	}

	if (NeoTextTileAttrib[nSlot]) {
		free(NeoTextTileAttrib[nSlot]);
		NeoTextTileAttrib[nSlot] = NULL;
	}
	NeoTextTileAttribActive = NULL;
}

static void NeoUpdateTextAttribBIOS(int nOffset, int nSize)
{
	for (int i = nOffset & ~31; i < nOffset + nSize; i += 32) {
		NeoTextTileAttribBIOS[i >> 5] = (((long long*)NeoTextROMBIOS)[(i >> 3) + 0] ||
										 ((long long*)NeoTextROMBIOS)[(i >> 3) + 1] ||
										 ((long long*)NeoTextROMBIOS)[(i >> 3) + 2] ||
										 ((long long*)NeoTextROMBIOS)[(i >> 3) + 3])
									  ? 0 : 1;
	}
}

static inline void NeoUpdateTextAttribOne(const int nOffset)
{
	NeoTextTileAttribActive[nOffset >> 5] = 1;

	for (int i = nOffset; i < nOffset + 32; i += 4) {
		if (*((unsigned int*)(NeoTextROMCurrent + i))) {
			NeoTextTileAttribActive[nOffset >> 5] = 0;
			break;
		}
	}
}

static void NeoUpdateTextAttrib(int nOffset, int nSize)
{
	nOffset &= ~0x1F;

	for (int i = nOffset; i < nOffset + nSize; i += 32) {
		NeoUpdateTextAttribOne(i);
	}
}

void NeoUpdateTextOne(int nOffset, const unsigned char byteValue)
{
	nOffset = (nOffset & ~0x1F) | (((nOffset ^ 0x10) & 0x18) >> 3) | ((nOffset & 0x07) << 2);

	if (byteValue) {
		NeoTextTileAttribActive[nOffset >> 5] = 0;
	} else {
		if (NeoTextTileAttribActive[nOffset >> 5] == 0 && NeoTextROMCurrent[nOffset]) {
			NeoTextTileAttribActive[nOffset >> 5] = 1;
			NeoUpdateTextAttribOne(nOffset);
		}
	}

	NeoTextROMCurrent[nOffset] = byteValue;
}

static inline void NeoTextDecodeTile(const unsigned char* pData, unsigned char* pDest)
{
	unsigned char nBuffer[32];

	for (int i = 0; i < 8; i++) {
		nBuffer[0 + i * 4] = pData[16 + i];
		nBuffer[1 + i * 4] = pData[24 + i];
		nBuffer[2 + i * 4] = pData[ 0 + i];
		nBuffer[3 + i * 4] = pData[ 8 + i];
	}

	for (int i = 0; i < 32; i++) {
		pDest[i]  = nBuffer[i] << 4;
		pDest[i] |= nBuffer[i] >> 4;
	}
}

void NeoDecodeTextBIOS(int nOffset, const int nSize, unsigned char* pData)
{
	unsigned char* pEnd = pData + nSize;

	for (unsigned char* pDest = NeoTextROMBIOS + (nOffset & ~0x1F); pData < pEnd; pData += 32, pDest += 32) {
		NeoTextDecodeTile(pData, pDest);
	}

//	if (NeoTextTileAttribBIOS) {
//		NeoUpdateTextAttribBIOS(0, nSize);
//	}	
}

void NeoDecodeText(int nOffset, const int nSize, unsigned char* pData, unsigned char* pDest)
{
	unsigned char* pEnd = pData + nSize;

	for (pData += (nOffset & ~0x1F); pData < pEnd; pData += 32, pDest += 32) {
		NeoTextDecodeTile(pData, pDest);
	}
}

void NeoUpdateText(int nOffset, const int nSize, unsigned char* pData, unsigned char* pDest)
{
	NeoDecodeText(nOffset, nSize, pData, pDest);
	if (NeoTextTileAttribActive) {
		NeoUpdateTextAttrib((nOffset & ~0x1F), nSize);
	}	
}

void NeoSetTextSlot(int nSlot)
{
	NeoTextROMCurrent       = NeoTextROM[nSlot];
	NeoTextTileAttribActive = NeoTextTileAttrib[nSlot];
}

int NeoInitText(int nSlot)
{
	if (nSlot < 0) {
		NeoTextTileAttribBIOS    = (char*)malloc(0x1000);
		for (int i = 0; i < 0x1000; i++) {
			NeoTextTileAttribBIOS[i] = 1;
		}
		NeoUpdateTextAttribBIOS(0, 0x020000);

		return 0;
	}
		
	int nTileNum = nNeoTextROMSize[nSlot] >> 5;

//	NeoExitText(nSlot);

	NeoTextTileAttrib[nSlot] = (char*)malloc((nTileNum < 0x1000) ? 0x1000 : nTileNum);

	if (nNeoScreenWidth == 304) {
		nMinX = 1;
		nMaxX = 39;
	} else {
		nMinX = 0;
		nMaxX = 40;
	}

	// Set up tile attributes

	NeoTextROMCurrent       = NeoTextROM[nSlot];
	NeoTextTileAttribActive = NeoTextTileAttrib[nSlot];
	for (int i = 0; i < ((nTileNum < 0x1000) ? 0x1000 : nTileNum); i++) {
		NeoTextTileAttribActive[i] = 1;
	}
	NeoUpdateTextAttrib(0, nNeoTextROMSize[nSlot]);

	// Set up tile bankswitching

	nBankswitch[nSlot] = 0;
	if (nNeoTextROMSize[nSlot] > 0x040000) {
//		if (BurnDrvGetHardwareCode() & HARDWARE_SNK_CMC50) {
		if (BurnDrvGetHardwareCode() & HARDWARE_SNK_ALTERNATE_TEXT) {
			nBankswitch[nSlot] = 2;

			// Precompute lookup-tables
			for (int x = nMinX; x < nMaxX; x++) {
				nBankLookupAddress[x] = (x / 6) << 5;
				nBankLookupShift[x] = (5 - (x % 6)) << 1;
			}

		} else {
			nBankswitch[nSlot] = 1;
		}
	}

	return 0;
}
