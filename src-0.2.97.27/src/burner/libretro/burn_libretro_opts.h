#ifndef _LIBRETRO_OPTIMIZATIONS_H_
#define _LIBRETRO_OPTIMIZATIONS_H_

#define LIBRETRO_COLOR_15BPP_BGR(color) ((((color & 0x1f) << 10) | (((color & 0x3e0) >> 5) << 5) | (((color & 0x7c00) >> 10))) & 0x7fff)

#define LIBRETRO_COLOR_15BPP_BGR565(colors) (((colors & 0xf800) >> 11) | ((colors & 0x07c0) >> 1) | ((colors & 0x1f) << 10))

#endif
