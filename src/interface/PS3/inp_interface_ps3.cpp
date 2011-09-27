// Burner Input module
#include "burner.h"

unsigned int nInputSelect = 0;
bool bInputOkay = false;

extern struct InputInOut InputInOutCellInput;

static struct InputInOut *pInputInOut[]=
{
	&InputInOutCellInput,
};


static InterfaceInfo InpInfo = { NULL, };

extern int CellinpState(int nCode);
#define CinpState(nCode) CellinpState(nCode)

int InputInit()
{
	bInputOkay = true;
	return bInputOkay;
}

int InputExit()
{
	bInputOkay = false;

	return bInputOkay;
}

// This will process all PC-side inputs and optionally update the emulated game side.
void InputMake(void)
{
	struct GameInp* pgi;
	unsigned int i;

	// Do one frames worth of keyboard input sliders
	// Begin of InputTick()

	for (i = 0, pgi = GameInp; i < nGameInpCount; i++, pgi++)
	{
		int nAdd = 0;
		if ((pgi->nInput &  GIT_GROUP_SLIDER) == 0) // not a slider
			continue;

		if (pgi->nInput == GIT_KEYSLIDER)
		{
			// Get states of the two keys
			if (CinpState(pgi->Input.Slider.SliderAxis.nSlider[0]))
				nAdd -= 0x100;
			if (CinpState(pgi->Input.Slider.SliderAxis.nSlider[1]))
				nAdd += 0x100;
		}

		// nAdd is now -0x100 to +0x100

		// Change to slider speed
		nAdd *= pgi->Input.Slider.nSliderSpeed;
		nAdd /= 0x100;

		if (pgi->Input.Slider.nSliderCenter)
		{ // Attact to center
			int v = pgi->Input.Slider.nSliderValue - 0x8000;
			v *= (pgi->Input.Slider.nSliderCenter - 1);
			v /= pgi->Input.Slider.nSliderCenter;
			v += 0x8000;
			pgi->Input.Slider.nSliderValue = v;
		}

		pgi->Input.Slider.nSliderValue += nAdd;
		// Limit slider
		if (pgi->Input.Slider.nSliderValue < 0x0100)
			pgi->Input.Slider.nSliderValue = 0x0100;
		if (pgi->Input.Slider.nSliderValue > 0xFF00)
			pgi->Input.Slider.nSliderValue = 0xFF00;
	}
	// End of InputTick()

	for (i = 0, pgi = GameInp; i < nGameInpCount; i++, pgi++)
	{
		if (pgi->Input.pVal == NULL)
			continue;

		switch (pgi->nInput)
		{
			case 0: // Undefined
				pgi->Input.nVal = 0;
				break;
			case GIT_CONSTANT: // Constant value
				pgi->Input.nVal = pgi->Input.Constant.nConst;
				*(pgi->Input.pVal) = pgi->Input.nVal;
				break;
			case GIT_SWITCH:
				{ // Digital input
					int s = CinpState(pgi->Input.Switch.nCode);

					if (pgi->nType & BIT_GROUP_ANALOG)
					{
						// Set analog controls to full
						if (s)
							pgi->Input.nVal = 0xFFFF;
						else
							pgi->Input.nVal = 0x0001;
#ifdef LSB_FIRST
						*(pgi->Input.pShortVal) = pgi->Input.nVal;
#else
						*((int *)pgi->Input.pShortVal) = pgi->Input.nVal;
#endif
					}
					else
					{
						// Binary controls
						if (s)
							pgi->Input.nVal = 1;
						else
							pgi->Input.nVal = 0;
						*(pgi->Input.pVal) = pgi->Input.nVal;
					}

					break;
				}
			case GIT_KEYSLIDER:						// Keyboard slider
				{
					int nSlider = pgi->Input.Slider.nSliderValue;
					if (pgi->nType == BIT_ANALOG_REL) {
						nSlider -= 0x8000;
						nSlider >>= 4;
					}

					pgi->Input.nVal = (unsigned short)nSlider;
#ifdef LSB_FIRST
					*(pgi->Input.pShortVal) = pgi->Input.nVal;
#else
					*((int *)pgi->Input.pShortVal) = pgi->Input.nVal;
#endif
					break;
				}
		}
	}
}
