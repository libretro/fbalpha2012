/******************************************************************************
 *
 * CZ80 (Z80 CPU emulator) version 0.9
 * Compiled with Dev-C++
 * Copyright 2004-2005 Stphane Dallongeville
 *
 * (Modified by NJ)
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/******************************/
/* Compiler dependant defines */
/******************************/
/*
#define UINT8	unsigned char
#define INT8	signed char
#define UINT16	unsigned short
#define INT16	signed short
#define UINT32	unsigned int
#define INT32	signed int
*/
#include "cz80.h"

#define CF					0x01
#define NF					0x02
#define PF					0x04
#define VF					PF
#define XF					0x08
#define HF					0x10
#define YF					0x20
#define ZF					0x40
#define SF					0x80

#if CZ80_USE_JUMPTABLE
#define _SSOP(A,B)			A##B
#define OP(A)				_SSOP(OP,A)
#define OPCB(A)				_SSOP(OPCB,A)
#define OPED(A)				_SSOP(OPED,A)
#define OPXY(A)				_SSOP(OPXY,A)
#define OPXYCB(A)			_SSOP(OPXYCB,A)
#else
#define OP(A)				case A
#define OPCB(A)				case A
#define OPED(A)				case A
#define OPXY(A)				case A
#define OPXYCB(A)			case A
#endif

#define USE_CYCLES(A)		CPU->nCyclesLeft -= (A);
#define ADD_CYCLES(A)		CPU->nCyclesLeft += (A);

#define RET(A)															\
        if ((CPU->nCyclesLeft -= A) > 0) goto Cz80_Exec;				\
        goto Cz80_Exec_End;

#define SET_PC(A)															\
	CPU->BasePC = (unsigned int)CPU->FetchData[(A) >> CZ80_FETCH_SFT];		\
	 if(CPU->BasePC<=0) \
	{ \
		CPU->nCyclesLeft=-1; \
		goto Cz80_Exec_Really_End; \
	} \
	PCDiff = (UINT32)CPU->Fetch[(A) >> CZ80_FETCH_SFT] - (UINT32)CPU->FetchData[(A) >> CZ80_FETCH_SFT];\
	PC = (unsigned int)(A) + CPU->BasePC;

#define GET_OP()			(*(UINT8 *)(PC + PCDiff))

#define READ_OP()			GET_OP(); PC++

#define READ_ARG()			(*(UINT8 *)PC++)

#if CZ80_LITTLE_ENDIAN
#define READ_ARG16()		(*(UINT8 *)PC | (*(UINT8 *)(PC + 1) << 8)); PC += 2
#else
#define READ_ARG16()		(*(UINT8 *)(PC + 1) | (*(UINT8 *)PC << 8)); PC += 2
#endif

#define READ_MEM8(A, D)												\
	{																\
		UINT8 * ptr = CPU->Read[(A) >> CZ80_FETCH_SFT];				\
		if ( ptr ) {												\
			D = ptr[A];												\
		} else {													\
			CPU->PC = PC;											\
			D = CPU->Read_Byte(A);									\
		}															\
	}

#define READ_MEM16(A, D)											\
	{																\
		UINT8 * ptr = CPU->Read[(A) >> CZ80_FETCH_SFT];				\
		if ( ptr ) {												\
			D = ptr[A] | (ptr[(A)+1] << 8);							\
		} else {													\
			CPU->PC = PC;											\
			D = CPU->Read_Byte(A) | (CPU->Read_Byte((A)+1) << 8);	\
		}															\
	}

#define WRITE_MEM8(A, D)											\
	{																\
		UINT8 * ptr = CPU->Write[(A) >> CZ80_FETCH_SFT];			\
		if ( ptr ) ptr[A] = D;										\
		else {														\
			CPU->PC = PC;											\
			CPU->Write_Byte(A, D);									\
		}															\
	}

#define WRITE_MEM16(A, D)											\
	{																\
		UINT8 * ptr = CPU->Write[(A) >> CZ80_FETCH_SFT];			\
		if ( ptr ) {												\
			ptr[A] = D;												\
			ptr[(A)+1] = (D) >> 8;									\
		} else {													\
			CPU->PC = PC;											\
			CPU->Write_Byte(A, D);									\
			CPU->Write_Byte((A)+1, (D) >> 8);						\
		}															\
	}

#define PUSH_16(A)													\
	{																\
		UINT8 *ptr;													\
		zSP -= 2; 													\
		ptr = CPU->Write[(zSP) >> CZ80_FETCH_SFT];					\
		if ( ptr ) {												\
			ptr[zSP] = A;											\
			ptr[(zSP)+1] = (A) >> 8;								\
		}															\
	}

#define POP_16(A)													\
	{ 																\
		UINT8 *ptr = CPU->Read[(zSP) >> CZ80_FETCH_SFT];			\
		if ( ptr )													\
			A = ptr[zSP] | (ptr[(zSP)+1] << 8);						\
		zSP += 2; 													\
	}

#define IN(A)				CPU->IN_Port(A)
#define OUT(A, D)			CPU->OUT_Port(A, D)

static UINT8 SZ[256];
static UINT8 SZP[256];
static UINT8 SZ_BIT[256];
static UINT8 SZHV_inc[256];
static UINT8 SZHV_dec[256];
#if CZ80_BIG_FLAGS_ARRAY
static UINT8 SZHVC_add[2*256*256];
static UINT8 SZHVC_sub[2*256*256];
#endif


/******************************************************************************
	CZ80C郃^tF[X֐
******************************************************************************/

inline static void Cz80_InitFlags()
{
	UINT32 i, j, p;
#if CZ80_BIG_FLAGS_ARRAY
	int oldval, newval, val;
	UINT8 *padd, *padc, *psub, *psbc;
#endif
	
	// flags tables initialisation
	for (i = 0; i < 256; i++)
	{
		SZ[i] = i & (SF | YF | XF);
		if (!i) SZ[i] |= ZF;

		SZ_BIT[i] = i & (SF | YF | XF);
		if (!i) SZ_BIT[i] |= ZF | PF;

		for (j = 0, p = 0; j < 8; j++) if (i & (1 << j)) p++;
		SZP[i] = SZ[i];
		if (!(p & 1)) SZP[i] |= PF;

		SZHV_inc[i] = SZ[i];
		if(i == 0x80) SZHV_inc[i] |= VF;
		if((i & 0x0f) == 0x00) SZHV_inc[i] |= HF;

		SZHV_dec[i] = SZ[i] | NF;
		if (i == 0x7f) SZHV_dec[i] |= VF;
		if ((i & 0x0f) == 0x0f) SZHV_dec[i] |= HF;
	}

#if CZ80_BIG_FLAGS_ARRAY
	{
	padd = &SZHVC_add[  0*256];
	padc = &SZHVC_add[256*256];
	psub = &SZHVC_sub[  0*256];
	psbc = &SZHVC_sub[256*256];

	for (oldval = 0; oldval < 256; oldval++)
	{
		for (newval = 0; newval < 256; newval++)
		{
			/* add or adc w/o carry set */
			val = newval - oldval;
			*padd = (newval) ? ((newval & 0x80) ? SF : 0) : ZF;
			*padd |= (newval & (YF | XF));	/* undocumented flag bits 5+3 */
			if ((newval & 0x0f) < (oldval & 0x0f)) *padd |= HF;
			if (newval < oldval ) *padd |= CF;
			if ((val ^ oldval ^ 0x80) & (val ^ newval) & 0x80) *padd |= VF;
			padd++;

			/* adc with carry set */
			val = newval - oldval - 1;
			*padc = (newval) ? ((newval & 0x80) ? SF : 0) : ZF;
			*padc |= (newval & (YF | XF));	/* undocumented flag bits 5+3 */
			if ((newval & 0x0f) <= (oldval & 0x0f)) *padc |= HF;
			if (newval <= oldval) *padc |= CF;
			if ((val ^ oldval ^ 0x80) & (val ^ newval) & 0x80) *padc |= VF;
			padc++;

			/* cp, sub or sbc w/o carry set */
			val = oldval - newval;
			*psub = NF | ((newval) ? ((newval & 0x80) ? SF : 0) : ZF);
			*psub |= (newval & (YF | XF));	/* undocumented flag bits 5+3 */
			if ((newval & 0x0f) > (oldval & 0x0f)) *psub |= HF;
			if (newval > oldval) *psub |= CF;
			if ((val^oldval) & (oldval^newval) & 0x80) *psub |= VF;
			psub++;

			/* sbc with carry set */
			val = oldval - newval - 1;
			*psbc = NF | ((newval) ? ((newval & 0x80) ? SF : 0) : ZF);
			*psbc |= (newval & (YF | XF));	/* undocumented flag bits 5+3 */
			if ((newval & 0x0f) >= (oldval & 0x0f)) *psbc |= HF;
			if (newval >= oldval) *psbc |= CF;
			if ((val ^ oldval) & (oldval^newval) & 0x80) *psbc |= VF;
			psbc++;
		}
	}
	}
#endif
}

inline static void Cz80_Init(cz80_struc *CPU)
{
	memset(CPU, 0, sizeof(cz80_struc));

	CPU->pzR8[0] = &zB;
	CPU->pzR8[1] = &zC;
	CPU->pzR8[2] = &zD;
	CPU->pzR8[3] = &zE;
	CPU->pzR8[4] = &zH;
	CPU->pzR8[5] = &zL;
	CPU->pzR8[6] = &zF;	// ̓sAAあƓւ
	CPU->pzR8[7] = &zA;	// ̓sAFあƓւ

	CPU->pzR16[0] = pzBC;
	CPU->pzR16[1] = pzDE;
	CPU->pzR16[2] = pzHL;
	CPU->pzR16[3] = pzAF;

}

/*--------------------------------------------------------
	CPU
s
--------------------------------------------------------*/

inline static INT32 Cz80_Exec(cz80_struc* CPU)
{

#if CZ80_USE_JUMPTABLE
    #include "cz80jmp.c"
#endif

    UINT32 PC;
    UINT32 PCDiff;    
    
    UINT32 Opcode;
    
    UINT32 adr = 0;
	UINT32 res;
	UINT32 val;
    
    INT32 nTodo = 0;

    PC = CPU->PC;
    PCDiff = (UINT32)CPU->Fetch[(zRealPC) >> CZ80_FETCH_SFT] - (UINT32)CPU->FetchData[(zRealPC) >> CZ80_FETCH_SFT];
    
//	CPU->nEI = 0;
	
	goto Cz80_Try_Int;

Cz80_Exec:
    {
        union16 *data = pzHL;
        Opcode = READ_OP();
        
        #include "cz80_op.c"
        
    }
    
Cz80_Try_Int:
	
	// DOZE Try Interrupt
	
	if (( CPU->nInterruptLatch & CZ80_IRQSTATUS_NONE ) == 0 ) {

		if ( zIFF1 == 0 ) goto Cz80_Exec_End;
		
		if ( GET_OP() == 0x76 ) PC++;
		
		zIFF = 0;
		if ( zIM == 0 ) {
			PUSH_16( zRealPC );
			SET_PC( CPU->nInterruptLatch & 0x38 );
			if ( CPU->nInterruptLatch & CZ80_IRQSTATUS_AUTO ) CPU->nInterruptLatch = CZ80_IRQSTATUS_NONE;
			CPU->nCyclesLeft -= 13;
		} else {
			if ( zIM == 2 ) {
				int nTabAddr = 0, nIntAddr = 0;
				nTabAddr = ((unsigned short)zI << 8) + (CPU->nInterruptLatch & 0xFF);
				READ_MEM16( nTabAddr, nIntAddr );
				PUSH_16( zRealPC );
				SET_PC( nIntAddr );
				if ( CPU->nInterruptLatch & CZ80_IRQSTATUS_AUTO ) CPU->nInterruptLatch = CZ80_IRQSTATUS_NONE;
				CPU->nCyclesLeft -= 19;
			} else {
				PUSH_16( zRealPC );
				SET_PC( 0x38 );
				if ( CPU->nInterruptLatch & CZ80_IRQSTATUS_AUTO ) CPU->nInterruptLatch = CZ80_IRQSTATUS_NONE;
				CPU->nCyclesLeft -= 13;
			}
		}
	}
	
	if ( CPU->nEI == 8 ) {
		CPU->nEI = 0;
		goto Cz80_Exec;
	}

	if ( CPU->nCyclesLeft < 0 ) {
		goto Cz80_Exec_Really_End;
	}
	
	if ( GET_OP() == 0x76 ) {
		int nDid = (CPU->nCyclesLeft >> 2) + 1;
		zR1 = (zR + nDid) & 0x7F;
		CPU->nCyclesLeft -= nDid;
		goto Cz80_Exec_Really_End;
	}
		
	CPU->nEI = 1;
	goto Cz80_Exec;

Cz80_Exec_End:

	if ( CPU->nEI == 8 ) {
		CPU->nEI = 0;
		goto Cz80_Exec;
	}

	if ( CPU->nEI == 2 ) {
		// (do one more instruction before interrupt)
		nTodo = CPU->nCyclesLeft;
		CPU->nCyclesLeft = 0;
		CPU->nEI = 4;
		goto Cz80_Exec;
	}
	
	if ( CPU->nEI == 4 ) {
		CPU->nCyclesLeft += nTodo;
		nTodo = 0;
		CPU->nEI = 8;
		goto Cz80_Try_Int;
	}
	
	if ( CPU->nCyclesLeft > 0 ) goto Cz80_Exec;
	
Cz80_Exec_Really_End:
	
    CPU->PC = PC;
    // update R register
    zR1 = (zR + ((CPU->nCyclesSegment - CPU->nCyclesLeft) >> 2)) & 0x7F;
	
    return CPU->nCyclesSegment - CPU->nCyclesLeft;
}


inline static UINT32 Cz80_Get_BC(cz80_struc *CPU)
{
    return zBC;
}

inline static UINT32 Cz80_Get_DE(cz80_struc *CPU)
{
    return zDE;
} 

inline static UINT32 Cz80_Get_HL(cz80_struc *CPU)
{
    return zHL;
} 

inline static UINT32 Cz80_Get_PC(cz80_struc *CPU)
{
    return CPU->PC - CPU->BasePC;
}

inline static void Cz80_Set_PC(cz80_struc *CPU, UINT32 val)
{
	// don't not use this while CZ80 is executting !!!
    CPU->BasePC = (UINT32) CPU->FetchData[val >> CZ80_FETCH_SFT];
    if(CPU->BasePC<=0) return;
    CPU->PC = val + CPU->BasePC;
}

inline static void Cz80_Reset(cz80_struc *CPU)
{
	memset(CPU, 0, (INT32)&CPU->nCyclesTotal - (INT32)CPU);

	zIX = zIY = 0xffff;
	zF = ZF;
	
	Cz80_Set_PC(CPU, 0);
	
	zSP = 0xf000;
	zF2 = ZF;
}

inline static INT32 Cz80_Set_NMI(cz80_struc *CPU)
{
    zIFF1 = 0;
	PUSH_16( CPU->PC - CPU->BasePC );
	Cz80_Set_PC( CPU, 0x66 );
	return 12;
}


