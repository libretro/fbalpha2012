#ifndef __CELL_STREAM_H
#define __CELL_STREAM_H

#define CELL_AUDIO_BLOCK_SAMPLES_X2 512
#define SAMPLES_TO_FRAMES(x) ((x) >> 1)
#define FRAMES_TO_SAMPLES(x) ((x) << 1)

typedef void* cell_audio_handle_t;

struct cell_audio_params
{
   uint32_t channels; // Audio channels.
   uint32_t samplerate; // Audio samplerate.
   uint32_t buffer_size; // Desired buffer size in bytes, if 0, a sane default will be provided.

   void *userdata; // Custom userdata that is passed to sample_cb.
   const char *device; // Only used for RSound atm for IP address, etc.
};

typedef struct cell_audio_driver
{
   cell_audio_handle_t (*init)(const struct cell_audio_params *params);

   int32_t (*write)(cell_audio_handle_t handle, const int16_t* data, uint32_t samples);
   //uint32_t (*write_avail)(cell_audio_handle_t handle);

   void (*pause)(cell_audio_handle_t handle);
   int32_t (*unpause)(cell_audio_handle_t handle);
   uint32_t (*is_paused)(cell_audio_handle_t handle);

   void (*free)(cell_audio_handle_t handle);
} cell_audio_driver_t;

extern const cell_audio_driver_t cell_audio_audioport;
extern const cell_audio_driver_t cell_audio_rsound;

#endif
