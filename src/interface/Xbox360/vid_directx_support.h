#ifndef VID_DIRECTX_SUPPORT_H
#define VID_DIRECTX_SUPPORT_H

#define DIRECTDRAW_VERSION 0x0700		// Use this DirectDraw version
#include <xtl.h>
#include "vid_support.h"

// General
void VidSExit();
 
#if 0
// Gamma controls
void VidSRestoreGamma();
int VidSUpdateGamma();
int VidSSetupGamma(IDirectDrawSurface7* pSurf);
#endif

// Select optimal full-screen resolution
struct VidSDisplayScoreInfo {
	// Filled in advance by the caller
	unsigned int nRequestedWidth; unsigned int nRequestedHeight; unsigned int nRequestedDepth; unsigned int nRequestedZoom;
	unsigned int nModeWidth; unsigned int nModeHeight; unsigned int nModeDepth;
	// Filled by the scoring routine
	unsigned int nBestWidth; unsigned int nBestHeight; unsigned int nBestDepth;
	unsigned int nBestScore;
};

int VidSScoreDisplayMode(VidSDisplayScoreInfo* pScoreInfo);
int VidSInitScoreInfo(VidSDisplayScoreInfo* pScoreInfo);

void VidSRestoreScreenMode();
int VidSEnterFullscreenMode(int nZoom, int nDepth);

// Text display
void VidSExitOSD();
int VidSInitOSD(int nFlags);
int VidSRestoreOSD();
 
// Chat display
#define MAX_CHAT_SIZE (128)
extern bool bEditActive;
extern bool bEditTextChanged;
extern TCHAR EditText[MAX_CHAT_SIZE + 1];
int VidSAddChatMsg(const TCHAR* pID, int nIDRGB, const TCHAR* pMain, int nMainRGB);

extern TCHAR OSDMsg[128];
extern unsigned int nOSDTimer;
extern TCHAR tinyMsg[64];
extern unsigned int nTinyTimer;

// osd text display for dx9, added by regret
void VidSKillOSDMsg();
void VidSKillTinyOSDMsg();

#endif
