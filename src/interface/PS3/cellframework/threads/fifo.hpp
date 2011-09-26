#ifndef __FIFO_BUFFER_H
#define __FIFO_BUFFER_H
#include <deque>
#include <stddef.h>

template<class T, class Container = std::deque<T> >
class FifoBuffer
{
   public:
      explicit FifoBuffer(size_t num_elems);

      size_t read_avail() const;
      size_t write_avail() const;
      void resize(size_t num_elems);

      size_t write(const T* in, size_t num_elems);
      size_t read(T* out, size_t num_elems);
      void clear();

   private:
      Container m_container;
      size_t m_max;
};

template<class T, class Container>
FifoBuffer<T, Container>::FifoBuffer(size_t num_elems) : m_max(num_elems)
{}

template<class T, class Container>
size_t FifoBuffer<T, Container>::read_avail() const
{
   return m_container.size();
}

template<class T, class Container>
size_t FifoBuffer<T, Container>::write_avail() const
{
   return m_max - m_container.size();
}

template<class T, class Container>
size_t FifoBuffer<T, Container>::write(const T* in, size_t num_elems)
{
   size_t write_amount = num_elems > write_avail() ? write_avail() : num_elems;
   m_container.insert(m_container.end(), in, in + write_amount);
   return write_amount;
}

template<class T, class Container>
size_t FifoBuffer<T, Container>::read(T* out, size_t num_elems)
{
   size_t read_amount = num_elems > read_avail() ? read_avail() : num_elems;
   std::copy(m_container.begin(), m_container.begin() + read_amount, out);
   m_container.erase(m_container.begin(), m_container.begin() + read_amount);
   return read_amount;
}

template<class T, class Container>
void FifoBuffer<T, Container>::clear()
{
   m_container.clear();
}

template<class T, class Container>
void FifoBuffer<T, Container>::resize(size_t num_elems)
{
   size_t size = m_container.size();
   m_max = num_elems;
   if (size > m_max)
   {
      m_container.erase(m_container.begin() + m_max, m_container.end());
   }
}
#endif


