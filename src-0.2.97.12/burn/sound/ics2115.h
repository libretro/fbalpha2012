extern unsigned char *ICSSNDROM;
extern unsigned int nICSSNDROMLen;

extern unsigned char ics2115read(unsigned char offset);
extern void ics2115write(unsigned char offset, unsigned char data);

extern int ics2115_init();
extern void ics2115_exit();
extern void ics2115_reset();

extern unsigned short ics2115_soundlatch_r(int i);
extern void ics2115_soundlatch_w(int i, unsigned short d);

extern void ics2115_frame();
extern void ics2115_update(int length);
extern void ics2115_scan(int nAction,int *pnMin);
