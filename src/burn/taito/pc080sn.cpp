// PC080SN

#include "tiles_generic.h"
#include "taito_ic.h"

unsigned char *PC080SNRam = NULL;
static UINT16 PC080SNCtrl[8];
static int BgScrollX;
static int BgScrollY;
static int FgScrollX;
static int FgScrollY;
static int PC080SNNumTiles;
static int PC080SNXOffset;
static int PC080SNYOffset;
static int PC080SNFgTransparentPen;
static int PC080SNYInvert;
static int PC080SNDblWidth;
static int PC080SNCols;

void PC080SNDrawBgLayer(int Opaque, unsigned char *pSrc)
{
	int mx, my, Offset,  Colour, x, y, TileIndex = 0, Flip, xFlip, yFlip;
	UINT16 Attr, Code;

	UINT16 *VideoRam = (UINT16*)PC080SNRam + 0x0000;
	UINT16 *BgScrollRam = NULL;

	if (!PC080SNDblWidth) BgScrollRam = (UINT16*)PC080SNRam + 0x2000;
	
	int BgScrollActive = 0;
	if (BgScrollRam) {
		for (int i = 0; i < 512; i++) {
			if (swapWord(BgScrollRam[i])) {
				BgScrollActive = 1;
				break;
			}
		}
	}
	
	for (my = 0; my < 64; my++) {
		for (mx = 0; mx < PC080SNCols; mx++) {
			Offset = 2 * TileIndex;
			if (!PC080SNDblWidth) {
				Attr = swapWord(VideoRam[Offset + 0]);
				Code = swapWord(VideoRam[Offset + 1]) & (PC080SNNumTiles - 1);
			} else {
				Attr = swapWord(VideoRam[TileIndex + 0x0000]);
				Code = swapWord(VideoRam[TileIndex + 0x2000] & 0x3fff);
			}
			Colour = Attr & 0x1ff;
			Flip = (Attr & 0xc000) >> 14;
			xFlip = (Flip >> 0) & 0x01;
			yFlip = (Flip >> 1) & 0x01;
			
			x = 8 * mx;
			y = 8 * my;
			
			x -= 16;
			
			x -= PC080SNXOffset;
			y -= PC080SNYOffset;
			
			if (BgScrollActive) {
				int px, py;
			
				UINT32 nPalette = Colour << 4;
			
				for (py = 0; py < 8; py++) {
					for (px = 0; px < 8; px++) {
						unsigned char c = pSrc[(Code * 64) + (py * 8) + px];
						if (xFlip) c = pSrc[(Code * 64) + (py * 8) + (7 - px)];
						if (yFlip) c = pSrc[(Code * 64) + ((7 - py) * 8) + px];
						if (xFlip && yFlip) c = pSrc[(Code * 64) + ((7 - py) * 8) + (7 - px)];
					
						if (c || Opaque) {
							int xPos = x + px;
							int yPos = y + py;
							yPos -= BgScrollY & 0x1ff;
					
							if (yPos < -8) yPos += 512;
							if (yPos >= 512) yPos -= 512;			
					
							if (yPos >= 0 && yPos < nScreenHeight) {
								if (!PC080SNDblWidth) {					
									xPos -= ((BgScrollX - swapWord(BgScrollRam[yPos + PC080SNYOffset])) & 0x1ff);
									if (xPos < -8) xPos += 512;
									if (xPos >= 512) xPos -= 512;
								} else {
									xPos -= BgScrollX & 0x3ff;
									if (xPos < -8) xPos += 1024;
									if (xPos >= 1024) xPos -= 1024;
								}							
							
								unsigned short* pPixel = pTransDraw + (yPos * nScreenWidth);
						
								if (xPos >= 0 && xPos < nScreenWidth) {
									pPixel[xPos] = c | nPalette;
								}
							}
						}
					}
				}
			} else {
				if (!PC080SNDblWidth) {
					x -= BgScrollX & 0x1ff;
					if (x < -8) x += 512;
					if (x >= 512) x -= 512;
				} else {
					x -= BgScrollX & 0x3ff;
					if (x < -8) x += 1024;
					if (x >= 1024) x -= 1024;
				}
				
				y -= BgScrollY & 0x1ff;
				if (y < -8) y += 512;
				if (y >= 512) y -= 512;
				
				if (Opaque) {
					if (x > 8 && x < (nScreenWidth - 8) && y > 8 && y < (nScreenHeight - 8)) {
						if (xFlip) {
							if (yFlip) {
								Render8x8Tile_FlipXY(pTransDraw, Code, x , y, Colour, 4, 0, pSrc);
							} else {
								Render8x8Tile_FlipX(pTransDraw, Code, x , y, Colour, 4, 0, pSrc);
							}
						} else {
							if (yFlip) {
								Render8x8Tile_FlipY(pTransDraw, Code, x , y, Colour, 4, 0, pSrc);
							} else {
								Render8x8Tile(pTransDraw, Code, x , y, Colour, 4, 0, pSrc);
							}
						}
					} else {
						if (xFlip) {
							if (yFlip) {
								Render8x8Tile_FlipXY_Clip(pTransDraw, Code, x , y, Colour, 4, 0, pSrc);
							} else {
								Render8x8Tile_FlipX_Clip(pTransDraw, Code, x , y, Colour, 4, 0, pSrc);
							}
						} else {
							if (yFlip) {
								Render8x8Tile_FlipY_Clip(pTransDraw, Code, x , y, Colour, 4, 0, pSrc);
							} else {
								Render8x8Tile_Clip(pTransDraw, Code, x , y, Colour, 4, 0, pSrc);
							}
						}
					}
				} else {
					if (x > 8 && x < (nScreenWidth - 8) && y > 8 && y < (nScreenHeight - 8)) {
						if (xFlip) {
							if (yFlip) {
								Render8x8Tile_Mask_FlipXY(pTransDraw, Code, x , y, Colour, 4, 0, 0, pSrc);
							} else {
								Render8x8Tile_Mask_FlipX(pTransDraw, Code, x , y, Colour, 4, 0, 0, pSrc);
							}
						} else {
							if (yFlip) {
								Render8x8Tile_Mask_FlipY(pTransDraw, Code, x , y, Colour, 4, 0, 0, pSrc);
							} else {
								Render8x8Tile_Mask(pTransDraw, Code, x , y, Colour, 4, 0, 0, pSrc);
							}
						}
					} else {
						if (xFlip) {
							if (yFlip) {
								Render8x8Tile_Mask_FlipXY_Clip(pTransDraw, Code, x , y, Colour, 4, 0, 0, pSrc);
							} else {
								Render8x8Tile_Mask_FlipX_Clip(pTransDraw, Code, x , y, Colour, 4, 0, 0, pSrc);
							}
						} else {
							if (yFlip) {
								Render8x8Tile_Mask_FlipY_Clip(pTransDraw, Code, x , y, Colour, 4, 0, 0, pSrc);
							} else {
								Render8x8Tile_Mask_Clip(pTransDraw, Code, x , y, Colour, 4, 0, 0, pSrc);
							}
						}
					}
				}
			}
			
			TileIndex++;
		}
	}
}

void PC080SNDrawFgLayer(unsigned char *pSrc)
{
	int mx, my, Offset, Attr, Code, Colour, x, y, TileIndex = 0, Flip, xFlip, yFlip;
	
	UINT16 *VideoRam = (UINT16*)PC080SNRam + 0x4000;
	UINT16 *FgScrollRam = NULL;

	if (!PC080SNDblWidth) FgScrollRam = (UINT16*)PC080SNRam + 0x6000;
	
	int FgScrollActive = 0;
	if (FgScrollRam) {
		for (int i = 0; i < 512; i++) {
			if (FgScrollRam[i]) {
				FgScrollActive = 1;
				break;
			}
		}
	}
	
	for (my = 0; my < 64; my++) {
		for (mx = 0; mx < PC080SNCols; mx++) {
			Offset = 2 * TileIndex;
			if (!PC080SNDblWidth) {
				Attr = swapWord(VideoRam[Offset + 0]);
				Code = swapWord(VideoRam[Offset + 1]) & (PC080SNNumTiles - 1);
			} else {
				Attr = swapWord(VideoRam[TileIndex + 0x0000]);
				Code = swapWord(VideoRam[TileIndex + 0x2000]) & 0x3fff;
			}
			Colour = Attr & 0x1ff;
			Flip = (Attr & 0xc000) >> 14;
			xFlip = (Flip >> 0) & 0x01;
			yFlip = (Flip >> 1) & 0x01;
			
			x = 8 * mx;
			y = 8 * my;
			
			x -= 16;
			
			x -= PC080SNXOffset;
			y -= PC080SNYOffset;
			
			if (FgScrollActive) {
				int px, py;
			
				UINT32 nPalette = Colour << 4;
			
				for (py = 0; py < 8; py++) {
					for (px = 0; px < 8; px++) {
						unsigned char c = pSrc[(Code * 64) + (py * 8) + px];
						if (xFlip) c = pSrc[(Code * 64) + (py * 8) + (7 - px)];
						if (yFlip) c = pSrc[(Code * 64) + ((7 - py) * 8) + px];
						if (xFlip && yFlip) c = pSrc[(Code * 64) + ((7 - py) * 8) + (7 - px)];
					
						if (c != PC080SNFgTransparentPen) {
							int xPos = x + px;
							int yPos = y + py;
							yPos -= FgScrollY & 0x1ff;
					
							if (yPos < -8) yPos += 512;
							if (yPos >= 512) yPos -= 512;			
					
							if (yPos >= 0 && yPos < nScreenHeight) {					
								if (!PC080SNDblWidth) {
									xPos -= ((FgScrollX - FgScrollRam[yPos + PC080SNYOffset]) & 0x1ff);
									if (xPos < -8) xPos += 512;
									if (xPos >= 512) xPos -= 512;
								} else {
									xPos -= FgScrollX & 0x3ff;
									if (xPos < -8) xPos += 1024;
									if (xPos >= 1024) xPos -= 1024;
								}
						
								unsigned short* pPixel = pTransDraw + (yPos * nScreenWidth);
							
								if (xPos >= 0 && xPos < nScreenWidth) {
									pPixel[xPos] = c | nPalette;
								}
							}
						}
					}
				}
			} else {
				if (!PC080SNDblWidth) {
					x -= FgScrollX & 0x1ff;
					if (x < -8) x += 512;
					if (x >= 512) x -= 512;
				} else {
					x -= FgScrollX & 0x3ff;
					if (x < -8) x += 1024;
					if (x >= 1024) x -= 1024;
				}
				
				y -= FgScrollY & 0x1ff;
				if (y < -8) y += 512;
				if (y >= 512) y -= 512;
				
				if (x > 8 && x < (nScreenWidth - 8) && y > 8 && y < (nScreenHeight - 8)) {
					if (xFlip) {
						if (yFlip) {
							Render8x8Tile_Mask_FlipXY(pTransDraw, Code, x , y, Colour, 4, PC080SNFgTransparentPen, 0, pSrc);
						} else {
							Render8x8Tile_Mask_FlipX(pTransDraw, Code, x , y, Colour, 4, PC080SNFgTransparentPen, 0, pSrc);
						}
					} else {
						if (yFlip) {
							Render8x8Tile_Mask_FlipY(pTransDraw, Code, x , y, Colour, 4, PC080SNFgTransparentPen, 0, pSrc);
						} else {
							Render8x8Tile_Mask(pTransDraw, Code, x , y, Colour, 4, PC080SNFgTransparentPen, 0, pSrc);
						}
					}
				} else {
					if (xFlip) {
						if (yFlip) {
							Render8x8Tile_Mask_FlipXY_Clip(pTransDraw, Code, x , y, Colour, 4, PC080SNFgTransparentPen, 0, pSrc);
						} else {
							Render8x8Tile_Mask_FlipX_Clip(pTransDraw, Code, x , y, Colour, 4, PC080SNFgTransparentPen, 0, pSrc);
						}
					} else {
						if (yFlip) {
							Render8x8Tile_Mask_FlipY_Clip(pTransDraw, Code, x , y, Colour, 4, PC080SNFgTransparentPen, 0, pSrc);
						} else {
							Render8x8Tile_Mask_Clip(pTransDraw, Code, x , y, Colour, 4, PC080SNFgTransparentPen, 0, pSrc);
						}
					}
				}
			}
			
			TileIndex++;
		}
	}
}

void PC080SNSetScrollX(unsigned int Offset, UINT16 Data)
{
	PC080SNCtrl[Offset] = Data;
	
	switch (Offset) {
		case 0: {
			BgScrollX = -Data;
			break;
		}
		
		case 1: {
			FgScrollX = -Data;
			break;
		}
	}
}

void PC080SNSetScrollY(unsigned int Offset, UINT16 Data)
{
	PC080SNCtrl[Offset + 2] = Data;
	
	if (PC080SNYInvert) Data = -Data;
	
	switch (Offset) {
		case 0: {
			BgScrollY = -Data;
			break;
		}
		
		case 1: {
			FgScrollY = -Data;
			break;
		}
	}
}

void PC080SNCtrlWrite(unsigned int Offset, UINT16 Data)
{
	PC080SNCtrl[Offset + 4] = Data;
	
	switch (Offset) {
		case 0: {
			if (Data & 1) bprintf(PRINT_NORMAL, _T("PC080SN Flipped\n"));
			return;
		}
	}
}

void PC080SNOverrideFgScroll(int xScroll, int yScroll)
{
	FgScrollX = xScroll;
	FgScrollY = yScroll;
}

void PC080SNReset()
{
	memset(PC080SNCtrl, 0, 8 * sizeof(UINT16));
	BgScrollX = 0;
	BgScrollY = 0;
	FgScrollX = 0;
	FgScrollY = 0;
}

void PC080SNInit(int nNumTiles, int xOffset, int yOffset, int yInvert, int DblWidth)
{
	PC080SNRam = (unsigned char*)malloc(0x10000);
	memset(PC080SNRam, 0, 0x10000);
	
	PC080SNNumTiles = nNumTiles;
	
	PC080SNXOffset = xOffset;
	PC080SNYOffset = yOffset;
	
	PC080SNFgTransparentPen = 0;
	PC080SNYInvert = yInvert;
	
	TaitoIC_PC080SNInUse = 1;
	
	PC080SNCols = 64;
	if (DblWidth) PC080SNCols = 128;
	
	PC080SNDblWidth = DblWidth;
}

void PC080SNSetFgTransparentPen(int Pen)
{
	PC080SNFgTransparentPen = Pen;
}

void PC080SNExit()
{
	free(PC080SNRam);
	PC080SNRam = NULL;

	memset(PC080SNCtrl, 0, 8 * sizeof(UINT16));
	BgScrollX = 0;
	BgScrollY = 0;
	FgScrollX = 0;
	FgScrollY = 0;
	
	PC080SNNumTiles = 0;
	PC080SNXOffset = 0;
	PC080SNYOffset = 0;
	PC080SNFgTransparentPen = 0;
	PC080SNYInvert = 0;
	PC080SNDblWidth = 0;
	PC080SNCols = 0;
}

void PC080SNScan(int nAction)
{
	struct BurnArea ba;
	
	if (nAction & ACB_MEMORY_RAM) {
		memset(&ba, 0, sizeof(ba));
		ba.Data	  = PC080SNRam;
		ba.nLen	  = 0x10000;
		ba.szName = "PC080SN Ram";
		BurnAcb(&ba);
	}
	
	if (nAction & ACB_DRIVER_DATA) {
		SCAN_VAR(PC080SNCtrl);
		SCAN_VAR(BgScrollX);
		SCAN_VAR(BgScrollY);
		SCAN_VAR(FgScrollX);
		SCAN_VAR(FgScrollY);
	}
}
