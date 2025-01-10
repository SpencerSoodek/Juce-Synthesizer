#pragma once
#include <JuceHeader.h>
#include "SynthVariables.h"

class Voice {
public:
	Voice();
    Voice(SynthVariables* sv, float sampleRate);
    SynthVariables* synthVariables;
	float sampleRate;
	double currentAngle = 0.0;
	double angleDelta = 0.0;
	double currentFrequency = 500.0;

    int midiNoteNumber = 0;

	float sinOscillatorAmplitude();
	float sawOscillatorAmplitude();
	float squareOscillatorAmplitude();
	float whiteNoise();
    float sawtoothBandLimited();
    float squareBandLimited();
    float currentSample();

    void noteOn(int midiNoteNumber, float velocity);
    void noteOff();

    void updateAngleDelta();

	bool playing = false;
	bool released = false;
	int samplesSinceStart = 0;
	int samplesSinceReleased = 0;


private:
	double midiToFreq(int midiNoteNumber);
    juce::Random random;
};