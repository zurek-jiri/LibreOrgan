# Third-party software in LibreOrgan Sampler

LibreOrgan Sampler itself is free software under the GNU General Public License v3
(see `LICENSE`). It contains or links the following software; the same notices
and the full licence texts are shown in the program under Configuration Editor >
About & licences.

| Component | Version | Licence | Location |
|---|---|---|---|
| JUCE framework (Raw Material Software Limited) | 9.0.3 | GNU AGPLv3 (dual-licensed with the commercial JUCE licence; used here under the AGPLv3) | `JUCE/` (git submodule, tag 9.0.3) |
| Steinberg ASIO SDK (Steinberg Media Technologies GmbH) | 2.3.4 | GNU GPLv3 option of the dual licence (proprietary ASIO licence or GPLv3) | `ASIOSDK/` |
| Signalsmith Stretch (Signalsmith Audio Ltd) | main, 2026 | MIT | `ThirdParty/signalsmith/signalsmith-stretch.h` |
| Signalsmith Linear (Signalsmith Audio Ltd) | main, 2026 | MIT | `ThirdParty/signalsmith/signalsmith-linear/` |
| libarchive (Tim Kientzle and contributors) | 3.8.9 | BSD 2-clause (its COPYING) | `ThirdParty/libarchive/` (git submodule, tag v3.8.9); the Visual Studio build links `ThirdParty/libarchive-prebuilt/win-x64/archive.lib`, built from the same sources with the same options. Reads and unpacks RAR sample archives inside the program. |
| unarr (the unarr project authors: The Unarchiver project, Simon Bünzli, Felix Kauselmann, Bastien Nocera) | 1.2.0 (commit 9c8e898) | GNU LGPLv3 (its COPYING) | `ThirdParty/unarr/` (git submodule), compiled into the program from its RAR sources; the header `ThirdParty/unarr-include/unarr.h` is generated from its `unarr.h.in`. Reads the RAR 4 files libarchive cannot unpack (PPMd blocks, the RAR filters of older WinRAR versions). |

ASIO is a trademark and software of Steinberg Media Technologies GmbH. ASIO
support is compiled into the Windows build only; the macOS and Linux builds use
CoreAudio and ALSA/JACK through JUCE.

No samples are part of this repository or of any release; the sample folders
you play (Sonus Paradisi and others) remain the property of their authors.

The licence texts: `LICENSE` (GPLv3), `ThirdParty/licence-texts/AGPL-3.0.txt`,
`ASIOSDK/LICENSE.txt`, `ThirdParty/signalsmith/LICENSE-signalsmith-stretch.txt`,
`ThirdParty/signalsmith/LICENSE-signalsmith-linear.txt`,
`ThirdParty/notices/libarchive-COPYING.txt`, `ThirdParty/notices/unarr-COPYING.txt`
(LGPLv3), and the notices in `ThirdParty/notices/`.
