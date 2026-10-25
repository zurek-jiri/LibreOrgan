# Altenbruch, Klapmeyer Organ

<!-- Written by OrganSampler. To publish it: attach this README and the zip beside it to the form (a free GitHub account):
     https://github.com/zurek-jiri/OrganSampler/issues/new?template=organ_configuration.yml -->

A complete OrganSampler configuration of Altenbruch, Klapmeyer Organ, as recorded by Sonus Paradisi in one perspective, default, each on its own audio route: 4 divisions and 35 rows in the Stop List, transmissions and
extensions included (only the rows nested in stop folders are not counted). Whoever owns
the recordings loads the whole instrument in a single step.

**Download:** [Altenbruch SemiDry Extended.portable.zip](https://github.com/zurek-jiri/OrganSampler/raw/main/configurations/altenbruch-klapmeyer-organ/Altenbruch%20SemiDry%20Extended.portable.zip) - *Load Configuration* in OrganSampler opens the zip as it is (0.8.195) and copies the configuration into your configurations folder; or unpack it and put the `Altenbruch SemiDry Extended.portable.json` there yourself.

**No samples are published here.** The configuration merely names the folders it expects;
the recordings themselves are the property of their producer and are obtained from them.

## The configuration

- **Organ:** Altenbruch, Klapmeyer Organ.
- **Samples:** Sonus Paradisi - [https://www.sonusparadisi.cz/en/organs/germany/klapmeyer-organ-altenbruch.html](https://www.sonusparadisi.cz/en/organs/germany/klapmeyer-organ-altenbruch.html); the folders `002067` and `002068` (as they come, or in their RAR archives); perspectives default. OrganSampler 0.8.198 or later.
- **Expects:** library root of any name, holding the trees `002067/pipes/bw`, `002067/pipes/hw`, `002067/pipes/pedal` and `002068/pipes/rw`.
- **Audio layout:** every division has the routes default; auxiliary (unused).
- **RAM:**
  - **1.9 GB** - 16-bit, 32 kHz, releases at half the rate, packed; *estimated*
  - **2.7 GB** - 16-bit, 48 kHz, full releases, packed; *measured*
  - **19 GB** - 32-bit float, the files' own rate, full releases, no packing; *estimated*
- **Saved with:** OrganSampler 0.8.198.
- **Author:** Jiri Zurek, 25 October 2026.

## Where to put the file

Put `Altenbruch SemiDry Extended.portable.json` into your configurations folder (Configuration Editor > *Configurations folder*; `Documents\OrganSampler\Configurations` by default) and load it from there. The sample folders are found in your sample locations (the folders above your sample libraries, remembered by the program and listed in the Configuration Editor); one not found is asked for once. The folders `002067` and `002068` must stand side by side in one folder:

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
|  | R. Blockfloit 2' | O. Trometh 8' |  |
|  | R. Sexquialtera 1 1/3' |  |  |
|  | R. Dulcian 16' |  |  |
|  | R. Kromhorn 8' |  |  |

<!-- published by @zurek-jiri (issue #99, 2026-10-25) -->
