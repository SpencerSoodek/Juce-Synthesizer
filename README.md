MySynthesizer is a synthesizer built using C++ and the JUCE Framework. Users can generate sine, sawtooth, square waves, and white noise. The synthesizer features an adjustable amplitude envelope with ADSR controls, and an adjustable biquad filter with controls for the cutoff frequency, resonance, and filter type. The synthesizer uses an FIR filter at the nyquist frequency, and synthesizes band limmited waves from the fundamental frequency to the nyqust to prevent aliasing.

All audio synthesis, filtering, and amplitude envelope logic is written in the code. The JUCE library is used to input MIDI notes and to output audio, and JUCE filters are not used.

To run this project:
* Install the JUCE framework on your system.
* Clone the repository.
* Using Projucer from the JUCE Framework, open "MySynthesizer.jucer".
* Select an exporter and save the project. Open the project in your IDE.
* Build and run the project using your IDE.
