# Este, St. Maria delle Grazie, Mascioni Organ

<!-- Written by LibreOrgan Sampler. To publish it: attach the zip beside this README (it holds the configuration and this README) to the form (a free GitHub account):
     https://github.com/zurek-jiri/LibreOrgan/issues/new?template=organ_configuration.yml -->

A complete LibreOrgan Sampler configuration of Este, St. Maria delle Grazie, Mascioni Organ, as recorded by Sonus Paradisi in 4 perspectives, Direct, Diffuse, Distant and Rear, each on its own audio route: 3 divisions and 28 rows in the Stop List, transmissions and
extensions included (only the rows nested in stop folders are not counted). Whoever owns
the recordings loads the whole instrument in a single step.

**Download:** [Este, St. Maria, Mascioni.portable.zip](https://github.com/zurek-jiri/LibreOrgan/raw/main/libreorgans/este-st-maria-delle-grazie-mascioni-organ/Este,%20St.%20Maria,%20Mascioni.portable.zip) - *Load Configuration* in LibreOrgan Sampler opens the zip as it is (0.8.195) and copies the configuration into your LibreOrgans folder; or unpack it and put the `Este, St. Maria, Mascioni.portable.json` there yourself.

**No samples are published here.** The configuration merely names the folders it expects;
the recordings themselves are the property of their producer and are obtained from them.

## The configuration

- **Organ:** Este, St. Maria delle Grazie, Mascioni Organ.
- **Samples:** Sonus Paradisi - [https://www.sonusparadisi.cz/en/organs/italy/este-st-maria-delle-grazie-mascioni-organ.html](https://www.sonusparadisi.cz/en/organs/italy/este-st-maria-delle-grazie-mascioni-organ.html); the folders `002709` and `002710` (as they come, or in their RAR archives); perspectives Direct (`02-dirMk22-SW`), Diffuse (`03-difMk2S-SW`), Distant (`04-distMk2H-SW`) and Rear (`05-rearSenn-SW`). LibreOrgan Sampler 0.9.53 or later.
- **Expects:** library root of any name, holding the trees `002709/pipes/GO`, `002709/pipes/PE`, `002709/pipes/SW`, `002710/pipes/GO`, `002710/pipes/PE` and `002710/pipes/SW`.
- **Audio layout:** every division has the routes direct, diffuse, distant and rear; auxiliary (unused).
- **RAM:**
  - **7.1 GB** - 16-bit, 32 kHz, releases at half the rate, packed; *estimated*
  - **8.5 GB** - 16-bit, 48 kHz, releases at half the rate, packed; *measured*
  - **71 GB** - 32-bit float, the files' own rate, full releases, no packing; *estimated*
- **Saved with:** LibreOrgan Sampler 0.9.53.
- **Author:** Jiri Zurek, 4 November 2026.

## Where to put the file

Put `Este, St. Maria, Mascioni.portable.json` into your LibreOrgans folder (Configuration Editor > *LibreOrgans folder*; `Documents\LibreOrgan\LibreOrgans` by default) and load it from there. The sample folders are found in your sample locations (the folders above your sample libraries, remembered by the program and listed in the Configuration Editor). One not found shows "Sample folder not found" under its division: click it and choose any folder of the samples (the tree, a folder above it, a neighbouring tree) - the program finds the tree, the other missing ones around it, and remembers the place for every configuration to come. The folders `002709` and `002710` must stand side by side in one folder:

```
<a sample location>\
├── 002709\
│   ├── pipes\GO\
│   ├── pipes\PE\
│   └── pipes\SW\
└── 002710\
    ├── pipes\GO\
    ├── pipes\PE\
    └── pipes\SW\
```

## Stop List

| Pedal | I | II |
|---|---|---|
| PED Contrabasso 16' | GO Principale 16' | REC Principale 8' |
| PED Subbasso 16' | GO Unda maris 8' | REC Bordone 8' |
| PED Principale 8' | GO Principale 8' | REC Viola 8' |
| PED Basso armonico 8' | GO Flauto 8' | REC Flauto camino 4' |
| PED Tromba 16' | GO Dulciana 8' | REC Ottava 4' |
| PED Bordone 8' | GO Ottava 4' | REC Cornetto 2' |
| PED Tromba 8' | GO Flauto 4' | REC Ripieno 2' |
| PED Trombina 4' | GO Ripieno 2 file 2' | REC Clarinetto 8' |
|  | GO Sesquialtera 2 2/3' | REC Coro viole 8' [II] |
|  | GO Ripieno 1' |  |
|  | GO Tromba 8' |  |

<!-- published by @zurek-jiri (issue #201, 2026-11-04) -->
