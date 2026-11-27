# Altenbruch, Klapmeyer Organ

<!-- Written by LibreOrgan Sampler. To publish it: attach the zip beside this README (it holds the configuration and this README) to the form (a free GitHub account):
     https://github.com/zurek-jiri/LibreOrgan/issues/new?template=organ_configuration.yml -->

A complete LibreOrgan Sampler configuration of Altenbruch, Klapmeyer Organ, as recorded by Sonus Paradisi in one perspective, default, each on its own audio route: 4 divisions and 37 rows in the Stop List, transmissions and
extensions included (only the rows nested in stop folders are not counted). Whoever owns
the recordings loads the whole instrument in a single step.

**Download:** [Altenbruch SemiDry Extended.portable.zip](https://github.com/zurek-jiri/LibreOrgan/raw/main/libreorgans/altenbruch-klapmeyer-organ/Altenbruch%20SemiDry%20Extended.portable.zip) - *Load Configuration* in LibreOrgan Sampler opens the zip as it is and copies the configuration into your LibreOrgans folder; or unpack it and put the `Altenbruch SemiDry Extended.portable.json` there yourself.

**No samples are published here.** The configuration merely names the folders it expects;
the recordings themselves are the property of their producer and are obtained from them.

## The configuration

- **Organ:** Altenbruch, Klapmeyer Organ.
- **Samples:** Sonus Paradisi - [https://www.sonusparadisi.cz/en/organs/germany/klapmeyer-organ-altenbruch.html](https://www.sonusparadisi.cz/en/organs/germany/klapmeyer-organ-altenbruch.html); the folders `002067` and `002068` (as they come, or in their RAR archives); perspectives default. LibreOrgan Sampler 0.9.146 or later.
- **Expects:** library root of any name, holding the trees `002067/pipes/bw`, `002067/pipes/hw`, `002067/pipes/pedal` and `002068/pipes/rw`.
- **Audio layout:** every division has the routes default and noises; auxiliary (unused).
- **RAM:**
  - **2.1 GB** - 16-bit, 32 kHz, releases at half the rate, packed; *estimated*
  - **2.9 GB** - 16-bit, 48 kHz, full releases, packed; *measured*
  - **21 GB** - 32-bit float, the files' own rate, full releases, no packing; *estimated*
- **Saved with:** LibreOrgan Sampler 0.9.146.
- **Author:** Jiri Zurek, 27 November 2026.

## Where to put the file

Put `Altenbruch SemiDry Extended.portable.json` into your LibreOrgans folder (Configuration Editor > *LibreOrgans folder*; `Documents\LibreOrgan\LibreOrgans` by default) and load it from there. The sample folders are found in your sample locations (the folders above your sample libraries, remembered by the program and listed in the Configuration Editor). One not found shows "Sample folder not found" under its division: click it and choose any folder of the samples (the tree, a folder above it, a neighbouring tree) - the program finds the tree, the other missing ones around it, and remembers the place for every configuration to come. The folders `002067` and `002068` must stand side by side in one folder:

```
<a sample location>\
├── 002067\
│   ├── pipes\bw\
│   ├── pipes\hw\
│   └── pipes\pedal\
└── 002068\
    └── pipes\rw\
```

## Stop List

| Pedal | I | II | III |
|---|---|---|---|
| P. Untersatz 16' | R. Gedact 8' | O. Principahl 8' | B. Gedact 8' |
| P. Gedact 8' | R. Principahl 8' | O. Quintadohn 16' | B. Gedact 4' |
| P. Principahl 8' | R. Gedact 4' | O. Octav 4' | B. Octav 2' |
| P. Posaun 16' | R. Octav 4' | O. Gedact 8' | B. Quint 1 1/3' |
| P. Octav 4' | R. Mixtur 2/3' | O. Mixtur 1 1/3' | B. Scharff 1/2' |
| P. Trometh 8' | R. Quintadohn 8' | O. Waldtfloit 2' | B. Knopregal 8' |
| P. Mixtur 2' | R. Octav 2' | O. Cimbel 1/2' |  |
| P. Corneth 2' | R. Nasat 2 2/3' | O. Vox humana 8' |  |
| Blower On/Off | R. Blockfloit 2' | O. Trometh 8' |  |
| Cimbelstern | R. Sexquialtera 1 1/3' |  |  |
|  | R. Dulcian 16' |  |  |
|  | R. Kromhorn 8' |  |  |

<!-- published by @zurekjiri (issue #273, 2026-11-27) -->
