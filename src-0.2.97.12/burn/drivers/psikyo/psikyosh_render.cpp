// Video rendering module for Psikyo SH2 games
// Lots of code here and there ripped directly from MAME
// Thanks to David Haywood for the initial MAME driver
// as well as some other valuable pointers.

#include "tiles_generic.h" // nScreenWidth & nScreenHeight
#include "psikyosh_render.h" // contains loads of macros

unsigned char *pPsikyoshTiles;
unsigned int  *pPsikyoshSpriteBuffer;
unsigned int  *pPsikyoshBgRAM;
unsigned int  *pPsikyoshVidRegs;
unsigned int  *pPsikyoshPalRAM;
unsigned int  *pPsikyoshZoomRAM;

static unsigned char *DrvTransTab;
static unsigned char alphatable[0x100];

static unsigned short *DrvPriBmp;
static unsigned char *DrvZoomBmp;
static int nDrvZoomPrev = -1;
static unsigned int  *DrvTmpDraw;
static unsigned int  *DrvTmpDraw_ptr;

static int nGraphicsMin0;  // minimum tile number 4bpp
static int nGraphicsMin1;  // for 8bpp
static int nGraphicsSize;  // normal
static int nGraphicsSize0; // for 4bpp
static int nGraphicsSize1; // for 8bpp

//--------------------------------------------------------------------------------

static inline unsigned int alpha_blend(unsigned int d, unsigned int s, unsigned int p)
{
	if (p == 0) return d;

	int a = 256 - p;

	return (((((s & 0xff00ff) * p) + ((d & 0xff00ff) * a)) & 0xff00ff00) |
		((((s & 0x00ff00) * p) + ((d & 0x00ff00) * a)) & 0x00ff0000)) >> 8;
}

//--------------------------------------------------------------------------------

static void draw_blendy_tile(int gfx, int code, int color, int sx, int sy, int fx, int fy, int alpha, int z)
{
	color <<= 4;
	unsigned int *pal = pBurnDrvPalette + color;

	if (gfx == 0) {
		code &= 0x7ffff;
		code -= nGraphicsMin0;
		if (code < 0 || code > nGraphicsSize0) return;

		if (DrvTransTab[code >> 3] & (1 << (code & 7))) return;

		unsigned char *src = pPsikyoshTiles + (code << 7);
	
		int inc = 8;
		if (fy) {
			inc = -8;
			src += 0x78;
		}

		if (sx >= 0 && sx < (nScreenWidth-15) && sy >= 0 && sy <= (nScreenHeight-15)) {
			if (z > 0) {
				if (fx) {
					if (alpha == 0xff) {
						PUTPIXEL_4BPP_NORMAL_PRIO_FLIPX()
					} else if (alpha >= 0) {
						PUTPIXEL_4BPP_ALPHA_PRIO_FLIPX()
					} else {
						PUTPIXEL_4BPP_ALPHATAB_PRIO_FLIPX()
					}
				} else {
					if (alpha == 0xff) {
						PUTPIXEL_4BPP_NORMAL_PRIO()
					} else if (alpha >= 0) {
						PUTPIXEL_4BPP_ALPHA_PRIO()
					} else {
						PUTPIXEL_4BPP_ALPHATAB_PRIO()
					}
				}
			} else {
				if (fx) {
					if (alpha == 0xff) {
						PUTPIXEL_4BPP_NORMAL_FLIPX()
					} else if (alpha >= 0) {
						PUTPIXEL_4BPP_ALPHA_FLIPX()
					} else {
						PUTPIXEL_4BPP_ALPHATAB_FLIPX()
					}
				} else {
					if (alpha == 0xff) {
						PUTPIXEL_4BPP_NORMAL()
					} else if (alpha >= 0) {
						PUTPIXEL_4BPP_ALPHA()
					} else {
						PUTPIXEL_4BPP_ALPHATAB()
					}
				}
			}
		} else {
			if (z > 0) {
				if (fx) {
					if (alpha == 0xff) {
						PUTPIXEL_4BPP_NORMAL_PRIO_FLIPX_CLIP()
					} else if (alpha >= 0) {
						PUTPIXEL_4BPP_ALPHA_PRIO_FLIPX_CLIP()
					} else {
						PUTPIXEL_4BPP_ALPHATAB_PRIO_FLIPX_CLIP()
					}
				} else {
					if (alpha == 0xff) {
						PUTPIXEL_4BPP_NORMAL_PRIO_CLIP()
					} else if (alpha >= 0) {
						PUTPIXEL_4BPP_ALPHA_PRIO_CLIP()
					} else {
						PUTPIXEL_4BPP_ALPHATAB_PRIO_CLIP()
					}
				}
			} else {
				if (fx) {
					if (alpha == 0xff) {
						PUTPIXEL_4BPP_NORMAL_FLIPX_CLIP()
					} else if (alpha >= 0) {
						PUTPIXEL_4BPP_ALPHA_FLIPX_CLIP()
					} else {
						PUTPIXEL_4BPP_ALPHATAB_FLIPX_CLIP()
					}
				} else {
					if (alpha == 0xff) {
						PUTPIXEL_4BPP_NORMAL_CLIP()
					} else if (alpha >= 0) {
						PUTPIXEL_4BPP_ALPHA_CLIP()
					} else {
						PUTPIXEL_4BPP_ALPHATAB_CLIP()
					}
				}
			}
		}
	} else {
		code &= 0x3ffff;
		code -= nGraphicsMin1;
		if (code < 0 || code > nGraphicsSize0) return;

		if (DrvTransTab[(code >> 3) + 0x10000] & (1 << (code & 7))) return;

		unsigned char *src = pPsikyoshTiles + (code << 8);

		int inc = 16;
		if (fy) {
			inc = -16;
			src += 0xf0;
		}

		if (sx >= 0 && sx < (nScreenWidth-15) && sy >= 0 && sy < (nScreenHeight-15)) {
			if (z > 0) {
				if (fx) {
					if (alpha == 0xff) {
						PUTPIXEL_8BPP_NORMAL_PRIO_FLIPX()
					} else if (alpha >= 0) {
						PUTPIXEL_8BPP_ALPHA_PRIO_FLIPX()
					} else {
						PUTPIXEL_8BPP_ALPHATAB_PRIO_FLIPX()
					}
				} else {
					if (alpha == 0xff) {
						PUTPIXEL_8BPP_NORMAL_PRIO()
					} else if (alpha >= 0) {
						PUTPIXEL_8BPP_ALPHA_PRIO()
					} else {
						PUTPIXEL_8BPP_ALPHATAB_PRIO()
					}
				}
			} else {
				if (fx) {
					if (alpha == 0xff) {
						PUTPIXEL_8BPP_NORMAL_FLIPX()
					} else if (alpha >= 0) {
						PUTPIXEL_8BPP_ALPHA_FLIPX()
					} else {
						PUTPIXEL_8BPP_ALPHATAB_FLIPX()
					}
				} else {
					if (alpha == 0xff) {
						PUTPIXEL_8BPP_NORMAL()
					} else if (alpha >= 0) {
						PUTPIXEL_8BPP_ALPHA()
					} else {
						PUTPIXEL_8BPP_ALPHATAB()
					}
				}
			}
		} else {
			if (z > 0) {
				if (fx) {
					if (alpha == 0xff) {
						PUTPIXEL_8BPP_NORMAL_PRIO_FLIPX_CLIP()
					} else if (alpha >= 0) {
						PUTPIXEL_8BPP_ALPHA_PRIO_FLIPX_CLIP()
					} else {
						PUTPIXEL_8BPP_ALPHATAB_PRIO_FLIPX_CLIP()
					}
				} else {
					if (alpha == 0xff) {
						PUTPIXEL_8BPP_NORMAL_PRIO_CLIP()
					} else if (alpha >= 0) {
						PUTPIXEL_8BPP_ALPHA_PRIO_CLIP()
					} else {
						PUTPIXEL_8BPP_ALPHATAB_PRIO_CLIP()
					}
				}
			} else {
				if (fx) {
					if (alpha == 0xff) {
						PUTPIXEL_8BPP_NORMAL_FLIPX_CLIP()
					} else if (alpha >= 0) {
						PUTPIXEL_8BPP_ALPHA_FLIPX_CLIP()
					} else {
						PUTPIXEL_8BPP_ALPHATAB_FLIPX_CLIP()
					}
				} else {
					if (alpha == 0xff) {
						PUTPIXEL_8BPP_NORMAL_CLIP()
					} else if (alpha >= 0) {
						PUTPIXEL_8BPP_ALPHA_CLIP()
					} else {
						PUTPIXEL_8BPP_ALPHATAB_CLIP()
					}
				}
			}

		}
	}
}

static void draw_prezoom(int gfx, int code, int high, int wide)
{
	// these probably aren't the safest routines, but they should be pretty fast.

	if (gfx) {
		int tileno = (code & 0x3ffff) - nGraphicsMin1;
		if (tileno < 0 || tileno > nGraphicsSize1) tileno = 0;
		if (nDrvZoomPrev == tileno) return;
		nDrvZoomPrev = tileno;
		unsigned int *gfxptr = (unsigned int*)(pPsikyoshTiles + (tileno << 8));

		for (int ytile = 0; ytile < high; ytile++)
		{
			for (int xtile = 0; xtile < wide; xtile++)
			{
				unsigned int *dest = (unsigned int*)(DrvZoomBmp + (ytile << 12) + (xtile << 4));

				for (int ypixel = 0; ypixel < 16; ypixel++, gfxptr += 4) {

					dest[0] = gfxptr[0];
					dest[1] = gfxptr[1];
					dest[2] = gfxptr[2];
					dest[3] = gfxptr[3];

					dest += 64;
				}
			}
		}
	} else {
		int tileno = (code & 0x7ffff) - nGraphicsMin0;
		if (tileno < 0 || tileno > nGraphicsSize0) tileno = 0;
		if (nDrvZoomPrev == tileno) return;
		nDrvZoomPrev = tileno;
		unsigned char *gfxptr = pPsikyoshTiles + (tileno << 7);
		for (int ytile = 0; ytile < high; ytile++)
		{
			for (int xtile = 0; xtile < wide; xtile++)
			{
				unsigned char *dest = DrvZoomBmp + (ytile << 12) + (xtile << 4);

				for (int ypixel = 0; ypixel < 16; ypixel++, gfxptr += 8)
				{
					for (int xpixel = 0; xpixel < 16; xpixel+=2)
					{
						int c = gfxptr[xpixel>>1];
						dest[xpixel    ] = c >> 4;
						dest[xpixel + 1] = c & 0x0f;
					}

					dest += 256;
				}
			}
		}
	}
}

static void psikyosh_drawgfxzoom(int gfx, unsigned int code, int color, int flipx, int flipy, int offsx, 
				 int offsy, int alpha, int zoomx, int zoomy, int wide, int high, int z)
{
	if (~nBurnLayer & 8) return;
	if (!zoomx || !zoomy) return;

	if (zoomx == 0x400 && zoomy == 0x400)
	{
		int xstart, ystart, xend, yend, xinc, yinc, code_offset = 0;

		if (flipx)	{ xstart = wide-1; xend = -1;   xinc = -1; }
		else		{ xstart = 0;      xend = wide; xinc = +1; }

		if (flipy)	{ ystart = high-1; yend = -1;   yinc = -1; }
		else		{ ystart = 0;      yend = high; yinc = +1; }

		for (int ytile = ystart; ytile != yend; ytile += yinc )
		{
			for (int xtile = xstart; xtile != xend; xtile += xinc )
			{
				int sx = offsx + (xtile << 4);
				int sy = offsy + (ytile << 4);

				draw_blendy_tile(gfx, code + code_offset++, color, sx, sy, flipx, flipy, alpha, z);
			}
		}
	}
	else
	{
		draw_prezoom(gfx, code, high, wide);

		{
			unsigned int *pal = pBurnDrvPalette + (color << 4);

			int sprite_screen_height = ((high << 24) / zoomy + 0x200) >> 10;
			int sprite_screen_width  = ((wide << 24) / zoomx + 0x200) >> 10;

			if (sprite_screen_width && sprite_screen_height)
			{
				int sx = offsx;
				int sy = offsy;
				int ex = sx + sprite_screen_width;
				int ey = sy + sprite_screen_height;

				int x_index_base;
				int y_index;

				int dx, dy;

				if (flipx) { x_index_base = (sprite_screen_width-1)*zoomx; dx = -zoomx; }
				else	   { x_index_base = 0; dx = zoomx; }

				if (flipy) { y_index = (sprite_screen_height-1)*zoomy; dy = -zoomy; }
				else	   { y_index = 0; dy = zoomy; }

				{
					if (sx < 0) {
						int pixels = 0-sx;
						sx += pixels;
						x_index_base += pixels*dx;
					}
					if (sy < 0 ) {
						int pixels = 0-sy;
						sy += pixels;
						y_index += pixels*dy;
					}
					if (ex > nScreenWidth) {
						int pixels = ex-(nScreenWidth-1)-1;
						ex -= pixels;
					}
					if (ey > nScreenHeight)	{
						int pixels = ey-(nScreenHeight-1)-1;
						ey -= pixels;
					}
				}

				if (ex > sx)
				{
					if (alpha == 0xff) {
						if (z > 0) {
							PUTPIXEL_ZOOM_NORMAL_PRIO()
						} else {
							PUTPIXEL_ZOOM_NORMAL()
						}
					} else if (alpha >= 0) {
						if (z > 0) {
							PUTPIXEL_ZOOM_ALPHA_PRIO()
						} else {
							PUTPIXEL_ZOOM_ALPHA()
						}
					} else {
						if (z > 0) {
							PUTPIXEL_ZOOM_ALPHATAB_PRIO()
						} else {
							PUTPIXEL_ZOOM_ALPHATAB()
						}
					}
				}
			}
		}
	}
}

static void draw_sprites(unsigned char req_pri)
{
	unsigned int   *src = pPsikyoshSpriteBuffer;
	unsigned short *list = (unsigned short *)src + 0x3800/2;
	unsigned short listlen = 0x800/2;
	unsigned short listcntr = 0;
	unsigned short *zoom_table = (unsigned short *)pPsikyoshZoomRAM;
	unsigned char  *alpha_table = (unsigned char *)pPsikyoshVidRegs;

	while (listcntr < listlen)
	{
		unsigned int xpos, ypos, high, wide, flpx, flpy, zoomx, zoomy, tnum, colr, dpth, pri;
		int alpha;

		unsigned int listdat = list[listcntr ^ 1];
		unsigned int sprnum = (listdat & 0x03ff) << 2;

		pri = (src[sprnum+1] & 0x00003000) >> 12;
		pri = (pPsikyoshVidRegs[2] << (pri << 2)) >> 28;

		if (pri == req_pri)
		{
			ypos  = (src[sprnum+0] & 0x03ff0000) >> 16;
			xpos  = (src[sprnum+0] & 0x000003ff);
			high  =((src[sprnum+1] & 0x0f000000) >> 24) + 1;
			wide  =((src[sprnum+1] & 0x00000f00) >>  8) + 1;
			flpy  = (src[sprnum+1] & 0x80000000) >> 31;
			flpx  = (src[sprnum+1] & 0x00008000) >> 15;
			zoomy = (src[sprnum+1] & 0x00ff0000) >> 16;
			zoomx = (src[sprnum+1] & 0x000000ff);
			tnum  = (src[sprnum+2] & 0x0007ffff);
			dpth  = (src[sprnum+2] & 0x00800000) >> 23;
			colr  = (src[sprnum+2] & 0xff000000) >> 24;
			alpha = (src[sprnum+2] & 0x00700000) >> 20;

			if (ypos & 0x200) ypos -= 0x400;
			if (xpos & 0x200) xpos -= 0x400;

			alpha = alpha_table[alpha ^ 3];

			if (alpha & 0x80) {
				alpha = -1;
			} else {
				alpha = alphatable[alpha | 0xc0];
			}

			if (zoom_table[zoomy ^ 1] && zoom_table[zoomx ^ 1])
			{
				psikyosh_drawgfxzoom(dpth, tnum, colr, flpx, flpy, xpos, ypos, alpha, 
					(unsigned int)zoom_table[zoomx ^ 1],(unsigned int)zoom_table[zoomy ^ 1], wide, high, listcntr);
			}
		}

		listcntr++;
		if (listdat & 0x4000) break;
	}
}

static void draw_layer(int layer, int bank, int alpha, int scrollx, int scrolly)
{
	if ((bank < 0x0c) || (bank > 0x1f)) return;

	if (alpha & 0x80) {
		alpha = -1;
	} else {
		alpha = alphatable[alpha | 0xc0];
	}

	int attr = pPsikyoshVidRegs[7] << (layer << 2);
	int gfx  = attr & 0x00004000;
	int size =(attr & 0x00001000) ? 32 : 16;
	int wide = size * 16;

	for (int offs = 0; offs < size * 32; offs++) {
		int sx = (offs & 0x1f) << 4;
		int sy = (offs >> 5) << 4;

		sx = (sx + scrollx) & 0x1ff;
		sy = (sy + scrolly) & (wide-1);
		if (sx >= nScreenWidth)  sx -= 0x200;
		if (sy >= nScreenHeight) sy -= wide;
		if (sx < -15 || sy < -15) continue;

		unsigned int code  = pPsikyoshBgRAM[(bank*0x800)/4 + offs - 0x4000/4];

		draw_blendy_tile(gfx, code & 0x7ffff, (code >> 24), sx, sy, 0, 0, alpha, 0);
	}
}

static void draw_bglayer(int layer)
{
	if (!(nBurnLayer & 1)) return;

	int scrollx, scrolly, bank, alpha;
	int scrollbank = ((pPsikyoshVidRegs[6] << (layer << 3)) >> 24) & 0x7f;
	int offset = (scrollbank == 0x0b) ? 0x200 : 0;

	bank    = (pPsikyoshBgRAM[0x17f0/4 + offset + layer] & 0x000000ff);
	alpha   = (pPsikyoshBgRAM[0x17f0/4 + offset + layer] & 0x0000bf00) >> 8;
	scrollx = (pPsikyoshBgRAM[0x13f0/4 + offset + layer] & 0x000001ff);
	scrolly = (pPsikyoshBgRAM[0x13f0/4 + offset + layer] & 0x03ff0000) >> 16;

	if (scrollbank == 0x0d) scrollx += 0x08;

	draw_layer(layer, bank, alpha, scrollx, scrolly);
}

static void draw_bglayertext(int layer)
{
	if (~nBurnLayer & 2) return;

	int scrollx, scrolly, bank, alpha;
	int scrollbank = ((pPsikyoshVidRegs[6] << (layer << 3)) >> 24) & 0x7f;

	bank    = (pPsikyoshBgRAM[(scrollbank*0x800)/4 + 0x0400/4 - 0x4000/4] & 0x000000ff);
	alpha   = (pPsikyoshBgRAM[(scrollbank*0x800)/4 + 0x0400/4 - 0x4000/4] & 0x0000bf00) >> 8;
	scrollx = (pPsikyoshBgRAM[(scrollbank*0x800)/4 - 0x4000/4           ] & 0x000001ff);
	scrolly = (pPsikyoshBgRAM[(scrollbank*0x800)/4 - 0x4000/4           ] & 0x03ff0000) >> 16;

	draw_layer(layer, bank, alpha, scrollx, scrolly);

	bank    = (pPsikyoshBgRAM[(scrollbank*0x800)/4 + 0x0400/4 + 0x20/4 - 0x4000/4] & 0x000000ff);
	alpha   = (pPsikyoshBgRAM[(scrollbank*0x800)/4 + 0x0400/4 + 0x20/4 - 0x4000/4] & 0x0000bf00) >> 8;
	scrollx = (pPsikyoshBgRAM[(scrollbank*0x800)/4 - 0x4000/4 + 0x20/4           ] & 0x000001ff);
	scrolly = (pPsikyoshBgRAM[(scrollbank*0x800)/4 - 0x4000/4 + 0x20/4           ] & 0x03ff0000) >> 16;

	draw_layer(layer, bank, alpha, scrollx, scrolly);
}

static void draw_bglayerscroll(int layer)
{
	if (!(nBurnLayer & 4)) return;

	int scrollx, bank, alpha;
	int scrollbank = ((pPsikyoshVidRegs[6] << (layer << 3)) >> 24) & 0x7f;

	bank    = (pPsikyoshBgRAM[(scrollbank*0x800)/4 + 0x0400/4 - 0x4000/4] & 0x000000ff);
	alpha   = (pPsikyoshBgRAM[(scrollbank*0x800)/4 + 0x0400/4 - 0x4000/4] & 0x0000bf00) >> 8;
	scrollx = (pPsikyoshBgRAM[(scrollbank*0x800)/4 - 0x4000/4           ] & 0x000001ff);
//	scrolly = (pPsikyoshBgRAM[(scrollbank*0x800)/4 - 0x4000/4           ] & 0x03ff0000) >> 16;

	draw_layer(layer, bank, alpha, scrollx, 0);
}

static void draw_background(unsigned char req_pri)
{
	for (int i = 0; i < 3; i++)
	{
		if (!((pPsikyoshVidRegs[7] << (i << 2)) & 0x8000))
			continue;

		int bgtype = ((pPsikyoshVidRegs[6] << (i << 3)) >> 24) & 0x7f;

		switch (bgtype)
		{
			case 0x0a: // Normal
				if((pPsikyoshBgRAM[0x17f0/4 + (i*0x04)/4] >> 24) == req_pri)
					draw_bglayer(i);
				break;

			case 0x0b: // Alt / Normal
				if((pPsikyoshBgRAM[0x1ff0/4 + (i*0x04)/4] >> 24) == req_pri)
					draw_bglayer(i);
				break;

			case 0x0c: // Using normal for now
			case 0x0d: // Using normal for now
				if((pPsikyoshBgRAM[(bgtype*0x800)/4 + 0x400/4 - 0x4000/4] >> 24) == req_pri)
					draw_bglayertext(i);
				break;

			case 0x0e:
			case 0x10: case 0x11: case 0x12: case 0x13:
			case 0x14: case 0x15: case 0x16: case 0x17:
			case 0x18: case 0x19: case 0x1a: case 0x1b:
			case 0x1c: case 0x1d: case 0x1e: case 0x1f:
				if((pPsikyoshBgRAM[(bgtype*0x800)/4 + 0x400/4 - 0x4000/4] >> 24) == req_pri)
					draw_bglayerscroll(i);
				break;
		}
	}
}

static void prelineblend()
{
	unsigned int *linefill = pPsikyoshBgRAM;
	unsigned int *destline = DrvTmpDraw;

	for (int y = 0; y < nScreenHeight; y++, destline+=nScreenWidth) {
		if (linefill[y] & 0xff) {
			for (int x = 0; x < nScreenWidth; x++) {
				destline[x] = linefill[y] >> 8;
			}
		}
	}
}

static void postlineblend()
{
	unsigned int *lineblend = pPsikyoshBgRAM + 0x0400/4;
	unsigned int *destline = DrvTmpDraw;

	for (int y = 0; y < nScreenHeight; y++, destline+=nScreenWidth) {
		if (lineblend[y] & 0x80) {
			for (int x = 0; x < nScreenWidth; x++) {
				destline[x] = lineblend[y] >> 8;
			}
		}
		else if (lineblend[y] & 0x7f) {
			for (int x = 0; x < nScreenWidth; x++) {
				destline[x] = alpha_blend(destline[x], lineblend[y] >> 8, (lineblend[y] & 0x7f) << 1);
			}
		}
	}
}

int PsikyoshDraw()
{
	{
		for (int i = 0; i < 0x5000 / 4; i++) {
			pBurnDrvPalette[i] = pPsikyoshPalRAM[i] >> 8;
		}
	}

	if (nBurnBpp == 4) {
		DrvTmpDraw = (unsigned int*)pBurnDraw;
	} else {
		DrvTmpDraw = DrvTmpDraw_ptr;
	}

	memset (DrvTmpDraw, 0, nScreenWidth * nScreenHeight * sizeof(int));
	memset (DrvPriBmp, 0, nScreenWidth * nScreenHeight * sizeof(short));

	unsigned int *psikyosh_vidregs = pPsikyoshVidRegs;

	prelineblend();

	for (unsigned int i = 0; i < 8; i++) {
		draw_sprites(i);
		draw_background(i);
		if ((psikyosh_vidregs[2] & 0x0f) == i) postlineblend();
	}

	if (nBurnBpp < 4) {
		for (int i = 0; i < nScreenWidth * nScreenHeight; i++) {
			int d = DrvTmpDraw[i];
			PutPix(pBurnDraw + i * nBurnBpp, BurnHighCol(d>>16, d>>8, d, 0));
		}
	}

	return 0;
}

static void fill_alphatable()
{
	for (int i = 0; i < 0xc0; i++)
		alphatable[i] = 0xff;

	for (int i = 0; i < 0x40; i++) {
		alphatable[i | 0xc0] = ((0x3f - i) * 0xff) / 0x3f;
	}
}

static void calculate_transtab()
{
	DrvTransTab = (unsigned char*)malloc(0x18000);

	memset (DrvTransTab, 0xff, 0x18000);

	// first calculate all 4bpp tiles
	for (int i = 0; i < nGraphicsSize; i+= 0x80) {
		for (int j = 0; j < 0x80; j++) {
			if (pPsikyoshTiles[i + j]) {
				DrvTransTab[(i>>10) + 0x00000] &= ~(1 << ((i >> 7) & 7));
				break;
			}
		}
	}

	// next, calculate all 8bpp tiles
	for (int i = 0; i < nGraphicsSize; i+= 0x100) {
		for (int j = 0; j < 0x100; j++) {
			if (pPsikyoshTiles[i + j]) {
				DrvTransTab[(i>>11) + 0x10000] &= ~(1 << ((i >> 8) & 7));
				break;
			}
		}
	}
}

void PsikyoshVideoInit(int gfx_max, int gfx_min)
{
	DrvZoomBmp	= (unsigned char *)malloc(16 * 16 * 16 * 16);
	DrvPriBmp	= (unsigned short*)malloc(320 * 240 * sizeof(short));
	DrvTmpDraw_ptr	= (unsigned int  *)malloc(320 * 240 * sizeof(int));

	if (BurnDrvGetFlags() & BDF_ORIENTATION_VERTICAL) {
		BurnDrvGetVisibleSize(&nScreenHeight, &nScreenWidth);
	} else {
		BurnDrvGetVisibleSize(&nScreenWidth, &nScreenHeight);
	}

	nGraphicsSize  = gfx_max - gfx_min;
	nGraphicsMin0  = (gfx_min / 128);
	nGraphicsMin1  = (gfx_min / 256);
	nGraphicsSize0 = (nGraphicsSize / 128) - 1;
	nGraphicsSize1 = (nGraphicsSize / 256) - 1;

	calculate_transtab();
	fill_alphatable();
}

void PsikyoshVideoExit()
{
	if (DrvZoomBmp) {
		free (DrvZoomBmp);
	}
	DrvZoomBmp = NULL;

	if (DrvPriBmp) {
		free (DrvPriBmp);
	}
	DrvPriBmp = NULL;

	if (DrvTmpDraw_ptr) {
		free (DrvTmpDraw_ptr);
	}
	DrvTmpDraw_ptr = NULL;
	DrvTmpDraw = NULL;

	if (DrvTransTab) {
		free (DrvTransTab);
	}
	DrvTransTab = NULL;

	nDrvZoomPrev		= -1;
	pPsikyoshTiles		= NULL;
	pPsikyoshSpriteBuffer	= NULL;
	pPsikyoshBgRAM		= NULL;
	pPsikyoshVidRegs	= NULL;
	pPsikyoshPalRAM		= NULL;
	pPsikyoshZoomRAM	= NULL;
	pBurnDrvPalette		= NULL;

	nScreenWidth = nScreenHeight = 0;
}
