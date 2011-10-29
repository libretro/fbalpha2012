
// h6280.cpp
void h6280_set_irq_line(int irqline, int state);
void h6280_init(int (*irqcallback)(int));

// h6280_intf.cpp
#define H6280_READ		1
#define H6280_WRITE		2
#define H6280_FETCH		4

#define H6280_ROM			(H6280_READ | H6280_FETCH)
#define H6280_RAM			(H6280_READ | H6280_FETCH | H6280_WRITE)

void h6280MapMemory(unsigned char *src, unsigned int start, unsigned int finish, int type);

void h6280SetIrqCallbackHandler(int (*callback)(int));
void h6280SetlinesCallback(void  (*setlines_callback)(int lines));

void h6280SetWritePortHandler(void (*write)(unsigned char, unsigned char));
void h6280SetWriteHandler(void (*write)(unsigned int, unsigned char));
void h6280SetReadHandler(unsigned char (*read)(unsigned int));

void h6280_write_port(unsigned char port, unsigned char data);
void h6280_write(unsigned int address, unsigned char data);
unsigned char h6280_read(unsigned int address);
unsigned char h6280_fetch(unsigned int address);

void h6280Init(int );
void h6280Open(int );
void h6280Reset();
int h6280Run(int cycles);
void h6280Close();
void h6280Exit();

extern int nh6280CpuCount;

#define H6280_IRQSTATUS_NONE	0
#define H6280_IRQSTATUS_ACK	1
#define H6280_IRQSTATUS_AUTO	2

#define H6280_INPUT_LINE_NMI	0x20

void h6280SetIRQLine(int line, int state);

//int h6280CpuScan(int nAction, int *);

int h6280TotalCycles();
void h6280NewFrame();
void h6280RunEnd();

int h6280GetActive();

void h6280_write_rom(unsigned int address, unsigned char data);


unsigned char h6280_irq_status_r(unsigned int offset);
void h6280_irq_status_w(unsigned int offset, unsigned char data);
unsigned char h6280_timer_r(unsigned int);
void h6280_timer_w(unsigned int offset, unsigned char data);
