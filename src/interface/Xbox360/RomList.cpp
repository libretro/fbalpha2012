#include <xmedia2.h>
#include "RomList.h"
#include "burner.h"
#include "Splash.h"
#include "version.h"

HXUIOBJ hPreviewVideo;

extern BOOL IsCurrentlyInGame;
char szRoms[MAX_PATH]; 
char szRomPath[MAX_PATH];

extern IDirect3DDevice9 *pDevice;
extern wchar_t msg[1024];
extern int ArcadeJoystick;
extern void simpleReinitScrn(const bool& reinitVid);
char InGamePreview[MAX_PATH];

int nLastRom = 0;
int nLastFilter = 0;
int HideChildren = 0;
int ThreeOrFourPlayerOnly = 0;

wchar_t DeviceText[60];
extern bool exitGame;
std::vector<std::string> m_ListData;
std::vector<std::string> m_vecAvailRomIndex;
std::vector<std::string> m_vecAvailRomReleasedBy;
std::vector<std::string> m_vecAvailRomManufacturer;
std::vector<std::string> m_vecAvailRomInfo;
std::vector<std::string> m_vecAvailRomParent;

map<int, int> m_HardwareFilterMap;
map<int, std::string> m_HardwareFilterDesc;
map<int, std::string> m_HardwareFilterBackGroundMap;

std::string ReplaceCharInString(const std::string & source, char charToReplace, const std::string replaceString);

#define ALL 0
#define CPS1 1
#define CPS2 2
#define CPS3 3
#define NEOGEO 4
#define TAITO 5
#define SEGA 6
#define PGM 7
#define PSYKIO 8
#define KONAMI 9
#define KANEKO 10
#define CAVE 11
#define TOAPLAN 12
#define SEGAMD 13
#define MISC 14

#define MASKMISC		(1 << (HARDWARE_PREFIX_MISC		>> 24))
#define MASKCPS			(1 << (HARDWARE_PREFIX_CPS1		>> 24))
#define MASKNEOGEO		(1 << (HARDWARE_PREFIX_SNK		>> 24))
#define MASKSEGA		(1 << (HARDWARE_PREFIX_SEGA		>> 24))
#define MASKTOAPLAN		(1 << (HARDWARE_PREFIX_TOAPLAN	>> 24))
#define MASKCAVE		(1 << (HARDWARE_PREFIX_CAVE		>> 24))
#define MASKCPS2		(1 << (HARDWARE_PREFIX_CPS2		>> 24))
#define MASKMD			(1 << (HARDWARE_PREFIX_SEGAMD	>> 24))
#define MASKPGM			(1 << (HARDWARE_PREFIX_PGM		>> 24))
#define MASKCPS3		(1 << (HARDWARE_PREFIX_CPS3		>> 24))
#define MASKTAITO		(1 << (HARDWARE_PREFIX_TAITO	>> 24))
#define MASKPSIKYO		(1 << (HARDWARE_PREFIX_PSIKYO	>> 24))
#define MASKKANEKO16	(1 << (HARDWARE_PREFIX_KANEKO16	>> 24))
#define MASKKONAMI		(1 << (HARDWARE_PREFIX_KONAMI	>> 24))
#define MASKPACMAN		(1 << (HARDWARE_PREFIX_PACMAN	>> 24))
#define MASKGALAXIAN	(1 << (HARDWARE_PREFIX_GALAXIAN >> 24))
#define MASKATARI		(1 << (HARDWARE_PREFIX_ATARI	>> 24))

#define MASKALL \
	(MASKMISC | MASKCPS | MASKNEOGEO | MASKSEGA | MASKTOAPLAN \
	| MASKCAVE | MASKCPS2 | MASKMD | MASKPGM | MASKCPS3 \
	| MASKTAITO | MASKPSIKYO | MASKKANEKO16 | MASKKONAMI | MASKPACMAN \
	| MASKGALAXIAN | MASKATARI)


static int CurrentFilter = 0;
char rom_filename[256];


wchar_t ucString[42];

HXUIOBJ hRomListScene;
HXUIOBJ hInGameOptionsScene = NULL;
LPCWSTR MultiCharToUniChar(char* mbString)
{
	int len = strlen(mbString) + 1;	
	mbstowcs(ucString, mbString, len);
	return (LPCWSTR)ucString;
}

const char *GetFilename (char *path, const char *InFileName, char *fext)
{
    static char filename [MAX_PATH + 1];
    char dir [_MAX_DIR + 1];
    char drive [_MAX_DRIVE + 1];
    char fname [42];
    char ext [_MAX_EXT + 1];
   _splitpath (InFileName, drive, dir, fname, ext);

   std::string fatxfname(fname);
	if (fatxfname.length() > 37)
	{
			fatxfname = fatxfname.substr(0,36);
	}

   _snprintf(filename, sizeof(filename),  "%s%s.%s",path, 
              fatxfname.c_str(), fext);
    return (filename);
}

const char *GetFilenameWithoutExtension (char *path, const char *InFileName)
{
    static char filename [MAX_PATH + 1];
    char dir [_MAX_DIR + 1];
    char drive [_MAX_DRIVE + 1];
    char fname [42];
    char ext [_MAX_EXT + 1];
   _splitpath (InFileName, drive, dir, fname, ext);

   std::string fatxfname(fname);
	if (fatxfname.length() > 37)
	{
			fatxfname = fatxfname.substr(0,36);
	}

   _snprintf(filename, sizeof(filename),  "%s%s",path, 
              fatxfname.c_str());
    return (filename);
}

void InRescanRomsFirstFunc(XUIMessage *pMsg, InRescanRomsStruct* pData, char *szPath)
{
    XuiMessage(pMsg,XM_MESSAGE_ON_RESCAN_ROMS);
    _XuiMessageExtra(pMsg,(XUIMessageData*) pData, sizeof(*pData));

	
}


// Handler for the XM_NOTIFY message
HRESULT CRomListScene::OnNotifyPress( HXUIOBJ hObjPressed, 
       BOOL& bHandled )
    {
		int nIndex;
		wchar_t HardwareBGImage[60];
 
        if ( hObjPressed == m_RomList )
        {
			exitGame = FALSE;
			nIndex = m_RomList.GetCurSel();
			m_RomList.SetShow(false);
			UpdateConsole("Loading Game....");

			nBurnFPS = 6000;

			XuiVideoPause(hPreviewVideo, true);
			if (directLoadGame((TCHAR *)m_vecAvailRomIndex[nIndex].c_str()) == 0)
			{
				nLastRom = m_RomList.GetCurSel();
				nLastFilter = CurrentFilter;
				mediaInit();
				XuiSceneCreate( L"file://game:/media/FBNext.xzp#src\\Interface\\Xbox360\\Skin\\", L"InGameOptions.xur", NULL, &hInGameOptionsScene );
				this->NavigateForward(hInGameOptionsScene);					
				simpleReinitScrn(true);
				RunMessageLoop(); 
				configAppSaveXml();
				
				m_RomList.SetShow(true);
				m_RomList.SetFocus();		
				m_RomList.SetCurSel(nIndex);
			}
			else
			{
				const WCHAR * button_text = L"OK"; 
				ShowMessageBoxEx(L"XuiMessageBox1",NULL,L"FBANext - Rom Error", msg, 1, (LPCWSTR*)&button_text,NULL,  XUI_MB_CENTER_ON_PARENT, NULL); 

				m_RomList.SetShow(true); 
				m_RomList.SetCurSel(nIndex);
			}
			

			
			UpdateConsole("");
			
			bHandled = TRUE;
			return S_OK;
  
        }
		else if (hObjPressed == m_UseArcadeStickSettings)
		{
			if (m_UseArcadeStickSettings.IsChecked())
			{
				ArcadeJoystick = 1;
			}
			else
			{
				ArcadeJoystick = 0;
			}

		}
		else if (hObjPressed == m_34PlayerOnly)
		{

			if (m_34PlayerOnly.IsChecked())
			{
				ThreeOrFourPlayerOnly = 1;
			}
			else
			{
				ThreeOrFourPlayerOnly = 0;
			}

			XuiImageElementSetImagePath(m_PreviewImage.m_hObj, L"");

			XUIMessage xuiMsg;
			InRescanRomsStruct msgData;
			InRescanRomsFirstFunc( &xuiMsg, &msgData, NULL);
			XuiSendMessage( m_RomList.m_hObj, &xuiMsg );

			bHandled = TRUE;
			return S_OK;
		}
		else if (hObjPressed == m_HideChildren)
		{
			if (m_HideChildren.IsChecked())
			{
				HideChildren = 1;
			}
			else
			{
				HideChildren = 0;
			}

			XuiImageElementSetImagePath(m_PreviewImage.m_hObj, L"");

			XUIMessage xuiMsg;
			InRescanRomsStruct msgData;
			InRescanRomsFirstFunc( &xuiMsg, &msgData, NULL);
			XuiSendMessage( m_RomList.m_hObj, &xuiMsg );

			bHandled = TRUE;
			return S_OK;
		}
		else if (hObjPressed == m_PrevDevice)
		{
			CurrentFilter--;

			if (CurrentFilter < 0)
			{
				CurrentFilter = 14;
			}
			
			XUIMessage xuiMsg;
			InRescanRomsStruct msgData;
			InRescanRomsFirstFunc( &xuiMsg, &msgData, NULL);
			XuiSendMessage( m_RomList.m_hObj, &xuiMsg );
	
			swprintf_s(DeviceText, L"Current Hardware : %S", m_HardwareFilterDesc[CurrentFilter].c_str());
			m_DeviceText.SetText(DeviceText);

			swprintf_s(HardwareBGImage,L"%S",m_HardwareFilterBackGroundMap[CurrentFilter].c_str());			 			 
			XuiImageElementSetImagePath(m_SkinImage.m_hObj, HardwareBGImage);

			m_RomList.SetCurSel(0);		


			return S_OK;

		}
		else if (hObjPressed == m_NextDevice)
		{
			CurrentFilter++;

			if (CurrentFilter > 14)
			{
				CurrentFilter = 0;
			}

			//XuiImageElementSetImagePath(m_PreviewImage.m_hObj, L"");

			XUIMessage xuiMsg;
			InRescanRomsStruct msgData;
			InRescanRomsFirstFunc( &xuiMsg, &msgData, NULL);
			XuiSendMessage( m_RomList.m_hObj, &xuiMsg );
	
			swprintf_s(DeviceText, L"Current Hardware : %S", m_HardwareFilterDesc[CurrentFilter].c_str());
			m_DeviceText.SetText(DeviceText);

			swprintf_s(HardwareBGImage,L"%S",m_HardwareFilterBackGroundMap[CurrentFilter].c_str());			 			 
			XuiImageElementSetImagePath(m_SkinImage.m_hObj, HardwareBGImage);

			m_RomList.SetCurSel(0);			 

			return S_OK;

		}
	
        bHandled = TRUE;
        return S_OK;
    }


    //----------------------------------------------------------------------------------
    // Performs initialization tasks - retreives controls.
    //----------------------------------------------------------------------------------
HRESULT CRomListScene::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
    {
		
		wchar_t VersionText[60];
		wchar_t HardwareBGImage[60];
 
        // Retrieve controls for later use.
        GetChildById( L"XuiAddToFavorites", &m_AddToFavorites );
        GetChildById( L"XuiImage1", &m_SkinImage );
        GetChildById( L"XuiMainMenu", &m_Back );		 
		GetChildById( L"XuiRomList", &m_RomList );
		GetChildById( L"XuiRomPreview", &m_PreviewImage );	
		GetChildById( L"XuiRomTitle", &m_TitleImage );			 
		GetChildById( L"XuiBackVideoRoms", &m_BackVideo );
		GetChildById( L"XuiCurrentDeviceText", &m_DeviceText);
		GetChildById( L"XuiToggleHardwareNext", &m_NextDevice);
		GetChildById( L"XuiToggleHardwarePrev", &m_PrevDevice);
		GetChildById( L"XuiOptions", &m_ConsoleOutput);

		GetChildById( L"XuiGameInfo", &m_GameInfo);
		GetChildById( L"XuiRomName", &m_RomName);
		GetChildById( L"XuiROMInfo", &m_RomInfo);
		GetChildById( L"XuiReleasedBy", &m_ReleasedBy);
		GetChildById( L"XuiGenre", &m_Genre);

		GetChildById( L"XuiVersion", &m_Version);
		GetChildById( L"XuiNumberOfRoms", &m_NumberOfGames);
		GetChildById( L"XuiCheckboxHideChildren", &m_HideChildren);
		GetChildById( L"XuiCheckboxShow34PlayerOnly", &m_34PlayerOnly);
		GetChildById( L"XuiArcadeStick", &m_UseArcadeStickSettings);
		
 
		if (HideChildren==1)
		{
			m_HideChildren.SetCheck(true);
		}

		if (ThreeOrFourPlayerOnly==1)
		{
			m_34PlayerOnly.SetCheck(true);
		}

		if (ArcadeJoystick==1)
		{
			m_UseArcadeStickSettings.SetCheck(true);
		}

		m_RomList.DiscardResources(XUI_DISCARD_ALL);
		m_RomList.SetFocus();
		m_RomList.SetCurSel(0);
 
		swprintf_s(DeviceText, L"Current Hardware : All Games", szRomPath);
		m_DeviceText.SetText(DeviceText);
 
		hRomListScene = this->m_hObj;

		// build the hardware filter map
 
		m_HardwareFilterMap[ALL] = MASKALL;
		m_HardwareFilterMap[CPS1] = MASKCPS;
		m_HardwareFilterMap[CPS2] = MASKCPS2;
		m_HardwareFilterMap[CPS3] = MASKCPS3;
		m_HardwareFilterMap[NEOGEO] = MASKNEOGEO;
		m_HardwareFilterMap[TAITO] = MASKTAITO;
		m_HardwareFilterMap[SEGA] = MASKSEGA;
		m_HardwareFilterMap[PGM] = MASKPGM;
		m_HardwareFilterMap[PSYKIO] = MASKPSIKYO;
		m_HardwareFilterMap[KONAMI] = MASKKONAMI;
		m_HardwareFilterMap[KANEKO] = MASKKANEKO16;
		m_HardwareFilterMap[CAVE] = MASKCAVE;
		m_HardwareFilterMap[TOAPLAN] = MASKTOAPLAN;
		m_HardwareFilterMap[SEGAMD] = MASKMD;
		m_HardwareFilterMap[MISC] = MASKMISC;

		m_HardwareFilterDesc[ALL] = "All Hardware";
		m_HardwareFilterDesc[CPS1] = "Capcom CPS1";
		m_HardwareFilterDesc[CPS2] = "Capcom CPS2";
		m_HardwareFilterDesc[CPS3] = "Capcom CPS3";
		m_HardwareFilterDesc[NEOGEO] = "NeoGeo";
		m_HardwareFilterDesc[TAITO] = "Taito";
		m_HardwareFilterDesc[SEGA] = "Sega System 16";
		m_HardwareFilterDesc[PGM] = "PGM";
		m_HardwareFilterDesc[PSYKIO] = "Psykio";
		m_HardwareFilterDesc[KONAMI] = "Konami";
		m_HardwareFilterDesc[KANEKO] = "Kaneko 16";
		m_HardwareFilterDesc[CAVE] = "Cave";
		m_HardwareFilterDesc[TOAPLAN] = "Toaplan";
		m_HardwareFilterDesc[SEGAMD] = "Sega Megadrive";
		m_HardwareFilterDesc[MISC] = "Misc";

		m_HardwareFilterBackGroundMap[ALL] = "fbanext-bg-all.png";
		m_HardwareFilterBackGroundMap[CPS1] = "fbanext-bg-cps1.png";
		m_HardwareFilterBackGroundMap[CPS2] = "fbanext-bg-cps2.png";
		m_HardwareFilterBackGroundMap[CPS3] = "fbanext-bg-cps3.png";
		m_HardwareFilterBackGroundMap[NEOGEO] = "fbanext-bg-ng.png";
		m_HardwareFilterBackGroundMap[TAITO] = "fbanext-bg-tai.png";
		m_HardwareFilterBackGroundMap[SEGA] = "fbanext-bg-s16.png";
		m_HardwareFilterBackGroundMap[PGM] = "fbanext-bg-pgm.png";
		m_HardwareFilterBackGroundMap[PSYKIO] = "fbanext-bg-psi.png";
		m_HardwareFilterBackGroundMap[KONAMI] = "fbanext-bg-kon.png";
		m_HardwareFilterBackGroundMap[KANEKO] = "fbanext-bg-kan.png";
		m_HardwareFilterBackGroundMap[CAVE] = "fbanext-bg-cave.png";
		m_HardwareFilterBackGroundMap[TOAPLAN] = "fbanext-bg-toa.png";
		m_HardwareFilterBackGroundMap[SEGAMD] = "fbanext-bg-md.png";
		m_HardwareFilterBackGroundMap[MISC] = "fbanext-bg-misc.png";

		m_ListData.clear();

		swprintf(VersionText,L"%S",szAppBurnVer);
		m_Version.SetText(VersionText);
 
		CurrentFilter = nLastFilter;
		XuiImageElementSetImagePath(m_PreviewImage.m_hObj, L"");

		XUIMessage xuiMsg;
		InRescanRomsStruct msgData;
		InRescanRomsFirstFunc( &xuiMsg, &msgData, NULL);
		XuiSendMessage( m_RomList.m_hObj, &xuiMsg );

		swprintf_s(DeviceText, L"Current Hardware : %S", m_HardwareFilterDesc[CurrentFilter].c_str());
		m_DeviceText.SetText(DeviceText);

		swprintf_s(HardwareBGImage,L"%S",m_HardwareFilterBackGroundMap[CurrentFilter].c_str());			 		 
		XuiImageElementSetImagePath(m_SkinImage.m_hObj, HardwareBGImage);
 
		m_RomList.SetShow(true);
		m_RomList.SetFocus();		
		m_RomList.SetCurSel(nLastRom);
		m_RomList.SetCurSelVisible(nLastRom);

		bHandled = TRUE;
        return S_OK;
    }

CRomList::CRomList()
{
	
}
int CRomList::InitRomList()
{

	RomCount = 0;
	RomListOK = false;

	m_vecAvailRomList.clear();
	m_vecAvailRomIndex.clear();

	return 0;
} 

int CRomList::FreeRomList()
{
	m_vecAvailRomList.clear();
	m_vecAvailRomIndex.clear();

	return 0;
}

int CRomList::AvRoms()
{

	RomCount = m_vecAvailRomList.size();
	 
	return 0;
}

HRESULT CRomList::OnNotify( XUINotify *hObj, BOOL& bHandled )
{	
	static char szCurrentVideo[42];
	char szRequestedVideo[42];
	wchar_t videoPath[MAX_PATH];
	wchar_t previewPath[MAX_PATH];
	wchar_t titlePath[MAX_PATH];
	wchar_t InfoText[60];

	char VideoFName[MAX_PATH];
	char PreviewFName[MAX_PATH];
	char TitleFName[MAX_PATH];
	
	HXUIOBJ hPreviewImage;
	HXUIOBJ hTitleImage;

	HXUIOBJ hGameInfoText;
	HXUIOBJ hRomNameText;
	HXUIOBJ hRomInfoText;
	HXUIOBJ hReleasedText;
	HXUIOBJ hGenreText;
	HXUIOBJ hParentText;

	int nIndex = 0;
	switch(hObj->dwNotify)
	{
		case XN_SELCHANGED:			 						 
			nIndex = XuiListGetCurSel( this->m_hObj, NULL );
			
			if (strcmp((char *)m_vecAvailRomParent[nIndex].c_str(),"No Parent Rom") == 0)
			{
				strcpy(szRequestedVideo,GetFilenameWithoutExtension("",(char *)m_vecAvailRomIndex[nIndex].c_str()));				 
			}
			else
			{
				strcpy(szRequestedVideo,GetFilenameWithoutExtension("",(char *)m_vecAvailRomParent[nIndex].c_str()));				
			}

			strcpy(VideoFName, GetFilename("GAME:\\videos\\", szRequestedVideo, "wmv"));

			strcpy(PreviewFName, GetFilename("GAME:\\Previews\\", (char *)m_vecAvailRomIndex[nIndex].c_str(), "png"));
			strcpy(TitleFName, GetFilename("GAME:\\Titles\\", (char *)m_vecAvailRomIndex[nIndex].c_str(), "png"));			 

			XuiElementGetChildById( hRomListScene, 
                L"XuiVideoPreview", &hPreviewVideo );

			XuiElementGetChildById( hRomListScene, 
                L"XuiRomPreview", &hPreviewImage );

			XuiElementGetChildById( hRomListScene, 
                L"XuiRomTitle", &hTitleImage );

			XuiElementGetChildById( hRomListScene, 
                L"XuiGameInfo", &hGameInfoText );

			XuiElementGetChildById( hRomListScene, 
                L"XuiRomName", &hRomNameText );

			XuiElementGetChildById( hRomListScene, 
                L"XuiROMInfo", &hRomInfoText );

			XuiElementGetChildById( hRomListScene, 
                L"XuiReleasedBy", &hReleasedText );


			XuiElementGetChildById( hRomListScene, 
                L"XuiGenre", &hGenreText );

			XuiElementGetChildById( hRomListScene, 
                L"XuiParent", &hParentText );

			swprintf_s(InfoText,L"%S", m_vecAvailRomReleasedBy[nIndex].c_str() );
			XuiTextElementSetText(hReleasedText,InfoText);

			swprintf_s(InfoText,L"%S", m_vecAvailRomInfo[nIndex].c_str() );
			XuiTextElementSetText(hRomInfoText,InfoText);

			swprintf_s(InfoText,L"%S", m_vecAvailRomIndex[nIndex].c_str() );
			XuiTextElementSetText(hRomNameText,InfoText);

			swprintf_s(InfoText,L"%S", m_vecAvailRomParent[nIndex].c_str() );
			XuiTextElementSetText(hParentText,InfoText);

			swprintf_s(InfoText,L"%S", m_vecAvailRomManufacturer[nIndex].c_str() );
			XuiTextElementSetText(hGameInfoText,InfoText);

			if (strcmp(szCurrentVideo,szRequestedVideo) != 0)
			{
				XuiElementSetShow(hPreviewVideo, false);
				XuiVideoPause(hPreviewVideo, true);
				strcpy(szCurrentVideo, "");
			}
			
			if (GetFileAttributes(VideoFName) != -1 && (strcmp(szCurrentVideo, szRequestedVideo) != 0))
			{

				strcpy(szCurrentVideo,szRequestedVideo);

				XuiElementSetShow(hPreviewVideo, true);

				PlayLoopingMovie(VideoFName);
 
			}

			if (GetFileAttributes(PreviewFName) != -1)
			{
				string previewName(PreviewFName);				
				XuiElementSetShow(hPreviewImage, true);
				
	 
				swprintf_s(previewPath,L"file://%S", ReplaceCharInString("GAME:\\Previews\\", '\\', "/").c_str());
				//XuiElementDiscardResources(hPreviewImage, XUI_DISCARD_ALL);
				XuiElementSetBasePath(hPreviewImage, previewPath);
 
				previewName = ReplaceCharInString(previewName, '\\',"/");
				swprintf_s(previewPath,L"file://%S", previewName.c_str());
				XuiImageElementSetImagePath(hPreviewImage, previewPath);


				 
			}
			else
			{
				XuiElementSetShow(hPreviewImage, true);				
				XuiImageElementSetImagePath(hPreviewImage, L"no_title.png");
			}

			if (GetFileAttributes(TitleFName) != -1)
			{
				string titleName(TitleFName);

				titleName = ReplaceCharInString(titleName, '\\',"/");
				swprintf_s(titlePath,L"file://%S", titleName.c_str());
				XuiImageElementSetImagePath(hTitleImage, titlePath);


				swprintf_s(titlePath,L"file://%S", ReplaceCharInString("GAME:\\Title\\", '\\', "/").c_str());				 
				XuiElementSetBasePath(hTitleImage, titlePath);
 
			}
			else
			{				 
				XuiImageElementSetImagePath(hTitleImage, L"no_title.png");
			}

			break;
	}

	return S_OK;

}

HRESULT CRomList::PlayLoopingMovie(LPCSTR szFileName)
{
    HRESULT hr;

    IXAudio2* pXAudio2 = NULL;
    IXMedia2XmvPlayer* xmvPlayer = NULL;
    IXAudio2MasteringVoice* pMasterVoice = NULL;

    // Initialize the IXAudio2 and IXAudio2MasteringVoice interfaces.
    if ( FAILED(hr = XAudio2Create( &pXAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR ) ) )
    {
        return S_FALSE;
    }

    if ( FAILED(hr = pXAudio2->CreateMasteringVoice( &pMasterVoice, XAUDIO2_DEFAULT_CHANNELS,
                            XAUDIO2_DEFAULT_SAMPLERATE, 0, 0, NULL ) ) )
    {
        return S_FALSE;
    }


    // Create a new XMEDIA_XMV_CREATE_PARAMETERS struct.
    XMEDIA_XMV_CREATE_PARAMETERS parameters;
    ZeroMemory(&parameters, sizeof (parameters));

    // Specify that the movie should be created from a file
    // and provide the file location.
    parameters.createType = XMEDIA_CREATE_FROM_FILE;
    parameters.createFromFile.szFileName = szFileName;
	parameters.createFromFile.dwIoBlockSize = 8192 << 1;

    // Specify that this should be a looping movie.
    parameters.dwFlags = XMEDIA_CREATE_FOR_LOOP | XMEDIA_CREATE_BACKGROUND_MUSIC;
    
    // Create the player using the XMEDIA_XMV_CREATE_PARAMETERS struct.
    if( FAILED (hr = XMedia2CreateXmvPlayer(pDevice, pXAudio2, &parameters, &xmvPlayer)))
    {
        return S_FALSE;
    }
	
	XuiVideoSetPlayer(hPreviewVideo,xmvPlayer);
	XuiVideoSetVolume(hPreviewVideo,-20.0);

	pDevice->SetRenderState( D3DRS_VIEWPORTENABLE, FALSE );
	pDevice->SetRenderState( D3DRS_VIEWPORTENABLE, TRUE );

    // Release the audio objects
    if(pXAudio2)
    {
        pXAudio2->Release();
        pXAudio2 = 0;
    }

    // Release the movie object
    if(xmvPlayer)
    {
        xmvPlayer->Release();
        xmvPlayer = 0;
    }

    return hr;
}
 

HRESULT CRomList::OnRescanRoms( char *szPath,  BOOL& bHandled )
{ 
	HXUIOBJ hRomCountText;
	bool IsFiltered = false;
	wchar_t RomCountText[60];
	std::vector<std::string> vecTempRomList;
	std::vector<std::string> vecAvailRomListFileName;
	std::vector<std::string> vecAvailRomList;
	std::vector<int>		 vecAvailRomIndex;

	InitRomList();

	DeleteItems(0, m_vecAvailRomList.size());

  
	m_vecAvailRomList.clear();
	m_vecAvailRomReleasedBy.clear();
	m_vecAvailRomInfo.clear();
	m_vecAvailRomParent.clear();
	m_vecAvailRomManufacturer.clear();

	if (m_ListData.empty())
	{
		for (int d = 0; d < DIRS_MAX; d++) 
		{
                
			if (!_tcsicmp(szAppRomPaths[d], _T(""))) {
					continue; // skip empty path
			}

			strcpy((char *)szRoms, szAppRomPaths[d]);
			strcat(szRoms, "*.*");

			HANDLE hFind;	
			WIN32_FIND_DATAA oFindData;

			hFind = FindFirstFile(szRoms, &oFindData);

			if (hFind != INVALID_HANDLE_VALUE)
			{
				do
				{					
					m_ListData.push_back(_strlwr(oFindData.cFileName));			 

				} while (FindNextFile(hFind, &oFindData));	
			}
		}

		std::sort(m_ListData.begin(), m_ListData.end());
	}
	 
	int tempgame;
	tempgame = nBurnDrvSelect;
 
	InitRomList();

	// Now build a vector of Burn Roms
	for (unsigned int i = 0; i < nBurnDrvCount-1; i++) {
		nBurnDrvSelect = i;	
		char *szName;
	 
		if (BurnDrvGetArchiveName(&szName, 0)) {
		 
		}
 
		vecAvailRomListFileName.push_back(szName);
		vecAvailRomList.push_back(BurnDrvGetTextA(DRV_FULLNAME));
		vecAvailRomIndex.push_back(i);
 
	}

	// For each *.zip we have, see if there is a matching burn rom
	// if so add it to the m_vec members and we are done.

	for (unsigned int x = 0; x < vecAvailRomListFileName.size(); x++) {
		for (unsigned int y = 0; y < m_ListData.size(); y++) {
			if (m_ListData[y] == vecAvailRomListFileName[x]) {
				nBurnDrvSelect = vecAvailRomIndex[x];
 
				const int nHardware = 1 << (BurnDrvGetHardwareCode() >> 24);

				if (CurrentFilter > 0)
				{
					IsFiltered = (nHardware) == m_HardwareFilterMap[CurrentFilter];
				}
				else
				{
					IsFiltered = true;
				}

#if defined (FBA_DEBUG)
				if ((IsFiltered))  // skip roms marked as not working
#else
				if (BurnDrvIsWorking() && (IsFiltered))  // skip roms marked as not working
#endif
				{

				    int nNumPlayers = BurnDrvGetMaxPlayers();

					if ((HideChildren == 1 && (BurnDrvGetTextA(DRV_PARENT) == NULL && !(BurnDrvGetFlags() & BDF_CLONE))) ||
						(HideChildren == 1 && (BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_CAPCOM_CPS3) ||
						(HideChildren == 0))
					{
						  
						if ((ThreeOrFourPlayerOnly == 1 && nNumPlayers > 2) || ThreeOrFourPlayerOnly == 0)
						{
							m_vecAvailRomIndex.push_back(vecAvailRomListFileName[x]);
							m_vecAvailRomList.push_back(BurnDrvGetTextA(DRV_FULLNAME));
							m_vecAvailRomReleasedBy.push_back(BurnDrvGetTextA(DRV_MANUFACTURER));		

							if (BurnDrvGetTextA(DRV_SYSTEM))
							{
								m_vecAvailRomManufacturer.push_back(BurnDrvGetTextA(DRV_SYSTEM));
							}
							else
							{
								m_vecAvailRomManufacturer.push_back("Unknown");
							}

							if (BurnDrvGetTextA(DRV_COMMENT))
							{
								m_vecAvailRomInfo.push_back(BurnDrvGetTextA(DRV_COMMENT));
							}
							else
							{
								m_vecAvailRomInfo.push_back("No Additional Information");
							}

							if (BurnDrvGetTextA(DRV_PARENT))
							{
								m_vecAvailRomParent.push_back(BurnDrvGetTextA(DRV_PARENT));
							}
							else
							{
								m_vecAvailRomParent.push_back("No Parent Rom");
							}
						}
					}
					
				}
				break;
			}

		}

	}

	AvRoms();
	nBurnDrvSelect = tempgame;
	InsertItems( 0, RomCount );
 
	XuiElementGetChildById( hRomListScene, L"XuiNumberOfRoms", &hRomCountText );

	if (CurrentFilter > 0 || HideChildren || ThreeOrFourPlayerOnly)
	{
		swprintf(RomCountText,L"%d/%d Games Found (Filtered)",RomCount, nBurnDrvCount);
	}
	 
	else
	{
		swprintf(RomCountText,L"%d/%d Games Found",RomCount, nBurnDrvCount);
	}

	XuiTextElementSetText(hRomCountText,RomCountText);

	//m_NumberOfGames.SetText(RomCountText);
 
	bHandled = TRUE;	
    return( S_OK );
}

HRESULT CRomList::OnInit(XUIMessageInit *pInitData, BOOL& bHandled)
{
 
	XUIMessage xuiMsg;
	InRescanRomsStruct msgData;
	InRescanRomsFirstFunc( &xuiMsg, &msgData,NULL );
	XuiSendMessage( m_hObj, &xuiMsg );

	bHandled = TRUE;
    return S_OK;
}

HRESULT CRomList::OnGetItemCountAll(
        XUIMessageGetItemCount *pGetItemCountData, 
        BOOL& bHandled)
    {
        pGetItemCountData->cItems = RomCount;
        bHandled = TRUE;
        return S_OK;
    }


// Gets called every frame
HRESULT CRomList::OnGetSourceDataText(
    XUIMessageGetSourceText *pGetSourceTextData, 
    BOOL& bHandled)
{
	
    
    if( ( 0 == pGetSourceTextData->iData ) && ( ( pGetSourceTextData->bItemData ) ) ) {

			LPCWSTR lpszwBuffer = MultiCharToUniChar((char *)m_vecAvailRomList[pGetSourceTextData->iItem].c_str());

            pGetSourceTextData->szText = lpszwBuffer;

            bHandled = TRUE;
        }
        return S_OK;

}


std::string ReplaceCharInString(  
    const std::string & source, 
    char charToReplace, 
    const std::string replaceString 
    ) 
{ 
    std::string result; 
 
    // For each character in source string: 
    const char * pch = source.c_str(); 
    while ( *pch != '\0' ) 
    { 
        // Found character to be replaced? 
        if ( *pch == charToReplace ) 
        { 
            result += replaceString; 
        } 
        else 
        { 
            // Just copy original character 
            result += (*pch); 
        } 
 
        // Move to next character 
        ++pch; 
    } 
 
    return result; 
} 

