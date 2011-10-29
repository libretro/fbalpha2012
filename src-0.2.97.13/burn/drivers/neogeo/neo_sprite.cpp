#include "neogeo.h"

unsigned char* NeoZoomROM;

unsigned char* NeoSpriteROM[MAX_SLOT] = { NULL, };

unsigned int nNeoTileMask[MAX_SLOT];
int nNeoMaxTile[MAX_SLOT];

static unsigned char* NeoSpriteROMActive;
static unsigned int nNeoTileMaskActive;
static int nNeoMaxTileActive;

static unsigned char* NeoTileAttrib[MAX_SLOT] = { NULL, };
static unsigned char* NeoTileAttribActive;

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

static 	unsigned short BankAttrib01, BankAttrib02, BankAttrib03;


// Include the tile rendering functions
#include "neo_sprite_func.h"

int NeoRenderSprites()
{
	if (nLastBPP != nBurnBpp ) {
		nLastBPP = nBurnBpp;

		RenderBank = RenderBankNormal[nBurnBpp - 2];
	}

	if (!NeoSpriteROMActive || !(nBurnLayer & 1)) {
		return 0;
	}

//	unsigned short BankAttrib01, BankAttrib02, BankAttrib03;

	nNeoSpriteFrame04 = nNeoSpriteFrame & 3;
	nNeoSpriteFrame08 = nNeoSpriteFrame & 7;
	
	// ssrpg hack! - NeoCD/SDL
	int nStart = 0;
	if (SekReadWord(0x108) == 0x0085) {
		unsigned short *vidram = (unsigned short*)NeoGraphicsRAM;

	   	if ((vidram[0x8202] & 0x40) == 0 && (vidram[0x8203] & 0x40) != 0) {
			nStart = 3;

			while ((vidram[0x8200 + nStart] & 0x40) != 0) nStart++;

			if (nStart == 3) nStart = 0;
		}
	}

	for (int nBank = 0; nBank < 0x17D; nBank++) {
		int zBank = (nBank + nStart) % 0x17d;
		BankAttrib01 = *((unsigned short*)(NeoGraphicsRAM + 0x010000 + (zBank << 1)));
		BankAttrib02 = *((unsigned short*)(NeoGraphicsRAM + 0x010400 + (zBank << 1)));
		BankAttrib03 = *((unsigned short*)(NeoGraphicsRAM + 0x010800 + (zBank << 1)));

		pBank = (unsigned short*)(NeoGraphicsRAM + (zBank << 7));

		if (BankAttrib02 & 0x40) {
			nBankXPos += nBankXZoom + 1;
		} else {
			nBankYPos = (0x0200 - (BankAttrib02 >> 7)) & 0x01FF;
			nBankXPos = (BankAttrib03 >> 7);
			if (nNeoScreenWidth == 304) {
				nBankXPos -= 8;
			}

			nBankYZoom = BankAttrib01 & 0xFF;
			nBankSize  = BankAttrib02 & 0x3F;

//			if (nBankSize > 0x10 && nSliceStart == 0x10) bprintf(PRINT_NORMAL, _T("bank: %04X, x: %04X, y: %04X, zoom: %02X, size: %02X.\n"), zBank, nBankXPos, nBankYPos, nBankYZoom, nBankSize);
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

void NeoUpdateSprites(int nOffset, int nSize)
{
	for (int i = nOffset & ~127; i < nOffset + nSize; i += 128) {
		bool bTransparent = true;
		for (int j = i; j < i + 128; j++) {
			if (NeoSpriteROMActive[j]) {
				bTransparent = false;
				break;
			}
		}
		if (bTransparent) {
			NeoTileAttribActive[i >> 7] = 1;
		} else {
			NeoTileAttribActive[i >> 7] = 0;
		}
	}
}

void NeoSetSpriteSlot(int nSlot)
{
	NeoTileAttribActive = NeoTileAttrib[nSlot];
	NeoSpriteROMActive  = NeoSpriteROM[nSlot];
	nNeoTileMaskActive  = nNeoTileMask[nSlot];
	nNeoMaxTileActive   = nNeoMaxTile[nSlot];
}

int NeoInitSprites(int nSlot)
{
	// Create a table that indicates if a tile is transparent
	NeoTileAttrib[nSlot] = (unsigned char*)malloc(nNeoTileMask[nSlot] + 1);

	for (int i = 0; i < nNeoMaxTile[nSlot]; i++) {
		bool bTransparent = true;
		for (int j = i << 7; j < (i + 1) << 7; j++) {
			if (NeoSpriteROM[nSlot][j]) {
				bTransparent = false;
				break;
			}
		}
		if (bTransparent) {
			NeoTileAttrib[nSlot][i] = 1;
		} else {
			NeoTileAttrib[nSlot][i] = 0;
		}
	}
	for (unsigned int i = nNeoMaxTile[nSlot]; i < nNeoTileMask[nSlot] + 1; i++) {
		NeoTileAttrib[nSlot][i] = 1;
	}

	NeoTileAttribActive = NeoTileAttrib[nSlot];
	NeoSpriteROMActive  = NeoSpriteROM[nSlot];
	nNeoTileMaskActive  = nNeoTileMask[nSlot];
	nNeoMaxTileActive   = nNeoMaxTile[nSlot];

	return 0;
}

void NeoExitSprites(int nSlot)
{
	if (NeoTileAttrib[nSlot]) {
		free(NeoTileAttrib[nSlot]);
		NeoTileAttrib[nSlot] = NULL;
	}
	NeoTileAttribActive = NULL;
}
