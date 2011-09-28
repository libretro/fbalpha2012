#ifndef __LIBSNES_TYPEDEFS_H
#define __LIBSNES_TYPEDEFS_H

#include <stdint.h>

#define XMemSet(x,y,z) memset(x,y,z)
#define _T(x) x
#define __forceinline	__attribute((always_inline))

typedef uint8_t UINT8;
typedef uint32_t UINT32;
#endif
