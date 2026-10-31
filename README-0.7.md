# WRAITH 0.7 — experimental MIDI resonator

Two VST3 targets: Instrument and FX. FX now declares MIDI input and processes MIDI note-on/off while receiving audio. Four modal resonances per MIDI voice are excited by incoming audio. Resonator Mix and Synth Mix are independent; Synth Mix defaults to zero. Existing granular path and Dry/Wet remain.

In Ableton Live, put WRAITH FX on an audio track and route MIDI from a MIDI track to WRAITH FX if the host exposes the plugin MIDI input. Play notes while audio is running. Set Resonator Mix and Resonance above zero; set Synth Mix to zero to hear only audio-driven modes.

This is a source-level prototype, not yet compiled or tested in Ableton. Keep the working 0.6 version backed up.
