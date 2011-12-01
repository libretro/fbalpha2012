// PSGL driver
#ifndef _VID_PSGL_H_
#define _VID_PSGL_H_

#include <PSGL/psgl.h>
#include <PSGL/psglu.h>
#include <cell/dbgfont.h>
#include "burnint.h"
#include "vid_support-ps3.h"

enum menu_type
{
	TEXTURE_BACKDROP,
	TEXTURE_MENU
};

extern void psglSetVSync(uint32_t enable);
extern void psglInitGL(void);
extern void dbgFontInit(void);
extern void psglResolutionPrevious(void);
extern void psglResolutionNext(void);
extern int32_t psglInitShader(const char* filename, unsigned index);
extern uint32_t psglGetCurrentResolutionId(void);
extern void psglResolutionSwitch(void);
extern void psglRenderStretch(void);
extern void psglRenderPaused(void);			 
extern void psglRenderAlpha(void);
extern void CalculateViewports(void);
extern void psglRender(void);
extern void setlinear(unsigned int smooth);
bool LoadMenuTexture(enum menu_type type, const char * path);

extern uint32_t m_overscan;
extern float m_overscan_amount;
extern struct BurnDriver * pDriver[];

#define VidFrame() \
	uint32_t current_selected_game_index = nBurnDrvSelect; \
	pBurnDraw = pVidImage; \
	nBurnPitch = nVidImagePitch; \
	pDriver[current_selected_game_index]->Frame(); \
	psglRender();

#define psglClearUI() glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

#define psglRenderUI() psglSwap(); cellSysutilCheckCallback();

#endif
