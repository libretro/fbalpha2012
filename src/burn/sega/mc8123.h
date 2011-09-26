#ifndef MC8123_H
#define MC8123_H

void mc8123_decrypt_rom(int banknum, int numbanks, unsigned char *pRom, unsigned char *pFetch, unsigned char *pKey);

#endif
