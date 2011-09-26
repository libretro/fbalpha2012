
#define MSM5205_S96_3B 0     /* prescaler 1/96(4KHz) , data 3bit */
#define MSM5205_S48_3B 1     /* prescaler 1/48(8KHz) , data 3bit */
#define MSM5205_S64_3B 2     /* prescaler 1/64(6KHz) , data 3bit */
#define MSM5205_SEX_3B 3     /* VCLK slave mode      , data 3bit */

#define MSM5205_S96_4B 4     /* prescaler 1/96(4KHz) , data 4bit */
#define MSM5205_S48_4B 5     /* prescaler 1/48(8KHz) , data 4bit */
#define MSM5205_S64_4B 6     /* prescaler 1/64(6KHz) , data 4bit */
#define MSM5205_SEX_4B 7     /* VCLK slave mode      , data 4bit */

void MSM5205ResetWrite(int chip, int reset);
void MSM5205DataWrite(int chip, int data);
void MSM5205VCLKWrite(int chip, int reset);
void MSM5205PlaymodeWrite(int chip, int select);

void MSM5205SetVolume(int chip, int volume);

void MSM5205Init(int chip, int (*stream_sync)(int), int clock, void (*vclk_callback)(), int select, int volume, int bAdd);
void MSM5205Reset();

/*
	All MSM5205 sounds need to be rendered in one call
	or the data will likely be output badly
*/

void MSM5205Render(int chip, short *buffer, int len);
void MSM5205Exit();

void MSM5205Scan(int nAction, int *pnMin);

/*
	MSM5205CalcInterleave is used to calculate after how
	many cycles that the sound cpu has ran does
	MSM5205Update need to be called. It returns how many
	slices must be made of the sound cpu's cycles
*/	

int MSM5205CalcInterleave(int chip, int cpu_speed);
void MSM5205Update();
