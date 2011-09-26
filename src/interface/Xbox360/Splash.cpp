

#include "burner.h"
#include "splash.h"


extern BOOL IsCurrentlyInGame;

HRESULT CSplashScene::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{
 
	GetChildById( L"XuiButtonContinue", &m_Continue);
 
	return S_OK;
}


HRESULT CSplashScene::OnNotifyPress( HXUIOBJ hObjPressed, BOOL& bHandled )
{
		 
	 
		if ( hObjPressed == m_Continue)
        {
			 
			IsCurrentlyInGame = true;
			bHandled = TRUE;
			return S_OK;
			
        }
	 
 
}