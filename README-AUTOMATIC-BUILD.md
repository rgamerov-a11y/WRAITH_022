# WRAITH 0.5 — automatic Windows VST3 build

This package contains the WRAITH 0.5 source and GitHub Actions workflow. The workflow builds a Windows x64 VST3 bundle and uploads it as an artifact.

## Build in GitHub Actions

1. Extract this ZIP.
2. Upload the **contents** of the extracted folder to your WRAITH repository, replacing the prior source files. Include `.github/workflows/build-windows.yml`.
3. Commit the changes to `main` or `master`.
4. Open **Actions** and select **Build WRAITH 0.5 VST3 (Windows x64)**. You can also use **Run workflow**.
5. After a green successful run, open that run and download the artifact **WRAITH-0.5-VST3-Windows-x64**.
6. Extract the artifact and copy the complete `WRAITH.vst3` bundle folder to `C:\Program Files\Common Files\VST3\`.
7. Restart Ableton Live and test MIDI polyphony, audio input capture, Reverse Grains and Freeze Texture.

The first configure fetches JUCE 8.0.6 from the official JUCE repository. No repository secrets are required.

## Local build

Install Visual Studio 2022 with **Desktop development with C++**, CMake 3.22+, and Git. Run `BUILD-WINDOWS.bat` or execute:

```bat
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release --target WRAITH_VST3 --parallel
```

## Important

A successful build is not proof of complete host certification. In particular, test whether your Ableton Live version exposes the plugin's audio input bus. Host routing for a MIDI-capable instrument with audio input may vary. Start with a low master level.
