#ifndef NO_PNG
#include "burner.h"
#include "png.h"

#define SSHOT_NOERROR 0
#define SSHOT_ERROR_BPP_NOTSUPPORTED 1
#define SSHOT_LIBPNG_ERROR 2
#define SSHOT_OTHER_ERROR 3
#define MAX_PREV_NUM 20

static unsigned char* pConvertedImage = NULL;

// ==> save preview image, modified by regret
static void PreviewName(char * szName)
{
	char szPath[MAX_PATH] = "";

	_tcscpy(szPath, getMiscPath(PATH_PREVIEW));
	// get md image path
	if ((BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_SEGA_MEGADRIVE)
		_tcscat(szPath, "MD\\");

	sprintf(szName, "%s%s.png", szPath, BurnDrvGetText(DRV_NAME));
	if (!fileExists(szName))
		return;

	for (int i = 2; i < MAX_PREV_NUM; i++)
	{
		sprintf(szName, "%s%s [%02i].png", szPath, BurnDrvGetText(DRV_NAME), i);
		if (!fileExists(szName))
			return;
	}
}
// <== save preview image

// convert image, added and fixed rotation by regret
unsigned char* ConvertVidImage(unsigned char* src, int bFlipVertical)
{
	unsigned char* pImage = src;
	unsigned char* pTemp = NULL;
	int w, h;

	BurnDrvGetVisibleSize(&w, &h);

	// Convert the image to 32-bit
	if (nVidImageBPP < 4) {
		pTemp = (unsigned char*)malloc(w * h * sizeof(int));

		if (nVidImageBPP == 2) {
			for (int i = 0; i < h * w; i++) {
				unsigned short nColour = ((unsigned short*)pImage)[i];

				// Red
				*(pTemp + i * 4 + 0) = (unsigned char)((nColour & 0x1F) << 3);
				*(pTemp + i * 4 + 0) |= *(pTemp + 4 * i + 0) >> 5;

				if (nVidImageDepth == 15) {
					// Green
					*(pTemp + i * 4 + 1) = (unsigned char)(((nColour >> 5) & 0x1F) << 3);
					*(pTemp + i * 4 + 1) |= *(pTemp + i * 4 + 1) >> 5;
					// Blue
					*(pTemp + i * 4 + 2) = (unsigned char)(((nColour >> 10)& 0x1F) << 3);
					*(pTemp + i * 4 + 2) |= *(pTemp + i * 4 + 2) >> 5;
				}

				if (nVidImageDepth == 16) {
					// Green
					*(pTemp + i * 4 + 1) = (unsigned char)(((nColour >> 5) & 0x3F) << 2);
					*(pTemp + i * 4 + 1) |= *(pTemp + i * 4 + 1) >> 6;
					// Blue
					*(pTemp + i * 4 + 2) = (unsigned char)(((nColour >> 11) & 0x1F) << 3);
					*(pTemp + i * 4 + 2) |= *(pTemp + i * 4 + 2) >> 5;
				}
			}
		} else {
			memset(pTemp, 0, w * h * sizeof(int));
			for (int i = 0; i < h * w; i++) {
				*(pTemp + i * 4 + 0) = *(pImage + i * 3 + 0);
				*(pTemp + i * 4 + 1) = *(pImage + i * 3 + 1);
				*(pTemp + i * 4 + 2) = *(pImage + i * 3 + 2);
			}
		}

		pConvertedImage = pTemp;
		pImage = pConvertedImage;
	}

	// Rotate and flip the image
	if (BurnDrvGetFlags() & BDF_ORIENTATION_VERTICAL)
	{
		unsigned char* pOldTemp = pTemp;
		pTemp = (unsigned char*)malloc(w * h * sizeof(int));

		for (int x = 0; x < h; x++)
		{
			for (int y = 0; y < w; y++)
			{
				if (BurnDrvGetFlags() & BDF_ORIENTATION_FLIPPED)
					((unsigned int*)pTemp)[(w - y - 1) + x * w] = ((unsigned int*)pImage)[x + y * h];
				else
					((unsigned int*)pTemp)[y + (h - x - 1) * w] = ((unsigned int*)pImage)[x + y * h];
			}
		}

		free(pOldTemp);
		pConvertedImage = pTemp;
		pImage = pConvertedImage;
	}
	else if (BurnDrvGetFlags() & BDF_ORIENTATION_FLIPPED)
	{
		unsigned char* pOldTemp = pTemp;
		pTemp = (unsigned char*)malloc(w * h * sizeof(int));

		for (int y = h - 1; y >= 0; y--)
		{
			for (int x = w - 1; x >= 0; x--)
				((unsigned int*)pTemp)[(w - x - 1) + (h - y - 1) * w] = ((unsigned int*)pImage)[x + y * w];
		}

		free(pOldTemp);
		pConvertedImage = pTemp;
		pImage = pConvertedImage;
	}

	if (bFlipVertical)
	{
		unsigned char* pOldTemp = pTemp;
		pTemp = (unsigned char*)malloc(w * h * sizeof(int));

		for (int y = 0; y < h; y++)
		{
			for (int x = 0; x < w; x++)
				((unsigned int*)pTemp)[x + (h - y - 1) * w] = ((unsigned int*)pImage)[x + y * w];
		}

		free(pOldTemp);
		pConvertedImage = pTemp;
		pImage = pConvertedImage;
	}

	return pImage;
}

int MakeScreenShot(bool bScrShot, int Type)  // 0 = Title, 1 = Preview
{

#if defined (_XBOX)	// add ps3 later
	char szAuthor[MAX_PATH]; char szDescription[MAX_PATH]; char szCopyright[MAX_PATH]; char szSoftware[MAX_PATH]; char szSource[MAX_PATH];
	png_text text_ptr[8] = { { 0, 0, 0, 0 }, };
	int num_text = 8;

	time_t currentTime;
	tm* tmTime;
	png_time_struct png_time;

	TCHAR szSShotName[MAX_PATH];
	int w, h;

	// do our PNG construct things
	png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr) {
		return SSHOT_LIBPNG_ERROR;
	}

	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
		return SSHOT_LIBPNG_ERROR;
	}

	if (setjmp(png_jmpbuf(png_ptr))) {
		png_destroy_write_struct(&png_ptr, &info_ptr);
		_tremove(szSShotName);
		return SSHOT_LIBPNG_ERROR;
	}

	if (pVidImage == NULL) {
		return SSHOT_OTHER_ERROR;
	}

	if (nVidImageBPP < 2 || nVidImageBPP > 4) {
		return SSHOT_ERROR_BPP_NOTSUPPORTED;
	}

	BurnDrvGetVisibleSize(&w, &h);

	// Convert the image to 32-bit
	unsigned char* pSShot = ConvertVidImage(pVidImage, 0);

	// Get the time
	time(&currentTime);
	tmTime = localtime(&currentTime);
	png_convert_from_time_t(&png_time, currentTime);

	// create dir if dir doesn't exist
	if (!directoryExists(getMiscPath(PATH_SCREENSHOT))) {
		CreateDirectory(getMiscPath(PATH_SCREENSHOT), NULL);
	}

	if (bScrShot) {
		// construct our filename -> "romname-mm-dd-hms.png"

		switch (Type)
		{
			case 0:
				sprintf(szSShotName, "%s%s.png", getMiscPath(PATH_TITLE), BurnDrvGetTextA(DRV_NAME));
				break;
			case 1:		 		 
				sprintf(szSShotName, "%s%s.png", getMiscPath(PATH_PREVIEW), BurnDrvGetTextA(DRV_NAME));		
				break;
			case 2: // cache:
				sprintf(szSShotName, "cache:\\preview.png");		
				break;
		}

	} else {
		// Make preview
		PreviewName(szSShotName);
	}

	FILE* ff = _tfopen(szSShotName, _T("wb"));
	if (ff == NULL)
	{
		png_destroy_write_struct(&png_ptr, &info_ptr);

		free(pConvertedImage);
		pConvertedImage = NULL;

		return SSHOT_OTHER_ERROR;
	}

	// Fill the PNG text fields
	sprintf(szAuthor, APP_TITLE " v%.20ls", szAppBurnVer);
	sprintf(szDescription, "Screenshot of %s", decorateGameName(nBurnDrvSelect));
	sprintf(szCopyright, "%s %s", BurnDrvGetTextA(DRV_DATE), BurnDrvGetTextA(DRV_MANUFACTURER));
	sprintf(szSoftware, APP_TITLE " v%.20ls using LibPNG " PNG_LIBPNG_VER_STRING, szAppBurnVer);
	sprintf(szSource, "%s video game hardware", BurnDrvGetTextA(DRV_SYSTEM));

	text_ptr[0].key = "Title";
	text_ptr[0].text = (char*)BurnDrvGetTextA(DRV_FULLNAME);
	text_ptr[1].key = "Author";
	text_ptr[1].text = szAuthor;
	text_ptr[2].key = "Description";
	text_ptr[2].text = szDescription;
	text_ptr[3].key = "Copyright";
	text_ptr[3].text = szCopyright;
	text_ptr[4].key = "Creation Time";	text_ptr[4].text = png_convert_to_rfc1123(png_ptr, &png_time);
	text_ptr[5].key = "Software";		text_ptr[5].text = szSoftware;
	text_ptr[6].key = "Source";			text_ptr[6].text = szSource;
	text_ptr[7].key = "Comment";		text_ptr[7].text = "This screenshot was created by running the game in an emulator; it might not accurately reflect the actual hardware the game was designed to run on.";

	for (int i = 0; i < num_text; i++)
		text_ptr[i].compression = PNG_TEXT_COMPRESSION_NONE;

	png_set_text(png_ptr, info_ptr, text_ptr, num_text);

	png_init_io(png_ptr, ff);

	png_set_IHDR(png_ptr, info_ptr, w, h, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	png_write_info(png_ptr, info_ptr);

	png_set_filler(png_ptr, 0, PNG_FILLER_AFTER);

	png_set_bgr(png_ptr);

	png_bytep* pSShotImageRows = (png_bytep*)malloc(h * sizeof(png_bytep));
	for (int y = 0; y < h; y++)
		pSShotImageRows[y] = pSShot + (y * w * sizeof(int));

	png_write_image(png_ptr, pSShotImageRows);
	png_write_end(png_ptr, info_ptr);

	free(pSShotImageRows);
	fclose(ff);
	png_destroy_write_struct(&png_ptr, &info_ptr);

	free(pConvertedImage);
	pConvertedImage = NULL;

#endif
	return SSHOT_NOERROR;
}
#endif
