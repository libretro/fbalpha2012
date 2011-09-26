// icon load module, added by regret
// load ico from file or memory

/* changelog:
 update 2: chang to save icons in imagelist
 update 1: create
*/

#include "burner.h"
#include "iconload.h"
#include <shellapi.h>
#include "../File_Extractor/fex/fex.h"

// load icon
static map<string, int> iconMap;
static HIMAGELIST himlIconsLarge = NULL; // handle to image list
static HIMAGELIST himlIconsSmall = NULL;
static HICON hLoadIcon = NULL;

typedef struct
{
	BYTE    bWidth;               /* Width of the image */
	BYTE    bHeight;              /* Height of the image (times 2) */
	BYTE    bColorCount;          /* Number of colors in image (0 if >=8bpp) */
	BYTE    bReserved;            /* Reserved */
	WORD    wPlanes;              /* Color Planes */
	WORD    wBitCount;            /* Bits per pixel */
	DWORD   dwBytesInRes;         /* how many bytes in this resource? */
	DWORD   dwImageOffset;        /* where in the file is this image */
} ICONDIRENTRY, *LPICONDIRENTRY;

typedef struct
{
	UINT            Width, Height, Colors; /* Width, Height and bpp */
	LPBYTE          lpBits;                /* ptr to DIB bits */
	DWORD           dwNumBytes;            /* how many bytes? */
	LPBITMAPINFO    lpbi;                  /* ptr to header */
	LPBYTE          lpXOR;                 /* ptr to XOR image bits */
	LPBYTE          lpAND;                 /* ptr to AND image bits */
} ICONIMAGE, *LPICONIMAGE;

HICON loadIconFromFile(HINSTANCE instance, const TCHAR* filename, UINT index)
{
	if (!instance || !filename) {
		return NULL;
	}
	return ExtractIcon(instance, filename, index);
}

HICON loadIconFromBuffer(const LPBYTE buf, unsigned int size)
{
	/* Is there a WORD? */
	if (!buf || size < sizeof(WORD)) {
		return NULL;
	}

	UINT bufferIndex = 0;

	/* Was it 'reserved' ?	 (ie 0) */
	if ((WORD)(buf[bufferIndex]) != 0) {
		return NULL;
	}

	bufferIndex += sizeof(WORD);

	/* Is there a WORD? */
	if (size - bufferIndex < sizeof(WORD)) {
		return NULL;
	}

	/* Was it type 1? */
	if ((WORD)(buf[bufferIndex]) != 1) {
		return NULL;
	}

	bufferIndex += sizeof(WORD);

	/* Is there a WORD? */
	if (size - bufferIndex < sizeof(WORD)) {
		return NULL;
	}

	/* Then that's the number of images in the ICO file */
	UINT imageNumber = (WORD)(buf[bufferIndex]);

	/* Is there at least one icon in the file? */
	if (imageNumber < 1) {
		return NULL;
	}

	bufferIndex += sizeof(WORD);

	/* Is there enough space for the icon directory entries? */
	if ((bufferIndex + imageNumber * sizeof(ICONDIRENTRY)) > size) {
		return NULL;
	}

	/* Assign icon directory entries from buffer */
	LPICONDIRENTRY lpIDE = (LPICONDIRENTRY)(&buf[bufferIndex]);
	bufferIndex += imageNumber * sizeof(ICONDIRENTRY);

	ICONIMAGE IconImage;
	IconImage.dwNumBytes = lpIDE->dwBytesInRes;

	/* Seek to beginning of this image */
	if (lpIDE->dwImageOffset > size) {
		return NULL;
	}

	bufferIndex = lpIDE->dwImageOffset;

	/* Read it in */
	if ((bufferIndex + lpIDE->dwBytesInRes) > size) {
		return NULL;
	}

	IconImage.lpBits = &buf[bufferIndex];
	bufferIndex += lpIDE->dwBytesInRes;

	HICON hIcon = NULL;

#if 0  // Not sure we want odd sized icons.
	hIcon = CreateIconFromResourceEx(IconImage.lpBits, IconImage.dwNumBytes, TRUE, 0x00030000,
			(*(LPBITMAPINFOHEADER)(IconImage.lpBits)).biWidth, (*(LPBITMAPINFOHEADER)(IconImage.lpBits)).biHeight/2, 0);
#endif

	/* It failed, odds are good we're on NT so try the non-Ex way */
	if (hIcon == NULL) {
		/* We would break on NT if we try with a 16bpp image */
		if (((LPBITMAPINFO)IconImage.lpBits)->bmiHeader.biBitCount != 16) {
			hIcon = CreateIconFromResourceEx(IconImage.lpBits, IconImage.dwNumBytes, TRUE, 0x00030000, 0, 0, LR_DEFAULTSIZE);
		}
	}
	return hIcon;
}

// load icon from archive
static inline bool error(const char* error)
{
	if (error) {
#ifdef _DEBUG
		printf("fex Error: %s\n", error);
#endif
		return true;
	}
	return false;
}

extern bool gameInList(const char* name);

static inline bool getIcon(void* buffer, long len, string name)
{
	if (!buffer || len <= 0) {
		return false;
	}

	// load icon from buffer
	hLoadIcon = loadIconFromBuffer((LPBYTE)buffer, len);
	if (!hLoadIcon) {
		return false;
	}

	// add to imagelist
	iconMap[name] = ImageList_AddIcon(himlIconsLarge, hLoadIcon);
	ImageList_AddIcon(himlIconsSmall, hLoadIcon);
	DestroyIcon(hLoadIcon);
	return true;
}

int loadIconFromArchive(const TCHAR* arc)
{
	File_Extractor* fex_ico = NULL;
	fex_err_t err = fex_open(&fex_ico, WtoA(arc));
	if (error(err)) {
		return 1;
	}

	void* buf = NULL;

	while (!fex_done(fex_ico)) {
		fex_stat(fex_ico);

		long len = fex_size(fex_ico);

		if (buf) {
			free(buf);
		}
		buf = (unsigned char*)malloc(len);
		if (!buf) {
			break;
		}

		string gamename = getBaseNameA(fex_name(fex_ico));
		// check if the icon is needed
		if (gameInList(gamename.c_str())) {
			err = fex_read(fex_ico, buf, len); // extract file
			if (!error(err)) {
				getIcon(buf, len, gamename);
			}
		}

		err = fex_next(fex_ico);
		if (error(err)) {
			// stop loading?
		}
	}

	fex_close(fex_ico);
	if (buf) {
		free(buf);
	}
	return 0;
}

int findIconMap(const char* name)
{
	map<string, int>::iterator it = iconMap.find(name);
	if (it != iconMap.end()) {
		return it->second;
	}
	return -1;
}

void drawIcon(bool large, int id, HDC hdc, int x, int y)
{
	ImageList_Draw(large ? himlIconsLarge : himlIconsSmall, id, hdc, x, y, ILD_NORMAL);
}

int createIconList()
{
	clearIcons();

	// Create a masked image list large enough to hold the icons.
	himlIconsLarge = ImageList_Create(32, 32, ILC_COLORDDB | ILC_MASK, 1, nBurnDrvCount);
	if (!himlIconsLarge) {
		return 1;
	}
	himlIconsSmall = ImageList_Create(16, 16, ILC_COLORDDB | ILC_MASK, 1, nBurnDrvCount);
	if (!himlIconsSmall) {
		return 1;
	}
	return 0;
}

void clearIcons()
{
	ImageList_Destroy(himlIconsLarge);
	ImageList_Destroy(himlIconsSmall);
	himlIconsLarge = NULL;
	himlIconsSmall = NULL;
	iconMap.clear();
}
