#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif

#ifndef WM_THEMECHANGED
#define WM_THEMECHANGED 0x031A 
#endif

#include "burner.h"
#include <uxtheme.h>
#include <tmschema.h>
#include <vector>

#define ORIGINALPROCPROP		TEXT("ImageButtonOriginalProc")

#ifdef __GNUC__
#pragma GCC system_header
#pragma Warning(Off, "*ISO C++ forbids casting between pointer-to-function and pointer-to-object*")
#endif

//typedef std::basic_string<TCHAR> tstring;
typedef struct tagIMAGEBUTTON
{
    //Constructor to avoid crash with MinGW (with ZeroMemory)
    //or compiler error on Visual C++ (with "= {0}", dunno how to call that :)
    tagIMAGEBUTTON() : button(NULL), cursor(NULL), normalBitmap(NULL), 
        disabledBitmap(NULL), hoverBitmap(NULL), normalIcon(NULL), disabledIcon(NULL),
        hoverIcon(NULL), isIcon(FALSE), isOver(FALSE), isPressed(FALSE) 
    { 
        imageSize.cx = 0;
        imageSize.cy = 0; 
    }
    
    HWND button;                    //button handle
    HCURSOR cursor;                 //cursor handle
    HBITMAP normalBitmap;           //normal state bitmap
    HBITMAP disabledBitmap;         //disabled state bitmap
    HBITMAP hoverBitmap;            //hover state bitmap
    HICON normalIcon;               //normal state icon
    HICON disabledIcon;             //disabled state icon
    HICON hoverIcon;                //hover state icon
    BOOL isIcon;                    //TRUE = icon FALSE = bitmap
    BOOL isOver;                    //TRUE = mouse hovers button
    BOOL isPressed;                 //TRUE = button is pressed
    SIZE imageSize;                 //icon size
} IMAGEBUTTON;

std::vector<IMAGEBUTTON> imageButtons;
HTHEME buttonTheme = NULL;
BOOL enableButtonTheme = FALSE;

namespace ib
{
    typedef HRESULT (WINAPI *PFNCLOSETHEMEDATA)(HTHEME);
    typedef HRESULT (WINAPI *PFNDRAWTHEMEBACKGROUND)(HTHEME,HDC,int,int,const LPRECT,const LPRECT);
    typedef HTHEME (WINAPI *PFNOPENTHEMEDATA)(HWND,LPCWSTR);
    typedef HRESULT (WINAPI *PFNDRAWTHEMETEXT)(HTHEME,HDC,int,int,LPCWSTR,int,DWORD,DWORD,const LPRECT);
    typedef HRESULT (WINAPI *PFNGETTHEMETEXTEXTENT)(HTHEME,HDC,int,int,LPCWSTR,int,DWORD,const LPRECT,LPRECT);
    typedef COLORREF (*PFNMODIFYCOLOR)(COLORREF);
    
    PFNOPENTHEMEDATA pOpenThemeData = NULL;
    PFNDRAWTHEMEBACKGROUND pDrawThemeBackground = NULL;
    PFNCLOSETHEMEDATA pCloseThemeData = NULL;
    PFNDRAWTHEMETEXT pDrawThemeText = NULL;
    PFNGETTHEMETEXTEXTENT pGetThemeTextExtent = NULL;
    HMODULE themesDll = NULL;
    
    BOOL InitThemesDll()
    {
        themesDll = LoadLibrary(_T("UXTHEME.DLL"));
        if(themesDll)
        {
            pOpenThemeData = (PFNOPENTHEMEDATA)GetProcAddress(themesDll, "OpenThemeData");
            pDrawThemeBackground = (PFNDRAWTHEMEBACKGROUND)GetProcAddress(themesDll, "DrawThemeBackground");
            pCloseThemeData = (PFNCLOSETHEMEDATA)GetProcAddress(themesDll, "CloseThemeData");
            pDrawThemeText = (PFNDRAWTHEMETEXT)GetProcAddress(themesDll, "DrawThemeText");
            pGetThemeTextExtent = (PFNGETTHEMETEXTEXTENT)GetProcAddress(themesDll, "GetThemeTextExtent");

            if (pOpenThemeData && pCloseThemeData && pDrawThemeBackground
             && pDrawThemeText && pGetThemeTextExtent)
            {
                return TRUE;			
            }
            
            //Something went wrong!
            FreeLibrary(themesDll);
            themesDll = NULL;
            
            return FALSE;
        }
        
        return FALSE;
    }
    void FreeThemesDll()
    {
        pCloseThemeData(buttonTheme);
		buttonTheme = NULL; //this line
        
        FreeLibrary(themesDll);
        themesDll = NULL;
    }
    int FindStructNumber(HWND hwnd)
    {
        int index = -1;
        for (unsigned int i = 0; i<imageButtons.size(); i++)
        {
            if(hwnd == imageButtons[i].button)
            {
                index = i;
                break;
            }    
        }
        
        return index;
    }
    COLORREF LightenColor(COLORREF color)
    {
		BYTE red = GetRValue(color);
		BYTE green = GetGValue(color);
		BYTE blue = GetBValue(color);
		
		double factor = 0.4;
		BYTE lightRed = (BYTE)(factor*(255-red) + red);
		BYTE lightGreen = (BYTE)(factor*(255-green) + green);
		BYTE lightBlue = (BYTE)(factor*(255-blue) + blue);
		
		return RGB(lightRed, lightGreen, lightBlue);
    }
    COLORREF GrayColor(COLORREF color)
    {
        BYTE newColor = (BYTE)(
            (GetRValue(color) * 0.299) +
            (GetGValue(color) * 0.587) +
            (GetBValue(color) * 0.114)
        );
        
        return LightenColor( RGB(newColor, newColor, newColor) );
    }
    
    HBITMAP ModifyBitmap(HBITMAP bitmap, PFNMODIFYCOLOR pModifyColor)
    {
        if (!bitmap)
            return NULL;
        
        HDC	mainDC = GetDC(NULL);
        HDC	memDC1 = CreateCompatibleDC(mainDC);
        HDC	memDC2 = CreateCompatibleDC(mainDC);
        
        BITMAP bm;
        GetObject(bitmap, sizeof(bm), &bm);
        
        HBITMAP	grayBitmap = CreateCompatibleBitmap(mainDC, bm.bmWidth, bm.bmHeight);
        if (grayBitmap)
        {
            HBITMAP	oldBmp1 = (HBITMAP)SelectObject(memDC1, grayBitmap);
            HBITMAP	oldBmp2 = (HBITMAP)SelectObject(memDC2, bitmap);
            
            for (int y = 0; y<bm.bmHeight; y++)
            {
                for (int x = 0; x<bm.bmWidth; x++)
                {
                    COLORREF pixelColor = GetPixel(memDC2, x,y);
                    COLORREF newPixelColor = (*pModifyColor)(pixelColor);
                    
                    SetPixel(memDC1, x,y, newPixelColor);
                }
            }
            
            SelectObject(memDC1, oldBmp1);
            SelectObject(memDC2, oldBmp2);
        }
        
        DeleteDC(memDC1);
        DeleteDC(memDC2);
        ReleaseDC(NULL, mainDC);
        
        return grayBitmap;
    }
    HICON ModifyIcon(HICON icon, PFNMODIFYCOLOR pModifyColor)
    {
        if (!icon)
            return NULL;
        
        HDC mainDC = GetDC(NULL);
        HDC memDC1 = CreateCompatibleDC(mainDC);
        HDC memDC2 = CreateCompatibleDC(mainDC);
        
        ICONINFO iconInfo;  GetIconInfo(icon, &iconInfo);
        BITMAP bm;          GetObject(iconInfo.hbmColor, sizeof(BITMAP), &bm);
        
        ICONINFO grayedIconInfo;
        grayedIconInfo.hbmColor = CreateBitmap(iconInfo.xHotspot*2, iconInfo.yHotspot*2, 
            bm.bmPlanes, bm.bmBitsPixel, NULL);
        
        HBITMAP oldBmp1 = (HBITMAP)SelectObject(memDC1, iconInfo.hbmColor);
        HBITMAP oldBmp2 = (HBITMAP)SelectObject(memDC2, grayedIconInfo.hbmColor);
        BitBlt(memDC2, 0,0, iconInfo.xHotspot*2,iconInfo.yHotspot*2, memDC1, 0,0,SRCCOPY);
        
        for (unsigned int y = 0; y < iconInfo.yHotspot*2; y++)
        {
            for (unsigned int x = 0; x < iconInfo.xHotspot*2; x++)
            {
                COLORREF pixelColor = GetPixel(memDC2, x,y);
                COLORREF newPixelColor = (*pModifyColor)(pixelColor);
                
                if (pixelColor)
                    SetPixel(memDC2, x,y, newPixelColor);
            } // for
        } // for
        
        SelectObject(memDC1, oldBmp1);
        SelectObject(memDC2, oldBmp2);
        
        grayedIconInfo.hbmMask = iconInfo.hbmMask;
        grayedIconInfo.fIcon = TRUE;
        HICON grayIcon = CreateIconIndirect(&grayedIconInfo);
        
        DeleteObject(grayedIconInfo.hbmColor);
        //DeleteObject(grayedIconInfo.hbmMask);
        
        DeleteObject(iconInfo.hbmColor);
        DeleteObject(iconInfo.hbmMask);
        DeleteDC(memDC1);
        DeleteDC(memDC2);
        ReleaseDC(NULL, mainDC);
        
        return grayIcon;
    }
    
    void DrawItem(LPDRAWITEMSTRUCT lpdis, int i)
    {
        //Window text
        TCHAR buttonText[256] = _T("");
        GetWindowText(imageButtons[i].button, buttonText, 256);
        int buttonTextSize = _tcslen(buttonText);
        
        //Button draw state...
        DWORD themeState = 0;      //...with xp themes
        DWORD edgeState = 0;       //...without xp themes
        
        //For xp themes only
        WCHAR *wButtonText = NULL;
        int wButtonTextSize = 0;
        
        if (buttonTheme)
        {
            #ifdef UNICODE
                wButtonTextSize = buttonTextSize;
                wButtonText = new WCHAR[buttonTextSize+1];
                wcscpy(wButtonText, buttonText);
            #else
                wButtonTextSize = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, buttonText, 
                    buttonTextSize, NULL, 0);
                wButtonText = new WCHAR[wButtonTextSize];
                MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, buttonText, buttonTextSize, 
                    wButtonText, wButtonTextSize);
            #endif
        }
        
    //Mouse is over and button is pressed	
        if (imageButtons[i].isPressed && imageButtons[i].isOver )
        {
            if (buttonTheme)
                themeState = PBS_PRESSED;
            else
            {
                OffsetRect(&lpdis->rcItem, 1, 1);
                edgeState = EDGE_SUNKEN;
            }
        }
    //Mouse is over, button isn't pressed
        else if ( imageButtons[i].isOver ) 
        {
            if (buttonTheme)
                themeState = PBS_HOT;
            else
                edgeState = EDGE_RAISED;
        }
    //disabled state
        else if (lpdis->itemState & ODS_DISABLED)
        {
            if (buttonTheme)
                themeState = PBS_DISABLED;
            else
            {
                // Set gray text.
                SetTextColor(lpdis->hDC, RGB(135,135,135));
                edgeState = EDGE_RAISED;
            }
        }
    //normal state
        else
        {
            if (buttonTheme)
            {
                if(lpdis->itemState & ODS_FOCUS)
                    themeState  = PBS_DEFAULTED;
                else
                    themeState = PBS_NORMAL;
            }
            else
                edgeState = EDGE_RAISED;
        }
        
        //Check if there'a an image to be drawn
        BOOL isImage = FALSE;
        {
            if (imageButtons[i].isIcon)
            {
                if (lpdis->itemState & ODS_DISABLED && imageButtons[i].disabledIcon)
                    isImage = TRUE;
                else if (imageButtons[i].isOver && imageButtons[i].hoverIcon)
                    isImage = TRUE;
                else if (imageButtons[i].normalIcon)
                    isImage = TRUE;
                else
                    isImage = FALSE;
            }
            else
            {
                if (lpdis->itemState & ODS_DISABLED && imageButtons[i].disabledBitmap)
                    isImage = TRUE;
                else if (imageButtons[i].isOver && imageButtons[i].hoverBitmap)
                    isImage = TRUE;
                else if (imageButtons[i].normalBitmap)
                    isImage = TRUE;
                else
                    isImage = FALSE;
            }
        }
        
        //Do some calculation (text and image position)
        RECT rcText, rcImage;
        {
            //Text will not wrap, so "lpdis->rcItem.bottom" value won't change. What i need is the width!
            RECT temp_rcText = { 0, 0, 1, lpdis->rcItem.bottom};
            
            //Let's calculate how much space the text will take
            if (buttonTheme)
                //DrawThemeText with DT_CALCRECT doesn't work (bug?)
                pGetThemeTextExtent(buttonTheme, lpdis->hDC, BP_PUSHBUTTON, themeState, wButtonText, 
                    wButtonTextSize, DT_CALCRECT, NULL, &temp_rcText);
            else
                DrawText(lpdis->hDC, buttonText, buttonTextSize, &temp_rcText, DT_CALCRECT);
            
            //Caption cannot be wider than the button width plus 
            //the icon width and the border of the button.
            int maxTextWidth = lpdis->rcItem.right - imageButtons[i].imageSize.cx - 6;
            if (temp_rcText.right >= maxTextWidth)
                temp_rcText.right = maxTextWidth -2;
            
            rcText.top = 0;
            rcText.bottom = lpdis->rcItem.bottom;
            if (isImage)
            {
                //Get the left and top position based on text lenght and image dimensions
                rcText.left = (lpdis->rcItem.right/2) - (temp_rcText.right/2) + (imageButtons[i].imageSize.cx/2);
                rcText.right = rcText.left + temp_rcText.right + imageButtons[i].imageSize.cx;
                
                //Image position (-3 is needed to separate text from image)
                rcImage.left = rcText.left - imageButtons[i].imageSize.cx - 3;
                rcImage.top = (lpdis->rcItem.bottom/2) - (imageButtons[i].imageSize.cy/2);
            }
            else
            {
                rcText.left = (lpdis->rcItem.right/2) - (temp_rcText.right/2);
                rcText.right = rcText.left + temp_rcText.right;
            }
        }
        
    // Draw background and text.
        if (buttonTheme)
        {
            pDrawThemeBackground(buttonTheme, lpdis->hDC, BP_PUSHBUTTON, themeState, &(lpdis->rcItem), 0);
            
            pDrawThemeText(buttonTheme, lpdis->hDC, BP_PUSHBUTTON, themeState, wButtonText, wButtonTextSize,
                    DT_VCENTER|DT_SINGLELINE, 0, &rcText);
        }
        else
        {
            SetBkMode(lpdis->hDC, TRANSPARENT);
            FillRect(lpdis->hDC, &(lpdis->rcItem), GetSysColorBrush(COLOR_BTNFACE));
            DrawEdge(lpdis->hDC, &(lpdis->rcItem), edgeState, BF_RECT);
            
            DrawText(lpdis->hDC, buttonText, buttonTextSize, &rcText, 
                DT_VCENTER|DT_SINGLELINE);
        }
        
    // Draw images.
        // If bitmap...
        if (isImage)
        {
            if ( !imageButtons[i].isIcon )
            {
                if (lpdis->itemState & ODS_DISABLED)
                {
                    if (imageButtons[i].disabledBitmap)
                    {
                        DrawState(lpdis->hDC, NULL, NULL, (LONG)imageButtons[i].disabledBitmap, 0, 
                            rcImage.left, rcImage.top, imageButtons[i].imageSize.cx, 
                            imageButtons[i].imageSize.cy, DST_BITMAP);
                    }
                    else
                    {
                        DrawState(lpdis->hDC, NULL, NULL, (LONG)imageButtons[i].normalBitmap, 0, 
                            rcImage.left, rcImage.top, imageButtons[i].imageSize.cx, 
                            imageButtons[i].imageSize.cy, DST_BITMAP|DSS_DISABLED);
                    }
                }
                else if (imageButtons[i].isOver && imageButtons[i].hoverBitmap)
                {
                    DrawState(lpdis->hDC, NULL, NULL, (LONG)imageButtons[i].hoverBitmap, 0, 
                        rcImage.left, rcImage.top, imageButtons[i].imageSize.cx, 
                        imageButtons[i].imageSize.cy, DST_BITMAP);
                }
                else
                {
                    DrawState(lpdis->hDC, NULL, NULL, (LONG)imageButtons[i].normalBitmap, 0, 
                        rcImage.left, rcImage.top, imageButtons[i].imageSize.cx, 
                        imageButtons[i].imageSize.cy, DST_BITMAP);
                }
            }
            //...else if icon...
            else
            {
                // Draw a disabled icon.
                if (lpdis->itemState & ODS_DISABLED)
                {
                    if (imageButtons[i].disabledIcon)
                    {
                        DrawIconEx(lpdis->hDC, rcImage.left, rcImage.top, imageButtons[i].disabledIcon, 
                            imageButtons[i].imageSize.cx, imageButtons[i].imageSize.cy, 0, NULL, DI_NORMAL);
                    }
                    else
                    {
                        DrawState(lpdis->hDC, NULL, NULL, (LONG)imageButtons[i].normalIcon, 0, 
                            rcImage.left, rcImage.top, imageButtons[i].imageSize.cx, 
                            imageButtons[i].imageSize.cy, DST_ICON|DSS_DISABLED);
                    }
                }
                else if (imageButtons[i].isOver && imageButtons[i].hoverIcon)
                {
                    DrawIconEx(lpdis->hDC, rcImage.left, rcImage.top, imageButtons[i].hoverIcon, 
                        imageButtons[i].imageSize.cx, imageButtons[i].imageSize.cy, 0, NULL, DI_NORMAL);
                }
                else
                {
                    DrawIconEx(lpdis->hDC, rcImage.left, rcImage.top, imageButtons[i].normalIcon,
                        imageButtons[i].imageSize.cx, imageButtons[i].imageSize.cy, 0, NULL, DI_NORMAL);
                }
            }
        }
        
        if (wButtonText)
            delete [] wButtonText;
    }

    LRESULT CALLBACK ButtonParentProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        WNDPROC originalProc = (WNDPROC)GetProp(hwnd, ORIGINALPROCPROP);
        
        switch (message)
        {
            case WM_THEMECHANGED:
            {
                if(buttonTheme)
                    pCloseThemeData(buttonTheme);
                buttonTheme = pOpenThemeData(hwnd, L"Button");
            }
            break;
            
            case WM_DRAWITEM:
            {
                LPDRAWITEMSTRUCT lpdis = (LPDRAWITEMSTRUCT)lParam;
                if (lpdis->CtlType == ODT_BUTTON)
                {
                    int i = FindStructNumber(lpdis->hwndItem);
                    if (i != -1)
                        DrawItem(lpdis, i);
                }
            }
            break;
            
            case WM_ENABLE:
            {
                if (wParam == FALSE)
                {
                    //Redraw all the child windows.
                    for (unsigned int i = 0; i<imageButtons.size(); i++)
                    {
                        if (hwnd == GetParent(imageButtons.at(i).button))
                        {
                            imageButtons.at(i).isOver = FALSE;
                            imageButtons.at(i).isPressed = FALSE;
                            
                            //Invalidate client & non-client area
                            SetWindowPos(imageButtons.at(i).button, NULL, 0,0,0,0,
                                SWP_NOZORDER|SWP_NOMOVE|SWP_NOSIZE|SWP_FRAMECHANGED);
                        }
                    }
                }
            }
            break;
            
            case WM_DESTROY:
            {
                SetWindowLong(hwnd, GWL_WNDPROC, (LONG)originalProc);
                RemoveProp(hwnd, ORIGINALPROCPROP);
            }
            break;
        }
        
        return CallWindowProc(originalProc, hwnd, message, wParam, lParam);
    }
    LRESULT CALLBACK ButtonProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        static WNDPROC originalProc = (WNDPROC) GetProp(hwnd, ORIGINALPROCPROP);
        
        // Determine which button this procedure refers to.
        int btnIndex = FindStructNumber(hwnd);
        if (btnIndex == -1)
        {
            SetWindowLong(hwnd, GWL_WNDPROC, (LONG)originalProc);
            RemoveProp(hwnd, ORIGINALPROCPROP);
            return CallWindowProc(originalProc, hwnd, message, wParam, lParam);
        }    
        
        switch (message)
        {
            case WM_SETCURSOR:
                SetCursor(imageButtons[btnIndex].cursor);
                return TRUE;
            
            case WM_LBUTTONDOWN:
                imageButtons[btnIndex].isPressed = TRUE;
                break;
            
            case WM_LBUTTONUP:
            {
                imageButtons[btnIndex].isPressed = FALSE;
                
                //I call the default behavior before releasing the capture,
                //otherwise the parent window will never receive the
                //notification.
                CallWindowProc(originalProc, hwnd, message, wParam, lParam);
                
                ReleaseCapture();
            }
            //No need to call again the original proc
            return 1;
            
            case WM_ENABLE:
            {
                if (wParam == FALSE)
                {
                    //When the visual styles are active..
                    if (buttonTheme)
                    {
                        HWND parent = GetParent(hwnd);
                        
                        //...get the button rect in parent points...
                        RECT buttonRect; GetWindowRect(hwnd, &buttonRect);
                        MapWindowPoints(NULL, parent, (LPPOINT)&buttonRect, 2);
                        
                        //...and redraw only the area parent window where the 
                        //button is (because it'll look bad if we dont' do this)
                        InvalidateRect(parent, &buttonRect, TRUE);
                    }
                }
            }
            break;
            
            case WM_DESTROY:
            {
                DeleteObject(imageButtons[btnIndex].hoverBitmap);
                DeleteObject(imageButtons[btnIndex].disabledBitmap);
                DeleteObject(imageButtons[btnIndex].normalBitmap);
                DestroyIcon(imageButtons[btnIndex].hoverIcon);
                DestroyIcon(imageButtons[btnIndex].disabledIcon);
                DestroyIcon(imageButtons[btnIndex].normalIcon);
                
                //Move all the structures down one level by erasing
                //the current structure.
                for(unsigned int i = 0; i<imageButtons.size(); i++)
                {
                    if (i == (unsigned int)btnIndex)
                        imageButtons.erase( imageButtons.begin()+i );
                }
                
                SetWindowLong(hwnd, GWL_WNDPROC, (LONG)originalProc);
                RemoveProp(hwnd, ORIGINALPROCPROP);
                
                //If no imagebuttons is left, close the 
                //theme data and free library
                if (themesDll && imageButtons.empty())
                    FreeThemesDll();
            }
            break;
            
            case WM_MOUSEMOVE:
            {
                //Mouse is over the button, but it
                //hasn't yet captured it.
                if (GetCapture() != hwnd)
                {
                    imageButtons[btnIndex].isOver = TRUE;
                    SetCapture(hwnd);
                    
                    InvalidateRect(hwnd, NULL, FALSE);
                }
                else
                {			    
                    RECT rect; GetWindowRect(hwnd, &rect);
                    POINT pt; GetCursorPos(&pt);
                    
                    //Mouse is over the button?
                    if (!PtInRect(&rect, pt))
                    {
                        //No longer!
                        imageButtons[btnIndex].isOver = FALSE;
                        
                        if(!imageButtons[btnIndex].isPressed)
                        {
                            ReleaseCapture();
                            InvalidateRect(hwnd, NULL, TRUE);
                        }
                    }
                    else
                    {
                        //If i didn't release the captured mouse (because
                        //button is still pressed) "if (GetCapture() != hwnd)"
                        //code will never be executed, and isOver state won't
                        //change if the mouse is over or not. That's why i'm 
                        //using "else" opposite to "if (!PtInRect(&rect, pt))"
                        imageButtons[btnIndex].isOver = TRUE;
                        
                        //If you don't believe me, remove this "else" case 
                        //and try to click the button, move the mouse outside the
                        //button and move again the mouse over the button. 
                        //The button state won't switch to pressed state.
                        //Ok now mail me saying "i'm a dumb and you're the 
                        //mastah." ghghgh :)
                    }
                }
            }
            break;
        }
        
        return CallWindowProc(originalProc, hwnd, message, wParam, lParam);
    }
}

void ImageButton_EnableXPThemes()
{
    enableButtonTheme = TRUE;
}

BOOL ImageButton_Create(HWND button)
{
    if ( !button )
        return FALSE;
    
    // Check if button has BS_OWNERDRAW style.
    DWORD dwStyle = GetWindowLong(button, GWL_STYLE);
    if ((dwStyle & BS_OWNERDRAW) != BS_OWNERDRAW) return FALSE;
    
    // We only need one call of InitThemes and one call of OpenThemeData.
    if (!buttonTheme && enableButtonTheme)
    {
        // Ensure that Comctl32.dll is loaded correctly (and only one time).
        InitCommonControls();
        
        if (ib::InitThemesDll() == TRUE)
            buttonTheme = ib::pOpenThemeData(button, L"Button");
    }
    
    IMAGEBUTTON imageButton;
    imageButton.button = button;
    imageButton.cursor = LoadCursor(NULL, IDC_ARROW);
    
    imageButtons.push_back(imageButton);
    
    // Subclass the main window to handle WM_DRAWITEM message.
    HWND hwndParent = GetParent(button);
    if (NULL != hwndParent)
    {
        WNDPROC parentOrigProc = (WNDPROC) GetWindowLong(hwndParent, GWL_WNDPROC);
        if (parentOrigProc != ib::ButtonParentProc)
        {
            SetProp(hwndParent, ORIGINALPROCPROP, (HANDLE)parentOrigProc);
            SetWindowLong(hwndParent, GWL_WNDPROC, (LONG)ib::ButtonParentProc);
        }
    }
    
    // Subclass the button.
    WNDPROC buttonOrigProc = (WNDPROC)GetWindowLong(button, GWL_WNDPROC);
    SetProp(button, ORIGINALPROCPROP, (HANDLE)buttonOrigProc);
    SetWindowLong(button, GWL_WNDPROC, (LONG)ib::ButtonProc);
    
    // Set button font.
    HFONT defaultFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    SendMessage(button, WM_SETFONT, (WPARAM)defaultFont, MAKELPARAM(TRUE, 0));
    
    return TRUE;
}
BOOL ImageButton_Create(HWND hwndParent, UINT ctrlID)
{
    return ImageButton_Create(GetDlgItem(hwndParent, ctrlID));
}
void ImageButton_Create(int buttonCount, ...)
{
    va_list argList;
    va_start(argList, buttonCount);
    
    while(buttonCount--)
    {
        HWND button = va_arg(argList, HWND);
        ImageButton_Create(button);
    }
    
    va_end(argList);
}

BOOL ImageButton_SetBitmap(HWND button, HBITMAP normalBitmap, HBITMAP disabledBitmap, HBITMAP hoverBitmap)
{
    int i = ib::FindStructNumber(button);
    if (i == -1) return FALSE;
    
    //Normal bitmap
    if (normalBitmap == NULL)
    {
        DeleteObject(imageButtons[i].normalBitmap);
        imageButtons[i].normalBitmap = NULL;
    }
    else
        imageButtons[i].normalBitmap = normalBitmap;
    
    //Hover bitmap
    if (hoverBitmap == (HBITMAP)IBO_MAINTAIN)
    {
        if (!imageButtons[i].hoverBitmap)
            return FALSE;
    }
    else if (hoverBitmap == (HBITMAP)IBO_LIGHTEN)
    {
        HBITMAP newBmp = ib::ModifyBitmap(imageButtons[i].normalBitmap, &ib::LightenColor);
        if (!newBmp)
            return FALSE;
        
        if (imageButtons[i].hoverBitmap)
            DeleteObject(imageButtons[i].hoverBitmap);
        
        imageButtons[i].hoverBitmap = newBmp;
    }
    else if (hoverBitmap != NULL)
    {
        if(imageButtons[i].hoverBitmap)
            DeleteObject(imageButtons[i].hoverBitmap);
        
        imageButtons[i].hoverBitmap = hoverBitmap;
    }
    else
    {
        DeleteObject(imageButtons[i].hoverBitmap);
        imageButtons[i].hoverBitmap = NULL;
    }
    
    //Disabled bitmap
    if (disabledBitmap == (HBITMAP)IBO_MAINTAIN)
    {
        if (!imageButtons[i].disabledBitmap)
            return FALSE;
    }
    else if (disabledBitmap == (HBITMAP)IBO_GRAY)
    {
        HBITMAP newBmp = ib::ModifyBitmap(imageButtons[i].normalBitmap, &ib::GrayColor);
        if (!newBmp)
            return FALSE;
        
        if (imageButtons[i].disabledBitmap)
            DeleteObject(imageButtons[i].disabledBitmap);
        
        imageButtons[i].disabledBitmap = newBmp;
    }
    else if (disabledBitmap != NULL)
    {
        if(imageButtons[i].disabledBitmap)
            DeleteObject(imageButtons[i].disabledBitmap);
        
        imageButtons[i].disabledBitmap = disabledBitmap;
    }
    else
    {
        DeleteObject(imageButtons[i].disabledBitmap);
        imageButtons[i].disabledBitmap = NULL;
    }
    
	//Fill imageSize member
	BITMAP bm; GetObject(imageButtons[i].normalBitmap, sizeof(bm), &bm);
    imageButtons[i].imageSize.cx = bm.bmWidth;
    imageButtons[i].imageSize.cy = bm.bmHeight;
	
	imageButtons[i].isIcon = FALSE;
	
	// Redraw the button to make refresh immediate.
	InvalidateRect(button, NULL, TRUE);
    
    return TRUE;
}
BOOL ImageButton_SetBitmap(HWND button, HINSTANCE instance, UINT normalBitmapID, UINT disabledBitmapID, UINT hoverBitmapID)
{
    HBITMAP normalBitmap = NULL;
    if (normalBitmapID == (UINT)IBO_MAINTAIN)
        normalBitmap = (HBITMAP)normalBitmapID;
    else
        normalBitmap = (HBITMAP)LoadImage(instance, MAKEINTRESOURCE(normalBitmapID), IMAGE_BITMAP, 0,0,0);
    
    HBITMAP hoverBitmap = NULL;
    if (hoverBitmapID == (UINT)IBO_MAINTAIN || hoverBitmapID == (UINT)IBO_LIGHTEN)
        hoverBitmap = (HBITMAP)hoverBitmapID;
    else 
        hoverBitmap = (HBITMAP)LoadImage(instance, MAKEINTRESOURCE(hoverBitmapID), IMAGE_BITMAP, 0,0,0);
    
    HBITMAP disabledBitmap = NULL;
    if (disabledBitmapID == (UINT)IBO_MAINTAIN || disabledBitmapID == (UINT)IBO_GRAY)
        disabledBitmap = (HBITMAP)disabledBitmapID;
    else 
        disabledBitmap = (HBITMAP)LoadImage(instance, MAKEINTRESOURCE(disabledBitmapID), IMAGE_BITMAP, 0,0,0);
    
    int res = ImageButton_SetBitmap(button, normalBitmap, disabledBitmap, hoverBitmap);
    if (!res)
    {
        DeleteObject(normalBitmap);
        DeleteObject(hoverBitmap);
        DeleteObject(disabledBitmap);
    }
    
    return res;
}
BOOL ImageButton_SetBitmap(HWND button, UINT normalBitmapID, UINT disabledBitmapID, UINT hoverBitmapID)
{    
    return ImageButton_SetBitmap(button, GetModuleHandle(NULL), normalBitmapID, disabledBitmapID, hoverBitmapID);
}

BOOL ImageButton_SetIcon(HWND button, HICON normalIcon, HICON disabledIcon, HICON hoverIcon)
{    
    int i = ib::FindStructNumber(button);
    if (i == -1) return FALSE;
    
    //Normal icon
    if (normalIcon == NULL)
    {
        DestroyIcon(imageButtons[i].normalIcon);
        imageButtons[i].normalIcon = NULL;
    }
    else
        imageButtons[i].normalIcon = normalIcon;
    
    //Hover bitmap
    if (hoverIcon == (HICON)IBO_MAINTAIN)
    {
        if (!imageButtons[i].hoverIcon)
            return FALSE;
    }
    else if (hoverIcon == (HICON)IBO_LIGHTEN)
    {
        HICON newIcon = ib::ModifyIcon(imageButtons[i].normalIcon, &ib::LightenColor);
        if (!newIcon)
            return FALSE;
        
        if (imageButtons[i].hoverIcon)
            DeleteObject(imageButtons[i].hoverIcon);
        
        imageButtons[i].hoverIcon = newIcon;
    }
    else if (hoverIcon != NULL)
    {
        if(imageButtons[i].hoverIcon)
            DeleteObject(imageButtons[i].hoverIcon);
        
        imageButtons[i].hoverIcon = hoverIcon;
    }
    else
    {
        DestroyIcon(imageButtons[i].hoverIcon);
        imageButtons[i].hoverIcon = NULL;
    }
    
    //Disabled bitmap
    if (disabledIcon == (HICON)IBO_MAINTAIN)
    {
        if (!imageButtons[i].disabledIcon)
            return FALSE;
    }
    else if (disabledIcon == (HICON)IBO_GRAY)
    {
        HICON newIcon = ib::ModifyIcon(imageButtons[i].normalIcon, &ib::GrayColor);
        if (!newIcon)
            return FALSE;
        
        if (imageButtons[i].disabledIcon)
            DeleteObject(imageButtons[i].disabledIcon);
        
        imageButtons[i].disabledIcon = newIcon;
    }
    else if (disabledIcon != NULL)
    {
        if(imageButtons[i].disabledIcon)
            DeleteObject(imageButtons[i].disabledIcon);
        
        imageButtons[i].disabledIcon = disabledIcon;
    }
    else
    {
        DestroyIcon(imageButtons[i].disabledIcon);
        imageButtons[i].disabledIcon = NULL;
    }
    
    imageButtons[i].imageSize.cx = imageButtons[i].imageSize.cx == 0 ? 16 : imageButtons[i].imageSize.cx;
    imageButtons[i].imageSize.cy = imageButtons[i].imageSize.cy == 0 ? 16 : imageButtons[i].imageSize.cy;
	
	imageButtons[i].isIcon = TRUE;
	
	//Redraw the button to make refresh immediate.
	InvalidateRect(button, NULL, TRUE);
    
    return TRUE;
}
BOOL ImageButton_SetIcon(HWND button, HINSTANCE instance, UINT normalIconID, UINT disabledIconID, UINT hoverIconID, int iconWidth, int iconHeight)
{
    int i = ib::FindStructNumber(button);
    if (i == -1) return FALSE;
    
    HICON normalIcon = NULL;
    if (normalIconID == (UINT)IBO_MAINTAIN)
        normalIcon = (HICON)normalIconID;
    else
        normalIcon = (HICON)LoadImage(instance, MAKEINTRESOURCE(normalIconID), IMAGE_ICON, 0,0,0);
    
    HICON hoverIcon = NULL;
    if (hoverIconID == (UINT)IBO_MAINTAIN || hoverIconID == (UINT)IBO_LIGHTEN)
        hoverIcon = (HICON)hoverIconID;
    else 
        hoverIcon = (HICON)LoadImage(instance, MAKEINTRESOURCE(hoverIconID), IMAGE_ICON, 0,0,0);
    
    HICON disabledIcon = NULL;
    if (disabledIconID == (UINT)IBO_MAINTAIN || disabledIconID == (UINT)IBO_GRAY)
        disabledIcon = (HICON)disabledIconID;
    else 
        disabledIcon = (HICON)LoadImage(instance, MAKEINTRESOURCE(disabledIconID), IMAGE_ICON, 0,0,0);
    
    imageButtons[i].imageSize.cx = iconWidth;
    imageButtons[i].imageSize.cy = iconHeight;
    
    int res = ImageButton_SetIcon(button, normalIcon, disabledIcon, hoverIcon);
    if (!res)
    {
        DestroyIcon(normalIcon);
        DestroyIcon(disabledIcon);
        DestroyIcon(hoverIcon);
    }
    
    return res;
}
BOOL ImageButton_SetIcon(HWND button, UINT normalIconID, UINT disabledIconID, UINT hoverIconID, int iconWidth, int iconHeight)
{
    return ImageButton_SetIcon(button, GetModuleHandle(NULL), normalIconID, disabledIconID, hoverIconID, iconWidth, iconHeight);
}

BOOL ImageButton_SetCursor(HWND button, HCURSOR cursor)
{
    int i = ib::FindStructNumber(button);
    if (i == -1) return FALSE;
    
    if (!cursor)
        return FALSE;
    
    imageButtons[i].cursor = cursor;
    return TRUE;
}
BOOL ImageButton_SetCursor(HWND button, HINSTANCE instance, UINT cursorID)
{
    int i = ib::FindStructNumber(button);
    if (i == -1) return FALSE;
    
    HCURSOR cursor = (HCURSOR)LoadImage(instance, MAKEINTRESOURCE(cursorID), 
        IMAGE_CURSOR, 0,0, LR_DEFAULTSIZE);
    return ImageButton_SetCursor(button, cursor);
}

BOOL ImageButtons_SetCursor(HCURSOR cursor)
{
    if (cursor == NULL)
        return FALSE;
    
    for (unsigned int i = 0; i<imageButtons.size(); i++)
        imageButtons[i].cursor = cursor;
    
    return TRUE;
}
BOOL ImageButtons_SetCursor(HINSTANCE instance, UINT cursorID)
{    
    HCURSOR cursor = (HCURSOR)LoadImage(instance, MAKEINTRESOURCE(cursorID), 
        IMAGE_CURSOR, 0,0, LR_DEFAULTSIZE);
    return ImageButtons_SetCursor(cursor);
}

#ifdef __GNUC__
#pragma Warning(On, "*ISO C++ forbids casting between pointer-to-function and pointer-to-object*")
#endif
