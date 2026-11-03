# Cembalo Ruckers Model

<!-- Written by LibreOrgan Sampler. To publish it: attach the zip beside this README (it holds the configuration and this README) to the form (a free GitHub account):
     https://github.com/zurek-jiri/LibreOrgan/issues/new?template=organ_configuration.yml -->

A complete LibreOrgan Sampler configuration of Cembalo Ruckers Model, as recorded by Sonus Paradisi in one perspective, default, each on its own audio route: 3 divisions and 4 rows in the Stop List, transmissions and
extensions included (only the rows nested in stop folders are not counted). Whoever owns
the recordings loads the whole instrument in a single step.

**Download:** [Cembalo Ruckers XY.portable.zip](https://github.com/zurek-jiri/LibreOrgan/raw/main/libreorgans/cembalo-ruckers-model/Cembalo%20Ruckers%20XY.portable.zip) - *Load Configuration* in LibreOrgan Sampler opens the zip as it is (0.8.195) and copies the configuration into your LibreOrgans folder; or unpack it and put the `Cembalo Ruckers XY.portable.json` there yourself.

**No samples are published here.** The configuration merely names the folders it expects;
the recordings themselves are the property of their producer and are obtained from them.

## The configuration

- **Organ:** Cembalo Ruckers Model.
- **Samples:** Sonus Paradisi - [https://www.sonusparadisi.cz/en/organs/harpsichords-etc/cembalo-ruckers-model.html](https://www.sonusparadisi.cz/en/organs/harpsichords-etc/cembalo-ruckers-model.html); the folders `000876` and `000877` (as they come, or in their RAR archives); perspectives default. LibreOrgan Sampler 0.9.48 or later.
- **Expects:** library root of any name, holding the trees `000876/pipe` and `000877/pipe/Cembalo_XY`.
- **Audio layout:** Pedal: no routeauxiliary (unused); I: the routes default; auxiliary (unused); II: the routes default; auxiliary (unused).
- **RAM:**
  - **1.0 GB** - 16-bit, 32 kHz, releases at half the rate, packed; *estimated*
  - **2.6 GB** - 16-bit, 48 kHz, full releases, no packing; *measured*
  - **10 GB** - 32-bit float, the files' own rate, full releases, no packing; *estimated*
- **Saved with:** LibreOrgan Sampler 0.9.48.
- **Author:** Jiri Zurek, 3 November 2026.

## Where to put the file

Put `Cembalo Ruckers XY.portable.json` into your LibreOrgans folder (Configuration Editor > *LibreOrgans folder*; `Documents\LibreOrgan\LibreOrgans` by default) and load it from there. The sample folders are found in your sample locations (the folders above your sample libraries, remembered by the program and listed in the Configuration Editor). One not found shows "Sample folder not found" under its division: click it and choose any folder of the samples (the tree, a folder above it, a neighbouring tree) - the program finds the tree, the other missing ones around it, and remembers the place for every configuration to come. The folders `000876` and `000877` must stand side by side in one folder:

```
<a sample location>\
├── 000876\
│   └── pipe\
└── 000877\
    └── pipe\Cembalo_XY\
```

## Stop List

| I | II |
|---|---|
| Unter 8' | Ober 8' |
| Unter 4' | Ober Laute 8' |

<!-- published by @zurek-jiri (issue #191, 2026-11-03) -->
