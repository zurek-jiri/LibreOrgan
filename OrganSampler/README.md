# LibreOrgan Sampler

A pipe-organ sampler built on JUCE 9: massive polyphony, per-key voicing,
multiple divisions and perspectives with independent audio routes, IR reverb
per route, wind and tremulant models, temperaments, transmissions with
transposition (extensions), velocity layers, general crescendo and
combinations. Free software under the GNU GPL v3 (see the `LICENSE` and
`THIRD-PARTY-NOTICES.md` in the repository root, or *About & licences...* in
the program's Configuration Editor). The executable is fully portable: every
data table and licence text is embedded at build time.

- Version: see `CHANGELOG.md` (semantic versioning; the window title shows it).
- User documentation: `USER_MANUAL.md`.

## Building

The portable build for every platform is the `CMakeLists.txt` in the
repository root (see the root `README.md`):

```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

Windows developers may use the Projucer project `LibreOrgan.jucer` instead
(Visual Studio 2026 exporter). Its JUCE module path is `../JUCE/modules`, the
header search paths point at `../ASIOSDK` and `../ThirdParty/signalsmith`
relative to the build folder; nothing outside the repository is needed.

```
MSBuild Builds\VisualStudio2026\LibreOrgan_App.vcxproj /p:Configuration=Release /p:Platform=x64
```

The pre-build step runs `Source/GenerateTemperamentsData.ps1`, which embeds
`Source/temperaments.csv` into `Source/TemperamentsData.h` and (through
`Source/GenerateLicencesData.ps1`) the licence texts and notices of
`ThirdParty/` into `Source/LicencesData.h`. Both generated headers are
committed, so builds without PowerShell (macOS, Linux) use them as they are;
run the scripts after changing `temperaments.csv` or a notice. The result is
`Builds\VisualStudio2026\x64\Release\App\LibreOrgan.exe`.

Audio back-ends: ASIO (Steinberg ASIO SDK 2.3.4 under its GPLv3 option),
Windows Audio and the aggregated Windows Audio device are compiled on Windows
only; macOS uses CoreAudio, Linux ALSA/JACK.

## Self-test

```
LibreOrgan Sampler --selftest "<division sample folder>"
```

Loads the folder, exercises the engine (kernels, transmissions, filled pipes,
velocity layers, pitch-shifted copies, tutti stress, recording) and writes
`selftest.txt` into the application-data folder (`%APPDATA%\LibreOrgan` on
Windows). Environment variables such as `ORGAN_SELFTEST_IR`,
`ORGAN_SELFTEST_BLOCK` and `ORGAN_NO_TONEFILTER` vary the test; see
`Source/EngineSelfTest.h`. GUI checks driven from PowerShell live in
`Tests/gui/`.

## Releasing a version

1. Raise the version in `LibreOrgan.jucer`, `JuceLibraryCode/JuceHeader.h`,
   `Builds/VisualStudio2026/resources.rc`, the `JUCE_APP_VERSION` defines in
   `LibreOrgan_App.vcxproj` and `project(LibreOrgan Sampler VERSION ...)` in the
   root `CMakeLists.txt` (all together).
2. Add the entry to `CHANGELOG.md`.
3. Push a tag `v<version>`: the GitHub workflow builds Windows, macOS and
   Linux, runs the self-test and attaches the executables to the release.
