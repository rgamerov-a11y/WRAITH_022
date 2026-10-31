# WRAITH 0.8 — Morphing Resonator (experimental)

Two VST3 plugins: WRAITH Instrument and WRAITH FX. Based on the supplied working 0.7 source.

New controls: Character (0 Strings, 0.5 Metal, 1 Cloud), Harmonics, Damping.

The FX morphs four MIDI-tuned audio-excited resonant partials continuously. Existing MIDI chords, granular Freeze/Reverse, Dry/Wet, Synth Mix and Resonator Mix remain.

This is an uncompiled prototype; build and test with GitHub Actions on Windows. Character changes resonator frequencies in real time; rapid modulation can cause audible artifacts. Glide and independent Resonance Freeze are not implemented in this build.

For Ableton Live, route MIDI from a separate MIDI track to WRAITH FX on an audio track.
