# Chemnitz, Lutherkirche, Sauer Organ

<!-- Written by LibreOrgan Sampler. To publish it: attach the zip beside this README (it holds the configuration and this README) to the form (a free GitHub account):
     https://github.com/zurek-jiri/LibreOrgan/issues/new?template=organ_configuration.yml -->

A complete LibreOrgan Sampler configuration of Chemnitz, Lutherkirche, Sauer Organ, as recorded by Sonus Paradisi in 4 perspectives, Direct, Diffuse, Distant and Rear, each on its own audio route: 4 divisions and 58 rows in the Stop List, transmissions and
extensions included (only the rows nested in stop folders are not counted). Whoever owns
the recordings loads the whole instrument in a single step.

**Download:** [Chemnitz, Lutherkirche, Sauer.portable.zip](https://github.com/zurek-jiri/LibreOrgan/raw/main/libreorgans/chemnitz-lutherkirche-sauer-organ/Chemnitz,%20Lutherkirche,%20Sauer.portable.zip) - *Load Configuration* in LibreOrgan Sampler opens the zip as it is (0.8.195) and copies the configuration into your LibreOrgans folder; or unpack it and put the `Chemnitz, Lutherkirche, Sauer.portable.json` there yourself.

**No samples are published here.** The configuration merely names the folders it expects;
the recordings themselves are the property of their producer and are obtained from them.

## The configuration

- **Organ:** Chemnitz, Lutherkirche, Sauer Organ.
- **Samples:** Sonus Paradisi - [https://www.sonusparadisi.cz/en/organs/germany/chemitz-lutherkirche-sauer-organ.html](https://www.sonusparadisi.cz/en/organs/germany/chemitz-lutherkirche-sauer-organ.html); the folders `002287`, `002288` and `002289` (as they come, or in their RAR archives); perspectives Direct, Diffuse (`04-AB`), Distant and Rear. LibreOrgan Sampler 0.9.65 or later.
- **Expects:** library root of any name, holding the trees `002287/pipes/I`, `002288/pipes/II`, `002288/pipes/III`, `002288/pipes/pedal`, `002289/pipes/II`, `002289/pipes/III` and `002289/pipes/pedal`.
- **Audio layout:** every division has the routes direct, diffuse, distant and rear; auxiliary (unused).
- **RAM:**
  - **8.3 GB** - 16-bit, 32 kHz, releases at half the rate, packed; *estimated*
  - **10 GB** - 16-bit, 48 kHz, releases at half the rate, packed; *measured*
  - **83 GB** - 32-bit float, the files' own rate, full releases, no packing; *estimated*
- **Saved with:** LibreOrgan Sampler 0.9.65.
- **Author:** Jiri Zurek, 5 November 2026.

## Where to put the file

Put `Chemnitz, Lutherkirche, Sauer.portable.json` into your LibreOrgans folder (Configuration Editor > *LibreOrgans folder*; `Documents\LibreOrgan\LibreOrgans` by default) and load it from there. The sample folders are found in your sample locations (the folders above your sample libraries, remembered by the program and listed in the Configuration Editor). One not found shows "Sample folder not found" under its division: click it and choose any folder of the samples (the tree, a folder above it, a neighbouring tree) - the program finds the tree, the other missing ones around it, and remembers the place for every configuration to come. The folders `002287`, `002288` and `002289` must stand side by side in one folder:

```
<a sample location>\
├── 002287\
│   └── pipes\I\
├── 002288\
│   ├── pipes\II\
│   ├── pipes\III\
│   └── pipes\pedal\
└── 002289\
    ├── pipes\II\
    ├── pipes\III\
    └── pipes\pedal\
```

## Stop List

| Pedal | I | II | III |
|---|---|---|---|
| Untersatz 32' | Principal 16' | Bordun 16' | Lieb. Ged. 16' |
| Untersatz 64' (Ext.) | Flute 16' (Ext.) | Principal 8' | Principal 8' |
| Principal 16' | Principal 8' | Schalmey 8' | Liebl. Ged. 8' |
| Subbass 16' | Doppelflote 8' | Konzertflote 8' | Viola 8' |
| Violon 16' | Gamba 8' | Quintaton 8' | Aeolin 8' |
| Dulciana 16' | Flute 8' | Salicional 8' | Soloflote 8' |
| Quintbass 10 2/3' | Gemshorn 8' | Rohrflote 8' | Voix cel. 8' |
| Bassflote 8' | Bourdon 8' | Octave 4' | Fugara 4' |
| Octave 8' | Octave 4' | Traversflote 4' | Fernflote 4' |
| Cello 8' | Rohrflote 4' | Piccolo 2' | Nasard 2 2/3' |
| Octave 4' | Rauschquinte 2 2/3' | Quint 2 2/3' (Ext.) | Flautino 2' |
| Posaune 16' | Octave 2' (Ext.) | Mixtut 4f 2' | Oboe 8' |
| Posaune 32' (Ext.) | Mixtur 3f 2' | Cornet [III] |  |
| Trompete 8' | Cornett 3-4f. 2 2/3' | Clarinette 8' |  |
|  | Fagott 16' | Tuba 8' (Trsm.) |  |
|  | Trompete 8' |  |  |
|  | Trompete 4' (Ext.) |  |  |

<!-- published by @zurek-jiri (issue #220, 2026-11-05) -->
