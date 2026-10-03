#include "PluginEditor.h"
#include <BinaryData.h>

MeowSynthEditor::MeowSynthEditor (MeowSynthProcessor& p) : AudioProcessorEditor (&p), proc (p)
{
    for (int i = 0; i < 16; ++i)
    {
        int size = 0;
        const auto name = juce::String::formatted ("f%02d_png", i + 1);
        if (auto* data = BinaryData::getNamedResource (name.toRawUTF8(), size))
            frames[(size_t) i] = juce::ImageCache::getFromMemory (data, size);
    }

    auto setup = [this] (juce::Slider& s, juce::Label& l, const juce::String& text)
    {
        s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 70, 16);
        s.setColour (juce::Slider::rotarySliderFillColourId, juce::Colour (0xffff8fb8));
        addAndMakeVisible (s);
        l.setText (text, juce::dontSendNotification);
        l.setJustificationType (juce::Justification::centred);
        l.setColour (juce::Label::textColourId, juce::Colours::white);
        addAndMakeVisible (l);
    };
    setup (rootS, rootL, "Root");
    setup (tuneS, tuneL, "Tune");
    setup (gainS, gainL, "Gain");
    setup (relS,  relL,  "Release");
    gainS.setTextValueSuffix (" dB");
    relS.setTextValueSuffix (" ms");
    tuneS.setTextValueSuffix (" st");

    soundBox.addItemList ({ "Boykisser", "Cat Meme", "Alternate", "Layered" }, 1);
    addAndMakeVisible (soundBox);
    for (auto* b : { &sustainB, &oneShotB })
    {
        b->setColour (juce::ToggleButton::textColourId, juce::Colours::white);
        addAndMakeVisible (*b);
    }

    auto& a = proc.apvts;
    rootA = std::make_unique<SA> (a, "root", rootS);
    tuneA = std::make_unique<SA> (a, "tune", tuneS);
    gainA = std::make_unique<SA> (a, "gain", gainS);
    relA  = std::make_unique<SA> (a, "release", relS);
    soundA = std::make_unique<CA> (a, "sound", soundBox);
    sustainA = std::make_unique<BA> (a, "sustain", sustainB);
    oneShotA = std::make_unique<BA> (a, "oneshot", oneShotB);

    setSize (360, 540);
    startTimerHz (60);
}

void MeowSynthEditor::timerCallback()
{
    const bool active = proc.animActive.load();
    // the GIF follows the meow itself: it starts with the note, runs at the
    // (pitched) speed of the sound, and is cut off when the note is released
    frameIdx = active ? juce::jlimit (0, 15, (int) (proc.animPhase.load() * 16.0f)) : 0;
    const float target = active ? proc.animStretch.load() : 0.0f;
    shownStretch += (target - shownStretch) * 0.2f;
    repaint (catPanel);
}

void MeowSynthEditor::paint (juce::Graphics& g)
{
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff2b1a3d), 0, 0,
                                             juce::Colour (0xff12091c), 0, (float) getHeight(), false));
    g.fillAll();

    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (22.0f, juce::Font::bold));
    g.drawText ("MeowSynth", 0, 8, getWidth(), 28, juce::Justification::centred);

    auto panel = catPanel.toFloat();
    g.setColour (juce::Colour (0xfffafafa));
    g.fillRoundedRectangle (panel, 14.0f);
    g.setColour (juce::Colour (0xffff8fb8));
    g.drawRoundedRectangle (panel, 14.0f, 2.0f);

    const auto& img = frames[(size_t) frameIdx];
    if (img.isValid())
    {
        juce::Graphics::ScopedSaveState ss (g);
        juce::Path clip; clip.addRoundedRectangle (panel.reduced (2.0f), 12.0f);
        g.reduceClipRegion (clip);

        // cat sits on the bottom of the panel and stretches taller the longer you hold the note
        const float baseH = panel.getHeight() * 0.78f;
        const float baseW = baseH * (float) img.getWidth() / (float) img.getHeight();
        const float h = baseH * (1.0f + 0.30f * shownStretch);
        const float w = baseW * (1.0f - 0.08f * shownStretch);
        g.drawImage (img, juce::Rectangle<float> (panel.getCentreX() - w * 0.5f, panel.getBottom() - h - 6.0f, w, h),
                     juce::RectanglePlacement::stretchToFit);
    }
}

void MeowSynthEditor::resized()
{
    auto r = getLocalBounds().reduced (14);
    r.removeFromTop (32);
    catPanel = r.removeFromTop (310);
    r.removeFromTop (12);

    auto row = r.removeFromTop (30);
    soundBox.setBounds (row.removeFromLeft (120));
    row.removeFromLeft (8);
    sustainB.setBounds (row.removeFromLeft (100));
    oneShotB.setBounds (row);
    r.removeFromTop (10);

    const int w = r.getWidth() / 4;
    juce::Slider* sl[] = { &rootS, &tuneS, &gainS, &relS };
    juce::Label*  lb[] = { &rootL, &tuneL, &gainL, &relL };
    for (int i = 0; i < 4; ++i)
    {
        auto c = r.removeFromLeft (w);
        lb[i]->setBounds (c.removeFromTop (18));
        sl[i]->setBounds (c);
    }
}
