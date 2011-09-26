// win32 string conversion, added by regret
#include "burner.h"
#include "strconv.h"

static WCHAR* _wbuf = NULL;
static char* _buf = NULL;

void strConvClean()
{
	if (_wbuf)
		delete [] _wbuf;

	if (_buf)
		delete [] _buf;
}

// UCS2(W) - ANSI(A)
WCHAR* AtoW(const char* src, BOOL noStatic)
{
	return (WCHAR*)src;
}

// UCS2(W) - UTF-8(U8) - ANSI(A)
WCHAR* U8toW(const char* src, BOOL noStatic)
{
	return (WCHAR*)src;
}

char* WtoU8(const WCHAR* src, BOOL noStatic)
{
	return (WCHAR*)src;
}

char* WtoA(const WCHAR* src, BOOL noStatic)
{
	return (WCHAR*)src;
}

char* AtoU8(const char* src, BOOL noStatic)
{
	return (WCHAR*)src;
}

char* U8toA(const char* src, BOOL noStatic)
{
	return (WCHAR*)src;
}
