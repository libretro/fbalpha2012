
#include "cps3.h"

#define CPS3_VOICES		16

#define CPS3_SND_INT_RATE		(nBurnFPS / 100)
#define CPS3_SND_RATE			(42954500 / 3 / 384)
#define CPS3_SND_BUFFER_SIZE	(CPS3_SND_RATE / CPS3_SND_INT_RATE)
#define CPS3_SND_LINEAR_SHIFT	12

typedef struct {
	uint16_t regs[16];
	uint16_t frac;
	uint32_t pos;
} cps3_voice;

typedef struct {
	uint8_t * rombase;
	uint16_t key;
	uint32_t delta;
	cps3_voice voice[CPS3_VOICES];
} cps3snd_chip;

static cps3snd_chip * chip;

uint8_t __fastcall cps3SndReadByte(uint32_t addr)
{
	addr &= 0x000003ff;
	//bprintf(PRINT_NORMAL, _T("SND Attempt to read byte value of location %8x\n"), addr);
	return 0;
}

uint16_t __fastcall cps3SndReadWord(uint32_t addr)
{
	addr &= 0x000003ff;

	if (addr < 0x200)
		return chip->voice[addr >> 5].regs[(addr>>1) & 0xf];
	else if (addr == 0x200)
		return chip->key;
	//else
		//bprintf(PRINT_NORMAL, _T("SND Attempt to read word value of location %8x\n"), addr);
	return 0;
}

uint32_t __fastcall cps3SndReadLong(uint32_t addr)
{
	addr &= 0x000003ff;
	//bprintf(PRINT_NORMAL, _T("SND Attempt to read long value of location %8x\n"), addr);
	return 0;
}

void __fastcall cps3SndWriteByte(uint32_t addr, uint8_t data)
{
	addr &= 0x000003ff;
	//bprintf(PRINT_NORMAL, _T("SND Attempt to write byte value %2x to location %8x\n"), data, addr);
}

void __fastcall cps3SndWriteWord(uint32_t addr, uint16_t data)
{
	addr &= 0x000003ff;
	
	if (addr < 0x200)
	{
		chip->voice[addr >> 5].regs[(addr>>1) & 0xf] = data;
		//bprintf(PRINT_NORMAL, _T("SND Attempt to write word value %4x to Chip[%02d][%02d] %s\n"), data, addr >> 5, (addr>>2) & 7, (addr & 0x02) ? "lo" : "hi" );
	}
	else if (addr == 0x200)
	{
		uint16_t key = data;
		for (int i = 0; i < CPS3_VOICES; i++)
		{
			// Key off -> Key on
			if ((key & (1 << i)) && !(chip->key & (1 << i)))
			{
				chip->voice[i].frac = 0;
				chip->voice[i].pos = 0;
			}
		}
		chip->key = key;
	}
	#if 0
   else
		bprintf(PRINT_NORMAL, "SND Attempt to write word value %4x to location %8x\n", data, addr);
	#endif
}

void __fastcall cps3SndWriteLong(uint32_t addr, uint32_t data)
{
	//addr &= 0x000003ff;
	//bprintf(PRINT_NORMAL, "SND Attempt to write long value %8x to location %8x\n", data, addr);
}

int cps3SndInit(uint8_t * sndrom)
{
	chip = (cps3snd_chip *) malloc( sizeof(cps3snd_chip) );
	if ( chip ) {
		memset( chip, 0, sizeof(cps3snd_chip) );
		chip->rombase = sndrom;
		
		/* 
		 * CPS-3 Sound chip clock: 42954500 / 3 / 384 = 37286.89
		 * Sound interupt 80Hz 
		 */
		
		if (nBurnSoundRate)
		{
			//chip->delta = 37286.9 / nBurnSoundRate;
			chip->delta = (CPS3_SND_BUFFER_SIZE << CPS3_SND_LINEAR_SHIFT) / nBurnSoundLen;
			//bprintf(0, "BurnSnd %08x, %d, %d\n", chip->delta, chip->burnlen, nBurnSoundLen);
		}
		
		return 0;
	}
	return 1;
}

void cps3SndReset()
{
}

void cps3SndExit()
{
	free( chip );
}

void cps3SndUpdate()
{
#ifndef SN_TARGET_PS3
	if (!pBurnSoundOut)
	{
		// TODO: ???
		// chip->key = 0;
		return;	
	}
#endif
	
	memset(pBurnSoundOut, 0, nBurnSoundLen * 2 * 2 );
	signed char * base = (signed char *)chip->rombase;
	cps3_voice *vptr = &chip->voice[0];

	for(int i=0; i<CPS3_VOICES; i++, vptr++)
	{
		if (chip->key & (1 << i))
		{

			uint32_t start = ((vptr->regs[ 3] << 16) | vptr->regs[ 2]) - 0x400000;
			uint32_t end   = ((vptr->regs[11] << 16) | vptr->regs[10]) - 0x400000;
			uint32_t loop  = ((vptr->regs[ 9] << 16) | vptr->regs[ 7]) - 0x400000;
			uint32_t step  = ( vptr->regs[ 6] * chip->delta ) >> CPS3_SND_LINEAR_SHIFT;

			int vol_l = (int16_t)vptr->regs[15];
			int vol_r = (int16_t)vptr->regs[14];

			uint32_t pos = vptr->pos;
			uint32_t frac = vptr->frac;

			/* Go through the buffer and add voice contributions */
			int16_t * buffer = (int16_t *)pBurnSoundOut;

			for (uint32_t j=0; j<nBurnSoundLen; j++)
			{
				int32_t sample;

				pos += (frac >> 12);
				frac &= 0xfff;

				if (start + pos >= end)
				{
					if (vptr->regs[5])
						pos = loop - start;
					else
					{
						chip->key &= ~(1 << i);
						break;
					}
				}

				// 8bit sample store with 16bit bigend ???
				sample = base[(start + pos) ^ 1];
				frac += step;

				int32_t sample_l;

				sample_l = ((sample * vol_r) >> 8) + buffer[0];

				if(sample_l > 32767)
					buffer[0] = 32767;
				else if(sample_l < -32768)
					buffer[0] = -32768;
				else
					buffer[0] = sample_l;

				sample_l = ((sample * vol_l) >> 8) + buffer[1];

				if(sample_l > 32767)
					buffer[1] = 32767;
				else if(sample_l < -32768)
					buffer[1] = -32768;
				else
					buffer[1] = sample_l;

				buffer += 2;
			}


			vptr->pos = pos;
			vptr->frac = frac;
		}
	}
	
}

int cps3SndScan(int nAction)
{
	if (nAction & ACB_DRIVER_DATA)
	{
		SCAN_VAR( chip->voice );
		SCAN_VAR( chip->key );
		
	}
	return 0;
}

