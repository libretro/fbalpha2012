#ifndef INTERFACE_H
#define INTERFACE_H

#ifdef FBA_DEBUG
 #define PRINT_DEBUG_INFO
#endif

// GameInp structure
#include "gameinp.h"

extern bool bInputOkay;

// Input

int InputInit();
int InputExit();
void InputMake(void);

// Audio
extern int		nAudSampleRate;		// sample rate
extern int		nAudSegCount;		// Segments in the pdsbLoop buffer
extern int		nAudSegLen;		// Segment length in samples (calculated from sound rate/FPS)
extern int		nAudAllocSegLen;	// Allocated segment length in samples
extern int16_t *	pAudNextSound;		// The next sound segment we will add to the sample loop
extern bool		bAudOkay;		// True if sound was inited okay
extern unsigned char *	pVidTransImage;

// Video
extern bool		bVidOkay;
extern bool		bVidRecalcPalette;
extern int		bVidCorrectAspect;
extern int		nVidRotationAdjust;
extern int		bVidTripleBuffer;
extern int		bVidVSync;
extern unsigned int vidFilterLinear;
extern int		nVidScrnAspectX;
extern int		nVidScrnAspectY;
extern float		vidScrnAspect;
extern bool		autoVidScrnAspect;
extern unsigned char*	pVidImage;
extern int		nVidImageWidth;
extern int		nVidImageHeight;
extern int		nVidImageLeft;
extern int		nVidImageTop;
extern int		nVidImagePitch;
extern int		shaderindex;

int VidInit();
int VidExit();
int VidFrame_RecalcPalette();
int VidFrame_Recalc();
int VidFrame();
int VidRecalcPal();
extern int _psglInit(void);
extern int _psglExit(void);


extern "C" unsigned int (__cdecl *VidHighCol) (int r, int g, int b, int i);

#endif
