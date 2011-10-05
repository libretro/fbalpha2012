#include "neogeo.h"
#include "typedefs.h"

// some of this is thanks to iq_132

void NeoSaveDecryptedCRoms()
{
	extern unsigned int nSpriteSize;
	extern unsigned int nNeoNumSpriteRom;
	
	if (!NeoSpriteROM) return;
	
	if (!(BurnDrvGetHardwareCode() & HARDWARE_SNK_ENCRYPTED_A) && !(BurnDrvGetHardwareCode() & HARDWARE_SNK_ENCRYPTED_B)) return;
	
	char szFileName[MAX_PATH];
	TCHAR szProgressText[50];
	
	if (!(nSpriteSize & 0xFFFFFF)) {
		for (unsigned int i = 1; i < nNeoNumSpriteRom + 1; i += 2) {
			_stprintf(szProgressText, _T("Saving decrypted C%d and C%d ROMs..."), i, i + 1);
			BurnUpdateProgress(0.0, szProgressText, 0);
			
			sprintf(szFileName, "%x%02x-c%i_decrypted.bin", Neo68KROM[0x109], Neo68KROM[0x108], i);
			FILE *C1 = fopen(szFileName, "wb");
			if (C1 == NULL) return;
			
			sprintf(szFileName, "%x%02x-c%i_decrypted.bin", Neo68KROM[0x109], Neo68KROM[0x108], i + 1);
			FILE *C2 = fopen(szFileName, "wb");
			if (C2 == NULL) return;
			
			unsigned int RomSize = nSpriteSize / nNeoNumSpriteRom;
			for (unsigned int j = 0; j < RomSize * 2; j += 2) {
				fwrite(NeoSpriteROM + ((i - 1) * RomSize) + j + 0, 1, 1, C1);
				fwrite(NeoSpriteROM + ((i - 1) * RomSize) + j + 1, 1, 1, C2);
			}
			
			fclose(C1);
			fclose(C2);
		}
	}
}
