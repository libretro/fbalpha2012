#ifndef _LOCALISE_H_
#define _LOCALISE_H_

extern bool bLanguageActive;
extern TCHAR szLanguage[MAX_PATH];
extern HMODULE hLanguage;

HMODULE FBALocaliseInstance();

void FBALocaliseExit();
int FBALocaliseInit(TCHAR* lanaugae);
int FBALocaliseLoad();

HBITMAP FBALoadBitmap(int id);
HMENU FBALoadMenu(int id);
INT_PTR FBADialogBox(int id, HWND parent, DLGPROC lpDialogFunc);
HWND FBACreateDialog(int id, HWND parent, DLGPROC lpDialogFunc);
int FBALoadString(UINT id, LPTSTR buffer, int maxsize);
TCHAR* FBALoadStringEx(UINT id, bool translate = true);

#endif // _LOCALISE_H_