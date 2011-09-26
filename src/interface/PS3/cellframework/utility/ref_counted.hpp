#ifndef __UTILS_REF_COUNTED_H
#define __UTILS_REF_COUNTED_H

template<class T>
class ref_counted
{
   public:
      unsigned& ref()
      {
         return cnt;
      }

   private:
      static unsigned cnt;
};

template<class T>
unsigned ref_counted<T>::cnt = 0;

#endif
