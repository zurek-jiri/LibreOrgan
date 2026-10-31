# Bückeburg, Janke Organ

<!-- Written by LibreOrgan Sampler. To publish it: attach the zip beside this README (it holds the configuration and this README) to the form (a free GitHub account):
     https://github.com/zurek-jiri/LibreOrgan/issues/new?template=organ_configuration.yml -->

A complete LibreOrgan Sampler configuration of Bückeburg, Janke Organ, as recorded by Sonus Paradisi in 3 perspectives, Direct, Diffuse and Rear, each on its own audio route: 4 divisions and 47 rows in the Stop List, transmissions and
extensions included (only the rows nested in stop folders are not counted). Whoever owns
the recordings loads the whole instrument in a single step.

**Download:** [Buckeburg, Janke Organ.portable.zip](https://github.com/zurek-jiri/LibreOrgan/raw/main/libreorgans/b-ckeburg-janke-organ/Buckeburg,%20Janke%20Organ.portable.zip) - *Load Configuration* in LibreOrgan Sampler opens the zip as it is (0.8.195) and copies the configuration into your LibreOrgans folder; or unpack it and put the `Buckeburg, Janke Organ.portable.json` there yourself.

**No samples are published here.** The configuration merely names the folders it expects;
the recordings themselves are the property of their producer and are obtained from them.

## The configuration

- **Organ:** Bückeburg, Janke Organ.
- **Samples:** Sonus Paradisi - [https://www.sonusparadisi.cz/en/organs/germany/buckeburg-janke-organ.html](https://www.sonusparadisi.cz/en/organs/germany/buckeburg-janke-organ.html); the folders `002306` and `002307` (as they come, or in their RAR archives); perspectives Direct, Diffuse and Rear. LibreOrgan Sampler 0.9.17 or later.
- **Expects:** library root of any name, holding the trees `002306/pipes/Buckeburg_1noc_PED`, `002306/pipes/Buckeburg_3noc_OW`, `002306/pipes/Buckeburg_4noc_UW`, `002306/pipes/Buckeburg_5noc_HW`, `002307/pipes/Buckeburg_1noc_PED` and `002307/pipes/Buckeburg_4noc_UW`.
- **Audio layout:** every division has the routes direct, diffuse and rear; auxiliary (unused).
- **RAM:**
  - **5.9 GB** - 16-bit, 32 kHz, releases at half the rate, packed; *measured*
  - **59 GB** - 32-bit float, the files' own rate, full releases, no packing; *estimated*
- **Saved with:** LibreOrgan Sampler 0.9.17.
- **Author:** Jiri Zurek, 31 October 2026.

## Where to put the file

Put `Buckeburg, Janke Organ.portable.json` into your LibreOrgans folder (Configuration Editor > *LibreOrgans folder*; `Documents\LibreOrgan\LibreOrgans` by default) and load it from there. The sample folders are found in your sample locations (the folders above your sample libraries, remembered by the program and listed in the Configuration Editor). One not found shows "Sample folder not found" under its division: click it and choose any folder of the samples (the tree, a folder above it, a neighbouring tree) - the program finds the tree, the other missing ones around it, and remembers the place for every configuration to come. The folders `002306` and `002307` must stand side by side in one folder:

```
<a sample location>\
├── 002306\
│   ├── pipes\Buckeburg_1noc_PED\
│   ├── pipes\Buckeburg_3noc_OW\
│   ├── pipes\Buckeburg_4noc_UW\
│   └── pipes\Buckeburg_5noc_HW\
└── 002307\
    ├── pipes\Buckeburg_1noc_PED\
    └── pipes\Buckeburg_4noc_UW\
```

## Stop List

| Pedal | I | II | III |
|---|---|---|---|
| Ped Untersatz 32' | HW Bordun 16' | OW Quintadena 16' | BW Principal 4' |
| Ped Principal 16' | HW Viola da gamba 8' | OW Salicional 8' | BW Quintadena 8' |
| Ped Subbass 16' | HW Principal 8' | OW Principal 8' | BW Doppelegedackt 8' |
| Ped Octavbass 8' | HW Holzflote 8' | OW Rohrflote 8' | BW Traversflote 8' |
| Ped Gemshorn 8' | HW Octave 4' | OW Octave 4' | BW Holzflote 4' |
| Ped Octave 4' | HW Gemshorn 4' | OW Undamaris 8' | BW Quintflote 2 2/3' |
| Ped Mixtur 2 2/3' | HW Quinta 2 2/3' | OW Octave 2' | BW Hohlflote 2' |
| Ped Posaune 16' | HW Tertia 1 3/5' | OW Flote 4' | BW Terzflote 1 3/5' |
| Ped Trompete 8' | HW Octave 2' | OW Quinta 1 1/3' | BW Sifflote 1' |
| Ped Trompete 4' | HW Cornet 4f 4' | OW Nasat 2 2/3' | BW Regal 8' |
|  | HW Mixtur 1 1/3' | OW Mixtur 1' |  |
|  | HW Fagott 16' | OW Waldflote 2' |  |
|  | HW Trompete 8' | OW Sesquialtera 1 1/3' |  |
|  |  | OW Voxhumana 8' |  |

<!-- published by @zurek-jiri (issue #130, 2026-10-31) -->
