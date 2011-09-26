// DirectDraw blitter

#include "burner.h"
#include "vid_directx_support.h"
#include "vid_filter.h"

#ifdef _MSC_VER
#pragma comment(lib, "ddraw")
#endif

static IDirectDraw7* DtoDD = NULL;				// DirectDraw interface
static IDirectDrawSurface7* DtoPrim = NULL;		// Primary surface
static IDirectDrawSurface7* DtoBack = NULL;		// Back buffer surface
static IDirectDrawSurface7* pddsDtos = NULL;	// The screen surface

static RECT Src = { 0, 0, 0, 0 };
static RECT Dest = { 0, 0, 0, 0 };

static inline int DtoPrimClear()
{
	if (DtoPrim == NULL) {
		return 1;
	}

	VidSClearSurface(DtoPrim, 0, NULL);			// Clear 1st page

	if (DtoBack) {								// We're using a triple buffer
		VidSClearSurface(DtoBack, 0, NULL);		// Clear 2nd page
		DtoPrim->Flip(NULL, DDFLIP_WAIT);
		VidSClearSurface(DtoBack, 0, NULL);		// Clear 3rd page
	}

	return 0;
}

static inline int DtoPrimInit(int bTriple)
{
	DDSURFACEDESC2 ddsd;

	// Create the primary surface
	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	if (bTriple) {								// Make a primary surface capable of triple bufferring
		ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX | DDSCAPS_VIDEOMEMORY;
		ddsd.dwBackBufferCount = 2;
	} else {
		ddsd.dwFlags = DDSD_CAPS;
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
	}

	if (FAILED(DtoDD->CreateSurface(&ddsd, &DtoPrim, NULL))) {
		return 1;
	}

	if (bTriple) {
		// Get the back buffer
		memset(&ddsd.ddsCaps, 0, sizeof(ddsd.ddsCaps));
		ddsd.ddsCaps.dwCaps = DDSCAPS_BACKBUFFER;

		if (FAILED(DtoPrim->GetAttachedSurface(&ddsd.ddsCaps, &DtoBack))) {
			// Failed to make triple buffer
			RELEASE(DtoPrim)
			return 1;
		}

		DtoPrimClear();							// Clear surfaces
	}

	return 0;
}

// Create a secondary DirectDraw surface for the game image
static inline int DtosMakeSurf()
{
	if (DtoDD == NULL) {
		return 1;
	}

	// allocate buffer in Video memory
	DDSURFACEDESC2 ddsd;
	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);

	ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
	ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_VIDEOMEMORY;//DDSCAPS_SYSTEMMEMORY;
	ddsd.dwWidth = VidGetTextureSize(nGameWidth * nPreScaleZoom);
	ddsd.dwHeight = VidGetTextureSize(nGameHeight * nPreScaleZoom);

	if (FAILED(DtoDD->CreateSurface(&ddsd, &pddsDtos, NULL))) {
		return 1;
	}

	nVidScrnDepth = VidSGetSurfaceDepth(pddsDtos);		// Get colourdepth of primary surface

	VidSClearSurface(pddsDtos, 0, NULL);

	return 0;
}

static inline int vidScale(RECT* rect, int width, int height)
{
	if (vidUseFilter && vidForceFilterSize) {
		return VidFilterScale(rect, width, height);
	}
	return VidSScaleImage(rect, width, height);
}

static inline int DtosExit()
{
	RELEASE(pddsDtos);

	VidSFreeVidImage();

	VidSExitOSD();

	return 0;
}

static inline int DtosInit()
{
	if (DtoDD == NULL) {
		return 1;
	}

	if (nRotateGame & 1) {
		nVidImageWidth = nGameHeight;
		nVidImageHeight = nGameWidth;
	} else {
		nVidImageWidth = nGameWidth;
		nVidImageHeight = nGameHeight;
	}

	nVidImageDepth = VidSGetSurfaceDepth(DtoPrim);		// Get color depth of primary surface
	nVidImageBPP = (nVidImageDepth + 7) >> 3;

	// Make the normal memory buffer
	if (VidSAllocVidImage()) {
		DtosExit();
		return 1;
	}

	// Make the DirectDraw secondary surface
	if (DtosMakeSurf()) {
		DtosExit();
		return 1;
	}

	// Use our callback to get colors:
	SetBurnHighCol(nVidImageDepth);

	Dest.left = 0; Dest.right = -1;

	RECT rect = { 0, 0, 0, 0 };
	getClientScreenRect(hVidWnd, &rect);

	vidScale(&rect, nGameWidth, nGameHeight);

	int nFlags = 2;
	if (nVidScrnWidth < 1280) {
		nFlags++;
		if (nVidScrnWidth < 1024) {
			nFlags++;
			if (nVidScrnWidth < 640) {
				nFlags++;
			}
		}
	}
	VidSInitOSD(nFlags);

	return 0;
}

static inline int vidExit()
{
	VidSRestoreScreenMode();

	VidFilterExit();

	DtosExit();

	RELEASE(DtoPrim)				// a single call releases all surfaces
	DtoBack = NULL;

	VidSExit();

	RELEASE(DtoDD)

	return 0;
}

static GUID MyGuid;
static int nCurrentDriver;
static int nAdapterCount;

static BOOL PASCAL EnumAdapterCount(GUID FAR* pGuid, LPSTR pszDesc, LPSTR pszName, LPVOID pContext, HMONITOR hMonitor)
{
	if (pGuid) {
		nAdapterCount++;
	}
	return DDENUMRET_OK;
}

static BOOL PASCAL EnumAdapters(GUID FAR* pGuid, LPSTR pszDesc, LPSTR pszName, LPVOID pContext, HMONITOR hMonitor)
{
	InterfaceInfo* pInfo = (InterfaceInfo*)pContext;
	if (!pInfo) {
		return FALSE;
	}

	if (nCurrentDriver && pGuid) {
		MONITORINFOEX mi;
		mi.cbSize = sizeof(mi);

		GetMonitorInfo(hMonitor, (MONITORINFO*)&mi);

		pInfo->deviceName[nCurrentDriver - 1] = (TCHAR*)malloc(256 * sizeof(TCHAR));
		if (!pInfo->deviceName[nCurrentDriver - 1]) {
			return TRUE;
		}
		_tcscpy(pInfo->deviceName[nCurrentDriver - 1], mi.szDevice);
	}

	nCurrentDriver++;

	return DDENUMRET_OK;
}

static BOOL PASCAL MyEnumDisplayDrivers(GUID FAR* pGuid, LPSTR pszDesc, LPSTR pszName, LPVOID pContext, HMONITOR hMonitor)
{
	if (nCurrentDriver == nVidAdapter + 1 && pGuid) {
		memcpy(&MyGuid, pGuid, sizeof(GUID));
	}

	if (nCurrentDriver == 0) {
		dprintf(_T("    %s\n"), pszDesc);
	} else {
		MONITORINFOEX mi;
		mi.cbSize = sizeof(mi);

		GetMonitorInfo(hMonitor, (MONITORINFO*)&mi);

		dprintf(_T("    Display %d (%s on %s"), nCurrentDriver, mi.szDevice, pszDesc);
		if (mi.dwFlags & MONITORINFOF_PRIMARY) {
			dprintf(_T(", primary"));
		}
		dprintf(_T(")\n"));
	}

	nCurrentDriver++;

	return DDENUMRET_OK;
}

static inline int vidInit()
{
	hVidWnd = hScrnWnd;

	memset(&MyGuid, 0, sizeof(GUID));

	if (nVidFullscreen) {
		nCurrentDriver = 0;
		dprintf(_T(" ** Enumerating available DirectDraw drivers:\n"));
		DirectDrawEnumerateExA(MyEnumDisplayDrivers, NULL, DDENUM_ATTACHEDSECONDARYDEVICES | DDENUM_DETACHEDSECONDARYDEVICES | DDENUM_NONDISPLAYDEVICES);
	}

	// Get pointer to DirectDraw device
	DirectDrawCreateEx(nVidAdapter ? &MyGuid : NULL, (void**)&DtoDD, IID_IDirectDraw7, NULL);

	VidSInit(DtoDD);

	VidInitInfo();

	DtoPrim = NULL;					// No primary surface yet
	DtoBack = NULL;

	// Remember the changes to the display
	if (nVidFullscreen) {
		if (VidSEnterFullscreenMode(0, 0)) {
			vidExit();
			return 1;
		}
	} else {
		DtoDD->SetCooperativeLevel(hVidWnd, DDSCL_NORMAL);
	}

#ifdef PRINT_DEBUG_INFO
	{
		// Display amount of free video memory
		DDSCAPS2 ddsCaps2;
		DWORD dwTotal;
		DWORD dwFree;

		memset(&ddsCaps2, 0, sizeof(ddsCaps2));
		ddsCaps2.dwCaps = DDSCAPS_PRIMARYSURFACE;

		dprintf(_T(" ** Starting DirectDraw7 blitter.\n"));

		if (SUCCEEDED(DtoDD->GetAvailableVidMem(&ddsCaps2, &dwTotal, &dwFree))) {
			dprintf(_T("  * Initialising video: Total video memory minus display surface: %.2fMB.\n"), (double)dwTotal / (1024 * 1024));
		}

		if (bDrvOkay) {
			if (BurnDrvGetFlags() & BDF_ORIENTATION_FLIPPED) {
				if (nRotateGame & 2) {
					dprintf(_T("  * Using graphics hardware to rotate the image 180 degrees.\n"));
				} else {
					dprintf(_T("  * Warning: Graphics hardware does not support mirroring blits.\n    Image orientation will be incorrect.\n"));
				}
			}
		}
	}
#endif

	if (bVidTripleBuffer && nVidFullscreen) {
		if (DtoPrimInit(1)) {			// Try to make triple buffer
#ifdef PRINT_DEBUG_INFO
			dprintf(_T("  * Warning: Couldn't allocate a triple-buffering surface.\n"));
#endif
			// If we fail, fail entirely and make a normal buffer
			RELEASE(DtoPrim)
		}
	}

	if (DtoPrim == NULL) {
		// No primary surface yet, so try normal
		if (DtoPrimInit(0)) {
#ifdef PRINT_DEBUG_INFO
	   		dprintf(_T("  * Error: Couldn't create primary surface.\n"));
#endif
			vidExit();
			return 1;
		}
	}

	if (nVidFullscreen) {
		DtoDD->Compact();
	}

	// for filter
	VidFilterParamInit();

	VidSClipperInit(DtoPrim);

	// Init the buffer surfaces
	if (DtosInit()) {
		vidExit();
		return 1;
	}

	if (nPreScaleEffect) {
		if (VidFilterInit(nPreScaleEffect, nRotateGame)) {
			if (VidFilterInit(0, nRotateGame)) {
#ifdef PRINT_DEBUG_INFO
				dprintf(_T("  * Error: Couldn't initialise pixel filter.\n"));
#endif
				vidExit();
				return 1;
			}
		}
	}

#ifdef PRINT_DEBUG_INFO
	{
		DDSCAPS2 ddsCaps2;
		DWORD dwTotal;
		DWORD dwFree;

		memset(&ddsCaps2, 0, sizeof(ddsCaps2));
		ddsCaps2.dwCaps = DDSCAPS_PRIMARYSURFACE;

		if (SUCCEEDED(DtoDD->GetAvailableVidMem(&ddsCaps2, &dwTotal, &dwFree))) {
			dprintf(_T("  * Initialisation complete: %.2fMB video memory free.\n"), (double)dwFree / (1024 * 1024));
			dprintf(_T("    Displaying and rendering in %i-bit mode, emulation running in %i-bit mode.\n"), nVidScrnDepth, nVidImageDepth);
			dprintf(_T("    Transferring the image to video memory before blitting.\n"));
			if (nVidFullscreen) {
				dprintf(_T("    Running in fullscreen mode (%i x %i)"), nVidScrnWidth, nVidScrnHeight);
				if (DtoBack != NULL) {
					dprintf(_T(", using a triple buffer"));
				}
				dprintf(_T(".\n"));
			} else {
				dprintf(_T("    Running in windowed mode.\n"));
			}
		}
	}
#endif

	return 0;
}

// Copy pVidImage to pddsDtos, rotate, fixed by regret
static inline void vidRenderRotate(unsigned char* pd, int pitch)
{
	unsigned char* ps = NULL;
	unsigned char* pdd = NULL;

	switch (nVidImageBPP) {
		case 4: {		// 32bit
			switch (nRotateGame) {
				case 1: {
					for (int y = 0; y < nGameHeight; y++, pd += pitch) {
						ps = pVidImage + (nGameHeight - 1 - y) * nVidImageBPP;
						pdd = pd;
						for (int x = 0; x < nGameWidth; x++) {
							*(int*)pdd = *(int*)ps;
							ps += nVidImagePitch;
							pdd += nVidImageBPP;
						}
					}
					break;
				}
				case 2: {
					pd += nGameHeight * pitch - nVidImageBPP;
					for (int y = 0; y < nGameHeight; y++) {
						ps = pVidImage + y * nVidImagePitch;
						for (int x = 0; x < nGameWidth; x++) {
							*(int*)pd = *(int*)ps;
							ps += nVidImageBPP;
							pd -= nVidImageBPP;
						}
					}
					break;
				}
				case 3: {
					for (int y = 0; y < nGameHeight; y++, pd += pitch) {
						ps = pVidImage + y * nVidImageBPP;
						pdd = pd + (nGameWidth - 1) * nVidImageBPP;
						for (int x = 0; x < nGameWidth; x++) {
							*(int*)pdd = *(int*)ps;
							ps += nVidImagePitch;
							pdd -= nVidImageBPP;
						}
					}
					break;
				}
			}
			break;
		}

		case 3: {		// 24bit
			switch (nRotateGame) {
				case 1: {
					for (int y = 0; y < nGameHeight; y++, pd += pitch) {
						ps = pVidImage + (nGameHeight - 1 - y) * nVidImagePitch;
						pdd = pd;
						for (int x = 0; x < nGameWidth; x++) {
							pdd[0] = ps[0];
							pdd[1] = ps[1];
							pdd[2] = ps[2];
							ps += nVidImagePitch;
							pdd += nVidImageBPP;
						}
					}
					break;
				}
				case 2: {
					pd += nGameHeight * pitch - nVidImageBPP;
					for (int y = 0; y < nGameHeight; y++) {
						ps = pVidImage + y * nVidImagePitch;
						for (int x = 0; x < nGameWidth; x++) {
							pd[0] = ps[0];
							pd[1] = ps[1];
							pd[2] = ps[2];
							ps += nVidImageBPP;
							pd -= nVidImageBPP;
						}
					}
					break;
				}
				case 3: {
					for (int y = 0; y < nGameHeight; y++, pd += pitch) {
						ps = pVidImage + y * nVidImageBPP;
						pdd = pd + (nGameWidth - 1) * nVidImageBPP;
						for (int x = 0; x < nGameWidth; x++) {
							pdd[0] = ps[0];
							pdd[1] = ps[1];
							pdd[2] = ps[2];
							ps += nVidImagePitch;
							pdd -= nVidImageBPP;
						}
					}
					break;
				}
			}
			break;
		}

		case 2:	{		// 16bit
			switch (nRotateGame) {
				case 1: {
					for (int y = 0; y < nGameHeight; y++, pd += pitch) {
						ps = pVidImage + (nGameHeight - 1 - y) * nVidImageBPP;
						pdd = pd;
						for (int x = 0; x < nGameWidth; x++) {
							*(short*)pdd = *(short*)ps;
							ps += nVidImagePitch;
							pdd += nVidImageBPP;
						}
					}
					break;
				}
				case 2: {
					pd += nGameHeight * pitch - nVidImageBPP;
					for (int y = 0; y < nGameHeight; y++) {
						ps = pVidImage + y * nVidImagePitch;
						for (int x = 0; x < nGameWidth; x++) {
							*(short*)pd = *(short*)ps;
							ps += nVidImageBPP;
							pd -= nVidImageBPP;
						}
					}
					break;
				}
				case 3: {
					for (int y = 0; y < nGameHeight; y++, pd += pitch) {
						ps = pVidImage + y * nVidImageBPP;
						pdd = pd + (nGameWidth - 1) * nVidImageBPP;
						for (int x = 0; x < nGameWidth; x++) {
							*(short*)pdd = *(short*)ps;
							ps += nVidImagePitch;
							pdd -= nVidImageBPP;
						}
					}
					break;
				}
			}
			break;
		}
	}
}

// Copy pVidImage to pddsDtos, don't rotate
static inline void vidRenderNoRotate(unsigned char* pd, int pitch)
{
	unsigned char* ps = pVidImage + nVidImageLeft * nVidImageBPP;
	for (int y = 0; y < nVidImageHeight; y++, pd += pitch, ps += nVidImagePitch) {
		memcpy(pd, ps, nVidImagePitch);
	}
}

static inline int vidBurnToSurf()
{
	if (pddsDtos == NULL) {
		return 1;
	}

	if (DtoPrim->IsLost()) {		// We've lost control of the screen
		return 1;
	}

	// Lock the surface so we can write to it
	DDSURFACEDESC2 ddsd;
	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);

	if (FAILED(pddsDtos->Lock(NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL))) {
		return 1;
	}

	unsigned char* pd = (unsigned char*)ddsd.lpSurface;
	int pitch = ddsd.lPitch;

	if (nPreScaleEffect) {
		VidFilterApplyEffect(pd, pitch);
	} else {
		if (nRotateGame == 0) {
			vidRenderNoRotate(pd, pitch);
		} else {
			vidRenderRotate(pd, pitch);
		}
	}

	pddsDtos->Unlock(NULL);

	return 0;
}

// Run one frame and render the screen
static inline int vidFrame(bool bRedraw)			// bRedraw = 0
{
	if (pVidImage == NULL) {
		return 1;
	}

	if (DtoPrim->IsLost()) {		// We've lost control of the screen
		if (VidSRestoreOSD()) {
			return 1;
		}

		if (FAILED(DtoDD->RestoreAllSurfaces())) {
			return 1;
		}

		DtoPrimClear();
	}

	if (bDrvOkay) {
		if (bRedraw) {				// Redraw current frame
			if (BurnDrvRedraw()) {
				BurnDrvFrame();		// No redraw function provided, advance one frame
			}
		} else {
			BurnDrvFrame();			// Run one frame and draw the screen
		}
	}

	vidBurnToSurf();

	return 0;
}

// Paint the Dtos surface onto the primary surface
static inline int vidPaint(int bValidate)
{
	if (DtoPrim == NULL || pddsDtos == NULL) {
		return 1;
	}

	if (DtoPrim->IsLost()) {		// We've lost control of the screen
		return 1;
	}

	if (!nVidFullscreen) {			// Check if the window has changed since we prepared the image
		RECT rect = { 0, 0, 0, 0 };
		getClientScreenRect(hVidWnd, &rect);

		vidScale(&rect, nGameWidth, nGameHeight);

		if (Dest.left != rect.left || Dest.right != rect.right || Dest.top != rect.top || Dest.bottom != rect.bottom) {
			bValidate |= 2;
		}
	}

	// calc
	getClientScreenRect(hVidWnd, &Dest);

	vidScale(&Dest, nGameWidth, nGameHeight);

	Src.right = nGameWidth;
	Src.bottom = nGameHeight;

	if (nPreScaleEffect) {
		if (nPreScale & 1) {
			Src.right *= nPreScaleZoom;
		}
		if (nPreScale & 2) {
			Src.bottom *= nPreScaleZoom;
		}
	}

	if (bValidate & 2) {
		vidBurnToSurf();
	}

	// vsync
	if (bVidVSync) {
		RECT window;
		GetWindowRect(hVidWnd, &window);

		BOOL in_vblank = FALSE;
		DWORD line;

		while (true) {
			DtoDD->GetScanLine(&line);
			DtoDD->GetVerticalBlankStatus(&in_vblank);
			if (in_vblank || line >= window.bottom) {
				break;
			}
//			Sleep(1);
		}
	}

	if (DtoBack != NULL) {			// Triple bufferring
		if (FAILED(DtoBack->Blt(&Dest, pddsDtos, &Src, DDBLT_ASYNC, 0))) {
			if (FAILED(DtoBack->Blt(&Dest, pddsDtos, &Src, DDBLT_WAIT, 0))) {
				return 1;
			}
		}

		VidSDisplayOSD(DtoBack, &Dest, 0);

		DtoPrim->Flip(NULL, DDFLIP_WAIT);
	} else {						// Normal
		RECT rect = { 0, 0, nGameWidth, nGameHeight };

		// Display OSD text message
		if (nPreScaleEffect) {
			if (nPreScale & 1) {
				rect.right *= nPreScaleZoom;
			}
			if (nPreScale & 2) {
				rect.bottom *= nPreScaleZoom;
			}
		}
		VidSDisplayOSD(pddsDtos, &rect, 0);

		if (FAILED(DtoPrim->Blt(&Dest, pddsDtos, &Src, DDBLT_ASYNC, 0))) {
			if (FAILED(DtoPrim->Blt(&Dest, pddsDtos, &Src, DDBLT_WAIT, 0))) {
				return 1;
			}
		}
	}

	if (bValidate & 1) {
		// Validate the rectangle we just drew
		POINT c = {0, 0};
		ClientToScreen(hVidWnd, &c);
		Dest.left -= c.x; Dest.right -= c.x;
		Dest.top -= c.y; Dest.bottom -= c.y;
		ValidateRect(hVidWnd, &Dest);
	}

	return 0;
}

static inline int vidGetSettings(InterfaceInfo* pInfo)
{
	if (nVidFullscreen && DtoBack) {
		IntInfoAddStringModule(pInfo, _T("Using a triple buffer"));
	} else {
		IntInfoAddStringModule(pInfo, _T("Using Blt() to transfer the image"));
	}

	IntInfoAddStringModule(pInfo, _T("Using video memory for the final blit"));

	if (nRotateGame) {
		TCHAR* pszEffect[8] = { _T(""), _T(""), _T(""), _T(""), _T(""), _T(""), _T(""), _T("") };
		TCHAR szString[MAX_PATH] = _T("");

		pszEffect[0] = _T("Using ");
		if (nRotateGame & 1) {
			pszEffect[1] = _T("software rotation");
		}
		if (nRotateGame & 2) {
			if (nRotateGame & 1) {
				pszEffect[2] = _T(" and");
			}
			pszEffect[3] = _T(" hardware mirroring");
		}
		pszEffect[4] = _T(", ");

		_sntprintf(szString, sizearray(szString), _T("%s%s%s%s%s%s%s%s"), pszEffect[0], pszEffect[1], pszEffect[2], pszEffect[3], pszEffect[4], pszEffect[5], pszEffect[6], pszEffect[7]);
		IntInfoAddStringModule(pInfo, szString);
	}

#if 0
	// get device info, added by regret
	nAdapterCount = 0;
	if (FAILED(DirectDrawEnumerateExA(EnumAdapterCount, NULL, DDENUM_ATTACHEDSECONDARYDEVICES | DDENUM_DETACHEDSECONDARYDEVICES | DDENUM_NONDISPLAYDEVICES))) {
		return 1;
    }
	pInfo->deviceNum = nAdapterCount;
	pInfo->deviceName = (TCHAR**)malloc((pInfo->deviceNum) * sizeof(TCHAR*));
	if (!pInfo->deviceName) {
		return 1;
	}
	memset(pInfo->deviceName, 0, pInfo->deviceNum * sizeof(TCHAR*));

	nCurrentDriver = 0;
	if (FAILED(DirectDrawEnumerateExA(EnumAdapters, (VOID*)pInfo, DDENUM_ATTACHEDSECONDARYDEVICES | DDENUM_DETACHEDSECONDARYDEVICES | DDENUM_NONDISPLAYDEVICES))) {
		return 1;
    }
#endif

	return 0;
}

// The video output plugin:
struct VidOut VidOutDDraw = { vidInit, vidExit, vidFrame, vidPaint, vidScale, vidGetSettings };
