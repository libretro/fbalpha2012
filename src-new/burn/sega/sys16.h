#include "tiles_generic.h"
#include "burn_ym2151.h"
#include "burn_ym2203.h"
#include "burn_ym2413.h"
#include "burn_ym2612.h"
#include "rf5c68.h"
#include "burn_gun.h"
#include "bitswap.h"
#include "genesis_vid.h"
#include "8255ppi.h"

#define SYS16_ROM_PROG		1
#define SYS16_ROM_TILES		2
#define SYS16_ROM_SPRITES	3
#define SYS16_ROM_Z80PROG	4
#define SYS16_ROM_KEY		5
#define SYS16_ROM_7751PROG	6
#define SYS16_ROM_7751DATA	7
#define SYS16_ROM_UPD7759DATA	8
#define SYS16_ROM_PROG2		9
#define SYS16_ROM_ROAD		10
#define SYS16_ROM_PCMDATA	11
#define SYS16_ROM_Z80PROG2	12
#define SYS16_ROM_Z80PROG3	13
#define SYS16_ROM_Z80PROG4	14
#define SYS16_ROM_PCM2DATA	15
#define SYS16_ROM_PROM 		16
#define SYS16_ROM_PROG3		17
#define SYS16_ROM_SPRITES2	18
#define SYS16_ROM_RF5C68DATA	19

// sys16_run.cpp
extern unsigned char  System16InputPort0[8];
extern unsigned char  System16InputPort1[8];
extern unsigned char  System16InputPort2[8];
extern unsigned char  System16InputPort3[8];
extern unsigned char  System16InputPort4[8];
extern unsigned char  System16InputPort5[8];
extern unsigned char  System16InputPort6[8];
extern unsigned char  System16Gear;
extern int            System16AnalogPort0;
extern int            System16AnalogPort1;
extern int            System16AnalogPort2;
extern int            System16AnalogPort3;
extern int            System16AnalogPort4;
extern int            System16AnalogPort5;
extern int            System16AnalogSelect;
extern unsigned char  System16Dip[3];
extern unsigned char  System16Input[7];
extern unsigned char  System16Reset;
extern unsigned char *System16Rom;
extern unsigned char *System16Code;
extern unsigned char *System16Rom2;
extern unsigned char *System16Z80Rom;
extern unsigned char  *System16UPD7759Data;
extern unsigned char *System16PCMData;
extern unsigned char *System16RF5C68Data;
extern unsigned char *System16Prom;
extern unsigned char *System16Key;
extern unsigned char *System16Ram;
extern unsigned char *System16ExtraRam;
extern unsigned char *System16Z80Ram;
extern unsigned char *System16TempGfx;
extern unsigned char *System16TileRam;
extern unsigned char *System16TextRam;
extern unsigned char *System16TextRam;
extern unsigned char *System16SpriteRam;
extern unsigned char *System16SpriteRamBuff;
extern unsigned char *System16SpriteRam2;
extern unsigned char *System16RotateRam;
extern unsigned char *System16RotateRamBuff;
extern unsigned char *System16PaletteRam;
extern unsigned char *System16RoadRam;
extern unsigned char *System16RoadRamBuff;
extern unsigned int  *System16Palette;
extern unsigned char *System16Tiles;
extern unsigned char *System16Sprites;
extern unsigned char *System16Sprites2;
extern unsigned char *System16Roads;
extern unsigned int System16NumTiles;
extern unsigned int System16RomSize;
extern unsigned int System16Rom2Size;
extern unsigned int System16TileRomSize;
extern unsigned int System16SpriteRomSize;
extern unsigned int System16Sprite2RomSize;
extern unsigned int System16RoadRomSize;
extern unsigned int System16Z80RomSize;
extern unsigned int System16PCMDataSize;
extern unsigned int System16PCMDataSizePreAllocate;
extern unsigned int System16ExtraRamSize;
extern unsigned int System16SpriteRamSize;
extern unsigned int System16SpriteRam2Size;
extern unsigned int System16RotateRamSize;
extern unsigned int System16UPD7759DataSize;

extern unsigned char System16VideoControl;
extern int System16SoundLatch;
extern bool System16BTileAlt;
extern bool Shangon;
extern bool Hangon;
extern bool System16Z80Enable;

extern int YBoardIrq2Scanline;
extern int System16YM2413IRQInterval;

extern bool System16HasGears;

extern int nSystem16CyclesDone[4]; 
extern unsigned int System16ClockSpeed;

typedef void (*Sim8751)();
extern Sim8751 Simulate8751;

typedef void (*System16Map68K)();
extern System16Map68K System16Map68KDo;

typedef void (*System16MapZ80)();
extern System16MapZ80 System16MapZ80Do;

typedef int (*System16CustomLoadRom)();
extern System16CustomLoadRom System16CustomLoadRomDo;

typedef int (*System16CustomDecryptOpCode)();
extern System16CustomDecryptOpCode System16CustomDecryptOpCodeDo;

typedef unsigned char (*System16ProcessAnalogControls)(UINT16 value);
extern System16ProcessAnalogControls System16ProcessAnalogControlsDo;

typedef void (*System16MakeAnalogInputs)();
extern System16MakeAnalogInputs System16MakeAnalogInputsDo;

int CustomLoadRom20000();
int CustomLoadRom40000();

UINT16 System16MultiplyChipRead(int which, int offset);
void System16MultiplyChipWrite(int which, int offset, UINT16 data);
UINT16 System16CompareTimerChipRead(int which, int offset);
void System16CompareTimerChipWrite(int which, int offset, UINT16 data);
UINT16 System16DivideChipRead(int which, int offset);
void System16DivideChipWrite(int which, int offset, UINT16 data);

int System16Init();
int System16Exit();
int System16AFrame();
int System16BFrame();
int System18Frame();
int HangonFrame();
int HangonYM2203Frame();
int OutrunFrame();
int XBoardFrame();
int YBoardFrame();
int System16Scan(int nAction,int *pnMin);

// d_sys16a.cpp
void System16APPI0WritePortA(UINT8 data);
void System16APPI0WritePortB(UINT8 data);
void System16APPI0WritePortC(UINT8 data);
extern unsigned short __fastcall System16AReadWord(unsigned int a);
extern unsigned char __fastcall System16AReadByte(unsigned int a);
void __fastcall System16AWriteWord(unsigned int a, unsigned short d);
void __fastcall System16AWriteByte(unsigned int a, unsigned char d);

// d_sys16b.cpp
extern unsigned char __fastcall System16BReadByte(unsigned int a);
void __fastcall System16BWriteByte(unsigned int a, unsigned char d);
void __fastcall System16BWriteWord(unsigned int a, unsigned short d);

// d_sys18.cpp
extern unsigned short __fastcall System18ReadWord(unsigned int a);
extern unsigned char __fastcall System18ReadByte(unsigned int a);
void __fastcall System18WriteWord(unsigned int a, unsigned short d);
void __fastcall System18WriteByte(unsigned int a, unsigned char d);

// d_hangon.cpp
void HangonPPI0WritePortA(UINT8 data);
void HangonPPI0WritePortB(UINT8 data);
void HangonPPI0WritePortC(UINT8 data);
UINT8 HangonPPI1ReadPortC();
void HangonPPI1WritePortA(UINT8 data);
extern unsigned short __fastcall HangonReadWord(unsigned int a);
extern unsigned char __fastcall HangonReadByte(unsigned int a);
void __fastcall HangonWriteWord(unsigned int a, unsigned short d);
void __fastcall HangonWriteByte(unsigned int a, unsigned char d);

// d_outrun.cpp
void OutrunPPI0WritePortC(UINT8 data);
extern unsigned short __fastcall OutrunReadWord(unsigned int a);
extern unsigned char __fastcall OutrunReadByte(unsigned int a);
void __fastcall OutrunWriteWord(unsigned int a, unsigned short d);
void __fastcall OutrunWriteByte(unsigned int a, unsigned char d);
extern unsigned char __fastcall Outrun2ReadByte(unsigned int a);
void __fastcall Outrun2WriteWord(unsigned int a, unsigned short d);
void __fastcall Outrun2WriteByte(unsigned int a, unsigned char d);

// d_xbrd.cpp
extern unsigned short __fastcall XBoardReadWord(unsigned int a);
extern unsigned char __fastcall XBoardReadByte(unsigned int a);
void __fastcall XBoardWriteWord(unsigned int a, unsigned short d);
void __fastcall XBoardWriteByte(unsigned int a, unsigned char d);
extern unsigned short __fastcall XBoard2ReadWord(unsigned int a);
extern unsigned char __fastcall XBoard2ReadByte(unsigned int a);
void __fastcall XBoard2WriteWord(unsigned int a, unsigned short d);
void __fastcall XBoard2WriteByte(unsigned int a, unsigned char d);

// d_ybrd.cpp
extern unsigned short __fastcall YBoardReadWord(unsigned int a);
extern unsigned char __fastcall YBoardReadByte(unsigned int a);
void __fastcall YBoardWriteWord(unsigned int a, unsigned short d);
void __fastcall YBoardWriteByte(unsigned int a, unsigned char d);
extern unsigned short __fastcall YBoard2ReadWord(unsigned int a);
void __fastcall YBoard2WriteWord(unsigned int a, unsigned short d);
extern unsigned short __fastcall YBoard3ReadWord(unsigned int a);
extern unsigned char __fastcall YBoard3ReadByte(unsigned int a);
void __fastcall YBoard3WriteWord(unsigned int a, unsigned short d);

// sys16_gfx.cpp
extern int System16VideoEnable;
extern int System18VdpEnable;
extern int System18VdpMixing;
extern int System16ScreenFlip;
extern int System16SpriteShadow;
extern int System16SpriteXOffset;
extern int System16SpriteBanks[16];
extern int System16TileBanks[8];
extern int System16OldTileBanks[8];
extern int System16Page[4];
extern int System16OldPage[4];
extern unsigned char BootlegFgPage[4];
extern unsigned char BootlegBgPage[4];
extern int System16ScrollX[4];
extern int System16ScrollY[4];
extern int System16ColScroll;
extern int System16RowScroll;
extern int System16RoadControl;
extern int System16RoadColorOffset1;
extern int System16RoadColorOffset2;
extern int System16RoadColorOffset3;
extern int System16RoadXOffset;
extern int System16RoadPriority;
extern int System16PaletteEntries;
extern int System16TilemapColorOffset;
extern int System16TileBankSize;
extern int System16RecalcBgTileMap;
extern int System16RecalcBgAltTileMap;
extern int System16RecalcFgTileMap;
extern int System16RecalcFgAltTileMap;
extern int System16CreateOpaqueTileMaps;
extern int System16IgnoreVideoEnable;

extern bool bSystem16BootlegRender;

extern unsigned short *pTempDraw;

void System16Decode8x8Tiles(unsigned char *pTile, int Num, int offs1, int offs2, int offs3);
void OutrunDecodeRoad();
void HangonDecodeRoad();
void System16ATileMapsInit(int bOpaque);
void System16BTileMapsInit(int bOpaque);
void System16TileMapsExit();
void System16ARender();
void System16BRender();
void System16BootlegRender();
void System16BAltRender();
void System16ATileByteWrite(unsigned int Offset, unsigned char d);
void System16ATileWordWrite(unsigned int Offset, unsigned short d);
void System16BTileByteWrite(unsigned int Offset, unsigned char d);
void System16BTileWordWrite(unsigned int Offset, unsigned short d);
void System18Render();
void HangonRender();
void HangonAltRender();
void OutrunRender();
void ShangonRender();
void XBoardRender();
void YBoardRender();

// fd1089.cpp
void FD1089Decrypt();

// sys16_fd1094.cpp
void fd1094_driver_init(int nCPU);
void fd1094_machine_init();
void fd1094_exit();

// genesis_vid.cpp


// rf5c68.cpp
void rf5c68_init(int clock);
void rf5c68_update(short* pSoundBuf, int nSegmentLength);
unsigned char rf5c68_read(int offset);
void rf5c68_write(int offset, unsigned char data);
void rf5c68_reg_write(int offset, unsigned char data);
