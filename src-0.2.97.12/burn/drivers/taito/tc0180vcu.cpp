#include "tiles_generic.h"
#include "taito_ic.h"
#include "taito.h"

unsigned char *TC0180VCURAM;
unsigned char *TC0180VCUScrollRAM;
unsigned char *TC0180VCUFbRAM; // framebuffer ram

static unsigned short *TC0180VCUFramebuffer[2];

static int TC0180VCU_y_offset;
static int TC0180VCU_x_offset;

static unsigned char TC0180VCUControl[0x10];

static unsigned char *tiledata[2];
static unsigned int tilemask[2];
static unsigned char *transtiletab[2];

static unsigned char *dummy_tile = NULL;

static int *TC0180VCU_scrollx[2];
static int *TC0180VCU_scrolly[2];

static int flipscreen;
static int framebuffer_page;

void TC0180VCUFramebufferWrite(int offset)
{
	offset &= 0x3fffe;
	int data = *((unsigned short*)(TC0180VCUFbRAM + offset));

	int fb = (offset >> 17) & 1;

	offset &= 0x1fffe;

	TC0180VCUFramebuffer[fb][offset + 0] = data >> 8;
	TC0180VCUFramebuffer[fb][offset + 1] = data & 0xff;
}

unsigned short TC0180VCUFramebufferRead(int offset)
{
	offset &= 0x3fffe;

	int fb = (offset >> 17) & 1;

	offset &= 0x1fffe;

	return (TC0180VCUFramebuffer[fb][offset + 0] << 8) | (TC0180VCUFramebuffer[fb][offset + 1] & 0xff);
}

unsigned char TC0180VCUReadControl()
{
	return TC0180VCUControl[7];
}

unsigned char TC0180VCUReadRegs(int offset)
{
	offset >>= 1;
	offset &= 0x0f;

	return TC0180VCUControl[offset];
}

void TC0180VCUWriteRegs(int offset, int data)
{
	offset >>= 1;
	offset &= 0x0f;

	TC0180VCUControl[offset] = data;

	if (offset == 7) {
		if (data & 0x80) {
			framebuffer_page = (data & 0x40) ? 0 : 1;
		}
	}
}

void TC0180VCUReset()
{
	for (int i = 0; i < 2; i++) {

		memset (TC0180VCUFramebuffer[i], 0, 512 * 256 * sizeof(short));
		memset (TC0180VCU_scrollx[i], 0, 256 * sizeof(int));
		memset (TC0180VCU_scrolly[i], 0, 256 * sizeof(int));
	}

	memset (TC0180VCUControl, 	0, 16);
	memset (TC0180VCURAM,		0, 0x010000);
	memset (TC0180VCUScrollRAM,	0, 0x000800);
	memset (TC0180VCUFbRAM,		0, 0x040000);

	flipscreen = 0;
	framebuffer_page = 0;
}

static void create_transtile_table(int tile)
{
	int size = (tile) ? (16 * 16) : (8 * 8);

	if (tilemask[tile]) {
		int len = (tilemask[tile] + 1);

		transtiletab[tile] = (unsigned char*)malloc(len);

		memset (transtiletab[tile], 1, len);

		for (int i = 0; i < len * size; i++)
		{
			if (tiledata[tile][i]) {
				transtiletab[tile][i / size] = 0;
				i|=(size-1);
			}
		}
	}
}

void TC0180VCUInit(unsigned char *gfx0, int mask0, unsigned char *gfx1, int mask1, int global_x, int global_y)
{
	TaitoIC_TC0180VCUInUse = 1;

	for (int i = 0; i < 2; i++)
	{
		TC0180VCUFramebuffer[i] = (unsigned short*)malloc(512 * 256 * sizeof(short));
		TC0180VCU_scrollx[i] = (int*)malloc(257 * sizeof(int));
		TC0180VCU_scrolly[i] = (int*)malloc(257 * sizeof(int));
	}

	TC0180VCURAM		= (unsigned char*)malloc(0x010000);
	TC0180VCUScrollRAM	= (unsigned char*)malloc(0x000800);
	TC0180VCUFbRAM		= (unsigned char*)malloc(0x040000);

	tilemask[0] = mask0;
	tilemask[1] = mask1;
	tiledata[0] = gfx0;
	tiledata[1] = gfx1;

	if (mask0) create_transtile_table(0);
	if (mask1) create_transtile_table(1);

	if (mask0 == 0) {
		dummy_tile = (unsigned char*)malloc(0x100);
		transtiletab[1] = (unsigned char*)malloc(1);
		tiledata[1] = dummy_tile;
	}

	TC0180VCU_y_offset = global_y;
	TC0180VCU_x_offset = global_x;

	TC0180VCUReset();
}

void TC0180VCUExit()
{
	for (int i = 0; i < 2; i++)
	{
		if (TC0180VCU_scrollx[i]) {
			free (TC0180VCU_scrollx[i]);
			TC0180VCU_scrollx[i] = NULL;
		}

		if (TC0180VCU_scrolly[i]) {
			free (TC0180VCU_scrolly[i]);
			TC0180VCU_scrolly[i] = NULL;
		}

		tilemask[i] = ~0;
		tiledata[i] = NULL;

		if (TC0180VCUFramebuffer[i]) {
			free (TC0180VCUFramebuffer[i]);
			TC0180VCUFramebuffer[i] = NULL;
		}

		if (transtiletab[i]) {
			free (transtiletab[i]);
			transtiletab[i] = NULL;
		}
	}

	if (dummy_tile) {
		free (dummy_tile);
		dummy_tile = NULL;
	}

	if (TC0180VCURAM) {
		free (TC0180VCURAM);
		TC0180VCURAM = NULL;
	}

	if (TC0180VCUScrollRAM) {
		free (TC0180VCUScrollRAM);
		TC0180VCUScrollRAM = NULL;
	}

	if (TC0180VCUFbRAM) {
		free (TC0180VCUFbRAM);
		TC0180VCUFbRAM = NULL;
	}

	TC0180VCU_y_offset = 0;
	TC0180VCU_x_offset = 0;
}

static void update_scroll(int plane)
{
	flipscreen = TC0180VCUReadControl() & 0x10;

	unsigned short *scrollram = (unsigned short*)TC0180VCUScrollRAM;

	int lines_per_block = 256 - TC0180VCUControl[2 + plane];
	int number_of_blocks = 256 / lines_per_block;

	for (int i = 0; i < number_of_blocks; i++)
	{
		int scrollx = scrollram[plane * 0x200 + i * 2 * lines_per_block + 0];
		int scrolly = scrollram[plane * 0x200 + i * 2 * lines_per_block + 1];

		int min_y = (i + 0) * lines_per_block - 0;
		int max_y = (i + 1) * lines_per_block - 1;

		if (min_y <= max_y)
		{
			for (int y = min_y; y <= max_y; y++) {
				TC0180VCU_scrollx[plane][y] = -(scrollx & 0x3ff);
				TC0180VCU_scrolly[plane][y] = -(scrolly & 0x3ff);
			}
		}
	}
}

void TC0180VCUDrawLayer(int colorbase, int ctrl_offset, int transparent) // 0, -1
{
	update_scroll(ctrl_offset);

	unsigned short *ram = (unsigned short*)TC0180VCURAM;

	int bank0 = (TC0180VCUControl[ctrl_offset] << 12) & 0xf000; // tile bank
	int bank1 = (TC0180VCUControl[ctrl_offset] <<  8) & 0xf000; // color bank

	int *scroll_x = TC0180VCU_scrollx[ctrl_offset];
	int *scroll_y = TC0180VCU_scrolly[ctrl_offset];

	int lines = TC0180VCUControl[2 + ctrl_offset];

	if (lines)
	{
		int screen_width = nScreenWidth - 1;
		int screen_height = nScreenHeight - 1;

		unsigned short *dest;

		for (int sy = 0; sy < nScreenHeight; sy++)
		{
			if (flipscreen) {
				dest = pTransDraw + (screen_height - sy) * nScreenWidth;
			} else {
				dest = pTransDraw + sy * nScreenWidth;
			}

			int scly = (sy + scroll_y[(sy + TC0180VCU_y_offset) & 0xff] + TC0180VCU_y_offset) & 0x3ff;

			int scly_off = (scly >> 4) << 6;
			int scly_ts  = (scly & 0x0f) << 4;

			int sclx_base = scroll_x[(sy + TC0180VCU_y_offset) & 0xff] + TC0180VCU_x_offset;

			for (int sx = 0; sx < nScreenWidth + 16; sx+=16)
			{
				int sclx = (sx + sclx_base) & 0x3ff;

				int offs = scly_off | (sclx >> 4);

				int attr = ram[offs + bank1];
				int code = ram[offs + bank0];
				int color = (attr & 0x003f) + colorbase;
				code &= tilemask[1];

				if (!transparent) {
					if (transtiletab[1][code]) continue;
				}

				{
					int sx4 = sx - (sclx & 0x0f);

					color <<= 4;
					unsigned char *src = tiledata[1] + code * 256;

					if (attr & 0x80) {			// flipy
						src += (scly_ts ^ 0xf0);
					} else {
						src += (scly_ts);
					}

					int flipx = ((attr & 0x40) >> 6) * 0x0f;

					if (flipscreen) {
						if (!transparent) { // transparency
							for (int sxx = 0; sxx < 16; sxx++, sx4++) {
								if (sx4 >= nScreenWidth || sx4 < 0) continue;
			
								int pxl = src[sxx ^ flipx];
		
								if (pxl != transparent) {
									dest[(screen_width - sx4)] = pxl | color;
								}
							}
						} else {
							for (int sxx = 0; sxx < 16; sxx++, sx4++) {
								if (sx4 >= nScreenWidth || sx4 < 0) continue;

								dest[(screen_width - sx4)] = src[sxx ^ flipx] | color;
							}
						}
					} else {
						if (!transparent) { // transparency
							for (int sxx = 0; sxx < 16; sxx++, sx4++) {
								if (sx4 >= nScreenWidth || sx4 < 0) continue;
			
								int pxl = src[sxx ^ flipx];
		
								if (pxl != transparent) {
									dest[sx4] = pxl | color;
								}
							}
						} else {
							for (int sxx = 0; sxx < 16; sxx++, sx4++) {
								if (sx4 >= nScreenWidth || sx4 < 0) continue;

								dest[sx4] = src[sxx ^ flipx] | color;
							}
						}
					}
				}
			}
		}
	}
	else
	{
		for (int offs = 0; offs < 64 * 64; offs++)
		{
			int sx = (offs & 0x3f) << 4;
			int sy = (offs >> 6) << 4;
	
			sy -= scroll_y[(sy + TC0180VCU_y_offset) & 0xff];
			if (sy >= 0x400-15) sy -= 0x400;

			sx -= scroll_x[(sy + TC0180VCU_y_offset) & 0xff];
			if (sx >= 0x400-15) sx -= 0x400;

			int attr  = ram[offs + bank1];
			int code  = ram[offs + bank0];
	
			int color = (attr & 0x003f) + colorbase;
			int flipx = (attr & 0x0040);
			int flipy = (attr & 0x0080);
	
			code &= tilemask[1];

			if (flipscreen) {
				sx = (nScreenWidth - 16) - sx;
				sy = 240 - sy;
				flipx ^= 0x40;
				flipy ^= 0x80;
			}

			sx -= TC0180VCU_x_offset;
			sy -= TC0180VCU_y_offset;

			if (!transparent) {
				if (transtiletab[1][code]) continue;

				if (sy >= 0 && sy < (nScreenHeight - 15) && sx >= 0 && sx < (nScreenWidth - 15)) {
					if (flipy) {
						if (flipx) {
							Render16x16Tile_Mask_FlipXY(pTransDraw, code, sx, sy, color, 4, 0, 0, tiledata[1]);
						} else {
							Render16x16Tile_Mask_FlipY(pTransDraw, code, sx, sy, color, 4, 0, 0, tiledata[1]);
						}
					} else {
						if (flipx) {
							Render16x16Tile_Mask_FlipX(pTransDraw, code, sx, sy, color, 4, 0, 0, tiledata[1]);
						} else {
							Render16x16Tile_Mask(pTransDraw, code, sx, sy, color, 4, 0, 0, tiledata[1]);
						}
					}
				} else {
					if (flipy) {
						if (flipx) {
							Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, code, sx, sy, color, 4, 0, 0, tiledata[1]);
						} else {
							Render16x16Tile_Mask_FlipY_Clip(pTransDraw, code, sx, sy, color, 4, 0, 0, tiledata[1]);
						}
					} else {
						if (flipx) {
							Render16x16Tile_Mask_FlipX_Clip(pTransDraw, code, sx, sy, color, 4, 0, 0, tiledata[1]);
						} else {
							Render16x16Tile_Mask_Clip(pTransDraw, code, sx, sy, color, 4, 0, 0, tiledata[1]);
						}
					}
				}
			} else {
				if (sy >= 0 && sy < (nScreenHeight - 15) && sx >= 0 && sx < (nScreenWidth - 15)) {
					if (flipy) {
						if (flipx) {
							Render16x16Tile_FlipXY(pTransDraw, code, sx, sy, color, 4, 0, tiledata[1]);
						} else {
							Render16x16Tile_FlipY(pTransDraw, code, sx, sy, color, 4, 0, tiledata[1]);
						}
					} else {
						if (flipx) {
							Render16x16Tile_FlipX(pTransDraw, code, sx, sy, color, 4, 0, tiledata[1]);
						} else {
							Render16x16Tile(pTransDraw, code, sx, sy, color, 4, 0, tiledata[1]);
						}
					}
				} else {
					if (flipy) {
						if (flipx) {
							Render16x16Tile_FlipXY_Clip(pTransDraw, code, sx, sy, color, 4, 0, tiledata[1]);
						} else {
							Render16x16Tile_FlipY_Clip(pTransDraw, code, sx, sy, color, 4, 0, tiledata[1]);
						}
					} else {
						if (flipx) {
							Render16x16Tile_FlipX_Clip(pTransDraw, code, sx, sy, color, 4, 0, tiledata[1]);
						} else {
							Render16x16Tile_Clip(pTransDraw, code, sx, sy, color, 4, 0, tiledata[1]);
						}
					}
				}
			}
		}
	}
}

void TC0180VCUDrawCharLayer(int colorbase)
{
	if (tilemask[0] == 0) return;

	unsigned short *ram = (unsigned short*)TC0180VCURAM;

	int bank0 = (TC0180VCUControl[6] & 0x0f) << 11; // tile bank

	for (int offs = 0; offs < 64 * 32; offs++)
	{
		int sx = (offs & 0x3f) << 3;
		int sy = (offs >> 6) << 3;

		int code  = ram[offs + bank0];
		int color = (code >> 12) + colorbase;

		code = (code & 0x07ff) | (TC0180VCUControl[4 + ((code & 0x800) >> 11)] << 11);

		code &= tilemask[0];

		if (transtiletab[0][code]) continue;

		if (flipscreen) {
			sx = (nScreenWidth - 8) - sx;
			sy = 248 - sy;

			Render8x8Tile_Mask_FlipXY_Clip(pTransDraw, code, sx - TC0180VCU_x_offset, sy - TC0180VCU_y_offset, color, 4, 0, 0, tiledata[0]);
		} else {
			Render8x8Tile_Mask_Clip(pTransDraw, code, sx - TC0180VCU_x_offset, sy - TC0180VCU_y_offset, color, 4, 0, 0, tiledata[0]);
		}
	}
}

void TC0180VCUFramebufferDraw(int priority, int color_base)
{
	priority <<= 4;

	int ctrl = TC0180VCUReadControl();

	if (ctrl & 0x08)
	{
		if (ctrl & 0x10)	// flip screen
		{
			for (int y = 0; y < nScreenHeight; y++)
			{
				unsigned short *src = TC0180VCUFramebuffer[framebuffer_page & 1] + (y + TC0180VCU_y_offset) * 512 + TC0180VCU_x_offset;
				unsigned short *dst = pTransDraw + ((nScreenHeight - 1) - y) * nScreenWidth + (nScreenWidth - 1);

				for (int x = 0; x < nScreenWidth; x++)
				{
					int c = *src++;

					if (c != 0) *dst = color_base + c;
					dst--;
				}
			}
		}
		else
		{
			for (int y = 0; y < nScreenHeight; y++)
			{
				unsigned short *src = TC0180VCUFramebuffer[framebuffer_page & 1] + (y + TC0180VCU_y_offset) * 512 + TC0180VCU_x_offset;
				unsigned short *dst = pTransDraw + y * nScreenWidth;

				for (int x = 0; x < nScreenWidth; x++)
				{
					int c = *src++;

					if (c != 0) *dst = color_base + c;
					dst++;
				}
			}
		}
	}
	else
	{
		if (ctrl & 0x10)   // flip screen
		{
			for (int y = 0; y < nScreenHeight; y++)
			{
				unsigned short *src = TC0180VCUFramebuffer[framebuffer_page & 1] + (y + TC0180VCU_y_offset) * 512 + TC0180VCU_x_offset;
				unsigned short *dst = pTransDraw + ((nScreenHeight - 1) - y) * nScreenWidth + (nScreenWidth - 1);

				for (int x = 0; x < nScreenWidth; x++)
				{
					int c = *src++;

					if (c != 0 && (c & 0x10) == priority)
						*dst = color_base + c;
					dst--;
				}
			}
		}
    		else
		{
			for (int y = 0; y < nScreenHeight; y++)
			{
				unsigned short *src = TC0180VCUFramebuffer[framebuffer_page & 1] + (y + TC0180VCU_y_offset) * 512 + TC0180VCU_x_offset;
				unsigned short *dst = pTransDraw + y * nScreenWidth;

				for (int x = 0; x < nScreenWidth; x++)
				{
					int c = *src++;

					if (c != 0 && (c & 0x10) == priority)
						*dst = color_base + c;
					dst++;
				}
			}
		}
	}
}

void TC0180VCUDrawSprite(unsigned short *dest)
{
	int t_swide = nScreenWidth;  nScreenWidth  = 512; // hack to allow use of generic tile routines
	int t_shigh = nScreenHeight; nScreenHeight = 256;

	int xlatch = 0;
	int ylatch = 0;
	int x_no = 0;
	int y_no = 0;
	int x_num = 0;
	int y_num = 0;
	int big_sprite = 0;
	unsigned int zoomx;
	unsigned int zoomy;
	unsigned int zx;
	unsigned int zy;
	unsigned int zoomxlatch = 0;
	unsigned int zoomylatch = 0;

	unsigned short *ram = (unsigned short*)TaitoSpriteRam;

	for (int offs = (0x1980 - 16) / 2; offs >=0; offs -= 8)
	{
		int code  = ram[offs + 0] & tilemask[1];
		int color = ram[offs + 1];
		int x     = ram[offs + 2] & 0x03ff;
		int y     = ram[offs + 3] & 0x03ff;

		int data  = ram[offs + 5];

		int flipx = color & 0x4000;
		int flipy = color & 0x8000;

		if (x >= 0x200) x -= 0x400;
		if (y >= 0x200) y -= 0x400;

		if (data)
		{
			if (!big_sprite)
			{
				x_num  = (data >> 8) & 0xff;
				y_num  = (data >> 0) & 0xff;
				x_no   = 0;
				y_no   = 0;
				xlatch = x;
				ylatch = y;
				data   = ram[offs + 4];
				zoomxlatch = (data >> 8) & 0xff;
				zoomylatch = (data >> 0) & 0xff;
				big_sprite = 1;
			}
		}

		data = ram[offs + 4];
		zoomx = (data >> 8) & 0xff;
		zoomy = (data >> 0) & 0xff;
		zx = (0x100 - zoomx) / 16;
		zy = (0x100 - zoomy) / 16;

		if (big_sprite)
		{
			zoomx = zoomxlatch;
			zoomy = zoomylatch;

			x = xlatch + x_no * (0x100 - zoomx) / 16;
			y = ylatch + y_no * (0x100 - zoomy) / 16;
			zx = xlatch + (x_no + 1) * (0x100 - zoomx) / 16 - x;
			zy = ylatch + (y_no + 1) * (0x100 - zoomy) / 16 - y;
			y_no++;

			if (y_no > y_num)
			{
				y_no = 0;
				x_no++;

				if (x_no > x_num) big_sprite = 0;
			}
		}

		if (zoomx || zoomy )
		{
			RenderZoomedTile(dest, tiledata[1], code, (color & 0x3f) << 4, 0, x, y, flipx, flipy, 16, 16, zx << 12, zy << 12);
		}
		else
		{
			if (flipy) {
				if (flipx) {
					Render16x16Tile_Mask_FlipXY_Clip(dest, code, x, y, color & 0x3f, 4, 0, 0, tiledata[1]);
				} else {
					Render16x16Tile_Mask_FlipY_Clip(dest, code, x, y, color & 0x3f, 4, 0, 0, tiledata[1]);
				}
			} else {
				if (flipx) {
					Render16x16Tile_Mask_FlipX_Clip(dest, code, x, y, color & 0x3f, 4, 0,0, tiledata[1]);
				} else {
					Render16x16Tile_Mask_Clip(dest, code, x, y, color & 0x3f, 4, 0, 0, tiledata[1]);
				}
			}
		}
	}

	nScreenWidth  = t_swide;
	nScreenHeight = t_shigh;
}

void TC0180VCUBufferSprites()
{
	int ctrl = TC0180VCUReadControl();

	if (~ctrl & 0x01) {
		memset (TC0180VCUFbRAM + framebuffer_page * 0x20000, 0, 512 * 256);
		memset (TC0180VCUFramebuffer[framebuffer_page], 0, 512 * 256 * sizeof(short));
	}

	if (~ctrl & 0x80) {
		framebuffer_page ^= 1;
	}

	if (tilemask[1]) {
		TC0180VCUDrawSprite(TC0180VCUFramebuffer[framebuffer_page]);
	}
}

void TC0180VCUScan(int nAction)
{
	struct BurnArea ba;

	if (nAction & ACB_VOLATILE)
	{
	bprintf (0, _T("yup\n"));

		ba.Data	  = (unsigned char*)TC0180VCUFramebuffer[0];
		ba.nLen	  = 512 * 256 * sizeof(short);
		ba.szName = "Framebuffer 0";
		BurnAcb(&ba);

		ba.Data	  = (unsigned char*)TC0180VCUFramebuffer[1];
		ba.nLen	  = 512 * 256 * sizeof(short);
		ba.szName = "Framebuffer 1";
		BurnAcb(&ba);

		ba.Data	  = TC0180VCURAM;
		ba.nLen	  = 0x10000;
		ba.szName = "Tilemap RAM";
		BurnAcb(&ba);

		ba.Data	  = TC0180VCUScrollRAM;
		ba.nLen	  = 0x00800;
		ba.szName = "Scroll RAM";
		BurnAcb(&ba);

		ba.Data	  = TC0180VCUFbRAM;
		ba.nLen	  = 0x00800;
		ba.szName = "Framebuffer RAM";
		BurnAcb(&ba);

		ba.Data	  = TC0180VCUControl;
		ba.nLen	  = 0x00010;
		ba.szName = "Control RAM";
		BurnAcb(&ba);

		SCAN_VAR(framebuffer_page);
	}
}
