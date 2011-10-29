#include "tiles_generic.h"
#include "konamiic.h"

#define MAX_K053936	2

static int nRamLen[MAX_K053936] = { 0, 0 };
static int nWidth[MAX_K053936] = { 0, 0 };
static int nHeight[MAX_K053936] = { 0, 0 };
static unsigned short *tscreen[MAX_K053936] = { NULL, NULL };
static unsigned char *ramptr[MAX_K053936] = { NULL, NULL };
static unsigned char *rambuf[MAX_K053936] = { NULL, NULL };

static int K053936Wrap[MAX_K053936] = { 0, 0 };
static int K053936Offset[MAX_K053936][2] = { { 0, 0 }, { 0, 0 } };

static void (*pTileCallback0)(int offset, unsigned short *ram, int *code, int *color, int *sx, int *sy, int *fx, int *fy);
static void (*pTileCallback1)(int offset, unsigned short *ram, int *code, int *color, int *sx, int *sy, int *fx, int *fy);

void K053936Reset()
{
	for (int i = 0; i < MAX_K053936; i++) {
		if (rambuf[i]) {
			memset (rambuf[i], 0, nRamLen[i]);
		}
	}
}

void K053936Init(int chip, unsigned char *ram, int len, int w, int h, void (*pCallback)(int offset, unsigned short *ram, int *code, int *color, int *sx, int *sy, int *fx, int *fy))
{
	ramptr[chip] = ram;

	nRamLen[chip] = len;

	if (rambuf[chip] == NULL) {
		rambuf[chip] = (unsigned char*)malloc(len);
	}

	nWidth[chip] = w;
	nHeight[chip] = h;

	if (tscreen[chip] == NULL) {
		tscreen[chip] = (unsigned short*)malloc(w * h * 2);
	}

	if (chip == 0) {
		pTileCallback0 = pCallback;
	}
	if (chip == 1) {
		pTileCallback1 = pCallback;
	}

	KonamiIC_K053936InUse = 1;
}

void K053936Exit()
{
	for (int i = 0; i < MAX_K053936; i++) {
		nRamLen[i] = 0;
		nWidth[i] = 0;
		nHeight[i] = 0;
		if (tscreen[i] != NULL) {
			free (tscreen[i]);
			tscreen[i] = NULL;
		}
		ramptr[i] = NULL;
		if (rambuf[i] != NULL) {
			free (rambuf[i]);
			rambuf[i] = NULL;
		}
		K053936Wrap[i] = 0;
		K053936Offset[i][0] = K053936Offset[i][1] = 0;
	}
}

void K053936PredrawTiles(int chip, unsigned char *gfx, int transparent, int tcol)
{
	int twidth = nWidth[chip];
	unsigned short *ram = (unsigned short*)ramptr[chip];
	unsigned short *buf = (unsigned short*)rambuf[chip];

	for (int i = 0; i < nRamLen[chip] / 2; i++)
	{
		if (ram[i] != buf[i]) {
			int sx;
			int sy;
			int code;
			int color;
			int fx;
			int fy;
		
			if (chip) {
				pTileCallback1(i, ram, &code, &color, &sx, &sy, &fx, &fy);
			} else {
				pTileCallback0(i, ram, &code, &color, &sx, &sy, &fx, &fy);
			}
		
			// draw tile
			{
				if (fy) fy  = 0xf0;
				if (fx) fy |= 0x0f;
		
				unsigned char *src = gfx + (code * 16 * 16);
				unsigned short *sdst = tscreen[chip] + (sy * twidth) + sx;
		
				for (int y = 0; y < 16; y++) {
					for (int x = 0; x < 16; x++) {
						int pxl = src[((y << 4) | x) ^ fy];
						if (transparent) {
							if (pxl == tcol) pxl |= 0x8000;
						}
		
						sdst[x] = pxl | color;
					}
					sdst += twidth;
				}
			}
		}
		buf[i] = ram[i];
	}
}

static inline void copy_roz(int chip, int minx, int maxx, int miny, int maxy, unsigned int startx, unsigned int starty, int incxx, int incxy, int incyx, int incyy, int transp)
{
	unsigned short *dst = pTransDraw;
	unsigned short *src = tscreen[chip];

	int hmask = nHeight[chip] - 1;
	int wmask = nWidth[chip] - 1;

	int wrap = K053936Wrap[chip];

	for (int sy = miny; sy < maxy; sy++, startx+=incyx, starty+=incyy)
	{
		unsigned int cx = startx;
		unsigned int cy = starty;

		if (transp) {
			if (wrap) {
				for (int x = minx; x < maxx; x++, cx+=incxx, cy+=incxy, dst++)
				{
					int pxl = src[(((cy >> 16) & hmask) << 10) + ((cx >> 16) & wmask)];
		
					if (!(pxl & 0x8000)) {
						*dst = pxl;
					}
				}
			} else {
				for (int x = minx; x < maxx; x++, cx+=incxx, cy+=incxy, dst++)
				{
					int yy = cy >> 16;
					if (yy > hmask || yy < 0) continue;
					int xx = cx >> 16;
					if (xx > wmask || xx < 0) continue;

					int pxl = src[(yy << 10) + xx];

					if (!(pxl & 0x8000)) {
						*dst = pxl;
					}
				}
			}
		} else {
			if (wrap) {
				for (int x = minx; x < maxx; x++, cx+=incxx, cy+=incxy, dst++) {
					*dst = src[(((cy >> 16) & hmask) << 10) + ((cx >> 16) & wmask)] & 0x7fff;
				}
			} else {
				for (int x = minx; x < maxx; x++, cx+=incxx, cy+=incxy, dst++) {
					int yy = cy >> 16;
					if (yy > hmask || yy < 0) continue;
					int xx = cx >> 16;
					if (xx > wmask || xx < 0) continue;

					*dst = src[(yy << 10) + xx] & 0x7fff;
				}
			}
		}
	}
}

void K053936Draw(int chip, unsigned short *ctrl, unsigned short *linectrl, int transp)
{
	if (ctrl[0x07] & 0x0040 && linectrl)	// Super!
	{
		UINT32 startx,starty;
		int incxx,incxy;

		int minx, maxx, maxy, miny, y;

		if ((ctrl[0x07] & 0x0002) && ctrl[0x09])	// glfgreat
		{
			minx = ctrl[0x08] + K053936Offset[chip][0]+2;
			maxx = ctrl[0x09] + K053936Offset[chip][0]+2 - 1;
			if (minx < 0) minx = 0;
			if (maxx > nScreenWidth) maxx = nScreenWidth;

			y = ctrl[0x0a] + K053936Offset[chip][1]-2;
			if (y < 0) y = 0;
			maxy = ctrl[0x0b] + K053936Offset[chip][1]-2 - 1;
			if (maxy > nScreenHeight) maxy = nScreenHeight;
		}
		else
		{
			minx = 0;
			maxx = nScreenWidth;

			y = 0;
			maxy = nScreenHeight;
		}

		while (y <= maxy)
		{
			UINT16 *lineaddr = linectrl + 4*((y - K053936Offset[chip][1]) & 0x1ff);
			miny = maxy = y;

			startx = 256 * (INT16)(lineaddr[0] + ctrl[0x00]);
			starty = 256 * (INT16)(lineaddr[1] + ctrl[0x01]);
			incxx  =       (INT16)(lineaddr[2]);
			incxy  =       (INT16)(lineaddr[3]);

			if (ctrl[0x06] & 0x8000) incxx *= 256;
			if (ctrl[0x06] & 0x0080) incxy *= 256;

			startx -= K053936Offset[chip][0] * incxx;
			starty -= K053936Offset[chip][0] * incxy;

			copy_roz(chip, minx, maxx, miny, maxy, startx << 5, starty << 5, incxx << 5, incxy << 5, 0, 0, transp);

			y++;
		}
	}
	else	// simple
	{
		UINT32 startx,starty;
		int incxx,incxy,incyx,incyy;

		startx = 256 * (INT16)(ctrl[0x00]);
		starty = 256 * (INT16)(ctrl[0x01]);
		incyx  =       (INT16)(ctrl[0x02]);
		incyy  =       (INT16)(ctrl[0x03]);
		incxx  =       (INT16)(ctrl[0x04]);
		incxy  =       (INT16)(ctrl[0x05]);

		if (ctrl[0x06] & 0x4000) { incyx *= 256; incyy *= 256; }
		if (ctrl[0x06] & 0x0040) { incxx *= 256; incxy *= 256; }

		startx -= K053936Offset[chip][1] * incyx;
		starty -= K053936Offset[chip][1] * incyy;

		startx -= K053936Offset[chip][0] * incxx;
		starty -= K053936Offset[chip][0] * incxy;

		copy_roz(chip, 0, nScreenWidth, 0, nScreenHeight, startx << 5, starty << 5, incxx << 5, incxy << 5, incyx << 5, incyy << 5, transp);
	}
}

void K053936EnableWrap(int chip, int status)
{
	K053936Wrap[chip] = status;
}

void K053936SetOffset(int chip, int xoffs, int yoffs)
{
	K053936Offset[chip][0] = xoffs;
	K053936Offset[chip][1] = yoffs;
}

void K053936Scan(int nAction)
{
	if (nAction & ACB_DRIVER_DATA) {
		SCAN_VAR(K053936Wrap[0]);
		SCAN_VAR(K053936Wrap[1]);
	}
}

