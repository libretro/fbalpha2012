//ref: http://vba-rerecording.googlecode.com/svn/trunk/src/2xsai.cpp

static unsigned int colorMask = 0xF7DEF7DE;
static unsigned int lowPixelMask = 0x08210821;
static unsigned int qcolorMask = 0xE79CE79C;
static unsigned int qlowpixelMask = 0x18631863;
static unsigned int redblueMask = 0xF81F;
static unsigned int greenMask = 0x7E0;

int Init_2xSaI(unsigned int BitFormat, unsigned int systemColorDepth)
{
  if(systemColorDepth == 16) {
    if (BitFormat == 565) {
      colorMask = 0xF7DEF7DE;
      lowPixelMask = 0x08210821;
      qcolorMask = 0xE79CE79C;
      qlowpixelMask = 0x18631863;
      redblueMask = 0xF81F;
      greenMask = 0x7E0;
    } else if (BitFormat == 555) {
      colorMask = 0x7BDE7BDE;
      lowPixelMask = 0x04210421;
      qcolorMask = 0x739C739C;
      qlowpixelMask = 0x0C630C63;
      redblueMask = 0x7C1F;
      greenMask = 0x3E0;
    } else {
      return 0;
    }
  } else if(systemColorDepth == 32) {
    colorMask = 0xfefefe;
    lowPixelMask = 0x010101;
    qcolorMask = 0xfcfcfc;
    qlowpixelMask = 0x030303;
  } else
    return 0;

  return 1;
}

static inline int GetResult1 (unsigned int A, unsigned int B, unsigned int C, unsigned int D,
                              unsigned int /* E */)
{
    int x = 0;
    int y = 0;
    int r = 0;

    if (A == C)
      x += 1;
    else if (B == C)
      y += 1;
    if (A == D)
      x += 1;
    else if (B == D)
      y += 1;
    if (x <= 1)
      r += 1;
    if (y <= 1)
      r -= 1;
    return r;
}

static inline int GetResult2 (unsigned int A, unsigned int B, unsigned int C, unsigned int D,
                              unsigned int /* E */)
{
  int x = 0;
  int y = 0;
  int r = 0;

  if (A == C)
    x += 1;
  else if (B == C)
    y += 1;
  if (A == D)
    x += 1;
  else if (B == D)
    y += 1;
  if (x <= 1)
    r -= 1;
  if (y <= 1)
    r += 1;
  return r;
}

static inline int GetResult (unsigned int A, unsigned int B, unsigned int C, unsigned int D)
{
  int x = 0;
  int y = 0;
  int r = 0;

  if (A == C)
    x += 1;
  else if (B == C)
    y += 1;
  if (A == D)
    x += 1;
  else if (B == D)
    y += 1;
  if (x <= 1)
    r += 1;
  if (y <= 1)
    r -= 1;
  return r;
}

static inline unsigned int INTERPOLATE (unsigned int A, unsigned int B)
{
  if (A != B) {
    return (((A & colorMask) >> 1) + ((B & colorMask) >> 1) +
            (A & B & lowPixelMask));
  } else
    return A;
}

static inline unsigned int Q_INTERPOLATE (unsigned int A, unsigned int B, unsigned int C, unsigned int D)
{
  register unsigned int x = ((A & qcolorMask) >> 2) +
    ((B & qcolorMask) >> 2) +
    ((C & qcolorMask) >> 2) + ((D & qcolorMask) >> 2);
  register unsigned int y = (A & qlowpixelMask) +
    (B & qlowpixelMask) + (C & qlowpixelMask) + (D & qlowpixelMask);

  y = (y >> 2) & qlowpixelMask;
  return x + y;
}

#define BLUE_MASK565 0x001F001F
#define RED_MASK565 0xF800F800
#define GREEN_MASK565 0x07E007E0

#define BLUE_MASK555 0x001F001F
#define RED_MASK555 0x7C007C00
#define GREEN_MASK555 0x03E003E0

void Super2xSaI (unsigned char *srcPtr, unsigned int srcPitch,
                 unsigned char * /* deltaPtr */, unsigned char *dstPtr, unsigned int dstPitch,
                 int width, int height)
{
  unsigned short *bP;
  unsigned char  *dP;
  unsigned int inc_bP;
  unsigned int Nextline = srcPitch >> 1;

    {
      inc_bP = 1;

      for (; height; height--) {
        bP = (unsigned short *) srcPtr;
        dP = (unsigned char *) dstPtr;

        for (unsigned int finish = width; finish; finish -= inc_bP) {
          unsigned int color4, color5, color6;
          unsigned int color1, color2, color3;
          unsigned int colorA0, colorA1, colorA2, colorA3,
            colorB0, colorB1, colorB2, colorB3, colorS1, colorS2;
          unsigned int product1a, product1b, product2a, product2b;

          //---------------------------------------    B1 B2
          //                                         4  5  6 S2
          //                                         1  2  3 S1
          //                                           A1 A2

          colorB0 = *(bP - Nextline - 1);
          colorB1 = *(bP - Nextline);
          colorB2 = *(bP - Nextline + 1);
          colorB3 = *(bP - Nextline + 2);

          color4 = *(bP - 1);
          color5 = *(bP);
          color6 = *(bP + 1);
          colorS2 = *(bP + 2);

          color1 = *(bP + Nextline - 1);
          color2 = *(bP + Nextline);
          color3 = *(bP + Nextline + 1);
          colorS1 = *(bP + Nextline + 2);

          colorA0 = *(bP + Nextline + Nextline - 1);
          colorA1 = *(bP + Nextline + Nextline);
          colorA2 = *(bP + Nextline + Nextline + 1);
          colorA3 = *(bP + Nextline + Nextline + 2);

          //--------------------------------------
          if (color2 == color6 && color5 != color3) {
            product2b = product1b = color2;
          } else if (color5 == color3 && color2 != color6) {
            product2b = product1b = color5;
          } else if (color5 == color3 && color2 == color6) {
            register int r = 0;

            r += GetResult (color6, color5, color1, colorA1);
            r += GetResult (color6, color5, color4, colorB1);
            r += GetResult (color6, color5, colorA2, colorS1);
            r += GetResult (color6, color5, colorB2, colorS2);

            if (r > 0)
              product2b = product1b = color6;
            else if (r < 0)
              product2b = product1b = color5;
            else {
              product2b = product1b = INTERPOLATE (color5, color6);
            }
          } else {
            if (color6 == color3 && color3 == colorA1
                && color2 != colorA2 && color3 != colorA0)
              product2b =
                Q_INTERPOLATE (color3, color3, color3, color2);
            else if (color5 == color2 && color2 == colorA2
                     && colorA1 != color3 && color2 != colorA3)
              product2b =
                Q_INTERPOLATE (color2, color2, color2, color3);
            else
              product2b = INTERPOLATE (color2, color3);

            if (color6 == color3 && color6 == colorB1
                && color5 != colorB2 && color6 != colorB0)
              product1b =
                Q_INTERPOLATE (color6, color6, color6, color5);
            else if (color5 == color2 && color5 == colorB2
                     && colorB1 != color6 && color5 != colorB3)
              product1b =
                Q_INTERPOLATE (color6, color5, color5, color5);
            else
              product1b = INTERPOLATE (color5, color6);
          }

          if (color5 == color3 && color2 != color6 && color4 == color5
              && color5 != colorA2)
            product2a = INTERPOLATE (color2, color5);
          else
            if (color5 == color1 && color6 == color5
                && color4 != color2 && color5 != colorA0)
              product2a = INTERPOLATE (color2, color5);
            else
              product2a = color2;

          if (color2 == color6 && color5 != color3 && color1 == color2
              && color2 != colorB2)
            product1a = INTERPOLATE (color2, color5);
          else
            if (color4 == color2 && color3 == color2
                && color1 != color5 && color2 != colorB0)
              product1a = INTERPOLATE (color2, color5);
            else
              product1a = color5;

#ifdef WORDS_BIGENDIAN
          product1a = (product1a << 16) | product1b;
          product2a = (product2a << 16) | product2b;
#else
          product1a = product1a | (product1b << 16);
          product2a = product2a | (product2b << 16);
#endif

          *((unsigned int *) dP) = product1a;
          *((unsigned int *) (dP + dstPitch)) = product2a;

          bP += inc_bP;
          dP += sizeof (unsigned int);
        }                       // end of for ( finish= width etc..)

        srcPtr   += srcPitch;
        dstPtr   += dstPitch * 2;
//        deltaPtr += srcPitch;
      }                 // endof: for (; height; height--)
    }
}

void Super2xSaI32 (unsigned char *srcPtr, unsigned int srcPitch,
                   unsigned char * /* deltaPtr */, unsigned char *dstPtr, unsigned int dstPitch,
                   int width, int height)
{
  unsigned int *bP;
  unsigned int *dP;
  unsigned int inc_bP;
  unsigned int Nextline = srcPitch >> 2;
  inc_bP = 1;

  for (; height; height--) {
    bP = (unsigned int *) srcPtr;
    dP = (unsigned int *) dstPtr;

    for (unsigned int finish = width; finish; finish -= inc_bP) {
      unsigned int color4, color5, color6;
      unsigned int color1, color2, color3;
      unsigned int colorA0, colorA1, colorA2, colorA3,
        colorB0, colorB1, colorB2, colorB3, colorS1, colorS2;
      unsigned int product1a, product1b, product2a, product2b;

      //---------------------------------------    B1 B2
      //                                         4  5  6 S2
      //                                         1  2  3 S1
      //                                           A1 A2

      colorB0 = *(bP - Nextline - 1);
      colorB1 = *(bP - Nextline);
      colorB2 = *(bP - Nextline + 1);
      colorB3 = *(bP - Nextline + 2);

      color4 = *(bP - 1);
      color5 = *(bP);
      color6 = *(bP + 1);
      colorS2 = *(bP + 2);

      color1 = *(bP + Nextline - 1);
      color2 = *(bP + Nextline);
      color3 = *(bP + Nextline + 1);
      colorS1 = *(bP + Nextline + 2);

      colorA0 = *(bP + Nextline + Nextline - 1);
      colorA1 = *(bP + Nextline + Nextline);
      colorA2 = *(bP + Nextline + Nextline + 1);
      colorA3 = *(bP + Nextline + Nextline + 2);

      //--------------------------------------
      if (color2 == color6 && color5 != color3) {
        product2b = product1b = color2;
      } else if (color5 == color3 && color2 != color6) {
        product2b = product1b = color5;
      } else if (color5 == color3 && color2 == color6) {
        register int r = 0;

        r += GetResult (color6, color5, color1, colorA1);
        r += GetResult (color6, color5, color4, colorB1);
        r += GetResult (color6, color5, colorA2, colorS1);
        r += GetResult (color6, color5, colorB2, colorS2);

        if (r > 0)
          product2b = product1b = color6;
        else if (r < 0)
          product2b = product1b = color5;
        else {
          product2b = product1b = INTERPOLATE (color5, color6);
        }
      } else {
        if (color6 == color3 && color3 == colorA1
            && color2 != colorA2 && color3 != colorA0)
          product2b =
            Q_INTERPOLATE (color3, color3, color3, color2);
        else if (color5 == color2 && color2 == colorA2
                 && colorA1 != color3 && color2 != colorA3)
          product2b =
            Q_INTERPOLATE (color2, color2, color2, color3);
        else
          product2b = INTERPOLATE (color2, color3);

        if (color6 == color3 && color6 == colorB1
            && color5 != colorB2 && color6 != colorB0)
          product1b =
            Q_INTERPOLATE (color6, color6, color6, color5);
        else if (color5 == color2 && color5 == colorB2
                 && colorB1 != color6 && color5 != colorB3)
          product1b =
            Q_INTERPOLATE (color6, color5, color5, color5);
        else
          product1b = INTERPOLATE (color5, color6);
      }

      if (color5 == color3 && color2 != color6 && color4 == color5
          && color5 != colorA2)
        product2a = INTERPOLATE (color2, color5);
      else
        if (color5 == color1 && color6 == color5
            && color4 != color2 && color5 != colorA0)
          product2a = INTERPOLATE (color2, color5);
        else
          product2a = color2;

      if (color2 == color6 && color5 != color3 && color1 == color2
          && color2 != colorB2)
        product1a = INTERPOLATE (color2, color5);
      else
        if (color4 == color2 && color3 == color2
            && color1 != color5 && color2 != colorB0)
          product1a = INTERPOLATE (color2, color5);
        else
          product1a = color5;
      *(dP) = product1a;
      *(dP+1) = product1b;
      *(dP + (dstPitch >> 2)) = product2a;
      *(dP + (dstPitch >> 2) + 1) = product2b;

      bP += inc_bP;
      dP += 2;
    }                       // end of for ( finish= width etc..)

    srcPtr   += srcPitch;
    dstPtr   += dstPitch * 2;
//        deltaPtr += srcPitch;
  }                 // endof: for (; height; height--)
}

void SuperEagle (unsigned char *srcPtr, unsigned int srcPitch, unsigned char * /* deltaPtr */,
                 unsigned char *dstPtr, unsigned int dstPitch, int width, int height)
{
  unsigned char  *dP;
  unsigned short *bP;
//  unsigned short *xP;
  unsigned int inc_bP;

  {
    inc_bP = 1;

    unsigned int Nextline = srcPitch >> 1;

    for (; height; height--) {
      bP = (unsigned short *) srcPtr;
//      xP = (unsigned short *) deltaPtr;
      dP = dstPtr;
      for (unsigned int finish = width; finish; finish -= inc_bP) {
        unsigned int color4, color5, color6;
        unsigned int color1, color2, color3;
        unsigned int colorA1, colorA2, colorB1, colorB2, colorS1, colorS2;
        unsigned int product1a, product1b, product2a, product2b;

        colorB1 = *(bP - Nextline);
        colorB2 = *(bP - Nextline + 1);

        color4 = *(bP - 1);
        color5 = *(bP);
        color6 = *(bP + 1);
        colorS2 = *(bP + 2);

        color1 = *(bP + Nextline - 1);
        color2 = *(bP + Nextline);
        color3 = *(bP + Nextline + 1);
        colorS1 = *(bP + Nextline + 2);

        colorA1 = *(bP + Nextline + Nextline);
        colorA2 = *(bP + Nextline + Nextline + 1);

        // --------------------------------------
        if (color2 == color6 && color5 != color3) {
          product1b = product2a = color2;
          if ((color1 == color2) || (color6 == colorB2)) {
            product1a = INTERPOLATE (color2, color5);
            product1a = INTERPOLATE (color2, product1a);
            //                       product1a = color2;
          } else {
            product1a = INTERPOLATE (color5, color6);
          }

          if ((color6 == colorS2) || (color2 == colorA1)) {
            product2b = INTERPOLATE (color2, color3);
            product2b = INTERPOLATE (color2, product2b);
            //                       product2b = color2;
          } else {
            product2b = INTERPOLATE (color2, color3);
          }
        } else if (color5 == color3 && color2 != color6) {
          product2b = product1a = color5;

          if ((colorB1 == color5) || (color3 == colorS1)) {
            product1b = INTERPOLATE (color5, color6);
            product1b = INTERPOLATE (color5, product1b);
            //                       product1b = color5;
          } else {
            product1b = INTERPOLATE (color5, color6);
          }

          if ((color3 == colorA2) || (color4 == color5)) {
            product2a = INTERPOLATE (color5, color2);
            product2a = INTERPOLATE (color5, product2a);
            //                       product2a = color5;
          } else {
            product2a = INTERPOLATE (color2, color3);
          }

        } else if (color5 == color3 && color2 == color6) {
          register int r = 0;

          r += GetResult (color6, color5, color1, colorA1);
          r += GetResult (color6, color5, color4, colorB1);
          r += GetResult (color6, color5, colorA2, colorS1);
          r += GetResult (color6, color5, colorB2, colorS2);

          if (r > 0) {
            product1b = product2a = color2;
            product1a = product2b = INTERPOLATE (color5, color6);
          } else if (r < 0) {
            product2b = product1a = color5;
            product1b = product2a = INTERPOLATE (color5, color6);
          } else {
            product2b = product1a = color5;
            product1b = product2a = color2;
          }
        } else {
          product2b = product1a = INTERPOLATE (color2, color6);
          product2b =
            Q_INTERPOLATE (color3, color3, color3, product2b);
          product1a =
            Q_INTERPOLATE (color5, color5, color5, product1a);

          product2a = product1b = INTERPOLATE (color5, color3);
          product2a =
            Q_INTERPOLATE (color2, color2, color2, product2a);
          product1b =
            Q_INTERPOLATE (color6, color6, color6, product1b);

          //                    product1a = color5;
          //                    product1b = color6;
          //                    product2a = color2;
          //                    product2b = color3;
        }
#ifdef WORDS_BIGENDIAN
        product1a = (product1a << 16) | product1b;
        product2a = (product2a << 16) | product2b;
#else
        product1a = product1a | (product1b << 16);
        product2a = product2a | (product2b << 16);
#endif

        *((unsigned int *) dP) = product1a;
        *((unsigned int *) (dP + dstPitch)) = product2a;
//        *xP = color5;

        bP += inc_bP;
//        xP += inc_bP;
        dP += sizeof (unsigned int);
      }                 // end of for ( finish= width etc..)

      srcPtr += srcPitch;
      dstPtr += dstPitch * 2;
//      deltaPtr += srcPitch;
    }                   // endof: for (height; height; height--)
  }
}

void SuperEagle32 (unsigned char *srcPtr, unsigned int srcPitch, unsigned char * /* deltaPtr */,
                   unsigned char *dstPtr, unsigned int dstPitch, int width, int height)
{
  unsigned int  *dP;
  unsigned int *bP;
//  unsigned int *xP;
  unsigned int inc_bP;

  inc_bP = 1;

  unsigned int Nextline = srcPitch >> 2;

  for (; height; height--) {
    bP = (unsigned int *) srcPtr;
//    xP = (unsigned int *) deltaPtr;
    dP = (unsigned int *)dstPtr;
    for (unsigned int finish = width; finish; finish -= inc_bP) {
      unsigned int color4, color5, color6;
      unsigned int color1, color2, color3;
      unsigned int colorA1, colorA2, colorB1, colorB2, colorS1, colorS2;
      unsigned int product1a, product1b, product2a, product2b;

      colorB1 = *(bP - Nextline);
      colorB2 = *(bP - Nextline + 1);

      color4 = *(bP - 1);
      color5 = *(bP);
      color6 = *(bP + 1);
      colorS2 = *(bP + 2);

      color1 = *(bP + Nextline - 1);
      color2 = *(bP + Nextline);
      color3 = *(bP + Nextline + 1);
      colorS1 = *(bP + Nextline + 2);

      colorA1 = *(bP + Nextline + Nextline);
      colorA2 = *(bP + Nextline + Nextline + 1);

      // --------------------------------------
      if (color2 == color6 && color5 != color3) {
        product1b = product2a = color2;
        if ((color1 == color2) || (color6 == colorB2)) {
          product1a = INTERPOLATE (color2, color5);
          product1a = INTERPOLATE (color2, product1a);
          //                       product1a = color2;
        } else {
          product1a = INTERPOLATE (color5, color6);
        }

        if ((color6 == colorS2) || (color2 == colorA1)) {
          product2b = INTERPOLATE (color2, color3);
          product2b = INTERPOLATE (color2, product2b);
          //                       product2b = color2;
        } else {
          product2b = INTERPOLATE (color2, color3);
        }
      } else if (color5 == color3 && color2 != color6) {
        product2b = product1a = color5;

        if ((colorB1 == color5) || (color3 == colorS1)) {
          product1b = INTERPOLATE (color5, color6);
          product1b = INTERPOLATE (color5, product1b);
          //                       product1b = color5;
        } else {
          product1b = INTERPOLATE (color5, color6);
        }

        if ((color3 == colorA2) || (color4 == color5)) {
          product2a = INTERPOLATE (color5, color2);
          product2a = INTERPOLATE (color5, product2a);
          //                       product2a = color5;
        } else {
          product2a = INTERPOLATE (color2, color3);
        }

      } else if (color5 == color3 && color2 == color6) {
        register int r = 0;

        r += GetResult (color6, color5, color1, colorA1);
        r += GetResult (color6, color5, color4, colorB1);
        r += GetResult (color6, color5, colorA2, colorS1);
        r += GetResult (color6, color5, colorB2, colorS2);

        if (r > 0) {
          product1b = product2a = color2;
          product1a = product2b = INTERPOLATE (color5, color6);
        } else if (r < 0) {
          product2b = product1a = color5;
          product1b = product2a = INTERPOLATE (color5, color6);
        } else {
          product2b = product1a = color5;
          product1b = product2a = color2;
        }
      } else {
        product2b = product1a = INTERPOLATE (color2, color6);
        product2b =
          Q_INTERPOLATE (color3, color3, color3, product2b);
        product1a =
          Q_INTERPOLATE (color5, color5, color5, product1a);

        product2a = product1b = INTERPOLATE (color5, color3);
        product2a =
          Q_INTERPOLATE (color2, color2, color2, product2a);
        product1b =
          Q_INTERPOLATE (color6, color6, color6, product1b);

        //                    product1a = color5;
        //                    product1b = color6;
        //                    product2a = color2;
        //                    product2b = color3;
      }
      *(dP) = product1a;
      *(dP+1) = product1b;
      *(dP + (dstPitch >> 2)) = product2a;
      *(dP + (dstPitch >> 2) +1) = product2b;
//      *xP = color5;

      bP += inc_bP;
//      xP += inc_bP;
      dP += 2;
    }                 // end of for ( finish= width etc..)

    srcPtr += srcPitch;
    dstPtr += dstPitch * 2;
//    deltaPtr += srcPitch;
  }                   // endof: for (height; height; height--)
}

void _2xSaI (unsigned char *srcPtr, unsigned int srcPitch, unsigned char * /* deltaPtr */,
             unsigned char *dstPtr, unsigned int dstPitch, int width, int height)
{
  unsigned char  *dP;
  unsigned short *bP;
  unsigned int inc_bP;

  {
    inc_bP = 1;

    unsigned int Nextline = srcPitch >> 1;

    for (; height; height--) {
      bP = (unsigned short *) srcPtr;
      dP = dstPtr;

      for (unsigned int finish = width; finish; finish -= inc_bP) {

        register unsigned int colorA, colorB;
        unsigned int colorC, colorD,
          colorE, colorF, colorG, colorH,
          colorI, colorJ, colorK, colorL,

          colorM, colorN, colorO, colorP;
        unsigned int product, product1, product2;

        //---------------------------------------
        // Map of the pixels:                    I|E F|J
        //                                       G|A B|K
        //                                       H|C D|L
        //                                       M|N O|P
        colorI = *(bP - Nextline - 1);
        colorE = *(bP - Nextline);
        colorF = *(bP - Nextline + 1);
        colorJ = *(bP - Nextline + 2);

        colorG = *(bP - 1);
        colorA = *(bP);
        colorB = *(bP + 1);
        colorK = *(bP + 2);

        colorH = *(bP + Nextline - 1);
        colorC = *(bP + Nextline);
        colorD = *(bP + Nextline + 1);
        colorL = *(bP + Nextline + 2);

        colorM = *(bP + Nextline + Nextline - 1);
        colorN = *(bP + Nextline + Nextline);
        colorO = *(bP + Nextline + Nextline + 1);
        colorP = *(bP + Nextline + Nextline + 2);

        if ((colorA == colorD) && (colorB != colorC)) {
          if (((colorA == colorE) && (colorB == colorL)) ||
              ((colorA == colorC) && (colorA == colorF)
               && (colorB != colorE) && (colorB == colorJ))) {
            product = colorA;
          } else {
            product = INTERPOLATE (colorA, colorB);
          }

          if (((colorA == colorG) && (colorC == colorO)) ||
              ((colorA == colorB) && (colorA == colorH)
               && (colorG != colorC) && (colorC == colorM))) {
            product1 = colorA;
          } else {
            product1 = INTERPOLATE (colorA, colorC);
          }
          product2 = colorA;
        } else if ((colorB == colorC) && (colorA != colorD)) {
          if (((colorB == colorF) && (colorA == colorH)) ||
              ((colorB == colorE) && (colorB == colorD)
               && (colorA != colorF) && (colorA == colorI))) {
            product = colorB;
          } else {
            product = INTERPOLATE (colorA, colorB);
          }

          if (((colorC == colorH) && (colorA == colorF)) ||
              ((colorC == colorG) && (colorC == colorD)
               && (colorA != colorH) && (colorA == colorI))) {
            product1 = colorC;
          } else {
            product1 = INTERPOLATE (colorA, colorC);
          }
          product2 = colorB;
        } else if ((colorA == colorD) && (colorB == colorC)) {
          if (colorA == colorB) {
            product = colorA;
            product1 = colorA;
            product2 = colorA;
          } else {
            register int r = 0;

            product1 = INTERPOLATE (colorA, colorC);
            product = INTERPOLATE (colorA, colorB);

            r += GetResult1 (colorA, colorB, colorG, colorE, colorI);
            r += GetResult2 (colorB, colorA, colorK, colorF, colorJ);
            r += GetResult2 (colorB, colorA, colorH, colorN, colorM);
            r += GetResult1 (colorA, colorB, colorL, colorO, colorP);

            if (r > 0)
              product2 = colorA;
            else if (r < 0)
              product2 = colorB;
            else {
              product2 = Q_INTERPOLATE (colorA, colorB, colorC, colorD);
            }
          }
        } else {
          product2 = Q_INTERPOLATE (colorA, colorB, colorC, colorD);

          if ((colorA == colorC) && (colorA == colorF)
              && (colorB != colorE) && (colorB == colorJ)) {
            product = colorA;
          } else if ((colorB == colorE) && (colorB == colorD)
                     && (colorA != colorF) && (colorA == colorI)) {
            product = colorB;
          } else {
            product = INTERPOLATE (colorA, colorB);
          }

          if ((colorA == colorB) && (colorA == colorH)
              && (colorG != colorC) && (colorC == colorM)) {
            product1 = colorA;
          } else if ((colorC == colorG) && (colorC == colorD)
                     && (colorA != colorH) && (colorA == colorI)) {
            product1 = colorC;
          } else {
            product1 = INTERPOLATE (colorA, colorC);
          }
        }

#ifdef WORDS_BIGENDIAN
        product = (colorA << 16) | product ;
        product1 = (product1 << 16) | product2 ;
#else
        product = colorA | (product << 16);
        product1 = product1 | (product2 << 16);
#endif
        *((int *) dP) = product;
        *((unsigned int *) (dP + dstPitch)) = product1;

        bP += inc_bP;
        dP += sizeof (unsigned int);
      }                 // end of for ( finish= width etc..)

      srcPtr += srcPitch;
      dstPtr += dstPitch * 2;
//      deltaPtr += srcPitch;
    }                   // endof: for (height; height; height--)
  }
}

void _2xSaI32 (unsigned char *srcPtr, unsigned int srcPitch, unsigned char * /* deltaPtr */,
               unsigned char *dstPtr, unsigned int dstPitch, int width, int height)
{
  unsigned int  *dP;
  unsigned int *bP;
  unsigned int inc_bP = 1;

  unsigned int Nextline = srcPitch >> 2;

  for (; height; height--) {
    bP = (unsigned int *) srcPtr;
    dP = (unsigned int *) dstPtr;

    for (unsigned int finish = width; finish; finish -= inc_bP) {
      register unsigned int colorA, colorB;
      unsigned int colorC, colorD,
        colorE, colorF, colorG, colorH,
        colorI, colorJ, colorK, colorL,

        colorM, colorN, colorO, colorP;
      unsigned int product, product1, product2;

      //---------------------------------------
      // Map of the pixels:                    I|E F|J
      //                                       G|A B|K
      //                                       H|C D|L
      //                                       M|N O|P
      colorI = *(bP - Nextline - 1);
      colorE = *(bP - Nextline);
      colorF = *(bP - Nextline + 1);
      colorJ = *(bP - Nextline + 2);

      colorG = *(bP - 1);
      colorA = *(bP);
      colorB = *(bP + 1);
      colorK = *(bP + 2);

      colorH = *(bP + Nextline - 1);
      colorC = *(bP + Nextline);
      colorD = *(bP + Nextline + 1);
      colorL = *(bP + Nextline + 2);

      colorM = *(bP + Nextline + Nextline - 1);
      colorN = *(bP + Nextline + Nextline);
      colorO = *(bP + Nextline + Nextline + 1);
      colorP = *(bP + Nextline + Nextline + 2);

      if ((colorA == colorD) && (colorB != colorC)) {
        if (((colorA == colorE) && (colorB == colorL)) ||
            ((colorA == colorC) && (colorA == colorF)
             && (colorB != colorE) && (colorB == colorJ))) {
          product = colorA;
        } else {
          product = INTERPOLATE (colorA, colorB);
        }

        if (((colorA == colorG) && (colorC == colorO)) ||
            ((colorA == colorB) && (colorA == colorH)
             && (colorG != colorC) && (colorC == colorM))) {
          product1 = colorA;
        } else {
          product1 = INTERPOLATE (colorA, colorC);
        }
        product2 = colorA;
      } else if ((colorB == colorC) && (colorA != colorD)) {
        if (((colorB == colorF) && (colorA == colorH)) ||
            ((colorB == colorE) && (colorB == colorD)
             && (colorA != colorF) && (colorA == colorI))) {
          product = colorB;
        } else {
          product = INTERPOLATE (colorA, colorB);
        }

        if (((colorC == colorH) && (colorA == colorF)) ||
            ((colorC == colorG) && (colorC == colorD)
             && (colorA != colorH) && (colorA == colorI))) {
          product1 = colorC;
        } else {
          product1 = INTERPOLATE (colorA, colorC);
        }
        product2 = colorB;
      } else if ((colorA == colorD) && (colorB == colorC)) {
        if (colorA == colorB) {
          product = colorA;
          product1 = colorA;
          product2 = colorA;
        } else {
          register int r = 0;

          product1 = INTERPOLATE (colorA, colorC);
          product = INTERPOLATE (colorA, colorB);

          r += GetResult1 (colorA, colorB, colorG, colorE, colorI);
          r += GetResult2 (colorB, colorA, colorK, colorF, colorJ);
          r += GetResult2 (colorB, colorA, colorH, colorN, colorM);
          r += GetResult1 (colorA, colorB, colorL, colorO, colorP);

          if (r > 0)
            product2 = colorA;
          else if (r < 0)
            product2 = colorB;
          else {
            product2 = Q_INTERPOLATE (colorA, colorB, colorC, colorD);
          }
        }
      } else {
        product2 = Q_INTERPOLATE (colorA, colorB, colorC, colorD);

        if ((colorA == colorC) && (colorA == colorF)
            && (colorB != colorE) && (colorB == colorJ)) {
          product = colorA;
        } else if ((colorB == colorE) && (colorB == colorD)
                   && (colorA != colorF) && (colorA == colorI)) {
          product = colorB;
        } else {
          product = INTERPOLATE (colorA, colorB);
        }

        if ((colorA == colorB) && (colorA == colorH)
            && (colorG != colorC) && (colorC == colorM)) {
          product1 = colorA;
        } else if ((colorC == colorG) && (colorC == colorD)
                   && (colorA != colorH) && (colorA == colorI)) {
          product1 = colorC;
        } else {
          product1 = INTERPOLATE (colorA, colorC);
        }
      }
      *(dP) = colorA;
      *(dP + 1) = product;
      *(dP + (dstPitch >> 2)) = product1;
      *(dP + (dstPitch >> 2) + 1) = product2;

      bP += inc_bP;
      dP += 2;
    }                 // end of for ( finish= width etc..)

    srcPtr += srcPitch;
    dstPtr += dstPitch * 2;
//    deltaPtr += srcPitch;
  }                   // endof: for (height; height; height--)
}
