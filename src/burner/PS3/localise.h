#ifndef _LOCALISE_H_
#define _LOCALISE_H_

extern bool bLanguageActive;
extern TCHAR szLanguage[MAX_PATH];
 
void FBALocaliseExit();
int FBALocaliseInit(TCHAR* language);
int FBALocaliseLoad();

HBITMAP FBALoadBitmap(int id);
 
int FBALoadString(int id, char * buffer, int maxsize);
TCHAR* FBALoadStringEx(int id, bool translate = true);

#define MAKEINTRESOURCE(x) x

#define IDS_ERR_ERROR			(0)
#define IDS_ERR_WARNING			(1)
#define IDS_ERR_INFORMATION		(2)
//#define IDS_ERR_UNKNOWN			(3)

#define IDS_ERR_ERROR_TXT			"Error"
#define IDS_ERR_WARNING_TXT			"Warning"
#define IDS_ERR_INFORMATION_TXT		"Information"
#define IDS_ERR_UNKNOWN_TXT			"Unknown"

#define IDS_ERR_LOAD_OK			"The ROMset is OK."
#define	IDS_ERR_LOAD_PROBLEM	"The ROMset exhibits the following problems:"
#define	IDS_ERR_LOAD_NODATA		"No data was found, the game will not run!"
#define	IDS_ERR_LOAD_ESS_MISS	"essential data is missing, the game will not run!"
#define	IDS_ERR_LOAD_ESS_BAD	"essential data is invalid, the game might not run!"
#define	IDS_ERR_LOAD_DATA_MISS	"data is missing."
#define	IDS_ERR_LOAD_DATA_BAD	"data is invalid."
#define	IDS_ERR_LOAD_FOUND		"%.32hs was found (%.512s).\n"
#define	IDS_ERR_LOAD_NOTFOUND	"%.32hs was not found.\n"
#define	IDS_ERR_LOAD_INVALID	"The following ROMs are invalid:\n"
#define	IDS_ERR_LOAD_CRC		"%.32hs has a CRC of %.8X (correct is %.8X).\n"
#define	IDS_ERR_LOAD_SMALL		"%.32hs is %dk which is too small (correct is %dkB).\n"
#define	IDS_ERR_LOAD_LARGE		"%.32hs is %dk which is too large (correct is %dkB).\n"
#define	IDS_ERR_LOAD_DET_ESS	"essential "
#define	IDS_ERR_LOAD_DET_PRG	"program "
#define	IDS_ERR_LOAD_DET_BIOS	"BIOS "
#define	IDS_ERR_LOAD_DET_GRA	"graphics "
#define	IDS_ERR_LOAD_DET_SND	"sound "
#define	IDS_ERR_LOAD_DET_ROM	"ROM "
#define	IDS_ERR_LOAD_DISK		"Error reading %.32hs from %.512s"
#define	IDS_ERR_LOAD_DISK_CRC	"CRC mismatch reading %.32hs from %.512s"
#define	IDS_ERR_LOAD_REQUEST	"Error loading %.32hs, requested by %.128s.\nThe emulation will likely suffer problems."

#endif // _LOCALISE_H_
