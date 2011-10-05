#include "burnint.h"
#include "rf5c68.h"
#include "burn_sound.h"

#define NUM_CHANNELS	(8)

static unsigned int nUpdateStep;

struct pcm_channel
{
	unsigned char	enable;
	unsigned char	env;
	unsigned char	pan;
	unsigned char	start;
	unsigned int	addr;
	unsigned short	step;
	unsigned short	loopst;
};

struct rf5c68pcm
{
	struct pcm_channel	chan[NUM_CHANNELS];
	unsigned char		cbank;
	unsigned char		wbank;
	unsigned char		enable;
	unsigned char		data[0x10000];
};

static struct rf5c68pcm *chip = NULL;

static int *left = NULL;
static int *right = NULL;

void RF5C68PCMUpdate(short* pSoundBuf, int length)
{
	if (!chip->enable) return;
	
	int i, j;
	
	left = (int*)malloc(nBurnSoundLen * sizeof(int));
	right = (int*)malloc(nBurnSoundLen * sizeof(int));
	
	memset(left, 0, length * sizeof(int));
	memset(right, 0, length * sizeof(int));
	
	for (i = 0; i < NUM_CHANNELS; i++) {
		pcm_channel *chan = &chip->chan[i];
		
		if (chan->enable) {
			int lv = (chan->pan & 0xf) * chan->env;
			int rv = ((chan->pan >> 4) & 0xf) * chan->env;
		
			for (j = 0; j < length; j++) {
				int sample;
				
				sample = chip->data[(chan->addr >> 11) & 0xffff];
				if (sample == 0xff) {
					chan->addr = chan->loopst << 11;
					sample = chip->data[(chan->addr >> 11) & 0xffff];
					if (sample == 0xff) break;
				}
				
				chan->addr += (chan->step * nUpdateStep) >> 15;//(chan->step * 1181) / 1000;
				
				if (sample & 0x80) {
					sample &= 0x7f;
					left[j] += (sample * lv) >> 5;
					right[j] += (sample * rv) >> 5;
				} else {
					left[j] -= (sample * lv) >> 5;
					right[j] -= (sample * rv) >> 5;
				}
			}
		}
	}
	
	for (i = 0; i < length; i++) {
		if (left[i] > 32767) left[i] = 32767;
		if (left[i] < -32768) left[i] = -32768;
		left[i] = left[i] & ~0x3f;
		if (right[i] > 32767) right[i] = 32767;
		if (right[i] < -32768) right[i] = -32768;
		right[i] = right[i] & ~0x3f;
		
		pSoundBuf[i + 0] = left[i];
		pSoundBuf[i + 1] = right[i];
	}
	
	free(left);
	left = NULL;
	free(right);
	right = NULL;
}

void RF5C68PCMReset()
{
	memset(chip, 0, sizeof(*chip));
	memset(chip->data, 0xff, sizeof(chip->data));
}

void RF5C68PCMInit(int clock)
{
	chip = (struct rf5c68pcm*)malloc(sizeof(struct rf5c68pcm));
	
	int Rate = clock / 384;
	
	nUpdateStep = (int)(((float)Rate / nBurnSoundRate) * 32768);
}

void RF5C68PCMRegWrite(unsigned char offset, unsigned char data)
{
	struct pcm_channel *chan = &chip->chan[chip->cbank];
	int i;
	
	/* force the stream to update first */
//	stream_update(chip->stream);

	switch (offset) {
		case 0x00: {
			chan->env = data;
			break;
		}
		
		case 0x01: {
			chan->pan = data;
			break;
		}
		
		case 0x02: {
			chan->step = (chan->step & 0xff00) | (data & 0xff);
			break;
		}
		
		case 0x03: {
			chan->step = (chan->step & 0xff) | ((data << 8) & 0xff00);
			break;
		}
		
		case 0x04: {
			chan->loopst = (chan->loopst & 0xff00) | (data & 0xff);
			break;
		}
		
		case 0x05: {
			chan->loopst = (chan->loopst & 0xff) | ((data << 8) & 0xff00);
			break;
		}
		
		case 0x06: {
			chan->start = data;
			if (!chan->enable) chan->addr = chan->start << (8 + 11);
			break;
		}
		
		case 0x07: {
			chip->enable = (data >> 7) & 1;
			if (data & 0x40) {
				chip->cbank = data & 7;
			} else {
				chip->wbank = data & 15;
			}
			break;
		}
		
		case 0x08: {
			for (i = 0; i < 8; i++) {
				chip->chan[i].enable = (~data >> i) & 1;
				if (!chip->chan[i].enable) chip->chan[i].addr = chip->chan[i].start << (8 + 11);
			}
			break;
		}
	}
}

unsigned char RF5C68PCMRead(unsigned short offset)
{
	return chip->data[chip->wbank * 0x1000 + offset];
}

void RF5C68PCMWrite(unsigned short offset, unsigned char data)
{
	chip->data[(chip->wbank * 0x1000) + offset] = data;
}

#undef NUM_CHANNELS
