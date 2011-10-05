// Driver State Compression module
#include "zlib.h"

#include "burnint.h"

static unsigned char* Comp = NULL;		// Compressed data buffer
static int nCompLen = 0;
static int nCompFill = 0;				// How much of the buffer has been filled so far

static z_stream Zstr;					// Deflate stream

// -----------------------------------------------------------------------------
// Compression

static int CompEnlarge(int nAdd)
{
	void* NewMem = NULL;

	// Need to make more room in the compressed buffer
	NewMem = realloc(Comp, nCompLen + nAdd);
	if (NewMem == NULL) {
		return 1;
	}

	Comp = (unsigned char*)NewMem;
	memset(Comp + nCompLen, 0, nAdd);
	nCompLen += nAdd;

	return 0;
}

static int CompGo(int bFinish)
{
	int nResult = 0;
	int nAvailOut = 0;

	bool bRetry, bOverflow;

	do {

		bRetry = false;

		// Point to the remainder of out buffer
		Zstr.next_out = Comp + nCompFill;
		nAvailOut = nCompLen - nCompFill;
		if (nAvailOut < 0) {
			nAvailOut = 0;
		}
		Zstr.avail_out = nAvailOut;

		// Try to deflate into the buffer (there may not be enough room though)
		if (bFinish) {
			nResult = deflate(&Zstr, Z_FINISH);					// deflate and finish
			if (nResult != Z_OK && nResult != Z_STREAM_END) {
				return 1;
			}
		} else {
			nResult = deflate(&Zstr, 0);						// deflate
			if (nResult != Z_OK) {
				return 1;
			}
		}

		nCompFill = Zstr.next_out - Comp;						// Update how much has been filled

		// Check for overflow
		bOverflow = bFinish ? (nResult == Z_OK) : (Zstr.avail_out <= 0);

		if (bOverflow) {
			if (CompEnlarge(4 * 1024)) {
				return 1;
			}

			bRetry = true;
		}
	} while (bRetry);

	return 0;
}

static int __cdecl StateCompressAcb(struct BurnArea* pba)
{
	// Set the data as the next available input
	Zstr.next_in = (unsigned char*)pba->Data;
	Zstr.avail_in = pba->nLen;

	CompGo(0);													// Compress this Area

	Zstr.avail_in = 0;
	Zstr.next_in = NULL;

	return 0;
}

// Compress a state using deflate
int BurnStateCompress(unsigned char** pDef, int* pnDefLen, int bAll)
{
	void* NewMem = NULL;

	memset(&Zstr, 0, sizeof(Zstr));

	Comp = NULL; nCompLen = 0; nCompFill = 0;					// Begin with a zero-length buffer
	if (CompEnlarge(8 * 1024)) {
		return 1;
	}

	deflateInit(&Zstr, Z_DEFAULT_COMPRESSION);

	BurnAcb = StateCompressAcb;									// callback our function with each area

	if (bAll) BurnAreaScan(ACB_FULLSCAN | ACB_READ, NULL);		// scan all ram, read (from driver <- decompress)
	else      BurnAreaScan(ACB_NVRAM    | ACB_READ, NULL);		// scan nvram,   read (from driver <- decompress)

	// Finish off
	CompGo(1);

	deflateEnd(&Zstr);

	// Size down
	NewMem = realloc(Comp, nCompFill);
	if (NewMem) {
		Comp = (unsigned char*)NewMem;
		nCompLen = nCompFill;
	}

	// Return the buffer
	if (pDef) {
		*pDef = Comp;
	}
	if (pnDefLen) {
		*pnDefLen = nCompFill;
	}

	return 0;
}

// -----------------------------------------------------------------------------
// Decompression

static int __cdecl StateDecompressAcb(struct BurnArea* pba)
{
	Zstr.next_out =(unsigned char*)pba->Data;
	Zstr.avail_out = pba->nLen;

	inflate(&Zstr, Z_SYNC_FLUSH);

	Zstr.avail_out = 0;
	Zstr.next_out = NULL;

	return 0;
}

int BurnStateDecompress(unsigned char* Def, int nDefLen, int bAll)
{
	memset(&Zstr, 0, sizeof(Zstr));
	inflateInit(&Zstr);

	// Set all of the buffer as available input
	Zstr.next_in = (unsigned char*)Def;
	Zstr.avail_in = nDefLen;

	BurnAcb = StateDecompressAcb;								// callback our function with each area

	if (bAll) BurnAreaScan(ACB_FULLSCAN | ACB_WRITE, NULL);		// scan all ram, write (to driver <- decompress)
	else      BurnAreaScan(ACB_NVRAM    | ACB_WRITE, NULL);		// scan nvram,   write (to driver <- decompress)

	inflateEnd(&Zstr);
	memset(&Zstr, 0, sizeof(Zstr));

	return 0;
}
