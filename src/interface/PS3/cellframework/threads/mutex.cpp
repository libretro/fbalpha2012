#include "mutex.hpp"

namespace Threads {

Mutex::Mutex()
{
   pthread_mutex_init(&m_lock, NULL);
}

Mutex::~Mutex()
{
   pthread_mutex_destroy(&m_lock);
}

void Mutex::lock()
{
   pthread_mutex_lock(&m_lock);
}

void Mutex::unlock()
{
   pthread_mutex_unlock(&m_lock);
}

}
