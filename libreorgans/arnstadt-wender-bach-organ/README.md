# Arnstadt, Wender (Bach Organ)

<!-- Written by LibreOrgan Sampler. To publish it: attach the zip beside this README (it holds the configuration and this README) to the form (a free GitHub account):
     https://github.com/zurek-jiri/LibreOrgan/issues/new?template=organ_configuration.yml -->

A complete LibreOrgan Sampler configuration of Arnstadt, Wender (Bach Organ), as recorded by Sonus Paradisi in 4 perspectives, Diffuse, Direct, Distant and Rear, each on its own audio route: 3 divisions and 21 rows in the Stop List, transmissions and
extensions included (only the rows nested in stop folders are not counted). Whoever owns
the recordings loads the whole instrument in a single step.

**Download:** [Arnstadt, Wender (Bach Organ).portable.zip](https://github.com/zurek-jiri/LibreOrgan/raw/main/libreorgans/arnstadt-wender-bach-organ/Arnstadt,%20Wender%20(Bach%20Organ).portable.zip) - *Load Configuration* in LibreOrgan Sampler opens the zip as it is (0.8.195) and copies the configuration into your LibreOrgans folder; or unpack it and put the `Arnstadt, Wender (Bach Organ).portable.json` there yourself.

**No samples are published here.** The configuration merely names the folders it expects;
the recordings themselves are the property of their producer and are obtained from them.

## The configuration

- **Organ:** Arnstadt, Wender (Bach Organ).
- **Samples:** Sonus Paradisi - [https://www.sonusparadisi.cz/en/organs/germany/arnstadt-bach-organ.html](https://www.sonusparadisi.cz/en/organs/germany/arnstadt-bach-organ.html); the folders `002310` and `002701` (as they come, or in their RAR archives); perspectives Diffuse, Direct, Distant and Rear. LibreOrgan Sampler 0.9.29 or later.
- **Expects:** library root of any name, holding the trees `002310/pipes/Arnstadt1noc_OW-Haupt`, `002310/pipes/Arnstadt2nocPed`, `002310/pipes/Arnstadt2nocUW-Brust`, `002701/pipes/Arnstadt1noc_OW-Haupt`, `002701/pipes/Arnstadt2nocPed` and `002701/pipes/Arnstadt2nocUW-Brust`.
- **Audio layout:** every division has the routes diffuse, direct, distant and rear; auxiliary (unused).
- **RAM:**
  - **4.2 GB** - 16-bit, 32 kHz, releases at half the rate, packed; *estimated*
  - **6.2 GB** - 16-bit, 48 kHz, full releases, packed; *measured*
  - **42 GB** - 32-bit float, the files' own rate, full releases, no packing; *estimated*
- **Saved with:** LibreOrgan Sampler 0.9.29.
- **Author:** Jiri Zurek, 1 November 2026.

## Where to put the file

Put `Arnstadt, Wender (Bach Organ).portable.json` into your LibreOrgans folder (Configuration Editor > *LibreOrgans folder*; `Documents\LibreOrgan\LibreOrgans` by default) and load it from there. The sample folders are found in your sample locations (the folders above your sample libraries, remembered by the program and listed in the Configuration Editor). One not found shows "Sample folder not found" under its division: click it and choose any folder of the samples (the tree, a folder above it, a neighbouring tree) - the program finds the tree, the other missing ones around it, and remembers the place for every configuration to come. The folders `002310` and `002701` must stand side by side in one folder:

```
<a sample location>\
├── 002310\
│   ├── pipes\Arnstadt1noc_OW-Haupt\
│   ├── pipes\Arnstadt2nocPed\
│   └── pipes\Arnstadt2nocUW-Brust\
└── 002701\
    ├── pipes\Arnstadt1noc_OW-Haupt\
    ├── pipes\Arnstadt2nocPed\
    └── pipes\Arnstadt2nocUW-Brust\
```

## Stop List

| Pedal | I | II |
|---|---|---|
| ped Subbass 16' | BP Still gedackt 8' | OW Principal 8' |
| ped Posaunen bass 16' | BP Principal 4' | OW Quintadena 8' |
| Ped Principal bass 8' | BP Spitzflote 4' | OW Quinta 5 1/3' |
| ped Cornet bass 2' | BP Quinte 2 2/3' | OW Gross gedakt 8' |
|  | BP Nachthorn 4' | OW Octava 4' |
|  | BP Mixtur 3fach 2' | OW Gemshorn 8' |
|  | BP Sesquialtera doppelt 1 3/5' | OW Mixtur 4fach 2' |
|  |  | OW Viol di gamb 8' |
|  |  | OW Cymbel doppelt 1' |
|  |  | OW Trompet 8' |

<!-- published by @zurek-jiri (issue #156, 2026-11-01) -->
