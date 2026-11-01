# WRAITH 1.5 — Reverb PRE / POST

New **REVERB** knob: -1 = PRE (input reverb excites resonators), 0 = no reverb, +1 = POST (resonator output feeds reverb). The existing **TIME** controls the decay of both reverbs; **POSITION** controls harmonic excitation nodes.

This is an original DSP implementation based on the *control behavior* described in the provided Ghosts manual screenshots, not a reproduction of proprietary circuitry. JUCE Reverb roomSize approaches 0.999 at maximum Time: this is a long decay, **not a mathematically infinite freeze**.

Build target: WRAITH_FX_VST3. Windows build and audio have not been validated.
