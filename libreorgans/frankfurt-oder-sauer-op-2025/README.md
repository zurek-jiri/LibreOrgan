# Frankfurt (Oder), Sauer op. 2025

<!-- Written by LibreOrgan Sampler. To publish it: attach the zip beside this README (it holds the configuration and this README) to the form (a free GitHub account):
     https://github.com/zurek-jiri/LibreOrgan/issues/new?template=organ_configuration.yml -->

A complete LibreOrgan Sampler configuration of Frankfurt (Oder), Sauer op. 2025, as recorded by Sonus Paradisi in 4 perspectives, Diffuse, Direct, Dry and Rear, each on its own audio route: 4 divisions and 52 rows in the Stop List, transmissions and
extensions included (only the rows nested in stop folders are not counted). Whoever owns
the recordings loads the whole instrument in a single step.

**Download:** [Frankfurt (Oder), Sauer op. 2025.portable.zip](https://github.com/zurek-jiri/LibreOrgan/raw/main/libreorgans/frankfurt-oder-sauer-op-2025/Frankfurt%20(Oder),%20Sauer%20op.%202025.portable.zip) - *Load Configuration* in LibreOrgan Sampler opens the zip as it is (0.8.195) and copies the configuration into your LibreOrgans folder; or unpack it and put the `Frankfurt (Oder), Sauer op. 2025.portable.json` there yourself.

**No samples are published here.** The configuration merely names the folders it expects;
the recordings themselves are the property of their producer and are obtained from them.

## The configuration

- **Organ:** Frankfurt (Oder), Sauer op. 2025.
- **Samples:** Sonus Paradisi - [https://www.sonusparadisi.cz/en/organs/germany/frankfurt-oder-sauer-op-2025.html](https://www.sonusparadisi.cz/en/organs/germany/frankfurt-oder-sauer-op-2025.html); the folders `002704`, `002705` and `002706` (as they come, or in their RAR archives); perspectives Diffuse, Direct, Dry and Rear. LibreOrgan Sampler 0.9.53 or later.
- **Expects:** library root of any name, holding the trees `002704/pipes/HW`, `002704/pipes/Ped`, `002704/pipes/Pos`, `002704/pipes/SW`, `002705/pipes/HW`, `002705/pipes/Ped`, `002705/pipes/Pos`, `002705/pipes/SW`, `002706/pipes/HW`, `002706/pipes/Ped`, `002706/pipes/Pos` and `002706/pipes/SW`.
- **Audio layout:** every division has the routes diffuse, direct, dry and rear; auxiliary (unused).
- **RAM:**
  - **11 GB** - 16-bit, 32 kHz, releases at half the rate, packed; *estimated*
  - **13 GB** - 16-bit, 48 kHz, releases at half the rate, packed; *measured*
  - **106 GB** - 32-bit float, the files' own rate, full releases, no packing; *estimated*
- **Saved with:** LibreOrgan Sampler 0.9.53.
- **Author:** Jiri Zurek, 4 November 2026.

## Where to put the file

Put `Frankfurt (Oder), Sauer op. 2025.portable.json` into your LibreOrgans folder (Configuration Editor > *LibreOrgans folder*; `Documents\LibreOrgan\LibreOrgans` by default) and load it from there. The sample folders are found in your sample locations (the folders above your sample libraries, remembered by the program and listed in the Configuration Editor). One not found shows "Sample folder not found" under its division: click it and choose any folder of the samples (the tree, a folder above it, a neighbouring tree) - the program finds the tree, the other missing ones around it, and remembers the place for every configuration to come. The folders `002704`, `002705` and `002706` must stand side by side in one folder:

```
<a sample location>\
├── 002704\
│   ├── pipes\HW\
│   ├── pipes\Ped\
│   ├── pipes\Pos\
│   └── pipes\SW\
├── 002705\
│   ├── pipes\HW\
│   ├── pipes\Ped\
│   ├── pipes\Pos\
│   └── pipes\SW\
└── 002706\
    ├── pipes\HW\
    ├── pipes\Ped\
    ├── pipes\Pos\
    └── pipes\SW\
```

## Stop List

| Pedal | I | II | III |
|---|---|---|---|
| Ped. Subbass 32' | Pos Holzgedackt 8' | HW Pommer 16' | SW Bordun 16' |
| Ped. Prinzipal 16' | Pos Quintadena 8' | HW Gemshorn 8' | SW Schwebung 8' |
| Ped. Subbass 16' | Pos Prästant 4' | HW Prinzipal 8' | SW Holzprinzipal 8' |
| Ped. Oktavbass 8' | Pos Rohrflöte 4' | HW Klein Gedackt 4' | SW Rohrgedackt 8' |
| Ped. Pommer 8' | Pos Oktave 2' | HW Oktave 4' | SW Oktave 4' |
| Ped. Hohlflöte 4' | Pos Gemshorn 1' | HW Nasat 2 2/3' | SW Salizional 8' |
| Ped. Rohrpfeife 2' | Pos Quinte 1 1/3' | HW Oktave 2' | SW Quintan 1 1/3' |
| Ped. Mixtur 7 f. 4' | Pos Terz 1 3/5' | HW Blockflöte 2' | SW Spitzflöte 4' |
| Ped. Groß Sesquialt 3f 5 1/3' | Pos Scharff 4 f 1' | HW Mixtur 5-6 f. 2' | SW Mixtur 5-7 f. 2' |
| Ped. Posaune 16' | Pos Holzregal 8' | HW Zimbel 3f. 1/3' | SW Waldflöte 2' |
| Ped. Sordun 32' |  | HW Fagott 16' | SW Dulzian 16' |
| Ped. Bombarde 8' |  | HW Trompete 8' | SW Septime 1 1/7' |
| Ped. Corno 4' |  |  | SW Oktävlein 1' |
| Ped. Singend Kornett 2' |  |  | SW Sesquialt 2f. 1 1/3' |
|  |  |  | SW Oboe 8' |
|  |  |  | SW Helltrompete 4' |

<!-- published by @zurek-jiri (issue #208, 2026-11-04) -->
