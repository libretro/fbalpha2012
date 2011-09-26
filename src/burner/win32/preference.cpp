// Preference module, added by regret

/* changelog:
 update 2: add property sheet dialog
 update 1: create
*/

#include "burner.h"
#include "preference.h"

// ----------------------------------------------------------------------------
// Video config
static HWND hVideoDlg = NULL;

static int dispApapterInit()
{
	if (!hVideoDlg) {
		return 1;
	}

	unsigned int count = 0;
	int ret = 0;
	DISPLAY_DEVICE dev;
	memset(&dev, 0, sizeof(dev));
	dev.cb = sizeof(dev);

	do {
		ret = EnumDisplayDevices(NULL, count, &dev, 0);
		if (ret) {
			if (dev.StateFlags & DISPLAY_DEVICE_ACTIVE) {
				SendDlgItemMessage(hVideoDlg, IDC_PREF_DISPADAPTER, CB_ADDSTRING, 0, (LPARAM)dev.DeviceName);
			}
			count++;
		}
	} while (ret);

	SendDlgItemMessage(hVideoDlg, IDC_PREF_DISPADAPTER, CB_SETCURSEL, (WPARAM)nVidAdapter, 0);

	return 0;
}

static int videoModeInit()
{
	if (!hVideoDlg) {
		return 1;
	}

	for (int i = 0; i < VID_LEN; i++) {
		SendDlgItemMessage(hVideoDlg, IDC_PREF_VIDEOMODE, CB_ADDSTRING, 0, (LPARAM)VidDriverName(i));
	}
	SendDlgItemMessage(hVideoDlg, IDC_PREF_VIDEOMODE, CB_SETCURSEL, (WPARAM)nVidSelect, 0);

	return 0;
}

static int fullResInit()
{
	if (!hVideoDlg) {
		return 1;
	}

	DEVMODE devMode;
	devMode.dmSize = sizeof(DEVMODE);
	devMode.dmDriverExtra = 0;

	DISPLAY_DEVICE device;
	device.cb = sizeof(DISPLAY_DEVICE);
	EnumDisplayDevices(NULL, nVidAdapter, &device, 0);

	// fullscreen res box
	// Run through all possible screenmodes and fill the combobox
	for (int i = 0; EnumDisplaySettings(device.DeviceName, i, &devMode); i++) {
		// We're only interested in 16, 24, or 32 bit modes.
		if (devMode.dmBitsPerPel & 0x30) {
			long item = 0, itemValue;
			long newRes = (devMode.dmPelsWidth << 16) | devMode.dmPelsHeight;

			// See if the resolution is already in the combobox
			do {
				itemValue = SendDlgItemMessage(hVideoDlg, IDC_PREF_FULLSCREENRES, CB_GETITEMDATA, item, 0);
				item++;
			} while ((itemValue != CB_ERR) && (itemValue != newRes));

			// If not, add it
			if (itemValue == CB_ERR) {
				TCHAR szRes[MAX_PATH];
				_stprintf(szRes, _T("%li x %li"), devMode.dmPelsWidth, devMode.dmPelsHeight);
				item = SendDlgItemMessage(hVideoDlg, IDC_PREF_FULLSCREENRES, CB_ADDSTRING, 0, (LPARAM)&szRes);
				SendDlgItemMessage(hVideoDlg, IDC_PREF_FULLSCREENRES, CB_SETITEMDATA, item, newRes);
			}
		}
	}

	// set select
	long myRes = (nVidWidth << 16) | nVidHeight;
	long count = SendDlgItemMessage(hVideoDlg, IDC_PREF_FULLSCREENRES, CB_GETCOUNT, 0, 0);
	long item = 0;
	for (long i = 0; i < count; i++) {
		item = SendDlgItemMessage(hVideoDlg, IDC_PREF_FULLSCREENRES, CB_GETITEMDATA, i, 0);
		if (myRes == item) {
			SendDlgItemMessage(hVideoDlg, IDC_PREF_FULLSCREENRES, CB_SETCURSEL, (WPARAM)i, 0);
			break;
		}
	}

	// depth
	SendDlgItemMessage(hVideoDlg, IDC_PREF_16BIT, BM_SETCHECK, (nVidDepth == 16) ? BST_CHECKED : BST_UNCHECKED, 0);
	SendDlgItemMessage(hVideoDlg, IDC_PREF_32BIT, BM_SETCHECK, (nVidDepth == 32) ? BST_CHECKED : BST_UNCHECKED, 0);

	return 0;
}

static void enableVideoOptions(int mode)
{
	if (mode < 0 || mode >= VID_LEN) {
		return;
	}

	switch (mode) {
		case VID_D3D7:
			EnableWindow(GetDlgItem(hVideoDlg, IDC_PREF_3DPROJECTION), TRUE);
			EnableWindow(GetDlgItem(hVideoDlg, IDC_PREF_TEXTUREMANAGE), TRUE);
			EnableWindow(GetDlgItem(hVideoDlg, IDC_PREF_MOTIONBLUR), FALSE);
			EnableWindow(GetDlgItem(hVideoDlg, IDC_PREF_HARDWAREVERTEX), FALSE);
			break;

		case VID_D3D:
			EnableWindow(GetDlgItem(hVideoDlg, IDC_PREF_3DPROJECTION), FALSE);
			EnableWindow(GetDlgItem(hVideoDlg, IDC_PREF_TEXTUREMANAGE), FALSE);
			EnableWindow(GetDlgItem(hVideoDlg, IDC_PREF_MOTIONBLUR), TRUE);
			EnableWindow(GetDlgItem(hVideoDlg, IDC_PREF_HARDWAREVERTEX), TRUE);
			break;

		default:
			EnableWindow(GetDlgItem(hVideoDlg, IDC_PREF_3DPROJECTION), FALSE);
			EnableWindow(GetDlgItem(hVideoDlg, IDC_PREF_TEXTUREMANAGE), FALSE);
			EnableWindow(GetDlgItem(hVideoDlg, IDC_PREF_MOTIONBLUR), FALSE);
			EnableWindow(GetDlgItem(hVideoDlg, IDC_PREF_HARDWAREVERTEX), FALSE);
			break;
	}
}

static void videoOptionInit()
{
	if (!hVideoDlg) {
		return;
	}

	CheckDlgButton(hVideoDlg, IDC_PREF_MOTIONBLUR, vidMotionBlur ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hVideoDlg, IDC_PREF_TEXTUREMANAGE, nVidDXTextureManager ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hVideoDlg, IDC_PREF_FORCE_16BIT, bVidForce16bit ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hVideoDlg, IDC_PREF_HARDWAREVERTEX, vidHardwareVertex ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hVideoDlg, IDC_PREF_3DPROJECTION, (nVid3DProjection & 1) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hVideoDlg, IDC_PREF_COLOR_ADJUST, bcolorAdjust ? BST_CHECKED : BST_UNCHECKED);

	CheckDlgButton(hVideoDlg, IDC_PREF_TRIPLE, bVidTripleBuffer ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hVideoDlg, IDC_PREF_VSYNC, bVidVSync ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hVideoDlg, IDC_PREF_FORCE60HZ, bForce60Hz ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hVideoDlg, IDC_PREF_AUTOFRAMESKIP, autoFrameSkip ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hVideoDlg, IDC_PREF_BLITTERPREVIEW, bVidUsePlaceholder ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hVideoDlg, IDC_PREF_AUTOMONITORASPECT, autoVidScrnAspect ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hVideoDlg, IDC_PREF_MONITORMIRRORVERT, (nVidRotationAdjust & 2) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hVideoDlg, IDC_PREF_ROTATEVERTICAL, !(nVidRotationAdjust & 1) ? BST_CHECKED : BST_UNCHECKED);

	EnableWindow(GetDlgItem(hVideoDlg, IDC_PREF_SETPROJECTION), (nVid3DProjection & 1));
	EnableWindow(GetDlgItem(hVideoDlg, IDC_PREF_SETCOLOR), bcolorAdjust);
	EnableWindow(GetDlgItem(hVideoDlg, IDC_PREF_CUSTOMMONITORASPECT), !autoVidScrnAspect);

	enableVideoOptions(nVidSelect);
}

static void prefVideoExit()
{
	if (!hVideoDlg) {
		return;
	}

	int mode = SendDlgItemMessage(hVideoDlg, IDC_PREF_VIDEOMODE, CB_GETCURSEL, 0, 0);
	VidSelect(mode);

	nVidAdapter = SendDlgItemMessage(hVideoDlg, IDC_PREF_DISPADAPTER, CB_GETCURSEL, 0, 0);

	// fullscreen resolution
	long resItem = SendDlgItemMessage(hVideoDlg, IDC_PREF_FULLSCREENRES, CB_GETCURSEL, 0, 0);
	long itemValue = SendDlgItemMessage(hVideoDlg, IDC_PREF_FULLSCREENRES, CB_GETITEMDATA, resItem, 0);
	nVidWidth = itemValue >> 16;
	nVidHeight = itemValue & 0xFFFF;

	if (BST_CHECKED == IsDlgButtonChecked(hVideoDlg, IDC_PREF_16BIT)) {
		nVidDepth = 16;
	} else if (BST_CHECKED == IsDlgButtonChecked(hVideoDlg, IDC_PREF_32BIT)) {
		nVidDepth = 32;
	}

	vidMotionBlur = (BST_CHECKED == IsDlgButtonChecked(hVideoDlg, IDC_PREF_MOTIONBLUR)) ? 1 : 0;
	nVidDXTextureManager = (BST_CHECKED == IsDlgButtonChecked(hVideoDlg, IDC_PREF_TEXTUREMANAGE)) ? 1 : 0;
	bVidForce16bit = (BST_CHECKED == IsDlgButtonChecked(hVideoDlg, IDC_PREF_FORCE_16BIT)) ? 1 : 0;
	vidHardwareVertex = (BST_CHECKED == IsDlgButtonChecked(hVideoDlg, IDC_PREF_HARDWAREVERTEX)) ? 1 : 0;
	nVid3DProjection = (BST_CHECKED == IsDlgButtonChecked(hVideoDlg, IDC_PREF_3DPROJECTION)) ? 1 : 0;

	bcolorAdjust = (BST_CHECKED == IsDlgButtonChecked(hVideoDlg, IDC_PREF_COLOR_ADJUST)) ? 1 : 0;
	bVidTripleBuffer = (BST_CHECKED == IsDlgButtonChecked(hVideoDlg, IDC_PREF_TRIPLE)) ? 1 : 0;
	bVidVSync = (BST_CHECKED == IsDlgButtonChecked(hVideoDlg, IDC_PREF_VSYNC)) ? 1 : 0;
	bForce60Hz = (BST_CHECKED == IsDlgButtonChecked(hVideoDlg, IDC_PREF_FORCE60HZ)) ? 1 : 0;
	autoFrameSkip = (BST_CHECKED == IsDlgButtonChecked(hVideoDlg, IDC_PREF_AUTOFRAMESKIP)) ? 1 : 0;
	bVidUsePlaceholder = (BST_CHECKED == IsDlgButtonChecked(hVideoDlg, IDC_PREF_BLITTERPREVIEW)) ? 1 : 0;
	autoVidScrnAspect = (BST_CHECKED == IsDlgButtonChecked(hVideoDlg, IDC_PREF_AUTOMONITORASPECT)) ? 1 : 0;

	if (nVidRotationAdjust & 1) {
		if (BST_CHECKED == IsDlgButtonChecked(hVideoDlg, IDC_PREF_ROTATEVERTICAL)) {
			nVidRotationAdjust ^= 1;
		}
	} else {
		if (BST_CHECKED != IsDlgButtonChecked(hVideoDlg, IDC_PREF_ROTATEVERTICAL)) {
			nVidRotationAdjust ^= 1;
		}
	}

	if (!(nVidRotationAdjust & 2)) {
		nVidRotationAdjust |= (BST_CHECKED == IsDlgButtonChecked(hVideoDlg, IDC_PREF_MONITORMIRRORVERT)) ? 2 : 0;
	} else {
		if (BST_CHECKED != IsDlgButtonChecked(hVideoDlg, IDC_PREF_MONITORMIRRORVERT)) {
			nVidRotationAdjust ^= 2;
		}
	}
}

static INT_PTR CALLBACK prefVideoDlgProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch (Msg) {
		case WM_INITDIALOG:
			wndInMid(hDlg, hScrnWnd);
			SetFocus(hDlg);
			hVideoDlg = hDlg;

			// init controls
			videoModeInit();
			dispApapterInit();
			fullResInit();
			videoOptionInit();

			return TRUE;

		case WM_COMMAND: {
			int Id = LOWORD(wParam);
			int Notify = HIWORD(wParam);

			if ((Id == IDOK && Notify == BN_CLICKED) || (Id == IDCANCEL && Notify == BN_CLICKED)) {
				SendMessage(hDlg, WM_CLOSE, 0, 0);
				return 0;
			}
			else if (Id == IDC_PREF_SETPROJECTION && Notify == BN_CLICKED) {
				screenAngleDialog(hDlg);
				return 0;
			}
			else if (Id == IDC_PREF_SETCOLOR && Notify == BN_CLICKED) {
				colorAdjustDialog(hDlg);
				return 0;
			}
			else if (Id == IDC_PREF_CUSTOMMONITORASPECT && Notify == BN_CLICKED) {
				aspectSetDialog(hDlg);
				return 0;
			}
			else if (Id == IDC_PREF_AUTOMONITORASPECT && Notify == BN_CLICKED) {
				BOOL checked = (BST_CHECKED == IsDlgButtonChecked(hDlg, Id));
				EnableWindow(GetDlgItem(hDlg, IDC_PREF_CUSTOMMONITORASPECT), !checked);
				return 0;
			}
			else if (Id == IDC_PREF_3DPROJECTION && Notify == BN_CLICKED) {
				BOOL checked = (BST_CHECKED == IsDlgButtonChecked(hDlg, Id));
				EnableWindow(GetDlgItem(hDlg, IDC_PREF_SETPROJECTION), checked);
				return 0;
			}
			else if (Id == IDC_PREF_COLOR_ADJUST && Notify == BN_CLICKED) {
				BOOL checked = (BST_CHECKED == IsDlgButtonChecked(hDlg, Id));
				EnableWindow(GetDlgItem(hDlg, IDC_PREF_SETCOLOR), checked);
				return 0;
			}

			if (Notify == CBN_SELCHANGE) {
				if ((HWND)lParam == GetDlgItem(hDlg, IDC_PREF_VIDEOMODE)) {
					int mode = SendDlgItemMessage(hDlg, IDC_PREF_VIDEOMODE, CB_GETCURSEL, 0, 0);
					enableVideoOptions(mode);
				}
				else if ((HWND)lParam == GetDlgItem(hDlg, IDC_PREF_DISPADAPTER)) {
					// reinit fullscreen resolution if adapter changed
					nVidAdapter = SendDlgItemMessage(hDlg, IDC_PREF_DISPADAPTER, CB_GETCURSEL, 0, 0);
					fullResInit();
				}
			}
		}
		break;

		case WM_NOTIFY: {
			switch (((NMHDR *)lParam)->code) {
				case PSN_APPLY:
					prefVideoExit();
					break;
			}
		}
		break;

		case WM_CLOSE:
			prefVideoExit();

			EndDialog(hDlg, 0);
			hVideoDlg = NULL;
			break;
	}

	return 0;
}

void prefVideoCreate(HWND parent)
{
	FBADialogBox(IDD_PREF_VIDEO, parent, (DLGPROC)prefVideoDlgProc);
}

// ----------------------------------------------------------------------------
// Audio config

static HWND hAudioDlg = NULL;

static int audioOutputInit()
{
	if (!hAudioDlg) {
		return 1;
	}

	const TCHAR* list = audio.driver_list();

	// audio output string list
	TCHAR szToken[MAX_PATH];
	_tcscpy(szToken, list);
	TCHAR* token = _tcstok(szToken, _T(";"));
	int index = 0;
	while (token) {
		SendDlgItemMessage(hAudioDlg, IDC_PREF_AUDIOOUTPUT, CB_ADDSTRING, 0, (LPARAM)token);
		// set selected audio output
		if (!_tcscmp(audSelect, token)) {
			SendDlgItemMessage(hAudioDlg, IDC_PREF_AUDIOOUTPUT, CB_SETCURSEL, (WPARAM)index, 0);
		}
		token = _tcstok(NULL, _T(";"));
		index++;
	}
	free(token);

	return 0;
}

static int audioDeviceInit()
{
	InterfaceInfo* info = audio.get();
	if (!info || !hAudioDlg) {
		return 1;
	}

	SendDlgItemMessage(hAudioDlg, IDC_PREF_AUDIODEVICE, CB_RESETCONTENT, 0, 0);
	for (unsigned int i = 0; i < info->deviceNum; i++) {
		SendDlgItemMessage(hAudioDlg, IDC_PREF_AUDIODEVICE, CB_ADDSTRING, 0, (LPARAM)info->deviceName[i]);
	}
	SendDlgItemMessage(hAudioDlg, IDC_PREF_AUDIODEVICE, CB_SETCURSEL, (WPARAM)audio.getdevice(), 0);

	return 0;
}

static void audioSampleInit()
{
	if (!hAudioDlg) {
		return;
	}

	SendDlgItemMessage(hAudioDlg, IDC_PREF_SAMPLERATE, CB_ADDSTRING, 0, (LPARAM)_T("11025Hz"));
	SendDlgItemMessage(hAudioDlg, IDC_PREF_SAMPLERATE, CB_ADDSTRING, 0, (LPARAM)_T("22050Hz"));
	SendDlgItemMessage(hAudioDlg, IDC_PREF_SAMPLERATE, CB_ADDSTRING, 0, (LPARAM)_T("44100Hz"));
	SendDlgItemMessage(hAudioDlg, IDC_PREF_SAMPLERATE, CB_ADDSTRING, 0, (LPARAM)_T("48000Hz"));

	int index = 0;
	if (nAudSampleRate > 0) {
		if (nAudSampleRate <= 11025) {
			index = 0;
		}
		else if (nAudSampleRate <= 22050) {
			index = 1;
		}
		else if (nAudSampleRate <= 44100) {
			index = 2;
		}
		else if (nAudSampleRate <= 48000) {
			index = 3;
		}
		else {
			index = 3;
		}
	}
	SendDlgItemMessage(hAudioDlg, IDC_PREF_SAMPLERATE, CB_SETCURSEL, (WPARAM)index, 0);

	if (bDrvOkay) {
		EnableWindow(GetDlgItem(hAudioDlg, IDC_PREF_SAMPLERATE), FALSE);
	}
}

static void audioOptionInit()
{
	if (!hAudioDlg) {
		return;
	}

	CheckDlgButton(hAudioDlg, IDC_PREF_INTERPOLATE_1, (nInterpolation == 1) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hAudioDlg, IDC_PREF_INTERPOLATE_3, (nInterpolation == 3) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hAudioDlg, IDC_PREF_INTERPOLATE_FM_3, (nFMInterpolation == 3) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hAudioDlg, IDC_PREF_LOWPASS, (nAudDSPModule & 1) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hAudioDlg, IDC_PREF_STEREOUPMIX, audStereoUpmixing ? BST_CHECKED : BST_UNCHECKED);

	EnableWindow(GetDlgItem(hAudioDlg, IDC_PREF_INTERPOLATE_1), !bDrvOkay);
	EnableWindow(GetDlgItem(hAudioDlg, IDC_PREF_INTERPOLATE_3), !bDrvOkay);
	EnableWindow(GetDlgItem(hAudioDlg, IDC_PREF_INTERPOLATE_FM_3), !bDrvOkay);

	int xaudio2 = _tcscmp(_T("XAudio2"), audSelect);
	EnableWindow(GetDlgItem(hAudioDlg, IDC_PREF_STEREOUPMIX), (xaudio2 == 0) ? TRUE : FALSE);
}

static void audioFrameInit()
{
	if (!hAudioDlg) {
		return;
	}

	// Initialise slider
	TCHAR szText[MAX_PATH];
	_sntprintf(szText, sizearray(szText), FBALoadStringEx(IDS_NUMDLG_FRAME), nAudSegCount, int(1000.0 * (nAudSegCount - 1.0) / 60));
	SetDlgItemText(hAudioDlg, IDC_PREF_AUDFRAME_TEXT, szText);

	// limit in 2 - 10
	SendDlgItemMessage(hAudioDlg, IDC_PREF_AUDFRAME, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(2, 10));
	SendDlgItemMessage(hAudioDlg, IDC_PREF_AUDFRAME, TBM_SETTIC, 0, (LPARAM)6);
	SendDlgItemMessage(hAudioDlg, IDC_PREF_AUDFRAME, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)nAudSegCount);
}

static void audioOutputChange()
{
	if (!hAudioDlg) {
		return;
	}

	// get selected audio output
	int sel = SendDlgItemMessage(hAudioDlg, IDC_PREF_AUDIOOUTPUT, CB_GETCURSEL, 0, 0);
	TCHAR output[MAX_PATH] = _T("");
	SendDlgItemMessage(hAudioDlg, IDC_PREF_AUDIOOUTPUT, CB_GETLBTEXT, (WPARAM)sel, (LPARAM)output);

	// reinit audio
	audio.select(output);
	mediaReInitAudio();

	// reinit device if output changed
	audioDeviceInit();

	int xaudio2 = _tcscmp(_T("XAudio2"), output);
	EnableWindow(GetDlgItem(hAudioDlg, IDC_PREF_STEREOUPMIX), (xaudio2 == 0) ? TRUE : FALSE);
}

static void prefAudioExit()
{
	if (!hAudioDlg) {
		return;
	}

	int index = SendDlgItemMessage(hAudioDlg, IDC_PREF_AUDIODEVICE, CB_GETCURSEL, 0, 0);
	if (audio.getdevice() != index) {
		audio.setdevice(index);
	}

	index = SendDlgItemMessage(hAudioDlg, IDC_PREF_SAMPLERATE, CB_GETCURSEL, 0, 0);
	if (index == 0) {
		nAudSampleRate = 11025;
	}
	else if (index == 1) {
		nAudSampleRate = 22050;
	}
	else if (index == 2) {
		nAudSampleRate = 44100;
	}
	else {
		nAudSampleRate = 48000;
	}

	nInterpolation = (BST_CHECKED == IsDlgButtonChecked(hAudioDlg, IDC_PREF_INTERPOLATE_1)) ? 1 : 3;
	nFMInterpolation = (BST_CHECKED == IsDlgButtonChecked(hAudioDlg, IDC_PREF_INTERPOLATE_FM_3)) ? 3 : 0;
	nAudDSPModule = (BST_CHECKED == IsDlgButtonChecked(hAudioDlg, IDC_PREF_LOWPASS)) ? 1 : 0;
	audStereoUpmixing = (BST_CHECKED == IsDlgButtonChecked(hAudioDlg, IDC_PREF_STEREOUPMIX)) ? 1 : 0;

	nAudSegCount = SendDlgItemMessage(hAudioDlg, IDC_PREF_AUDFRAME, TBM_GETPOS, 0, 0);
}

static INT_PTR CALLBACK prefAudioDlgProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch (Msg) {
		case WM_INITDIALOG:
			wndInMid(hDlg, hScrnWnd);
			SetFocus(hDlg);
			hAudioDlg = hDlg;

			// init controls
			audioOutputInit();
			audioDeviceInit();
			audioSampleInit();
			audioOptionInit();
			audioFrameInit();

			return TRUE;

		case WM_COMMAND: {
			int Id = LOWORD(wParam);
			int Notify = HIWORD(wParam);

			if ((Id == IDOK && Notify == BN_CLICKED) || (Id == IDCANCEL && Notify == BN_CLICKED)) {
				SendMessage(hDlg, WM_CLOSE, 0, 0);
				return 0;
			}

			if ((HWND)lParam == GetDlgItem(hDlg, IDC_PREF_AUDIOOUTPUT)) {
				if (Notify == CBN_SELCHANGE) {
					audioOutputChange();
				}
			}
		}
		break;

		case WM_HSCROLL:
			switch (LOWORD(wParam)) {
				case TB_BOTTOM:
				case TB_ENDTRACK:
				case TB_LINEDOWN:
				case TB_LINEUP:
				case TB_PAGEDOWN:
				case TB_PAGEUP:
				case TB_THUMBPOSITION:
				case TB_THUMBTRACK:
				case TB_TOP: {
					TCHAR szText[MAX_PATH];
					int frame = SendDlgItemMessage(hDlg, IDC_PREF_AUDFRAME, TBM_GETPOS, 0, 0);
					_sntprintf(szText, sizearray(szText), FBALoadStringEx(IDS_NUMDLG_FRAME), frame, int(1000.0 * (frame - 1.0) / 60));
					SetDlgItemText(hDlg, IDC_PREF_AUDFRAME_TEXT, szText);
					break;
				}
			}
			break;

		case WM_NOTIFY: {
			switch (((NMHDR *)lParam)->code) {
				case PSN_APPLY:
					prefAudioExit();
					break;
			}
		}
		break;

		case WM_CLOSE:
			prefAudioExit();

			EndDialog(hDlg, 0);
			hAudioDlg = NULL;
			break;
	}

	return 0;
}

void prefAudioCreate(HWND parent)
{
	FBADialogBox(IDD_PREF_AUDIO, parent, (DLGPROC)prefAudioDlgProc);
}

// ----------------------------------------------------------------------------
// Misc config

static HWND hMiscDlg = NULL;

static void miscOptionInit()
{
	if (!hMiscDlg) {
		return;
	}

	CheckDlgButton(hMiscDlg, IDC_PREF_ASM68K, bBurnUseASM68K ? BST_CHECKED : BST_UNCHECKED);
#ifndef NO_COMBO
	CheckDlgButton(hMiscDlg, IDC_PREF_INPUTMACRO, nInputMacroEnabled ? BST_CHECKED : BST_UNCHECKED);
#endif
#ifndef NO_AUTOFIRE
	CheckDlgButton(hMiscDlg, IDC_PREF_AUTOFIRE, nAutofireEnabled ? BST_CHECKED : BST_UNCHECKED);
#endif

	CheckDlgButton(hMiscDlg, IDC_PREF_ONTOP, bShowOnTop ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hMiscDlg, IDC_PREF_AUTOPAUSE, bAutoPause ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hMiscDlg, IDC_PREF_PROCESSKEY, bAlwaysProcessKey ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hMiscDlg, IDC_PREF_AUTOFULLSCREEN, bFullscreenOnStart ? BST_CHECKED : BST_UNCHECKED);

	CheckDlgButton(hMiscDlg, IDC_PREF_MODELESSMENU, bModelessMenu ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hMiscDlg, IDC_PREF_DISABLESPLASH, nDisableSplash ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hMiscDlg, IDC_PREF_RANDOMSKIN, nRandomSkin ? BST_CHECKED : BST_UNCHECKED);
#ifndef NO_GDI_PLUS
	CheckDlgButton(hMiscDlg, IDC_PREF_GDIPLUS, bUseGdip ? BST_CHECKED : BST_UNCHECKED);
#endif

	// enable controls
	EnableWindow(GetDlgItem(hMiscDlg, IDC_PREF_ASM68K), !bDrvOkay);
#ifndef NO_COMBO
	EnableWindow(GetDlgItem(hMiscDlg, IDC_PREF_INPUTMACRO), !bDrvOkay);
#endif
}

static void miscSliderInit()
{
	if (!hMiscDlg) {
		return;
	}

	// Initialise slider
	TCHAR szText[16];
	_sntprintf(szText, sizearray(szText), _T("%d"), nAppThreadPriority);
	SetDlgItemText(hMiscDlg, IDC_PREF_THREADPRIORITY_TEXT, szText);

	// thread priority, limit in -15 ~ 1
	SendDlgItemMessage(hMiscDlg, IDC_PREF_THREADPRIORITY, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(-15, 1));
	SendDlgItemMessage(hMiscDlg, IDC_PREF_THREADPRIORITY, TBM_SETTIC, 0, 0);
	SendDlgItemMessage(hMiscDlg, IDC_PREF_THREADPRIORITY, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)nAppThreadPriority);
}

static void miscComboboxInit()
{
	if (!hMiscDlg) {
		return;
	}

#ifndef NO_IMAGEMENU
	for (int i = IDS_PREF_MENUNONE; i <= IDS_PREF_MENUEXTRA; i++) {
		SendDlgItemMessage(hMiscDlg, IDC_PREF_MENUSTYLE, CB_ADDSTRING, 0, (LPARAM)FBALoadStringEx(i));
	}
	SendDlgItemMessage(hMiscDlg, IDC_PREF_MENUSTYLE, CB_SETCURSEL, (WPARAM)menuNewStyle, 0);
#endif

	for (int i = IDS_PREF_EFFECTNONE; i <= IDS_PREF_EFFECTTV; i++) {
		SendDlgItemMessage(hMiscDlg, IDC_PREF_EFFECTTYPE, CB_ADDSTRING, 0, (LPARAM)FBALoadStringEx(i));
	}
	SendDlgItemMessage(hMiscDlg, IDC_PREF_EFFECTTYPE, CB_SETCURSEL, (WPARAM)nShowEffect, 0);

	TCHAR speedstr[32] = _T("");
	for (int i = 2; i < 9; i++) {
		_stprintf(speedstr, _T("%d%%"), i * 100);
		SendDlgItemMessage(hMiscDlg, IDC_PREF_FASTFORWARD, CB_ADDSTRING, 0, (LPARAM)speedstr);
	}
	SendDlgItemMessage(hMiscDlg, IDC_PREF_FASTFORWARD, CB_SETCURSEL, (WPARAM)nFastSpeed - 1, 0);
}

static void prefMiscExit()
{
	if (!hMiscDlg) {
		return;
	}

	bBurnUseASM68K = (BST_CHECKED == IsDlgButtonChecked(hMiscDlg, IDC_PREF_ASM68K)) ? 1 : 0;
#ifndef NO_COMBO
	nInputMacroEnabled = (BST_CHECKED == IsDlgButtonChecked(hMiscDlg, IDC_PREF_INPUTMACRO)) ? 1 : 0;
#endif
#ifndef NO_AUTOFIRE
	nAutofireEnabled = (BST_CHECKED == IsDlgButtonChecked(hMiscDlg, IDC_PREF_AUTOFIRE)) ? 1 : 0;
#endif

	bShowOnTop = (BST_CHECKED == IsDlgButtonChecked(hMiscDlg, IDC_PREF_ONTOP)) ? 1 : 0;
	if (bDrvOkay && bShowOnTop) {
		SetWindowPos(hScrnWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW | SWP_NOMOVE);
	}
	else if (!bShowOnTop) {
		SetWindowPos(hScrnWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW | SWP_NOMOVE);
	}

	bAutoPause = (BST_CHECKED == IsDlgButtonChecked(hMiscDlg, IDC_PREF_AUTOPAUSE)) ? 1 : 0;
	bAlwaysProcessKey = (BST_CHECKED == IsDlgButtonChecked(hMiscDlg, IDC_PREF_PROCESSKEY)) ? 1 : 0;
	bFullscreenOnStart = (BST_CHECKED == IsDlgButtonChecked(hMiscDlg, IDC_PREF_AUTOFULLSCREEN)) ? 1 : 0;

	bModelessMenu = (BST_CHECKED == IsDlgButtonChecked(hMiscDlg, IDC_PREF_MODELESSMENU)) ? 1 : 0;
	nDisableSplash = (BST_CHECKED == IsDlgButtonChecked(hMiscDlg, IDC_PREF_DISABLESPLASH)) ? 1 : 0;
	nRandomSkin = (BST_CHECKED == IsDlgButtonChecked(hMiscDlg, IDC_PREF_RANDOMSKIN)) ? 1 : 0;
#ifndef NO_GDI_PLUS
	bUseGdip = (BST_CHECKED == IsDlgButtonChecked(hMiscDlg, IDC_PREF_GDIPLUS)) ? 1 : 0;
#endif

#ifndef NO_IMAGEMENU
	menuNewStyle = SendDlgItemMessage(hMiscDlg, IDC_PREF_MENUSTYLE, CB_GETCURSEL, 0, 0);
#endif
	nShowEffect = SendDlgItemMessage(hMiscDlg, IDC_PREF_EFFECTTYPE, CB_GETCURSEL, 0, 0);
	nFastSpeed = SendDlgItemMessage(hMiscDlg, IDC_PREF_FASTFORWARD, CB_GETCURSEL, 0, 0) + 1;

	// set thread priority
	int priority = SendDlgItemMessage(hMiscDlg, IDC_PREF_THREADPRIORITY, TBM_GETPOS, 0, 0);
	if (nAppThreadPriority != priority) {
		nAppThreadPriority = priority;
		SetThreadPriority(GetCurrentThread(), nAppThreadPriority);
	}
}

static INT_PTR CALLBACK prefMiscDlgProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch (Msg) {
		case WM_INITDIALOG:
			wndInMid(hDlg, hScrnWnd);
			SetFocus(hDlg);
			hMiscDlg = hDlg;

			// init controls
			miscOptionInit();
			miscSliderInit();
			miscComboboxInit();

			return TRUE;

		case WM_COMMAND: {
			int Id = LOWORD(wParam);
			int Notify = HIWORD(wParam);

			if ((Id == IDOK && Notify == BN_CLICKED) || (Id == IDCANCEL && Notify == BN_CLICKED)) {
				SendMessage(hDlg, WM_CLOSE, 0, 0);
				return 0;
			}

			if ((HWND)lParam == GetDlgItem(hDlg, IDC_PREF_AUDIOOUTPUT)) {
				if (Notify == CBN_SELCHANGE) {
				}
			}
		}
		break;

		case WM_HSCROLL:
			switch (LOWORD(wParam)) {
				case TB_BOTTOM:
				case TB_ENDTRACK:
				case TB_LINEDOWN:
				case TB_LINEUP:
				case TB_PAGEDOWN:
				case TB_PAGEUP:
				case TB_THUMBPOSITION:
				case TB_THUMBTRACK:
				case TB_TOP: {
					TCHAR szText[16];
					int priority = SendDlgItemMessage(hDlg, IDC_PREF_THREADPRIORITY, TBM_GETPOS, 0, 0);
					_sntprintf(szText, sizearray(szText), _T("%d"), priority);
					SetDlgItemText(hDlg, IDC_PREF_THREADPRIORITY_TEXT, szText);
					break;
				}
			}
			break;

		case WM_NOTIFY: {
			switch (((NMHDR *)lParam)->code) {
				case PSN_APPLY:
					prefMiscExit();
					break;
			}
		}
		break;

		case WM_CLOSE:
			prefMiscExit();

			EndDialog(hDlg, 0);
			hMiscDlg = NULL;
			break;
	}

	return 0;
}

void prefMiscCreate(HWND parent)
{
	FBADialogBox(IDD_PREF_MISC, parent, (DLGPROC)prefMiscDlgProc);
}

// Property sheet
void preferenceCreate(HWND parent)
{
	HINSTANCE instance = FBALocaliseInstance();
	PROPSHEETPAGE psp[3];
	PROPSHEETHEADER psh;

	for (int i = 0; i < sizearray(psp); i++) {
		psp[i].dwSize = sizeof(PROPSHEETPAGE);
		psp[i].dwFlags = PSP_DEFAULT;
		psp[i].hInstance = instance;
		psp[i].lParam = 0;
		psp[i].pfnCallback = NULL;
	}
	psp[0].pszTemplate = MAKEINTRESOURCE(IDD_PREF_VIDEO);
	psp[0].pfnDlgProc = (DLGPROC)prefVideoDlgProc;
	psp[1].pszTemplate = MAKEINTRESOURCE(IDD_PREF_AUDIO);
	psp[1].pfnDlgProc = (DLGPROC)prefAudioDlgProc;
	psp[2].pszTemplate = MAKEINTRESOURCE(IDD_PREF_MISC);
	psp[2].pfnDlgProc = (DLGPROC)prefMiscDlgProc;

	psh.dwSize = sizeof(PROPSHEETHEADER);
	psh.dwFlags = PSH_PROPSHEETPAGE | PSH_NOAPPLYNOW | PSH_NOCONTEXTHELP;
	psh.hwndParent = parent;
	psh.hInstance = instance;
	psh.pszCaption = FBALoadStringEx(IDS_PREF_SETTINGS);
	psh.nPages = sizearray(psp);
	psh.nStartPage = 0;
	psh.ppsp = (LPCPROPSHEETPAGE)&psp;
	psh.pfnCallback = NULL;
	PropertySheet(&psh);
}
