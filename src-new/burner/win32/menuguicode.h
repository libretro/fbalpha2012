//#define WINVER 0x0501

#include "burner.h"
#include <tchar.h>
#include <vector>
#include <string>

//Mingw headers' missing #defines
#ifndef ODS_HOTLIGHT
#define ODS_HOTLIGHT 0x0040
#endif
#ifndef DT_HIDEPREFIX
#define DT_HIDEPREFIX 0x00100000
#endif
#ifndef WM_UNINITMENUPOPUP
#define WM_UNINITMENUPOPUP 0x0125
#endif

#define ORIGINALPROCPROP        _T("ImageMenuOriginalProc")
#define MENUTITLEHEIGHT         16
#define COMMONTEXTFLAGS         DT_SINGLELINE|DT_VCENTER
#define MENUBARITEMTEXTFLAGS    COMMONTEXTFLAGS|DT_CENTER
#define MENUITEMTEXTFLAGS       COMMONTEXTFLAGS|DT_LEFT
#define MENUACCTEXTFLAGS        COMMONTEXTFLAGS|DT_RIGHT

#ifdef __GNUC__
#pragma GCC system_header
#pragma Warning(Off, "*ISO C++ forbids casting between pointer-to-function and pointer-to-object*")
#endif

typedef std::basic_string<TCHAR> tstring;
typedef BOOL (*PFNGRADIENTFILL) (HDC, PTRIVERTEX, ULONG, PVOID, ULONG, ULONG);
typedef struct tagIMAGEMENUITEM
{
    //General values to identify the structure
    HMENU itemMenu; //Menu where this item is
    HMENU itemSubMenu; //Submenu (should be != NULL only for menubar items and items with a submenu)
    HWND itemParentWnd; //When the window this HWND point to it's destroyed, i can delete the menu items that have the same HWND.
    int itemPos;
    UINT itemID;
    BOOL isMenuBarItem;
    BOOL isSeparator;
    
    //Image
    BOOL isImage;
    BOOL isIcon;
    HBITMAP normalBitmap;
    HICON normalIcon;
} IMAGEMENUITEM;
typedef struct tagMENUCOLOR
{
    COLORREF borderColor;
    COLORREF firstColor;
    COLORREF secondColor;
    BOOL isBorder;
    BOOL isGradient;
    BOOL isVertGradient;
} MENUCOLOR;
typedef struct tagMENUSTYLE
{
    MENUCOLOR menubarItem;              //Normal state (menubar item)
    MENUCOLOR menubarItemSelected;      //Selected state (menubar item)
    MENUCOLOR menubarItemPressed;       //Pressed state (menubar item)
    MENUCOLOR itemImgBk;                //Normal state (image bkgnd)
    MENUCOLOR itemBk;                   //Normal state (item bkgnd)
    MENUCOLOR itemSelected;             //Selected state (item)
    MENUCOLOR checkmarkBk;              //Normal state (checkmark bkgnd)
    MENUCOLOR checkmarkSelectedBk;      //Selected state (checkmark bkgnd)
    MENUCOLOR checkmarkDisabledBk;      //Disabled state (checkmark bkgnd)
    COLORREF checkmarkColor;            //Normal state (checkmark)
    COLORREF checkmarkSelColor;         //Selected state (checkmark)
    COLORREF checkmarkDisColor;         //Disabled state (checkmark)
    COLORREF menuBorderColor;           //Border color (menu). This should be the same color of the border in the "menubarItemPressed" structure.
    COLORREF textColor;                 //Normal color (text)
    COLORREF selectedTextColor;         //Selected color (text)
    COLORREF disabledTextColor;         //Disabled color (text)
    BOOL isMenuBarMergedWithSubmenu;    //Draw the menubar item and the submenu like one piece?
    BOOL isShorterSeparator;            //Draw a shorter separator (= no separator on image gradient)?
    BOOL isMenuBorder;                  //The menuBorderColor variable exist?
    BOOL isImage3dRect;                 //Draw a 3d rect around the image?
} MENUSTYLE;
enum drawSteps 
{ 
    MENUBARITEM_NORMAL, MENUBARITEM_HOVER, MENUBARITEM_PRESSED,
    ITEM_IMAGEBKGND, ITEM_BKGND, ITEM_SELECTED
};

std::vector<IMAGEMENUITEM>imageItems;
std::vector<IMMENUPROPS>menuProps;

//Options
int menuStyle = BASIC;

//Other global variables
int currentMenuBarItemWidth = 0; //Auto-explicative
int openMenus = 0; //Auto-explicative. When "openMenus == 1 && isMenuBarItemSubMenu" i can draw the white bar
HMENU currentMenu = NULL; //Currently displayed menu. This handle is obtained in the WM_INITMENUPOPUP message handler.
HMENU previousMenu = NULL; //Previously displayed menu.
WNDPROC originalMenuProc = NULL; //The original menu window proc, directly from the #32768 class.

//Add your own style!
MENUSTYLE styleProps[] = { 
    //Basic
    { 
        { 0, GetSysColor(COLOR_BTNFACE), 0, FALSE,FALSE,FALSE }, //menu bar item props
        { 0, GetSysColor(COLOR_HIGHLIGHT), 0, FALSE,FALSE,FALSE },
        { 0, GetSysColor(COLOR_HIGHLIGHT), 0, FALSE,FALSE,FALSE },
        { 0, GetSysColor(COLOR_MENU), 0, FALSE,FALSE,FALSE }, //normal item props
        { 0, GetSysColor(COLOR_MENU), 0, FALSE,FALSE,FALSE },
        { 0, GetSysColor(COLOR_HIGHLIGHT), 0, FALSE,FALSE,FALSE },
        { 0, GetSysColor(COLOR_BTNFACE), 0, FALSE,FALSE,FALSE }, //checkmark props
        { 0, GetSysColor(COLOR_BTNHIGHLIGHT), 0, FALSE,FALSE,FALSE },
        { 0, RGB(230,230,230), 0, FALSE,FALSE,FALSE },
        RGB(0,0,0), RGB(0,0,0), RGB(0,0,0), 
        RGB(192,192,192), //menu border prop
        RGB(0,0,0), RGB(255,255,255), RGB(192,192,192), //text props
        FALSE, FALSE, FALSE, FALSE
    },
    //Gray
    { 
        { 0, GetSysColor(COLOR_BTNFACE), 0, FALSE,FALSE,FALSE },
        { RGB(172,172,172), RGB(250,250,250), RGB(180,180,180), TRUE,TRUE,TRUE },
        { RGB(172,172,172), RGB(180,180,180), RGB(250,250,250), TRUE,TRUE,TRUE },
        { 0, RGB(240,240,240), RGB(200,200,200), FALSE,TRUE,TRUE }, //normal item props
        { 0, RGB(240,240,240), RGB(200,200,200), FALSE,TRUE,TRUE },
        { GetSysColor(COLOR_HIGHLIGHT), RGB(208,227,255), RGB(158,177,255), TRUE,TRUE,TRUE },
        { RGB(120,120,120), RGB(120,120,120), 0, TRUE,FALSE,FALSE }, //checkmark props
        { RGB(140,140,140), GetSysColor(COLOR_BTNHIGHLIGHT), 0, TRUE,FALSE,FALSE },
        { 0, RGB(230,230,230), 0, FALSE,FALSE,FALSE },
        RGB(255,255,255), RGB(0,0,0), RGB(0,0,0), 
        RGB(172,172,172), //menu border prop
        RGB(0,0,0), RGB(0,0,0), RGB(142,142,142), //text props
        TRUE, FALSE, TRUE, FALSE
    },
    //Office
    { 
        { 0, GetSysColor(COLOR_BTNFACE), 0, FALSE,FALSE,FALSE },
        { GetSysColor(COLOR_HIGHLIGHT), RGB(198,217,251), 0, TRUE,FALSE,TRUE },
        { RGB(100,100,100), RGB(255,255,255), 0, TRUE,FALSE,FALSE },
        { 0, GetSysColor(COLOR_BTNFACE), 0, FALSE,FALSE,FALSE }, //normal item props
        { 0, GetSysColor(COLOR_MENU), 0, FALSE,FALSE,FALSE },
        { GetSysColor(COLOR_HIGHLIGHT), RGB(198,217,251), 0, TRUE,FALSE,TRUE },
        { GetSysColor(COLOR_HIGHLIGHT), RGB(198,217,235), 0, TRUE,FALSE,FALSE }, //checkmark props
        { GetSysColor(COLOR_HIGHLIGHT), RGB(210,240,255), 0, TRUE,FALSE,FALSE },
        { 0, RGB(230,230,230), 0, FALSE,FALSE,FALSE },
        RGB(0,0,0), RGB(0,0,0), RGB(0,0,0), 
        RGB(100,100,100), //menu border prop
        RGB(0,0,0), RGB(0,0,0), RGB(192,192,192), //text props
        TRUE, TRUE, TRUE, FALSE
    },
    //Office 2003
    { 
        { 0, GetSysColor(COLOR_BTNFACE), 0, FALSE,FALSE,FALSE },
        { RGB(0,0,0), RGB(255,237,213), RGB(255,186,94), TRUE,TRUE,TRUE },
        { RGB(0,0,0), RGB(223,238,253), RGB(146,198,250), TRUE,TRUE,TRUE },
        { 0, RGB(223,238,253), RGB(146,198,250), FALSE,TRUE,FALSE }, //normal item props
        { 0, GetSysColor(COLOR_MENU), 0, FALSE,FALSE,FALSE },
        { RGB(0,0,150), RGB(255,237,213), RGB(255,186,94), TRUE,TRUE,TRUE },
        { 0, RGB(255,237,213), RGB(255,186,94), FALSE,TRUE,TRUE },  //checkmark props
        { RGB(0,0,0), RGB(146,198,250), RGB(223,238,253), TRUE,TRUE,FALSE },
        { 0, RGB(240,240,240), RGB(190,190,190), FALSE,TRUE,TRUE },
        RGB(0,0,0), RGB(0,0,0), RGB(0,0,0),  
        RGB(0,0,0), //menu border prop
        RGB(0,0,0), RGB(0,0,0), RGB(192,192,192), //text props
        TRUE, TRUE, TRUE, FALSE
    },
    // Office 2007
    {
        { 0, GetSysColor(COLOR_BTNFACE), 0, FALSE,FALSE,FALSE },
        { RGB(255,189,105), RGB(255,245,204), RGB(255,223,132),TRUE,TRUE,TRUE },
        { RGB(124,124,148), RGB(232,233,241), RGB(186,185,205),TRUE,TRUE,TRUE },
        { 0, RGB(249,249,255), RGB(159,157,185), FALSE,TRUE,FALSE }, //normal item props
        { 0, GetSysColor(COLOR_MENU), 0, FALSE,FALSE,FALSE },
        { RGB(255,189,105), RGB(255,238,194), RGB(255,238,194),TRUE,TRUE,TRUE },
        { RGB(255,171,63), RGB(255,192,111), RGB(255,192,111),FALSE,TRUE,TRUE },  //checkmark props
        { RGB(251,140,60), RGB(254,128,62), RGB(254,128,62), TRUE,TRUE,FALSE },
        { 0, RGB(240,240,240), RGB(190,190,190), FALSE,TRUE,TRUE },
        RGB(0,0,0), RGB(0,0,0), RGB(0,0,0),
        RGB(124,124,148), //menu border prop
        RGB(0,0,0), RGB(0,0,0), RGB(192,192,192), //text props
        TRUE, TRUE, TRUE, FALSE
    },
	// Visual Studio (by CaptainCPS-X)
	{
        { 0, GetSysColor(COLOR_BTNFACE), 0, FALSE,FALSE,FALSE },
        { RGB(255,189,105), RGB(255,245,204), RGB(255,223,132),TRUE,TRUE,TRUE },
        { RGB(124,124,148), RGB(232,233,241), RGB(186,185,205),TRUE,TRUE,TRUE },
        { 0, RGB(255,255,255), RGB(196,195,172), FALSE,TRUE,FALSE }, //...............normal item props (gray)
        { 0, RGB(240,240,240), RGB(200,200,200), FALSE,TRUE,FALSE },
		{ RGB(46,106,197), RGB(255,255,255), RGB(193,210,238),TRUE,TRUE,TRUE }, // Highlight props        
		{ GetSysColor(COLOR_HIGHLIGHT), RGB(198,217,235), 0, TRUE,FALSE,FALSE }, //checkmark props
        { GetSysColor(COLOR_HIGHLIGHT), RGB(210,240,255), 0, TRUE,FALSE,FALSE },
		{ 0, RGB(240,240,240), RGB(190,190,190), FALSE,TRUE,TRUE },
        RGB(0,0,0), RGB(0,0,0), RGB(0,0,0),
        RGB(0,0,0), //..........................................................menu border prop
        RGB(0,0,0), RGB(0,0,0), RGB(192,192,192), //..................................text props
        TRUE, TRUE, TRUE, FALSE
	}
};

int FindImageMenuItemStruct(UINT itemID)
{
    for (unsigned int i = 0; i<imageItems.size(); i++)
    {
        if (itemID == imageItems.at(i).itemID)
            return i;
    }
    return -1;
}
int FindMenuPropsStruct(HMENU thisMenu)
{
    for (unsigned int i = 0; i<menuProps.size(); i++)
    {
        if (thisMenu == menuProps.at(i).menuHandle)
            return i;
    }
    
    return -1;
}

//This function walk the chain of submenus and parent menus
//to find if the menu passed as parameter is a submenu
//of a menu bar item.
BOOL IsMenuBarItemSubMenu(HMENU menuToCheck)
{
    for(unsigned int i = 0; i<imageItems.size(); i++)
    {
        if (imageItems.at(i).itemSubMenu == menuToCheck)
        {
            if (imageItems.at(i).isMenuBarItem)
                return TRUE;
            else
                return IsMenuBarItemSubMenu(imageItems.at(i).itemMenu);
        }
    }
    
    return FALSE;
}

HFONT CreateBoldFont(HFONT startingFont)
{
    LOGFONT lf; 
    GetObject(startingFont, sizeof(lf), &lf);
    lf.lfWeight = FW_BOLD;
    
    DeleteObject(startingFont);
    
    return CreateFontIndirect(&lf);
}
HFONT CreateVerticalFont(HFONT startingFont)
{
    LOGFONT lf; 
    GetObject(startingFont, sizeof(lf), &lf);
    lf.lfEscapement = 900;
    lf.lfOrientation = 900;
    
    //Windows 98/me requires a TrueType (TT) 
    //font to create a vertical font.
    lf.lfOutPrecision = OUT_TT_ONLY_PRECIS; 
    
    DeleteObject(startingFont);
    
    return CreateFontIndirect(&lf);
}
tstring GetMenuItemText(const tstring& menuCaption)
{
    unsigned int tabPos = menuCaption.find_first_of( _T('\t') );
    if (tabPos != tstring::npos)
        return menuCaption.substr(0, tabPos);
    
    return menuCaption;
    
}
tstring GetMenuItemAcc(const tstring& menuCaption)
{
    unsigned int tabPos = menuCaption.find_last_of( _T('\t') );
    if (tabPos != tstring::npos)
        return menuCaption.substr(tabPos+1);
    
    return _T("");
}

//This function draw only the sides specified by the sides array.
void SideFrameRect(HDC hdc, LPRECT pRc, COLORREF borderColor, BOOL sides[])
{
    HPEN borderPen = CreatePen(PS_SOLID, 0, borderColor);
	HPEN oldPen = (HPEN)SelectObject(hdc, borderPen);
	
	//The rect is always too wide (?)
	pRc->right -= 1;
	
	if (sides[0]) //left
	{
        MoveToEx(hdc, pRc->left, pRc->top, NULL);
        LineTo(hdc, pRc->left, pRc->bottom);
	}
	if (sides[1]) //top
	{
	    MoveToEx(hdc, pRc->left, pRc->top, NULL);
        LineTo(hdc, pRc->right, pRc->top);
	}
	if (sides[2]) //right
	{
	    MoveToEx(hdc, pRc->right, pRc->top, NULL);
        LineTo(hdc, pRc->right, pRc->bottom);
	}
	if (sides[3]) //bottom
	{
	    MoveToEx(hdc, pRc->left, pRc->bottom, NULL);
        LineTo(hdc, pRc->right, pRc->bottom);
	}
	
	SelectObject(hdc, oldPen);
	DeleteObject(borderPen);
}
void GradientFillRect(HDC hdc, LPRECT rcGradient, COLORREF firstColor, COLORREF secondColor, BOOL isVertical = FALSE)
{
    //Manual GradientFill since the one in Windows 98/Me 
    //is buggy and make ImageMenu crash, and i should load
    //each time the GradientFill function on Windows XP.
    //Why bothering if this function works good as well?
    
    
    BYTE startRed = GetRValue(firstColor);
    BYTE startGreen = GetGValue(firstColor);
    BYTE startBlue = GetBValue(firstColor);
    
    BYTE endRed = GetRValue(secondColor);
    BYTE endGreen  = GetGValue(secondColor);
    BYTE endBlue = GetBValue(secondColor);
    
    HBRUSH endColor = CreateSolidBrush(secondColor);
    FillRect(hdc, rcGradient, endColor);
    DeleteObject(endColor);
    
    //Gradient line width/height
    int dy = 2;
    
    int length = (isVertical ? rcGradient->bottom - rcGradient->top 
        : rcGradient->right - rcGradient->left) - dy;
    
    for(int dn = 0; dn <= length; dn += dy)
    {
        BYTE currentRed = (BYTE)MulDiv(endRed-startRed, dn, length) + startRed;
        BYTE currentGreen = (BYTE)MulDiv(endGreen-startGreen, dn, length) + startGreen;
        BYTE currentBlue = (BYTE)MulDiv(endBlue-startBlue, dn, length) + startBlue;
        
        RECT currentRect = {0, 0, 0, 0}; 
        if(isVertical)
        {
            currentRect.left = rcGradient->left;
            currentRect.top = rcGradient->top + dn;
            currentRect.right = currentRect.left + rcGradient->right - rcGradient->left;
            currentRect.bottom = currentRect.top + dy;
        }
        else
        {
            currentRect.left = rcGradient->left + dn;
            currentRect.top = rcGradient->top;
            currentRect.right = currentRect.left + dy;
            currentRect.bottom = currentRect.top + rcGradient->bottom - rcGradient->top;
        }
        
        HBRUSH currentColor = CreateSolidBrush( RGB(currentRed,currentGreen,currentBlue) );
        FillRect(hdc, &currentRect, currentColor);
        DeleteObject(currentColor);
    }
}

void MenuStyle_DrawCheckBackground(HDC hdc, LPRECT rcDest, BOOL isSelected, BOOL isDisabled)
{
    MENUCOLOR *checkBkColor = NULL;
    if (isDisabled)
        checkBkColor = &styleProps[menuStyle].checkmarkDisabledBk;
    else if (isSelected)
        checkBkColor = &styleProps[menuStyle].checkmarkSelectedBk;
    else
        checkBkColor = &styleProps[menuStyle].checkmarkBk;
    
    //Set the color of the pen. The pen is used to 
    //draw the border of the round rect.
    HPEN borderPen = CreatePen (PS_SOLID, 0, checkBkColor->borderColor);
    HPEN oldPen = (HPEN)SelectObject(hdc, borderPen);
    RoundRect(hdc, rcDest->left, rcDest->top, rcDest->right, rcDest->bottom, 4,4);
    SelectObject(hdc, oldPen);
	DeleteObject(borderPen);
    
    //Draw the background inside the rect
    RECT rcCheckBK = *rcDest;
    InflateRect(&rcCheckBK, -1,-1);
    
    if (checkBkColor->isGradient)
        GradientFillRect(hdc, &rcCheckBK, checkBkColor->firstColor, checkBkColor->secondColor, 
            checkBkColor->isVertGradient);
    else
    {
        HBRUSH bkgndBrush = CreateSolidBrush(checkBkColor->firstColor);
        FillRect(hdc, &rcCheckBK, bkgndBrush);
        DeleteObject(bkgndBrush);
    }
}
void MenuStyle_DrawCheckMark(HDC hdc, LPRECT rcDest, BOOL isSelected, BOOL isDisabled)
{
    COLORREF checkmarkColor = 0;
    if (isDisabled)
        checkmarkColor = styleProps[menuStyle].checkmarkDisColor;
    else if (isSelected)
        checkmarkColor = styleProps[menuStyle].checkmarkSelColor;
    else
        checkmarkColor = styleProps[menuStyle].checkmarkColor;
    
    RECT rcCheckMark = *rcDest;
    InflateRect(&rcCheckMark, -1,-1);
    
    int x = (rcCheckMark.right-4)/2;
    int y = (rcCheckMark.bottom + rcCheckMark.top-5)/2;
    int dp = 0; //This variable makes the checkmark smaller if set to 1, if you want.
    
	HPEN checkmPen = CreatePen(PS_SOLID, 0, checkmarkColor);
	HPEN oldPen = (HPEN)SelectObject(hdc, checkmPen);
	
	MoveToEx(hdc, x, y+2, NULL);
	LineTo(hdc, x, y+5-dp);
	MoveToEx(hdc, x+1, y+3, NULL);
	LineTo(hdc, x+1, y+6-dp);
	MoveToEx(hdc, x+2, y+4, NULL);
	LineTo(hdc, x+2, y+7-dp);
	MoveToEx(hdc, x+3, y+3, NULL);
	LineTo(hdc, x+3, y+6-dp);
	MoveToEx(hdc, x+4, y+2, NULL);
	LineTo(hdc, x+4, y+5-dp);
	MoveToEx(hdc, x+5, y+1, NULL);
	LineTo(hdc, x+5, y+4-dp);
	MoveToEx(hdc, x+6, y, NULL);
	LineTo(hdc, x+6, y+3-dp);
	
	SelectObject(hdc, oldPen);
	DeleteObject(checkmPen);
}
void MenuStyle_DrawRadioCheckMark(HDC hdc, LPRECT rcDest, BOOL isSelected, BOOL isDisabled)
{
    COLORREF checkmarkColor = 0;
    if (isDisabled)
        checkmarkColor = styleProps[menuStyle].checkmarkDisColor;
    else if (isSelected)
        checkmarkColor = styleProps[menuStyle].checkmarkSelColor;
    else
        checkmarkColor = styleProps[menuStyle].checkmarkColor;
    
    //Border color...
	HPEN radioPen = CreatePen(PS_SOLID, 0, checkmarkColor);
	HPEN oldPen = (HPEN)SelectObject(hdc, radioPen);
	
	//...and background color (same color)
	HBRUSH radioBrush = CreateSolidBrush(checkmarkColor);
	HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, radioBrush);
	
	RECT rcCheckMark = *rcDest;
    InflateRect(&rcCheckMark, -6,-6);
	RoundRect(hdc, rcCheckMark.left, rcCheckMark.top, 
        rcCheckMark.right, rcCheckMark.bottom, 8,8);
	
	SelectObject(hdc, oldPen);
	SelectObject(hdc, oldBrush);
	DeleteObject(radioPen);
	DeleteObject(radioBrush);
}
void MenuStyle_DrawRect(HDC hdc, LPRECT pRc, int drawStep)
{
    MENUCOLOR currentDrawStyle; 
    BOOL isMenuBarItemAttached = FALSE;
    
    switch(drawStep)
    {
        case MENUBARITEM_NORMAL:
            currentDrawStyle = styleProps[menuStyle].menubarItem;
            break;
        case MENUBARITEM_HOVER:
            currentDrawStyle = styleProps[menuStyle].menubarItemSelected;
            pRc->top += styleProps[menuStyle].isMenuBarMergedWithSubmenu ? 1 : 0;
            break;
        case MENUBARITEM_PRESSED:
            currentDrawStyle = styleProps[menuStyle].menubarItemPressed;
            //Draw the three side border only menu bar item is pressed.
            isMenuBarItemAttached = styleProps[menuStyle].isMenuBarMergedWithSubmenu;
            //The menus with attached the submenus i've seen 
            //have a 1 pixel distance from the top of the window.
            pRc->top += isMenuBarItemAttached ? 1 : 0;
            break;
        case ITEM_IMAGEBKGND:
            currentDrawStyle = styleProps[menuStyle].itemImgBk;
            break;
        case ITEM_BKGND:
            currentDrawStyle = styleProps[menuStyle].itemBk;
            break;
        case ITEM_SELECTED:
            currentDrawStyle = styleProps[menuStyle].itemSelected;
            break;
    }
    
    if(currentDrawStyle.isGradient)
        GradientFillRect(hdc, pRc, currentDrawStyle.firstColor, currentDrawStyle.secondColor, currentDrawStyle.isVertGradient);
    else
    {
        HBRUSH rectBrush = CreateSolidBrush(currentDrawStyle.firstColor);
        FillRect(hdc, pRc, rectBrush);
        DeleteObject(rectBrush);
    }
    
    if (currentDrawStyle.isBorder)
    {
        if(isMenuBarItemAttached)
        {
            //Draw only three sides of the border: left-top-right
            BOOL sides[4] = {TRUE, TRUE, TRUE, FALSE};
            SideFrameRect(hdc, pRc, currentDrawStyle.borderColor, sides);
        }
        else
        {
            HBRUSH borderBrush = CreateSolidBrush(currentDrawStyle.borderColor);
            FrameRect(hdc, pRc, borderBrush);
            DeleteObject(borderBrush);
        }
    }
}
void MenuStyle_DrawTitle(HWND hwnd, HDC hdc)
{
    int mpIndex = FindMenuPropsStruct(currentMenu);
    if (mpIndex == -1) return;
    if (menuProps[mpIndex].menuTitle && 
        !_tcscmp(menuProps[mpIndex].menuTitle, _T(""))) return;
    
    //Get menu window dimension
    RECT windowRC; GetWindowRect(hwnd, &windowRC);
    RECT rcTitle;
    
    if (menuProps[mpIndex].IsVertTitle() )
        SetRect(&rcTitle, 2,2, MENUTITLEHEIGHT+2, windowRC.bottom-windowRC.top-2);
    else
        SetRect(&rcTitle, 2,2, windowRC.right-windowRC.left-2, MENUTITLEHEIGHT);
    
    //Fill the whole non client area for the title
    FillRect(hdc, &rcTitle, (HBRUSH)GetStockObject(WHITE_BRUSH));
    
    //Leave some space between the title and the items
    if (menuProps[mpIndex].IsVertTitle() )
        rcTitle.right -= 1;
    else
        rcTitle.bottom -= 1;
    
    //Fill the text background
    if (menuProps[mpIndex].IsGradient())
    {
        GradientFillRect(hdc, &rcTitle, menuProps[mpIndex].firstColor, menuProps[mpIndex].secondColor, 
            menuProps[mpIndex].IsVertGradient() );
    }
    else
    {
        COLORREF fillColor;
        
        if (menuProps[mpIndex].IsCustomBkColor() )
            fillColor = menuProps[mpIndex].firstColor;
        else //Fill the rect with default color
            fillColor = RGB(20,60,230);
        
        HBRUSH titleBkBrush = CreateSolidBrush(fillColor);
        FillRect(hdc, &rcTitle, titleBkBrush);
        DeleteObject(titleBkBrush);
    }
    
    HFONT normFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    DWORD drawtextFlags = 0;
    
    //Create a bold font
    HFONT titleFont = CreateBoldFont(normFont);
    
    //If the title is vertical, create a vertical font.
    if ( menuProps[mpIndex].IsVertTitle() )
    {
        drawtextFlags = DT_BOTTOM;
        titleFont = CreateVerticalFont(titleFont);
    }
    else
        drawtextFlags = DT_CENTER|DT_VCENTER;
    
    //Draw the text
    HFONT oldFont = (HFONT)SelectObject(hdc, titleFont);
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, menuProps[mpIndex].textColor);
    DrawText(hdc, menuProps[mpIndex].menuTitle, _tcslen(menuProps[mpIndex].menuTitle), &rcTitle, DT_SINGLELINE|drawtextFlags);
    
    //...
    SelectObject(hdc, oldFont);
    DeleteObject(titleFont);
}
void MenuStyle_DrawBorder(HWND hwnd, HDC hdc)
{
    //Get menu window dimension and normalize the resulting rect.
    RECT windowRC; GetWindowRect(hwnd, &windowRC);
    windowRC.right -= windowRC.left; windowRC.left = 0;
    windowRC.bottom -= windowRC.top; windowRC.top = 0;
    
    HBRUSH borderBrush = CreateSolidBrush(styleProps[menuStyle].menuBorderColor);
    FrameRect(hdc, &windowRC, borderBrush);
    DeleteObject(borderBrush);
    
    //When there's only the main menu (menuDisplayed == 1) and 
    //the menu currently open is a child of the main menu bar,
    //i draw a white bar whose width is the current menu bar item's width
    if (openMenus == 1
     && IsMenuBarItemSubMenu(currentMenu)
     && styleProps[menuStyle].isMenuBarMergedWithSubmenu
     && currentMenuBarItemWidth > 1)
    {
        RECT whiteBar = {1, 0, currentMenuBarItemWidth-1, 1};
        FillRect(hdc, &whiteBar, (HBRUSH)GetStockObject(WHITE_BRUSH));
    }
}

void ImageMenu_OnDrawItem(WPARAM /*wParam*/, LPARAM lParam, int i)
{
    LPDRAWITEMSTRUCT lpdi = (LPDRAWITEMSTRUCT)lParam;
    
    TCHAR menuItemText[256] = _T("");
    
    //Get menu text and flags
    MENUITEMINFO mii = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    mii.cbSize = sizeof(MENUITEMINFO);
    mii.fMask = MIIM_STRING|MIIM_FTYPE;
    mii.dwTypeData = menuItemText;
    mii.cch = 256;
    GetMenuItemInfo(imageItems.at(i).itemMenu, imageItems.at(i).itemID, MF_BYCOMMAND, &mii);
    
    tstring menuCaption = menuItemText;
    
    RECT rcText = lpdi->rcItem;
    RECT rcSelection = lpdi->rcItem;
    RECT rcGradient = lpdi->rcItem;
    RECT rcImage = lpdi->rcItem;
    
    SetBkMode(lpdi->hDC, TRANSPARENT);
    
    //Items from the menu bar (the bar always visible in a window menu)
    if (imageItems.at(i).isMenuBarItem)
    {
        //Menuitem is grayed
        if (lpdi->itemState & ODS_GRAYED)
        {
            MenuStyle_DrawRect(lpdi->hDC, &rcSelection, MENUBARITEM_NORMAL);
            
            //Draw the menu text with a white color and an offset by 1
            RECT rcOffSet = rcText;
            OffsetRect(&rcOffSet, 1,1);
            SetTextColor(lpdi->hDC, GetSysColor(COLOR_BTNHILIGHT));
            DrawText(lpdi->hDC, menuCaption.c_str(), menuCaption.size(), &rcOffSet, MENUBARITEMTEXTFLAGS);
            
            //Then, draw the same string over it.
            SetTextColor(lpdi->hDC, styleProps[menuStyle].disabledTextColor);
        }
        //Menu item is hovered
        else if (lpdi->itemState & ODS_HOTLIGHT) 
        {
            MenuStyle_DrawRect(lpdi->hDC, &rcSelection, MENUBARITEM_HOVER);
            SetTextColor(lpdi->hDC, styleProps[menuStyle].selectedTextColor);
        }
        //Menu item is selected
        else if (lpdi->itemState & ODS_SELECTED) 
        {
            currentMenuBarItemWidth = lpdi->rcItem.right - lpdi->rcItem.left;
            
            MenuStyle_DrawRect(lpdi->hDC, &rcSelection, MENUBARITEM_PRESSED);
            SetTextColor(lpdi->hDC, styleProps[menuStyle].selectedTextColor);
        }
        //Menuitem is in a normal state
        else
        {
            MenuStyle_DrawRect(lpdi->hDC, &rcSelection, MENUBARITEM_NORMAL);
            SetTextColor(lpdi->hDC, styleProps[menuStyle].textColor);
        }
        DrawText(lpdi->hDC, menuCaption.c_str(), menuCaption.size(), &rcText, MENUBARITEMTEXTFLAGS);
    }
    //Normal items
    else
    {
        rcImage.right = GetSystemMetrics(SM_CXSMICON)+4;
        rcGradient.right = rcImage.right+1;
        rcText.left += rcGradient.right+4;
        rcText.right -= 5;
        
        //Separator
        if (imageItems.at(i).isSeparator)
        {
            //If the normal image bkgnd has a vertical gradient 
            //(ex: the "gray" style), this gradient with the separator
            //looks orrible, so i don't draw it if it exists.
            if ( !styleProps[menuStyle].itemImgBk.isVertGradient )
                MenuStyle_DrawRect(lpdi->hDC, &rcGradient, ITEM_IMAGEBKGND);
            
            RECT rcSep = lpdi->rcItem;
            rcSep.top += 1;
            if (styleProps[menuStyle].isShorterSeparator)
                rcSep.left += rcGradient.right+2;
            
            DrawEdge(lpdi->hDC, &rcSep, EDGE_ETCHED, BF_TOP);
            
            //No need to draw anything else.
            return;
        }
        
        //Get text and accelerator
        tstring menuText = GetMenuItemText(menuCaption);
        tstring menuAcc = GetMenuItemAcc(menuCaption);
        
        //Menu item is grayed
        if (lpdi->itemState & ODS_GRAYED)
        {
            MenuStyle_DrawRect(lpdi->hDC, &rcSelection, ITEM_BKGND);
            MenuStyle_DrawRect(lpdi->hDC, &rcGradient, ITEM_IMAGEBKGND);
            
            //Draw a white item with a 1px offset
            RECT rcOffSet = rcText;
            OffsetRect(&rcOffSet, 1,1);
            SetTextColor(lpdi->hDC, GetSysColor(COLOR_BTNHILIGHT));
            
            DrawText(lpdi->hDC, menuText.c_str(), menuText.size(), &rcOffSet, MENUITEMTEXTFLAGS);
            DrawText(lpdi->hDC, menuAcc.c_str(), menuAcc.size(), &rcOffSet, MENUACCTEXTFLAGS);
            
            SetTextColor(lpdi->hDC, styleProps[menuStyle].disabledTextColor);
        }
        //Menu item is selected
        else if (lpdi->itemState & ODS_SELECTED) 
        {
            MenuStyle_DrawRect(lpdi->hDC, &rcSelection, ITEM_SELECTED);
            SetTextColor(lpdi->hDC, styleProps[menuStyle].selectedTextColor);
        }
        //Menuitem is in a normal state
        else
        {
            MenuStyle_DrawRect(lpdi->hDC, &rcSelection, ITEM_BKGND);
            MenuStyle_DrawRect(lpdi->hDC, &rcGradient, ITEM_IMAGEBKGND);
            SetTextColor(lpdi->hDC, styleProps[menuStyle].textColor);
        }
        
        //Draw text and accelerator
        DrawText(lpdi->hDC, menuText.c_str(), menuText.size(), &rcText, MENUITEMTEXTFLAGS);
        DrawText(lpdi->hDC, menuAcc.c_str(), menuAcc.size(), &rcText, MENUACCTEXTFLAGS);
        
        //Menu item is checked
        if (lpdi->itemState & ODS_CHECKED)
        {
            RECT rcCheck = rcImage;
            InflateRect(&rcCheck, -1,-1);
            rcCheck.right += 1;
            
            //Draw the check background
            MenuStyle_DrawCheckBackground(lpdi->hDC, &rcCheck, 
                lpdi->itemState & ODS_SELECTED, //Selected?
                lpdi->itemState & ODS_GRAYED); //Grayed?
            
            //Draw the checkmark only if the item doesn't have an image
            if (!imageItems.at(i).isImage)
            {
                //Is the item a radio item?
                if (mii.fType & MFT_RADIOCHECK)
                {
                    MenuStyle_DrawRadioCheckMark(lpdi->hDC, &rcCheck, 
                        lpdi->itemState & ODS_SELECTED, //Selected?
                        lpdi->itemState & ODS_GRAYED); //Grayed?
                }
                //No? Normal checkmark
                else
                {
                    MenuStyle_DrawCheckMark(lpdi->hDC, &rcCheck, 
                        lpdi->itemState & ODS_SELECTED, //Selected?
                        lpdi->itemState & ODS_GRAYED); //Grayed?
                }
                
                //I don't need to draw anything else.
                return;
            }
            
            //Else, keep drawing the image over the checkmark
        }
        
        //Images?
        if (imageItems.at(i).isImage)
        {
            rcImage.left += 3;
            rcImage.top += 2;
            
            //Icon?
            if (imageItems.at(i).isIcon && imageItems.at(i).normalIcon)
            {
                //Item is grayed?
                if (lpdi->itemState & ODS_GRAYED)
                {
                    //Draw a disabled icon
                    DrawState(lpdi->hDC, NULL, NULL, (LONG)imageItems.at(i).normalIcon, 0, rcImage.left, rcImage.top, 
                        16,16, DST_ICON|DSS_DISABLED);
                }
                else
                    DrawIconEx(lpdi->hDC, rcImage.left, rcImage.top, imageItems.at(i).normalIcon, 16,16, 0, NULL, DI_NORMAL);
            }
            //Bitmap?
            else if (imageItems.at(i).normalBitmap) 
            {
                //Item is grayed?
                if (lpdi->itemState & ODS_GRAYED)
                {
                    //Draw a disabled bitmap
                    DrawState(lpdi->hDC, NULL, NULL, (LONG)imageItems.at(i).normalBitmap, 0, rcImage.left, 
                        rcImage.top, 16, 16, DST_BITMAP | DSS_DISABLED);
                }
                else
                {
                    HDC memDC = CreateCompatibleDC(lpdi->hDC);
                    HBITMAP oldBmp = (HBITMAP)SelectObject(memDC, imageItems.at(i).normalBitmap);
                    
                    BitBlt(lpdi->hDC, rcImage.left, rcImage.top, 16,16, memDC, 0,0, SRCCOPY);
                    
                    SelectObject(memDC, oldBmp);
                    DeleteDC(memDC);
                }
            }
        }
    }
}
void ImageMenu_OnMeasureItem(WPARAM /*wParam*/, LPARAM lParam, int i)
{
    LPMEASUREITEMSTRUCT lpmi = (LPMEASUREITEMSTRUCT)lParam;
    
    //It's a separator?
    if (imageItems.at(i).isSeparator)
    {
        lpmi->itemWidth = 0;
        lpmi->itemHeight = 3;
        return;
    }
    
    //Get menu text
    TCHAR menuItemText[256];
    GetMenuString(imageItems.at(i).itemMenu, imageItems.at(i).itemID, menuItemText, 256, MF_BYCOMMAND);
    
    RECT captionSize = {0,0,1,1};
    HDC hdc = GetDC(NULL);
    HFONT oldFont = (HFONT)SelectObject(hdc, GetStockObject(DEFAULT_GUI_FONT));
    
    //GetTextExtentPoint32 compute the & character (and there's no flag
    //to avoid this), so the returned string width will be 1 char longer.
    //I have to use DrawText with DT_CALCRECT.
    DrawText(hdc, menuItemText, _tcslen(menuItemText), &captionSize, DT_CALCRECT|COMMONTEXTFLAGS);
    
    SelectObject(hdc, oldFont);
    ReleaseDC(NULL, hdc);
    
    //It's a menu bar item?
    if (imageItems.at(i).isMenuBarItem)
    {
        lpmi->itemWidth = captionSize.right+2;
        lpmi->itemHeight = captionSize.bottom+2;
    }
    //It's a normal item.
    else
    {
        //+20 or the menu window will end when the longer
        //word ends, and that doesn't really look good.
        lpmi->itemWidth = captionSize.right + GetSystemMetrics(SM_CXSMICON)+20;
        lpmi->itemHeight = GetSystemMetrics(SM_CYSMICON)+4;
    }
}

LRESULT CALLBACK ImageMenu_MenuWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
        //Taken from http://www.codeproject.com/cs/miscctrl/flatmenuform.asp?df=100&forumid=15651&select=935646#xx935646xx
        //This REDUCES/INCREASES the NON-CLIENT area by
        //increasing the window's width/height.
        case WM_WINDOWPOSCHANGING:
        {
            WINDOWPOS *pos = (WINDOWPOS*)lParam;
            if( !(pos->flags & SWP_NOSIZE) ) 
            {
                int mpIndex = FindMenuPropsStruct(currentMenu);
                
                //No title
                if (mpIndex == -1 || 
                    (menuProps[mpIndex].menuTitle && 
                     !_tcscmp(menuProps[mpIndex].menuTitle, _T(""))))
                {
                    pos->cx -= 2;
                    pos->cy -= 2;
                }
                else if ( menuProps[mpIndex].IsVertTitle() )
                {
                    pos->cx += MENUTITLEHEIGHT-2;
                    pos->cy -= 2;
                }
                else
                {
                    pos->cx -= 2;
                    pos->cy += MENUTITLEHEIGHT-4;
                }
            }
            
            if ( !(pos->flags & SWP_NOMOVE) )
            {
                //If the previous menu has a title (in the non-client area),
                //the submenu position is calculated using the client
                //area, and thus is displayed to the left or to the top
                //of its normal position. This restore its right position.
                int mpIndex = FindMenuPropsStruct(previousMenu);
                if (mpIndex != -1 && menuProps[mpIndex].menuTitle && 
                     _tcscmp(menuProps[mpIndex].menuTitle, _T("")) ) //Title not empty
                {
                    if ( menuProps[mpIndex].IsVertTitle() )
                        pos->x += MENUTITLEHEIGHT;
                    else
                        pos->y += MENUTITLEHEIGHT;
                }
            }
        }
        //Call default window proc with new values
        break;
        
        //This instead MOVES the CLIENT area...
        //(its name suggest something else, don't you think?)
        case WM_NCCALCSIZE:
        {
            NCCALCSIZE_PARAMS *nccs = (NCCALCSIZE_PARAMS*)lParam;
            nccs->rgrc->right += 2;
            nccs->rgrc->bottom += 2;
            
            int mpIndex = FindMenuPropsStruct(currentMenu);
            if (mpIndex == -1 || 
                (menuProps[mpIndex].menuTitle && 
                !_tcscmp(menuProps[mpIndex].menuTitle, _T(""))) )
            {
                nccs->rgrc->top -= 1;
                nccs->rgrc->left -= 1;
            }
            else if ( menuProps[mpIndex].IsVertTitle() )
            {
                nccs->rgrc->top -= 1;
                nccs->rgrc->left += MENUTITLEHEIGHT-1;
            }
            else
            {
                nccs->rgrc->top += MENUTITLEHEIGHT-3;
                nccs->rgrc->left -= 1;
            }
        }
        //Call default window proc with new values
        break;
        
        case WM_NCPAINT:
        {
            CallWindowProc(originalMenuProc, hwnd, msg, wParam, lParam);
            HDC menuDC = GetWindowDC(hwnd);
            
            //Draw the title (if any)
            MenuStyle_DrawTitle(hwnd, menuDC);
            
            //Draw the border
            if (styleProps[menuStyle].isMenuBorder)
                MenuStyle_DrawBorder(hwnd, menuDC);
            
            ReleaseDC(hwnd, menuDC);
        }
        return 0; 
        
        case WM_PRINT:
        {
            LRESULT res = CallWindowProc(originalMenuProc, hwnd, msg, wParam, lParam);        
            HDC menuDC = (HDC)wParam;
            
            //Draw the title
            MenuStyle_DrawTitle(hwnd, menuDC);
            
            //Draw the border
            if (styleProps[menuStyle].isMenuBorder)
                MenuStyle_DrawBorder(hwnd, menuDC);
            return res;
        }
    }
    
    return CallWindowProc(originalMenuProc, hwnd, msg, wParam, lParam);
}
LRESULT CALLBACK ImageMenu_ParentProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    WNDPROC originalProc = (WNDPROC)GetProp(hwnd, ORIGINALPROCPROP);
    static std::vector<HMENU> menuPopups;
    
    switch(msg)
    {
        case WM_INITMENUPOPUP:
        {
            HMENU thisMenu = (HMENU)wParam;
            
            menuPopups.push_back(thisMenu);
            
            //Global variables
            openMenus       += 1;
            currentMenu     = thisMenu;
            previousMenu    = (menuPopups.size() > 1 ? menuPopups.at( menuPopups.size()-2) : NULL);
        }
        break;
        
        case WM_UNINITMENUPOPUP:
        {
            //When TrackPopupMenu is called with TPM_NONOTIFY,
            //WM_INITMENUPOPUP is not called, but WM_UNINITMENUPOPUP 
            //is! So i need to check if along with the creation
            //of this menu, Windows sent a WM_INITMENUPOPUP.
            //This vector is also useful to get current and previous displayed menus.
            for (unsigned int i = 0; i<menuPopups.size(); i++)
            {
                if ( HMENU(wParam) == menuPopups.at(i) )
                {
                    //The previous menu is the last but one
                    //menu in the vector.
                    openMenus       -= 1;
                    currentMenu     = (i == 0 ? NULL : menuPopups.at(i-1));
                    previousMenu    = (i <= 1 ? NULL : menuPopups.at(i-2));
                    
                    menuPopups.erase( menuPopups.begin() +i );
                    
                    //Little hack: if a popupmenu from a main menu is destroyed, WM_UNINITMENUPOPUP
                    //is sent AFTER the parent menu (if it exists) is redrawn, so the border is drawn
                    //as the menu isn't a main menu cause menuDisplayed is not yet == 1. 
                    //(see DrawBorder() comments for more details)
                    HWND previousMenuHwnd = FindWindow(_T("#32768"), NULL);
                    RedrawWindow(previousMenuHwnd, NULL,NULL, RDW_ERASE|RDW_FRAME|RDW_INVALIDATE|RDW_UPDATENOW);
                    
                    break;
                }
            }
        }
        break;
        
        case WM_DRAWITEM:
        {
            int i = FindImageMenuItemStruct( ((LPDRAWITEMSTRUCT)lParam)->itemID );
            if (i == -1) break;
            
            ImageMenu_OnDrawItem(wParam, lParam, i);
        }
        break;
        
        case WM_MEASUREITEM:
        {
            int i = FindImageMenuItemStruct( ((LPMEASUREITEMSTRUCT)lParam)->itemID );
            if (i == -1) break;
            
            ImageMenu_OnMeasureItem(wParam, lParam, i);
        }
        break;
        
        case WM_DESTROY:
        {
            //In reverse order, i delete the menuitems that have "itemParentWnd == hwnd".
            for (int i = imageItems.size()-1; i>=0; i--)
            {
                if(imageItems.at(i).itemParentWnd == hwnd)
                {
                    DeleteObject(imageItems.at(i).normalBitmap);
                    DestroyIcon(imageItems.at(i).normalIcon);
                    
                    imageItems.erase(imageItems.begin() + i);
                }
            }
            
            //Empty the popups vector.
            menuPopups.clear();
            
            //Unsubclass main window
            SetWindowLong(hwnd, GWL_WNDPROC, (LONG)originalProc);
            RemoveProp(hwnd, ORIGINALPROCPROP);
        }
        break;
    }
    
    return CallWindowProc(originalProc, hwnd, msg, wParam, lParam);
}


BOOL ImageMenu_AddItem(HMENU itemMenu, HMENU itemSubMenu, HWND itemParentWnd, int itemPos, BOOL isMenuBarItem)
{
    if (!itemMenu) return FALSE;
    if (!itemParentWnd) return FALSE;
    
    //Does this item already exist?
    for (unsigned int i = 0; i<imageItems.size(); i++)
    {
        //Item found if it's in the same menu and has the same position
        if (imageItems.at(i).itemMenu == itemMenu && imageItems.at(i).itemPos == itemPos)
        {
            DeleteObject(imageItems.at(i).normalBitmap);
            DestroyIcon(imageItems.at(i).normalIcon);
            
            imageItems.erase( imageItems.begin()+i );
            break; //Stop searching
        }
    }
    
    MENUITEMINFO mii = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    mii.cbSize = sizeof(MENUITEMINFO);
    mii.fMask = MIIM_FTYPE|MIIM_ID;
    GetMenuItemInfo(itemMenu, itemPos, TRUE, &mii);
    
    mii.fMask = MIIM_FTYPE;
    mii.fType = MFT_OWNERDRAW | (mii.fType & MFT_SEPARATOR ? MFT_SEPARATOR : 0);
    SetMenuItemInfo(itemMenu, itemPos, TRUE, &mii);
    
    IMAGEMENUITEM imi = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    imi.itemMenu = itemMenu;
    imi.itemSubMenu = itemSubMenu;
    imi.itemParentWnd = itemParentWnd;
    imi.itemPos = itemPos;
    imi.itemID = mii.wID;
    imi.isMenuBarItem = isMenuBarItem;
    imi.isSeparator = (mii.fType & MFT_SEPARATOR);
    
    imageItems.push_back(imi);
    return TRUE;
}
BOOL ImageMenu_Fill(HWND hwnd, HMENU menuHandle, BOOL isMenuBar)
{
    int menuCount = GetMenuItemCount(menuHandle);
    if (menuCount == -1)
        return FALSE;
    
    for (int i = 0; i<menuCount; i++)
    {
        //Submenu? Then i must fill that too.
        HMENU subMenu = GetSubMenu(menuHandle, i);
        if (subMenu)
            ImageMenu_Fill(hwnd, subMenu, FALSE);
        
        ImageMenu_AddItem(menuHandle, subMenu, hwnd, i, isMenuBar);
    }
    
    return TRUE;
}

BOOL ImageMenu_Create(HWND hwnd, HMENU menuHandle, BOOL isMenuBar)
{
    //Superclass menu class
    if (originalMenuProc == NULL)
    {
        WNDCLASS wc = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        GetClassInfo(NULL, _T("#32768"), &wc);
        
        //Save original proc
        originalMenuProc = wc.lpfnWndProc;
        //Store our new window procedure
        wc.lpfnWndProc = (WNDPROC)ImageMenu_MenuWndProc;
        wc.hInstance = GetModuleHandle(NULL);
        //Make sure classes doesn't conflict
        wc.style &= ~CS_GLOBALCLASS;
        
        RegisterClass(&wc);
    }
    
    //Subclass the main window to handle WM_DRAWITEM & WM_MEASUREITEM
    WNDPROC originalProc = (WNDPROC)GetWindowLong(hwnd, GWL_WNDPROC);
    if (originalProc != ImageMenu_ParentProc)
    {
        //Windows ME(98?) with an UNICODE build returns a different
        //WNDPROC pointer even if i already subclassed the parent window,
        //(that is, originalProc and ImageMenu_ParentProc should point to
        //the same procedure but they don't). Trying to subclass it 
        //again using the old procedure (originalProc) makes imagemenu crash 
        //after i call SetWindowLong, probably because they ARE the same 
        //procedure even if their pointers doesn't coincide. And the subclass
        //procedure will end up being caught in an infinite loop.
        if ( !GetProp(hwnd, ORIGINALPROCPROP) )
            if ( SetProp(hwnd, ORIGINALPROCPROP, (HANDLE)originalProc) )
                SetWindowLong(hwnd, GWL_WNDPROC, (LONG)ImageMenu_ParentProc);
    }
    
    return ImageMenu_Fill(hwnd, menuHandle, isMenuBar);
}
BOOL ImageMenu_CreatePopup(HWND hwnd, HMENU menuHandle)
{
    //Popup menus do not have the menu bar
    return ImageMenu_Create(hwnd, menuHandle, FALSE);
}
void ImageMenu_Remove(HMENU menu)
{
    //Reverse order to maintain index
    for (unsigned int i=imageItems.size()-1; ; --i)
    {
        //When recursively deleting items, the index
        //we're currently inspecting can point to nothing.
        if (i >= imageItems.size())
            continue;
        
        if (imageItems.at(i).itemMenu == menu)
        {
            HMENU subMenu = imageItems.at(i).itemSubMenu;
            
            //Remove the props associated with this menu
            ImageMenu_RemoveMenuProps(imageItems.at(i).itemMenu);
            
            //Remove the images and the menu struct
            DeleteObject(imageItems.at(i).normalBitmap);
            DestroyIcon(imageItems.at(i).normalIcon);
            imageItems.erase(imageItems.begin() + i);
            
            //Delete submenu after parent menu
            //item has been deleted. 
            if (subMenu)
                ImageMenu_Remove(subMenu);
        }
    }
}

BOOL ImageMenu_SetItemImage(IMITEMIMAGE* imi)
{
    int i = FindImageMenuItemStruct(imi->itemID);
    if (i == -1) return FALSE;
    
    //Bitmap...
    if (imi->mask & IMIMF_BITMAP)
    {
        HBITMAP newBitmap = NULL;
        //...from resource
        if (imi->mask & IMIMF_LOADFROMRES)
            newBitmap = (HBITMAP)LoadImage(imi->hInst, imi->imageStr, IMAGE_BITMAP, 16,16,0);
         //...from file
        else if (imi->mask & IMIMF_LOADFROMFILE)
            newBitmap = (HBITMAP)LoadImage(NULL, imi->imageStr, IMAGE_BITMAP, 16,16, LR_LOADFROMFILE);
         //...from handle
        else
            newBitmap = imi->normalBitmap;
        
        if (newBitmap)
        {
            if (imageItems.at(i).normalBitmap)
                DeleteObject(imageItems.at(i).normalBitmap);
            imageItems.at(i).normalBitmap = newBitmap;
        }
        else return FALSE;
    }
    //Icon...
    else if (imi->mask & IMIMF_ICON)
    {
        HICON newIcon = NULL;
        //...from resource
        if (imi->mask & IMIMF_LOADFROMRES)
            newIcon = (HICON)LoadImage(imi->hInst, imi->imageStr, IMAGE_ICON, 16,16,0);
         //...from file
        else if (imi->mask & IMIMF_LOADFROMFILE)
            newIcon = (HICON)LoadImage(NULL, imi->imageStr, IMAGE_ICON, 16,16, LR_LOADFROMFILE);
         //...from handle
        else
            newIcon = imi->normalIcon;
        
        if (newIcon)
        {
            if (imageItems.at(i).normalIcon)
                DestroyIcon(imageItems.at(i).normalIcon);
            imageItems.at(i).normalIcon = newIcon;
        }
        else return FALSE;
    }
    else if (imi->mask & IMIMF_NOIMAGE)
    {
        DeleteObject(imageItems.at(i).normalBitmap);
        DestroyIcon(imageItems.at(i).normalIcon);
    }
    else return FALSE;
    
    if(imageItems.at(i).normalBitmap == NULL && imageItems.at(i).normalIcon == NULL)
        imageItems.at(i).isImage = FALSE;
    else
       imageItems.at(i).isImage = TRUE;
    
    if(imageItems.at(i).normalBitmap)
        imageItems.at(i).isIcon = FALSE;
    else if (imageItems.at(i).normalIcon)
        imageItems.at(i).isIcon = TRUE;
    
    return TRUE;
}
void ImageMenu_SetStyle(int newMenuStyle)
{
    menuStyle = newMenuStyle;
}

void ImageMenu_SetMenuProps(IMMENUPROPS *mp)
{
    int mpIndex = FindMenuPropsStruct(mp->menuHandle);
    
    //Doesn't exist, add it.
    if (mpIndex == -1)
        menuProps.push_back(*mp);
    //Else modify it
    else
    {
        menuProps[mpIndex].flags |= mp->flags;
        
        //Valid variables:
        // - menuTitle
        // - textColor
        if (mp->flags & IMPF_TITLE && mp->menuTitle)
        {
            _tcsncpy(menuProps[mpIndex].menuTitle, mp->menuTitle, 256);
            menuProps[mpIndex].textColor = mp->textColor;
        }
        
        //Valid variables:
        // - firstColor
        // - secondColor
        if (mp->flags & IMPF_BKGND)
        {
            menuProps[mpIndex].firstColor = mp->firstColor;
            menuProps[mpIndex].secondColor = mp->secondColor;
        }
    }
}
void ImageMenu_SetMenuTitleProps(HMENU menuHandle, LPTSTR title, BOOL isVerticalTitle, COLORREF textColor)
{
    IMMENUPROPS mp;
    mp.menuHandle = menuHandle;
    mp.flags = IMPF_TITLE | (isVerticalTitle ? IMPF_VERTICALTITLE : 0);
    mp.textColor = textColor;
    
    if (title)
        _tcsncpy(mp.menuTitle, title, 256);
    
    ImageMenu_SetMenuProps(&mp);
}
void ImageMenu_SetMenuTitleBkProps(HMENU menuHandle, COLORREF firstColor, COLORREF secondColor, BOOL isGradient, BOOL isVerticalGradient)
{
    IMMENUPROPS mp;
    mp.menuHandle = menuHandle;
    mp.flags = IMPF_BKGND| (isGradient ? (isVerticalGradient ? IMPF_VERTGRADIENT : IMPF_HORZGRADIENT) : 0);
    mp.firstColor = firstColor;
    mp.secondColor = secondColor;
    
    ImageMenu_SetMenuProps(&mp);
}
void ImageMenu_RemoveMenuProps(HMENU menuHandle)
{
    int mpIndex = FindMenuPropsStruct(menuHandle);
    if(mpIndex != -1)
        menuProps.erase(menuProps.begin() + mpIndex);
}

#ifdef __GNUC__
#pragma Warning(On, "*ISO C++ forbids casting between pointer-to-function and pointer-to-object*")
#endif
