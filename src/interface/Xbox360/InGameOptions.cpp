#include <xtl.h>
#include "burner.h"
#include "vid_filter.h"
#include "InGameOptions.h"
#include "RomList.h"
#include "inp_keys.h"

extern int bRunPause;

bool DoReset = false;
bool exitGame;
extern HXUIOBJ hRomListScene;
extern HXUIOBJ phObj;
HXUIOBJ hFilterOptionsScene;
HXUIOBJ hDipOptionsScene;
HXUIOBJ hOtherOptionsScene;
HXUIOBJ hInputOptionsScene;

#define KEY(x) { pgi->nInput = GIT_SWITCH; pgi->Input.Switch = (unsigned short)(x); }
#define MACRO(x) { pgi->Macro.nMode = 1; pgi->Macro.Switch = (unsigned short)(x); }

HWND hInpDIPSWDlg = NULL; // Handle to the DIPSW Dialog
static HWND hInpDIPSWList = NULL;

static unsigned char nPrevDIPSettings[4];
static unsigned int nDIPGroup;
static unsigned int nDIPSel;
static unsigned int nInpSel;
static int nDIPOffset;

extern void setPauseMode(bool bPause);
extern void setPauseModeScreen(bool bPause);
extern int BurnStateSave(TCHAR* szName, int bAll);
extern int BurnStateLoad(TCHAR* szName, int bAll, int (*pLoadGame)());
extern void simpleReinitScrn(const bool& reinitVid);
extern LPCWSTR MultiCharToUniChar(char* mbString);
extern unsigned int vidFilterLinear;
// Handler for the XM_NOTIFY message

std::vector<std::string> m_DipListData;
std::vector<std::string> m_DipListValues;
std::vector<int> m_DipListOffsets;

std::vector<std::string> m_InputListData;
std::vector<std::string> m_InputListValues;
std::vector<int> m_InputListOffsets;

std::vector<std::string> m_InputSettingsData;
std::vector<std::string> m_InputSettingsValues;
std::vector<int> m_InputSettingsOffsets;

wchar_t DipSetting[64];
wchar_t InpSetting[64];
wchar_t InpListSetting[64];
wchar_t SpeedAdjustString[16];
wchar_t AnalogAdjustString[16];
 
int bAdjustScreen;

static void InpDIPSWGetOffset()
{
	BurnDIPInfo bdi;
	nDIPOffset = 0;
	for (int i = 0; BurnDrvGetDIPInfo(&bdi, i) == 0; i++) {
		if (bdi.nFlags == 0xF0) {
			nDIPOffset = bdi.nInput;
			break;
		}
	}
}

void InpDIPSWResetDIPs()
{
	int i = 0;
	BurnDIPInfo bdi;
	struct GameInp* pgi = NULL;

	InpDIPSWGetOffset();

	while (BurnDrvGetDIPInfo(&bdi, i) == 0) {
		if (bdi.nFlags == 0xFF) {
			pgi = GameInp + bdi.nInput + nDIPOffset;
			if (pgi) {
				pgi->Input.Constant = (pgi->Input.Constant & ~bdi.nMask) | (bdi.nSetting & bdi.nMask);
			}
		}
		i++;
	}
}


static bool CheckSetting(int i)
{
	BurnDIPInfo bdi;
	BurnDrvGetDIPInfo(&bdi, i);
	struct GameInp* pgi = GameInp + bdi.nInput + nDIPOffset;

	if (!pgi) {
		return false;
	}

	if ((pgi->Input.Constant & bdi.nMask) == bdi.nSetting) {
		unsigned char nFlags = bdi.nFlags;
		if ((nFlags & 0x0F) <= 1) {
			return true;
		} else {
			for (int j = 1; j < (nFlags & 0x0F); j++) {
				BurnDrvGetDIPInfo(&bdi, i + j);
				pgi = GameInp + bdi.nInput + nDIPOffset;
				if (nFlags & 0x80) {
					if ((pgi->Input.Constant & bdi.nMask) == bdi.nSetting) {
						return false;
					}
				} else {
					if ((pgi->Input.Constant & bdi.nMask) != bdi.nSetting) {
						return false;
					}
				}
			}
			return true;
		}
	}
	return false;
}

 

 

static int InpDIPSWExit()
{
	hInpDIPSWList = NULL;
	hInpDIPSWDlg = NULL;

	if (!bAltPause && bRunPause) {
		bRunPause = 0;
	}
	GameInpCheckMouse();
	return 0;
}

static void InpDIPSWCancel()
{
	int i = 0, j = 0;
	BurnDIPInfo bdi;
	struct GameInp* pgi = NULL;

	while (BurnDrvGetDIPInfo(&bdi, i) == 0) {
		if (bdi.nInput >= 0 && bdi.nFlags == 0xFF) {
			pgi = GameInp + bdi.nInput + nDIPOffset;
			if (pgi) {
				pgi->Input.Constant = nPrevDIPSettings[j];
				j++;
			}
		}
		i++;
	}
}
 

HRESULT CInGameOptions::OnNotifyPress( HXUIOBJ hObjPressed, 
       BOOL& bHandled )
    {
		 
	 
		if ( hObjPressed == m_SaveState)
        {
			const WCHAR * button_text = L"OK";
			 
			TCHAR romName[50];
			
			_stprintf(romName, _T("GAME:\\savestates\\%S.fs"), BurnDrvGetText(DRV_NAME));
		   
			if (StatedSave(0)==0)	 
				ShowMessageBoxEx(NULL,NULL,L"FBANext - Save State", L"Saved state successfully.", 1, (LPCWSTR*)&button_text,NULL,  XUI_MB_CENTER_ON_PARENT, NULL); 
			else
				ShowMessageBoxEx(NULL,NULL,L"FBANext - Save State", L"Error saving savestate file.", 1, (LPCWSTR*)&button_text,NULL,  XUI_MB_CENTER_ON_PARENT, NULL); 
			 

			bHandled = TRUE;
			return S_OK;
			
        }
		else if (hObjPressed == m_LoadState)
		{
			int len = 0;

			const WCHAR * button_text = L"OK";

			TCHAR romName[50];
			
			_stprintf(romName, _T("GAME:\\savestates\\%S.fs"), BurnDrvGetText(DRV_NAME));
		   
			int nRet = StatedLoad(0);

			if (nRet==0)
				ShowMessageBoxEx(NULL,NULL,L"FBANext - Load State", L"Loaded state successfully", 1, (LPCWSTR*)&button_text,NULL,  XUI_MB_CENTER_ON_PARENT, NULL); 
			else
				ShowMessageBoxEx(NULL,NULL,L"FBANext - Load State", L"Error loading savestate file.", 1, (LPCWSTR*)&button_text,NULL,  XUI_MB_CENTER_ON_PARENT, NULL);  
 			 			 

			bHandled = TRUE;
			return S_OK;			


		} 
		else if (hObjPressed == m_BackToGame)
		{ 			 
			bHandled = TRUE;			 
			bRunPause = 0;
			return S_OK;

		}
		else if (hObjPressed == m_FilterOptions)
		{
			XuiSceneCreate( L"file://game:/media/FBNext.xzp#src\\Interface\\Xbox360\\Skin\\", L"Filters.xur", NULL, &hFilterOptionsScene );
			this->NavigateForward(hFilterOptionsScene);	
			bHandled = TRUE;			 			 
			return S_OK;

		}
		else if (hObjPressed == m_SetInputOptions)
		{
			XuiSceneCreate( L"file://game:/media/FBNext.xzp#src\\Interface\\Xbox360\\Skin\\", L"InputOptions.xur", NULL, &hInputOptionsScene );
			this->NavigateForward(hInputOptionsScene);	
			bHandled = TRUE;			 			 
			return S_OK;

		}
		else if (hObjPressed == m_OtherOptions)
		{
			XuiSceneCreate( L"file://game:/media/FBNext.xzp#src\\Interface\\Xbox360\\Skin\\", L"OtherOptions.xur", NULL, &hOtherOptionsScene );
			this->NavigateForward(hOtherOptionsScene);	
			bHandled = TRUE;			 			 
			return S_OK;
		}
		else if (hObjPressed == m_SetDipSwitches)
		{
			XuiSceneCreate( L"file://game:/media/FBNext.xzp#src\\Interface\\Xbox360\\Skin\\", L"DipOptions.xur", NULL, &hDipOptionsScene );
			this->NavigateForward(hDipOptionsScene);	
			bHandled = TRUE;			 			 
			return S_OK;
		}
		else if (hObjPressed == m_ExitGame)
		{	
			bRunPause = 0;
			exitGame = TRUE;		
			bHandled = TRUE;			 			     
			this->NavigateBack(hRomListScene);
 			return S_OK;
		}	
		else if (hObjPressed == m_AdjustScreen)
		{
			bRunPause = 0;
			bHandled = TRUE;
			bAdjustScreen = 1;
			return S_OK;
		}
		else if (hObjPressed == m_Reset)
		{		  
			DoReset = true;
			bRunPause = 0;
			bHandled = TRUE;
			return S_OK;
		}										
		else if (hObjPressed == m_TakePreview)
		{
			 

			if (MakeScreenShot(true, 1)==0)
			{
		 
 				bHandled = TRUE;
 				return S_OK;
 			} 

		}
		else if (hObjPressed == m_TakeTitle)
		{
			 

			if (MakeScreenShot(true, 0)==0)
			{
		 
 				bHandled = TRUE;
 				return S_OK;
 			} 

		}


 
        bHandled = TRUE;
        return S_OK;
    }


    //----------------------------------------------------------------------------------
    // Performs initialization tasks - retreives controls.
    //----------------------------------------------------------------------------------
HRESULT CInGameOptions::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
    {
        // Retrieve controls for later use.
        GetChildById( L"XuiSaveStateButton", &m_SaveState );
        GetChildById( L"XuiLoadStateButton", &m_LoadState );
 
 
 
		GetChildById( L"XuiButtonReset", &m_Reset);
 
		GetChildById( L"XuiPreviewImage", &m_PreviewImage );
		GetChildById( L"XuiPreviewSmall", &m_PreviewSmallImage );
		GetChildById( L"XuiButtonBackToGame", &m_BackToGame);
		GetChildById( L"XuiButtonExitGame", &m_ExitGame);
 
 
 
		GetChildById( L"XuiButtonTakePreview", &m_TakePreview);
		GetChildById( L"XuiButtonTakeTitle", &m_TakeTitle);
 
		GetChildById( L"XuiButtonFilters", &m_FilterOptions);
		GetChildById( L"XuiButtonSetDipSwitches", &m_SetDipSwitches);
		GetChildById( L"XuiButtonMapInputs", &m_SetInputOptions);
	
		GetChildById( L"XuiButtonMiscOptions", &m_OtherOptions);
		GetChildById( L"XuiButtonAdjustScreen", &m_AdjustScreen);

        return S_OK;
    }


HRESULT CInGameOptions::OnInGameMenu( int iVal1,  BOOL& bHandled )
{ 
	
	DeleteFile("cache:\\preview.png");
 
	if (MakeScreenShot(true, 2)==0)
	{
		HRESULT hr;
 
		m_PreviewImage.DiscardResources(XUI_DISCARD_TEXTURES|XUI_DISCARD_VISUALS);
		m_PreviewSmallImage.DiscardResources(XUI_DISCARD_TEXTURES|XUI_DISCARD_VISUALS);

		hr = m_PreviewSmallImage.SetBasePath(L"file://cache:/");
		hr = m_PreviewSmallImage.SetImagePath(L"file://cache:/preview.png");

		bHandled = TRUE;
	} 
 

    return( S_OK );
}

void InGameMenuFirstFunc(XUIMessage *pMsg, InGameMenuStruct* pData, int iVal1)
{
    XuiMessage(pMsg,XM_MESSAGE_ON_INGAME_MENU);
    _XuiMessageExtra(pMsg,(XUIMessageData*) pData, sizeof(*pData));

	
}


HRESULT CFilterOptions::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{
 
	GetChildById( L"XuiAspectRatio", &m_AspectRatio);
	GetChildById( L"XuiPointFiltering", &m_PointFiltering);
	GetChildById( L"XuiRadioGroupFilters", &m_FilterGroup);
	GetChildById( L"XuiButtonBackToMain", &m_BackToMain);
	GetChildById( L"XuiButtonBackToGame", &m_BackToGame);

	if (bVidCorrectAspect==1)
	{
		m_AspectRatio.SetCheck(true);
	}
	else
	{
		m_AspectRatio.SetCheck(false);
	}

	if (vidFilterLinear==1)
	{
		m_PointFiltering.SetCheck(false);
	}
	else
	{
		m_PointFiltering.SetCheck(true);
	}

	m_FilterGroup.SetCurSel(nVidFilter);

	return S_OK;
}

// Handler for the XM_NOTIFY message
HRESULT CFilterOptions::OnNotifyPress( HXUIOBJ hObjPressed, 
       BOOL& bHandled )
{

	if (hObjPressed == m_BackToGame)
	{ 			 
		configAppSaveXml();
		bHandled = TRUE;			 
		bRunPause = 0;
		return S_OK;
	}
	else if (hObjPressed == m_BackToMain)
	{
		this->NavigateBack();
		configAppSaveXml();
		bHandled = TRUE;			 		
		return S_OK;
	}
	else if (hObjPressed == m_PointFiltering)
	{
		if(m_PointFiltering.IsChecked()) {				 
			vidFilterLinear = 0;
		} else {				 
			vidFilterLinear = 1; 
		}
		bHandled = TRUE;
		return S_OK;
	}
	else if (hObjPressed == m_AspectRatio)
	{
		if(m_AspectRatio.IsChecked()) {				 
			bVidCorrectAspect = 1;				 
			bVidFullStretch = 0;
		} else {				 
			bVidCorrectAspect = 0;
			bVidFullStretch = 1;
		}

		bHandled = TRUE;
		return S_OK;

	}

	switch(m_FilterGroup.GetCurSel())
	{
		case FILTER_NONE:
			vidUseFilter = 0;
			nVidFilter = 0;
			break;
		case FILTER_EPXB:
			vidUseFilter = 1;
			nVidFilter = 1;
			break;
		case FILTER_EPXC:
			vidUseFilter = 1;
			nVidFilter = 2;
			break;
		case FILTER_SCALE2X:
			vidUseFilter = 1;
			nVidFilter = 3;
			break;
		case FILTER_SCALE3X:
			vidUseFilter = 1;
			nVidFilter = 4;
			break;
		case FILTER_2XSAI:
			vidUseFilter = 1;
			nVidFilter = 5;
			break;
		case FILTER_SUPER2XSAI:
			vidUseFilter = 1;
			nVidFilter = 6;
			break;
		case FILTER_SUPEREAGLE:
			vidUseFilter = 1;
			nVidFilter = 7;
			break;
		case FILTER_2XPMHQ:
			vidUseFilter = 1;
			nVidFilter = 8;
			break;
		case FILTER_HQ2X:
			vidUseFilter = 1;
			nVidFilter = 9;
			break;
		case FILTER_HQ2XS:
			vidUseFilter = 1;
			nVidFilter = 10;
			break;
		case FILTER_HQ2XBOLD:
			vidUseFilter = 1;
			nVidFilter = 11;
			break;
		case FILTER_HQ3X:
			vidUseFilter = 1;
			nVidFilter = 12;
			break;
		case FILTER_HQ3XS:
			vidUseFilter = 1;
			nVidFilter = 13;
			break;
		case FILTER_HQ3XBOLD:
			vidUseFilter = 1;
			nVidFilter = 14;
			break;
		case FILTER_SCANLINE:
			vidUseFilter = 1;
			nVidFilter = 15;
			break;
		case FILTER_SCANLINE50:
			vidUseFilter = 1;
			nVidFilter = 16;
			break;
		case FILTER_SCANLINE25:
			vidUseFilter = 1;
			nVidFilter = 17;
			break;
		case FILTER_INTERSCANLINE:
			vidUseFilter = 1;
			nVidFilter = 18;
			break;
		case FILTER_INTERSCANLINE50:
			vidUseFilter = 1;
			nVidFilter = 19;
			break;
		case FILTER_INTERSCANLINE25:
			vidUseFilter = 1;
			nVidFilter = 20;
			break;
	}
 
	return S_OK;
}


// Handler for the XM_NOTIFY message
HRESULT CDipOptions::OnNotifyPress( HXUIOBJ hObjPressed, 
       BOOL& bHandled )
{	
    if ( hObjPressed == m_DipList )
    {
		nDIPSel = m_DipList.GetCurSel();	

		XUIMessage xuiMsg;
		XUIMessageInit xuiMsgInit;
		XuiMessageInit( &xuiMsg, &xuiMsgInit, (void *)&m_DipSettingsList );
		// send the XM_INIT message
		XuiSendMessage( m_DipSettingsList.m_hObj, &xuiMsg );

		//m_DipSettingsList.SetCurSel(0);
		m_DipSettingsList.SetFocus();
		bHandled = TRUE;			 		 
		return S_OK;
    }
	else if (hObjPressed == m_DipSettingsList)
	{
		int id = m_DipSettingsList.GetCurSel();
		BurnDIPInfo bdi = {0, 0, 0, 0, NULL};
		int j = 0;
		for (int i = 0; i <= id; i++) {
			do {
				BurnDrvGetDIPInfo(&bdi, nDIPGroup + 1 + j++);
			} while (bdi.nFlags == 0);
		}

		struct GameInp* pgi = GameInp + bdi.nInput + nDIPOffset;
		pgi->Input.Constant = (pgi->Input.Constant & ~bdi.nMask) | (bdi.nSetting & bdi.nMask);
		if (bdi.nFlags & 0x40) {
			while (BurnDrvGetDIPInfo(&bdi, nDIPGroup + 1 + j++) == 0) {
				if (bdi.nFlags == 0) {
					pgi = GameInp + bdi.nInput + nDIPOffset;
					pgi->Input.Constant = (pgi->Input.Constant & ~bdi.nMask) | (bdi.nSetting & bdi.nMask);
				} else {
					break;
				}
			}
		}

		XUIMessage xuiMsg;
		XUIMessageInit xuiMsgInit;
		XuiMessageInit( &xuiMsg, &xuiMsgInit, (void *)&m_DipList );
		// send the XM_INIT message
		XuiSendMessage( m_DipList.m_hObj, &xuiMsg );

		m_DipList.SetFocus();
		bHandled = TRUE;			 		 
		return S_OK;
	}
	else if (hObjPressed == m_ResetDips)
	{
		InpDIPSWResetDIPs();

		XUIMessage xuiMsg;
		XUIMessageInit xuiMsgInit;
		XuiMessageInit( &xuiMsg, &xuiMsgInit, (void *)&m_DipList );
		// send the XM_INIT message
		XuiSendMessage( m_DipList.m_hObj, &xuiMsg );

		m_DipList.SetFocus();

		bHandled = TRUE;			 		 
		return S_OK;

	}
	else if (hObjPressed == m_BackToGame)
	{ 			 
		bHandled = TRUE;			 
		bRunPause = 0;
		return S_OK;
	}
	else if (hObjPressed == m_BackToMain)
	{
		this->NavigateBack();
		bHandled = TRUE;			 		
		return S_OK;
	}
    else
    {
        return S_OK;
    }
    bHandled = TRUE;
    return S_OK;

}


HRESULT CDipOptions::OnInit(XUIMessageInit *pInitData, BOOL& bHandled)
{
    // Retrieve controls for later use.

	HRESULT hr;

    GetChildById( L"XuiButtonBackToMain", &m_BackToMain );
    GetChildById( L"XuiButtonBackToGame", &m_BackToGame );
    GetChildById( L"XuiDipList", &m_DipList );
    GetChildById( L"XuiPreviewSmall", &m_PreviewSmallImage );
	GetChildById( L"XuiDipSettings",&m_DipSettingsList);
	GetChildById( L"XuiResetDips",&m_ResetDips);
		 
	m_PreviewSmallImage.DiscardResources(XUI_DISCARD_TEXTURES|XUI_DISCARD_VISUALS);

	hr = m_PreviewSmallImage.SetBasePath(L"file://cache:/");
	hr = m_PreviewSmallImage.SetImagePath(L"file://cache:/preview.png");

    return S_OK;
}
 
CDipList::CDipList()
{
	hInpDIPSWList = HWND(0x10);
}

HRESULT CDipList::OnInit(XUIMessageInit *pInitData, BOOL& bHandled)
{
 
	if (hInpDIPSWList == NULL) {
		return 1;
	}

	m_DipList.clear();
	m_DipListData.clear();
	m_DipListOffsets.clear();
 
	BurnDIPInfo bdi;
	unsigned int i = 0, j = 0, k = 0;
	char* pDIPGroup = NULL;
	while (BurnDrvGetDIPInfo(&bdi, i) == 0) {
		if ((bdi.nFlags & 0xF0) == 0xF0) {
		   	if (bdi.nFlags == 0xFE || bdi.nFlags == 0xFD) {
				pDIPGroup = (char*)bdi.szText;
				k = i;
			}
			i++;
		} else {
			if (CheckSetting(i)) { 
				if (pDIPGroup)
				{
					m_DipList[std::string(pDIPGroup)] = std::string(bdi.szText);
					m_DipListData.push_back(pDIPGroup);
					m_DipListOffsets.push_back(k);
				}
				j++;
			}
			i += (bdi.nFlags & 0x0F);
		}
	}
 
	DeleteItems( 0, m_DipList.size() );
	InsertItems( 0, m_DipList.size() );

    return S_OK;
}
   
    // Gets called every frame
HRESULT CDipList::OnGetSourceDataText(
        XUIMessageGetSourceText *pGetSourceTextData, 
        BOOL& bHandled)
{
	 
	

    if( ( 0 == pGetSourceTextData->iData ) && ( ( pGetSourceTextData->bItemData ) ) ) {

		LPCWSTR lpszwBuffer = MultiCharToUniChar((char *)m_DipListData[pGetSourceTextData->iItem].c_str());
		swprintf(DipSetting,L"%s : %S", lpszwBuffer, m_DipList[std::string((char *)m_DipListData[pGetSourceTextData->iItem].c_str())].c_str());
		pGetSourceTextData->szText = DipSetting;
        bHandled = TRUE;
    }
    return S_OK;
}
    
HRESULT CDipList::OnGetItemCountAll(
        XUIMessageGetItemCount *pGetItemCountData, 
        BOOL& bHandled)
{
    pGetItemCountData->cItems = m_DipListData.size();
    bHandled = TRUE;
    return S_OK;
}   

CDipSettingsList::CDipSettingsList()
{
	 
}

HRESULT CDipSettingsList::OnInit(XUIMessageInit *pInitData, BOOL& bHandled)
{
 
	int nSel = nDIPSel;

	m_DipListValues.clear();

	if (nSel >= 0) {
 
		if (m_DipListOffsets.size() > 0)
		{
			nDIPGroup = m_DipListOffsets[nSel];

			BurnDIPInfo bdiGroup;
			BurnDrvGetDIPInfo(&bdiGroup, nDIPGroup);

			int nCurrentSetting = 0;
			for (int i = 0, j = 0; i < bdiGroup.nSetting; i++) {
				TCHAR szText[MAX_PATH];
				BurnDIPInfo bdi;

				do {
					BurnDrvGetDIPInfo(&bdi, nDIPGroup + 1 + j++);
				} while (bdi.nFlags == 0);
				if (bdiGroup.szText) {
					_stprintf(szText, _T("%hs: %hs"), bdiGroup.szText, bdi.szText);
				} else {
					_stprintf(szText, _T("%hs"), bdi.szText);
				}
			 
				m_DipListValues.push_back(std::string(szText));

				if (CheckSetting(nDIPGroup + j)) {
					nCurrentSetting = i;
				}
			}
		}
		 
	}

	DeleteItems( 0, m_DipListValues.size() );
	InsertItems( 0, m_DipListValues.size() );

    return S_OK;
}
   
    // Gets called every frame
HRESULT CDipSettingsList::OnGetSourceDataText(
        XUIMessageGetSourceText *pGetSourceTextData, 
        BOOL& bHandled)
{
	 
    if( ( 0 == pGetSourceTextData->iData ) && ( ( pGetSourceTextData->bItemData ) ) ) {

		LPCWSTR lpszwBuffer = MultiCharToUniChar((char *)m_DipListValues[pGetSourceTextData->iItem].c_str());
        pGetSourceTextData->szText = lpszwBuffer;
        bHandled = TRUE;
    }
     
    return S_OK;
}
    
HRESULT CDipSettingsList::OnGetItemCountAll(
        XUIMessageGetItemCount *pGetItemCountData, 
        BOOL& bHandled)
{

    pGetItemCountData->cItems = m_DipListValues.size();
    bHandled = TRUE;
     
    return S_OK;
}   


// Handler for the XM_NOTIFY message
HRESULT CInputOptions::OnNotifyPress( HXUIOBJ hObjPressed, 
       BOOL& bHandled )
{	
   struct GameInp* pgi;

    if ( hObjPressed == m_InputList )
    {
		nInpSel = m_InputList.GetCurSel();	

		XUIMessage xuiMsg;
		XUIMessageInit xuiMsgInit;
		XuiMessageInit( &xuiMsg, &xuiMsgInit, (void *)&m_InputSettingsList );
		// send the XM_INIT message
		XuiSendMessage( m_InputSettingsList.m_hObj, &xuiMsg );
		 						
		m_InputSettingsList.SetFocus();

		bHandled = TRUE;			 		 
		return S_OK;
    }
	else if (hObjPressed == m_InputSettingsList)
	{
		int id = m_InputSettingsList.GetCurSel();		

		pgi = GameInp + m_InputListOffsets[m_InputList.GetCurSel()];
 		 
 
		if (strstr(m_InputListData[m_InputList.GetCurSel()].c_str(), "Service"))
		{
			switch (id)
			{
			case 0:
				pgi->nInput = GIT_SWITCH;
				pgi->Input.Switch = (unsigned short)FBK_C;
				break;
			case 1:
				pgi->nInput = GIT_SWITCH;
				pgi->Input.Switch = (unsigned short)FBK_Z;
				break;
			case 2:
				pgi->nInput = GIT_SWITCH;
				pgi->Input.Switch = (unsigned short)FBK_X;
				break;
			case 3:
				pgi->nInput = GIT_SWITCH;
				pgi->Input.Switch = (unsigned short)FBK_V;
				break;
			case 4:	
				pgi->nInput = GIT_SWITCH;
				pgi->Input.Switch = (unsigned short)FBK_1;
				break;
			case 5:
				pgi->nInput = GIT_SWITCH;
				pgi->Input.Switch = (unsigned short)FBK_5;
				break;
			case 6:
				pgi->nInput = GIT_SWITCH;
				pgi->Input.Switch = (unsigned short)FBK_S;
				break;
			case 7:
				pgi->nInput = GIT_SWITCH;
				pgi->Input.Switch = (unsigned short)FBK_D;
				break;
			case 8:
				pgi->nInput = GIT_SWITCH;
				pgi->Input.Switch = (unsigned short)FBK_F1;
				break;
			case 9:
				pgi->nInput = GIT_SWITCH;
				pgi->Input.Switch = (unsigned short)FBK_F2;
				break;
			case 10: 
				pgi->nInput = GIT_SWITCH;
				pgi->Input.Switch = (unsigned short)XBOX_LEFT_TRIGGER; 
				break;
			case 11:
				pgi->nInput = GIT_SWITCH;
				pgi->Input.Switch = (unsigned short)XBOX_RIGHT_TRIGGER;			 
				break;			 
			case 12:
				pgi->nInput = GIT_SWITCH;
				pgi->Input.Switch = (unsigned short)FBK_F1 & FBK_F2;			 
				break;
			}
		}

		else if (strstr(m_InputListData[m_InputList.GetCurSel()].c_str(), "P1"))
		{
			switch (id)
			{
			case 0:
				if (pgi->nInput & GIT_GROUP_MACRO)
				{
					MACRO(FBK_C);
				}
				else
				{
					KEY(FBK_C);
				}			 
				break;
			case 1:
				if (pgi->nInput & GIT_GROUP_MACRO)
				{
					MACRO(FBK_Z);
				}
				else
				{
					KEY(FBK_Z);
				}	
				break;
			case 2:
				if (pgi->nInput & GIT_GROUP_MACRO)
				{
					MACRO(FBK_X);
				}
				else
				{
					KEY(FBK_X);
				}
				break;
			case 3:
				if (pgi->nInput & GIT_GROUP_MACRO)
				{
					MACRO(FBK_V);
				}
				else
				{
					KEY(FBK_V);
				}
				break;
			case 4:	
				if (pgi->nInput & GIT_GROUP_MACRO)
				{
					MACRO(FBK_1);
				}
				else
				{
					KEY(FBK_1);
				}
				break;
			case 5:
				if (pgi->nInput & GIT_GROUP_MACRO)
				{
					MACRO(FBK_5);
				}
				else
				{
					KEY(FBK_5);
				}
				break;
			case 6:
				if (pgi->nInput & GIT_GROUP_MACRO)
				{
					MACRO(FBK_S);
				}
				else
				{
					KEY(FBK_S);
				}
				break;
			case 7:
				if (pgi->nInput & GIT_GROUP_MACRO)
				{
					MACRO(FBK_D);
				}
				else
				{
					KEY(FBK_D);
				}
				break;
			case 8:
				if (pgi->nInput & GIT_GROUP_MACRO)
				{
					MACRO(FBK_F1);
				}
				else
				{
					KEY(FBK_F1);
				}
				break;
			case 9:
				if (pgi->nInput & GIT_GROUP_MACRO)
				{
					MACRO(FBK_F2);
				}
				else
				{
					KEY(FBK_F2);
				}
				break;
			case 10: 
				if (pgi->nInput & GIT_GROUP_MACRO)
				{
					MACRO(XBOX_LEFT_TRIGGER);
				}
				else
				{
					KEY(XBOX_LEFT_TRIGGER);
				}
				break;
			case 11:
				if (pgi->nInput & GIT_GROUP_MACRO)
				{
					MACRO(XBOX_RIGHT_TRIGGER);
				}
				else
				{
					KEY(XBOX_RIGHT_TRIGGER);
				}		 
				break;			
			case 12:
				if (pgi->nInput & GIT_GROUP_MACRO)
				{
					MACRO(FBK_F1 & FBK_F2);
				}
				else
				{
					KEY(FBK_F1 & FBK_F2);
				}		 			 
				break;
			}
		}
 
		else if (strstr(m_InputListData[m_InputList.GetCurSel()].c_str(), "P2"))
		{
			switch (id)
			{
			case 0:				 
				if (pgi->nInput & GIT_GROUP_MACRO)
				{
					MACRO(0x4082);
				}
				else
				{
					KEY(0x4082);
				}

				break;
			case 1:
				if (pgi->nInput & GIT_GROUP_MACRO)
				{
					MACRO(0x4080);
				}
				else
				{
					KEY(0x4080);
				}
				break;
			case 2:
				if (pgi->nInput & GIT_GROUP_MACRO)
				{
					MACRO(0x4081);
				}
				else
				{
					KEY(0x4081);
				}
				break;
			case 3:
				if (pgi->nInput & GIT_GROUP_MACRO)
				{
					MACRO(0x4083);
				}
				else
				{
					KEY(0x4083);
				}
				break;
			case 4:	
				if (pgi->nInput & GIT_GROUP_MACRO)
				{
					MACRO(0x03);
				}
				else
				{
					KEY(0x03);
				}
				break;
			case 5:
				if (pgi->nInput & GIT_GROUP_MACRO)
				{
					MACRO(0x07);
				}
				else
				{
					KEY(0x07);
				}
				break;
			case 6:
				if (pgi->nInput & GIT_GROUP_MACRO)
				{
					MACRO(0x4084);
				}
				else
				{
					KEY(0x4084);
				}
				break;
			case 7:
				if (pgi->nInput & GIT_GROUP_MACRO)
				{
					MACRO(0x4085);
				}
				else
				{
					KEY(0x4085);
				}
				break;
			case 8:
				if (pgi->nInput & GIT_GROUP_MACRO)
				{
					MACRO(FBK_F1 | 0x4000);
				}
				else
				{
					KEY(FBK_F1 | 0x4000);
				}
				break;
			case 9:
				if (pgi->nInput & GIT_GROUP_MACRO)
				{
					MACRO(FBK_F2 | 0x4000);
				}
				else
				{
					KEY(FBK_F2 | 0x4000);
				}
				break;
			case 10: 
				if (pgi->nInput & GIT_GROUP_MACRO)
				{
					MACRO(XBOX_LEFT_TRIGGER | 0x4000);
				}
				else
				{
					KEY(XBOX_LEFT_TRIGGER | 0x4000);
				} 
				break;
			case 11:
				if (pgi->nInput & GIT_GROUP_MACRO)
				{
					MACRO(XBOX_RIGHT_TRIGGER | 0x4000);
				}
				else
				{
					KEY(XBOX_RIGHT_TRIGGER | 0x4000);
				} 
				break;			 
			case 12:
				if (pgi->nInput & GIT_GROUP_MACRO)
				{
					MACRO(0x4000 | (FBK_F1 & FBK_F2));
				}
				else
				{
					KEY(0x4000 | (FBK_F1 & FBK_F2));
				}		 			 
				break;
			}


		}

		else if (strstr(m_InputListData[m_InputList.GetCurSel()].c_str(), "P3"))
		{

			switch (id)
			{
			case 0:
				if (pgi->nInput & GIT_GROUP_MACRO)
				{
					MACRO(0x4182);
				}
				else
				{
					KEY(0x4182);
				} 
				break;
			case 1:				 
				if (pgi->nInput & GIT_GROUP_MACRO)
				{
					MACRO(0x4180);
				}
				else
				{
					KEY(0x4180);
				} 
				break;
			case 2:
				if (pgi->nInput & GIT_GROUP_MACRO)
				{
					MACRO(0x4181);
				}
				else
				{
					KEY(0x4181);
				} 
				break;
			case 3:
				if (pgi->nInput & GIT_GROUP_MACRO)
				{
					MACRO(0x4183);
				}
				else
				{
					KEY(0x4183);
				} 
				break; 
			case 4:	
				if (pgi->nInput & GIT_GROUP_MACRO)
				{
					MACRO(0x04);
				}
				else
				{
					KEY(0x04);
				} 
				break;
			case 5:
				if (pgi->nInput & GIT_GROUP_MACRO)
				{
					MACRO(0x08);
				}
				else
				{
					KEY(0x08);
				} 
				break;
			case 6:
				if (pgi->nInput & GIT_GROUP_MACRO)
				{
					MACRO(0x4184);
				}
				else
				{
					KEY(0x4184);
				} 
				break;
			case 7:
				if (pgi->nInput & GIT_GROUP_MACRO)
				{
					MACRO(0x4185);
				}
				else
				{
					KEY(0x4185);
				} 
				break;
			case 8:
				if (pgi->nInput & GIT_GROUP_MACRO)
				{
					MACRO(FBK_F1 | 0x4100);
				}
				else
				{
					KEY(FBK_F1 | 0x4100);
				}
				break;
			case 9:
				if (pgi->nInput & GIT_GROUP_MACRO)
				{
					MACRO(FBK_F2 | 0x4100);
				}
				else
				{
					KEY(FBK_F2 | 0x4100);
				}
				break;
			case 10: 
				if (pgi->nInput & GIT_GROUP_MACRO)
				{
					MACRO(XBOX_LEFT_TRIGGER | 0x4100);
				}
				else
				{
					KEY(XBOX_LEFT_TRIGGER | 0x4100);
				} 
				break;
			case 11:
				if (pgi->nInput & GIT_GROUP_MACRO)
				{
					MACRO(XBOX_RIGHT_TRIGGER | 0x4100);
				}
				else
				{
					KEY(XBOX_RIGHT_TRIGGER | 0x4100);
				} 
				break;
			case 12:
				if (pgi->nInput & GIT_GROUP_MACRO)
				{
					MACRO(0x4100 | (FBK_F1 & FBK_F2));
				}
				else
				{
					KEY(0x4100 | (FBK_F1 & FBK_F2));
				}		 			 
				break;
			}
 
		}

		else if (strstr(m_InputListData[m_InputList.GetCurSel()].c_str(), "P4"))
		{
			switch (id)
			{
			case 0:
				if (pgi->nInput & GIT_GROUP_MACRO)
				{
					MACRO(0x4282);
				}
				else
				{
					KEY(0x4282);
				} 
				break;
			case 1:
				if (pgi->nInput & GIT_GROUP_MACRO)
				{
					MACRO(0x4280);
				}
				else
				{
					KEY(0x4280);
				} 
				break;
			case 3:
				if (pgi->nInput & GIT_GROUP_MACRO)
				{
					MACRO(0x4283);
				}
				else
				{
					KEY(0x4283);
				}  
				break;
			case 4:	
				if (pgi->nInput & GIT_GROUP_MACRO)
				{
					MACRO(0x05);
				}
				else
				{
					KEY(0x05);
				} 
				break;
			case 5:
				if (pgi->nInput & GIT_GROUP_MACRO)
				{
					MACRO(0x09);
				}
				else
				{
					KEY(0x09);
				} 
				break;
			case 6:
				if (pgi->nInput & GIT_GROUP_MACRO)
				{
					MACRO(0x4284);
				}
				else
				{
					KEY(0x4284);
				} 
				break;
			case 7:
				if (pgi->nInput & GIT_GROUP_MACRO)
				{
					MACRO(0x4285);
				}
				else
				{
					KEY(0x4285);
				} 
				break;
			case 8:
				if (pgi->nInput & GIT_GROUP_MACRO)
				{
					MACRO(FBK_F1 | 0x4200);
				}
				else
				{
					KEY(FBK_F1 | 0x4200);
				}
				break;
			case 9:
				if (pgi->nInput & GIT_GROUP_MACRO)
				{
					MACRO(FBK_F2 | 0x4200);
				}
				else
				{
					KEY(FBK_F2 | 0x4200);
				}
				break;
			case 10: 
				if (pgi->nInput & GIT_GROUP_MACRO)
				{
					MACRO(XBOX_LEFT_TRIGGER | 0x4200);
				}
				else
				{
					KEY(XBOX_LEFT_TRIGGER | 0x4200);
				} 
				break;
			case 11:
				if (pgi->nInput & GIT_GROUP_MACRO)
				{
					MACRO(XBOX_RIGHT_TRIGGER | 0x4200);
				}
				else
				{
					KEY(XBOX_RIGHT_TRIGGER | 0x4200);
				} 
				break;
			case 12:
				if (pgi->nInput & GIT_GROUP_MACRO)
				{
					MACRO(0x4200 | (FBK_F1 & FBK_F2));
				}
				else
				{
					KEY(0x4200 | (FBK_F1 & FBK_F2));
				}		 			 
				break;
			}


		}
		else
		{
			switch (id)
			{
			case 0:
				pgi->nInput = GIT_SWITCH;
				pgi->Input.Switch = (unsigned short)FBK_C;
				break;
			case 1:
				pgi->nInput = GIT_SWITCH;
				pgi->Input.Switch = (unsigned short)FBK_Z;
				break;
			case 2:
				pgi->nInput = GIT_SWITCH;
				pgi->Input.Switch = (unsigned short)FBK_X;
				break;
			case 3:
				pgi->nInput = GIT_SWITCH;
				pgi->Input.Switch = (unsigned short)FBK_V;
				break;
			case 4:	
				pgi->nInput = GIT_SWITCH;
				pgi->Input.Switch = (unsigned short)FBK_1;
				break;
			case 5:
				pgi->nInput = GIT_SWITCH;
				pgi->Input.Switch = (unsigned short)FBK_5;
				break;
			case 6:
				pgi->nInput = GIT_SWITCH;
				pgi->Input.Switch = (unsigned short)FBK_S;
				break;
			case 7:
				pgi->nInput = GIT_SWITCH;
				pgi->Input.Switch = (unsigned short)FBK_D;
				break;
			case 8:
				pgi->nInput = GIT_SWITCH;
				pgi->Input.Switch = (unsigned short)FBK_F1;
				break;
			case 9:
				pgi->nInput = GIT_SWITCH;
				pgi->Input.Switch = (unsigned short)FBK_F2;
				break;
			case 10: 
				pgi->nInput = GIT_SWITCH;
				pgi->Input.Switch = (unsigned short)XBOX_LEFT_TRIGGER; 
				break;
			case 11:
				pgi->nInput = GIT_SWITCH;
				pgi->Input.Switch = (unsigned short)XBOX_RIGHT_TRIGGER;			 
				break;			 
			}

		}

		
		XUIMessage xuiMsg;
		XUIMessageInit xuiMsgInit;
		XuiMessageInit( &xuiMsg, &xuiMsgInit, (void *)&m_InputList );
		// send the XM_INIT message
		XuiSendMessage( m_InputList.m_hObj, &xuiMsg );
	
		m_InputList.SetFocus();

		bHandled = TRUE;			 		 
		return S_OK;
	}
	else if (hObjPressed == m_SavePreset)
	{

		const WCHAR * button_text = L"OK";

		if (SaveDefaultInput()==0)
		{
			ShowMessageBoxEx(NULL,NULL,L"FBANext - Input Preset", L"Preset Saved.", 1, (LPCWSTR*)&button_text,NULL,  XUI_MB_CENTER_ON_PARENT, NULL); 
		}
		else
		{
			ShowMessageBoxEx(NULL,NULL,L"FBANext - Input Preset", L"Error saving preset.", 1, (LPCWSTR*)&button_text,NULL,  XUI_MB_CENTER_ON_PARENT, NULL); 
		}
	
		bHandled = TRUE;			 		 
		return S_OK;
	}
	else if (hObjPressed == m_ResetInputs)
	{

		struct GameInp* pgi = NULL;
		unsigned int i;

		for (i = 0, pgi = GameInp; i < nGameInpCount; i++, pgi++) {
			struct BurnInputInfo bii;

			// Get the extra info about the input
			bii.szInfo = NULL;
			BurnDrvGetInputInfo(&bii, i);
			if (bii.pVal == NULL) {
				continue;
			}
			if (bii.szInfo == NULL) {
				bii.szInfo = "";
			}
			GamcPlayer(pgi, (char*)bii.szInfo, 0, -1);						// Keyboard
			GamcAnalogKey(pgi, (char*)bii.szInfo, 0, 1);
			GamcMisc(pgi, (char*)bii.szInfo, 0);
		}

 

		XUIMessage xuiMsg;
		XUIMessageInit xuiMsgInit;
		XuiMessageInit( &xuiMsg, &xuiMsgInit, (void *)&m_InputList );
		// send the XM_INIT message
		XuiSendMessage( m_InputList.m_hObj, &xuiMsg );
	
		m_InputList.SetFocus();
		bHandled = TRUE;			 		 
		return S_OK;
	}
	else if (hObjPressed == m_BackToGame)
	{ 			 
		bHandled = TRUE;			 
		bRunPause = 0;
		return S_OK;
	}
	else if (hObjPressed == m_BackToMain)
	{
		this->NavigateBack();
		bHandled = TRUE;			 		
		return S_OK;
	}
    else
    {
        return S_OK;
    }
    bHandled = TRUE;
    return S_OK;

}


HRESULT CInputOptions::OnInit(XUIMessageInit *pInitData, BOOL& bHandled)
{
 
    // Retrieve controls for later use.

	HRESULT hr;

    GetChildById( L"XuiButtonBackToMain", &m_BackToMain );
    GetChildById( L"XuiButtonBackToGame", &m_BackToGame );
    GetChildById( L"XuiInputList", &m_InputList );
    GetChildById( L"XuiPreviewSmall", &m_PreviewSmallImage );
	GetChildById( L"XuiInputSettings",&m_InputSettingsList);
	GetChildById( L"XuiResetInputs", &m_ResetInputs);
	GetChildById( L"XuiSavePreset", &m_SavePreset);
		 
	m_PreviewSmallImage.DiscardResources(XUI_DISCARD_TEXTURES|XUI_DISCARD_VISUALS);

	hr = m_PreviewSmallImage.SetBasePath(L"file://cache:/");
	hr = m_PreviewSmallImage.SetImagePath(L"file://cache:/preview.png");

    return S_OK;
}
 
CInputList::CInputList()
{
	 
}

HRESULT CInputList::OnInit(XUIMessageInit *pInitData, BOOL& bHandled)
{
 
	unsigned int i, j = 0;
	//bLastValDefined = 0;


	// get button info
	int nButtons = 0; // buttons per player
	int nPlayer = 0;
	int nPlayerTemp = 0;

	m_InputList.clear();
	m_InputListData.clear();
	m_InputListOffsets.clear();

	// Add all the input names to the list
	for (unsigned int i = 0; i < nGameInpCount; i++) {
		// Get the name of the input
		struct BurnInputInfo bii;
		bii.szName = NULL;
		BurnDrvGetInputInfo(&bii, i);

		// skip unused inputs
		if (bii.pVal == NULL) {
			continue;
		}
		if (bii.szName == NULL)	{
			bii.szName = "";
		}
 
		m_InputList[std::string(bii.szName)] = std::string(" ");
		m_InputListData.push_back(std::string(bii.szName));
		m_InputListOffsets.push_back(j);
 		 
		j++;


		
	}

	struct GameInp* pgi = GameInp + nGameInpCount;
	for (unsigned int i = 0; i < nMacroCount; i++, pgi++) {
		if (pgi->nInput & GIT_GROUP_MACRO) {

			//pgi->Macro.szName

			m_InputList[std::string(pgi->Macro.szName)] = std::string(" ");
			m_InputListData.push_back(std::string(pgi->Macro.szName));
			m_InputListOffsets.push_back(j);

		}

		j++;
	}	 


	pgi = NULL;

	// Update the values of all the inputs
	int z = 0;
	for (i = 0, pgi = GameInp; i < nGameInpCount; i++, pgi++) {
		if (pgi->Input.pVal == NULL) {
			continue;
		}

		BurnInputInfo bii;
		bii.szName = NULL;
		BurnDrvGetInputInfo(&bii, i);

		// skip unused inputs
		if (bii.pVal == NULL) {
			continue;
		}
		if (bii.szName == NULL)	{
			bii.szName = "";
		}
 
		const char * pszVal = InpToDesc(pgi);

		m_InputList[m_InputListData[z].c_str()] = std::string(pszVal);

		j++;
		z++;
 

	}

	for (i = 0, pgi = GameInp + nGameInpCount; i < nMacroCount; i++, pgi++) {
		if (pgi->nInput & GIT_GROUP_MACRO) {
			TCHAR* pszVal = InpMacroToDesc(pgi);
			m_InputList[m_InputListData[z].c_str()] = std::string(pszVal);
			 
		}

		j++;
		z++;
	}


	DeleteItems( 0, m_InputList.size() );
	InsertItems( 0, m_InputList.size() );
 
    return S_OK;
}
   
    // Gets called every frame
HRESULT CInputList::OnGetSourceDataText(
        XUIMessageGetSourceText *pGetSourceTextData, 
        BOOL& bHandled)
{
    if( ( 0 == pGetSourceTextData->iData ) && ( ( pGetSourceTextData->bItemData ) ) ) {

		LPCWSTR lpszwBuffer = MultiCharToUniChar((char *)m_InputListData[pGetSourceTextData->iItem].c_str());
		swprintf(InpSetting,L"%s : %S", lpszwBuffer, m_InputList[std::string((char *)m_InputListData[pGetSourceTextData->iItem].c_str())].c_str());
		pGetSourceTextData->szText = InpSetting;
        bHandled = TRUE;
    }

    return S_OK;
}
    
HRESULT CInputList::OnGetItemCountAll(
        XUIMessageGetItemCount *pGetItemCountData, 
        BOOL& bHandled)
{
    pGetItemCountData->cItems = m_InputListData.size();     
    bHandled = TRUE;
    return S_OK;
}   

CInputSettingsList::CInputSettingsList()
{
	 
}

HRESULT CInputSettingsList::OnInit(XUIMessageInit *pInitData, BOOL& bHandled)
{
	m_InputSettingsList.clear();
	m_InputSettingsData.clear();
	m_InputSettingsOffsets.clear();

	m_InputSettingsList[XINPUT_GAMEPAD_X] = std::string(_T("X Button"));
	m_InputSettingsList[XINPUT_GAMEPAD_A] = std::string(_T("A Button"));
	m_InputSettingsList[XINPUT_GAMEPAD_B] = std::string(_T("B Button"));
	m_InputSettingsList[XINPUT_GAMEPAD_Y] = std::string(_T("Y Button"));
	m_InputSettingsList[XINPUT_GAMEPAD_START] = std::string(_T("Start Button"));
	m_InputSettingsList[XINPUT_GAMEPAD_BACK] = std::string(_T("Back Button"));
	m_InputSettingsList[XINPUT_GAMEPAD_LEFT_SHOULDER] = std::string(_T("Left Shoulder"));
	m_InputSettingsList[XINPUT_GAMEPAD_RIGHT_SHOULDER] = std::string(_T("Right Shoulder"));
	m_InputSettingsList[XINPUT_GAMEPAD_LEFT_THUMB] = std::string(_T("Left Thumb"));
	m_InputSettingsList[XINPUT_GAMEPAD_RIGHT_THUMB] = std::string(_T("Right Thumb"));
	m_InputSettingsList[0x88] = std::string(_T("Left Trigger"));
	m_InputSettingsList[0x8A] = std::string(_T("Right Trigger"));
	m_InputSettingsList[XINPUT_GAMEPAD_LEFT_THUMB | XINPUT_GAMEPAD_RIGHT_THUMB] = std::string(_T("Left Thumb + Right Thumb"));

	m_InputSettingsData.push_back(std::string(_T("X Button")));
	m_InputSettingsData.push_back(std::string(_T("A Button")));
	m_InputSettingsData.push_back(std::string(_T("B Button")));
	m_InputSettingsData.push_back(std::string(_T("Y Button")));
	m_InputSettingsData.push_back(std::string(_T("Start Button")));
	m_InputSettingsData.push_back(std::string(_T("Back Button")));
	m_InputSettingsData.push_back(std::string(_T("Left Shoulder")));
	m_InputSettingsData.push_back(std::string(_T("Right Shoulder")));
	m_InputSettingsData.push_back(std::string(_T("Left Thumb")));
	m_InputSettingsData.push_back(std::string(_T("Right Thumb")));
	m_InputSettingsData.push_back(std::string(_T("Left Trigger")));
	m_InputSettingsData.push_back(std::string(_T("Right Trigger")));
	m_InputSettingsData.push_back(std::string(_T("Left Thumb + Right Thumb")));

	m_InputSettingsOffsets.push_back(0);
	m_InputSettingsOffsets.push_back(1);
	m_InputSettingsOffsets.push_back(2);
	m_InputSettingsOffsets.push_back(3);
	m_InputSettingsOffsets.push_back(4);
	m_InputSettingsOffsets.push_back(5);
	m_InputSettingsOffsets.push_back(6);
	m_InputSettingsOffsets.push_back(7);
	m_InputSettingsOffsets.push_back(8);
	m_InputSettingsOffsets.push_back(9);
	m_InputSettingsOffsets.push_back(10);
	m_InputSettingsOffsets.push_back(11);
	m_InputSettingsOffsets.push_back(12);

	DeleteItems( 0, m_InputSettingsList.size() );
	InsertItems( 0, m_InputSettingsList.size() );

    return S_OK;
}
   
    // Gets called every frame
HRESULT CInputSettingsList::OnGetSourceDataText(
        XUIMessageGetSourceText *pGetSourceTextData, 
        BOOL& bHandled)
{      

    if( ( 0 == pGetSourceTextData->iData ) && ( ( pGetSourceTextData->bItemData ) ) ) {

		LPCWSTR lpszwBuffer = MultiCharToUniChar((char *)m_InputSettingsData[pGetSourceTextData->iItem].c_str());
		swprintf(InpListSetting,L"%s", lpszwBuffer);
		pGetSourceTextData->szText = InpListSetting;
        bHandled = TRUE;
    }

	return S_OK;
}
    
HRESULT CInputSettingsList::OnGetItemCountAll(
        XUIMessageGetItemCount *pGetItemCountData, 
        BOOL& bHandled)
{

    pGetItemCountData->cItems = m_InputSettingsData.size();     
    bHandled = TRUE;
     
    return S_OK;
}   
 

    //----------------------------------------------------------------------------------
    // Performs initialization tasks - retreives controls.
    //----------------------------------------------------------------------------------
HRESULT COtherOptions::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
    {
        // Retrieve controls for later use.
		GetChildById( L"XuiSlider68kSpeed",   &m_68000Speed);
		GetChildById( L"XuiStickSensitivity", &m_AnalogSensitivity);		 		
		GetChildById( L"XuiButtonBackToMain", &m_BackToMain);
		GetChildById( L"XuiButtonBackToGame", &m_BackToGame);
		GetChildById( L"XuiSpeedLbl", &m_68000SpeedText);
        GetChildById( L"XuiStickLbl", &m_AnalogLabel);

		swprintf_s(SpeedAdjustString, L"%i Percent", nBurnCPUSpeedAdjust * 100 / 256);
		m_68000SpeedText.SetText(SpeedAdjustString);
		m_68000Speed.SetValue(nBurnCPUSpeedAdjust * 100 / 256);

		swprintf_s(AnalogAdjustString, L"%i Percent", nAnalogSpeed * 100 / 256);
		m_AnalogLabel.SetText(AnalogAdjustString);
		m_AnalogSensitivity.SetValue(nAnalogSpeed * 100 / 256);

		return S_OK;
    }


// Handler for the XM_NOTIFY message
HRESULT COtherOptions::OnNotifyPress( HXUIOBJ hObjPressed, 
       BOOL& bHandled )
{		 

	if (hObjPressed == m_BackToGame)
	{ 			 
		configAppSaveXml();
		bHandled = TRUE;			 
		bRunPause = 0;
		return S_OK;
	}
	else if (hObjPressed == m_BackToMain)
	{
		this->NavigateBack();
		configAppSaveXml();
		bHandled = TRUE;			 		
		return S_OK;
	}

	bHandled = TRUE;			 		 
	return S_OK;
}

HRESULT COtherOptions::OnNotifyValueChanged(HXUIOBJ hObjSource, XUINotifyValueChanged *pNotifyValueChangedData, BOOL& bHandled)
{
	int nValue;
	
	if ( hObjSource == m_68000Speed )
    {
	
		nValue = (int)((double)pNotifyValueChangedData->nValue * 256.0 / 100.0 + 0.5);
		nBurnCPUSpeedAdjust = nValue;
		swprintf_s(SpeedAdjustString, L"%i Percent", nBurnCPUSpeedAdjust * 100 / 256);	
		m_68000SpeedText.SetText(SpeedAdjustString);
	}
	else if (hObjSource == m_AnalogSensitivity)
	{

		nValue = (int)((double)pNotifyValueChangedData->nValue * 256.0 / 100.0 + 0.5);
		nAnalogSpeed = nValue;
		swprintf_s(AnalogAdjustString, L"%i Percent", nAnalogSpeed * 100 / 256);	
		m_AnalogLabel.SetText(AnalogAdjustString);

	}

    // The new value of the slider control is in pNotifyValueChangedData->nValue.
    bHandled = TRUE;
    return S_OK;
}
