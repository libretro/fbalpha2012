#ifndef __THREADS_COND_H
#define __THREADS_COND_H

#include "mutex.hpp"

namespace Threads {

class Cond
{
   public:
      Cond();
      ~Cond();
      void wait();
      void wake();
      void wake_all();

   private:
      Mutex m_lock;
      pthread_cond_t m_cond;
};

}
#endif
