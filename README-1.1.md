# WRAITH 1.1 — Resonant FX

Single VST3 audio effect for Ableton Live, with MIDI-controlled polyphonic resonances and a monochrome interactive UI.

## Changes from 1.0
- Only `WRAITH FX` is built. No Instrument target.
- Granular synthesis, audio capture, reverse and freeze removed from C++ DSP and UI.
- No internal synthesizer. Incoming audio excites 16 MIDI-tuned resonator voices.
- Character Morph: Strings → Metal → Cloud, continuous horizontal fader.
- Resonator controls: Harmonics, Damping, Glide.
- Single Filter control: dark low-pass → open → thin high-pass.
- Compact Mix and Out controls; monochrome MIDI-reactive visualization.
- Reverb/decay/resonance retain internal parameters and can be automated via the host.

## Build
Use GitHub Actions workflow **Build WRAITH 1.1 FX VST3**. The workflow packages one `WRAITH FX.vst3`.

## Test
Put WRAITH FX on an audio track. Route a MIDI track to the effect in Ableton Live. Play audio and MIDI notes simultaneously. Check Character, Filter, Glide, Mix and Out. No resonator sound is expected without incoming audio or without MIDI notes.

## Caveats
The source archive is not a prebuilt Windows binary and has not been compiled on a Windows machine here. Back up WRAITH 1.0 before replacing your repository. Existing project presets may need adjustment because some removed parameter IDs are no longer present.
