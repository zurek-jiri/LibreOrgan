# Adlington Hall Organ

<!-- Written by LibreOrgan Sampler. To publish it: attach this README and the zip beside it to the form (a free GitHub account):
     https://github.com/zurek-jiri/LibreOrgan/issues/new?template=organ_configuration.yml -->

A complete LibreOrgan Sampler configuration of Adlington Hall Organ, as recorded by Sonus Paradisi in 3 perspectives, Diffuse, Direct and Rear, each on its own audio route: 3 divisions and 15 rows in the Stop List, transmissions and
extensions included (only the rows nested in stop folders are not counted). Whoever owns
the recordings loads the whole instrument in a single step.

**Download:** [Adlington Surround.portable.zip](https://github.com/zurek-jiri/LibreOrgan/raw/main/libreorgans/adlington-hall-organ/Adlington%20Surround.portable.zip) - unpack it, the `Adlington Surround.portable.json` inside goes where the next section says.

**No samples are published here.** The configuration merely names the folders it expects;
the recordings themselves are the property of their producer and are obtained from them.

## The configuration

- **Organ:** Adlington Hall Organ.
- **Samples:** Sonus Paradisi - [https://www.sonusparadisi.cz/en/organs/england/adlington-hall-organ.html](https://www.sonusparadisi.cz/en/organs/england/adlington-hall-organ.html); the folders `002707` and `002708` (as they come, or in their RAR archives); perspectives Diffuse, Direct and Rear. LibreOrgan Sampler 0.8.194 or later.
- **Expects:** library root of any name, holding the trees `002707/pipes` and `002708/pipes`.
- **Audio layout:** Pedal: no routeauxiliary (unused); I: the routes diffuse, direct and rear; auxiliary (unused); II: the routes diffuse, direct and rear; auxiliary (unused).
- **RAM:**
  - **2.9 GB** - 16-bit, 32 kHz, releases at half the rate, packed; *estimated*
  - **4.1 GB** - 16-bit, 48 kHz, full releases, packed; *measured*
  - **29 GB** - 32-bit float, the files' own rate, full releases, no packing; *estimated*
- **Saved with:** LibreOrgan Sampler 0.8.194.
- **Author:** Jiri Zurek, 25 October 2026.

## Where to put the file

Put `Adlington Surround.portable.json` into your LibreOrgans folder (Configuration Editor > *LibreOrgans folder*; `Documents\LibreOrgan\LibreOrgans` by default) and load it from there. The sample folders are found in your sample locations (the folders above your sample libraries, remembered by the program and listed in the Configuration Editor); one not found is asked for once. The folders `002707` and `002708` must stand side by side in one folder:

```
<a sample location>\
├── 002707\
│   └── pipes\
└── 002708\
    └── pipes\
```

## Stop List

| I | II |
|---|---|
| Ch. Flute 4' | GO. Open Diapason 8' |
| Ch. Bassoon 8' | GO. Stopt diapason 8' |
| Ch. Stopt Diapason 8' (Trsm.) | GO. Principal 4' |
|  | GO. Bl. Flute Bass 2' |
|  | GO. Ttwelfth 2 2/3' |
|  | GO. Bl. Flute Treble 2' (Ext.) |
|  | GO. Fifteenth 2' |
|  | GO. Tierce 1 3/5' |
|  | GO. Nineteenth 1 1/3' |
|  | GO. Trumpet 8' |
|  | GO. Twenty Second 1' |
|  | GO. Vox Humana 8' |

<!-- published by @zurek-jiri (issue #97, 2026-10-25) -->
