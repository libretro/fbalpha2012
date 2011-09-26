#ifndef __CELL_STREAM_H
#define __CELL_STREAM_H

#include "../common/celltypes.h"

typedef void* cell_audio_handle_t;


typedef u32 (*cell_audio_sample_cb_t)(s16 * out, u32 samples, void * userdata);

struct cell_audio_params
{
   u32 channels; // Audio channels.
   u32 samplerate; // Audio samplerate.
   u32 buffer_size; // Desired buffer size in bytes, if 0, a sane default will be provided.

   cell_audio_sample_cb_t sample_cb; // Can choose to use a callback for audio rather than blocking interface with write/write_avail. If this is not NULL, callback will be used. If NULL, you have to write audio using write/write_avail.

   void *userdata; // Custom userdata that is passed to sample_cb.

   const char *device; // Only used for RSound atm for IP address, etc.
};

typedef struct cell_audio_driver
{
   cell_audio_handle_t (*init)(const struct cell_audio_params *params);

   s32 (*write)(cell_audio_handle_t handle, const s16* data, u32 samples);
   u32 (*write_avail)(cell_audio_handle_t handle);

   void (*pause)(cell_audio_handle_t handle);
   s32 (*unpause)(cell_audio_handle_t handle);
   u32 (*is_paused)(cell_audio_handle_t handle);

   void (*free)(cell_audio_handle_t handle);
} cell_audio_driver_t;


extern const cell_audio_driver_t cell_audio_audioport;
extern const cell_audio_driver_t cell_audio_rsound;

#endif
