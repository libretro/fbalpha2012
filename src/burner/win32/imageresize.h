// image resize, added by regret
#ifndef IMAGERESIZE_H
#define IMAGERESIZE_H

//int img_resize(IMAGE* img);

void resizeImageFree();
HBITMAP resizeBitmap(HWND hWnd, HBITMAP hBitmap, unsigned int width, unsigned int height, const BOOL& keepAspect);

#endif // IMAGERESIZE_H
