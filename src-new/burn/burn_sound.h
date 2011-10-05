// burn_sound.h - General sound support functions
// based on code by Daniel Moreno (ComaC) < comac2k@teleline.es >

extern "C" {
	int ChannelMix_QS_A(int* Dest, int nLen,
								char* Sample, int LoopEnd,
								int* Pos,
								int VolL, int VolR,
								int LoopLen,
								int IncPos,
								char* EndBuff);

	void BurnSoundCopyClamp_A(int* Src, short* Dest, int Len);
	void BurnSoundCopyClamp_Add_A(int* Src, short* Dest, int Len);

	void BurnSoundCopyClamp_Mono_A(int* Src, short* Dest, int Len);
	void BurnSoundCopyClamp_Mono_Add_A(int* Src, short* Dest, int Len);

	void BurnSoundCopy_FM_A(short* SrcL, short* SrcR, short* Dest, int Len, int VolL, int VolR);
	void BurnSoundCopy_FM_Add_A(short* SrcL, short* SrcR, short* Dest, int Len, int VolL, int VolR);

	/* SrcOPN should have left channel data at SrcOPN, right channel at SrcOPN + 4096, SrcPSG should have all summed channels */
	void BurnSoundCopy_FM_OPN_A(short* SrcOPN, int* SrcPSG, short* Dest, int Len, int VolPSGL, int VolPSGR);
	void BurnSoundCopy_FM_OPN_Add_A(short* SrcOPN, int* SrcPSG, short* Dest, int Len, int VolPSGL, int VolPSGR);
}

void BurnSoundCopyClamp_C(int* Src, short* Dest, int Len);
void BurnSoundCopyClamp_Add_C(int* Src, short* Dest, int Len);
void BurnSoundCopyClamp_Mono_C(int* Src, short* Dest, int Len);
void BurnSoundCopyClamp_Mono_Add_C(int* Src, short* Dest, int Len);

extern int cmc_4p_Precalc();

#ifdef __ELF__
 #define Precalc _Precalc
#endif

extern short Precalc[4096 * 4];

#define INTERPOLATE4PS_8BIT(fp, sN, s0, s1, s2)      (((int)((sN) * Precalc[(int)(fp) * 4 + 0]) + (int)((s0) * Precalc[(int)(fp) * 4 + 1]) + (int)((s1) * Precalc[(int)(fp) * 4 + 2]) + (int)((s2) * Precalc[(int)(fp) * 4 + 3])) / 64)
#define INTERPOLATE4PS_16BIT(fp, sN, s0, s1, s2)     (((int)((sN) * Precalc[(int)(fp) * 4 + 0]) + (int)((s0) * Precalc[(int)(fp) * 4 + 1]) + (int)((s1) * Precalc[(int)(fp) * 4 + 2]) + (int)((s2) * Precalc[(int)(fp) * 4 + 3])) / 16384)
#define INTERPOLATE4PS_CUSTOM(fp, sN, s0, s1, s2, v) (((int)((sN) * Precalc[(int)(fp) * 4 + 0]) + (int)((s0) * Precalc[(int)(fp) * 4 + 1]) + (int)((s1) * Precalc[(int)(fp) * 4 + 2]) + (int)((s2) * Precalc[(int)(fp) * 4 + 3])) / (int)(v))

#define INTERPOLATE4PU_8BIT(fp, sN, s0, s1, s2)      (((unsigned int)((sN) * Precalc[(int)(fp) * 4 + 0]) + (unsigned int)((s0) * Precalc[(int)(fp) * 4 + 1]) + (unsigned int)((s1) * Precalc[(int)(fp) * 4 + 2]) + (unsigned int)((s2) * Precalc[(int)(fp) * 4 + 3])) / 64)
#define INTERPOLATE4PU_16BIT(fp, sN, s0, s1, s2)     (((unsigned int)((sN) * Precalc[(int)(fp) * 4 + 0]) + (unsigned int)((s0) * Precalc[(int)(fp) * 4 + 1]) + (unsigned int)((s1) * Precalc[(int)(fp) * 4 + 2]) + (unsigned int)((s2) * Precalc[(int)(fp) * 4 + 3])) / 16384)
#define INTERPOLATE4PU_CUSTOM(fp, sN, s0, s1, s2, v) (((unsigned int)((sN) * Precalc[(int)(fp) * 4 + 0]) + (unsigned int)((s0) * Precalc[(int)(fp) * 4 + 1]) + (unsigned int)((s1) * Precalc[(int)(fp) * 4 + 2]) + (unsigned int)((s2) * Precalc[(int)(fp) * 4 + 3])) / (unsigned int)(v))
