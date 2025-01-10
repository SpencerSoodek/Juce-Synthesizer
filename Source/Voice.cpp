
#include "Voice.h";

Voice::Voice() {
    this->sampleRate = 44100;
    this->synthVariables = nullptr;
}

Voice::Voice(SynthVariables* sv, float sampleRate) {
    this->synthVariables = sv;
    this->sampleRate = sampleRate;
    int osc = synthVariables->oscillator;
    std::cout << osc << std::endl;
}

float Voice::sinOscillatorAmplitude() {
    return (float)std::sin(currentAngle);
}

float Voice::sawOscillatorAmplitude() {
    return (-1.0 + 2 * (currentAngle / (2 * (juce::MathConstants<float>::pi))));
}

float Voice::sawtoothBandLimited() {
    int numPartials = static_cast<int>((sampleRate / 2.0f) / currentFrequency);
    float twoPiAngle = 2.0f * juce::MathConstants<float>::pi * (currentAngle / (2.0f * juce::MathConstants<float>::pi));

    float amplitude = 0.0f;
    for (int n = 1; n <= numPartials; n++) {
        amplitude += (1.0f / n) * std::sin(n * twoPiAngle);
    }
    return 2.0f * amplitude / juce::MathConstants<float>::pi;
}


float Voice::squareOscillatorAmplitude() {
    if (currentAngle <= juce::MathConstants<float>::pi) {
        return 1;
    }
    else {
        return 0;
    }
}

float Voice::squareBandLimited() {
    int numPartials = static_cast<int>((sampleRate / 2.0f) / currentFrequency);
    float twoPiAngle = 2.0f * juce::MathConstants<float>::pi * (currentAngle / (2.0f * juce::MathConstants<float>::pi));

    float amplitude = 0.0f;
    for (int n = 1; n <= numPartials; n += 2) {
        amplitude += (1.0f / n) * std::sin(n * twoPiAngle);
    }
    return 2.0f * amplitude / juce::MathConstants<float>::pi;
}

float Voice::whiteNoise() {
    return random.nextFloat() * 2.0f - 1.0f;
}

void Voice::noteOn(int midiNoteNumber, float velocity) {
    this->midiNoteNumber = midiNoteNumber;
    double newFreq = midiToFreq(midiNoteNumber);
    currentFrequency = newFreq;
    released = false;
    this->playing = true;
    samplesSinceStart = 0;
    updateAngleDelta();
    int al = synthVariables->attackLength;
    std::cout << al << std::endl;
}
void Voice::noteOff() {
    midiNoteNumber = 0;
    released = true;
    samplesSinceReleased = 0;
}

void Voice::updateAngleDelta() {
    auto cyclesPerSample = currentFrequency / sampleRate;
    angleDelta = cyclesPerSample * 2.0 * juce::MathConstants<double>::pi;
}

double Voice::midiToFreq(int midiNoteNumber) {
    return 440.0 * std::pow(2.0, ((midiNoteNumber - 69) / 12.0));
}

float Voice::currentSample() {
    float currSample = 0.0f;
    int osc = synthVariables->oscillator;
    switch (osc) {
    case 1:
        currSample = sinOscillatorAmplitude();
        break;
    case 2:
        currSample = sawtoothBandLimited();
        break;
    case 3:
        currSample = squareBandLimited();
        break;
    case 4:
        currSample = whiteNoise();
        break;
    }
    currentAngle += angleDelta;
    if (currentAngle >= 2.0f * juce::MathConstants<float>::pi) {
        currentAngle -= 2.0f * juce::MathConstants<float>::pi;
    }
    float msSinceStart = samplesSinceStart / sampleRate * 1000.0;
    samplesSinceStart++;
    // Attack amplitude
    if (msSinceStart < synthVariables->attackLength && !released) {
        currSample *= (msSinceStart / synthVariables->attackLength);
    }
    //Decay amplitude; 
    if (msSinceStart >= synthVariables->attackLength && msSinceStart < synthVariables->attackLength + synthVariables->decayLength && !released) {
        float msSinceAttackEnd = msSinceStart - synthVariables->attackLength;
        float decayAmplitude = 1.0f - (msSinceAttackEnd / synthVariables->decayLength) * (1.0f - synthVariables->sustain);
        currSample *= decayAmplitude;
    }
    // Sustain amplitude
    if (msSinceStart >= synthVariables->attackLength + synthVariables->decayLength && !released) {
        currSample *= synthVariables->sustain;
    }
    // Release amplitude
    if (released) {
        float msSinceRelease = samplesSinceReleased / sampleRate * 1000.0;
        samplesSinceReleased++;
        if (msSinceRelease >= synthVariables->releaseLength) {
            playing = false;
            samplesSinceReleased = 0;
        }
        float releaseAmplitude = synthVariables->sustain - ((msSinceRelease / synthVariables->releaseLength) * synthVariables->sustain);
        releaseAmplitude = juce::jlimit(0.0f, 1.0f, releaseAmplitude);
        currSample *= releaseAmplitude;
    }
    return currSample;
}
