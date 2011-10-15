#define SEGA_MD_ROM_LOAD_NORMAL										0x10
#define SEGA_MD_ROM_LOAD16_WORD_SWAP								0x20
#define SEGA_MD_ROM_LOAD16_BYTE										0x30
#define SEGA_MD_ROM_LOAD16_WORD_SWAP_CONTINUE_040000_100000			0x40
#define SEGA_MD_ROM_OFFS_000000										0x01
#define SEGA_MD_ROM_OFFS_000001										0x02
#define SEGA_MD_ROM_OFFS_020000										0x03
#define SEGA_MD_ROM_OFFS_080000										0x04
#define SEGA_MD_ROM_OFFS_100000										0x05
#define SEGA_MD_ROM_OFFS_100001										0x06
#define SEGA_MD_ROM_OFFS_200000										0x07

extern unsigned char MegadriveReset;
extern unsigned char bMegadriveRecalcPalette;
extern unsigned char MegadriveJoy1[12];
extern unsigned char MegadriveJoy2[12];
extern unsigned char MegadriveJoy3[12];
extern unsigned char MegadriveJoy4[12];
extern unsigned char MegadriveDIP[2];
extern unsigned short *MegadriveCurPal;

int MegadriveInit();
int MegadriveExit();
int MegadriveFrame();
int MegadriveScan(int nAction, int *pnMin);
