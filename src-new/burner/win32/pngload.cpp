#include "burner.h"
#include "PNGlib.h"

char szSupportArchive[13][MAX_PATH] = { { "previews" },{ "titles" },{ "select" },{ "versus" },{ "howto" },{ "scores" },{ "bosses" },{ "gameover" },{ "flyers" },{ "marquees" },{ "controls" },{ "cabinets" },{ "pcbs" } };
static int iArchive = 0;
static int nPngOutfAuto = PNG_OUTF_AUTO;
int bImageOrientation;
static bool bKeepFileAspectRatio = true;

static int GetArchiveIndex(char* szPath) {

	int nIndex = 0;

	TCHAR szSupportPath[13][MAX_PATH];
	
	_stprintf(szSupportPath[0], szAppPreviewsPath);
	_stprintf(szSupportPath[1], szAppTitlesPath);
	_stprintf(szSupportPath[2], szAppSelectPath);
	_stprintf(szSupportPath[3], szAppVersusPath);
	_stprintf(szSupportPath[4], szAppHowtoPath);
	_stprintf(szSupportPath[5], szAppScoresPath);
	_stprintf(szSupportPath[6], szAppBossesPath);
	_stprintf(szSupportPath[7], szAppGameoverPath);
	_stprintf(szSupportPath[8], szAppFlyersPath);
	_stprintf(szSupportPath[9], szAppMarqueesPath);
	_stprintf(szSupportPath[10], szAppControlsPath);
	_stprintf(szSupportPath[11], szAppCabinetsPath);
	_stprintf(szSupportPath[12], szAppPCBsPath);

	for(int x = 0; x < 13; x++) {
		char* szCmp;
		szCmp =  _TtoA(szSupportPath[x]);
		if(!strcmp(szPath, szCmp)) {
			nIndex = x;
			break;
		}
	}

	return nIndex;
}

// Function to check if a file exist at the specified location
int CheckFile(TCHAR* szName, TCHAR* szPath, TCHAR* szExt) {

	TCHAR szFile[MAX_PATH] = _T("");
	_stprintf(szFile, _T("%s%s%s"), szPath, szName, szExt);

	FILE* pFile = _tfopen(szFile, _T("rb"));

	if(pFile) {
		fclose(pFile);
		return 0;
	}

	return 1;
}

int CheckZipFile(TCHAR* szName, TCHAR* szPath, TCHAR* szExt) {
	void* pBuffer	= NULL;
	int nSize		= 0;

	iArchive = GetArchiveIndex(_TtoA(szPath));

	char szTemp[3][MAX_PATH];
	sprintf(szTemp[0], _TtoA(szName));
	sprintf(szTemp[1], _TtoA(szAppArchivesPath)); // use archives path
	sprintf(szTemp[2], _TtoA(szExt));

	char szZip[MAX_PATH] = "";
	char szFileName[MAX_PATH] = "";
	sprintf(szZip, "%s%s.zip", szTemp[1], szSupportArchive[iArchive]);
	sprintf(szFileName, "%s%s", szTemp[0], szTemp[2]);

	if(!ZipLoadOneFile(szZip, szFileName, &pBuffer, &nSize)) {
		nSize = 0;
		free(pBuffer);
		return 0;
	}

	return 1;
}

HBITMAP ScaleAlt(HWND hWnd, HBITMAP hSclBmp, int destWidth, int destHeight, int srcWidth, int srcHeight) {

    HDC			mainDC	= CreateCompatibleDC(GetDC(hWnd));
    HDC			memDC1	= CreateCompatibleDC(mainDC);
	HDC			memDC2	= CreateCompatibleDC(mainDC);

    BITMAP bm;
    GetObject(hSclBmp, sizeof(bm), &bm);

	HBITMAP		gBmp	= CreateCompatibleBitmap(GetDC(hWnd), destWidth, destHeight);

    if (gBmp)
    {
        HBITMAP	oldBmp1 = (HBITMAP)SelectObject(memDC1, gBmp);
        HBITMAP	oldBmp2 = (HBITMAP)SelectObject(memDC2, hSclBmp);
        
		SetStretchBltMode(memDC1, HALFTONE);
		SetBrushOrgEx(memDC1, 0, 0, 0);

		StretchBlt(
			memDC1	, 0, 0, destWidth	, destHeight, // destination
			memDC2	, 0, 0, srcWidth	, srcHeight,  // source
			(DWORD)SRCCOPY
		);		
        
        SelectObject(memDC1, oldBmp1);
        SelectObject(memDC2, oldBmp2);
    }
    
    DeleteDC(memDC1);
    DeleteDC(memDC2);
    ReleaseDC(NULL, mainDC);
    
    return gBmp;
}

HBITMAP PNGtoBMP(HWND hWnd, char* szPath, char* szDrvName, int nLoadMethod, int nMaxW, int nMaxWY) {

	iArchive = GetArchiveIndex(szPath);

	if(iArchive <= 7) {
		bKeepFileAspectRatio = false;
	} else {
		bKeepFileAspectRatio = true;
	}

	HBITMAP hBitmap = NULL;
	PNGINFO pngInfo;

	// Initialize
	PNG_Init(&pngInfo);

	if(nLoadMethod == FBA_LM_FILE) {

		char szFile[MAX_PATH] = "";
		sprintf(szFile, "%s%s.png", szPath, szDrvName);

		if(!PNG_LoadFile(&pngInfo, szFile)) return NULL;					// Load file

		if (!PNG_Decode(&pngInfo)) return NULL;								// Decode

		hBitmap = PNG_CreateBitmap(&pngInfo, hWnd, nPngOutfAuto, false);	// Create bitmap

		if (hBitmap == NULL) return NULL;
	}

	if(nLoadMethod == FBA_LM_ZIP_BUFF) {

		// load zip
		char szFile[MAX_PATH] = "", szPNG[MAX_PATH] = "";
		sprintf(szFile, "%s%s.zip", _TtoA(szAppArchivesPath), szSupportArchive[iArchive]);
		sprintf(szPNG, "%s.png", szDrvName);

		void* pBuffer	= NULL;
		int nSize		= 0;
		int nRet		= ZipLoadOneFile(szFile, szPNG, &pBuffer, &nSize);

		if (!nRet) {			
			if (!PNG_LoadCustom(&pngInfo, (LPBYTE)pBuffer, nSize)) return NULL;		// Load file

			if (!PNG_Decode(&pngInfo)) return NULL;									// Decode

			hBitmap = PNG_CreateBitmap(&pngInfo, hWnd, nPngOutfAuto, false);		// Create bitmap

			if(hBitmap == NULL) return NULL;

			free(pBuffer);
		} else {
			return NULL;
		}
	}

	// Image Width and Height calculation

	int nW, nH, nARX, nARY;
	int nFW = 0, nFH = 0;
	float	nAR;

	if(bKeepFileAspectRatio == false) {
		BurnDrvGetVisibleSize(&nW, &nH);	// width, height
		BurnDrvGetAspect(&nARX, &nARY);		// horizontal / vertical aspect ratio, ex-> 'X=4, Y=3'
	} else {
		nW = pngInfo.iWidth;
		nH = pngInfo.iHeight;
	}

	if(bKeepFileAspectRatio == false) {
		// vertical
		if(nARX < nARY) {
			bImageOrientation = 1;
		}  
		// horizontal
		if(nARY <= nARX) {
			bImageOrientation = 0;
		}		
		nAR = (float)nARX / (float)nARY;

	} else {
		// vertical
		if(nW < nH) {
			bImageOrientation = 1;
		}  
		// horizontal
		if(nH <= nW) {
			bImageOrientation = 0;
		}
		nAR = (float)nW / (float)nH;
	}

//	nMaxW = bGameInfoOpen ? (bImageOrientation ? nMaxW : nMaxWY) : nMaxW;
	nMaxW = bImageOrientation ? nMaxWY : nMaxW;

	// vertical
	if(bImageOrientation == 1) {
		nFH = nMaxW;
		nFW = (int)(nFH * nAR);
	}

	// horizontal
	if(bImageOrientation == 0) {
		nFW = nMaxW;
		nFH = (int)(nFW / nAR);
	}

	HBITMAP hFinalBitmap = NULL;

	hFinalBitmap = ScaleBitmap(hBitmap, nFW, nFH);
	
	if(!hFinalBitmap) {
		hFinalBitmap = ScaleAlt(hWnd, hBitmap, nFW, nFH, nW, nH);
	}

	// Cleanup
	PNG_Cleanup(&pngInfo);

	return hFinalBitmap;
}

HBITMAP PNGtoBMP_Simple(HWND hWnd, char* szFile) {

	HBITMAP hBitmap = NULL;

	PNGINFO pngInfo;
	PNG_Init(&pngInfo);													// Initialize

	if(!PNG_LoadFile(&pngInfo, szFile)) return NULL;					// Load file

	if(!PNG_Decode(&pngInfo)) return NULL;								// Decode

	hBitmap = PNG_CreateBitmap(&pngInfo, hWnd, nPngOutfAuto, false);	// Create bitmap

	if(!hBitmap) return NULL;

	PNG_Cleanup(&pngInfo);												// Cleanup

	return hBitmap;
}

void UpdatePreview(bool bPrevReset, HWND hDlg, TCHAR* szPreviewDir)
{
	//nBurnDrvSelect = nDialogSelect;

	HBITMAP hNewImage	= NULL;
	TCHAR	szFDriver[MAX_PATH] = _T(""), szDrvName[MAX_PATH] = _T(""), szDrvParent[MAX_PATH] = _T("");
	bool	bLoadPNG	= false;
	bool	bLoadZipPNG = false;
	bool	bParent		= false;

	// Check if we can get the values and proceed (FBA will crash if used incorrecly)
	if(BurnDrvGetText(DRV_NAME)) _stprintf(szDrvName, BurnDrvGetText(DRV_NAME));
	if(BurnDrvGetText(DRV_PARENT)) bParent = true;
	if(bParent) _stprintf(szDrvParent, BurnDrvGetText(DRV_PARENT));

	static int nIndex;
	int nOldIndex;	
	
	nOldIndex = nIndex;
	nIndex++;

	if(bPrevReset) {
		nIndex = 1;
		nOldIndex = -1;
		if (hPrevBmp) {
			DeleteObject((HGDIOBJ)hPrevBmp);
			hPrevBmp = NULL;
		}
		if (nTimer) {
			KillTimer(hDlg, nTimer);
			nTimer = 0;
		}
	}

	do {
		// -----------------------------------------------------------------------------------
		// Try to load a .PNG preview image from directory

		if (nIndex == 1) {
			if(!CheckFile(szDrvName,	szPreviewDir, _T(".png")))	{
				_stprintf(szFDriver, szDrvName);
				bLoadPNG = true;
				break;
			}
		}
		_stprintf(szDrvName, _T("%s-p%02i"), szDrvName, nIndex);
		if(!CheckFile(szDrvName,	szPreviewDir, _T(".png")))	{
			_stprintf(szFDriver, szDrvName);
			bLoadPNG = true;
			break;
		}
		// Try the parent
		if (bParent) {
			if (nIndex == 1) {
				if(!CheckFile(szDrvParent,		szPreviewDir, _T(".png")))	{
					_stprintf(szFDriver, szDrvParent);
					bLoadPNG = true;
					break;
				}
			}
			_stprintf(szDrvParent, _T("%s-p%02i"), szDrvParent, nIndex);
			if(!CheckFile(szDrvParent,		szPreviewDir, _T(".png")))	{
				_stprintf(szFDriver, szDrvParent);
				bLoadPNG = true;
				break;
			}
		}
		// -----------------------------------------------------------------------------------

		// -----------------------------------------------------------------------------------
		// Try to load a .PNG preview image from ZIP

		// Reset these values first...
		if(BurnDrvGetText(DRV_NAME)) _stprintf(szDrvName, BurnDrvGetText(DRV_NAME));
		if(BurnDrvGetText(DRV_PARENT)) bParent = true;
		if(bParent) _stprintf(szDrvParent, BurnDrvGetText(DRV_PARENT));

		if (nIndex == 1) {
			if(!CheckZipFile(szDrvName,	szPreviewDir, _T(".png")))	{
				_stprintf(szFDriver, szDrvName);
				bLoadZipPNG = true;
				bLoadPNG = true;
				break;
			}
		}
		_stprintf(szDrvName, _T("%s-p%02i"), szDrvName, nIndex);
		if(!CheckZipFile(szDrvName,	szPreviewDir, _T(".png")))	{
			_stprintf(szFDriver, szDrvName);
			bLoadZipPNG = true;
			bLoadPNG = true;
			break;
		}
		// Try the parent
		if (bParent) {
			if (nIndex == 1) {
				if(!CheckZipFile(szDrvParent,	szPreviewDir, _T(".png")))	{
					_stprintf(szFDriver, szDrvParent);
					bLoadZipPNG = true;
					bLoadPNG = true;
					break;
				}
			}
			_stprintf(szDrvParent, _T("%s-p%02i"), szDrvParent, nIndex);
			if(!CheckZipFile(szDrvParent,	szPreviewDir, _T(".png")))	{
				_stprintf(szFDriver, szDrvParent);
				bLoadZipPNG = true;
				bLoadPNG = true;
				break;
			}
		}
		// -----------------------------------------------------------------------------------

		if (nIndex == 1) break;
		if (!bLoadPNG) nIndex = 1;

	} while(!bLoadPNG);

	if (bLoadPNG && nIndex != nOldIndex) {
		char szTemp[2][MAX_PATH];		
		sprintf(szTemp[0], _TtoA(szPreviewDir));
		sprintf(szTemp[1], _TtoA(szFDriver));

		// Load PNG from ZIP if we can't get a handle from the directory file
		if(bLoadZipPNG) {
			hNewImage = PNGtoBMP(hDlg, szTemp[0], szTemp[1], FBA_LM_ZIP_BUFF, 304, 230);
		} else {
			hNewImage = PNGtoBMP(hDlg, szTemp[0], szTemp[1], FBA_LM_FILE, 304, 230);
		}
	}

	if (hNewImage) {

		if(hPrevBmp) DeleteObject((HGDIOBJ)hPrevBmp);
		hPrevBmp = hNewImage;

		if (bImageOrientation == 0) {
			SendDlgItemMessage(hDlg, IDC_SCREENSHOT_H, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hPrevBmp);
			SendDlgItemMessage(hDlg, IDC_SCREENSHOT_V, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)NULL);
			ShowWindow(GetDlgItem(hDlg, IDC_SCREENSHOT_V), SW_HIDE);
		} else {
			SendDlgItemMessage(hDlg, IDC_SCREENSHOT_H, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)NULL);
			ShowWindow(GetDlgItem(hDlg, IDC_SCREENSHOT_V), SW_SHOW);
			SendDlgItemMessage(hDlg, IDC_SCREENSHOT_V, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hPrevBmp);
		}

		nTimer = SetTimer(hDlg, 1, 2500, NULL);

	} else {
		// We couldn't load a new image for this game, so kill the timer (it will be restarted when a new game is selected)
		if (nTimer) {
			KillTimer(hDlg, nTimer);
			nTimer = 0;
		}
		if (!hPrevBmp) {
			if (_tcslen(szPlaceHolder)) {
				LPTSTR p = _tcsrchr(szPlaceHolder, '.');
				if (!_tcsicmp(p+1, _T("bmp"))) {
					hPrevBmp = (HBITMAP)LoadImage(hAppInst, szPlaceHolder, IMAGE_BITMAP, 304, 228, LR_LOADFROMFILE);
				} else {
					if (!_tcsicmp(p+1, _T("png"))) {
						FILE *fp = _tfopen(szPlaceHolder, _T("rb"));
						if (fp) {
							char szTemp[MAX_PATH];
							sprintf(szTemp, _TtoA(szPlaceHolder));
							hPrevBmp = PNGtoBMP_Simple(hScrnWnd, szTemp);
							fclose(fp);
						}
					}
				}
			} else {
				hPrevBmp = (HBITMAP)LoadImage(hAppInst, MAKEINTRESOURCE(BMP_PREVIEW), IMAGE_BITMAP, 304, 228, 0);
			}

			SendDlgItemMessage(hDlg, IDC_SCREENSHOT_H, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hPrevBmp);
			SendDlgItemMessage(hDlg, IDC_SCREENSHOT_V, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)NULL);
			ShowWindow(GetDlgItem(hDlg, IDC_SCREENSHOT_V), SW_HIDE);
		}
	}
}
