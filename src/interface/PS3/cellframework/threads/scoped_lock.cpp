#include "scoped_lock.hpp"

namespace Threads {

ScopedLock::ScopedLock(Mutex& lock) : m_lock(lock)
{
   m_lock.lock();
}

ScopedLock::~ScopedLock()
{
   m_lock.unlock();
}

}
