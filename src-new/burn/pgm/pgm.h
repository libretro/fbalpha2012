#include "tiles_generic.h"
#include "arm7_intf.h"
#include "ics2115.h"

#define HARDWARE_IGS_JAMMAPCB		0x0002

#ifdef LSB_FIRST
#define SWAP_WORD_LSB(x)	(((x&0xff)<<8)|((x>>8)&0xff))
#else
#define SWAP_WORD_LSB(x)	(x)
#endif

// pgm_run
extern int nPGM68KROMLen;
extern int nPGMSPRColMaskLen;
extern int nPGMSPRMaskMaskLen;
extern int nPGMTileROMLen;
extern int nPGMExternalARMLen;

extern unsigned char *PGM68KRAM;
extern unsigned char *PGM68KROM;
extern unsigned char *PGMTileROM;
extern unsigned char *PGMTileROMExp;
extern unsigned char *PGMSPRColROM;
extern unsigned char *PGMSPRMaskROM;
extern unsigned char *PGMARMROM;
extern unsigned char *PGMUSER0;
extern unsigned char *PGMARMRAM0;
extern unsigned char *PGMARMRAM1;
extern unsigned char *PGMARMRAM2;
extern unsigned char *PGMARMShareRAM;
extern unsigned char *PGMARMShareRAM2;
extern unsigned short *PGMRowRAM;
extern unsigned short *PGMPalRAM;
extern unsigned short *PGMVidReg;
extern unsigned short *PGMSprBuf;
extern unsigned int *PGMBgRAM;
extern unsigned int *PGMTxtRAM;
extern unsigned int *RamCurPal;
extern unsigned char nPgmPalRecalc;

extern unsigned char PgmJoy1[];
extern unsigned char PgmJoy2[];
extern unsigned char PgmJoy3[];
extern unsigned char PgmJoy4[];
extern unsigned char PgmBtn1[];
extern unsigned char PgmBtn2[];
extern unsigned char PgmInput[];
extern unsigned char PgmReset;

extern void (*pPgmInitCallback)();
extern void (*pPgmResetCallback)();
extern int (*pPgmScanCallback)(int, int*);
extern void (*pPgmProtCallback)();

extern int nPGMEnableIRQ4;
extern int nPGMArm7Type;

int pgmInit();
int pgmExit();
int pgmFrame();
int pgmScan(int nAction, int *pnMin);

// pgm_draw
void pgmInitDraw();
void pgmExitDraw();
int pgmDraw();

// pgm_prot
void install_protection_asic3_orlegend();
void install_protection_asic25_asic12_dw2();
void install_protection_asic25_asic22_killbld();
void install_protection_asic25_asic28_olds();
void install_protection_asic27_kov();
void install_protection_asic27a_kovsh();
void install_protection_asic27a_martmast();
void install_protection_asic27a_oldsplus();
void install_protection_asic27a_puzlstar();
void install_protection_asic27a_ddp2();
void install_protection_asic27a_svg();
void install_protection_asic27a_ketsui();
void install_protection_asic27a_ddp3();

// pgm_crypt
void pgm_decrypt_kov();
void pgm_decrypt_kovsh();
void pgm_decrypt_kovshp();
void pgm_decrypt_puzzli2();
void pgm_decrypt_dw2();
void pgm_decrypt_photoy2k();
void pgm_decrypt_puzlstar();
void pgm_decrypt_dw3();
void pgm_decrypt_killbld();
void pgm_decrypt_dfront();
void pgm_decrypt_ddp2();
void pgm_decrypt_martmast();
void pgm_decrypt_kov2();
void pgm_decrypt_kov2p();
void pgm_decrypt_theglad();
void pgm_decrypt_killbldp();
void pgm_decrypt_oldsplus();
void pgm_decrypt_svg();
void pgm_decrypt_happy6in1();
void pgm_decrypt_dw2001();
void pgm_decrypt_py2k2();
void pgm_decrypt_espgaluda();
void pgm_decrypt_ketsui();

void pgm_decode_kovqhsgs_gfx_block(unsigned char *src);
void pgm_decode_kovqhsgs_tile_data(unsigned char *source);
void pgm_decrypt_kovqhsgs();
void pgm_decrypt_kovlsqh2();
void pgm_decrypt_kovassg();
