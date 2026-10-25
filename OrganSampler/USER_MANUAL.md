# OrganSampler - User Manual

**PIPE ORGAN SAMPLER FOR CREATIVE MINDS.**
**Design your console, Build your stop list, Perform ... !**

OrganSampler plays sampled pipe organs: every pipe is a recording, every stop
a folder of recordings, and the program turns any number of such folders into
an organ with up to seven manuals and a pedal, with couplers, combinations, a
crescendo, swell boxes, tremulants, a wind model, reverb per microphone
perspective, and extensions and transmissions built from the stops you have.

The whole manual is inside the program: *Configuration Editor > About &
licences...* shows it as the first entry of the dialog, with a chapter box
above the text that jumps to any chapter and a *Find* box that searches the
text (type a word, press Enter for the next occurrence, *Previous* for the one
before). Chapter names in the text, and "part N", are links: click them to
go there. The "?" button beside every option of the Configuration Editor
opens the same dialog at the chapter of that very option (part 8, the
reference), the heading painted orange. Parts 1 to 7 explain the program
feature by feature. When the program starts with an empty instrument, the stop
workspace shows a short hint with a button that opens this manual; it
disappears as soon as a folder or a configuration is loaded.

Contents

1. Getting started

    1.1 What you need

    1.2 Installation

    1.3 The first start, step by step

    1.4 How the sample folders must be organised

    1.5 Configurations and settings

2. The main window

    2.1 Top bar buttons

    2.2 The combination row

    2.3 The crescendo

    2.4 Global sound sliders and the polyphony display

    2.5 Division headers

    2.6 Two program windows

    2.7 Screens and the size of the interface

    2.8 Dialogs on small screens

3. Stops on the grid and their voicing

    3.1 Stop buttons

    3.2 Arranging the grid

    3.3 The voicing row

    3.4 Transmissions and extensions

    3.5 Velocity layers

    3.6 Stop-Merging

4. The Stop List (building the organ)

    4.1 Folder trees and the pool

    4.2 Editing the table

    4.3 Apply and reload

    4.4 What the loading dialog shows

5. The Divisions dialog

    5.1 Keyboard, MIDI and compass

    5.2 Swell, tremulant, wind

    5.3 The audio tabs

    5.4 Sample channels and output channels

    5.5 The auxiliary route

    5.6 Audio drivers: many sound cards at once

    5.7 Audio devices not ready when OrganSampler starts

    5.8 Getting the most polyphony

6. Playing aids

    6.1 Couplers

    6.2 Combinations and the crescendo

    6.3 Temperaments

    6.4 MIDI learn

    6.5 Recording

7. Filled pipes, extensions and the transposition methods

    7.1 What a filled pipe is

    7.2 The transposition methods

    7.3 The settings that go with the methods

    7.4 The cache

8. Configuration Editor reference

    8.1 Engine

    8.2 Global sound

    8.3 Advanced global audio

    8.4 Console

    8.5 Folder name patterns (one per line)

    8.6 Division sections (Pedal, Manual I ... Manual VII)

    8.7 Keeping track: the activity log

    8.8 Free software, licences, platforms

---

# 1. Getting started

## 1.1 What you need

- Samples: folders of WAV files, one folder per stop. Sonus Paradisi
  sample folders (wave format) usually work with OrganSampler as they are.
  Other samples must follow the conventions explained in "How the sample
  folders must be organised" below.
- An audio interface. ASIO gives the lowest latency and the most polyphony on
  Windows; Windows Audio (WASAPI) works too. macOS uses CoreAudio, Linux ALSA
  or JACK.
- A MIDI keyboard or an organ console with MIDI out, one MIDI channel per
  division.

No organ definition file is needed: you load the folder trees of your samples,
build the stops and the whole virtual organ from them yourself (the program
proposes a layout at once), play, and save the result as a configuration for
the next time (see "Step 7: Save" below).

If you already have a pre-saved organ configuration - someone kind has done
this work for you - you save all of that time: *Load Configuration* in the
top bar, choose the file, and voilà, the whole virtual organ is ready to play.
You can of course go on improving it in any way you like, and save it again.

## 1.2 Installation

OrganSampler is one executable file. It comes from the releases page as the
archive native to your system - `OrganSampler-windows.zip` (with
`OrganSampler.exe` inside), `OrganSampler-macos.zip` or
`OrganSampler-macos-10.11-intel.zip` (the app), `OrganSampler-linux-x86_64.tar.gz`
(`tar xzf`; the program stays executable). Unpack it, put the program
anywhere and start it; no installation is needed, and nothing is read from
beside it. Everything the program remembers
(settings, voicing, combinations, the filled-pipes cache) lives in the user's
application-data folder:

- Windows: `%APPDATA%\OrganSampler`
- macOS: `~/Library/Application Support/OrganSampler`
- Linux: `~/.config/OrganSampler`

The window title shows the program version.

On Linux the executable file itself cannot carry an icon (unlike a Windows
.exe): in the file manager it always shows the generic "executable" symbol,
on every Linux program. The application menu, the dock and the task bar take
the icon from a `.desktop` entry: *Install menu entry and icon* at the
bottom of the Configuration Editor writes that entry
(`~/.local/share/applications/organsampler.desktop`, pointing at the
executable where it is now) and the icon in every size from 16 to 512 px
under `~/.local/share/icons/hicolor` - the same hand-tuned images as the
Windows icon, size for size. Then open the application menu (the launcher
button in the panel, or the Super/Windows key) and type "Organ": OrganSampler
is there, in the Multimedia (Sound & Video) group - Audio Production on
Ubuntu Studio - with its icon; right-click
it to pin it to the panel or the dock. Install again after moving the
executable. On macOS the application bundle
carries the icon. The window itself shows it on every platform.

## 1.3 The first start, step by step

### Step 1: Size

The window opens sized for your screen; resize it by dragging its edges like
any other window. If the elements of the console - buttons, sliders, text -
are too big or too small for your screen, open *Configuration Editor* (top
bar) and move *Interface size*.

### Step 2: Divisions

The box in the upper right corner of the main window (or *Configuration
Editor > Console > Divisions*) sets how many manuals the console has; the
pedal is always there.

### Step 3: MIDI

Click *Settings* in a division's header: the Divisions dialog opens. Choose
the *MIDI keyboard device* and *MIDI channel* of every division. Press a key:
the keyboard drawing of the division lights up. Quicker: right-click the
keyboard icon beside the division's name in its header (or the keyboard
drawing in the Divisions dialog), choose *MIDI Learn...* and press any key on
that keyboard - the device and the channel are taken from it.

### Step 4: Samples

Click *Stop List* in the top bar, then *Load folder...* and choose the top
folder of the samples for one division (the folder that holds that division's
stop folders, see "How the sample folders must be organised") - or the RAR
archive(s) the samples came in, which the program reads as the folders they
hold (see "Sample archives (RAR)"). The program
asks which division its stops go to. Repeat for every division; or load one
tree holding everything and move stops between divisions in the table. Click
*Apply and reload*. The stops appear as buttons on the grid. This step comes
before the audio, because the perspectives of a division (and so the audio
tabs for them) exist only once its stops have been built from the folders.

### Step 5: Audio

Click *Settings* in a division's header again and go to the audio tabs at the
bottom of the Divisions dialog: one tab per perspective of the division (the
default perspective, or Direct, Diffuse, Rear ... as the folders had them) and
one tab *auxiliary*. Every tab is an audio route with its own device, sample
rate, buffer size and output channels. On the first tab choose the audio
device type (on Windows preferably ASIO), the device, the sample rate and the
buffer size (512 samples, about 10 ms, is a good choice: see "Getting the most
polyphony"), and tick the output channels of the device in the list on the
left of the tab - a stereo pair for this perspective, or more channels. Then
either repeat this on every tab of every division - different channels for
different divisions and perspectives are what a multi-channel interface is
for - or, when all divisions are to play through the same device, tick *Set
for all perspectives and divisions* on one tab: the device, sample rate and
buffer size are copied to every route; the channels stay as ticked on each
tab. Which drivers to use - ASIO for everything on Windows when you can, what
WASAPI and the aggregated device are for, and what mixing them costs - is
explained in "Audio drivers: many sound cards at once". How mono and stereo
samples are spread over the ticked channels is explained in "Sample channels
and output channels". Leave the *auxiliary* tab of every division at *none*
for now ("The auxiliary route").

### Step 6: Play

Engage the stops and play.

### Step 7: Save

*Save Configuration* in the top bar writes everything - folders, stops,
voicing, audio, MIDI, combinations, couplers, crescendo - into one file. The
configuration is OrganSampler's counterpart of what other organ samplers call
an organ definition file (ODF), with the difference that you never write it:
you build the organ in the program and save it. *Load the last configuration
at startup* (Configuration Editor > Console) opens it again next time;
otherwise *Load Configuration*. Keep every folder tree of the organ on one
disk (ideally under one folder): the configuration then stores the sample
paths relative to that place and can move with it. Trees on another disk are
stored with absolute paths; when such a tree is not found at the next load,
the program asks where it is.

## 1.4 How the sample folders must be organised

The samples form a tree of folders: a top folder (the one you load with *Load
folder...*) holding one folder per stop, each stop folder holding the WAV
files of its pipes and, inside it, the folders of the release samples. A set
with several microphone perspectives has the stop folders several times, each
with the prefix of its perspective. The whole tree of a small set:

```
POS Organ Samples                <- the top folder: "Load folder..." in the Stop List
├── Principal8                   <- one folder per stop (here without perspectives)
│   ├── 036-C.wav                <- a WAV per pipe, named after its MIDI key (several for one key play in turn)
│   ├── 037-C#.wav
│   ├── ...
│   └── rel                      <- the release samples, named like the attacks
│       ├── 036-C.wav
│       └── ...
├── dir-Trompete 8               <- the "Direct" perspective of Trompete 8
│   ├── 036-C.wav
│   ├── ...
│   ├── rel500                   <- releases for keys held up to 500 ms
│   ├── rel1500                  <- ... up to 1500 ms
│   └── rel                      <- ... and longer
├── dif-Trompete 8               <- the "Diffuse" perspective of the same stop
│   └── ...
├── rear-Trompete_8              <- the "Rear" perspective, (underscore ignored) 
│   └── ...
├── Mixtura                      <- a stop without a footage in its name: fine
│   ├── ...
│   └── Chiff                    <- a one-shot stop (nested inside another stop's folder: allowed)
│       └── ...
└── Octava4
    ├── direct-Octava4           <- the perspectives can be nested like this
    │   └── ...
    ├── diffuse-Octava4
    │   └── ...
    └── rear-Octava4
        └── ...
```

The rules, one by one:

- **One folder per stop.** Any folder that contains WAV samples is
  interpreted as one stop (register), however deeply it is nested in the
  folder tree. A folder whose name starts with "rel" is interpreted as the
  releases of its parent folder; "rel" folders never form stops of their own.
  The label of the stop is built from the folder path (not from the bare
  folder name!), with slashes, underscores and similar characters left out.
  The only exception: when a folder is nested inside a folder of the same
  name - the perspective prefix, the footage and an L/R mark aside
  (direct-Octava4 inside "Octava4" in the example above; "02-direct-HW_Bl.Flute"
  inside "HW_Bl.Flute2") - the parent adds nothing and the label carries the
  name once only ("Octava 4'"); this holds for folders read out of a RAR
  archive as for unpacked ones. **Two components of one stop:** a stop
  recorded as a left and a right half, the folders named alike with a single
  L and R right after the footage or at the end of the name
  ("AB_quintaton16L" + "AB_quintaton16R", "AB_glockentonL" + "AB_glockentonR"),
  is ONE stop "quintaton 16'" whose two folders both play as its ranks (the
  Stop List cell shows "+ half"); a rank count after a number ("Mixtur4x",
  "Zimbel3") is no such mark. A bare footage number at the end of the folder name
  (Nasat3, Nasat_3, Nasat 3 ...) is shown with its foot mark ("Nasat 3'") on
  the stop label; a mutation written without spaces ("Nasard 223", "Terz
  135", "Larigot 113") is read as 2 2/3', 1 3/5', 1 1/3', and the organ
  builders' shorthand is understood: "3" is the quint 2 2/3', "6" the 5 1/3',
  "12" the 10 2/3', "1 1/2" the 1 1/3' (no rank of 3' or 6' exists; the
  transposition is counted against the real footage). A folder name
  without a footage is no problem: the program learns everything it can about
  the samples from their smpl chunk (next rule), and from that it knows the
  footage, the tuning and the temperament of the rank.
- **The smpl chunk** is a small block of data inside a WAV file that samplers
  use; most audio editors and tools can write it. OrganSampler reads these
  fields of it:
  - *MIDI unity note* (dwMIDIUnityNote, 0 to 127): the measured pitch rounded
    down (yes, rounded down!) to the nearest MIDI note. Zero is allowed, but
    then every pipe counts as exactly in tune.
  - *MIDI pitch fraction* (dwMIDIPitchFraction): the fractional offset above
    the unity note - the fine tuning of the recorded pitch as a fraction of a
    semitone, scaled from 0 to 2^32-1 for 0 to 100 cents. Combined with the
    unity note it gives the exact recorded pitch, from which the tuning, the
    temperament and the footage of the pipe are calculated. Note that
    dwMIDIPitchFraction is always a positive number: a slightly flat A4 cannot
    be coded into the smpl chunk as A4 minus 4 cents, only as G#4 plus 96
    cents. This is of paramount importance for reading the pitch of a sample
    correctly!
  - *The loops* (cSampleLoops and the loop list, dwStart and dwEnd of each
    loop in sample frames): the sustain is looped between these points while
    the key is held. Several loops are allowed; the loop type and the other
    fields of the chunk are ignored. A file without loops is looped as a
    whole (see "Loop points and pitch" below).
  When the files have no smpl chunk at all, it is no tragedy, though it does
  limit what OrganSampler can do. The samples are loaded and played just as
  they are: the program then knows nothing about their tuning (no retuning,
  no temperament, no celeste detection; the footage is taken from the folder
  name only, without correction) - but the stop plays.
- **Missing pipes and missing loops** are no obstacle. Keys the folder has no
  file for are filled by repitching the neighbouring samples (see "What a
  filled pipe is"); a sample without loop points is marked by an orange key
  number in the voicing and plays from start to end, then loops from the
  start again.
- **Which key a file belongs to is determined by its name.** The name of a
  WAV sample must begin with the MIDI key number, optionally followed by the
  note designation (036.wav, 036-C.wav, 059-B3.wav ...).
  Several files with the same key number are round-robin variants of that
  pipe, played in turn: whatever else the file name carries (`036_1.wav`,
  `036-C_bis.wav`) marks a variant. The one exception is a two- or
  three-digit suffix at the very end after an underscore (`060-C4_070.wav`):
  that is a velocity layer (see "Velocity layers"). The two combine:
  `036-C_bis_070.wav` is a round-robin variant inside the layer 070. Mono and
  stereo files are both fine; how they are spread over the output channels is
  explained in "Sample channels and output channels".
- **Releases** (the sound after the key is let go) in a folder named `rel`
  inside the stop folder, the files named like the attacks. Several release
  folders for notes of different lengths are honoured: `rel500` holds the
  releases for keys held up to 500 ms, `rel1500` up to 1.5 s, and the folder
  with the largest number the rest - plain `rel` counts as `rel99999`, so it
  takes everything the numbered folders do not. The program picks the folder
  matching how long the key was held. A stop folder with its release folders
  looks like this:

```
Principal 8
    036-C.wav
    037-C#.wav
    ...
    096-C.wav
    rel500
        036-C.wav
        037-C#.wav
        ...
    rel1500
        036-C.wav
        ...
    rel
        036-C.wav
        ...
```

  A set with one release per pipe has just the folder `rel`. A stop without any
  release folder plays as well: when the key is let go the sound fades out
  (the *Sustain fade-out* of Global sound) and the voice falls silent, with no
  room decay.
- **Loop points and pitch** come from the WAV's smpl chunk. A sustained sample
  without loop points is looped as a whole (a short fade at its end, no click).
- **Microphone perspectives.** When the same stops exist under several
  prefixes ("dir-Principal 8", "dif-Principal 8", "rear-Principal 8") the
  prefixes are recognised as perspectives (Direct, Diffuse, Rear) and each
  becomes an independent audio route with its own device, channels, volume and
  reverb. The prefixes are listed in *Configuration Editor > Folder name
  patterns > Perspective prefixes*, one line per perspective: all the prefixes
  that belong to the same perspective stand on one line, separated by
  semicolons, and the perspective is named after the first entry of its line
  ("Direct;01-dirMk2H-;dir" is the perspective "Direct", whichever of the
  three prefixes a folder carries). New prefixes found in a set are added
  there by themselves: one sharing its stem with a known perspective joins its
  line, an unknown one gets a line of its own, which you can then merge by
  hand.
- **Sample rates.** Everything is converted to 48 kHz while loading (96 and
  44.1 kHz files included), with a high-quality filter, so samples of different
  rates sound alike and need the same RAM. The audio device may run at any
  rate.
- **One-shot stops.** A folder whose name matches one of the patterns listed
  in *One-shot stops (folder contains)* (defaults "Chiff", "Chimes", "Tubular",
  "Celesta", "Harp", "Glockenspiel" - "Celesta" the keyboard instrument, not
  the "Celeste" rank of strings) holds samples that
  play once from start to end without looping, however long the key is held:
  chiffs recorded separately, percussion, bells. Their files are named like
  any other pipe's; no release folder is needed.
- **Folders to skip** (tremulant recordings, say) are listed in *Ignored
  folders (folder contains)* (defaults "trem", "*_tr" and "output" - the
  summed or harmonic renderings some sets keep under a rank, "outputsum",
  "outputharm"): such folders are not loaded. Besides the patterns, a folder
  named after its sibling or its parent with "tr" or "trem" appended
  ("Pos_Bourdontr" beside "Pos_Bourdon", "HW_Gedackt8TREM" beside
  "HW_Gedackt8") is the tremmed recording of that stop and is skipped by every
  tree scan too (0.8.135; such folders stood in the pool as a second copy of
  every stop). A folder a Stop List row names explicitly is loaded whatever
  its name.
- **How the folder-name patterns match.** A pattern without a star matches
  anywhere in the folder name ("trem" matches "Principal 8 trem" and
  "tremulant"). A star at the start anchors the pattern to the end of the
  name: "*_tr" matches only folders that end in "_tr" ("Principal8_tr"), not
  "Principal_tremolo". A star at the end anchors it to the start ("dir-*").
  Stars at both ends match anywhere, like no star at all. Capital letters do
  not matter; underscores and other characters count exactly as written.

### Sample archives (RAR)

Sample folders are mostly distributed as RAR archives. They need not be
unpacked: *Load folder...* in the Stop List takes the `.rar` file(s) as well
as a folder, and reads the samples out of the archives.

- **Reading from the archives:** in *Load folder...* choose *Browse
  archives...* (several with Ctrl or Shift; a multi-volume set by its first
  part, `.part1.rar` or `.part01.rar`, the other parts lying beside it) or
  paste the paths, one per line, and click *Load from the archive*. The
  archive is read as the folders it holds: its top-level folders become
  folder trees "beside" the archive, as if it had been unpacked there, and
  the program finds their contents inside the archive. A configuration names
  those folders exactly as it would name unpacked ones, so the same
  configuration serves an unpacked and a packed set (a folder that is on the
  disk always wins over the archive). When the archive holds a Hauptwerk
  organ definition, the program offers to import it - the whole organ, read
  from the archives. *Load Configuration* takes `.rar` files as well - one,
  or several at once when an organ comes in several archives: the
  configurations and definitions found inside are listed in a dialog of its
  own ("Load which one", in the middle of the screen, closed only by Load or
  Cancel) and the one you pick is loaded straight from the archives (the
  others serve its sample folders). *Merge another configuration* (right-click Load Configuration)
  takes archives the same way, so an organ can be put together archive after
  archive.
  Two steps happen, and they read different things. *Reading the archives*
  reads the archive's headers only - what files it holds, how big they are
  and where their bytes lie (a few megabytes: seconds on an SSD, a few
  minutes for a 60 GB set on a hard disk, where every header is a seek). That
  index is made once and kept under the settings folder; the next load finds
  it there. *Loading saved divisions* then reads the samples themselves,
  as any load does. The index must come first: the program has to know
  which folders the archive holds before a definition can be planned or a
  Stop List built from them.
  Every file is then read on its own, so the loader reads the files in
  parallel; a set that was stored without compression reads as fast as plain
  files, a compressed one at the speed of the decompression (100-200 MB per
  second and core, all cores at once). RAR 4 and RAR 5; a solid archive is
  read in sequence, which is slower; password-protected archives are refused.
  What you gain: the disk space (a packed set is a half to two thirds of the
  unpacked one), fewer bytes read from a slow disk, and nothing to unpack.
  What you pay: the first load indexes the archive, and a compressed set
  takes the decompression time at every load - from a fast SSD an unpacked
  set loads faster.
- **Which archives are read:** only the archives you choose - in *Load
  folder...*, *Load Configuration* or a merge - with their parts, never
  another archive that happens to lie beside them. Every chosen archive is
  written into `ArchiveFolders.txt` beside the open configuration (in the
  configurations folder, see "Where configurations and samples live";
  without a configuration into the settings folder; one path per line,
  editable by hand), so the next start finds it again; a line naming a
  folder stands for every archive in that folder, and so does a folder
  `OrganRarPackages` beside the configurations folder. A sample folder that
  is not on the disk is looked for in those archives by the tail of its path
  (`OrganInstallationPackages/002735/pipes` ...). The folder of every chosen
  archive is also remembered as a sample location, so an unpacked set beside
  its archives is found as well.
- **Unpacking in OrganSampler:** tick *Unpack the archives into a folder
  first* in the same window, choose the folder (proposed: the folder of the
  first archive; each archive's own folder tree is created inside it) and
  click *Unpack and load*: the archives are unpacked one after another with
  one progress bar (it can be cancelled), then their folder trees are loaded.
  The top-bar button *Unpack archive* does the unpacking alone; it is hidden
  by default (Configuration Editor > Console shows it).
- **With a free program:** 7-Zip on Windows (https://www.7-zip.org), Keka or
  The Unarchiver on macOS, `unrar x` or `7z x` in a terminal on Linux. WinRAR
  itself unpacks as well (as a trial). Right-click the archive, choose
  "Extract to ...".
- **A zip** needs no program at all: every system opens it by a double-click.

Unpacked or not, keep the folder structure as the archive has it: the
configurations published for a set name its folders as the archive created
them. Unpacked samples load faster from a fast disk (no decompression) and
the archives can go; packed ones save the disk space.

## 1.5 Configurations and settings

A **configuration** is one file holding everything about an instrument: the
folder trees, the Stop List, voicing, audio and MIDI settings, couplers,
combinations, crescendo, window layout - the organ definition, written by the
program. *Save Configuration* and *Load Configuration* are in the top bar;
*Load Configuration* takes a configuration (`.json`), a Hauptwerk organ
definition (see "Hauptwerk organ definitions") or a RAR archive holding
either, read straight from the archive. A
configuration counts as open once it was loaded or saved in the session (its
name is in the window title). When you quit, or load another configuration,
while the open configuration has unsaved changes, the program asks whether to
save the changes first: *Save* writes them into the file and goes on, *Don't
Save* goes on and leaves the file as it was, *Cancel* stays. An instrument
built with no file open is offered a new file the same way (*Save As...*).
*Shut down PC* never asks:
it saves into the open configuration (or into a rescue file) and shuts down.

Before the samples of a loaded configuration (or of a Hauptwerk organ
definition) load, a box asks how they are to be held in RAM: *Sample storage
in RAM*, *Compress samples in RAM* and *Sample rate on loading* (see the
Configuration Editor's Engine section for what they mean). The box shows the
configuration file's own values when it carries them, else this computer's
last choices; what you choose loads and is saved with the configuration.
*Cancel* leaves the loading alone.
Starting the program without *Load the last configuration at startup* begins
with an empty instrument; nothing stops you from loading folders and playing
without any configuration at all.

### Loading a Hauptwerk organ definition (ODF)

*Load Configuration* also opens a Hauptwerk organ definition
(`<organ>.Organ_Hauptwerk_xml`, the files in Hauptwerk's `OrganDefinitions`
folder) and makes an OrganSampler configuration of it - the quick way to a
sample set that came with a Hauptwerk definition. What is taken from the
definition: the divisions in the order of their keyboards (the pedal first)
with their key ranges - a division of ours is a keyboard, so every Hauptwerk
division a keyboard plays joins it, the ones it plays only with a ventil
drawn included (Caen: "Pedal" and "PedalAnches" are one division of ours,
the reeds after the flues; the definition links keys to divisions through
its switches, and the import follows those links) -, the stops with their
sample folders, the perspectives
(front, direct, diffuse, rear ... as the ranks are named, turned into the
perspective patterns), and the per-pipe level adjustments as the voicing.
Hauptwerk has no volume per perspective, only the pipes, so a perspective
recorded quieter is quieter in every one of its pipes; OrganSampler has three
steps, and the import uses them (0.8.126): the median of all pipe levels of a
division's perspective becomes that route's volume (the division dialog's
audio tab), the median of what is left becomes the stop's volume (averaged
over its perspectives; most pipes stand at 0 dB, a few voiced ones must not
pull the whole stop), the rest the per-key voicing. The way from a
stop to its folders is the definition's own: stop > its StopRank rows > the
ranks > the folders of their pipe samples; the folders decide what a stop
is. Who owns a rank several stops play (0.8.135): the stop whose name the
rank carries ("Rear: GO. Bombarde 16" belongs to "GO. Bombarde 16", whatever
the file order - the pedal's "Ped. Bombarde 16" borrows it, not the other
way round), else the stop playing it straight over its whole division, then
the one with the most keys. A stop with ranks of its own is a stop of its
own and is loaded from them, whatever it borrows besides (Billerbeck's "Tuba
16" plays its own rank and takes three top keys from the "Tuba 4": the
filled pipes cover those). A stop with NO rank of its own - its rows play
another stop's ranks shifted (a 4' playing the 8' rank an octave up, a stop
borrowed into another division) - is an extension or transmission in
Hauptwerk's terms as in ours: it is made as a transmission of the owner it
borrows the most keys from, with that shift, not loaded a second time. A
stop goes to the division of ours whose keyboard plays its Hauptwerk
division; a FLOATING division - one no keyboard reaches without a coupler
(Bellevue's "Chamade", played from every keyboard through "Bom-Ped/Gt/Pos/Rec
8") - is placed stop by stop where the name says ("GO. Trompette en chamade
8" on the Great, "Ped. Clairon en chamade" on the Pedal, "5. Tuba 16" on V).
A keyboard whose own division its keys reach only under a condition switch
(Ashton: "1. unison off" conditions the link from the keys of 1. manual to
the Choir) gets that switch as its Unison Off coupler (0.8.144). A stop
none of whose ranks has release samples (a carillon, chimes) is a
one-shot stop: its folder names join the One-shot Patterns of the
configuration. The couplers of the definition (0.8.138: its KeyAction rows -
the keys of one keyboard playing another keyboard or a division, shifted by
0, 12 or -12 semitones, or the top note only) become the couplers of ours
that exist for them - a keyboard pulling a higher division at 8', 16', 4' or
as a melody coupler, itself at 16' and 4', I from II or III - and only those are
shown on the console; a coupler of the definition OrganSampler has no
coupler for (one to a floating division whose stops lie in several
divisions, a 24' coupler) is named in the import's report. The catalog of
couplers has the "downward" ones since 0.8.139 - a keyboard pulling a lower
manual, I/II, I/III, II/III and so on at 8', 16', 4', as a melody or bass
coupler, listed after the usual ones and hidden until shown (Couplers
dialog) or brought by an import; the bass couplers to the pedal are the
"pedal from the manual" couplers (the manual's lowest key plays the pedal) -
Doesburg's "bass_1" to "bass_4" become them (0.8.140). After an import the
console shows exactly the definition's couplers: every other coupler of the
catalog is hidden, whatever the organ before had shown (0.8.142). The
stops take their places on the grid from the definition's jamb page - the
page holding the most drawknobs ("Simple Jamb"): within every division,
the drawknobs' columns and rows become the grid's columns and rows, the
picture copied (a drawknob whose name the definition cut short is found
all the same); a division laid out too wide or too tall to be ours stays
in the table's order (0.8.142). A division column narrower than the page's
columns folds the rightmost column into its neighbour (left, right, left,
right ...); widen the division's column to see the page's layout. An extension's button carries the
definition's name of it ("P. tuba 4"), not the owner's name with the
footage shifted. The general crescendo takes them too: a stage
that draws a coupler's drawstop switches it on in that step. A stop
with several ranks in ONE perspective (Caen: a stereo stop recorded as a
left and a right rank, "Quintaton 16 (L component)" and "(R component)",
two folders for the same perspective) becomes a stop folder: its own row
carries the Hauptwerk name and no folders, its children - one per rank, the
ranks matched across the perspectives by their names - carry the folders,
so the button plays them all. Ranks of different perspectives stay one
stop with one folder per perspective, as ever. The compass of every stop
comes from the definition too (0.8.128): the keys its StopRank rows give it
- a Cornet from c1 up, a stop of several pieces as their union - and the
keys of the division outside them are switched off in the stop's voicing,
so they stay silent and no filled pipes are made for them (switch them on
again in the voicing dialog if you want them filled). The speed of the
definition's tremulant (the middle of the range it wanders in) becomes the
tremulant rate of every division whose pipes it modulates; the tremulant
itself stays off until you draw it. The definition's general crescendo
becomes ours (0.8.129): Hauptwerk keeps one combination per stage
("Cresc.Comb.1" to "Cresc.Comb.64"), the stops each stage draws become a
step of our crescendo (every row on or off, the hand registration
cancelled, as there); more than 60 stages are spread over our 60 steps, and
couplers are not taken. The rows of
the Stop List are named after the folders, as for any organ built from
folders; the Hauptwerk stop name becomes the alias, the caption on the
button. Everything else - couplers, combinations, tremulants, wind,
displays, noises (blower, key and stop action, Zimbelstern...) - is ignored:
the folder structure decides the organ, as always. Tremmed samples (the
"tremmed" ranks, the `trem` folders, the `..._TR` sibling folders of the
divisions, the `...tr` / `...TREM` siblings of the stop folders) are left out
and the configuration starts with the default Ignored Patterns; OrganSampler
has its own tremulant. (A stop the definition plays from folders named TREM
only - Billerbeck's "Grosse flute 32" - is taken as Hauptwerk has it, and the
import's report says so.) Names and aliases the import makes put a capital
after a division abbreviation ("P. Trombone 16", "3 Geigenpr 8"); so do the
row names of every tree scan. Encrypted sample sets (`.hbw` files) cannot be
played and the import says so.

The samples must lie where Hauptwerk keeps them: the definition in
`OrganDefinitions`, the packages in `OrganInstallationPackages/<six-digit
id>` beside it (`D:\Hauptwerk\HauptwerkSampleSetsAndComponents\...` on a
usual installation) - or in the RAR archives the set came in, read without
unpacking (see "Sample archives (RAR)"). The configuration is written as
`<organ name>.json` into your configurations folder (0.8.190, see "Where
configurations and samples live"; `Documents\OrganSampler\Configurations`
by default), so nothing of ours lands inside Hauptwerk's tree and all your
organs are in one place, the unpacked and the archived ones alike. The
`OrganInstallationPackages` folder is its library root; the folder above it
is remembered as a sample location, so the file works from anywhere.
Versions 0.8.158-0.8.189 wrote the file into an `OrganConfigurations` folder
beside the folder that holds `HauptwerkSampleSetsAndComponents`
(`D:\OrganConfigurations` for the usual `D:\Hauptwerk\...`), 0.8.125-0.8.157 beside `OrganDefinitions`,
earlier ones into the numbered package; those files keep working where they
are. The transpositions the definition implies (its pipe notes against the
sample file names) are taken as they are, and every stop plays the keys of
its own rows in the definition: a rank folder used by a bass stop and a
treble stop is loaded ONCE, by the first stop, and the second stop becomes a
transmission of it with the keys of its own rows (Adlington's "Bl. Flute
Bass" and "Bl. Flute Treble"); the keys outside each stop's rows are
switched off in its voicing (an extension's too). A folder is never loaded
twice. A division played by
several keyboards belongs to the keyboard with the most keys; a pedal
keyboard that only pulls a manual's division down leaves P. empty (a note
says so - couple it from that manual). It is loaded at once and saved
again with the voicing once the folders are loaded. Then check
the Stop List, give the perspectives their audio routes and the divisions
their MIDI keyboards, and *Save Configuration* - as a portable one to
publish it.

### Merging another configuration into the loaded organ

Right-click *Load Configuration* > *Merge another configuration into this
organ* adds a second configuration (an OrganSampler `.json`, or a Hauptwerk
organ definition) to the one that is loaded, division by division: the added
pedal joins your pedal, its first manual your first manual, and so on; a
division you do not have yet is created. The added stops become further rows
of the Stop List under the existing rows of their division (drag them where
they belong), their folder trees join yours, and their voicing comes with
them. On the grid the added stops keep the layout they came with (a
Hauptwerk definition's jamb page), set right under your last row; when the
added configuration is the same organ in a further volume (its stops share
folders with your rows), its page becomes the layout of the whole
division - your rows take its cells too, so the picture is the full organ's
page, without empty rows between the old and the new stops. What would collide is left as it is and listed in the message at the
end: a stop whose sample folder is already loaded is shared and not added; a
perspective your organ already has keeps its audio route and settings, a new
perspective brings the added configuration's; a division that has stops
keeps its MIDI keyboard, an empty one takes the added configuration's keys,
wind, tremulant and (when included) MIDI; couplers, combinations, crescendo,
temperament, the global sound settings and the MIDI buttons stay yours. Two
stops of one name in one division: the added one gets "(2)". Two organs
rarely stand at one pitch (a1 = 440, 415, 465 Hz ...), so the merge switches
Temperaments > *Equal at 440 Hz* on: every stop of the merged organ is tuned
to 440 Hz and they play together in tune. The same organ in a further volume
(stops sharing folders with yours) keeps its pitch: nothing is retuned. Before anything
is loaded the program sizes the added sample folders against the memory that
is free now and refuses the merge when they would not fit (nothing is
changed then). The merged organ is loaded but not saved: *Save Configuration*
keeps it.

### Portable configuration (to share with others)

A configuration saved as it is carries the sound cards, the output channels
and the MIDI keyboards of *your* computer, which another computer does not
have. To give an organ to somebody else, tick **Portable configuration (to
share with others)** in the *Save Configuration* box (its "?" opens this
chapter). Such a configuration holds the
organ only - the folder trees, the Stop List, the voicing, the IR reverbs of
the routes, couplers, combinations, crescendo, temperaments - and nothing of
the computer it came from: no MIDI devices, no audio devices (a route that
was none stays none), every route on the output channels 1-2, no
render-thread, interface-size, window or cache settings, no measured
polyphony. Loaded on another computer, every route that had a device - the
auxiliary routes with their reverbs included - plays through that
computer's default sound device at once; the computer keeps its own
interface size and thread settings, and the activity log notes it (no
message box since 0.8.135). Since 0.8.144 every configuration file, portable
or not, leaves the settings of this computer alone: render threads, the
sample storage and its rate, the window places, the measurement limit - a
file saved with other ones never switches them behind your back. What to set up:
the MIDI keyboard of every division (*Settings* in the division header,
right-click the keyboard icon for *MIDI Learn*), the sound card, buffer and
output channels of every route in the audio tabs, then *Measure the maximum
polyphony* in the Configuration Editor and *Save Configuration*. Published
configurations (see "Organ configurations" on the website) are saved this way.

Beside a portable configuration the program writes two more files for
publishing it in the project's `configurations/` folder on GitHub:
`<name>.zip`, the configuration zipped (a zip is downloaded from GitHub with
one click, a bare `.json` would open as text), and `<name>.README.md`, the
README in the published form - the perspectives, the folders the
configuration expects and where to put the file, the routes, the RAM as
one bullet per figure with its storage options (0.8.189): the least the
organ takes (16-bit, 32 kHz, releases at half the rate, packed - measured
when it was loaded so, else estimated at the save from a share of the
loaded samples), the figure measured at the save with the options the organ
was loaded with, and the full quality (32-bit float, the files' own rate,
full releases, no packing - roughly ten times the least)
(`configurations/INDEX.md` shows the least as its RAM column; the price of
those options is polyphony, see the storage options in the Configuration
Editor chapter), the version, and the Stop List as a table in the order of
your grid with the buttons' captions, transmissions and extensions
included. The save asks you once for what the README cannot know - the
organ (instrument, builder, place), the producer of the samples, the link
to the page where the sample folders are bought or downloaded, your name -
and keeps the answers in the configuration; nothing in the README is
filled in by hand. Then send the two files: open the form
https://github.com/zurek-jiri/OrganSampler/issues/new?template=organ_configuration.yml
(a free GitHub account is needed), drag the zip and the README into its
*Files* field and submit. A robot checks them - a real organ configuration
saved with the organ loaded, and the README written for it -, proposes them
as a pull request and answers in the form; nothing else is needed, no
knowledge of Git. After the merge the organ appears as a line of
`configurations/INDEX.md`, the list of all published organs, in a folder
named after the organ as you named it at the save. A new version is sent
the same way and replaces the old one by itself (0.8.188: the organ is known
by that name, whatever the file is called); the form *Remove my
configuration* takes it off the list again.

Paths inside a configuration are stored relative to the common folder of the
sample trees where possible, so a configuration kept beside its sample folders
travels with them (another drive letter, another computer). Trees on a
different disk get absolute paths. A configuration whose trees are not on
this computer loads all the same, with the trees that are here: every
division that uses a missing tree shows an orange notice under its header,
"Sample folder not found", with the folder's name (the full paths are in its
tooltip). Click it: a menu offers *Replace "..." with a folder on this
computer...* for every missing tree - choose the folder where that tree is
now, whatever its name, and the stops come alive at once, their rows keep
their folders - or *Open the Stop List*, where *Unload folder...* also
offers *Replace "..." with another folder...* for every tree and removes a
tree you no longer have. The notice goes when no tree of the division is
missing.

Paths are stored with "/" and resolved on every platform; a configuration
saved on Windows loads on Linux and macOS and the other way round.

## Where configurations and samples live

Since 0.8.190 every organ configuration of a computer lives in ONE folder,
the **configurations folder**: `Documents\OrganSampler\Configurations` by
default (`~/Documents/OrganSampler/Configurations` on macOS and Linux),
chosen anew in the Configuration Editor (*Configurations folder* > Choose).
The Hauptwerk and RAR imports write there, *Save Configuration* and *Load
Configuration* start there. A configuration loaded from anywhere else
(Downloads, beside the samples, inside an archive) is offered a copy into
the folder in the load box (0.8.191, *This file*: copy it, or load it where
it is); the copy becomes the open file, the original stays. The folder can
be moved: nothing in it points at its own place. (Up to 0.8.189 the imports
made an `OrganConfigurations` folder beside every sample set, which nobody
found again.) The setting belongs to the computer, never to a configuration
file.

The samples and the impulse responses are found through the **sample and
IR locations**: the folders under which your sample libraries, archives and
IR collections lie (`D:\Hauptwerk\HauptwerkSampleSetsAndComponents`,
`E:\sonus`, `D:\IR`, a USB disk). A configuration names its trees relative
to its library root (`OrganInstallationPackages/003135/pipe/HW`) with the
root's name, and an IR by its path inside the library (an IR packed with
the organ, as in the Rural Organ) or by its full path (an IR of a
collection elsewhere); at a load each is looked for at the path it had, then
under every location - `<location>/<root name>/<path>`, `<location>/<path>`
and the last one to three parts of the old path - and taken silently when
found, so a sample set or a collection moved within a known location is
found by its name. The program fills the list by itself: the folder above
the library root at every portable save, the root where it was found at a
load, the folder of every archive you choose, the collection of every IR you
choose, the folder you point to when a tree is asked for. The Configuration
Editor shows the list (*Sample and IR locations*), adds a folder by hand and
forgets them all. A tree found nowhere is asked for once, with the "Sample
folder not found" notice under the division; an IR found nowhere shows
"FILE NOT FOUND - choose it" on its route in the Divisions dialog; the
answer joins the locations.

A word to the user: keeping track of where the sample folders and archives
are is a nightmare for the programmer, and every move of a sample set is a
question to you. Think well about where your sample folders go before you
put them there, so that you never have to move them again - one disk, one
folder for the libraries, the archives beside them.

The filled pipes cache has its own folder in the same way (*Filled pipes
cache folder* in the Configuration Editor), a setting of the computer.

**Voicing** (everything set in the voicing rows of the stops) is saved
automatically and also with the configuration; *Save Voicing* and *Load
Voicing* (hidden from the top bar by default) exchange it as a separate file.

A configuration holds only what the instrument uses now: the divisions in
use, the audio routes that exist (the audio tabs of the Divisions dialog)
and their settings. Nothing is kept for later. A division's own audio route
that no stop plays through - every stop of the division has a perspective of
its own - has no tab, and so it has no device either: the program closes it
and remembers none. The same for the routes of divisions beyond the division
count, for a route whose perspective is gone, and for the IR and other
settings of an auxiliary route without a device. When you want something
again, you set it up again.

### GLOBAL RESET

The *GLOBAL RESET* button of the top bar offers what to reset, all unticked:
*Unload everything* empties the instrument completely (stops, folders,
transmissions, couplers, patterns, MIDI, audio routing); the other options
(audio, MIDI, transmissions, patterns, couplers, the grid) reset only what is
ticked and keep the loaded stops.

---

# 2. The main window

From top to bottom: the top bar of buttons, the combination row, the row of
global sound sliders with the polyphony display, and the stop workspace - one
column per division with its header and its stop grid.

## 2.1 Top bar buttons

Every button can be shown or hidden in *Configuration Editor > Console* ("Top
bar: ..."); *Configuration Editor* itself always stays. Every button can be
fired from a MIDI control: right-click it and choose *Learn MIDI On trigger*
(a learned button fires even when the bar hides it).

- **PANIC** silences every sounding pipe at once.
- **Record** records the stereo mix of the organ to a WAV file (a file dialog
  asks where); clicking again stops the recording with a short fade.
- **Stop List** opens the table of the whole organ: see "4. The Stop List
  (building the organ)".
- **Couplers...** opens the couplers dialog: see "Couplers".
- **Temperaments** opens the temperament dialog: see "Temperaments".
- **Hide all voicing** folds every open voicing row on the grid (each row
  opens again with its own corner button).
- **Perspective Patterns..., Ignored Patterns..., One-shot Patterns...** edit
  the folder-name lists (the same lists as in the Configuration Editor, see
  "Folder name patterns (one per line)").
- **Stop-Merging** renders the enabled stops into one new stop: see
  "Stop-Merging".
- **Load Voicing / Save Voicing** (hidden by default): the voicing as a file.
- **Load Configuration / Save Configuration**: the instrument as a file.
- **Shut down PC** (hidden by default): saves the open configuration and the
  settings, quits and shuts the computer down at once. No dialog, no question
  about the configuration, nothing to answer: it is meant for headless
  consoles (a MIDI piston on a console without a screen), where nobody could
  react. An instrument without an open configuration file is kept as
  "Shutdown rescue configuration.json" beside the settings.
- **GLOBAL RESET**: see "GLOBAL RESET".
- **Configuration Editor**: every program setting in one scrolling dialog:
  see "8. Configuration Editor reference".

## 2.2 The combination row

The row holds the combination pistons (their number is set with the
*Pistons* box in the row, or in *Configuration Editor > Console > Combination
pistons*), the *Set* button, the *<<* and *>>* stepping buttons, the bank
buttons (*Bank ▼* / *Bank ▲* with the bank's number), the display of the
combination in use, *Tutti* and *X* (all stops on / all off), *Save* and
*Load* for combination files, and the crescendo.

- Click a piston to recall its registration (stops, transmissions, couplers,
  tremulants, Unison Off).
- With *Set* on, clicking a piston stores the current registration in it.
- *<<* and *>>* step through the pistons, the bank buttons through the banks
  of pistons; all are MIDI-learnable, as is every piston (right-click).
- *Tutti* switches every stop on, *X* every stop off.
- *Save* / *Load* write and read all pistons as one file. Combinations are also
  part of the configuration.

## 2.3 The crescendo

The crescendo display is a row of segments, one per step (30 by default).
Drag it, or move a learned MIDI controller (right-click the display: *Learn
MIDI continuous controller*). *Cresc.* switches the crescendo on: switching it
on remembers the registration set by hand, switching it off brings that
registration back. *Crescendo settings...* (right-click the display) opens the
dialog in which every step is taught: switch *Set* on, engage the stops, click
the step. Moving from one step to the next switches only the difference: the
stops, transmissions and couplers the two steps have in common stay as they
sound, so a chord held through the crescendo never restarts; only what the
new step adds starts to sound and only what it drops stops. The dialog also sets the
number of steps, whether the crescendo may switch off hand-registered stops
(otherwise the steps only add), clears all steps, and saves or loads the steps
as a file.

## 2.4 Global sound sliders and the polyphony display

The sliders of *Configuration Editor > Global sound* can each be shown on the
main panel (the checkbox beside them in the editor). Master volume, pitch,
sustain fade-out, release fade-in, release volume, release cutting, attack
minimum time and delay: all are described in "Global sound". Every one of
them is MIDI-learnable (right-click). The polyphony display shows the voices
sounding now (*NOW*), the most at once over the last seconds (*PEAK*), the
*DSP* load - the time the busiest audio thread needed for its last block as a
share of the block's duration; above 100 % that thread could not deliver in
time (crackle), see "Getting the most polyphony" -, the memory the program
holds (*RAM GB*, whole gigabytes rounded up; orange when less than a tenth
of the installed memory is free: the system then pages samples out and a
block stalls on reading them back), and the polyphony *LIMIT* (editable; the
measurement writes its verdict into it the moment its report opens).

The loading never lets the system page: before every stop (and every 32
files inside a large one) the program looks at the memory and stops loading
when any sign says it is short - less free than 512 MB or a sixteenth of the
installed memory, its own footprint within the system's share (1.5 GB or a
fifth; 1 GB on a computer of 4 GB and less), the program swapped out by more
than 256 MB since the load began (Linux; on macOS the system's compressor
growing that much while less than twice the reserve is free - it grows as
macOS squeezes other programs, which is what it should do), or the macOS
kernel reporting memory pressure. The stops loaded so far make the
instrument; a stop cut in the middle of its files, or with one perspective
loaded and another not, is dropped whole, and every stop left out is named
once in the message and stays in the pool of the Stop List - nothing stands
on the grid half loaded. The polyphony report then says the loading stopped
on purpose; it never claims the system is paging when it is not. A folder
tree that lies inside another loaded tree (the whole library and one of its
sub-folders both loaded) is the same folders twice: the inner tree is
dropped from the pool, its stops point into the outer tree, and a message
after the load names it (0.8.143) - load a tree once, whole or in parts,
never both. A stop
whose samples the program holds already (a reload of the same organ: the
new load adopts them, nothing is decoded) is loaded whatever the memory
says. The loading dialog has a *Cancel* button (0.8.139): the loading stops
at the next stop, the instrument is built from what was loaded, the rest is
named like after a memory stop and stays in the pool. A stop left out of
the load keeps its row in the Stop List and is offered in the pool of its
division ("Add stop" on the grid, 0.8.142): taking it reloads the division
with that row, no second row is made.

The report also profiles the device thread (0.8.137): per route callback,
in microseconds, how much went into this thread's own rendering of voices,
into waiting for the other lanes and merging their output, into the mixing
(block context, harmonic shelf, channel separation, safety clip), into the
IR hand-over (and of it, waiting for the convolution task) and into the
bookkeeping (voices in and out, recording, load statistics); with the period
for scale and the share of the time the device threads spent inside the
callbacks, and the longest route callback of the run with its parts - the
sound breaks on such a peak, never on the average, and how late a lane
joined a job after it opened. It is what tells, on a
computer whose render cores are not full when the sound breaks, where the
time goes. Since 0.8.143 the render workers wake ahead of every period
(the cadence is known) instead of being woken by the job, and the device
thread takes no more voices at the tail of a job when the lanes still at
work finish them sooner: the peaks of 2-2.5 ms that limited a 16-core
computer at 55 000 voices were the device thread rendering alone while the
workers woke up. The limit the measurement judges by - how much of its period the
longest driver callback may take - is the *Measurement limit* of the
Configuration Editor (0.8.139): 85 % by default, which leaves headroom for
what the computer does besides; up to 100 %, where a level counts as broken
only when the driver actually called late - dangerous for playing, the
limit it sets breaks up on the first callback that runs a little longer.
The measurement does not start while the filled pipes are still being built
(the orange tracks under the stop buttons), and the report says when the
run ran out of keys and strikes before the computer did. The "free" figure is what the
system could give the program now: Windows' available physical memory,
Linux's MemAvailable (the reclaimable cache included), macOS's free and
inactive pages; the footprint rule is the one that bites on an 8 GB machine.

## 2.5 Division headers

Each division column starts with a one-line header: a keyboard icon that
lights when a key is pressed (right-click it: *MIDI Learn...* learns the
division's keyboard from the next key pressed, *Clear learned keyboard MIDI*
forgets it), the division's name, the swell display (drag it, or right-click it
to learn the swell pedal's MIDI controller), the *Tremulant* button
(registration, like a stop) and *Settings*, which opens the Divisions dialog at
that division. Drag the dividers between the columns to share out the width.

Every feature has its settings where it sits, by right-click: the swell bar's
menu offers *Swell settings...* (maximum attenuation, shutter inertia,
harmonic filter), the *Tremulant* button's menu *Tremulant settings...*
(rate, depth, drift between ranks, volume per cent, harmonic filter and its
phase), and a right-click on the header's own background lists the swell,
tremulant and wind settings of the division together with the Divisions
dialog and the Configuration Editor. The rows are the same as the
Configuration Editor's and take effect at once; a row marked "(all
divisions)" is one setting for every division. The crescendo display and
the IR caption of an audio tab have such menus too.
The mouse wheel scrolls the workspace everywhere - except inside the
rectangle of an open voicing (a stop's row with its per-key sliders), where
it moves the sliders; the rest of that division's column scrolls as usual.
The button of a stop folder switches all its children with it, their buttons
included; a child can still be switched alone to hear it by itself.

## 2.6 Two program windows

*Configuration Editor > Console > Program windows* opens a second window
(empty at first). In the Configuration Editor's division sections, *Shown in
window* moves a division to window 1 or 2; a division is in one window only.
The second window's position is remembered; closing it returns to one window.

## 2.7 Screens and the size of the interface

Every size in the program is a design size for a 2560 x 1440 screen at the
system's 100 %; the operating system's display scale applies on top. *Interface
size* (Configuration Editor > Console) is the program's own factor over that,
estimated at the first start so that Full HD, 4K and Retina screens show the
same amount of console, then saved and never changed by the program. Dialogs,
menus, tooltips and message boxes follow it; text stays sharp at every size.

## 2.8 Dialogs on small screens

Every dialog sits in a scrolling viewport: when its window is smaller than the
dialog, scrollbars appear and every control stays reachable.

---

# 3. Stops on the grid and their voicing

## 3.1 Stop buttons

A stop is a button on the grid of its division: click to engage it, click
again to disengage it. Engaged buttons are orange. The small corner button of
a stop ("+") opens its **voicing row**; "-" folds it again. *Hide all voicing*
in the top bar folds every row.

A button carrying "[+N]" is a **stop folder**: a stop with N other stops
indented under it; its button switches them all. Open its voicing and the
children appear under it, each with its own button (for voicing them one at a
time) and voicing row.

A button whose caption ends in "(Trsm.)" or "(Ext.)" is a **transmission**:
the pipes of another stop played here, possibly transposed. "(Ext.)" marks an
extension - the stop's own pipes in its own division at another footage (a
16' or 4' extension of an 8' stop); "(Trsm.)" marks a transmission into
another division, at the same footage or at another one. An alias carries the
same mark after it. "Transmissions and extensions" and "What a filled
pipe is" explain transmissions and the pipes they
need beyond the stop's compass.

## 3.2 Arranging the grid

Press a stop button for two seconds, release, and the whole console enters
**arrange mode**: every stop and transmission can be dragged to another cell of
its division or into another division. A click on empty workspace ends the
mode. While arranging:

- Dragging a stop into another division's grid moves it there with its
  voicing and transmissions.
- Dragging with the right mouse button, or with Shift held, makes a **copy**: a
  new transmission of the stop appears where the copy lands (also in the
  stop's own division, as an extension). A copied transmission keeps its
  transposition.
- Every button carries a red x in its top-left corner: it removes the stop (a
  warning lists its transmissions first; they go with it) or the transmission.
  A stop folder with stops still indented under it is not removed; take them
  out first.
- **Making a stop folder:** let the dragged item rest over another stop's
  button until a dim orange bar appears under that button, then pull it a
  little *down* onto the bar; it lights up, and the drop indents the item under
  that stop. Pull back up and the drop is an ordinary move or swap again. A
  quick drop never indents. This works for a copy too: a new extension or
  transmission dragged with the right button or with Shift and dropped on
  the lit bar is created straight inside the folder. Dragging an item out of
  an open voicing onto a cell of the grid takes it out of the folder.
- A **free cell** (dashed) offers, on a click, "New stop folder here" (an empty
  stop ready to take children), "New stop here" with the ready-made stops of
  the loaded folders that no division of the instrument uses yet, and the
  division's pooled stops and transmissions, which return to the grid in that
  cell. A stop already in use anywhere is not offered: it is moved on the
  grid (arrange mode) or given a transmission instead.

None of this loads anything again: stops keep their samples and the copies of
their filled pipes.

## 3.3 The voicing row

Open it with the corner button. Its first line shows the transposition, the
perspectives, for a stop with velocity layers one tick box per layer, the
pitch information, the alias and the switches; the second line the sliders:

- **Transposition** (right after the button of every stop with pipes) - the
  stop's transposition against its sample files on their own numbers (the
  file "036" on key 36), every semitone from -24 to +24 as a pure interval
  with the footage each step gives: "0 -> 8' (as the original)", "-12 -> 16'",
  "+12 -> 4'", "+7 -> 5 1/3'", "+19 -> 2 2/3'", and for the other steps the
  pure (just) interval's fraction - "+1 -> 7 1/2'", "+3 -> 6 2/3'",
  "+5 -> 6'", "+9 -> 4 4/5'" (octaves printed bold). "The original" is what
  the samples' own pitch (the smpl chunk) says the files sound at on their
  numbers. -12 puts the file 024 on key 36, an octave down. The *Orig. Pitch*
  line of the row shows that real footage of the files; the footage in
  the button's caption follows the transposition, and so do the stop's transmissions and
  extensions, which add their own transposition on top. A change loads the
  stop's samples again on their keys at once (the fills follow; the keys
  held fall silent first and the stop starts no voice while it reloads). The same
  setting is the Stop List column "Transp.". Each key's tooltip in the rows
  below names the file it plays.
  The program sets it by itself where it can: a Hauptwerk import takes it
  from the definition (its pipe notes against the file names: Doesburg's
  files sound 8' on their numbers and the definition puts "024" on key 36,
  so its 8' stops read "-12 -> 8'" and its 16' stops "-12 -> 16'"); a folder
  loaded on its own gets it from the samples' pitch against the footage
  written in the folder's name - "quintaton 16" whose files sound 8' on
  their numbers gets -12, and the activity log says so ("Transposition of
  ..."). Only a whole difference of two semitones or more, agreed by most
  of the pipes, counts: an organ standing a semitone low or high as a whole
  (a1 = 409 Hz) is a tuning, not a transposition, and stays at 0. The value
  found is kept in the configuration; a folder without a footage in its name
  is left at 0 - transpose it yourself. Clearing the Stop List cell makes the
  program find it again at the next load.
- **Alias** - the caption of the button. *Automatic* shows the folder name;
  type any text to show that instead (an alias without a footage gets the
  stop's footage appended, and a footage written in the alias follows the
  transposition). The alias is only a caption: saved states,
  combinations and MIDI learn keep using the stop's real name, so renaming
  breaks nothing. A Hauptwerk import writes the definition's stop names
  without their footage, so the footage shown is always the sounding one.
- **One-shot** (after the alias, every stop with pipes) - the stop plays each
  sample once from start to end whenever the key is released (chimes, harp,
  celesta): nothing of it is cut or faded by the key going up. The switch
  starts as the *One-shot Patterns* say for the folder's name and overrides
  them for this stop once touched; saved with the voicing.
- **Rank count [II]** (after the alias; shown on stop folders only, and on a
  stop with items indented under it) - whether the button carries the Roman
  numeral of the ranks that sound with it. Off: the name alone. On by
  default; saved with the configuration.
- **Perspectives** - the microphone perspectives this stop has, each with its
  own on/off tick box (their volumes are the routes' volumes in the Divisions
  dialog).
- **Stop volume** (-24 to +12 dB, default 0) - the level of the whole stop.
- **Velocity layers** - a tick box per layer and one for the plain files; see
  "Velocity layers" below.
- **Attack Gain** (-6 to +6 dB) - the start of every note is raised or
  lowered by this much: more chiff, or a softer speaking, without touching the
  held tone.
- **Attack Time** (0-500 ms, default 150) - how long the attack gain acts at
  the start of the note: it holds for the first half of this time and fades
  back to the normal level over the second half.
- **Random pitch error** (0-40 cents, default 4) - every pipe gets its own
  small random detune at every note, as real pipes never speak twice alike.
  The division's *Random tuning error of pipes* switch must be on.
- **Undulation (beats/s at a1)** - for celestes: the stop beats this many
  times per second against an in-tune rank at a1 (negative = flat). A rank
  recorded sharp or flat throughout is recognised as a celeste and starts with
  its recorded undulation; the slider overrides it once moved. How much faster
  the top of the compass beats than the bottom is *Celeste beats* in the
  Configuration Editor. The line reads "needs 440 Hz" when the set's pitch
  has not been retuned to a1 = 440 Hz yet (Temperaments dialog).
- **Formants (filled pipes)** (0-100 %, default 30) - the timbre of the pipes
  this stop lacks and borrows from neighbours with a pitch shift: see "The
  settings that go with the methods".
- **Orig. Pitch / temperament** - a line of information: the recorded pitch
  (a1 = ... Hz), the footage read from the samples (with the sounding footage
  of a mixture or mutation in brackets, and the breaks), and the temperament
  the recording was found to be in.

Under the sliders are two folding rows of per-key sliders (orange headers;
each stop remembers its own folding):

- **Volume per key** (-9 to +9 dB) - the level of every pipe. Under every note
  name a tick box switches the pipe on or off: an unticked pipe is silent on
  the stop's own button and is never filled from a neighbour, so a cornet or
  any stop that must not have the full compass gets no extension. The stop's
  transmissions are not affected (each transmission has its own boxes).
- **Treble per key** (-9 to +9 dB) - a high shelf per pipe, above the
  frequency set in *Treble per key: shelf start frequency* (default 1800 Hz).
- A *Reset to 0 dB* button sits beside each header; dragging with the right
  mouse button paints one value across a row of sliders.
- The sliders light up while the key sounds. A **green thumb** marks a filled
  pipe (hover it: the tooltip names the stop, the sample file and the shift it
  was made with, "HW Tromp 16', sample 039-D#.wav, -12 st (from key 51)"). An
  **orange note name** means the sample has no loop points. A **grey slider**
  means the stop has no pipe there at all.

## 3.4 Transmissions and extensions

A transmission is an extra button that plays the pipes of another stop, on any
division (its own included). It is made in the Stop List (the *Transmissions*
cell of the stop's row) or on the grid (a right-button or Shift drag in arrange
mode). Each transmission has its own voicing row, opened like a stop's, with:

- **Transposition (extension)** - the key sounds the pipe this many semitones
  away, -24 to +24, every step a pure interval written as the footage it
  gives (octaves printed bold). -12 makes a 16' extension of an 8' stop,
  +12 a 4' one; the button caption follows: "Principal 16' (Ext.)" in the
  stop's own division, "Principal 16' (Trsm.)" in another division.
  It counts from the source stop as it sounds, its own transposition included.
  The shifted pipe is retuned so that the new footage is a pure interval of
  the played key (a 2 2/3' shifted by +5 gives an exact 2'), the temperament
  follows the played key, wind, tremulant and detune stay with the pipe.
- **Taken from** - the source stop, its division and footage.
- **Volume vs. parent stop** (-10 to +10 dB), the per-key volume and treble
  sliders and the pipe on/off boxes of the transmission alone.
- **Formants (filled pipes)** for the pipes only this transmission asks for
  (beyond the stop's compass).

A pipe is one set of voices, no matter how many keys hold it: the same pipe
reached through the stop itself, through a transmission and through a coupler
sounds once, and stops only when no route holds it any more. While several
routes hold it, the loudest of them sets its volume and the highest *Treble
per key* setting among them sets its tone; when the loudest route lets go,
the volume glides to the next loudest, and the tone follows in the same way.
The tuning is different: a pipe can be retuned differently in different
transpositions (the pure intervals of the extensions, see above), and a
sounding pipe cannot change its pitch. So the pipe keeps the tuning of the
key that started it for as long as it sounds - even after that key has been
released, while other keys still hold it; only when it has stopped and is
started again does it take the tuning of the new key. Switching a
transmission off releases exactly the pipes that no other route holds.
Keys whose pipe lies beyond the rank's compass (the bottom octave of a 16'
extension) play **filled pipes**: see "What a filled pipe is".

## 3.5 Velocity layers

An organ key never plays softer when it is struck gently, so the velocity
never changes the volume of the held note. What it does depends on the stop:

- A stop **with velocity layers** plays the layer the velocity falls into, at
  that layer's own level - a gentle stroke selects the recording of a gentle
  stroke.
- A stop **without velocity layers** softens the attack of a gentle stroke,
  in volume and in colour, over its first few tens of milliseconds: the note
  starts with a short fade-in (*Key velocity: fade-in at the lowest
  velocity*, default 40 ms at the lowest velocity) and with its treble cut
  (*Key velocity: treble cut on the attack at the lowest velocity*, default
  9 dB), which opens up again over *Key velocity: treble cut opens over*
  (default 80 ms). Both effects are strongest at velocity 1 and shrink to
  nothing at velocity 127; the held tone is not touched. The result is a
  chiff that is blunted by a gentle touch, as on a tracker action.

A sample file is a velocity layer when its name ends in "_NNN" (two or three
digits, 1 to 127, after an underscore): `060-C4_070.wav` plays for key
velocities up to 70, above that the plain `060-C4.wav` plays, or the next
layer up when there are several (`_025`, `_070`, `_100`). A key that has layers
but no plain file plays its highest layer for every velocity. Release files
carry the same suffix: the release of a "_070" attack is the "_070" release
from the rel folder; a velocity without a release of its own takes the nearest
one that exists. Filled pipes follow the layers as well.

The voicing row of a stop with layers shows a tick box per layer, named by the
suffix ("_070"), and one for the plain files, the top layer (its box reads
"_128"). Unticked, a
layer's velocities use the next layer up or the plain samples; the plain layer
unticked sends its velocities to the highest ticked layer and the plain files
never play (0.8.187). When nothing is ticked, nothing plays. The choice is
saved with the voicing. The keyboard drawing of the Divisions dialog shows the
MIDI velocity of the key pressed last in a small badge, so you can see which
layer a stroke reaches. The special use the tick boxes are there for: some
organs keep the original recordings as "_070" and fine-tuned versions (the
harmonics tuned inside the audio, a rare treatment for detuned mixtures) as
the plain files. Untick "_070" and only the fine-tuned samples sound, whatever
the velocity. *Ignore MIDI velocity (organ keys)* in a division's settings
sends every key as 127: always the top layer.

## 3.6 Stop-Merging

*Stop-Merging* (top bar, hidden by default) renders the engaged stops and
transmissions, with their whole voicing (volume, treble, attack, per-key
settings, transposition), into one new stop per perspective: a folder named
after the stops (`StopMerged_Principal_8_Octava_4`) inside the loaded folder
tree, with proper WAV files, loop points and release folders, ready to load as
an ordinary stop. A transmission merged alone gives a complete stop of the
transposed rank - a way to turn a pitch-shifted extension into files. While
the copies of a transmission's filled pipes are still being built the merge
waits with a message.

---

# 4. The Stop List (building the organ)

*Stop List* in the top bar (also *Stop building...* in the Divisions dialog and
in every division section of the Configuration Editor) opens one table for
the whole organ: one row per stop with its division, name, alias, "Files vs
keys" and transmissions, and one column per perspective holding the sample
folder that plays that perspective of the stop.

## 4.1 Folder trees and the pool

- **Load folder...** loads a folder tree (the samples of an organ, or a part of them) and
  asks which division its stops go to. One window: the folder tree, or the
  RAR archive(s) the samples came in - read as the folders they hold, or
  unpacked first when the switch says so (see "Sample archives (RAR)").
  A tree is loaded once and its folders
  can serve any division. When a tree of the organ is missing on this
  computer (a configuration from elsewhere: the division header says
  "Sample folder not found"), a loaded folder of the same name takes the
  missing tree's place and the stops built from it keep their samples.
- **Unload folder...** removes one tree from the whole organ after saying how
  many stops lose their samples (their rows stay as empty stops). **Unload all
  folder trees** empties the organ; the Stop List opens again with the empty
  rows, *Remove empty stops* clears them and *Apply and reload* applies that
  even though no tree is loaded. The same menu offers **Replace "..." with
  another folder...** for every tree: the tree is taken from another folder
  (a moved disk, a configuration from another computer), its rows keep their
  folders and the divisions that use it reload.
- The table starts out as the automatic rules built the divisions (from the
  folder names and the perspective prefixes), so it shows what was
  recognised, and it can be corrected where the rules could not pair the
  folders - when the perspectives of one stop live in two folder trees, for
  example.
- The info line at the bottom counts the sample folders found in all trees,
  how many the table uses and how many are free.

## 4.2 Editing the table

- **Order:** drag a stop name up or down. The order is the load order and the
  default order on the grid. When rows were dragged, *Apply* asks whether the
  grid should follow the new order; otherwise every stop keeps its cell.
- **Division:** drag a row into another division's block, or choose the
  division in the row's combo box. The stop moves with its folders; voicing,
  grid cell and transmissions start afresh.
- **Stop folders:** drag a name a little to the *right* and the row is
  indented under the row above: it becomes that stop's child and sounds with
  it. Drag it *left* to make it independent again. One level only.
- **Folder cells:** click a cell to choose one of the *free* folders of all
  loaded trees (a folder serves one cell only); the small x beside a cell
  frees its folder. A cell can also be dragged onto another cell (empty: takes
  the folder; occupied: swaps). The whole path appears in a tip while the
  mouse rests on the cell.
- **Transmissions:** the cell shows where the stop's transmissions go
  (division and transposition); click it to add one into any division (its
  own included, as often as wanted) or to remove one. Transmissions act at
  once; they need no Apply. With **Show Trsm.** (bottom bar) every
  transmission is also listed as a grey row of the division it plays in,
  movable like a stop.
- **Add stop...** adds, per division, an empty stop folder or one of the
  ready-made stops of the loaded trees that no division uses yet (added
  whole, with the folders of every perspective; a stop whose folders are in
  the table already is not offered). The x at a row's start
  removes the stop (its folders become free). **Remove empty stops** removes
  every row without folders that has no children.
- **Aliases from folders** gives every stop without an alias the bare name
  of its sample folder: no parent folders ("TA - fletna" becomes "Fletna",
  shown as "Fletna 8'"), no perspective prefix, a capital first letter, a
  footage at the end of the folder name with its foot mark; a stop without a
  footage in its name shows the measured one (a mixture's first sounding
  harmonic). Aliases you typed stay. Applied with *Apply and reload*.
- **Add perspective...** adds a column (a known perspective name or a new
  one); the x right after the perspective's name in the column header
  removes it. "default" is the route of folders without a perspective prefix.
- **Swap divisions...** exchanges the whole stop lists of two divisions, as if
  their folders had been swapped (combinations and MIDI bindings follow).
  **Reset division...** forgets one division's table.
- **Name:** derived from the folders; a typed name is kept until the field is
  cleared. The name is the stop's identity in voicing, combinations and
  configurations; the alias is only what is shown.
- **Transp.:** the stop's transposition in semitones against its files on
  their own numbers; a pure interval: -12 puts the file 024 on key 36 (an
  octave down: Doesburg's stops, a 16' made of files that sound 8'), +12 an
  octave up, +7 a fifth. Empty ("auto"): found by the program from the
  samples' pitch and the footage in the folder's name, or from a Hauptwerk
  definition, and written here once found. The voicing of every stop offers
  the same setting (with the footage each step gives) and applies it at once;
  applied here with *Apply and reload* (see "The voicing row").

## 4.3 Apply and reload

*Apply and reload* (grey while the table equals what is loaded) saves the table
and loads the changed divisions from it; unchanged divisions are left alone.
Stops whose folders are already loaded keep their samples and copies; only new
folders are read and only missing copies are built. Folders left out of the
table are removed from RAM. A stop that has transmissions cannot be removed or renamed
without a warning: its transmissions die with it. The table travels with the
configuration and is used at every later load. Closing the window with
changes not applied asks: *Apply and reload*, *Discard* or *Cancel* (back to
the table) - nothing is lost by the close button alone.

## 4.4 What the loading dialog shows

Loading is quick, so the dialog is rarely looked at: it shows the samples being
read per division and in total, then *Compiling the Stop List*, the steps that
build the console, each with its percentage. Opening an ASIO driver is a step
of its own and can take a few seconds. Only when the dialog closes are the
copies of the filled pipes (see "The transposition methods") built in the
background: a dark track
along the bottom of a stop button means the stop still waits, an orange fill
shows the pipes done. The organ can be played at once; a filled pipe plays in
a simpler way until its copy exists.

---

# 5. The Divisions dialog

*Settings* in a division header opens the dialog at that division; the
division is chosen at the top. The settings of a division are the same as in
its section of the Configuration Editor ("Division sections (Pedal, Manual I
... Manual VII)" describes every one of them); the dialog adds the keyboard
drawing, the coupler and swell learning, and the audio tabs.

## 5.1 Keyboard, MIDI and compass

- *MIDI keyboard device* and *MIDI channel*. When another division listens to
  the same device and channel a note "Shared with ..." appears (one key then
  plays all of them). A keyboard plugged in while the program runs is opened
  at once (the activity log says so) and MIDI learn hears it; open the
  Divisions dialog again to see it in the device list. One unplugged is let
  go and comes back when plugged in again.
- *Ignore MIDI velocity (organ keys)*: every key is sent as full velocity.
- *Bass octave layout*: Standard, Short octave, "C D D# ...", "C D E ..." for
  historic bass octaves.
- *Compass*: two boxes for the lowest and the highest key of the division
  (defaults: pedal 36 to 67, manuals 36 to 96). Keys outside are silent on
  every stop and transmission of the division; the per-key voicing sliders
  cover exactly this range. An extension ends with the compass.
- *MIDI transposition* (the last line of the MIDI column, default 0): the
  keys of the MIDI keyboard play this many semitones away. With -12 the key 036 plays the
  samples of 024, with +12 those of 048. For a keyboard whose keys send the
  wrong octave, or for playing a transposed part.
- The keyboard drawing shows the key pressed last with its MIDI velocity in a
  small badge, so you can see which velocity layer a stroke reaches.

## 5.2 Swell, tremulant, wind

The swell pedal is learned by right-clicking the swell bar in the division
header. *Swell: maximum attenuation when closed* and *Swell: shutter inertia*
shape it. The *Tremulant* header (click to fold) opens rate, depth and the
phase drift between ranks; the *Wind model* header opens depth, frequency,
oscillations, strength, settling lag and the divisions fed from the same wind.
All of them are described in "Division sections (Pedal, Manual I ... Manual
VII)", from "Swell: maximum attenuation when closed" on.

The tremulant's strength wanders between 60 and 100 % of the set depth, and
every key stroke shakes the wind with a random strength, so nothing repeats
mechanically.

## 5.3 The audio tabs

Every perspective of the division has an audio tab (the default perspective,
Direct, Diffuse, Rear ... as the sample folders have them), plus one tab
**auxiliary** (see "The auxiliary route" below). Every tab is one **audio
route**: the pipes of that perspective rendered to that device and those
channels. Which devices and drivers to use is explained in "Audio drivers:
many sound cards at once" below. Each tab, top to bottom:

- **Perspective volume** (-31 dB switches the perspective off) and **Channel
  separation**: at 100 % every pipe sounds only on its own channel pair (mono
  samples are spread over the pairs, stereo samples keep theirs); below 100 %
  every channel of the route also carries that share of the other channels; at
  0 % all channels carry the same sum.
- **IR reverb**: *Load IR* (a WAV, AIFF or FLAC impulse response: mono,
  stereo, or true stereo with four channels LL LR RL RR), *Clear*, the IR
  caption (hover it for everything known about the IR; right-click it for the
  IR settings), *Reverb dry/wet*, and *Set effects for all ...*. The IR
  settings: *High cut* and *Low cut* on the wet signal (they grow with the
  dry/wet ratio; defaults 2000 Hz and 195 Hz, so more reverb also means a
  rounder sound), *Pre-delay* (the wet signal runs one audio buffer behind
  the dry one in any case - 5 ms at a buffer of 256, 10 ms on Windows Audio;
  the set pre-delay includes that), *Wet volume* (default -4 dB, so that more wet
  makes the whole quieter rather than louder), *Decay* (shortens the IR to
  this share of its length) and *Fade-out* (the share of the used IR's end
  that dies away along a gentle curve; proposed automatically when *Decay* is
  moved; 0 % = the IR exactly as recorded).
- **The device box**: audio device type, device, sample rate, buffer size,
  and under it *Driver Control Panel* (the driver's own settings, e.g. the RME
  buffer size), *Reopen device* and *Set for all ...*.

*Set for all* copies the device, sample rate and buffer size of this tab (the
rate and buffer size the device really runs at, as its boxes show them) to
every perspective of every division ("none" is copied too); on the auxiliary
tab it acts on the auxiliary routes of all divisions. *Set effects for all*
does the same for the IR, dry/wet, IR settings, perspective volume and channel
separation; the box unticks itself when one route's effects change afterwards.

## 5.4 Sample channels and output channels

Every route plays on the output channels ticked in the list on the left of
its tab - any channels of the device, in any number. The pipes of the route
are spread over those channels by their key number, so that neighbouring keys
come from neighbouring channels and a chord spreads across the speakers like
the pipes across a windchest:

- A **mono sample** goes to one channel: key after key, the ticked channels
  are taken in turn (with four channels: C on the first, C# on the second, D
  on the third, D# on the fourth, E on the first again ...).
- A **stereo sample** goes to a pair of channels, its left channel to one and
  its right channel to the next. With an **even** number of ticked channels
  the pairs are fixed (the first and second, the third and fourth ...) and
  neighbouring keys alternate between the pairs. With an **odd** number of
  ticked channels the pairs shift by one channel from key to key, so the
  stereo samples circulate through all the channels.
- With a single output channel everything plays on it (a stereo sample mixed
  down).

*Channel separation* below 100 % lets every channel of the route also carry a
share of the others, which softens this spreading; at 0 % all channels carry
the same sum. The IR reverb of a route is convolved per pair of its channels.

## 5.5 The auxiliary route

The **auxiliary** tab of every division is one more audio route: a mix of
everything the division plays - all its perspectives together, every pipe
rendered once more into it - with its own device, output channels, volume,
channel separation and IR reverb.

What it is good for:

- **Monitoring.** Send the auxiliary route to headphones or to the speakers of
  a control room: you hear the whole division in one mix while the
  perspectives play through their own outputs.
- **Dry samples with IR reverb.** Here the auxiliary route is
  irreplaceable. A dry set (close microphones, no room) gets its room from an
  impulse response on its routes. Give the auxiliary route the rear speakers
  and a *different* IR than the main routes (a rear or more distant impulse
  response): the result is a fake "Rear" perspective that the set never had.

What it costs: an auxiliary route with a device means every sounding pipe is
rendered twice (once in its perspective, once in the mix), so it uses up the
DSP time twice as fast and the reachable polyphony is halved. For normal use
leave the auxiliary device at *none* - then the route costs nothing at all.
With dry samples the halved polyphony does not matter: dry samples have a
single perspective and are light to render anyway.

*Set for the auxiliary routes of all divisions* on the auxiliary tab copies
its device to the auxiliary routes of every division; with *none* it switches
them all off at once.

## 5.6 Audio drivers: many sound cards at once

OrganSampler can play through **several sound cards at the same time** - every
division, every perspective, every auxiliary route on the device you choose -
and it can even **mix different kinds of driver** in one organ. An organ can
spread over every output of every interface in the room: the Hauptwerk
division on an RME, the Rückpositiv on a second card, the pedal on a third,
headphones on the on-board sound chip. Which drivers there are and how to
set them up differs by operating system: see "Audio on Windows", "Audio on
Linux" and "Audio on macOS" below. Two things hold everywhere:

**The buffer size decides more than anything else.** Every block of audio has
exactly its buffer's duration to be rendered: at 64 samples (1.3 ms at
48 kHz) the sound breaks at a few hundred voices, however the routes and
IRs are set; at 512 samples (10.7 ms, unnoticeable on an organ) the same
computer renders tens of thousands. 512 is the default for a device opened
for the first time (ASIO keeps its control panel's size, JACK the server's
shared clock), and the value to measure the polyphony with. A measurement
that finds a buffer below 256 samples on any route says so first and asks
for 512, or 256 at the least, before anything else.

**One rate for every device.** Every audio route renders at the rate of its
device, and the devices of one organ may run at different rates and buffer
sizes - the program does not stop you. The samples are loaded at 48 kHz (see
"Sample rate on loading"): a device at 44.1 or 96 kHz gets its route's pipes
read at the matching speed, through the same interpolation as a transposed
pipe, which costs a little more per voice and is a step below the quality of
the resampling on load. So give every device 48 kHz and the same buffer size.
A device opened for the first time, or chosen for a route that had none, runs
at the loading rate, 48 kHz, when it offers it (a fresh device would
otherwise open at the lowest rate from 44.1 kHz up); a device chosen for a
route keeps the rate the route ran at when it offers it; a portable
configuration carries the rate and buffer of every route with it. *Set for
all* copies the rate the device of that tab really runs at - look at the
rate box before you copy it.

### Audio on Windows: many sound cards at once

ASIO devices, Windows Audio (WASAPI) devices and the Aggregated WASAPI
device can all be open at once. No other organ sampler on Windows does this.

The kinds of driver:

- **ASIO**: the professional driver of an audio interface. The lowest
  latency, the most polyphony. One ASIO driver is opened once and shared by
  every route that uses it; a second interface with its own ASIO driver is
  opened beside it. Use ASIO for every route when you can. Its buffer size
  is set in the driver's control panel (*Driver Control Panel* in the audio
  tab opens it).
- **Windows Audio (WASAPI)**: the driver of any Windows sound device, the
  on-board chip included. Larger buffers, more latency, less polyphony per
  route than ASIO, but it works with every device.
- **Aggregated WASAPI**: several Windows sound devices joined into one device
  with all their channels, for a route that must play on more channels than
  one card has. Every Windows sound device appears as a stereo pair in the
  channel list of the tab; the ticked channels are the devices the route
  opens - exactly those, on every path (loading a configuration, *Set for
  all*, a switch of the type, a reopen). A device that cannot be opened (an
  unplugged interface, a digital output with no cable) is refused: its
  channels are set to none, the list shows them unticked with the reason
  after their names, and the activity log names them - the same as the
  Windows Audio type, which refuses such a device outright. No ticked
  channel means none: the route is silent until you tick some. Every
  aggregated route is its own stream with its own thread (see "Render
  threads for Aggregated WASAPI"). Keep its buffer at 480 samples or more
  (the default): the Windows audio engine feeds a shared stream in 10 ms
  packets of 480 samples at 48 kHz, so a smaller buffer only splits each
  packet into shorter deadlines - an IR's tail FFT alone can exceed a block
  of 256 - without lowering the latency at all.

The price of mixing drivers (ASIO beside WASAPI in one organ) - read this
before you do it:

- **Less polyphony.** Every WASAPI route renders its pipes on its own thread,
  separately from the ASIO routes; the processor's time is split, and the DSP
  load reaches 100 % sooner than with ASIO alone.
- **Different latencies.** ASIO and WASAPI answer at different speeds, so
  divisions on different drivers do not speak at the same moment after the
  key is pressed. While playing this is confusing - though now and then it
  imitates, by accident, the different delays of the actions of a real
  organ's divisions.

The best layout for the highest polyphony is therefore: one ASIO driver, all
routes on it, the output channels spread over its outputs.

### Audio on Linux

**What Linux does with your sound cards.** Every current distribution runs
the sound server PipeWire. It takes every sound card for itself and offers
the programs a stereo "default" output - the desktop's mixer, in which every
card is reduced to a single stereo pair (the profile "Analog Stereo Output",
"Stereo Output 5/6" and the like). An eight-output interface is a stereo
card to the desktop. An organ needs every output of every interface, so the
first thing to do is to take the interface out of the desktop's mixer:

**Step 1 - set the interface to the "Pro Audio" profile.** With this profile
PipeWire hands the card over as it is: every output and every input becomes
a port of its own, nothing is mixed, remapped or volume-controlled on the
way, and the card runs at PipeWire's clock. Where to set it:

- KDE Plasma (Ubuntu Studio, Kubuntu, Fedora KDE): *System Settings >
  Sound*. Every card is listed with its *Profile* box; choose **Pro Audio**
  for the interface the organ plays through.
- GNOME (Ubuntu, Fedora): *Settings > Sound > Output*: pick the interface
  under *Output Device* and set *Configuration* to **Pro Audio**.
- Any desktop: the program `pavucontrol`, tab *Configuration*, the card's
  profile box.

The desktop's own sounds then no longer go to that interface unless you
choose it as the system output; use the on-board chip for the desktop and
the interface for the organ. After switching the profile the card appears
under a new name (its own, "Komplete Audio 6" instead of "Analog Stereo
1/2"), so choose it again in the audio tabs.

**Step 2 - the packages.** OrganSampler needs PipeWire's JACK library and
its command-line tools; most audio-oriented distributions have them, others
do not. Open a terminal (*Konsole* or *Terminal* in the application menu;
Ctrl+Alt+T on most desktops) and type the line for your distribution, then
Enter and your password:

- Ubuntu, Debian, Mint, Pop!_OS: `sudo apt install pipewire-jack pipewire-bin pavucontrol`
- Fedora: `sudo dnf install pipewire-jack-audio-connection-kit pipewire-utils pavucontrol`
- Arch, Manjaro: `sudo pacman -S pipewire-jack pavucontrol`
- openSUSE: `sudo zypper install pipewire-libjack-0_3 pipewire-tools pavucontrol`

To check that PipeWire is the server, type `pactl info | grep "Server Name"`:
the answer should read "PulseAudio (on PipeWire ...)". Log out and in again
after installing.

**Step 3 - realtime permission for your user.** The thread that renders the
audio must run with realtime scheduling, or the desktop, the browser and
every other program can push it aside and the sound breaks up long before
the processor is busy (measured: far less polyphony through ALSA than
through JACK on the same card, for this reason alone). Linux grants
realtime scheduling only to users with a realtime limit, which the audio
distributions (Ubuntu Studio, AV Linux, Fedora Jam) set up for the group
`audio`; a plain desktop installation does not. Check in a terminal:

- `ulimit -r` - the answer must be 95 or more (0 means no realtime).
- `groups` - the list must contain `audio`.
- `cat /etc/security/limits.d/*audio*` - shows the rules themselves
  (`@audio - rtprio 95`, `@audio - memlock unlimited`).
- With the program running and a device open:
  `ps -eLo tid,cls,rtprio,comm | grep -i organ` - one of its threads must
  show `FF` (SCHED_FIFO) and a priority (70 under ALSA); under JACK the
  realtime thread is PipeWire's own and does not appear here.

If not, type these lines (each followed by Enter and, the first time, your
password), then log out and in again:

```
sudo usermod -aG audio $USER
sudo sh -c 'printf "@audio - rtprio 95\n@audio - memlock unlimited\n" > /etc/security/limits.d/audio.conf'
```

OrganSampler raises its audio thread to realtime by itself at the first
callback of a device and writes the outcome into the activity log ("Audio
thread of ... raised to realtime", "realtime already" under JACK, or
"realtime refused" with the reason). Through JACK the thread is PipeWire's
own, which the system has already made realtime; through ALSA it is the
program's, and needs the permission above.

**The two kinds of driver in OrganSampler:**

- **JACK** - use this one. It is PipeWire itself: OrganSampler loads
  PipeWire's own JACK library at start (the activity log says "JACK:
  PipeWire's libjack loaded from ..."), no separate JACK server is needed
  or wanted. Every card in the Pro Audio profile appears under its own name
  with all its channels (a Komplete Audio 6: playback_AUX0 ... AUX5 =
  outputs 1-6, the S/PDIF pair included); tick the channels of every route
  as on Windows - where you send a route, there it plays. No patchbay
  (qpwgraph, Patchance, Carla) is needed: OrganSampler connects its outputs
  to the chosen channels itself. The list shows only clients with audio
  ports (PipeWire also exposes MIDI bridges to JACK, which are no audio
  devices). The buffer size and the sample rate are PipeWire's, the same
  for every JACK route and every other program: choose them in the usual
  *Audio buffer size* and *Sample rate* boxes of any JACK route; the choice
  is handed to PipeWire, the program waits until the server runs the new
  clock, then opens the route, and the other JACK routes follow by
  themselves. (From a terminal the same is `pw-metadata -n settings 0
  clock.force-quantum 512` and `pw-metadata -n settings 0 clock.force-rate
  48000`.) *Reopen device* keeps the route's buffer size and rate (a JACK
  device has no "driver's own" values to fall back to). The status line
  under the boxes says "output latency not reported by the driver" for
  JACK: PipeWire tells its clients no port latency, so the real latency is
  at least one buffer. If the JACK type lists no device at all, the JACK
  library was not found: install `pipewire-jack` as above, or start the
  program with `pw-jack ./OrganSampler`.
- Moving a route from JACK to ALSA on the same card: PipeWire keeps the
  card for a few seconds after the last JACK client left it, so the ALSA
  entry stays listed for that while and the program waits up to seven
  seconds for the card to be released when you choose it. A card the
  desktop uses as its system output is never released: pick another card
  as the system output first.
- **ALSA** - the plain Linux driver, for a quick start or a card that
  PipeWire does not use. "PipeWire Sound Server" and "Default ALSA Output"
  are the desktop's stereo output: two channels, the system mix. A card's
  own entries ("Direct hardware device without any conversions", its S/PDIF
  or HDMI entry) open only while nothing else holds the card; ALSA's
  template entries (surround 4.0 ... 7.1, front, rear, dmix) are never
  shown. When the list is made every device is tried, and one that cannot be
  opened at that moment (held by PipeWire or another program) is left out,
  with the reason in the activity log; a card PipeWire holds for one of this
  program's own JACK routes stays listed. *List unusable devices too* in
  the audio tab (where Windows has the driver control panel button) lists
  everything regardless; the same switch is in the settings editor as *Audio
  devices: list also the ALSA devices that cannot be opened*.

A menu entry with the icon, so that the program starts from the
application menu like any other: *Install menu entry and icon* in the
Configuration Editor (see chapter 1).

### Audio on macOS

To be continued. macOS has one driver, CoreAudio, and joins several cards
into an aggregate device itself (*Audio MIDI Setup* in the Utilities
folder); OrganSampler lists the CoreAudio devices and should play through
them like through Windows Audio. The project has no Mac to test this on;
reports are welcome. Two builds: `OrganSampler-macos.zip` for Apple silicon
Macs on **macOS 11 Big Sur or newer**, and `OrganSampler-macos-10.11-intel.zip`
for Intel Macs on **macOS 10.11 El Capitan or newer** (10.11 is the oldest
system the framework allows; a Mac on 10.10 needs the free upgrade to the
newest system Apple offers for it, 10.13 High Sierra for the 2010 models).

## 5.7 Audio devices not ready when OrganSampler starts

Devices that cannot be opened at start-up (unplugged, driver missing, or merely
late - an RME interface registers a few seconds after it switches its sample
rate) are waited for: a box "Waiting for audio devices" appears and the program
checks every second for six seconds. Devices still absent afterwards are listed
once in one message; their routes stay silent until the device appears, then
reopen by themselves. A device you choose yourself is never swapped back.

## 5.8 Getting the most polyphony

Every sounding pipe is one **voice** per audio route of its division. A
division with three perspectives (Direct, Diffuse, Rear) renders every pipe
three times, one voice per route. The auxiliary route adds one more voice per
pipe while it has a device. The *DSP* field of the main window shows how much
of the available time the busiest audio thread needs; at 100 % the sound
breaks up.

So the polyphony a computer reaches is not a property of the computer alone:
it depends on the whole configuration of the organ - the number of audio
routes (every perspective of every division, every auxiliary route with a
device), the IR reverbs on them, the drivers and their buffer sizes, and the
samples themselves (their number of perspectives, the length of their releases).
The same computer that renders 25 000 voices of a one-perspective dry set
may manage a third of that with three perspectives, an auxiliary route and
IRs. This is why *Maximum polyphony* is saved with the configuration, and
why "Measure the maximum polyphony" works only with the organ loaded and its
audio outputs configured: a general value would not fit any organ. What
decides how many voices fit:

- **The auxiliary route.** With a device it renders every pipe once more and
  halves the polyphony. Set it to *none* unless you need it (see "The
  auxiliary route").
- **IR reverb.** The convolution of an impulse response is heavy and its
  cost does not depend on how many pipes sound: it is paid on every route
  that has an IR, at every audio period. The convolutions run on the render
  workers, in parallel, one buffer behind the dry signal (that buffer is
  part of the pre-delay), so they take render time away from the pipes but
  do not pile up on the audio thread. A true stereo IR (four channels)
  costs twice as much as a stereo one, a long IR more than a short one
  (*Decay* shortens it). A route that has been silent for longer than its
  IR switches the convolution off until the next sound, so idle
  perspectives cost nothing. Dry/wet at 0 %, or *Clear*, removes the cost
  entirely.
- **ASIO.** All routes on one ASIO driver are rendered by one audio thread
  and the render workers behind it (*Render threads* in the Configuration
  Editor): the voices of every route are shared out over all the processor's
  cores. This is the layout with the highest polyphony. The buffer size sets
  how much time each block of audio may take: a buffer of 512 samples allows
  twice the work of 256, at twice the latency.
- **The buffer size: do not chase the smallest one.** A small buffer
  (64 or 128 samples) cuts the reachable polyphony dramatically, because
  every block must be rendered in a fraction of the time. An organ does not
  need it: the action of a real organ has its own delay, and a latency of
  about 10 ms (512 samples at 48 kHz) is absolutely fine for organ playing -
  even 20 ms (1024 samples) is still perfectly comfortable. Choose the
  larger buffer and take the polyphony.
- **Windows Audio.** All routes on one Windows Audio device share one stream
  and one callback, rendered by the whole pool of *Render threads* - the
  same layout as ASIO, with the 10 ms period of the Windows audio engine
  (a buffer of 480 samples at 48 kHz) instead of the driver's buffer. Only
  an Aggregated WASAPI route is rendered by its own thread (*Render threads
  for Aggregated WASAPI*).
- **Mixed ASIO and Windows Audio.** Works; the ASIO workers leave processor
  time to the WASAPI threads. But see "Audio drivers: many sound cards at
  once" for the price.
- **Several devices against one.** Every open device - an ASIO driver, a
  Windows Audio device - has a driver thread of its own, and the part of a
  callback that is not shared out over the workers (the hand-over of each
  route, the mix into the driver's buffer, the driver's own copy) runs on
  it. With all routes on one device those parts run one after another on
  one thread; spread over several devices they run beside each other. So an
  organ spread over many Windows Audio devices, or over two ASIO interfaces,
  can measure *more* than the same organ on one ASIO driver - that is
  expected (at the price of the different latencies above). Which channels
  the routes use on one device makes no difference: two routes on the same
  pair cost the same as two routes on different pairs. The measurement
  itself has a spread of some per cent between runs (the regulator ends at
  the first late callback, and those are not quite repeatable), so a
  difference of that size between two layouts says nothing.

The *DSP* value is the whole callback of the busiest audio device - the
render of all its routes, the note bookkeeping of attacks and releases, the
copies to the device - as a share of the device's period, held at its peak
for a moment. That is what decides about dropouts: at 100 % the next
callback is late. A value of several hundred percent means the audio thread
was kept waiting by something else on the computer, not that the voices took
that long; the activity log notes every such event.

### What moves the measured figure, and what the measurement cannot see

Lessons from many runs on four computers (two-core laptops, a 16-core
desktop, a 16-core notebook), 2026-10:

- **The buffer size** is the first thing: on the same computer and organ a
  buffer of 256 gave 1000 voices, 512 gave 2300-2900 and 1024 gave 3100-3800
  (two cores); 49 000 / 64 000 / 74 000 on sixteen cores. Compare runs only
  at the same buffer.
- **The device path.** Linux: PipeWire's "Default ALSA Output" is one layer
  further from the card than the card's own ALSA device ("Direct hardware
  device") or JACK; on two cores half the runs on the default output ended
  on a single callback of 107-130 % of its period (the system took the core
  for a moment), none did on the direct device or on JACK. Measure on the
  direct device or JACK. Windows: ASIO or one Windows Audio device for all
  routes; macOS: Built-in Output or the card's own device.
- **Compress samples in RAM** halves the figure, on a big organ more (Caen on
  the 16-core notebook: 60 000 -> 25 000). Measure with it off unless you
  will play with it on.
- **Two-core laptops** scatter by a third between runs: the render worker
  on the second core is taken by the system now and then (the report shows
  it as "the latest a lane joined a job" of several milliseconds). Take the
  median of three runs, not the best.
- **Aggregated WASAPI** is a different animal: every route is a shared-mode
  stream of its own, mixed by Windows. Each route renders on its own thread
  (one lane, capped at cores / routes), so a big organ spread over fifteen
  routes measures like ASIO (Doesburg, 16-core: 42 600 against 45 400 on
  ASIO), while the whole organ on two routes is bound to two cores. The
  mixer of Windows has its say after our buffer: an endpoint with "audio
  enhancements" on (loudness equalisation) boosts every quiet stream on its
  own, and fifteen streams of one route each came out some 20 dB louder than
  the same organ on ASIO (the program's own levels in the activity log were
  the same) - switch the enhancements off in the endpoint's properties, or
  use ASIO or one Windows Audio device for the card.
- **What the measurement cannot see.** It judges every callback of the
  program's own audio threads (its length against the period, late calls).
  A dropout made after our buffer leaves the program - a shared-mode mixer
  of the system with many streams on one endpoint, an exclusive-mode device
  of ours opened on the same endpoint and getting no callbacks, a USB card
  starved by the system - is inaudible to it: the run is "clean" and the
  sound is not. Such a case shows in the activity log as "Audio callback
  anomaly: ... no callbacks" for a route, or not at all; the cure is the
  device layout (one driver for everything, ASIO where there is one, no
  unused routes left open on the same endpoint), not the limit.

---

# 6. Playing aids

## 6.1 Couplers

*Couplers...* (top bar) shows one column per division (the keyboard being
played) with every coupler it can have: every division pulled at unison, at
16' (sub-octave) and at 4' (super-octave) pitch - the division itself
included, so "I/I 16'" is an octave coupler - plus the melody and bass
couplers and **Unison Off** at the top of the column. Ticked couplers appear
on the division's coupler rail in the main window, where they are shown like
stops (named "II/I", "II/I 16'" and so on: the division pulled over the
keyboard played). A coupler is registration: part of combinations and the
crescendo, MIDI-learnable. The rail's buttons can be rearranged by dragging.

- **Unison Off** silences the division's own pitch: key 60 held on that
  division does not sound 60 there; its couplers still work.
- **Melody coupler**: only the highest key held on the division is coupled;
  **bass coupler**: only the lowest. *Melody/bass couplers: new extreme key
  only* and *Melody couplers: legato grace time* (Configuration Editor >
  Console) refine their behaviour.

## 6.2 Combinations and the crescendo

See "The combination row" and "The crescendo".

## 6.3 Temperaments

*Temperaments* (top bar) opens the temperament dialog: a list of thirty
historical temperaments (Werckmeister, Kirnberger, Valotti, Neidhardt,
meantone, Pythagorean, Silbermann, Sorge ...) and the custom ones you add. The
"+" beside a temperament shows its cent deviations per pitch class; those of a
custom temperament can be edited.

Two temperaments matter:

- **The samples' base temperament** is the temperament the organ was
  *recorded* in. The program reads the recorded pitch of every pipe from the
  smpl chunks, measures how each pitch class deviates, and compares the
  result with the tables. When two thirds or more of the ranks agree on one
  temperament it is chosen by itself ("Base temperament set from the
  samples" in the activity log) - for most sample folders you never touch it. When the
  ranks disagree, or when the set has no smpl pitches, choose it by hand:
  **double-click** the temperament in the dialog (or pick it in
  *Configuration Editor > Console > Samples' base temperament*). The voicing
  row of every stop reports what its own ranks were found to be in
  ("temperament = ..."), which helps the choice. "Equal" is reported whenever
  the recording matches equal temperament.
- **The target temperament** is the temperament the organ *plays* in. A
  **single click** on a temperament selects it (or *Target temperament* in
  the Configuration Editor). Every pipe is then retuned by the difference
  between the target and the base: the recorded pitches are taken out and
  the chosen ones put in. Choosing the recorded temperament itself as the
  target plays the samples exactly as recorded. Every temperament can be
  switched from a MIDI control (right-click it).

**a1 = 440 Hz** (the button in the dialog) retunes every pipe from its recorded
pitch to a1 = 440 Hz. Its purpose is to bring the samples of different organs
onto one pitch: organs are recorded at different temperatures and different
diapasons (a1 = 435, 440, 442, 465 Hz ...), and an organ built from the stops
of two such organs would be out of tune with itself. With every organ at 440 Hz
the stops agree, and celeste beats and the global *Pitch* slider (Global
sound, cents on top of everything) mean what they say. The voicing row's
*Undulation* line reads "needs 440 Hz" until this is done. A large retune
(an organ far from 440 Hz - Adlington at a1 = 409 Hz is 128 cents away -
or a big global Pitch) is not played by speeding every pipe up: the common
part of it is applied once to the mixed output of every route with a proper
resampler (0.8.163), so the pipes keep their recorded speed; only the
cents of temperament and detune stay in the pipes themselves, and the high
pipes play those through the full kernel (Engine > *Full kernel from*,
0.8.164), so no foreign tone appears under a retuned top octave.

## 6.4 MIDI learn

Everything that can be played from a console can be learned: stops,
transmissions, couplers, tremulants, pistons and the stepping buttons, the
crescendo (a continuous controller), the swell of every division (the swell
bar in the header), every global slider, every top-bar button, the keyboards
themselves (right-click the keyboard icon beside the division's name in its
header, or the keyboard drawing in the Divisions dialog). Right-click the control and choose the
*Learn MIDI ...* entry, then move the control on the console. The bindings are
part of the configuration.

## 6.5 Recording

*Record* in the top bar records the stereo mix to a WAV file; click again to
stop (the recording ends with a short fade).

---

# 7. Filled pipes, extensions and the transposition methods

## 7.1 What a filled pipe is

Every rank is completed to the full keyboard: pipes it does not have are
played from neighbouring real pipes, transposed. This covers the extra octave
of an extension (a 16' or 4' transmission of an 8' stop), sample folders whose
top or bottom keys are missing, and gaps inside the compass. Only the pipes
that a stop can be asked to play are filled: the compass of its own division and, for
every transmission, the compass of the receiving division shifted by the
transposition. A filled pipe keeps everything a real pipe has: its own voicing
slider, random detune, wind and tremulant behaviour, release samples (from the
same neighbour, treated the same way); it costs nothing while silent.

Which real pipe serves a missing one is decided by *Missing pipes: neighbours
borrowed as a group*: with the default 12, every missing pipe is the
octave transposition of a real one, so the chromatic variety of the rank is
kept. A rank's fill map is sticky: when the compass grows (a new transmission)
the pipes already filled keep their source, so their copies are never built
again.

## 7.2 The transposition methods

How the neighbour's sample gets its new pitch is *Filled pipes: transposition
method* (Configuration Editor > Advanced global audio). The default is *Pitch
shift + time stretch*, which gives the best results for most stops; the phase
vocoder is an optional alternative for those who want it. Every method has
its weakness; which one sounds best depends on the stop, the size of the shift
and the ear. Listening tests with reeds (Trompete, Vox humana) show the
differences most clearly; a 16' extension of an 8' reed is the typical case.

**Resampling (pitch and length change together).** The sample is played faster
or slower at every key press, like a tape. Every harmonic exact, nothing added,
nothing to build: it plays at once. Weaknesses: an octave down lasts twice as
long - the attack speaks slowly, the room decays for twice the time, two
octaves down four times; the timbre moves fully with the pitch, so the
transposed pipe sounds like a bigger or smaller pipe than it stands for. To
soften that, a pipe resampled upwards automatically loses 0.1 dB of level and
0.1 dB of treble per semitone (an octave up: -1.2 dB each), because a pipe
played faster comes out brighter and louder than a real pipe of that pitch.
This compensation belongs to plain resampling only: the three other methods
settle the loudness and the timbre of their copies when they build them, and
a pipe that still plays resampled while its copy is being built gets it until
the copy is ready.

The three other methods build a copy of every filled pipe in the background
(when the stop loads, when its compass or a transmission changes, when a
setting changes). Until a pipe's copy is ready the pipe plays by plain
resampling, so the organ is playable from the first moment; the stop buttons
show the build going on (a dark track along the bottom, filling orange), and
the pipes switch to their copies one by one as they are finished.

**Pitch shift (resampling + timbre correction).** A copy of every filled
pipe's attacks and releases is built in the background when the stop loads or
its compass changes: the sample resampled with the best filter, its spectral
envelope (the formants, the body of the sound) moved back by the stop's
*Formants* value, the loops set to whole periods, the loudness matched to the
source, and the release's decay laid out again to the source's duration
(*Filled pipes: release length*). Nothing in the copy varies with time, so
nothing modulates or hisses. Weakness: the attack still speaks at the
resampled speed - slow downwards, fast upwards.

**Pitch shift + time stretch (attack and release).** The same copy, and the
attack (downwards) and the release are time-stretched back towards the
source's duration (*time stretch of the attack*, *time stretch of the
release*). Upwards the attack is left as resampled (shorter), which the ear
accepts. Weaknesses: downwards the time stretch frays the attack - the
speaking of the pipe becomes rough or choppy, worse the larger the shift; try
a lower attack stretch for such a stop, or the pitch shift method without
stretch.

**Phase vocoder (Signalsmith Stretch).** Keeps every length and every loop
point: the pipe speaks at its real speed and the room decays in its real time.
The analysis block is sized from the pitch, so deep pipes keep their harmonics.
Excellent within an octave either way. Weaknesses: upwards the vocoder
slightly wobbles the pitch (a gentle unsteadiness of the tone); two octaves
away the copy loses level and body; the attack's transient is smeared over the
analysis block (half a second for a 32' pipe) and inharmonic content can
tremulate. And it often breaks the loops: the loop points are kept, but the
vocoder's output at the loop end no longer matches the loop start exactly, so
the seam becomes audible as a click or a bump at every repeat. The setting
*Filled pipes: loop crossfade* exists for this - a long blend over the seam
hides it at least partly. The slowest build of the three, though still
seconds per stop.

The other methods can also worsen the seamlessness of a loop, unfortunately:
every resampled copy has its loop points moved to whole periods of the new
pitch, and a seam that was perfect in the recording is then only nearly
perfect. The loop crossfade helps there too; listen to a held note of the
lowest extension pipes and raise it when the loop is heard.

Practical advice: stay with the default, *Pitch shift + time stretch*, unless a
stop sounds wrong with it; then try the plain pitch shift (no frayed attack)
or, for shifts within an octave, the phase vocoder (the speaking at its real
speed, at the price of the loops). Compare on the lowest keys of a reed
extension, where everything is heard.

## 7.3 The settings that go with the methods

All in *Configuration Editor > Advanced global audio* unless said otherwise;
the sliders that do not apply to the chosen method are greyed out. Every one
of them has its own chapter in "Advanced global audio".

- **Formants (filled pipes)** - in every stop's and transmission's voicing
  row (see "The voicing row"). How far the body of the sound follows the shift: 0 % keeps the
  envelope where the recording had it (the brightest result), 100 % moves it
  fully with the pitch (plain transposition, veiled two octaves down). The
  value is for shifts downwards up to an octave; shifts upwards follow 30
  points more; beyond an octave every further octave in either direction adds
  15 points with the resampling methods and 25 with the phase vocoder, because
  a copy two octaves away is a pipe of another size. A pipe is one copy: inside
  the stop's own compass the stop's value applies; a pipe beyond it, asked for
  by transmissions only, takes the value of the first transmission in the
  stop's list whose shifted compass holds it. Changing it rebuilds only the
  pipes concerned. Default 30 %; the right value is a matter of listening.
- **Filled pipes: release length** (pitch shift method) - the decay of a
  release copy laid out to this share of the source's duration (the room does
  not grow or shrink with the pipe).
- **Filled pipes: time stretch of the attack** (time stretch method) and
  **time stretch of the release** (time stretch method both ways; the phase
  vocoder upwards only, where the kept length is too long for a smaller pipe).
- **Filled pipes: release played from** (default 140 ms) and **Filled pipes:
  release level matched to the source** (off by default) act at playback and
  rebuild nothing: the first skips the start of every release copy, where the
  methods leave their worst; the second plays the copy at the level relation
  the source pipe's release has to its sustain.
- *Filled pipes: loop crossfade*, *Filled pipes: loudness kept when pitch
  shifting*, *Filled pipes: resampling quality* - see their chapters.
- The global **Sustain fade-out on key release** and **Release sample
  fade-in** (Global sound) are set for the sounding c1 and grow 1.5 times per
  octave below it, so a 32' extension gets the slow fades a big pipe needs; a
  filled pipe counts as the pipe it stands for.

## 7.4 The cache

The copies of the filled pipes take a while to build for a large organ: a
minute or more for a big set with many extensions, every time the organ loads.
The cache keeps them on disk so that this happens once.

**The switch.** *Configuration Editor > Advanced global audio > Filled
pipes: keep the generated copies on disk (cache)*, on by default since 0.8.124
(it was off, and a reload of a big organ computed every filled pipe again even
on a 16-core; the activity log's "Copy build" lines say when it is OFF). On,
every copy that is built is also written to a file, and at the next load
of the same stops with the same settings the copies are read from the files
instead of being computed: the build after loading takes seconds instead of
minutes. A long build that is interrupted (by an operation in the voicing)
still writes the pipes it has finished.

**What is stored.** One file per rank and folder tree (per perspective of a
stop), whatever the settings. The settings a file was built with are written
inside it; a file built with other settings is simply replaced by the new
build, so the cache never holds two versions of the same copies and never
plays a stale one. The cache therefore takes about as much room on disk as
the copies take in RAM: tens of MB for a small set, hundreds of MB to a few GB
for a large organ with many extensions. Files of an older cache format are
removed by themselves.

**The three buttons** (Configuration Editor, under the cache switch):

- The row *Filled pipes cache: <size> in <folder>* shows how much the cache
  holds and where. Its button **Empty the cache** deletes every file. Use it
  to get the disk space back, or after replacing the samples wholesale; the
  copies are built again (and cached again) the next time they are needed.
  Nothing else is lost.
- *Filled pipes cache: files of stops not loaded now* - **Remove them**
  deletes only the files of ranks that are not loaded at the moment: the
  stops of other configurations, of folders you have unloaded or deleted.
  The files of the loaded organ stay, so its next load is still fast. This is
  the button for routine housekeeping.
- *Filled pipes cache folder* - **Choose...** picks another folder for the
  cache (the default is `FilledPipesCache` in the application-data folder,
  usually on the system drive). Choose a big drive, for example beside the
  samples; the files built so far are moved there, so nothing has to be
  rebuilt.

The cache is a convenience, not a necessity: with it off everything works the
same, only the build after each load takes its full time.

---

# 8. Configuration Editor reference

*Configuration Editor* (top bar) holds every setting of the program in one
scrolling dialog, in sections: Engine, Global sound, Advanced global audio
(collapsed by default: click its title), Console, Folder name patterns, and
one section per division (collapsed by default). The folding is remembered.
The "?" beside every option opens this manual at the option's chapter below.
At the top: *About & licences...* with this manual (a chapter box jumps to any
chapter), the program's licence and, in full, the licences of the software it
contains. Every change is applied and saved at once; double-click a slider to
return to its default.

Beside it *Show settings.json* opens the settings file in the system's
editor and *Refresh values* reads every control again from the program (after
a change made elsewhere, a MIDI-learned control for example).

## 8.1 Engine

### Audio devices: list also the ALSA devices that cannot be opened (Linux)

Linux only. Off: the audio device lists hold only the ALSA devices the program
could open when it looked. On: every device ALSA names is listed, a device
in use by another program or without a working configuration included - for
a device that shows up only once another program lets it go.

### This computer

The processor, its physical cores and logical CPUs, and the RAM. Information
only; it tells what the thread settings below can use.

### Render threads

How many CPU threads render pipes for ASIO routes, the audio driver's own
thread included. (The note bookkeeping - MIDI commands, attacks and releases,
the return of finished voices - runs on a thread of its own beside them, so
the audio callback only renders; before 0.8.45 it took up to a third of every
period inside the callback.) Default: all physical cores - keep it there. Hyper-threads
(the logical CPUs) share a core's execution units and memory pipeline: measured
on a 16-core processor, 31 threads rendered a tutti slower than 16 (0.95 ms
against 0.78 ms per block) and with peaks four times as high (up to 4.5 ms
against 0.9 ms), and such peaks are what breaks the sound up. So the
"logical" choices lower the reachable polyphony; never use them. Fewer threads
leave room for other programs. Changing it briefly mutes the engine. The list
holds only the choices this computer can tell apart: without hyper-threading
"All logical CPUs" is the same as "All physical cores" and is not shown, a
single core has no "all - 1". A hybrid processor (Intel 12th generation and
later: performance and efficiency cores; Windows tells them apart) offers
*Performance cores only* (0.8.140): a render worker on an efficiency core
renders several times slower and the device thread waits for it - the
measurement's longest callback of an Ultra 7 notebook was 2.6 ms of which
2.4 ms waiting for the lanes; the report says so for such a processor. The
setting belongs to the computer, never to an
organ: a portable configuration carries none, and one of an older version that
does is ignored (this computer's own setting stays). The default is every
physical core on every computer: a setting that is not the default and was
never chosen in this editor on this computer (older versions let a portable
file bring it in - the Rural Organ file left two-core laptops rendering on one
thread) goes back to the default at the next start; a choice made here stays.
All physical cores means all of them on a two-core laptop as well. The render
workers and the control thread run at audio priority on every system (Linux:
SCHED_FIFO when the system allows it - the activity log says "Render worker:
SCHED_FIFO" or "REFUSED" with what to allow; macOS: a time-constraint thread
like CoreAudio's own), and on one or two cores the device thread yields the
core while it waits for a lane and the lanes take small chunks, so the second
thread helps instead of holding the block up.

### Interpolation kernel

The code that renders every pipe (0.8.124). *Automatic* (the default) takes
the kernel this processor is known to run well: AVX2 (eight frames a step,
current x86-64 processors), SSE2 (four frames, x86-64 without AVX2 such as a
Core 2 Duo - since 0.8.126: measured with this switch on the same routes, SSE2
gave 2500 voices against 1700 scalar on such a Mac; what had made those
laptops slower in 0.8.119-0.8.122 was the thread priority, fixed in 0.8.123,
not the kernel) or NEON (four frames, Apple silicon and Linux arm64). The
list holds every kernel this computer
can run: *scalar* (one frame a step, every CPU), *SSE2* (four frames, every
x86-64), *AVX2* (eight frames, when the CPU has it), *NEON* (arm64). Pick one
to compare it with another: the change takes effect at once, no reload, and
the polyphony measurement run with each tells which gives more pipes on this
computer at this buffer - the report names the kernel it ran with. The
setting belongs to the computer (a portable configuration carries none); a
name this computer cannot run means Automatic. The environment variable
`ORGAN_KERNEL=scalar`, `sse2`, `avx2` or `neon` sets the Automatic choice for
one start (a script measuring one after another); the editor's choice wins
over it. The self-test (`--kerneltest`) checks every kernel of the CPU against
the scalar code and tells its speed; the activity log says which kernel runs
("Render kernel: ...").

### Full kernel from (sounding pitch)

Every pipe whose sounding fundamental lies at or above this frequency plays
through the 8-tap windowed-sinc kernel instead of the two-point linear one,
for its whole length, loop splices included. The linear kernel leaves an
image of a retuned high tone *below* its fundamental, 48000 x (ratio - 1)
Hz under it at about -26 dB: a pipe at 9 kHz retuned by 20 cents (a
temperament, Equal at 440 Hz) gets a foreign tone 560 Hz under it. Low
pipes hide it (their image is 50 dB down and sits beside the partials), the
top octaves do not. The sinc kernel puts it below -44 dB. The default
3000 Hz (2/3' at c1) covers the top octaves of the 2' and 1' stops and the
mixtures at a few per cent of extra work; 0 gives the sinc to every pipe
(three to five times the work per voice, the kerneltest prints the figure
of this CPU - for a computer with CPU to spare),
20000 switches it off. The slider is logarithmic, so the low end is easy to
reach. Takes effect for the next notes, no reload; this computer's setting.

### Currently active

The threads really rendering now and the interpolation kernel in use.
Information only.

### Render threads for Aggregated WASAPI

Windows only. The threads used by routes on an Aggregated WASAPI device (the virtual
device that joins several Windows Audio endpoints). Every aggregated route
is a stream with a device thread of its own, so an organ with ten such
routes already renders on ten threads beside each other; the default of 1
(the device thread alone, no workers) is right for that layout, and more
threads per route only fight each other there. Raise it when one or two
aggregated routes carry the whole organ (a big division on a single route is
otherwise bound to one core): the route then shares its pipes out over the
set number of threads, like a route on ASIO does over all *Render threads*.
Whatever is set, a route gets at most cores / aggregated routes lanes
(0.8.148): fifteen routes with sixteen lanes each kept sixteen workers
spinning for fifteen posters and starved the endpoint threads - the sound
broke at one pipe, clean with one lane. The activity log says when the cap
applies ("Aggregated WASAPI: N route(s) on M cores - L lane(s) per route").
Routes on ASIO drivers and on Windows Audio devices do not use this setting:
every such device is one stream with one callback, and all its routes are
rendered by the whole pool of *Render threads* (since 0.8.47 for Windows
Audio too - before, every Windows Audio route had a stream and a thread of
its own, so a big division on one route was bound to a single core).

### Sample storage in RAM

*32-bit float* keeps the samples exactly as read (full fidelity); *16-bit
integer* halves the RAM and allows more polyphony at the cost of a little
hiss. *16-bit integer, releases at half the rate* (0.8.143) stores the
release samples - a large part of a set and a decaying tail - at half the
storage rate: they lose what lies above 12 kHz (at the 48 kHz base) and take
a quarter of the RAM they would; nothing is requantised, the room noise of
the tail stays as recorded. The first 1.3 seconds of a halved release play
through an eight-tap windowed-sinc kernel (0.8.147): the plain linear kernel
let the images of what the file holds through at only 7-11 dB below it - a
digital ringing at the start of a reed's release - and the sinc puts them
40-50 dB down; after 1.3 s, where the treble has died, the linear kernel
takes over. Only a release whose treble above 11 kHz is louder than -18 dB
of the whole (a reed in the treble) keeps the full rate; a halved release
with nothing above 6 kHz louder than -40 dB has nothing to mirror and plays
through the cheap linear kernel (0.8.149: the sinc costs ten times the
linear kernel - every release through it took half the polyphony of a
two-core laptop). The log says how many went each way. Takes effect the
next time a division's folders are loaded.

### Compress samples in RAM (lossless)

Off by default (0.8.145). On, the 16-bit samples are packed in RAM the way
FLAC packs a file: a predictor per block of 256 frames and the residuals
with the bits they need. Every frame comes back exactly - loops, pitch and
voicing are untouched - and a set takes about a third less RAM; the log
says how much ("Packed samples: ..."). **The price:** every voice unpacks
the piece it plays before every block, about as much work again as the
interpolation itself, so half the polyphony is gone, up to two thirds on a
big organ (Caen on a 16-core notebook: 60 000 to 25 000 voices) - measure it,
the report shows the unpacking as its own figure. Use it only when an organ
does not fit the memory otherwise. 32-bit float storage is packed as 24-bit
integers: bit for bit when the files hold such integers (24-bit or 16-bit WAV),
rounded to 24 bits for real floats (32-bit float WAV, a sample resampled on
loading, a filled pipe's copy) - half a 24-bit step at most, below what any
converter resolves; the log says how many were rounded. **Loading takes
markedly longer with packing on** (every sample is packed and verified frame
by frame while it loads: about 2.5 times the decoding), and longer again with
*releases at half the rate* (every release is resampled through the sinc
filter): a big organ that loads in a minute at 16-bit, 48 kHz takes several
with both on. Takes effect the next time a division's folders are loaded.

### Sample rate on loading

*Resample everything to 48 kHz* (default): files recorded at 96, 88.2 or 44.1
kHz are converted once while loading with a high-quality filter, so every set
plays exactly like a 48 kHz one and a 96 kHz set needs half the RAM it would
otherwise; loading takes longer. *36 kHz* and *32 kHz* (0.8.143) store every
file at that rate: what lies above 17 or 15 kHz goes - organ pipes carry next
to nothing there - and the RAM drops by a quarter or a third; the kernel
interpolates from the coarser grid at the same cost, so more stops fit and
the polyphony stays. *Keep each file's own rate* loads faster and plays by
interpolation. A filter alone would save nothing: a 16-bit sample is 16 bits
whatever it holds; only the coarser grid saves. Takes effect at the next load.

A word on who hears the difference. Hearing loses its top end with the
years: past fifty, few people hear anything above 17 kHz, and past sixty the
limit is nearer 15 kHz. An organist over fifty can take *36 kHz* and lose
nothing audible; over sixty, *32 kHz* costs nothing either - what the coarser
grid leaves out is already beyond reach - and saves a third of the RAM.

### IR processing

How the IR reverbs of the routes are convolved. *Full* convolves every route
with its impulse response as it is. That is heavy on memory traffic rather
than on arithmetic: a true stereo IR of 7 s on six routes (three divisions,
each with its default and its auxiliary route, as in Rural Organ) streams
more than 3 GB/s through the partitioned convolutions, and a weak computer -
an old laptop, a Core 2 Duo - cannot render the pipes beside it; it crackles
at a few dozen voices whatever the buffer size. *Light* runs the convolution
at a quarter of the sample rate (half when the high cut of the wet signal
lies above 2.5 kHz, so that the wet band stays intact), trims the IR where
its energy has fallen 60 dB below the whole (a church IR of 7 s is there
after 3.5 s; what follows is inaudible under the organ) and reduces a true
stereo IR to its direct channels - a small fraction of the work, the same
room, a little less air in the top of the reverb. *Automatic* (default)
measures every reverb when it is built - two tail periods of noise through
its engines, timed - and chooses Light when the routes' reverbs would take
more than 18 % of a render thread per second of audio (35 % for a single
route), or when a convolution falls behind its period twelve times in a row
while playing; it remembers that choice for this computer and returns to
Full when Full would clearly fit again (below 12 %). The line below the
setting says what the reverbs cost now; the activity log records every
measurement and choice. Full and Light set the mode by hand. The setting
belongs to the computer, not to the organ: a portable configuration does not
carry it.

Routes that share everything the reverb depends on share one convolution
(since 0.8.109): rendered by the same audio device to the same output
channels, with the same impulse response, dry/wet, wet volume, high and low
cut, pre-delay, decay and fade-out. Convolution is linear - the reverb of the
summed dry signal of such routes is the sum of their reverbs - so the sound is
the same and the work is paid once per group: Rural Organ's three divisions on
one device, each with its default and its auxiliary route, run two
convolutions instead of six. The IR caption of a route that rides on
another's convolution says *(shared)* and its tooltip names the route; a
route whose settings differ in anything keeps its own convolution. Nothing
needs to be set: the grouping follows the settings and the devices.

### Maximum polyphony

The number of voices that may sound at once (64 to 500000, default 30000).
The ceiling is far above what a computer of 2026 renders - a 16-core machine
manages 25 000 to 30 000 voices of a wet set with three perspectives at a
buffer of 256 - and leaves room for the computers of the years to come; the
program is as stable at any value, the ceiling only sizes the voice pool.
When it is reached the oldest voices are faded out. Set it below what the
computer can render so that an overload fades old voices rather than crackles:
set the maximum polyphony so that the *DSP* value never exceeds 100 %, to
avoid cracks and dropouts. The polyphony display of the main window shows
*NOW* (the voices sounding), *PEAK* (the most that sounded since the last
reset), *DSP* (the load) and *LIMIT* (this setting).

The value belongs to the organ, not to the computer alone: the number of
audio routes, the auxiliary routes, the IR reverbs, the drivers and the buffer
sizes all change it (see "Getting the most polyphony"). It is saved with the
configuration. Under it, *Measurement limit* (60-100 %, default 85 %) is the
share of its period the longest driver callback may take before the
measurement counts a level as broken - see "Measuring the maximum polyphony".
The easiest way to find the polyphony is the button below,
"Measure the maximum polyphony", once the organ is loaded and its audio
outputs are configured. On a computer where the full registration breaks
the sound up at the very first strike (no second stays clean), the
measurement retreats by itself: the octave couplers go off, half of every
division's stops are switched off, and the climb starts again from one key -
up to five times, down to a thirty-second of the stops - until a second
stays clean; the report then says with how many stops it measured, and the
limit found is what the computer renders cleanly whatever the stops. By hand:

1. Engage a large number of stops with long releases (a full registration of
   wet samples).
2. Bash the keys: play many quick staccato notes over many keys at once
   (palm smearing - repeated fast strokes with the flat hands over the
   keyboards), so that as many releases as possible sound at the same time:
   that produces the highest polyphony. Remember:
   the longer the release, the more polyphony it costs, because every voice
   keeps its place in the polyphony until its release has died away
   completely.
3. Watch *DSP* and *PEAK* while you do it. At the moment the audio starts to
   break up (crackle, dropouts), note the *PEAK* value.
4. Enter a somewhat lower number as the limit.
5. Repeat until no breaking up of the audio (dropouts, crackle) is ever heard.

Everything in "Getting the most polyphony" (the auxiliary routes, the IR
reverbs, the drivers) raises or lowers the number you will arrive at. Above
all, do not be afraid of a larger audio buffer: a latency of about 10 ms
(512 samples at 48 kHz) is absolutely fine for organ playing, and even 20 ms
is still perfectly comfortable, while chasing the smallest buffer cuts the
reachable polyphony dramatically.

### Measure the maximum polyphony

*Measure...* does the procedure above for you, with the loaded organ and its
configured audio outputs - every route, auxiliary route, IR reverb and
driver as it is. The button is greyed out until samples are loaded: a
measurement without the organ would not say anything about it. What
happens:

1. The sound is turned down to -52 dB at once (everything is rendered as
   usual; you hear the storm very quietly). The polyphony limit is out of the
   way (500 000) for the run, and so are *Attack minimum time* and *Attack
   minimum delay*, which would hold the fast strikes back. The idle audio
   devices are listened to for two seconds first. The progress window says
   what is under test at its top - the number of stops, the routes by driver
   type and device ("Windows Audio: 2 on "A", 1 on "B""), the number of
   routes with an IR - and grows with that text; below it, the current
   target, the stimulus and the voices now.
2. Every stop is engaged and keys are struck again and again on every
   division - each over its own compass, the pedal from its lowest key -
   slowly at first: one key, twice a second. The voices are watched
   every second - the most voices sounding at once in that second, counted
   by the engine at every change (a stroke's attacks over the releases of
   the last one), the same figure the limit is compared with. Whenever they stop rising below the current target, the
   stimulus grows at once - more keys spread over the compass (up to every
   key of every division), then a faster rate (up to 16 strikes a second).
   No coupler is used: every division is struck itself. A wet set with several perspectives and
   long releases reaches tens of thousands of voices with a handful of keys;
   a dry set gives little more than what is being pressed, so it ends up with
   every key hammered and the couplers on. The IR reverbs of the routes run
   as they are: on a dry set they are the main cost, and the report says so.
3. The target grows in steps of 2 000 voices (a tenth above 20 000). Every
   target reached is held for four seconds and judged: clean means every
   callback of the busiest device - the render of all its routes, the note
   bookkeeping, everything on that thread - stayed within 85 % of its period,
   no single route exceeded its budget, and the driver never called late (a
   callback more than half a period behind its time is a dropout). The first
   breakup ends the climb; the run then narrows down between the last clean
   and the failing number of keys - twice, up to five times while the
   failing level still lies more than a tenth above the clean one - letting
   the tails die away in between. When the keys cannot be split any further
   and the gap is still wide (a weak computer: clean at 474 voices, broken at
   742), the run narrows down by stops as well: at the failing number of keys
   a share of the registration is engaged - every button of the grid counts,
   transmissions and extensions included - halved towards the clean side, up
   to four times, so that the result is what the computer really renders, not
   the last coarse step below it. When the clean and the failing level differ
   in the rate (every key was down already), the rate between them is split
   first. A level whose callbacks stay between 85 and 100 % of the period for
   two seconds is neither clean nor a breakup: it counts as a failure, and the
   run narrows down below it. Below 1 000 voices the result is taken as
   measured, not rounded. When even all 61 keys struck 16 times a second produce no more
   voices and the audio stays clean, the organ itself is the limit, not the
   computer. The *DSP* field may read lower than 85 % at the moment the run
   ends: it shows a peak that fades within a fraction of a second, while the
   judge takes the single longest callback and every late one. Above the
   measured value the sound is often still fine - until the first callback
   that is not; that is what the margin is for.
4. *Maximum polyphony* is set to the most voices that stayed clean. The
   slider is yours afterwards: lower it for more headroom (couplers, long
   releases, other programs running), raise it if you dare. The
   registration and the two attack settings are restored; the release tails
   of the run ring out at the measurement's low level for four seconds, then
   the level rises 4.5 dB a second - about twelve seconds to normal (the
   same after Cancel) - and the new limit takes effect eight seconds after
   the end, once the tails are gone.
5. The report names the result and gives advice: every route on a Windows
   Audio device (rendered by its own thread alone) and every auxiliary route
   with a device (every pipe once more), with the share of its period it
   needed, and a larger ASIO buffer (set in the driver's control panel) when
   the computer is the limit. Then it says where the time went - measured
   on this computer during this run, not guessed:
   - the CPU of the cores the program renders on (the render threads: 16
     threads busy on a 32-thread processor are 100 % here and 50 % in a task
     manager), on average and at the peak, and the whole computer's share;
     when the sound broke while the render cores were less than 60 % busy,
     the time went into one thread - the device callback - not into the
     rendering;
   - the render pool: how busy its lanes were inside the route jobs (the rest
     is waiting for the slowest lane and the hand-over between the jobs, one
     job per route and block);
   - the kernel of this processor (AVX2, SSE2, NEON or scalar): nanoseconds
     per voice and frame with the sample in the cache and streaming from RAM,
     measured right then (about a second), and the ceiling that gives at this
     buffer with these render cores - the measured limit as a share of it;
   - what one voice cost a lane per block during the run, in its parts:
     the bookkeeping (commands, pitch, gains, tone targets), the span logic
     (loops, splices, envelopes), the interpolation kernel and the per-key
     tone filtering - timed on the lanes themselves while the run went;
   - the callback time of the run split into a fixed part per block (the
     hand-over, the IR heads, the bookkeeping) and a part per voice. The
     report does not forecast other buffer sizes: on a two-core laptop a
     doubled buffer gave 17 % more voices, on a Ryzen a quarter of the
     buffer still gave 62 % of the voices - no model fits, the buffer is
     cheap to try and the measurement tells. A buffer below 256 samples
     gets a recommendation to try 256 or 512, nothing more.
   On Linux the report names the CPU governor ("performance" keeps the
   clock up), on Windows the power plan; when the CPU clocks cannot be read
   it says so. During the run the progress text shows the CPU of the render
   cores and the lane utilisation every second. The measurement is also
   started from the audio tab of a division's Settings (*Measure polyphony*,
   beside *Reopen device*).

Then save the configuration: the value belongs to this organ. Measure again
after changing the audio routes, the IRs, the buffer size or the samples.
The run takes one to two minutes; *Cancel* restores everything.

## 8.2 Global sound

Every slider of this section has a checkbox that shows it on the main panel
too (the shown sliders share a row with the polyphony display). All are
MIDI-learnable (right-click the slider on the main panel).

### Master volume

The level of the whole organ, -60 to +12 dB (default -18 dB). Lower it when the
output clips with full registration; the perspective and division volumes set
the balance below it.

### Global pitch

Shifts the whole organ by -200 to +200 cents (default 0) on top of the
temperament, for playing with instruments tuned elsewhere than a1 = 440 Hz.

### Sustain fade-out on key release

How long the sounding pipe fades out when its key is released and the release
sample takes over (default 70 ms). Too short and the join clicks or the pipe
stops unnaturally; too long and the release sounds smeared. The value is for
the sounding c1 (MIDI 60 of an 8' rank, the rank's footage counted in): pipes
below it get 1.5 times as much per octave (a 32' C 3.4 times), because a big
pipe's sound cannot stop as fast; higher pipes the value as set. A filled pipe
counts as the pipe it stands for.

### Release sample fade-in

How long the release sample fades in when the key is released (default 70
ms), scaled by pitch exactly like the sustain fade-out. The two together shape
the join between the held note and its release.

### Release sample volume

The level of all release samples, -20 to +6 dB (default 0); the leftmost
position "-inf" silences them. Quieter releases make a drier organ without
losing the room entirely.

### Release cutting

Release samples are faded out over this share of their length and end there
(5 to 100 %, default 100 = the full release). Shorter releases free polyphony
sooner on a weak computer and dry a wet set.

### Attack minimum time

The shortest time a pipe sounds once it has started (0 to 500 ms, default
80): a key released sooner is released when this time is up, so even the
briefest touch lets the pipe speak and lets its chiff be heard, as on a real
action. Longer values make staccato playing more deliberate.

### Attack minimum delay

A small pause between the key press and the start of the pipe (0 to 500 ms,
default 2), varied at random by a couple of milliseconds so that a chord's
pipes do not all speak in the same instant. Keys repeated faster than the delay
follow each other without it, and a key struck again right after its release
gets only half or none of it, so fast repetitions stay crisp.

## 8.3 Advanced global audio

### Swell: harmonic filter

Closing the swell box does not only make the pipes quieter: it also takes away
their brilliance. This is a high shelf above about 3 kHz that follows the
shutters: this much loss with the box closed (0 to 12 dB, default 8). 0 dB =
off. The
three harmonic filters (swell, wind, tremulant) add up, capped at 12 dB; when
all three are 0 dB the filter costs nothing.

### Wind: harmonic filter

The same shelf following the wind sag: this much high-frequency loss at the
full sag (default 2 dB). Big chords starve the wind, and a starved pipe is
duller as well as flatter.

### Wind: volume change per cent of sag

How much quieter a pipe gets for every cent the wind sag lowers its pitch (0
to 0.6 dB per cent, default 0.2). Together with the division's *Wind: depth*
it decides how strongly a full chord sags in level.

### Wind: one stop's share of the full sag

The sag a single engaged 8' stop produces, relative to the sag with many stops
engaged (2 to 100 %, default 20). Smaller footages count less. A lower value
keeps a solo stop steady and lets the sag come with full registration.

### Wind: one pipe's share of the full sag

The sag with one key held, relative to the sag with many keys held (default
50 %).

### Wind: two pipes' share of the full sag

The sag with two keys held (default 50 %); from three keys on the rest of the
sag closes gradually. One and two pipes together shape how a single note, an
interval and a chord differ.

### Wind: maximum wind model effect

The strength of the whole wind model, 0 to 100 % (default 100). The steady sag
of held keys and engaged stops follows it directly; every key stroke shakes the
wind with a random 40 to 100 % of it, so repeated notes never sound mechanical.
0 % switches the wind model off everywhere.

### Tremulant: harmonic filter

The shelf following the tremulant: this much high-frequency loss at the bottom
of its dip (default 4 dB), so the tone breathes as well as the level.

### Tremulant: harmonic filter phase

How far the tone oscillation lags the volume oscillation (0 to 360 degrees,
default 90): 0 = the tone dips exactly when the level does, 90 = a quarter cycle
later, 180 = the tone dips when the level peaks. Real tremulants differ here;
listen with a soft flute.

### Key velocity: fade-in at the lowest velocity

An organ key never plays softer when it is struck gently. Instead, a gentle
stroke on a stop without velocity layers starts the note with a short fade-in:
this long at the lowest velocity (default 40 ms), shrinking linearly to nothing
at velocity 127. Stops with velocity layers pick the layer instead and get no
fade-in. With *Ignore MIDI velocity* on for a division there is never any. See
"Velocity layers".

### Key velocity: treble cut on the attack at the lowest velocity

The chiff of a gently struck key is blunted: the pipe starts with this much
high-frequency loss (a high shelf above the treble start frequency) at velocity
1, less for firmer strokes, none at 127 (default 9 dB). The level itself stays
normal. Stops with velocity layers are not touched.

### Key velocity: treble cut opens over

How long the treble cut of a gentle key takes to open fully after the key goes
down (5 to 1000 ms, default 80).

### Tremulant: volume change per cent of pitch

The tremulant modulates pitch (the division's *Tremulant depth*) and level;
this is the level modulation per cent of pitch modulation (default 0.187 dB
per cent). Higher values give a tremulant that is more heard in the loudness
than in the pitch.

### Missing pipes: neighbours borrowed as a group

Pipes a rank does not have (beyond its compass, in gaps, the extra octave of an
extension) are filled from the nearest real pipes: this many adjacent pipes are
taken as a group and transposed by the group's width, and the group repeats for
further pipes (1 to 12, default 12). With 12 every missing pipe is the octave
transposition of a real one, so the rank's chromatic variety is kept; with 1
the edge pipe alone serves all missing pipes. Fewer semitones of transposition
sound better, so a last group smaller than the setting takes the outermost real
pipes with the smallest shift possible. Changing it refills every stop and
rebuilds the copies. See "What a filled pipe is".

### Retune limit per pipe

How far a pipe's recorded pitch (its smpl chunk) may lie from its key before
the pipe counts as a faulty sample. Within the limit the pipe is retuned to
its key when a1 = 440 Hz or a temperament asks for it; beyond it the pipe is
switched off and played from its neighbours like a missing pipe: the key's
thumb in the voicing turns green like a filled key's and its tooltip says so
("Faulty pipe: its own sample 035-B.wav
sounds 400 cents off its key, beyond the retune limit - switched off and
taken from the neighbour"). A pipe a third off is a wrong file, not a
tuning: Adlington's "Ch. Flute" has its 035 and 037 recorded a major third
low. 200 cents (a whole tone) by default; the setting travels with the
organ and takes effect at once (the fills and the copies of the stops are
rebuilt).

### Filled pipes: transposition method

How a pipe beyond the compass (or in a gap) gets its pitch from a neighbour's
sample: *Resampling*, *Pitch shift (resampling + timbre correction)*, *Pitch
shift + time stretch (attack and release)* (the default) or *Phase vocoder
(Signalsmith Stretch)* (optional, for those who prefer it). "The
transposition methods" describes each with its strengths and weaknesses. Switching the method rebuilds the copies of every
stop (from the cache when it is on); until a copy is ready the pipe plays
resampled. The sliders below that do not apply to the chosen method are greyed
out.

### Filled pipes: release length (pitch shift)

Pitch shift method only. How long a filled pipe's release (the room decay) is,
as a share of its source's duration (25 to 200 %, default 100). Resampling
alone would make a release shifted two octaves down four times as long and one
shifted two octaves up a quarter, but the room does not grow or shrink with the
pipe, so the decay is laid out again to this length in both directions (100 % =
exactly the source's duration). Longer than 100 % gives a bigger room to the
extension than to the stop; shorter dries it. Changing it rebuilds the release
copies.

### Filled pipes: time stretch of the attack

Pitch shift + time stretch only. How far the attack (everything before the
first loop) is stretched back from the resampled length towards the source's
(0 to 100 %, default 50). 0 % leaves it as resampled - two octaves down the
attack is four times as slow; 100 % gives it the source's duration; 50 % is
half way on a logarithmic scale (two octaves down: twice as slow, which the ear
accepts where four times is too much). Applies to shifts downwards; upwards the
attack stays as resampled. Higher values fray the attack more (see "The
transposition methods"); lower
the slider for a stop whose speaking turns rough. Changing it rebuilds the
copies.

### Filled pipes: time stretch of the release

Pitch shift + time stretch: how far the release is stretched back from the
resampled length towards the source's, in both directions (0 to 100 %, default
55). 100 % gives it the source's duration (a 32' extension's room decay as long
as the 8' pipe's), 0 % leaves it as resampled, 55 % lies between on a
logarithmic scale. Phase vocoder (the label then reads "upwards"): a release
shifted upwards keeps the source's length, too long for a smaller pipe, and is
stretched towards the length the resampling would give; shifts downwards are
left alone. Changing it rebuilds the release copies.

### Filled pipes: release played from

Every pitch-shift method. A filled pipe's release copy is played from this
many milliseconds in (0 to 300, default 140), skipping its start, where the
methods leave their worst: a stop transient that no longer belongs, a level
that does not follow the sustain. Applied when the key is released: nothing is
rebuilt, the change is heard at once. Lower it if the releases of an extension
start too late or too softly, raise it if they begin with a bump.

### Filled pipes: release level matched to the source

Every pitch-shift method. On: a filled pipe's release copy is played at the
level relation the source pipe's release has to its sustain (measured when the
copy was built), so the step at the key release is the real pipe's. Off
(default): the copy plays at its own level. Applied at playback, nothing is
rebuilt; switch it on when the releases of an extension are clearly louder or
softer than those of the real pipes beside them.

### Filled pipes: loop crossfade

The splice of the loops of the filled pipes' copies, as a share of each loop's
length (0 to 100 %, default 40). 0 % is the ordinary short splice (the loop
points of a copy are moved to whole periods of its pitch anyway); a longer
blend hides a seam that is not quite periodic, at the cost of a slight chorus
inside the loop. Applied to the existing copies at once. This is the remedy
for the loops the phase vocoder breaks (its output at the loop end no longer
matches the loop start) and for the seams the other methods make slightly
worse; raise it when a held extension pipe clicks or bumps at every repeat of
its loop. See "The transposition methods".

### Filled pipes: loudness kept when pitch shifting

Pitch shift methods. On (default): every shifted copy is brought to the
loudness of its source sample, so a filled pipe is as strong as the pipe it was
made from. Off: the copy keeps whatever level the shift left it with (a copy
with its formants moved down loses level).

### Filled pipes: keep the generated copies on disk (cache)

On by default (0.8.124; off before). On: the copies of the filled pipes are written to files as
they are built and read back the next time the same stops load with the same
settings, so the build after loading takes seconds instead of minutes - the
build of a large organ otherwise takes a minute or more at every load. One
file per rank and folder tree, whatever the settings: a tree built with other
settings replaces its file, so the cache takes about as much room on disk as
the copies in RAM (hundreds of MB for a large organ) and never plays a stale
copy. Switch it on when the build after loading annoys you; switch it off
when disk space is short - everything works the same without it. "The cache"
describes it in full.

### Filled pipes cache: Empty the cache

The row shows the size of the cache and the folder it lives in. *Empty the
cache* deletes every cached file at once - to get the disk space back, or
after you have replaced your sample folders. The copies are built again, and
cached again, the next time they are needed; nothing else is lost.

### Filled pipes cache: files of stops not loaded now

*Remove them* deletes only the cache files of ranks that are not loaded at
this moment: the stops of other configurations, of folders you unloaded or
deleted from the disk. The files of the loaded organ stay, so its next load
is still fast. A message says how many files went. This is the button for
routine housekeeping.

### Filled pipes cache folder

Where the cache files live. By default `FilledPipesCache` in the
application-data folder, usually on the system drive; the row then says
"(default)". *Choose...* opens a folder dialog: pick a folder on a big drive
(beside the samples, say). The files built so far are moved there at once, so
nothing has to be rebuilt. The folder is remembered in the settings.

### Filled pipes: resampling quality

How the sample of a neighbouring pipe is played at another pitch. *Linear* is
the interpolation of every other voice (cheapest, audible images at large
shifts). *Cubic interpolation* lowers the interpolation images by about 20 dB
for these voices only. *Cubic + anti-alias filter* (default) also removes what
would fold back when a pipe is transposed upwards: a low-passed copy of the
source is built for each such filled pipe (a few MB per rank, nothing for stops
without upward shifts). Real pipes are never touched.

### Treble per key: shelf start frequency

The *Treble per key* sliders of the voicing raise or lower everything above
this frequency (120 to 5000 Hz, default 1800): a high shelf whose transition
band runs from here to about twice this frequency. A pipe whose recorded
fundamental lies higher starts its shelf at that fundamental instead. The
velocity treble cut uses the same frequency.

### Celeste beats: top of compass vs bottom

Undulating (celeste) stops beat this many times faster at the top of their
compass than at the bottom (1 to 20, default 12); the a1 key always beats as
set in the stop's voicing. Equal beats everywhere (1x) detune the bass most,
as a real celeste tuned to constant beats would.

## 8.4 Console

### Interface size

The size of every window, button, slider and text, in per cent of the design
size (50 to 300 %). The first start estimates it from the screen so that Full
HD, 4K and Retina screens show the console alike; double-click returns to that
estimate. Applied when the slider is released: every open window keeps its
place and size on the screen, only its content changes size.

### Divisions

How many manuals the console has (1 to 7; the pedal is always there). Reducing
the number forgets everything about the removed divisions (folders, audio,
MIDI, grid); a division added later starts empty, with the wind and tremulant
profiles of a neighbouring division.

### Combination pistons

The number of combination pistons in the combination row (0 to 10). See "The
combination row".

### Program windows

*One window*, or *Two windows*: a second program window opens, and every
division section below gets *Shown in window* to move a division there. The
second window's position is remembered; closing it returns to one window.

### Top bar: PANIC

Shows or hides the PANIC button, which silences every sounding pipe at once.

### Top bar: Record

Shows or hides the Record button (the stereo mix to a WAV file).

### Top bar: Stop List

Shows or hides the Stop List button (see "4. The Stop List (building the
organ)").

### Top bar: Couplers

Shows or hides the Couplers button (see "Couplers").

### Top bar: Temperaments

Shows or hides the Temperaments button (see "Temperaments").

### Top bar: Hide/Show all voicing

Shows or hides the *Hide all voicing* button, which folds every open voicing
row.

### Top bar: Perspective Patterns

Shows or hides the button editing the perspective prefixes (the same list as
*Folder name patterns > Perspective prefixes*).

### Top bar: Ignored Patterns

Shows or hides the button editing the ignored folders list.

### Top bar: One-shot Patterns

Shows or hides the button editing the one-shot stops list.

### Top bar: Stop-Merging

Shows or hides the Stop-Merging button (see "Stop-Merging"). Hidden by
default.

### Top bar: Load Voicing

Shows or hides the Load Voicing button (the voicing from a file). Hidden by
default.

### Top bar: Save Voicing

Shows or hides the Save Voicing button. Hidden by default.

### Top bar: Load Configuration

Shows or hides the Load Configuration button (an OrganSampler `.json`, or a
Hauptwerk organ definition - see "Loading a Hauptwerk organ definition").
Right-click: merge another configuration into the loaded organ.

### Top bar: Save Configuration

Shows or hides the Save Configuration button.

### Top bar: Shut down PC

Shows or hides the Shut down PC button (see "Top bar buttons"): save, quit and shut the
computer down without any question. Hidden by default. A hidden button still
fires from its learned MIDI control, which is how a headless console uses it.

### Top bar: Unpack archive

Shows or hides the Unpack archive button (see "Sample archives (RAR)"): RAR
archives of sample folders unpacked by the program into a folder of your
choice. Hidden by default since *Load folder...* reads the archives itself
(and unpacks on request); the button beside this switch does the same
without the top bar.

### Top bar: GLOBAL RESET

Shows or hides the GLOBAL RESET button (see "GLOBAL RESET").

### Melody/bass couplers: new extreme key only

On (default): a melody (bass) coupler couples a key only when it becomes the
new highest (lowest) key held; when that key is released the coupler does not
fall back to the next one. Off: the coupled note always follows the highest
(lowest) key currently held.

### Melody couplers: legato grace time

On: when the highest key changes in legato playing, the melody coupler waits a
moment before moving to the new key, so a brief overlap of two keys does not
make the solo stop jump. Off: it follows at once.

### Load the last configuration at startup

Off by default. On: the configuration file used last time is opened when the
program starts. Otherwise the program starts with an empty instrument and
nothing is loaded until you ask.

### Samples' base temperament

The temperament the loaded samples were *recorded* in. It is found from the
samples by itself when two thirds or more of the ranks agree (the activity
log says so); set it by hand only when the ranks disagree or the set has no
smpl pitches - the voicing row of every stop reports what its ranks were found
to be in. Every pipe's retuning to the target temperament starts from here;
a wrong base temperament puts the whole organ slightly out of tune. See
"Temperaments".

### Target temperament

The temperament the organ *plays* in (see "Temperaments"). Every pipe is retuned by the
difference between the target and the samples' base temperament, per pitch
class. Choosing the base temperament itself plays the samples as recorded.
The Temperaments dialog of the top bar does the same with one click, and
every temperament can be switched from a MIDI control.

### Window size

The size of the main window, saved automatically. Information only; the window
returns to where it was last time, always with its title bar on the screen.

## 8.5 Folder name patterns (one per line)

### One-shot stops (folder contains)

Folders whose name matches one of these patterns hold samples that play once
without looping (chiff, percussion, bells). Defaults: "Chiff", "Chimes",
"Tubular", "Celesta", "Harp", "Glockenspiel" (a Hauptwerk import adds the
folders of every stop the definition plays without release samples). The patterns
match like those of *Ignored folders (folder contains)* (a star at the start
or the end anchors them). Applies to the next folder load.

### Ignored folders (folder contains)

Folders whose name matches one of these patterns are not loaded at all
(tremulant recordings, say). Defaults: "trem" (anywhere in the name),
"*_tr" (names ending in "_tr") and "output" (the "outputsum"/"outputharm"
renderings under a rank). A folder named after its sibling or parent with
"tr"/"trem" appended is skipped by every tree scan besides (see "Folders to
skip" in chapter 2). A pattern without a star matches anywhere, a
star at the start anchors it to the end of the name, a star at the end to its
start; underscores count. Applies to the next folder load.

### Perspective prefixes

Folder name prefixes that mark a microphone perspective, one line per
perspective. Every prefix that belongs to the same perspective must stand on
that one line, separated by semicolons; the perspective takes its name from
the first entry of the line ("Direct;01-dirMk2H-;dir" makes folders with any
of the three prefixes the perspective "Direct"). Defaults: Direct, Diffuse and
Rear. Prefixes found in a newly loaded set are added here by themselves (a
prefix that shares its stem with a known perspective joins its line; an
unknown one gets a line of its own - move it onto the right line if it is the
same perspective). Applies to the next folder load.

## 8.6 Division sections (Pedal, Manual I ... Manual VII)

One collapsed section per division; the same settings as in the Divisions
dialog (see "Keyboard, MIDI and compass", "Swell, tremulant, wind" and "The
audio tabs").

### Stops and sample folders

The folder trees loaded for this division, and the *Stop building...* button
that opens the Stop List (see "Folder trees and the pool") at it.

### Shown in window

With two program windows: the window this division is shown in. A division is
in one window only; the divisions of each window share its width.

### MIDI keyboard device

The MIDI input that plays this division. Several divisions may listen to the
same device (on different channels, or on the same channel to play them
together).

### MIDI channel

The MIDI channel (1 to 16) of the division on that device.

### Ignore MIDI velocity (organ keys)

(The Divisions dialog names the same switch *Disable MIDI velocity*.)
On: every key is sent as full velocity, so there is never a velocity fade-in
or treble cut (see "Key velocity: fade-in at the lowest velocity") and always
the top velocity layer (see "Velocity layers"). For consoles whose keys send
unreliable velocities.

### Bass octave layout

How the lowest octave of the keyboard is laid out, for playing historic music
on a historic layout: *Standard*; *Short octave* (the bottom E key sounds C,
F# sounds D, G# sounds E, the keys C to D# are silent); *C D D# ...* (no C#);
*C D E ...* (no C# and no D#). The compass of the division is set on the same
line of the Divisions dialog.

### MIDI transposition

The keys of this division's MIDI keyboard play this many semitones away from
what they send (-24 to +24, default 0): with -12 the key 036 plays the pipe
of 024, with +12 the pipe of 048. For a keyboard that sends the wrong octave,
or for playing a part transposed. Keys that land outside the MIDI range are
silent. In the Divisions dialog the box is the last line of the MIDI column.

### Swell pedal controller

The MIDI controller learned for this division's swell (right-click the swell
bar in the division header to learn it). Information only.

### Swell: maximum attenuation when closed

How much quieter the division is with the swell box fully closed (0 to 60 dB,
default 13). The harmonic filter (*Swell: harmonic filter*) dulls the tone at
the same time.

### Swell: shutter inertia (settling time)

How slowly the shutters follow the pedal (50 to 2000 ms, default 300). Larger
= heavier, smoother movement; a sudden pedal stroke then takes this long to
arrive.

### Division volume

The level of the whole division, -60 to +12 dB (default 0), for balancing the
divisions against each other.

### Audio output channels (default perspective)

The channels of the division's audio device that the default perspective
plays on, as numbers separated by commas (the same channels the Divisions
dialog shows as a list of tick boxes on the audio tab; either place changes
the other). Press Enter or click outside to apply. How mono and stereo
samples are spread over these channels is
explained in "Sample channels and output channels". The device itself is
chosen in the Divisions dialog's audio tab; which devices and drivers to use,
and what the auxiliary route is for, is explained in "Audio drivers: many
sound cards at once" and "The auxiliary route".

### Perspective output channels

The same for one named perspective (Direct, Diffuse, Rear, auxiliary): the
channels of that perspective's own device.

### Perspective volume

The level of one perspective of this division (-31 to +6 dB, default 0); -31
dB switches the perspective off completely and saves its voices.

### Perspective channel separation

At 100 % (default) every pipe sounds only on its own channel pair of the
perspective; less lets the other channels carry that share of it too; 0 % = all
channels carry the same sum.

### Random tuning error of pipes

(The Divisions dialog names it *Allow Random Tuning Error*.)
On: the pipes of this division get the small random detune set per stop
(*Random pitch error* in the voicing). Off: every pipe speaks exactly at its
pitch.

### Pipe coupling (detuned pipes pull together)

(The Divisions dialog names it *Allow Pipe Coupling*.)
On: pipes of the same pitch sounding together pull each other into tune a
little, as neighbouring pipes on one chest do. Off: every pipe keeps its own
detune.

### Tremulant rate

The speed of the division's tremulant (2 to 11 Hz, default 4.4).

### Tremulant depth

The pitch modulation of the tremulant (0 to 50 cents, default 6); the level
modulation follows it by *Tremulant: volume change per cent of pitch*. The
strength wanders between 60 and 100 % of this from cycle to cycle.

### Tremulant phase drift between ranks

How far the ranks of the division drift apart in the tremulant's cycle (0 to
360 degrees, default 260): with 0 every stop dips at the same moment, with a
large value the stops of a full registration shimmer rather than pulse.

### Wind: depth (maximum pitch sag)

How far the pitch sags with the wind at its lowest (0 to 50 cents, default 8).
The sag is reached with many stops engaged and many keys held, the shares being
set in Advanced global audio. 0 switches the wind model off for this division.

### Wind: oscillation frequency

The frequency the wind pressure oscillates at after a key stroke (4 to 10 Hz,
default 7.5), the "bounce" of the bellows.

### Wind: number of oscillations

How many oscillations die away after a stroke (0 to 20, default 7).

### Wind: oscillation strength

The impulse a key stroke gives the wind (0 to 100 %, default 40); 100 % is
twice the impulse of 50 %, and the sag ceiling grows with it.

### Wind: sag settling lag

How long the wind pressure takes to settle after stops or keys change (100 to
400 ms, default 255). The same lag governs the volume glide when a pipe held
through several routes passes from one route to another.

### Wind: also affects divisions

Tick the other divisions fed from this wind supply: they receive half of this
division's sag, so a heavy chord on one manual disturbs the other.

---

## 8.7 Keeping track: the activity log

Everything the program does at a coarse level - devices opened, divisions
loaded, builds of the filled pipes' copies - is noted in `CrashLogs\activity.log`
in the application-data folder; a crash writes a file beside it. When the
window answers nothing for eight seconds the log records where it was waiting.
That folder is what a report needs - and *Report a problem...* sends it for
you.

### Report a problem

*Report a problem...* at the top of the Configuration Editor opens a dialog
for what happened and how to reproduce it. When you press *Create the report
and open GitHub*, the program writes a folder `OrganSampler-report-<date>`
into `Reports` in its application-data folder (0.8.188; nothing goes onto
your Desktop) with `logs.zip` (the activity log, the crash reports,
`settings.json`, the last self-test report and `system.txt` - the version,
the computer, every audio route with its device, buffer, rate and channels;
no samples, no voicing, no combinations) and PNG screenshots of every window
of the program as it is at that moment (keep the dialog in question open;
untick the box if you do not want them), plus any screenshot files you add
with *Add screenshot files...*. Then the GitHub bug report opens in your
browser with its fields filled in from the dialog: drag `logs.zip` into
"Logs" (the first field of the form) and the pictures into "Screenshots",
check the text, submit. The report folder itself opens in the file manager
beside it, and the program's own message sits at the bottom of the screen,
out of the way. The program puts the browser on the left half of the screen
and the folder on the right half, side by side, so the zip is dragged across
(0.8.139; Windows always, macOS through AppleScript - the browser only with
the accessibility permission -, Linux where wmctrl is installed). The
link carries the computer and the routes only while it stays short (0.8.138;
GitHub answered a long one with an error after its sign-in) - the whole
list is in `system.txt` of `logs.zip` in any case. A free
GitHub account is needed to submit; the report is public, like every issue
of a free program.

GitHub may first ask you to sign in or to make the account, and after that
it sometimes lands on another page instead of the form. The message that
stays open in OrganSampler has *Open the GitHub form* for that: press it
again, signed in, and the form opens filled in. The same link is in the
report folder as `Report on GitHub.html` - a double-click opens it, even
days later.

## 8.8 Free software, licences, platforms

OrganSampler is free software under the GNU GPL v3; the source code is on
GitHub and every release is built there from it. *About & licences...* at the
top of the Configuration Editor shows this manual, the program's licence and
the licences of the software it contains: JUCE (AGPLv3), the Steinberg ASIO SDK (under its
GPLv3 option; ASIO is a trademark and software of Steinberg Media Technologies
GmbH), Signalsmith Stretch and Signalsmith Linear (MIT). The same program is
built for Windows, macOS and Linux; the macOS and Linux builds have had less
playing time than the Windows build. What each build needs:

- **Windows:** `OrganSampler-windows.zip` - Windows 10 or 11, 64-bit. ASIO,
  Windows Audio (WASAPI) and the aggregated WASAPI device.
- **macOS:** `OrganSampler-macos.zip` runs on Apple silicon Macs with macOS
  11 Big Sur or newer; `OrganSampler-macos-10.11-intel.zip` on Intel Macs
  with macOS 10.11 El Capitan or newer. CoreAudio.
- **Linux:** `OrganSampler-linux-x86_64.tar.gz` - x86-64 with glibc 2.38 or
  newer - a distribution of 2024 or later: Ubuntu 24.04, Linux Mint 22,
  Debian 13, Fedora 39 and their successors. ALSA or JACK (PipeWire).

Where the program keeps its files (settings, the activity log, crash
reports) - the "application-data folder" of this manual:

- Windows: `%APPDATA%\OrganSampler` (`C:\Users\<you>\AppData\Roaming\OrganSampler`)
- macOS: `~/Library/OrganSampler` (in Finder: *Go > Go to Folder...*, type
  `~/Library/OrganSampler`)
- Linux: `~/.config/OrganSampler`

`CrashLogs/activity.log` is the activity log, `CrashLogs/crash-<date>.txt`
a crash report with the stack of the crash; macOS writes its own report as
well, in `~/Library/Logs/DiagnosticReports` (named `OrganSampler_...`).
`settings.json` is the state of the program, `settings-<date>.json` the
snapshot of each of the last seven days. If the program crashes at every
start, move `settings.json` away (keep it for the report) and it starts
clean; a crash while an audio device was being opened is caught by the
program itself: the next start opens that device with the default buffer
and rate and says so.
