#pragma once
#include <JuceHeader.h>

class FirFilter {
public:
	FirFilter();
	FirFilter(int firSize, float cutoff, int sampleRate);
	~FirFilter() = default;
	void setCutoff(float cutoff);
	float applyFilter(float amplitude);

private:
	void updateImpulse();

	int firSize;
	float cutoff;
	int sampleRate;
	std::vector<double> impulse;
	std::vector<float> filterBuffer;

	int bufferIndex = 0;
};