// Video Output - (calls all the Vid Out plugins)
#include "burner.h"
#ifndef SN_TARGET_PS3
#include "effect/effect.h"
#include "vid_filter.h"
#else
#include "highcol.h"
#endif

#undef ENABLE_PREVIEW

static InterfaceInfo VidInfo = { NULL, };

unsigned int nVidSelect = 2;					// Which video output is selected
static unsigned int nVidActive = 0;				// Which video output is actived

bool bVidOkay = false;

#ifndef SN_TARGET_PS3
int nVidWidth = 640;
int nVidHeight = 480;
#endif

int nVidDepth = 16;
int nVidRefresh = 0;
int nVidFullscreen = 0;
int bVidFullStretch = 0;						// 1 = stretch to fill the entire window/screen
int bVidCorrectAspect = 1;						// 1 = stretch to fill the window/screen while maintaining the correct aspect ratio
int bVidVSync = 0;								// 1 = sync blits/pageflips/presents to the screen
int bVidTripleBuffer = 0;						// 1 = use triple buffering
int nVidRotationAdjust = 0;						// & 1: do not rotate the graphics for vertical games,  & 2: Reverse flipping for vertical games
int bVidForce16bit = 1;							// Emulate the game in 16-bit even when the screen is 32-bit (D3D blitter)
unsigned int vidFilterLinear = 0;				// 1 = linear filter, or point filter
unsigned int vidHardwareVertex = 0;				// 1 = use hardware vertex processing
unsigned int vidMotionBlur = 0;					// 1 = motion blur
unsigned int vidUseFilter = 0;
unsigned int vidForceFilterSize = 0;
#ifdef SN_TARGET_PS3
int nVidOriginalScrnAspectX;
int nVidOriginalScrnAspectY;
#endif

int nVidDXTextureManager = 0;					// How to transfer the game image to video memory and/or a texture --
												//  0 = blit from system memory / use driver/DirectX texture management
												//  1 = copy to a video memory surface, then use bltfast()
unsigned int nVid3DProjection = 0;				// Options for the 3D projection effct
float fVidScreenAngle = 0.174533f;				// The angle at which to tilt the screen backwards (in radians, D3D blitter)
float fVidScreenCurvature = 0.698132f;			// The angle of the maximum screen curvature (in radians, D3D blitter)

#if defined (_WIN32)
 HWND hVidWnd = NULL;							// Actual window used for video
#endif

int nVidScrnWidth = 0, nVidScrnHeight = 0;		// Actual Screen dimensions (0 if in windowed mode)
int nVidScrnDepth = 0;							// Actual screen depth

int nVidScrnAspectX = 4, nVidScrnAspectY = 3;	// Aspect ratio of the display screen
#ifdef SN_TARGET_PS3
int nVidScrnAspectMode = ASPECT_RATIO_4_3;
#endif
float vidScrnAspect = (float)4 / (float)3;				// Aspect ratio
extern bool autoVidScrnAspect = true;			// Automatical Aspect ratio

unsigned char* pVidImage = NULL;				// Memory buffer
int nVidImageWidth = DEFAULT_IMAGE_WIDTH;		// Memory buffer size
int nVidImageHeight = DEFAULT_IMAGE_HEIGHT;		//
int nVidImageLeft = 0, nVidImageTop = 0;		// Memory buffer visible area offsets
int nVidImagePitch = 0, nVidImageBPP = 0;		// Memory buffer pitch and bytes per pixel
int nVidImageDepth = 0;							// Memory buffer bits per pixel

int effect_depth = 16;

unsigned int nVidAdapter = 0;

unsigned int (__cdecl *VidHighCol) (int r, int g, int b, int i);
static bool bVidRecalcPalette;

static unsigned char* pVidTransImage = NULL;
static unsigned int* pVidTransPalette = NULL;
const int transPaletteSize = 65536;

int nXOffset = 0;
int nYOffset = 0;
int nXScale = 0;
int nYScale = 0;

static unsigned int __cdecl HighCol15(int r, int g, int b, int  /* i */)
{
#ifndef SN_TARGET_PS3
	colorAdjust(r, g, b);
#endif

	unsigned int t;
	t  = (r << 7) & 0x7C00;
	t |= (g << 2) & 0x03E0;
	t |= (b >> 3) & 0x001F;
	return t;
}

#if defined (SN_TARGET_PS3)
	extern struct VidOut VidOutPSGL;
#elif defined (_XBOX)
	extern struct VidOut VidOutD3D;
#elif defined (_WIN32)
	extern struct VidOut VidOutDDraw;
	extern struct VidOut VidOutD3D7;
	extern struct VidOut VidOutD3D;
	#ifndef NO_OPENGL
	extern struct VidOut VidOutWGL;
	#endif
#elif defined (BUILD_SDL)
	extern struct VidOut VidOutSDLFX;
	extern struct VidOut VidOutSDLOpenGL;
#endif

VidOut* VidDriver(unsigned int driver)
{
#if defined (SN_TARGET_PS3)
	if (driver == VID_PSGL)
		return &VidOutPSGL;
#elif defined (_XBOX)
	if (driver == VID_D3D)
		return &VidOutD3D;
#elif defined (_WIN32)
	if (driver >= VID_LEN)
		return &VidOutDDraw;
	else if (driver == VID_DDRAW)
		return &VidOutDDraw;
	else if (driver == VID_D3D7)
		return &VidOutD3D7;
	else if (driver == VID_D3D)
		return &VidOutD3D;
	#ifndef NO_OPENGL
	else if (driver == VID_WGL)
		return &VidOutWGL;
	#endif
 
#elif defined (BUILD_SDL)
	else if (driver == VID_SDLFX)
		return &VidOutSDLFX;
#endif

	   
}

int VidSelect(unsigned int driver)
{
	if (driver < VID_LEN) {
		nVidSelect = driver;
		return 0;
	}
	return 1;
}

// Forward to VidOut functions
int VidInit()
{
	VidExit();

	int nRet = 1;
	
	nShowEffect = 0;

#ifndef SN_TARGET_PS3
	if (!bDrvOkay && nShowEffect)
	{
		// for show intro
		if (nVidSelect < VID_LEN)
		{
			nVidActive = nVidSelect;
			if ((nRet = VidDriver(nVidActive)->Init()) == 0)
			{
				nBurnBpp = nVidImageBPP; // Set Burn library Bytes per pixel

				bVidOkay = true;

				effect_depth = nVidScrnDepth; // set effect depth

				if (nVidImageBPP > 2)
				{
					nBurnBpp = 2;

					pVidTransPalette = (unsigned int*)malloc(transPaletteSize * sizeof(int));
					pVidTransImage = (unsigned char*)malloc(nVidImageWidth * nVidImageHeight * sizeof(short));

					BurnHighCol = HighCol15;

					if (pVidTransPalette == NULL || pVidTransImage == NULL)
					{
						VidExit();
						nRet = 1;
					}
				}
			}
		}
	} else {
#endif
		// show skin

		HBITMAP hbitmap = NULL;


#if defined (_WIN32) && defined (ENABLE_PREVIEW)
		BITMAP bitmap;

		if (!bDrvOkay && bVidUsePlaceholder) {
			hbitmap = loadSkin(hScrnWnd);
			GetObject(hbitmap, sizeof(BITMAP), &bitmap);

			nVidImageWidth = bitmap.bmWidth; nVidImageHeight = bitmap.bmHeight;
			nVidImageLeft = nVidImageTop = 0;
		}
#endif
		
		if ((nVidSelect < VID_LEN) && (bDrvOkay || hbitmap)) {
			nVidActive = nVidSelect;						 
			if ((nRet = VidDriver(nVidActive)->Init()) == 0) {
				nBurnBpp = nVidImageBPP; // Set Burn library Bytes per pixel
				bVidOkay = true;

				if (bDrvOkay && (BurnDrvGetFlags() & BDF_16BIT_ONLY) && nVidImageBPP > 2) {
					nBurnBpp = 2;

					pVidTransPalette = (unsigned int*)malloc(transPaletteSize * sizeof(int));
#ifdef SN_TARGET_PS3
					pVidTransImage = (unsigned char*)malloc(nVidImageWidth * nVidImageHeight * (nVidImageBPP >> 1) * sizeof(short));
#else
					pVidTransImage = (unsigned char*)malloc(nVidImageWidth * nVidImageHeight * (nVidImageBPP / 2) * sizeof(short));
#endif

					BurnHighCol = HighCol15;

					if (pVidTransPalette == NULL || pVidTransImage == NULL) {
						VidExit();
						nRet = 1;
					}
				}
			}
		}

#if defined (_WIN32) && defined (ENABLE_PREVIEW)
		if (bVidOkay && hbitmap) {
			BITMAPINFO bitmapinfo;
			unsigned char* pLineBuffer = (unsigned char*)malloc(bitmap.bmWidth * 4);
			HDC hDC = GetDC(hVidWnd);

			if (hDC && pLineBuffer)
			{
				memset(&bitmapinfo, 0, sizeof(BITMAPINFO));
				bitmapinfo.bmiHeader.biSize = sizeof(BITMAPINFO);
				bitmapinfo.bmiHeader.biWidth = bitmap.bmWidth;
				bitmapinfo.bmiHeader.biHeight = bitmap.bmHeight;
				bitmapinfo.bmiHeader.biPlanes = 1;
				bitmapinfo.bmiHeader.biBitCount = 24;
				bitmapinfo.bmiHeader.biCompression = BI_RGB;

				for (int y = 0; y < nVidImageHeight; y++)
				{
					unsigned char* pd = pVidImage + y * nVidImagePitch;
					unsigned char* ps = pLineBuffer;

					GetDIBits(hDC, hbitmap, nVidImageHeight - 1 - y, 1, ps, &bitmapinfo, DIB_RGB_COLORS);

					for (int x = 0; x < nVidImageWidth; x++, ps += 3)
					{
						unsigned int nColour = VidHighCol(ps[2], ps[1], ps[0], 0);
						switch (nVidImageBPP)
						{
							case 2:
								*((unsigned short*)pd) = (unsigned short)nColour;
								pd += 2;
								break;
							case 3:
								pd[0] = (nColour >> 16) & 0xFF;
								ps[1] = (nColour >>  8) & 0xFF;
								pd[2] = (nColour >>  0) & 0xFF;
								pd += 3;
								break;
							case 4:
								*((unsigned int*)pd) = nColour;
								pd += 4;
								break;
						}
					}
				}
			}
			if (hDC)
				ReleaseDC(hVidWnd, hDC);

			free(pLineBuffer);
		}

		if (hbitmap)
			DeleteObject(hbitmap);
#endif

#ifndef SN_TARGET_PS3
	}
#endif

	return nRet;
}

int VidExit()
{
	IntInfoFree(&VidInfo);

	if (!bVidOkay)
		return 1;

	int nRet = VidDriver(nVidActive)->Exit();

#if defined (_WIN32)
	hVidWnd = NULL;
#endif

	bVidOkay = false;

	nVidImageWidth = DEFAULT_IMAGE_WIDTH;
	nVidImageHeight = DEFAULT_IMAGE_HEIGHT;

	nVidImageBPP = nVidImageDepth = 0;
	nBurnPitch = nBurnBpp = 0;

	free(pVidTransPalette);
	pVidTransPalette = NULL;
	free(pVidTransImage);
	pVidTransImage = NULL;

	return nRet;
}

#ifdef SN_TARGET_PS3
void CalculateViewports()
{
   extern void CalculateViewport();
   CalculateViewport();
}
#endif

#ifdef SN_TARGET_PS3
#define VidDoFrame(bRedraw) \
if (pVidTransImage) \
{ \
	unsigned short* pSrc = (unsigned short*)pVidTransImage; \
	unsigned char* pDest = pVidImage; \
	\
	if (bVidRecalcPalette) \
	{ \
		uint64_t r = 0; \
		do{ \
			uint64_t g = 0; \
			do{ \
				uint64_t b = 0; \
				do{ \
					uint64_t r_ = r | (r >> 5); \
					uint64_t g_ = g | (g >> 5); \
					uint64_t b_ = b | (b >> 5); \
					pVidTransPalette[(r << 7) | (g << 2) | (b >> 3)] = ARGB(r_,g_,b_); \
					b += 8; \
				}while(b < 256); \
				g += 8; \
			}while(g < 256); \
			r += 8; \
		}while(r < 256); \
		\
		bVidRecalcPalette = false; \
	} \
	\
	pBurnDraw = pVidTransImage; \
	nBurnPitch = nVidImageWidth << 1; \
	\
	extern void _psglRender();  \
	BurnDrvFrame(); \
	_psglRender(); \
	\
	/* set avi buffer, modified by regret */ \
	\
	pBurnDraw = NULL; \
	nBurnPitch = 0; \
	\
	int y = 0; \
	do{ \
		int x = 0; \
		do{ \
			((unsigned int*)pDest)[x] = pVidTransPalette[pSrc[x]]; \
			x++; \
		}while(x < nVidImageWidth); \
		y++; \
		pSrc += nVidImageWidth; \
		pDest += nVidImagePitch; \
	}while(y < nVidImageHeight); \
} \
   else \
   { \
		pBurnDraw = pVidImage; \
		nBurnPitch = nVidImagePitch; \
      \
		extern void _psglRender();  \
		BurnDrvFrame(); \
		_psglRender(); \
      \
		/* set avi buffer, modified by regret */ \
      \
		pBurnDraw = NULL; \
		nBurnPitch = 0; \
	} \
	return 0;
#else
static int VidDoFrame(bool bRedraw)
{
	int nRet;

	if (pVidTransImage)
	{
		unsigned short* pSrc = (unsigned short*)pVidTransImage;
		unsigned char* pDest = pVidImage;

		if (bVidRecalcPalette)
		{
			for (int r = 0; r < 256; r += 8)
			{
				for (int g = 0; g < 256; g += 8)
				{
					for (int b = 0; b < 256; b += 8)
						pVidTransPalette[(r << 7) | (g << 2) | (b >> 3)] = VidHighCol(r | (r >> 5), g | (g >> 5), b | (b >> 5), 0);
				}
			}

			bVidRecalcPalette = false;
		}

		pBurnDraw = pVidTransImage;
		nBurnPitch = nVidImageWidth * 2;

#if defined (_XBOX)
		extern inline int dx9Frame(bool bRedraw);
		nRet = dx9Frame(bRedraw);
#else
		nRet = VidDriver(nVidActive)->Frame(bRedraw);
#endif

		// set avi buffer, modified by regret

		pBurnDraw = NULL;
		nBurnPitch = 0;

		switch (nVidImageBPP)
		{
			case 3:
				for (int y = 0; y < nVidImageHeight; y++, pSrc += nVidImageWidth, pDest += nVidImagePitch)
				{
					for (int x = 0; x < nVidImageWidth; x++)
					{
						unsigned int c = pVidTransPalette[pSrc[x]];
						*(pDest + (x * 3) + 0) = c & 0xFF;
						*(pDest + (x * 3) + 1) = (c >> 8) & 0xFF;
						*(pDest + (x * 3) + 2) = c >> 16;
					}
				}
				break;
			case 4:
				for (int y = 0; y < nVidImageHeight; y++, pSrc += nVidImageWidth, pDest += nVidImagePitch)
				{
					for (int x = 0; x < nVidImageWidth; x++)
						((unsigned int*)pDest)[x] = pVidTransPalette[pSrc[x]];
				}
				break;
		}
	}
	else
	{
		pBurnDraw = pVidImage;
		nBurnPitch = nVidImagePitch;

#if defined (_XBOX)
		extern inline int dx9Frame(bool bRedraw);
		nRet = dx9Frame(bRedraw);
#else
		nRet = VidDriver(nVidActive)->Frame(bRedraw);
#endif

		// set avi buffer, modified by regret

		pBurnDraw = NULL;
		nBurnPitch = 0;
	}

	return nRet;
}
#endif

#ifdef SN_TARGET_PS3
int VidFrame()
{
	VidDoFrame(0);
}
#else
int VidFrame()
{
	// ==> Update effect, added by regret
	if (nShowEffect && bVidOkay)
		return VidDoFrame(0);
	// <== Update effect

	if (bVidOkay && bDrvOkay)
		return VidDoFrame(0);
	else
		return 1;
}
#endif

#ifdef SN_TARGET_PS3
int VidRedraw()
{
	VidDoFrame(1);
}
#else
int VidRedraw()
{
	if (bVidOkay /* && bDrvOkay */)
		return VidDoFrame(1);
	else
		return 1;
}
#endif

int VidRecalcPal()
{
	bVidRecalcPalette = true;

	return BurnRecalcPal();
}

// If bValidate & 1, the video code should use ValidateRect() to validate the rectangle it draws.
#ifndef SN_TARGET_PS3
int VidPaint(int bValidate)
{
	if (bVidOkay /* && bDrvOkay */)
		return VidDriver(nVidActive)->Paint(bValidate);
	else
		return 1;
}
#endif

// reinit video, added by regret

#ifdef SN_TARGET_PS3
int VidReinit()
{
	VidInit();

	if (bRunPause || !bDrvOkay)
		VidRedraw();

	CalculateViewports();
	return 0;
}
#else
int VidReinit()
{
	VidInit();
	if (!bVidOkay)
		return 1;

	if (bRunPause || !bDrvOkay)
   {
		VidRedraw();
	}
	return 0;
}
#endif

#ifndef  SN_TARGET_PS3
int VidScale(RECT* pRect, int nGameWidth, int nGameHeight)
{
 	if (bVidOkay)
      return VidDriver(nVidActive)->Scale(pRect, nGameWidth, nGameHeight);
   else
		return VidDriver(nVidSelect)->Scale(pRect, nGameWidth, nGameHeight);
}
#endif

const TCHAR* VidDriverName(unsigned int driver)
{
	if (driver >= VID_LEN)
		return FBALoadStringEx(1);

#if defined (_WIN32)
	else if (driver == VID_DDRAW)
		return _T("DirectDraw");
	else if (driver == VID_D3D7)
		return _T("Direct3D7");
	else if (driver == VID_D3D)
		return _T("Direct3D9");
#ifndef NO_OPENGL
	else if (driver == VID_WGL)
		return _T("OpenGL");
#endif
#elif defined (BUILD_SDL)
	else if (driver == VID_SDLFX)
		return _T("SDLFX");
#endif

	return FBALoadStringEx(1);
}

#ifdef SN_TARGET_PS3
const TCHAR* VidGetName()
{
	return VidDriverName(nVidActive);
}
#else
const TCHAR* VidGetName()
{
	if (bVidOkay)
		return VidDriverName(nVidActive);
	else
		return VidDriverName(nVidSelect);
}
#endif

InterfaceInfo* VidGetInfo()
{
	if (IntInfoInit(&VidInfo))
	{
		IntInfoFree(&VidInfo);
		return NULL;
	}

	if (bVidOkay) {
		TCHAR szString[MAX_PATH] = _T("");
		RECT rect;

		VidInfo.pszModuleName = VidGetName();

#if defined (_WIN32)
		getClientScreenRect(hVidWnd, &rect);
		if (nVidFullscreen == 0) {
			_sntprintf(szString, sizearray(szString), _T("Running in windowed mode, %ix%i, %ibpp"), rect.right - rect.left, rect.bottom - rect.top, nVidScrnDepth);
		}
		else
		{
			_sntprintf(szString, sizearray(szString), _T("Running fullscreen, %ix%i, %ibpp, Using adapter %d"), nVidScrnWidth, nVidScrnHeight, nVidScrnDepth, nVidAdapter);
		}
#elif defined (BUILD_SDL)
		_sntprintf(szString, sizearray(szString), _T("Filler for fullscreen/windowed mode & image size"));
#endif

		IntInfoAddStringInterface(&VidInfo, szString);

		_sntprintf(szString, sizearray(szString), _T("Source image %ix%i, %ibpp"), nVidImageWidth, nVidImageHeight, nVidImageDepth);
		IntInfoAddStringInterface(&VidInfo, szString);

		if (pVidTransImage) {
			_sntprintf(szString, sizearray(szString), _T("Using generic software 15->%ibpp wrapper"), nVidImageDepth);
			IntInfoAddStringInterface(&VidInfo, szString);
		}

		if (bVidVSync) {
			_sntprintf(szString, sizearray(szString), _T("VSync enabled"));
			IntInfoAddStringInterface(&VidInfo, szString);
		}

#ifndef SN_TARGET_PS3
		if (vidUseFilter) {
			_sntprintf(szString, sizearray(szString), _T("Using pixel filter: %s (%ix zoom)"), VidFilterGetEffect(nVidFilter), nPreScaleZoom);
			IntInfoAddStringInterface(&VidInfo, szString);
		}
#endif

		if (VidDriver(nVidActive)->GetSetting)
			VidDriver(nVidActive)->GetSetting(&VidInfo);
	}
	else
		IntInfoAddStringInterface(&VidInfo, _T("Video plugin not initialised"));

	return &VidInfo;
}
