// Burner xml config file module, added by regret

/* changelog:
 update 4: change format, improve compatibility
 update 3: improve xml loading
 update 2: add inline functions
 update 1: create
*/

#include "burner.h"
#include "maphkeys.h"
#ifndef NO_IPS
#include "patch.h"
#endif
#include "../lib/ticpp/ticpp.h"
#include <locale.h>

int nIniVersion = 0;
static string str = "";
const float configVersion = 0.03f;

// inline functions
static inline void addTextNode(ticpp::Element& parent, const char* title, const char* value)
{
	if (!value) {
		return;
	}

	ticpp::Text myText(value);
	if (!title) {
		parent.SetText(myText);
	} else {
		ticpp::Element myTitle(title);
		myTitle.LinkEndChild(&myText);
		parent.LinkEndChild(&myTitle);
	}
}

template <typename T>
static inline void setAttr(ticpp::Element& element, const char* attr, const T& value)
{
	if (!attr) {
		return;
	}
	element.SetAttribute(attr, value);
}

static inline ticpp::Element* findElement(ticpp::Element* element, const char* attr)
{
	if (!element || !attr) {
		return NULL;
	}
	return element->FirstChildElement(attr, false);
}

template <typename T>
static inline void getAttr(ticpp::Element* element, const char* attr, T* value)
{
	if (!element || !attr || !value) {
		return;
	}
	element->GetAttribute(attr, value, false);
}

static inline void getTextStr(ticpp::Element* element, TCHAR* name)
{
	if (!element || !name) {
		return;
	}

	str = "";
	element->GetText(&str, false);

	if (str != "") {
		_tcscpy(name, U8toW(str.c_str()));
	}
}

// get config filename
static inline void createConfigName(char* config)
{
	sprintf(config, "%s.xml", WtoA(szAppExeName));
}

// Read in the config file for the whole application
int configAppLoadXml()
{
	setlocale(LC_ALL, "");

	char configName[MAX_PATH];
	createConfigName(configName);

	if (!fileExists(AtoW(configName))) {
		return 1;
	}

	try
	{
		ticpp::Document doc(configName);
	    doc.LoadFile();

		ticpp::Element* root = doc.FirstChildElement();
		float xmlVersion;
		root->GetAttribute("version", &xmlVersion, false);
		if (xmlVersion < configVersion) {
			// TODO: don't read config ?
		}

		ticpp::Element* element, *parent, *child; //

		element = findElement(root, "version");
		element->GetText(&str);
		nIniVersion = strtol(str.c_str(), NULL, 0);

		// emulation
		element = findElement(root, "emulation");
		getAttr(element, "asm-68k", &bBurnUseASM68K);
		getAttr(element, "all-ram", &bDrvSaveAll);

		// video
		element = findElement(root, "video");
		child = findElement(element, "fullscreen");
		getAttr(child, "width", &nVidWidth);
		getAttr(child, "height", &nVidHeight);
		getAttr(child, "depth", &nVidDepth);
		getAttr(child, "refresh", &nVidRefresh);

		child = findElement(element, "adjust");
		getAttr(child, "rotate-vertical", &nVidRotationAdjust);

		child = findElement(element, "screen-size");
		getAttr(child, "window", &nWindowSize);

		child = findElement(element, "window-position");
		getAttr(child, "x", &nWindowPosX);
		getAttr(child, "y", &nWindowPosY);

		child = findElement(element, "stretch");
		getAttr(child, "full-stretch", &bVidFullStretch);
		getAttr(child, "correct-aspect", &bVidCorrectAspect);

		child = findElement(element, "color");
		getAttr(child, "enable", &bcolorAdjust);
		getAttr(child, "contrast", &color_contrast);
		getAttr(child, "brightness", &color_brightness);
		getAttr(child, "gamma", &color_gamma);
		getAttr(child, "grayscale", &color_grayscale);
		getAttr(child, "invert", &color_invert);

		child = findElement(element, "vsync");
		getAttr(child, "enable", &bVidVSync);
		child = findElement(element, "triple-buffer");
		getAttr(child, "enable", &bVidTripleBuffer);

		// render
		parent = findElement(element, "render");
		child = findElement(parent, "render-driver");
		getAttr(child, "driver", &nVidSelect);
		getAttr(child, "adapter", &nVidAdapter);
		child = findElement(parent, "filter");
		getAttr(child, "linear", &vidFilterLinear);
		getAttr(child, "use-pixelfilter", &vidUseFilter);
		getAttr(child, "pixel-filter", &nVidFilter);
		child = findElement(parent, "option");
		getAttr(child, "force-16bit", &bVidForce16bit);
		getAttr(child, "hardware-vertex", &vidHardwareVertex);
		getAttr(child, "motion-blur", &vidMotionBlur);
		getAttr(child, "projection", &nVid3DProjection);
		getAttr(child, "angel", &fVidScreenAngle);
		getAttr(child, "curvature", &fVidScreenCurvature);
		getAttr(child, "dxmanager", &nVidDXTextureManager);

		// video others
		child = findElement(element, "monitor");
		getAttr(child, "auto-aspect", &autoVidScrnAspect);
		getAttr(child, "aspect-x", &nVidScrnAspectX);
		getAttr(child, "aspect-y", &nVidScrnAspectY);

		child = findElement(element, "frame");
		getAttr(child, "auto-frameskip", &autoFrameSkip);
		getAttr(child, "force-60hz", &bForce60Hz);

		// audio
		element = findElement(root, "audio");
		if (element) {
			child = findElement(element, "sound");
			getTextStr(child, audSelect);
			child = findElement(element, "device");
			getAttr(child, "ds", &dsDevice);
			getAttr(child, "xa2", &xa2Device);
			getAttr(child, "oal", &oalDevice);
			child = findElement(element, "setting");
			getAttr(child, "rate", &nAudSampleRate);
			getAttr(child, "frame", &nAudSegCount);
			getAttr(child, "dsp", &nAudDSPModule);
			getAttr(child, "pcm-interp", &nInterpolation);
			getAttr(child, "fm-interp", &nFMInterpolation);
			getAttr(child, "stereo-upmix", &audStereoUpmixing);
		}

		// gui
		element = findElement(root, "gui");
		if (element) {
			child = findElement(element, "language");
			getTextStr(child, szLanguage);
			child = findElement(element, "gamelist");
			getTextStr(child, szTransGamelistFile);

			child = findElement(element, "chat-font");
			getAttr(child, "min-size", &nMinChatFontSize);
			getAttr(child, "max-size", &nMaxChatFontSize);

			child = findElement(element, "menu");
			getAttr(child, "modeless", &bModelessMenu);
			getAttr(child, "style", &menuNewStyle);

			child = findElement(element, "gui-misc");
			getAttr(child, "on-top", &bShowOnTop);
			getAttr(child, "auto-fullscreen", &bFullscreenOnStart);
			getAttr(child, "nosplash", &nDisableSplash);
			getAttr(child, "splash-time", &nSplashTime);

			child = findElement(element, "gamelist-dlg");
			getAttr(child, "options", &nLoadMenuShowX);
			getAttr(child, "drivers", &nLoadDriverShowX);
			getAttr(child, "sys-sel", &nSystemSel);
			getAttr(child, "tab-sel", &nTabSel);
			child = findElement(child, "user-filter");
			getTextStr(child, szUserFilterStr);

			child = findElement(element, "ips");
			getAttr(child, "language", &nPatchLang);
			getAttr(child, "dependancy", &bEnforceDep);

			child = findElement(element, "skin");
			getAttr(child, "use-placeholder", &bVidUsePlaceholder);
			getAttr(child, "random", &nRandomSkin);
			getTextStr(child, szPlaceHolder);
		}

		// preferences
		element = findElement(root, "preferences");
		if (element) {
			child = findElement(element, "settings");
			getAttr(child, "always-processkey", &bAlwaysProcessKey);
			getAttr(child, "auto-pause", &bAutoPause);
			getAttr(child, "avi-audio", &nAviIntAudio);
			getAttr(child, "use-gdip", &bUseGdip);

			child = findElement(element, "fastforward");
			getAttr(child, "speed", &nFastSpeed);
			child = findElement(element, "thread");
			getAttr(child, "priority", &nAppThreadPriority);
			child = findElement(element, "autofire");
			getAttr(child, "enable", &nAutofireEnabled);
			getAttr(child, "default-delay", &autofireDefaultDelay);
			child = findElement(element, "macro");
			getAttr(child, "enable", &nInputMacroEnabled);
			child = findElement(element, "misc");
			getAttr(child, "effect", &nShowEffect);

			child = findElement(element, "controls");
			if (child) {
				child = child->FirstChildElement();
				for (int i = 0; i < 4, child; i++, child = child->NextSiblingElement(false)) {
					getTextStr(child, szPlayerDefaultIni[i]);
				}
			}
		}

		// paths
		element = findElement(root, "paths");
		if (element) {
			child = findElement(element, "rom");
			if (child) {
				child = child->FirstChildElement();
				for (int i = 0; i < DIRS_MAX, child;
					i++, child = child->NextSiblingElement(false)) {
					getTextStr(child, szAppRomPaths[i]);
				}
			}

			child = findElement(element, "misc");
			if (child) {
				child = child->FirstChildElement();
				for (int i = PATH_PREVIEW; i < PATH_SUM, child;
					i++, child = child->NextSiblingElement(false)) {
					getTextStr(child, szMiscPaths[i]);
				}
			}
		}

		// hotkeys
		int keymode = 0;
		element = findElement(root, "hotkeys");
		if (element) {
			for (int i = 0; !lastCustomKey(customKeys[i]); i++) {
				CustomKey& customkey = customKeys[i];

				child = findElement(element, customkey.config_code);
				if (!child) {
					continue;
				}
				getAttr(child, "key", &customkey.key);
				getAttr(child, "mod", &customkey.keymod);
			}
		}
	}
	catch (ticpp::Exception& ex)
	{
		return 1;
	}

	return 0;
}

// Write out the config file for the whole application
int configAppSaveXml()
{
	if (bCmdOptUsed) {
		return 1;
	}

	char configName[MAX_PATH];
	createConfigName(configName);

	try {
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

		// emulation
		ticpp::Element emulation("emulation");
		root.LinkEndChild(&emulation);
		setAttr(emulation, "asm-68k", bBurnUseASM68K);
		setAttr(emulation, "all-ram", bDrvSaveAll);

		// video
		ticpp::Element video("video");
		root.LinkEndChild(&video);

		ticpp::Element fullscreen("fullscreen");
		video.LinkEndChild(&fullscreen);
		setAttr(fullscreen, "width", nVidWidth);
		setAttr(fullscreen, "height", nVidHeight);
		setAttr(fullscreen, "depth", nVidDepth);
		setAttr(fullscreen, "refresh", nVidRefresh);

		ticpp::Element adjust("adjust");
		video.LinkEndChild(&adjust);
		setAttr(adjust, "rotate-vertical", nVidRotationAdjust);

		ticpp::Element screen_size("screen-size");
		video.LinkEndChild(&screen_size);
		setAttr(screen_size, "window", nWindowSize);

		ticpp::Element window_position("window-position");
		video.LinkEndChild(&window_position);
		setAttr(window_position, "x", nWindowPosX);
		setAttr(window_position, "y", nWindowPosY);

		ticpp::Element stretch("stretch");
		video.LinkEndChild(&stretch);
		setAttr(stretch, "full-stretch", bVidFullStretch);
		setAttr(stretch, "correct-aspect", bVidCorrectAspect);

		ticpp::Element color("color");
		video.LinkEndChild(&color);
		setAttr(color, "enable", bcolorAdjust);
		setAttr(color, "contrast", color_contrast);
		setAttr(color, "brightness", color_brightness);
		setAttr(color, "gamma", color_gamma);
		setAttr(color, "grayscale", color_grayscale);
		setAttr(color, "invert", color_invert);

		ticpp::Element vsync("vsync");
		video.LinkEndChild(&vsync);
		setAttr(vsync, "enable", bVidVSync);

		ticpp::Element triple_buffer("triple-buffer");
		video.LinkEndChild(&triple_buffer);
		setAttr(triple_buffer, "enable", bVidTripleBuffer);

		// video render
		ticpp::Element render("render");
		video.LinkEndChild(&render);

		ticpp::Element render_driver("render-driver");
		render.LinkEndChild(&render_driver);
		setAttr(render_driver, "driver", nVidSelect);
		setAttr(render_driver, "adapter", nVidAdapter);

		ticpp::Element filter("filter");
		render.LinkEndChild(&filter);
		setAttr(filter, "linear", vidFilterLinear);
		setAttr(filter, "use-pixelfilter", vidUseFilter);
		setAttr(filter, "pixel-filter", nVidFilter);

		ticpp::Element option("option");
		render.LinkEndChild(&option);
		setAttr(option, "force-16bit", bVidForce16bit);
		setAttr(option, "hardware-vertex", vidHardwareVertex);
		setAttr(option, "motion-blur", vidMotionBlur);
		setAttr(option, "projection", nVid3DProjection);
		setAttr(option, "angel", fVidScreenAngle);
		setAttr(option, "curvature", fVidScreenCurvature);
		setAttr(option, "dxmanager", nVidDXTextureManager);

		// video others
		ticpp::Element monitor("monitor");
		video.LinkEndChild(&monitor);
		setAttr(monitor, "auto-aspect", autoVidScrnAspect);
		setAttr(monitor, "aspect-x", nVidScrnAspectX);
		setAttr(monitor, "aspect-y", nVidScrnAspectY);

		ticpp::Element frame("frame");
		video.LinkEndChild(&frame);
		setAttr(frame, "auto-frameskip", autoFrameSkip);
		setAttr(frame, "force-60hz", bForce60Hz);

		// audio
		ticpp::Element audio("audio");
		root.LinkEndChild(&audio);
		addTextNode(audio, "sound", WtoU8(audSelect));

		ticpp::Element device("device");
		audio.LinkEndChild(&device);
		setAttr(device, "ds", dsDevice);
		setAttr(device, "xa2", xa2Device);
		setAttr(device, "oal", oalDevice);

		ticpp::Element audio_set("setting");
		audio.LinkEndChild(&audio_set);
		setAttr(audio_set, "rate", nAudSampleRate);
		setAttr(audio_set, "frame", nAudSegCount);
		setAttr(audio_set, "dsp", nAudDSPModule);
		setAttr(audio_set, "pcm-interp", nInterpolation);
		setAttr(audio_set, "fm-interp", nFMInterpolation);
		setAttr(audio_set, "stereo-upmix", audStereoUpmixing);

		// gui
		ticpp::Element gui("gui");
		root.LinkEndChild(&gui);
		addTextNode(gui, "language", WtoU8(szLanguage));
		addTextNode(gui, "gamelist", WtoU8(szTransGamelistFile));

		ticpp::Element chat("chat-font");
		gui.LinkEndChild(&chat);
		setAttr(chat, "min-size", nMinChatFontSize);
		setAttr(chat, "max-size", nMaxChatFontSize);

		ticpp::Element menu("menu");
		gui.LinkEndChild(&menu);
		setAttr(menu, "modeless", bModelessMenu);
		setAttr(menu, "style", menuNewStyle);

		ticpp::Element gui_misc("gui-misc");
		gui.LinkEndChild(&gui_misc);
		setAttr(gui_misc, "on-top", bShowOnTop);
		setAttr(gui_misc, "auto-fullscreen", bFullscreenOnStart);
		setAttr(gui_misc, "nosplash", nDisableSplash);
		setAttr(gui_misc, "splash-time", nSplashTime);

		// gui load game dialog
		ticpp::Element gamelist("gamelist-dlg");
		gui.LinkEndChild(&gamelist);
		setAttr(gamelist, "options", nLoadMenuShowX);
		setAttr(gamelist, "drivers", nLoadDriverShowX);
		setAttr(gamelist, "sys-sel", nSystemSel);
		setAttr(gamelist, "tab-sel", nTabSel);
		addTextNode(gamelist, "user-filter", WtoU8(szUserFilterStr));

		// gui ips
		ticpp::Element ips("ips");
		gui.LinkEndChild(&ips);
		setAttr(ips, "language", nPatchLang);
		setAttr(ips, "dependancy", bEnforceDep);

		ticpp::Element skin("skin");
		gui.LinkEndChild(&skin);
		setAttr(skin, "use-placeholder", bVidUsePlaceholder);
		setAttr(skin, "random", nRandomSkin);
		addTextNode(skin, NULL, WtoU8(szPlaceHolder));

		// preferences
		ticpp::Element preference("preferences");
		root.LinkEndChild(&preference);

		ticpp::Element settings("settings");
		preference.LinkEndChild(&settings);
		setAttr(settings, "always-processkey", bAlwaysProcessKey);
		setAttr(settings, "auto-pause", bAutoPause);
		setAttr(settings, "avi-audio", nAviIntAudio);
		setAttr(settings, "use-gdip", bUseGdip);
		ticpp::Element fastforward("fastforward");
		preference.LinkEndChild(&fastforward);
		setAttr(fastforward, "speed", nFastSpeed);
		ticpp::Element thread("thread");
		preference.LinkEndChild(&thread);
		setAttr(thread, "priority", nAppThreadPriority);
		ticpp::Element autofire("autofire");
		preference.LinkEndChild(&autofire);
		setAttr(autofire, "enable", nAutofireEnabled);
		setAttr(autofire, "default-delay", autofireDefaultDelay);
		ticpp::Element macro("macro");
		preference.LinkEndChild(&macro);
		setAttr(macro, "enable", nInputMacroEnabled);

		// pref misc
		ticpp::Element pref_misc("misc");
		preference.LinkEndChild(&pref_misc);
		setAttr(pref_misc, "effect", nShowEffect);

		ticpp::Element controls("controls");
		preference.LinkEndChild(&controls);
		for (int i = 0; i < 4; i++) {
			sprintf(tempStr, "default%d", i);
			addTextNode(controls, tempStr, WtoU8(szPlayerDefaultIni[i]));
		}

		// paths
		ticpp::Element paths("paths");
		root.LinkEndChild(&paths);

		ticpp::Element rom_path("rom");
		paths.LinkEndChild(&rom_path);
		for (int i = 0; i < DIRS_MAX; i++) {
			addTextNode(rom_path, "path", WtoU8(szAppRomPaths[i]));
		}

		ticpp::Element misc_path("misc");
		paths.LinkEndChild(&misc_path);
		for (int i = PATH_PREVIEW; i < PATH_SUM; i++) {
			sprintf(tempStr, "path%d", i);
			addTextNode(misc_path, tempStr, WtoU8(szMiscPaths[i]));
		}

		// hotkeys
		ticpp::Element hotkeys("hotkeys");
		root.LinkEndChild(&hotkeys);

		for (int i = 0; !lastCustomKey(customKeys[i]); i++) {
			CustomKey& customkey = customKeys[i];

			ticpp::Element key(customkey.config_code);
			hotkeys.LinkEndChild(&key);
			setAttr(key, "key", customkey.key);
			setAttr(key, "mod", customkey.keymod);
		}

		// save file
		doc.SaveFile(configName, TIXML_ENCODING_UTF8);
	}
	catch (ticpp::Exception& ex) {
		return 1;
	}

	return 0;
}
