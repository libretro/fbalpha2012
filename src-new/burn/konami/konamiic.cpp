#include "tiles_generic.h"
#include "konamiic.h"

unsigned int KonamiIC_K051960InUse = 0;
unsigned int KonamiIC_K052109InUse = 0;
unsigned int KonamiIC_K051316InUse = 0;
unsigned int KonamiIC_K053245InUse = 0;
unsigned int KonamiIC_K053247InUse = 0;
unsigned int KonamiIC_K053936InUse = 0;

unsigned short *konami_temp_screen = NULL;
int K05324xZRejection = -1;

void K05324xSetZRejection(int z)
{
	K05324xZRejection = z;
}

unsigned char K052109_051960_r(int offset)
{
	if (K052109RMRDLine == 0)
	{
		if (offset >= 0x3800 && offset < 0x3808)
			return K051937Read(offset - 0x3800);
		else if (offset < 0x3c00)
			return K052109Read(offset);
		else
			return K051960Read(offset - 0x3c00);
	}
	else return K052109Read(offset);
}

void K052109_051960_w(int offset, int data)
{
	if (offset >= 0x3800 && offset < 0x3808)
		K051937Write(offset - 0x3800,data);
	else if (offset < 0x3c00)
		K052109Write(offset,         data);
	else
		K051960Write(offset - 0x3c00,data);
}

static void shuffle(unsigned short *buf, int len)
{
	if (len == 2 || len & 3) return;

	len >>= 1;

	for (int i = 0; i < len/2; i++)
	{
		int t = buf[len/2 + i];
		buf[len/2 + i] = buf[len + i];
		buf[len + i] = t;
	}

	shuffle(buf,       len);
	shuffle(buf + len, len);
}

void konami_rom_deinterleave_2(unsigned char *src, int len)
{
	shuffle((unsigned short*)src,len/2);
}

void konami_rom_deinterleave_4(unsigned char *src, int len)
{
	konami_rom_deinterleave_2(src, len);
	konami_rom_deinterleave_2(src, len);
}

// xbbbbbgggggrrrrr (used mostly by Konami-custom cpu games)
void KonamiRecalcPal(unsigned char *src, unsigned int *dst, int len)
{
	unsigned char r,g,b;
	unsigned short *p = (unsigned short*)src;
	for (int i = 0; i < len / 2; i++) {
		unsigned short d = (p[i] << 8) | (p[i] >> 8);

		b = (d >> 10) & 0x1f;
		g = (d >>  5) & 0x1f;
		r = (d >>  0) & 0x1f;

		r = (r << 3) | (r >> 2);
		g = (g << 3) | (g >> 2);
		b = (b << 3) | (b >> 2);

		dst[i] = BurnHighCol(r, g, b, 0);
	}
}

void KonamiICReset()
{
	if (KonamiIC_K051960InUse) K051960Reset();
	if (KonamiIC_K052109InUse) K052109Reset();
	if (KonamiIC_K051316InUse) K051316Reset();
	if (KonamiIC_K053245InUse) K053245Reset();
	if (KonamiIC_K053247InUse) K053247Reset();
	if (KonamiIC_K053936InUse) K053936Reset();

	// No init's, so always reset these
	K053251Reset();
	K054000Reset();
	K051733Reset();
}

void KonamiICExit()
{
	if (konami_temp_screen) {
		free (konami_temp_screen);
		konami_temp_screen = NULL;
	}

	if (KonamiIC_K051960InUse) K051960Exit();
	if (KonamiIC_K052109InUse) K052109Exit();
	if (KonamiIC_K051316InUse) K051316Exit();
	if (KonamiIC_K053245InUse) K053245Exit();
	if (KonamiIC_K053247InUse) K053247Exit();
	if (KonamiIC_K053936InUse) K053936Exit();

	KonamiIC_K051960InUse = 0;
	KonamiIC_K052109InUse = 0;
	KonamiIC_K051316InUse = 0;
	KonamiIC_K053245InUse = 0;
	KonamiIC_K053247InUse = 0;

	K05324xZRejection = -1;
}

void KonamiICScan(int nAction)
{
	if (KonamiIC_K051960InUse) K051960Scan(nAction);
	if (KonamiIC_K052109InUse) K052109Scan(nAction);
	if (KonamiIC_K051316InUse) K051316Scan(nAction);
	if (KonamiIC_K053245InUse) K053245Scan(nAction);
	if (KonamiIC_K053247InUse) K053247Scan(nAction);
	if (KonamiIC_K053936InUse) K053936Scan(nAction);

	K053251Scan(nAction);
	K054000Scan(nAction);
	K051733Scan(nAction);
}

/*
	Transfer and blend pixels

	* highlights are right? where is this used?
	* also, has a limitation of 0x4000 colors, some games may need more.
*/
void KonamiBlendCopy(unsigned int *palette /* 32-bit color */, unsigned int *drvpalette /* n-bit color */)
{
	unsigned char *dst = pBurnDraw;

	for (int i = 0; i < nScreenWidth * nScreenHeight; i++)
	{
		if (pTransDraw[i] >= 0x4000)		// shadow & highlight
		{
			unsigned int source = palette[pTransDraw[i] & 0x3fff];
			unsigned int blend = palette[konami_temp_screen[i] & 0x3fff];

			int a = 0x99; // shadow (255 * 0.60)
			if (pTransDraw[i] & 0x4000) a = 0xa9; // highlight (255 * (1 / 0.60)) & 0xff; (0x1a9 otherwise) right?

			int ad = 256 - a;

			// ripped from MAME...
			unsigned int p = ((((source & 0x0000ff) * a + (blend & 0x0000ff) * ad) >> 8)) |
		  			 ((((source & 0x00ff00) * a + (blend & 0x00ff00) * ad) >> 8) & 0x00ff00) |
		  			 ((((source & 0xff0000) * a + (blend & 0xff0000) * ad) >> 8) & 0xff0000);

			PutPix(dst, BurnHighCol(p >> 16, p >> 8, p, 0));
		}
		else		// normal
		{
			PutPix(dst, drvpalette[pTransDraw[i]]);
		}

		dst += nBurnBpp;
	}
}
