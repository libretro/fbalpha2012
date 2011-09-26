#ifndef __MUTEX_H
#define __MUTEX_H
#include <pthread.h>

namespace Threads {

class Cond;
class Mutex
{
   friend class Cond;
   public:
      Mutex();
      ~Mutex();
      void lock();
      void unlock();

   private:
      pthread_mutex_t m_lock;
};

}

#endif
