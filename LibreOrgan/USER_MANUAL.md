# LibreOrgan Sampler - User Manual

**PIPE ORGAN SAMPLER FOR CREATIVE MINDS.**
**Design your console, build your stop list, perform!**

LibreOrgan Sampler plays sampled pipe organs: every pipe is a recording, every stop a
folder of recordings. The program turns any number of such folders into an
organ with up to seven manuals and a pedal - couplers, combinations, crescendo,
swell boxes, tremulants, a wind model, reverb per microphone perspective, and
extensions and transmissions built from the stops you have.

This manual lives inside the program: *Configuration Editor > About &
licences...*. The chapter box above the text jumps to any chapter, *Find*
searches it, *A-* / *A+* (or Ctrl + wheel) change the text size. Text can be
selected with the mouse and copied (Ctrl+C, or the right-click menu). Chapter
names shown as links jump to the chapter. **F1** over any control of any
window opens its chapter, and every right-click menu ends with *Manual: ...*;
the "?" beside every option of the Configuration Editor does the same, with
the heading at the top. Parts 1 to 7 explain the program as you use it, part
8 is the reference of every option, part 10 holds the technical details.

Contents

1. [Getting started](#1-getting-started)
2. [The main window](#2-the-main-window)
3. [Stops and their voicing](#3-stops-and-their-voicing)
4. [The Stop List](#4-the-stop-list)
5. [Divisions and audio](#5-divisions-and-audio)
6. [Playing](#6-playing)
7. [Filled pipes](#7-filled-pipes)
8. [Configuration Editor reference](#8-configuration-editor-reference)
9. [Messages, the activity log, reporting a problem, licences](#9-messages-the-activity-log-reporting-a-problem-licences)
10. [Technical details](#10-technical-details)

---

# 1. Getting started

## 1.1 What you need

- Samples: folders of WAV files, one folder per stop. Sonus Paradisi sample
  folders (wave format) work as they are, packed in their RAR archives or
  unpacked. Other samples must follow "How the sample folders must be
  organised".
- An audio interface. ASIO on Windows gives the lowest latency and the most
  polyphony; Windows Audio works too. macOS: CoreAudio. Linux: JACK (PipeWire)
  or ALSA.
- A MIDI keyboard or an organ console with MIDI out.

No organ definition file is needed: load the folders, the program proposes the
organ, you shape it and save it as a configuration. A configuration somebody
else published for the same samples saves you all of that: *Load
Configuration*, choose the file, play. LibreOrgan Sampler does not load Hauptwerk
organ definitions either - it has no use for them, the organ is built from the
sample folders. Where a definition lies beside the samples it is read for two
things only: where the sample folders are and how the stops are arranged in the
divisions. From that LibreOrgan Sampler deduces the microphone perspectives and the
relative volumes of the stops, the divisions and, where needed, the pipes - a
computation of its own, beyond what the definition holds (Hauptwerk knows no
perspectives and no relative volumes, only absolute pipe levels and
"stop-ranks") - and everything else comes from the folders.

## 1.2 Installation

One executable, no installation. Download the archive of your system from the
releases page, unpack it, put the program anywhere, start it:
`LibreOrgan-windows.zip` (Windows 10/11, 64-bit), `LibreOrgan-macos.zip`
(Apple silicon, macOS 11 or newer), `LibreOrgan-macos-10.11-intel.zip`
(Intel Macs, macOS 10.11 or newer), `LibreOrgan-linux-x86_64.tar.gz`
(`tar xzf`; Ubuntu 24.04, Mint 22, Debian 13, Fedora 39 or newer),
`LibreOrgan-linux-arm64.tar.gz` (a Raspberry Pi 4 or 5 with a 64-bit OS,
Raspberry Pi OS Bookworm or newer - a test build, not yet tried on real
hardware by the project; count on a few thousand pipes of polyphony and on
the saving options of the load box). The window title shows the version.

Linux: *Install menu entry and icon* at the bottom of the Configuration Editor
puts the program into the application menu with its icon. The entry follows
the executable: after moving it or downloading a new one, start that file
once from the file manager and the menu runs it from then on.

## 1.3 The first start, step by step

### Step 1: Size

Resize the window like any other. If the console is too big or too small for
your screen, move *Interface size* in the Configuration Editor.

### Step 2: Divisions

The *Divisions:* box in the upper left corner of the main window sets how
many manuals the console has; the pedal is always there.

### Step 3: MIDI

Right-click the keyboard icon in a division's header, choose *MIDI Learn...*
and press any key on the keyboard that is to play the division. Repeat for
every division. (*Settings* in the header opens the Divisions dialog, where the
device and channel can also be chosen by hand.)

### Step 4: Samples

*Stop List* in the top bar, *Load folder...*, choose the top folder of the
samples of one division - or the RAR archive(s) the set came in - and say
which division its stops go to. Repeat for every division, or load one tree
holding everything and move the stops between divisions in the table. *Apply
and reload*: the stops appear as buttons ([The loading window](#the-loading-window)).
A bought RAR set, a configuration
from GitHub or an installed Hauptwerk organ take shorter roads: "The ways to
an organ".

### Step 5: Audio

*Settings* in a division's header, then the audio tabs of the Divisions
dialog: one tab per perspective of the division and one *auxiliary* tab. On a tab choose the device type (Windows: ASIO when you have it), the
device, 48 kHz, a buffer of 512 samples, and tick the output channels of the
device the perspective plays through. *Set for all perspectives and divisions*
copies the device to every route; the tick box *Set these channels for all
divisions (this perspective)* above the channel list copies the channels to
the same perspective of every division (a later change of one route's
channels unticks it). Leave the *auxiliary* tab at *none*. Details: [The audio tabs](#53-the-audio-tabs).

### Step 6: Play

Draw the stops and play.

### Step 7: Save

*Save Configuration* writes everything - folders, stops, voicing, audio, MIDI,
combinations, couplers, crescendo - into one file in your LibreOrgans
folder. *Load the last configuration at startup* (Configuration Editor >
Console) opens it again next time.

## 1.4 Where the files are

**Your samples: anywhere you like.** Keep every folder tree of an organ on one
disk, under one folder, and never move it again: the configuration stores the
paths relative to that place, and every place you once chose is remembered.

**RAR archives: anywhere you like.** The program reads the samples straight
out of the archives you choose - in *Load folder...*, *Load Configuration* or
a merge - and remembers where they lie (`Locations.txt`, below), so the next
configuration of the same set finds them without a question. It never opens an
archive you did not choose.

**Linux: mount the sample disk permanently.** A disk the desktop mounts only
when you open it in the file manager (`/run/media/...`) is not there after a
reboot. Give it a permanent mount ("Mount at system startup" in the desktop's
Disks & Devices settings, or `/etc/fstab`) and point the locations there. If a
remembered location does lie under `/run/media` and is missing at a load, the
program mounts the disk by its label itself and says so in the activity log.

**Your configurations** live in one folder: `Documents\LibreOrgan\
Configurations` (chosen anew in the Configuration Editor, *LibreOrgans
folder*). Every import writes there, Load and Save start there; a file opened
from elsewhere is copied there by the load box, or loaded where it is. The
exports for publishing go into its `Publish` subfolder; recordings into
*Recordings folder* (`LibreRecordings` beside the configurations by default).

**The settings of this computer**: `C:\Users\<you>\AppData\Roaming\LibreOrgan`
on Windows, `~/Library/LibreOrgan` on macOS, `~/.config/LibreOrgan` on
Linux. In it `settings.json`, `Locations.txt` (the sample locations and the
chosen archives, one path per line, editable), `Patterns.json` (your folder
name patterns), the archive indexes, the filled pipes cache, the activity log
and the reports. The Configuration Editor shows the locations (*Sample and IR
locations*) and opens the file (*Show*).

How a configuration finds its folders on a new computer: "Where the program
looks for folders".

## 1.5 The ways to an organ

Every one of them ends with *Save Configuration*.

### A set in RAR archives

Put all the archives of the set in one folder on your sample disk. *Load
Configuration*, choose all of them at once (Ctrl or Shift in the file dialog):
the program reads what they hold, offers the Hauptwerk organ definition (or a
configuration) found inside and loads the whole organ from the archives. The
archives are remembered. A set sold as several separate archives can be loaded
from one of them: the others with a name like it are read too.

### A configuration published on GitHub

Download the zip from `libreorgans/INDEX.md` of the project, *Load
Configuration*, choose the zip as it is: the configuration inside is copied
into your LibreOrgans folder and loaded. The samples are looked for in your
sample locations; on a computer that knows none yet, "Sample folders not
found" shows the way.

### A Hauptwerk organ already installed

*Load Configuration*, choose the organ definition in Hauptwerk's
`OrganDefinitions` folder (`<organ>.Organ_Hauptwerk_xml`). LibreOrgan Sampler does
not load the definition - it builds the organ from the sample folders, as
always. The definition is read for two things only: where the sample folders
are and how the stops are arranged in the divisions. From that LibreOrgan Sampler
deduces the microphone perspectives and the relative volumes of the stops, the
divisions and the pipes - its own computation, which the definition does not
hold (Hauptwerk knows no perspectives and no relative volumes, only absolute
pipe levels and "stop-ranks") - and the rest - the pipes, their pitch, footage
and tuning - comes from the folders themselves. The result is an
LibreOrgan Sampler configuration in your LibreOrgans folder, which you shape and
save like any other. Encrypted sets (`.hbw`) cannot be read. The rules:
[Hauptwerk import rules](#104-hauptwerk-import-rules).

### A set without a definition

*Stop List* > *Load folder...* with the top folder of the samples, say which
division, repeat per division, *Apply and reload*. [The Stop List](#4-the-stop-list) explains
the table, [How the sample folders must be organised](#17-how-the-sample-folders-must-be-organised) what the program makes
of the folder names.

### A second volume or perspective set of the same organ

Best: load all the volumes at once - choose every archive of the set together
in *Load Configuration*. Volume by volume: load the first, then **right-click**
*Load Configuration* > *Merge another configuration into this organ* with the
second one's definition, configuration or archives. Stops you have gain the
new perspective, new stops are added. A perspective set whose folders stand
beside yours under a new prefix joins your rows the same way, in the merge and
in *Load folder...* alike. How the program decides that two folders are the
same pipes: [The tuning gate of a perspective join](#105-the-tuning-gate-of-a-perspective-join).

### The samples moved, a new disk, a new computer

Load the configuration as it is: the trees are looked for at their old place,
then in every remembered location; a folder of the right name but with other
samples is refused. What is not found shows the notice below; one click puts
everything right and the new place is remembered.

### Sample folders not found

A division that misses a folder tree shows the orange notice "Sample folder
not found" under its header. Click it, pick the tree, and choose **any folder
of the set** - the tree itself, a folder above it, or a neighbouring tree. The
program finds the tree from there, checks that it holds the stop folders the
table expects, looks for the other missing trees around it, loads everything
found and remembers the place. The Stop List offers the same for every missing
tree (orange cells, *Load folder...*), and *Unload folder...* drops a tree you
do not own.

## 1.6 Configurations

### Saving and loading

A configuration is one file holding the whole instrument. *Save Configuration*
and *Load Configuration* are in the top bar; Load also takes a published
`.zip`, RAR archives, or a Hauptwerk definition (not loaded as such: read for
the folder paths and the stop arrangement only; the perspectives and the
relative volumes are LibreOrgan Sampler's own deduction, the organ is built from the
folders). A configuration with unsaved
changes asks at quit or at the next load: *Save*, *Don't Save* or *Cancel*.
Before the samples load, a box asks how they are to be held in RAM (storage,
packing, rate - [Sample storage in RAM](#sample-storage-in-ram)); the choice is saved with the
configuration. Paths are stored with "/" and work on every platform.

### The loading window

Three lines: what is being read, what the program does right now (the stop
being decoded, the tuning analysis, the pipe fills), and the reading speed
("Reading from the disk at N MB/s" - a big organ loads as fast as the disk
delivers, about 500 MB/s from a SATA SSD, several times that from NVMe).
Then *Compiling the Stop List* with its steps. *Cancel* stops the loading at
the next stop; the instrument is built from what was loaded. Only after the
window closes are the copies of the filled pipes built in the background (a
dark track under a stop button, filling orange); the organ plays at once.

### Portable configuration (to share with others)

Tick **Portable configuration (to share with others)** in the Save box. The
export holds the organ alone - folders, Stop List, voicing, IR reverbs,
couplers, combinations, crescendo, temperaments - and nothing of your computer
(no MIDI or audio devices; every route plays through the default device on the
other computer). It never touches your ordinary configuration: it writes
`<name>.portable.json`, `<name>.portable.zip` (the json with its README - the
one file to upload) and `<name>.portable.README.md` into the `Publish`
subfolder. The save asks once for the organ, the samples' producer, the link
and your name; the README is written by the program.

Then *Publish on GitHub*: the form opens in your browser with the title and the
organ filled in, the `Publish` folder beside it - drag the zip into the form
and submit. A robot checks it and proposes it; after the merge the organ is a
line of `libreorgans/INDEX.md`. A new version replaces the old one.

### Merging another configuration into the loaded organ

Right-click *Load Configuration* > *Merge another configuration into this
organ*. The added stops become further rows under the rows of their division
(drag them where they belong), their folder trees join yours, their voicing
comes along. When the program finds that an added folder is a further
perspective of a stop you have, it joins that stop's row as the new
perspective instead of adding a row. A stop whose folder is already loaded is
shared, not added; two stops of one name get "(2)". Two organs rarely stand at
one pitch, so the merge switches *Equal at 440 Hz* on. The merged organ is
loaded, not saved: *Save Configuration* keeps it.

### GLOBAL RESET

*GLOBAL RESET* (top bar) offers what to reset: *Unload everything* empties the
instrument; the other boxes (audio, MIDI, transmissions, patterns, couplers,
the grid) reset only what is ticked.

## 1.7 How the sample folders must be organised

### Folders and files

A top folder holds one folder per stop; a stop folder holds the WAV files of
its pipes and the folders of its releases. A set with several microphone
perspectives has the stop folders several times, each with its perspective's
prefix:

```
POS Organ Samples                <- the top folder: "Load folder..." in the Stop List
├── Principal8                   <- one folder per stop
│   ├── 036-C.wav                <- one WAV per pipe, named after its MIDI key
│   ├── ...
│   └── rel                      <- the releases, named like the attacks
├── dir-Trompete 8               <- the "Direct" perspective of Trompete 8
├── dif-Trompete 8               <- the "Diffuse" perspective of the same stop
├── rear-Trompete 8              <- the "Rear" perspective
└── Mixtura
    └── Chiff                    <- a one-shot stop, nested: allowed
```

- Any folder holding WAV files is a stop, however deep it is nested. A folder
  whose name starts with "rel" holds the releases of its parent. The stop's
  name comes from the folder's name: a footage at its end ("Nasat 3" = 2 2/3')
  is shown with its foot mark; a name without footage is fine, the pitch is
  read from the samples.
- A file name begins with the MIDI key number, optionally with the note
  (036.wav, 036-C.wav, 059-B3.wav). Several files with the same number are
  round-robin variants; a two- or three-digit suffix after an underscore
  (`060-C4_070.wav`) is a velocity layer ([Velocity layers](#35-velocity-layers)).
- Mono and stereo files are both fine. Loop points and the recorded pitch come
  from the WAV's smpl chunk ([The smpl chunk and the file names](#101-the-smpl-chunk-and-the-file-names)); files
  without it play as they are, but cannot be retuned.
- Missing pipes and missing loops are no obstacle: gaps are filled from the
  neighbouring pipes ([What a filled pipe is](#71-what-a-filled-pipe-is)), a file without loops plays to
  its end and loops as a whole.
- A stop recorded as a left and a right half (`quintaton16L` and
  `quintaton16R`) is one stop with two folders.

### Releases

The folder `rel` inside the stop folder, files named like the attacks. Several
folders for different note lengths are honoured: `rel500` (or `rel_0500`) for
keys held up to 500 ms, `rel1500` up to 1.5 s, plain `rel` for the rest. A
stop without releases plays too: the sound fades out when the key goes up.

### Microphone perspectives

Folders of the same stops under several prefixes (`dir-`, `dif-`, `rear-`) are
recognised as perspectives, each with its own audio route. The prefixes are
listed in *Perspective prefixes* (Configuration Editor > Folder name patterns;
also *Perspective Patterns...* in the top bar), one line per perspective: the
name first, then every prefix of that perspective, separated by semicolons.
New prefixes found in a set are added by themselves; a perspective added or
renamed in the Stop List gets its line at once.

### One-shot stops and folders to skip

Folders matching *One-shot stops (folder contains)* ("Chiff", "Chimes",
"Tubular", "Celesta", "Harp", "Glockenspiel") play once from start to end
whenever the key is released - chiffs, percussion, bells. Folders matching
*Ignored folders (folder contains)* ("trem", "\*_tr", "output") are not loaded;
tremmed recordings named after their sibling with "tr"/"trem" appended are
skipped as well. A pattern without a star matches anywhere in the name, "\*x"
the end of it, "x\*" its start.

### Sample archives (RAR)

Nothing needs unpacking. *Load folder...*, *Load Configuration* and *Merge*
take `.rar` files (a multi-volume set by its first part, several archives at
once with Ctrl or Shift) and read the samples out of them; a folder that is
also on the disk wins over the archive. The first load of an archive reads its
index (seconds on an SSD, minutes for a big set on a hard disk); the index is
kept. A stored (uncompressed) archive reads as fast as plain files, a
compressed one at the speed of the decompression. Only the archives you chose
are ever read, and every one of them is remembered. *Unpack the archives into
a folder first* in the Load folder window unpacks them instead (keep the
folder tree as the archive has it).

---

# 2. The main window

From top to bottom: the top bar of buttons, the combination row, the global
sound sliders with the polyphony display, and the workspace - one column per
division with its header and its stop grid. A message strip appears in the
lower left corner when the program has something to say ([Messages](#91-messages)).

## 2.1 Top bar buttons

Every button can be hidden (Configuration Editor > Console > [Top bar](#top-bar)) and
fired from a MIDI control (right-click it, *Learn MIDI On trigger*; a learned
button fires even when hidden).

- **PANIC** silences every sounding pipe.
- **Record** records the stereo mix to a WAV file; click again to stop
  ([Recording](#65-recording)).
- **Stop List** - the table of the whole organ ([The Stop List](#4-the-stop-list)).
- **Guided tour** (on a teaching organ such as the Rural Organ demo) walks
  through the console in ten steps with an arrow to every element.
- **Couplers...** ([Couplers](#61-couplers)), **Temperaments** ([Temperaments](#63-temperaments)).
- **Hide all voicing** folds every open voicing row.
- **Perspective Patterns..., Ignored Patterns..., One-shot Patterns...** -
  the folder name lists ([Folder name patterns](#85-folder-name-patterns-one-per-line)).
- **Stop-Merging** renders the engaged stops into one new stop
  ([Stop-Merging](#36-stop-merging)).
- **Load Voicing / Save Voicing** (hidden by default) - the voicing as a file.
- **Load Configuration / Save Configuration** - the instrument as a file; a
  right-click on Load offers the merge.
- **Shut down PC** (hidden by default) saves everything, quits and shuts the
  computer down without a question - for a console without a screen (a MIDI
  piston). An instrument without a file is kept as "Shutdown rescue
  configuration.json" beside the settings.
- **GLOBAL RESET** ([GLOBAL RESET](#global-reset)), **Configuration Editor** ("Configuration
  Editor reference").

## 2.2 The combination row

The pistons (their number: the *Pistons* box in the row), *Set*, the stepping
buttons *<<* / *>>*, the bank buttons, the display of the combination in use,
*Tutti* (every stop on), *X* (every stop off), *Save* / *Load* for
combination files, and the crescendo.

- Click a piston to recall its registration (stops, transmissions, couplers,
  tremulants, Unison Off). With *Set* on, a click stores the current
  registration in the piston.
- A piston that holds a registration is a darker grey (a taught crescendo
  step the same). The piston in use is orange; other pistons holding the
  same registration are a lighter dark grey.
- *<<*, *>>*, the bank buttons and every piston are MIDI-learnable
  (right-click). Combinations are part of the configuration.

## 2.3 The crescendo

A row of segments, one per step (40 by default). Drag it or move a learned
MIDI controller (right-click: *Learn MIDI continuous controller*). *Cresc.*
switches it on; switching it off brings back the registration set by hand.
*Crescendo settings...* (right-click the display) teaches the steps: *Set*
on, engage the stops, click the step; the dialog also sets the number of
steps, whether the crescendo may switch hand-registered stops off, and saves
or loads the steps as a file. Moving between steps switches only the
difference: a chord held through the crescendo never restarts.

## 2.4 Global sound sliders and the polyphony display

The sliders of *Configuration Editor > Global sound* can each be shown on the
main panel (the checkbox beside them in the editor); all are MIDI-learnable.
The polyphony display shows the voices sounding now (*NOW*), the most at once
over the last seconds (*PEAK*), the *DSP* load (the time the busiest audio
thread needed for its last block, as a share of the block; above 100 % the
sound breaks - [Getting the most polyphony](#57-getting-the-most-polyphony)), the memory the program holds
(*RAM GB*; orange when the computer is nearly full) and the polyphony *LIMIT*
(editable; the measurement writes its verdict there). The DSP cell shows
"!N" in orange when the program found N clicks in its own output ("Output
discontinuity detector").

The loading never lets the system page: it stops when the memory is short,
builds the instrument from the stops loaded so far and names the rest in a
message; they stay in the pool of the Stop List. The *Cancel* button of
[The loading window](#the-loading-window) does the same. Details: [Memory and the loading stop](#106-memory-and-the-loading-stop).

## 2.5 Division headers

Each column starts with a header: the keyboard icon (lights when a key is
pressed; right-click: *MIDI Learn...*, *Clear learned keyboard MIDI*), the
division's name, the swell bar (drag it; right-click to learn the swell pedal
or open *Swell settings...*), the *Tremulant* button (registration, like a
stop; right-click: *Tremulant settings...*) and *Settings*, which opens the
Divisions dialog. A right-click on the header's background lists the swell,
tremulant and wind settings of the division. Drag the dividers between the
columns to share out the width. The mouse wheel scrolls the workspace, except
over the sliders of an open voicing row.

## 2.6 Two program windows, screens, small screens

*Console > Program windows* opens a second window; *Shown in window* in a
division's section moves the division there. *Interface size* is the
program's own scale over the system's display scale, estimated at the first
start for Full HD, 4K and Retina screens alike. Every dialog scrolls when the
window is smaller than its content.

---

# 3. Stops and their voicing

## 3.1 Stop buttons

A stop is a button on its division's grid: click to engage (orange), click
again to disengage. The small corner button ("+") opens its **voicing row**,
"-" folds it.

A button carrying a Roman numeral in brackets ("Cornet [III]") is a **stop
folder**: a stop with other stops indented under it; its button switches
them all. Open its voicing and the children appear under it, each with its
own button and voicing row (a child can be switched alone to hear it by
itself). The numeral can be hidden per stop (*Rank count*).

A caption ending in "(Ext.)" or "(Trsm.)" is a **transmission**: the pipes
of another stop played here, possibly transposed - "(Ext.)" in the stop's own
division (a 16' or 4' extension of an 8' stop), "(Trsm.)" in another one.
[Transmissions and extensions](#34-transmissions-and-extensions).

## 3.2 Arranging the grid

Press a stop button for two seconds, release - or right-click the empty
workspace and choose *Enter arrange mode*: every stop and transmission can
be dragged to another cell or into another division. A click on empty
workspace ends the mode.

- Dragging a stop into another division moves it there with its voicing and
  transmissions. A name is unique within a division: a stop whose name the
  other division has already becomes "Principal 8 (2)" there, its cell and
  its transmissions' cells go with it.
- Dragging with the **right mouse button**, or with **Shift** held, makes a
  **copy**: a new transmission where the copy lands (in the stop's own
  division: an extension).
- The red x in a button's corner removes the stop (its transmissions go with
  it, a warning lists them) or the transmission. A stop folder with children
  is not removed; take them out first.
- **Making a stop folder:** let the dragged item rest over another stop's
  button until a dim orange bar appears under that button, then pull it a
  little *down* onto the bar - it lights up, and the drop indents the item
  under that stop. Pull back up and the drop is an ordinary move again; a
  quick drop never indents. A copy (right button or Shift) dropped on the lit
  bar becomes an extension or transmission inside the folder. **Taking a stop
  out:** open the folder's voicing and drag the child from there onto a cell
  of the grid.
- A **free cell** (dashed) offers on a click: a new empty stop folder, a new
  stop from the ready-made stops of the loaded folders that no division uses
  yet, and the division's pooled stops and transmissions.

The same can be done in the table of the
Stop List ([The table](#42-the-table)).

## 3.3 The voicing row

Open it with the corner button. The first line: transposition, perspectives,
velocity layers, pitch information, alias and switches; the second line the
sliders; under them two folding rows of per-key sliders.

### Transposition

The stop's transposition against its sample files on their own numbers (the
file "036" on key 36): every semitone from -24 to +24, each as a pure
interval with the footage it gives - "0 -> 8' (as the original)", "-12 ->
16'", "+7 -> 5 1/3'", "+3 -> 6 2/3'" (octaves bold). The button's footage
follows it, and so do the stop's transmissions. A change reloads the stop's
samples at once. The program sets it by itself where it can: from a
Hauptwerk definition, or from the samples' pitch against the footage in the
folder's name ("quintaton 16" whose files sound 8' on their numbers gets
-12). A whole organ standing a semitone off is a tuning, not a transposition,
and stays at 0. The same setting is the Stop List column "Transp."; clearing
that cell lets the program find it again.

### Alias

The caption of the button. *Automatic* shows the folder name; type any text
to show that instead (an alias without a footage gets the stop's footage
appended). The alias is only a caption: saved states, combinations and MIDI
bindings use the stop's real name.

### One-shot

The stop plays each sample once from start to end whenever the key is
released (chimes, harp, celesta); nothing is cut or faded by the key going
up. The switch starts as the *One-shot stops* patterns say and overrides
them for this stop once touched.

### Rank count

On stop folders: whether the button carries the Roman numeral of the ranks
that sound with it.

### Perspective boxes

One on/off box per microphone perspective of the stop (their volumes are the
routes' volumes in the Divisions dialog).

### Velocity layer boxes

A tick box per layer and one for the plain files: [Velocity layers](#35-velocity-layers).

### Stop volume

-24 to +12 dB, the level of the whole stop.

### Attack Gain and Attack Time

The start of every note raised or lowered by Attack Gain (-6 to +6 dB) for
the first half of Attack Time (0-500 ms, default 150), fading back over the
second half: more chiff or a softer speaking, the held tone untouched.

### Random pitch error

0-40 cents (default 4): every pipe gets its own small random detune at every
note, as real pipes never speak twice alike. The division's *Random tuning
error of pipes* must be on.

### Undulation (beats/s at a1)

For celestes: the stop beats this many times per second against an in-tune
rank at a1 (negative = flat). A rank recorded sharp or flat throughout is
recognised as a celeste and starts with its recorded undulation. The line
reads "needs 440 Hz" until the organ is retuned to a1 = 440 Hz
([Temperaments](#63-temperaments)).

### Formants (filled pipes)

0-100 % (default 30): how far the body of the sound follows the pitch shift
of the pipes this stop borrows from neighbours - [Filled pipes](#7-filled-pipes).

### Orig. Pitch / temperament

Information: the recorded pitch (a1 = ... Hz), the footage read from the
samples (the sounding footage of a mixture or mutation in brackets, and the
breaks) and the temperament the recording was found to be in. How the
footage is measured: [How the footage of a stop is measured](#102-how-the-footage-of-a-stop-is-measured).

### Volume per key and Treble per key

Two folding rows of per-key sliders (-9 to +9 dB; the treble is a high shelf
above *Treble per key: shelf start frequency*). Under every note name a tick
box switches the pipe on or off: an unticked pipe is silent and is never
filled from a neighbour - a cornet keeps its compass. *Reset to 0 dB* beside
each header; dragging with the right mouse button paints one value across a
row. The sliders light while the key sounds; a **green thumb** marks a filled
pipe (hover it for its source), an **orange note name** a sample without
loop points, a **grey slider** no pipe at all.

## 3.4 Transmissions and extensions

A transmission is an extra button that plays the pipes of another stop, on
any division (its own included). Made in the Stop List (the *Transmissions*
cell) or on the grid (a right-button or Shift drag in arrange mode). Its
voicing row has its own **Transposition** (-24 to +24, a pure interval each:
-12 makes a 16' extension of an 8' stop, +12 a 4' one; the caption follows),
*Taken from* (the source), *Volume vs. parent stop* (-10 to +10 dB), its own
per-key sliders and pipe boxes, and *Formants* for the pipes only it asks for.
The shifted pipe is retuned so that the new footage is a pure interval of the
played key; the temperament follows the played key.

A pipe sounds once however many keys hold it (the stop, a transmission, a
coupler): the loudest route sets its volume, and it keeps the tuning of the
key that started it until it stops. Keys beyond the rank's compass (the
bottom octave of a 16' extension) play **filled pipes**: [Filled pipes](#7-filled-pipes).

## 3.5 Velocity layers

An organ key never plays softer when struck gently. A stop **with velocity
layers** plays the layer the velocity falls into - a gentle stroke selects
the recording of a gentle stroke. A stop **without layers** softens the
attack of a gentle stroke in volume and colour over its first tens of
milliseconds (the *Key velocity* settings of [Advanced global audio](#83-advanced-global-audio)), the
held tone untouched. *Ignore MIDI velocity (organ keys)* in the division's
settings - on by default - sends every key at full velocity.

A file is a layer when its name ends in "_NNN" (`060-C4_070.wav` plays up to
velocity 70; above it the plain file or the next layer). The voicing row shows
a tick box per layer and one for the plain files ("_128"). An unticked layer
sends its velocities to the next layer up or the plain files; with nothing
ticked the plain files play (*Velocity layers: an unticked _128 never plays*
makes such a key silent instead). Some organs keep the original recordings as
"_070" and fine-tuned versions as the plain files: untick "_070" and only the
fine-tuned ones sound. The keyboard drawing of the Divisions dialog shows the
velocity of the last key.

## 3.6 Stop-Merging

*Stop-Merging* (top bar, hidden by default) renders the engaged stops and
transmissions, with their whole voicing, into one new stop per perspective: a
folder named after the stops (`StopMerged_Principal_8_Octava_4`) inside the
loaded folder tree, with proper WAV files, loop points and release folders. A
transmission merged alone turns a pitch-shifted extension into files.

---

# 4. The Stop List

*Stop List* in the top bar opens the **Stop List dialog**: one table for the
whole organ, one row per stop with its division, name, alias, transposition
and transmissions, and one column per perspective holding the sample folder
that plays that perspective of the stop. Everything the grid does with stops
can be done here as well, and this is where the organ is built from folders.

## 4.1 Folder trees and the pool

- **Load folder...** loads a folder tree (the samples of an organ, or part of
  them) or the RAR archive(s) they came in, and asks which division its stops
  go to. A tree is loaded once and its folders can serve any division. A tree
  loaded into an organ that has rows already is first tried as a further
  perspective of the loaded stops: folders named like the rows' folders under
  a new prefix, with the same tuning, join their rows as the new perspective
  ([The tuning gate of a perspective join](#105-the-tuning-gate-of-a-perspective-join)); the rest become new rows.
  When a tree of the organ is missing on this computer, the menu lists it as
  **NOT FOUND on this computer: replace "..." with a folder on this
  computer...** ([Sample folders not found](#sample-folders-not-found)).
  A folder holding the sample files themselves is loaded as a tree of that
  one stop.
- **Unload folder...** removes one tree from the whole organ. The rows that
  had folders of that tree only leave the table with it, buttons and all -
  the one case a row goes by itself; a row with a folder of another tree
  stays. A tree not on this computer is marked NOT FOUND here, so a package
  you do not own can be dropped. **Unload all folder trees** empties the
  organ. **Replace "..." with another folder...** takes a tree from another
  place (any folder of the set will do).
- The table starts out as the automatic rules built the divisions from the
  folder names and the perspective prefixes, and can be corrected wherever
  the rules could not pair the folders.
- The info line at the bottom counts the folders found, used, free and
  missing.

## 4.2 The table

- **Order:** the rows open in the order of the grid (row by row, left to
  right). Drag a stop name up or down to change it; *Apply* asks whether the
  grid should follow.
- **Division:** drag a row into another division's block, or choose it in
  the row's combo box. The stop moves with its folders.
- **Stop folders:** drag a name a little to the **right** and the row is
  indented under the row above: it becomes that stop's child and sounds with
  it - that is a stop folder ("Cornet [III]"). Drag it **left** to take it
  out again. One level only. (On the grid: [Arranging the grid](#32-arranging-the-grid).)
- **Folder cells:** click a cell to choose one of the free folders of all
  loaded trees (a folder serves one cell only); the x beside a cell frees its
  folder; a cell dragged onto another takes or swaps. The path shows in a
  tip; an orange cell is a folder not on this computer.
- **Transmissions:** the cell shows where the stop's transmissions go; click
  it to add one into any division or to remove one. They act at once, no
  Apply needed. *Show Trsm.* lists every transmission as a grey row of the
  division it plays in.
- **Add stop...** adds an empty stop folder or one of the ready-made stops no
  division uses yet. The x at a row's start removes the stop (its folders
  become free). **Remove empty stops** removes every row without folders and
  children.
- **Aliases from folders** gives every stop without an alias the bare name of
  its sample folder (no parents, no perspective prefix, a capital, the
  footage with its foot mark).
- **Name** is the stop's identity (voicing, combinations, MIDI); derived from
  the folders, a typed name is kept until cleared. **Alias** is only what is
  shown. **Transp.**: [Transposition](#transposition) - empty means found by the program.
- **Swap divisions...** exchanges the stop lists of two divisions;
  **Reset division...** forgets one division's table.

## 4.3 Perspectives

Every perspective is a column of the table and an audio route of every
division. **Add perspective...** adds a column (a known name or a new one)
and writes its line into *Perspective prefixes*; the x after the name in the
column header removes the column. **To rename a perspective, double-click
its name in the column header** and type the new one: the column, the
patterns line, the routes and the saved voicing follow after *Apply and
reload*. "default" is the route of folders without a perspective prefix.

**The order of the perspectives** - the columns here, the boxes of every
voicing row, the audio tabs of the Divisions dialog - is the order of the
lines in *Perspective prefixes*, "default" first. **Drag a column header
left or right** to change it: the lines take the new order, saved with the
configuration, and the voicing boxes and the audio tabs follow at once;
nothing is reloaded.

## 4.4 Ignored and one-shot patterns

*Ignored Patterns...* and *One-shot Patterns...* (top bar, or the
Configuration Editor's [Folder name patterns](#85-folder-name-patterns-one-per-line)) are lists of folder-name
parts. A folder matching an ignored pattern is not loaded; when the list is
changed with an organ loaded, the next *Apply and reload* takes the folders
that match out of their rows (the log names them), so a rank you decided to
skip disappears from the grid. The lists are saved with the configuration,
and your own additions are kept on this computer (`Patterns.json`) through
every load.

## 4.5 Apply and reload

*Apply and reload* (grey while the table equals what is loaded) saves the
table and loads the changed divisions. Stops whose folders are loaded keep
their samples and copies; only new folders are read, only missing copies
built; folders left out of the table are freed. Moving rows between
divisions or into a stop folder reloads nothing. A stop with transmissions
cannot be removed or renamed without a warning. Closing the window with
changes not applied asks: *Apply and reload*, *Discard* or *Cancel*. What
the window shows while it loads: [The loading window](#the-loading-window).

---

# 5. Divisions and audio

*Settings* in a division header opens the Divisions dialog at that division.
Its settings are the same as the division's section of the Configuration
Editor ([Division sections](#86-division-sections-pedal-manual-i--manual-vii)); the dialog adds the keyboard drawing and the
audio tabs.

## 5.1 Keyboard, MIDI and compass

- *MIDI keyboard device* and *MIDI channel* ("Shared with ..." appears when
  another division listens to the same). A keyboard plugged in while the
  program runs is opened at once.
- *Ignore MIDI velocity (organ keys)* (the dialog says *Disable MIDI
  velocity*): on by default - every key is a full stroke ([Velocity layers](#35-velocity-layers)).
- *Bass octave layout* for historic bass octaves; *Compass*: the lowest and
  highest key (defaults: pedal 36 to 67, manuals 36 to 96; keys outside are
  silent on every stop of the division); *MIDI transposition*: the keyboard
  plays this many semitones away.
- The keyboard drawing shows the last key with its velocity.

## 5.2 Swell, tremulant, wind

### The swell

The swell bar in the division header shows the shutters: drag it, or move the
learned swell pedal. Right-click it: *Start MIDI Learn* (move the pedal through
its whole range, then *Finish MIDI Learn*), *Clear MIDI assignment*, *Swell
settings...*. Its settings: [Swell: maximum attenuation when closed](#swell-maximum-attenuation-when-closed) and [Swell: shutter inertia (settling time)](#swell-shutter-inertia-settling-time) per division (the Divisions dialog, the
division's section of the Editor, or the right-click menu), and [Swell: harmonic filter](#swell-harmonic-filter) for every
division (the box also dulls the tone). The shutters follow the pedal with the
inertia set, so a sudden stroke arrives smoothly.

### The tremulant

The *Tremulant* button in the division header is registration like a stop: it
sits in combinations and crescendo steps and is MIDI-learnable (right-click:
*Learn*, *Clear MIDI assignment*, *Tremulant settings...*). Its settings per
division: [Tremulant rate](#tremulant-rate), [Tremulant depth](#tremulant-depth) (the pitch modulation) and [Tremulant phase drift between ranks](#tremulant-phase-drift-between-ranks) (0 = every stop dips at once,
a large value lets a full registration shimmer); for every division: [Tremulant: volume change per cent of pitch](#tremulant-volume-change-per-cent-of-pitch) (how
much the level follows the pitch), [Tremulant: harmonic filter](#tremulant-harmonic-filter) and [Tremulant: harmonic filter phase](#tremulant-harmonic-filter-phase) (the tone breathes with the
level). The strength wanders between 60 and 100 % of the depth from cycle to
cycle, and the children of a stop folder are spread in phase like the ranks of
a mixture, so nothing pulses mechanically.

### The wind model

The wind has no button: it sags under a heavy registration and many keys, and
every key stroke shakes it a little. Right-click the division header's
background for *Wind settings...* (or the Divisions dialog). Per division: [Wind: depth (maximum pitch sag)](#wind-depth-maximum-pitch-sag)
(0 switches it off for the division), [Wind: oscillation frequency](#wind-oscillation-frequency), [Wind: number of oscillations](#wind-number-of-oscillations), [Wind: oscillation strength](#wind-oscillation-strength), [Wind: sag settling lag](#wind-sag-settling-lag) and [Wind: also affects divisions](#wind-also-affects-divisions) (the other
divisions fed from the same wind get half of this one's sag); for every
division: [Wind: harmonic filter](#wind-harmonic-filter), [Wind: volume change per cent of sag](#wind-volume-change-per-cent-of-sag), the shares of one stop, one pipe and two pipes in the full
sag, and [Wind: maximum wind model effect](#wind-maximum-wind-model-effect) (0 % switches the whole model off).

## 5.3 The audio tabs

Every perspective of the division has an audio tab, plus one **auxiliary**
tab. Every tab is one **audio route**: the pipes of that perspective rendered
to that device and those channels. Each tab, top to bottom:

- **Perspective volume** (-31 dB switches the perspective off) and **Channel
  separation** (at 100 % every pipe sounds only on its own channel pair, at
  0 % all channels carry the same sum).
- **IR reverb**: *Load IR* (WAV, AIFF or FLAC; mono, stereo or true stereo
  with four channels; the chooser starts where an IR was chosen last),
  *Clear* (the reverb is gone at once), the IR caption (hover for its details,
  right-click for the IR settings: high and low cut, pre-delay, wet volume,
  decay, fade-out), *Reverb dry/wet*, *Set effects for all ...*.
- **Output channels**: tick any channels of the device, in any number. The
  pipes are spread over them by key number, so a chord spreads across the
  speakers like pipes across a windchest (a mono sample to one channel in
  turn, a stereo sample to a pair; an odd number of channels lets the pairs
  circulate). The tick box **Set these channels for all divisions (this
  perspective)** above the list copies these channels to the same perspective
  of every division; a later change of one route's channels unticks it, like
  the effects box.
- **The device box**: type, device, sample rate, buffer size; *Driver
  Control Panel*, *Reopen device*, and **Set for all perspectives and
  divisions**, which copies the device, rate and buffer size to every route
  (on the auxiliary tab: to the auxiliary routes of all divisions).
- **A device that is not there** (switched off, unplugged, taken by another
  program): the route keeps it and keeps its channels - the line under the
  device box says *NOT AVAILABLE now* and the channel list shows the stored
  channels greyed. The program tries to open it every 2 seconds, also while
  this dialog is open, and the moment it answers every route set to it plays
  again. A device missing at start is reported once, in a box, after a 6 s
  wait. A device that opened but gives no audio is reopened once by itself;
  when that does not help, the DSP cell reads *no audio* and the Messages
  name it. ASIO lists every installed driver whether its hardware is on or not;
  only the attempt to open it tells.

**The buffer size decides more than anything else.** 512 samples at 48 kHz
(10.7 ms, unnoticeable on an organ) renders tens of thousands of voices; 64
samples breaks at a few hundred. 512 is the default and the size to measure
with. **One rate for every device**: the samples are loaded at 48 kHz, so
give every device 48 kHz and the same buffer size; a device at another rate
gets its pipes resampled on the way, at a small cost and a step below the
quality of the resampling on load.

## 5.4 The auxiliary route

One more route per division: a mix of everything the division plays, every
pipe rendered once more, with its own device, channels, volume, separation
and IR. For monitoring (headphones, a control room), or to give a dry set a
second room: the auxiliary route on the rear speakers with a different IR is
a "Rear" perspective the set never had. It halves the reachable polyphony
while it has a device; leave it at *none* otherwise - then it costs nothing.

## 5.5 Sound cards on Windows, Linux and macOS

LibreOrgan Sampler plays through **several sound cards at the same time**, every
route on the device you choose, and can mix kinds of driver in one organ.

### Audio on Windows

- **ASIO**: the professional driver of an interface - the lowest latency, the
  most polyphony. One driver is opened once and shared by every route on it;
  a second interface with its own driver is opened beside it. Its buffer size
  is set in the driver's control panel. Use it for every route when you can.
- **Windows Audio (WASAPI)**: any Windows sound device, the on-board chip
  included. More latency, less polyphony per route.
- **Aggregated WASAPI**: several Windows devices joined into one with all
  their channels; every device is a stereo pair in the channel list and the
  ticked channels are the devices the route opens. Keep its buffer at 480
  samples or more (the Windows audio engine feeds 10 ms packets).

Mixing ASIO and WASAPI routes in one organ works, at a price: less polyphony
(the time is split between threads) and different latencies per division.
The highest polyphony: one ASIO driver, all routes on it.

### Audio on Linux

Every current distribution runs PipeWire, which reduces every card to a
stereo pair for the desktop. An organ needs every output, so:

1. **Set the interface to the "Pro Audio" profile** (KDE: System Settings >
   Sound, the card's Profile box; GNOME: Settings > Sound > Output >
   Configuration; any desktop: `pavucontrol`, tab Configuration). The card
   then appears under its own name with every channel; use the on-board chip
   for the desktop.
2. **Install PipeWire's JACK library**: `sudo apt install pipewire-jack
   pipewire-bin pavucontrol` (Ubuntu, Debian, Mint), `sudo dnf install
   pipewire-jack-audio-connection-kit pipewire-utils pavucontrol` (Fedora),
   `sudo pacman -S pipewire-jack pavucontrol` (Arch), `sudo zypper install
   pipewire-libjack-0_3 pipewire-tools pavucontrol` (openSUSE). Log out and
   in again.
3. **Realtime permission for your user**: `ulimit -r` must say 95 or more
   and `groups` must list `audio`. If not:
   ```
   sudo usermod -aG audio $USER
   sudo sh -c 'printf "@audio - rtprio 95\n@audio - memlock unlimited\n" > /etc/security/limits.d/audio.conf'
   ```
   then log out and in again. Without it the desktop pushes the audio thread
   aside and the sound breaks long before the processor is busy. The activity
   log says whether the thread was raised to realtime.

The two drivers: **JACK** (use this one - it is PipeWire itself; every card
in the Pro Audio profile appears with all its channels, the program connects
its outputs itself, the buffer size and rate chosen in any JACK route are
handed to PipeWire for every program) and **ALSA** (the plain driver; the
desktop's stereo output or a card nothing else holds; *List unusable
devices* in the tab lists everything regardless). A card the desktop uses
as its system output is never released to ALSA: pick another one as the
system output first.

### Audio on macOS

One driver, CoreAudio. Every audio device of the system is listed; several
cards are joined into one with *Audio MIDI Setup* (Utilities), which the
program then plays through like any other device. The audio threads are
given time-constraint priority by themselves. Two builds: Apple silicon
(macOS 11 or newer) and Intel (macOS 10.11 or newer; an Intel Mac of 2010 on
macOS 10.13 is one of the test machines).

## 5.6 Audio devices not ready at start

Devices that cannot be opened at start-up (unplugged, late - an RME
registers a few seconds after a rate switch) are waited for six seconds;
devices still absent are named once, their routes stay silent until the
device appears and then reopen by themselves.

## 5.7 Getting the most polyphony

Every sounding pipe is one **voice** per audio route of its division: three
perspectives render every pipe three times, the auxiliary route once more.
So the polyphony a computer reaches depends on the organ - the routes, the
IRs on them, the drivers and buffer sizes, the samples - which is why
*Maximum polyphony* is saved with the configuration and measured with the
organ loaded (*Measure the maximum polyphony*). What decides:

- **The buffer size**: 512 samples allows twice the work of 256; an organ
  does not need a small buffer.
- **The auxiliary route** halves the polyphony while it has a device.
- **IR reverb** costs on every route that has one, whether pipes sound or
  not (a silent route switches it off); true stereo IRs cost twice, long IRs
  more (*Decay*). Dry/wet 0 % or *Clear* removes the cost.
- **The driver layout**: all routes on one ASIO driver (or one Windows Audio
  device) share the whole pool of render threads. Every open device has a
  driver thread of its own, so an organ spread over several devices can
  measure more than on one - at the price of different latencies.
- **Compress samples in RAM** and **Full kernel from** on an old processor
  cost render time; the report names what was in force.

The *DSP* value is the whole callback of the busiest device as a share of
its period; several hundred per cent means the audio thread was kept waiting
by something else on the computer. What the measurement sees and what it
cannot: [The polyphony measurement](#108-the-polyphony-measurement).

---

# 6. Playing

## 6.1 Couplers

*Couplers...* (top bar): one column per division (the keyboard played) with
every coupler it can have - every division at unison, 16' and 4' (the
division itself included: "I/I 16'" is an octave coupler), the melody and
bass couplers and **Unison Off**. Ticked couplers appear on the division's
coupler rail in the main window, shown like stops ("II/I 16'" = division II
pulled over keyboard I at 16'); they are registration (combinations,
crescendo, MIDI). **Unison Off** silences the division's own pitch; its
couplers still work. A **melody coupler** couples only the highest key held,
a **bass coupler** only the lowest (*Melody/bass couplers: new extreme key
only*, *Melody couplers: legato grace time*).

## 6.2 Combinations and the crescendo

[The combination row](#22-the-combination-row) and [The crescendo](#23-the-crescendo).

## 6.3 Temperaments

*Temperaments* (top bar): thirty historical temperaments and your own ("+"
shows the cent deviations per pitch class; a custom one can be edited).

- **The samples' base temperament** is the temperament the organ was
  recorded in. The program measures it from every rank's recorded pitches
  and chooses it by itself when two thirds of the ranks agree ("Base
  temperament set from the samples" in the activity log). When they
  disagree, or the set has no pitch data, **double-click** the temperament
  (or *Samples' base temperament* in the Configuration Editor). Every
  stop's voicing row reports what its own ranks were found to be in.
- **The target temperament** is the temperament the organ plays in: a
  **single click** (or *Target temperament*). Every pipe is retuned by the
  difference; choosing the recorded temperament itself plays the samples as
  recorded. Every temperament is MIDI-learnable.
- **Two organs merged**, each recorded in its own temperament: nothing to do.
  The retune works rank by rank - every rank's own recorded deviations are
  taken out and the target's put in - so both organs play the target
  temperament in tune with each other. The base temperament shown is the one
  that prevails among the ranks; a rank whose recording could not be
  measured takes that one.

**Set diapason a1 = 440 Hz (all stops)** retunes every pipe from its recorded
pitch to a1 = 440 Hz, so
that the stops of different organs (recorded at 435, 440, 442, 465 Hz ...)
agree, and celeste beats and the global *Pitch* mean what they say; the
merge switches it on. A large retune is applied to the mixed output of every
route with a proper resampler, so the pipes keep their recorded speed
([Retune buses and the full kernel](#109-retune-buses-and-the-full-kernel)).

## 6.4 MIDI learn

Everything a console can play is learned by a right-click and *Learn MIDI
...*, then a move of the control: stops, transmissions, couplers, tremulants,
pistons and stepping buttons, the crescendo, the swell bars, every global
slider, every top-bar button, the keyboards themselves (the keyboard icon in
the header). The bindings are part of the configuration.

## 6.5 Recording

*Record* in the top bar records the stereo mix to a WAV file; click again to
stop (a short fade ends it). The file dialog starts in *Recordings folder*
(Configuration Editor > Console; `LibreRecordings` beside the LibreOrgans
folder by default).

---

# 7. Filled pipes

## 7.1 What a filled pipe is

Every rank is completed to the full keyboard: pipes it does not have are
played from neighbouring real pipes, transposed - the extra octave of an
extension, missing top or bottom keys, gaps inside the compass. Only the
pipes a stop can be asked to play are filled (its division's compass and
every transmission's shifted compass). A filled pipe has everything a real
pipe has (voicing slider, detune, wind, tremulant, releases) and costs
nothing while silent. *Missing pipes: neighbours borrowed as a group* (12 by
default) makes every missing pipe the octave transposition of a real one, so
the rank keeps its chromatic variety; in a sparse rank the neighbouring
filled pipes take turns between the real pipes within reach. A pipe switched
off in the voicing row is never filled.

Percussion and bell-like stops (Chimes, Glockenspiel, Celesta, Harp) have
inharmonic partials: a pipe of theirs transposed sounds plainly wrong, and
so does the stop once retuned. Leave such a stop at transposition 0, keep its
tuning as recorded, and switch off the keys it does not have instead of
filling them.

## 7.2 The transposition methods

*Filled pipes: transposition method* (Configuration Editor > Advanced global
audio > Filled pipes). The default *Pitch shift + time stretch* suits most
stops; compare on the lowest keys of a reed extension.

- **Resampling**: the sample played faster or slower, like a tape. Exact,
  nothing to build - but an octave down lasts twice as long and the timbre
  moves fully with the pitch. (An upward resampled pipe loses 0.1 dB of
  level and treble per semitone to sound like a pipe of its pitch.)
- **Pitch shift**: a copy built in the background - resampled with the best
  filter, the formants moved back by the stop's *Formants* value, loops on
  whole periods, loudness matched, the release's decay laid out to the
  source's duration. The attack still speaks at the resampled speed.
- **Pitch shift + time stretch**: the same copy, the attack (downwards) and
  the release stretched back towards the source's duration. Large shifts
  downwards can fray the attack: lower the attack stretch or use the plain
  pitch shift for such a stop.
- **Phase vocoder**: keeps every length and loop point; excellent within an
  octave, but it wobbles slightly upwards, loses body two octaves away and
  often breaks the loops (*Filled pipes: loop crossfade* hides the seam).

Until a pipe's copy is ready it plays by resampling; the stop buttons show
the build (a dark track filling orange). The settings of the methods are
their own options in [Filled pipes](#7-filled-pipes) of the reference; the per-stop
*Formants* is in the voicing row.

## 7.3 The cache

*Filled pipes: keep the generated copies on disk (cache)* (on by default)
writes every copy to a file, so the next load of the same stops with the
same settings reads them instead of building them: seconds instead of
minutes. One file per rank and tree; a file built with other settings is
replaced. *Empty the cache*, *Remove them* (the files of stops not loaded
now) and *Choose...* (another folder, the files moved there) are in the same
section.

---

# 8. Configuration Editor reference

*Configuration Editor* (top bar) holds every setting in one scrolling dialog:
Engine, Global sound, Advanced global audio (with its Filled pipes
subsection), Console (with Top bar), Folder name patterns, and one section
per division. Collapsed sections open with a click on their title; the
folding is remembered. Every change applies and saves at once; double-click
a slider for its default. The "?" beside an option opens this manual at the
option's chapter. At the top: *About & licences...* (this manual, the
messages, the licences), *Report a problem...*, *Show settings.json*,
*Refresh values* (reads every control again, after a MIDI-learned change for
example) and, on Linux, *Install menu entry and icon*.

## 8.1 Engine

Settings of this computer: a portable configuration carries none of them.

### This computer

The processor, its physical cores and logical CPUs, the RAM. Information.

### Render threads

How many threads render pipes for ASIO and Windows Audio routes, the driver's
thread included. Default and best: *All physical cores* - on a two-core
laptop as well. Hyper-threads render a tutti slower and with higher peaks
(measured on 16 cores: never use the "logical" choices); fewer threads leave
room for other programs. A hybrid processor (performance and efficiency
cores) offers *Performance cores only*. The workers run at audio priority on
every system ([The render pool and the threads](#1011-the-render-pool-and-the-threads)).

### Interpolation kernel

The code that renders every pipe. *Automatic* takes the best this processor
runs: AVX2 (current x86-64), SSE2 (older x86-64 - on a Core 2 Duo Mac 2500
voices against 1700 scalar), NEON (Apple silicon, Linux arm64). Pick one to
compare; the change takes effect at once and the measurement report names
the kernel it ran with.

### Currently active

The threads rendering now and the kernel in use. Information.

### Full kernel from (sounding pitch)

Pipes whose sounding fundamental lies at or above this frequency play through
the 8-tap sinc kernel for their whole length. The plain linear kernel leaves
an image of a retuned high tone *below* its fundamental (a pipe at 9 kHz
retuned by 20 cents gets a foreign tone 560 Hz under it); the sinc puts it
below -44 dB. Default 3000 Hz on a CPU with AVX2 (a few per cent of extra
work), 20000 Hz (off) without AVX2, where the sinc doubles the cost of every
voice it touches (a Core 2 Duo Mac: 3900 voices off, 780 on). 0 gives it to
every pipe. Logarithmic slider, effect for the next notes.

### Render threads for Aggregated WASAPI

Windows. The threads of a route on the Aggregated WASAPI device. Every such
route is a stream with its own thread, so ten routes render on ten threads
already: the default 1 is right for that. Raise it when one or two routes
carry the whole organ. A route never gets more than cores / aggregated
routes lanes.

### Sample storage in RAM

*32-bit float* keeps the samples as read; *16-bit integer* halves the RAM
with a little hiss; *16-bit integer, releases at half the rate* stores the
releases - a large part of a set - at half the rate (they lose what lies
above 12 kHz; a reed whose release is bright above 11 kHz keeps the full
rate). Effect at the next load; the load box asks it before every load and
saves it with the organ. What each choice costs, measured: "RAM: the storage
options measured".

### Compress samples in RAM (lossless)

Off by default. The samples are packed in RAM like FLAC packs a file: every
frame comes back exactly and a set takes a third less RAM; float storage is
packed as 24-bit integers (a 32-bit float file is rounded to 24 bits - half
a step at most). The price: every voice unpacks what it plays before every
block, so about half the polyphony goes (a big organ: two thirds), and the
load takes about 2.5 times as long. Use it when an organ does not fit the
memory otherwise.

### Sample rate on loading

*Resample everything to 48 kHz* (default): every file converted once with a
high-quality filter. *36 kHz* / *32 kHz*: what lies above 17 or 15 kHz goes -
pipes carry next to nothing there, and hearing past fifty rarely reaches 17
kHz - and the RAM drops by a quarter or a third at the same cost per voice.
*Keep each file's own rate* loads faster and plays by interpolation.

### IR processing

How the IR reverbs are convolved. *Full*: every route with its impulse
response as it is - heavy on memory traffic; an old laptop cannot render
pipes beside six long true-stereo IRs. *Light*: the convolution at a quarter
of the rate, the IR trimmed where it has fallen 60 dB, a true stereo IR
reduced to its direct channels - a fraction of the work, the same room.
*Automatic* (default) measures every reverb when it is built and chooses
Light when the reverbs would take more than 18 % of a render thread (35 %
for a single route) or fall behind while playing. Routes with the same device,
channels, IR and settings share one convolution ("(shared)" in the caption).

### IR reverbs now

What the reverbs cost now. Information.

### Maximum polyphony

The voices that may sound at once (64 to 500 000, default 30 000); the oldest
are faded out beyond it. Set it below what the computer renders, so that an
overload fades old voices rather than crackles. It belongs to the organ
(routes, IRs, drivers, buffers all change it) and is saved with the
configuration; *Measure the maximum polyphony* finds it. By hand: full
registration of a wet set, many quick staccato notes over many keys, watch
*DSP* and *PEAK*, note the peak when the sound breaks, enter a little less.

### Measurement limit of DSP power

How much of its period the longest callback of the busiest device may take
before the measurement counts a level as broken (60 to 100 %, default 85).
85 leaves headroom for what the computer does besides; 100 counts a level
as broken only when the driver really called late - the limit it finds breaks
up on the first callback that runs a little longer.

### Measure the maximum polyphony

*Measure...* runs the procedure above with the loaded organ and its audio
routes as they are: the sound turned down to -52 dB, every stop engaged, keys
struck on every division faster and wider until the voices stop rising, the
target raised in steps and held four seconds each, the first breakup
narrowed down by keys, rate and stops. *Maximum polyphony* is set to the
most that stayed clean, the registration restored, the level returns within
seconds. The report says where the time went (the render cores, the pool,
the kernel, the cost per voice) and what to change; it never forecasts other
buffer sizes. One to two minutes; *Cancel* restores everything. Save the
configuration afterwards; measure again after a change of routes, IRs,
buffer or samples. Details: [The polyphony measurement](#108-the-polyphony-measurement).

## 8.2 Global sound

Every slider has a checkbox that shows it on the main panel too; all are
MIDI-learnable there.

### Master volume

The whole organ, -60 to +12 dB (default -18). Lower it when a full
registration clips.

### Global pitch

-200 to +200 cents (default 0) on top of the temperament, for playing with
instruments tuned elsewhere than a1 = 440 Hz.

### Sustain fade-out on key release

How long the sounding pipe fades when its key goes up and the release takes
over (default 110 ms). Set for the sounding c1; pipes below it get 1.25 times
as much per octave (138 ms on the small c, 172 on the C of an 8' rank, 215
on a 16' C, 269 on a 32' C), because a big pipe cannot stop as fast.

### Release sample fade-in

How long the release sample fades in (default 110 ms), scaled by pitch like
the fade-out. The two together shape the join between the note and its
release.

### Release sample volume

All release samples, -20 to +10 dB (default 0); "-inf" silences them.

### Release cutting

Releases are faded out over this share of their length and end there (5 to
100 %, default 100). Shorter releases free polyphony sooner and dry a wet set.

### Release played from

Every release is played from this many milliseconds into its file (0 to 300,
default 0), skipping a start that begins too aggressively. A release shorter
than the setting is not played at all - the key ends with the sustain fade
alone. Heard at once.

### Attack minimum time

The shortest time a pipe sounds once started (0 to 500 ms, default 80): even
the briefest touch lets the chiff be heard.

### Attack minimum delay

A small pause between the key press and the pipe's start (0 to 500 ms,
default 2), varied at random so that a chord's pipes do not all speak in the
same instant; fast repetitions stay crisp.

## 8.3 Advanced global audio

### Swell: harmonic filter

A closed swell box also takes the brilliance: a high shelf above about 3 kHz
following the shutters, this much loss with the box closed (0 to 12 dB,
default 8). The three harmonic filters (swell, wind, tremulant) add up,
capped at 12 dB; at 0 dB they cost nothing.

### Wind: harmonic filter

The same shelf following the wind sag: this much loss at the full sag
(default 2 dB).

### Wind: volume change per cent of sag

How much quieter a pipe gets per cent the sag lowers its pitch (0 to 0.6 dB,
default 0.2).

### Wind: one stop's share of the full sag

The sag a single engaged 8' stop produces, relative to many stops (2 to
100 %, default 20); smaller footages count less.

### Wind: one pipe's share of the full sag

The sag with one key held, relative to many (default 50 %).

### Wind: two pipes' share of the full sag

The sag with two keys held (default 50 %); from three keys on the rest closes
gradually.

### Wind: maximum wind model effect

The strength of the whole wind model, 0 to 100 % (default 100); every key
stroke shakes the wind with a random 40 to 100 % of it. 0 switches it off.

### Tremulant: harmonic filter

The shelf following the tremulant: this much loss at the bottom of its dip
(default 4 dB).

### Tremulant: harmonic filter phase

How far the tone oscillation lags the volume oscillation (0 to 360 degrees,
default 90).

### Tremulant: volume change per cent of pitch

The level modulation per cent of pitch modulation (default 0.187 dB).

### Key velocity: full strike from velocity

From this velocity up a stroke is a full one (2 to 127, default 80); the two
settings below ramp from velocity 1 to here.

### Key velocity: fade-in at the lowest velocity

A gentle stroke on a stop without velocity layers starts with this fade-in at
the lowest velocity (default 100 ms), shrinking to nothing at the full
strike. [Velocity layers](#35-velocity-layers).

### Key velocity: treble cut on the attack at the lowest velocity

The chiff of a gently struck key is blunted by this much high-frequency loss
at velocity 1 (default 9 dB), less for firmer strokes.

### Key velocity: treble cut opens over

How long that cut takes to open after the key goes down (5 to 1000 ms,
default 180).

### Velocity layers: an unticked _128 never plays

Off by default: with nothing ticked in a stop's velocity boxes the plain
files play. On: such a key is silent.

### Treble per key: shelf start frequency

The *Treble per key* sliders act above this frequency (120 to 5000 Hz,
default 1800); a pipe whose fundamental lies higher starts there.

### Celeste beats: top of compass vs bottom

Celeste stops beat this many times faster at the top of the compass than at
the bottom (1 to 20, default 12); a1 beats as the voicing says.

### Retune pipes off by more than

A pipe further off its own rank's tuning than this is pulled to its key even
with *Equal at 440 Hz* off (default 50 cents, 0 = never); the key's tooltip
says so. Travels with the organ.

### Replace pipes off by more than

How far a pipe's recorded pitch may lie from its key before the pipe counts
as a faulty sample (default 200 cents). Beyond it the pipe is switched off
and filled from its neighbours; its thumb turns green and the tooltip says
so. Travels with the organ.

### Filled pipes: resampling quality

How a neighbour's sample is played at another pitch while no copy exists:
*Linear*, *Cubic interpolation* (images 20 dB lower), *Cubic + anti-alias
filter* (default; also removes what folds back on upward shifts, a low-passed
copy per such pipe). Real pipes are never touched.

### Filled pipes: transposition method

*Resampling*, *Pitch shift*, *Pitch shift + time stretch* (default) or *Phase
vocoder*: [The transposition methods](#72-the-transposition-methods). Switching rebuilds the copies (from the
cache when on); the sliders that do not apply are greyed out.

### Missing pipes: neighbours borrowed as a group

This many adjacent real pipes are taken as a group and transposed by the
group's width for the missing ones (1 to 12, default 12: every missing pipe
an octave transposition of a real one). Changing it refills every stop. In a
sparse rank neighbouring filled pipes take turns between the real pipes within
that width.

### Filled pipes: release length (pitch shift)

Pitch shift method: the decay of a filled pipe's release as a share of its
source's duration (25 to 200 %, default 100) - the room does not grow with
the pipe.

### Filled pipes: time stretch of the attack

Pitch shift + time stretch: how far the attack is stretched back from the
resampled length towards the source's (0 to 100 %, default 50, logarithmic).
Higher values fray the attack; lower it for a stop whose speaking turns rough.

### Filled pipes: time stretch of the release

The same for the release, both directions (default 55). Phase vocoder: a
release shifted upwards is stretched towards the resampled length.

### Filled pipes: release played from

A filled pipe's release copy is played from this many milliseconds in (0 to
300, default 140), skipping the start where the methods leave their worst.
Heard at once.

### Filled pipes: loop crossfade

The splice of the copies' loops as a share of each loop (0 to 100 %, default
40): a longer blend hides a seam that is not quite periodic. Raise it when a
held extension pipe bumps at every repeat.

### Filled pipes: release level matched to the source

On: a release copy plays at the level relation the source's release has to
its sustain. Off (default): at its own level.

### Filled pipes: loudness kept when pitch shifting

On (default): every shifted copy is brought to the loudness of its source.

### Filled pipes: keep the generated copies on disk (cache)

On by default: the copies are written to files and read back at the next
load - seconds instead of minutes. [The cache](#73-the-cache).

### Filled pipes cache: Empty the cache

The size of the cache and its folder, with *Empty the cache*: every file
deleted, built again when needed.

### Filled pipes cache: files of stops not loaded now

*Remove them* deletes the files of ranks not loaded now; the loaded organ's
stay.

### Filled pipes cache folder

*Choose...* picks another folder (a big drive); the files are moved there.

## 8.4 Console

### Interface size

Every window, button and text in per cent of the design size (50 to 300 %),
estimated from the screen at the first start; applied when the slider is
released.

### Divisions

How many manuals the console has (1 to 7; the pedal is always there).
Reducing it forgets the removed divisions.

### Combination pistons

The number of pistons in the combination row (0 to 10).

### Program windows

*One window* or *Two windows* ("Two program windows, screens, small
screens").

### Melody/bass couplers: new extreme key only

On (default): a melody (bass) coupler couples a key only when it becomes the
new highest (lowest) key held. Off: the coupled note always follows the
extreme key.

### Melody couplers: legato grace time

On: the melody coupler waits a moment when the highest key changes in legato,
so a brief overlap does not make the solo stop jump.

### Load the last configuration at startup

Off by default. On: the configuration used last time opens at the start.

### Glow: a pressed button and a dragged stop glow for a moment

Off by default: the golden halo of the instruction video on every pressed
button and dragged stop.

### Output discontinuity detector: count the clicks in the program's own output

On by default. The program counts every jump between two blocks of its own
output that the sound does not explain - after the rendering, at the end of
every route, on the way to the driver - logs it with the route and stage and
shows "!N" in orange in the DSP cell. A few microseconds per period. "The
discontinuity detector".

### LibreOrgans folder

Where configurations live (`Documents\LibreOrgan\LibreOrgans` by
default): every import and save goes there, Load starts there. *Choose...*
picks another; the path stands after the button.

### Recordings folder

Where *Record* writes its files (`LibreRecordings` beside the LibreOrgans
folder by default). *Choose...* picks another.

### Sample and IR locations

The places the program looks for sample trees and impulse responses - every
folder and archive you ever chose, counted after the button. *Add folder...*
adds one.

### Sample and IR locations: the file

*Show* opens `Locations.txt` (one path per line; edit it with any text
editor, the program reads it at the next load).

### Sample and IR locations: forget the folders

*Forget* drops the folder lines; the archives chosen stay.

### Samples' base temperament

The temperament the samples were recorded in - found by itself when two
thirds of the ranks agree; set it by hand when they disagree or the set has
no pitch data. [Temperaments](#63-temperaments).

### Target temperament

The temperament the organ plays in. [Temperaments](#63-temperaments).

### Window size

The size of the main window, saved by itself. Information.

### Top bar

One switch per top-bar button: PANIC, Record, Stop List, Couplers,
Temperaments, Hide/Show all voicing, Perspective Patterns, Ignored Patterns,
One-shot Patterns, Stop-Merging, Load Voicing, Save Voicing, Load
Configuration, Save Configuration, Shut down PC, Unpack archive, GLOBAL RESET
([Top bar buttons](#21-top-bar-buttons)). A hidden button still fires from its learned MIDI
control. The button beside each switch does the same as the one in the top
bar.

## 8.5 Folder name patterns (one per line)

The three lists travel with the organ and belong to this computer as well:
every pattern you add is kept in `Patterns.json` beside the settings and
merged into every organ you load; a pattern you remove is removed there too.
The "?" of the pattern dialogs opens these chapters.

### One-shot stops (folder contains)

Folders whose name matches play once without looping (chiff, percussion,
bells). Defaults: Chiff, Chimes, Tubular, Celesta, Harp, Glockenspiel. A
pattern without a star matches anywhere, "\*x" the end, "x\*" the start.

### Ignored folders (folder contains)

Folders whose name matches are not loaded. Defaults: "trem", "\*_tr",
"output". Changed with an organ loaded, the next *Apply and reload* takes the
matching folders out of their rows.

### Perspective prefixes

One line per perspective: its name first, then its folder prefixes,
separated by semicolons ("Direct;01-dirMk2H-;dir-"). The name is what the
Stop List column and the audio route are called; the prefixes are cut off the
folder names. The order of the lines is the order of the perspectives
everywhere (the Stop List columns, the voicing boxes, the audio tabs):
reorder them here, or drag a column header in the Stop List. Prefixes found
in a set are added by themselves; a perspective renamed in the Stop List
(double-click the column header) renames the line.

## 8.6 Division sections (Pedal, Manual I ... Manual VII)

One collapsed section per division, the same settings as the Divisions
dialog.

### Stops and sample folders

The folder trees of this division and *Stop building...*, which opens the
Stop List.

### Shown in window

With two program windows: which one shows this division.

### MIDI keyboard device

The MIDI input that plays this division; several divisions may listen to the
same device.

### MIDI channel

1 to 16 on that device.

### Ignore MIDI velocity (organ keys)

On by default: every key is a full stroke - no velocity fade-in or treble
cut, always the top velocity layer. Off for a stop with velocity layers you
want to play by touch.

### Bass octave layout

*Standard*, *Short octave* (E sounds C, F# D, G# E), *C D D# ...*, *C D E
...* for historic bass octaves.

### MIDI transposition

The keyboard plays this many semitones away from what it sends (-24 to +24).

### Swell pedal controller

The controller learned for the swell (right-click the swell bar). Information.

### Swell: maximum attenuation when closed

How much quieter the division is with the box closed (0 to 60 dB, default
13).

### Swell: shutter inertia (settling time)

How slowly the shutters follow the pedal (50 to 2000 ms, default 300).

### Division volume

-60 to +12 dB (default 0), for balancing the divisions.

### Audio output channels (default perspective)

The channels of the device the default perspective plays on, as numbers
separated by commas (the same as the tick boxes of the audio tab). "The audio
tabs".

### Perspective output channels

The same for a named perspective (Direct, Diffuse, Rear, auxiliary).

### Perspective volume

-31 to +6 dB (default 0); -31 switches the perspective off and saves its
voices.

### Perspective channel separation

100 % (default): every pipe only on its own channel pair; 0 %: all channels
the same sum.

### Random tuning error of pipes

(The Divisions dialog says *Allow Random Tuning Error*.) On: the pipes get
the small random detune set per stop (*Random pitch error*).

### Pipe coupling (detuned pipes pull together)

(The Divisions dialog says *Allow Pipe Coupling*.) On: the pipes of the same
pitch sounding together pull each other into tune a little, as on one chest.

### Tremulant rate

2 to 11 Hz (default 4.4).

### Tremulant depth

The pitch modulation, 0 to 50 cents (default 6); the strength wanders between
60 and 100 % of it.

### Tremulant phase drift between ranks

0 to 360 degrees (default 260): with 0 every stop dips at once, with a large
value a full registration shimmers.

### Wind: depth (maximum pitch sag)

How far the pitch sags with the wind at its lowest (0 to 50 cents, default
8); 0 switches the wind model off for this division.

### Wind: oscillation frequency

The bounce of the bellows after a key stroke (2 to 10 Hz, default 7.5).

### Wind: number of oscillations

How many die away after a stroke (0 to 20, default 7).

### Wind: oscillation strength

The impulse a key stroke gives the wind (0 to 100 %, default 40).

### Wind: sag settling lag

How long the pressure takes to settle after stops or keys change (100 to 400
ms, default 255).

### Wind: also affects divisions

The other divisions fed from this wind: they receive half of this division's
sag.

---

# 9. Messages, the activity log, reporting a problem, licences

## 9.1 Messages

What the program has to tell after an action - a tree found, folders
replaced, what a merge did, what the load left out - no longer interrupts
you with a box: a strip in the lower left corner of the main window shows
"Messages (N new)" with the last title for ten seconds, and the texts are
collected under **Messages** in the *About & licences...* window (the entry
above the manual; a click on the strip opens it). Questions, warnings and the
polyphony report still come as boxes.

## 9.2 The activity log

Everything the program does at a coarse level - devices opened, divisions
loaded, builds of the copies, the memory and disk figures of every load - is
noted in `CrashLogs/activity.log` in the settings folder ("Where the files
are"); a crash writes `crash-<date>.txt` beside it, and a window that
answers nothing for eight seconds records where it was waiting.
`settings.json` is the state of the program, `settings-<date>.json` the
snapshot of each of the last seven days; if the program crashes at every
start, move `settings.json` away and it starts clean.

## 9.3 Report a problem

*Report a problem...* at the top of the Configuration Editor asks what
happened and how to reproduce it. *Create the report and open GitHub* writes
a folder `LibreOrgan-report-<date>` into `Reports` in the settings folder
with `logs.zip` (the activity log, the crash reports, `settings.json`, the
last self-test, `system.txt` with the computer and every audio route - no
samples, no voicing) and screenshots of every open window, then opens the
GitHub form in your browser with its fields filled in and the folder beside
it: drag `logs.zip` into "Logs" and the pictures into "Screenshots", submit.
A free GitHub account is needed; the report is public. If GitHub lands
elsewhere after the sign-in, *Open the GitHub form* in the program's message
opens the form again, and `Report on GitHub.html` in the folder does the same
days later.

## 9.4 Free software, licences, platforms

LibreOrgan Sampler is free software under the GNU GPL v3; the source is on GitHub
and every release is built there. *About & licences...* shows the program's
licence and those of the software it contains: JUCE (AGPLv3), the Steinberg
ASIO SDK (GPLv3 option; ASIO is a trademark of Steinberg Media Technologies
GmbH), Signalsmith Stretch and Signalsmith Linear (MIT), libarchive and
unarr for the archives. Builds: Windows 10/11 64-bit; macOS 11+ (Apple
silicon) and 10.11+ (Intel); Linux x86-64 with glibc 2.38 or newer.

---

# 10. Technical details

The chapters above link here. Nothing in this part is needed to play.

## 10.1 The smpl chunk and the file names

The smpl chunk is a small block inside a WAV file that samplers use; most
audio editors can write it. LibreOrgan Sampler reads: *MIDI unity note*
(dwMIDIUnityNote, the recorded pitch rounded DOWN to the nearest MIDI note),
*MIDI pitch fraction* (dwMIDIPitchFraction, the offset above it as a fraction
of a semitone, 0 to 2^32-1 for 0 to 100 cents - always positive: an A4 four
cents flat is coded as G#4 plus 96 cents), and *the loops* (the loop list's
start and end in frames; several loops allowed, the type ignored). Unity note
and fraction give the exact recorded pitch, from which the tuning, the
temperament, the footage and the celeste detection follow. Files without the
chunk play as they are: no retuning, no temperament, the footage from the
folder name only.

File names: the MIDI key number first, optionally a note name (036.wav,
036-C.wav, 059-B3.wav). Several files of one key are round-robin variants
played in turn, whatever else the name carries (`036_1.wav`, `036-C_bis.wav`);
a two- or three-digit suffix after an underscore at the very end is a
velocity layer (`036-C_bis_070.wav`: a variant inside layer 070). The release
folders: `rel500` (also `rel_0500`, `rel-500`, `release 500ms`) for keys held
up to 500 ms, `rel1500` up to 1.5 s, plain `rel` (= `rel99999`) for the rest,
files named like the attacks.

Folder names: the stop's label is built from the folder path; a folder nested
inside a folder of the same name (perspective prefix, footage and an L/R mark
aside: "02-direct-HW_Bl.Flute" inside "HW_Bl.Flute2") adds nothing. A single
L or R right after the footage or at the end ("AB_quintaton16L" +
"AB_quintaton16R") marks the two halves of one stop; a rank count ("Mixtur4x",
"Zimbel3") is no such mark. A mutation written without spaces ("Nasard 223",
"Terz 135") is read as 2 2/3', 1 3/5'; the organ builders' shorthand "3",
"6", "12", "1 1/2" means 2 2/3', 5 1/3', 10 2/3', 1 1/3'. A folder named after
its sibling or parent with "tr" or "trem" appended is the tremmed recording
and is skipped by every scan; a folder a Stop List row names explicitly is
loaded whatever its name.

The transposition a folder gets by itself: the mode of (recorded pitch - file
number) over the pipes against the footage in the folder's name, when three
or more samples agree by 70 % and the difference is two semitones or more.
Stops with no footage in the name stay at 0.

## 10.2 How the footage of a stop is measured

The footage shown as "Orig. Pitch" is the first harmonic the lowest pipes
really carry, measured on the audio (65 536 frames from 0.3 s in, the first
harmonic within 34 dB of the strongest). A recording loses the fundamental of
the deepest pipes - a 32' C at 16 Hz has it 50 dB under the second harmonic -
so when the lowest pipe sounds below 45 Hz the pipes 6, 7, 9, 10 and 12
semitones above are asked whether they have a clear fundamental; two of them
suffice and the stop keeps its 32'. A mixture has no fundamental on any pipe
and keeps its first sounding harmonic, hidden breaks included; the higher
pipes never decide the harmonic number.

## 10.3 Where the program looks for folders

A configuration names its trees relative to its library root
(`OrganInstallationPackages/003135/pipe/HW` with the root's name) and an IR by
its path inside the library or by its full path. At a load each is looked for
at the path it had; then under every sample location:
`<location>/<root name>/<path>`, `<location>/<path>`, the last one to three
parts of the old path, and one level around every location (the folder above
and its first subfolders); then beside the configuration file (also the
folder the file was chosen from when a copy is loaded); then, on a computer
with no location yet, by the root's name at the top of every fixed drive,
under every first-level folder and in the Hauptwerk layout
(`<folder>\HauptwerkSampleSetsAndComponents\<root>`). A tree never takes
another's place by its last folder name alone - only by the same path below
the root. What is found is taken silently and the root noted as a location;
the list is `Locations.txt` (the folders, and the archives you chose - only
those are read, never another archive lying beside them).

A clicked folder ([Sample folders not found](#sample-folders-not-found)) is searched from itself and up
to four ancestors for the longest tail of the missing path that exists under
it; the tree found must hold at least one of the stop folders the rows
expect, else the box refuses it; the other missing trees are looked for at
the same place under the new prefix, then by their tails.

The relocation of a whole library: keep every tree of an organ under one
root; the root's parent is what the program remembers. The LibreOrgans
folder can be moved as a whole: nothing in it points at its own place.

## 10.4 Hauptwerk import rules

LibreOrgan Sampler does not load a Hauptwerk organ definition: nothing of it runs
here, and the program would have no use for it - the organ is built from the
sample folders, exactly as from a tree loaded by hand. A definition is read for
two things only, where the sample folders are and how the stops are arranged in
the divisions. From that LibreOrgan Sampler deduces the microphone perspectives and
the relative volumes of the stops, the divisions and the pipes: that is its
own computation, beyond the definition - Hauptwerk knows neither perspectives
nor relative volumes, it has absolute pipe levels and "stop-ranks" only. The
pipes, their pitch, footage, tuning and loops come from the folders. What
follows is how that arrangement is read.

What the definition gives: the keyboards in their order (the pedal first)
with their key ranges - a division of ours is a keyboard, and every Hauptwerk
division a keyboard's keys reach through the definition's switches joins it
(ventil-conditioned ones included; a division reached from several keyboards
goes to the one with the most keys; a floating division no keyboard reaches
without a coupler is placed stop by stop where the name says); the stops
with their sample folders (stop > StopRank rows > ranks > pipe folders); the
perspectives from the rank names ("Front A (direct): ..." -> Direct, the
folder prefixes learned into the patterns); the compass of every stop from
its own rows (keys outside switched off in the voicing, no fills); the
per-pipe levels, split in three - the median of a division's perspective
becomes the route volume, the median of the rest the stop volume, the
remainder the per-key voicing; the tremulant rate (the middle of its range);
the couplers the definition has that exist in our catalog, and only those
shown; the general crescendo (one stage per Hauptwerk combination "Cresc...",
spread over our steps when more than 60); the stops' places on the jamb page
with the most drawknobs (columns and rows copied).

Ownership: a rank several stops play belongs to the stop whose name it
carries, else the stop playing it straight over the whole division, else the
one with the most keys. A stop with ranks of its own is loaded from them; a
stop with none becomes a transmission of the owner it borrows the most keys
from, with that shift ("P. Tuba 4" = the Tuba 16' shifted). A folder is never
loaded twice. A stop with several ranks in one perspective (a left and a
right component) becomes a stop folder with one child per rank. Ranks
without release samples make a one-shot stop. Tremmed ranks and the noise
ranks (blower, action) are left out; combinations, wind, displays are ignored.
Encrypted sets (.hbw/.hbx) refuse the import.

Names: a row is named after its stop folder as it stands on disk, with the
definition's perspective prefix cut off ("Re_Principal32" becomes
"Principal32" when "Re_" is the Rear prefix; a division mark like "Ped_"
stays); the Hauptwerk stop name becomes the alias, without its footage (the
caption's footage is the sounding one). A configuration whose rows an older
import named with the prefix is renamed the same way when it loads, every
reference along (voicing, grid, combinations, crescendo, MIDI); a note in
Messages says so, and Save Configuration keeps the new names. A capital follows a division mark
("P. Trombone 16", "G.O. Trompette 8", "ped subbass" -> "Ped Subbass").
Transpositions come from the definition's pipe notes against the file names
("024" on key 36 gives -12). The configuration goes into the LibreOrgans
folder as `<organ>.json`; the folder above `OrganInstallationPackages` is
noted as a sample location. An import report with everything the plan
decided is written to `ImportReports` in the settings folder.

## 10.5 The tuning gate of a perspective join

Two folders are the same pipes in two perspectives only when their files
carry the same tuning: the smpl chunks (unity note and fraction) of the
same-named files - eight per folder, spread from the lowest pipe up - must be
equal within 0.02 cent on at least two thirds of them and on three files at
least. A foreign pipe never matches a fraction bit for bit; a producer's
retouch of a few headers does not refuse a real perspective. A folder with a
degenerate smpl (no chunk, few distinct values, every fraction 0) is compared
by its measured pitch instead (the four lowest files, within 2 cents). The
name rule before the gate: the whole stem after the prefix must be equal
("HW_Principal8" is not "Principal8"), the base folder's head a known
perspective prefix, and the new head on three folders at least. The log
says for every pair "Tuning gate: ... the same pipes / NOT the same pipes".

A head is a perspective only when the same stems exist under another head
(and the gate agrees) or when it carries a perspective word (dir, dif, rea,
far, near, close, dry, wet, front, ab, ortf ...); a lone head such as "HW_"
or "Ped_" is a division mark and never becomes a perspective.

## 10.6 Memory and the loading stop

Before every stop (and every 32 files inside a large one) the loader looks at
the memory and stops when any sign says it is short: less free than 512 MB or
a sixteenth of the installed memory, the program's footprint within the
system's share (1.5 GB or a fifth; 1 GB on 4 GB and less), the program
swapped out by more than 256 MB since the load began (on macOS the
compressor growing that much while less than twice the reserve is free), or
the macOS kernel reporting memory pressure. The stops loaded so far make the
instrument; a stop cut in the middle of its files is dropped whole and named
in the message; the pool keeps them. A stop whose samples the program holds
already (a reload) is adopted whatever the memory says. The copy build of
the filled pipes stops the same way. Before the load the WAV bytes of the
trees are estimated against the free memory and a warning is logged; the
load is never refused.

The "free" figure: Windows' available physical memory, Linux's MemAvailable,
macOS's free and inactive pages. The *RAM GB* cell and the figures of a
portable save are the process (Windows: the larger of working set and
private bytes; macOS: the physical footprint; Linux: resident + swapped).

## 10.7 RAM: the storage options measured

One organ, 53.9 GB of 32-bit float WAV, the whole program with the copies of
the filled pipes:

| Storage | Rate on loading | Releases | Packing in RAM | RAM |
|---|---|---|---|---|
| 32-bit float | 48 kHz | full rate | off | 83 GB |
| 32-bit float | 48 kHz | full rate | on | 29 GB |
| 16-bit | 48 kHz | full rate | off | 49 GB |
| 16-bit | 48 kHz | full rate | on | 15 GB |
| 16-bit | 48 kHz | half the rate | off | 32 GB |
| 16-bit | 48 kHz | half the rate | on | 12 GB |
| 16-bit | 32 kHz | half the rate | on | 8 GB |

A published configuration's README states the least RAM the organ takes with
every saving option (measured when saved so, else estimated from a sample of
the loaded pipes), and INDEX.md lists it. Loading takes markedly longer with
packing (about 2.5 times) and with half-rate releases (every release through
the sinc filter); a cold load runs at the disk's speed (about 500 MB/s from a
SATA SSD), a load from the page cache many times faster - never compare the
two.

## 10.8 The polyphony measurement

What it does: the level down to -52 dB, the limit out of the way, the attack
minimum time and delay off, two seconds of idle listening; every stop
engaged (every button of the grid, transmissions included); keys struck
again and again on every division over its own compass, one key twice a
second at first, more keys and a faster rate whenever the voices stop rising
(up to every key, 16 strikes a second); the target raised in steps of 2000
(a tenth above 20 000), each reached target held four seconds and judged
clean when every callback of the busiest device stayed within the
*Measurement limit of DSP power* and the driver never called late; the first
breakup narrowed down by keys, by rate and finally by the share of stops
engaged, so that the result is what the computer renders, not the last
coarse step. Below 1000 voices the result is taken as measured. When all
keys at the fastest rate produce no more voices and the sound stays clean,
the organ itself is the limit. Afterwards the level glides up at 26 dB/s
after a short hold and the new limit takes effect once the tails are gone.

The report: the CPU of the render cores (16 threads busy on a 32-thread
processor are 100 %), the lane utilisation of the pool, the kernel's
nanoseconds per voice and frame from cache and from RAM with the ceiling
that gives at this buffer, the cost of one voice per block in its parts
(bookkeeping, spans, kernel, tone filter, unpacking), the callback split into
a fixed part per block and a part per voice, and the device thread's own
profile: own render, waiting for the lanes, mixing, IR hand-over,
bookkeeping, the longest route callback of the run and the latest a lane
joined a job. On Linux the CPU governor, on Windows the power plan.

What moves the figure: the buffer size first (two cores: 1000 voices at
256, 2300-2900 at 512, 3100-3800 at 1024; sixteen cores: 49 000 / 64 000 /
74 000 - compare runs at one buffer only); the device path (Linux: measure
on the card's direct ALSA device or JACK, not on PipeWire's default output;
Windows: ASIO or one Windows Audio device for all routes); *Compress samples
in RAM* halves the figure; *Full kernel from* on a processor without AVX2
divides it by two to five; two-core laptops scatter by a third between runs
(take the median of three); Aggregated WASAPI renders every route on its own
thread (a big organ over fifteen routes measures like ASIO, the whole organ
on two routes is bound to two cores).

What it cannot see: a dropout made after the program's buffer - the
system's shared mixer with many streams on one endpoint, a USB card starved
by the system, an endpoint's "audio enhancements" boosting every quiet stream
- leaves the run clean and the sound not. Such a case shows in the activity
log as "Audio callback anomaly: ... no callbacks" for a route, or not at all;
the cure is the device layout (one driver for everything, ASIO where there is
one, no unused routes left open on the same endpoint), never the limit. The
DSP cell can read lower than the limit when the run ends: it shows a fading
peak, the judge takes the single longest callback.

## 10.9 Retune buses and the full kernel

A pipe is retuned by its own recorded deviation (its rank's smpl pitches),
the temperament of the played key, the random detune, wind and tremulant,
and the transposition of its transmission - every pipe and every rank for
itself, which is why two perspectives recorded with a real tuning difference
are each pulled to the same pitch with *Set diapason a1 = 440 Hz* on, and
keep their difference with it off. The common part of a large retune (a1 = 440 Hz on an
organ at 409 Hz, a big global pitch) is not played by speeding every pipe up:
the route's voices of that retune are rendered into a bus and the bus is
resampled once with a proper sinc filter into the route's output, so the
pipes keep their recorded speed and the images of the cheap kernel stay
away. Up to three buses per route, joined within 6 cents. The cents that
stay in the pipes themselves (temperament, detune) play through the linear
kernel, or through the 8-tap sinc with phase interpolation above *Full
kernel from (sounding pitch)*; the sinc also serves the first 1.3 s of a
half-rate release and the loop splices of the high pipes. 32 or 36 kHz
storage is no retune: its images stay.

## 10.10 The discontinuity detector

A jump is counted when the step from the last sample of a block to the first
of the next is more than twelve times the mean step of this and the previous
block, above -74 dBFS, and the previous block was not silent (a chord
starting on a silent route is not a click). Counted per route after the pool
render (stage 0, the sharpest: with IR reverb on the route the wet masks a
small dry step at stage 1), after separation, reverb and clip (stage 1), and
per device callback on the driver's buffer, on the route's own channels only.
The log line names the route, the stage and the lanes that rendered without a
scratch buffer; the counters reset after a render gap, a device start or a
mute. It costs a few microseconds per period; a 15-route organ at idle pays
about 7 % of a 512-sample period for it.

## 10.11 The render pool and the threads

The routes of one device are one stream: the driver's callback calls every
route in turn, each route's voices become one job of the render pool, the
device thread and the workers take chunks of it (lanes), the device thread
merges their scratch buffers and mixes. The IR convolution of a route runs as
a pool task posted by the last route of its group and joined by the first
route of the next block, so it takes render time but never piles up on the
audio thread. The workers wake ahead of every period (the cadence is known)
and the device thread takes no more voices at the tail of a job when the
lanes still at work would finish them sooner. The workers and the control
thread (MIDI, attacks and releases, finished voices) run at audio priority:
Windows THREAD_PRIORITY_HIGHEST with MMCSS, Linux SCHED_FIFO when allowed
(the log says "Render worker: SCHED_FIFO" or "REFUSED" with what to allow),
macOS a time-constraint thread; a real-time thread waiting for another never
yields the core (under SCHED_FIFO that starves the window thread). On one or
two cores the device thread yields while it waits and the lanes take small
chunks. The kernels: AVX2 (eight frames a step), SSE2 and NEON (four), scalar;
the self-test (`--kerneltest`) checks every kernel against the scalar code and
prints its speed.

## 10.12 RAR archives: how they are read

The program keeps its own index of every archive you choose (a walk of the
headers, cached under `ArchiveIndex` in the settings folder, keyed by the
first volume's size and date; a minute for 18 000 headers on a hard disk,
seconds on an SSD): the files with their volumes and pieces. A stored file
is read straight from the volumes; a packed one is unpacked from its own
bytes alone, so the loader reads files in parallel (100-120 MB/s per thread
packed, 1 GB/s stored); a solid archive is read in sequence. RAR 4 and RAR 5,
multi-volume sets (.partNN, .r00), recovery records, unicode names;
encrypted archives are refused. Archives packed with PPMd or old filters go
through a second reader (unarr) after the first fails, logged once. A
configuration never names an archive: every path is a disk path, and a folder
that is also on the disk wins over the archive.

## 10.13 Files written by the program

In the settings folder ([Where the files are](#14-where-the-files-are)): `settings.json` (this
computer: devices, MIDI, the editor's machine settings, the last
configuration) with `settings-<date>.json` snapshots; `Locations.txt`;
`Patterns.json`; `ArchiveIndex/`; `FilledPipesCache/` (or the folder chosen);
`CrashLogs/activity.log` and `crash-<date>.txt`; `Reports/`;
`ImportReports/`; `Shutdown rescue configuration.json` after *Shut down PC*
without a file. In the LibreOrgans folder: `<organ>.json` and its
`.json.bak`, `Publish/` with the exports, `../LibreRecordings` by default.
A configuration file holds only what the instrument uses now - the divisions
in use, the routes that exist and their settings; a route no stop plays
through has no tab and no device. Paths are stored with "/"; every path under
the library root is relative, only a tree outside it absolute. The voicing is
saved automatically and with the configuration; *Save Voicing* / *Load
Voicing* exchange it as a file.
