#include "IncomingMessageCallback.h"
#include "MainComponent.h"

IncomingMessageCallback::IncomingMessageCallback(MainComponent* owner, const juce::MidiMessage& message, const juce::String& source)
    : owner(owner), message(message), source(source)
{}

void IncomingMessageCallback::messageCallback()
{
    if (owner != nullptr)
        owner->addMessageToList(message, source);
}
