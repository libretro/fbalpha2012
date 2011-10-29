
// use these in arm.cpp
void Arm7_program_write_byte_32le(unsigned int addr, unsigned char data);
void Arm7_program_write_word_32le(unsigned int addr, unsigned short data);
void Arm7_program_write_dword_32le(unsigned int addr, unsigned int data);
unsigned char  Arm7_program_read_byte_32le(unsigned int addr);
unsigned short Arm7_program_read_word_32le(unsigned int addr);
unsigned int   Arm7_program_read_dword_32le(unsigned int addr);
unsigned short Arm7_program_opcode_word_32le(unsigned int addr);
unsigned int   Arm7_program_opcode_dword_32le(unsigned int addr);

void arm7_set_irq_line(int irqline, int state);

// use these everywhere else
#define ARM7_READ		1
#define ARM7_WRITE		2
#define ARM7_FETCH		4

#define ARM7_ROM		(ARM7_READ | ARM7_FETCH)
#define ARM7_RAM		(ARM7_READ | ARM7_FETCH | ARM7_WRITE)

void Arm7MapMemory(unsigned char *src, int start, int finish, int type);

void Arm7SetWriteByteHandler(void (*write)(unsigned int, unsigned char));
void Arm7SetWriteWordHandler(void (*write)(unsigned int, unsigned short));
void Arm7SetWriteLongHandler(void (*write)(unsigned int, unsigned int));
void Arm7SetReadByteHandler(unsigned char (*read)(unsigned int));
void Arm7SetReadWordHandler(unsigned short (*read)(unsigned int));
void Arm7SetReadLongHandler(unsigned int (*read)(unsigned int));

int Arm7Scan(int nAction);

void Arm7RunEnd();
void Arm7BurnCycles(int cycles);
int Arm7TotalCycles();
void Arm7NewFrame();

void Arm7Init(int);
void Arm7Open(int );
void Arm7Reset();
int Arm7Run(int cycles);

// defined in arm7core.h
#define ARM7_IRQ_LINE		0
#define ARM7_FIRQ_LINE		1

#define ARM7_CLEAR_LINE		0
#define ARM7_ASSERT_LINE	1
#define ARM7_HOLD_LINE		2

void Arm7SetIRQLine(int line, int state);

void Arm7Exit();
void Arm7Close();

// speed hack stuff
void Arm7SetIdleLoopAddress(unsigned int address);

// for cheat handling
void Arm7_write_rom_byte(unsigned int addr, unsigned char data);
