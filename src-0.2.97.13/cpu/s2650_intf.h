
#define MAX_S2650	4
extern int nActiveS2650;
extern int s2650Count;

void s2650_write(unsigned short address, unsigned char data);
unsigned char s2650_read(unsigned short address);
unsigned char s2650_fetch(unsigned short address);
void s2650_write_port(unsigned short port, unsigned char data);
unsigned char s2650_read_port(unsigned short port);

void s2650_write_rom(unsigned short address, unsigned char data);

typedef int (*s2650irqcallback)(int);
extern s2650irqcallback s2650_irqcallback[MAX_S2650];
void s2650SetIrqCallback(int (*irqcallback)(int));

void s2650_open(int num);
void s2650_close();
void s2650_init(int num);
void s2650_exit();
void s2650_reset(void);
void s2650_set_irq_line(int irqline, int state);
int s2650_get_pc();

#define S2650_READ	1
#define S2650_WRITE	2
#define S2650_FETCH	4
#define S2650_ROM	(S2650_READ | S2650_FETCH)
#define S2650_RAM	(S2650_ROM | S2650_WRITE)

void s2650MapMemory(unsigned char *src, int start, int end, int type);

void s2650SetWriteHandler(void (*write)(unsigned short, unsigned char));
void s2650SetReadHandler(unsigned char (*read)(unsigned short));
void s2650SetOutHandler(void (*write)(unsigned short, unsigned char));
void s2650SetInHandler(unsigned char (*read)(unsigned short));

int s2650Run(int cycles);
#define s2650Reset()	 s2650_reset()
void s2650Open(int num);
void s2650Close();
void s2650Exit();
void s2650Init(int num);
int s2650GetPc();

int s2650GetActive();

int s2650Scan(int nAction,int */*pnMin*/);

/* fake control port   M/~IO=0 D/~C=0 E/~NE=0 */
#define S2650_CTRL_PORT 0x100

/* fake data port      M/~IO=0 D/~C=1 E/~NE=0 */
#define S2650_DATA_PORT 0x101

/* extended i/o ports  M/~IO=0 D/~C=x E/~NE=1 */
#define S2650_EXT_PORT	0xff

/* Fake Sense Line */
#define S2650_SENSE_PORT 0x102
