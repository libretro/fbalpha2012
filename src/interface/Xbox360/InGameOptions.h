//--------------------------------------------------------------------------------------
// XuiTutorial.cpp
//
// Shows how to display and use a simple XUI scene.
//
// Xbox Advanced Technology Group.
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#include <xtl.h>
#include <xui.h>
#include <xuiapp.h>
#include <algorithm>
#include <new>
#include <iostream>
#include <vector>


#ifndef INGAMEOPTIONS_H
#define INGAMEOPTIONS_H
 

#define XM_MESSAGE_ON_INGAME_MENU		XM_USER
#define XM_MESSAGE_ON_DIPSWITCH_MENU	XM_USER+1
#define XM_MESSAGE_ON_INPUT_MENU		XM_USER+2

typedef struct
{
    int iVal1;    
}
InGameMenuStruct;

// Information for one list item.
typedef struct _LIST_ITEM_INFO {
    LPCWSTR pwszName;
    LPCWSTR pwszValue;
} LIST_ITEM_INFO;

// List data.
typedef struct _tagListData {
    int nItems;
    LIST_ITEM_INFO *pItems;
} LIST_DATA;

void InGameMenuFirstFunc(XUIMessage *pMsg, InGameMenuStruct* pData, int iVal1);

// Define the message map macro
#define XUI_ON_XM_MESSAGE_ON_INGAME_MENU(MemberFunc)\
    if (pMessage->dwMessage == XM_MESSAGE_ON_INGAME_MENU)\
    {\
        InGameMenuStruct *pData = (InGameMenuStruct *) pMessage->pvData;\
        return MemberFunc(pData->iVal1,  pMessage->bHandled);\
    }

//--------------------------------------------------------------------------------------
// Scene implementation class.
//--------------------------------------------------------------------------------------
class CInGameOptions : public CXuiSceneImpl
{

protected:

    // Control and Element wrapper objects.
    CXuiControl m_SaveState;
    CXuiControl m_LoadState;

	CXuiControl m_BackToGame;

	CXuiControl m_ExitGame;
	CXuiControl m_TakePreview;
	CXuiControl m_TakeTitle;
	CXuiControl m_Reset;


	CXuiControl m_FilterOptions;
	CXuiControl m_SetDipSwitches;
	CXuiControl m_SetInputOptions;
	CXuiControl m_OtherOptions;

	CXuiControl m_AdjustScreen;
 
	CXuiImageElement m_PreviewImage;
	CXuiImageElement m_PreviewSmallImage;

    // Message map.
    XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT( OnInit )
		XUI_ON_XM_NOTIFY_PRESS( OnNotifyPress )	
		XUI_ON_XM_MESSAGE_ON_INGAME_MENU( OnInGameMenu )
    XUI_END_MSG_MAP()

	


 
public:
    HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled );
	HRESULT OnNotifyPress( HXUIOBJ hObjPressed, BOOL& bHandled );
	HRESULT OnInGameMenu ( int iVal1, BOOL& bHandled );

	 
public:

    // Define the class. The class name must match the ClassOverride property
    // set for the scene in the UI Authoring tool.
    XUI_IMPLEMENT_CLASS( CInGameOptions, L"InGameOptions", XUI_CLASS_SCENE )
};

class CFilterOptions : public CXuiSceneImpl
{

protected:

	CXuiRadioGroup m_FilterGroup;
	CXuiControl m_BackToMain;
	CXuiControl m_BackToGame;
	CXuiCheckbox m_AspectRatio;
	CXuiCheckbox m_PointFiltering;

	    // Message map.
    XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT( OnInit )
		XUI_ON_XM_NOTIFY_PRESS( OnNotifyPress )			 
    XUI_END_MSG_MAP()
public:
    HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled );
	HRESULT OnNotifyPress( HXUIOBJ hObjPressed, BOOL& bHandled );
	HRESULT OnInGameMenu ( int iVal1, BOOL& bHandled );

public:

    // Define the class. The class name must match the ClassOverride property
    // set for the scene in the UI Authoring tool.
    XUI_IMPLEMENT_CLASS( CFilterOptions, L"FilterOptions", XUI_CLASS_SCENE )

};

class CDipOptions : public CXuiSceneImpl
{

protected:	 
	CXuiControl m_BackToMain;
	CXuiControl m_BackToGame;
	CXuiControl m_ResetDips;
	CXuiList	m_DipList;
	CXuiList	m_DipSettingsList;
    CXuiImageElement m_PreviewSmallImage;

	    // Message map.
    XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT( OnInit )
		XUI_ON_XM_NOTIFY_PRESS( OnNotifyPress )			 
    XUI_END_MSG_MAP()
public:
    HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled );
	HRESULT OnNotifyPress( HXUIOBJ hObjPressed, BOOL& bHandled );	
public:

    // Define the class. The class name must match the ClassOverride property
    // set for the scene in the UI Authoring tool.
    XUI_IMPLEMENT_CLASS( CDipOptions, L"DipOptions", XUI_CLASS_SCENE )

};


class CDipList : CXuiListImpl
{

public:

	std::map<std::string,std::string> m_DipList;

	XUI_IMPLEMENT_CLASS(CDipList, L"DipList", XUI_CLASS_LIST);

	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT(OnInit)
		XUI_ON_XM_GET_SOURCE_TEXT(OnGetSourceDataText)
		XUI_ON_XM_GET_ITEMCOUNT_ALL(OnGetItemCountAll)	 	 
	XUI_END_MSG_MAP()

	CDipList();
 
    HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled );	
	HRESULT OnGetSourceDataText(XUIMessageGetSourceText *pGetSourceTextData, BOOL& bHandled);
	HRESULT OnGetItemCountAll(XUIMessageGetItemCount *pGetItemCountData, BOOL& bHandled);	 
	 
};


class CDipSettingsList : CXuiListImpl
{

public:

	std::map<std::string,std::string> m_DipSettingsList;

	XUI_IMPLEMENT_CLASS(CDipSettingsList, L"DipSettingsList", XUI_CLASS_LIST);

	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT(OnInit)
		XUI_ON_XM_GET_SOURCE_TEXT(OnGetSourceDataText)
		XUI_ON_XM_GET_ITEMCOUNT_ALL(OnGetItemCountAll)	 	 
	XUI_END_MSG_MAP()

	CDipSettingsList();
 
    HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled );	
	HRESULT OnGetSourceDataText(XUIMessageGetSourceText *pGetSourceTextData, BOOL& bHandled);
	HRESULT OnGetItemCountAll(XUIMessageGetItemCount *pGetItemCountData, BOOL& bHandled);	 
	 
};


class CInputOptions : public CXuiSceneImpl
{

protected:	 
	CXuiControl m_BackToMain;
	CXuiControl m_BackToGame;
	CXuiList	m_InputList;
	CXuiList	m_InputSettingsList;
    CXuiImageElement m_PreviewSmallImage;
	CXuiControl m_ResetInputs;
	CXuiControl m_SavePreset;
 
	    // Message map.
    XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT( OnInit )
		XUI_ON_XM_NOTIFY_PRESS( OnNotifyPress )			 
    XUI_END_MSG_MAP()
public:
    HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled );
	HRESULT OnNotifyPress( HXUIOBJ hObjPressed, BOOL& bHandled );	
public:

    // Define the class. The class name must match the ClassOverride property
    // set for the scene in the UI Authoring tool.
    XUI_IMPLEMENT_CLASS( CInputOptions, L"InputOptions", XUI_CLASS_SCENE )

};


class CInputList : CXuiListImpl
{

public:

	std::map<std::string,std::string> m_InputList;

	XUI_IMPLEMENT_CLASS(CInputList, L"InputList", XUI_CLASS_LIST);

	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT(OnInit)
		XUI_ON_XM_GET_SOURCE_TEXT(OnGetSourceDataText)
		XUI_ON_XM_GET_ITEMCOUNT_ALL(OnGetItemCountAll)	 	 
	XUI_END_MSG_MAP()

	CInputList();
 
    HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled );	
	HRESULT OnGetSourceDataText(XUIMessageGetSourceText *pGetSourceTextData, BOOL& bHandled);
	HRESULT OnGetItemCountAll(XUIMessageGetItemCount *pGetItemCountData, BOOL& bHandled);	 
	 
};


class CInputSettingsList : CXuiListImpl
{

public:

	std::map<int,std::string> m_InputSettingsList;

	XUI_IMPLEMENT_CLASS(CInputSettingsList, L"InputSettingsList", XUI_CLASS_LIST);

	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT(OnInit)
		XUI_ON_XM_GET_SOURCE_TEXT(OnGetSourceDataText)
		XUI_ON_XM_GET_ITEMCOUNT_ALL(OnGetItemCountAll)	 	 
	XUI_END_MSG_MAP()

	CInputSettingsList();
 
    HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled );	
	HRESULT OnGetSourceDataText(XUIMessageGetSourceText *pGetSourceTextData, BOOL& bHandled);
	HRESULT OnGetItemCountAll(XUIMessageGetItemCount *pGetItemCountData, BOOL& bHandled);	 
	 
};
 
class COtherOptions : public CXuiSceneImpl
{

protected:	 
	CXuiControl m_BackToMain;
	CXuiControl m_BackToGame;
    CXuiImageElement m_PreviewSmallImage;
	CXuiSlider m_68000Speed;
	CXuiSlider m_AnalogSensitivity;
	CXuiTextElement m_68000SpeedText;
	CXuiTextElement m_AnalogLabel;
	
 
	    // Message map.
    XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT( OnInit )
		XUI_ON_XM_NOTIFY_PRESS( OnNotifyPress )			 
		XUI_ON_XM_NOTIFY_VALUE_CHANGED( OnNotifyValueChanged )
    XUI_END_MSG_MAP()
public:
    HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled );
	HRESULT OnNotifyPress( HXUIOBJ hObjPressed, BOOL& bHandled );
	HRESULT OnNotifyValueChanged(HXUIOBJ hObjSource, XUINotifyValueChanged *pNotifyValueChangedData, BOOL& bHandled);
public:

    // Define the class. The class name must match the ClassOverride property
    // set for the scene in the UI Authoring tool.
    XUI_IMPLEMENT_CLASS( COtherOptions, L"OtherOptions", XUI_CLASS_SCENE )

};
#endif