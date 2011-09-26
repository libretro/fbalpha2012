#include "burnint.h"
#include "msm5205.h"
#include "math.h"

#define MAX_MSM5205	2

struct _MSM5205_state
{
	INT32 data;               /* next adpcm data              */
	INT32 vclk;               /* vclk signal (external mode)  */
	INT32 reset;              /* reset pin signal             */
	INT32 prescaler;          /* prescaler selector S1 and S2 */
	INT32 bitwidth;           /* bit width selector -3B/4B    */
	INT32 signal;             /* current ADPCM signal         */
	INT32 step;               /* current ADPCM step           */
	int volume;

	INT32 clock;		  /* clock rate */

	void (*vclk_callback)();  /* VCLK callback              */
	int (*stream_sync)(int);
	int select;       	  /* prescaler / bit width selector        */
	int bAdd;
	int streampos;

	int diff_lookup[49*16];
};

static short *stream[MAX_MSM5205];
static struct _MSM5205_state chips[MAX_MSM5205];
static struct _MSM5205_state *voice;

static void MSM5205_playmode(int chip, int select);

static const int index_shift[8] = { -1, -1, -1, -1, 2, 4, 6, 8 };

static void ComputeTables(int chip)
{
	voice = &chips[chip];

	/* nibble to bit map */
	static const int nbl2bit[16][4] =
	{
		{ 1, 0, 0, 0}, { 1, 0, 0, 1}, { 1, 0, 1, 0}, { 1, 0, 1, 1},
		{ 1, 1, 0, 0}, { 1, 1, 0, 1}, { 1, 1, 1, 0}, { 1, 1, 1, 1},
		{-1, 0, 0, 0}, {-1, 0, 0, 1}, {-1, 0, 1, 0}, {-1, 0, 1, 1},
		{-1, 1, 0, 0}, {-1, 1, 0, 1}, {-1, 1, 1, 0}, {-1, 1, 1, 1}
	};

	int step, nib;

	/* loop over all possible steps */
	for (step = 0; step <= 48; step++)
	{
		/* compute the step value */
		int stepval = (int)(floor (16.0 * pow (11.0 / 10.0, (double)step)));

		/* loop over all nibbles and compute the difference */
		for (nib = 0; nib < 16; nib++)
		{
			voice->diff_lookup[step*16 + nib] = nbl2bit[nib][0] *
				(stepval   * nbl2bit[nib][1] +
				 stepval/2 * nbl2bit[nib][2] +
				 stepval/4 * nbl2bit[nib][3] +
				 stepval/8);
		}
	}
}

static void MSM5205_playmode(int , int select)
{
	static const int prescaler_table[4] = {96,48,64,0};
	int prescaler = prescaler_table[select & 3];
	int bitwidth = (select & 4) ? 4 : 3;

	if( voice->prescaler != prescaler )
	{
		voice->prescaler = prescaler;

		if( prescaler )
		{
// clock * prescaler
// 384000 / 48 -> 8000

// if cpu is 4000000 (4mhz)
// check MSM5205 every 4000000 / 8000 -> 500 cycles

//			attotime period = attotime_mul(ATTOTIME_IN_HZ(voice->clock), prescaler);
//			timer_adjust_periodic(voice->timer, period, 0, period);
		}
	}

	if( voice->bitwidth != bitwidth )
	{
		voice->bitwidth = bitwidth;
	}
}

static void MSM5205StreamUpdate(int chip)
{
	voice = &chips[chip];

	unsigned int len = voice->stream_sync((nBurnSoundLen * nBurnFPS) / 100);
	if (len > (unsigned int)nBurnSoundLen) len = nBurnSoundLen;
	unsigned int pos = voice->streampos;

	if (pos >= len) return;

	len -= pos;
	voice->streampos = pos + len;

	if (pos == 0) {
		memset (stream[chip], 0, nBurnSoundLen * sizeof(short));
	}

	{
		short *buffer = stream[chip];
		buffer += pos;

		if(voice->signal)
		{
			int i = 0;

			int volval = ((voice->signal * 16) * voice->volume) / 100;
			short val = volval;
			while (len)
			{
				buffer[i] = val;
				len--; i++;
			}
		} else {
			memset (buffer, 0, sizeof(short) * len);
		}
	}
}

static void MSM5205_vclk_callback(int chip)
{
	voice = &chips[chip];

	if(voice->vclk_callback)(*voice->vclk_callback)();

	int new_signal;

	if(voice->reset)
	{
		new_signal = 0;
		voice->step = 0;
	}
	else
	{
		int val = voice->data;
		new_signal = voice->signal + voice->diff_lookup[voice->step * 16 + (val & 15)];
		if (new_signal > 2047) new_signal = 2047;
		else if (new_signal < -2048) new_signal = -2048;
		voice->step += index_shift[val & 7];
		if (voice->step > 48) voice->step = 48;
		else if (voice->step < 0) voice->step = 0;
	}
	/* update when signal changed */
	if( voice->signal != new_signal)
	{
		MSM5205StreamUpdate(chip);
		voice->signal = new_signal;
	}
}

void MSM5205Render(int chip, short *buffer, int len)
{
	voice = &chips[chip];
	short *source = stream[chip];

	MSM5205StreamUpdate(chip);

	voice->streampos = 0;
	
	if (voice->bAdd) {
		for (int i = 0; i < len; i++)
		{
			int nSample0 = buffer[0] + source[i];
			int nSample1 = buffer[1] + source[i];

			if (nSample0 < -32768) {
				nSample0 = -32768;
			} else {
				if (nSample0 > 32767) {
					nSample0 = 32767;
				}
			}

			if (nSample1 < -32768) {
				nSample1 = -32768;
			} else {
				if (nSample1 > 32767) {
					nSample1 = 32767;
				}
			}

			buffer[0] = nSample0;
			buffer[1] = nSample1;
			buffer += 2;
		}
	} else {
		for (int i = 0; i < len; i++)
		{
			buffer[0] = buffer[1] = source[i];
			buffer+=2;
		}
	}
}

void MSM5205Reset()
{
	for (int chip = 0; chip < MAX_MSM5205; chip++)
	{
		voice = &chips[chip];

		if (stream[chip] == NULL) continue;

		voice->data    = 0;
		voice->vclk    = 0;
		voice->reset   = 0;
		voice->signal  = 0;
		voice->step    = 0;

		MSM5205_playmode(chip,voice->select);
		voice->streampos = 0;
	}
}

void MSM5205Init(int chip, int (*stream_sync)(int), int clock, void (*vclk_callback)(), int select, int volume, int bAdd)
{
	voice = &chips[chip];

	memset (voice, 0, sizeof(_MSM5205_state));

	voice->stream_sync	= stream_sync;
	voice->vclk_callback	= vclk_callback;
	voice->select		= select;
	voice->clock		= clock;
	voice->bAdd		= bAdd;
	voice->volume		= volume;

	stream[chip]		= (short*)malloc(nBurnSoundLen * sizeof(short));

	ComputeTables (chip);
}

void MSM5205Exit()
{
	for (int chip = 0; chip < MAX_MSM5205; chip++)
	{
		voice = &chips[chip];

		if (stream[chip] == NULL) continue;

		memset (voice, 0, sizeof(_MSM5205_state));

		free (stream[chip]);
		stream[chip] = NULL;
	}
}

void MSM5205VCLKWrite(int chip, int vclk)
{
	voice = &chips[chip];

	if (voice->prescaler == 0)
	{
		if( voice->vclk != vclk)
		{
			voice->vclk = vclk;
			if( !vclk ) MSM5205_vclk_callback(chip);
		}
	}
}

void MSM5205ResetWrite(int chip, int reset)
{
	voice = &chips[chip];
	voice->reset = reset;
}

void MSM5205DataWrite(int chip, int data)
{
	voice = &chips[chip];

	if( voice->bitwidth == 4)
		voice->data = data & 0x0f;
	else
		voice->data = (data & 0x07)<<1;
}

void MSM5205PlaymodeWrite(int chip, int select)
{
	voice = &chips[chip];
	MSM5205_playmode(chip,select);
}

void MSM5205SetVolume(int chip, int volume)
{
	voice = &chips[chip];
	voice->volume = volume;
}

void MSM5205Update()
{
	for (int chip = 0; chip < MAX_MSM5205; chip++)
	{
		voice = &chips[chip];

		if (voice->prescaler) {
			MSM5205_vclk_callback(chip);
		} else {
			if (stream[chip]) {
				MSM5205StreamUpdate(chip);
			}
		}
	}
}

// see MSM5205_playmode for a more in-depth explanation of this
int MSM5205CalcInterleave(int chip, int cpu_speed)
{
	static const int table[4] = {96, 48, 64, 0};

	voice = &chips[chip];

	if ((voice->select & 3) == 3) {
		return 133;  // (usually...)
	}

	int ret = cpu_speed / (cpu_speed / (voice->clock / table[voice->select & 3]));

	return ret / (nBurnFPS / 100);
}

void MSM5205Scan(int nAction, int *pnMin)
{
	if (pnMin != NULL) {
		*pnMin = 0x029708;
	}

	if (nAction & ACB_DRIVER_DATA) {
		for (int chip = 0; chip < MAX_MSM5205; chip++) {
			voice = &chips[chip];

			SCAN_VAR(voice->data);

			SCAN_VAR(voice->vclk);
			SCAN_VAR(voice->reset);
			SCAN_VAR(voice->prescaler);
			SCAN_VAR(voice->bitwidth);
			SCAN_VAR(voice->signal);
			SCAN_VAR(voice->step);
			SCAN_VAR(voice->volume);
		}
	}
}
