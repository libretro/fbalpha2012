
#include "effect.h"
#include "burner.h"

#ifndef M_PI
#ifndef PI
#define M_PI 3.1415926535897932384626433832795
#else
#define M_PI PI
#endif
#endif


#define ROOTRAND     20   // max/min decrease of the root of the flames
#define DECAY        5    // how far should the flames go up on the screen?
#define MINY         0    // startingline of the flame routine (should be adjusted along with MinY above)
#define SMOTH        1    // how descrete can the flames be?
#define MINFIRE      50   // limit between the "starting to burn" and the "is burning" routines
#define XSTART       4    // startingpos on the screen, should be divideable by 4 without remain!*/
#define XEND         287
#define WIDTH        (XEND - XSTART + 1)   // +xend-xstart
#define FIREINCREASE 3    // 3 = Wood, 90 = Gazolin

WORD pal[256];

void genpal()
{
	int i;

	pal[0] = pal[1] = 0;
	for (i = 0; i < 8; i++)
		pal[i + 2] = MAKE_COLOR(i * 4, 0, 0);
	for ( ; i < 16; i++)
		pal[i + 2] = MAKE_COLOR(31, (i * 4) - 32, 0);
	for ( ; i < 24; i++)
		pal[i + 2] = MAKE_COLOR(31, 31, (i * 4) - 32);
	for (i += 2; i < 256; i++)
		pal[i] = MAKE_COLOR(31, 31, 31);
}

int started = 0;

#define randint(a) (rand() % (a))
#define randreal() (((double)rand()) / ((double)RAND_MAX))
#define rand1(a)   ((randint(a * 2 + 1)) - a)

BYTE flamearray[SCRW];
int  morefire = 0;
BYTE vidbuffer[SCRW * SCRH];

// damn, this seems like such a waste
BYTE pt[SCRW * SCRH];

void UpdateBurnEffect()
{
	int i,j;
	int x,p;
	int v;

	if (!started)
	{
		started = 1;
		memset(flamearray, 0, SCRW);
		morefire = 1;
		memset(pt, 0, SCRH * SCRW);
		genpal();
	}

	// put the values from FlameArray on the bottom line of the screen
	memcpy(pt + ((SCRH - 1) * SCRW) + XSTART, flamearray, WIDTH);

	// this loop makes the actual flames

	for (i = XSTART; i <= XEND; i++)
	{
		for (j = MINY; j <= (SCRH-1); j ++)
		{
			v = pt[j*SCRW + i];
			if (!v || v < DECAY || i <= XSTART || i >= XEND)
				pt[(j-1)*SCRW + i] = 0;
			else
				pt[((j-1)*SCRW) + (i-(randint(3)-1))] = v - randint(DECAY);
		}
	}

	if (!randint(150)) memset(flamearray + XSTART + randint(XEND - XSTART - 5), 255, 5);

	// this loop controls the "root" of the flames ie. the values in FlameArray
	for (i = XSTART; i <= XEND; i++)
	{
		x = flamearray[i];
		if (x < MINFIRE)    // increase by the "burnability"
		{
			// starting to burn:
			if (x > 10)  x += randint(FIREINCREASE);
		}
		else // otherwise randomize and increase by intensity (is burning)
			x += rand1(ROOTRAND) + morefire;
		if (x > 255)  x = 255;    // x too large
		flamearray[i] = x;
	}

#if 0
	// pour a little water on both sides of the fire to make it look nice on the sides
	for (i = 1; i <= WIDTH / 8; i ++)
	{
		x = (int)floor(sqrt(randreal())*WIDTH/8);
		flamearray[XSTART + x] = 0;
		flamearray[XEND - x] = 0;
	}
#endif

	// smoothen the values of FrameArray to avoid "descrete" flames
	p=0;
	for (i = XSTART + SMOTH; i <= XEND - SMOTH; i++)
	{
		x=0;
		for (j =- SMOTH; j <= SMOTH; j++) x += flamearray[i+j];
		flamearray[i] = x / ((SMOTH << 1) + 1);
	}

	for (x=0; x < SCRW*SCRH; x++)
	{
		i = vidbuffer[x];
		j = pt[x] >> 3;

		if (j > i) vidbuffer[x] = j;
		else       vidbuffer[x] = ((i + j) >> 1) + 1;

		i =  vidbuffer[x];
		pEffect[x] = pal[i];
	}
}
