# OrganSampler

**Pipe organ sampler for creative minds.** Design your console, build your stop
list, perform.

**[⬇ RELEASES - download the program](https://github.com/zurek-jiri/OrganSampler/releases) &nbsp;|&nbsp; [⬇ Rural Organ demo](https://github.com/zurek-jiri/OrganSampler/releases/tag/rural-organ-0.8) &nbsp;|&nbsp; [Configurations, Organs](configurations/INDEX.md)**

[![Latest build](https://img.shields.io/github/v/tag/zurek-jiri/OrganSampler?sort=semver&label=download%20the%20latest%20build)](https://github.com/zurek-jiri/OrganSampler/releases)

[![The console with the Stralsund, St. Jakobi organ: four divisions with their stop buttons, stop folders, extensions and transmissions, the voicing bars under the stops, couplers and tremulants in the headers, the swell shutters at different positions, a combination engaged, the crescendo and the polyphony display above](site/assets/main.jpg)](site/assets/main.jpg)

OrganSampler plays sampled pipe organs from plain folders of WAV files - one
folder per stop, its pipes as WAV files - and turns any number of such folders into
an organ with up to seven manuals and a pedal: massive polyphony, per-key
voicing, divisions and microphone perspectives with independent audio routes on
any number of sound cards, IR reverb per route, wind and tremulant models,
temperaments, transmissions with transposition (extensions), velocity layers,
general crescendo and combinations. No organ definition file is needed: you load
the folders, the program proposes the organ, you shape it and save it as a
configuration. Free software under the GNU GPL v3. Windows, macOS and Linux.

**Your own organ, from any sources.** This is what sets OrganSampler apart:
mix stops from different sample folders and different producers into one
console, duplicate a stop as a transmission or an extension (a 16' from an
8', a mutation from a principal), merge two configurations into one organ,
move stops between divisions, and a Hauptwerk organ definition opens as a
configuration too.

**Light on RAM, CPU and disk.** On a modest computer the saving options of
the load box squeeze an 80 GB sample set into 8 GB of RAM (the table below),
and the samples are read straight from their packed RAR archives - nothing to
unpack, no second copy on the disk. On a strong computer the same program
reaches up to 500 000 pipes of polyphony; it measures what your machine can do
and sets its limit itself.

**One organ, eight ways to load it: from 83 GB down to 8 GB.** Measured on
Stralsund, St. Jakobi, Wegscheider organ (Sonus Paradisi), three
perspectives, 53.9 GB of 32-bit float WAV files at 48 kHz - the whole program
as loaded, the filled pipes' copies included (OrganSampler 0.8.184, 24 October
2026). The same organ, the same sound to any normal ear, one tenth of the
memory with every saving in force:

| Storage | Rate on loading | Releases | Packing in RAM | RAM |
|---|---|---|---|---|
| 32-bit float | 48 kHz (as the files) | full rate | off | **83 GB** |
| 32-bit float | 48 kHz | full rate | on | 29 GB |
| 16-bit | 48 kHz | full rate | off | 49 GB |
| 16-bit | 48 kHz | full rate | on | 15 GB |
| 16-bit | 48 kHz | half the rate | off | 32 GB |
| 16-bit | 48 kHz | half the rate | on | 12 GB |
| 16-bit | 32 kHz | half the rate | on | **8 GB** |

The packing is lossless for 16-bit samples and for 24-bit files; a 32-bit
float file (as this organ's) is rounded to 24 bits first, half a converter
step. Every load box offers these choices; the choice travels with the organ.

## Try it in five minutes

1. **The program:** the latest build on the
   [releases page](https://github.com/zurek-jiri/OrganSampler/releases)
   (pre-release builds for testers; one executable, no installation - unpack
   the archive of your system: `OrganSampler-windows.zip` for Windows 10/11
   64-bit; `OrganSampler-macos.zip` for macOS 11 Big Sur or newer on Apple
   silicon, `OrganSampler-macos-10.11-intel.zip` for 10.11 El Capitan or newer
   on Intel Macs; `OrganSampler-linux-x86_64.tar.gz` for Linux x86-64 with
   glibc 2.38 or newer - Ubuntu 24.04, Mint 22, Debian 13, Fedora 39 and newer).
2. **An organ to play:** download
   [**RuralOrgan.rar**](https://github.com/zurek-jiri/OrganSampler/releases/download/rural-organ-0.8/RuralOrgan.rar)
   (about 700 MB) - the **Rural Organ** demo: a small village organ, three
   divisions, 24 stops, **dry recordings with IR reverb**, with its
   sample folders and a portable configuration. Start OrganSampler,
   *Load Configuration*, choose `RuralOrgan.rar` itself - the samples are
   read from the archive (or unpack it and choose `Rural Organ.json`). It plays
   through your default sound device at once; a message says what to set
   up next. Details: [`configurations/rural-organ`](configurations/rural-organ/README.md).
3. **More organs:** OrganSampler ships no other samples. Sample folders in
   wave format from [**Sonus Paradisi**](https://www.sonusparadisi.cz/en/organs.html)
   work with OrganSampler as they are, straight from their RAR archives too - and their
   [Free stuff](https://www.sonusparadisi.cz/en/free-stuff.html) page has
   free instruments and impulse responses. Ready-made configurations for
   them: [`configurations/INDEX.md`](configurations/INDEX.md) (how to use and
   publish one: [`configurations/`](configurations/README.md)).

## The program at work

The program at work on the Stralsund organ, with sound - the organ loaded, a missing sample folder found with one click, MIDI learn, stops drawn and played, the voicing row with the per-pipe sliders, arrange mode by a two-second press, a stop folder enriched by dragging, a transmission dragged into the pedal (right mouse button or Shift) and transposed, couplers, a combination, the swell shutters, the tremulant, the crescendo, temperaments, the Stop List. Every frame and every note come from the program itself ([full size on the website](https://zurek-jiri.github.io/OrganSampler/)):

https://github.com/user-attachments/assets/24b5890e-af7d-4d8e-b8a5-da2631e3529b

## Links

- Website: https://zurek-jiri.github.io/OrganSampler/
- Downloads: https://github.com/zurek-jiri/OrganSampler/releases
- User manual: [`OrganSampler/USER_MANUAL.md`](OrganSampler/USER_MANUAL.md)
  (also inside the program: Configuration Editor > About & licences)
- Changes: [`OrganSampler/CHANGELOG.md`](OrganSampler/CHANGELOG.md)
- Configurations, Organs - ready-made organs to download and share: the list
  [`configurations/INDEX.md`](configurations/INDEX.md), how it works
  [`configurations/`](configurations/README.md)
- Problems and ideas: *Configuration Editor > Report a problem...* in the
  program bundles the logs and opens the
  [bug report](https://github.com/zurek-jiri/OrganSampler/issues/new/choose);
  see [`CONTRIBUTING.md`](CONTRIBUTING.md)
- Third-party software and licences: [`THIRD-PARTY-NOTICES.md`](THIRD-PARTY-NOTICES.md)

The executable is portable: every data table and every licence text is embedded
at build time, nothing is read beside the executable. Settings live in the
user's application-data folder (`%APPDATA%\OrganSampler` on Windows,
`~/Library/Application Support/OrganSampler` on macOS, `~/.config/OrganSampler`
on Linux).

## Building

Everything the build needs is in the repository (JUCE as a git submodule, the
Steinberg ASIO SDK under its GPLv3 option, Signalsmith Stretch); nothing refers
to a path outside it.

```
git clone --recurse-submodules https://github.com/zurek-jiri/OrganSampler.git
cd OrganSampler
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

The JUCE submodule is pinned to JUCE 9.0.3. JUCE 9 needs CMake 3.22 or newer,
and on Linux the XInput2 and EGL development packages in addition to the usual
ones.

The result is `build/OrganSampler_artefacts/Release/OrganSampler.exe`
(Windows), `OrganSampler.app` (macOS) or `OrganSampler` (Linux).

Linux needs the usual JUCE development packages (ALSA, JACK, X11, FreeType,
fontconfig, Mesa; the list is in `.github/workflows/build.yml`). Audio on macOS
is CoreAudio, on Linux ALSA or JACK; ASIO, Windows Audio and the aggregated
Windows Audio device exist in the Windows build only.

Windows developers may also use the Projucer project
`OrganSampler/OrganSampler.jucer` (Visual Studio 2026 exporter under
`OrganSampler/Builds`). Its paths are relative to the repository as well.

### Continuous integration

`.github/workflows/build.yml` builds Release on Windows, macOS and Linux for
every push and pull request, runs the engine self-test on synthetic sample
folders (Windows and Linux) and uploads the packaged builds as artifacts - the
native archive of each system: a zip with `OrganSampler.exe` on Windows, a zip
of the app bundle on macOS, a tar.gz on Linux (it keeps the executable bit).
A tag `v*` also attaches them to a GitHub release. The macOS and Linux builds
are not signed: macOS asks for confirmation the first time (right-click > Open).

## Self-test

```
OrganSampler --selftest "<division sample folder>"
```

Loads the folder without the GUI, exercises the engine (kernels, transmissions,
filled pipes, velocity layers, pitch-shifted copies, tutti stress, recording)
and writes `selftest.txt` into the application-data folder.

## Samples

No samples are part of this repository or of any release: they are bought or
downloaded elsewhere - from [Sonus Paradisi](https://www.sonusparadisi.cz/)
above all - and remain the property of their authors. OrganSampler plays
plain folders of WAV files: one folder per stop, named after the stop, with
the WAVs of its pipes named after their keys (`060-C4.wav`; several files for
one key play in turn; velocity layers `060-C4_070.wav`; releases in
`rel`, `rel500` ... subfolders); loop points and pitch belong ideally in the
WAV's `smpl` chunk, but the program copes without them. See "How the sample
folders must be organised" in the user
manual, and the website for where the samples come from.
