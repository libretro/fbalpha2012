
extern unsigned char *NMK004OKIROM0;
extern unsigned char *NMK004OKIROM1;
extern unsigned char *NMK004PROGROM;

void NMK004_init();

void NMK004Write(int, int data);
unsigned char NMK004Read();

void NMK004_irq(int irq);
