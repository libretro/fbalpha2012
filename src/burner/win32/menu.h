// FBAS menu
#ifndef _MENU_H_
#define _MENU_H_

// variables
extern bool bMenuEnabled;
extern HMENU hMainMenu;				// Handle to main menu
extern HMENU hMenuPopup;			// Handle to a popup version of the menu
extern HMENU hMenuSelPop;			// Handle to gamelist context menu
extern bool bModelessMenu;			// use modeless menu
extern int menuNewStyle;

// menu type
enum MENU_TYPE {
	MENUT_FILE = 0, MENUT_SETTING, MENUT_GAME, MENUT_MISC, MENUT_HELP,
	MENUT_ALL
};

int menuCreate();
void menuDestroy();
void menuReinit();
void menuSync(MENU_TYPE type);
int menuModifyAccString(unsigned int id, const TCHAR* accel);

void createImageMenu(HWND, HMENU);	// Create ownerdraw menu
void removeImageMenu(HMENU);		// Remove ownerdraw menu

#endif // _MENU_H_
