// FB Alpha sample player module

#include "burnint.h"
#include "direct.h"
#include "samples.h"

#define SAMPLE_DIRECTORY	szAppSamplesPath

#define get_long()	((ptr[3] << 24) | (ptr[2] << 16) | (ptr[1] << 8) | (ptr[0] << 0))
#define get_short()	((ptr[1] << 8) | (ptr[0] << 0))

static int bAddToStream = 0;
static int nTotalSamples = 0;
static int nSampleSetGain = 100;

struct sample_format
{
	unsigned char *data;
	unsigned int length;
	unsigned int position;
	unsigned char playing;
	unsigned char loop;
	unsigned char flags;
};

static struct sample_format *samples		= NULL; // store samples
static struct sample_format *sample_ptr		= NULL; // generic pointer for sample

static void make_raw(unsigned char *src, unsigned int len)
{
	unsigned char *ptr = src;

	if (ptr[0] != 'R' || ptr[1] != 'I' || ptr[2] != 'F' || ptr[3] != 'F') return;
	ptr += 4; // skip RIFF

	unsigned int length = get_long();	ptr += 4; // total length of file
	if (len < length) length = len - 8;		  // first 8 bytes (RIFF + Len)

	/* "WAVEfmt " */			ptr += 8; // WAVEfmt + 1 space
	unsigned int length2 = get_long();	ptr += 4; // Wavefmt length
/*	unsigned short format = get_short();  */ptr += 2; // format?
	unsigned short channels = get_short();	ptr += 2; // channels
	unsigned int sample_rate = get_long();	ptr += 4; // sample rate
/*	unsigned int speed = get_long();      */ptr += 4; // speed - should equal (bits * channels * sample_rate)
/*	unsigned short align = get_short();   */ptr += 2; // block align	should be ((bits / 8) * channels)
	unsigned short bits = get_short() / 8;	ptr += 2; // bits per sample	(0010)
	ptr += length2 - 16;				  // get past the wave format chunk

	// are we in the 'data' chunk? if not, skip this chunk.
	if (ptr[0] != 'd' || ptr[1] != 'a' || ptr[2] != 't' || ptr[3] != 'a') {
		ptr += 4; // skip tag

		unsigned int length3 = get_long(); ptr += 4;
		ptr += length3;
	}

	/* "data" */				ptr += 4; // "data"
	unsigned int data_length = get_long();	ptr += 4; // should be up to the data...

	if ((len - (ptr - src)) < data_length) data_length = len - (ptr - src);

	unsigned int converted_len = (unsigned int)((float)(data_length * (nBurnSoundRate * 1.00000 / sample_rate) / (bits * channels)));
	if (converted_len == 0) return; 

	sample_ptr->data = (unsigned char*)malloc(converted_len * 4);

	// up/down sample everything and convert to raw 16 bit stereo
	{
		short *data = (short*)sample_ptr->data;
		short *poin = (short*)ptr;
		unsigned char *poib = ptr;
	
		for (unsigned int i = 0; i < converted_len; i++)
		{
			unsigned int x = (unsigned int)((float)(i * (sample_rate * 1.00000 / nBurnSoundRate)));

			if (bits == 2) {						//  signed 16 bit, stereo & mono
				data[i * 2 + 0] = poin[x * channels + 0             ];
				data[i * 2 + 1] = poin[x * channels + (channels / 2)];
			}

			if (bits == 1) {						// unsigned 8 bit, stereo & mono
				data[i * 2 + 0] = (poib[x * channels + 0             ] - 128) << 8;
				data[i * 2 + 1] = (poib[x * channels + (channels / 2)] - 128) << 8;
			}
		}

		// now go through and set the gain
		for (unsigned int i = 0; i < converted_len * 2; i++)
		{
			int d = (data[i] * nSampleSetGain) / 100;
			if (d >  0x7fff) d =  0x7fff;
			if (d < -0x7fff) d = -0x7fff;
			data[i] = (short)d;
		}
	}

	sample_ptr->length = converted_len;
	sample_ptr->playing = 0;
	sample_ptr->position = 0;
}

void BurnSamplePlay(int sample)
{
	if (sample >= nTotalSamples) return;

	sample_ptr = &samples[sample];

	if (sample_ptr->flags & SAMPLE_IGNORE) return;

	sample_ptr->playing = 1;
	sample_ptr->position = 0;
}

void BurnSamplePause(int sample)
{
	if (sample >= nTotalSamples) return;

	sample_ptr = &samples[sample];
	sample_ptr->playing = 0;
}

void BurnSampleResume(int sample)
{
	if (sample >= nTotalSamples) return;

	sample_ptr = &samples[sample];
	sample_ptr->playing = 1;
}

void BurnSampleStop(int sample)
{
	if (sample >= nTotalSamples) return;

	sample_ptr = &samples[sample];
	sample_ptr->playing = 0;
	sample_ptr->position = 0;
}

void BurnSampleSetLoop(int sample, bool dothis)
{
	if (sample >= nTotalSamples) return;

	sample_ptr = &samples[sample];

	if (sample_ptr->flags & SAMPLE_NOLOOP) return;

	sample_ptr->loop = (dothis ? 1 : 0);
}

int BurnSampleGetStatus(int sample)
{
	if (sample >= nTotalSamples) return -1;

	sample_ptr = &samples[sample];
	return (sample_ptr->playing);
}

int BurnSampleGetPosition(int sample)
{
	if (sample >= nTotalSamples) return -1;

	sample_ptr = &samples[sample];
	return (sample_ptr->position);
}

void BurnSampleSetPosition(int sample, unsigned int position)
{
	if (sample >= nTotalSamples) return;

	sample_ptr = &samples[sample];
	sample_ptr->position = position;
}

void BurnSampleReset()
{
	for (int i = 0; i < nTotalSamples; i++) {
		BurnSampleStop(i);

		if (sample_ptr->flags & SAMPLE_AUTOLOOP) {
			BurnSampleSetLoop(i, true);
			BurnSamplePlay(i);
		}
	}
}

extern int __cdecl ZipLoadOneFile(const char* arcName, const char* fileName, void** Dest, int* pnWrote);
char* TCHARToANSI(const TCHAR* pszInString, char* pszOutString, int nOutSize);
#define _TtoA(a)	TCHARToANSI(a, NULL, 0)

void BurnSampleInit(int nGain /*volume percentage!*/, int bAdd /*add sample to stream?*/)
{
	if (nBurnSoundRate == 0) {
		nTotalSamples = 0;
		return;
	}

	int length;
	char path[256];
	char setname[128];
	void *destination = NULL;
	char szTempPath[MAX_PATH];
	sprintf(szTempPath, _TtoA(SAMPLE_DIRECTORY));

	strcpy(setname, BurnDrvGetTextA(DRV_SAMPLENAME));
	sprintf(path, "%s%s.zip", szTempPath, setname);
	
	FILE *test = fopen(path, "rb");
	if (!test) return;
	fclose(test);

	bAddToStream = bAdd;
	nSampleSetGain = nGain;
	nTotalSamples = 0;

	struct BurnSampleInfo si;
	int nSampleOffset = -1;
	do {
		BurnDrvGetSampleInfo(&si, ++nSampleOffset);
		if (si.nFlags) nTotalSamples++;
	} while (si.nFlags);
	
	samples = (sample_format*)malloc(sizeof(sample_format) * nTotalSamples);
	memset (samples, 0, sizeof(sample_format) * nTotalSamples);

	for (int i = 0; i < nTotalSamples; i++) {
		BurnDrvGetSampleInfo(&si, i);
		char *szSampleName = NULL;
		BurnDrvGetSampleName(&szSampleName, i, 0);
		sample_ptr = &samples[i];

		if (si.nFlags == 0) break;

		sprintf (path, "%s%s.zip", szTempPath, setname);

		destination = NULL;
		length = 0;
		ZipLoadOneFile((const char*)path, (const char*)szSampleName, &destination, &length);
		
		if (length) {
			make_raw((unsigned char*)destination, length);

			sample_ptr->flags = si.nFlags;
		} else {
			sample_ptr->flags = SAMPLE_IGNORE;
		}

		if (destination) {
			free (destination);
		}

		destination = NULL;
	}
}

void BurnSampleExit()
{
	for (int i = 0; i < nTotalSamples; i++) {
		sample_ptr = &samples[i];
		if (sample_ptr->data != NULL) {
			free (sample_ptr->data);
			sample_ptr->data = NULL;
		}
	}

	if (samples) {
		free (samples);
		samples = NULL;
	}

	sample_ptr = NULL;
	nTotalSamples = 0;
	bAddToStream = 0;
	nSampleSetGain = 100;
}

void BurnSampleRender(short *pDest, unsigned int pLen)
{
	if (pBurnSoundOut == NULL) return;

	int nFirstSample = 0;
	unsigned int *dest = (unsigned int*)pDest;

	for (int i = 0; i < nTotalSamples; i++)
	{
		sample_ptr = &samples[i];
		if (sample_ptr->playing == 0) continue;

		int playlen = pLen;
		int loop = sample_ptr->loop;
		int length = sample_ptr->length;
		int position = sample_ptr->position;

		unsigned int *data = (unsigned int*)sample_ptr->data;

		if (loop) {
			if (bAddToStream == 0 && nFirstSample == 0) {
				for (int j = 0; j < playlen; j++, position++) {
					dest[j] = data[position % length];
				}
			} else {
				position *= 2;
				length *= 2;
				short *dst = (short*)dest;
				short *dat = (short*)data;

				for (int j = 0; j < playlen; j++, position+=2, dst += 2) {
					int t0 = dst[0] + dat[(position + 0) % length];
					int t1 = dst[1] + dat[(position + 1) % length];

					if (t0 > 0x7fff) t0 = 0x7fff;
					if (t1 > 0x7fff) t1 = 0x7fff;
					if (t0 < -0x7fff) t0 = -0x7fff;
					if (t1 < -0x7fff) t1 = -0x7fff;
					dst[0] = t0;
					dst[1] = t1;
				}
			}
		} else {
			length = length - position;

			if (length <= 0) {
				if (loop == 0) {
					sample_ptr->playing = 0;
					continue;
				}
			}

			data += position;
			if (playlen > length) playlen = length;

			if (bAddToStream == 0 && nFirstSample == 0) {
				for (int j = 0; j < playlen; j++) {
					dest[j] = data[j];
				}
			} else {
				short *dst = (short*)dest;
				short *dat = (short*)data;

				for (int j = 0; j < playlen; j++, dst +=2, dat+=2) {
					int t0 = dst[0] + dat[0];
					int t1 = dst[1] + dat[1];

					if (t0 > 0x7fff) t0 = 0x7fff;
					if (t1 > 0x7fff) t1 = 0x7fff;
					if (t0 < -0x7fff) t0 = -0x7fff;
					if (t1 < -0x7fff) t1 = -0x7fff;
					dst[0] = t0;
					dst[1] = t1;
				}
			}
		}

		sample_ptr->position += playlen;
		nFirstSample++;
	}
}

int BurnSampleScan(int nAction, int *pnMin)
{
	if (pnMin != NULL) {
		*pnMin = 0x029707;
	}

	if (nAction & ACB_DRIVER_DATA) {
		for (int i = 0; i < nTotalSamples; i++) {
			sample_ptr = &samples[i];
			SCAN_VAR(sample_ptr->playing);
			SCAN_VAR(sample_ptr->loop);
			SCAN_VAR(sample_ptr->position);
		}
	}

	return 0;
}
