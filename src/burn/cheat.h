#ifndef CHEAT_H
#define CHEAT_H

#define CHEAT_MAX_ADDRESS ( 64)
#define CHEAT_MAX_OPTIONS (192)
#define CHEAT_MAX_NAME	  (128)

extern bool bCheatsAllowed;

struct CheatAddressInfo {
	int nCPU;
	int nAddress;
	unsigned int nValue;
	unsigned int nOriginalValue;
};

struct CheatOption {
	TCHAR szOptionName[CHEAT_MAX_NAME];
	struct CheatAddressInfo AddressInfo[CHEAT_MAX_ADDRESS + 1];
};

struct CheatInfo {
	struct CheatInfo* pNext;
	struct CheatInfo* pPrevious;
	int nType;									// Cheat type
	int nStatus;								// 0 = Inactive
	int nCurrent;								// Currently selected option
	int nDefault;								// Default option
	TCHAR szCheatName[CHEAT_MAX_NAME];
	struct CheatOption* pOption[CHEAT_MAX_OPTIONS];
};

extern CheatInfo* pCheatInfo;

int cheatUpdate();
int cheatEnable(int nCheat, int nOption);
int cheatApply();
int cheatInit();
void cheatExit(bool exitCpuReg = true);


#ifndef NO_CHEATSEARCH
// Cheat search
struct CheatSearchInfo {
	unsigned int size;
	unsigned char* RAM;
	unsigned char* CRAM;
	int* ALL_BITS;
};

extern CheatSearchInfo cheatSearchInfo;

int cheatSearchInit();
void cheatSearchExit();
unsigned char cheatSearchGet(unsigned int address);
void cheatSearchCopyRAM(unsigned char* ram);
#endif

#endif /* NO_CHEATSEARCH */
