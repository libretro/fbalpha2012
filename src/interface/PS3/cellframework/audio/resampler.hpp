#ifndef SN_TARGET_PS3
#ifndef __AUDIO_RESAMPLER_HPP
#define __AUDIO_RESAMPLER_HPP

#include <deque>
#include <stddef.h>

namespace AudioStream {

class Resampler
{
   public:
      template <class T>
      Resampler(T& drain_obj) : callback(new Caller<T>(drain_obj)) {}
      virtual ~Resampler() { delete callback; }

      virtual ssize_t pull(float *out, size_t samples) = 0;

   protected:
      ssize_t read(float *out, size_t samples);

   private:
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
};

}

#endif
#endif
