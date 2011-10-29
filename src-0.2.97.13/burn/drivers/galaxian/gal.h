#include "tiles_generic.h"
#include "dac.h"
#include "8255ppi.h"
#include "bitswap.h"
#include "s2650_intf.h"
#include "sn76496.h"

#include "driver.h"
extern "C" {
 #include "ay8910.h"
}

// ROM types
#define GAL_ROM_Z80_PROG1				1
#define GAL_ROM_Z80_PROG2				2
#define GAL_ROM_Z80_PROG3				3
#define GAL_ROM_TILES_SHARED				4
#define GAL_ROM_TILES_CHARS				5
#define GAL_ROM_TILES_SPRITES				6
#define GAL_ROM_PROM					7
#define GAL_ROM_S2650_PROG1				8

#define GAL_ROM_OFFSET_Z80_PROG2			GalZ80Rom1Num
#define GAL_ROM_OFFSET_Z80_PROG3			GalZ80Rom1Num + GalZ80Rom2Num
#define GAL_ROM_OFFSET_TILES_SHARED			GalZ80Rom1Num + GalZ80Rom2Num + GalZ80Rom3Num
#define GAL_ROM_OFFSET_TILES_CHARS			GalZ80Rom1Num + GalZ80Rom2Num + GalZ80Rom3Num + GalTilesSharedRomNum
#define GAL_ROM_OFFSET_TILES_SPRITES			GalZ80Rom1Num + GalZ80Rom2Num + GalZ80Rom3Num + GalTilesSharedRomNum + GalTilesCharRomNum
#define GAL_ROM_OFFSET_PROM				GalZ80Rom1Num + GalZ80Rom2Num + GalZ80Rom3Num + GalTilesSharedRomNum + GalTilesCharRomNum + GalTilesSpriteRomNum
#define GAL_ROM_OFFSET_S2650_PROG1			GalZ80Rom1Num + GalZ80Rom2Num + GalZ80Rom3Num + GalTilesSharedRomNum + GalTilesCharRomNum + GalTilesSpriteRomNum + GalPromRomNum

// IRQ types
#define GAL_IRQ_TYPE_NMI				1
#define GAL_IRQ_TYPE_IRQ0				2

// Sound hardware types
#define GAL_SOUND_HARDWARE_TYPE_GALAXIAN		1
#define GAL_SOUND_HARDWARE_TYPE_ZIGZAGAY8910		2
#define GAL_SOUND_HARDWARE_TYPE_JUMPBUGAY8910		3
#define GAL_SOUND_HARDWARE_TYPE_CHECKMANAY8910		4
#define GAL_SOUND_HARDWARE_TYPE_CHECKMAJAY8910		5
#define GAL_SOUND_HARDWARE_TYPE_MSHUTTLEAY8910		6
#define GAL_SOUND_HARDWARE_TYPE_KINGBALLDAC		7
#define GAL_SOUND_HARDWARE_TYPE_FROGGERAY8910		8
#define GAL_SOUND_HARDWARE_TYPE_KONAMIAY8910		9
#define GAL_SOUND_HARDWARE_TYPE_EXPLORERAY8910		10
#define GAL_SOUND_HARDWARE_TYPE_SCORPIONAY8910		11
#define GAL_SOUND_HARDWARE_TYPE_SFXAY8910DAC		12
#define GAL_SOUND_HARDWARE_TYPE_BONGOAY8910		13
#define GAL_SOUND_HARDWARE_TYPE_AD2083AY8910		14
#define GAL_SOUND_HARDWARE_TYPE_RACKNROLSN76496		15
#define GAL_SOUND_HARDWARE_TYPE_HEXPOOLASN76496		16
#define GAL_SOUND_HARDWARE_TYPE_HUNCHBACKAY8910		17

// Palette Numbers
#define GAL_PALETTE_NUM_COLOURS_PROM			64
#define GAL_PALETTE_NUM_COLOURS_STARS			64
#define GAL_PALETTE_NUM_COLOURS_BULLETS			8
#define GAL_PALETTE_NUM_COLOURS_BACKGROUND		256
#define GAL_PALETTE_STARS_OFFSET			GAL_PALETTE_NUM_COLOURS_PROM
#define GAL_PALETTE_BULLETS_OFFSET			GAL_PALETTE_NUM_COLOURS_PROM + GAL_PALETTE_NUM_COLOURS_STARS
#define GAL_PALETTE_BACKGROUND_OFFSET			GAL_PALETTE_NUM_COLOURS_PROM + GAL_PALETTE_NUM_COLOURS_STARS + GAL_PALETTE_NUM_COLOURS_BULLETS

// gal_gfx.cpp
extern unsigned char GalFlipScreenX;
extern unsigned char GalFlipScreenY;
extern unsigned char *GalGfxBank;
extern unsigned char GalPaletteBank;
extern unsigned char GalSpriteClipStart;
extern unsigned char GalSpriteClipEnd;
extern unsigned char FroggerAdjust;
extern unsigned char GalBackgroundRed;
extern unsigned char GalBackgroundGreen;
extern unsigned char GalBackgroundBlue;
extern unsigned char GalBackgroundEnable;
extern unsigned char SfxTilemap;
extern unsigned char GalOrientationFlipX;
extern unsigned char GalColourDepth;
extern unsigned char DarkplntBulletColour;
extern unsigned char DambustrBgColour1;
extern unsigned char DambustrBgColour2;
extern unsigned char DambustrBgPriority;
extern unsigned char DambustrBgSplitLine;
extern unsigned char *RockclimTiles;
extern unsigned short RockclimScrollX;
extern unsigned short RockclimScrollY;
extern int CharPlaneOffsets[2];
extern int CharXOffsets[8];
extern int CharYOffsets[8];
extern int SpritePlaneOffsets[2];
extern int SpriteXOffsets[16];
extern int SpriteYOffsets[16];

typedef void (*GalRenderBackground)();
extern GalRenderBackground GalRenderBackgroundFunction;
typedef void (*GalCalcPalette)();
extern GalCalcPalette GalCalcPaletteFunction;
typedef void (*GalDrawBullet)(int, int, int);
extern GalDrawBullet GalDrawBulletsFunction;
typedef void (*GalExtendTileInfo)(unsigned short*, int*, int, int);
extern GalExtendTileInfo GalExtendTileInfoFunction;
typedef void (*GalExtendSpriteInfo)(const unsigned char*, int*, int*, unsigned char*, unsigned char*, unsigned short*, unsigned char*);
extern GalExtendSpriteInfo GalExtendSpriteInfoFunction;
typedef void (*GalRenderFrame)();
extern GalRenderFrame GalRenderFrameFunction;

void UpperExtendTileInfo(unsigned short *Code, int*, int, int);
void UpperExtendSpriteInfo(const unsigned char*, int*, int*, unsigned char*, unsigned char*, unsigned short *Code, unsigned char*);
void PiscesExtendTileInfo(unsigned short *Code, int*, int, int);
void PiscesExtendSpriteInfo(const unsigned char*, int*, int*, unsigned char*, unsigned char*, unsigned short *Code, unsigned char*);
void Batman2ExtendTileInfo(unsigned short *Code, int*, int, int);
void GmgalaxExtendTileInfo(unsigned short *Code, int*, int, int);
void GmgalaxExtendSpriteInfo(const unsigned char*, int*, int*, unsigned char*, unsigned char*, unsigned short *Code, unsigned char*);
void MooncrstExtendTileInfo(unsigned short *Code, int*, int, int);
void MooncrstExtendSpriteInfo(const unsigned char*, int*, int*, unsigned char*, unsigned char*, unsigned short *Code, unsigned char*);
void MoonqsrExtendTileInfo(unsigned short *Code, int*, int Attr, int);
void MoonqsrExtendSpriteInfo(const unsigned char *Base, int*, int*, unsigned char*, unsigned char*, unsigned short *Code, unsigned char*);
void SkybaseExtendTileInfo(unsigned short *Code, int*, int, int);
void SkybaseExtendSpriteInfo(const unsigned char*, int*, int*, unsigned char*, unsigned char*, unsigned short *Code, unsigned char*);
void RockclimExtendSpriteInfo(const unsigned char*, int*, int*, unsigned char*, unsigned char*, unsigned short *Code, unsigned char*);
void JumpbugExtendTileInfo(unsigned short *Code, int*, int, int);
void JumpbugExtendSpriteInfo(const unsigned char*, int*, int*, unsigned char*, unsigned char*, unsigned short *Code, unsigned char*);
void FroggerExtendTileInfo(unsigned short*, int *Colour, int, int);
void FroggerExtendSpriteInfo(const unsigned char*, int*, int*, unsigned char*, unsigned char*, unsigned short*, unsigned char *Colour);
void CalipsoExtendSpriteInfo(const unsigned char *Base, int*, int*, unsigned char *xFlip, unsigned char *yFlip, unsigned short *Code, unsigned char*);
void MshuttleExtendTileInfo(unsigned short *Code, int*, int Attr, int);
void MshuttleExtendSpriteInfo(const unsigned char *Base, int*, int*, unsigned char*, unsigned char*, unsigned short *Code, unsigned char*);
void Fourin1ExtendTileInfo(unsigned short *Code, int*, int, int);
void Fourin1ExtendSpriteInfo(const unsigned char*, int*, int*, unsigned char*, unsigned char*, unsigned short *Code, unsigned char*);
void DkongjrmExtendSpriteInfo(const unsigned char *Base, int*, int*, unsigned char *xFlip, unsigned char*, unsigned short *Code, unsigned char*);
void MarinerExtendTileInfo(unsigned short *Code, int*, int, int x);
void MimonkeyExtendTileInfo(unsigned short *Code, int*, int, int);
void MimonkeyExtendSpriteInfo(const unsigned char*, int*, int*, unsigned char*, unsigned char*, unsigned short *Code, unsigned char*);
void DambustrExtendTileInfo(unsigned short *Code, int*, int, int x);
void Ad2083ExtendTileInfo(unsigned short *Code, int *Colour, int Attr, int);
void Ad2083ExtendSpriteInfo(const unsigned char *Base, int*, int*, unsigned char *xFlip, unsigned char*, unsigned short *Code, unsigned char*);
void RacknrolExtendTileInfo(unsigned short *Code, int*, int, int x);
void HardCodeGalaxianPROM();
void HardCodeMooncrstPROM();
void GalaxianCalcPalette();
void RockclimCalcPalette();
void MarinerCalcPalette();
void StratgyxCalcPalette();
void RescueCalcPalette();
void MinefldCalcPalette();
void DarkplntCalcPalette();
void DambustrCalcPalette();
void GalaxianDrawBackground();
void RockclimDrawBackground();
void JumpbugDrawBackground();
void FroggerDrawBackground();
void TurtlesDrawBackground();
void ScrambleDrawBackground();
void AnteaterDrawBackground();
void MarinerDrawBackground();
void StratgyxDrawBackground();
void RescueDrawBackground();
void MinefldDrawBackground();
void DambustrDrawBackground();
void GalaxianDrawBullets(int Offs, int x, int y);
void TheendDrawBullets(int Offs, int x, int y);
void ScrambleDrawBullets(int, int x, int y);
void MoonwarDrawBullets(int, int x, int y);
void MshuttleDrawBullets(int, int x, int y);
void DarkplntDrawBullets(int, int x, int y);
void DambustrDrawBullets(int Offs, int x, int y);
void GalDraw();
void DkongjrmRenderFrame();
void DambustrRenderFrame();

// gal_run.cpp
extern unsigned char GalInputPort0[8];
extern unsigned char GalInputPort1[8];
extern unsigned char GalInputPort2[8];
extern unsigned char GalInputPort3[8];
extern unsigned char GalDip[7];
extern unsigned char GalInput[4];
extern unsigned char GalReset;
extern unsigned char GalFakeDip;
extern int           GalAnalogPort0;
extern int           GalAnalogPort1;

extern unsigned char *GalMem;
extern unsigned char *GalMemEnd;
extern unsigned char *GalRamStart;
extern unsigned char *GalRamEnd;
extern unsigned char *GalZ80Rom1;
extern unsigned char *GalZ80Rom1Op;
extern unsigned char *GalZ80Rom2;
extern unsigned char *GalZ80Rom3;
extern unsigned char *GalS2650Rom1;
extern unsigned char *GalZ80Ram1;
extern unsigned char *GalZ80Ram2;
extern unsigned char *GalVideoRam;
extern unsigned char *GalVideoRam2;
extern unsigned char *GalSpriteRam;
extern unsigned char *GalScrollVals;
extern unsigned char *GalProm;
extern unsigned char *GalChars;
extern unsigned char *GalSprites;
extern unsigned char *GalTempRom;
extern unsigned int *GalPalette;
extern unsigned int GalZ80Rom1Size;
extern unsigned int GalZ80Rom1Num;
extern unsigned int GalZ80Rom2Size;
extern unsigned int GalZ80Rom2Num;
extern unsigned int GalZ80Rom3Size;
extern unsigned int GalZ80Rom3Num;
extern unsigned int GalS2650Rom1Size;
extern unsigned int GalS2650Rom1Num;
extern unsigned int GalTilesSharedRomSize;
extern unsigned int GalTilesSharedRomNum;
extern unsigned int GalTilesCharRomSize;
extern unsigned int GalTilesCharRomNum;
extern unsigned int GalNumChars;
extern unsigned int GalTilesSpriteRomSize;
extern unsigned int GalTilesSpriteRomNum;
extern unsigned int GalNumSprites;
extern unsigned int GalPromRomSize;
extern unsigned int GalPromRomNum;

typedef void (*GalPostLoadCallback)();
extern GalPostLoadCallback GalPostLoadCallbackFunction;

extern unsigned char GalIrqType;
extern unsigned char GalIrqFire;
extern int nGalCyclesDone[3], nGalCyclesTotal[3];

extern unsigned char ZigzagAYLatch;
extern unsigned char GalSoundLatch;
extern unsigned char GalSoundLatch2;
extern unsigned char KingballSound;
extern unsigned char KonamiSoundControl;
extern unsigned char SfxSampleControl;
extern unsigned char KingballSpeechDip;
extern unsigned short ScrambleProtectionState;
extern unsigned char ScrambleProtectionResult;
extern unsigned char MoonwarPortSelect;
extern unsigned char MshuttleAY8910CS;
extern unsigned char GmgalaxSelectedGame;
extern unsigned char Fourin1Bank;
extern unsigned char GameIsGmgalax;
extern unsigned char CavelonBankSwitch;
extern unsigned char GalVBlank;

unsigned char KonamiPPIReadIN0();
unsigned char KonamiPPIReadIN1();
unsigned char KonamiPPIReadIN2();
unsigned char KonamiPPIReadIN3();
int GalInit();
void MapMooncrst();
void MapJumpbug();
void MapFrogger();
void KonamiPPIInit();
void MapTheend();
void MapTurtles();
void MapScobra();
int GalExit();
int KonamiExit();
int GalFrame();
int GalScan(int nAction, int *pnMin);

// gal_sound.cpp
extern short* pFMBuffer;
extern short* pAY8910Buffer[9];
extern unsigned char GalSoundType;
extern unsigned char GalSoundVolumeShift;
extern unsigned char HunchbksSoundIrqFire;
extern unsigned char GalLastPort2;
extern unsigned char GalShootEnable;
extern unsigned char GalNoiseEnable;
extern int GalNoiseVolume;
extern double GalShootWavePos;
extern double GalNoiseWavePos;
extern int GalPitch;
extern int GalVol;
extern int GalLfoVolume[3];
extern double GalLfoFreq;
extern double GalLfoFreqFrameVar;
extern int GalLfoBit[4];

void GalSoundReset();
void GalSoundInit();
void GalSoundExit();
void GalSoundScan(int nAction, int *pnMin);
unsigned char CheckmajPortARead(unsigned int);
unsigned char BongoDipSwitchRead(unsigned int);
unsigned char KonamiSoundLatchRead(unsigned int);
unsigned char KonamiSoundTimerRead(unsigned int);
unsigned char FroggerSoundTimerRead(unsigned int);
unsigned char HunchbksSoundTimerRead(unsigned int);
void KonamiSoundLatchWrite(unsigned char d);
void KonamiSoundControlWrite(unsigned char d);
void HunchbksSoundControlWrite(unsigned char d);
void SfxSoundLatch2Write(unsigned int, unsigned int d);
void SfxSampleControlWrite(unsigned int, unsigned int d);
void FroggerSoundInit();
void FroggerSoundNoEncryptionInit();
void KonamiSoundInit();
void HunchbksSoundInit();
void GalRenderSoundSamples(short *pSoundBuf, int nLength);
void GalaxianSoundWrite(unsigned int Offset, unsigned char d);
void GalaxianLfoFreqWrite(unsigned int Offset, unsigned char d);
void GalaxianSoundUpdateTimers();

// gal_stars.cpp
extern int GalStarsEnable;
extern int GalStarsScrollPos;
extern int GalStarsBlinkState;
extern int GalBlinkTimerStartFrame;

void GalInitStars();
void GalaxianRenderStarLayer();
void JumpbugRenderStarLayer();
void ScrambleRenderStarLayer();
void MarinerRenderStarLayer();
void RescueRenderStarLayer();
