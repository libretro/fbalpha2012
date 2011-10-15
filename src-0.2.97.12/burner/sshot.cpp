#include "burner.h"

#define SSHOT_NOERROR 0
#define SSHOT_ERROR_BPP_NOTSUPPORTED 1
#define SSHOT_LIBPNG_ERROR 2
#define SSHOT_OTHER_ERROR 3

#define SSHOT_DIRECTORY "screenshots\\"

static unsigned char* pSShot = NULL;
static unsigned char* pConvertedImage = NULL;
static png_bytep* pSShotImageRows = NULL;
static FILE* ff;

int MakeScreenShot()
{
	char szAuthor[256]; char szDescription[256]; char szCopyright[256];	char szSoftware[256]; char szSource[256];
	png_text text_ptr[8] = { { 0, 0, 0, 0, 0, 0, 0 }, };
	int num_text = 8;

    time_t currentTime;
    tm* tmTime;
    png_time_struct png_time_now;

    char szSShotName[MAX_PATH];
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
		free(pConvertedImage);
		pConvertedImage = NULL;

		free(pSShotImageRows);
		pSShotImageRows = NULL;

		fclose(ff);
        remove(szSShotName);

		return SSHOT_LIBPNG_ERROR;
    }

	if (pVidImage == NULL) {
		return SSHOT_OTHER_ERROR;
	}

    if (nVidImageBPP < 2 || nVidImageBPP > 4) {
        return SSHOT_ERROR_BPP_NOTSUPPORTED;
    }

	BurnDrvGetVisibleSize(&w, &h);

	pSShot = pVidImage;

	// Convert the image to 32-bit
	if (nVidImageBPP < 4) {
		unsigned char* pTemp = (unsigned char*)malloc(w * h * sizeof(int));

		if (nVidImageBPP == 2) {
			for (int i = 0; i < h * w; i++) {
				unsigned short nColour = ((unsigned short*)pSShot)[i];

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
		        *(pTemp + i * 4 + 0) = *(pSShot + i * 3 + 0);
		        *(pTemp + i * 4 + 1) = *(pSShot + i * 3 + 1);
		        *(pTemp + i * 4 + 2) = *(pSShot + i * 3 + 2);
			}
        }

		pConvertedImage = pTemp;

        pSShot = pConvertedImage;
	}

	// Rotate and flip the image
	if (BurnDrvGetFlags() & BDF_ORIENTATION_VERTICAL) {
		unsigned char* pTemp = (unsigned char*)malloc(w * h * sizeof(int));

		for (int x = 0; x < h; x++) {
			if (BurnDrvGetFlags() & BDF_ORIENTATION_FLIPPED) {
				for (int y = 0; y < w; y++) {
					((unsigned int*)pTemp)[(w - y - 1) + x * w] = ((unsigned int*)pSShot)[x + y * h];
				}
			} else {
				for (int y = 0; y < w; y++) {
					((unsigned int*)pTemp)[y + (h - x - 1) * w] = ((unsigned int*)pSShot)[x + y * h];
				}
			}
		}

		free(pConvertedImage);
		pConvertedImage = pTemp;

        pSShot = pConvertedImage;
	}
	else if (BurnDrvGetFlags() & BDF_ORIENTATION_FLIPPED) { // fixed rotation by regret
		unsigned char* pTemp = (unsigned char*)malloc(w * h * sizeof(int));

		for (int y = h - 1; y >= 0; y--) {
			for (int x = w - 1; x >= 0; x--) {
				((unsigned int*)pTemp)[(w - x - 1) + (h - y - 1) * w] = ((unsigned int*)pSShot)[x + y * w];
			}
		}

		free(pConvertedImage);
		pConvertedImage = pTemp;

        pSShot = pConvertedImage;
	}

	// Get the time
	time(&currentTime);
    tmTime = localtime(&currentTime);
	png_convert_from_time_t(&png_time_now, currentTime);

	// construct our filename -> "romname-mm-dd-hms.png"
    sprintf(szSShotName,"%s%s-%.2d-%.2d-%.2d%.2d%.2d.png", SSHOT_DIRECTORY, BurnDrvGetTextA(DRV_NAME), tmTime->tm_mon + 1, tmTime->tm_mday, tmTime->tm_hour, tmTime->tm_min, tmTime->tm_sec);

	ff = fopen(szSShotName, "wb");
	if (ff == NULL) {
		png_destroy_write_struct(&png_ptr, &info_ptr);

		free(pConvertedImage);
		pConvertedImage = NULL;

		return SSHOT_OTHER_ERROR;
	}

	// Fill the PNG text fields
#ifdef _UNICODE
	sprintf(szAuthor, APP_TITLE " v%.20ls", szAppBurnVer);
#else
	sprintf(szAuthor, APP_TITLE " v%.20s", szAppBurnVer);
#endif
	sprintf(szDescription, "Screenshot of %s", DecorateGameName(nBurnDrvSelect));
	sprintf(szCopyright, "%s %s", BurnDrvGetTextA(DRV_DATE), BurnDrvGetTextA(DRV_MANUFACTURER));
#ifdef _UNICODE
	sprintf(szSoftware, APP_TITLE " v%.20ls using LibPNG " PNG_LIBPNG_VER_STRING, szAppBurnVer);
#else
	sprintf(szSoftware, APP_TITLE " v%.20s using LibPNG " PNG_LIBPNG_VER_STRING, szAppBurnVer);
#endif
	sprintf(szSource, "%s video game hardware", BurnDrvGetTextA(DRV_SYSTEM));

	text_ptr[0].key = "Title";			text_ptr[0].text = BurnDrvGetTextA(DRV_FULLNAME);
	text_ptr[1].key = "Author";			text_ptr[1].text = szAuthor;
	text_ptr[2].key = "Description";	text_ptr[2].text = szDescription;
	text_ptr[3].key = "Copyright";		text_ptr[3].text = szCopyright;
	text_ptr[4].key = "Creation Time";	text_ptr[4].text = (char*)png_convert_to_rfc1123(png_ptr, &png_time_now);
	text_ptr[5].key = "Software";		text_ptr[5].text = szSoftware;
	text_ptr[6].key = "Source";			text_ptr[6].text = szSource;
	text_ptr[7].key = "Comment";		text_ptr[7].text = "This screenshot was created by running the game in an emulator; it might not accurately reflect the actual hardware the game was designed to run on.";

	for (int i = 0; i < num_text; i++) {
		text_ptr[i].compression = PNG_TEXT_COMPRESSION_NONE;
	}

	png_set_text(png_ptr, info_ptr, text_ptr, num_text);

	png_init_io(png_ptr, ff);

    png_set_IHDR(png_ptr, info_ptr, w, h, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_write_info(png_ptr, info_ptr);

	png_set_filler(png_ptr, 0, PNG_FILLER_AFTER);

    png_set_bgr(png_ptr);

	pSShotImageRows = (png_bytep*)malloc(h * sizeof(png_bytep));
    for (int y = 0; y < h; y++) {
        pSShotImageRows[y] = pSShot + (y * w * sizeof(int));
    }

	png_write_image(png_ptr, pSShotImageRows);
	png_write_end(png_ptr, info_ptr);

	free(pSShotImageRows);
	pSShotImageRows = NULL;

	fclose(ff);

	png_destroy_write_struct(&png_ptr, &info_ptr);

	free(pConvertedImage);
	pConvertedImage = NULL;

	return SSHOT_NOERROR;
}
