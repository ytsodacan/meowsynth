#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <array>
#include <atomic>

class MeowSynthProcessor : public juce::AudioProcessor
{
public:
    MeowSynthProcessor();
    ~MeowSynthProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout&) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "MeowSynth"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 2.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    juce::AudioProcessorValueTreeState apvts;

    // Read by the editor to animate the cat
    std::atomic<bool>  animActive  { false };
    std::atomic<float> animPhase   { 0.0f };   // 0..1 position inside the meow
    std::atomic<float> animStretch { 0.0f };   // 0..1, grows the longer the note is held

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();

    struct Sample
    {
        juce::AudioBuffer<float> data;
        double rate = 44100.0;
        int loopStart = 0, loopEnd = 0;
    };
    std::array<Sample, 2> samples;
    void loadSample (Sample& s, const char* data, int size);

    struct Voice
    {
        bool active = false, held = false, primary = true;
        int note = 0, sample = 0;
        double pos = 0.0, ratio = 1.0, heldSec = 0.0;
        float vel = 1.0f, env = 0.0f;
        juce::uint64 order = 0;
    };
    static constexpr int numVoices = 16;
    std::array<Voice, numVoices> voices;
    juce::uint64 orderCounter = 0;
    int roundRobin = 0;
    double hostRate = 44100.0;

    // cached params
    std::atomic<float> *pSound, *pRoot, *pTune, *pGain, *pRelease, *pSustain, *pOneShot;

    void noteOn (int note, float vel);
    void noteOff (int note);
    Voice& allocVoice();
    void startVoice (int note, float vel, int sampleIdx, bool primary);
    void render (juce::AudioBuffer<float>&, int start, int num);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MeowSynthProcessor)
};
