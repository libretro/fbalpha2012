// Display a dialog containing error messages
#include <stdio.h>  
#include "burner.h"
  
static TCHAR* pszBufferEnglish = NULL;
static TCHAR* pszBufferLocal = NULL;

static int nBufferEnglishSize = 0;
static int nBufferLocalSize = 0;

static int nPopupFlags;

char msg[1024];

static int FBAPopupLog()
{
	TCHAR* pszTypeEnglish;

	switch (nPopupFlags & 7)
	{
		case PUF_TYPE_ERROR:
			pszTypeEnglish = FBALoadStringEx(IDS_ERR_ERROR, false);
			break;
		case PUF_TYPE_WARNING:
			pszTypeEnglish = FBALoadStringEx(IDS_ERR_WARNING, false);
			break;
		default:
			pszTypeEnglish = FBALoadStringEx(IDS_ERR_INFORMATION, false);
			break;
	}

	for (TCHAR* szText = pszBufferEnglish; ; )
	{
		int nLen;

		for (nLen = 0; szText[nLen] && szText[nLen] != _T('\n'); nLen++) { }

		if (nLen)
		{
			TCHAR szFormat[16];
			_stprintf(szFormat, _T("    %%.%is\n"), nLen);
		}

		if (!szText[nLen])
			break;

		szText += nLen + 1;
	}

	return 0;
}

// ----------------------------------------------------------------------------

int FBAPopupAddText(int nFlags, TCHAR* pszFormat, ...)
{
	TCHAR szString[1024] = _T("");
	TCHAR* pszStringEnglish = NULL;

	va_list vaEnglish;
	va_start(vaEnglish, pszFormat);
	char* s1 = va_arg( vaEnglish, char * );
	char* s2 = va_arg( vaEnglish, char * );
	pszStringEnglish = pszFormat;

	// See if we need to load strings from reources
	if (nFlags & PUF_TEXT_TRANSLATE) {
		TCHAR* pszStringLocal;

		va_list vaLocal;
		va_start(vaLocal, pszFormat);

		pszStringLocal = pszFormat;

		// Add the translated string if present
		if (bLanguageActive && (nFlags & PUF_TEXT_TRANSLATE) &&  pszStringLocal && *pszStringLocal)
		{
			// Add the translated string
			int nLen = snprintf(szString, sizearray(szString), pszStringLocal, vaLocal);
			if (nLen > 0)
			{
				TCHAR* pszNewBuffer = (TCHAR*)realloc(pszBufferLocal, (nLen + nBufferLocalSize + 1) * sizeof(TCHAR));
				if (pszNewBuffer)
				{
					pszBufferLocal = pszNewBuffer;
					_tcsncpy(pszBufferLocal + nBufferLocalSize, szString, nLen);
					nBufferLocalSize += nLen;
					pszBufferLocal[nBufferLocalSize] = 0;
				}
			}
		}

		va_end(vaLocal);
	}

	// Add the English language string
	if (pszStringEnglish && *pszStringEnglish)
	{
		int nLen = snprintf(szString, sizearray(szString), pszStringEnglish, s1, s2);
		if (nLen > 0)
		{
			TCHAR* pszNewBuffer = (TCHAR*)realloc(pszBufferEnglish, (nLen + nBufferEnglishSize + 1) * sizeof(TCHAR));
			if (pszNewBuffer)
			{
				pszBufferEnglish = pszNewBuffer;
				_tcsncpy(pszBufferEnglish + nBufferEnglishSize, szString, nLen);
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

	free(pszBufferLocal);
	pszBufferLocal = NULL;
	nBufferLocalSize = 0;

	return 0;
}

// ----------------------------------------------------------------------------

int FBAPopupDisplay(int nFlags)
{
	nPopupFlags = nFlags;
 
	FBAPopupLog();

	sprintf(msg,"%s",pszBufferEnglish);
			 
	UpdateConsoleXY(msg, 0.05f, 0.08f);		 
	sys_timer_usleep(3000000);

	FBAPopupDestroyText();

	return 1;
}
