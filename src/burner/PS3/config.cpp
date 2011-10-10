// Burner xml config file module, added by regret

#include "burner.h"

#ifndef NO_IPS
//#include "patch.h"
#endif

#include "config_file.h"

int nPatchLang = 0;
int bEnforceDep = 0;
int nLoadMenuShowX;
int nLoadMenuShowY;

int nIniVersion = 0;
static string str = "";
const float configVersion = 0.03f;

// Read in the config file for the whole application (NOTE: Not actually XML - and we like it that way)
int configAppLoadXml()
{
	config_file_t * currentconfig = config_file_new(SYS_CONFIG_FILE);

	// video
	init_setting_int("rotate-vertical", nVidRotationAdjust, 0);
	init_setting_int("vsync", bVidVSync, 1);
	init_setting_int("triple-buffer", bVidTripleBuffer, 1);

	// render
	init_setting_uint("linear", vidFilterLinear, 1);
	init_setting_uint("linear2", vidFilterLinear2, 1);
	init_setting_char("currentshader", selectedShader[0].filename, "stock.cg");
	init_setting_char("currentshader2", selectedShader[1].filename, "stock.cg");

	init_setting_int("xoffset", nXOffset, 0);
	init_setting_int("yoffset", nYOffset, 0);
	init_setting_int("xscale", nXScale, 0);
	init_setting_int("yscale", nYScale, 0);
	init_setting_uint("scalingfactor", bVidScalingFactor, 1);
	init_setting_bool("fbomode", bVidFBOEnabled, false);

	init_setting_uint("resolutionid", currentAvailableResolutionId, 0);

	init_setting_uint("firststartup", bBurnFirstStartup, 1);

	// video others
	init_setting_int("aspectx", nVidScrnAspectX, 4);
	init_setting_int("aspecty", nVidScrnAspectY, 3);
	init_setting_int("aspectmode", nVidScrnAspectMode, 0);

	// gui
	init_setting_int("lastrom", nLastRom, 0);
	init_setting_int("lastfilter", nLastFilter, 0);
	init_setting_int("hidechildren", HideChildren, 0);
	init_setting_int("showthreefourplayeronly", ThreeOrFourPlayerOnly, 0);

	init_setting_int("nloadmenushowx", nLoadMenuShowX, 0);
	init_setting_int("language", nPatchLang, 0);
	init_setting_int("dependancy", bEnforceDep, 0)

	char tempStr[64] = "";

	for (int i = 0; i < 4; i++)
	{
		sprintf(tempStr, "controlsdefault%d", i);
		init_setting_char(tempStr, szPlayerDefaultIni[i], "");
	}

	init_setting_char("rompath", szAppRomPaths[0], "/dev_hdd0/game/FBAN00000/USRDIR/roms/");

	//paths
	for (int i = PATH_PREVIEW; i < PATH_SUM; i++)
	{
		sprintf(tempStr, "path%d", i);
		init_setting_char(tempStr, szMiscPaths[i], szMiscPaths[i]);
	}

	return 0;
}

// Write out the config file for the whole application
int configAppSaveXml()
{
	config_file_t * currentconfig = config_file_new(SYS_CONFIG_FILE);

	char tempStr[64] = "";

	// title
	sprintf(tempStr, "0x%06X", nBurnVer);
	config_set_string(currentconfig,  "version", tempStr);

	// video
	config_set_int(currentconfig, "rotate-vertical", nVidRotationAdjust);
	config_set_int(currentconfig, "vsync", bVidVSync);
	config_set_int(currentconfig, "triple-buffer", bVidTripleBuffer);

	// video render
	config_set_string(currentconfig, "currentshader", selectedShader[0].filename);
	config_set_string(currentconfig, "currentshader2", selectedShader[1].filename);

	config_set_uint(currentconfig, "linear", vidFilterLinear);
	config_set_uint(currentconfig, "linear2", vidFilterLinear2);

	config_set_int(currentconfig, "xoffset", nXOffset);
	config_set_int(currentconfig, "yoffset", nYOffset);
	config_set_int(currentconfig, "xscale", nXScale);
	config_set_int(currentconfig, "yscale", nYScale);
	config_set_int(currentconfig, "scalingfactor", bVidScalingFactor);
	config_set_bool(currentconfig, "fbomode", bVidFBOEnabled);

	config_set_uint(currentconfig, "resolutionid", currentAvailableResolutionId);
	config_set_uint(currentconfig, "firststartup", bBurnFirstStartup);

	// video others
	config_set_int(currentconfig, "aspectx", nVidScrnAspectX);
	config_set_int(currentconfig, "aspecty", nVidScrnAspectY);
	config_set_int(currentconfig, "aspectmode", nVidScrnAspectMode);

	// gui
	config_set_int(currentconfig, "lastrom", nLastRom);
	config_set_int(currentconfig, "lastfilter", nLastFilter);

	config_set_int(currentconfig, "hidechildren", HideChildren);
	config_set_int(currentconfig, "showthreefourplayeronly", ThreeOrFourPlayerOnly);

	// gui load game dialog
	config_set_int(currentconfig, "nloadmenushowx", nLoadMenuShowX);

	// gui ips
	config_set_int(currentconfig, "language", nPatchLang);
	config_set_int(currentconfig, "dependancy", bEnforceDep);

	// preferences

	for (int i = 0; i < 4; i++)
	{
		sprintf(tempStr, "controlsdefault%d", i);
		config_set_string(currentconfig, tempStr, szPlayerDefaultIni[i]);
	}

	// paths

	//for (int i = 0; i < DIRS_MAX; i++)
	//addTextNode(rom_path, "path", szAppRomPaths[i]);
	config_set_string(currentconfig, "rompath", szAppRomPaths[0]);

	for (int i = PATH_PREVIEW; i < PATH_SUM; i++)
	{
		sprintf(tempStr, "path%d", i);
		config_set_string(currentconfig, tempStr, szMiscPaths[i]);
	}

	// save file
	config_file_write(currentconfig, SYS_CONFIG_FILE);

	return 0;
}
