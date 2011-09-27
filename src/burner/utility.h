// utility, added by regret

/* changelog:
 update 3: add file state check functions
 update 2: add some file related functions
 update 1: create
*/

#ifndef _UTILITY_H_
#define _UTILITY_H_

#if defined (_XBOX)
#include <xtl.h>
#elif defined (SN_TARGET_PS3)
#include <cell/cell_fs.h>
#else
#include <unistd.h>
#endif
#include "../burn/fbatypes.h"

// macro for getting array size
#define sizearray(a) (sizeof(a) / sizeof((a)[0]))

// functions
inline char strlower(char c)
{
	return (c >= 'A' && c <= 'Z') ? c + ('a' - 'A') : c;
}
inline char strupper(char c)
{
	return (c >= 'a' && c <= 'z') ? c - ('a' - 'A') : c;
}

template<typename T>
inline void swap(T& x, T& y)
{
	T temp(x);
	x = y;
	y = temp;
}

template<typename T>
inline T* allocate(size_t size, const T& value)
{
	T* array = new T[size];
	for (size_t i = 0; i < size; i++) {
		array[i] = value;
	}
	return array;
}

// get string until "\r\n"
inline void getLine(char* p)
{
	if (!p) {
		return;
	}

	for (size_t i = 0; i < strlen(p); i++) {
		if (p[i] == '\r' || p[i] == '\n') {
			p[i] = '\0';
			break;
		}
	}
}

// path check (add trailing backslash)
inline void pathCheck(char * path)
{
	if (!path)
		return;

	size_t len = strlen(path);
	if (len == 0)
		return;

	if (path[len - 1] != '\\')
	{
		path[len] = '\\';
		path[len + 1] = '\0';
	}
}

// "foo/bar.c" -> "bar"
inline char * getBaseName(const char * name)
{
	static char filename[256];

#if defined (_XBOX)
	const char * p = strrchr(name, '\\');
#else
	const char * p = strrchr(name, '/');
#endif
	if(p)
		strcpy(filename, p + 1);
	else
		strcpy(filename, name);

	for (size_t i = strlen(filename); i > 0; i--) {
		if (filename[i] == '.') {
			filename[i] = '\0';
		}
	}

	return filename;
}

inline char* getBaseNameA(const char* name)
{
	static char filename[256];

#if defined (_XBOX)
	const char * p = strrchr(name, '\\');
#else
	const char * p = strrchr(name, '/');
#endif
	if (p) {
		strcpy(filename, p + 1);
	} else {
		strcpy(filename, name);
	}

	for (size_t i = strlen(filename); i > 0; i--) {
		if (filename[i] == '.') {
			filename[i] = '\0';
		}
	}

	return filename;
}

inline char* getFilenameA(char* fullname)
{
	size_t len = strlen(fullname);
	if (len == 0) {
		return fullname;
	}

	for (size_t i = len - 1; i >= 1; i--) {
		if (fullname[i] == '\\' || fullname[i] == '/') {
			return fullname + i + 1;
		}
	}

	return fullname;
}

inline char * getFilenameW(char * fullname)
{
	size_t len = strlen(fullname);
	if (len == 0) {
		return fullname;
	}

	for (size_t i = len - 1; i >= 1; i--) {
		if (fullname[i] == '\\' || fullname[i] == '/') {
			return fullname + i + 1;
		}
	}

	return fullname;
}

inline char * getFileExt(const char * filename)
{
	static char ext[32] = "";

	const char * p = strrchr(filename, '.');
	if (p) {
		strcpy(ext, p + 1);
		return ext;
	}
	return NULL;
}

inline long getFileSize(FILE* file)
{
	if (!file)
		return 0;

	fseek(file, 0, SEEK_END);
	long size = ftell(file);
	fseek(file, 0, SEEK_SET);
	return size;
}

// read file content to buffer, return file size
inline size_t getFileBuffer(const char * filename, void** buffer)
{
	assert(!(*buffer));

	FILE* file = fopen(filename, "rt");
	if (!file) {
		return 0;
	}

	long size = getFileSize(file);
	size_t read_size = 0;
	void* temp = NULL;

	if (size > 0) {
		temp = (void*)malloc(size);
		read_size = fread(temp, 1, size, file);
	}
	fclose(file);

	if (read_size > 0 && temp) {
		*buffer = (void*)malloc(read_size);
		memcpy(*buffer, temp, read_size);
	}
	free(temp);

	return read_size;
}

// check file state
inline bool fileExists(const char * filename)
{

#if defined (_XBOX)
// changed to allow running from usb drives
	if (GetFileAttributes(filename)!=-1)
	{
		return true;
	}
	else
	{
		return false;
	}
#elif defined (SN_TARGET_PS3)
	CellFsStat sb;
	if (cellFsStat(filename,&sb) == CELL_FS_SUCCEEDED)	 
		return true;
	else
		return false;
#else
	return true; //
#endif

}

inline bool fileReadable(const char* filename)
{
#if defined (_XBOX)
	return true;
#else
	
 
	return true;
#endif
}

// Macros for parsing text
#define SKIP_WS(s) while (isspace(*s)) { s++; }			// Skip whitespace
#define FIND_WS(s) while (*s && !isspace(*s)) { s++; }	// Find whitespace
#define FIND_QT(s) while (*s && *s != '\"') { s++; }	// Find quote

// config file parsing
#define QUOTE_MAX (128)										// Maximum length of "quoted strings"

inline char* labelCheck(char* s, char* label)
{
	if (!s || !label) {
		return NULL;
	}
	size_t len = strlen(label);

	SKIP_WS(s);							// Skip whitespace

	if (strncmp(s, label, len)) {		// Doesn't match
		return NULL;
	}
	return s + len;
}

// Read a (quoted) string from szSrc and point to the end
inline int quoteRead(char** ppQuote, char** ppEnd, char* src)
{
	static char quote[QUOTE_MAX];
	char* s = src;
	char* e;

	// Skip whitespace
	SKIP_WS(s);

	e = s;

	if (*s == '\"') {					// Quoted string
		s++;
		e++;
		// Find end quote
		FIND_QT(e);
		strncpy(quote, s, e - s);
		// Zero-terminate
		quote[e - s] = '\0';
		e++;
	} else {								// Non-quoted string
		// Find whitespace
		FIND_WS(e);
		strncpy(quote, s, e - s);
		// Zero-terminate
		quote[e - s] = '\0';
	}

	if (ppQuote) {
		*ppQuote = quote;
	}
	if (ppEnd) {
		*ppEnd = e;
	}

	return 0;
}

inline bool skipComma(char** s)
{
	while (**s && **s != ',') {
		(*s)++;
	}

	if (**s == ',') {
		(*s)++;
	}

	if (**s) {
		return true;
	}

	return false;
}

#endif
