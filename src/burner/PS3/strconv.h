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

WCHAR* AtoW(const char* src, BOOL noStatic = FALSE);
char* WtoA(const WCHAR* src, BOOL noStatic = FALSE);
WCHAR* U8toW(const char* src, BOOL noStatic = FALSE);
char* WtoU8(const WCHAR* src, BOOL noStatic = FALSE);
char* AtoU8(const char* src, BOOL noStatic = FALSE);
char* U8toA(const char* src, BOOL noStatic = FALSE);

void strConvClean();

#endif
