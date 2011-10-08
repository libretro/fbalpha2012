#include "cps.h"
#include "bitswap.h"
// CPS (general)

int Cps = 0;							// 1 = CPS1, 2 = CPS2, 3 = CPS Changer
int Cps1Qs = 0;
int Cps1Pic = 0;
int kludge = 0;							// Game kludges

int nCPS68KClockspeed = 0;
int nCpsCycles = 0;						// 68K Cycles per frame
int	nCpsZ80Cycles;

unsigned char *CpsGfx =NULL; unsigned int nCpsGfxLen =0; // All the graphics
unsigned char *CpsRom =NULL; unsigned int nCpsRomLen =0; // Program Rom (as in rom)
unsigned char *CpsCode=NULL; unsigned int nCpsCodeLen=0; // Program Rom (decrypted)
unsigned char *CpsZRom=NULL; unsigned int nCpsZRomLen=0; // Z80 Roms
char *CpsQSam=NULL; unsigned int nCpsQSamLen=0;	// QSound Sample Roms
unsigned char *CpsAd  =NULL; unsigned int nCpsAdLen  =0; // ADPCM Data
unsigned char *CpsStar=NULL;
unsigned int nCpsGfxScroll[4]={0,0,0,0}; // Offset to Scroll tiles
unsigned int nCpsGfxMask=0;	  // Address mask

void (*pCpsInitCallback)() = NULL;

// ----------------------------------------------------------------

/* Game specific data */
struct CPS1config
{
	const char *name;             /* game driver name */

	/* Some games interrogate a couple of registers on bootup. */
	/* These are CPS1 board B self test checks. They wander from game to */
	/* game. */
	int cpsb_addr;        /* CPS board B test register address */
	int cpsb_value;       /* CPS board B test register expected value */

	/* some games use as a protection check the ability to do 16-bit multiplies */
	/* with a 32-bit result, by writing the factors to two ports and reading the */
	/* result from two other ports. */
	/* It looks like this feature was introduced with 3wonders (CPSB ID = 08xx) */
	int mult_factor1;
	int mult_factor2;
	int mult_result_lo;
	int mult_result_hi;

	int layer_control;
	int priority[4];
	int control_reg;  /* Control register? seems to be always 0x3f */

	/* ideally, the layer enable masks should consist of only one bit, */
	/* but in many cases it is unknown which bit is which. */
	int layer_enable_mask[5];

	int MapperId;

	int kludge;
};

struct CPS1config *cps1_game_config;

/*                     CPSB ID    multiply protection  ctrl     priority masks   unknwn     layer enable masks  */
#define CPS_B_01     0x00,0x0000, 0,0,0,0, /* n/a */   0x66,{0x68,0x6a,0x6c,0x6e},0x70, {0x02,0x04,0x08,0x30,0x30}
#define CPS_B_02     0x60,0x0002, 0,0,0,0, /* n/a */   0x6c,{0x6a,0x68,0x66,0x64},0x62, {0x02,0x04,0x08,0x00,0x00}
#define CPS_B_03     0x00,0x0000, 0,0,0,0, /* n/a */   0x70,{0x6e,0x6c,0x6a,0x68},0x66, {0x20,0x10,0x08,0x00,0x00}
#define CPS_B_04     0x60,0x0004, 0,0,0,0, /* n/a */   0x6e,{0x66,0x70,0x68,0x72},0x6a, {0x02,0x04,0x08,0x00,0x00}
#define CPS_B_05     0x60,0x0005, 0,0,0,0, /* n/a */   0x68,{0x6a,0x6c,0x6e,0x70},0x72, {0x02,0x08,0x20,0x14,0x14}
#define CPS_B_11     0x72,0x0401, 0,0,0,0, /* n/a */   0x66,{0x68,0x6a,0x6c,0x6e},0x70, {0x08,0x10,0x20,0x00,0x00}
#define CPS_B_12     0x60,0x0402, 0,0,0,0, /* n/a */   0x6c,{0x6a,0x68,0x66,0x64},0x62, {0x02,0x04,0x08,0x00,0x00}
#define CPS_B_13     0x6e,0x0403, 0,0,0,0, /* n/a */   0x62,{0x64,0x66,0x68,0x6a},0x6c, {0x20,0x02,0x04,0x00,0x00}
#define CPS_B_14     0x5e,0x0404, 0,0,0,0, /* n/a */   0x52,{0x54,0x56,0x58,0x5a},0x5c, {0x08,0x20,0x10,0x00,0x00}
#define CPS_B_15     0x4e,0x0405, 0,0,0,0, /* n/a */   0x42,{0x44,0x46,0x48,0x4a},0x4c, {0x04,0x02,0x20,0x00,0x00}
#define CPS_B_16     0x40,0x0406, 0,0,0,0, /* n/a */   0x4c,{0x4a,0x48,0x46,0x44},0x42, {0x10,0x0a,0x0a,0x00,0x00}
#define CPS_B_17     0x48,0x0407, 0,0,0,0, /* n/a */   0x54,{0x52,0x50,0x4e,0x4c},0x4a, {0x08,0x10,0x02,0x00,0x00}
#define CPS_B_18     0xd0,0x0408, 0,0,0,0, /* n/a */   0xdc,{0xda,0xd8,0xd6,0xd4},0xd2, {0x10,0x08,0x02,0x00,0x00}
#define CPS_B_21_DEF 0x72,0x0000, 0x40,0x42,0x44,0x46, 0x66,{0x68,0x6a,0x6c,0x6e},0x70, {0x02,0x04,0x08,0x30,0x30}	// pang3 sets layer enable to 0x26 on startup
#define CPS_B_21_BT1 0x72,0x0800, 0x4e,0x4c,0x4a,0x48, 0x68,{0x66,0x64,0x62,0x60},0x70, {0x20,0x04,0x08,0x12,0x12}
#define CPS_B_21_BT2 0x00,0x0000, 0x5e,0x5c,0x5a,0x58, 0x60,{0x6e,0x6c,0x6a,0x68},0x70, {0x30,0x08,0x30,0x00,0x00}
#define CPS_B_21_BT3 0x00,0x0000, 0x46,0x44,0x42,0x40, 0x60,{0x6e,0x6c,0x6a,0x68},0x70, {0x20,0x12,0x12,0x00,0x00}
#define CPS_B_21_BT4 0x00,0x0000, 0x46,0x44,0x42,0x40, 0x68,{0x66,0x64,0x62,0x60},0x70, {0x20,0x10,0x02,0x00,0x00}
#define CPS_B_21_BT5 0x72,0x0000, 0x4e,0x4c,0x4a,0x48, 0x60,{0x6e,0x6c,0x6a,0x68},0x70, {0x20,0x04,0x02,0x00,0x00}
#define CPS_B_21_BT6 0x00,0x0000, 0x00,0x00,0x00,0x00, 0x60,{0x6e,0x6c,0x6a,0x68},0x70, {0x20,0x14,0x14,0x00,0x00}
#define CPS_B_21_BT7 0x00,0x0000, 0x00,0x00,0x00,0x00, 0x6c,{0x00,0x00,0x00,0x00},0x52, {0x14,0x02,0x14,0x00,0x00}
#define CPS_B_21_QS1 0x00,0x0000, 0x00,0x00,0x00,0x00, 0x62,{0x64,0x66,0x68,0x6a},0x6c, {0x10,0x08,0x04,0x00,0x00}
#define CPS_B_21_QS2 0x00,0x0000, 0x00,0x00,0x00,0x00, 0x4a,{0x4c,0x4e,0x40,0x42},0x44, {0x16,0x16,0x16,0x00,0x00}
#define CPS_B_21_QS3 0x4e,0x0c00, 0x00,0x00,0x00,0x00, 0x52,{0x54,0x56,0x48,0x4a},0x4c, {0x04,0x02,0x20,0x00,0x00}
#define CPS_B_21_QS4 0x6e,0x0c01, 0x00,0x00,0x00,0x00, 0x56,{0x40,0x42,0x68,0x6a},0x6c, {0x04,0x08,0x10,0x00,0x00}
#define CPS_B_21_QS5 0x5e,0x0c02, 0x00,0x00,0x00,0x00, 0x6a,{0x6c,0x6e,0x70,0x72},0x5c, {0x04,0x08,0x10,0x00,0x00}
#define HACK_B_1     0x00,0x0000, 0x00,0x00,0x00,0x00, 0x54,{0x52,0x50,0x4e,0x4c},0x5c, {0xff,0xff,0xff,0x00,0x00}
#define HACK_B_2     0x00,0x0000, 0x00,0x00,0x00,0x00, 0x60,{0x68,0x6a,0x6c,0x6e},0x70, {0x02,0x04,0x08,0x00,0x00}
#define HACK_B_3     0x00,0x0000, 0x00,0x00,0x00,0x00, 0xc4,{0x52,0x50,0x4e,0x4c},0x4a, {0x02,0x02,0x08,0x00,0x00}
#define HACK_B_4     0x00,0x0000, 0x00,0x00,0x00,0x00, 0x70,{0x6e,0x6c,0x6a,0x68},0x66, {0x02,0x04,0x08,0x00,0x00}

/******************************************************************************************************************

kludge ID:		effect:

0			nothing
1			ghouls (4-way stick)
2			3wonders (not used)
3			msword & knights (not used)
4			mercs (different scroll 2 layout)
5			pang3 (68k@12mhz, gfx 2M*2 ((0 4 2 6)/1M) ROM_GROUPWORD | ROM_SKIP(6), pang3 eeprom)
6			cawingb (scroll1xoff = 0xffc0, CpsInp008)
7			forgottn (msm6295@6061khz, analog controls)
8			xmcota (not used)
9			ssf2t (not used)
10			sf2hack (68k@12mhz, scroll1xoff = -0x0c, scroll2xoff = -0x0e, scroll3xoff = -0x10, CpsInp019, CpsInp176, use only sprite port 0x910000, draw the sprites in reverse order)
11			wofhack (68k@12mhz, gfx 2M*2 ((0 4 2M0 2M4 2 6 2M2 2M6)/512k) ROM_GROUPWORD | ROM_SKIP(6), scroll1xoff = scroll2xoff = scroll3xoff = 0xffc0, CpsInp006, CpsInp007, CpsInp008, CpsInp009)
12			pichack (68k@12mhz, gfx 512k*4*2 ((0 4 1 5 2 6 3 7)/256k) ROM_SKIP(7))
13			knightsb2 (draw the sprites in reverse order)
14			captcommb (scroll1xoff = -0x08, scroll2xoff = -0x0a, scroll3xoff = -0x0c, gfx 1M*4 ((0 4 2M0 2M4 1 5 2M1 2M5 2 6 2M2 2M6 3 7 2M3 2M7)/256k) ROM_SKIP(7))
15			punipic2 (68k@12mhz, gfx 2M*2 ((0 2M0 4 2M4 2 2M2 6 2M6)/512k) ROM_GROUPWORD | ROM_SKIP(6))
16			fcrash (gfx 128k*4*4 ((0 1 2 3)/128k) ROM_SKIP(3))
17			sf2mdt (68k@12mhz, gfx 512k*4*3 ((0 1 2 3)/512k) ROM_SKIP(3))
18			68k@12mhz
19			kodb (gfx 512k*8 ((0 1 2 3 4 5 6 7)/512k) ROM_SKIP(7))
20			sf2m3 && sf2m8 (68k@12mhz, scroll1xoff = scroll2xoff = scroll3xoff = -0x10, draw the sprites in reverse order)
21			sf2ebbl (68k@10mhz, gfx 512k*4*3 ((0 2 4 6 2M0 2M2 2M4 2M6 4M0 4M2 4M4 4M6)/512k) ROM_GROUPWORD | ROM_SKIP(6) + gfx 128k*4 ((0 4 1 5 2 6 3 7)/64k)  ROM_SKIP(7), scroll1xoff = -0x0c, scroll2xoff = -0x0e, scroll3xoff = -0x10, CpsInp019, CpsInp176, use only sprite port 0x910000, draw the sprites in reverse order)

******************************************************************************************************************/

static struct CPS1config cps1_config_table[]=
{
	/* name        CPSB        gfx mapper   kludge */
	{"forgottn", CPS_B_01,     mapper_LW621, 7 },
	{"forgottnu",CPS_B_01,     mapper_LWCHR, 7 },
	{"lostwrld", CPS_B_01,     mapper_LWCHR, 7 },
	{"lostwrldo",CPS_B_01,     mapper_LWCHR, 7 },
	{"ghouls",   CPS_B_01,     mapper_DM620, 1 },
	{"ghoulsu",  CPS_B_01,     mapper_DM620, 1 },
	{"daimakai", CPS_B_01,     mapper_DM22A, 1 },
	{"daimakair",CPS_B_21_DEF, mapper_DAM63B, 1 },
	{"strider",  CPS_B_01,     mapper_ST24M1, 0 },
	{"striderua",CPS_B_01,     mapper_ST24M1, 0 },
	{"striderj", CPS_B_01,     mapper_ST22B, 0 },
	{"striderjr",CPS_B_01,     mapper_ST24M1, 0 },
	{"dynwar",   CPS_B_02,     mapper_TK22B, 0 },
	{"dynwaru",  CPS_B_02,     mapper_TK22B, 0 },
	{"dynwarj",  CPS_B_02,     mapper_TK22B, 0 },
	{"willow",   CPS_B_03,     mapper_WL24B, 0 },
	{"willowj",  CPS_B_03,     mapper_WL24B, 0 },
	{"willowje", CPS_B_03,     mapper_WL24B, 0 },
	{"fcrash",   CPS_B_04,     mapper_S224B, 16 }, // bootleg
	{"ffight",   CPS_B_04,     mapper_S224B, 0 },
	{"ffightu",  CPS_B_04,     mapper_S224B, 0 },
	{"ffightua", CPS_B_01,     mapper_S224B, 0 },
	{"ffightub", CPS_B_05,     mapper_S224B, 0 }, // I think
	{"ffightj",  CPS_B_04,     mapper_S224B, 0 },
	{"ffightj1", CPS_B_02,     mapper_S224B, 0 },
	{"ffightj2", CPS_B_01,     mapper_S224B, 0 },
	{"ffightjh", CPS_B_01,     mapper_S224B, 0 },
	{"1941",     CPS_B_05,     mapper_YI24B, 0 },
	{"1941j",    CPS_B_05,     mapper_YI24B, 0 },
	{"unsquad",  CPS_B_11,     mapper_AR24B, 0 },	/* CPSB ID not checked, but it's the same as sf2eg */
	{"area88",   CPS_B_11,     mapper_AR22B, 0 },	/* CPSB ID not checked, but it's the same as sf2eg */
	{"mercs",    CPS_B_12,     mapper_0224B, 4 },	/* (uses port 74) */
	{"mercsu",   CPS_B_12,     mapper_0224B, 4 },	/* (uses port 74) */
	{"mercsua",  CPS_B_12,     mapper_0224B, 4 },	/* (uses port 74) */
	{"mercsj",   CPS_B_12,     mapper_0224B, 4 },	/* (uses port 74) */
	{"msword",   CPS_B_13,     mapper_MS24B, 0 },	/* CPSB ID not checked, but it's the same as sf2j */
	{"mswordr1", CPS_B_13,     mapper_MS24B, 0 },	/* CPSB ID not checked, but it's the same as sf2j */
	{"mswordu",  CPS_B_13,     mapper_MS24B, 0 },	/* CPSB ID not checked, but it's the same as sf2j */
	{"mswordj",  CPS_B_13,     mapper_MS24B, 0 },	/* CPSB ID not checked, but it's the same as sf2j */
	{"mtwins",   CPS_B_14,     mapper_CK24B, 0 },
	{"chikij",   CPS_B_14,     mapper_CK24B, 0 },
	{"nemo",     CPS_B_15,     mapper_NM24B, 0 },
	{"nemoj",    CPS_B_15,     mapper_NM24B, 0 },
	{"cawing",   CPS_B_16,     mapper_CA24B, 0 },
	{"cawingr1", CPS_B_16,     mapper_CA24B, 0 },
	{"cawingu",  CPS_B_16,     mapper_CA24B, 0 },
	{"cawingj",  CPS_B_16,     mapper_CA22B, 0 },
	{"sf2",      CPS_B_11,     mapper_STF29, 0 },
	{"sf2eb",    CPS_B_17,     mapper_STF29, 0 },
	{"sf2ebbl",  CPS_B_17,     mapper_STF29, 21 },
	{"sf2ua",    CPS_B_17,     mapper_STF29, 0 },
	{"sf2ub",    CPS_B_17,     mapper_STF29, 0 },
	{"sf2ud",    CPS_B_05,     mapper_STF29, 0 },
	{"sf2ue",    CPS_B_18,     mapper_STF29, 0 },
	{"sf2uf",    CPS_B_15,     mapper_STF29, 0 },
	{"sf2ui",    CPS_B_14,     mapper_STF29, 0 },
	{"sf2uk",    CPS_B_17,     mapper_STF29, 0 }, // check CPS_B
	{"sf2j",     CPS_B_13,     mapper_STF29, 0 },
	{"sf2ja",    CPS_B_17,     mapper_STF29, 0 },
	{"sf2jc",    CPS_B_12,     mapper_STF29, 0 },
	{"sf2qp1",   CPS_B_17,     mapper_STF29, 0 },
	/* from here onwards the CPS-B board has suicide battery and multiply protection */
	{"3wonders", CPS_B_21_BT1, mapper_RT24B, 0 },
	{"3wondersu",CPS_B_21_BT1, mapper_RT24B, 0 },
	{"wonder3",  CPS_B_21_BT1, mapper_RT22B, 0 },
	{"3wondersh",CPS_B_02,     mapper_RT24B, 0 },
	{"kod",      CPS_B_21_BT2, mapper_KD29B, 0 },
	{"kodu",     CPS_B_21_BT2, mapper_KD29B, 0 },
	{"kodj",     CPS_B_21_BT2, mapper_KD29B, 0 },
	{"kodb",     CPS_B_21_BT2, mapper_KD29B, 19 },	/* bootleg, doesn't use multiply protection */
	{"captcomm", CPS_B_21_BT3, mapper_CC63B, 0 },
	{"captcommu",CPS_B_21_BT3, mapper_CC63B, 0 },
	{"captcommj",CPS_B_21_BT3, mapper_CC63B, 0 },
	{"captcommb",CPS_B_21_BT3, mapper_CC63B, 14 },
	{"knights",  CPS_B_21_BT4, mapper_KR63B, 0 },
	{"knightsu", CPS_B_21_BT4, mapper_KR63B, 0 },
	{"knightsj", CPS_B_21_BT4, mapper_KR63B, 0 },
	{"knightsb", CPS_B_21_BT4, mapper_KR63B, 0 },
	{"sf2ce",    CPS_B_21_DEF, mapper_S9263B, 18 },
	{"sf2ceua",  CPS_B_21_DEF, mapper_S9263B, 18 },
	{"sf2ceub",  CPS_B_21_DEF, mapper_S9263B, 18 },
	{"sf2ceuc",  CPS_B_21_DEF, mapper_S9263B, 18 },
	{"sf2cej",   CPS_B_21_DEF, mapper_S9263B, 18 },
	{"sf2rb",    CPS_B_21_DEF, mapper_S9263B, 18 },
	{"sf2rb2",   CPS_B_21_DEF, mapper_S9263B, 18 },
	{"sf2rb3",   CPS_B_21_DEF, mapper_S9263B, 18 },
	{"sf2red",   CPS_B_21_DEF, mapper_S9263B, 18 },
	{"sf2v004",  CPS_B_21_DEF, mapper_S9263B, 18 },
	{"sf2acc",   CPS_B_21_DEF, mapper_S9263B, 18 },
	{"sf2accp2", CPS_B_21_DEF, mapper_S9263B, 18 },
	{"sf2dkot2", CPS_B_21_DEF, mapper_S9263B, 18 },
	{"sf2m1",    CPS_B_21_DEF, mapper_S9263B, 18 },
	{"sf2m2",    CPS_B_21_DEF, mapper_S9263B, 10 },
	{"sf2m3",    HACK_B_3,     mapper_S9263B, 20 },
	{"sf2m4",    HACK_B_1,     mapper_S9263B, 10 },
	{"sf2m5",    CPS_B_21_DEF, mapper_S9263B, 10 },
	{"sf2m6",    CPS_B_21_DEF, mapper_S9263B, 10 },
	{"sf2m7",    CPS_B_21_DEF, mapper_S9263B, 10 },
	{"sf2yyc",   CPS_B_21_DEF, mapper_S9263B, 10 },
	{"sf2koryu", CPS_B_21_DEF, mapper_S9263B, 10 },
	{"sf2mdt",   CPS_B_21_DEF, mapper_S9263B, 17 },
	{"varth",    CPS_B_04,     mapper_VA63B, 18 },	/* CPSB test has been patched out (60=0008) */
	{"varthr1",  CPS_B_04,     mapper_VA63B, 18 },	/* CPSB test has been patched out (60=0008) */
	{"varthu",   CPS_B_04,     mapper_VA63B, 18 },	/* CPSB test has been patched out (60=0008) */
	{"varthj",   CPS_B_21_BT5, mapper_VA22B, 18 },	/* CPSB test has been patched out (72=0001) */
	{"cworld2j", CPS_B_21_BT6, mapper_Q522B, 18 },	/* The 0x76 priority values are incorrect values */
	{"wof",      CPS_B_21_DEF, mapper_TK263B, 18 },	/* bootleg? */
	{"wofa",     CPS_B_21_DEF, mapper_TK263B, 18 },	/* bootleg? */
	{"wofu",     CPS_B_21_QS1, mapper_TK263B, 18 },
	{"wofj",     CPS_B_21_QS1, mapper_TK263B, 18 },
	{"dino",     CPS_B_21_QS2, mapper_CD63B, 18 },	/* layer enable never used */
	{"dinou",    CPS_B_21_QS2, mapper_CD63B, 18 },	/* layer enable never used */
	{"dinoj",    CPS_B_21_QS2, mapper_CD63B, 18 },	/* layer enable never used */
	{"dinopic",  CPS_B_21_QS2, mapper_CD63B, 12 },	/* layer enable never used */
	{"dinopic2", CPS_B_21_QS2, mapper_CD63B, 12 },	/* layer enable never used */
	{"punisher", CPS_B_21_QS3, mapper_PS63B, 18 },
	{"punisheru",CPS_B_21_QS3, mapper_PS63B, 18 },
	{"punisherj",CPS_B_21_QS3, mapper_PS63B, 18 },
	{"punipic",  CPS_B_21_QS3, mapper_PS63B, 12 },
	{"punipic2", CPS_B_21_QS3, mapper_PS63B, 15 },
	{"punipic3", CPS_B_21_QS3, mapper_PS63B, 18 },
	{"slammast", CPS_B_21_QS4, mapper_MB63B, 18 },
	{"slammastu",CPS_B_21_QS4, mapper_MB63B, 18 },
	{"mbomberj", CPS_B_21_QS4, mapper_MB63B, 18 },
	{"mbombrd",  CPS_B_21_QS5, mapper_MB63B, 18 },
	{"mbombrdj", CPS_B_21_QS5, mapper_MB63B, 18 },
	{"sf2hf",    CPS_B_21_DEF, mapper_S9263B, 18 },
	{"sf2t",     CPS_B_21_DEF, mapper_S9263B, 18 },
	{"sf2tj",    CPS_B_21_DEF, mapper_S9263B, 18 },
	{"qad",      CPS_B_21_BT7, mapper_QD22B, 18 },	/* TODO: layer enable */
	{"qadj",     CPS_B_21_DEF, mapper_qadj, 18 },
	{"qtono2",   CPS_B_21_DEF, mapper_qtono2, 18 },
	{"megaman",  CPS_B_21_DEF, mapper_RCM63B, 18 },
	{"rockmanj", CPS_B_21_DEF, mapper_RCM63B, 18 },
	{"pnickj",   CPS_B_21_DEF, mapper_pnickj, 18 },
	{"pang3",    CPS_B_21_DEF, mapper_pang3, 5 },	/* EEPROM port is among the CPS registers */
	{"pang3n",   CPS_B_21_DEF, mapper_pang3, 5 },	/* EEPROM port is among the CPS registers */
	{"pang3j",   CPS_B_21_DEF, mapper_pang3, 5 },	/* EEPROM port is among the CPS registers */

	{"sfzch",    CPS_B_21_DEF, mapper_sfzch, 0 },
	{"sfach",    CPS_B_21_DEF, mapper_sfzch, 0 },

	/* CPS1 hack games */
	{"cawingb",  CPS_B_16,     mapper_CA24B, 6 },
	{"daimakb",  CPS_B_01,     mapper_DM22A, 1 },
	{"kodh",     CPS_B_21_DEF, mapper_KD29B, 0 },	/* bootleg, doesn't use multiply protection */
	{"dinoh",    CPS_B_21_DEF, mapper_CD63B, 18 },	/* layer enable never used */
	{"dinoha",   CPS_B_21_DEF, mapper_CD63B, 18 },	/* layer enable never used */
	{"dinohb",   CPS_B_21_QS2, mapper_CD63B, 18 },	/* layer enable never used */
	{"dinohc",   CPS_B_21_DEF, mapper_CD63B, 18 },	/* layer enable never used */
	{"knightsh", CPS_B_21_DEF, mapper_KR63B, 0 },
	{"knightsb2",CPS_B_21_BT4, mapper_KR63B, 13 },
	{"knightsjb",CPS_B_21_DEF, mapper_KR63B, 0 },
	{"punisherh",CPS_B_21_DEF, mapper_PS63B, 18 },
	{"sf2b",     CPS_B_17,     mapper_STF29, 0 },
	{"sf2m8",    HACK_B_3,     mapper_S9263B, 20 },
	{"sf2m9",    CPS_B_21_DEF, mapper_S9263B, 10 },
	{"sf2m10",   CPS_B_21_DEF, mapper_S9263B, 10 },
	{"sf2m11",   HACK_B_1,     mapper_S9263B, 10 },
	{"sf2m12",   HACK_B_1,     mapper_S9263B, 10 },
	{"sf2m13",   HACK_B_4,     mapper_S9263B, 10 },
	{"sf2tlona", CPS_B_21_DEF, mapper_S9263B, 10 },
	{"sf2tlonb", CPS_B_21_DEF, mapper_S9263B, 10 },
	{"sf2th",    CPS_B_21_DEF, mapper_S9263B, 10 },
	{"wofh",     HACK_B_2,     mapper_TK263B, 11 },
	{"wofha",    HACK_B_2,     mapper_TK263B, 11 },
	{"wofhfh",   CPS_B_21_DEF, mapper_TK263B, 18 },
	{"wof3js",   CPS_B_21_DEF, mapper_TK263B, 18 },
	{"wofsj",    HACK_B_2,     mapper_TK263B, 11 },
	{"wofsja",   HACK_B_2,     mapper_TK263B, 11 },
	{"wofsjb",   CPS_B_21_DEF, mapper_TK263B, 18 },
	{"wof3sj",   HACK_B_2,     mapper_TK263B, 11 },
	{"wof3sja",  HACK_B_2,     mapper_TK263B, 11 },
	{"wofb",     CPS_B_21_DEF, mapper_TK263B, 12 },
	{"sf2ceh",   CPS_B_21_DEF, mapper_S9263B, 18 },
	{"sf2ceb",   CPS_B_21_DEF, mapper_S9263B, 18 },
	{"pnicku",   CPS_B_21_DEF, mapper_pnickj, 18 },
	{"slammasta",CPS_B_21_QS4, mapper_MB63B, 18 },
	{"slammasth",CPS_B_21_QS4, mapper_MB63B, 18 },
	{"mbombrdu", CPS_B_21_QS5, mapper_MB63B, 18 },
	{"mbombrda", CPS_B_21_QS5, mapper_MB63B, 18 },
	{"mbombrdh", CPS_B_21_QS5, mapper_MB63B, 18 },
	{"mbombrdje",CPS_B_21_QS5, mapper_MB63B, 18 },

	{0,          CPS_B_21_DEF, mapper_cps2,   0 }, // default
};

// ----------------------------------------------------------------
// Get info stored in table above & make it useful to FBA

void CpsGetInfo()
{
	struct CPS1config *pCFG = &cps1_config_table[0];

	// Set variables
	{
		while(pCFG->name)
		{
			if (strcmp(pCFG->name, BurnDrvGetTextA(DRV_NAME)) == 0)
			{
				break;
			}
			pCFG++;
		}
		cps1_game_config = pCFG;

		// Layer control register
		nCpsLcReg = cps1_game_config->layer_control;

		// Layer enable
		CpsLayEn[1] = cps1_game_config->layer_enable_mask[0];
		CpsLayEn[2] = cps1_game_config->layer_enable_mask[1];
		CpsLayEn[3] = cps1_game_config->layer_enable_mask[2];
		CpsLayEn[4] = cps1_game_config->layer_enable_mask[3];
		CpsLayEn[5] = cps1_game_config->layer_enable_mask[4];

		// Multiply protection
		CpsMProt[0] = cps1_game_config->mult_factor1;
		CpsMProt[1] = cps1_game_config->mult_factor2;
		CpsMProt[2] = cps1_game_config->mult_result_lo;
		CpsMProt[3] = cps1_game_config->mult_result_hi;

		// Priority Masks
		MaskAddr[0] = cps1_game_config->priority[0];
		MaskAddr[1] = cps1_game_config->priority[1];
		MaskAddr[2] = cps1_game_config->priority[2];
		MaskAddr[3] = cps1_game_config->priority[3];

		SetGfxMapper(cps1_game_config->MapperId);

		if (Cps == 2) {
			nCpsGfxScroll[1] = nCpsGfxScroll[2] = nCpsGfxScroll[3] = 0x800000;
		}

		if (Ssf2 | Ssf2t)
		{
			nCpsGfxScroll[3] = 0;
		}

		// Board ID test
		CpsBID[0] = cps1_game_config->cpsb_addr;
		CpsBID[1] = (cps1_game_config->cpsb_value >> 8) & 0xff;
		CpsBID[2] = (cps1_game_config->cpsb_value >> 0) & 0xff;

		kludge = cps1_game_config->kludge;
	}
}

// ----------------------------------------------------------------

// Separate out the bits of a byte
static inline unsigned int Separate(unsigned int b)
{
	unsigned int a = b;									// 00000000 00000000 00000000 11111111
	a  =((a & 0x000000F0) << 12) | (a & 0x0000000F);	// 00000000 00001111 00000000 00001111
	a = ((a & 0x000C000C) <<  6) | (a & 0x00030003);	// 00000011 00000011 00000011 00000011
	a = ((a & 0x02020202) <<  3) | (a & 0x01010101);	// 00010001 00010001 00010001 00010001

	return a;
}

// Precalculated table of the Separate function
static unsigned int SepTable[256];

static int SepTableCalc()
{
	static int bDone = 0;
	if (bDone)
		return 0;										// Already done it

	for (uint32_t i = 0; i < 256; i++)
		SepTable[i] = Separate(255 - i);

	bDone = 1;											// done it
	return 0;
}

// Allocate space and load up a rom
static int LoadUp(unsigned char** pRom, int* pnRomLen, int nNum)
{
	unsigned char *Rom;
	struct BurnRomInfo ri;

	ri.nLen = 0;
	BurnDrvGetRomInfo(&ri, nNum);	// Find out how big the rom is
	if (ri.nLen <= 0) {
		return 1;
	}

	// Load the rom
	Rom = (unsigned char*)malloc(ri.nLen);
	if (Rom == NULL)
		return 1;

	if (BurnLoadRom(Rom,nNum,1)) {
		free(Rom);
		return 1;
	}

	// Success
	*pRom = Rom; *pnRomLen = ri.nLen;
	return 0;
}

static int LoadUpSplit(unsigned char** pRom, int* pnRomLen, int nNum)
{
	unsigned char *Rom;
	struct BurnRomInfo ri;
	unsigned int nRomSize[4], nTotalRomSize;
	int i;

	ri.nLen = 0;
	for (i = 0; i < 4; i++)
	{
		BurnDrvGetRomInfo(&ri, nNum + i);
		nRomSize[i] = ri.nLen;
	}

	nTotalRomSize = nRomSize[0] + nRomSize[1] + nRomSize[2] + nRomSize[3];
	if (!nTotalRomSize) return 1;

	Rom = (unsigned char*)malloc(nTotalRomSize);
	if (Rom == NULL) return 1;

	int Offset = 0;
	for (i = 0; i < 4; i++) {
		if (i > 0) Offset += nRomSize[i - 1];
		if (BurnLoadRom(Rom + Offset, nNum + i, 1)) {
			free(Rom);
			return 1;
		}
	}

	*pRom = Rom;
	*pnRomLen = nTotalRomSize;

	return 0;
}

// ----------------------------CPS1--------------------------------
// Load 1 rom and interleave in the CPS style:
// rom  : aa bb
// --ba --ba --ba --ba --ba --ba --ba --ba 8 pixels (four bytes)
//                                                  (skip four bytes)

static int CpsLoadOne(unsigned char* Tile, int nNum, int nWord, int nShift)
{
	unsigned char *Rom = NULL; int nRomLen=0;
	unsigned char *pt = NULL, *pr = NULL;
	int i;

	LoadUp(&Rom, &nRomLen, nNum);
	if (Rom == NULL)
		return 1;

	nRomLen &= ~1;								// make sure even

	for (i = 0, pt = Tile, pr = Rom; i < nRomLen; pt += 8) {
		unsigned int Pix;						// Eight pixels
		unsigned char b;
		b = *pr++; i++; Pix = SepTable[b];
		if (nWord) {
			b = *pr++; i++; Pix |= SepTable[b] << 1;
		}

		Pix <<= nShift;
		*((unsigned int *)pt) |= Pix;
	}

	free(Rom);
	return 0;
}

static int CpsLoadOneHack160(unsigned char *Tile, int nNum, int nWord, int nOffset)
{
	int i = 0;
	unsigned char *Rom1 = NULL, *Rom2 = NULL;
	int nRomLen1 = 0, nRomLen2 = 0;
	unsigned char *pt = NULL, *pr = NULL;

	LoadUp(&Rom1, &nRomLen1, nNum);
	if (Rom1 == NULL)
		return 1;

	LoadUp(&Rom2, &nRomLen2, nNum + 1);
	if (Rom2 == NULL)
		return 1;

	for (i = 0, pt = Tile, pr = Rom1 + (nRomLen1/4 * nOffset); i < nRomLen1/4; pt += 8) {
		unsigned int Pix;		// Eight pixels
		unsigned char b;
		b = *pr++; i++; Pix = SepTable[b];
		if (nWord)
			b = *pr++; i++; Pix |= SepTable[b] << 1;

		Pix <<= 0;
		*((unsigned int *)pt) |= Pix;
	}

	for (i = 0, pt = Tile, pr = Rom2 + (nRomLen2/4 * nOffset); i < nRomLen2/4; pt += 8) {
		unsigned int Pix;		// Eight pixels
		unsigned char b;
		b = *pr++; i++; Pix = SepTable[b];
		if (nWord)
			b = *pr++; i++; Pix |= SepTable[b] << 1;

		Pix <<= 2;
		*((unsigned int *)pt) |= Pix;
	}

	free(Rom2);
	free(Rom1);
	return 0;
}

static int CpsLoadOneHack(unsigned char *Tile, int nNum, int nWord, int nOffset)
{
	int i = 0;
	unsigned char *Rom1 = NULL, *Rom2 = NULL, *Rom3 = NULL, *Rom4 = NULL;
	int nRomLen1 = 0, nRomLen2 = 0, nRomLen3 = 0, nRomLen4 = 0;
	unsigned char *pt = NULL, *pr = NULL;

	LoadUp(&Rom1, &nRomLen1, nNum);
	if (Rom1 == NULL)
		return 1;

	LoadUp(&Rom2, &nRomLen2, nNum + 1);

	if (Rom2 == NULL)
		return 1;
	LoadUp(&Rom3, &nRomLen3, nNum + 2);

	if (Rom3 == NULL)
		return 1;
	LoadUp(&Rom4, &nRomLen4, nNum + 3);

	if (Rom4 == NULL)
		return 1;

	for (i = 0, pt = Tile, pr = Rom1 + (nRomLen1/2 * nOffset); i < nRomLen1/2; pt += 8) {
		unsigned int Pix;		// Eight pixels
		unsigned char b;
		b = *pr++; i++; Pix = SepTable[b];
		if (nWord)
			b = *pr++; i++; Pix |= SepTable[b] << 1;

		Pix <<= 0;
		*((unsigned int *)pt) |= Pix;
	}

	for (i = 0, pt = Tile, pr = Rom2 + (nRomLen2/2 * nOffset); i < nRomLen2/2; pt += 8) {
		unsigned int Pix;		// Eight pixels
		unsigned char b;
		b = *pr++; i++; Pix = SepTable[b];
		if (nWord)
			b = *pr++; i++; Pix |= SepTable[b] << 1;

		Pix <<= 1;
		*((unsigned int *)pt) |= Pix;
	}

	for (i = 0, pt = Tile, pr = Rom3 + (nRomLen3/2 * nOffset); i < nRomLen3/2; pt += 8) {
		unsigned int Pix;		// Eight pixels
		unsigned char b;
		b = *pr++; i++; Pix = SepTable[b];
		if (nWord)
			b = *pr++; i++; Pix |= SepTable[b] << 1;

		Pix <<= 2;
		*((unsigned int *)pt) |= Pix;
	}

	for (i = 0, pt = Tile, pr = Rom4 + (nRomLen4/2 * nOffset); i < nRomLen4/2; pt += 8) {
		unsigned int Pix;		// Eight pixels
		unsigned char b;
		b = *pr++; i++; Pix = SepTable[b];
		if (nWord)
			b = *pr++; i++; Pix |= SepTable[b] << 1;

		Pix <<= 3;
		*((unsigned int *)pt) |= Pix;
	}

	free(Rom4);
	free(Rom3);
	free(Rom2);
	free(Rom1);
	return 0;
}

static int CpsLoadOneBoot(unsigned char *Tile, int nNum, int nWord, int nShift)
{
	unsigned char *Rom = NULL; int nRomLen=0;
	unsigned char *pt = NULL, *pr = NULL;
	int i;

	LoadUp(&Rom, &nRomLen, nNum);

	if (Rom == NULL)
		return 1;

	nRomLen &= ~1;								// make sure even

	for (i = 0, pt = Tile, pr = Rom; i < nRomLen; pt += 4) {
		unsigned int Pix;						// Eight pixels
		unsigned char b;
		b = *pr++; i++; Pix = SepTable[b];
		if (nWord)
			b = *pr++; i++; Pix |= SepTable[b] << 1;

		Pix <<= nShift;
		*((unsigned int *)pt) |= Pix;
	}

	free(Rom);
	return 0;
}

static int CpsLoadOnePang(unsigned char *Tile,int nNum,int nWord,int nShift)
{
	int i=0;
	unsigned char *Rom = NULL; int nRomLen = 0;
	unsigned char *pt = NULL, *pr = NULL;

	LoadUp(&Rom, &nRomLen, nNum);
	if (Rom == NULL) {
		return 1;
	}

	nRomLen &= ~1; // make sure even

	for (i = 0x100000, pt = Tile, pr = Rom + 0x100000; i < nRomLen; pt += 8) {
		unsigned int Pix; // Eight pixels
		unsigned char b;
		b = *pr++; i++; Pix = SepTable[b];
		if (nWord) {
			b = *pr++; i++; Pix |= SepTable[b] << 1;
		}

		Pix <<= nShift;
		*((unsigned int *)pt) |= Pix;
	}

	free(Rom);
	return 0;
}

int CpsLoadTiles(unsigned char* Tile, int nStart)
{
	// left  side of 16x16 tiles
	CpsLoadOne(Tile,     nStart    , 1, 0);
	CpsLoadOne(Tile,     nStart + 1, 1, 2);
	// right side of 16x16 tiles
	CpsLoadOne(Tile + 4, nStart + 2, 1, 0);
	CpsLoadOne(Tile + 4, nStart + 3, 1, 2);
	return 0;
}

int CpsLoadTilesByte(unsigned char* Tile, int nStart)
{
	CpsLoadOne(Tile,     nStart + 0, 0, 0);
	CpsLoadOne(Tile,     nStart + 1, 0, 1);
	CpsLoadOne(Tile,     nStart + 2, 0, 2);
	CpsLoadOne(Tile,     nStart + 3, 0, 3);
	CpsLoadOne(Tile + 4, nStart + 4, 0, 0);
	CpsLoadOne(Tile + 4, nStart + 5, 0, 1);
	CpsLoadOne(Tile + 4, nStart + 6, 0, 2);
	CpsLoadOne(Tile + 4, nStart + 7, 0, 3);
	return 0;
}

int CpsLoadTilesForgottnAlt(unsigned char* Tile, int nStart)
{
	CpsLoadOne(Tile + 0 + 0x000000, nStart +  0, 0, 0);
	CpsLoadOne(Tile + 0 + 0x000000, nStart +  1, 0, 1);
	CpsLoadOne(Tile + 0 + 0x000000, nStart +  2, 1, 2);
	CpsLoadOne(Tile + 4 + 0x000000, nStart +  3, 1, 0);
	CpsLoadOne(Tile + 4 + 0x000000, nStart +  4, 0, 2);
	CpsLoadOne(Tile + 4 + 0x000000, nStart +  5, 0, 3);
	CpsLoadOne(Tile + 0 + 0x100000, nStart +  6, 0, 0);
	CpsLoadOne(Tile + 0 + 0x100000, nStart +  7, 0, 1);
	CpsLoadOne(Tile + 4 + 0x100000, nStart +  8, 0, 2);
	CpsLoadOne(Tile + 4 + 0x100000, nStart +  9, 0, 3);
	CpsLoadOne(Tile + 0 + 0x200000, nStart + 10, 1, 0);
	CpsLoadOne(Tile + 0 + 0x200000, nStart + 11, 0, 2);
	CpsLoadOne(Tile + 0 + 0x200000, nStart + 12, 0, 3);
	CpsLoadOne(Tile + 4 + 0x200000, nStart + 13, 1, 0);
	CpsLoadOne(Tile + 4 + 0x200000, nStart + 14, 0, 2);
	CpsLoadOne(Tile + 4 + 0x200000, nStart + 15, 0, 3);
	CpsLoadOne(Tile + 0 + 0x300000, nStart + 16, 0, 2);
	CpsLoadOne(Tile + 0 + 0x300000, nStart + 17, 0, 3);
	CpsLoadOne(Tile + 4 + 0x300000, nStart + 18, 0, 2);
	CpsLoadOne(Tile + 4 + 0x300000, nStart + 19, 0, 3);
	return 0;
}

int CpsLoadTilesPang(unsigned char* Tile, int nStart)
{
	CpsLoadOne(    Tile,     nStart,     1, 0);
	CpsLoadOne(    Tile,     nStart + 1, 1, 2);
	CpsLoadOnePang(Tile + 4, nStart,     1, 0);
	CpsLoadOnePang(Tile + 4, nStart + 1, 1, 2);
	return 0;
}

int CpsLoadTilesHack160(unsigned char* Tile, int nStart)
{
	CpsLoadOneHack160(Tile + 0 + 0x000000, nStart, 1, 0);
	CpsLoadOneHack160(Tile + 4 + 0x000000, nStart, 1, 1);
	CpsLoadOneHack160(Tile + 0 + 0x200000, nStart, 1, 2);
	CpsLoadOneHack160(Tile + 4 + 0x200000, nStart, 1, 3);
	return 0;
}

int CpsLoadTilesHack160a(unsigned char* Tile, int nStart)
{
	CpsLoadOneHack160(Tile + 0 + 0x000000, nStart, 1, 0);
	CpsLoadOneHack160(Tile + 0 + 0x200000, nStart, 1, 1);
	CpsLoadOneHack160(Tile + 4 + 0x000000, nStart, 1, 2);
	CpsLoadOneHack160(Tile + 4 + 0x200000, nStart, 1, 3);
	return 0;
}

int CpsLoadTilesHack(unsigned char* Tile, int nStart)
{
	CpsLoadOneHack(Tile,     nStart, 0, 0);
	CpsLoadOneHack(Tile + 4, nStart, 0, 1);
	return 0;
}

int CpsLoadTilesBoot(unsigned char* Tile, int nStart)
{
	CpsLoadOneHack(Tile + 0 + 0x000000, nStart, 0, 0);
	CpsLoadOneHack(Tile + 4 + 0x000000, nStart, 0, 1);
	CpsLoadOneHack(Tile + 0 + 0x200000, nStart, 0, 2);
	CpsLoadOneHack(Tile + 4 + 0x200000, nStart, 0, 3);
	return 0;
}

int CpsLoadTilesBoot2(unsigned char* Tile, int nStart)
{
	CpsLoadOneBoot(Tile, nStart,     0, 0);
	CpsLoadOneBoot(Tile, nStart + 1, 0, 1);
	CpsLoadOneBoot(Tile, nStart + 2, 0, 2);
	CpsLoadOneBoot(Tile, nStart + 3, 0, 3);
	return 0;
}

int CpsLoadStars(unsigned char* pStar, int nStart)
{
	unsigned char* pTemp[2] = { NULL, NULL};
	int nLen;

	for (int i = 0; i < 2; i++) {
		if (LoadUp(&pTemp[i], &nLen, nStart + (i << 1))) {
			free(pTemp[0]);
			free(pTemp[1]);
		}
	}

	for (int i = 0; i < 0x1000; i++) {
		pStar[i] = pTemp[0][i << 1];
		pStar[0x01000 + i] = pTemp[1][i << 1];
	}

	free(pTemp[0]);
	free(pTemp[1]);

	return 0;
}

int CpsLoadStarsByte(unsigned char* pStar, int nStart)
{
	unsigned char* pTemp[2] = { NULL, NULL};
	int nLen;

	for (int i = 0; i < 2; i++) {
		if (LoadUp(&pTemp[i], &nLen, nStart + (i * 4))) {
			free(pTemp[0]);
			free(pTemp[1]);
		}
	}

	for (int i = 0; i < 0x1000; i++) {
		pStar[i] = pTemp[0][i];
		pStar[0x01000 + i] = pTemp[1][i];
	}

	free(pTemp[0]);
	free(pTemp[1]);

	return 0;
}

int CpsLoadStarsForgottnAlt(unsigned char* pStar, int nStart)
{
	unsigned char* pTemp[2] = { NULL, NULL};
	int nLen;

	for (int i = 0; i < 2; i++) {
		if (LoadUp(&pTemp[i], &nLen, nStart + (i * 3))) {
			free(pTemp[0]);
			free(pTemp[1]);
		}
	}

	for (int i = 0; i < 0x1000; i++) {
		pStar[i] = pTemp[0][i << 1];
		pStar[0x01000 + i] = pTemp[1][i << 1];
	}

	free(pTemp[0]);
	free(pTemp[1]);

	return 0;
}

// ----------------------------CPS2--------------------------------
// Load 1 rom and interleave in the CPS2 style:
// rom  : aa bb -- -- (4 bytes)
// --ba --ba --ba --ba --ba --ba --ba --ba 8 pixels (four bytes)
//                                                  (skip four bytes)

// memory 000000-100000 are in even word fields of first 080000 section
// memory 100000-200000 are in  odd word fields of first 080000 section
// i = ABCD nnnn nnnn nnnn nnnn n000
// s = 00AB Cnnn nnnn nnnn nnnn nnD0

static inline void Cps2Load100000(unsigned char* Tile, unsigned char* Sect, int nShift)
{
	unsigned char *pt, *pEnd, *ps;
	pt = Tile; pEnd = Tile + 0x100000; ps = Sect;

	do {
		unsigned int Pix;				// Eight pixels
		Pix  = SepTable[ps[0]];
		Pix |= SepTable[ps[1]] << 1;
		Pix <<= nShift;
		*((unsigned int*)pt) |= Pix;

		pt += 8; ps += 4;
	}
	while (pt < pEnd);
}

static int Cps2LoadOne(unsigned char* Tile, int nNum, int nWord, int nShift)
{
	unsigned char *Rom = NULL; int nRomLen = 0;
	unsigned char *pt, *pr;

	LoadUp(&Rom, &nRomLen, nNum);
	if (Rom == NULL) {
		return 1;
	}

	if (nWord == 0) {
		unsigned char*Rom2 = NULL; int nRomLen2 = 0;
		unsigned char*Rom3 = Rom;

		LoadUp(&Rom2, &nRomLen2, nNum + 1);
		if (Rom2 == NULL) {
			return 1;
		}

		nRomLen <<= 1;
		Rom = (unsigned char*)malloc(nRomLen);
		if (Rom == NULL) {
			free(Rom2);
			free(Rom3);
			return 1;
		}

		for (int i = 0; i < nRomLen2; i++) {
			Rom[(i << 1) + 0] = Rom3[i];
			Rom[(i << 1) + 1] = Rom2[i];
		}

		free(Rom2);
		free(Rom3);
	}

	// Go through each section
	pt = Tile; pr = Rom;
	for (int b = 0; b < nRomLen >> 19; b++) {
		Cps2Load100000(pt, pr,     nShift); pt += 0x100000;
		Cps2Load100000(pt, pr + 2, nShift); pt += 0x100000;
		pr += 0x80000;
	}

	free(Rom);

	return 0;
}

static int Cps2LoadSplit(unsigned char* Tile, int nNum, int nShift)
{
	unsigned char *Rom = NULL; int nRomLen = 0;
	unsigned char *pt, *pr;

	LoadUpSplit(&Rom, &nRomLen, nNum);
	if (Rom == NULL) {
		return 1;
	}

	// Go through each section
	pt = Tile; pr = Rom;
	for (int b = 0; b < nRomLen >> 19; b++) {
		Cps2Load100000(pt, pr,     nShift); pt += 0x100000;
		Cps2Load100000(pt, pr + 2, nShift); pt += 0x100000;
		pr += 0x80000;
	}

	free(Rom);

	return 0;
}

int Cps2LoadTiles(unsigned char* Tile, int nStart)
{
	// left  side of 16x16 tiles
	Cps2LoadOne(Tile,     nStart,     1, 0);
	Cps2LoadOne(Tile,     nStart + 1, 1, 2);
	// right side of 16x16 tiles
	Cps2LoadOne(Tile + 4, nStart + 2, 1, 0);
	Cps2LoadOne(Tile + 4, nStart + 3, 1, 2);

	return 0;
}

int Cps2LoadTilesSplit(unsigned char* Tile, int nStart)
{
	// left  side of 16x16 tiles
	Cps2LoadSplit(Tile,     nStart +  0, 0);
	Cps2LoadSplit(Tile,     nStart +  4, 2);
	// right side of 16x16 tiles
	Cps2LoadSplit(Tile + 4, nStart +  8, 0);
	Cps2LoadSplit(Tile + 4, nStart + 12, 2);

	return 0;
}

int Cps2LoadTilesSIM(unsigned char* Tile, int nStart)
{
	Cps2LoadOne(Tile,     nStart,     0, 0);
	Cps2LoadOne(Tile,     nStart + 2, 0, 2);
	Cps2LoadOne(Tile + 4, nStart + 4, 0, 0);
	Cps2LoadOne(Tile + 4, nStart + 6, 0, 2);

	return 0;
}

// ----------------------------------------------------------------

// The file extension indicates the data contained in a file.
// it consists of 2 numbers optionally followed by a single letter.
// The letter indicates the version. The meaning for the nubmers
// is as follows:
// 01 - 02 : Z80 program
// 03 - 10 : 68K program (filenames ending with x contain the XOR table)
// 11 - 12 : QSound sample data
// 13 - nn : Graphics data

static unsigned int nGfxMaxSize;

static int CpsGetROMs(bool bLoad)
{
	char* pRomName;
	struct BurnRomInfo ri;

//	unsigned char* CpsCodeLoad = CpsCode;
	unsigned char* CpsRomLoad = CpsRom;
	unsigned char* CpsGfxLoad = CpsGfx;
	unsigned char* CpsZRomLoad = CpsZRom;
	unsigned char* CpsQSamLoad = (unsigned char*)CpsQSam;

	int nGfxNum = 0;

	if (bLoad)
	{
		if (/*!CpsCodeLoad || */!CpsRomLoad || !CpsGfxLoad || !CpsZRomLoad || !CpsQSamLoad)
			return 1;
	} else {
		nCpsCodeLen = nCpsRomLen = nCpsGfxLen = nCpsZRomLen = nCpsQSamLen = 0;

		nGfxMaxSize = 0;
		if (BurnDrvGetHardwareCode() & HARDWARE_CAPCOM_CPS2_SIMM)
			nGfxMaxSize = ~0U;
	}

	for (int i = 0; !BurnDrvGetRomName(&pRomName, i, 0); i++) {

		BurnDrvGetRomInfo(&ri, i);

		// SIMM Graphics ROMs
		if (BurnDrvGetHardwareCode() & HARDWARE_CAPCOM_CPS2_SIMM) {
			if ((ri.nType & BRF_GRA) && (ri.nType & 8)) {
				if (bLoad) {
					Cps2LoadTilesSIM(CpsGfxLoad, i);
					CpsGfxLoad += ri.nLen * 8;
					i += 7;
				}
				else
					nCpsGfxLen += ri.nLen;
				continue;
			}

			// SIMM QSound sample ROMs
			if ((ri.nType & BRF_SND) && (ri.nType & 8)) {
				if (bLoad) {
					if (ri.nType & 2) {
						BurnLoadRom(CpsQSamLoad + 0, i + 0, 2);
						BurnLoadRom(CpsQSamLoad + 1, i + 1, 2);
						i++;
					} else {
						BurnLoadRom(CpsQSamLoad, i, 1);
						BurnByteswap(CpsQSamLoad, ri.nLen);
					}
					CpsQSamLoad += ri.nLen;
				} else {
					nCpsQSamLen += ri.nLen;
				}
				continue;
			}
		}

		// 68K program ROMs
		if ((ri.nType & 7) == 1) {
			if (bLoad) {
				BurnLoadRom(CpsRomLoad, i, 1);
				CpsRomLoad += ri.nLen;
			}
			else
				nCpsRomLen += ri.nLen;
			continue;
		}

		// Z80 program ROMs
		if ((ri.nType & 7) == 4) {
			if (bLoad) {
				BurnLoadRom(CpsZRomLoad, i, 1);
				CpsZRomLoad += ri.nLen;
			}
			else
				nCpsZRomLen += ri.nLen;
			continue;
		}

		// Normal Graphics ROMs
		if (ri.nType & BRF_GRA) {
			if (bLoad) {
				if ((ri.nType & 15) == 6) {
					Cps2LoadTilesSplit(CpsGfxLoad, i);
					CpsGfxLoad += (nGfxMaxSize == ~0U ? ri.nLen : nGfxMaxSize) * 4;
					i += 15;
				} else {
					Cps2LoadTiles(CpsGfxLoad, i);
					CpsGfxLoad += (nGfxMaxSize == ~0U ? ri.nLen : nGfxMaxSize) * 4;
					i += 3;
				}
			} else {
				if (ri.nLen > nGfxMaxSize)
					nGfxMaxSize = ri.nLen;

				if (ri.nLen < nGfxMaxSize)
					nGfxMaxSize = ~0U;

				nCpsGfxLen += ri.nLen;
				nGfxNum++;
			}
			continue;
		}

		// QSound sample ROMs
		if (ri.nType & BRF_SND) {
			if (bLoad) {
				BurnLoadRom(CpsQSamLoad, i, 1);
				BurnByteswap(CpsQSamLoad, ri.nLen);
				CpsQSamLoad += ri.nLen;
			}
			else
				nCpsQSamLen += ri.nLen;
			continue;
		}
	}

	if (bLoad) {
		cps2_decryption();	// Decrypt program code
	} else {
		cps2_decrypt_init();	// Find encrypted program code length

		if (nGfxMaxSize != ~0U)
			nCpsGfxLen = nGfxNum * nGfxMaxSize;

#if 1 && defined FBA_DEBUG
		bprintf(PRINT_IMPORTANT, _T("  - 68K ROM size:\t0x%08X\n"), nCpsRomLen);
		bprintf(PRINT_IMPORTANT, _T("  - Z80 ROM size:\t0x%08X\n"), nCpsZRomLen);
		bprintf(PRINT_IMPORTANT, _T("  - Graphics data:\t0x%08X\n"), nCpsGfxLen);
		bprintf(PRINT_IMPORTANT, _T("  - QSound data:\t0x%08X\n"), nCpsQSamLen);
#endif
		// omit checking nCpsCodeLen for phoenix rom
		if (!nCpsRomLen || !nCpsGfxLen || !nCpsZRomLen || !nCpsQSamLen/* || !nCpsCodeLen*/)
			return 1;
	}

	return 0;
}

// ----------------------------------------------------------------

int Cps1GetRoms(bool bLoad)
{
	int i = 0;

	struct BurnRomInfo ri;
	struct BurnRomInfo ni;

	unsigned char *CpsRomLoad = CpsRom;
	unsigned char *CpsGfxLoad = CpsGfx;
	unsigned char *CpsZRomLoad = CpsZRom;
	unsigned char *CpsAdLoad = CpsAd;
	unsigned char *CpsQSamLoad = (unsigned char*)CpsQSam;

	if (bLoad)
	{
		if (!CpsRomLoad || !CpsGfxLoad || !CpsZRomLoad || (Cps1Qs && !nCpsQSamLen))
			return 1;
	}
	else
		nCpsCodeLen = nCpsRomLen = nCpsGfxLen = nCpsZRomLen = nCpsQSamLen = nCpsAdLen = 0;

	while (1)
	{
		ri.nLen = 0;
		BurnDrvGetRomInfo(&ri, i);
		if (ri.nLen == 0)
			break;

		if ((ri.nType & 7) == 1)	// Load 68k prg roms
		{
			if (bLoad) {
				ni.nLen = 0;
				BurnDrvGetRomInfo(&ni, i+1);

				if (ri.nLen < 0x080000 && ni.nLen == ri.nLen)
				{
					BurnLoadRom(CpsRomLoad + 1, i + 0, 2);
					BurnLoadRom(CpsRomLoad + 0, i + 1, 2);
					CpsRomLoad += ri.nLen << 1; i++;
				} else {
					BurnLoadRom(CpsRomLoad + 0, i + 0, 1);
					CpsRomLoad += ri.nLen;
				}
			} else {
				nCpsRomLen += ri.nLen;
			}
		}

		if ((ri.nType & 7) == 2)	// Load gfx roms
		{
			if (bLoad)
			{
				if (ri.nLen == 0x080000 && kludge != 12 && kludge != 17 && kludge != 19)
				{
					CpsLoadTiles(CpsGfxLoad, i);
					CpsGfxLoad += 4 * ri.nLen; i += 3;
				}
				else if ((ri.nLen == 0x080000 && kludge == 12) || (ri.nLen == 0x020000 && kludge == 21)) // pic bootlegs && sf2ebbl
				{
					CpsLoadTilesHack(CpsGfxLoad, i);
					CpsGfxLoad += 4 * ri.nLen; i += 3;
				}
				else if (ri.nLen == 0x020000 && kludge == 7) // forgottn
				{
					CpsLoadTilesForgottnAlt(CpsGfxLoad, i);
					CpsGfxLoad += 32 * ri.nLen; i += 19;
				}
				else if ((ri.nLen == 0x020000 && kludge == 16) || (ri.nLen == 0x080000 && kludge == 17)) // fcrash && sf2mdt
				{
					CpsLoadTilesBoot2(CpsGfxLoad, i);
					CpsGfxLoad += 4 * ri.nLen; i += 3;
				}
				else if ((ri.nLen == 0x020000 && kludge != 7 && kludge != 16 && kludge != 21) || (ri.nLen == 0x080000 && kludge == 19) || ri.nLen == 0x010000 || ri.nLen == 0x040000)
				{
					CpsLoadTilesByte(CpsGfxLoad, i);
					CpsGfxLoad += 8 * ri.nLen; i += 7;
				}
				else if (ri.nLen == 0x100000 && kludge == 14) // captcommb
				{
					CpsLoadTilesBoot(CpsGfxLoad, i);
					CpsGfxLoad += 4 * ri.nLen; i += 3;
				}
				else if (ri.nLen == 0x200000 && kludge != 5 && kludge != 15) // wofhack bootlegs
				{
					CpsLoadTilesHack160(CpsGfxLoad, i);
					CpsGfxLoad += 2 * ri.nLen; i += 1;
				}
				else if (ri.nLen == 0x200000 && kludge == 15) // punipic2
				{
					CpsLoadTilesHack160a(CpsGfxLoad, i);
					CpsGfxLoad += 2 * ri.nLen; i += 1;
				}
				else if (ri.nLen == 0x200000 && kludge == 5) // Pang! 3
				{
					CpsLoadTilesPang(CpsGfxLoad, i);
					CpsGfxLoad += 2 * ri.nLen; i += 1;
				}
			} else {
				nCpsGfxLen += ri.nLen;
			}
		}

		if ((ri.nType & 7) == 3)	// Load z80 prg roms
		{
			if (bLoad)
			{
				BurnLoadRom(CpsZRomLoad, i, 1);
				CpsZRomLoad += ri.nLen;
			} else {
				nCpsZRomLen += ri.nLen;
			}
		}

		if ((ri.nType & 7) == 4)	// Load PCM roms
		{
			if (bLoad)
			{
				BurnLoadRom(CpsAdLoad, i, 1);
				CpsAdLoad += ri.nLen;
			} else {
				nCpsAdLen += ri.nLen;
			}
		}

		if ((ri.nType & 7) == 5)	// Load Q-Sound roms
		{
			Cps1Qs = 1;

			if (bLoad)
			{
				BurnLoadRom(CpsQSamLoad, i, 1);
				CpsQSamLoad += ri.nLen;
			} else {
				nCpsQSamLen += ri.nLen;
			}
		}

		i++;
	}

	if (bLoad)
	{
		CpsStar = CpsGfx + nCpsGfxLen - 0x2000; // Set stars region
	} else {
		nCpsZRomLen <<= 1;
		if (kludge == 5) nCpsGfxLen <<= 1;	// Double gfx size for Pang! 3

		if (!nCpsRomLen || !nCpsGfxLen || !nCpsZRomLen || (Cps1Qs && !nCpsQSamLen))
			return 1;
	}

	return 0;
}

int Cps2Init()
{
	Cps = 2; // Cps 2

	CpsGetInfo();

	if (CpsGetROMs(false))
		return 1;

	int nMemLen, i;

	BurnSetRefreshRate(80000/51.8/25.9);

	if (!nCPS68KClockspeed)
		nCPS68KClockspeed = 11800000;

	nCPS68KClockspeed = nCPS68KClockspeed * 100 / nBurnFPS;

	nMemLen = nCpsGfxLen + nCpsRomLen + nCpsCodeLen + nCpsZRomLen + nCpsQSamLen + nCpsAdLen;

	// Allocate space for Program and data roms
	CpsGfx = (unsigned char*)malloc(nMemLen);

	if (CpsGfx == NULL)
		return 1;

	memset(CpsGfx, 0, nMemLen);

	CpsRom  = CpsGfx + nCpsGfxLen;
	CpsCode = CpsRom + nCpsRomLen;

	CpsZRom = CpsCode + nCpsCodeLen;

	CpsQSam = (char*)(CpsZRom + nCpsZRomLen);
	CpsAd   = (unsigned char*)(CpsQSam + nCpsQSamLen);

	// Create Gfx addr mask
	for (i = 0; i < 31; i++)
	{
		if ((1 << i) >= (int)nCpsGfxLen)
			break;
	}
	nCpsGfxMask = (1 << i) - 1;

#if 0	// Really needed anymore?
	if (nCpsZRomLen >= 5) {
		// 77->cfff and rst 00 in case driver doesn't load
		unsigned char Z80hack[8] = { 0x3e, 0x77, 0x32, 0xff, 0xcf, 0xc7 };
		memcpy (CpsZRom, Z80hack, 8);
	}
#endif

	SepTableCalc();					// Precalc the separate table

	CpsReset = 0; Cpi01A = Cpi01C = Cpi01E = 0;	// blank other inputs

	if (CpsGetROMs(true))
		return 1;

	if (pCpsInitCallback)
		pCpsInitCallback();

	return Cps2RunInit();
}

int CpsInit()
{
	// figure out what system we're looking at
	int flag = (BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK);

	switch (flag) {
		case HARDWARE_CAPCOM_CPS1:
		case HARDWARE_CAPCOM_CPS1_QSOUND:
		case HARDWARE_CAPCOM_CPS1_GENERIC:
			Cps = 1; // Cps 1
			break;

		case HARDWARE_CAPCOM_CPSCHANGER:
			Cps = 3; // Cps Changer (Cps 1)
			break;

		case HARDWARE_CAPCOM_CPS2:
			Cps = 2; // Cps 2
			break;
	}

	CpsGetInfo();

	if (Cps == 2)
	{
		if (CpsGetROMs(false))
			return 1;
	}
	else
	{
		if (Cps1GetRoms(false))
			return 1;
		nCpsGfxLen += 0x2000;	// Extra space for "Stars"
	}

	int nMemLen, i;

	if (Cps == 2)
		BurnSetRefreshRate(80000/51.8/25.9);
	else
		BurnSetRefreshRate(59.61);

	if (!nCPS68KClockspeed)
	{
		if (Cps == 2)
			nCPS68KClockspeed = 11800000;
		else
		{
			if (kludge == 5 || kludge == 10 || kludge == 11 || kludge == 12 || kludge == 15 || kludge == 17 || kludge == 18 || kludge == 20)
				nCPS68KClockspeed = 12000000;
			else
				nCPS68KClockspeed = 10000000;
		}
	}
	nCPS68KClockspeed = nCPS68KClockspeed * 100 / nBurnFPS;

	nMemLen = nCpsGfxLen + nCpsRomLen + nCpsCodeLen + nCpsZRomLen + nCpsQSamLen + nCpsAdLen;

	if (Cps1Qs == 1)
		nMemLen += nCpsZRomLen;

	// Allocate space for Program and data roms
	CpsGfx = (unsigned char*)malloc(nMemLen);

	if (CpsGfx == NULL)
		return 1;

	memset(CpsGfx, 0, nMemLen);

	CpsRom  = CpsGfx + nCpsGfxLen;
	CpsCode = CpsRom + nCpsRomLen;

	if (Cps1Qs == 1)
	{
		CpsEncZRom = CpsCode + nCpsCodeLen;
		CpsZRom = CpsEncZRom + nCpsZRomLen;
	}
	else
		CpsZRom = CpsCode + nCpsCodeLen;

	CpsQSam = (char*)(CpsZRom + nCpsZRomLen);
	CpsAd   = (unsigned char*)(CpsQSam + nCpsQSamLen);

	// Create Gfx addr mask
	for (i = 0; i < 31; i++)
	{
		if ((1 << i) >= (int)nCpsGfxLen)
			break;
	}
	nCpsGfxMask = (1 << i) - 1;

#if 0	// Really needed anymore?
	if (nCpsZRomLen >= 5) {
		// 77->cfff and rst 00 in case driver doesn't load
		unsigned char Z80hack[8] = { 0x3e, 0x77, 0x32, 0xff, 0xcf, 0xc7 };
		memcpy (CpsZRom, Z80hack, 8);
	}
#endif

	SepTableCalc();									  // Precalc the separate table

	CpsReset = 0; Cpi01A = Cpi01C = Cpi01E = 0;		  // blank other inputs

	if (Cps == 2)
	{
		if (CpsGetROMs(true))
			return 1;
	}
	else
	{
		if (Cps1GetRoms(true))
			return 1;
		nCpsGfxLen -= 0x2000;	// stars
	}

	if (pCpsInitCallback)
		pCpsInitCallback();

	return CpsRunInit();
}

int CpsExit()
{
	CpsRunExit();

	CpsLayEn[1] = CpsLayEn[2] = CpsLayEn[3] = CpsLayEn[4] = CpsLayEn[5] = 0;
	nCpsLcReg = 0;
	nCpsGfxScroll[1] = nCpsGfxScroll[2] = nCpsGfxScroll[3] = 0;
	nCpsGfxMask = 0;

	Scroll1TileMask = 0;
	Scroll2TileMask = 0;
	Scroll3TileMask = 0;

	free(BootlegSpriteRam);
	BootlegSpriteRam = NULL;

	nCpsCodeLen = nCpsRomLen = nCpsGfxLen = nCpsZRomLen = nCpsQSamLen = nCpsAdLen = 0;
	CpsCode = CpsRom = CpsZRom = CpsAd = CpsStar = NULL;
	CpsQSam = NULL;

	free(CpsGfx);
	CpsGfx = NULL;

	nCPS68KClockspeed = 0;
	Cps = 0;
	Cps1Qs = 0;
	Cps1QsHack = 0;

	pCpsInitCallback = NULL;
	kludge = 0;

	Pzloop2 = 0;
	Kodb = 0;

	return 0;
}
