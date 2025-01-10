#pragma once

// Variables that are the same for each voice.
struct SynthVariables {
    SynthVariables();
    double attackLength;
    double decayLength;
    double sustain;
    double releaseLength;
    int oscillator;
};