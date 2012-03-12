#ifndef _FBA_ENDIAN_H
#define _FBA_ENDIAN_H

typedef union {
  struct { UINT8 h3,h2,h,l; } b;
  struct { UINT16 h,l; } w;
  UINT32 d;
} PAIR;

/*   PS3 doesn't seem to have byteswap 64-bit intrinsic, so define this union
     here for later use */
#ifdef __CELLOS_LV2__
typedef union {
   UINT64 ll;
   struct { UINT32 l, h; } l;
} BYTESWAP_INT64;
#endif

/* Xbox 360 */
#if defined(_XBOX)
#define BURN_ENDIAN_SWAP_INT8(x)				(x^1)
#define BURN_ENDIAN_SWAP_INT16(x)				(_byteswap_ushort(x))
#define BURN_ENDIAN_SWAP_INT32(x)				(_byteswap_ulong(x))
#define BURN_ENDIAN_SWAP_INT64(x)				(_byteswap_uint64(x))
/* PlayStation3 */
#elif defined(__CELLOS_LV2__)
#include <ppu_intrinsics.h>
#define BURN_ENDIAN_SWAP_INT8(x)				(x^1)
#define BURN_ENDIAN_SWAP_INT16(x)				({uint16_t t; __sthbrx(&t, x); t;})
#define BURN_ENDIAN_SWAP_INT32(x)				({uint32_t t; __stwbrx(&t, x); t;})
static inline UINT64 BURN_ENDIAN_SWAP_INT64(UINT64 x)
{
	BYTESWAP_INT64 r = {0};
	r.l.l = BURN_ENDIAN_SWAP_INT32(x);
	r.l.h = BURN_ENDIAN_SWAP_INT32(x >> 32);
	return r.ll;
}
#endif

#endif
