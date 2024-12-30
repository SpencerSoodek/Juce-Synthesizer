#include "MainComponent.h"
#include "IncomingMessageCallback.h"

//==============================================================================
MainComponent::MainComponent() :
    keyboardComponent(keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard),
    startTime(juce::Time::getMillisecondCounterHiRes() * 0.001)
{
    // Make sure you set the size of the component after
    // you add any child components.
    keyboardState.addListener(this);
    setSize(800, 600);
    setAudioChannels(0, 2);
    addAndMakeVisible(levelSlider);
    levelSlider.setRange(0.0, 1.0);
    addAndMakeVisible(levelLabel);
    levelLabel.setText("Level", juce::dontSendNotification);
    levelLabel.attachToComponent(&levelSlider, true);
    freqSlider.setRange(20.0, 20000.0);
    freqSlider.setSkewFactorFromMidPoint(5000.0);
    freqSlider.setValue(currentFrequency, juce::dontSendNotification);
    freqSlider.onValueChange = [this] {
        if (currentSampleRate > 0.0) {
            updateAngleDelta();
        }
        };

    addAndMakeVisible(freqSlider);
    freqLabel.setText("Frequency", juce::dontSendNotification);
    freqLabel.attachToComponent(&freqSlider, true);

    addAndMakeVisible(attackSlider);
    attackSlider.setRange(0.0, 1000.0);
    attackLabel.setText("Attack", juce::dontSendNotification);
    attackLabel.attachToComponent(&attackSlider, true);
    attackSlider.setValue(attackLength);
    attackSlider.onValueChange = [this] {attackLength = attackSlider.getValue(); };

    addAndMakeVisible(decaySlider);
    decaySlider.setRange(0.0, 1000.0);
    decayLabel.setText("Decay", juce::dontSendNotification);
    decayLabel.attachToComponent(&decaySlider, true);
    decaySlider.setValue(decayLength);
    decaySlider.onValueChange = [this] {decayLength = decaySlider.getValue(); };

    addAndMakeVisible(sustainSlider);
    sustainSlider.setRange(0.0, 1.0);
    sustainLabel.setText("Sustain", juce::dontSendNotification);
    sustainLabel.attachToComponent(&sustainSlider, true);
    sustainSlider.setValue(sustain);
    sustainSlider.onValueChange = [this] {sustain = sustainSlider.getValue(); };

    addAndMakeVisible(releaseSlider);
    releaseSlider.setRange(0.0, 1000.0);
    releaseLabel.setText("Release", juce::dontSendNotification);
    releaseLabel.attachToComponent(&releaseSlider, true);
    releaseSlider.setValue(releaseLength);
    releaseSlider.onValueChange = [this] {releaseLength = releaseSlider.getValue(); };

    addAndMakeVisible(keyboardComponent);
    addAndMakeVisible(midiMessagesBox);
    midiMessagesBox.setMultiLine(true);
    midiMessagesBox.setReturnKeyStartsNewLine(true);
    midiMessagesBox.setReadOnly(true);
    midiMessagesBox.setScrollbarsShown(true);
    midiMessagesBox.setCaretVisible(false);
    midiMessagesBox.setPopupMenuEnabled(true);
    midiMessagesBox.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0x32ffffff));
    midiMessagesBox.setColour(juce::TextEditor::outlineColourId, juce::Colour(0x1c000000));
    midiMessagesBox.setColour(juce::TextEditor::shadowColourId, juce::Colour(0x16000000));
}

MainComponent::~MainComponent()
{
    // This shuts down the audio device and clears the audio source.
    shutdownAudio();
}

//==============================================================================
void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    // This function will be called when the audio device is started, or when
    // its settings (i.e. sample rate, block size, etc) are changed.

    // You can use this function to initialise any resources you might need,
    // but be careful - it will be called on the audio thread, not the GUI thread.

    // For more details, see the help for AudioProcessor::prepareToPlay()
    juce::String message;
    message << "Preparing to play audio...\n";
    message << " samplesPerBlockExpected = " << samplesPerBlockExpected << "\n";
    message << " sampleRate = " << sampleRate;
    juce::Logger::getCurrentLogger()->writeToLog(message);

    currentSampleRate = sampleRate;
    updateAngleDelta();
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    auto level = (float)levelSlider.getValue();
    auto levelScale = level * 2.0f;


    auto* leftBuffer = bufferToFill.buffer->getWritePointer(0, bufferToFill.startSample);
    auto* rightBuffer = bufferToFill.buffer->getWritePointer(1, bufferToFill.startSample);
    for (auto sample = 0; sample < bufferToFill.numSamples; ++sample) {
        auto currentSample = (float)std::sin(currentAngle);
        currentAngle += angleDelta;

        if (playing) {
            float msSinceStart = samplesSinceStart / currentSampleRate * 1000.0;
            samplesSinceStart++;
            // Attack amplitude
            if (msSinceStart < attackLength && !released) {
                currentSample *= (msSinceStart / attackLength);
            }
            //Decay amplitude; 
            if (msSinceStart >= attackLength && msSinceStart < attackLength + decayLength && !released) {
                float msSinceAttackEnd = msSinceStart - attackLength;
                float decayAmplitude = 1.0f - (msSinceAttackEnd / decayLength) * (1.0f - sustain);
                currentSample *= decayAmplitude;
            }
            // Sustain amplitude
            if (msSinceStart >= attackLength + decayLength && !released) {
                currentSample *= sustain;
            }
            // Release amplitude
            if (released) {
                float msSinceRelease = samplesSinceReleased / currentSampleRate * 1000;
                samplesSinceReleased++;
                if (msSinceRelease >= releaseLength) {
                    playing = false;
                    samplesSinceReleased = 0;
                }
                float releaseAmplitude = sustain - ((msSinceRelease / releaseLength) * sustain);
                releaseAmplitude = juce::jlimit(0.0f, 1.0f, releaseAmplitude);
                currentSample *= releaseAmplitude;
            }
        }
        else {
            currentSample = 0;
        }


        leftBuffer[sample] = currentSample * levelScale - level;
        rightBuffer[sample] = currentSample * levelScale - level;
    }
}


void MainComponent::releaseResources()
{
    // This will be called when the audio device stops, or when it is being
    // restarted due to a setting change.

    // For more details, see the help for AudioProcessor::releaseResources()
    juce::Logger::getCurrentLogger()->writeToLog("Releasing audio resources");
}

//==============================================================================
void MainComponent::paint(juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    // You can add your drawing code here!
}

void MainComponent::resized()
{
    auto sliderLeft = 120;
    int sliderRight = getWidth() - sliderLeft - 10;
    levelSlider.setBounds(sliderLeft, 20, sliderRight, 20);
    freqSlider.setBounds(sliderLeft, 50, sliderRight, 20);
    attackSlider.setBounds(sliderLeft, 80, sliderRight, 20);
    decaySlider.setBounds(sliderLeft, 110, sliderRight, 20);
    sustainSlider.setBounds(sliderLeft, 140, sliderRight, 20);
    releaseSlider.setBounds(sliderLeft, 170, sliderRight, 20);
    keyboardComponent.setBounds(0, getHeight() - 150, getWidth(), 150);
    //midiMessagesBox.setBounds(10, 80, getWidth() - 20, getHeight() - 240);
}

void MainComponent::updateAngleDelta() {
    auto cyclesPerSample = currentFrequency / currentSampleRate;
    angleDelta = cyclesPerSample * 2.0 * juce::MathConstants<double>::pi;
}

double midiToFreq(int midiNoteNumber) {
    return 440.0 * std::pow(2.0, ((midiNoteNumber - 69) / 12.0));
}



void MainComponent::handleNoteOn(juce::MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity) {
    auto m = juce::MidiMessage::noteOn(midiChannel, midiNoteNumber, velocity);
    m.setTimeStamp(juce::Time::getMillisecondCounterHiRes() * .001);
    double newFreq = midiToFreq(midiNoteNumber);
    currentFrequency = newFreq;
    released = false;
    playing = true;
    samplesSinceStart = 0;
    freqSlider.setValue(newFreq, juce::dontSendNotification);
    updateAngleDelta();
    std::cout << "currentFrequency" << std::endl;;
    postMessageToList(m, "On-Screen keyboard");

}void MainComponent::handleNoteOff(juce::MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity) {
    auto m = juce::MidiMessage::noteOff(midiChannel, midiNoteNumber, velocity);
    m.setTimeStamp(juce::Time::getMillisecondCounterHiRes() * .001);
    released = true;
    samplesSinceReleased = 0;

    postMessageToList(m, "On-Screen keyboard");
}

void MainComponent::handleIncomingMidiMessage(juce::MidiInput* source, const juce::MidiMessage& message) {
    const juce::ScopedValueSetter<bool> scopedInputFlag(isAddingFromMidiInput, true);
    keyboardState.processNextMidiEvent(message);
    postMessageToList(message, source->getName());
}

void MainComponent::postMessageToList(const juce::MidiMessage& message, const juce::String& source)
{
    (new IncomingMessageCallback(this, message, source))->post();
}

void MainComponent::addMessageToList(const juce::MidiMessage& message, const juce::String& source) {
    auto time = message.getTimeStamp() - startTime;
    auto hours = ((int)(time / 3600.0)) % 24;
    auto minutes = ((int)(time / 60.0)) % 60;
    auto seconds = ((int)time) % 60;
    auto millis = ((int)(time * 1000.0)) % 1000;

    auto timecode = juce::String::formatted("%02d:%02d:%02d.%03d", hours, minutes, seconds, millis);
    auto description = MainComponent::getMidiMessageDescription(message);

    juce::String midiMessageString(timecode + " - " + description + " (" + source + ")");
    logMessage(midiMessageString);
}

void MainComponent::logMessage(const juce::String& m)
{
    midiMessagesBox.moveCaretToEnd();
    midiMessagesBox.insertTextAtCaret(m + juce::newLine);
}

juce::String MainComponent::getMidiMessageDescription(const juce::MidiMessage& m)
{
    if (m.isNoteOn())           return "Note on " + juce::MidiMessage::getMidiNoteName(m.getNoteNumber(), true, true, 3);
    if (m.isNoteOff())          return "Note off " + juce::MidiMessage::getMidiNoteName(m.getNoteNumber(), true, true, 3);
    if (m.isProgramChange())    return "Program change " + juce::String(m.getProgramChangeNumber());
    if (m.isPitchWheel())       return "Pitch wheel " + juce::String(m.getPitchWheelValue());
    if (m.isAftertouch())       return "After touch " + juce::MidiMessage::getMidiNoteName(m.getNoteNumber(), true, true, 3) + ": " + juce::String(m.getAfterTouchValue());
    if (m.isChannelPressure())  return "Channel pressure " + juce::String(m.getChannelPressureValue());
    if (m.isAllNotesOff())      return "All notes off";
    if (m.isAllSoundOff())      return "All sound off";
    if (m.isMetaEvent())        return "Meta event";

    if (m.isController())
    {
        juce::String name(juce::MidiMessage::getControllerName(m.getControllerNumber()));

        if (name.isEmpty())
            name = "[" + juce::String(m.getControllerNumber()) + "]";

        return "Controller " + name + ": " + juce::String(m.getControllerValue());
    }

    return juce::String::toHexString(m.getRawData(), m.getRawDataSize());
}
