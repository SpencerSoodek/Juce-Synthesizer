#pragma once


#include <JuceHeader.h>

class MainComponent;

class IncomingMessageCallback : public juce::CallbackMessage
{
public:
    IncomingMessageCallback(MainComponent* owner, const juce::MidiMessage& message, const juce::String& source);

    void messageCallback() override;

private:
    juce::AudioAppComponent::SafePointer<MainComponent> owner;
    juce::MidiMessage message;
    juce::String source;
};
