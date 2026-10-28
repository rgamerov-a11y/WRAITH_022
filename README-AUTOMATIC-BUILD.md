# WRAITH — automatic Windows VST3 build

This package adds a GitHub Actions workflow to the WRAITH source project. It builds the VST3 on a Windows x64 runner and publishes a ZIP artifact containing the `WRAITH.vst3` bundle.

## Easiest route: build in GitHub Actions

1. Sign in to GitHub and create a **new private repository** (recommended if you want to keep the project private).
2. Extract this ZIP on your computer.
3. Upload the **contents** of the extracted folder to the repository. Make sure `.github/workflows/build-windows.yml` is included; on Windows, enable viewing hidden files if needed.
4. Open the repository's **Actions** tab and select **Build WRAITH VST3 (Windows x64)**.
5. Click **Run workflow**. The workflow fetches JUCE from its official GitHub repository, configures CMake, and builds the Release VST3.
6. When the run completes successfully, open that run and download the artifact named **WRAITH-VST3-Windows-x64**.
7. Extract the downloaded ZIP. It contains a `WRAITH.vst3` folder/bundle. Copy that entire folder to:
   `C:\Program Files\Common Files\VST3\`
8. Restart Ableton Live and rescan plug-ins if necessary.

The workflow also runs automatically on pushes and pull requests targeting `main` or `master`. It requires no repository secrets.

## Build locally instead

Install Visual Studio 2022 with **Desktop development with C++**, CMake 3.22+, and Git. Open **x64 Native Tools Command Prompt for VS 2022** in the extracted project folder and run `BUILD-WINDOWS.bat`. The first configure needs internet access to fetch JUCE.

## Important limitations

- A successful CI build confirms compilation and packaging, not a full audio-quality or Ableton-host certification. Load the plugin in Ableton and test MIDI input, chords, sustain, and parameters.
- This is still WRAITH prototype 0.2: a polyphonic additive/inharmonic resonator prototype. Granular playback, reverse grains, and shimmer pitch shifting are not implemented yet.
- GitHub Actions builds Windows x64. The workflow does not sign the plugin; Windows may show a security warning for downloaded files.
