#ifndef AY8910_H
#define AY8910_H

#define MAX_8910 5
#define ALL_8910_CHANNELS -1

struct AY8910interface
{
	int num;	/* total number of 8910 in the machine */
	int baseclock;
	int mixing_level[MAX_8910];
    read8_handler portAread[MAX_8910];
    read8_handler portBread[MAX_8910];
    write8_handler portAwrite[MAX_8910];
    write8_handler portBwrite[MAX_8910];
	void (*handler[MAX_8910])(int irq);	/* IRQ handler for the YM2203 */
};

extern int ay8910_index_ym;

void AY8910_set_clock(int chip, int clock);
void AY8910Reset(int chip);

void AY8910Update(int chip, INT16** buffer, int length);

void AY8910Write(int chip, int a, int data);
int AY8910Read(int chip);

void AY8910Reset(int chip);
void AY8910Exit(int chip);
int AY8910Init(int chip, int clock, int sample_rate,
		read8_handler portAread, read8_handler portBread,
		write8_handler portAwrite, write8_handler portBwrite);

int AY8910InitYM(int chip, int clock, int sample_rate,
		read8_handler portAread, read8_handler portBread,
		write8_handler portAwrite, write8_handler portBwrite,
		void (*update_callback)(void));

int AY8910Scan(int nAction, int* pnMin);

int AY8910SetPorts(int chip, read8_handler portAread, read8_handler portBread,
		write8_handler portAwrite, write8_handler portBwrite);

#endif
