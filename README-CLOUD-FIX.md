# WRAITH 1.2.1 — Cloud clarity fix

Cloud mode was overloading the resonator bus when multiple MIDI notes overlapped. This patch lowers Cloud gain, avoids sub-octave resonance accumulation, reduces detune, shortens excess modal feedback and applies gentler saturation. Strings remains unchanged; Metal is unchanged except for the existing character crossfade.

Build via the existing GitHub Actions workflow. Windows/Ableton compilation and audio tests have not been performed here.
