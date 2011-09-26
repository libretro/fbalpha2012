/*********************************************************

    Konami 053260 PCM Sound Chip

*********************************************************/

#include "burnint.h"

/* 2004-02-28: Fixed ppcm decoding. Games sound much better now.*/

#define BASE_SHIFT	16

#define K053260_INLINE		static inline

static unsigned int nUpdateStep;

struct k053260_channel_def {
	unsigned long		rate;
	unsigned long		size;
	unsigned long		start;
	unsigned long		bank;
	unsigned long		volume;
	int					play;
	unsigned long		pan;
	unsigned long		pos;
	int					loop;
	int					ppcm; /* packed PCM ( 4 bit signed ) */
	int					ppcm_data;
};
struct k053260_chip_def {
	int								mode;
	int								regs[0x30];
	unsigned char					*rom;
	int								rom_size;
	unsigned long					*delta_table;
	k053260_channel_def channels[4];
};

static k053260_chip_def Chips[2];
static k053260_chip_def *ic;

static void InitDeltaTable(int rate, int clock ) {
	int		i;
	double	base = ( double )rate;
	double	max = (double)(clock); /* Hz */
	unsigned long val;

	for( i = 0; i < 0x1000; i++ ) {
		double v = ( double )( 0x1000 - i );
		double target = (max) / v;
		double fixed = ( double )( 1 << BASE_SHIFT );

		if ( target && base ) {
			target = fixed / ( base / target );
			val = ( unsigned long )target;
			if ( val == 0 )
				val = 1;
		} else
			val = 1;

		ic->delta_table[i] = val;
	}
}

void K053260Reset(int chip)
{
	ic = &Chips[chip];

	for(int i = 0; i < 4; i++ ) {
		ic->channels[i].rate = 0;
		ic->channels[i].size = 0;
		ic->channels[i].start = 0;
		ic->channels[i].bank = 0;
		ic->channels[i].volume = 0;
		ic->channels[i].play = 0;
		ic->channels[i].pan = 0;
		ic->channels[i].pos = 0;
		ic->channels[i].loop = 0;
		ic->channels[i].ppcm = 0;
		ic->channels[i].ppcm_data = 0;
	}
}

K053260_INLINE int limit( int val, int max, int min ) {
	if ( val > max )
		val = max;
	else if ( val < min )
		val = min;

	return val;
}

#define MAXOUT 0x3fff
#define MINOUT -0x4000

void K053260Update(int chip, short *pBuf, int length)
{
	static const long dpcmcnv[] = { 0,1,2,4,8,16,32,64, -128, -64, -32, -16, -8, -4, -2, -1};

	int i, j, lvol[4], rvol[4], play[4], loop[4], ppcm_data[4], ppcm[4];
	unsigned char *rom[4];
	unsigned long delta[4], end[4], pos[4];
	int dataL, dataR;
	signed char d;
	ic = &Chips[chip];

	/* precache some values */
	for ( i = 0; i < 4; i++ ) {
		rom[i]= &ic->rom[ic->channels[i].start + ( ic->channels[i].bank << 16 )];
		delta[i] = (ic->delta_table[ic->channels[i].rate] * nUpdateStep) >> 15;
		lvol[i] = ic->channels[i].volume * ic->channels[i].pan;
		rvol[i] = ic->channels[i].volume * ( 8 - ic->channels[i].pan );
		end[i] = ic->channels[i].size;
		pos[i] = ic->channels[i].pos;
		play[i] = ic->channels[i].play;
		loop[i] = ic->channels[i].loop;
		ppcm[i] = ic->channels[i].ppcm;
		ppcm_data[i] = ic->channels[i].ppcm_data;
		if ( ppcm[i] )
			delta[i] /= 2;
	}

		for ( j = 0; j < length; j++ ) {

			dataL = dataR = 0;

			for ( i = 0; i < 4; i++ ) {
				/* see if the voice is on */
				if ( play[i] ) {
					/* see if we're done */
					if ( ( pos[i] >> BASE_SHIFT ) >= end[i] ) {

						ppcm_data[i] = 0;
						if ( loop[i] )
							pos[i] = 0;
						else {
							play[i] = 0;
							continue;
						}
					}

					if ( ppcm[i] ) { /* Packed PCM */
						/* we only update the signal if we're starting or a real sound sample has gone by */
						/* this is all due to the dynamic sample rate convertion */
						if ( pos[i] == 0 || ( ( pos[i] ^ ( pos[i] - delta[i] ) ) & 0x8000 ) == 0x8000 )

						 {
							int newdata;
							if ( pos[i] & 0x8000 ){

								newdata = ((rom[i][pos[i] >> BASE_SHIFT]) >> 4) & 0x0f; /*high nybble*/
							}
							else{
								newdata = ( ( rom[i][pos[i] >> BASE_SHIFT] ) ) & 0x0f; /*low nybble*/
							}

							ppcm_data[i] = (( ( ppcm_data[i] * 62 ) >> 6 ) + dpcmcnv[newdata]);

							if ( ppcm_data[i] > 127 )
								ppcm_data[i] = 127;
							else
								if ( ppcm_data[i] < -128 )
									ppcm_data[i] = -128;
						}

						d = ppcm_data[i];

						pos[i] += delta[i];
					} else { /* PCM */
						d = rom[i][pos[i] >> BASE_SHIFT];

						pos[i] += delta[i];
					}

					if ( ic->mode & 2 ) {
						dataL += ( d * lvol[i] ) >> 2;
						dataR += ( d * rvol[i] ) >> 2;
					}
				}
			}

			pBuf[0] += limit( dataL, MAXOUT, MINOUT );
			pBuf[1] += limit( dataR, MAXOUT, MINOUT );
			pBuf += 2;
		}

	/* update the regs now */
	for ( i = 0; i < 4; i++ ) {
		ic->channels[i].pos = pos[i];
		ic->channels[i].play = play[i];
		ic->channels[i].ppcm_data = ppcm_data[i];
	}
}

void K053260Init(int chip, int clock, unsigned char *rom, int nLen)
{
	ic = &Chips[chip];
	memset (ic, 0, sizeof(k053260_chip_def));

	int rate = clock / 32;
	int i;
	
	nUpdateStep = (int)(((float)rate / nBurnSoundRate) * 32768);

	ic->mode = 0;
	ic->rom = rom;
	ic->rom_size = nLen - 1;

	K053260Reset(chip);

	for ( i = 0; i < 0x30; i++ )
		ic->regs[i] = 0;

	ic->delta_table = ( unsigned long * )malloc( 0x1000 * sizeof( unsigned long ) );

	InitDeltaTable( rate, clock );

	/* setup SH1 timer if necessary */
//	if ( ic->intf->irq )
//		timer_pulse( attotime_mul(ATTOTIME_IN_HZ(clock), 32), NULL, 0, ic->intf->irq );
}

void K053260Exit()
{
	for (int i = 0; i < 2; i++) {
		ic = &Chips[i];

		if (ic->delta_table) {
			free (ic->delta_table);
			ic->delta_table = NULL;
		}
	}
	
	nUpdateStep = 0;
}

K053260_INLINE void check_bounds(int channel ) {

	int channel_start = ( ic->channels[channel].bank << 16 ) + ic->channels[channel].start;
	int channel_end = channel_start + ic->channels[channel].size - 1;

	if ( channel_start > ic->rom_size ) {
		ic->channels[channel].play = 0;

		return;
	}

	if ( channel_end > ic->rom_size ) {
		ic->channels[channel].size = ic->rom_size - channel_start;
	}
}

void K053260Write(int chip, int offset, unsigned char data)
{
	int i, t;
	int r = offset;
	int v = data;

	ic = &Chips[chip];

	if ( r > 0x2f ) {
		return;
	}

	/* before we update the regs, we need to check for a latched reg */
	if ( r == 0x28 ) {
		t = ic->regs[r] ^ v;

		for ( i = 0; i < 4; i++ ) {
			if ( t & ( 1 << i ) ) {
				if ( v & ( 1 << i ) ) {
					ic->channels[i].play = 1;
					ic->channels[i].pos = 0;
					ic->channels[i].ppcm_data = 0;
					check_bounds( i );
				} else
					ic->channels[i].play = 0;
			}
		}

		ic->regs[r] = v;
		return;
	}

	/* update regs */
	ic->regs[r] = v;

	/* communication registers */
	if ( r < 8 )
		return;

	/* channel setup */
	if ( r < 0x28 ) {
		int channel = ( r - 8 ) / 8;

		switch ( ( r - 8 ) & 0x07 ) {
			case 0: /* sample rate low */
				ic->channels[channel].rate &= 0x0f00;
				ic->channels[channel].rate |= v;
			break;

			case 1: /* sample rate high */
				ic->channels[channel].rate &= 0x00ff;
				ic->channels[channel].rate |= ( v & 0x0f ) << 8;
			break;

			case 2: /* size low */
				ic->channels[channel].size &= 0xff00;
				ic->channels[channel].size |= v;
			break;

			case 3: /* size high */
				ic->channels[channel].size &= 0x00ff;
				ic->channels[channel].size |= v << 8;
			break;

			case 4: /* start low */
				ic->channels[channel].start &= 0xff00;
				ic->channels[channel].start |= v;
			break;

			case 5: /* start high */
				ic->channels[channel].start &= 0x00ff;
				ic->channels[channel].start |= v << 8;
			break;

			case 6: /* bank */
				ic->channels[channel].bank = v & 0xff;
			break;

			case 7: /* volume is 7 bits. Convert to 8 bits now. */
				ic->channels[channel].volume = ( ( v & 0x7f ) << 1 ) | ( v & 1 );
			break;
		}

		return;
	}

	switch( r ) {
		case 0x2a: /* loop, ppcm */
			for ( i = 0; i < 4; i++ )
				ic->channels[i].loop = ( v & ( 1 << i ) ) != 0;

			for ( i = 4; i < 8; i++ )
				ic->channels[i-4].ppcm = ( v & ( 1 << i ) ) != 0;
		break;

		case 0x2c: /* pan */
			ic->channels[0].pan = v & 7;
			ic->channels[1].pan = ( v >> 3 ) & 7;
		break;

		case 0x2d: /* more pan */
			ic->channels[2].pan = v & 7;
			ic->channels[3].pan = ( v >> 3 ) & 7;
		break;

		case 0x2f: /* control */
			ic->mode = v & 7;
			/* bit 0 = read ROM */
			/* bit 1 = enable sound output */
			/* bit 2 = unknown */
		break;
	}
}

unsigned char K053260Read(int chip, int offset)
{
	ic = & Chips[chip];

	switch ( offset ) {
		case 0x29: /* channel status */
			{
				int i, status = 0;

				for ( i = 0; i < 4; i++ )
					status |= ic->channels[i].play << i;

				return status;
			}
		break;

		case 0x2e: /* read rom */
			if ( ic->mode & 1 ) {
				unsigned int offs = ic->channels[0].start + ( ic->channels[0].pos >> BASE_SHIFT ) + ( ic->channels[0].bank << 16 );

				ic->channels[0].pos += ( 1 << 16 );

				if ( offs > (unsigned int)ic->rom_size ) {

					return 0;
				}

				return ic->rom[offs];
			}
		break;
	}

	return ic->regs[offset];
}

int K053260Scan(int nAction)
{
	struct BurnArea ba;
	char szName[32];

	if ((nAction & ACB_DRIVER_DATA) == 0) {
		return 1;
	}

	for (int i = 0; i < 2; i++) {
		ic = &Chips[i];

		sprintf(szName, "k053260 regs %d", 0);
		ba.Data		= ic->regs;
		ba.nLen		= 0x30 * sizeof(int);
		ba.nAddress = 0;
		ba.szName	= szName;
		BurnAcb(&ba);

		sprintf(szName, "k053260 channels # %d", 0);
		ba.Data		= ic->channels;
		ba.nLen		= 4 * sizeof(k053260_channel_def);
		ba.nAddress = 0;
		ba.szName	= szName;
		BurnAcb(&ba);

		SCAN_VAR(ic->mode);
	}

	return 0;
}

#undef K053260_INLINE
