#include "effect.h"
#include "burner.h"

int nShowEffect = 0;

int effect_depth = 0;
unsigned short* pEffect = NULL;

void UpdateTVEffect()
{
	int c;
	for (int i = 0; i < SCRW * SCRH; i+=2)
	{
		c = rand();
		pEffect[i] = MAKE_COLOR(c, c, c);
		c >>= 5;
		pEffect[i + 1] = MAKE_COLOR(c, c, c);
	}
}

void UpdateEffect()
{
	pEffect = (unsigned short*)pBurnDraw;
	if (pEffect == NULL) {
		return;
	}

	switch (nShowEffect)
	{
		case 0:
			return;

		case 1:
			UpdateBurnEffect();
			break;
		case 2:
			UpdateSmokeEffect();
			break;
		case 3:
			UpdateWaterEffect();
			break;
		case 4:
			UpdateTVEffect();
			break;

		default:
			UpdateBurnEffect();
			break;
	}
}
