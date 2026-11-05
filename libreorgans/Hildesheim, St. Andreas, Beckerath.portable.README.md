# Hildesheim, St. Andreas, Beckerath Organ

<!-- Written by LibreOrgan Sampler. To publish it: attach the zip beside this README (it holds the configuration and this README) to the form (a free GitHub account):
     https://github.com/zurek-jiri/LibreOrgan/issues/new?template=organ_configuration.yml -->

A complete LibreOrgan Sampler configuration of Hildesheim, St. Andreas, Beckerath Organ, as recorded by Sonus Paradisi in 3 perspectives, Direct, Diffuse and Rear, each on its own audio route: 5 divisions and 65 rows in the Stop List, transmissions and
extensions included (only the rows nested in stop folders are not counted). Whoever owns
the recordings loads the whole instrument in a single step.

**Download:** [Hildesheim, St. Andreas, Beckerath.portable.zip](https://github.com/zurek-jiri/LibreOrgan/raw/main/libreorgans/hildesheim-st-andreas-beckerath-organ/Hildesheim,%20St.%20Andreas,%20Beckerath.portable.zip) - *Load Configuration* in LibreOrgan Sampler opens the zip as it is (0.8.195) and copies the configuration into your LibreOrgans folder; or unpack it and put the `Hildesheim, St. Andreas, Beckerath.portable.json` there yourself.

**No samples are published here.** The configuration merely names the folders it expects;
the recordings themselves are the property of their producer and are obtained from them.

## The configuration

- **Organ:** Hildesheim, St. Andreas, Beckerath Organ.
- **Samples:** Sonus Paradisi - [https://www.sonusparadisi.cz/en/organs/germany/hildesheim-st-andreas-beckerath-1966.html](https://www.sonusparadisi.cz/en/organs/germany/hildesheim-st-andreas-beckerath-1966.html); the folders `002718` and `002719` (as they come, or in their RAR archives); perspectives Direct (`02dif-PE`), Diffuse (`03dis-PE`) and Rear (`04rea-PE`). LibreOrgan Sampler 0.9.61 or later.
- **Expects:** library root of any name, holding the trees `002718/pipes/1RW`, `002718/pipes/2HW`, `002718/pipes/3OW`, `002718/pipes/PED`, `002719/pipes/1RW`, `002719/pipes/2HW`, `002719/pipes/3OW`, `002719/pipes/4BW` and `002719/pipes/PED`.
- **Audio layout:** every division has the routes direct, diffuse and rear; auxiliary (unused).
- **RAM:**
  - **11 GB** - 16-bit, 32 kHz, releases at half the rate, packed; *estimated*
  - **13 GB** - 16-bit, 48 kHz, releases at half the rate, packed; *measured*
  - **114 GB** - 32-bit float, the files' own rate, full releases, no packing; *estimated*
- **Saved with:** LibreOrgan Sampler 0.9.61.
- **Author:** Jiri Zurek, 5 November 2026.

## Where to put the file

Put `Hildesheim, St. Andreas, Beckerath.portable.json` into your LibreOrgans folder (Configuration Editor > *LibreOrgans folder*; `Documents\LibreOrgan\LibreOrgans` by default) and load it from there. The sample folders are found in your sample locations (the folders above your sample libraries, remembered by the program and listed in the Configuration Editor). One not found shows "Sample folder not found" under its division: click it and choose any folder of the samples (the tree, a folder above it, a neighbouring tree) - the program finds the tree, the other missing ones around it, and remembers the place for every configuration to come. The folders `002718` and `002719` must stand side by side in one folder:

```
<a sample location>\
├── 002718\
│   ├── pipes\1RW\
│   ├── pipes\2HW\
│   ├── pipes\3OW\
│   └── pipes\PED\
└── 002719\
    ├── pipes\1RW\
    ├── pipes\2HW\
    ├── pipes\3OW\
    ├── pipes\4BW\
    └── pipes\PED\
```

## Stop List

| Pedal | I | II | III | IV |
|---|---|---|---|---|
| P. Prinzipal 32' | 1. Prinzipal 8' | 2. Prinzipal 16' | 3. Quintadena 16' | 4. Holzgedackt 8' |
| P. Oktave 16' | 1. Rohrfloete 8' | 2. Oktave 8' | 3. Violprinzipal 8' | 4. Holzprinzipal 4' |
| P. Subbass 16' | 1. Quintadena 8' | 2. Koppelgedackt 8' | 3. Holzflote 8' | 4. Waldfloete 2' |
| P. Oktave 8' | 1. Oktave 4' | 2. Oktave 4' | 3. Oktave 4' | 4. Gemsquinte 1 1/3' |
| P. Holzfloete 8' | 1. Blockfloete 4' | 2. Quinte 2 2/3' | 3. Rohrfloete 4' | 4. Schwiegel 1' |
| P. Oktave 4' | 1. Quintfloete 2 2/3' | 2. Oktave 2' | 3. Nasat 2 2/3' | 4. Schlagtone 3f 2/5' |
| P. Hornaliquot 5 1/3' | 1. Gemshorn 2' | 2. Mixtur 6f 2' | 3. Hohlfloete 2' | 4. Scharfzimbel 4f 1/2' |
| P. Nachthorn 2' | 1. Oktave 2' | 2. Scharf 4f 2/3' | 3. Terz 1 3/5' | 4. Regal 8' |
| P. Rauschpfeife 3f 4' | 1. Quinte 1 1/3' | 2. Trompete 16' | 3. Septime 1 1/7' | 4. Schalmei 4' |
| P. Mixtur 6f 2 2/3' | 1. Scharfmixtur 5f 1 1/3' | 2. Trompete 8' | 3. Siffloete 1' |  |
| P. Posaune 32' | 1. Sesquialtera 1 1/3' | 2. Trompete 4' | 3. None 8/9' |  |
| P. Posaune 16' | 1. Dulzian 16' | 2. Glocken-Hw 1 3/5' | 3. Scharf 4-6f 1' |  |
| P. Trompete 8' | 1. Barpfeife 8' |  | 3. Klgd.Zimbel 3f 1/6' |  |
| P. Trompete 4' |  |  | 3. Englishhorn 16' |  |
| P. Zink 2' |  |  | 3. Oboe 8' |  |
| P. Glocken-Ped 1 3/5' (Trsm.) |  |  |  |  |

<!-- published by @zurek-jiri (issue #212, 2026-11-05) -->
