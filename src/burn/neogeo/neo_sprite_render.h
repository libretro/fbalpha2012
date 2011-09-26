// Create a unique name for each of the functions
#define FN(a,b,c,d) RenderBank ## a ## _ZOOM ## b ## c ## d
#define FUNCTIONNAME(a,b,c,d) FN(a,b,c,d)

#if DOCLIP == 0
 #define CLIP _NOCLIP
 #define TESTCLIP(x) 1
#elif DOCLIP == 1
 #define CLIP _CLIP
 #define TESTCLIP(x) (nBankXPos + x) >= 0 && (nBankXPos + x) < nNeoScreenWidth
#else
 #error illegal doclip value.
#endif

#if ISOPAQUE == 0
 #define OPACITY _TRANS
 #define TESTCOLOUR(x) x
#else
 #error illegal isopaque value
#endif

#if BPP == 16
 #define PLOTPIXEL(a,b) if (TESTCOLOUR(b) && TESTCLIP(a)) {			\
   	*((unsigned short*)pPixel) = (unsigned short)pTilePalette[b];	\
 }
#elif BPP == 24
 #define PLOTPIXEL(a,b) if (TESTCOLOUR(b) && TESTCLIP(a)) {			\
	unsigned int nRGB = pTilePalette[b];							\
	pPixel[0] = (unsigned char)nRGB;								\
	pPixel[1] = (unsigned char)(nRGB >> 8);							\
	pPixel[2] = (unsigned char)(nRGB >> 16);						\
 }
#elif BPP == 32
 #define PLOTPIXEL(a,b) if (TESTCOLOUR(b) && TESTCLIP(a)) {			\
	 *((unsigned int*)pPixel) = (unsigned int)pTilePalette[b];		\
 }
#else
 #error unsupported bitdepth specified.
#endif

#if XZOOM == 0
 #define PLOTLINE(OFFSET,ADVANCECOLUMN)		\
	nColour = pTileData[nLine + 1];			\
	PLOTPIXEL(OFFSET( 0),nColour & 0x0F);
#elif XZOOM == 1
 #define PLOTLINE(OFFSET,ADVANCECOLUMN)		\
	nColour = pTileData[nLine];				\
	nColour >>= 16;							\
	PLOTPIXEL(OFFSET( 0),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour = pTileData[nLine + 1];			\
	PLOTPIXEL(OFFSET( 1),nColour & 0x0F);
#elif XZOOM == 2
 #define PLOTLINE(OFFSET,ADVANCECOLUMN)		\
	nColour = pTileData[nLine];				\
	nColour >>= 16;							\
	PLOTPIXEL(OFFSET( 0),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour = pTileData[nLine + 1];			\
	PLOTPIXEL(OFFSET( 1),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 16;							\
	PLOTPIXEL(OFFSET( 2),nColour & 0x0F);
#elif XZOOM == 3
 #define PLOTLINE(OFFSET,ADVANCECOLUMN)		\
	nColour = pTileData[nLine];				\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET( 0),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET( 1),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour = pTileData[nLine + 1];			\
	PLOTPIXEL(OFFSET( 2),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 16;							\
	PLOTPIXEL(OFFSET( 3),nColour & 0x0F);
#elif XZOOM == 4
 #define PLOTLINE(OFFSET,ADVANCECOLUMN)		\
	nColour = pTileData[nLine];				\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET( 0),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET( 1),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour = pTileData[nLine + 1];			\
	PLOTPIXEL(OFFSET( 2),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 16;							\
	PLOTPIXEL(OFFSET( 3),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET( 4),nColour & 0x0F);
#elif XZOOM == 5
 #define PLOTLINE(OFFSET,ADVANCECOLUMN)		\
	nColour = pTileData[nLine];				\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET( 0),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET( 1),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET( 2),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour = pTileData[nLine + 1];			\
	PLOTPIXEL(OFFSET( 3),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 16;							\
	PLOTPIXEL(OFFSET( 4),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET( 5),nColour & 0x0F);
#elif XZOOM == 6
 #define PLOTLINE(OFFSET,ADVANCECOLUMN)		\
	nColour = pTileData[nLine];				\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET( 0),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET( 1),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET( 2),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour = pTileData[nLine + 1];			\
	PLOTPIXEL(OFFSET( 3),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET( 4),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET( 5),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET( 6),nColour & 0x0F);
#elif XZOOM == 7
 #define PLOTLINE(OFFSET,ADVANCECOLUMN)		\
	nColour = pTileData[nLine];				\
	PLOTPIXEL(OFFSET( 0),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET( 1),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET( 2),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET( 3),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour = pTileData[nLine + 1];			\
	PLOTPIXEL(OFFSET( 4),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET( 5),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET( 6),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET( 7),nColour & 0x0F);
#elif XZOOM == 8
 #define PLOTLINE(OFFSET,ADVANCECOLUMN)		\
	nColour = pTileData[nLine];				\
	PLOTPIXEL(OFFSET( 0),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET( 1),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET( 2),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET( 3),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour = pTileData[nLine + 1];			\
	PLOTPIXEL(OFFSET( 4),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET( 5),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET( 6),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET( 7),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET( 8),nColour & 0x0F);
#elif XZOOM == 9
 #define PLOTLINE(OFFSET,ADVANCECOLUMN)		\
	nColour = pTileData[nLine];				\
	PLOTPIXEL(OFFSET( 0),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET( 1),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET( 2),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET( 3),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET( 4),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour = pTileData[nLine + 1];			\
	PLOTPIXEL(OFFSET( 5),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET( 6),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET( 7),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET( 8),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET( 9),nColour & 0x0F);
#elif XZOOM == 10
 #define PLOTLINE(OFFSET,ADVANCECOLUMN)		\
	nColour = pTileData[nLine];				\
	PLOTPIXEL(OFFSET( 0),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET( 1),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET( 2),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET( 3),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET( 4),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour = pTileData[nLine + 1];			\
	PLOTPIXEL(OFFSET( 5),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET( 6),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET( 7),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET( 8),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET( 9),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET(10),nColour & 0x0F);
#elif XZOOM == 11
 #define PLOTLINE(OFFSET,ADVANCECOLUMN)		\
	nColour = pTileData[nLine];				\
	PLOTPIXEL(OFFSET( 0),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET( 1),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET( 2),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET( 3),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET( 4),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET( 5),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour = pTileData[nLine + 1];			\
	PLOTPIXEL(OFFSET( 6),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET( 7),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET( 8),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET( 9),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET(10),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET(11),nColour & 0x0F);
#elif XZOOM == 12
 #define PLOTLINE(OFFSET,ADVANCECOLUMN)		\
	nColour = pTileData[nLine];				\
	PLOTPIXEL(OFFSET( 0),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET( 1),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET( 2),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET( 3),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET( 4),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET( 5),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour = pTileData[nLine + 1];			\
	PLOTPIXEL(OFFSET( 6),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET( 7),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET( 8),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET( 9),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET(10),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET(11),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET(12),nColour & 0x0F);
#elif XZOOM == 13
 #define PLOTLINE(OFFSET,ADVANCECOLUMN)		\
	nColour = pTileData[nLine];				\
	PLOTPIXEL(OFFSET( 0),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET( 1),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET( 2),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET( 3),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET( 4),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET( 5),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET( 6),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour = pTileData[nLine + 1];			\
	PLOTPIXEL(OFFSET( 7),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET( 8),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET( 9),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET(10),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET(11),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET(12),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET(13),nColour & 0x0F);
#elif XZOOM == 14
 #define PLOTLINE(OFFSET,ADVANCECOLUMN)		\
	nColour = pTileData[nLine];				\
	PLOTPIXEL(OFFSET( 0),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET( 1),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET( 2),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET( 3),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET( 4),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 8;							\
	PLOTPIXEL(OFFSET(5),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET( 6),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour = pTileData[nLine + 1];			\
	PLOTPIXEL(OFFSET( 7),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET( 8),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET( 9),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET(10),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET(11),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET(12),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET(13),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET(14),nColour & 0x0F);
#elif XZOOM == 15
 #define PLOTLINE(OFFSET,ADVANCECOLUMN)		\
	nColour = pTileData[nLine];				\
	PLOTPIXEL(OFFSET( 0),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET( 1),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET( 2),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET( 3),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET( 4),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET( 5),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET( 6),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET( 7),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour = pTileData[nLine +	1];			\
	PLOTPIXEL(OFFSET( 8),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET( 9),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET(10),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET(11),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET(12),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET(13),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET(14),nColour & 0x0F);	\
	ADVANCECOLUMN;							\
	nColour >>= 4;							\
	PLOTPIXEL(OFFSET(15),nColour & 0x0F);
#else
 #error unsupported zoom factor specified.
#endif

#define NORMALOFFSET(x) (x)
#define MIRROROFFSET(x) (XZOOM - x)

static void FUNCTIONNAME(BPP,XZOOM,CLIP,OPACITY)()
{
	unsigned char *pTileRow, *pPixel;
	int y;
	int nColour;
	int nTileNumber;
	int nTile, nLine;
	int nPrevTile;
	int nSize, nMax;
	int nPass;

	int nTileAttrib = -1;
	unsigned char nTransparent = 0;
	unsigned char nFullWrap = 0;

	unsigned char* pZoomValue = NeoZoomROM + (nBankYZoom << 8);

	pTileRow = pBurnDraw + (nSliceStart - 0x10) * (BPP >> 3) * nNeoScreenWidth + nBankXPos * (BPP >> 3);

	// Draw the first 16 tiles from top to bottom, starting at the sprite position

	nSize = (nBankSize << 4) - 1;
	if (nSize > nBankYZoom) {
		nSize = nBankYZoom;
	}

	nPass = 0;

	if (nBankYPos < nSliceStart) {							// Sprite starts in top 16 pixels
		if (nBankYPos < 0x10) {
			nPass = 1;
		}

		y = nSliceStart - nBankYPos;
		nMax = nSize + 1;
		if (nMax > y + nSliceSize) {
			nMax = y + nSliceSize;
		}
	} else {
		if (nBankYPos < nSliceEnd) {						// Sprite starts on screen
			y = 0;
			pTileRow += (nBankYPos - nSliceStart) * (BPP >> 3) * nNeoScreenWidth;
			nMax = nSize + 1;
			if (nMax > nSliceSize - (nBankYPos - nSliceStart)) {
				nMax = nSliceSize - (nBankYPos - nSliceStart);
			}
		} else {											// Sprite wraps around completely
			if (nBankYPos >= 240) {
				y = (nBankYPos + nBankYZoom + nBankYZoom + 2) & 0x01FF;
				if (nBankSize == 0x20 && y < nSliceEnd && y < nBankYPos - nBankYZoom) {
					if (y > nSliceStart) {
						y -= nSliceStart;
						nMax = nSize + 1;
						if (nMax > nSliceSize - y) {
							nMax = nSliceSize - y;
						}
						pTileRow += y * (BPP >> 3) * nNeoScreenWidth;

						nFullWrap = y;
						y = 0;
					} else {
						if (y < 0x10) {						// Sprite starts in top 16 pixels
							nPass = 1;
						}
						y = nSliceStart - y;
						nMax = nSize + 1;
						if (nMax > nSliceSize + y) {
							nMax = nSliceSize + y;
						}
					}
				} else {
					y = 0;
					nMax = 0;
				}
			} else {
				y = 0;
				nMax = 0;
			}
		}
	}

	nPrevTile = -1;

	do {
		if (y >= nMax) {
			if (nPass) {
				break;
			}
			nPass++;

			// Check if part of the sprite wraps around and is visible
			y = 0x0210 - nBankYPos;
			if (y >= nSize) {
				if (nBankSize != 0x20 || y > nSize * 2 || nBankYPos >= 240) {
					break;
				}

				y = nSize - (nBankYPos - 0x10 - nSize - 1);

				if (y < 0 || y > nSize || y <= (240 - nBankYPos)) {
					break;
				}
			}

			y += nSliceStart - 0x10;

			nMax = nSize + 1;
			if (nMax > nSliceSize + y) {
				nMax = nSliceSize + y;
			} else {
				if (y >= nMax) {
#if 0
					if (nBankSize < 0x20) {
						break;
					}
					y = nSize + nBankYZoom - 0xFF;
#else
					break;
#endif
				}
			}

			pTileRow = pBurnDraw + (nSliceStart - 0x10) * (BPP >> 3) * nNeoScreenWidth + nBankXPos * (BPP >> 3);
#if 0
			if (y < 0) {
				pTileRow -= y * (BPP >> 3) * nNeoScreenWidth;
				y = 0;
			}
#endif
		}

        nTile = pZoomValue[y] >> 4;

		if (nTile != nPrevTile) {
			nPrevTile = nTile;

			nTileNumber = pBank[nTile << 1];
			nTileAttrib = pBank[(nTile << 1) + 1];

			nTileNumber += (nTileAttrib & 0xF0) << 12;
			nTileNumber &= nNeoTileMask;

			if (nTileAttrib & 8) {
				nTileNumber &= ~7;
				nTileNumber |= nNeoSpriteFrame08;
			} else {
				if (nTileAttrib & 4) {
					nTileNumber &= ~3;
					nTileNumber |= nNeoSpriteFrame04;
				}
			}

			nTransparent = NeoTileAttrib[nTileNumber];

			if (nTransparent == 0) {
				pTileData = (unsigned int*)(NeoSpriteROM + (nTileNumber << 7));
				pTilePalette = &NeoPalette[(nTileAttrib & 0xFF00) >> 4];
			}
		}

		if (nTransparent == 0) {
			nLine = (pZoomValue[y] & 0x0F) << 1;
			if (nTileAttrib & 2) {							// Flip Y
				nLine ^= 0x1E;
			}

			if (nTileAttrib & 1) {							// Flip X
				pPixel = pTileRow + XZOOM * (BPP >> 3);
				PLOTLINE(MIRROROFFSET,pPixel -= (BPP >> 3));
			} else {
				pPixel = pTileRow;
				PLOTLINE(NORMALOFFSET,pPixel += (BPP >> 3));
			}
		}

		y++;
		pTileRow += ((BPP >> 3) * nNeoScreenWidth);

	} while (1);

	if (nBankSize <= 0x10) {
		return;
	}

	// Draw the next tile(s) from the bottom up, starting at the sprite position

	nPrevTile = -1;

	nSize = ((nBankSize - 0x10) << 4) - 1;
	if (nSize > nBankYZoom) {
		nSize = nBankYZoom;
	}

	nPass = 0;

	if (nBankYPos <= 240) {
		int nYPos = nBankYPos;

		if ((nBankSize != 0x20 && nBankYPos < (0x0100 - nSize)) || (nBankSize == 0x18 && nBankYPos >= 0x80)) {
			nYPos = (nBankYPos + nBankYZoom + nSize + 2) & 0x01FF;
			if (nYPos > 240) {
				y = 0;
				nMax = 0;
			}
		}

		if (nYPos <= nSliceStart) {									// Sprite starts above the screen
			y = nYPos;
			nMax = y;
		} else {													// Sprite starts on the screen
			if (nYPos <= nSliceEnd) {
				pTileRow = pBurnDraw + nBankXPos * (BPP >> 3) + (nYPos - 0x11) * (BPP >> 3) * nNeoScreenWidth;
				y = 0;
				nMax = nSize + 1;
				if (nMax >= nYPos - nSliceStart) {
					nMax = nYPos - nSliceStart;
				} else {
					nPass = 1;
				}
			} else {
				pTileRow = pBurnDraw + nBankXPos * (BPP >> 3) + (nSliceEnd - 0x11) * (BPP >> 3) * nNeoScreenWidth;
				y = nYPos - nSliceEnd;
				nMax = nSize + 1;
				if (nMax >= y + nSliceSize) {
					nMax = y + nSliceSize;
				} else {
					nPass = 1;
				}
			}
		}
	} else {
		if (nBankYPos >= 240 && nBankYPos <= 240 + nBankYZoom) {	// Sprite starts below the screen and is partly visible
			int nSlice;

			y = nBankYPos - nSliceEnd;

			if (y <= nBankYZoom) {
				nMax = nBankYZoom + 1;

				nSlice = nSliceSize;
				if (y < nBankYZoom - nSize) {
					nSlice -= nBankYZoom - nSize - y;
					y = nBankYZoom - nSize;
				}

#if 0
				if (nBankSize == 0x20) {
					y -= 0xFF - nBankYZoom;
				}
#endif
				pTileRow = pBurnDraw + nBankXPos * (BPP >> 3) + (nSliceStart + nSlice - 0x11) * (BPP >> 3) * nNeoScreenWidth;

				if (nMax > nSlice + y) {
					nMax = nSlice + y;
				}
			} else {
				y = 0;
				nMax = 0;
			}
		}
	}

	do {

		if (y >= nMax) {
			if (nPass) {
				break;
			}
			nPass++;

			if (nFullWrap > 0 || nBankYPos >= 0x01F0) {
				// Sprite wraps around completely

				if (nFullWrap) {
					y = nFullWrap;
				} else {
					y = nBankYPos - (nBankYZoom + nBankYZoom + 2 + nSliceStart);
					if (y <= 0 || y > nBankYPos - nSliceEnd || (!nFullWrap && y < nBankYZoom)) {
						break;
					}
				}

				nMax = nBankYZoom + 1;
				if (y < 224) {
					pTileRow = pBurnDraw + nBankXPos * (BPP >> 3) + (nSliceStart - 0x11 + y) * (BPP >> 3) * nNeoScreenWidth;
					if (nMax > y) {
						nMax = y;
					}
					y = 0;
				} else {
					pTileRow = pBurnDraw + nBankXPos * (BPP >> 3) + (nSliceEnd - 0x11) * (BPP >> 3) * nNeoScreenWidth;
					if (nMax > nSliceSize) {
						nMax = nSliceSize;
					}
					y -= nSliceSize;
				}
			} else {
				// Check if part of the sprite wraps around

				y = nBankYZoom - (nSliceEnd - 1 - nBankYPos - nBankYZoom);
				if (y < 0 || y >= nBankYZoom || y < nMax) {
					break;
				}

				nMax = nBankYZoom + 1;

				if (nMax > nSliceSize + y) {
					nMax = nSliceSize + y;
				}

				pTileRow = pBurnDraw + nBankXPos * (BPP >> 3) + (nSliceEnd - 0x11) * (BPP >> 3) * nNeoScreenWidth;
			}
		}

		nTile = 0x10 + ((pZoomValue[y] >> 4) ^ 0x0F);

		if (nTile != nPrevTile) {
			nPrevTile = nTile;

			nTileNumber = pBank[nTile << 1];
			nTileAttrib = pBank[(nTile << 1) + 1];

			nTileNumber += (nTileAttrib & 0xF0) << 12;
			nTileNumber &= nNeoTileMask;

			if (nTileAttrib & 8) {
				nTileNumber &= ~7;
				nTileNumber |= nNeoSpriteFrame08;
			} else {
				if (nTileAttrib & 4) {
					nTileNumber &= ~3;
					nTileNumber |= nNeoSpriteFrame04;
				}
			}

			nTransparent = NeoTileAttrib[nTileNumber];

			if (nTransparent == 0) {
				pTileData = (unsigned int*)(NeoSpriteROM + (nTileNumber << 7));
				pTilePalette = &NeoPalette[(nTileAttrib & 0xFF00) >> 4];
			}
		}

		if (nTransparent == 0) {
			nLine = (pZoomValue[y] & 0x0F) << 1;
			if ((nTileAttrib & 2) == 0) {				// Flip Y
				nLine ^= 0x1E;
			}

			if (nTileAttrib & 1) {						// Flip X
				pPixel = pTileRow + XZOOM * (BPP >> 3);

				PLOTLINE(MIRROROFFSET,pPixel -= (BPP >> 3));
			} else {
				pPixel = pTileRow;
				PLOTLINE(NORMALOFFSET,pPixel += (BPP >> 3));
			}
		}

		y++;
		pTileRow -= (BPP >> 3) * nNeoScreenWidth;

	} while (1);
}

#undef PLOTLINE
#undef OPACITY
#undef PLOTPIXEL
#undef TESTCOLOUR
#undef TESTCLIP
#undef CLIP
