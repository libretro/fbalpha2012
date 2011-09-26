// Module for DirectInput
#include "burner.h"
#include <math.h>
// Key codes
#include "inp_keys.h"

#define DIRECTINPUT_VERSION 0x0700
#include <dinput.h>

#ifdef _MSC_VER
#pragma comment(lib, "dinput")
#pragma comment(lib, "dxguid")
#endif

#define LIST_DEVICES
//#define LIST_CONTROLS

static IDirectInput7* pDI = NULL;					// DirectInput interface
static HWND hDinpWnd = NULL;						// The window handle to use for DirectInput

#define MAX_KEYBOARD	(1)
#define MAX_JOYSTICK	(8)
#define MAX_JOYAXIS		(8)
#define MAX_MOUSE		(4)
#define MAX_MOUSEAXIS	(3)

const int DEADZONE = 0x8000;

static struct DinpKeyboardProperties {
	IDirectInputDevice* lpdid;
	unsigned char state[256];
	unsigned char bReadStatus;
} DinpKeyboardProperties[MAX_KEYBOARD];

static struct DinpJoyProperties {
	IDirectInputDevice7* lpdid7;
	DIJOYSTATE dijs;
	DWORD dwAxisType[MAX_JOYAXIS];
	DWORD dwAxisBaseline[MAX_JOYAXIS];
	DWORD dwAxes;
	DWORD dwPOVs;
	DWORD dwButtons;
	unsigned char bReadStatus;
} DinpJoyProperties[MAX_JOYSTICK];

static struct DinpMouseProperties {
	IDirectInputDevice7* lpdid7;
	DIMOUSESTATE2 dims;
	DWORD dwAxisType[MAX_MOUSEAXIS];
	DWORD dwAxes;
	DWORD dwButtons;
	unsigned char bReadStatus;
} DinpMouseProperties[MAX_MOUSE];

static int DinpKeyboardCount = 0;					// Number of keyboards connected to this machine
static int DinpJoyCount = 0;						// Number of joysticks connected to this machine
static int DinpMouseCount = 0;						// Number of mice connected to this machine

// ----------------------------------------------------------------------------

#if defined LIST_DEVICES && defined FBA_DEBUG
static const TCHAR* GetDeviceSubtype(DWORD dwDevType)
{
	switch (GET_DIDEVICE_TYPE(dwDevType)) {
		case DIDEVTYPE_KEYBOARD: {
			switch (GET_DIDEVICE_SUBTYPE(dwDevType)) {
				case DIDEVTYPEKEYBOARD_PCXT:
					return _T("PC XT keyboard");
				case DIDEVTYPEKEYBOARD_PCAT:
					return _T("PC AT keyboard");
				case DIDEVTYPEKEYBOARD_PCENH:
					return _T("Enhanced PC keyboard");
				case DIDEVTYPEKEYBOARD_JAPAN106:
					return _T("106/109 Japanese keyboard");
			}
			return _T("Unknown keyboard type");
		}
		case DIDEVTYPE_MOUSE: {
			switch (GET_DIDEVICE_SUBTYPE(dwDevType)) {
				case DIDEVTYPEMOUSE_TRADITIONAL:
					return _T("Mouse");
				case DIDEVTYPEMOUSE_FINGERSTICK:
					return _T("Fingerstick");
				case DIDEVTYPEMOUSE_TOUCHPAD:
					return _T("Touchpad");
				case DIDEVTYPEMOUSE_TRACKBALL:
					return _T("Trackball");
			}
			return _T("Unknown mouse type");
		}
		case DIDEVTYPE_JOYSTICK: {
			switch (GET_DIDEVICE_SUBTYPE(dwDevType)) {
				case DIDEVTYPEJOYSTICK_TRADITIONAL:
					return _T("Joystick");
				case DIDEVTYPEJOYSTICK_FLIGHTSTICK:
					return _T("Flightstick");
				case DIDEVTYPEJOYSTICK_GAMEPAD:
					return _T("Gamepad");
				case DIDEVTYPEJOYSTICK_RUDDER:
					return _T("Rudder");
				case DIDEVTYPEJOYSTICK_WHEEL:
					return _T("Wheel");
			}
			return _T("Unknown joystick type");
		}
	}

	return _T("Unknown device");
}
#endif

// ----------------------------------------------------------------------------

// Get a DI interface to the keyboard
static int DinpKeyboardInitMaster()
{
	if (FAILED(pDI->CreateDevice(GUID_SysKeyboard, &DinpKeyboardProperties[0].lpdid, NULL))) {
		return 1;
	}

#if defined LIST_DEVICES && defined FBA_DEBUG
	{
		DIDEVICEINSTANCE didi;
		DIDEVCAPS didcl;

		memset(&didi, 0, sizeof(didi));
		didi.dwSize = sizeof(didi);
		memset(&didcl, 0, sizeof(didcl));
		didcl.dwSize = sizeof(didcl);

		DinpKeyboardProperties[0].lpdid->GetDeviceInfo(&didi);
		DinpKeyboardProperties[0].lpdid->GetCapabilities(&didcl);
		dprintf(_T("  * System keyboard: %s\n"), didi.tszProductName);
		dprintf(_T("    %s; %i keys\n"), GetDeviceSubtype(didcl.dwDevType), didcl.dwButtons);
	}
#endif

	DinpKeyboardProperties[0].lpdid->SetDataFormat(&c_dfDIKeyboard);
	DinpKeyboardProperties[0].lpdid->SetCooperativeLevel(hDinpWnd, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);
	DinpKeyboardProperties[0].lpdid->Acquire();

	DinpKeyboardCount = 1;

	return 0;
}

// ----------------------------------------------------------------------------

// Callback that evaluates and sets up each joystick axis
static BOOL CALLBACK DinpJoyEnumDeviceObjectsCallback(LPCDIDEVICEOBJECTINSTANCE lpdidoi, LPVOID pvRef)
{
// We can use lpdidoi->dwOfs to determine the axis, as we are only using joystick devices
// Note that under DirectX 7.0 or lower, slider 0 can appear as the Z axis;
// This is reflected in lpdidoi->dwType, but not lpdidoi->dwOfs or lpdidoi->dwUsage

#ifdef _M_X64
 #define USE_DEVICETYPE
#endif

#ifdef USE_DEVICETYPE
	DWORD dwInstanceOfs[MAX_JOYAXIS] = { DIJOFS_X, DIJOFS_Y, DIJOFS_Z, DIJOFS_RX, DIJOFS_RY, DIJOFS_RZ, DIJOFS_SLIDER(0), DIJOFS_SLIDER(1) };
#endif

#if defined LIST_DEVICES && defined LIST_CONTROLS && defined FBA_DEBUG
	switch (DIDFT_GETTYPE(lpdidoi->dwType)) {
		case DIDFT_RELAXIS: {
 #ifdef USE_DEVICETYPE
			dprintf(_T("    axis (relative) %2i: "), DIDFT_GETINSTANCE(lpdidoi->dwType));
 #else
			dprintf(_T("    axis (relative) %2i: "), lpdidoi->dwOfs / sizeof(DWORD));
 #endif
			break;
		}
		case DIDFT_ABSAXIS: {
 #ifdef USE_DEVICETYPE
			dprintf(_T("    axis (absolute) %2i: "), DIDFT_GETINSTANCE(lpdidoi->dwType));
 #else
			dprintf(_T("    axis (absolute) %2i: "), lpdidoi->dwOfs / sizeof(DWORD));
 #endif
			break;
		}
		case DIDFT_AXIS: {
 #ifdef USE_DEVICETYPE
			dprintf(_T("    axis            %2i: "), DIDFT_GETINSTANCE(lpdidoi->dwType));
 #else
			dprintf(_T("    axis            %2i: "), lpdidoi->dwOfs / sizeof(DWORD));
 #endif
			break;
		}

		case DIDFT_PSHBUTTON: {
			dprintf(_T("    pushbutton      %2i: "), DIDFT_GETINSTANCE(lpdidoi->dwType));
			break;
		}
		case DIDFT_TGLBUTTON: {
			dprintf(_T("    toggle          %2i: "), DIDFT_GETINSTANCE(lpdidoi->dwType));
			break;
		}
		case DIDFT_BUTTON: {
			dprintf(_T("    button          %2i: "), DIDFT_GETINSTANCE(lpdidoi->dwType));
			break;
		}


		case DIDFT_POV: {
			dprintf(_T("    POV hat         %2i: "), DIDFT_GETINSTANCE(lpdidoi->dwType));
			break;
		}

		case DIDFT_COLLECTION: {
			dprintf(_T("    Collection      %2i: "), DIDFT_GETINSTANCE(lpdidoi->dwType));
			break;
		}
		default: {
			dprintf(_T("    Unknown         %2i: "), DIDFT_GETINSTANCE(lpdidoi->dwType));
			break;
		}
	}

	dprintf(_T("%s%s\n"), lpdidoi->tszName, (lpdidoi->dwType & DIDFT_FFACTUATOR) ? _T(" (force)") : _T(""));

#endif

	if (!(DIDFT_GETTYPE(lpdidoi->dwType) & DIDFT_AXIS)) {
		return DIENUM_CONTINUE;
	}

	struct DinpJoyProperties* joydevice = (struct DinpJoyProperties*)pvRef;

	// Store the flags for later use
#ifdef USE_DEVICETYPE
	joydevice->dwAxisType[DIDFT_GETINSTANCE(lpdidoi->dwType)] = lpdidoi->dwType;
#else
	joydevice->dwAxisType[lpdidoi->dwOfs / sizeof(DWORD)] = lpdidoi->dwType;
#endif

	// Set axis range
	DIPROPRANGE diprg;
	memset(&diprg, 0, sizeof(diprg));
	diprg.diph.dwSize = sizeof(diprg);
	diprg.diph.dwHeaderSize = sizeof(diprg.diph);
	diprg.diph.dwHow = DIPH_BYOFFSET;
#ifdef USE_DEVICETYPE
	diprg.diph.dwObj = dwInstanceOfs[DIDFT_GETINSTANCE(lpdidoi->dwType)];
#else
	diprg.diph.dwObj = lpdidoi->dwOfs;
#endif
	diprg.lMin = -0x10000;
	diprg.lMax = 0x10000;

	joydevice->lpdid7->SetProperty(DIPROP_RANGE, &diprg.diph);

	// Set deadzone to 1% except on the Z axis (usually a throttle)
#ifdef USE_DEVICETYPE
	// Check both lpdidoi->dwOfs and lpdidoi->dwType to catch re-assigned sliders
	if (lpdidoi->dwOfs != DIJOFS_Z && dwInstanceOfs[DIDFT_GETINSTANCE(lpdidoi->dwType)] != DIJOFS_Z) {
#else
	if (lpdidoi->dwOfs != DIJOFS_Z) {
#endif
		DIPROPDWORD dipdw;
		memset(&dipdw, 0, sizeof(dipdw));
		dipdw.diph.dwSize = sizeof(dipdw);
		dipdw.diph.dwHeaderSize = sizeof(dipdw.diph);
		dipdw.diph.dwHow = DIPH_BYOFFSET;
#ifdef USE_DEVICETYPE
		dipdw.diph.dwObj = dwInstanceOfs[DIDFT_GETINSTANCE(lpdidoi->dwType)];
#else
		dipdw.diph.dwObj = lpdidoi->dwOfs;
#endif
		dipdw.dwData = 10000 / 100;

		joydevice->lpdid7->SetProperty(DIPROP_DEADZONE, &dipdw.diph);
	}

	return DIENUM_CONTINUE;

#undef USE_DEVICETYPE
}

// Set up a single joystick
static int DinpJoystickInitSingle(int i)
{
	struct DinpJoyProperties* Joystick = &DinpJoyProperties[i];
	if (Joystick->lpdid7 == NULL) {
		return 1;
	}

	if (FAILED(Joystick->lpdid7->SetDataFormat(&c_dfDIJoystick))) {
		return 1;
	}

	DIDEVCAPS didcl;
	memset(&didcl, 0, sizeof(didcl));
	didcl.dwSize = sizeof(DIDEVCAPS);
	if (FAILED(Joystick->lpdid7->GetCapabilities(&didcl))) {
		return 1;
	}

	Joystick->dwAxes = didcl.dwAxes;
	Joystick->dwPOVs = didcl.dwPOVs;
	Joystick->dwButtons = didcl.dwButtons;

#if defined LIST_DEVICES && defined FBA_DEBUG
	DIDEVICEINSTANCE didi;
	memset(&didi, 0, sizeof(didi));
	didi.dwSize = sizeof(didi);
	Joystick->lpdid7->GetDeviceInfo(&didi);

	dprintf(_T("  * %s\n"), didi.tszProductName);
	dprintf(_T("    %s; %i axes, %i POV hats, %i buttons\n"), GetDeviceSubtype(didcl.dwDevType), didcl.dwAxes, didcl.dwPOVs, didcl.dwButtons);
#endif

#if defined LIST_DEVICES && defined FBA_DEBUG
	Joystick->lpdid7->EnumObjects(DinpJoyEnumDeviceObjectsCallback, &DinpJoyProperties[i], DIDFT_ALL);
#else
	Joystick->lpdid7->EnumObjects(DinpJoyEnumDeviceObjectsCallback, &DinpJoyProperties[i], DIDFT_AXIS);
#endif

	Joystick->lpdid7->SetCooperativeLevel(hDinpWnd, DISCL_EXCLUSIVE | DISCL_BACKGROUND);
	Joystick->lpdid7->Acquire();

	return 0;
}

// Callback that evaluates each joystick DirectInput device
static BOOL CALLBACK DinpJoyEnumDevicesProc(LPCDIDEVICEINSTANCE lpdidi, LPVOID /*pvRef*/)
{
	if (DinpJoyCount >= MAX_JOYSTICK) {				// Already have the maximum number of joysticks
		return DIENUM_STOP;
	}
	if (lpdidi == NULL) {
		return DIENUM_CONTINUE;
	}

	// Create the DirectInput interface
	IDirectInputDevice* lpdid = NULL;
	if (FAILED(pDI->CreateDevice(lpdidi->guidInstance, &lpdid, NULL))) {
		return DIENUM_CONTINUE;
	}

	// Get the DirectInputDevice7 interface so we can use the Poll method
	IDirectInputDevice7* lpdid7 = NULL;
	if (SUCCEEDED(lpdid->QueryInterface(IID_IDirectInputDevice7, (void**)&lpdid7))) {

		// We now have a IDirectInputDevice7 interface

		memset(&DinpJoyProperties[DinpJoyCount], 0, sizeof(DinpJoyProperties[DinpJoyCount]));

		DinpJoyProperties[DinpJoyCount].lpdid7 = lpdid7;

		if (DinpJoystickInitSingle(DinpJoyCount) == 0) {
			DinpJoyCount++;
		}
	}
	RELEASE(lpdid);

	return DIENUM_CONTINUE;
}

// Get a DI interface to each joystick
static int DinpJoystickInitMaster()
{
	// Enumerate and set up the joysticks connected to the system
	if (FAILED(pDI->EnumDevices(DIDEVTYPE_JOYSTICK, DinpJoyEnumDevicesProc, NULL, DIEDFL_ATTACHEDONLY))) {
		return 1;
	}
	return 0;
}

// ----------------------------------------------------------------------------

// Callback that evaluates and sets up each mouse axis
static BOOL CALLBACK DinpMouseEnumDeviceObjectsCallback(LPCDIDEVICEOBJECTINSTANCE lpdidoi, LPVOID pvRef)
{
#if defined LIST_DEVICES && defined LIST_CONTROLS && defined FBA_DEBUG
	dprintf(_T("    axis %i: "), DIDFT_GETINSTANCE(lpdidoi->dwType));
	dprintf(_T("%s%s\n"), lpdidoi->tszName, (lpdidoi->dwType & DIDFT_FFACTUATOR) ? _T(" (force)") : _T(""));
#endif

	// Store the flags for later use
	((struct DinpMouseProperties*)pvRef)->dwAxisType[DIDFT_GETINSTANCE(lpdidoi->dwType)] = lpdidoi->dwType;

	return DIENUM_CONTINUE;
}

// Set up a single mouse
static int DinpMouseInitSingle(int i)
{
	struct DinpMouseProperties* Mouse = &DinpMouseProperties[i];
	if (Mouse->lpdid7 == NULL) {
		return 1;
	}

	if (FAILED(Mouse->lpdid7->SetDataFormat(&c_dfDIMouse2))) {
		return 1;
	}

	DIDEVCAPS didcl;
	memset(&didcl, 0, sizeof(didcl));
	didcl.dwSize = sizeof(didcl);
	Mouse->lpdid7->GetCapabilities(&didcl);

	Mouse->dwAxes = didcl.dwAxes;
	Mouse->dwButtons = didcl.dwButtons;

#if defined LIST_DEVICES && defined FBA_DEBUG
	DIDEVICEINSTANCE didi;
	memset(&didi, 0, sizeof(didi));
	didi.dwSize = sizeof(didi);
	Mouse->lpdid7->GetDeviceInfo(&didi);

	dprintf(_T("  * %s%s\n"), (i == 0) ? _T("System mouse: ") : _T(""), didi.tszProductName);
	dprintf(_T("    %s; %i axes, %i buttons\n"), GetDeviceSubtype(didcl.dwDevType), didcl.dwAxes, didcl.dwButtons);
#endif

	Mouse->lpdid7->EnumObjects(DinpMouseEnumDeviceObjectsCallback, &DinpMouseProperties[i], DIDFT_AXIS);

	Mouse->lpdid7->SetCooperativeLevel(hDinpWnd, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);
	Mouse->lpdid7->Acquire();

	return 0;
}

// Callback that evaluates each mouse DirectInput device
static BOOL CALLBACK DinpMouseEnumDevicesProc(LPCDIDEVICEINSTANCE lpdidi, LPVOID /*pvRef*/)
{
	if (DinpMouseCount >= MAX_MOUSE) {			// Already have the maximum number of joysticks
		return DIENUM_STOP;
	}
	if (lpdidi == NULL) {
		return DIENUM_CONTINUE;
	}

	// Create the DirectInput1 interface
	IDirectInputDevice* lpdid = NULL;
	if (FAILED(pDI->CreateDevice(lpdidi->guidInstance, &lpdid, NULL))) {
		return DIENUM_CONTINUE;
	}

	// Get the DirectInputDevice7 interface so we can use the Poll method
	IDirectInputDevice7* lpdid7 = NULL;
	if (SUCCEEDED(lpdid->QueryInterface(IID_IDirectInputDevice7, (void**)&lpdid7))) {

		// We now have a IDirectInputDevice7 interface

		memset(&DinpMouseProperties[DinpMouseCount], 0, sizeof(DinpMouseProperties[DinpMouseCount]));

		DinpMouseProperties[DinpMouseCount].lpdid7 = lpdid7;

		if (DinpMouseInitSingle(DinpMouseCount) == 0) {
			DinpMouseCount++;
		}
	}
	RELEASE(lpdid);

	return DIENUM_CONTINUE;
}

// Get a DI interface to all mice
static int DinpMouseInitMaster()
{
	// Enumerate and set up the mice connected to the system
	// Note that under Win2K/WinXP only one mouse device will be enumerated
	if (FAILED(pDI->EnumDevices(DIDEVTYPE_MOUSE, DinpMouseEnumDevicesProc, NULL, DIEDFL_ATTACHEDONLY))) {
		return 1;
	}
	return 0;
}

// ----------------------------------------------------------------------------

int DinpSetCooperativeLevel(bool bExclusive, bool bForeGround)
{
	if (DinpKeyboardProperties[0].lpdid) {
		DinpKeyboardProperties[0].lpdid->Unacquire();
		if (bForeGround) {
			DinpKeyboardProperties[0].lpdid->SetCooperativeLevel(hDinpWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND | (nVidFullscreen ? DISCL_NOWINKEY : 0));
		} else {
			DinpKeyboardProperties[0].lpdid->SetCooperativeLevel(hDinpWnd, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND | (nVidFullscreen ? DISCL_NOWINKEY : 0));
		}
		DinpKeyboardProperties[0].lpdid->Acquire();
	}

	// Mouse 0 is the system mouse
	if (DinpMouseProperties[0].lpdid7) {
		DinpMouseProperties[0].lpdid7->Unacquire();
		if (bExclusive) {
			POINT point = { 0, 0 };
			RECT rect;

			// Ensure the cursor is centered on the input window
			ClientToScreen(hDinpWnd, &point);
			GetClientRect(hDinpWnd, &rect);
			SetCursorPos(point.x + rect.right / 2, point.y + rect.bottom / 2);

			SetForegroundWindow(hDinpWnd);
			SetWindowPos(hDinpWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);

			DinpMouseProperties[0].lpdid7->SetCooperativeLevel(hDinpWnd, DISCL_EXCLUSIVE    | DISCL_FOREGROUND);
		} else {
			DinpMouseProperties[0].lpdid7->SetCooperativeLevel(hDinpWnd, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);
		}
		DinpMouseProperties[0].lpdid7->Acquire();
	}

	// Windows/DirectInput don't always hide the cursor for us
	if (bDrvOkay && (bExclusive || nVidFullscreen)) {
		while (ShowCursor(FALSE) >= 0) {}
	} else {
		while (ShowCursor(TRUE)  <  0) {}
	}

	return 0;
}

// ----------------------------------------------------------------------------

int DinpExit()
{
	// Release the keyboard interface
	RELEASE(DinpKeyboardProperties[0].lpdid);
	DinpKeyboardCount = 0;

	// Release the joystick interfaces
	for (int i = 0; i < MAX_JOYSTICK; i++) {
		RELEASE(DinpJoyProperties[i].lpdid7);
	}
	DinpJoyCount = 0;

	// Release the mouse interface
	for (int i = 0; i < MAX_MOUSE; i++) {
		RELEASE(DinpMouseProperties[i].lpdid7);
	}
	DinpMouseCount = 0;

	// Release the DirectInput interface
	RELEASE(pDI);

	return 0;
}

int DinpInit()
{
	hDinpWnd = hScrnWnd;

	DinpExit();

	memset(&DinpKeyboardProperties, 0, sizeof(DinpKeyboardProperties));
	memset(&DinpJoyProperties, 0, sizeof(DinpJoyProperties));
	memset(&DinpMouseProperties, 0, sizeof(DinpMouseProperties));

	if (FAILED(DirectInputCreateEx(hAppInst, DIRECTINPUT_VERSION, IID_IDirectInput7, (void**)&pDI, NULL))) {
		return 1;
	}

#if defined LIST_DEVICES && defined FBA_DEBUG
	dprintf(_T("*** Enumerating input devices\n"));
#endif

	// Set up the keyboard
	DinpKeyboardInitMaster();

	// Set up the mouse
	DinpMouseInitMaster();

	// Set up the joysticks
	DinpJoystickInitMaster();

	return 0;
}

// ----------------------------------------------------------------------------

// Call before checking for Input in a frame
int DinpStart()
{
	// No joysticks have been read for this frame
	for (int i = 0; i < DinpJoyCount; i++) {
		DinpJoyProperties[i].bReadStatus = 0;
	}

	// Mouse not read this frame
	for (int i = 0; i < DinpMouseCount; i++) {
		DinpMouseProperties[i].bReadStatus = 0;
	}

	// Keyboard not read this frame
	DinpKeyboardProperties[0].bReadStatus = 0;

	return 0;
}

// ----------------------------------------------------------------------------

// Read the keyboard
static int ReadKeyboard(struct DinpKeyboardProperties* Keyboard)
{
	HRESULT nRet = DI_OK;

	if (Keyboard->bReadStatus) {					// Already read this frame - ready to go
		return 0;
	}
	if (Keyboard->lpdid == NULL) {
		return 1;
	}

	// Read keyboard, reacquiring if neccesary
	for (int j = 0; j < 2; j++) {
		nRet = Keyboard->lpdid->GetDeviceState(sizeof(Keyboard->state), (void*)Keyboard->state);
		if (nRet != DIERR_INPUTLOST && nRet != DIERR_NOTACQUIRED) {
			break;
		}
		Keyboard->lpdid->Acquire();					// If DIERR_INPUTLOST, reacquire and try again
	}
	if (FAILED(nRet)) {								// Failed to read the keyboard
		return 1;
	}

	// The keyboard has been successfully Read this frame
	Keyboard->bReadStatus = 1;

	return 0;
}

// ----------------------------------------------------------------------------

// Read one of the joysticks
static int ReadJoystick(struct DinpJoyProperties* Joystick)
{
	HRESULT nRet = DI_OK;

	if (Joystick->bReadStatus == 1) {				// Already read this frame - ready to go
		return 0;
	}
	if (Joystick->bReadStatus == 2) {				// Error reading joystick
		return 1;
	}

	Joystick->bReadStatus = 2;						// Mark joystick as trying to read

	// Poll + read joystick, reacquiring if neccesary
	for (int j = 0; j < 2; j++) {
		Joystick->lpdid7->Poll();

		nRet = Joystick->lpdid7->GetDeviceState(sizeof(Joystick->dijs), &Joystick->dijs);
		if (nRet != DIERR_INPUTLOST && nRet != DIERR_NOTACQUIRED) {
			break;
		}
		Joystick->lpdid7->Acquire();				// If DIERR_INPUTLOST, reacquire and try again
	}
	if (FAILED(nRet)) {
		return 1;
	}

	// This joystick had been successfully Read this frame
	Joystick->bReadStatus = 1;

	return 0;
}

// Check a subcode (the 40xx bit in 4001, 4102 etc) for a joystick input code
static int JoystickState(struct DinpJoyProperties* Joystick, unsigned int nSubCode)
{
	if (nSubCode < 0x10) {										// Joystick directions
		// Some drivers don't assign unused axes a value of 0
		if ((Joystick->dwAxisType[nSubCode >> 1] & DIDFT_AXIS) == 0) {
			return 0;
		}

		switch (nSubCode) {
			case 0x00: return Joystick->dijs.lX < -DEADZONE;	// Left
			case 0x01: return Joystick->dijs.lX > DEADZONE;		// Right
			case 0x02: return Joystick->dijs.lY < -DEADZONE;	// Up
			case 0x03: return Joystick->dijs.lY > DEADZONE;		// Down
			case 0x04: return Joystick->dijs.lZ < -DEADZONE;
			case 0x05: return Joystick->dijs.lZ > DEADZONE;
			case 0x06: return Joystick->dijs.lRx < -DEADZONE;
			case 0x07: return Joystick->dijs.lRx > DEADZONE;
			case 0x08: return Joystick->dijs.lRy < -DEADZONE;
			case 0x09: return Joystick->dijs.lRy > DEADZONE;
			case 0x0A: return Joystick->dijs.lRz < -DEADZONE;
			case 0x0B: return Joystick->dijs.lRz > DEADZONE;
			case 0x0C: return Joystick->dijs.rglSlider[0] < -DEADZONE;
			case 0x0D: return Joystick->dijs.rglSlider[0] > DEADZONE;
			case 0x0E: return Joystick->dijs.rglSlider[1] < -DEADZONE;
			case 0x0F: return Joystick->dijs.rglSlider[1] > DEADZONE;
		}
	}

	if (nSubCode < 0x10 + (Joystick->dwPOVs << 2)) {			// POV hat controls
		DWORD dwHatDirection = Joystick->dijs.rgdwPOV[(nSubCode & 0x0F) >> 2];
		if ((LOWORD(dwHatDirection) != 0xFFFF)) {
			switch (nSubCode & 3) {
				case 0:											// Left
					return dwHatDirection >= 22500 && dwHatDirection <= 31500;
				case 1:											// Right
					return dwHatDirection >=  4500 && dwHatDirection <= 13500;
				case 2:											// Up
					return dwHatDirection >= 31500 || dwHatDirection <=  4500;
				case 3:											// Down
					return dwHatDirection >= 13500 && dwHatDirection <= 22500;
			}
		}
		return 0;
	}
	if (nSubCode < 0x80) {										// Undefined
		return 0;
	}
	if (nSubCode < 0x80 + Joystick->dwButtons) {				// Joystick buttons
		return (Joystick->dijs.rgbButtons[nSubCode & 0x7F] & 0x80) ? 1 : 0;
	}

	return 0;
}

// ----------------------------------------------------------------------------

// Read the mouse
static int ReadMouse(struct DinpMouseProperties* Mouse)
{
	HRESULT nRet = DI_OK;

	if (Mouse->bReadStatus) {						// Already read this frame - ready to go
		return 0;
	}
	if (Mouse->lpdid7 == NULL) {
		return 1;
	}

	// Read mouse, reacquiring if neccesary
	for (int j = 0; j < 2; j++) {
		nRet = Mouse->lpdid7->GetDeviceState(sizeof(Mouse->dims), (void*)&Mouse->dims);
		if (nRet != DIERR_INPUTLOST) {
			break;
		}
		Mouse->lpdid7->Acquire();					// If DIERR_INPUTLOST, reacquire and try again
	}
	if (FAILED(nRet)) {								// Failed to read the mouse
		return 1;
	}

	// The mouse has been successfully Read this frame
	Mouse->bReadStatus = 1;

	return 0;
}

// Check a subcode (the 80xx bit in 8001, 8102 etc) for a mouse input code
static int CheckMouseState(struct DinpMouseProperties* Mouse, unsigned int nSubCode)
{
	if (nSubCode < 0x80) {							// Undefined
		return 0;
	}
	if (nSubCode < 0x80 + Mouse->dwButtons) {		// Mouse buttons
		return (Mouse->dims.rgbButtons[nSubCode & 0x7F] & 0x80) ? 1 : 0;
	}

	return 0;
}

// ----------------------------------------------------------------------------

// Get the state (pressed = 1, not pressed = 0) of a particular input code
int DinpState(int nCode)
{
	if (nCode < 0) {
		return 0;
	}

	if (nCode < 0x100) {
		if (ReadKeyboard(&DinpKeyboardProperties[0]) != 0) {	// Check keyboard has been read - return not pressed on error
			return 0;
		}
		return (DinpKeyboardProperties[0].state[nCode] & 0x80) ? 1 : 0;
	}

	if (nCode < 0x4000) {
		return 0;
	}

	if (nCode < 0x8000) {

		// Codes 4000-8000 = Joysticks

		int i = (nCode - 0x4000) >> 8;
		if (i >= DinpJoyCount) {						// This joystick number isn't connected
			return 0;
		}
		if (ReadJoystick(&DinpJoyProperties[i]) != 0) {	// Error polling the joystick
			return 0;
		}

		// Find the joystick state in our array
		return JoystickState(&DinpJoyProperties[i], nCode & 0xFF);
	}

	if (nCode < 0xC000) {

		// Codes 8000-C000 = Mouse

		int i = (nCode - 0x8000) >> 8;
		if (i >= DinpMouseCount) {						// This mouse number isn't connected
			return 0;
		}
		if (ReadMouse(&DinpMouseProperties[i]) != 0) {	// Error Reading the mouse
			return 0;
		}
		return CheckMouseState(&DinpMouseProperties[i], nCode & 0xFF);
	}

	return 0;
}

// Read one joystick axis
int DinpJoyAxis(int i, int nAxis)
{
	struct DinpJoyProperties* Joystick = &DinpJoyProperties[i];

	if (i < 0 || i >= DinpJoyCount) {				// This joystick isn't connected
		return 0;
	}
	if (ReadJoystick(Joystick) != 0) {				// Error polling the joystick
		return 0;
	}

	// Some drivers don't assign unused axes a value of 0
	if ((Joystick->dwAxisType[nAxis & (MAX_JOYAXIS - 1)] & DIDFT_AXIS) == 0) {
		return 0;
	}

	switch (nAxis) {
		case 0:
			return Joystick->dijs.lX;
		case 1:
			return Joystick->dijs.lY;
		case 2:
			return Joystick->dijs.lZ;
		case 3:
			return Joystick->dijs.lRx;
		case 4:
			return Joystick->dijs.lRy;
		case 5:
			return Joystick->dijs.lRz;
		case 6:
			return Joystick->dijs.rglSlider[0];
		case 7:
			return Joystick->dijs.rglSlider[1];
	}

	return 0;
}

// Read one mouse axis
int DinpMouseAxis(int i, int nAxis)
{
	struct DinpMouseProperties* Mouse = &DinpMouseProperties[i];

	if (i < 0 || i >= DinpMouseCount) {				// This mouse isn't connected
		return 0;
	}
	if (ReadMouse(Mouse) != 0) {					// Error polling the mouse
		return 0;
	}

	// Some drivers don't assign unused axes a value of 0
	if ((Mouse->dwAxisType[nAxis & (MAX_MOUSEAXIS - 1)] & DIDFT_AXIS) == 0) {
		return 0;
	}
	switch (nAxis) {
		case 0:
			return Mouse->dims.lX;
		case 1:
			return Mouse->dims.lY;
		case 2:
			return Mouse->dims.lZ;
	}

	return 0;
}

// This function finds which key is pressed
// To ensure analog joystick axes are handled correctly, call with CreateBaseline = true the 1st time
int DinpFind(bool CreateBaseline)
{
	int nRetVal = -1;															// assume nothing pressed

	// check if any keyboard keys are pressed
	if (ReadKeyboard(&DinpKeyboardProperties[0]) == 0) {
		for (int i = 0; i < 0x100; i++) {
			if (DinpKeyboardProperties[0].state[i] & 0x80) {
				nRetVal = i;
				goto End;
			}
		}
	}

	// Now check all the connected joysticks
	for (int i = 0; i < DinpJoyCount; i++) {
		struct DinpJoyProperties* Joystick = &DinpJoyProperties[i];
		if (ReadJoystick(Joystick) != 0) {										// There was an error polling the joystick
			continue;
		}
		for (unsigned int j = 0; j < 0x10; j++) {								// Axes
			int nDelta = Joystick->dwAxisBaseline[j >> 1] - DinpJoyAxis(i, (j >> 1));
			if (nDelta < -0x4000 || nDelta > 0x4000) {
				if (JoystickState(Joystick, j)) {
					nRetVal = 0x4000 | (i << 8) | j;
					goto End;
				}
			}
		}

		for (unsigned int j = 0x10; j < 0x10 + (Joystick->dwPOVs << 2); j++) {	// POV hats
			if (JoystickState(Joystick, j)) {
				nRetVal = 0x4000 | (i << 8) | j;
				goto End;
			}
		}

		for (unsigned int j = 0x80; j < 0x80 + Joystick->dwButtons; j++) {		// Buttons
			if (JoystickState(Joystick, j)) {
				nRetVal = 0x4000 | (i << 8) | j;
				goto End;
			}
		}
	}

	// Now check all the connected mice
	for (int i = 0; i < DinpMouseCount; i++) {
		struct DinpMouseProperties* Mouse = &DinpMouseProperties[i];
		if (ReadMouse(Mouse) == 0) {
			int nMaxDelta = 0, nMaxAxis = 0;

			for (unsigned int j = 0x80; j < 0x80 + Mouse->dwButtons; j++) {
				if (CheckMouseState(Mouse, j)) {
					nRetVal = 0x8000 | (i << 8) | j;
					goto End;
				}
			}

			for (unsigned int j = 0; j < MAX_MOUSEAXIS; j++) {
				int nDelta = DinpMouseAxis(i, j);
				if (abs(nMaxDelta) < abs(nDelta)) {
					nMaxDelta = nDelta;
					nMaxAxis = j;
				}
			}
			if (nMaxDelta < 0) {
				return 0x8000 | (i << 8) | (nMaxAxis << 1) | 0;
			}
			if (nMaxDelta > 0) {
				return 0x8000 | (i << 8) | (nMaxAxis << 1) | 1;
			}
		}
	}

End:

	if (CreateBaseline) {
		for (int i = 0; i < DinpJoyCount; i++) {
			for (int j = 0; j < 8; j++) {
				DinpJoyProperties[i].dwAxisBaseline[j] = DinpJoyAxis(i, j);
			}
		}
	}

	return nRetVal;
}

int DinpGetControlName(int nCode, TCHAR* pszDeviceName, TCHAR* pszControlName)
{
	IDirectInputDevice7* lpdid7 = NULL;
	DWORD* pdwAxisType = NULL;
	DWORD dwMouseAxes = 0, dwPOVs = 0, dwButtons = 0;
	DWORD dwObj = 0;
	int nDeviceType = nCode & 0xC000;

	if (pszDeviceName) {
		pszDeviceName[0] = _T('\0');
	}
	if (pszControlName) {
		pszControlName[0] = _T('\0');
	}

	switch (nDeviceType) {
		case 0x0000: {
			int i = (nCode >> 8) & 0x3F;

			if (i >= DinpKeyboardCount) {			// This keyboard isn't connected
				return 0;
			}
			if (DinpKeyboardProperties[i].lpdid == NULL) {
				return 1;
			}

			if (pszDeviceName) {
				DIDEVICEINSTANCE didi;

				memset(&didi, 0, sizeof(didi));
				didi.dwSize = sizeof(didi);

				DinpKeyboardProperties[i].lpdid->GetDeviceInfo(&didi);
				_sntprintf(pszDeviceName, MAX_PATH, _T("System keyboard: %s"), didi.tszInstanceName);
			}
			if (pszControlName) {
				DIDEVICEOBJECTINSTANCE didoi;

				memset(&didoi, 0, sizeof(didoi));
				didoi.dwSize = sizeof(didoi);

				if (SUCCEEDED(DinpKeyboardProperties[i].lpdid->GetObjectInfo(&didoi, DIDFT_MAKEINSTANCE(nCode & 0xFF) | DIDFT_PSHBUTTON, DIPH_BYID))) {
					_tcsncpy(pszControlName, didoi.tszName, MAX_PATH);
				}
			}

			return 0;
		}
		case 0x4000: {
			int i = (nCode >> 8) & 0x3F;

			if (i >= DinpJoyCount) {				// This joystick isn't connected
				return 0;
			}
			lpdid7 = DinpJoyProperties[i].lpdid7;
			pdwAxisType = DinpJoyProperties[i].dwAxisType;
			dwPOVs = DinpJoyProperties[i].dwPOVs;
			dwButtons = DinpJoyProperties[i].dwButtons;
			break;
		}
		case 0x8000: {
			int i = (nCode >> 8) & 0x3F;

			if (i >= DinpMouseCount) {				// This mouse isn't connected
				return 0;
			}
			lpdid7 = DinpMouseProperties[i].lpdid7;
			pdwAxisType = DinpMouseProperties[i].dwAxisType;
			dwMouseAxes = DinpMouseProperties[i].dwAxes;
			dwButtons = DinpMouseProperties[i].dwButtons;
			break;
		}
	}

	if (lpdid7 == NULL) {
		return 1;
	}

	if (pszDeviceName) {
		DIDEVICEINSTANCE didi;

		memset(&didi, 0, sizeof(didi));
		didi.dwSize = sizeof(didi);

		if (SUCCEEDED(lpdid7->GetDeviceInfo(&didi))) {

			// Special treatment for the system mouse
			if ((nCode & 0xFF00) == 0x8000) {
				_sntprintf(pszDeviceName, MAX_PATH, _T("System mouse: %s"), didi.tszInstanceName);
			} else {
				_tcsncpy(pszDeviceName, didi.tszInstanceName, MAX_PATH);
			}
		}
	}
	if (pszControlName) {
		DIDEVICEOBJECTINSTANCE didoi;

		unsigned int nSubCode = nCode & 0xFF;

		if ((nSubCode >> 1) < ((nDeviceType == 0x4000) ? MAX_JOYAXIS : MAX_MOUSEAXIS)) {
			if ((pdwAxisType[nSubCode >> 1] & DIDFT_AXIS) != 0) {
				dwObj = pdwAxisType[nSubCode >> 1];
			}
		}
		if (nSubCode >= 0x10 && nSubCode < 0x10 + (dwPOVs << 2)) {
			dwObj = DIDFT_POV | DIDFT_MAKEINSTANCE((nSubCode & 0x0F) >> 2);
		}
		if (nSubCode >= 0x80 && nSubCode < 0x80 + dwButtons) {
			dwObj = DIDFT_PSHBUTTON | DIDFT_MAKEINSTANCE((nSubCode & 0x7F) + dwMouseAxes);
		}

		memset(&didoi, 0, sizeof(didoi));
		didoi.dwSize = sizeof(didoi);

		if (SUCCEEDED(lpdid7->GetObjectInfo(&didoi, dwObj, DIPH_BYID))) {
			_tcsncpy(pszControlName, didoi.tszName, MAX_PATH);
		}
	}

	return 0;
}

// ----------------------------------------------------------------------------

struct InputInOut InputInOutDInput = { DinpInit, DinpExit, DinpSetCooperativeLevel, DinpStart, DinpState, DinpJoyAxis, DinpMouseAxis, DinpFind, DinpGetControlName, NULL };
