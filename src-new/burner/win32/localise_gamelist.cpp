#include "burner.h"

TCHAR szGamelistLocalisationTemplate[MAX_PATH] = _T("");

static void MakeOfn()
{
	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hScrnWnd;
	ofn.lpstrFilter = _T("FB Alpha gamelist localisation templates (*.glt)\0*.glt\0\0)");
	ofn.lpstrFile = szChoice;
	ofn.nMaxFile = sizeof(szChoice) / sizeof(TCHAR);
	ofn.lpstrInitialDir = _T(".\\config\\localisation");
	ofn.Flags = OFN_NOCHANGEDIR | OFN_HIDEREADONLY;
	ofn.lpstrDefExt = _T("glt");
	return;
}

int SelectGameListLocalisationTemplate()
{
	int nRet;
	int bOldPause;

	MakeOfn();
	ofn.lpstrTitle = FBALoadStringEx(hAppInst, IDS_GAMELANG_LOAD, true);

	bOldPause = bRunPause;
	bRunPause = 1;
	nRet = GetOpenFileName(&ofn);
	bRunPause = bOldPause;

	if (nRet == 0) {		// Error
		return 1;
	}

	szGamelistLocalisationTemplate[0] = _T('\0');
	memcpy(szGamelistLocalisationTemplate, szChoice, sizeof(szChoice));
	
	BurnDoGameListLocalisation();
	
	return nRet;
}

static int WriteGamelistLocalisationTemplate(TCHAR* pszTemplate)
{
	unsigned int nOldDrvSelect = nBurnDrvSelect;
	FILE* fp = _tfopen(pszTemplate, _T("wt"));
		
	if (fp == NULL) {
		return 1;
	}
	
	_ftprintf(fp, _T("// game list translation template for FB Alpha\n\n"));
	_ftprintf(fp, _T("version 0x%06X\n\n"), nBurnVer);
	
	for (unsigned int i = 0; i < nBurnDrvCount; i++) {
		nBurnDrvSelect = i;
		_ftprintf(fp, _T("%s\t%s\n"), BurnDrvGetText(DRV_NAME), BurnDrvGetText(DRV_FULLNAME));
	}

	fclose(fp);
	
	nBurnDrvSelect = nOldDrvSelect;

	return 0;
}

int ExportGameListLocalisationTemplate()
{
	int nRet;
	int bOldPause;

	MakeOfn();
	ofn.lpstrTitle = FBALoadStringEx(hAppInst, IDS_GAMELANG_EXPORT, true);
	ofn.Flags |= OFN_OVERWRITEPROMPT;

	bOldPause = bRunPause;
	bRunPause = 1;
	nRet = GetSaveFileName(&ofn);
	bRunPause = bOldPause;

	if (nRet == 0) {		// Error
		return 1;
	}

	return WriteGamelistLocalisationTemplate(szChoice);
}
