#include "tiles_generic.h"
#include "konamiic.h"

static unsigned short *K051316TileMap[3];
static void (*K051316Callback[3])(int *code,int *color,int *flags);
static int K051316Depth[3];
static int K051316TransColor[3];
static unsigned char *K051316Gfx[3];
static unsigned char *K051316GfxExp[3];
static int K051316Mask[3];
static int K051316Offs[3][2];

static unsigned char *K051316Ram[3];
static unsigned char K051316Ctrl[3][16];
static unsigned char K051316Wrap[3];

// Decode 4bpp graphics
static void K051316GfxExpand(unsigned char *src, unsigned char *dst, int len)
{
	for (int i = 0; i < len; i++) {
		int d = src[i];
		dst[i * 2 + 0] = d >> 4;
		dst[i * 2 + 1] = d & 0x0f;
	}
}

void K051316Init(int chip, unsigned char *gfx, unsigned char *gfxexp, int mask, void (*callback)(int *code,int *color,int *flags), int bpp, int transp)
{
	K051316Ram[chip] = (unsigned char*)malloc(0x800);
	K051316TileMap[chip] = (unsigned short*)malloc(((32 * 16) * (32 * 16)) * sizeof(short));

	K051316Callback[chip] = callback;	

	K051316Depth[chip] = bpp;
	K051316Gfx[chip] = gfx;
	K051316GfxExp[chip] = gfxexp;

	K051316Mask[chip] = mask;

	if (bpp == 4) {
		K051316GfxExpand(gfx, gfxexp, mask+1);
	}

	KonamiIC_K051316InUse = 1;

	K051316Offs[chip][0] = K051316Offs[chip][1] = 0;

	K051316TransColor[chip] = transp;
}

void K051316Reset()
{
	for (int i = 0; i < 3; i++)
	{
		if (K051316Ram[i]) {
			memset (K051316Ram[i], 0, 0x800);
		}

		memset (K051316Ctrl[i], 0, 16);

		K051316Wrap[i] = 0;

		if (K051316TileMap[i]) {
			memset (K051316TileMap[i], 0, (32 * 16) * (32 * 16) * sizeof(short));
		}
	}
}

void K051316Exit()
{
	for (int i = 0; i < 3; i++)
	{
		if (K051316Ram[i]) {
			free (K051316Ram[i]);
		}
		K051316Ram[i] = NULL;

		if (K051316TileMap[i]) {
			free (K051316TileMap[i]);
		}
		K051316TileMap[i] = NULL;

		K051316Callback[i] = NULL;
	}
}

void K051316SetOffset(int chip, int xoffs, int yoffs)
{
	K051316Offs[chip][0] = xoffs;
	K051316Offs[chip][1] = yoffs;
}

unsigned char K051316ReadRom(int chip, int offset)
{
	if ((K051316Ctrl[chip][0x0e] & 0x01) == 0)
	{
		int addr = offset + (K051316Ctrl[chip][0x0c] << 11) + (K051316Ctrl[chip][0x0d] << 19);
		if (K051316Depth[chip] <= 4) addr /= 2;
		addr &= K051316Mask[chip];

		return K051316Gfx[chip][addr];
	}

	return 0;
}

unsigned char K051316Read(int chip, int offset)
{
	return K051316Ram[chip][offset];
}

static inline void K051316_write_tile(int offset, int chip)
{
	offset &= 0x3ff;

	int sx = (offset & 0x1f) << 4;
	int sy = (offset >> 5) << 4;

	int code = K051316Ram[chip][offset];
	int color = K051316Ram[chip][offset + 0x400];
	int flags = 0;

	(*K051316Callback[chip])(&code,&color,&flags);

	unsigned char *src = K051316GfxExp[chip] + (code * 16 * 16);
	unsigned short *dst;

	color <<= K051316Depth[chip];

	int flipx = flags & 1;
	int flipy = flags & 2;
	if (flipx) flipx = 0x0f;
	if (flipy) flipy = 0x0f;

	for (int y = 0; y < 16; y++, sy++)
	{
		dst = K051316TileMap[chip] + ((sy << 9) + sx);

		for (int x = 0; x < 16; x++)
		{
			int pxl = src[((y^flipy) << 4) | (x ^ flipx)];

			if (pxl != K051316TransColor[chip]) {
				dst[x] = color | pxl;
			} else {
				dst[x] = 0x8000 | color | pxl;
			}
		}
	}
}

void K051316Write(int chip, int offset, int data)
{
	K051316Ram[chip][offset] = data;
	K051316_write_tile(offset & 0x3ff, chip);
}

void K051316WriteCtrl(int chip, int offset, int data)
{
	K051316Ctrl[chip][offset & 0x0f] = data;
}

void K051316WrapEnable(int chip, int status)
{
	K051316Wrap[chip] = status;
}

static inline void copy_roz(int chip, unsigned int startx, unsigned int starty, int incxx, int incxy, int incyx, int incyy, int wrap, int transp)
{
	unsigned short *dst = pTransDraw;
	unsigned short *src = K051316TileMap[chip];

	unsigned int hshift = 512 << 16;
	unsigned int wshift = 512 << 16;

	for (int sy = 0; sy < nScreenHeight; sy++, startx+=incyx, starty+=incyy)
	{
		unsigned int cx = startx;
		unsigned int cy = starty;

		if (wrap) {
			if (transp) {
				for (int x = 0; x < nScreenWidth; x++, cx+=incxx, cy+=incxy, dst++)
				{
					int pxl = src[(((cy >> 16) & 0x1ff) << 9) | ((cx >> 16) & 0x1ff)];
			
					if (!(pxl & 0x8000)) {
						*dst = pxl;
					}
				}
			} else {
				for (int x = 0; x < nScreenWidth; x++, cx+=incxx, cy+=incxy, dst++) {
					*dst = src[(((cy >> 16) & 0x1ff) << 9) | ((cx >> 16) & 0x1ff)] & 0x7fff;
				}
			}
		} else {
			if (transp) {
				for (int x = 0; x < nScreenWidth; x++, cx+=incxx, cy+=incxy, dst++)
				{
					if (cx < wshift && cy < hshift) {
						int pxl = src[(((cy >> 16) & 0x1ff) << 9) | ((cx >> 16) & 0x1ff)];
						if (!(pxl & 0x8000)) {
							*dst = pxl;
						}
					}
				}
			} else {
				for (int x = 0; x < nScreenWidth; x++, cx+=incxx, cy+=incxy, dst++)
				{
					unsigned int pos = ((cy >> 16) << 9) | (cx >> 16);

					if (pos >= 0x40000) continue;

					*dst = src[pos] & 0x7fff;
				}
			}
		}
	}
}

void K051316_zoom_draw(int chip, int /*flags*/)
{
	unsigned int startx,starty;
	int incxx,incxy,incyx,incyy;

	startx = 256 * ((short)(256 * K051316Ctrl[chip][0x00] + K051316Ctrl[chip][0x01]));
	incxx  =        (short)(256 * K051316Ctrl[chip][0x02] + K051316Ctrl[chip][0x03]);
	incyx  =        (short)(256 * K051316Ctrl[chip][0x04] + K051316Ctrl[chip][0x05]);
	starty = 256 * ((short)(256 * K051316Ctrl[chip][0x06] + K051316Ctrl[chip][0x07]));
	incxy  =        (short)(256 * K051316Ctrl[chip][0x08] + K051316Ctrl[chip][0x09]);
	incyy  =        (short)(256 * K051316Ctrl[chip][0x0a] + K051316Ctrl[chip][0x0b]);

	startx -= (16 + K051316Offs[chip][1]) * incyx;
	starty -= (16 + K051316Offs[chip][1]) * incyy;

	startx -= (89 + K051316Offs[chip][0]) * incxx;
	starty -= (89 + K051316Offs[chip][0]) * incxy;

	copy_roz(chip, startx << 5,starty << 5,incxx << 5,incxy << 5,incyx << 5,incyy << 5, K051316Wrap[chip], K051316TransColor[chip]+1); // transp..
}

void K051316RedrawTiles(int chip)
{
	if (K051316Ram[chip]) {
		for (int j = 0; j < 0x400; j++) {
			K051316_write_tile(j, chip);
		}
	}
}

void K051316Scan(int nAction)
{
	struct BurnArea ba;
	
	if (nAction & ACB_MEMORY_RAM) {
		for (int i = 0; i < 3; i++) {
			if (K051316Ram[i]) {
				memset(&ba, 0, sizeof(ba));
				ba.Data	  = K051316Ram[i];
				ba.nLen	  = 0x800;
				ba.szName = "K052109 Ram";
				BurnAcb(&ba);
			}

			memset(&ba, 0, sizeof(ba));
			ba.Data	  = K051316Ctrl[i];
			ba.nLen	  = 0x010;
			ba.szName = "K052109 Control";
			BurnAcb(&ba);	
		}
	}

	if (nAction & ACB_DRIVER_DATA) {
		SCAN_VAR(K051316Wrap[0]);
		SCAN_VAR(K051316Wrap[1]);
		SCAN_VAR(K051316Wrap[2]);
	}

	if (nAction & ACB_WRITE) {
		K051316RedrawTiles(0);
		K051316RedrawTiles(1);
		K051316RedrawTiles(2);
	}
}
