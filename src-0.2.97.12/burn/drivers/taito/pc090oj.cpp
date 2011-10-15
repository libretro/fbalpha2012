// PC090OJ

#include "tiles_generic.h"
#include "taito_ic.h"

unsigned char *PC090OJRam = NULL;
static int PC090OJNumTiles;
static int PC090OJXOffset;
static int PC090OJYOffset;
static int PC090OJUseBuffer;
static int PC090OJPaletteOffset;
int PC090OJSpriteCtrl;

void PC090OJDrawSprites(unsigned char *pSrc)
{
	UINT16 *VideoRam = (UINT16*)PC090OJRam;
	
	int PC090OJCtrl = VideoRam[0xdff];
	
	int Offset;
	int SpriteColBank = (PC090OJSpriteCtrl & 0x0f) << 4;
	
	for (Offset = 0x400 - 4; Offset >= 0; Offset -= 4)
	{
		int xFlip, yFlip;
		int x, y;
		int Data, Code, Colour;

		Data = VideoRam[Offset + 0];
		yFlip = (Data & 0x8000) >> 15;
		xFlip = (Data & 0x4000) >> 14;
		Colour = (Data & 0x000f) | SpriteColBank | PC090OJPaletteOffset;

		Code = VideoRam[Offset + 2] & 0x1fff;
		if (Code >= PC090OJNumTiles) {
			Code %= PC090OJNumTiles; // ok? Mod seems to work fine for bonze.
		}

		x = VideoRam[Offset + 3] & 0x1ff;
		y = VideoRam[Offset + 1] & 0x1ff;

		if (x > 0x140) x -= 0x200;
		if (y > 0x140) y -= 0x200;
		
		if (!(PC090OJCtrl & 1))	{
			x = 320 - x - 16;
			y = 256 - y - 16;
			xFlip = !xFlip;
			yFlip = !yFlip;
		}

		x -= PC090OJXOffset;
		y -= PC090OJYOffset;

		if (x > 16 && x < (nScreenWidth - 16) && y > 16 && y < (nScreenHeight - 16)) {
			if (xFlip) {
				if (yFlip) {
					Render16x16Tile_Mask_FlipXY(pTransDraw, Code, x, y, Colour, 4, 0, 0, pSrc);
				} else {
					Render16x16Tile_Mask_FlipX(pTransDraw, Code, x, y, Colour, 4, 0, 0, pSrc);
				}
			} else {
				if (yFlip) {
					Render16x16Tile_Mask_FlipY(pTransDraw, Code, x, y, Colour, 4, 0, 0, pSrc);
				} else {
					Render16x16Tile_Mask(pTransDraw, Code, x, y, Colour, 4, 0, 0, pSrc);
				}
			}
		} else {
			if (xFlip) {
				if (yFlip) {
					Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, Code, x, y, Colour, 4, 0, 0, pSrc);
				} else {
					Render16x16Tile_Mask_FlipX_Clip(pTransDraw, Code, x, y, Colour, 4, 0, 0, pSrc);
				}
			} else {
				if (yFlip) {
					Render16x16Tile_Mask_FlipY_Clip(pTransDraw, Code, x, y, Colour, 4, 0, 0, pSrc);
				} else {
					Render16x16Tile_Mask_Clip(pTransDraw, Code, x, y, Colour, 4, 0, 0, pSrc);
				}
			}
		}
	}
}

void PC090OJReset()
{
	PC090OJSpriteCtrl = 0;
}

void PC090OJInit(int nNumTiles, int xOffset, int yOffset, int UseBuffer)
{
	PC090OJRam = (unsigned char*)malloc(0x4000);
	memset(PC090OJRam, 0, 0x4000);
	
	PC090OJNumTiles = nNumTiles;
	
	PC090OJXOffset = xOffset;
	PC090OJYOffset = yOffset;
	PC090OJUseBuffer = UseBuffer;
	PC090OJPaletteOffset = 0;
	
	TaitoIC_PC090OJInUse = 1;
}

void PC090OJSetPaletteOffset(int Offset)
{
	PC090OJPaletteOffset = Offset;
}

void PC090OJExit()
{
	free(PC090OJRam);
	PC090OJRam = NULL;
	
	PC090OJNumTiles = 0;
	PC090OJXOffset = 0;
	PC090OJYOffset = 0;
	PC090OJUseBuffer = 0;
	PC090OJPaletteOffset = 0;
	
	PC090OJSpriteCtrl = 0;
}

void PC090OJScan(int nAction)
{
	struct BurnArea ba;
	
	if (nAction & ACB_MEMORY_RAM) {
		memset(&ba, 0, sizeof(ba));
		ba.Data	  = PC090OJRam;
		ba.nLen	  = 0x4000;
		ba.szName = "PC090OJ Ram";
		BurnAcb(&ba);
	}
	
	if (nAction & ACB_DRIVER_DATA) {
		SCAN_VAR(PC090OJSpriteCtrl);
	}
}
