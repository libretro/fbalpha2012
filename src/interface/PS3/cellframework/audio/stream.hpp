#ifndef __AUDIO_STREAM_H
#define __AUDIO_STREAM_H

#include <stddef.h>
#include "../threads/thread_fifo.hpp"
#include "../threads/cond.hpp"
#include "../threads/thread.hpp"
#include <cell/audio.h>
#include <algorithm>
#include <cell/sysmodule.h>
#include <limits>
#include "../utility/ref_counted.hpp"

namespace AudioStream {
   
#define AUDIO_CHANNELS (2)
#define AUDIO_OUT_RATE (48000.0)
#define AUDIO_BLOCKS (8)
#define FIFO_SIZE 8092

   namespace Internal {
      
      template <class C>
      inline void array_to_float(float * __restrict__ out, const C * __restrict__ in)
      {
         //frames is 256, samples is frames * 2
         size_t samples = 512;
         for (size_t i = 0; i < 512; i++)
            out[i] = (float)in[i]/(std::numeric_limits<C>::max() + 1);
      }      

      class AudioPortRef : public ref_counted<AudioPortRef>
      {
         public:
            AudioPortRef()
            {
               if (ref() == 0)
               {
                  cellSysmoduleLoadModule(CELL_SYSMODULE_AUDIO);
                  cellAudioInit();
               }
               ref()++;
            }

            ~AudioPortRef()
            {
               if (ref() == 1)
               {
                  cellSysmoduleUnloadModule(CELL_SYSMODULE_AUDIO);
                  cellAudioQuit();
               }
               ref()--;
            }
      };
   }


/*
template<class T>
class Stream
{
   public:

      Stream() : m_fn(Internal::array_to_float<T>) {}

      // Returns number of samples you can write without blocking.
      virtual size_t write_avail() = 0;
      // Writes 'samples' samples to buffer. Will block until everything is written.
      virtual size_t write(const T* in, size_t samples) = 0;
      // Notifies interface that you will not be writing more data to buffer until unpause();
      virtual void pause() {};
      // Notifies interface that you would like to start writing data again.
      virtual void unpause() {};
      // Returns false if initialization failed. It is possible that that a pause()/unpause() sequence will remedy this.
      virtual bool alive() const { return true; }
      // Sets a callback for converting T frames of audio to a floating point 2 channel [-1.0, 1.0] frames. It is called on from a thread, so make sure code is thread safe. This callback needs to be used if you use an audio format that is not 2 channels of audio, or your sample format is not trivial to translate for floating point. This can be changed on the fly to allow for using self defined DSP effects, etc.
      virtual void set_float_conv_func(void (*fn)(float * out, const T* in, size_t frames))
      {
         Threads::ScopedLock foo(lock);
         m_fn = fn;
      }

      virtual ~Stream() {}
   protected:
      void convert_frames(float *out, const T* in, size_t frames)
      {
         Threads::ScopedLock foo(lock);
         m_fn(out, in, frames);
      }
   private:
      void (*m_fn)(float *, const T*, size_t);
      Threads::Mutex lock;
};
*/

template<class T>
class AudioPort : public Internal::AudioPortRef
{
   public:
      AudioPort(unsigned channels, unsigned samplerate, size_t num_samples = 8092) : fifo(Threads::ThreadFifo<T>(num_samples)), quit_thread(false), input_rate(samplerate), fifo_size(num_samples), m_chan(channels)
      {
         CellAudioPortParam params;

         params.nChannel = AUDIO_CHANNELS;
         params.nBlock = AUDIO_BLOCKS;
         params.attr = 0;

         cellAudioPortOpen(&params, &audio_port);

         cellAudioPortStart(audio_port);
         out_buffer = (float*)memalign(128, 512 * sizeof(float));
         thread = new Threads::Thread(&AudioPort<T>::event_loop, this);
      }

      size_t write_avail()
      {
         return fifo.write_avail();
      }

      size_t write(const T* in, size_t samples)
      {
         while (samples > 0)
         {
            size_t write_amount = samples > FIFO_SIZE ? FIFO_SIZE : samples;

            size_t written = 0;
            for(;;)
            {
               size_t wrote = fifo.write(in, write_amount - written);
               written += wrote;
               in += wrote;
               samples -= wrote;

               if (written == write_amount)
                  break;
               cond.wait();
            }
         }

         return samples;
      }

      ~AudioPort()
      {
         quit_thread = true;
         thread->join();
         delete thread;
         free(out_buffer);

         cellAudioPortStop(audio_port);
         cellAudioPortClose(audio_port);
      }

   private:
      Threads::Mutex lock;

      Threads::ThreadFifo<T> fifo;
      Threads::Cond cond;
      volatile bool quit_thread;
      Threads::Thread *thread;
      uint64_t input_rate;
      uint32_t audio_port;
      size_t fifo_size;
      static int ref_count;
      float *out_buffer;
      unsigned m_chan;
      
      void event_loop()
      {
         sys_event_queue_t id;
         sys_ipc_key_t key;
         sys_event_t event;

         cellAudioCreateNotifyEventQueue(&id, &key);
         cellAudioSetNotifyEventQueue(key);
         
         T buf[512];
         while (!quit_thread)
         {
            size_t has_read = fifo.read(buf, 512);
            memset(buf + has_read, 0, (512 - has_read) * sizeof(T));

            Internal::array_to_float(out_buffer, buf);
            sys_event_queue_receive(id, &event, SYS_NO_TIMEOUT);
            cellAudioAdd2chData(audio_port, out_buffer, CELL_AUDIO_BLOCK_SAMPLES, 1.0);
            cond.wake();
         }

         cellAudioRemoveNotifyEventQueue(key);
      }
};

}

#endif

