#include "pgm.h"

static int nTileMask = 0;
static unsigned char sprmsktab[0x100];
static unsigned char  *SpritePrio;	// sprite priorities
static unsigned short *pTempScreen;	// sprites
static unsigned short *pTempDraw;	// pre-zoomed sprites
static unsigned char  *tiletrans;	// tile transparency table
static unsigned char  *texttrans;	// text transparency table

#define draw_pixel_nomask(n)				\
	dest[xoff + n] = adata[aoffset] | palt;		\
	pdest[xoff + n] = prio;				\
	aoffset++					\

#define draw_pixel(n, mskno)				\
	if (msk & mskno) {				\
		draw_pixel_nomask(n);			\
	}						\

inline static unsigned int CalcCol(unsigned short nColour)
{
	int r, g, b;

	r = (nColour & 0x7C00) >> 7;  // Red 
	r |= r >> 5;
	g = (nColour & 0x03E0) >> 2;	// Green
	g |= g >> 5;
	b = (nColour & 0x001F) << 3;	// Blue
	b |= b >> 5;

	return BurnHighCol(r, g, b, 0);
}

static void pgm_prepare_sprite(int wide, int high,int palt, int boffset)
{
	unsigned char * bdata = PGMSPRMaskROM;
	unsigned char * adata = PGMSPRColROM;
	unsigned short* dest = pTempDraw;
	int bdatasize = nPGMSPRMaskMaskLen;
	int adatasize = nPGMSPRColMaskLen;

	wide *= 16;
	palt *= 32;

	unsigned int aoffset = (bdata[(boffset+3) & bdatasize] << 24) | (bdata[(boffset+2) & bdatasize] << 16) | (bdata[(boffset+1) & bdatasize] << 8) | (bdata[(boffset) & bdatasize]);
	aoffset = (aoffset >> 2) * 3;

	boffset += 4;

	for (int ycnt = 0 ; ycnt < high; ycnt++) {
		dest = pTempDraw + (ycnt * wide);

		for (int xcnt = 0 ; xcnt < wide; xcnt+=16) {
			unsigned short msk = (bdata[(boffset+1) & bdatasize] << 8) | bdata[boffset & bdatasize];

			if (msk == 0xffff) {
				*dest++ = 0x8000;
				*dest++ = 0x8000;
				*dest++ = 0x8000;
				*dest++ = 0x8000;
				*dest++ = 0x8000;
				*dest++ = 0x8000;
				*dest++ = 0x8000;
				*dest++ = 0x8000;
				*dest++ = 0x8000;
				*dest++ = 0x8000;
				*dest++ = 0x8000;
				*dest++ = 0x8000;
				*dest++ = 0x8000;
				*dest++ = 0x8000;
				*dest++ = 0x8000;
				*dest++ = 0x8000;
			}
			else if (msk == 0)
			{
				*dest++ = adata[aoffset++ & adatasize] | palt;
				*dest++ = adata[aoffset++ & adatasize] | palt;
				*dest++ = adata[aoffset++ & adatasize] | palt;
				*dest++ = adata[aoffset++ & adatasize] | palt;
				*dest++ = adata[aoffset++ & adatasize] | palt;
				*dest++ = adata[aoffset++ & adatasize] | palt;
				*dest++ = adata[aoffset++ & adatasize] | palt;
				*dest++ = adata[aoffset++ & adatasize] | palt;
				*dest++ = adata[aoffset++ & adatasize] | palt;
				*dest++ = adata[aoffset++ & adatasize] | palt;
				*dest++ = adata[aoffset++ & adatasize] | palt;
				*dest++ = adata[aoffset++ & adatasize] | palt;
				*dest++ = adata[aoffset++ & adatasize] | palt;
				*dest++ = adata[aoffset++ & adatasize] | palt;
				*dest++ = adata[aoffset++ & adatasize] | palt;
				*dest++ = adata[aoffset++ & adatasize] | palt;
			}
			else
			{
				for (int x = 0; x < 16; x++)
				{
					if (msk & 1)
					{
						dest[x] = 0x8000;
					}
					else
					{
						dest[x] = adata[aoffset++ & adatasize] | palt;
					}
	
					msk >>= 1;
				}

				dest += 16;
			}

			boffset+=2;
		}
	}
}

static inline void draw_sprite_line(int wide, unsigned short* dest, unsigned char *pdest, int xzoom, int xgrow, int yoffset, int flip, int xpos, int prio)
{
	int xzoombit;
	int xoffset;
	int xcnt = 0, xcntdraw = 0;
	int xdrawpos = 0;

	wide *= 16;
	flip &= 1;

	while (xcnt < wide)
	{
		if (flip) xoffset = wide - xcnt - 1;
		else	  xoffset = xcnt;

		unsigned int srcdat = pTempDraw[yoffset + xoffset];
		xzoombit = (xzoom >> (xcnt & 0x1f)) & 1;

		if (xzoombit == 1 && xgrow == 1)
		{
			xdrawpos = xpos + xcntdraw;

			if (!(srcdat & 0x8000))
			{
				if ((xdrawpos >= 0) && (xdrawpos < nScreenWidth)) {
					dest[xdrawpos] = srcdat;
					pdest[xdrawpos] = prio;
				}

				xdrawpos = xpos + xcntdraw + 1;

				if ((xdrawpos >= 0) && (xdrawpos < nScreenWidth)) {
					dest[xdrawpos] = srcdat;
					pdest[xdrawpos] = prio;
				}
			}

			xcntdraw+=2;
		}
		else if (xzoombit == 1 && xgrow == 0)
		{
			/* skip this column */
		}
		else //normal column
		{
			xdrawpos = xpos + xcntdraw;

			if (!(srcdat & 0x8000))
			{
				if ((xdrawpos >= 0) && (xdrawpos < nScreenWidth)) {
					dest[xdrawpos] = srcdat;
					pdest[xdrawpos] = prio;
				}
			}

			xcntdraw++;
		}

		xcnt++;

		if (xdrawpos == nScreenWidth) break;
	}
}

static void pgm_draw_sprite_nozoom(int wide, int high, int palt, int boffset, int xpos, int ypos, int flipx, int flipy, int prio)
{
	unsigned short *dest = pTempScreen;
	unsigned char *pdest = SpritePrio;
	unsigned char * bdata = PGMSPRMaskROM;
	unsigned char * adata = PGMSPRColROM;
	int bdatasize = nPGMSPRMaskMaskLen;
	int adatasize = nPGMSPRColMaskLen;
	int yoff, xoff;

	unsigned short msk;

	unsigned int aoffset = (bdata[(boffset+3) & bdatasize] << 24) | (bdata[(boffset+2) & bdatasize] << 16) | (bdata[(boffset+1) & bdatasize] << 8) | (bdata[boffset & bdatasize]);
	aoffset = (aoffset >> 2) * 3;
	aoffset &= adatasize;

	boffset += 4;
	wide <<= 4;

	palt <<= 5;

	for (int ycnt = 0; ycnt < high; ycnt++) {
		if (flipy) {
			yoff = ypos + ((high-1) - ycnt);
			if (yoff < 0) break;
			if (yoff < nScreenHeight) {
				dest = pTempScreen + (yoff * nScreenWidth);
				pdest = SpritePrio + (yoff * nScreenWidth);
			}
		} else {
			yoff = ypos + ycnt;
			if (yoff >= nScreenHeight) break;
			if (yoff >= 0)  {
				dest = pTempScreen + (yoff * nScreenWidth);
				pdest = SpritePrio + (yoff * nScreenWidth);
			}
		}

		if (yoff >= 0 && yoff < nScreenHeight && xpos >= 0 && (xpos + wide) < nScreenWidth)
		{
			for (int xcnt = 0; xcnt < wide; xcnt+=8)
			{
				msk = bdata[boffset & bdatasize] ^ 0xff;	
				boffset++;
				aoffset &= adatasize; // not 100% safe, but faster...

				if (msk == 0) { // transparent
					aoffset += sprmsktab[msk];
					continue;
				}

				if (flipx) {
					xoff = xpos + (wide - xcnt);
					if (msk == 0xff) { // opaque
						draw_pixel_nomask( 0);
						draw_pixel_nomask(-1);
						draw_pixel_nomask(-2);
						draw_pixel_nomask(-3);
						draw_pixel_nomask(-4);
						draw_pixel_nomask(-5);
						draw_pixel_nomask(-6);
						draw_pixel_nomask(-7);
					} else {
						draw_pixel( 0, 0x01);
						draw_pixel(-1, 0x02);
						draw_pixel(-2, 0x04);
						draw_pixel(-3, 0x08);
						draw_pixel(-4, 0x10);
						draw_pixel(-5, 0x20);
						draw_pixel(-6, 0x40);
						draw_pixel(-7, 0x80);
					}
				} else {
					xoff = xpos + xcnt;
					if (msk == 0xff) { // opaque
						draw_pixel_nomask( 0);
						draw_pixel_nomask( 1);
						draw_pixel_nomask( 2);
						draw_pixel_nomask( 3);
						draw_pixel_nomask( 4);
						draw_pixel_nomask( 5);
						draw_pixel_nomask( 6);
						draw_pixel_nomask( 7);
					} else {
						draw_pixel( 0, 0x01);
						draw_pixel( 1, 0x02);
						draw_pixel( 2, 0x04);
						draw_pixel( 3, 0x08);
						draw_pixel( 4, 0x10);
						draw_pixel( 5, 0x20);
						draw_pixel( 6, 0x40);
						draw_pixel( 7, 0x80);
					}
				}
			}
		} else {
			for (int xcnt = 0; xcnt < wide; xcnt+=8)
			{
				msk = bdata[boffset & bdatasize] ^ 0xff;
				boffset++;
				aoffset &= adatasize;

				if (yoff < 0 || yoff >= nScreenHeight || msk == 0) {
					aoffset += sprmsktab[msk];

					continue;
				}

				if (flipx) {
					xoff = xpos + (wide - xcnt);

					if (xoff < -7 || xoff >= nScreenWidth+8) {
						aoffset += sprmsktab[msk];
						continue;
					}

					for (int x = 0; x < 8; x++, xoff--)
					{
						if (msk & 0x0001)
						{
							if (xoff >= 0 && xoff < nScreenWidth) {
								dest[xoff] = adata[aoffset] | palt;
								pdest[xoff] = prio;
							}
		
							aoffset++;
						}
		
						msk >>= 1;
						if (!msk) break;
					}
				} else {
					xoff = xpos + xcnt;

					if (xoff < -7 || xoff >= nScreenWidth) {
						aoffset += sprmsktab[msk];
						continue;
					}

					for (int x = 0; x < 8; x++, xoff++)
					{
						if (msk & 0x0001)
						{
							if (xoff >= 0 && xoff < nScreenWidth) {
								dest[xoff] = adata[aoffset] | palt;
								pdest[xoff] = prio;
							}
		
							aoffset++;
						}
		
						msk >>= 1;
						if (!msk) break;
					}
				}
			}
		}
	}
}

static void draw_sprite_new_zoomed(int wide, int high, int xpos, int ypos, int palt, int boffset, int flip, unsigned int xzoom, int xgrow, unsigned int yzoom, int ygrow, int prio )
{
	if (!wide) return;

	if (yzoom == 0 && xzoom == 0) {
		pgm_draw_sprite_nozoom(wide, high, palt, boffset, xpos, ypos, flip & 1, flip & 2, prio);
		return;
	}

	int ycnt;
	int ydrawpos;
	unsigned short *dest;
	unsigned char *pdest;
	int yoffset;
	int ycntdraw;
	int yzoombit;

	pgm_prepare_sprite(wide, high, palt, boffset);

	ycnt = 0;
	ycntdraw = 0;
	while (ycnt < high)
	{
		yzoombit = (yzoom >> (ycnt&0x1f))&1;

		if (yzoombit == 1 && ygrow == 1) // double this line
		{
			ydrawpos = ypos + ycntdraw;

			if (!(flip&0x02)) yoffset = (ycnt*(wide*16));
			else yoffset = ( (high-ycnt-1)*(wide*16));
			if ((ydrawpos >= 0) && (ydrawpos < 224))
			{
				dest = pTempScreen + ydrawpos * nScreenWidth;
				pdest = SpritePrio + ydrawpos * nScreenWidth;
				draw_sprite_line(wide, dest, pdest, xzoom, xgrow, yoffset, flip, xpos, prio);
			}
			ycntdraw++;

			ydrawpos = ypos + ycntdraw;
			if (!(flip&0x02)) yoffset = (ycnt*(wide*16));
			else yoffset = ( (high-ycnt-1)*(wide*16));
			if ((ydrawpos >= 0) && (ydrawpos < 224))
			{
				dest = pTempScreen + ydrawpos * nScreenWidth;
				pdest = SpritePrio + ydrawpos * nScreenWidth;
				draw_sprite_line(wide, dest, pdest, xzoom, xgrow, yoffset, flip, xpos, prio);
			}
			ycntdraw++;

			if (ydrawpos == 224) ycnt = high;
		}
		else if (yzoombit ==1 && ygrow == 0)
		{
			/* skip this line */
			/* we should process anyway if we don't do the pre-decode.. */
		}
		else /* normal line */
		{
			ydrawpos = ypos + ycntdraw;

			if (!(flip&0x02)) yoffset = (ycnt*(wide*16));
			else yoffset = ( (high-ycnt-1)*(wide*16));
			if ((ydrawpos >= 0) && (ydrawpos < 224))
			{
				dest = pTempScreen + ydrawpos * nScreenWidth;
				pdest = SpritePrio + ydrawpos * nScreenWidth;
				draw_sprite_line(wide, dest, pdest, xzoom, xgrow, yoffset, flip, xpos, prio);
			}
			ycntdraw++;

			if (ydrawpos == 224) ycnt = high;
		}

		ycnt++;
	}
}

static void pgm_drawsprites()
{
	unsigned short *source = PGMSprBuf;
	unsigned short *finish = PGMSprBuf + 0xa00/2;
	unsigned short *zoomtable = &PGMVidReg[0x1000/2];

	while (finish > source)
	{
		if (source[4] == 0) break; // right?

		int xpos =  source[0] & 0x07ff;
		int ypos =  source[1] & 0x03ff;
		int xzom = (source[0] & 0x7800) >> 11;
		int xgrow= (source[0] & 0x8000) >> 15;
		int yzom = (source[1] & 0x7800) >> 11;
		int ygrow= (source[1] & 0x8000) >> 15;
		int palt = (source[2] & 0x1f00) >> 8;
		int flip = (source[2] & 0x6000) >> 13;
		int boff =((source[2] & 0x007f) << 16) | (source[3] & 0xffff);
		int wide = (source[4] & 0x7e00) >> 9;
		int prio = (source[2] & 0x0080) >> 7;
		int high =  source[4] & 0x01ff;

		if (xgrow) xzom = 0x10-xzom;
		if (ygrow) yzom = 0x10-yzom;

		unsigned int xzoom = (zoomtable[xzom*2]<<16)|zoomtable[xzom*2+1];
		unsigned int yzoom = (zoomtable[yzom*2]<<16)|zoomtable[yzom*2+1];

		if (xpos > 0x3ff) xpos -=0x800;
		if (ypos > 0x1ff) ypos -=0x400;

		draw_sprite_new_zoomed(wide, high, xpos, ypos, palt, boff * 2, flip, xzoom, xgrow, yzoom, ygrow, prio);

		source += 5;
	}
}

static void copy_sprite_priority(int prio)
{
	unsigned short *dest = pTransDraw;
	unsigned short *src = pTempScreen;
	unsigned char *pri = SpritePrio;
	for (int i = 0; i < nScreenWidth * nScreenHeight; i++)
	{
		if (pri[i] == prio) {
			dest[i] = src[i];
		}
	}
}

static void draw_text()
{
	unsigned short *vram = (unsigned short*)PGMTxtRAM;

	int scrollx = ((signed short)PGMVidReg[0x6000 / 2]) & 0x1ff;
	int scrolly = ((signed short)PGMVidReg[0x5000 / 2]) & 0x0ff;

	for (int offs = 0; offs < 64 * 32; offs++)
	{
		int code = vram[offs * 2];
		if (texttrans[code] == 0) continue; // transparent

		int sx = (offs & 0x3f) << 3;
		int sy = (offs >> 6) << 3;

		sx -= scrollx;
		if (sx < -7) sx += 512;
		sy -= scrolly;
		if (sy < -7) sy += 256;

		if (sx >= nScreenWidth || sy >= nScreenHeight) continue;

		int attr  = vram[offs * 2 + 1];
		int color = ((attr & 0x3e) >> 1) | 0x80;
		int flipx =  (attr & 0x40);
		int flipy =  (attr & 0x80);

		if (sx < 0 || sy < 0 || sx >= nScreenWidth - 8 || sy >= nScreenHeight - 8)
		{
			if (texttrans[code] & 2) { // opaque
				if (flipy) {
					if (flipx) {
						Render8x8Tile_FlipXY_Clip(pTransDraw, code, sx, sy, color, 4, 0, PGMTileROM);
					} else {
						Render8x8Tile_FlipY_Clip(pTransDraw, code, sx, sy, color, 4, 0, PGMTileROM);
					}
				} else {
					if (flipx) {
						Render8x8Tile_FlipX_Clip(pTransDraw, code, sx, sy, color, 4, 0, PGMTileROM);
					} else {
						Render8x8Tile_Clip(pTransDraw, code, sx, sy, color, 4, 0, PGMTileROM);
					}
				}
			} else {
				if (flipy) {
					if (flipx) {
						Render8x8Tile_Mask_FlipXY_Clip(pTransDraw, code, sx, sy, color, 4, 15, 0, PGMTileROM);
					} else {
						Render8x8Tile_Mask_FlipY_Clip(pTransDraw, code, sx, sy, color, 4, 15, 0, PGMTileROM);
					}
				} else {
					if (flipx) {
						Render8x8Tile_Mask_FlipX_Clip(pTransDraw, code, sx, sy, color, 4, 15, 0, PGMTileROM);
					} else {
						Render8x8Tile_Mask_Clip(pTransDraw, code, sx, sy, color, 4, 15, 0, PGMTileROM);
					}
				}
			}
		}
		else
		{
			if (texttrans[code] & 2) { // opaque
				if (flipy) {
					if (flipx) {
						Render8x8Tile_FlipXY(pTransDraw, code, sx, sy, color, 4, 0, PGMTileROM);
					} else {
						Render8x8Tile_FlipY(pTransDraw, code, sx, sy, color, 4, 0, PGMTileROM);
					}
				} else {
					if (flipx) {
						Render8x8Tile_FlipX(pTransDraw, code, sx, sy, color, 4, 0, PGMTileROM);
					} else {
						Render8x8Tile(pTransDraw, code, sx, sy, color, 4, 0, PGMTileROM);
					}
				}
			} else {
				if (flipy) {
					if (flipx) {
						Render8x8Tile_Mask_FlipXY(pTransDraw, code, sx, sy, color, 4, 15, 0, PGMTileROM);
					} else {
						Render8x8Tile_Mask_FlipY(pTransDraw, code, sx, sy, color, 4, 15, 0, PGMTileROM);
					}
				} else {
					if (flipx) {
						Render8x8Tile_Mask_FlipX(pTransDraw, code, sx, sy, color, 4, 15, 0, PGMTileROM);
					} else {
						Render8x8Tile_Mask(pTransDraw, code, sx, sy, color, 4, 15, 0, PGMTileROM);
					}
				}
			}
		}
	}
}

static void draw_background()
{
	unsigned short *vram = (unsigned short*)PGMBgRAM;
	unsigned short *dst   = pTransDraw;

	unsigned short *rowscroll = PGMRowRAM;
	int yscroll = (signed short)PGMVidReg[0x2000 / 2];
	int xscroll = (signed short)PGMVidReg[0x3000 / 2];

	// check to see if we need to do line scroll
	int t = 0;
	{
		unsigned short *rs = rowscroll;
		for (int i = 0; i < 224; i++) {
			if (rs[0] != rs[i]) {
				t = 1;
				break;
			}
		}
	}

	// no line scroll (fast)
	if (t == 0)
	{
		yscroll &= 0x1ff;
		xscroll &= 0x7ff;

		for (int offs = 0; offs < 64 * 16; offs++)
		{
			int sx = (offs & 0x3f) << 5;
			int sy = (offs >> 6) << 5;

			sx -= xscroll;
			if (sx < -31) sx += 2048;
			sy -= yscroll;
			if (sy < -31) sy += 512;

			if (sx >= nScreenWidth || sy >= nScreenHeight) continue;

			int code = vram[offs * 2];
			if (code >= nTileMask) continue;
			if (tiletrans[code] == 0) continue; // transparent
			int color = ((vram[offs*2+1] & 0x3e) >> 1) | 0x20;
			int flipy = vram[offs*2+1] & 0x80;
			int flipx = vram[offs*2+1] & 0x40;

			if (sx < 0 || sy < 0 || sx >= nScreenWidth - 32 || sy >= nScreenHeight - 32)
			{
				if (tiletrans[code] & 2) { // opaque
					if (flipy) {
						if (flipx) {
							Render32x32Tile_FlipXY_Clip(pTransDraw, code, sx, sy, color, 5, 0, PGMTileROMExp);
						} else {
							Render32x32Tile_FlipY_Clip(pTransDraw, code, sx, sy, color, 5, 0, PGMTileROMExp);
						}
					} else {
						if (flipx) {
							Render32x32Tile_FlipX_Clip(pTransDraw, code, sx, sy, color, 5, 0, PGMTileROMExp);
						} else {
							Render32x32Tile_Clip(pTransDraw, code, sx, sy, color, 5, 0, PGMTileROMExp);
						}
					}
				} else {
					if (flipy) {
						if (flipx) {
							Render32x32Tile_Mask_FlipXY_Clip(pTransDraw, code, sx, sy, color, 5, 0x1f, 0, PGMTileROMExp);
						} else {
							Render32x32Tile_Mask_FlipY_Clip(pTransDraw, code, sx, sy, color, 5, 0x1f, 0, PGMTileROMExp);
						}
					} else {
						if (flipx) {
							Render32x32Tile_Mask_FlipX_Clip(pTransDraw, code, sx, sy, color, 5, 0x1f, 0, PGMTileROMExp);
						} else {
							Render32x32Tile_Mask_Clip(pTransDraw, code, sx, sy, color, 5, 0x1f, 0, PGMTileROMExp);
						}
					}
				}
			}
			else
			{
				if (tiletrans[code] & 2) { // opaque
					if (flipy) {
						if (flipx) {
							Render32x32Tile_FlipXY(pTransDraw, code, sx, sy, color, 5, 0, PGMTileROMExp);
						} else {
							Render32x32Tile_FlipY(pTransDraw, code, sx, sy, color, 5, 0, PGMTileROMExp);
						}
					} else {
						if (flipx) {
							Render32x32Tile_FlipX(pTransDraw, code, sx, sy, color, 5, 0, PGMTileROMExp);
						} else {
							Render32x32Tile(pTransDraw, code, sx, sy, color, 5, 0, PGMTileROMExp);
						}
					}
				} else {
					if (flipy) {
						if (flipx) {
							Render32x32Tile_Mask_FlipXY(pTransDraw, code, sx, sy, color, 5, 0x1f, 0, PGMTileROMExp);
						} else {
							Render32x32Tile_Mask_FlipY(pTransDraw, code, sx, sy, color, 5, 0x1f, 0, PGMTileROMExp);
						}
					} else {
						if (flipx) {
							Render32x32Tile_Mask_FlipX(pTransDraw, code, sx, sy, color, 5, 0x1f, 0, PGMTileROMExp);
						} else {
							Render32x32Tile_Mask(pTransDraw, code, sx, sy, color, 5, 0x1f, 0, PGMTileROMExp);
						}
					}
				}
			}
		}

		return;
	}

	// do line scroll (slow)
	for (int y = 0; y < 224; y++, dst += nScreenWidth)
	{
		int scrollx = (xscroll + rowscroll[y]) & 0x7ff;
		int scrolly = (yscroll + y) & 0x7ff;

		for (int x = 0; x < 480; x+=32)
		{
			int sx = x - (scrollx & 0x1f);
			if (sx >= nScreenWidth) break;

			int offs = ((scrolly & 0x1e0) << 2) | (((scrollx + x) & 0x7e0) >> 4);

			int code  = vram[offs];
			if (code >= nTileMask) continue;
			if (tiletrans[code] == 0) continue;

			int attr  = vram[offs + 1];
			int color = ((attr & 0x3e) << 4) | 0x400;
			int flipx = ((attr & 0x40) >> 6) * 0x1f;
			int flipy = ((attr & 0x80) >> 7) * 0x1f;

			unsigned char *src = PGMTileROMExp + (code * 1024) + (((scrolly ^ flipy) & 0x1f) << 5);

			if (sx >= 0 && sx <= 415) {
				for (int xx = 0; xx < 32; xx++, sx++) {
					int pxl = src[xx^flipx];
	
					if (pxl != 0x1f) {
						dst[sx] = pxl | color;
					}
				}
			} else {
				for (int xx = 0; xx < 32; xx++, sx++) {
					if (sx < 0) continue;
					if (sx >= nScreenWidth) break;
	
					int pxl = src[xx^flipx];
	
					if (pxl != 0x1f) {
						dst[sx] = pxl | color;
					}
				}
			}
		}
	}
}

int pgmDraw()
{
	if (nPgmPalRecalc) {
		for (int i = 0; i < 0x1200 / 2; i++) {
			RamCurPal[i] = CalcCol(PGMPalRAM[i]);
		}
		nPgmPalRecalc = 0;
	}

	{
		// black / magenta
		RamCurPal[0x1200/2] = (nBurnLayer & 1) ? 0 : BurnHighCol(0xff, 0, 0xff, 0);
	}

	// Fill in background color (0x1200/2)
	// also, clear buffers
	{
		for (int i = 0; i < nScreenWidth * nScreenHeight; i++) {
			pTransDraw[i] = 0x900;
			pTempScreen[i] = 0;
			SpritePrio[i] = 0xff;
		}
	}

	pgm_drawsprites();
	if (nSpriteEnable & 1) copy_sprite_priority(1);
	if (nBurnLayer & 1) draw_background();
	if (nSpriteEnable & 2) copy_sprite_priority(0);
	if (nBurnLayer & 2) draw_text();

	BurnTransferCopy(RamCurPal);

	return 0;
}

void pgmInitDraw() // preprocess some things...
{
	GenericTilesInit();

	pTempDraw = (unsigned short*)malloc(0x400 * 0x200 * 2);
	SpritePrio = (unsigned char*)malloc(nScreenWidth * nScreenHeight);
	pTempScreen = (unsigned short*)malloc(nScreenWidth * nScreenHeight * sizeof(short));

	// Find transparent tiles so we can skip them
	{
		nTileMask = ((nPGMTileROMLen / 5) * 8) / 0x400; // also used to set max. tile

		// background tiles
		tiletrans = (unsigned char*)malloc(nTileMask);
		memset (tiletrans, 0, nTileMask);
	
		for (int i = 0; i < nTileMask << 10; i += 0x400)
		{
			int k = 0x1f;
			for (int j = 0; j < 0x400; j++)
			{
				if (PGMTileROMExp[i+j] != 0x1f) {
					tiletrans[i/0x400] = 1;
				}
				k &= (PGMTileROMExp[i+j] ^ 0x1f);
			}
			if (k) tiletrans[i/0x400] |= 2;
		}

		// character tiles
		texttrans = (unsigned char*)malloc(0x10000);
		memset (texttrans, 0, 0x10000);

		for (int i = 0; i < 0x400000; i += 0x40)
		{
			int k = 0xf;
			for (int j = 0; j < 0x40; j++)
			{
				if (PGMTileROM[i+j] != 0xf) {
					texttrans[i/0x40] = 1;
				}
				k &= (PGMTileROM[i+j] ^ 0xf);
			}
			if (k) texttrans[i/0x40] |= 2;
		}
	}

	// set up table to count bits in sprite mask data
	// gives a good speedup in sprite drawing. ^^
	{
		memset (sprmsktab, 0, 0x100);
		for (int i = 0; i < 0x100; i++) {
			for (int j = 0; j < 8; j++) {
				if (i & (1 << j)) {
					sprmsktab[i]++;
				}
			}
		}
	}
}

void pgmExitDraw()
{
	nTileMask = 0;
	free (pTempDraw);
	free (tiletrans);
	free (texttrans);
	free (pTempScreen);
	free (SpritePrio);

	GenericTilesExit();
}
