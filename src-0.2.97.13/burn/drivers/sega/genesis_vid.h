#include "driver.h"

extern UINT16 GenesisPaletteBase;
extern UINT16 GenesisBgPalLookup[4];
extern UINT16 GenesisSpPalLookup[4];

int StartGenesisVDP(int ScreenNum, unsigned int* pal);
void GenesisVDPExit();
void GenesisVDPScan();
UINT16 GenesisVDPRead(unsigned int offset);
void GenesisVDPWrite(unsigned int offset, UINT16 data);
void vdp_drawline(UINT16 *bitmap, unsigned int line, int bgfill);
