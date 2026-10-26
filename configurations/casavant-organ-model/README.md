# Casavant Organ Model

<!-- Written by OrganSampler. To publish it: attach this README and the zip beside it to the form (a free GitHub account):
     https://github.com/zurek-jiri/OrganSampler/issues/new?template=organ_configuration.yml -->

A complete OrganSampler configuration of Casavant Organ Model, as recorded by Sonus Paradisi in 2 perspectives, Front and Rear, each on its own audio route: 4 divisions and 74 rows in the Stop List, transmissions and
extensions included (only the rows nested in stop folders are not counted). Whoever owns
the recordings loads the whole instrument in a single step.

**Download:** [Casavant, Bellevue, Surround.portable.zip](https://github.com/zurek-jiri/OrganSampler/raw/main/configurations/casavant-organ-model/Casavant,%20Bellevue,%20Surround.portable.zip) - *Load Configuration* in OrganSampler opens the zip as it is (0.8.195) and copies the configuration into your configurations folder; or unpack it and put the `Casavant, Bellevue, Surround.portable.json` there yourself.

**No samples are published here.** The configuration merely names the folders it expects;
the recordings themselves are the property of their producer and are obtained from them.

## The configuration

- **Organ:** Casavant Organ Model.
- **Samples:** Sonus Paradisi - [https://www.sonusparadisi.cz/en/organs/u-s-a/casavant-organ-model.html](https://www.sonusparadisi.cz/en/organs/u-s-a/casavant-organ-model.html); the folders `002096`, `002097`, `002098`, `002099` and `002100` (as they come, or in their RAR archives); perspectives Front and Rear. OrganSampler 0.8.212 or later.
- **Expects:** library root of any name, holding the trees `002096/grand`, `002096/ped`, `002096/rec`, `002097/ped`, `002097/pos`, `002097/rec`, `002098/grand`, `002098/ped`, `002099/pos` and `002100/rec`.
- **Audio layout:** every division has the routes front and rear; auxiliary (unused).
- **RAM:**
  - **4.9 GB** - 16-bit, 32 kHz, releases at half the rate, packed; *estimated*
  - **7.1 GB** - 16-bit, 48 kHz, full releases, packed; *measured*
  - **49 GB** - 32-bit float, the files' own rate, full releases, no packing; *estimated*
- **Saved with:** OrganSampler 0.8.212.
- **Author:** Jiri Zurek, 26 October 2026.

## Where to put the file

Put `Casavant, Bellevue, Surround.portable.json` into your configurations folder (Configuration Editor > *Configurations folder*; `Documents\OrganSampler\Configurations` by default) and load it from there. The sample folders are found in your sample locations (the folders above your sample libraries, remembered by the program and listed in the Configuration Editor). One not found shows "Sample folder not found" under its division: click it and choose any folder of the samples (the tree, a folder above it, a neighbouring tree) - the program finds the tree, the other missing ones around it, and remembers the place for every configuration to come. The folders `002096`, `002097`, `002098`, `002099` and `002100` must stand side by side in one folder:

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
| Ped. Soubasse 32' | GO. Violonbasse 16' | Pos. Principal 8' | Rec. Bourdon 16' |
| Ped. Bourdon 32' | GO. Chimes | Pos. Bourdon 8' | Rec. Diapason 8' |
| Ped. Montre 16' | GO. Montre 8' | Pos. Voce umana 8' | Rec. Flute majeure 8' |
| Ped. Contrebasse 16' | GO. Flute a cheminee 8' | Pos. Flute douce 8' | Rec. Viole de gambe 8' |
| Ped. Soubasse 16' | GO. Violon 8' | Pos. Flute celeste 8' | Rec. Voix celeste 8' |
| Ped. Violonbasse 16' (Trsm.) | GO. Flute harmonique 8' | Pos. Principal 4' | Rec. Octave 4' |
| Ped. Octavebass 8' | GO. Prestant 4' | Pos. Flute a fuseau 4' | Rec. Flute a cheminee 4' |
| Ped. Bourdon 16' (Trsm.) | GO. Flute ouverte 4' | Pos. Doublette 2' | Rec. Quarte de nazard 2' |
| Ped. Octave 4' | GO. Doublette 2' | Pos. Larigot 1 1/3' | Rec. Nazard 2 2/3' |
| Ped. Flute bouchee 8' | GO. Quinte majeure 2 2/3' | Pos. Cymbale IV 2/3' | Rec. Piccolo 1' |
| Ped. Mixture V 2 2/3' | GO. Fourniture V 1 1/3' | Pos. Sesquialtera II 2 2/3' | Rec. Tierce 1 3/5' |
| Ped. Flute a cheminee 8' (Trsm.) | GO. Cornet III 2 2/3' | Pos. Doucaine 16' | Rec. Pleinjeu V 2' |
| Ped. Contretrombone 32' | GO. Bombarde 16' | Pos. Bombarde en chamade 16' | Rec. Basson 16' |
| Ped. Flute ouverte 4' | GO. Trompette 8' | Pos. Cromorne 8' | Rec. Trompette en chamade 8' (Trsm.) |
| Ped. Trombone 16' | GO. Trompette en chamade 8' | Pos. Trompette en chamade 8' (Trsm.) | Rec. Trompette 8' |
| Ped. Theorbe III 10 2/3' | GO. Clairon 4' | Pos. Clairon en chamade 4' | Rec. Hautbois 8' |
| Ped. Trompette 8' |  |  | Rec. Clairon 4' |
| Ped. Bombarde 16' (Trsm.) |  |  | Rec. Voix humaine 8' |
| Ped. Clairon 4' |  |  |  |
| Ped. Doucaine 16' (Trsm.) |  |  |  |
| Ped. Basson 16' (Trsm.) |  |  |  |
| Ped. Trompette en chamade 8' (Trsm.) |  |  |  |
| Ped. Clairon en chamade 4' (Trsm.) |  |  |  |
| Ped. Hautbois 4' (Trsm.) |  |  |  |

<!-- published by @zurek-jiri (issue #107, 2026-10-26) -->
