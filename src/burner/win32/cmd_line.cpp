// process command line, added by regret
// based on cmd parser from gens-rerecording (http://code.google.com/p/gens-rerecording/)

/* changelog:
 update 2: add -ips
 update 1: create (ref: gens-rr)
*/

#include "burner.h"
#include "cmd_line.h"
#ifndef NO_IPS
#include "patch.h"
#endif

//To add additional commandline options
//1) add the identifier (-rom, -play, etc) into the argCmds array
//2) add a variable to store the argument in the list under "Strings that will get parsed"
//3) add an entry in the switch statement in order to assign the variable
//4) add code under the "execute commands" section to handle the given commandline

int processCmdLine(LPSTR lpCmdLine)
{
	if (!lpCmdLine || !lpCmdLine[0]) {
		return 0;
	}

	string argumentList;					//Complete command line argument
	argumentList.assign(lpCmdLine);			//Assign command line to argumentList
	size_t argLength = argumentList.size();	//Size of command line argument

	//List of valid commandline args
	string argCmds[] = { "-g", "-listxml", "-w", "-r", "-d", "-ips" };

	//Strings that will get parsed:
	string romToLoad = "";					// The game's romname
	bool listInfo = false;					// List game info
	bool runInWindow = false;				// Run in a window instead of fullscreen
	string resolution = "";					// Specify a resolution
	string depth = "";						// Specify a depth
	string ipsToLoad;						// Apply IPS
	bool useIPS = false;

	//Temps for finding string list
	bool findCmd = false;	// find commandline args flag
	size_t commandBegin = 0;	//Beginning of Command
	size_t commandEnd = 0;		//End of Command
	string newCommand;		//Will hold newest command being parsed in the loop
	string trunc;			//Truncated argList (from beginning of command to end of argumentList

	//--------------------------------------------------------------------------------------------
	//Commandline parsing loop
	for (int x = 0; x < sizearray(argCmds); x++)
	{
		findCmd = false;

		if (argumentList.find(argCmds[x]) != string::npos)
		{
			commandBegin = argumentList.find(argCmds[x]) + argCmds[x].size() + 1;	//Find beginning of new command
			if (commandBegin < argumentList.length())
			{
				trunc = argumentList.substr(commandBegin);							//Truncate argumentList
				commandEnd = trunc.find(" ");										//Find next space, if exists, new command will end here
				if (argumentList[commandBegin] == '\"')								//Actually, if it's in quotes, extend to the end quote
				{
					commandEnd = trunc.find('\"', 1);
					if (commandEnd >= 0)
						commandBegin++, commandEnd--;
				}
				if (commandEnd < 0) commandEnd = argLength;							//If no space, new command will end at the end of list
				newCommand = argumentList.substr(commandBegin, commandEnd);			//assign freshly parsed command to newCommand
			}
			findCmd = true;
		}
		else
		{
			newCommand = "";
		}

		//Assign newCommand to appropriate variable
		if (findCmd)
		{
			switch (x)
			{
				case 0:	// -game
					romToLoad = newCommand;
					break;
				case 1:	// -listinfo
					listInfo = true;
					break;
				case 2:	// -win
					runInWindow = true;
					break;
				case 3:	// -r
					resolution = newCommand;
					break;
				case 4:	// -d
					depth = newCommand;
					break;
				case 5:	// -ips
					useIPS = true;
					ipsToLoad = newCommand;
					break;
			}
		}
	}

	//--------------------------------------------------------------------------------------------
	//Execute commands

	if (listInfo) {
		write_xmlfile(NULL, stdout);
		return 1;
	}

	// game options
	bCmdOptUsed = 1;
	bool bFullscreen = true;
	if (runInWindow) {
		bCmdOptUsed = 0;
		bFullscreen = false;
	}
	if (bFullscreen) {
		nVidFullscreen = 1;
	}

	if (resolution[0]) {
		size_t xpos = resolution.find("x");
		if (xpos != string::npos) {
			nVidWidth = atoi(resolution.substr(0, xpos).c_str());
			nVidHeight = atoi(resolution.substr(xpos + 1).c_str());
		}
	}

	if (depth[0]) {
		nVidDepth = atoi(depth.c_str());
	}

	// load rom
	if (romToLoad[0]) {
		if (romToLoad.find(".fs") != string::npos) {
			if (BurnStateLoad(AtoW(romToLoad.c_str()), 1, &DrvInitCallback)) {
				return 1;
			} else {
//				bRunPause = 1;
			}
		} else {
			if (romToLoad.find(".fr") != string::npos) {
				if (StartReplay(AtoW(romToLoad.c_str()))) {
					return 1;
				}
			} else {
				unsigned int i = BurnDrvGetIndexByNameA(romToLoad.c_str());
				if (i < nBurnDrvCount) {
					nBurnDrvSelect = i;
#ifndef NO_IPS
					// apply ips
					if (useIPS) {
						if (ipsToLoad[0]) {
							// TODO: parse ips.dat
						}
						bDoPatch = true;
						loadActivePatches();
					}
#endif
					// run game
//					mediaInit();

					if (!_tcscmp(audSelect, _T("DirectSound"))) {
						scrnInit(); // fixme: only need for dsound
					}
					mediaReInitAudio();

					BurnerDrvInit(i, true);
				} else {
					FBAPopupAddText(PUF_TEXT_DEFAULT, MAKEINTRESOURCE(IDS_ERR_UI_NOSUPPORT), romToLoad.c_str(), _T(APP_TITLE));
					FBAPopupDisplay(PUF_TYPE_ERROR);
					return 1;
				}
			}
		}
	}

//	POST_INITIALISE_MESSAGE;

	if (!nVidFullscreen) {
		menuSync(MENUT_ALL);
	}

	return 0;
}

int parseCmdLine(LPSTR cmd)
{
#if 0
	if (!cmd || !cmd[0]) {
		return 0;
	}

#define	MAX_PARAMETERS 30

	char temp[1024];
	strcpy(temp, cmd);
	char* p = temp;

	// get parameters
	static char* parameters[MAX_PARAMETERS];
	int	count =	0;

	while (count < MAX_PARAMETERS && *p)
	{
		SKIP_WS(p);
		if (*p == '"')
		{
			p++;
			parameters [count++] = p;
			while (*p && *p	!= '"')
				p++;
			*p++ = 0;
		}
		else
			if (*p == '\'')
			{
				p++;
				parameters [count++] = p;
				while (*p && *p	!= '\'')
					p++;
				*p++ = 0;
			}
			else
			{
				parameters [count++] = p;
				while (*p && !isspace(*p))
					p++;
				if (!*p)
					break;
				*p++ = 0;
			}
	}

	// parse
#endif

	return 0;
}
