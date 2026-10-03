#pragma once
#include "PluginProcessor.h"
#include <array>

class MeowSynthEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit MeowSynthEditor (MeowSynthProcessor&);
    ~MeowSynthEditor() override = default;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    MeowSynthProcessor& proc;
    std::array<juce::Image, 16> frames;
    int   frameIdx = 0;
    float shownStretch = 0.0f;

    juce::ComboBox soundBox;
    juce::Slider rootS, tuneS, gainS, relS;
    juce::Label  rootL, tuneL, gainL, relL;
    juce::ToggleButton sustainB { "Hold loop" }, oneShotB { "One shot" };

    using SA = juce::AudioProcessorValueTreeState::SliderAttachment;
    using CA = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using BA = juce::AudioProcessorValueTreeState::ButtonAttachment;
    std::unique_ptr<SA> rootA, tuneA, gainA, relA;
    std::unique_ptr<CA> soundA;
    std::unique_ptr<BA> sustainA, oneShotA;

    juce::Rectangle<int> catPanel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MeowSynthEditor)
};
