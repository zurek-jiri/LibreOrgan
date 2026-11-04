# The Forcalquier Organ Model

<!-- Written by LibreOrgan Sampler. To publish it: attach the zip beside this README (it holds the configuration and this README) to the form (a free GitHub account):
     https://github.com/zurek-jiri/LibreOrgan/issues/new?template=organ_configuration.yml -->

A complete LibreOrgan Sampler configuration of The Forcalquier Organ Model, as recorded by Sonus Paradisi in one perspective, default, each on its own audio route: 4 divisions and 36 rows in the Stop List, transmissions and
extensions included (only the rows nested in stop folders are not counted). Whoever owns
the recordings loads the whole instrument in a single step.

**Download:** [Forcalquier Wet.portable.zip](https://github.com/zurek-jiri/LibreOrgan/raw/main/libreorgans/the-forcalquier-organ-model/Forcalquier%20Wet.portable.zip) - *Load Configuration* in LibreOrgan Sampler opens the zip as it is (0.8.195) and copies the configuration into your LibreOrgans folder; or unpack it and put the `Forcalquier Wet.portable.json` there yourself.

**No samples are published here.** The configuration merely names the folders it expects;
the recordings themselves are the property of their producer and are obtained from them.

## The configuration

- **Organ:** The Forcalquier Organ Model.
- **Samples:** Sonus Paradisi - [https://www.sonusparadisi.cz/en/organs/discontinued-organ-models/forcalquier-france.html](https://www.sonusparadisi.cz/en/organs/discontinued-organ-models/forcalquier-france.html); the folders `000346` and `000347` (as they come, or in their RAR archives); perspectives default. LibreOrgan Sampler 0.9.53 or later.
- **Expects:** library root of any name, holding the trees `000346/pipe` and `000347/pipe`.
- **Audio layout:** every division has the routes default; auxiliary (unused).
- **RAM:**
  - **2.4 GB** - 16-bit, 32 kHz, releases at half the rate, packed; *estimated*
  - **2.8 GB** - 16-bit, 48 kHz, releases at half the rate, packed; *measured*
  - **24 GB** - 32-bit float, the files' own rate, full releases, no packing; *estimated*
- **Saved with:** LibreOrgan Sampler 0.9.53.
- **Author:** Jiri Zurek, 4 November 2026.

## Where to put the file

Put `Forcalquier Wet.portable.json` into your LibreOrgans folder (Configuration Editor > *LibreOrgans folder*; `Documents\LibreOrgan\LibreOrgans` by default) and load it from there. The sample folders are found in your sample locations (the folders above your sample libraries, remembered by the program and listed in the Configuration Editor). One not found shows "Sample folder not found" under its division: click it and choose any folder of the samples (the tree, a folder above it, a neighbouring tree) - the program finds the tree, the other missing ones around it, and remembers the place for every configuration to come. The folders `000346` and `000347` must stand side by side in one folder:

```
<a sample location>\
├── 000346\
│   └── pipe\
└── 000347\
    └── pipe\
```

## Stop List

| Pedal | I | II | III |
|---|---|---|---|
| P. Soubasse 16' | 1. Bourdon 8' | 2. Bourdon 16' | 3. Bourdon 8' |
| P. Flute 16' | 1. Prestant 4' | 2. Gambe 8' | 3. Flute 4' |
| P. Flute 8' | 1. Flute 4' | 2. Bourdon 8' | 3. Cornet 2 2/3' |
| P. Flute 4' | 1. Nazard 2 2/3' | 2. Flute 8' | 3. Voix humaine 8' |
| P. Plein Jeu 2 2/3' | 1. Doublette 2' | 2. Prestant 4' | 3. Hautbois 8' |
| P. Bombarde 16' | 1. Quarte 2' | 2. Flute 4' |  |
| P. Trompette 8' | 1. Tierce 1 3/5' | 2. Doublette 2' |  |
| P. Clairon 4' | 1. Larigot 1 1/3' | 2. Sifflet 1' |  |
|  | 1. Plein Jeu 1' | 2. Grand Cornet 8' |  |
|  | 1. Cromorne 8' | 2. Plein Jeu 1' |  |
|  |  | 2. Trompette 1ere 8' |  |
|  |  | 2. Trompette 2eme 8' |  |
|  |  | 2. Clairon 4' |  |

<!-- published by @zurek-jiri (issue #203, 2026-11-04) -->
