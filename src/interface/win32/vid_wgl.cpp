// OpenGL video output, added by regret
// TODO: osd
#ifndef NO_OPENGL

#include "burner.h"
#include "vid_support.h"
#include "vid_filter.h"

#include "vid_opengl.h"

#ifdef _MSC_VER
#pragma comment(lib, "opengl32")
#endif

static bool fullSwitch = false;

GLuint OpenGL::gltexture = 0;
unsigned int* OpenGL::buffer = 0;
unsigned OpenGL::iwidth = 0;
unsigned OpenGL::iheight = 0;

HDC display = 0;
HGLRC wglcontext = 0;

static int nGameImageWidth, nGameImageHeight;
static int nImageWidth, nImageHeight;

static RECT Dest;

// ----------------------------------------------------------------------------

//turn vsync on or off
typedef BOOL (APIENTRY *PFNWGLSWAPINTERVALFARPROC)(int);

static inline void setVSync(int interval)
{
	const char* extensions = (const char *)glGetString(GL_EXTENSIONS);

	if (strstr(extensions, "WGL_EXT_swap_control") == 0) {
		return;
	}

	PFNWGLSWAPINTERVALFARPROC wglSwapIntervalEXT = (PFNWGLSWAPINTERVALFARPROC)wglGetProcAddress("wglSwapIntervalEXT");
	if (wglSwapIntervalEXT) {
		wglSwapIntervalEXT(interval);
	}
}

// ----------------------------------------------------------------------------

static inline void wglRelease()
{
	OpenGL::term();

	if (wglcontext) {
		wglDeleteContext(wglcontext);
		wglcontext = 0;
	}
}

static inline int wglExit()
{
	VidFilterExit();

	wglRelease();

	VidSFreeVidImage();

	nRotateGame = 0;

	return 0;
}

static inline int wglTextureInit()
{
	if (nRotateGame & 1) {
		nVidImageWidth = nGameHeight;
		nVidImageHeight = nGameWidth;
	} else {
		nVidImageWidth = nGameWidth;
		nVidImageHeight = nGameHeight;
	}

	nGameImageWidth = nVidImageWidth;
	nGameImageHeight = nVidImageHeight;

	nVidImageDepth = nVidScrnDepth;

	nVidImageBPP = (nVidImageDepth + 7) >> 3;
	nBurnBpp = nVidImageBPP;					// Set Burn library Bytes per pixel

	// Use our callback to get colors:
	SetBurnHighCol(nVidImageDepth);

	// Make the normal memory buffer
	if (VidSAllocVidImage()) {
		wglExit();
		return 1;
	}

	unsigned int nTextureWidth = VidGetTextureSize(nGameImageWidth * nPreScaleZoom);
	unsigned int nTextureHeight = VidGetTextureSize(nGameImageHeight * nPreScaleZoom);

	OpenGL::resize(nTextureWidth, nTextureHeight);

	return 0;
}

static inline int wglInit()
{
	if (hScrnWnd == NULL) {
		return 1;
	}

	hVidWnd = hScrnWnd;

	// set fullscreen
	nVidScrnDepth = 32;
	nVidScrnWidth = nVidWidth; nVidScrnHeight = nVidHeight;
	if (nVidFullscreen) {
		if (scrnSetFull(nVidFullscreen)) {
			return 1;
		}
		fullSwitch = true;
	}
	else if (fullSwitch) {
		scrnSetFull(false);
		fullSwitch = false;
	}

	GLuint pixel_format;
	PIXELFORMATDESCRIPTOR pfd;
	memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
	pfd.nSize      = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion   = 1;
	pfd.dwFlags    = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;

	display = GetDC(hVidWnd);
	pixel_format = ChoosePixelFormat(display, &pfd);
	SetPixelFormat(display, pixel_format, &pfd);

	wglcontext = wglCreateContext(display);
	wglMakeCurrent(display, wglcontext);

	OpenGL::init();

	VidInitInfo();

	// for filter
	VidFilterParamInit();

	// Initialize the buffer surfaces
	if (wglTextureInit()) {
		wglExit();
		return 1;
	}

	if (nPreScaleEffect) {
		if (VidFilterInit(nPreScaleEffect, 0)) {
			if (VidFilterInit(0, 0)) {
				wglExit();
				return 1;
			}
		}
	}

	OpenGL::setlinear(vidFilterLinear);

	setVSync(bVidVSync);

	nImageWidth = nImageHeight = 0;

	return 0;
}

static inline int wglScale(RECT* pRect, int nWidth, int nHeight)
{
	if (vidUseFilter && vidForceFilterSize) {
		return VidFilterScale(pRect, nWidth, nHeight);
	}
	return VidSScaleImage(pRect, nWidth, nHeight);
}

static inline int wglRender()
{
	getClientScreenRect(hVidWnd, &Dest);

	int ww = Dest.right - Dest.left;
	int wh = Dest.bottom - Dest.top;

	wglScale(&Dest, nGameWidth, nGameHeight);

	int nNewImageWidth  = nRotateGame ? (Dest.bottom - Dest.top) : (Dest.right - Dest.left);
	int nNewImageHeight = nRotateGame ? (Dest.right - Dest.left) : (Dest.bottom - Dest.top);

	if (nImageWidth != nNewImageWidth || nImageHeight != nNewImageHeight) {
		nImageWidth  = nNewImageWidth;
		nImageHeight = nNewImageHeight;

		// Set the size of the image on the PC screen
		int vpx, vpy, vpw, vph;
		if (nVidFullscreen) {
			vpx = Dest.left;
			vpy = Dest.top;
			vpw = Dest.right - Dest.left;
			vph = Dest.bottom - Dest.top;
		} else {
			vpx = (ww - (Dest.right - Dest.left)) / 2;
			vpy = (wh - (Dest.bottom - Dest.top)) / 2;
			vpw = Dest.right - Dest.left;
			vph = Dest.bottom - Dest.top;
		}
		OpenGL::setview(vpx, vpy, vpw, vph, nImageWidth, nImageHeight);
	}

	unsigned int* pd;
	unsigned int pitch;
	OpenGL::lock(pd, pitch);

	if (nPreScaleEffect) {
		VidFilterApplyEffect((unsigned char*)pd, pitch);
	} else {
		unsigned char* ps = pVidImage + nVidImageLeft * nVidImageBPP;
		int s = nVidImageWidth * nVidImageBPP;

		VidSCpyImg32((unsigned char*)pd, pitch, ps, s, nVidImageWidth, nVidImageHeight);
	}

	unsigned int inwidth = nGameImageWidth;
	unsigned int inheight = nGameImageHeight;
#if 1
	if (nPreScaleEffect) {
		if (nPreScale & 1) {
			inwidth *= nPreScaleZoom;
		}
		if (nPreScale & 2) {
			inheight *= nPreScaleZoom;
		}
	}
#endif

	OpenGL::refresh(inwidth, inheight, nImageWidth, nImageHeight, nRotateGame);

	SwapBuffers(display);

	return 0;
}

// Run one frame and render the screen
static inline int wglFrame(bool bRedraw)			// bRedraw = 0
{
	if (pVidImage == NULL) {
		return 1;
	}

	if (bDrvOkay) {
		if (bRedraw) {						// Redraw current frame
			if (BurnDrvRedraw()) {
				BurnDrvFrame();				// No redraw function provided, advance one frame
			}
		} else {
			BurnDrvFrame();					// Run one frame and draw the screen
		}
	}

	wglRender();

	return 0;
}

static inline int wglPaint(int bValidate)
{
#if 1
	wglRender();
#else
	RECT rect = { 0, 0, 0, 0 };

	if (!nVidFullscreen) {
		getClientScreenRect(hVidWnd, &rect);

		wglScale(&rect, nGameWidth, nGameHeight);

		if ((rect.right - rect.left) != (Dest.right - Dest.left)
			|| (rect.bottom - rect.top ) != (Dest.bottom - Dest.top)) {
			bValidate |= 2;
		}
	}

	if (bValidate & 2) {
		wglRender();
	}

	if (bValidate & 1) {
		SwapBuffers(display);

#if 0
		POINT c = {0, 0};
		ClientToScreen(hVidWnd, &c);
		rect.left -= c.x; rect.right -= c.x;
		rect.top -= c.y; rect.bottom -= c.y;

		// Validate the rectangle we just drew
		ValidateRect(hVidWnd, &rect);
#endif
	}
#endif
	return 0;
}

// ----------------------------------------------------------------------------

static inline int wglGetSettings(InterfaceInfo* pInfo)
{
	TCHAR szString[MAX_PATH] = _T("");

	if (nVidFullscreen) {
		if (bVidTripleBuffer) {
			IntInfoAddStringModule(pInfo, _T("Using a triple buffer"));
		} else {
			IntInfoAddStringModule(pInfo, _T("Using a double buffer"));
		}
	}

	switch (vidFilterLinear) {
		case 0: {
			IntInfoAddStringModule(pInfo, _T("Applying pointer filter"));
			break;
		}
		case 1: {
			IntInfoAddStringModule(pInfo, _T("Applying linear filter"));
			break;
		}
		default: {
			IntInfoAddStringModule(pInfo, _T("Applying point filter"));
		}
	}

	return 0;
}

// The Video Output plugin:
struct VidOut VidOutWGL = { wglInit, wglExit, wglFrame, wglPaint, wglScale, wglGetSettings };

#endif
