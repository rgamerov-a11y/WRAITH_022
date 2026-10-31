# WRAITH 1.2 — Enhanced Resonance

Windows 11 / Ableton Live / JUCE 8 / VST3 MIDI-controlled audio effect.

- One VST3 FX target only (`WRAITH_FX_VST3`).
- 16-voice MIDI-controlled six-mode resonator; Character morph: Strings → Metal → Cloud.
- Resonator: Harmonics, Damping, Decay, Glide, Position, Root (MIDI transposition ±12 semitones).
- Texture: Tone, Space, Time, Motion; Filter, Mix, Out.
- No Chord, no internal synth, no granular engine.
- Root affects newly triggered notes; already sounding voices retain their pitch until retrigger.
- Time adjusts reverb room size and tail, not infinite sustain.
- Motion provides gentle frequency modulation; Space controls reverb width and wetness.
- Output soft saturation added; avoid extreme wet/output settings to prevent loud peaks.

## Build
Push the source tree to the root of your GitHub repository and run `.github/workflows/build-windows.yml`.
Download artifact `WRAITH-1.2-Windows-VST3`. Windows compilation and Ableton runtime testing are still required.
