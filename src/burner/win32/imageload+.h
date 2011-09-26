// image load module, added by regret
#ifndef _IMAGELOADP_H_
#define _IMAGELOADP_H_

#ifndef NO_GDI_PLUS
// image load use GDI+
void GDIPInit();
void GDIPShutdown();
#endif

HBITMAP loadImageFromFile(TCHAR* filename, HWND hWnd = NULL, const BOOL& keepAspect = FALSE, const BOOL& resize = TRUE);
HBITMAP loadImageFromBuffer(const void* buf, unsigned int size, HWND hWnd = NULL, const BOOL& keepAspect = FALSE, const BOOL& resize = TRUE);

HBITMAP loadAnImage(TCHAR* szFileName);
HBITMAP loadPNGFromFile(TCHAR* filename, HWND hWnd, const BOOL& keepAspect = FALSE, const BOOL& resize = TRUE);
HBITMAP loadPNGFromBuffer(const void* buf, unsigned int size, HWND hWnd, const BOOL& keepAspect = FALSE, const BOOL& resize = TRUE);

#endif // _IMAGELOADP_H_
