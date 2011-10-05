// Very simple module to download a file using WinInet API (CaptainCPS-X)
#include "burner.h"
#include <wininet.h>

bool bEnableAutoSupportFileDownload = false;
static TCHAR szFILESERVER1[512] = _T("http://fightercore.plesk3.freepgs.com/files/neosource/fba/support/");

// Concept only!
HINTERNET	dlUrl		= NULL;
HINTERNET	dlOpen		= NULL;
char*		dlContent	= NULL;
FILE*		dlPointer	= NULL;
DWORD		dlBuffer	= 0;
int			nFileDownloadCMD = -1;

void DlWriteData() 
{
	if(InternetReadFile(dlUrl, dlContent, sizeof(dlContent), &dlBuffer) && dlBuffer != 0) {
		fwrite(dlContent, 1, dlBuffer, dlPointer);
		dlContent[dlBuffer] = '\0';
		DlWriteData();									// loop
	} else {
		if(dlUrl)		InternetCloseHandle(dlUrl);
		dlUrl			= NULL;

		//if(dlOpen)		InternetCloseHandle(dlOpen);
		//dlOpen			= NULL;

		if(dlContent)	free(dlContent);
		dlContent		= NULL;

		if(dlPointer)	fclose(dlPointer);
		dlPointer		= NULL;

		dlBuffer		= 0;
	}
}


int FileDownload(TCHAR* szLocalImageDir, TCHAR* szLocalFilePath, TCHAR* szFile, TCHAR* szServerDir)
{
	if (bEnableAutoSupportFileDownload == false) return 0;

	//int nErrorCode		= 0;
	TCHAR szStatusCode[32];                                              
	DWORD dwStatusCodeSize	= 32; 
	TCHAR szFinalURL[512];

	_stprintf(szFinalURL, _T("%s%s%s"), szFILESERVER1, szServerDir, szFile);					// Prepare final URL for current download
	//dlOpen	= InternetOpen(_T("FBA_Download"), INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);		// Start initialization for this connection
	dlUrl	= InternetOpenUrl(dlOpen, szFinalURL, NULL, 0, INTERNET_FLAG_NO_AUTO_REDIRECT, 0);	// Connect to the final URL to start transfer later

	if(HttpQueryInfo(dlUrl, HTTP_QUERY_STATUS_CODE, szStatusCode, &dwStatusCodeSize, NULL)) 
	{
		long nStatusCode = _ttol(szStatusCode);

		if (nStatusCode == HTTP_STATUS_OK) {
			dlPointer = _tfopen(szLocalFilePath, _T("wb"));			// Create a new empty file for writing binary data

			// Check if the file was created...
			if(!dlPointer) {
				CreateDirectory(szLocalImageDir, NULL);				// Make sure a directory is present for the file
				dlPointer = _tfopen(szLocalFilePath, _T("wb"));		// Try creating the file again...
				if(!dlPointer) return 0;							// error creating the file
			}

			TCHAR szContentSize[32];                                              
			DWORD dwContentSizeSize = 32;   
			if (HttpQueryInfo(dlUrl, HTTP_QUERY_CONTENT_LENGTH, szContentSize, &dwContentSizeSize, NULL)) {
				long nContentSize	= _ttol(szContentSize);
				dlContent			= (char*)malloc(nContentSize);
				DlWriteData();	// Transfer the file contents and write them to the created file

			} else {
				return 0;	// HttpQueryInfo() failed [?]
			}
		} else {
			return 0;
		}
		//nErrorCode = nStatusCode;
	} else {
		return 0; // server not available
	}

	//if(nErrorCode != HTTP_STATUS_OK) return 0; // error

	return 1; // All good!
}
