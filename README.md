# WRAITH VST3 — Source Edition 0.2

Original polyphonic resonator-instrument prototype for Windows 11 and Ableton Live.
This project is inspired by broad experimental resonator and ambient-instrument ideas; it contains no Kinotone Audio code, samples, branding, or copied assets.

## Features in this source stage
- JUCE 8 + CMake project; VST3 and Standalone targets
- MIDI input and 16 voice slots
- Event-offset rendering: note/controller events split the audio block at event sample positions
- MIDI note-on/note-off, velocity, channel-aware note handling
- Sustain pedal CC64, All Sound Off CC120, All Notes Off CC123
- Original additive/inharmonic resonator-style oscillator voice
- Resonance, Decay, Space, Motion and Output controls
- Stereo reverb and saved parameter state

## Not implemented yet
- Granular buffer engine, reverse grains, shimmer pitch shifting
- Full physical string modelling
- MIDI pitch bend and aftertouch
- Preset browser and factory presets
- Automated audio/MIDI regression tests
- Verified compiled binary or confirmed Ableton-host test

This is source code, not a prebuilt or certified plugin. Build and host testing remain required.

## Requirements
- Windows 11 x64
- Visual Studio 2022 with "Desktop development with C++"
- CMake 3.22 or newer
- Git
- Internet connection for the first configure (JUCE is fetched from its official repository)

## Build
Open "x64 Native Tools Command Prompt for VS 2022" and run these commands from this folder:

```bat
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release --target WRAITH_VST3
```

Look for `WRAITH.vst3` under `build\\WRAITH_artefacts\\Release\\VST3\\`.
If JUCE did not copy it to the standard location, copy it to:

`C:\\Program Files\\Common Files\\VST3\\`

Restart Ableton Live. Create a MIDI track, load WRAITH as the instrument, select/arm the MIDI input and play a triad, then a 7-note chord. Test CC64 sustain and the Decay/Space controls.

## Known prototype caveat
The synthesis engine is a simple real-time oscillator model designed to establish MIDI/polyphonic architecture. It is not yet the intended final hybrid granular/resonator sound, and has not been compiled or validated in Ableton as part of this source delivery. Monitor at a low level during first tests.

## Automatic Windows build via GitHub Actions

See [`README-AUTOMATIC-BUILD.md`](README-AUTOMATIC-BUILD.md). The workflow at `.github/workflows/build-windows.yml` builds the Release VST3 on a Windows x64 runner and uploads a downloadable artifact named `WRAITH-VST3-Windows-x64`.
