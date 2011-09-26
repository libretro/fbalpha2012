#include "burnint.h"
#include "burn_sound.h"
#include "segapcm.h"

struct segapcm
{
	UINT8  *ram;
	UINT8 low[16];
	const UINT8 *rom;
	int bankshift;
	int bankmask;
	int UpdateStep;
};

static struct segapcm *Chip = NULL;
static int *Left = NULL;
static int *Right = NULL;

void SegaPCMUpdate(short* pSoundBuf, int nLength)
{
	int Channel;
	
	memset(Left, 0, nLength * sizeof(int));
	memset(Right, 0, nLength * sizeof(int));

	for (Channel = 0; Channel < 16; Channel++) {
		if (!(Chip->ram[0x86 + (8 * Channel)] & 1)) {
			UINT8 *Base = Chip->ram + (8 * Channel);
			UINT8 Flags = Base[0x86];
			const UINT8 *Rom = Chip->rom + ((Flags & Chip->bankmask) << Chip->bankshift);
			UINT32 Addr = (Base[5] << 16) | (Base[4] << 8) | Chip->low[Channel];
			UINT16 Loop = (Base[0x85] << 8) | Base[0x84];
			UINT8 End = Base[6] + 1;
			UINT8 Delta = Base[7];
			UINT8 VolL = Base[2];
			UINT8 VolR = Base[3];
			int i;
			
			for (i = 0; i < nLength; i++) {
				INT8 v = 0;

				if ((Addr >> 16) == End) {
					if (!(Flags & 2))
						Addr = Loop << 8;
					else
					{
						Flags |= 1;
						break;
					}
				}

				v = Rom[Addr >> 8] - 0x80;
				
				Left[i] += v * VolL;
				Right[i] += v * VolR;
				Addr += (Delta * Chip->UpdateStep) >> 16;
			}

			Base[0x86] = Flags;
			Base[4] = Addr >> 8;
			Base[5] = Addr >> 16;
			Chip->low[Channel] = Flags & 1 ? 0 : Addr;
		}
	}
	
	for (int i = 0; i < nLength; i++) {
		if (Left[i] > 32767) Left[i] = 32767;
		if (Left[i] < -32768) Left[i] = -32768;
		
		if (Right[i] > 32767) Right[i] = 32767;
		if (Right[i] < -32768) Right[i] = -32768;
		
		pSoundBuf[0] += Left[i];
		pSoundBuf[1] += Right[i];
		pSoundBuf += 2;
	}
}

void SegaPCMInit(int clock, int bank, UINT8 *pPCMData, int PCMDataSize)
{
	int Mask, RomMask;
	
	Chip = (struct segapcm*)malloc(sizeof(*Chip));
	memset(Chip, 0, sizeof(*Chip));

	Chip->rom = pPCMData;
	
	Chip->ram = (UINT8*)malloc(0x800);
	memset(Chip->ram, 0xff, 0x800);
	
	Left = (int*)malloc(nBurnSoundLen * sizeof(int));
	Right = (int*)malloc(nBurnSoundLen * sizeof(int));

	Chip->bankshift = bank;
	Mask = bank >> 16;
	if(!Mask)
		Mask = BANK_MASK7 >> 16;

	for (RomMask = 1; RomMask < PCMDataSize; RomMask *= 2) {}
	RomMask--;

	Chip->bankmask = Mask & (RomMask >> Chip->bankshift);
	
	double Rate = (double)clock / 128 / nBurnSoundRate;
	Chip->UpdateStep = (int)(Rate * 0x10000);
}

void SegaPCMExit()
{
	free(Chip);
	Chip = NULL;
	
	free(Left);
	Left = NULL;
	
	free(Right);
	Right = NULL;
}

int SegaPCMScan(int nAction,int *pnMin)
{
	struct BurnArea ba;
	char szName[16];
	
	if ((nAction & ACB_DRIVER_DATA) == 0) {
		return 1;
	}
	
	if (pnMin != NULL) {
		*pnMin = 0x029680;
	}
	
	sprintf(szName, "SegaPCM");
	ba.Data		= &Chip;
	ba.nLen		= sizeof(struct segapcm);
	ba.nAddress = 0;
	ba.szName	= szName;
	BurnAcb(&ba);
	
	return 0;
}

unsigned char SegaPCMRead(unsigned int Offset)
{
	return Chip->ram[Offset & 0x07ff];
}

void SegaPCMWrite(unsigned int Offset, unsigned char Data)
{
	Chip->ram[Offset & 0x07ff] = Data;
}
