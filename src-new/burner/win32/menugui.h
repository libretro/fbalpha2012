//******************************************
// Author:          Massimo Galbusera
// Email:           kkez@winapizone.net
// Website:         http://www.winapizone.net
// File:            ImageMenu.cpp
// Version:         1.1
// Created on:      7 February 2006
// Last updated on: 2 October 2006
// Compiled on:     MinGW
// Compatible with: Window 98, ME, 2000, XP and 2003
//
//-------------------------------------------------------------
// I don't have Visual c++ so i can't test this with that 
// compiler. If you managed to compile it with it, please let me
// know about it so i can add it to the supported compilers.
//-------------------------------------------------------------
//
// You are free to use/modify this code but leave this header intact.
// This file is public domain so you are free to use it any of
// your applications (Freeware, Shareware, Commercial). All I ask:
// please let me know you're using this extension, so i can
// add a link to your program on my website.
//******************************************

enum menuStyle {
    BASIC,
    GRAY,
    OFFICE,
    OFFICE2003,
    OFFICE2007,
	CAPXVSTUDIO // by CaptainCPS-X
};
enum imItemImageFlags { 
    IMIMF_LOADFROMFILE = 1, 
    IMIMF_LOADFROMRES = 2, 
    IMIMF_ICON = 4, 
    IMIMF_BITMAP = 8, 
    IMIMF_NOIMAGE = 16
};
enum imPropsFlags {
    IMPF_TITLE = 1,
    IMPF_VERTICALTITLE = 2,
    IMPF_BKGND = 4,
    IMPF_CUSTOMBKCOLOR = 8,
    IMPF_HORZGRADIENT = 16,
    IMPF_VERTGRADIENT = 32
};

typedef struct tagIMMENUPROPS
{
    tagIMMENUPROPS() : menuHandle(NULL), flags(0), textColor(RGB(255,255,255)),
        firstColor(0), secondColor(0) { }
    
    HMENU menuHandle;
    DWORD flags;
    //Title text props
    TCHAR menuTitle[256];
    COLORREF textColor;
    //Title background colors
    COLORREF firstColor;
    COLORREF secondColor;
    
    BOOL IsGradient() { return flags&IMPF_HORZGRADIENT || flags&IMPF_VERTGRADIENT; }
    BOOL IsVertGradient() { return flags&IMPF_VERTGRADIENT; }
    BOOL IsVertTitle() { return flags&IMPF_VERTICALTITLE; }
    BOOL IsCustomBkColor() { return !IsGradient() && flags&IMPF_CUSTOMBKCOLOR; }
} IMMENUPROPS;
typedef struct tagIMITEMIMAGE
{
    tagIMITEMIMAGE() : mask(0), itemID(0), imageStr(NULL), hInst(NULL), normalIcon(NULL), normalBitmap(NULL) { }
    
    UINT mask;
    UINT itemID;
    LPTSTR imageStr;
    HINSTANCE hInst;
    HICON normalIcon;
    HBITMAP normalBitmap;
} IMITEMIMAGE;

BOOL ImageMenu_AddItem(HMENU itemMenu, HMENU itemSubMenu, HWND itemParentWnd, int itemPos, BOOL isMenuBarItem);
BOOL ImageMenu_Fill(HWND hwnd, HMENU menuHandle, BOOL isMenuBar);

BOOL ImageMenu_Create(HWND hwnd, HMENU hMenu, BOOL isMenuBar = TRUE);
BOOL ImageMenu_CreatePopup(HWND hwnd, HMENU hMenu);
void ImageMenu_Remove(HMENU menu);

BOOL ImageMenu_SetItemImage(IMITEMIMAGE* imi);
void ImageMenu_SetStyle(int newMenuStyle);

void ImageMenu_SetMenuProps(IMMENUPROPS *mp);
void ImageMenu_SetMenuTitleProps(HMENU menuHandle, LPTSTR title, BOOL isVerticalTitle, COLORREF textColor = RGB(255,255,255));
void ImageMenu_SetMenuTitleBkProps(HMENU menuHandle, COLORREF firstColor, COLORREF secondColor, BOOL isGradient, BOOL isVerticalGradient);
void ImageMenu_RemoveMenuProps(HMENU menuHandle);
