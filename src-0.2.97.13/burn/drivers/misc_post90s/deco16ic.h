
// deco16 tilemap routines

extern unsigned short *deco16_pf_control[2];
extern unsigned char *deco16_pf_ram[4];
extern unsigned char *deco16_pf_rowscroll[4];

extern unsigned short deco16_priority;

void deco16_set_bank_callback(int tmap, int (*callback)(const int bank));
void deco16_set_color_base(int tmap, int base);
void deco16_set_color_mask(int tmap, int mask);
void deco16_set_transparency_mask(int tmap, int mask);
void deco16_set_gfxbank(int tmap, int small, int big);
void deco16_set_global_offsets(int x, int y);

void deco16_set_scroll_offs(int tmap, int size, int offsetx, int offsety);

int deco16_get_tilemap_size(int tmap);

extern unsigned char *deco16_prio_map;
void deco16_clear_prio_map();
void deco16_draw_prio_sprite(unsigned short *dest, unsigned char *gfx, int code, int color, int sx, int sy, int flipx, int flipy, int pri);
void deco16_draw_prio_sprite(unsigned short *dest, unsigned char *gfx, int code, int color, int sx, int sy, int flipx, int flipy, int pri, int spri);
void deco16_draw_alphaprio_sprite(unsigned int *palette, unsigned char *gfx, int code, int color, int sx, int sy, int flipx, int flipy, int pri, int spri, int alpha);

void deco16_set_graphics(unsigned char *gfx0, int len0, unsigned char *gfx1, int len1, unsigned char *gfx2, int len2);
void deco16_set_graphics(int num, unsigned char *gfx, int len, int size /*tile size*/); // individual bank

void deco16Init(int no_pf34, int split, int full_width);
void deco16Reset();
void deco16Exit();

void deco16Scan();

void deco16_pf12_update();
void deco16_pf34_update();

#define DECO16_LAYER_OPAQUE		0x010000
#define DECO16_LAYER_PRIORITY(x)	((x) & 0xff)
#define DECO16_LAYER_8BITSPERPIXEL	0x100000
#define DECO16_LAYER_5BITSPERPIXEL	0x200000
#define DECO16_LAYER_4BITSPERPIXEL	0x000000 	// just to clarify
#define DECO16_LAYER_TRANSMASK0		0x000100
#define DECO16_LAYER_TRANSMASK1		0x000000

void deco16_draw_layer(int tmap, unsigned short *dest, int flags);

void deco16_tile_decode(unsigned char *src, unsigned char *dst, int len, int type);
void deco16_sprite_decode(unsigned char *gfx, int len);

void deco16_palette_recalculate(unsigned int *palette, unsigned char *pal);

#define deco16_write_control_word(num, addr, a, d)		\
	if ((addr & 0xfffffff0) == a) {				\
		deco16_pf_control[num][(addr & 0x0f)/2] = d;	\
		return;						\
	}


#define deco16_read_control_word(num, addr, a)			\
	if ((addr & 0xfffffff0) == a) {				\
		return deco16_pf_control[num][(addr & 0x0f)/2];	\
	}


#define deco16ic_71_read()	(0xffff)

// common sound hardware...

extern int deco16_soundlatch;

void deco16SoundReset();
void deco16SoundInit(unsigned char *rom, unsigned char *ram, int huc_clock, int ym2203, void (ym2151_port)(unsigned int,unsigned int), double ym2151vol, int msmclk0, double msmvol0, int msmclk1, double msmvol1);
void deco16SoundExit();
void deco16SoundUpdate(short *buf, int len);


// decrypt routines

void deco56_decrypt_gfx(unsigned char *rom, int len);
void deco74_decrypt_gfx(unsigned char *rom, int len);
void deco56_remap_gfx(unsigned char *rom, int len);

void deco102_decrypt_cpu(unsigned char *data, unsigned char *ops, int size, int address_xor, int data_select_xor, int opcode_select_xor);

void deco156_decrypt(unsigned char *src, int len);

// protection routines

extern unsigned short *deco16_prot_ram;
extern unsigned short *deco16_prot_inputs;
extern unsigned short *deco16_buffer_ram;
extern int deco16_vblank;

void deco16_66_prot_w(int offset, unsigned short data, int mask); // mutant fighter
unsigned short deco16_66_prot_r(int offset);

void deco16_60_prot_w(int offset, unsigned short data, int mask); // edward randy
unsigned short deco16_60_prot_r(int offset);

unsigned short deco16_104_cninja_prot_r(int offset); // caveman ninja

unsigned short deco16_146_funkyjet_prot_r(int offset); // funky jet

void deco16_104_rohga_prot_w(int offset, unsigned short data, int mask); // rohga
unsigned short deco16_104_rohga_prot_r(int offset);

unsigned short deco16_104_prot_r(int offset);

void deco16_146_nitroball_prot_w(int offset, unsigned short data, int mask);
unsigned short deco16_146_nitroball_prot_r(int offset);

void deco16_146_fghthist_prot_w(int offset, unsigned int data, unsigned int mem_mask);
unsigned int deco16_146_fghthist_prot_r(int offset);

