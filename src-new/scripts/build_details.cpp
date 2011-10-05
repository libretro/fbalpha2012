#include <stdio.h>

#define MAKE_STRING_2(s) #s
#define MAKE_STRING(s) MAKE_STRING_2(s)

#if defined _MSC_VER
 #if _M_IX86 == 500
  #define BUILD_CPU i586
 #elif _M_IX86 == 600
  #define BUILD_CPU i686
 #elif _M_IX86 == 700
  #define BUILD_CPU Pentium4 / Athlon
 #endif
#elif defined __GNUC__
 #ifdef __i586__
  #define BUILD_CPU i586
 #elif __i686__
  #define BUILD_CPU i686
 #elif __pentium4__
  #define BUILD_CPU Pentium4
 #elif __k6__
  #define BUILD_CPU K6
 #elif __athlon__
  #define BUILD_CPU Athlon
 #endif
#endif

#ifndef BUILD_CPU
 #define BUILD_CPU Unknown CPU
#endif

int main(int /*argc*/, char** /*argv*/)
{
	printf("#define BUILD_TIME %s\n", __TIME__);
	printf("#define BUILD_DATE %s\n", __DATE__);

#ifdef _UNICODE
	printf("#define BUILD_CHAR Unicode\n");
#else
	printf("#define BUILD_CHAR ANSI\n");
#endif

	printf("#define BUILD_CPU  " MAKE_STRING(BUILD_CPU) "\n");

#if defined __GNUC__
	printf("#define BUILD_COMP GCC %i.%i.%i\n", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
#elif defined _MSC_VER
 #if _MSC_VER >= 1300 && _MSC_VER < 1310
	printf("#define BUILD_COMP Visual C++ 2002\n");
 #elif _MSC_VER >= 1310 && _MSC_VER < 1320
	printf("#define BUILD_COMP Visual C++ 2003\n");
 #else
	printf("#define BUILD_COMP Visual C++ %i.%i\n", _MSC_VER / 100 - 6, _MSC_VER % 100 / 10);
 #endif
#else
	printf("#define BUILD_COMP Unknown compiler\n");
#endif

// Visual C's resource compiler doesn't define _MSC_VER, but we need it for VERSION resources
#ifdef _MSC_VER
	printf("#ifndef _MSC_VER\n");
	printf(" #define _MSC_VER  %i\n", _MSC_VER);
	printf("#endif\n");
#endif

	return 0;
}
