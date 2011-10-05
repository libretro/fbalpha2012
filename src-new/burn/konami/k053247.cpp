#include "tiles_generic.h"
#include "konamiic.h"

#define K053247_CUSTOMSHADOW	0x20000000
#define K053247_SHDSHIFT		20

static unsigned char  K053246Regs[8];
static unsigned char  K053246_OBJCHA_line;
static unsigned char *K053247Ram;
static unsigned short K053247Regs[16];

static unsigned char *K053246Gfx;
static unsigned int   K053246Mask;

static unsigned short *K053247Temp = NULL;

static int K053247_dx;
static int K053247_dy;
static int K053247_wraparound;

static int K053247Flags;

static void (*K053247Callback)(int *code, int *color, int *priority);

void K053247Reset()
{
	memset (K053247Ram,  0, 0x1000);
	memset (K053247Regs, 0, 16 * sizeof (short));
	memset (K053246Regs, 0, 8);

	K053246_OBJCHA_line = 0; // clear
}

void K053247Scan(int nAction)
{
	struct BurnArea ba;
	
	if (nAction & ACB_MEMORY_RAM) {
		memset(&ba, 0, sizeof(ba));
		ba.Data	  = K053247Ram;
		ba.nLen	  = 0x1000;
		ba.szName = "K053247 Ram";
		BurnAcb(&ba);

		ba.Data	  = K053247Regs;
		ba.nLen	  = 0x0010 * sizeof(short);
		ba.szName = "K053247 Regs";
		BurnAcb(&ba);

		ba.Data	  = K053246Regs;
		ba.nLen	  = 0x0008;
		ba.szName = "K053246 Regs";
		BurnAcb(&ba);

		SCAN_VAR(K053246_OBJCHA_line);
		SCAN_VAR(K053247_wraparound);
	}
}

void K053247Init(unsigned char *gfxrom, int gfxlen, void (*Callback)(int *code, int *color, int *priority), int flags)
{
	K053247Ram = (unsigned char*)malloc(0x1000);

	K053246Gfx = gfxrom;
	K053246Mask = gfxlen;

	K053247Callback = Callback;

	K053247_dx = 0;
	K053247_dy = 0;
	K053247_wraparound = 1;

	if (konami_temp_screen == NULL) {
		int width, height;
		BurnDrvGetVisibleSize(&width, &height);
		konami_temp_screen = (unsigned short*)malloc(width * height * 2);
	}

	K053247Temp = konami_temp_screen;

	K053247Flags = flags; // 0x02 highlight, 0x01 shadow

	KonamiIC_K053247InUse = 1;
}

void K053247Exit()
{
	K053247Temp = NULL;

	if (K053247Ram) {
		free (K053247Ram);
		K053247Ram = NULL;
	}

	K053247Flags = 0;

	memset (K053247Regs, 0, 16 * sizeof(short));
}

void K053247Export(unsigned char **ram, unsigned char **gfx, void (**callback)(int *, int *, int *), int *dx, int *dy)
{
	if (ram) *ram = K053247Ram;
	if (gfx) *gfx = K053246Gfx;

	if (dx)	*dx = K053247_dx;
	if (dy)	*dy = K053247_dy;

	if(callback) *callback = K053247Callback;
}

void K053247GfxDecode(unsigned char *src, unsigned char *dst, int len) // 16x16
{
	for (int i = 0; i < len; i++)
	{
		int t = src[i^1];
		dst[(i << 1) + 0] = t >> 4;
		dst[(i << 1) + 1] = t & 0x0f;
	}
}

void K053247SetSpriteOffset(int offsx, int offsy)
{
	K053247_dx = offsx;
	K053247_dy = offsy;
}

void K053247WrapEnable(int status)
{
	K053247_wraparound = status;
}

unsigned char K053247Read(int offset)
{
	return K053247Ram[offset & 0xfff];
}

void K053247Write(int offset, int data)
{
	if (data & 0x10000) { // use word
		*((unsigned short*)(K053247Ram + (offset & 0xffe))) = data;
	} else {
		K053247Ram[offset & 0xfff] = data;
	}
}

unsigned char K053246Read(int offset)
{
	if (K053246_OBJCHA_line) // assert_line
	{
		int addr;

		addr = (K053246Regs[6] << 17) | (K053246Regs[7] << 9) | (K053246Regs[4] << 1) | ((offset & 1) ^ 1);
		addr &= K053246Mask;

		return K053246Gfx[addr];
	}
	else
	{
		return 0;
	}
}

void K053246Write(int offset, int data)
{
	if (data & 0x10000) { // handle it as a word
		*((unsigned short*)(K053246Regs + (offset & 6))) = data;
	} else {
		K053246Regs[offset & 7] = data;
	}
}

void K053246_set_OBJCHA_line(int state)
{
	K053246_OBJCHA_line = state;
}

int K053246_is_IRQ_enabled()
{
	return K053246Regs[5] & 0x10;
}

static void RenderZoomedShadowTile(unsigned char *gfx, int code, int color, unsigned char *t, int sx, int sy, int fx, int fy, int width, int height, int zoomx, int zoomy, int highlight)
{
	int h = ((zoomy << 4) + 0x8000) >> 16;
	int w = ((zoomx << 4) + 0x8000) >> 16;

	if (!h || !w || sx + w < 0 || sy + h < 0 || sx >= nScreenWidth || sy >= nScreenHeight) return;

	if (fy) fy  = (height-1)*width;
	if (fx) fy |= (width-1);

	int hz = (height << 12) / h;
	int wz = (width << 12) / w;

	int starty = 0, startx = 0, endy = h, endx = w;
	if (sy < 0) starty = 0 - sy;
	if (sx < 0) startx = 0 - sx;
	if (sy + h >= nScreenHeight) endy -= (h + sy) - nScreenHeight;
	if (sx + w >= nScreenWidth ) endx -= (w + sx) - nScreenWidth;

	unsigned char  *src = gfx + (code * width * height);
	unsigned short *dst = K053247Temp + (sy + starty) * nScreenWidth + sx;
	unsigned short *pTemp = pTransDraw + (sy + starty) * nScreenWidth + sx;

	int or1 = 0x8000 >> highlight;

	for (int y = starty; y < endy; y++)
	{
		int zy = ((y * hz) >> 12) * width;

		for (int x = startx; x < endx; x++)
		{
			int pxl = src[(zy + ((x * wz) >> 12)) ^ fy];

			if (pxl) {
				if (t[pxl] == 2) {
					dst[x] = color | pxl;
					pTemp[x] |= or1;
				} else {
					pTemp[x] = color | pxl;
				}
			}
		}

		dst += nScreenWidth;
		pTemp += nScreenWidth;
	}
}

void K053247SpritesRender(unsigned char *gfxbase, int priority)
{
#define NUM_SPRITES 256

	unsigned char dtable[256];
	unsigned char stable[256];
	unsigned char *wtable;

	memset(dtable, 1, 256);
	dtable[0] = 0;
	memset(stable, 2, 256);
	stable[0] = 0;

	static const int xoffset[8] = { 0, 1, 4, 5, 16, 17, 20, 21 };
	static const int yoffset[8] = { 0, 2, 8, 10, 32, 34, 40, 42 };

	int sortedlist[NUM_SPRITES];
	int offs,zcode;
	int ox,oy,color,code,size,w,h,x,y,xa,ya,flipx,flipy,mirrorx,mirrory,shadow,zoomx,zoomy,primask;
	int nozoom,count,temp,shdmask;

	int flipscreenx = K053246Regs[5] & 0x01;
	int flipscreeny = K053246Regs[5] & 0x02;
	int offx = (short)((K053246Regs[0] << 8) | K053246Regs[1]);
	int offy = (short)((K053246Regs[2] << 8) | K053246Regs[3]);

	unsigned short *SprRam = (unsigned short*)K053247Ram;

	int screen_width = nScreenWidth-1;

	if (K053247Flags & 1) {
		if (K053247Flags & 2) {
			shdmask = 3;
		} else {
			shdmask = 0;
		}
	} else {
		shdmask = -1;
	}

	// Prebuild a sorted table by descending Z-order.
	zcode = K05324xZRejection;
	offs = count = 0;

	if (zcode == -1)
	{
		for (; offs<0x800; offs+=8)
			if (SprRam[offs] & 0x8000) sortedlist[count++] = offs;
	}
	else
	{
		for (; offs<0x800; offs+=8)
			if ((SprRam[offs] & 0x8000) && ((SprRam[offs] & 0xff) != zcode)) sortedlist[count++] = offs;
	}

	w = count;
	count--;
	h = count;

	if (!(K053247Regs[0xc/2] & 0x10))
	{
		// sort objects in decending order(smaller z closer) when OPSET PRI is clear
		for (y=0; y<h; y++)
		{
			offs = sortedlist[y];
			zcode = SprRam[offs] & 0xff;
			for (x=y+1; x<w; x++)
			{
				temp = sortedlist[x];
				code = SprRam[temp] & 0xff;
				if (zcode <= code) { zcode = code; sortedlist[x] = offs; sortedlist[y] = offs = temp; }
			}
		}
	}
	else
	{
		// sort objects in ascending order(bigger z closer) when OPSET PRI is set
		for (y=0; y<h; y++)
		{
			offs = sortedlist[y];
			zcode = SprRam[offs] & 0xff;
			for (x=y+1; x<w; x++)
			{
				temp = sortedlist[x];
				code = SprRam[temp] & 0xff;
				if (zcode >= code) { zcode = code; sortedlist[x] = offs; sortedlist[y] = offs = temp; }
			}
		}
	}

	for (int i = 0; i <= count; i++)
	{
		offs = sortedlist[i];

		code = SprRam[offs+1];
		shadow = color = SprRam[offs+6];
		primask = 0;

		(*K053247Callback)(&code,&color,&primask);

		if (primask != priority) continue;	//--------------------------------------- fix me!

		temp = SprRam[offs];

		size = (temp & 0x0f00) >> 8;
		w = 1 << (size & 0x03);
		h = 1 << ((size >> 2) & 0x03);

		/* the sprite can start at any point in the 8x8 grid. We have to */
		/* adjust the offsets to draw it correctly. Simpsons does this all the time. */
		xa = 0;
		ya = 0;
		if (code & 0x01) xa += 1;
		if (code & 0x02) ya += 1;
		if (code & 0x04) xa += 2;
		if (code & 0x08) ya += 2;
		if (code & 0x10) xa += 4;
		if (code & 0x20) ya += 4;
		code &= ~0x3f;

		oy = (short)SprRam[offs+2];
		ox = (short)SprRam[offs+3];

		if (K053247_wraparound)
		{
			offx &= 0x3ff;
			offy &= 0x3ff;
			oy &= 0x3ff;
			ox &= 0x3ff;
		}

		y = zoomy = SprRam[offs+4] & 0x3ff;
		if (zoomy) zoomy = (0x400000+(zoomy>>1)) / zoomy; else zoomy = 0x800000;
		if (!(temp & 0x4000))
		{
			x = zoomx = SprRam[offs+5] & 0x3ff;
			if (zoomx) zoomx = (0x400000+(zoomx>>1)) / zoomx;
			else zoomx = 0x800000;
		}
		else { zoomx = zoomy; x = y; }

		if ( K053246Regs[5] & 0x08 ) // Check only "Bit #3 is '1'?" (NOTE: good guess)
		{
			zoomx >>= 1;		// Fix sprite width to HALF size
			ox = (ox >> 1) + 1;	// Fix sprite draw position
			if (flipscreenx) ox += screen_width;
			nozoom = 0;
		}
		else
			nozoom = (x == 0x40 && y == 0x40);

		flipx = temp & 0x1000;
		flipy = temp & 0x2000;
		mirrorx = shadow & 0x4000;
		if (mirrorx) flipx = 0; // documented and confirmed
		mirrory = shadow & 0x8000;

		int highlight = 0;

		wtable = dtable;
		if (color == -1)
		{
			// drop the entire sprite to shadow unconditionally
			if (shdmask < 0) continue;
			color = 0;
			shadow = -1;
			wtable = stable;
		}
		else
		{
			if (shdmask >= 0)
			{
				shadow = (color & 0x20000000) ? (color >> 20) : (shadow >> 10);
				if (shadow &= 3) {
					if (((shadow-1) & shdmask) == 1) highlight = 1;
				}
			}
			else
				shadow = 0;
		}

		color &= 0xffff; // strip attribute flags

		if (flipscreenx)
		{
			ox = -ox;
			if (!mirrorx) flipx = !flipx;
		}
		if (flipscreeny)
		{
			oy = -oy;
			if (!mirrory) flipy = !flipy;
		}

		// apply wrapping and global offsets
		if (K053247_wraparound)
		{
			ox = ( ox - offx) & 0x3ff;
			oy = (-oy - offy) & 0x3ff;
			if (ox >= 0x300) ox -= 0x400;
			if (oy >= 0x280) oy -= 0x400;
		}
		else
		{
			ox =  ox - offx;
			oy = -oy - offy;
		}
		ox += K053247_dx;
		oy -= K053247_dy;

		ox = ((ox+16) & 0x3ff) - 16;
		oy = ((oy+16) & 0x3ff) - 16;

		// apply global and display window offsets

		/* the coordinates given are for the *center* of the sprite */
		ox -= (zoomx * w) >> 13;
		oy -= (zoomy * h) >> 13;

		dtable[15] = shadow ? 2 : 1;

		for (y = 0;y < h;y++)
		{
			int sx,sy,zw,zh;

			sy = oy + ((zoomy * y + (1<<11)) >> 12);
			zh = (oy + ((zoomy * (y+1) + (1<<11)) >> 12)) - sy;

			for (x = 0;x < w;x++)
			{
				int c,fx,fy;

				sx = ox + ((zoomx * x + (1<<11)) >> 12);
				zw = (ox + ((zoomx * (x+1) + (1<<11)) >> 12)) - sx;
				c = code;
				if (mirrorx)
				{
					if ((flipx == 0) ^ ((x<<1) < w))
					{
						/* mirror left/right */
						c += xoffset[(w-1-x+xa)&7];
						fx = 1;
					}
					else
					{
						c += xoffset[(x+xa)&7];
						fx = 0;
					}
				}
				else
				{
					if (flipx) c += xoffset[(w-1-x+xa)&7];
					else c += xoffset[(x+xa)&7];
					fx = flipx;
				}

				if (mirrory)
				{
					if ((flipy == 0) ^ ((y<<1) >= h))
					{
						/* mirror top/bottom */
						c += yoffset[(h-1-y+ya)&7];
						fy = 1;
					}
					else
					{
						c += yoffset[(y+ya)&7];
						fy = 0;
					}
				}
				else
				{
					if (flipy) c += yoffset[(h-1-y+ya)&7];
					else c += yoffset[(y+ya)&7];
					fy = flipy;
				}

				if (shadow || wtable == stable) {
					if (mirrory && h == 1)
						RenderZoomedShadowTile(gfxbase, c, color << 4, wtable, sx, sy, flipx, !flipy, 16, 16, zw << 12, zh << 12, highlight);

					RenderZoomedShadowTile(gfxbase, c, color << 4, wtable, sx, sy, flipx,  flipy, 16, 16, zw << 12, zh << 12, highlight);
					continue;
				}

				if (mirrory && h == 1)
				{
					if (nozoom)
					{
						if (!flipy) {
							if (flipx) {
								Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, c, sx, sy, color, 4, 0, 0, gfxbase);
							} else {
								Render16x16Tile_Mask_FlipY_Clip(pTransDraw, c, sx, sy, color, 4, 0, 0, gfxbase);
							}
						} else {
							if (flipx) {
								Render16x16Tile_Mask_FlipX_Clip(pTransDraw, c, sx, sy, color, 4, 0, 0, gfxbase);
							} else {
								Render16x16Tile_Mask_Clip(pTransDraw, c, sx, sy, color, 4, 0, 0, gfxbase);
							}
						}
					}
					else
					{
						RenderZoomedTile(pTransDraw, gfxbase, c, color << 4, 0, sx, sy, fx, !fy, 16, 16, (zw << 16) >> 4, (zh << 16) >> 4);
					}
				}

				if (nozoom)
				{
					if (flipy) {
						if (flipx) {
							Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, c, sx, sy, color, 4, 0, 0, gfxbase);
						} else {
							Render16x16Tile_Mask_FlipY_Clip(pTransDraw, c, sx, sy, color, 4, 0, 0, gfxbase);
						}
					} else {
						if (flipx) {
							Render16x16Tile_Mask_FlipX_Clip(pTransDraw, c, sx, sy, color, 4, 0, 0, gfxbase);
						} else {
							Render16x16Tile_Mask_Clip(pTransDraw, c, sx, sy, color, 4, 0, 0, gfxbase);
						}
					}
				}
				else
				{
					RenderZoomedTile(pTransDraw, gfxbase, c, color << 4, 0, sx, sy, fx, fy, 16, 16, (zw << 16) >> 4, (zh << 16) >> 4);
				}


			} // end of X loop
		} // end of Y loop

	} // end of sprite-list loop
#undef NUM_SPRITES
}
