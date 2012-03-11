#ifndef _FBA_ENDIAN_H
#define _FBA_ENDIAN_H

typedef union {
  struct { UINT8 h3,h2,h,l; } b;
  struct { UINT16 h,l; } w;
  UINT32 d;
} PAIR;

#endif