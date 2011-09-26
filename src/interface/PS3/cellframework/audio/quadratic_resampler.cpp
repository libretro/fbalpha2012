#ifndef SN_TARGET_PS3
#include "quadratic_resampler.hpp"
#include <vector>

//hardcode channels
#define AUDIO_CHANNELS 2

namespace AudioStream {

inline size_t Resampler::required_samples(size_t samples)
{
   size_t after_sum = sum_output_samples + samples;

   size_t min_input_samples = (size_t)((after_sum / ratio) + 4);
   return ((min_input_samples - sum_input_samples)/AUDIO_CHANNELS) * AUDIO_CHANNELS;
}

inline void Resampler::poly_create_3(float *poly, float *y)
{
   poly[2] = (y[0] - 2*y[1] + y[2])/2;
   poly[1] = -1.5*y[0] + 2*y[1] - 0.5*y[2];
   poly[0] = y[0];
}

size_t Resampler::process(float *out_data)
{
   //hardcode samples
   size_t frames = 256;
   size_t frames_used = 0;
   uint64_t pos_out;
   double pos_in = 0.0;

   uint64_t sum_output_frames = sum_output_samples >> 1;
   uint64_t sum_input_frames = sum_input_samples >> 1;
   for (uint64_t x = sum_output_frames; x < sum_output_frames + frames; x++)
   {
      pos_out = x - sum_output_frames;
      pos_in  = ((double)x / ratio) - (double)sum_input_frames;
         
         float poly[3];
         float rdata[3];
         float x_val;

         if ((int)pos_in == 0)
         {
            rdata[0] = data[0];
            rdata[1] = data[AUDIO_CHANNELS + 0];
            rdata[2] = data[4 + 0];
            x_val = pos_in;
         }
         else
         {
            rdata[0] = data[((int)pos_in - 1) * AUDIO_CHANNELS + 0];
            rdata[1] = data[((int)pos_in + 0) * AUDIO_CHANNELS + 0];
            rdata[2] = data[((int)pos_in + 1) * AUDIO_CHANNELS + 0];
            x_val = pos_in - (int)pos_in + 1.0;
         }

         poly_create_3(poly, rdata);

         out_data[pos_out * AUDIO_CHANNELS + 0] = poly[2] * x_val * x_val + poly[1] * x_val + poly[0];

         float poly1[3];
         float rdata1[3];
         float x_val1;

         if ((int)pos_in == 0)
         {
            rdata1[0] = data[1];
            rdata1[1] = data[AUDIO_CHANNELS + 1];
            rdata1[2] = data[4 + 1];
            x_val1 = pos_in;
         }
         else
         {
            rdata1[0] = data[((int)pos_in - 1) * AUDIO_CHANNELS + 1];
            rdata1[1] = data[((int)pos_in + 0) * AUDIO_CHANNELS + 1];
            rdata1[2] = data[((int)pos_in + 1) * AUDIO_CHANNELS + 1];
            x_val1 = pos_in - (int)pos_in + 1.0;
         }

         poly_create_3(poly1, rdata1);

         out_data[pos_out * AUDIO_CHANNELS + 1] = poly1[2] * x_val1 * x_val1 + poly1[1] * x_val1 + poly1[0];
   }
   frames_used = (int)pos_in;
   return frames_used * AUDIO_CHANNELS;
}

ssize_t Resampler::pull(float *out)
{
   //Hardcode samples - 256 x 2 = 512

   // How many samples must we have to resample?
   size_t req_samples = required_samples(512);

   // Do we need to read more data?
   ssize_t ret;
   if (data.size() < req_samples)
   {
      size_t must_read = req_samples - data.size();
      std::vector<float> buffer(must_read);
      ret = read(&buffer[0], must_read);

      if (ret <= 0) // We're done.
         return -1;
      data.insert(data.end(), buffer.begin(), buffer.end());
   }

   // Phew. We should have enough data in our buffer now to be able to process the data we need.

   size_t samples_used = process(out);
   sum_input_samples += samples_used;
   data.erase(data.begin(), data.begin() + samples_used);
   sum_output_samples += 512;

   return 512;
}

}
#endif
