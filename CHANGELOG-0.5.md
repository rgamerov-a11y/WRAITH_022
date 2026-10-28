# WRAITH 0.5 change log

- Added an 8-second preallocated stereo input-capture ring buffer.
- Added input-backed grain playback with a four-second selectable read window.
- Added Reverse Grains and Freeze Texture parameters and UI toggles.
- Preserved the 16-voice resonator, MIDI note handling, sustain, reverb, parameter automation and state serialization.
- Updated project version and GitHub Actions artifact name to 0.5.

## Known verification items

- The source and workflow are prepared, but this environment has not compiled the project because JUCE could not be fetched here.
- Confirm that the selected Ableton Live version exposes/routes the instrument's audio input bus as expected.
- Captured audio is runtime memory and is not serialized into the project; recapture after reopening a session.
