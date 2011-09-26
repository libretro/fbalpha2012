#ifndef _EFFECT_H
#define _EFFECT_H


#define SCRW 304
#define SCRH 224

#define MAKE_COLOR_555(r, g, b) \
  ((WORD)( ((b) & 0x1F) | (((g) & 0x1F) << 5) | (((r) & 0x1F) << 10) ))

#define MAKE_COLOR_565(r, g, b) \
  ((WORD)( ((b) & 0x1F) | (((g) & 0x1F) << 6) | (((r) & 0x1F) << 11) ))

#define MAKE_COLOR(r, g, b) \
  ( (effect_depth != 15) ? MAKE_COLOR_555(r, g, b) : MAKE_COLOR_565(r, g, b) )


extern int effect_depth;
extern unsigned short* pEffect;

void UpdateEffect();
void UpdateTVEffect();
void UpdateSmokeEffect();
void UpdateBurnEffect();
void UpdateWaterEffect();

#endif // _EFFECT_H