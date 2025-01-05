#include "BiquadFilter.h"

BiquadFilter::BiquadFilter() {
	this->cutoffFrequency = 20000;
	this->q = .71f;
	this->sampleRate = 44100;
	this->filterType = 1;
	this->z1 = 0.0f;
	this->z2 = 0.0f;
	calculateCoefficients();
}

BiquadFilter::BiquadFilter(float cutoff, float q, float sampleRate, int filterType)
{
	this->cutoffFrequency = cutoff;
	this->q = q;
	this->sampleRate = sampleRate;
	this->filterType = filterType;
	this->z1 = 0.0f;
	this->z2 = 0.0f;
	calculateCoefficients();
}

void BiquadFilter::setCutoff(float cutoff) {
	this->cutoffFrequency = cutoff;
	calculateCoefficients();
}

void BiquadFilter::setResonance(float q) {
	this->q = q;
	calculateCoefficients();
}

void BiquadFilter::setFilterType(int filterType) {
	this->filterType = filterType;
	calculateCoefficients();
}

void BiquadFilter::calculateCoefficients() {
	switch (filterType) {
	case 1:
		calculateLPFCoefficients();
		break;
	case 2:
		calculateHPFCoefficients();
		break;
	default:
		break;
	}
}

void BiquadFilter::calculateLPFCoefficients() {
	w0 = 2.0f * juce::MathConstants<float>::pi * (cutoffFrequency / sampleRate);
	const float cosW0 = cos(w0);
	const float sinW0 = sin(w0);
	alpha = sinW0 / (2.0f * q);
	b0 = (1.0f - cosW0) / 2.0f;
	b1 = (1.0f - cosW0);
	b2 = (1.0f - cosW0) / 2.0f;
	a0 = 1.0f + alpha;
	a1 = -2.0f * cosW0;
	a2 = 1.0f - alpha;
}

void BiquadFilter::calculateHPFCoefficients() {
	w0 = 2 * juce::MathConstants<float>::pi * (cutoffFrequency / sampleRate);
	const float cosW0 = cos(w0);
	const float sinW0 = sin(w0);
	alpha = sinW0 / (2.0f * q);
	b0 = (1.0f + cosW0) / 2.0f;
	b1 = 0.0f - (1.0f + cosW0);
	b2 = (1.0f + cosW0) / 2;
	a0 = 1.0f + alpha;
	a1 = -2.0f * cosW0;
	a2 = 1.0f - alpha;
}

float BiquadFilter::applyFilter(float z) {
	float result = (b0 * z + b1 * z1 + b2 * z2) / a0;
	z2 = z1;
	z1 = z;
	return result;
}