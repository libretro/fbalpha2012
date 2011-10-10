// Burner data file module (for ROM managers)
// written    2001 LoqiqX
// updated 11/2003 by LvR    -- essentially a rewrite
// updated    2008 by regret -- support clrmamepro xml dat

#include "burner.h"
#include "../libs/ticpp/ticpp.h"

// ==> write clrmamepro xml dat, added by regret
#ifdef TIXML_USE_TICPP
int write_xmlfile(const char* szFilename, FILE* file)
{
		char str[128];
		sprintf(str, "%s v%.20s", APP_TITLE, szAppBurnVer);

		// root
		ticpp::Document doc;
		ticpp::Declaration decl("1.0", "", "");
		doc.LinkEndChild(&decl);
		ticpp::Element root("datafile");
		root.SetAttribute("build", str);
		root.SetAttribute("debug", "no");
		doc.LinkEndChild(&root);

		// header
		ticpp::Element header("header");
		ticpp::Element name("name", APP_TITLE);
		ticpp::Element desc("description", str);
		ticpp::Element category("category", APP_DESCRIPTION);
		ticpp::Element version("version", szAppBurnVer);
		//		ticpp::Element data("data", MAKE_STRING(BUILD_DATE));
		header.LinkEndChild(&name);
		header.LinkEndChild(&desc);
		header.LinkEndChild(&category);
		header.LinkEndChild(&version);
		//		header.LinkEndChild(&data);
		root.LinkEndChild(&header);

		// game
		int nRet = 0;
		unsigned int nOldSelect = nBurnDrvSelect;				// preserve the currently selected driver
		unsigned int nParentSelect, nBoardROMSelect;

		char sgName[MAX_PATH];
		char spName[MAX_PATH];
		char sbName[MAX_PATH];
		unsigned int i = 0;
		int nPass = 0;

		// Go over each of the games
		for (unsigned int nGameSelect = 0; nGameSelect < nBurnDrvCount; nGameSelect++)
		{
			nBurnDrvSelect = nGameSelect; 					// Switch to driver nGameSelect

			if (BurnDrvGetFlags() & BDF_BOARDROM)
				continue;

			strcpy(sgName, BurnDrvGetTextA(DRV_NAME));
			strcpy(spName, ""); 						// make sure this string is empty before we start
			strcpy(sbName, ""); 						// make sure this string is empty before we start

			// Check to see if the game has a parent
			if (BurnDrvGetTextA(DRV_PARENT))
			{
				while (BurnDrvGetTextA(DRV_PARENT))
				{
					strcpy(spName, BurnDrvGetTextA(DRV_PARENT));
					nParentSelect = BurnDrvGetIndexByNameA(spName);
					nBurnDrvSelect = nParentSelect;
					if (nParentSelect >= nBurnDrvCount)
					{
						nParentSelect = -1U;
						break;
					}
				}

				nBurnDrvSelect = nGameSelect; 				// restore driver select
			}
			else
				nParentSelect = nGameSelect;

			// Check to see if the game has a BoardROM
			if (BurnDrvGetTextA(DRV_BOARDROM)) {
				strcpy(sbName, BurnDrvGetTextA(DRV_BOARDROM));
				nBoardROMSelect = BurnDrvGetIndexByNameA(sbName);
				if (nBoardROMSelect >= nBurnDrvCount)
					nBoardROMSelect = -1U;

				nBurnDrvSelect = nGameSelect; 				// restore driver select
			}
			else
				nBoardROMSelect = nGameSelect;

			// Report problems
			//			if (nParentSelect == -1U) {
			//				fprintf(file, "# Missing parent %s. It needs to be added to " APP_TITLE "!\n\n", spName);
			//			}
			//			if (nBoardROMSelect == -1U) {
			//				fprintf(file, "# Missing boardROM %s. It needs to be added to " APP_TITLE "!\n\n", sbName);
			//			}

			// Write the header
			ticpp::Element game("game");
			game.SetAttribute("name", sgName);

			if (nParentSelect != nGameSelect && nParentSelect != -1U)
			{
				game.SetAttribute("cloneof", spName);
				game.SetAttribute("romof", spName);
			}
			else
			{
				// Add "romof" (but not 'cloneof') line for games that have boardROMs
				if (nBoardROMSelect != nGameSelect && nBoardROMSelect != -1U)
					game.SetAttribute("romof", sbName);
			}

			ticpp::Element gamedesc("description", decorateGameName(nBurnDrvSelect));
			ticpp::Element gameyear("year", BurnDrvGetTextA(DRV_DATE));
			ticpp::Element gamemanuf("manufacturer", BurnDrvGetTextA(DRV_MANUFACTURER));
			game.LinkEndChild(&gamedesc);
			game.LinkEndChild(&gameyear);
			game.LinkEndChild(&gamemanuf);

			// Write the individual ROM info
			for (nPass = 0; nPass < 2; nPass++) {
				nBurnDrvSelect = nGameSelect;

				// Skip pass 0 if possible
				if (nPass == 0 && (nBoardROMSelect == nGameSelect || nBoardROMSelect == -1U))
					continue;

				// Go over each of the files needed for this game (upto 0x0100)
				for (i = 0, nRet = 0; nRet == 0 && i < 0x100; i++)
				{
					int nRetTmp = 0;
					struct BurnRomInfo ri;
					int nLen; unsigned int nCrc;
					char *szPossibleName = NULL;
					int j, nMerged = 0;

					memset(&ri, 0, sizeof(ri));

					// Get info on this file
					nBurnDrvSelect = nGameSelect;
					nRet = BurnDrvGetRomInfo(&ri, i);
					nRet += BurnDrvGetRomName(&szPossibleName, i, 0);

					if (ri.nLen == 0) continue;

					if (nRet == 0) {
						struct BurnRomInfo riTmp;
						char *szPossibleNameTmp;
						nLen = ri.nLen; nCrc = ri.nCrc;

						// Check for files from boardROMs
						if (nBoardROMSelect != nGameSelect && nBoardROMSelect != -1U) {
							nBurnDrvSelect = nBoardROMSelect;
							nRetTmp = 0;

							// Go over each of the files needed for this game (upto 0x0100)
							for (j = 0; nRetTmp == 0 && j < 0x100; j++) {
								memset(&riTmp, 0, sizeof(riTmp));

								nRetTmp += BurnDrvGetRomInfo(&riTmp, j);
								nRetTmp += BurnDrvGetRomName(&szPossibleNameTmp, j, 0);

								if (nRetTmp == 0)
								{
									if (riTmp.nLen && riTmp.nCrc == nCrc && !strcmp(szPossibleName, szPossibleNameTmp))
									{
										// This file is from a boardROM
										nMerged |= 2;
										nRetTmp++;
									}
								}
							}
						}

						if (!nMerged && nParentSelect != nGameSelect && nParentSelect != -1U)
						{
							nBurnDrvSelect = nParentSelect;
							nRetTmp = 0;

							// Go over each of the files needed for this game (upto 0x0100)
							for (j = 0; nRetTmp == 0 && j < 0x100; j++)
							{
								memset(&riTmp, 0, sizeof(riTmp));

								nRetTmp += BurnDrvGetRomInfo(&riTmp, j);
								nRetTmp += BurnDrvGetRomName(&szPossibleNameTmp, j, 0);

								if (nRetTmp==0) {
									if (riTmp.nLen && riTmp.nCrc == nCrc && !strcmp(szPossibleName, szPossibleNameTmp)) {
										// This file is from a parent set
										nMerged |= 1;
										nRetTmp++;
									}
								}
							}
						}

						nBurnDrvSelect = nGameSelect; 				// Switch back to game
					}

					ticpp::Element rom("rom");
					bool addNode = false;

					// Selectable BIOS meta info
					if (nPass == 0 && (nMerged & 2) && (ri.nType & BRF_SELECT))
					{
						//fprintf(file, "\tbiosset ( name %d description \"%s\" %s)\n", i - 128, szPossibleName, ri.nType & BRF_OPT ? "" : "default yes ");
					}
					// File info
					if (nPass == 1 && !nMerged)
					{
						rom.SetAttribute("name", szPossibleName);
						rom.SetAttribute("size", ri.nLen);

						if (ri.nType & BRF_NODUMP)
							rom.SetAttribute("status", "nodump");
						else
						{
							sprintf(str, "%08x", ri.nCrc);
							rom.SetAttribute("crc", str);
						}
						addNode = true;
					}
					if (nPass == 1 && nMerged) {
						rom.SetAttribute("name", szPossibleName);
						rom.SetAttribute("merge", szPossibleName);
						rom.SetAttribute("size", ri.nLen);

						// Selectable BIOS file info
						if (nMerged & 2 && ri.nType & BRF_SELECT)
						{
							sprintf(str, "%08x", ri.nCrc);
							rom.SetAttribute("crc", str);
						}
						// Files from parent/boardROMs
						else {
							if (ri.nType & BRF_NODUMP)
								rom.SetAttribute("status", "nodump");
							else
							{
								sprintf(str, "%08x", ri.nCrc);
								rom.SetAttribute("crc", str);
							}
						}
						addNode = true;
					}

					if (addNode)
						game.LinkEndChild(&rom);
				}
			}

			root.LinkEndChild(&game);
		}

		// Do another pass over each of the games to find boardROMs
		for (nBurnDrvSelect = 0; nBurnDrvSelect < nBurnDrvCount; nBurnDrvSelect++)
		{
			if (!(BurnDrvGetFlags() & BDF_BOARDROM))
				continue;

			ticpp::Element game("game");
			game.SetAttribute("isbios", "yes");
			game.SetAttribute("name", BurnDrvGetTextA(DRV_NAME));
			ticpp::Element gamedesc("description", decorateGameName(nBurnDrvSelect));
			ticpp::Element gameyear("year", BurnDrvGetTextA(DRV_DATE));
			ticpp::Element gamemanuf("manufacturer", BurnDrvGetTextA(DRV_COMMENT));
			game.LinkEndChild(&gamedesc);
			game.LinkEndChild(&gameyear);
			game.LinkEndChild(&gamemanuf);

			for (int nPass = 0; nPass < 2; nPass++)
			{
				// Go over each of the individual files (upto 0x0100)
				for (int i = 0; i < 0x100; i++)
				{
					struct BurnRomInfo ri;
					char *szPossibleName = NULL;

					memset(&ri, 0, sizeof(ri));

					nRet = BurnDrvGetRomInfo(&ri, i);
					nRet += BurnDrvGetRomName(&szPossibleName, i, 0);

					if (ri.nLen == 0)
						continue;

					if (nRet == 0)
					{
						if (nPass != 0)
						{
							sprintf(str, "%08x", ri.nCrc);

							ticpp::Element rom("rom");
							rom.SetAttribute("name", szPossibleName);
							rom.SetAttribute("size", ri.nLen);
							rom.SetAttribute("crc", str);
							game.LinkEndChild(&rom);
						}
					}
				}
			}

			root.LinkEndChild(&game);
		}

		// Restore current driver
		nBurnDrvSelect = nOldSelect;

		// write xml
		if (file)
			doc.SaveFile(file);
		else
			doc.SaveFile(szFilename);

	return 0;
}
#endif

// <== write clrmamepro xml dat

int write_datfile(FILE* file)
{
	int nRet = 0;
	unsigned int nOldSelect = nBurnDrvSelect;						// preserve the currently selected driver
	unsigned int nParentSelect, nBoardROMSelect;

	char sgName[MAX_PATH];
	char spName[MAX_PATH];
	char sbName[MAX_PATH];
	unsigned int i = 0;
	int nPass = 0;

	// Go over each of the games
	for (unsigned int nGameSelect = 0;nGameSelect < nBurnDrvCount; nGameSelect++)
	{
		nBurnDrvSelect = nGameSelect;								// Switch to driver nGameSelect

		if (BurnDrvGetFlags() & BDF_BOARDROM)
			continue;

		strcpy(sgName, BurnDrvGetTextA(DRV_NAME));
		strcpy(spName, "");											// make sure this string is empty before we start
		strcpy(sbName, "");											// make sure this string is empty before we start

		// Check to see if the game has a parent
		if (BurnDrvGetTextA(DRV_PARENT))
		{
			while (BurnDrvGetTextA(DRV_PARENT))
			{
				strcpy(spName, BurnDrvGetTextA(DRV_PARENT));
				nParentSelect = BurnDrvGetIndexByNameA(spName);
				nBurnDrvSelect = nParentSelect;
				if (nParentSelect >= nBurnDrvCount) {
					nParentSelect = -1U;
					break;
				}
			}

			nBurnDrvSelect = nGameSelect;							// restore driver select
		}
		else
			nParentSelect = nGameSelect;

		// Check to see if the game has a BoardROM
		if (BurnDrvGetTextA(DRV_BOARDROM))
		{
			strcpy(sbName, BurnDrvGetTextA(DRV_BOARDROM));
			nBoardROMSelect = BurnDrvGetIndexByNameA(sbName);
			if (nBoardROMSelect >= nBurnDrvCount)
				nBoardROMSelect = -1U;

			nBurnDrvSelect = nGameSelect;							// restore driver select
		}
		else
			nBoardROMSelect = nGameSelect;

		// Report problems
		if (nParentSelect == -1U)
			fprintf(file, "# Missing parent %s. It needs to be added to " APP_TITLE "!\n\n", spName);
		if (nBoardROMSelect == -1U)
			fprintf(file, "# Missing boardROM %s. It needs to be added to " APP_TITLE "!\n\n", sbName);

		// Write the header
		fprintf(file, "game (\n");
		fprintf(file, "\tname %s\n", sgName);

		if (nParentSelect != nGameSelect && nParentSelect != -1U)
		{
			fprintf(file, "\tcloneof %s\n", spName);
			fprintf(file, "\tromof %s\n", spName);
		}
		else
		{
			// Add "romof" (but not 'cloneof') line for games that have boardROMs
			if (nBoardROMSelect != nGameSelect && nBoardROMSelect != -1U)
				fprintf(file, "\tromof %s\n", sbName);
		}

		fprintf(file, "\tdescription \"%s\"\n", decorateGameName(nBurnDrvSelect));
		fprintf(file, "\tyear %s\n", BurnDrvGetTextA(DRV_DATE));
		fprintf(file, "\tmanufacturer \"%s\"\n", BurnDrvGetTextA(DRV_MANUFACTURER));

		// Write the individual ROM info
		for (nPass = 0; nPass < 2; nPass++)
		{
			nBurnDrvSelect = nGameSelect;

			// Skip pass 0 if possible
			if (nPass == 0 && (nBoardROMSelect == nGameSelect || nBoardROMSelect == -1U))
				continue;

			// Go over each of the files needed for this game (upto 0x0100)
			for (i = 0, nRet = 0; nRet == 0 && i < 0x100; i++)
			{
				int nRetTmp = 0;
				struct BurnRomInfo ri;
				int nLen; unsigned int nCrc;
				char* szPossibleName = NULL;
				int j, nMerged = 0;

				memset(&ri, 0, sizeof(ri));

				// Get info on this file
				nBurnDrvSelect = nGameSelect;
				nRet = BurnDrvGetRomInfo(&ri, i);
				nRet += BurnDrvGetRomName(&szPossibleName, i, 0);

				if (ri.nLen == 0) continue;

				if (nRet == 0)
				{
					struct BurnRomInfo riTmp;
					char* szPossibleNameTmp;
					nLen = ri.nLen; nCrc = ri.nCrc;

					// Check for files from boardROMs
					if (nBoardROMSelect != nGameSelect && nBoardROMSelect != -1U) {
						nBurnDrvSelect = nBoardROMSelect;
						nRetTmp = 0;

						// Go over each of the files needed for this game (upto 0x0100)
						for (j = 0; nRetTmp == 0 && j < 0x100; j++)
						{
							memset(&riTmp, 0, sizeof(riTmp));

							nRetTmp += BurnDrvGetRomInfo(&riTmp, j);
							nRetTmp += BurnDrvGetRomName(&szPossibleNameTmp, j, 0);

							if (nRetTmp == 0)
							{
								if (riTmp.nLen && riTmp.nCrc == nCrc && !strcmp(szPossibleName, szPossibleNameTmp))
								{
									// This file is from a boardROM
									nMerged |= 2;
									nRetTmp++;
								}
							}
						}
					}

					if (!nMerged && nParentSelect != nGameSelect && nParentSelect != -1U) {
						nBurnDrvSelect = nParentSelect;
						nRetTmp = 0;

						// Go over each of the files needed for this game (upto 0x0100)
						for (j = 0; nRetTmp == 0 && j < 0x100; j++)
						{
							memset(&riTmp, 0, sizeof(riTmp));

							nRetTmp += BurnDrvGetRomInfo(&riTmp, j);
							nRetTmp += BurnDrvGetRomName(&szPossibleNameTmp, j, 0);

							if (nRetTmp == 0)
							{
								if (riTmp.nLen && riTmp.nCrc == nCrc && !strcmp(szPossibleName, szPossibleNameTmp))
								{
									// This file is from a parent set
									nMerged |= 1;
									nRetTmp++;
								}
							}
						}
					}

					nBurnDrvSelect = nGameSelect;					// Switch back to game
				}

				// Selectable BIOS meta info
				if (nPass == 0 && (nMerged & 2) && (ri.nType & BRF_SELECT))
					fprintf(file, "\tbiosset ( name %d description \"%s\" %s)\n", i - 128, szPossibleName, ri.nType & BRF_OPT ? "" : "default yes ");
				// File info
				if (nPass == 1 && !nMerged)
				{
					if (ri.nType & BRF_NODUMP)
						fprintf(file, "\trom ( name %s size %d flags nodump )\n", szPossibleName, ri.nLen);
					else
						fprintf(file, "\trom ( name %s size %d crc %08x )\n", szPossibleName, ri.nLen, ri.nCrc);
				}
				if (nPass == 1 && nMerged)
				{
					// Selectable BIOS file info
					if ((nMerged & 2) && (ri.nType & BRF_SELECT))
						fprintf(file, "\trom ( name %s merge %s bios %d size %d crc %08x )\n", szPossibleName, szPossibleName, i - 128, ri.nLen, ri.nCrc);
					// Files from parent/boardROMs
					else
					{
						if (ri.nType & BRF_NODUMP)
							fprintf(file, "\trom ( name %s merge %s size %d flags nodump )\n", szPossibleName, szPossibleName, ri.nLen);
						else
							fprintf(file, "\trom ( name %s merge %s size %d crc %08x )\n", szPossibleName, szPossibleName, ri.nLen, ri.nCrc);
					}
				}
			}
		}

		fprintf(file, ")\n\n");
	}

	// Do another pass over each of the games to find boardROMs
	for (nBurnDrvSelect = 0; nBurnDrvSelect < nBurnDrvCount; nBurnDrvSelect++)
	{
		int i, nPass;

		if (!(BurnDrvGetFlags() & BDF_BOARDROM))
			continue;

		fprintf(file, "resource (\n");
		fprintf(file, "\tname %s\n", BurnDrvGetTextA(DRV_NAME));
		fprintf(file, "\tdescription \"%s\"\n", decorateGameName(nBurnDrvSelect));
		fprintf(file, "\tyear %s\n", BurnDrvGetTextA(DRV_DATE));
		fprintf(file, "\tmanufacturer \"%s\"\n", BurnDrvGetTextA(DRV_COMMENT));

		for (nPass = 0; nPass < 2; nPass++)
		{
			// Go over each of the individual files (upto 0x0100)
			for (i = 0; i < 0x100; i++)
			{
				struct BurnRomInfo ri;
				char* szPossibleName = NULL;

				memset(&ri, 0, sizeof(ri));

				nRet = BurnDrvGetRomInfo(&ri, i);
				nRet += BurnDrvGetRomName(&szPossibleName, i, 0);

				if (ri.nLen == 0) continue;

				if (nRet == 0)
				{
					if (nPass == 0)
					{
						if (ri.nType & BRF_SELECT)
							fprintf(file, "\tbiosset ( name %d description \"%s\" %s)\n", i, szPossibleName, ri.nType & 0x80 ? "" : "default yes ");
					}
					else
					{
						if (ri.nType & BRF_SELECT)
							fprintf(file, "\trom ( name %s bios %d size %d crc %08x )\n", szPossibleName, i, ri.nLen, ri.nCrc);
						else
							fprintf(file, "\trom ( name %s size %d crc %08x )\n", szPossibleName, ri.nLen, ri.nCrc);
					}
				}
			}
		}

		fprintf(file, ")\n");
	}

	// Restore current driver
	nBurnDrvSelect = nOldSelect;

	return 0;
}

int create_datfile(char* szFilename, int type)
{
	#ifdef TIXML_USE_TICPP
	if (type == 1)
		return write_xmlfile(szFilename, NULL);	// write clrmamepro xml dat
	#endif

	FILE* file = fopen(szFilename, "w");
	if (!file)
		return 1;

	char* version = szAppBurnVer;

	if (type == 0)
	{
		fprintf(file, "clrmamepro (\n");
		fprintf(file, "\tname \"" APP_TITLE "\"\n");
		fprintf(file, "\tdescription \"" APP_TITLE " v%.20s\"\n", version);
		fprintf(file, "\tcategory \"" APP_DESCRIPTION "\"\n");
		fprintf(file, "\tversion %s\n", version);
		fprintf(file, "\tauthor \"" APP_TITLE " v%.20s\"\n", version);
		fprintf(file, "\tforcezipping zip\n");
		fprintf(file, ")\n\n");
	}

	int ret = write_datfile(file);

	fclose(file);

	return ret;
}
