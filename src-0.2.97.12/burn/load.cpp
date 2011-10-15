// Burn - Rom Loading module
#include "burnint.h"

// Load a rom and separate out the bytes by nGap
// Dest is the memory block to insert the rom into
static int LoadRom(unsigned char *Dest,int i,int nGap,int bXor)
{
  int nRet=0,nLen=0;
  if (BurnExtLoadRom==NULL) return 1; // Load function was not defined by the application

  // Find the length of the rom (as given by the current driver)
  {
    struct BurnRomInfo ri;
    ri.nType=0;
    ri.nLen=0;
    BurnDrvGetRomInfo(&ri,i);
    if (ri.nType==0) return 0; // Empty rom slot - don't load anything and return success
    nLen=ri.nLen;
  }
  
  char* RomName = ""; //add by emufan
  BurnDrvGetRomName(&RomName, i, 0);

  if (nLen<=0) return 1;

  if (nGap>1 || bXor)
  {
    unsigned char *Load=NULL;
    unsigned char *pd=NULL,*pl=NULL,*LoadEnd=NULL;
    int nLoadLen=0;

    // Allocate space for the file
    Load=(unsigned char *)malloc(nLen);
    if (Load==NULL) return 1;
    memset(Load,0,nLen);

    // Load in the file
    nRet=BurnExtLoadRom(Load,&nLoadLen,i);
	if (bDoIpsPatch) IpsApplyPatches(Load, RomName);
    if (nRet!=0) { free(Load); return 1; }

    if (nLoadLen<0) nLoadLen=0;
    if (nLoadLen>nLen) nLoadLen=nLen;

    // Loaded rom okay. Now insert into Dest
    LoadEnd=Load+nLoadLen;
    pd=Dest; pl=Load;
    // Quickly copy in the bytes with a gap of 'nGap' between each byte

    if (bXor)
    {
      do { *pd ^= *pl++; pd+=nGap; } while (pl<LoadEnd);
    }
    else
    {
      do { *pd  = *pl++; pd+=nGap; } while (pl<LoadEnd);
    }
    free(Load);
  }
  else
  {
    // If no XOR, and gap of 1, just copy straight in
    nRet=BurnExtLoadRom(Dest,NULL,i);
	if (bDoIpsPatch) IpsApplyPatches(Dest, RomName);
    if (nRet!=0) return 1;
  }

  return 0;
}

int BurnLoadRom(unsigned char *Dest,int i,int nGap)
{
  return LoadRom(Dest,i,nGap,0);
}

int BurnXorRom(unsigned char *Dest,int i,int nGap)
{
  return LoadRom(Dest,i,nGap,1);
}

// Separate out a bitfield into Bit number 'nField' of each nibble in pDest
// (end result: each dword in memory carries the 8 pixels of a tile line).
int BurnLoadBitField(unsigned char *pDest,unsigned char *pSrc,int nField,int nSrcLen)
{
  int nPix=0;
  for (nPix=0; nPix<(nSrcLen<<3); nPix++)
  {
    int nBit;
    // Get the bitplane pixel value (on or off)
    nBit=(*pSrc)>>(7-(nPix&7)); nBit&=1;
    nBit<<=nField; // Move to correct bit for this field

    // use low nibble for each even pixel
    if ((nPix&1)==1) nBit<<=4; // use high nibble for each odd pixel

    *pDest|=nBit; // OR into destination
    if ((nPix&1)==1) pDest++;
    if ((nPix&7)==7) pSrc++;
  }
  return 0;
}

