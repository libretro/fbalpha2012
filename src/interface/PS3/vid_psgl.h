// PSGL driver
#ifndef _VID_PSGL_H_
#define _VID_PSGL_H_

#include <PSGL/psgl.h>
#include <PSGL/psglu.h>
#include <cell/dbgfont.h>
#include "vid_support-ps3.h"

extern void psglSetVSync(uint32_t enable);
extern void psglInitGL(void);
extern void dbgFontInit(void);
extern void psglResolutionPrevious(void);
extern void psglResolutionNext(void);
extern int32_t psglInitShader(const char* filename);
extern uint32_t psglGetCurrentResolutionId(void);
extern void psglResolutionSwitch(void);
extern void psglRenderStretch(void);
extern void psglRenderPaused(void);			 
extern void psglRenderAlpha(void);
extern void CalculateViewports(void);
extern void psglRender(void);

#define psglRedraw() \
	if(pVidTransImage) \
	{ \
		VidFrame_Recalc(); \
	} \
	else \
	{ \
		VidFrame(); \
	}

#define psglClearUI() glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

#define psglRenderUI() \
	psglSwap(); \
	cellSysutilCheckCallback();

#define set_cg_params() \
cgGLSetStateMatrixParameter(ModelViewProj_cgParam, CG_GL_MODELVIEW_PROJECTION_MATRIX, CG_GL_MATRIX_IDENTITY); \
cgGLSetParameter2f(cg_video_size, nVidImageWidth, nVidImageHeight); \
cgGLSetParameter2f(cg_texture_size, nVidImageWidth, nVidImageHeight); \
cgGLSetParameter2f(cg_output_size, cg_viewport_width, cg_viewport_height); \
cgGLSetParameter2f(cg_v_video_size, nVidImageWidth, nVidImageHeight); \
cgGLSetParameter2f(cg_v_texture_size, nVidImageWidth, nVidImageHeight); \
cgGLSetParameter2f(cg_v_output_size, cg_viewport_width, cg_viewport_height); \
cgGLSetParameter1f(cgp_timer, frame_count); \
cgGLSetParameter1f(cgp_vertex_timer, frame_count);

#define setview(x, y, w, h, outwidth, outheight) \
   float device_aspect = psglGetDeviceAspectRatio(psgl_device); \
   glMatrixMode(GL_PROJECTION); \
   glLoadIdentity(); \
   GLfloat left = 0; \
   GLfloat right = outwidth; \
   GLfloat bottom = 0; \
   GLfloat top = outheight; \
   GLfloat zNear = -1.0; \
   GLfloat zFar = 1.0; \
   float desired_aspect = vidScrnAspect; \
   GLuint lowerleft_x, lowerleft_y, viewport_width, viewport_height;  \
   GLuint real_width = w, real_height = h; \
   if(custom_aspect_ratio_mode) \
   { \
      lowerleft_x = x + nXOffset; \
      lowerleft_y = y + nYOffset; \
      real_width = w + nXScale; \
      real_height = h + nYScale; \
      left = x + nXOffset; \
      right = y + nYOffset; \
      bottom = gl_height + nYOffset; \
   } \
   else if ( (int)(device_aspect*1000) > (int)(desired_aspect*1000) ) \
   { \
      float delta = (desired_aspect / device_aspect - 1.0) / 2.0 + 0.5; \
      lowerleft_x = w * (0.5 - delta); \
      lowerleft_y = 0; \
      real_width = (2.0 * w * delta); \
      real_height = h; \
   } \
   else if ( (int)(device_aspect*1000) < (int)(desired_aspect*1000) ) \
   { \
      float delta = (device_aspect / desired_aspect - 1.0) / 2.0 + 0.5; \
      lowerleft_x = 0; \
      lowerleft_y = h * (0.5 - delta); \
      real_width = w; \
      real_height = (2.0 * h * delta); \
   } \
   else  \
   { \
      lowerleft_x = 0; \
      lowerleft_y = 0; \
      real_width = w; \
      real_height = h; \
   } \
   glViewport(lowerleft_x, lowerleft_y, real_width, real_height); \
   cg_viewport_width = real_width; \
   cg_viewport_height = real_height; \
   cgGLSetStateMatrixParameter(ModelViewProj_cgParam, CG_GL_MODELVIEW_PROJECTION_MATRIX, CG_GL_MATRIX_IDENTITY); \
   /* glOrthof(left, right, bottom, top, zNear, zFar); */ \
   glMatrixMode(GL_MODELVIEW); \
   glLoadIdentity();

#endif
