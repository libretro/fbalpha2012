// image resize, added by regret

/* changelog:
 update 2: changed to use SetStretchBltMode(, HALFTONE)
 update 1: create
*/

#include "burner.h"
#include "imageresize.h"

// This is for non-rotated games
#define PNG_XSIZE_HORI 304
#define PNG_YSIZE_HORI 224
// This is for rotated games
#define PNG_XSIZE_VERT 168
#define PNG_YSIZE_VERT 224

static HBITMAP hResizedBitmap = NULL;
static int offsetx, offsety;

// ==> resize png with libpng
#if 0
#include "png.h"

// Resize the image to the required size using point filtering
int img_resize(IMAGE* img)
{
	if (!img) {
		return 1;
	}

	if ((img->width == PNG_XSIZE_HORI && img->height == PNG_YSIZE_HORI)
		|| (img->width == PNG_XSIZE_VERT && img->height == PNG_YSIZE_VERT)) {
		return 0;
	}

	bImageOrientation = 0;
	if (img->width < img->height) {
		bImageOrientation = 1;
	}

	IMAGE new_img;
	memset(&new_img, 0, sizeof(IMAGE));
	new_img.width = bImageOrientation ? PNG_XSIZE_VERT : PNG_XSIZE_HORI;
	new_img.height = bImageOrientation ? PNG_YSIZE_VERT : PNG_YSIZE_HORI;
	if (img_alloc(&new_img)) {
		return 1;
	}

	for (int y = 0; y < new_img.height; y++) {
		int row = img->height * y / new_img.height;
		for (int x = 0; x < new_img.width; x++) {
			new_img.rowptr[y][x * 3 + 0] = img->rowptr[row][img->width * x / new_img.width * 3 + 0];
			new_img.rowptr[y][x * 3 + 1] = img->rowptr[row][img->width * x / new_img.width * 3 + 1];
			new_img.rowptr[y][x * 3 + 2] = img->rowptr[row][img->width * x / new_img.width * 3 + 2];
		}
	}

	img_free(img);

	memcpy(img, &new_img, sizeof(IMAGE));

	return 0;
}
#endif
// <== resize png with libpng


void resizeImageFree()
{
	if (hResizedBitmap) {
		DeleteObject(hResizedBitmap);
		hResizedBitmap = NULL;
	}
}

// * Remember to free return HBITMAP!
static HBITMAP scaleBitmap(HWND hWnd, HBITMAP hBitmap, int width, int height)
{
	resizeImageFree();

	if (hBitmap == NULL) {
		return NULL;
	}

	HDC	hdc = GetDC(hWnd);
	HBITMAP	hNewBitmap = CreateCompatibleBitmap(hdc, width, height);
	HDC	hMemDCDest = CreateCompatibleDC(hdc);
	HDC	hMemDCSrc = CreateCompatibleDC(hdc);

	if (!hNewBitmap || !hMemDCDest || !hMemDCSrc) {
		return NULL;
	}

	BITMAP bm;
	GetObject(hBitmap, sizeof(bm), &bm);

	HBITMAP	hOldBmpDest = (HBITMAP)SelectObject(hMemDCDest, hNewBitmap);
	HBITMAP	hOldBmpSrc = (HBITMAP)SelectObject(hMemDCSrc, hBitmap);

	SetStretchBltMode(hMemDCDest, HALFTONE);
	SetBrushOrgEx(hMemDCDest, 0, 0, 0);
	StretchBlt(hMemDCDest, 0, 0, width, height, hMemDCSrc, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);

	// free
	SelectObject(hMemDCDest, hOldBmpDest);
	SelectObject(hMemDCSrc, hOldBmpSrc);
	DeleteDC(hMemDCDest);
	DeleteDC(hMemDCSrc);
	ReleaseDC(hWnd, hdc);

	return hNewBitmap;
}

// calculate proper width and height, added by regret
// param: aspect = w / h
static inline void calcSize(unsigned int& outw, unsigned int& outh, const unsigned int& w, const unsigned int& h, const float& aspect)
{
	if (w == 0 || h == 0) {
		return;
	}

	bool vertical = false;
	if (w < h) {
		vertical = true;
	}

	outw = w; outh = h;

	if (!vertical) {
		if (outw > PNG_XSIZE_HORI) {
			while (outw > PNG_XSIZE_HORI) {
				outw--;
			}
			outh = outw / aspect;
		}

		if (outh > PNG_YSIZE_HORI) {
			while (outh > PNG_YSIZE_HORI) {
				outh--;
			}
			outw = outh * aspect;
		}
	} else {
		if (outh > PNG_YSIZE_HORI) {
			while (outh > PNG_YSIZE_HORI) {
				outh--;
			}
			outw = outh * aspect;
		}
	}
}

void getResizeOffset(int& x, int& y)
{
	x = offsetx;
	y = offsety;
}

HBITMAP resizeBitmap(HWND hWnd, HBITMAP hBitmap, unsigned int width, unsigned int height, const BOOL& keepAspect)
{
	if (hBitmap == NULL) {
		return NULL;
	}

	unsigned int myWidth = 0, myHeight = 0;
	float aspect = (float)width / height;

	// check aspect
	if (keepAspect) {
//		int gameWidth, gameHeight;
//		BurnDrvGetVisibleSize(&gameWidth, &gameHeight);
		int aspectX, aspectY;
		BurnDrvGetAspect(&aspectX, &aspectY);

		aspect = (float)aspectX / aspectY;
	}

	calcSize(myWidth, myHeight, width, height, aspect);

	offsetx = (PNG_XSIZE_HORI - myWidth) / 2;
	offsety = (PNG_YSIZE_HORI - myHeight) / 2;

	if (width != myWidth || height != myHeight) {
		// resize hBitmap
		hResizedBitmap = scaleBitmap(hWnd, hBitmap, myWidth, myHeight);
		return hResizedBitmap ? hResizedBitmap : hBitmap;
	}

	return hBitmap;
}
