//
// FB Alpha Seibu sound hardware module
// 
// Games using this hardware:
//
//	Dead Angle	2x YM2203 + adpcm -- not implemented
//
// 	Dynamite Duke   1x YM3812 + 1x M6295
//	Toki		1x YM3812 + 1x M6295
//	Raiden          1x YM3812 + 1x M6295
//	Blood Brothers  1x YM3812 + 1x M6295
//	D-Con		1x YM3812 + 1x M6295
//	Legionnaire	1x YM3812 + 1x M6295
//
//	SD Gundam PSK	1x YM2151 + 1x M6295
//	Raiden II	1x YM2151 + 2x M6295
//	Raiden DX	1x YM2151 + 2x M6295
//	Zero Team	1x YM2151 + 2x M6295
//
// 	Cross Shooter	1x YM2151 + ?
//	Cabal		1x YM2151 + adpcm -- not implemented
//

#include "burnint.h"
#include "burn_ym3812.h"
#include "burn_ym2151.h"
#include "burn_ym2203.h"
#include "msm6295.h"
#include "bitswap.h"

static unsigned char main2sub[2];
static unsigned char sub2main[2];
static int main2sub_pending;
static int sub2main_pending;
static int SeibuSoundBank;

unsigned char *SeibuZ80DecROM;
unsigned char *SeibuZ80ROM;
unsigned char *SeibuZ80RAM;
int seibu_coin_input;

static int seibu_sndcpu_frequency;
static int seibu_snd_type;
static int is_sdgndmps = 0;

enum
{
	VECTOR_INIT,
	RST10_ASSERT,
	RST10_CLEAR,
	RST18_ASSERT,
	RST18_CLEAR
};

static void update_irq_lines(int param)
{
	static int irq1,irq2;

	switch(param)
	{
		case VECTOR_INIT:
			irq1 = irq2 = 0xff;
			break;

		case RST10_ASSERT:
			irq1 = 0xd7;
			break;

		case RST10_CLEAR:
			irq1 = 0xff;
			break;

		case RST18_ASSERT:
			irq2 = 0xdf;
			break;

		case RST18_CLEAR:
			irq2 = 0xff;
			break;
	}

	if ((irq1 & irq2) == 0xff) {
		ZetSetIRQLine(0, ZET_IRQSTATUS_NONE);
	} else	{
		if (irq2 == 0xdf) {
			ZetSetVector(irq1 & irq2);
			ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);
		}
	}
}

unsigned char seibu_main_word_read(int offset)
{
	offset = (offset >> 1) & 7;

	switch (offset)
	{
		case 2:
		case 3:
			return sub2main[offset-2];
		case 5:
			return main2sub_pending ? 1 : 0;
		default:
			return 0xff;
	}
}

void seibu_main_word_write(int offset, unsigned char data)
{
	offset = (offset >> 1) & 7;

	switch (offset)
	{
		case 0:
		case 1:
			main2sub[offset] = data;
			break;

		case 4:
			if (is_sdgndmps) update_irq_lines(RST10_ASSERT);
			update_irq_lines(RST18_ASSERT);
			break;

		case 6:
			sub2main_pending = 0;
			main2sub_pending = 1;
			break;

		default:
			break;
	}
}

void seibu_sound_mustb_write_word(int /*offset*/, unsigned char data)
{
	main2sub[0] = data & 0xff;
	main2sub[1] = data >> 8;
	
	update_irq_lines(RST18_ASSERT);
}

static void seibu_z80_bank(int data)
{
	SeibuSoundBank = data & 1;

	ZetMapArea(0x8000, 0xffff, 0, SeibuZ80ROM + 0x10000 + (data & 1) * 0x8000);
	ZetMapArea(0x8000, 0xffff, 2, SeibuZ80ROM + 0x10000 + (data & 1) * 0x8000);
}

void __fastcall seibu_sound_write(unsigned short address, unsigned char data)
{
	switch (address)
	{
		case 0x4000:
			main2sub_pending = 0;
			sub2main_pending = 1;
		return;

		case 0x4001:
			update_irq_lines(VECTOR_INIT);
		return;

		case 0x4002:
			// rst10 ack (unused)
		return;

		case 0x4003:
			update_irq_lines(RST18_CLEAR);
		return;

		case 0x4007:
			seibu_z80_bank(data);
		return;

		case 0x4008:
			switch (seibu_snd_type & 3)
			{
				case 0:
					BurnYM3812Write(0, data);
				return;

				case 1:
					BurnYM2151SelectRegister(data);
				return;

				case 2:
					BurnYM2203Write(0, 0, data);
				return;
			}
		return;

		case 0x4009:
			switch (seibu_snd_type & 3)
			{
				case 0:
					BurnYM3812Write(1, data);
				return;

				case 1:
					BurnYM2151WriteRegister(data);
				return;

				case 2:
					BurnYM2203Write(0, 1, data);
				return;
			}
		return;

		case 0x4018:
		case 0x4019:
			sub2main[address & 1] = data;
		return;

		case 0x401b:
			// coin counters
		return;

		case 0x6000:
			MSM6295Command(0, data);
		return;

		case 0x6002:
			if (seibu_snd_type & 4) MSM6295Command(1, data);
		return;

		// type 2
		case 0x6008:
		case 0x6009:
			if (seibu_snd_type == 2) BurnYM2203Write(1, address & 1, data);
		return;
	}
}

unsigned char __fastcall seibu_sound_read(unsigned short address)
{
	switch (address)
	{
		case 0x4008:
			switch (seibu_snd_type & 3)
			{
				case 0:
					return BurnYM3812Read(0);

				case 1:
					return BurnYM2151ReadStatus();

				case 2:
					return BurnYM2203Read(0, 0);
			}
			return 0;

		case 0x4009: {
			if (seibu_snd_type < 2) return 0;
			return BurnYM2203Read(0, 1);
		}

		case 0x4010:
		case 0x4011:
			return main2sub[address & 1];

		case 0x4012:
			return sub2main_pending ? 1 : 0;

		case 0x4013:
			return seibu_coin_input;

		case 0x6000:
			return MSM6295ReadStatus(0);

		case 0x6002:
			if (seibu_snd_type & 4) return MSM6295ReadStatus(0);
	}

	return 0;
}

static unsigned char decrypt_data(int a,int src)
{
	if ( BIT(a,9)  &  BIT(a,8))             src ^= 0x80;
	if ( BIT(a,11) &  BIT(a,4) &  BIT(a,1)) src ^= 0x40;
	if ( BIT(a,11) & ~BIT(a,8) &  BIT(a,1)) src ^= 0x04;
	if ( BIT(a,13) & ~BIT(a,6) &  BIT(a,4)) src ^= 0x02;
	if (~BIT(a,11) &  BIT(a,9) &  BIT(a,2)) src ^= 0x01;

	if (BIT(a,13) &  BIT(a,4)) src = BITSWAP08(src,7,6,5,4,3,2,0,1);
	if (BIT(a, 8) &  BIT(a,4)) src = BITSWAP08(src,7,6,5,4,2,3,1,0);

	return src;
}

static unsigned char decrypt_opcode(int a,int src)
{
	if ( BIT(a,9)  &  BIT(a,8))             src ^= 0x80;
	if ( BIT(a,11) &  BIT(a,4) &  BIT(a,1)) src ^= 0x40;
	if (~BIT(a,13) & BIT(a,12))             src ^= 0x20;
	if (~BIT(a,6)  &  BIT(a,1))             src ^= 0x10;
	if (~BIT(a,12) &  BIT(a,2))             src ^= 0x08;
	if ( BIT(a,11) & ~BIT(a,8) &  BIT(a,1)) src ^= 0x04;
	if ( BIT(a,13) & ~BIT(a,6) &  BIT(a,4)) src ^= 0x02;
	if (~BIT(a,11) &  BIT(a,9) &  BIT(a,2)) src ^= 0x01;

	if (BIT(a,13) &  BIT(a,4)) src = BITSWAP08(src,7,6,5,4,3,2,0,1);
	if (BIT(a, 8) &  BIT(a,4)) src = BITSWAP08(src,7,6,5,4,2,3,1,0);
	if (BIT(a,12) &  BIT(a,9)) src = BITSWAP08(src,7,6,4,5,3,2,1,0);
	if (BIT(a,11) & ~BIT(a,6)) src = BITSWAP08(src,6,7,5,4,3,2,1,0);

	return src;
}

static void seibu_sound_decrypt(int length)
{
	for (int i = 0; i < length; i++)
	{
		unsigned char src = SeibuZ80ROM[i];

		SeibuZ80ROM[i] = decrypt_data(i,src);
		SeibuZ80DecROM[i] = decrypt_opcode(i,src);
	}
}

static int DrvSynchroniseStream(int nSoundRate)
{
	return (long long)ZetTotalCycles() * nSoundRate / seibu_sndcpu_frequency;
}

static void DrvFMIRQHandler(int, int nStatus)
{
	if (nStatus) {
		update_irq_lines(RST10_ASSERT);
	} else {
		update_irq_lines(RST10_CLEAR);
	}
}

static void Drv2151FMIRQHandler(int nStatus)
{
	DrvFMIRQHandler(0, nStatus);
}

static double Drv2203GetTime()
{
	return (double)ZetTotalCycles() / seibu_sndcpu_frequency;
}

void seibu_sound_reset()
{
	ZetOpen(0);
	ZetReset();
	update_irq_lines(VECTOR_INIT);
	ZetClose();

	switch (seibu_snd_type & 3)
	{
		case 0:
			BurnYM3812Reset();
		break;

		case 1:
			BurnYM2151Reset();
		break;

		case 2:
			BurnYM2203Reset();
		break;
	}

	MSM6295Reset(0);
	if (seibu_snd_type & 4) MSM6295Reset(1);

	memset (main2sub, 0, 2);
	memset (sub2main, 0, 2);
	main2sub_pending = 0;
	sub2main_pending = 0;

	seibu_coin_input = 0;
	SeibuSoundBank = 0;
}

void seibu_sound_init(int type, int len, int freq0 /*cpu*/, int freq1 /*ym*/, int freq2 /*oki*/)
{
	seibu_snd_type = type;

	if (len && SeibuZ80DecROM != NULL) {
		seibu_sound_decrypt(len);
	} else {
		SeibuZ80DecROM = SeibuZ80ROM;
	}

	seibu_sndcpu_frequency	= freq0;

	ZetInit(1);
	ZetOpen(0);
	ZetMapArea(0x0000, 0x1fff, 0, SeibuZ80ROM);
	ZetMapArea(0x0000, 0x1fff, 2, SeibuZ80DecROM, SeibuZ80ROM);
	ZetMapArea(0x2000, 0x27ff, 0, SeibuZ80RAM);
	ZetMapArea(0x2000, 0x27ff, 1, SeibuZ80RAM);
	ZetMapArea(0x2000, 0x27ff, 2, SeibuZ80RAM);
	ZetSetWriteHandler(seibu_sound_write);
	ZetSetReadHandler(seibu_sound_read);
	ZetMemEnd();
	ZetClose();

	switch (seibu_snd_type & 3)
	{
		case 0:
			BurnYM3812Init(freq1, &DrvFMIRQHandler, &DrvSynchroniseStream, 0);
			BurnTimerAttachZetYM3812(freq0);
		break;

		case 1:
			BurnYM2151Init(freq1, 100.0);
			BurnYM2151SetIrqHandler(&Drv2151FMIRQHandler);
		break;

		case 2:
			BurnYM2203Init(2, freq1, DrvFMIRQHandler, DrvSynchroniseStream, Drv2203GetTime, 0);
			BurnTimerAttachZet(freq0);
		break;
	}

	MSM6295Init(0, freq2, 100.0, 1);
	if (seibu_snd_type & 4) MSM6295Init(1, freq2, 100.0, 1);

	// init kludge for sdgndmps
	if (!strcmp(BurnDrvGetTextA(DRV_NAME), "sdgndmps")) {
		is_sdgndmps = 1;
	}
}

void seibu_sound_exit()
{
	switch (seibu_snd_type & 3)
	{
		case 0:
			BurnYM3812Exit();
		break;

		case 1:
			BurnYM2151Exit();
		break;

		case 2:
			BurnYM2203Exit();
		break;
	}
	
	MSM6295Exit(0);
	if (seibu_snd_type & 4) MSM6295Exit(1);

	ZetExit();

	MSM6295ROM = NULL;

	SeibuZ80DecROM = NULL;
	SeibuZ80ROM = NULL;
	SeibuZ80RAM = NULL;
	seibu_sndcpu_frequency = 0;
	is_sdgndmps = 0;
}

void seibu_sound_update(short *pbuf, int nLen)
{
	switch (seibu_snd_type & 3)
	{
		case 0:
			BurnYM3812Update(pbuf, nLen);
		break;

		case 1:
			BurnYM2151Render(pbuf, nLen);
		break;

		case 2:
			BurnYM2203Update(pbuf, nLen);
		break;
	}

	if (seibu_snd_type & 4) 
		MSM6295Render(1, pBurnSoundOut, nBurnSoundLen);
	MSM6295Render(0, pBurnSoundOut, nBurnSoundLen);
}

void seibu_sound_scan(int *pnMin, int nAction)
{
	if (nAction & ACB_VOLATILE)
	{		
		ZetScan(nAction);

		switch (seibu_snd_type & 3)
		{
			case 0:
				BurnYM3812Scan(nAction, pnMin);
			break;
	
			case 1:
				BurnYM2203Scan(nAction, pnMin);
			break;
	
			case 2:
				BurnYM2151Scan(nAction);
			break;
		}
		
		MSM6295Scan(0, nAction);
		if (seibu_snd_type & 4) {
			MSM6295Scan(1, nAction);
		}

		SCAN_VAR(main2sub[0]);
		SCAN_VAR(main2sub[1]);
		SCAN_VAR(sub2main[0]);
		SCAN_VAR(sub2main[1]);
		SCAN_VAR(main2sub_pending);
		SCAN_VAR(sub2main_pending);
		SCAN_VAR(SeibuSoundBank);
	}

	if (nAction & ACB_WRITE)
	{
		ZetOpen(0);
		seibu_z80_bank(SeibuSoundBank);
		ZetClose();
	}
}
			
