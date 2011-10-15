#include "toaplan.h"

int ToaOpaquePriority;

static unsigned char* pTile;
static unsigned int* pTileData;
static unsigned int* pTilePalette;

int Rallybik = 0;
int Hellfire = 0;

typedef void (*RenderTileFunction)();
static RenderTileFunction* RenderTile;

static int nTileXPos, nTileYPos;

// Include the tile rendering functions
#include "toa_gp9001_func.h"

static int nLastBPP = 0;

struct ToaTile {
	unsigned int nTileAttrib;
	short nTileXPos;
	short nTileYPos;
};

extern unsigned int* ToaPalette2;

// ----------------------------------------------------------------------------
// Rally Bike custom sprite function

static void rallybik_draw_sprites(int priority)
{
	unsigned short *sprite = (unsigned short*)FCU2RAM; 

	for (int offs = 0; offs < (0x1000/2); offs += 4)
	{
		int attrib = sprite[offs + 1];

		if ((attrib & 0x0c00) == priority)
		{
			int sy = (sprite[offs + 3] >> 7) & 0x1ff;

			if (sy != 0x0100)
			{
				int code	= sprite[offs] & 0x7ff;
				int color	= attrib & 0x3f;
				int sx		= (sprite[offs + 2] >> 7) & 0x1ff;
				int flipx	= attrib & 0x100;
				int flipy	= attrib & 0x200;
				if (flipx) sx -= 15;

				sx -= 31;
				sy -= 16;

				if (sy < -15 || sx < -15 || sy >= 240 || sx >= 320) continue;

				{
					int flip = 0;
					if (flipx) flip |= 0x0f;
					if (flipy) flip |= 0xf0;
					unsigned char *gfx = FCU2ROM + (code * 0x100);

					pTilePalette = &ToaPalette2[color << 4];

					for (int y = 0; y < 16; y++, sy++) {
						if (sy < 0 || sy >= 240) continue;

						for (int x = 0; x < 16; x++, sx++) {
							if (sx < 0 || sx >= 320) continue;

							int pxl = gfx[((y * 16) + x) ^ flip];

							if (pxl) {
								PutPix(pBurnDraw + ((sy * 320) + sx) * nBurnBpp, pTilePalette[pxl]);
							}
						}
						sx -= 16;
					}
				}
			}
		}
	}
}


// ----------------------------------------------------------------------------
// FCU-2 functions

unsigned char* FCU2ROM;
unsigned int nFCU2ROMSize;
static unsigned char* FCU2TileAttrib;

static unsigned int nFCU2MaxSprite;

unsigned char* FCU2RAM;
unsigned char* FCU2RAMSize;
unsigned int FCU2Pointer;

static unsigned char* pFCU2SpriteBuffer;

static unsigned short** pFCU2SpriteQueue[16];
static unsigned short** pFCU2SpriteQueueData = NULL;

//static int nFCU2SpriteXOffset, nFCU2SpriteYOffset;

static void FCU2PrepareSprites()
{
	unsigned short* pSpriteInfo;
	int nSprite;

	for (int nPriority = 0; nPriority < 16; nPriority++) {
		pFCU2SpriteQueue[nPriority] = &pFCU2SpriteQueueData[(nPriority << 8) + nPriority];
	}

	for (nSprite = 0, pSpriteInfo = (unsigned short*)pFCU2SpriteBuffer; nSprite < 0x0100; nSprite++, pSpriteInfo += 4) {
		if (!(pSpriteInfo[0] & 0x8000)) {				// Sprite is enabled
			*pFCU2SpriteQueue[pSpriteInfo[1] >> 12]++ = pSpriteInfo;
		}
	}
}

static void FCU2RenderSpriteQueue(int nPriority)
{
	unsigned short* pSpriteInfo;
	unsigned char* pSpriteData;
	int nSpriteXPos, nSpriteYPos;
	int nSpriteXSize, nSpriteYSize;
	unsigned int nSpriteNumber;
	int x, y, s;

	*pFCU2SpriteQueue[nPriority] = NULL;
	pFCU2SpriteQueue[nPriority] = &pFCU2SpriteQueueData[(nPriority << 8) + nPriority];

	while ((pSpriteInfo = *pFCU2SpriteQueue[nPriority]++) != NULL) {

		pTilePalette = &ToaPalette2[(pSpriteInfo[1] & 0x3F) << 4];
		nSpriteNumber = pSpriteInfo[0] & nFCU2MaxSprite;

		pSpriteData = FCU2ROM + (nSpriteNumber << 5);

		s = (pSpriteInfo[1] & 0x0FC0) >> 6;

		nSpriteXSize = ((((unsigned short*)FCU2RAMSize)[s] >> 0) & 0x0F);
		nSpriteYSize = ((((unsigned short*)FCU2RAMSize)[s] >> 4) & 0x0F);

		nSpriteXPos = (pSpriteInfo[2] >> 7) + 0;//nFCU2SpriteXOffset;
		nSpriteXPos &= 0x01FF;
		nSpriteYPos = (pSpriteInfo[3] >> 7) + 0;//nFCU2SpriteYOffset;
		nSpriteYPos &= 0x01FF;

		if (Hellfire) nSpriteYPos -= 16;

		if (nSpriteXPos > 384) {
			nSpriteXPos -= 0x0200;
		}
		if (nSpriteYPos > 384) {
			nSpriteYPos -= 0x0200;
		}
		
//		if (nSpriteNumber) {
//			printf("Sprite\t0x%04X\n", nSpriteNumber);
//			printf("xp: %4i, yp: %4i\n", nSpriteXPos, nSpriteYPos);
//			printf("xs: %4i, ys: %4i\n", nSpriteXSize, nSpriteYSize);
//		}

		for (y = 0, nTileYPos = nSpriteYPos; y < nSpriteYSize; y++, nTileYPos += 8) {
			for (x = 0, nTileXPos = nSpriteXPos; x < nSpriteXSize; x++, nTileXPos += 8, nSpriteNumber++, pSpriteData += 32) {
				if (nSpriteNumber > nFCU2MaxSprite) {
					break;
				}
				if (FCU2TileAttrib[nSpriteNumber]) {
					// Skip tile if it's completely off the screen
					if (!(nTileXPos <= -8 || nTileXPos >= 320 || nTileYPos <= -8 || nTileYPos >= 240)) {
						pTileData = (unsigned int*)pSpriteData;
						pTile = pBurnBitmap + (nTileXPos * nBurnColumn) + (nTileYPos * nBurnRow);
						if (nTileXPos < 0 || nTileXPos > 312 || nTileYPos < 0 || nTileYPos > 232) {
							RenderTile[1]();
						} else {
							RenderTile[0]();
						}
					}
				}
			}
		}
	}
}

// ----------------------------------------------------------------------------
// BCU-2 functions

unsigned char* BCU2ROM;
unsigned int nBCU2ROMSize;
static unsigned char* BCU2TileAttrib;

static unsigned int nBCU2MaxTile;

unsigned char* BCU2RAM;

unsigned short BCU2Reg[8];

unsigned int BCU2Pointer;

static ToaTile* pBCU2TileQueue[16];
static ToaTile* pBCU2TileQueueData = NULL;

int nBCU2TileXOffset = 0, nBCU2TileYOffset = 0;

static void BCU2QueueLayer(unsigned short* pTilemap, int nXPos, int nYPos)
{
	if (Hellfire) nYPos += 16;

	int x, y;
	int nTileRow, nTileColumn;
	unsigned int nTileNumber, nTileAttrib;

	for (y = 0; y < 31; y++) {

		nTileRow = (((nYPos >> 3) + y) << 7) & 0x1F80;

		for (x = 0; x < 41; x++) {
			nTileColumn = (((nXPos >> 3) + x) << 1) & 0x7E;
			nTileNumber = pTilemap[nTileRow + nTileColumn + 1];
			nTileAttrib = pTilemap[nTileRow + nTileColumn];

			if (!(nTileNumber & 0x8000) && (nTileAttrib & 0xF000)) {
				pBCU2TileQueue[nTileAttrib >> 12]->nTileAttrib = (nTileAttrib << 16) | nTileNumber;
				pBCU2TileQueue[nTileAttrib >> 12]->nTileXPos = (x << 3) - (nXPos & 7);
				pBCU2TileQueue[nTileAttrib >> 12]->nTileYPos = (y << 3) - (nYPos & 7);
				pBCU2TileQueue[nTileAttrib >> 12]++;
			}
		}
	}
}

static void BCU2PrepareTiles()
{
	for (int nPriority = 0; nPriority < 16; nPriority++) {
		pBCU2TileQueue[nPriority] = &pBCU2TileQueueData[nPriority * 1280 * 4];
	}

	BCU2QueueLayer((unsigned short*)(BCU2RAM + 0xC000), (BCU2Reg[6] >> 7) - nBCU2TileXOffset + nLayer3XOffset, (BCU2Reg[7] >> 7) - nBCU2TileYOffset + nLayer3YOffset);
	BCU2QueueLayer((unsigned short*)(BCU2RAM + 0x8000), (BCU2Reg[4] >> 7) - nBCU2TileXOffset + nLayer2XOffset, (BCU2Reg[5] >> 7) - nBCU2TileYOffset + nLayer2YOffset);
	BCU2QueueLayer((unsigned short*)(BCU2RAM + 0x4000), (BCU2Reg[2] >> 7) - nBCU2TileXOffset + nLayer1XOffset, (BCU2Reg[3] >> 7) - nBCU2TileYOffset + nLayer1YOffset);
	BCU2QueueLayer((unsigned short*)(BCU2RAM + 0x0000), (BCU2Reg[0] >> 7) - nBCU2TileXOffset + nLayer0XOffset, (BCU2Reg[1] >> 7) - nBCU2TileYOffset + nLayer0YOffset);
}

static void BCU2RenderTileQueue(int nPriority)
{
	unsigned int nTileNumber, nTileAttrib;
	unsigned char* pTileStart;
	unsigned char nOpacity;

	pBCU2TileQueue[nPriority]->nTileAttrib = 0;
	pBCU2TileQueue[nPriority] = &pBCU2TileQueueData[nPriority * 1280 * 4];

	while ((nTileAttrib = pBCU2TileQueue[nPriority]->nTileAttrib) != 0) {
		nTileXPos = pBCU2TileQueue[nPriority]->nTileXPos;
		nTileYPos = pBCU2TileQueue[nPriority]->nTileYPos;
		pBCU2TileQueue[nPriority]++;

		nTileNumber = nTileAttrib & nBCU2MaxTile;

		pTileStart = BCU2ROM + (nTileNumber << 5);
		pTilePalette = &ToaPalette[(nTileAttrib >> 12) & 0x03F0];

		pTile = pBurnBitmap + (nTileXPos * nBurnColumn) + (nTileYPos * nBurnRow);
		
		if ((nOpacity = BCU2TileAttrib[nTileNumber]) != 0 || nPriority < ToaOpaquePriority) {
			if (nPriority < ToaOpaquePriority) nOpacity = 9;

			pTileData = (unsigned int*)pTileStart;
			if (nTileXPos >= 0 && nTileXPos < 312 && nTileYPos >= 0 && nTileYPos < 232) {
				RenderTile[nOpacity - 1]();
			} else {
				if (nTileXPos > -8 && nTileXPos < 320 && nTileYPos > -8 && nTileYPos < 240) {
					RenderTile[nOpacity]();
				}
			}
		}
	}
}

void ToaBufferFCU2Sprites()
{
	memcpy(pFCU2SpriteBuffer, FCU2RAM, 0x0800);
}

int ToaRenderBCU2()
{
	if (nLastBPP != nBurnBpp ) {
		nLastBPP = nBurnBpp;

#ifdef DRIVER_ROTATION
		if (bRotatedScreen) {
			RenderTile = RenderTile_ROT270[nBurnBpp - 2];
		} else {
			RenderTile = RenderTile_ROT0[nBurnBpp - 2];
		}
#else
		RenderTile = RenderTile_ROT0[nBurnBpp - 2];
#endif
	}

	BCU2PrepareTiles();
	if (!Rallybik) {
		FCU2PrepareSprites();
	}

	for (int nPriority = 0; nPriority < 16; nPriority++) {
		if (Rallybik) {
			rallybik_draw_sprites(nPriority << 8);
		} else {
			FCU2RenderSpriteQueue(nPriority);
		}
		BCU2RenderTileQueue(nPriority);
	}

	return 0;
}

int ToaInitBCU2()
{
	int nSize;

	nLastBPP = 0;

	nBCU2MaxTile = (nBCU2ROMSize - 1) >> 5;

	nSize = 1280 * 4 * 0x10 * sizeof(ToaTile);
	pBCU2TileQueueData = (ToaTile*)malloc(nSize);
	memset(pBCU2TileQueueData, 0, nSize);

	BCU2TileAttrib = (unsigned char*)malloc(32768);
	memset(BCU2TileAttrib, 0, 32768);
	for (unsigned int j = 0; j < (nBCU2ROMSize >> 5); j++) {
		bool bTransparent = true, bSolid = true;
		int nTwoPixels;
		for (unsigned int k = (j << 5); k < ((j << 5) + 32); k++) {
			if ((nTwoPixels = BCU2ROM[k]) != 0) {
				bTransparent = false;
			}
			if ((nTwoPixels & 0xF0) == 0 || (nTwoPixels & 0x0F) == 0) {
				bSolid = false;
			}
		}
		if (bTransparent) {
			BCU2TileAttrib[j] = 0;
		} else {
			if (bSolid) {
				BCU2TileAttrib[j] = 9;
			} else {
				BCU2TileAttrib[j] = 1;
			}
		}
	}

	nFCU2MaxSprite = (nFCU2ROMSize - 1) >> 5;

	nSize = 0x10 * 0x101 * sizeof(int);
	pFCU2SpriteQueueData = (unsigned short**)malloc(nSize);
	memset(pFCU2SpriteQueueData, 0, nSize);

	pFCU2SpriteBuffer = (unsigned char*)malloc(0x0800);

	FCU2TileAttrib = (unsigned char*)malloc(32768);
	memset(FCU2TileAttrib, 0, 32768);
	for (unsigned int j = 0; j < (nFCU2ROMSize >> 5); j++) {
		bool bTransparent = true, bSolid = true;
		int nTwoPixels;
		for (unsigned int k = (j << 5); k < ((j << 5) + 32); k++) {
			if ((nTwoPixels = FCU2ROM[k]) != 0) {
				bTransparent = false;
			}
			if ((nTwoPixels & 0xF0) == 0 || (nTwoPixels & 0x0F) == 0) {
				bSolid = false;
			}
		}
		if (bTransparent) {
			FCU2TileAttrib[j] = 0;
		} else {
			if (bSolid) {
				FCU2TileAttrib[j] = 9;
			} else {
				FCU2TileAttrib[j] = 1;
			}
		}
	}

	if (!nLayer0XOffset) {
		nLayer0XOffset = 0x01EF + 6;
	}
	if (!nLayer1XOffset) {
		nLayer1XOffset = 0x01EF + 4;
	}
	if (!nLayer2XOffset) {
		nLayer2XOffset = 0x01EF + 2;
	}
	if (!nLayer3XOffset) {
		nLayer3XOffset = 0x01EF + 0;
	}

	if (!nLayer0YOffset) {
		nLayer0YOffset = 0x0101;
	}
	if (!nLayer1YOffset) {
		nLayer1YOffset = 0x0101;
	}
	if (!nLayer2YOffset) {
		nLayer2YOffset = 0x0101;
	}
	if (!nLayer3YOffset) {
		nLayer3YOffset = 0x0101;
	}

	ToaOpaquePriority = 0;

	return 0;
}

int ToaExitBCU2()
{
	nLayer0XOffset = 0;
	nLayer1XOffset = 0;
	nLayer2XOffset = 0;
	nLayer3XOffset = 0;
	nLayer0YOffset = 0;
	nLayer1YOffset = 0;
	nLayer2YOffset = 0;
	nLayer3YOffset = 0;

	free(pBCU2TileQueueData);
	pBCU2TileQueueData = NULL;

	free(BCU2TileAttrib);
	BCU2TileAttrib = NULL;

	free(pFCU2SpriteQueueData);
	pFCU2SpriteQueueData = NULL;

	free(pFCU2SpriteBuffer);
	pFCU2SpriteBuffer = NULL;

	free(FCU2TileAttrib);
	FCU2TileAttrib = NULL;

	return 0;
}

