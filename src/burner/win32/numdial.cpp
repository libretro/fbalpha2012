// number dialogs
#include "burner.h"
#include <ctype.h>

static int nExitStatus = 0;

// ----------------------------------------------------------------------------
// CPU clock dialog

static INT_PTR CALLBACK CPUClockProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM)	// LPARAM lParam
{
	switch (Msg) {
		case WM_INITDIALOG: {
			TCHAR szText[16];
			nExitStatus = 0;

			wndInMid(hDlg, hScrnWnd);

			// Initialise slider
			SendDlgItemMessage(hDlg, IDC_CPUCLOCK_SLIDER, TBM_SETRANGE, 0, (LPARAM)MAKELONG(0x80, 0x0500));
			SendDlgItemMessage(hDlg, IDC_CPUCLOCK_SLIDER, TBM_SETLINESIZE, 0, (LPARAM)0x05);
			SendDlgItemMessage(hDlg, IDC_CPUCLOCK_SLIDER, TBM_SETPAGESIZE, 0, (LPARAM)0x10);
			SendDlgItemMessage(hDlg, IDC_CPUCLOCK_SLIDER, TBM_SETTIC, 0, (LPARAM)0x0100);
			SendDlgItemMessage(hDlg, IDC_CPUCLOCK_SLIDER, TBM_SETTIC, 0, (LPARAM)0x0120);
			SendDlgItemMessage(hDlg, IDC_CPUCLOCK_SLIDER, TBM_SETTIC, 0, (LPARAM)0x0140);
			SendDlgItemMessage(hDlg, IDC_CPUCLOCK_SLIDER, TBM_SETTIC, 0, (LPARAM)0x0180);
			// Extra values (FB Alpha Plus!)
			SendDlgItemMessage(hDlg, IDC_CPUCLOCK_SLIDER, TBM_SETTIC, 0, (LPARAM)0x0280);
			SendDlgItemMessage(hDlg, IDC_CPUCLOCK_SLIDER, TBM_SETTIC, 0, (LPARAM)0x0380);
			SendDlgItemMessage(hDlg, IDC_CPUCLOCK_SLIDER, TBM_SETTIC, 0, (LPARAM)0x0480);

			// Set slider to current value
			SendDlgItemMessage(hDlg, IDC_CPUCLOCK_SLIDER, TBM_SETPOS, (WPARAM)true, (LPARAM)nBurnCPUSpeedAdjust);

			// Set the edit control to current value
			_stprintf(szText, _T("%i"), nBurnCPUSpeedAdjust * 100 / 256);
			SendDlgItemMessage(hDlg, IDC_CPUCLOCK_EDIT, WM_SETTEXT, 0, (LPARAM)szText);

			return TRUE;
		}
		case WM_COMMAND: {
			switch (HIWORD(wParam)) {
				case BN_CLICKED: {
					if (LOWORD(wParam) == IDOK) {
						nExitStatus = 1;
						SendMessage(hDlg, WM_CLOSE, 0, 0);
					}
					if (LOWORD(wParam) == IDCANCEL) {
						nExitStatus = -1;
						SendMessage(hDlg, WM_CLOSE, 0, 0);
					}
					break;
				}
				case EN_UPDATE: {
					if (nExitStatus == 0) {
						TCHAR szText[16] = _T("");
						bool bValid = 1;
						int nValue;

						if (SendDlgItemMessage(hDlg, IDC_CPUCLOCK_EDIT, WM_GETTEXTLENGTH, 0, 0) < 16) {
							SendDlgItemMessage(hDlg, IDC_CPUCLOCK_EDIT, WM_GETTEXT, (WPARAM)16, (LPARAM)szText);
						}

						// Scan string in the edit control for illegal characters
						for (int i = 0; szText[i]; i++) {
							if (!_istdigit(szText[i])) {
								bValid = 0;
								break;
							}
						}

						if (bValid) {
							nValue = _tcstol(szText, NULL, 0);
							if (nValue < 25) {
								nValue = 25;
							} else { // Extra values (FB Alpha Plus!)
								if (nValue > 500) {
									nValue = 500;
								}
							}

							nValue = (int)((double)nValue * 256.0 / 100.0 + 0.5);

							// Set slider to current value
							SendDlgItemMessage(hDlg, IDC_CPUCLOCK_SLIDER, TBM_SETPOS, (WPARAM)true, (LPARAM)nValue);
						}
					}
					break;
				}
			}
			break;
		}

		case WM_HSCROLL: {
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
					if (nExitStatus == 0) {
						TCHAR szText[16];
						int nValue;

						// Update the contents of the edit control
						nValue = SendDlgItemMessage(hDlg, IDC_CPUCLOCK_SLIDER, TBM_GETPOS, 0, 0);
						nValue = (int)((double)nValue * 100.0 / 256.0 + 0.5);
						_stprintf(szText, _T("%i"), nValue);
						SendDlgItemMessage(hDlg, IDC_CPUCLOCK_EDIT, WM_SETTEXT, 0, (LPARAM)szText);
					}
					break;
				}
			}
			break;
		}

		case WM_CLOSE:
			if (nExitStatus == 1) {
				TCHAR szText[16] = _T("");

				SendDlgItemMessage(hDlg, IDC_CPUCLOCK_EDIT, WM_GETTEXT, (WPARAM)16, (LPARAM)szText);
				nBurnCPUSpeedAdjust = _tcstol(szText, NULL, 0);
				if (nBurnCPUSpeedAdjust < 25) {
					nBurnCPUSpeedAdjust = 25;
				} else { // Extra values (FB Alpha Plus!)
					if (nBurnCPUSpeedAdjust > 500) {
						nBurnCPUSpeedAdjust = 500;
					}
				}

				nBurnCPUSpeedAdjust = (int)((double)nBurnCPUSpeedAdjust * 256.0 / 100.0 + 0.5);
			}
			EndDialog(hDlg, 0);
			break;
	}

	return 0;
}

void CPUClockDialog(HWND parent)
{
	FBADialogBox(IDD_CPUCLOCK, parent, (DLGPROC)CPUClockProc);
}

// ----------------------------------------------------------------------------
// color adjust dialog, added by regret

static INT_PTR CALLBACK colorAdjustProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM)
{
	switch (Msg) {
		case WM_INITDIALOG: {
			wndInMid(hDlg, hScrnWnd);
			SetFocus(hDlg);

			// Initialise slider
			TCHAR szText[32];

			_sntprintf(szText, sizearray(szText), FBALoadStringEx(IDS_COLOR_CONTRAST), color_contrast);
			SetDlgItemText(hDlg, IDC_CONTRAST_TEXT, szText);
			_sntprintf(szText, sizearray(szText), FBALoadStringEx(IDS_COLOR_BRIGHTNESS), color_brightness);
			SetDlgItemText(hDlg, IDC_BRIGHT_TEXT, szText);
			_sntprintf(szText, sizearray(szText), FBALoadStringEx(IDS_COLOR_GAMMA), color_gamma);
			SetDlgItemText(hDlg, IDC_GAMMA_TEXT, szText);

			SendDlgItemMessage(hDlg, IDC_CONTRAST_SLIDER, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(-95, 95));
			SendDlgItemMessage(hDlg, IDC_CONTRAST_SLIDER, TBM_SETTIC, 0, 0);
			SendDlgItemMessage(hDlg, IDC_BRIGHT_SLIDER, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(-95, 95));
			SendDlgItemMessage(hDlg, IDC_BRIGHT_SLIDER, TBM_SETTIC, 0, 0);
			SendDlgItemMessage(hDlg, IDC_GAMMA_SLIDER, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(-95, 95));
			SendDlgItemMessage(hDlg, IDC_GAMMA_SLIDER, TBM_SETTIC, 0, 0);
			SendDlgItemMessage(hDlg, IDC_CONTRAST_SLIDER, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)color_contrast);
			SendDlgItemMessage(hDlg, IDC_BRIGHT_SLIDER, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)color_brightness);
			SendDlgItemMessage(hDlg, IDC_GAMMA_SLIDER, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)color_gamma);

			CheckDlgButton(hDlg, IDC_GRAYSCALE, color_grayscale ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hDlg, IDC_INVERT, color_invert ? BST_CHECKED : BST_UNCHECKED);

			// Update the screen
			if (bVidOkay && bDrvOkay) {
				VidRedraw();
				VidPaint(0);
			}

			return TRUE;
		}

		case WM_COMMAND: {
			switch (HIWORD(wParam)) {
				case BN_CLICKED: {
					bool update = false;

					if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
						SendMessage(hDlg, WM_CLOSE, 0, 0);
					}
					else if (LOWORD(wParam) == ID_DEFAULT) {
						color_contrast = color_brightness = color_gamma = 0;

						TCHAR szText[32];
						_sntprintf(szText, sizearray(szText), FBALoadStringEx(IDS_COLOR_CONTRAST), color_contrast);
						SetDlgItemText(hDlg, IDC_CONTRAST_TEXT, szText);

						_sntprintf(szText, sizearray(szText), FBALoadStringEx(IDS_COLOR_BRIGHTNESS), color_brightness);
						SetDlgItemText(hDlg, IDC_BRIGHT_TEXT, szText);

						_sntprintf(szText, sizearray(szText), FBALoadStringEx(IDS_COLOR_GAMMA), color_gamma);
						SetDlgItemText(hDlg, IDC_GAMMA_TEXT, szText);

						SendDlgItemMessage(hDlg, IDC_CONTRAST_SLIDER, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)color_contrast);
						SendDlgItemMessage(hDlg, IDC_BRIGHT_SLIDER, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)color_brightness);
						SendDlgItemMessage(hDlg, IDC_GAMMA_SLIDER, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)color_gamma);

						update = true;
					}

					if (LOWORD(wParam) == IDC_GRAYSCALE) {
						color_grayscale = !color_grayscale;
						update = true;
					}
					else if (LOWORD(wParam) == IDC_INVERT) {
						color_invert = !color_invert;
						update = true;
					}

					// Update the screen
					if (update && bVidOkay && bDrvOkay) {
						VidRedraw();
						VidPaint(0);
					}
					break;
				}
				break;
			}
		}

		case WM_HSCROLL: {
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
					TCHAR szText[32];

					color_contrast = SendDlgItemMessage(hDlg, IDC_CONTRAST_SLIDER, TBM_GETPOS, 0, 0);
					_sntprintf(szText, sizearray(szText), FBALoadStringEx(IDS_COLOR_CONTRAST), color_contrast);
					SetDlgItemText(hDlg, IDC_CONTRAST_TEXT, szText);

					color_brightness = SendDlgItemMessage(hDlg, IDC_BRIGHT_SLIDER, TBM_GETPOS, 0, 0);
					_sntprintf(szText, sizearray(szText), FBALoadStringEx(IDS_COLOR_BRIGHTNESS), color_brightness);
					SetDlgItemText(hDlg, IDC_BRIGHT_TEXT, szText);

					color_gamma = SendDlgItemMessage(hDlg, IDC_GAMMA_SLIDER, TBM_GETPOS, 0, 0);
					_sntprintf(szText, sizearray(szText), FBALoadStringEx(IDS_COLOR_GAMMA), color_gamma);
					SetDlgItemText(hDlg, IDC_GAMMA_TEXT, szText);

					// Update the screen
					if (bVidOkay && bDrvOkay) {
						VidRedraw();
						VidPaint(0);
					}
					break;
				}
			}
			break;
		}

		case WM_CLOSE:
			EndDialog(hDlg, 0);
			break;
	}

	return 0;
}

void colorAdjustDialog(HWND parent)
{
	FBADialogBox(IDD_COLORADJUST, parent, (DLGPROC)colorAdjustProc);
}

// ----------------------------------------------------------------------------
// aspect set dialog, added by regret

static INT_PTR CALLBACK aspectProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch (Msg) {
		case WM_INITDIALOG:
			TCHAR szText[16];
			wndInMid(hDlg, hScrnWnd);

			// Set the edit control to current values
			_stprintf(szText, _T("%i"), nVidScrnAspectX);
			SendDlgItemMessage(hDlg, IDC_ASPECTX_EDIT, WM_SETTEXT, 0, (LPARAM)szText);
			_stprintf(szText, _T("%i"), nVidScrnAspectY);
			SendDlgItemMessage(hDlg, IDC_ASPECTY_EDIT, WM_SETTEXT, 0, (LPARAM)szText);
			return TRUE;

		case WM_COMMAND:
			switch (HIWORD(wParam)) {
				case BN_CLICKED:
					if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
						SendMessage(hDlg, WM_CLOSE, 0, 0);
					}
					break;

				case EN_UPDATE:
					TCHAR szText[16] = _T("");
					int aspect = 0;

					switch (LOWORD(wParam)) {
						case IDC_ASPECTX_EDIT:
							SendDlgItemMessage(hDlg, IDC_ASPECTX_EDIT, WM_GETTEXT, (WPARAM)sizearray(szText), (LPARAM)szText);
							aspect = _tcstol(szText, NULL, 0);
							if (aspect > 0) {
								nVidScrnAspectX = aspect;
							}
							break;
						case IDC_ASPECTY_EDIT:
							SendDlgItemMessage(hDlg, IDC_ASPECTY_EDIT, WM_GETTEXT, (WPARAM)sizearray(szText), (LPARAM)szText);
							aspect = _tcstol(szText, NULL, 0);
							if (aspect > 0) {
								nVidScrnAspectY = aspect;
							}
							break;
					}
					break;
			}
			break;

		case WM_CLOSE:
			EndDialog(hDlg, 0);
			break;
	}

	return FALSE;
}

void aspectSetDialog(HWND parent)
{
	FBADialogBox(IDD_ASPECTSET, parent, (DLGPROC)aspectProc);
}

// ----------------------------------------------------------------------------
// Screen Angle dialog
static INT_PTR CALLBACK screenAngleProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	const double PI = 3.14159265358979323846f;			// Pi
	const double DEGTORAD = 0.01745329251994329547f;	// Degrees to Radians
	const double RADTODEG = 57.29577951308232286465f;	// Radians to Degrees

	static float fPrevScreenAngle, fPrevScreenCurvature;
	static HWND hScreenAngleSlider, hScreenAngleEdit;

	switch (Msg) {
		case WM_INITDIALOG: {
			TCHAR szText[16];
			fPrevScreenAngle = fVidScreenAngle;
			fPrevScreenCurvature = fVidScreenCurvature;
			hScreenAngleSlider = GetDlgItem(hDlg, IDC_SCREENANGLE_SLIDER);
			hScreenAngleEdit = GetDlgItem(hDlg, IDC_SCREENANGLE_EDIT);
			nExitStatus = 0;

			wndInMid(hDlg, hScrnWnd);

			// Initialise sliders
			SendDlgItemMessage(hDlg, IDC_SCREENANGLE_SLIDER, TBM_SETRANGE, 0, (LPARAM)MAKELONG(0, 12000));
			SendDlgItemMessage(hDlg, IDC_SCREENANGLE_SLIDER, TBM_SETLINESIZE, 0, (LPARAM)100);
			SendDlgItemMessage(hDlg, IDC_SCREENANGLE_SLIDER, TBM_SETPAGESIZE, 0, (LPARAM)250);
			SendDlgItemMessage(hDlg, IDC_SCREENANGLE_SLIDER, TBM_SETTIC, 0, (LPARAM)1000);
			SendDlgItemMessage(hDlg, IDC_SCREENANGLE_SLIDER, TBM_SETTIC, 0, (LPARAM)2250);
			SendDlgItemMessage(hDlg, IDC_SCREENANGLE_SLIDER, TBM_SETTIC, 0, (LPARAM)6000);

			SendDlgItemMessage(hDlg, IDC_SCREENCURVATURE_SLIDER, TBM_SETRANGE, 0, (LPARAM)MAKELONG(0, 12000));
			SendDlgItemMessage(hDlg, IDC_SCREENCURVATURE_SLIDER, TBM_SETLINESIZE, 0, (LPARAM)150);
			SendDlgItemMessage(hDlg, IDC_SCREENCURVATURE_SLIDER, TBM_SETPAGESIZE, 0, (LPARAM)375);
			SendDlgItemMessage(hDlg, IDC_SCREENCURVATURE_SLIDER, TBM_SETTIC, 0, (LPARAM)3000);
			SendDlgItemMessage(hDlg, IDC_SCREENCURVATURE_SLIDER, TBM_SETTIC, 0, (LPARAM)4500);
			SendDlgItemMessage(hDlg, IDC_SCREENCURVATURE_SLIDER, TBM_SETTIC, 0, (LPARAM)6000);

			// Set sliders to current value
			SendDlgItemMessage(hDlg, IDC_SCREENANGLE_SLIDER, TBM_SETPOS, (WPARAM)true, (LPARAM)(fVidScreenAngle * RADTODEG * 100.0f));
			SendDlgItemMessage(hDlg, IDC_SCREENCURVATURE_SLIDER, TBM_SETPOS, (WPARAM)true, (LPARAM)(fVidScreenCurvature * RADTODEG * 150.0f));

			// Set the edit controls to current value
			_stprintf(szText, _T("%0.2f"), fVidScreenAngle * RADTODEG);
			SendDlgItemMessage(hDlg, IDC_SCREENANGLE_EDIT, WM_SETTEXT, 0, (LPARAM)szText);

			_stprintf(szText, _T("%0.2f"), fVidScreenCurvature * RADTODEG);
			SendDlgItemMessage(hDlg, IDC_SCREENCURVATURE_EDIT, WM_SETTEXT, 0, (LPARAM)szText);

			// Update the screen
			if (bVidOkay) {
				VidPaint(2);
			}

			return TRUE;
		}
		case WM_COMMAND: {
			switch (HIWORD(wParam)) {
				case BN_CLICKED: {
					if (LOWORD(wParam) == IDOK) {
						nExitStatus = 1;
						SendMessage(hDlg, WM_CLOSE, 0, 0);
					}
					if (LOWORD(wParam) == IDCANCEL) {
						nExitStatus = -1;
						SendMessage(hDlg, WM_CLOSE, 0, 0);
					}
					break;
				}
				case EN_UPDATE: {
					if (nExitStatus == 0) {
						TCHAR szText[16] = _T("");
						bool bPoint = 0;
						bool bValid = 1;

						if ((HWND)lParam == hScreenAngleEdit) {
							if (SendDlgItemMessage(hDlg, IDC_SCREENANGLE_EDIT, WM_GETTEXTLENGTH, 0, 0) < 16) {
								SendDlgItemMessage(hDlg, IDC_SCREENANGLE_EDIT, WM_GETTEXT, (WPARAM)16, (LPARAM)szText);
							}
						} else {
							if (SendDlgItemMessage(hDlg, IDC_SCREENCURVATURE_EDIT, WM_GETTEXTLENGTH, 0, 0) < 16) {
								SendDlgItemMessage(hDlg, IDC_SCREENCURVATURE_EDIT, WM_GETTEXT, (WPARAM)16, (LPARAM)szText);
							}
						}

						// Scan string in the edit control for illegal characters
						for (int i = 0; szText[i]; i++) {
							if (szText[i] == '.') {
								if (bPoint) {
									bValid = 0;
									break;
								}
							} else {
								if (!_istdigit(szText[i])) {
									bValid = 0;
									break;
								}
							}
						}

						if (bValid) {
							if ((HWND)lParam == hScreenAngleEdit) {
								fVidScreenAngle = _tcstod(szText, NULL) * DEGTORAD;
								if (fVidScreenAngle < 0.0f) {
									fVidScreenAngle = 0.0f;
								} else {
									if (fVidScreenAngle > PI / 1.5f) {
										fVidScreenAngle = PI / 1.5f;
									}
								}

								// Set slider to current value
								SendDlgItemMessage(hDlg, IDC_SCREENANGLE_SLIDER, TBM_SETPOS, (WPARAM)true, (LPARAM)(fVidScreenAngle * RADTODEG * 100.0f));
							} else {
								fVidScreenCurvature = _tcstod(szText, NULL) * DEGTORAD;
								if (fVidScreenCurvature < 0.0f) {
									fVidScreenCurvature = 0.0f;
								} else {
									if (fVidScreenCurvature > PI / 2.25f) {
										fVidScreenCurvature = PI / 2.25f;
									}
								}

								// Set slider to current value
								SendDlgItemMessage(hDlg, IDC_SCREENCURVATURE_SLIDER, TBM_SETPOS, (WPARAM)true, (LPARAM)(fVidScreenCurvature * RADTODEG * 150.0f));
							}

							// Update the screen
							if (bVidOkay) {
								VidPaint(2);
							}
						}
					}
					break;
				}
			}
			break;
		}

		case WM_HSCROLL: {
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
					if (nExitStatus == 0) {
						TCHAR szText[16];

						// Update the contents of the edit control
						if ((HWND)lParam == hScreenAngleSlider) {
							fVidScreenAngle = (float)SendDlgItemMessage(hDlg, IDC_SCREENANGLE_SLIDER, TBM_GETPOS, 0, 0) * DEGTORAD / 100.0f;
							_stprintf(szText, _T("%0.2f"), fVidScreenAngle * RADTODEG);
							SendDlgItemMessage(hDlg, IDC_SCREENANGLE_EDIT, WM_SETTEXT, 0, (LPARAM)szText);
						} else {
							fVidScreenCurvature = (float)SendDlgItemMessage(hDlg, IDC_SCREENCURVATURE_SLIDER, TBM_GETPOS, 0, 0) * DEGTORAD / 150.0f;
							_stprintf(szText, _T("%0.2f"), fVidScreenCurvature * RADTODEG);
							SendDlgItemMessage(hDlg, IDC_SCREENCURVATURE_EDIT, WM_SETTEXT, 0, (LPARAM)szText);
						}

						// Update the screen
						if (bVidOkay) {
							VidPaint(0);
						}
					}
					break;
				}
			}
			break;
		}

		case WM_CLOSE:
			if (nExitStatus != 1) {
				fVidScreenAngle = fPrevScreenAngle;
				fVidScreenCurvature = fPrevScreenCurvature;
			}
			EndDialog(hDlg, 0);
			break;
	}

	return 0;
}

void screenAngleDialog(HWND parent)
{
	FBADialogBox(IDD_SCREENANGLE, parent, (DLGPROC)screenAngleProc);
}

#if 0
// ----------------------------------------------------------------------------
// audio frame dialog, modified by regret
static INT_PTR CALLBACK audFrameProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM)
{
	switch (Msg) {
		case WM_INITDIALOG:
			wndInMid(hDlg, hScrnWnd);

			// Initialise slider
			TCHAR szText[64];
			_sntprintf(szText, sizearray(szText), FBALoadStringEx(IDS_NUMDLG_FRAME), nAudSegCount, int(1000.0 * (nAudSegCount - 1.0) / 60));
			SetDlgItemText(hDlg, IDC_AUDFRAME_TEXT, szText);

			// limit in 2 - 10
			SendDlgItemMessage(hDlg, IDC_AUDFRAME_SLIDER, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(2, 10));
			SendDlgItemMessage(hDlg, IDC_AUDFRAME_SLIDER, TBM_SETTIC, 0, (LPARAM)6);
			SendDlgItemMessage(hDlg, IDC_AUDFRAME_SLIDER, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)nAudSegCount);
			return TRUE;

		case WM_COMMAND:
			if (HIWORD(wParam) == BN_CLICKED) {
				if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
					SendMessage(hDlg, WM_CLOSE, 0, 0);
				}
				break;
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
					TCHAR szText[64];
					int frame = SendDlgItemMessage(hDlg, IDC_AUDFRAME_SLIDER, TBM_GETPOS, 0, 0);
					_sntprintf(szText, sizearray(szText), FBALoadStringEx(IDS_NUMDLG_FRAME), frame, int(1000.0 * (frame - 1.0) / 60));
					SetDlgItemText(hDlg, IDC_AUDFRAME_TEXT, szText);
					break;
				}
			}
			break;

		case WM_CLOSE:
			nAudSegCount = SendDlgItemMessage(hDlg, IDC_AUDFRAME_SLIDER, TBM_GETPOS, 0, 0);
			EndDialog(hDlg, 0);
			break;
	}

	return 0;
}

int audFrameCreate(HWND parent)
{
	FBADialogBox(IDD_FRAMES, parent, (DLGPROC)audFrameProc);
	return 1;
}

// ----------------------------------------------------------------------------
// Feedback intensity dialog
static INT_PTR CALLBACK PhosphorProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)	// LPARAM lParam
{
	static int nPrevIntensity;
	static int nPrevSaturation;

	switch (Msg) {
		case WM_INITDIALOG: {
			TCHAR szText[16];
			nPrevIntensity = nVidFeedbackIntensity;
			nPrevSaturation = nVidFeedbackOverSaturation;
			nExitStatus = 0;

			wndInMid(hDlg, hScrnWnd);

			// Initialise sliders
			SendDlgItemMessage(hDlg, IDC_PHOSPHOR_1_SLIDER, TBM_SETRANGE, 0, (LPARAM)MAKELONG(0, 255));
			SendDlgItemMessage(hDlg, IDC_PHOSPHOR_1_SLIDER, TBM_SETPAGESIZE, 0, (LPARAM)8);
			SendDlgItemMessage(hDlg, IDC_PHOSPHOR_1_SLIDER, TBM_SETTIC, 0, (LPARAM)127);
			SendDlgItemMessage(hDlg, IDC_PHOSPHOR_1_SLIDER, TBM_SETTIC, 0, (LPARAM)63);
			SendDlgItemMessage(hDlg, IDC_PHOSPHOR_1_SLIDER, TBM_SETTIC, 0, (LPARAM)31);
			SendDlgItemMessage(hDlg, IDC_PHOSPHOR_1_SLIDER, TBM_SETTIC, 0, (LPARAM)15);

			SendDlgItemMessage(hDlg, IDC_PHOSPHOR_2_SLIDER, TBM_SETRANGE, 0, (LPARAM)MAKELONG(0, 127));
			SendDlgItemMessage(hDlg, IDC_PHOSPHOR_2_SLIDER, TBM_SETPAGESIZE, 0, (LPARAM)4);
			SendDlgItemMessage(hDlg, IDC_PHOSPHOR_2_SLIDER, TBM_SETTIC, 0, (LPARAM)63);
			SendDlgItemMessage(hDlg, IDC_PHOSPHOR_2_SLIDER, TBM_SETTIC, 0, (LPARAM)31);
			SendDlgItemMessage(hDlg, IDC_PHOSPHOR_2_SLIDER, TBM_SETTIC, 0, (LPARAM)15);

			// Set slider to current values
			SendDlgItemMessage(hDlg, IDC_PHOSPHOR_1_SLIDER, TBM_SETPOS, (WPARAM)true, (LPARAM)nVidFeedbackIntensity);
			SendDlgItemMessage(hDlg, IDC_PHOSPHOR_2_SLIDER, TBM_SETPOS, (WPARAM)true, (LPARAM)nVidFeedbackOverSaturation);

			// Set the edit control to current values
			_stprintf(szText, _T("%i"), nVidFeedbackIntensity);
			SendDlgItemMessage(hDlg, IDC_PHOSPHOR_1_EDIT, WM_SETTEXT, 0, (LPARAM)szText);
			_stprintf(szText, _T("%i"), nVidFeedbackOverSaturation);
			SendDlgItemMessage(hDlg, IDC_PHOSPHOR_2_EDIT, WM_SETTEXT, 0, (LPARAM)szText);

			return TRUE;
		}
		case WM_COMMAND: {
			switch (HIWORD(wParam)) {
				case BN_CLICKED: {
					if (LOWORD(wParam) == IDOK) {
						nExitStatus = 1;
						SendMessage(hDlg, WM_CLOSE, 0, 0);
					}
					if (LOWORD(wParam) == IDCANCEL) {
						nExitStatus = -1;
						SendMessage(hDlg, WM_CLOSE, 0, 0);
					}
					break;
				}
				case EN_UPDATE: {
					if (nExitStatus == 0) {
						TCHAR szText[16] = _T("");
						bool bValid = 1;

						switch (LOWORD(wParam)) {
							case IDC_PHOSPHOR_1_EDIT:
								if (SendDlgItemMessage(hDlg, IDC_PHOSPHOR_1_EDIT, WM_GETTEXTLENGTH, 0, 0) < 16) {
									SendDlgItemMessage(hDlg, IDC_PHOSPHOR_1_EDIT, WM_GETTEXT, (WPARAM)16, (LPARAM)szText);
								}

								// Scan string in the edit control for illegal characters
								for (int i = 0; szText[i]; i++) {
									if (!_istdigit(szText[i])) {
										bValid = 0;
										break;
									}
								}

								if (bValid) {
									nVidFeedbackIntensity = _tcstol(szText, NULL, 0);
									if (nVidFeedbackIntensity < 0) {
										nVidFeedbackIntensity = 0;
									} else {
										if (nVidFeedbackIntensity > 255) {
											nVidFeedbackIntensity = 255;
										}
									}

									// Set slider to current value
									SendDlgItemMessage(hDlg, IDC_PHOSPHOR_1_SLIDER, TBM_SETPOS, (WPARAM)true, (LPARAM)nVidFeedbackIntensity);
								}
								break;
							case IDC_PHOSPHOR_2_EDIT:
								if (SendDlgItemMessage(hDlg, IDC_PHOSPHOR_2_EDIT, WM_GETTEXTLENGTH, 0, 0) < 16) {
									SendDlgItemMessage(hDlg, IDC_PHOSPHOR_2_EDIT, WM_GETTEXT, (WPARAM)16, (LPARAM)szText);
								}

								// Scan string in the edit control for illegal characters
								for (int i = 0; szText[i]; i++) {
									if (!_istdigit(szText[i])) {
										bValid = 0;
										break;
									}
								}

								if (bValid) {
									nVidFeedbackOverSaturation = _tcstol(szText, NULL, 0);
									if (nVidFeedbackOverSaturation < 0) {
										nVidFeedbackOverSaturation = 0;
									} else {
										if (nVidFeedbackOverSaturation > 255) {
											nVidFeedbackOverSaturation = 255;
										}
									}

									// Set slider to current value
									SendDlgItemMessage(hDlg, IDC_PHOSPHOR_2_SLIDER, TBM_SETPOS, (WPARAM)true, (LPARAM)nVidFeedbackOverSaturation);

									// Update the screen
									if (bVidOkay) {
										VidPaint(2);
									}
								}
								break;
						}
					}
					break;
				}
			}
			break;
		}

		case WM_HSCROLL: {
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
					if (nExitStatus == 0) {
						TCHAR szText[16];

						// Update the contents of the edit control
						switch (GetDlgCtrlID((HWND)lParam)) {
							case IDC_PHOSPHOR_1_SLIDER:
								nVidFeedbackIntensity = SendDlgItemMessage(hDlg, IDC_PHOSPHOR_1_SLIDER, TBM_GETPOS, 0, 0);
								_stprintf(szText, _T("%i"), nVidFeedbackIntensity);
								SendDlgItemMessage(hDlg, IDC_PHOSPHOR_1_EDIT, WM_SETTEXT, 0, (LPARAM)szText);
								break;
							case IDC_PHOSPHOR_2_SLIDER:
								nVidFeedbackOverSaturation = SendDlgItemMessage(hDlg, IDC_PHOSPHOR_2_SLIDER, TBM_GETPOS, 0, 0);
								_stprintf(szText, _T("%i"), nVidFeedbackOverSaturation);
								SendDlgItemMessage(hDlg, IDC_PHOSPHOR_2_EDIT, WM_SETTEXT, 0, (LPARAM)szText);
								break;
						}
					}
					break;
				}
			}
			break;
		}

		case WM_CLOSE:
			if (nExitStatus != 1) {
				nVidFeedbackIntensity = nPrevIntensity;
				nVidFeedbackOverSaturation = nPrevSaturation;
			}
			EndDialog(hDlg, 0);
			break;
	}

	return 0;
}

void PhosphorDialog(HWND parent)
{
	FBADialogBox(IDD_PHOSPHOR, parent, (DLGPROC)PhosphorProc);
}

// ----------------------------------------------------------------------------
// Scanline intensity dialog

static INT_PTR CALLBACK ScanlineProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM)	// LPARAM lParam
{
	static int nPrevIntensity;

	switch (Msg) {
		case WM_INITDIALOG: {
			TCHAR szText[16];
			nPrevIntensity = nVidScanIntensity;
			nExitStatus = 0;

			wndInMid(hDlg, hScrnWnd);

			// Initialise slider
			SendDlgItemMessage(hDlg, IDC_SCANLINE_SLIDER, TBM_SETRANGE, 0, (LPARAM)MAKELONG(0, 255));
			SendDlgItemMessage(hDlg, IDC_SCANLINE_SLIDER, TBM_SETPAGESIZE, 0, (LPARAM)16);
			SendDlgItemMessage(hDlg, IDC_SCANLINE_SLIDER, TBM_SETTIC, 0, (LPARAM)191);
			SendDlgItemMessage(hDlg, IDC_SCANLINE_SLIDER, TBM_SETTIC, 0, (LPARAM)127);
			SendDlgItemMessage(hDlg, IDC_SCANLINE_SLIDER, TBM_SETTIC, 0, (LPARAM)63);

			// Set slider to current value
			SendDlgItemMessage(hDlg, IDC_SCANLINE_SLIDER, TBM_SETPOS, (WPARAM)true, (LPARAM)nVidScanIntensity & 0xFF);

			// Set the edit control to current value
			_stprintf(szText, _T("%i"), nVidScanIntensity & 0xFF);
			SendDlgItemMessage(hDlg, IDC_SCANLINE_EDIT, WM_SETTEXT, 0, (LPARAM)szText);

			// Update the screen
			if (bDrvOkay) {
				VidPaint(2);
			}

			return TRUE;
		}
		case WM_COMMAND: {
			switch (HIWORD(wParam)) {
				case BN_CLICKED: {
					if (LOWORD(wParam) == IDOK) {
						nExitStatus = 1;
						SendMessage(hDlg, WM_CLOSE, 0, 0);
					}
					if (LOWORD(wParam) == IDCANCEL) {
						nExitStatus = -1;
						SendMessage(hDlg, WM_CLOSE, 0, 0);
					}
					break;
				}
				case EN_UPDATE: {
					if (nExitStatus == 0) {
						TCHAR szText[16] = _T("");
						bool bValid = 1;

						if (SendDlgItemMessage(hDlg, IDC_SCANLINE_EDIT, WM_GETTEXTLENGTH, 0, 0) < 16) {
							SendDlgItemMessage(hDlg, IDC_SCANLINE_EDIT, WM_GETTEXT, (WPARAM)16, (LPARAM)szText);
						}

						// Scan string in the edit control for illegal characters
						for (int i = 0; szText[i]; i++) {
							if (!_istdigit(szText[i])) {
								bValid = 0;
								break;
							}
						}

						if (bValid) {
							nVidScanIntensity = _tcstol(szText, NULL, 0);
							if (nVidScanIntensity < 0) {
								nVidScanIntensity = 0;
							} else {
								if (nVidScanIntensity > 255) {
									nVidScanIntensity = 255;
								}
							}

							// Set slider to current value
							SendDlgItemMessage(hDlg, IDC_SCANLINE_SLIDER, TBM_SETPOS, (WPARAM)true, (LPARAM)nVidScanIntensity);

							nVidScanIntensity |= (nVidScanIntensity << 8) | (nVidScanIntensity << 16);

							// Update the screen
							if (bVidOkay) {
								VidPaint(2);
							}
						}
					}
					break;
				}
			}
			break;
		}

		case WM_HSCROLL: {
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
					if (nExitStatus == 0) {
						TCHAR szText[16];

						// Update the contents of the edit control
						nVidScanIntensity = SendDlgItemMessage(hDlg, IDC_SCANLINE_SLIDER, TBM_GETPOS, 0, 0);
						_stprintf(szText, _T("%i"), nVidScanIntensity);
						SendDlgItemMessage(hDlg, IDC_SCANLINE_EDIT, WM_SETTEXT, 0, (LPARAM)szText);

						nVidScanIntensity |= (nVidScanIntensity << 8) | (nVidScanIntensity << 16);
						// Update the screen
						if (bVidOkay) {
//							VidRedraw();
							VidPaint(2);
						}
					}
					break;
				}
			}
			break;
		}

		case WM_CLOSE:
			if (nExitStatus != 1) {
				nVidScanIntensity = nPrevIntensity;
			}
			EndDialog(hDlg, 0);
			break;
	}

	return 0;
}

void ScanlineDialog(HWND parent)
{
	FBADialogBox(IDD_SCANLINE, parent, (DLGPROC)ScanlineProc);
}

// ----------------------------------------------------------------------------
static INT_PTR CALLBACK DefInpProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM)
{
 	int nRet = 0;
	BOOL fTrue = FALSE;

	switch (Msg) {
		case WM_INITDIALOG:
			wndInMid(hDlg, hScrnWnd);

			nRet = nAudSegCount;
			SetWindowText(hDlg, FBALoadStringEx(IDS_NUMDLG_FRAME));
			SetDlgItemInt(hDlg, IDC_VALUE_EDIT, nRet, TRUE);
			return TRUE;

		case WM_COMMAND:
			if (LOWORD(wParam) == IDC_VALUE_CLOSE) {
				SendMessage(hDlg, WM_CLOSE, 0, 0);
			} else {
				if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDCANCEL) {
					SendMessage(hDlg, WM_CLOSE, 0, 0);
				}
			}
			break;

		case WM_CLOSE:
			nRet = GetDlgItemInt(hDlg, IDC_VALUE_EDIT, &fTrue, TRUE);
			if (fTrue) {
				nAudSegCount = nRet;
				// limit in 2 - 10
				if (nAudSegCount < 2) {
					nAudSegCount = 2;
				} else {
					if (nAudSegCount > 10) {
						nAudSegCount = 10;
					}
				}
			}
			EndDialog(hDlg, 0);
	}

	return 0;
}

int NumDialCreate(int, HWND parent)
{
	FBADialogBox(IDD_VALUE, parent, (DLGPROC)DefInpProc);
	return 1;
}

// ----------------------------------------------------------------------------
// Cubic filter quality dialog

static INT_PTR CALLBACK CubicProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM)	// LPARAM lParam
{
	static double dPrevB, dPrevC;

	switch (Msg) {
		case WM_INITDIALOG: {
			TCHAR szText[16];
			dPrevB = dVidCubicB;
			dPrevC = dVidCubicC;
			nExitStatus = 0;

			wndInMid(hDlg, hScrnWnd);

			// Initialise slider
			SendDlgItemMessage(hDlg, IDC_SCANLINE_SLIDER, TBM_SETRANGE, 0, (LPARAM)MAKELONG(0, 10000));
			SendDlgItemMessage(hDlg, IDC_SCANLINE_SLIDER, TBM_SETLINESIZE, 0, (LPARAM)100);
			SendDlgItemMessage(hDlg, IDC_SCANLINE_SLIDER, TBM_SETPAGESIZE, 0, (LPARAM)500);
			SendDlgItemMessage(hDlg, IDC_SCANLINE_SLIDER, TBM_SETTIC, 0, (LPARAM)3333);
			SendDlgItemMessage(hDlg, IDC_SCANLINE_SLIDER, TBM_SETTIC, 0, (LPARAM)5000);
			SendDlgItemMessage(hDlg, IDC_SCANLINE_SLIDER, TBM_SETTIC, 0, (LPARAM)7500);

			// Set slider to current value
			SendDlgItemMessage(hDlg, IDC_SCANLINE_SLIDER, TBM_SETPOS, (WPARAM)true, (LPARAM)((1.0 - dVidCubicB) * 10000));

			// Set the edit control to current value
			_stprintf(szText, _T("%.3lf"), 1.0 - dVidCubicB);
			SendDlgItemMessage(hDlg, IDC_SCANLINE_EDIT, WM_SETTEXT, 0, (LPARAM)szText);

			SetWindowText(hDlg, _T("Select desired filter sharpness"));

			// Update the screen
			if (bDrvOkay) {
				VidPaint(2);
			}

			return TRUE;
		}
		case WM_COMMAND: {
			switch (HIWORD(wParam)) {
				case BN_CLICKED: {
					if (LOWORD(wParam) == IDOK) {
						nExitStatus = 1;
						SendMessage(hDlg, WM_CLOSE, 0, 0);
					}
					if (LOWORD(wParam) == IDCANCEL) {
						nExitStatus = -1;
						SendMessage(hDlg, WM_CLOSE, 0, 0);
					}
					break;
				}
				case EN_UPDATE: {
					if (nExitStatus == 0) {
						TCHAR szText[16] = _T("");
						bool bPoint = 0;
						bool bValid = 1;

						if (SendDlgItemMessage(hDlg, IDC_SCANLINE_EDIT, WM_GETTEXTLENGTH, 0, 0) < 16) {
							SendDlgItemMessage(hDlg, IDC_SCANLINE_EDIT, WM_GETTEXT, (WPARAM)16, (LPARAM)szText);
						}

						// Scan string in the edit control for illegal characters
						for (int i = 0; szText[i]; i++) {
							if (szText[i] == _T('.')) {
								if (bPoint) {
									bValid = 0;
									break;
								}
							} else {
								if (!_istdigit(szText[i])) {
									bValid = 0;
									break;
								}
							}
						}

						if (bValid) {
							dVidCubicB = 1.0 - _tcstod(szText, NULL);
							if (dVidCubicB < 0.0) {
								dVidCubicB = 0.0;
							} else {
								if (dVidCubicB > 1.0) {
									dVidCubicB = 1.0;
								}
							}

							// Set slider to current value
							SendDlgItemMessage(hDlg, IDC_SCANLINE_SLIDER, TBM_SETPOS, (WPARAM)true, (LPARAM)((1.0 - dVidCubicB) * 10000));

							// Update the screen
							if (bVidOkay) {
								VidPaint(2);
							}
						}
					}
					break;
				}
			}
			break;
		}

		case WM_HSCROLL: {
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
					if (nExitStatus == 0) {
						TCHAR szText[16];

						// Update the contents of the edit control
						dVidCubicB = 1.0 - (double)SendDlgItemMessage(hDlg, IDC_SCANLINE_SLIDER, TBM_GETPOS, 0, 0) / 10000;
						_stprintf(szText, _T("%.3lf"), 1.0 - dVidCubicB);
						SendDlgItemMessage(hDlg, IDC_SCANLINE_EDIT, WM_SETTEXT, 0, (LPARAM)szText);

						// Update the screen
						if (bVidOkay) {
//							VidRedraw();
							VidPaint(2);
						}
					}
					break;
				}
			}
			break;
		}

		case WM_CLOSE:
			if (nExitStatus != 1) {
				dVidCubicB = dPrevB;
				dVidCubicC = dPrevC;
			}
			EndDialog(hDlg, 0);
			break;
	}

	return 0;
}

void CubicSharpnessDialog(HWND parent)
{
	FBADialogBox(IDD_SCANLINE, parent, (DLGPROC)CubicProc);
}

// ----------------------------------------------------------------------------
// Screen visible area offset dialog

#define MAX_VIS_OFFSET 63

static INT_PTR CALLBACK visibleOffsetProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch (Msg) {
		case WM_INITDIALOG:
			TCHAR szText[16];
			wndInMid(hDlg, hScrnWnd);

			// Set the edit control to current values
			_stprintf(szText, _T("%i"), nScrnVisibleOffset[0]);
			SendDlgItemMessage(hDlg, IDC_AREATOP_EDIT, WM_SETTEXT, 0, (LPARAM)szText);
			_stprintf(szText, _T("%i"), nScrnVisibleOffset[1]);
			SendDlgItemMessage(hDlg, IDC_AREALEFT_EDIT, WM_SETTEXT, 0, (LPARAM)szText);
			_stprintf(szText, _T("%i"), nScrnVisibleOffset[3]);
			SendDlgItemMessage(hDlg, IDC_AREARIGHT_EDIT, WM_SETTEXT, 0, (LPARAM)szText);
			_stprintf(szText, _T("%i"), nScrnVisibleOffset[2]);
			SendDlgItemMessage(hDlg, IDC_AREABOTTOM_EDIT, WM_SETTEXT, 0, (LPARAM)szText);
			return TRUE;

		case WM_COMMAND:
			switch (HIWORD(wParam)) {
				case BN_CLICKED:
					if (LOWORD(wParam) == IDOK) {
						SendMessage(hDlg, WM_CLOSE, 0, 0);
					}
					break;
				case EN_UPDATE:
					TCHAR szText[16] = _T("");
					int nGetOffset = 0;

					switch (LOWORD(wParam)) {
						case IDC_AREATOP_EDIT:
							if (SendDlgItemMessage(hDlg, IDC_AREATOP_EDIT, WM_GETTEXTLENGTH, 0, 0) < sizearray(szText)) {
								SendDlgItemMessage(hDlg, IDC_AREATOP_EDIT, WM_GETTEXT, (WPARAM)sizearray(szText), (LPARAM)szText);
								nGetOffset = _tcstol(szText, NULL, 0);
								if (nGetOffset > MAX_VIS_OFFSET) {
									nScrnVisibleOffset[0] = MAX_VIS_OFFSET;
								} else {
									nScrnVisibleOffset[0] = nGetOffset;
								}
							}
							break;
						case IDC_AREALEFT_EDIT:
							if (SendDlgItemMessage(hDlg, IDC_AREALEFT_EDIT, WM_GETTEXTLENGTH, 0, 0) < sizearray(szText)) {
								SendDlgItemMessage(hDlg, IDC_AREALEFT_EDIT, WM_GETTEXT, (WPARAM)sizearray(szText), (LPARAM)szText);
								nGetOffset = _tcstol(szText, NULL, 0);
								if (nGetOffset > MAX_VIS_OFFSET) {
									nScrnVisibleOffset[1] = MAX_VIS_OFFSET;
								} else {
									nScrnVisibleOffset[1] = nGetOffset;
								}
							}
							break;
						case IDC_AREARIGHT_EDIT:
							if (SendDlgItemMessage(hDlg, IDC_AREARIGHT_EDIT, WM_GETTEXTLENGTH, 0, 0) < sizearray(szText)) {
								SendDlgItemMessage(hDlg, IDC_AREARIGHT_EDIT, WM_GETTEXT, (WPARAM)sizearray(szText), (LPARAM)szText);
								nGetOffset = _tcstol(szText, NULL, 0);
								if (nGetOffset > MAX_VIS_OFFSET) {
									nScrnVisibleOffset[3] = MAX_VIS_OFFSET;
								} else {
									nScrnVisibleOffset[3] = nGetOffset;
								}
							}
							break;
						case IDC_AREABOTTOM_EDIT:
							if (SendDlgItemMessage(hDlg, IDC_AREABOTTOM_EDIT, WM_GETTEXTLENGTH, 0, 0) < sizearray(szText)) {
								SendDlgItemMessage(hDlg, IDC_AREABOTTOM_EDIT, WM_GETTEXT, (WPARAM)sizearray(szText), (LPARAM)szText);
								nGetOffset = _tcstol(szText, NULL, 0);
								if (nGetOffset > MAX_VIS_OFFSET) {
									nScrnVisibleOffset[2] = MAX_VIS_OFFSET;
								} else {
									nScrnVisibleOffset[2] = nGetOffset;
								}
							}
							break;
					}

					// Update the screen
					scrnSize();
					break;
			}
			break;

		case WM_CLOSE:
			EndDialog(hDlg, 0);
			break;
	}

	return 0;
}

void scrnVisibleOffsetDialog(HWND parent)
{
	FBADialogBox(IDD_VISOFFSET, parent, (DLGPROC)visibleOffsetProc);
}
#endif
