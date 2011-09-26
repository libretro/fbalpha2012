#ifndef SN_TARGET_PS3
class Audio {
public:
	virtual int init() { return 0; }
	virtual int exit() { return 0; }
	virtual int play() { return 1; }
	virtual int stop() { return 0; }
	virtual int blank() { return 0; }
	virtual int check() { return 0; }
	virtual int setvolume(int vol) { return 0; }
	virtual int setfps() { return 0; }
	virtual int set(int (*callback)(int)) { return 0; }
	virtual int get(void* info) { return 0; }

	Audio() {
		loopLen = 0;
		fps = 0;
		volume = 1.0f;
	}
	virtual ~Audio() {}

protected:
	float volume;
	int loopLen;	// Loop length (in bytes) calculated
	int fps;		// Application fps * 100
};
#endif
