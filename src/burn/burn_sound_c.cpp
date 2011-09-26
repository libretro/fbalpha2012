#include "burnint.h"
#include "burn_sound.h"

#define CLIP(A) ((A) < -0x8000 ? -0x8000 : (A) > 0x7fff ? 0x7fff : (A))

void BurnSoundCopyClamp_C(int *Src, int16_t *Dest, int Len)
{
	Len *= 2;
	while (Len--) {
		*Dest = CLIP((*Src >> 8));
		Src++;
		Dest++;
	}
}

void BurnSoundCopyClamp_Add_C(int *Src, int16_t *Dest, int Len)
{
	Len *= 2;
	while (Len--) {
		*Dest = CLIP((*Src >> 8) + *Dest);
		Src++;
		Dest++;
	}
}

void BurnSoundCopyClamp_Mono_C(int *Src, int16_t *Dest, int Len)
{
	while (Len--) {
		Dest[0] = CLIP((*Src >> 8));
		Dest[1] = CLIP((*Src >> 8));
		Src++;
		Dest += 2;
	}
}

void BurnSoundCopyClamp_Mono_Add_C(int *Src, int16_t *Dest, int Len)
{
	while (Len--) {
		Dest[0] = CLIP((*Src >> 8) + Dest[0]);
		Dest[1] = CLIP((*Src >> 8) + Dest[1]);
		Src++;
		Dest += 2;
	}
}

// converted by regret, thanks to XingXing
void BurnSoundCopy_FM_C(int16_t* SrcL, int16_t* SrcR, int16_t* Dest, int Len, int VolL, int VolR)
{
	int volL = VolL >> 10;
	int volR = VolR >> 10;

	while (Len--) {
		Dest[0] = CLIP((*SrcL * volL) >> 8);
		Dest[1] = CLIP((*SrcR * volR) >> 8);
		SrcL++;
		SrcR++;
		Dest += 2;
	}
}

void BurnSoundCopy_FM_Add_C(int16_t* SrcL, int16_t* SrcR, int16_t* Dest, int Len, int VolL, int VolR)
{
	int volL = VolL >> 10;
	int volR = VolR >> 10;

	while (Len--) {
		Dest[0] = CLIP(((*SrcL * volL) >> 8) + Dest[0]);
		Dest[1] = CLIP(((*SrcR * volR) >> 8) + Dest[1]);
		SrcL++;
		SrcR++;
		Dest += 2;
	}
}

int16_t Precalc[4096 *4];

// Routine used to precalculate the table used for interpolation
int cmc_4p_Precalc()
{
	int a, x, x2, x3;

	for (a = 0; a < 4096; a++)
	{
		x  = a  << 2;			// x = 0..16384
		x2 = (x  * x) >> 14;	// pow(x, 2);
		x3 = (x2 * x) >> 14;	// pow(x, 3);

		Precalc[a * 4 + 0] = (int16_t)(-x / 3 + x2 / 2 - x3 / 6);
		Precalc[a * 4 + 1] = (int16_t)(-x / 2 - x2     + x3 / 2 + 16384);
		Precalc[a * 4 + 2] = (int16_t)( x     + x2 / 2 - x3 / 2);
		Precalc[a * 4 + 3] = (int16_t)(-x / 6 + x3 / 6);
	}

	return 0;
}

#undef CLIP
