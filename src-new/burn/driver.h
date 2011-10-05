/*
 * For the MAME sound cores
 */

#ifndef DRIVER_H
#define DRIVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#if !defined (_WIN32)
 #define __cdecl
#endif

#ifndef INLINE
 #define INLINE __inline static
#endif

#define FBA

typedef signed char INT8;
typedef unsigned char UINT8;
typedef signed short INT16;
typedef unsigned short UINT16;
typedef signed int INT32;
typedef unsigned int UINT32;
typedef signed long long INT64;
typedef unsigned long long UINT64;
#define OSD_CPU_H

/* OPN */
#define HAS_YM2203  1
#define HAS_YM2608  1
#define HAS_YM2610  1
#define HAS_YM2610B 1
#define HAS_YM2612  1
#define HAS_YM3438  1
/* OPL */
#define HAS_YM3812  1
#define HAS_YM3526  1
#define HAS_Y8950   1

enum {
	CLEAR_LINE = 0,
	ASSERT_LINE,
	HOLD_LINE,
	PULSE_LINE
};

#define timer_get_time() BurnTimerGetTime()

#define READ8_HANDLER(name) 	UINT8 name(void)
#define WRITE8_HANDLER(name) 	void  name(UINT8 data)

#ifdef __cplusplus
 extern "C" {
#endif
  double BurnTimerGetTime(void);

  typedef unsigned char (*read8_handler)(unsigned int offset);
  typedef void (*write8_handler)(unsigned int offset,unsigned int data);

 #ifdef MAME_USE_LOGERROR
  void __cdecl logerror(char* szFormat, ...);
 #else
  #define logerror
 #endif
#ifdef __cplusplus
 }
#endif

#endif /* DRIVER_H */
