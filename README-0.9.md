# WRAITH 0.9 — Expressive Resonator

Based on WRAITH 0.8. Adds Glide (0–500 ms) and separate Resonance Freeze.

Resonance Freeze latches currently held MIDI notes on activation, sustains their resonator modes with near-unity feedback and disables further input excitation for those voices. New notes can play on top. Turning Freeze off releases latched voices. Granular Freeze remains independent. Glide smoothly interpolates oscillator and resonator tuning from the previous played pitch.

**Note:** Freeze is a long-decay modal hold, not a mathematically infinite spectral freeze. At high levels feedback can cause extended ringing; use output gain responsibly.

Build using the existing GitHub Actions workflow. Windows build has not been tested here.
