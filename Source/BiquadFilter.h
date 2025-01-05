#pragma once
#include "JuceHeader.h"

class BiquadFilter {
public:
	BiquadFilter();
	BiquadFilter(float cutoff, float q, float sampleRate, int filterType);
	void setCutoff(float cutoff);
	void setResonance(float q);
	/*Types of filter
	* 1: LPF
	* 2: HPF
	*/
	void setFilterType(int type);
	float applyFilter(float z);
private:
	int filterType;
	int sampleRate;
	float cutoffFrequency;
	float q;
	float bandwith;
	float w0, alpha, a0, a1, a2, b0, b1, b2;
	float z1, z2;

	void calculateCoefficients();
	void calculateLPFCoefficients();
	void calculateHPFCoefficients();
};