// CPS ----------------------------------
#include "burnint.h"

#include "msm6295.h"
#include "eeprom.h"
#include "timer.h"

// Maximum number of beam-synchronized interrupts to check
#define MAX_RASTER 10

extern unsigned int CpsMProt[4];									// Mprot changes
extern unsigned int CpsBID[3];										// Board ID changes

// cps.cpp
extern int Cps;														// 1 = CPS1, 2 = CPS2, 3 = CPS CHanger
extern int Cps1Qs;
extern int Cps1Pic;
extern int nCPS68KClockspeed;
extern int nCpsCycles;												// Cycles per frame
extern int nCpsZ80Cycles;
extern unsigned char *CpsGfx;  extern unsigned int nCpsGfxLen;		// All the graphics
extern unsigned char *CpsRom;  extern unsigned int nCpsRomLen;		// Program Rom (as in rom)
extern unsigned char *CpsCode; extern unsigned int nCpsCodeLen;		// Program Rom (decrypted)
extern unsigned char *CpsZRom; extern unsigned int nCpsZRomLen;		// Z80 Roms
extern          char *CpsQSam; extern unsigned int nCpsQSamLen;		// QSound Sample Roms
extern unsigned char *CpsAd;   extern unsigned int nCpsAdLen;		// ADPCM Data
extern unsigned int nCpsGfxScroll[4];								// Offset to Scroll tiles
extern unsigned int nCpsGfxMask;									// Address mask
extern unsigned char* CpsStar;
int CpsInit();
int Cps2Init();
int CpsExit();
int CpsLoadTiles(unsigned char *Tile,int nStart);
int CpsLoadTilesByte(unsigned char *Tile,int nStart);
int CpsLoadTilesForgottnAlt(unsigned char* Tile, int nStart);
int CpsLoadTilesPang(unsigned char *Tile,int nStart);
int CpsLoadTilesHack160(unsigned char *Tile,int nStart);
int CpsLoadTilesBootleg(unsigned char *Tile, int nStart);
int CpsLoadTilesCaptcomb(unsigned char *Tile, int nStart);
int CpsLoadTilesPunipic2(unsigned char *Tile, int nStart);
int CpsLoadTilesSf2ebbl(unsigned char *Tile, int nStart);
int CpsLoadStars(unsigned char *pStar, int nStart);
int CpsLoadStarsByte(unsigned char *pStar, int nStart);
int CpsLoadStarsForgottnAlt(unsigned char *pStar, int nStart);
int Cps2LoadTiles(unsigned char *Tile,int nStart);
int Cps2LoadTilesSIM(unsigned char *Tile,int nStart);

// cps_config.h
#define CPS_B_01		0
#define CPS_B_02		1
#define CPS_B_03		2
#define CPS_B_04		3
#define CPS_B_05		4
#define CPS_B_11		5
#define CPS_B_12		6
#define CPS_B_13		7
#define CPS_B_14		8
#define CPS_B_15		9
#define CPS_B_16		10
#define CPS_B_17		11
#define CPS_B_18		12
#define CPS_B_21_DEF		13
#define CPS_B_21_BT1		14
#define CPS_B_21_BT2		15
#define CPS_B_21_BT3		16
#define CPS_B_21_BT4		17
#define CPS_B_21_BT5		18
#define CPS_B_21_BT6		19
#define CPS_B_21_BT7		20
#define CPS_B_21_QS1		21
#define CPS_B_21_QS2		22
#define CPS_B_21_QS3		23
#define CPS_B_21_QS4		24
#define CPS_B_21_QS5		25
#define HACK_B_1		26
#define HACK_B_2		27
#define HACK_B_3		28
#define HACK_B_4		29

#define GFXTYPE_SPRITES		(1<<0)
#define GFXTYPE_SCROLL1		(1<<1)
#define GFXTYPE_SCROLL2		(1<<2)
#define GFXTYPE_SCROLL3		(1<<3)
#define GFXTYPE_STARS		(1<<4)

#define mapper_LWCHR		0
#define mapper_LW621		1
#define mapper_DM620		2
#define mapper_ST24M1		3
#define mapper_DM22A		4
#define mapper_DAM63B		5
#define mapper_ST22B		6
#define mapper_TK22B		7
#define mapper_WL24B		8
#define mapper_S224B		9
#define mapper_YI24B		10
#define mapper_AR24B		11
#define mapper_AR22B		12
#define mapper_O224B		13
#define mapper_MS24B		14
#define mapper_CK24B		15
#define mapper_NM24B		16
#define mapper_CA24B		17
#define mapper_CA22B		18
#define mapper_STF29		19
#define mapper_RT24B		20
#define mapper_RT22B		21
#define mapper_KD29B		22
#define mapper_CC63B		23
#define mapper_KR63B		24
#define mapper_S9263B		25
#define mapper_VA63B		26
#define mapper_VA22B		27
#define mapper_Q522B		28
#define mapper_TK263B		29
#define mapper_CD63B		30
#define mapper_PS63B		31
#define mapper_MB63B		32
#define mapper_QD22B		33
#define mapper_QD63B		34
#define mapper_qtono2		35
#define mapper_RCM63B		36
#define mapper_PKB10B		37
#define mapper_pang3		38
#define mapper_sfzch		39
#define mapper_cps2		40
#define mapper_frog		41
extern void SetGfxMapper(int MapperId);
extern int GfxRomBankMapper(int Type, int Code);
extern void SetCpsBId(int CpsBId, int bStars);

// cps_pal.cpp
extern unsigned int* CpsPal;										// Hicolor version of palette
extern unsigned int* CpsObjPal;										// Pointer to lagged obj palette
extern int nLagObjectPalettes;										// Lag object palettes by one frame if non-zero
int CpsPalInit();
int CpsPalExit();
int CpsPalUpdate(unsigned char *pNewPal,int bRecalcAll);
int CpsStarPalUpdate(unsigned char* pNewPal, int nLayer, int bRecalcAll);

// cps_mem.cpp
extern unsigned char *CpsRam90;
extern unsigned char *CpsZRamC0,*CpsZRamF0;
extern unsigned char *CpsSavePal;
extern unsigned char *CpsRam708,*CpsReg,*CpsFrg;
extern unsigned char *CpsSaveReg[MAX_RASTER + 1];
extern unsigned char *CpsSaveFrg[MAX_RASTER + 1];
extern unsigned char *CpsRamFF;
void CpsMapObjectBanks(int nBank);
int CpsMemInit();
int CpsMemExit();
int CpsAreaScan(int nAction,int *pnMin);

// cps_run.cpp
extern unsigned char CpsReset;
extern unsigned char Cpi01A, Cpi01C, Cpi01E;
extern int nIrqLine50, nIrqLine52;								// The scanlines at which the interrupts are triggered
extern int CpsDrawSpritesInReverse;
int CpsRunInit();
int CpsRunExit();
int Cps1Frame();
int Cps2Frame();

inline static unsigned char* CpsFindGfxRam(int nAddr,int nLen)
{
  nAddr&=0xffffff; // 24-bit bus
  if (nAddr>=0x900000 && nAddr+nLen<=0x930000) return CpsRam90+nAddr-0x900000;
  return NULL;
}

// cps_rw.cpp
// Treble Winner - Added INP(1FD) for sf2ue
#define CPSINPSET INP(000) INP(001) INP(006) INP(007) INP(008) INP(010) INP(011) INP(012) INP(018) INP(019) INP(020) INP(021) INP(029) INP(176) INP(177) INP(179) INP(186) INP(1fd)

// prototype for input bits
#define INP(nnn) extern unsigned char CpsInp##nnn[8];
CPSINPSET
#undef  INP

#define INP(nnn) extern unsigned char Inp##nnn;
CPSINPSET
#undef  INP

#define CPSINPEX INP(c000) INP(c001) INP(c002) INP(c003)

#define INP(nnnn) extern unsigned char CpsInp##nnnn[8];
CPSINPEX
#undef  INP

// For the Forgotten Worlds analog controls
extern unsigned short CpsInp055, CpsInp05d;
extern unsigned short CpsInpPaddle1, CpsInpPaddle2;

extern int PangEEP;
extern int Forgottn;
extern int Cps1QsHack;
extern int Kodh;
extern int Cawingb;
extern int Wofh;
extern int Sf2thndr;
extern int Pzloop2;
extern int Ssf2tb;
extern int Dinopic;
extern int Dinohunt;
extern int Port6SoundWrite;

extern unsigned char* CpsEncZRom;

int CpsRwInit();
int CpsRwExit();
int CpsRwGetInp();
unsigned char __fastcall CpsReadByte(unsigned int a);
void __fastcall CpsWriteByte(unsigned int a, unsigned char d);
unsigned short __fastcall CpsReadWord(unsigned int a);
void __fastcall CpsWriteWord(unsigned int a, unsigned short d);

// cps_draw.cpp
extern unsigned char CpsRecalcPal;				// Flag - If it is 1, recalc the whole palette
extern int nCpsLcReg;							// Address of layer controller register
extern int CpsLayEn[6];							// bits for layer enable
extern int nStartline, nEndline;				// specify the vertical slice of the screen to render
extern int nRasterline[MAX_RASTER + 2];			// The lines at which an interrupt occurs
extern int MaskAddr[4];
extern int CpsLayer1XOffs;
extern int CpsLayer2XOffs;
extern int CpsLayer3XOffs;
extern int CpsLayer1YOffs;
extern int CpsLayer2YOffs;
extern int CpsLayer3YOffs;
void DrawFnInit();
int  CpsDraw();
int  CpsRedraw();

int QsndInit();
void QsndExit();
void QsndReset();
void QsndNewFrame();
void QsndEndFrame();
void QsndSyncZ80();
int QsndScan(int nAction);

// qs_z.cpp
int QsndZInit();
int QsndZExit();
int QsndZScan(int nAction);

// qs_c.cpp
int QscInit(int nRate, int nVolumeShift);
void QscReset();
void QscExit();
int QscScan(int nAction);
void QscNewFrame();
void QscWrite(int a, int d);
int QscUpdate(int nEnd);

// cps_tile.cpp
extern unsigned int* CpstPal;
extern unsigned int nCpstType; extern int nCpstX,nCpstY;
extern unsigned int nCpstTile; extern int nCpstFlip;
extern short* CpstRowShift;
extern unsigned int CpstPmsk; // Pixel mask

inline static void CpstSetPal(int nPal)
{
	nPal <<= 4;
	nPal &= 0x7F0;
	CpstPal= CpsPal + nPal;
}

// ctv.cpp
extern int nBgHi;
extern unsigned short  ZValue;
extern unsigned short *ZBuf;
extern unsigned short *pZVal;
extern unsigned int    nCtvRollX,nCtvRollY;
extern unsigned char  *pCtvTile;					// Pointer to tile data
extern int             nCtvTileAdd;					// Amount to add after each tile line
extern unsigned char  *pCtvLine;					// Pointer to output bitmap
typedef int (*CtvDoFn)();
typedef int (*CpstOneDoFn)();
extern CtvDoFn CtvDoX[0x20];
extern CtvDoFn CtvDoXM[0x20];
extern CtvDoFn CtvDoXB[0x20];
extern CpstOneDoFn CpstOneDoX[3];
extern CpstOneDoFn CpstOneObjDoX[2];
int CtvReady();

// nCpstType constants
// To get size do (nCpstType & 24) + 8
#define CTT_FLIPX ( 1)
#define CTT_CARE  ( 2)
#define CTT_ROWS  ( 4)
#define CTT_8X8   ( 0)
#define CTT_16X16 ( 8)
#define CTT_32X32 (24)

// cps_obj.cpp
extern int nCpsObjectBank;

extern unsigned char *BootlegSpriteRam;

extern int Sf2Hack;

int  CpsObjInit();
int  CpsObjExit();
int  CpsObjGet();
void CpsObjDrawInit();
int  Cps1ObjDraw(int nLevelFrom,int nLevelTo);
int  Cps2ObjDraw(int nLevelFrom,int nLevelTo);

// cps_scr.cpp
#define SCROLL_2 0
#define SCROLL_3 1
extern int Ghouls;
extern int Mercs;
extern int Sf2jc;
extern int Ssf2t;
extern int Qad;
extern int Xmcota;

extern int Scroll1TileMask;
extern int Scroll2TileMask;
extern int Scroll3TileMask;
int Cps1Scr1Draw(unsigned char *Base,int sx,int sy);
int Cps1Scr3Draw(unsigned char *Base,int sx,int sy);
int Cps2Scr1Draw(unsigned char *Base,int sx,int sy);
int Cps2Scr3Draw(unsigned char *Base,int sx,int sy);

// cpsr.cpp
extern unsigned char *CpsrBase;						// Tile data base
extern int nCpsrScrX,nCpsrScrY;						// Basic scroll info
extern unsigned short *CpsrRows;					// Row scroll table, 0x400 words long
extern int nCpsrRowStart;							// Start of row scroll (can wrap?)

// Information needed to draw a line
struct CpsrLineInfo {
	int nStart;										// 0-0x3ff - where to start drawing tiles from
	int nWidth;										// 0-0x400 - width of scroll shifts
													// e.g. for no rowscroll at all, nWidth=0
	int nTileStart;									// Range of tiles which are visible onscreen
	int nTileEnd;									// (e.g. 0x20 -> 0x50 , wraps around to 0x10)
	short Rows[16];									// 16 row scroll values for this line
	int nMaxLeft, nMaxRight;						// Maximum row shifts left and right
};
extern struct CpsrLineInfo CpsrLineInfo[15];
int Cps1rPrepare();
int Cps2rPrepare();

// cpsrd.cpp
int Cps1rRender();
int Cps2rRender();

// dc_input.cpp
extern struct BurnInputInfo CpsFsi[0x1B];

// ps.cpp
extern unsigned char PsndCode, PsndFade;			// Sound code/fade sent to the z80 program
int PsndInit();
int PsndExit();
void PsndNewFrame();
int PsndSyncZ80(int nCycles);
int PsndScan(int nAction);

// ps_z.cpp
int PsndZInit();
int PsndZExit();
int PsndZScan(int nAction);
extern int Kodb;

// ps_m.cpp
extern int bPsmOkay;								// 1 if the module is okay
int PsmInit();
int PsmExit();
void PsmNewFrame();
int PsmUpdate(int nEnd);

// kabuki.cpp
void wof_decode();
void dino_decode();
void punisher_decode();
void slammast_decode();

// cps2_crypt.cpp
void cps2_decrypt_game_data();
