// Direct3D9 video output
// rewritten by regret  (Motion Blur source from VBA-M)
#include "burner.h"
#include "vid_directx_support.h"
#include "vid_filter.h"

#ifdef _MSC_VER
#pragma comment(lib, "d3d9")
#pragma comment(lib, "d3dx9")
#endif

#include <d3d9.h>
#include <d3dx9effect.h>

const float PI = 3.14159265358979323846f;

struct d3dvertex {
	float x, y, z, rhw; //screen coords
	float u, v;         //texture coords
};

struct transp_vertex {
	float x, y, z, rhw;
	D3DCOLOR color;
	float u, v;
};

#define D3DFVF_LVERTEX2 (D3DFVF_XYZRHW | D3DFVF_TEX1)

static IDirect3D9* pD3D = NULL;				// Direct3D interface
static D3DPRESENT_PARAMETERS d3dpp;
static IDirect3DDevice9* pD3DDevice = NULL;

IDirect3DTexture9* emuTexture[2];
static unsigned char mbCurrentTexture = 0;	// current texture for motion blur
static bool mbTextureEmpty = true;
static d3dvertex vertex[4];
static transp_vertex transpVertex[4];

static IDirect3DTexture9* pTexture = NULL;
static int nTextureWidth = 0;
static int nTextureHeight = 0;
D3DFORMAT textureFormat;

static ID3DXFont* pFont = NULL;				// OSD font
static ID3DXFont* pTinyFont = NULL;			// tiny font
static D3DCOLOR osdColor = D3DCOLOR_ARGB(0xFF, 0xFF, 0xFF, 0xFF);

static int nGameImageWidth, nGameImageHeight;
static int nImageWidth, nImageHeight, nImageZoom;

#if 0
struct {
	unsigned int t_usage, v_usage;
	unsigned int t_pool,  v_pool;
	unsigned int lock;
	unsigned int filter;
} flags;

struct {
	bool dynamic;     //device supports dynamic textures
	bool stretchrect; //device supports StretchRect
} caps;
#endif

static RECT Dest;

// ----------------------------------------------------------------------------

static inline TCHAR* TextureFormatString(D3DFORMAT nFormat)
{
	switch (nFormat) {
		case D3DFMT_X1R5G5B5:
			return _T("16-bit xRGB 1555");
		case D3DFMT_R5G6B5:
			return _T("16-bit RGB 565");
		case D3DFMT_X8R8G8B8:
			return _T("32-bit xRGB 8888");
		case D3DFMT_A8R8G8B8:
			return _T("32-bit ARGB 8888");
		case D3DFMT_A16B16G16R16F:
			return _T("64-bit ARGB 16161616fp");
		case D3DFMT_A32B32G32R32F:
			return _T("128-bit ARGB 32323232fp");
	}

	return _T("unknown format");
}

// Select optimal full-screen resolution
static inline int dx9SelectFullscreenMode(VidSDisplayScoreInfo* pScoreInfo)
{
	pScoreInfo->nBestWidth = nVidWidth;
	pScoreInfo->nBestHeight = nVidHeight;

	if (!bDrvOkay && (pScoreInfo->nBestWidth < 640 || pScoreInfo->nBestHeight < 480)) {
		return 1;
	}

	return 0;
}

// ----------------------------------------------------------------------------

static inline void dx9ReleaseTexture()
{
	RELEASE(pTexture);
	RELEASE(emuTexture[0]);
	RELEASE(emuTexture[1]);
}

static inline int dx9Exit()
{
	VidFilterExit();

	dx9ReleaseTexture();

	VidSFreeVidImage();

	RELEASE(pFont);
	RELEASE(pTinyFont);
	RELEASE(pD3DDevice);
	RELEASE(pD3D);

	nRotateGame = 0;

	return 0;
}

static inline int dx9Resize(int width, int height)
{
	if (FAILED(pD3DDevice->CreateTexture(width, height, 1, 0, textureFormat, D3DPOOL_SYSTEMMEM, &pTexture, NULL))) {
#ifdef PRINT_DEBUG_INFO
		dprintf(_T("  * Error: Couldn't create texture.\n"));
#endif
		return 1;
	}
#ifdef PRINT_DEBUG_INFO
	dprintf(_T("  * Allocated a %i x %i (%s) image texture.\n"), width, height, TextureFormatString(textureFormat));
#endif

	if (!emuTexture[0]) {
		if (FAILED(pD3DDevice->CreateTexture(width, height, 1, 0, textureFormat, D3DPOOL_DEFAULT, &emuTexture[0], NULL))) {
#ifdef PRINT_DEBUG_INFO
			dprintf(_T("  * Error: Couldn't create emu texture 0.\n"));
#endif
			return 1;
		}
	}

	if (!emuTexture[1] && vidMotionBlur) {
		if (FAILED(pD3DDevice->CreateTexture(width, height, 1, 0, textureFormat, D3DPOOL_DEFAULT, &emuTexture[1], NULL))) {
#ifdef PRINT_DEBUG_INFO
			dprintf(_T("  * Error: Couldn't create emu texture 1.\n"));
#endif
			return 1;
		}
		mbTextureEmpty = true;
	}

	return 0;
}

static inline int dx9TextureInit()
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

	// Determine if we should use a texture format different from the screen format
	if ((bDrvOkay && VidFilterCheckDepth(nPreScaleEffect, 32) != 32)
		|| (bDrvOkay && bVidForce16bit)) {
		nVidImageDepth = 16;
	}

	switch (nVidImageDepth) {
		case 32:
			textureFormat = D3DFMT_X8R8G8B8;
			break;
		case 24:
			textureFormat = D3DFMT_R8G8B8;
			break;
		case 16:
			textureFormat = D3DFMT_R5G6B5;
			break;
		case 15:
			textureFormat = D3DFMT_X1R5G5B5;
			break;
	}

	nVidImageBPP = (nVidImageDepth + 7) >> 3;
	nBurnBpp = nVidImageBPP;					// Set Burn library Bytes per pixel

	// Use our callback to get colors:
	SetBurnHighCol(nVidImageDepth);

	// Make the normal memory buffer
	if (VidSAllocVidImage()) {
		dx9Exit();
		return 1;
	}

	nTextureWidth = VidGetTextureSize(nGameImageWidth * nPreScaleZoom);
	nTextureHeight = VidGetTextureSize(nGameImageHeight * nPreScaleZoom);

	if (dx9Resize(nTextureWidth, nTextureHeight)) {
		return 1;
	}

	return 0;
}

//  Vertex format:
  //
  //  0----------1
  //  |         /|
  //  |       /  |
  //  |     /    |
  //  |   /      |
  //  | /        |
  //  2----------3
  //
  //  (x,y) screen coords, in pixels
  //  (u,v) texture coords, betweeen 0.0 (top, left) to 1.0 (bottom, right)
static inline int dx9SetVertex(
	unsigned int px, unsigned int py, unsigned int pw, unsigned int ph,
    unsigned int tw, unsigned int th,
    unsigned int x, unsigned int y, unsigned int w, unsigned int h
    )
{
	// configure triangles
	// -0.5f is necessary in order to match texture alignment to display pixels
	float diff = -0.5f;
#if 1
	if (nRotateGame & 1) {
		if (nRotateGame & 2) {
			vertex[2].x = vertex[3].x = (double)(y    ) + diff;
			vertex[0].x = vertex[1].x = (double)(y + h) + diff;
			vertex[1].y = vertex[3].y = (double)(x + w) + diff;
			vertex[0].y = vertex[2].y = (double)(x    ) + diff;
		} else {
			vertex[0].x = vertex[1].x = (double)(y    ) + diff;
			vertex[2].x = vertex[3].x = (double)(y + h) + diff;
			vertex[1].y = vertex[3].y = (double)(x    ) + diff;
			vertex[0].y = vertex[2].y = (double)(x + w) + diff;
		}
	} else {
		if (nRotateGame & 2) {
			vertex[1].x = vertex[3].x = (double)(y    ) + diff;
			vertex[0].x = vertex[2].x = (double)(y + h) + diff;
			vertex[2].y = vertex[3].y = (double)(x    ) + diff;
			vertex[0].y = vertex[1].y = (double)(x + w) + diff;
		} else {
			vertex[0].x = vertex[2].x = (double)(x    ) + diff;
			vertex[1].x = vertex[3].x = (double)(x + w) + diff;
			vertex[0].y = vertex[1].y = (double)(y    ) + diff;
			vertex[2].y = vertex[3].y = (double)(y + h) + diff;
		}
	}

	double rw = (double)w / (double)pw * (double)tw;
	double rh = (double)h / (double)ph * (double)th;
	vertex[0].u = vertex[2].u = (double)(px    ) / rw;
	vertex[1].u = vertex[3].u = (double)(px + w) / rw;
	vertex[0].v = vertex[1].v = (double)(py    ) / rh;
	vertex[2].v = vertex[3].v = (double)(py + h) / rh;
#else
	if (nRotateGame & 1) {
		if (nRotateGame & 2) {
			vertex[1].x = vertex[3].x = (float)(y + h) + diff;
			vertex[0].x = vertex[2].x = (float)(y    ) + diff;
			vertex[2].y = vertex[3].y = (float)(x + w) + diff;
			vertex[0].y = vertex[1].y = (float)(x    ) + diff;
		} else {
			vertex[0].x = vertex[2].x = (float)(y + h) + diff;
			vertex[1].x = vertex[3].x = (float)(y    ) + diff;
			vertex[2].y = vertex[3].y = (float)(x    ) + diff;
			vertex[0].y = vertex[1].y = (float)(x + w) + diff;
		}
	} else {
		if (nRotateGame & 2) {
			vertex[2].x = vertex[3].x = (float)(x    ) + diff;
			vertex[0].x = vertex[1].x = (float)(y + h) + diff;
			vertex[0].y = vertex[2].y = (float)(y    ) + diff;
			vertex[1].y = vertex[3].y = (float)(x + w) + diff;
		} else {
			vertex[0].x = vertex[1].x = (float)(x    ) + diff;
			vertex[2].x = vertex[3].x = (float)(x + w) + diff;
			vertex[0].y = vertex[2].y = (float)(y + h) + diff;
			vertex[1].y = vertex[3].y = (float)(y    ) + diff;
		}
	}

	vertex[0].u = vertex[1].u = 0.0f;
	vertex[2].u = vertex[3].u = (float)pw / (float)tw;
	vertex[0].v = vertex[2].v = (float)ph / (float)th;
	vertex[1].v = vertex[3].v = 0.0f;
#endif

	// Z-buffer and RHW are unused for 2D blit, set to normal values
	vertex[0].z = vertex[1].z = vertex[2].z = vertex[3].z = 0.0f;
	vertex[0].rhw = vertex[1].rhw = vertex[2].rhw = vertex[3].rhw = 1.0f;

	// configure semi-transparent triangles
	if (vidMotionBlur) {
		D3DCOLOR semiTrans = D3DCOLOR_ARGB(0x7F, 0xFF, 0xFF, 0xFF);
		transpVertex[0].x = vertex[0].x;
		transpVertex[0].y = vertex[0].y;
		transpVertex[0].z = vertex[0].z;
		transpVertex[0].rhw = vertex[0].rhw;
		transpVertex[0].color = semiTrans;
		transpVertex[0].u = vertex[0].u;
		transpVertex[0].v = vertex[0].v;
		transpVertex[1].x = vertex[1].x;
		transpVertex[1].y = vertex[1].y;
		transpVertex[1].z = vertex[1].z;
		transpVertex[1].rhw = vertex[1].rhw;
		transpVertex[1].color = semiTrans;
		transpVertex[1].u = vertex[1].u;
		transpVertex[1].v = vertex[1].v;
		transpVertex[2].x = vertex[2].x;
		transpVertex[2].y = vertex[2].y;
		transpVertex[2].z = vertex[2].z;
		transpVertex[2].rhw = vertex[2].rhw;
		transpVertex[2].color = semiTrans;
		transpVertex[2].u = vertex[2].u;
		transpVertex[2].v = vertex[2].v;
		transpVertex[3].x = vertex[3].x;
		transpVertex[3].y = vertex[3].y;
		transpVertex[3].z = vertex[3].z;
		transpVertex[3].rhw = vertex[3].rhw;
		transpVertex[3].color = semiTrans;
		transpVertex[3].u = vertex[3].u;
		transpVertex[3].v = vertex[3].v;
	}

	return 0;
}

// ==> osd for dx9 video output (ugly), added by regret
static inline int dx9CreateFont()
{
	if (pFont) {
		return 0;
	}

	HRESULT hr = D3DXCreateFont(pD3DDevice, d3dpp.BackBufferHeight / 20,
		0, FW_SEMIBOLD, 1, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
		DEFAULT_PITCH || FF_DONTCARE,
		_T("Arial"), &pFont);

	if (FAILED(hr)) {
		return 1;
	}

	if (pTinyFont) {
		return 0;
	}

	hr = D3DXCreateFont(pD3DDevice, d3dpp.BackBufferHeight / 22,
		0, FW_SEMIBOLD, 1, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
		DEFAULT_PITCH || FF_DONTCARE,
		_T("Arial"), &pTinyFont);

	if (FAILED(hr)) {
		return 1;
	}

	return 0;
}

static inline void dx9DrawText()
{
	if (!nOSDTimer && !nTinyTimer) {
		return;
	}

	if (nFramesEmulated > nOSDTimer) {
		VidSKillShortMsg();
		VidSKillOSDMsg();
	}
	if (nFramesEmulated > nTinyTimer) {
		VidSKillTinyMsg();
		VidSKillTinyOSDMsg();
	}

	RECT osdRect;
	if (nVidFullscreen) {
		osdRect.left = Dest.left;
		osdRect.top = Dest.top;
		osdRect.right = Dest.right - 1;
		osdRect.bottom = Dest.bottom - 1;
	} else {
		osdRect.left = 0;
		osdRect.top = 0;
		osdRect.right = Dest.right - Dest.left - 1;
		osdRect.bottom = Dest.bottom - Dest.top - 1;
	}

	if (nOSDTimer) {
		pFont->DrawText(NULL, OSDMsg, -1, &osdRect, DT_RIGHT | DT_TOP, osdColor);
	}
	if (nTinyTimer) {
		pTinyFont->DrawText(NULL, tinyMsg, -1, &osdRect, DT_RIGHT | DT_BOTTOM, osdColor);
	}
}
// <== osd for dx9 video output (ugly)

static inline int dx9Init()
{
	if (hScrnWnd == NULL) {
		return 1;
	}

#ifdef ENABLE_PROFILING
	ProfileInit();
#endif

#ifdef PRINT_DEBUG_INFO
	dprintf(_T("*** Initialising Direct3D 9 blitter.\n"));
#endif

	hVidWnd = hScrnWnd;

	// Get pointer to Direct3D
	if ((pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == NULL) {
#ifdef PRINT_DEBUG_INFO
		dprintf(_T("  * Error: Couldn't initialise Direct3D.\n"));
#endif
		dx9Exit();
		return 1;
	}

	// check selected atapter
	if (nVidAdapter >= pD3D->GetAdapterCount()) {
		nVidAdapter = D3DADAPTER_DEFAULT;
	}

	D3DDISPLAYMODE dm;
	pD3D->GetAdapterDisplayMode(nVidAdapter, &dm);

	memset(&d3dpp, 0, sizeof(d3dpp));
	if (nVidFullscreen) {
		VidSDisplayScoreInfo ScoreInfo;
		memset(&ScoreInfo, 0, sizeof(VidSDisplayScoreInfo));

		if (dx9SelectFullscreenMode(&ScoreInfo)) {
			dx9Exit();
#ifdef PRINT_DEBUG_INFO
			dprintf(_T("  * Error: Couldn't determine display mode.\n"));
#endif
			return 1;
		}

		bool sizefit = true;
		if (ScoreInfo.nBestWidth > dm.Width || ScoreInfo.nBestHeight > dm.Height) {
			sizefit = false;
		}

		d3dpp.BackBufferWidth = sizefit ? ScoreInfo.nBestWidth : dm.Width;
		d3dpp.BackBufferHeight = sizefit ? ScoreInfo.nBestHeight : dm.Height;
		d3dpp.BackBufferFormat = (nVidDepth == 16) ? D3DFMT_R5G6B5 : D3DFMT_X8R8G8B8;
		d3dpp.SwapEffect = D3DSWAPEFFECT_FLIP;
		d3dpp.BackBufferCount = bVidTripleBuffer ? 2 : 1;
		d3dpp.hDeviceWindow = hVidWnd;
		d3dpp.FullScreen_RefreshRateInHz = dm.RefreshRate;//D3DPRESENT_RATE_DEFAULT;
		d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
	} else {
		d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
		d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;//D3DSWAPEFFECT_COPY;
		d3dpp.BackBufferCount = 1;
		d3dpp.hDeviceWindow = hVidWnd;
		d3dpp.Windowed = TRUE;
		d3dpp.PresentationInterval = bVidVSync ? D3DPRESENT_INTERVAL_ONE : D3DPRESENT_INTERVAL_IMMEDIATE;
	}

	DWORD dwBehaviorFlags = D3DCREATE_FPU_PRESERVE;
	if (vidHardwareVertex) {
		dwBehaviorFlags |= D3DCREATE_HARDWARE_VERTEXPROCESSING;
	} else {
		dwBehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	}
#ifdef _DEBUG
	dwBehaviorFlags |= D3DCREATE_DISABLE_DRIVER_MANAGEMENT;
#endif

	if (FAILED(pD3D->CreateDevice(nVidAdapter, D3DDEVTYPE_HAL, hVidWnd, dwBehaviorFlags, &d3dpp, &pD3DDevice))) {
//	if (FAILED(pD3D->CreateDevice(pD3D->GetAdapterCount() - 1, D3DDEVTYPE_REF, hVidWnd, dwBehaviorFlags, &d3dpp, &pD3DDevice))) {

#ifdef PRINT_DEBUG_INFO
		dprintf(_T("  * Error: Couldn't create Direct3D device.\n"));
#endif

		if (nVidFullscreen) {
			FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_ERR_UI_FULL_PROBLEM), d3dpp.BackBufferWidth, d3dpp.BackBufferHeight, d3dpp.BackBufferFormat, d3dpp.FullScreen_RefreshRateInHz);
			if (d3dpp.BackBufferWidth != 320 && d3dpp.BackBufferHeight != 240) {
				FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_ERR_UI_FULL_CUSTRES));
			}
			FBAPopupDisplay(PUF_TYPE_ERROR);
		}

		dx9Exit();
		return 1;
	}

	{
		nVidScrnWidth = dm.Width; nVidScrnHeight = dm.Height;
		nVidScrnDepth = (dm.Format == D3DFMT_R5G6B5) ? 16 : 32;
	}

	VidInitInfo();

#if 0
	// detect device capabilities
	D3DCAPS9 d3dcaps;
	pD3DDevice->GetDeviceCaps(&d3dcaps);

	caps.dynamic = bool(d3dcaps.Caps2 & D3DCAPS2_DYNAMICTEXTURES);
	caps.stretchrect = (d3dcaps.DevCaps2 & D3DDEVCAPS2_CAN_STRETCHRECT_FROM_TEXTURES) &&
		(d3dcaps.StretchRectFilterCaps & D3DPTFILTERCAPS_MINFPOINT)  &&
		(d3dcaps.StretchRectFilterCaps & D3DPTFILTERCAPS_MAGFPOINT)  &&
		(d3dcaps.StretchRectFilterCaps & D3DPTFILTERCAPS_MINFLINEAR) &&
		(d3dcaps.StretchRectFilterCaps & D3DPTFILTERCAPS_MAGFLINEAR);

	if (caps.dynamic) {
		flags.t_usage = D3DUSAGE_DYNAMIC;
		flags.v_usage = D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC;
		flags.t_pool  = D3DPOOL_DEFAULT;
		flags.v_pool  = D3DPOOL_DEFAULT;
		flags.lock    = D3DLOCK_NOSYSLOCK | D3DLOCK_DISCARD;
	} else {
		flags.t_usage = 0;
		flags.v_usage = D3DUSAGE_WRITEONLY;
		flags.t_pool  = D3DPOOL_MANAGED;
		flags.v_pool  = D3DPOOL_MANAGED;
		flags.lock    = D3DLOCK_NOSYSLOCK | D3DLOCK_DISCARD;
	}

	pD3DDevice->SetDialogBoxMode(false);

	pD3DDevice->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1);
	pD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	pD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

	pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1);
	pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

	pD3DDevice->SetRenderState(D3DRS_LIGHTING, false);
	pD3DDevice->SetRenderState(D3DRS_ZENABLE,  false);
	pD3DDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	pD3DDevice->SetRenderState(D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA);
	pD3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	pD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, false);

	pD3DDevice->SetVertexShader(NULL);
	pD3DDevice->SetFVF(D3DFVF_LVERTEX2);
#endif

	// enable vertex alpha blending
	if (vidMotionBlur) {
		pD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	} else {
		mbCurrentTexture = 0;
		pD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	}
	pD3DDevice->SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1);
	pD3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	pD3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	// apply vertex alpha values to texture
	pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);

	// for filter
	VidFilterParamInit();

	// Initialize the buffer surfaces
	if (dx9TextureInit()) {
		dx9Exit();
		return 1;
	}

	if (nPreScaleEffect) {
		if (VidFilterInit(nPreScaleEffect, 0)) {
#ifdef PRINT_DEBUG_INFO
			dprintf(_T("  * Error: Couldn't initialise pixel filter.\n"));
#endif
			dx9Exit();
			return 1;
		}
	}

	pD3DDevice->SetSamplerState(0, D3DSAMP_MINFILTER, vidFilterLinear ? D3DTEXF_LINEAR : D3DTEXF_POINT);
	pD3DDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, vidFilterLinear ? D3DTEXF_LINEAR : D3DTEXF_POINT);

	nImageWidth = 0; nImageHeight = 0;

	// Clear the swapchain's buffers
	if (nVidFullscreen) {
		for (int i = 0; i < 3; i++) {
			pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
			pD3DDevice->Present(NULL, NULL, NULL, NULL);
		}
	} else {
		RECT rect;
		getClientScreenRect(hVidWnd, &rect);

		pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
		pD3DDevice->Present(&rect, &rect, NULL, NULL);
	}

	// Create osd font
	dx9CreateFont();

#ifdef PRINT_DEBUG_INFO
	{
		dprintf(_T("  * Initialisation complete: %.2lfMB texture memory free (total).\n"), (double)pD3DDevice->GetAvailableTextureMem() / (1024 * 1024));
		dprintf(_T("    Displaying and rendering in %i-bit mode, emulation running in %i-bit mode.\n"), nVidScrnDepth, nVidImageDepth);
		if (nVidFullscreen) {
			dprintf(_T("    Running in fullscreen mode (%i x %i), "), nVidScrnWidth, nVidScrnHeight);
			dprintf(_T("using a %s buffer.\n"), bVidTripleBuffer ? _T("triple") : _T("double"));
		} else {
//			dprintf(_T("    Running in windowed mode, using D3DSWAPEFFECT_COPY to present the image.\n"));
		}
	}
#endif

	return 0;
}

static inline int dx9Reset()
{
#ifdef PRINT_DEBUG_INFO
	dprintf(_T("*** Resestting Direct3D device.\n"));
#endif

	if (pFont) {
		pFont->OnLostDevice();
	}
	if (pTinyFont) {
		pTinyFont->OnLostDevice();
	}

	dx9ReleaseTexture();

	if (FAILED(pD3DDevice->Reset(&d3dpp))) {
		return 1;
	}

	if (pFont) {
		pFont->OnResetDevice();
	}
	if (pTinyFont) {
		pTinyFont->OnResetDevice();
	}

	if (vidMotionBlur) {
		pD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	} else {
		mbCurrentTexture = 0;
		pD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	}
	pD3DDevice->SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1);
	pD3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	pD3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	// apply vertex alpha values to texture
	pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);

	dx9TextureInit();

	nImageWidth = 0; nImageHeight = 0;

	return 0;
}

static inline int dx9Scale(RECT* rect, int width, int height)
{
	if (vidUseFilter && vidForceFilterSize) {
		return VidFilterScale(rect, width, height);
	}
	return VidSScaleImage(rect, width, height);
}

// Copy BlitFXsMem to pddsBlitFX
static inline int dx9Render()
{
	getClientScreenRect(hVidWnd, &Dest);

	dx9Scale(&Dest, nGameWidth, nGameHeight);

	{
		int nNewImageWidth  = nRotateGame ? (Dest.bottom - Dest.top) : (Dest.right - Dest.left);
		int nNewImageHeight = nRotateGame ? (Dest.right - Dest.left) : (Dest.bottom - Dest.top);

		if (nImageWidth != nNewImageWidth || nImageHeight != nNewImageHeight) {
			nImageWidth  = nNewImageWidth;
			nImageHeight = nNewImageHeight;

			int nWidth = nGameImageWidth;
			int nHeight = nGameImageHeight;

			if (nPreScaleEffect) {
				if (nPreScale & 1) {
					nWidth *= nPreScaleZoom;
				}
				if (nPreScale & 2) {
					nHeight *= nPreScaleZoom;
				}
			}

			if (nVidFullscreen) {
				dx9SetVertex(0, 0, nWidth, nHeight, nTextureWidth, nTextureHeight,
					nRotateGame ? Dest.top : Dest.left, nRotateGame ? Dest.left : Dest.top, nImageWidth, nImageHeight);
			} else {
				dx9SetVertex(0, 0, nWidth, nHeight, nTextureWidth, nTextureHeight, 0, 0, nImageWidth, nImageHeight);
			}

			D3DVIEWPORT9 vp;

			// Set the size of the image on the PC screen
			if (nVidFullscreen) {
				vp.X = Dest.left;
				vp.Y = Dest.top;
				vp.Width = Dest.right - Dest.left;
				vp.Height = Dest.bottom - Dest.top;
				vp.MinZ = 0.0f;
				vp.MaxZ = 1.0f;
			} else {
				vp.X = 0;
				vp.Y = 0;
				vp.Width = Dest.right - Dest.left;
				vp.Height = Dest.bottom - Dest.top;
				vp.MinZ = 0.0f;
				vp.MaxZ = 1.0f;
			}

			pD3DDevice->SetViewport(&vp);
		}
	}

	pD3DDevice->BeginScene();

	{
		// Copy the game image onto a texture for rendering
		D3DLOCKED_RECT d3dlr;
		pTexture->LockRect(0, &d3dlr, 0, 0);

		int pitch = d3dlr.Pitch;
		unsigned char* pd = (unsigned char*)d3dlr.pBits;

		if (nPreScaleEffect) {
			VidFilterApplyEffect(pd, pitch);
		} else {
			unsigned char* ps = pVidImage + nVidImageLeft * nVidImageBPP;
			int s = nVidImageWidth * nVidImageBPP;

			switch (nVidImageDepth) {
				case 32:
					VidSCpyImg32(pd, pitch, ps, s, nVidImageWidth, nVidImageHeight);
					break;
				case 16:
					VidSCpyImg16(pd, pitch, ps, s, nVidImageWidth, nVidImageHeight);
					break;
			}
		}

		pTexture->UnlockRect(0);
	}

	pD3DDevice->UpdateTexture(pTexture, emuTexture[mbCurrentTexture]);

	if (vidMotionBlur) {
		// Motion Blur enabled
		if (!mbTextureEmpty) {
			// draw previous frame to the screen
			pD3DDevice->SetTexture( 0, emuTexture[mbCurrentTexture ^ 1]);
			pD3DDevice->SetFVF(D3DFVF_LVERTEX2);
			pD3DDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertex, sizeof(d3dvertex));
			// draw the current frame with transparency to the screen
			pD3DDevice->SetTexture(0, emuTexture[mbCurrentTexture]);
			pD3DDevice->SetFVF(D3DFVF_LVERTEX2 | D3DFVF_DIFFUSE);
			pD3DDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, transpVertex, sizeof(transp_vertex));
		} else {
			mbTextureEmpty = false;
			// draw the current frame to the screen
			pD3DDevice->SetTexture(0, emuTexture[ mbCurrentTexture]);
			pD3DDevice->SetFVF(D3DFVF_LVERTEX2);
			pD3DDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertex, sizeof(d3dvertex));
		}
		mbCurrentTexture ^= 1; // switch current texture
	} else {
		// draw the current frame to the screen
		pD3DDevice->SetTexture( 0, emuTexture[mbCurrentTexture] );
		pD3DDevice->SetFVF(D3DFVF_LVERTEX2);
		pD3DDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertex, sizeof(d3dvertex));
	}

	// draw osd text
	dx9DrawText();

	pD3DDevice->EndScene();

	return 0;
}

// Run one frame and render the screen
static inline int dx9Frame(bool bRedraw)		// bRedraw = 0
{
	if (pVidImage == NULL) {
		return 1;
	}

	HRESULT nCoopLevel = pD3DDevice->TestCooperativeLevel();
	if (nCoopLevel != D3D_OK) {			// We've lost control of the screen
		if (nCoopLevel != D3DERR_DEVICENOTRESET) {
			return 1;
		}

		if (dx9Reset()) {
			return 1;
		}
	}

#ifdef ENABLE_PROFILING
//	ProfileProfileStart(0);
#endif

	if (bDrvOkay) {
		if (bRedraw) {					// Redraw current frame
			if (BurnDrvRedraw()) {
				BurnDrvFrame();			// No redraw function provided, advance one frame
			}
		} else {
			BurnDrvFrame();				// Run one frame and draw the screen
		}
	}

#ifdef ENABLE_PROFILING
//	ProfileProfileEnd(0);
	ProfileProfileStart(1);
#endif

	dx9Render();

#ifdef ENABLE_PROFILING
	ProfileProfileEnd(1);

	dprintf(_T("    blit %3.2lf (effect p1 %3.2lf - effect p2 %3.2lf)\n"),
		ProfileProfileReadAverage(1), ProfileProfileReadAverage(2), ProfileProfileReadAverage(0));
#endif

	return 0;
}

// Paint the BlitFX surface onto the primary surface
static inline int dx9Paint(int bValidate)
{
	if (pD3DDevice->TestCooperativeLevel()) {		// We've lost control of the screen
		return 1;
	}

	RECT rect = { 0, 0, 0, 0 };

	if (!nVidFullscreen) {
		getClientScreenRect(hVidWnd, &rect);

		dx9Scale(&rect, nGameWidth, nGameHeight);

		if ((rect.right - rect.left) != (Dest.right - Dest.left)
			|| (rect.bottom - rect.top ) != (Dest.bottom - Dest.top)) {
			bValidate |= 2;
		}
	}

	if (bValidate & 2) {
		dx9Render();
	}

	if (nVidFullscreen) {
#if 0
		if (bVidVSync) {
			while(true) {
				D3DRASTER_STATUS status;
				pD3DDevice->GetRasterStatus(0, &status);
				if (status.InVBlank) {
					break;
				}
			}
	    }
#endif
		pD3DDevice->Present(NULL, NULL, NULL, NULL);
	} else {
#if 0
		if (bVidVSync) {
			D3DRASTER_STATUS rs;
			RECT window;
			GetWindowRect(hVidWnd, &window);

			while (true) {
				pD3DDevice->GetRasterStatus(0, &rs);
				if (rs.InVBlank || rs.ScanLine >= window.bottom) {
					break;
				}
//				Sleep(1);
			}
		}
#endif

		RECT src = { 0, 0, Dest.right - Dest.left, Dest.bottom - Dest.top };
		POINT c = { 0, 0 };
		ClientToScreen(hVidWnd, &c);
		RECT dst = { rect.left - c.x, rect.top - c.y, rect.right - c.x, rect.bottom - c.y };

		pD3DDevice->Present(&src, &dst, NULL, NULL);

		// Validate the rectangle we just drew
		if (bValidate & 1) {
			ValidateRect(hVidWnd, &dst);
		}
	}

	return 0;
}

// ----------------------------------------------------------------------------

static inline int dx9GetSettings(InterfaceInfo* pInfo)
{
	TCHAR szString[MAX_PATH] = _T("");

	if (nVidFullscreen) {
		if (bVidTripleBuffer) {
			IntInfoAddStringModule(pInfo, _T("Using a triple buffer"));
		} else {
			IntInfoAddStringModule(pInfo, _T("Using a double buffer"));
		}
	} else {
//		IntInfoAddStringModule(pInfo, _T("Using D3DSWAPEFFECT_COPY to present the image"));
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
	if (vidMotionBlur) {
		IntInfoAddStringModule(pInfo, _T("Applying motion blur effect"));
	}

#if 0
	// get device info, added by regret
	unsigned int adapterCount = pD3D->GetAdapterCount();
	if (adapterCount <= 0) {
		return 1;
	}
	if (nVidAdapter >= adapterCount) {
		nVidAdapter = 0;
	}

	pInfo->deviceNum = adapterCount;
	pInfo->deviceName = (TCHAR**)malloc((pInfo->deviceNum) * sizeof(TCHAR*));
	if (!pInfo->deviceName) {
		return 1;
	}
	memset(pInfo->deviceName, 0, pInfo->deviceNum * sizeof(TCHAR*));

	D3DADAPTER_IDENTIFIER9 identifier;

	for (int i = 0; i < adapterCount; i++) {
		if (!FAILED(pD3D->GetAdapterIdentifier(i, 0, &identifier))) {
			pInfo->deviceName[i] = (TCHAR*)malloc(256 * sizeof(TCHAR));
			if (!pInfo->deviceName[i]) {
				return 1;
			}
			_tcscpy(pInfo->deviceName[i], AtoW(identifier.DeviceName));
		}
	}
#endif

	return 0;
}

// The Video Output plugin:
struct VidOut VidOutD3D = { dx9Init, dx9Exit, dx9Frame, dx9Paint, dx9Scale, dx9GetSettings };
