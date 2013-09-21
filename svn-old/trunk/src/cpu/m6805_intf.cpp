#include "burnint.h"
#include "m6805_intf.h"

static INT32 ADDRESS_MAX;
static INT32 ADDRESS_MASK;
static INT32 MPAGE;
static INT32 MPAGE_MASK;
static INT32 MPAGE_SHIFT;

#define READ		0
#define WRITE		1
#define FETCH		2

static UINT8 (*m6805Read)(UINT16 address);
static void (*m6805Write)(UINT16 address, UINT8 data);

static UINT8 *mem[3][0x100];

void m6805MapMemory(UINT8 *ptr, INT32 nStart, INT32 nEnd, INT32 nType)
{
#if defined FBA_DEBUG
	if (!DebugCPU_M6805Initted) bprintf(PRINT_ERROR, _T("m6805MapMemory called without init\n"));
#endif

	for (INT32 i = nStart / MPAGE; i < (nEnd / MPAGE) + 1; i++)
	{
		if (nType & (1 <<  READ)) mem[ READ][i] = ptr + ((i * MPAGE) - nStart);
		if (nType & (1 << WRITE)) mem[WRITE][i] = ptr + ((i * MPAGE) - nStart);
		if (nType & (1 << FETCH)) mem[FETCH][i] = ptr + ((i * MPAGE) - nStart);
	}
}

void m6805SetWriteHandler(void (*write)(UINT16, UINT8))
{
#if defined FBA_DEBUG
	if (!DebugCPU_M6805Initted) bprintf(PRINT_ERROR, _T("m6805SetWriteHandler called without init\n"));
#endif

	m6805Write = write;
}

void m6805SetReadHandler(UINT8 (*read)(UINT16))
{
#if defined FBA_DEBUG
	if (!DebugCPU_M6805Initted) bprintf(PRINT_ERROR, _T("m6805SetReadHandler called without init\n"));
#endif

	m6805Read = read;
}

void m6805_write(UINT16 address, UINT8 data)
{
	address &= ADDRESS_MASK;

	if (mem[WRITE][address >> MPAGE_SHIFT] != NULL) {
		mem[WRITE][address >> MPAGE_SHIFT][address & MPAGE_MASK] = data;
		return;
	}

	if (m6805Write != NULL) {
		m6805Write(address, data);
		return;
	}

	return;
}

UINT8 m6805_read(UINT16 address)
{
	address &= ADDRESS_MASK;

	if (mem[READ][address >> MPAGE_SHIFT] != NULL) {
		return mem[READ][address >> MPAGE_SHIFT][address & MPAGE_MASK];
	}

	if (m6805Read != NULL) {
		return m6805Read(address);
	}

	return 0;
}

UINT8 m6805_fetch(UINT16 address)
{
	address &= ADDRESS_MASK;

	if (mem[FETCH][address >> MPAGE_SHIFT] != NULL) {
		return mem[FETCH][address >> MPAGE_SHIFT][address & MPAGE_MASK];
	}

	return m6805_read(address);
}

void m6805_write_rom(UINT32 address, UINT8 data)
{
#if defined FBA_DEBUG
	if (!DebugCPU_M6805Initted) bprintf(PRINT_ERROR, _T("m6805_write_rom called without init\n"));
#endif

	address &= ADDRESS_MASK;

	if (mem[READ][address >> MPAGE_SHIFT] != NULL) {
		mem[READ][address >> MPAGE_SHIFT][address & MPAGE_MASK] = data;
	}

	if (mem[WRITE][address >> MPAGE_SHIFT] != NULL) {
		mem[WRITE][address >> MPAGE_SHIFT][address & MPAGE_MASK] = data;
	}

	if (mem[FETCH][address >> MPAGE_SHIFT] != NULL) {
		mem[FETCH][address >> MPAGE_SHIFT][address & MPAGE_MASK] = data;
	}

	if (m6805Write != NULL) {
		m6805Write(address, data);
		return;
	}

	return;
}

INT32 m6805GetActive()
{
	return 0;
}

static UINT8 m6805CheatRead(UINT32 a)
{
	return m6805_read(a);
}

static cpu_core_config M6805CheatCpuConfig =
{
	m6805Open,
	m6805Close,
	m6805CheatRead,
	m6805_write_rom,
	m6805GetActive,
	m6805TotalCycles,
	m6805NewFrame,
	m6805Run,
	m6805RunEnd,
	m6805Reset,	// different for differen types...
	1<<16,
	0
};

void m6805Init(INT32 num, INT32 max)
{
	DebugCPU_M6805Initted = 1;
	
	ADDRESS_MAX  = max;
	ADDRESS_MASK = ADDRESS_MAX - 1;
	MPAGE	     = ADDRESS_MAX / 0x100;
	MPAGE_MASK    = MPAGE - 1;
	MPAGE_SHIFT   = 0;
	for (MPAGE_SHIFT = 0; (1 << MPAGE_SHIFT) < MPAGE; MPAGE_SHIFT++) {}

	memset (mem[0], 0, MPAGE * sizeof(UINT8 *));
	memset (mem[1], 0, MPAGE * sizeof(UINT8 *));
	memset (mem[2], 0, MPAGE * sizeof(UINT8 *));

	for (INT32 i = 0; i < num; i++)
		CpuCheatRegister(i, &M6805CheatCpuConfig);
}

void m6805Exit()
{
#if defined FBA_DEBUG
	if (!DebugCPU_M6805Initted) bprintf(PRINT_ERROR, _T("m6805Exit called without init\n"));
#endif

	ADDRESS_MAX	= 0;
	ADDRESS_MASK	= 0;
	MPAGE		= 0;
	MPAGE_MASK	= 0;
	MPAGE_SHIFT	= 0;
	
	DebugCPU_M6805Initted = 0;
}

void m6805Open(INT32)
{
#if defined FBA_DEBUG
	if (!DebugCPU_M6805Initted) bprintf(PRINT_ERROR, _T("m6805Open called without init\n"));
#endif
}

void m6805Close()
{
#if defined FBA_DEBUG
	if (!DebugCPU_M6805Initted) bprintf(PRINT_ERROR, _T("m6805Close called without init\n"));
#endif
}
