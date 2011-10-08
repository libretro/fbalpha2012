// autofire support, added by regret
// only support P1 and P2 first 4 buttons
// it's easy to add autofire for other players/buttons but I have no interest ^^

/* changelog:
 update 5: extend autofire support to 6 buttons
 update 4: chang autofire clear timer, add method2 for pgm games
 update 3: change autofire delay
 update 2: add check button state, it will not conflict with combo keys
 update 1: create
*/

#ifndef NO_AUTOFIRE

#include "burner.h"
#include "autofire.h"

int nAutofireEnabled = 1;
unsigned int autofireDefaultDelay = 3;	// autofire default delay (1 ~ 99)
unsigned int autofireDelay = 3;			// autofire delay (1 ~ 99)

struct AutofireButton {
	unsigned char* value;
	int inpIndex;
};

static AutofireButton autofires[MAX_PLAYER][MAX_AUTOFIRE_BUTTONS]; // [player][button]
static bool autofireOn[MAX_PLAYER][MAX_AUTOFIRE_BUTTONS];
//static int autofireSize; // number of buttons per player
static int delayCounter = autofireDelay;
static bool method2 = false; // another autofire clear timer

// check other buttons state
// only if [player][button] clicked, return true
static inline bool checkButtonState(const int& player, const int& fire)
{
	if (player < P_1 || player > P_4 || fire < F_1 || fire > F_6)
		return false;

	static struct GameInp* gi = GameInp;
	static unsigned short val = 0;

	for (int j = 0; j < MAX_AUTOFIRE_BUTTONS; j++) {
		if (j != fire && !autofireOn[P_1][F_1 + j]
			&& autofires[P_1 + player][F_1 + j].value && autofires[P_1 + player][F_1 + j].inpIndex >= 0) {
			val = 0;
			gi = GameInp + autofires[P_1 + player][F_1 + j].inpIndex;

			if (gi->Input.pVal)
				val = *gi->Input.pVal;

			if (val == 1)
				return false;
		}
	}

	return true;
}

// clear button value
static inline void clearAutofire()
{
	for (int i = 0; i < MAX_PLAYER; i++) {
		for (int j = 0; j < MAX_AUTOFIRE_BUTTONS; j++) {
			if (autofireOn[P_1 + i][F_1 + j] && autofires[P_1 + i][F_1 + j].value
				&& *(autofires[P_1 + i][F_1 + j].value) && checkButtonState(P_1 + i, F_1 + j)) {
				*(autofires[P_1 + i][F_1 + j].value) = 0;
			}
		}
	}
}

// reset data
static inline void resetAutofire()
{
	for (int i = 0; i < MAX_PLAYER; i++) {
		for (int j = 0; j < MAX_AUTOFIRE_BUTTONS; j++) {
			autofires[i][j].value = NULL;
			autofires[i][j].inpIndex = -1;
			autofireOn[i][j] = false;
		}
	}
}

// init autofire buttons
void __cdecl initAutofire()
{
	if (!nAutofireEnabled)
		return;

	// check delay value
	if (autofireDelay < 1 || autofireDelay > 99)
		autofireDelay = autofireDefaultDelay;

	delayCounter = autofireDelay;

	// choose autofire method
	method2 = false;
	int flag = (BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK);
	if (flag == HARDWARE_IGS_PGM)
		method2 = true;

	// reset values
	resetAutofire();

	// get button info
	int nButtons = 0; // buttons per player
	int nPlayer = 0;
	int nPlayerTemp = 0;
	BurnInputInfo bii;

	for (unsigned int i = 0; i < nGameInpCount; i++) {
		memset(&bii, 0, sizeof(bii));
		BurnDrvGetInputInfo(&bii, i);

		if (!bii.szName || !bii.pVal)
			continue;

		if ((bii.szName[0] == 'P' || bii.szName[0] == 'p')
			&& bii.szName[1] >= '1' && bii.szName[1] <= '4') {
			nPlayer = bii.szName[1] - '1';

			if (nPlayer != nPlayerTemp) {
				// if it is next player
//				autofireSize = nButtons;
				nButtons = 0;
			}
			if (!strncmp(" fire", bii.szInfo + 2, 5) && nButtons < MAX_AUTOFIRE_BUTTONS) {
				autofires[nPlayer][nButtons].value = bii.pVal;
				autofires[nPlayer][nButtons].inpIndex = i;
				nButtons++;
			}

			nPlayerTemp = nPlayer;
		}
	}
}

// do autofire check every frame
void __cdecl doAutofire()
{
	if (!nAutofireEnabled)
		return;

	// autofire delay
	if (delayCounter > 0) {
		delayCounter--;

		if (method2)
			clearAutofire();

		return;
	}
	delayCounter = autofireDelay;

	if (!method2)
		clearAutofire();
}

// enable/disable autofire buttons
int setAutofire(const int& player, const int& fire, bool state)
{
	if (!nAutofireEnabled)
		return 0;

	if (player < P_1 || player > P_4 || fire < F_1 || fire > F_6)
		return 1;

	autofireOn[player][fire] = state;

#if 0
	// add message
	TCHAR msg[64] = _T("");
	if (autofireOn[player][fire])
		sprintf(msg, FBALoadStringEx(IDS_AUTOFIRE_ON), player + 1, fire + 1);
	else
		sprintf(msg, FBALoadStringEx(IDS_AUTOFIRE_OFF), player + 1, fire + 1);
	VidSNewShortMsg(msg);
#endif
	return 0;
}

bool getAutofire(const int& player, const int& fire)
{
	if (!nAutofireEnabled)
		return false;

	if (player < P_1 || player > P_4 || fire < F_1 || fire > F_6)
		return false;

	return autofireOn[player][fire];
}
#endif
