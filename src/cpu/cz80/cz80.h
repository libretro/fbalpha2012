/******************************************************************************
 *
 * CZ80 (Z80 CPU emulator) version 0.9
 * Compiled with Dev-C++
 * Copyright 2004-2005 Stphane Dallongeville
 *
 * (Modified by NJ)
 *
 *****************************************************************************/

#ifndef CZ80_H
#define CZ80_H

#ifndef FASTCALL
 #undef __fastcall
 #define __fastcall
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*************************************/
/* Z80 core Structures & definitions */
/*************************************/

#define CZ80_FETCH_BITS         8   // [4-12]   default = 8
#define CZ80_FETCH_SFT          (16 - CZ80_FETCH_BITS)
#define CZ80_FETCH_BANK         (1 << CZ80_FETCH_BITS)

#define CZ80_LITTLE_ENDIAN		0
#define CZ80_USE_JUMPTABLE		0
#define CZ80_BIG_FLAGS_ARRAY	1
#define CZ80_ENCRYPTED_ROM		0
#define CZ80_EMULATE_R_EXACTLY	0

#define zR8(A)		(*CPU->pzR8[A])
#define zR16(A)		(CPU->pzR16[A]->W)

#define pzAF		&(CPU->AF)
#define zAF			CPU->AF.W
#define zlAF		CPU->AF.B.L
#define zhAF		CPU->AF.B.H
#define zA			zhAF
#define zF			zlAF

#define pzBC		&(CPU->BC)
#define zBC			CPU->BC.W
#define zlBC		CPU->BC.B.L
#define zhBC		CPU->BC.B.H
#define zB			zhBC
#define zC			zlBC

#define pzDE		&(CPU->DE)
#define zDE			CPU->DE.W
#define zlDE		CPU->DE.B.L
#define zhDE		CPU->DE.B.H
#define zD			zhDE
#define zE			zlDE

#define pzHL		&(CPU->HL)
#define zHL			CPU->HL.W
#define zlHL		CPU->HL.B.L
#define zhHL		CPU->HL.B.H
#define zH			zhHL
#define zL			zlHL

#define zAF2		CPU->AF2.W
#define zlAF2		CPU->AF2.B.L
#define zhAF2		CPU->AF2.B.H
#define zA2			zhAF2
#define zF2			zlAF2

#define zBC2		CPU->BC2.W
#define zDE2		CPU->DE2.W
#define zHL2		CPU->HL2.W

#define pzIX		&(CPU->IX)
#define zIX			CPU->IX.W
#define zlIX		CPU->IX.B.L
#define zhIX		CPU->IX.B.H

#define pzIY		&(CPU->IY)
#define zIY			CPU->IY.W
#define zlIY		CPU->IY.B.L
#define zhIY		CPU->IY.B.H

#define pzSP		&(CPU->SPTR)
#define zSP			CPU->SPTR.W
#define zlSP		CPU->SPTR.B.L
#define zhSP		CPU->SPTR.B.H

#define zRealPC		(PC - CPU->BasePC)
#define zPC			PC

#define zI			CPU->I
#define zIM			CPU->IM

#define zwR			CPU->R.W
#define zR1			CPU->R.B.L
#define zR2			CPU->R.B.H
#define zR			zR1

#define zIFF		CPU->IFF.W
#define zIFF1		CPU->IFF.B.L
#define zIFF2		CPU->IFF.B.H

#define CZ80_SF_SFT	 7
#define CZ80_ZF_SFT	 6
#define CZ80_YF_SFT	 5
#define CZ80_HF_SFT	 4
#define CZ80_XF_SFT	 3
#define CZ80_PF_SFT	 2
#define CZ80_VF_SFT	 2
#define CZ80_NF_SFT	 1
#define CZ80_CF_SFT	 0

#define CZ80_SF		(1 << CZ80_SF_SFT)
#define CZ80_ZF		(1 << CZ80_ZF_SFT)
#define CZ80_YF		(1 << CZ80_YF_SFT)
#define CZ80_HF		(1 << CZ80_HF_SFT)
#define CZ80_XF		(1 << CZ80_XF_SFT)
#define CZ80_PF		(1 << CZ80_PF_SFT)
#define CZ80_VF		(1 << CZ80_VF_SFT)
#define CZ80_NF		(1 << CZ80_NF_SFT)
#define CZ80_CF		(1 << CZ80_CF_SFT)

#define CZ80_IFF_SFT	CZ80_PF_SFT
#define CZ80_IFF		CZ80_PF

#define CZ80_IRQSTATUS_NONE	0x8000
#define CZ80_IRQSTATUS_AUTO	0x2000
#define CZ80_IRQSTATUS_ACK	0x1000

typedef union
{
	struct
	{
#if CZ80_LITTLE_ENDIAN
		unsigned char L;
		unsigned char H;
#else
		unsigned char H;
		unsigned char L;
#endif
	} B;
	unsigned short W;
} union16;

typedef struct cz80_t
{
	union
	{
		unsigned char r8[8];
		union16 r16[4];
/*#ifndef __cplusplus*/
		struct
		{
			union16 BC;
			union16 DE;
			union16 HL;
			union16 AF;
		};
/*#endif*/
	};

	union16 IX;
	union16 IY;
	union16 SPTR;

	unsigned char I;
	unsigned char IM;
	
	union16 BC2;
	union16 DE2;
	union16 HL2;
	union16 AF2;

	union16 R;
	union16 IFF;

	unsigned int PC;
	int BasePC;

	int nCyclesTotal;
	int nCyclesSegment;
	int nCyclesLeft;

	int nEI;
	int nInterruptLatch;

	unsigned char * Fetch[CZ80_FETCH_BANK];
	unsigned char * FetchData[CZ80_FETCH_BANK];
	unsigned char * Read[CZ80_FETCH_BANK];
	unsigned char * Write[CZ80_FETCH_BANK];
	
	unsigned char *pzR8[8];
	union16 *pzR16[4];

/*
	unsigned char (*InnerReadByte)(unsigned short address);
	unsigned short (*InnerReadWord)(unsigned short address);
	void (*InnerWriteByte)(unsigned short address, unsigned char data);
	void (*InnerWriteWord)(unsigned short address, unsigned short data);
*/

	unsigned char   (__fastcall *Read_Byte)(unsigned short address);
	void (__fastcall *Write_Byte)(unsigned short address, unsigned char data);
	unsigned char   (__fastcall *IN_Port)(unsigned short port);
	void (__fastcall *OUT_Port)(unsigned short port, unsigned char value);

//	int  (*Interrupt_Callback)(int irqline);

} cz80_struc;

/*************************/
/* Publics Z80 functions */
/*************************/
/*
void Cz80_InitFlags();

void Cz80_Init(cz80_struc*);

void Cz80_Reset(cz80_struc*);

int  Cz80_Exec(cz80_struc*);

int  Cz80_Set_NMI(cz80_struc*);

unsigned int Cz80_Get_BC(cz80_struc*);

unsigned int Cz80_Get_DE(cz80_struc*);

unsigned int Cz80_Get_HL(cz80_struc*);

unsigned int Cz80_Get_PC(cz80_struc*);
*/


#ifdef __cplusplus
};
#endif

#endif	/* CZ80_H */
