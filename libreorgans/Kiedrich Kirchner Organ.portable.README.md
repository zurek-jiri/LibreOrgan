# Kiedrich, Kirchner Organ

<!-- Written by LibreOrgan Sampler. To publish it: attach the zip beside this README (it holds the configuration and this README) to the form (a free GitHub account):
     https://github.com/zurek-jiri/LibreOrgan/issues/new?template=organ_configuration.yml -->

A complete LibreOrgan Sampler configuration of Kiedrich, Kirchner Organ, as recorded by Sonus Paradisi in 3 perspectives, Direct, Diffuse and Rear, each on its own audio route: 3 divisions and 22 rows in the Stop List, transmissions and
extensions included (only the rows nested in stop folders are not counted). Whoever owns
the recordings loads the whole instrument in a single step.

**Download:** [Kiedrich Kirchner Organ.portable.zip](https://github.com/zurek-jiri/LibreOrgan/raw/main/libreorgans/kiedrich-kirchner-organ/Kiedrich%20Kirchner%20Organ.portable.zip) - *Load Configuration* in LibreOrgan Sampler opens the zip as it is and copies the configuration into your LibreOrgans folder; or unpack it and put the `Kiedrich Kirchner Organ.portable.json` there yourself.

**No samples are published here.** The configuration merely names the folders it expects;
the recordings themselves are the property of their producer and are obtained from them.

## The configuration

- **Organ:** Kiedrich, Kirchner Organ.
- **Samples:** Sonus Paradisi - [https://www.sonusparadisi.cz/en/organs/germany/kiedrich-virtual-organ-model.html](https://www.sonusparadisi.cz/en/organs/germany/kiedrich-virtual-organ-model.html); the folders `001841`, `001842`, `001843` and `001844` (as they come, or in their RAR archives); perspectives Direct (`02-Dry`), Diffuse (`04-AB`) and Rear (`rea22`). LibreOrgan Sampler 0.9.152 or later.
- **Expects:** library root of any name, holding the trees `001841/pipes/ab`, `001841/pipes/ort`, `001841/pipes/rea`, `001842/pipes/ab`, `001843/pipes/ort` and `001844/pipes/rea`.
- **Audio layout:** every division has the routes direct, diffuse, rear and noises; auxiliary (unused).
- **RAM:**
  - **3.5 GB** - 16-bit, 32 kHz, releases at half the rate, packed; *estimated*
  - **31 GB** - 32-bit float, 48 kHz, full releases, packed only as far as the memory asked; *measured*
- **Saved with:** LibreOrgan Sampler 0.9.152.
- **Author:** Jiri Zurek, 28 November 2026.

## Where to put the file

Put `Kiedrich Kirchner Organ.portable.json` into your LibreOrgans folder (Configuration Editor > *LibreOrgans folder*; `Documents\LibreOrgan\LibreOrgans` by default) and load it from there. The sample folders are found in your sample locations (the folders above your sample libraries, remembered by the program and listed in the Configuration Editor). One not found shows "Sample folder not found" under its division: click it and choose any folder of the samples (the tree, a folder above it, a neighbouring tree) - the program finds the tree, the other missing ones around it, and remembers the place for every configuration to come. The folders `001841`, `001842`, `001843` and `001844` must stand side by side in one folder:

```
<a sample location>\
├── 001841\
│   ├── pipes\ab\
│   ├── pipes\ort\
│   └── pipes\rea\
├── 001842\
│   └── pipes\ab\
├── 001843\
│   └── pipes\ort\
└── 001844\
    └── pipes\rea\
```

## Stop List

| Pedal | I | II |
|---|---|---|
| SubBass 16' | Gedact 8' | Princip. 8' |
| Dopp. Quint 5 1/3' | Floete 4' | Gr. Ged. 16' |
| Princip. 8' | Octav 4' | Oct. 4' |
| Quint 2 2/3' | WaldFloete 2' | Floet. Ged. 4' |
| Octav 4' | Quint 1 1/3' | Sup. Oct. 2' |
| Mixtur 4fach 2' | Sup. Octa. 1' | Quint 2 2/3' |
| Sup. Octav 2' |  | Mixtur 4fach 1 1/3' |
| Calcant |  | Cymb. 2fach 1/2' |

<!-- published by @zurekjiri (issue #358, 2026-11-28) -->
