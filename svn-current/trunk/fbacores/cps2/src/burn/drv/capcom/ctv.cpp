#include "cps.h"

// CPS Tile Variants
// horizontal/vertical clip rolls
UINT32 nCtvRollX=0,nCtvRollY=0;
// Add 0x7fff after each pixel/line
// If nRollX/Y&0x20004000 both == 0, you can draw the pixel

UINT8 *pCtvTile=NULL; // Pointer to tile data
INT32 nCtvTileAdd=0; // Amount to add after each tile line
UINT8 *pCtvLine=NULL; // Pointer to output bitmap

// Include all tile variants:
#include "ctv.h"

static INT32 nLastBpp=0;
INT32 CtvReady()
{
  // Set up the CtvDoX functions to point to the correct bpp functions.
  // Must be called before calling CpstOne
   memcpy(CtvDoX,CtvDo2,sizeof(CtvDoX));
   memcpy(CtvDoXM,CtvDo2m,sizeof(CtvDoXM));
   memcpy(CtvDoXB,CtvDo2b,sizeof(CtvDoXB));
  nLastBpp=nBurnBpp;
  return 0;
}
