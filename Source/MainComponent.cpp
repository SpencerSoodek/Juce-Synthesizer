#include "MainComponent.h"
#include "IncomingMessageCallback.h"

//==============================================================================
MainComponent::MainComponent() :
    keyboardComponent(keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard),
    startTime(juce::Time::getMillisecondCounterHiRes() * 0.001)
{
    for (int i = 0; i < 8; ++i) {
        voices[i] = Voice(&synthVariables, 44100);
    }
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

    addAndMakeVisible(attackSlider);
    attackSlider.setRange(0.0, 1000.0);
    attackLabel.setText("Attack", juce::dontSendNotification);
    attackLabel.attachToComponent(&attackSlider, true);
    attackSlider.setValue(synthVariables.attackLength);
    attackSlider.onValueChange = [this] {synthVariables.attackLength = attackSlider.getValue(); };

    addAndMakeVisible(decaySlider);
    decaySlider.setRange(0.0, 1000.0);
    decayLabel.setText("Decay", juce::dontSendNotification);
    decayLabel.attachToComponent(&decaySlider, true);
    decaySlider.setValue(synthVariables.decayLength);
    decaySlider.onValueChange = [this] {synthVariables.decayLength = decaySlider.getValue(); };

    addAndMakeVisible(sustainSlider);
    sustainSlider.setRange(0.0, 1.0);
    sustainLabel.setText("Sustain", juce::dontSendNotification);
    sustainLabel.attachToComponent(&sustainSlider, true);
    sustainSlider.setValue(synthVariables.sustain);
    sustainSlider.onValueChange = [this] {synthVariables.sustain = sustainSlider.getValue(); };

    addAndMakeVisible(releaseSlider);
    releaseSlider.setRange(0.0, 1000.0);
    releaseLabel.setText("Release", juce::dontSendNotification);
    releaseLabel.attachToComponent(&releaseSlider, true);
    releaseSlider.setValue(synthVariables.releaseLength);
    releaseSlider.onValueChange = [this] {synthVariables.releaseLength = releaseSlider.getValue(); };

    addAndMakeVisible(oscSelectorMenu);
    oscSelectorMenu.addItem("Sine", 1);
    oscSelectorMenu.addItem("Saw", 2);
    oscSelectorMenu.addItem("Square", 3);
    oscSelectorMenu.addItem("White Noise", 4);
    oscSelectorMenu.onChange = [this]{ oscSelectorMenuChanged(); };
    oscSelectorMenu.setSelectedId(1);

    addAndMakeVisible(filterCutoffSlider);
    filterCutoffSlider.setRange(0.0f, 20000.0f);
    filterCutoffSlider.setSkewFactorFromMidPoint(5000.0);
    filterCutoffLabel.setText("Filter cutoff", juce::dontSendNotification);
    filterCutoffLabel.attachToComponent(&filterCutoffSlider, true);
    filterCutoffSlider.setValue(20000.0f);
    filterCutoffSlider.onValueChange = [this] {adjustableFilter.setCutoff(filterCutoffSlider.getValue()); };

    addAndMakeVisible(filterResonanceSlider);
    filterResonanceSlider.setRange(0.1f, 18.0f);
    filterResonanceLabel.setText("Filter Resonance", juce::dontSendNotification);
    filterResonanceLabel.attachToComponent(&filterResonanceSlider, true);
    filterResonanceSlider.setValue(0.71f);
    filterResonanceSlider.onValueChange = [this] {adjustableFilter.setResonance(filterResonanceSlider.getValue()); };

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

    addAndMakeVisible(filterTypeMenu);
    filterTypeMenu.addItem("Low Pass", 1);
    filterTypeMenu.addItem("High Pass", 2);
    filterTypeMenu.onChange = [this] { adjustableFilter.setFilterType(filterTypeMenu.getSelectedId()); };
    filterTypeMenu.setSelectedId(1);
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

    for (int i = 0; i < 8; ++i) {
        voices[i] = Voice(&synthVariables, sampleRate);
    }

    juce::String message;
    message << "Preparing to play audio...\n";
    message << " samplesPerBlockExpected = " << samplesPerBlockExpected << "\n";
    message << " sampleRate = " << sampleRate;
    juce::Logger::getCurrentLogger()->writeToLog(message);

    currentSampleRate = sampleRate;
    currentNyquist = currentSampleRate / 2.0;
    nyquistFilter = FirFilter::FirFilter(129, currentNyquist, currentSampleRate);
    adjustableFilter = BiquadFilter::BiquadFilter(currentNyquist, 3.0f, currentSampleRate, 1);
    updateAngleDelta();
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    auto level = (float)levelSlider.getValue();


    auto* leftBuffer = bufferToFill.buffer->getWritePointer(0, bufferToFill.startSample);
    auto* rightBuffer = bufferToFill.buffer->getWritePointer(1, bufferToFill.startSample);
    for (auto sample = 0; sample < bufferToFill.numSamples; ++sample) {
        float currentSample = 0;
        for (int i = 0; i < std::size(voices); i++) {
            if (voices[i].playing) {
                currentSample += voices[i].currentSample();
            }
        }
        currentSample = adjustableFilter.applyFilter(currentSample);
        currentSample = nyquistFilter.applyFilter(currentSample);

        leftBuffer[sample] = currentSample * level;
        rightBuffer[sample] = currentSample * level;
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
    attackSlider.setBounds(sliderLeft, 50, sliderRight, 20);
    decaySlider.setBounds(sliderLeft, 80, sliderRight, 20);
    sustainSlider.setBounds(sliderLeft, 110, sliderRight, 20);
    releaseSlider.setBounds(sliderLeft, 140, sliderRight, 20);
    oscSelectorMenu.setBounds(sliderLeft, 170, sliderRight, 20);
    filterCutoffSlider.setBounds(sliderLeft, 200, sliderRight, 20);
    filterResonanceSlider.setBounds(sliderLeft, 230, sliderRight, 20);
    filterTypeMenu.setBounds(sliderLeft, 260, sliderRight, 20);
    keyboardComponent.setBounds(0, getHeight() - 150, getWidth(), 150);
    //midiMessagesBox.setBounds(10, 80, getWidth() - 20, getHeight() - 240);
}

void MainComponent::updateAngleDelta() {
    auto cyclesPerSample = currentFrequency / currentSampleRate;
    angleDelta = cyclesPerSample * 2.0 * juce::MathConstants<double>::pi;
}


void MainComponent::handleNoteOn(juce::MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity) {
    auto m = juce::MidiMessage::noteOn(midiChannel, midiNoteNumber, velocity);
    m.setTimeStamp(juce::Time::getMillisecondCounterHiRes() * .001);
    for (int i = 0; i < std::size(voices); i++) {
        if (!voices[i].playing) {
            voices[i].noteOn(midiNoteNumber, velocity);
            break;
        }
    }
}

void MainComponent::handleNoteOff(juce::MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity) {
    auto m = juce::MidiMessage::noteOff(midiChannel, midiNoteNumber, velocity);
    m.setTimeStamp(juce::Time::getMillisecondCounterHiRes() * .001);
    for (int i = 0; i < std::size(voices); i++) {
        if (voices[i].playing && voices[i].midiNoteNumber == midiNoteNumber && !voices[i].released) {
            voices[i].noteOff();
            break;
        }
    }
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

void MainComponent::oscSelectorMenuChanged() {
    synthVariables.oscillator = oscSelectorMenu.getSelectedId();
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
