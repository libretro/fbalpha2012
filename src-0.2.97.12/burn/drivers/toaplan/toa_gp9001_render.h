// Graphics format: 8 × 8 pixel tiles, 4 bits/pixel.

// Create a unique name for each of the functions
#define FN(a,b,c,d,e) RenderTile ## a ## _ROT ## b  ## c ## d ## e
#define FUNCTIONNAME(a,b,c,d,e) FN(a,b,c,d,e)

#if ROT == 0

 #if XFLIP == 0
  #define ADVANCECOLUMN pPixel += (BPP >> 3)
 #elif XFLIP == 1
  #define ADVANCECOLUMN pPixel -= (BPP >> 3)
 #else
  #error illegal XFLIP value
 #endif

 #if YFLIP == 0
  #define ADVANCEROW pTileRow += ((BPP >> 3) * 320)
 #elif YFLIP == 1
  #define ADVANCEROW pTileRow -= ((BPP >> 3) * 320)
 #else
  #error illegal YFLIP value
 #endif

#elif ROT == 270

 #if XFLIP == 0
  #define ADVANCECOLUMN pPixel -= ((BPP >> 3) * 240)
 #elif XFLIP == 1
  #define ADVANCECOLUMN pPixel += ((BPP >> 3) * 240)
 #else
  #error illegal XFLIP value
 #endif

 #if YFLIP == 0
  #define ADVANCEROW pTileRow += (BPP >> 3)
 #elif YFLIP == 1
  #define ADVANCEROW pTileRow -= (BPP >> 3)
 #else
  #error illegal YFLIP value
 #endif

#else
 #error unsupported rotation angle specified
#endif

#if DOCLIP == 0
 #define CLIP _NOCLIP
 #define TESTCLIP(x) (1)
#elif DOCLIP == 1
 #define CLIP _CLIP
 #define TESTCLIP(x) (nTileXPos >= -(x) && nTileXPos < (320 - (x)))
#else
 #error illegal doclip value.
#endif

#if ISOPAQUE == 0
 #define OPACITY _TRANS
 #define TESTCOLOUR(x) (x)
#elif ISOPAQUE == 1
 #define OPACITY _OPAQUE
 #define TESTCOLOUR(x) (1)
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

// For decorating the function name
#if XFLIP == 1
 #if YFLIP == 1
  #define FLIP _FLIPXY
 #else
  #define FLIP _FLIPX
 #endif
#elif YFLIP == 1
 #define FLIP _FLIPY
#else
 #define FLIP _NOFLIP
#endif

static void FUNCTIONNAME(BPP,ROT,FLIP,CLIP,OPACITY)()
{
	unsigned char *pTileRow, *pPixel;
	int y, nColour;

 #if ROT == 0
  #if YFLIP == 0
	for (y = 0, pTileRow = pTile; y < 8; y++, ADVANCEROW) {
  #else
	for (y = 7, pTileRow = pTile + ((BPP >> 3) * 320 * 7); y >= 0; y--, ADVANCEROW) {
  #endif
 #else
  #if YFLIP == 0
	for (y = 0, pTileRow = pTile; y < 8; y++, ADVANCEROW) {
  #else
	for (y = 7, pTileRow = pTile + ((BPP >> 3) * 7); y >= 0; y--, ADVANCEROW) {
  #endif
 #endif

 #if DOCLIP == 1
		if (nTileYPos + y < 0 || nTileYPos + y >= 240) {
			pTileData++;
			continue;
		}
 #endif

 #if ROT == 0
  #if XFLIP == 0
   #define OFFSET(a) a
		pPixel = pTileRow;
  #else
   #define OFFSET(a) (7 - a)
		pPixel = pTileRow + ((BPP >> 3) * 7);
  #endif
 #else
  #if XFLIP == 0
   #define OFFSET(a) a
		pPixel = pTileRow;
  #else
   #define OFFSET(a) (7 - a)
		pPixel = pTileRow - ((BPP >> 3) * 240 * 7);
  #endif
 #endif

		nColour = *pTileData++;
		PLOTPIXEL(OFFSET(0), nColour & 0x0F);
		ADVANCECOLUMN;
		nColour >>= 4;
		PLOTPIXEL(OFFSET(1), nColour & 0x0F);
		ADVANCECOLUMN;

		nColour >>= 4;
		PLOTPIXEL(OFFSET(2), nColour & 0x0F);
		ADVANCECOLUMN;
		nColour >>= 4;
		PLOTPIXEL(OFFSET(3), nColour & 0x0F);
		ADVANCECOLUMN;

		nColour >>= 4;
		PLOTPIXEL(OFFSET(4), nColour & 0x0F);
		ADVANCECOLUMN;
		nColour >>= 4;
		PLOTPIXEL(OFFSET(5), nColour & 0x0F);
		ADVANCECOLUMN;

		nColour >>= 4;
		PLOTPIXEL(OFFSET(6), nColour & 0x0F);
		ADVANCECOLUMN;
		nColour >>= 4;
		PLOTPIXEL(OFFSET(7), nColour & 0x0F);

 #undef OFFSET

	}
}

#undef FLIP
#undef CLIP
#undef PLOTPIXEL
#undef TESTCLIP
#undef TESTCOLOUR
#undef ADVANCEROW
#undef ADVANCECOLUMN
#undef OPACITY
#undef FUNCTIONNAME
#undef FN

