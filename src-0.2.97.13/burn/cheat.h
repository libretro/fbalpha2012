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

int CheatUpdate();
int CheatEnable(int nCheat, int nOption);
int CheatApply();
int CheatInit();
void CheatExit();

#define CHEATSEARCH_SHOWRESULTS		3
extern unsigned int CheatSearchShowResultAddresses[CHEATSEARCH_SHOWRESULTS];
extern unsigned int CheatSearchShowResultValues[CHEATSEARCH_SHOWRESULTS];

int CheatSearchInit();
void CheatSearchExit();
void CheatSearchStart();
unsigned int CheatSearchValueNoChange();
unsigned int CheatSearchValueChange();
unsigned int CheatSearchValueDecreased();
unsigned int CheatSearchValueIncreased();
void CheatSearchDumptoFile();
