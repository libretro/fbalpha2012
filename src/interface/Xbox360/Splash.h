
#include <xtl.h>
#include <xui.h>
#include <xuiapp.h>
 
#ifndef SPLASHSCENE_H
#define SPLASHSCENE_H
  
//--------------------------------------------------------------------------------------
// Scene implementation class.
//--------------------------------------------------------------------------------------
class CSplashScene : public CXuiSceneImpl
{

	CXuiControl m_Continue; 

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

	XUI_IMPLEMENT_CLASS( CSplashScene, L"SplashScene", XUI_CLASS_SCENE )
};


#endif