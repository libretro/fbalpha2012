// FB Alpha - Emulator for MC68000/Z80 based arcade games
//            Refer to the "license.txt" file for more info

// Main module

// #define USE_SDL					// define if SDL is used
// #define DONT_DISPLAY_SPLASH		// Prevent Splash screen from being displayed
#define APP_DEBUG_LOG			// log debug messages to zzBurnDebug.html

#ifdef USE_SDL
 #include "SDL.h"
#endif

#include "burner.h"

#ifdef _MSC_VER
//  #include <winable.h>
 #ifdef _DEBUG
  #include <crtdbg.h>
 #endif
#endif

#include "version.h"

HINSTANCE hAppInst = NULL;			// Application Instance
HANDLE hMainThread;
long int nMainThreadID;
int nAppThreadPriority = THREAD_PRIORITY_NORMAL;
int nAppShowCmd;

static TCHAR szCmdLine[1024] = _T("");

HACCEL hAccel = NULL;

int nAppVirtualFps = 6000;			// App fps * 100

TCHAR szAppBurnVer[16] = _T("");
TCHAR szAppExeName[EXE_NAME_SIZE + 1];

bool bCmdOptUsed = 0;
bool bAlwaysProcessKeyboardInput = false;

bool bNoChangeNumLock = 1;
static bool bNumlockStatus;

bool bMonitorAutoCheck = true;

// Used for the load/save dialog in commdlg.h (savestates, input replay, wave logging)
TCHAR szChoice[MAX_PATH] = _T("");
OPENFILENAME ofn;

#if defined (UNICODE)
char* TCHARToANSI(const TCHAR* pszInString, char* pszOutString, int nOutSize)
{

	static char szStringBuffer[1024];
	memset(szStringBuffer, 0, sizeof(szStringBuffer));

	char* pszBuffer = pszOutString ? pszOutString : szStringBuffer;
	int nBufferSize = pszOutString ? nOutSize * 2 : sizeof(szStringBuffer);

	if (WideCharToMultiByte(CP_ACP, 0, pszInString, -1, pszBuffer, nBufferSize, NULL, NULL)) {
		return pszBuffer;
	}

	return NULL;
}

TCHAR* ANSIToTCHAR(const char* pszInString, TCHAR* pszOutString, int nOutSize)
{
	static TCHAR szStringBuffer[1024];

	TCHAR* pszBuffer = pszOutString ? pszOutString : szStringBuffer;
	int nBufferSize  = pszOutString ? nOutSize * 2 : sizeof(szStringBuffer);

	if (MultiByteToWideChar(CP_ACP, 0, pszInString, -1, pszBuffer, nBufferSize)) {
		return pszBuffer;
	}

	return NULL;
}
#else
char* TCHARToANSI(const TCHAR* pszInString, char* pszOutString, int /*nOutSize*/)
{
	if (pszOutString) {
		strcpy(pszOutString, pszInString);
		return pszOutString;
	}

	return (char*)pszInString;
}

TCHAR* ANSIToTCHAR(const char* pszInString, TCHAR* pszOutString, int /*nOutSize*/)
{
	if (pszOutString) {
		_tcscpy(pszOutString, pszInString);
		return pszOutString;
	}

	return (TCHAR*)pszInString;
}
#endif

CHAR *astring_from_utf8(const char *utf8string)
{
	WCHAR *wstring;
	int char_count;
	CHAR *result;

	// convert MAME string (UTF-8) to UTF-16
	char_count = MultiByteToWideChar(CP_UTF8, 0, utf8string, -1, NULL, 0);
	wstring = (WCHAR *)malloc(char_count * sizeof(*wstring));
	MultiByteToWideChar(CP_UTF8, 0, utf8string, -1, wstring, char_count);

	// convert UTF-16 to "ANSI code page" string
	char_count = WideCharToMultiByte(CP_ACP, 0, wstring, -1, NULL, 0, NULL, NULL);
	result = (CHAR *)malloc(char_count * sizeof(*result));
	if (result != NULL)
		WideCharToMultiByte(CP_ACP, 0, wstring, -1, result, char_count, NULL, NULL);

	if (wstring) {
		free(wstring);
		wstring = NULL;
	}
	return result;
}

char *utf8_from_astring(const CHAR *astring)
{
	WCHAR *wstring;
	int char_count;
	CHAR *result;

	// convert "ANSI code page" string to UTF-16
	char_count = MultiByteToWideChar(CP_ACP, 0, astring, -1, NULL, 0);
	wstring = (WCHAR *)malloc(char_count * sizeof(*wstring));
	MultiByteToWideChar(CP_ACP, 0, astring, -1, wstring, char_count);

	// convert UTF-16 to MAME string (UTF-8)
	char_count = WideCharToMultiByte(CP_UTF8, 0, wstring, -1, NULL, 0, NULL, NULL);
	result = (CHAR *)malloc(char_count * sizeof(*result));
	if (result != NULL)
		WideCharToMultiByte(CP_UTF8, 0, wstring, -1, result, char_count, NULL, NULL);

	if (wstring) {
		free(wstring);
		wstring = NULL;
	}
	return result;
}

WCHAR *wstring_from_utf8(const char *utf8string)
{
	int char_count;
	WCHAR *result;

	// convert MAME string (UTF-8) to UTF-16
	char_count = MultiByteToWideChar(CP_UTF8, 0, utf8string, -1, NULL, 0);
	result = (WCHAR *)malloc(char_count * sizeof(*result));
	if (result != NULL)
		MultiByteToWideChar(CP_UTF8, 0, utf8string, -1, result, char_count);

	return result;
}

char *utf8_from_wstring(const WCHAR *wstring)
{
	int char_count;
	char *result;

	// convert UTF-16 to MAME string (UTF-8)
	char_count = WideCharToMultiByte(CP_UTF8, 0, wstring, -1, NULL, 0, NULL, NULL);
	result = (char *)malloc(char_count * sizeof(*result));
	if (result != NULL)
		WideCharToMultiByte(CP_UTF8, 0, wstring, -1, result, char_count, NULL, NULL);

	return result;
}

#if defined (FBA_DEBUG)
 static TCHAR szConsoleBuffer[1024];
 static int nPrevConsoleStatus = -1;

 static HANDLE DebugBuffer;
 static FILE *DebugLog = NULL;
 static bool bEchoLog = true; // false;
#endif

#if defined (FBA_DEBUG)
// Debug printf to a file
static int __cdecl AppDebugPrintf(int nStatus, TCHAR* pszFormat, ...)
{
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

	return 0;
}
#endif

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

  #ifdef _UNICODE
		DebugLog = _tfopen(_T("zzBurnDebug.html"), _T("wb"));

		if (ftell(DebugLog) == 0) {
			WRITE_UNICODE_BOM(DebugLog);

			_ftprintf(DebugLog, _T("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\">"));
			_ftprintf(DebugLog, _T("<html><head><meta http-equiv=Content-Type content=\"text/html; charset=unicode\"></head><body><pre>"));
		}
  #else
		DebugLog = _tfopen(_T("zzBurnDebug.html"), _T("wt"));

		if (ftell(DebugLog) == 0) {
			_ftprintf(DebugLog, _T("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\">"));
			_ftprintf(DebugLog, _T("<html><head><meta http-equiv=Content-Type content=\"text/html; charset=windows-%i\"></head><body><pre>"), GetACP());
		}
  #endif

		_ftprintf(DebugLog, _T("</font><font size=larger color=#000000>"));
		_ftprintf(DebugLog, _T("Debug log created by ") _T(APP_TITLE) _T(" v%.20s on %s\n<br>"), szAppBurnVer, _tasctime(tmTime));
	}
 #endif

	{
		// Initialise the debug console

		COORD DebugBufferSize = { 80, 1000 };

		{

			// Since AttachConsole is only present in Windows XP, import it manually

#if _WIN32_WINNT >= 0x0500 && defined (_MSC_VER)
// #error Manually importing AttachConsole() function, but compiling with _WIN32_WINNT >= 0x0500
			if (!AttachConsole(ATTACH_PARENT_PROCESS)) {
				AllocConsole();
			}
#else
 #define ATTACH_PARENT_PROCESS ((DWORD)-1)

			BOOL (WINAPI* pAttachConsole)(DWORD dwProcessId) = NULL;
			HINSTANCE hKernel32DLL = LoadLibrary(_T("kernel32.dll"));

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
		_sntprintf(szConsoleBuffer, 1024, _T("Welcome to the ") _T(APP_TITLE) _T(" debug console.\n"));
		WriteConsole(DebugBuffer, szConsoleBuffer, _tcslen(szConsoleBuffer), NULL, NULL);

		SetConsoleTextAttribute(DebugBuffer, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
		if (DebugLog) {
			_sntprintf(szConsoleBuffer, 1024, _T("Debug messages are logged in zzBurnDebug.html"));
			if (!DebugLog || bEchoLog) {
				_sntprintf(szConsoleBuffer + _tcslen(szConsoleBuffer), 1024 - _tcslen(szConsoleBuffer), _T(", and echod to this console"));
			}
			_sntprintf(szConsoleBuffer + _tcslen(szConsoleBuffer), 1024 - _tcslen(szConsoleBuffer), _T(".\n\n"));
		} else {
			_sntprintf(szConsoleBuffer, 1024, _T("Debug messages are echod to this console.\n\n"));
		}
		WriteConsole(DebugBuffer, szConsoleBuffer, _tcslen(szConsoleBuffer), NULL, NULL);
	}

	nPrevConsoleStatus = -1;

	bprintf = AppDebugPrintf;							// Redirect Burn library debug to our function
#endif

	return 0;
}

void MonitorAutoCheck()
{
	RECT rect;
	int x, y;

	SystemParametersInfo(SPI_GETWORKAREA, 0, &rect, 0);

	x = GetSystemMetrics(SM_CXSCREEN);
	y = GetSystemMetrics(SM_CYSCREEN);

	TCHAR szResXY[256] = _T("");
	_stprintf(szResXY, _T("%dx%d"), x, y);

	// Normal CRT (4:3) ( Verified at Wikipedia.Org )
	if( !_tcscmp(szResXY, _T("320x240"))	|| 
		!_tcscmp(szResXY, _T("512x384"))	||
		!_tcscmp(szResXY, _T("640x480"))	|| 
		!_tcscmp(szResXY, _T("800x600"))	||
		!_tcscmp(szResXY, _T("832x624"))	||
		!_tcscmp(szResXY, _T("1024x768"))	||
		!_tcscmp(szResXY, _T("1120x832"))	|| 
		!_tcscmp(szResXY, _T("1152x864"))	|| 
		!_tcscmp(szResXY, _T("1280x960"))	||
		!_tcscmp(szResXY, _T("1280x1024"))	|| 
		!_tcscmp(szResXY, _T("1400x1050"))	||  
		!_tcscmp(szResXY, _T("1600x1200"))	||
		!_tcscmp(szResXY, _T("2048x1536"))	||
		!_tcscmp(szResXY, _T("2800x2100"))	||
		!_tcscmp(szResXY, _T("3200x2400"))	|| 
		!_tcscmp(szResXY, _T("4096x3072"))	||
		!_tcscmp(szResXY, _T("6400x4800")) ){
		nVidScrnAspectX = 4; 
		nVidScrnAspectY = 3;
	}

	// Normal LCD (5:4) ( Verified at Wikipedia.Org )
	if( !_tcscmp(szResXY, _T("320x256"))	||
		!_tcscmp(szResXY, _T("640x512"))	||
		!_tcscmp(szResXY, _T("1280x1024"))	|| 
		!_tcscmp(szResXY, _T("2560x2048"))	||
		!_tcscmp(szResXY, _T("5120x4096")) ){
		nVidScrnAspectX = 5; 
		nVidScrnAspectY = 4;
	}

	// CRT Widescreen (16:9) ( Verified at Wikipedia.Org )
	if( !_tcscmp(szResXY, _T("480x270"))  ||
		!_tcscmp(szResXY, _T("1280x720")) ||
		!_tcscmp(szResXY, _T("1360x768")) ||
		!_tcscmp(szResXY, _T("1366x768")) ||
		!_tcscmp(szResXY, _T("1920x1080"))) {
		nVidScrnAspectX = 16; 
		nVidScrnAspectY = 9;
	}

	// LCD Widescreen (16:10) ( Verified at Wikipedia.Org )
	if(	!_tcscmp(szResXY, _T("320x200"))	|| 
		!_tcscmp(szResXY, _T("1280x800"))	||
		!_tcscmp(szResXY, _T("1440x900"))	|| 
		!_tcscmp(szResXY, _T("1680x1050"))	||
		!_tcscmp(szResXY, _T("1920x1200"))	||
		!_tcscmp(szResXY, _T("2560x1600"))	||
		!_tcscmp(szResXY, _T("3840x2400"))	||
		!_tcscmp(szResXY, _T("5120x3200"))	||
		!_tcscmp(szResXY, _T("7680x4800")) ){
		nVidScrnAspectX = 16; 
		nVidScrnAspectY = 10;
	}
}

bool SetNumLock(bool bState)
{
	BYTE keyState[256];
	
	if (bNoChangeNumLock) return 0;

	GetKeyboardState(keyState);
	if ((bState && !(keyState[VK_NUMLOCK] & 1)) || (!bState && (keyState[VK_NUMLOCK] & 1))) {
		keybd_event(VK_NUMLOCK, 0, KEYEVENTF_EXTENDEDKEY, 0 );

		keybd_event(VK_NUMLOCK, 0, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
	}

	return keyState[VK_NUMLOCK] & 1;
}

#include <wininet.h>

void GenBurnDrvHeader()
{
	// create custom burn_drivers.h with all drivers info
	FILE* fp = NULL;
	fp = fopen("burn_drivers.h", "w");
	if(fp) 
	{
		fprintf(fp, "// Driverlist Generated via custom %s [ v%.20s ][ Windows ]\n", APP_TITLE, TCHARToANSI(szAppBurnVer, NULL, 0));
		fprintf(fp, "#include <stdio.h>\n");
		fprintf(fp, "#include <stdint.h>\n");

		fprintf(fp, "\n");

		fprintf(fp, "struct FBA_DRV \n{ \n");

		fprintf(fp, "\t" "uint32_t nDrv; \n");
		fprintf(fp, "\t" "char szName[32]; \n");
		fprintf(fp, "\t" "char szParent[32];\n");
		fprintf(fp, "\t" "char szBoardROM[32];\n");
		fprintf(fp, "\t" "char szTitle[256];\n");
		fprintf(fp, "\t" "char szYear[8];\n");
		fprintf(fp, "\t" "char szCompany[256];\n");
		fprintf(fp, "\t" "char szSystem[256];\n");
		fprintf(fp, "\t" "uint32_t nMaxPlayers;\n");
		fprintf(fp, "\t" "uint32_t nWidth; \n");
		fprintf(fp, "\t" "uint32_t nHeight;\n");
		fprintf(fp, "\t" "uint32_t nAspectX;\n");
		fprintf(fp, "\t" "uint32_t nAspectY;\n");
		fprintf(fp, "\t" "char szSystemFilter[32];\n");

		fprintf(fp, "};\n");
		
		fprintf(fp, "\n");

		fprintf(fp, "FBA_DRV fba_drv[] = \n{ \n");

		static int CapcomMiscValue		= HARDWARE_PREFIX_CAPCOM_MISC >> 24;
		static int MASKCAPMISC			= 1 << CapcomMiscValue;
		static int CaveValue			= HARDWARE_PREFIX_CAVE >> 24;
		static int MASKCAVE				= 1 << CaveValue;
		static int CpsValue				= HARDWARE_PREFIX_CAPCOM >> 24;
		static int MASKCPS				= 1 << CpsValue;
		static int Cps2Value			= HARDWARE_PREFIX_CPS2 >> 24;
		static int MASKCPS2				= 1 << Cps2Value;
		static int Cps3Value			= HARDWARE_PREFIX_CPS3 >> 24;
		static int MASKCPS3				= 1 << Cps3Value;
		static int DataeastValue		= HARDWARE_PREFIX_DATAEAST >> 24;
		static int MASKDATAEAST			= 1 << DataeastValue;
		static int GalaxianValue		= HARDWARE_PREFIX_GALAXIAN >> 24;
		static int MASKGALAXIAN			= 1 << GalaxianValue;
		static int IremValue			= HARDWARE_PREFIX_IREM >> 24;
		static int MASKIREM				= 1 << IremValue;
		static int KanekoValue			= HARDWARE_PREFIX_KANEKO >> 24;
		static int MASKKANEKO			= 1 << KanekoValue;
		static int KonamiValue			= HARDWARE_PREFIX_KONAMI >> 24;
		static int MASKKONAMI			= 1 << KonamiValue;
		static int NeogeoValue			= HARDWARE_PREFIX_SNK >> 24;
		static int MASKNEOGEO			= 1 << NeogeoValue;
		static int PacmanValue			= HARDWARE_PREFIX_PACMAN >> 24;
		static int MASKPACMAN			= 1 << PacmanValue;
		static int PgmValue				= HARDWARE_PREFIX_IGS_PGM >> 24;
		static int MASKPGM				= 1 << PgmValue;
		static int PsikyoValue			= HARDWARE_PREFIX_PSIKYO >> 24;
		static int MASKPSIKYO			= 1 << PsikyoValue;
		static int SegaValue			= HARDWARE_PREFIX_SEGA >> 24;
		static int MASKSEGA				= 1 << SegaValue;
		static int SetaValue			= HARDWARE_PREFIX_SETA >> 24;
		static int MASKSETA				= 1 << SetaValue;
		static int TaitoValue			= HARDWARE_PREFIX_TAITO >> 24;
		static int MASKTAITO			= 1 << TaitoValue;
		static int TechnosValue			= HARDWARE_PREFIX_TECHNOS >> 24;
		static int MASKTECHNOS			= 1 << TechnosValue;
		static int ToaplanValue			= HARDWARE_PREFIX_TOAPLAN >> 24;
		static int MASKTOAPLAN			= 1 << ToaplanValue;
		static int MiscPre90sValue		= HARDWARE_PREFIX_MISC_PRE90S >> 24;
		static int MASKMISCPRE90S		= 1 << MiscPre90sValue;
		static int MiscPost90sValue		= HARDWARE_PREFIX_MISC_POST90S >> 24;
		static int MASKMISCPOST90S		= 1 << MiscPost90sValue;
		static int MegadriveValue		= HARDWARE_PREFIX_SEGA_MEGADRIVE >> 24;
		static int MASKMEGADRIVE		= 1 << MegadriveValue;
		static int PCEngineValue		= HARDWARE_PREFIX_PCENGINE >> 24;
		static int MASKPCENGINE			= 1 << PCEngineValue;
		static int SnesValue			= HARDWARE_PREFIX_NINTENDO_SNES >> 24;
		static int MASKSNES				= 1 << SnesValue;
		//static int MASKALL				= MASKCAPMISC | MASKCAVE | MASKCPS | MASKCPS2 | MASKCPS3 | MASKDATAEAST | MASKGALAXIAN | MASKIREM | MASKKANEKO | MASKKONAMI | MASKNEOGEO | MASKPACMAN | MASKPGM | MASKPSIKYO | MASKSEGA | MASKSETA | MASKTAITO | MASKTECHNOS | MASKTOAPLAN | MASKMISCPRE90S | MASKMISCPOST90S | MASKMEGADRIVE | MASKPCENGINE | MASKSNES;

		int nTotalDrv = 0;
		for (unsigned int i = 0; i < nBurnDrvCount; i++) 
		{
			nBurnDrvActive = i;

			int nHardware = 1 << (BurnDrvGetHardwareCode() >> 24);

			char szSystemFilter[32] = { 0 };

			if ((nHardware & MASKCAPMISC))		sprintf(szSystemFilter, "MASKCAPMISC");
			if ((nHardware & MASKCAVE))			sprintf(szSystemFilter, "MASKCAVE");
			if ((nHardware & MASKCPS))			sprintf(szSystemFilter, "MASKCPS");
			if ((nHardware & MASKCPS2))			sprintf(szSystemFilter, "MASKCPS2");
			if ((nHardware & MASKCPS3))			sprintf(szSystemFilter, "MASKCPS3");
			if ((nHardware & MASKDATAEAST))		sprintf(szSystemFilter, "MASKDATAEAST");
			if ((nHardware & MASKGALAXIAN))		sprintf(szSystemFilter, "MASKGALAXIAN");
			if ((nHardware & MASKIREM))			sprintf(szSystemFilter, "MASKIREM");
			if ((nHardware & MASKKANEKO))		sprintf(szSystemFilter, "MASKKANEKO");
			if ((nHardware & MASKKONAMI))		sprintf(szSystemFilter, "MASKKONAMI");
			if ((nHardware & MASKNEOGEO))		sprintf(szSystemFilter, "MASKNEOGEO");
			if ((nHardware & MASKPACMAN))		sprintf(szSystemFilter, "MASKPACMAN");
			if ((nHardware & MASKPGM))			sprintf(szSystemFilter, "MASKPGM");
			if ((nHardware & MASKPSIKYO))		sprintf(szSystemFilter, "MASKPSIKYO");
			if ((nHardware & MASKSEGA))			sprintf(szSystemFilter, "MASKSEGA");
			if ((nHardware & MASKSETA))			sprintf(szSystemFilter, "MASKSETA");
			if ((nHardware & MASKTAITO))		sprintf(szSystemFilter, "MASKTAITO");
			if ((nHardware & MASKTECHNOS))		sprintf(szSystemFilter, "MASKTECHNOS");
			if ((nHardware & MASKTOAPLAN))		sprintf(szSystemFilter, "MASKTOAPLAN");
			if ((nHardware & MASKMISCPRE90S))	sprintf(szSystemFilter, "MASKMISCPRE90S");
			if ((nHardware & MASKMISCPOST90S))	sprintf(szSystemFilter, "MASKMISCPOST90S");

			//if ((nHardware & MASKMEGADRIVE))	sprintf(szSystemFilter, "MASKMEGADRIVE");
			//if ((nHardware & MASKPCENGINE))		sprintf(szSystemFilter, "MASKPCENGINE");
			//if ((nHardware & MASKSNES))			sprintf(szSystemFilter, "MASKSNES");

			// skip WIP drivers...
			if ((nHardware & MASKMEGADRIVE) || 
				(nHardware & MASKPCENGINE) ||
				(nHardware & MASKSNES)) 
			{
				continue;
			}

			nTotalDrv++;

			INT32 nWidth, nHeight;
			BurnDrvGetFullSize(&nWidth, &nHeight);

			INT32 nAspectX, nAspectY;
			BurnDrvGetAspect(&nAspectX, &nAspectY);

			fprintf(fp, "\t" "{ %d, \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", %d, %d, %d, %d, %d, \"%s\" }, \n", 
				i,									// Drv ID
				BurnDrvGetTextA(DRV_NAME),			// Romset (Ex. mslug)
				BurnDrvGetTextA(DRV_PARENT) ? BurnDrvGetTextA(DRV_PARENT) : " ",		// Parent (Ex. sf2)
				BurnDrvGetTextA(DRV_BOARDROM) ? BurnDrvGetTextA(DRV_BOARDROM) : " ",	// ...
				BurnDrvGetTextA(DRV_FULLNAME),		// Title
				BurnDrvGetTextA(DRV_DATE),			// Year
				BurnDrvGetTextA(DRV_MANUFACTURER),	// Company (Ex. Capcom)
				BurnDrvGetTextA(DRV_SYSTEM),		// System (Ex. Neo-Geo)
				BurnDrvGetMaxPlayers(),				// No. Players
				nWidth,								// Game Width
				nHeight,							// Game Height
				nAspectX,							// Aspect Ratio (X)
				nAspectY,							// Aspect Ratio (Y)
				szSystemFilter						// System Filter			
			);
		}

		fprintf(fp, "}; \n");

		fprintf(fp, "// --------------------------------------------------------- \n");
		fprintf(fp, "// Total Drivers: %d \n", nTotalDrv);
		fprintf(fp, "// --------------------------------------------------------- \n");
		fclose(fp);
		fp = NULL;
	}
}

void GenRomInfoHeader()
{
	FILE* fp = NULL;
	fp = fopen("burn_rominfo.h", "w");
	if(fp)
	{
		fprintf(fp, "// Romsinfo Generated via custom %s [ v%.20s ][ Windows ]\n", APP_TITLE, TCHARToANSI(szAppBurnVer, NULL, 0));
		fprintf(fp, "#include <stdio.h>\n");
		fprintf(fp, "#include <stdint.h>\n");

		fprintf(fp, "\n");

		fprintf(fp, "struct BurnRomInfo { \n");
		fprintf(fp, "\tchar szName[100]; \n");
		fprintf(fp, "\tuint32_t nLen; \n");
		fprintf(fp, "\tuint32_t nCrc; \n");
		fprintf(fp, "\tchar szType[128]; \n");
		fprintf(fp, "}; \n\n");

		fprintf(fp, "struct BurnSampleInfo { \n");
		fprintf(fp, "\tchar szName[100]; \n");
		fprintf(fp, "\tuint32_t nFlags; \n");
		fprintf(fp, "}; \n\n");

		fprintf(fp, "struct FBA_ROMINFO { \n");
		fprintf(fp, "\tunsigned int nDrv; \n");
		fprintf(fp, "\tchar szName[32];\n");
		fprintf(fp, "\tBurnRomInfo roms_info[0x100];\n");
		fprintf(fp, "\tBurnRomInfo board_roms_info[0x100];\n");
		fprintf(fp, "\tBurnSampleInfo samples_info[0x100];\n");
		fprintf(fp, "};\n");

		fprintf(fp, "\n");

		fprintf(fp, "FBA_ROMINFO fba_romsinfo[] = \n{ \n");
						
		for (unsigned int ii = 0; ii < nBurnDrvCount; ii++) 
		{
			nBurnDrvActive = ii;			
			
			fprintf(fp, "\t// --------------------------------------------------------- \n");
			fprintf(fp, "\t// %s \n", BurnDrvGetTextA(DRV_FULLNAME));
			fprintf(fp, "\t// --------------------------------------------------------- \n");
			
			fprintf(fp, "\t{ \n");

			fprintf(fp, "\t\t// [ driver index ] \n");

			fprintf(fp, "\t\t%d, \n", ii);

			fprintf(fp, "\t\t// [ romset ] \n");

			fprintf(fp, "\t\t\"%s\", \n", BurnDrvGetTextA(DRV_NAME));

			fprintf(fp, "\t\t// [ roms_info ] \n");

			fprintf(fp, "\t\t{ \n");

			int RomPos = 0;
			for (int i = 0; i < 0x100; i++) { // assume max 0x100 roms per game

				int nRet;
				struct BurnRomInfo ri;
				//char nLen[10] = "";
				char nCrc[8] = "";
				char *szRomName = NULL;
				TCHAR Type[100] = _T("");
				TCHAR FormatType[100] = _T("");

				memset(&ri, 0, sizeof(ri));
	
				nRet = BurnDrvGetRomInfo(&ri, i);
				nRet += BurnDrvGetRomName(&szRomName, i, 0);
		
				if (ri.nLen == 0) continue;		
				if (ri.nType & BRF_BIOS) continue;
		
				fprintf(fp, "\t\t\t{ ");
				
				fprintf(fp, "\"%s\", ", szRomName);

				//sprintf(nLen, "%d", ri.nLen);
				fprintf(fp, "%d, ", ri.nLen);
		
				sprintf(nCrc, "%08X", ri.nCrc);
				if (!(ri.nType & BRF_NODUMP)) 
				{
					fprintf(fp, "0x%s, ", nCrc);
				} else {
					fprintf(fp, "0x00000000, ");
				}
		
				if (ri.nType & BRF_ESS) _stprintf(Type, FBALoadStringEx(hAppInst, IDS_ROMINFO_ESSENTIAL, true), Type);
				if (ri.nType & BRF_OPT) _stprintf(Type, FBALoadStringEx(hAppInst, IDS_ROMINFO_OPTIONAL, true), Type);
				if (ri.nType & BRF_PRG)	_stprintf(Type, FBALoadStringEx(hAppInst, IDS_ROMINFO_PROGRAM, true), Type);
				if (ri.nType & BRF_GRA) _stprintf(Type, FBALoadStringEx(hAppInst, IDS_ROMINFO_GRAPHICS, true), Type);
				if (ri.nType & BRF_SND) _stprintf(Type, FBALoadStringEx(hAppInst, IDS_ROMINFO_SOUND, true), Type);
				if (ri.nType & BRF_BIOS) _stprintf(Type, FBALoadStringEx(hAppInst, IDS_ROMINFO_BIOS, true), Type);
				
				for (int j = 0; j < 98; j++) {
					FormatType[j] = Type[j + 2];
				}
		
				if (ri.nType & BRF_NODUMP) 
				{
					fprintf(fp, "\"%s\", ", "No Dump");
				} else {
					fprintf(fp, "\"%s\", ", TCHARToANSI(FormatType, NULL, 0));
				}

				fprintf(fp, "}, \n");

				RomPos++;
			}

			fprintf(fp, "\t\t}, \n");
			
			fprintf(fp, "\t\t// [ board_roms_info ] \n");

			fprintf(fp, "\t\t{ \n");					

			// Check for board roms
			if (BurnDrvGetTextA(DRV_BOARDROM)) {				
				
				char szBoardName[8] = "";
				unsigned int nOldDrvSelect = nBurnDrvActive;
				strcpy(szBoardName, BurnDrvGetTextA(DRV_BOARDROM));
			
				for (unsigned int i = 0; i < nBurnDrvCount; i++) {
					nBurnDrvActive = i;
					if (!strcmp(szBoardName, BurnDrvGetTextA(DRV_NAME))) break;
				}
			
				for (int j = 0; j < 0x100; j++) {					

					int nRetBoard;
					struct BurnRomInfo riBoard;
					//char nLenBoard[10] = "";
					char nCrcBoard[8] = "";
					char *szBoardRomName = NULL;
					TCHAR BoardType[100] = _T("");
					TCHAR BoardFormatType[100] = _T("");

					memset(&riBoard, 0, sizeof(riBoard));

					nRetBoard = BurnDrvGetRomInfo(&riBoard, j);
					nRetBoard += BurnDrvGetRomName(&szBoardRomName, j, 0);
		
					if (riBoard.nLen == 0) continue;
				
					fprintf(fp, "\t\t\t{ ");

					fprintf(fp, "\"%s\", ", szBoardRomName);

					//sprintf(nLenBoard, "%d", riBoard.nLen);
					fprintf(fp, "%d, ", riBoard.nLen);
		
					sprintf(nCrcBoard, "%08X", riBoard.nCrc);
					if (!(riBoard.nType & BRF_NODUMP)) 
					{
						fprintf(fp, "0x%s, ", nCrcBoard);
					} else {
						fprintf(fp, "0x00000000, ");
					}
			
					if (riBoard.nType & BRF_ESS) _stprintf(BoardType, FBALoadStringEx(hAppInst, IDS_ROMINFO_ESSENTIAL, true), BoardType);
					if (riBoard.nType & BRF_OPT) _stprintf(BoardType, FBALoadStringEx(hAppInst, IDS_ROMINFO_OPTIONAL, true), BoardType);
					if (riBoard.nType & BRF_PRG) _stprintf(BoardType, FBALoadStringEx(hAppInst, IDS_ROMINFO_PROGRAM, true), BoardType);
					if (riBoard.nType & BRF_GRA) _stprintf(BoardType, FBALoadStringEx(hAppInst, IDS_ROMINFO_GRAPHICS, true), BoardType);
					if (riBoard.nType & BRF_SND) _stprintf(BoardType, FBALoadStringEx(hAppInst, IDS_ROMINFO_SOUND, true), BoardType);
					if (riBoard.nType & BRF_BIOS) _stprintf(BoardType, FBALoadStringEx(hAppInst, IDS_ROMINFO_BIOS, true), BoardType);
		
					for (int k = 0; k < 98; k++) {
						BoardFormatType[k] = BoardType[k + 2];
					}
		
					if (riBoard.nType & BRF_NODUMP) 
					{
						fprintf(fp, "\"%s\", ", "No Dump");
					} else {
						fprintf(fp, "\"%s\", ", TCHARToANSI(BoardFormatType, NULL, 0));
					}
								
					fprintf(fp, "}, \n");

					RomPos++;
				}
		
				nBurnDrvActive = nOldDrvSelect;
			} else {			
				fprintf(fp, "\t\t\t{ 0 }, \n");				
			}

			fprintf(fp, "\t\t}, \n");

			fprintf(fp, "\t\t// [ samples ] \n");

			fprintf(fp, "\t\t{ \n");

			int SamplePos = 0;
			if (BurnDrvGetTextA(DRV_SAMPLENAME) != NULL) 
			{
				for (int i = 0; i < 0x100; i++) { // assume max 0x100 samples per game
					
					int nRet;
					struct BurnSampleInfo si;
					char *szSampleName = NULL;

					memset(&si, 0, sizeof(si));

					nRet = BurnDrvGetSampleInfo(&si, i);
					nRet += BurnDrvGetSampleName(&szSampleName, i, 0);
		
					if (si.nFlags == 0) continue;		
		
					fprintf(fp, "\t\t\t{ ");
		
					fprintf(fp, "\"%s\", \n", szSampleName);

					fprintf(fp, "%d \n", si.nFlags);

					fprintf(fp, "}, \n");

					SamplePos++;
				}
			} else {
				fprintf(fp, "\t\t\t{ 0 }, \n");
			}

			fprintf(fp, "\t\t}, \n");

			fprintf(fp, "\t}, \n");
		}

		fprintf(fp, "}; \n");

		fclose(fp);
		fp = NULL;
	}
}

static int AppInit()
{
#if defined (_MSC_VER) && defined (_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_CHECK_ALWAYS_DF);			// Check for memory corruption
	_CrtSetDbgFlag(_CRTDBG_DELAY_FREE_MEM_DF);			//
	_CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF);				//
#endif

	OpenDebugLog();

	// Create a handle to the main thread of execution
	DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &hMainThread, 0, false, DUPLICATE_SAME_ACCESS);
	
	// Init the Burn library
	BurnLibInit();
	
	// Load config for the application
	ConfigAppLoad();

	FBALocaliseInit(szLocalisationTemplate);
	BurnerDoGameListLocalisation();

	if (bMonitorAutoCheck) MonitorAutoCheck();

#if 1 || !defined (FBA_DEBUG)
	// print a warning if we're running for the 1st time
	if (nIniVersion < nBurnVer) {
		ScrnInit();
		//SplashDestroy(1);
		FirstUsageCreate();

		ConfigAppSave();								// Create initial config file
	}
#endif

	// Set the thread priority for the main thread
	SetThreadPriority(GetCurrentThread(), nAppThreadPriority);

	bCheatsAllowed = true;

#ifdef USE_SDL
	SDL_Init(0);
#endif

	ComputeGammaLUT();

	if (VidSelect(nVidSelect)) {
		nVidSelect = 0;
		VidSelect(nVidSelect);
	}

	hAccel = LoadAccelerators(hAppInst, MAKEINTRESOURCE(IDR_ACCELERATOR));

	// Build the ROM information
	CreateROMInfo(NULL);
	
	// Write a clrmame dat file if we are verifying roms
#if defined (ROM_VERIFY)
	create_datfile(_T("fba.dat"), 0);	
#endif

	bNumlockStatus = SetNumLock(false);
	
	if(bEnableIcons && !bIconsLoaded) {
		// load driver icons
		LoadDrvIcons();
		bIconsLoaded = 1;
	}

	GenBurnDrvHeader();
	GenRomInfoHeader();

	return 0;
}

static int AppExit()
{
	if(bIconsLoaded) {
		// unload driver icons
		UnloadDrvIcons();
		bIconsLoaded = 0;
	}
	
	SetNumLock(bNumlockStatus);

	DrvExit();						// Make sure any game driver is exitted
	FreeROMInfo();
	MediaExit();
	BurnLibExit();					// Exit the Burn library

#ifdef USE_SDL
	SDL_Quit();
#endif

	FBALocaliseExit();
	BurnerExitGameListLocalisation();

	if (hAccel) {
		DestroyAcceleratorTable(hAccel);
		hAccel = NULL;
	}

	SplashDestroy(1);

	CloseHandle(hMainThread);

	CloseDebugLog();

	return 0;
}

void AppCleanup()
{
	StopReplay();
	WaveLogStop();

	AppExit();
}

int AppMessage(MSG *pMsg)
{
	if (IsDialogMessage(hInpdDlg, pMsg))	 return 0;
	if (IsDialogMessage(hInpCheatDlg, pMsg)) return 0;
	if (IsDialogMessage(hInpDIPSWDlg, pMsg)) return 0;
	if (IsDialogMessage(hDbgDlg, pMsg))		 return 0;

	if (IsDialogMessage(hInpsDlg, pMsg))	 return 0;
	if (IsDialogMessage(hInpcDlg, pMsg))	 return 0;

	return 1; // Didn't process this message
}

bool AppProcessKeyboardInput()
{
	if (hwndChat) {
		return false;
	}

	return true;
}

int ProcessCmdLine()
{
	unsigned int i;
	int nOptX = 0, nOptY = 0, nOptD = 0;
	int nOpt1Size;
	TCHAR szOpt2[3] = _T("");
	TCHAR szName[MAX_PATH];

	if (szCmdLine[0] == _T('\"')) {
		int nLen = _tcslen(szCmdLine);
		nOpt1Size = 1;
		while (szCmdLine[nOpt1Size] != _T('\"') && nOpt1Size < nLen) {
			nOpt1Size++;
		}
		if (nOpt1Size == nLen) {
			szName[0] = 0;
		} else {
			nOpt1Size++;
			_tcsncpy(szName, szCmdLine + 1, nOpt1Size - 2);
			szName[nOpt1Size - 2] = 0;
		}
	} else {
		int nLen = _tcslen(szCmdLine);
		nOpt1Size = 0;
		while (szCmdLine[nOpt1Size] != _T(' ') && nOpt1Size < nLen) {
			nOpt1Size++;
		}
		_tcsncpy(szName, szCmdLine, nOpt1Size);
		szName[nOpt1Size] = 0;
	}

	if (_tcslen(szName)) {
		if (_tcscmp(szName, _T("-listinfo")) == 0) {
			write_datfile(DAT_ARCADE_ONLY, stdout);
			return 1;
		}
		
		if (_tcscmp(szName, _T("-listinfomdonly")) == 0) {
			write_datfile(DAT_MEGADRIVE_ONLY, stdout);
			return 1;
		}
		
		if (_tcscmp(szName, _T("-listinfopceonly")) == 0) {
			write_datfile(DAT_PCENGINE_ONLY, stdout);
			return 1;
		}
		
		if (_tcscmp(szName, _T("-listinfotg16only")) == 0) {
			write_datfile(DAT_TG16_ONLY, stdout);
			return 1;
		}
		
		if (_tcscmp(szName, _T("-listinfosgxonly")) == 0) {
			write_datfile(DAT_SGX_ONLY, stdout);
			return 1;
		}
		
		if (_tcscmp(szName, _T("-listextrainfo")) == 0) {
			int nWidth;
			int nHeight;
			int nAspectX;
			int nAspectY;
			for (i = 0; i < nBurnDrvCount; i++) {
				nBurnDrvActive = i;
				BurnDrvGetVisibleSize(&nWidth, &nHeight);
				BurnDrvGetAspect(&nAspectX, &nAspectY);
				printf("%s\t%ix%i\t%i:%i\t0x%08X\t\"%s\"\t%i\t%i\t%x\t%x\t\"%s\"\n", BurnDrvGetTextA(DRV_NAME), nWidth, nHeight, nAspectX, nAspectY, BurnDrvGetHardwareCode(), BurnDrvGetTextA(DRV_SYSTEM), BurnDrvIsWorking(), BurnDrvGetMaxPlayers(), BurnDrvGetGenreFlags(), BurnDrvGetFamilyFlags(), BurnDrvGetTextA(DRV_COMMENT));
			}			
			return 1;
		}
	}

	_stscanf(&szCmdLine[nOpt1Size], _T("%2s %i x %i x %i"), szOpt2, &nOptX, &nOptY, &nOptD);

	if (_tcslen(szName)) {
		bool bFullscreen = 1;
		bCmdOptUsed = 1;

		if (_tcscmp(szOpt2, _T("-r")) == 0) {
			if (nOptX && nOptY) {
				nVidWidth = nOptX;
				nVidHeight = nOptY;
			}
			if (nOptD) {
				nVidDepth = nOptD;
			}
		} else {
			if (_tcscmp(szOpt2, _T("-a")) == 0) {
				bVidArcaderes = 1;
			} else {
				if (_tcscmp(szOpt2, _T("-w")) == 0) {
					bCmdOptUsed = 0;
					bFullscreen = 0;
				}
			}
		}

		if (bFullscreen) {
			nVidFullscreen = 1;
		}

		if (_tcscmp(&szName[_tcslen(szName) - 3], _T(".fs")) == 0) {
			if (BurnStateLoad(szName, 1, &DrvInitCallback)) {
				return 1;
			} else {
//				bRunPause = 1;
			}
		} else {
			if (_tcscmp(&szName[_tcslen(szName) - 3], _T(".fr")) == 0) {
				if (StartReplay(szName)) {
					return 1;
				}
			} else {
				for (i = 0; i < nBurnDrvCount; i++) {
					nBurnDrvActive = i;
					if ((_tcscmp(BurnDrvGetText(DRV_NAME), szName) == 0) && (!(BurnDrvGetFlags() & BDF_BOARDROM))){
						MediaInit();
						DrvInit(i, true);
						break;
					}
				}
				if (i == nBurnDrvCount) {
					FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_ERR_UI_NOSUPPORT), szName, _T(APP_TITLE));
					FBAPopupDisplay(PUF_TYPE_ERROR);
					return 1;
				}
			}
		}
	}

	POST_INITIALISE_MESSAGE;

	if (!nVidFullscreen) {
		MenuEnableItems();
	}

	return 0;
}

// Main program entry point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR lpCmdLine, int nShowCmd)
{
	DSCore_Init();
	DICore_Init();
	DDCore_Init();
	Dx9Core_Init();

	// Try to initiate DWMAPI.DLL on Windows 7
	if(IsWindows7()) {
		InitDWMAPI();
	}

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
	
#if !defined (DONT_DISPLAY_SPLASH)
	if (lpCmdLine[0] == 0) SplashCreate();
#endif

	nAppShowCmd = nShowCmd;

	AppDirectory();								// Set current directory to be the applications directory

	// Make sure there are roms and cfg subdirectories
	TCHAR szDirs[19][MAX_PATH] = {
		{_T("config")},
		{_T("config/games")},
		{_T("config/ips")},
		{_T("config/localisation")},
		{_T("config/presets")},
		{_T("recordings")},
		{_T("ROMs")},
		{_T("savestates")},
		{_T("screenshots")},
		{_T("support/")},
		{_T("support/previews/")},
		{_T("support/titles/")},
		{_T("support/icons/")},
		{_T("support/cheats/")},
		{_T("support/hiscores/")},
		{_T("support/samples/")},
		{_T("support/ips/")},
		{_T("support/neocdz/")},
		{_T("neocdiso/")},
	};

	for(int x = 0; x < 19; x++) {
		CreateDirectory(szDirs[x], NULL);
	}

	//
	
	{
		INITCOMMONCONTROLSEX initCC = {
			sizeof(INITCOMMONCONTROLSEX),
			ICC_BAR_CLASSES | ICC_COOL_CLASSES | ICC_LISTVIEW_CLASSES | ICC_PROGRESS_CLASS | ICC_TREEVIEW_CLASSES,
		};
		InitCommonControlsEx(&initCC);
	}

	if (lpCmdLine) {
		_tcscpy(szCmdLine, ANSIToTCHAR(lpCmdLine, NULL, 0));
	}

	if (!(AppInit())) {							// Init the application
		if (!(ProcessCmdLine())) {
			MediaInit();

			RunMessageLoop();					// Run the application message loop
		}
	}
	
	ConfigAppSave();							// Save config for the application

	AppExit();									// Exit the application	

	return 0;
}
