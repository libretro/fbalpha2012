/*** m6800: Portable 6800 class emulator *************************************/

#ifndef _M6800_H
#define _M6800_H

//#include "cpuintrf.h"

/* 6800 Registers */
typedef struct
{
//  int     subtype;        /* CPU subtype */
	PAIR	ppc;			/* Previous program counter */
	PAIR	pc; 			/* Program counter */
	PAIR	s;				/* Stack pointer */
	PAIR	x;				/* Index register */
	PAIR	d;				/* Accumulators */
	UINT8	cc; 			/* Condition codes */
	UINT8	wai_state;		/* WAI opcode state ,(or sleep opcode state) */
	UINT8	nmi_state;		/* NMI line state */
	UINT8	irq_state[2];	/* IRQ line state [IRQ1,TIN] */
	UINT8	ic_eddge;		/* InputCapture eddge , b.0=fall,b.1=raise */

//	int		(*irq_callback)(int irqline);
	int 	extra_cycles;	/* cycles used for interrupts */
	void	(* const * insn)(void);	/* instruction table */
	const UINT8 *cycles;			/* clock cycle of instruction table */
	/* internal registers */
	UINT8	port1_ddr;
	UINT8	port2_ddr;
	UINT8	port3_ddr;
	UINT8	port4_ddr;
	UINT8	port1_data;
	UINT8	port2_data;
	UINT8	port3_data;
	UINT8	port4_data;
	UINT8	tcsr, rmcr;			/* Timer Control and Status Register */
	UINT8	pending_tcsr;	/* pending IRQ flag for clear IRQflag process */
	UINT8	irq2;			/* IRQ2 flags */
	UINT8	ram_ctrl;
	PAIR	counter;		/* free running counter */
	PAIR	output_compare;	/* output compare       */
	UINT16	input_capture;	/* input capture        */

	PAIR	timer_over;
}   m6800_Regs;

enum {
	M6800_PC=1, M6800_S, M6800_A, M6800_B, M6800_X, M6800_CC,
	M6800_WAI_STATE };

#define M6800_WAI		8			/* set when WAI is waiting for an interrupt */
#define M6800_SLP		0x10		/* HD63701 only */


#define M6800_IRQ_LINE	0			/* IRQ line number */
#define M6800_TIN_LINE	1			/* P20/Tin Input Capture line (eddge sense)     */
									/* Active eddge is selecrable by internal reg.  */
									/* raise eddge : CLEAR_LINE  -> ASSERT_LINE     */
									/* fall  eddge : ASSERT_LINE -> CLEAR_LINE      */
									/* it is usuali to use PULSE_LINE state         */
#define M6800_INPUT_LINE_NMI	32
									
unsigned char M6800ReadByte(unsigned short Address);
void M6800WriteByte(unsigned short Address, unsigned char Data);
unsigned char M6800ReadOp(unsigned short Address);
unsigned char M6800ReadOpArg(unsigned short Address);
unsigned char M6800ReadPort(unsigned short Address);
void M6800WritePort(unsigned short Address, unsigned char Data);

void m6800_init();
void hd63701_init();
void m6803_init();
void m6801_init();
void m6800_reset(void);
int m6800_get_pc();
void m6800_get_context(void *dst);
void m6800_set_context(void *src);
int m6800_execute(int cycles);
int m6803_execute(int cycles);
int hd63701_execute(int cycles);
void m6800_set_irq_line(int irqline, int state);

void m6803_internal_registers_w(unsigned short offset, unsigned char data);
unsigned char m6803_internal_registers_r(unsigned short offset);

//extern void m6800_get_info(UINT32 state, cpuinfo *info);

/****************************************************************************/
/* Read a byte from given memory location                                   */
/****************************************************************************/
/* ASG 971005 -- changed to program_read_byte_8/program_write_byte_8 */
#define M6800_RDMEM(Addr) ((unsigned)M6800ReadByte(Addr))

/****************************************************************************/
/* Write a byte to given memory location                                    */
/****************************************************************************/
#define M6800_WRMEM(Addr,Value) (M6800WriteByte(Addr,Value))

/****************************************************************************/
/* M6800_RDOP() is identical to M6800_RDMEM() except it is used for reading */
/* opcodes. In case of system with memory mapped I/O, this function can be  */
/* used to greatly speed up emulation                                       */
/****************************************************************************/
#define M6800_RDOP(Addr) ((unsigned)M6800ReadOp(Addr))

/****************************************************************************/
/* M6800_RDOP_ARG() is identical to M6800_RDOP() but it's used for reading  */
/* opcode arguments. This difference can be used to support systems that    */
/* use different encoding mechanisms for opcodes and opcode arguments       */
/****************************************************************************/
#define M6800_RDOP_ARG(Addr) ((unsigned)M6800ReadOpArg(Addr))

#define M6800_io_read_byte_8(Port)		((unsigned)M6800ReadPort(Port))
#define M6800_io_write_byte_8(Port,Value)	(M6800WritePort(Port, Value))

#ifndef FALSE
#    define FALSE 0
#endif
#ifndef TRUE
#    define TRUE (!FALSE)
#endif

#ifdef	MAME_DEBUG
offs_t m6800_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram);
offs_t m6801_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram);
offs_t m6802_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram);
ffs_t m6803_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram);
offs_t m6808_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram);
offs_t hd63701_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram);
offs_t nsc8105_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram);
#endif

#endif /* _M6800_H */
