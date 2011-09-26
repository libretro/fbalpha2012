// IPS

/* changelog:
 update 1: create
*/

#ifndef NO_IPS

#include "burner.h"

#define IPS_SIGNATURE	"PATCH"
#define IPS_TAG_EOF		"EOF"

#define BYTE3_TO_UINT(bp) \
     (((unsigned int)(bp)[0] << 16) & 0x00FF0000) | \
     (((unsigned int)(bp)[1] << 8) & 0x0000FF00) | \
     ((unsigned int)(bp)[2] & 0x000000FF)

#define BYTE2_TO_UINT(bp) \
    (((unsigned int)(bp)[0] << 8) & 0xFF00) | \
    ((unsigned int) (bp)[1] & 0x00FF)

void applyIPSpatch(const char* ips_path, long ips_size, unsigned char* base, int base_len)
{
	FILE* f = NULL;
	if ((f = fopen(ips_path, "rb")) == NULL) {
		return;
	}

	char buf[6];
	memset(buf, 0, sizeof buf);
	fread(buf, 1, 5, f);
	if (strcmp(buf, IPS_SIGNATURE)) {
		fclose(f);
		return;
	}

	int Offset, Size;
	UINT8* mem8 = NULL;
	UINT8 ch = 0;
	int bRLE = 0;

	while (!feof(f)) {
		// read patch address offset
		fread(buf, 1, 3, f);
		buf[3] = 0;
		if (strcmp(buf, IPS_TAG_EOF) == 0)
			break;

		Offset = BYTE3_TO_UINT(buf);

		// read patch length
		fread(buf, 1, 2, f);
		Size = BYTE2_TO_UINT(buf);

		bRLE = (Size == 0);
		if (bRLE) {
			fread(buf, 1, 2, f);
			Size = BYTE2_TO_UINT(buf);
			ch = fgetc(f);
		}

#if 0
		if ((base_size > 0) && (Size + Offset > base_size))
			break; // out of range
#endif

		while (Size--) {
			mem8 = base + Offset;
			Offset++;
			*mem8 = bRLE ? ch : fgetc(f);
		}
	}

	fclose(f);
}

#endif
