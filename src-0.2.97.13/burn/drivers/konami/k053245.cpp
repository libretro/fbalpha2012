#include "tiles_generic.h"
#include "konamiic.h"

static void (*K053245Callback[2])(int *code,int *color,int *priority);

static unsigned char *K053245Ram[2];
static unsigned char *K053245Buf[2];
static unsigned char *K053245Gfx[2];
static int K053245Mask[2];
static int K053245_dx[2];
static int K053245_dy[2];

static unsigned short *K053245Temp = NULL;

static unsigned char K053244Regs[2][0x10];
static int K053244Bank[2];

static int K053245Active = 0;

// Init, Reset, Exit

int K053245Reset()
{
	for (int i = 0; i < K053245Active; i++) {
		memset (K053245Ram[i], 0, 0x800);
		memset (K053245Buf[i], 0, 0x800);
		memset (K053244Regs[i], 0, 0x10);
	
		K053244Bank[i] = 0;
	}

	return 0;
}

void K053245GfxDecode(unsigned char *src, unsigned char *dst, int len)
{
	int Plane[4]  = { 24, 16, 8, 0 };
	int XOffs[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 8*32+0, 8*32+1, 8*32+2, 8*32+3, 8*32+4, 8*32+5, 8*32+6, 8*32+7 };
	int YOffs[16] = { 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,16*32, 17*32, 18*32, 19*32, 20*32, 21*32, 22*32, 23*32 };

	GfxDecode(len >> 7, 4, 16, 16, Plane, XOffs, YOffs, 0x400, src, dst);
}

void K053245Init(int chip, unsigned char *gfx, int mask, void (*callback)(int *code,int *color,int *priority))
{
	K053245Ram[chip] = (unsigned char*)malloc(0x800); // enough
	K053245Buf[chip] = (unsigned char*)malloc(0x800); // enough

	K053245Mask[chip] = mask;

	K053245Gfx[chip] = gfx;

	K053245Callback[chip] = callback;

	K053245Active++;

	KonamiIC_K053245InUse = 1;

	if (konami_temp_screen == NULL) {
		int width, height;
		BurnDrvGetVisibleSize(&width, &height);
		konami_temp_screen = (unsigned short*)malloc(width * height * 2);
	}

	K053245Temp = konami_temp_screen;

	K053245Reset();
}

void K053245Exit()
{
	K053245Temp = NULL;

	for (int i = 0; i < K053245Active; i++) {
		if (K053245Ram[i]) {
			free (K053245Ram[i]);
			K053245Ram[i] = NULL;
		}
		if (K053245Buf[i]) {
			free (K053245Buf[i]);
			K053245Buf[i] = NULL;
		}

		K053245_dx[i] = 0;
		K053245_dy[i] = 0;
	}

	K053245Active = 0;
}

// Useful functions

void K053245SetSpriteOffset(int chip,int offsx, int offsy)
{
	K053245_dx[chip] = offsx;
	K053245_dy[chip] = offsy;
}

void K053245ClearBuffer(int chip)
{
	unsigned short *buf = (unsigned short*)K053245Buf[chip];

	for (int i = 0; i < 0x800/2; i+=8) buf[i] = 0;
}

void K053245UpdateBuffer(int chip)
{
	memcpy (K053245Buf[chip], K053245Ram[chip], 0x800);
}

void K053244BankSelect(int chip, int bank)
{
	K053244Bank[chip] = bank;
}


// 8 bit read/write handlers

unsigned char K053245Read(int chip, int offset)
{
	return K053245Ram[chip][offset ^ 1]; //
}

void K053245Write(int chip, int offset, int data)
{
	K053245Ram[chip][offset ^ 1] = data; //
}

unsigned short K053245ReadWord(int chip, int offset)
{
	unsigned short *ret = (unsigned short*)K053245Ram[chip];

#if 0
	int r = ret[offset];

	return (r << 8) | (r >> 8);
#else
	return ret[offset];
#endif
}

void K053245WriteWord(int chip, int offset, int data)
{
	unsigned short *ret = (unsigned short*)K053245Ram[chip];

#if 0
	ret[offset] = (data << 8) | (data >> 8);
#else
	ret[offset] = data;
#endif
}

unsigned char K053244Read(int chip, int offset)
{
	if (K053244Regs[chip][5] & 0x10 && (offset & 0x0c) == 0x0c)
	{
		int addr  = (K053244Bank[chip]    << 19) | ((K053244Regs[chip][11] & 0x07) << 18);
		    addr |= (K053244Regs[chip][8] << 10) |  (K053244Regs[chip][ 9] << 2);
		    addr |= (offset & 3) ^ 1;
		    addr &=  K053245Mask[chip];

		return K053245Gfx[chip][addr];
	}
	else if (offset == 0x06) {
		K053245UpdateBuffer(chip);
	}

	return 0;
}

void K053244Write(int chip, int offset, int data)
{
	K053244Regs[chip][offset] = data;

	if (offset == 0x06) {
		K053245UpdateBuffer(chip);
	}
}

// Sprite Rendering

static void RenderZoomedShadowTile(unsigned char *gfx, int code, int color, int sx, int sy, int fx, int fy, int width, int height, int zoomx, int zoomy)
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
	unsigned short *dst = konami_temp_screen + (sy + starty) * nScreenWidth + sx;
	unsigned short *pTemp = pTransDraw + (sy + starty) * nScreenWidth + sx;

	int or1 = 0x8000;

	for (int y = starty; y < endy; y++)
	{
		int zy = ((y * hz) >> 12) * width;

		for (int x = startx; x < endx; x++)
		{
			int pxl = src[(zy + ((x * wz) >> 12)) ^ fy];

			if (pxl) {
				if (pxl == 15) {
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

void K053245SpritesRender(int chip, unsigned char *gfxdata, int priority)
{
#define NUM_SPRITES 128
	int offs,pri_code,i;
	int sortedlist[NUM_SPRITES];
	int flipscreenX, flipscreenY, spriteoffsX, spriteoffsY;

	flipscreenX = K053244Regs[chip][5] & 0x01;
	flipscreenY = K053244Regs[chip][5] & 0x02;
	spriteoffsX = (K053244Regs[chip][0] << 8) | K053244Regs[chip][1];
	spriteoffsY = (K053244Regs[chip][2] << 8) | K053244Regs[chip][3];

	for (offs = 0;offs < NUM_SPRITES;offs++)
		sortedlist[offs] = -1;

	unsigned short *sprbuf = (unsigned short*)K053245Buf[chip];

	/* prebuild a sorted table */
	for (i=0x800/2, offs=0; offs<i; offs+=8)
	{
		pri_code = sprbuf[offs];
		if (pri_code & 0x8000)
		{
			pri_code &= 0x007f;

			if (offs && pri_code == K05324xZRejection) continue;

			if (sortedlist[pri_code] == -1) sortedlist[pri_code] = offs;
		}
	}

	for (pri_code = 0; pri_code < NUM_SPRITES; pri_code++)
	{
		int ox,oy,color,code,size,w,h,x,y,flipx,flipy,mirrorx,mirrory,shadow,zoomx,zoomy,pri;

		offs = sortedlist[pri_code];
		if (offs == -1) continue;

		code = sprbuf[offs+1];
		code = ((code & 0xffe1) + ((code & 0x0010) >> 2) + ((code & 0x0008) << 1)
				 + ((code & 0x0004) >> 1) + ((code & 0x0002) << 2));
		color = sprbuf[offs+6] & 0x00ff;
		pri = 0;

		(*K053245Callback[chip])(&code,&color,&pri);

		if (pri != priority) continue;//------------------------------------------------------- OK??

		size = (sprbuf[offs] & 0x0f00) >> 8;

		w = 1 << (size & 0x03);
		h = 1 << ((size >> 2) & 0x03);

		/* zoom control:
           0x40 = normal scale
          <0x40 enlarge (0x20 = double size)
          >0x40 reduce (0x80 = half size)
        */
		zoomy = sprbuf[offs+4];
		if (zoomy > 0x2000) continue;
		if (zoomy) zoomy = (0x400000+zoomy/2) / zoomy;
		else zoomy = 2 * 0x400000;
		if ((sprbuf[offs] & 0x4000) == 0)
		{
			zoomx = sprbuf[offs+5];
			if (zoomx > 0x2000) continue;
			if (zoomx) zoomx = (0x400000+zoomx/2) / zoomx;
			else zoomx = 2 * 0x400000;
//          else zoomx = zoomy; /* workaround for TMNT2 */
		}
		else zoomx = zoomy;

		ox = sprbuf[offs+3] + spriteoffsX;
		oy = sprbuf[offs+2];

		ox += K053245_dx[chip];
		oy += K053245_dy[chip];

		flipx = sprbuf[offs] & 0x1000;
		flipy = sprbuf[offs] & 0x2000;
		mirrorx = sprbuf[offs+6] & 0x0100;
		if (mirrorx) flipx = 0; // documented and confirmed
		mirrory = sprbuf[offs+6] & 0x0200;
		shadow = sprbuf[offs+6] & 0x0080;

		if (flipscreenX)
		{
			ox = 512 - ox;
			if (!mirrorx) flipx = !flipx;
		}
		if (flipscreenY)
		{
			oy = -oy;
			if (!mirrory) flipy = !flipy;
		}

		ox = (ox + 0x5d) & 0x3ff;
		if (ox >= 768) ox -= 1024;
		oy = (-(oy + spriteoffsY + 0x07)) & 0x3ff;
		if (oy >= 640) oy -= 1024;

		/* the coordinates given are for the *center* of the sprite */
		ox -= (zoomx * w) >> 13;
		oy -= (zoomy * h) >> 13;

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
					if ((flipx == 0) ^ (2*x < w))
					{
						/* mirror left/right */
						c += (w-x-1);
						fx = 1;
					}
					else
					{
						c += x;
						fx = 0;
					}
				}
				else
				{
					if (flipx) c += w-1-x;
					else c += x;
					fx = flipx;
				}
				if (mirrory)
				{
					if ((flipy == 0) ^ (2*y >= h))
					{
						/* mirror top/bottom */
						c += 8*(h-y-1);
						fy = 1;
					}
					else
					{
						c += 8*y;
						fy = 0;
					}
				}
				else
				{
					if (flipy) c += 8*(h-1-y);
					else c += 8*y;
					fy = flipy;
				}

				c = (c & 0x3f) | (code & ~0x3f);

				if (shadow) {
					RenderZoomedShadowTile(gfxdata, c, color << 4, sx, sy, fx, fy, 16, 16, zw << 12, zh << 12);
					continue;
				}

				if (zoomx == 0x10000 && zoomy == 0x10000)
				{
					if (fy) {
						if (fx) {
							Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, c, sx, sy, color, 4, 0, 0, gfxdata);
						} else {
							Render16x16Tile_Mask_FlipY_Clip(pTransDraw, c, sx, sy, color, 4, 0, 0, gfxdata);
						}
					} else {
						if (fx) {
							Render16x16Tile_Mask_FlipX_Clip(pTransDraw, c, sx, sy, color, 4, 0, 0, gfxdata);
						} else {
							Render16x16Tile_Mask_Clip(pTransDraw, c, sx, sy, color, 4, 0, 0, gfxdata);
						}
					}
				}
				else
				{
					RenderZoomedTile(pTransDraw, gfxdata, c, color << 4, 0, sx, sy, fx, fy, 16, 16, zw << 12, zh << 12);
				}
			}
		}
	}
}

void K053245Scan(int nAction)
{
	struct BurnArea ba;
	
	if (nAction & ACB_MEMORY_RAM) {
		for (int i = 0; i < 2; i++) {
			if (K053245Ram[i]) {
				memset(&ba, 0, sizeof(ba));
				ba.Data	  = K053245Ram[i];
				ba.nLen	  = 0x800;
				ba.szName = "K053245 Ram";
				BurnAcb(&ba);

				ba.Data	  = K053245Buf[i];
				ba.nLen	  = 0x800;
				ba.szName = "K053245 Buffer";
				BurnAcb(&ba);
			}

			memset(&ba, 0, sizeof(ba));
			ba.Data	  = K053244Regs[i];
			ba.nLen	  = 0x010;
			ba.szName = "K053244 Registers";
			BurnAcb(&ba);	
		}
	}

	if (nAction & ACB_DRIVER_DATA) {
		SCAN_VAR(K053244Bank[0]);
		SCAN_VAR(K053244Bank[1]);
	}
}
