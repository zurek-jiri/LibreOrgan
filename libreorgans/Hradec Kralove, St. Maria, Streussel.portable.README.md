# Hradec Kralove, St. Maria, Streussel Organ

<!-- Written by LibreOrgan Sampler. To publish it: attach the zip beside this README (it holds the configuration and this README) to the form (a free GitHub account):
     https://github.com/zurek-jiri/LibreOrgan/issues/new?template=organ_configuration.yml -->

A complete LibreOrgan Sampler configuration of Hradec Kralove, St. Maria, Streussel Organ, as recorded by Sonus Paradisi in one perspective, default, each on its own audio route: 3 divisions and 34 rows in the Stop List, transmissions and
extensions included (only the rows nested in stop folders are not counted). Whoever owns
the recordings loads the whole instrument in a single step.

**Download:** [Hradec Kralove, St. Maria, Streussel.portable.zip](https://github.com/zurek-jiri/LibreOrgan/raw/main/libreorgans/hradec-kralove-st-maria-streussel-organ/Hradec%20Kralove,%20St.%20Maria,%20Streussel.portable.zip) - *Load Configuration* in LibreOrgan Sampler opens the zip as it is (0.8.195) and copies the configuration into your LibreOrgans folder; or unpack it and put the `Hradec Kralove, St. Maria, Streussel.portable.json` there yourself.

**No samples are published here.** The configuration merely names the folders it expects;
the recordings themselves are the property of their producer and are obtained from them.

## The configuration

- **Organ:** Hradec Kralove, St. Maria, Streussel Organ.
- **Samples:** Sonus Paradisi - [https://www.sonusparadisi.cz/en/organs/czech/hrade-kralove-maria-church.html](https://www.sonusparadisi.cz/en/organs/czech/hrade-kralove-maria-church.html); the folders `001810` (as they come, or in their RAR archives); perspectives default. LibreOrgan Sampler 0.9.64 or later.
- **Expects:** library root of any name, holding the trees `001810/pipes/hw`, `001810/pipes/ped` and `001810/pipes/pos`.
- **Audio layout:** every division has the routes default; auxiliary (unused).
- **RAM:**
  - **1.5 GB** - 16-bit, 32 kHz, releases at half the rate, packed; *estimated*
  - **1.7 GB** - 16-bit, 48 kHz, releases at half the rate, packed; *measured*
  - **15 GB** - 32-bit float, the files' own rate, full releases, no packing; *estimated*
- **Saved with:** LibreOrgan Sampler 0.9.64.
- **Author:** Jiri Zurek, 5 November 2026.

## Where to put the file

Put `Hradec Kralove, St. Maria, Streussel.portable.json` into your LibreOrgans folder (Configuration Editor > *LibreOrgans folder*; `Documents\LibreOrgan\LibreOrgans` by default) and load it from there. The sample folders are found in your sample locations (the folders above your sample libraries, remembered by the program and listed in the Configuration Editor). One not found shows "Sample folder not found" under its division: click it and choose any folder of the samples (the tree, a folder above it, a neighbouring tree) - the program finds the tree, the other missing ones around it, and remembers the place for every configuration to come. The folder `001810` is expected like this:

```
<a sample location>\
└── 001810\
    ├── pipes\hw\
    ├── pipes\ped\
    └── pipes\pos\
```

## Stop List

| Pedal | I | II |
|---|---|---|
| Flutte 32' (Trsm.) | Viola 16' (Ext.) | Copula major 8' |
| Violon 16' (Trsm.) | Flauta 16' (Ext.) | Salicional 8' |
| Subbas 16' (Trsm.) | Principal 8' | Principal 4' |
| Subbas offen 16' | Flauta 8' | Copula minor 4' |
| Octavbas 8' | Gamba 8' | Fugara 4' (Ext.) |
| Cello 8' (Trsm.) | Viola 8' | Nasat 2 2/3' (Ext.) |
| Principal bas 8' | Octava 4' | Terz 1 3/5' (Ext.) |
| Quintbas 5 1/3' | Flauta 4' (Ext.) | Superoctav 2' |
| Principal bas 4' | Quinta 2 2/3' | Quint 1 1/3' (Ext.) |
| Cornetbas 3x 2 2/3' | Terzia 1 3/5' (Ext.) | Mixtura 3x 1' |
|  | Octava 2' |  |
|  | Rausch quint 2x 1 1/3' |  |
|  | Mixtura 3x 1' |  |
|  | Cimbel 5x 2/3' |  |

<!-- published by @zurek-jiri (issue #216, 2026-11-05) -->
