#ifndef SN_TARGET_PS3
#include "resampler.hpp"
#include <stddef.h>
#include <vector>
#include <algorithm>

namespace AudioStream {

ssize_t Resampler::read(float *out, size_t samples)
{
   while (buf.size() < samples)
   {
      float *data;
      ssize_t size = callback->read(&data);
      if (size <= 0)
         return size;
      if (data == NULL)
         return -1;

      buf.insert(buf.end(), data, data + size);
   }

   std::copy(buf.begin(), buf.begin() + samples, out);
   buf.erase(buf.begin(), buf.begin() + samples);
   return samples;
}

}
#endif
