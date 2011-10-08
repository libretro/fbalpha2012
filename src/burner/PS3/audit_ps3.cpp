// Display a dialog containing error messages
#include <stdio.h>  
#include "burner.h"
  
static char* pszBufferEnglish = NULL;

static int nBufferEnglishSize = 0;

static int nPopupFlags;

char msg[1024];

static int FBAPopupLog()
{
	for (char* szText = pszBufferEnglish; ; )
	{
		int nLen;

		for (nLen = 0; szText[nLen] && szText[nLen] != '\n'; nLen++) { }

		if (nLen)
		{
			char szFormat[16];
			sprintf(szFormat, "    %%.%is\n", nLen);
		}

		if (!szText[nLen])
			break;

		szText += nLen + 1;
	}

	return 0;
}

// ----------------------------------------------------------------------------

int FBAPopupAddText(int nFlags, char* pszFormat, ...)
{
	char szString[1024] = "";
	char* pszStringEnglish = NULL;

	va_list vaEnglish;
	va_start(vaEnglish, pszFormat);
	char* s1 = va_arg( vaEnglish, char * );
	char* s2 = va_arg( vaEnglish, char * );
	pszStringEnglish = pszFormat;

	// See if we need to load strings from reources
	if (nFlags & PUF_TEXT_TRANSLATE)
	{
		char* pszStringLocal;

		va_list vaLocal;
		va_start(vaLocal, pszFormat);

		pszStringLocal = pszFormat;

		va_end(vaLocal);
	}

	// Add the English language string
	if (pszStringEnglish && *pszStringEnglish)
	{
		int nLen = snprintf(szString, sizearray(szString), pszStringEnglish, s1, s2);
		if (nLen > 0)
		{
			char* pszNewBuffer = (char*)realloc(pszBufferEnglish, (nLen + nBufferEnglishSize + 1) * sizeof(char));
			if (pszNewBuffer)
			{
				pszBufferEnglish = pszNewBuffer;
				strncpy(pszBufferEnglish + nBufferEnglishSize, szString, nLen);
				nBufferEnglishSize += nLen;
				pszBufferEnglish[nBufferEnglishSize] = 0;
			}
		}
	}

	va_end(vaEnglish);

	return 0;
}

int FBAPopupDestroyText()
{
	free(pszBufferEnglish);
	pszBufferEnglish = NULL;
	nBufferEnglishSize = 0;

	return 0;
}

// ----------------------------------------------------------------------------

int FBAPopupDisplay(int nFlags)
{
	nPopupFlags = nFlags;
 
	FBAPopupLog();

	sprintf(msg,"%s",pszBufferEnglish);
			 
	UpdateConsoleXY(msg, 0.05f, 0.08f);		 

	FBAPopupDestroyText();

	return 1;
}
