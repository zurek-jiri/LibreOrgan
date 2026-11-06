# Adriatic Organ Model

<!-- Written by LibreOrgan Sampler. To publish it: attach the zip beside this README (it holds the configuration and this README) to the form (a free GitHub account):
     https://github.com/zurek-jiri/LibreOrgan/issues/new?template=organ_configuration.yml -->

A complete LibreOrgan Sampler configuration of Adriatic Organ Model, as recorded by Sonus Paradisi in 2 perspectives, Rear and default, each on its own audio route: 2 divisions and 10 rows in the Stop List, transmissions and
extensions included (only the rows nested in stop folders are not counted). Whoever owns
the recordings loads the whole instrument in a single step.

**Download:** [Adriatic Organ Model.portable.zip](https://github.com/zurek-jiri/LibreOrgan/raw/main/libreorgans/adriatic-organ-model/Adriatic%20Organ%20Model.portable.zip) - *Load Configuration* in LibreOrgan Sampler opens the zip as it is (0.8.195) and copies the configuration into your LibreOrgans folder; or unpack it and put the `Adriatic Organ Model.portable.json` there yourself.

**No samples are published here.** The configuration merely names the folders it expects;
the recordings themselves are the property of their producer and are obtained from them.

## The configuration

- **Organ:** Adriatic Organ Model.
- **Samples:** Sonus Paradisi - [https://www.sonusparadisi.cz/en/organs/italy/adriatic-organ-model.html](https://www.sonusparadisi.cz/en/organs/italy/adriatic-organ-model.html); the folders `000822`, `000823`, `000825` and `000826` (as they come, or in their RAR archives); perspectives Rear (`06_rea`) and default. LibreOrgan Sampler 0.9.66 or later.
- **Expects:** library root of any name, holding the trees `000822/pipe`, `000823/pipe`, `000825/pipe` and `000826/pipe`.
- **Audio layout:** Pedal: the routes default; auxiliary (unused); I: the routes rear and default; auxiliary (unused).
- **RAM:**
  - **1.5 GB** - 16-bit, 32 kHz, releases at half the rate, packed; *estimated*
  - **1.7 GB** - 16-bit, 48 kHz, releases at half the rate, packed; *measured*
  - **15 GB** - 32-bit float, the files' own rate, full releases, no packing; *estimated*
- **Saved with:** LibreOrgan Sampler 0.9.66.
- **Author:** Jiri Zurek, 6 November 2026.

## Where to put the file

Put `Adriatic Organ Model.portable.json` into your LibreOrgans folder (Configuration Editor > *LibreOrgans folder*; `Documents\LibreOrgan\LibreOrgans` by default) and load it from there. The sample folders are found in your sample locations (the folders above your sample libraries, remembered by the program and listed in the Configuration Editor). One not found shows "Sample folder not found" under its division: click it and choose any folder of the samples (the tree, a folder above it, a neighbouring tree) - the program finds the tree, the other missing ones around it, and remembers the place for every configuration to come. The folders `000822`, `000823`, `000825` and `000826` must stand side by side in one folder:

```
<a sample location>\
├── 000822\
│   └── pipe\
├── 000823\
│   └── pipe\
├── 000825\
│   └── pipe\
└── 000826\
    └── pipe\
```

## Stop List

| I |
|---|
| 01 Principale bassi 8' |
| 02 Principale soprani 8' (Ext.) |
| 03 Ottava 4' |
| 04 Quintadecima 2' |
| 05 Decimanona 1 1/3' |
| 06 Vigesima seconda 1' |
| 07 Flauto in VIII soprani 4' (Ext.) |
| 08 Flauto in VIII bassi 4' |
| 09 Tromboncini bassi 8' |
| 10 Tromboncini soprani 8' (Ext.) |

<!-- published by @zurek-jiri (issue #224, 2026-11-06) -->
