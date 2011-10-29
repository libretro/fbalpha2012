#ifndef CD_INTERFACE_H_
#define CD_INTERFACE_H_

// ----------------------------------------------------------------------------
// CD emulation module

enum CDEmuStatusValue { idle = 0, reading, playing, paused, seeking, fastforward, fastreverse };

extern TCHAR CDEmuImage[MAX_PATH];

int CDEmuInit();
int CDEmuExit();
int CDEmuStop();
int CDEmuPlay(unsigned char M, unsigned char S, unsigned char F);
int CDEmuLoadSector(int LBA, char* pBuffer);
unsigned char* CDEmuReadTOC(int track);
unsigned char* CDEmuReadQChannel();
int CDEmuGetSoundBuffer(short* buffer, int samples);

static inline CDEmuStatusValue CDEmuGetStatus()
{
	extern CDEmuStatusValue CDEmuStatus;

	return CDEmuStatus;
}

static inline void CDEmuStartRead()
{
	extern CDEmuStatusValue CDEmuStatus;

	CDEmuStatus = seeking;
}

static inline void CDEmuPause()
{
	extern CDEmuStatusValue CDEmuStatus;

	CDEmuStatus = paused;
}

// ----------------------------------------------------------------------------
// Support functions

void CDEmuPrintCDName();

#endif /*CD_INTERFACE_H_*/
