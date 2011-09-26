#ifndef HIPASS_INCLUDED
#define HIPASS_INCLUDED

class LowPass2
{
public:
	LowPass2(void);
	LowPass2(double Freq, double SampleRate, double Q, double Gain,
		 double Freq2, double Q2, double Gain2);

	void Filter(short int *Buff, int Tam);
	void SetParam(double Freq, double SampleRate, double Q, double Gain,
		      double Freq2, double Q2, double Gain2);

private:
	int a0, a1, a2, b0, b1, b2;
	int i0, i1, i2, o0, o1, o2;

	int a0b, a1b, a2b, b0b, b1b, b2b;
	int o0b, o1b, o2b;

	// Yes, I know some of these are not used
};

#endif
