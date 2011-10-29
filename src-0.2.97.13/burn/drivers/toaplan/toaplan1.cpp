#include "toaplan.h"

// This function decodes the tile data for the GP9001 chip in place.
int ToaLoadTiles(unsigned char* pDest, int nStart, int nROMSize)
{
	unsigned char* pTile;
	int nSwap = 3;

	BurnLoadRom(pDest + 3, nStart + 0, 4);
	BurnLoadRom(pDest + 1, nStart + 1, 4);
	BurnLoadRom(pDest + 2, nStart + 2, 4);
	BurnLoadRom(pDest + 0, nStart + 3, 4);

	for (pTile = pDest; pTile < (pDest + nROMSize); pTile += 4) {
		unsigned char data[4];
		for (int n = 0; n < 4; n++) {
			int m = 7 - (n << 1);
			unsigned char nPixels = ((pTile[0 ^ nSwap] >> m) & 1) << 0;
			nPixels |= ((pTile[2 ^ nSwap] >> m) & 1) << 1;
			nPixels |= ((pTile[1 ^ nSwap] >> m) & 1) << 2;
			nPixels |= ((pTile[3 ^ nSwap] >> m) & 1) << 3;
			nPixels |= ((pTile[0 ^ nSwap] >> (m - 1)) & 1) << 4;
			nPixels |= ((pTile[2 ^ nSwap] >> (m - 1)) & 1) << 5;
			nPixels |= ((pTile[1 ^ nSwap] >> (m - 1)) & 1) << 6;
			nPixels |= ((pTile[3 ^ nSwap] >> (m - 1)) & 1) << 7;

			data[n] = nPixels;
		}

		for (int n = 0; n < 4; n++) {
			pTile[n] = data[n];
		}
	}
	return 0;
}

// ----------------------------------------------------------------------------
// CPU synchronisation

int nToa1Cycles68KSync;

// Callbacks for the FM chip

void toaplan1FMIRQHandler(int, int nStatus)
{
	if (nStatus) {
		ZetSetIRQLine(0xFF, ZET_IRQSTATUS_ACK);
	} else {
		ZetSetIRQLine(0,    ZET_IRQSTATUS_NONE);
	}
}

int toaplan1SynchroniseStream(int nSoundRate)
{
	return (long long)ZetTotalCycles() * nSoundRate / 3500000;
}

// ----------------------------------------------------------------------------

inline void toaplan1SynchroniseZ80(int nExtraCycles)
{
	int nCycles = ((long long)SekTotalCycles() * nCyclesTotal[1] / nCyclesTotal[0]) + nExtraCycles;

	if (nCycles <= ZetTotalCycles()) {
		return;
	}

	nToa1Cycles68KSync = nCycles - nExtraCycles;

	BurnTimerUpdateYM3812(nCycles);
}

unsigned char __fastcall toaplan1ReadByteZ80RAM(unsigned int sekAddress)
{
//	bprintf(PRINT_NORMAL, _T("    Z80 %04X read\n"), sekAddress & 0x0FFF);

	toaplan1SynchroniseZ80(0);
	return RamZ80[(sekAddress & 0x0FFF) >> 1];
}

unsigned short __fastcall toaplan1ReadWordZ80RAM(unsigned int sekAddress)
{
//	bprintf(PRINT_NORMAL, _T("    Z80 %04X read\n"), sekAddress & 0x0FFF);

	toaplan1SynchroniseZ80(0);
	return RamZ80[(sekAddress & 0x0FFF) >> 1];
}

void __fastcall toaplan1WriteByteZ80RAM(unsigned int sekAddress, unsigned char byteValue)
{
//	bprintf(PRINT_NORMAL, _T("    Z80 %04X -> %02X\n"), sekAddress & 0x0FFF, byteValue);

	toaplan1SynchroniseZ80(0);
	RamZ80[(sekAddress & 0x0FFF) >> 1] = byteValue;
}

void __fastcall toaplan1WriteWordZ80RAM(unsigned int sekAddress, unsigned short wordValue)
{
//	bprintf(PRINT_NORMAL, _T("    Z80 %04X -> %04X\n"), sekAddress & 0x0FFF, wordValue);

	toaplan1SynchroniseZ80(0);
	RamZ80[(sekAddress & 0x0FFF) >> 1] = wordValue & 0xFF;
}

// ----------------------------------------------------------------------------
