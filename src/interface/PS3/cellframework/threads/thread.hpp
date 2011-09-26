#include <pthread.h>

namespace Threads {

   namespace Internal {

      class Callable
      {
         public:
            virtual void run() = 0;
            virtual ~Callable() {}
      };

      extern "C"
      void* _Entry(void *data);
   }

class Thread
{
   public:

      template<class C, class P>
      Thread(void (C::*memberfn)(P), C* obj, P arg) : joinable(true)
      {
         callptr = new member_arg<C, P>(memberfn, obj, arg);
         start();
      }

      template<class C>
      Thread(void (C::*memberfn)(), C* obj) : joinable(true)
      {
         callptr = new member<C>(memberfn, obj);
         start();
      }

      template<class P>
      Thread(void (*fn)(P), P data) : joinable(true)
      {
         callptr = new func_arg<P>(fn, data);
         start();
      }

      Thread(void (*fn)()) : joinable(true)
      {
         callptr = new func(fn);
         start();
      }

      void join()
      {
         if (joinable)
            pthread_join(m_id, NULL);
         joinable = false;
      }

      ~Thread()
      {
         join();
         if (callptr != NULL)
            delete callptr;
      }

   private:

      // Cannot copy a thread. 
      void operator=(const Thread&);
      Thread(const Thread&);

      pthread_t m_id;
      bool joinable;

      void start()
      {
         pthread_create(&m_id, NULL, Internal::_Entry, reinterpret_cast<void*>(callptr));
      }

      Internal::Callable *callptr;

      template<class P>
      class func_arg : public Internal::Callable
      {
         public:
         func_arg(void (*in_fn)(P), P in) : fn(in_fn), arg(in) {}

         void run()
         {
            fn(arg);
         }

         void (*fn)(P);
         P arg;
      };

      class func : public Internal::Callable
      {
         public:
         func(void (*in_fn)()) : fn(in_fn) {}

         void run()
         {
            fn();
         }

         void (*fn)();
      };

      template<class C>
      class member : public Internal::Callable
      {
         public:
         member(void (C::*memberfn)(), C* in_obj) : obj(in_obj), membfn(memberfn) {}
         void run()
         {
            (obj->*membfn)();
         }

         C* obj;
         void (C::*membfn)();
      };

      template<class C, class P>
      class member_arg : public Internal::Callable
      {
         public:
         member_arg(void (C::*memberfn)(P), C* in_obj, P in_arg) : obj(in_obj), membfn(memberfn), arg(in_arg) {}

         void run()
         {
            (obj->*membfn)(arg);
         }

         C* obj;
         void (C::*membfn)(P);
         P arg;
      };
};

}


