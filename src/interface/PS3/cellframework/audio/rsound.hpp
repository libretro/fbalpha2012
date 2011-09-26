#ifndef __AUDIO_RSOUND_H
#define __AUDIO_RSOUND_H

#include "stream.hpp"
#include "rsound.h"
#include <string>
#include "../network/network.hpp"

namespace AudioStream {

template <class T>
class RSound : public Stream<T>, public Network::NetworkInterface
{
   public:
      RSound(std::string server, int channels, int samplerate, int buffersize = 8092, int latency = 64)
      {
         rsd_init(&rd);
         int format = type_to_format(T());
         rsd_set_param(rd, RSD_FORMAT, &format);
         rsd_set_param(rd, RSD_CHANNELS, &channels);
         rsd_set_param(rd, RSD_HOST, const_cast<char*>(server.c_str()));
         rsd_set_param(rd, RSD_SAMPLERATE, &samplerate);
         buffersize *= sizeof(T);
         rsd_set_param(rd, RSD_BUFSIZE, &buffersize);
         rsd_set_param(rd, RSD_LATENCY, &latency);
         m_latency = latency;

         int rc = rsd_start(rd);
         if (rc < 0) // Couldn't start, don't do anything after this, might implement some proper error handling here.
         {
            runnable = false;
         }
         else
         {
            runnable = true;
         }
         emptybuf = (uint8_t*)memalign(128, buffersize);
         memset(emptybuf, 0, buffersize);
      }

      ~RSound()
      {
         free(emptybuf);
         rsd_stop(rd);
         rsd_free(rd);
      }

      size_t write(const T* in, size_t samples)
      {
         if (!runnable)
            return 0;

         rsd_delay_wait(rd);
         size_t rc = rsd_write(rd, in, samples * sizeof(T))/sizeof(T);
         
         // Close to underrun, fill up buffer
         if (rsd_delay_ms(rd) < m_latency / 2)
         {
            size_t size = rsd_get_avail(rd);
            rsd_write(rd, emptybuf, size);
         }
         return rc;
      }

      bool alive() const
      {
         return runnable;
      }

      size_t write_avail()
      {
         if (!runnable)
            return 0;

         // We'll block
         if (rsd_delay_ms(rd) > m_latency)
            return 0;

         return rsd_get_avail(rd) / sizeof(T);
      }

      void pause()
      {
         if (runnable)
         {
            runnable = false;
            rsd_pause(rd, 1);
         }
      }

      void unpause()
      {
         if (!runnable)
         {
            if (rsd_pause(rd, 0) < 0)
               runnable = false;
            else
               runnable = true;
         }
      }

   private:
      bool runnable;
      rsound_t *rd;
      int m_latency;
      uint8_t *emptybuf;

      int type_to_format(uint8_t) { return RSD_U8; }
      int type_to_format(int8_t) { return RSD_S8; }
      int type_to_format(int16_t) { return RSD_S16_NE; }
      int type_to_format(uint16_t) { return RSD_U16_NE; }
};

}

#endif
