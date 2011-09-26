#ifndef _STRCONV_H_
#define _STRCONV_H_

// Used to convert strings when possibly needed
#define MAX_STR_LEN (MAX_PATH * 3)

#if defined (_XBOX)
inline int AtoW(const char* src, WCHAR* dst, int bufsize, int max_len = -1) {
	return ::MultiByteToWideChar(CP_ACP, 0, src, max_len, dst, bufsize);
}
inline int WtoA(const WCHAR* src, char* dst, int bufsize, int max_len = -1) {
	return ::WideCharToMultiByte(CP_ACP, 0, src, max_len, dst, bufsize, 0, 0);
}
inline int WtoU8(const WCHAR* src, char* dst, int bufsize, int max_len = -1) {
	return ::WideCharToMultiByte(CP_UTF8, 0, src, max_len, dst, bufsize, 0, 0);
}
inline int U8toW(const char* src, WCHAR* dst, int bufsize, int max_len = -1) {
	return ::MultiByteToWideChar(CP_UTF8, 0, src, max_len, dst, bufsize);
}

WCHAR* AtoW(const char* src, BOOL noStatic = FALSE);
char* WtoA(const WCHAR* src, BOOL noStatic = FALSE);
WCHAR* U8toW(const char* src, BOOL noStatic = FALSE);
char* WtoU8(const WCHAR* src, BOOL noStatic = FALSE);
char* AtoU8(const char* src, BOOL noStatic = FALSE);
char* U8toA(const char* src, BOOL noStatic = FALSE);
#else

inline int AtoW(const char* src, WCHAR* dst, int bufsize, int max_len = -1) {
	return strlen(src);
}
inline int WtoA(const WCHAR* src, char* dst, int bufsize, int max_len = -1) {
	return strlen(src);
}
inline int WtoU8(const WCHAR* src, char* dst, int bufsize, int max_len = -1) {
	return strlen(src);
}
inline int U8toW(const char* src, WCHAR* dst, int bufsize, int max_len = -1) {
	return strlen(src);
}

WCHAR* AtoW(const char* src, BOOL noStatic = FALSE);
char* WtoA(const WCHAR* src, BOOL noStatic = FALSE);
WCHAR* U8toW(const char* src, BOOL noStatic = FALSE);
char* WtoU8(const WCHAR* src, BOOL noStatic = FALSE);
char* AtoU8(const char* src, BOOL noStatic = FALSE);
char* U8toA(const char* src, BOOL noStatic = FALSE);
#endif
void strConvClean();

#endif
