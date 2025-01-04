#include "FirFilter.h"

FirFilter::FirFilter()
	: firSize(1), cutoff(500.0f), sampleRate(44100), impulse(1, 1.0), filterBuffer(1, 0.0f) {
	updateImpulse();
}

FirFilter::FirFilter(int firSize, float cutoff, int sampleRate)
	: firSize(firSize), cutoff(cutoff), sampleRate(sampleRate), impulse(firSize, 0.0), filterBuffer(firSize, 0.0f) {
	if (firSize <= 0 || sampleRate <= 0) {
		throw std::invalid_argument("Invalid FIR size or sample rate");
	}
	updateImpulse();
}

void FirFilter::setCutoff(float cutoff) {
	this->cutoff = cutoff;
	updateImpulse();
}

void FirFilter::updateImpulse() {

	float twoPi = juce::MathConstants<float>::pi * 2.0f;
	float fourPi = juce::MathConstants<float>::pi * 4.0f;
	const float nyquist = sampleRate / 2;

	for (int i = 0; i < firSize; i++) {
		if (i == firSize / 2) {
			impulse[i] = 2.0 * (cutoff/nyquist);
		}
		else {
			double n = i - firSize / 2.0;
			impulse[i] = (sin(twoPi * (cutoff / nyquist) * n) / (juce::MathConstants<float>::pi * n)) *
				(0.54 - 0.46 * cos(twoPi * i / (firSize - 1)));
		}
	}
}

float FirFilter::applyFilter(float amplitude) {
	filterBuffer[bufferIndex] = amplitude;
	bufferIndex = (bufferIndex + 1) % firSize;

	float result = 0.0f;
	int index = bufferIndex;
	for (int i = 0; i < firSize; i++) {
		result += filterBuffer[index] * impulse[i];
		index = (index + 1) % firSize;
	}

	return result;
}