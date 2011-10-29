
// konami.cpp
void konami_set_irq_line(int irqline, int state);
void konami_init(int (*irqcallback)(int));

// konami_intf.cpp
#define KON_READ		1
#define KON_WRITE		2
#define KON_FETCH		4

#define KON_ROM			(KON_READ | KON_FETCH)
#define KON_RAM			(KON_READ | KON_FETCH | KON_WRITE)

void konamiMapMemory(unsigned char *src, unsigned short start, unsigned short finish, int type);

void konamiSetIrqCallbackHandler(int (*callback)(int));
void konamiSetlinesCallback(void  (*setlines_callback)(int lines));

void konamiSetWriteHandler(void (*write)(unsigned short, unsigned char));
void konamiSetReadHandler(unsigned char (*read)(unsigned short));

void konami_write(unsigned short address, unsigned char data);
unsigned char konami_read(unsigned short address);
unsigned char konami_fetch(unsigned short address);

void konamiInit(int );
void konamiOpen(int );
void konamiReset();
int konamiRun(int cycles);
void konamiClose();
void konamiExit();

extern int nKonamiCpuCount;

#define KONAMI_IRQ_LINE		0
#define KONAMI_FIRQ_LINE	1

#define KONAMI_CLEAR_LINE	0
#define KONAMI_HOLD_LINE	1
#define KONAMI_INPUT_LINE_NMI	2

void konamiSetIrqLine(int line, int state);

int konamiCpuScan(int nAction, int *);

int konamiTotalCycles();
void konamiNewFrame();

int konamiGetActive();

void konami_write_rom(unsigned short address, unsigned char data);
