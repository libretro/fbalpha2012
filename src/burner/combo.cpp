//------------------------------------------------------------------------------
//                     Input Macro driver  v0.5a
//
//                         written by Gangta
//                      gangtakun@hotmail.com
//                    ported and fixed by emufan
//------------------------------------------------------------------------------
//
//Credits:
//
// - HyperYagami: External file support is based on his cheat engine
// - Jyaku: For online/offline testing and reporting bugs
// - Mr K aka MHz: Ideas of input macro came from Kawaks input macro
// - Emufan: For porting the original FBA Combo source
//
//-------------------------------------------------------------------------------
//
//Version History:
//
// v0.1  - Primitive input macro
//         Each macro is hard coded into the game driver
// v0.2  - Intput macro can be disabled or enabled
//       - Added more input macro types (normal, auto, block)
// v0.3  - External macro support based on HyperYagami's cheat engine.
//         Write your own input macro for each game:)
// v0.4  - Added some shortcut moves and ~xx for holding xx frames
//         Switched the style similar to Kawaks macro
// v0.4a - Fixed weird crash bug (thanks to Jyaku for testing)
// v0.5  - Increased macro capacity and fixed some minor bugs (made by emufan)
// v0.5a - Source code cleanup (updated by regret)
//
//-------------------------------------------------------------------------------
//
//How to write input macro:
//   explained in macros.dat in config\macros dir and in howtomacro.txt
//
/////////////////////////////////////////////////////////////////////////////////

#ifndef NO_COMBO

#include "burner.h"
#include "combo.h"

#define COMBO_MAX_CHAR_PER_LINE	1200
#define COMBO_MAX_TOKENS		5
#define MAX_COMBO_LEN			1024	// maximum length of move per each combo
#define MAX_COMBO_NAME			120		// maximum length of name per each combo
#define MAX_NUM_COMBO			100		// maximum # of combo per game, (100 / Player)
#define MAX_PLAYER				4
#define MAX_BUTTONS				8
#define MAX_ARROWS				5

static unsigned char *arrows[MAX_PLAYER][MAX_ARROWS];	// [which player][which arrow]
static unsigned char *buttons[MAX_PLAYER][MAX_BUTTONS];	// [which player][which button]
static int            buttonSize;						// number of buttons per player

static char  masterCombo[MAX_NUM_COMBO][MAX_COMBO_LEN];	// master combo
static char  holdKeys[MAX_NUM_COMBO][MAX_COMBO_LEN];	// holding buttons/arrows
static char* combo_ptr[MAX_NUM_COMBO];					// switches between master/hold/shortcut
static char* temp_ptr[MAX_NUM_COMBO];					// where to continue on the master combo
static int   Player[MAX_NUM_COMBO];						// which player owns each combo
static char  comboType[MAX_NUM_COMBO];					// type of each combo

// flags
static short comboState[MAX_NUM_COMBO];					// state of each combo
static short reverseCondition[MAX_NUM_COMBO];			// for reversing 360/720 moves

// things that will be exported
unsigned char Combo[MAX_NUM_COMBO] = {0,};				// if Combo[0]=1, combo[0] will be started
static int nForward = 0;

// contains the names for each combo
// DrvCombo[n].szName will point to this later
static char szComboName[MAX_NUM_COMBO][MAX_COMBO_NAME] = {{'\0',},};

// valid shortcut move list
static char QCF[] = "D,DF,F,";							// quarter circle forward
static char QCB[] = "D,DB,B,";							// quarter circle back
static char HCF[] = "B,BD,D,DF,F,";						// half circle forward
static char HCB[] = "F,FD,D,DB,B,";						// half circle back
static char n360[] = "F,FD,D,DB,B,BU,U,";				//                 clockwise 270
static char r360[] = "B,BD,D,DF,F,FU,U,";				//         counter clockwise 270
static char n720[] = "F,FD,D,DB,B,BU,U,UF,F,FD,D,DB,B,BU,U,";	//         clockwise 630
static char r720[] = "B,BD,D,DF,F,FU,U,UB,B,BD,D,DF,F,FU,U,";	// counter clockwise 630
static char nDRG[] = "F,D,DF,";							// right, down right, right
static char rDRG[] = "B,D,DB,";							// left,  down left,  left
static char Empty[]= "\0";

BurnInputInfo DrvMacro[] = {
	{"", BIT_DIGITAL, Combo + 0,  ""},
	{"", BIT_DIGITAL, Combo + 1,  ""},
	{"", BIT_DIGITAL, Combo + 2,  ""},
	{"", BIT_DIGITAL, Combo + 3,  ""},
	{"", BIT_DIGITAL, Combo + 4,  ""},
	{"", BIT_DIGITAL, Combo + 5,  ""},
	{"", BIT_DIGITAL, Combo + 6,  ""},
	{"", BIT_DIGITAL, Combo + 7,  ""},
	{"", BIT_DIGITAL, Combo + 8,  ""},
	{"", BIT_DIGITAL, Combo + 9,  ""},
	{"", BIT_DIGITAL, Combo + 10, ""},
	{"", BIT_DIGITAL, Combo + 11, ""},
	{"", BIT_DIGITAL, Combo + 12, ""},
	{"", BIT_DIGITAL, Combo + 13, ""},
	{"", BIT_DIGITAL, Combo + 14, ""},
	{"", BIT_DIGITAL, Combo + 15, ""},
	{"", BIT_DIGITAL, Combo + 16, ""},
	{"", BIT_DIGITAL, Combo + 17, ""},
	{"", BIT_DIGITAL, Combo + 18, ""},
	{"", BIT_DIGITAL, Combo + 19, ""},
	{"", BIT_DIGITAL, Combo + 20, ""},
	{"", BIT_DIGITAL, Combo + 21, ""},
	{"", BIT_DIGITAL, Combo + 22, ""},
	{"", BIT_DIGITAL, Combo + 23, ""},
	{"", BIT_DIGITAL, Combo + 24, ""},
	{"", BIT_DIGITAL, Combo + 25, ""},
	{"", BIT_DIGITAL, Combo + 26, ""},
	{"", BIT_DIGITAL, Combo + 27, ""},
	{"", BIT_DIGITAL, Combo + 28, ""},
	{"", BIT_DIGITAL, Combo + 29, ""},
	{"", BIT_DIGITAL, Combo + 30, ""},
	{"", BIT_DIGITAL, Combo + 31, ""},
	{"", BIT_DIGITAL, Combo + 32, ""},
	{"", BIT_DIGITAL, Combo + 33, ""},
	{"", BIT_DIGITAL, Combo + 34, ""},
	{"", BIT_DIGITAL, Combo + 35, ""},
	{"", BIT_DIGITAL, Combo + 36, ""},
	{"", BIT_DIGITAL, Combo + 37, ""},
	{"", BIT_DIGITAL, Combo + 38, ""},
	{"", BIT_DIGITAL, Combo + 39, ""},
	{"", BIT_DIGITAL, Combo + 40, ""},
	{"", BIT_DIGITAL, Combo + 41, ""},
	{"", BIT_DIGITAL, Combo + 42, ""},
	{"", BIT_DIGITAL, Combo + 43, ""},
	{"", BIT_DIGITAL, Combo + 44, ""},
	{"", BIT_DIGITAL, Combo + 45, ""},
	{"", BIT_DIGITAL, Combo + 46, ""},
	{"", BIT_DIGITAL, Combo + 47, ""},
	{"", BIT_DIGITAL, Combo + 48, ""},
	{"", BIT_DIGITAL, Combo + 49, ""},
	{"", BIT_DIGITAL, Combo + 50, ""},
	{"", BIT_DIGITAL, Combo + 51, ""},
	{"", BIT_DIGITAL, Combo + 52, ""},
	{"", BIT_DIGITAL, Combo + 53, ""},
	{"", BIT_DIGITAL, Combo + 54, ""},
	{"", BIT_DIGITAL, Combo + 55, ""},
	{"", BIT_DIGITAL, Combo + 56, ""},
	{"", BIT_DIGITAL, Combo + 57, ""},
	{"", BIT_DIGITAL, Combo + 58, ""},
	{"", BIT_DIGITAL, Combo + 59, ""},
	{"", BIT_DIGITAL, Combo + 60, ""},
	{"", BIT_DIGITAL, Combo + 61, ""},
	{"", BIT_DIGITAL, Combo + 62, ""},
	{"", BIT_DIGITAL, Combo + 63, ""},
	{"", BIT_DIGITAL, Combo + 64, ""},
	{"", BIT_DIGITAL, Combo + 65, ""},
	{"", BIT_DIGITAL, Combo + 66, ""},
	{"", BIT_DIGITAL, Combo + 67, ""},
	{"", BIT_DIGITAL, Combo + 68, ""},
	{"", BIT_DIGITAL, Combo + 69, ""},
	{"", BIT_DIGITAL, Combo + 70, ""},
	{"", BIT_DIGITAL, Combo + 71, ""},
	{"", BIT_DIGITAL, Combo + 72, ""},
	{"", BIT_DIGITAL, Combo + 73, ""},
	{"", BIT_DIGITAL, Combo + 74, ""},
	{"", BIT_DIGITAL, Combo + 75, ""},
	{"", BIT_DIGITAL, Combo + 76, ""},
	{"", BIT_DIGITAL, Combo + 77, ""},
	{"", BIT_DIGITAL, Combo + 78, ""},
	{"", BIT_DIGITAL, Combo + 79, ""},
	{"", BIT_DIGITAL, Combo + 80, ""},
	{"", BIT_DIGITAL, Combo + 81, ""},
	{"", BIT_DIGITAL, Combo + 82, ""},
	{"", BIT_DIGITAL, Combo + 83, ""},
	{"", BIT_DIGITAL, Combo + 84, ""},
	{"", BIT_DIGITAL, Combo + 85, ""},
	{"", BIT_DIGITAL, Combo + 86, ""},
	{"", BIT_DIGITAL, Combo + 87, ""},
	{"", BIT_DIGITAL, Combo + 88, ""},
	{"", BIT_DIGITAL, Combo + 89, ""},
	{"", BIT_DIGITAL, Combo + 90, ""},
	{"", BIT_DIGITAL, Combo + 91, ""},
	{"", BIT_DIGITAL, Combo + 92, ""},
	{"", BIT_DIGITAL, Combo + 93, ""},
	{"", BIT_DIGITAL, Combo + 94, ""},
	{"", BIT_DIGITAL, Combo + 95, ""},
	{"", BIT_DIGITAL, Combo + 96, ""},
	{"", BIT_DIGITAL, Combo + 97, ""},
	{"", BIT_DIGITAL, Combo + 98, ""},
	{"", BIT_DIGITAL, Combo + 99, ""},
};

// find and return the shortcut move
static inline char* ShortMove(char* move)
{
	if (strncmp(move, "QCF", 3) == 0) return QCB;
	if (strncmp(move, "QCB", 3) == 0) return QCF;
	if (strncmp(move, "HCF", 3) == 0) return HCB;
	if (strncmp(move, "HCB", 3) == 0) return HCF;
	if (strncmp(move, "+360", 4) == 0) return n360;
	if (strncmp(move, "-360", 4) == 0) return r360;
	if (strncmp(move, "+720", 4) == 0) return n720;
	if (strncmp(move, "-720", 4) == 0) return r720;
	if (strncmp(move, "+DRG", 4) == 0) return rDRG;
	if (strncmp(move, "-DRG", 4) == 0) return nDRG;
	return Empty;    // shortcut move not found
}

// find and return the reverse equivalent shortcut move
static inline char* ReverseShortMove(char* move)
{
	if (strncmp(move, "QCF", 3) == 0) return QCF;
	if (strncmp(move, "QCB", 3) == 0) return QCB;
	if (strncmp(move, "HCF", 3) == 0) return HCF;
	if (strncmp(move, "HCB", 3) == 0) return HCB;
	if (strncmp(move, "+360", 4) == 0) return r360;
	if (strncmp(move, "-360", 4) == 0) return n360;
	if (strncmp(move, "+720", 4) == 0) return r720;
	if (strncmp(move, "-720", 4) == 0) return n720;
	if (strncmp(move, "+DRG", 4) == 0) return nDRG;
	if (strncmp(move, "-DRG", 4) == 0) return rDRG;
	return Empty;    // shortcut move not found
}

//===================================================//
//                                                   //
// InitComboButtons()                                //
//                                                   //
// rewritten by emufan                               //
//===================================================//
static void InitComboButtons()
{
	if (nInputMacroEnabled == 0)
		return;

	DrvCombo = DrvMacro; // init DrvCombo

	int nInpCount = 0; // total inputs count (sub combo count)
	for (unsigned int i = 0; i < 0x1000; i++) {
		if (BurnDrvGetInputInfo(NULL, i)) {
			nInpCount = i - nComCount;
			break;
		}
	}

	int nButtons = 0; // buttons per player
	int nPlayerTemp = 0;
	BurnInputInfo bii;
	for (int i = 0; i < nInpCount; i++) {
		memset(&bii, 0, sizeof(bii));
		BurnDrvGetInputInfo(&bii, i);

		if ((bii.szName[0] == 'P' || bii.szName[0] == 'p') && bii.szName[1] >= '1' && bii.szName[1] <= '4')
		{
			int nPlayer = bii.szName[1] - '1';

			if (nPlayer != nPlayerTemp) {
				// if it is next player
				buttonSize = nButtons;
				nButtons = 0;
			}
			if (strncmp(" fire", bii.szInfo + 2, 5) == 0 && nButtons < MAX_BUTTONS)
			{
				nButtons++;
				buttons[nPlayer][nButtons] = bii.pVal;
			}
			if (strncmp(" up", bii.szInfo + 2, 3) == 0)
				arrows[nPlayer][1] = bii.pVal;
			if (strncmp(" down", bii.szInfo + 2, 5) == 0)
				arrows[nPlayer][2] = bii.pVal;
			if (strncmp(" left", bii.szInfo + 2, 5) == 0)
				arrows[nPlayer][3] = bii.pVal;
			if (strncmp(" right", bii.szInfo + 2, 5) == 0)
				arrows[nPlayer][4] = bii.pVal;

			nPlayerTemp = nPlayer;
		}
	}
}

// process the "macros.dat" and build the combo list
void __cdecl ConstructComboList()
{
	if (nInputMacroEnabled == 0)
		return;

	// open the macros.dat
	FILE* fp = fopen("config\\macros\\macros.dat", "rt");
	if (fp == NULL) {
		nInputMacroEnabled = 0;
		nComCount = 0; // no combo file -> no combo
		return;
	}

	ResetComboList();

	unsigned int j = 0;
	unsigned char bShouldQuit = 0;

	char szName[MAX_PATH],                           // will contain zipname of the parent
	     szLine[COMBO_MAX_CHAR_PER_LINE],
	     cComboType[MAX_NUM_COMBO],
	     szComboTemp[MAX_NUM_COMBO][MAX_COMBO_LEN],
	     szComboNameTemp[MAX_NUM_COMBO][MAX_COMBO_NAME];

	char *szToken = NULL,                            // string before next ':'
	     *szComboTokens[COMBO_MAX_TOKENS] = {NULL,};  // array of strings seperated by ':'

	// total number of combos
	nComCount = 0;

	// Initialize arrow pointers & button pointers for each player
	InitComboButtons();

	// get the driver zip name (fixed by FerchogtX)
	// case 1: Clones
	if (BurnDrvGetTextA(DRV_PARENT) && (BurnDrvGetFlags() & BDF_CLONE)) {
		sprintf(szName, "%s", BurnDrvGetTextA(DRV_PARENT));
		// case 2: Parents that uses ROMs from other games
	}
	else if (BurnDrvGetTextA(DRV_PARENT)) {
		sprintf(szName, "%s", BurnDrvGetTextA(DRV_PARENT));
		// case 3: Parents
	} else {
		sprintf(szName, "%s", BurnDrvGetTextA(DRV_NAME));
	}

	for (;;) {
		// if end of file, quit processing the file
		if (fgets(szLine, sizeof(szLine), fp) == NULL) break;

		// skip comment line
		if ((szLine[0] == '/') && (szLine[1] == '/')) continue;
		// NULL-ify the last array element anyway
		szLine[COMBO_MAX_CHAR_PER_LINE-1] = 0;
		// Get rid of the linefeed at the end
		size_t nLen = strlen(szLine);
		if (szLine[nLen - 1] == 10) {
			szLine[nLen - 1]=0;
			nLen--;
		}

		// cleanups for the later sanity checks
		for (j = 0; j < COMBO_MAX_TOKENS; j++) {
			if (szComboTokens[j] != NULL)
				szComboTokens[j] = NULL;
			else break;
		}

		// Tokenize the input string (get the string before ":")
		szToken = strtok(szLine, ":");
		j = 0;

		while ((szToken!=NULL) && (j<COMBO_MAX_TOKENS)) {
			szComboTokens[j] = szToken;
			// if not for this game
			// don't even bother the rest of the string
			if ((j==1) && (_stricmp(szComboTokens[j], szName))) {
				if (bShouldQuit == 0) bShouldQuit = 1;   // macro is not for this game
				else bShouldQuit = 2;                    // macros for this game have ended
				break;
			}
			j++;
			szToken = strtok(NULL, ":");                 // get next token
		}

		// if macro is not for this game, skip this macro
		if (bShouldQuit == 1) {
			bShouldQuit = 0;
			continue;
		}

		// if macros for this game have ended, quit processing the file
		if (bShouldQuit == 2) {
			// clean up
			if (szToken != NULL) {
				free(szToken);
				szToken = NULL;
			}
			break;
		}

		// the macro is for this game

		// This is NESSESARY for avoiding problems with "certain" games (reincorporated by FerchogtX)
		// if macro type = e, change clone name to parent and skip this macro
		if (j==3 && szComboTokens[0][0]=='e') {
			// type e: 3 tokens;
			strcpy(szName, szComboTokens[2]);
			continue;
		}

		// if type != e and the macro is for this game, process the macro
		else if (j) {
			++nComCount;

			cComboType[nComCount-1] = szComboTokens[0][0];
			strcpy(szComboTemp[nComCount-1], szComboTokens[2]);
			strcpy(szComboNameTemp[nComCount-1], szComboTokens[3]);
		}
	} // end of for(;;)

	// if there exist combo(s) and within allowed number,
	// distribute combo(s) to each player
	if (nComCount > 0 && nComCount * nMaxPlayers <= MAX_NUM_COMBO) {
		int count = 0;
		char player = '1';

		for (int p = 0; p < nMaxPlayers; p++, player++) {
			for (unsigned int c = 0; c < nComCount; c++, count++) {
				comboState[count] = 0;
				comboType[count] = cComboType[c];

				// first character of the combo name is always 'P'
				strcpy(szComboName[count], "Px ");

				// write the player number to each combo name
				szComboName[count][1] = player;

				// append the combo names read from the file
				strcat(szComboName[count]+3, szComboNameTemp[c]);
				// make sure the last character of combo name is null
				szComboName[count][MAX_COMBO_NAME] = '\0';

				// copy the combo moves read from the file
				strcat(masterCombo[count], szComboTemp[c]);
				strcat(masterCombo[count], ",");
				// make sure the last character of combo move is null
				masterCombo[count][MAX_COMBO_LEN] = '\0';
			}
		}
	} // end of if(nComCount)

	nComCount = nComCount*nMaxPlayers;         // change # count from (per player) to (total)
	for (unsigned int c = 0; c < nComCount; c++) {
		DrvCombo[c].szName = szComboName[c];   // set combo names in the DrvCombo structure
		DrvCombo[c].szInfo = szComboName[c];   // set combo infos in the DrvCombo structure

		// set player value for the combo
		switch (szComboName[c][1])
		{
			case '1':
				Player[c] = 0;
				break; // player 1
			case '2':
				Player[c] = 1;
				break; // player 2
			case '3':
				Player[c] = 2;
				break; // player 3
			case '4':
				Player[c] = 3;
				break; // player 4
		}
	}

	// finished constructing combo, clean up
#ifndef _DEBUG
	free(szToken);
#endif

	for (int i = 0; i < COMBO_MAX_TOKENS; i++) {
		if (szComboTokens[i]) {
#ifndef _DEBUG
			free(szComboTokens[i]);
#endif
			szComboTokens[i] = NULL;
		}
	}
	if (fp) {
		fclose(fp);
		fp = NULL;
	}
} // end of ConstructComboList()

// go through player[n]'s buttons/arrows and clear them
// this is only used for type 'b' combo to block other inputs
static inline void ClearInput(int n)
{
	int i;
	for (i = 1; i <= buttonSize; i++) // not always 6 buttons
		*(buttons[Player[n]][i]) = 0;
	for (i = 1; i <= 4; i++)          // always 4 arrows (up, down, left, right)
		*(arrows[Player[n]][i]) = 0;
}

// build the holding keys/arrows
static inline char* BuildHoldKeys(int n)
{
	static char* idx_ptr = --combo_ptr[n]; // go to first possible hold key
	static int idx = 0;

	// build the hold keys/directions
	while (idx_ptr != masterCombo[n] && *idx_ptr != ',')
		holdKeys[n][idx++] = *(idx_ptr--);

	// get the last hold key
	if (idx_ptr == masterCombo[n])
		holdKeys[n][idx++] = *idx_ptr;

	// mark the end of holding keys/arrows
	holdKeys[n][idx++] = ',';
	holdKeys[n][idx] = '\0';

	return holdKeys[n];
}

// finds and returns a pointer to the shortcut move
static char* GetShortMove(int n)
{
	static char* idx_ptr = ++combo_ptr[n]; // go to the first character of the short move

	// if it is a clockwise shortcut move,
	if (*idx_ptr == '+') {
		// if player[n] is holding left, return the reverse equivalent
		if (reverseCondition[n] == 3)
			return ReverseShortMove(idx_ptr);
		// otherwise, just return the normal short move
		else
			return ShortMove(idx_ptr);
	}

	// if it is a counter clockwise shortcut move,
	if (*idx_ptr == '-') {
		// if player[n] is holding right, return the reverse equivalent
		if (reverseCondition[n] == 4)
			return ReverseShortMove(idx_ptr);
		// otherwise, just return the normal short move
		else
			return ShortMove(idx_ptr);
	}

	return ShortMove(idx_ptr);
}

// Process Combo[n]
static void DoCombo(int n)
{
	static bool bShortMoveStarted[MAX_NUM_COMBO] = {false,}; // see if in shortcut move or not
	static bool bHoldStarted[MAX_NUM_COMBO] = {false,};      // see if in holding or not
	static int  nFrameCounter[MAX_NUM_COMBO] = {0,};         // counter for holding xx frames

	// if combo[n] is just starting
	if (combo_ptr[n] == NULL) {
		// start with the master combo
		combo_ptr[n] = masterCombo[n];

		// set conditions for reverse moves
		if (*(arrows[Player[n]][3]) == 1)      // if player[n] is holding left
			reverseCondition[n] = 3;
		else if (*(arrows[Player[n]][4]) == 1) // if player[n] is holding right
			reverseCondition[n] = 4;
		else
			reverseCondition[n] = 0;
	}

	// if type 'b' combo is executing, block other input
	if (comboType[n] == 'b')
		ClearInput(n);

	// process each keys/arrows
	do {
		switch (*(combo_ptr[n])) {
			case '~':
				bHoldStarted[n] = true;                  // mark holding started
				nFrameCounter[n] = atoi(combo_ptr[n]+1); // conver string to integer

				// find and save the position of the next move
				temp_ptr[n] = strpbrk(combo_ptr[n]+1, ",");
				temp_ptr[n]++;                       // skip the ',' in the next move
				combo_ptr[n] = BuildHoldKeys(n);     // point to holdKeys[n]
				return;                              // for the first time, hold keys are already pressed, just return

			case '(':
				bShortMoveStarted[n] = true;         // mark hort cut move started

				// find and save the position of the next move
				temp_ptr[n] = strpbrk(combo_ptr[n]+1, ",");
				temp_ptr[n]++;                       // skip the ',' in the next move
				combo_ptr[n] = GetShortMove(n);      // point to the shortcut move
				continue;                            // do not increment move pointer at this time

			case '1':
				*(buttons[Player[n]][1]) = 1; break; // LP
			case '2':
				*(buttons[Player[n]][2]) = 1; break; // MP
			case '3':
				*(buttons[Player[n]][3]) = 1; break; // HP
			case '4':
				*(buttons[Player[n]][4]) = 1; break; // LK
			case '5':
				*(buttons[Player[n]][5]) = 1; break; // MK
			case '6':
				*(buttons[Player[n]][6]) = 1; break; // HK
			case 'U':
				*(arrows[Player[n]][1]) = 1; break;  // up
			case 'D':
				*(arrows[Player[n]][2]) = 1; break;  // down
			case 'L':
				*(arrows[Player[n]][3]) = 1; break;  // left
			case 'R':
				*(arrows[Player[n]][4]) = 1; break;  // right
			case 'F':                                // Forward
				if (Player[n] == 0) {
					if (nForward == 0) *(arrows[Player[n]][3]) = 1;
					else *(arrows[Player[n]][4])=1;
				} else {
					if (nForward == 1) *(arrows[Player[n]][3]) = 1;
					else *(arrows[Player[n]][4])=1;
				}
				break;
			case 'B':                                // Back
				if (Player[n] == 0) {
					if (nForward == 0) *(arrows[Player[n]][4]) = 1;
					else *(arrows[Player[n]][3]) = 1;
				} else {
					if (nForward == 1) *(arrows[Player[n]][4]) = 1;
					else *(arrows[Player[n]][3]) = 1;
				}
				break;

			case '\0':                               // finished something

			if (bShortMoveStarted[n]) {
				// finished shortcut move
				bShortMoveStarted[n] = false;        // mark shortcut move finished
				combo_ptr[n] = temp_ptr[n];          // back to master move
				continue;                            // do not increment combo pointer at this time
			}
			if (bHoldStarted[n]) {
				// finished holding buttons/arrows for this frame
				if (--nFrameCounter[n] == 0) {
					// finished holding for xx frames
					bHoldStarted[n] = false;         // mark hold finished
					combo_ptr[n] = temp_ptr[n];      // back to master move
				} else {
					combo_ptr[n] = holdKeys[n];
				}
				continue;                            // do not increment combo pointer at this time
			}

			// master combo is finished
			combo_ptr[n]= NULL;

			switch(comboType[n])
			{
				case 'a': comboState[n] = 0; break;  // type 'a' combo ended, repeat while holding
				case 'b': comboState[n] = 2; break;  // type 'b' combo ended, don't repeat while holding
				case 'n': comboState[n] = 2; break;  // type 'n' combo ended, don't repeat while holding
			} // end of switch

			return;                                  // master combo ended, stop processing Combo[n]
		} // end of switch

		++combo_ptr[n];  // go to next move/button
	} while (combo_ptr[n] && *(combo_ptr[n]) != ',');
}  // finished processing Combo[n] for this frame

// Process intput combo
void __cdecl ProcessCombo()
{
	if (nInputMacroEnabled == 0)
		return;

	static unsigned int i = 0;

	// set flags for which combos are started
	for (i = 0; i < nComCount; i++) {
		if (Combo[i]) {
			// if combo button pressed
			if (comboState[i] == 0)     // if 'a' type combo finished
				comboState[i] = 1;      // flag = process combo while holding
		}
		else {
			// if combo button not pressed
			if (comboState[i] == 2)     // if 'n' type or 'b' type combo finished
				comboState[i] = 0;      // flag = do not process combo while holding
		}
	}

	// process each combo
	for (i = 0; i < nComCount; i++) {
		if (comboState[i] == 1)     // if flag = process combo
			DoCombo(i);             // do the combo
	}
}

// run this on exit in order to reset all the combos
void ResetComboList()
{
	if (nInputMacroEnabled == 0)
		return;

	for (unsigned int i = 0; i < nComCount; i++)
		masterCombo[i][0] = '\0';
}

#endif
