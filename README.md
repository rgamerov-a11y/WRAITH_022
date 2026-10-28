# WRAITH VST3 — Live-Input Granular Resonator 0.5

WRAITH is an original polyphonic resonator and granular texture instrument for Windows x64 and Ableton Live. Version 0.5 adds a real-time input capture ring buffer, reverse grains and a freeze mode while retaining the resonator, MIDI polyphony and stereo reverb from 0.4.

## New in 0.5

- **Live input capture:** incoming stereo/mono audio is written to a preallocated 8-second ring buffer. Grains read from up to the most recent 4 seconds.
- **Reverse Grains:** new grains play backwards when enabled; already-running grains keep their direction so switching is smooth.
- **Freeze Texture:** stops writing to the capture buffer and pins grain reading to the capture head at the moment Freeze was enabled. Turn Freeze off to resume capture.
- **Low audio-thread overhead:** capture memory is allocated in `prepareToPlay`; the capture path does not allocate or lock in `processBlock`.
- **16-voice MIDI polyphony** with note-on/off, sustain CC64, All Sound Off CC120 and All Notes Off CC123.
- Existing Resonance, Decay, Space, Motion, Grain Mix, Grain Size, Grain Density, Grain Tone and Output controls remain automatable and state-save capable.

## Controls

- **Resonance** — emphasizes inharmonic partials.
- **Decay** — note release duration.
- **Space** — stereo reverb amount.
- **Motion** — movement in the resonator stereo field and partial relationship.
- **Grain Mix** — blends captured-input grains with the resonator.
- **Grain Size** — grain duration, 25–280 ms.
- **Grain Density** — new grains per second per active voice.
- **Grain Tone** — adjusts grain stereo spread and the procedural fallback tone before enough input has been captured.
- **Reverse Grains** — reverses newly spawned grains.
- **Freeze Texture** — holds the capture buffer and creates grains from the frozen material.
- **Output** — master level.

## Using input capture in Ableton Live

1. Route an audio signal into WRAITH's **Input** bus (for example, a resampled synth, external input, or audio track routed to the track hosting WRAITH).
2. Start playback and allow a moment of audio to be captured.
3. Play MIDI notes. Raise Grain Mix to hear captured material as pitched granular texture alongside the resonator.
4. Enable **Freeze Texture** to hold the captured material; enable **Reverse Grains** for backwards grains.
5. Start with Output low and watch the track/master meter when using dense chords.

Host routing for a MIDI-capable plugin with an audio input can vary. If Live does not expose the input bus for this instrument, use a supported audio routing path or report the exact plugin/track configuration; the source includes the input bus but host behavior must be verified in the target Live version.

## Build on Windows 11 x64

Requirements: Visual Studio 2022 with **Desktop development with C++**, CMake 3.22+, Git, and internet access for the first JUCE fetch.

Run `BUILD-WINDOWS.bat`, or execute in an x64 Native Tools Command Prompt for VS 2022:

```bat
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release --target WRAITH_VST3 --parallel
```

The bundle should appear under `build\WRAITH_artefacts\Release\VST3\WRAITH.vst3`. Copy the entire `.vst3` bundle folder to `C:\Program Files\Common Files\VST3\`, then restart Ableton Live and rescan plugins.

## Automated GitHub Actions build

The included `.github/workflows/build-windows.yml` runs on pushes to `main`/`master` and can be started manually from the repository's **Actions** tab. It uploads **`WRAITH-0.5-VST3-Windows-x64`** for 30 days. Upload the contents of this project folder to your repository, including `.github/workflows/build-windows.yml`; this package does not push changes to GitHub automatically.

## Verification status and limitations

This delivery contains source code and a Windows CI workflow, not a prebuilt binary verified in this environment. A successful GitHub Actions run confirms compilation and packaging, not complete Ableton host certification or audio quality. Test the input bus, capture, Reverse, Freeze, MIDI chords, sustain and state restoration in your own Live setup. If the host does not expose audio input for this instrument category, plugin bus/category adjustments may be required.
