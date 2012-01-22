#ifndef _PS3OPTS_H_
#define _PS3OPTS_H_

#ifdef __cplusplus
extern "C" void ps3memcpy(void *dst, const void * src, size_t n);
#endif

#define memcpy ps3memcpy

#endif
