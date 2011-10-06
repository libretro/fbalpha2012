#ifndef __AUDIO_DRIVER_H
#define __AUDIO_DRIVER_H

#define AUDIO_SEGMENT_LENGTH 801
#define AUDIO_SEGMENT_LENGTH_TIMES_CHANNELS 1602

extern bool bAudPlaying;

int audio_new(void);
int audio_exit(void);
int audio_init(void);
int audio_blank(void);
int audio_stop(void);
void audio_play(void);

#define audio_check() \
pBurnSoundOut = pAudNextSound; \
driver->write(audio_handle, pBurnSoundOut, AUDIO_SEGMENT_LENGTH_TIMES_CHANNELS);

#endif
