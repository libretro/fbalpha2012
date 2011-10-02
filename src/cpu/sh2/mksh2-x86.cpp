/*
 * SH-2 Core Creator
 *
 * (C) 2008 OopsWare
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>


static FILE *OpCodeFile=NULL;

void ot(char *format, ...)
{
	va_list valist=NULL;
	int i, len;

	for(i=0, len=strlen(format); i < len && format[i] != '\n'; i++);
	if(i < len-1 && format[len-1] != '\n') printf("\nWARNING: possible improper newline placement:\n%s\n", format);

	va_start(valist,format);
	if (OpCodeFile) vfprintf(OpCodeFile,format,valist);
	va_end(valist);
}

//#define Rn	((opcode>>8)&15)
//#define Rm	((opcode>>4)&15)

void Op____(unsigned short)
{
//	ot("	\n");
}

void OpSTCSR(unsigned short n)
{
	ot("	sh2->r[%d] = sh2->sr;\n", (n >> 8) & 0x0f);
}

void OpBSRF(unsigned short n)
{
	ot("	sh2->pr = sh2->pc + 2;\n");
	ot("	sh2->delay = sh2->pc;\n");
	ot("	sh2->pc += sh2->r[%d] + 2;\n", (n >> 8) & 0x0f);
	ot("	sh2->sh2_icount--;\n");
}

void OpMOVBS0(unsigned short op)
{
	int n = (op>>8) & 0x0f;
	int m = (op>>4) & 0x0f;
	ot("	sh2->ea = sh2->r[%d] + sh2->r[0];\n", n);
	ot("	WB( sh2->ea, sh2->r[%d] & 0x000000ff );\n", m);
}

void OpMOVWS0(unsigned short op)
{
	int n = (op>>8) & 0x0f;
	int m = (op>>4) & 0x0f;
	ot("	sh2->ea = sh2->r[%d] + sh2->r[0];\n", n);
	ot("	WW( sh2->ea, sh2->r[%d] & 0x0000ffff );\n", m);
}

void OpMOVLS0(unsigned short op)
{
	int n = (op>>8) & 0x0f;
	int m = (op>>4) & 0x0f;
	ot("	sh2->ea = sh2->r[%d] + sh2->r[0];\n", n);
	ot("	WL( sh2->ea, sh2->r[%d] );\n", m);
}

void OpMULL(unsigned short op)
{
	int n = (op>>8) & 0x0f;
	int m = (op>>4) & 0x0f;
	ot("	sh2->macl = sh2->r[%d] * sh2->r[%d];\n", n, m);
	ot("	sh2->sh2_icount--;\n");
}

void OpCLRT(unsigned short)
{
	ot("	sh2->sr &= ~T;\n");
}

void OpSTSMACH(unsigned short n)
{
	ot("	sh2->r[%d] = sh2->mach;\n", (n >> 8) & 0x0f);
}

void OpRTS(unsigned short)
{
	ot("	sh2->delay = sh2->pc;\n");
	ot("	sh2->pc = sh2->ea = sh2->pr;\n");
	ot("	sh2->sh2_icount--;\n");
}

void OpMOVBL0(unsigned short op)
{
	int n = (op>>8) & 0x0f;
	int m = (op>>4) & 0x0f;
	ot("	sh2->ea = sh2->r[%d] + sh2->r[0];\n", m);
	ot("	sh2->r[%d] = (UINT32)(INT32)(INT16)(INT8) RB( sh2->ea );\n", n);
}

void OpMOVWL0(unsigned short op)
{
	int n = (op>>8) & 0x0f;
	int m = (op>>4) & 0x0f;
	ot("	sh2->ea = sh2->r[%d] + sh2->r[0];\n", m);
	ot("	sh2->r[%d] = (UINT32)(INT32)(INT16) RW( sh2->ea );\n", n);
}

void OpMOVLL0(unsigned short op)
{
	int n = (op>>8) & 0x0f;
	int m = (op>>4) & 0x0f;
	ot("	sh2->ea = sh2->r[%d] + sh2->r[0];\n", m);
	ot("	sh2->r[%d] = RL( sh2->ea );\n", n);
}


void OpMAC_L(unsigned short )
{
	ot("	unsigned int n = (opcode>>8) & 0x0f;\n");
	ot("	unsigned int m = (opcode>>4) & 0x0f;\n");
	ot("\n");
	ot("	UINT32 RnL, RnH, RmL, RmH, Res0, Res1, Res2;\n");
	ot("	UINT32 temp0, temp1, temp2, temp3;\n");
	ot("	INT32 tempm, tempn, fnLmL;\n");
	ot("\n");
	ot("	tempn = (INT32) RL( sh2->r[n] );\n");
	ot("	sh2->r[n] += 4;\n");
	ot("	tempm = (INT32) RL( sh2->r[m] );\n");
	ot("	sh2->r[m] += 4;\n");
	ot("	if ((INT32) (tempn ^ tempm) < 0)\n");
	ot("		fnLmL = -1;\n");
	ot("	else\n");
	ot("		fnLmL = 0;\n");
	ot("	if (tempn < 0)\n");
	ot("		tempn = 0 - tempn;\n");
	ot("	if (tempm < 0)\n");
	ot("		tempm = 0 - tempm;\n");
	ot("	temp1 = (UINT32) tempn;\n");
	ot("	temp2 = (UINT32) tempm;\n");
	ot("	RnL = temp1 & 0x0000ffff;\n");
	ot("	RnH = (temp1 >> 16) & 0x0000ffff;\n");
	ot("	RmL = temp2 & 0x0000ffff;\n");
	ot("	RmH = (temp2 >> 16) & 0x0000ffff;\n");
	ot("	temp0 = RmL * RnL;\n");
	ot("	temp1 = RmH * RnL;\n");
	ot("	temp2 = RmL * RnH;\n");
	ot("	temp3 = RmH * RnH;\n");
	ot("	Res2 = 0;\n");
	ot("	Res1 = temp1 + temp2;\n");
	ot("	if (Res1 < temp1)\n");
	ot("		Res2 += 0x00010000;\n");
	ot("	temp1 = (Res1 << 16) & 0xffff0000;\n");
	ot("	Res0 = temp0 + temp1;\n");
	ot("	if (Res0 < temp0)\n");
	ot("		Res2++;\n");
	ot("	Res2 = Res2 + ((Res1 >> 16) & 0x0000ffff) + temp3;\n");
	ot("	if (fnLmL < 0)\n");
	ot("	{\n");
	ot("		Res2 = ~Res2;\n");
	ot("		if (Res0 == 0)\n");
	ot("			Res2++;\n");
	ot("		else\n");
	ot("			Res0 = (~Res0) + 1;\n");
	ot("	}\n");
	ot("	if (sh2->sr & S)\n");
	ot("	{\n");
	ot("		Res0 = sh2->macl + Res0;\n");
	ot("		if (sh2->macl > Res0)\n");
	ot("			Res2++;\n");
	ot("		Res2 += (sh2->mach & 0x0000ffff);\n");
	ot("		if (((INT32) Res2 < 0) && (Res2 < 0xffff8000))\n");
	ot("		{\n");
	ot("			Res2 = 0x00008000;\n");
	ot("			Res0 = 0x00000000;\n");
	ot("		}\n");
	ot("		else if (((INT32) Res2 > 0) && (Res2 > 0x00007fff))\n");
	ot("		{\n");
	ot("			Res2 = 0x00007fff;\n");
	ot("			Res0 = 0xffffffff;\n");
	ot("		}\n");
	ot("		sh2->mach = Res2;\n");
	ot("		sh2->macl = Res0;\n");
	ot("	}\n");
	ot("	else\n");
	ot("	{\n");
	ot("		Res0 = sh2->macl + Res0;\n");
	ot("		if (sh2->macl > Res0)\n");
	ot("			Res2++;\n");
	ot("		Res2 += sh2->mach;\n");
	ot("		sh2->mach = Res2;\n");
	ot("		sh2->macl = Res0;\n");
	ot("	}\n");
	ot("	sh2->sh2_icount -= 2;\n");
}

void OpSTCGBR(unsigned short n)
{
	ot("	sh2->r[%d] = sh2->gbr;\n", (n >> 8) & 0x0f);
}

void OpSETT(unsigned short )
{
	ot("	sh2->sr |= T;\n");
}

void OpDIV0U(unsigned short )
{
	ot("	sh2->sr &= ~(M | Q | T);\n");
}

void OpSTSMACL(unsigned short n)
{
	ot("	sh2->r[%d] = sh2->macl;\n", (n >> 8) & 0x0f);
}

void OpSLEEP(unsigned short )
{
	ot("	sh2->pc -= 2;\n");
	ot("	sh2->sh2_icount -= 2;\n");
}

void OpSTCVBR(unsigned short n)
{
	ot("	sh2->r[%d] = sh2->vbr;\n", (n >> 8) & 0x0f);
}

void OpBRAF(unsigned short n)
{
	ot("	sh2->delay = sh2->pc;\n");
	ot("	sh2->pc += sh2->r[%d] + 2;\n", (n >> 8) & 0x0f);
	ot("	sh2->sh2_icount--;\n");
}

void OpCLRMAC(unsigned short)
{
	ot("	sh2->mach = 0;\n");
	ot("	sh2->macl = 0;\n");
}

void OpMOVT(unsigned short n)
{
	ot("	sh2->r[%d] = sh2->sr & T;\n", (n >> 8) & 0x0f);
}

void OpSTSPR(unsigned short n)
{
	ot("	sh2->r[%d] = sh2->pr;\n", (n >> 8) & 0x0f);
}

void OpRTE(unsigned short)
{
	ot("	sh2->ea = sh2->r[15];\n");
	ot("	sh2->delay = sh2->pc;\n");
	ot("	sh2->pc = RL( sh2->ea );\n");
	ot("	sh2->r[15] += 4;\n");
	ot("	sh2->ea = sh2->r[15];\n");
	ot("	sh2->sr = RL( sh2->ea ) & FLAGS;\n");
	ot("	sh2->r[15] += 4;\n");
	ot("	sh2->sh2_icount -= 3;\n");
	ot("	sh2->test_irq = 1;\n");
}

void OpMOVLS4(unsigned short)
{
	ot("	sh2->ea = sh2->r[(opcode>>8) & 0x0f] + ((opcode<<2) & 0x3c);\n");
	ot("	WL( sh2->ea, sh2->r[(opcode>>4) & 0x0f] );\n");
}

void OpMOVBS(unsigned short n)
{
	ot("	sh2->ea = sh2->r[%d];\n", (n >> 8) & 0x0f);
	ot("	WB( sh2->ea, sh2->r[%d] & 0x000000ff);\n", (n >> 4) & 0x0f);
}

void OpMOVWS(unsigned short n)
{
	ot("	sh2->ea = sh2->r[%d];\n", (n >> 8) & 0x0f);
	ot("	WW( sh2->ea, sh2->r[%d] & 0x0000ffff);\n", (n >> 4) & 0x0f);
}

void OpMOVLS(unsigned short n)
{
	ot("	sh2->ea = sh2->r[%d];\n", (n >> 8) & 0x0f);
	ot("	WL( sh2->ea, sh2->r[%d] );\n", (n >> 4) & 0x0f);
}

void OpMOVBM(unsigned short n)
{
	ot("	UINT32 data = sh2->r[%d] & 0x000000ff;\n", (n >> 4) & 0x0f);
	ot("	sh2->r[%d] -= 1;\n", (n >> 8) & 0x0f);
	ot("	WB( sh2->r[%d], data );\n", (n >> 8) & 0x0f);
}

void OpMOVWM(unsigned short n)
{
	ot("	UINT32 data = sh2->r[%d] & 0x0000ffff;\n", (n >> 4) & 0x0f);
	ot("	sh2->r[%d] -= 2;\n", (n >> 8) & 0x0f);
	ot("	WW( sh2->r[%d], data );\n", (n >> 8) & 0x0f);
}

void OpMOVLM(unsigned short n)
{
	ot("	UINT32 data = sh2->r[%d];\n", (n >> 4) & 0x0f);
	ot("	sh2->r[%d] -= 4;\n", (n >> 8) & 0x0f);
	ot("	WL( sh2->r[%d], data );\n", (n >> 8) & 0x0f);
}

void OpDIV0S(unsigned short n)
{
	ot("	if ((sh2->r[%d] & 0x80000000) == 0)\n", (n >> 8) & 0x0f);
	ot("		sh2->sr &= ~Q;\n");
	ot("	else\n");
	ot("		sh2->sr |= Q;\n");
	ot("	if ((sh2->r[%d] & 0x80000000) == 0)\n", (n >> 4) & 0x0f);
	ot("		sh2->sr &= ~M;\n");
	ot("	else\n");
	ot("		sh2->sr |= M;\n");
	ot("	if ((sh2->r[%d] ^ sh2->r[%d]) & 0x80000000)\n", (n >> 4) & 0x0f, (n >> 8) & 0x0f);
	ot("		sh2->sr |= T;\n");
	ot("	else\n");
	ot("		sh2->sr &= ~T;\n");
}

void OpTST(unsigned short n)
{
	ot("	if ((sh2->r[%d] & sh2->r[%d]) == 0)\n", (n >> 8) & 0x0f, (n >> 4) & 0x0f);
	ot("		sh2->sr |= T;\n");
	ot("	else\n");
	ot("		sh2->sr &= ~T;\n");
}

void OpAND(unsigned short n)
{
	ot("	sh2->r[%d] &= sh2->r[%d];\n", (n >> 8) & 0x0f, (n >> 4) & 0x0f);
}

void OpXOR(unsigned short n)
{
	ot("	sh2->r[%d] ^= sh2->r[%d];\n", (n >> 8) & 0x0f, (n >> 4) & 0x0f);
}

void OpOR(unsigned short n)
{
	ot("	sh2->r[%d] |= sh2->r[%d];\n", (n >> 8) & 0x0f, (n >> 4) & 0x0f);
}

void OpCMPSTR(unsigned short n)
{
	ot("	unsigned int n = (opcode>>8) & 0x0f;\n");
	ot("	unsigned int m = (opcode>>4) & 0x0f;\n");	
	ot("	UINT32 temp;\n");
	ot("	INT32 HH, HL, LH, LL;\n");
	ot("	temp = sh2->r[n] ^ sh2->r[m];\n");
	ot("	HH = (temp >> 24) & 0xff;\n");
	ot("	HL = (temp >> 16) & 0xff;\n");
	ot("	LH = (temp >> 8) & 0xff;\n");
	ot("	LL = temp & 0xff;\n");
	ot("	if (HH && HL && LH && LL)\n");
	ot("		sh2->sr &= ~T;\n");
	ot("	else\n");
	ot("		sh2->sr |= T;\n");
}

void OpXTRCT(unsigned short n)
{
	ot("	unsigned int n = (opcode>>8) & 0x0f;\n");
	ot("	unsigned int m = (opcode>>4) & 0x0f;\n");	
	ot("	UINT32 temp;\n");
	ot("	temp = (sh2->r[m] << 16) & 0xffff0000;\n");
	ot("	sh2->r[n] = (sh2->r[n] >> 16) & 0x0000ffff;\n");
	ot("	sh2->r[n] |= temp;\n");
}

void OpMULU(unsigned short n)
{
	ot("	sh2->macl = (UINT16) sh2->r[%d] * (UINT16) sh2->r[%d];\n", (n >> 8) & 0x0f, (n >> 4) & 0x0f);
}

void OpMULS(unsigned short n)
{
	ot("	sh2->macl = (INT16) sh2->r[%d] * (INT16) sh2->r[%d];\n", (n >> 8) & 0x0f, (n >> 4) & 0x0f);
}

void OpCMPEQ(unsigned short n)
{
	ot("	if (sh2->r[%d] == sh2->r[%d])\n", (n >> 8) & 0x0f, (n >> 4) & 0x0f);
	ot("		sh2->sr |= T;\n");
	ot("	else\n");
	ot("		sh2->sr &= ~T;\n");
}
void OpCMPHS(unsigned short n)
{
	ot("	if ((UINT32) sh2->r[%d] >= (UINT32) sh2->r[%d])\n", (n >> 8) & 0x0f, (n >> 4) & 0x0f);
	ot("		sh2->sr |= T;\n");
	ot("	else\n");
	ot("		sh2->sr &= ~T;\n");
}

void OpCMPGE(unsigned short n)
{
	ot("	if ((INT32) sh2->r[%d] >= (INT32) sh2->r[%d])\n", (n >> 8) & 0x0f, (n >> 4) & 0x0f);
	ot("		sh2->sr |= T;\n");
	ot("	else\n");
	ot("		sh2->sr &= ~T;\n");
}

void OpDIV1(unsigned short )
{
	ot("	unsigned int n = (opcode>>8) & 0x0f;\n");
	ot("	unsigned int m = (opcode>>4) & 0x0f;\n");
	ot("\n");
	ot("	UINT32 tmp0;\n");
	ot("	UINT32 old_q;\n");
    ot("\n");
	ot("	old_q = sh2->sr & Q;\n");
	ot("	if (0x80000000 & sh2->r[n])\n");
	ot("		sh2->sr |= Q;\n");
	ot("	else\n");
	ot("		sh2->sr &= ~Q;\n");
	ot("\n");
	ot("	sh2->r[n] = (sh2->r[n] << 1) | (sh2->sr & T);\n");
    ot("\n");
	ot("	if (!old_q)\n");
	ot("	{\n");
	ot("		if (!(sh2->sr & M))\n");
	ot("		{\n");
	ot("			tmp0 = sh2->r[n];\n");
	ot("			sh2->r[n] -= sh2->r[m];\n");
	ot("			if(!(sh2->sr & Q))\n");
	ot("				if(sh2->r[n] > tmp0)\n");
	ot("					sh2->sr |= Q;\n");
	ot("				else\n");
	ot("					sh2->sr &= ~Q;\n");
	ot("			else\n");
	ot("				if(sh2->r[n] > tmp0)\n");
	ot("					sh2->sr &= ~Q;\n");
	ot("				else\n");
	ot("					sh2->sr |= Q;\n");
	ot("		}\n");
	ot("		else\n");
	ot("		{\n");
	ot("			tmp0 = sh2->r[n];\n");
	ot("			sh2->r[n] += sh2->r[m];\n");
	ot("			if(!(sh2->sr & Q))\n");
	ot("			{\n");
	ot("				if(sh2->r[n] < tmp0)\n");
	ot("					sh2->sr &= ~Q;\n");
	ot("				else\n");
	ot("					sh2->sr |= Q;\n");
	ot("			}\n");
	ot("			else\n");
	ot("			{\n");
	ot("				if(sh2->r[n] < tmp0)\n");
	ot("					sh2->sr |= Q;\n");
	ot("				else\n");
	ot("					sh2->sr &= ~Q;\n");
	ot("			}\n");
	ot("		}\n");
	ot("	}\n");
	ot("	else\n");
	ot("	{\n");
	ot("		if (!(sh2->sr & M))\n");
	ot("		{\n");
	ot("			tmp0 = sh2->r[n];\n");
	ot("			sh2->r[n] += sh2->r[m];\n");
	ot("			if(!(sh2->sr & Q))\n");
	ot("				if(sh2->r[n] < tmp0)\n");
	ot("					sh2->sr |= Q;\n");
	ot("				else\n");
	ot("					sh2->sr &= ~Q;\n");
	ot("			else\n");
	ot("				if(sh2->r[n] < tmp0)\n");
	ot("					sh2->sr &= ~Q;\n");
	ot("				else\n");
	ot("					sh2->sr |= Q;\n");
	ot("		}\n");
	ot("		else\n");
	ot("		{\n");
	ot("			tmp0 = sh2->r[n];\n");
	ot("			sh2->r[n] -= sh2->r[m];\n");
	ot("			if(!(sh2->sr & Q))\n");
	ot("				if(sh2->r[n] > tmp0)\n");
	ot("					sh2->sr &= ~Q;\n");
	ot("				else\n");
	ot("					sh2->sr |= Q;\n");
	ot("			else\n");
	ot("				if(sh2->r[n] > tmp0)\n");
	ot("					sh2->sr |= Q;\n");
	ot("				else\n");
	ot("					sh2->sr &= ~Q;\n");
	ot("		}\n");
	ot("	}\n");
    ot("\n");
	ot("	tmp0 = (sh2->sr & (Q | M));\n");
	ot("	if((!tmp0) || (tmp0 == 0x300)) /* if Q == M set T else clear T */\n");
	ot("		sh2->sr |= T;\n");
	ot("	else\n");
	ot("		sh2->sr &= ~T;\n");
}

void OpDMULU(unsigned short n)
{
	ot("	unsigned int n = (opcode>>8) & 0x0f;\n");
	ot("	unsigned int m = (opcode>>4) & 0x0f;\n");
	ot("\n");
	ot("	UINT32 RnL, RnH, RmL, RmH, Res0, Res1, Res2;\n");
	ot("	UINT32 temp0, temp1, temp2, temp3;\n");
	ot("\n");
	ot("	RnL = sh2->r[n] & 0x0000ffff;\n");
	ot("	RnH = (sh2->r[n] >> 16) & 0x0000ffff;\n");
	ot("	RmL = sh2->r[m] & 0x0000ffff;\n");
	ot("	RmH = (sh2->r[m] >> 16) & 0x0000ffff;\n");
	ot("	temp0 = RmL * RnL;\n");
	ot("	temp1 = RmH * RnL;\n");
	ot("	temp2 = RmL * RnH;\n");
	ot("	temp3 = RmH * RnH;\n");
	ot("	Res2 = 0;\n");
	ot("	Res1 = temp1 + temp2;\n");
	ot("	if (Res1 < temp1)\n");
	ot("		Res2 += 0x00010000;\n");
	ot("	temp1 = (Res1 << 16) & 0xffff0000;\n");
	ot("	Res0 = temp0 + temp1;\n");
	ot("	if (Res0 < temp0)\n");
	ot("		Res2++;\n");
	ot("	Res2 = Res2 + ((Res1 >> 16) & 0x0000ffff) + temp3;\n");
	ot("	sh2->mach = Res2;\n");
	ot("	sh2->macl = Res0;\n");
	ot("	sh2->sh2_icount--;\n");
}

void OpCMPHI(unsigned short n)
{
	ot("	if ((UINT32) sh2->r[%d] > (UINT32) sh2->r[%d])\n", (n >> 8) & 0x0f, (n >> 4) & 0x0f );
	ot("		sh2->sr |= T;\n");
	ot("	else\n");
	ot("		sh2->sr &= ~T;\n");
}

void OpCMPGT(unsigned short n)
{
	ot("	if ((INT32) sh2->r[%d] > (INT32) sh2->r[%d])\n", (n >> 8) & 0x0f, (n >> 4) & 0x0f );
	ot("		sh2->sr |= T;\n");
	ot("	else\n");
	ot("		sh2->sr &= ~T;\n");
}

void OpSUB(unsigned short n)
{
	ot("	sh2->r[%d] -= sh2->r[%d];\n", (n >> 8) & 0x0f, (n >> 4) & 0x0f );
}

void OpSUBC(unsigned short n)
{
	ot("	unsigned int n = (opcode>>8) & 0x0f;\n");
	ot("	unsigned int m = (opcode>>4) & 0x0f;\n");
	ot("\n");
	ot("	UINT32 tmp0, tmp1;\n");
	ot("	\n");
	ot("	tmp1 = sh2->r[n] - sh2->r[m];\n");
	ot("	tmp0 = sh2->r[n];\n");
	ot("	sh2->r[n] = tmp1 - (sh2->sr & T);\n");
	ot("	if (tmp0 < tmp1)\n");
	ot("		sh2->sr |= T;\n");
	ot("	else\n");
	ot("		sh2->sr &= ~T;\n");
	ot("	if (tmp1 < sh2->r[n])\n");
	ot("		sh2->sr |= T;\n");
}

void OpSUBV(unsigned short n)
{
	ot("	unsigned int n = (opcode>>8) & 0x0f;\n");
	ot("	unsigned int m = (opcode>>4) & 0x0f;\n");
	ot("\n");
	ot("	INT32 dest, src, ans;\n");
	ot("	\n");
	ot("	if ((INT32) sh2->r[n] >= 0)\n");
	ot("		dest = 0;\n");
	ot("	else\n");
	ot("		dest = 1;\n");
	ot("	if ((INT32) sh2->r[m] >= 0)\n");
	ot("		src = 0;\n");
	ot("	else\n");
	ot("		src = 1;\n");
	ot("	src += dest;\n");
	ot("	sh2->r[n] -= sh2->r[m];\n");
	ot("	if ((INT32) sh2->r[n] >= 0)\n");
	ot("		ans = 0;\n");
	ot("	else\n");
	ot("		ans = 1;\n");
	ot("	ans += dest;\n");
	ot("	if (src == 1)\n");
	ot("	{\n");
	ot("		if (ans == 1)\n");
	ot("			sh2->sr |= T;\n");
	ot("		else\n");
	ot("			sh2->sr &= ~T;\n");
	ot("	}\n");
	ot("	else\n");
	ot("		sh2->sr &= ~T;\n");
}

void OpADD(unsigned short n)
{
	ot("	sh2->r[%d] += sh2->r[%d];\n", (n >> 8) & 0x0f, (n >> 4) & 0x0f );
}

void OpDMULS(unsigned short n)
{
	ot("	unsigned int n = (opcode>>8) & 0x0f;\n");
	ot("	unsigned int m = (opcode>>4) & 0x0f;\n");
	ot("\n");

	ot("	UINT32 RnL, RnH, RmL, RmH, Res0, Res1, Res2;\n");
	ot("	UINT32 temp0, temp1, temp2, temp3;\n");
	ot("	INT32 tempm, tempn, fnLmL;\n");
    ot("	\n");
	ot("	tempn = (INT32) sh2->r[n];\n");
	ot("	tempm = (INT32) sh2->r[m];\n");
	ot("	if (tempn < 0)\n");
	ot("		tempn = 0 - tempn;\n");
	ot("	if (tempm < 0)\n");
	ot("		tempm = 0 - tempm;\n");
	ot("	if ((INT32) (sh2->r[n] ^ sh2->r[m]) < 0)\n");
	ot("		fnLmL = -1;\n");
	ot("	else\n");
	ot("		fnLmL = 0;\n");
	ot("	temp1 = (UINT32) tempn;\n");
	ot("	temp2 = (UINT32) tempm;\n");
	ot("	RnL = temp1 & 0x0000ffff;\n");
	ot("	RnH = (temp1 >> 16) & 0x0000ffff;\n");
	ot("	RmL = temp2 & 0x0000ffff;\n");
	ot("	RmH = (temp2 >> 16) & 0x0000ffff;\n");
	ot("	temp0 = RmL * RnL;\n");
	ot("	temp1 = RmH * RnL;\n");
	ot("	temp2 = RmL * RnH;\n");
	ot("	temp3 = RmH * RnH;\n");
	ot("	Res2 = 0;\n");
	ot("	Res1 = temp1 + temp2;\n");
	ot("	if (Res1 < temp1)\n");
	ot("		Res2 += 0x00010000;\n");
	ot("	temp1 = (Res1 << 16) & 0xffff0000;\n");
	ot("	Res0 = temp0 + temp1;\n");
	ot("	if (Res0 < temp0)\n");
	ot("		Res2++;\n");
	ot("	Res2 = Res2 + ((Res1 >> 16) & 0x0000ffff) + temp3;\n");
	ot("	if (fnLmL < 0)\n");
	ot("	{\n");
	ot("		Res2 = ~Res2;\n");
	ot("		if (Res0 == 0)\n");
	ot("			Res2++;\n");
	ot("		else\n");
	ot("			Res0 = (~Res0) + 1;\n");
	ot("	}\n");
	ot("	sh2->mach = Res2;\n");
	ot("	sh2->macl = Res0;\n");
	ot("	sh2->sh2_icount--;\n");
}

void OpADDC(unsigned short n)
{
	ot("	unsigned int n = (opcode>>8) & 0x0f;\n");
	ot("	unsigned int m = (opcode>>4) & 0x0f;\n");
	ot("\n");
	
	ot("	UINT32 tmp0, tmp1;\n");
    ot("	\n");
	ot("	tmp1 = sh2->r[n] + sh2->r[m];\n");
	ot("	tmp0 = sh2->r[n];\n");
	ot("	sh2->r[n] = tmp1 + (sh2->sr & T);\n");
	ot("	if (tmp0 > tmp1)\n");
	ot("		sh2->sr |= T;\n");
	ot("	else\n");
	ot("		sh2->sr &= ~T;\n");
	ot("	if (tmp1 > sh2->r[n])\n");
	ot("		sh2->sr |= T;\n");
} 

void OpADDV(unsigned short n)
{
	ot("	unsigned int n = (opcode>>8) & 0x0f;\n");
	ot("	unsigned int m = (opcode>>4) & 0x0f;\n");
	ot("\n");

	ot("	INT32 dest, src, ans;\n");
    ot("	\n");
	ot("	if ((INT32) sh2->r[n] >= 0)\n");
	ot("		dest = 0;\n");
	ot("	else\n");
	ot("		dest = 1;\n");
	ot("	if ((INT32) sh2->r[m] >= 0)\n");
	ot("		src = 0;\n");
	ot("	else\n");
	ot("		src = 1;\n");
	ot("	src += dest;\n");
	ot("	sh2->r[n] += sh2->r[m];\n");
	ot("	if ((INT32) sh2->r[n] >= 0)\n");
	ot("		ans = 0;\n");
	ot("	else\n");
	ot("		ans = 1;\n");
	ot("	ans += dest;\n");
	ot("	if (src == 0 || src == 2)\n");
	ot("	{\n");
	ot("		if (ans == 1)\n");
	ot("			sh2->sr |= T;\n");
	ot("		else\n");
	ot("			sh2->sr &= ~T;\n");
	ot("	}\n");
	ot("	else\n");
	ot("		sh2->sr &= ~T;\n");
} 
  

void OpSHLL(unsigned short n)
{
	ot("	sh2->sr = (sh2->sr & ~T) | ((sh2->r[%d] >> 31) & T);\n", (n >> 8) & 0x0f);
	ot("	sh2->r[%d] <<= 1;\n", (n >> 8) & 0x0f);
}
		
void OpSHLR(unsigned short n)
{
	ot("	sh2->sr = (sh2->sr & ~T) | (sh2->r[%d] & T);\n", (n >> 8) & 0x0f);
	ot("	sh2->r[%d] >>= 1;\n", (n >> 8) & 0x0f);
}
		
void OpSTSMMACH(unsigned short n)
{
	ot("	sh2->r[%d] -= 4;\n", (n >> 8) & 0x0f);
	ot("	sh2->ea = sh2->r[%d];\n", (n >> 8) & 0x0f);
	ot("	WL( sh2->ea, sh2->mach );\n");
}

void OpSTCMSR(unsigned short n)
{
	ot("	sh2->r[%d] -= 4;\n", (n >> 8) & 0x0f);
	ot("	sh2->ea = sh2->r[%d];\n", (n >> 8) & 0x0f);
	ot("	WL( sh2->ea, sh2->sr );\n");
	ot("	sh2->sh2_icount--;\n");
}
	
void OpROTL(unsigned short n)
{
	ot("	sh2->sr = (sh2->sr & ~T) | ((sh2->r[%d] >> 31) & T);\n", (n >> 8) & 0x0f);
	ot("	sh2->r[%d] = (sh2->r[%d] << 1) | (sh2->r[%d] >> 31);\n", (n >> 8) & 0x0f, (n >> 8) & 0x0f, (n >> 8) & 0x0f);
}
		
void OpROTR(unsigned short n)
{
	ot("	sh2->sr = (sh2->sr & ~T) | (sh2->r[%d] & T);\n", (n >> 8) & 0x0f);
	ot("	sh2->r[%d] = (sh2->r[%d] >> 1) | (sh2->r[%d] << 31);\n", (n >> 8) & 0x0f, (n >> 8) & 0x0f, (n >> 8) & 0x0f);
}
		
void OpLDSMMACH(unsigned short n)
{
	ot("	sh2->ea = sh2->r[%d];\n", (n >> 8) & 0x0f);
	ot("	sh2->mach = RL( sh2->ea );\n");
	ot("	sh2->r[%d] += 4;\n", (n >> 8) & 0x0f);
}
	
void OpLDCMSR(unsigned short n)
{
	ot("	sh2->ea = sh2->r[%d];\n", (n >> 8) & 0x0f);
	ot("	sh2->sr = RL( sh2->ea ) & FLAGS;\n");
	ot("	sh2->r[%d] += 4;\n", (n >> 8) & 0x0f);
	ot("	sh2->sh2_icount -= 2;\n");
	ot("	sh2->test_irq = 1;\n");
}
	
void OpSHLL2(unsigned short n)
{
	ot("	sh2->r[%d] <<= 2;\n", (n >> 8) & 0x0f);
}
	
void OpSHLR2(unsigned short n)
{
	ot("	sh2->r[%d] >>= 2;\n", (n >> 8) & 0x0f);
}
	
void OpLDSMACH(unsigned short n)
{
	ot("	sh2->mach = sh2->r[%d];\n", (n >> 8) & 0x0f);
}

void OpJSR(unsigned short n)
{
	ot("	sh2->delay = sh2->pc;\n");
	ot("	sh2->pr = sh2->pc + 2;\n");
	ot("	sh2->pc = sh2->ea = sh2->r[%d];\n", (n >> 8) & 0x0f);
	ot("	sh2->sh2_icount--;\n");
}

void OpLDCSR(unsigned short n)
{
	ot("	sh2->sr = sh2->r[%d] & FLAGS;\n", (n >> 8) & 0x0f);
	ot("	sh2->test_irq = 1;\n");
}

void OpMAC_W(unsigned short n)
{
	ot("	unsigned int n = (opcode>>8) & 0x0f;\n");
	ot("	unsigned int m = (opcode>>4) & 0x0f;\n");
	ot("\n");
	ot("	INT32 tempm, tempn, dest, src, ans;\n");
	ot("	UINT32 templ;\n");
	ot("\n");
	ot("	tempn = (INT32) RW( sh2->r[n] );\n");
	ot("	sh2->r[n] += 2;\n");
	ot("	tempm = (INT32) RW( sh2->r[m] );\n");
	ot("	sh2->r[m] += 2;\n");
	ot("	templ = sh2->macl;\n");
	ot("	tempm = ((INT32) (short) tempn * (INT32) (short) tempm);\n");
	ot("	if ((INT32) sh2->macl >= 0)\n");
	ot("		dest = 0;\n");
	ot("	else\n");
	ot("		dest = 1;\n");
	ot("	if ((INT32) tempm >= 0)\n");
	ot("	{\n");
	ot("		src = 0;\n");
	ot("		tempn = 0;\n");
	ot("	}\n");
	ot("	else\n");
	ot("	{\n");
	ot("		src = 1;\n");
	ot("		tempn = 0xffffffff;\n");
	ot("	}\n");
	ot("	src += dest;\n");
	ot("	sh2->macl += tempm;\n");
	ot("	if ((INT32) sh2->macl >= 0)\n");
	ot("		ans = 0;\n");
	ot("	else\n");
	ot("		ans = 1;\n");
	ot("	ans += dest;\n");
	ot("	if (sh2->sr & S)\n");
	ot("	{\n");
	ot("		if (ans == 1)\n");
	ot("			{\n");
	ot("				if (src == 0)\n");
	ot("					sh2->macl = 0x7fffffff;\n");
	ot("				if (src == 2)\n");
	ot("					sh2->macl = 0x80000000;\n");
	ot("			}\n");
	ot("	}\n");
	ot("	else\n");
	ot("	{\n");
	ot("		sh2->mach += tempn;\n");
	ot("		if (templ > sh2->macl)\n");
	ot("			sh2->mach += 1;\n");
	ot("		}\n");
	ot("	sh2->sh2_icount -= 2;\n");
}

void OpDT(unsigned short n)
{
	ot("	unsigned int n = (opcode>>8) & 0x0f;\n");	
	ot("	sh2->r[n]--;\n");
	ot("	if (sh2->r[n] == 0)\n");
	ot("		sh2->sr |= T;\n");
	ot("	else\n");
	ot("		sh2->sr &= ~T;\n");
	ot("#if BUSY_LOOP_HACKS\n");
	ot("	{\n");
	ot("		UINT32 next_opcode = OPRW(sh2->ppc & AM);\n");
	ot("		if (next_opcode == 0x8bfd)\n");
	ot("		{\n");
	ot("			while (sh2->r[n] > 1 && sh2->sh2_icount > 4)\n");
	ot("			{\n");
	ot("				sh2->r[n]--;\n");
	ot("				sh2->sh2_icount -= 4;\n");
	ot("			}\n");
	ot("		}\n");
	ot("	}\n");
	ot("#endif\n");
}
		
void OpCMPPZ(unsigned short n)
{
	ot("	if ((INT32) sh2->r[%d] >= 0)\n", (n >> 8) & 0x0f);
	ot("		sh2->sr |= T;\n");
	ot("	else\n");
	ot("		sh2->sr &= ~T;\n");
}
	
void OpSTSMMACL(unsigned short n)
{
	ot("	sh2->r[%d] -= 4;\n", (n >> 8) & 0x0f);
	ot("	sh2->ea = sh2->r[%d];\n", (n >> 8) & 0x0f);
	ot("	WL( sh2->ea, sh2->macl );\n");
}
	
void OpSTCMGBR(unsigned short n)
{
	ot("	sh2->r[%d] -= 4;\n", (n >> 8) & 0x0f);
	ot("	sh2->ea = sh2->r[%d];\n", (n >> 8) & 0x0f);
	ot("	WL( sh2->ea, sh2->gbr );\n");
	ot("	sh2->sh2_icount--;\n");
}

void OpCMPPL(unsigned short n)
{
	ot("	if ((INT32) sh2->r[%d] > 0)\n", (n >> 8) & 0x0f);
	ot("		sh2->sr |= T;\n");
	ot("	else\n");
	ot("		sh2->sr &= ~T;\n");
}

void OpLDSMMACL(unsigned short n)
{
	ot("	sh2->ea = sh2->r[%d];\n", (n >> 8) & 0x0f);
	ot("	sh2->macl = RL( sh2->ea );\n");
	ot("	sh2->r[%d] += 4;\n", (n >> 8) & 0x0f);
}

void OpLDCMGBR(unsigned short n)
{
	ot("	sh2->ea = sh2->r[%d];\n", (n >> 8) & 0x0f);
	ot("	sh2->gbr = RL( sh2->ea );\n");
	ot("	sh2->r[%d] += 4;\n", (n >> 8) & 0x0f);
	ot("	sh2->sh2_icount -= 2;\n");
}

void OpSHLL8(unsigned short n)
{
	ot("	sh2->r[%d] <<= 8;\n", (n >> 8) & 0x0f);
}

void OpSHLR8(unsigned short n)
{
	ot("	sh2->r[%d] >>= 8;\n", (n >> 8) & 0x0f);
}

void OpLDSMACL(unsigned short n)
{
	ot("	sh2->macl = sh2->r[%d];\n", (n >> 8) & 0x0f);
}

void OpTAS(unsigned short n)
{
	ot("	unsigned int n = (opcode>>8) & 0x0f;\n");	
	ot("	UINT32 temp;\n");
	ot("	sh2->ea = sh2->r[n];\n");
	ot("	temp = RB( sh2->ea );\n");
	ot("	if (temp == 0)\n");
	ot("		sh2->sr |= T;\n");
	ot("	else\n");
	ot("		sh2->sr &= ~T;\n");
	ot("	temp |= 0x80;\n");
	ot("	WB( sh2->ea, temp );\n");
	ot("	sh2->sh2_icount -= 3;\n");
}

void OpLDCGBR(unsigned short n)
{
	ot("	sh2->gbr = sh2->r[%d];\n", (n >> 8) & 0x0f);
}


void OpSHAL(unsigned short n)
{
	ot("	sh2->sr = (sh2->sr & ~T) | ((sh2->r[%d] >> 31) & T);\n", (n >> 8) & 0x0f);
	ot("	sh2->r[%d] <<= 1;\n", (n >> 8) & 0x0f);
}
		
void OpSHAR(unsigned short n)
{
	ot("	sh2->sr = (sh2->sr & ~T) | (sh2->r[%d] & T);\n", (n >> 8) & 0x0f);
	ot("	sh2->r[%d] = (UINT32)((INT32)sh2->r[%d] >> 1);\n", (n >> 8) & 0x0f, (n >> 8) & 0x0f);
}
		
void OpSTSMPR(unsigned short n)
{
	ot("	sh2->r[%d] -= 4;\n", (n >> 8) & 0x0f);
	ot("	sh2->ea = sh2->r[%d];\n", (n >> 8) & 0x0f);
	ot("	WL( sh2->ea, sh2->pr );\n");
}

void OpSTCMVBR(unsigned short n)
{
	ot("	sh2->r[%d] -= 4;\n", (n >> 8) & 0x0f);
	ot("	sh2->ea = sh2->r[%d];\n", (n >> 8) & 0x0f);
	ot("	WL( sh2->ea, sh2->vbr );\n");
	ot("	sh2->sh2_icount--;\n");
}

void OpROTCL(unsigned short n)
{
	ot("	UINT32 temp;\n");
	ot("	temp = (sh2->r[%d] >> 31) & T;\n", (n >> 8) & 0x0f);
	ot("	sh2->r[%d] = (sh2->r[%d] << 1) | (sh2->sr & T);\n", (n >> 8) & 0x0f, (n >> 8) & 0x0f);
	ot("	sh2->sr = (sh2->sr & ~T) | temp;\n");
}
	
void OpROTCR(unsigned short n)
{
	ot("	UINT32 temp;\n");
	ot("	temp = (sh2->sr & T) << 31;\n");
	ot("	if (sh2->r[%d] & T)\n", (n >> 8) & 0x0f);
	ot("		sh2->sr |= T;\n");
	ot("	else\n");
	ot("		sh2->sr &= ~T;\n");
	ot("	sh2->r[%d] = (sh2->r[%d] >> 1) | temp;\n", (n >> 8) & 0x0f, (n >> 8) & 0x0f);
}
	
void OpLDSMPR(unsigned short n)
{
	ot("	sh2->ea = sh2->r[%d];\n", (n >> 8) & 0x0f);
	ot("	sh2->pr = RL( sh2->ea );\n");
	ot("	sh2->r[%d] += 4;\n", (n >> 8) & 0x0f);
}

void OpLDCMVBR(unsigned short n)
{
	ot("	sh2->ea = sh2->r[%d];\n", (n >> 8) & 0x0f);
	ot("	sh2->vbr = RL( sh2->ea );\n");
	ot("	sh2->r[%d] += 4;\n", (n >> 8) & 0x0f);
	ot("	sh2->sh2_icount -= 2;\n");
}

void OpSHLL16(unsigned short n)
{
	ot("	sh2->r[%d] <<= 16;\n", (n >> 8) & 0x0f);
}

void OpSHLR16(unsigned short n)
{
	ot("	sh2->r[%d] >>= 16;\n", (n >> 8) & 0x0f);
}

void OpLDSPR(unsigned short n)
{
	ot("	sh2->pr = sh2->r[%d];\n", (n >> 8) & 0x0f);
}
	
void OpJMP(unsigned short n)
{
	ot("	sh2->delay = sh2->pc;\n");
	ot("	sh2->pc = sh2->ea = sh2->r[%d];\n", (n >> 8) & 0x0f);
}
		
void OpLDCVBR(unsigned short n)
{
	ot("	sh2->vbr = sh2->r[%d];\n", (n >> 8) & 0x0f);
}

void OpMOVLL4(unsigned short n)
{
	ot("	sh2->ea = sh2->r[(opcode>>4) & 0x0f] + ((opcode<<2) & 0x3c);\n");	
	ot("	sh2->r[(opcode>>8) & 0x0f] = RL( sh2->ea );\n");
}


void OpMOVBL(unsigned short n)
{
	ot("	sh2->ea = sh2->r[%d];\n", (n >> 4) & 0x0f);
	ot("	sh2->r[%d] = (UINT32)(INT32)(INT16)(INT8) RB( sh2->ea );\n", (n >> 8) & 0x0f);
}

void OpMOVWL(unsigned short n)
{
	ot("	sh2->ea = sh2->r[%d];\n", (n >> 4) & 0x0f);
	ot("	sh2->r[%d] = (UINT32)(INT32)(INT16) RW( sh2->ea );\n", (n >> 8) & 0x0f);
}

void OpMOVLL(unsigned short n)
{
	ot("	sh2->ea = sh2->r[%d];\n", (n >> 4) & 0x0f);
	ot("	sh2->r[%d] = RL( sh2->ea );\n", (n >> 8) & 0x0f);
}

void OpMOV(unsigned short n)
{
	ot("	sh2->r[%d] = sh2->r[%d];\n", (n >> 8) & 0x0f, (n >> 4) & 0x0f);
}

void OpMOVBP(unsigned short n)
{
	ot("	sh2->r[%d] = (UINT32)(INT32)(INT16)(INT8) RB( sh2->r[%d] );\n", (n >> 8) & 0x0f, (n >> 4) & 0x0f);
	ot("	if (%d != %d)\n", (n >> 8) & 0x0f, (n >> 4) & 0x0f);
	ot("		sh2->r[%d] += 1;\n", (n >> 4) & 0x0f);
}

void OpMOVWP(unsigned short n)
{
	ot("	sh2->r[%d] = (UINT32)(INT32)(INT16) RW( sh2->r[%d] );\n", (n >> 8) & 0x0f, (n >> 4) & 0x0f);
	ot("	if (%d != %d)\n", (n >> 8) & 0x0f, (n >> 4) & 0x0f);
	ot("		sh2->r[%d] += 2;\n", (n >> 4) & 0x0f);
}

void OpMOVLP(unsigned short n)
{
	ot("	sh2->r[%d] = RL( sh2->r[%d] );\n", (n >> 8) & 0x0f, (n >> 4) & 0x0f);
	ot("	if (%d != %d)\n", (n >> 8) & 0x0f, (n >> 4) & 0x0f);
	ot("		sh2->r[%d] += 4;\n", (n >> 4) & 0x0f);
}

void OpNOT(unsigned short n)
{
	ot("	sh2->r[%d] = ~sh2->r[%d];\n", (n >> 8) & 0x0f, (n >> 4) & 0x0f);
}

void OpSWAPB(unsigned short n)
{
	ot("	UINT32 temp0, temp1;\n");
	ot("	temp0 = sh2->r[%d] & 0xffff0000;\n", (n >> 4) & 0x0f);
	ot("	temp1 = (sh2->r[%d] & 0x000000ff) << 8;\n", (n >> 4) & 0x0f);
	ot("	sh2->r[%d] = (sh2->r[%d] >> 8) & 0x000000ff;\n", (n >> 8) & 0x0f, (n >> 4) & 0x0f);
	ot("	sh2->r[%d] = sh2->r[%d] | temp1 | temp0;\n", (n >> 8) & 0x0f, (n >> 8) & 0x0f);
}

void OpSWAPW(unsigned short n)
{
	ot("	UINT32 temp;\n");
	ot("	temp = (sh2->r[%d] >> 16) & 0x0000ffff;\n", (n >> 4) & 0x0f);
	ot("	sh2->r[%d] = (sh2->r[%d] << 16) | temp;\n", (n >> 8) & 0x0f, (n >> 4) & 0x0f);
}

void OpNEGC(unsigned short n)
{
	ot("	UINT32 temp;\n");
	ot("	temp = sh2->r[%d];\n", (n >> 4) & 0x0f);
	ot("	sh2->r[%d] = -temp - (sh2->sr & T);\n", (n >> 8) & 0x0f);
	ot("	if (temp || (sh2->sr & T))\n");
	ot("		sh2->sr |= T;\n");
	ot("	else\n");
	ot("		sh2->sr &= ~T;\n");
}

void OpNEG(unsigned short n)
{
	ot("	sh2->r[%d] = 0 - sh2->r[%d];\n", (n >> 8) & 0x0f, (n >> 4) & 0x0f);
}

void OpEXTUB(unsigned short n)
{
	ot("	sh2->r[%d] = sh2->r[%d] & 0x000000ff;\n", (n >> 8) & 0x0f, (n >> 4) & 0x0f);
}

void OpEXTUW(unsigned short n)
{
	ot("	sh2->r[%d] = sh2->r[%d] & 0x0000ffff;\n", (n >> 8) & 0x0f, (n >> 4) & 0x0f);
}

void OpEXTSB(unsigned short n)
{
	ot("	sh2->r[%d] = ((INT32)sh2->r[%d] << 24) >> 24;\n", (n >> 8) & 0x0f, (n >> 4) & 0x0f);
}

void OpEXTSW(unsigned short n)
{
	ot("	sh2->r[%d] = ((INT32)sh2->r[%d] << 16) >> 16;\n", (n >> 8) & 0x0f, (n >> 4) & 0x0f);
}

void OpADDI(unsigned short n)
{
	ot("	sh2->r[%d] += (INT32)(INT16)(INT8)(opcode & 0xff);\n", (n >> 8) & 0x0f);
}

void OpMOVBS4(unsigned short n)
{
	ot("	sh2->ea = sh2->r[%d] + %d;\n", (n >> 4) & 0x0f, (n >> 0) & 0x0f);
	ot("	WB( sh2->ea, sh2->r[0] & 0x000000ff );\n");
}

void OpMOVWS4(unsigned short n)
{
	ot("	sh2->ea = sh2->r[%d] + %d * 2;\n", (n >> 4) & 0x0f, (n >> 0) & 0x0f);
	ot("	WW( sh2->ea, sh2->r[0] & 0x0000ffff );\n");
}

void OpMOVBL4(unsigned short n)
{
	ot("	sh2->ea = sh2->r[%d] + %d;\n", (n >> 4) & 0x0f, (n >> 0) & 0x0f);
	ot("	sh2->r[0] = (UINT32)(INT32)(INT16)(INT8) RB( sh2->ea );\n");
}

void OpMOVWL4(unsigned short n)
{
	ot("	sh2->ea = sh2->r[%d] + %d * 2;\n", (n >> 4) & 0x0f, (n >> 0) & 0x0f);
	ot("	sh2->r[0] = (UINT32)(INT32)(INT16) RW( sh2->ea );\n");
}

void OpCMPIM(unsigned short n)
{
	ot("	if (sh2->r[0] == 0x%08x)\n", (unsigned int)(signed int)(signed short)(signed char)(n & 0xff) );
	ot("		sh2->sr |= T;\n");
	ot("	else\n");
	ot("		sh2->sr &= ~T;\n");
}

void OpBT(unsigned short n)
{
	ot("	if ((sh2->sr & T) != 0) {\n");
//	ot("		INT32 disp = ((INT32)d << 24) >> 24;\n");
	ot("		sh2->pc = sh2->ea = sh2->pc + (%d * 2) + 2;\n", (signed char)(n & 0xff));
	ot("		change_pc(sh2->pc & AM);\n");
	ot("		sh2->sh2_icount -= 2;\n");
	ot("	}\n");
}

void OpBF(unsigned short n)
{
	ot("	if ((sh2->sr & T) == 0) {\n");
//	ot("		INT32 disp = ((INT32)d << 24) >> 24;\n");
	ot("		sh2->pc = sh2->ea = sh2->pc + (%d * 2) + 2;\n", (signed char)(n & 0xff));
	ot("		change_pc(sh2->pc & AM);\n");
	ot("		sh2->sh2_icount -= 2;\n");
	ot("	}\n");
}

void OpBTS(unsigned short n)
{
	ot("	if ((sh2->sr & T) != 0) {\n");
	ot("		sh2->delay = sh2->pc;\n");
//	ot("		INT32 disp = ((INT32)d << 24) >> 24;\n");
	ot("		sh2->pc = sh2->ea = sh2->pc + (%d * 2) + 2;\n", (signed char)(n & 0xff));
	ot("		sh2->sh2_icount --;\n");
	ot("	}\n");
}

void OpBFS(unsigned short n)
{
	ot("	if ((sh2->sr & T) == 0) {\n");
	ot("		sh2->delay = sh2->pc;\n");
//	ot("		INT32 disp = ((INT32)d << 24) >> 24;\n");
	ot("		sh2->pc = sh2->ea = sh2->pc + (%d * 2) + 2;\n", (signed char)(n & 0xff));
	ot("		sh2->sh2_icount --;\n");
	ot("	}\n");
}

void OpMOVWI(unsigned short n)
{
	ot("	sh2->ea = sh2->pc + (opcode & 0xff) * 2 + 2;\n");
	ot("	sh2->r[%d] = (UINT32)(INT32)(INT16) RW( sh2->ea );\n", (n >> 8) & 0x0f);
}

void OpBRA(unsigned short n)
{
	ot("	INT32 disp = ((INT32)opcode << 20) >> 20;\n");
	ot("#if BUSY_LOOP_HACKS\n");
	ot("	if (disp == -2) {\n");
	ot("		UINT32 next_opcode = OPRW(sh2->ppc & AM);\n");
	ot("		if (next_opcode == 0x0009)\n");
	ot("			sh2->sh2_icount %= 3;\n");
	ot("	}\n");
	ot("#endif\n");
	ot("	sh2->delay = sh2->pc;\n");
	ot("	sh2->pc = sh2->ea = sh2->pc + disp * 2 + 2;\n");
	ot("	sh2->sh2_icount--;\n");
}

void OpBSR(unsigned short n)
{
	ot("	INT32 disp = ((INT32)opcode << 20) >> 20;\n");
	ot("	sh2->pr = sh2->pc + 2;\n");
	ot("	sh2->delay = sh2->pc;\n");
	ot("	sh2->pc = sh2->ea = sh2->pc + disp * 2 + 2;\n");
	ot("	sh2->sh2_icount--;\n");
}


void OpMOVBSG(unsigned short n)
{
	ot("	sh2->ea = sh2->gbr + %d;\n", n & 0xff);
	ot("	WB( sh2->ea, sh2->r[0] & 0x000000ff );\n");
}

void OpMOVWSG(unsigned short n)
{
	ot("	sh2->ea = sh2->gbr + %d * 2;\n", n & 0xff);
	ot("	WW( sh2->ea, sh2->r[0] & 0x0000ffff );\n");
}

void OpMOVLSG(unsigned short n)
{
	ot("	sh2->ea = sh2->gbr + %d * 4;\n", n & 0xff);
	ot("	WL( sh2->ea, sh2->r[0] );\n");
}

void OpTRAPA(unsigned short n)
{
	ot("	sh2->ea = sh2->vbr + (opcode & 0xff) * 4;\n");
	ot("	sh2->r[15] -= 4;\n");
	ot("	WL( sh2->r[15], sh2->sr );\n");
	ot("	sh2->r[15] -= 4;\n");
	ot("	WL( sh2->r[15], sh2->pc );\n");
	ot("	sh2->pc = RL( sh2->ea );\n");
	ot("	change_pc(sh2->pc & AM);\n");
	ot("	sh2->sh2_icount -= 7;\n");
}

void OpMOVBLG(unsigned short n)
{
	ot("	sh2->ea = sh2->gbr + %d;\n", n & 0xff);
	ot("	sh2->r[0] = (UINT32)(INT32)(INT16)(INT8) RB( sh2->ea );\n");
}

void OpMOVWLG(unsigned short n)
{
	ot("	sh2->ea = sh2->gbr + %d * 2;\n", n & 0xff);
	ot("	sh2->r[0] = (INT32)(INT16) RW( sh2->ea );\n");
}

void OpMOVLLG(unsigned short n)
{
	ot("	sh2->ea = sh2->gbr + %d * 4;\n", n & 0xff);
	ot("	sh2->r[0] = RL( sh2->ea );\n");
}

void OpMOVA(unsigned short n)
{
	ot("	sh2->ea = ((sh2->pc + 2) & ~3) + %d * 4;\n", n & 0xff);
	ot("	sh2->r[0] = sh2->ea;\n");
}
	
void OpTSTI(unsigned short n)
{
	ot("	if (((%d) & sh2->r[0]) == 0)\n", n & 0xff);
	ot("		sh2->sr |= T;\n");
	ot("	else\n");
	ot("		sh2->sr &= ~T;\n");
}
	
void OpANDI(unsigned short n)
{
	ot("	sh2->r[0] &= %d;\n", n & 0xff);
}
	
void OpXORI(unsigned short n)
{
	ot("	sh2->r[0] ^= %d;\n", n & 0xff);
}
	
void OpORI(unsigned short n)
{
	ot("	sh2->r[0] |= %d;\n", n & 0xff);
	ot("	sh2->sh2_icount -= 2;\n");
}
	
void OpTSTM(unsigned short n)
{
	ot("	sh2->ea = sh2->gbr + sh2->r[0];\n");
	ot("	if ((%d & RB( sh2->ea )) == 0)\n", n & 0xff);
	ot("		sh2->sr |= T;\n");
	ot("	else\n");
	ot("		sh2->sr &= ~T;\n");
	ot("	sh2->sh2_icount -= 2;\n");
}
	
void OpANDM(unsigned short n)
{
	ot("	sh2->ea = sh2->gbr + sh2->r[0];\n");
	ot("	UINT32 temp = %d & RB( sh2->ea );\n", n & 0xff);
	ot("	WB( sh2->ea, temp );\n");
	ot("	sh2->sh2_icount -= 2;\n");
}
	
void OpXORM(unsigned short n)
{
	ot("	UINT32 imm = %d & 0xff;\n", n & 0xff);
	ot("	sh2->ea = sh2->gbr + sh2->r[0];\n");
	ot("	UINT32 temp = RB( sh2->ea );\n");
	ot("	temp ^= imm;\n");
	ot("	WB( sh2->ea, temp );\n");
	ot("	sh2->sh2_icount -= 2;\n");
}
	
void OpORM(unsigned short n)
{
	ot("	sh2->ea = sh2->gbr + sh2->r[0];\n");
	ot("	UINT32 temp = RB( sh2->ea );\n");
	ot("	temp |= %d;\n", n & 0xff);
	ot("	WB( sh2->ea, temp );\n");
}
	

void OpMOVLI(unsigned short n)
{
	ot("	UINT32 disp = opcode & 0xff;\n");
	ot("	sh2->ea = ((sh2->pc + 2) & ~3) + disp * 4;\n");
	ot("	sh2->r[%d] = RL( sh2->ea );\n", (n >> 8) & 0x0f);
}

void OpMOVI(unsigned short n)
{
	ot("	sh2->r[%d] = (UINT32)(INT32)(INT16)(INT8)( opcode & 0xff );\n", (n >> 8) & 0x0f);
}




struct OpCodeDef {
	char name[16];
	int value;
	int mask;
	
	int needed;
	
	int min;
	void (*OpGenerator)(unsigned short i);
	char ref[32];
	
} opcodes[] = {

/*	Name		Value	Mask	0x????  Min Generator
	----------- ------- ------- ------- --- --------- */	

	// 0x0???
	//{"NOP",	0x0000, 0xf03f, 0x0000, -1, Op____, 	""},
	//{"NOP",	0x0001, 0xf03f, 0x0000, -1, Op____,		""},
	{"STCSR",	0x0002, 0xf03f, 0x0f00, -1, OpSTCSR,	"STC SR,Rn"},
	{"BSRF",	0x0003, 0xf03f, 0x0f00, -1, OpBSRF,		"BSRF Rn"},
	{"MOVBS0",	0x0004, 0xf00f, 0x0ff0, -1, OpMOVBS0,	"MOV.B Rm,@(R0,Rn)"},
	{"MOVWS0",	0x0005, 0xf00f, 0x0ff0, -1, OpMOVWS0,	"MOV.W Rm,@(R0,Rn)"},
	{"MOVLS0",	0x0006, 0xf00f, 0x0ff0, -1, OpMOVLS0,	"MOV.L Rm,@(R0,Rn)"},
	{"MULL",	0x0007, 0xf00f, 0x0ff0, -1, OpMULL,		"MUL.L Rm,Rn"},
	{"CLRT",	0x0008, 0xf03f, 0x0000, -1, OpCLRT,		"CLRT"},
	//{"NOP",	0x0009, 0xf03f, 0x0000, -1, Op____,		""},
	{"STSMACH",	0x000a, 0xf03f, 0x0f00, -1, OpSTSMACH,	"STS MACH,Rn"},
	{"RTS",		0x000b, 0xf03f, 0x0000, -1, OpRTS,		"RTS"},
	{"MOVBL0",	0x000c, 0xf00f, 0x0ff0, -1, OpMOVBL0,	"MOV.B @(R0,Rm),Rn"},
	{"MOVWL0",	0x000d, 0xf00f, 0x0ff0, -1, OpMOVWL0,	"MOV.W @(R0,Rm),Rn"},
	{"MOVLL0",	0x000e, 0xf00f, 0x0ff0, -1, OpMOVLL0,	"MOV.L @(R0,Rm),Rn"},
	{"MAC_L",	0x000f, 0xf00f, 0x0000, -1, OpMAC_L,	"MAC.L @Rm+,@Rn+"},
                                                   		
	//{"NOP",	0x0010, 0xf03f, 0x0000, -1, Op____,		""},
	//{"NOP",	0x0011, 0xf03f, 0x0000, -1, Op____,		""},
	{"STCGBR",	0x0012, 0xf03f, 0x0f00, -1, OpSTCGBR,	"STC GBR,Rn"},
	//{"NOP",	0x0013, 0xf03f, 0x0000, -1, Op____,		""},
	//{"MOVBS0",	0x0014, 0xf03f, 0x0ff0, -1, Op____, ""},
	//{"MOVWS0",	0x0015, 0xf03f, 0x0ff0, -1, Op____, ""},
	//{"MOVLS0",	0x0016, 0xf03f, 0x0ff0, -1, Op____, ""},
	//{"MULL",	0x0017, 0xf03f, 0x0ff0, -1, Op____,		""},
	{"SETT",	0x0018, 0xf03f, 0x0000, -1, OpSETT,		"SETT"},
	{"DIV0U",	0x0019, 0xf03f, 0x0000, -1, OpDIV0U,	"DIV0U"},
	{"STSMACL",	0x001a, 0xf03f, 0x0f00, -1, OpSTSMACL,	"STS MACL,Rn"},
	{"SLEEP",	0x001b, 0xf03f, 0x0000, -1, OpSLEEP, 	"SLEEP"},
	//{"MOVBL0",	0x001c, 0xf03f, 0x0ff0, -1, Op____, ""},
	//{"MOVWL0",	0x001d, 0xf03f, 0x0ff0, -1, Op____, ""},
	//{"MOVLL0",	0x001e, 0xf03f, 0x0ff0, -1, Op____, ""},
	//{"MAC_L",	0x001f, 0xf03f, 0x0ff0, -1, Op____, 	""},

	//{"NOP",	0x0020, 0xf03f, 0x0000, -1, Op____, 	""},
	//{"NOP",	0x0021, 0xf03f, 0x0000, -1, Op____, 	""},
	{"STCVBR",	0x0022, 0xf03f, 0x0f00, -1, OpSTCVBR,	"STC VBR,Rn"},
	{"BRAF",	0x0023, 0xf03f, 0x0f00, -1, OpBRAF, 	"BRAF Rn"},
	//{"MOVBS0",	0x0024, 0xf03f, 0x0000, -1, Op____, ""},
	//{"MOVWS0",	0x0025, 0xf03f, 0x0000, -1, Op____, ""},
	//{"MOVLS0",	0x0026, 0xf03f, 0x0000, -1, Op____, ""},
	//{"MULL",	0x0027, 0xf03f, 0x0000, -1, Op____,		""},
	{"CLRMAC",	0x0028, 0xf03f, 0x0000, -1, OpCLRMAC,	"CLRMAC"},
	{"MOVT",	0x0029, 0xf03f, 0x0f00, -1, OpMOVT,		"MOVT Rn"},
	{"STSPR",	0x002a, 0xf03f, 0x0f00, -1, OpSTSPR,	"STS PR,Rn"},
	{"RTE",		0x002b, 0xf03f, 0x0000, -1, OpRTE,		"RTE"},
	//{"MOVBL0",	0x002c, 0xf03f, 0x0000, -1, Op____, ""},
	//{"MOVWL0",	0x002d, 0xf03f, 0x0000, -1, Op____, ""},
	//{"MOVLL0",	0x002e, 0xf03f, 0x0000, -1, Op____, ""},
	//{"MAC_L",	0x002f, 0xf03f, 0x0000, -1, Op____,		""},
	
	//{"NOP",	0x0030, 0xf03f, 0x0000, -1, Op____,		""},
	//{"NOP",	0x0031, 0xf03f, 0x0000, -1, Op____,		""},
	//{"NOP",	0x0032, 0xf03f, 0x0000, -1, Op____,		""},
	//{"NOP",	0x0033, 0xf03f, 0x0000, -1, Op____,		""},
	//{"MOVBS0",	0x0034, 0xf03f, 0x0000, -1, Op____, ""},
	//{"MOVWS0",	0x0035, 0xf03f, 0x0000, -1, Op____, ""},
	//{"MOVLS0",	0x0036, 0xf03f, 0x0000, -1, Op____, ""},
	//{"MULL",	0x0037, 0xf03f, 0x0000, -1, Op____,		""},
	//{"NOP",	0x0038, 0xf03f, 0x0000, -1, Op____,		""},
	//{"NOP",	0x0039, 0xf03f, 0x0000, -1, Op____,		""},
	//{"NOP",	0x003a, 0xf03f, 0x0000, -1, Op____,		""},
	//{"NOP",	0x003b, 0xf03f, 0x0000, -1, Op____,		""},
	//{"MOVBL0",	0x003c, 0xf03f, 0x0000, -1, Op____, ""},
	//{"MOVWL0",	0x003d, 0xf03f, 0x0000, -1, Op____, ""},
	//{"MOVLL0",	0x003e, 0xf03f, 0x0000, -1, Op____, ""},
	//{"MAC_L",	0x003f, 0xf03f, 0x0000, -1, Op____,		""},
	
	// 0x1???
	{"MOVLS4",	0x1000, 0xf000, 0x0000, -1, OpMOVLS4,	"MOV.L Rm,@(disp4,Rn)"},	// 
	
	// 0x2???
	{"MOVBS",	0x2000, 0xf00f, 0x0ff0, -1, OpMOVBS,	"MOV.B Rm,@Rn"},
	{"MOVWS",	0x2001, 0xf00f, 0x0ff0, -1, OpMOVWS,	"MOV.W Rm,@Rn"},
	{"MOVLS",	0x2002, 0xf00f, 0x0ff0, -1, OpMOVLS,	"MOV.L Rm,@Rn"},
	//{"NOP",	0x2003, 0xf00f, 0x0000, -1, Op____,		""},
	{"MOVBM",	0x2004, 0xf00f, 0x0ff0, -1, OpMOVBM,	"MOV.B Rm,@-Rn"},
	{"MOVWM",	0x2005, 0xf00f, 0x0ff0, -1, OpMOVWM,	"MOV.W Rm,@-Rn"},
	{"MOVLM",	0x2006, 0xf00f, 0x0ff0, -1, OpMOVLM,	"MOV.L Rm,@-Rn"},
	{"DIV0S",	0x2007, 0xf00f, 0x0ff0, -1, OpDIV0S,	"DIV0S Rm,Rn"},
	{"TST",		0x2008, 0xf00f, 0x0ff0, -1, OpTST,		"TST Rm,Rn"},
	{"AND",		0x2009, 0xf00f, 0x0ff0, -1, OpAND,		"AND Rm,Rn"},
	{"XOR",		0x200a, 0xf00f, 0x0ff0, -1, OpXOR,		"XOR Rm,Rn"},
	{"OR",		0x200b, 0xf00f, 0x0ff0, -1, OpOR,		"OR Rm,Rn"},
	{"CMPSTR",	0x200c, 0xf00f, 0x0000, -1, OpCMPSTR,	"CMP_STR Rm,Rn"},	// ????
	{"XTRCT",	0x200d, 0xf00f, 0x0000, -1, OpXTRCT,	"XTRCT Rm,Rn"},
	{"MULU",	0x200e, 0xf00f, 0x0ff0, -1, OpMULU,		"MULU Rm,Rn"},
	{"MULS",	0x200f, 0xf00f, 0x0ff0, -1, OpMULS,		"MULS Rm,Rn"},
	
	// 0x3???
	{"CMPEQ",	0x3000, 0xf00f, 0x0ff0, -1, OpCMPEQ,	"CMP_EQ Rm,Rn"},
	//{"NOP",	0x3001, 0xf00f, 0x0000, -1, Op____,		""},
	{"CMPHS",	0x3002, 0xf00f, 0x0ff0, -1, OpCMPHS,	"CMP_HS Rm,Rn"},
	{"CMPGE",	0x3003, 0xf00f, 0x0ff0, -1, OpCMPGE,	"CMP_GE Rm,Rn"},
	{"DIV1",	0x3004, 0xf00f, 0x0000, -1, OpDIV1,		"DIV1 Rm,Rn"},
	{"DMULU",	0x3005, 0xf00f, 0x0000, -1, OpDMULU,	"DMULU.L Rm,Rn"},
	{"CMPHI",	0x3006, 0xf00f, 0x0ff0, -1, OpCMPHI,	"CMP_HI Rm,Rn"},
	{"CMPGT",	0x3007, 0xf00f, 0x0ff0, -1, OpCMPGT,	"CMP_GT Rm,Rn"},
	{"SUB",		0x3008, 0xf00f, 0x0ff0, -1, OpSUB,		"SUB Rm,Rn"},
	//{"NOP",	0x3009, 0xf00f, 0x0000, -1, Op____,		""},
	{"SUBC",	0x300a, 0xf00f, 0x0000, -1, OpSUBC,		"SUBC Rm,Rn"},
	{"SUBV",	0x300b, 0xf00f, 0x0000, -1, OpSUBV,		"SUBV Rm,Rn"},
	{"ADD",		0x300c, 0xf00f, 0x0ff0, -1, OpADD,		"ADD Rm,Rn"},
	{"DMULS",	0x300d, 0xf00f, 0x0000, -1, OpDMULS,	"DMULS.L Rm,Rn"},
	{"ADDC",	0x300e, 0xf00f, 0x0000, -1, OpADDC,		"ADDC Rm,Rn"},
	{"ADDV",	0x300f, 0xf00f, 0x0000, -1, OpADDV,		"ADDV Rm,Rn"},
	
	// 0x4???
	{"SHLL",	0x4000, 0xf03f, 0x0f00, -1, OpSHLL,		"SHAL Rn"},
	{"SHLR",	0x4001, 0xf03f, 0x0f00, -1, OpSHLR,		"SHLR Rn"},
	{"STSMMACH",0x4002, 0xf03f, 0x0f00, -1, OpSTSMMACH,	"STS.L MACH,@-Rn"},
	{"STCMSR",	0x4003, 0xf03f, 0x0f00, -1, OpSTCMSR,	"STC.L SR,@-Rn"},
	{"ROTL",	0x4004, 0xf03f, 0x0f00, -1, OpROTL,		"ROTL Rn"},
	{"ROTR",	0x4005, 0xf03f, 0x0f00, -1, OpROTR,		"ROTR Rn"},
	{"LDSMMACH",0x4006, 0xf03f, 0x0f00, -1, OpLDSMMACH,	"LDS.L @Rn+,MACH"},
	{"LDCMSR",	0x4007, 0xf03f, 0x0f00, -1, OpLDCMSR,	"LDC.L @Rn+,SR"},
	{"SHLL2",	0x4008, 0xf03f, 0x0f00, -1, OpSHLL2,	"SHLL2 Rn"},
	{"SHLR2",	0x4009, 0xf03f, 0x0f00, -1, OpSHLR2,	"SHLR2 Rn"},
	{"LDSMACH",	0x400a, 0xf03f, 0x0f00, -1, OpLDSMACH,	"LDS Rn,MACH"},
	{"JSR",		0x400b, 0xf03f, 0x0f00, -1, OpJSR,		"JSR @Rn"},
	//{"NOP",	0x400c, 0xf03f, 0x0000, -1, Op____,		""},
	//{"NOP",	0x400d, 0xf03f, 0x0000, -1, Op____,		""},
	{"LDCSR",	0x400e, 0xf03f, 0x0f00, -1, OpLDCSR,	"LDC Rn,SR"},
	{"MAC_W",	0x400f, 0xf00f, 0x0000, -1, OpMAC_W,	"MAC.W @Rm+,@Rn+"},

	{"DT",		0x4010, 0xf03f, 0x0000, -1, OpDT,		"DT Rn"},
	{"CMPPZ",	0x4011, 0xf03f, 0x0f00, -1, OpCMPPZ,	"CMP_PZ Rn"},
	{"STSMMACL",0x4012, 0xf03f, 0x0f00, -1, OpSTSMMACL,	"STS.L MACL,@-Rn"},
	{"STCMGBR",	0x4013, 0xf03f, 0x0f00, -1, OpSTCMGBR,	"STC.L GBR,@-Rn"},
	//{"NOP",	0x4014, 0xf03f, 0x0000, -1, Op____,		""},
	{"CMPPL",	0x4015, 0xf03f, 0x0f00, -1, OpCMPPL,	"CMP_PL Rn"},
	{"LDSMMACL",0x4016, 0xf03f, 0x0f00, -1, OpLDSMMACL,	"LDS.L @Rn+,MACL"},
	{"LDCMGBR",	0x4017, 0xf03f, 0x0f00, -1, OpLDCMGBR,	"LDC.L @Rm+,GBR"},
	{"SHLL8",	0x4018, 0xf03f, 0x0f00, -1, OpSHLL8,	"SHLL8 Rn"},
	{"SHLR8",	0x4019, 0xf03f, 0x0f00, -1, OpSHLR8,	"SHLR8 Rn"},
	{"LDSMACL",	0x401a, 0xf03f, 0x0f00, -1, OpLDSMACL,	"LDS Rn,MACL"},
	{"TAS",		0x401b, 0xf03f, 0x0000, -1, OpTAS,		"TAS.B @Rn"},
	//{"NOP",	0x401c, 0xf03f, 0x0000, -1, Op____,		""},
	//{"NOP",	0x401d, 0xf03f, 0x0000, -1, Op____,		""},
	{"LDCGBR",	0x401e, 0xf03f, 0x0f00, -1, OpLDCGBR,	"LDC Rn,GBR"},
	//{"MAC_W",	0x401f, 0xf03f, 0x0000, -1, Op____,		""},

	{"SHAL",	0x4020, 0xf03f, 0x0f00, -1, OpSHAL,		"SHAL Rn"},
	{"SHAR",	0x4021, 0xf03f, 0x0f00, -1, OpSHAR,		"SHAR Rn"},
	{"STSMPR",	0x4022, 0xf03f, 0x0f00, -1, OpSTSMPR,	"STS.L PR,@-Rn"},
	{"STCMVBR",	0x4023, 0xf03f, 0x0f00, -1, OpSTCMVBR,	"STC.L VBR,@-Rn"},
	{"ROTCL",	0x4024, 0xf03f, 0x0f00, -1, OpROTCL,	"ROTCL Rn"},
	{"ROTCR",	0x4025, 0xf03f, 0x0f00, -1, OpROTCR,	"ROTCR Rn"},
	{"LDSMPR",	0x4026, 0xf03f, 0x0f00, -1, OpLDSMPR,	"LDS.L @Rn+,PR"},
	{"LDCMVBR",	0x4027, 0xf03f, 0x0f00, -1, OpLDCMVBR,	"LDC.L @Rn+,VBR"},
	{"SHLL16",	0x4028, 0xf03f, 0x0f00, -1, OpSHLL16,	"SHLL16 Rn"},
	{"SHLR16",	0x4029, 0xf03f, 0x0f00, -1, OpSHLR16,	"SHLR16 Rn"},
	{"LDSPR",	0x402a, 0xf03f, 0x0f00, -1, OpLDSPR,	"LDS Rn,PR"},
	{"JMP",		0x402b, 0xf03f, 0x0f00, -1, OpJMP,		"JMP @Rn"},
	//{"NOP",	0x402c, 0xf03f, 0x0000, -1, Op____,		""},
	//{"NOP",	0x402d, 0xf03f, 0x0000, -1, Op____,		""},
	{"LDCVBR",	0x402e, 0xf03f, 0x0f00, -1, OpLDCVBR,	"LDC Rn,VBR"},
	//{"MAC_W",	0x402f, 0xf03f, 0x0000, -1, Op____,		""},

	//{"NOP",	0x4030, 0xf03f, 0x0000, -1, Op____,		""},
	//{"NOP",	0x4031, 0xf03f, 0x0000, -1, Op____,		""},
	//{"NOP",	0x4032, 0xf03f, 0x0000, -1, Op____,		""},
	//{"NOP",	0x4033, 0xf03f, 0x0000, -1, Op____,		""},
	//{"NOP",	0x4034, 0xf03f, 0x0000, -1, Op____,		""},
	//{"NOP",	0x4035, 0xf03f, 0x0000, -1, Op____,		""},
	//{"NOP",	0x4036, 0xf03f, 0x0000, -1, Op____,		""},
	//{"NOP",	0x4037, 0xf03f, 0x0000, -1, Op____,		""},
	//{"NOP",	0x4038, 0xf03f, 0x0000, -1, Op____,		""},
	//{"NOP",	0x4039, 0xf03f, 0x0000, -1, Op____,		""},
	//{"NOP",	0x403a, 0xf03f, 0x0000, -1, Op____,		""},
	//{"NOP",	0x403b, 0xf03f, 0x0000, -1, Op____,		""},
	//{"NOP",	0x403c, 0xf03f, 0x0000, -1, Op____,		""},
	//{"NOP",	0x403d, 0xf03f, 0x0000, -1, Op____,		""},
	//{"NOP",	0x403e, 0xf03f, 0x0000, -1, Op____,		""},
	//{"MAC_W",	0x403f, 0xf03f, 0x0000, -1, Op____,		""},

	// 0x5???
	{"MOVLL4",	0x5000, 0xf000, 0x0000, -1, OpMOVLL4,	"MOV.L @(disp4,Rm),Rn"},

	// 0x6???
	{"MOVBL",	0x6000, 0xf00f, 0x0ff0, -1, OpMOVBL,	"MOV.B @Rm,Rn"},
	{"MOVWL",	0x6001, 0xf00f, 0x0ff0, -1, OpMOVWL,	"MOV.W @Rm,Rn"},
	{"MOVLL",	0x6002, 0xf00f, 0x0ff0, -1, OpMOVLL,	"MOV.L @Rm,Rn"},
	{"MOV",		0x6003, 0xf00f, 0x0ff0, -1, OpMOV,		"MOV Rm,Rn"},
	{"MOVBP",	0x6004, 0xf00f, 0x0ff0, -1, OpMOVBP,	"MOV.B @Rm+,Rn"},
	{"MOVWP",	0x6005, 0xf00f, 0x0ff0, -1, OpMOVWP,	"MOV.W @Rm+,Rn"},
	{"MOVLP",	0x6006, 0xf00f, 0x0ff0, -1, OpMOVLP,	"MOV.L @Rm+,Rn"},
	{"NOT",		0x6007, 0xf00f, 0x0ff0, -1, OpNOT,		"NOT Rm,Rn"},
	{"SWAPB",	0x6008, 0xf00f, 0x0ff0, -1, OpSWAPB,	"SWAP.B Rm,Rn"},
	{"SWAPW",	0x6009, 0xf00f, 0x0ff0, -1, OpSWAPW,	"SWAP.W Rm,Rn"},
	{"NEGC",	0x600a, 0xf00f, 0x0ff0, -1, OpNEGC,		"NEGC Rm,Rn"},
	{"NEG",		0x600b, 0xf00f, 0x0ff0, -1, OpNEG,		"NEG Rm,Rn"},
	{"EXTUB",	0x600c, 0xf00f, 0x0ff0, -1, OpEXTUB,	"EXTU.B Rm,Rn"},
	{"EXTUW",	0x600d, 0xf00f, 0x0ff0, -1, OpEXTUW,	"EXTU.W Rm,Rn"},
	{"EXTSB",	0x600e, 0xf00f, 0x0ff0, -1, OpEXTSB,	"EXTS.B Rm,Rn"},
	{"EXTSW",	0x600f, 0xf00f, 0x0ff0, -1, OpEXTSW,	"EXTS.W Rm,Rn"},
	
	// 0x7???
	{"ADDI",	0x7000, 0xf000, 0x0f00, -1, OpADDI,		"ADD #imm,Rn"},
	
	// 0x8???
	{"MOVBS4",	0x8000, 0xff00, 0x00ff, -1, OpMOVBS4,	"MOV.B R0,@(disp4,Rm)"},
	{"MOVWS4",	0x8100, 0xff00, 0x00ff, -1, OpMOVWS4,	"MOV.W R0,@(disp4,Rn)"},
	//{"NOP",	0x8200, 0xff00, 0x0000, -1, Op____,		""},
	//{"NOP",	0x8300, 0xff00, 0x0000, -1, Op____,		""},
	{"MOVBL4",	0x8400, 0xff00, 0x00ff, -1, OpMOVBL4,	"MOV.B @(disp4,Rm),R0"},
	{"MOVWL4",	0x8500, 0xff00, 0x00ff, -1, OpMOVWL4,	"MOV.W @(disp4,Rm),R0"},
	//{"NOP",	0x8600, 0xff00, 0x0000, -1, Op____,		""},
	//{"NOP",	0x8700, 0xff00, 0x0000, -1, Op____,		""},
	{"CMPIM",	0x8800, 0xff00, 0x00ff, -1, OpCMPIM,	"CMP/EQ #imm,R0"},
	{"BT",		0x8900, 0xff00, 0x00ff, -1, OpBT,		"BT disp8"},
	//{"NOP",	0x8a00, 0xff00, 0x0000, -1, Op____,		""},
	{"BF",		0x8b00, 0xff00, 0x00ff, -1, OpBF,		"BF disp8"},
	//{"NOP",	0x8c00, 0xff00, 0x0000, -1, Op____,		""},
	{"BTS",		0x8d00, 0xff00, 0x00ff, -1, OpBTS,		"BTS disp8"},
	//{"NOP",	0x8e00, 0xff00, 0x0000, -1, Op____,		""},
	{"BFS",		0x8f00, 0xff00, 0x00ff, -1, OpBFS,		"BFS disp8"},

	// 0x9???
	{"MOVWI",	0x9000, 0xf000, 0x0f00, -1, OpMOVWI,	"MOV.W @(disp8,PC),Rn"},

	// 0xa???
	{"BRA",		0xa000, 0xf000, 0x0000, -1, OpBRA,		"BRA disp12"},

	// 0xb???
	{"BSR",		0xb000, 0xf000, 0x0000, -1, OpBSR,		"BSR disp12"},

	// 0xc???
	{"MOVBSG",	0xc000, 0xff00, 0x00ff, -1, OpMOVBSG,	"MOV.B R0,@(disp8,GBR)"},
	{"MOVWSG",	0xc100, 0xff00, 0x00ff, -1, OpMOVWSG,	"MOV.W R0,@(disp8,GBR)"},
	{"MOVLSG",	0xc200, 0xff00, 0x00ff, -1, OpMOVLSG,	"MOV.L R0,@(disp8,GBR)"},
	{"TRAPA",	0xc300, 0xff00, 0x0000, -1, OpTRAPA,	"TRAPA #imm"},
	{"MOVBLG",	0xc400, 0xff00, 0x00ff, -1, OpMOVBLG,	"MOV.B @(disp8,GBR),R0"},
	{"MOVWLG",	0xc500, 0xff00, 0x00ff, -1, OpMOVWLG,	"MOV.W @(disp8,GBR),R0"},
	{"MOVLLG",	0xc600, 0xff00, 0x00ff, -1, OpMOVLLG,	"MOV.L @(disp8,GBR),R0"},
	{"MOVA",	0xc700, 0xff00, 0x00ff, -1, OpMOVA,		"MOVA @(disp8,PC),R0"},
	{"TSTI",	0xc800, 0xff00, 0x00ff, -1, OpTSTI,		"TST #imm,R0"},
	{"ANDI",	0xc900, 0xff00, 0x00ff, -1, OpANDI,		"AND #imm,R0"},
	{"XORI",	0xca00, 0xff00, 0x00ff, -1, OpXORI,		"XOR #imm,R0"},
	{"ORI",		0xcb00, 0xff00, 0x00ff, -1, OpORI,		"OR #imm,R0"},
	{"TSTM",	0xcc00, 0xff00, 0x00ff, -1, OpTSTM,		"TST.B #imm,@(R0,GBR)"},
	{"ANDM",	0xcd00, 0xff00, 0x00ff, -1, OpANDM,		"AND.B #imm,@(R0,GBR)"},
	{"XORM",	0xce00, 0xff00, 0x00ff, -1, OpXORM,		"XOR.B #imm,@(R0,GBR)"},
	{"ORM",		0xcf00, 0xff00, 0x00ff, -1, OpORM,		"OR.B #imm,@(R0,GBR)"},
	
	// 0xd???
	{"MOVLI",	0xd000, 0xf000, 0x0f00, -1, OpMOVLI,	"MOV.L @(disp8,PC),Rn"},

	// 0xe???
	{"MOVI",	0xe000, 0xf000, 0x0f00, -1, OpMOVI,		"MOV #imm,Rn"},

	// 0xf???
	//{"NOP",	0xf000, 0xf000, 0x0000, -1, Op____,		""},
};


int opcount = sizeof(opcodes) / sizeof(struct OpCodeDef);

main()
{
	printf("\nSH-2 Core Creator v0.1\n(C) 2008, OopsWare.\n\n");
	OpCodeFile = fopen("sh2op.c","wt"); if (OpCodeFile==NULL) return 1;
	printf("Making sh2.inc ...\n");
	
	int jtb[0x10000];
    
    for (int i=0; i<0x10000; i++) {
    	// ppppnnnn mmmm----
    	int j;
    	for (j=0;j<opcount;j++) {
    		if ( (opcodes[j].mask & i) == opcodes[j].value ) {
				if (opcodes[j].min == -1) {
					opcodes[j].min = i;
					jtb[i] = i;
				} else {
					jtb[i] = (opcodes[j].min & (opcodes[j].needed ^ 0xffff)) | (i & opcodes[j].needed);
					
				}
    		    break; 	
    		}
    	}
    	// not found
    	if (j==opcount ) jtb[i] = -1;	
    }
    
	ot("// --- Op Funcs ---------------------------------\n");
	
	ot("\n// NOP\n");
	ot("static void Op____(unsigned short /*opcode*/)\n");
	ot("{\n");
	ot("}\n\n");
	
	for (int i=0;i<opcount; i++)
		switch( opcodes[i].needed ) {
		case 0x0f00: // need Rn
			ot("// %s\n", opcodes[i].ref);
			for(int j=0; j<16; j++) {
				ot("static void Op%04X(unsigned short opcode)\n", opcodes[i].min | (j<<8));
				ot("{\n");
				opcodes[i].OpGenerator( j<<8 );
				ot("}\n\n");
			}
			break;
		case 0x0ff0: // need Rn and Rm
			ot("// %s\n", opcodes[i].ref);
			for(int j=0; j<256; j++) {
				ot("static void Op%04X(unsigned short /*opcode*/)\n", opcodes[i].min | (j<<4));
				ot("{\n");
				opcodes[i].OpGenerator( j<<4 );
				ot("}\n\n");
			}
			break;
		case 0x00ff: // need Rd and Rm / imm
			ot("// %s\n", opcodes[i].ref);
			for(int j=0; j<256; j++) {
				ot("static void Op%04X(unsigned short /*opcode*/)\n", opcodes[i].min | (j<<0));
				ot("{\n");
				opcodes[i].OpGenerator( j );
				ot("}\n\n");
			}
			break;			
			
			
			
		default:
			ot("// %s\n", opcodes[i].ref);
			ot("static void Op%04X(unsigned short opcode)\n", opcodes[i].min);
			ot("{\n");
			opcodes[i].OpGenerator( 0 );
			ot("}\n\n");	
		}
	
	ot("\n// --- Jump Table ---------------------------------\n\n");
	ot("void (*opcode_jumptable[0x10000])(unsigned short) = {\n");
		
	char buf[256];
	char buf2[256];
	
	for (int i=0;i<0x10000;i++) {
		if ((i & 7) == 0) {
			if (jtb[i] == -1)	sprintf(buf, "\t\tOp____");
			else				sprintf(buf, "\t\tOp%04X", jtb[i] );
		} else {
			if (jtb[i] == -1)	strcat(buf, ", Op____");
			else {				
								sprintf(buf2, ", Op%04X", jtb[i] );
								strcat(buf, buf2);	
			}
		}
		if ((i & 7) == 7)		ot("%s,\t\t// %04x\n", buf, i );
	}
	
	ot("\t};\n");
	ot("\n// --- Jump Table End -----------------------------\n\n");
	
	fclose(OpCodeFile);
	printf("Done!\n\n");
	return 0;
}
