// FB Alpha - Emulator for MC68000/Z80 based arcade games
//            Refer to the "license.txt" file for more info

// Main module

//#define APP_DEBUG_LOG			// log debug messages to zzBurnDebug.html
#include "main.h"
#include "burner.h"
#include "Splash.h"
#include "RomList.h"
#include "InGameOptions.h"
#include "version.h"

BOOL IsCurrentlyInGame = false;
HXUIOBJ hMainScene;
HINSTANCE hAppInst = NULL;		// Application Instance
HANDLE hMainThread;
int nAppThreadPriority = THREAD_PRIORITY_NORMAL;
int nAppShowCmd;

//HACCEL hAccel = NULL;
TCHAR szAppBurnVer[16] = _T("");
TCHAR szAppExeName[EXE_NAME_SIZE + 1];

bool bCmdOptUsed = 0;
bool bAlwaysProcessKey = false;
static HMODULE hModRich20A = NULL;

int ArcadeJoystick = 0;

// Used for the load/save dialog in commdlg.h (savestates, input replay, wave logging)
TCHAR szChoice[MAX_PATH] = _T("");
 
int Mounted[20];
#if defined (FBA_DEBUG)
 static TCHAR szConsoleBuffer[1024];
 static int nPrevConsoleStatus = -1;

 static HANDLE DebugBuffer;
 static FILE* DebugLog = NULL;
 static bool bEchoLog = true; // false;
#endif

extern HXUIOBJ hRomListScene;


HRESULT CBurnApp::RegisterXuiClasses()
{	
    // We must register the video control classes
	XuiVideoRegister();
	XuiHtmlRegister();  
	XuiSoundXAudioRegister();	 
	CRomListScene::Register();
	CRomList::Register();
	CSplashScene::Register();
	CInGameOptions::Register();
	CFilterOptions::Register();
	CDipOptions::Register();
	CDipList::Register();
	CDipSettingsList::Register();
	CInputOptions::Register();
	CInputList::Register();
	CInputSettingsList::Register();
	COtherOptions::Register();
 
    // Register any other classes necessary for the app/scene
    return S_OK;
}

//--------------------------------------------------------------------------------------
// Name: UnregisterXuiClasses
// Desc: Unregisters all the scene classes.
//--------------------------------------------------------------------------------------
HRESULT CBurnApp::UnregisterXuiClasses()
{
	CRomListScene::Unregister();
	CRomList::Unregister();
	CSplashScene::Unregister();
	CInGameOptions::Unregister();
	CFilterOptions::Unregister();
	CDipOptions::Unregister();
	CDipList::Unregister();
	CDipSettingsList::Unregister();
	CInputOptions::Unregister();
	CInputList::Unregister();
	CInputSettingsList::Unregister();
	COtherOptions::Unregister();
    return S_OK;
}

// Global XUI Stuff
CBurnApp app;
IDirect3DDevice9 *pDevice;
IDirect3D9 *pD3D;
D3DPRESENT_PARAMETERS d3dpp;

HRESULT InitD3D( IDirect3DDevice9 **ppDevice, 
    D3DPRESENT_PARAMETERS *pd3dPP )
{
    
	// Get the user video settings.
	XVIDEO_MODE VideoMode; 
	XMemSet( &VideoMode, 0, sizeof(XVIDEO_MODE) ); 
	XGetVideoMode( &VideoMode );

    pD3D = Direct3DCreate9( D3D_SDK_VERSION );

    // Set up the presentation parameters.
    ZeroMemory( pd3dPP, sizeof( D3DPRESENT_PARAMETERS ) );

	if (!VideoMode.fIsWideScreen)
	{
		pd3dPP->Flags |=  D3DPRESENTFLAG_NO_LETTERBOX;
	}
    pd3dPP->BackBufferWidth        = 1280;
    pd3dPP->BackBufferHeight       = 720;
    pd3dPP->BackBufferFormat       = D3DFMT_X8R8G8B8; 
    pd3dPP->PresentationInterval   = D3DPRESENT_INTERVAL_ONE;
 
    // Create the device.
    return pD3D->CreateDevice(
                    0, 
                    D3DDEVTYPE_HAL,
                    NULL,
                    D3DCREATE_FPU_PRESERVE,
                    pd3dPP,
                    ppDevice );
}


// Debug printf to a file
static int __cdecl AppDebugPrintf(int nStatus, TCHAR* pszFormat, ...)
{
#if defined (FBA_DEBUG)
 
#endif

	return 0;
}



int dprintf(TCHAR* pszFormat, ...)
{
#if defined (FBA_DEBUG)
	 
#else
	(void)pszFormat;
#endif

	return 0;
}

void CloseDebugLog()
{
#if defined (FBA_DEBUG)
 
#endif
}

int OpenDebugLog()
{
 
	return 0;
}

void DebugMsg(const char* format, ...)
{
    char buffer[256];
	std::ofstream writeLog;

	va_list ap;
	va_start (ap, format);
	vsprintf_s(buffer, sizeof(buffer), format, ap);
	va_end (ap);

	//always write to file
	writeLog.open("game:\\debug.log",std::ofstream::app);
	if (writeLog.is_open())
	{
		writeLog.write(buffer, strlen(buffer));
		writeLog.write("\n",1);
	}
	writeLog.close();

}



int DriveMounted(string path)
{
	WIN32_FIND_DATA findFileData;
	memset(&findFileData,0,sizeof(WIN32_FIND_DATA));
	string searchcmd = path + "\\*.*";
	//debugLog(searchcmd.c_str());
	HANDLE hFind = FindFirstFile(searchcmd.c_str(), &findFileData);
	if (hFind == INVALID_HANDLE_VALUE)
	{
		return 0;
	}
	FindClose(hFind);

	return 1;
}

HRESULT Mount( int Device, char* MountPoint )
{
	char MountConv[260];
	sprintf_s( MountConv,"\\??\\%s", MountPoint );

	char * SysPath = NULL;
	switch( Device )
	{
		case DEVICE_MEMORY_UNIT0:
			SysPath = "\\Device\\Mu0";
			break;
		case DEVICE_MEMORY_UNIT1:
			SysPath = "\\Device\\Mu1";
			break;
		case DEVICE_MEMORY_ONBOARD:
			SysPath = "\\Device\\BuiltInMuSfc";
			break;
		case DEVICE_CDROM0:
			SysPath = "\\Device\\Cdrom0";
			break;
		case DEVICE_HARDISK0_PART1:
			SysPath = "\\Device\\Harddisk0\\Partition1";
			break;
		case DEVICE_HARDISK0_SYSPART:
			SysPath = "\\Device\\Harddisk0\\SystemPartition";
			break;
		case DEVICE_USB0:
			SysPath = "\\Device\\Mass0";
			break;
		case DEVICE_USB1:
			SysPath = "\\Device\\Mass1";
			break;
		case DEVICE_USB2:
			SysPath = "\\Device\\Mass2";
			break;
		case DEVICE_CACHE:
			SysPath = "\\Device\\Harddisk0\\Cache0";
			break;
//		case DEVICE_TEST:
//			SysPath = "\\Device\\";
//			break;
	}

	STRING sSysPath = { (USHORT)strlen( SysPath ), (USHORT)strlen( SysPath ) + 1, SysPath };
	STRING sMountConv = { (USHORT)strlen( MountConv ), (USHORT)strlen( MountConv ) + 1, MountConv };
	int res = ObCreateSymbolicLink( &sMountConv, &sSysPath );

	if (res != 0)
		return res;

	return DriveMounted(MountPoint);
}

HRESULT RenderGame( IDirect3DDevice9 *pDevice )
	{
    // Render game graphics.
    pDevice->Clear(
        0,
        NULL,
        D3DCLEAR_TARGET | D3DCLEAR_STENCIL | D3DCLEAR_ZBUFFER,
        D3DCOLOR_ARGB( 255, 0, 0, 0 ),
        1.0,
        0 );

    return S_OK;
}


void MountAll()
{
	memset(&Mounted,0,20);

 	Mounted[DEVICE_USB0] = Mount(DEVICE_USB0,"Usb0:");
 	Mounted[DEVICE_USB1] = Mount(DEVICE_USB1,"Usb1:");
 	Mounted[DEVICE_USB2] = Mount(DEVICE_USB2,"Usb2:");
 	Mounted[DEVICE_HARDISK0_PART1] = Mount(DEVICE_HARDISK0_PART1,"Hdd1:");
 	Mounted[DEVICE_HARDISK0_SYSPART] = Mount(DEVICE_HARDISK0_SYSPART,"HddX:");
 	Mounted[DEVICE_MEMORY_UNIT0] = Mount(DEVICE_MEMORY_UNIT0,"Memunit0:");
 	Mounted[DEVICE_MEMORY_UNIT1] = Mount(DEVICE_MEMORY_UNIT1,"Memunit1:");
	Mounted[DEVICE_MEMORY_ONBOARD] = Mount(DEVICE_MEMORY_ONBOARD,"OnBoardMU:"); 
	Mounted[DEVICE_CDROM0] = Mount(DEVICE_CDROM0,"Dvd:"); 
}

static int AppInit()
{
 
#if defined (_MSC_VER) && defined (_DEBUG)
	// Check for memory corruption
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
#endif

	OpenDebugLog();
 
	// Load config for the application
	configAppLoadXml();

	setWindowAspect();
 
	SplashCreate();


#if 1 || !defined (FBA_DEBUG)
	// print a warning if we're running for the 1st time
	if (nIniVersion < nBurnVer) {
		scrnInit();
		//FirstUsageCreate(hScrnWnd);
		configAppSaveXml();		// Create initial config file
	}
#endif

	// Set the thread priority for the main thread
	SetThreadPriority(GetCurrentThread(), nAppThreadPriority);

	// Init the Burn library
	BurnLibInit();

	if (VidSelect(nVidSelect)) {
		nVidSelect = VID_D3D;
		VidSelect(nVidSelect);
	}

	if (audio.select(audSelect)) {
		audio.select(_T("XAudio2"));
	}
 	
	// Build the ROM information

	// Get romset information
	//getAllRomsetInfo();
	
	nVidFullscreen = 1;
	
	BurnExtLoadOneRom = archiveLoadOneFile;
 
	return 0;
}

static int AppExit()
{
	BurnerDrvExit();				// Make sure any game driver is exitted
	mediaExit();					// Exit media
	scrnExit();						// Exit the screen window
	BurnLibExit();					// Exit the Burn library

	freeAuditState();				// Free audit state 
	auditCleanup();					// Free audit info
 
	FBALocaliseExit();
 
 
	CloseHandle(hMainThread);

	CloseDebugLog();

	return 0;
}

void AppCleanup()
{
//	StopReplay();
//	waveLogStop();
//	AviStop();

	AppExit();
	strConvClean();
}

 

bool AppProcessKeyboardInput()
{
                                                                                                                                                              
	return true;
}

// Main program entry point
int WINAPI main()
{
	HRESULT hr;
	DWORD dwPlayerIndex = 0;
	XINPUT_CAPABILITIES Capabilities;
	
	LPSTR str = GetCommandLine();

	MountAll();
	XSetFileCacheSize(0x100000);
	XMountUtilityDriveEx(XMOUNTUTILITYDRIVE_FORMAT0,8192, 0);
	// Provide a custom exception handler
	SetUnhandledExceptionFilter(ExceptionFilter);
 
	// Make version string
	if (nBurnVer & 0xFF) {
		// private version (alpha)
		_stprintf(szAppBurnVer, _T("%x.%x.%x.%02x"), nBurnVer >> 20, (nBurnVer >> 16) & 0x0F, (nBurnVer >> 8) & 0xFF, nBurnVer & 0xFF);
	} else {
		// public version
		_stprintf(szAppBurnVer, _T("%x.%x.%x"), nBurnVer >> 20, (nBurnVer >> 16) & 0x0F, (nBurnVer >> 8) & 0xFF);
	}
 
    // Initialize D3D
    hr = InitD3D( &pDevice, &d3dpp );
 
    // Initialize the application.    
	 
    hr = app.InitShared( pDevice, &d3dpp, 
        XuiD3DXTextureLoader );
 
    if( FAILED( hr ) )
    { 
        OutputDebugString( "Failed intializing application.\n" );
        return 0;
    }

    // Register a default typeface
    hr = app.RegisterDefaultTypeface( L"Arial Unicode MS", L"file://game:/media/fba.ttf" );
	
    if( FAILED( hr ) )
    {
        OutputDebugString( "Failed to register default typeface.\n" );
        return 0;
    }

 
	appDirectory();					// Set current directory to be the applications directory

	createNeedDir();				// Make sure there are roms and cfg subdirectories

	AppInit();

//#define PGO_PROFILE 
#if defined (PGO_PROFILE)

	directLoadGame("rtypeleo.zip");
	mediaInit();
	RunMessageLoop();

#else
 
    // Load the skin file used for the scene.
	app.LoadSkin( L"file://game:/media/FBNext.xzp#src\\Interface\\Xbox360\\Skin\\skin.xur" );     	
	XuiSceneCreate( L"file://game:/media/FBNext.xzp#src\\Interface\\Xbox360\\Skin\\", L"RomList.xur", NULL, &hMainScene );	
	XuiSceneNavigateFirst( app.GetRootObj(), hMainScene, XUSER_INDEX_FOCUS ); 
 
    while( TRUE ) { 
 
		if (XInputGetCapabilities(dwPlayerIndex, XINPUT_FLAG_GAMEPAD, 
		&Capabilities) == ERROR_SUCCESS)
		{
			if ((Capabilities.Type == XINPUT_DEVTYPE_GAMEPAD) 
				&& (Capabilities.SubType == XINPUT_DEVSUBTYPE_ARCADE_STICK ))
			{
				ArcadeJoystick = 1;
			}
		}
  
		RenderGame( pDevice );
 
		// Update XUI
		app.RunFrame();

		// Render XUI
		hr = app.Render();

		// Update XUI Timers
		hr = XuiTimersRun();
 
		// Present the frame.
		pDevice->Present( NULL, NULL, NULL, NULL );
		
    }
#endif
 
	AppExit();						// Exit the application
	configAppSaveXml();				// Save config for the application
	strConvClean();					// Free string conversion
	return 0;
}

void ExceptionToDebug(struct _EXCEPTION_POINTERS *lpExceptionInfo)
{
	string description = "";
	string exceptionname = "";
	switch(lpExceptionInfo->ExceptionRecord->ExceptionCode)
	{
	case EXCEPTION_ACCESS_VIOLATION :
		exceptionname = "EXCEPTION_ACCESS_VIOLATION";
		description = "The thread tried to read from or write to a virtual address for which it does not have the appropriate access.";
		break;
	case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
		description = "The thread tried to access an array element that is out of bounds and the underlying hardware supports bounds checking.";
		exceptionname = "EXCEPTION_ARRAY_BOUNDS_EXCEEDED";
		break;
	case EXCEPTION_BREAKPOINT :
		description = "A breakpoint was encountered. ";
		exceptionname = "EXCEPTION_BREAKPOINT";
		break;
	case EXCEPTION_DATATYPE_MISALIGNMENT :
		description = "The thread tried to read or write data that is misaligned on hardware that does not provide alignment. For example, 16-bit values must be aligned on 2-byte boundaries; 32-bit values on 4-byte boundaries, and so on. ";
		exceptionname = "EXCEPTION_DATATYPE_MISALIGNMENT";
		break;
	case EXCEPTION_FLT_DENORMAL_OPERAND :
		description = "One of the operands in a floating-point operation is denormal. A denormal value is one that is too small to represent as a standard floating-point value. ";
		exceptionname = "EXCEPTION_FLT_DENORMAL_OPERAND";
		break;
	case EXCEPTION_FLT_DIVIDE_BY_ZERO :
		description = "The thread tried to divide a floating-point value by a floating-point divisor of zero. ";
		exceptionname = "EXCEPTION_FLT_DIVIDE_BY_ZERO";
		break;
	case EXCEPTION_FLT_INEXACT_RESULT :
		description="The result of a floating-point operation cannot be represented exactly as a decimal fraction. ";
		exceptionname ="EXCEPTION_FLT_INEXACT_RESULT";
		break;
	case EXCEPTION_FLT_INVALID_OPERATION :
		description = "This exception represents any floating-point exception not included in this list. ";
		exceptionname = "EXCEPTION_FLT_INVALID_OPERATION";
		break;
	case EXCEPTION_FLT_OVERFLOW :
		description = "The exponent of a floating-point operation is greater than the magnitude allowed by the corresponding type. ";
		exceptionname = "EXCEPTION_FLT_OVERFLOW";
		break;
	case EXCEPTION_FLT_STACK_CHECK:
		description = "The stack overflowed or underflowed as the result of a floating-point operation. ";
		exceptionname = "EXCEPTION_FLT_STACK_CHECK";
		break;
	case EXCEPTION_FLT_UNDERFLOW :
		description = "The exponent of a floating-point operation is less than the magnitude allowed by the corresponding type. ";
		exceptionname = "EXCEPTION_FLT_UNDERFLOW";
		break;
	case EXCEPTION_GUARD_PAGE :
		description = "The thread attempted to read from or write to a guard page. ";
		exceptionname= "EXCEPTION_GUARD_PAGE";
		break;
	case EXCEPTION_ILLEGAL_INSTRUCTION :
		description="The thread tried to execute an invalid instruction. ";
		exceptionname = "EXCEPTION_ILLEGAL_INSTRUCTION";
		break;
	case EXCEPTION_IN_PAGE_ERROR:
		description= "The thread tried to access a page that was not present, and the system was unable to load the page. ";
		exceptionname = "EXCEPTION_IN_PAGE_ERROR";
		break;
	case EXCEPTION_INT_DIVIDE_BY_ZERO :
		description = "The thread tried to divide an integer value by an integer divisor of zero. ";
		exceptionname = "EXCEPTION_INT_DIVIDE_BY_ZERO";
		break;
	case EXCEPTION_INT_OVERFLOW :
		description = "The result of an integer operation caused a carry out of the most significant bit of the result. ";
		exceptionname = "EXCEPTION_INT_OVERFLOW";
		break;
	case EXCEPTION_INVALID_DISPOSITION:
		description = "An exception handler returned an invalid disposition to the exception dispatcher. Programmers using a high-level language such as C should never encounter this exception. ";
		exceptionname = "EXCEPTION_INVALID_DISPOSITION";
		break;
	case EXCEPTION_INVALID_HANDLE:
		description = "The thread tried to perform an operation using an invalid handle. ";
		exceptionname = "EXCEPTION_INVALID_HANDLE";
		break;
	case EXCEPTION_NONCONTINUABLE_EXCEPTION :
		description = "The thread tried to continue execution after a noncontinuable exception occurred. ";
		exceptionname = "EXCEPTION_NONCONTINUABLE_EXCEPTION";
		break;
	case EXCEPTION_PRIV_INSTRUCTION :
		description = "The thread tried to execute a privileged instruction. ";
		exceptionname = "EXCEPTION_PRIV_INSTRUCTION";
		break;
	case EXCEPTION_SINGLE_STEP :
		description = "A trace trap or other single-instruction mechanism signaled that one instruction has been executed. ";
		exceptionname = "EXCEPTION_SINGLE_STEP";
		break;
	case EXCEPTION_STACK_OVERFLOW :
		description = "A stack overflow occurred. ";
		exceptionname = "EXCEPTION_STACK_OVERFLOW";
		break;
	default :
		description ="Unknow Exception";
		exceptionname = "Unknow Exception";
		break;

	}
	DebugMsg(exceptionname.c_str());
	DebugMsg(description.c_str());
	
}


// changed this based on http://helldoc.blogspot.com/2010/04/development-on-retail-360.html
LONG CALLBACK ExceptionFilter(_EXCEPTION_POINTERS* pExceptionInfo)
{
	ExceptionToDebug(pExceptionInfo);


	return 0;
}


int ProcessCmdLine()
{


	return 0;
}

int ProgressUpdateBurner(double dProgress, const TCHAR* pszText, bool bAbs)
{
	wchar_t OutputText[255];

	swprintf_s(OutputText, L"%S", pszText);
	HXUIOBJ hLabel;
	XuiElementGetChildById( hMainScene, L"XuiOptions", &hLabel );
	XuiTextElementSetText( hLabel, OutputText );
	RenderGame( pDevice );
	// Update XUI
	app.RunFrame();
	// Render XUI
	app.Render();
	// Update XUI Timers
	XuiTimersRun();
	// Present the frame.
	pDevice->Present( NULL, NULL, NULL, NULL );

	return 0;

}

void UpdateConsole(char *text)
{
	wchar_t OutputText[255];

	swprintf_s(OutputText, L"%S", text);
	HXUIOBJ hLabel;
	XuiElementGetChildById( hMainScene, L"XuiOptions", &hLabel );
	XuiTextElementSetText( hLabel, OutputText );
	RenderGame( pDevice );
	// Update XUI
	app.RunFrame();
	// Render XUI
	app.Render();
	// Update XUI Timers
	XuiTimersRun();
	// Present the frame.
	pDevice->Present( NULL, NULL, NULL, NULL );
}

int SplashCreate(void)
{

   // Load the skin file used for the scene.
	app.LoadSkin( L"file://game:/media/FBNext.xzp#src\\Interface\\Xbox360\\Skin\\skin.xur" );     	
	XuiSceneCreate( L"file://game:/media/FBNext.xzp#src\\Interface\\Xbox360\\Skin\\", L"Splash.xur", NULL, &hMainScene );	
	XuiSceneNavigateFirst( app.GetRootObj(), hMainScene, XUSER_INDEX_FOCUS ); 	 

	if (!fileExists("GAME:\\runonce.dat"))
	{
		while(!IsCurrentlyInGame)
		{

			 			 
				// Render game graphics.
	 
				RenderGame( pDevice );
	  	 
				// Update XUI
				app.RunFrame();

				// Render XUI
				app.Render();

				// Update XUI Timers
				XuiTimersRun();

				// Present the frame.
				pDevice->Present( NULL, NULL, NULL, NULL );
			 
		}

		HANDLE hFile = CreateFile("GAME:\\runonce.dat", GENERIC_WRITE, FILE_SHARE_WRITE,
                   NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

		CloseHandle(hFile);
	}
 
	return 0;

}