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
		UINT8 *Regs = Chip->ram + 8 * Channel;
		if (!(Regs[0x86] & 1)) {
			const UINT8 *Rom = Chip->rom + ((Regs[0x86] & Chip->bankmask) << Chip->bankshift);
			UINT32 Addr = (Regs[0x85] << 16) | (Regs[0x84] << 8) | Chip->low[Channel];
			UINT32 Loop = (Regs[0x05] << 16) | (Regs[0x04] << 8);
			UINT8 End = Regs[6] + 1;
			int i;
			
			for (i = 0; i < nLength; i++) {
				INT8 v = 0;

				if ((Addr >> 16) == End) {
					if (Regs[0x86] & 2) {
						Regs[0x86] |= 1;
						break;
					} else {
						Addr = Loop;
					}
				}

				v = Rom[Addr >> 8] - 0x80;

				Left[i] += v * Regs[2];
				Right[i] += v * Regs[3];
				Addr = (Addr + ((Regs[7] * Chip->UpdateStep) >> 16)) & 0xffffff;
			}

			Regs[0x84] = Addr >> 8;
			Regs[0x85] = Addr >> 16;
			Chip->low[Channel] = Regs[0x86] & 1 ? 0 : Addr;
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
