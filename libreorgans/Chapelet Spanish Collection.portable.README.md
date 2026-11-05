# Chapelet Spanish Collection

<!-- Written by LibreOrgan Sampler. To publish it: attach the zip beside this README (it holds the configuration and this README) to the form (a free GitHub account):
     https://github.com/zurek-jiri/LibreOrgan/issues/new?template=organ_configuration.yml -->

A complete LibreOrgan Sampler configuration of Chapelet Spanish Collection, as recorded by Sonus Paradisi in 2 perspectives, Diffuse and Direct, each on its own audio route: 4 divisions and 36 rows in the Stop List, transmissions and
extensions included (only the rows nested in stop folders are not counted). Whoever owns
the recordings loads the whole instrument in a single step.

**Download:** [Chapelet Spanish Collection.portable.zip](https://github.com/zurek-jiri/LibreOrgan/raw/main/libreorgans/chapelet-spanish-collection/Chapelet%20Spanish%20Collection.portable.zip) - *Load Configuration* in LibreOrgan Sampler opens the zip as it is (0.8.195) and copies the configuration into your LibreOrgans folder; or unpack it and put the `Chapelet Spanish Collection.portable.json` there yourself.

**No samples are published here.** The configuration merely names the folders it expects;
the recordings themselves are the property of their producer and are obtained from them.

## The configuration

- **Organ:** Chapelet Spanish Collection.
- **Samples:** Sonus Paradisi - [https://www.sonusparadisi.cz/en/organs/spain/chapelet-spanish-collection.html](https://www.sonusparadisi.cz/en/organs/spain/chapelet-spanish-collection.html); the folders `002290` and `002291` (as they come, or in their RAR archives); perspectives Diffuse and Direct (`03-ORTF`). LibreOrgan Sampler 0.9.64 or later.
- **Expects:** library root of any name, holding the trees `002290/pipes/Spanel_Old`, `002291/pipes/GO`, `002291/pipes/pedal` and `002291/pipes/positif`.
- **Audio layout:** every division has the routes diffuse and direct; auxiliary (unused).
- **RAM:**
  - **2.3 GB** - 16-bit, 32 kHz, releases at half the rate, packed; *estimated*
  - **2.8 GB** - 16-bit, 48 kHz, releases at half the rate, packed; *measured*
  - **23 GB** - 32-bit float, the files' own rate, full releases, no packing; *estimated*
- **Saved with:** LibreOrgan Sampler 0.9.64.
- **Author:** Jiri Zurek, 5 November 2026.

## Where to put the file

Put `Chapelet Spanish Collection.portable.json` into your LibreOrgans folder (Configuration Editor > *LibreOrgans folder*; `Documents\LibreOrgan\LibreOrgans` by default) and load it from there. The sample folders are found in your sample locations (the folders above your sample libraries, remembered by the program and listed in the Configuration Editor). One not found shows "Sample folder not found" under its division: click it and choose any folder of the samples (the tree, a folder above it, a neighbouring tree) - the program finds the tree, the other missing ones around it, and remembers the place for every configuration to come. The folders `002290` and `002291` must stand side by side in one folder:

```
<a sample location>\
├── 002290\
│   └── pipes\Spanel_Old\
└── 002291\
    ├── pipes\GO\
    ├── pipes\pedal\
    └── pipes\positif\
```

## Stop List

| Pedal | I | II | III |
|---|---|---|---|
| Ped. Contras 16' | I. Violon 8' | II. Flautado 8' | III. Flautado bajos 8' |
| Ped. Contras 8' | I. Tapadillo 4' | II. Octava 4' | III. Flautado disc. 8' (Ext.) |
| Ped. Oct 4' | I. Flautin 2' | II. Docena 2 2/3' | III. Octava bajos 4' |
| Ped. Corno 2' | I. Nasardo 1 1/3' | II. Quincena 2' | III. Octava disc. 4' |
|  | I. Cimbala 2/3' | II. Nasardo 1 3/5' | III. Docena bajos 2 2/3' |
|  | I. Viejas 8' | II. Lleno 1' | III. Docena disc. 2 2/3' (Ext.) |
|  |  | II. Trompeta 8' | III. Quincena bajos 2' |
|  |  | II. Clarin 4' | III. Quincena disc. 2' (Ext.) |
|  |  |  | III. Decinovena bajos 1 1/3' |
|  |  |  | III. Decinovena disc. 1 1/3' (Ext.) |
|  |  |  | III. Lleno bajos 1' |
|  |  |  | III. Lleno disc. 1' (Ext.) |
|  |  |  | III. Trompeta bajos 8' |
|  |  |  | III. Corneta disc. 8' |
|  |  |  | III. Bajoncillo bajos 4' |
|  |  |  | III. Clarin disc. 8' (Ext.) |
|  |  |  | III. Dulziana bajos 8' |
|  |  |  | III. Dulziana disc. 8' (Ext.) |

<!-- published by @zurek-jiri (issue #218, 2026-11-05) -->
