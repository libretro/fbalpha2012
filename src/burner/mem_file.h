// memory file class, by regret

/* changelog:
 update 1: create (ref: fex, mame)
*/

#ifndef _MEM_FILE_H_
#define _MEM_FILE_H_

class Mem_File {
public:
	Mem_File(char* p, int s)
		: begin_(p)
	{
		set_size(s);
		utf8_ = false;
	}

	virtual ~Mem_File() {
		delete begin_; // notice: delete buffer
	}

	// size of file
	int size() const { return size_; }

	// number of bytes remaining until end of file
	int remain() const { return remain_; }

	// current position in file
	int tell() const { return size_ - remain(); }

	// if is utf8 file
	void set_utf8(bool u) { utf8_ = u; }

	// goes to new position
	int seek(int n) {
		assert(n >= 0);

		if (n < 0 || n > size())
			return 1;

		if (n == tell())
			return 0;

		set_tell(n);
		return 0;
	}

	// reads min(n, remain()) bytes and sets *s to this number
	int read(char* p, int n, int* s) {
		assert( n >= 0 );

		n = min(n, remain());
		if (s) *s = 0;
		if (n <= 0)
			return 1;

		memcpy(p, begin_ + tell(), n);
		remain_ -= n;
		if (s) *s = n;
		return 0;
	}

	// get a character
	char getc() {
		char c = *(begin_ + tell());
		if (seek(tell() + 1))
			return EOF;
		return c;
	}

	// fixme: get utf8 chars
	int gets(char* s, int n) {
		assert( n >= 0 );

		n = min(n, remain());
		if (n <= 0)
			return 1;

		char* cur = s;
		bool eof = false;

		while (n > 0) {
			char c = getc();
			if (c == EOF) {
				eof = true;
				break;
			}

			// if there's a CR, look for an LF afterwards
			if (c == 0x0d) {
				char c2 = getc();
				if (c2 != 0x0a) {
					seek(tell() - 1);
				}
				*cur++ = 0x0d;
				n--;
				break;
			}
			// if there's an LF, reinterp as a CR for consistency
			else if (c == 0x0a) {
				*cur++ = 0x0d;
				n--;
				break;
			}

			// otherwise, pop the character in and continue
			*cur++ = c;
			n--;
		}

		// if we put nothing in
		if (cur == s)
			return 1;

		// otherwise, terminate
		if (n > 0)
			*cur++ = 0;

		return eof ? 1 : 0;
	}

protected:
	// sets remain
	void set_remain(int n) { assert(n >= 0); remain_ = n; }
	// sets reported position
	void set_tell(int i) { assert(0 <= i && i <= size_); set_remain(size_ - i); }
	// sets size and resets position
	void set_size(int n) { size_ = n; set_remain(n); }

private:
	char* begin_;
	int remain_;
	int size_;
	bool utf8_;
};

#endif
