# Doksy-Kruh Renaissance Organ

<!-- Written by LibreOrgan Sampler. To publish it: attach the zip beside this README (it holds the configuration and this README) to the form (a free GitHub account):
     https://github.com/zurek-jiri/LibreOrgan/issues/new?template=organ_configuration.yml -->

A complete LibreOrgan Sampler configuration of Doksy-Kruh Renaissance Organ, as recorded by Sonus Paradisi in one perspective, default, each on its own audio route: 2 divisions and 11 rows in the Stop List, transmissions and
extensions included (only the rows nested in stop folders are not counted). Whoever owns
the recordings loads the whole instrument in a single step.

The organ was recorded for HW1. It has no native multiple releases. Therefore, it is recommended to use it with an appropriate IR. This configuration was created using the IR freely available in the "Rural Organ" sample set downloadable from gitHub.

**Download:** [Doksy-Kruh, Renaissance Organ.portable.zip](https://github.com/zurek-jiri/LibreOrgan/raw/main/libreorgans/doksy-kruh-renaissance-organ/Doksy-Kruh,%20Renaissance%20Organ.portable.zip) - *Load Configuration* in LibreOrgan Sampler opens the zip as it is (0.8.195) and copies the configuration into your LibreOrgans folder; or unpack it and put the `Doksy-Kruh, Renaissance Organ.portable.json` there yourself.

**No samples are published here.** The configuration merely names the folders it expects;
the recordings themselves are the property of their producer and are obtained from them.

## The configuration

- **Organ:** Doksy-Kruh Renaissance Organ.
- **Samples:** Sonus Paradisi - [https://www.sonusparadisi.cz/en/organs/discontinued-organ-models/doky-kruh.html](https://www.sonusparadisi.cz/en/organs/discontinued-organ-models/doky-kruh.html); the folders `000333` and `000334` (as they come, or in their RAR archives); perspectives default. LibreOrgan Sampler 0.9.52 or later.
- **Expects:** library root of any name, holding the trees `000333/pipe` and `000334/pipe`.
- **Audio layout:** Pedal: the routes default; auxiliary (unused); I: the routes auxiliary and default.
- **RAM:**
  - **1.0 GB** - 16-bit, 32 kHz, releases at half the rate, packed; *estimated*
  - **1.0 GB** - 16-bit, 48 kHz, releases at half the rate, packed; *measured*
  - **9.7 GB** - 32-bit float, the files' own rate, full releases, no packing; *estimated*
- **Saved with:** LibreOrgan Sampler 0.9.52.
- **Author:** Jiri Zurek, 4 November 2026.

## Where to put the file

Put `Doksy-Kruh, Renaissance Organ.portable.json` into your LibreOrgans folder (Configuration Editor > *LibreOrgans folder*; `Documents\LibreOrgan\LibreOrgans` by default) and load it from there. The sample folders are found in your sample locations (the folders above your sample libraries, remembered by the program and listed in the Configuration Editor). One not found shows "Sample folder not found" under its division: click it and choose any folder of the samples (the tree, a folder above it, a neighbouring tree) - the program finds the tree, the other missing ones around it, and remembers the place for every configuration to come. The folders `000333` and `000334` must stand side by side in one folder:

```
<a sample location>\
├── 000333\
│   └── pipe\
└── 000334\
    └── pipe\
```

## Stop List

| Pedal | I |
|---|---|
| P. Sedecima 1' | I. Regal 8' |
| P. BauerFlet 2' | I. Mixtur 3f. 1' |
|  | I. Principal 4' |
|  | I. Quinta 2 2/3' |
|  | I. Quinta 1 1/3' |
|  | I. SpitzFlet 4' |
|  | I. Copl 8' |
|  | I. Octava 2' |
|  | I. Nassat 2 2/3' |

<!-- published by @zurek-jiri (issue #197, 2026-11-04) -->
