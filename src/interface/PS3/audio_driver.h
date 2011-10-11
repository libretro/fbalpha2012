#ifndef __AUDIO_DRIVER_H
#define __AUDIO_DRIVER_H

extern bool bAudPlaying;
extern int nAudAllocSegLen;
extern int nAudSize;

int audio_new(void);
int audio_exit(void);
int audio_init(void);

#define audio_check(audiosize) \
pBurnSoundOut = pAudNextSound; \
driver->write(audio_handle, pBurnSoundOut, audiosize);

#define audio_play() bAudPlaying = true;
#define audio_stop() !(bAudPlaying = false)
// Write silence into the buffer
#define audio_blank() if (pAudNextSound) memset(pAudNextSound, 0, nAudAllocSegLen);
#endif
