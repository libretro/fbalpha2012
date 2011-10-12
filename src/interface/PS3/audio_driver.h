#ifndef __AUDIO_DRIVER_H
#define __AUDIO_DRIVER_H

#define AUDIO_SEGMENT_LENGTH 801
#define AUDIO_SEGMENT_LENGTH_TIMES_CHANNELS 1602

extern bool bAudPlaying;
extern bool bAudReinit;

int audio_new(void);
int audio_exit(void);
int audio_init(int samplerate);

#define audio_check(audio_samples) driver->write(audio_handle, pBurnSoundOut, audio_samples);

#define audio_play() bAudPlaying = true;
#define audio_stop() !(bAudPlaying = false)
// Write silence into the buffer
#define audio_blank() if (pAudNextSound) memset(pAudNextSound, 0, nAudAllocSegLen);
#endif
