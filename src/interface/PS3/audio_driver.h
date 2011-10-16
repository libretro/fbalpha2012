#ifndef __AUDIO_DRIVER_H
#define __AUDIO_DRIVER_H

#define SAMPLERATE_11KHZ 11025
#define SAMPLERATE_22KHZ 22050
#define SAMPLERATE_44KHZ 44100
#define SAMPLERATE_48KHZ 48010

extern int bAudSetSampleRate;
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
