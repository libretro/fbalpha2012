#ifndef SN_TARGET_PS3
#ifndef __AUDIO_QUAD_RESAMPLER_HPP
#define __AUDIO_QUAD_RESAMPLER_HPP

//#include "resampler.hpp"
#include <deque>
#include <stdint.h>

namespace AudioStream {

class Resampler
{
   public:
      template <class T>
      Resampler(T& drain_obj, double in_ratio, unsigned in_channels) 
      {
         callback = new Caller<T>(drain_obj);
         ratio = in_ratio;
         channels = in_channels;
         sum_output_samples = 0;
         sum_input_samples = 0;
      }
      
      ~Resampler()
      {
         delete callback;
      }

      ssize_t pull(float *out);
   private:
      //resampler base
      struct Callback
      {
         virtual ssize_t read(float **data) = 0;
         virtual ~Callback() {}
      };
      template<class T>
      struct Caller : public Callback
      {
         Caller(T& in) : obj(in) {}
         ssize_t read(float **data) { return obj(data); }
         T& obj;
      };
      Callback *callback;
      std::deque<float> buf;
      
      unsigned channels;
      uint64_t sum_output_samples;
      uint64_t sum_input_samples;
      double ratio;
      std::deque<float> data;

      inline size_t required_samples(size_t samples);
      inline void poly_create_3(float *poly, float *y);
      size_t process(float *out_data);
      ssize_t read(float *out, size_t samples)
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
};

}

#endif
#endif
