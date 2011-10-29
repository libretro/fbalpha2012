#ifndef _neocdlist_
#define _neocdlist_

struct NGCDGAME
{
	TCHAR*	pszName;		// Short name
	TCHAR*	pszTitle;		// Title
	TCHAR*	pszYear;		// Release Year
	TCHAR*	pszCompany;		// Developer
	int		id;				// Game ID
};

NGCDGAME*	GetNeoGeoCDInfo(int nID);
int			GetNeoCDTitle(unsigned int nGameID);
void		SetNeoCDTitle(TCHAR* pszTitle); 
int			GetIso68KRomID(FILE* fp);
int			GetNeoGeoCD_Identifier();

bool	IsNeoGeoCD();		// neo_run.cpp
TCHAR*	GetIsoPath();	// cd_isowav.cpp

int		NeoCDInfo_Init();
TCHAR*	NeoCDInfo_Text(int nText);
int		NeoCDInfo_ID();
void	NeoCDInfo_Exit();


#endif
