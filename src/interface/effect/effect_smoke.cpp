
#include "effect.h"
#include "burner.h"

#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define ABS(x)    (((x) >= 0) ? (x) : ( - (x)))

#define FIRE_HOTSPOTS   80

int fire_hotspot[FIRE_HOTSPOTS];

extern BYTE vidbuffer[];

BYTE fire_line  [SCRW];
BYTE fire_buffer[SCRW * SCRH];

int fire_init_flag = 0;
WORD fire_pal[256];

void build_pal()
{
  int i;
  for(i = 0; i < 32; i++) fire_pal[i] = MAKE_COLOR(i / 4, i / 2, i);
  for( ; i < 256; i++)    fire_pal[i] = MAKE_COLOR(31, 31, 31);
}

void draw_bottom_line_of_fire()
{
  int count, count2;

  memset(&fire_line, 0, SCRW);

  for(count = 0; count < FIRE_HOTSPOTS; count++)
  {
    for(count2 = (fire_hotspot [count] - 20);
        count2 < (fire_hotspot [count] + 20); count2++)
    {
      if(count2 >= 0 && count2 < SCRW)
      {
        fire_line[count2] = MIN((fire_line [count2] + 20) -
          ABS(fire_hotspot[count] - count2), 256);
      }
    }

    fire_hotspot[count] += (rand() & 7) - 3;

    if     (fire_hotspot[count] < 0)     fire_hotspot [count] += SCRW;
    else if(fire_hotspot[count] >= SCRW) fire_hotspot [count] -= SCRW;
  }

  for(count = 0; count < SCRW; count ++)
    fire_buffer[((SCRH - 1) * (SCRW)) + count] = fire_line[count];
}

void init_fire()
{
  int x, y, pixel, count;

  build_pal();

  srand(time(0));

  for(count = 0; count < FIRE_HOTSPOTS; count++)
      fire_hotspot [count] = (rand () % SCRW);

  for(count = 0; count < SCRH; count++)
  {
    draw_bottom_line_of_fire();

    for(y = 0; y < (SCRH - 1); y ++)
    {
      for(x = 0; x < SCRW; x ++)
      {
        pixel = fire_buffer [((y + 1) * SCRW) + x];
        if(pixel > 0) pixel--;
        fire_buffer[(y * SCRW) + x] = pixel;
      }
    }
  }

  fire_init_flag = 1;
}

void UpdateSmokeEffect()
{
  int x, y, pixel, pixel2, i;

  if(!fire_init_flag) init_fire();

  draw_bottom_line_of_fire();

  for(y = 0; y < (SCRH - 1); y++)
  {
    for(x = 0; x < SCRW; x++)
    {
      pixel = fire_buffer [((y + 1) * SCRW) + x];
      if(pixel > 0) pixel--;
      fire_buffer[(y * SCRW) + x] = pixel;
    }
  }

  i = 0;
  for(y = 0; y < SCRH; y++)
  {
    for(x = 0; x < SCRW; x ++)
    {
      pixel  = vidbuffer[i];
      pixel2 = (fire_buffer[i] / 8);

      if(pixel2 > pixel) vidbuffer[i] = pixel2;
      else               vidbuffer[i] = (((pixel + pixel2) / 2) + 1);

      pEffect[i] = fire_pal[vidbuffer[i]];
      i++;
    }
  }
}
