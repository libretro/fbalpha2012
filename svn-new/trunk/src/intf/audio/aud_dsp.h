// dsp.cpp

#ifndef _FBA_INTF_AUDIO_DSP_H
#define _FBA_INTF_AUDIO_DSP_H

INT32 DspInit();
INT32 DspExit();
INT32 DspDo(INT16* Wave, INT32 nCount);

#endif
