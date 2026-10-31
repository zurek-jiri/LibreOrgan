# Billerbeck Dom, Fleiter organ

<!-- Written by LibreOrgan Sampler. To publish it: attach the zip beside this README (it holds the configuration and this README) to the form (a free GitHub account):
     https://github.com/zurek-jiri/LibreOrgan/issues/new?template=organ_configuration.yml -->

A complete LibreOrgan Sampler configuration of Billerbeck Dom, Fleiter organ, as recorded by Sonus Paradisi in 4 perspectives, Diffuse, Direct, Dry and Rear, each on its own audio route: 6 divisions and 84 rows in the Stop List, transmissions and
extensions included (only the rows nested in stop folders are not counted). Whoever owns
the recordings loads the whole instrument in a single step.

**Download:** [Fleiter, Billerbeck, 8 chan..portable.zip](https://github.com/zurek-jiri/LibreOrgan/raw/main/libreorgans/billerbeck-dom-fleiter-organ/Fleiter,%20Billerbeck,%208%20chan..portable.zip) - *Load Configuration* in LibreOrgan Sampler opens the zip as it is (0.8.195) and copies the configuration into your LibreOrgans folder; or unpack it and put the `Fleiter, Billerbeck, 8 chan..portable.json` there yourself.

**No samples are published here.** The configuration merely names the folders it expects;
the recordings themselves are the property of their producer and are obtained from them.

## The configuration

- **Organ:** Billerbeck Dom, Fleiter organ.
- **Samples:** Sonus Paradisi - [https://www.sonusparadisi.cz/en/organs/germany/billerbeck-dom-fleiter-organ.html](https://www.sonusparadisi.cz/en/organs/germany/billerbeck-dom-fleiter-organ.html); the folders `002283`, `002284`, `002285` and `002286` (as they come, or in their RAR archives); perspectives Diffuse (`04-distMk2H`), Direct (`03-difMk22`), Dry and Rear (`05-rearMK4`). LibreOrgan Sampler 0.8.225 or later.
- **Expects:** library root of any name, holding the trees `002283/Cham`, `002283/HW`, `002283/Ped`, `002283/Pos`, `002283/Schw`, `002284/Car`, `002284/Cham`, `002284/HW`, `002284/Ped`, `002284/Pos`, `002284/Schw`, `002285/Car`, `002285/Cham`, `002285/HW`, `002285/Ped`, `002285/Pos`, `002285/Schw`, `002286/Cham`, `002286/HW`, `002286/Ped`, `002286/Pos` and `002286/Schw`.
- **Audio layout:** every division has the routes diffuse, direct, dry and rear; auxiliary (unused).
- **RAM:**
  - **15 GB** - 16-bit, 32 kHz, releases at half the rate, packed; *measured*
  - **148 GB** - 32-bit float, the files' own rate, full releases, no packing; *estimated*
- **Saved with:** LibreOrgan Sampler 0.8.225.
- **Author:** Jiri Zurek, 29 October 2026.

## Where to put the file

Put `Fleiter, Billerbeck, 8 chan..portable.json` into your LibreOrgans folder (Configuration Editor > *LibreOrgans folder*; `Documents\LibreOrgan\LibreOrgans` by default) and load it from there. The sample folders are found in your sample locations (the folders above your sample libraries, remembered by the program and listed in the Configuration Editor). One not found shows "Sample folder not found" under its division: click it and choose any folder of the samples (the tree, a folder above it, a neighbouring tree) - the program finds the tree, the other missing ones around it, and remembers the place for every configuration to come. The folders `002283`, `002284`, `002285` and `002286` must stand side by side in one folder:

```
<a sample location>\
├── 002283\
│   ├── Cham\
│   ├── HW\
│   ├── Ped\
│   ├── Pos\
│   └── Schw\
├── 002284\
│   ├── Car\
│   ├── Cham\
│   ├── HW\
│   ├── Ped\
│   ├── Pos\
│   └── Schw\
├── 002285\
│   ├── Car\
│   ├── Cham\
│   ├── HW\
│   ├── Ped\
│   ├── Pos\
│   └── Schw\
└── 002286\
    ├── Cham\
    ├── HW\
    ├── Ped\
    ├── Pos\
    └── Schw\
```

## Stop List

| Pedal | I | II | III | IV | V |
|---|---|---|---|---|---|
| P. Grosse flute 64' (Ext.) | 1. Principal 8' | 2. Grande Bourdon 32' | 3. Rohrbourdon 16' | 4. Flute harmonique 8' | 5. Tuba 16' |
| P. Contrabass 32' (Ext.) | 1. Rohrflote 8' | 2. Violone 16' | 3. Doppelflote 8' | 4. Cornett 8' | 5. Tuba 8' (Ext.) |
| P. Bourdon 32' | 1. Salizional 8' | 2. Principal 16' | 3. Geigenprincipal 8' | 4. Trompeta magna 16' | 5. Tuba 4' |
| P. Grosse flute 32' | 1. Unda maris 8' | 2. Flute harmonique 8' | 3. Bourdon 8' | 4. Trompeta real 8' |  |
| P. Contrabass 16' | 1. Praestant 4' | 2. Principal major 8' | 3. Viola 8' | 4. Trompeta Clarin 4' |  |
| P. Flute 16' (Ext.) | 1. Holzflote 4' | 2. Viola da Gamba 8' | 3. Vox coelestis 8' | 4. Vox Ludgeri 8' |  |
| P. Principalbass 16' | 1. Waldflote 2' | 2. Praestant 8' | 3. Principal 4' | 4. Carillon 8' (Trsm.) |  |
| P. Soubasse 16' (Ext.) | 1. Quinte 1 1/3' | 2. Gedackt 8' | 3. Flute octaviante 4' |  |  |
| P. Bourdon 8' (Trsm.) | 1. Mixtur 4f. 1 1/3' | 2. Octave major 4' | 3. Fugara 4' |  |  |
| P. Octavbass 8' | 1. Sesquialtera 2 2/3' | 2. Flute conique 4' | 3. Nazard harmonique 2 2/3' |  |  |
| P. Flute 8' | 1. Trompete 8' | 2. Octave minor 4' | 3. Octavin 2' |  |  |
| P. Cello 8' | 1. Cromorne 8' | 2. Quinte 2 2/3' | 3. Tierce 1 3/5' |  |  |
| P. Choralbass 4' | 1. Carillon | 2. Octave 2' | 3. Fourniture 5f. 2 2/3' |  |  |
| P. Viola tenore 4' |  | 2. Cornett 5f. 8' | 3. Basson 16' |  |  |
| P. Carillon 4' (Trsm.) |  | 2. Mixture major 2' | Bombarde 16' (Ext.) |  |  |
| P. Posaune 64' (Ext.) |  | 2. Mixture minor 1' | 3. Hautbois 8' |  |  |
| P. Contrebomb. 32' |  | 2. Bombarde 16' | 3. Trompette harmonique 8' |  |  |
| P. Posaune 16' |  | 2. Trompete 8' | 3. Vox humana 8' |  |  |
| P. Fagott 16' |  | 2. Carillon 8' (Trsm.) | 3. Clarion 4' |  |  |
| P. Trompetenbass 8' |  |  | 3. Carillon |  |  |
| P. Clarion 4' |  |  |  |  |  |
| 5. Tuba 2' (Trsm.) |  |  |  |  |  |

<!-- published by @zurek-jiri (issue #124, 2026-10-29) -->
