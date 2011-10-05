#include "gal.h"

GalRenderBackground GalRenderBackgroundFunction;
GalCalcPalette GalCalcPaletteFunction;
GalDrawBullet GalDrawBulletsFunction;
GalExtendTileInfo GalExtendTileInfoFunction;
GalExtendSpriteInfo GalExtendSpriteInfoFunction;
GalRenderFrame GalRenderFrameFunction;

unsigned char GalFlipScreenX;
unsigned char GalFlipScreenY;
unsigned char *GalGfxBank;
unsigned char GalPaletteBank;
unsigned char GalSpriteClipStart;
unsigned char GalSpriteClipEnd;
unsigned char FroggerAdjust;
unsigned char GalBackgroundRed;
unsigned char GalBackgroundGreen;
unsigned char GalBackgroundBlue;
unsigned char GalBackgroundEnable;
unsigned char SfxTilemap;
unsigned char GalOrientationFlipX;
unsigned char GalColourDepth;
unsigned char DarkplntBulletColour;
unsigned char DambustrBgColour1;
unsigned char DambustrBgColour2;
unsigned char DambustrBgPriority;
unsigned char DambustrBgSplitLine;
unsigned char *RockclimTiles;
unsigned short RockclimScrollX;
unsigned short RockclimScrollY;

// Graphics decode helpers
int CharPlaneOffsets[2]   = { 0, 0x4000 };
int CharXOffsets[8]       = { 0, 1, 2, 3, 4, 5, 6, 7 };
int CharYOffsets[8]       = { 0, 8, 16, 24, 32, 40, 48, 56 };
int SpritePlaneOffsets[2] = { 0, 0x4000 };
int SpriteXOffsets[16]    = { 0, 1, 2, 3, 4, 5, 6, 7, 64, 65, 66, 67, 68, 69, 70, 71 };
int SpriteYOffsets[16]    = { 0, 8, 16, 24, 32, 40, 48, 56, 128, 136, 144, 152, 160, 168, 176, 184 };

// Tile extend helpers
void UpperExtendTileInfo(unsigned short *Code, int*, int, int)
{
	*Code += 0x100;
}

void PiscesExtendTileInfo(unsigned short *Code, int*, int, int)
{
	*Code |= GalGfxBank[0] << 8;
}

void Batman2ExtendTileInfo(unsigned short *Code, int*, int, int)
{
	if (*Code & 0x80) *Code |= GalGfxBank[0] << 8;
}

void GmgalaxExtendTileInfo(unsigned short *Code, int*, int, int)
{
	*Code |= GalGfxBank[0] << 9;
}

void MooncrstExtendTileInfo(unsigned short *Code, int*, int, int)
{
	if (GalGfxBank[2] && (*Code & 0xc0) == 0x80) *Code = (*Code & 0x3f) | (GalGfxBank[0] << 6) | (GalGfxBank[1] << 7) | 0x0100;
}

void MoonqsrExtendTileInfo(unsigned short *Code, int*, int Attr, int)
{
	*Code |= (Attr & 0x20) << 3;
}

void SkybaseExtendTileInfo(unsigned short *Code, int*, int, int)
{
	*Code |= GalGfxBank[2] << 8;
}

void JumpbugExtendTileInfo(unsigned short *Code, int*, int, int)
{
	if ((*Code & 0xc0) == 0x80 && (GalGfxBank[2] & 0x01)) *Code += 128 + ((GalGfxBank[0] & 0x01) << 6) + ((GalGfxBank[1] & 0x01) << 7) + ((~GalGfxBank[4] & 0x01) << 8);
}

void FroggerExtendTileInfo(unsigned short*, int *Colour, int, int)
{
	*Colour = ((*Colour >> 1) & 0x03) | ((*Colour << 2) & 0x04);
}

void MshuttleExtendTileInfo(unsigned short *Code, int*, int Attr, int)
{
	*Code |= (Attr & 0x30) << 4;
}

void Fourin1ExtendTileInfo(unsigned short *Code, int*, int, int)
{
	*Code |= Fourin1Bank << 8;
}

void MarinerExtendTileInfo(unsigned short *Code, int*, int, int x)
{
	unsigned char *Prom = GalProm + 0x120;
	
	*Code |= (Prom[x] & 0x01) << 8;
}

void MimonkeyExtendTileInfo(unsigned short *Code, int*, int, int)
{
	*Code |= (GalGfxBank[0] << 8) | (GalGfxBank[1] << 9);
}

void DambustrExtendTileInfo(unsigned short *Code, int*, int, int x)
{
	if (GalGfxBank[0] == 0) {
		*Code |= 0x300;
	} else {
		if (x == 28) {
			*Code |= 0x300;
		} else {
			*Code &= 0xff;
		}
	}
}

void Ad2083ExtendTileInfo(unsigned short *Code, int *Colour, int Attr, int)
{
	int Bank = Attr & 0x30;
	*Code |= (Bank << 4);
	*Colour |= ((Attr & 0x40) >> 3);
}

void RacknrolExtendTileInfo(unsigned short *Code, int*, int, int x)
{
	unsigned char Bank = GalGfxBank[x] & 7;	
	*Code |= Bank << 8;
}

// Sprite extend helpers
void UpperExtendSpriteInfo(const unsigned char*, int*, int*, unsigned char*, unsigned char*, unsigned short *Code, unsigned char*)
{
	*Code += 0x40;
}

void PiscesExtendSpriteInfo(const unsigned char*, int*, int*, unsigned char*, unsigned char*, unsigned short *Code, unsigned char*)
{
	*Code |= GalGfxBank[0] << 6;
}

void GmgalaxExtendSpriteInfo(const unsigned char*, int*, int*, unsigned char*, unsigned char*, unsigned short *Code, unsigned char*)
{
	*Code |= (GalGfxBank[0] << 7) | 0x40;
}

void MooncrstExtendSpriteInfo(const unsigned char*, int*, int*, unsigned char*, unsigned char*, unsigned short *Code, unsigned char*)
{
	if (GalGfxBank[2] && (*Code & 0x30) == 0x20) *Code = (*Code & 0x0f) | (GalGfxBank[0] << 4) | (GalGfxBank[1] << 5) | 0x40;
}

void MoonqsrExtendSpriteInfo(const unsigned char *Base, int*, int*, unsigned char*, unsigned char*, unsigned short *Code, unsigned char*)
{
	*Code |= (Base[2] & 0x20) << 1;
}

void SkybaseExtendSpriteInfo(const unsigned char*, int*, int*, unsigned char*, unsigned char*, unsigned short *Code, unsigned char*)
{
	*Code |= GalGfxBank[2] << 6;
}

void RockclimExtendSpriteInfo(const unsigned char*, int*, int*, unsigned char*, unsigned char*, unsigned short *Code, unsigned char*)
{
	if (GalGfxBank[2]) *Code |= 0x40;
}

void JumpbugExtendSpriteInfo(const unsigned char*, int*, int*, unsigned char*, unsigned char*, unsigned short *Code, unsigned char*)
{
	if ((*Code & 0x30) == 0x20 && (GalGfxBank[2] & 0x01) != 0) *Code += 32 + ((GalGfxBank[0] & 0x01) << 4) + ((GalGfxBank[1] & 0x01) << 5) + ((~GalGfxBank[4] & 0x01) << 6);
}

void FroggerExtendSpriteInfo(const unsigned char*, int*, int*, unsigned char*, unsigned char*, unsigned short*, unsigned char *Colour)
{
	*Colour = ((*Colour >> 1) & 0x03) | ((*Colour << 2) & 0x04);
}

void CalipsoExtendSpriteInfo(const unsigned char *Base, int*, int*, unsigned char *xFlip, unsigned char *yFlip, unsigned short *Code, unsigned char*)
{
	*Code = Base[1];
	*xFlip = 0;
	*yFlip = 0;
}

void MshuttleExtendSpriteInfo(const unsigned char *Base, int*, int*, unsigned char*, unsigned char*, unsigned short *Code, unsigned char*)
{
	*Code |= (Base[2] & 0x30) << 2;
}

void Fourin1ExtendSpriteInfo(const unsigned char*, int*, int*, unsigned char*, unsigned char*, unsigned short *Code, unsigned char*)
{
	*Code |= Fourin1Bank << 6;
}

void DkongjrmExtendSpriteInfo(const unsigned char *Base, int*, int*, unsigned char *xFlip, unsigned char*, unsigned short *Code, unsigned char*)
{
	*Code = (Base[1] & 0x7f) | 0x80;
	*xFlip = 0;
}

void MimonkeyExtendSpriteInfo(const unsigned char*, int*, int*, unsigned char*, unsigned char*, unsigned short *Code, unsigned char*)
{
	*Code |= (GalGfxBank[0] << 6) | (GalGfxBank[1] << 7);
}

void Ad2083ExtendSpriteInfo(const unsigned char *Base, int*, int*, unsigned char *xFlip, unsigned char*, unsigned short *Code, unsigned char*)
{
	*Code = (Base[1] & 0x7f) | ((Base[2] & 0x30) << 2);
	*xFlip = 0;
}

// Hardcode a Galaxian PROM for any games that are missing a PROM dump
void HardCodeGalaxianPROM()
{
	GalProm[0x00]= 0x00;
	GalProm[0x01]= 0x00;
	GalProm[0x02]= 0x00;
	GalProm[0x03]= 0xf6;
	GalProm[0x04]= 0x00;
	GalProm[0x05]= 0x16;
	GalProm[0x06]= 0xc0;
	GalProm[0x07]= 0x3f;
	GalProm[0x08]= 0x00;
	GalProm[0x09]= 0xd8;
	GalProm[0x0a]= 0x07;
	GalProm[0x0b]= 0x3f;
	GalProm[0x0c]= 0x00;
	GalProm[0x0d]= 0xc0;
	GalProm[0x0e]= 0xc4;
	GalProm[0x0f]= 0x07;
	GalProm[0x10]= 0x00;
	GalProm[0x11]= 0xc0;
	GalProm[0x12]= 0xa0;
	GalProm[0x13]= 0x07;
	GalProm[0x14]= 0x00;
	GalProm[0x15]= 0x00;
	GalProm[0x16]= 0x00;
	GalProm[0x17]= 0x07;
	GalProm[0x18]= 0x00;
	GalProm[0x19]= 0xf6;
	GalProm[0x1a]= 0x07;
	GalProm[0x1b]= 0xf0;
	GalProm[0x1c]= 0x00;
	GalProm[0x1d]= 0x76;
	GalProm[0x1e]= 0x07;
	GalProm[0x1f]= 0xc6;
}

void HardCodeMooncrstPROM()
{
	GalProm[0x00]= 0x00;
	GalProm[0x01]= 0x7a;
	GalProm[0x02]= 0x36;
	GalProm[0x03]= 0x07;
	GalProm[0x04]= 0x00;
	GalProm[0x05]= 0xf0;
	GalProm[0x06]= 0x38;
	GalProm[0x07]= 0x1f;
	GalProm[0x08]= 0x00;
	GalProm[0x09]= 0xc7;
	GalProm[0x0a]= 0xf0;
	GalProm[0x0b]= 0x3f;
	GalProm[0x0c]= 0x00;
	GalProm[0x0d]= 0xdb;
	GalProm[0x0e]= 0xc6;
	GalProm[0x0f]= 0x38;
	GalProm[0x10]= 0x00;
	GalProm[0x11]= 0x36;
	GalProm[0x12]= 0x07;
	GalProm[0x13]= 0xf0;
	GalProm[0x14]= 0x00;
	GalProm[0x15]= 0x33;
	GalProm[0x16]= 0x3f;
	GalProm[0x17]= 0xdb;
	GalProm[0x18]= 0x00;
	GalProm[0x19]= 0x3f;
	GalProm[0x1a]= 0x57;
	GalProm[0x1b]= 0xc6;
	GalProm[0x1c]= 0x00;
	GalProm[0x1d]= 0xc6;
	GalProm[0x1e]= 0x3f;
	GalProm[0x1f]= 0xff;
}

// Pallet generation
#define RGB_MAXIMUM			224
#define MAX_NETS			3
#define MAX_RES_PER_NET			18
#define Combine2Weights(tab,w0,w1)	((int)(((tab)[0]*(w0) + (tab)[1]*(w1)) + 0.5))
#define Combine3Weights(tab,w0,w1,w2)	((int)(((tab)[0]*(w0) + (tab)[1]*(w1) + (tab)[2]*(w2)) + 0.5))

static double ComputeResistorWeights(int MinVal, int MaxVal, double Scaler, int Count1, const int *Resistances1, double *Weights1, int PullDown1, int PullUp1,	int Count2, const int *Resistances2, double *Weights2, int PullDown2, int PullUp2, int Count3, const int *Resistances3, double *Weights3, int PullDown3, int PullUp3)
{
	int NetworksNum;

	int ResCount[MAX_NETS];
	double r[MAX_NETS][MAX_RES_PER_NET];
	double w[MAX_NETS][MAX_RES_PER_NET];
	double ws[MAX_NETS][MAX_RES_PER_NET];
	int r_pd[MAX_NETS];
	int r_pu[MAX_NETS];

	double MaxOut[MAX_NETS];
	double *Out[MAX_NETS];

	int i, j, n;
	double Scale;
	double Max;

	NetworksNum = 0;
	for (n = 0; n < MAX_NETS; n++) {
		int Count, pd, pu;
		const int *Resistances;
		double *Weights;

		switch (n) {
			case 0: {
				Count = Count1;
				Resistances = Resistances1;
				Weights = Weights1;
				pd = PullDown1;
				pu = PullUp1;
				break;
			}
			
			case 1: {
				Count = Count2;
				Resistances = Resistances2;
				Weights = Weights2;
				pd = PullDown2;
				pu = PullUp2;
				break;
			}
		
			case 2:
			default: {
				Count = Count3;
				Resistances = Resistances3;
				Weights = Weights3;
				pd = PullDown3;
				pu = PullUp3;
				break;
			}
		}

		if (Count > 0) {
			ResCount[NetworksNum] = Count;
			for (i = 0; i < Count; i++) {
				r[NetworksNum][i] = 1.0 * Resistances[i];
			}
			Out[NetworksNum] = Weights;
			r_pd[NetworksNum] = pd;
			r_pu[NetworksNum] = pu;
			NetworksNum++;
		}
	}

	for (i = 0; i < NetworksNum; i++) {
		double R0, R1, Vout, Dst;

		for (n = 0; n < ResCount[i]; n++) {
			R0 = (r_pd[i] == 0) ? 1.0 / 1e12 : 1.0 / r_pd[i];
			R1 = (r_pu[i] == 0) ? 1.0 / 1e12 : 1.0 / r_pu[i];

			for (j = 0; j < ResCount[i]; j++) {
				if (j == n) {
					if (r[i][j] != 0.0) R1 += 1.0 / r[i][j];
				} else {
					if (r[i][j] != 0.0) R0 += 1.0 / r[i][j];
				}
			}

			R0 = 1.0/R0;
			R1 = 1.0/R1;
			Vout = (MaxVal - MinVal) * R0 / (R1 + R0) + MinVal;

			Dst = (Vout < MinVal) ? MinVal : (Vout > MaxVal) ? MaxVal : Vout;

			w[i][n] = Dst;
		}
	}

	j = 0;
	Max = 0.0;
	for (i = 0; i < NetworksNum; i++) {
		double Sum = 0.0;

		for (n = 0; n < ResCount[i]; n++) Sum += w[i][n];

		MaxOut[i] = Sum;
		if (Max < Sum) {
			Max = Sum;
			j = i;
		}
	}

	if (Scaler < 0.0) {
		Scale = ((double)MaxVal) / MaxOut[j];
	} else {
		Scale = Scaler;
	}

	for (i = 0; i < NetworksNum; i++) {
		for (n = 0; n < ResCount[i]; n++) {
			ws[i][n] = w[i][n] * Scale;
			(Out[i])[n] = ws[i][n];
		}
	}

	return Scale;

}

void GalaxianCalcPalette()
{
	static const int RGBResistances[3] = {1000, 470, 220};
	double rWeights[3], gWeights[3], bWeights[2];
	
	ComputeResistorWeights(0, RGB_MAXIMUM, -1.0, 3, &RGBResistances[0], rWeights, 470, 0, 3, &RGBResistances[0], gWeights, 470, 0, 2, &RGBResistances[1], bWeights, 470, 0);
			
	// Colour PROM
	for (int i = 0; i < 32; i++) {
		unsigned char Bit0, Bit1, Bit2, r, g, b;

		Bit0 = BIT(GalProm[i + (GalPaletteBank * 0x20)],0);
		Bit1 = BIT(GalProm[i + (GalPaletteBank * 0x20)],1);
		Bit2 = BIT(GalProm[i + (GalPaletteBank * 0x20)],2);
		r = Combine3Weights(rWeights, Bit0, Bit1, Bit2);

		Bit0 = BIT(GalProm[i + (GalPaletteBank * 0x20)],3);
		Bit1 = BIT(GalProm[i + (GalPaletteBank * 0x20)],4);
		Bit2 = BIT(GalProm[i + (GalPaletteBank * 0x20)],5);
		g = Combine3Weights(gWeights, Bit0, Bit1, Bit2);

		Bit0 = BIT(GalProm[i + (GalPaletteBank * 0x20)],6);
		Bit1 = BIT(GalProm[i + (GalPaletteBank * 0x20)],7);
		b = Combine2Weights(bWeights, Bit0, Bit1);

		GalPalette[i] = BurnHighCol(r, g, b, 0);
	}
	
	// Stars
	for (int i = 0; i < GAL_PALETTE_NUM_COLOURS_STARS; i++) {
		int Bits, r, g, b;
		int Map[4] = {0x00, 0x88, 0xcc, 0xff};

		Bits = (i >> 0) & 0x03;
		r = Map[Bits];
		Bits = (i >> 2) & 0x03;
		g = Map[Bits];
		Bits = (i >> 4) & 0x03;
		b = Map[Bits];

		GalPalette[i + GAL_PALETTE_STARS_OFFSET] = BurnHighCol(r, g, b, 0);
	}

	// Bullets
	for (int i = 0; i < GAL_PALETTE_NUM_COLOURS_BULLETS - 1; i++) {
		GalPalette[i + GAL_PALETTE_BULLETS_OFFSET] = BurnHighCol(0xff, 0xff, 0xff, 0);
	}
	GalPalette[GAL_PALETTE_NUM_COLOURS_BULLETS - 1 + GAL_PALETTE_BULLETS_OFFSET] = BurnHighCol(0xff, 0xff, 0x00, 0);
}

void RockclimCalcPalette()
{
	static const int RGBResistances[3] = {1000, 470, 220};
	double rWeights[3], gWeights[3], bWeights[2];
	
	ComputeResistorWeights(0, RGB_MAXIMUM, -1.0, 3, &RGBResistances[0], rWeights, 470, 0, 3, &RGBResistances[0], gWeights, 470, 0, 2, &RGBResistances[1], bWeights, 470, 0);
			
	// Colour PROM
	for (int i = 0; i < 64; i++) {
		unsigned char Bit0, Bit1, Bit2, r, g, b;

		Bit0 = BIT(GalProm[i + (GalPaletteBank * 0x20)],0);
		Bit1 = BIT(GalProm[i + (GalPaletteBank * 0x20)],1);
		Bit2 = BIT(GalProm[i + (GalPaletteBank * 0x20)],2);
		r = Combine3Weights(rWeights, Bit0, Bit1, Bit2);

		Bit0 = BIT(GalProm[i + (GalPaletteBank * 0x20)],3);
		Bit1 = BIT(GalProm[i + (GalPaletteBank * 0x20)],4);
		Bit2 = BIT(GalProm[i + (GalPaletteBank * 0x20)],5);
		g = Combine3Weights(gWeights, Bit0, Bit1, Bit2);

		Bit0 = BIT(GalProm[i + (GalPaletteBank * 0x20)],6);
		Bit1 = BIT(GalProm[i + (GalPaletteBank * 0x20)],7);
		b = Combine2Weights(bWeights, Bit0, Bit1);

		GalPalette[i] = BurnHighCol(r, g, b, 0);
	}
	
	// Stars
	for (int i = 0; i < GAL_PALETTE_NUM_COLOURS_STARS; i++) {
		int Bits, r, g, b;
		int Map[4] = {0x00, 0x88, 0xcc, 0xff};

		Bits = (i >> 0) & 0x03;
		r = Map[Bits];
		Bits = (i >> 2) & 0x03;
		g = Map[Bits];
		Bits = (i >> 4) & 0x03;
		b = Map[Bits];

		GalPalette[i + GAL_PALETTE_STARS_OFFSET] = BurnHighCol(r, g, b, 0);
	}

	// Bullets
	for (int i = 0; i < GAL_PALETTE_NUM_COLOURS_BULLETS - 1; i++) {
		GalPalette[i + GAL_PALETTE_BULLETS_OFFSET] = BurnHighCol(0xff, 0xff, 0xff, 0);
	}
	GalPalette[GAL_PALETTE_NUM_COLOURS_BULLETS - 1 + GAL_PALETTE_BULLETS_OFFSET] = BurnHighCol(0xff, 0xff, 0x00, 0);
}

void MarinerCalcPalette()
{
	GalaxianCalcPalette();
	
	for (int i = 0; i < 16; i++) {
		int b = 0x0e * BIT(i, 0) + 0x1f * BIT(i, 1) + 0x43 * BIT(i, 2) + 0x8f * BIT(i, 3);
		GalPalette[i + GAL_PALETTE_BACKGROUND_OFFSET] = BurnHighCol(0, 0, b, 0);
	}
}

void StratgyxCalcPalette()
{
	GalaxianCalcPalette();
	
	for (int i = 0; i < 8; i++) {
		int r = BIT(i, 0) * 0x7c;
		int g = BIT(i, 1) * 0x3c;
		int b = BIT(i, 2) * 0x47;
		GalPalette[i + GAL_PALETTE_BACKGROUND_OFFSET] = BurnHighCol(r, g, b, 0);
	}
}

void RescueCalcPalette()
{
	GalaxianCalcPalette();
	
	for (int i = 0; i < 128; i++) {
		int b = i * 2;
		GalPalette[i + GAL_PALETTE_BACKGROUND_OFFSET] = BurnHighCol(0, 0, b, 0);
	}
}

void MinefldCalcPalette()
{
	RescueCalcPalette();
	
	for (int i = 0; i < 128; i++) {
		int r = (int)(i * 1.5);
		int g = (int)(i * 0.75);
		int b = i / 2;
		GalPalette[i + 128 + GAL_PALETTE_BACKGROUND_OFFSET] = BurnHighCol(r, g, b, 0);
	}
}

void DarkplntCalcPalette()
{
	static const int RGBResistances[3] = {1000, 470, 220};
	double rWeights[3], gWeights[3], bWeights[2];
	
	ComputeResistorWeights(0, RGB_MAXIMUM, -1.0, 3, &RGBResistances[0], rWeights, 470, 0, 3, &RGBResistances[0], gWeights, 470, 0, 2, &RGBResistances[1], bWeights, 470, 0);
			
	// Colour PROM
	for (int i = 0; i < 32; i++) {
		unsigned char Bit0, Bit1, Bit2, r, g, b;

		Bit0 = BIT(GalProm[i + (GalPaletteBank * 0x20)],0);
		Bit1 = BIT(GalProm[i + (GalPaletteBank * 0x20)],1);
		Bit2 = BIT(GalProm[i + (GalPaletteBank * 0x20)],2);
		r = Combine3Weights(rWeights, Bit0, Bit1, Bit2);

		g = 0;

		Bit0 = BIT(GalProm[i + (GalPaletteBank * 0x20)],3);
		Bit1 = BIT(GalProm[i + (GalPaletteBank * 0x20)],4);
		Bit2 = BIT(GalProm[i + (GalPaletteBank * 0x20)],5);
		b = Combine2Weights(bWeights, Bit0, Bit1);

		GalPalette[i] = BurnHighCol(r, g, b, 0);
	}
	
	// Stars
	for (int i = 0; i < GAL_PALETTE_NUM_COLOURS_STARS; i++) {
		int Bits, r, g, b;
		int Map[4] = {0x00, 0x88, 0xcc, 0xff};

		Bits = (i >> 0) & 0x03;
		r = Map[Bits];
		Bits = (i >> 2) & 0x03;
		g = Map[Bits];
		Bits = (i >> 4) & 0x03;
		b = Map[Bits];

		GalPalette[i + GAL_PALETTE_STARS_OFFSET] = BurnHighCol(r, g, b, 0);
	}

	// Bullets
	GalPalette[0 + GAL_PALETTE_BULLETS_OFFSET] = BurnHighCol(0xef, 0x00, 0x00, 0);
	GalPalette[1 + GAL_PALETTE_BULLETS_OFFSET] = BurnHighCol(0x00, 0x00, 0xef, 0);
}

void DambustrCalcPalette()
{
	static const int RGBResistances[3] = {1000, 470, 220};
	double rWeights[3], gWeights[3], bWeights[2];
	
	ComputeResistorWeights(0, RGB_MAXIMUM, -1.0, 3, &RGBResistances[0], rWeights, 470, 0, 3, &RGBResistances[0], gWeights, 470, 0, 2, &RGBResistances[1], bWeights, 470, 0);
			
	// Colour PROM
	for (int i = 0; i < 32; i++) {
		unsigned char Bit0, Bit1, Bit2, r, g, b;

		Bit0 = BIT(GalProm[i + (GalPaletteBank * 0x20)],0);
		Bit1 = BIT(GalProm[i + (GalPaletteBank * 0x20)],1);
		Bit2 = BIT(GalProm[i + (GalPaletteBank * 0x20)],2);
		b = Combine3Weights(rWeights, Bit0, Bit1, Bit2);

		Bit0 = BIT(GalProm[i + (GalPaletteBank * 0x20)],3);
		Bit1 = BIT(GalProm[i + (GalPaletteBank * 0x20)],4);
		Bit2 = BIT(GalProm[i + (GalPaletteBank * 0x20)],5);
		r = Combine3Weights(gWeights, Bit0, Bit1, Bit2);

		Bit0 = BIT(GalProm[i + (GalPaletteBank * 0x20)],6);
		Bit1 = BIT(GalProm[i + (GalPaletteBank * 0x20)],7);
		g = Combine2Weights(bWeights, Bit0, Bit1);

		GalPalette[i] = BurnHighCol(r, g, b, 0);
	}
	
	// Stars
	for (int i = 0; i < GAL_PALETTE_NUM_COLOURS_STARS; i++) {
		int Bits, r, g, b;
		int Map[4] = {0x00, 0x88, 0xcc, 0xff};

		Bits = (i >> 0) & 0x03;
		r = Map[Bits];
		Bits = (i >> 2) & 0x03;
		g = Map[Bits];
		Bits = (i >> 4) & 0x03;
		b = Map[Bits];

		GalPalette[i + GAL_PALETTE_STARS_OFFSET] = BurnHighCol(r, g, b, 0);
	}

	// Bullets
	for (int i = 0; i < GAL_PALETTE_NUM_COLOURS_BULLETS - 1; i++) {
		GalPalette[i + GAL_PALETTE_BULLETS_OFFSET] = BurnHighCol(0xff, 0xff, 0xff, 0);
	}
	GalPalette[GAL_PALETTE_NUM_COLOURS_BULLETS - 1 + GAL_PALETTE_BULLETS_OFFSET] = BurnHighCol(0xff, 0xff, 0x00, 0);
	
	for (int i = 0; i < 8; i++) {
		int r = BIT(i, 0) * 0x47;
		int g = BIT(i, 1) * 0x47;
		int b = BIT(i, 2) * 0x4f;
		GalPalette[i + GAL_PALETTE_BACKGROUND_OFFSET] = BurnHighCol(r, g, b, 0);
	}
}

#undef RGB_MAXIMUM
#undef MAX_NETS
#undef MAX_RES_PER_NET
#undef Combine_2Weights
#undef Combine_3Weights

// Background and Stars rendering
void GalaxianDrawBackground()
{
	if (GalStarsEnable) GalaxianRenderStarLayer();
}

void RockclimDrawBackground()
{
	int mx, my, Code, Colour, x, y, TileIndex = 0;

	for (my = 0; my < 32; my++) {
		for (mx = 0; mx < 64; mx++) {
			Code = GalVideoRam2[TileIndex];
			Colour = 0;
			
			x = 8 * mx;
			y = 8 * my;
			
			x -= RockclimScrollX & 0x1ff;
			y -= RockclimScrollY & 0xff;
			
			if (x < -8) x += 512;
			if (y < -8) y += 256;
			
			y -= 16;

			if (x > 8 && x < (nScreenWidth - 8) && y > 8 && y < (nScreenHeight - 8)) {
				Render8x8Tile(pTransDraw, Code, x, y, Colour, 4, 32, RockclimTiles);
			} else {
				Render8x8Tile_Clip(pTransDraw, Code, x, y, Colour, 4, 32, RockclimTiles);
			}

			TileIndex++;
		}
	}
}

void JumpbugDrawBackground()
{
	if (GalStarsEnable) JumpbugRenderStarLayer();
}

void FroggerDrawBackground()
{
	GalPalette[GAL_PALETTE_BACKGROUND_OFFSET] = BurnHighCol(0, 0, 0x47, 0);
	
	if (GalFlipScreenX) {
		for (int y = 0; y < nScreenHeight; y++) {
			for (int x = nScreenWidth - 1; x > 128 - 8; x--) {
				pTransDraw[(y * nScreenWidth) + x] = GAL_PALETTE_BACKGROUND_OFFSET;
			}
		}
	} else {
		for (int y = 0; y < nScreenHeight; y++) {
			for (int x = 0; x < 128 + 8; x++) {
				pTransDraw[(y * nScreenWidth) + x] = GAL_PALETTE_BACKGROUND_OFFSET;
			}
		}
	}
}

void TurtlesDrawBackground()
{
	GalPalette[GAL_PALETTE_BACKGROUND_OFFSET] = BurnHighCol(GalBackgroundRed * 0x55, GalBackgroundGreen * 0x47, GalBackgroundBlue * 0x55, 0);
	
	for (int y = 0; y < nScreenHeight; y++) {
		for (int x = 0; x < nScreenWidth; x++) {
			pTransDraw[(y * nScreenWidth) + x] = GAL_PALETTE_BACKGROUND_OFFSET;
		}
	}
}

void ScrambleDrawBackground()
{
	GalPalette[GAL_PALETTE_BACKGROUND_OFFSET] = BurnHighCol(0, 0, 0x56, 0);
	
	if (GalBackgroundEnable) {
		for (int y = 0; y < nScreenHeight; y++) {
			for (int x = 0; x < nScreenWidth; x++) {
				pTransDraw[(y * nScreenWidth) + x] = GAL_PALETTE_BACKGROUND_OFFSET;
			}
		}
	}
	
	if (GalStarsEnable) ScrambleRenderStarLayer();
}

void AnteaterDrawBackground()
{
	GalPalette[GAL_PALETTE_BACKGROUND_OFFSET] = BurnHighCol(0, 0, 0x56, 0);
	
	if (GalBackgroundEnable) {
		if (GalFlipScreenX) {
			for (int y = 0; y < nScreenHeight; y++) {
				for (int x = nScreenWidth - 1; x > 256 - 56; x--) {
					pTransDraw[(y * nScreenWidth) + x] = GAL_PALETTE_BACKGROUND_OFFSET;
				}
			}
		} else {
			for (int y = 0; y < nScreenHeight; y++) {
				for (int x = 0; x < 56; x++) {
					pTransDraw[(y * nScreenWidth) + x] = GAL_PALETTE_BACKGROUND_OFFSET;
				}
			}
		}
	}
}

void MarinerDrawBackground()
{
	unsigned char *BgColourProm = GalProm + 0x20;
	int x;

	if (GalFlipScreenX) {
		for (x = 0; x < 32; x++) {
			int Colour;
		
			if (x == 0) {
				Colour = 0;
			} else {
				Colour = BgColourProm[0x20 + x - 1];
			}
		
			int xStart = 8 * (31 - x);
			for (int sy = 0; sy < nScreenHeight; sy++) {
				for (int sx = xStart; sx < xStart + 8; sx++) {
					pTransDraw[(sy * nScreenWidth) + sx] = GAL_PALETTE_BACKGROUND_OFFSET + Colour;
				}
			}
		}
	} else {
		for (x = 0; x < 32; x++) {
			int Colour;
		
			if (x == 31) {
				Colour = 0;
			} else {
				Colour = BgColourProm[x + 1];
			}
		
			int xStart = x * 8;
			for (int sy = 0; sy < nScreenHeight; sy++) {
				for (int sx = xStart; sx < xStart + 8; sx++) {
					pTransDraw[(sy * nScreenWidth) + sx] = GAL_PALETTE_BACKGROUND_OFFSET + Colour;
				}
			}
		}
	}
	
	if (GalStarsEnable) MarinerRenderStarLayer();
}

void StratgyxDrawBackground()
{
	unsigned char *BgColourProm = GalProm + 0x20;
	
	for (int x = 0; x < 32; x++) {
		int xStart, Colour = 0;

		if ((~BgColourProm[x] & 0x02) && GalBackgroundRed)   Colour |= 0x01;
		if ((~BgColourProm[x] & 0x02) && GalBackgroundGreen) Colour |= 0x02;
		if ((~BgColourProm[x] & 0x01) && GalBackgroundBlue)  Colour |= 0x04;

		if (GalFlipScreenX) {
			xStart = 8 * (31 - x);
		} else {
			xStart = 8 * x;
		}
		
		for (int sy = 0; sy < nScreenHeight; sy++) {
			for (int sx = xStart; sx < xStart + 8; sx++) {
				pTransDraw[(sy * nScreenWidth) + sx] = GAL_PALETTE_BACKGROUND_OFFSET + Colour;
			}
		}
	}
}

void RescueDrawBackground()
{
	if (GalBackgroundEnable) {
		int x;

		for (x = 0; x < 128; x++) {
			for (int y = 0; y < nScreenHeight; y++) {
				pTransDraw[(y * nScreenWidth) + x] = GAL_PALETTE_BACKGROUND_OFFSET + x;
			}
		}
		
		for (x = 0; x < 120; x++) {
			for (int y = 0; y < nScreenHeight; y++) {
				pTransDraw[(y * nScreenWidth) + (x + 128)] = GAL_PALETTE_BACKGROUND_OFFSET + x + 8;
			}
		}
		
		for (x = 0; x < 8; x++) {
			for (int y = 0; y < nScreenHeight; y++) {
				pTransDraw[(y * nScreenWidth) + (x + 248)] = GAL_PALETTE_BACKGROUND_OFFSET;
			}
		}
	}

	if (GalStarsEnable) RescueRenderStarLayer();
}

void MinefldDrawBackground()
{
	if (GalBackgroundEnable) {
		int x;

		for (x = 0; x < 128; x++) {
			for (int y = 0; y < nScreenHeight; y++) {
				pTransDraw[(y * nScreenWidth) + x] = GAL_PALETTE_BACKGROUND_OFFSET + x;
			}
		}
		
		for (x = 0; x < 120; x++) {
			for (int y = 0; y < nScreenHeight; y++) {
				pTransDraw[(y * nScreenWidth) + (x + 128)] = GAL_PALETTE_BACKGROUND_OFFSET + x + 128;
			}
		}
		
		for (x = 0; x < 8; x++) {
			for (int y = 0; y < nScreenHeight; y++) {
				pTransDraw[(y * nScreenWidth) + (x + 248)] = GAL_PALETTE_BACKGROUND_OFFSET;
			}
		}
	}

	if (GalStarsEnable) RescueRenderStarLayer();
}

void DambustrDrawBackground()
{
	int xClipStart = GalFlipScreenX ? 254 - DambustrBgSplitLine : 0;
	int xClipEnd = GalFlipScreenX ? 0 : 254 - DambustrBgSplitLine;
	
	for (int x = 0; x < 256 - DambustrBgSplitLine; x++) {
		if (DambustrBgPriority && (x < xClipStart || x > xClipEnd)) continue;
		for (int y = 0; y < nScreenHeight; y++) {
			pTransDraw[(y * nScreenWidth) + x] = GAL_PALETTE_BACKGROUND_OFFSET + ((GalFlipScreenX) ? DambustrBgColour2 : DambustrBgColour1);
		}
	}
	
	for (int x = 255; x > 256 - DambustrBgSplitLine; x--) {
		if (DambustrBgPriority && (x < xClipStart || x > xClipEnd)) continue;
		for (int y = 0; y < nScreenHeight; y++) {
			pTransDraw[(y * nScreenWidth) + x] = GAL_PALETTE_BACKGROUND_OFFSET + ((GalFlipScreenX) ? DambustrBgColour1 : DambustrBgColour2);
		}
	}

	if (GalStarsEnable && !DambustrBgPriority) GalaxianRenderStarLayer();
}

// Char Layer rendering
static void GalRenderBgLayer(unsigned char *pVideoRam)
{
	int mx, my, Attr, Colour, x, y, TileIndex = 0, RamPos;
	unsigned short Code;
	
	for (my = 0; my < 32; my++) {
		for (mx = 0; mx < 32; mx++) {
			RamPos = TileIndex & 0x1f;
			Code = pVideoRam[TileIndex];
			Attr = GalSpriteRam[(RamPos * 2) + 1];
			Colour = Attr  & ((GalColourDepth == 3) ? 0x03 : 0x07);
			
			if (GalExtendTileInfoFunction) GalExtendTileInfoFunction(&Code, &Colour, Attr, RamPos);
			
			if (SfxTilemap) {
				x = 8 * my;
				y = 8 * mx;
			} else {
				x = 8 * mx;
				y = 8 * my;
			}
		
			y -= 16;
		
			if (GalFlipScreenX) x = nScreenWidth - 8 - x;
			if (GalFlipScreenY) y = nScreenHeight - 8 - y;
		
			int px, py;
		
			UINT32 nPalette = Colour << GalColourDepth;
		
			for (py = 0; py < 8; py++) {
				for (px = 0; px < 8; px++) {
					unsigned char c = GalChars[(Code * 64) + (py * 8) + px];
					if (GalFlipScreenX) c = GalChars[(Code * 64) + (py * 8) + (7 - px)];
					if (GalFlipScreenY) c = GalChars[(Code * 64) + ((7 - py) * 8) + px];
					if (GalFlipScreenX && GalFlipScreenY) c = GalChars[(Code * 64) + ((7 - py) * 8) + (7 - px)];
				
					if (c) {
						int xPos = x + px;
						int yPos = y + py;
					
						if (SfxTilemap) {
							if (GalFlipScreenX) {
								xPos += GalScrollVals[mx];
							} else {
								xPos -= GalScrollVals[mx];
							}
							
							if (xPos < 0) xPos += 256;
							if (xPos > 255) xPos -= 256;
						} else {
							if (GalFlipScreenY) {
								yPos += GalScrollVals[mx];
							} else {
								yPos -= GalScrollVals[mx];
							}
							
							if (yPos < 0) yPos += 256;
							if (yPos > 255) yPos -= 256;
						}
						
						if (GalOrientationFlipX) {
							xPos = nScreenWidth - 1 - xPos;
						}
					
						if (yPos >= 0 && yPos < nScreenHeight) {					
							unsigned short* pPixel = pTransDraw + (yPos * nScreenWidth);
						
							if (xPos >= 0 && xPos < nScreenWidth) {
								pPixel[xPos] = c | nPalette;
							}
						}
					}
				}
			}
		
			TileIndex++;
		}
	}
}

// Sprite Rendering
static void GalRenderSprites(const unsigned char *SpriteBase)
{
	int SprNum;
	int ClipOfs = GalFlipScreenX ? 16 : 0;
	int xMin = GalSpriteClipStart - ClipOfs;
	int xMax = GalSpriteClipEnd - ClipOfs + 1;
	
	for (SprNum = 7; SprNum >= 0; SprNum--) {
		const unsigned char *Base = &SpriteBase[SprNum * 4];
		unsigned char Base0 = FroggerAdjust ? ((Base[0] >> 4) | (Base[0] << 4)) : Base[0];
		int sy = 240 - (Base0 - (SprNum < 3));
		unsigned short Code = Base[1] & 0x3f;
		unsigned char xFlip = Base[1] & 0x40;
		unsigned char yFlip = Base[1] & 0x80;
		unsigned char Colour = Base[2] & ((GalColourDepth == 3) ? 0x03 : 0x07);
		int sx = Base[3];

		if (GalExtendSpriteInfoFunction) GalExtendSpriteInfoFunction(Base, &sx, &sy, &xFlip, &yFlip, &Code, &Colour);
		
		if (GalFlipScreenX) {
			sx = 242 - sx;
			xFlip = !xFlip;
		}
		
		if (sx < xMin || sx > xMax) continue;
		
		if (GalFlipScreenY) {
			sy = 240 - sy;
			yFlip = !yFlip;
		}
		
		sy -= 16;
		
		if (GalOrientationFlipX) {
			sx = 242 - 1 - sx;
			xFlip = !xFlip;
		}
		
		if (sx > 16 && sx < (nScreenWidth - 16) && sy > 16 && sy < (nScreenHeight - 16)) {
			if (xFlip) {
				if (yFlip) {
					Render16x16Tile_Mask_FlipXY(pTransDraw, Code, sx, sy, Colour, GalColourDepth, 0, 0, GalSprites);
				} else {
					Render16x16Tile_Mask_FlipX(pTransDraw, Code, sx, sy, Colour, GalColourDepth, 0, 0, GalSprites);
				}
			} else {
				if (yFlip) {
					Render16x16Tile_Mask_FlipY(pTransDraw, Code, sx, sy, Colour, GalColourDepth, 0, 0, GalSprites);
				} else {
					Render16x16Tile_Mask(pTransDraw, Code, sx, sy, Colour, GalColourDepth, 0, 0, GalSprites);
				}
			}
		} else {
			if (xFlip) {
				if (yFlip) {
					Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, Code, sx, sy, Colour, GalColourDepth, 0, 0, GalSprites);
				} else {
					Render16x16Tile_Mask_FlipX_Clip(pTransDraw, Code, sx, sy, Colour, GalColourDepth, 0, 0, GalSprites);
				}
			} else {
				if (yFlip) {
					Render16x16Tile_Mask_FlipY_Clip(pTransDraw, Code, sx, sy, Colour, GalColourDepth, 0, 0, GalSprites);
				} else {
					Render16x16Tile_Mask_Clip(pTransDraw, Code, sx, sy, Colour, GalColourDepth, 0, 0, GalSprites);
				}
			}
		}
	}
}

// Bullet rendering
static inline void GalDrawPixel(int x, int y, int Colour)
{
	if (y >= 0 && y < nScreenHeight && x >= 0 && x < nScreenWidth) {
		pTransDraw[(y * nScreenWidth) + x] = Colour;
	}
}

void GalaxianDrawBullets(int Offs, int x, int y)
{
	x -= 4;
	
	GalDrawPixel(x++, y, GAL_PALETTE_BULLETS_OFFSET + Offs);
	GalDrawPixel(x++, y, GAL_PALETTE_BULLETS_OFFSET + Offs);
	GalDrawPixel(x++, y, GAL_PALETTE_BULLETS_OFFSET + Offs);
	GalDrawPixel(x++, y, GAL_PALETTE_BULLETS_OFFSET + Offs);
}

void TheendDrawBullets(int Offs, int x, int y)
{
	x -= 4;
	
	GalPalette[GAL_PALETTE_BULLETS_OFFSET + 7] = BurnHighCol(0xff, 0x00, 0xff, 0);
	
	GalDrawPixel(x++, y, GAL_PALETTE_BULLETS_OFFSET + Offs);
	GalDrawPixel(x++, y, GAL_PALETTE_BULLETS_OFFSET + Offs);
	GalDrawPixel(x++, y, GAL_PALETTE_BULLETS_OFFSET + Offs);
	GalDrawPixel(x++, y, GAL_PALETTE_BULLETS_OFFSET + Offs);
}

void ScrambleDrawBullets(int, int x, int y)
{
	x -= 6;
	
	GalDrawPixel(x, y, GAL_PALETTE_BULLETS_OFFSET + 7);
}

void MoonwarDrawBullets(int, int x, int y)
{
	x -= 6;
	
	GalPalette[GAL_PALETTE_BULLETS_OFFSET + 7] = BurnHighCol(0xef, 0xef, 0x97, 0);
	
	GalDrawPixel(x, y, GAL_PALETTE_BULLETS_OFFSET + 7);
}

void MshuttleDrawBullets(int, int x, int y)
{
	GalPalette[GAL_PALETTE_BULLETS_OFFSET + 0] = BurnHighCol(0xff, 0xff, 0xff, 0);
	GalPalette[GAL_PALETTE_BULLETS_OFFSET + 1] = BurnHighCol(0xff, 0xff, 0x00, 0);
	GalPalette[GAL_PALETTE_BULLETS_OFFSET + 2] = BurnHighCol(0x00, 0xff, 0xff, 0);
	GalPalette[GAL_PALETTE_BULLETS_OFFSET + 3] = BurnHighCol(0x00, 0xff, 0x00, 0);
	GalPalette[GAL_PALETTE_BULLETS_OFFSET + 4] = BurnHighCol(0xff, 0x00, 0xff, 0);
	GalPalette[GAL_PALETTE_BULLETS_OFFSET + 5] = BurnHighCol(0xff, 0x00, 0x00, 0);
	GalPalette[GAL_PALETTE_BULLETS_OFFSET + 6] = BurnHighCol(0x00, 0x00, 0xff, 0);
	GalPalette[GAL_PALETTE_BULLETS_OFFSET + 7] = BurnHighCol(0x00, 0x00, 0x00, 0);
	
	--x;
	GalDrawPixel(x, y, ((x & 0x40) == 0) ? GAL_PALETTE_BULLETS_OFFSET +  + ((x >> 2) & 7) : GAL_PALETTE_BULLETS_OFFSET +  + 4);
	--x;
	GalDrawPixel(x, y, ((x & 0x40) == 0) ? GAL_PALETTE_BULLETS_OFFSET +  + ((x >> 2) & 7) : GAL_PALETTE_BULLETS_OFFSET +  + 4);
	--x;
	GalDrawPixel(x, y, ((x & 0x40) == 0) ? GAL_PALETTE_BULLETS_OFFSET +  + ((x >> 2) & 7) : GAL_PALETTE_BULLETS_OFFSET +  + 4);
	--x;
	GalDrawPixel(x, y, ((x & 0x40) == 0) ? GAL_PALETTE_BULLETS_OFFSET +  + ((x >> 2) & 7) : GAL_PALETTE_BULLETS_OFFSET +  + 4);
}

void DarkplntDrawBullets(int, int x, int y)
{
	if (GalFlipScreenX) x++;
	
	x -= 6;
	
	GalDrawPixel(x, y, GAL_PALETTE_BULLETS_OFFSET + DarkplntBulletColour);
}

void DambustrDrawBullets(int Offs, int x, int y)
{
	int Colour;
	
	if (GalFlipScreenX) x++;
	
	x -= 6;
	
	for (int i = 0; i < 2; i++) {
		if (Offs < 16) {
			Colour = GAL_PALETTE_BULLETS_OFFSET + 7;
			y--;
		} else {
			Colour = GAL_PALETTE_BULLETS_OFFSET;
			x--;
		}
	}
	
	GalDrawPixel(x, y, Colour);
}

static void GalDrawBullets(const unsigned char *Base)
{
	for (int y = 0; y < nScreenHeight; y++) {
		unsigned char Shell = 0xff;
		unsigned char Missile = 0xff;
		unsigned char yEff;
		int Which;
		
		yEff = (GalFlipScreenY) ? (y + 16 - 1) ^ 255 : y + 16 - 1;
		
		for (Which = 0; Which < 3; Which++) {
			if ((unsigned char)(Base[Which * 4 + 1] + yEff) == 0xff) Shell = Which;
		}
		
		yEff = (GalFlipScreenY) ? (y + 16) ^ 255 : y + 16;
		
		for (Which = 3; Which < 8; Which++) {
			if ((unsigned char)(Base[Which * 4 + 1] + yEff) == 0xff) {
				if (Which != 7) {
					Shell = Which;
				} else {
					Missile = Which;
				}
			}
		}
		
		if (Shell != 0xff) GalDrawBulletsFunction(Shell, (GalOrientationFlipX) ? Base[Shell * 4 + 3] : 255 - Base[Shell * 4 + 3], y);
		if (Missile != 0xff) GalDrawBulletsFunction(Missile, (GalOrientationFlipX) ? Base[Missile * 4 + 3] : 255 - Base[Missile * 4 + 3], y);
	}
}

// Render a frame
void GalDraw()
{
	if (GalRenderFrameFunction) {
		GalRenderFrameFunction();
	} else {
		BurnTransferClear();
		GalCalcPaletteFunction();
		if (GalRenderBackgroundFunction) GalRenderBackgroundFunction();
		GalRenderBgLayer(GalVideoRam);
		GalRenderSprites(&GalSpriteRam[0x40]);
		if (GalDrawBulletsFunction) GalDrawBullets(&GalSpriteRam[0x60]);
		BurnTransferCopy(GalPalette);
	}
}

void DkongjrmRenderFrame()
{
	BurnTransferClear();
	GalCalcPaletteFunction();
	if (GalRenderBackgroundFunction) GalRenderBackgroundFunction();
	GalRenderBgLayer(GalVideoRam);
	GalRenderSprites(&GalSpriteRam[0x40]);
	GalRenderSprites(&GalSpriteRam[0x60]);
	GalRenderSprites(&GalSpriteRam[0xc0]);
	GalRenderSprites(&GalSpriteRam[0xe0]);
	if (GalDrawBulletsFunction) GalDrawBullets(&GalSpriteRam[0x60]);
	BurnTransferCopy(GalPalette);
}

void DambustrRenderFrame()
{
	BurnTransferClear();
	GalCalcPaletteFunction();
	if (GalRenderBackgroundFunction) GalRenderBackgroundFunction();
	GalRenderBgLayer(GalVideoRam);
	GalRenderSprites(&GalSpriteRam[0x40]);
	if (GalDrawBulletsFunction) GalDrawBullets(&GalSpriteRam[0x60]);
	if (DambustrBgPriority) {
		if (GalRenderBackgroundFunction) GalRenderBackgroundFunction();
		memset(GalVideoRam2, 0x20, 0x400);
		for (int i = 0; i < 32; i++) {
			int Colour = GalSpriteRam[(i << 1) | 1] & 7;
			if (Colour > 3) {
				for (int j = 0; j < 32; j++) GalVideoRam2[(32 * j) + i] = GalVideoRam[(32 * j) + i];
			}
		}
		GalRenderBgLayer(GalVideoRam2);
	}	
	BurnTransferCopy(GalPalette);
}
