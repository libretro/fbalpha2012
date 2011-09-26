// Cheat module
// changed to read cheat file from memory, by regret
#include "burner.h"
#include "cheat.h"
#include "mem_file.h" // memory file

#ifdef _DEBUG
 #define _CRTDBG_MAP_ALLOC
 #include <crtdbg.h>

 #define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

// try to find and read kawaks/nebula/mame cheat files
//#define LOAD_OTHERCHEAT

static TCHAR szCheatFilename[MAX_PATH] = _T("");
vector<Mem_File*> cheatTempFiles; // save temp files

static Mem_File* getIncludeFile(const TCHAR* name);

static void cheatError(const TCHAR* pszFilename, int nLineNumber, const CheatInfo* pCheat, const TCHAR* pszInfo, const TCHAR* pszLine)
{
	FBAPopupAddText(PUF_TEXT_NO_TRANSLATE, _T("Cheat file %s is malformed.\nPlease remove or repair the file.\n\n"), pszFilename);
	if (pCheat) {
		FBAPopupAddText(PUF_TEXT_NO_TRANSLATE, _T("Parse error at line %i, in cheat \"%s\".\n"), nLineNumber, pCheat->szCheatName);
	} else {
		FBAPopupAddText(PUF_TEXT_NO_TRANSLATE, _T("Parse error at line %i.\n"), nLineNumber);
	}

	if (pszInfo) {
		FBAPopupAddText(PUF_TEXT_NO_TRANSLATE, _T("Problem:\t%s.\n"), pszInfo);
	}
	if (pszLine) {
		FBAPopupAddText(PUF_TEXT_NO_TRANSLATE, _T("Text:\t%s\n"), pszLine);
	}

	FBAPopupDisplay(PUF_TYPE_ERROR);
}

#ifdef LOAD_OTHERCHEAT
// parse nebula/kawaks cheat data
static int parseNebulaCheat(TCHAR* pszFilename)
{
	FILE* fp = _tfopen(pszFilename, _T("rt"));
	if (fp == NULL) {
		return 1;
	}

	int nLen;
	int i, j, n = 0;
	TCHAR tmp[32];
	TCHAR szLine[2048];

	CheatInfo* pCurrentCheat = NULL;

	while (1) {
		if (_fgetts(szLine, sizearray(szLine), fp) == NULL)
			break;

		nLen = _tcslen(szLine);

		if (nLen < 3 || szLine[0] == '[')
			continue;

		if (!_tcsncmp (_T("Name="), szLine, 5)) {
			n = 0;

			// Link new node into the list
			CheatInfo* pPreviousCheat = pCurrentCheat;
			pCurrentCheat = (CheatInfo*)malloc(sizeof(CheatInfo));
			if (pCheatInfo == NULL) {
				pCheatInfo = pCurrentCheat;
			}

			memset(pCurrentCheat, 0, sizeof(CheatInfo));
			pCurrentCheat->pPrevious = pPreviousCheat;
			if (pPreviousCheat) {
				pPreviousCheat->pNext = pCurrentCheat;
			}

			// Fill in defaults
			pCurrentCheat->nType = 0;							// Default to cheat type 0 (apply each frame)
			pCurrentCheat->nStatus = -1;						// Disable cheat
			pCurrentCheat->nDefault = 0;						// Set default option

			_tcsncpy (pCurrentCheat->szCheatName, szLine + 5, QUOTE_MAX);
			pCurrentCheat->szCheatName[nLen-6] = '\0';

			continue;
		}

		if (!_tcsncmp (_T("Default="), szLine, 8) && n >= 0) {
			_tcsncpy (tmp, szLine + 8, nLen-9);
			tmp[nLen-9] = '\0';
			_stscanf (tmp, _T("%d"), &(pCurrentCheat->nDefault));
			continue;
		}

		i = 0, j = 0;
		while (i < nLen) {
			if (szLine[i] == '=' && i < 4)
				j = i+1;

			if (szLine[i] == ',' || szLine[i] == '\n') {
				if (pCurrentCheat->pOption[n] == NULL) {
					pCurrentCheat->pOption[n] = (CheatOption*)malloc(sizeof(CheatOption));
				}
				memset(pCurrentCheat->pOption[n], 0, sizeof(CheatOption));

				_tcsncpy (pCurrentCheat->pOption[n]->szOptionName, szLine + j, QUOTE_MAX * sizeof(TCHAR));
				pCurrentCheat->pOption[n]->szOptionName[i-j] = '\0';

				i++;
				j = i;
				break;
			}
			i++;
		}

		int nAddress = -1;
		int nValue = 0;
		int nCurrentAddress = 0;

		while (nCurrentAddress < CHEAT_MAX_ADDRESS) {
			if (i == nLen) break;

			if (szLine[i] == ',' || szLine[i] == '\n') {
				_tcsncpy (tmp, szLine + j, i-j);
				tmp[i-j] = '\0';

				if (nAddress == -1) {
					_stscanf (tmp, _T("%x"), &nAddress);
				} else {
					_stscanf (tmp, _T("%x"), &nValue);

					pCurrentCheat->pOption[n]->AddressInfo[nCurrentAddress].nCPU = 0; 	// Always
					pCurrentCheat->pOption[n]->AddressInfo[nCurrentAddress].nAddress = nAddress ^ 1;
					pCurrentCheat->pOption[n]->AddressInfo[nCurrentAddress].nValue = nValue;
					nCurrentAddress++;

					nAddress = -1;
					nValue = 0;
				}
				j = i+1;
			}
			i++;
		}
		n++;
	}

	fclose (fp);

	return 0;
}

// parse mame cheat data
static int parseMAMECheat()
{
	// macro define
#define AddressInfo()	\
	int k = (flags >> 20) & 3;	\
	for (int i = 0; i < k+1; i++) {	\
		pCurrentCheat->pOption[n]->AddressInfo[nCurrentAddress].nCPU = 0;	\
		pCurrentCheat->pOption[n]->AddressInfo[nCurrentAddress].nAddress = nAddress + i;	\
		pCurrentCheat->pOption[n]->AddressInfo[nCurrentAddress].nValue = (nValue >> ((k*8)-(i*8))) & 0xff;	\
		nCurrentAddress++;	\
	}	\

#define OptionName(a)	\
	if (pCurrentCheat->pOption[n] == NULL) {						\
		pCurrentCheat->pOption[n] = (CheatOption*)malloc(sizeof(CheatOption));		\
	}											\
	memset(pCurrentCheat->pOption[n], 0, sizeof(CheatOption));				\
	_tcsncpy (pCurrentCheat->pOption[n]->szOptionName, a, QUOTE_MAX * sizeof(TCHAR));	\

#define tmpcpy(a)	\
	_tcsncpy (tmp, szLine + c0[a] + 1, c0[a+1] - (c0[a]+1));	\
	tmp[c0[a+1] - (c0[a]+1)] = '\0';				\

	// open mame cheat.dat
	_stprintf(szCheatFilename, _T("%s%s"), getMiscPath(PATH_CHEAT), _T("cheat.dat"));
	FILE* fz = _tfopen(szCheatFilename, _T("rt"));
	if (fz == NULL) {
		return 1;
	}

	TCHAR tmp[MAX_PATH];
	TCHAR gName[MAX_PATH];
	TCHAR szLine[2048];

	int nLen;
	int n = 0;
	int menu = 0;
	int nFound = 0;
	int nCurrentAddress = 0;
	unsigned int flags = 0;
	unsigned int nAddress = 0;
	unsigned int nValue = 0;

	CheatInfo* pCurrentCheat = NULL;
	_stprintf(gName, _T(":%s:"), BurnDrvGetText(DRV_NAME));

	while (1) {
		if (_fgetts(szLine, sizearray(szLine), fz) == NULL)
			break;

		nLen = _tcslen(szLine);

		if (szLine[0] == ';')
			continue;

		if (_tcsncmp(szLine, gName, _tcslen(gName))) {
			if (nFound) break;
			else continue;
		}

		nFound = 1;

		int c0[16], c1 = 0;				// find colons / break
		for (int i = 0; i < nLen; i++)
			if (szLine[i] == ':' || szLine[i] == '\n')
				c0[c1++] = i;

		tmpcpy(1);						// control flags
		_stscanf(tmp, _T("%x"), &flags);

		tmpcpy(2);						// cheat address
		_stscanf(tmp, _T("%x"), &nAddress);

		tmpcpy(3);						// cheat value
		_stscanf(tmp, _T("%x"), &nValue);

		tmpcpy(5);						// cheat name

		if (flags & 0x80007f00) continue;		// skip various cheats

		// controls how many bytes we're going to patch (only allow single bytes for now)
	//	if (flags & 0x00300000) continue;
	//	nValue &= 0x000000ff;			// only use a single byte

		if ( flags & 0x00008000 || (flags & 0x0001000 && !menu)) {
			if (nCurrentAddress < CHEAT_MAX_ADDRESS) {
				AddressInfo();
			}

			continue;
		}

		if (~flags & 0x00010000) {
			n = 0;
			menu = 0;
			nCurrentAddress = 0;

			// Link new node into the list
			CheatInfo* pPreviousCheat = pCurrentCheat;
			pCurrentCheat = (CheatInfo*)malloc(sizeof(CheatInfo));
			if (pCheatInfo == NULL) {
				pCheatInfo = pCurrentCheat;
			}

			memset(pCurrentCheat, 0, sizeof(CheatInfo));
			pCurrentCheat->pPrevious = pPreviousCheat;
			if (pPreviousCheat) {
				pPreviousCheat->pNext = pCurrentCheat;
			}

			// Fill in defaults
			pCurrentCheat->nType = 0;							// Default to cheat type 0 (apply each frame)
			pCurrentCheat->nStatus = -1;						// Disable cheat
			pCurrentCheat->nDefault = 0;						// Set default option

			_tcsncpy(pCurrentCheat->szCheatName, tmp, QUOTE_MAX);

			if (_tcslen(tmp) <= 0 || flags == 0x60000000) {
				n++;
				continue;
			}

			OptionName(_T("Disabled"));

			if (nAddress) {
				n++;

				OptionName(tmp);
				AddressInfo();
			} else {
				menu = 1;
			}

			continue;
		}

		if (flags & 0x00010000 && menu) {
			n++;
			nCurrentAddress = 0;

			OptionName(tmp);
			AddressInfo();

			continue;
		}
	}

	fclose (fz);

	return 0;
}
#endif

// ==> load cheat from memory, added by regret
static int parseMemCheatFile(Mem_File* file)
{
#define INSIDE_NOTHING (0xFFFF & (1 << (sizeof(TCHAR) * 8) - 1))

	if (!file) {
		return 1;
	}

	bool utf8File = false;
	// read file header
	unsigned char header[4] = { 0, };
	file->read((char*)header, sizearray(header), NULL);

	// check if it's a UTF-8 file
	if (header[0] == 0xEF && header[1] == 0xBB && header[2] == 0xBF) {
		utf8File = true;
	}
	file->seek(0);

	if (utf8File) {
		file->seek(3);
	}
	file->set_utf8(utf8File);

	TCHAR szLine[2048];
	TCHAR* s;
	TCHAR* t;
	size_t nLen;

	int nLine = 0;
	TCHAR nInside = INSIDE_NOTHING;

	CheatInfo* pCurrentCheat = NULL;

	while (1) {
		char szTemp[2048] = "";
		if (file->gets(szTemp, sizearray(szTemp))) {
			break;
		}
		_tcscpy(szLine, utf8File ? U8toW(szTemp) : AtoW(szTemp));

		nLine++;

		nLen = _tcslen(szLine);
		// Get rid of the linefeed at the end
		while (nLen != 0 && (szLine[nLen - 1] == 0x0A || szLine[nLen - 1] == 0x0D)) {
			szLine[nLen - 1] = 0;
			nLen--;
		}

		s = szLine;												// Start parsing

		if (s[0] == _T('/') && s[1] == _T('/')) {				// Comment
			continue;
		}

		if ((t = labelCheck(s, _T("include"))) != 0) {			// Include a file
			s = t;

			// Read name of the cheat file
			TCHAR* szQuote = NULL;
			quoteRead(&szQuote, NULL, s);

			Mem_File* mem_file = getIncludeFile(szQuote);
			if (!mem_file || parseMemCheatFile(mem_file)) {
				cheatError(szCheatFilename, nLine, NULL, _T("included file doesn't exist"), szLine);
			}

			continue;
		}

		if ((t = labelCheck(s, _T("cheat"))) != 0) {			// Add new cheat
			s = t;

			// Read cheat name
			TCHAR* szQuote = NULL;
			TCHAR* szEnd = NULL;

			quoteRead(&szQuote, &szEnd, s);

			s = szEnd;

			if ((t = labelCheck(s, _T("advanced"))) != 0) {		// Advanced cheat
				s = t;
			}

			SKIP_WS(s);

			if (nInside == _T('{')) {
				cheatError(szCheatFilename, nLine, pCurrentCheat, _T("missing closing bracket"), NULL);
				break;
			}
#if 0
			if (*s != _T('\0') && *s != _T('{')) {
				cheatError(szCheatFilename, nLine, NULL, _T("malformed cheat declaration"), szLine);
				break;
			}
#endif
			nInside = *s;

			// Link new node into the list
			CheatInfo* pPreviousCheat = pCurrentCheat;
			pCurrentCheat = (CheatInfo*)malloc(sizeof(CheatInfo));
			if (pCheatInfo == NULL) {
				pCheatInfo = pCurrentCheat;
			}

			memset(pCurrentCheat, 0, sizeof(CheatInfo));
			pCurrentCheat->pPrevious = pPreviousCheat;
			if (pPreviousCheat) {
				pPreviousCheat->pNext = pCurrentCheat;
			}

			// Fill in defaults
			pCurrentCheat->nType = 0;							// Default to cheat type 0 (apply each frame)
			pCurrentCheat->nStatus = -1;						// Disable cheat

			memcpy(pCurrentCheat->szCheatName, szQuote, QUOTE_MAX);

			continue;
		}

		if ((t = labelCheck(s, _T("type"))) != 0) {				// Cheat type
			if (nInside == INSIDE_NOTHING || pCurrentCheat == NULL) {
				cheatError(szCheatFilename, nLine, pCurrentCheat, _T("rogue cheat type"), szLine);
				break;
			}

			s = t;

			// Set type
			pCurrentCheat->nType = _tcstol(s, NULL, 0);

			continue;
		}

		if ((t = labelCheck(s, _T("default"))) != 0) {			// Default option
			if (nInside == INSIDE_NOTHING || pCurrentCheat == NULL) {
				cheatError(szCheatFilename, nLine, pCurrentCheat, _T("rogue default"), szLine);
				break;
			}

			s = t;

			// Set default option
			pCurrentCheat->nDefault = _tcstol(s, NULL, 0);

			continue;
		}

		int n = _tcstol(s, &t, 0);
		if (t != s) {				   							// New option
			if (nInside == INSIDE_NOTHING || pCurrentCheat == NULL) {
				cheatError(szCheatFilename, nLine, pCurrentCheat, _T("rogue option"), szLine);
				break;
			}

			// Link a new Option structure to the cheat
			if (n < CHEAT_MAX_OPTIONS) {
				s = t;

				// Read option name
				TCHAR* szQuote = NULL;
				TCHAR* szEnd = NULL;
				if (quoteRead(&szQuote, &szEnd, s)) {
					cheatError(szCheatFilename, nLine, pCurrentCheat, _T("option name omitted"), szLine);
					break;
				}
				s = szEnd;

				if (pCurrentCheat->pOption[n] == NULL) {
					pCurrentCheat->pOption[n] = (CheatOption*)malloc(sizeof(CheatOption));
				}
				memset(pCurrentCheat->pOption[n], 0, sizeof(CheatOption));

				memcpy(pCurrentCheat->pOption[n]->szOptionName, szQuote, QUOTE_MAX * sizeof(TCHAR));

				int nCurrentAddress = 0;
				bool bOK = true;
				while (nCurrentAddress < CHEAT_MAX_ADDRESS) {
					int nCPU = 0, nAddress = 0, nValue = 0;

					if (skipComma(&s)) {
						nCPU = _tcstol(s, &t, 0);		// CPU number
						if (t == s) {
							cheatError(szCheatFilename, nLine, pCurrentCheat, _T("CPU number omitted"), szLine);
							bOK = false;
							break;
						}
						s = t;

						skipComma(&s);
						nAddress = _tcstol(s, &t, 0);	// Address
						if (t == s) {
							bOK = false;
							cheatError(szCheatFilename, nLine, pCurrentCheat, _T("address omitted"), szLine);
							break;
						}
						s = t;

						skipComma(&s);
						nValue = _tcstol(s, &t, 0);		// Value
						if (t == s) {
							bOK = false;
							cheatError(szCheatFilename, nLine, pCurrentCheat, _T("value omitted"), szLine);
							break;
						}
					} else {
						if (nCurrentAddress) {			// Only the first option is allowed no address
							break;
						}
						if (n) {
							bOK = false;
							cheatError(szCheatFilename, nLine, pCurrentCheat, _T("CPU / address / value omitted"), szLine);
							break;
						}
					}

					pCurrentCheat->pOption[n]->AddressInfo[nCurrentAddress].nCPU = nCPU;
					pCurrentCheat->pOption[n]->AddressInfo[nCurrentAddress].nAddress = nAddress;
					pCurrentCheat->pOption[n]->AddressInfo[nCurrentAddress].nValue = nValue;
					nCurrentAddress++;
				}

				if (!bOK) {
					break;
				}
			}

			continue;
		}

		SKIP_WS(s);
		if (*s == _T('}')) {
			if (nInside != _T('{')) {
				cheatError(szCheatFilename, nLine, pCurrentCheat, _T("missing opening bracket"), NULL);
				break;
			}

			nInside = INSIDE_NOTHING;
		}

		// Line isn't (part of) a valid cheat
#if 0
		if (*s) {
			cheatError(szCheatFilename, nLine, NULL, _T("rogue line"), szLine);
			break;
		}
#endif
	}

	return 0;
}

static int getCheatArcName(TCHAR* name)
{
	if (!name) {
		return 1;
	}

	_stprintf(name, _T("%s%s"), getMiscPath(PATH_CHEAT), getMiscArchiveName(PATH_CHEAT));

	int ret = archiveCheck(name);
	if (ret == ARC_NONE) {
		return 1; // no archive
	}
	return 0;
}

static int cheatLoadArchive()
{
	TCHAR szCheatArcName[MAX_PATH] = _T("");
	getCheatArcName(szCheatArcName);

	_stprintf(szCheatFilename, _T("%s.ini"), BurnDrvGetText(DRV_NAME));
	void* buf = NULL;
	int size = 0;
	if (archiveLoadOneFile(szCheatArcName, szCheatFilename, &buf, &size)) {
		return 1;
	}

	Mem_File* mem_file = new Mem_File((char*)buf, size);
	cheatTempFiles.push_back(mem_file);

	return parseMemCheatFile(mem_file);
}

static Mem_File* getIncludeFile(const TCHAR* name)
{
	_stprintf(szCheatFilename, _T("%s%s.ini"), getMiscPath(PATH_CHEAT), name);

	void* buf = NULL;
	int size = getFileBuffer(szCheatFilename, &buf);
	if (size) {
		Mem_File* mem_file = new Mem_File((char*)buf, size);
		cheatTempFiles.push_back(mem_file);
		return mem_file;
	}

	// load from archive
	TCHAR szCheatArcName[MAX_PATH] = _T("");
	getCheatArcName(szCheatArcName);

	_stprintf(szCheatFilename, _T("%s.ini"), name);
	if (archiveLoadOneFile(szCheatArcName, szCheatFilename, &buf, &size)) {
		return NULL;
	}

	// write file content to buffer
	Mem_File* mem_file = new Mem_File((char*)buf, size);
	cheatTempFiles.push_back(mem_file);
	return mem_file;
}

int clearCheatTempFiles()
{
	for (size_t i = 0; i < cheatTempFiles.size(); i++) {
		delete cheatTempFiles[i];
	}
	cheatTempFiles.clear();
	return 0;
}
// <== load cheat from memory

int configCheatLoad(const TCHAR* filename)
{
	if (!filename) {
		_stprintf(szCheatFilename, _T("%s%s.ini"), getMiscPath(PATH_CHEAT), BurnDrvGetText(DRV_NAME));
	} else {
		_tcsncpy(szCheatFilename, filename, sizearray(szCheatFilename));
	}

	Mem_File* mem_file = NULL;

	// write file content to buffer
	void* buf = NULL;
	int size = getFileBuffer(szCheatFilename, &buf);
	if (size) {
		mem_file = new Mem_File((char*)buf, size);
		cheatTempFiles.push_back(mem_file);
	}

	if (parseMemCheatFile(mem_file)) {
		// try to load cheat from archive files
		if (cheatLoadArchive()) {
			return 1;
		}
	}

	clearCheatTempFiles();

#ifdef LOAD_OTHERCHEAT
	// try to find and read kawaks/nebula/mame cheat files
	_stprintf(szCheatFilename, _T("%s%s.dat"), getMiscPath(PATH_CHEAT), BurnDrvGetText(DRV_NAME));
	if (parseNebulaCheat(szCheatFilename)) {
		if (parseMAMECheat()) {
			return 1;
		}
	}
#endif

	if (pCheatInfo) {
		int nCurrentCheat = 0;
		while (cheatEnable(nCurrentCheat, -1) == 0) {
			nCurrentCheat++;
		}

		cheatUpdate();
	}

	return 0;
}

// cheat reload, added by regret
int configCheatReload(const TCHAR* filename)
{
	cheatExit(false);
	clearCheatTempFiles();

	cheatInit();
	configCheatLoad(filename);
	return 0;
}
