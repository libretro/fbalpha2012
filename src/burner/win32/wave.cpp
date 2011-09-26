// Wave log module
#include "burner.h"
#include "wave_writer.h"

bool soundLogStart = false;

static void WaveMakeOfn(TCHAR* pszFilter)
{
	_stprintf(pszFilter, FBALoadStringEx(IDS_DISK_FILE_SOUND), _T(APP_TITLE));
	memcpy(pszFilter + _tcslen(pszFilter), _T(" (*.wav)\0*.wav\0\0"), 16 * sizeof(TCHAR));

	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hScrnWnd;
	ofn.lpstrFilter = pszFilter;
	ofn.lpstrFile = szChoice;
	ofn.nMaxFile = sizearray(szChoice);
	ofn.lpstrInitialDir = _T(".\\wav");
	ofn.Flags = OFN_NOCHANGEDIR | OFN_HIDEREADONLY;
	ofn.lpstrDefExt = _T("wav");
	return;
}

int waveLogStart()
{
	TCHAR szFilter[1024];
	int nRet;
	int bOldPause;

	waveLogStop(); // make sure old log is closed

	WaveMakeOfn(szFilter);
	bOldPause = bRunPause;
	bRunPause = 1;
	nRet = GetSaveFileName(&ofn);
	bRunPause = bOldPause;
	if (nRet == 0) {
		return 1;
	}

	{
		if (!wave_open(nAudSampleRate, WtoA(szChoice)))
		{
			FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_ERR_DISK_CREATE));
			FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_DISK_SOUND));
			FBAPopupDisplay(PUF_TYPE_ERROR);
			return 1;
		}
		wave_enable_stereo();
		soundLogStart = true;
	}
	menuSync(MENUT_FILE);
	return 0;
}

int waveLogStop()
{
	if (soundLogStart)
	{
		wave_close();
		soundLogStart = false;
		menuSync(MENUT_FILE);
	}
	return 0;
}

void waveLogWrite()
{
	// log to the file
	if (soundLogStart && pBurnSoundOut != NULL) {
		wave_write(pBurnSoundOut, nBurnSoundLen << 1);
		pBurnSoundOut = NULL;
	}
}
