// FB Alpha Plus! Neo-Geo ROMs saving driver module
// Module version: v0.4a
// Whole driver module written by IQ_132 - http://neosource.1emulation.com/.
// ===============================================================
// Revision History:
// - Version 0.4a
//   Trimmed C-Saving routine and optimized it (Thanks to IQ_132).
// ---------------------------------------------------------------
// - Version 0.4
//   Renamed some kludges and added routines for some new games.
// ---------------------------------------------------------------
// - Version 0.3
//   Fixed svcchaos c1 and c2 code (again, thanks to Jimmy_Page)
//   Added a code to save a 16mb V1-ROM for kof2003 (thanks to Jimmy_page)
// ---------------------------------------------------------------
// - Version 0.2
//   Fixed C-Decryption for svcchaos.
//   Added the hability for naming the files from the cartridge ID.
//   Fixed interleaving for using less RAM.
//   Changed V-ROM saving routine.
// ---------------------------------------------------------------
// - Version 0.1
//   First version.
//   Added some tweaks for C-Decryption saving.
// ===============================================================

#include "neogeo.h"

int bsavedecryptedcs = 0;
int bsavedecryptedps = 0;
int bsavedecrypteds1 = 0;
int bsavedecryptedvs = 0;
int bsavedecryptedm1 = 0;


// 8 C-ROMs for standard ROMsets, number will vary
void SaveDecCROMs()
{
	extern unsigned int nSpriteSize;

	if (NeoSpriteROM == NULL) {
		return;
	}

	TCHAR szNames[64] = _T("");
	char name[64] = "";
	int divby = 0x1000000;

	BurnUpdateProgress(0.0, _T("Initializing save routine..."), 0);

	if (!(nSpriteSize & 0xFFFFFF)) {
		if (!strcmp(BurnDrvGetTextA(DRV_NAME), "svcpcb")) divby = 0x4000000;
		if (!strcmp(BurnDrvGetTextA(DRV_NAME), "kf2k3pcb")) divby = 0x2000000;

		for (unsigned int i = 0; i < (nSpriteSize / divby); i++) {
			_stprintf(szNames, _T("Saving decrypted C%d and C%d ROMs..."), ((i*2)+1), ((i*2)+2));
			BurnUpdateProgress(0.0, szNames, 0);

			sprintf(name, "%X%2.2X-c%d_decrypted.bin", Neo68KROM[0x109], Neo68KROM[0x108], ((i*2)+1));
			FILE* C1 = fopen(name, "wb");
			sprintf(name, "%X%2.2X-c%d_decrypted.bin", Neo68KROM[0x109], Neo68KROM[0x108], ((i*2)+2));
			FILE* C2 = fopen(name, "wb");

			if (C1 && C2) {
				for (int j = 0; j < divby; j += 2) {
					fwrite(NeoSpriteROM + i * divby + j + 0, 1, 1, C1);
					fwrite(NeoSpriteROM + i * divby + j + 1, 1, 1, C2);
				}
				fclose(C1);
				fclose(C2);
			}
		}
		return;
	}
}

// For compatibility issues, only 1 P-ROM will be saved
void SaveDecPROM()
{
	extern unsigned int nCodeSize;

	char name[64] = "";

	BurnUpdateProgress(0.0, _T("Saving decrypted P-ROM..."), 0);
	sprintf (name, "%X%2.2X-p1_decrypted.bin", Neo68KROM[0x109], Neo68KROM[0x108]);
	FILE* file = fopen(name, "wb");
	if (file) {
		fwrite(Neo68KROM, 1, nCodeSize, file);
		fclose(file);
	}
}

// Either extracted from C Data or descrambled text roms
void SaveDecSROM()
{
	char name[64] = "";

	BurnUpdateProgress(0.0, _T("Saving decrypted S1-ROM..."), 0);
	sprintf (name, "%X%2.2X-s1_decrypted.bin", Neo68KROM[0x109], Neo68KROM[0x108]);
	FILE* file = fopen(name, "wb");
	if (file) {
		fwrite(NeoTextROM + 0x020000, 1, nNeoTextROMSize, file);
		fclose(file);
	}
}

// Standard decrypted samples are saved with this
void SaveDecVROMs(int nNumber)
{
	extern unsigned char* YM2610ADPCMAROM;
	extern int nYM2610ADPCMASize;

	char name[64] = "";

	// Special handler for unique V-ROM
	if (nNumber == 1) {
		BurnUpdateProgress(0.0, _T("Saving decrypted V1 ROM..."), 0);
		sprintf (name, "%X%2.2X-v1_decrypted.bin", Neo68KROM[0x109], Neo68KROM[0x108]);
		FILE* V1ROM = fopen(name, "wb");
		if (V1ROM) {
			fwrite(YM2610ADPCMAROM, 1, nYM2610ADPCMASize, V1ROM);
			fclose(V1ROM);
		}
	} else if (nNumber == 3) {
		BurnUpdateProgress(0.0, _T("Saving decrypted V1 V2 V3 ROM..."), 0);
		sprintf (name, "%X%2.2X-v1_decrypted.bin", Neo68KROM[0x109], Neo68KROM[0x108]);
		FILE* V1ROM = fopen(name, "wb");
		if (V1ROM) {
			fwrite(YM2610ADPCMAROM, 1, 0x400000, V1ROM);
			fclose(V1ROM);
		}
		sprintf (name, "%X%2.2X-v2_decrypted.bin", Neo68KROM[0x109], Neo68KROM[0x108]);
		FILE* V2ROM = fopen(name, "wb");
		if (V2ROM) {
			fwrite(YM2610ADPCMAROM + 0x400000, 1, 0x400000, V2ROM);
			fclose(V2ROM);
		}
		sprintf (name, "%X%2.2X-v3_decrypted.bin", Neo68KROM[0x109], Neo68KROM[0x108]);
		FILE* V3ROM = fopen(name, "wb");
		if (V3ROM) {
			fwrite(YM2610ADPCMAROM + 0x800000, 1, 0x200000, V3ROM);
			fclose(V3ROM);
		}
	} else {
		BurnUpdateProgress(0.0, _T("Saving decrypted V1 and V2 ROMs..."), 0);
		sprintf (name, "%X%2.2X-v1_decrypted.bin", Neo68KROM[0x109], Neo68KROM[0x108]);
		FILE* V1ROM = fopen(name, "wb");
		if (V1ROM) {
			fwrite(YM2610ADPCMAROM, 1, 0x800000, V1ROM);
			fclose(V1ROM);
		}
		sprintf (name, "%X%2.2X-v2_decrypted.bin", Neo68KROM[0x109], Neo68KROM[0x108]);
		FILE* V2ROM = fopen(name, "wb");
		if (V2ROM) {
			fwrite(YM2610ADPCMAROM + 0x800000, 1, 0x800000, V2ROM);
			fclose(V2ROM);
		}
	}
}

void SaveDecM1ROM()
{
	char name[64] = "";

	BurnUpdateProgress(0.0, _T("Saving decrypted M1-ROM..."), 0);
	sprintf (name, "%X%2.2X-m1_decrypted.bin", Neo68KROM[0x109], Neo68KROM[0x108]);
	FILE* file = fopen(name, "wb");
	if (file) {
		fwrite(NeoZ80ROM, 1, nNeoM1ROMSize, file);
		fclose(file);
	}
}
