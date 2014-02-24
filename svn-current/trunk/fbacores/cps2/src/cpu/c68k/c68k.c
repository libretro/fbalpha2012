/******************************************************************************
 *
 * C68K (68000 CPU emulator) version 0.80
 * Compiled with Dev-C++
 * Copyright 2003-2004 Stephane Dallongeville
 *
 * (Modified by NJ)
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "c68k.h"

#ifndef IRQ_LINE_STATE
#define IRQ_LINE_STATE

#define CLEAR_LINE		0		/* clear (a fired, held or pulsed) line */
#define ASSERT_LINE		1		/* assert an interrupt immediately */
#define HOLD_LINE		2		/* hold interrupt line until acknowledged */
#define PULSE_LINE		3		/* pulse interrupt line for one instruction */
#define IRQ_LINE_NMI	127		/* IRQ line for NMIs */
#endif

#define UINT8	unsigned char
#define INT8	char
#define UINT16	unsigned short
#define INT16	short
#define UINT32	unsigned int
#define INT32	int
#define u16 	unsigned short
#define u32 	unsigned int

#define ALIGN_DATA				__attribute__((aligned(4)))

/******************************************************************************
	}N
******************************************************************************/

#include "c68kmacro.h"


/******************************************************************************
	O[o\
******************************************************************************/

c68k_struc ALIGN_DATA C68K;


/******************************************************************************
	̃[Jϐ
******************************************************************************/

static void  **JumpTable=NULL;
//static UINT8 ALIGN_DATA c68k_bad_address[1 << C68K_FETCH_SFT];


/******************************************************************************
	[J֐
******************************************************************************/

/*--------------------------------------------------------
	荞݃R[obN
--------------------------------------------------------*/

static INT32 C68k_InterruptCallback(INT32 line)
{
	return C68K_INTERRUPT_AUTOVECTOR_EX + line;
}


/*--------------------------------------------------------
	ZbgR[obN
--------------------------------------------------------*/

static void C68k_ResetCallback(void)
{
}


/******************************************************************************
	C68KC^tF[X֐
******************************************************************************/


static int bC68KInit = 1;



/*--------------------------------------------------------
	CPUs
--------------------------------------------------------*/

INT32 C68k_Exec(c68k_struc *CPU, INT32 cycles)
{
	if (CPU)
	{
		UINT32 PC;
		UINT32 Opcode;
		UINT32 adr;
		UINT32 res;
		UINT32 src;
		UINT32 dst;

		PC = CPU->PC;
		CPU->ICount = cycles;

C68k_Check_Interrupt:
		CHECK_INT
		if (!CPU->HaltState)
		{

C68k_Exec_Next:
			if (CPU->ICount > 0)
			{
			#ifdef FBA_DEBUG
				if(CPU->Dbg_CallBack) {
					CPU->PC = PC;
					CPU->Dbg_CallBack();
				}
			#endif
				Opcode = READ_IMM_16();
				PC += 2;
				goto *JumpTable[Opcode];

				#include "c68k_op.h"
			}
		}

		CPU->PC = PC;

		return cycles - CPU->ICount;
	}
	else
	{
		#include "c68k_ini.h"
	}

	return 0;
}


/*--------------------------------------------------------
	荞ݏ
--------------------------------------------------------*/

void C68k_Set_IRQ(c68k_struc *CPU, INT32 line, INT32 state)
{
	CPU->IRQState = state;
	if (state == CLEAR_LINE)
	{
		CPU->IRQLine = 0;
	}
	else
	{
		CPU->IRQLine = line;
		CPU->HaltState = 0;
	}
}


/*--------------------------------------------------------
	WX^擾
--------------------------------------------------------*/

UINT32 C68k_Get_Reg(c68k_struc *CPU, INT32 regnum)
{
	switch (regnum)
	{
	case C68K_PC:  return (CPU->PC - CPU->BasePC);
	case C68K_USP: return (CPU->flag_S ? CPU->USP : CPU->A[7]);
	case C68K_MSP: return (CPU->flag_S ? CPU->A[7] : CPU->USP);
	case C68K_SR:  return GET_SR();
	case C68K_D0:  return CPU->D[0];
	case C68K_D1:  return CPU->D[1];
	case C68K_D2:  return CPU->D[2];
	case C68K_D3:  return CPU->D[3];
	case C68K_D4:  return CPU->D[4];
	case C68K_D5:  return CPU->D[5];
	case C68K_D6:  return CPU->D[6];
	case C68K_D7:  return CPU->D[7];
	case C68K_A0:  return CPU->A[0];
	case C68K_A1:  return CPU->A[1];
	case C68K_A2:  return CPU->A[2];
	case C68K_A3:  return CPU->A[3];
	case C68K_A4:  return CPU->A[4];
	case C68K_A5:  return CPU->A[5];
	case C68K_A6:  return CPU->A[6];
	case C68K_A7:  return CPU->A[7];
	default: return 0;
	}
}


/*--------------------------------------------------------
	WX^ݒ
--------------------------------------------------------*/

void C68k_Set_Reg(c68k_struc *CPU, INT32 regnum, UINT32 val)
{
	switch (regnum)
	{
	case C68K_PC:
		CPU->PC = CPU->Rebase_PC( val );
		//CPU->BasePC = CPU->Fetch[(val >> C68K_FETCH_SFT) & C68K_FETCH_MASK];
		//CPU->BasePC -= val & 0xff000000;
		//CPU->PC = val + CPU->BasePC;
		break;

	case C68K_USP:
		if (CPU->flag_S) CPU->USP = val;
		else CPU->A[7] = val;
		break;

	case C68K_MSP:
		if (CPU->flag_S) CPU->A[7] = val;
		else CPU->USP = val;
		break;

	case C68K_SR: SET_SR(val); break;
	case C68K_D0: CPU->D[0] = val; break;
	case C68K_D1: CPU->D[1] = val; break;
	case C68K_D2: CPU->D[2] = val; break;
	case C68K_D3: CPU->D[3] = val; break;
	case C68K_D4: CPU->D[4] = val; break;
	case C68K_D5: CPU->D[5] = val; break;
	case C68K_D6: CPU->D[6] = val; break;
	case C68K_D7: CPU->D[7] = val; break;
	case C68K_A0: CPU->A[0] = val; break;
	case C68K_A1: CPU->A[1] = val; break;
	case C68K_A2: CPU->A[2] = val; break;
	case C68K_A3: CPU->A[3] = val; break;
	case C68K_A4: CPU->A[4] = val; break;
	case C68K_A5: CPU->A[5] = val; break;
	case C68K_A6: CPU->A[6] = val; break;
	case C68K_A7: CPU->A[7] = val; break;
	default: break;
	}

}


/*--------------------------------------------------------
	t胃Fb`AhXݒ
--------------------------------------------------------*/

#if 0
void C68k_Set_Fetch(c68k_struc *CPU, UINT32 low_adr, UINT32 high_adr, UINT32 fetch_adr)
{
	UINT32 i, j;

	i = (low_adr >> C68K_FETCH_SFT) & C68K_FETCH_MASK;
	j = (high_adr >> C68K_FETCH_SFT) & C68K_FETCH_MASK;
	fetch_adr -= i << C68K_FETCH_SFT;
	while (i <= j) CPU->Fetch[i++] = fetch_adr;
}
#endif


/*--------------------------------------------------------
	胁[h/Cg֐ݒ
--------------------------------------------------------*/

void C68k_Set_ReadB(c68k_struc *CPU, UINT8 (*Func)(UINT32 address))
{
	CPU->Read_Byte = Func;
	CPU->Read_Byte_PC_Relative = Func;
}

void C68k_Set_ReadW(c68k_struc *CPU, UINT16 (*Func)(UINT32 address))
{
	CPU->Read_Word = Func;
	CPU->Read_Word_PC_Relative = Func;
}

void C68k_Set_ReadB_PC_Relative(c68k_struc *CPU, UINT8 (*Func)(UINT32 address))
{
	CPU->Read_Byte_PC_Relative = Func;
}

void C68k_Set_ReadW_PC_Relative(c68k_struc *CPU, UINT16 (*Func)(UINT32 address))
{
	CPU->Read_Word_PC_Relative = Func;
}

void C68k_Set_WriteB(c68k_struc *CPU, void (*Func)(UINT32 address, UINT8 data))
{
	CPU->Write_Byte = Func;
}

void C68k_Set_WriteW(c68k_struc *CPU, void (*Func)(UINT32 address, UINT16 data))
{
	CPU->Write_Word = Func;
}


/*--------------------------------------------------------
	R胁[obN֐ݒ
--------------------------------------------------------*/

void C68k_Set_IRQ_Callback(c68k_struc *CPU, INT32 (*Func)(INT32 irqline))
{
	CPU->Interrupt_CallBack = Func;
}

void C68k_Set_Reset_Callback(c68k_struc *CPU, void (*Func)(void))
{
	CPU->Reset_CallBack = Func;
}


/*--------------------------------------------------------
	CPU?
--------------------------------------------------------*/
void C68k_Init(c68k_struc *CPU)
{
	// These callbacks are set before calling C68k_Init, no need to stub them
	// This fixes outrun eventually
	//if(!CPU->Interrupt_CallBack) CPU->Interrupt_CallBack = C68k_InterruptCallback;
	//if(!CPU->Reset_CallBack) CPU->Reset_CallBack = C68k_ResetCallback;
	CPU->Interrupt_CallBack = C68k_InterruptCallback;
	CPU->Reset_CallBack = C68k_ResetCallback;
	if ( !bC68KInit ) {
		C68k_Exec(NULL, 0);
		bC68KInit = 1;
	}

}
void C68k_Exit()
{
	if(JumpTable!=NULL)
	{
		free(JumpTable);
		JumpTable=NULL;
		bC68KInit = 0;
	}
}
/*--------------------------------------------------------
	CPU?Zbg
--------------------------------------------------------*/

void C68k_Reset(c68k_struc *CPU)
{
	UINT32 PC;

	memset(CPU, 0, (UINT32)&CPU->BasePC - (UINT32)CPU);

	CPU->flag_I = 7;
	CPU->flag_S = C68K_SR_S;

	CPU->A[7]  = READ_PCREL_32(0);
	PC = READ_PCREL_32(4);

	C68k_Set_Reg(CPU, C68K_PC, PC);
}
