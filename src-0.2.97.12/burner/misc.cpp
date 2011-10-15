// Misc functions module
#include <math.h>
#include "burner.h"

// ---------------------------------------------------------------------------
// Software gamma

int bDoGamma = 0;
int bHardwareGammaOnly = 1;
double nGamma = 1.25;
const int nMaxRGB = 255;

static unsigned char GammaLUT[256];

void ComputeGammaLUT()
{
	for (int i = 0; i < 256; i++) {
		GammaLUT[i] = (unsigned char)((double)nMaxRGB * pow((i / 255.0), nGamma));
	}
}

// Standard callbacks for 16/24/32 bit color:
static unsigned int __cdecl HighCol15(int r, int g, int b, int  /* i */)
{
	unsigned int t;
	t =(r<<7)&0x7c00; // 0rrr rr00 0000 0000
	t|=(g<<2)&0x03e0; // 0000 00gg ggg0 0000
	t|=(b>>3)&0x001f; // 0000 0000 000b bbbb
	return t;
}

static unsigned int __cdecl HighCol16(int r, int g, int b, int /* i */)
{
	unsigned int t;
	t =(r<<8)&0xf800; // rrrr r000 0000 0000
	t|=(g<<3)&0x07e0; // 0000 0ggg ggg0 0000
	t|=(b>>3)&0x001f; // 0000 0000 000b bbbb
	return t;
}

// 24-bit/32-bit
static unsigned int __cdecl HighCol24(int r, int g, int b, int  /* i */)
{
	unsigned int t;
	t =(r<<16)&0xff0000;
	t|=(g<<8 )&0x00ff00;
	t|=(b    )&0x0000ff;

	return t;
}

static unsigned int __cdecl HighCol15Gamma(int r, int g, int b, int  /* i */)
{
	unsigned int t;
	t = (GammaLUT[r] << 7) & 0x7C00; // 0rrr rr00 0000 0000
	t |= (GammaLUT[g] << 2) & 0x03E0; // 0000 00gg ggg0 0000
	t |= (GammaLUT[b] >> 3) & 0x001F; // 0000 0000 000b bbbb
	return t;
}

static unsigned int __cdecl HighCol16Gamma(int r, int g ,int b, int  /* i */)
{
	unsigned int t;
	t = (GammaLUT[r] << 8) & 0xF800; // rrrr r000 0000 0000
	t |= (GammaLUT[g] << 3) & 0x07E0; // 0000 0ggg ggg0 0000
	t |= (GammaLUT[b] >> 3) & 0x001F; // 0000 0000 000b bbbb
	return t;
}

// 24-bit/32-bit
static unsigned int __cdecl HighCol24Gamma(int r, int g, int b, int  /* i */)
{
	unsigned int t;
	t = (GammaLUT[r] << 16);
	t |= (GammaLUT[g] << 8);
	t |= GammaLUT[b];

	return t;
}

int SetBurnHighCol(int nDepth)
{
	VidRecalcPal();

	if (bDoGamma && ((nVidFullscreen && !bVidUseHardwareGamma) || (!nVidFullscreen && !bHardwareGammaOnly))) {
		if (nDepth == 15) {
			VidHighCol = HighCol15Gamma;
		}
		if (nDepth == 16) {
			VidHighCol = HighCol16Gamma;
		}
		if (nDepth > 16) {
			VidHighCol = HighCol24Gamma;
		}
	} else {
		if (nDepth == 15) {
			VidHighCol = HighCol15;
		}
		if (nDepth == 16) {
			VidHighCol = HighCol16;
		}
		if (nDepth > 16) {
			VidHighCol = HighCol24;
		}
	}
	if ((bDrvOkay && !(BurnDrvGetFlags() & BDF_16BIT_ONLY)) || nDepth <= 16) {
		BurnHighCol = VidHighCol;
	}

	return 0;
}

// ---------------------------------------------------------------------------

char* DecorateGameName(unsigned int nBurnDrv)
{
	static char szDecoratedName[256];
	unsigned int nOldBurnDrv = nBurnDrvSelect;

	nBurnDrvSelect = nBurnDrv;

	const char* s1 = "";
	const char* s2 = "";
	const char* s3 = "";
	const char* s4 = "";
	const char* s5 = "";
	const char* s6 = "";
	const char* s7 = "";
	const char* s8 = "";
	const char* s9 = "";
	const char* s10 = "";
	const char* s11 = "";
	const char* s12 = "";
	const char* s13 = "";
	const char* s14 = "";

	s1 = BurnDrvGetTextA(DRV_FULLNAME);
	if ((BurnDrvGetFlags() & BDF_DEMO) || (BurnDrvGetFlags() & BDF_HACK) || (BurnDrvGetFlags() & BDF_HOMEBREW) || (BurnDrvGetFlags() & BDF_PROTOTYPE) || (BurnDrvGetFlags() & BDF_BOOTLEG) || (BurnDrvGetTextA(DRV_COMMENT) && strlen(BurnDrvGetTextA(DRV_COMMENT)) > 0)) {
		s2 = " [";
		if (BurnDrvGetFlags() & BDF_DEMO) {
			s3 = "Demo";
			if ((BurnDrvGetFlags() & BDF_HACK) || (BurnDrvGetFlags() & BDF_HOMEBREW) || (BurnDrvGetFlags() & BDF_PROTOTYPE) || (BurnDrvGetFlags() & BDF_BOOTLEG) || (BurnDrvGetTextA(DRV_COMMENT) && strlen(BurnDrvGetTextA(DRV_COMMENT)) > 0)) {
				s4 = ", ";
			}
		}
		if (BurnDrvGetFlags() & BDF_HACK) {
			s5 = "Hack";
			if ((BurnDrvGetFlags() & BDF_HOMEBREW) || (BurnDrvGetFlags() & BDF_PROTOTYPE) || (BurnDrvGetFlags() & BDF_BOOTLEG) || (BurnDrvGetTextA(DRV_COMMENT) && strlen(BurnDrvGetTextA(DRV_COMMENT)) > 0)) {
				s6 = ", ";
			}
		}
		if (BurnDrvGetFlags() & BDF_HOMEBREW) {
			s7 = "Homebrew";
			if ((BurnDrvGetFlags() & BDF_PROTOTYPE) || (BurnDrvGetFlags() & BDF_BOOTLEG) || (BurnDrvGetTextA(DRV_COMMENT) && strlen(BurnDrvGetTextA(DRV_COMMENT)) > 0)) {
				s8 = ", ";
			}
		}
		if (BurnDrvGetFlags() & BDF_PROTOTYPE) {
			s9 = "Prototype";
			if ((BurnDrvGetFlags() & BDF_BOOTLEG) || (BurnDrvGetTextA(DRV_COMMENT) && strlen(BurnDrvGetTextA(DRV_COMMENT)) > 0)) {
				s10 = ", ";
			}
		}		
		if (BurnDrvGetFlags() & BDF_BOOTLEG) {
			s11 = "Bootleg";
			if (BurnDrvGetTextA(DRV_COMMENT) && strlen(BurnDrvGetTextA(DRV_COMMENT)) > 0) {
				s12 = ", ";
			}
		}
		if (BurnDrvGetTextA(DRV_COMMENT) && strlen(BurnDrvGetTextA(DRV_COMMENT)) > 0) {
			s13 = BurnDrvGetTextA(DRV_COMMENT);
		}
		s14 = "]";
	}

	sprintf(szDecoratedName, "%s%s%s%s%s%s%s%s%s%s%s%s%s%s", s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11, s12, s13, s14);

	nBurnDrvSelect = nOldBurnDrv;
	return szDecoratedName;
}

TCHAR* DecorateGenreInfo()
{
	int nGenre = BurnDrvGetGenreFlags();
	int nFamily = BurnDrvGetFamilyFlags();
	
	static TCHAR szDecoratedGenre[256];
	TCHAR szFamily[256];
	
	_stprintf(szDecoratedGenre, _T(""));
	_stprintf(szFamily, _T(""));
	
	if (nGenre) {
		if (nGenre & GBF_HORSHOOT) {
			_stprintf(szDecoratedGenre, _T("%s%s, "), szDecoratedGenre, FBALoadStringEx(hAppInst, IDS_GENRE_HORSHOOT, true));
		}
		
		if (nGenre & GBF_VERSHOOT) {
			_stprintf(szDecoratedGenre, _T("%s%s, "), szDecoratedGenre, FBALoadStringEx(hAppInst, IDS_GENRE_VERSHOOT, true));
		}
		
		if (nGenre & GBF_SCRFIGHT) {
			_stprintf(szDecoratedGenre, _T("%s%s, "), szDecoratedGenre, FBALoadStringEx(hAppInst, IDS_GENRE_SCRFIGHT, true));
		}
		
		if (nGenre & GBF_VSFIGHT) {
			_stprintf(szDecoratedGenre, _T("%s%s, "), szDecoratedGenre, FBALoadStringEx(hAppInst, IDS_GENRE_VSFIGHT, true));
		}
		
		if (nGenre & GBF_BIOS) {
			_stprintf(szDecoratedGenre, _T("%s%s, "), szDecoratedGenre, FBALoadStringEx(hAppInst, IDS_GENRE_BIOS, true));
		}
		
		if (nGenre & GBF_BREAKOUT) {
			_stprintf(szDecoratedGenre, _T("%s%s, "), szDecoratedGenre, FBALoadStringEx(hAppInst, IDS_GENRE_BREAKOUT, true));
		}
		
		if (nGenre & GBF_CASINO) {
			_stprintf(szDecoratedGenre, _T("%s%s, "), szDecoratedGenre, FBALoadStringEx(hAppInst, IDS_GENRE_CASINO, true));
		}
		
		if (nGenre & GBF_BALLPADDLE) {
			_stprintf(szDecoratedGenre, _T("%s%s, "), szDecoratedGenre, FBALoadStringEx(hAppInst, IDS_GENRE_BALLPADDLE, true));
		}
		
		if (nGenre & GBF_MAZE) {
			_stprintf(szDecoratedGenre, _T("%s%s, "), szDecoratedGenre, FBALoadStringEx(hAppInst, IDS_GENRE_MAZE, true));
		}
		
		if (nGenre & GBF_MINIGAMES) {
			_stprintf(szDecoratedGenre, _T("%s%s, "), szDecoratedGenre, FBALoadStringEx(hAppInst, IDS_GENRE_MINIGAMES, true));
		}
		
		if (nGenre & GBF_PINBALL) {
			_stprintf(szDecoratedGenre, _T("%s%s, "), szDecoratedGenre, FBALoadStringEx(hAppInst, IDS_GENRE_PINBALL, true));
		}
		
		if (nGenre & GBF_PLATFORM) {
			_stprintf(szDecoratedGenre, _T("%s%s, "), szDecoratedGenre, FBALoadStringEx(hAppInst, IDS_GENRE_PLATFORM, true));
		}
		
		if (nGenre & GBF_PUZZLE) {
			_stprintf(szDecoratedGenre, _T("%s%s, "), szDecoratedGenre, FBALoadStringEx(hAppInst, IDS_GENRE_PUZZLE, true));
		}
		
		if (nGenre & GBF_QUIZ) {
			_stprintf(szDecoratedGenre, _T("%s%s, "), szDecoratedGenre, FBALoadStringEx(hAppInst, IDS_GENRE_QUIZ, true));
		}
		
		if (nGenre & GBF_SPORTSMISC) {
			_stprintf(szDecoratedGenre, _T("%s%s, "), szDecoratedGenre, FBALoadStringEx(hAppInst, IDS_GENRE_SPORTSMISC, true));
		}
		
		if (nGenre & GBF_SPORTSFOOTBALL) {
			_stprintf(szDecoratedGenre, _T("%s%s, "), szDecoratedGenre, FBALoadStringEx(hAppInst, IDS_GENRE_SPORTSFOOTBALL, true));
		}
		
		if (nGenre & GBF_MISC) {
			_stprintf(szDecoratedGenre, _T("%s%s, "), szDecoratedGenre, FBALoadStringEx(hAppInst, IDS_GENRE_MISC, true));
		}
		
		if (nGenre & GBF_MAHJONG) {
			_stprintf(szDecoratedGenre, _T("%s%s, "), szDecoratedGenre, FBALoadStringEx(hAppInst, IDS_GENRE_MAHJONG, true));
		}
		
		if (nGenre & GBF_RACING) {
			_stprintf(szDecoratedGenre, _T("%s%s, "), szDecoratedGenre, FBALoadStringEx(hAppInst, IDS_GENRE_RACING, true));
		}
		
		if (nGenre & GBF_SHOOT) {
			_stprintf(szDecoratedGenre, _T("%s%s, "), szDecoratedGenre, FBALoadStringEx(hAppInst, IDS_GENRE_SHOOT, true));
		}
		
		szDecoratedGenre[_tcslen(szDecoratedGenre) - 2] = _T('\0');
	}
	
	if (nFamily) {
		_stprintf(szFamily, _T(" ("));
		
		if (nFamily & FBF_MSLUG) {
			_stprintf(szFamily, _T("%s%s, "), szFamily, FBALoadStringEx(hAppInst, IDS_FAMILY_MSLUG, true));
		}
		
		if (nFamily & FBF_SF) {
			_stprintf(szFamily, _T("%s%s, "), szFamily, FBALoadStringEx(hAppInst, IDS_FAMILY_SF, true));
		}
		
		if (nFamily & FBF_KOF) {
			_stprintf(szFamily, _T("%s%s, "), szFamily, FBALoadStringEx(hAppInst, IDS_FAMILY_KOF, true));
		}
		
		if (nFamily & FBF_DSTLK) {
			_stprintf(szFamily, _T("%s%s, "), szFamily, FBALoadStringEx(hAppInst, IDS_FAMILY_DSTLK, true));
		}
		
		if (nFamily & FBF_FATFURY) {
			_stprintf(szFamily, _T("%s%s, "), szFamily, FBALoadStringEx(hAppInst, IDS_FAMILY_FATFURY, true));
		}
		
		if (nFamily & FBF_SAMSHO) {
			_stprintf(szFamily, _T("%s%s, "), szFamily, FBALoadStringEx(hAppInst, IDS_FAMILY_SAMSHO, true));
		}
		
		if (nFamily & FBF_19XX) {
			_stprintf(szFamily, _T("%s%s, "), szFamily, FBALoadStringEx(hAppInst, IDS_FAMILY_19XX, true));
		}
		
		if (nFamily & FBF_SONICWI) {
			_stprintf(szFamily, _T("%s%s, "), szFamily, FBALoadStringEx(hAppInst, IDS_FAMILY_SONICWI, true));
		}
		
		if (nFamily & FBF_PWRINST) {
			_stprintf(szFamily, _T("%s%s, "), szFamily, FBALoadStringEx(hAppInst, IDS_FAMILY_PWRINST, true));
		}		
		
		szFamily[_tcslen(szFamily) - 2] = _T(')');
		szFamily[_tcslen(szFamily) - 1] = _T('\0');
	}
	
	_stprintf(szDecoratedGenre, _T("%s%s"), szDecoratedGenre, szFamily);
	
	return szDecoratedGenre;
}

// ---------------------------------------------------------------------------
// config file parsing

TCHAR* LabelCheck(TCHAR* s, TCHAR* pszLabel)
{
	int nLen;
	if (s == NULL) {
		return NULL;
	}
	if (pszLabel == NULL) {
		return NULL;
	}
	nLen = _tcslen(pszLabel);

	SKIP_WS(s);													// Skip whitespace

	if (_tcsncmp(s, pszLabel, nLen)){							// Doesn't match
		return NULL;
	}
	return s + nLen;
}

int QuoteRead(TCHAR** ppszQuote, TCHAR** ppszEnd, TCHAR* pszSrc)	// Read a (quoted) string from szSrc and point to the end
{
	static TCHAR szQuote[QUOTE_MAX];
	TCHAR* s = pszSrc;
	TCHAR* e;

	// Skip whitespace
	SKIP_WS(s);

	e = s;

	if (*s == _T('\"')) {										// Quoted string
		s++;
		e++;
		// Find end quote
		FIND_QT(e);
		_tcsncpy(szQuote, s, e - s);
		// Zero-terminate
		szQuote[e - s] = _T('\0');
		e++;
	} else {													// Non-quoted string
		// Find whitespace
		FIND_WS(e);
		_tcsncpy(szQuote, s, e - s);
		// Zero-terminate
		szQuote[e - s] = _T('\0');
	}

	if (ppszQuote) {
		*ppszQuote = szQuote;
	}
	if (ppszEnd)	{
		*ppszEnd = e;
	}

	return 0;
}

