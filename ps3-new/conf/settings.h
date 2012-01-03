/******************************************************************************* 
 * settings.h - Genesis Plus GX PS3
 *
 *  Created on: Aug 14, 2011
********************************************************************************/

/* Emulator-specific additions */
#ifndef PS3SETTINGS_H
#define PS3SETTINGS_H

#define SOUND_MODE_NORMAL	1
#define SOUND_MODE_HEADSET	2
#define SOUND_MODE_RSOUND	3

#define MAX_PATH_LENGTH		1024

#define EMULATOR_SPECIFIC_SETTINGS_UINT32() \
	uint32_t	EmulatedSystem; \
	uint32_t	FMVolume; \
	uint32_t	PSGVolume; \
	uint32_t	SixButtonPad; \
	uint32_t	ExtraCart;

#define EMULATOR_SPECIFIC_SETTINGS_CHAR() \
	char		ActionReplayROMPath[MAX_PATH_LENGTH]; \
	char		GameGenieROMPath[MAX_PATH_LENGTH]; \
	char		SKUpmemROMPath[MAX_PATH_LENGTH]; \
	char		BIOS[MAX_PATH_LENGTH]; \
	char		SKROMPath[MAX_PATH_LENGTH];

struct SSettings
{
	int		PS3OverscanAmount;
	uint32_t	PS3KeepAspect;
	uint32_t	PS3Smooth;
	uint32_t	PS3Smooth2;
	uint32_t	PS3OverscanEnabled;
	uint32_t	PS3PALTemporalMode60Hz;
	uint32_t	PS3FontSize;
	uint32_t	PS3CurrentResolution;
	uint32_t	SoundMode;
	uint32_t	Throttled;
	uint32_t	TripleBuffering;
	uint32_t	ControlScheme;
	uint32_t	SaveCustomControlScheme;
	uint32_t	ScaleEnabled;
	uint32_t	ScaleFactor;
	uint32_t	ViewportX;
	uint32_t	ViewportY;
	uint32_t	ViewportWidth;
	uint32_t	ViewportHeight;
	uint32_t	CurrentSaveStateSlot;
	uint32_t	ApplyShaderPresetOnStartup;
	uint32_t	ScreenshotsEnabled;
	EMULATOR_SPECIFIC_SETTINGS_UINT32();
	char		GameAwareShaderPath[MAX_PATH_LENGTH];
	char		PS3PathSaveStates[MAX_PATH_LENGTH];
	char		PS3PathSRAM[MAX_PATH_LENGTH];
	char		PS3PathROMDirectory[MAX_PATH_LENGTH];
	char		PS3CurrentShader[MAX_PATH_LENGTH];
	char		PS3CurrentShader2[MAX_PATH_LENGTH];
	char		ShaderPresetPath[MAX_PATH_LENGTH];
	char		ShaderPresetTitle[MAX_PATH_LENGTH];
	char		PS3CurrentBorder[MAX_PATH_LENGTH];
	char		RSoundServerIPAddress[MAX_PATH_LENGTH];
	char		PS3CurrentInputPresetTitle[MAX_PATH_LENGTH];
	EMULATOR_SPECIFIC_SETTINGS_CHAR();
};

extern struct SSettings		Settings;

#endif
