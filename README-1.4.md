# WRAITH 1.4 — Sound Lab

Temporary compact UI without visualization. MIDI-only note input; audio-input resonators.

## DSP changes
- Separate filtered reverb return from direct resonances. Spatial feed high-passed ~145–270 Hz to reduce low-frequency accumulation.
- Energy-following gain management before saturator, strongest for Cloud.
- Reduced Cloud fundamental accumulation and Cloud gain.
- Slightly reduced high filter-Q ceiling to avoid piercing peaks.
- All 1.3 parameters preserved, including Character, Position, Root, Decay, Space, Time, Motion, and 4-control filter.

## Build
Push contents to repository root; run GitHub Actions `Build WRAITH 1.4 FX VST3`.

## Testing
Try sustained 1-note, 4–6-note chords, and quick chord changes. Check Cloud with high Space/Time and monitor CPU/peak levels. Windows VST3 build and listening tests have not been performed here.
