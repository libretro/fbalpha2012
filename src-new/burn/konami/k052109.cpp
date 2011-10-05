// K052109

#include "tiles_generic.h"
#include "konamiic.h"

static unsigned char *K052109Ram = NULL;

typedef void (*K052109_Callback)(int Layer, int Bank, int *Code, int *Colour, int *xFlip, int *Priority);
static K052109_Callback K052109Callback;
static int K052109ScrollX[3];
static int K052109ScrollY[3];
static unsigned char K052109ScrollCtrl;
static unsigned char K052109CharRomBank[4];
static unsigned char K052109CharRomBank2[4];
int K052109RMRDLine;
static unsigned char K052109RomSubBank;
static unsigned int K052109RomMask;
static unsigned char *K052109Rom;
static int K052109FlipEnable;
int K052109_irq_enabled;

static int K052109ScrollXOff[3];
static int K052109ScrollYOff[3];

static int K052109EnableRows[3];
static int K052109EnableLine[3];
static int K052109ScrollRows[3][256];
static int K052109EnableCols[3];
static int K052109ScrollCols[3][64];

static int has_extra_video_ram = 0; // xmen kludge

void K052109UpdateScroll()
{
	K052109EnableLine[2] = K052109EnableLine[1] = 0;
	K052109EnableRows[2] = K052109EnableRows[1] = 0;
	K052109EnableCols[2] = K052109EnableCols[1] = 0;

	if ((K052109ScrollCtrl & 0x03) == 0x02) // row scroll
	{
		unsigned char *scrollram = &K052109Ram[0x1a00];

		K052109ScrollY[1] = K052109Ram[0x180c];
		K052109EnableRows[1] = 1;

		K052109ScrollX[1] = 0;

		for (int offs = 0;offs < 32;offs++)
		{
			int xscroll = scrollram[2*(offs * 8)+0] + 256 * scrollram[2*(offs*8)+1];
			xscroll -= 6;
			K052109ScrollRows[1][offs] = xscroll & 0x1ff;
		}
	} else {
		if ((K052109ScrollCtrl & 0x03) == 0x03) { // line scroll
			unsigned char *scrollram = &K052109Ram[0x1a00];

			K052109ScrollY[1] = K052109Ram[0x180c];
			K052109ScrollX[1] = 0;

			K052109EnableLine[1] = 1;
	
			for (int offs = 0;offs < 256;offs++)
			{
				int xscroll = scrollram[2*offs+0] + 256 * scrollram[2*offs+1];
				xscroll -= 6;
				K052109ScrollRows[1][offs] = xscroll & 0x1ff;
			}
		} else {
			if ((K052109ScrollCtrl & 0x04) == 0x04) { // column scroll
				unsigned char *scrollram = &K052109Ram[0x1800];

				K052109ScrollX[1] = ((K052109Ram[0x1a00] | (K052109Ram[0x1a01] << 8)) - 6) & 0x1ff;
				K052109ScrollY[1] = 0;
				K052109EnableCols[1] = 1;

				for (int offs = 0; offs < 512; offs++) {
					K052109ScrollCols[1][((offs+K052109ScrollX[1]) & 0x1ff) / 8] = scrollram[offs/8];
				}


			} else { // scroll
				K052109ScrollX[1] = (K052109Ram[0x1a00] + (K052109Ram[0x1a01] << 8) - 6) & 0x1ff;
				K052109ScrollY[1] = K052109Ram[0x180c];
			}
		}
	}
	
	if ((K052109ScrollCtrl & 0x18) == 0x10) {
		unsigned char *scrollram = &K052109Ram[0x3a00];

		K052109ScrollY[2] = K052109Ram[0x380c];
		K052109ScrollX[2] = 0;

		K052109EnableRows[2] = 1;

		for (int offs = 0;offs < 32;offs++)
		{
			int xscroll = scrollram[2*(offs * 8)+0] + 256 * scrollram[2*(offs * 8)+1];
			xscroll -= 6;
			K052109ScrollRows[2][offs] = xscroll & 0x1ff;
		}
	} else {
		if ((K052109ScrollCtrl & 0x18) == 0x18) { // line scroll 
			unsigned char *scrollram = &K052109Ram[0x3a00];

			K052109ScrollY[2] = K052109Ram[0x380c];
			K052109ScrollX[2] = 0;

			K052109EnableLine[2] = 1;
	
			for (int offs = 0;offs < 256;offs++)
			{
				K052109ScrollRows[2][offs] = ((scrollram[2*offs+0] + (scrollram[2*offs+1] << 8)) - 6) & 0x1ff;
			}
		} else {
			if ((K052109ScrollCtrl & 0x20) == 0x20) { // column scroll
				unsigned char *scrollram = &K052109Ram[0x3800];

				K052109ScrollX[2] = ((K052109Ram[0x3a00] | (K052109Ram[0x3a01] << 8)) - 6) & 0x1ff;
				K052109ScrollY[2] = 0;
				K052109EnableCols[2] = 1;

				for (int offs = 0; offs < 512; offs++) {
					K052109ScrollCols[2][((offs+K052109ScrollX[2]) & 0x1ff) / 8] = scrollram[offs/8];
				}

			} else { // scroll
				K052109ScrollX[2] = (K052109Ram[0x3a00] + (K052109Ram[0x3a01] << 8) - 6) & 0x1ff;
				K052109ScrollY[2] = K052109Ram[0x380c];
			}
		}
	}		
}

void K052109AdjustScroll(int x, int y)
{
	for (int i = 0; i < 3; i++) {
		K052109ScrollXOff[i] = x;
		K052109ScrollYOff[i] = y;
	}
}

void K052109RenderLayerLineScroll(int nLayer, int Opaque, unsigned char *pSrc)
{
	unsigned short *dst = pTransDraw;

	int Priority = nLayer >> 4;
	nLayer &= 0x03;

	for (int my = 0; my < 256; my++) {

		int y2 = (my - (K052109ScrollYOff[nLayer] + 16)) & 0xff;
		if (y2 < 0 || y2 >= nScreenHeight) continue;

		for (int mx = 0; mx < 64; mx++) {
			int TileIndex = ((my >> 3) << 6) | mx;

			int Colour = K052109Ram[TileIndex + 0x0000];
			int Code = K052109Ram[TileIndex + 0x2000] + (K052109Ram[TileIndex + 0x4000] << 8);
			
			if (nLayer == 1) {
				Colour = K052109Ram[TileIndex + 0x0800];
				Code = K052109Ram[TileIndex + 0x2800] + (K052109Ram[TileIndex + 0x4800] << 8);
			}
			
			if (nLayer == 2) {
				Colour = K052109Ram[TileIndex + 0x1000];
				Code = K052109Ram[TileIndex + 0x3000] + (K052109Ram[TileIndex + 0x5000] << 8);
			}
			
			int Bank = K052109CharRomBank[(Colour & 0x0c) >> 2];
			if (has_extra_video_ram) Bank = (Colour & 0x0c) >> 2;	/* kludge for X-Men */

			Colour = (Colour & 0xf3) | ((Bank & 0x03) << 2);
			Bank >>= 2;
			
			int yFlip = Colour & 0x02;
			int xFlip = 0;

			int Prio = 0;

			K052109Callback(nLayer, Bank, &Code, &Colour, &xFlip, &Prio);

			if (Prio != Priority) continue;

			if (xFlip && !(K052109FlipEnable & 1)) xFlip = 0;
			if (yFlip && !(K052109FlipEnable & 2)) yFlip = 0;

			int x = 8 * mx;
			int y = my;

			y -= (K052109ScrollY[nLayer] + K052109ScrollYOff[nLayer] + 16) & 0xff;
			if (K052109EnableLine[nLayer]) x -= (104 + K052109ScrollRows[nLayer][my] + K052109ScrollXOff[nLayer]) & 0x1ff;

			if (x < -8) x += 512;
			if (y < -8) y += 256;

			if (x >= nScreenWidth) continue;

			unsigned char *src = pSrc + (Code * 0x40);
			src += (yFlip ? (~y & 7) : (y & 7)) << 3;

			if (xFlip) xFlip = 0x07;

			int col = Colour << 4;

			for (int xx = 0; xx < 8; xx++, x++) {
				if (x < 0 || x >= nScreenWidth) continue;

				int pxl = src[xx ^ xFlip];
				if (!Opaque && !pxl) continue;

				dst[x] = col | pxl;
			}
		}

		dst += nScreenWidth;
	}
}

void K052109RenderLayer(int nLayer, int Opaque, unsigned char *pSrc)
{
	if (K052109EnableLine[nLayer]) {
		K052109RenderLayerLineScroll(nLayer, Opaque, pSrc);
		return;
	}

	int mx, my, Bank, Code, Colour, x, y, xFlip = 0, yFlip, Priority, TileIndex = 0;

	Priority = nLayer >> 4;
	nLayer &= 0x03;

	for (my = 0; my < 32; my++) {
		for (mx = 0; mx < 64; mx++) {
			TileIndex = (my << 6) | mx;

			Colour = K052109Ram[TileIndex + 0x0000];
			Code = K052109Ram[TileIndex + 0x2000] + (K052109Ram[TileIndex + 0x4000] << 8);
			
			if (nLayer == 1) {
				Colour = K052109Ram[TileIndex + 0x0800];
				Code = K052109Ram[TileIndex + 0x2800] + (K052109Ram[TileIndex + 0x4800] << 8);
			}
			
			if (nLayer == 2) {
				Colour = K052109Ram[TileIndex + 0x1000];
				Code = K052109Ram[TileIndex + 0x3000] + (K052109Ram[TileIndex + 0x5000] << 8);
			}
			
			Bank = K052109CharRomBank[(Colour & 0x0c) >> 2];
			if (has_extra_video_ram) Bank = (Colour & 0x0c) >> 2;	/* kludge for X-Men */
		
			Colour = (Colour & 0xf3) | ((Bank & 0x03) << 2);
			Bank >>= 2;
			
			yFlip = Colour & 0x02;

			int Prio = 0;

			K052109Callback(nLayer, Bank, &Code, &Colour, &xFlip, &Prio);

			if (Prio != Priority) continue;

			if (xFlip && !(K052109FlipEnable & 1)) xFlip = 0;
			if (yFlip && !(K052109FlipEnable & 2)) yFlip = 0;

			x = 8 * mx;
			y = 8 * my;

			int scrollx = K052109ScrollX[nLayer] + K052109ScrollXOff[nLayer];
			int scrolly = K052109ScrollY[nLayer] + K052109ScrollYOff[nLayer];

			if (K052109EnableRows[nLayer]) scrollx += K052109ScrollRows[nLayer][(((my << 3) - scrolly) &  0xff) >> 3];
			if (K052109EnableCols[nLayer]) scrolly += K052109ScrollCols[nLayer][mx];

			x -= (scrollx + 104) & 0x1ff;
			y -= (scrolly +  16) & 0xff;

			if (x < -8) x += 512;
			if (y < -8) y += 256;

			if (x >= nScreenWidth || y >= nScreenHeight) continue;

			if (Opaque) {
				if (x >= 0 && x <= (nScreenWidth - 8) && y >= 0 && y <= (nScreenHeight - 8)) {
					if (xFlip) {
						if (yFlip) {
							Render8x8Tile_FlipXY(pTransDraw, Code, x, y, Colour, 4, 0, pSrc);
						} else {
							Render8x8Tile_FlipX(pTransDraw, Code, x, y, Colour, 4, 0, pSrc);
						}
					} else {
						if (yFlip) {
							Render8x8Tile_FlipY(pTransDraw, Code, x, y, Colour, 4, 0, pSrc);
						} else {
							Render8x8Tile(pTransDraw, Code, x, y, Colour, 4, 0, pSrc);
						}
					}
				} else {
					if (xFlip) {
						if (yFlip) {
							Render8x8Tile_FlipXY_Clip(pTransDraw, Code, x, y, Colour, 4, 0, pSrc);
						} else {
							Render8x8Tile_FlipX_Clip(pTransDraw, Code, x, y, Colour, 4, 0, pSrc);
						}
					} else {
						if (yFlip) {
							Render8x8Tile_FlipY_Clip(pTransDraw, Code, x, y, Colour, 4, 0, pSrc);
						} else {
							Render8x8Tile_Clip(pTransDraw, Code, x, y, Colour, 4, 0, pSrc);
						}
					}
				}
			} else {
				if (x >= 0 && x <= (nScreenWidth - 8) && y >= 0 && y <= (nScreenHeight - 8)) {
					if (xFlip) {
						if (yFlip) {
							Render8x8Tile_Mask_FlipXY(pTransDraw, Code, x, y, Colour, 4, 0, 0, pSrc);
						} else {
							Render8x8Tile_Mask_FlipX(pTransDraw, Code, x, y, Colour, 4, 0, 0, pSrc);
						}
					} else {
						if (yFlip) {
							Render8x8Tile_Mask_FlipY(pTransDraw, Code, x, y, Colour, 4, 0, 0, pSrc);
						} else {
							Render8x8Tile_Mask(pTransDraw, Code, x, y, Colour, 4, 0, 0, pSrc);
						}
					}
				} else {
					if (xFlip) {
						if (yFlip) {
							Render8x8Tile_Mask_FlipXY_Clip(pTransDraw, Code, x, y, Colour, 4, 0, 0, pSrc);
						} else {
							Render8x8Tile_Mask_FlipX_Clip(pTransDraw, Code, x, y, Colour, 4, 0, 0, pSrc);
						}
					} else {
						if (yFlip) {
							Render8x8Tile_Mask_FlipY_Clip(pTransDraw, Code, x, y, Colour, 4, 0, 0, pSrc);
						} else {
							Render8x8Tile_Mask_Clip(pTransDraw, Code, x, y, Colour, 4, 0, 0, pSrc);
						}
					}
				}
			}
		}
	}
}

unsigned char K052109Read(unsigned int Offset)
{
	if (Offset > 0x5fff) return 0;

	if (K052109RMRDLine) {
		int Flags = 0;
		int Code = (Offset & 0x1fff) >> 5;
		int Colour = K052109RomSubBank;
		int Bank  =  K052109CharRomBank[(Colour & 0x0c) >> 2] >> 2;
		    Bank |= (K052109CharRomBank2[(Colour & 0x0c) >> 2] >> 2);

		if (has_extra_video_ram)
			Code |= Colour << 8;	/* kludge for X-Men */
		else
			K052109Callback(0, Bank, &Code, &Colour, &Flags, &Flags /*actually priority*/);

		int Addr = (Code << 5) + (Offset & 0x1f);
		Addr &= K052109RomMask;

		return K052109Rom[Addr];
	}

	return K052109Ram[Offset];
}

void K052109Write(unsigned int Offset, unsigned char Data)
{
	if (Offset > 0x5fff) return;

	K052109Ram[Offset] = Data;

	if (Offset >= 0x4000) has_extra_video_ram = 1;  /* kludge for X-Men */

	if ((Offset & 0x1fff) >= 0x1800) {
		switch (Offset) {
			case 0x1c80: {
				K052109ScrollCtrl = Data;
				return;
			}

			case 0x1d00: {
				K052109_irq_enabled = Data & 0x04;
				return;
			}

			case 0x1d80: {
				K052109CharRomBank[0] = Data & 0x0f;
				K052109CharRomBank[1] = (Data >> 4) & 0x0f;
				return;
			}
			
			case 0x1e00: // Normal..
			case 0x3e00: // Suprise Attack
			{
				K052109RomSubBank = Data;
				return;
			}
			
			case 0x1e80: {
				// flip
				K052109FlipEnable = ((Data & 0x06) >> 1);
				return;
			}
			
			case 0x1f00: {
				K052109CharRomBank[2] = Data & 0x0f;
				K052109CharRomBank[3] = (Data >> 4) & 0x0f;
				return;
			}

			case 0x3d80: // Surprise Attack (rom test)
			{
//				K052109CharRomBank2[0];
//				K052109CharRomBank2[1];
				return;
			}

			case 0x3f00: // Surprise Attack (rom test)
			{
//				K052109CharRomBank2[2];
//				K052109CharRomBank2[3];
				return;
			}

			case 0x180c:
			case 0x180d:
			case 0x1a00:
			case 0x1a01:
			case 0x380c:
			case 0x380d:
			case 0x3a00:
			case 0x3a01: {
				// Scroll Writes
				return;
			}
			
			case 0x1c00: {
				//???
				return;
			}
		}
	}
}

void K052109SetCallback(void (*Callback)(int Layer, int Bank, int *Code, int *Colour, int *xFlip, int *Priority))
{
	K052109Callback = Callback;
}

void K052109Reset()
{
	memset(K052109ScrollX, 0, 3 * sizeof(int));
	memset(K052109ScrollY, 0, 3 * sizeof(int));
	K052109ScrollCtrl = 0;
	memset(K052109CharRomBank, 0, 4);
	memset(K052109CharRomBank2,0, 4);
	K052109RMRDLine = 0;
	K052109RomSubBank = 0;
	K052109_irq_enabled = 0;
	memset (K052109Ram, 0, 0x6000);

	memset (K052109EnableRows, 0, 3 * sizeof(int));
	memset (K052109EnableLine, 0, 3 * sizeof(int));
	memset (K052109ScrollRows, 0, 256 * 3 * sizeof(int));
	memset (K052109EnableCols, 0, 3 * sizeof(int));
	memset (K052109ScrollCols, 0, 64 * 3 * sizeof(int));
}

void K052109Init(unsigned char *pRomSrc, unsigned int RomMask)
{
	K052109Ram = (unsigned char*)malloc(0x6000);
	
	K052109RomMask = RomMask;
	
	K052109Rom = pRomSrc;
	
	KonamiIC_K052109InUse = 1;

	for (int i = 0; i < 3; i++) {
		K052109ScrollXOff[i]=0;
		K052109ScrollYOff[i]=0;
	}

	has_extra_video_ram = 0;
}

void K052109Exit()
{
	free(K052109Ram);
	K052109Ram = NULL;
	
	K052109Callback = NULL;
	K052109RomMask = 0;
	K052109Rom = NULL;
	
	memset(K052109ScrollX, 0, 3);
	memset(K052109ScrollY, 0, 3);
	K052109ScrollCtrl = 0;
	memset(K052109CharRomBank, 0, 4);
	K052109RMRDLine = 0;
	K052109RomSubBank = 0;
	K052109FlipEnable = 0;
	K052109_irq_enabled = 0;

	has_extra_video_ram = 0;
}

void K052109Scan(int nAction)
{
	struct BurnArea ba;
	
	if (nAction & ACB_MEMORY_RAM) {
		memset(&ba, 0, sizeof(ba));
		ba.Data	  = K052109Ram;
		ba.nLen	  = 0x6000;
		ba.szName = "K052109 Ram";
		BurnAcb(&ba);
	}

	if (nAction & ACB_DRIVER_DATA) {
		SCAN_VAR(K052109ScrollX[0]);
		SCAN_VAR(K052109ScrollX[1]);
		SCAN_VAR(K052109ScrollX[2]);
		SCAN_VAR(K052109ScrollY[0]);
		SCAN_VAR(K052109ScrollY[1]);
		SCAN_VAR(K052109ScrollY[2]);
		SCAN_VAR(K052109ScrollCtrl);
		SCAN_VAR(K052109ScrollCtrl);
		SCAN_VAR(K052109CharRomBank[0]);
		SCAN_VAR(K052109CharRomBank[1]);
		SCAN_VAR(K052109CharRomBank[2]);
		SCAN_VAR(K052109CharRomBank[3]);
		SCAN_VAR(K052109CharRomBank2[0]);
		SCAN_VAR(K052109CharRomBank2[1]);
		SCAN_VAR(K052109CharRomBank2[2]);
		SCAN_VAR(K052109CharRomBank2[3]);
		SCAN_VAR(K052109RMRDLine);
		SCAN_VAR(K052109RomSubBank);
		SCAN_VAR(K052109FlipEnable);
		SCAN_VAR(K052109_irq_enabled);
		SCAN_VAR(has_extra_video_ram);
	}
}
