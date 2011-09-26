// background image support, by regret

/* changelog:
 update 2: add load jpg image
 update 1: create (ref: mameui)
*/

#include "burner.h"
#include "imageload+.h"

#define MAX_SKINFILES 100
int nRandomSkin = 0;

bool bVidUsePlaceholder = true;				// Use a placeholder image when no game is loaded
TCHAR szPlaceHolder[MAX_PATH] = _T("");

// This is for background image, performs the needed stretching
// for avoiding the use of more images for this :)
static void StretchBitmap(HDC hDC, HWND hWnd, HBITMAP hBitmap)
{
	RECT rect;
	GetClientRect(hWnd, &rect);

	HDC memDC = CreateCompatibleDC(hDC);
	HGDIOBJ hOldObj = SelectObject(memDC, hBitmap);

	BITMAP bm;
	GetObject(hBitmap, sizeof(BITMAP), (LPSTR)&bm);

	SetStretchBltMode(hDC, HALFTONE);
	SetBrushOrgEx(hDC, 0, 0, 0);
	StretchBlt(hDC, 0, 0, rect.right, rect.bottom, memDC, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);

	SelectObject(memDC, hOldObj);
	DeleteDC(memDC);
	if (hBitmap) {
		DeleteObject(hBitmap);
	}
}

// from FBA Plus!
void paintSkin(HWND hWnd)
{
	// Win9x-ME plataforms has problems with the preview blitter option.
	// Anyway this is a "hotfix" for this problem, this seems to be
	// related to some options that only Win2000-XP builds can handle.
	// This consists only in enable the posibility of loading a bitmap
	// as a background for the aplication, something old I know, also
	// performs an stretching to the image to fit the client size. Is
	// the best I can do for this odd OSes (including mine XD)
	// Almost forgot to mention... you have to disable the "Preview
	// blitter" option to enable this only if you use win9x or ME OS.
	PAINTSTRUCT ps;
	HBITMAP hBitmap = loadSkin(hWnd);
	BeginPaint(hWnd, &ps);
	StretchBitmap(ps.hdc, hWnd, hBitmap);
	EndPaint(hWnd, &ps);
}

HBITMAP loadSkin(HWND hWnd)
{
	if (!szPlaceHolder || _tcslen(szPlaceHolder) <= 0) {
		return LoadBitmap(hAppInst, MAKEINTRESOURCE(BMP_SPLASH));
	}

	HBITMAP hBitmap = NULL;

	if (bUseGdip) {
		hBitmap = loadImageFromFile(szPlaceHolder, hWnd, FALSE, FALSE);
	} else {
		TCHAR* ext = getFileExt(szPlaceHolder);
		if (ext && !_tcsicmp(ext, _T("bmp"))) {
			hBitmap = (HBITMAP)LoadImage(hAppInst, szPlaceHolder, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		}
		else if (ext && !_tcsicmp(ext, _T("png"))) {
			hBitmap = loadImageFromFile(szPlaceHolder, hWnd, FALSE, FALSE);
		} else {
			hBitmap = loadAnImage(szPlaceHolder);
		}
	}

	if (hBitmap == NULL) {
		hBitmap = LoadBitmap(hAppInst, MAKEINTRESOURCE(BMP_SPLASH));
	}
	return hBitmap;
}

int selectSkin()
{
	memset(&ofn, 0, sizeof(ofn));
	ofn.lpstrTitle = _T("Select Background Skin");
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hScrnWnd;
	ofn.lpstrFilter = _T("Images (*.png;*.bmp;*.jpg;*.gif)\0*.png;*.bmp;*.jpg;*.gif\0All Files (*.*)\0*.*\0\0");
	ofn.lpstrFile = szPlaceHolder;
	ofn.nMaxFile = sizearray(szPlaceHolder);
	ofn.lpstrInitialDir = getMiscPath(PATH_SKIN);
	ofn.Flags = OFN_NOCHANGEDIR | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	ofn.lpstrDefExt = _T("png");

	int bOldPause = bRunPause;
	bRunPause = 1;

	int nRet = GetOpenFileName(&ofn);
	bRunPause = bOldPause;

	if (nRet == 0) {
		return 1;
	}
	return 0;
}

// most source from MAME32. ^^
void randomSelectSkin()
{
	if (!nRandomSkin) {
		return;
	}

	WIN32_FIND_DATA wfd;
	HANDLE hFind;
	TCHAR szFile[MAX_PATH];
	unsigned int count = 0;
	const TCHAR* pszDir = getMiscPath(PATH_SKIN);

	TCHAR* buf = (TCHAR *)malloc(_MAX_FNAME * MAX_SKINFILES * sizeof(TCHAR));
	if (buf == NULL)
		return;

	_stprintf(szFile, _T("%s*.bmp"), pszDir);
	hFind = FindFirstFile(szFile, &wfd);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				continue;
			}

			memcpy(buf + count * _MAX_FNAME, wfd.cFileName, _MAX_FNAME);
			count++;
		} while (count < MAX_SKINFILES && FindNextFile(hFind, &wfd));

		FindClose(hFind);
	}

	_stprintf(szFile, _T("%s*.png"), pszDir);
	hFind = FindFirstFile(szFile, &wfd);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				continue;
			}

			memcpy(buf + count * _MAX_FNAME, wfd.cFileName, _MAX_FNAME);
			count++;
		} while (count < MAX_SKINFILES && FindNextFile(hFind, &wfd));

		FindClose(hFind);
	}

	_stprintf(szFile, _T("%s*.jpg"), pszDir);
	hFind = FindFirstFile(szFile, &wfd);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				continue;
			}

			memcpy(buf + count * _MAX_FNAME, wfd.cFileName, _MAX_FNAME);
			count++;
		} while (count < MAX_SKINFILES && FindNextFile(hFind, &wfd));

		FindClose(hFind);
	}

	_stprintf(szFile, _T("%s*.gif"), pszDir);
	hFind = FindFirstFile(szFile, &wfd);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				continue;
			}

			memcpy(buf + count * _MAX_FNAME, wfd.cFileName, _MAX_FNAME);
			count++;
		} while (count < MAX_SKINFILES && FindNextFile(hFind, &wfd));

		FindClose(hFind);
	}

	if (count) {
		srand((unsigned int)time(NULL));
		_stprintf(szPlaceHolder, _T("%s%s"), pszDir, buf + (rand() % count) * _MAX_FNAME);
	}

	free(buf);
}
