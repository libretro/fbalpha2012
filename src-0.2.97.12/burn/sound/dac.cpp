#include "burnint.h"
#include "burn_sound.h"

#define DEFAULT_SAMPLE_RATE (48000 * 4)

struct dac_info
{
	INT16			Output;
	INT16			UnsignedVolTable[256];
	INT16			SignedVolTable[256];
	unsigned int 		Delta;
};

static struct dac_info *chip = NULL;
static int nVolShift;
static int bAddSignal;

void DACUpdate(short* Buffer, int Length)
{
	INT16 Out = chip->Output;
	
	while (Length--) {
		if (bAddSignal) {
			Buffer[0] += Out;
			Buffer[1] += Out;
		} else {
			Buffer[0] = Out;
			Buffer[1] = Out;
		}
		Buffer += 2;
	}
}

void DACWrite(UINT8 Data)
{
	INT16 Out = chip->UnsignedVolTable[Data];
	
	chip->Output = Out >> nVolShift;
}

void DACSignedWrite(UINT8 Data)
{
	INT16 Out = chip->SignedVolTable[Data];
	
	chip->Output = Out >> nVolShift;
}

static void DACBuildVolTable()
{
	int i;
	
	for (i = 0;i < 256;i++) {
		chip->UnsignedVolTable[i] = i * 0x101 / 2;
		chip->SignedVolTable[i] = i * 0x101 - 0x8000;
	}
}

void DACInit(int Clock, int bAdd)
{
	chip = (struct dac_info*)malloc(sizeof(struct dac_info));
	memset(chip, 0, sizeof(chip));
	
	chip->Delta = (Clock) ? Clock : DEFAULT_SAMPLE_RATE;
	
	DACBuildVolTable();
	
	nVolShift = 0;
	bAddSignal = bAdd;
	
	chip->Output = 0;
}

void DACSetVolShift(int nShift)
{
	nVolShift = nShift;
}

void DACReset()
{
	chip->Output = 0;
}

void DACExit()
{
	free(chip);
	chip = NULL;
}

int DACScan(int nAction,int *pnMin)
{
	struct BurnArea ba;
	char szName[16];
	
	if ((nAction & ACB_DRIVER_DATA) == 0) {
		return 1;
	}
	
	if (pnMin != NULL) {
		*pnMin = 0x029678;
	}
	
	sprintf(szName, "DAC #0");
	ba.Data		= &chip;
	ba.nLen		= sizeof(struct dac_info);
	ba.nAddress = 0;
	ba.szName	= szName;
	BurnAcb(&ba);
	
	return 0;
}

#undef DEFAULT_SAMPLE_RATE
