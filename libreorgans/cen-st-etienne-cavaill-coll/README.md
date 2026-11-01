# Cen, St. Etienne, Cavaillé-Coll

<!-- Written by LibreOrgan Sampler. To publish it: attach the zip beside this README (it holds the configuration and this README) to the form (a free GitHub account):
     https://github.com/zurek-jiri/LibreOrgan/issues/new?template=organ_configuration.yml -->

A complete LibreOrgan Sampler configuration of Cen, St. Etienne, Cavaillé-Coll, as recorded by Sonus Paradisi in 2 perspectives, Rear and Front, each on its own audio route: 4 divisions and 55 rows in the Stop List, transmissions and
extensions included (only the rows nested in stop folders are not counted). Whoever owns
the recordings loads the whole instrument in a single step.

**Download:** [Caen Surround.portable.zip](https://github.com/zurek-jiri/LibreOrgan/raw/main/libreorgans/cen-st-etienne-cavaill-coll/Caen%20Surround.portable.zip) - *Load Configuration* in LibreOrgan Sampler opens the zip as it is (0.8.195) and copies the configuration into your LibreOrgans folder; or unpack it and put the `Caen Surround.portable.json` there yourself.

**No samples are published here.** The configuration merely names the folders it expects;
the recordings themselves are the property of their producer and are obtained from them.

## The configuration

- **Organ:** Cen, St. Etienne, Cavaillé-Coll.
- **Samples:** Sonus Paradisi - [https://www.sonusparadisi.cz/en/organs/france/caen-st-etienne.html](https://www.sonusparadisi.cz/en/organs/france/caen-st-etienne.html); the folders `001318`, `001319`, `001320`, `001691`, `001692`, `001693`, `001694` and `001695` (as they come, or in their RAR archives); perspectives Rear and Front. LibreOrgan Sampler 0.9.30 or later.
- **Expects:** library root of any name, holding the trees `001318/pipe/go`, `001318/pipe/rec`, `001319/pipe/go`, `001320/pipe/ped`, `001320/pipe/pos`, `001320/pipe/rec`, `001691/pipe/rec`, `001692/pipe/go`, `001692/pipe/ped`, `001693/pipe/ped`, `001693/pipe/pos`, `001693/pipe/rec`, `001694/pipe/rec`, `001695/pipe/go`, `001695/pipe/ped` and `001695/pipe/rec`.
- **Audio layout:** every division has the routes rear and front; auxiliary (unused).
- **RAM:**
  - **5.7 GB** - 16-bit, 32 kHz, releases at half the rate, packed; *measured*
  - **57 GB** - 32-bit float, the files' own rate, full releases, no packing; *estimated*
- **Saved with:** LibreOrgan Sampler 0.9.30.
- **Author:** Jiri Zurek, 1 November 2026.

## Where to put the file

Put `Caen Surround.portable.json` into your LibreOrgans folder (Configuration Editor > *LibreOrgans folder*; `Documents\LibreOrgan\LibreOrgans` by default) and load it from there. The sample folders are found in your sample locations (the folders above your sample libraries, remembered by the program and listed in the Configuration Editor). One not found shows "Sample folder not found" under its division: click it and choose any folder of the samples (the tree, a folder above it, a neighbouring tree) - the program finds the tree, the other missing ones around it, and remembers the place for every configuration to come. The folders `001318`, `001319`, `001320`, `001691`, `001692`, `001693`, `001694` and `001695` must stand side by side in one folder:

```
<a sample location>\
├── 001318\
│   ├── pipe\go\
│   └── pipe\rec\
├── 001319\
│   └── pipe\go\
├── 001320\
│   ├── pipe\ped\
│   ├── pipe\pos\
│   └── pipe\rec\
├── 001691\
│   └── pipe\rec\
├── 001692\
│   ├── pipe\go\
│   └── pipe\ped\
├── 001693\
│   ├── pipe\ped\
│   ├── pipe\pos\
│   └── pipe\rec\
├── 001694\
│   └── pipe\rec\
└── 001695\
    ├── pipe\go\
    ├── pipe\ped\
    └── pipe\rec\
```

## Stop List

| Pedal | I | II | III |
|---|---|---|---|
| Ped: Bourdon 32' | GO: Montre 16' | Pos: Bourdon 16' | Rec: Quintaton 16' |
| Ped: Contrebasse 16' | GO: Bourdon 16' | Pos: Cor de nuit 8' | Rec: Diapason 8' |
| Ped: Soubasse 16' | GO: Montre 8' | Pos: Principal 8' | Rec: Flute travers 8' |
| Ped: Grosse flute 8' | GO: Bourdon 8' | Pos: Salicional 8' | Rec: Viole de gambe 8' |
| Ped: Violoncelle 8' | GO: Flute harm. 8' | Pos: Prestant 4' | Rec: Voix celeste 8' |
| Ped: Bourdon doux 8' | GO: Gambe 8' | Pos: Unda maris 8' | Rec: Flute octav. 4' |
| Ped: Flute 4' | GO: Prestant 4' | Pos: Flute douce 4' | Rec: Nasard 2 2/3' |
| Ped: Contrebombarde 32' | GO: Flute octav. 4' | Pos: Carillon III 2 2/3' | Rec: Octavin 2' |
| Ped: Bombarde 16' | GO: Doublette 2' | Pos: Nasard 2 2/3' | Rec: Cornet II-IV |
| Ped: Trompette 8' | GO: Quinte 2 2/3' | Pos: Tierce 1 3/5' | Rec: Plein Jeu |
| Ped: Clairon 4' | GO: Plein Jeu VII 2' | Pos: Basson 16' | Rec: Bombarde 16' |
|  | GO: Cornet V 8' | Pos: Trompette 8' | Rec: Basson-hautbois |
|  | GO: Bombarde 16' | Pos: Cromorne 8' | Rec: Trompette 8' |
|  | GO: Trompette 8' |  | Rec: Voix humaine 8' |
|  | GO: Clairon 4' |  | Rec: Clairon 4' |
|  |  |  | Rec: Clarinette 8' |

<!-- published by @zurek-jiri (issue #158, 2026-11-01) -->
