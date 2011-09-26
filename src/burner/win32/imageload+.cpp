// image load plus module, added by regret
// load image (mainly png) from file or memory

/* changelog:
 update 5: move icon load to separate file
 update 4: add load icon
 update 3: remove PNGlib, restore load png with libpng and add load from memory
 update 2: add load image through GDI+
 update 1: use PNGlib to load png
*/

#include "burner.h"
#include "imageload+.h"
#include "imageresize.h"

#include "png.h"
#include <ole2.h>
#include <olectl.h>

bool bUseGdip = false; // use GDI+

#ifndef NO_GDI_PLUS

#ifdef _MSC_VER
#pragma comment(lib, "gdiplus")
#endif
#include <gdiplus.h>
using namespace Gdiplus;

bool init_gdip = false;
GdiplusStartupInput gdiplusStartupInput;
ULONG_PTR gdiplusToken;
static Color color(0xff, 0xff, 0xff);

void GDIPInit()
{
	if (!init_gdip) {
		GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
		init_gdip = true;
	}
}

void GDIPShutdown()
{
	if (init_gdip) {
		GdiplusShutdown(gdiplusToken);
	}
}

HBITMAP loadImageFromFile(TCHAR* filename, HWND hWnd, const BOOL& keepAspect, const BOOL& resize)
{
	if (!filename) {
		return NULL;
	}

	if (!bUseGdip) {
		return loadPNGFromFile(filename, hWnd, keepAspect, resize);
	}

	HBITMAP hBitmap = NULL;
	Bitmap bm(filename);
	bm.GetHBITMAP(color, &hBitmap);

	if (hBitmap == NULL) {
		return NULL;
	}

	if (resize) {
		hBitmap = resizeBitmap(hWnd, hBitmap, bm.GetWidth(), bm.GetHeight(), keepAspect);
	}
	return hBitmap;
}

HBITMAP loadImageFromBuffer(const void* buf, unsigned int size, HWND hWnd, const BOOL& keepAspect, const BOOL& resize)
{
	if (!buf) {
		return NULL;
	}

	if (!bUseGdip) {
		return loadPNGFromBuffer(buf, size, hWnd, keepAspect, resize);
	}

	HGLOBAL hBuffer = ::GlobalAlloc(GMEM_MOVEABLE, size);
	if (!hBuffer) {
		return NULL;
	}

	Bitmap* pBitmap = NULL;
	Status stat;

	void* pBuffer = ::GlobalLock(hBuffer);
	if (pBuffer)
	{
		memcpy(pBuffer, buf, size);

		IStream* pStream = NULL;
		if (::CreateStreamOnHGlobal(hBuffer, FALSE, &pStream) == S_OK) {
			// create a new Image object based on pStream
			pBitmap = Bitmap::FromStream(pStream);
			stat = pBitmap->GetLastStatus();
			pStream->Release();
		}
		::GlobalUnlock(hBuffer);
	}
	::GlobalFree(hBuffer);

	if (stat != Ok) {
		if (pBitmap) {
			delete pBitmap;
		}
		return NULL;
	}

	HBITMAP hBitmap = NULL;
	pBitmap->GetHBITMAP(color, &hBitmap);

	if (resize) {
		hBitmap = resizeBitmap(hWnd, hBitmap, pBitmap->GetWidth(), pBitmap->GetHeight(), keepAspect);
	}

	if (pBitmap) {
		delete pBitmap;
	}

	return hBitmap;
}

#endif /* NO_GDI_PLUS */


// ==> load image with OleLoadPicture
HBITMAP loadAnImage(TCHAR* szFileName)
{
	if (!szFileName) {
		return NULL;
	}

	// Read file in memory
	FILE* file = _tfopen(szFileName, _T("rb"));
	if (!file) return NULL;

	long size = getFileSize(file);

	HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, size);
	if (!hGlobal) {
		fclose(file);
		return NULL;
	}

	void* pData = GlobalLock(hGlobal);
	fread(pData, 1, size, file);
	fclose(file);
	GlobalUnlock(hGlobal);

	// Use IPicture stuff to load JPG / GIF files
	IPicture* pPicture = NULL;
	IStream* pStream = NULL;

	if (CreateStreamOnHGlobal(hGlobal, TRUE, &pStream) != S_OK) {
		GlobalFree(hGlobal);
		return NULL;
	}
	if (OleLoadPicture(pStream, size, FALSE, IID_IPicture, (LPVOID *)&pPicture) != S_OK) {
		pPicture = NULL;
	}
	pStream->Release();
	GlobalFree(hGlobal);
	if (!pPicture) return NULL;

	HBITMAP hB = NULL;
	pPicture->get_Handle((unsigned int*)&hB);
	// Copy the image
	// Necessary, because upon pPicture's release, the handle is destroyed
	HBITMAP hBB = (HBITMAP)CopyImage(hB, IMAGE_BITMAP, 0, 0, LR_COPYRETURNORG);
	pPicture->Release();
	return hBB;
}
// <== load image with OleLoadPicture

// ==> Load png with libpng, modified by regret
#define PNG_SIG_CHECK_BYTES 4

typedef struct tagIMAGE {
	LONG   width;
	LONG   height;
	DWORD  rowbytes;
	DWORD  imgbytes;
	BYTE** rowptr;
	BYTE*  bmpbits;
} IMAGE;

typedef struct READDATA
{
	BYTE* buffer;
	unsigned int size;
	unsigned int* done_bytes;
} READDATA, *LPREADDATA;

static inline void img_free(IMAGE* image)
{
	if (!image) {
		return;
	}

	free(image->rowptr);
	free(image->bmpbits);
}

static inline int img_alloc(IMAGE* image)
{
	if (!image) {
		return 1;
	}

	image->rowbytes = ((DWORD)image->width * 24 + 31) / 32 * 4;
	image->imgbytes = image->rowbytes * image->height;
	image->rowptr = (BYTE**)malloc((size_t)image->height * sizeof(BYTE*));
	image->bmpbits = (BYTE*)malloc((size_t)image->imgbytes);

	if (image->rowptr == NULL || image->bmpbits == NULL) {
		img_free(image);
		return 1;
	}

	for (int y = 0; y < image->height; y++) {
		image->rowptr[image->height - y - 1] = image->bmpbits + y * image->rowbytes;
	}

	return 0;
}

static HBITMAP createBitmap(HWND hWnd, IMAGE* image)
{
	if (!image) {
		return NULL;
	}

	// Create a bitmap for the image
	BITMAPINFO* bi = (BITMAPINFO*)HeapAlloc(GetProcessHeap(), 0, sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD));
	if (bi == NULL) {
		img_free(image);
		return NULL;
	}

	bi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bi->bmiHeader.biWidth = image->width;
	bi->bmiHeader.biHeight = image->height;
	bi->bmiHeader.biPlanes = 1;
	bi->bmiHeader.biBitCount = 24;
	bi->bmiHeader.biCompression = BI_RGB;
	bi->bmiHeader.biSizeImage = image->imgbytes;
	bi->bmiHeader.biXPelsPerMeter = 0;
	bi->bmiHeader.biYPelsPerMeter = 0;
	bi->bmiHeader.biClrUsed = 0;
	bi->bmiHeader.biClrImportant = 0;

	HDC hDC = GetDC(hWnd);
	BYTE* pBits = NULL;
	HBITMAP hBitmap = CreateDIBSection(hDC, (BITMAPINFO*)bi, DIB_RGB_COLORS, (void**)&pBits, NULL, 0);
	if (pBits) {
		memcpy(pBits, image->bmpbits, image->imgbytes);
	}
	ReleaseDC(hWnd, hDC);
	HeapFree(GetProcessHeap(), 0, bi);
	img_free(image);

	return hBitmap;
}

static void png_warning(png_structp png_ptr, png_const_charp message)
{
	fprintf(stdout, "libpng warning: %s\n", message);
}

static void png_error(png_structp png_ptr, png_const_charp message)
{
   png_warning(png_ptr, message);
   longjmp(png_ptr->jmpbuf, 1);
}

static void png_cleanup(png_structp png_ptr, png_infop info_ptr, FILE* file)
{
	if (file) {
		fclose(file);
	}

	png_destroy_read_struct(png_ptr ? &png_ptr : (png_structpp)NULL,
		info_ptr ? &info_ptr : (png_infopp)NULL,
		(png_infopp)NULL);
}

HBITMAP loadPNGFromFile(TCHAR* filename, HWND hWnd, const BOOL& keepAspect, const BOOL& resize)
{
	if (!filename) {
		return NULL;
	}
	FILE* file = _tfopen(filename, _T("rb"));
	if (!file) {
		return NULL;
	}

	png_structp png_ptr = NULL;
	png_infop info_ptr = NULL;

	// check signature
	unsigned char pngsig[PNG_SIG_CHECK_BYTES];
	if (fread(pngsig, 1, PNG_SIG_CHECK_BYTES, file) != PNG_SIG_CHECK_BYTES
		|| png_sig_cmp(pngsig, (png_size_t)0, PNG_SIG_CHECK_BYTES)) {
		png_cleanup(png_ptr, info_ptr, file);
		return NULL;
	}

	// Initialize stuff
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr) {
		png_cleanup(png_ptr, info_ptr, file);
		return NULL;
	}

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		png_cleanup(png_ptr, info_ptr, file);
		return NULL;
	}
	png_set_error_fn(png_ptr, NULL, (png_error_ptr)png_error, (png_error_ptr)png_warning); // set error functions

	if (setjmp(png_jmpbuf(png_ptr))) {
		png_cleanup(png_ptr, info_ptr, file);
		return NULL;
	}
	png_init_io(png_ptr, file);

	if (setjmp(png_jmpbuf(png_ptr))) {
		png_cleanup(png_ptr, info_ptr, file);
		return NULL;
	}
	png_set_sig_bytes(png_ptr, PNG_SIG_CHECK_BYTES);

	if (setjmp(png_jmpbuf(png_ptr))) {
		png_cleanup(png_ptr, info_ptr, file);
		return NULL;
	}
	png_read_info(png_ptr, info_ptr);

	png_uint_32 width, height;
	int bit_depth, color_type;
	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, NULL, NULL, NULL);
    if (!(width > 0 && height > 0)) {
		png_cleanup(png_ptr, info_ptr, file);
		return NULL;
    }

//	if (width > 1024 || height > 1024) {
//		longjmp(png_ptr->jmpbuf, 1);
//	}

	// Instruct libpng to convert the image to 24-bit RGB format
	if (color_type == PNG_COLOR_TYPE_PALETTE) {
		png_set_palette_to_rgb(png_ptr);
	}
	if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
		png_set_gray_to_rgb(png_ptr);
	}
	if (bit_depth == 16) {
		png_set_strip_16(png_ptr);
	}
	if (color_type & PNG_COLOR_MASK_ALPHA) {
		png_set_strip_alpha(png_ptr);
	}

	IMAGE img;
	memset(&img, 0, sizeof(IMAGE));

	// We need the image to be either wxhx24 or hxwx24
	img.width = (LONG)width;
	img.height = (LONG)height;

	// Initialize our img structure
	if (img_alloc(&img)) {
		longjmp(png_ptr->jmpbuf, 1);
	}

	// If bad things happen in libpng we need to do img_free(&img) as well
	if (setjmp(png_jmpbuf(png_ptr))) {
		png_cleanup(png_ptr, info_ptr, file);
		img_free(&img);
		return NULL;
	}

	// Read the .PNG image
	png_set_bgr(png_ptr);
	png_read_update_info(png_ptr, info_ptr);

	if (setjmp(png_jmpbuf(png_ptr))) {
		png_cleanup(png_ptr, info_ptr, file);
		img_free(&img);
		return NULL;
	}
	png_read_image(png_ptr, img.rowptr);
	png_read_end(png_ptr, (png_infop)NULL);

	png_cleanup(png_ptr, info_ptr, file);

	// Create a bitmap for the image
	HBITMAP hBitmap = createBitmap(hWnd, &img);
	if (hBitmap && resize) {
		return resizeBitmap(hWnd, hBitmap, width, height, keepAspect);
	}
	return hBitmap;
}

// read callback
static void pngReadCallback(png_structp png_ptr, png_bytep data, png_size_t length)
{
	if (!png_ptr || !png_ptr->jmpbuf) {
		return;
	}

	LPREADDATA pwd = (LPREADDATA)png_get_io_ptr(png_ptr);
	if (!pwd || !pwd->done_bytes) {
		longjmp(png_ptr->jmpbuf, E_POINTER);
	}

	if (pwd->buffer) {
		if (*pwd->done_bytes + length > pwd->size) {
			longjmp(png_ptr->jmpbuf, E_OUTOFMEMORY);
		}
		memcpy(data, pwd->buffer + *pwd->done_bytes, length);
		*pwd->done_bytes += length;
	}
}

HBITMAP loadPNGFromBuffer(const void* buf, unsigned int size, HWND hWnd, const BOOL& keepAspect, const BOOL& resize)
{
	if (!buf || size < PNG_SIG_CHECK_BYTES) {
		return NULL;
	}

	// check signature
	if (png_sig_cmp((png_bytep)buf, (png_size_t)0, PNG_SIG_CHECK_BYTES)) {
		return NULL;
	}

	// Initialize stuff
	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr) {
		return NULL;
	}

	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		png_cleanup(png_ptr, info_ptr, NULL);
		return NULL;
	}
	png_set_error_fn(png_ptr, NULL, (png_error_ptr)png_error, (png_error_ptr)png_warning); // set error functions

	if (setjmp(png_jmpbuf(png_ptr))) {
		png_cleanup(png_ptr, info_ptr, NULL);
		return NULL;
	}

	// set our own read function
	READDATA srd;
	srd.buffer = (BYTE*)buf;
	srd.size = size;
	unsigned int read_size = 0;
	srd.done_bytes = &read_size;
	png_set_read_fn(png_ptr, (png_voidp)&srd, (png_rw_ptr)pngReadCallback);

	if (setjmp(png_jmpbuf(png_ptr))) {
		png_cleanup(png_ptr, info_ptr, NULL);
		return NULL;
	}
    // if read from memory, the num_bytes must be 0
	png_set_sig_bytes(png_ptr, 0 /*PNG_SIG_CHECK_BYTES*/);

	if (setjmp(png_jmpbuf(png_ptr))) {
		png_cleanup(png_ptr, info_ptr, NULL);
		return NULL;
	}
	png_read_info(png_ptr, info_ptr);

	png_uint_32 width, height;
	int bit_depth, color_type;
	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, NULL, NULL, NULL);
    if (!(width > 0 && height > 0)) {
		png_cleanup(png_ptr, info_ptr, NULL);
		return NULL;
    }

	// Instruct libpng to convert the image to 24-bit RGB format
	if (color_type == PNG_COLOR_TYPE_PALETTE) {
		png_set_palette_to_rgb(png_ptr);
	}
	if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
		png_set_gray_to_rgb(png_ptr);
	}
	if (bit_depth == 16) {
		png_set_strip_16(png_ptr);
	}
	if (color_type & PNG_COLOR_MASK_ALPHA) {
		png_set_strip_alpha(png_ptr);
	}

	IMAGE img;
	memset(&img, 0, sizeof(IMAGE));

	// We need the image to be either wxhx24 or hxwx24
	img.width = (LONG)width;
	img.height = (LONG)height;

	// Initialize our img structure
	if (img_alloc(&img)) {
		longjmp(png_ptr->jmpbuf, 1);
	}

	// If bad things happen in libpng we need to do img_free(&img) as well
	if (setjmp(png_jmpbuf(png_ptr))) {
		png_cleanup(png_ptr, info_ptr, NULL);
		img_free(&img);
		return NULL;
	}

	// Read the .PNG image
	png_set_bgr(png_ptr);
	png_read_update_info(png_ptr, info_ptr);

	if (setjmp(png_jmpbuf(png_ptr))) {
		png_cleanup(png_ptr, info_ptr, NULL);
		img_free(&img);
		return NULL;
	}
	png_read_image(png_ptr, img.rowptr);
	png_read_end(png_ptr, (png_infop)NULL);

	png_cleanup(png_ptr, info_ptr, NULL);

	// Create a bitmap for the image
	HBITMAP hBitmap = createBitmap(hWnd, &img);
	if (hBitmap && resize) {
		return resizeBitmap(hWnd, hBitmap, width, height, keepAspect);
	}
	return hBitmap;
}
// <== Load png with libpng
