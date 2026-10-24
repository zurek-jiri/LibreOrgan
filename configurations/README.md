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
   **Download** link: a zip with the `.json` inside. Unpack it and put the
   `.json` **beside the samples** - in the folder that contains their top
   folders, as the README shows. A configuration stores its
   sample paths relative to that folder (the "library root"), so the samples
   are found by themselves.
3. In OrganSampler: *Load Configuration* in the top bar, choose the file.

If the samples lie elsewhere, the configuration loads all the same with the
trees that are found; every division that misses a tree shows an orange
notice "Sample folder not found: <folder>" under its header. Open the Stop
List, *Load folder...* and choose the folder of that name wherever it is on
your computer: it takes the missing tree's place and the stops keep their
samples. Then *Save Configuration* to keep the paths.

## Publishing a configuration

One folder per organ, named after the organ, holding the two files (the
robot described below makes the folder by itself):

```
configurations/
└── stralsund-jakobi-wegscheider/
    ├── README.md
    └── Stralsund-Jakobi-Wegscheider.zip      <- the configuration .json, zipped
```

Save the configuration with **Portable configuration (to share with
others)** ticked in the Save Configuration box: it then carries the organ
only - no MIDI devices, no audio devices or channels, nothing of your
computer (the manual chapter of the same name explains it). OrganSampler
writes two more files beside it (since 0.8.96): `<name>.zip`, the
configuration zipped for the upload, and `<name>.README.md`, the README -
the perspectives, the folders the configuration expects and where to put the
file, the routes, the version and the Stop List in the order of your grid.
The save asks you once for what the README cannot know - the organ, the
producer and product of the samples, the link to the page where the sample
folders are bought or downloaded, your name - and writes them in; nothing is
filled in by hand. Then send the two files; no knowledge of Git is needed, a
free GitHub account is:

1. Open the form
   https://github.com/zurek-jiri/OrganSampler/issues/new?template=organ_configuration.yml
2. Drag `<name>.zip` and `<name>.README.md` into its *Files* field, tick the
   publication box, press **Submit**.

A robot takes the two files from the issue, checks them and proposes them
as a pull request; it answers in the issue, and when something is missing it
says what (edit the issue and it tries again). The maintainer merges the
pull request, and the organ gets its own folder under `configurations/` and
a line of [INDEX.md](INDEX.md) - automatically. Those who know Git may open
the pull request themselves: `<name>.zip` and `<name>.README.md` flat in
`configurations/`. Published configurations are under the repository's
licence (GPLv3).

## The published configurations

The list is [INDEX.md](INDEX.md): one line per organ with its divisions,
Stop List rows, RAM (as measured with the organ loaded, and the least it can
take with every saving option of the load box - 16-bit, 32 kHz on loading,
releases at half the rate, packed samples) and perspectives, written
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
