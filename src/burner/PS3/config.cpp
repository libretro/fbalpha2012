// Burner xml config file module, added by regret

/* changelog:
 update 4: change format, improve compatibility
 update 3: improve xml loading
 update 1: create
*/

#include "burner.h"

#ifndef NO_IPS
//#include "patch.h"
#endif

#include "../libs/ticpp/ticpp.h"
#include <locale.h>

int nPatchLang = 0;
int bEnforceDep = 0;
int nLoadMenuShowX;
int nLoadMenuShowY;

int nIniVersion = 0;
static string str = "";
const float configVersion = 0.03f;

static void addTextNode(ticpp::Element& parent, const char* title, const char* value)
{
	if (!value)
		return;

	ticpp::Text myText(value);
	if (!title)
		parent.SetText(myText);
	else
	{
		ticpp::Element myTitle(title);
		myTitle.LinkEndChild(&myText);
		parent.LinkEndChild(&myTitle);
	}
}

template <typename T>
static void setAttr(ticpp::Element& element, const char* attr, const T& value)
{
	if (!attr)
		return;

	element.SetAttribute(attr, value);
}

static ticpp::Element* findElement(ticpp::Element* element, const char* attr)
{
	if (!element || !attr)
		return NULL;

	return element->FirstChildElement(attr, false);
}

template <typename T>
static void getAttr(ticpp::Element* element, const char* attr, T* value)
{
	if (!element || !attr || !value)
		return;

	element->GetAttribute(attr, value, false);
}

static void getTextStr(ticpp::Element* element, TCHAR* name)
{
	if (!element || !name)
		return;

	str = "";
	element->GetText(&str, false);

	if (str != "")
		strcpy(name, str.c_str());
}

// get config filename
static void createConfigName(char* config)
{
	sprintf(config, "/dev_hdd0/game/FBAN00000/USRDIR/fbanext-ps3.xml");
}

// Read in the config file for the whole application
int configAppLoadXml()
{
	setlocale(LC_ALL, "");

	char configName[MAX_PATH];
	createConfigName(configName);

	if (!fileExists(configName))
		return 1;

	ticpp::Document doc(configName);
	doc.LoadFile();

	ticpp::Element* root = doc.FirstChildElement();
	float xmlVersion;
	root->GetAttribute("version", &xmlVersion, false);
	if (xmlVersion < configVersion)
	{
		// TODO: don't read config ?
	}

	ticpp::Element* element, *parent, *child;

	element = findElement(root, "version");
	element->GetText(&str);
	nIniVersion = strtol(str.c_str(), NULL, 0);

	// video
	element = findElement(root, "video");

	child = findElement(element, "adjust");
	getAttr(child, "rotate-vertical", &nVidRotationAdjust);

	child = findElement(element, "vsync");
	getAttr(child, "enable", &bVidVSync);
	child = findElement(element, "triple-buffer");
	getAttr(child, "enable", &bVidTripleBuffer);

	// render
	parent = findElement(element, "render");
	child = findElement(parent, "filter");
	getAttr(child, "linear", &vidFilterLinear);
	child = findElement(parent, "option");
	getAttr(child, "currentshader", &shaderindex);

	getAttr(child, "x-offset", &nXOffset);
	getAttr(child, "y-offset", &nYOffset);
	getAttr(child, "x-scale", &nXScale);
	getAttr(child, "y-scale", &nYScale);


	// video others
	child = findElement(element, "monitor");
	getAttr(child, "aspect-x", &nVidScrnAspectX);
	getAttr(child, "aspect-y", &nVidScrnAspectY);
	getAttr(child, "aspect-mode", &nVidScrnAspectMode);

	// gui
	element = findElement(root, "gui");
	if (element) {
		child = findElement(element, "gui-misc");
		getAttr(child, "lastRom", &nLastRom);
		getAttr(child, "lastFilter", &nLastFilter);
		getAttr(child, "hideChildren", &HideChildren);
		getAttr(child, "showThreeFourPlayerOnly", &ThreeOrFourPlayerOnly);

		child = findElement(element, "gamelist-dlg");
		getAttr(child, "options", &nLoadMenuShowX);

		child = findElement(element, "ips");
		getAttr(child, "language", &nPatchLang);
		getAttr(child, "dependancy", &bEnforceDep);

	}

	// preferences
	element = findElement(root, "preferences");
	if (element)
	{
		child = findElement(element, "controls");
		if (child)
		{
			child = child->FirstChildElement();
			for (int i = 0; i < 4, child; i++, child = child->NextSiblingElement(false))
				getTextStr(child, szPlayerDefaultIni[i]);
		}
	}

	// paths
	element = findElement(root, "paths");
	if (element) {
		child = findElement(element, "rom");
		if (child)
		{
			child = child->FirstChildElement();
			for (int i = 0; i < DIRS_MAX, child; i++, child = child->NextSiblingElement(false))
				getTextStr(child, szAppRomPaths[i]);
		}

		child = findElement(element, "misc");
		if (child)
		{
			child = child->FirstChildElement();
			for (int i = PATH_PREVIEW; i < PATH_SUM, child; i++, child = child->NextSiblingElement(false))
				getTextStr(child, szMiscPaths[i]);
		}
	}
	return 0;
}

// Write out the config file for the whole application
int configAppSaveXml()
{
	char configName[MAX_PATH];
	createConfigName(configName);

	char tempStr[64] = "";

	// root
	ticpp::Document doc;
	ticpp::Declaration decl("1.0", "UTF-8", "");
	doc.LinkEndChild(&decl);

	ticpp::Element root("configuration");
	setAttr(root, "version", configVersion);
	ticpp::Comment comment("Don't edit this file manually unless you know what you're doing\n" \
			APP_TITLE " will restore default settings when this file is deleted");
	doc.LinkEndChild(&comment);
	doc.LinkEndChild(&root);

	// title
	sprintf(tempStr, "0x%06X", nBurnVer);
	addTextNode(root, "version", tempStr);

	// video
	ticpp::Element video("video");
	root.LinkEndChild(&video);

	ticpp::Element adjust("adjust");
	video.LinkEndChild(&adjust);
	setAttr(adjust, "rotate-vertical", nVidRotationAdjust);

	ticpp::Element vsync("vsync");
	video.LinkEndChild(&vsync);
	setAttr(vsync, "enable", bVidVSync);

	ticpp::Element triple_buffer("triple-buffer");
	video.LinkEndChild(&triple_buffer);
	setAttr(triple_buffer, "enable", bVidTripleBuffer);

	// video render
	ticpp::Element render("render");
	video.LinkEndChild(&render);

	ticpp::Element filter("filter");
	render.LinkEndChild(&filter);
	setAttr(filter, "linear", vidFilterLinear);
	ticpp::Element option("option");
	render.LinkEndChild(&option);
	setAttr(option, "currentshader", shaderindex);

	setAttr(option, "x-offset", nXOffset);
	setAttr(option, "y-offset", nYOffset);
	setAttr(option, "x-scale",  nXScale);
	setAttr(option, "y-scale",  nYScale);

	// video others
	ticpp::Element monitor("monitor");
	video.LinkEndChild(&monitor);
	setAttr(monitor, "aspect-x", nVidScrnAspectX);
	setAttr(monitor, "aspect-y", nVidScrnAspectY);
#ifdef SN_TARGET_PS3
	setAttr(monitor, "aspect-mode", nVidScrnAspectMode);
#endif

	// gui
	ticpp::Element gui("gui");
	root.LinkEndChild(&gui);
	//addTextNode(gui, "gamelist", szTransGamelistFile);

	ticpp::Element gui_misc("gui-misc");
	gui.LinkEndChild(&gui_misc);
	setAttr(gui_misc, "lastRom", nLastRom);
	setAttr(gui_misc, "lastFilter", nLastFilter);

	setAttr(gui_misc, "hideChildren", HideChildren);
	setAttr(gui_misc, "showThreeFourPlayerOnly", ThreeOrFourPlayerOnly);

	// gui load game dialog
	ticpp::Element gamelist("gamelist-dlg");
	gui.LinkEndChild(&gamelist);
	setAttr(gamelist, "options", nLoadMenuShowX);

	// gui ips
	ticpp::Element ips("ips");
	gui.LinkEndChild(&ips);
	setAttr(ips, "language", nPatchLang);
	setAttr(ips, "dependancy", bEnforceDep);

	// preferences
	ticpp::Element preference("preferences");
	root.LinkEndChild(&preference);

	ticpp::Element controls("controls");
	preference.LinkEndChild(&controls);

	for (int i = 0; i < 4; i++)
	{
		sprintf(tempStr, "default%d", i);
		addTextNode(controls, tempStr, szPlayerDefaultIni[i]);
	}

	// paths
	ticpp::Element paths("paths");
	root.LinkEndChild(&paths);

	ticpp::Element rom_path("rom");
	paths.LinkEndChild(&rom_path);

	for (int i = 0; i < DIRS_MAX; i++)
		addTextNode(rom_path, "path", szAppRomPaths[i]);

	ticpp::Element misc_path("misc");
	paths.LinkEndChild(&misc_path);

	for (int i = PATH_PREVIEW; i < PATH_SUM; i++)
	{
		sprintf(tempStr, "path%d", i);
		addTextNode(misc_path, tempStr, szMiscPaths[i]);
	}

	// save file
	doc.SaveFile(configName, TIXML_ENCODING_UTF8);

	return 0;
}
