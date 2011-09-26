extern unsigned char MegadriveReset;
extern unsigned char bMegadriveRecalcPalette;
extern unsigned char MegadriveJoy1[12];
extern unsigned char MegadriveJoy2[12];
extern unsigned char MegadriveDIP[2];
extern unsigned short *MegadriveCurPal;

int MegadriveInit();
int MegadriveNoByteswapInit();
int MegadriveSsf2Init();
int MegadriveRiseRealDumpInit();
int MegadriveF22RealDumpInit();
int MegadriveBackup_0x200000_0x800_Init();
int MegadriveBackup_0x200000_0x2000_Init();
int MegadriveBackup_0x200000_0x4000_Init();
int MegadriveNoByteswapBackup_0x200000_0x4000_Init();
int MegadriveBackup_0x200000_0x10000_Init();
int MegadriveNoByteswapBackup_0x200000_0x10000_Init();
int MegadriveBackup_0x300000_0x10000_Init();
int MegadriveBackup_Sks3_Init();
int MegadriveBackup_Sks3_0x4000_Init();
int RadicaInit();

int MegadriveExit();
int MegadriveFrame();
int MegadriveScan(int nAction, int *pnMin);
