#pragma once

#include <JuceHeader.h>
#include "IncomingMessageCallback.h"
#include "OnePoleFilter.h"
#include "FirFilter.h"

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent :
    public juce::AudioAppComponent,
    private juce::MidiInputCallback,
    private juce::MidiKeyboardStateListener
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    void handleNoteOn(juce::MidiKeyboardState* source, int midiChannel, int midiNoteNumber, float velocity) override;
    void handleNoteOff(juce::MidiKeyboardState* source, int midiChannel, int midiNoteNumber, float velocity) override;
    void postMessageToList(const juce::MidiMessage& message, const juce::String& source);
    void addMessageToList(const juce::MidiMessage& message, const juce::String& source);
    void handleIncomingMidiMessage(juce::MidiInput* source, const juce::MidiMessage& message);
    void oscSelectorMenuChanged();

    float sinOscillatorAmplitude();
    float sawOscillatorAmplitude();
    float squareOscillatorAmplitude();

    float sawtoothBandLimited();
    float squareBandLimited();

    

    void logMessage(const juce::String& m);

    static juce::String getMidiMessageDescription(const juce::MidiMessage& m);


    //==============================================================================
    void paint(juce::Graphics& g) override;
    void resized() override;

    void updateAngleDelta();
    double startTime;

private:
    //==============================================================================
    // Your private member variables go here...
    juce::Slider levelSlider;
    juce::Label levelLabel;
    juce::Slider freqSlider;
    juce::Label freqLabel;
    juce::TextEditor midiMessagesBox;
    juce::Random random;
    juce::Slider attackSlider;
    juce::Label attackLabel;
    juce::Slider decaySlider;
    juce::Label decayLabel;
    juce::Slider sustainSlider;
    juce::Label sustainLabel;
    juce::Slider releaseSlider;
    juce::Label releaseLabel;

    juce::Label oscSelectorLabel;
    juce::ComboBox oscSelectorMenu;



    bool isAddingFromMidiInput = false;
    double currentSampleRate = 0.0;
    double currentNyquist = 0.0;
    double currentAngle = 0.0;
    double angleDelta = 0.0;
    double currentFrequency = 500.0;

    juce::MidiKeyboardState keyboardState;
    juce::MidiKeyboardComponent keyboardComponent;

    FirFilter nyquistFilter;

    bool playing = false;
    bool released = false;
    int samplesSinceStart = 0;
    int samplesSinceReleased = 0;
    double attackLength = 100;
    double decayLength = 100;
    double sustain = .5;
    double releaseLength = 100;
    int oscillator = 1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
