// burn_sound.h - General sound support functions
// based on code by Daniel Moreno (ComaC) < comac2k@teleline.es >
#ifndef BURN_SOUND_H
#define BURN_SOUND_H

#include "fbatypes.h"

void BurnSoundCopyClamp_C(int* Src, int16_t * Dest, int Len);
void BurnSoundCopyClamp_Add_C(int* Src, int16_t * Dest, int Len);
void BurnSoundCopyClamp_Mono_C(int* Src, int16_t * Dest, int Len);
void BurnSoundCopyClamp_Mono_Add_C(int* Src, int16_t * Dest, int Len);
void BurnSoundCopy_FM_C(int16_t* SrcL, int16_t* SrcR, int16_t* Dest, int Len, int VolL, int VolR);
void BurnSoundCopy_FM_Add_C(int16_t* SrcL, int16_t* SrcR, int16_t* Dest, int Len, int VolL, int VolR);

extern int cmc_4p_Precalc();

#ifdef __ELF__
 #define Precalc _Precalc
#endif

extern "C" int16_t Precalc[];

#define INTERPOLATE4PS_8BIT(fp, sN, s0, s1, s2)      (((int)((sN) * Precalc[(int)(fp) * 4 + 0]) + (int)((s0) * Precalc[(int)(fp) * 4 + 1]) + (int)((s1) * Precalc[(int)(fp) * 4 + 2]) + (int)((s2) * Precalc[(int)(fp) * 4 + 3])) / 64)
#define INTERPOLATE4PS_16BIT(fp, sN, s0, s1, s2)     (((int)((sN) * Precalc[(int)(fp) * 4 + 0]) + (int)((s0) * Precalc[(int)(fp) * 4 + 1]) + (int)((s1) * Precalc[(int)(fp) * 4 + 2]) + (int)((s2) * Precalc[(int)(fp) * 4 + 3])) / 16384)
#define INTERPOLATE4PS_CUSTOM(fp, sN, s0, s1, s2, v) (((int)((sN) * Precalc[(int)(fp) * 4 + 0]) + (int)((s0) * Precalc[(int)(fp) * 4 + 1]) + (int)((s1) * Precalc[(int)(fp) * 4 + 2]) + (int)((s2) * Precalc[(int)(fp) * 4 + 3])) / (int)(v))

#define INTERPOLATE4PU_8BIT(fp, sN, s0, s1, s2)      (((unsigned int)((sN) * Precalc[(int)(fp) * 4 + 0]) + (unsigned int)((s0) * Precalc[(int)(fp) * 4 + 1]) + (unsigned int)((s1) * Precalc[(int)(fp) * 4 + 2]) + (unsigned int)((s2) * Precalc[(int)(fp) * 4 + 3])) / 64)
#define INTERPOLATE4PU_16BIT(fp, sN, s0, s1, s2)     (((unsigned int)((sN) * Precalc[(int)(fp) * 4 + 0]) + (unsigned int)((s0) * Precalc[(int)(fp) * 4 + 1]) + (unsigned int)((s1) * Precalc[(int)(fp) * 4 + 2]) + (unsigned int)((s2) * Precalc[(int)(fp) * 4 + 3])) / 16384)
#define INTERPOLATE4PU_CUSTOM(fp, sN, s0, s1, s2, v) (((unsigned int)((sN) * Precalc[(int)(fp) * 4 + 0]) + (unsigned int)((s0) * Precalc[(int)(fp) * 4 + 1]) + (unsigned int)((s1) * Precalc[(int)(fp) * 4 + 2]) + (unsigned int)((s2) * Precalc[(int)(fp) * 4 + 3])) / (unsigned int)(v))

#endif
