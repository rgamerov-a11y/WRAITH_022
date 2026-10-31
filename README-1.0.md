# WRAITH 1.0 — Interactive UI preview

Based on the supplied working WRAITH 0.9 repository.

- Rebuilt minimal dark JUCE interface, central animated procedural particle field (30 FPS).
- Character morph changes the field between Strings, Metal and Cloud; Motion changes its speed.
- MIDI note-on triggers a visual ripple, with note pitch controlling its horizontal position and velocity its energy. Click the field to preview a ripple.
- Removed Resonance Freeze parameter, DSP latch, and GUI control. Granular Freeze and Glide remain.
- Existing Instrument and FX VST3 targets and GitHub Actions workflow retained.

## Important

The particle field is a **procedural approximation inspired by the supplied 01.mp4**, not embedded footage or an exact recreation. It is rendered with JUCE Graphics on the UI thread, never the real-time audio thread. Visual performance and plugin compilation still require testing on Windows with GitHub Actions and Ableton Live.

Removing the Resonance Freeze parameter can affect old session automation targeting that parameter. Back up your working 0.9 project and plugin before installing 1.0.
