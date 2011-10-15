// konamiic.cpp
//---------------------------------------------------------------------------------------------------------------
extern unsigned int KonamiIC_K051960InUse;
extern unsigned int KonamiIC_K052109InUse;
extern unsigned int KonamiIC_K051316InUse;
extern unsigned int KonamiIC_K053245InUse;
extern unsigned int KonamiIC_K053247InUse;
extern unsigned int KonamiIC_K053936InUse;

extern unsigned short *konami_temp_screen;
void KonamiBlendCopy(unsigned int *palette /* 32-bit color */, unsigned int *drvpalette /* n-bit color */);

extern int K05324xZRejection;
void K05324xSetZRejection(int z);

void KonamiICReset();
void KonamiICExit();
void KonamiICScan(int nAction);

void konami_rom_deinterleave_2(unsigned char *src, int len);
void konami_rom_deinterleave_4(unsigned char *src, int len);

void KonamiRecalcPal(unsigned char *src, unsigned int *dst, int len);


void K052109_051960_w(int offset, int data);
unsigned char K052109_051960_r(int offset);

// k051960.cpp
//---------------------------------------------------------------------------------------------------------------
extern int K051960ReadRoms;
extern int K052109_irq_enabled;
void K051960SpritesRender(unsigned char *pSrc, int priority);
unsigned char K0519060FetchRomData(unsigned int Offset);
unsigned char K051960Read(unsigned int Offset);
void K051960Write(unsigned int Offset, unsigned char Data);
void K051960SetCallback(void (*Callback)(int *Code, int *Colour, int *Priority, int *Shadow));
void K051960SetSpriteOffset(int x, int y);
void K051960Reset();
void K051960Init(unsigned char* pRomSrc, unsigned int RomMask);
void K051960Exit();
void K051960Scan(int nAction);
void K051937Write(unsigned int Offset, unsigned char Data);
unsigned char K051937Read(unsigned int Offset);

// k052109.cpp
//---------------------------------------------------------------------------------------------------------------
extern int K052109RMRDLine;
extern int K051960_irq_enabled;
extern int K051960_nmi_enabled;
extern int K051960_spriteflip;

void K052109UpdateScroll();
void K052109AdjustScroll(int x, int y);
void K052109RenderLayer(int nLayer, int Opaque, unsigned char *pSrc);
unsigned char K052109Read(unsigned int Offset);
void K052109Write(unsigned int Offset, unsigned char Data);
void K052109SetCallback(void (*Callback)(int Layer, int Bank, int *Code, int *Colour, int *xFlip, int *priority));
void K052109Reset();
void K052109Init(unsigned char *pRomSrc, unsigned int RomMask);
void K052109Exit();
void K052109Scan(int nAction);

#define K051960ByteRead(nStartAddress)						\
if (a >= nStartAddress && a <= nStartAddress + 0x3ff) {				\
	return K051960Read(a - nStartAddress);					\
}

#define K051960ByteWrite(nStartAddress)						\
if (a >= nStartAddress && a <= nStartAddress + 0x3ff) {				\
	K051960Write((a - nStartAddress), d);					\
	return;									\
}

#define K051960WordWrite(nStartAddress)						\
if (a >= nStartAddress && a <= nStartAddress + 0x3ff) {				\
	if (a & 1) {								\
		K051960Write((a - nStartAddress) + 1, d & 0xff);		\
	} else {								\
		K051960Write((a - nStartAddress) + 0, (d >> 8) & 0xff);		\
	}									\
	return;									\
}

#define K051937ByteRead(nStartAddress)						\
if (a >= nStartAddress && a <= nStartAddress + 7) {				\
	int Offset = (a - nStartAddress);					\
										\
	if (Offset == 0) {							\
		static int Counter;						\
		return (Counter++) & 1;						\
	}									\
										\
	if (K051960ReadRoms && (Offset >= 0x04 && Offset <= 0x07)) {		\
		return K0519060FetchRomData(Offset & 3);			\
	}									\
										\
	return 0;								\
}

#define K015937ByteWrite(nStartAddress)						\
if (a >= nStartAddress && a <= nStartAddress + 7) {				\
	K051937Write((a - nStartAddress), d);					\
	return;									\
}

#define K052109WordNoA12Read(nStartAddress)					\
if (a >= nStartAddress && a <= nStartAddress + 0x7fff) {			\
	int Offset = (a - nStartAddress) >> 1;					\
	Offset = ((Offset & 0x3000) >> 1) | (Offset & 0x07ff);			\
										\
	if (a & 1) {								\
		return K052109Read(Offset + 0x2000);				\
	} else {								\
		return K052109Read(Offset + 0x0000);				\
	}									\
}


#define K052109WordNoA12Write(nStartAddress)					\
if (a >= nStartAddress && a <= nStartAddress + 0x7fff) {			\
	int Offset = (a - nStartAddress) >> 1;					\
	Offset = ((Offset & 0x3000) >> 1) | (Offset & 0x07ff);			\
										\
	if (a & 1) {								\
		K052109Write(Offset + 0x2000, d);				\
	} else {								\
		K052109Write(Offset + 0x0000, d);				\
	}									\
	return;									\
}

// K051316.cpp
//---------------------------------------------------------------------------------------------------------------
void K051316Init(int chip, unsigned char *gfx, unsigned char *gfxexp, int mask, void (*callback)(int *code,int *color,int *flags), int bpp, int transp);
void K051316Reset();
void K051316Exit();

void K051316RedrawTiles(int chip);

unsigned char K051316ReadRom(int chip, int offset);
unsigned char K051316Read(int chip, int offset);
void K051316Write(int chip, int offset, int data);

void K051316WriteCtrl(int chip, int offset, int data);
void K051316WrapEnable(int chip, int status);
void K051316SetOffset(int chip, int xoffs, int yoffs);
void K051316_zoom_draw(int chip, int flags);
void K051316Scan(int nAction);

// K053245.cpp
//---------------------------------------------------------------------------------------------------------------
int K053245Reset();
void K053245GfxDecode(unsigned char *src, unsigned char *dst, int len);
void K053245Init(int chip, unsigned char *gfx, int mask, void (*callback)(int *code,int *color,int *priority));
void K053245Exit();

void K053245SpritesRender(int chip, unsigned char *gfxdata, int priority);

void K053245SetSpriteOffset(int chip,int offsx, int offsy);
void K053245ClearBuffer(int chip);
void K053245UpdateBuffer(int chip);
void K053244BankSelect(int chip, int bank);

unsigned short K053245ReadWord(int chip, int offset);
void K053245WriteWord(int chip, int offset, int data);

unsigned char K053245Read(int chip, int offset);
void K053245Write(int chip, int offset, int data);
unsigned char K053244Read(int chip, int offset);
void K053244Write(int chip, int offset, int data);

void K053245Scan(int nAction);

// K053251.cpp
//---------------------------------------------------------------------------------------------------------------
void K053251Reset();

void K053251Write(int offset, int data);

int K053251GetPriority(int idx);
int K053251GetPaletteIndex(int idx);

void K053251Write(int offset, int data);
int K053251GetPriority(int idx);
int K053251GetPaletteIndex(int idx);

void K053251Scan(int nAction);

// K053247.cpp
//---------------------------------------------------------------------------------------------------------------
void K053247Reset();
void K053247Init(unsigned char *gfxrom, int gfxlen, void (*Callback)(int *code, int *color, int *priority), int flags);
void K053247Exit();
void K053247Scan(int nAction);

void K053247Export(unsigned char **ram, unsigned char **gfx, void (**callback)(int *, int *, int *), int *dx, int *dy);
void K053247GfxDecode(unsigned char *src, unsigned char *dst, int len); // 16x16
void K053247SetSpriteOffset(int offsx, int offsy);
void K053247WrapEnable(int status);

void K053246_set_OBJCHA_line(int state); // 1 assert, 0 clear
int K053246_is_IRQ_enabled();

unsigned char K053247Read(int offset);
void K053247Write(int offset, int data);
unsigned char K053246Read(int offset);
void K053246Write(int offset, int data);

void K053247SpritesRender(unsigned char *gfxbase, int priority);

// k054000.cpp
//------------------------------------------------------------------------------------------
void K054000Reset();
void K054000Write(int offset, int data);
unsigned char K054000Read(int address);
void K054000Scan(int nAction);

// K051733.cpp
//------------------------------------------------------------------------------------------
void K051733Reset();
void K051733Write(int offset, int data);
unsigned char K051733Read(int offset);
void K051733Scan(int nAction);

// K053936.cpp
//------------------------------------------------------------------------------------------
void K053936Init(int chip, unsigned char *ram, int len, int w, int h, void (*pCallback)(int offset, unsigned short *ram, int *code, int *color, int *sx, int *sy, int *fx, int *fy));
void K053936Reset();
void K053936Exit();
void K053936Scan(int nAction);

void K053936EnableWrap(int chip, int status);
void K053936SetOffset(int chip, int xoffs, int yoffs);

void K053936PredrawTiles(int chip, unsigned char *gfx, int transparent, int tcol /*transparent color*/);
void K053936Draw(int chip, unsigned short *ctrl, unsigned short *linectrl, int transp);

