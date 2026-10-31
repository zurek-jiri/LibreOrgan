# Brescia, St. Carlo, Antegnati Organ

<!-- Written by LibreOrgan Sampler. To publish it: attach the zip beside this README (it holds the configuration and this README) to the form (a free GitHub account):
     https://github.com/zurek-jiri/LibreOrgan/issues/new?template=organ_configuration.yml -->

A complete LibreOrgan Sampler configuration of Brescia, St. Carlo, Antegnati Organ, as recorded by Sonus Paradisi in 2 perspectives, Pedale and default, each on its own audio route: 2 divisions and 12 rows in the Stop List, transmissions and
extensions included (only the rows nested in stop folders are not counted). Whoever owns
the recordings loads the whole instrument in a single step.

**Download:** [Brescia, St. Carlo, Antegnati Organ.portable.zip](https://github.com/zurek-jiri/LibreOrgan/raw/main/libreorgans/brescia-st-carlo-antegnati-organ/Brescia,%20St.%20Carlo,%20Antegnati%20Organ.portable.zip) - *Load Configuration* in LibreOrgan Sampler opens the zip as it is (0.8.195) and copies the configuration into your LibreOrgans folder; or unpack it and put the `Brescia, St. Carlo, Antegnati Organ.portable.json` there yourself.

**No samples are published here.** The configuration merely names the folders it expects;
the recordings themselves are the property of their producer and are obtained from them.

## The configuration

- **Organ:** Brescia, St. Carlo, Antegnati Organ.
- **Samples:** Sonus Paradisi - [https://www.sonusparadisi.cz/en/organs/italy/brescia-st-carlo.html](https://www.sonusparadisi.cz/en/organs/italy/brescia-st-carlo.html); the folders `000370` (as they come, or in their RAR archives); perspectives Pedale and default. LibreOrgan Sampler 0.9.13 or later.
- **Expects:** library root of any name, holding the trees `000370/pipe`.
- **Audio layout:** Pedal: the routes pedale; auxiliary (unused); I: the routes default; auxiliary (unused).
- **RAM:**
  - **1.3 GB** - 16-bit, 32 kHz, releases at half the rate, packed; *estimated*
  - **3.8 GB** - 16-bit, 48 kHz, full releases, no packing; *measured*
  - **13 GB** - 32-bit float, the files' own rate, full releases, no packing; *estimated*
- **Saved with:** LibreOrgan Sampler 0.9.13.
- **Author:** Jiri Zurek, 31 October 2026.

## Where to put the file

Put `Brescia, St. Carlo, Antegnati Organ.portable.json` into your LibreOrgans folder (Configuration Editor > *LibreOrgans folder*; `Documents\LibreOrgan\LibreOrgans` by default) and load it from there. The sample folders are found in your sample locations (the folders above your sample libraries, remembered by the program and listed in the Configuration Editor). One not found shows "Sample folder not found" under its division: click it and choose any folder of the samples (the tree, a folder above it, a neighbouring tree) - the program finds the tree, the other missing ones around it, and remembers the place for every configuration to come. The folder `000370` is expected like this:

```
<a sample location>\
└── 000370\
    └── pipe\
```

## Stop List

| Pedal | I |
|---|---|
| 12 Contrabassi (Pedale) 16' | 01 Principale bassi 8' |
|  | 02 Principale soprani 8' (Ext.) |
|  | 11 Fiffaro 8' |
|  | 03 Ottava 4' |
|  | 09 Flauto in VIII 4' |
|  | 04 Quinta Decima 2' |
|  | 10 Flauto in XII 2 2/3' |
|  | 05 Decima IX 1 1/3' |
|  | 06 Vigesima II 1' |
|  | 07 Vigesima VI 2/3' |
|  | 08 Vigesima IX 1/2' |

<!-- published by @zurek-jiri (issue #128, 2026-10-31) -->
