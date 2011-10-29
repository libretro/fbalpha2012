
#include "tiles_generic.h"
#include "math.h"

#define TABLE_SIZE	0x10000 // excessive?

struct vector_line {
	int x;
	int y;
	int color;
	unsigned char intensity;
};

static struct vector_line *vector_table;
struct vector_line *vector_ptr; // pointer
static int vector_cnt;

void vector_add_point(int x, int y, int color, int intensity)
{
	vector_ptr->x = x >> 16;
	vector_ptr->y = y >> 16;
	vector_ptr->color = color;
	vector_ptr->intensity = intensity;

	vector_cnt++;
	if (vector_cnt > (TABLE_SIZE - 2)) return;
	vector_ptr++;
	vector_ptr->color = -1; // mark it as the last one to save some cycles later...
}

static void lineSimple(int x0, int y0, int x1, int y1, int color, int intensity)
{
 	int dx = x1 - x0;
	int dy = y1 - y0;

	color = color * 256 + intensity;

	if (!dx && dy) // vertical line
	{
		int sy = (y1 < y0) ? y1 : y0;
		int y2 = (y1 < y0) ? (y0 - y1) : (y1 - y0);

		if (x1 >= 0 && x1 < nScreenWidth)
		{
			unsigned short *dst = pTransDraw + x1;

			for (int y = 0; y < y2; y++, sy++) {
				if (sy >= 0 && sy < nScreenHeight) {
					dst[sy * nScreenWidth] = color;
				}
			}
		}
	}
	else if (!dy && dx) // horizontal line
	{
		int sx = (x1 < x0) ? x1 : x0;
		int x2 = (x1 < x0) ? (x0 - x1) : (x1 - x0);

		if (y1 >= 0 && y1 < nScreenHeight)
		{
			unsigned short *dst = pTransDraw + y1 * nScreenWidth;

			for (int x = 0; x < x2; x++, sx++) {
				if (sx >= 0 && sx < nScreenWidth) {
					dst[sx] = color;
				}
			}
		}
	}
	else if (dx && dy) // can we optimize further?
	{
		int md = (dy << 16) / dx;
		int zd = (y0 << 16) - (md * x0) + 0x8000; // + 0x8000 for rounding!

		dx = (x1 > x0) ? 1 : -1;

		while (x0 != x1) {
			x0 += dx;
			y0 =  ((md * x0) + zd) >> 16;

			if (x0 >= 0 && x0 < nScreenWidth && y0 >= 0 && y0 < nScreenHeight) {
				pTransDraw[y0 * nScreenWidth + x0] = color;
			}
		}
	}
	else // point
	{
		if (x0 >= 0 && x0 < nScreenWidth && y0 >= 0 && y0 < nScreenHeight) {
			pTransDraw[y0 * nScreenWidth + x0] = color;
		}
	}
}

void draw_vector(unsigned int *palette)
{
	struct vector_line *ptr = &vector_table[0];

	int prev_x = 0, prev_y = 0;

	BurnTransferClear();

	for (int i = 0; i < vector_cnt && i < TABLE_SIZE; i++, ptr++)
	{
		if (ptr->color == -1) break;

		int curr_y = ptr->y;
		int curr_x = ptr->x;

		if (ptr->intensity != 0) { // intensity 0 means turn off the beam...
			lineSimple(curr_x, curr_y, prev_x, prev_y, ptr->color, ptr->intensity);
		}

		prev_x = curr_x;
		prev_y = curr_y;
	}

	BurnTransferCopy(palette);
}

void vector_reset()
{
	vector_cnt = 0;
	vector_ptr = &vector_table[0];
	vector_ptr->color = -1;
}

void vector_init()
{
	GenericTilesInit();

	vector_table = (struct vector_line*)malloc(TABLE_SIZE * sizeof(vector_line));

	memset (vector_table, 0, TABLE_SIZE * sizeof(vector_line));

	vector_reset();
}

void vector_exit()
{
	GenericTilesExit();

	free (vector_table);
	vector_table = NULL;
	vector_ptr = NULL;
}

int vector_scan(int nAction)
{
	struct BurnArea ba;

	if (nAction & ACB_VOLATILE) {		
		memset(&ba, 0, sizeof(ba));

		ba.Data	  = (unsigned char*)vector_table;
		ba.nLen	  = TABLE_SIZE * sizeof(vector_line);
		ba.szName = "Vector Table";
		BurnAcb(&ba);

		SCAN_VAR(vector_cnt);
	}

	if (nAction & ACB_WRITE) {
		vector_ptr = &vector_table[vector_cnt];
	}

	return 0;
}
