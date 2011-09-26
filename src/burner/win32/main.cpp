// FB Alpha - Emulator for MC68000/Z80 based arcade games
//            Refer to the "license.txt" file for more info

// Main module

//#define APP_DEBUG_LOG			// log debug messages to zzBurnDebug.html

#include "burner.h"
#include "cmd_line.h"
#include "imageload+.h"
#include "imageresize.h"
#include "maphkeys.h"

#ifdef _MSC_VER
 #ifdef _DEBUG
  #define _CRTDBG_MAP_ALLOC
  #include <crtdbg.h>
 #endif

 #pragma comment(lib, "comctl32")
 #pragma comment(lib, "winmm")

#if 0
 #if defined _M_IX86
  #pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
 #elif defined _M_IA64
  #pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
 #elif defined _M_X64
  #pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
 #else
  #pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
 #endif
#endif
#endif

#include "version.h"

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

// Used for the load/save dialog in commdlg.h (savestates, input replay, wave logging)
TCHAR szChoice[MAX_PATH] = _T("");
OPENFILENAME ofn;

#if defined (FBA_DEBUG)
 static TCHAR szConsoleBuffer[1024];
 static int nPrevConsoleStatus = -1;

 static HANDLE DebugBuffer;
 static FILE* DebugLog = NULL;
 static bool bEchoLog = true; // false;
#endif

// Debug printf to a file
static int __cdecl AppDebugPrintf(int nStatus, TCHAR* pszFormat, ...)
{
#if defined (FBA_DEBUG)
	va_list vaFormat;

	va_start(vaFormat, pszFormat);

	if (DebugLog) {

		if (nStatus != nPrevConsoleStatus) {
			switch (nStatus) {
				case PRINT_ERROR:
					_ftprintf(DebugLog, _T("</font><font color=#FF3F3F>"));
					break;
				case PRINT_IMPORTANT:
					_ftprintf(DebugLog, _T("</font><font color=#000000>"));
					break;
				default:
					_ftprintf(DebugLog, _T("</font><font color=#009F00>"));
			}
		}
		_vftprintf(DebugLog, pszFormat, vaFormat);
		fflush(DebugLog);
	}

	if (!DebugLog || bEchoLog) {
		_vsntprintf(szConsoleBuffer, 1024, pszFormat, vaFormat);

		if (nStatus != nPrevConsoleStatus) {
			switch (nStatus) {
				case PRINT_UI:
					SetConsoleTextAttribute(DebugBuffer,                                                       FOREGROUND_INTENSITY);
					break;
				case PRINT_IMPORTANT:
					SetConsoleTextAttribute(DebugBuffer, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
					break;
				case PRINT_ERROR:
					SetConsoleTextAttribute(DebugBuffer, FOREGROUND_RED | FOREGROUND_GREEN |                   FOREGROUND_INTENSITY);
					break;
				default:
					SetConsoleTextAttribute(DebugBuffer,                  FOREGROUND_GREEN |                   FOREGROUND_INTENSITY);
			}
		}

		WriteConsole(DebugBuffer, szConsoleBuffer, _tcslen(szConsoleBuffer), NULL, NULL);
	}

	nPrevConsoleStatus = nStatus;

	va_end(vaFormat);
#endif

	return 0;
}

int dprintf(TCHAR* pszFormat, ...)
{
#if defined (FBA_DEBUG)
	va_list vaFormat;
	va_start(vaFormat, pszFormat);

	_vsntprintf(szConsoleBuffer, 1024, pszFormat, vaFormat);

	if (nPrevConsoleStatus != PRINT_UI) {
		if (DebugLog) {
			_ftprintf(DebugLog, _T("</font><font color=#9F9F9F>"));
		}
		SetConsoleTextAttribute(DebugBuffer, FOREGROUND_INTENSITY);
		nPrevConsoleStatus = PRINT_UI;
	}

	if (DebugLog) {
		_ftprintf(DebugLog, szConsoleBuffer);
		fflush(DebugLog);
	}
	WriteConsole(DebugBuffer, szConsoleBuffer, _tcslen(szConsoleBuffer), NULL, NULL);
	va_end(vaFormat);
#else
	(void)pszFormat;
#endif

	return 0;
}

void CloseDebugLog()
{
#if defined (FBA_DEBUG)
	if (DebugLog) {

		_ftprintf(DebugLog, _T("</pre></body></html>"));

		fclose(DebugLog);
		DebugLog = NULL;
	}

	if (DebugBuffer) {
		CloseHandle(DebugBuffer);
		DebugBuffer = NULL;
	}

	FreeConsole();
#endif
}

int OpenDebugLog()
{
#if defined (FBA_DEBUG)
 #if defined (APP_DEBUG_LOG)

    time_t nTime;
	tm* tmTime;

	time(&nTime);
	tmTime = localtime(&nTime);

	{
		// Initialise the debug log file

		DebugLog = _tfopen(_T("zzBurnDebug.html"), _T("wb"));

		if (ftell(DebugLog) == 0) {
			WRITE_UNICODE_BOM(DebugLog);

			_ftprintf(DebugLog, _T("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\">"));
			_ftprintf(DebugLog, _T("<html><head><meta http-equiv=Content-Type content=\"text/html; charset=unicode\"></head><body><pre>"));
		}

		_ftprintf(DebugLog, _T("</font><font size=larger color=#000000>"));
		_ftprintf(DebugLog, _T("Debug log created by ") _T(APP_TITLE) _T(" v%.20s on %s\n<br>"), szAppBurnVer, _tasctime(tmTime));
	}
 #endif

	{
		// Initialise the debug console

		COORD DebugBufferSize = { 80, 1000 };

		{

			// Since AttachConsole is only present in Windows XP, import it manually

#if defined (_MSC_VER)
// #error Manually importing AttachConsole() function, but compiling with _WIN32_WINNT >= 0x0500
			if (!AttachConsole(ATTACH_PARENT_PROCESS)) {
				AllocConsole();
			}
#else
 #define ATTACH_PARENT_PROCESS ((DWORD)-1)

			BOOL (WINAPI* pAttachConsole)(DWORD dwProcessId) = NULL;
			HMODULE hKernel32DLL = LoadLibrary(_T("kernel32.dll"));

			if (hKernel32DLL) {
				pAttachConsole = (BOOL (WINAPI*)(DWORD))GetProcAddress(hKernel32DLL, "AttachConsole");
			}
			if (pAttachConsole) {
				if (!pAttachConsole(ATTACH_PARENT_PROCESS)) {
					AllocConsole();
				}
			} else {
				AllocConsole();
			}
			if (hKernel32DLL) {
				FreeLibrary(hKernel32DLL);
			}

 #undef ATTACH_PARENT_PROCESS
#endif

		}

		DebugBuffer = CreateConsoleScreenBuffer(GENERIC_WRITE, FILE_SHARE_READ, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
		SetConsoleScreenBufferSize(DebugBuffer, DebugBufferSize);
		SetConsoleActiveScreenBuffer(DebugBuffer);
		SetConsoleTitle(_T(APP_TITLE) _T(" Debug console"));

		SetConsoleTextAttribute(DebugBuffer, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
		_sntprintf(szConsoleBuffer, sizearray(szConsoleBuffer), _T("Welcome to the ") _T(APP_TITLE) _T(" debug console.\n"));
		WriteConsole(DebugBuffer, szConsoleBuffer, _tcslen(szConsoleBuffer), NULL, NULL);

		SetConsoleTextAttribute(DebugBuffer, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
		if (DebugLog) {
			_sntprintf(szConsoleBuffer, sizearray(szConsoleBuffer), _T("Debug messages are logged in zzBurnDebug.html"));
			if (!DebugLog || bEchoLog) {
				_sntprintf(szConsoleBuffer + _tcslen(szConsoleBuffer), 1024 - _tcslen(szConsoleBuffer), _T(", and echod to this console"));
			}
			_sntprintf(szConsoleBuffer + _tcslen(szConsoleBuffer), 1024 - _tcslen(szConsoleBuffer), _T(".\n\n"));
		} else {
			_sntprintf(szConsoleBuffer, sizearray(szConsoleBuffer), _T("Debug messages are echod to this console.\n\n"));
		}
		WriteConsole(DebugBuffer, szConsoleBuffer, _tcslen(szConsoleBuffer), NULL, NULL);
	}

	nPrevConsoleStatus = -1;

	bprintf = AppDebugPrintf;						// Redirect Burn library debug to our function
#endif

	return 0;
}

static int AppInit()
{
#if defined (_MSC_VER) && defined (_DEBUG)
	// Check for memory corruption
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
#endif

	OpenDebugLog();

	// Create a handle to the main thread of execution
	DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &hMainThread, 0, false, DUPLICATE_SAME_ACCESS);

	// Load config for the application
	configAppLoadXml();

	setWindowAspect();

#ifndef NO_GDI_PLUS
	GDIPInit();
#endif

	// Set random skin
	randomSelectSkin();

	SplashCreate();

	FBALocaliseInit(szLanguage);

	hModRich20A = LoadLibrary(_T("riched20.dll"));

#if 1 || !defined (FBA_DEBUG)
	// print a warning if we're running for the 1st time
	if (nIniVersion < nBurnVer) {
		scrnInit();
		FirstUsageCreate(hScrnWnd);
		configAppSaveXml();		// Create initial config file
	}
#endif

	// Set the thread priority for the main thread
	SetThreadPriority(GetCurrentThread(), nAppThreadPriority);

	// Init the Burn library
	BurnLibInit();

	if (VidSelect(nVidSelect)) {
		nVidSelect = VID_DDRAW;
		VidSelect(nVidSelect);
	}

	if (audio.select(audSelect)) {
		audio.select(_T("DirectSound"));
	}

//	hAccel = LoadAccelerators(hAppInst, MAKEINTRESOURCE(IDR_ACCELERATOR));

	// Build the ROM information
	CreateROMInfo();

	// Get romset information
//	getAllRomsetInfo();

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
	resizeImageFree();				// Free GDI object
	auditCleanup();					// Free audit info
	clearNodeInfo();				// Free node info

	if (hModRich20A) {
		FreeLibrary(hModRich20A);
	}

	FBALocaliseExit();

//	if (hAccel) {
//		DestroyAcceleratorTable(hAccel);
//		hAccel = NULL;
//	}

	SplashDestroy(1);

#ifndef NO_GDI_PLUS
	GDIPShutdown();
#endif

	CloseHandle(hMainThread);

	CloseDebugLog();

	return 0;
}

void AppCleanup()
{
	StopReplay();
	waveLogStop();
	AviStop();

	AppExit();
	strConvClean();
}

int AppMessage(MSG* msg)
{
	if (dialogIsDlgMessage(msg)) {
		return 0;
	}
	return 1; // Didn't process this message
}

bool AppProcessKeyboardInput()
{
	if (ChatActivated()) {
		return false;
	}
	return true;
}

// Main program entry point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR lpCmdLine, int nShowCmd)
{
	// Provide a custom exception handler
	SetUnhandledExceptionFilter(ExceptionFilter);

	hAppInst = hInstance;

	// Make version string
	if (nBurnVer & 0xFF) {
		// private version (alpha)
		_stprintf(szAppBurnVer, _T("%x.%x.%x.%02x"), nBurnVer >> 20, (nBurnVer >> 16) & 0x0F, (nBurnVer >> 8) & 0xFF, nBurnVer & 0xFF);
	} else {
		// public version
		_stprintf(szAppBurnVer, _T("%x.%x.%x"), nBurnVer >> 20, (nBurnVer >> 16) & 0x0F, (nBurnVer >> 8) & 0xFF);
	}

	nAppShowCmd = nShowCmd;

	appDirectory();					// Set current directory to be the applications directory

	createNeedDir();				// Make sure there are roms and cfg subdirectories

	{
		INITCOMMONCONTROLSEX initCC = {
			sizeof(INITCOMMONCONTROLSEX),
			ICC_BAR_CLASSES | ICC_COOL_CLASSES | ICC_LISTVIEW_CLASSES | ICC_PROGRESS_CLASS | ICC_TREEVIEW_CLASSES,
		};

		if (!InitCommonControlsEx(&initCC)) {
			InitCommonControls();
		}
	}

	if (!AppInit()) {				// Init the application
		if (!processCmdLine(lpCmdLine)) {
//			if (!bDrvOkay) {
				mediaInit();
//			}

			RunMessageLoop();		// Run the application message loop
		}
	}

	AppExit();						// Exit the application

	configAppSaveXml();				// Save config for the application

	strConvClean();					// Free string conversion

	return 0;
}
