#ifndef __AUDIO_DRIVER_H
#define __AUDIO_DRIVER_H

extern bool bAudPlaying;

int audio_new(void);
int audio_exit(void);
int audio_init(void);
int audio_blank(void);
int audio_stop(void);
void audio_check(void);
void audio_play(void);

#endif
