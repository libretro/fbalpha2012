#include "burnint.h"
#include "burn_sound.h"

short Precalc[4096 *4];

// Routine used to precalculate the table used for interpolation
int cmc_4p_Precalc()
{
	int a, x, x2, x3;

	for (a = 0; a < 4096; a++) {
		x  = a  * 4;			// x = 0..16384
		x2 = x  * x / 16384;	// pow(x, 2);
		x3 = x2 * x / 16384;	// pow(x, 3);

		Precalc[a * 4 + 0] = (short)(-x / 3 + x2 / 2 - x3 / 6);
		Precalc[a * 4 + 1] = (short)(-x / 2 - x2     + x3 / 2 + 16384);
		Precalc[a * 4 + 2] = (short)( x     + x2 / 2 - x3 / 2);
		Precalc[a * 4 + 3] = (short)(-x / 6 + x3 / 6);
	}

	return 0;
}
