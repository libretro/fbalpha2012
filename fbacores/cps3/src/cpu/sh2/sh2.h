
#ifndef FASTCALL
 #undef __fastcall
 #define __fastcall
#endif

typedef UINT8 (__fastcall *pSh2ReadByteHandler)(UINT32 a);
typedef void (__fastcall *pSh2WriteByteHandler)(UINT32 a, UINT8 d);
typedef UINT16 (__fastcall *pSh2ReadWordHandler)(UINT32 a);
typedef void (__fastcall *pSh2WriteWordHandler)(UINT32 a, UINT16 d);
typedef UINT32 (__fastcall *pSh2ReadLongHandler)(UINT32 a);
typedef void (__fastcall *pSh2WriteLongHandler)(UINT32 a, UINT32 d);

void __fastcall Sh2WriteByte(unsigned int a, unsigned char d);
unsigned char __fastcall Sh2ReadByte(unsigned int a);

int Sh2Init(int nCount);
int Sh2Exit();

void Sh2Open(const int i);

void Sh2Reset();
void Sh2Reset(unsigned int pc, unsigned r15); // hack
void Sh2Run(int cycles);

int Sh2MapMemory(unsigned char* pMemory, unsigned int nStart, unsigned int nEnd, int nType);
int Sh2MapHandler(uintptr_t nHandler, unsigned int nStart, unsigned int nEnd, int nType);

int Sh2SetReadByteHandler(int i, pSh2ReadByteHandler pHandler);
int Sh2SetWriteByteHandler(int i, pSh2WriteByteHandler pHandler);
int Sh2SetReadWordHandler(int i, pSh2ReadWordHandler pHandler);
int Sh2SetWriteWordHandler(int i, pSh2WriteWordHandler pHandler);
int Sh2SetReadLongHandler(int i, pSh2ReadLongHandler pHandler);
int Sh2SetWriteLongHandler(int i, pSh2WriteLongHandler pHandler);

#define SH2_IRQSTATUS_NONE	(0x00)
#define SH2_IRQSTATUS_AUTO	(0x01)

void Sh2SetIRQLine(const int line, const int state);

void Sh2StopRun();
void Sh2SetVBR(unsigned int i);

int Sh2TotalCycles();

int Sh2Scan(int);

