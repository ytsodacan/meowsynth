#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <BinaryData.h>
#include <cmath>

MeowSynthProcessor::MeowSynthProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "STATE", createLayout())
{
    loadSample (samples[0], BinaryData::boykisser_wav, BinaryData::boykisser_wavSize);
    loadSample (samples[1], BinaryData::catmeme_wav,   BinaryData::catmeme_wavSize);

    pSound   = apvts.getRawParameterValue ("sound");
    pRoot    = apvts.getRawParameterValue ("root");
    pTune    = apvts.getRawParameterValue ("tune");
    pGain    = apvts.getRawParameterValue ("gain");
    pRelease = apvts.getRawParameterValue ("release");
    pSustain = apvts.getRawParameterValue ("sustain");
    pOneShot = apvts.getRawParameterValue ("oneshot");
}

juce::AudioProcessorValueTreeState::ParameterLayout MeowSynthProcessor::createLayout()
{
    using namespace juce;
    AudioProcessorValueTreeState::ParameterLayout l;
    l.add (std::make_unique<AudioParameterChoice> (ParameterID { "sound", 1 }, "Meow",
           StringArray { "Boykisser", "Cat Meme", "Alternate", "Layered" }, 0));
    l.add (std::make_unique<AudioParameterInt>   (ParameterID { "root", 1 }, "Root Note", 12, 108, 60));
    l.add (std::make_unique<AudioParameterFloat> (ParameterID { "tune", 1 }, "Fine Tune",
           NormalisableRange<float> (-12.0f, 12.0f, 0.01f), 0.0f));
    l.add (std::make_unique<AudioParameterFloat> (ParameterID { "gain", 1 }, "Gain",
           NormalisableRange<float> (-24.0f, 6.0f, 0.1f), 0.0f));
    l.add (std::make_unique<AudioParameterFloat> (ParameterID { "release", 1 }, "Release",
           NormalisableRange<float> (5.0f, 2000.0f, 1.0f, 0.4f), 120.0f));
    l.add (std::make_unique<AudioParameterBool>  (ParameterID { "sustain", 1 }, "Hold Loop", true));
    l.add (std::make_unique<AudioParameterBool>  (ParameterID { "oneshot", 1 }, "One Shot", false));
    return l;
}

void MeowSynthProcessor::loadSample (Sample& s, const char* data, int size)
{
    juce::AudioFormatManager fm;
    fm.registerBasicFormats();
    std::unique_ptr<juce::AudioFormatReader> r (
        fm.createReaderFor (std::make_unique<juce::MemoryInputStream> (data, (size_t) size, false)));
    if (r == nullptr) return;

    s.rate = r->sampleRate;
    s.data.setSize (1, (int) r->lengthInSamples);
    r->read (&s.data, 0, (int) r->lengthInSamples, 0, true, false);

    // sustain-loop = the steady "mmmeeow" middle of the sound
    const int n = s.data.getNumSamples();
    s.loopStart = (int) (n * 0.30);
    s.loopEnd   = (int) (n * 0.62);
}

void MeowSynthProcessor::prepareToPlay (double sr, int)
{
    hostRate = sr;
    for (auto& v : voices) v = {};
}

bool MeowSynthProcessor::isBusesLayoutSupported (const BusesLayout& l) const
{
    return l.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

MeowSynthProcessor::Voice& MeowSynthProcessor::allocVoice()
{
    for (auto& v : voices) if (! v.active) return v;
    Voice* oldest = &voices[0];
    for (auto& v : voices) if (v.order < oldest->order) oldest = &v;
    return *oldest;
}

void MeowSynthProcessor::startVoice (int note, float vel, int sampleIdx, bool primary)
{
    auto& v = allocVoice();
    const double semis = (double) note - pRoot->load() + pTune->load();
    v.active  = true;
    v.held    = true;
    v.primary = primary;
    v.note    = note;
    v.sample  = sampleIdx;
    v.pos     = 0.0;
    v.ratio   = std::pow (2.0, semis / 12.0) * samples[(size_t) sampleIdx].rate / hostRate;
    v.heldSec = 0.0;
    v.vel     = vel;
    v.env     = 0.0f;
    v.order   = ++orderCounter;
}

void MeowSynthProcessor::noteOn (int note, float vel)
{
    const int mode = (int) pSound->load();
    if (mode == 0)      startVoice (note, vel, 0, true);
    else if (mode == 1) startVoice (note, vel, 1, true);
    else if (mode == 2) { startVoice (note, vel, roundRobin, true); roundRobin ^= 1; }
    else                { startVoice (note, vel, 0, true); startVoice (note, vel, 1, false); }
}

void MeowSynthProcessor::noteOff (int note)
{
    for (auto& v : voices)
        if (v.active && v.held && v.note == note) v.held = false;
}

void MeowSynthProcessor::render (juce::AudioBuffer<float>& buf, int start, int num)
{
    if (num <= 0) return;
    auto* L = buf.getWritePointer (0, start);
    auto* R = buf.getWritePointer (1, start);

    const float gain   = juce::Decibels::decibelsToGain (pGain->load()) * 0.8f;
    const bool  loopOn = pSustain->load() > 0.5f;
    const bool  oneShot = pOneShot->load() > 0.5f;
    const float atkInc = 1.0f / (float) (0.003 * hostRate);
    const float relDec = 1.0f / (float) (juce::jmax (1.0f, pRelease->load()) * 0.001 * hostRate);
    const int   xf = 256;

    for (auto& v : voices)
    {
        if (! v.active) continue;
        auto& s = samples[(size_t) v.sample];
        const int len = s.data.getNumSamples();
        if (len < 4) { v.active = false; continue; }
        const float* d = s.data.getReadPointer (0);
        const int loopLen = s.loopEnd - s.loopStart;

        for (int i = 0; i < num; ++i)
        {
            const bool gate = v.held || oneShot;
            if (gate) v.env = juce::jmin (1.0f, v.env + atkInc);
            else      { v.env -= relDec; if (v.env <= 0.0f) { v.active = false; break; } }

            auto at = [&] (double p)
            {
                const int i0 = juce::jlimit (0, len - 1, (int) p);
                const int i1 = juce::jmin (i0 + 1, len - 1);
                const float f = (float) (p - (int) p);
                return d[i0] + (d[i1] - d[i0]) * f;
            };

            float out = at (v.pos);
            const bool looping = loopOn && v.held && loopLen > xf * 2;
            if (looping && v.pos > s.loopEnd - xf)
            {
                const float t = (float) ((v.pos - (s.loopEnd - xf)) / xf);
                out = out * (1.0f - t) + at (v.pos - loopLen) * t;
            }

            out *= v.env * v.vel * gain;
            L[i] += out;
            R[i] += out;

            v.pos += v.ratio;
            if (looping && v.pos >= s.loopEnd) v.pos -= loopLen;
            if (v.pos >= len - 1) { v.active = false; break; }
        }
        v.heldSec += (double) num / hostRate;
    }
}

void MeowSynthProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();

    int pos = 0;
    const int n = buffer.getNumSamples();
    for (const auto meta : midi)
    {
        const int t = juce::jlimit (pos, n, meta.samplePosition);
        render (buffer, pos, t - pos);
        pos = t;
        const auto m = meta.getMessage();
        if (m.isNoteOn())       noteOn (m.getNoteNumber(), m.getFloatVelocity());
        else if (m.isNoteOff()) noteOff (m.getNoteNumber());
        else if (m.isAllNotesOff() || m.isAllSoundOff()) for (auto& v : voices) v.held = false;
    }
    render (buffer, pos, n - pos);

    // tell the editor what the newest meow is doing
    const Voice* newest = nullptr;
    for (auto& v : voices)
        if (v.active && v.primary && (newest == nullptr || v.order > newest->order)) newest = &v;

    if (newest != nullptr)
    {
        const auto& s = samples[(size_t) newest->sample];
        animPhase.store ((float) (newest->pos / juce::jmax (1, s.data.getNumSamples())));
        animStretch.store (newest->held ? (float) juce::jmin (1.0, newest->heldSec / 1.5) : 0.0f);
        animActive.store (true);
    }
    else animActive.store (false);
}

juce::AudioProcessorEditor* MeowSynthProcessor::createEditor() { return new MeowSynthEditor (*this); }

void MeowSynthProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    if (auto xml = apvts.copyState().createXml()) copyXmlToBinary (*xml, dest);
}

void MeowSynthProcessor::setStateInformation (const void* data, int size)
{
    if (auto xml = getXmlFromBinary (data, size))
        if (xml->hasTagName (apvts.state.getType())) apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new MeowSynthProcessor(); }
