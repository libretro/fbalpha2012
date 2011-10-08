
#define WIN32_LEAN_AND_MEAN

#include "kailleraclient.h"

static HMODULE hDLL = NULL;
static char DLL_Name[MAX_PATH];

int (WINAPI *kailleraGetVersion) (char *);
int (WINAPI *kailleraSetInfos) (kailleraInfos*);
int (WINAPI *kailleraSelectServerDialog)(HWND);
int (WINAPI *kailleraModifyPlayValues)(void *, int);
int (WINAPI *kailleraChatSend)(char *);

int (WINAPI *kailleraInit)(void);
int (WINAPI *kailleraShutdown)(void);
int (WINAPI *kailleraEndGame)(void);

int (WINAPI *kailleraChatSend_cb)(char *);
int (WINAPI *kailleraChatSend_cb_update)(void);

static int WINAPI kailleraNullFunc0(void)
{
	return 0;
}
static int WINAPI kailleraNullFunc1(char *version)
{
	return 0;
}
static int WINAPI kailleraNullFunc2(kailleraInfos *infos)
{
	return 0;
}

static int WINAPI kailleraNullFunc3(HWND parent)
{
	return 0;
}
static int WINAPI kailleraNullFunc4(void *values, int size)
{
	return 0;
}
static void KailleraFuncReset(void)
{
	kailleraGetVersion			 = kailleraNullFunc1;
	kailleraInit				 = kailleraNullFunc0;
	kailleraShutdown			 = kailleraNullFunc0;
	kailleraSetInfos			 = kailleraNullFunc2;
	kailleraSelectServerDialog	 = kailleraNullFunc3;
	kailleraModifyPlayValues	 = kailleraNullFunc4;
	kailleraChatSend			 = kailleraNullFunc1;
	kailleraEndGame				 = kailleraNullFunc0;
	kailleraChatSend_cb			 = kailleraNullFunc1;
	kailleraChatSend_cb_update	 = kailleraNullFunc0;
}

BOOL LoadLibrary_KailleraClient_DLL(const char *fname)
{
	UINT error_mode;

	if (hDLL != NULL)
	{
		return TRUE;
	}

	KailleraFuncReset();

	strcpy (DLL_Name, fname);
	error_mode = SetErrorMode(0);
	hDLL = LoadLibrary(fname);
	SetErrorMode(error_mode);

	if (hDLL == NULL)
		goto err;

	kailleraGetVersion = (int (WINAPI *)(char *))GetProcAddress(hDLL, "_kailleraGetVersion@4");
	if (kailleraGetVersion == NULL)	goto err;
	kailleraInit = (int (WINAPI *)(void))GetProcAddress(hDLL, "_kailleraInit@0");
	if (kailleraInit == NULL)	goto err;
	kailleraShutdown = (int (WINAPI *)(void))GetProcAddress(hDLL, "_kailleraShutdown@0");
	if (kailleraShutdown == NULL)	goto err;
	kailleraSetInfos = (int (WINAPI *)(kailleraInfos*))GetProcAddress(hDLL, "_kailleraSetInfos@4");
	if (kailleraSetInfos == NULL)	goto err;

	kailleraSelectServerDialog = (int (WINAPI *)(HWND))GetProcAddress(hDLL, "_kailleraSelectServerDialog@4");
	if (kailleraSelectServerDialog == NULL)	goto err;
	kailleraModifyPlayValues = (int (WINAPI *)(void *, int))GetProcAddress(hDLL, "_kailleraModifyPlayValues@8");
	if (kailleraModifyPlayValues == NULL)	goto err;
	kailleraChatSend = (int (WINAPI *)(char *))GetProcAddress(hDLL, "_kailleraChatSend@4");
	if (kailleraChatSend == NULL)	goto err;

	kailleraEndGame = (int (WINAPI *)(void))GetProcAddress(hDLL, "_kailleraEndGame@0");
	if (kailleraEndGame == NULL)	goto err;

	kailleraChatSend_cb = kailleraChatSend;
	kailleraChatSend_cb_update = kailleraNullFunc0;

	return TRUE;

err:
	FreeLibrary_KailleraClient_DLL();
	return FALSE;
}


void FreeLibrary_KailleraClient_DLL(void)
{
	DLL_Name[0] = 0;
	if (hDLL != NULL)
	{
		FreeLibrary(hDLL);
		hDLL = NULL;

		KailleraFuncReset();
	}
}

#if 0
static void *tmpfunc[10];
void KailleraClient_DLL_SaveFunc(void)
{
	int i=0;
	if (hDLL != NULL)
	{
		tmpfunc[i++] = (void*)kailleraGetVersion;
		tmpfunc[i++] = (void*)kailleraInit;
		tmpfunc[i++] = (void*)kailleraShutdown;
		tmpfunc[i++] = (void*)kailleraSetInfos;
		tmpfunc[i++] = (void*)kailleraSelectServerDialog;
		tmpfunc[i++] = (void*)kailleraModifyPlayValues;
		tmpfunc[i++] = (void*)kailleraChatSend;
		tmpfunc[i++] = (void*)kailleraEndGame;

		tmpfunc[i++] = (void*)kailleraChatSend_cb;
		tmpfunc[i++] = (void*)kailleraChatSend_cb_update;
	}
}
void KailleraClient_DLL_LoadFunc(void)
{
	int i=0;
	if (hDLL != NULL)
	{
		kailleraGetVersion				= tmpfunc[i++];
		kailleraInit					= tmpfunc[i++];
		kailleraShutdown				= tmpfunc[i++];
		kailleraSetInfos				= tmpfunc[i++];
		kailleraSelectServerDialog		= tmpfunc[i++];
		kailleraModifyPlayValues		= tmpfunc[i++];
		kailleraChatSend				= tmpfunc[i++];
		kailleraEndGame					= tmpfunc[i++];

		kailleraChatSend_cb				= tmpfunc[i++];
		kailleraChatSend_cb_update		= tmpfunc[i++];
	}
}
#endif

BOOL Kaillera_GetVersion(const char *fname, char *version)
{
	HMODULE hTempDLL = NULL;
	UINT error_mode;

	int (WINAPI *Temp_GetKailleraVersion)(char *);

	DLL_Name[MAX_PATH-1] = 0;

	if (hDLL != NULL &&
		!strcmp(DLL_Name, fname))
	{
		kailleraGetVersion(version);
		return TRUE;
	}

	error_mode = SetErrorMode(0);
	hTempDLL = LoadLibrary(fname);
	SetErrorMode(error_mode);
	if (hTempDLL == NULL)
	   goto err;

	Temp_GetKailleraVersion = (int (WINAPI *)(char *))GetProcAddress(hTempDLL, "_kailleraGetVersion@4");
	if (GetVersion == NULL)	goto err;

	Temp_GetKailleraVersion(version);

	FreeLibrary(hTempDLL);
	return TRUE;

err:
	if (hTempDLL != NULL)
		FreeLibrary(hTempDLL);
	return FALSE;
}
