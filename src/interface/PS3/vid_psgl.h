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
extern void setlinear(unsigned int smooth);

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
cgGLSetParameter2f(cg_video_size, nVidImageWidth, nVidImageHeight); \
cgGLSetParameter2f(cg_texture_size, nVidImageWidth, nVidImageHeight); \
cgGLSetParameter2f(cg_output_size, cg_viewport_width, cg_viewport_height); \
cgGLSetParameter2f(cg_v_video_size, nVidImageWidth, nVidImageHeight); \
cgGLSetParameter2f(cg_v_texture_size, nVidImageWidth, nVidImageHeight); \
cgGLSetParameter2f(cg_v_output_size, cg_viewport_width, cg_viewport_height); \
cgGLSetParameter1f(cgp_timer, frame_count); \
cgGLSetParameter1f(cgp_vertex_timer, frame_count);

#define glCalculateViewport(outwidth, outheight) \
   float device_aspect = psglGetDeviceAspectRatio(psgl_device); \
   glMatrixMode(GL_PROJECTION); \
   glLoadIdentity(); \
   GLfloat left = 0.0f; \
   GLfloat right = (float)outwidth; \
   GLfloat bottom = 0.0f; \
   GLfloat top = (float)outheight; \
   GLfloat zNear = -1.0f; \
   GLfloat zFar = 1.0f; \
   float desired_aspect = vidScrnAspect; \
   GLuint lowerleft_x = 0; \
   GLuint lowerleft_y = 0; \
   GLuint viewport_width, viewport_height;  \
   GLuint real_width = gl_width; \
   GLuint real_height = gl_height; \
   if(custom_aspect_ratio_mode) \
   { \
      lowerleft_x = nXOffset; \
      lowerleft_y = nYOffset; \
      real_width = gl_width + nXScale; \
      real_height = gl_height + nYScale; \
      left = nXOffset; \
      right = nYOffset; \
      bottom = gl_height + nYOffset; \
   } \
   else if ( (int)(device_aspect*1000) > (int)(desired_aspect*1000) ) \
   { \
      float delta = (desired_aspect / device_aspect - 1.0) / 2.0 + 0.5; \
      lowerleft_x = gl_width * (0.5 - delta); \
      real_width = (2.0 * gl_width * delta); \
   } \
   else if ( (int)(device_aspect*1000) < (int)(desired_aspect*1000) ) \
   { \
      float delta = (device_aspect / desired_aspect - 1.0) / 2.0 + 0.5; \
      lowerleft_y = gl_height * (0.5 - delta); \
      real_height = (2.0 * gl_height * delta); \
   } \
   glViewport(lowerleft_x, lowerleft_y, real_width, real_height); \
   cg_viewport_width = real_width; \
   cg_viewport_height = real_height; \
   cgGLSetStateMatrixParameter(ModelViewProj_cgParam, CG_GL_MODELVIEW_PROJECTION_MATRIX, CG_GL_MATRIX_IDENTITY); \
   glOrthof(left, right, bottom, top, zNear, zFar); \
   glClearColor(0.0f, 0.0f, 0.0f, 1.0f); \
   glClear(GL_COLOR_BUFFER_BIT); \
   glMatrixMode(GL_MODELVIEW); \
   glLoadIdentity();

#endif
