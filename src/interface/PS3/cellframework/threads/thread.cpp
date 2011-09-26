#include "thread.hpp"

namespace Threads {
   namespace Internal {

      void* _Entry(void *data)
      {
         Callable *call = reinterpret_cast<Internal::Callable*>(data);
         call->run();
         pthread_exit(NULL);
         return NULL;
      }

   }
}

