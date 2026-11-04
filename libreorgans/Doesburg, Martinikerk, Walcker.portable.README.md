# Doesburg, Martinikerk, Walcker

<!-- Written by LibreOrgan Sampler. To publish it: attach the zip beside this README (it holds the configuration and this README) to the form (a free GitHub account):
     https://github.com/zurek-jiri/LibreOrgan/issues/new?template=organ_configuration.yml -->

A complete LibreOrgan Sampler configuration of Doesburg, Martinikerk, Walcker, as recorded by Sonus Paradisi in 3 perspectives, Dry, Diffuse and Rear, each on its own audio route: 5 divisions and 76 rows in the Stop List, transmissions and
extensions included (only the rows nested in stop folders are not counted). Whoever owns
the recordings loads the whole instrument in a single step.

**Download:** [Doesburg, Martinikerk, Walcker.portable.zip](https://github.com/zurek-jiri/LibreOrgan/raw/main/libreorgans/doesburg-martinikerk-walcker/Doesburg,%20Martinikerk,%20Walcker.portable.zip) - *Load Configuration* in LibreOrgan Sampler opens the zip as it is (0.8.195) and copies the configuration into your LibreOrgans folder; or unpack it and put the `Doesburg, Martinikerk, Walcker.portable.json` there yourself.

**No samples are published here.** The configuration merely names the folders it expects;
the recordings themselves are the property of their producer and are obtained from them.

## The configuration

- **Organ:** Doesburg, Martinikerk, Walcker.
- **Samples:** Sonus Paradisi - [https://www.sonusparadisi.cz/en/organs/netherlands/doesburg-martinikerk-walcker-organ.html](https://www.sonusparadisi.cz/en/organs/netherlands/doesburg-martinikerk-walcker-organ.html); the folders `002292`, `002293` and `002294` (as they come, or in their RAR archives); perspectives Dry (`su`), Diffuse (`AB`) and Rear (`Re`). LibreOrgan Sampler 0.9.51 or later.
- **Expects:** library root of any name, holding the trees `002292/I_Man`, `002292/II_Pos`, `002292/Ped`, `002293/I_MAN`, `002293/II_Pos`, `002293/III`, `002293/IV`, `002293/Ped`, `002294/I_MAN`, `002294/II_Pos`, `002294/III`, `002294/IV` and `002294/Ped`.
- **Audio layout:** every division has the routes diffuse; auxiliary, dry and rear (unused).
- **RAM:**
  - **7.6 GB** - 16-bit, 32 kHz, releases at half the rate, packed; *estimated*
  - **9.4 GB** - 16-bit, 48 kHz, releases at half the rate, packed; *measured*
  - **76 GB** - 32-bit float, the files' own rate, full releases, no packing; *estimated*
- **Saved with:** LibreOrgan Sampler 0.9.51.
- **Author:** Jiri Zurek, 4 November 2026.

## Where to put the file

Put `Doesburg, Martinikerk, Walcker.portable.json` into your LibreOrgans folder (Configuration Editor > *LibreOrgans folder*; `Documents\LibreOrgan\LibreOrgans` by default) and load it from there. The sample folders are found in your sample locations (the folders above your sample libraries, remembered by the program and listed in the Configuration Editor). One not found shows "Sample folder not found" under its division: click it and choose any folder of the samples (the tree, a folder above it, a neighbouring tree) - the program finds the tree, the other missing ones around it, and remembers the place for every configuration to come. The folders `002292`, `002293` and `002294` must stand side by side in one folder:

```
<a sample location>\
├── 002292\
│   ├── I_Man\
│   ├── II_Pos\
│   └── Ped\
├── 002293\
│   ├── I_MAN\
│   ├── II_Pos\
│   ├── III\
│   ├── IV\
│   └── Ped\
└── 002294\
    ├── I_MAN\
    ├── II_Pos\
    ├── III\
    ├── IV\
    └── Ped\
```

## Stop List

| Pedal | I | II | III | IV |
|---|---|---|---|---|
| Grand Bourdon 32' | 1. Prinzipal 16' | 2. Bourdon 16' | 3. Lieblich-Gedackt 16' | 4. Quintatoen 16' |
| P. Prinzipalbass 16' | 1. Gross-Prinzipal 8' | 2. Syntematophon 8' | 3. Lieblich-Gedackt 8' | 4. Echo-Bourdon 8' |
| P. Subbass 16' | 1. Bourdon 8' | 2. Floeten-Prinzipal 8' | 3. Geigen-Prinzipal 8' | 4. Nachthorn 8' |
| P. Harmonikabass 16' | 1. Viola di Gamba 8' | 2. Doppel-Gedackt 8' | 3. Konzert-Floete 8' | 4. Echo-Gamba 8' |
| P. Kontrabass 16' (Trsm.) | 1. Jubal-Floete 8' | 2. Flute harm. 8' | 3. Quintatoen 8' | 4. Vox angelika 8' |
| P. Floetenbass 16' | 1. Gemshorn 8' | 2. Cello 8' | 3. Viola d amour 8' | 4. Spitz-Floete 4' |
| P. Echobass 16' (Trsm.) | 1. Dulciana 8' | 2. Salicional 8' | 3. Aeoline 8' | 4. Glockenton 1 1/3' |
| P. Gedacktbass 16' (Trsm.) | 1. Praestant 4' | 2. Harmonika 8' | 3. Vox coelestis 8' | 4. Echo-Trompete 8' |
| P. Quintbass 10 2/3' | 1. Rohr-Floete 4' | 2. Prinzipal 4' | 3. Liebes-Geige 4' | 4. Vox humana 8' |
| P. Floetenbass 8' (Trsm.) | 1. Oktave 2' | 2. Orchester-Floete 4' | 3. Flauto dolce 4' |  |
| P. Oktavbass 8' | 1. Kornett 2 2/3' | 2. Piccolo 2' | 3. Flautino 2' |  |
| P. Bourdon doux 8' (Trsm.) | 1. Mixtur 2' | 2. Kornettino 2 2/3' | 3. Sesquialtera 2 2/3' |  |
| 1. Jubal-Floete 8' (Trsm.) | 1. Trompete 8' | 2. Gross-Mixtur 2' | 3. Cymbel 2' |  |
| P. Bassfloete 8' (Trsm.) |  | 2. Nasard 2 2/3' | 3. Klarinette 8' |  |
| P. Violoncello 8' (Trsm.) |  | 2. Basson 16' |  |  |
| P. Choralbass 4' |  | 2. Trompete harm. 8' |  |  |
| P. Mixtur 2' (Trsm.) |  | 2. Clairon harm. 4' |  |  |
| P. Bombardon 32' |  | 2. Orchester-Oboe 8' |  |  |
| P. Bassonbass 16' (Trsm.) |  |  |  |  |
| P. Posaune 16' |  |  |  |  |
| P. Tuba 8' |  |  |  |  |
| P. Clairon harm. 4' (Trsm.) |  |  |  |  |

<!-- published by @zurek-jiri (issue #195, 2026-11-04) -->
