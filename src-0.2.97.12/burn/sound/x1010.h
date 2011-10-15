#define SETA_NUM_CHANNELS	16
#define SETA_NUM_BANKS 		(0x100000 / 0x20000)

#define FREQ_BASE_BITS		8					// Frequency fixed decimal shift bits
#define ENV_BASE_BITS		 16					// wave form envelope fixed decimal shift bits
#define	VOL_BASE	(2 * 32 * 256 / 30)			// Volume base

struct x1_010_info
{
	int	rate;									// Output sampling rate (Hz)
	//sound_stream *	stream;					// Stream handle
	int	address;								// address eor data
	int	sound_enable;							// sound output enable/disable
	unsigned char reg[0x2000];					// X1-010 Register & wave form area
	unsigned char HI_WORD_BUF[0x2000];			// X1-010 16bit access ram check avoidance work
	unsigned int smp_offset[SETA_NUM_CHANNELS];
	unsigned int env_offset[SETA_NUM_CHANNELS];
	unsigned int base_clock;
	unsigned int sound_banks[SETA_NUM_BANKS];
};

typedef struct {
	unsigned char	status;
	unsigned char	volume;					// volume / wave form no.
	unsigned char	frequency;			// frequency / pitch lo
	unsigned char	pitch_hi;				// reserved / pitch hi
	unsigned char	start;					// start address / envelope time
	unsigned char	end;						// end address / envelope no.
	unsigned char	reserve[2];
} X1_010_CHANNEL;

extern unsigned char *X1010SNDROM;

extern struct x1_010_info * x1_010_chip;

void x1010_sound_bank_w(unsigned int offset, unsigned short data);
unsigned char x1010_sound_read(unsigned int offset);
unsigned short x1010_sound_read_word(unsigned int offset);
void x1010_sound_update();
void x1010_sound_init(unsigned int base_clock, int address);
void x1010_scan(int nAction,int *pnMin);
void x1010_exit();
