# Dom Bédos Organ Model

<!-- Written by LibreOrgan Sampler. To publish it: attach the zip beside this README (it holds the configuration and this README) to the form (a free GitHub account):
     https://github.com/zurek-jiri/LibreOrgan/issues/new?template=organ_configuration.yml -->

A complete LibreOrgan Sampler configuration of Dom Bédos Organ Model, as recorded by Sonus Paradisi in 2 perspectives, Rear and default, each on its own audio route: 6 divisions and 56 rows in the Stop List, transmissions and
extensions included (only the rows nested in stop folders are not counted). Whoever owns
the recordings loads the whole instrument in a single step.

**Download:** [Dom Bedos Organ Model.portable.zip](https://github.com/zurek-jiri/LibreOrgan/raw/main/libreorgans/dom-b-dos-organ-model/Dom%20Bedos%20Organ%20Model.portable.zip) - *Load Configuration* in LibreOrgan Sampler opens the zip as it is (0.8.195) and copies the configuration into your LibreOrgans folder; or unpack it and put the `Dom Bedos Organ Model.portable.json` there yourself.

**No samples are published here.** The configuration merely names the folders it expects;
the recordings themselves are the property of their producer and are obtained from them.

## The configuration

- **Organ:** Dom Bédos Organ Model.
- **Samples:** Sonus Paradisi - [https://www.sonusparadisi.cz/en/organs/france/dom-bedos-organ-model.html](https://www.sonusparadisi.cz/en/organs/france/dom-bedos-organ-model.html); the folders `000862`, `000863`, `000864` and `000865` (as they come, or in their RAR archives); perspectives Rear and default. LibreOrgan Sampler 0.8.219 or later.
- **Expects:** library root of any name, holding the trees `000862/pipe/echo_fr`, `000862/pipe/positiv_fr`, `000862/pipe/recit_fr`, `000862/pipe/reson_fr`, `000863/pipe/go_fr`, `000863/pipe/pedal_fr`, `000864/pipe/echo_re`, `000864/pipe/positiv_re`, `000864/pipe/recit_re`, `000864/pipe/reson_re`, `000865/pipe/go_re` and `000865/pipe/pedal_re`.
- **Audio layout:** every division has the routes rear and default; auxiliary (unused).
- **RAM:**
  - **4.2 GB** - 16-bit, 32 kHz, releases at half the rate, packed; *estimated*
  - **19 GB** - 16-bit, 48 kHz, full releases, no packing; *measured*
  - **42 GB** - 32-bit float, the files' own rate, full releases, no packing; *estimated*
- **Saved with:** LibreOrgan Sampler 0.8.219.
- **Author:** Jiri Zurek, 27 October 2026.

## Where to put the file

Put `Dom Bedos Organ Model.portable.json` into your LibreOrgans folder (Configuration Editor > *LibreOrgans folder*; `Documents\LibreOrgan\LibreOrgans` by default) and load it from there. The sample folders are found in your sample locations (the folders above your sample libraries, remembered by the program and listed in the Configuration Editor). One not found shows "Sample folder not found" under its division: click it and choose any folder of the samples (the tree, a folder above it, a neighbouring tree) - the program finds the tree, the other missing ones around it, and remembers the place for every configuration to come. The folders `000862`, `000863`, `000864` and `000865` must stand side by side in one folder:

```
<a sample location>\
├── 000862\
│   ├── pipe\echo_fr\
│   ├── pipe\positiv_fr\
│   ├── pipe\recit_fr\
│   └── pipe\reson_fr\
├── 000863\
│   ├── pipe\go_fr\
│   └── pipe\pedal_fr\
├── 000864\
│   ├── pipe\echo_re\
│   ├── pipe\positiv_re\
│   ├── pipe\recit_re\
│   └── pipe\reson_re\
└── 000865\
    ├── pipe\go_re\
    └── pipe\pedal_re\
```

## Stop List

| Pedal | I | II | III | IV | V |
|---|---|---|---|---|---|
| Ped. Bombarde 16' | Pos. Bourdon 8' | GO. Bourdon 16' | Res. Trompette de bataille 8' | Rec. Cornet V 8' | Ech. Bourdon 8' |
| Ped. Clairon 4' | Pos. Cymbale III 1/2' | GO. Bourdon 8' | Res. Bombarde 16' | Rec. Flute 8' | Ech. Doublette 2' |
| Ped. Gros Cromorne 8' | Pos. Clairon 4' | GO. Cymbale VI 1' | Res. Clairon 4' | Rec. Hautbois 8' | Ech. Fourniture 3 rg. 2/3' |
| Ped. Flute 16' | Pos. Cornet 5 rg. 8' | GO. Clairon 4' |  | Rec. Trompette 8' | Ech. Musette 8' |
| Ped. Flute 4' | Pos. Cromorne 8' | GO. Grand Cornet 5 rg. 8' |  |  | Ech. Nazard 2 2/3' |
| Ped. Flute 8' | Pos. Doublette 2' | GO. Doublette 2' |  |  | Ech. Prestant 4' |
| Ped. Trompette I 8' | Pos. Flute a Cheminee 4' | GO. Flute 8' |  |  | Ech. Tierce 1 3/5' |
| Ped. Trompette II 8' | Pos. Fourniture IV 1 1/3' | GO. Fourniture V 2' |  |  |  |
|  | Pos. Larigot 1 1/3' | GO. Grosse Tierce 3 1/5' |  |  |  |
|  | Pos. Montre 8' | GO. Grosse Fourniture III 4' |  |  |  |
|  | Pos. Nazard 2 2/3' | GO. Gross Nazard 5 1/3' |  |  |  |
|  | Pos. Prestant 4' | GO. Montre 8' |  |  |  |
|  | Pos. Quarte de Nazard 2' | GO. Montre 32' |  |  |  |
|  | Pos. Tierce 1 3/5' | GO. Montre 8' |  |  |  |
|  | Pos. Trompette 8' | GO. Nazard 2 2/3' |  |  |  |
|  |  | GO. Prestant 4' |  |  |  |
|  |  | GO. Tierce 1 3/5' |  |  |  |
|  |  | GO. Trompette 8' |  |  |  |
|  |  | GO. Voix Humaine 8' |  |  |  |

<!-- published by @zurek-jiri (issue #116, 2026-10-27) -->
