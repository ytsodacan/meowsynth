# MeowSynth (VST3 / AU for FL Studio and any DAW)

Meow sampler synth. Play MIDI notes -> meow at that pitch, with a little cat that
moves with the meow. Hold a note longer: the meow loops its middle and the cat stretches taller.

Sounds: Boykisser (first meow only), Cat Meme, Alternate, Layered.
Knobs: Root note, Tune, Gain, Release. Toggles: Hold loop, One shot.

## Build for every platform (no tools needed): GitHub Actions
1. Create a free GitHub repo, upload everything in this folder (keep the .github folder).
2. Actions tab -> "Build MeowSynth" -> wait ~10-15 min. It builds Windows, macOS and Linux.
3. Download the artifact for your OS.

## Install
- Windows: copy MeowSynth.vst3 to C:\Program Files\Common Files\VST3\
- macOS:   copy MeowSynth.vst3 to /Library/Audio/Plug-Ins/VST3/  (AU version -> /Library/Audio/Plug-Ins/Components/)
           Unsigned downloads get quarantined: run  xattr -cr /Library/Audio/Plug-Ins/VST3/MeowSynth.vst3
           (and the same for the .component if you use AU)
- Linux:   copy MeowSynth.vst3 to ~/.vst3/
Then in FL Studio: Options > Manage plugins > Find plugins, and add MeowSynth from the plugin picker.

## Build locally
  cmake -B build -DCMAKE_BUILD_TYPE=Release
  cmake --build build --config Release
