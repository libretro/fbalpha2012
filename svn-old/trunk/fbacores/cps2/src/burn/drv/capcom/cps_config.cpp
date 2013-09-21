#include "cps.h"

struct GfxRange {
	INT32 Type;
	INT32 Start;
	INT32 End;
	INT32 Bank;
};

static const struct GfxRange *GfxBankMapper = NULL;
static INT32 GfxBankSizes[4] = { 0, 0, 0, 0 };

static const struct GfxRange mapper_cps2_table[] =
{
	{ GFXTYPE_SCROLL1 | GFXTYPE_SCROLL2 | GFXTYPE_SCROLL3 | GFXTYPE_SPRITES, 0x00000, 0x1ffff, 1 },
	{ 0                                                                    ,       0,       0, 0 }
};


void SetGfxMapper(INT32 MapperId)
{
   GfxBankSizes[0] = 0x20000;
   GfxBankSizes[1] = 0x20000;
   GfxBankSizes[2] = 0x00000;
   GfxBankSizes[3] = 0x00000;
   GfxBankMapper = mapper_cps2_table;
}

INT32 GfxRomBankMapper(INT32 Type, INT32 Code)
{
	const struct GfxRange *Range = GfxBankMapper;
	INT32 Shift = 0;

	switch (Type) {
		case GFXTYPE_SPRITES: Shift = 1; break;
		case GFXTYPE_SCROLL1: Shift = 0; break;
		case GFXTYPE_SCROLL2: Shift = 1; break;
		case GFXTYPE_SCROLL3: Shift = 3; break;
	}

	Code <<= Shift;

	while (Range->Type) {
		if (Code >= Range->Start && Code <= Range->End)	{
			if (Range->Type & Type)	{
				INT32 Base = 0;
				INT32 i;

				for (i = 0; i < Range->Bank; ++i)
					Base += GfxBankSizes[i];

				return (Base + (Code & (GfxBankSizes[Range->Bank] - 1))) >> Shift;
			}
		}

		++Range;
	}

//	bprintf(PRINT_NORMAL, _T("tile %02x/%04x out of range\n"), Type,Code>>Shift);

	return -1;
}

void SetCpsBId(INT32 CpsBId, INT32 bStars)
{
   CpsBID[0]   = 0x32;
   CpsBID[1]   = 0x00;
   CpsBID[2]   = 0x00;

   CpsMProt[0] = 0x40;
   CpsMProt[1] = 0x42;
   CpsMProt[2] = 0x44;
   CpsMProt[3] = 0x46;

   nCpsLcReg   = 0x66;
   MaskAddr[0] = 0x68;
   MaskAddr[1] = 0x6a;
   MaskAddr[2] = 0x6c;
   MaskAddr[3] = 0x6e;

   nCpsPalCtrlReg = 0x70;

   CpsLayEn[1] = 0x02;
   CpsLayEn[2] = 0x04;
   CpsLayEn[3] = 0x08;
}
