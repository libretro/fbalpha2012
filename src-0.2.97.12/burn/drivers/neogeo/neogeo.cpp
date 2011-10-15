#include "neogeo.h"
// Neo Geo -- misc. support functions

unsigned char nNeoProtectionXor;
unsigned int nNeoNumSpriteRom;

// This function loads the 68K ROMs
int NeoLoadCode(int nOffset, int nNum, unsigned char* pDest)
{
	struct BurnRomInfo ri;

	for (int i = 0; i < nNum; i++) {
		ri.nLen = 0;
		BurnDrvGetRomInfo(&ri, nOffset + i);

		if ((BurnDrvGetHardwareCode() & HARDWARE_SNK_P32) && (i == 0))
		{
			if (BurnLoadRom(pDest + 0, nOffset + i + 0, 2)) return 1;
			if (BurnLoadRom(pDest + 1, nOffset + i + 1, 2)) return 1;
 
			for (unsigned int j = 0; j < ri.nLen << 1; j+=4)
				BurnByteswap(pDest + j + 1, 2);

			i++;
			pDest += ri.nLen << 1;
			continue;
		}

		if (BurnLoadRom(pDest, nOffset + i, 1)) {
			return 1;
		}

		if ((BurnDrvGetHardwareCode() & HARDWARE_SNK_SWAPP) && (i == 0)) {
			for (unsigned int j = 0; j < (ri.nLen / 2); j++) {
				int k = pDest[j];
				pDest[j] = pDest[j + (ri.nLen / 2)];
				pDest[j + (ri.nLen / 2)] = k;
			}
		}

		pDest += ri.nLen;
	}

	return 0;
}

// This function loads and pre-processes the sprite data
int NeoLoadSprites(int nOffset, int nNum, unsigned char* pDest, unsigned int nSpriteSize)
{
	struct BurnRomInfo ri;
	ri.nType = 0;
	ri.nLen = 0;

	int ROM32MB = 0;
	unsigned int nRomSize = 0;
	
	nNeoNumSpriteRom = nNum;

	if (BurnDrvGetHardwareCode() & (HARDWARE_SNK_ENCRYPTED_A | HARDWARE_SNK_ENCRYPTED_B)) {
		if (BurnDrvGetHardwareCode() & HARDWARE_SNK_ENCRYPTED_A) {
			NeoGfxDecryptCMC42Init();
		} else {
			if (BurnDrvGetHardwareCode() & HARDWARE_SNK_ENCRYPTED_B) {
				NeoGfxDecryptCMC50Init();
			}
		}

		nRomSize = ((nSpriteSize / nNum) & 0xf00000) ? 0x1000000 : 0x2000000;
		unsigned char* pBuf = (unsigned char *)malloc( nRomSize );
		if (pBuf == NULL) {
			return 1;
		}

		for (int i = 0; i < ((nNum + ROM32MB) >> 1); i++) {
			if ((nSpriteSize / nNum) == 0x2000000) {		// svcpcb
				ROM32MB = 2;
				BurnLoadRom(pBuf, nOffset + i, 1);

				pcb_gfx_crypt(pBuf, 0);
			} else {						// standard
				BurnLoadRom(pBuf + 0, nOffset + 0 + (i << 1), 2);
				BurnLoadRom(pBuf + 1, nOffset + 1 + (i << 1), 2);

				if (nRomSize == 0x2000000) {			// ms5pcb, svcpcba, kf2k3pcb
					for (int j = 0; j < 0x2000000; j+=4)
						BurnByteswap(pBuf + j + 1, 2);

					pcb_gfx_crypt(pBuf, nNum & 2);
				}		
			}

			BurnUpdateProgress(1.0 / ((double)(nSpriteSize/0x800000) * 8.0 / (nRomSize / 0x400000) / 3.0), _T("Decrypting graphics..."), 0);
			NeoGfxDecryptDoBlock(nNeoProtectionXor, pBuf, nRomSize * i, nRomSize, nSpriteSize);
		}

		free(pBuf);

	} else {
		nSpriteSize = 0;

		// Compute correct size to gaps into account (Kizuna)
		for (int i = 0; i < nNum - 2; i++) {
			BurnDrvGetRomInfo(&ri, nOffset + i);
			if (ri.nLen > nRomSize) {
				nRomSize = ri.nLen;
			}
		}

		for (int i = 0; i < (nNum >> 1); i++) {
			BurnLoadRom(pDest + nSpriteSize + 0, nOffset + (i << 1), 2);
			BurnLoadRom(pDest + nSpriteSize + 1, nOffset + 1 + (i << 1), 2);

			BurnDrvGetRomInfo(&ri, nOffset + (i << 1));

			if (i < (nNum >> 1) - 1) {
				nSpriteSize += nRomSize << 1;
			} else {
				nSpriteSize += ri.nLen << 1;
			}
		}

	 	// Swap data for viewpoin, aof, ssideki, kotm2, more
	 	if (BurnDrvGetHardwareCode() & HARDWARE_SNK_SWAPC) {
			for (int i = 0; i < 0x200000; i++) {
				unsigned char n = pDest[i + 0x400000];
				pDest[i + 0x400000] = pDest[i + 0x200000];
				pDest[i + 0x200000] = n;
	 		}
	 	}
	}

	return 0;
}

void NeoDecodeSprites(unsigned char* pDest, int nSize)
{	
	for (int i = 0; i < 8; i++) {

		unsigned char* pStart = pDest + i * (nSize >> 3);
		unsigned char* pEnd = pStart + (nSize >> 3);

		{
			int nStep = 8;
			if (BurnDrvGetHardwareCode() & (HARDWARE_SNK_ENCRYPTED_A | HARDWARE_SNK_ENCRYPTED_B)) {
				nStep *= 4;
			}
			BurnUpdateProgress(1.0 / nStep, i ? NULL : _T("Preprocessing graphics..."), 0);
		}

		// Pre-process the sprite graphics
		for (unsigned char* pTile = pStart; pTile < pEnd; pTile += 128) {
			unsigned int data[32];

			for (int y = 0; y < 16; y++) {
				unsigned int n = 0;
				for (int x = 0; x < 8; x++) {
					unsigned int m = ((pTile[67 + (y << 2)] >> x) & 1) << 3;
					m |= ((pTile[65 + (y << 2)] >> x) & 1) << 2;
					m |= ((pTile[66 + (y << 2)] >> x) & 1) << 1;
					m |= ((pTile[64 + (y << 2)] >> x) & 1) << 0;

					n |= m << (x << 2);
				}
				data[(y << 1) + 0] = n;

				n = 0;
				for (int x = 0; x < 8; x++) {
					unsigned int m = ((pTile[3 + (y << 2)] >> x) & 1) << 3;
					m |= ((pTile[1 + (y << 2)] >> x) & 1) << 2;
					m |= ((pTile[2 + (y << 2)] >> x) & 1) << 1;
					m |= ((pTile[0 + (y << 2)] >> x) & 1) << 0;

					n |= m << (x << 2);
				}
				data[(y << 1) + 1] = n;
			}
			for (int n = 0; n < 32; n++) {
				((unsigned int*)pTile)[n] = data[n];
			}
		}
	}
}

void NeoDecodeText(unsigned char* pDest, int nSize)
{
	// Pre-process the text layer graphics
	for (unsigned char* pTile = pDest; pTile < (pDest + 0x20000 + nSize); pTile += 32) {
		unsigned char data[32];
		for (int n = 0; n < 8; n++) {
			data[0 + n * 4] = pTile[16 + n];
			data[1 + n * 4] = pTile[24 + n];
			data[2 + n * 4] = pTile[ 0 + n];
			data[3 + n * 4] = pTile[ 8 + n];
		}

		for (int n = 0; n < 32; n++) {
			pTile[n] = data[n] << 4;
			pTile[n] |= data[n] >> 4;
		}
	}
}

int NeoLoadADPCM(int nOffset, int nNum, unsigned char* pDest)
{
	struct BurnRomInfo ri;
	ri.nLen = 0;
	BurnDrvGetRomInfo(&ri, nOffset);

	for (int i = 0; i < nNum; i++) {
		BurnLoadRom(pDest, nOffset + i, 1);
		pDest += ri.nLen;
	}

	return 0;
}

// This function fills the screen with the first palette entry
void NeoClearScreen()
{
	unsigned int nColour = NeoPalette[0x0FFF];

	if (nColour) {
		switch (nBurnBpp) {
			case 4: {
				unsigned int* pClear = (unsigned int*)pBurnDraw;
				for (int i = 0; i < nNeoScreenWidth * 224 / 8; i++) {
					*pClear++ = nColour;
					*pClear++ = nColour;
					*pClear++ = nColour;
					*pClear++ = nColour;
					*pClear++ = nColour;
					*pClear++ = nColour;
					*pClear++ = nColour;
					*pClear++ = nColour;
				}
				break;
			}

			case 3: {
				unsigned char* pClear = pBurnDraw;
				unsigned char r = nColour;
				unsigned char g = (r >> 8) & 0xFF;
				unsigned char b = (r >> 16) & 0xFF;
				r &= 0xFF;
				for (int i = 0; i < nNeoScreenWidth * 224; i++) {
					*pClear++ = r;
					*pClear++ = g;
					*pClear++ = b;
				}
				break;
			}

			case 2: {
				unsigned int* pClear = (unsigned int*)pBurnDraw;
				nColour |= nColour << 16;
				for (int i = 0; i < nNeoScreenWidth * 224 / 16; i++) {
					*pClear++ = nColour;
					*pClear++ = nColour;
					*pClear++ = nColour;
					*pClear++ = nColour;
					*pClear++ = nColour;
					*pClear++ = nColour;
					*pClear++ = nColour;
					*pClear++ = nColour;
				}
				break;
			}
		}
	} else {
		memset(pBurnDraw, 0, nNeoScreenWidth * 224 * nBurnBpp);
	}
}

