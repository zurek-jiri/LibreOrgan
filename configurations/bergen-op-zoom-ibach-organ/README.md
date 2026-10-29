# Bergen op Zoom, Ibach organ

<!-- Written by OrganSampler. To publish it: attach the zip beside this README (it holds the configuration and this README) to the form (a free GitHub account):
     https://github.com/zurek-jiri/OrganSampler/issues/new?template=organ_configuration.yml -->

A complete OrganSampler configuration of Bergen op Zoom, Ibach organ, as recorded by Sonus Paradisi in 4 perspectives, Diffuse, Direct, Rear and Distant, each on its own audio route: 4 divisions and 54 rows in the Stop List, transmissions and
extensions included (only the rows nested in stop folders are not counted). Whoever owns
the recordings loads the whole instrument in a single step.

**Download:** [Bergen op Zoom Ibach organ Surround 8-chan.portable.zip](https://github.com/zurek-jiri/OrganSampler/raw/main/configurations/bergen-op-zoom-ibach-organ/Bergen%20op%20Zoom%20Ibach%20organ%20Surround%208-chan.portable.zip) - *Load Configuration* in OrganSampler opens the zip as it is (0.8.195) and copies the configuration into your configurations folder; or unpack it and put the `Bergen op Zoom Ibach organ Surround 8-chan.portable.json` there yourself.

**No samples are published here.** The configuration merely names the folders it expects;
the recordings themselves are the property of their producer and are obtained from them.

## The configuration

- **Organ:** Bergen op Zoom, Ibach organ.
- **Samples:** Sonus Paradisi - [https://www.sonusparadisi.cz/en/organs/netherlands/bergen-op-zoom-ibach-organ.html](https://www.sonusparadisi.cz/en/organs/netherlands/bergen-op-zoom-ibach-organ.html); the folders `002082`, `002083`, `002084`, `002085`, `002086`, `002087` and `002088` (as they come, or in their RAR archives); perspectives Diffuse, Direct, Rear and Distant (`06-far`). OrganSampler 0.8.224 or later.
- **Expects:** library root of any name, holding the trees `002082/pipes`, `002083/pipes`, `002084/pipes`, `002085/pipes`, `002086/pipes`, `002087/pipes` and `002088/pipes`.
- **Audio layout:** every division has the routes diffuse, direct, rear and distant; auxiliary (unused).
- **RAM:**
  - **8.3 GB** - 16-bit, 32 kHz, releases at half the rate, packed; *estimated*
  - **44 GB** - 16-bit, 48 kHz, full releases, no packing; *measured*
  - **83 GB** - 32-bit float, the files' own rate, full releases, no packing; *estimated*
- **Saved with:** OrganSampler 0.8.224.
- **Author:** Jiri Zurek, 29 October 2026.

## Where to put the file

Put `Bergen op Zoom Ibach organ Surround 8-chan.portable.json` into your configurations folder (Configuration Editor > *Configurations folder*; `Documents\OrganSampler\Configurations` by default) and load it from there. The sample folders are found in your sample locations (the folders above your sample libraries, remembered by the program and listed in the Configuration Editor). One not found shows "Sample folder not found" under its division: click it and choose any folder of the samples (the tree, a folder above it, a neighbouring tree) - the program finds the tree, the other missing ones around it, and remembers the place for every configuration to come. The folders `002082`, `002083`, `002084`, `002085`, `002086`, `002087` and `002088` must stand side by side in one folder:

```
<a sample location>\
├── 002082\
│   └── pipes\
├── 002083\
│   └── pipes\
├── 002084\
│   └── pipes\
├── 002085\
│   └── pipes\
├── 002086\
│   └── pipes\
├── 002087\
│   └── pipes\
└── 002088\
    └── pipes\
```

## Stop List

| Pedal | I | II | III |
|---|---|---|---|
| P. Principal 32' (Ext.) | 1. Principal 16' | 2. Bordun 16' | Salicet 16' (Ext.) |
| P. Violonbass 64' (Ext.) | 1. Quintaton 16' | 2. Bassethorn 8' | 3. Salicional 8' |
| P. Principal 16' | 1. Principal 8' | 2. Principal 8' | 3. Dolce 8' |
| P. Violonbass 16' | 1. Gemshorn 8' | 2. Rohrflaut 8' | 3. Flaut angelika 8' |
| P. Octavbass 8' | 1. Octav 4' | 2. Octav 4' | 3. Quintaton 8' |
| P. Subbass 16' | 1. Viola di gamba 8' | 2. Flautdolce 4' | 3. Viola 4' |
| P. Quintbass 5 1/3' | 1. Quint 2 2/3' | 2. Mixtur 1' | 3. Flaut travers 4' |
| P. Gedaktbass 8' | 1. Grossgedakt 8' | Nasat 2 2/3' (Ext.) | 3. Euphone 8' |
| P. Octavbass 4' | 1. Octav 2' | 2. Fagottoboe 8' | 3. Flageolet 2' |
| P. Jubal 8' | 1. Hohlflaut 4' | 2. Flautino 2' | 3. Euphone 16' (Ext.) |
| P. Posaunenbass 32' (Ext.) | 1. Scharff 2' | Carillon [III] |  |
| Cello 8' (Trsm.) | Terz 1 3/5' (Ext.) |  |  |
| P. Posaunenbass 16' | 1. Fagott 16' |  |  |
| 1. Trompete 16' (Trsm.) | 1. Cornett 4' |  |  |
| P. Claironbass 4' | 1. Trompete 16' (Ext.) |  |  |
| P. Trompetenbass 8' | 1. Trompete 8' |  |  |
|  | Clairon 4' (Ext.) |  |  |

<!-- published by @zurek-jiri (issue #118, 2026-10-29) -->
