# Stralsund, St. Jakobi - the Wegscheider organ

A complete OrganSampler configuration of the Wegscheider organ in the church of St. Jakobi, Stralsund, as recorded by Sonus Paradisi in 3 perspectives, Direct, Diffuse and Rear, each on its own audio route: 4 divisions and 71 rows in the Stop List, transmissions and
extensions included (only the rows nested in stop folders are not counted). Whoever owns
the recordings loads the whole instrument in a single step.

**Download:** [Stralsund-Jakobi-Wegscheider.zip](Stralsund-Jakobi-Wegscheider.zip?raw=true) - unpack it, the `Stralsund-Jakobi-Wegscheider.json` inside goes where the next section says.

**No samples are published here.** The configuration merely names the folders it expects;
the recordings themselves are the property of their producer and are obtained from them.

## The configuration

- **Organ:** the organ of St. Jakobi, Stralsund, built by Kristian Wegscheider.
- **Samples:** Sonus Paradisi, *Stralsund, St. Jakobi, Wegscheider organ* (https://www.sonusparadisi.cz/), two Hauptwerk installation packages; the folders `003135` and `003136`, unpacked as plain WAV files; perspectives Direct (`01-dirMk2H`), Diffuse (`02-difMk2H`) and Rear (`03-rearMk4`). OrganSampler 0.8.92 or later.
- **Expects:** library root of any name, holding the trees `003135/pipe/HW`, `003135/pipe/OW`, `003135/pipe/PE`, `003135/pipe/UW`, `003136/pipe/HW`, `003136/pipe/OW`, `003136/pipe/PE` and `003136/pipe/UW`.
- **Audio layout:** every division has the routes direct, diffuse and rear; auxiliary (unused).
- **Saved with:** OrganSampler 0.8.90.
- **Author:** zurek-jiri, 15 October 2026.

## Where to put the file

The folders `003135` and `003136` must stand side by side in one folder. Place `Stralsund-Jakobi-Wegscheider.json` in that folder:

```
<any folder>\
├── Stralsund-Jakobi-Wegscheider.json
├── 003135\
│   ├── pipe\HW\
│   ├── pipe\OW\
│   ├── pipe\PE\
│   └── pipe\UW\
└── 003136\
    ├── pipe\HW\
    ├── pipe\OW\
    ├── pipe\PE\
    └── pipe\UW\
```

## Stop List

| Pedal | I | II | III |
|---|---|---|---|
| Ped Principalbass 32' | UW Gedackt 8' | HW Principal 16' | OW Quintadena 16' |
| PEd Subbass 16' | UW Quintadena 8' | HW Gamba 8' | OW Flote 8' |
| PEd Princip 16' | UW Principal 8' | HW Octava 8' | OW Principal 8' |
| Ped Offenbass 16' | UW Traversflote 4' | HW Rohrflote 8' | OW Lamento 8' |
| Ped Octave 8' | UW Octave 4' | HW Quinte 6' | OW Octava 4' |
| Ped BassFloete 8' | UW Fugara 4' | HW Gemshorn 8' | OW Salicional 8' |
| Ped octave 4' | UW Gemshorn 2' | HW Octave 4' | OW Octava 2' |
| Ped Mixtur 2 2/3' | UW Nasard 3' | HW SpitzFl 4' | OW Gedeckt 4' |
| Ped ContraPos 32' | UW Quinte 1 1/3' | HW Octave 2' | OW Mixtur 1 1/3' |
| Ped Posaune 16' | UW Tertia 1 3/5' | HW Quinte 3' | OW Sesquialtera 2 2/3' |
| Ped Fagot 16' | UW Mixtur 1' | HW Mixtur 2' | OW Trompete 8' |
| Ped Trompete 8' | UW Hautbois 8' | HW Tromp 16' | OW VoxHumana 8' |
| Ped Schalmey 4' | UW Gedackt 16' (Ext.) | HW Zimbel 1' | Vox Humana Cornet [V] |
| Ped Principalbass 64' (Ext.) | UW Gedackt 4' (Ext.) | HW Cornet 8' | Bombarde 16' (Ext.) |
| Cornet [III] | UW Octave 2' (Ext.) | HW Rohrflote 16' (Ext.) | Clairon 4' (Ext.) |
| Ped Posaune 64' (Ext.) | UW Gedackt 2' (Ext.) | Montre 32' (Ext.) |  |
| BauernFlöte 2' (Trsm.) | Regal 16' (Trsm.) | HW Gamba 16' (Ext.) |  |
| Ped ContraPos 64' (Ext.) | Septime 1 1/7' (Ext.) | HW Tromp 8' (Ext.) |  |
|  |  | Flöte 2' (Trsm.) |  |
|  |  | HW Tromp 4' (Ext.) |  |
