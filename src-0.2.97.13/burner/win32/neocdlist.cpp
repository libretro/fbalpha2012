// ---------------------------------------------------------------------------------------
// NeoGeo CD Game Info Module (by CaptainCPS-X)
// ---------------------------------------------------------------------------------------
#include "burner.h"
#include "neocdlist.h"

struct NGCDGAME games[] = 
{
// ---------------------------------------------------------------------------------------------------------------------------------------------//
//	* Name				* Title														* Year			* Company					* Game ID		//
// ---------------------------------------------------------------------------------------------------------------------------------------------//
	{ _T("nam1975")		, _T("NAM-1975")											, _T("1990")	, _T("SNK")					, 0x0001 },		//
	{ _T("bstars")		, _T("Baseball Stars Professional")							, _T("1991")	, _T("SNK")					, 0x0002 },		//
	{ _T("tpgolf")		, _T("Top Player's Golf")									, _T("1990")	, _T("SNK")					, 0x0003 },		//
	{ _T("mahretsu")	, _T("Mahjong Kyo Retsuden - Nishi Nihon Hen")				, _T("1990")	, _T("SNK")					, 0x0004 },		//
	{ _T("maglord")		, _T("Magician Lord")										, _T("1990")	, _T("ADK")					, 0x0005 },		//
	{ _T("ridhero")		, _T("Riding Hero")											, _T("1991")	, _T("SNK")					, 0x0006 },		//
	{ _T("alpham2")		, _T("Alpha Mission II / ASO II - Last Guardian")			, _T("1994")	, _T("SNK")					, 0x0007 },		//
	{ _T("ncombat")		, _T("Ninja Combat")										, _T("1990")	, _T("SNK/ADK")				, 0x0009 },		//
	{ _T("cyberlip")	, _T("Cyber-Lip")											, _T("1991")	, _T("SNK")					, 0x0010 },		//
	{ _T("superspy")	, _T("The Super Spy")										, _T("1990")	, _T("SNK")					, 0x0011 },		//
	{ _T("mutnat")		, _T("Mutation Nation")										, _T("1995")	, _T("SNK")					, 0x0014 },		//
	{ _T("sengoku")		, _T("Sengoku / Sengoku Denshou")							, _T("1994")	, _T("SNK")					, 0x0017 },		//
	{ _T("burningf")	, _T("Burning Fight")										, _T("1994")	, _T("SNK")					, 0x0018 },		//
	{ _T("lbowling")	, _T("League Bowling")										, _T("1994")	, _T("SNK")					, 0x0019 },		//
	{ _T("gpilots")		, _T("Ghost Pilots")										, _T("1991")	, _T("SNK")					, 0x0020 },		//
	{ _T("joyjoy")		, _T("Puzzled / Joy Joy Kid")								, _T("1990")	, _T("SNK")					, 0x0021 },		//
	{ _T("bjourney")	, _T("Blue's Journey / Raguy")								, _T("1990")	, _T("SNK/ADK")				, 0x0022 },		//
	{ _T("lresort")		, _T("Last Resort")											, _T("1994")	, _T("SNK")					, 0x0024 },		//
	{ _T("2020bb")		, _T("2020 Super Baseball")									, _T("1994")	, _T("SNK")					, 0x0030 },		//
	{ _T("socbrawl")	, _T("Soccer Brawl")										, _T("1991")	, _T("SNK")					, 0x0031 },		//
	{ _T("roboarmy")	, _T("Robo Army")											, _T("1991")	, _T("SNK")					, 0x0032 },		//
	{ _T("fatfury")		, _T("Fatal Fury - The Battle of Fury")						, _T("1994")	, _T("SNK")					, 0x0033 },		//
	{ _T("fbfrenzy")	, _T("Football Frenzy")										, _T("1994")	, _T("SNK")					, 0x0034 },		//
	{ _T("crswords")	, _T("Crossed Swords")										, _T("1994")	, _T("SNK/ADK")				, 0x0037 },		//
	{ _T("rallych")		, _T("Rally Chase")											, _T("1991")	, _T("SNK/ADK")				, 0x0038 },		//
	{ _T("kotm2")		, _T("King of the Monsters 2")								, _T("1992")	, _T("SNK")					, 0x0039 },		//
	{ _T("sengoku2")	, _T("Sengoku 2 / Sengoku Denshou 2")						, _T("1994")	, _T("SNK")					, 0x0040 },		//
	{ _T("bstars2")		, _T("Baseball Stars 2")									, _T("1992")	, _T("SNK")					, 0x0041 },		//
	{ _T("3countb")		, _T("3 Count Bout / Fire Suplex")							, _T("1995")	, _T("SNK")					, 0x0043 },		//
	{ _T("aof")			, _T("Art of Fighting / Ryuuko no Ken")						, _T("1994")	, _T("SNK")					, 0x0044 },		//
	{ _T("samsho")		, _T("Samurai Shodown / Samurai Spirits")					, _T("1993")	, _T("SNK")					, 0x0045 },		//
	{ _T("tophuntr")	, _T("Top Hunter - Roddy & Cathy")							, _T("1994")	, _T("SNK")					, 0x0046 },		//
	{ _T("fatfury2")	, _T("Fatal Fury 2 / Garou Densetsu 2 - Aratanaru Tatakai")	, _T("1994")	, _T("SNK")					, 0x0047 },		//
	{ _T("janshin")		, _T("Janshin Densetsu - Quest of the Jongmaster")			, _T("1995")	, _T("Yubis")				, 0x0048 },		//
	{ _T("ncommand")	, _T("Ninja Commando")										, _T("1992")	, _T("SNK")					, 0x0050 },		//
	{ _T("viewpoin")	, _T("Viewpoint")											, _T("1992")	, _T("Sammy/Aicom")			, 0x0051 },		//
	{ _T("ssideki")		, _T("Super Sidekicks / Tokuten Oh")						, _T("1993")	, _T("SNK")					, 0x0052 },		//
	{ _T("wh1")			, _T("World Heroes")										, _T("1992")	, _T("ADK")					, 0x0053 },		//
	{ _T("crsword2")	, _T("Crossed Swords II")									, _T("1995")	, _T("SNK/ADK")				, 0x0054 },		//
	{ _T("kof94")		, _T("The King of Fighters '94")							, _T("1994")	, _T("SNK")					, 0x0055 },		//
	{ _T("aof2")		, _T("Art of Fighting 2 / Ryuuko no Ken 2")					, _T("1994")	, _T("SNK")					, 0x0056 },		//
	{ _T("wh2")			, _T("World Heroes 2")										, _T("1995")	, _T("SNK/ADK")				, 0x0057 },		//
	{ _T("fatfursp")	, _T("Fatal Fury Special / Garou Densetsu Special")			, _T("1994")	, _T("SNK")					, 0x0058 },		//
	{ _T("savagere")	, _T("Savage Reign / Fu'un Mokujiroku - Kakutou Sousei")	, _T("1995")	, _T("SNK")					, 0x0059 },		//
	{ _T("ssideki2")	, _T("Super Sidekicks 2 / Tokuten Oh 2")					, _T("1994")	, _T("SNK")					, 0x0061 },		//
	{ _T("samsho2")		, _T("Samurai Shodown 2 / Shin Samurai Spirits")			, _T("1994")	, _T("SNK")					, 0x0063 },		//
	{ _T("wh2j")		, _T("World Heroes 2")										, _T("1995")	, _T("SNK/ADK")				, 0x0064 },		//
	{ _T("wjammers")	, _T("Windjammers / Flying Power Disc")						, _T("1994")	, _T("Data East")			, 0x0065 },		//
	{ _T("karnovr")		, _T("Karnov's Revenge / Fighters History Dynamite")		, _T("1994")	, _T("Data East")			, 0x0066 },		//
	{ _T("pspikes2")	, _T("Power Spikes II")										, _T("1994")	, _T("Video System")		, 0x0068 },		//
	{ _T("aodk")		, _T("Aggressors of Dark Kombat / Tsuukai GanGan Koushinkyoku")	, _T("1994"), _T("SNK/ADK")				, 0x0074 },		//
	{ _T("sonicwi2")	, _T("Aero Fighters 2 / Sonic Wings 2")						, _T("1994")	, _T("Video System")		, 0x0075 },		//
	{ _T("galaxyfg")	, _T("Galaxy Fight - Universal Warriors")					, _T("1995")	, _T("Sunsoft")				, 0x0078 },		//
	{ _T("strhoop")		, _T("Street Hoop / Dunk Dream")							, _T("1994")	, _T("Data East")			, 0x0079 },		//
	{ _T("quizkof")		, _T("Quiz King of Fighters")								, _T("1993")	, _T("SNK/Saurus")			, 0x0080 },		//
	{ _T("ssideki3")	, _T("Super Sidekicks 3 - The Next Glory / Tokuten Oh 3 - Eikoue No Chousen"), _T("1995") , _T("SNK")	, 0x0081 },		//
	{ _T("doubledr")	, _T("Double Dragon")										, _T("1995")	, _T("Technos")				, 0x0082 },		//
	{ _T("pbobblen")	, _T("Puzzle Bobble / Bust-A-Move")							, _T("1994")	, _T("SNK")					, 0x0083 },		//
	{ _T("kof95")		, _T("The King of Fighters '95")							, _T("1995")	, _T("SNK")					, 0x0084 },		//
	{ _T("ssrpg")		, _T("Shinsetsu Samurai Spirits - Bushidohretsuden")		, _T("1997")	, _T("SNK")					, 0x0085 },		//
	{ _T("samsho3")		, _T("Samurai Shodown 3 / Samurai Spirits 3")				, _T("1995")	, _T("SNK")					, 0x0087 },		//
	{ _T("stakwin")		, _T("Stakes Winner - GI Kanzen Seiha Heno Machi")			, _T("1995")	, _T("Saurus")				, 0x0088 },		//
	{ _T("pulstar")		, _T("Pulstar")												, _T("1995")	, _T("Aicom")				, 0x0089 },		//
	{ _T("whp")			, _T("World Heroes Perfect")								, _T("1995")	, _T("ADK")					, 0x0090 },		//
	{ _T("kabukikl")	, _T("Kabuki Klash / Tengai Makyou Shinden - Far East of Eden")	, _T("1995"), _T("Hudson")				, 0x0092 },		//
	{ _T("gowcaizr")	, _T("Voltage Fighter Gowcaizer / Choujin Gakuen Gowcaizer"), _T("1995")	, _T("Technos")				, 0x0094 },		//
	{ _T("rbff1")		, _T("Real Bout Fatal Fury")								, _T("1995")	, _T("SNK")					, 0x0095 },		//
	{ _T("aof3")		, _T("Art of Fighting 3: Path of the Warrior")				, _T("1996")	, _T("SNK")					, 0x0096 },		//
	{ _T("sonicwi3")	, _T("Aero Fighters 3 / Sonic Wings 3")						, _T("1995")	, _T("SNK")					, 0x0097 },		//
	{ _T("fromanc2")	, _T("Idol Mahjong Final Romance 2")						, _T("1995")	, _T("Video Systems")		, 0x0098 },		//
	{ _T("turfmast")	, _T("Neo Turf Masters / Big Tournament Golf")				, _T("1996")	, _T("Nazca")				, 0x0200 },		//
	{ _T("mslug")		, _T("Metal Slug - Super Vehicle-001")						, _T("1996")	, _T("Nazca")				, 0x0201 },		//
	{ _T("mosyougi")	, _T("Shougi no Tatsujin - Master of Syougi")				, _T("1995")	, _T("ADK")					, 0x0203 },		//
	{ _T("adkworld")	, _T("ADK World / ADK Special")								, _T("1995")	, _T("ADK")					, 0x0204 },		//
	{ _T("ngcdsp")		, _T("Neo Geo CD Special")									, _T("1995")	, _T("SNK")					, 0x0205 },		//
	{ _T("zintrick")	, _T("Zintrick / Oshidashi Zintrick")						, _T("1996")	, _T("ADK")					, 0x0211 },		//
	{ _T("overtop")		, _T("OverTop")												, _T("1996")	, _T("ADK")					, 0x0212 },		//
	{ _T("neodrift")	, _T("Neo DriftOut")										, _T("1996")	, _T("Visco")				, 0x0213 },		//
	{ _T("kof96")		, _T("The King of Fighters '96")							, _T("1996")	, _T("SNK")					, 0x0214 },		//
	{ _T("ninjamas")	, _T("Ninja Master's - Haou Ninpou-Chou")					, _T("1996")	, _T("ADK/SNK")				, 0x0217 },		//
	{ _T("ragnagrd")	, _T("Ragnagard / Shinouken")								, _T("1996")	, _T("Saurus")				, 0x0218 },		//
	{ _T("pgoal")		, _T("Pleasuregoal - 5 on 5 Mini Soccer / Futsal")			, _T("1996")	, _T("Saurus")				, 0x0219 },		//
	{ _T("ironclad")	, _T("Ironclad / Choutetsu Brikin'ger")						, _T("1996")	, _T("Saurus")				, 0x0220 },		//
	{ _T("magdrop2")	, _T("Magical Drop 2")										, _T("1996")	, _T("Data East")			, 0x0221 },		//
	{ _T("samsho4")		, _T("Samurai Shodown IV - Amakusa's Revenge")				, _T("1996")	, _T("SNK")					, 0x0222 },		//
	{ _T("rbffspec")	, _T("Real Bout Fatal Fury Special")						, _T("1996")	, _T("SNK")					, 0x0223 },		//
	{ _T("twinspri")	, _T("Twinkle Star Sprites")								, _T("1996")	, _T("ADK")					, 0x0224 },		//
	{ _T("kof96ngc")	, _T("The King of Fighters '96 NEOGEO Collection")			, _T("1996")	, _T("SNK")					, 0x0229 },		//
	{ _T("breakers")	, _T("Breakers")											, _T("1996")	, _T("Visco")				, 0x0230 },		//
	{ _T("kof97")		, _T("The King of Fighters '97")							, _T("1997")	, _T("SNK")					, 0x0232 },		//
	{ _T("lastblad")	, _T("The Last Blade / Bakumatsu Roman - Gekka no Kenshi")	, _T("1997")	, _T("SNK")					, 0x0234 },		//
	{ _T("rbff2")		, _T("Real Bout Fatal Fury 2 / Garou Densetsu 2 - Aratanaru Tatakai"), _T("1998"), _T("SNK")			, 0x0240 },		//
	{ _T("mslug2")		, _T("Metal Slug 2 - Super Vehicle-001/II")					, _T("1998")	, _T("SNK")					, 0x0241 },		//
	{ _T("kof98")		, _T("The King of Fighters '98 - The Slugfest")				, _T("1998")	, _T("SNK")					, 0x0242 },		//
	{ _T("lastbld2")	, _T("The Last Blade 2")									, _T("1998")	, _T("SNK")					, 0x0243 },		//
	{ _T("kof99")		, _T("The King of Fighters '99 - Millennium Battle")		, _T("1999")	, _T("SNK")					, 0x0251 },		//
	{ _T("fatfury3")	, _T("Fatal Fury 3 - Road to the Final Victory / Garou Densetsu 3 - Harukanaru Tatakai"), _T("1995"), _T("SNK"), 0x069c },		//
// ---------------------------------------------------------------------------------------------------------------------------------------------//
//	* There is a total of 97 Games on the Master List																							//
// ---------------------------------------------------------------------------------------------------------------------------------------------//
};

NGCDGAME* GetNeoGeoCDInfo(int nID)
{
	for(int nGame = 0; nGame < 97; nGame++) {
		if( nID == games[nGame].id ) {
			return &games[nGame];
		}
	}

	return NULL;
}

// Update the main window title
void SetNeoCDTitle(TCHAR* pszTitle) 
{
	TCHAR szText[1024] = _T("");
	_stprintf(szText, _T(APP_TITLE) _T( " v%.20s") _T(SEPERATOR_1) _T("%s") _T(SEPERATOR_1) _T("%s"), szAppBurnVer, BurnDrvGetText(DRV_FULLNAME), pszTitle);
	
	SetWindowText(hScrnWnd, szText);
}

NGCDGAME* game;

// Get the title 
int GetNeoCDTitle(unsigned int nGameID) 
{
	game = (NGCDGAME*)malloc(sizeof(NGCDGAME));
	memset(game, 0, sizeof(NGCDGAME));
	
	if(GetNeoGeoCDInfo(nGameID))
	{		
		memcpy(game, GetNeoGeoCDInfo(nGameID), sizeof(NGCDGAME));

		bprintf(PRINT_NORMAL, _T("    Title: %s \n")		, game->pszTitle);
		bprintf(PRINT_NORMAL, _T("    Shortname: %s \n")	, game->pszName);
		bprintf(PRINT_NORMAL, _T("    Year: %s \n")			, game->pszYear);
		bprintf(PRINT_NORMAL, _T("    Company: %s \n")		, game->pszCompany);
		
		// Update the main window title
		SetNeoCDTitle(game->pszTitle);

		return 1;
	}

	game = NULL;
	return 0;
}

// Read ISO and look for 68K ROM standard program header, ID is always there
// Note: This function works very quick, doesn't compromise performance :)
// it just read each sector first 264 bytes aproximately only.
int GetIso68KRomID(FILE* fp)
{
	bprintf(PRINT_NORMAL, _T("    Reading Neo-Geo CD ISO... \n"));

	// Get ISO Size (bytes)
	fseek(fp, 0, SEEK_END);
	long lSize = 0;
	lSize = ftell(fp);
	rewind(fp);

	// If it has at least 16 sectors proceed
	if(lSize > (2048 * 16)) 
	{	
		// Check for Standard ISO9660 Identifier
		unsigned char IsoHeader[2048 * 16 + 1];
		unsigned char IsoCheck[6];
		
		fread(IsoHeader, 1, 2048 * 16 + 1, fp); // advance to sector 16 and PVD Field 2
		fread(IsoCheck, 1, 5, fp);	// get Standard Identifier Field from PVD
		
		// Verify that we have indeed a valid ISO9660 MODE1/2048
		if(!memcmp(IsoCheck, "CD001", 5))
		{
			bprintf(PRINT_NORMAL, _T("    Standard ISO9660 Identifier Found. \n"));			
			// ISO is good! rewind the file pointer :)
			rewind(fp); 
		} else {

			bprintf(PRINT_NORMAL, _T("    Standard ISO9660 Identifier Not Found, cannot continue. \n"));
			return 0;
		}

		// Calculate total ISO sectors
		long lSectors = lSize / 2048;

		// Allocate 2048 bytes to hold one sector data 
		unsigned char* pSectorData = (unsigned char*)malloc(sizeof(unsigned char) * 2048);
		
		if(pSectorData) 
		{
			bprintf(PRINT_NORMAL, _T("    Reading CD image data sectors for 68K ROM header... \n"));

			// Look for the main 68K ROM Program Header
			for(int nSector = 0; nSector < lSectors; nSector++) 
			{
				fread(pSectorData, 1, 2048, fp);

				// Check for standard .PRG header
				if (pSectorData[0x100] == 'N' && 
					pSectorData[0x101] == 'E' && 
					pSectorData[0x102] == 'O' &&
					pSectorData[0x103] == '-' &&
					pSectorData[0x104] == 'G' && 
					pSectorData[0x105] == 'E' && 
					pSectorData[0x106] == 'O' )
				{	
					bprintf(PRINT_NORMAL, _T("    CD image has a valid 68K ROM header, game info has been adquired. \n"));

					unsigned int nNeoGeoCD_ID	= 0;
					char szValue[] = "0000";

					// Get the NeoGeo CD game ID field
					sprintf(szValue, "%02X%02X", pSectorData[0x108], pSectorData[0x109]);					
					sscanf(szValue, "%X", &nNeoGeoCD_ID);

					bprintf(PRINT_NORMAL, _T("    Found Game ID: 0x%04X at Sector %d \n\n"), nNeoGeoCD_ID, nSector);

					// done! =D, now get all the info!
					GetNeoCDTitle(nNeoGeoCD_ID);

					return 1;
				}
			}
		}
	}
	bprintf(PRINT_NORMAL, _T("    CD image doesn't have a valid 68K ROM header. \n"));
	return 0;
}

// This will do everything
int GetNeoGeoCD_Identifier()
{
	if(!GetIsoPath() || !IsNeoGeoCD()) {
		return 0;
	}

	// Make sure we have a valid ISO file extension...
	if(_tcsstr(GetIsoPath(), _T(".iso")) || _tcsstr(GetIsoPath(), _T(".ISO")) ) 
	{
		FILE* fp = _tfopen(GetIsoPath(), _T("rb"));
		if(fp) 
		{
			// Read ISO and look for 68K ROM standard program header, ID is always there
			// Note: This function works very quick, doesn't compromise performance :)
			// it just read each sector first 264 bytes aproximately only.
			GetIso68KRomID(fp);

		} else {

			bprintf(PRINT_NORMAL, _T("    Couldn't open %s \n"), GetIsoPath());
			return 0;
		}

		if(fp) fclose(fp);

	} else {

		bprintf(PRINT_NORMAL, _T("    File doesn't have a valid ISO extension [ .iso / .ISO ] \n"));
		return 0;
	}

	return 1;
}

int NeoCDInfo_Init() 
{
	NeoCDInfo_Exit();
	return GetNeoGeoCD_Identifier();
}

TCHAR* NeoCDInfo_Text(int nText)
{
	if(!game || !IsNeoGeoCD() || !bDrvOkay) return NULL;

	switch(nText) 
	{
		case DRV_NAME:			return game->pszName;
		case DRV_FULLNAME:		return game->pszTitle;
		case DRV_MANUFACTURER:	return game->pszCompany;
		case DRV_DATE:			return game->pszYear;
	}

	return NULL;
}

int NeoCDInfo_ID() 
{
	if(!game || !IsNeoGeoCD() || !bDrvOkay) return 0;
	return game->id;
}

void NeoCDInfo_Exit() 
{
	if(game) {
		free(game);
		game = NULL;
	}
}
