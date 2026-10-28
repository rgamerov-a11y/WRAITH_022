# WRAITH VST3 — Hybrid Resonator + Granular Texture 0.4

Original polyphonic instrument prototype for Windows 11 and Ableton Live. WRAITH combines an inharmonic resonator voice with a real-time grain cloud built from an internally generated excitation texture. No third-party samples or copied instrument code are used.

## New in 0.4
- Six overlapping grains per note voice, with Hann windows, randomized start positions, pitch-following playback rate, and stereo spread.
- Four new automatable controls: **Grain Mix**, **Grain Size**, **Grain Density**, and **Grain Tone**.
- 16-voice MIDI polyphony, event-offset note rendering, sustain pedal CC64, All Sound Off CC120 and All Notes Off CC123 retained.
- Grain source is generated internally at prepare time; no sample files are required.
- Fixed the pluck transient so it no longer restarts at each MIDI event/block split.
- Expanded plugin UI for the new hybrid controls.

## Controls
- **Resonance** — emphasizes inharmonic partials.
- **Decay** — note release duration.
- **Space** — stereo reverb amount.
- **Motion** — movement in the resonator stereo field and partial relationship.
- **Grain Mix** — crossfades in the granular layer while slightly softening the resonator layer.
- **Grain Size** — grain duration, 25–280 ms.
- **Grain Density** — new grains per second per active voice.
- **Grain Tone** — blends soft, filtered texture and brighter noise/partial content; also widens grain panning.
- **Output** — master level.

## Build on Windows 11 x64
Requirements: Visual Studio 2022 with **Desktop development with C++**, CMake 3.22+, Git, and internet access for the first JUCE fetch.

Run `BUILD-WINDOWS.bat`, or execute in an x64 Native Tools Command Prompt for VS 2022:

```bat
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release --target WRAITH_VST3 --parallel
```

The bundle should appear under `build\WRAITH_artefacts\Release\VST3\WRAITH.vst3`. Copy the entire `.vst3` bundle folder to `C:\Program Files\Common Files\VST3\`, then restart Ableton Live and rescan plugins.

## Automated GitHub Actions build
The included `.github/workflows/build-windows.yml` runs on pushes to `main`/`master` and can be started manually from the repository's **Actions** tab. It uploads the artifact **WRAITH-0.4-VST3-Windows-x64** for 30 days. Upload the contents of this project folder to your repository (including the hidden `.github` folder); this package does not push changes to GitHub automatically.

## Important status
This is source code plus a Windows CI build workflow, not a binary verified in this delivery. The granular layer is a procedural excitation-texture grain cloud, not a user-loaded sample-buffer/granular sampler. Reverse playback, shimmer pitch shifting, user sample import, preset browser, pitch bend/aftertouch and automated audio regression tests remain future work. A successful GitHub Actions build confirms compilation and packaging, not full host certification. Test with the master level low first.
