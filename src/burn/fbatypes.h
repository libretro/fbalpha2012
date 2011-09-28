#ifndef _FBATYPES_H_
#define _FBATYPES_H_

#if defined (SN_TARGET_PS3)

#ifndef FALSE
#define FALSE               0
#endif

#ifndef TRUE
#define TRUE                1
#endif

#ifdef SN_TARGET_PS3
#include <stdint.h>
typedef char  TCHAR;
typedef int32_t HWND;
typedef int32_t HFONT;
typedef int32_t HBITMAP;
typedef uint32_t BOOL;
typedef char WCHAR;
#else
typedef char				TCHAR;
typedef int					HWND;
typedef int					HFONT;
typedef int					HBITMAP;
typedef int					BOOL;
typedef char				WCHAR;
#endif
#define _T(x) x
#define __forceinline	__attribute((always_inline))
#define _stprintf sprintf
#define	_sntprintf	snprintf
#define _tcslen	strlen
#define _stricmp(x,y) strcasecmp(x,y) 
#define XMemSet(x,y,z) memset(x,y,z)
#define _tcsicmp(s1, s2) strcasecmp(s1, s2)
#define _tcslen     strlen
#define _tcscpy     strcpy
#define _tcscmp		strcmp
#define _tcscpy_s   strcpy_s
#define _tcsncpy    strncpy
#define _tcsncpy_s  strncpy_s
#define _tcscat     strcat
#define _tcscat_s   strcat_s
#define _tcsupr     strupr
#define _tcsupr_s   strupr_s
#define _tcslwr     strlwr
#define _tcslwr_s   strlwr_s
#define _fgetts fgets
#define _stprintf_s sprintf_s 
#define _tprintf    printf

#define _vstprintf_s    vsprintf_s
#define _vstprintf      vsprintf
#define _tcsrchr strrchr
#define _tfopen fopen
#define _tcsncmp strncmp
#define _istalpha(x) isalpha(x)
#define _istalnum(x) isalnum(x)
#define _totupper(x) toupper(x)
#define _totlower(x) tolower(x)
#define _istlower(x) islower(x)
#define _istupper(x) isupper(x)
#define _istdigit(x) isdigit(x)
#define _istspace(x) isspace(x)
#define _tscanf     scanf
#define _taccess _access
#define _tcstol     strtol
#define _tcstoul    strtoul
#define _tstof      atof
#define _tstoi      atoi
#define _tstoi64    _atoi64
#define _tstoi64    _atoi64
#define _ttoi       atoi
#define _ttoi64     _atoi64
#define _ttol       atol
#define _ftprintf   fprintf
#define _strnicmp(s1, s2, n) strncasecmp(s1, s2, (n))
#else
//#include "tchar.h"
#endif

#ifdef _MSC_VER
 typedef signed char int8_t;
 typedef signed short int16_t;
 typedef signed int int32_t;
 typedef signed long long int64_t;
 typedef int64_t intmax_t;
 #if defined(_WIN64)
 typedef int64_t intptr_t;
 #else
 typedef int32_t intptr_t;
 #endif

 typedef unsigned char uint8_t;
 typedef unsigned short uint16_t;
 typedef unsigned int uint32_t;
 typedef unsigned long long uint64_t;
 typedef uint64_t uintmax_t;
 #if defined(_WIN64)
 typedef uint64_t uintptr_t;
 #else
 typedef uint32_t uintptr_t;
 #endif
#else
 #include <stdint.h>
#endif

/* 8-bit values */
typedef uint8_t  UINT8;
typedef int8_t   INT8;

/* 16-bit values */
typedef uint16_t UINT16;
typedef int16_t  INT16;

/* 32-bit values */
typedef uint32_t UINT32;
typedef int32_t  INT32;

/* 64-bit values */
typedef uint64_t UINT64;
typedef int64_t  INT64;

#endif // _FBATYPES_H_
