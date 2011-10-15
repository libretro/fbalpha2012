
// use these in arm.cpp
void Arm_program_write_byte_32le(unsigned int addr, unsigned char data);
void Arm_program_write_dword_32le(unsigned int addr, unsigned int data);
unsigned char  Arm_program_read_byte_32le(unsigned int addr);
unsigned int   Arm_program_read_dword_32le(unsigned int addr);
unsigned int   Arm_program_opcode_dword_32le(unsigned int addr);

void arm_set_irq_line(int irqline, int state);

// use these everywhere else
#define ARM_READ		1
#define ARM_WRITE		2
#define ARM_FETCH		4

#define ARM_ROM		(ARM_READ | ARM_FETCH)
#define ARM_RAM		(ARM_READ | ARM_FETCH | ARM_WRITE)

void ArmMapMemory(unsigned char *src, int start, int finish, int type);

void ArmSetWriteByteHandler(void (*write)(unsigned int, unsigned char));
void ArmSetWriteLongHandler(void (*write)(unsigned int, unsigned int));
void ArmSetReadByteHandler(unsigned char (*read)(unsigned int));
void ArmSetReadLongHandler(unsigned int (*read)(unsigned int));

void ArmInit(int);
void ArmOpen(int );
void ArmReset();
int ArmRun(int cycles);
int ArmScan(int nAction, int *);

#define ARM_IRQ_LINE		0
#define ARM_FIRQ_LINE		1

#define ARM_CLEAR_LINE		0
#define ARM_ASSERT_LINE		1
#define ARM_HOLD_LINE		2

void ArmSetIRQLine(int line, int state);

void ArmExit();
void ArmClose();

extern unsigned int ArmSpeedHackAddress;
void ArmIdleCycles(int cycles);
void ArmSetSpeedHack(unsigned int address, void (*pCallback)());

unsigned int ArmGetPc();

unsigned int ArmRemainingCycles();
unsigned int ArmGetTotalCycles();
void ArmRunEnd();
void ArmNewFrame();

// for cheat handling
void Arm_write_rom_byte(unsigned int addr, unsigned char data);



//-----------------------------------------------------------------------------------------------------------------------------------------------
// Macros for reading/writing to 16-bit RAM regions

#define Write16Long(ram, a, b)							\
	if (address >= a && address <= b) {					\
		*((unsigned short*)(ram + (((address - a) & ~3)/2))) = data;	\
		return;								\
	}

#define Write16Byte(ram, a, b)							\
	if (address >= a && address <= b) {					\
		if (~address & 2) {						\
			int offset = address - a;				\
			ram[(offset & 1) | ((offset & ~3) / 2)] = data;		\
		}								\
		return;								\
	}

#define Read16Long(ram, a, b)							\
	if (address >= a && address <= b) {					\
		return *((unsigned short*)(ram + (((address - a) & ~3)/2))) | ~0xffff;	\
	}

#define Read16Byte(ram, a, b)							\
	if (address >= a && address <= b) {					\
		if (~address & 2) {						\
			int offset = address - a;				\
			return ram[(offset & 1) | ((offset & ~3) / 2)];		\
		} else {							\
			return 0xff;						\
		}								\
	}
