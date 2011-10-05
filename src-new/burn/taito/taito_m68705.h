#include "m6805_intf.h"

struct m68705_interface {
	void (*portA_out)(unsigned char *data);
	void (*portB_out)(unsigned char *data);
	void (*portC_out)(unsigned char *data);
	void (*ddrA_out)(unsigned char *data);
	void (*ddrB_out)(unsigned char *data);
	void (*ddrC_out)(unsigned char *data);
	void (*portA_in)();
	void (*portB_in)();
	void (*portC_in)();
};

extern unsigned char portA_in;
extern unsigned char portA_out;
extern unsigned char ddrA;
extern unsigned char portB_in;
extern unsigned char portB_out;
extern unsigned char ddrB;
extern unsigned char portC_in;
extern unsigned char portC_out;
extern unsigned char ddrC;

extern unsigned char from_main;
extern unsigned char from_mcu;
extern int mcu_sent;
extern int main_sent;

void m67805_taito_reset();
void m67805_taito_init(unsigned char *rom, unsigned char *ram, m68705_interface *interface);
void m67805_taito_exit();
int m68705_taito_scan(int nAction);

void standard_taito_mcu_write(int data);
int standard_taito_mcu_read();
void standard_m68705_portB_out(unsigned char *data);
void standard_m68705_portC_in();

extern m68705_interface standard_m68705_interface;
