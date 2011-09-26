// modeless dialog manager, added by regret

/* changelog:
 update 1: create
*/

#include "burner.h"

map<int, HWND> dialogMap;

void dialogAdd(int id, HWND dialog)
{
	dialogDelete(id);
	dialogMap[id] = dialog;
}

HWND dialogGet(int id)
{
	map<int, HWND>::iterator iter = dialogMap.find(id);
	if (iter != dialogMap.end()) {
		return iter->second;
	}
	return NULL;
}

void dialogDelete(int id)
{
	map<int, HWND>::iterator iter = dialogMap.find(id);
	if (iter != dialogMap.end()) {
		DestroyWindow(iter->second);
		dialogMap.erase(iter);
	}
}

bool dialogIsEmpty()
{
	return dialogMap.empty();
}

void dialogClear()
{
	map<int, HWND>::iterator iter = dialogMap.begin();
	for (; iter != dialogMap.end(); iter++) {
		DestroyWindow(iter->second);
	}
	dialogMap.clear();
}

bool dialogIsDlgMessage(MSG* msg)
{
	if (!msg) {
		return false;
	}

	map<int, HWND>::iterator iter = dialogMap.begin();
	for (; iter != dialogMap.end(); iter++) {
		if (IsDialogMessage(iter->second, msg)) {
			return true;
		}
	}
	return false;
}
