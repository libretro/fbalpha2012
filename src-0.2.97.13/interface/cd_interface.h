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
#ifndef __LIBSNES__
	extern CDEmuStatusValue CDEmuStatus;

	return CDEmuStatus;
#else
   return idle;
#endif
}

static inline void CDEmuStartRead()
{
#ifndef __LIBSNES__
	extern CDEmuStatusValue CDEmuStatus;

	CDEmuStatus = seeking;
#endif
}

static inline void CDEmuPause()
{
#ifndef __LIBSNES__
	extern CDEmuStatusValue CDEmuStatus;

	CDEmuStatus = paused;
#endif
}

// ----------------------------------------------------------------------------
// Support functions

void CDEmuPrintCDName();

#endif /*CD_INTERFACE_H_*/
