# MeowSynth (VST3 for FL Studio)

Meow sampler synth. Play MIDI notes -> meow at that pitch, with a little cat that
moves with the meow. Hold a note longer: the meow loops its middle and the cat stretches taller.

Sounds: Boykisser (first meow only), Cat Meme, Alternate, Layered.
Knobs: Root note, Tune, Gain, Release. Toggles: Hold loop, One shot.

## Build (Windows, no tools needed): GitHub Actions
1. Create a free GitHub repo, upload everything in this folder (keep .github/).
2. Actions tab -> "Build MeowSynth" -> wait ~10 min -> download the MeowSynth-Windows artifact.
3. Copy MeowSynth.vst3 into C:\Program Files\Common Files\VST3\
4. FL Studio: Options > Manage plugins > Find plugins (or "Verify plugins"), then add MeowSynth from the plugin picker (More plugins).

## Build locally
Needs CMake + Visual Studio (Windows) or Xcode (Mac):
  cmake -B build -DCMAKE_BUILD_TYPE=Release
  cmake --build build --config Release --target MeowSynth_VST3
