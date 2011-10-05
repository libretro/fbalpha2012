// Cheat module

#include "burnint.h"
#include "vez.h"
#include "sh2.h"
#include "m6502.h"
#include "m6809_intf.h"
#include "hd6309_intf.h"
#include "m6800_intf.h"
#include "s2650_intf.h"
#include "konami_intf.h"
#include "arm7_intf.h"

bool bCheatsAllowed;
CheatInfo* pCheatInfo = NULL;

static bool bCheatsEnabled = false;

//----------------------------------------------------
// Cpu interface for cheat application

#define MAX_CHEAT_CPU	8	// enough?

static int nActiveCheatCpus;

struct cheat_subs {
	int nCpu;	// Which cpu is this? (SekOpen(#), ZetOpen(#))
	void (*cpu_open)(int);
	void (*write)(unsigned int, unsigned char);
	unsigned char (*read)(unsigned int);
	void (*cpu_close)();
	int (*active_cpu)();
	unsigned int nMemorySize;
};

struct cheat_subs cheat_sub_block[MAX_CHEAT_CPU];
struct cheat_subs *cheat_subptr;

//---------------------------------------------------
// Dummy handlers

static int  CheatDummyGetActive() { return -1; }
static void CheatDummyOpen(int) {}
static void CheatDummyClose() {}
static void CheatDummyWriteByte(unsigned int, unsigned char) {}
static unsigned char CheatDummyReadByte(unsigned int) { return 0; }

// -----------------------------------------------
// Set up handlers for cpu cores that aren't totally compatible

#define CHEAT_READ(name,funct) \
static unsigned char name##ReadByteCheat(unsigned int a) {	\
	return funct;					\
}

#define CHEAT_WRITE(name,funct)	\
static void name##WriteByteCheat(unsigned int a, unsigned char d) {	\
	funct;	\
}

CHEAT_READ(Sek,     SekReadByte(a))
CHEAT_READ(Sh2,     Sh2ReadByte(a))
CHEAT_WRITE(Sh2,    Sh2WriteByte(a,d))
CHEAT_READ(Zet,     ZetReadByte(a))
CHEAT_WRITE(Zet,    ZetWriteRom(a,d))
CHEAT_READ(M6800,   M6800ReadByte(a))
CHEAT_WRITE(M6800,  M6800WriteRom(a,d))
CHEAT_READ(M6809,   M6809ReadByte(a))
CHEAT_WRITE(M6809,  M6809WriteRom(a,d))
CHEAT_READ(HD6309,  HD6309ReadByte(a))
CHEAT_WRITE(HD6309, HD6309WriteRom(a,d))
CHEAT_READ(m6502,   m6502_read_byte(a))
CHEAT_WRITE(m6502,  m6502_write_rom(a,d))
CHEAT_READ(s2650,   s2650_read(a))
CHEAT_WRITE(s2650,  s2650_write_rom(a,d))
CHEAT_READ(konami,  konami_read(a))
CHEAT_WRITE(konami, konami_write_rom(a,d))

//------------------------------------------------
// Central cpu registry for functions necessary for cheats

void CpuCheatRegister(int type, int nNum)
{
	cheat_subptr = &cheat_sub_block[nActiveCheatCpus];
	nActiveCheatCpus++;

	switch (type)
	{
		case 0x0000: // m68k
		{
			cheat_subptr->nCpu = nNum;
			cheat_subptr->cpu_open = SekOpen;
			cheat_subptr->cpu_close = SekClose;
			cheat_subptr->active_cpu = SekGetActive;
			cheat_subptr->write = SekWriteByteROM;
			cheat_subptr->read = SekReadByteCheat;
			cheat_subptr->nMemorySize = 0x01000000;
		}
		break;

		case 0x0001: // NEC V30 / V33
		{
			cheat_subptr->nCpu = nNum;
			cheat_subptr->cpu_open = VezOpen;
			cheat_subptr->cpu_close = VezClose;
			cheat_subptr->active_cpu = VezGetActive;
			cheat_subptr->write = cpu_writemem20;
			cheat_subptr->read = cpu_readmem20;
			cheat_subptr->nMemorySize = 0x00100000;
		}
		break;

		case 0x0002: // SH2
		{
			cheat_subptr->nCpu = nNum;
			cheat_subptr->cpu_open = Sh2Open;
			cheat_subptr->cpu_close = Sh2Close;
			cheat_subptr->active_cpu = Sh2GetActive;
			cheat_subptr->write = Sh2WriteByteCheat;
			cheat_subptr->read = Sh2ReadByteCheat;
			cheat_subptr->nMemorySize = 0x02080000; // Good enough for CPS3
		}
		break;

		case 0x0003: // M6502
		{
			cheat_subptr->nCpu = nNum;
			cheat_subptr->cpu_open = m6502Open;
			cheat_subptr->cpu_close = m6502Close;
			cheat_subptr->active_cpu = m6502GetActive;
			cheat_subptr->write = m6502WriteByteCheat;
			cheat_subptr->read = m6502ReadByteCheat;
			cheat_subptr->nMemorySize = 0x00010000;
		}
		break;

		case 0x0004: // Z80
		{
			cheat_subptr->nCpu = nNum;
			cheat_subptr->cpu_open = ZetOpen;
			cheat_subptr->cpu_close = ZetClose;
			cheat_subptr->active_cpu = ZetGetActive;
			cheat_subptr->write = ZetWriteByteCheat;
			cheat_subptr->read = ZetReadByteCheat;
			cheat_subptr->nMemorySize = 0x00010000;
		}
		break;

		case 0x0005: // M6809
		{
			cheat_subptr->nCpu = nNum;
			cheat_subptr->cpu_open = M6809Open;
			cheat_subptr->cpu_close = M6809Close;
			cheat_subptr->active_cpu = M6809GetActive;
			cheat_subptr->write = M6809WriteByteCheat;
			cheat_subptr->read = M6809ReadByteCheat;
			cheat_subptr->nMemorySize = 0x00010000;
		}
		break;

		case 0x0006: // HD6309
		{
			cheat_subptr->nCpu = nNum;
			cheat_subptr->cpu_open = HD6309Open;
			cheat_subptr->cpu_close = HD6309Close;
			cheat_subptr->active_cpu = HD6309GetActive;
			cheat_subptr->write = HD6309WriteByteCheat;
			cheat_subptr->read = HD6309ReadByteCheat;
			cheat_subptr->nMemorySize = 0x00010000;
		}
		break;

		case 0x0007: // M6800
		{
			cheat_subptr->nCpu = nNum;
			cheat_subptr->cpu_open = CheatDummyOpen;
			cheat_subptr->cpu_close = CheatDummyClose;
			cheat_subptr->active_cpu = CheatDummyGetActive;
			cheat_subptr->write = M6800WriteByteCheat;
			cheat_subptr->read = M6800ReadByteCheat;
			cheat_subptr->nMemorySize = 0x00010000;
		}
		break;

		case 0x0008: // S2650
		{
			cheat_subptr->nCpu = nNum;
			cheat_subptr->cpu_open = s2650Open;
			cheat_subptr->cpu_close = s2650Close;
			cheat_subptr->active_cpu = s2650GetActive;
			cheat_subptr->write = s2650WriteByteCheat;
			cheat_subptr->read = s2650ReadByteCheat;
			cheat_subptr->nMemorySize = 0x00010000;
		}
		break;

		case 0x0009: // Konami Custom
		{
			cheat_subptr->nCpu = nNum;
			cheat_subptr->cpu_open = konamiOpen;
			cheat_subptr->cpu_close = konamiClose;
			cheat_subptr->active_cpu = konamiGetActive;
			cheat_subptr->write = konamiWriteByteCheat;
			cheat_subptr->read = konamiReadByteCheat;
			cheat_subptr->nMemorySize = 0x00010000;
		}
		break;

		case 0x000a: // ARM7
		{
			cheat_subptr->nCpu = nNum;
			cheat_subptr->cpu_open = Arm7Open;
			cheat_subptr->cpu_close = Arm7Close;
			cheat_subptr->active_cpu = CheatDummyGetActive;
			cheat_subptr->write = Arm7_write_rom_byte;
			cheat_subptr->read = Arm7_program_read_byte_32le;
			cheat_subptr->nMemorySize = 0x80000000; // enough for PGM...
		}
		break;

 		// Just in case the called cpu isn't supported and so MinGW
		// doesn't complain about unused functions...
		default:
		{
			cheat_subptr->nCpu = 0;
			cheat_subptr->cpu_open = CheatDummyOpen;
			cheat_subptr->cpu_close = CheatDummyClose;
			cheat_subptr->active_cpu = CheatDummyGetActive;
			cheat_subptr->write = CheatDummyWriteByte;
			cheat_subptr->read = CheatDummyReadByte;
			cheat_subptr->nMemorySize = 0;
		}
		break;
	}
}

int CheatUpdate()
{
	bCheatsEnabled = false;

	if (bCheatsAllowed) {
		CheatInfo* pCurrentCheat = pCheatInfo;
		CheatAddressInfo* pAddressInfo;

		while (pCurrentCheat) {
			if (pCurrentCheat->nStatus > 1) {
				pAddressInfo = pCurrentCheat->pOption[pCurrentCheat->nCurrent]->AddressInfo;
				if (pAddressInfo->nAddress) {
					bCheatsEnabled = true;
				}
			}
			pCurrentCheat = pCurrentCheat->pNext;
		}
	}

	return 0;
}

int CheatEnable(int nCheat, int nOption)
{
	int nCurrentCheat = 0;
	CheatInfo* pCurrentCheat = pCheatInfo;
	CheatAddressInfo* pAddressInfo;
	int nOpenCPU = -1;

	if (!bCheatsAllowed) {
		return 1;
	}

	if (nOption >= CHEAT_MAX_OPTIONS) {
		return 1;
	}

	cheat_subptr = &cheat_sub_block[0]; // first cpu...

	while (pCurrentCheat && nCurrentCheat <= nCheat) {
		if (nCurrentCheat == nCheat) {

			if (nOption == -1) {
				nOption = pCurrentCheat->nDefault;
			}

			if (pCurrentCheat->nType != 1) {

				// Return OK if the cheat is already active with the same option
				if (pCurrentCheat->nCurrent == nOption) {
					return 0;
				}

				// Deactivate old option (if any)
				pAddressInfo = pCurrentCheat->pOption[nOption]->AddressInfo;
				while (pAddressInfo->nAddress) {

					if (pAddressInfo->nCPU != nOpenCPU) {

						if (nOpenCPU != -1) {
							cheat_subptr->cpu_close();
						}

						nOpenCPU = pAddressInfo->nCPU;
						cheat_subptr = &cheat_sub_block[nOpenCPU];
						cheat_subptr->cpu_open(cheat_subptr->nCpu);
					}

					// Write back original values to memory
					cheat_subptr->write(pAddressInfo->nAddress, pAddressInfo->nOriginalValue);
					pAddressInfo++;
				}
			}

			// Activate new option
			pAddressInfo = pCurrentCheat->pOption[nOption]->AddressInfo;
			while (pAddressInfo->nAddress) {

				if (pAddressInfo->nCPU != nOpenCPU) {
					if (nOpenCPU != -1) {
						cheat_subptr->cpu_close();
					}

					nOpenCPU = pAddressInfo->nCPU;
					cheat_subptr = &cheat_sub_block[nOpenCPU];
					cheat_subptr->cpu_open(cheat_subptr->nCpu);
				}

				// Copy the original values
				pAddressInfo->nOriginalValue = cheat_subptr->read(pAddressInfo->nAddress);

				if (pCurrentCheat->nType != 0) {
					if (pAddressInfo->nCPU != nOpenCPU) {
						if (nOpenCPU != -1) {
							cheat_subptr->cpu_close();
						}

						nOpenCPU = pAddressInfo->nCPU;
						cheat_subptr = &cheat_sub_block[nOpenCPU];
						cheat_subptr->cpu_open(cheat_subptr->nCpu);
					}

					// Activate the cheat
					cheat_subptr->write(pAddressInfo->nAddress, pAddressInfo->nValue);
				}

				pAddressInfo++;
			}

			// Set cheat status and active option
			if (pCurrentCheat->nType != 1) {
				pCurrentCheat->nCurrent = nOption;
			}
			if (pCurrentCheat->nType == 0) {
				pCurrentCheat->nStatus = 2;
			}
			if (pCurrentCheat->nType == 2) {
				pCurrentCheat->nStatus = 1;
			}

			break;
		}
		pCurrentCheat = pCurrentCheat->pNext;
		nCurrentCheat++;
	}

	if (nOpenCPU != -1) {
		cheat_subptr->cpu_close();
	}

	CheatUpdate();

	if (nCurrentCheat == nCheat && pCurrentCheat) {
		return 0;
	}

	return 1;
}

int CheatApply()
{
	if (!bCheatsEnabled) {
		return 0;
	}

	int nOpenCPU = -1;

	CheatInfo* pCurrentCheat = pCheatInfo;
	CheatAddressInfo* pAddressInfo;
	while (pCurrentCheat) {
		if (pCurrentCheat->nStatus > 1) {
			pAddressInfo = pCurrentCheat->pOption[pCurrentCheat->nCurrent]->AddressInfo;
			while (pAddressInfo->nAddress) {

				if (pAddressInfo->nCPU != nOpenCPU) {
					if (nOpenCPU != -1) {
						cheat_subptr->cpu_close();
					}

					nOpenCPU = pAddressInfo->nCPU;
					cheat_subptr = &cheat_sub_block[nOpenCPU];
					cheat_subptr->cpu_open(cheat_subptr->nCpu);
				}

				cheat_subptr->write(pAddressInfo->nAddress, pAddressInfo->nValue);
				pAddressInfo++;
			}
		}
		pCurrentCheat = pCurrentCheat->pNext;
	}

	if (nOpenCPU != -1) {
		cheat_subptr->cpu_close();
	}

	return 0;
}

int CheatInit()
{
	CheatExit();

	bCheatsEnabled = false;

	return 0;
}

void CheatExit()
{
	if (pCheatInfo) {
		CheatInfo* pCurrentCheat = pCheatInfo;
		CheatInfo* pNextCheat;

		do {
			pNextCheat = pCurrentCheat->pNext;
			for (int i = 0; i < CHEAT_MAX_OPTIONS; i++) {
				free(pCurrentCheat->pOption[i]);
			}
			free(pCurrentCheat);
		} while ((pCurrentCheat = pNextCheat) != 0);
	}

	nActiveCheatCpus = 0;
	for (int i = 0; i < MAX_CHEAT_CPU; i++) {
		CpuCheatRegister(-1, i); // set them all to dummy...
	}
	nActiveCheatCpus = 0;

	pCheatInfo = NULL;
}

// Cheat search

static unsigned char *MemoryValues = NULL;
static unsigned char *MemoryStatus = NULL;
static UINT32 nMemorySize = 0;

#define NOT_IN_RESULTS	0
#define IN_RESULTS	1

unsigned int CheatSearchShowResultAddresses[CHEATSEARCH_SHOWRESULTS];
unsigned int CheatSearchShowResultValues[CHEATSEARCH_SHOWRESULTS];

int CheatSearchInit()
{
	return 1;
}

void CheatSearchExit()
{
	free(MemoryValues);
	MemoryValues = NULL;
	free(MemoryStatus);
	MemoryStatus = NULL;
	
	nMemorySize = 0;
	
	memset(CheatSearchShowResultAddresses, 0, CHEATSEARCH_SHOWRESULTS);
	memset(CheatSearchShowResultValues, 0, CHEATSEARCH_SHOWRESULTS);
}

void CheatSearchStart()
{
	UINT32 nAddress;
	
	int nActiveCPU = 0;
	cheat_subptr = &cheat_sub_block[nActiveCPU]; // first cpu only (ok?)
	
	nActiveCPU = cheat_subptr->active_cpu();
	if (nActiveCPU >= 0) cheat_subptr->cpu_close();
	cheat_subptr->cpu_open(cheat_subptr->nCpu);
	nMemorySize = cheat_subptr->nMemorySize;

	MemoryValues = (unsigned char*)malloc(nMemorySize);
	MemoryStatus = (unsigned char*)malloc(nMemorySize);

	for (nAddress = 0; nAddress < nMemorySize; nAddress++) {
		MemoryValues[nAddress] = cheat_subptr->read(nAddress);
	}
	
	cheat_subptr->cpu_close();
	if (nActiveCPU >= 0) cheat_subptr->cpu_open(nActiveCPU);
			
	memset(MemoryStatus, IN_RESULTS, nMemorySize);
}

static void CheatSearchGetResults()
{
	UINT32 nAddress;
	unsigned int nResultsPos = 0;
	
	memset(CheatSearchShowResultAddresses, 0, CHEATSEARCH_SHOWRESULTS);
	memset(CheatSearchShowResultValues, 0, CHEATSEARCH_SHOWRESULTS);
	
	for (nAddress = 0; nAddress < nMemorySize; nAddress++) {		
		if (MemoryStatus[nAddress] == IN_RESULTS) {
			CheatSearchShowResultAddresses[nResultsPos] = nAddress;
			CheatSearchShowResultValues[nResultsPos] = MemoryValues[nAddress];
			nResultsPos++;
		}
	}
}

unsigned int CheatSearchValueNoChange()
{
	unsigned int nMatchedAddresses = 0;
	UINT32 nAddress;
	
	int nActiveCPU = 0;
	
	nActiveCPU = cheat_subptr->active_cpu();
	if (nActiveCPU >= 0) cheat_subptr->cpu_close();
	cheat_subptr->cpu_open(0);
	
	for (nAddress = 0; nAddress < nMemorySize; nAddress++) {
		if (MemoryStatus[nAddress] == NOT_IN_RESULTS) continue;
		if (cheat_subptr->read(nAddress) == MemoryValues[nAddress]) {
			MemoryValues[nAddress] = cheat_subptr->read(nAddress);
			nMatchedAddresses++;
		} else {
			MemoryStatus[nAddress] = NOT_IN_RESULTS;
		}
	}

	cheat_subptr->cpu_close();
	if (nActiveCPU >= 0) cheat_subptr->cpu_open(nActiveCPU);
	
	if (nMatchedAddresses <= CHEATSEARCH_SHOWRESULTS) CheatSearchGetResults();
	
	return nMatchedAddresses;
}

unsigned int CheatSearchValueChange()
{
	unsigned int nMatchedAddresses = 0;
	UINT32 nAddress;
	
	int nActiveCPU = 0;
	
	nActiveCPU = cheat_subptr->active_cpu();
	if (nActiveCPU >= 0) cheat_subptr->cpu_close();
	cheat_subptr->cpu_open(0);
	
	for (nAddress = 0; nAddress < nMemorySize; nAddress++) {
		if (MemoryStatus[nAddress] == NOT_IN_RESULTS) continue;
		if (cheat_subptr->read(nAddress) != MemoryValues[nAddress]) {
			MemoryValues[nAddress] = cheat_subptr->read(nAddress);
			nMatchedAddresses++;
		} else {
			MemoryStatus[nAddress] = NOT_IN_RESULTS;
		}
	}
	
	cheat_subptr->cpu_close();
	if (nActiveCPU >= 0) cheat_subptr->cpu_open(nActiveCPU);
	
	if (nMatchedAddresses <= CHEATSEARCH_SHOWRESULTS) CheatSearchGetResults();
	
	return nMatchedAddresses;
}

unsigned int CheatSearchValueDecreased()
{
	unsigned int nMatchedAddresses = 0;
	UINT32 nAddress;
	
	int nActiveCPU = 0;
	
	nActiveCPU = cheat_subptr->active_cpu();
	if (nActiveCPU >= 0) cheat_subptr->cpu_close();
	cheat_subptr->cpu_open(0);

	for (nAddress = 0; nAddress < nMemorySize; nAddress++) {
		if (MemoryStatus[nAddress] == NOT_IN_RESULTS) continue;
		if (cheat_subptr->read(nAddress) < MemoryValues[nAddress]) {
			MemoryValues[nAddress] = cheat_subptr->read(nAddress);
			nMatchedAddresses++;
		} else {
			MemoryStatus[nAddress] = NOT_IN_RESULTS;
		}
	}

	cheat_subptr->cpu_close();
	if (nActiveCPU >= 0) cheat_subptr->cpu_open(nActiveCPU);
	
	if (nMatchedAddresses <= CHEATSEARCH_SHOWRESULTS) CheatSearchGetResults();
	
	return nMatchedAddresses;
}

unsigned int CheatSearchValueIncreased()
{
	unsigned int nMatchedAddresses = 0;
	UINT32 nAddress;
	
	int nActiveCPU = 0;
	
	nActiveCPU = cheat_subptr->active_cpu();
	if (nActiveCPU >= 0) cheat_subptr->cpu_close();
	cheat_subptr->cpu_open(0);

	for (nAddress = 0; nAddress < nMemorySize; nAddress++) {
		if (MemoryStatus[nAddress] == NOT_IN_RESULTS) continue;
		if (cheat_subptr->read(nAddress) > MemoryValues[nAddress]) {
			MemoryValues[nAddress] = cheat_subptr->read(nAddress);
			nMatchedAddresses++;
		} else {
			MemoryStatus[nAddress] = NOT_IN_RESULTS;
		}
	}
	
	cheat_subptr->cpu_close();
	if (nActiveCPU >= 0) cheat_subptr->cpu_open(nActiveCPU);
	
	if (nMatchedAddresses <= CHEATSEARCH_SHOWRESULTS) CheatSearchGetResults();
	
	return nMatchedAddresses;
}

void CheatSearchDumptoFile()
{
	FILE *fp = fopen("cheatsearchdump.txt", "wt");
	UINT32 nAddress;
	
	if (fp) {
		char Temp[256];
		
		for (nAddress = 0; nAddress < nMemorySize; nAddress++) {
			if (MemoryStatus[nAddress] == IN_RESULTS) {
				sprintf(Temp, "Address %08X Value %02X\n", nAddress, MemoryValues[nAddress]);
				fwrite(Temp, 1, strlen(Temp), fp);
			}
		}
		
		fclose(fp);
	}
}

#undef NOT_IN_RESULTS
#undef IN_RESULTS
