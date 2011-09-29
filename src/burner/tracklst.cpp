// sound track list, by regret

/* changelog:
 update 1: create (ref: caname)
*/

#include "burner.h"
#include "tracklst.h"


#if defined (_XBOX)
#define MAX_PLAYLIST_ITEM 128
#define MAX_PLAYLIST_SECT 32

int nTrackCnt, nSectCnt;
int nCurrentTrack, nCurrentSect;
bool bNoSect;

static bool bNoTrack;

struct playlist_t
{
	UINT16 code;
	char desc[80];
	char notes[80];
};
static playlist_t playlist[MAX_PLAYLIST_SECT][MAX_PLAYLIST_ITEM];


// send sound code to system
void sendSoundCode(UINT16 nCode)
{
	int nHardware = (BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK);

	switch (nHardware) {
		case HARDWARE_SNK_NEOGEO:
			NeoZ80Cmd(nCode);
			break;

		case HARDWARE_CAPCOM_CPS1:
		case HARDWARE_CAPCOM_CPS1_GENERIC:
		case HARDWARE_CAPCOM_CPSCHANGER:
			CpsSoundCmd(nCode);
			break;

		case HARDWARE_CAPCOM_CPS2:
		case HARDWARE_CAPCOM_CPS1_QSOUND:
			QSoundCMD(nCode);
			break;

		default:
			break;
	}
}

// get track/section info from sound.dat
int getTrackCode(int nSect, int nTrack)
{
	return playlist[nSect][nTrack].code;
}

TCHAR* getTrackDesc(int nSect, int nTrack, bool bIsSect)
{
	char TrackTitle[180] = "";
	char Notes[81] = "";

	if (playlist[nSect][nTrack].desc[0])
		strcpy(TrackTitle, playlist[nSect][nTrack].desc);

	if (!bIsSect)
	{
		if (playlist[nSect][nTrack].notes[0])
		{
			sprintf(Notes, " - %s", playlist[nSect][nTrack].notes);
			strcat(TrackTitle, Notes);
		}
	}

	return (TCHAR *)U8toW(TrackTitle);
}

static TCHAR* getTrackInfo(bool bIsSect)
{
	static TCHAR TrackTitle[180] = _T("");
	TCHAR* pTrackDesc = NULL;

	if (bIsSect)
	{
		pTrackDesc = getTrackDesc(0, nCurrentSect - 1, bIsSect);
		sprintf(TrackTitle, _T("Sect%d: %s"), nCurrentSect, pTrackDesc);
	}
	else
	{
		pTrackDesc = getTrackDesc(nCurrentSect, nCurrentTrack, bIsSect);
		sprintf(TrackTitle, _T("%.4x: %s"), playlist[nCurrentSect][nCurrentTrack].code, pTrackDesc);
	}

	return TrackTitle;
}

static void displayTrackInfo()
{
	VidSNewTinyMsg(getTrackInfo(false), 0x90C0E0);
}

static void displaySectInfo()
{
	VidSNewTinyMsg(getTrackInfo(true), 0x90C0E0);
}

// switch track
static void sendPlaylistCode()
{
	sendSoundCode(playlist[nCurrentSect][nCurrentTrack].code);
}

void playNextTrack()
{
	if (bNoTrack || !bDrvOkay)
		return;

	nCurrentTrack++;
	if (nCurrentTrack >= nTrackCnt)
		nCurrentTrack = 0;

	sendPlaylistCode();
	displayTrackInfo();
}

void playPreviousTrack()
{
	if (bNoTrack || !bDrvOkay)
		return;

	nCurrentTrack--;
	if (nCurrentTrack < 0)
		nCurrentTrack = nTrackCnt - 1;

	sendPlaylistCode();
	displayTrackInfo();
}

void playCurrentTrack()
{
	if (bNoTrack || !bDrvOkay)
		return;

	if (nCurrentTrack < 0)
		nCurrentTrack = nTrackCnt - 1;
	else if (nCurrentTrack >= nTrackCnt)
		nCurrentTrack = 0;

	sendPlaylistCode();
	displayTrackInfo();
}

// switch section if playlist has
void selectNextSect()
{
	if (bNoTrack || bNoSect)
		return;

	nCurrentSect++;

	if (nCurrentSect > nSectCnt)
		nCurrentSect = 1;

	nCurrentTrack = 0;
	nTrackCnt = playlist[0][nCurrentSect].code;

	displaySectInfo();
}

void selectPreviousSect()
{
	if (bNoTrack || bNoSect || !bDrvOkay) return;

	nCurrentSect--;
	if (nCurrentSect < 1) {
		nCurrentSect = nSectCnt;
	}
	nCurrentTrack = 0;
	nTrackCnt = playlist[0][nCurrentSect].code;

	displaySectInfo();
}

void setCurrentSect()
{
	if (bNoTrack || bNoSect || !bDrvOkay) return;

	if (nCurrentSect > nSectCnt) {
		nCurrentSect = 1;
	}
	else if (nCurrentSect < 1) {
		nCurrentSect = nSectCnt;
	}
	nCurrentTrack = 0;
	nTrackCnt = playlist[0][nCurrentSect].code;
}

// ==> parse sound.dat, ported from caname
static inline int atohex(char* s)
{
	int res = 0;

	while (*s) {
		res *= 0x10;
		if ((*s >= 'A' && *s <= 'F')
			|| (*s >= 'a' && *s <= 'f')) {
			res += (toupper(*s) - 'A') + 10;
		} else {
			res += *s - '0';
		}
		s++;
	}

	return res;
}

static int parseSoundData()
{
	FILE* fp = NULL;
	char name[MAX_PATH];
	char setname[MAX_PATH];
	char tmpbuf[MAX_PATH], tmpnum[5] = { 0, 0, 0, 0, 0 };
	UINT16 tmpcode;
	size_t i, j;
	size_t name_length;
	int item, playlist_format;

	memset(&playlist, 0, sizeof(playlist_t) * MAX_PLAYLIST_SECT * MAX_PLAYLIST_ITEM);

	sprintf(name, "config\\sound.dat");
	strcpy(setname, BurnDrvGetTextA(DRV_NAME));

loading:
	name_length = strlen(setname);

	fp = fopen(name, "r");
	if (fp == NULL)
		return 0; // cannot open file

	playlist_format = 2;
	playlist[0][0].code = 0;
	item = 0;

	/* Ex.: kof98:0721:Opening */
	/* Ex.: kof98:sect:BGM */
	while (1) {
		if (!fgets(tmpbuf, sizearray(tmpbuf), fp)) {
			break;
		}

		if (tmpbuf[0] == ';' || (tmpbuf[0] == '/' && tmpbuf[1] == '/'))
			continue; // skip comment

		if (tmpbuf[name_length] != ':') continue;

		if (strncmp(tmpbuf, setname, name_length) != 0) {
			if (playlist_format == 2) {
				continue;
			} else {
				if (playlist_format)
					playlist[0][0].code = item;
				break;
			}
		}

		for (i = 0 , j = name_length + 1; i <= 3; i++, j++) {
			tmpnum[i] = tmpbuf[j];
		}

		if (playlist_format == 2) {
			if (strncmp(tmpnum, "secs", 3) == 0) {
				bNoSect = false;
				playlist_format = 0;
			} else {
				playlist_format = 1;
				nSectCnt = 1;
			}
		}

		if (strncmp(tmpnum, "sect", 3) == 0) {
			if (playlist_format)
				continue;

			if (nSectCnt > 0)
				playlist[0][nSectCnt].code = item;

			item = 0;

			if (tmpnum[3] == 'e') {
				playlist[0][0].code = nSectCnt;
				break;
			}

			i = 0;
			j = name_length + 6;

			while (tmpbuf[j] != '\n') {
				playlist[0][nSectCnt].desc[i] = tmpbuf[j];
				i++;
				j++;
			}

//			playlist[0][item].desc[++i] = '\n';
			nSectCnt++;
			playlist[0][0].code = nSectCnt;

			if (nSectCnt >= MAX_PLAYLIST_SECT)
				break;

			continue;
		}

		// clone set
		if (strncmp(tmpnum, "name", 4) == 0) {
			i = 0;
			j = name_length + 6;
			while (tmpbuf[j] != '\n') {
				setname[i] = tmpbuf[j];
				i++;
				j++;
			}
			setname[i] = '\0';
			fclose(fp);
			goto loading;
		}

		tmpcode = atohex(tmpnum);

		if (tmpcode == 0 && item) {
			if (playlist_format) {
				playlist[0][0].code = item;
				break;
			} else {
				continue;
			}
		}

		if (item >= MAX_PLAYLIST_ITEM) {
			if (playlist_format) {
				playlist[0][0].code = item;
				break;
			} else {
				continue;
			}
		}

		// notes code
		if (tmpcode == 0xffff) {
			i = 0;
			j = name_length + 6;
			while (tmpbuf[j] != '\n') {
				playlist[nSectCnt][item - 1].notes[i] = tmpbuf[j];
				i++;
				j++;
			}
		} else {
			playlist[nSectCnt][item].code = tmpcode;

			i = 0;
			j = name_length + 6;
			while (tmpbuf[j] != '\n') {
				playlist[nSectCnt][item].desc[i] = tmpbuf[j];
				i++;
				j++;
			}
//			playlist[nSectCnt][item].desc[++i] = '\n';
			item++;
		}
	}

	fclose(fp);

	return playlist[0][0].code;
}
// <== parse sound.dat

int parseTracklist()
{
	resetTracklist();

	if (parseSoundData() == 0) {
		return 1;
	}

	nCurrentSect = 1;
	if (bNoSect) {
		nTrackCnt = playlist[0][0].code;
	} else {
		nTrackCnt = playlist[0][nCurrentSect].code;
	}

	bNoTrack = false;
	return 0; // success
}

void resetTracklist()
{
	nTrackCnt = nSectCnt = 0;
	nCurrentTrack = nCurrentSect = 0;
	bNoSect = true;
	bNoTrack = true;
}

#endif
