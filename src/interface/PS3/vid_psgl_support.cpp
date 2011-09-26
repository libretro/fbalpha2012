// Support functions for blitters that use PSGL
#include "burner.h"
#include "vid_psgl_support.h"

// ---------------------------------------------------------------------------
// General


void VidSExit()
{
	 
}
 
// ---------------------------------------------------------------------------
// Fullscreen mode support routines

int VidSScoreDisplayMode(VidSDisplayScoreInfo* pScoreInfo)
{
	 

	return 0;
}

int VidSInitScoreInfo(VidSDisplayScoreInfo* pScoreInfo)
{
 
	return 0;
}
 
void VidSRestoreScreenMode()
{
	 
}

// Enter fullscreen mode, select optimal full-screen resolution
int VidSEnterFullscreenMode(int nZoom, int nDepth)
{
	 

	return 0;
}
 

static void VidSExitTinyMsg()
{
 
}

static void VidSExitShortMsg()
{
 
}

void VidSExitChat()
{
	 
}

static void VidSExitEdit()
{
 
}

void VidSExitOSD()
{
 
}

static int VidSInitTinyMsg(int nFlags)
{
	 

	return 0;
}

static int VidSInitShortMsg(int nFlags)
{
	 
	 
	return 0;
}

static int VidSInitChat(int nFlags)
{
 
	return 0;
}

static int VidSInitEdit(int nFlags)
{
 

	return 0;
}

int VidSInitOSD(int nFlags)
{ 
	return 0;
}

int VidSRestoreOSD()
{
 

	return 0;
}

 

static int VidSDrawChat(RECT* dest)
{
 
	return 0;
}
 

int VidSNewTinyMsg(const TCHAR* pText, int nRGB, int nDuration, int nPriority)	// int nRGB = 0, int nDuration = 0, int nPriority = 5
{
	 
	return 0;
 }

int VidSNewShortMsg(const TCHAR* pText, int nRGB, int nDuration, int nPriority)	// int nRGB = 0, int nDuration = 0, int nPriority = 5
{
 
	return 0;
}

void VidSKillShortMsg()
{
	 
}

void VidSKillTinyMsg()
{
	 
}

void VidSKillOSDMsg()
{
	 
}

void VidSKillTinyOSDMsg()
{
	 
}

int VidSAddChatMsg(const TCHAR* pID, int nIDRGB, const TCHAR* pMain, int nMainRGB)
{
 

	return 0;
}
