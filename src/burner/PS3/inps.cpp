// Burner Input Set dialog module
#include "burner.h"

HWND hInpsDlg = NULL; // Handle to the Input Set Dialog
 
unsigned int nInpsInput = 0; // The input number we are redefining
static struct BurnInputInfo bii; // Info about the input
static int nDlgState = 0;
static int nInputCode = -1; // If in state 3, code N was nInputCode
static int nCounter = 0; // Counter of frames since the dialog was made
static struct GameInp* pgi = NULL; // Current GameInp
static struct GameInp OldInp; // Old GameInp
static int bOldPush = 0; // 1 if the push button was pressed last time

static bool bGrabMouse = false;

static bool bOldLeftAltkeyMapped;

static int InpsInit()
{
	return 0;
}

static int InpsExit()
{
	return 0;
}

static int SetInput(int nCode)
{
	return 0;
}

static int InpsPushUpdate()
{
	int nPushState = 0;
	return nPushState;
}

static void InpsUpdateControl(int nCode)
{
}

int InpsUpdate()
{
	return 0;
}
 

int InpsCreate()
{
	return 0;
}
