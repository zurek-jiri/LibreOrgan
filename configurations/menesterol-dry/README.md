# Menesterol, Dry

<!-- Written by OrganSampler. To publish it: attach the zip beside this README (it holds the configuration and this README) to the form (a free GitHub account):
     https://github.com/zurek-jiri/OrganSampler/issues/new?template=organ_configuration.yml -->

A complete OrganSampler configuration of Menesterol, Dry, as recorded by Sonus Paradisi in one perspective, Dry, each on its own audio route: 3 divisions and 23 rows in the Stop List, transmissions and
extensions included (only the rows nested in stop folders are not counted). Whoever owns
the recordings loads the whole instrument in a single step.

**Download:** [memesdry.portable.zip](https://github.com/zurek-jiri/OrganSampler/raw/main/configurations/menesterol-dry/memesdry.portable.zip) - *Load Configuration* in OrganSampler opens the zip as it is (0.8.195) and copies the configuration into your configurations folder; or unpack it and put the `memesdry.portable.json` there yourself.

**No samples are published here.** The configuration merely names the folders it expects;
the recordings themselves are the property of their producer and are obtained from them.

## The configuration

- **Organ:** Menesterol, Dry.
- **Samples:** Sonus Paradisi - [nevime](nevime); the folders `000880` and `000881` (as they come, or in their RAR archives); perspectives Dry. OrganSampler 0.8.216 or later.
- **Expects:** library root of any name, holding the trees `000880/pipe/pist/GrandOrgue`, `000880/pipe/pist/recit`, `000881/pipe/pist/GrandOrgue`, `000881/pipe/pist/pedal` and `000881/pipe/pist/recit`.
- **Audio layout:** every division has the routes dry; auxiliary (unused).
- **RAM:**
  - **0.9 GB** - 16-bit, 32 kHz, releases at half the rate, packed; *estimated*
  - **3.2 GB** - 16-bit, 48 kHz, full releases, no packing; *measured*
  - **9.2 GB** - 32-bit float, the files' own rate, full releases, no packing; *estimated*
- **Saved with:** OrganSampler 0.8.216.
- **Author:** koro, 27 October 2026.

## Where to put the file

Put `memesdry.portable.json` into your configurations folder (Configuration Editor > *Configurations folder*; `Documents\OrganSampler\Configurations` by default) and load it from there. The sample folders are found in your sample locations (the folders above your sample libraries, remembered by the program and listed in the Configuration Editor). One not found shows "Sample folder not found" under its division: click it and choose any folder of the samples (the tree, a folder above it, a neighbouring tree) - the program finds the tree, the other missing ones around it, and remembers the place for every configuration to come. The folders `000880` and `000881` must stand side by side in one folder:

```
<a sample location>\
├── 000880\
│   ├── pipe\pist\GrandOrgue\
│   └── pipe\pist\recit\
└── 000881\
    ├── pipe\pist\GrandOrgue\
    ├── pipe\pist\pedal\
    └── pipe\pist\recit\
```

## Stop List

| Pedal | I | II |
|---|---|---|
| Ped. Soubasse 16' | G.O. Bourdon 8' | Rec. Bourdon 8' |
| Ped. Flute 8' | G.O. Montre 8' | Rec. Prestant 4' |
| Ped. Principal 4' | G.O. Nasard 2 2/3' | Rec. Flute 4' |
| Ped. Flute 2' | G.O. Prestant 4' | Rec. Doublette 2' |
| Ped. Posaune 16' | G.O. Flute 2' | Rec. Quinte 1 1/3' |
|  | G.O. Bourdon 16' | Rec. Sifflet 1' |
|  | G.O. Plein Jeu 1 1/3' | Rec. Dulzian 8' |
|  | G.O. Sesquialtera 1 1/3' | Rec. Cymbale 2/3' |
|  | G.O. Trompette B 8' |  |
|  | G.O. Trompette D 8' (Ext.) |  |

<!-- published by @zurek-jiri (issue #111, 2026-10-27) -->
