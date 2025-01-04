#include "OnePoleFilter.h"
#include <JuceHeader.h>

OnePoleFilter::OnePoleFilter() {
	a0 = 1.0;
	b1 = 0.0;
	z1 = 0.0;
	sampleRate = 44100;
}

void OnePoleFilter::setFrequency(float freq) {
	auto theta = juce::MathConstants<float>::pi * 2.0f * freq / sampleRate;
	b1 = exp(-theta);
	a0 = 1.0 - b1;
}

float OnePoleFilter::applyFilter(float amplitude) {
	z1 = amplitude * a0 + z1 * b1;
	return z1;
}

void OnePoleFilter::setSampleRate(int newSampleRate) {
	sampleRate = newSampleRate;
}