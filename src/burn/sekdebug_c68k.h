typedef enum SekRegister {
	SEK_REG_D0, SEK_REG_D1, SEK_REG_D2, SEK_REG_D3, SEK_REG_D4, SEK_REG_D5, SEK_REG_D6, SEK_REG_D7,
	SEK_REG_A0, SEK_REG_A1, SEK_REG_A2, SEK_REG_A3, SEK_REG_A4, SEK_REG_A5, SEK_REG_A6, SEK_REG_A7,
	SEK_REG_PC,
	SEK_REG_SR,
	SEK_REG_SP, SEK_REG_USP, SEK_REG_ISP, SEK_REG_MSP,
	SEK_REG_VBR,
	SEK_REG_SFC, SEK_REG_DFC,
	SEK_REG_CACR, SEK_REG_CAAR
};

extern void (*SekDbgBreakpointHandlerRead)(unsigned int, int);
extern void (*SekDbgBreakpointHandlerFetch)(unsigned int, int);
extern void (*SekDbgBreakpointHandlerWrite)(unsigned int, int);

void SekDbgDisableBreakpoints();
void SekDbgEnableBreakpoints();
void SekDbgEnableSingleStep();

int SekDbgSetBreakpointDataRead(unsigned int nAddress, int nIdentifier);
int SekDbgSetBreakpointDataWrite(unsigned int nAddress, int nIdentifier);
int SekDbgSetBreakpointFetch(unsigned int nAddress, int nIdentifier);

int SekDbgGetCPUType();
int SekDbgGetPendingIRQ();
unsigned int SekDbgGetRegister(SekRegister nRegister);
bool SekDbgSetRegister(SekRegister nRegister, unsigned int nValue);
