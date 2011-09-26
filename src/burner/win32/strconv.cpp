// win32 string conversion, added by regret
#include "burner.h"
#include "strconv.h"

static WCHAR* _wbuf = NULL;
static char* _buf = NULL;

void strConvClean()
{
	if (_wbuf) {
		delete [] _wbuf;
	}
	if (_buf) {
		delete [] _buf;
	}
}

// UCS2(W) - ANSI(A)
WCHAR* AtoW(const char* src, BOOL noStatic)
{
	WCHAR* wtmp = NULL;
	WCHAR*& wbuf = noStatic ? wtmp : _wbuf;

	if (wbuf) {
		delete [] wbuf;
		wbuf = NULL;
	}

	int len;
	if ((len = AtoW(src, NULL, 0)) > 0) {
		wbuf = new WCHAR [len];
		AtoW(src, wbuf, len);
	}
	return wbuf;
}

// UCS2(W) - UTF-8(U8) - ANSI(A)
WCHAR* U8toW(const char* src, BOOL noStatic)
{
	WCHAR* wtmp = NULL;
	WCHAR*& wbuf = noStatic ? wtmp : _wbuf;

	if (wbuf) {
		delete [] wbuf;
		wbuf = NULL;
	}

	int len;
	if ((len = U8toW(src, NULL, 0)) > 0) {
		wbuf = new WCHAR [len];
		U8toW(src, wbuf, len);
	}
	return wbuf;
}

char* WtoU8(const WCHAR* src, BOOL noStatic)
{
	char* tmp = NULL;
	char*& buf = noStatic ? tmp : _buf;

	if (buf) {
		delete [] buf;
		buf = NULL;
	}

	int len;
	if ((len = WtoU8(src, NULL, 0)) > 0) {
		buf = new char [len];
		WtoU8(src, buf, len);
	}
	return buf;
}

char* WtoA(const WCHAR* src, BOOL noStatic)
{
	char* tmp = NULL;
	char*& buf = noStatic ? tmp : _buf;

	if (buf) {
		delete [] buf;
		buf = NULL;
	}

	int	len;
	if ((len = WtoA(src, NULL, 0)) > 0) {
		buf = new char [len];
		WtoA(src, buf, len);
	}
	return buf;
}

char* AtoU8(const char* src, BOOL noStatic)
{
	char* tmp = NULL;
	char*& buf = noStatic ? tmp : _buf;

	if (buf) {
		delete [] buf;
		buf = NULL;
	}

	WCHAR* wsrc = AtoW(src, TRUE);
	if (wsrc) {
		buf = WtoU8(wsrc, TRUE);
	}
	delete [] wsrc;
	return buf;
}

char* U8toA(const char* src, BOOL noStatic)
{
	char* tmp = NULL;
	char*& buf = noStatic ? tmp : _buf;

	if (buf) {
		delete [] buf;
		buf = NULL;
	}

	WCHAR* wsrc = U8toW(src, TRUE);
	if (wsrc) {
		buf = WtoA(wsrc, TRUE);
	}
	delete [] wsrc;
	return buf;
}
