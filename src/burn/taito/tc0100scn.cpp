// TC0100SCN

#include "tiles_generic.h"
#include "taito_ic.h"

unsigned char *TC0100SCNRam[TC0100SCN_MAX_CHIPS];
static unsigned char *TC0100SCNChars;
UINT16 TC0100SCNCtrl[TC0100SCN_MAX_CHIPS][8];
static int BgScrollX;
static int BgScrollY;
static int FgScrollX;
static int FgScrollY;
static int CharScrollX;
static int CharScrollY;
static int TC0100SCNTilesNum;
static int TC0100SCNXOffset;
static int TC0100SCNYOffset;
static int TC0100SCNFlipScreenX;
static int TC0100SCNFlip;
static unsigned char *TC0100SCNPriorityMap = NULL;
static int TC0100SCNColourDepth;
static int TC0100SCNGfxBank;
static int TC0100SCNGfxMask;
static int TC0100SCNDblWidth;

void TC0100SCNCtrlWordWrite(unsigned int Offset, UINT16 Data)
{
	TC0100SCNCtrl[0][Offset] = Data;
	
	switch (Offset) {
		case 0x00: {
			BgScrollX = -Data;
			return;
		}
		
		case 0x01: {
			FgScrollX = -Data;
			return;
		}
		
		case 0x02: {
			CharScrollX = -Data;
			return;
		}
		
		case 0x03: {
			BgScrollY = -Data;
			return;
		}
		
		case 0x04: {
			FgScrollY = -Data;
			return;
		}
		
		case 0x05: {
			CharScrollY = -Data;
			return;
		}
		
		case 0x06: {
			TC0100SCNDblWidth = (Data & 0x10) >> 4;
			return;
		}
		
		case 0x07: {
			TC0100SCNFlip = (Data & 0x01) ? 1: 0;
			return;
		}
	}
	
	bprintf(PRINT_IMPORTANT, _T("TC0100 Ctrl Word Write %02X, %04X\n"), Offset, Data);
}

int TC0100SCNBottomLayer()
{
	return (TC0100SCNCtrl[0][6] & 0x08) >> 3;
}

void TC0100SCNRenderBgLayer(int Opaque, unsigned char *pSrc)
{
	int mx, my, Attr, Code, Colour, x, y, TileIndex = 0, Offset, Flip, xFlip, yFlip;
	
	UINT16 *VideoRam = (UINT16*)TC0100SCNRam[0];
	UINT16 *BgScrollRam = (UINT16*)TC0100SCNRam[0] + (0xc000 / 2);
	int Columns = 64;
	
	if (TC0100SCNDblWidth) {
		VideoRam = (UINT16*)TC0100SCNRam[0];
		BgScrollRam = (UINT16*)TC0100SCNRam[0] + (0x10000 / 2);
		Columns = 128;
	}
	
	for (my = 0; my < 64; my++) {
		for (mx = 0; mx < Columns; mx++) {
			Offset = 2 * TileIndex;
			Attr = swapWord(VideoRam[Offset]);
			Code = (swapWord(VideoRam[Offset + 1]) & TC0100SCNGfxMask) + (TC0100SCNGfxBank << 15);
			Colour = Attr & 0xff;
			Flip = (Attr & 0xc000) >> 14;
			xFlip = (Flip >> 0) & 0x01;
			yFlip = (Flip >> 1) & 0x01;
			
			x = 8 * mx;
			y = 8 * my;
			
			x -= 16;
			if (!TC0100SCNFlip && !TC0100SCNFlipScreenX) x -= TC0100SCNXOffset;
			if (TC0100SCNFlip || TC0100SCNFlipScreenX) x += TC0100SCNXOffset;
			y -= TC0100SCNYOffset;
			
			if (TC0100SCNFlipScreenX) {
				xFlip = !xFlip;
				x = nScreenWidth - 8 - x;
			}
			
			if (TC0100SCNFlip) {
				xFlip = !xFlip;
				x = nScreenWidth - 8 - x;
				yFlip = !yFlip;
				y = nScreenHeight + 8 - y;
			}
						
			int px, py;
			
			UINT32 nPalette = Colour << TC0100SCNColourDepth;
			
			for (py = 0; py < 8; py++) {
				for (px = 0; px < 8; px++) {
					unsigned char c = pSrc[(Code * 64) + (py * 8) + px];
					if (xFlip) c = pSrc[(Code * 64) + (py * 8) + (7 - px)];
					if (yFlip) c = pSrc[(Code * 64) + ((7 - py) * 8) + px];
					if (xFlip && yFlip) c = pSrc[(Code * 64) + ((7 - py) * 8) + (7 - px)];
					
					if (c || Opaque) {
						int xPos = x + px;
						int yPos = y + py;
						
						if (!TC0100SCNFlip) yPos -= BgScrollY & 0x1ff;
						if (TC0100SCNFlip) yPos += BgScrollY & 0x1ff;
					
						if (yPos < -8) yPos += 512;
						if (yPos >= 512) yPos -= 512;
					
						if (yPos >= 0 && yPos < nScreenHeight) {					
							if (!TC0100SCNDblWidth) {							
								if (!TC0100SCNFlip && !TC0100SCNFlipScreenX) xPos -= ((BgScrollX - swapWord(BgScrollRam[yPos + TC0100SCNYOffset])) & 0x1ff);
								if (TC0100SCNFlip || TC0100SCNFlipScreenX) xPos += ((BgScrollX + swapWord(BgScrollRam[yPos + TC0100SCNYOffset])) & 0x1ff);
						
								if (xPos < -8) xPos += 512;
								if (xPos >= 512) xPos -= 512;
							} else {
								if (!TC0100SCNFlip && !TC0100SCNFlipScreenX) xPos -= ((BgScrollX - swapWord(BgScrollRam[yPos + TC0100SCNYOffset])) & 0x3ff);
								if (TC0100SCNFlip || TC0100SCNFlipScreenX) xPos += ((BgScrollX + swapWord(BgScrollRam[yPos + TC0100SCNYOffset])) & 0x3ff);
						
								if (xPos < -8) xPos += 1024;
								if (xPos >= 1024) xPos -= 1024;
							}
						
							unsigned short* pPixel = pTransDraw + (yPos * nScreenWidth);
						
							if (xPos >= 0 && xPos < nScreenWidth) {
								pPixel[xPos] = c | nPalette;
								if (TC0100SCNPriorityMap) TC0100SCNPriorityMap[(yPos * nScreenWidth) + xPos] = 1;
							}
						}
					}
				}
			}
			
			TileIndex++;
		}
	}
}


void TC0100SCNRenderBgLayerHack(unsigned char *pSrc)
{
	int mx, my, Attr, Code, Colour, x, y, TileIndex = 0, Offset, Flip, xFlip, yFlip;
	
	UINT16 *VideoRam = (UINT16*)TC0100SCNRam[0];
	UINT16 *BgScrollRam = (UINT16*)TC0100SCNRam[0] + (0xc000 / 2);
	int Columns = 64;
 
	for (my = 0; my < 64; my++) {
		for (mx = 0; mx < Columns; mx++) {
			Offset = 2 * TileIndex;
			Attr = swapWord(VideoRam[Offset]);
			Code = (swapWord(VideoRam[Offset + 1]) & TC0100SCNGfxMask) + (TC0100SCNGfxBank << 15);
			Colour = Attr & 0xff;
			Flip = (Attr & 0xc000) >> 14;
			xFlip = (Flip >> 0) & 0x01;
			yFlip = (Flip >> 1) & 0x01;
			
			x = 8 * mx;
			y = 8 * my;
			
			x -= 16;
			x -= TC0100SCNXOffset;		 
			y -= TC0100SCNYOffset;
			
			 
						
			int px, py;
			
			UINT32 nPalette = Colour << TC0100SCNColourDepth;
			
			for (py = 0; py < 8; py++) {
				for (px = 0; px < 8; px++) {
					unsigned char c = pSrc[(Code * 64) + (py * 8) + px];
				 	
					if (c ) {
						int xPos = x + px;
						int yPos = y + py;
						
						yPos -= BgScrollY & 0x1ff;
						 
					
						if (yPos < -8) yPos += 512;
						if (yPos >= 512) yPos -= 512;
					
						if (yPos >= 0 && yPos < nScreenHeight) {					
							 						
								 xPos -= ((BgScrollX - swapWord(BgScrollRam[yPos + TC0100SCNYOffset])) & 0x1ff);
							 
								if (xPos < -8) xPos += 512;
								if (xPos >= 512) xPos -= 512;
							
						
							unsigned short* pPixel = pTransDraw + (yPos * nScreenWidth);
						
							if (xPos >= 0 && xPos < nScreenWidth) {
								pPixel[xPos] = c | nPalette;								
							}
						}
					}
				}
			}
			
			TileIndex++;
		}
	}
}


void TC0100SCNRenderFgLayer(int Opaque, unsigned char *pSrc)
{
	int mx, my, Attr, Code, Colour, x, y, TileIndex = 0, Offset, Flip, xFlip, yFlip;
	
	UINT16 *VideoRam = (UINT16*)TC0100SCNRam[0] + (0x8000 / 2);
	UINT16 *FgScrollRam = (UINT16*)TC0100SCNRam[0] + (0xc400 / 2);
	UINT16 *FgColScrollRam = (UINT16*)TC0100SCNRam[0] + (0xe000 / 2);
	int Columns = 64;
	
	if (TC0100SCNDblWidth) {
		//VideoRam = (UINT16*)TC0100SCNRam[0] + (0x8000 / 2);
		FgScrollRam = (UINT16*)TC0100SCNRam[0] + (0x10400 / 2);
		FgColScrollRam = (UINT16*)TC0100SCNRam[0] + (0x10800 / 2);
		Columns = 128;
	}
	
#pragma omp parallel private(mx, my) num_threads(2)
{
#pragma omp for
	for (my = 0; my < 64; my++) {
		for (mx = 0; mx < Columns; mx++) {
			Offset = 2 * TileIndex;
			Attr = swapWord(VideoRam[Offset]);
			Code = (swapWord(VideoRam[Offset + 1]) & TC0100SCNGfxMask) + (TC0100SCNGfxBank << 15);
			Colour = Attr & 0xff;
			Flip = (Attr & 0xc000) >> 14;
			xFlip = (Flip >> 0) & 0x01;
			yFlip = (Flip >> 1) & 0x01;
			
			x = 8 * mx;
			y = 8 * my;
			
			x -= 16;
			(!TC0100SCNFlip && !TC0100SCNFlipScreenX) ? x -= TC0100SCNXOffset : x += TC0100SCNXOffset;
			 
			y -= TC0100SCNYOffset;
			
			if (TC0100SCNFlipScreenX) {
				xFlip = !xFlip;
				x = nScreenWidth - 8 - x;
			}
			
			if (TC0100SCNFlip) {
				xFlip = !xFlip;
				x = nScreenWidth - 8 - x;
				yFlip = !yFlip;
				y = nScreenHeight + 8 - y;
			}
			
			int px, py;
			
			UINT32 nPalette = Colour << TC0100SCNColourDepth;
			
			for (py = 0; py < 8; py++) {
				for (px = 0; px < 8; px++) {
					unsigned char c = pSrc[(Code * 64) + (py * 8) + px];
					xFlip ? c = pSrc[(Code * 64) + (py * 8) + (7 - px)] : 0;
					yFlip ? c = pSrc[(Code * 64) + ((7 - py) * 8) + px] : 0;
					(xFlip && yFlip) ? c = pSrc[(Code * 64) + ((7 - py) * 8) + (7 - px)] : 0;
					
					if (c || Opaque) {
						int xPos = x + px;
						int yPos = y + py;

						(!TC0100SCNFlip) ? yPos -= (FgScrollY - swapWord(FgColScrollRam[mx])) & 0x1ff : yPos += (FgScrollY + swapWord(FgColScrollRam[mx])) & 0x1ff;
						 
						if (yPos < -8) yPos += 512;
						if (yPos >= 512) yPos -= 512;
					
						if (yPos >= 0 && yPos < nScreenHeight) {
							if (!TC0100SCNDblWidth) {					
								(!TC0100SCNFlip && !TC0100SCNFlipScreenX) ? xPos -= ((FgScrollX - swapWord(FgScrollRam[yPos + TC0100SCNYOffset])) & 0x1ff) : xPos += ((FgScrollX + swapWord(FgScrollRam[yPos + TC0100SCNYOffset])) & 0x1ff);								 						
								if (xPos < -8) xPos += 512;
								if (xPos >= 512) xPos -= 512;
							} else {
								(!TC0100SCNFlip && !TC0100SCNFlipScreenX) ? xPos -= ((FgScrollX - swapWord(FgScrollRam[yPos + TC0100SCNYOffset])) & 0x3ff) : xPos += ((FgScrollX + swapWord(FgScrollRam[yPos + TC0100SCNYOffset])) & 0x3ff);
 						
								if (xPos < -8) xPos += 1024;
								if (xPos >= 1024) xPos -= 1024;
							}
						
							unsigned short* pPixel = pTransDraw + (yPos * nScreenWidth);
						
							if (xPos >= 0 && xPos < nScreenWidth) {
								pPixel[xPos] = c | nPalette;
								TC0100SCNPriorityMap ? TC0100SCNPriorityMap[(yPos * nScreenWidth) + xPos] = 2 : 0;
							}
						}
					}
				}
			}
			
			TileIndex++;
		}
	}
}
}

/// Hack version for chasehq/others

void TC0100SCNRenderFgLayerHack(unsigned char *pSrc)
{
	int mx, my, Attr, Code, Colour, x, y, TileIndex = 0, Offset, Flip, xFlip, yFlip;
	
	UINT16 *VideoRam = (UINT16*)TC0100SCNRam[0] + (0x8000 / 2);
	UINT16 *FgScrollRam = (UINT16*)TC0100SCNRam[0] + (0xc400 / 2);
	UINT16 *FgColScrollRam = (UINT16*)TC0100SCNRam[0] + (0xe000 / 2);
	int Columns = 64;
	
#pragma omp parallel private(mx, my) num_threads(2)

{

#pragma omp for
	for (my = 0; my < 64; my++) {
		for (mx = 0; mx < Columns; mx++) {
			Offset = 2 * TileIndex;
			Attr = swapWord(VideoRam[Offset]);
			Code = (swapWord(VideoRam[Offset + 1]) & TC0100SCNGfxMask) + (TC0100SCNGfxBank << 15);
			Colour = Attr & 0xff;
			Flip = (Attr & 0xc000) >> 14;
			xFlip = (Flip >> 0) & 0x01;
			yFlip = (Flip >> 1) & 0x01;
			
			x = 8 * mx;
			y = 8 * my;
			
			x -= 16;
			x -= TC0100SCNXOffset;			 
			y -= TC0100SCNYOffset;
					
			
			int px, py;
			
			UINT32 nPalette = Colour << TC0100SCNColourDepth;
			
			for (py = 0; py < 8; py++) {
				for (px = 0; px < 8; px++) {
					unsigned char c = pSrc[(Code * 64) + (py * 8) + px];
					 
					if (c) {
						int xPos = x + px;
						int yPos = y + py;

						yPos -= (FgScrollY -  (swapWord(FgColScrollRam[mx]))) & 0x1ff;
						 
						if (yPos < -8) 
							yPos += 512;
						if (yPos >= 512) 
							yPos -= 512;
					
						if (yPos >= 0 && yPos < nScreenHeight) {
							 				
							xPos -= ((FgScrollX - (swapWord(FgScrollRam[yPos + TC0100SCNYOffset]))) & 0x1ff); 
							if (xPos < -8) xPos += 512;
							if (xPos >= 512) xPos -= 512;
								
							unsigned short* pPixel = pTransDraw + (yPos * nScreenWidth);

#if defined (_XBOX)
							__dcbt(0, &pPixel[xPos]);
#endif

							if (xPos >= 0 && xPos < nScreenWidth) {									 
								pPixel[xPos] = c | nPalette;							
							}
						}
					}
				}
			}
			
			TileIndex++;
		}
	}
}
}


static int TC0100SCNPlaneOffsets[2] = { 8, 0 };
static int TC0100SCNXOffsets[8]     = { 0, 1, 2, 3, 4, 5, 6, 7 };
static int TC0100SCNYOffsets[8]     = { 0, 16, 32, 48, 64, 80, 96, 112 };

void TC0100SCNRenderCharLayer()
{
	int mx, my, Attr, Code, Colour, x, y, TileIndex = 0, Flip, xFlip, yFlip;
	
	UINT16 *VideoRam = (UINT16*)TC0100SCNRam[0] + (0x4000 / 2);
	UINT16 *CharRam = (UINT16*)TC0100SCNRam[0] + (0x6000 / 2);
	int Columns = 64;
	int Rows = 64;
	
	if (TC0100SCNDblWidth) {
		VideoRam = (UINT16*)TC0100SCNRam[0] + (0x12000 / 2);
		CharRam = (UINT16*)TC0100SCNRam[0] + (0x11000 / 2);
		Columns = 128;
		Rows = 32;
	}
	
	GfxDecode(256, 2, 8, 8, TC0100SCNPlaneOffsets, TC0100SCNXOffsets, TC0100SCNYOffsets, 0x80, (UINT8*)CharRam, TC0100SCNChars);
	
	for (my = 0; my < Rows; my++) {
		for (mx = 0; mx < Columns; mx++) {
			Attr = swapWord(VideoRam[TileIndex]);
			Code = Attr & 0xff;
			Colour = (Attr >> 6) & 0xfc;
			Flip = (Attr & 0xc000) >> 14;
			xFlip = (Flip >> 0) & 0x01;
			yFlip = (Flip >> 1) & 0x01;
			
			x = 8 * mx;
			y = 8 * my;
			
			x -= 16;
			if (!TC0100SCNFlip && !TC0100SCNFlipScreenX) x -= TC0100SCNXOffset;
			if (TC0100SCNFlip || TC0100SCNFlipScreenX) x += TC0100SCNXOffset;
			y -= TC0100SCNYOffset;
			
			if (TC0100SCNFlipScreenX) {
				xFlip = !xFlip;
				x = nScreenWidth - 8 - x;
			}
			
			if (TC0100SCNFlip) {
				xFlip = !xFlip;
				x = nScreenWidth - x;
				yFlip = !yFlip;
				y = nScreenHeight + 8 - y;
			}
			
			if (!TC0100SCNDblWidth) {
				if (!TC0100SCNFlip && !TC0100SCNFlipScreenX) x -= CharScrollX & 0x1ff;
				if (TC0100SCNFlip || TC0100SCNFlipScreenX) x += CharScrollX & 0x1ff;
				
				if (x < -8) x += 512;
				if (x >= 512) x -= 512;
				
				if (!TC0100SCNFlip) y -= CharScrollY & 0x1ff;
				if (TC0100SCNFlip) y += CharScrollY & 0x1ff;
				
				if (y < -8) y += 512;
				if (y >= 512) y -= 512;
			} else {
				if (!TC0100SCNFlip && !TC0100SCNFlipScreenX) x -= CharScrollX & 0x3ff;
				if (TC0100SCNFlip || TC0100SCNFlipScreenX) x += CharScrollX & 0x3ff;
				
				if (x < -8) x += 1024;
				if (x >= 1024) x -= 1024;
				
				if (!TC0100SCNFlip) y -= CharScrollY & 0xff;
				if (TC0100SCNFlip) y += CharScrollY & 0xff;
				
				if (y < -8) y += 256;
				if (y >= 256) y -= 256;
			}
			
			int px, py;
			
			UINT32 nPalette = Colour << 2;
			
			for (py = 0; py < 8; py++) {
				for (px = 0; px < 8; px++) {
					unsigned char c = TC0100SCNChars[(Code * 64) + (py * 8) + px];
					if (xFlip) c = TC0100SCNChars[(Code * 64) + (py * 8) + (7 - px)];
					if (yFlip) c = TC0100SCNChars[(Code * 64) + ((7 - py) * 8) + px];
					if (xFlip && yFlip) c = TC0100SCNChars[(Code * 64) + ((7 - py) * 8) + (7 - px)];
					
					if (c) {
						int xPos = x + px;
						int yPos = y + py;

						if (yPos >= 0 && yPos < nScreenHeight) {					
							unsigned short* pPixel = pTransDraw + (yPos * nScreenWidth);
						
							(xPos >= 0 && xPos < nScreenWidth) ? (pPixel[xPos] = c | nPalette) : 0;								
							if (TC0100SCNPriorityMap) TC0100SCNPriorityMap[(yPos * nScreenWidth) + xPos] = 4;
							 
						}
					}
				}
			}			
			
			TileIndex++;
		}
	}
}

void TC0100SCNRenderCharLayerHack()
{
	int mx, my, Attr, Code, Colour, x, y, TileIndex = 0, Flip, xFlip, yFlip;
	
	UINT16 *VideoRam = (UINT16*)TC0100SCNRam[0] + (0x4000 / 2);
	UINT16 *CharRam = (UINT16*)TC0100SCNRam[0] + (0x6000 / 2);
	int Columns = 64;
	int Rows = 64;
 
	GfxDecode(256, 2, 8, 8, TC0100SCNPlaneOffsets, TC0100SCNXOffsets, TC0100SCNYOffsets, 0x80, (UINT8*)CharRam, TC0100SCNChars);
	
	for (my = 0; my < Rows; my++) {
		for (mx = 0; mx < Columns; mx++) {
			Attr = swapWord(VideoRam[TileIndex]);
			Code = Attr & 0xff;
			Colour = (Attr >> 6) & 0xfc;
			Flip = (Attr & 0xc000) >> 14;
			xFlip = (Flip >> 0) & 0x01;
			yFlip = (Flip >> 1) & 0x01;
			
			x = 8 * mx;
			y = 8 * my;
			
			x -= 16;
			x -= TC0100SCNXOffset;			 
			y -= TC0100SCNYOffset;
			
			x -= CharScrollX & 0x1ff;
			if (x < -8) x += 512;
			if (x >= 512) x -= 512;

			y -= CharScrollY & 0x1ff;

			if (y < -8) y += 512;
			if (y >= 512) y -= 512;
 
			int px, py;
			
			UINT32 nPalette = Colour << 2;
			
			for (py = 0; py < 8; py++) {
				for (px = 0; px < 8; px++) {
					unsigned char c = TC0100SCNChars[(Code * 64) + (py * 8) + px];
				 	
					if (c) {
						int xPos = x + px;
						int yPos = y + py;

						if (yPos >= 0 && yPos < nScreenHeight) {					
							unsigned short* pPixel = pTransDraw + (yPos * nScreenWidth);
						
							(xPos >= 0 && xPos < nScreenWidth) ? (pPixel[xPos] = c | nPalette) : 0;								
							  
						}
					}
				}
			}			
			
			TileIndex++;
		}
	}
}


void TC0100SCNReset()
{
	memset(TC0100SCNChars, 0, 256 * 8 * 8);
	memset(TC0100SCNCtrl[0], 0, 8);
	BgScrollX = 0;
	BgScrollY = 0;
	FgScrollX = 0;
	FgScrollY = 0;
	CharScrollX = 0;
	CharScrollY = 0;
	TC0100SCNFlip = 0;
	TC0100SCNGfxBank = 0;
	
	TC0100SCNDblWidth = 0;
}

void TC0100SCNInit(int Num, int nNumTiles, int xOffset, int yOffset, int xFlip, unsigned char *PriorityMap)
{
	for (int i = 0; i < Num; i++) {
		TC0100SCNRam[i] = (unsigned char*)malloc(0x14000);
		memset(TC0100SCNRam[i], 0, 0x14000);
	}
		
	TC0100SCNChars = (unsigned char*)malloc(256 * 8 * 8);
	memset(TC0100SCNChars, 0, 256 * 8 * 8);
	
	TC0100SCNTilesNum = nNumTiles;
	
	TC0100SCNXOffset = xOffset;
	TC0100SCNYOffset = yOffset;
	
	TC0100SCNFlipScreenX = xFlip;
	
	TC0100SCNPriorityMap = PriorityMap;
	
	TC0100SCNColourDepth = 4;
	
	TC0100SCNGfxMask = nNumTiles - 1;
	
	TC0100SCNDblWidth = 0;
	
	TaitoIC_TC0100SCNInUse = 1;
}

void TC0100SCNSetColourDepth(int ColourDepth)
{
	TC0100SCNColourDepth = ColourDepth;
}

void TC0100SCNSetGfxMask(int Mask)
{
	TC0100SCNGfxMask = Mask;
}

void TC0100SCNSetGfxBank(int Bank)
{
	TC0100SCNGfxBank = Bank & 0x01;
}

void TC0100SCNExit()
{
	for (int i = 0; i < TC0100SCN_MAX_CHIPS; i++) {
		free(TC0100SCNRam[i]);
		TC0100SCNRam[i] = NULL;
		
		memset(TC0100SCNCtrl[i], 0, 8);
	}
	
	free(TC0100SCNChars);
	TC0100SCNChars = NULL;	
	
	BgScrollX = 0;
	BgScrollY = 0;
	FgScrollX = 0;
	FgScrollY = 0;
	CharScrollX = 0;
	CharScrollY = 0;
	
	TC0100SCNTilesNum = 0;
	TC0100SCNXOffset = 0;
	TC0100SCNYOffset = 0;
	TC0100SCNFlipScreenX = 0;
	TC0100SCNFlip = 0;
	TC0100SCNColourDepth = 0;
	TC0100SCNGfxBank = 0;
	TC0100SCNGfxMask = 0;
	TC0100SCNDblWidth = 0;
	
	TC0100SCNPriorityMap = NULL;
}

void TC0100SCNScan(int nAction)
{
	struct BurnArea ba;
	
	if (nAction & ACB_MEMORY_RAM) {
		memset(&ba, 0, sizeof(ba));
		ba.Data	  = TC0100SCNRam[0];
		ba.nLen	  = 0x14000;
		ba.szName = "TC0100SCN Ram";
		BurnAcb(&ba);
	}
	
	if (nAction & ACB_DRIVER_DATA) {
		SCAN_VAR(TC0100SCNCtrl[0]);
		SCAN_VAR(BgScrollX);
		SCAN_VAR(BgScrollY);
		SCAN_VAR(FgScrollX);
		SCAN_VAR(FgScrollY);
		SCAN_VAR(CharScrollX);
		SCAN_VAR(CharScrollY);
		SCAN_VAR(TC0100SCNFlip);
		SCAN_VAR(TC0100SCNGfxBank);
		SCAN_VAR(TC0100SCNDblWidth);
	}
}
