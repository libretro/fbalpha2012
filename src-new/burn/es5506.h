/**********************************************************************************************
 *
 *   Ensoniq ES5505/6 driver
 *   by Aaron Giles
 *
 **********************************************************************************************/

//#pragma once

#ifndef __ES5506_H__
#define __ES5506_H__

//#include "devlegcy.h"

typedef void (*irq_callback)(int param);
typedef UINT16(port_read)();

typedef struct _es5505_interface es5505_interface;
struct _es5505_interface
{
	const char * region0;						/* memory region where the sample ROM lives */
	const char * region1;						/* memory region where the sample ROM lives */
	void (*irq_callback)(int state);	/* irq callback */
	UINT16 (*read_port)();			/* input port read */
};

//READ16_DEVICE_HANDLER( es5505_r );
//WRITE16_DEVICE_HANDLER( es5505_w );
//void es5505_voice_bank_w(device_t *device, int voice, int bank);

//DECLARE_LEGACY_SOUND_DEVICE(ES5505, es5505);


typedef struct _es5506_interface es5506_interface;
struct _es5506_interface
{
	const char * region0;						/* memory region where the sample ROM lives */
	const char * region1;						/* memory region where the sample ROM lives */
	const char * region2;						/* memory region where the sample ROM lives */
	const char * region3;						/* memory region where the sample ROM lives */
	void (*irq_callback)(int state);	/* irq callback */
	UINT16 (*read_port)();			/* input port read */
};

//READ8_DEVICE_HANDLER( es5506_r );
//WRITE8_DEVICE_HANDLER( es5506_w );
//void es5506_voice_bank_w(device_t *device, int voice, int bank);

//DECLARE_LEGACY_SOUND_DEVICE(ES5506, es5506);

void ES5506Update(short *pBuffer, int samples);
#define ES5505Update ES5506Update
void ES5506Init(int clock, unsigned char*region0, unsigned char*region1, unsigned char*region2, unsigned char*region3, irq_callback callback);
void ES5506Exit();
void ES5506Reset();
void ES5506Write(unsigned int offset, UINT8 data);
UINT8 ES5506Read(unsigned int offset);
void es5506_voice_bank_w(int voice, int bank);
void ES5505Init(int clock, unsigned char*region0, unsigned char*region1, irq_callback callback);
#define ES5505Reset	ES5506Reset
#define ES5505Exit	ES5505Exit
void ES5505Write(unsigned int offset, UINT16 data);
UINT16 ES5505Read(unsigned int offset);
void es5505_voice_bank_w(int voice, int bank);

#endif /* __ES5506_H__ */
