#ifndef _LIBSNES_OPTIMIZATIONS_H_
#define _LIBSNES_OPTIMIZATIONS_H_

#define LIBSNES_COLOR_15BPP_BGR(color) ((((color & 0x1f) << 10) | (((color & 0x3e0) >> 5) << 5) | (((color & 0x7c00) >> 10))) & 0x7fff)

#endif
