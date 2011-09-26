// Biomechanical Toy & Maniac Square

#include "burnint.h"
#include "msm6295.h"


#define spritelistentries 512

static unsigned char *Mem=NULL,*MemEnd=NULL;
static unsigned char *RamStart=NULL,*RamEnd=NULL;
static unsigned char *Rom=NULL,*Ram=NULL,*VideoRam=NULL,*SpriteRam=NULL,*PaletteRam=NULL,*ScreenRam=NULL;
static unsigned char drvReset = 0;

unsigned char *bmsprites=NULL;
unsigned char *spritetemp2=NULL;
static unsigned int VidRegs[5] = {0,0,0,0,0};
//static unsigned char *pTileData;
static int Scroll1X;
static int Scroll1Y;
static int Scroll2X;
static int Scroll2Y;
static int SndBank;
static int nCyclesTotal[1];

static unsigned char  BiomtoyInputPort0[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static unsigned char  BiomtoyInputPort1[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static unsigned char  BiomtoyDip[2]        = {0, 0};
static unsigned short BiomtoyInput[2]      = {0x00,0x00};

static unsigned int *LocalPalette=NULL;

struct g_sprite
{
	int number;
};

static g_sprite* pSpriteList[5] = {NULL,NULL,NULL,NULL,NULL};
static int spritelistcount[5] = {0,0,0,0,0};


static struct BurnInputInfo BiomtoyInputList[]=
{
	{"Coin 1"            , BIT_DIGITAL  , BiomtoyInputPort0 + 6, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL  , BiomtoyInputPort1 + 6, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL  , BiomtoyInputPort0 + 7, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL  , BiomtoyInputPort1 + 7, "p2 start"  },

	{"P1 Up"             , BIT_DIGITAL  , BiomtoyInputPort0 + 0, "p1 up"     },
	{"P1 Down"           , BIT_DIGITAL  , BiomtoyInputPort0 + 1, "p1 down"   },
	{"P1 Left"           , BIT_DIGITAL  , BiomtoyInputPort0 + 3, "p1 left"   },
	{"P1 Right"          , BIT_DIGITAL  , BiomtoyInputPort0 + 2, "p1 right"  },
	{"P1 Fire 1"         , BIT_DIGITAL  , BiomtoyInputPort0 + 5, "p1 fire 1" },
	{"P1 Fire 2"         , BIT_DIGITAL  , BiomtoyInputPort0 + 4, "p1 fire 2" },

	{"P2 Up"             , BIT_DIGITAL  , BiomtoyInputPort1 + 0, "p2 up"     },
	{"P2 Down"           , BIT_DIGITAL  , BiomtoyInputPort1 + 1, "p2 down"   },
	{"P2 Left"           , BIT_DIGITAL  , BiomtoyInputPort1 + 3, "p2 left"   },
	{"P2 Right"          , BIT_DIGITAL  , BiomtoyInputPort1 + 2, "p2 right"  },
	{"P2 Fire 1"         , BIT_DIGITAL  , BiomtoyInputPort1 + 5, "p2 fire 1" },
	{"P2 Fire 2"         , BIT_DIGITAL  , BiomtoyInputPort1 + 4, "p2 fire 2" },

	{"Reset"			 , BIT_DIGITAL	, &drvReset			   , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH, BiomtoyDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH, BiomtoyDip + 1       , "dip"       },
};

STDINPUTINFO(Biomtoy)

static struct BurnDIPInfo BiomtoyDIPList[]=
{
	// Defaults
	{0x11, 0xff, 0xff, 0xff, NULL},
	{0x12, 0xff, 0xff, 0xff, NULL},

	// DIP SW1
	{0,		0xfe, 0, 12,	  "Coin 1"},
	{0x11, 0x01, 0xf0, 0xe0, "4 coins 1 credit"},
	{0x11, 0x01, 0xf0, 0x10, "3 coins 1 credit"},
	{0x11, 0x01, 0xf0, 0x90, "2 coins 1 credit"},
	{0x11, 0x01, 0xf0, 0xa0, "3 coins 2 credits"},
	{0x11, 0x01, 0xf0, 0xf0, "1 coin 1 credit"},
	{0x11, 0x01, 0xf0, 0x60, "2 coins 3 credits"},
	{0x11, 0x01, 0xf0, 0x70, "1 coin 2 credits"},
	{0x11, 0x01, 0xf0, 0xb0, "1 coin 3 credits"},
	{0x11, 0x01, 0xf0, 0x30, "1 coin 4 credits"},
	{0x11, 0x01, 0xf0, 0xd0, "1 coin 5 credits"},
	{0x11, 0x01, 0xf0, 0x50, "1 coin 6 credits"},
	{0x11, 0x01, 0xf0, 0x00, "Free play"},

	{0,		0xfe, 0, 12,	  "Coin 2"},
	{0x11, 0x01, 0x0f, 0x0e, "4 coins 1 credit"},
	{0x11, 0x01, 0x0f, 0x01, "3 coins 1 credit"},
	{0x11, 0x01, 0x0f, 0x09, "2 coins 1 credit"},
	{0x11, 0x01, 0x0f, 0x0a, "3 coins 2 credits"},
	{0x11, 0x01, 0x0f, 0x0f, "1 coin 1 credit"},
	{0x11, 0x01, 0x0f, 0x06, "2 coins 3 credits"},
	{0x11, 0x01, 0x0f, 0x07, "1 coin 2 credits"},
	{0x11, 0x01, 0x0f, 0x0b, "1 coin 3 credits"},
	{0x11, 0x01, 0x0f, 0x03, "1 coin 4 credits"},
	{0x11, 0x01, 0x0f, 0x0d, "1 coin 5 credits"},
	{0x11, 0x01, 0x0f, 0x05, "1 coin 6 credits"},
	{0x11, 0x01, 0x0f, 0x00, "Free play"},

	// DIP SW2
	{0,		0xfe, 0,	4,	  "Difficulty"},
	{0x12, 0x01, 0xc0, 0x40, "Easy"},
	{0x12, 0x01, 0xc0, 0xc0, "Normal"},
	{0x12, 0x01, 0xc0, 0x80, "Hard"},
	{0x12, 0x01, 0xc0, 0x00, "Hardest"},

	{0,		0xfe, 0,	4,	  "Lives"},
	{0x12, 0x01, 0x30, 0x20, "0"},
	{0x12, 0x01, 0x30, 0x10, "1"},
	{0x12, 0x01, 0x30, 0x30, "2"},
	{0x12, 0x01, 0x30, 0x00, "3"},

	{0,		0xfe, 0,	2,	  "Advertise sound"},
	{0x12, 0x01, 0x08, 0x00, "No"},
	{0x12, 0x01, 0x08, 0x08, "Yes"},

	{0,		0xfe, 0,	2,	  "Test mode"},
	{0x12, 0x01, 0x01, 0x01, "No"},
	{0x12, 0x01, 0x01, 0x00, "Yes"},
};

STDDIPINFO(Biomtoy)

static struct BurnDIPInfo ManiacspDIPList[]=
{
	// Defaults
	{0x11, 0xff, 0xff, 0xff, NULL},
	{0x12, 0xff, 0xff, 0xf7, NULL},

	// DIP SW1
	{0,		0xfe, 0, 11,	  "Coin 1"},
	{0x11, 0x01, 0xf0, 0xe0, "4 coins 1 credit"},
	{0x11, 0x01, 0xf0, 0x10, "3 coins 1 credit"},
	{0x11, 0x01, 0xf0, 0x90, "2 coins 1 credit"},
	{0x11, 0x01, 0xf0, 0xa0, "3 coins 2 credits"},
	{0x11, 0x01, 0xf0, 0xf0, "1 coin 1 credit"},
	{0x11, 0x01, 0xf0, 0x60, "2 coins 3 credits"},
	{0x11, 0x01, 0xf0, 0x70, "1 coin 2 credits"},
	{0x11, 0x01, 0xf0, 0xb0, "1 coin 3 credits"},
	{0x11, 0x01, 0xf0, 0x30, "1 coin 4 credits"},
	{0x11, 0x01, 0xf0, 0xd0, "1 coin 5 credits"},
	{0x11, 0x01, 0xf0, 0x50, "1 coin 6 credits"},
	{0x11, 0x01, 0xff, 0x00, "Free play"}, // doesn't work

	{0,		0xfe, 0, 11,	  "Coin 2"},
	{0x11, 0x01, 0x0f, 0x0e, "4 coins 1 credit"},
	{0x11, 0x01, 0x0f, 0x01, "3 coins 1 credit"},
	{0x11, 0x01, 0x0f, 0x09, "2 coins 1 credit"},
	{0x11, 0x01, 0x0f, 0x0a, "3 coins 2 credits"},
	{0x11, 0x01, 0x0f, 0x0f, "1 coin 1 credit"},
	{0x11, 0x01, 0x0f, 0x06, "2 coins 3 credits"},
	{0x11, 0x01, 0x0f, 0x07, "1 coin 2 credits"},
	{0x11, 0x01, 0x0f, 0x0b, "1 coin 3 credits"},
	{0x11, 0x01, 0x0f, 0x03, "1 coin 4 credits"},
	{0x11, 0x01, 0x0f, 0x0d, "1 coin 5 credits"},
	{0x11, 0x01, 0x0f, 0x05, "1 coin 6 credits"},
	{0x11, 0x01, 0xff, 0x00, "Free play"}, // doesn't work

	// DIP SW2
	{0,		0xfe, 0,	4,	  "Game difficulty"},
	{0x12, 0x01, 0xc0, 0x40, "Easy"},
	{0x12, 0x01, 0xc0, 0xc0, "Normal"},
	{0x12, 0x01, 0xc0, 0x80, "Hard"},
	{0x12, 0x01, 0xc0, 0x00, "Hardest"},

	{0,		0xfe, 0,	2,	  "Advertise sound"},
	{0x12, 0x01, 0x04, 0x00, "No"},
	{0x12, 0x01, 0x04, 0x04, "Yes"},

	{0,		0xfe, 0,	2,	  "Test mode"},
	{0x12, 0x01, 0x01, 0x01, "No"},
	{0x12, 0x01, 0x01, 0x00, "Yes"},

	{0,		0xfe, 0,	2,	  "Sound"},
	{0x12, 0x01, 0x08, 0x00, "Stereo"},
	{0x12, 0x01, 0x08, 0x08, "Mono"},
};

STDDIPINFO(Maniacsp)


inline void BiomtoyMakeInputs()
{
	// Reset Inputs
	BiomtoyInput[0] = BiomtoyInput[1] = 0x3fff;

	// Compile Digital Inputs
	for (int i = 0; i < 8; i++) {
		BiomtoyInput[0] -= (BiomtoyInputPort0[i] & 1) << i;
	}

	for (int i = 0; i < 8; i++) {
		BiomtoyInput[1] -= (BiomtoyInputPort1[i] & 1) << i;
	}
}

// Pal routines
static inline unsigned int CalcCol(unsigned short nColour)
{
	int r, g, b;
	nColour = swapWord(nColour);
	r = (nColour & 0x001F) << 3;	// Red
	r |= r >> 5;
	g = (nColour & 0x03E0) >> 2;  	// Green
	g |= g >> 5;
	b = (nColour & 0x7C00) >> 7;	// Blue
	b |= b >> 5;

	return BurnHighCol(r, g, b, 0);
}

int BiomtoyCalcPalette()
{
	int i;
	unsigned short* ps;
	unsigned int* pd;

	for (i = 0, ps = (unsigned short*)PaletteRam, pd = LocalPalette; i < 0x400; i++, ps++, pd++) {
		*pd = CalcCol(*ps);
	}

	return 0;
}

void init_spritelist()
{
	for (int i=0; i<5;i++ )
	{
		pSpriteList[i]=(g_sprite*)malloc(spritelistentries*sizeof(g_sprite));
		memset(pSpriteList[i],0,sizeof(pSpriteList[i]));
	}
}

void exit_spritelist()
{
	for (int i=0; i<5; i++)
	{
		free(pSpriteList[i]);
	}
}


inline void spritetest(int Number, int Startx, int Starty, int Colour, bool flipx,bool flipy)
{
	int x, y, tempCol, sx, sy;
	unsigned char *temp;

	if (flipx&&flipy)
	{
		Startx+=7;
		Starty+=7;
		for (y = 0; y < 8; y++) {
			for (x = 0; x < 8; x++) {
				tempCol = bmsprites[((Number &  0x1FFFF) * 64) + (y * 8) + x];

				if (tempCol == 0)
					continue;	// pixel is transparent

				sx = Startx - x;
				sy = Starty - y;

				if (sx >= 0 && sx < 320 && sy >= 0 && sy <240) {
					temp = pBurnDraw + (nBurnPitch * sy) + (nBurnBpp * sx);
					PutPix(temp, LocalPalette[tempCol | (Colour << 4)]);
				}
			}
		}

		return;
	}

	if (flipx)
	{
		Startx+=7;
		for (y = 0; y < 8; y++) {
			for (x = 0; x < 8; x++) {
				tempCol = bmsprites[((Number &  0x1FFFF) * 64) + (y * 8) + x];

				if (tempCol == 0) continue;	// pixel is transparent

				sx = Startx - x;
				sy = Starty + y;

				if (sx >= 0 && sx < 320 && sy >= 0 && sy <240) {
					temp = pBurnDraw + (nBurnPitch * sy) + (nBurnBpp * sx);
					PutPix(temp, LocalPalette[tempCol | (Colour << 4)]);
				}
			}
		}


		return;
	}

	if (flipy)
	{
		Starty+=7;
		for (y = 0; y < 8; y++) {
			for (x = 0; x < 8; x++) {
				tempCol = bmsprites[((Number &  0x1FFFF) * 64) + (y * 8) + x];

				if (tempCol == 0) continue;	// pixel is transparent

				sx = Startx + x;
				sy = Starty - y;

				if (sx >= 0 && sx < 320 && sy >= 0 && sy <240) {
					temp = pBurnDraw + (nBurnPitch * sy) + (nBurnBpp * sx);
					PutPix(temp, LocalPalette[tempCol | (Colour << 4)]);
				}
			}
		}
		return;
	}

	for (y = 0; y < 8; y++) {
		for (x = 0; x < 8; x++) {
			tempCol = bmsprites[((Number &  0x1FFFF) * 64) + (y * 8) + x];

			if (tempCol == 0) continue;	// pixel is transparent

			sx = Startx + x;
			sy = Starty + y;

			if (sx >= 0 && sx < 320 && sy >= 0 && sy <240) {
				temp = pBurnDraw + (nBurnPitch * sy) + (nBurnBpp * sx);
				PutPix(temp, LocalPalette[tempCol | (Colour << 4)]);
			}
		}
	}
}

inline void tiletest(int Number, int Startx, int Starty, int Colour,bool flipx)
{
	int x, y, tempCol, sx, sy;
	unsigned char *temp;

	if (flipx)
	{
		Startx+=15;
		for (y = 0; y < 16; y++) {
			for (x = 0; x < 16; x++) {
				tempCol = bmsprites[((Number &  0x07FFF) * 256) + (y * 16) + x];

				if (tempCol == 0) continue;	// pixel is transparent

				sx = Startx - x;
				sy = Starty + y;

				if (sx >= 0 && sx < 320 && sy >= 0 && sy <240) {
					temp = pBurnDraw + (nBurnPitch * sy) + (nBurnBpp * sx);
					PutPix(temp, LocalPalette[tempCol | (Colour << 4)]);
				}
			}
		}
		return;
	}

	for (y = 0; y < 16; y++) {
		for (x = 0; x < 16; x++) {
			tempCol = bmsprites[((Number &  0x07FFF) * 256) + (y * 16) + x];

			if (tempCol == 0) continue;	// pixel is transparent

			sx = Startx + x;
			sy = Starty + y;

			if (sx >= 0 && sx < 320 && sy >= 0 && sy <240) {
				temp = pBurnDraw + (nBurnPitch * sy) + (nBurnBpp * sx);
				PutPix(temp, LocalPalette[tempCol | (Colour << 4)]);
			}
		}
	}
}

static void BiomtoyRenderBgLayer0(int layerpri)
{
	int mx, my,data,data2, Code, Colour, x, y, TileIndex = 0;
	unsigned short* VidRamWord = ((unsigned short*)VideoRam);

	Scroll1Y= VidRegs[0];
	Scroll1X= VidRegs[1]+4;
	for (my = 0; my < 32; my++) {
		for (mx = 0; mx < 32; mx++) {
			data = swapWord(VidRamWord[TileIndex << 1]);
			data2 = swapWord(VidRamWord[(TileIndex << 1) + 1]);
			Code = ((data & 0xfffc) >> 2);
			//Code = ((VidRamWord[TileIndex + 1] << 8) | FstarfrcVideo2Ram[TileIndex + 0]) & 0x1fff;
			Colour =data2 & 0x3f;
			int tilepri=(data2 >> 6) & 0x03;
			if (tilepri!=layerpri)
			{
				return;
			}
			x = 16 * mx;
			y = 16 * my;

			x -= Scroll1X;
			y -= Scroll1Y;

			x &= 0x1ff;
			y &= 0x1ff;

			if (x >= 496)
				x -= 512;
			if (y >= 496)
				y -= 512;

			y -= 16;
			Code+=0x4000;
			//		Render16x16Tile_Mask_Clip(Code, x, y, Colour, 4, 0,0,  bmsprites);

			tiletest(Code, x, y, Colour& 0x3f,data & 0x03);

			TileIndex ++;
		}
	}
}

static void BiomtoyRenderBgLayer1(int layerpri)
{
	int mx, my,data,data2, Code, Colour, x, y, TileIndex = 0;
	unsigned short* VidRamWord = ((unsigned short*)VideoRam);

	Scroll2Y= VidRegs[2];
	Scroll2X= VidRegs[3];


	for (my = 0; my < 32; my++) {
		for (mx = 0; mx < 32; mx++) {

			data = swapWord(VidRamWord[(0x1000/2) + (TileIndex << 1)]);
			data2 = swapWord(VidRamWord[(0x1000/2) + (TileIndex << 1) + 1]);
			Code = ((data & 0xfffc) >> 2);

			//Code = ((VidRamWord[TileIndex + 1] << 8) | FstarfrcVideo2Ram[TileIndex + 0]) & 0x1fff;
			Colour =data2 & 0x3f;
			int tilepri=(data2 >> 6) & 0x03;
			if (tilepri!=layerpri)
			{
				return;
			}
			x = 16 * mx;
			y = 16 * my;

			x -= Scroll2X;
			y -= Scroll2Y;

			x &= 0x1ff;
			y &= 0x1ff;

			if (x >= 496) x -= 512;
			if (y >= 496) y -= 512;

			y -= 16;
			Code+=0x4000;
			//	Render16x16Tile_Mask_Clip(Code, x, y, Colour, 4, 0,0,  bmsprites);
			tiletest(Code, x, y, Colour& 0x3f,data & 0x03);

			TileIndex ++;
		}
	}
}


static void gaelco_sort_sprites(void)
{
	int i;

	unsigned short* SprRamWord = ((unsigned short*)SpriteRam);
	spritelistcount[0] = 0;
	spritelistcount[1] = 0;
	spritelistcount[2] = 0;
	spritelistcount[3] = 0;
	spritelistcount[4] = 0;

	for (i = 3; i < (0x1000 - 6)/2; i += 4){
		int color = (swapWord(SprRamWord[i+2]) & 0x7e00) >> 9;
		int priority = (swapWord(SprRamWord[i]) & 0x3000) >> 12;

		/* palettes 0x38-0x3f are used for high priority sprites in Big Karnak */
		if (color >= 0x38){
			pSpriteList[4][spritelistcount[4]].number = i;
			spritelistcount[4]++;
		}

		/* save sprite number in the proper array for later */
		pSpriteList[priority][spritelistcount[priority]].number = i;
		spritelistcount[priority]++;
	}
}
static void draw_spritelist(int pri)
{
	int j, x, y, ex, ey;

	unsigned short* SprRamWord = ((unsigned short*)SpriteRam);

	static int x_offset[2] = {0x0,0x2};
	static int y_offset[2] = {0x0,0x1};

	for (j = 0; j < spritelistcount[pri]; j++){
		int i = pSpriteList[pri][j].number;
		int sx = swapWord(SprRamWord[i+2]) & 0x01ff;
		int sy = (240 - (swapWord(SprRamWord[i]) & 0x00ff)) & 0x00ff;
		int number = swapWord(SprRamWord[i+3]);
		int color = (swapWord(SprRamWord[i+2]) & 0x7e00) >> 9;
		int attr = (swapWord(SprRamWord[i]) & 0xfe00) >> 9;

		int xflip = attr & 0x20;
		int yflip = attr & 0x40;
		int spr_size;

		if (attr & 0x04){
			spr_size = 1;
		}
		else{
			spr_size = 2;
			number &= (~3);
		}

		for (y = 0; y < spr_size; y++){
			for (x = 0; x < spr_size; x++){

				ex = xflip ? (spr_size-1-x) : x;
				ey = yflip ? (spr_size-1-y) : y;
				spritetest((number + x_offset[ex] + y_offset[ey])&  0x1FFFF,(sx-0x0f+x*8),(sy+y*8)-16,color,xflip,yflip);
				//Render8x8Tile_Mask_Clip((number + x_offset[ex] + y_offset[ey])&  0x1FFFF, (sx-0x0f+x*8), (sy+y*8)-16,color,4, 0,0, bmsprites);
				//(int nTileNumber, int startx, int starty,int nTilePalette,unsigned char *TilePointer)
				//TestDrawItem((number + x_offset[ex] + y_offset[ey])&  0x1FFFF,(sx-0x0f+x*8),(sy+y*8)-16,color, bmsprites);
			}
		}
	}
}


// ----------------------------------------------------------------------------

// This routine is called first to determine how much memory is needed (MemEnd-(unsigned char *)0),
// and then to set up all the pointers

static int MemIndex()
{
	unsigned char *Next; Next=Mem;
	Rom = Next; Next += 0x100000; // 68000 program rom
	MSM6295ROM = Next; Next += 0x140000;
	RamStart = Next;
	Ram = Next; Next += 0x10000; // work ram :)
	VideoRam   = Next; Next += 0x2000;
	ScreenRam  = Next; Next += 0x2000;
	SpriteRam  = Next; Next += 0x1000;
	PaletteRam = Next; Next += 0x0800;
	RamEnd = Next;
	LocalPalette = (unsigned int*)Next; Next += 0x00400 * sizeof(unsigned int);
	MemEnd = Next;

	return 0;
}


static void BiomtoyDecode(int tb, int td)
{
	int ta,tc,tb2;

	tb2 = tb;

	for(ta=0x00000;ta<0x40000;ta++){
		tc=spritetemp2[ta];
		bmsprites[tb+0]|=((tc&0x80)>>7)<<td;
		bmsprites[tb+1]|=((tc&0x40)>>6)<<td;
		bmsprites[tb+2]|=((tc&0x20)>>5)<<td;
		bmsprites[tb+3]|=((tc&0x10)>>4)<<td;
		bmsprites[tb+4]|=((tc&0x08)>>3)<<td;
		bmsprites[tb+5]|=((tc&0x04)>>2)<<td;
		bmsprites[tb+6]|=((tc&0x02)>>1)<<td;
		bmsprites[tb+7]|=((tc&0x01)>>0)<<td;
		tb+=8;
	}
//	printf ("%x\n",tb);


	tb = tb2+0x400000;
	for(ta=0x40000;ta<0x80000;ta++){
		tc=spritetemp2[ta];
		bmsprites[tb+0]|=((tc&0x80)>>7)<<td;
		bmsprites[tb+1]|=((tc&0x40)>>6)<<td;
		bmsprites[tb+2]|=((tc&0x20)>>5)<<td;
		bmsprites[tb+3]|=((tc&0x10)>>4)<<td;
		bmsprites[tb+4]|=((tc&0x08)>>3)<<td;
		bmsprites[tb+5]|=((tc&0x04)>>2)<<td;
		bmsprites[tb+6]|=((tc&0x02)>>1)<<td;
		bmsprites[tb+7]|=((tc&0x01)>>0)<<td;
		tb+=16;
		if((tb&0xFF)==0){tb-=0xF8;}
		else{if((tb&0xFF)==8){tb-=0x8;}}
	}
}

static int LoadRoms()
{
	int nRet=0;
	// Load program roms and byteswap
	nRet=BurnLoadRom(Rom+0x00000,1,2); if (nRet!=0) return 1;
	nRet=BurnLoadRom(Rom+0x00001,0,2); if (nRet!=0) return 1;

	nRet=BurnLoadRom(MSM6295ROM,10,1); if (nRet!=0) return 1;
	nRet=BurnLoadRom(MSM6295ROM+0x40000,10,1); if (nRet!=0) return 1;
	nRet=BurnLoadRom(MSM6295ROM+0xc0000,11,1); if (nRet!=0) return 1;

	bmsprites=(unsigned char *)malloc(0x800000);
	spritetemp2=(unsigned char *)malloc(0x80000);

	nRet=BurnLoadRom(spritetemp2,2,1); if (nRet!=0) return 1;
	BiomtoyDecode(0x000000,3);

	nRet=BurnLoadRom(spritetemp2,3,1); if (nRet!=0) return 1;
	BiomtoyDecode(0x000000,2);

	nRet=BurnLoadRom(spritetemp2,4,1); if (nRet!=0) return 1;
	BiomtoyDecode(0x000000,1);

	nRet=BurnLoadRom(spritetemp2,5,1); if (nRet!=0) return 1;
	BiomtoyDecode(0x000000,0);

	nRet=BurnLoadRom(spritetemp2,6,1); if (nRet!=0) return 1;
	BiomtoyDecode(0x200000,3);

	nRet=BurnLoadRom(spritetemp2,7,1); if (nRet!=0) return 1;
	BiomtoyDecode(0x200000,2);

	nRet=BurnLoadRom(spritetemp2,8,1); if (nRet!=0) return 1;
	BiomtoyDecode(0x200000,1);

	nRet=BurnLoadRom(spritetemp2,9,1); if (nRet!=0) return 1;
	BiomtoyDecode(0x200000,0);

	free(spritetemp2);

	return nRet;
}

static int ManiacspLoadRoms()
{
	int nRet=0;
	// Load program roms and byteswap
	nRet=BurnLoadRom(Rom+0x00000,1,2); if (nRet!=0) return 1;
	nRet=BurnLoadRom(Rom+0x00001,0,2); if (nRet!=0) return 1;

	nRet=BurnLoadRom(MSM6295ROM,6,1); if (nRet!=0) return 1;
	nRet=BurnLoadRom(MSM6295ROM+0x40000,6,1); if (nRet!=0) return 1;
	nRet=BurnLoadRom(MSM6295ROM+0xc0000,6,1); if (nRet!=0) return 1;

	bmsprites=(unsigned char *)malloc(0x800000);
	spritetemp2=(unsigned char *)malloc(0x80000);

	nRet=BurnLoadRom(spritetemp2,2,1); if (nRet!=0) return 1;
	nRet=BurnLoadRom(spritetemp2+0x40000,2,1); if (nRet!=0) return 1;
	BiomtoyDecode(0x000000,3);

	nRet=BurnLoadRom(spritetemp2,3,1); if (nRet!=0) return 1;
	nRet=BurnLoadRom(spritetemp2+0x40000,3,1); if (nRet!=0) return 1;
	BiomtoyDecode(0x000000,2);

	nRet=BurnLoadRom(spritetemp2,4,1); if (nRet!=0) return 1;
	nRet=BurnLoadRom(spritetemp2+0x40000,4,1); if (nRet!=0) return 1;
	BiomtoyDecode(0x000000,1);

	nRet=BurnLoadRom(spritetemp2,5,1); if (nRet!=0) return 1;
	nRet=BurnLoadRom(spritetemp2+0x40000,5,1); if (nRet!=0) return 1;
	BiomtoyDecode(0x000000,0);

	free(spritetemp2);

	return nRet;
}


unsigned short __fastcall BiomtoyReadWord(unsigned int a)
{
	switch (a){
	case 0x700000:
		return BiomtoyDip[1];  // DIP SW2
	case 0x700002:
		return BiomtoyDip[0];  // DIP SW1
	case 0x700004:
		return BiomtoyInput[0]; // Player 1
	case 0x700006:
		return BiomtoyInput[1]; // Player 2
	default:
	//	bprintf(PRINT_NORMAL, _T("readw at %x\n"),a);
		return 0xFF;
	}
}

unsigned char __fastcall BiomtoyReadByte(unsigned int a)
{
	switch (a){
case 0x70000f:
	return MSM6295ReadStatus(0);
	// Maniac Square
	case 0x700001:
		return BiomtoyDip[1];  // DIP SW2
	case 0x700003:
		return BiomtoyDip[0];  // DIP SW1
	case 0x700005:
		return BiomtoyInput[0]; // Player 1
	case 0x700007:
		return BiomtoyInput[1]; // Player 2
	}
//	bprintf(PRINT_NORMAL, _T("readb at %x\n"),a);
	return 0;
}

void __fastcall BiomtoyWriteWord(unsigned int a,unsigned short d)
{
	switch (a)
	{
	case 0x70000d:
//		bprintf(PRINT_NORMAL, _T("11\n"));
		return;

	case 0x108000:
		VidRegs[0]=d;
		return;
	case 0x108002:
		VidRegs[1]=d;
		return;
	case 0x108004:
		VidRegs[2]=d;
		return;
	case 0x108006:
		VidRegs[3]=d;
		return;
	case 0x10800c:
		// this is listed as watchdog reset in mame, so investigate this a bit more
		return;
	default:
//		bprintf(PRINT_NORMAL, _T("writew %x at %x\n"),d,a);
		return;
	}

	return;
}


void __fastcall BiomtoyWriteByte(unsigned int a,unsigned char d)
{
	switch (a){
	case 0x70000d:
		SndBank = (d & 0x0f);
		memcpy(MSM6295ROM + 0x30000, MSM6295ROM + 0x40000 + (0x10000 * SndBank), 0x10000);
		return;
	case 0x70000f:
		MSM6295Command(0, d);
		return;
	}
	return;
}

static int drvDoReset()
{
	SekOpen(0);
	SekReset();
	SekClose();

	SndBank = 0;
	memcpy(MSM6295ROM + 0x30000, MSM6295ROM + 0x40000 + (0x10000 * SndBank), 0x10000);

	MSM6295Reset(0);

	return 0;
}

int BiomtoyInit()
{
	int nRet=0; int nLen=0;

	//--------------Allocate Memory-----------------
	Mem=NULL;
	MemIndex();
	nLen=MemEnd-(unsigned char *)0;
	Mem=(unsigned char *)malloc(nLen);
	if (Mem==NULL)
		return 1;
	memset(Mem,0,nLen); // blank all memory
	MemIndex(); // Index the allocated memory

	//--------------- Load Roms -------------------
	if (!strcmp(BurnDrvGetTextA(DRV_NAME), "maniacsp")) {
		nRet=ManiacspLoadRoms();
	} else {
		nRet=LoadRoms();
	}
	if (nRet!=0)
		return 1; // Load the roms into memory
	init_spritelist();

	// ----------------- Cpu 1 68k ------------------------
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(    Rom  ,0x000000,0x0fffff,SM_ROM); // 68000 Rom : 0x100000
	SekMapMemory(VideoRam ,0x100000,0x101fff,SM_RAM); // video ram : 0x2000
	SekMapMemory(ScreenRam ,0x102000,0x103fff,SM_RAM); // screen ram : 0x2000
	SekMapMemory(PaletteRam,0x200000,0x2007ff,SM_WRITE);
	SekMapMemory(SpriteRam,0x440000,0x440fff,SM_RAM); // sprite ram : 0x1000
	SekMapMemory(    Ram  ,0xff0000,0xffffff,SM_RAM); // 68000 ram
	SekSetReadWordHandler(0,BiomtoyReadWord);
	SekSetWriteWordHandler(0,BiomtoyWriteWord);
	SekSetReadByteHandler(0,BiomtoyReadByte);
	SekSetWriteByteHandler(0,BiomtoyWriteByte);
	SekClose();
	MSM6295Init(0, 8000, 100, 0);

	//-------------------------------------

	BurnTransferInit();
	drvDoReset();
	return 0;
}

int BiomtoyExit()
{
	SekExit(); // Deallocate 68000
	BurnTransferExit();
	exit_spritelist();
	MSM6295Exit(0);
	free(bmsprites);
	free(Mem);
	return 0;
}


static int BiomtoyGFX()
{
	gaelco_sort_sprites();
	BiomtoyRenderBgLayer1(3);
	BiomtoyRenderBgLayer0(3);
	draw_spritelist(3);
	BiomtoyRenderBgLayer1(2);
	BiomtoyRenderBgLayer0(2);
	draw_spritelist(2);
	BiomtoyRenderBgLayer1(1);
	BiomtoyRenderBgLayer0(1);
	draw_spritelist(1);
	BiomtoyRenderBgLayer1(0);
	BiomtoyRenderBgLayer0(0);
	draw_spritelist(0);
	return 0;
}


static int BiomtoyDraw()
{
	BurnClearScreen();
	BiomtoyCalcPalette();
	BiomtoyGFX();
	return 0;
}


int BiomtoyFrame()
{
	if (drvReset) {	// Reset machine
		drvDoReset();
	}

	BiomtoyMakeInputs();

	SekOpen(0);
	nCyclesTotal[0] = (int)((long long)24000000 * nBurnCPUSpeedAdjust / (0x0100 * 60));
	SekRun(nCyclesTotal[0]);
	SekSetIRQLine(6, SEK_IRQSTATUS_AUTO);
	SekClose();
	if (pBurnSoundOut)
		MSM6295Render(0, pBurnSoundOut, nBurnSoundLen);

	if (pBurnDraw) {
		BiomtoyDraw();														// Draw screen if needed
	}

	return 0;
}

static int BiomtoyScan(int nAction,int *pnMin)
{
	struct BurnArea ba;

	if (pnMin) {						// Return minimum compatible version
		*pnMin = 0x029521;
	}

	if (nAction & ACB_MEMORY_RAM) {								// Scan all memory, devices & variables
		ba.Data		= Ram;
		ba.nLen		= 0x10000;
		ba.nAddress = 0;
		ba.szName	= "68K RAM";
		BurnAcb(&ba);

		ba.Data		= VideoRam;
		ba.nLen		= 0x02000;
		ba.nAddress = 0;
		ba.szName	= "Video RAM";
		BurnAcb(&ba);

		ba.Data		= ScreenRam;
		ba.nLen		= 0x02000;
		ba.nAddress = 0;
		ba.szName	= "Screen RAM";
		BurnAcb(&ba);

		ba.Data		= SpriteRam;
		ba.nLen		= 0x01000;
		ba.nAddress = 0;
		ba.szName	= "Sprite RAM";
		BurnAcb(&ba);

		ba.Data		= PaletteRam;
		ba.nLen		= 0x00800;
		ba.nAddress = 0;
		ba.szName	= "Palette";
		BurnAcb(&ba);
	}

	if (nAction & ACB_DRIVER_DATA) {

		SekScan(nAction);				      // Scan 68000

		MSM6295Scan(0, nAction);

		// Scan critical driver variables
		SCAN_VAR(SndBank);
		SCAN_VAR(BiomtoyInput);
		SCAN_VAR(BiomtoyDip);
	}

	if (nAction & ACB_WRITE) {
		memcpy(MSM6295ROM + 0x30000, MSM6295ROM + 0x40000 + (0x10000 * SndBank), 0x10000);
	}

	return 0;
}

// ----------------------------------------------------------------------------
// Rom information

// Biomechanical Toy

static struct BurnRomInfo BiomtoyRomDesc[]=
{
	{ "d18", 0x080000, 0x4569ce64, BRF_ESS | BRF_PRG }, //  0 68000 code (even)
	{ "d16", 0x080000, 0x739449bd, BRF_ESS | BRF_PRG }, //  1            (odd)

	{ "j6",  0x80000,  0xe923728b, BRF_GRA }, 			//  2 Graphics
	{ "j7",  0x80000,  0x0e18fac2, BRF_GRA }, 			//  3
	{ "j9",  0x80000,  0x1c93f050, BRF_GRA }, 			//  4
	{ "j10", 0x80000,  0x8e3e96cc, BRF_GRA }, 			//  5

	{ "h6",  0x80000,  0x9416a729, BRF_GRA }, 			//  6 Graphics
	{ "h7",  0x80000,  0x9c984d7b, BRF_GRA }, 			//  7
	{ "h9",  0x80000,  0x8c1f6718, BRF_GRA }, 			//  8
	{ "h10", 0x80000,  0xaca1702b, BRF_GRA }, 			//  9

	{ "c1",  0x080000, 0x0f02de7e, BRF_SND }, 			// 10 OKI adpcm
	{ "c3",  0x080000, 0x914e4bbc, BRF_SND }, 			// 11
};

STD_ROM_PICK(Biomtoy)
STD_ROM_FN(Biomtoy)

struct BurnDriver BurnDrvBiomtoy = {
	"biomtoy", NULL, NULL, "1995",
	"Biomechanical Toy (unprotected)\0", NULL, "Zeus", "Gaelco '91 - '96",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING,2,HARDWARE_MISC_MISC,
	NULL,BiomtoyRomInfo,BiomtoyRomName,BiomtoyInputInfo,BiomtoyDIPInfo,
	BiomtoyInit,BiomtoyExit,BiomtoyFrame,NULL,BiomtoyScan,
	NULL,320,240,4,3
};

// Maniac Square

static struct BurnRomInfo ManiacspRomDesc[]=
{
	{ "d18", 0x020000, 0x740ecab2, BRF_ESS | BRF_PRG }, //  0 68000 code (even)
	{ "d16", 0x020000, 0xc6c42729, BRF_ESS | BRF_PRG }, //  1            (odd)

	{ "f3",  0x40000,  0xe7f6582b, BRF_GRA }, //  2 Graphics
	{ "f2",  0x40000,  0xca43a5ae, BRF_GRA }, //  3
	{ "f1",  0x40000,  0xfca112e8, BRF_GRA }, //  4
	{ "f0",  0x40000,  0x6e829ee8, BRF_GRA }, //  5

	{ "c1",  0x080000, 0x2557f2d6, BRF_SND }, //  6 OKI adpcm
};

STD_ROM_PICK(Maniacsp)
STD_ROM_FN(Maniacsp)

struct BurnDriver BurnDrvManiacsp = {
	"maniacsp", "maniacsq", NULL, "1996",
	"Maniac Square (prototype)\0", "Missing GFXs", "Gaelco", "Gaelco '91 - '96",
	NULL, NULL, NULL, NULL,
	BDF_CLONE | BDF_PROTOTYPE,2,HARDWARE_MISC_MISC,
	NULL,ManiacspRomInfo,ManiacspRomName,BiomtoyInputInfo,ManiacspDIPInfo,
	BiomtoyInit,BiomtoyExit,BiomtoyFrame,NULL,BiomtoyScan,
	NULL,320,240,4,3
};
