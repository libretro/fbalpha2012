#include "cps.h"

// CPS Scroll2 with Row scroll - Draw
static int nKnowBlank=-1;	// The tile we know is blank
static int nFirstY, nLastY;
static int bVCare;

inline static unsigned short *FindTile(int fx,int fy)
{
  int p; unsigned short *pst;
  // Find tile address
  p=((fy&0x30)<<8) | ((fx&0x3f)<<6) | ((fy&0x0f)<<2);
  pst=(unsigned short *)(CpsrBase + p);
  return pst;
}

// Draw a tile line without Row Shift
static void Cps1TileLine(int y,int sx)
{
  int x,ix,iy,sy;

  bVCare=0;
  if (y<0 || y>=14-1) bVCare=1; // Take care on the edges

  ix=(sx>>4)+1; sx&=15; sx=16-sx;
  sy=16-(nCpsrScrY&15); iy=(nCpsrScrY>>4)+1;
  nCpstY=sy+(y<<4);

  for (x=-1; x<24; x++)
  {
    unsigned short *pst; int t,a;
    // Don't need to clip except around the border
    if (bVCare || x<0 || x>=24-1) nCpstType=CTT_16X16 | CTT_CARE;
    else nCpstType=CTT_16X16;

    pst=FindTile(ix+x,iy+y);
    t=pst[0];
    
    if (Scroll2TileMask) t &= Scroll2TileMask;

   t = GfxRomBankMapper(GFXTYPE_SCROLL2, t);
   if (t == -1) continue;
   
    t<<=7; // Get real tile address
    t+=nCpsGfxScroll[2]; // add on offset to scroll tile
    if (t==nKnowBlank) continue; // Don't draw: we know it's blank
    
    a=pst[1];

    CpstSetPal(0x40 | (a&0x1f));
    nCpstX=sx+(x<<4); nCpstTile=t; nCpstFlip=(a>>5)&3;

	if(nBgHi) CpstPmsk=*(unsigned short *)(CpsSaveReg[0] + MaskAddr[(a&0x180)>>7]);
    if(CpstOneDoX[nBgHi]()) nKnowBlank=t;
  }
}

static void Cps2TileLine(int y,int sx)
{
  int x,ix,iy,sy;

  ix=(sx>>4)+1; sx&=15; sx=16-sx;
  sy=16-(nCpsrScrY&15); iy=(nCpsrScrY>>4)+1;
  nCpstY=sy+(y<<4);

  for (x=-1; x<24; x++)
  {
    unsigned short *pst; int t,a;
    // Don't need to clip except around the border
    if (bVCare || x<0 || x>=24-1) nCpstType=CTT_16X16 | CTT_CARE;
    else nCpstType=CTT_16X16;

    pst=FindTile(ix+x,iy+y);
    t=pst[0];
    t<<=7; // Get real tile address
    t+=nCpsGfxScroll[2]; // add on offset to scroll tiles
	if (t==nKnowBlank) continue; // Don't draw: we know it's blank
    a=pst[1];

    CpstSetPal(0x40 | (a&0x1f));
    nCpstX=sx+(x<<4); nCpstTile=t; nCpstFlip=(a>>5)&3;
    if(CpstOneDoX[2]()) nKnowBlank=t;
  }
}

// Draw a tile line with Row Shift
static void Cps1TileLineRows(int y,struct CpsrLineInfo *pli)
{
  int sy,iy,x;
  int nTileCount;
  int nLimLeft,nLimRight;

  bVCare=0;
  if (y<0 || y>=14-1) bVCare=1; // Take care on the edges

  nTileCount=pli->nTileEnd-pli->nTileStart;

  sy=16-(nCpsrScrY&15); iy=(nCpsrScrY>>4)+1;
  nCpstY=sy+(y<<4);
  CpstRowShift=pli->Rows;

  // If these rowshift limits go off the edges, we should take
  // care drawing the tile.
  nLimLeft =pli->nMaxLeft;
  nLimRight=pli->nMaxRight;
  for (x=0; x<nTileCount; x++,
    nLimLeft+=16, nLimRight+=16)
  {
    unsigned short *pst; int t,a; int tx; int bCare;
    tx=pli->nTileStart+x;

    // See if we have to clip vertically anyway
    bCare=bVCare;
    if (bCare==0) // If we don't...
    {
      // Check screen limits of this tile
      if (nLimLeft <      0) bCare=1; // Will cross left egde
      if (nLimRight> 384-16) bCare=1; // Will cross right edge
    }
    if (bCare) nCpstType=CTT_16X16 | CTT_ROWS | CTT_CARE;
    else       nCpstType=CTT_16X16 | CTT_ROWS;

    pst=FindTile(tx,iy+y);
    t=pst[0];
    
    if (Scroll2TileMask) t &= Scroll2TileMask;

    t = GfxRomBankMapper(GFXTYPE_SCROLL2, t);
    if (t == -1) continue;
    
    t<<=7; // Get real tile address
    t+=nCpsGfxScroll[2]; // add on offset to scroll tiles
    if (t==nKnowBlank) continue; // Don't draw: we know it's blank

    a=pst[1];

    CpstSetPal(0x40 | (a&0x1f));

    nCpstX=x<<4; nCpstTile=t; nCpstFlip=(a>>5)&3;

	if (nBgHi) {
		CpstPmsk = *(unsigned short*)(CpsSaveReg[0] + MaskAddr[(a & 0x180) >> 7]);
	}

	if(CpstOneDoX[nBgHi]()) nKnowBlank=t;
  }
}

static void Cps2TileLineRows(int y,struct CpsrLineInfo *pli)
{
  int sy,iy,x;
  int nTileCount;
  int nLimLeft,nLimRight;

  nTileCount=pli->nTileEnd-pli->nTileStart;

  sy=16-(nCpsrScrY&15); iy=(nCpsrScrY>>4)+1;
  nCpstY=sy+(y<<4);
  CpstRowShift=pli->Rows;

  // If these rowshift limits go off the edges, we should take
  // care drawing the tile.
  nLimLeft =pli->nMaxLeft;
  nLimRight=pli->nMaxRight;
  for (x=0; x<nTileCount; x++,
    nLimLeft+=16, nLimRight+=16)
  {
    unsigned short *pst; int t,a; int tx; int bCare;
    tx=pli->nTileStart+x;

    // See if we have to clip vertically anyway
    bCare=bVCare;
    if (bCare==0) // If we don't...
    {
      // Check screen limits of this tile
      if (nLimLeft <      0) bCare=1; // Will cross left egde
      if (nLimRight> 384-16) bCare=1; // Will cross right edge
    }
    if (bCare) nCpstType=CTT_16X16 | CTT_ROWS | CTT_CARE;
    else       nCpstType=CTT_16X16 | CTT_ROWS;

    pst=FindTile(tx,iy+y);
    t=pst[0];
    t<<=7; // Get real tile address
    t+=nCpsGfxScroll[2]; // add on offset to scroll tiles

    if (t==nKnowBlank) continue; // Don't draw: we know it's blank
    a=pst[1];

    CpstSetPal(0x40 | (a&0x1f));

    nCpstX=x<<4; nCpstTile=t; nCpstFlip=(a>>5)&3;
    if(CpstOneDoX[2]()) nKnowBlank=t;
  }
}

int Cps1rRender()
{
  int y; struct CpsrLineInfo *pli;
  if (CpsrBase==NULL) return 1;

  nKnowBlank=-1; // We don't know which tile is blank yet

  for (y=-1,pli=CpsrLineInfo; y<14; y++,pli++)
  {
    if (pli->nWidth==0)
	  Cps1TileLine(y,pli->nStart); // no rowscroll needed
    else
      Cps1TileLineRows(y,pli); // row scroll
  }
  return 0;
}

int Cps2rRender()
{
	int y;
	struct CpsrLineInfo *pli;
	if (CpsrBase==NULL) return 1;

	nKnowBlank = -1;					// We don't know which tile is blank yet

	nLastY = (nEndline + (nCpsrScrY & 15)) >> 4;
	nFirstY = (nStartline + (nCpsrScrY & 15)) >> 4;
	for (y = nFirstY - 1, pli = CpsrLineInfo + nFirstY; y < nLastY; y++, pli++) {

		bVCare = ((y << 4) < nStartline) | (((y << 4) + 16) >= nEndline);

		if (pli->nWidth==0) {
			Cps2TileLine(y,pli->nStart);	// no rowscroll needed
		} else {
			Cps2TileLineRows(y,pli);		// row scroll
		}
	}
	return 0;
}
