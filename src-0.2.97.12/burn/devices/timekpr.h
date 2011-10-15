#define TIMEKEEPER_M48T58 ( 1 )
#define TIMEKEEPER_M48T02 ( 2 )
#define TIMEKEEPER_MK48T08 ( 3 )

UINT8 TimeKeeperRead(unsigned int offset);
void TimeKeeperWrite(int offset, UINT8 data);
void TimeKeeperTick();
void TimeKeeperInit(int type, UINT8 *data);
void TimeKeeperScan(int nAction);
