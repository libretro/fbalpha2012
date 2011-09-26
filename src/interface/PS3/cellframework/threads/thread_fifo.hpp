#ifndef __THREAD_FIFO_H
#define __THREAD_FIFO_H

#include <deque>
#include <stddef.h>
#include "fifo.hpp"
#include "scoped_lock.hpp"
#include "mutex.hpp"

namespace Threads {

template<class T, class Container = std::deque<T> >
class ThreadFifo : public FifoBuffer<T, Container>
{
   public:
      ThreadFifo(size_t num_elems);
      size_t read_avail();
      size_t write_avail();
      size_t write(const T* in, size_t num_elems);
      size_t read(T* in, size_t num_elems);
      void clear();
      void resize(size_t num_elems);
   private:
      Mutex m_lock;
};

template<class T, class Container>
ThreadFifo<T, Container>::ThreadFifo(size_t num_elems) : FifoBuffer<T, Container>(num_elems)
{}

template<class T, class Container>
size_t ThreadFifo<T, Container>::read_avail()
{
   ScopedLock lock(m_lock);
   return FifoBuffer<T, Container>::read_avail();
}

template<class T, class Container>
size_t ThreadFifo<T, Container>::write_avail()
{
   ScopedLock lock(m_lock);
   return FifoBuffer<T, Container>::write_avail();
}

template<class T, class Container>
size_t ThreadFifo<T, Container>::write(const T* in, size_t num_elems)
{
   ScopedLock lock(m_lock);
   return FifoBuffer<T, Container>::write(in, num_elems);
}

template<class T, class Container>
size_t ThreadFifo<T, Container>::read(T* in, size_t num_elems)
{
   ScopedLock lock(m_lock);
   return FifoBuffer<T, Container>::read(in, num_elems);
}

template<class T, class Container>
void ThreadFifo<T, Container>::clear()
{
   ScopedLock lock(m_lock);
   FifoBuffer<T, Container>::clear();
}

template<class T, class Container>
void ThreadFifo<T, Container>::resize(size_t num_elems)
{
   ScopedLock lock(m_lock);
   FifoBuffer<T, Container>::resize(num_elems);
}
}

#endif
