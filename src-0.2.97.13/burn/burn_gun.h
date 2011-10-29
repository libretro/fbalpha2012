#define MAX_GUNS	4

extern int nBurnGunNumPlayers;

extern int BurnGunX[MAX_GUNS];
extern int BurnGunY[MAX_GUNS];

unsigned char BurnGunReturnX(int num);
unsigned char BurnGunReturnY(int num);

extern void BurnGunInit(int nNumPlayers, bool bDrawTargets);
void BurnGunExit();
void BurnGunScan();
extern void BurnGunDrawTarget(int num, int x, int y);
extern void BurnGunMakeInputs(int num, short x, short y);
