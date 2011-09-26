extern unsigned char TaitoInputPort0[8];
extern unsigned char TaitoInputPort1[8];
extern unsigned char TaitoInputPort2[8];
extern unsigned char TaitoInputPort3[8];
extern unsigned char TaitoInputPort4[8];
extern unsigned char TaitoInputPort5[8];
extern unsigned char TaitoDip[2];
extern unsigned char TaitoInput[6];
extern unsigned char TaitoReset;

extern int TaitoAnalogPort0;
extern int TaitoAnalogPort1;
extern int TaitoAnalogPort2;
extern int TaitoAnalogPort3;

extern unsigned char *TaitoMem;
extern unsigned char *TaitoMemEnd;
extern unsigned char *TaitoRamStart;
extern unsigned char *TaitoRamEnd;
extern unsigned char *Taito68KRam1;
extern unsigned char *Taito68KRam2;
extern unsigned char *TaitoSharedRam;
extern unsigned char *TaitoZ80Ram1;
extern unsigned char *TaitoZ80Ram2;
extern unsigned char *TaitoPaletteRam;
extern unsigned char *TaitoSpriteRam;
extern unsigned char *TaitoSpriteRam2;
extern unsigned char *TaitoSpriteRamBuffered;
extern unsigned char *TaitoSpriteRamDelayed;
extern unsigned char *TaitoSpriteExtension;
extern unsigned char *TaitoVideoRam;
extern unsigned int  *TaitoPalette;
extern unsigned char *TaitoPriorityMap;

extern unsigned char TaitoZ80Bank;
extern unsigned char TaitoSoundLatch;
extern unsigned char TaitoCpuACtrl;
extern int TaitoRoadPalBank;

extern int TaitoXOffset;
extern int TaitoYOffset;
extern int TaitoIrqLine;
extern int TaitoFrameInterleave;
extern int TaitoNumEEPROM;
extern int TaitoFlipScreenX;

extern int TaitoNum68Ks;
extern int TaitoNumZ80s;
extern int TaitoNumYM2610;
extern int TaitoNumYM2151;
extern int TaitoNumYM2203;
extern int TaitoNumMSM5205;
extern int TaitoNumMSM6295;

extern int nTaitoCyclesDone[4], nTaitoCyclesTotal[4];
extern int nTaitoCyclesSegment;

typedef void (*TaitoRender)();
extern TaitoRender TaitoDrawFunction;
typedef void (*TaitoMakeInputs)();
extern TaitoMakeInputs TaitoMakeInputsFunction;
typedef int (*TaitoResetFunc)();
extern TaitoResetFunc TaitoResetFunction;

extern int TaitoDoReset();

#define TAITO_68KROM1			1
#define TAITO_68KROM1_BYTESWAP		2
#define TAITO_68KROM1_BYTESWAP_JUMPING	3
#define TAITO_68KROM2			4
#define TAITO_68KROM2_BYTESWAP		5
#define TAITO_Z80ROM1			6
#define TAITO_Z80ROM2			7
#define TAITO_CHARS			8
#define TAITO_CHARS_BYTESWAP		9
#define TAITO_CHARSB			10
#define TAITO_CHARSB_BYTESWAP		11
#define TAITO_SPRITESA			12
#define TAITO_SPRITESA_BYTESWAP		13
#define TAITO_SPRITESA_BYTESWAP32	14
#define TAITO_SPRITESB			15
#define TAITO_SPRITESB_BYTESWAP		16
#define TAITO_SPRITESB_BYTESWAP32	17
#define TAITO_ROAD			18
#define TAITO_SPRITEMAP			19
#define TAITO_YM2610A			20
#define TAITO_YM2610B			21
#define TAITO_MSM5205			22
#define TAITO_MSM5205_BYTESWAP		23
#define TAITO_CHARS_PIVOT		24
#define TAITO_MSM6295			25
#define TAITO_DEFAULT_EEPROM		26

extern unsigned char *Taito68KRom1;
extern unsigned char *Taito68KRom2;
extern unsigned char *TaitoZ80Rom1;
extern unsigned char *TaitoZ80Rom2;
extern unsigned char *TaitoChars;
extern unsigned char *TaitoCharsB;
extern unsigned char *TaitoCharsPivot;
extern unsigned char *TaitoSpritesA;
extern unsigned char *TaitoSpritesB;
extern unsigned char *TaitoSpriteMapRom;
extern unsigned char *TaitoYM2610ARom;
extern unsigned char *TaitoYM2610BRom;
extern unsigned char *TaitoMSM5205Rom;
extern unsigned char *TaitoMSM6295Rom;
extern unsigned char *TaitoDefaultEEProm;

extern unsigned int Taito68KRom1Num;
extern unsigned int Taito68KRom2Num;
extern unsigned int TaitoZ80Rom1Num;
extern unsigned int TaitoZ80Rom2Num;
extern unsigned int TaitoCharRomNum;
extern unsigned int TaitoCharBRomNum;
extern unsigned int TaitoCharPivotRomNum;
extern unsigned int TaitoSpriteARomNum;
extern unsigned int TaitoSpriteBRomNum;
extern unsigned int TaitoRoadRomNum;
extern unsigned int TaitoSpriteMapRomNum;
extern unsigned int TaitoYM2610ARomNum;
extern unsigned int TaitoYM2610BRomNum;
extern unsigned int TaitoMSM5205RomNum;
extern unsigned int TaitoMSM6295RomNum;
extern unsigned int TaitoDefaultEEPromNum;

extern unsigned int Taito68KRom1Size;
extern unsigned int Taito68KRom2Size;
extern unsigned int TaitoZ80Rom1Size;
extern unsigned int TaitoZ80Rom2Size;
extern unsigned int TaitoCharRomSize;
extern unsigned int TaitoCharRomBSize;
extern unsigned int TaitoCharPivotRomSize;
extern unsigned int TaitoSpriteARomSize;
extern unsigned int TaitoSpriteBRomSize;
extern unsigned int TaitoRoadRomSize;
extern unsigned int TaitoSpriteMapRomSize;
extern unsigned int TaitoYM2610ARomSize;
extern unsigned int TaitoYM2610BRomSize;
extern unsigned int TaitoMSM5205RomSize;
extern unsigned int TaitoMSM6295RomSize;
extern unsigned int TaitoDefaultEEPromSize;

extern unsigned int TaitoCharModulo;
extern unsigned int TaitoCharNumPlanes;
extern unsigned int TaitoCharWidth;
extern unsigned int TaitoCharHeight;
extern unsigned int TaitoNumChar;
extern int *TaitoCharPlaneOffsets;
extern int *TaitoCharXOffsets;
extern int *TaitoCharYOffsets;

extern unsigned int TaitoCharBModulo;
extern unsigned int TaitoCharBNumPlanes;
extern unsigned int TaitoCharBWidth;
extern unsigned int TaitoCharBHeight;
extern unsigned int TaitoNumCharB;
extern int *TaitoCharBPlaneOffsets;
extern int *TaitoCharBXOffsets;
extern int *TaitoCharBYOffsets;

extern unsigned int TaitoCharPivotModulo;
extern unsigned int TaitoCharPivotNumPlanes;
extern unsigned int TaitoCharPivotWidth;
extern unsigned int TaitoCharPivotHeight;
extern unsigned int TaitoNumCharPivot;
extern int *TaitoCharPivotPlaneOffsets;
extern int *TaitoCharPivotXOffsets;
extern int *TaitoCharPivotYOffsets;

extern unsigned int TaitoSpriteAModulo;
extern unsigned int TaitoSpriteANumPlanes;
extern unsigned int TaitoSpriteAWidth;
extern unsigned int TaitoSpriteAHeight;
extern unsigned int TaitoNumSpriteA;
extern int *TaitoSpriteAPlaneOffsets;
extern int *TaitoSpriteAXOffsets;
extern int *TaitoSpriteAYOffsets;
extern int TaitoSpriteAInvertRom;

extern unsigned int TaitoSpriteBModulo;
extern unsigned int TaitoSpriteBNumPlanes;
extern unsigned int TaitoSpriteBWidth;
extern unsigned int TaitoSpriteBHeight;
extern unsigned int TaitoNumSpriteB;
extern int *TaitoSpriteBPlaneOffsets;
extern int *TaitoSpriteBXOffsets;
extern int *TaitoSpriteBYOffsets;

extern int TaitoLoadRoms(int bLoad);
extern int TaitoExit();
