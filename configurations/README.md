# Configurations, Organs

A configuration is a complete organ built in OrganSampler - the folder trees
of the samples, the Stop List, the voicing, the audio routes, couplers,
combinations and crescendo - saved with *Save Configuration* as one `.json`
file. With a published configuration a player who owns the same sample
folders loads the whole organ in one step instead of building it.

**No samples are ever published here.** A configuration only names the
sample folders it needs; the samples themselves are bought or downloaded
elsewhere - from [Sonus Paradisi](https://www.sonusparadisi.cz/) above all
(their [Free stuff](https://www.sonusparadisi.cz/en/free-stuff.html) page has
free instruments to begin with) - and remain the property of their authors.
Impulse responses belong to the sample folders as well. A folder here holds
exactly two files, the configuration zipped (one `.json` inside - a zip is
downloaded from GitHub with one click, a bare `.json` would open as text) and
its `README.md`; a check in the repository (`Configurations check` workflow)
refuses anything else.

## Using a published configuration

1. Get the sample folders the configuration was made for (its README says
   which and where from).
2. Find the organ in [INDEX.md](INDEX.md), open its README and click its
   **Download** link: a zip with the `.json` inside.
3. In OrganSampler: *Load Configuration* in the top bar, choose the zip as
   it is (0.8.195; nothing to unpack) - the load box copies the
   configuration into your **configurations folder** (Configuration Editor >
   *Configurations folder*; `Documents\OrganSampler\Configurations` by
   default - since 0.8.190 every configuration lives there, away from the
   samples). The sample folders are found in your *sample locations* (the folders
   above your sample libraries, which the program remembers by itself and
   lists in the Configuration Editor); the README shows the folder tree
   expected under one of them.

If a tree is not found, the configuration loads all the same with the trees
that are; every division that misses a tree shows an orange notice "Sample
folder not found: <folder>" under its header. Click it and choose the folder
where that tree is now: it takes the missing tree's place, the stops keep
their samples, and the folder above it is remembered as a sample location.
Then *Save Configuration* to keep the paths.

## Publishing a configuration

One folder per organ, named after the organ as you name it at the save
(lower case, other characters as `-`), holding the two files (the robot
described below makes the folder by itself):

```
configurations/
└── stralsund-st-jakobi-kr-wegscheider/
    ├── README.md
    └── Stralsund_Jakobi_Wegscheider.zip      <- the configuration .json, zipped
```

Save the configuration with **Portable configuration (to share with
others)** ticked in the Save Configuration box: it then carries the organ
only - no MIDI devices, no audio devices or channels, nothing of your
computer (the manual chapter of the same name explains it). OrganSampler
writes it as an export into the `Publish` subfolder of your configurations
folder (0.8.194; your ordinary configuration stays untouched):
`<name>.portable.json`, `<name>.portable.zip` (the configuration zipped for
the upload) and `<name>.portable.README.md`, the README - the perspectives,
the folders the configuration expects, the routes, the RAM, the version and
the Stop List in the order of your grid. The save asks you once for what the
README cannot know - the organ, the producer of the samples, the link to the
page where the sample folders are bought or downloaded, your name - and
writes them in; nothing is filled in by hand. Then a window offers **Publish
on GitHub**: it opens the form with the title and the organ filled in and
the `Publish` folder beside it; no knowledge of Git is needed, a free GitHub
account is:

1. Press **Publish on GitHub** (or open the form yourself:
   https://github.com/zurek-jiri/OrganSampler/issues/new?template=organ_configuration.yml).
2. Drag `<name>.portable.zip` and `<name>.portable.README.md` into its
   *Files* field, tick the publication box, press **Submit**.

A robot takes the two files from the issue, checks them - the zip must hold
one `.json` that really is an organ configuration of OrganSampler, saved
portable with the organ loaded (divisions, sample folder trees, stops, the
RAM figure), and the README must be the one the program wrote for that zip
(title, trees, divisions, Stop List table) - and proposes them as a pull
request; it answers in the issue, and when something is wrong it says what
(edit the issue and it tries again). The maintainer merges the pull request,
and the organ gets its own folder under `configurations/` and a line of
[INDEX.md](INDEX.md) - automatically. Those who know Git may open the pull
request themselves: `<name>.zip` and `<name>.README.md` flat in
`configurations/`. Published configurations are under the repository's
licence (GPLv3).

**A new version of an organ** is sent the same way. The robot knows the
organ by the name given at the save (`publication` in the configuration),
whatever the file is called: after the merge the new files replace the
published folder, and a folder of another name holding the same organ goes
with it. The publisher is recorded in the README as an invisible HTML
comment (`published by @login`); a new version from another account is
proposed all the same, with that said in the pull request and the issue -
the maintainer's merge decides.

**Removing one**, without Git:

- Anybody who published an organ: the form
  https://github.com/zurek-jiri/OrganSampler/issues/new?template=organ_configuration_remove.yml
  with any part of the organ's name. The robot removes the folder when the
  issue's author is the publisher (or the maintainer), answers and closes the
  issue; when several organs match it lists them - edit the issue with more
  of the name.
- The maintainer: GitHub > *Actions* > *Remove a configuration* > *Run
  workflow* > any part of the organ's name (the folder, the README title or
  the organ as named at the save; case does not matter) > *Run workflow*. One
  match: the folder goes, INDEX.md is regenerated, both committed by the
  workflow; several: they are listed in the job and nothing happens.

The Rural Organ demo is never replaced or removed by the robots.

**Featured organs.** [INDEX.md](INDEX.md) has two tables: *Featured* and
*All the others*. The featured ones are the folder names listed in
[FEATURED.txt](FEATURED.txt) (one per line; the Rural Organ demo by
default). To put an organ there or take it out: edit `FEATURED.txt` on
GitHub (the pencil; INDEX.md follows by itself), or GitHub > *Actions* >
*Feature a configuration* > *Run workflow* > any part of the organ's name +
*feature* / *unfeature* > *Run workflow*.

## The published configurations

The list is [INDEX.md](INDEX.md): one line per organ with its divisions,
Stop List rows, RAM (the least the organ takes, with every saving option of
the load box - 16-bit, 32 kHz on loading, releases at half the rate, packed
samples - measured or estimated at the save; the full quality takes roughly
ten times as much) and perspectives, written
automatically from the configuration files. The README of each organ says how
to download its `.json`.

## The demo organ: Rural Organ

The one exception to "no samples here": **Rural Organ**, a small village
organ - three divisions, 24 stops, dry recordings with IR reverb -
packaged with its sample folders and its portable configuration so that
OrganSampler can be tried at once. See [`rural-organ/README.md`](rural-organ/README.md);
the package `RuralOrgan.rar` (about 700 MB; *Load Configuration* opens the `.rar` itself, or unpack it) is attached to the release
https://github.com/zurek-jiri/OrganSampler/releases/tag/rural-organ-0.8.
The samples are provided by Sonus Paradisi free of charge to try the program.
