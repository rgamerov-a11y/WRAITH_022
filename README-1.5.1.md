# WRAITH 1.5.1 — Mix & Utility

Replaces the Sparks-style active filter with independent LOW-PASS, HIGH-PASS and one-knob COMPRESSION. The existing MIX knob remains in OUTPUT. RESET returns LP to fully open, HP to fully open and compression to zero; DSP parameter smoothing is 80 ms. RESET does not touch MIX, OUT, reverb or resonator parameters.

Existing legacy filter parameters remain in the APVTS for compatibility but are bypassed. Existing presets default to the neutral new filter settings. This is a WRAITH-specific digital implementation, not an exact Ribbons algorithm.

Build with GitHub Actions on Windows. The Windows VST3 has not been compiled or tested here.
