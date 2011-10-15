#include "neogeo.h"

int nNeoScreenWidth;

unsigned char* NeoSpriteROM;
unsigned char* NeoZoomROM;

static unsigned char* NeoTileAttrib = NULL;

unsigned int nNeoTileMask;
int nNeoMaxTile;

int nSliceStart, nSliceEnd, nSliceSize;

static unsigned int* pTileData;
static unsigned int* pTilePalette;

static unsigned short* pBank;

static int nBankSize;
static int nBankXPos, nBankYPos;
static int nBankXZoom, nBankYZoom;

static int nNeoSpriteFrame04, nNeoSpriteFrame08;

static int nLastBPP = -1;

typedef void (*RenderBankFunction)();
static RenderBankFunction* RenderBank;

// Include the tile rendering functions
#include "neo_sprite_func.h"

int NeoRenderSprites()
{
	if (nLastBPP != nBurnBpp ) {
		nLastBPP = nBurnBpp;

		RenderBank = RenderBankNormal[nBurnBpp - 2];
	}

	if (!(nBurnLayer & 1)) {
		return 0;
	}

	unsigned short BankAttrib01, BankAttrib02, BankAttrib03;

	nNeoSpriteFrame04 = nNeoSpriteFrame & 3;
	nNeoSpriteFrame08 = nNeoSpriteFrame & 7;

	for (int nBank = 1; nBank < 0x17D; nBank++) {

		BankAttrib01 = *((unsigned short*)(NeoGraphicsRAM + 0x010000 + (nBank << 1)));
		BankAttrib02 = *((unsigned short*)(NeoGraphicsRAM + 0x010400 + (nBank << 1)));
		BankAttrib03 = *((unsigned short*)(NeoGraphicsRAM + 0x010800 + (nBank << 1)));

		pBank = (unsigned short*)(NeoGraphicsRAM + (nBank << 7));

		if (BankAttrib02 & 0x40) {
			nBankXPos += nBankXZoom + 1;
		} else {
			nBankYPos = (0x0200 - (BankAttrib02 >> 7)) & 0x01FF;
			nBankXPos = (BankAttrib03 >> 7);
			if (nNeoScreenWidth == 304) {
				nBankXPos -= 8;
			}

			nBankYZoom = BankAttrib01 & 0xFF;

			nBankSize = (BankAttrib02 & 0x3F);
			if (nBankSize > 0x20) {
				nBankSize = 0x20;
			}

//			if (nBankSize /* > 0x10 */ && nSliceStart == 0x10) bprintf(PRINT_NORMAL, _T("bank: %04X, x: %04X, y: %04X, zoom: %02X, size: %02X.\n"), nBank, nBankXPos, nBankYPos, nBankYZoom, nBankSize);
		}

		if (nBankSize) {

			nBankXZoom = (BankAttrib01 >> 8) & 0x0F;
			if (nBankXPos >= 0x01E0) {
				nBankXPos -= 0x200;
			}

			if (nBankXPos >= 0 && nBankXPos < (nNeoScreenWidth - nBankXZoom - 1)) {
				RenderBank[nBankXZoom]();
			} else {
				if (nBankXPos >= -nBankXZoom && nBankXPos < nNeoScreenWidth) {
					RenderBank[nBankXZoom + 16]();
				}
			}
		}
	}

//	bprintf(PRINT_NORMAL, _T("\n"));

	return 0;
}

int NeoInitSprites()
{
	// Create a table that indicates if a tile is transparent
	NeoTileAttrib = (unsigned char*)malloc(nNeoTileMask + 1);
	for (int i = 0; i < nNeoMaxTile; i++) {
		bool bTransparent = true;
		for (int j = i << 7; j < (i + 1) << 7; j++) {
			if (NeoSpriteROM[j]) {
				bTransparent = false;
				break;
			}
		}
		if (bTransparent) {
			NeoTileAttrib[i] = 1;
		} else {
			NeoTileAttrib[i] = 0;
		}
	}

	for (unsigned int i = nNeoMaxTile; i < nNeoTileMask + 1; i++) {
		NeoTileAttrib[i] = 1;
	}

	return 0;
}

void NeoExitSprites()
{
	free(NeoTileAttrib);
}

