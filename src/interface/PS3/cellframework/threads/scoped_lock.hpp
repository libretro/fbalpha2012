#ifndef __THREADS_SCOPED_LOCK
#define __THREADS_SCOPED_LOCK

#include "mutex.hpp"

namespace Threads {

class ScopedLock
{
   public:
      explicit ScopedLock(Mutex& lock);
      ~ScopedLock();
   private:
      Mutex& m_lock;
};

}

#endif
