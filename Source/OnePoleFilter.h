#pragma once

class OnePoleFilter {
public:
	OnePoleFilter();
	void setFrequency(float freq);
	float applyFilter(float amplitude);
	void setSampleRate(int newSampleRate);

private:
	double a0;
	double b1;
	double z1;
	int sampleRate;
};