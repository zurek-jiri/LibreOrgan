# Burton-Berlin, Hill

<!-- Written by LibreOrgan Sampler. To publish it: attach the zip beside this README (it holds the configuration and this README) to the form (a free GitHub account):
     https://github.com/zurek-jiri/LibreOrgan/issues/new?template=organ_configuration.yml -->

A complete LibreOrgan Sampler configuration of Burton-Berlin, Hill, as recorded by Sonus Paradisi in 3 perspectives, Direct, Diffuse and Rear, each on its own audio route: 4 divisions and 48 rows in the Stop List, transmissions and
extensions included (only the rows nested in stop folders are not counted). Whoever owns
the recordings loads the whole instrument in a single step.

**Download:** [Hill, Burton-Berlin, Surround.portable.zip](https://github.com/zurek-jiri/LibreOrgan/raw/main/libreorgans/burton-berlin-hill/Hill,%20Burton-Berlin,%20Surround.portable.zip) - *Load Configuration* in LibreOrgan Sampler opens the zip as it is (0.8.195) and copies the configuration into your LibreOrgans folder; or unpack it and put the `Hill, Burton-Berlin, Surround.portable.json` there yourself.

**No samples are published here.** The configuration merely names the folders it expects;
the recordings themselves are the property of their producer and are obtained from them.

## The configuration

- **Organ:** Burton-Berlin, Hill.
- **Samples:** Sonus Paradisi - [https://www.sonusparadisi.cz/en/organs/england/william-hill-english-organ-model.html](https://www.sonusparadisi.cz/en/organs/england/william-hill-english-organ-model.html); the folders `002261`, `002262`, `002263`, `002264`, `002265` and `002266` (as they come, or in their RAR archives); perspectives Direct, Diffuse and Rear. LibreOrgan Sampler 0.9.28 or later.
- **Expects:** library root of any name, holding the trees `002261/pipes`, `002262/pipes`, `002263/pipes`, `002264/pipes`, `002265/pipes` and `002266/pipes`.
- **Audio layout:** every division has the routes direct, diffuse and rear; auxiliary (unused).
- **RAM:**
  - **4.6 GB** - 16-bit, 32 kHz, releases at half the rate, packed; *estimated*
  - **7.5 GB** - 16-bit, 48 kHz, full releases, packed; *measured*
  - **46 GB** - 32-bit float, the files' own rate, full releases, no packing; *estimated*
- **Saved with:** LibreOrgan Sampler 0.9.28.
- **Author:** Jiri Zurek, 1 November 2026.

## Where to put the file

Put `Hill, Burton-Berlin, Surround.portable.json` into your LibreOrgans folder (Configuration Editor > *LibreOrgans folder*; `Documents\LibreOrgan\LibreOrgans` by default) and load it from there. The sample folders are found in your sample locations (the folders above your sample libraries, remembered by the program and listed in the Configuration Editor). One not found shows "Sample folder not found" under its division: click it and choose any folder of the samples (the tree, a folder above it, a neighbouring tree) - the program finds the tree, the other missing ones around it, and remembers the place for every configuration to come. The folders `002261`, `002262`, `002263`, `002264`, `002265` and `002266` must stand side by side in one folder:

```
<a sample location>\
├── 002261\
│   └── pipes\
├── 002262\
│   └── pipes\
├── 002263\
│   └── pipes\
├── 002264\
│   └── pipes\
├── 002265\
│   └── pipes\
└── 002266\
    └── pipes\
```

## Stop List

| Pedal | I | II | III |
|---|---|---|---|
| PED. Tromba 8' | CH. Orchestral Oboe 8' | GT. Tuba 8' | SW. Clarion 4' |
| PED. Choralflute 4' | CH. Clarinet 8' | GT. Mixture [III] | SW. Oboe 8' |
| PED. Trombone 16' | CH. Tuba 8' (Trsm.) | GT. Trumpet 8' | SW. Cornopean 8' |
| PED. Bassflute 8' | CH. Tierce 1 3/5' | GT. Fifteenth 2' | SW. Double Clarinet 16' |
| PED. Viola 8' | CH. Larigot 1 1/3' | GT. Twelfth 2 2/3' | SW. Mixture 1 3/5' |
| PED. Principal 8' | CH. Piccolo 2' | GT. Harmonic Flute 4' | SW. Fifteenth 2' |
| PED. Echobass 16' (Trsm.) | CH. Nazard 2 2/3' | GT. Principal 4' | SW. Gemshorn 4' |
| PED. Open Diapason 16' | CH. Stopped Flute 4' | GT. Gamba 8' | SW. Suabe Flute 4' |
| PED. Bourdon 16' | CH. Viola d Orchestre 8' | GT. Open Diapason 8' | SW. Vox Angelica 8' |
| PED. Resultantbass 32' | CH. Gedact 8' | GT. Small Open Diapason 8' | SW. Voix Celeste 8' |
| PED. Violone 16' | CH. Aeoline 8' | GT. Bourdon 16' | SW. Open Diapason 8' |
|  |  | GT. Stopped Diapason 8' | SW. Salicional 8' |
|  |  |  | SW. Bourdon 16' |
|  |  |  | SW. Rohrflote 8' |

<!-- published by @zurek-jiri (issue #142, 2026-11-01) -->
