/*  RSound - A PCM audio client/server
 *  Copyright (C) 2010 - Hans-Kristian Arntzen
 * 
 *  RSound is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  RSound is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with RSound.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef RSD_RESAMPLER
#define RSD_RESAMPLER

#include "../common/celltypes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef u32 (*resampler_cb_t) (void *cb_data, float **data);

typedef struct resampler resampler_t;

resampler_t* resampler_new(resampler_cb_t func, double ratio, int channels, void* cb_data);
u32 resampler_cb_read(resampler_t *state, u32 frames, float *data);
void resampler_free(resampler_t* state);

static inline void resampler_float_to_s16(s16 * restrict out, const float * restrict in, u32 samples)
{
   for (u32 i = 0; i < samples; i++)
   {
      s32 temp = in[i] * 0x7FFF; 
      if (temp > 0x7FFE)
         out[i] = 0x7FFE;
      else if (temp < -0x8000)
         out[i] = -0x8000;
      else
         out[i] = (s16)temp;
   }
}

static inline void resampler_s16_to_float(float * restrict out, const s16 * restrict in, u32 samples)
{
   for (u32 i = 0; i < samples; i++)
      out[i] = (float)in[i]/0x8000;
}

#ifdef __cplusplus
}
#endif

#endif
