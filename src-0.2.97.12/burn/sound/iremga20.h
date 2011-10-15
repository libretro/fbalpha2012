
void iremga20_write(int device, int offset, int data);
unsigned char iremga20_read(int device, int offset);

void iremga20_reset(int device);
void iremga20_init(int device, unsigned char *rom, int rom_size, int frequency);
void iremga20_update(int device, short *buffer, int length);
void iremga20_exit();

int iremga20_scan(int device, int nAction, int *pnMin);
