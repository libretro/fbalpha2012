#define ARGB(r, g, b) ((r << 16) | (g << 8) | b)
#define RGB15(r, g, b) (((r << 7) & 0x7c00) | ((g << 2) & 0x03e0) | ((b >> 3) & 0x001f))
#define RGB16(r, g, b) (((r << 8) & 0xf800) | ((g << 3) & 0x07e0) | ((b >> 3) & 0x001f))
