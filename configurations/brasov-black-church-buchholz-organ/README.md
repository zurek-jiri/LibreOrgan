# Brasov, Black Church, Buchholz Organ

<!-- Written by OrganSampler. To publish it: attach the zip beside this README (it holds the configuration and this README) to the form (a free GitHub account):
     https://github.com/zurek-jiri/OrganSampler/issues/new?template=organ_configuration.yml -->

A complete OrganSampler configuration of Brasov, Black Church, Buchholz Organ, as recorded by Sonus Paradisi in 3 perspectives, Diffuse, Direct and Rear, each on its own audio route: 5 divisions and 63 rows in the Stop List, transmissions and
extensions included (only the rows nested in stop folders are not counted). Whoever owns
the recordings loads the whole instrument in a single step.

**Download:** [Brasov Surround.portable.zip](https://github.com/zurek-jiri/OrganSampler/raw/main/configurations/brasov-black-church-buchholz-organ/Brasov%20Surround.portable.zip) - *Load Configuration* in OrganSampler opens the zip as it is (0.8.195) and copies the configuration into your configurations folder; or unpack it and put the `Brasov Surround.portable.json` there yourself.

**No samples are published here.** The configuration merely names the folders it expects;
the recordings themselves are the property of their producer and are obtained from them.

## The configuration

- **Organ:** Brasov, Black Church, Buchholz Organ.
- **Samples:** Sonus Paradisi - [https://www.sonusparadisi.cz/en/organs/germany/bra.html](https://www.sonusparadisi.cz/en/organs/germany/bra.html); the folders `001722`, `001723`, `001724`, `001725`, `001726`, `001727`, `001728` and `001729` (as they come, or in their RAR archives); perspectives Diffuse, Direct and Rear. OrganSampler 0.8.226 or later.
- **Expects:** library root of any name, holding the trees `001722/_II`, `001723/_I`, `001723/_III`, `001723/_IV`, `001723/_P`, `001724/_I`, `001725/_II`, `001726/_III`, `001727/_IV`, `001728/_P`, `001729/_I` and `001729/_P`.
- **Audio layout:** every division has the routes diffuse, direct and rear; auxiliary (unused).
- **RAM:**
  - **5.5 GB** - 16-bit, 32 kHz, releases at half the rate, packed; *estimated*
  - **15 GB** - 32-bit float, 32 kHz, full releases, packed; *measured*
  - **55 GB** - 32-bit float, the files' own rate, full releases, no packing; *estimated*
- **Saved with:** OrganSampler 0.8.226.
- **Author:** Jiri Zurek, 29 October 2026.

## Where to put the file

Put `Brasov Surround.portable.json` into your configurations folder (Configuration Editor > *Configurations folder*; `Documents\OrganSampler\Configurations` by default) and load it from there. The sample folders are found in your sample locations (the folders above your sample libraries, remembered by the program and listed in the Configuration Editor). One not found shows "Sample folder not found" under its division: click it and choose any folder of the samples (the tree, a folder above it, a neighbouring tree) - the program finds the tree, the other missing ones around it, and remembers the place for every configuration to come. The folders `001722`, `001723`, `001724`, `001725`, `001726`, `001727`, `001728` and `001729` must stand side by side in one folder:

```
<a sample location>\
├── 001722\
│   └── _II\
├── 001723\
│   ├── _I\
│   ├── _III\
│   ├── _IV\
│   └── _P\
├── 001724\
│   └── _I\
├── 001725\
│   └── _II\
├── 001726\
│   └── _III\
├── 001727\
│   └── _IV\
├── 001728\
│   └── _P\
└── 001729\
    ├── _I\
    └── _P\
```

## Stop List

| Pedal | I | II | III | IV |
|---|---|---|---|---|
| P. Untersatz 32' | U. Salicional 16' | R. Rohrfloete 8' | M. Principal 16' | O. Bourdon 16' |
| P. Principal 32' | U. Principal 8' | R. Violino 8' | M. Quintatoen 16' | O. Principal 8' |
| P. Principal 16' | U. Gedact 8' | R. Principal 4' | M. Principal 8' | O. Salicional 8' |
| P. Subbass 16' | U. Flauto traverso 8' | R. Vox angelica 8' | M. Gemshorn 8' | O. Hohlfloete 8' |
| P. Violone 16' | U. Viola da Gamba 8' | R. Fagott 16' | M. Viola da Gamba 8' | O. Gedact 8' |
| P. Nasard 10 2/3' | U. Octava 4' | R. Trompete 8' | M. Rohrfloete 8' | O. Quintatoen 8' |
| P. Principal 8' | U. Flauto dolce 4' | R. Clarinetto 8' | M. Nasard 5 1/3' | O. Octava 4' |
| P. Violone 8' | U. Viole d Amour 4' |  | M. Octava 4' | O. Fugara 4' |
| P. Bassfloete 8' | U. Gemshorn 2 2/3' |  | M. Spitzfloete 4' | O. Rohrfloete 4' |
| P. Gemshorn 8' | U. Decimaquinta 2' |  | M. Waldfloete 4' | O. Superoctava 2' |
| P. Octava 4' | U. Progressio harmonica 4' |  | M. Quinta 2 2/3' | O. Nasard 2 2/3' |
| P. Quinta 5 1/3' |  |  | M. Superoctava 2' | O. Mixtur 2' |
| P. Mixtur 4' |  |  | M. Cornett 8' | O. Hautbois 8' |
| P. Contraposaune 32' |  |  | M. Scharff 2' |  |
| P. Posaune 16' |  |  | M. Cimbel 1 1/3' |  |
| P. Trompete 8' |  |  |  |  |
| P. Cornetta 4' |  |  |  |  |

<!-- published by @zurek-jiri (issue #126, 2026-10-29) -->
