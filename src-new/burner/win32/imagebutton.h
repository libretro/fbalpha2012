//******************************************
// Author:  Massimo Galbusera       http://www.winapizone.net
// Program: ImageButton.exe
// FILE:    ImageButton.h
//******************************************

enum ibOptions { 
    IBO_MAINTAIN = -1, 
    IBO_GRAY = -2, 
    IBO_LIGHTEN = -3
};

void ImageButton_EnableXPThemes();

BOOL ImageButton_Create(HWND button);
BOOL ImageButton_Create(HWND hwndParent, UINT ctrlID);
void ImageButton_Create(int buttonCount, ...);

BOOL ImageButton_SetBitmap(HWND button, HBITMAP normalBitmap, HBITMAP disabledBitmap=NULL, HBITMAP hoverBitmap=NULL);
BOOL ImageButton_SetBitmap(HWND button, HINSTANCE instance, UINT normalBitmap, UINT disabledBitmapId, UINT hoverBitmap);
BOOL ImageButton_SetBitmap(HWND button, UINT normalBitmapID, UINT disabledBitmapId, UINT hoverBitmapID);

BOOL ImageButton_SetIcon(HWND button, HICON normalIcon, HICON disabledIcon, HICON hoverIcon);
BOOL ImageButton_SetIcon(HWND button, HINSTANCE instance, UINT normalIconID, UINT disabledIconId, UINT hoverIconID, int iconWidth, int iconHeight);
BOOL ImageButton_SetIcon(HWND button, UINT normalIconID, UINT disabledIconId, UINT hoverIconID, int iconWidth=16, int iconHeight=16);

BOOL ImageButton_SetCursor(HWND button, HCURSOR hCursor);
BOOL ImageButton_SetCursor(HWND button, HINSTANCE instance, UINT cursorID);

BOOL ImageButtons_SetCursor(HCURSOR cursor);
BOOL ImageButtons_SetCursor(HINSTANCE instance, UINT cursorID);
