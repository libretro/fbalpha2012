/* ASG 971222 -- rewrote this interface */
#ifndef __NEC_H_
#define __NEC_H_

/* NEC registers */
typedef union
{                   /* eight general registers */
    unsigned short w[8];    /* viewed as 16 bits registers */
    unsigned char  b[16];   /* or as 8 bit registers */
} necbasicregs;

typedef struct
{
	necbasicregs regs;
 	unsigned short	sregs[4];

	unsigned short	ip;

	signed int	SignVal;
    unsigned int  AuxVal, OverVal, ZeroVal, CarryVal, ParityVal; /* 0 or non-0 valued flags */
	unsigned char	TF, IF, DF, MF; 	/* 0 or 1 valued flags */	/* OB[19.07.99] added Mode Flag V30 */
	unsigned int	int_vector;
	unsigned int	pending_irq;
	unsigned int	nmi_state;
	unsigned int	irq_state;
	unsigned char	no_interrupt;

	int     (*irq_callback)(int irqline);
	
	unsigned int cpu_type;
	unsigned int prefix_base;
	signed char seg_prefix;
	int nec_ICount;	
	
} nec_Regs;


enum {
	NEC_PC=0,
	NEC_IP, NEC_AW, NEC_CW, NEC_DW, NEC_BW, NEC_SP, NEC_BP, NEC_IX, NEC_IY,
	NEC_FLAGS, NEC_ES, NEC_CS, NEC_SS, NEC_DS,
	NEC_VECTOR, NEC_PENDING };

void nec_set_context(nec_Regs * src);
void nec_reset();
int nec_execute(int cycles);
void nec_set_irq_line(int irqline, int state);
void nec_int(unsigned int wektor);

#endif
