
#include "burn_ym3812.h"
#include "burn_ym2151.h"
#include "burn_ym2203.h"
#include "msm6295.h"

extern unsigned char *SeibuZ80DecROM;
extern unsigned char *SeibuZ80ROM;
extern unsigned char *SeibuZ80RAM;

extern int seibu_coin_input;

unsigned char seibu_main_word_read(int offset);
void seibu_main_word_write(int offset, unsigned char data);
void seibu_sound_mustb_write_word(int offset, unsigned char data);

void seibu_sound_reset();

void seibu_sound_update(short *pbuf, int nLen);

/*
	Type 0 - YM3812
	Type 1 - YM2151
	Type 2 - YM2203

	all init a single oki6295
	add 4 to init a second oki6295
*/

void seibu_sound_init(int type, int encrypted_len, int freq0 /*cpu*/, int freq1 /*ym*/, int freq2 /*oki*/);
void seibu_sound_exit();

void seibu_sound_scan(int *pnMin, int nAction);
