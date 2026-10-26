# Ashton Lewis Organ

<!-- Written by OrganSampler. To publish it: attach this README and the zip beside it to the form (a free GitHub account):
     https://github.com/zurek-jiri/OrganSampler/issues/new?template=organ_configuration.yml -->

A complete OrganSampler configuration of Ashton Lewis Organ, as recorded by Sonus Paradisi in 4 perspectives, Diffuse, Direct, Distant and Rear, each on its own audio route: 5 divisions and 47 rows in the Stop List, transmissions and
extensions included (only the rows nested in stop folders are not counted). Whoever owns
the recordings loads the whole instrument in a single step.

**Download:** [Ashton, Albion Church,Surround.portable.zip](https://github.com/zurek-jiri/OrganSampler/raw/main/configurations/ashton-lewis-organ/Ashton,%20Albion%20Church,Surround.portable.zip) - *Load Configuration* in OrganSampler opens the zip as it is (0.8.195) and copies the configuration into your configurations folder; or unpack it and put the `Ashton, Albion Church,Surround.portable.json` there yourself.

**No samples are published here.** The configuration merely names the folders it expects;
the recordings themselves are the property of their producer and are obtained from them.

## The configuration

- **Organ:** Ashton Lewis Organ.
- **Samples:** Sonus Paradisi - [https://www.sonusparadisi.cz/en/organs/england/ashton-lewis-organ.html](https://www.sonusparadisi.cz/en/organs/england/ashton-lewis-organ.html); the folders `002735` and `002736` (as they come, or in their RAR archives); perspectives Diffuse, Direct, Distant and Rear. OrganSampler 0.8.203 or later.
- **Expects:** library root of any name, holding the trees `002735/pipes/CH`, `002735/pipes/GT`, `002735/pipes/PE`, `002735/pipes/SO`, `002735/pipes/SW`, `002736/pipes/CH`, `002736/pipes/PE`, `002736/pipes/SO` and `002736/pipes/SW`.
- **Audio layout:** every division has the routes diffuse, direct, distant and rear; auxiliary (unused).
- **RAM:**
  - **9.4 GB** - 16-bit, 32 kHz, releases at half the rate, packed; *estimated*
  - **13 GB** - 16-bit, 48 kHz, full releases, packed; *measured*
  - **94 GB** - 32-bit float, the files' own rate, full releases, no packing; *estimated*
- **Saved with:** OrganSampler 0.8.203.
- **Author:** Jiri Zurek, 26 October 2026.

## Where to put the file

Put `Ashton, Albion Church,Surround.portable.json` into your configurations folder (Configuration Editor > *Configurations folder*; `Documents\OrganSampler\Configurations` by default) and load it from there. The sample folders are found in your sample locations (the folders above your sample libraries, remembered by the program and listed in the Configuration Editor). One not found shows "Sample folder not found" under its division: click it and choose any folder of the samples (the tree, a folder above it, a neighbouring tree) - the program finds the tree, the other missing ones around it, and remembers the place for every configuration to come. The folders `002735` and `002736` must stand side by side in one folder:

```
<a sample location>\
├── 002735\
│   ├── pipes\CH\
│   ├── pipes\GT\
│   ├── pipes\PE\
│   ├── pipes\SO\
│   └── pipes\SW\
└── 002736\
    ├── pipes\CH\
    ├── pipes\PE\
    ├── pipes\SO\
    └── pipes\SW\
```

## Stop List

| Pedal | I | II | III | IV |
|---|---|---|---|---|
| P. Trombone 16' | 1. Flauto traverso 4' | 2. Clairon 4' | 3. Clarion 4' | 4. Tuba 8' |
| P. Tromba 8' | 1. Harmonic piccolo 2' | 2. Tuba 8' (Trsm.) | 3. Oboe 8' | 4. Cor anglais 8' |
| P. Tuba 8' (Trsm.) | 1. Salicional 8' | 2. Mixture iv rks. 1 1/3' | 3. Horn 8' | 4. Vox humana 8' |
| P. Bass flute 8' | 1. Dulciana 8' | 2. Trumpet 8' | 3. Double trumpet 16' | 4. Clarinet 8' |
| P. Octave 8' | 1. Lieblich gedeckt 8' | 2. Twelfth 2 2/3' | 3. Flautina 2' | 4. Orchestral oboe 8' |
| P. Sub bass 16' |  | 2. Fifteenth 2' | 3. Mixture iii rks 2' | 4. Harmonic flute 8' |
| P. Violone 16' |  | 2. Octave 4' | 3. Gaigen principal 4' | 4. Concert flute 4' |
| P. II open wood 16' |  | 2. Harmonic flute 4' | 3. Voix celestes 8' |  |
| P. I open wood 16' |  | 2. II open diapason 8' | 3. Viole de gamba 8' |  |
| P. Acoustic bass 16' |  | 2. Harmonic flute 8' | 3. Echo salicional 8' |  |
|  |  | 2. Double open diapason 16' | 3. Rohr flöte 8' |  |
|  |  | 2. I open diapason 8' | 3. Gaigen diapason 8' |  |
|  |  |  | 3. Lieblich bourdon 16' |  |

<!-- published by @zurek-jiri (issue #103, 2026-10-26) -->
