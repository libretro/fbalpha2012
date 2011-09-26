// icon load module, added by regret
#ifndef _ICONLOAD_H_
#define _ICONLOAD_H_

HICON loadIconFromFile(HINSTANCE instance, const TCHAR* filename, UINT index);
HICON loadIconFromBuffer(const LPBYTE buf, unsigned int size);

int loadIconFromArchive(const TCHAR* arc);
int findIconMap(const char* name);
void drawIcon(bool large, int id, HDC hdc, int x, int y);
int createIconList();
void clearIcons();

#endif // _ICONLOAD_H_