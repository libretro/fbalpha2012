#include "burner.h"
#include "cheat.h"
#include "vid_directx_support.h"
#include "../../lib/kaillera/kailleraclient.h"

const int MAXPLAYER = 4;
static int nPlayerInputs[MAXPLAYER], nCommonInputs, nDIPInputs;
static int nPlayerOffset[MAXPLAYER], nCommonOffset, nDIPOffset;

const int INPUTSIZE = 8 * (4 + 8);
static unsigned char nControls[INPUTSIZE];

#define ID_NETCHAT 999
HWND hwndChat = NULL;

int kNetGame = 0;							// Non-zero if Kaillera is being used

// Inputs are assumed to be in the following order:
// All player 1 controls
// All player 2 controls (if any)
// All player 3 controls (if any)
// All player 4 controls (if any)
// All common controls
// All DIP switches

int KailleraInitInput()
{
	if (nGameInpCount == 0) {
		return 1;
	}

	BurnInputInfo bii;
	memset(&bii, 0, sizeof(bii));

	unsigned int i = 0;

	nPlayerOffset[0] = 0;
	do {
		BurnDrvGetInputInfo(&bii, i);
		i++;
	} while (!_strnicmp(bii.szName, "P1", 2) && i <= nGameInpCount);
	i--;
	nPlayerInputs[0] = i - nPlayerOffset[0];

	for (int j = 1; j < MAXPLAYER; j++) {
		char szString[3] = "P?";
		szString[1] = j + '1';
		nPlayerOffset[j] = i;
		while (!_strnicmp(bii.szName, szString, 2) && i < nGameInpCount) {
			i++;
			BurnDrvGetInputInfo(&bii, i);
		}
		nPlayerInputs[j] = i - nPlayerOffset[j];
	}

	nCommonOffset = i;
	while ((bii.nType & BIT_GROUP_CONSTANT) == 0 && i < nGameInpCount){
		i++;
		BurnDrvGetInputInfo(&bii, i);
	};
	nCommonInputs = i - nCommonOffset;

	nDIPOffset = i;
	nDIPInputs = nGameInpCount - nDIPOffset;

#if 0 && defined FBA_DEBUG
	dprintf(_T("  * Kaillera inputs configured as follows --\n"));
	for (int j = 0; j < MAXPLAYER; j++) {
		dprintf(_T("    p%d offset %d, inputs %d.\n"), j + 1, nPlayerOffset[j], nPlayerInputs[j]);
	}
	dprintf(_T("    common offset %d, inputs %d.\n"), nCommonOffset, nCommonInputs);
	dprintf(_T("    dip offset %d, inputs %d.\n"), nDIPOffset, nDIPInputs);
#endif

	return 0;
}

int KailleraGetInput()
{
	static int i, j, k;

	static BurnInputInfo bii;
	memset(&bii, 0, sizeof(bii));

	// Initialize controls to 0
	memset(nControls, 0, INPUTSIZE);

	// Pack all DIP switches + common controls + player 1 controls
	for (i = 0, j = 0; i < nPlayerInputs[0]; i++, j++) {
		BurnDrvGetInputInfo(&bii, i + nPlayerOffset[0]);
		if (*bii.pVal && bii.nType == BIT_DIGITAL) {
			nControls[j >> 3] |= (1 << (j & 7));
		}
	}
	for (i = 0; i < nCommonInputs; i++, j++) {
		BurnDrvGetInputInfo(&bii, i + nCommonOffset);
		if (*bii.pVal) {
			nControls[j >> 3] |= (1 << (j & 7));
		}
	}

	// Convert j to byte count
	j = (j + 7) >> 3;

	// Analog controls/constants
	for (i = 0; i < nPlayerInputs[0]; i++) {
		BurnDrvGetInputInfo(&bii, i + nPlayerOffset[0]);
		if (*bii.pVal && bii.nType != BIT_DIGITAL) {
			if (bii.nType & BIT_GROUP_ANALOG) {
				nControls[j++] = *bii.pShortVal >> 8;
				nControls[j++] = *bii.pShortVal & 0xFF;
			} else {
				nControls[j++] = *bii.pVal;
			}
		}
	}

	// DIP switches
	for (i = 0; i < nDIPInputs; i++, j++) {
		BurnDrvGetInputInfo(&bii, i + nDIPOffset);
		nControls[j] = *bii.pVal;
	}

	// k has the size of all inputs for one player
	k = j + 1;

	// Send the control block to the Kaillera DLL & retrieve all controls
	if (kailleraModifyPlayValues(nControls, k) == -1) {
		kNetGame = 0;
		kDropped = 0x00; // FBA Combo - extra players input hack
		return 1;
	}

	// Decode Player 1 input block
	for (i = 0, j = 0; i < nPlayerInputs[0]; i++, j++) {
		BurnDrvGetInputInfo(&bii, i + nPlayerOffset[0]);
		if (bii.nType == BIT_DIGITAL) {
			if (nControls[j >> 3] & (1 << (j & 7))) {
				*bii.pVal = 0x01;
			} else {
				*bii.pVal = 0x00;
			}
		}
	}
	for (i = 0; i < nCommonInputs; i++, j++) {
		BurnDrvGetInputInfo(&bii, i + nCommonOffset);
		if (nControls[j >> 3] & (1 << (j & 7))) {
			*bii.pVal = 0x01;
		} else {
			*bii.pVal = 0x00;
		}
	}

	// Convert j to byte count
	j = (j + 7) >> 3;

	// Analog inputs
	for (i = 0; i < nPlayerInputs[0]; i++) {
		BurnDrvGetInputInfo(&bii, i + nDIPOffset);
		if (bii.nType & BIT_GROUP_ANALOG) {
			*bii.pShortVal = (nControls[j] << 8) | nControls[j + 1];
			j += 2;
		}
	}

	// DIP switches
	for (i = 0; i < nDIPInputs; i++, j++) {
		BurnDrvGetInputInfo(&bii, i + nDIPOffset);
		*bii.pVal = nControls[j];
	}

	// Decode other player's input blocks
	for (int l = 1; l < MAXPLAYER; l++) {
		if (nPlayerInputs[l]) {
			for (i = 0, j = k * (l << 3); i < nPlayerInputs[l]; i++, j++) {
				BurnDrvGetInputInfo(&bii, i + nPlayerOffset[l]);
				if (bii.nType == BIT_DIGITAL) {
					if (nControls[j >> 3] & (1 << (j & 7))) {
						*bii.pVal = 0x01;
					} else {
						*bii.pVal = 0x00;
					}
				}
			}

//			for (i = 0; i < nCommonInputs; i++, j++) {
#if 0
				// Allow other players to use common inputs
				BurnDrvGetInputInfo(&bii, i + nCommonOffset);
				if (nControls[j >> 3] & (1 << (j & 7))) {
					*bii.pVal |= 0x01;
				}
#endif
//			}
			j += nCommonInputs;

			// Convert j to byte count
			j = (j + 7) >> 3;

			// Analog inputs/constants
			for (i = 0; i < nPlayerInputs[l]; i++) {
				BurnDrvGetInputInfo(&bii, i + nPlayerOffset[l]);
				if (bii.nType != BIT_DIGITAL) {
					if (bii.nType & BIT_GROUP_ANALOG) {
						*bii.pShortVal = (nControls[j] << 8) | nControls[j + 1];
						j += 2;
					}
				}
			}

#if 0
			// For a DIP switch to be set to 1, ALL players must set it
			for (i = 0; i < nDIPInputs; i++, j++) {
				BurnDrvGetInputInfo(&bii, i + nDIPOffset);
				*bii.pVal &= nControls[j];
			}
#endif
		}
	}

	return 0;
}

static char* CreateKailleraList()
{
	unsigned int nOldDrvSelect = nBurnDrvSelect;
	int nSize = 256 * 1024;
	char* pList = (char*)malloc(nSize);
	char* pName = pList;

	if (pList == NULL) {
		return NULL;
	}

	// Add chat option to the gamelist
	pName += sprintf(pName, "* Chat only");
	pName++;

	if (avOk) {
		// Add all the driver names to the list
		for (nBurnDrvSelect = 0; nBurnDrvSelect < nBurnDrvCount; nBurnDrvSelect++) {

			if (BurnDrvGetFlags() & BDF_GAME_WORKING && getAuditState(nBurnDrvSelect) != AUDIT_FAIL) {
				char* szDecoratedName = decorateKailleraGameName(nBurnDrvSelect);

				if (pName + strlen(szDecoratedName) >= pList + nSize) {
					char* pNewList;
					nSize <<= 1;
					pNewList = (char*)realloc(pList, nSize);
					if (pNewList == NULL) {
						return NULL;
					}
					pName -= (unsigned int)pList;
					pList = pNewList;
					pName += (unsigned int)pList;
				}
				pName += sprintf(pName, "%s", szDecoratedName);
				pName++;
			}
		}
	}

	*pName = '\0';
	pName++;

	nBurnDrvSelect = nOldDrvSelect;

	return pList;
}

void DeActivateChat()
{
	bEditActive = false;
	DestroyWindow(hwndChat);
	hwndChat = NULL;
}

int ActivateChat()
{
	if (!hwndChat) {
		RECT rect;
		GetClientRect(hScrnWnd, &rect);

		DeActivateChat();

		// Create an invisible edit control
		hwndChat = CreateWindow(
			_T("EDIT"), NULL,
			WS_CHILD | ES_LEFT,
			0, rect.bottom - 32, rect.right, 32,
			hScrnWnd, (HMENU)ID_NETCHAT,
			(HINSTANCE)GetWindowLongPtr(hScrnWnd, GWLP_HINSTANCE), NULL);

		EditText[0] = 0;
		bEditTextChanged = true;
		bEditActive = true;

		SendMessage(hwndChat, EM_LIMITTEXT, MAX_CHAT_SIZE, 0);			// Limit the amount of text
	}

	SetFocus(hwndChat);
	return 0;
}

bool ChatActivated()
{
	return hwndChat ? true : false;
}

HWND GetChatWindow()
{
	return hwndChat;
}

static int WINAPI gameCallback(char* game, int player, int numplayers)
{
	bool bFound = false;
	HWND hActive;

	for (nBurnDrvSelect = 0; nBurnDrvSelect < nBurnDrvCount; nBurnDrvSelect++) {

		char* szDecoratedName = decorateKailleraGameName(nBurnDrvSelect);

		if (!strcmp(szDecoratedName, game)) {
			bFound = true;
			break;
		}
	}

	if (!bFound) {
		kailleraEndGame();
		return 1;
	}

	kNetGame = 1;
	kDropped = 0xFF<<numplayers; // FBA Combo - extra players input hack
	hActive = GetActiveWindow();

	bCheatsAllowed = false;								// Disable cheats during netplay
	audio.stop();										// Stop while we load roms
	BurnerDrvInit(nBurnDrvSelect, false);				// Init the game driver
	scrnInit();
	audio.play();										// Restart sound
	VidInit();
	SetFocus(hScrnWnd);

//	dprintf(_T(" ** OSD startnet text sent.\n"));

	TCHAR szTemp1[256];
	TCHAR szTemp2[256];
	VidSAddChatMsg(FBALoadStringEx(IDS_NETPLAY_START), 0xFFFFFF, BurnDrvGetText(DRV_FULLNAME), 0xFFBFBF);
	_sntprintf(szTemp1, sizearray(szTemp1), FBALoadStringEx(IDS_NETPLAY_START_YOU), player);
	_sntprintf(szTemp2, sizearray(szTemp2), FBALoadStringEx(IDS_NETPLAY_START_TOTAL), numplayers);
	VidSAddChatMsg(szTemp1, 0xFFFFFF, szTemp2, 0xFFBFBF);

	RunMessageLoop();

	BurnerDrvExit();
	FBA_KailleraEnd();

	bCheatsAllowed = true;								// reenable cheats netplay has ended

	SetFocus(hActive);
	return 0;
}

static void WINAPI kChatCallback(char* nick, char* text)
{
	TCHAR szTemp[128];
	_sntprintf(szTemp, sizearray(szTemp), _T("%.32hs "), nick);
	VidSAddChatMsg(szTemp, 0xBFBFFF, AtoW(text), 0x7F7FFF);
}

static void WINAPI kDropCallback(char* nick, int playernb)
{
	TCHAR szTemp[128];
	_sntprintf(szTemp, sizearray(szTemp), FBALoadStringEx(IDS_NETPLAY_DROP), playernb, nick);
	VidSAddChatMsg(szTemp, 0xFFFFFF, NULL, 0);

	kDropped |= 1 << (playernb - 1); // FBA Combo - extra players input hack
}

BOOL FBA_KailleraInit()
{
	if (LoadLibrary_KailleraClient_DLL("kailleraclient.dll") == FALSE) {
		MessageBox(hScrnWnd, _T("Couldn't find kailleraclient.dll"), _T(APP_TITLE), MB_OK | MB_ICONERROR);
		return FALSE;
	}
	kailleraInit();

	// turn off macro and autofire
#ifndef NO_COMBO
	nInputMacroEnabled = 0;
#endif
#ifndef NO_AUTOFIRE
	nAutofireEnabled = 0;
#endif

	return TRUE;
}

void DoNetGame()
{
	kailleraInfos ki;
	char tmpver[128];
	char* gameList;

	if (bDrvOkay) {
		BurnerDrvExit();
		scrnTitle();
	}
	menuSync(MENUT_ALL);

	_snprintf(tmpver, 128, APP_TITLE " v%.20ls", szAppBurnVer);

	gameList = CreateKailleraList();

	ki.appName = tmpver;
	ki.gameList = gameList;
	ki.gameCallback = &gameCallback;
	ki.chatReceivedCallback = &kChatCallback;
	ki.clientDroppedCallback = &kDropCallback;
	ki.moreInfosCallback = NULL;

	kailleraSetInfos(&ki);

	kailleraSelectServerDialog(NULL);

	free(gameList);

	kailleraShutdown(); // kaillera close
	FreeLibrary_KailleraClient_DLL();

	POST_INITIALISE_MESSAGE;
}

void FBA_KailleraEnd()
{
	if (kNetGame) {
		kNetGame = 0;
		kailleraEndGame();
		DeActivateChat();
	}
}

void FBA_KailleraSend()
{
	if (hwndChat) {
		int i = 0;
		while (EditText[i]) {
			if (EditText[i++] != 0x20) {
				break;
			}
		}
		if (i) {
			kailleraChatSend(WtoA(EditText));
		}
		DeActivateChat();
	}
}
