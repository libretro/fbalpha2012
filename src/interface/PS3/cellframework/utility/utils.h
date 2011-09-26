/*
 * utils.h
 *
 *  Created on: Nov 16, 2010
 *      Author: halsafar
 */

#ifndef UTILS_H_
#define UTILS_H_

// Platform dependant
#ifdef __CELLOS_LV2__
#include <sys/sys_time.h>
static uint64_t get_usec()
{
   return sys_time_get_system_time();
}
#else
#include <time.h>
static uint64_t get_usec()
{
   struct timespec tv;
   clock_gettime(CLOCK_MONOTONIC, &tv);
   return (uint64_t)tv.tv_sec * 1000000 + (uint64_t)tv.tv_nsec / 1000;
}
#endif

#endif /* UTILS_H_ */
