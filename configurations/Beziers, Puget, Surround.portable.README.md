# Beziers Cathedral Puget Organ

<!-- Written by OrganSampler. To publish it: attach the zip beside this README (it holds the configuration and this README) to the form (a free GitHub account):
     https://github.com/zurek-jiri/OrganSampler/issues/new?template=organ_configuration.yml -->

A complete OrganSampler configuration of Beziers Cathedral Puget Organ, as recorded by Sonus Paradisi in 3 perspectives, Diffuse, Direct and Rear, each on its own audio route: 4 divisions and 56 rows in the Stop List, transmissions and
extensions included (only the rows nested in stop folders are not counted). Whoever owns
the recordings loads the whole instrument in a single step.

**Download:** [Beziers, Puget, Surround.portable.zip](https://github.com/zurek-jiri/OrganSampler/raw/main/configurations/beziers-cathedral-puget-organ/Beziers,%20Puget,%20Surround.portable.zip) - *Load Configuration* in OrganSampler opens the zip as it is (0.8.195) and copies the configuration into your configurations folder; or unpack it and put the `Beziers, Puget, Surround.portable.json` there yourself.

**No samples are published here.** The configuration merely names the folders it expects;
the recordings themselves are the property of their producer and are obtained from them.

## The configuration

- **Organ:** Beziers Cathedral Puget Organ.
- **Samples:** Sonus Paradisi - [https://www.sonusparadisi.cz/en/organs/france/beziers-cathedral-puget-organ.html](https://www.sonusparadisi.cz/en/organs/france/beziers-cathedral-puget-organ.html); the folders `002721` and `002722` (as they come, or in their RAR archives); perspectives Diffuse, Direct and Rear. OrganSampler 0.8.224 or later.
- **Expects:** library root of any name, holding the trees `002721/pipes/GO`, `002721/pipes/RE`, `002722/pipes/GO`, `002722/pipes/PE`, `002722/pipes/PO` and `002722/pipes/RE`.
- **Audio layout:** every division has the routes diffuse, direct and rear; auxiliary (unused).
- **RAM:**
  - **7.8 GB** - 16-bit, 32 kHz, releases at half the rate, packed; *estimated*
  - **11 GB** - 16-bit, 48 kHz, full releases, packed; *measured*
  - **78 GB** - 32-bit float, the files' own rate, full releases, no packing; *estimated*
- **Saved with:** OrganSampler 0.8.224.
- **Author:** Jiri Zurek, 29 October 2026.

## Where to put the file

Put `Beziers, Puget, Surround.portable.json` into your configurations folder (Configuration Editor > *Configurations folder*; `Documents\OrganSampler\Configurations` by default) and load it from there. The sample folders are found in your sample locations (the folders above your sample libraries, remembered by the program and listed in the Configuration Editor). One not found shows "Sample folder not found" under its division: click it and choose any folder of the samples (the tree, a folder above it, a neighbouring tree) - the program finds the tree, the other missing ones around it, and remembers the place for every configuration to come. The folders `002721` and `002722` must stand side by side in one folder:

```
<a sample location>\
├── 002721\
│   ├── pipes\GO\
│   └── pipes\RE\
└── 002722\
    ├── pipes\GO\
    ├── pipes\PE\
    ├── pipes\PO\
    └── pipes\RE\
```

## Stop List

| Pedal | I | II | III |
|---|---|---|---|
| Ped. Flute de 32' (Ext.) | GO. Montre 16' | Pos. Montre 8' | Rec. Bourdon de 16' (Ext.) |
| Ped. Flute de 16' | GO. Bourdon de 16' | Pos. Keraulophone 8' | Rec. Bourdon de 8' |
| Soubasse 16' (Trsm.) | GO. Montre de 8' | Pos. Bourdon 8' | Rec. Gambe 8' |
| Contrebasse 16' (Trsm.) | GO. Gambe 8' | Pos. Unda maris 8' | Rec. Flute harmon. 8' |
| Ped. Flute de 8' | GO. Bourdon de 8' | Pos. Prestant 4' | Rec. Voix celeste 8' |
| Bourdon 8' (Trsm.) | GO. Solicional 8' | Pos. Flute 4' | Rec. Flute octaviante 4' |
| Cello 8' (Trsm.) | GO. Prestant 4' | Pos. Doublette 2' | Rec. Octavin 2' |
| Ped. Flute de 4' | GO. Gros nasard 5 1/3' | Pos. Nazard 2 2/3' | Bombarde 16' (Ext.) |
| Ped. Bombarde 32' (Ext.) | GO. Quinte 2 2/3' | Larigot 1 1/3' (Ext.) | Rec. Clarinette 8' |
| Ped. Bombarde 16' | GO. Doublette 2' | Piccolo 1' (Ext.) | Rec. Trompete 8' |
| Ped. Trompete 8' | GO. Fourniture progr. 2' | Pos. Fourniture 7rgs 1 1/3' | Rec. Hautbois et basson 8' |
| Ped. Clairon 4' | GO. Gran cornet 8' | Tierce 1 3/5' (Ext.) | Rec. Clairon 4' |
|  | GO. Bombarde 16' | Pos. Trompete 8' | Rec. Vox humana 8' |
|  | GO. 1ere Trompette 8' | Pos. Cromorne 8' |  |
|  | GO. 2eme Trompette 8' | Pos. Clairon 4' |  |
|  | GO. Clairon 4' |  |  |

<!-- published by @zurek-jiri (issue #122, 2026-10-29) -->
