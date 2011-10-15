enum {
	M6502_REG_PC, M6502_REG_A, M6502_REG_X, 
	M6502_REG_Y,  M6502_REG_P, M6502_REG_S
};


extern int nM6502Count;

void m6502Reset();

void m6502Init(int num);
void m6502Exit();

void m6502Open(int num);
void m6502Close();
int m6502GetActive();

int m6502Run(int timerTicks);


#define M6502_NMI	0xfffa
#define M6502_IRQ	0xfffe
#define M6502_CLEAR	-1

void m6502SetIRQ(int vector);

#define M6502_READ	1
#define M6502_WRITE	2
#define M6502_FETCH	4

#define M6502_RAM	(M6502_READ | M6502_WRITE | M6502_FETCH)
#define M6502_ROM	(M6502_READ | M6502_FETCH)


int m6502MapMemory(unsigned char* pMemory, unsigned short nStart, unsigned short nEnd, int nType);

void m6502SetReadHandler(unsigned char (*read)(unsigned short addr));
void m6502SetFetchHandler(unsigned char (*fetch)(unsigned short addr));
void m6502SetWriteHandler(void (*write)(unsigned short addr, unsigned char data));

int m6502_get_reg(int reg);
void m6502_set_reg(int reg, int data);

int m6502Scan(int nAction); 

unsigned char m6502_read_byte(unsigned short address);
unsigned char m6502_fetch_byte(unsigned short address);
void m6502_write_byte(unsigned short address, unsigned char data);

void m6502_write_rom(unsigned short address, unsigned char data);

int m6502TotalCycles();
void m6502NewFrame();

