#ifndef _STRCONV_H_
#define _STRCONV_H_

// Used to convert strings when possibly needed
#define MAX_STR_LEN (MAX_PATH * 3)

inline int AtoW(const char* src, WCHAR* dst, int bufsize, int max_len = -1)
{
	return strlen(src);
}

inline int WtoA(const WCHAR* src, char* dst, int bufsize, int max_len = -1)
{
	return strlen(src);
}

inline int WtoU8(const WCHAR* src, char* dst, int bufsize, int max_len = -1)
{
	return strlen(src);
}

inline int U8toW(const char* src, WCHAR* dst, int bufsize, int max_len = -1)
{
	return strlen(src);
}

WCHAR* AtoW(const char* src, int noStatic = 0);
char* WtoA(const WCHAR* src, int noStatic = 0);
WCHAR* U8toW(const char* src, int noStatic = 0);
char* WtoU8(const WCHAR* src, int noStatic = 0);
char* AtoU8(const char* src, int noStatic = 0);
char* U8toA(const char* src, int noStatic = 0);

void strConvClean();

#endif
