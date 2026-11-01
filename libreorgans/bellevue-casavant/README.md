# Bellevue, Casavant

<!-- Written by LibreOrgan Sampler. To publish it: attach the zip beside this README (it holds the configuration and this README) to the form (a free GitHub account):
     https://github.com/zurek-jiri/LibreOrgan/issues/new?template=organ_configuration.yml -->

A complete LibreOrgan Sampler configuration of Bellevue, Casavant, as recorded by Sonus Paradisi in 2 perspectives, Front and Rear, each on its own audio route: 4 divisions and 74 rows in the Stop List, transmissions and
extensions included (only the rows nested in stop folders are not counted). Whoever owns
the recordings loads the whole instrument in a single step.

**Download:** [Bellevue, Casavant, op.3742.portable.zip](https://github.com/zurek-jiri/LibreOrgan/raw/main/libreorgans/bellevue-casavant/Bellevue,%20Casavant,%20op.3742.portable.zip) - *Load Configuration* in LibreOrgan Sampler opens the zip as it is (0.8.195) and copies the configuration into your LibreOrgans folder; or unpack it and put the `Bellevue, Casavant, op.3742.portable.json` there yourself.

**No samples are published here.** The configuration merely names the folders it expects;
the recordings themselves are the property of their producer and are obtained from them.

## The configuration

- **Organ:** Bellevue, Casavant.
- **Samples:** Sonus Paradisi - [https://www.sonusparadisi.cz/en/organs/u-s-a/casavant-organ-model.html](https://www.sonusparadisi.cz/en/organs/u-s-a/casavant-organ-model.html); the folders `002096`, `002097`, `002098`, `002099` and `002100` (as they come, or in their RAR archives); perspectives Front and Rear. LibreOrgan Sampler 0.9.28 or later.
- **Expects:** library root of any name, holding the trees `002096/grand`, `002096/ped`, `002096/rec`, `002097/ped`, `002097/pos`, `002097/rec`, `002098/grand`, `002098/ped`, `002099/pos` and `002100/rec`.
- **Audio layout:** every division has the routes front and rear; auxiliary (unused).
- **RAM:**
  - **4.8 GB** - 16-bit, 32 kHz, releases at half the rate, packed; *estimated*
  - **7.0 GB** - 16-bit, 48 kHz, full releases, packed; *measured*
  - **48 GB** - 32-bit float, the files' own rate, full releases, no packing; *estimated*
- **Saved with:** LibreOrgan Sampler 0.9.28.
- **Author:** Jiri Zurek, 1 November 2026.

## Where to put the file

Put `Bellevue, Casavant, op.3742.portable.json` into your LibreOrgans folder (Configuration Editor > *LibreOrgans folder*; `Documents\LibreOrgan\LibreOrgans` by default) and load it from there. The sample folders are found in your sample locations (the folders above your sample libraries, remembered by the program and listed in the Configuration Editor). One not found shows "Sample folder not found" under its division: click it and choose any folder of the samples (the tree, a folder above it, a neighbouring tree) - the program finds the tree, the other missing ones around it, and remembers the place for every configuration to come. The folders `002096`, `002097`, `002098`, `002099` and `002100` must stand side by side in one folder:

```
<a sample location>\
├── 002096\
│   ├── grand\
│   ├── ped\
│   └── rec\
├── 002097\
│   ├── ped\
│   ├── pos\
│   └── rec\
├── 002098\
│   ├── grand\
│   └── ped\
├── 002099\
│   └── pos\
└── 002100\
    └── rec\
```

## Stop List

| Pedal | I | II | III |
|---|---|---|---|
| Ped. Clairon 4' | GO. Clairon 4' | Pos. Clairon en chamade 4' | Rec. Clairon 4' |
| Ped. Clairon en chamade 4' (Trsm.) | GO. Trompette 8' | Pos. Cromorne 8' | Rec. Voix humaine 8' |
| Ped. Trompette 8' | GO. Trompette en chamade 8' | Pos. Trompette en chamade 8' (Trsm.) | Rec. Trompette 8' |
| Ped. Hautbois 4' (Trsm.) | GO. Bombarde 16' | Pos. Doucaine 16' | Rec. Hautbois 8' |
| Ped. Trombone 16' | GO. Fourniture V 1 1/3' | Pos. Bombarde en chamade 16' | Rec. Basson 16' |
| Ped. Trompette en chamade 8' (Trsm.) | GO. Cornet III 2 2/3' | Pos. Cymbale IV 2/3' | Rec. Trompette en chamade 8' (Trsm.) |
| Ped. Contretrombone 32' | GO. Doublette 2' | Pos. Sesquialtera II 2 2/3' | Rec. Pleinjeu V 2' |
| Ped. Doucaine 16' (Trsm.) | GO. Quinte majeure 2 2/3' | Pos. Doublette 2' | Rec. Piccolo 1' |
| Ped. Mixture V 2 2/3' | GO. Prestant 4' | Pos. Larigot 1 1/3' | Rec. Tierce 1 3/5' |
| Ped. Basson 16' (Trsm.) | GO. Flute ouverte 4' | Pos. Principal 4' | Rec. Quarte de nazard 2' |
| Ped. Octave 4' | GO. Violon 8' | Pos. Flute a fuseau 4' | Rec. Nazard 2 2/3' |
| Ped. Bombarde 16' (Trsm.) | GO. Flute harmonique 8' | Pos. Flute douce 8' | Rec. Octave 4' |
| Ped. Octavebass 8' | GO. Montre 8' | Pos. Flute celeste 8' | Rec. Flute a cheminee 4' |
| Ped. Theorbe III 10 2/3' | GO. Flute a cheminee 8' | Pos. Bourdon 8' | Rec. Viole de gambe 8' |
| Ped. Soubasse 16' | GO. Violonbasse 16' | Pos. Voce umana 8' | Rec. Voix celeste 8' |
| Ped. Flute ouverte 4' | GO. Chimes | Pos. Principal 8' | Rec. Diapason 8' |
| Ped. Montre 16' |  |  | Rec. Flute majeure 8' |
| Ped. Flute bouchee 8' |  |  | Rec. Bourdon 16' |
| Ped. Soubasse 32' |  |  |  |
| Ped. Flute a cheminee 8' (Trsm.) |  |  |  |
| Ped. Bourdon 16' (Trsm.) |  |  |  |
| Ped. Contrebasse 16' |  |  |  |
| Ped. Violonbasse 16' (Trsm.) |  |  |  |
| Ped. Bourdon 32' |  |  |  |

<!-- published by @zurek-jiri (issue #144, 2026-11-01) -->
