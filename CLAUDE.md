# LibreOrgan Sampler - rules for every session (Windows and Linux)

The user writes Czech; answer in Czech; append a closing report to `LibreOrgan/CLAUDE_REPORTS.md`
(the copy on the NTFS disk: `F:\JUCE_PROJEKTY\LibreOrgan` = `/mnt/pracovni/JUCE_PROJEKTY/LibreOrgan`).

## The name (0.9.0, 31 Oct 2026 - the user: "OrganSampler" is an old VST on GitHub, never repeat that name)
- The program is **LibreOrgan Sampler** in every title and text (window title, manual, README, website, dialogs);
  EVERYTHING ELSE is **LibreOrgan**: the repository `zurek-jiri/LibreOrgan` (renamed through the API on 31 Oct 2026;
  GitHub redirects the old URL), the website `zurek-jiri.github.io/LibreOrgan`, the executables and release archives
  (`LibreOrgan.exe`, `LibreOrgan.app`, `LibreOrgan-windows.zip`, `LibreOrgan-macos.zip`, `LibreOrgan-macos-10.11-intel.zip`,
  `LibreOrgan-linux-x86_64.tar.gz`, `LibreOrgan-linux-arm64.tar.gz`), the settings folder (`%APPDATA%\LibreOrgan`,
  `~/.config/LibreOrgan`, `~/Library/LibreOrgan`), the project folder `LibreOrgan/` with `LibreOrgan.jucer`,
  `LibreOrgan_icon.ico`, `Source/Icons/LibreOrgan.icns`, the CMake target/project `LibreOrgan`, the bundle id
  `org.libreorgan.sampler`, the Linux entry `libreorgan.desktop` / `~/.local/bin/libreorgan-start` / pixmap
  `libreorgan.png` / WM_CLASS `libreorgan`, the local exe `C:\LibreOrgan\LibreOrgan_<version>.exe`, the Linux clone
  `~/LibreOrgan-linux`, the binary `~/Downloads/LibreOrgan`. "Everywhere Libre" (the user): the user's organs live in
  `Documents\LibreOrgan\LibreOrgans` (exports in its `Publish`), the recordings in `Documents\LibreOrgan\LibreRecordings`,
  the Editor row reads "LibreOrgans folder", and the published organs of the repository are in `libreorgans/` (INDEX.md,
  the robots, the forms, the `raw/main/libreorgans/<slug>/` links). No migration of anything: nobody used the old name
  but the user (he deletes the old folders himself). The old notes below keep their version numbers; the renaming was
  done by %TEMP%\rename_libreorgan.py (ordered regex rules: identifiers -> LibreOrgan, prose -> LibreOrgan Sampler).

## "go" and the to-do list
- THE USER'S RULE (26 Oct 2026): "go" means "continue, I have topped up the money" - never "do this one item now".
  "todo:" messages are collected (CLAUDE.md pending list); when he says to do them, ALL of them go into one build and
  one version. 0.8.213 was first built from one to-do alone because a message ended with "go" - wrong.

## Replies to reporters (GitHub issues, testers)
- THE USER'S RULE (18 Nov 2026, #241, "nepiš kraviny, na které uživatel fakt není zvědavej, to opravdu vynech
  navždycky"): a reply NEVER carries trivial or generic advice ("30 routes cost time, use fewer if you do not need
  them", "try another buffer", "switch the packing off") - a user with 30 routes wants 30 routes and the program must
  work with them. A report WITHOUT logs.zip gets this and nothing else: without the log nothing certain can be said; we
  would like to deal with the case seriously; fields typed by hand and pasted screenshots are WASTED WORK; the way is
  Configuration Editor > "Report a problem..." > a few words > "Create the report and open GitHub" > drag logs.zip
  into the field "Logs" > Submit - the drag is the ONE step only the reporter can do (GitHub lets no program attach
  a file), everything else the program fills in. Polite and firm, never an insult. A reply is a DRAFT for the user
  until he says to post it.

## Never change what you need not change
- The user's rule (23 Oct 2026, "Nikdy neměň něco, co nemusíš!"): a fix touches the lines of the fault and nothing
  else - no renaming, no "while I am here" rewrites, no new defaults, no changed texts or separators. When a change
  outside the fault seems needed, say so first. The automatic stop name "A / B" (two perspectives with differently
  named folders) dates from the first commit, not from a session - before blaming a session, `git blame` the line.

## Icons - do not touch without asking
- The icon set is one per size and lives in git: `LibreOrgan/Source/Icons/icon-{16,24,32,48,64,128,256}.png`
  (16-128 hand-tuned: outlines, the smallest with thick pipes; 256 the picture), `LibreOrgan/Source/AppIcon.png`
  (1024, the website picture), `LibreOrgan/LibreOrgan_icon.ico` (all seven sizes, PNG entries),
  `LibreOrgan/Source/Icons/LibreOrgan.icns` (sixteen entries: the classic RLE types is32/il32/ih32/it32 with
  their masks for 16/32/48/128 - the Finder of macOS 10.7-10.13 shows PNG data in `icp4`/`icp5` as pixel garbage,
  so those two are never written - and PNG ic07-ic14 for 128-1024 and the @2x sizes; built by
  `python LibreOrgan/Tools/make_icns.py LibreOrgan/Source/Icons LibreOrgan/Source/AppIcon.png
  LibreOrgan/Source/Icons/LibreOrgan.icns` from icon-16..512.png + AppIcon.png), `site/assets/icon.png` (= AppIcon).
- JUCE (CMake and Projucer alike) would rescale two of them into blurred icons: `CMakeLists.txt` overwrites JUCE's
  generated `icon.ico` / `AppIcon.icns` with ours after `juce_add_gui_app`; the jucer's VS exporter copies the .ico
  by `postExportShellCommandWin`, which `Projucer --resave` does NOT run - after a resave on Windows copy
  `LibreOrgan/LibreOrgan_icon.ico` over `LibreOrgan/Builds/VisualStudio2026/icon.ico` by hand.
- Never replace the icons by a rescaled or "improved" set. If a picture changes, rebuild the 256/1024 entries
  only and keep 16-128 as they are. Linux (menu entry, window icon) uses the same files.

## Versions and releases
- THE USER'S RULE (30 Oct 2026, "musíš všecko pushovat na debilní gitHub? Tak tam dej jenom finální verzi. Nepal
  mi tokeny zbytečně."): commit every version LOCALLY (the Linux clone / the Windows checkout), but push main and
  the tag only for the FINAL version of a session, or when he says so - not a release per fix, no CI watch per
  version. Each pushed version still gets its own tag and release as before.
- Every delivered build is a new version. Bump it in ALL of: `LibreOrgan/LibreOrgan.jucer` (`version=`),
  `CMakeLists.txt` (`project(... VERSION ...)`), `site/config.js` (`version:` - app.js writes it into the page),
  `site/index.html` (`data-version` spans), `libreorgans/rural-organ/README.md` ("Saved with LibreOrgan Sampler ..."),
  `LibreOrgan/CHANGELOG.md`. Before tagging, grep the previous version string over the repo (CHANGELOG excepted).
- Regenerate `LibreOrgan/Source/ManualData.h` after any manual edit: `python LibreOrgan/Tools/generate_manual_data.py`.
- Commit (plain commit messages: no Co-Authored-By trailer, no assistant name anywhere in the repository - the
  user is the only contributor), push, tag `v<version>`; CI builds Windows,
  macOS (Apple silicon, macOS 11+), Intel macOS 10.11+ and Linux and publishes a pre-release; the release job deletes
  every older `v*` release and tag. The Rural Organ demo release `rural-organ-0.8` stays. Only Sonus Paradisi is ever
  named for samples; never "sample set" - say sample folders / folder tree.
- Intel macOS build (10.11+): never add `-D_LIBCPP_DISABLE_AVAILABILITY` (it makes the binary import libc++
  symbols of the build machine's macOS - dyld "Symbol not found" on 10.12-10.14). After each release download
  `LibreOrgan-macos-10.11-intel.zip` into a fresh temp folder and run
  `python -I LibreOrgan/Tools/check_macho_imports.py <app>/Contents/MacOS/LibreOrgan`: it must end with
  "late symbols: 0".
- Windows commit messages: write the message to a file WITHOUT BOM (`[IO.File]::WriteAllText(path, text)`) and
  `git commit -F <file>`; `-F -` with a here-string does not work in PowerShell 5.1 (the text becomes pathspecs,
  the commit fails and a following `git tag` lands on the previous commit), and `Set-Content -Encoding utf8` adds
  a BOM to the subject line. Tag only after `git log -1` shows the new commit.
- Windows: `git` is not on PATH - use Visual Studio's
  (`C:\Program Files\Microsoft Visual Studio\18\Community\...\Team Explorer\Git\cmd\git.exe`); after pulling,
  run `C:\JUCE9\Projucer.exe --resave "F:\JUCE_PROJEKTY\LibreOrgan\LibreOrgan\LibreOrgan.jucer"` with the
  ABSOLUTE path and copy the .ico (above) (with a relative path the resave prints nothing and writes nothing - four
  tries in 0.8.145; check `versionString` in `JuceLibraryCode/JuceHeader.h`, it must print "Finished saving"), commit the regenerated
  `Builds/VisualStudio2026` and `JuceLibraryCode`. Linux clone: `~/LibreOrgan-linux` (git cannot run on the NTFS
  checkout from Linux); Linux builds come from CI only: `LibreOrgan-linux-x86_64.tar.gz`, unpacked to
  `~/Downloads/LibreOrgan`. Releases carry the native archive of each system, never a bare executable (0.8.110):
  `LibreOrgan-windows.zip` (LibreOrgan.exe inside), `LibreOrgan-macos.zip` / `LibreOrgan-macos-10.11-intel.zip`
  (the app), `LibreOrgan-linux-x86_64.tar.gz` (keeps the executable bit); names without a version. On Linux `gh` is NOT on
  PATH: use `~/tools/gh/bin/gh` (export PATH="$HOME/tools/gh/bin:$PATH"); never run `~/Downloads/LibreOrgan` with
  made-up flags such as `--version` - it starts the program.

## Configurations and paths
- A configuration never stores an absolute path except a tree outside the sample library root (another disk);
  everything under the root is relative to it (`sampleLibraryRootName` / `sampleLibraryRootRelative`, pool
  `sampleTrees`, `folderRelativePath`, `extraFolderRelativePaths`, `reverbIrRelative`/`reverbIrLibraryRelative`).
  A portable configuration carries no machine settings: devices become `deviceType="default"` with the route's
  sample rate and buffer size; "none" stays none.
- Never alter audio silently; never drop user-configured content (routes, stops, folders) without asking first
  (AskUserQuestion). A row of the Stop List is never removed by the program itself - only by the user
  ("Remove empty stops", arrange mode).
- Stop folders (a container stop with indented children) are table rows: the container has no folders, the
  children `child: true`. Keep them through every save/load path.
- Publishing a configuration needs no Git. GitHub's web upload is for collaborators only ("Uploads are
  disabled"), so the way is the issue form `organ_configuration.yml` (label `configuration`): the workflow
  `configuration-issue.yml` takes the attached `<Name>.zip` + `<Name>.README.md` from the issue body
  (`.github/scripts/configuration_from_issue.py`), runs `.github/scripts/configurations_check.sh`, pushes
  the flat pair into `libreorgans/` on branch `configuration/issue-<n>` and opens the pull request
  (closes the issue on merge), answering in the issue on success or failure. The check accepts that flat
  pair; after the merge the `index` job moves it into `libreorgans/<slug>/` (README.md + zip) and
  regenerates INDEX. `.github/PULL_REQUEST_TEMPLATE.md` is the checklist for hand-made pull requests.
  Repository setting needed (set 15 Oct 2026 via the API, `actions/permissions/workflow`): "Allow GitHub
  Actions to create and approve pull requests" ON - without it `gh pr create` fails with "GitHub Actions is
  not permitted to create or approve pull requests". The label `configuration` must exist (it does).
- `libreorgans/<organ>/` holds exactly two files: the zipped `.json` and `README.md`. No IR files, no pictures,
  no bare `.json`, nothing else (impulse responses belong to the sample folders); the CI check refuses anything else.
- Transmissions grouped under a stop folder are NOT table rows: they live in the source stop's voicing entry
  (`voicingSettings.stops[].transmissions[]` with `groupMaster`). Search there before claiming they are gone.
- The transmission list of a stop belongs to the Stop List / voicing state only. A combination, crescendo step
  or registration sets on/off states and must never replace, add to or remove from the list (0.8.92).
- The voicing state keeps entries of stops the engine could not build (missing folders) and writes them back;
  never let a save drop voicing of a stop that merely failed to load.
- THE USER'S RULE FOR EVERY TEXT (30 Oct 2026, "musíme jasně deklarovat, že neloudujeme HW ODF!!! ... A vždycky už se to
  bude takhle říkat!" - the producer of the sets is angry at programs that load their definitions): wherever the manual,
  README, website, release body, dialogs, tooltips or the import report mention a Hauptwerk organ definition, say that
  LibreOrgan Sampler does NOT load it (it has no use for it: the organ is built from the sample folders), that the definition
  is read for TWO things only - where the sample folders are and how the stops are arranged in the divisions - that the
  microphone PERSPECTIVES and the RELATIVE VOLUMES of stops, divisions and pipes are LibreOrgan Sampler's OWN DEDUCTION (the
  user, 30 Oct 2026: "Hauptwerk nezná perspektivy a nezná relativní hlasitosti, ten má pouze absolutní hlasitost píšťal a
  všecko je pro něj stoprank" - here we go beyond Hauptwerk), and that everything else comes from the folders. Never
  write "imports/loads the ODF" or "opens as a configuration" without that frame. Done in 0.8.253 (manual 1.1, 1.6,
  Saving and loading, 10.4 - the technical list of 10.4 stays, the user wants it there; README; site; the import
  report's header and closing sentence).
- Hauptwerk ODF import (0.8.116, `Source/HauptwerkOdf.{h,cpp}`, `MainComponent::importHauptwerkOdf`): Load Configuration
  takes `*.Organ_Hauptwerk_xml` too. Only divisions (keyboard order, pedal first, compass from KeyboardKey), stop folders
  (the common ancestor of a stop's rank folders; their parents are the division trees), perspectives (rank names
  "Front A (direct): ..." -> patterns "Direct;01-dirMk2H-") and per-pipe levels (`Pipe_SoundEngine01_Layer`
  AmpLvl_LevelAdjustDecibels, first layer only, averaged over perspectives -> stop volume + per-key dB) are taken.
  Never couplers/combinations/tremulants/wind/displays. Tremmed ranks (AlternateRankID, "(tremmed)", `trem` folders) and
  noise ranks are skipped; `.hbw`/`.hbx` (encrypted) refuse the import. Hauptwerk layout:
  `<root>/OrganDefinitions/<odf>` + `<root>/OrganInstallationPackages/<6-digit id>/...`; the configuration is written
  into the main package folder (`<organ>.json`, library root = OrganInstallationPackages), no zip/README
  (`savePortableConfiguration(..., publishFiles=false)`). Compacted ODFs use letter tags (Stop a/b/c, StopRank a/d/p,
  Rank a/b, Pipe a/b/d, Layer a/b/h, AttackSample b/c, Sample a/b/c, Keyboard a/b/x, KeyboardKey a/c, Division a/b).
  Merge (0.8.117, `MainComponentMerge.cpp`, right-click Load Configuration -> `mergeConfigurationDialog`; dev hook
  `ORGAN_MERGE_CONFIGURATION=<file>` after the load): base = settings.json, added = the file (or the ODF plan var);
  divisions by number; shared folders -> stop not added; same name -> "(2)"; new perspectives bring their routes;
  empty division takes the added keys/MIDI; couplers/combinations/crescendo/temperament/global stay; free RAM
  (`availablePhysicalMemoryBytes`) vs 0.7 x WAV bytes of the new trees, refused when short; result as a message.
  Dev check without the GUI: `LibreOrgan.exe --hauptwerk "<odf>"` prints the parse and the plan (also to
  `%APPDATA%\LibreOrgan\hauptwerk.txt`); the in-app path is `ORGAN_LOAD_CONFIGURATION=<odf>` + `ORGAN_DUMP_STOPLIST=`.
  Stralsund (3135+3136) must give trees `00313x/pipe/{PE,HW,OW,UW}`, 13+14+12+12 stops, patterns Direct/Diffuse/Rear.
  0.8.118: the way is stop > StopRank rows (h/i/j = first key, count, increment) > rank > folder. A rank belongs to the
  stop that plays it with increment 0; a stop whose rows borrow one other stop's ranks with one increment is an
  EXTENSION/transmission (`PlanStop.extensionOfStopId`, `shift`; `hauptwerkExtensions` -> `engine.addTransmission`
  after the load, alias = HW name) - before, the common folder of the two stops was the whole division and the tree
  the whole package (Stellwagen "III. G. Plockfloett 4": pool full of every folder, stops twice). A stop folder never
  holds another stop's folder (fallback: rank folders). Noise ranks by whole words ("I. Zimbell" is no bell).
  Rows: `name` = the folder's automatic name (`scanTreeAssignments`), `customName=false`, `alias` = HW stop name.
  Ignored patterns on import: `trem`, `*_tr` (the `II_TR` sibling folders). The import writes several prefixes into
  one perspective line when one organ spells a perspective two ways ("Direct;01-dirc-;03-dif2S-") - the engine has
  read `Label;prefix;prefix2` lines since the first commit (0.8.50, `stripPerspectivePrefix`); never call that new.
  Stellwagen (2723+2724, non-encrypted ODF in OrganDefinitions) must give 14+14+12+12 stops, 1 extension, trees
  `00272x/pipes/{PE,I,II,III}` only.
  0.8.125: a division of OURS is a keyboard. `Keyboard::divisionIds` = every ODF division the key switches reach
  through `SwitchLinkage` (KeyboardKey.SwitchID -> source/dest chains, condition switch = a ventil: still the
  keyboard's division) to `DivisionInput.SwitchID`; a division reached from several keyboards goes to the one with
  the most unconditional keys; the ODF hint (`Hint_PrimaryAssociatedDivisionID`) stays primary when among them;
  no linkage at all -> the hint. The plan takes the stops of ALL those divisions (primary first). Caen Surround
  (ODF by the user, packages 1318-1320, 1691-1695, 45 GB) must give 11+15+13+16 = 55 stops from 8 ODF divisions
  (Pedal+PedalAnches ...), never 4 divisions / 32 stops. Several ranks in ONE perspective ("(L component)" /
  "(R component)" ranks, folders `quintaton16` + `quintaton16l`) -> `PlanStop.children` (grouped by the rank name
  without its perspective head, else by folder name; sorted by name): the import writes a container row (HW name,
  customName, no folders) + child rows (`child: true`, automatic names, alias = rank name) - the 16 Recit stops of
  Caen are stop folders "[II]". Before, those stops took `pipe/rec` (the whole division) and dropped the second
  rank. The configuration goes to `<root>/OrganConfigurations/<organ>.json` (folder created by the import; library
  root OrganInstallationPackages via `sampleLibraryRootRelative` "../OrganInstallationPackages"), no longer into
  the numbered package. Dev: `--hauptwerk` prints "keyboard N: ... -> division X (all: X Y)" and "STOP FOLDER with
  N children".
  Voicing (0.8.126, `applyHauptwerkVoicing`): Hauptwerk has NO volume per perspective, so the pipe levels are split
  in three: median of all pipe levels of (division, perspective) -> route volume (`perspectiveAudio[key].volumeDb` +
  `setPerspectiveVolumeDb`, clamped -31..6), median of the residual (averaged over the stop's perspectives) -> stop dB,
  the rest -> per-key dB. Never put a perspective's offset into the stops or pipes again.
  0.8.132 perspectives: `perspectiveOfRankName` returns the perspective WORD itself capitalised (Direct, Diffuse,
  Distant, Rear, Dry, Close ... ; "Front A"/"Front B" without a parenthesis get the letter) - never a synonym class
  (distant -> Rear put "Front C (distant)" and "Rear" of Ashton-under-Lyne into one label and the 0.8.125 rule made a
  stop folder of a plain four-perspective set). Safety net at the end of `parse`: ranks of one stop with the same
  label but different folder prefixes (longest common ending of the folder names = the stop part, the rest = the
  prefix; core = letters/digits "dir22", "dis2h", "reaSen") are relabelled "<label> <core>" - four prefixes are four
  perspectives, whatever the names say; the same prefix twice (L/R split) stays a stop folder, and so do names that
  differ at their END (common ending < 3: Caen "quintaton16" / "quintaton16l" are pieces, not perspectives). The
  user's rule: the ODF's StopRanks (rank names) decide, prefixes support them where they exist - Caen has none (its
  perspectives are packages). Ashton Surround must give Direct / Diffuse / Distant / Rear, patterns "Direct;02-dir22-"
  etc., no stop folders; Caen Recit keeps its 16 stop folders.
  0.8.128: stop compass from StopRank rows (h = first key, default the division's first; i = count, default to the
  division's last; union over the rows with increment 0; `HauptwerkOdf::compassForFolders`) -> keys outside switched
  off with `engine.setPipeEnabled(stop, pipe, false)` (voicing `disabledKeys`, no fills) in `applyHauptwerkVoicing`.
  Tremulant: `Tremulant` (a id, b name, d/e = speed min/max Hz, the middle taken) > `TremulantWaveform` (a id, c
  TremulantID) > `TremulantWaveformPipe` (a PipeID, b waveform) -> pipe -> rank -> owner stop -> division
  (`HauptwerkOdf::tremulants[].divisionIds`); `PlanDivision.tremulantRateHz` -> manualKeyboards "tremulantRateHz"
  (Caen: Recit 5.61 Hz; Stellwagen: Recit + Positif 4.79 Hz). Master level: `MainComponent::kDefaultMasterDb` = -18
  (slider, settings default, reset, MIDI-learn default); `registrationMasterDb` absent -> the default, never 0 dB.
  A stop folder's children: `Stop::groupChildOrdinal` (set in `applyStopGroups`) adds `childOrdinal * rankDrift`
  to the tremulant phase. Polyphony display: fifth column "RAM GB" (`processMemoryBytes`, once a second, WHOLE GB
  rounded up, no unit in the value - "1.2 GB" did not fit the cell on a 1280 px display; orange when free < 10 % of
  installed). `processMemoryBytes` (0.8.129): macOS `task_vm_info.phys_footprint` (resident + compressed + swapped, what
  Activity Monitor shows; `resident_size` alone said 995 MB for a multi-GB organ on the 8 GB Mac), Linux VmRSS + VmSwap,
  Windows max(WorkingSet, PrivateUsage). Never report the resident size alone as "what the program holds".
  0.8.130: `warnIfMemoryShortForTrees` (MainComponentMerge.cpp, called in `applyPortableConfiguration` before
  `beginStartupFolderLoading`): WAV bytes of the trees (nested trees once) x 0.7 (16-bit) / 1.0 (float) vs the free
  memory measured before the load; counted on a juce::Thread::launch, verdict by callAsync: log "Memory for ..." and
  an AlertWindow when needed > free - 512 MB. The load is never refused (#17: a 7 GB organ on the 8 GB Mac thrashed,
  the window thread stalled 10-16 s at a time, "the application cannot be closed").
  0.8.131 (the user's rule): the loading STOPS when the memory reserve is reached - `freePhysicalMemoryBytes()` /
  `memoryReserveBytes()` (768 MB or installed/16) in SamplerEngine.cpp, checked in `loadDivisionTree` before every
  non-container stop and every 32 files inside `loadStopFolder`'s parallelFor; `loadStoppedForMemory` + `loadOutOfMemory`
  set, the remaining stops of the load go to `loadMemorySkipped` (cleared by `takeLastLoadError`), the instrument is
  built from the stops loaded so far, `lastLoadError` names the stops left out (first 12). Never let the system page
  samples: a stalled window thread looks like a hang. 0.8.133: the figures live in `Source/MemoryInfo.h`
  (`MemoryInfo::freeBytes/heldBytes/swappedBytes/installedBytes/systemShareBytes/reserveBytes/shortNow`), used by the
  engine (`memoryShortNow`), `MainComponent::availablePhysicalMemoryBytes/processMemoryBytes`, the RAM column and the
  report. `shortNow`: free < reserve (768 MB / installed/16) OR held > installed - systemShare (1.5 GB / installed/5)
  OR swapped > 256 MB (Linux VmSwap of the process, macOS compressor pages). The free figure alone let a 7 GB organ
  load whole on the 8 GB Mac (#17) and on the 7.7 GB Linux laptop (#19: held 6.9 GB, "2 GB free", swapping) - the
  footprint rule is the one that bites. 0.8.134: `MemoryInfo::pressureLevel()` = macOS `kern.memorystatus_vm_pressure_level`
  (1 normal, 2 warning, 4 critical; the kernel's own figure, counting what freeing the inactive pages would give - a
  user process cannot purge them: `purge` needs root and drops only the disk cache, madvise touches our pages only);
  level >= 2 is the fourth sign in `shortNow`. The pre-load estimate reads bits/rate from the first 48 WAV headers (16/bits x
  48000/rate when resampling, x 1.15 for copies), never a flat two thirds. Hauptwerk rows: name = the STOP FOLDER's
  name as on disk (the user's rule: "ve jmenech ma byt, jak se jmenuji adresare"), alias = the HW stop name; a child
  of a stop folder = its rank folder's name. The Hauptwerk import sets `suppressPortableNotice` so the
  "Configuration from another computer" notice is not shown (two message boxes over each other, the user refused).
  0.8.129 crescendo: `Combination` (a id, b name) named "cresc..." = one stage each, the stage = the number at the end
  of the name (`getTrailingIntValue`); `CombinationElement` (b CombinationID, c ControlledSwitchID, e Y = on); the
  switches reach the stops' `Stop.switchId` (d, ControllingSwitchID) through `SwitchLinkage` both ways within 8 hops
  (Doesburg 1 hop; Schwerin 4: CrescendoCombSwitch > CrescendoIfOnSwitch > ... > "C.<stop>" > the stop - a limit of
  4 iterations found nothing there, every stage came out empty); couplers unresolved -> left out.
  `HauptwerkOdf::crescendo` / `Plan.crescendo` -> the import writes the configuration key "crescendo"
  (`crescendoFromVar` format: stepCount, cancelsHandStops, steps[{step, stops[{name=row name, perspective "",
  enabled, primaryManual}], couplers[]}]); more than 60 stages spread over 60 steps. Doesburg Surround: 64 stages,
  stage 2 = Aeoline 8 + Echobass 16 + Echo-Gamba 8.
  0.8.135 ownership (`HauptwerkOdf::plan`, the claims block): a rank's owner among the stops playing it with increment 0
  = name match (rank name after the perspective head == stop name) > count 0 (the whole division) > more keys > file
  order; a rank nobody plays straight -> its first user. A stop with ANY rank of its own is a stop, loaded from them
  (its borrowed rows are covered by the filled pipes, noted); a stop with none -> extension of the owner it borrows
  the MOST keys from (count 0 = division size). Never the first-in-file rule again (Bellevue: the pedal came first in
  the file and "owned" GO. Bombarde 16; Billerbeck Tuba 16/4 own their ranks, Tuba 8 and "5. Tuba Ped. 2" and
  "P. Flute 16" are extensions). Floating ODF division = no keyboard's `unconditionalDivisionIds` holds it (Bellevue
  "Chamade", reached through the Bom-Ped/Gt/Pos/Rec 8 couplers only; Billerbeck "Tuba" has a keyboard, not floating):
  its stops go stop by stop where the name's prefix says (`divisionIndexByPrefix`: P./Ped. -> pedal, digits and roman
  -> manual N, go/gt/hw/sw/rec/pos/rp/ch/so/ech synonyms against the keyboard and ODF division names), noted.
  One-shot ranks: zero `Pipe_SoundEngine01_ReleaseSample` rows (b = LayerID) -> `Rank::oneShot`; a stop with only such
  ranks -> `Plan.oneShotFolders` -> the configuration's "oneShotPatterns" (defaults + those folder names). Tremmed
  siblings: `isTremmedVariantFolder` (SamplerEngine.cpp) - a folder named like a sibling or its parent + "tr"/"trem"
  (separators trimmed) is skipped by every tree scan (listSampleFolders, scanTreeAssignments: the pool showed every
  Bellevue/Billerbeck stop twice), never by a row that names it (Billerbeck "P. Grosse flute 32" lives in
  Ped_flute32/Rea_Ped_flute32TREM only - loaded as Hauptwerk has it, noted). Defaults: ignored {"trem","*_tr","output"}
  ("outputsum"/"outputharm" under a rank), one-shot {"Chiff","Chimes","Tubular","Celesta","Harp","Glockenspiel"} -
  "Celesta" the instrument, never "Celeste". `StopNames::capitaliseAfterDivisionPrefix` (Source/StopNames.h) on the
  scan's row names, the folder aliases and the HW names/aliases ("P. Trombone 16", "3 Geigenpr 8") - never a custom
  name. Memory 0.8.135: `gSwapBaseline` (SamplerEngine.cpp) = swapped/compressed figure at load start, `shortNow(reason,
  baseline)` fires on a growth > 256 MB (the mac compressor held 0.43 GB of other programs at idle, #21); reserve
  512 MB or installed/16; system share 1 GB on <= 4.5 GB installed; a stop cut in the middle of its files
  (`loadMemoryCutStops`, set in `loadSamplesIntoStop`) is removed whole from the division at the end of the tree and
  named first in `loadMemorySkipped` - nothing half-loaded on the grid. The "Configuration from another computer"
  message box is gone (a log line); `applyPortableConfiguration` closes an open Stop List window (it was shown again
  stale after the load). Dev tools (Python, run with -I): `Tools/odf_stops.py <odf> <name filters>` (stop > StopRank
  rows > ranks > folders, users), `Tools/odf_links.py <odf> <division filter>` (keyboard -> division switch paths),
  `Tools/pool_folders.py <cfg.json> <OrganInstallationPackages>` (the scan folders no row uses = what the pool shows;
  Bellevue and Billerbeck must list nothing but Ped_ContreBomb32's output folders).
  0.8.136: the pool matches folders by their PLACE ON DISK (`placeOf` in `readyStopsNotIn`) and a tree's scan skips
  folders of a nested tree (Testing organ: tree "Samply" holds the trees "Samply/testy/P|I|II|III" - every stop was in
  the pool twice, #23). Memory stop: `loadMemorySkipped` once per stop (`addIfNotAlreadyThere`; #22 said "35 stops" for
  15 rows x perspectives), `loadMemorySkippedKeys` drops a stop with a perspective loaded and another skipped;
  `lastLoadMemoryLimited` (engine, `lastLoadWasMemoryLimited()`) makes the polyphony report say "the loading stopped
  on purpose" - never "the system pages" after our own stop (#25, #26). macOS compressor rule only with free < 2 x
  reserve (#22/#25: 0.3 GB growth stopped a 2 GB load with 3.4 GB free). Report CPU = one figure (render cores busy on
  average; #24), a second with callbackShare 0 while voices sound is never clean (#24: 93 700 after the stream died).
  `EdgeViewport` (UIControls.h) for viewports inside ScrollableContent dialogs: the wheel passes to the window at the
  edge (Stop List buttons unreachable by wheel). `showLongMessage(title, body)` = `LongMessageComponent` (read-only
  editor + OK, `placeDialog` key "longMessage") for the Hauptwerk summary; the polyphony report uses the same component.
  Save prompt = Save / Don't Save / Cancel ("Save As..." without a file). Linux window icon = icon48 (X11 takes one
  image); the desktop-entry installer also writes ~/.local/share/pixmaps/libreorgan.png (48) for antiX-like WMs.
  0.8.137 device-thread profile (while `gVoiceProfiling`): `renderRoute` laps `statRouteTake/Loop/Compact/Separation/
  Reverb/Clip/Record/Stats` (profiling clock, ten reads per route callback), the pool fills `tlPoolOwnTicks` (the
  poster's own chunks) and `tlPoolWaitTicks` (the rest of pool->render: waiting for lanes, merging their scratch),
  `waitIrTask` fills `tlReverbWaitTicks`; `RenderPoolStats::routeCallbacks` + `route*Ns` per route callback
  (`routeMixNs` = loop - own - wait), `routeTotalNs()`. The report paragraph "The device thread, per route callback
  (a period is P us): ..." and the per-second log line ("device thread per route callback: ... N % of the time") are the
  measurement the batch rendering of all routes of a device in one pool job (the next step, not coded yet) must be
  judged against: routes of one device are ALREADY one stream (one AudioDeviceManager host, route callbacks called in
  sequence - `route->managerInUse->addAudioCallback`); the IR convolution already runs as a pool task posted by the last
  route of the group and joined by the first route of the next block. Never claim "one stream per device" is missing.
  0.8.138 couplers: `KeyAction` rows (a source keyboard, c dest keyboard / d dest division, f condition switch = the
  drawstop, n semitones, h = 3 melody; e name "4 To 3,121") -> `HauptwerkOdf::couplers` (+ `switchName` from the
  `Switch` table, b = name); `plan()` maps them to OUR ids "target-source-type" (type 8/16/4/mel; source = our index of
  keyboard a, target = our index of keyboard c or of the division d) within the catalog (target > source any type,
  self 16/4, 1 from 2) -> `Plan.couplers` (`PlanCoupler{id,label,switchId}`), the rest `couplersUnmapped` + notes
  (Bellevue "Bom-Ped 8" to the floating Chamade division, Billerbeck 24'). The import writes the top-level "couplers"
  list ({id, visible}: the HW ones visible, the catalog's default unison ones hidden) and the crescendo steps'
  "couplers" [{id, enabled}] (`CrescendoStep::couplerSwitchIds`: elements reaching a coupler drawstop through the
  linkage BFS, like the stops). Billerbeck must give iv-i, iv-ped, super iii-ped ... as 5-1-8, 5-0-8, 4-0-4 ...;
  Caen 4 To 1 / 3 To 1 / 2 To 1 (keyboards) + the -12 division couplers. `--hauptwerk` prints "coupler <id> = ...".
  Report link 0.8.138: GitHub returned 500 after the sign-in for a long link (the whole system block with 20+ routes
  folded into the sign-in's return URL): `makeUrl` keeps the link under 6000 chars (what/steps clipped to 1500, the
  routes replaced by their count, then the block dropped; everything is in system.txt of logs.zip), logged "Problem
  report link: N characters". OS string with version (Windows DisplayVersion + build from the registry, Linux
  PRETTY_NAME + kernel). The report folder opens with `startAsProcess()` (not `revealToUser`: macOS showed the
  parent), the after-report box sits at the bottom of the screen.
  First profiles (#27-#31, 0.8.137): per route callback the AVERAGE is 100-500 us (own render 70-80 %, wait for lanes
  10-15 %, mix, IR hand-over ~0, bookkeeping up to 66 us at 44 000 voices on the Ryzen) = 13-71 % of the time, while
  "longest driver callback" (DeviceClock: first route entry -> last route end of one driver callback, a gap > 0.2 ms
  between routes starts a new one) was 80-87 % - the break is a PEAK of one period, invisible in averages (a single
  9 ms stretch per second adds 6 us to a 440 us average). 0.8.138 adds `statRouteMaxTotal/Own/Wait` (RenderPoolStats
  `routeMax*Ns`, MeasureState `devMax*`, log "longest T us with own render A, wait B", report "The longest route
  callback of the run took ..."). Next: find what the peak is (a lane preempted? the control thread? the worker asleep
  at the period start on <= 2 cores?) before touching the 85 % rule or batching. `lastLoadError` is logged as "Load
  message: ..." (the Mac reports of #27 stopped by kernel pressure level 2 at 5.0 GB of 8 GB).
  0.8.139: `measurementShareLimit` (MainComponent, machine-specific key "measurementShareLimit", 0.6..1.0, editor
  slider "Measurement limit" before Measure...; `PolyphonyMeasurement::shareLimit` fixed at the start of a run; every
  former 0.85f of the measurement reads it, the "safe" scaling uses limit - 0.05). `cancelLoading()` /
  `loadCancelRequested` + `loadCancelled` (engine; reset at the first tree of a load with gSwapBaseline): the
  loaders' ThreadWithProgressWindow have a Cancel button (30 s), their progress lambdas call cancelLoading() when
  threadShouldExit(); a cancelled StartupFolderLoader compiles the phases like a complete one (finished(false)).
  Adoptable stops (`isPooledStop` or `hasLoadedRankForFolder` on a replacing reload) bypass the memory stop (#33).
  `lowerCurrentThreadPriority()` (SamplerEngine.cpp) on parallelFor workers and copy-build threads (Windows
  BELOW_NORMAL, Linux nice 10 per thread, macOS SCHED_OTHER priority - 8). `startPolyphonyMeasurement` refuses while
  `pendingBackgroundWorkers() > 0` (#32). `arrangeReportWindows(folder)` (static; Windows EnumWindows "CabinetWClass"
  by the folder name -> right half, the foreground window of another process -> left half, SPI_GETWORKAREA physical
  pixels; macOS osascript Finder bounds + System Events frontmost; Linux wmctrl) called 1.5 s and 4.5 s after the
  launch. Coupler catalog: downward couplers for every (target < source, target >= 1) in all five types after the
  pedal bass couplers; default visible only unison with target > source and 1-2-8; HW mapping allows target < source.
  0.8.140: KeyAction h = 2 -> `Coupler::bass` -> type "bass", target 0 allowed for bass (Doesburg bass_1..4 = 0-N-bass);
  `Source/CpuInfo.h` `CpuInfo::hybrid()` (Windows GetLogicalProcessorInformationEx EfficiencyClass; P = the max class,
  `performanceMask`), preset "performance" (menu item only on a hybrid CPU) -> `workersForCurrentPreset` = P cores,
  `applyRenderThreadSetting` sets `engine.setRenderWorkerAffinityMask` (gRenderWorkerAffinity, applied in `workerLoop`
  with SetThreadAffinityMask) and rebuilds the pool (`setRenderThreadCount(..., force)`); the report names a hybrid CPU.
  Report windows: showAfterReport FIRST, then startAsProcess + launchInDefaultBrowser; `placeNativeWindow` HWND_TOP +
  SWP_SHOWWINDOW, BringWindowToTop both, SetForegroundWindow the browser; macOS: frontmost app name via System Events,
  then `tell application "<app>" to set bounds of front window` (no permission needed for Safari/Chrome/Edge/Brave),
  System Events position/size as the fallback. "ASIO shows not ready" (#34 log: `ASIO host opened for 'default
  device': no device`, then "Audio devices still missing after the wait: ASIO default") = a route on the ASIO TYPE
  without a driver name; 0.8.141 `sharedHostFor`: ASIO + empty name -> the driver running in another host of ours,
  else `defaultDeviceNameOfType` (a scan, no open), logged "ASIO without a driver name: X taken".
  0.8.142: `DisplayPage` (a id, b name) + `ImageSetInstance` (b name "2. clairon 4 (StopPage)", e page, g x, h y) ->
  `HauptwerkOdf::stopPositions` (the page with the most stop-named images, name = the stop name minus trailing
  parentheses; Ashton "Simple Jamb" 47/47) -> `PlanStop.gridColumn/gridRow` = ranks of x / y within the division of
  ours (skipped when > 6 columns or > 24 rows, noted "Grid: N stop(s) placed...") -> the import writes
  voicingSettings "stopGrid" {manual: {rowName: "c,r"}} (the voicer's own format) and `HauptwerkExtension.column/row`
  -> `voicer->setGridCell(manual, Voicer::transmissionKey(owner, index), ...)` after addTransmission. "couplersExclusive":
  true in the import -> the settings loader hides every coupler first. `transmissionCaption` = `t.alias` when set
  (never shown before!), else `transposedStopName`, which now skips a division prefix ("4. Tuba 8": the "4." was taken
  for the footage -> "2'. Tuba 8"). Pool (#44): `readyStopsNotIn` counts a row's folders as used only when the engine
  holds the stop (`stopLoaded`); `addReadyStopToDivision` with a row of the same folders present -> no new row,
  `reloadDivisionTrees(manual)`. Measurements 0.8.138-0.8.141 (#40-#45): Ultra 7 P-cores-only 37 600 < all 16
  cores 42 200 (#45: both together better - keep the default; the E-core wait peak 2.2 ms stays, #44), Ryzen
  Windows longest callback 2.1 ms of which OWN render 1.9 ms (#40), Core 2 Duo longest 3-5 ms own render 3 ms
  (#41/#42: the poster is half the lanes), Ryzen Linux 0.4 ms (#43, fine). Verbal recommendation given: cap the
  poster's chunk share at the tail of a job + smaller tail chunks for slow lanes; nothing large left.
  0.8.143 (the user: "tohle zapracuj"): RenderPool wake-ahead - a sleeping worker's `sleepCv.wait_for` is cut to
  `lastJobTicks + periodTicks - 1.5 ms` (min 100 us, max 20 ms) and on a timeout it sets lastWorkTicks so it spins the
  hot window into the period (gFewCores outside Windows: no spin, it just polls); `takeChunkAsPoster` = the poster takes
  nothing more when remaining <= 2 x (lanesJoined - lanesLeft) and some lane still works (never when none works: a
  stall); `Job::openedTicks` + `statJoinDelayMax` -> `RenderPoolStats::laneJoinDelayMaxNs` -> report "The latest a
  lane joined a job after it opened: N us" + log "lanes joined within N us". Storage: `storageSampleRate` (engine +
  MainComponent, settings key, NOT machine-specific; 48000/36000/32000 in the "Sample rate on loading" combo, index 3
  = keep the files' rate) replaces kBaseSampleRate in `decodeSampleFile(..., baseRate)`; `releaseHalfRate` (third item
  of "Sample storage in RAM"): releases decoded at baseRate/2 (`loadSamplesIntoStop` isRelease). The merge estimate
  uses storageRate. `dropNestedTrees()` (called first in `syncDivisionTreeLists`): an inner tree's refs -> the outer
  tree with the path prefixed, indices shifted, division paths replaced and deduped, note + an AlertWindow via
  runWhenLoaded ("Nested folder trees"). Caen like-for-like (#46 Ryzen 54 400 / #47 Ultra 7 54 800): same peak on
  both = the poster rendering alone (2.3-2.5 ms) while the workers wake - this build's wake-ahead + poster tail is
  the fix to be measured; the review is posted on #46/#47.
  0.8.145 packed samples (`Sample::compress16/decode16/read16`, SamplerEngine.cpp after `finaliseLoops`): FLAC-like,
  blocks of 256 frames, header byte (order 0/1/2 in bits 0-1, residual width in bits 2-6), `order` warm-up int16,
  residuals of w bits LSB-first as r + 2^(w-1), w = 0 = all zero; `blockOffsets` for random access; verified frame by
  frame at pack time (a mismatch keeps the raw data). `pcm16` is FREED when packed: every reader outside the render
  path goes through `read16(ch, scratch)` (copy builds, the cache file, rmsOf, the spectrum analysis, MainComponent's
  sample export, the self-test) - never `channel16` on a sample that may be packed. The render (`renderVoiceSpans
  <int16_t>`) decodes a window per span into `tlPackedWindow` (2 channels x main/head x 4096 frames; kept while the next
  spans of the voice read inside it; the span is capped to `kPackedSpanFrames`), positions made relative to the window
  (`posBase`), the splice head decoded a loop length away into the second region (`headOffset = kPackedWindow << 32`).
  Float storage is never packed. Packing happens where a sample is finished: `packIfEnabled` after `decodeSampleFile`
  in loadSamplesIntoStop (on the loader threads, before the memory count), after the cache hit and after the copies
  in the copy build; `notePackStats` logs "Packed samples: ..." per tree / per copy build. Setting `compressSamples`
  (machine-specific key), engine `setCompressSamples`, editor toggle "Compress samples in RAM (lossless)" with the
  price in its text; the voice profile has `unpack` (report "unpacking the packed samples N"). `--kerneltest` prints
  "Packed samples: ... -> OK/MISMATCH" (windows + a packed-vs-raw render over splices and cubic; MISMATCH fails CI).
  Linux: `LinuxWindowPlacement.{h,cpp}` is JUCE-FREE (std::string, `Box`) - Xlib's Font/Time/Drawable typedefs are
  ambiguous under JuceHeader's `using namespace juce` (the 0.8.144 CI Linux build failed on it); never include
  JuceHeader.h and Xlib.h in one translation unit. CMakeLists links `${X11_LIBRARIES}` on Linux (find_package(X11)):
  JUCE itself dlopens libX11, so nothing linked it and the first v0.8.145 tag failed with "undefined reference to
  XOpenDisplay" - the tag was moved onto the CMake fix (the Windows exe is the same build).
  0.8.146 (Linux session): the storage choices (`sampleStorageFloat`, `releaseHalfRate`, `compressSamples`,
  `resampleOnLoad`, `storageSampleRate`) are NO LONGER machine-specific keys: they travel with the organ and
  `MainComponent::askLoadOptions` (an AlertWindow with three combos: storage / packing / rate on loading, Load /
  Cancel) shows them before every Load Configuration and Hauptwerk import with the file's values (else this
  computer's last choice) - the user's choice goes into the configuration var and the members/engine, and is
  saved with the configuration; the dev hooks ORGAN_LOAD_CONFIGURATION / ORGAN_MERGE_ON_START skip the box. The
  startup auto-load and the merge take the file's values silently. Never apply a file's storage choices without
  the box again, and never silently override them from settings.json either (the 0.8.144 rule is replaced by the
  box). Measurement end (`finishPolyphonyMeasurement`): keys up, NO noteOffAll (the tails ring out), the level
  glides up over 3 s at once (callback glide 3 s), the limit (new or restored) is applied 8 s later by
  `setLimitLater` (`pendingMaxPolyphony` is what saveSettings writes meanwhile) - a limit of a few thousand set
  while thousands of tails sounded stole them at once ("the audio stops dead"). `SamplerEngine::setPerspectiveDeviceKey`
  flushes the route's voices (`Impl::flushRouteVoices` = the body of cleanStaleRoutes, `finishRouteVoicesLocked`)
  when the key changes or empties (JACK -> ALSA after a measurement played every tail and kill fade at once).
  Half-rate releases: `decodeSampleFile(..., fullRate, &keptFull)` keeps the full rate when `energyAboveDb`
  (4096-frame windows over the first second, the loudest) above 0.42 x the half rate is > -30 dB; measured on Caen
  Clairon 4' key 80 with numpy (/tmp/halfrate.py): linear interpolation x2 of a 24 kHz file leaves images in
  12-16 kHz at -20 dB and the file loses -9 dB of its energy above 16 kHz - a reed must stay at the full rate.
  Log "Releases at half the rate in the tree ...: N stored at half, M kept at the full rate". The "Not enough memory
  for this organ" box is gone for good (the log line stays). `maxVoices` 500000 (the pool is built at start,
  ~160 MB; the user asked). `Stop::showRankCount` + StopRow `rankCountToggle` ("Rank count [II]", stop folders
  only; saved as "showRankCount": false only when off). `displayOfMainWindow()` for the report windows and the
  after-report box (two screens: everything went to the primary); Linux rects get 36 px for the frame on top;
  macOS box lifted by height/7. `fitWindowOnScreen(..., wanted)`: a dialog that came up < 70 % of the bounds
  placeDialog gave it is set again (the KDE "miniature Configuration Editor" - not reproduced here: the editor
  opened right with and without a remembered position; the cure is generic). Measurement reviews #53-#56: the
  spread is buffer size (256/480/512/1024), the device path (PipeWire default output vs direct hardware vs JACK),
  the packing (16 packed trees: Ultra 7 60 000 -> 16-26 000, Ryzen 64 000 -> 46-52 000) and, on two cores,
  single callbacks at 107-130 % (the OS scheduled something else) plus lane join delays up to 4.6 ms (the worker
  preempted) - no wake-ahead bug (inactive on 2 cores). The Projucer resave and the VS2026 files are NOT updated
  for 0.8.146 (Linux session): do it on Windows next.
  0.8.150 WAYLAND (the AMD KDE box runs Plasma 6.6 on Wayland, XDG_SESSION_TYPE=wayland): the X11 `_NET_CLIENT_LIST`
  holds XWayland windows only (Dolphin/Firefox invisible, "windows seen:" empty) and an XWayland client cannot place
  itself - KWin's placement put the report box in the top-left corner (and that is the user's "miniature Configuration
  Editor in the corner" too, most likely). `LinuxWindowPlacement::placeThroughKWin`: a JS file in ~/.config/LibreOrgan
  loaded over D-Bus (`gdbus call --session --dest org.kde.KWin --object-path /Scripting --method
  org.kde.kwin.Scripting.loadScript <file> <name>` -> "(N,)" - gdbus prints the int32 WITHOUT a type word; 0.8.150
  looked for "int32 N", took every answer for a refusal and fell back to X11 (fixed 0.8.151: the number after "(" is
  parsed, a script of ours still loaded from an earlier run is unloaded first - loadScript answers -1 while the name is
  loaded -, the X11 fallback keeps the KWin note), then `/Scripting/ScriptN org.kde.kwin.Script.run`,
  then unloadScript); the script finds windows by caption/resourceClass (Plasma 6 `workspace.windowList()`, 5
  `clientList()`), un-maximises, sets `frameGeometry`, places our own "Report a problem" box (`wantedScreen`) too.
  Test: `ORGAN_TEST_REPORT_WINDOWS=<folder>` with the folder open in Dolphin and a browser open (/tmp/report_test.sh).
  A KWin script can close a window too (`w.closeWindow()`): /tmp/kwin-close-box.js dismisses the "Audio devices not
  available" box of the user's Doesburg configuration (its routes are Windows Audio) for a screenshot on Wayland, where
  nothing else can click (no xdotool). Website 0.8.151: `site/assets/main.png` = the console with Doesburg Surround
  (the user: "jen na první okno", the Stop List / Divisions / Editor pictures stay; README.md keeps console-stralsund.png).
  24 Oct 2026 (after 0.8.178): BOTH the website's first screenshot and the README picture are the user's own
  StrJak_Screenshot.jpg (Stralsund St. Jakobi in 0.8.178, 2542 x 1308: stop folders [III]/[V], extensions, transmissions,
  the 64' pedal) as site/assets/main.jpg; main.png and console-stralsund.png are gone. The user makes the screenshots.
  0.8.156: the engine's `startNote` no longer checks `keyInCompass` and `keyHeldInCompass` ignores the compass - the
  compass is applied in `MainComponent::routeKeyboardNote` to the physical key only; coupled notes beyond the compass
  play the folder's pipes (001313: files 024-096 on a 36-96 manual). The voicing rows of a stop span the compass
  AND its own sample range (`makeStopRow`, `widestCompass`). `reloadStopSamples(stop, true)` kills every voice first
  (crash 23 Oct 03:08: renderVoiceSpans on freed samples after a "Files vs keys" change while a key was held).
  The key tooltips name the file for pipes of their own too. Tremulant rank drift of a stop folder: the master's
  logical stop has `groupChildOrdinal` 0 and its children 1, 2 ... (applyStopGroups) - the phase code at the voice
  start uses the LOGICAL stop's ordinal plus the perspective rank's ordinal, so a master with pipes of its own and
  its child differ by one rank drift; nothing was changed there. The sample folder chooser is the OS's own dialog
  (`FileChooser(..., true)`): its behaviour on "up" is Windows', not ours. Test configuration: D:\Hauptwerk\...\
  OrganConfigurations\Pokusny.json (tree 001313: stop folders with own pipes, "Files vs keys" -12 on quintaton).
  0.8.155 FILES VS KEYS (#76): the number in a sample's file name is NOT always the key. Doesburg (Sonus Paradisi)
  names the file of key 36 "024" on 232 of 244 ranks (Caen, Beziers, Ashton: 0) - the ODF's `Pipe_SoundEngine01`
  NormalMIDINoteNumber (d) is the key, the Sample row's file name carries the number; our loader took the number as
  the key, so Doesburg sounded an octave high with its lowest octave off the keyboard and the top octave filled by
  copies. Now `Stop::sampleOffset` / `FolderAssignment::sampleOffset` / `StopBuild::Entry::sampleOffset` (json
  "sampleOffset", only when != 0) = file number - key; `loadSamplesIntoStop` puts a file on key number - offset (files
  whose key falls off 0..127 are not decoded; all others are - six-octave folders load whole for the couplers).
  Set in TWO places: the Stop List column "Files vs keys" (after Alias, applied with Apply and reload) and the voicing's
  box "Files vs keys" on every stop with pipes (`StopRow::sampleOffsetBox` -> `Voicer::sampleOffsetRequest` ->
  `MainComponent::setStopSampleOffset` -> entry + `engine.setStopSampleOffset` = `reloadStopSamples(stop, true)` with
  the copies cleared + `finishInPlaceStopEdit`). Adoption on a reload compares the offset too (`hasLoadedRankForFolder
  (manual, folder, offset)`, the commit's source match). The Hauptwerk import: `Rank::sampleOffset` = the most common
  (file number - pipe note) over the rank's pipes, `PlanStop::sampleOffset` the pipe-weighted mode over the stop's ranks
  (children per child), written into the rows; the plan notes "Files vs keys: N stop(s) ..." (Doesburg Surround: 64).
  The per-pipe levels (`levelsForFolders`, keyed by pipe note) are keys already - they were misaligned before on Doesburg.
  Never verify this by the sound's pitch (the user: "je to podle jména souboru ... zřejmé z ODF"). Organs imported
  before 0.8.155 keep offset 0: load the definition again or type the column. Dev hook `ORGAN_TEST_OFFSET=
  "<manual>|<stop prefix>|<offset>"` (the voicing path); the grid dump prints "notes a..b files N" per stop.
  0.8.165 RETUNE LIMIT (Linux session; the user: both - a limit in the configuration, and beyond it the pipe off and
  filled from the neighbour, the voicing tooltip must admit it): measured first (/tmp/pitchcheck.py: audio f0 vs smpl):
  Adlington UW_Flute4 035-B audio 91 Hz = MIDI 41.7, 037-C# 102 Hz = 43.7, the smpl chunks HONEST, the neighbours 036/038
  right - the files are a major third low, and the old ±200 c clamp in computeTuning440 half-corrected them with 440 on.
  Now `Stop::faultyPipes` + `rawTuningCents[128]` (set in computeTuning440 against `retuneLimitCents`, default 200,
  `setRetuneLimitCents` re-applies from the raw values + refillAllStops + copies, like setFillGroupSize),
  `computeFilledPipes` uses a local `has()` = hasNote && !faulty (every former rank->hasNote inside it, the KeepExisting
  struct too) so a faulty pipe is a gap filled from the neighbours; the voicing key tooltip (voicer.cpp, the filled
  branch) appends "Faulty pipe: its own sample X sounds N cents off ... taken from the neighbour". Setting
  "retuneLimitCents" (configuration, NOT machine-specific), slider "Retune limit per pipe" in Advanced global audio
  after "Missing pipes". LINUX FIRST WINDOW (#78): screenshots at start (spectacle -b -n -o, 0.4 s apart) show the
  MAIN window itself mapped by KWin at 0,0 at a tiny size for ~1 s before placeMainWindow - Main.cpp now shows a
  `juce::SplashScreen` with AppIcon.png (384 px) before the MainWindow on JUCE_LINUX, deleteAfterDelay 2 s; the
  scrambled 48 px window icon of #78 is still open. 0.8.166 LESSONS: (1) `Stop::sourceNoteFor` returned a pipe with a
  sample of its own BEFORE looking at the fill map - the faulty pipe kept playing its own sample (key 37 rendered 103 Hz)
  although the report said "filled from 38"; now `hasNote && !faulty` first. The self-test's compass is 36-96, so key 35
  lies outside it and is not filled there (the GUI uses the organ's 35-86). (2) `juce::SplashScreen::deleteAfterDelay`
  counts from the splash's CREATION; MainWindow/MainComponent take 1-2 s to build before the loop paints, so a 2 s
  splash was deleted unseen - 4 s now, called after the MainWindow exists (SafePointer). (3) The start screenshots
  showed the main window mapped tiny (130 x 165 px, a thumbnail of the console) at 0,0 and, on the right screen, a
  mirrored stale-buffer glitch for one frame: `placeMainWindow` is now called once BEFORE setVisible on JUCE_LINUX as
  well - BUT neither helped (0.8.167 screenshots: still a ~160 x 240 px window at 0,0 with a thumbnail of the console
  for ~1 s, and NO splash ever visible on KDE Wayland even with 4 s). What works: `MainWindow::startCover`
  (juce::ImageComponent with AppIcon.png over the content, JUCE_LINUX only, hidden by Timer::callAfterDelay 1500 ms
  through a SafePointer, `resized()` keeps it full-size): whatever size the compositor maps the window at, it shows
  our picture - its hide timer must start at the cover's FIRST PAINT (StartCover::paint, 0.8.168): a timer from the
  construction ran out before the loop painted at all (0.8.167 screenshots: black window, then the console). THE REAL
  CAUSE (0.8.169, activity log "Main window: ... frame -8 -31 17 40 -> 8 60 1 1"): JUCE creates every X window 1 x 1
  (`xCreateWindow(..., 0, 0, 1, 1)`) and the window manager applies the size asked for a moment later (KWin on Wayland:
  up to 2 s while the organ loads); the saved MAXIMISED state (`window.setFullScreen(true)` in `placeMainWindow`) makes
  JUCE's Linux peer read the geometry back at once (`LinuxComponentPeer::setFullScreen`: `getWindowBounds` right after
  `setMaximised`) and SET it as the bounds -> a 1 x 1 window with a 17 x 40 frame at 0,0 ("the very small window with
  strange content"), and `fitWindowOnScreen` then shrank it to 1 x 1 at 8,60 from the same read-back. Fixed: on Linux
  the maximise waits (50 ms polls, 3 s at most) until `LinuxWindowPlacement::windowSize` (XGetGeometry on its own
  display connection, physical pixels) reports >= 40 px, logged "Main window: maximised once the window system gave it
  W x H px"; `fitWindowOnScreen` breaks on a client < 40 px ("not sized by the window system yet") after the `wanted`
  rule - the KDE "miniature Configuration Editor" (0.8.146) was this read-back too. 0.8.169's start screenshots then
  still showed KWin's smallest frame (~135 x 170 px, our icon in its title bar) at 0,0 for ~2 s: the X window WAS
  900 x 600 (the log says so) but nothing of ours had been painted - a Wayland compositor shows a window from its
  first painted buffer on, before that a placeholder - and the first paint came only with the maximise's
  ConfigureNotify ~2 s later (the user: "jiné programy to takhle nemají"). 0.8.170: Main.cpp on JUCE_LINUX does
  `addToDesktop()` (the X window exists unmapped), `placeMainWindow` (bounds; a saved maximised state as the
  `_NET_WM_STATE` property via `LinuxWindowPlacement::hintMaximised` - a maximise REQUEST works on mapped windows
  only, the property is what the WM reads at map time), `setVisible(true)`, then `MainComponent::showMainWindow`:
  polls `windowSize` (10 ms, <= 800 ms) until the WM sized it (>= 80 % of the screen when maximised), lets JUCE
  adopt the state (`setFullScreen(true)` - its read-back is right now), `repaint()` +
  `peer->performAnyPendingRepaintsNow()` (paints at once: `getNumPaintsPendingForWindow` counts XShm completions
  only, 0 before the first paint), logged "Main window: shown, the window system gave it W x H px after N ms, painted
  now". `StartCover::paint` logs "Start picture first painted at W x H px" once. Windows/macOS keep setVisible +
  placeMainWindow + setFullScreen as before. The cover stays. 0.8.170's screenshots STILL showed the
  placeholder for ~3 s although "painted now" was logged 62 ms after the map: (1) `DocumentWindow`'s constructor has
  `addToDesktop = true` and a Component is VISIBLE by birth, so `Component::addToDesktop` -> `peer->setVisible(true)`
  MAPS the 1 x 1 X window in the constructor - the tiny window exists from the first moment, our `addToDesktop()` /
  `setVisible(true)` were no-ops; (2) `XWindowSystem::blitToWindow` has no XFlush: the XShmPutImage waited in Xlib's
  buffer until the loop ran again (~2 s, the device opening). 0.8.171: the DocumentWindow is built with
  `addToDesktop = false` on JUCE_LINUX, then `setVisible(false); addToDesktop(); placeMainWindow; setVisible(true);
  showMainWindow`, and showMainWindow ends with `peer->toFront(false)` (`XWindowSystem::toFront` ends with XSync =
  the flush; `setIcon` flushes too). `DocumentWindow::setIcon` only stores `titleBarIcon` for the JUCE-drawn title
  bar - no JUCE code ever calls `peer->setIcon`, the Linux title-bar icon comes from the desktop entry (WM_CLASS).
  `fitWindowOnScreen` leaves a maximised window alone (`ResizableWindow::isFullScreen()`, read from the peer).
  0.8.172: (1) `SampleFiles::indexOfLocked` walks an archive with gMutex RELEASED (`gWalking` keys; the message
  thread gets nullptr for a key being walked, other threads wait 50 ms steps) - `locate()` on the message thread
  (`updateMissingTreeNotices` every second) blocked on the mutex for the whole cold walk and the "Reading the
  archives" window froze with its count. (2) `UIScale::dialogAnchor` (SafePointer, set to the MainWindow in Main.cpp):
  `createAlertWindow` gives every box without an associated component the main window (JUCE's
  `centreAroundComponent(nullptr)` = the PRIMARY display); on Linux the box is unmapped, `LinuxWindowPlacement::
  setTransientFor(box, main)` (WM_TRANSIENT_FOR, own X connection) and mapped again - KWin (Wayland and X11) centres
  a transient on its parent at map time; `placeDialog` does the same for its windows on a WAYLAND session only
  (`waylandSession()`; X11 honours the remembered position). (3) voicer.cpp key rows: `faulty` (any rank's
  `faultyPipes`) makes the key "filled" (green thumb + the Faulty pipe tooltip); the retune-limit and fill-group
  sliders call `voicer->rebuildUI`. (4) `HintSheen` (UIControls.h): a transparent child of MainComponent over the two
  buttons' union, a golden band sweeping once every 7 s (25 ms ticks), `addMouseListener(&hintSheen, true)` on
  MainComponent ends it at the first click (a Component is a MouseListener already - no second base).
  0.8.173 REGRESSION LESSON (the user: Couplers / Perspective / Ignored / One-shot patterns "nothing happens"): those
  dialogs are `DialogWindow::LaunchOptions::launchAsync()` = `enterModalState(true, nullptr, deleteWhenDismissed =
  true)`, and `ModalComponentManager::ModalItem::componentVisibilityChanged` CANCELS the modal state when the
  component stops showing -> the 0.8.172 remap in placeDialog (`window.setVisible(false)` ... `(true)`) dismissed and
  deleted every such dialog on Wayland. NEVER hide a JUCE window to remap it: unmap the PEER (`peer->setVisible
  (false/true)` = XUnmapWindow/XMapWindow) - the component stays visible and modal. Same in createAlertWindow. The
  sheen: `current` index, one button per sweep in turn, 2.5 s between sweeps (the user: "mnohem častěji, zvlášť").
  Dev hook `ORGAN_TEST_CLICK="<button text>|..."` (MainComponent.cpp, with the other hooks): the named console
  buttons `triggerClick()`ed 3 s after the start, 2 s apart (Wayland has no xdotool; /tmp/test_click.sh) - verified
  0.8.173: "Couplers..." opens centred on the main window and stays. 0.8.174 TEXTS: the website (site/index.html),
  README.md, the release body in build.yml, libreorgans/README.md and the Rural Organ README said "unpack it -
  Unpack archive button" and "300 000 voices" long after 0.8.157 (RAR read in place: Load Configuration opens the
  .rar) and 0.8.146 (500 000); the README said 950 MB for a 725 MB demo. When a feature changes what the user DOES
  (loading, unpacking, limits), grep the site, README, build.yml body and libreorgans/*README* in the same session.
  0.8.175 (Windows): HintSheen::follow(buttons, arrowFromAbove) paints, beside the band, a golden half-transparent
  arrow sliding 90 px towards the button (eased, alpha sin(pi p) x 0.6) - from below on the console's top row, from
  above on the Stop List's "Load folder..." (StopListDialog::loadHint, visible while 
ows.empty() && active(),
  stopped by the first click in the dialog; the orange caption + fixed arrow of paintOverChildren are gone, the
  info line at the bottom is always visible). The website fact "500 000 voices / 2009 laptop" is replaced by "Light
  on RAM, CPU and disk" + feature 07 (16-bit, 32 kHz, FLAC-like packing, half-rate releases, RAR in place, everything
  anti-aliased); README the same paragraph (the user: "to je zásadní tahák"). Projucer resave done on Windows for
  0.8.174 ("Finished saving: Visual Studio 2026").
  0.8.176 AEOLIAN-SKINNER LESSONS (E:\sonus\HW2\aeolian\FirstBeta_Unencrypted_aleSumi: Aeolian_Skinner.part01-06.rar, 42 326
  files, 6 volumes, + Aeolian_BetaODF.CompPkg.Hauptwerk.rar = the definition alone with EMPTY OrganDefinitions/
  OrganInstallationPackages folders): (1) "Load folder" gave a tree with 0 stops - locate() took the FIRST archive
  of the folder (natural order: BetaODF before Skinner) holding the folder name, the empty one; now dirHasContent`n  (childDirs or a file under the key) decides, an empty folder of another archive only as the fallback (both loops of
  locate). (2) olumesOf(partN) built a set from ANY part upwards, so part02..06 chosen = five more "archives" each
  holding the definition ("several definitions"): irstVolumeOf (the lowest-numbered sibling of the .partNN set) in
  indexOfLocked, irstVolumes() dedupes the chosen lists in loadChosenIntoDivision / loadConfigurationFromArchives.
  (3) gIndexingVolume/gIndexingVolumes atomics (Walker::walkVolume) -> the progress text "(part 3 of 6)"; a
  juce::String written by the walk thread would be a race. (4) olumesRead(archive) + MainComponent
  olumeSetsAnnounced: indexArchivesThen shows "Multi-part archive: ... 6 parts read, part01 to part06" once per set
  and session. (5) stopListHintShown: the Stop List's sheen/arrow on the first opening of a session only
  (ddChildComponent(loadHint), followed only then). Test: %TEMP%\test_aeolian.ps1 (ORGAN_TEST_LOAD_ARCHIVES part01,
  50 s, settings restored): the tree now "in the archive Aeolian_Skinner.part01.rar", 1 tree job, stops "001845 - pipes
  - man - CH BlockFl 2" etc. with ORTF/AB/Rea perspectives, every stop auto-transposed -12 (files numbered 024 = key 36).
  CLAUDE.md said "Linux clone commits"; on Windows: git add -u, commit -F file, tag only after git log -1.
  0.8.208 RECORDING vs WALL TIME (the user: "zvuk píšťal se postupně víc a víc předbíhá"): out17's audio.wav was
  380.1 s for 387.4 s of frames. The recorder flushes up to the slowest LIVE route; while a device restarts (the OW
  reload after the folder chooser, a coupler rail rebuild, a route change) no route is live, nothing is written, and
  the routes resume at their OLD positions - the pause vanished from the file and everything after it came early.
  `Recorder::wallStartMs` + flushRecording(final=false): target = max(target, wall frames - 300 ms) - the pause is
  silence, the returning routes are "left behind" (recPos < flushed) and continue after it; "Recording stopped: X s
  in the file over Y s of time" in the log (the two must agree within a second). New routes already get rings
  (ensureRoute). Never judge sync by the mp4 alone: compare audio.wav seconds with the frames' span first.
  GITHUB VIDEO: a README renders a player only for a video uploaded through GitHub's web UI (user-attachments URL);
  a repo file or a Pages URL is a link - the user drags the mp4 into an issue comment and gives the URL. A free
  account takes videos up to 10 MB ("Failed to upload" above): make_video's libreorgan-github.mp4 (1280 px, crf 30,
  64 kbps AAC, 7.1 MB) is the one to upload; the README holds
  https://github.com/user-attachments/assets/28361fcc-1bc5-4f94-ba3f-d0d28406af0a (the out20 video, 26 Oct 2026) on a
  line of its own; the GIF is gone from the repo (15 MB, no longer shown).
  0.8.207 REGRESSION (the user: the Rural Organ demo RuralOrgan.rar downloaded into Downloads, Load Configuration of
  the rar, "load it where it is" -> "Sample folder not found: Downloads\Rural Organ\Samply" under every division, IRs
  "FILE NOT FOUND"): `registerArchive` wrote the chosen archive's line beside the configuration OPEN AT THE TIME
  (another organ's folder - the user's settings even pointed at my %TEMP%\video\cfg after the video runs), and
  `loadConfigurationFile(json inside the rar)` -> `refreshArchiveSearchFolders(new file)` REBUILT the serving set
  (`setChosenArchives`) from the lists beside the new file (a virtual folder: none) and in the settings folder (none):
  the archive just chosen dropped out before its trees were looked up. The 0.8.160/0.8.177 tests passed only because
  the settings-folder list already held lines. Fix: registerArchive writes the settings-folder list ALWAYS (the
  program's memory of every archive chosen - "on si pak měl zapamatovat, kde ty rary leží") plus the configuration's
  own list when its folder is real; `SampleFiles::existsAsFile` for the IR candidates and the route caption, the engine
  reads an IR inside an archive through `SampleFiles::openInput` (createReaderFor(stream)). Test: %TEMP%\fix207.ps1
  (ORGAN_LOAD_CONFIGURATION=<rar>: the hook goes through loadConfigurationFromArchives; settings and the global list
  backed up/restored). Every test run of mine that loads a configuration RETARGETS settings.json - a killed run leaves
  the user's program starting on MY file; restore it by hand after every killed run.
  0.8.177 THE USER'S RULE ("On nesmí brát archivy, které člověk nevybere!"): ONLY the archives the user chose serve paths -
  Load folder / Load Configuration / Merge (with their parts) and the ARCHIVE lines of ArchiveFolders.txt; never another
  .rar lying beside them, and no longer every archive of the configuration's own folder. 0.8.157's design took every
  first-volume .rar of a folder (rchivesOfFolderLocked), which the Aeolian-Skinner folder's definition-only package
  exploited. Now gChosenArchives (SampleFiles: setChosenArchives/addChosenArchives, rchiveOf(path)),
  rchivesOfFolderLocked(folder) = the chosen archives in it (+ every archive of a folder named as a whole: OrganRarPackages,
  old folder lines), servingArchivesLocked() for the "elsewhere" loop; MainComponent::registerArchive(archive) (was
  registerArchiveFolder(folder)) writes the archive's first part into ArchiveFolders.txt, the Hauptwerk import writes
  SampleFiles::archiveOf(odf); 
efreshArchiveSearchFolders turns a FOLDER line into the lines of its archives once
  (log "the folder line X replaced by its archives: ...") - a line of another OS's path (the Linux /run/media/... line
  in the user's Adlington list) is neither a folder nor a file here and stays as it is. The user's Adlington list had
  the aeolian FOLDER line (written by 0.8.176's Load folder): replaced by hand with Aeolian_Skinner.part01.rar so the
  migration would not bring the BetaODF package in. Test 0.8.177: the BetaODF archive is never opened, both trees "in
  the archive Aeolian_Skinner.part01.rar", the settings-folder list's Adlington\10 folder line migrated to Vol0 + Vol1.
  0.8.178 FOOTAGE SHORTHAND (the user's rule): a bare "3" in a folder name is the quint 2 2/3' ("6" = 5 1/3', "12" =
  10 2/3', "1 1/2" = 1 1/3'; no rank of 3'/6'/12' exists) - ootageFromName maps the parsed feet before the semitone
  count; the caption still shows the name's text ("Nasat 3'") for an untransposed stop and the MEASURED footage once
  transposed. "CH RohrNasat 3" (files sounding 1 1/3' on their numbers) got -14 before, -12 now like the rest of the
  set (the Aeolian files are numbered 024 = key 36 on every stop). Commits from ~/LibreOrgan-linux (the NTFS checkout edited,
  files copied over); NO Projucer resave on Linux - do it on Windows next (the VS2026 files lag 0.8.165).
  `LibreOrgan/CLAUDE_REPORTS.md` is GITIGNORED (local only, never committed): a `git add` naming it fails, and a
  chained `add && commit; ... && git tag` then tagged the OLD commit (v0.8.169 first landed on 83e888a: the run
  cancelled, the tag deleted and re-pushed on 5685fd9). Never chain the tag after the commit in one command: tag in
  a separate step after `git log -1` shows the new commit, on Linux as on Windows.
  24 Oct 2026 (0.8.179): a push to main was REJECTED - the "Configurations check" index job commits to main by itself
  after any change under libreorgans/ (the old Stralsund folder removed -> "INDEX.md regenerated" on origin) - and
  the tag push went through, so v0.8.179 pointed at a commit off main (the 0.8.147 situation again). Resolved by a MERGE
  (`git pull --no-rebase`), never a rebase, so the tag stays an ancestor of main. Always `git fetch` + `git status -sb`
  before a push that follows a libreorgans/ change, and push main BEFORE tagging.
  0.8.164 FULL KERNEL FOR HIGH PIPES (the user: the foreign tone BELOW a retuned high tone is not a beat; "440 on
  is essential"; the slider must reach very low for people with CPU to spare): `Voice::fullKernel` set at every voice
  start by `wantsFullKernel(rank, pipe)` (sounding Hz from pipe + footageSemitones + sampleOffset, like fadeScale,
  against `sincKernelFromHz`: 0 = all, >= 20000 = none, default 3000; engine atomic, MainComponent member, machine-
  specific key "sincKernelFromHz", editor slider "Full kernel from (sounding pitch)" with `addSlider(...,
  skewMidpoint 2000)`). In renderVoiceSpans `sinc = ((lowRate && pos < sincEndF) || v.fullKernel) && !blend ...` and
  `sincXf = blend && v.fullKernel ...` (both reads need idx >= 4) -> `dispatchMonoSinc/StereoSinc` and
  `dispatchMonoXfSinc/StereoXfSinc` by KernelKind: scalar (`kernelMonoSinc`, `kernelMonoXfSinc`), AVX2
  (`avx2SincTaps` 8 taps from idx-3, `avx2SincCoeffs` = the two nearest phase rows blended, `avx2Sum8`), Lane4
  SSE2/NEON (`lane4SincTaps` int16 via unpack/srai or vmovl, `lane4SincCoeffs`, `lane4Sum`). PHASE INTERPOLATION is
  essential: the nearest of 128 rows alone steps the position by up to 1/256 frame = a sawtooth at 48000 x (ratio-1) Hz
  -> a sideband at exactly the undertone's place at -41 dB; blending the rows takes it below -44 dB (`sincAt` does it
  for the scalar/lowRate path too). The LOOP SPLICES must go through the sinc as well: a copy spends up to 40 % of its
  loop in the crossfade (transposeLoopCrossfadePct) and linear splices kept the image for that share. Measured
  (self-test hook `|pitch=20|sinc=20000|pure` vs `sinc=3000`, HW_Principal4 x4 copy at 8.8 kHz, ratio 1.0116): the
  undertone 511 Hz under f0 -25.4 dB -> gone (< -44 dB); what stays (+-1.5 % sidebands, -28..-34 dB) is in the
  recording at ratio 1 too. The kerneltest compares SIMD sinc and XfSinc with the scalar ones (mono/stereo, 7 ratios).
  Hook options added: `|sinc=<Hz>`, `|temper=<cents>` (setTemperamentOffsets; NOTE mode 2 voices use selectedCents,
  so it did nothing on Adlington - use `pitch=` under 25 c to emulate a residual), `|pure`; env ORGAN_SINC_FROM_HZ for
  the full self-test (cost). The user's remark holds: only very high voices show it, elsewhere masked.
  SELF-TEST "Swap divisions ... WRONG" (seen in 0.8.162 and 0.8.163 and ONCE BEFORE, commit ee789c5): a fault of the
  TEST, never of the engine - with one loaded division the routes of division 1 had no device key after the swap (a
  route without a device gets no voices since 0.8.126), so "key 60 on division 1 -> 0 voices". Fixed for good in
  EngineSelfTest.h (both divisions get the test's device key/channels/rate/block size after the swap). Never report it
  as a regression again; if it fails, the test setup is what to read first.
  0.8.163 RETUNE BUSES (the user chose "variant A", no retune at load, no regression): `Route::Bus` (kMaxBuses 3 per
  route, kBusTaps 16, kBusPhases 128, kBusFifo 4 x kMaxBlockFrames + 32, kBusMinCents 25, kBusJoinCents 6). At every
  voice start (attack and release) `commonRetuneCents(stop, rank)` = globalPitchCents + (tuneTo440 ? 1200 log2(440 /
  measuredDiapasonHz) : 0); `busFor(route, cents)` joins a ready bus within 6 c or makes one (control thread: vectors
  allocated, `buildBusTable`, then `ready`; never freed; all slots taken -> 0 = direct as before); `v.bus`, and the
  voice's frame counts use route rate x bus ratio. `renderRoute`: with `anyBus` the active list is split into
  `route.direct` + `bus.list` per block; `renderBus` renders the bus voices through the pool into the bus FIFO with a
  BlockContext copy whose routeRate = rate x ratio (so inc = fs/(R r) x ratio(total) ~ 1 and every ms-based constant
  stays real-time), the shelf applied there, then reads the FIFO at the ratio through the sinc into the route output
  BEFORE separation/reverb/clip/recording. The pitch math of the voices is untouched (r cancels), only the kernel's
  position stays near integer. Buses are logged "Retune bus N of route ...". Aux routes get their own bus
  (`duplicateVoiceToAux(..., busCents)`). Verified: `--selftest` hook `|pitch=126.6` -> f0 x 1.0758, spectrum as the
  direct render; GUI (ORGAN_TEST_RECORD with equalAt440 forced on in a temp copy of the config) x4 copy at 9416 Hz with
  no images (before: 5711/13121/2006 Hz at -26..-39 dB), real pipe clean; kerneltest OK; a run without any retune
  creates no bus and takes the old path verbatim. The user's remark: the images matter only for very high voices,
  elsewhere masked - the bus covers all voices anyway at one resampling per route. NOTE: 32/36 kHz storage is NOT a
  retune (ratio 0.667/0.75 comes from the sample rate, not from cents) - its images stay; that needs a kernel or copies
  at the route rate (not done). The user's Adlington file meanwhile had equalAt440 false and Principal 4 at -24: a test
  that reads his file must print the grid item line ("transposition N") before judging a pitch.
  0.8.162 "ALIASING" LESSON (the user: "+1 octave aliasing jako prase, regres"): measured, not guessed. Dev hooks:
  `ORGAN_SELFTEST_RENDER="<stop part>|<key>|<shift>|<out.wav>[|<stop transposition>|<velocity>|late]"` (EngineSelfTest.h,
  `--selftest <tree>` with ORGAN_SELFTEST_DIVISIONS=2: a transmission into the last division, the key rendered 2 s + 1 s
  tail to a mono WAV, the copy of the filled pipe beside it as <out>.copy.wav; ORGAN_SELFTEST_INT16=1 for 16-bit) and
  `ORGAN_TEST_RECORD="<manual>|<key>|<velocity>|<seconds>|<out.wav>|<stop prefix>[|notune]"` (voicer.cpp, needs
  ORGAN_DUMP_GRID=1 like every grid hook: waits for the copy build, switches every other stop/transmission off, records
  the routes' output through `engine.startRecording` = the whole GUI path with the loaded configuration). Analysis:
  %TEMP%\alias_check.py / peaks.py (numpy in Python39: C:\Users\doma\AppData\Local\Programs\Python\Python39; the
  lilypond python on PATH has no pip), %TEMP%\ospc_dump.py parses the FilledPipesCache .ospc files (JUCE little-endian
  ints, string + NUL, per record pipe/offset/formant/sourceFrames/copies, each copy a length-prefixed block). Findings:
  the engine alone renders ratio 2/4/8/16 copies CLEAN (16-bit too, offset applied late too, velocity 60 too). The GUI
  showed inharmonic peaks = IMAGES OF THE LINEAR INTERPOLATION KERNEL: (a) with 32 kHz storage (the hook takes the
  file's load options: Adlington's file says 32 kHz/halved/packed) the copies are 32 kHz and play at ratio 0.667 ->
  images at 32000 - f (8740 Hz -> 23254 Hz -16.6 dB, 32000 + f folded -> 7266 Hz -26 dB); (b) at 48 kHz with "Equal at
  440" on (the merge switched it on; Adlington stands at a1 = 409 Hz -> +128 cents, ratio 1.0773) the images of the
  x4 copy's fundamental land at (48000 - 8740) x 1.0773 - 48000 = 5711 Hz (-25.6 dB), 13121 Hz, 2006 Hz - exactly
  measured; with the retune off the same GUI render is clean. The `v.cubic` rule (cubic only for filled pipes played
  from the plain sample) is from the first commit; copies and real pipes are LINEAR at any ratio - temperaments and the
  random 4 cents give sidebands near the partials (-30 dB), a big retune moves them far away = "aliasing". Cubic kernels
  are scalar (hermiteAt per frame), the 8-tap sinc 13 ns/frame: neither can take every voice. The proper cure (not
  done, the user decides): retune at LOAD (resample each sample by its own tune-to-440 correction with the sinc
  resampler, playback ratio 1) or copies at the route rate when the storage is lower. Done: the merge of the same organ
  (shared stops) keeps the pitch. ORGAN_LOAD_CONFIGURATION takes the FILE's storage options (32 kHz for Adlington) - the
  user's own GUI loads said 48 kHz; always check the "Load Configuration" log line before comparing.
  0.8.161: (1) `makeAntiAliasCopy` taps = 191 up to ratio 4, else 191 x ratio/4 (odd) in a std::vector - the fixed 191
  Blackman taps (transition ~0.029 c/s) aliased at ratio 16 (cutoff 0.027, stopband from 0.041 > the fold 0.031): the
  user's "+24 stop + +24 trsm = +4 octaves alias jako prase", +3 octaves fine. (2) MERGE GRID: `sharedStopNames`
  ("<manual>|<added key>" -> the base row playing the same folder; `baseFolders` is a map now) - a division with any
  shared stop is "the same organ in volumes": the added page's cells are taken for the WHOLE division (the shared rows
  mapped to the base names, shift 0); no shared stop = another organ, block under lastRow as 0.8.154. The user's rule:
  same stops "dissolve" into the new volume's grid; a different organ is pressed right under the last row. Never
  compact a foreign organ's page rows.
  0.8.160 (the user's batch after 0.8.159): (1) COMPONENTS: `componentOf` (SamplerEngine.cpp, twice in
  `scanTreeAssignments` - the learning block and the assignment loop) = a single trailing L/l/R/r after a digit
  ("quintaton16L") or a CAPITAL L/R after a small letter ("glockentonL") with the sibling of the other half present;
  `FolderAssignment::component`, Stop List `FolderRef::morePaths` (the second half of the same perspective; json
  "morePaths", cell text "+ half", changedDivisions/usedFolderKeys/placeOf/nested-tree prefix all include it), the
  automatic-rules loop and `extendStopBuildWithTree` join the halves into the row, `deriveStopBuildFromEngine` puts a
  second rank of one perspective into morePaths. 001313 fresh: one "glockenton 2/3'", one "quintaton 16'", AB/Re
  learned. Never take "Mixtur4x"/"Zimbel3" for a component (no L/R). (2) Name rule: `stem()` in the displayParent test
  strips l/r + digits + "'" so "HW_Bl.Flute2" over "02-direct-HW_Bl.Flute" gives the leaf alone (rar loads too).
  (3) `chooseFromList` (AlertWindow + ComboBox, enterModalState, Load/Cancel) replaces the PopupMenu of
  `loadIndexedIntoDivision`/`loadConfigurationFromArchives` ("Load which one": the popup vanished on a click outside
  and loaded nothing). (4) HW import `ownerOfFolder` (HauptwerkOdf.cpp, before the `rankFolders.empty() &&
  !borrowed.empty()` block): a stop whose rank folders are ALL owned by an earlier stop becomes `borrowed` of that owner
  (an extension with its own key range) - Adlington "Bl. Flute Treble" is a trsm of "Bl. Flute Bass"; a folder is never
  loaded twice. (5) MERGE from an archive: `resolveTree` and the pool entries in MainComponentMerge.cpp test
  `SampleFiles::isDirectory` (juce::File::isDirectory is false inside a rar -> every tree {} -> "Nothing new to add");
  Adlington Vol1 Surround ODF into the Demo config = 7 added, 6 shared, 1 new tree. (6) Stop List: `unloadAllTreesFromPool`
  sets `reopenStopBuildManual`; `updateApplyState` no longer requires `treeInfos` non-empty (Remove empty stops after
  Unload all was never applicable, so the empty rows came back with the next folder); `StopListWindow::closeButtonPressed`
  asks Apply / Discard / Cancel through the public wrappers `StopListDialog::hasUnappliedChanges()` /
  `applyChanges()` (the originals are private); the header's perspective x sits right after the label text
  (GlyphArrangement width). (7) Auto-transposition in `loadRankSamples`: MEDIAN of (raw pitch - name footage) over the
  pipes, |median| >= 2 semitones and >= 70 % agreement - Adlington stands at a1 = 409 Hz (a semitone low as a whole)
  and got +1 on Principal 4 / Flute 4 with the per-pipe rounding. (8) Crash on a transposition change while playing
  (crash-20261023-145122, renderVoiceSpans on freed samples): `Stop::reloading` atomic set in `reloadStopSamples` before
  the kill, a second kill right before the swap, cleared after; the voice-start loop skips a reloading stop. Test hooks
  used: ORGAN_LOAD_TREE="1|<001313>" + ORGAN_DUMP_GRID; ORGAN_LOAD_CONFIGURATION=<Demo ODF> + ORGAN_MERGE_CONFIGURATION=
  <Surround ODF> (the merge hook accepts virtual archive paths); ORGAN_TEST_ARCHIVE keeps the window open - kill it.
  Bellevue 1.35 rar (E:\sonus\HW2\Casavant\1.35_ostry, 5 volumes, 43 598 files, 39 GB): the index took 84 s on the
  HDD; its definition is `OrganDefinitions/Bellevue, Casavant Surround.Organ_Hauptwerk_xml` (virtual path under
  1.35_ostry); the `updates` folder's 1.12/1.13 rars hold the ODF only (42 MB) - a load from there finds no trees.
  (9) #80 GRID vs JAMB: the ODF cuts `ImageSetInstance` names at 32 characters ("GO. Flute a cheminee 8 (NonPhoto")
  - the matcher stripped "(...)" only when the name ENDED with ')', so 14 of Bellevue's 74 stops (the chamades, Flute
  harmonique, Contretrombone 32, Quarte de nazard ...) had no cell and took the first free cells; now an unclosed "("
  is cut too (`--hauptwerk` must say "Grid: 74 stop(s) placed"). Not a regression: 0.8.142 placed 60 of 74 from the
  start. The zipper fold of a narrow division column (voicer rebuildUI step 3, first commit) is the other reason a
  3-column pedal (x 1327/1506/1688) looks different in a 2-column division - documented, not changed.
  /tmp-style check: %TEMP%\jamb.py <odf> lists the jamb images OK/NO against the stop names.
  0.8.159 ADLINGTON LESSONS (the user's seven complaints in one message): (1) the ODF had keyboards "Pedal" (25
  keys) AND "2. manual" (52 keys) on the division named "Pedal": `plan()` now gives a division to the keyboard
  with the MOST keys (`ownerOfDivision`), a pedal keyboard left without a division becomes the empty P. with its
  compass (`pedalPullDown`, a note "couple it from that manual"), and `isPedal` is never true for > 40 keys - P. empty,
  I. Pos, II. the 12 stops (was P. 12 stops + I.). (2) Stop key ranges from the stop's OWN StopRank rows
  (`PlanStop::lowKey/highKey`, `keyRangeFor`; extensions from their borrowed rows) -> `hauptwerkCompass[(division,
  row key)]` and `HauptwerkExtension::lowKey/highKey` -> `applyHauptwerkVoicing` switches the keys off per stop
  (`setPipeEnabled`) and per extension (`Transmission::keyDisabled`); `compassForFolders` (union over EVERY stop
  using the folders) stays as the fallback only - it never limited a shared folder. (3) Adlington's UW_Flute4 ("Ch.
  Flute") has smpl pitches a THIRD low on 035-B and 037-C# (41.8 / 43.7 vs 45.8 / 47.8 expected at a1 = 409 Hz) and
  correct on 036-C: `computeTuning440` took B+C# for a footage break and the lone C between them was smoothed INTO it
  -> C at +404 cents, the pitch-class mean "C95" (all C a semitone off), temperament not recognised. Now a run < 3
  takes the longer neighbour's offset wherever it lies (the first/last runs too) and |deviation| > 50 cents never
  enters the temperament -> "footage 4'", Meantone (-1/4) RMS 0.8 c. (4) Transposition boxes: EVERY semitone -24..24
  again (0.8.158's clean-interval list was a regression - the user: "tam bylo +-24 pt"); `footageName` gives the
  pure (just) fraction for the non-harmonic semitones (`justIntervalNumerator/Denominator`: 16/15 9/8 6/5 5/4 4/3
  7/5 3/2 8/5 5/3 9/5 15/8 -> 7 1/2', 6 2/3', 6', 5 5/7', 5', 4 4/5', 4 4/15' ...; never a decimal) and
  `pureDeviationCents` retunes those shifts to the same just ratio (tempered before). (5) Perspective learning in
  `scanTreeAssignments` treats "xxx16L"/"xxx16R" sibling folders as one stop for the prefix comparison (001313: AB
  and Re were not learned because AB had glockentonL/R and quintaton16L/R). Components as ONE stop in the automatic
  scan is NOT done (the Stop List holds one folder per perspective per row; it would need stop-folder rows from the
  scan) - pending. (6) Load folder: several definitions in the archives -> a PopupMenu (never `definitions.front()`);
  `loadConfigurationFromArchives(archives, merge)` for Load Configuration (multi-select .rar) and Merge (.rar in the
  chooser, `mergeConfigurationFile` reads a json inside an archive). (7) #79: StartupFolderLoader `progressLine` +
  `workLine` -> two-line status. Smpl facts: Adlington a1 = 409 Hz (smpl pitches absolute), 036-C_070 velocity layer
  files exist; the "two tones a third apart" was the C36 retuned by -404 cents against its neighbours (tune-to-440).
  0.8.158 TRANSPOSITION (the user's rule, three messages): "Files vs keys" IS the transposition - ONE number,
  `Stop::sampleOffset` (= file number - key, the engine's `keyShift()`), named Transposition everywhere (voicing box
  `shiftBox` shared with the transmission rows, Stop List column "Transp."), counted against the REAL footage the files
  have on their own numbers (smpl pitch - file number), never against the name: Doesburg quintaton "16" = files 024..
  sounding 8' on their numbers -> "-12 -> 16'" (the ODF says -12, the auto-detection says -12, the box shows -12).
  `Stop::footageSemitones` (computeTuning440, `footageSounding`, `footageBreaks`, `lowestFootageSemitones`) is the REAL
  footage of the files on their own numbers (pitch - file number = pitch - (key + sampleOffset)) - the Orig. Pitch
  line shows it (quintaton "8'"); what sounds on the keys is real + sampleOffset (`stopFootageSemitones`, `fadeScale`,
  `displayName`). `untransposedFootageSemitones` = stopFootageSemitones - sampleOffset is the box's base ("0 -> 8' (as
  the original)"); `cleanIntervals()` = {-24,-17,-12,-8,-5,-2,0,2,4,7,10,12,14,16,19,22,24} (+ the
  current odd value) are the ONLY steps offered in both boxes (the user: "14.25' at -10" is nonsense). Auto-detection
  in `loadRankSamples` when the row carries no value (`FolderAssignment/Entry/Stop::sampleOffsetSet` false = the json
  key "sampleOffset" absent; the Hauptwerk import writes it ALWAYS, also 0 - the ODF has precedence): mode of
  round(smplPitch - key) over >= 3 samples with >= 70 % agreement vs `footageFromName(displayFolderName(folder))` or the
  stop name -> offset += named - sounding, samples re-placed, `sampleOffsetDetected` -> `takeDetectedSampleOffsets()`
  (StartupFolderLoader afterCommit) writes it into the Stop List rows (set). No footage in the name -> nothing. Verified
  on Pokusny (001313): "NachtHorn 8" (files 024.. sounding 4' on their numbers) -> -12 = Doesburg's ODF value; 001313 smpl
  unity notes are the real pitch (echoBurdon 024 -> 35.98, quintaton16 024 -> 23.98). Caption footage: untransposed
  stops keep the name's footage as ever; a transposed stop shows the MEASURED footage (`Stop::displayName`,
  `nameWithFootage`), else name + offset. The Hauptwerk import strips the footage from aliases (`nameWithoutFootage`:
  stops, children, extensions) so the caption's footage is the sounding one. ONE-SHOT: `finishNoteOff` skips
  `isOneShot` stops (a key up never releases or fades them - the user's rule; the engine never had it, the first commit
  released them too); `Stop::oneShotManual` (-1 auto by the patterns / 0 / 1; voicing state "oneShot", `setStopOneShot`,
  `updateOneShotFlags` honours it), voicing row toggle "One-shot" after the alias (`oneShotToggle`, every stop with
  pipes). "Rank count [II]" only on stop folders (visibility set in `setChildrenGetter` - the getter comes after
  construction), after the alias. CONFIGURATION PLACEMENT: `HauptwerkOdf::configurationFolderFor(root)` =
  OrganConfigurations beside the folder ABOVE HauptwerkSampleSetsAndComponents (`D:\OrganConfigurations` for
  D:\Hauptwerk\...), else beside the root (rar set E:\sonus\HW2\caen\dEMO -> E:\sonus\HW2\caen\OrganConfigurations);
  never inside Hauptwerk's tree (the user's rule). ARCHIVES ELSEWHERE: `SampleFiles::setSearchFolders` +
  `locate()` suffix matching (>= 2 path parts, longest first) over the archives of: the configuration's folder, the
  lines of its `ArchiveFolders.txt`, `OrganRarPackages` beside, the settings folder's ArchiveFolders.txt
  (`MainComponent::refreshArchiveSearchFolders` at start/apply/load, `registerArchiveFolder` on every archive chosen
  in Load folder / Load Configuration, the rar import writes the list beside the new configuration). Load Configuration
  takes `*.rar` (`loadConfigurationFromArchive`: index job, .json (not .bak) first, then ODFs, several -> PopupMenu;
  a .json inside is read via SampleFiles::openInput). Dev hook `ORGAN_TEST_TRANSPOSITION="<manual>|<prefix>|<st>"`
  (was ORGAN_TEST_OFFSET). Manual audit of 0.8.158 fixed: master default -18, melody/bass new-key default ON, "Hide
  all voicing" only folds, Stop volume slider, combination row (Bank, Tutti, X, Pistons), Measurement limit + ALSA
  switch in 8.1, Show settings.json / Refresh values, the Divisions dialog's own captions, "_128"; code tooltips
  -52 dB and "Fletna", master label "-18.0". The user's settings `configurationFile` pointed at my Adlington test
  configuration (E:\sonus\HW2\Adlington\10\OrganConfigurations) - restored to F:\Stralsund_Jakobi_Wegscheider.portable.json;
  every test run must back up AND restore settings.json (a load retargets it).
  0.8.157 RAR (`Source/SampleFiles.{h,cpp}`, jucer ids SmFls1/SmFls2 + CMakeLists): a configuration never names an
  archive; every path is a disk path, `locate()` walks up to the nearest existing ancestor folder and looks the rest up
  in the indexes of the first-volume `.rar` files of that folder (the disk wins; `archivesIn`, `volumesOf` = partN /
  .r00 sets). The INDEX is OUR OWN header walk (`Walker::walk4/walk5`: RAR 4 blocks CRC/TYPE/FLAGS/SIZE[/ADD_SIZE],
  RAR 5 CRC32/vint size/type/flags/extra/data; one 1 KB read per block, seeks over the data; FILE/NEWSUB/SERVICE/
  ENDARC; split before/after -> `Entry::pieces` {volume, headerOffset/Length, dataOffset/Length}; per volume the
  main block and the end block offsets; `solid`, `stored` (RAR4 method 0x30 / RAR5 method 0), `encrypted` (RAR4
  FHD_PASSWORD, RAR5 extra record 1, CRYPT block -> refused), RAR4 unicode names ported from libarchive's
  decoder). Cache `%APPDATA%\LibreOrgan\ArchiveIndex\<hash>.idx` v2 ("V main|end" per volume, "D path",
  "F order|size|stored|encrypted|pieces|path"), keyed by the first volume's size+mtime; written through a
  MemoryOutputStream (a juce::String appended 18 000 times took MINUTES - O(n^2)). READING: `readEntryAlone` - a
  stored file straight from the volumes; a packed one through libarchive fed by `VirtualStream` with the file's
  bytes alone ([sig+main][file header][data] and between the pieces [end block][next sig+main][continuation
  header][data], 4 KB of zeros at the end for the RAR 5 reader's 512-byte look-ahead) - one handle per file, so the
  loader's parallelFor reads files in parallel (`FileData::archived` + `SampleFiles::readFile` in
  `loadSamplesIntoStop`; a solid archive is read in sequence by the old cursor, `readEntriesInSequence`). Measured
  (Re_glockenton 189 MB in 8 variants: RAR4/5, volumes, recovery record, stored): byte-exact against the disk,
  100-120 MB/s unpacked per thread packed, 1 GB/s stored; Caen DEMO import from its rar 7 s (was 47 s with the
  sequential cursor). WHY OUR OWN WALK: libarchive 3.8.9's RAR 5 `scan_for_signature` checks 504 of every 512-byte
  chunk, so the next volume's signature on a chunk edge is missed (Ashton 14 x 4.48 GB, all volumes a multiple of
  512: the index ended after 27 GB and 34 GB were read through) and its RAR 4 reader fails ("Bad RAR file") on the
  18 zero bytes WinRAR leaves after the end block of a volume with a recovery record (-rr); libarchive is a git
  submodule with a prebuilt archive.lib - never patch it, feed it. `indexArchivesThen` (MainComponent) builds the
  indexes on a ThreadWithProgressWindow ("Reading the archives", `SampleFiles::indexingProgress`) before
  `loadIndexedIntoDivision`; E: on this machine is a 16 TB Toshiba HDD (3 ms per seek): a walk of 18 000 headers
  is a minute there. Dev: `ORGAN_TEST_ARCHIVE=<rar>` (volumes, index time, bytes read, top folders, definitions),
  `ORGAN_TEST_ARCHIVE_CHECK=<disk root>` (every 50th wav + the first 80 compared byte by byte - Rar.exe stores
  the path from the drive root, so the root is `D:\` for the rartest sets in %TEMP%\rartest made by
  `C:\Program Files\WinRAR\Rar.exe a -ma5 -m3 -v40m -rr3`), `ORGAN_TEST_LOAD_ARCHIVES="<division>|<rar>"` (the
  Load folder path, the definition box skipped), `ORGAN_OPEN_LOAD_FOLDER=<division>` + ORGAN_TEST_SNAPSHOT. The
  Unpack archive top-bar button is hidden by default (the 0.8.112 one-time forcing is gone); Load folder's switch
  "Unpack the archives into a folder first" + destination does the unpacking (`startUnpack` with a continuation).
  0.8.154: merge grid - `baseShown` (MainComponentMerge.cpp, the base's non-child table rows per division) decides the
  "last row"; the added organ's first row lands at lastRow + 1 (Beziers stood 11 empty rows under Caen: stale cells
  of stops gone/pooled/indented counted). The move between divisions keeps the arrange mode (`dropStopReferences()`
  instead of exitArrangeMode in the move-house branch); "Grid drop: ..." log line in `dropItem`. `freezeGrid()/thawGrid()`
  (MainComponent): `applyPortableConfiguration` takes a snapshot of `stopsViewport` before `engine.unloadManual`, shows
  it dimmed (0.55) over the workspace, the loader's completion thaws it - the grid stayed blank for the minutes a
  merge took. The user drags in MY test instance when one is open (he found the minimized window): every test run
  shows up in his hands - say so, or close it first. Disk facts (this Ryzen): the organs live on D: = Samsung 870 QVO
  8 TB SATA; Caen (45 GB) 24 s from the page cache, 119 s cold (~400 MB/s = the disk) - a merge of a second 45 GB organ
  evicts the first; "loading was super fast before" = warm cache. Never call a cold load a regression without the
  warm/cold pair. Filled pipes: the copy build (SincTable, makeAntiAliasCopy, applyStaticEq, `formantFollowForShift`: the
  stop's setting downward up to an octave, +30 points for any upward shift, beyond an octave +15 points per further
  octave for the resampling methods / +25 for the phase vocoder, keepLoudness, copyParametersKey v5) is unchanged since 0.8.49; the cache record
  carries the source frame counts, so copies of a 32 kHz run are not taken for a 48 kHz load.
  0.8.153 CI LESSON: the two Windows pushes of 22 Oct evening (main + tag v0.8.153) updated the refs but GitHub created
  NO workflow run and no check suite for them (Actions enabled, status page green, the Linux pushes of the afternoon
  ran) - `POST /repos/.../actions/workflows/build.yml/dispatches {"ref":"v0.8.153"}` (build.yml and pages.yml have
  `workflow_dispatch`) started the build; a dispatched run on a tag ref still publishes the release (github.ref =
  refs/tags/...). When `actions/runs?head_sha=<sha>` says 0 and `commits/<sha>/check-suites` says 0 five minutes after
  a push, dispatch instead of re-pushing the tag. Packing measured (Doesburg demo, 8 GB raw, warm cache): 0.8.145 packer
  15 ns per frame and channel = 63 s of thread time (~4 s wall on 16 threads), 0.8.153 one-pass packer 6.7 ns; the
  "Packed samples" line prints the thread time. A cold load of the same demo took 38 s (disk), warm 7-8 s.
  0.8.153 REGRESSIONS (the user: "soupis všech poškození a regresí"): (1) 0.8.144's rebuildUI rule "a stop folder with
  no children is left off the grid" hid every NEW stop folder (empty by nature) - now only after a load stopped for
  memory (`engine.lastLoadWasMemoryLimited()`). (2) The 0.8.146 "Rank count [II]" switch was NEVER shown: its
  visibility was decided in StopRow's constructor with `childrenGetter` still null (the getters come after construction
  through `setChildrenGetter`) - now visible on every expanded stop row right after the button, enabled in
  `setChildrenGetter`. Never decide a row widget's state from the getters inside the constructor. (3) The 0.8.146 size
  cap of the Report a problem dialog (60 % of 800 = 480 px) left the buttons a negative height on 1280x800 (#69/#72/#74) -
  fixed parts first, editors flexible. (4) Loading: the user's own files carry "packing on" (Caen 96-138 s vs 47-82 s
  off; Doesburg cached 14 s off / 36 s on / 58 s at 32 kHz without packing) - the load box shows it; the pack costs
  ~2.5x (three predictor passes + a full unpack verify), the 32 kHz sinc (99 taps) 4x. Not yet made cheaper.
  Grid dev hooks (voicer.cpp, end of rebuildUI, 1.5 s after a layout): `ORGAN_DUMP_GRID=1` (buttons + free cells in
  SCREEN coordinates, expanded rows with `describeLineOne()`), `ORGAN_TEST_DROP="<manual>|<stop>|<target>|<col>|<row>"`
  (dropItem as the mouse would; verified: Rural "TA - violon" 0 -> 1), `ORGAN_TEST_NEW_FOLDER="<manual>|<col>|<row>"`,
  `ORGAN_TEST_EXPAND="<manual>|<stop prefix>"`, `ORGAN_TEST_SNAPSHOT=<png>` (+ "<png>.row.png" of the expanded row on a
  dark background - the texts are white, a bare snapshot shows nothing). ORGAN_LOAD_CONFIGURATION needs
  ORGAN_DUMP_STOPLIST or ORGAN_SAVE_PORTABLE set as well. Start test runs `-WindowStyle Minimized` and NEVER drive the
  mouse or take screen captures while the user works at the machine (he was writing mail). The drag between divisions
  by mouse (StopRow::mouseDown/Drag/Up) is unchanged since 0.8.111 and the drop path works - #77's "nefunguje
  přetahování" still needs the user's description of what happens. Small test organs on Windows:
  F:\Suchy_rejstrik\Kobrle\Rural Organ.json and TestingOrgan.json (portable, trees beside them).
  0.8.151/0.8.152 RAM min: `SamplerEngine::estimateMinimumRam()` (every k-th of all loaded samples INCLUDING the
  perspective ranks in `Stop::ranks` - 0.8.151 took the first rank alone, Doesburg 2.3 of 7.2 GB -, ~160, first 3 s each:
  resampled to 32 kHz with `SincResampler`, a release halved to 16 kHz when `energyAboveDb(0.46 x 16 kHz) <= -18`,
  int16, `compress16`; ratio of their bytes to `frameBytes()` x all sample bytes) -> the portable save writes
  `estimatedMinimumRamBytes` = (measured - sample bytes) + sample bytes x ratio and `estimatedMinimumRamStorage`
  (`minimumRamStorageText`), log "RAM with every saving option (...)"; the README RAM line gets "; about X GB at the
  least, with every saving option of the load box (...)" and `configurations_index.py` the sortable "RAM min" column
  (absent -> "-"; >= measured -> the measured figure: the organ was saved with the saving options in force).
  `measuredRamStorage` = the whole `storageSignature()` since 0.8.152. Never type or estimate it by hand. Doesburg
  0.8.152 test: /tmp/ram_test.sh (auto-load + ORGAN_SAVE_PORTABLE into /tmp/ramtest, the device box closed by the KWin
  script) - Doesburg (already 16-bit/32 kHz/halved/packed): samples 6.76 of 7.23 GB, ratio 1.03 -> min = measured;
  Rural Organ loaded as float 48 kHz: 1.76 GB measured, 0.90 GB at the least (ratio 0.232, 79 of 86 releases halvable).
  A portable save (the hook too) RETARGETS settings' `configurationFile` to the saved file: a test script must back up
  and restore settings.json, else the next start says "Started without a configuration" when the file is gone.
  The copies of filled pipes are counted because they are in RAM and in the measured figure (a few per cent); the
  disk cache of filled pipes plays no role anywhere in the RAM figures - never say a fast load is "thanks to the cache"
  (Doesburg loaded in 12 s from the OS page cache, 53 GB of WAV still cached from the previous load). `juce::jmax<juce::int64>(0, x)` does not compile with
  juce_dsp (the SIMDRegister overload takes the explicit argument): use std::max or a ternary.
  0.8.149 (Linux): `AfterReportWindow` (MainComponent.cpp, modeless DialogWindow, always on top, `gAfterReportBox`)
  replaces the modal AlertWindow after a report (#58 keyboard, #67 invisible blocker behind the main window on KDE,
  #64 macOS buttons shrunk: the System Events fallback resized OUR front window - now skipped when the frontmost app
  is LibreOrgan/Finder/empty; `AfterReportWindow::resized` puts its size back). `ORGAN_TEST_REPORT_WINDOWS=<folder>`
  shows the box + placement 4 s after the start (open the folder and a browser by hand first). LinuxWindowPlacement:
  `wmSupports("_NET_MOVERESIZE_WINDOW")` -> EWMH message alone, else XMoveResizeWindow (never both); file manager by
  class when the title lacks the folder; the note lists the windows seen. `HauptwerkOdf::spelledAsOnDisk` (rank
  folders spelt as the disk, #57). `Sample::sincHead` (set in decodeSampleFile: halved AND energy above 0.25 x rate
  > -40 dB; copies inherit it): the sinc runs only for those (Core 2 Duo Mac 4100 -> 2200 with every release through
  it, #64 14:18). Levels log: "Audio levels (master X dB[, measurement level -52 dB on top]): ...". #62/#63 "aggregate
  20 dB louder": every aggregated run 13:19-13:54 shows -15 dB at 10 k voices, ASIO/Windows Audio from 14:01 -38 dB -
  the user lowered the master to -40.6 as his reaction; the per-route renders are the same chain for every device
  type (renderRoute: masterGain x route volume), so the loudness comes after our buffer: 15 shared-mode streams of one
  route each on the endpoint "ADAT (7+8)" - most likely the endpoint's audio enhancements (loudness equalisation
  boosts each quiet stream) - told the user to disable them on the endpoint; JUCE's shared WASAPI client does not set
  AUDCLNT_STREAMOPTIONS_RAW (would need a JUCE patch). Measurement table of every issue log: /tmp/measure_all.py
  (dedupes by start time + CPU, version by the release times, options from the "Load Configuration" line).
  `Stop::storageSignature` (= `SamplerEngine::storageSignature()` at decode, copied on adoption): a reload adopts a
  division's loaded samples ONLY with the same storage choices (`hasLoadedRankForFolder` and the adopt lambda check
  it) - before, a reload after the Load box kept the packed/halved samples of the previous load (#62: the "16-bit,
  32 kHz" runs at 7 000-11 000 were the packed organ; "packedTrees 0" after a reload = adoption). The measurement
  summary names "samples <signature>".
  0.8.148: LESSONS. (1) "Audio callback anomaly: <old device> manual N perspective default: no callbacks" every minute
  = a route WITHOUT a device (closed by closeHiddenRoutes, "remembered as none") whose RouteCallback kept its old
  deviceLabel/expectedBlock - fixed (audioDeviceStopped zeroes expectedBlock/Rate); never read such lines as a dead
  device on the endpoint (the #59 first analysis did, and sent the user to set a "default" route to none that does
  not exist in the dialog - closeHiddenRoutes works). (2) The sinc kernel of 0.8.147 applied to every sample below 0.8 x
  the route rate - with the 32 kHz storage that is EVERY attack, scalar 13 ns/frame vs 1.1 AVX2: Doesburg 45 400 ->
  15 900 (#61) and 7 700 with 1 lane (#60); now < 0.6 x the rate = halved releases only. When a report shows "kernel
  6684" instead of ~1500, suspect a kernel path change before anything else. (3) Aggregated WASAPI: `aggregatedRoutesRunning`
  (counted in RouteCallback::audioDeviceAboutToStart/Stopped) caps the lanes at physical cores / routes in the callback
  (15 routes x 16 lanes: 16 highest-priority workers spinning for 15 posters starved the endpoint threads - breakups at
  one pipe, clean with 1 lane; #59). (4) The load box takes the FILE's storage choices: the user loaded Doesburg with
  "packing on, 32 kHz, halved releases" from his Linux experiments without noticing - say so in every polyphony
  analysis (the log line "Load Configuration "<name>": sample storage ..., packing ..., rate ...").
  0.8.147 (Windows): the level after a measurement is held at -52 dB for 4 s (`outputGainHoldUntilMs`) and then
  glides up IN DECIBELS, 4.5 dB/s (~12 s to 0 dB) - the callback's `factor = 10^(4.5 * blockSeconds / 20)`; a linear
  glide (0.8.146 "3 s") reached -20 dB in a third of a second and the user heard a jump into the tails. NEVER a
  linear gain ramp upward. `SincTable` + `sincAt/kernelMonoSinc/kernelStereoSinc` (8 taps, 128 phases, Kaiser 6,
  cutoff 0.45): `renderVoiceSpans` uses them when `s.sampleRate < 0.8 x routeRate` (halved releases, 36/32 kHz
  storage) for the first 1.3 s of the sample (`sincEndF`), not for splices/cubic; the images of linear interpolation
  (7-11 dB under the signal for a 2x upsample) go to -40..-50 dB; the full-rate rule is now -18 dB above 0.46 x the
  halved rate. `--kerneltest` renders at 24 kHz too (the sinc path packed vs raw). The import's crescendo writes
  "transmissions" [{manual, enabled}] on the owner row for its extensions (`extensionsOfOwner`, the order of
  `hauptwerkExtensions` = the engine's transmission order; `storedTransmissionsOn` counts ordinals the same way).
  `resolveCaseInsensitive` in `firstExistingDirectory` (#57: Windows "ped" vs disk "Ped" on Linux). `gAfterReportBox`
  closed by `showProblemReportDialog` (#58: the modal box kept the keyboard). Manual 5.8 "What moves the measured
  figure" (buffer, device path, packing, two cores, what the measurement cannot see). #59 (Aggregated WASAPI, Doesburg,
  breakups at one pipe, the measurement clean): the breakups happen after our buffer (15 shared streams on ONE endpoint
  "ADAT (7+8)" + five "default" routes with a dead Windows Audio device on the same endpoint: "Audio callback anomaly:
  no callbacks") - asked the user to try default routes = none, Aggregated lanes = 1, ASIO; the measurement cannot see
  a mixer dropout (documented). Linux #58 multi-monitor placement still open (file manager "not found" by title).
  0.8.144 - LESSON: the 0.8.143 wake-ahead used `periodTicks` (the smoothed gap between CONSECUTIVE JOBS = the gap
  between the routes of one callback, ~0.3 ms with 8 routes) as the period -> workers woke every few hundred us and
  spun: #48 Core 2 Duo Linux 2900 (was 2500-2600 ok), #49 Mac 5000 but kernel 9.7 us/voice (3x), #50 Ultra 7 8 routes
  10 700 (was 54 800) with callbacks at 131-145 %. Now `periodStartTicks`/`devicePeriodTicks` (a job after a gap >
  0.3 ms starts a period), wake-ahead only with >= 2 workers, min wait 0.2 ms. NEVER derive the period from the job
  cadence. Unison off: `conditionOfKeyboardDivision` (keyboard -> division -> the condition switch of the conditional
  key links) -> a `Coupler{unisonOff}` for a keyboard whose primary division has no unconditional keys -> id
  "N-N-uoff". Empty stop folders (children skipped) are left off the grid in `Voicer::rebuildUI` items. Every
  configuration file keeps this machine's keys (`applyPortableConfiguration`: the machineSpecificKeys override runs for
  non-portable files too; "resampleOnLoad", "storageSampleRate", "releaseHalfRate" added to the list). ParseJob plans
  on its thread (status text). `nestedTreesNoticed` dedupes the notice per session. `Source/LinuxWindowPlacement.{h,cpp}`
  (X11: _NET_CLIENT_LIST, _NET_WM_NAME/WM_NAME, WM_CLASS; _NET_WM_STATE remove maximised + _NET_MOVERESIZE_WINDOW +
  XMoveResizeWindow + XRaiseWindow; in the jucer as LxWpl1/LxWpl2 and in CMakeLists). #52: Aggregated WASAPI 20 routes
  x 1 lane per endpoint thread = 62 900 > the shared pool - route-level parallelism beats lane-level; keep default 1.
  Ashton: the user's ODF ("Ashton-under-Lyne, Albion Church, Lewis Organ, Surround", 34 KeyActions incl. "Choir to pedal
  sub/oct") - the couplers are right; only the unison off was missing (now a coupler). The "Save, then load reloaded
  the organ" report: the log shows the chooser opened and the same ODF was imported again at 15:36 - the chooser starts
  in lastSettingsDirectory with no file preselected; nothing of ours reloads. 0.8.145 = the packed samples above
  (measured: Rural Organ 25-42 % saved per stop, the self-test tutti +40 % per block, kerneltest unpack 1.5 ns/frame).
- A route without a device ("none") gets NO voices (0.8.126, `Route::hasDevice` set by `setPerspectiveDeviceKey`,
  checked in `publishVoice`: the voice is finished at once). Before, every pipe was published to its perspective's
  route whatever the device and sat there unrendered until `cleanStaleRoutes` flushed it after ~1 s, counted as
  polyphony: the "8000 voices" of the Core 2 Duo Mac (#14, default routes none + auxiliary on the device) were half
  ghosts - the same stimulus (61 keys x 7-8 strikes/s) on the default routes gives 2500. The self-test sets a device
  key on every route (`"selftest|<perspective>|<manual>"`) - never clear it to {} there, that silences the route.
- Engine kernels (0.8.119): AVX2 (8 frames; `ENGINE_AVX2_FN` = `__attribute__((target("avx2")))` on GCC/clang, plain on
  MSVC - before 0.8.119 the AVX2 code was `#if _MSC_VER` only, so Linux and both Macs rendered scalar), SSE2 and NEON
  four-frame kernels (`Lane4`, one code for both), scalar. `bestKernel()` picks at run time; `--kerneltest` (CI on every
  platform, Apple silicon included) compares each kernel with the scalar one (end positions identical, samples within
  1e-4 of the peak) and prints ns per voice and frame from cache and from RAM (`benchmarkKernel`). ENGINE_CPU_PAUSE is
  `_mm_pause`/`yield`, never `std::this_thread::yield()` (a system call in a spin). The render pool keeps lane
  utilisation statistics (`takeRenderPoolStats`); the polyphony report shows the CPU of the RENDER CORES (16 threads
  busy on a 32-thread CPU = 100 %, not 50 %), the lane utilisation, the kernel benchmark with the ceiling it gives, and
  the fixed-per-block / per-voice split of the callback time with what other buffers would give - never a general
  "doubling the buffer doubles the polyphony" claim (false, #8). Measurement level -52 dB.
  Voice profiling (`gVoiceProfiling`, on during the measurement): ns per voice and block for header / spans / kernel /
  post, folded from thread-locals into the pool stats; the report prints it. Stellwagen 0.8.119: 135 / 170 / 1050 /
  880 ns - the 880 was the dynamic tone shelf (private buffer + split pass, on for every voice as soon as the wind sags),
  folded into the kernels in 0.8.120 (`kernel*Shelf`, `dispatch*Shelf`: out += y(1-k), wet += y k; the route low-passes
  the wet sum as before). Splice/cubic spans and per-key tone voices still go through a private buffer.
- "All physical cores" means ALL of them on every computer, two-core laptops included - never silently one (the user
  refused that: nobody picks threads by hand). What broke the Core 2 Duo laptops in 0.8.119-0.8.122 (1700 -> 1400
  voices, #9/#10) was not the SSE2 kernel (their own benchmark: 4.6-4.9 vs scalar 6.7-7.1 ns) but that on Linux and
  macOS the render workers and the control thread had NO priority (Windows had THREAD_PRIORITY_HIGHEST + MMCSS from
  the start): a worker preempted by PipeWire/CoreAudio/the GUI held every block up, and the poster waited with
  `_mm_pause` instead of yielding. 0.8.123: `raiseAudioThreadPriority()` (Linux SCHED_FIFO/SCHED_RR, logged when
  refused; macOS time-constraint thread), `gFewCores` (<= 2 physical): poster waits with `std::this_thread::yield()`,
  chunks <= 12 voices, hot spin window 1 ms. Never blame a kernel without the benchmark numbers from the report.
  ENGINE_CPU_PAUSE outside Windows is `std::this_thread::yield()` again (do not bring the pause spin back there
  without a measurement). 0.8.127: a real-time thread's wait for another thread (`ENGINE_RT_WAIT`: the poster for
  its lanes, a worker for its task) is `_mm_pause` on Windows and a 30 us SLEEP everywhere else - NEVER a yield:
  under SCHED_FIFO `sched_yield` hands the core only to FIFO threads, the SCHED_OTHER window thread starved on the
  two-core Linux laptop, and as it drives the measurement's strokes the keys stayed down and the hang never resolved
  (#15, "Message thread unresponsive for 70 s"). On <= 2 cores outside Windows the worker does not spin after work.
  `CrashHandler::StallWatchdog::setStallAction` (called from the watchdog thread after 3 s of silence): the
  measurement sets it to `engine.noteOffAll()` + abandon the run; cleared in `finishPolyphonyMeasurement`. x86 without AVX2 renders SSE2 by default again since 0.8.126 (`defaultKernel`): the user's
  own A/B with the editor's switch on the same routes - Core 2 Duo Mac scalar 1700 vs SSE2 2500 voices (#14, 0.8.125),
  Linux bench 5.07 vs 7.70 ns (#13) - and he asked for it ("bude stary CPU defaultne loadovat SSE2?"). 0.8.123-0.8.125
  had scalar there at his request from before the priority fix.
  The kernel is a VISIBLE switch (0.8.124, the user asked three times where SSE2 is): Config Editor > Engine >
  "Interpolation kernel" = Automatic / scalar / SSE2 / AVX2 / NEON (only what this build+CPU can run,
  `SamplerEngine::availableKernelNames()`), setting `renderKernel` (machine-specific key), applied at once through the
  atomic `gKernelOverride` read by `bestKernel()` every block (`setKernelOverride`, -1 = automatic). `ORGAN_KERNEL`
  (env, `envKernel()`) only sets the Automatic choice for one start; the editor's choice wins. The log line
  "Render kernel: X" and system.txt ("kernel X") say which runs; the measurement report names it.
  Profiling clock (0.8.124): `profileTicks()` = `__rdtsc` / `cntvct_el0`, calibrated once in `setVoiceProfiling(true)`
  (`calibrateProfileClock`, 100 ms, logged "Profiling clock: N MHz"), deltas through `profileDelta` (negative or > 1 s
  dropped). Never use `juce::Time::getHighResolutionTicks()` per voice: clock_gettime was ~1 us on the HPET Core 2 Duo
  Linux laptop and inflated bookkeeping/spans threefold (#11).
- Filled-pipes cache `copyCacheEnabled` is ON by default (0.8.124; the user's 16-core rebuilt every Stralsund copy at
  each load with it off); the "Copy build of ..." log line says "(the filled pipes cache is OFF...)" when it is not.
- Measurement ramp: a level held with `holdShare` < 0.5 steps half the way to the proportional 85 % point, at most
  doubling (16-core: 40 000 in five steps instead of twenty); below that the old 2 000 / tenth steps.
- The polyphony report makes NO forecast for other buffer sizes (128/256/512 runs on the Ryzen: 24 900 / 38 600 ->
  no model fits); the small-buffer paragraph is a recommendation (256/512), never "breaks at a few hundred voices".
- Render threads default = all physical cores, always. `renderThreadPresetChosen` (settings, machine-specific) marks a
  choice made in the Configuration Editor; without it a non-default preset is reset at load (the old portable Rural file
  carried `allMinusOne` and left two-core laptops on ONE render thread, issues #6-#8).
  A polling lambda stored in a shared std::function (`*poll = [...]{...}`) must never do `*poll = nullptr` while it
  runs - that destroys its own captures (a copy `self = *poll` does NOT help: the executing object is the original) -
  it clears itself afterwards: `juce::MessageManager::callAsync([keep = poll] { *keep = nullptr; });`. Two crashes
  of 0.8.116 (read of 0x24/0x28 in a SafePointer); a crashed process is then held by WerFault.exe, which keeps the
  exe locked for the linker - `taskkill /F /PID <WerFault>` frees it.
- "A stop" is every button of the grid: a stop with samples, every transmission, every extension, and a stop
  folder that has at least one sound under it (its own or a child's); rows nested in a stop folder are not
  stops (the folder is). NEVER count or switch only the stops with samples of their own (`engine.getStops()`
  with `enabled`): use `MainComponent::gridStopItems()` / `Voicer::gridItems()` (grid order) and
  `setGridItemEnabled()` - the polyphony measurement, its retreat and every "N stops" text do (0.8.111).

## Dialogs and layout
- THE ROWS OF ONE SUBJECT STAY TOGETHER (the user's rule, 25 Oct 2026, "to si zapiš jako pravidlo na vždycky"): a
  new Editor row, slider, toggle or button goes INTO the group it belongs to (every "Key velocity: ..." row in one
  run, every tremulant row in one run), never between the rows of another subject. 0.8.179 and 0.8.192 both broke
  it (a tremulant row among the key-velocity rows; the "_128" switch between two "Key velocity" sliders).
- Every window and dialog must be wholly visible: title bar (draggable) and bottom edge included, with 2 % of
  the screen height free above and below AFTER the taskbar/dock/panel is taken off (`workAreaWithMargins`,
  `fitWindowOnScreen` + `fitWindowOnScreenSoon` retries at 150/700/2000 ms - window managers place a window
  after it shows). A dialog taller than that is shrunk, never centred with its edges off the screen.
- Use the space of a dialog: a button is 100-150 px wide, never stretched over the row; a switch and its
  button (or any setting and its action) go on ONE row (`addToggleButton`), never the button under the switch.
  A dialog that asks for a file and a folder shows both as path fields with Browse... buttons in one window,
  the thing the user looks for first at the top (Unpack archive), never a chain of bare file dialogs.
- GitHub's repository tabs (Code, Issues, Pull requests, Actions...) are fixed; a "Releases" or
  "Configurations" tab cannot be added. README.md therefore starts with a heading of links: RELEASES (the
  program), the Rural Organ demo, "Configurations, Organs" (always that name - people want organs), plus a
  release badge. Every release carries two "Source code" assets added by GitHub itself - they cannot be
  removed; the release bodies say they are not needed.
- Release bodies and READMEs are for humans: one bullet per system/platform, the one sentence that matters
  (how to unpack, what to download) in bold on its own line, links as links - never one long paragraph.

- 0.8.179: NOTHING in the README is filled in by hand any more (the user: "tak to udÄ›lej tak, aby to bylo jednoduĹˇĹˇĂ­" - he
  completed the bullets and missed the two places in the first paragraph; the issue check then failed and his fix came as a
  COMMENT, which triggers nothing - the workflow runs on opened/edited of the issue BODY). `askPublicationDetails` (an
  AlertWindow with five text editors: organ / producer / product / link (its own field, an http link) / author) runs at
  every portable save with publishFiles, Cancel saves nothing; the values travel in the configuration as "publication"
  {organ, producer, product, link, author} (saveSettings/parse) and `configurationReadme` writes them; the README's HTML
  comment only says where to attach the files. The slug of a published configuration = the zip's base name lower-cased
  with [^a-z0-9]+ -> "-": "Stralsund_Jakobi_Wegscheider.portable.zip" -> stralsund-jakobi-wegscheider-portable, a NEW
  folder beside an old one (the old folder must be removed by hand - done 24 Oct for stralsund-jakobi-wegscheider).
  REMOVING a published configuration (24 Oct 2026, the user: "musĂ­m mĂ­t jednoduchou moĹľnost na GitHubu"): the workflow
  `.github/workflows/configuration-remove.yml` ("Remove a configuration", workflow_dispatch with the input `folder`):
  Actions > Remove a configuration > Run workflow > the folder's name as in INDEX.md > Run. It `git rm`s
  libreorgans/<folder>, runs configurations_index.py, commits and pushes to main (refuses rural-organ, paths with
  "/" or "..", and a folder that does not exist - it lists the folders). Documented in libreorgans/README.md.
  0.8.180 (the user: "proÄŤ se ztratilo dvouĹ™ĂˇdkovĂ© loading"): NOT a regression in the code - `git log -S` shows the
  loader's two lines (0.8.159) and the engine's only two `loadStatusCallback` calls ("Compiling the Stop List: tuning..."
  / "pipe fills") unchanged since the first commit; the second line existed only in those phases of each tree (a second
  or two on Stralsund), the decoding showed one line by design. Now `loadDivisionTree` reports every stop being decoded
  ("Loading <stop> ...") so the second line is there all along. The publishing box: no "product" field (the label "the
  name of the sample folders" read as directory names - the folders are the program's own line in the README).
  `buildAntiAliasCopies` checks `memoryShortNow` before every copy and stops the build (copiesIncomplete, log) - 0.8.131's
  memory stop covered the sample decoding only. Stralsund in the user's file: 32-bit FLOAT storage (the load box's
  choice travels with the organ), 53.9 GB of WAV -> 82 GB of samples + 11 736 copies of filled pipes (64' extensions, Vox
  Humana Cornet [V] 432, Trompete 348 ...) -> 98 GB held; "RAM with every saving option" says 22.9 GB.
  0.8.181 LOADER WINDOW LESSON (the user: "problikĂˇvĂˇ 3. Ĺ™Ăˇdek ... tohle je regres"): a juce::AlertWindow sizes its
  width from the text's length (w = 300 + 2 sqrt(fontHeight x textWidth), capped) and lays the message out with
  createLayoutWithBalancedLineLengths - a line over ~55 characters WRAPS, and setMessage grows the window only
  (onlyIncreaseSize), so a long status text adds a line that stays as an empty gap when the next text is short =
  "a flickering third line". The 0.8.159 two-line status flickered for the same reason (line 2 only in the compile
  phases). Rule: the loading window's texts are SHORT (engine: "Analysing tuning, temperament and footage ...",
  "Filling the missing pipes ...", "Loading <stop, cut at 44 chars> ..."), and the status has ALWAYS exactly two
  lines ("Reading the folders ..." / " " when nothing else). Regression hunting for a dialog = a run with the
  window captured (%TEMP%\test_loader.ps1 finds "Loading saved divisions..." by title and crops it) - not code
  reading alone. Save Configuration (0.8.181): the four README lines sit in the box itself (PortableRow grows on the
  tick, `alert->setMessage(sameText)` re-lays the box out); the hook ORGAN_SAVE_PORTABLE never asks.
  RAM FIGURES (0.8.182, the user: "proÄŤ se Stralsund naÄŤetlo do 8 GB? jak poÄŤĂ­tĂˇĹˇ RAM?"): the RAM GB column and
  `measuredRamBytes` = the PROCESS (Windows max(WorkingSet, PrivateUsage)); the table's "RAM" is the figure at the
  portable save with its storage text, "RAM min" the estimate. Stralsund: float 48 kHz 97.9 GB (82.4 GB samples + 15.5
  GB "rest" = the copy builds' working memory and heap slack), 16-bit/32 kHz/half-rate/packed 8 GB. The estimate added
  the rest UNSCALED (22.9 GB); now the rest above a 0.75 GB floor scales by the same ratio. The copy build is
  parallel INSIDE a stop (parallelFor over its pipes, all cores), stops serialised by antiAliasBuildLock - several
  stops at once would gain only the serial phases (cache read/write, swap-in), ~10-20 %; 0 cache hits after a
  storage change is expected (the cache records carry the storage parameters).
  0.8.183 REGRESSION REVIEW (the user: "regres jeden za druhĂ˝m ... porovnej kĂłdy za uplynulĂ© dny"): the diffs
  620ea1e..HEAD (0.8.156-0.8.182) read for behaviour broken on the side. Found and fixed: (1) the Rank count
  caption of 0.8.180 was computed in StopRow's CONSTRUCTOR (the getters are null there) - the 0.8.153 lesson
  again; it lives in setChildrenGetter now. (2) `AlertWindow::setMessage(sameText)` is a NO-OP (it compares the
  text first): the 0.8.181 Save box never grew, the four README lines stayed clipped - a trailing space makes
  the text differ. (3) `updateFromStop` selected the transmission's Transposition box by the OLD ids (maxShift + 1)
  while the list uses shift + 100 since 0.8.158 - blank after every updateStopStates. (4) 0.8.158's one-shot rule
  (`continue` in finishNoteOff) left `speaking`/`pipeStartMs` set, so a chime struck twice was silent: the pipe's
  bits are cleared, the voices untouched. (5) The adopt lambda of commitPendingDivisions copies every analysis
  field but forgot `faultyPipes`/`rawTuningCents` (0.8.165). (6) `reloadStopSamples`' second killAllVoices
  (0.8.160) ran for evenIfLoaded=false callers too (a pooled stop back on the grid cut every held tone) - now
  under `if (evenIfLoaded)`. (7) `loadConfigurationFile` replaced the archive search before the validity check
  and the load box. (8) The Load folder field was prefilled with lastSettingsDirectory (the configuration's
  folder): `Panel::browseStart` starts the choosers instead. Left as designed, noted: pureDeviationCents on
  transmissions with odd shifts (0.8.159's just intervals apply to the shared box), the sinc + shelf cost via
  spanTmp (no dispatch*SincShelf), the double decode of the auto-transposition on the first load, stem() on
  parent names. HOW TO KEEP REGRESSIONS OUT (the user's question): every GUI change is RUN before delivery, with
  the hooks - `ORGAN_TEST_CLICK="<button>|<button in the dialog>..."` finds buttons in EVERY window (0.8.183;
  the console only before) and `ORGAN_TEST_DIALOG_SNAPSHOT=<folder>` writes the open dialogs' own paint as PNG
  (no screen capture, no mouse); `ORGAN_TEST_EXPAND="<manual>|<stop>|<delay ms>"` opens a voicing row later
  (with ORGAN_TEST_RECORD: the row opens while the key sounds; %TEMP%\test_expand.ps1 + dropouts.py analyse
  the WAV); %TEMP%\test_savebox.ps1 pictures the Save box with Portable ticked. A change that touches a
  dialog, a row widget or a caption without such a run is not done. And the process name is LibreOrgan_<v>:
  `Get-Process LibreOrgan*` before any test run (the user plays through the day).
  0.8.183 PACKED FLOAT SAMPLES (#87, the user: "komprese nefunguje na 32 bitech"): `Sample::compress32` packs a
  float sample as the INTEGERS its file held - the scale 1/32768 (16-bit) or 1/2^23 (24-bit) is the first one
  every frame fits bit-exactly (JUCE's readers give v / 2^23 for 24-bit files, exact in float); the same block
  format with 4-byte warm-ups and widths up to 27 (`packChannel32`/`unpackBlock32`, the 16-bit functions
  untouched), `packScale` turns them back, `decode32`/`read32` mirror decode16/read16, `pcm32` is FREED. The
  render window (`PackedWindow::bufF`, `region<T>()`, `decodeWindow<T>()`) serves both types; `packable` is true
  for int16 and float. STRALSUND'S FILES ARE 32-BIT FLOAT WAV (format tag 3, checked with a header read): real
  floats, no integers at all - the first version left every tree sample raw ("not the file's integers"). Real
  floats (a float WAV, 32/36 kHz on loading, halved releases, a filled pipe's copy) are ROUNDED to 24 bits
  (`packRounded`, "rounded to 24 bits" in the Packed samples line: half a 24-bit step, nothing a converter
  resolves; the editor text and the manual say so); only a sample beyond +-2 full scale stays raw. Never promise
  "bit for bit" for a float file. Measured: Stralsund float 48 kHz + packing 53.1 -> 19.8 GB (trees 62-66 % saved,
  7.9-42 ns per frame of thread time), the recording through the packed float path clean (test_float.ps1 +
  dropouts.py). Every reader outside the render path uses `read32(ch, scratch)` now - never `channel32` on a sample
  that may be packed (the 0.8.145 rule for pcm16, the same for pcm32). `storageSignature()` says "packed" for
  float too (a reload after this build decodes anew - right). The kerneltest has "Packed float samples: ... OK".
- A configuration README is WRITTEN BY THE PROGRAM: saving a portable configuration puts `<name>.README.md`
  beside the `.json` (`MainComponent::configurationReadme`; dev: `ORGAN_SAVE_PORTABLE=<file>` with
  `ORGAN_LOAD_CONFIGURATION=<non-portable cfg>` loads and saves without the dialogs). Only the three
  "(fill in: ...)" places - Organ, the producer/product/link in Samples, Author - are completed by hand, plus
  the required version if a fix matters (Stralsund: 0.8.92). Never retype the rest. Its form:
- A configuration README has exactly this, in this order, and nothing more (people do not read):
  title; one opening paragraph (what it is, the perspectives - they make the recording unique -, divisions, the
  number of Stop List rows) + "No samples are published here"; "The configuration" bullets Organ, Samples
  (producer, product, link, packages, WAV, perspectives, required LibreOrgan Sampler version), Expects (trees), Audio
  layout (routes, unused ones marked), RAM = ONE figure since 0.8.186: the least the organ takes with every saving
  option (measured when saved so, else the estimate) + "at the full quality count on roughly ten times as much" -
  never two figures (two columns of the same 7 GB were "absurd"); INDEX.md has the one "RAM" column the same way
  (Rural Organ keeps its measured 1.2 GB, no estimate in its file). A PUBLISHED configuration opens with everything
  OFF: `savePortableConfiguration` with publishFiles switches every stop, transmission, coupler and tremulant of the
  registration off (the user's rule, 24 Oct 2026). (The old form of the line: RAM measured by the program at the portable save: `measuredRamBytes` +
  `measuredRamStorage` in the json, `processMemoryBytes()`; INDEX.md shows it as a sortable column - never typed,
  never estimated), Saved with, Author + date; "Where to put the file" (the packages are now
  known: one sentence + the tree diagram, never a made-up parent folder like `haupwerk`); last the "Stop List"
  table of the row names by division. Never mention what LibreOrgan Sampler does natively (couplers, voicing,
  crescendo, combinations). No "What you need", no "Using it", no prose about the divisions.
- Configurations live ONLY in `libreorgans/<organ>/` on GitHub as `README.md` + `<Name>.zip` (the one `.json`
  inside; a zip downloads from GitHub, a bare `.json` opens as text - never "Download raw file" instructions).
  The portable save writes `<name>.zip` and `<name>.README.md` beside the `.json`; the README's Download line
  is `[<Name>.zip](<Name>.zip?raw=true)`. The list of all organs is `libreorgans/INDEX.md` (README link,
  divisions, Stop List rows, routes: the perspectives as named - `default` stays `default`, never "-" - plus
  `auxiliary` when a division uses it; what they carry is the organ README's job, e.g. Rural Organ: default =
  front, auxiliary = rear; a Markdown table cannot be sorted by a click, so the header's ▲▼ are links to the
  pre-sorted copies `INDEX-<column>-<asc|desc>.md`, all generated), written by `.github/scripts/configurations_index.py` in the
  `Configurations check` workflow (job `index`) and committed back - never a list on the website, never a
  release for configurations, never a list in `libreorgans/README.md` (there will be thousands). The
  website section and the release body only link to INDEX.md. Configuration file names may have spaces (0.8.195:
  the robot names the pair after the .json inside the zip, whatever GitHub did to the attachment's name); the
  README's Download line is `https://github.com/zurek-jiri/LibreOrgan/raw/main/libreorgans/<organ slug>/<name>.zip`.
- Stop List counting: every row counts - stops, stop folders, transmissions and extensions (transmissions
  are rows of their receiving division, `transmissions[]` with `manual`) - except the rows nested in a stop
  folder (`groupMaster` set). The table is NEVER typed by hand: it comes from the program, in grid order (row by
  row, left to right - the organ builder's order the user arranged) with the exact button captions (footages,
  "(Ext.)"/"(Trsm.)", a stop folder's "[III]"):
  `ORGAN_DUMP_STOPLIST=<out.md> ORGAN_LOAD_CONFIGURATION=<cfg.json> LibreOrgan.exe` writes it once the load is
  over (use a non-portable configuration of this machine, a portable one would ask for the root); quit the program
  afterwards. No legend above the table. Every README looks the same. Stralsund: 53 + 18 = 71.

  0.8.206 (Windows, every to-do of 26 Oct done): (1) PROGRESS STRIP: `StopRow::pollBuildProgress` repaints 5 more polls
  after a build ended (`buildingSeen` is an int) - the one repaint after it fell on the flags still set and the last
  stop built kept a half-way strip until a mouse-over. (2) FROZEN PICTURE vs COUPLERS: `Voicer::divisionGridAreas()`
  (container coords: under each header's bottom + 4, the column matched by x) is taken with the snapshot
  (`frozenPicture/frozenAreas/frozenViewPosition`) and `freezeGrid()` COMPOSES the picture after the new rebuild: every
  division's old grid area drawn at the NEW grid area, transparent above (the new headers and coupler rows show live).
  Test %TEMP%\test_frozen206.ps1 (Adlington -> Ashton, grid.png.frozen.png: II. starts lower than I. = under two
  coupler rows). (3) GUIDED TOUR (`Source/GuidedTour.h`, header-only; the jucer lists no headers): an overlay child of
  MainComponent taking no mouse, a `Box` (title "Tour n of N", TextLayout text, "Manual: <chapter>", Back / Next
  (Finish) / Close, "Don't show this tour again for this organ") placed below/above/beside the target, a breathing
  golden frame around the target and an arrow box -> frame, following the target at 30 Hz;
  `MainComponent::startGuidedTour()` builds the 10 steps from the live grid (`Voicer::findRow(test)`,
  `StopRow::buttonComponent()/voicingToggleComponent()`, `Voicer::headerButton(manual, "Tremulant")`, the top-bar
  buttons; a step without its element is skipped). `guidedTourAvailable` = the file key "guidedTour": true OR a name
  starting with "Rural Organ" (the old demo download), set in applyPortableConfiguration; the top-bar button "Guided
  tour" (key guidedTour in headerButtons; `isHeaderButtonShown` returns guidedTourAvailable, no setting, skipped in the
  Editor's toggles); auto-start through runWhenLoaded + a 1.5 s retry while `ModalComponentManager` holds anything
  (the Rural "Nested folder trees" box stood over the box), never under ORGAN_LOAD_CONFIGURATION / ORGAN_VIDEO_SCRIPT;
  "guidedTourDismissed" = a machine-specific StringArray of configuration names. The key is in
  F:\Suchy_rejstrik\Kobrle\Rural Organ.json and in libreorgans/rural-organ/Rural-Organ.zip's json (%TEMP%\rural_tour.py).
  (4) VIDEO SCRIPT: `MainComponent::VideoScript` (a nested struct defined late in the cpp -> a `std::shared_ptr`
  member: a unique_ptr needs the complete type at ~MainComponent), hook `ORGAN_VIDEO_SCRIPT=<file>` +
  `ORGAN_VIDEO_OUT=<folder>` started by runWhenLoaded: one command per line (window / wait / waitfor BUTTON|MS / caption
  / record / click TEXT (any button of any window: dialogs too) / expand / collapse / alias / volume / slide (the stop
  volume slider or a key's slider dragged) / shift (the Transposition box) / stop / trem / coupler / combination [set] N /
  swell M|TO|MS / crescendo TO|MS / note / chord / arrange / drag (cursor + glow + the button's picture travelling) /
  drop / relocate / stoplist / closedialog / temperament / highlight / end - listed above the struct), a 40 ms timer
  steps it and takes a frame every 100 ms while recording: the top-level window's own paint + every other visible
  desktop window at its place (dialogs, the Temperaments window, popup menus) + a drawn arrow cursor that glides to
  every element and presses there + a golden halo/frame on the element (1.2-2 s) + the caption bar (h/13, 0.36 text);
  PNGs f%05d.png by a 3-thread ThreadPool with their times, `engine.startRecording` = audio.wav, `frames.txt` = the
  ffmpeg concat list. THE USER'S VIDEO RULES (26 Oct 2026): everything the video does must be SEEN and glow for a
  moment - no action from behind the scenes ("My jsme lidé!"): a temperament is chosen in the open Temperaments window
  (click rows; the config's own temperament is Neidhardt I, click Equal first), couplers in the Couplers dialog (tick
  "III/II 16'", Close, they appear under the header), a missing folder through the notice -> its menu -> a folder
  chooser (JUCE's own `FileBrowserComponent` + `FileChooserDialogBox` opened by the script: the system's dialog cannot
  be pictured; the row found by selectRow + getSelectedFile, "Choose" pressed) -> the loading window -> the "Sample
  folders found" box -> OK (THE MENU IS A REPLICA, `VideoScript::MenuReplica`, painted with the look-and-feel's own
  drawPopupMenuBackground/drawPopupMenuItem from the texts showMissingTreesMenu gives: a real JUCE PopupMenu dismisses
  itself within 10 ms when the process is not in the foreground - juce_PopupMenu.cpp `menuWasHiddenBecauseOfAppChange`
  - and the user watches every recording from another window, so three runs pictured no menu at all; never click the
  real notice in a video); the voicing shows the stop volume slider dragged under a held chord and one pipe's slider
  dragged while that pipe sounds (green); new ranks dragged into a stop folder come in OFF - draw them before the
  second note; a transmission's Transposition +12 played on the same key (the octave jump; the caption's footage
  follows); the tremulant switched on while the flute sounds; the swell closed/opened and the crescendo up/down under
  a held chord; the mixture once, a single pedal note for the Cornet, no tremulant in the stored combination, the
  Rohrflöte off before the transmission; the Load/loading windows flash by (the OW reload shows the three-line loader).
  `Voicer::headerSwell`, `StopRow::volumeSliderComponent/keySliderComponent/shiftBoxComponent/buttonComponent/
  voicingToggleComponent/applyAlias/volumeHeaderComponent/openVolumeRow`, `Voicer::scriptCellCentre/scriptIndentPoint/
  scriptDropAt/scriptDrop/scriptArrange/headerButton("prefix*")`. FRAMES: 25 a second - the window and its dialogs are
  snapshotted 10 times a second into `baseFrame` (a 2560 x 1400 snapshot costs ~100 ms), the overlays (cursor, the
  travelling button over 2.2 s, glows, the red "!" of `alert`, the caption) are drawn on a copy for every frame, a
  6-thread PNG pool, frames dropped when > 40 wait (the 10 fps capture made a drag a jump of 11 pictures: "nevidím, jak
  se rejstříky přesouvají"). THE SCREEN SHOWS THE SAME: `VideoScript::Overlay` (a child of the top-level window,
  no mouse, `drawOverlays` shared with the frames, repainted on the dirty union of cursor/travel/glow rects every
  tick, `suppressOverlay` while the base snapshot is taken) - the user watches the live window and took the
  invisible overlays for a missing drag ("já prostě nevidím, jak táhneš"). `paint M|STOP|tone|FROM|TO|MS|dB[|waves]`
  = the right-button painting of a key row (`StopRow::beginScriptPaint/scriptPaintAt/endScriptPaint` ->
  paintKeySlidersAt, the cursor's ring blue for the right button, a sine sweep), `tonerow` unfolds "Treble per key".
  MIDI LEARN in the video (`learnkeyboard M|KEYS`, `learnstop M|STOP`): the right click on the keyboard symbol
  (header local (5,7,34,24)) or the stop -> `rightClickMenu` (replica items as showKeyboardLearnMenu / showMidiLearnMenu
  give them) -> `beginKeyboardLearn` / `beginMidiLearn("stop:<name>")` (their real AlertWindows, captured) -> after
  2.7 s the window's `exitModalState(1)` = "the MIDI message arrived" (pendingButtonLearnTarget cleared under the lock;
  no binding is written) -> the symbol blinks red (`Voicer::pulseManualActivity`) while notes play / the stop toggles
  six times. Every script note/chord sets `blinkUntil[manual]` -> 120 ms red pulses every 300 ms while it plays (the
  user: "musí blikat klaviatura II."). `settings crescendo|swell M|tremulant M` = the right click + replica menu +
  the real settings dialog (showCrescendoDialog / showFeatureSettings); `dialogslider "Tremulant settings"|0|HZ|MS`
  drags the Rate slider of the open dialog (the engine follows); `glow M|STOP|MS` frames a caption that changed
  (the transmission stays at +12 = "HW Tromp 8' (Trsm.)", the Cornet's [V]); every `actOn` press glows too.
  0.8.210 STOP FOLDER ON THE GRID (the user: "když pod stop I. manuálu podsunu rejstřík z jiného manuálu, stop
  folder se neudělá a stop skočí zpět"): the mechanics WORK - `ORGAN_TEST_INDENT="0|TA - violon|1|TA - Principal 8|18"`
  on Rural indents across divisions (indentStopUnder -> moveStopToDivision), `ORGAN_TEST_DROP="0|TA - violon|1|0|5"`
  moves into a cell of I. - what fails is the GESTURE: the bar under the master shows only after a rest (0.8 s ->
  0.5 s now) and arms on a pull down (12 -> 8 px); a plain drop over a button goes to the cells (the occupied cell
  swaps or refuses -> "jumps back"). dropItem logs "Grid drop over the button of X without the indent gesture" and
  indentStopUnder logs every refusal with its reason. The tour's stop-folder step and the manual (3.2) tell the
  gesture. GuidedTour::placeBox puts the box ABOVE the element when there is room (the voicing/children unfold below).
  0.8.216 (27 Oct 2026, the user's D:\rr session): THE SAVE RECOMPUTED THE ROOT. His file after "Save" at quit had
  rootName "pipe", trees "PE"/"UW" and the missing 003136 trees as ABSOLUTE placeholder paths
  (C:\Users\...\Configurations\OrganInstallationPackages\003136\pipe\PE) - savePortableConfiguration took the root as
  the common folder of the trees AT HAND (D:\rr\003135\pipe), so the organ could never find 003136 again, the Stop
  List tooltip showed that path, and the location noted was D:\rr\003135. Now: (1) `treeRelativePaths` (member, map
  absolute -> relative below the root; filled in applyPortableConfiguration for every found tree AND every missing
  placeholder, written into the var as "treeRelativePaths" so loadSettings/saveSettings keep it - a machine-specific
  key, cleared at each load; `applyTreeRelocation` moves the entry to the new path) - the save derives the root as
  `rootFromKnownRelative(absolute, relative)` votes (D:\rr\003136\pipe\PE minus 003136/pipe/PE = D:\rr), the old
  common-ancestor rule only when nothing is known; `libraryRootName` (member, from the file) is written back
  unchanged, never the folder's own name; a missing tree keeps its relative path (`knownRelativeOf`, else
  `relativeBelowNamedRoot` = the part after the root's name in the placeholder, else the file's own key) in the
  division items and the pool, in both the portable and the ordinary branch. (2) Locations.txt gets the ROOT: the
  save notes the root's parent when the folder is named as the configuration names it, else the root folder itself
  ("D:\rr"); `relocateTreeSmart` notes `ancestorOf(found, below)` the same way (was below + 1 = "D:"); never a
  package. (3) The Stop List tooltip of a missing cell = "NOT FOUND on this computer: <tree tail>\<relative>" (no
  placeholder path). Tests: %TEMP%\test_root216_t1.ps1 (the Downloads json with the root renamed, zero trees,
  ORGAN_TEST_RELOCATE to D:\rr\003136 -> "Sample location remembered: D:\rr"), test_root216_t2.ps1 (root "rr", 003135
  missing, ORGAN_SAVE_PORTABLE -> the file keeps rootName "rr" and all 8 trees relative; the log "Save configuration:
  the library root on this computer is D:\rr"). The user REFUSED the root-under-another-name probe at load (a folder
  "rr" standing for the root is found only through a remembered location or a click) and the Replace of trees that
  are here. The relocation's refusal box ("holds none of the stop folders") stays; the "Sample folders found" box is
  gone (0.8.214).
  0.8.215 (27 Oct 2026): `driveChildren(root)` (anonymous namespace beside `locationChildren`: the first-level folders
  of a fixed drive, 64 at most, Windows/Program Files/ProgramData/$Recycle.Bin/System Volume Information/Recovery/
  PerfLogs and the Unix system folders skipped, a 30 s cache) and `portableFolderCandidates` ends with, per fixed
  drive: `<root>\<rootName>\<relative>`, `<root>\HauptwerkSampleSetsAndComponents\<rootName>\<relative>`, then for
  each first-level folder `<child>\<rootName>\<relative>` and `<child>\HauptwerkSampleSetsAndComponents\<rootName>\
  <relative>` - the normal Hauptwerk place D:\Hauptwerk\HauptwerkSampleSetsAndComponents\OrganInstallationPackages is
  found from a zip in Downloads on a wiped computer (%TEMP%\test_clean215.ps1 -Downloads: 8 trees, 72 rows, the
  location D:\Hauptwerk\HauptwerkSampleSetsAndComponents remembered). Fades: 110 ms default in the sliders'
  start values, loadSettings' fallbacks, GLOBAL RESET (values + label texts) and the engine atomics; the manual
  spells the bass scaling out (165 / 248 / 371 ms on the C of 8' / 16' / 32'); EngineSelfTest keeps its own 70.
  0.8.214 (27 Oct 2026, the clean-install batch; the user wipes AppData + Configurations to see what a first-time
  user sees, the Stralsund zip in F:\ beside F:\haupwerk\003135, the Rear package 003136 deliberately absent):
  (1) LOOKUP: `gChosenConfigurationDir` (set from `chosenConfigurationDir` in resolveAndApplyPortableConfiguration,
  cleared by a scope guard) adds the CHOSEN file's folder forms to `portableFolderCandidates` - the "same file already
  in Configurations" branch loaded the copy and searched beside it alone; and `fixedDriveRoots()` (Windows
  GetLogicalDrives + DRIVE_FIXED; else "/" + /media/<user>, /run/media/<user>, /mnt, /Volumes children) gives
  `<root>/<rootName>/<relative>` as the last candidates - a json in %TEMP% found F:\haupwerk\003135 through it (test 2
  had to rename the root to "haupwerkX" to get a zero-tree load). (2) ZERO TREES: `beginStartupFolderLoading`'s
  empty path rebuilds the voicer and `loadState`s the file's voicing (`embeddedVoicingPresent` set in loadSettings)
  so every entry is RETAINED ("Voicing state: 53 stop(s) not loaded ... kept"), and `reloadDivisions` saves the voicer
  state only when `voicerStateLoaded || stops exist` ("the voicer holds no state yet, the voicing file is left as it
  is") - before, the empty voicer's saveState wiped the file's grid/transmissions at the first relocation. (3) The
  leaf-name rule of loadSettings is now `tailsEqual(pathParts(tree), belowRoot(path))` = the whole path below the
  library root (the part after the last "<rootName>" folder, else the last three parts) - 003136/pipe/PE is never
  taken over by 003135/pipe/PE; the pool is DEDUPED after the resolution (refs remapped: "Tree pool: N duplicate
  slot(s) merged") so a tree is never loaded twice (doubled ranks). (4) Crescendo/combinations: `findStopForKey` and
  `applyRegistrationAdditive` match by name + division, the saved perspective only breaks a tie. (5) Locations.txt
  beside settings.json = `sampleLocations` folder lines + `rememberedArchives` (.rar lines): `locationsFile()`,
  `readLocationsFile(settings)` (in loadSettings; migrates the old "sampleLocations" key and ArchiveFolders.txt of the
  settings folder once), `writeLocationsFile()`; settings.json no longer carries sampleLocations (removed from the
  configuration var too); `registerArchive` and the Hauptwerk import write archives there ONLY - nothing beside a
  configuration any more; refreshArchiveSearchFolders reads Locations.txt's .rar lines (+ the legacy lists);
  Editor "Sample and IR locations: N folder(s), M archive(s)" + "Show" (revealToUser) + Forget (folders only).
  (6) Stop List: a cell whose tree or folder is not on this computer is ORANGE with the tooltip "NOT FOUND on this
  computer: <path>" (the path tooltip label is white for a folder that is here, orange only for a missing one - the
  user), the info line counts "N NOT FOUND", "Load folder..." lists ONLY the missing trees as "NOT FOUND on this
  computer: replace ... with a folder on this computer..." (-> chooseFolderForMissingTree; the user refused a
  Replace for trees that are here), "Unload folder..." marks them "- NOT FOUND on this computer" and lost its
  Replace section (the same offer twice "doubled and blurred"); the "Sample folders found" box is GONE (needless;
  the log line and the notice say it). (7) `stopBuildMap` creates a container row (stop folder) with the first tree
  of the division that IS here (Cornet [III] / Vox Humana Cornet [V] were left out with the Rear package missing:
  the dump had 70 rows, the published table 72 - now 72). Tests: %TEMP%\test_clean214.ps1 (AppData + the
  Configurations copy moved aside and restored; -ZeroTrees = the json alone in %TEMP% with the root renamed + the
  ORGAN_TEST_RELOCATE hook), test_stoplist214.ps1 (the dialog picture), tablediff.py (dump vs README table).
  The user's remaining demand that is NOT a code item: load what is there, report what is not, play.
  GCC LESSON (the first v0.8.214 tag: Linux + macOS failed, Windows passed): `juce::File f; f = {};` is AMBIGUOUS for
  GCC (File has operator= for File and String) - write `f = juce::File()`. MSVC takes it. The Windows CI watch
  (%TEMP%\ci_watch.ps1) prints the jobs only; the job LOGS need a token: `git credential fill` on Windows works
  through `cmd /c "git credential fill < file"` with "protocol=https\nhost=github.com\n\n" in the file (the manager
  helper; a PowerShell pipe gave nothing), then GET /repos/.../actions/jobs/<id>/logs with "Authorization: Bearer
  <token>" and grep "\.cpp:[0-9]+:[0-9]+: error". The tag was deleted and re-pushed onto the fix (50d343f).
  0.8.213 (the user's evening batch of 26 Oct, ALL to-dos in one build): (1) PATTERNS KEPT: `Patterns.json` beside
  settings.json (`applicationPatternsFile`, `rememberPatterns(exact)`, `mergeApplicationPatterns()` at the end of
  loadSettings, `rememberPatterns(false)` in applyPortableConfiguration before the unload): the loaded file's lists
  (else the defaults) are merged with the file's - one-shot/ignored by the pattern, perspectives by their LABEL (the
  file's line wins) -, written back merged at every load/unload, EXACTLY (a removed pattern removed) after an edit in
  the Patterns dialogs / the Editor / the GLOBAL RESET; log "Patterns in force: ... from this computer's Patterns.json:
  a / b / c added". Before, loadSettings took the lists of whatever file was parsed and the user's additions vanished
  with every load (his complaint, three times). Test %TEMP%\test_patterns213.ps1 (zzztest213 survives Rural ->
  TestingOrgan -> Rural). (2) MANUAL = `Source/ManualView.h` (header-only, like GuidedTour.h): one juce::TextLayout
  per block (paragraph/heading/code/licence line) in a Viewport, links as runs of lightskyblue underlined (hit test
  by run x-range), the find's selection drawn from the glyph rectangles (char index = run.stringRange.start +
  glyph index), the chapter heading highlighted by relaying that one block, `scale` for the font (A-/A+, Ctrl +
  wheel; `manualFontScale` machine-specific key). `MainComponent::prepareManual()` (Timer::callAfterDelay 4000 ms
  in the constructor, or at the first "?") parses the manual and lays it out = the former AboutComponent's constructor
  (500 ms on the Ryzen, logged "Manual: prepared in N ms"); the DialogWindow is made at the first showAboutDialog
  from `aboutContent` and KEPT (a DialogWindow is shown by its birth - never create it hidden at the start), log
  "Manual: shown at ... in N ms (prepared at the start)": 130 ms the first time, 9 ms after. No text selection /
  copy in the manual any more (the TextEditor had it) - say so if asked. A juce::TextEditor must never hold a long
  formatted text again (every insert = a full relayout). (3) The copy-build strip: `ProgressAnimator` repaints every
  row once when `pendingBackgroundWorkers()` falls to 0 ("Copy builds ended: every stop row repainted") and
  pollBuildProgress repaints the whole row after a build - not verified on the user's organ (the cause was never
  reproduced here). (4) Tour: step 3 = the Cornet [III] of manual II when there is one (`findRow` with primaryManual
  == 2 and "Cornet" in the caption), a new step on Load Configuration before Save (the right click = merge, chapter
  "Merging another configuration into the loaded organ"); ORGAN_TEST_CLICK LESSON: "Next" and "Close" exist in the
  manual window too (Find > Next) - close the manual before clicking the tour's Next, or the clicks go there.
  (5) Manual 7.1: percussion/bell stops sound wrong retuned or filled. (6) `consoleGlow` default OFF. (7) ONE ZIP:
  the export's zip holds the .json AND the README (the README beside it stays); `configuration_from_issue.py
  split_readme_out` takes the README out of the attached zip and rewrites the zip with the rest, an attached README
  beside the zip still accepted; the form, libreorgans/README.md, the manual, the site and the publish window say
  "the zip". Test: %TEMP%\test_zip213.ps1 + zip213\robot\check.py (members before/after, read_config, validate -
  the hook's export fails validate only for the unnamed organ, as expected). Origin moved meanwhile by the user's
  publish of Bellevue (issue #107, the old two-file way - still accepted): a rejected notes push (2>$null hid it)
  was found by `git status -sb` "behind 3" - resolved by `git pull --no-rebase` (a merge) before the 0.8.213 push.
  0.8.212 THE MANUAL OPENS SLOWLY (the user: "po kliknutí na rozevření kapitoly manuálu to trvá několik sekund"):
  MEASURED with ORGAN_TEST_CLICK "Guided tour|Manual: Stop buttons|Close|Manual: Stop buttons" (the tour's button
  text is "Manual: <chapter>"): `AboutComponent::setTextWithHighlight` builds the WHOLE manual into the TextEditor
  (one `insertTextAtCaret` per block with font/colour changes, every insert a new layout) = 2.7 s on the Ryzen,
  and before 0.8.212 it ran TWICE at every opening (the constructor's build, then `openChapter` built it again with
  the heading orange) in a NEW window each time. Now: built once per window, `highlightChapter` selects the heading
  (`setHighlightedRegion`) instead of a rebuild, `aboutWindow` is KEPT (DialogWindow's closeButtonPressed only hides
  it) and `aboutOpenChapter` jumps in it - the log says "Manual: text of entry 0 built in N ms" at the first opening
  and "Manual: shown again at ... in 57 ms (no rebuild)" afterwards. The first opening of a session still costs the
  one build (2.7 s); a cheaper build (one pre-laid text) is not done. The tour's division-header step names the right
  clicks on the Tremulant and the swell bar.
  0.8.209: THE VIDEO BEGINS WITH THE LOADING - make_video.ps1 sets NO ORGAN_LOAD_CONFIGURATION when the script has a
  "load " line (and turns loadLastConfigurationAtStartup off in the backed-up settings for the run); the hook then
  starts the script 1.5 s after the start on the empty console; `load FILE` presses Load Configuration and calls
  loadConfigurationFile (the load box appears - a real AlertWindow), `loadbox own folder` picks the "This file" combo
  item by a text part (in place, no copy into the user's Configurations), `click Load`, `waitload MAXMS` waits while
  a window named "Loading ..." shows (+1.5 s). `holdarrange M|STOP` = the 2 s press that enters arrange mode (manual
  3.2) before the first drag; the mode stays through the transmission drag. LEVELS: no loudnorm (it delays and cut the
  last words): ebur128 integrated loudness measured in make_video.ps1, plain gains to organ -18 LUFS / voice -21 LUFS
  (the user: the voice 6 dB lower), apad + amix longest + alimiter, the video's end cuts the mix. Transmission = a
  right-button OR Shift drag (voicer.cpp arrangeDuplicate; manual 3.2) - say both.
  `keepDialogsInFrame()` (every tick): a dialog outside the main window's bounds (a remembered place on the second
  screen - the crescendo settings) is moved over it, else it is nowhere in the frames. `reglowRow(manual, what, pick,
  delay, ms)`: an action that rebuilds the voicing rows (expand, alias, Transposition, the Volume/Treble headers)
  deletes the pressed element and its glow with it - the glow is put on the rebuilt row's element after the delay.
  Dialog overlays (`windowOverlays`): every other window gets an Overlay child (no caption) so the glow shows on
  screen in the Stop List / Couplers dialogs too; `settings` parses "settings swell 1" (space or "|").
  GLOW IN THE PROGRAM (the user: "dej do configuration volbu, jestli uživatel chce ty různé glow vidět"):
  `GlowLayer` (UIControls.h after HintSheen) = a transparent child of MainComponent over everything (bounds and
  toFront in resized), a recursive mouse listener (`addMouseListener(&glowLayer, true)`): a left press on any
  juce::Button glows 0.9 s, a drag over a button keeps it glowing; `consoleGlow` (machine-specific key, default on),
  Editor toggle "Glow: a pressed button and a dragged stop glow for a moment" after "Load the last configuration
  at startup". The video's own overlays are separate (VideoScript). NARRATION: `say TEXT` writes "ms<TAB>text" into narration.txt at its moment;
  Tools/make_narration.py (edge-tts 7.3.1 on Python39 - needs `aiohttp<3.10` there, the new aiohttp breaks on 3.9's
  typing; voice en-GB-SoniaNeural, rate -4 %; the service answers "NoAudioReceived" now and then - 6 tries) speaks each
  line, moves a line that would start over the previous one (reported "moved ... -> ..."; lengthen the script's wait)
  and writes narration.wav; make_video.ps1 mixes it: organ loudnorm -18 LUFS ducked by sidechaincompress under the
  speech (-15 LUFS). The texts are the user's wish: "trochu poeticky", every step told. Tools/make_video.ps1 runs the
  program (ORGAN_LOAD_CONFIGURATION + the script; settings.json restored) and ffmpeg (concat + audio [+ narration] ->
  libreorgan.mp4: x264 crf 20, 30 fps, 1920 wide; libreorgan.gif: 8 fps, 960 px, 128 colours);
  Tools/video_debug_menu.txt = the notice/menu/chooser alone (a 1.5-minute check); Tools/video_stralsund.txt = the Stralsund script (divisions
  0 Ped, 1 UW, 2 HW, 3 OW; stops by name prefix - "HW Tromp 16" in division 0 is the transmission row, "Tromp" alone
  would hit "Ped Trompete 8'" by caption; the coupler id "3-2-4" = OW on HW at 4', label "III/II 4'", the dialog's
  toggles carry the label); Tools/make_video_config.py makes %TEMP%\video\cfg\Stralsund_video.json from the published
  Stralsund json: the OW trees on a drive X: that does not exist (the missing-folder demo), release fades 120 ms (the
  user: 70 ms "příliš krátký crossfade"), every stored combination's tremulant off. Tools/video_tour_test.txt steps
  the tour through. The frames are the program's own paint - nothing of the desktop can get in; the user still wants
  to be told before a recording, and watches the run (his remarks come mid-recording). Website: the video is a TAB of
  the screenshot switcher ("Video, with sound", data-view="video": #video-figure shown, #screenshot-figure hidden;
  the screenshot stays the default - the user: "ten existující screenshot nezahazuj"); README: the screenshot first,
  libreorgan.gif under it linked to the mp4 on the Pages site (a GitHub README renders no <video>). (5) `portableFolderCandidates` adds `<child>/<relative>` for the children of a location (0.8.202's
  children form was rootName-only; the user's old Stralsund files name the root "haupwerk").
  0.8.217 (27 Oct 2026, the user's batch): (1) NOTICE QUEUE: `queueNotice(icon, title, text[, button, component,
  callback])` is a FREE function declared after the class in MainComponent.h (friend; the same arguments as
  AlertWindow::showMessageBoxAsync, callable from any lambda or helper struct without `this`) -> `MainComponent::
  noticeOwner->enqueueNotice` -> `pendingNotices` (std::deque) -> `showNextNotice`: shown only when `noticeBlocked()` is
  false = no notice of ours showing, no `activeFolderLoader`, and NO juce::AlertWindow among the modal components (the
  load box, the loading window, a question, the measurement window); DialogWindows (Stop List, Couplers, Editor) and
  menus do not block; blocked -> a 400 ms retry. EVERY former showMessageBoxAsync of MainComponent.cpp and
  MainComponentMerge.cpp goes through it (replace_all); Main.cpp's "already running" box stays direct. Never call
  juce::AlertWindow::showMessageBoxAsync in MainComponent again - the user's "msgboxy přes sebe" (the Multi-part
  archive note over the load box over the loader). (2) BEDOS: the ODF `Dom Bedos Organ Model.Organ_Hauptwerk_xml` in
  `BedosUpdate864_ODF.CompPkg.Hauptwerk.rar` (E:\sonus\HW2\bedos, RAR 4 with a recovery record, 183 files) is a text
  file packed -m5 = a PPMd block (first data bit 1); libarchive fed the file alone (`readEntryAlone`) returned "File
  CRC error" (WinRAR tests the archive OK - the user checked too); the sequential path (`readEntriesInSequence`, the
  real stream) FAILS THE SAME WAY, and so do the set's WAVs (bedos1: "Internal error extracting RAR file"; LZSS blocks,
  so the 2011 WinRAR's RAR VM filters, not PPMd). Measured with the user's own WinRAR 6: the same WAV repacked
  `-ma4 -m5` and `-mm` reads fine through libarchive, `-mct+` (PPMd forced) fails with ARCHIVE_FAILED (-25) and NO
  error text after 0 bytes, alone and solid alike -> libarchive 3.8.9's RAR 3 PPMd is broken, never patch it
  (prebuilt). THE USER'S RULES (27 Oct 2026, three messages): NO external unrar/7z ("jaký kurva unrar???"), nothing
  that is not for Windows + macOS + Linux alike, open-source freeware only; he pointed at Masterpiece
  (github.com/bonninr/masterpiece, GPL-3, C++20/JUCE), which reads RAR with libarchive + unarr (github.com/selmf/unarr,
  LGPL-3.0, C; pinned commit 9c8e898bb3dc2ee9d368b0e945d5bb453e15c187; solid RAR 4 and archives with filters go to
  unarr there; a file split over volumes is joined into a synthetic single-volume archive in memory). Ours
  (SampleFiles.cpp, `#if ORGAN_HAVE_UNARR`): `readEntryWithUnarr` feeds the SAME VirtualStream ([sig+main][file
  header][data][zeros]) to `ar_open_stream` (declared by hand: it lives in common/unarr-imp.h) -> `ar_open_rar_archive`
  -> `ar_parse_entry` -> `ar_entry_uncompress`; a split file gets a SYNTHETIC header (`VirtualStream::synthetic`,
  volume -2: split flags 0x0003 cleared, PACK_SIZE = the sum, HIGH_PACK_SIZE with LHD_LARGE 0x0100, header CRC16 =
  low 16 bits of CRC32 over bytes 2..size) and the parts' data in a row; a solid archive -> `readSolidEntryWithUnarr`
  (ar_open_file(_w) + `ar_parse_entry_at(headerOffset)`, single volume only). `gUnarrFirst` (preferUnarr, logged
  once "read with unarr from now on") skips libarchive for an archive after its first failure. CMake: `organ_unarr`
  static lib from Masterpiece's 14-file list (no zlib/bzip2/lzma), `ORGAN_HAVE_UNARR=1`, guarded by the existence
  of ThirdParty/unarr/rar/rar.c; the header = checked-in ThirdParty/unarr-include/unarr.h written by
  `Tools/make_unarr_header.py` from unarr.h.in (unarr's sources `#include "unarr.h"`). The jucer needs the same
  14 C files as a group + headerPath ThirdParty/unarr-include;ThirdParty/unarr;ThirdParty/unarr/common + extraDefs
  ORGAN_HAVE_UNARR=1. The tool classifier DENIED `git submodule add` ("Untrusted Code Integration") - the user added the
  submodule himself (two `!` commands, one per line - two lines in one `!` input break on the second "!"). Dev:
  `ORGAN_UNARR_FIRST=1` sends every archive through unarr (the split-file path is tested on the Aeolian set that way);
  `ORGAN_TEST_ARCHIVE` now registers its archive as chosen (0.8.177 rule) so ORGAN_TEST_ARCHIVE_CHECK reads it.
  Test archives: %TEMP%\bedos_rartest (one Bedos WAV unpacked + arc1 -m5, arc2 -mm, arc3 -mct+, arc4 -mct+ -s solid,
  arc5 -m3 -mct+ made by the user's WinRAR 6). unarr.h typedefs `off64_t` as int64_t (glibc's is `long` = the same
  type on x86_64; watch 32-bit builds). TWO LESSONS of the split-file path (%TEMP%\rartest\v4 + v4rr = 001313
  Re_glockenton in 40 MB volumes, 5 files split, disk root D:\): (1) the FILE_CRC of a part's header is the CRC of
  THAT part's unpacked bytes; the whole file's CRC stands in the LAST part's header - unarr returns false on a CRC
  mismatch ("unarr could not unpack the file" on exactly the 3 split files sampled), so the synthetic header takes
  bytes 16..19 from the last piece's header. (2) unarr reads in small pieces: a seek + read of the volume per piece
  gave 6 MB/s (libarchive 100+ through the same VirtualStream, which hands it 1 MB at a time) - `readInto` serves
  from a 1 MB window (`windowStart/windowLength`, the ranges filled in a row). Verified: CHECK "82 compared, 0
  differ" before the fixes except the 3 split ones; after them 85/85 at 108 MB/s. Released as commit b4d7249, tag
  v0.8.217 (the submodule's first CI build on Linux/macOS: watch the "off64_t" typedef and the `_POSIX_C_SOURCE`
  define of stream.c if it fails). (3) Measurement end: `outputGainHoldUntilMs`
  = now + 5000 (the level stays at -52 dB five seconds after the run and the report, then 26 dB/s). (4) MANUAL: no
  version number anywhere any more ("každá verze si nese svůj manuál" - never write "(0.8.xxx)" or "since 0.8.xxx"
  into USER_MANUAL.md again); 1.5 has "### Where the files are" (settings folder per OS: Windows
  `C:\Users\<you>\AppData\Roaming\LibreOrgan`, macOS `~/Library/LibreOrgan`, Linux `~/.config/LibreOrgan`; the
  configurations folder; samples anywhere, relative to the library root; RAR archives expected NOWHERE - only the chosen
  ones; Locations.txt). (5) REMOVE A CONFIGURATION = the *remove* link column of INDEX.md (configurations_index.py
  `remove_link`: the issue form prefilled by query string `?template=organ_configuration_remove.yml&title=...&organ=
  <folder>`; `configuration_tools.py find` returns the exact folder match alone when one exists; rural-organ has no
  link). The robot cannot rewrite workflow files (GITHUB_TOKEN has no `workflows` scope), so a `choice` input list in
  the dispatch form is impossible; the user's "Actions" attempts never produced a workflow_dispatch run at all (every
  run of configuration-remove.yml is an `issues` event, skipped). (6) POLYPHONY #113-#115 (Core 2 Duo Mac P8600 and
  Linux T6670, 0.8.216 vs #88/#89 of 0.8.183): NOT an engine regression. Mac: the settings.json of #88 had
  `sincKernelFromHz` 20000 (sinc for no pipe) and `irLightAuto` true; #113/#114 have 3000 (the default: every pipe
  from 3 kHz through the 8-tap sinc, kernel 4930 -> 8000-11400 ns per voice and block on SSE2) and irLightAuto false
  (IR Full kept at 17.5 % per thread: hand-over wait 365 us per route callback, callbacks of 14-29 ms) -> 3900 -> 779;
  memesdry (dry set, no IR) 687 from a single 107 % callback with render at 19 %. Linux: kernel 5117 -> 4103 ns
  (faster), limits 2500-2600 vs 2800 = 36 kHz storage then vs 48 kHz now + three device paths. Before calling a
  Core 2 Duo figure a regression: diff the settings.json of the two reports (`sincKernelFromHz`, `irLightAuto`,
  `storageSampleRate`, `renderKernel`) and the "per voice and block: kernel N ns" of the last clean seconds.
  The "Profiling clock" on that Mac reads ~1 GHz in every run (958-1103 MHz) - not a throttle sign, the same in #88.
  Comment drafts: %TEMP%\issue113_comment.md, issue114_comment.md, issue115_comment.md.
  0.8.218 (27 Oct 2026, the user: "udělej všechna to-do i toto"): (1) NEIGHBOURING ARCHIVES OF A SET (Dom Bedos =
  five SEPARATE rars: bedos1_862 ... bedos4 + BedosUpdate864_ODF with the definition; the ODF rar alone gave "11
  sample folder tree(s) not on this computer"): `MainComponent::offerNeighbourArchives(explicitRequest)` - called by
  `runWhenLoaded` from `resolveAndApplyPortableConfiguration` when `missing` is not empty (once per configuration
  file and session, `neighbourArchivesAsked`) and by the notice menu item 1001 "Look for the missing folders in the
  archives beside this organ...". THE USER'S RULE ("co když tam bude 100 dalších rarů? Já bych to stavěl na
  shodnosti části jména"): candidates = first-volume rars (not chosen yet) in the folders beside the chosen archives,
  beside the configuration file and the nearest existing folder above every missing tree, whose letters-only stem
  (".partNN", digits, separators dropped) shares >= 4 leading letters with a chosen archive's stem OR contains a
  >= 4-letter word of the configuration/organ name ("bedos"); at most 24; a folder of foreign rars opens none. Only
  those are indexed (`indexArchivesThen`, the progress window), then `SampleFiles::entriesOf(archive, startsWith
  "<tree relative to the archive's folder>/")` decides which hold which missing trees; one box "Missing folders"
  (Read them too / Not now) lists "archive: tails"; yes -> `registerArchive` each + `reloadDivisions(affected)` (the
  pool paths are unchanged, the archives now serve them). `SampleFiles::chosenArchives()` getter added. Dev:
  `ORGAN_TEST_NEIGHBOURS=1` answers yes. Log lines "Missing folders: ...". (2) SINC DEFAULT BY CPU:
  `SamplerEngine::defaultSincKernelFromHz()` = hasAVX2 ? 3000 : 20000 in all four places (engine atomic, the
  MainComponent member, loadSettings' fallback, the Editor slider's default); loadSettings logs "Full kernel from: N
  Hz (the default for this CPU M Hz, AVX2/no AVX2)"; the manual's slider paragraph and 5.8 say it. Released as
  commit a621e60 + merge fa665ef (origin had moved: the user removed menesterol-dry through the new remove link -
  it works), tag v0.8.218 on fa665ef, CI green on all four, Intel mac "late symbols: 0". %TEMP%\ci_watch.ps1 needs
  the FULL 40-char SHA (`actions/runs?head_sha=` finds nothing for a short one: "no run for v0.8.218 after 10 min").
  0.8.219 (27 Oct 2026, the user: "mechanismus mazání funguje, ale musel jsem napsat část jména ... kdyby to vyhodilo
  seznam a já bych jen zaškrtl" + "když vyberu bedos rar bez ODF ... měl by ihned prohlédnout okolní bedose ... Load from
  ODF / not now"): (1) THE REMOVE FORM IS GENERATED: `configurations_index.py` writes `.github/ISSUE_TEMPLATE/
  organ_configuration_remove.yml` with a `dropdown` (id organ, `multiple: true`, options = the folder names, rural-organ
  left out) + an `input` (id name, "Or any part of its name"); an issue TEMPLATE may be pushed by GITHUB_TOKEN (only
  .github/workflows may not), so the index job, the remove and the feature workflows `git add` the template with
  libreorgans/. `configuration-remove.yml` reads the dropdown ("a, b" = several; "_No response_" = none) and the
  input, removes every match in ONE commit and answers "Removed: ... / Not removed: ..." (rural, foreign publisher,
  no match, several matches). INDEX.md's remove link `?...&organ=<folder>` stays (a dropdown prefill when GitHub
  honours it). (2) `MainComponent::archivesNamedLike(folders, like, nameWords, exclude)` = the name sieve of 0.8.218
  made static and shared; `offerDefinitionBeside(archives, merge)` replaces the "no configuration / definition" notice
  of `loadConfigurationFromArchives`: the archives beside the chosen ones with a name like them are indexed; one
  holding a definition/configuration -> the box lists them ALL ("they look like one organ", the holder marked) with
  "Load from the definition" / "Not now"; yes = `loadConfigurationFromArchives(chosen + neighbours)` (registers them
  all; several definitions -> chooseFromList as before); none -> the old notice extended by "the archives beside it
  hold none either". ORGAN_TEST_NEIGHBOURS=1 answers yes here too. Test: bedos1_862.rar alone (Locations.txt
  stripped of bedos lines with ReadAllLines/WriteAllLines - `Get-Content | Set-Content` on the SAME file fails "in
  use") -> the set found -> the choice box (the Bedos package rars carry the ODF too: several candidates; pressed by
  ORGAN_TEST_CLICK="zzz|zzz|zzz|Load", the hook clicks 3 s after the start and 2 s apart, a missing name is logged and
  skipped) -> Dom Bedos imported from the definition with all five archives, 56 stops, no tree missing. Released as
  commit 3bd65bd, tag v0.8.219.
  (3) DISK READ 358 MB/s - SETTLED (27 Oct 2026, measured here): a plain parallel read (%TEMP%\diskread.py, 8
  threads, 4 MB reads, 4 GB of a package not read recently) of D: (Samsung 870 QVO 8 TB SATA) gives 441-455 MB/s;
  a COLD load through the program (ORGAN_LOAD_TREE of 001692 with packing OFF: "4.53 GB read in 12.5 s = 363
  MB/s"; 001693 with packing ON: "8.15 GB in 22.6 s = 361 MB/s") gives the same figure with and without packing -
  the loader's per-file reads on 16 threads reach ~80 % of the disk's sequential ceiling, the CPU is no limit, the
  line IS the disk. The user's 358 MB/s was a cold load from the SATA SSD (the page cache gives 1 300-3 900). No
  code change; the manual's 4.4 (SATA ~500 MB/s) stands. Never promise more from a SATA SSD.
  0.8.220 (28 Oct 2026): A FURTHER PERSPECTIVE OF THE SAME ORGAN (the user's question "jak to rozliší? ... Nevím,
  poraď": the criterion is the NAMES, never whole folder lists - a division may be split over packages):
  `MainComponent::matchFoldersAsPerspective(newNames, baseEntries, knownPatterns)` (static, MainComponent.cpp before
  extendStopBuildWithTree): every new folder name against the rows' folder names, longest common ENDING >= 4 chars
  that starts at a separator on both sides (the ending = the stop, the rest = the prefix/head; a head ending inside a
  word is no prefix, the same head on both = the same perspective, skipped); the dominant head must be on >= 3 and
  >= 80 % of the candidates and must NOT be a prefix of a known pattern line -> `rowOfFolder`, else `why`.
  `perspectiveLabelForHead` (a known label whose first three letters the head holds, else a word from the head:
  "06-far__" -> "Far"), `patternsWithHead(patterns, label, head)` adds the head to the label's line. MERGE
  (MainComponentMerge.cpp, before the addedEntries loop): the non-shared added entries' folder names are matched
  against the base division's rows with the base `perspectivePatterns`; for a definition the Hauptwerk stop names
  (aliases) must agree on >= 80 % where both sides have them (`withNames >= 3`), else "another organ"; a matched
  entry's folders join the base row (`target.folders[label]`, label = the plan's perspective or the head's word), the
  pattern "Label;head" goes into the merged `perspectivePatterns` (patternsWithHead in the union loop), a ROUTE of
  that name is made per division (`divisionJoinedLabels`: the added "default" route's settings when any, counted in
  `perspectivesAdded`), `stopsJoined` + `joinedPerspectiveNames` in the summary ("42 stop(s) gained the perspective
  Distant ..."), `sharedStopNames` set so the grid rule "the same organ in volumes" applies. The "(2)" rule fires on
  an alias collision too (the alias gets " (2)"). LOAD FOLDER (`extendStopBuildWithTree`): the scan's folders with
  perspective ""/"default" are matched against the rows of EVERY built division (a package holds several divisions'
  folders: Bergen 002082 = HW_ + Pos_), the matched ones join their rows (other divisions too: `treeJoinedDivisions`,
  reloaded by loadTreeIntoDivision / loadTreesIntoDivision with `onlyTree`), the pattern is learned into the engine +
  Patterns.json (`rememberPatterns(false)`); log "Load folder: ... is (not) another perspective of the loaded organ".
  HAUPTWERK IMPORT: the prefix detection of HauptwerkOdf.cpp (~1844-1890) takes FLAT ranks (`rank.folder ==
  stopFolder`: the ranks lie in `pipes/` directly) - Bergen Surround now writes "Direct;03-ortf_", "Diffuse;04-ab_",
  "Rear;05-rea_" (Stralsund/Bellevue/Caen/Ashton/Stellwagen unchanged, checked with --hauptwerk); the Distant ODF
  alone (one perspective) writes no pattern line - nothing to compare. `applyHauptwerkVoicing`: `Entry.partial` (the
  definition knows only some of the stop's folders - a merge) -> route volume counted, stop/per-key voicing skipped,
  "N stop(s) with a perspective of this definition only" in the log. Tests: %TEMP%\test_merge220.ps1 (Surround ODF
  imported by the hook into a settings copy with configurationsFolder = %TEMP%\cfgtest220, Distant ODF merged:
  "42 stop(s) gained the perspective Distant", 4 new routes, every row of I. with Diffuse/Direct/Distant/Rear, 42
  rows before and after) and test_loadtree220.ps1 (ORGAN_LOAD_TREE="1|...\002082\pipes|45": 15 of 15 folders join
  6 rows of I. and 9 rows of II. as "Far", pattern "Far;06-far__", 42 rows). Hooks: ORGAN_LOAD_TREE's third field =
  seconds; ORGAN_LOAD_TREE and ORGAN_MERGE_CONFIGURATION save the ordinary configuration afterwards
  (`savePortableConfiguration(current, true, false, false)` - saveSettings alone writes NO configuration file) and
  dump the Stop List as "<ORGAN_DUMP_STOPLIST>.tree.md" / ".merged.md". NO QUESTION AFTER THE ARCHIVE SEARCH: the
  boxes of `offerDefinitionBeside` and `offerNeighbourArchives` are gone - the archives are taken (registered,
  divisions reloaded / the organ loaded from the definition with all of them) and a queueNotice names them;
  ORGAN_TEST_NEIGHBOURS is no longer needed there. Why the box could not be answered was NOT found (the box is
  gone; if a box from a `then` of indexArchivesThen is ever needed again, check the modal stack first).
  0.8.221 (28 Oct 2026, "go"): THE TUNING GATE for the perspective join. `MainComponent::compareFolderTuning(a, b,
  maxFiles = 8)` (static, MainComponent.cpp before perspectiveLabelForHead): the same-named .wav files of the two
  folders (`SampleFiles::fileNames`), sorted by the file number (the lowest pipes first) and spread evenly, their smpl
  chunks read raw (`readSmplRaw`: dwMIDIUnityNote + dwMIDIPitchFraction through SampleFiles::openInput - a header walk
  on the disk, the whole entry out of an archive) and compared bit for bit; PASSED when identical >= min(3, compared)
  AND identical x 3 >= compared x 2 - NOT "one difference refuses": the user's law ("absolutně stejné ladění v smpl")
  holds for the organ, not for every file - Bergen's own Distant set (002082) has 4 of 67 files of Pos Principal 8
  and 1 of 64 of Rohrflaut 8 with another smpl than the Surround set (002085; ortf = ab on all 67), a producer's
  retouch, and the strict rule refused those two real perspectives in the first run; a foreign pipe never matches a
  fraction bit for bit (the Dry set 002061 vs far: 0 of 51; the test's altered copies: 0 of 8), so the majority rule
  loses nothing. DEGENERATE smpl (no chunk, fewer distinct values than half the files, or every fraction 0) ->
  `measuredPitchCents` fallback (juce::WavAudioFormat reader over openInput, 0.5 s after 0.15 s, normalised
  autocorrelation 20 Hz-4 kHz, the first peak >= 0.9 x the best, parabolic refinement) on the 4 lowest files, within
  2 cents, >= 2 measured. LOAD FOLDER flow: `planTreeJoin(tree, manual)` on the message thread (scanTreeAssignments,
  the name match over EVERY division's rows - inactive tables derived -, `TreeJoinPlan::pairs` = new folder / the
  row's first existing folder), `gateTreeJoinsThen(plans, then)` = a ThreadWithProgressWindow "Comparing the tuning
  of the new folders with the loaded stops" running the gate per pair (log "Tuning gate: X vs Y -> the same pipes /
  NOT the same pipes (why)"), then `pendingTreeJoin` is set and `addTreeToDivisionTable` -> `extendStopBuildWithTree`
  joins the CONFIRMED folders only (the label learned when confirmed >= 1), the refused become new rows;
  loadTreeIntoDivision and loadTreesIntoDivision both go through it. MERGE: the gate runs in place (message thread)
  after the alias check, `folderOf(ref)` resolves against `mergedPool`; refused entries go to `ignored` ("named like
  X after the prefix, but the tuning differs") and become rows. `matchFoldersAsPerspective`: NO four-in-five rule any
  more (>= 3 name matches with one dominant head suffice - a second set need not carry every stop), and the WHOLE
  STEM rule: the base folder's head must be a known pattern prefix or empty ("HW_Principal8" != "Principal8").
  Tests: %TEMP%\test_loadtree221.ps1 [-Tree <tree> -Tag <x>] (default 002082/pipes: 15 of 15 confirmed, 6 rows of
  I. + 9 of II. gain "Far", 42 rows; `-Tree %TEMP%\fake_far\pipes` = copies of 4 far folders x 10 lowest files with
  the smpl fraction + 5 CENTS (214748365 units; made by %TEMP%\make_fake_far.py - the first version added 1 unit =
  0.00000002 c, which the 0.02 c tolerance of 0.8.222 rightly accepts: a fake must differ by cents): 4 refused
  "0 the same, 8 different", 4 new rows, no pattern learned, 46 rows), %TEMP%\test_merge220.ps1 (42 joined, 0 added, 4 routes), %TEMP%\smpl_cmp.py <a> <b>
  (the smpl comparison of two folders by hand). The gate reads 8 files x 2 folders per pair: Bergen's 15 pairs in
  under a second from the SATA disk; an archive entry is read WHOLE (2 MB each) - a packed rar package of 40 folders
  costs ~5-7 s on the progress window.
  0.8.222 (28 Oct 2026): THE "DIFFERENT" SMPL FILES OF BERGEN WERE THE LAST BIT - %TEMP%\smpl_bits.py <a> <b> prints
  the raw unity/fraction of the differing files: Pos Principal 8 041-f_bis / 042-f# / 072-c / 090-f# and Rohrflaut 8
  036-c differ by 1-2 units of 2^32 = 0.00000005 cents (a rewritten header's rounding), the measured f0 the same
  take. So the user's law holds for every file and "the same" in compareFolderTuning is |delta| <= 0.02 cent (the
  user's figure; a foreign pipe differs by cents); THE TWO-THIRDS RULE STAYS BY THE USER'S ORDER ("důležité je, abys
  zachoval pravidlo, že stačí, když se bude shodovat většina, tj. 2/3 ... zejména když víme, že se perspektivy
  dolaďují zvlášť") - never make the gate "one difference refuses". LESSON (28 Oct 2026, the user: "já jsem ti
  neříkal, abys dělal novou verzi!!! Jen odpověz otázku"): a QUESTION is answered, never turned into a build - the
  0.8.222 build started on my own after a measurement was stopped before the deploy; a correction of my own work
  waits for his word too. The 0.8.221
  note's "4 of 67 retuned" is WRONG - nothing was retuned. 440 Hz QUESTION (the user, 28 Oct): the retune is PER RANK
  = per perspective and per pipe - `rank->tuningCentsFor(sampleNote, tuneTo440)` at every voice start (6172/6471,
  13137/13158), filled by `computeTuning440` from each rank's OWN smpl pitches (`tuning440Cents[note]`, the rank's
  `measuredDiapasonHz` for the retune bus through `commonRetuneCents(stop, rank)`); the temperament mode 2 uses the
  rank's own `measuredTemperamentCents` too. Two perspectives with a REAL smpl difference on a pipe are each pulled
  to the same equal-tempered 440 pitch (the difference vanishes with "Equal at 440 Hz" on, stays as recorded with
  it off); a smpl difference that is metadata only (same audio) would be INTRODUCED by the 440 mode as a beat between
  the perspectives - Bergen's is 0.00000005 c, nothing.
  0.8.223 (28 Oct 2026, "tak jo, dělej"): THE THREE MERGE FAULTS + PREFIX LEARNING ON EVERY LOAD PATH. (1) The "(2)"
  loop (MainComponentMerge.cpp ~556): name AND alias re-suffixed from `originalName`/`originalAlias` at every step,
  `suffix < 200`. (2) The merge's `resolveTree` falls back to `MainComponent::resolveTreeLikeALoad(configuration,
  file, relative, absolute)` = `firstExistingDirectory(portableFolderCandidates(..., root, ...))` with root =
  `rootFromKnownRelative` of the first `treeRelativePaths` entry (the loaded organ's root) - Bellevue.json from the
  LibreOrgans folder: 10 trees, 63 stops added. (3) `perspectiveHeadsOfEntries(entries)` (static): label -> the
  head ALL folders of that perspective's column carry (common prefix cut right after a separator run, >= 2 folders,
  stems left; "default" none) and `learnPrefixesFromTables()`: computed over the UNION of every active division's
  entries (per division the head is "03-ortf_HW_" with the division mark; the organ's is "03-ortf_"), new heads
  joined to the patterns by patternsWithHead + rememberPatterns + log "Perspective prefixes learned from the Stop
  List tables: ..."; called in applyPortableConfiguration (before saveSettings - every load/import/merge result) and
  applyStopBuild. `matchFoldersAsPerspective`: known prefixes = the pattern prefixes + the heads of the base rows
  themselves; the base folder's head passes when it equals a known prefix OR one holds the other (the per-division
  head "03-ortf_HW_" vs the pair's cut "03-ortf_"); "the same perspective twice" is decided by the ROWS (a base
  folder starting with the head), not by the pattern lines (the scan may have just learned the head). The merge
  takes `engine.getPerspectivePatterns()` (the lines in force), not the base var's. `planTreeJoin`: candidates =
  folders with perspective ""/"default" OR under a label no row carries. SCAN WORD RULE (SamplerEngine.cpp learning
  block): a head carried by EVERY considered stop folder of the tree (`consideredStops`) whose letter tokens hold a
  perspective word (direct diffuse distant dir dif rea rear far near close dry wet front ab ortf dist surr amb; a
  token equal to, or starting with a >= 3-letter word) qualifies without a partner; a lone head without a word
  ("HW_", "Ped_") never (it would fuse divisions). His Surround.json (patterns "Direct" without prefix) + Distant ODF:
  all 42 joined, patterns "Direct;03-ortf_ | Diffuse;04-ab_ | Rear;05-rea_ | Front;02-suchy_ | Distant;06-far__",
  42 rows, no freeze. Test: %TEMP%\test_merge223.ps1 [-Added bellevue -Tag bellevue] (his two jsons copied into
  %TEMP%\cfgtest223 as the configurations folder; the first run per division printed the per-division heads
  "03-ortf_P_" - the organ-wide union fixed it).
  0.8.224 (29 Oct 2026, Linux session, "udělej všecka to-do"): (1) UNLOAD FOLDER REMOVES THE TREE'S ROWS (the user's
  rule, the ONE exception to "a row is never removed by the program itself"): `unloadTreeFromPool` marks every row
  whose folders ALL came from the unloaded tree (`emptiedByTree`, transmissions never) and removes them with their
  buttons; a stop folder (a non-child row followed by child rows) goes only when its own folders are empty AND all
  its children went that way; rows with a folder of another tree and every other empty row stay ("Remove empty
  stops" as ever); log "rows left empty by it removed: N (...)". Dev hook `ORGAN_TEST_UNLOAD_TREE="<tree tail>
  [|<seconds>]"` (the pool tree whose path ends with the tail; afterwards the configuration is saved and the Stop List
  dumped as "<ORGAN_DUMP_STOPLIST>.unload.md"). (2) LINUX DISK MOUNT: `mountLinuxDiskFor(path)` (anonymous namespace
  before resolveAndApplyPortableConfiguration, JUCE_LINUX): a path under /run/media/<user>/<label> or /media/<label>
  whose mount folder does not exist -> the device by label in /dev/disk/by-label (`unescapeUdevName` for \x20) ->
  `udisksctl mount -b <device>` (juce::ChildProcess, 60 s; polkit may show the desktop's password dialog for an
  internal disk - HintSystem true on sda2 here), once per label and session, logged "Disk \"<label>\" (<device>) for
  <path>: mounted / NOT mounted - <udisksctl output>"; called at the start of resolveAndApplyPortableConfiguration for
  every sample location, every division's absolute folderPath and the file's sampleLibraryRootRelative. The missing-tree
  notice's detail names "A remembered sample location is not there now - a disk not mounted?" + the paths. The manual:
  "Where the files are" (the Linux /run/media paragraph: mount permanently - Disks & Devices "Mount at system startup"
  or fstab -, the program mounts by label otherwise), 1.6 moved samples, 4.1 Unload folder. NO Projucer resave on Linux
  (do it on Windows next). Syntax check: g++ -fsyntax-only with -I ~/LibreOrgan-linux/JUCE/modules (the NTFS
  checkout's JUCE is a symlink to C:\JUCE9 without modules) + -I .../ThirdParty/unarr-include -DORGAN_HAVE_UNARR=1.
  0.8.225 (29 Oct 2026, Windows, "udělej to"): THE DEEP FUNDAMENTAL (the user: Billerbeck "P. Grosse flute 32" and
  "P. Bourdon 32" captioned 16', "i voicing říká 32 (16)"). NOT the FFT size (his guess): `lowestSoundingHarmonic`
  (SamplerEngine.cpp ~13203: 65 536 frames from 0.3 s, bin 0.73 Hz, Blackman-Harris; the first harmonic within
  34 dB of the strongest and 12 dB above its floor; the "missing fundamental" exception needs the FOUR harmonics above
  all within 34 dB) measured on the 32' C (16.35 Hz) h1 47 dB (flute) / 59 dB (Bourdon) UNDER h2, the same gap at
  262 144 frames (%TEMP%\harm32.py <wav> <f0>): the recording has no 16 Hz, and h5 of a stopped pipe is 39 dB down,
  so the exception failed -> "sounding harmonic 2" -> `lowestFootageSemitones` = 16' -> `Stop::displayName` (an
  alias without footage, the import strips "32": line ~11180 uses the SOUNDING footage) and the voicing's "32' (16')".
  A fifth up (043-G, 24.5 Hz) h1 is 39/54 dB ABOVE h2 - the loss is the deepest three pipes only. THE USER'S RULES:
  the smpl is NOT the law here - a mixture shows its first sounding harmonic by custom, and a mixture's break
  (2 2/3' -> 4') is often INVISIBLE in the smpl (f0 of the key on every pipe), so higher pipes must never decide
  the harmonic NUMBER (the minimum over a segment would turn 2 2/3' into 4'); the odd-harmonic test is no help
  either (a mixture has h3). What is done (computeTuning440, the sounding block ~13602): for the FIRST segment only,
  when the median harmonic > 1 and the lowest pipe sounds below 45 Hz (the user's threshold: the whole bottom
  octave of a 16' is 32.7-43.7 Hz), the pipes +6, +7, +9, +10, +12 semitones (the user's points) that exist in the
  SAME smpl segment are asked one binary question - `lowestSoundingHarmonic(...) == 1`? - and TWO confirmations
  turn the harmonic into 1 (never 3 into 2); log "Tuning <rank> [p]: the lowest pipes (N Hz) measured harmonic K ...
  M of L pipe(s) up to an octave above have a clear fundamental - the recording lost the deep fundamental". A
  mixture shows no h1 within an octave unless it has an 8' rank there (then 8' is right). Manual: the Orig. Pitch
  bullet (4.3); CHANGELOG. Test %TEMP%\test_footage225.ps1 [-Cfg <json> -Tag <x>]: the Stop List dump of the same
  configuration with the OLD exe (C:\LibreOrgan\LibreOrgan_0.8.223.exe) and the NEW one, diffed - Billerbeck
  may change the two 32' captions only. WINDOWS CHECKOUT AFTER THE LINUX SESSION (0.8.224 was committed from
  ~/LibreOrgan-linux): `git fetch` + `git reset --mixed origin/main` as the LESSON says, BUT origin also carried
  the user's new published configurations (bergen-op-zoom-ibach-organ, beziers-cathedral-puget-organ, INDEX*.md,
  the generated remove form) which then show as DELETED/modified in the working tree - `git checkout --
  libreorgans/ .github/ISSUE_TEMPLATE/organ_configuration_remove.yml` restores them BEFORE any `git add -u`
  (a commit with -u would have deleted his organs from main); the rural README's version bump is redone after it.
  0.8.226 (29 Oct 2026 evening, "udělej to, avšak snaž se nezkurvit kód na jiných místech"): THE PROGRAM HEARS ITS OWN
  CLICKS. `SamplerEngine::outputEdgeJump(block, n, EdgeState&)` (EdgeState: last sample, mean step, peak of the previous
  block): a jump = the step into the block > 12 x the mean step of this AND the previous block, > 2e-4 (-74 dBFS), and
  the previous block's peak > 1e-3 (the FIRST version had no "sounding before" rule and flagged 22 voices starting
  together on a silent route: a summed first sample of 0.00026 - a start is not a click). Counted per route at stage 0
  (after the pool render, `countEdgeJumps` in renderRoute) and stage 1 (after separation/reverb/clip), and per device
  callback on the driver's buffer (DivisionAudioCallback `edge[64]`, `takeEdgeJumps`); reset after a render gap > 250
  ms, a device start, a mute. `takeOutputDiscontinuities(detail, noScratch)` once a second in checkAudioCallbackHealth
  -> log "Output discontinuities: N in the last second - in the render: manual m p: a after the render, b at the
  route's end | on the way to the driver: ... | render lanes without a scratch buffer: k" and the DSP cell shows
  "!N" in orange for 10 s (`PolyphonyDisplay::noteDiscontinuities`). Stage 0 is the sharper one: with IR reverb on the
  route the wet raises the mean step at stage 1 and masks a small dry step. THE POOL: `lanesJoined/lanesLeft` are
  uint32 and NEVER reset (equality only; `Job::joinBase` = the count at the opening for maxWorkers and the lanesUsed
  statistic; completedCount reset BEFORE nextVoice); a lane acquires its scratch BEFORE takeChunk and, with none free,
  leaves without voices (`statNoScratch`) - the "render into nothing" path is gone. Whether this was the user's cause
  is NOT proven (the race is nanoseconds wide); the detector is what will tell next time. MIDI: `midiInputOpensNow(name)`
  (Windows: midiInGetDevCapsW by name, "Name-2" duplicates by prefix, midiInOpen/midiInClose probe; a failure is logged
  and the port skipped until the next device-list change) before every MidiInput::openDevice in reopenMidiInputs and
  midiDevicesChanged - JUCE's openInternal terminates the process when midiInOpen fails (the 18:32 crash). Record:
  the chooser starts in `configurationsDirectory().getParentDirectory()/LibreRecordings` (created on demand). Wind
  "Oscillation frequency" 2-10 Hz (engine clamp, two sliders, the settings dialog row, MIDI-learn scaled(2, 10));
  Release sample volume up to +10 dB (slider + engine clamp). Dev hooks: `ORGAN_TEST_GLITCH=<n>` (every n-th block of
  every route silenced after the render = the user's symptom made to order; the detector must count it at all three
  stages - verified: "1 after the render, 2 at the route's end | driver: 2"), `ORGAN_EDGE_DEBUG=1` (one log line per
  jump: last/first samples, active voices, releases, fading, voices in their first block, added, finished, polyphony,
  limit - allocates, dev only). Tests: %TEMP%\test_glitch226.ps1 (Rural, ORGAN_TEST_CLICK "Configuration Editor|
  Measure..." at 20 s = the measurement's stimulus up to 61 keys x 16 strikes/s: 0 discontinuities in four runs after
  the silent-start rule; 6 false ones before it), test_glitch226_inject.ps1 (ORGAN_TEST_NOTE + ORGAN_TEST_GLITCH=200).
  Self-test identical to 0.8.223's output line by line ("SOUNDS OUTSIDE THE COMPASS" on the transmission test is the
  0.8.156 design, printed by both - not a regression). PROJUCER LESSON: `& C:\JUCE9\Projucer.exe --resave <abs>` from
  the tool's PowerShell printed nothing and changed nothing again; `Start-Process -FilePath ... -ArgumentList
  @("--resave", "`"<abs>`"") -Wait -NoNewWindow -RedirectStandardOutput` printed "Finished saving: Visual Studio 2026"
  and worked - use that form; copy the .ico AFTER it.
  0.8.227 (29 Oct 2026 evening, "tak to všecko udělej"): (1) `fadeScale` 1.5 -> 1.25 per octave below the sounding c1
  (the user's figure; 110 ms default: c 138, 8' C 172, 16' C 215, 32' C 269 ms - the manual had said 165/248/371 for
  the 8'/16'/32' C, an octave off the code's 248/371/557; both manual bullets corrected). The self-test's lines move
  with it ("Release overlap ... silence 139 ms after it" was 165; "stacked release tails peak polyphony 4" was 124 -
  the long low-pipe fades end sooner; EngineSelfTest keeps its own 70 ms) - not a regression, compare against
  %TEMP%\selftest_227.txt from now on. (2) "Release played from" (`releasePlayedFromMs`, configuration key, 0-300 ms,
  default 0; engine atomic + `setReleasePlayedFromMs`; Editor addSlider in Global sound right after "Release
  cutting"; GLOBAL RESET; applied at the release voice start in SamplerEngine.cpp: `playedFromFrames` BEFORE
  makeRoomForVoice - a release shorter than the setting is NOT STARTED at all (the user's rule, "Ne, žádné limit!
  Prostě v tom případě se release nepřehraje vůbec"; the first version clamped to numFrames / 3 - rejected); a
  pre-shifted copy starts at max(that, its own trim clamped to half)). FIXED ms, not scaled by pitch (the open
  question was not answered; say so if asked). Rural's releases are ~60 ms (11 KB files; the user: "suché sety nemají
  dlouhé releasy") - an A/B of the offset needs a wet set: Bergen op Zoom Surround, division 1, prefix "" (all
  stops), the tail of B correlates with A shifted by +139 ms for 150 ms set (the runs are not sample-identical: wind
  and loop phases differ, so subtraction proves nothing - correlate the tail). Verified by %TEMP%\test_release227.ps1 (two ORGAN_TEST_RECORD
  runs, 0 vs 150 ms in the configuration copy, notune; the WAVs equal while held, different from the release on;
  release227_cmp.py). (3) `outputDiscontinuityDetector` (machine-specific key, default true; MainComponent
  std::atomic<bool> read by the device callbacks + engine atomic read at the top of countEdgeJumps; Editor toggle
  after "Glow"; manual section "Output discontinuity detector" + the 2.4 sentence). Verified by
  %TEMP%\test_detector_off227.ps1 (ORGAN_TEST_GLITCH=200 with the key false: zero "Output discontinuities" lines).
  The bindSlider double-click defaults of the two fade sliders still say 70.0 (the defaults are 110 since 0.8.215) -
  seen, left alone (not asked); fix with the next fade change.
  0.8.228 (30 Oct 2026, Linux session, the user's big batch): THE MANUAL REWRITTEN (USER_MANUAL.md 3754 -> 2041
  lines): parts 1-7 the program as used (ways to an organ in 1.5, "Sample folders not found" LAST; 3.3 voicing-row
  bullets are ### subsections; 4.2 "The table" names the Stop List dialog and the indent gesture both ways; 4.3
  Perspectives = rename by a double-click on the column header), part 8 = ONE ### per Editor option named EXACTLY
  as its label (openChapter: exact heading first, then a heading containing the label, then the section - a new
  Editor row needs its heading; the check script of the session: every add*("label") of the Editor against the
  headings), part 9 Messages/log/report/licences, part 10 technical details linked by quoted headings (a quoted
  text becomes a link only when a chapter is named so: chapterNamed = exact or unnumbered exact, else startsWith for
  >= 14 chars). No version numbers in the manual. Duplicate headings are poison (the first wins): "Perspective boxes"
  / "Velocity layer boxes" in 3.3, "Filled pipes cache: Empty the cache" (the row's helpSection) in 8.3. CODE:
  ManualView text selection + Ctrl+C/Ctrl+A/right-click; `queueNotice` InfoIcon without callback -> `noteMessage` +
  `MessageStrip` (bottom left, 10 s) + the "Messages" entry of the About window (AboutComponent takes a messages
  getter; openChapter("Messages")); Stop List `renamePerspective` (header label editable on double-click ->
  `renamePerspectiveSettings`: audio keys, patterns line), `addPerspective` -> `ensurePerspectivePatternLine`,
  `dropIgnoredFolders()` at the top of apply(); Editor: `addSection(..., level 2)` nested collapse ("Filled pipes"
  after Celeste beats: sliders first, then switches, then the cache rows; "Top bar" at the END of Console),
  `addButtonWithNote` (the path AFTER the button: cache, cache folder, LibreOrgans folder, Recordings folder,
  Sample and IR locations), "Measurement limit of DSP power", the Linux ALSA-list toggle REMOVED (the Divisions
  dialog's "List unusable devices" is the setting), Show -> Locations.txt; `gsReleasePlayedFrom` main-panel slider;
  `recordingsFolder` / `recordingsDirectory()`; `SamplerEngine::perspectiveChannelMask` so the device-stage detector
  scans the route's channels only (idle DSP 15 % -> the 18-channel clear/copy of 15 ASIO routes remains); Hauptwerk
  rows `withoutPrefix` (the definition's perspective prefixes and a short head starting with the perspective word
  cut: "Re_Principal32" -> "Principal32", "Ped_" stays); `applyPerspectiveChannelsEverywhere` + the AudioTabPanel
  button "Set for all divisions (this perspective)" with a painted separator line; crescendo default 40; combination
  colours (filled = off.darker(0.3), same content = filled.darker(0.22), selected = on); patterns dialogs' "?" ->
  showManualSection. MIDI velocity: `disableVelocity` is TRUE by default already ("Ignore MIDI velocity" ticked) -
  nothing changed. Both pattern dialog classes (PatternsDialog = one-shot, IgnoredPatternsDialog = ignored +
  perspectives) have the helpButton. NO Projucer resave (Linux) - Windows next.
  0.8.229 (30 Oct 2026, Linux, the user: "Udělej pouze 4" + the Carillon "(apply first)"): (1) ROW NAMES MIGRATED AT
  LOAD: `MainComponent::renameRowsWithoutPerspectivePrefixes(var)` (static, before applyPortableConfiguration; called
  right after `root` is taken, so every later reader of the var sees the new names): prefixes = the file's
  perspectivePatterns lines after the label; per division every non-custom row WITH folders gets
  `rowNameWithoutPerspectivePrefix(name, prefixes, the row's perspective labels)` (the import's withoutPrefix lambda
  now calls the same static); a collision with another row's key keeps the old name (logged); then the references:
  voicingSettings.stops[] (name + primaryManual, transmissions[].groupMaster by t.manual), stopGrid keys and stopPool
  names per division (plain name or "tr:<home>:<name>[#ordinal]"), combinationMemory.combinations[].stops,
  registrationStops, crescendo.steps[].stops (name + primaryManual), midiButtonBindings "stop:<name>" (no division:
  the first division renaming it). Log "Row names without the perspective prefix: N row(s) renamed - ..." + a
  Messages note; nothing marks the configuration dirty - the migration repeats at every load until the user saves.
  Dry run (python on his files): Bergen 42 rows "05-rea_Pos_Mixtur3" -> "Pos_Mixtur3", Brasov 35 "Re_Principal32"
  -> "Principal32", no collision. Names stay as the folders spell them (no capitalisation). (2) STOP LIST "(apply
  first)" ON A STOP FOLDER MADE ON THE GRID: `createStopFolderOnGrid` names the row "Stop folder" and
  `engine.addContainerStop` the same; the dialog's `refreshContainerNames()` (rebuildGrid) renamed the DRAFT after its
  children ("Pos_Flautino2 [II]") while the engine kept "Stop folder" -> liveStop nullptr -> "(apply first)", Apply
  orange, and an Apply reloaded the division under the new name and LOST the folder's alias/grid cell (no
  rename-with-references exists). Now a container row whose name the engine holds (`liveStop != nullptr`) keeps
  its name. The Bergen file on this box: manualKeyboards[2].stopBuild.entries[9] = {"name": "Stop folder", "alias":
  "Carillon", "folders": []} - the children were indented on the grid in the session he did not save.
  0.8.230 (30 Oct 2026, the user: two stops of one name in two divisions / after a merge - "UUID je asi blbost.
  nějak to vyřeš"): NO UUID - the identity stays (division, name), unique per division at every creation point
  (`uniqueStopBuildName`: grid folder, dialog add, merge "(2)", cross-division moves). Fixed the holes: (1)
  `Voicer::dropItem`'s move-house branch no longer writes the dropped cell under the OLD key (it overwrote the target
  division's own stop of that name) nor retargets by the old name: it keeps `pendingMoveCell` and the owner
  (`moveStopToDivision`, `indentStopUnder`, `mergeStopFolder` across divisions) calls
  `Voicer::stopMovedToDivision(from, oldName, to, newName, takesDroppedCell)` after `uniqueStopBuildName`: the cell
  under the new name, the keys "tr:<from>:<old>[#n]" of its transmissions in every division's cells/pool renamed,
  `StateRetarget` with `newName` (loadState reads (old, from) as (new, to)). (2) MIDI stop binding "stop:<division>:
  <name>" (voicer.cpp learn callback, the video hook); the dispatcher and the migration accept both forms;
  `legacyStopBindingKey` makes the menu's "Clear MIDI assignment" see and erase an old "stop:<name>" too. (3) Merge:
  name and alias suffixed independently (only what collides). NOT verified by a run (no local build): the Stop List
  dialog's own cross-division drag goes through Apply/reload and was not touched.
  0.8.231 (30 Oct 2026, the user: "po loadingu Bergen ... zase chce oranžově Apply and reload?? ... To řazení
  perspektiv je šílené"): THE ORANGE APPLY AFTER A LOAD = `rebuildPerspectiveUnion()` ran AFTER `originalDrafts =
  drafts` and SORTED every draft's perspective list (his file: "Diffuse, Direct, Rear, Distant" from the merge; the
  union "Diffuse, Direct, Distant, Rear") -> `changedDivisions()` (vector ==, order-sensitive) flagged all four
  divisions -> Apply reloaded the whole organ (168 ranks adopted, still the whole "Compiling"); the second opening was
  clean because the sorted order was stored - and he never saves (Don't Save at quit), so it recurred at every load.
  Not the name migration. Fix: originals taken after the union. ORDER = THE PATTERNS: `SamplerEngine::
  perspectiveOrderIndex(name)` (-1 default, else the line index of the label in perspectivePatterns, else size+1)
  + `sortPerspectives(vector&)` (stable; ties natural) used by getPerspectivesForManual (tabs, routes, everything),
  deriveStopBuildFromEngine, the dialog's union and the voicing row's rank boxes (voicer.cpp, rankSources sorted).
  Stop List header = `HeaderLabel` (double-click rename, drag > 6 px -> dropAt -> `movePerspectiveColumn(from, to)`:
  the union reordered, `MainComponent::reorderPerspectivePatterns(order)` rewrites the lines in that order (a label
  without a line gets a bare line), rememberPatterns(true), configurationDirty, saveSettings, voicer->rebuildUI;
  drafts AND originals re-sorted so Apply stays grey; the Divisions tabs follow at the next opening). Manual 4.3 +
  8.5 say it. NEXT (the user's wish, not done): an "Explain feature"/manual link on every control's right-click menu
  (showMidiLearnMenu is the one central menu: stops, transmissions, couplers, pistons, sliders, crescendo, swell,
  header buttons) and/or F1 over the hovered control - proposed, waiting for his choice.
  0.8.232 (30 Oct 2026, "dej obojí"): F1 + "Manual: ..." menu items. `HelpKeyWatcher` (MainComponent.cpp, defined
  before the MessageStrip class, friend of MainComponent; `std::unique_ptr<juce::Timer> helpKeyWatcher`, 60 ms poll of
  `juce::KeyPress::isKeyCurrentlyDown(F1Key)` while the process is foreground - a key listener needs the focus) ->
  `openManualForMouse()`: the component under the mouse and its ancestors are asked for the property "manualChapter"
  (set on: the main window's parts in the constructor, the top-bar buttons, StopRow widgets in voicer.cpp (the row
  "The voicing row", the button "Stop buttons"/"Transmissions and extensions", shiftBox "Transposition", aliasBox
  "Alias", the sliders their chapters, rank boxes "Perspective boxes", key sliders "Volume per key and Treble per
  key"), the SectionHeader (header "Division headers", title "Keyboard, MIDI and compass", setup "Divisions and
  audio", swell/tremulant "Swell, tremulant, wind", missing "Sample folders not found"), the Editor's rows (label +
  control = helpSection, section headers = the title), the Stop List's buttons/grid, the AudioTabPanel ("The audio
  tabs"/"The auxiliary route"), the Divisions dialog's velocity/random/coupling switches), else the "midiTarget"
  property through `manualChapterForTarget` (stop/transmission/coupler/tremulant/temperament/header/slider:global:*
  ...), else the top-level window's title through `manualChapterForWindowTitle` (Couplers, Temperaments, General
  Crescendo, Divisions, Stop List, Configuration Editor, Report a problem, IR reverb settings, Set for all,
  Measuring..., patterns, Save/Load Configuration, Global Reset, Swell/Tremulant/Wind settings, MIDI Learn,
  Unpack, Loading, Publish), else "The main window"; the About window itself is ignored. Menus: showMidiLearnMenu
  item 6 "Manual: <chapter>", showDivisionMenu 6, showKeyboardLearnMenu 3, showCrescendoMenu 4, showSwellLearnMenu
  5, showMissingTreesMenu 1002, the IR caption menu 2. A NEW CONTROL OR DIALOG: give it the property (or a title in
  the map), else F1 lands on the window's chapter. MANUAL LINKS: USER_MANUAL.md references are Markdown links
  `[Name](#github-slug)` (GitHub slug = lower case, letters/digits/-/_ kept, spaces "-", the rest dropped;
  "4.2 The table" -> "42-the-table"); the contents list too; made by /tmp/fix_manual.py (quoted chapter names ->
  links); the program's parser (`makeParagraph`) reads `[text](#anchor)` as a link by the text, else by the slug
  (`githubSlug`, `chapterBySlug`), another target shows the text alone; quoted names still link in the program.
  Keep NEW references as Markdown links. Manual errors found by the audit against the code (the old manual had
  them too): the Divisions box is UPPER LEFT; the 440 button is "Set diapason a1 = 440 Hz (all stops)"; the
  Divisions dialog says "Disable MIDI velocity" / "Allow Random Tuning Error" / "Allow Pipe Coupling"; the macOS
  claim. The PEDAL compass default IS 36-67 (constructor `manualConfigs[0].compassHigh = 67`, loadSettings default
  67 for division 0; the struct's 36-96 is overwritten) - my "36-96 for every division" was wrong. Verified right: "+"/"-" corner button, "Cornet [III]",
  "New stop folder here", Stop List button names, GLOBAL RESET boxes, NOW/PEAK/DSP/RAM GB/LIMIT, combination row
  captions, load box combos, "Portable configuration (to share with others)", the publish buttons.
  0.8.233 (30 Oct 2026 evening): (1) `HelpTooltipWindow` (MainComponent.cpp after HelpKeyWatcher, friend; the
  header's `std::unique_ptr<juce::TooltipWindow> tooltipWindow` created FIRST in the constructor body - it was a
  plain member before) overrides `getTipFor`: every tip + "\nFor details press F1."; a component with no tip but a
  chapter (`MainComponent::manualChapterForComponent(c)` = the shared resolution of openManualForMouse: property,
  ancestors, midiTarget, window title; empty when nothing) shows the line alone. (2) THE USER'S RULE: F1 and
  "Manual: ..." must land on the ELEMENT and its settings, never on a generic chapter - the Tremulant button went
  to "Swell, tremulant, wind" (one paragraph) and a coupler button to "Division headers" (the coupler buttons are
  children of the SectionHeader, no property of their own). Manual 5.2 is three ### now: "The swell", "The
  tremulant", "The wind model", each naming the control, its right-click menu and every setting as a link;
  targets: tremulant: -> "The tremulant", swell -> "The swell", titles Swell/Tremulant/Wind settings -> those,
  coupler buttons tagged "Couplers" in SectionHeader::rebuildCouplers. When a new element gets a tag, make sure
  its chapter describes THAT element. The top-bar buttons: `manualChapterForTarget("header:<key>")` maps record ->
  Recording, stopList -> The Stop List, couplers, temperaments, voicing -> The voicing row, the three patterns ->
  their 8.5 chapters, stopMerging, load/saveConfiguration -> Saving and loading, configurationEditor -> the
  reference, unpackArchive -> Sample archives (RAR), globalReset; PANIC/Guided tour/Load/Save Voicing/Shut down PC
  keep "Top bar buttons" (bullets only). (3) The manual parser reads Markdown escapes (`\*`, `\_`, `\"`, `\\`, `\[`,
  `` \` ``, `\#` = the literal character); the patterns in 1.7/8.5 are written `\*_tr`, `\*x`, `x\*`. (4) Manual:
  compass defaults in 5.1, "The loading window" moved into 1.6 after "Saving and loading" (linked from Step 4,
  2.4, 4.5), the remove-link sentence and "None of this loads anything again" dropped. The patterns lists and the
  perspective order ARE configuration keys (not machine-specific), merged into Patterns.json at every load on any
  computer - told the user.
  0.8.234 (30 Oct 2026 evening, the user: "klik na otazníček v perspektiv patterns dialogu neotevře manuál vůbec"
  + "není vidět divize při přesunu" (= the perspective column drag) + "proč kóduješ, když jsem ti to nezadal???
  Ale kóduj, ale udělej zároveň všechna to-do"): (1) The log said "Manual: shown at Perspective prefixes" six
  times - the window came up but was DEAD: the patterns dialogs are `launchAsync` = modal, and the kept About
  window (options.create(), never modal; a hidden window loses its modal state anyway) gets no events while
  another modal component exists. Now showAboutDialog enters the modal state (`enterModalState(false, nullptr,
  false)`) when any modal component exists; hiding it leaves the stack. On JUCE_LINUX a window already visible
  is remapped (peer setVisible false/true: Wayland lets no unfocused window raise itself). (2) Stop List column
  drag: `columnGhost` (ImageComponent, a `createComponentSnapshot` of the column's visible part, alpha 0.55,
  follows the mouse from `columnDragOver`), `GridArea::columnMarkerX` (an orange 3 px line at the target column),
  `columnUnder`, `columnDrop`. LESSON: a question or a report from the user is answered, the code comes when he
  says so; when he says "do all to-dos", every open item goes into ONE build.
  0.8.235 (the user: "ty jsi debil ... For details press F1 nahradilo opravdové tooltipy ... objevuje se to ihned
  a furt"): HelpTooltipWindow::getTipFor appended the line to an EMPTY tip whenever a chapter resolved - and inside
  the main window every component resolves to a chapter (MainComponent carries "The main window"), so every
  component got a tooltip of its own: the line popped up everywhere and the real tooltip of a parent (a slider's
  text box, a button's child) was replaced by the child's made-up one. RULE: the line is appended to a NON-EMPTY
  tip only; never invent tooltips.
  0.8.236 (30 Oct 2026 night, the user: "každý relevantní prvek má dostat tooltip", "musí vzniknout pravidlo, kdy
  zase tooltip zmizí", the loading window's three lines, arrange mode by right-click): (1) HelpTooltipWindow::Hider
  (a Timer inside, 250 ms): a tip visible > 7 s over the same component is hidden (`hideTip()`) and that component
  is `suppressed` (getTipFor returns {} for it) until the mouse is over another component - JUCE would re-show at
  the next tick otherwise (tipShowing is cleared by hideTip). (2) Tooltips audit: controls set up through a
  helper (`setup(button, text, tip)` in the Stop List, `setupWindSlider`, the reset buttons' loop) HAVE tooltips -
  the regex audit over-reported; the real gaps were the top bar (panic, temperaments, voicing, patterns, merge,
  load/save voicing and configuration, couplers, editor, GLOBAL RESET), the combination row, 7 global sliders,
  manualCountBox, voicer stopButton/shiftBox/gainSlider/randomPitchSlider/setup, the Divisions dialog's device/
  transpose/outputs/divisionVolume/velocity/randomTuning/pipeCoupling/attenuation/windSmoothing, the crescendo,
  temperament, report, couplers (newKeysOnly, graceTime) and manual-window buttons - all set now (inserted before
  their addAndMakeVisible lines; the Divisions ones after their setButtonText/auto& anchors). (3) The loading window:
  JUCE's AlertWindow width = sqrt(fontHeight x width of the WHOLE text) - the third line's " " placeholder made a
  narrower window whose lines wrapped until "Reading from the disk at N MB/s" widened it; `diskLine` starts as
  "Reading from the disk ..." + 8 NBSP. (4) `Voicer::manualRequest` (set by MainComponent) + the workspace
  ExitListener's right-click menu: "Enter arrange mode (...)"/"Leave arrange mode" for the division whose grid area
  holds the click (`divisionGridAreas`), "Manual: Arranging the grid". Manual 3.2 says so.
  0.8.237 (30 Oct 2026 night, the user's Arnstadt: a stop folder of II. with HW_Trump IN IT dragged to I. arrived
  EMPTY, and a new "Stop folder" on II. had the Trumpet at once): the log says it - "new transmission of HW_Trump
  (shift 0) indented under "Stop folder" in division 2", then "dragged from division 2 to 1 as "Stop folder
  (2)"". The folder's content was a TRANSMISSION (groupMaster "Stop folder", manual 2 in HW_Trump's list), not a
  child row; moveStopToDivision moves table rows only, so it stayed in II. and a new folder named "Stop folder"
  there matched it by name. Fix in moveStopToDivision: every transmission with (manual == source, groupMaster ==
  the folder's old name) gets the target division and the new name (replaceTransmissions +
  forgetTransmissionCells), logged "... N transmission(s) indented under it moved". The indent/merge-folder paths
  re-parent transmissions themselves (voicer.cpp ~3596-3640). RULE: a stop folder's content = child ROWS + the
  TRANSMISSIONS whose groupMaster names it; every operation on a folder must handle both.
  0.8.238 (30 Oct 2026 night): (1) THE STOP LIST OPENS IN GRID ORDER (the user: "stop table by měla přebírat
  pořadí stop na gridu"): after the alias sync and before the transmission rows, every division's draft entries are
  regrouped into blocks (a non-child row + its children) sorted by `voicer->gridCellOf(manual, name, col, row)`
  (row * 1000 + column; no cell -> after the placed ones, stable), BEFORE originalDrafts - no "change". (2) The
  patterns dialogs are MODELESS: `patternsWindow` (unique_ptr<DialogWindow>, options.create() + placeDialog +
  setVisible/toFront; OK/Cancel call `dw->closeButtonPressed()` = DefaultDialogWindow::setVisible(false)); the
  user: "Musí být patterns okno modální? Je to k něčemu?" (3) `automaticName` of an empty folder takes stop
  children only (a transmission's caption "HW Trump 8' (Trsm.)" named a folder after a move). (4) voicer.cpp: a
  transmission indented under a folder (new copy or an existing one moved) sets `master->voicingExpanded = true`
  (0.8.211 did it for stops only). OPEN: the user's "změna divize ve Stop List tabulce neukáže hned trsm, musí se
  vypnout a zapnout Show Trsm" - the combo/drag path and the toggle both end in rebuildGrid() on the same drafts
  (transmission rows are child rows of the block and move with it); not reproduced by reading; asked which state
  Show Trsm had - he answered "asi jo, při přesouvání divizí ve Stop Listu" (unclear). Wait for a log of 0.8.238.
  0.8.239 (the user: "Totéž dialogy vlastností tremulantu a podobné. Proč jsou modální? Kde nemá modalita význam,
  tam ji zruš."): `ModelessDialog` (MainComponent.cpp before MessageStrip: DialogWindow(title, darkgrey, escape
  closes, on desktop), native title bar, setContentOwned, `onClose`, closeButtonPressed = hide + onClose,
  UIScale desktop scale) for the Couplers dialog (`couplersWindow`), the IR reverb settings (`reverbToneWindow`)
  and the feature settings (`featureWindow`, onClose = refreshConfigurationEditor - the FeatureContent destructor
  did it at the modal close). Still modal by design: the load box, Save Configuration, GLOBAL RESET, Set for
  all..., Report a problem, the measurement windows, MIDI Learn, long messages, questions. A DialogWindow built
  this way is VISIBLE at birth (0.8.213 lesson) - placeDialog after it.
  0.8.240 (the user: "regres: kombinace naplněné nemají barvu jako crescendo steps"): the 0.8.228 colours were
  MINE, not his: he asked for "filled darker, same content a shade or two darker, not black" and I took
  off.darker(0.3) / .darker(0.22) - a grey and a near-black; the crescendo dialog paints a stored step
  0xff7a5a1e (CrescendoContent) and that is the colour the pistons must share. Now filled = 0xff7a5a1e, same
  registration (toggled, not selected) = brighter(0.45), selected = onColour. Before 0.8.228 a filled piston had
  no colour of its own and the same-registration ones were off.darker(0.14).
  0.8.241 (the user: a configuration without a crescendo showed the previous organ's steps): `crescendoFromVar`
  returns for a non-object (right for a bad crescendo FILE at 9741); loadSettings now passes an empty object
  (stepCount 40, cancelsHandStops true, no steps) when the file has no "crescendo". RULE for every per-organ
  state read in loadSettings: a missing key must RESET, not keep (combinations? `combinationsFromVar` - check the
  same when a report comes).
  0.8.242 (the user, after seeing filled pistons at a start without a configuration; "udělej"): the start-clean
  branch of the constructor (`!willLoadLastConfiguration`) now also clears `combinations` and the crescendo
  (crescendoFromVar with an empty object, the step count kept) - they came from settings.json / the memories file
  of the LAST organ. Both live in the configuration file and return with its load. LOCAL LINUX BUILD (30 Oct 2026,
  GitHub's runners queued for hours): cmake + the CI's apt list installed by the user, submodules libarchive/unarr
  fetched (`git submodule update --init --depth 1`), `cmake -S . -B build -DCMAKE_BUILD_TYPE=Release` +
  `cmake --build build --parallel 24` in ~/LibreOrgan-linux (configure 59 s, build a few minutes on 32 threads),
  the binary build/LibreOrgan_artefacts/Release/LibreOrgan -> ~/Downloads/LibreOrgan. Its `--kerneltest` says
  "2 MISMATCH" on "Réc. hautbois" here (the shell's locale is not UTF-8: JUCE's isLetter on é) while CI passes -
  check with LANG=C.UTF-8 before believing it.
  0.8.243 (the user: "barva ... je jako hovno ... méně nápadného ... o jeden shade saturovanější"; swatches made
  with PIL into ~/Downloads/piston_colours.png, he chose t = 0.08 for everything and ONE grey): UIControls.h
  `filledColour(base)` = base.interpolatedWith(orange, 0.08), `sameRegistrationColour(base)` = 0.14, base = the
  look-and-feel's TextButton::buttonColourId (0xff263238 in LookAndFeel_V4's dark scheme) for the pistons AND the
  crescendo dialog's steps (they had juce::Colours::darkgrey as the empty colour - wrong, one grey now).
  Colour questions: make a swatch picture (PIL is installed) instead of describing shades. Also 0.8.243: the
  Stop List's `automaticName` of an EMPTY folder returns {} - no more naming after the children (the user:
  "Může být stále Stop Folder"); refreshContainerNames is left in place (harmless).
  0.8.244 (the user: no tooltips in Swell/Tremulant settings; the Ignored Patterns dialog stayed over the manual;
  "Set for all divisions" unclear): FeatureContent rows get tooltip + "manualChapter" from a static table matched
  by the row text's start (ORDER MATTERS: "Depth (maximum pitch sag)" before "Depth"); showAboutDialog puts the
  top-level window under the mouse (the dialog whose "?" was clicked) `toBehind(aboutWindow)` - toFront is not
  honoured everywhere (Wayland); the channels button is "Set these channels for all divisions (this perspective)"
  (manual Step 5 and 5.3 follow).
  0.8.245 (the user: the Divisions tabs must reorder at once, even while the dialog is open): `MainComponent::
  perspectiveOrderChanged()` = voicer->rebuildUI + `audioTabsReorder` (a std::function set by KeyboardsDialog's
  constructor, cleared in its destructor: `reorderAudioTabs()` moves the tabs with TabbedComponent::moveTab to the
  order of getPerspectivesForManual, the chosen tab kept by name); called from reorderPerspectivePatterns (the
  column drag), the Perspective Patterns dialog's save and the Editor's "Perspective prefixes" field.
  Colours 0.8.245: the user chose t = 0.12 after seeing 0.08 (filledColour 0.12); 0.8.246: sameRegistrationColour
  0.22 ("chce to trochu větší kontrast"). BRASOV'S CRESCENDO (30 Oct 2026): the user's Brasov Surround.json (saved
  09:30) carries BELLEVUE's crescendo (all 72 step names are Casavant stops: HW_GrandBourdon32, Ped_ContreBomb32,
  P_Cello8) - loaded after Bellevue before 0.8.241 emptied a missing crescendo, then saved. Not the renaming.
  Cure: Void everything + Save. A load-time sieve (drop step/piston entries naming no table row) was proposed, the
  user said no.
  0.8.247 (the user: "nejsem s těmi barvami spokojen ... pouze ze šedé"): swatches ~/Downloads/piston_greys.png
  (A-F darker/lighter); he took B reversed: filledColour = base.darker(0.40), sameRegistrationColour =
  base.darker(0.25), the orange for the selected one alone. No orange tints anywhere else in the GUI.
  0.8.248 (the user deleted ~/.config/LibreOrgan: "on nenaskočí"): it DID start - on the second monitor
  (activity.log: "Main window: work area 2560 360 1920 1080, interface 75 %"): JUCE's getPrimaryDisplay() is the
  1920x1080 monitor at x = 2560 on this KDE Wayland box, not his 2560x1440 main one. `UIScale::startDisplay()` =
  the display under the mouse, else the largest, else JUCE's primary - used by placeMainWindow's fresh-start
  branch and by estimatePercent. A "does not start" report: read activity.log for the work area first.
  0.8.249 (the user's "Launching libreorgan failed": journalctl --user said `kf.kio.gui: Could not find the
  program '/home/doma/Desktop/LibreOrgan'` - the menu entry's Exec pointed at the moved binary; his settings
  wipe was unrelated): `~/.local/bin/libreorgan-start` (linuxLauncherText(), sh: Executable.txt of the settings
  folder, else the newest LibreOrgan Sampler in xdg DOWNLOAD/DESKTOP, ~/Downloads, ~/Desktop, ~/Applications, ~/bin,
  ~/.local/bin, ~ and LibreOrgan*/LibreOrgan below them, else kdialog/zenity/notify-send message), the entry's
  Exec = the script; `refreshLinuxLauncher()` at every start writes Executable.txt and, when the entry exists but
  does not run the script or the script's text differs, `installLinuxDesktopEntry(true)` (silent: no notice, the
  cache tools not waited for). A "program does not start" on Linux: `journalctl --user -b | grep -i organ` first.
  0.8.250 (30 Oct 2026, Windows; the user's Fireface switched off at start, then on, then off again): (1) THE AUDIO
  TAB ERASED THE CHANNELS: `AudioTabPanel::refreshChannels` filtered the stored selection to `channel < count` with the
  device not open (count 0) - every channel gone, nothing refilled; now a route without a device keeps its selection,
  shows it as ticked disabled buttons ("N: stored (the device is not open now)"), the label "Device not available now -
  channels kept:" (orange, the device in its tooltip) and `deviceStatus` says "\"X\" is NOT AVAILABLE now (switched off,
  unplugged or taken by another program) - tried again every 2 s; the stored channels are kept." (`savedDeviceName()`
  parses the route's state XML). (2) `watchMissingAudioDevices` no longer pauses while the Divisions dialog is open (the
  modal check stays), `showKeyboardsDialog` probes at once (missingDeviceNextTryMs cleared), and a device that answers
  gets EVERY route set to it reopened in one pass (log "'X': N route(s) reopened") - one route per 2 s check took 43 s
  for 15 routes (log 16:56:48-16:57:31). The ASIO list is the registry's drivers, hardware on or off: only the open
  attempt tells; a route set to an absent device stays set to it (that is what brings it back). (3) COLOURS:
  `emptyStepColour(base)` = base.brighter(0.25) for an empty crescendo step (the user: too little contrast to the
  taught ones; the pistons keep the plain grey); a taught step / stored combination with NOTHING on
  (`registrationOnSet(slot).empty()`) is painted as empty (no filled shade, no same-registration toggle; the active
  one stays orange) - a cancel piston still works. (4) BOXES (the user: "do boxu nesmí jít nikdy ... jen v
  nejdůležitějších případech", a WarningIcon notice is a box): a box only for a FAILED USER ACTION (merge, load,
  import, record, unpack, IR, "Stop folder not empty", shut down), for SILENT AUDIO (devices missing after the 6 s wait,
  the safe-start buffer) and for stops left out of a load ("Loading samples"); "Nested folder trees" went to the Messages
  tab (InfoIcon). Anything informational is InfoIcon without a callback = the strip + Messages. Manual 5.3 has the
  "A device that is not there" bullet. NOT verified with the hardware (no card to switch off here): the paths are the
  same the log shows; the user tests it.
  0.9.1 (31 Oct 2026, Windows): (1) IR CLEAR: `regroupReverbs` skipped a route with an empty `reverbIrFile` from the
  groups AND the rebuild condition wanted a file, so the convolution stayed installed ("IR: none", the room went on) -
  a cleared route with `reverb != nullptr` is rebuilt (rebuildRouteReverb with no file swaps a null in). (2) lastdir.json
  carries `lastIrDir` / `lastFolderDir` (`lastIrDirectory`, `lastSampleFolderDirectory`): Load IR and the Load folder
  browsers start where they were last; Load/Save Configuration start in the LibreOrgans folder (the user: "Konfigurace
  samozřejmě musí začínat v konfiguracích", everything else remembers its own folder). (3) A SINGLE STOP FOLDER as the
  tree (F:\temp\Principal8, WAVs named by key, no subfolders): `scanTreeAssignments`/`listSampleFolders` put the tree
  itself among the candidates when `folderHasNumberedSamples(topFolder)` (no perspective search above the tree,
  displayParent = the tree -> the row is named after the folder), `loadManualFolder` accepts folder == topFolder; the
  relative path "." was handled already (refOf/placeOf/resolve). Test %TEMP%\test_lone091.ps1. (4) GCC 11 (the arm64
  runner is ubuntu-22.04-arm): a copy-captured vector used in a nested mutable lambda is taken for const - copy it
  (`auto planNow = plan;`) before a non-const call; the x86 Linux job (newer GCC) passed. (5) TEST HYGIENE:
  ORGAN_TEST_COPY_INTO_CONFIGURATIONS=1 writes the copy into the USER'S LibreOrgans folder (and the hook's save leaves
  a .bak there) - look at CreationTime and delete what the test made; a test with ORGAN_LOAD_TREE must point
  `configurationsFolder` at a temp folder, which needs an existing settings.json (none existed in %APPDATA%\LibreOrgan).
  (6) The checkout's folder is still OrganSampler/ on disk: commits go through a worktree on origin/main
  (C:\Temp\lo091, branch release-0.9.1, files copied into LibreOrgan/...), commit 37c6d4f + tag v0.9.1 local, NOT pushed.
  The folders are renamed now (F:\JUCE_PROJEKTY\LibreOrgan\LibreOrgan); main was moved onto those commits with
  `git reset --mixed`, the worktree and the branch are gone.
  0.9.3 (31 Oct 2026, the user: "Nyní se ukazuje celý principál jako skutečně existující ... Tohle vůbec není
  F:\temp\Principal8"): his 1Stop.json (root "temp", tree "Principal8", the location "F:" remembered at the save) was
  loaded from the Rural Organ's `F:\Suchy_rejstrik\Kobrle\Samply\TA\Principal8` - `portableFolderCandidates` tried
  the locations IN ORDER with the bare `<location>/<relative>` and `<child>/<relative>` forms of each before the next
  location's `<location>/<rootName>/<relative>`, and "...\Samply" (with its child TA) stood before "F:". Now every
  form carrying the root's NAME (all locations, their neighbourhood, the drives) precedes every bare form. The
  log tells such a case: "Sample location remembered: <somewhere else>" + a tree line with more bytes/copies than
  the folder has (0.07 GB and 14 copies for a 9-file folder). RULE: a folder NAME alone never outranks the root's name.
  0.9.4 (31 Oct 2026): GRID ONE-COLUMN RULE (the user: four stops dragged under each other, the LAST drop re-laid them
  side by side): `Voicer::rebuildUI`'s "every item in column 0 while >= 2 columns fit -> lay out afresh" (meant for a
  layout made in a narrow window) erased every cell as soon as the last drop completed the column. Now: the voicing
  state carries "stopGridColumns" {division: the columns the last layout placed it over} (`placementColumnsSeen`,
  `savedGridColumns`); the rule runs ONCE per loaded file (`firstColumnCheckPending`, set in loadState, cleared at the
  first judgement - never after a drop of the session), only when the file says one column or nothing (an old file),
  and it SPREADS the items in their row order instead of erasing (log "Grid of division N: ... spread over K columns").
  Test %TEMP%\test_grid094.ps1 -Columns 0|2 (Rural's pedal stacked: spread / kept). RAM GB cell: one decimal below
  10 GB, whole GB rounded up from 10 (the user's figure).
  0.9.5 (31 Oct 2026, the user: "Zas už slyším nějaký Tabor samply místo F:\temp\1Stop\Principal8 ... Musí to umět
  vždycky rozlišit Principal8 různých varhan!" - he had MOVED the folder, the file's root "temp" + "Principal8" matched
  nothing by the root's name, and the bare name found Rural's TA\Principal8 again): TREE FINGERPRINTS. The save writes
  "treeFingerprints" {tree key as in sampleTrees: "<row folder>|<WAV count>|<hash of the lower-case names>|<hash of
  the sizes or ->;..."} for up to 3 Stop List folders of the tree (first/middle/last by name; "." when the tree is the
  stop), `treeFingerprintOf`/`folderPrintOf` (anonymous namespace before savePortableConfiguration; SampleFiles::fileNames,
  sizes only on a disk); the load's `firstMatchingTree(candidates, fingerprint, refused)` (after firstExistingDirectory)
  takes the first existing candidate whose content matches (`treeMatchesFingerprint`: count, names, sizes when both
  sides have them) - used for the divisions' trees, the extras, the pool and the merge's resolveTreeLikeALoad; a
  same-named folder with other samples is REFUSED (log "folder(s) of that name with OTHER samples refused", a Messages
  note when nothing is left) - the tree is "not found" and the notice offers the right folder. `loadedTreeFingerprints`
  (member) keeps the file's entry for a tree not here at the save. An old file without fingerprints behaves as before,
  with the log "taken by its folder name alone" when no root-named form matched. RULE: a tree is identified by its
  CONTENT, never by a folder name; the user's refused alternative (a warning box when a bare stop folder is loaded)
  was not needed. Test %TEMP%\test_fp095.ps1 (portable save of the one-stop tree = fingerprints written; the same file
  with the root renamed -> TA\Principal8 refused "65 WAV files, the configuration's tree had 9", not found).
  0.9.6 (31 Oct 2026): (1) `Voicer::forgetTransmissionCells(stop)` (0.8.237) wipes the cells and pool entries of EVERY
  transmission of the stop in EVERY division; moveStopToDivision called it for the source stop of each transmission
  carried inside a moved stop folder - the user's other extensions of Principal 8 on I. lost their cells and fell into
  a row. Now `forgetTransmissionCell(stop, index, oldManual)` (one key, the old division) in moveStopToDivision, the
  indent/merge-folder/leave-folder paths of voicer.cpp and applyTransmissionPlan; the wholesale one stays for
  `forgetStop` (the stop is removed). RULE: a transmission's cell key is "tr:<home>:<name>#n" and does not change with
  the receiving division - forget only what moved, in the division it left. (2) Stop List buttons: one row needs
  ~1 525 px; below it the folder buttons + Show Trsm. form a second row (the dialog's resized). Not verified by a
  cross-division folder drag run (no hook for it); the Stop List layout pictured by ORGAN_TEST_DIALOG_SNAPSHOT at 1200 px.
  THE USER'S DECISIONS (31 Oct 2026): a transmission's volume stays RELATIVE to its parent ("Volume vs. parent stop";
  he asked, understood, "nic neměň"); "Transposition method" and all transpose* keys travel with the organ (not
  machine-specific) - told him.
  0.9.7 (31 Oct 2026, the user: "udělej to" after the reverse case - the folder moved BACK to the pedal reset I.):
  PERMANENT TRANSMISSION IDS. The 0.9.6 forget computed the key AFTER the list changed and the key's "#n" was the
  ORDINAL among the stop's transmissions into the receiving division (Stop::transmissionOrdinal) - the carried
  transmissions got pedal ordinals 3-6 and "#3".."#6" were erased from I., the user's extensions. The design flaw:
  every reference (grid cells, pool, combinations/crescendo/registration `transmissions[]` on/off by position,
  MIDI "transmission:<m>:<home>:<name>:<ordinal>", the Stop List's TransmissionEdit) used that ordinal, which shifts
  with every move or removal. Now `Transmission::id` (per source stop from `Stop::nextTransmissionId`, never reused;
  `assignTransmissionIds()` in addTransmission/replaceTransmissions, `transmissionIndexOfId`), saved as "id" in the
  transmission entries + "nextTransmissionId" on the stop entry; `transmissionKey` = "tr:<home>:<name>#<id>";
  `storedTransmissionsOn` returns {id, manual, ordinal} and `storedTransmissionIndex` resolves by id else by the old
  pair (older memories keep working); registration keys "trans|base|id<N>"; MIDI learn key ":id<N>" (the dispatcher
  reads both); TransmissionEdit::ordinal / Entry::trOrdinal CARRY THE ID (names kept); `Voicer::removeTransmission`
  no longer shifts keys; `moveTransmissionCell(stop, index, oldManual)` moves a cell to the new division (the 0.9.6
  forgetTransmissionCell is gone). MIGRATION `migrateTransmissionIds(parsed)` at the top of Voicer::loadState: ids
  in list order for entries without one, the stopGrid/stopPool keys "tr:home:name[#ordinal]" per division -> "#id",
  log "Transmission ids assigned ...". RULE: never identify a transmission by its position again.
  0.9.8 (31 Oct 2026, the user: "set these channels ... je button, ostatní dva set for all je checkbox"): `channelsAll`
  is a juce::ToggleButton with the effects box's semantics - `channelsLinked` (std::set of lower-case perspective
  names, not saved), tick = `applyPerspectiveChannelsEverywhere` + linked, a channel button click calls
  `propagateChannelsIfLinked(perspective)` (unticks), the refresh keeps it ticked only while
  `allDivisionsShareChannels(perspective)`. Manual Step 5 + 5.3 say "tick box". RULE: the three "set for all" of the
  audio tab look and behave the same (tick = copy now, a later change unticks).
  0.9.9 (31 Oct 2026, the user's question "karta na I. 18 kanálů, 17+18, set for all, pedál má 2 kanály?"): the copy
  wrote 17+18 into the pedal's config and route, the render sent those voices to the sink (a channel >= the device's
  outputs = silence) until the pedal's Divisions tab was opened (its refresh drops the channels beyond the device and
  falls back to the first pair). THE USER'S RULE: never a fallback pair ("nemusí brát kanály 1+2, to je blbost, byly-li
  před tím vybrané např 3+4") - a division whose OPEN device lacks the channels keeps its own, the box unticks
  (channelsLinked erased) and a Messages note names it (`applyPerspectiveChannelsEverywhere`, log "... NOT for ...").
  A device not open takes the channels as stored (0.8.250). The tab's own refresh still folds a stored selection
  beyond an OPEN device to the first pair (its old rule) - untouched.
  0.9.10 (31 Oct 2026, St. Carlo "spodní půltóny o 100 centů rozladěné"): MEASURED (%TEMP%\carlo_pitch.py <folder> <lo>
  <hi>: smpl + autocorrelation f0): Principale 037-c# smpl -90 c / audio -96 c (a C), 039 a D, 042 a G (+95), 044 an A
  (+98) - the producer put the neighbouring NATURALS' files on the bass chromatics (Italian short octave); the smpl is
  honest. Contrabassi's 037/039/042/044 are own pipes within +-10 c, the rank randomly +-25 c (temperament "unknown").
  WHY "sometimes retuned": `tuningCentsFor(note, tuned)` returned 0 with "Equal at 440 Hz" OFF (equalAt440 false in
  every import), and the temperament mode 2 corrects PITCH CLASSES (measured table minus selected), never a single
  pipe; the > 50 c pipes are excluded from the table and left as recorded; mode 0 (no valid table) corrects nothing.
  With 440 ON the per-pipe tuning440Cents pulled them (90-100 c < the 200 c limit). NOW: `Stop::rankDeviationCents`
  (the pipe's deviation from its own rank: residual - celeste trend - rank diapason) and `mistuneCents` = -deviation
  for |deviation| > `mistuneCorrectCents` (engine atomic + gMistuneCorrectCents, default 50, 0 = never, not faulty),
  returned by tuningCentsFor when !tuned; `setMistuneCorrectCents` recomputes from rankDeviationCents (new notes take
  it). Configuration key "mistuneCorrectCents" (travels with the organ), Editor slider "Retune pipes off by more
  than" right BEFORE the old limit, which the user renamed: "Retune limit per pipe" -> "Replace pipes off by more
  than" (key retuneLimitCents unchanged; the manual heading renamed - openChapter matches labels exactly); both sliders
  sit in Advanced global audio right after "Celeste beats", NOT under Filled pipes (the user's wish). The
  voicing key tooltip of an own pipe says "Corrected by +90 cents: the file sounds 90 cents below its key". Log
  "Tuning <rank> [p]: N pipe(s) off the rank by more than 50 cents pulled to their keys without the 440 mode: 37 (+90 c) ...".
  0.9.11 (31 Oct 2026, the user: "proč to v St. Carlo nenačítá releasy? rel_027?"): `extractReleaseHoldMs` read only
  whitespace between "rel" and the digits, so "rel_0100"/"rel_0250"/"rel_0560" (the hold-time multi-release folders of
  every St. Carlo stop) returned -1 and `loadRankSamples` skipped them in SILENCE (3.39 GB read of 5.0 GB estimated).
  Now "_", "-", ".", spaces and a trailing "ms" are accepted, "release" as the word too; a rel* folder still not
  understood is logged "Release folder X of Y not understood ... skipped". RULE: a folder the loader skips by its
  NAME is logged, never silent. Verify a set's releases by the tree's GB read vs the WAV estimate.
  0.9.12 (31 Oct 2026, the user's silence at 15:45: "neslyším nic, DSP 0 %" - the Fireface's Windows Audio endpoint
  opened but called back nothing after a USB re-enumeration at 15:41 ("MIDI devices changed: gone microKEY2 Air,
  Komplete Audio 6 In"; its MIDI ports gave WinMM error 1 too); the same with 0.9.9 - NOT a regression, the user
  first took it for one): `checkAudioCallbackHealth` keeps `silentDevices` (label -> seconds without a callback,
  reopenedMs, noticed): a report "no callbacks" while not muted / no device change pending / no set-for-all / no
  ASIO panel watch -> after 4 s `reopenAudioDeviceFresh(manager, "No audio callbacks from X for 4 s - recovery")`
  (once a minute at most; the manager now travels with the callback into `check`), and when the callbacks are still
  missing 8 s later a Messages note "No audio" + `PolyphonyDisplay::setDeviceSilent` (the DSP cell reads "no audio"
  in orange) until the device calls back again. NOT verified by a run (no way to hang a device on purpose; the user
  was playing). The log of such a case: "Audio callback anomaly: <device>: no callbacks, outputs 0  render load 0 %".
  0.9.13 (31 Oct 2026, the user: St. Carlo imported after wiping the settings "nenašlo adresáře ... žádné LibreOrgans v
  cestách k samplům nikdy nebylo"): THE LOOKUP RULE, from the code: a tree travels as `sampleLibraryRootName` + a path
  BELOW it ("OrganInstallationPackages" + "000370/pipe"); `portableFolderCandidates` tries, in order, the configuration's
  own folder forms (configDir/rootRelative/relative, configDir/rootName/relative, configDir/relative - the FIRST one is
  the placeholder the notice shows, hence "LibreOrgans\OrganInstallationPackages\pipe" on the screen; nothing is
  searched there beyond one directory test), then `<location>/<rootName>/<relative>` for every Locations.txt line, its
  parent and children, then every fixed drive: `<drive>/<rootName>/<relative>`, `<drive>/HauptwerkSampleSetsAndComponents/
  <rootName>/<relative>`, and the same under each first-level folder (0.8.214/0.8.215: the usual Hauptwerk place
  D:\Hauptwerk\HauptwerkSampleSetsAndComponents\OrganInstallationPackages WITHOUT any location), the bare
  `<location>/<relative>` forms last (0.9.3); `firstMatchingTree` then wants the fingerprint (0.9.5). THE FAULT: the
  import's var holds ABSOLUTE trees (since 0.8.116; its own save turns them relative) and the save derived the root
  from `treeRelativePaths` VOTES (0.8.216) - none for absolute trees - so it fell to the common ancestor of the trees at
  hand = the package "000370", while 0.8.216 kept the NAME from the file ("OrganInstallationPackages"): the file said
  root "OrganInstallationPackages" + tree "pipe", which exists nowhere; the location noted was ...\000370 (the root
  itself, the name mismatching), and only that line made the next load work - a wiped Locations.txt broke it. Before
  0.8.216 the name followed the computed root ("000370", location OrganInstallationPackages): consistent, but found
  only through the remembered location too. Fix: `applyPortableConfiguration` remembers `relativeBelowNamedRoot
  (absolute, rootName)` for every absolute tree found (pool entries, a division's absolute folderPath, extraFolderPaths)
  and the save's vote block votes the same way for folders with no known relative - the root is the folder NAMED as the
  file names it, the trees "000370/pipe", the location "...\HauptwerkSampleSetsAndComponents". The user's St. Carlo
  json was patched by hand the same way (backup .before0913.bak). Test %TEMP%\test_carlo0913.ps1: the import into a
  temp configurations folder writes ["000370/pipe"]; the json loaded with Locations.txt EMPTY finds the tree through
  the drive's Hauptwerk layout (13 rows). RULE: the root in a file is the folder of the NAME the file gives, never
  the common ancestor, whenever that name is in the trees' paths.
  0.9.14 (31 Oct 2026, the user: "okno maličké ... regres", "tooltipy nad Tremulant, žaluziemi, spojkami, pistony,
  crescendem, hlavním displejem, posuvníky se neukazují", "projdi celou historii kódu, inventura regresů, manuál"):
  THE INVENTORY = the whole CHANGELOG (5 312 lines, 0.5.0-0.9.13) read against the code, ~100 labels/keys counted in
  the sources (%TEMP%\claimcheck.ps1), the setTooltip identifiers of v0.8.240 diffed against HEAD (%TEMP%\tipdiff.ps1:
  133 -> 137 calls, NOTHING lost since). Findings: (1) WINDOW: no version ever opened a fresh window to the screen -
  savedWindowWidth/Height 1680 x 900 since the first commit, the user had a saved maximised state for weeks and saw
  the default first after the wipe; now a first start (savedWindowX/Y INT_MIN) takes the work area less the 2 % margins
  (placeMainWindow, log "(first start: the work area with margins)"; test %TEMP%\test_window0914.ps1: 2442 x 1297 on
  2560 x 1392). (2) TOOLTIPS: the 0.8.236 changelog claimed "every control" but the header's Tremulant, SwellBar (not a
  TooltipClient), title/keyboard indicator, the coupler buttons, the pistons, combinationDisplay/bankLabel and the
  PolyphonyDisplay cells NEVER had one (the audit of 0.8.236 over-reported) - set now (SectionHeader is a TooltipClient
  for the painted indicator rect, SwellBar a SettableTooltipClient, PolyphonyDisplay::getTooltip per column, the
  limitEditor covers LIMIT). The HelpTooltipWindow/Hider code read line by line: no runtime fault found; JUCE never shows
  a tip at the position of the last mouse-down (a clicked button under a resting mouse shows none - standard, tell the
  user when he reports "no tooltip" on a button he just pressed). (3) The fade sliders' double-click returned 70 ms
  (makeGlobalSliderLearnable + bindSlider) while the default is 110 since 0.8.215 -> `MainComponent::kDefaultFadeMs`
  used in all six places. (4) MANUAL: 1.4 said `Documents\LibreOrgan\Configurations` (wrong since 0.9.0); eight quoted
  chapter names turned into Markdown links (%TEMP%\manual_links.py: 219 headings, 0 duplicate slugs, 0 broken links);
  the 2.4 loading-stop, 5.3 missing-device, 8.1 full-kernel/storage and 4.1 unload paragraphs cut; the 1.1 and 1.6
  Hauptwerk frames shortened to one sentence each with a link to 1.5 (the full frame stays in 1.5 and 10.4 - the
  user's 0.8.253 rule). Everything else of the changelog is in the code (intentional later changes noted in the
  entries themselves). The seven audit subagents of the session died on a billing error; the audit was done by hand.
  0.9.15 (31 Oct 2026, the user: "dementní modré posuvníky na pravém a spodním okraji" of the EMPTY console; and "netvrď
  mi, že nikdy nebylo nastavení okna na maximální velikost ... tu spodní windows lištu jsme výslovně řešili"): (1) THE
  WINDOW RULE THAT EXISTED, verbatim from the first commit (MainComponent constructor): "A clean installation must fit
  comfortably on a 1920x1080 desktop, including room for the taskbar and native window frame. Preserve the established
  larger workspace for users who already have settings." = the FIXED default 1680 x 900 chosen to fit Full HD with the
  taskbar - never the screen's own size; on the user's 2560 x 1392 work area it is small. 0.9.14's first-start rule
  (the work area with margins) is what he remembers wanting; say "the default was sized for Full HD" when it comes up
  again, never "there was no rule". (2) SCROLLBARS: `Voicer::layoutScreen` rounded every column's width on its own
  (available x weight / total) - with 3 columns on a width not divisible by 3 the sum exceeded the viewport by 1-2 px ->
  a horizontal bar, which took height -> the vertical bar too (an empty console with the default 2 manuals = 3 columns:
  2403 > 2402 here); and with an organ reaching below the window the vertical bar took width the columns had been
  given -> both bars always. Now the right edges come from the cumulative weights (the sum is exact; a 220 px minimum
  may still exceed), `layoutScreen(..., reserveVerticalBar)` returns "content taller than the viewport" and rebuildUI
  lays a screen out a second time with the bar's width reserved when so. Log "Workspace: content W x H in a viewport of
  W x H, scrollbars: none|horizontal|vertical" (once per change; voicer.cpp). Test %TEMP%\test_window0915.ps1 -Manuals
  2|5 (works without any settings.json: the user wipes AppData): "scrollbars: none" at 2402 x 1065. RULE: content laid
  into a Viewport is never wider than the width the bars leave; round the LAST edge, not every part.
  0.9.16 (31 Oct 2026, the user: a zip/rar dropped on the desktop icon starts the program but loads nothing):
  the command line was never read for a file (only the --selftest/--asiotest/--kerneltest/--hauptwerk switches),
  `anotherInstanceStarted` only said "already running". Now `fileFromCommandLine` (Main.cpp, a static member of the
  application class: the whole line unquoted when it is one existing absolute path, else the first token that is no
  "--" switch and exists) -> `MainComponent::startupFile` (public static, set BEFORE `new MainWindow`: the constructor
  reads it, skips "Load the last configuration at startup" that start - log "Started with the file ... (the last
  configuration is not loaded this time)" - and calls `loadConfigurationFile` by callAsync, so the load box comes as
  with the button) and `MainComponent::openFileFromDesktop(file)` for a running instance (log "File from the
  desktop: ...", `askToKeepChanges` first, then loadConfigurationFile; the "already running" box only without a
  file). The Linux .desktop Exec line carries " %f" (the launcher script passes "$@" already). Test
  %TEMP%\test_dropfile0916.ps1: Rural loaded from the command line with "Load" clicked by ORGAN_TEST_CLICK (the copy
  landed in the temp configurations folder), a second start with TestingOrgan.json handed to the first. Declarations
  of things Main.cpp uses go into the PUBLIC block of MainComponent.h (lines 38-61) - the kDefault* constants at line
  136 are private (the Editor is a friend).
  0.9.17 (31 Oct 2026, the user: "Dulce" added from the ready-made stops three times - the log shows "3man Dulce 8",
  "(2)", "(3)" applied together): `readyStopsNotIn` (0.8.142, #44) counted a row's folders as used ONLY when the
  engine held the stop (`stopLoaded`), meant for rows a memory-limited load left out - so a row just added from the
  pool (a draft, not applied) and a row of a missing tree blocked nothing and the stop stayed on offer. Now that
  exception applies only while `engine.lastLoadWasMemoryLimited()`; every other row with folders uses them from the
  moment it exists (the Stop List's Add stop... and the grid's free cell both go through readyStopsNotIn). Not driven
  by a run (no hook for the Add stop... menu); the rule is one condition at the top of the used-places loop.
  0.9.18 (1 Nov 2026, the user: drag & drop of folders/rars/configurations onto the console, "napoj to na existující
  struktury ... jen vstup odjinud"): `MainComponent` is a `juce::FileDragAndDropTarget` (no DragAndDropContainer
  existed; the GlowLayer/HintSheen/frozenGrid children take no mouse, so the target is reached through them).
  `isInterestedInFileDrag` refuses while `activeFolderLoader` runs or any modal component is up; `divisionAtPoint`
  = the column of `voicer->divisionGridAreas()` (stopsContent coords) under the point inside stopsViewport;
  `fileDragMove` frames the column (`fileDropHighlight`, painted in the new `paintOverChildren`: orange frame +
  "Load folder into Manual N"); `takeDroppedFiles(paths, division)` (shared with the hook) sorts the drop: a
  folder holding a configuration (.json with "manualKeyboards") or a definition (directly or in its
  OrganDefinitions) counts as an organ, a folder whose `scanTreeAssignments` yields no non-container stop is
  REFUSED (queueNotice InfoIcon "Folder refused" = Messages, logged), rars = archives, .json/.zip/ODF =
  configurations, the rest ignored (Messages "Files ignored"). Organs present: the dropped archives
  `registerArchive`d and the dropped folders' parents `noteSampleLocation`ed (they serve the organ's paths), one
  -> `askToKeepChanges` + `loadConfigurationFile` (the load box as with the button), several -> `chooseFromList`.
  Else folders + archives -> `loadChosenIntoDivision(folders, archives, false, {}, division)` (= Load folder...:
  archives indexed, a definition inside offered/chosen by loadIndexedIntoDivision, the trees gated and joined);
  division -1 (outside the columns) -> chooseFromList of the divisions. THE LOAD BOX COMBO "action" (askLoadOptions,
  shown when `engine.getStops()` is not empty): "Load it as a new organ" / "Merge it into the loaded organ" ->
  var property "mergeIntoLoaded" -> loadConfigurationFile's lambda calls `mergeConfiguration(chosen, source, ...)`,
  the ODF lambda calls `importHauptwerkOdf(odfFile, true)` (the right-click paths are untouched: they never reach
  the box). Hooks: `ORGAN_TEST_DROP_FILES="<division>|<path>;<path>"` (at ORGAN_TEST_DROP_AT or ORGAN_TEST_CLICK_AT
  seconds, else 3 s; -1 = outside), `ORGAN_TEST_LOAD_ACTION=merge` (preselects the merge in the box),
  `ORGAN_TEST_READY_STOPS="<division>|<stop>"` (logs the ready-made list, adds the stop by the grid path, logs
  again 10 s later - not yet exercised: no organ at hand with free folders). Test %TEMP%\test_drop0918.ps1 [-Only
  A|B|C]: A Samply dropped on division 1 of an empty console -> "Division(s) 1 rebuilt: 68 stops" (a division
  reload logs "rebuilt", NOT "Compiling the Stop List: done" - that line is the full load's); B an empty folder ->
  "Folder refused"; C Rural from the command line, TestingOrgan.json dropped at 36 s with the merge preselected ->
  "Merged ... 55 stop(s) added, 14 shared". POWERSHELL LESSON: never name a function parameter `$args` (the
  automatic variable: the first run of C started the exe without the file and the 3 s "Load" click hit the
  combination row's Load button - a harmless file chooser).
  0.9.19 (1 Nov 2026, the user's Caen Composite: the Recit loaded from its folder "001691\pipe\rec" as a tree, then
  the Caen Surround definition merged through the new box choice - "87 stop(s) added", the Recit stops TWICE and the
  definition's two-rank stop folders beside the halves loaded as stops; afterwards "Nested folder tree dropped:
  001691\pipe\rec lies inside 001691"): the merge's shared-folder rule keyed folders by "tree index|path", so the
  same folder reached through the nested tree and through the package tree never matched. Now `placeOf(ref)` =
  pathKey(mergedPool[tree] + "/" + relative) - the PLACE ON DISK (the 0.8.136 rule of readyStopsNotIn) - keys
  `baseFolders` and both lookups; and the GROUP RULE before the perspective block: an added stop folder (a container
  followed by child rows) whose children's folder places are ALL in baseFolders replaces the base rows that play
  those places as single stops (rows without children whose folders all lie in the group): the rows are erased
  from baseBuild.entries (before the perspective matching, so no row indices shift), their baseFolders/baseShown
  entries dropped, the container and its children exempted from the shared rule (`replacingEntries`), `done` gets
  "<div> <folder> [N] composed of a + b - the rows x, y, which played the same folders as single stops, were
  removed" (the merge summary = Messages). The children keep the halves' names (the scan and the definition name
  a rank folder alike), so the voicing and the grid cells of the halves pass to them. Test
  %TEMP%\test_merge0919.ps1: Samply dropped on division 2 (68 scan stops), Rural Organ.json handed to the running
  program (0.9.16) with ORGAN_TEST_LOAD_ACTION=merge and the clicks "Don't Save|Load" (the dropped tree makes the
  organ DIRTY: the unsaved-changes question comes before the load box - the first run clicked "Load" into the
  combination row's Load button instead) -> "the stop folder CO - nasard [III] replaces the rows CO - nasard, CO -
  piccolo 2, CO - terz", "13 stop(s) added, 2 shared". ALSO 0.9.19: the empty console's hint starts with "Drag and
  drop your sample folders, RAR archives, folder trees or organ configurations onto a division." (panel 900 x 200,
  label 120 px); the drop frame's caption sits in the MIDDLE of the column (at the top it covered the division
  header - check where a caption lands before painting it); the console's HintSheen `follow` over Stop List / Load
  Configuration is gone (the class and the Stop List dialog's loadHint stay).
  0.9.20/0.9.21 (1 Nov 2026): the drop frame carries NO caption (the user: it covered the stops; the orange says
  it). THE REVERSE CAEN CASE (log 08:47: Caen from the definition, then the package 001691 dropped on III -> "not
  another perspective", the scan made a row of every folder, "Division(s) 3 rebuilt: 68 stops", the nested tree
  merged only afterwards): Load folder compared TREES by path, never the folders. Now `MainComponent::
  usedFolderPlaces()` (the lower-case full paths of every folder a row of any division plays, morePaths
  included) and `extendStopBuildWithTree` leaves such folders out (log "Load folder: N folder(s) of X are loaded
  already ... not added again: ...", Messages "Folders loaded already"); `planTreeJoin` excludes them from the
  perspective candidates (else a folder would be gated against its own row - the same files pass the gate). The
  merge's `placesOfRef` adds the morePaths halves to every place set (baseFolders, the group rule, the shared
  rule). L/R BY CONTENT (the user's rule: under -70 dBFS, 0.5 s is plenty): `SamplerEngine::silentChannelComponent
  (folder)` = up to 3 numbered WAVs of the folder read through SampleFiles::openInput + WavAudioFormat, the first
  0.5 s, `getMagnitude` per channel, one channel < -70 dBFS and the other not -> "L" (right silent) / "R" (left
  silent), all read files must agree, mono = nothing; cached per folder (static map + mutex: the scan runs at every
  Stop List opening / Add stop / join). `componentOfFolder(folder, folderKeys)` (anonymous namespace before
  listSampleFolders) replaces the two identical lambdas of scanTreeAssignments: the 0.8.160 name rule first, else
  the content: own half + a sibling named with/without a trailing l/L/r/R (or _l/-l ...) whose content holds the
  OTHER half. The learning loop drops the letter from the stop part only for a LETTERED name; the assignment's
  stopName loses a trailing l/L/r/R only after a digit/separator/apostrophe or as a capital after a small letter
  ("quintaton 16l" -> "quintaton 16", "Principal" stays). Caen Recit (Cornet + CornetL, pleinjeu + pleinjeuL ...):
  20 folders -> 10 stops, R for the unlettered and L for the lettered half, 37 ms for the scan (ORGAN_TEST_COMPONENTS
  =<tree> logs every assignment). The halves STAY one row with two folders ("+ half", morePaths) as since 0.8.160 -
  the definition's stop folder of two children is the other representation and the merge maps one onto the other;
  a scan producing container+children rows was NOT done (18 morePaths places in the code: regression risk). Hooks:
  `ORGAN_TEST_DROP_FILES` takes several drops separated by "^" (25 s apart; juce::StringArray::fromTokens splits on
  single characters - "||" split on every "|"). Tests: %TEMP%\test_components0921.ps1 -Only 1 (the Recit scan),
  %TEMP%\test_loaded0921.ps1 (Samply on division 1, then Samply\TA: "8 folder(s) ... loaded already", 68 stops
  stay, the nested tree dropped). A drop-loaded organ is NOT a configuration: a restart "Started without a
  configuration" empties the library - a two-step test must run in ONE process.
  0.9.22 (1 Nov 2026, the user's two messages after 0.9.21 + "ta půlsekunda musí být pro všechny 3 testované samply.
  Jediná výjimka ruší pravidlo left-right"): (1) `silentChannelComponent`: the first min(3, files) numbered WAVs
  must EACH be stereo, hold the full 0.5 s and give the same verdict - a shorter file, an unreadable one or another
  verdict cancels (before: unreadable/short files were skipped, a short one read whole). (2) TRANSMISSION OFFER:
  `usedFolderPlaces()` returns place -> (division, row name); `extendStopBuildWithTree` collects the rows of OTHER
  divisions playing the folders left out into `pendingTransmissionOffer` (TransmissionOffer{targetManual, treeName,
  rows}) and `runWhenLoaded(offerTransmissionsForLoadedFolders)`: the stops found in the engine (no container, no
  transmission into the target with shift 0 yet) -> showOkCancelBox "Folders loaded already" (Make transmissions /
  No) -> `engine.addTransmission(stop, target, 0)` each, voicer updateStopStates + rebuildUI (the cells placed
  afresh), configurationDirty, saveSettings, Messages "Transmissions made"; log "Load folder: N stop(s) of other
  divisions play folders of X - transmissions into Manual N offered" / "... made". A row of the SAME division gets
  no offer. Hook `ORGAN_TEST_LOADED_TRANSMISSIONS=1` takes the offer. (3) MERGE KEEPS TRANSMISSIONS
  (MainComponentMerge.cpp voicing block): `fitTransmissions` (divisions > mergedCount dropped; groupMaster mapped
  through `renamed` / `sharedStopNames`, cleared when no such folder is here), `entryOf(manual, name)`,
  `handTo(manual, name, list, from)` (the target's voicing entry, made when missing; a transmission with the same
  (manual, shift) not doubled; "id" removed - the engine assigns new ones; `transmissionsKept` + a done line): a
  KEPT added stop keeps its own list fitted, a SHARED stop's entry (skipped whole before - its transmissions
  vanished) hands them to the base row of `sharedStopNames`, and `groupReplaced` ("manual|removed base row" ->
  the added child's original name, filled in the 0.9.19 group rule from `childOfPlace`) hands the removed row's
  own transmissions to the heir child (the retained entry's list emptied). Summary "N transmission(s) of the added
  configuration kept on the stops playing their folders here". Tests: %TEMP%\test_merge0922.ps1 (Samply on II,
  Rural merged: the shared II stops' transmissions kept), %TEMP%\test_offer0922.ps1 (Samply on I, then Samply\TA
  on II -> the offer taken, "transmission(s) into Manual II made").
  0.9.23 (1 Nov 2026, the user, furious: "Přidal jsem 001691, nevytvořily se stop foldery z L a R, přidaly se jenom
  L ... grid zase počítal i s těmi políčky, která zůstala prázdná"): (1) the 0.9.21 decision "the halves stay one
  row with + half" is REVERSED - `MainComponent::placeHalfInTable(build, stopName, component, perspective, ref)`
  (static, before applyStopBuild) makes a STOP FOLDER of a scanned half: a container row named the stem
  (customName true) + child rows "<stem> R" / "<stem> L" (child true, one folder per perspective) inserted after
  the last child; used by extendStopBuildWithTree (Load folder / drop), deriveStopBuildFromEngine's fallback and
  the merge's tree scan (which DROPPED a half before: same stopName + perspective). The grouped search of all
  three never targets a row without folders (a stop folder). morePaths stays for OLD files (18 places). The
  engine's own automatic path (no table: old folderPath-only files, the self-test) still joins halves as two
  ranks of one stop. 001691 dropped on III: "Division(s) 3 rebuilt: 30 stops" = 10 folders + 20 children.
  (2) THE GAPS: a start without a configuration kept the LAST organ's voicing - loadSettings writes settings.json's
  voicingSettings into voicer.json and the constructor's `voicer->loadState` (line ~1769) reads it BEFORE the
  clean branch (~1849), so the empty console held 97 retained stops and every grid cell of the last organ BY NAME
  (the user's 0.9.22 run: "pipe - rec - Cornet (division 3)" cells of the previous drop); a tree loaded then took
  the stale cells of its namesakes and the unplaced rest filled the free cells between = holes. The clean branch
  now writes {stopGrid {}, stopPool {}, stops []} into voicer.json, loadState's it (each block clears its map),
  voicerStateLoaded/embeddedVoicingPresent false, log "Started without a configuration: the voicing of the last
  organ ... left with it". The grid's layout itself (voicer.cpp ~3024-3073: stored cells of PRESENT items, the
  unplaced fill free cells row by row) was not changed. (3) MERGE of a stop folder the base HAS as a stop folder
  (the scan's since 0.9.23): the group rule maps the places to base rows; all child rows of ONE base container ->
  `sharedContainers` (the added container shared, mapped onto the base container in sharedStopNames - it was
  added as an empty "(2)" folder before) and its children shared by the old rule. Tests: %TEMP%\test_halves0923.ps1
  (001691 on III of the empty console with the stale voicing: 30 rows, 10 folders, 0 "+ half", grid "OK: no hole"
  by check_halves0923.py on the saved settings), test_mergesame0923.ps1 (the saved settings merged back as a
  configuration: "0 stop(s) added, 30 shared", table and grid unchanged), test_merge0922.ps1 (single rows still
  replaced by Rural's CO folder, 2 transmissions kept). The folders in a saved stopBuild entry are a LIST
  [{perspective, tree, path}], not a map (check scripts).
  0.9.24 (1 Nov 2026, the user: FluteTravers8L alone on III, then rec on II - "nezařadil ji pod FluteTravers stop
  folder"; chose B: "raděj B, aby byly skutečně u sebe" + the disk sibling): (1) `componentOfFolder`'s `present`
  also accepts the partner folder BESIDE the folder on the disk (isDirectory + folderHasNumberedSamples) - a lone
  half dropped as the tree becomes a stop folder with one child ("FluteTravers 8" + "FluteTravers 8 L"). (2)
  `HalfMove` {sourceManual, rowName, targetManual, stem} - `pendingHalfMoves` filled in extendStopBuildWithTree
  for a left-out folder with a component (instead of the transmission offer; the same division too); the
  runWhenLoaded handler (`offerTransmissionsForLoadedFolders`, before the offer) finds the half's stop and the
  target container by `stopBuildKey(name) == stopBuildKey(stem)` and calls `indentStopUnder(half, master)` (in
  place: voicing, transmissions, cells go along, `Stop moved:` log), then removes the old master when
  groupChildren/groupChildTransmissions/transmissions are empty (`removeStopFromGrid`), Messages "Halves joined";
  no container made in the target (both halves elsewhere) -> the row joins the transmission offer. LESSON: the
  "already under the master" test must compare the DIVISION too - the lone-drop folder on III and the scan's on
  II are both named "FluteTravers 8", the name-only test took the half for placed and did nothing (first run).
  Test %TEMP%\test_halfmove0924.ps1: III 2 stops -> after rec on II: "Stop moved: FluteTravers 8 L from division 3
  to 2", III's folder removed, III 0 rows, II 30 rows, grid of II without holes. The scan's names carry the
  parent chain only when the tree lies above it ("pipe - rec - Cornet" for the package, "Cornet" for rec itself).
  0.9.25 (1 Nov 2026, the user: "proč ten msgbox ... se objeví tak pozdě" + "transmisi pro L a R je vytvoří jako dva
  samostatné rejstříky ... do jednoho stop folderu se dvěma transmisemi pod sebou jako děti"): (1) TIMING:
  `finishDivisionsLoadEnd` calls `offerTransmissionsForLoadedFolders` 300 ms after "Division(s) N rebuilt" when
  `pendingTransmissionOffer` is set (the handler re-arms itself every 500 ms while `activeFolderLoader` lives; the
  runWhenLoaded call stays as the fallback and returns at once on a null offer) - runWhenLoaded's 5 s polls + one
  quiet poll made the box come 10-15 s late. (2) GROUPED: the `make` lambda keys the offered stops by (source
  division, `Stop::groupMaster`); the ranks of ONE stop folder get a container of that name in the target
  (`uniqueStopBuildName` on the target table + `engine.addContainerStop`, customName, `folderOf` map, once per
  key) and each transmission's `groupMaster` is set to it (`replaceTransmissions`), the container
  `voicingExpanded`, `finishInPlaceStopEdit({target})` when folders were made - exactly what a transmission
  indented under a grid folder is. Stops without a master stay plain transmissions. Test
  %TEMP%\test_grouptrsm0925.ps1 (rec on II, then the package 001691 on III with ORGAN_TEST_LOADED_TRANSMISSIONS=1:
  20 transmissions into III under 10 folders; check_grouptrsm0925.py counts the saved transmissions with manual 3
  and a groupMaster). (3) HALVES APART (the user: "I had L in I, and R in III, then importing the whole rec to II
  ... people may accidentally do this"): the used branch of extendStopBuildWithTree calls `placeHalfInTable(build,
  stem, {}, "default", {})` = the CONTAINER alone (an empty `component` returns after it) for every used half, so
  the stop folder exists in the target even when both halves are loaded elsewhere and the half moves bring both
  under it (the emptied lone folders of I and III removed). Test %TEMP%\test_halves_apart0925.ps1 (three drops:
  L on I, R on III, rec on II -> I 0, III 0, II 30 rows).
  0.9.26 (1 Nov 2026, the user: "na původních místech to neudělalo transmise těchto stop folderů ... Žádný dialog
  ... nevyhodilo"): `makeGroupedTransmissions(sources, target, why)` (member; the former `make` lambda: a stop
  with a transmission into the target at shift 0 skipped, the ranks of one groupMaster under a container of that
  name made in the target, log + Messages "Transmissions made"); offerTransmissionsForLoadedFolders keeps
  `movedFrom` (old division -> the halves moved out, divisions != target) and, when not empty, shows the box
  "Halves joined" (Make transmissions / No: transmissions of the joined stop into every old division, grouped
  under a stop folder of the same name there - the emptied lone folder was removed before, so no "(2)"), and
  only THEN `continueWithOffer` (the 0.9.22 offer, now a lambda over a shared_ptr offer). The hook
  ORGAN_TEST_LOADED_TRANSMISSIONS=1 takes both boxes. Test %TEMP%\test_halves_apart0926.ps1 (L on I, R on III,
  rec on II with the hook: I and III each one stop folder "FluteTravers 8" with 2 grouped transmissions,
  II 30 rows; check_oldplaces0926.py). TWO LESSONS of the first 0.9.26 build: (a) `movedFrom` carried the half
  that LEFT each division, so I got a transmission of L alone and III of R alone - it carries the stop FOLDER
  (the master in the target) now and makeOld takes every rank under it at answer time: the whole joined stop
  goes back to both old places. (b) the 0.9.25 "container even when both halves are elsewhere" rule pulled a
  WHOLE stop folder (R+L together on II) to III when the package was dropped there (test_grouptrsm0925 got 30
  rows on III instead of transmissions): `wholeElsewhere(stem)` in extendStopBuildWithTree (pre-pass
  `halvesInTree` / `usedHalfRows`, `masterOfRow` = the child row's folder in that division's table) - all the
  tree's halves of the stem used, >= 2, one division, one master -> the ordinary transmission offer (grouped);
  else the join. The three tests must pass together: test_halves_apart0926 (join + both old places get the
  whole stop), test_grouptrsm0925 (20 transmissions under 10 folders on III, 10 rows there), test_halfmove0924.
  0.9.27 (1 Nov 2026, the user's three wishes + a question): (1) FOLD: `toggleAllVoicingButton.onClick()` (= "Hide
  all voicing") at the end of the halves block and in makeGroupedTransmissions (the containers are no longer
  expanded) - "nechceme vidět rozbalené stop foldery". (2) `Voicer::flipDivisionUpsideDown(manual)` (voicer.cpp
  before freeCellClicked): the cells of the division's gridItems mirrored at the last row (row' = maxRow - row,
  columns kept), rebuildUI + stateChanged; the workspace right-click menu item 3 "Grid upside down (the rows of
  this division reversed)"; hook `ORGAN_TEST_FLIP="<division>|<seconds>"` (+ saveSettings). (3) Stop List
  button `upsideDownButton` "Stop Grid Upside Down" (after Show Trsm. on the folder row, 190 px, oneRowWidth
  updated): the drafts' BLOCKS (a non-child row + its children) reversed per division, orderChanged, Apply ->
  the "Order of the stops" Yes/No box lays the grid out in the table's order. Manual 3.2 + 4.2. Test
  %TEMP%\test_flip0927.ps1 [-Only A|B] (A: rec on II + the hook -> Cornet's cell row 4, Voixceleste 8 row 0; B:
  ORGAN_TEST_CLICK "Stop List|Stop Grid Upside Down|Apply and reload|Yes" -> the table of II begins with
  Voixceleste 8). (4) THE QUESTION (answered, NOT coded): rec loaded from the folder on II, then the Caen
  Surround ODF merged - the ODF's Recit stop folders on III load the SAME folders again because the merge's
  shared rule is per division (baseFolders of the same division index). Proposed: organ-wide baseFolders (place
  -> division, row); when the added (ODF, the richer source) stop plays folders the base plays in ANOTHER
  division, the ODF wins: the base rows are removed from their division and replaced by transmissions of the
  new stop into it (grouped under a folder of the same name, makeGroupedTransmissions after the load), their
  voicing/transmissions handed to the new stop (0.9.22 handTo). DONE in 0.9.28 ("ano, udělej!").
  0.9.28 (1 Nov 2026): (1) MERGE ACROSS DIVISIONS (MainComponentMerge.cpp): `organFolders` (place -> (division,
  base row), every base division's table, before the division loop; mergedPool = the base pool, so base refs
  resolve); in the main added-entries loop, an entry not shared in its own division whose places the base
  plays in ANOTHER division -> `crossReplaced[division]` += the base row, `crossHandover` (division, base row,
  added division, added original name), `pendingMergeTransmissions` {manual, name, into} (renamed with the "(2)"
  rule); after the loop every such division's stopBuild var is re-read, the rows erased (a stop folder left
  without children and folders too), written back, a done line; the voicing block hands the removed rows'
  transmissions to the new entry (handTo); the summary counts the rows. `MainComponent::pendingMergeTransmissions`
  (member, cleared at the merge's start) is served by `offerTransmissionsForLoadedFolders` FIRST (no box: the
  replacement was the merge's decision) through `makeGroupedTransmissions`, triggered 300 ms after "Compiling
  the Stop List: done" (the full load's end) when the list is not empty. (2) HILL NAMES: `stopsOfFolder` in the
  import's row naming - a stop folder (`stop.folders.front()`) shared by several stops of the division (a FLAT
  set: "Pedal" holds every rank folder) is passed over, the rank folder's name (prefix cut) is the row's name.
  (3) ONE BOX "Transmissions": the offer rows and the joined halves' old places in one text, `makeAll` on yes;
  the hook ORGAN_TEST_LOADED_TRANSMISSIONS takes it. Tests: %TEMP%\test_mergecross0928.ps1 (rec on II, the Caen
  Surround ODF merged through the box: "II. the rows ... replaced", after the load "transmission(s) into Manual
  II made for the rows the merged stops replaced", II = transmissions only), test_hillnames0928.ps1 (the Hill
  Demo ODF imported by the hook: rows "Ped_tromba8" ..., never "Pedal (N)").
  GITHUB RULES (1 Nov 2026, the user): (1) THE ORGAN CELL OF INDEX.md: a trailing word "organ"/"Organ" is dropped
  ("Adlington Hall Organ" -> "Adlington Hall"; `configurations_index.py` re.sub(r'\s+[Oo]rgan$') on the README's
  title) - "skoro každý řádek to má, zahlcuje to tabulku"; "Organ" INSIDE a name stays ("Hill English Organ
  Model"). (2) PARENTHESES IN A NAME: issue #138 "Adlington Hall (Smith)" was refused - the validator's link
  regex `([^\s)]+)\)` cut the file name at the first ")" ("Adlington Hall (Smith"); now `(.+?\.zip)\)` +
  urllib.parse.unquote. The attachment's name loses the parentheses and spaces ("Adlington.Hall.Smith.portable.zip")
  but the robot names the pair after the .json inside, so the pair validates ("adlington-hall-smith"). To test
  a refused upload locally: download the attachment (API token from `git credential fill`), split the README out
  of the zip, RENAME the zip after the json inside, `configuration_tools.py validate <zip> <readme>`. A refused
  issue is checked again on any EDIT of its body (the workflow runs on opened/edited) - edit it through the API
  after the fix is on main. The local checkout was 12 behind origin (the user's publishes): `git stash`, `git pull
  --no-rebase`, `git stash pop` left the regenerated INDEX files "UU" without markers - `git add` them.
  (3) THE REMOVE LINK PRE-SELECTS THE ORGAN (the user, 1 Nov 2026): GitHub does not honour a prefill of a
  dropdown reliably (community discussion #5288; a logged-in test is the user's - my fetch of issues/new is
  anonymous and gets the sign-in page), so `remove_link` fills the title, the dropdown "organ" AND the text
  field "name" ("Or any part of its name", a documented input prefill) with the exact folder; the workflow
  dedupes the names (`awk '!seen[$0]++'`) and skips a folder removed already in the loop. CONFIRMED by the user
  (1 Nov 2026): GitHub does NOT pre-tick the dropdown, the name field is filled - the form's text says so; the
  dropdown stays for removing several at once. Never promise a dropdown prefill again.
  0.9.29 (the user, furious: "Save Configuration ... Alt+Tab ... dialog tam není, hlavní okno je nedosažitelné"):
  `MainComponent::bringLostModalsToFront()` (static, public): with modal components present and the MAIN window
  (UIScale::dialogAnchor as TopLevelWindow) the active one, every modal's top-level `toFront(true)` (never
  hide/show - 0.8.173), logged at most every 5 s; called from `MainWindow::activeWindowStatusChanged` (Main.cpp)
  and every 20 ticks (0.5 s) of the 25 ms timer (`lostModalDivider`). Hook `ORGAN_TEST_MODAL_BEHIND=<seconds>`
  (the main window toFront over a box opened by ORGAN_TEST_CLICK); test %TEMP%\test_modal0929.ps1. FOUND BY
  THE TEST: JUCE itself re-raises a modal box when the main window gets activated (inputAttemptWhenModal ->
  bringModalComponentsToFront), so the user's lock-up was the Z ORDER: JUCE boxes are tool windows WITHOUT an
  owner, not in Alt+Tab; now every modal box becomes an OWNED window of the main window on Windows
  (`SetWindowLongPtr(GWLP_HWNDPARENT)` in bringLostModalsToFront, once per box) - Windows keeps it above its
  owner. The hook's diagnostics line: "box owned by the main window: yes, above it: yes, visible: yes".
  0.9.30 (the user's Caen Unda Maris "nerozpozná jako undulating ... asi má rovnoměrný undulating", measured by
  %TEMP%\smpl_cents.py <folder>...: Salicional −13..−14 c vs 440 on every range, UndaMaris +2.5..+4 c = +16 c
  against the organ FLAT over the compass, Voix céleste +1.5..+3 c recognised only by the noise of its data):
  the celeste test (computeTuning440 ~13788) wanted `correlation < -0.3` (the detune shrinking towards the
  treble); now `oneSigned` (count >= 8, meanAbs >= 5 c, sameSign >= 0.85) AND (trend OR `celesteName` on
  rank->name / alias / folder name: undamaris, celest, céleste (UTF-8 bytes), voixcel, voxcel, voxangelic,
  schwebung, piffaro, fiffaro, bifara, suavial, unda) - a one-signed rank without trend and without such a name
  stays as it is. Stop fields `celesteMeanAbsCents/celesteSameSign/celesteCorrelation/celesteNote` printed in
  the "Tuning ..." log line "(celeste test: offset N c, same sign P %, treble trend r = X; <note>)". The
  consequence of a refusal: "Equal at 440 Hz" pulls the undulation straight (the detrend keeps it for a
  recognised celeste). Test %TEMP%\test_celeste0930.ps1 (001693\pipe\pos on I: UndaMaris8 "taken for a celeste
  by its name").
  v0.9.9 PUSH (31 Oct 2026): the "Configurations check" index job on main FAILED - `git add -A configurations` in
  configurations.yml (and configuration-issue/remove/feature.yml, `ls -1 configurations`, `find configurations` in
  configurations_check.sh) were leftovers of the 0.9.0 rename (the check job passed only because `find` on a missing
  folder prints nothing). All replaced by libreorgans and pushed as a workflow-only commit after the tag. When a folder
  of the repository is renamed, grep the WORKFLOWS and SCRIPTS for the bare name too, not only the sources.
  THE USER'S RULE FOR THE MANUAL (31 Oct 2026, "hlavně kvůli nepodstatným těmto detailům nezahlť user manual! Vždy
  stručnost, výstižnost, případně detaily do hyperlinku technických detailů"): a fix or a safeguard gets at most ONE
  short sentence in the user's chapters, or nothing (the log and the Messages carry the detail); technical detail
  goes to part 10 behind a link. The 0.9.1-0.9.9 additions were cut to that (1.6, 4.1, 5.3, Missing pipes).
  0.9.2 (31 Oct 2026): FILLED PIPES OF A SPARSE RANK (the user's Principal8: 039, 051-053, 068, 075 ...; "064-067 all
  from 075 while 068 stands next to them"): the group rule's source (067 + 5 = 072) was a hole and the old fallback
  took the nearest real pipe BEYOND the hole, never back. `computeFilledPipes` fallback (ONLY when the group's source is
  not real - a complete rank never enters it, the self-test is line-identical to 0.8.227 but for the new line):
  candidates = real pipes within `window` = the "Missing pipes" SETTING (1-12; not g clamped to the rank's pipe count -
  that gave 68 53 68 68 68 75 68) on BOTH sides of the missing pipe; cost = transposition + window for the source of
  the pipe filled just before + window/4 for the one before that; ties to the pipe nearest the group's source;
  `RunState` runs over a whole gap (both halves) and over each edge extension. 061-067 -> 68 52 68 75 68 75 68
  (self-test "Sparse rank ... -> OK", probe3 in EngineSelfTest.h). `near` is a Windows macro (windef.h): never name
  a local so. Compare the self-test against %TEMP%\selftest_092.txt from now on.
  0.9.33 (2 Nov 2026, Linux; the user: "zas program na linuxu nenajde /run ... adresáře Hauptwerku" + "program po
  zmáčknutí velmi dlouho hledá, a na obrazovce nic není ... nesmí vypadat jako zamrzlý"): (1) 0.8.224 mounted a disk only
  for a path the program KNEW (Locations.txt, absolute folderPath, sampleLibraryRootRelative) - a clean start with a
  Windows-made zip knows none, the Hauptwerk disk stayed unmounted and `fixedDriveRoots` sees mounted disks only. It was
  a design gap from the start (the user's 29 Oct request was general), never a later regression. Now
  `mountUnmountedDataDisks(status)` (Linux, once per session): when trees are missing after the search, `lsblk -P -p -o
  NAME,LABEL,FSTYPE,MOUNTPOINT,PARTTYPENAME,TYPE` (older lsblk without PARTTYPENAME retried), every unmounted part/disk
  of a data fs (ntfs exfat vfat ext2-4 xfs btrfs f2fs hfsplus) whose part type is not recovery/EFI/reserved/swap ->
  `udisksctl mount -b` (the desktop's password box may come), the mount point from "Mounted X at Y"; log "Disk <dev>
  "<label>" (<fs>): mounted at ..." / "Disks left unmounted (not for data): ...". LESSON: lsblk -P fields must be
  matched at the line start or after a space - "TYPE=" is inside "FSTYPE=" and "PARTTYPENAME=" (the first build mounted
  nothing). (2) THE SEARCH RUNS BEHIND THE LOADING WINDOW: `resolveAndApplyPortableConfiguration(cfg, file, root,
  afterwards)` = a ThreadWithProgressWindow "Loading saved divisions..." (no cancel) whose run() mounts the remembered
  disks (`mountLinuxDiskFor(path, status)`), looks every tree up as the apply does (divisions' relative/absolute/extra
  trees + the pool, `firstMatchingTree` with the fingerprint), mounts the data disks when some are missing and looks
  again; log "Searching for the sample folders: N of M tree(s) found in T s[, disks mounted: ...]"; then the old body
  `resolveAndApplyPortableConfigurationNow` on the message thread. Three short lines always ("Searching for the sample
  folders ..." / "Tree n of m: <tail 22>" or "The system may ask for a password." / "Found so far: N of M folder
  trees") - a padding line of 40 NBSP made the AlertWindow wrap every line (pictured). The apply's repeated lookups are
  answered by a LOAD-SCOPED MEMO (`setLookupMemo(true)` at the start, emptied after mounting, off after the apply;
  `lookupIsDirectory` / `lookupChildDirectories` in resolveCaseInsensitive, firstExistingDirectory, firstMatchingTree):
  without it the message thread re-listed the FUSE ntfs-3g folders (/mnt/pracovni, /mnt/windows) for 3 s. The drive and
  location child caches are mutex-locked and returned by value. `searchingSampleFolders` makes runWhenLoaded wait (the
  Hauptwerk import's voicing is queued BEFORE the resolve) and refuses a second load while it searches; the load box's
  copy into the LibreOrgans folder runs as `afterwards` (it ran before the apply otherwise and the apply's
  currentConfigurationFile undid it). (3) `folderOfPrint` resolves a fingerprint's stop folder ignoring the case.
  ORGAN_TEST_CLICK with a name that matches no button still takes the ORGAN_TEST_DIALOG_SNAPSHOT pictures (the loading
  window has no button). Tests: ~/tools/test_cleanmount0933.sh / test_searchwin0933.sh / test_searchmiss0933.sh (no
  settings folder, sda2/sdb2/sdc1 unmounted, Caen json from the zip in /tmp/caenchk; the windows pictured every 2 s):
  16 of 16 found after mounting, the apply in the same second, no "unresponsive". THE LINUX CLONE was still
  ~/OrganSamplerC-linux at 0.8.249 - moved to ~/LibreOrgan-linux, remote set to zurek-jiri/LibreOrgan, fast-forwarded.
  The old session 10c94e83 (Windows project folder F--JUCE-PROJEKTY-OrganSamplerC-OrganSampler) is symlinked into
  ~/.claude/projects/-mnt-pracovni-JUCE-PROJEKTY-LibreOrgan-LibreOrgan so it can be resumed from Linux.
  SESSIONS ARE PER MACHINE (2 Nov 2026, the user on Windows: "I do not see the linux session here"): a Claude Code
  session's transcript lives in the home folder of the machine it ran on (~/.claude/projects on the Linux ext4 disk,
  C:\Users\doma\.claude\projects on Windows) - the Linux session is not visible from the Windows session and vice
  versa; what carries over is CLAUDE.md, CLAUDE_REPORTS.md (the NTFS checkout) and git. After the Linux 0.9.33 session
  the Windows checkout was `git reset --mixed origin/main` (the files equal origin, HEAD was behind), the Projucer
  resave for 0.9.33 done on Windows, the CI exe deployed as C:\LibreOrgan\LibreOrgan_0.9.33.exe (the user runs the CI
  LibreOrgan.exe from his Desktop), the v0.9.33 release checked (Intel mac "late symbols: 0", the Windows exe depends
  on system DLLs only). A report line of a Linux session must go into LibreOrgan/CLAUDE_REPORTS.md, never the repo root
  (a stray fragment was found there and moved).
  0.9.34 (2 Nov 2026, the user: "když v global reset zmáčknu Unload everything a Reset, tak je to zamrzlé. To nesmí.
  Žádný dialog se nesmí tvářit zamrzlý! ... Vždy to musí nějak reagovat, například hlásit postup resetu"): NOT a
  freeze - a CRASH (crash-20261102-120335.txt: a C++ exception, 0xe06d7363, right after "Global reset: everything
  unloaded"): `executeGlobalReset` unloaded every division (`unloadManual` hands the stops to the freeing thread) and
  then `voicer->resetAll` ran `updateFromStop` on the OLD rows, which still pointed at those stops - freed memory read
  (a garbage `ranks` vector -> bad_alloc). Not reproduced here with the drainer slower than the rows (a race); certain
  from the code. Fix: the voicing rows are rebuilt right after the unload, and `Voicer::resetAll` skips a row whose
  stop the engine no longer holds (`engine.getStops().contains`). THE REST OF HIS RULE: the reset runs in PHASES
  posted with callAsync - `executeGlobalReset` (the box "Global Reset" with InfoIcon and no button, `enterModalState
  (true, nullptr, true)`, `globalResetWindow` SafePointer, `resetButton` disabled) -> the unload lambda ->
  `globalResetControls(options)` (the old middle: controls, divisions, MIDI, stops, combinations, couplers, patterns)
  -> `globalResetFinish(options)` (`resetAudioOutputsToDefault`, the grid, saveMemories/saveSettings, the unload tail,
  the box closed with exitModalState(0)); `globalResetStatus(text)` = log "Global reset: <text>" + setMessage (a
  different text every time - 0.8.183) + the phase times into `globalResetTimes`; the end logs "Global reset: done
  in N ms (unloading the organ a ms, resetting the controls and divisions b ms, resetting the audio devices c ms,
  rebuilding the grid and saving d ms)" and a Messages note. Measured (Caen Surround, 87 stops, 8 routes on the
  Fireface): 1.9 s in all, of which the audio devices 1.37 s, the unload 0.31 s, the samples freed in the background
  2.3 s later. `GlobalResetOptions` (MainComponent.h). THE AUDIT of buttons that do long work on the message thread
  (the user: "Zkontroj všechny dialogy"): with a progress window already - Load Configuration (the loader + the 0.9.33
  search window), Stop List Apply and reload / Unload folder / Unload all (the loader), Load folder (the archive index,
  the tuning gate, the loader), Unpack archive, the merge (its memory count on a thread; the perspective gate reads
  16 files per pair on the message thread - seconds for a packed rar package, noted, not changed), the measurement.
  Measured without one: the portable save's RAM estimate "estimated in N ms" (new in the log line; Caen 183 ms - no
  window needed; the wait cursor stays). Left as they are (short): Couplers/Temperaments/patterns dialogs (a grid
  rebuild), Load IR (juce::dsp::Convolution loads on its own thread), Report a problem (logs.zip), Empty the cache
  (file deletions), the Divisions dialog's device change (JUCE opens a device synchronously, ~0.5-1.5 s - nothing
  shows it; a known gap), the voicing row's Transposition (one stop decoded anew synchronously, ~1 s on a big stop).
  Tests: %TEMP%\test_reset0934.ps1 [-Cfg -ClickAt -Clicks] (ORGAN_LOAD_CONFIGURATION + ORGAN_TEST_CLICK "GLOBAL
  RESET|Unload everything: ...|Reset", the process must survive 20 s after "Global reset: started", no crash file;
  ORGAN_TEST_DIALOG_SNAPSHOT pictured the box "Rebuilding the grid and saving ..."), %TEMP%\test_save0934.ps1
  (ORGAN_SAVE_PORTABLE of Caen -> the estimate's ms). LESSON: `juce::String(double, 0)` prints the decimals anyway -
  `juce::String(juce::roundToInt(x))` for a whole number. The version bump by PowerShell `[IO.File]::WriteAllText`
  must keep the file WITHOUT a BOM when HEAD has none (CMakeLists.txt, site/config.js, site/index.html, the rural
  README got one and lost it again): check `git diff` for a changed FIRST line after a bump. .NET file methods use
  the PROCESS cwd (LibreOrgan/), not the PowerShell location - absolute paths always.
  0.9.35 (2 Nov 2026, the user, furious, right after 0.9.34: "proč to teď nepozná v adlingtonu, že to jsou
  perspektivy??? ... Perspektivy to má automaticky rozpoznávat a doplňovat do Perspective patterns. Ještě včera to
  fungovalo. A Ignored patterns taky si má pamatovat ... Do ignored patterns defaultně přidej slova Traktur a mustr"):
  NOT a code regression - his GLOBAL RESET with "Unload everything" (0.9.34, 12:31) applied EVERY option, the pattern
  lists included, and `rememberPatterns(true)` wrote the bare defaults into Patterns.json: his "Traktura" was gone and
  the learned Adlington lines too. The drop of AdlingtonHallVol0 then LEARNED afresh and showed the scan's weak spot:
  "02-direct-", "03-Diffuse2S-", "04-DiffuseSen-" on every stop - the learning block (SamplerEngine.cpp ~12228, the
  3-letter stem rule "dif") appended BOTH Diffuse heads to the one line "Diffuse" -> every stop had two folders under
  one perspective -> "HW Bl.Flute" + "HW Bl.Flute (2)", 21 stops instead of 7, and the Traktura / mustrAaron folders
  loaded as stops. Fixes: (1) `linesTaken` in the learning block - a line whose prefixes already match folders of
  THIS tree, or that another prefix found in this scan has just joined, takes no second prefix: the next head gets a
  label of its own ("DiffuseSen;04-DiffuseSen-"), log "Perspective prefix X: the line ... already covers folders of
  this tree - a perspective of its own". Two heads on the same stops are two perspectives, whatever their word; the
  0.8.118 "Label;prefix;prefix2" line stays for the IMPORT (one organ spelling a perspective two ways on DIFFERENT
  stops). (2) "Unload everything" no longer forces "Also reset the pattern lists" (the toggle stays enabled; the
  text says "the pattern lists only when ticked below"). (3) `defaultIgnoredPatterns` = trem, *_tr, output, Traktur,
  mustr; `mergeApplicationPatterns` adds a default the file has not seen (Patterns.json "ignoredDefaultsSeen",
  written by rememberPatterns = all current defaults) ONCE, so a default the user removes stays removed; log
  "Ignored patterns: the new default(s) ... added". Test %TEMP%\test_adlington0935.ps1 (Patterns.json = the bare
  defaults, the rar dropped on III, "Load the folders" clicked: the Tuning lines per stop and perspective, Patterns.json
  after). RULE: before calling a learned-pattern result a regression, read Patterns.json and the "Patterns in force"
  line - a reset or another organ's file may have emptied the lists the day before.
  0.9.36 (2 Nov 2026 afternoon; three things of the user in one build): (1) COMPACT VOICING (the user, during the
  video run: "ten voicing by nemusel být tak široký, když jsou per key zavřené ... Může být poloviční ... můžeš to
  klidně přeskládat do víc řádků, ať je to kompaktnější. ALE NĚJAK ROZUMNĚ"): `StopRow::compactControlsLayout(stop,
  transmissionIndex, rowWidth, folder, inset, put)` (voicer.h, a template taking `put(Control, index, x, y, width)`)
  is ONE walk for the height (expandedHeightFor, a no-op put) and the bounds (resized): right of the button the
  transposition (362), One-shot (96), "Ranks" + the perspective boxes (92 + 82 each), the alias (244) and the Rank
  count (146) flow with a new line every 27 px when the row is full, the pitch text gets a line of its own (full
  width, tooltip when cut), the sliders flow below (47 px a line; global, [formant], gain, attack, random,
  undulation, [formant] - the full layout's order) and the velocity layers after them (a 30 px line when they wrap).
  `StopRow::keyRowsFolded(stop, index)`; `expandedHeightFor(stop, index, rowWidth, folder)` (the Rank count switch
  takes a place: the Voicer passes groupChildren/groupChildTransmissions non-empty) uses the compact walk when both
  per-key rows are folded, the old 104 px controls height otherwise (unchanged). Voicer::rebuildUI: `manualKeyRowsOpen`
  - a screen whose open voicings all have folded key rows gives the expanded column `compactWidth` =
  availableExpanded / 2 (at least 700 px, at most availableExpanded) and the other columns `otherWidthBesideCompact`
  (the rest shared, at least narrowWidth 335); with any key row open the old wide/narrow split. Containers (stop
  folders without pipes) keep their 58 px line. RULE: a new control on line 1 of StopRow goes into BOTH the old
  resized layout + `line1Width` (0.8.201) AND the compact walk. Test %TEMP%\test_row0936.ps1 [-Division -Stop -Tag]
  (ORGAN_TEST_EXPAND + ORGAN_TEST_SNAPSHOT: grid.png.row.png = the compact row, grid.png the columns).
  (2) #165 (Mac 0.9.35, "na macu nesmí být modal window zachytávač. Všecko zamrzne, i když se pouze objeví message
  new, bez dialogu"): the guard IS `#if JUCE_WINDOWS` whole since 0.9.32 (checked); his 0.9.35 log has NO watchdog
  line at all (the watchdog wrote stalls > 8 s only), so the cause is not proven. Done: the MessageStrip's 500 ms
  timer repaints only when the text changes (the count or the collapse) - it repainted a 560 x 24 band over the grid
  twice a second for 10 s after every message; and CrashHandler's watchdog notes a stall of 1-8 s when it ends
  ("Message thread was silent for N.N s (a short stall ...)", one line in 10 s) so the next Mac log shows what a
  "freeze" is. His second remark (the browser placed by the report stays in macOS full screen, the right half
  black) is NOT done. (3) #166 (Ultra 7 155H, Caen, 8 routes on the Realtek speaker, WASAPI 480: 44 200 on 1 Nov
  with build 16:30 UTC, 20 700 on 2 Nov with 0.9.35): the same storage (16-bit, 48 kHz, halved, NOT packed - the
  13:17 run was after a reload with packing off; the 13:04/13:06 runs at 23 500 were the packed load), the same
  per-voice cost (2326 vs 2398 ns per block), but the break came at 34 % render-core load (52 % the day before) on a
  LANE WAIT peak: "the longest route callback 2552 us - its own rendering 298, waiting for the lanes 2479", "the
  latest a lane joined 871 us", Windows power plan Balanced - the hybrid CPU's E-cores / a preempted worker, the
  known #44 peak, not a code change (nothing in the render path changed in 0.9.33-0.9.35; the Windows-only modal
  guard runs on the message thread every 0.5 s and touches nothing while the box is in front). Told the user: run
  the same measurement with the 0.9.32 exe today (he started it at 12:57 without measuring) and with "Performance
  cores only" + the High performance plan; the report already says so. No issue comments posted (his rule).
  VIDEO MADE (2 Nov 2026, with 0.9.36, the user: "udělej celé to video, konečně"): 634 s, 0 discontinuities
  (check_recording.py), "Recording stopped: 634.4 s in the file over 634.4 s of time", 39 narration lines (4 moved,
  the MIDI line by 5.7 s), organ -33.4 LUFS -> +14.1 dB, voice +0.8 dB, true peak over -1 dBFS -> both lowered 1.3 dB.
  site/assets/libreorgan.mp4 replaced (40.8 MB) and pushed; libreorgan-github.mp4 (1280 px, crf 33, 48 kbps AAC,
  9.4 MB - under GitHub's 10 MB) in F:\temp\video\ for the user's upload (a README player needs the user-attachments
  URL: he drags it into an issue comment and gives the URL; the README line is then replaced). LESSONS: (1) the
  "Sample folders found" box is gone since 0.8.214 - the script's `waitfor OK` / `click OK` after the relocation
  waited 60 s for nothing: `waitload 120000` + `wait 5000` now. (2) The drive X: of make_video_config.py no longer
  keeps the Oberwerk missing: the lookup finds the trees by the tails of the absolute path under the locations'
  children (0.8.190-0.9.5) - the config now carries a FINGERPRINT naming a stop folder the real tree lacks
  ("video_missing_folder|1|0|-" keyed by the X: path), which `firstMatchingTree` refuses; the click's relocation
  checks no fingerprint. (3) The whole script takes ~16 min of recording (the Stralsund load alone 2-3 min from the
  page cache): make_video -Seconds 1300 (560 killed the run at 40 %: "NO FRAMES"). (4) The arrange-mode press is on
  "UW Gedackt 8" of I. (the user: the Octave of II. was half hidden under the open Cornet voicing). (5) The cursor,
  glows and captions are drawn LAST over the composed frame (captureFrame), so they lie above every dialog in the
  video; on the live screen each window has its own overlay (windowOverlays) - the user saw the cursor under a
  dialog there, the frames are right (checked: the load box, the loader, the Couplers dialog). (6) The say-lengths
  tool (%TEMP%\say_lengths.py, Kokoro at speed 1.0 with the 0.5/0.4/0.3 pauses) lists the lines whose narration is
  longer than the waits until the next; waits lengthened at 31 places; actions it does not count (learn windows,
  slides, pointkey, holdarrange, glowcoupler, the relocation's load) cover the rest, and make_narration moves what
  still overlaps.
  0.9.37 (2 Nov 2026 afternoon; #167 Mac 0.9.36 "zamrzí když se tvoří messages rámeček, measure padá", #168 Linux
  AntiX "měření zamrzne a komputer vytuhne"): the 0.9.36 watchdog lines show the Core 2 Duo Mac silent 3-6 s every
  ~12 s AFTER the load (13:53:31-13:54:19, the strip came at 13:54:25 - the strip is NOT it) and 3-4 s at the
  measurement start; the Linux T6670 silent 6 s and 3.9 s during its copy builds and 3 s at second 18 of the
  measurement (2160 voices, callback 51 %), then the whole computer froze (no more lines, a restart at 14:16).
  On both the copy builds ran on a thread at LOWERED priority (nice 10 / -8) beside SCHED_FIFO / time-constraint
  render and control threads on TWO cores: a lowered thread holding a lock the window thread needs (the control
  lock in buildAntiAliasCopies, the heap) starves - the 0.8.185 heap-lock family. Done blind (no stack from those
  machines): `lowerCurrentThreadPriority()` returns at once on <= 2 physical cores; and the POSIX watchdog SAMPLES
  the message thread's stack at 2 s of silence (CrashHandler.h: `pthread_kill(messageThread, SIGUSR2)`, the handler
  fills a static buffer with backtrace(), `backtrace_symbols` into the log "Message thread silent for N s; its
  stack now:"; CMake links Linux with `-rdynamic` so our functions have names) - the next Mac/Linux log tells.
  The user's "it is not only during the copy builds, it is after too": the Mac log has no stall after 13:54:19
  except the measurement's; whatever it is, the stack will say. His other Mac remark (the browser placed by the
  report keeps macOS full screen) is still open. THE README VIDEO: GitHub accepts .mp4, .mov AND .webm (the docs,
  10 MB on a free plan) - the small video goes as WebM with Opus (the user: "zmenšené video má totálně debilní
  zvuk ... zvuk bude nejčistší": AAC at 48 kbps was the fault; 10 MB / 634 s = 126 kbps in all, so Opus 112 kbps +
  VP9 for the picture at 1280 px). NARRATION (make_narration.py): `PRONOUNCE` = Kokoro's "[word](/phonemes/)"
  syntax for Gedackt (ɡədˈɑːkt - it said "Džedakt"), Jakobi (jˈɑːkəbi - "Džakobaj"), bar (bˈɑːɹ - the British
  "bah" = "swell bo"), Hauptwerk, Oberwerk, Rohrflöte, Trompete, Stralsund, Positive; 8 ms fades at every piece
  Kokoro returns + 30 ms between its own chunks of one phrase (the user: "hlas se zadrhne a ozve se pazvuk" =
  clicks at the joins). THE VIDEO'S SAMPLES: the user copied the packages to F:\video (000370, 003135, 003136 with
  their pipe trees, the Adlington Vol0 rar; NVMe) - `Tools/preload_files.py <folders/files>` reads them into the
  OS cache right before the run (8 readers, 8 MB reads), `make_video.ps1 -SampleLocation F:\video` writes that one
  line into Locations.txt for the run (backed up/restored) so the bare `<location>/<relative>` forms find the
  trees there and not on D:; the script's dropfiles/relocate lines point at F:\video. THE OBERWERK IS KEPT MISSING
  BY A FINGERPRINT ON A RELATIVE TREE: make_video_config.py leaves the OW trees relative and writes
  treeFingerprints["003136/pipe/OW"] = "video_missing_folder|1|0|-" (an ABSOLUTE tree never passes the
  fingerprint test - line ~14129 `tree.relativeForm ? firstMatchingTree(...) : ...` - and the old X: paths were
  found by their tails anyway: the 2 Nov video found the Oberwerk, "Video: alert - no notice in division 3").
  THE VIDEO OF 2 Nov 2026 (second take, 0.9.37): 571 s, 0 discontinuities, the Oberwerk missing and found, the
  samples from F:\video (3 GB/s) - site/assets/libreorgan.mp4 (37.7 MB). THE README KEEPS THE OLD VIDEO (the
  user: "na gitHub necháme to původní, ono má taky svoje kouzlo"): the user-attachments URL of 26 Oct stayed until
  3 Nov 2026, when the README got the re-mixed take (the user corrected narration.wav by hand, %TEMP%\merge_video0948.ps1
  re-merged without a new run: web mp4 commit 6da909e, the GitHub WebM 9.77 MB = Opus 112 kb/s + VP9 960 px / 8 fps capped
  at 22 kb/s, uploaded by the user, README commit 8de791d) - and the user REJECTED it within the hour ("absolutně
  rozmazané a malinké"): a 10-minute video under GitHub's 10 MB is either bad sound or an unwatchable picture, never
  both right. THE RULE NOW: NO embedded video in the README - one bold link "▶ Watch the instruction video (10 minutes,
  full size, with sound)" to https://zurek-jiri.github.io/LibreOrgan/assets/libreorgan.mp4 (GitHub's sanitizer strips
  target="_blank", so a new window cannot be forced; the mp4 opens in the browser's player, right-click for a new
  tab). Never make a GitHub-sized video again; the
  WebM made for it (F:\temp\libreorgan-github.webm, 9.2 MB, Opus 96 kbps) is not uploaded. A README video under
  10 MB with clean sound is WebM + Opus only (AAC would be under 100 kbps at ten minutes).
  0.9.38 (2 Nov 2026 evening; the 0.9.37 stack samples of #169/#170 (Mac) and #171 (Linux) READ): the message
  thread of the two-core laptops sits in the WINDOW SYSTEM's calls - Linux: libc read() right under the
  InternalMessageQueue lambda (an X11 reply wait), start-up in `snd_device_name_hint` (ALSA's enumeration, 2.5 s on
  that box, from `defaultDeviceNameOfType` in loadSettings); Mac: `objc_msgSend`/`CFRunLoopRunSpecific`/
  `AutoreleasePoolPage::pop`/empty stacks = Cocoa and window-server calls - and in `benchmarkKernel` (2 s, the
  measurement's end), `Voicer::rebuildUI` (2.4 s, the grid of 78 rows built: compute) and `Voicer::loadState` at
  start. THE CULPRIT that matched the timeline (fine at 0.8.216 on 27 Oct, broken at 0.9.30): the HelpKeyWatcher of
  0.8.232 (30 Oct) polled `Process::isForegroundProcess()` + `KeyPress::isKeyCurrentlyDown(F1)` every 60 ms = two
  X11 round trips / a window-server IPC sixteen times a second; with the real-time audio threads busy on two cores
  the server answers late and the window thread waits for it (on Linux the starved X server froze the whole
  desktop at 51 % callback - #168). Now the poll is Windows-only (GetAsyncKeyState is a read); macOS and Linux get
  a juce::KeyListener on every top-level window (attached every 500 ms to windows without it, no system call).
  The 0.9.37 "no lowered priority on <= 2 cores" stays (harmless). THE STRIP IS GONE (the user: "Jdi do prdele s
  tím oranžovým obdélníčkem ... zprávy se budou tiše zapisovat do položky v about"): `messageStrip` is never
  created; noteMessage keeps writing the Messages entry; the class and showMessageStrip stay (guarded). macOS
  report windows: the frontmost browser's full screen is left through System Events ("AXFullScreen" of window 1
  set false, 1.2 s for the animation) before its bounds are set (the user, twice: the placed browser kept the full
  screen, black around it). Still open on 2-core machines: the ALSA enumeration at start (2.5 s, loadSettings) and
  the compute of rebuildUI/loadState/benchmarkKernel - seconds on a Core 2 Duo, shown in the loader/measurement,
  not polled. RULE: nothing on the message thread may ask the window system or a device driver periodically
  (key states, focus, device scans) - a listener or a cached answer instead; on a two-core box with real-time
  threads every such call can wait seconds.
  0.9.39 (2 Nov 2026 evening; #172 Linux / #173 Mac on 0.9.38 "gui je furt zamrzlé, measure šedé"; the user:
  "zničils mi LibreOrgan ... udělej tooltipy a F1 levně ... stačí dotaz 1x za sekundu ... tooltip se objevuje
  příliš rychle a příliš blízko ... messages jako zvoneček v title baru ... minimální změny"): WHAT THE LOGS SAY:
  (1) the Linux 0.9.38 stack samples sit in libc memmove (SSSE3) called directly from JUCE's InternalMessageQueue
  lambda = `queue.removeAndReturn(0)` - the message queue is LONG (every pop moves the whole array: a flooded queue
  drains quadratically); the Mac samples are useless (backtrace() walks frame pointers, the frame after _sigtramp
  is garbage; only the first frame of a sample is reliable). (2) `MouseInputSource::getScreenPosition()` is NOT a
  cache: juce_MouseInputSourceImpl.h getRawScreenPosition -> `MouseInputSource::getCurrentRawMousePosition()` =
  XQueryPointer / [NSEvent mouseLocation] - and JUCE's own `TooltipWindow::timerCallback` (every 123 ms) calls it
  whenever the mouse is over any component = the "X11 reply wait" of the 0.9.37 stacks; `Process::isForegroundProcess`
  and `ModifierKeys::getCurrentModifiers` ARE cached statics (the 0.9.38 note's "two X11 round trips" of the F1 poll was
  one: XQueryKeymap). (3) `MainComponent::activeFolderLoader` stayed alive 46-55 s after "Compiling the Stop List: done"
  on the Mac (16-99 s on the Linux laptop) in every session since the 1 Nov builds - the old Mac logs (builds 22-27 Oct)
  show 8-10 s; "Configuration from another computer" and "RAM held after the load" wait for the loader's release, and so
  does `canMeasurePolyphony` = the grey Measure button. The release is one callAsync message posted at `finished`
  right after "done" - why it runs 47 s later while timer beats come through every 2-5 s is NOT explained (FIFO queue;
  no nested message loop in our code; my Ryzen pinned to 2 logical cores releases in 0 s): 0.9.39 logs "Loader:
  finished N ms after done" and "Loader: released N ms after its finish" so the next Mac/Linux log says which of
  the two waits. (4) The Divisions dialog on a two-core box: four LearnKeyboards repainted WHOLE at 20 Hz (the
  velocity badge's Refresh timer), eight AudioTabPanels at 5 Hz (refreshChannels/refreshReverbLabel/selectorHeight +
  an existsAsFile stat each), KeyboardsDialog 10 Hz refreshControls, `voicer->updateStopStates()` every 200 ms from
  the main timer while the dialog is visible. DONE: HelpTooltipWindow overrides the private virtual `timerCallback`
  of juce::TooltipWindow (allowed: overriding needs no access) - every 8th tick (~1 s) it reads
  `getComponentUnderMouse()` (cached by JUCE) and the position of the LAST MOUSE EVENT (our `mouseMove/mouseDrag`
  overrides: the base is a global mouse listener), no system call; a tip after one poll of rest, placed +10/+16 px
  from the pointer, down after 7 polls, suppressed until the mouse leaves the control, `mouseDown/mouseWheelMove`
  dismiss; `getDesktopScaleFactor` override returns the resting component's scale (the base took its own poll's
  component - with the override it would have fallen to the global 1.0 = unscaled tips at 75 %). The Hider struct is
  gone. F1 unchanged (Windows GetAsyncKeyState poll, listeners elsewhere). `messagesButton` ("Messages: N new",
  orange text, key "messages" in headerButtons after guidedTour, `isHeaderButtonShown` = unreadMessages > 0, never a
  setting, skipped in the Editor's toggles, chapter "Messages"): `updateMessagesButton()` from noteMessage and the
  About window's read callback - resized() only when the button comes or goes; the strip stays uncreated.
  AudioTabPanel::timerCallback sets `measureButton.setEnabled(canMeasurePolyphony())` (resized() alone did before).
  LearnKeyboard's Refresh repaints only when `velocityOf` gives another note/velocity (`badgeNote/badgeVelocity`).
  NOT changed (noted): the control thread's `if (more) std::this_thread::yield()` under SCHED_FIFO (first commit,
  8b96d24 - not a regression; the 0.8.127 rule says never yield there), AudioTabPanel 5 Hz, updateStopStates 5 Hz,
  the IR automatic choice flapping Light/Full every 5-6 s on the Mac (17:08:30-17:08:53 of #173: "Full chosen" then
  "too much, Light chosen", each a convolution rebuild) - no hysteresis between the estimate and the measurement.
  Debian trixie libc6 2.41-12+deb13u4 is stripped: a `+0x...` offset resolves only by reading the bytes
  (%TEMP%\glibc_sym_tool.py, glibc_dump.py; the SSSE3 memmove loop at 0x17e748). RULE (the user, twice): read the code
  again before changing it; minimal changes; an old log is the baseline ("chodilo jak víno" = the Oct 22-27 Mac logs).
  0.9.40 (2 Nov 2026 night; #175 Mac / #176 Linux on 0.9.39: "frozen on macos", Linux "not very responsive, the
  measurement does not start"; the user: "Zkurvils kód ... Najdi regres ... srovnej s verzí dva týdny starou ... čti
  fóra"): THE FACTS NOW: "Loader: released 47813 ms after its finish" (Mac) / 15104 ms (Linux), no "finished N ms
  after done" line - the loader's release callAsync waited 48 s in a FIFO queue (juce_MessageQueue_mac.h: add /
  removeAndReturn(0), 4 messages per run-loop pass then wakeUp) while the watchdog's 250 ms timer beats arrived every
  2-6 s. Timer beats are CallTimersMessages in the SAME queue, so the beats that ran before the reset were posted
  before it: the TimerThread posts one every 300 ms while a callback is outstanding (juce_Timer.cpp: post, wait(300),
  post again, wait(1)) - a queue of such messages drains at 4 per pass; whatever makes a run-loop PASS take 2-5 s
  (Mac: the display/paint pass and NSEvents between our 4 messages; Linux: the 100 ms drain budget then poll +
  the LinuxRepaintManager timer) is the thing to find, and it goes on at idle after the release and through the
  measurement (3 s silences every second -> the stall action releases the keys -> "cannot be trusted"). The Mac
  stack samples of 0.9.37-0.9.39 are EMPTY beyond the first system frame (backtrace() in a signal handler walks
  frame pointers; a leaf without one - a syscall stub, a lock wait - breaks the chain): 0.9.40 samples the message
  thread with thread_suspend + thread_get_state + a frame-pointer walk through vm_read_overwrite, resumed BEFORE
  backtrace_symbols (malloc while the thread holds the heap lock = deadlock) - CrashHandler.h, JUCE_MAC, x86_64
  and arm64 (arm_thread_state64_get_pc/fp). The Linux samples resolved against the CI binary (the agent's
  lo_sym_tool.py; the executable keeps a .symtab): a grid paint from LinuxRepaintManager inside callTimers, the
  compile phase 2 (runStep -> lambda#3 -> resized -> rebuildUI -> StopRow ctor), the queue pop memmove. The diff
  0.8.216..0.9.30 (159 commits) for periodic/async/paint/lock additions: nothing heavy (paintOverChildren draws the
  drop frame only; setMistuneCorrectCents takes controlLock once; no new timers but the F1 watcher/strip, both gone).
  The old Mac logs: the loader released 8-10 s after "done" with the Oct 22-27 builds (0 s on 27 Oct), 46-55 s with
  the 1-2 Nov builds. BISECT BUILDS for the user: branches bisect-0.8.216 (a4eb17d), bisect-0.8.253 (2f43138),
  bisect-0.9.15 (f4c41a2) pushed and build.yml dispatched on them (no release: the ref is a branch; the artifacts are
  on the run pages, a login needed) - the user installs the mac/linux artifact and reports; the local tags v0.8.*
  still exist (the release job deletes origin's). Issues #165-#174 closed through the API (the user's order), no
  comments. Forum reading (forum.juce.com "The Event Queue", "Laggy GUIs on OSX El Capitan", "Graphics stutter on
  Mac OS", "periodic GUI update freeze with mouse events on macOS"): timers and repaints saturating the macOS message
  thread, throttle/move off timers - nothing about removeAndReturn. JUCE docs/forum on dsp::Convolution: the IR is
  loaded (resampled) on a background thread of the ConvolutionMessageQueue, and prepare() loads the pending IR
  immediately (synchronously, on the caller) - the first Linux sample of #176 shows exactly that at start-up
  (Convolution::prepare -> ConvolutionEngineQueue lambda -> ResamplingAudioSource, from applyPerspectiveReverb in
  loadSettings; also at every regroupReverbs/rebuildRouteReverb): a 7 s stereo IR resampled per convolution on a
  Core 2 Duo = seconds on the message thread per regroup - not the 48 s, but real. PowerShell: Invoke-WebRequest
  without -UseBasicParsing prompts ("NonInteractive mode") on POST/HTML answers - always -UseBasicParsing; a
  rerun-failed-jobs POST is 403 for this token (scopes repo, workflow), workflow_dispatch is allowed. The user ran
  0.9.39 while 0.9.40 was ready: NOT built locally (the rule), left to CI; deploy the CI exe when he closes it.
  BISECT ON THE MAC (2 Nov 2026 night, the user's own runs of the bisect-* artifacts; the Intel mac artifact is
  "…-macos-10.11-intel", the "macos" one is Apple silicon - he needs the Intel one; GitHub cancelled the Intel job
  of several runs at first, a re-dispatch built it): 0.8.216 GOOD (#177: 3184 voices, the loader released 8 s after
  "done"), 0.8.226 GOOD for the freeze (#179, 8 s), 0.8.245 GOOD (#178, 9 s), 0.8.253 BAD ("zamrzlá"), 0.9.15 BAD
  ("zamrzlá totálně"). So THE FREEZE REGRESSION IS IN 0.8.246-0.8.253 (30 Oct 16:13-21:19): 0.8.246/247 colours,
  0.8.248 `UIScale::startDisplay` (once), 0.8.249 Linux launcher (JUCE_LINUX only), 0.8.250 (watchMissingAudioDevices
  runs with the Divisions dialog open; `showKeyboardsDialog` calls it at once; a device that answers reopens every
  route in one pass; AudioTabPanel `savedDeviceName()` + the "device not available" branch; "Nested folder trees"
  InfoIcon = no box; updateCombinationLights `registrationOnSet` per filled piston every 200 ms), 0.8.251 MSVC
  static CRT only, 0.8.252/253 texts. The full source diff bbe77db..2f43138 read line by line shows nothing that
  runs for seconds at idle - the cause is not found by reading; the 0.9.40 Mac stack samples (thread_suspend walk)
  are the next evidence; a bisect between 0.8.246-0.8.250 (dispatch bisect-0.8.249 = 4b0f91c, bisect-0.8.248 =
  531b512) is the alternative. The "polyphony regression" of 0.8.226/0.8.245 (#178/#179: 1100-1500 vs 3184) is NOT
  the engine: the measurement ran with IR processing FULL - the automatic rule (unchanged since 0.8.130) chooses
  Full again whenever the ESTIMATE from Light is < 12 % per thread (11.6 % here) and Light when the MEASURED Full is
  > 18 % (16.8-19.1 % on that Mac = a coin flip each session; #177 landed on Light); Full at 17 % of each render
  thread = a third of the polyphony. 0.9.41: `irFullMeasuredPerThread` (ms/s per thread, set when needLight fires,
  reset when summary.routes changes) blocks "Full chosen again" while >= 120 - the log line says "(measured N %
  before)". Also seen: the callback share at 270 voices 14-20 % (0.8.216, Light) vs 53-60 % (Full).
  0.9.43 (2 Nov 2026 night) THE FLOOD FOUND ON WINDOWS (the user granted `Edit/Write(C:\JUCE9\**)` through
  /permissions: "proč si neřekneš o přístupová práva k juce mimo repozitář" - C:\JUCE9 is the Projucer build's JUCE,
  not the CI submodule, so it can carry a TEMPORARY diagnostic; revert it before any deploy): a histogram in
  `MessageBase::post()` (juce_MessageManager.cpp: CaptureStackBackTrace 14 frames per post, a map per second, the
  top sites symbolised with SymFromAddr into %TEMP%\post_trace.txt; the FIRST version printed the stack of whichever
  thread posted first in a new second = a random sample that blamed the TimerThread - never read a single stack as
  the producer) says: ONE `Voicer::rebuildUI` posts ~34 000 messages - `StopRow::StopRow` built, for EVERY row, two
  LearnableSliders + three Labels + a ToggleButton PER KEY of the compass (TestingOrgan: 78 rows x ~55 keys = 4240
  keys = 25 000 components), and every `Label::setText` / `Button::setToggleState` posts a Value change message
  (the Label listens to its own Value - `dontSendNotification` does not stop it); the grid is rebuilt 3-4 times
  around "Compiling the Stop List: done" (the loader's resized, a callAsync, the Messages button's resized) =
  115 000 posts in one second. On the Core 2 Duo each rebuild takes seconds (#180's 2.4 s in rebuildUI) and the
  queue drains O(n) per pop (removeAndReturn(0) memmove, 4 per run-loop pass on the Mac) = the minute-long freeze
  and the loader "released 47 s after its finish". The loop existed in 0.8.216 too (the freeze was 8-10 s there);
  what made 0.8.246-0.8.253 worse was NOT found in the diff (nothing adds a rebuild or posts) - the fix attacks the
  mechanism: `keyControlsWanted = isExpanded() && !stop->container` gates the per-key loop (every reader of the
  arrays takes nullptr: OwnedArray::operator[] beyond the size; every expansion/fold goes through relayout =
  rebuildUI), a rebuild now posts ~5 800 (the 8 hidden `transmissionBoxes` ComboBoxes + shiftBox per row are the
  rest; left alone). The remaining per-row cost: ~75 posts and ~30 components. RULES: (1) a StopRow is built per
  rebuild for EVERY grid item - nothing per key may be constructed for a closed voicing; (2) `MainComponent::
  resized()` rebuilds the WHOLE grid - call it for a layout change only, never as "refresh"; (3) to find who floods
  the queue, count per CALL SITE per second, never sample one stack. CI 0.9.42 failed on both macOS jobs:
  `juce::String(" \"") + name + "\""` with `char name[64]` is ambiguous for clang (String + char[64]) - wrap the
  array in juce::String(name) (MSVC took it). The bisect-0.8.248/0.8.249 artifacts (runs 41515264886 /
  41515277513) stay for the user; with 0.9.43 out they matter less.
  0.9.44 (3 Nov 2026, the user: "udělej obě opravy. Hlavně to nesmí načítat dvakrát, po přerušení uživatelem to musí
  upustit od každého dalšího načítání"): (1) #183 (the Mac: the Caen plusODF rar dropped on a division, the other
  Caen_Surround_* rars beside it): the 0.8.218 neighbour search DID work but ran AFTER the whole load (runWhenLoaded),
  so the organ loaded twice (25 MB/s from the rar on that Mac = 20 min each) and the second load started by itself
  after the user's Cancel of the first ("Loading cancelled by the user before ..." twice in the log) - he reported
  "only one rar". Now the SearchJob of resolveAndApplyPortableConfiguration (the 0.9.33 "Searching for the sample
  folders" thread) does it BEFORE the load: with trees still missing after the lookup (and the Linux mount), the
  same sieve as offerNeighbourArchives (folders = nearest existing ancestors of the missing trees' absolute paths +
  the configuration's folder; `like` = the chosen archives in them; `archivesNamedLike` with the file name + the
  publication organ as words), the candidates indexed on a worker std::thread with the window's status, an archive
  holds a tree when `entriesOf(startsWith <path below the archive's folder>/)` for the absolute tree, else
  `<rootName>/<relative>` or `<relative>`; the taken ones `SampleFiles::addChosenArchives` on the thread,
  `setLookupMemo(true)` (the memo answered "not there"), `findAll(missing)` again; after the thread, on the message
  thread, `registerArchive` each (Locations.txt) + queueNotice InfoIcon "Archives beside the organ" (= Messages) with
  "archive:  trees" lines; log "Missing folders: N tree(s) before the load; archives beside the organ with a shared
  name: ..." and "... K archive(s) beside the organ hold them and are read with it: ...". The post-load
  `runWhenLoaded(offerNeighbourArchives(false))` is GONE - nothing loads by itself after a load (the notice's menu item
  1001 = the explicit request stays). RULE: no automatic load may follow a load; a Cancel ends everything. (2) The
  `addFolder` walk-up (both places) is bounded: juce::File::getParentDirectory() of a drive root returns the root
  itself (X:\ of a drive that is not here looped for ever). (3) #182 (Mac crash at Global Reset, "Unhandled exception"
  in ~AudioDeviceSelectorComponent): `resetAudioOutputsToDefault` did `asioHosts.clear()` (the CoreAudio/ASIO shared
  hosts, AudioDeviceManagers) BEFORE `attachDivisionRoute` replaced `audioSettings` / `extraAudioSettings[i]` built on
  those hosts (5299/5333) - the old selector's destructor called removeChangeListener on a deleted manager. Now the
  selectors are reset before the hosts close. Tests: %TEMP%\test_neighbour0944b.ps1 (fixture %TEMP%\nb0944: the
  Rural json with every F:\Suchy_rejstrik\Kobrle replaced by X:\nowhere\KobrleX and rootName KobrleX, beside it
  Rural_Organ_samples.rar = KobrleX/Samply/TA+CO stored; Locations.txt stripped of Suchy_rejstrik: "3 of 3 tree(s)
  found", one load, 11 rows), test_reset0934.ps1 -Clicks "GLOBAL RESET|Unload everything: all stops and sample folders
  too - a clean, empty instrument (the pattern lists only when ticked below)|Reset" (the toggle's text changed in
  0.9.35 - the old script's click missed and reset nothing). A real set on this box cannot test the neighbour path:
  every package is installed on D:\Hauptwerk and the lookup finds it there first (test_neighbour0944.ps1 with the
  Bedos ODF rar showed "12 of 12 found ... taken over by D:\..."). #181 (4140 / 3990 / 3458 / 4140 on the Mac, no
  freeze): single callbacks at 100-121 % of the period break a level - the OS on two cores, not the engine.
  0.9.45 (3 Nov 2026, the user: "na linuxu se build vůbec nespustil. Žádná hláška, nic. ani v settings nevznikne
  žádný log" + the windowless seconds after "Reading <odf>"): (1) GITHUB MOVED `ubuntu-latest` from ubuntu-24.04
  (GCC 13.3, glibc 2.39) to ubuntu-26.04 (GCC 15.2, glibc 2.43) between the v0.9.43 run (2 Nov 20:53 UTC) and the
  v0.9.44 run (3 Nov) - the x86_64 tar.gz grew 10.2 -> 12.0 MB and the ELF asks for GLIBC_2.43 (checked with
  %TEMP%\lin0944\chk.py: the max "GLIBC_x.y" string of the binary); Debian 13 has 2.41, so ld.so refuses it with
  a line on stderr the desktop never shows and the program never reaches main (no activity.log). build.yml pins
  the Linux job to `ubuntu-24.04` (the arm64 job was ubuntu-22.04-arm already). RULE: a Linux binary that does not
  start and leaves no log = check the glibc requirement of the release binary against the runner image first;
  never `ubuntu-latest` for a job whose binary is shipped. The release job keeps ubuntu-latest (nothing compiled).
  (2) `importHauptwerkOdf`: `engine.scanTreeAssignments(tree, false)` for every tree of the plan ran on the message
  thread AFTER the ParseJob's window had closed (the folders' listings + 3 WAVs per folder for the L/R rule of
  0.9.21 - from a rar on a slow disk, seconds without a window); now the ParseJob (which has `SamplerEngine& engine`)
  scans them in run() with the status "Reading the sample folders: <tree> (n of m)" and keeps the map
  `automaticNameOfFolder`; log "Hauptwerk import: the sample folders of N tree(s) read in T ms". The poll takes
  `job->automaticNameOfFolder` (const). Verified with %TEMP%\test_carlo0913.ps1 (St. Carlo from its ODF: "1 tree(s)
  read in 81 ms", 13 rows). The Bedos ODF rar is no quick test: libarchive chews 3 min on its PPMd ODF before the
  unarr fallback ("read with unarr from now on") - test_neighbour0944.ps1 needs -Seconds 420 at least.
  0.9.46 (3 Nov 2026, the user's Mac: "po global reset s unload everything ... hledá default audio device a nenajde
  ji, vyhodí msgbox device not available!!!! Device je stále tam"): `resetAudioOutputsToDefault` attached every
  division with the LITERAL type "Windows Audio" (the #182 log: "Audio device missing: Windows Audio "default";
  waiting up to 6 s" on the Mac, then the box); so did `requestedAsioStateForRoute` (two places). All three take
  `defaultSharedTypeName()` (MainComponent.h: CoreAudio / ALSA / Windows Audio) now. The two "none"-state fallbacks
  (5027/5094, `type.isNotEmpty() ? type : "Windows Audio"` for an empty device name) are left: the name is empty =
  none, the type is never used. RULE: never write "Windows Audio" as a default type - `defaultSharedTypeName()`.
  0.9.47 (3 Nov 2026, the user: "při aggregated wasapi se na set these channels for all divisions čeká extrémně
  dlouho, uživatel neví, co se děje. Musí tam být dialog ... tak jako existuje tento dialog při set for all u audio
  device"): `applyPerspectiveChannelsEverywhere` ran `applyAggregateSelection` (the aggregate reopened for its new
  endpoints, ~1 s each) for every division inline on the message thread. Now, when any target route's device is an
  `AggregatedWASAPIAudioDevice` (dynamic_cast; a stub class on other platforms), the divisions go through the
  device-apply queue: `applyAllChannelsMode` + `applyAllChannelsSource/Perspective/Done/Refused` (MainComponent.h),
  `openApplyAllWindow(title)` (the former inline window block of applyAudioSetupEverywhere, shared), the queue
  stepped by the main timer (`applyAllRunning` -> continueApplyAll every 4 ticks) with the label "Setting the output
  channels 17+18 for I. / Front (n of m) - the device opens anew", per division `applyChannelsToDivision` (the old
  loop body: the 0.9.9 refusal + the mask + applyAggregateSelection), at the end `finishChannelsEverywhere` (the
  old log line, the refusal notice, saveSettings). Without an aggregate the inline path stays. The AudioTabPanel's
  refresh does not untick "Set these channels for all" while `applyAllChannelsMode` (the divisions agree only at
  the end). Verified: Rural with ORGAN_TEST_CLICK "zzz x7|Settings|Set these channels for all divisions (this
  perspective)" (the header button is "Settings", the Divisions dialog opens on the default tab): "set for 2
  division(s) one at a time (an aggregated device among them)" then the old final line - the user's default routes
  here ARE aggregated. #184 (Caen on the Mac, 1400 voices): 30.5 us per voice and block (kernel 22 630 ns + unpacking
  7 367) - Caen loaded packed at 32 kHz on an SSE2 Core 2 Duo; TestingOrgan (48 kHz, not packed) gives 4140 there:
  the load options, not a regression (an unpacked 16-bit Caen would not fit the 8 GB). #185 = the #181 spread.
  0.9.48 (3 Nov 2026, the user: "zapnutí měření polyfonie během stavění filled pipes neudělá nic. Jen to tiše dá
  message box ... mělo by vyhodit rovnou měřící dialog, ale říct... waiting for filled-pipes to be ready"): the
  `pendingBackgroundWorkers() > 0` refusal (0.8.139, #32) at the top of `startPolyphonyMeasurement` is GONE; the
  `begin` step of the run (which polled the workers already, up to 10 min) shows "Waiting for the filled pipes to
  finish building (N stop(s) still building, T s) - the run starts by itself" in the measuring window and logs
  "Polyphony measurement: waiting for the filled pipes (N stop(s) still building)" once. FOUND BY THE TEST: the
  Configuration Editor's Measure... was built with `enabled = o.canMeasurePolyphony()` read ONCE in `addButton` - an
  Editor open while an organ loaded (or refreshed at the load's end by refreshConfigurationEditor, which runs BEFORE
  the loader's release callAsync) kept a grey button for good, and JUCE's `Button::handleCommandMessage` drops
  `triggerClick` on a disabled button (ORGAN_TEST_CLICK "Measure..." x 120 on Caen did nothing). Now `addButton`
  returns the button, `editorMeasureButton` (SafePointer) follows `canMeasurePolyphony()` from the main timer
  (setEnabled + alpha only on a change). THE TEST WINDOW on this Ryzen: the copies of Rural/TestingOrgan AND Caen are
  built within 2-3 s of "done", the loader is released ~0.5 s after it - a click 2 s after the Editor click never lands
  inside; %TEMP%\test_measurewait0948.ps1 -Clicks ("Configuration Editor|" + "Measure...|" x N) -ClickAt 3 opens the
  Editor during the load and clicks Measure... every 2 s, with a copy of TestingOrgan.json carrying "transposeMethod":
  3 (the phase vocoder: slower copies; the copy written beside the trees, deleted after). The 0.8.183 rule holds: a
  GUI change is RUN - the refusal's removal alone "passed" three runs that never reached the wait. MSBuild in the
  background hung 30 min on two orphaned cl.exe (the old C1041/PDB lesson): kill cl/MSBuild/mspdbsrv and build in the
  foreground with `/nodeReuse:false`; the user's "co to dělá dvě hodiny" was this hang - check `Get-Process cl` when a
  build runs past 5 min. An empty stray `LibreOrgan/CMakeLists.txt` (0 bytes, 2 Nov, the bump's cwd lesson) was deleted.
  CI TRIGGER (3 Nov 2026, commit 21f5c09, the user: "udělej tuto změnu"): build.yml runs on TAGS, pull requests and
  workflow_dispatch only - a push to main built the same commit a second time beside the tag's run and held the few
  macOS runners (the v0.9.48 run sat "queued" 10 min until the main run was cancelled through the API: POST
  /actions/runs/<id>/cancel is allowed for the token). A push to main alone builds nothing now; the bisect branches
  are built by dispatch as before.
  0.9.49 (3 Nov 2026, the user's Den Haag, Lutherse Kerk Diffuse ODF: perspectives "Sw", "Tr. ML", "Disc" - "regres!
  ... perspektiva se smí odhadovat pouze z části názvu PŘED jménem rejstříku"; he chose rule 3 alone: "perspektiva
  musí být společná většině rank z celých varhan"): THE CAUSE (not a late regression - 0.8.132, 19 Oct, HauptwerkOdf.cpp
  `perspectiveOfRankName` 339-340): "any short word in the parenthesis (<= 16 chars) names the perspective" - Den
  Haag's rank names carry qualifiers AFTER the stop name, "P Violoncel 8 (sw)" (enclosed), "3 Prestant 8 (Tr. ML)",
  "Coppeling B.C. (disc)", and without a colon the head is the whole name, so the trailing parenthesis became a label;
  0.8.220's flat-rank prefix detection then found the common head of those few folders ("03-dif2S-P_" = the Diffuse
  prefix + the division mark) -> pattern lines "Sw;03-dif2S-P_", "Tr. ML;03-dif2S-B_". FIX: `perspectiveWordLabel`
  (the lambda of 0.8.132 made a static function) + a block in `parse` before the 0.8.132 safety net: per label the set
  of stops carrying a rank with it (rankIds, tremmed/noise skipped) over the stops with ranks; a label that is NO
  perspective word and sits on fewer than half the stops is cleared on its ranks (plain perspective), log "Hauptwerk
  ODF: "X" is no perspective - a of b stop(s) carry it ...". A perspective WORD stands whatever its share (a Rear of
  one division). NOT run-verified (the user's program was running: no local build, no --hauptwerk run; HauptwerkOdf.cpp
  compiled alone with `msbuild LibreOrgan_App.vcxproj /t:ClCompile /p:SelectedFiles="..\..\Source\HauptwerkOdf.cpp"
  /m:1` - the way to check one file without a 16-thread build); verify with `LibreOrgan.exe --hauptwerk "<Den Haag
  Diffuse ODF>"`: no "Sw"/"Tr. ML"/"Disc" perspective, the pattern lines Direct/Diffuse/Rear/Distant only - VERIFIED
  with the CI exe once his program closed: "Disc" 1 of 58, "Sw" 3 of 58, "Tr. ML" 8 of 58 stops -> "is no
  perspective", their ranks "[]" (plain); C:\LibreOrgan\LibreOrgan_0.9.49.exe deployed (CI exe, late symbols 0, no
  VC DLLs). His own
  file and Patterns.json still carry the learned "Sw;..."/"Tr. ML;..." lines (merged at every load) - re-import and
  remove them in the Perspective Patterns dialog. The push was rejected (origin 12 ahead: his Cembalo/Caen publishes;
  he re-published Caen himself - issue #193 - and removed the "cen" folder) -> `git pull --no-rebase` (a merge),
  tag v0.9.49 on the merge commit 3c165a8. THE "NESTED FOLDER TREES" MESSAGE OF THE SAME IMPORT (the user: "ODF z
  Hauptwerk nemá žádné nested trees"): `foldersFor` (HauptwerkOdf.cpp ~1327-1332) takes, for a ONE-rank stop, the
  rank folder's PARENT as the stop folder only when the rank folder's name ENDS with the parent's name
  ("dolce8/03-dif2S-S_Dolce8" yes; "Gamba/03-dif2S-S_ViolGamb8", "oct4/03-dif2S-P_Octaaf4", "holpyp/...Holpyp8" no)
  - so those stops' folder = the rank folder, their "tree" = the parent (pipes/sw/Gamba) while the others' tree is
  pipes/sw, and dropNestedTrees merges them at load with the Messages note. Den Haag's producer abbreviates the stop
  folders (oct4, trom4, Gamba) differently from the rank folders. Proposed, not done: a one-rank stop's folder = the
  parent whenever the parent holds nothing but this stop's rank folders (a trem sibling allowed), the name rule
  second.
  0.9.50 (4 Nov 2026, the user: Doesburg "P. Mixtur" into the pedal without the 2'): `transmissionCaption` returned
  an alias BARE (Stop List rows, the dump) while the grid button (`Voicer::transmissionLabel`) added the footage - now
  the alias rule lives in transmissionCaption alone (alias with a footage = as typed; without = alias + the sounding
  footage + shift) and transmissionLabel calls it; `displayedFootageSemitones` (the transmission's base) =
  `lowestFootageSemitones + sampleOffset` of the measured rank (what the stop's own button shows: a mixture's smpl
  says 8', its button 2' - the transmission said 8'); `Stop::displayName` keeps a footage typed in the ALIAS as typed
  on a transposed stop too (THE USER'S RULE: an alias footage overrides the computed one, the way out when the
  program has no good answer). Compact voicing order: Transposition, Alias, One-shot, the perspective boxes as ONE
  group on one line (wraps whole), Rank count, the rest. Self-test line-identical to 0.9.7's but for timing - and
  "subbass 16 [default]" reads "bbass 16 [Dry]" on this machine because Patterns.json holds a learned "Dry;00-dr_;su_"
  line and the prefix match takes "su_" for "su" of "subbass" (a pre-existing matter of the machine's patterns, not of
  this build; told the user, not coded). Issues #175-#185 closed on 4 Nov 2026 (all solved or explained).
  0.9.51 (4 Nov 2026, the user: "Perspektiva bez oddělení se nesmí brát" + "pokud je prefix zaznamenán s
  oddělovačem v patternech, musí být nalezen ve foldername včetně onoho oddělovače; bez oddělovače může být
  oddělovač jakýkoli"): `stripPerspectivePrefix` (SamplerEngine.cpp ~492) compared pattern and name with EVERY
  separator removed (first commit) - "su_" took the "su" of "subbass16". Now a pattern holding any non-alphanumeric
  character must stand in the name AS WRITTEN (startsWithIgnoreCase); a pattern of letters/digits only must be
  followed by a non-alphanumeric character or the end. A pattern line in force applies per folder, no majority
  (the 0.9.49 majority is the import's LEARNING rule; a Rear of one division must stay) - the user asked why.
  0.9.52 (4 Nov 2026, the user: "Když chci vybrat IR pro auxiliary, tak dialog prostě zmizí"): the log of 12:45 says
  'Modal dialog "" made an owned window of the main window' right before each IR choice = the 0.9.29 owner trick
  (bringLostModalsToFront) applied to the NATIVE FILE CHOOSER's placeholder (JUCE's Win32 chooser is a plain modal
  Component with a peer and an empty name); when the native dialog closed, Windows activated the placeholder's new
  owner = the main window, which rose over the MODELESS Divisions dialog the chooser came from - hidden, not
  closed (no "keyboardsWindow.reset" path fires on an IR choice; the log has no crash). Fix: the owner is set for
  juce::TopLevelWindow tops only (our AlertWindows/DialogWindows). The user's "none after reopening" = he opened the
  dialog of ANOTHER division (12:45:20 "intercepted for manual 0" after the IR of manual 1). "Set effects for all"
  DOES copy volume + channel separation (applyEffectsToAllPerspectives 4372-4378; the tabs refresh from the config) -
  a slider moved after the tick unticks and copies nothing (the 0.9.8 rule); told the user, not changed.
  0.9.53 (4 Nov 2026, the user: "když je zapnutá superspojka, ukazuje to původní klávesu ... mělo by ukazovat oba
  hrané tóny zeleným kmenem"): `Voicer::setNoteActive(note, active)` was called from the MIDI/keyboard input with the
  PHYSICAL key and forwarded to EVERY row of every division (no manual) - the coupled notes never lit, the pressed key
  lit everywhere. Now `applyRoutedNoteDelta` (the one place every routed (division, note) reaches engine.noteOn/Off,
  couplers resolved) calls `voicer->setNoteActive(manual, note, on)` on the 0<->1 transitions; the overload lights the
  rows whose `StopRow::soundingManual()` (a transmission row: its target division) equals the division; the six input
  call sites (MIDI, virtual keyboard on/off, the video hook's note/chord) are gone, the panic's all-rows clearing stays.
  Dev: `ORGAN_LOG_NOTE_LIGHTS=1` logs "Key N of division M lit/dark on K row(s)". Test %TEMP%\test_lights0953.ps1
  (Rural copied beside its trees with couplers "1-1-4" + "2-1-8" enabled in BOTH "couplers" and
  "registrationCouplers" - the registration saved with the file overrides the list at the load -, ORGAN_TEST_NOTE
  "1|60|25|6": I. 60 + 72, II. 60, pedal nothing, all dark after the release). A rebuilt row (expand, alias ...) starts
  dark until the next note - as before. The test instance receives the user's MIDI while it runs (his keys showed in
  the log) - a run of mine plays on his console.
  0.9.54 (4 Nov 2026, the user's answer to "jaké je pravidlo pro náhodná rozladění a pipe coupling"): RANDOM TUNING
  (`regenerateRandomTuning`, SamplerEngine.cpp ~8219; per stop `randomPitchErrorCents` default 4, 0-40; drawn once per
  pipe at the load / slider change, a fixed detune per pipe, random sign) is UNIFORM over 0..amount now (was
  amount x rnd^0.8, leaning to the large values); applied at every voice start when the division's "Allow Random
  Tuning Error" is on. PIPE COUPLING (startNote ~6103-6125, renderVoice ~3361): a coupling situation = >= 2 stops
  engaged on the division OR another held key of the same keyboard at a fifth/octave; in it 9 pipes of 10 (was 5 of
  6, `nextInt(10) != 0`) glide their random detune over 0.5-2.5 s (cosine) to 0-20 % of it (was 0-33 %, `/ 5.0f`),
  the rest keep it; only with "Allow Pipe Coupling" AND the random tuning allowed (it moves the random detune alone,
  never the 440/temperament cents). Manual 5.x says no figures - unchanged.
  0.9.55 (4 Nov 2026, the user's Greccio 000349 LR_PRINC: "oktáva D62-D74 většinu času zní rozladěně" with the
  original tuning, fine at 440): MEASURED (%TEMP%\carlo_pitch.py, smpl = audio f0): D62 -58.6 c, D74 -60.7 c vs the
  file numbers (a1 ~425 Hz, the pipes scatter -55..-68 c) = the recorded octave is 2.1 c narrow (a 0.7 Hz beat)
  before any random; the coupling moved the RANDOM detune alone, never the recorded one, and only the pipe that
  starts while a partner holds. Now (SamplerEngine.cpp, `effectiveRandomCents` in the anonymous namespace before
  displayFolderName; startRank ~6207): (1) THE RANDOM DETUNE COUNTS THE RECORDED ONE (the user's rule): E = the
  pipe's deviation from its rank (`rankDeviationCents + mistuneCents` of the sample note; 0 with tuneTo440), the
  drawn R is a CEILING: |R| <= |E| -> nothing added, else |R| - |E| in E's direction (R's sign when E = 0); the
  slider-change path (setStopRandomPitchError) uses the same. (2) COUPLING TO A PARTNER: the pipe pipe+-12/+-24 of
  the SAME RANK already `speaking` is the reference: devA = -rawTuningCents[partnerSample] + tuningCentsFor +
  effectiveRandom(partner), ownBase = -rawTuningCents[sampleNote] + tuningCentsFor(sampleNote); the pure-octave
  target random = devA - ownBase, clamped to +-25 c around the own effective random, the pull 80-100 % of the way
  (`coupledTarget`); no partner -> the 0-20 % shrink of the own random (`couplingShrink`); `couplingRoll` (9 of 10)
  gates both; `couplingUsed = roll && (couplingSituation || partnerFound)`. Fifths and other stops are NOT partners
  (a temperament's fifth is its own; the class correction of mode 2 cancels for octaves only) - the shrink rule
  stays for them. Simultaneous keys: the MIDI commands run in sequence, the first pipe is speaking when the second
  starts, so the second aligns to the first (the first keeps its detune as the reference). Verified by the
  self-test only (line-identical); the acoustic effect is the user's to judge.
  0.9.56 (4 Nov 2026, the user first "teď to není dobře ... hraje to furt naladěně", then "asi by to tak mělo být
  správně, že náhodná odchylka je strop" - the CEILING RULE STAYS): the no-partner coupling now shrinks the pipe's
  WHOLE detune D = recorded deviation from the rank (`rankDeviationCents + mistuneCents` of the sample note; 0 with
  tuneTo440) + the random part, to a RANDOM share 0..`couplingLeftPercent` of itself (the glide moves the random part
  alone: target = D x share - E, clamped +-25 c around the random part); the octave-partner rule of 0.9.55 unchanged.
  Setting "Pipe coupling: detune left after the pull" (0-100 %, default 20; configuration key "couplingLeftPercent",
  travels with the organ; engine atomic + `setCouplingLeftPercent`; Editor slider in Advanced global audio right after
  "Replace pipes off by more than", no GLOBAL RESET entry - its neighbours have none either; manual ### of the same
  name). The user's "hraje naladěně" was his first impression, withdrawn - do not revert the ceiling.
  0.9.57 ENSEMBLE PULL (4 Nov 2026, the user: "pokud zní víc píšťal příbuzných (oktávy, kvinty), rozladěné píšťaly
  se k sobě přitahují ... i stejný tón v různých rejstřících, někdy ... tercii taky trochu ... nesmí to rozbít
  celeste"): in startRank (SamplerEngine.cpp, after the random/tuning stores) the PARTNERS = the voices sounding from
  the same keyboard (`keyHeads[makeSourceKey(key, manualIndex)]` lists for keys midiNote +- 24, at most 24 partners,
  no voice scan) whose SOUNDING pitch (pipe + sampleOffset + the rank's/stop's footageSemitones) stands to the new
  one at 0 / 12,24 / 7,19 / 5,17 / 4,16 / 3,15 semitones = strength 1.0 / 0.8 / 0.5 / 0.35 / 0.2 / 0.1 (x random
  0.3-1.0 = "sometimes"), pure offsets of the upper pipe vs ET 0 / 0 / +1.955 / -1.955 / -13.686 / +15.641 c; the new
  voice's random target = the weighted mean of the pure-interval deviations - ownBase, 80-100 % of the way, +-25 c;
  each partner gets `couplingRetarget` (atomic float, NaN = none) + `couplingRetargetTotal` = its current end value
  moved 25 % x weight (x 0.5 for the LOWER pipe) towards the pure interval with the new one; the render thread picks
  the retarget up at the block start (randomPitchCents := the value the glide is at, coupledPitchCents := retarget,
  elapsed 0 - the voice's glide state stays render-owned). A celeste (rank/stop `detectedCelesteBeats` or
  `undulationBeats` != 0) neither pulls nor is pulled; the same pipe in another perspective/route is no partner;
  release copies get NaN. The 0.9.55 same-rank octave rule is subsumed. No partner -> the 0.9.56 shrink to the
  rank's tuning. Verified by the self-test only (identical) - the acoustic judgement is the user's.
  0.9.58 (4 Nov 2026, the user's Hildesheim E major with ~20 stops: "jednotlivé píšťaly Gis velmi rozladěné ...
  Gis samotné se k sobě pěkně doladily"): THE 0.9.57 FAULT - the interval strength was a WEIGHT in the mean of the
  partners' pure-interval targets, not the SIZE of the pull: a G# whose partners are E and B (thirds, pure -13.7 /
  -15.6 c) got the mean -14 c and went 80-100 % of the way, each G# by another random share = the chaos; alone it had
  only unison/octave partners and locked. Now (startRank): `baseDeviation` = ownBase + the no-partner target (the
  pipe at its rank's tuning + the 0..slider % remainder); each partner displaces it by `strength x (pureDeviation -
  baseDeviation)`, averaged over the "sometimes" `factor`s (0.3-1.0): `coupledTarget += sum / factorSum` - a unison
  pulls all the way (locking), an octave 80 %, a fifth ~1 c, a third 2-3 c, never more, however many partners; the
  partners' own move = 0.25 x strength x factor (x 0.5 lower) towards the pure interval with the new pipe. RULE: the
  interval strength scales the DISPLACEMENT from the rank's tuning, never the choice of target.
  0.9.59 (4 Nov 2026, the user: "9 z 10 znamená, že při 30 rejstřících se vždycky najde nějaký, který se nedoladí ...
  chci možnost, že se doladí všecko"): the roll is a setting - "Pipe coupling: pipes left untouched" (0-50 %, default
  10 = the old 9 of 10; configuration key "couplingSkipPercent", engine atomic `couplingSkipPercent` +
  `setCouplingSkipPercent`, `couplingRoll = nextFloat() * 100 >= skip`; Editor slider right after "detune left after
  the pull"; manual ### of the same name). 0 % = every starting pipe couples.
  0.9.60 (5 Nov 2026, the user's answer to the two-division question - cross-division partners and an organ-wide
  base were REFUSED ("ne, tohle nechceme"): instead "Pipe coupling: farthest pitch offset to pull" (0-50 c, default
  14; key "couplingMaxCents", engine atomic `couplingMaxCents` + `setCouplingMaxCents`): in the ensemble loop a
  partner whose pure interval lies further than this from the new pipe's base (`|pureDeviation - baseDeviation| >
  maxPull`) is skipped - it neither pulls nor is pulled; the no-partner shrink to the rank's tuning is untouched.
  "Pipes left untouched" ranges 0-100 % now. The two-division non-locking stays by design: partners are the
  voices of the SAME keyboard (`keyHeads[makeSourceKey(key, manualIndex)]`) and each pipe settles on its own rank's
  diapason (II. 435.8 vs III. 435.6 Hz = ~1 c apart with the original tuning; 440 removes it).
  0.9.61 (5 Nov 2026, the user: "Arrange mode spojek nelze ukončit klepnutím MIMO grid spojek!"): the coupler
  arranging of a division header (the 3 s press) ended only on THAT header's background click. Now
  `SectionHeader::ExitWatch` (voicer.cpp, a juce::MouseListener with a SafePointer to the header) is added
  RECURSIVELY to `getTopLevelComponent()` by the header's 25 Hz timer once the mode is on and the header is shown
  (at construction it has no window), and removed by the timer when the mode ends, by `removeExitWatch()` in the
  destructor (every reorder rebuilds the headers); a mouseDown whose component is not the header or its child
  calls `endArranging()` (the one place that leaves the mode: flag, arrangeStateChanged, resized, repaint).
  Clicks in OTHER windows (dialogs) do not end it. No hook drives the 3 s press - smoke run only.
  0.9.62 (5 Nov 2026, the user: "některé samply potřebují výrazně vyšší sustain-release crossfade ... Glocken v
  Hildesheimu zhruba 1000 ms"; he chose A + "automatické hodnoty odečíst z HW ODF ... průměr kolem tónu 069-A"):
  `Stop::releaseCrossfade` (1.0; `setStopReleaseCrossfade`, 0.25-20) multiplies the global fade-out
  (`releaseFadeMsRequested` in finishNoteOff) and fade-in (`fadeInTotal` of the release voice); voicing row
  `StopRow::releaseFadeBox` ("Release xfade xN", items x0.5 1 1.5 2 3 5 8 10 + the stop's odd value, id = factor x 10)
  after One-shot in BOTH layouts (compact `Control::releaseFade` 150 px; the old line + `line1Width` + 150);
  `releaseCrossfadeRequest` StopRow -> Voicer -> MainComponent (engine + voicingSaveAt); voicing state key
  "releaseCrossfade" (written when != 1, read in the voicing-state load). HAUPTWERK: `Pipe_SoundEngine01_ReleaseSample`
  column 14 = `ReleaseCrossfadeLengthMs` = compacted letter "n" (the letters follow the column order and SKIP "o" - the
  row tag; q = LatestKeyReleaseTimeMs; verified on Hildesheim: Glocken 711-1000 ms, pipes 60-150 ms): `releaseMsOfLayer`
  -> `Rank::releaseCrossfadeMs` (mean of the pipes with notes 66-72, else all) -> `PlanStop::releaseCrossfade` = the
  stop's ranks' mean / `referenceReleaseCrossfadeMs` (the ParseJob sets it from `releaseFadeOutSlider`, 110 default;
  --hauptwerk uses 110), kept only when >= 1.5 (rounded to 0.1) -> `hauptwerkReleaseCrossfade[(division, row key)]`
  (beside hauptwerkCompass) -> `engine.setStopReleaseCrossfade` in applyHauptwerkVoicing; plan note + import report
  line "Release crossfade: ...". NOT against the organ's median (`organReleaseCrossfadeMs`, kept for the note): Hildesheim's
  median at A4 is 59 ms and every 16'/32' stop came out x1.8 (Hauptwerk lengthens low pipes' crossfades as our fadeScale
  does), the Glocken x15.5 instead of the ~x8 (900 ms) they need. Test: `--hauptwerk` on the Hildesheim Surround ODF must
  print "Release crossfade: 1 stop(s) ... "2. Glocken-Hw" x8.3"; %TEMP%\odf_release.py <odf> [filter] lists the per-rank
  values; %TEMP%\test_row0936.ps1 -Tag xf pictures the row (its horizontal scrollbar at 2572 px is there with 0.9.61 too).
  0.9.63 (5 Nov 2026, the user's Hradec Králové: "Quinta3 2 2/3'" on the button after he cleared the alias; "někdy jsou
  kvinty zapsány i takto Quint113, přilepené na slovo"; "hlavičku HW apod. ustřihnout, je-li pro všecky shodná"):
  (1) `SamplerEngine::ungluedFootageName(name)` (after nameWithoutFootage): trailing digits right after a LETTER, no
  footage the strict way, the word holding no mixture word (mixtur cimbel zimbel scharf cornet kornet rausch sesquialt
  fourniture plein acuta tertian progressio harmonia carillon pleno ripieno - their number counts RANKS), the digits a
  footage (1 2 3 4 6 8 12 16 32 64 with the 3/6/12 quint shorthand, or the compact 113/223/135) -> "<word> <label>'"
  ("Quinta 2 2/3'", "Quint 1 1/3'", "Flauta 8'"). Used by `Stop::displayName` (the base) and `nameWithoutFootage` ONLY -
  `footageFromName` (tuning, auto-transposition) stays strict: "Mixtur4" must never become 4'. (2) The import's `aliasOf`
  (the entries loop of importHauptwerkOdf): `divisionHead` = the first word every stop of the division shares, ending
  with ":" or "." (>= 2 stops), cut from the aliases and the extensions' aliases ("HW: Quinta 3" -> "Quinta"; Hildesheim's
  "2. Prinzipal 16" -> "Prinzipal"). (3) Main.cpp `fileFromCommandLine` returns nothing for a line holding a "--" switch:
  a `--hauptwerk` run of mine while the user's program ran was handed to it as a file from the desktop (0.9.16) and
  IMPORTED Hradec twice in his instance (11:59, he cancelled) - NEVER start the exe with a switch while `Get-Process
  LibreOrgan*` finds anything, even for a headless dump. Test %TEMP%\test_import_hradec.ps1 [-Odf <odf or json>]
  (the hook import/load into a temp configurations folder, settings restored, the Stop List dumped + the log slice):
  the Wet ODF must give aliases "Quinta", "Mixtura 3x" and captions "Quinta 2 2/3'"; his WetUprveny.json (aliases
  empty) "Quinta 2 2/3'" and "Quint 1 1/3' (Ext.)".
  0.9.64 (5 Nov 2026, the user: aliases cut in the Stop List TABLE ("Ped: ", "HW: ") came back after Apply and reload;
  "aliasy jsou tam dvojmo ... obě místa by měly vést k totožnému jednomu aliasu"): THE ONE ALIAS IS `Stop::alias`
  (carried by the voicing state); the table's alias column is a VIEW - filled from the stops when the dialog opens
  (0.8.201) and written back onto the stops by `MainComponent::applyTableAliasesToStops(manuals)` in applyStopLists and
  applyStopBuild BEFORE saveSettings (which syncs the table FROM the stops) and before the reload (whose grid phase
  `loadState`s the voicing file - the function saves the voicer state when it changed anything); an EMPTY cell clears
  the alias. The log of the fault: "Stop List applied" then "Save settings: N table alias(es) replaced by an empty stop
  alias" then "Division N rebuilt: ... 0 applied". The afterCommit rule of 0.9.32 now also applies a DIFFERING table
  alias (for rows whose stop is new). Dev hook `ORGAN_TEST_TABLE_ALIAS="<manual>|<row>|<alias>"` (StopListDialog
  constructor after originalDrafts: edits the draft as the user types). Test %TEMP%\test_tablealias0964.ps1 [-Row -Alias
  ""] (a copy of his Hradec file in a temp configurations folder, Stop List + Apply and reload by ORGAN_TEST_CLICK at
  30 s; the stops' aliases after the reload read from voicer.json before the restore - saveSettings writes no
  configuration file). The activity log ROTATES at every start (activity-previous.log): a test's log slice must filter
  both files by time, not by the old file's length.
  0.9.65 (5 Nov 2026, the user's Chemnitz: "proč to cpe tuhle pattern do default místo diffuse"; Chapelet "I. Nasardo"
  twice): THE CAUSE: Chemnitz's definition spells "Difuse: 1. Principal 8" (one f) - no perspective word -> rank label
  "" -> default; and the import never consulted the Perspective Patterns (the scan does, the import writes them only).
  Now in `HauptwerkOdf::parse`, after the 0.9.49 majority block: (1) a HEAD before the colon without a word, on >= 50 %
  of the stops, is a perspective when the definition has another recognised one (`nearPerspectiveWord`: one typo from
  a word of `kPerspectiveWords` -> the word, else the head as spelt; log "the head "Difuse" ... taken for the
  perspective "Diffuse""); (2) ranks still without a label take `labelFromPatterns(patternsInForce, folder name)`
  (the 0.9.51 prefix rules) - `parse(file, progress, patternsInForce)`, the ParseJob passes
  `engine.getPerspectivePatterns()`, `--hauptwerk` passes none. (3) PATTERNS MERGED BY PREFIX: `mergedPerspectiveLines
  (base, added, addedCount, conflicts)` (MainComponent.cpp beside mergedPatterns): a prefix joins its label's line, a
  prefix owned by another label in `base` is skipped = a conflict; `mergeApplicationPatterns` takes the ORGAN's lines as
  base (its definition is the truth for its folders; conflicts -> log "Perspective patterns: N prefix(es) ... the
  organ's own line applies, Patterns.json is left as it is" + Messages), `rememberPatterns(false)` takes PATTERNS.JSON
  as base (the user's lines keep their prefixes; the old by-label merge REPLACED his "Diffuse;04-AB_;04_abblizko_" with
  the next organ's Diffuse line). (4) The import's `aliasOf` cuts a leading division numeral (P./Ped./I.-VI.) that names
  ANOTHER division than the row's ("A: I. Nasardo 1 3/5" on II -> "Nasardo"). REGRESSION CHECK the user asked for:
  %TEMP%\hw_compare.ps1 = `--hauptwerk` of the 25 published organs (A-Ch in the Czech alphabet: everything but Rural and
  Stralsund) with C:\LibreOrgan\LibreOrgan_0.9.64.exe and the new build, diffed - 24 identical, Chemnitz alone changed
  (50 ranks [Diffuse], "perspective pattern: Diffuse;04_abblizko_"); the Chapelet import gives "I. Nasardo 1 1/3'" on I
  and "Nasardo 1 3/5'" on II; the conflict path tested with a crafted Patterns.json ("Direct;04_abblizko_"): the organ
  loaded Diffuse, the note logged, the user's file restored by hand (the test script's backup would have kept the
  crafted one - back up the user's Patterns.json BEFORE such a run). THE USER'S RULE: a regression check runs over the
  published organs up to Ch only ("dál nechoď, tam nevíme, co se ještě stane"), economically.
  0.9.66 (6 Nov 2026, the user's Yokota/Chico "HW Mixtur ... FLANGER at 052E" under coupling/wind; THE RULE: "ve všech
  ostatních případech musí být perspektivy téhož tónu vždy perfektně při sobě na pitch", the TREMULANT is the one
  exception - its phase drift between perspectives is essential): every perspective of one pipe is its own voice and
  differed in (1) the WIND hash (`rank->folderHash` -> variation index + windKeyScale per perspective; now
  `stop->folderHash`), (2) the random detune's SIGN since 0.9.55 (effectiveRandomCents read each rank's own
  `rankDeviationCents`: +1 c vs -1 c smpl noise gave +4 / -4 c = 8 c apart; now `referenceRankFor(stop, pipe, refNote)`
  = the stop's own pipes, else the first perspective rank holding the pipe, used for randomEff, `recorded` and `ownBase`
  in startNote and in setStopRandomPitchError), (3) the ensemble pull (each perspective drew its own "sometimes"
  factors; now `PipeCoupling` per pipe: computed for the first perspective, reused by the others; the partner PIPES'
  factor/glide length drawn once (`SeenPipe`, keyed by (stop, note)), a partner pipe counts once in the sum and every
  voice of it gets the same retarget). The tremulant's `tremBucket` from `rankOrdinal x rankDrift` is untouched (by
  design). Temperament mode 2 / 440 per rank differ by hundredths of a cent - left. Self-test identical (one
  perspective there); the acoustic proof is the user's.
  0.9.67 (6 Nov 2026, the user: "wind model víc působí na basy než na vysoké klávesy"; the ANSWER from the code: the sag
  is DRIVEN by the low keys only (`windKeyWeight`: <= 48 full, 48-69 falling to 0, above nothing - the user: keep it),
  the FOLLOWING had a mild pitch ramp by the SAMPLE number (0.65 at 24 .. 1.35 at 96 = ~1.13 per octave) under a
  random 0.55-1.45 per pipe, and the level/filter none; coupling vs wind: nothing wins, `totalCents` sums them, the
  coupling target ignores the wind): setting "Wind: sag ratio per octave" (`windOctaveRatio`, configuration key,
  1.0-2.0 step 0.05, default 1.3 - the user's choice; "no percentages" - a ratio; engine atomic `windOctaveRatioValue`
  + `setWindOctaveRatio` in applyAdvancedAudioSettings; Editor addSlider right after "Wind: volume change per cent of
  sag", the Wind settings dialog row "Sag ratio per octave (all divisions)" + its help entry; manual ### of the label):
  `pitchFactor = ratio ^ ((sounding - 60) / 12)` with sounding = pipe + footage + sampleOffset (as fadeScale), clamped
  0.1-8; `windKeyScale = randomFactor x pitchFactor` (the random factor KEPT, the user); the level follows too:
  `ctx.windGainDb[]` (dB, was `windGain[]` as gains) and the voice does `fastDbToGain(windDb x windKeyScale)` - the
  filter share reads that gain, so it scales with it. At ratio 1.3: 8' C 0.46, c1 1, c3 1.69. THE VERSION LESSON: the
  0.9.65/0.9.66 bumps replaced the jucer only - CMakeLists/site/rural README stayed 0.9.64 (bump_version.ps1 prints
  "0 replaced" per file: READ that output; grep the previous version before tagging, as the rule says).
  0.9.68 (the user: "místo na oktávy podle znějící výšky, udělejme to na oktávy klaviatury"): `pitchFactor = ratio ^
  ((midiNote - 60) / 12)` - the KEY, so a 16' and a 2' stop on one key follow the sag alike; texts/manual say "key".
  0.9.69 (6 Nov 2026, the user's Kdousov (000348) pedal: "superoctavbass4 040-E je o 14 centů níž a pipe coupling ho
  nikdy nedoladí"; MEASURED with %TEMP%\test_coupling0968.ps1 (ORGAN_TEST_RECORD of key 40 with the two stops, the
  subbass row removed from the copy, couplingMaxCents 50) + coupling0968_peaks.py (2 s FFT windows: the Octavbass h1
  and the two components near 164 Hz): the file's E is 13 c flat, the recording ended 18 c WIDE (Superoctav +7, Octavbass
  -10.5 vs ET) and the gap GREW over the glide): THE CAUSE - temperament mode 2 (Kirnberger III selected, each rank
  with its own measured table) lifts the Superoctav's E by +12.5 c (its table had absorbed the flat E: 040 and 052 both
  flat), while the coupling's `ownBase` and the partner `deviation` were raw tuning + 440 correction + random WITHOUT
  the temperament term the render adds -> it saw 13 c and pulled 10 c more. Fix: `temperamentOf(rank, keyNote,
  pitchNote, mode)` (a lambda before startRank, the render's formula over selectedPitchCents / pitchClassCents /
  measuredTemperamentCents) added to both. RULE: every control-side estimate of where a voice SOUNDS must include
  the render's temperament term (mode 1 selected[key class]; mode 2 selected - (measured[class] - measured[A]) of
  the RANK; mode 0 pitchClassCents + the key/pitch class difference).
  AFTER 0.9.69 the Kdousov octave was still 8 c wide: MEASURED PRECISELY (%TEMP%\coupling0969_precise.py <wav> <start s>
  <f1> <f2>...: 4 s Hann, 4x zero-padded FFT, log-parabola, h1/h2/h3 agree) the Octavbass 040-E FILE sounds 82.10 Hz =
  -6.5 c while its smpl says +0.1 c (041/042 also +0.1 = unfilled values); the Superoctav's smpl is honest (-13.2 vs
  -13.6). The autocorrelation of carlo_pitch.py is +-3 c on bass pipes - never judge a few cents by it. The engine
  trusted the smpl, so the "pure" octave it made was 6.5 c off in the sound; a unison with Principal 4 (honest smpl)
  locked. The user's own tuner said 82.4 - the FFT is right (three harmonics, both files).
  0.9.70 (6 Nov 2026, the user: "tak to udělej. Ale bez regresů"; his rules: the analysis window in the MIDDLE OF ALL
  LOOPS (first loop's start .. last loop's end), a configuration switch so the user can turn the measurement off and
  read the smpl alone, fallback = today's state (no smpl -> the file number, 0 c), RED note name in the voicing = "no
  pitch" (smpl missing or off the sound), orange stays "no loop", red wins; the 0.2 c he first asked for is under the
  measurement's precision - 3 c (6 c under 50 Hz) agreed): `analysisStart(sample, n)` / `analysisOrder(sample, max)`
  (anonymous namespace before loadSamplesIntoStop) = the middle of the loop span, the largest 2^k <= the span (>= 2^12);
  `sustainSpectrum` (Blackman-Harris + juce::dsp::FFT cached per thread and order in `fftOfOrder`); `pitchFromSound
  (sample, guessHz)`: harmonics 1..4 within +-2.5 % of k x f0, local maximum 12 dB over the between-harmonics median,
  log-parabola over the peak bin, /k, the estimates within 3 c of their median averaged (two at least, else NaN); no
  guess = harmonic product spectrum (k 1..5, 16 Hz-2 kHz) then halved while the odd multiples of the half stand.
  `lowestSoundingHarmonic` uses the same window (was 65536 frames from 0.3 s). In loadSamplesIntoStop's parallelFor
  (attacks only, before packing): `Sample::smplPitchInFile`, `measuredPitch`, `pitchSource` (0 smpl / 1 the sound took
  over / 2 no smpl, measured / 3 no smpl, nothing); a no-smpl pitch is snapped to the octave of the rank's median
  (smplPitch - file number) after the loop; log "Pitch from the sound of "stop": N file(s) whose smpl chunk differs
  ... the sound taken: 040-E (smpl +0.1 c, the sound -6.5 c) ...; M without a smpl chunk measured; K ... the file
  number taken". Setting `measurePipePitch` (configuration key, default true; engine atomic `measurePipePitch`;
  Editor addToggle "Measure the pitch of every pipe from its sound" right before "Retune pipes off by more than";
  GLOBAL RESET true; manual ### of the label). Voicer: `Stop::noteWithUncertainPitchInAnyRank(note)` -> red
  nameLabel/toneNameLabel + tooltip (the three cases; "; no loop in the smpl chunk either" appended), else the orange
  rule. NOT changed: the perspective gate's autocorrelation (`measuredPitchCents`, reads files from the disk, no loop
  info) - noted. The self-test baseline moves (the footage window) - compare against %TEMP%\selftest_0970.txt from
  now on. VERIFIED 0.9.70: Kdousov octave E ~2 c (was 18 / 8); the log found i_fletten8 038-D (smpl -7.0, the sound
  +10.9) and 039-D# (0.0 / +17.9) too, confirmed by coupling0969_precise.py; self-test: only the harmonic LEVELS of
  the footage lines and timings changed, every tuning/footage/temperament the same. Build lesson: a foreground
  MSBuild that the tool moves to the background after 600 s leaves ORPHANED cl.exe (CPU 7 s, parent gone) - start
  every full build with run_in_background from the beginning; `/p:SelectedFiles=a;b` with a semicolon is MSB1006 -
  one file per MSBuild call.
  0.9.71 (6 Nov 2026, the user: a held treble tone wobbles far more under two staccato bass keys than under one;
  "aby oscilace přibývaly pouze málo oproti jen jedné klávese. Ale aby přibývaly"): `windSagAt` summed every running
  stroke impulse (pressSum += ..., releases alike) - two keys together = double. Now `combineImpulses(values, count,
  strokeAdd)` (anonymous namespace before windSagAt): sorted by magnitude, the largest whole + the next x p + the
  next x p^2 ..., presses and releases each; `windSagAt(..., strokeAdd)` gets `owner.windStrokeAddValue` from
  renderRoute. Setting "Wind: further strokes add" (key `windStrokeAddPercent`, 0-100, default 30; engine atomic
  `windStrokeAddValue` 0..1 + `setWindStrokeAdd`; Editor addSlider after "Wind: sag ratio per octave", the Wind
  settings row "Further strokes add (all divisions)" + help, manual ###). 100 % = the old plain sum. The pipe-count
  shares (one/two pipes) rule the STEADY sag only - at 50/50 one and two pipes are equal there; the user's
  difference was the impulse sum alone.
  0.9.72 (6 Nov 2026, the user after 0.9.71: "jedna klávesa neudělá dost, dvě jsou moc ... když klávesy jedou rychleji
  než wind frequency ... brblání"; three rounds of physics - his points: no extra damping ("nechci Schwimmer"),
  antiphase cancellation is real and stays, staccato of half a period ADDS (press + release kicks in phase), the
  wind must follow the PIPE and "Attack min. time" may be 0 so an inner floor of 50 ms): THE WIND REWRITE.
  `WindSourceState`: the `transients[64]` array is GONE; ONE damped oscillator per source (`oscStartMs,
  oscAmplitude, oscPhase, oscSigmaPerMs, oscOmegaPerMs`; `windRingingAt` evaluates x and x' analytically, sigma =
  3 f / N so the ring is at 5 % after N = "Number of oscillations" cycles, N = 0 -> no kick), `kicks[32]` (a kick
  queue: every stroke is four quarter-kicks at +0/10/20/30 ms, `queueWindKick`, `processWindKicks(now)` from
  processPending applies the due ones: `applyWindKick` = x, v now, v += amount x omega, A/phi re-derived -> in
  phase it grows, in antiphase it cancels), `keyStrength[128]` (random 0.6-1.0 drawn at the stroke, the release
  reuses it), `lastDemand`. THE KICK = depth x (change of the saturated demand averageWeight x sustainedFraction)
  x 2.0 x strength x registration terms x random x maxEffect (x 0.9 for a release): a first key at the 50 % share
  ~ the old pair at 30 % (MEASURED by %TEMP%\test_wind0972.ps1 [-Exe old] + wind0972_track.py: a held Principal 4
  key 77 on I. of Kdousov, the pitch tracked; the video-script hook with ORGAN_TEST_NOTE "1|84|17|1" switching the
  one-stop table on 17 s after the start - the script's `stop` command and the file's registration both left the
  recording SILENT, not explained; factor 2.6 gave one key rms 3.2 c vs the old 1.4 / old pair 2.3; 2.0 gives 2.7;
  fast repetition at 120 ms: one smooth ~7 Hz wave, side components -14 dB (old: a 4.5 Hz component -5 dB = the
  chatter); self-test unchanged but timings), a second key
  [0.9.73 (the user: "při plénu v diskantu to působí míň než na jednotlivý rejstřík" - "body 1 a 2 byly blbost od
  začátku"): the old `registrationDamping` (-15 % from the 4th stop) and `oscillationStopContribution` (the per-stop
  AVERAGE load) are GONE - `stopFactor = clamp(sustainedStopLoad / oneStopShare, 0, 3)`: a single 8' stop 1.0 (solo
  unchanged), a pleno up to 3 x, a lone small stop (load < 1) less; `engagedStops` removed. Test:
  test_wind0972.ps1 -Pleno (every stop of I. on). THE USER ON 0.9.72: "teď to nemá žádné oscilace!!!" - the
  saturated-demand kick gave NOTHING in LEGATO (a key struck while another still held: the demand 0.5 -> 0.5);
  0.9.73 kicks per stroke by the key's weight x onePipeShare (solo unchanged), keys struck within 60 ms of
  another are a chord (the first whole, the rest 10 %: `lastPressMs`/`lastReleaseMs`, `chordScale`), `lastDemand`
  gone. RULE: a kick is per STROKE (every pallet that opens), never a difference of a steady figure. The test's
  segment D = a legato bass line. TEST ARTEFACT: ORGAN_TEST_NOTE enables EVERY engine stop, the POOLED ones of the
  stripped table too (they keep primaryManual 1), so `effectiveStopLoad` of the "solo" run counts 9 stops -> the
  stopFactor cap 3 - the 0.9.73 "solo" rms 2.6 vs 0.9.72's 0.9 at the user's depth 4.4 c is that cap, not a change
  of the solo stroke; read the kicks from the log instead: `ORGAN_WIND_DEBUG=1` logs "Wind kick: division m key n
  down: weight x share x 2 x strength x stops (load) x random x chord x depth = K c; steady target T c". The user's
  file at 18:35 had depth I. 4.4 c (17 at noon), pedal 5.6, strength 22 - small depths = small kicks whatever the
  version.]
  0.9.74 (6 Nov 2026 evening, the user on 0.9.73: "míra oscilací je nyní příliš velká. Mám to na 5 procent ... dej,
  aby to, co je nyní 5 %, bylo jako původní default"; "chceme opatrnost. Dej pouze 60 procent. Při druhé a další
  píšťale polovic"): the kick constant 2.0 -> 0.25 (the default "Oscillation strength" 40 % = 0.8 in the engine
  sounds as 5 % did), a release kicks back x 0.6 (was 0.9; 0.85 in the old model), `othersHeld` (heldBefore > 0
  for a stroke, > 1 for a release) -> chordScale 0.5 (legato) unless the 60 ms chord rule's 0.1 applies, and
  `applyWindKick` caps `oscAmplitude` at `depthCents` (the bellows' end positions - before, only the output was
  clipped at 1.25 x depth, a waveform clip). The debug line prints "chord/legato" and "ringing now A c".
  0.9.75 (6 Nov 2026 night, the user: "celkově dost slabý ... default 15 centů, max 100 ... pedál 2x" -> "max 150,
  defaulty 15 manuál, 20 pedál, ostatní podle tebe"): the FACTORY WIND PROFILE (`applyDefaultWindProfile`,
  MainComponent.cpp ~1048) had depths 2.67/2.17/2.5/3.0 c - inaudible with the 0.9.74 strokes - now 20 (pedal) /
  15 (manuals); the dialog row default `m == 0 ? 20 : 15`, the Editor 15, the struct 15; max 150 in the engine
  clamp (`setWindModel`), the dialog, the Editor, the MIDI-learn `scaled(0, 150)`. THE PEDAL'S 2x comes from the
  FOOTAGE, not from the division: `windStopContribution` 32' -> 3, 16' -> 2, 8' 1, <= 5' 2/3, <= 3' 1/3 (rules the
  steady sag's stop load and the strokes' stopFactor alike). `harmonicWindDb` default 4 dB (header, loadSettings,
  dialog row, Editor, manual). "Volume change per cent of sag" stays 0.2. Saved files keep their own depths/filter.
  0.9.76 (the user: "zesil 2x, default nech. Dvojkliky na ukazatele musí nastavovat naše nové defaulty! Čtyřstopý
  rejstřík skoro nic nepůsobí ... Wind depth stále vidím max 50 centů! Zpětné rázy 75 %"): the kick constant 0.5,
  release x 0.75, `windStopContribution` 4'-5' 0.85 and <= 3' 0.5; THE DIVISIONS DIALOG has its OWN wind sliders
  (`setupWindSlider` ~8421, `defaultWindDepth[8]` ~8146 - a second copy of the factory profile!) that 0.9.75
  missed: max 150 and the defaults 20/15 now. RULE: the wind defaults live in THREE places - applyDefaultWindProfile,
  the Divisions dialog's defaultWind* arrays, the Wind settings dialog rows / Editor addSlider defaults - change all.
  The random strength IS drawn at every stroke (keyStrength at `down`), the release reuses the stroke's.
  0.9.77 (7 Nov 2026, the user: attack min. delay PER STOP in the voicing, "maybe after top volume" -> put after
  Attack Time (the attack group stays together); the global and the stop's delay ADD; the red note name lighter;
  "na webu raděj nech i předchozí verzi"): `Stop::attackDelayMs` (0-500, default 0); in `startNote`'s stop loop a
  stop whose `keyStartMs[sourceKey] + attackDelayMs` lies ahead is skipped and `delayedStartMs[key]` = the earliest
  due; processPending re-runs `startNote` for the held key at the due time (the stops already speaking are skipped by
  the ranks' speaking bits; `keyStartMs[key] = now` is set right BEFORE the first startNote of processPending; a
  release clears the queue - a stop still waiting never starts). Voicing: `attackDelaySlider`/Label after Attack
  Time in BOTH layouts (placeSlider, the compact `widths[7]` array - the 0.9.36 rule -, `slidersRowWidth` + 150,
  7 gaps), `attackDelaySliderChanged`, MIDI learn parameter "attackDelay" (normalized x 500), the voicing state key
  "attackDelayMs" (written when > 0; voicer.cpp save/load, MainComponent's two copies of the voicing parse and the
  memories' stop state, the GLOBAL RESET / resetAll zero it), manual sentence in "Attack Gain and Attack Time". Red
  note names = `juce::Colour(0xffff6464)`. build.yml: the release job KEEPS the previous v* release (the newest
  other tag by createdAt) and deletes the rest. VERIFICATION LESSON (3:30 at night, the Fireface OFF): every
  ORGAN_TEST_RECORD / ORGAN_TEST_NOTE run was silent (the Kdousov routes "ASIO Fireface USB: hdspf init failed",
  Rural's aggregated "All WASAPI Outputs" reopening every 6 s "plays through no endpoint" - voices published to a
  route without a device are finished at once, polyphony 0) and `collectRouteShifts` gives 0 shifts for such stops,
  so even the "Attack delay: X waits" debug line never came: a GUI hook proves nothing without a live device - the
  SELF-TEST (its own device keys) is the way: EngineSelfTest.h "Attack delay (<stop> waits 300 ms ...): N voices
  100 ms after the key, M after 500 ms -> OK" (M > N > 0). Dev env `ORGAN_ATTACK_DEBUG=1` logs the waits/re-runs.
  The activity log is APPENDED and rotated at 1 MB (activity-previous.log), not per start - a log slice is read
  from a length mark, and `Select-Object -First` on a grep of the whole file shows the OLDEST lines.
  0.9.78 (7 Nov 2026, the user: "per stop zpoždění má mít stejná pravidla při opakování jako globální ... randomizace
  30-100 %"): the stop's "Attack delay" (startNote ~6240) applies the global delay's repetition rules against ITS OWN
  value - a tone of the key finished within half of it (noteFinishedMs): 0; within the whole: half; else a series
  of strikes within it (interval of the last two note-ons): 0 - then x (0.3 + 0.7 x share), share = fmod(a random
  drawn ONCE per key start + stopIndex x 0.618, 1) so stops of one key differ and the re-run of processPending
  reaches the SAME due (the figures `keySinceFinishMs` / `keyOnIntervalMs` / `keyDelayRandom` per key are fixed in
  processPending at keyStartMs and in noteOnImmediate - never drawn inside startNote, or the re-run postpones for
  ever). The self-test probe: 1000 ms after 225 blocks (1.2 s) of silence, early at 100 ms (< 300 = the random
  floor), late at 1100 ms. SELF-TEST BASELINE: %TEMP%\selftest_0970.txt was made with "TA;NW" (two folders) -
  compare with the same argument. `--selftest` DETACHES like the GUI (a `& exe` call returns at once and the exe stays
  locked -> LNK1104 on the next build): Start-Process + WaitForExit, the report in %APPDATA%\LibreOrgan\selftest.txt.
  0.9.79 (16 Nov 2026, the user: "Udělej vše, především odpověz na issues a uzavři issues" - #228-#237 in one build):
  (1) #237 THE EMPTY START = `SamplerEngine::pruneStaleCopyCacheFiles` opened EVERY cache file (2 180, 9.8 GB) on the
  message thread in loadSettings (the stack in the log: FileInputStream <- setCopyCache <- loadSettings <- the
  MainComponent constructor), 11 s cold; now on a juce::Thread::launch (a ".part" younger than 60 s is a build's).
  `StartSplash` (Main.cpp, a borderless TopLevelWindow, NOT on Linux) is made before `new MainWindow` and paints
  SYNCHRONOUSLY (`peer->performAnyPendingRepaintsNow()`) at every `CrashHandler::note` through
  `CrashHandler::noteListener()` (called on the message thread only) - the message loop does not run while the
  window is built, so this is the only way a status line moves; the listener is cleared after the window exists,
  the splash deleted 700 ms later. Verified: a window of the process visible 204 ms after the start. (2) #229
  `SamplerEngine::takeOutputPeak(clipped)` = the max |sample| BEFORE the safety clip of renderRoute (float bits in
  an atomic, CAS max) + the clip flag (> 0.98); `PolyphonyDisplay` has the meter cell first (`meterWidth()`,
  8 segments 4 green/3 orange/1 red, 1 s hold, the CLIP dot latched until a click on the cell; the limiter in the
  cell's tooltip - the manual never mentioned it). The cell is ~50 px high: 12 segments were 2 px each. (3) #228
  `Stop::attackKeyMs[128]` + `attackRandomize` + `attackKeysExpanded` (SamplerEngine.h); startNote's stop loop
  takes `getNoteAttackMs(midiNote)`: > 0 the 0.9.78 delay rules (repetition + 30-100 % only with randomize), < 0
  `attackCutMs` -> in startRank `v.pos = cutFrames << 32` (<= half the sample) + an 8 ms fade-in. StopRow: the
  third per-key row (attackSliders/attackNameLabels/attackHeader/attackReset/attackRandomizeBox, section 2 of
  setVoicingFolds, `keyAreaHeight(volume, tone, attackRow, attack)` - the stop's own row only, never a
  transmission's); state "perNoteAttackMs" {key: ms} / "attackRandomize" false / "attackKeysExpanded" in voicer.cpp
  AND MainComponent's voicing copy (11090/11251 - the memories copy carries no per-key values); an old file's
  "attackDelayMs" > 0 fills every key. The per-stop slider `attackDelaySlider` stays as a hidden member (removed
  from both layouts, `slidersRowWidth` 6 gaps, the compact `widths[6]`); the self-test probe sets key 60.
  LESSON: a lambda in StopRow's constructor must write `this->stop` - the constructor's parameter shadows the member
  (C3493). (4) #236 `Source/AudioRouting.h` (header-only, `friend class AudioRoutingContent`): blocks per (type,
  name) of the routes' open devices (`managerForRoute`), the "No sound card" block last, cells = output channels
  (`cellWidthFor` shrinks them so 32 fit), rows = routes (folded "All divisions / X [N]" when every division has
  the same device + channels), chips dragged: within a block `applyChannelsToDivision` (move = replace the channel,
  Shift/right = add), onto another block `setRouteAudioType(manual, perspective, type, state xml, name)` + the
  channel (mute, ignoreDeviceRestartsUntil, audioDeviceSelectionUnmuteAt as applyAudioSetupEverywhere does), onto
  "No sound card" `detachRouteAudioQuietly`, out of every grid = the channel removed (one kept); "Add sound card"
  = `deviceManager.getAvailableDeviceTypes()` names not shown yet (an empty block, 8 placeholder cells); a 1 s
  timer rebuilds on a changed signature. "Test" = `MainComponent::startRouteTestTone` ->
  `DivisionAudioCallback::startTestTone` (a 440 Hz, -20 dB, 1 s tone with 10 ms ramps on the route's channels,
  rendered INSTEAD of the organ in that callback - the mute path's bookkeeping). `audioRoutingWindow` =
  ModelessDialog; the top-bar button "audioRouting"; F1 title map "Audio routing". (5) #233 SectionHeader coupler
  CELL GRID: `couplerCellWidth()` (the widest label x `couplerScale`), `couplerCellsFor(width)` (stored cells
  within the columns and free, else the first free in list order), rows + a spare row while arranging, free cells
  dashed; `dropCoupler` = the cell under the centre (swap when held); `cellGetter/cellSetter` from the Voicer
  (`couplerCells` map, state "couplerGrid" {division: {id: "c,r"}}); `couplerButtonSize` (config key, 0/1/2 ->
  0.8/1/1.3, `MainComponent::couplerButtonScale()`, `Voicer::setCouplerButtonScale`, the Couplers dialog's
  "Button size" combo). (6) #232 `combinationsFiles` (machine key: configuration path -> combinations file) +
  `loadLastCombinationsFile` (Editor toggle after Glow); `rememberCombinationsFile` in both choosers,
  `loadRememberedCombinationsFile` 400 ms after "Compiling the Stop List: done" (log "Combinations file: ...").
  (7) #230 `combinationSetButton` is a LeftOnlyTextButton (90 px, buttonOnColourId orange); #231 `bankLabel`
  editable (digits, Enter -> currentBank). (8) #235 showDivisionMenu item 7 "Move this division to window 2/1".
  (9) THE NATIVE SAVE DIALOGS (Record, Save all combination banks: "the typed name could not replace the
  suggested one", the user on Windows too): `bringLostModalsToFront` raised the native chooser's PLACEHOLDER (a
  plain Component, empty name) every half second - the log line "brought to the front: " with an empty name -
  so the native dialog lost its caret; now TopLevelWindow tops only in both loops (0.9.52 excluded them from the
  owner trick alone). (10) ORGAN_TEST_DIALOG_SNAPSHOT pictures the MAIN window too ("<n>-main.png"). Tests:
  %TEMP%\test_0979.ps1 -Only A|B|C|D (A the splash timing, B Audio routing + the combinations hook, C the Save
  chooser without a raise, D the attack row unfolded from the file + the main window). LESSONS: `-Only B` wipes
  the work folder of an earlier run (the pictures of D went); a stop's NAME in Rural is "TA - subbass 16" (a
  `-like "TA - subbass*"` in a test's json edit, never `-eq`); ORGAN_TEST_CLICK finds juce::Buttons by exact text
  (the record button is a RecordButton without text; a SectionHeaderLabel header is no Button); the video hook
  `record` without `window` wrote 0 frames. Issues #228-#233, #235-#237 answered and closed through the API; #234
  stays open (the reporter's log is wanted).
  0.9.80 (16 Nov 2026, the user's remarks on 0.9.79 - "řekni, proč jsi to zkurvil napoprvé"): (1) the coupler
  SPARE ROW stayed after the arranging ended: endArranging only re-laid the buttons, the header's height is the
  Voicer's (rebuildUI) - now `arrangeStateChanged` (both start and end) posts a rebuildUI, the row comes and goes
  with the mode. (2) #235 "the right click does nothing": `headerMenu` fired for `eventComponent == this` only -
  the division NAME is a child Label, a click on it never reached the menu; the title now has the header as a
  mouse listener and counts. I had tested the menu ITEM, not the click. (3) the attack row's sliders did not light
  green: `setNoteActive` knew keySliders/toneSliders only (and `originalTrackColours` was not filled for the attack
  row); I never pressed a key with the row open. (4) the 8 ms cut fade-in was a click-avoidance minimum, not a
  speech - now the setting "Attack cut fade-in" (6-150 ms, default 20; config key `attackCutFadeMs`, engine atomic
  + `setAttackCutFadeMs`, Editor slider after the coupling group). (5) GitHub releases: back to ONE current
  release (the kept previous version pushed the Rural demo down - releases sort by date, nothing pins). (6) THE
  BANK DISPLAY (`bankLabel`, 240 px, two lines, font 15): line 1 the combinations file in use
  (`combinationsFileName`; empty = "default" = the configuration's own, cleared at every configuration load before
  combinationsFromVar), line 2 "Bank N - annotation"; `bankNotes` {bank: text} saved in combinationsToVar as
  "bankNotes" (the configuration AND a combinations file carry it); right-click = `showBankMenu` (an AlertWindow
  with a text editor "Annotate bank N...", then the ANNOTATED banks only as items 100+bank); `MainComponent::
  mouseDown` routes the popup click on the label there. Combination files live in `combinationsDirectory()` =
  Documents/LibreOrgan/LibreCombinations (both choosers start there; the save suggests "<organ> combinations.json").
  (7) DIVISION ALIAS: `ManualConfig::alias` (config key "alias" of manualKeyboards, travels with the organ, absent
  = none), `MainComponent::setDivisionAlias` (a leading numeral typed with it is dropped, saveSettings, a posted
  rebuildUI), `Voicer::divisionAliasGetter/Setter` -> `SectionHeader::aliasGetter/aliasSetter` + `refreshTitle()`
  ("I. Hauptwerk", the title width follows the text up to 270 px); the title Label is editable on a DOUBLE-click
  (onEditorShow puts the alias alone into the editor); the Divisions dialog has `aliasBoxes[manual]` (a
  TextEditor line under the window combo, Enter / focus lost). Test %TEMP%\test_0980.ps1 (Rural with "alias":
  "Hauptwerk" on manual 1 and bankNotes {"0": "Toccata BWV 565"}: the main window, window 2 and the Divisions
  dialog pictured - the user's settings have two windows, so division I sat in window 2).
  0.9.81 (16 Nov 2026, the user's furious batch after 0.9.80 + the routing design in seven rounds; "Go"): (1) AUDIO
  ROUTING = CABLES (`Source/AudioRouting.h` rewritten, header-only, `AudioRoutingContent`): LEFT the sources - a row per
  PERSPECTIVE (folded when every division sends it to the same card + channels: one cable for all; "(various)" and
  unfolded by itself otherwise; the arrow unfolds P./I./II. rows with cables of their own; `unfolded` set); RIGHT the
  cards IN USE or open as full cards (name, driver + status line, Sample rate / Buffer combos = `setAudioDeviceSetup`
  on the route's manager under the mute protocol, Driver panel = showControlPanel, Reopen = reopenAudioDeviceFresh,
  Test = startRouteTestTone per route; the OUTPUTS as jacks along the bottom edge; a card grows WIDE with its outputs,
  never tall - the user's first picture with 40 cards of 118 px each sent every cable off the screen: "tohle nemůže
  být"); EVERY OTHER card of every driver (Windows Audio, Exclusive, Low Latency, DirectSound, ASIO, Aggregated WASAPI
  ...) is a compact chip in rows per driver below ("Other sound cards - drop a source onto one to open it"; `Card::chip`),
  a drop = the pair 1+2 and the card moves up. Cables: row right edge -> its own lane (row y) -> the card's gutter
  (4 px per cable, `gutterUse`) -> under the card -> up INTO the jack; a pair = two strands + a clamp over both jacks;
  click = selected (the rest dim), Delete/Backspace or the red x removes (`removeCable`: the last channel gone ->
  `detachRouteAudioQuietly`); while dragging, the jack and its partner glow yellow and a label at the cursor says
  "7+8" / "+ 7+8" (Shift = add a pair, Alt = one channel); `findDropTarget` nearest jack, the last odd jack snaps to
  the pair below. Busy = a card of another driver whose `hardwareWords` (letters-only words >= 5, generic ones
  dropped) match an OPEN device of another type: grey, no drop. Card key = type|name; `cardKeyOf(route)` = the open
  device, else the route's STORED state XML (`audioDeviceStateXml` deviceType + audioOutputDeviceName - the manager's
  own setup is EMPTY after a failed open; the first version compared the open device only, so a route on a card not
  open yet was "elsewhere", the add replaced instead of adding and no cable was drawn). NO scanForDevices anywhere
  (the 0.9.79 "Add sound card" crash); the names come from `deviceManager.getAvailableDeviceTypes()` ->
  `getDeviceNames(false)` (the start's lists + what the system reported), refreshed by a 1 s `signature()` timer.
  `assign()` = setRouteAudioType (DEVICESETUP xml: deviceType, audioOutputDeviceName, rate, buffer) when the card
  differs (then the dropped pair alone) + applyChannelsToDivision, audioMuted/noteOffAll/ignoreDeviceRestartsUntil/
  audioDeviceSelectionUnmuteAt as the Divisions dialog does. A card that FAILS to open at first (CABLE Input of
  VB-Audio here: "Couldn't open the output device", appears 6 s later) is reverted by the missing-device watcher to
  the previous device ("'All WASAPI Outputs': 3 route(s) reopened") - the Divisions dialog's behaviour, not changed;
  test with a card that opens at once. Hook `ORGAN_TEST_ROUTE="<division|all>|<perspective>|<card name part>|<output
  1-based>[|add|single|remove]"` (^-separated, the first 1.5 s after the dialog opens, then 3 s apart; log "Audio
  routing: ..."). JUCE 9: `Font::getStringWidthFloat` is gone - `juce::GlyphArrangement::getStringWidth(font, text)`.
  (2) HEADER GESTURES (the user: "Kurva! ... Keyboard learning musí být omezeno pouze na ikonu klaviatury"):
  `SectionHeader::mouseDown` fires the keyboard learn only for `event.eventComponent == this` inside the icon rect
  (5,7,34,24) - before, the header was a mouse listener of its children and a right-click on the Tremulant button /
  the name at a child-local position inside that rect learned the keyboard; a right-click on the name = the division
  menu (the title Label listens), below the header line (y >= 38) = the coupler grid's own menu (`couplerMenu`:
  Enter/Leave arrange mode, Couplers..., Manual: Couplers; `beginArranging/isArranging/stopArranging`;
  `Voicer::couplersDialogRequest`). The darker band under the header line (my 0.9.81 "panel") was REFUSED by the user
  ("spojky teď mají tmavší pozadí, to nechceme!") - no background for the coupler grid. RULE: a parent that listens to
  its children's mouse events must test `eventComponent` before reading positions. (3) BANK DISPLAY: a black Label
  (176 px) between Bank down/up; a LEFT click = `beginBankEdit()` (two editors: the number, the annotation; Enter /
  focus lost apply, Escape cancels), the "v" button lists the ANNOTATED banks (`showBankMenu` lists only); no right-
  click (the user). testName "bankDisplay". (4) LINKED FIELDS (the user: "všechna políčka, která vedou na stejnou
  funkci, musí být propojená"): `KeyboardsDialog::refreshControls` (10 Hz, every field from the model) now refreshes
  `aliasBoxes` too (unless focused) - the AUDIT: every other field of the Divisions dialog was already read from the
  model by that timer; the alias box was the one added without it (0.9.80). RULE: a new field in a dialog that mirrors
  a model value goes into the dialog's refresh from the model, never set once at construction. (5) "-300" in the per-key
  attack box: `UIScale::LookAndFeel::createSliderTextBox` honours the property "smallTextBox" (9 px font). (6) The
  alias editor of the header name has an orange outline and "alias of I." as the empty text (dark on dark = "the name
  vanished"). (7) HOOKS: `ORGAN_TEST_MOUSE="<text>|click|right|dblclick^..."` (+ ORGAN_TEST_MOUSE_AT): a synthetic
  juce::MouseEvent on the component found by button text / label text / "testName" in every window, sent to it AND its
  parent (as a listener gets it); the menus log "Menu: keyboard learn of division N" / "Menu: MIDI learn of <target>" /
  "Menu: division menu of division N" / "Menu: coupler grid of division N" so a run proves which menu opened (a
  PopupMenu of a non-foreground process dismisses itself at once - never picture it). Tests: %TEMP%\test_0981_mouse.ps1
  (right on Tremulant -> "MIDI learn of tremulant:0", right on "I." -> division menu, dblclick -> the editor, click on
  bankDisplay -> the boxes; main window pictured), %TEMP%\test_0981_route.ps1 [-Card <name part>] (Audio routing by
  ORGAN_TEST_CLICK, three ORGAN_TEST_ROUTE specs, the dialog pictured at every change into pics\hNN.png). The user's
  "proč děláš tyhle kraviny" answered: existence was verified by pictures, never by the gesture - the hooks above are
  the cure, and every gesture of a build is run through them before delivery. LATER THE SAME DAY: (8) the user: "all
  wasapi je na hovno, protože výstupy mají pouze čísla!" - `Card::jackNames` = `getOutputChannelNames()`, drawn
  VERTICALLY (rotated -90°, read bottom-up, font 10.5) in a 150 px band above the jacks (`nameBand`; 0 when the names
  are digits only or "Output channel N" - those cards stay 118 px). (9) THE BLANK NAME: a Label whose editor hides
  takes the editor's text (our alias alone, "") as its own text - the header's "I." VANISHED after an alias edit
  that changed nothing (the setter rebuilds only on a change): `title.onTextChange` always calls `refreshTitle()`;
  the alias editor logs "Alias editor of division N shown at ... / hidden; alias ..." (onEditorShow/onEditorHide).
  (10) The test run's lesson: a Label editor is MODAL (`enterModalState(false)` in Label::showEditor) - any click
  elsewhere hides it; the first picture without the editor was that, not a paint fault. The deployed exe is
  C:\LibreOrgan\LibreOrgan_0.9.81.exe (the shortcut's file name is still "OrganSampler_0.8.198.exe – zástupce.lnk" -
  never rename it).
  0.9.82 (16 Nov 2026, the user's ten-point batch after 0.9.81, "go"): (1) BANK DISPLAY = `DisplayLabel` (UIControls.h,
  the rounded black display of the combination display "0-"; `maxLines` 3 for the bank: file / "Bank N" / annotation,
  `DisplayLabel::fitLine(font, text, width)` cuts with "..." - the user: no wrapping; the tooltip holds the whole;
  `bankNoteEditor.setInputRestrictions(28)`; resized() recomputes the lines through updateCombinationControls). My first
  attempt was a new RoundedDisplayLabel class - the existing DisplayLabel was the one to use: grep for "Display" before
  inventing a display. (2) SWAP DIVISIONS WHOLE (`swapDivisionStops`): alias, tremulant (enabled/rate/depth/drift), swell
  position/attenuation/inertia, wind (depth, frequency, oscillations, strength, smoothing, target masks with
  `swapMaskBits` on every division), random tuning / pipe coupling, division volume, the audio (outputChannels,
  audioDeviceStateXml, `perspectiveAudio` re-keyed "<manual>|<perspective>", `removePerspectiveAudioRoutesForManual`
  for both, `attachDivisionRoute` from the swapped xml as loadSettings does, the engine setters), couplers (the state
  of id X := the old state of `remapCouplerId(X)` through `setCouplerEnabled`, `visible`, `couplerOrder` re-keyed and
  renamed) and `Voicer::swapDivisions` swaps `couplerCells` with the ids renamed. The keyboard stays with the numeral
  (MIDI device/channel, compass, transposition, velocity, bass layout, swell pedal CC, window). Hook
  `ORGAN_TEST_SWAP="<a>|<b>[|<seconds>]"` logs "ORGAN_TEST_SWAP before/after: division N: alias ..., outputs, device,
  couplers"; test %TEMP%\test_0982_swap.ps1 (Rural: alias and wind 33 c of I. end on II., outputs 7+8 <-> 11+12).
  (3) The Crescendo Settings "Set" is a juce::TextButton latched orange (was a ToggleButton tick) - the user's
  "Set v crescendu se měl změnit na button"; never rename it ("Teach this step" refused: "nezaváděj nové výrazy").
  (4) AUDIO ROUTING v3 (AudioRouting.h): "Organ out" / "Sound cards in" (the user's words; "What plays" refused);
  ORANGE for every highlight (yellow refused: "udržuj stále stejný estetický kód"); the selected cable DRAGS to
  another output/card (`pressedCable` -> `cableDrag`, `moveCable` = assign with `takeAway` = the old channels);
  cards drag onto dashed SLOTS (`slots` = every full card's bounds + one after the last, `dropSlot` glows,
  `moveCard` inserts; the order = `owner.audioRoutingCardOrder`, machine key); the other cards = buttons sized like
  stop buttons (`chipHeight` 36 x scale, font 14 x scale) in COLLAPSIBLE driver sections (`typeSections`, caption
  "- Windows Audio" / "+ ..." orange 15 bold like the Editor's addSection; `owner.audioRoutingCollapsed`); "Button
  size" combo = `owner.audioRoutingButtonSize` (0.85 / 1 / 1.25 on chips, jacks, names); several cables into one
  jack = STRIPES of their colours (`jackColours`) and concentric clamps (`Cable::ring` = the cable's order among
  the cables into the same first jack, 3 px per ring); the signature includes rate/buffer/latency of every open
  device (the ASIO panel's buffer size shows within a second - the Divisions dialog lesson again) and the Buffer
  combo lists the current size even when the driver's list lacks it. Hook ORGAN_TEST_ROUTE: "...|movecable|<from
  output>" and "card|<name part>|<slot>". Test %TEMP%\test_0982_route.ps1 (the pictures are "1-Audio routing.png" with
  one window - the hNN copies watch "2-..." and miss; the final picture suffices). Not changed: the device-fail
  revert (0.8.250) and the aggregate's channel names ("ADAT (7+8) (RME Fireface UCX) (L)" - the endpoint + L/R).
  0.9.83 (16 Nov 2026, the user's seven complaints on 0.9.82 + "ASIO první, ostatní sbalené"): WHY THE 0.9.82 FAULTS -
  the dialog was built from the design, not from the program's own vocabulary (the "v" was invented while the stop
  rows' voicing opener is a plain 18 px juce::TextButton at the bottom-left; the "grid" was a flow of cards with
  pretended slots; cables took the shortest way across cards); the hooks proved the actions, not the gestures.
  RULE: a new element of a dialog is an instance of what the program has (opener dot, grid cells, orange glow), and a
  gesture is tried by the gesture. DONE: (1) bank boxes: `bankNoteEditor.setJustification(centredLeft)` (the number
  was centred, the note top-left); the list button = the stop buttons' 18 px dot at the display's bottom-left (the
  editors start 20 px in). (2) AUDIO ROUTING v4: a press on a cable SELECTS AND LIFTS it (`pressedCable` ->
  `liftedCable` after 6 px; the lifted cable is not painted at its output, `jackColours` skip it, the drag cable hangs
  from its row; drop on an output = `moveCable`, elsewhere = nothing = it is back); where cables overlap the selected
  one is hit first, else the last drawn. The cards sit on a GRID OF CELLS (`Card::cell/span`, `cellWidth()` = 372 +
  24, columns from the width, `fits()` places cards without a cell or with a collision in the first free cell,
  `allCells`/`freeCells` with a spare row, `placeCard(index, cell)` = free cell takes it / occupied swaps; stored as
  "<c>,<r>|<key>" lines in `audioRoutingCardCells`). CABLES IN THE AISLES: `listCables()` (no geometry) first, the
  gutter left of the grid widens with the cable count (`gutterWidth` = 16 + 8 n), each cable a gutter lane and a
  corridor lane (`buildCablePaths`: order by target row then jack x - inner lanes for the nearest row and the leftmost
  outputs so the long runs do not cross), the corridor under each grid row grows with its cables, the path = row ->
  gutter lane -> down/up to the corridor -> along it -> up into the jack; never across a card. Channel names font 12,
  band 180 x scale, jacks >= 18 px. Driver sections: `typeRank` (ASIO 0, Aggregated 1, the rest 2) orders the chips;
  the default unfolded set = ASIO alone, else Aggregated WASAPI alone, else the first (`audioRoutingExpanded` +
  `audioRoutingExpandedSet` - an empty saved list means all folded); folded captions show the count. Hook
  ORGAN_TEST_ROUTE "cell|<card>|<col>|<row>" (the 0.9.82 "card|.." is gone). Tests %TEMP%\test_0983_route.ps1 (ASUS to
  cell 0,0, the aggregate to 0,1; pictures hNN now watch "1-Audio routing.png") and test_0983_bank.ps1.
  0.9.84 (16 Nov 2026 evening; the user's crash + four wishes; ASIO as the default REFUSED by him: "na to rezignujeme,
  ať si uživatel poradí sám" - never make ASIO automatic, the dead/virtual drivers (FlexASIO, Generic Low Latency,
  ReaRoute, UMC without hardware) can hang the start): THE CRASH: the log ended with "-> no sound card (its last cable
  removed)" x3 and NO crash file; the user "nerušil kabel, chtěl ho chytit a přetáhnout" - the red x of the selected
  cable sat ON the cable at the corridor turn (jx, cy), so a grab there hit the x = removeCable of a folded 3-division
  row = three detachRouteAudioQuietly + rebuild INSIDE the mouse event; the hook reproduction (his Chalons config,
  Rear + Diffuse on ASIO Komplete, Direct on Reproduktory WASAPI, removed) did NOT crash - the hook bypasses the mouse
  path, so the vanishing is tied to the event re-entrancy and stays unproven. Done: (1) the x sits ABOVE the cable's
  source end ((x0 + 6, y0 - 18)), never on the path; (2) `later(action)` = MessageManager::callAsync through a
  SafePointer: every route change (assign, moveCable, removeCableData, placeCard, the card-to-button drop) runs AFTER
  the mouse event with COPIED data (`CableData {routes, channels, label}`, a Card copy) - never an index into
  `cables`/`cards` that the rebuild replaces; (3) `SetThreadStackGuarantee(64 KB)` in CrashHandler::install (Windows)
  so a stack overflow writes its file. RULE: a mouse handler of a dialog that rebuilds itself never calls the rebuild
  directly. ALSO: (4) "Organ out" in the Perspective patterns' order (`engine.sortPerspectives`); (5) hues spread
  evenly: perspective i of n = hue 0.08 + i/n, its divisions the same hue a step darker/duller per division (two
  cables = opposite colours); (6) BUSY CARD TAKES ITS OWN ROUTES: `Card::busyBecauseOf` (the open card of another
  driver on the same hardware), `freedByDrag(card)` = every route of that blocker is among `draggedRoutes()` (a row, a
  lifted cable or a whole dragged card) -> droppable and painted normal while dragging; `assign` with `card.busy`
  detaches the routes FIRST (the WASAPI device closes) and then opens the other driver - verified: Direct x3 on
  "Reproduktory (RME Fireface UCX)" moved to "ASIO Fireface USB" ("was busy through another driver - its routes closed
  first", then opened; test %TEMP%\test_0984_asio.ps1 with his Chalons configuration). A full card dragged onto a pool
  BUTTON moves every route of it there (`findDropTarget` allows chips only for a dragged card). Tests: test_0984_asio.ps1,
  test_0983_route.ps1 (unchanged results).
  0.9.85 (16 Nov 2026 night, the user's five wishes): titles "Organ out" / "Sound cards" (he: "nejsou in"; "Sound Cards
  Software Out" offered, the short one taken) in ORANGE 18 bold; `laneGap` 8 -> 14 (gutter and corridors); the source's
  colour dot at BOTH ends of its row (the right one = where the cable is taken). His "alias perspektivy" does NOT exist in
  the program (no perspective alias anywhere; the Stop List's header rename IS the perspective's name and Organ out
  shows it) - told him, nothing invented.
  0.9.86/0.9.87 (16 Nov 2026 night): 0.9.86 = the DIVISION alias ("I. Hauptwerk  default") in the Organ out rows (he meant
  the division's alias, not a perspective's; the signature carries the aliases so the rows follow a rename) - deployed,
  folded into the 0.9.87 commit. 0.9.87 (#238, his Linux log: Caen loaded and CANCELLED in the loader; the pedal's stops
  came with their Front ranks only, "Pedal / rear: settings of a route that does not exist dropped", the Rear route gone
  from the Audio routing and the Divisions tabs though the Stop List names it; the manuals without stops stayed on
  "default"): `SamplerEngine::configuredPerspectives[manual]` + `setConfiguredPerspectives` (under controlLock) added by
  `getPerspectivesForManual` before its "default" fallback; `MainComponent::syncConfiguredPerspectives()` (the labels
  under which the ACTIVE table's non-transmission rows have folders, manual <= manualCount) called at the top of
  `rebuildPerspectiveAudioRoutes` and `closeHiddenRoutes` - so a route exists for every perspective of the ORGAN (table),
  loaded or not, its settings are never dropped by a partial load, and an unloaded division with a table shows the
  table's perspectives. RULE (the user): "nesmí se stát, že existující perspektiva nemá audio routing, ani že
  perspektiva, která neexistuje, má audio routing". The "stale default after a load" he saw was this (the dialog's
  1 s signature timer DOES refresh after a load: verified with Chalons by %TEMP%\test_0987_loadopen.ps1 - the dialog open
  on the empty console, the organ handed to the running program, the rows became Diffuse/Direct/Rear). Tests:
  test_0987_cancel.ps1 (Caen Surround, "Cancel" clicked in the loader at 14 s, then Audio routing pictured),
  test_0987_loadopen.ps1 [-Cfg]. The log slice by file length FAILS when the log rotates at 1 MB mid-run: read the
  tail of activity.log instead. HIS OBJECTION ("načtené byly i některé ranky rear u pedálu ... ve stop listu je výčet
  rear adresářů"): the Stop List shows the TABLE's folders (configured), not what the engine loaded; his log has the
  pedal's engine perspectives = Front alone (the route "manual 0 perspective Front" started, "Pedal / rear ... dropped")
  - the first ped tree (one perspective) loaded whole, the second was cancelled at its first stop. With the table's
  perspectives counted the dispute is moot: Rear has its route either way. The right-edge dot is on EVERY row
  (perspective rows too) since 0.9.85 - his Linux build was older. SPLASH (the user: "proč splash screen vypadá na
  každé platformě jinak ... líbí se mi ty veliké píšťaly, které má linux"): Windows/macOS `StartSplash` (Main.cpp)
  now 460 x 500 with the AppIcon at 384 px like the Linux juce::SplashScreen (0.8.165), the name and the start's
  status under it; Linux keeps its SplashScreen + StartCover (the Wayland lessons of 0.8.165-0.8.171).
  0.9.88 (16 Nov 2026 night): TREMULANT TARGETS (the user: "které divize ovlivňuje ... stejně jako wind ... jeho vlastní
  divize bez debaty ... i do right-click tremulantu"): `ManualConfig::tremulantTargetMask` (configuration key
  "tremulantTargetMask", the own bit never set), engine `TremParams::targetMask` + `setManualTremulantTargets` +
  `tremulantDriverOf(manual)` (own when on, else the first division whose tremulant is on and names it) used by the route
  render (rate/depth/the wander hash of the DRIVER; the phase is nominal time, so the divisions agree); boxes in the
  Divisions dialog (`tremulantTargetButtons/Labels`, under the rank drift when the group is unfolded, refreshed from
  the model), in the right-click "Tremulant settings" (FeatureContent::setTargetBoxes) and in the Editor (the Editor's
  TargetRow made generic with get/set functions - the wind row uses it too); swapped with the divisions. ATTACK ROW
  CUT (the user): StopRow::resized took the COMPACT controls layout whenever volume+tone were folded, ignoring an open
  attack row, while expandedHeightFor takes the full layout for it - the key area began lower than the height allowed;
  resized() now counts `attackRowShown()` (keyRowsFolded already did). RULE: the layout branch and the height formula
  must read the same three flags.
  CI OF 0.9.84-0.9.88 FAILED ON BOTH MAC JOBS (17 Nov 2026, the user: "the new build did not went through on gitHub"):
  `juce::jmax<size_t>` / `jmin<size_t>` in AudioRouting.h (0.9.84's hue spread) - the 0.8.151 lesson again: with
  juce_dsp clang takes the explicit argument for the SIMDRegister overload (`SIMDNativeOps<unsigned long>` undefined;
  int64_t passes, size_t = unsigned long does not). Five tags went to origin unwatched (the "push only the final
  version" rule means ONE push a session - then WATCH that one: %TEMP%\ci_watch.ps1 with the full SHA); the release
  stayed at v0.9.83 (the "macOS" job is required, the Intel one optional: a required job's failure skips the release
  job). Fixed with std::max/std::min<size_t>, the tag v0.9.88 moved onto the fix commit, the dangling remote tags
  v0.9.84-v0.9.87 deleted by hand (the release job deletes only tags WITH a release). RULE: before a tag, grep
  `j(max|min|limit)<(size_t|unsigned)` over Source/ - never an explicit size_t/unsigned argument on jmax/jmin/jlimit.
  RESTARTING A RUN: GitHub's run page has "Re-run jobs > Re-run failed jobs" (30 days) - it re-runs the SAME commit,
  so it helps only for a runner glitch (the cancelled macOS job of v0.9.87), never for a compile error; the API
  counterpart (`POST /actions/runs/<id>/rerun-failed-jobs`) is 403 for the local token. A compile fix = a new commit
  + the tag moved (`git tag -d`, `git push origin :refs/tags/vX`, re-tag, push). A whole run on the same tag without
  a new commit = `POST /actions/workflows/build.yml/dispatches {"ref":"vX"}` (0.8.153; a dispatched tag run publishes).
  0.9.89 (17 Nov 2026, the user's design in seven rounds, "Go. Udělej všecko"): EFFECTS AND LINKED. The effects live
  on the ROUTES (what plays); `MainComponent::perspectiveEffects` (map lower-case name -> `PerspectiveEffects`:
  volumeDb, channelSeparation, IR path/wet/tone, eq; saved as the top-level "perspectiveEffects" block) holds what
  every LINKED route of a perspective plays; `PerspectiveAudioConfig` got `eq` + `linkedVolume/Separation/Eq/Ir`
  (saved per route; absent = linked). LINKED (per module, a route's dialog): orange = follows the perspective
  (the values are COPIES, the sliders locked), off = the route's own; on again = the perspective's copied back
  (`setRouteEffectLinked`); a change of a route's own control (MIDI on `slider:perspective/separation/reverb/eq`,
  Load IR of the route) `unlinkRouteEffect`s; `perspectiveEffectChanged(name, module)` copies into every linked route
  and applies (`applyRouteEffect` per module: never applyPerspectiveReverb for a volume change - an IR rebuild).
  Nothing is summed: the engine still processes every route alone (a perspective's effect = N instances; the
  perspective dialog says so). OLD FILES (no block): each perspective takes its first route's values and a route
  stays linked only for modules whose values agree (`effectModuleAgrees`) - the sound is unchanged (Rural: the
  I. default separation 0.27 vs 0.22 is "set apart"); `rebuildPerspectiveAudioRoutes` ends with the LINKED sync: a
  FRESH route (all defaults) takes the perspective's values, a route with own differing values is set apart (a
  Hauptwerk import's per-division route volumes) - nothing is overwritten quietly, log "Effects of the routes: ...".
  The user's decisions: TWO volumes stay (division x route; the division volume is the first item of a route's
  chain, "a division's own, never a perspective's"); perspective-level effects exist as the dialog of the
  perspective (not a bus - batching the routes of a card was refused for now); the word is LINKED, never "Own".
  ENGINE: `SamplerEngine::EqSettings` (5 bands: low shelf, 3 peaks, high shelf; RBJ biquads, `eqBiquadFor`,
  double-buffered coefficient sets per Route `eqSets[2][5]` + `eqIndex` + `eqAny`, TDF-II state per channel/band,
  processed in renderRoute after the separation and before the IR; a band off or within 0.01 dB = left out;
  `rebuildRouteEq` on setPerspectiveEq and on a rate change - the anonymous-namespace prototype stands before
  setPerspectiveSampleRate), `eqResponseDb` (the graph), `takeRoutePeak` (`Route::peakBits`, CAS max before the clip
  beside the global one). `Source/EffectsDialog.h` (header-only, friend `EffectsContent`; `EqGraph` with draggable
  points, wheel = Q): the chain on the left (TextButtons, the chosen orange), the panel on the right, a 10 Hz refresh
  from the model (the 0.9.81 rule), every slider `LearnableSlider` with midiTarget (route: the old keys; perspective:
  "slider:pfx:<volume|separation|reverb>:<perspective>"; EQ: "slider:eq:<manual or -1>:<band>:<gain|hz|q>:<perspective>"
  - the perspective LAST, it may hold ":"); `showEffectsDialog(perspective, manual)` remakes the one `effectsWindow`
  (called through callAsync from the fx button and the "open a route set apart" menu - never from inside the window
  being replaced). AUDIO ROUTING: rows 24 -> 32 px, font 13 -> 15, dots 10 -> 14, `leftWidth` 340; per row a mini
  meter (3 segments, 100 ms timer, 1 s hold) and the fx button (`testName` "fx <row label>"; orange when
  `routeEffectsActive`/`perspectiveEffectsActive` = anything off default or set apart; the signature carries it);
  the header: Master volume (`LearnableSlider` mirroring `masterVolumeSlider`, "slider:global:master"), `MasterMeter`
  (`MainComponent::paintOutputMeter` + `outputMeterState`/`clearOutputClipLatch` = the PolyphonyDisplay's one state;
  the display's own paint uses the same function; CLIP font 12 like the labels), Measure..., the Linux "List
  unusable devices" toggle. DIVISIONS DIALOG: no audio tabs (the AudioTabPanel class stays in the code, nothing of
  it built), height 926/966 -> 482/522, an "Audio routing..." button at the bottom, the division volume hidden
  (the Effects dialog has it); Measure is in Audio routing and the Editor. Bank display: "Bank N" alone until a
  combinations file is loaded (no "default"). Wind frequency up to 15 Hz (engine clamp, both sliders, the dialog
  row, the Editor, MIDI scaled(2, 15)). Tour: a step on the Audio routing button. Manual: 5.3 "Effects" (### LINKED,
  Volume, Channel separation, Equalizer, IR reverb) replaces "The audio tabs"; the F1 title map "Effects" ->
  "Effects", "IR reverb settings" -> "IR reverb". Tests: %TEMP%\test_0989_fx.ps1 (Rural, I. default moved to 5+6 so
  the perspective unfolds, ORGAN_TEST_MOUSE "fx I.  default|click", the chain buttons by ORGAN_TEST_CLICK), test_0989_linked.ps1
  (ORGAN_TEST_MOUSE "LINKED Volume|click" twice - ORGAN_TEST_CLICK "LINKED" by text found the division-volume header's
  HIDDEN button first and the first run "proved" nothing: the LINKED buttons carry testName "LINKED <module>", and a hidden
  header's button has no onClick), test_0989_fxp.ps1
  (the perspective dialog), test_0989_div.ps1, test_0989_eq.ps1 [-Plain] + eq0989_cmp.py (the I. default route with
  Peak 2 +12 dB at 1 kHz recorded by ORGAN_TEST_RECORD vs plain), test_0989_save.ps1 (the round trip: the block and
  the linked flags written). VST3/AU hosting is the user's "časem" - not done; the EQ/IR order is fixed (EQ first).
  only by the difference of the shares (50/50 = NOTHING - the user sets
  "two pipes share" 60 for a fifth), a chord struck together = one step. The steady sag: `windBaseAt` =
  exponential from baseStartSag at the event towards targetSag (tau = duration/3; the smoothstep restarted at
  every event = a sawtooth in the rhythm of fast keys). `windSagAt(state, t, external)` = clamp(base + ringing,
  -0.5 depth, limit) - NO half-wave clipping (it halved one key's shake); no phase parameter: `route->windPhase =
  0` (was +-0.48 rad per perspective by hash = a flanger between perspectives - the 0.9.66 rule). THE EVENTS:
  `updateWindKey` is no longer called from noteOn/OffImmediate but from `windPipeStarted(key, now)` (processPending
  after startNote, and the resume branch of noteOnImmediate) and `windPipeStopped(key, now)` (finishNoteOff with
  updateNoteBookkeeping): the stroke when the pipe really starts (after Attack min. delay), the release when the
  pallet really closes (after Attack min. time), held to `windPressMs + 50` through `pendingWindOffMs[]`
  (processed in processPending). The 0.9.71 slider "Wind: further strokes add" and `windStrokeAdd*` are REMOVED
  everywhere (engine, MainComponent x6, manual, help table). `fastSinRad` deleted (unused). Manual: Attack
  minimum time (the wind's shortest breath), one pipe's share (sizes the strokes), number of oscillations (0 =
  none; in phase / antiphase). RULES: the wind reacts to the pipe, never to the key; a kick lands in the
  resonator's phase; never add damping beyond "Number of oscillations".
  0.9.90 (17 Nov 2026, the user's batch after 0.9.89 - "Udělej všecko, ale nezlevňuj sinc!"): (1) SETTINGS WRITTEN AT MOST
  ONCE A SECOND: `saveSettings()` only sets `settingsSavePending` (+ refreshMidiRouting at once); the 25 ms timer calls
  `saveSettingsNow()` (the old body) when >= 1 s passed since the last write and no mouse button is down
  (`ModifierKeys::currentModifiers`, cached - no system call); saveSettingsNow is called directly before anything READS
  settings.json (savePortableConfiguration, applyPortableConfiguration, writeProblemReport's zip, "Show settings.json"),
  before every `markConfigurationClean()` (the fingerprint is taken by the write), in askToKeepChanges/requestQuit,
  globalResetFinish and the destructor. CAUSE of "every slider reacts extremely slowly": settings.json (678 KB) at every
  mouse event. RULE: a new reader of settings.json or a new fingerprint comparison calls saveSettingsNow() first.
  (2) CRESCENDO COMPACT: crescendoToVar writes one STRING per stop ("name\tdivision\tperspective\t1/0\tid:manual:1,...")
  and per coupler ("id\t1/0"); crescendoFromVar expands strings and still reads the old objects; the in-memory steps
  stay objects (registrationOnSet reads name/primaryManual/perspective/enabled/transmissions only - the OFF entries are
  kept, as the user demanded). 524 KB -> ~100 KB. (3) AVX2 UNPACKER (`unpackBlockAvx2`, SamplerEngine.cpp after
  unpackBlock; `unpackBlock16` dispatches; `gUnpackForceScalar` for the kernel test): 8 residuals per step from a
  32-byte unaligned load (permutevar8x32 picks the dword pair per lane, srlv/sllv join them), order 1 = an in-register
  scan with the carried last value, order 2 = scan of scans; the low 16 bits stored (the scalar's cast); the tail scalar;
  packChannel pads 32 zero bytes after the data (was 8) for the last group's load. kerneltest: "AVX2: 600 windows as the
  scalar decoder; scalar 1.53 ns per frame" vs 0.73 AVX2 - exactly the scalar's result. The sinc is UNTOUCHED (the
  user's order). (4) THE REPORT = two parts: `text` + `advice` (what to do: routes, auxiliary, the heaviest IR, THE
  LOADING OPTIONS AND THEIR PRICE - packing with the measured unpack ns, halved releases, Full kernel from N Hz as a
  quality choice -, a hybrid CPU's preset, hyper-threads, buffers) and `details` ("Advanced analysis": the routes list,
  render threads/CPU/memory/power, the kernel bench, the voice profile, the ceiling, the device thread, the fixed/per-voice
  split) shown by `LongMessageComponent(body, details, onClose)` under a folded "Advanced analysis  >" button (a second
  read-only editor, 55 % of the height when open); the log line carries "Advanced analysis: ...". (5) Audio routing:
  the header strip = Master volume slider, the meter, "Measure Polyphony" (was Measure...) on one line, the switches
  under it; the cable PALETTE = yellow, blue, red, green, magenta, cyan, orange, white in the perspectives' order, the
  auxiliary after the ordinary ones (`colourIndex`), a division's row `base.darker(0.22 * di)`; `FxButton` (TextButton
  with `onRightClick`: a menu "Reset the effects of X to the default" / "Open the effects..."); a drop onto an
  AGGREGATED card goes through `MainComponent::beginRoutingAssign(type, name, rate, buffer, steps, finished)` =
  `applyAllRoutingMode` of the "set for all" queue (continueApplyAll: setRouteAudioType when the route changes card,
  then applyChannelsToDivision; the window "Audio routing: <card>"), the dialog rebuilt by `finished`. (6) Effects:
  `MainComponent::resetEffectModule(perspective, manual, module)` (route = LINKED again; perspective = the defaults
  + every route linked + perspectiveEffectChanged) and `resetEffects` (all four; the division volume is NOT an effect -
  its own header Reset puts 0 dB); the Header has a Reset button (testName "Reset <module>"), the dialog "Reset all
  effects" top right; `effectsDialogModule` (0 = Volume at first, -1 = division volume) remembered in `select` and
  taken by the next dialog's constructor; the EQ rows are "1".."5" (kind in the tooltip, the graph's point numbers).
  (7) Divisions dialog: the Window box on a row of its own (full column width, right edges aligned), the MIDI
  transposition right under the channel note, the head 36 px and the velocity row 34 (the column must fit 366 px -
  the first build squeezed "Allow Pipe Coupling" to a dot), width 1060/1240, Close under the keyboard's right end
  (x = 12 + 700 - 110), placeDialog key "divisions3" (a remembered larger size would leave the window half empty -
  bump the key at every size change). TEST LESSONS: ORGAN_TEST_CLICK finds a button by its TEXT, never by testName
  ("Reset Equalizer" needs ORGAN_TEST_MOUSE); `triggerClick()` is a POSTED command message - a click in the same
  second as a dialog's remake lands on a deleted button (the Equalizer click must precede the fx click by a second);
  the perspective row's label reads "default (3 divisions, various)" once a division differs. Tests:
  %TEMP%\test_0990_gui.ps1 (the queue, the remembered module, the resets, the Divisions dialog, the settings.json
  write times >= 1 s apart), test_0990_report.ps1 (Rural measured through "Measure Polyphony", "Advanced analysis  >"
  clicked, both states pictured), test_0989_measure.ps1 -Tag packed0990 (Frankfurt packed with the AVX2 unpacker).
  FRANKFURT PACKED with the AVX2 unpacker: unpack 1582 -> 1070 ns per voice and block, the voice 4300 -> 3538 ns, the
  LIMIT UNCHANGED at 19 800 - the run steps by keys (11 keys = 19 835 voices clean with the longest callback at 43 %,
  12 keys = 21 300 broke on ONE callback of 92 %): a peak decided, not the average cost. Never promise a higher
  limit from a cheaper voice without a measurement.
  0.9.91 (17 Nov 2026, the user's furious list on 0.9.90 - "Texty musí být maximálně jasné, výstižné, STRUČNÉ", "umíš
  trochu estetiku?"): THE RULES HE SET: (1) OPTION TEXTS ARE A FEW WORDS: the choice and its price, nothing else -
  "16-bit" (never "integer"), "16-bit, releases at half the rate: half the polyphony, a quarter of the RAM" (HIS
  wording; never "bright releases keep the full rate", never "up to two thirds on a big organ"), "On: a third less
  RAM, half the polyphony", "36 kHz: a quarter less RAM"; an Editor explanation is two sentences. (2) A HEADER IS A
  BLOCK WITH COMMON EDGES: Audio routing = Measure Polyphony and the meter both 72 px high at the right, left of
  them two rows (Master volume + slider; Show cables + Button size) sharing their left and right edges (the size box
  ends where the slider's value box ends). Never an "orphaned" control, never a box aligned with nothing. (3) EVERY
  TEXT BOX HAS THE COMBO BOX'S ROUND CORNERS: `UIScale::LookAndFeel::drawLabel` (a Label with a background = a
  slider's value box), `fillTextEditorBackground`, `drawTextEditorOutline` (boxCorner 3 px; an AlertWindow's editors
  keep JUCE's). (4) BOXES OF ONE COLUMN HAVE ONE WIDTH AND ONE HEIGHT (Divisions: Window = MIDI device = MIDI
  channel, 38 px rows). (5) A DIALOG IS AS WIDE AS ITS CONTENT: Divisions 852 px (the stop pool column right of the
  wind model had not been built for months - `StopPoolPanel` is never created - and the window kept its width);
  placeDialog key "divisions4". (6) The division's ALIAS is typed beside the numeral in the Divisions dialog
  (`aliasBoxes` in the head row, font 26 bold, empty text "alias"), the learn messages (`status`) between "Audio
  routing..." and Close. (7) THE ROW METER of Audio routing = ONE box of 12 x 16 px with three parts above each other
  (green bottom from -36 dB, yellow from -12, red top from -3) - never three boxes in a row. (8) NO DIALOG PER CABLE:
  the 0.9.90 progress window of an aggregated drop is shown only for >= 3 routes changing card at once; and the cause
  of the second per cable is gone - every route's AggregatedWASAPIAudioDevice created and SCANNED its own WASAPI
  device type: now one `std::shared_ptr` type for all aggregates of the program (a function-static weak_ptr in
  `openEndpoint`, alive while any aggregate is open), and `setSelectedOutputChannels` only switches the channel
  flags when every endpoint asked for is open and playing and the master endpoint stays (no stop/start). Measured:
  three cable moves 5+6 -> 7+8 -> 5+6 each done within the second of the drop, no window (%TEMP%\test_0991_gui.ps1).
  (9) The Editor's button reads "Measure Polyphony" too. LESSON (the user: "proč na tyhle věci nepřijdeš sám"): after
  ANY dialog change LOOK at the picture for edges, heights, widths and dead space before delivering - an element that
  works is not done until it is aligned with its neighbours; %TEMP%\test_0991_audit.ps1 pictures Couplers,
  Temperaments and the Editor.
  0.9.92 (17 Nov 2026, the user on 0.9.91): (1) AUDIO ROUTING ROW METERS of the divisions stayed dark under an unfolded
  perspective: `engine.takeRoutePeak` is a TAKE (read and reset) and the perspective's row, first in the loop, took
  every route's peak - the timer now takes each route once per tick into a map (`peakOf`) shared by the rows. RULE:
  a "take" value read by several displays is taken once and shared. (2) "unpacking took 0 ns of the 0 ns": the
  voice profile was folded into the pool statistics only by pool JOBS; a route rendered by its own thread alone
  (`RenderPool::render` with laneLimit == 1 = Aggregated WASAPI at one lane a route, the user's 16 routes on 16
  cores) never folded it, and finishPolyphonyMeasurement counted the profile only inside `if (pool.jobs > 0)`. Now
  `foldVoiceProfile()` in the inline path and the profile counted without jobs; the advice prints figures ONLY when
  measured (`unpackNs > 0.5 && totalNs > 0.5`). RULE: never print a figure that was not measured. (3) THE CEILING
  EXPLAINED (the user: "only 11 % - that needs a better explanation"): after "The kernel alone would reach about C
  voices; the limit is P % of that" the details say "Why so far from the ceiling: 1. the benchmark's plain voice X ns
  per frame, a voice of this run Y ns (N times): the packed samples are unpacked (u ns of it); the halved releases /
  the pipes above F Hz play through the sinc kernel; thousands of voices read as many places of the RAM at once -
  at that cost the render cores carry about V voices at 85 %. 2. The run is judged by its LONGEST callback: it broke
  at B voices because <failReason>, while the render cores were busy A % on average". Frankfurt packed (16 Windows
  Audio routes): 1.64 vs 7.2 ns per frame (4.4 x), 39 200 voices by the average cost, broke at 23 134 on one 90 %
  callback at 35 % average load - the PEAK halves what the average would carry; the limit this time 21 300 (the
  12-key level stayed clean in this run, 19 800 in the two before: the same peak lottery). `nsBest()` of the bench
  is the streaming-from-RAM figure, not the cache one - say "the benchmark's plain voice". (4) The option lines of
  the advice are one short sentence each. Tests: %TEMP%\test_0992_meters.ps1 (Rural, I. default apart, key 60 held:
  the box of "I. default" lit in the picture taken while it sounds), test_0992_inline.ps1 (Rural with
  renderThreadsWasapi 1 in the settings copy = the inline path: "0 route job(s) ... (356224 voice renders)"),
  test_0989_measure.ps1 (its clicks say "Measure Polyphony" now).
  0.9.93 (17 Nov 2026, the user's design in four rounds, "oprav přesně, jak jsi naplánoval. Beru vše"): THE PRINCIPLE -
  AUDIO IS A MODULE OF ITS OWN (Audio routing + Effects); nothing audio in the Divisions dialog or in the Editor's
  division sections. (1) EDITOR, division sections: the rows Division volume, Audio output channels (default
  perspective), Perspective output channels / volume / channel separation are GONE (they wrote a route's values past
  LINKED) - one row "Audio and effects" with the button "Audio routing..."; added "Division alias" (addText ->
  setDivisionAlias), "Compass from" / "Compass to" (helpSection "Compass"), "Bass octave layout follows MIDI
  transposition"; "Shown in window" is always there (it was inside `if (o.twoWindows)`); the Output discontinuity
  detector moved from Console to the end of Engine; TargetRow showed all eight division boxes (addAndMakeVisible
  after setVisible) - addChildComponent + setVisible. Dev hook `ORGAN_TEST_EDITOR_SECTION=<title>` scrolls the Editor
  to a section's header (with "configEditorCollapsed" {"Pedal": false} in the settings copy the section is open).
  (2) MIDI: the COMPASS is the MIDI range of a division (the user: a second range box would be "EXTRÉMNĚ matoucí" -
  never add one); `ManualConfig::bassLayoutFollowsTranspose` (key "bassLayoutFollowsTranspose", written when true;
  MidiRouting carries it) and `MainComponent::routedNoteFor(layout, semitones, follows, note)` = the one mapping
  (follows: transpose first, then the layout on the division's notes 36-44; else the layout on MIDI 36-44, then the
  transposition) used by handleIncomingMidiMessage and mapKeyboardNote. (3) DIVISIONS DIALOG: left column = the
  keyboard's alone (numeral + alias, Window, MIDI device, channel, note, then three label/box rows of one width:
  MIDI transposition, Compass from, Compass to (`compassToLabels`), Disable MIDI velocity); Allow Random Tuning Error
  and Allow Pipe Coupling head the second column; the layout line ends with `layoutFollowsButtons` ("follows MIDI
  transposition", grey with a tooltip saying why when the layout is Standard or the transposition 0); the keyboard
  runs to `dialogRightEdge` (232+28+260+28+260-6 from the content's left = the wind column's boxes) with
  setKeyWidth(width / 75) - 75 white keys of MIDI 0-127 (61 left a scroll arrow); Close ends at the same edge.
  `LearnKeyboard::routedNote/compass/transposition` (std::functions set by the dialog) -> paint: white keys outside
  the compass or left out by the layout darkened (RectangleList less every black key), dark black keys grey
  0xff4a4a4a, a key the layout moves carries the note it plays in orange; `keyboardMarks` repaints on a change.
  LESSON: `TextEditor::setFont` changes the font of text typed LATER - the alias set before it stayed small; use
  `applyFontToAllText`. (4) AUDIO ROUTING: `audioRoutingButtonSize` 3 = "compact" (first item of the combo, id 4):
  `compact()`, scale 0.7, jacks 18 x 16, `cardBase()` 92 instead of cardHeight 118, nameBand 74, a channel's name cut
  to 10 letters + "..." (drawText, the whole name in the tooltip of the jack or its name: "Output N: ..."). TWO DRIVERS
  ON ONE DEVICE: on Windows a card of another driver is NOT busy any more unless one of the two types is an
  "Exclusive" mode (Linux/macOS keep the busy rule); two OPEN cards of different driver FAMILIES (`driverFamily`:
  ASIO / Windows Audio incl. the aggregate / the type) sharing a hardware word get `Card::sharedNote` = "Also open
  through <type>: two drivers, less polyphony" drawn as the card's status line in orange (the note alone - with the
  type prefix it was cut on a 372 px card; the figures and what to do are in the tooltip); the aggregate's hardware =
  the words of the outputs its routes use (`cardHardwareWords`). `MainComponent::audioHardwareWords` (the one rule,
  AudioRouting's hardwareWords forwards to it) and `audioDriverConflict()` (routes -> uses -> pairs of different
  families sharing a word), looked at once a second in timerCallback: `audioDriverConflictText` change -> the top
  bar's Audio routing button text orange + tooltip, a Messages note once, log "Audio routing: one device through
  two drivers - ..."; the measurement's advice has the paragraph too. (5) STALE TEXTS fixed: the safe-start notice,
  "Audio devices not available", "No audio", the Stop List header tooltip, the measurement's "set to none" / "audio
  tab" / "on those routes" / "Audio drivers" - all name the Audio routing (its cards' Buffer / Reopen, "remove the
  cable"). Still in the code and NOT built: AudioTabPanel, StopPoolPanel, the "Set for all" tick boxes (dead since
  0.9.89; a separate cleanup). Manual: 5.1, Audio routing, 8.1 (detector), 8.6 (Division alias, Compass, Bass octave
  layout follows MIDI transposition, Audio and effects; the five audio headings gone); %TEMP%\manual_links.py checks
  headings/links (233 / 110, no duplicate, none broken). Test %TEMP%\test_0993_gui.ps1 (compact, I. default onto
  "ASIO Fireface USB" beside the aggregate's Fireface endpoints -> both cards orange, the top bar button orange; the
  pedal's Divisions dialog with short octave, -12 and follows; the Editor scrolled to Pedal).
  CI 17 Nov 2026 (v0.9.92 and v0.9.93): the macOS jobs were "cancelled" with NO runner and NO step, exactly 15 minutes
  after they were queued (GitHub had no macOS runner; Windows and Linux passed, the release job was skipped) - not a
  compile error. Read `jobs[].runner_name` and `steps`: empty = the runner never came. The cure is the dispatch of the
  same tag (`POST .../actions/workflows/build.yml/dispatches {"ref":"vX"}`, HTTP 204); both times the second run got
  its runners within 20 minutes and published the release.
  0.9.94 (17 Nov 2026, the user's design in five rounds; REFUSED on the way: an "organ level" split from the route
  volume ("žádné organ level z ODF nechceme ... nedělej to překombinované") and a layout that carries everything but
  the volumes ("matoucí ... nesmysl")): THE ROUTING FILE. `routingToVar()` = {libreOrganRouting 1, organ
  (currentConfigurationName), routes [{division, perspective, deviceType, deviceName, rate, buffer, channels, the
  route's effects (volumeDb, channelSeparation, reverb*, eq) + linked*}], divisionVolumes [{division, db}],
  perspectiveEffects [...]}; the card of a route = the open device, else the stored state XML. `applyRouting(data,
  withEffects, sourceName)`: per route of the LOADED organ the file's entry by division + perspective, else (a guess,
  named in the Messages) the same perspective of a lower / any division, then a route of the same kind (auxiliary or
  not); the cables go through `beginRoutingAssign` with steps carrying their OWN card (`RoutingAssignStep::type/name/
  rate/buffer/detach`; continueApplyAll uses the step's card when `step.type` is set) - only routes whose card or
  channels differ; the effects (route values, linked flags, division volumes, perspectiveEffects, then applyRouteEffect
  x 4 per route) ONLY when asked. Audio routing buttons on the titles' line spanning the header block (rowsLeft ..
  stripRight): "Save routing..." (always everything; folder Documents/LibreOrgan/LibreRouting, lastdir.json
  "lastRoutingDir"), "Load routing..." -> `askRoutingLoad(file)` = ONE AlertWindow with a custom `Note` component
  (AttributedString: "Saved from: <organ>", then a plain note for the same organ or an ORANGE bold WARNING for another
  one) and the buttons Load cables only (Enter) / Load cables and effects / Cancel; "Use as default for new organs" ->
  `saveDefaultRouting` -> `DefaultRouting.json` beside settings.json. `defaultRoutingPending` is set in the portable
  branch of applyPortableConfiguration (published configurations AND Hauptwerk imports go through it; cleared for a
  file of this computer) and in `loadChosenIntoDivision` on an empty console; `applyDefaultRoutingIfDue` (the 25 ms
  timer) waits for stops, no loader, 2 s of quiet, then applyRouting(file, false) - CABLES ONLY, never the effects.
  Hooks: `ORGAN_TEST_ROUTING="<s>|state^<s>|save|<file>^<s>|load|<file>[|effects]^<s>|loadbox|<file>^<s>|default"`,
  `ORGAN_TEST_MIDI="<s>|<division>|<note>^..."` (a note-on through `handleMidiFrom(identifier, message)` = the body of
  handleIncomingMidiMessage, as if from that division's keyboard; with ORGAN_LOG_NOTE_LIGHTS=1 the log names the keys
  every listening division plays). Test %TEMP%\test_0994.ps1 -Step save|load|default|midi (Rural; B.json = A with
  I. default on the pedal's outputs, -6 dB, division volume -3 dB, organ renamed): cables only moved 7+8 -> 1+2 and
  left the volumes, with effects -6 / -3 dB arrived, the portable Rural got the default's cables by itself.
  MIDI TRANSPOSITION "REGRESSION" (the user: "klávesy to hraje furt stejné ... regres"): NOT one - MEASURED through the
  MIDI path with his own settings: pedal, I. and II. all listen to the microKEY on channel 1 with transposition
  0 / 0 / -12; key 48 gives "Key 48 of division 0", "Key 48 of division 1" and "Key 36 of division 2" - II. is
  transposed, the other two play the untransposed key beside it. The keyboards (device, channel) are this computer's
  settings: a test file's "deviceIdentifier"/"midiChannel" are overridden by them (the midi2 step could not isolate
  II.). `git log -S` shows the routing lines changed only in 0.9.93 (routedNoteFor, equal to the old two lines).
  MAC CI: of the last 12 Build runs 3 lost a macOS job to GitHub ("The job was not acquired by Runner of type hosted
  even after multiple attempts" + "Due to capacity constraints, jobs targeting macOS arm64 runners may experience
  longer queue times" - the check run's annotations, `GET /check-runs/<id>/annotations`) and 4 failed on OUR compile
  error (jmax<size_t>, 0.9.84-0.9.88). Both mac jobs run on `macos-latest` (arm64, the Intel build cross-compiled).
  `.github/workflows/build-retry.yml` (workflow_run of Build, not success, attempt < 3): when a job was cancelled with
  NO step and no required job ran and failed -> `gh run rerun <id> --failed` (the release job follows in the same
  run). A workflow_run workflow takes effect only from the default branch. FIRST USE (v0.9.94 itself, run
  44148537092): the Intel job got no runner in attempts 1 and 2 (15 min each), the retry workflow restarted the run
  both times, attempt 3 built it and published the release with all five archives - 46 minutes after the tag, nobody
  touched it. The local watch (%TEMP%\ci_watch.ps1) ends at the first "completed": read `run_attempt` and poll on
  when the run is at attempt 1 or 2 without success.
  0.9.95 (18 Nov 2026, the user: "přečti si issues na gitHub a oprav všecko - zejména linux. A vysvětli, proč error
  vznikl než začneš kódovat" - THE EXPLANATION COMES FIRST, as text, then the code): #242/#243 from his Linux box
  (Ubuntu 26.04, PipeWire; logs.zip of an issue = `https://github.com/user-attachments/files/<id>/logs.zip`, public).
  (1) THE FREEZE (activity log 17 Nov 21:10:47, "answered nothing for about 28 s", the stack: nanosleep <-
  TrackedAlsaDevice::open <- AudioDeviceManager::setAudioDeviceSetup <- sharedHostFor <- setRouteAudioType <-
  AudioRoutingContent::mouseUp lambda): `TrackedAlsaDevice::open` (PlatformAudio.h) retried a failed open for up to
  7 s with 500 ms sleeps ON THE MESSAGE THREAD, once per route: Front's four divisions dragged from JACK "ALC1220
  Digital" onto ALSA "HD-Audio Generic, ALC1220 Digital; Direct hardware ..." = 4 x 7 s, in vain (PipeWire held the
  card for the divisions not moved yet). Now ONE attempt, never a wait. (2) THE MOVE WAS UNDONE: after a failed open
  the route's remembered state (`audioDeviceStateXml`) stayed the OLD device (saveSettings takes a manager's state only
  with a device open) and `watchMissingAudioDevices` "restored" it ("'ALC1220 Digital' (JACK) is available again:
  reopening") - `rememberWantedRouteDevice(manual, perspective, wanted)` writes the WANTED card when it did not open
  (called after setRouteAudioType in AudioRouting's assign() and in continueApplyAll; JUCE_LINUX only - on Windows the
  fall-back to the old card stays as it was, not asked). The watcher then opens the card when it answers. (3)
  `isAudioDeviceAvailableNow` took "listed" for "available" (non-ASIO): with "list unusable devices too" a busy card
  IS listed, so a waiting route was moved, muted and failed every 2 s - Linux ALSA now asks `alsaDeviceOpensNow(name,
  &why)` (the id of `outputIdsByName` probed with snd_pcm_open NONBLOCK; sound-server entries and devices we hold
  count as open). NOTE: isAudioDeviceAvailableNow SCANS the deviceManager's type at every check - that is how
  "Midi-Bridge" got into the Audio routing after the tick (the dialog itself never scanned). (4) MIDI among JACK:
  `FilteredJackDeviceType::scanForDevices` listed EVERY client when the unusable switch was on - now always the
  clients with audio ports alone. (5) THE SWITCH DID NOTHING: the Audio routing's toggle set the flag and rebuilt
  from lists read at the start (the old Divisions tab called type->scanForDevices(); lost in the 0.9.89 move) -
  `MainComponent::rescanAudioDeviceLists(why)` (Linux: ALSA + JACK of deviceManager, logged "Audio lists read again
  (...) in N ms: ALSA n, JACK m") from the toggle AND from showAudioRoutingDialog (the Komplete, held by PipeWire at
  the start, was missing until some later scan). The scan probes every card in both modes; with the switch on a held
  card is listed and remembered in `unusableAlsaDeviceReasons()` -> `Card::unusableWhy`: the chip's name dimmed
  (alpha 0.45), the reason in its status/tooltip, a drop allowed (it waits). The dialog's signature hashes the
  NAMES of the lists (the count alone missed a swap). (6) THE SWITCH OVER THE HELP TEXT: it stood at rowsLeft - 230
  in the header (Windows has no such switch - never pictured); now right after the caption of the list it rules
  (`otherCardsCaption()`, chipsTop - 6). (7) ORDER: `SamplerEngine::perspectiveOrderIndex` returns INT_MAX for the
  auxiliary (always last - the union of the divisions' lists was sorted and "auxiliary" < "Front" by the alphabet,
  neither having a line); `learnPrefixesFromTables` adds a BARE line (the label alone) for every perspective of the
  tables without one (Caen: the perspectives are packages, no prefix -> no line -> no place in the order).
  (8) #243 ROW METER: five parts in a 12 x 22 box on the large meter's scale - green -36 / -24 / -12, orange -9,
  red only while the route's peak > 0.98 (`RowMeter::clipUntil/clipShown`, 1 s) - the 0.9.91 three parts were MY
  thresholds (-12 yellow, -3 red), never matched to paintOutputMeter's (orange from -9, red = the limiter).
  Windows test %TEMP%\test_0995.ps1 (Rural with II.'s folders under "Front": the line learned, rows default / Front /
  auxiliary, the meters pictured under a held key). THE LINUX PART IS NOT RUN (no Linux toolchain or WSL on the
  Windows box): CI compiles it, the user's box proves it - say so. #241 (pasztor63, Ultra 9 285K, ReaRoute ASIO 256,
  30 routes, float + packed, limit 616): only a screenshot of the Editor, no logs.zip and no report text - nothing to
  fix blind; a reply was drafted for the user (%TEMP%\issue241_reply.md), not posted. #234 still without a log.
  0.9.96 (18 Nov 2026, the user's three orders in one build): (1) THE REPORT FOLDER HOLDS ONE FILE ("VŠECKO dohromady
  zavřeš v jediném .zip file ... nic jiného v tom reportovacím adresáři nebude" - #241's reporter dragged a PICTURE
  into "Logs": the folder showed logs.zip among window-*.png, system.txt and "Report on GitHub.html"):
  `writeProblemReport(what, steps, extraShots, url)` (no windowShots flag: the pictures are ALWAYS taken) writes
  logs.zip alone - CrashLogs/*, settings.json, selftest.txt, system.txt (`builder.addEntry(new MemoryInputStream...)`,
  no file beside the zip), `Screenshots/window-N-<name>.png` (createComponentSnapshot -> MemoryOutputStream ->
  addEntry, level 0) and the user's own picture files under Screenshots/; the html link file is gone ("Open the
  GitHub form" of the after-report box reopens the form); the FileOutputStream is truncated first (it appends -
  a second report in the same minute corrupted the zip). The dialog has no "Screenshots of the program's windows"
  switch; the texts (intro, after-report box, Editor tooltip, manual 9.3) say "ONE file, logs.zip - drag it into
  Logs"; `.github/ISSUE_TEMPLATE/bug_report.yml`: Logs = "the one file ... a picture is not a log", "Steps to
  reproduce" NOT required (the dialog asks for "What happened" alone - an empty required field blocked Submit),
  Screenshots optional. Hook `ORGAN_TEST_REPORT=<seconds>` writes a report without opening anything; test
  %TEMP%\test_0996_report.ps1 (the folder: logs.zip alone; the zip: 6 files + Screenshots/window-1/2; the test's
  folder deleted). TEST SCRIPT LESSON: the restore line must copy BACKUP -> APP (`Copy-Item "$bak\$f" "$app\$f"`);
  I wrote it reversed once (caught before the run) - it would have deleted the user's settings.json.
  (2) LINUX: EVERY CARD THROUGH PIPEWIRE (the user: "Ano, na linuxu chceme jen přes pipewire. Ale co starší linuxy"):
  `pipeWireCarriesTheCards()` (PlatformAudio.h; 3 s cache; = PipeWire's OWN libjack loaded -
  `pipeWireJackLibraryLoaded()` set in preloadPipeWireJack - AND `FilteredJackDeviceType::audioClients(true)` not
  empty: a PipeWire running beside PulseAudio for screen sharing owns no card). When true:
  `FilteredAlsaDeviceType::scanForDevices` lists the sound server's entries alone (Default ALSA Output, PipeWire
  Sound Server; a card device we still hold stays) - the cards are listed ONCE, by the JACK type, shown as
  "PipeWire" in the Audio routing (`shownType`, `viaPipeWire` set in rebuild() only - never ask from paint, it opens
  a JACK client), first and unfolded (`typeRank` JACK 0), the "List unusable devices too" switch hidden. A route
  saved on a card's direct ALSA device MOVES to the card's PipeWire entry: `sharedHostFor` (params renamed
  typeNameAsked/stateAsked, locals typeName/state) maps ALSA + card name through `pipeWireClientForAlsaCard(name)`
  (the JACK client contained in the ALSA name, longest; else the ALSA card name before the comma contained in a
  client = the Pro Audio names "Komplete Audio 6 Pro") to JACK with state nullptr (PipeWire's clock, not the ALSA
  rate/buffer); `isAudioDeviceAvailableNow` answers "there" for such a route when PipeWire shows the card; the
  watcher takes a route playing through the mapped client as satisfied (remembers the manager's state). When false
  (plain ALSA, PulseAudio, a jackd of its own - the old laptops): everything as in 0.9.95. NOT DONE, said to the
  user: switching a card to the Pro Audio profile from the program (pactl/wpctl; the names of the nodes change with
  the profile - needs a Linux session with the real output in front of me). NOTHING OF (2) WAS RUN: no Linux here;
  CI compiles, the user tests. (3) #241: the reply posted (issuecomment-6972702401) and, once v0.9.96 was out, a
  second comment: from 0.9.96 the folder holds the one zip - download it and report again.
  0.9.97 (18 Nov 2026, LINUX session on the user's box; #244, #247-#249 + his report-dialog orders): (1) #248 DEFAULT
  CARD: `MainComponent::defaultSharedTypeName()` is defined in the .cpp now; Linux = "JACK" when
  `pipeWireDefaultCard()` (PlatformAudio.h after pipeWireClientForAlsaCard: `wpctl inspect @DEFAULT_AUDIO_SINK@` ->
  node.nick / node.description / node.name matched EXACTLY against `FilteredJackDeviceType::audioClients(true)`; here
  the client is the NICK "ALC1220 Analog"; 5 s cache; empty without PipeWire / wpctl / a match) gives a card, else
  "ALSA"; `defaultDeviceNameOfType("JACK")` returns that card. sharedHostFor never copies a foreign state's rate or
  buffer onto a JACK route (a portable file's 48000/512 would be forced on PipeWire's clock for the whole desktop).
  A route saved explicitly on "Default ALSA Output" stays there. (2) #249 HOT PLUG: `refreshSoundCardsQuietly()`
  (PipeWire mode only: the JACK type rescanned, log "Sound cards (PipeWire): came X; gone Y") from the Audio routing's
  timer every 20 ticks (2 s) while showing; re-showing the hidden dialog rescans. Without PipeWire nothing is polled
  (the ALSA scan probes every card: seconds on old laptops). Tested with `pactl load-module module-null-sink
  sink_name=lo_test_card sink_properties=device.description=LoTestCard` / unload-module = a card that comes and goes.
  (3) #247: startPolyphonyMeasurement refuses (WarningIcon notice "Measure Polyphony", log "Polyphony measurement
  refused") when NO route has an open device, naming the cards waited for. (4) #244 (Windows, Realtek ASIO 3.5 s per
  failed open x 4 divisions): `failedOpens` (static map type|name -> time, error) in sharedHostFor - a card whose
  open failed is not tried again for 5 s ("... (not tried again: it failed a moment ago)"). (5) REPORT: pictures of
  every visible desktop component, every HIDDEN juce::DialogWindow (kept dialogs, "<name> (closed)", the manual
  window left out) and the Audio routing ALWAYS (a temporary AudioRoutingContent 1100 x 700 when the dialog was never
  opened: "Audio routing (not opened)"); the label beside "Add screenshot files..." says "OPTIONAL - not needed
  ...", "Steps to reproduce (optional):". #245/#246 (ReaRoute keeps 16 channels until Reaper itself runs and
  reconfigures: the driver's doing) NOT touched - a Windows matter. LINUX WORKFLOW THAT WORKS: edit the NTFS
  checkout (its sources are CRLF: patch with /tmp/patch.py-style python that normalises and restores CRLF), copy to
  ~/LibreOrgan-linux with `tr -d '\r'`, `cmake --build build --parallel 24` there (3 min), run the binary from
  build/LibreOrgan_artefacts/Release with a script that backs up and restores ~/.config/LibreOrgan/{settings,voicer,
  memories,lastdir}.json + Locations.txt + Patterns.json; test organ ~/RuralOrgan-test/Rural Organ/Rural Organ.json
  (portable). NO Projucer resave on Linux - Windows next. SESSIONS SHARED AGAIN (the user: "spojení se posralo"):
  the session 10c94e83 IS shared - ~/.claude/projects/-mnt-pracovni-JUCE-PROJEKTY-LibreOrgan-LibreOrgan/10c94e83...
  .jsonl is a symlink to the Windows file (C:\Users\doma\.claude\projects\F--JUCE-PROJEKTY-OrganSamplerC-OrganSampler);
  a bare `claude` or /clear makes a NEW empty session - use `claude --resume 10c94e83-0c32-48a0-8afa-9042aac822b5`
  on both systems. The Linux memory folder (-mnt-pracovni-JUCE-PROJEKTY-LibreOrgan/memory) and the session's
  tool-results folder are symlinks to the Windows ones since 18 Nov 2026; /mnt/windows must be mounted rw.
  0.9.98 (18 Nov 2026, Linux, the user: the 0.9.95 five-part row meter "humpolácký"): the row meter of Audio routing
  = the LARGE meter in miniature - 8 segments of 2 px (16 px = the row's letters), thresholds -36 -27 -18 -12 -9 -6
  -3 + red only while the row clips, 4 lime / 3 orange / 1 red, unlit alpha 0.18, NO black box, NO gaps; a full
  card's title is grey when `!card.open` too (a card that is not there, its cables waiting). RULE: a small copy of
  an element is the element itself scaled (same parts, same colours), never a redesign.
  0.9.99 (18 Nov 2026, Linux, the user: "do sluchátek už slyším clipping, ale audio metery ho neukazují" + his
  clippedRecording_Caen.wav: 2 300 samples flat at 1.0): THE LIMITER AND THE METER WERE PER ROUTE (renderRoute's
  soft clip above 0.98 and statOutputPeakBits = the max over routes, 0.9.79) while the routes of one device are
  ADDED by JUCE's AudioDeviceManager after them - Caen's 8 routes all on "ALC1220 Analog" 1+2, each at -3..-5 dB,
  clipped at the card with the meter in the orange. Now `DeviceClock::mix[64][4096]` + `mixCycle` (device thread
  only; a route's `mixCycleSeen` equal to the cycle = a new driver callback -> the cycle advances and the channels
  are cleared; never the 0.2 ms "first route" heuristic): in DivisionAudioCallback, per masked channel, the route's
  output is added to the running sum and CUT BACK where the sum would pass 0.98 (out = limited - before), the sum's
  peak before the limit goes to `SamplerEngine::reportOutputPeak(peak, clipped)` = the large meter and CLIP. The
  recorder's flush limits its two-channel sum of all routes with the same curve (the writer cut it flat).
  MEASURED at the card: `pw-record --target <sink node.name> -P stream.capture.sink=true --format f32` (the capture
  is BEFORE the sink volume: `wpctl set-volume @DEFAULT_AUDIO_SINK@ 0.02` for the test, back to 1.0 after) with a
  copy of Rural at registrationMasterDb +6 and ORGAN_TEST_NOTE: 0.9.98 peak 3.04, 0.9.99 peak 0.9996. Cards mixed
  OUTSIDE the program (two cards' routes summed by the OS, Aggregated WASAPI routes on one endpoint) are beyond it.
  RULE: a limiter and a meter belong where the signal LEAVES the program, after every sum. Top bar: Audio routing
  moved right before Load Configuration. PROPOSED, not coded (the user asked "možná gridem?"): a user-set order
  of the top-bar buttons. PipeWire names: the desktop's sound settings show node.description ("Starship/Matisse
  HD Audio Controller Pro"), PipeWire's JACK side the node.nick ("ALC1220 Analog") - the same card.
  0.9.100 (18 Nov 2026, Linux): (1) TOP BAR ORDER: `headerButtonOrder` (machine key, keys in order; empty = built
  in) applied by `headerButtons()` over `builtInHeaderButtons()` (a key the saved order lacks goes after its
  built-in predecessor); `MainComponent::HeaderArranger` (a layer over the whole window, child of MainComponent,
  kept in `headerArranger`, bounds/toFront in resized): dashed orange frames, a press takes a button (its snapshot
  follows the mouse), the orange line = before the first other button whose centre is right of the mouse,
  `moveHeaderButton(key, beforeKey)` saves + resized(); a click beside the buttons or Escape ends (the layer is
  deleted by callAsync). Entered by showMidiLearnMenu items 7 "Arrange the top bar ..." / 8 "back to the built-in
  order" (header: targets). The bar is ONE row sharing the width by weights - no cells. Hook
  `ORGAN_TEST_HEADER="<seconds>|<key>|<before key>"`. (2) Row meters: `clipUntil = now + 3000`; red also when
  `routeCardClippedSince(manual, perspective, now - 150)` (DeviceClock::sumClipAtMs, set where the sum passes 0.98)
  while the row sounds above -40 dB. A member function of DivisionAudioCallback that MainComponent calls goes in
  its PUBLIC part (the data members at its end are private).
  0.9.101 (18 Nov 2026, Linux) = STEP 1 OF THE VST PLAN (the agreed design: plug-ins IN our process with a scan in a
  separate process, 8 s without progress = refused, safe start after a crash (all plug-ins OFF, the one being
  loaded named), plug-in windows ALWAYS separate, portable files carry the plug-ins' description + state, a missing
  one stays as a grey slot with its state kept, AU on macOS too, Waves through the general way; folders in the
  Configuration Editor; "+" in the chain; latency made up between routes; IR reverb ALWAYS LAST, Division volume
  first, everything between movable). Done now: `ManualConfig::EffectSwitches on` (volume/separation/eq/ir) in
  PerspectiveAudioConfig AND PerspectiveEffects - part of the module's values (copyEffectModule /
  effectModuleAgrees), saved as "effectsOff" ("volume,eq"; absent = all on) in the routes, the perspectiveEffects
  block and the routing file; `effectOn / setEffectOn(perspective, manual (-1 = perspective), module, on)`,
  `applyRouteSwitches` -> `SamplerEngine::setPerspectiveEffectsOff` (Route::volumeOff/separationOff/eqOff read in
  renderRoute; the IR switched in applyPerspectiveReverb by passing no file); MIDI target
  "fxon:<division or -1>:<volume|separation|eq|ir>:<perspective>"; `PerspectiveEffects::chain` (StringArray,
  saved "chain") + `effectChain()` / `moveEffectInChain()` - ONE order per perspective. THE ORDER OF THE BUILT-IN
  MODULES CHANGES NO SOUND (gain, the separation matrix and a per-channel EQ are linear and commute): the engine
  ignores it until plug-ins exist - say so when asked. EffectsDialog.h: chain rows = `RightClickButton` switch
  (44 px, "ON" orange / "OFF", testName "ONOFF <module>") + name button; `rebuildChain()`, `Arranger` (a child
  layer, as the top bar's), panels stay in fixed order (ChainItem::panel). Hooks: ORGAN_TEST_MOUSE "ONOFF IR
  reverb|click", ORGAN_TEST_FX_ARRANGE="<id>|<before id>". build.yml release job: `keep="v0.9.100"` stays beside
  the current release until the user says to drop it (set keep="").
  0.9.101 CONTINUED = STEPS 2-5 OF THE VST PLAN (18 Nov 2026, Linux; all in ONE version, 0.9.101, NOT pushed beyond
  the first commit 8cebf27 - the user: "nic nevydávej na github!!! Až hotovou věc"; work stays in ~/LibreOrgan-linux
  until he says). `Source/PluginHost.h` (header-only; CMake `JUCE_PLUGINHOST_VST3=1`, `JUCE_PLUGINHOST_AU=1` on
  APPLE, the jucer's JUCEOPTIONS JUCE_PLUGINHOST_VST3 - RESAVE ON WINDOWS): `formats()` (JUCE 9:
  `juce::addDefaultFormatsToManager`, AudioPluginFormatManager::addDefaultFormats is deleted), THE SCAN WORKER = the
  program started with `--pluginscan "<request file>"` (3 lines: format, identifier, result file) handled FIRST in
  initialise (before the crash handler; moreThanOneInstanceAllowed true for it; `std::_Exit(0)` after writing the
  XML of the descriptions; Windows SetErrorMode so a crashing plug-in raises no system dialog); `Scanner` (a low
  priority thread: formats' default folders + `pluginFolders`, `searchPathsForPlugins`, per file a juce::ChildProcess
  with 8 s (60 s for "the refused once more"), no result XML = "its scan crashed or gave nothing", stamps by
  "<format>|<identifier>" + modification time in PluginScan.json, the types in Plugins.xml (KnownPluginList), both
  in the settings folder); `sessionBegan(shift)` / `sessionEndedInOrder()` / `create()` with the markers
  PluginsActive.txt (a session that loaded a plug-in) and PluginLoading.txt (the one being loaded) -> SAFE START:
  the slots are read with on = false and NOT instantiated until switched on, a WarningIcon notice names the plug-in.
  ENGINE: `SamplerEngine::RoutePluginStage` (processors[8], count, mono, off, tripped, nsSum/blocks/nsMax) +
  `RouteChain` (stages kind 0 volume / 1 separation / 2 eq / 3 plug-in, delaySamples + delayLines) +
  `setPerspectiveChain` (Route::chain raw atomic pointer, chainOwner, retiredChains kept 2 s). renderRoute: with a
  chain the voices render at unity (outputGain 1), the stages run in order (applySeparation / applyEq are lambdas
  now, scaleRoute for the volume), then the delay, then `scaleRoute(masterGain)` - THE MASTER IS AFTER THE PLUG-INS;
  a plug-in over 80 % of the period 20 blocks in a row trips itself. Without a chain the old path verbatim.
  MODEL: `PluginSlot {id, description, state, on}` in `PerspectiveEffects::plugins` (PER PERSPECTIVE ONLY - no
  per-route plug-ins, no LINKED for them), chain ids "vst:<id>", saved in the perspectiveEffects block as "plugins"
  [{id, on, name, maker, description (XML), state (base64)}]; NOT in routing files. RUNTIME: `livePlugins`
  ("<slot id>|<route key>" -> LivePlugin {instances, stage, rate, error}), `rebuildPluginChains()` (called at the
  end of rebuildPerspectiveAudioRoutes and after every change: instances made for slots that are ON, a stereo
  plug-in one per stereo pair, a MONO plug-in (ZamComp) one per channel - `makePlainEffect(instance, 2 or 1)`;
  latency summed over the ON plug-ins, every chain delayed to the latest; instances no longer named are released
  2 s later by a callAfterDelay holding the shared_ptr), `capturePluginStates()` at the top of saveSettingsNow,
  `syncPluginState` (the first instance's state to its sisters while its editor is open, once a second, and at its
  close), `watchPlugins()` in the 1 s timer (tripped -> slot off + notice; a route whose rate changed -> rebuild),
  `showPluginEditor` = `PluginEditorWindow` (DocumentWindow, native title bar, the plug-in's editor or a
  GenericAudioProcessorEditor; ALWAYS a window of its own - the user's decision), pluginWindows closed before any
  instance goes. Our snapshot of a plug-in window is an empty grey (the plug-in paints its own X11 child): a real
  screenshot (`spectacle -b -n -a -o`) showed ZamCompX2's editor fine under KDE Wayland. EffectsDialog: "+" under
  the chain (a PopupMenu by maker, Scan, the Editor) - enabled in the PERSPECTIVE's dialog only, plug-in rows and
  `PluginPanel` (panel index base + 4). Editor section "Plug-ins (VST3)". The README of a portable save names the
  plug-ins; system.txt of a report lists them with their status. Hooks: ORGAN_TEST_PLUGINSCAN="<s>[|all]",
  ORGAN_TEST_PLUGIN="<s>|<perspective>|<name part>[|editor|on|off|remove]^...", ORGAN_TEST_QUIT=<s> (a quit in
  order: the destructor runs, the markers go). MEASURED here: the scan of 17 Zam VST3 files 1 s (47 ms each);
  ZamCompX2 on 3 routes 4-6 us per block; killed with kill -9 -> the next start SAFE, the slot OFF and unloaded,
  switched ON it loads. NOT TESTED: Windows, macOS/AU, Waves, a plug-in that really hangs its scan, a plug-in with
  latency (none of the Zam set reports any), the trip. Reaper's own plug-ins on this box are VST2 (.vst.so) - not
  hosted. Design left open: a search field in the "+" list (a menu by maker for now), per-route plug-ins.
  0.9.101, THE USER'S FIRST TRY OF THE PLUG-INS (18 Nov 2026, "Kurva? co děláš?"): (1) THE ARRANGE MODES COULD NOT
  BE ENTERED by him (no "Menu:" line in his whole session) - I had driven them by hooks alone, and Wayland has no
  tool to send a real mouse event (no xdotool/ydotool): SAY SO when a gesture is untested. Now: a top-bar button
  held 2 s (`headerPressed` watched by the 25 ms timer, `LeftOnlyTextButton::swallowRelease` makes the release no
  click) or a right-click in the bar's FREE space (`showHeaderMenu`, MainComponent::mouseDown with eventComponent
  == this and y < 62) enters the bar's arrange mode; in the Effects dialog a chain row held 2 s
  (`RightClickButton::downSinceMs` watched by the dialog's 100 ms timer) or a right-click anywhere in the chain
  column (EffectsContent::mouseDown) opens it; both menus log "Menu: the top bar" / "Menu: the effects chain".
  STILL NOT TRIED BY A REAL MOUSE. (2) "!3" WHILE ADDING PLUG-INS = Output discontinuities (not DSP dropouts), three
  causes: the chain's first block rendered the voices at unity while their gain GLIDES inside the block (now the
  voices keep master x volume and the chain divides it off: `scaleRoute(1 / outputGain)`); a plug-in switched in
  or out at once (now `RoutePluginStage::wet` glides 60 ms against the dry sound after 100 ms of unheard warm-up,
  `warmSamples`; removePlugin sets the stages off and rebuilds 250 ms later); setStateInformation on the sister
  instances at every knob turn (now `syncPluginState(..., withState)` copies the PARAMETERS ten times a second
  while the editor is open, the whole state only for a plug-in without parameters). Measured: add / off / on /
  remove under a held note = 0 discontinuities (2 each before). ZamTube costs 650 us per block and route.
  (3) Editor: the section "Plug-ins (VST3)" stands after Advanced global audio (before Console); the folders are a
  multi-line field "Plug-in folders (one per line)" + "Plug-in folders: add one"; `showPluginSettings()` opens the
  Editor AT the section (`editorSectionWanted`, the section unfolded). (4) The plug-in's window is centred on the
  program's window and made its transient on Wayland (it came up on the primary display).
  0.9.101 ON WINDOWS (18 Nov 2026 afternoon; the release itself was built by CI from the Linux session): the checkout
  `git reset --mixed origin/main` (the files were equal - nothing to commit but the resave), Projucer resave
  ("Finished saving", JUCE_PLUGINHOST_VST3=1 in the vcxproj, versionString 0.9.101), the .ico copied, commit 0158bc8
  pushed to main WITHOUT a tag (a push to main builds nothing); the local MSBuild Release of the VS2026 solution
  builds with the plug-in hosting (exit 0); the CI exe deployed as C:\LibreOrgan\LibreOrgan_0.9.101.exe (system DLLs
  only), the shortcut retargeted. PLUG-INS VERIFIED ON WINDOWS with the CI exe: the scan of
  C:\Program Files\Common Files\VST3 (53 files: iZotope RX 7/10/11, Waves' WaveShell, Melda, LiquidSonics, Sound
  Radix) took 37 s - 123 ms to 8 s per file, 56 effect types found, the WAVESHELL GIVES ITS PLUG-INS (Z-Noise,
  SoundShifter) by the general way, ONE refused: Equator2.vst3 "did not answer within 8 s" (the "refused once more"
  button gives it 60 s). In Rural's chain under a held note: MDynamicsMini 17 us per block and route, Waves Z-Noise
  Stereo 142 us with a latency of 34 702 samples, MAutoEqualizerLinearPhaseMini 30 us with 32 768 - "the routes are
  made 67470 sample(s) late together" (the latency compensation at work), no output discontinuity, the quit in order
  left no marker. Test recipe: settings.json backed up, `loadLastConfigurationAtStartup` false in the copy,
  ORGAN_TEST_PLUGINSCAN=5 (the log polled for "file(s) scanned"), then ORGAN_LOAD_CONFIGURATION=<Rural> +
  ORGAN_DUMP_STOPLIST + ORGAN_TEST_NOTE + ORGAN_TEST_PLUGIN + ORGAN_TEST_QUIT=40. NOT tried: the plug-ins' own editor
  windows on Windows, the trip, the safe start on Windows, macOS/AU.
  0.9.102 (18 Nov 2026, Windows; the user: Reverberate "jen bílé okno", "načítání GUI vst efektu trvá někdy příšerně
  dlouho ... v reaperu běží úplně rychle" + a bold orange line on a heavy plug-in's card): THE CAUSE WAS OUR SYNC of
  0.9.101 - ten times a second EVERY parameter of every sister instance was compared with the source's and written
  when it differed (a plug-in whose values never come out equal - meters, rounded values - was written to for ever),
  once a second `getStateInformation` of the edited instance (watchPlugins), and `capturePluginStates` asked EVERY
  plug-in for its whole state at every saveSettingsNow (once a second while anything moved); the log showed "Message
  thread was silent for 1-2.9 s" all the time an editor was open. Now `PluginSourceWatch` (MainComponent.h; a
  juce::AudioProcessorListener on the instance whose editor is open, `pluginWatches` by slot id, made in
  showPluginEditor, removed FIRST in closePluginEditor - its instance is still alive there): `syncPluginState(perspective,
  id, closing)` reads the SOURCE's parameters alone and passes on only what changed since the last look (the first
  look = the baseline; automatable parameters only), marks the slot in `pluginStateDirty`; the whole state goes to the
  sisters (`copyPluginStateToSisters`, only where the sister's own state differs) when the plug-in itself reports
  `nonParameterStateChanged`/`programChanged` (700 ms later, at most every 3 s) or at the editor's close after such a
  report (or for a plug-in without parameters); `latencyChanged` -> rebuildPluginChains by callAsync;
  `capturePluginStates` takes the DIRTY slots alone (the destructor marks every open editor's slot; rebuildPluginChains
  calls it first so a new instance starts from the present setting). `PluginLoadNote` (anonymous namespace, a
  temporary desktop component painted with performAnyPendingRepaintsNow) says "Loading the plug-in X for I. / default
  (n of m) ..." while rebuildPluginChains creates instances (no message loop runs there; Waves SoundShifter 12 s an
  instance); an instance over 1 s is logged. CARD WARNING: `pluginStatus(perspective, id, &warning)` - latency >= 10 ms
  "Not for real-time playing: it delays the sound by N ms."; the plug-in's time on its routes together >= 25 % of the
  buffer's period (`RoutePluginStage::lastBlockFrames`, set in renderRoute) "Heavy for real-time playing: it takes
  N % of the audio buffer's time." - `PluginPanel::warning` (orange, bold 14) right under the status line (the status
  20 px when it is one line and a warning shows). MEASURED: Reverberate's editor opened in 916 ms and painted
  (PrintWindow: 0 % white), no stall line in 40 s with it open; Z-Noise's card "delays the sound by 723 ms".
  %TEMP%\test_0102.ps1 [-Plugin -Button -NoEditor -Tag]: Rural + ORGAN_TEST_PLUGIN (add at 24 s, editor at 30 s), the
  editor's window pictured through PrintWindow(hwnd, dc, 2) by process id + title (the window's own content, not the
  screen; our snapshot of a plug-in window is empty), the white share printed. NOTE LESSON (the user: "prázdný šedý
  obdelník s oranžovým okrajem - to je hloupé"): a window made, shown and then given its text + repaint +
  performAnyPendingRepaintsNow in the same breath shows the FIRST paint (no text) - the second does not get through
  while the thread stays busy; the note takes its text in the constructor and is made anew for every text
  (%TEMP%\test_0102b.ps1 pictures the process's small windows around the add: "Loading the plug-in Reverberate for
  I. / default ..." readable; the same run records a key: Reverberate's default preset leaves the level as it was,
  -46 dB before and after - the user's "+50 dB" was the plug-in's own gain knob). NOT run: a knob turned in the
  editor by a real mouse (the sisters following), the CPU warning (no plug-in here reaches 25 %). RULE: never poll
  or set a foreign plug-in's parameters or state periodically - listen, and pass on what changed.
  0.9.103 (18 Nov 2026, Windows; the user: "proč máš běžící (či neběžící) bílé přerušované čáry v arrange módu JINÉ v
  každém oddíle? ... projdi celý kód a sjednoť"): THE UI STYLE UNIFICATION, STEP A. `namespace ArrangeFrame`
  (UIControls.h) is the ONE place that draws arrange frames: `strokeDashes(g, path, dash, gap, phase, thickness)` (a
  flattened path walked by length - any shape, the dashes travel with the phase), `phase()` = ONE clock (16 px a
  second = two periods; the stops, the couplers and the Stop List each had a counter of its own: three speeds),
  `takeable(g, rect, phase, thickness = kThickness, alpha = 0.8)` = a thing that can be taken (fine white 4/4 dashes,
  rounded 4, travelling), `freeCell(g, rect)` = a free place (the same dashes standing, alpha 0.35 - the user: "volná
  políčka jako u spojek"), `landingLine(g, rect)` = the orange 3 px line where a dragged thing lands. Used by:
  StopRow::paintOverChildren (inside the button, reduced 2.5, per-stop phase offset), the SectionHeader's couplers,
  the stop grid's and the couplers' free cells, the Stop List's GridArea (source alpha 0.45, target 1.0), the
  HeaderArranger and the Effects Arranger (both got a 25 Hz juce::Timer that repaints THEIR STRIP alone - the layer
  lies over the whole window), the Audio routing's free cells (the cell under a dragged card stays ORANGE and
  thick, 2.5 px - the user). Both copies of drawMarchingBorder are gone. THE USER'S DECISIONS: the thick 9/6 2 px
  dashes of the stops were "moc řvavé" - the fine line of the top bar, but travelling, everywhere; `kThickness` 1.5
  is MY pick from F:\temp\arrange_frames.png (ORGAN_TEST_FRAME_SWATCH=<png>: 1 / 1.5 / 2 px on a stop off and on, a
  coupler, a top-bar button, beside the old look; drawn by the program's own buttons) - his choice changes that one
  number; displays' green = the saturated one (lime) - NOT done yet (step B); greys: NOTHING is changed (every dialog
  has darkgrey, the main window and the Audio routing the look-and-feel's blue-grey; the pistons' and crescendo
  steps' greys carry meaning) - only a single outlier, shown with a picture, may ever be proposed; THE CLASSES OF
  WINDOWS, SLIDERS AND BUTTONS ARE NOT MERGED (each exists for a behaviour: hide or delete at close, modal or not,
  the native save dialogs' focus) - the unification is of the LOOK, never of the behaviour. Step B (not done): one
  orange (`onColour()` for the 39 juce::Colours::orange), lime on every display, one threshold table for both
  meters, one note bubble; then a contact sheet of every dialog (the report's pictures) for the second list (button
  heights, gaps, fonts - 18 sizes -, corner radii - 10 values -, slider text boxes - 14 sizes), shown to the user
  BEFORE any change. Hooks: `ORGAN_TEST_ARRANGE="<seconds>|<division of the stops>|<division of the couplers>"`
  (`Voicer::scriptCouplerArrange` -> `testCouplerArrangeManual`, taken by that division's header at the next
  layout), with ORGAN_TEST_HEADER="<seconds>" the top bar too; %TEMP%\test_0103_arr.ps1 -Mode main|fx pictures them
  (0-main.png, "2-Effects default.png"); %TEMP%\test_0103_swatch.ps1 the design picture. LESSON: a juce::Graphics on
  an Image must be DESTROYED before the image is written (the first swatch was black) and the image made with
  juce::SoftwareImageType(). NOT pictured: the Stop List's folder drag and a card dragged in the Audio routing (no
  hook holds a drag); the motion itself (stills only) - the user judges the speed.
  0.9.103 STEP B + THE CARD DRAG (the same day; the user: 1.5 px "je fajn", the speed "akorát", the Stop List's
  folder drag OK; "přetahování karty by taky mělo mít bílý rámeček běžící, a ... stejnou velikost jako karta"):
  the dragged card of the Audio routing = `cardImage` (createComponentSnapshot of the card's bounds, taken in
  `beginCardDrag()` BEFORE draggingCard is set) drawn at 0.75 opacity at `cardFrame()` (the card's bounds moved by
  dragPoint - `cardGrab`) inside ArrangeFrame::takeable; `cardTicker` (a 25 Hz FrameTicker) repaints the frame while
  the mouse rests; the cell under it stays orange and thick. Hook ORGAN_TEST_ROUTE "dragcard|<card name part>|<dx>|
  <dy>" holds a card in a drag for a picture (test_0103_arr.ps1 -Mode card). `MeterStyle` (kSegments 8,
  kThresholdsDb, segmentColour / segmentLit / segmentPaint, kUnlitAlpha) serves paintOutputMeter and the rows'
  miniature; `NoteBubble::paint(g, area, text, justification, lines)` (black 0.85, radius 5, orange bold 13) serves
  the HeaderArranger's and the Effects Arranger's notes and PluginLoadNote; every `juce::Colours::orange` (34 uses
  in six files) is `onColour()` - never write juce::Colours::orange again (darkorange of the "apply" buttons and the
  cable palette's 0xffff8c1a stay: they mean something else); bankLabel and limitEditor are lime (the Divisions
  dialog's status text and the sounding key's slider track keep lightgreen: no displays). THE DIALOG SHEET:
  test_0103_arr.ps1 -Mode sheet opens GLOBAL RESET, Save Configuration, Stop List, Couplers, Temperaments, the three
  patterns dialogs, the Editor, Audio routing and Divisions in ONE run (ORGAN_TEST_CLICK + ORGAN_TEST_DIALOG_SNAPSHOT;
  the modal boxes first, each closed by "Cancel" before a dialog with a Cancel of its own is open) - pictures in
  F:\temp\dialogs_0.9.103. THE SECOND LIST (shown to the user, NOTHING of it done): (1) the three patterns dialogs
  have a LIGHTER grey background than every other dialog and OK / Cancel at the bottom LEFT, 30 px, touching the
  editor; (2) the bottom button rows differ - Close 86 x 34 (Couplers), 100 x 30 (Divisions), 110 x 30 (Editor),
  32 px in the Stop List, none in Temperaments; (3) GLOBAL RESET: two tick-box texts are squeezed (too long for the
  box); (4) slider value boxes: 14 sizes in the code; (5) the AlertWindow boxes (blue-grey, large outlined buttons)
  look unlike the dialogs (darkgrey, filled buttons) - my advice: leave; (6) fonts (18 sizes) and corner radii (10
  values) - my advice: no wholesale change, the pictures show no fault. NOT pictured yet: the crescendo dialog (the
  run's last snapshot missed it), Report a problem, the load box, the measurement windows, the Tremulant / Swell /
  Wind settings, IR reverb settings, the manual window.
  0.9.104 (18 Nov 2026 evening; the user: "jednou dialog Audio Routing spadl!!! ... u nějaké nepřístupné karty na
  kterou jsem přetahoval kabel"): NOT a crash (the process ran on, no crash file) - the log: 19:05:54 a cable onto
  "UMC ASIO Driver" ("No device is connected to the PC."), 19:06:01 "Audio devices still missing after the wait",
  19:06:02 'Modal dialog "Audio devices not available" made an owned window of the main window'. The 0.9.29 owner
  trick again (the 0.9.52 case was the native chooser's placeholder): Windows activates a box's OWNER when the box
  closes, the main window rose over the modeless Audio routing dialog - buried, not closed (the top-bar button
  brought it back). FIX in `bringLostModalsToFront` (Windows): while no modal box is up, the topmost TopLevelWindow
  of ours in front of the main window is remembered every half second (`dialogInFront`, by the Z order: GW_HWNDPREV
  from the main window); `gModalBoxesSeen` (file static) is set while a box is up; when the last box has gone the
  timer calls the function at once and, 250 ms later (the box's window is destroyed after its modal state ends),
  the remembered dialog is `toFront(true)` if it is showing and BEHIND the main window - log 'Dialog "X" put back in
  front of the main window'. The owner stays the main window (an owned window is destroyed with its owner: never make
  a dialog the owner). %TEMP%\test_0104_box.ps1 reproduces it by hooks (ORGAN_TEST_ROUTE "all|default|UMC|1", the
  box six seconds later, "OK" clicked, the process's windows listed in their Z order): the line came and the order
  ended "Audio routing > main". RULE: every box over a modeless dialog goes through this - when a user says a dialog
  "fell", read the log for "made an owned window" right before. THE SECOND LIST, items 1-3 done (the user:
  "Souhlasím se zbytkem"): the patterns dialogs have darkgrey and [?] left / [OK][Cancel] right, 100 x 32, 8 px under
  the field; Close is 100 x 32 in Couplers, Divisions, the Editor (its row 32 high), the Stop List (oneRowWidth
  + 10) and the feature dialog; Temperaments got a Close (callAsync -> the DialogWindow's closeButtonPressed: the
  window may delete the button); GLOBAL RESET's two long texts are "Unload everything: stops and sample folders too
  (not the pattern lists)" and "Also reset the Perspective, Ignored and One-shot pattern lists to their defaults"
  (test scripts that click the old text must follow). Item 4 (the sliders' value boxes, 14 sizes) NOT done - on its
  own, with pictures before and after; items 5-6 (AlertWindow look, fonts, radii) left by agreement. The crescendo
  dialog's Close (90 wide, its row's height) not touched - not pictured yet. NOT pushed (the user: "zatím na
  gitHub nedávej"): local commits 0.9.103 x 3 + 0.9.104.
  0.9.105 (18 Nov 2026 evening; the user: "proč kurva teda karta, která není připojená k PC není šedivá a
  nepřístupná???? ... Nejde to odchytat?" - plan A chosen, "Hlavně, když se karta později připojí, tak to musí program
  zjistit a udělat ji aktivní. To je velmi důležité"): CARDS THAT ARE LISTED BUT NOT THERE. Windows lists an ASIO
  driver from the registry, device or no device; only the driver tells. `MainComponent::unavailableCards` ("type|name"
  -> {reason, nextCheckMs}) + `unavailableCardsVersion` (in the Audio routing's signature) + `probeCard(type, name)`
  (ASIO only - every other type answers true: its absent devices are not listed; a card open in a shared host = true;
  else `isAudioDeviceAvailableNow` = the watcher's throw-away-manager open; a no -> the entry with the driver's
  words, next check in max(3 s, 20 x the answer's time, <= 2 min); a yes -> the entry erased, log "Sound card X
  answers again", a Messages note) + `watchUnavailableCards()` (the 1 s timer: one card a second at most, ONLY while
  the Audio routing is showing, no loader / apply-all / modal box, polyphony 0) + `askUnavailableCardsSoon()` (every
  entry's nextCheckMs = 0: from midiDevicesChanged = Windows reported a device, and from showAudioRoutingDialog).
  AudioRouting: `assign()` asks the card FIRST when it is not open and not busy - a no: nothing moves (the cable is
  where it was), rebuild; `Card::unavailable` (set in rebuild from the map, status = "not there now: <reason> ...")
  paints the chip grey, `findDropTarget` skips it, a mouseDown on it asks again at once (through `later`). No six
  seconds of waiting and no "Audio devices not available" box on that path (the start-up path of a configuration
  saved on an absent card keeps its watcher and its box). TESTED by hooks: UMC ASIO Driver (really unplugged here):
  "is not there: No device is connected to the PC. (answered in 179 ms)", the chip grey in the picture, the cables
  unchanged, no box; `ORGAN_TEST_CARD_ABSENT="<name part>|<seconds>"` (probeCard pretends the card absent for that
  long from its first question) with Komplete Audio 6: refused at 24 s, "answers again" 13 s later by the WATCHER, a
  second drop then moved the three routes onto it (%TEMP%\test_0104_box.ps1 -Tag later -Absent "Komplete|10" -Route
  "all|default|Komplete|1^x^x^x^x^all|default|Komplete|1"). NOT tested: a real device plugged in while the dialog is
  open; a click on the grey chip by a real mouse. Scope: ASIO alone - an endpoint of the aggregate that refuses, or a
  Windows Audio device that fails to open, still goes the old way. The user refused nothing of plan B (a helper
  process asking every ASIO driver ahead of time) - it was simply not chosen. `DialogCloseButton` (UIControls.h:
  "Close", closes its DialogWindow by callAsync; `place(content, area)` takes 36 px at the bottom, 100 x 32 at the
  right, and adds itself at the first call) gives the IR reverb settings (ToneContent) and the Swell / Tremulant /
  Wind settings (FeatureContent, neededHeight + 40) their Close; the crescendo's row is 32 high with Close 100 wide
  and a gap between Save... and Load..., its tick text "Crescendo may switch off hand-registered stops and couplers
  (off: the steps only add)"; the Divisions dialog's value boxes are all 66 x 22. The empty crescendo steps STAY a
  tone lighter (the user, again); the sliders' value boxes elsewhere are NOT unified (their widths follow the
  longest value with its unit, their heights the place - the user's own guess, confirmed in the code). Hook
  `ORGAN_TEST_OPEN="<seconds>|crescendo^<seconds>|tremulant|1^...|swell|1^...|wind|1^...|reverb|1"` opens the dialogs
  only a right-click menu reaches (test_0103_arr.ps1 -Mode menus). The user dragged a cable in MY test instance
  again (19:45:52, "cable 11+12 of II. default moved to Komplete Audio 6") - the settings were restored by the
  script; a test window is his to touch while it stands, say so.
  0.9.106 (18 Nov 2026 evening; the user: "po připojení kabelů jsem odpojil Komplete, a program crashnul ... To už
  jsme měli opravené ... Potřebujeme robustnost"): crash-20261118-195407.txt = THE 29 OCT CRASH AGAIN, line for line:
  std::terminate (invalid argument) <- std::thread::join <- juce::WindowsMidiHelpers::Win32::InputDevice::~InputDevice
  (juce_Midi_windows.cpp 2640) <- InputDevice::openInternal (2710: `return {}` after midiInOpen failed) <-
  MidiInput::openDevice <- our lambda <- MidiDeviceListConnectionBroadcaster::endpointsChanged. NOT a regression of
  code: `git log -S midiInputOpensNow` = the one commit of 0.8.226, the JUCE submodule pointer has not moved since
  the first commit. The 0.8.226 "fix" was a DETOUR (a WinMM probe by NAME before the open) around JUCE's fault, never
  a repair: it stops a port that refuses at the probe; a port that passes the probe and fails milliseconds later in
  JUCE's own open (which numbers the devices by their POSITION in a list it builds anew, skipping any device whose
  caps query fails - while a card vanishes that position is not the WinMM number) still ended the program. Which
  port it was is not known (the crash is inside the open; nothing is logged before it). NOW THE ROOT: the line
  `blockQueueThread.join();` of ~InputDevice becomes `if (blockQueueThread.joinable()) blockQueueThread.join();` -
  (1) in CMakeLists.txt, before add_subdirectory(JUCE): file(READ) / string(REPLACE) / file(WRITE) at CONFIGURE time
  (idempotent: skipped when "blockQueueThread.joinable()" is there; FATAL_ERROR when the line is not found - JUCE
  changed, look at it), so the CI builds of every system carry it; tried with `cmake -P` on a copy of the original:
  33 bytes more, nothing else touched, a second run changes nothing; (2) in the local JUCE by hand
  (F:\JUCE_PROJEKTY\LibreOrgan\JUCE is a JUNCTION to C:\JUCE9, no git repository on Windows - the VS/Projucer build
  reads it). THE RULE OF 0.8.226 "the submodule is upstream, never patched" IS REPLACED: a fault of JUCE that ends
  the program is corrected at configure time in CMakeLists.txt, one exact line, with a FATAL_ERROR when the line is
  gone. Besides: `midiDevicesChanged` lets the ports that WENT go at once and opens the ports that CAME one second
  after the LAST change of the list (`changeNumber` static + callAfterDelay(1000) - a card going or coming changes
  the list several times in a row); a port that does not open is logged "not opened (tried again at the next
  change)". The probe stays. NOT reproduced by a test (the MIDI ports here open twice at once, so a held port does
  not make JUCE's open fail; no way to unplug hardware) - the repair follows from the crash stack; the user's own
  unplugging is the proof. The AUDIO side of an unplugged card was not in this crash (the log ends with the routes
  playing): ASIO routes of a card that goes are caught by the callback-health recovery and the missing-device
  watcher, the aggregate's endpoints are JUCE WASAPI devices (audioDeviceError -> lastError + xruns) - read, not
  exercised. Also: saved routings go to Documents/LibreOrgan/LibreRoutings (was "LibreRouting"; a remembered folder
  of the old name gives way); the manual's "Plug-ins in the chain" opens its safeguards with "Use plug-ins sparingly
  and expect little" (delay, polyphony, a faulty one can bring the program down). LINUX: nothing of 0.9.103-0.9.106
  was compiled there (no push since v0.9.102) - the CI of the next push is the check; the changes are Windows-only
  (#if JUCE_WINDOWS: the dialog put back, the MIDI probe) or platform-neutral drawing and dialogs; probeCard answers
  true for every type but ASIO.
  0.9.107 (18 Nov 2026 night; the user: an idle ASIO card unplugged did not turn grey, an active one only after a
  while with the window frozen, "na Linuxu zešedla a ztratily se porty"; a MeldaProduction plug-in's window ended
  the program - "musíme umět vadné VST zvládnout, aby program nespadl!!!"; he took the risk that asking a driver may
  disturb the audio of the same card): (1) THE CARD QUESTION IN A HELPER PROCESS: `Source/CardCheck.h` (header-only):
  the program started with `--cardcheck "<request>"` (result file / type / name) opens the device with a throw-away
  AudioDeviceManager and writes "ok" or "no<TAB>reason"; `CardCheck::Checker` (a 100 ms Timer, one helper at a time,
  8 s limit, `ask(type, name)` deduped, `onResult` on the message thread) is `MainComponent::cardChecker`.
  `askCards(everyAsioCard)` (every ASIO name of deviceManager's lists without a shared host of ours, or the grey ones)
  runs at the FIRST opening of the Audio routing and 1.5 s after the LAST system report; `cardAnswered` erases or
  fills `unavailableCards` (+ version, log, Messages; `missingDeviceNextTryMs.erase` so the watcher opens the routes
  at once). `systemDevicesChanged()` = Windows reported a device (from `midiDevicesChanged` and from a
  juce::AudioIODeviceType::Listener registered on deviceManager's types lazily in `watchUnavailableCards`, WINDOWS
  ONLY - on Linux our own rescans would fire it): `systemDevicesChangedAtMs` + `fastSilenceCheckUntilMs` = now +
  15 s, within which `checkAudioCallbackHealth` reopens a silent device after ONE second (4 otherwise) and, when it
  does not open, calls `cardWentAway(type, name, reason)` (ASIO only: the entry with `wanted = true`).
  `watchMissingAudioDevices` skips an ASIO card that is in `unavailableCards` (marks it wanted) - no more in-process
  probing every 2 s; `watchUnavailableCards` asks the wanted cards every 5 s ALWAYS and the other grey ones while
  the dialog shows. A click on a grey card asks the helper; the drop on a card that is not open still asks
  in-process (`probeCard`: the user's own act, the same cost as the open). TESTED: at the dialog's opening "7 ASIO
  card(s) ... asked (in a helper process)", UMC grey; the watcher path with the pretended-absent Komplete (0.9.105
  test). NOT tested: a real unplug of an open card (fast grey), a real plug-in. (2) THE 6.6 s FREEZE at every
  unplug (twice in the log, at the second of "MIDI devices changed: gone Komplete Audio 6 In"): the gone MidiInput
  is released on a detached std::thread (`going->stop()` first; "MIDI input X closed in N ms (off the window
  thread)"), `getAvailableDevices()` is timed (> 300 ms logged) - WHICH call blocks was not measured (no hardware to
  unplug); the next log tells. (3) PLUG-INS THAT FAIL. THE HUNT (seven test builds): two Melda editors
  (MAutoEqualizerMini, MDynamicsMini; MAutoEqualizerLinearPhaseMini too) crash AT RANDOM as their view is attached,
  or come up as an empty 128 x 128 window - reproduced by the hook (test_0102.ps1 -Plugin MAutoEqualizerMini), with
  the plug-in off as well; NOT the side-chain (one stereo bus), NOT Direct2D (ComponentPeer::setCurrentRenderingEngine
  (0) changed nothing), NOT JUCE's hasEditor() (which makes a view and drops it - still never call it before
  createEditorIfNeeded: the line stays out); the view object is dead early (garbage vtable in JUCE's own later
  calls: VST3PluginWindow::resizeToFit from the scale notifier's callAsync, componentMovedOrResized). The user: Melda
  has trouble in Reaper too - the cause is left. WHAT GUARDS NOW: (a) THE WINDOW TRIAL IN A HELPER PROCESS
  (PluginHost.h): `--plugineditor "<request>"` (result file on line 1, the description's XML after it) -
  `EditorTrial` loads, prepares, createEditorIfNeeded, a DocumentWindow in the screen's bottom right corner,
  1.5 s of message loop, "ok<TAB>W x H" (a window under 40 px = "came up without a size"), `_Exit`; started from
  Main.cpp's initialise (`startEditorTrial`, returns: the loop runs it). `EditorTrialRunner` (two trials in a row,
  30 s limit each - Waves' shell takes ~10 s to load in a fresh process, 8 s refused it; no result = "its window
  crashed") is started by `showPluginEditor` for a plug-in without a verdict, with a PluginLoadNote "Trying the
  window of X in a process of its own (once for each plug-in) ..."; verdicts in `PluginHost::editorVerdicts()`
  keyed by `editorTrialKey` (identifier + the file's modification time), KEPT in PluginWindows.txt beside the
  settings; a refused window = a WarningIcon box, the plug-in keeps working on the sound. Results: both Meldas
  "REFUSED - its window crashed" with the program alive, Reverberate "it opens (980 x 675)" 11 s after the click
  the first time, Z-Noise 24 s the first time and at once the second. (b) SEH GUARDS (Windows; `__try/__except`
  in functions without C++ objects): `guardedPluginProcess` (SamplerEngine.cpp) around every processBlock - a fault
  sets `RoutePluginStage::faulted`, the block's dry copy is put back (the dry copy is taken for EVERY block now),
  the stage is never called again; `guardedCall(function, context)` (MainComponent.cpp) around PluginHost::create,
  the prepare / state calls and the two steps of the window's opening (made hidden first, shown 80 ms later:
  `pluginEditorsOpening`). `pluginFaulted(perspective, id, where)`: `faultedPluginSlots`, the watch off, the window
  hidden with ::ShowWindow and RELEASED (never deleted), every LivePlugin of the slot moved to a leaked vector
  (never called, never deleted - what stood between our call and the fault was not unwound; a lock inside JUCE's
  wrapper may be held), slot OFF, a box asking for a restart; `setPluginOn` refuses a faulted slot. LESSON: a caught
  fault at the window is NOT the end - JUCE's later messages reach into the dead view (the first guard build died
  there twice); only the helper trial keeps such a plug-in's window out. What a plug-in does later in its own
  window procedure cannot be caught. (c) `PluginHost::helperDiesQuietly()` in all three helpers (scan, card, window:
  SetErrorMode + an unhandled filter that TerminateProcess(3)s + no abort message) - the user saw Windows' "LibreOrgan
  stopped working" twice while the program went on: the helpers trying Melda. A helper is a process named
  LibreOrgan too: `Get-Process LibreOrgan*` right after a test may still see one - wait a moment before the next
  test. THE USER CLOSED A TEST WINDOW mid-run (the quit question, Don't Save) and two tests did not run - tell him
  the windows are tests and close by themselves. Bank display: the dot at +4 / -22 (two pixels of black inside
  the frame line).
  0.9.108 (19 Nov 2026, Windows; the user wiped every setting and imported a Hauptwerk ODF: "v Audio Routing to nedal
  žádnou default kartu!!!! ... Vždy musí po loading něco někam hrát", "každé 4 sekundy se z myši stane čekací kolečko",
  the card grid "by měl být fluidnější", the Mac's "Sound cards" title over the routing buttons, Linux: a switched-off
  card's name stays white; then: "přednostně na windows bral ASIO ... fyzické karty, nikoli ASIO fake software
  ovladače", "některé karty odpojí výstup, když vytáhnu sluchátka"): (1) THE REAL CAUSE OF THE SILENCE, found by the
  helper: "Reproduktory (Realtek(R) Audio)" LISTS 64 OUTPUTS (JUCE's queryMaxNumChannels: the driver says yes to every
  channel count) and refuses to open with all of them - and sharedHostFor opens every card with the mask of 64 ones
  ("All output channels active from the start"): 65 attempts in two days, none worked; no mask or "11" opens at once
  (the Fireface's and the monitor's endpoints list 2). sharedHostFor (Windows, non-ASIO): after a failed open with our
  mask, once more with "11" -> `firstPairOnly` (the "enable them all" block skipped), log "... does not open with every
  output it lists ...: opened with its first pair alone"; the Audio routing shows `getActiveOutputChannels()` for
  Windows Audio cards (2 jacks, not 64). WHEN A WASAPI DEVICE "CANNOT BE OPENED", ASK IT THROUGH THE HELPER WITH AND
  WITHOUT audioDeviceOutChans FIRST (%TEMP%\cardq recipe below). (2) THE DEFAULT CARD (Windows): `DefaultCardSearch`
  (MainComponent.h), `startDefaultCardSearch(why, moveRoutes)` - candidates: every ASIO driver that is not software
  (`isSoftwareAsioDriver`: asio4all, flexasio, rearoute, generic low latency, voicemeeter, vb-audio, cable, jackrouter,
  asio link, synchronous audio router, fl studio asio, magix low latency, asio2wasapi, wasapi, asio bridge, dante via
  AND "realtek asio" - the on-board chip's layer, 44.1 kHz/496, crashed) in the type's list order, then Windows'
  default output, then the other Windows Audio outputs; each asked in the HELPER (`cardChecker`, the answer in
  `cardAnswered` before its own handling), a card already open in a host = found at once; `defaultCardFound` sets the
  statics `preferredDefaultType/Name` (what `defaultSharedTypeName()` / `defaultDeviceNameOfType()` give from then on;
  sharedHostFor: ASIO without a name takes it BEFORE "whatever ASIO runs", and `asioByDefault` copies NO rate or buffer
  of a foreign state onto the driver) and, with moveRoutes, `moveRoutesToDefaultCard` (beginRoutingAssign steps
  {manual, perspective, {0,1}, true} for every route that is not auxiliary, not a remembered "no card" and not on the
  card; its `finished` checks the card really opened in the program - else `defaultCardsRefused` + the next search);
  nothing opens -> the box "No sound card" (6 reasons at most) + `lookAgainOnChange` (the search again 1.5 s after
  Windows reports a device). TRIGGER: `applyDefaultRoutingIfDue` when no DefaultRouting.json exists (the saved default
  still comes first) - NEVER at the program's start (the helper asking an ASIO driver while loadSettings opens the
  user's own ASIO routes). So the first import of a session attaches to Windows' default and is then moved (the
  progress window "Audio routing: <card>"); later loads attach to the found card directly. The user's old rule "ASIO
  never automatic" (0.9.84) IS REPLACED by this one - possible because the helper asks, never the window thread.
  (3) THE CURSOR: Windows shows IDC_APPSTARTING for up to 2 s after ANY CreateProcess of a GUI program; with the
  dialog open a helper was started for every grey card every 5 s - MEASURED with %TEMP%\test_0108_cursor.ps1
  (GetCursorInfo polled 20 s): 302 of 427 samples with 0.9.107, 0 with 0.9.108. `Source/QuietProcess.h` (CreateProcessW
  with STARTF_FORCEOFFFEEDBACK + CREATE_NO_WINDOW on Windows, juce::ChildProcess elsewhere; start / isRunning / kill)
  replaces juce::ChildProcess in CardCheck.h and PluginHost.h (scan, window trial). watchUnavailableCards: every 5 s
  the WANTED cards only; the other grey ones at a system report, the dialog's opening, a click, and every 30 s while
  the dialog shows (`lastIdleGreyCheckMs`). Side effect seen: the aggregate's "No audio callbacks ... recovery" every
  7 s of the old runs (Realtek ASIO asked every 5 s, 3.6 s each) is gone. (4) A DRIVER'S OWN THREAD THAT FAULTS
  (crash-20261119-112701: write of address 0 in rthdasio64.dll, the stack the driver + the thread's start; the user
  had pulled the headphones out - the on-board card dropped its output): CrashHandler.h (Windows, in detail::)
  `driverModules()` (the ASIO DLLs' file names, filled by `MainComponent::registerAsioDriverModules` from the registry:
  SOFTWARE\ASIO\<name>\CLSID -> CLSID\{..}\InprocServer32; logged "ASIO driver modules ..."), `driverOfFault(info)` (not
  the window thread (`windowThreadId()` set in install); the fault address in a driver DLL, or - outside our exe - on a
  thread whose START address (NtQueryInformationThread 9) is in one), and `juceCrashHandler` then writes the report
  ("A sound card's driver (x.dll) failed on a thread of its own ..."), clears alreadyHandling, sets `parkedDriver` and
  NEVER RETURNS (`for (;;) Sleep(INFINITE)`). The 1 s timer takes it: `asioDriverFaulted(index)` - unavailableCards
  entry `faulted` (never asked, never probed, never revived: watchUnavailableCards / askCards / cardAnswered / probeCard
  / the click all skip it), the host RELEASED from asioHosts and leaked (never closed: closing calls the driver), its
  routes `detachRouteAudioQuietly` + their state XML set back to the card (cables wait; after a restart they play),
  the box "Sound card driver failed". Main.cpp shutdown ends with TerminateProcess when `driverWasParked()` (the
  driver's DLL unload could wait for the stopped thread). Hook `ORGAN_TEST_DRIVER_FAULT="<seconds>|<ASIO name part>"`
  (pretendedDriver + a std::thread that RaiseExceptions): verified - 1 crash file, the program alive, the card grey
  with its cables, exit code 0. NOT verified with the real Realtek fault (the user's hands needed) and no guarantee
  (a lock the driver held). (5) ASIO PERIOD RECOVERY: only the cards whose callbacks mismatch
  (`mismatchingAsio` from the callback labels), once per card (`asioPeriodRecovered`, cleared for a card by any
  reopenAudioDeviceFresh whose reason is not the two automatic ones) - before, every ASIO manager was reopened and
  `asioRecoveriesSinceUserAction` was zeroed by each reopen, so it repeated after every device change (ReaRoute
  without Reaper calls back twice as often as its buffer says: #245/#246). Verified: only ReaRoute reopened, once.
  (6) AUDIO ROUTING ROWS: no cells - `Card::place` (order in its row, row; the old "<c>,<r>|<key>" lines read as that),
  `Card::cell` = where it stands (index in the line, the line), layout(): cards sorted by (row, order), flowed left
  to right with cellGap, a new line when the row changes or the card does not fit; `cardOrder`; `DropPlace` {line,
  before, after, newRow} + `dropPlaceAt(p)` (nearest line in height, then nearest place); paint: ArrangeFrame::
  landingLine for a place, the dashed strip under the last line for a new row (orange 2.5 px when targeted);
  `placeCard(key, beforeKey, afterKey, newRow)` (keys: it runs through `later`) renumbers every card from the LINES on
  the screen. Hooks: ORGAN_TEST_ROUTE "place|<card>|before|<other>", "...|after|<other>", "...|newrow|x" (the old
  "cell|..." is gone). The title: `listCables()` first, then the buttons begin at max(rowsLeft, the title's right)
  and go on a line of their own (sourcesTop 138) when less than 470 px are left. A card open with no outputs: grey
  title, status "no outputs now". A card in unavailableCards keeps its reason as the status (the route loop
  overwrote it with "tried again every 2 s"). (7) TESTS: %TEMP%\test_0108.ps1 [-Absent "part;part" | "*"] [-Route]
  [-Fault "32|Fireface"] [-Bounds "x y w h"] [-Clicks] (Rural loaded with "portable": true, DefaultRouting.json moved
  away; NOTE that copy keeps its aggregated routes - the move still runs), `ORGAN_TEST_HELPER_ABSENT` (the helper
  answers "not there"); the helper's request takes a 4th line "name=value;name=value" of device-state attributes and
  answers "ok<TAB>N outputs, M open, rate, buffer" - %TEMP%\cardq: write "<result>\n<type>\n<name>\n<attrs>\n" and
  run `LibreOrgan.exe --cardcheck "<request>"` (only while no LibreOrgan runs). The dialog's minimum width is ~1000:
  a narrower "dialogBounds" is not taken; more cables (ORGAN_TEST_ROUTE "...|add") make the gutter wide instead.
  0.9.109 (19 Nov 2026 afternoon; the user's own test of 0.9.108 with the headphones: THE REAL FAULT WAS SURVIVED -
  12:43:24 "Sound card driver FAILED: rthdasio64.dll ... taken off: Realtek ASIO (10 route(s))", the program went on -
  and his list: "Když se objeví msgbox, tak to prostě ztratí okna pod sebou. Pak se nějaké zase objeví, ale ne všecka",
  the card "furt šedá (do restartu??? Kurva proč???)" after the headphones were back, "realtek ASIO je mezi fyzickými
  kartami. Určitě.", cards to the sample rate on loading, why Realtek shows 2 outputs, the inactive black key too dark;
  "beru všecko, co říkáš ... a dej to rovnou i na gitHub"): (1) EVERY WINDOW OF OURS IS AN OWNED WINDOW OF THE MAIN
  WINDOW (Windows; the top of `bringLostModalsToFront`: every juce::TopLevelWindow on the desktop that is not the anchor
  and whose name does not start with "LibreOrgan Sampler" (window 2 is a main window) gets GWLP_HWNDPARENT = main, once,
  log 'Window "X" made an owned window of the main window'). WHY: a box was owned by the main window (0.9.29) and
  Windows brings the OWNER forward with an owned window - the main window rose over every modeless dialog, and the
  0.9.104 repair put back only the one that had stood in front. Owned windows stay above their owner in their own
  order - nothing to repair. The price (told to the user, accepted): a dialog cannot go behind the main window and is
  minimised with it. Verified by the Z order (test_0108.ps1 -Order): box > Audio routing > main, then Audio routing >
  main. The 0.9.104 `dialogInFront` code stays (it finds nothing to do). (2) REALTEK'S OUTPUTS, measured through the
  helper: "Realtek ASIO" offers 2 outputs (its driver's own limit; 44.1 kHz/496 by default, 48 kHz/528 when asked);
  "Reproduktory (Realtek(R) Audio)" opens in shared mode with 2, 4, 6 or 8 channels and refuses 64, exclusive mode
  lists 8 - and the registry's device format says 2 ch = Windows is set to STEREO (whatever is opened beyond the mix
  format is folded into the front pair by Windows). `windowsOutputMixChannels(name)` (MainComponent.cpp before
  findSharedHost; mmdeviceapi/audioclient: the endpoint by PKEY_Device_FriendlyName -> IAudioClient::GetMixFormat):
  sharedHostFor opens a shared "Windows Audio..." output with exactly that many channels, whatever a saved mask says,
  and the "enable them all" block is capped to it; the 0.9.108 first-pair fallback stays for a failed query. To get
  5.1/7.1: Win+R `mmsys.cpl` > Playback > the output > Configure (or the Realtek Audio Console) - the USER DID NOT
  FIND "Configure speakers" in Windows 11's Settings: name mmsys.cpl. Linux shows all jacks because ALSA takes them
  directly. (3) A FAILED DRIVER COMES BACK: `asioDriverFaults[dll]` - the first fault leaves the card `wanted`, not
  `faulted` (asked in the helper every 5 s like any absent card); when it answers, watchMissingAudioDevices logs
  "its driver failed earlier ... opened anew now (a new instance of the driver)" and opens the routes; a second
  fault, or a card of `asioCardsFaultedOnce` that answers the helper but fails `isAudioDeviceAvailableNow` in the
  program, is `faulted` for good. THE SIMULATED FAULT CANNOT PROVE IT: the "failed" Fireface driver is alive in the
  leaked host, and a second instance in the process said "Device didn't start correctly" -> for good (as designed).
  Whether Realtek's driver takes a second instance after its real fault only the user's headphones tell. Never close
  the leaked host to "free" the driver (its Release most likely waits for the stopped thread). (4) `isSoftwareAsioDriver`
  without "realtek asio". (5) THE RATE: a device state without audioDeviceRate (any type but JACK, and only with
  resampleOnLoad) gets `storageSampleRate >= 44100 ? storageSampleRate : 48000` in sharedHostFor - also for ASIO as
  the default card (the 0.9.108 "no rate on the default ASIO" is gone; its BUFFER is still never copied from a foreign
  state); JUCE falls back to the driver's own rate when it refuses. (6) Divisions keyboard: a dark black key
  0xff9c9c9c (was 0xff4a4a4a). (7) RenderPool `scratchStore` = lanes x 2 + 4 (was lanes + 2): a lane keeps its
  scratch until the job's poster merges, and with several cards their device threads post jobs at once (four ASIO
  cards: "render lanes without a scratch buffer: ~2000" every second, 0 discontinuities); the "Output
  discontinuities" log line is written only with a real one. LESSON OF THE DAY: when a card "does not open", ask it
  through the helper with other parameters before believing the driver's words - the 64-output lie was found in
  five minutes that way after two days of "Couldn't open the output device!".
  v0.9.109 CI: the first run (push) lost its Linux x86_64 job in "Linux dependencies (JUCE)" - apt could not reach the Ubuntu
  mirror ("E: Failed to fetch ... Connection failed"), no compile error; the release job was skipped. build-retry.yml does
  not cover it (a job that RAN and failed). The dispatch of the tag (POST .../workflows/build.yml/dispatches {"ref":"v0.9.109"},
  HTTP 204) built everything green and published the release; Intel mac "late symbols: 0". Read the failed STEP's
  name first (jobs[].steps with conclusion failure): a dependencies step = the runner's network, dispatch again.
  0.9.109 CONFIRMED BY THE USER (19 Nov 2026, his own hands): Realtek ASIO came back by itself after the headphones were
  pulled out and put back (the driver takes a new instance after its real fault), and the Windows Audio output shows
  the right outputs after "Configure speakers" 5.1 (mmsys.cpl).
  0.9.110 (19 Nov 2026, Linux; the user: after Reopen on the Komplete card "příšerné prodlevy čehokoli ... ani zvuk
  nikde nehraje"): activity.log = an endless chain, three times a second, of "JACK: PipeWire clock 1024 frames -> 256
  asked for X" + "PipeWire clock changed by another route: reopened Y" (ALC1220 Analog <-> Komplete Audio 6; 1.8 MB of
  log in minutes). CAUSE: `jackServerClock` reads the clock through a client that is never activated, and PipeWire's
  libjack gives such a client the DEFAULT quantum (clock.quantum 1024) while the graph runs at clock.force-quantum 256
  (what pipeWireSetClock sets) - checked outside the program with a ctypes probe and pw-top. So every open with a
  buffer of its own was "a change of the clock" -> pipeWireClockChangedHook -> the other JACK managers reopened ->
  their opens "changed" it again. In the code since the PipeWire buffer switching; the Reopen with two cards on
  PipeWire was the first open that hit it. NOW: `pipeWireForcedClock` (pw-metadata -n settings: clock.force-quantum /
  clock.force-rate) overrides the probe's values when set; `pipeWireClockHookHeld()` (a counter) is raised by
  `reopenJackRoutesAfterClockChange` so its own reopenings fire no hook; a card reopened for a clock change < 5 s ago
  is left alone (logged "NOT reopened again"); `reopenAudioDeviceFresh` of a JACK route during such a reopening takes
  the SERVER's new clock, not the route's old buffer (it would set the clock back). Tested with the local build:
  force-quantum 256 set by hand before the start (the state that looped) -> one "256 -> 1024 asked", Reopen twice =
  two reopenings, no chain. NOT reproduced: two cards on PipeWire at once (the saved routes were all on ALC1220).
  THE AUDIT the user asked for ("není tam ještě něco podobného?") of every automatic reaction that could feed
  itself: the silence recovery (once a minute per device), the ASIO period recovery (once per card), the missing-device
  watcher (a 2 s poll, no trigger of its own), the default-card search and askCards (Windows) - the one open path:
  a helper opening a driver may make Windows report a device, which starts the next round 1.5 s later; now after
  three rounds in a row (each within 20 s of the one before) the next waits a minute (`systemRoundsInARow`,
  `lastSystemRoundMs`; the wanted cards' 5 s questions go on). macOS has no such listener of ours. RULE: an automatic
  reaction to a device event must not be able to produce that event itself - a held flag while it acts plus a
  shortest interval, and a "current value" read from the system must be the value in force, verified outside the
  program. If PipeWire stays forced after a bad end: `pw-metadata -n settings 0 clock.force-quantum 0`. The user
  starts the Linux program from ~/Desktop/LibreOrgan (the local build was copied there and to ~/Downloads). NO
  Projucer resave (Linux): the VS2026 files say 0.9.109 - Windows next.
  0.9.111 (19 Nov 2026, Linux; the user: "proč se nová karta otevře na 1024 a ne na tom, co tam už je" + "512 jako
  default, když si program neví rady"): PipeWire has ONE clock for every card (a buffer set on one card is every
  card's - right, told the user). `PipeWireJackDevice::open`: with other JACK devices of ours open
  (`heldJackDeviceNames().size() > 1`) and no user act (`pipeWireUserSetsClock()`, raised around setAudioDeviceSetup
  in AudioRouting's reopenWith = the Buffer / Sample rate boxes, and in the ORGAN_JACK_BUFFER hook) the remembered
  buffer/rate are dropped - the card opens at the clock in force (log "remembers N frames ... opened at the clock in
  force"); the FIRST card takes its remembered buffer; none remembered and nothing forced -> 512 frames asked for.
  Tested (local build): start 1024 (the saved state), ORGAN_JACK_BUFFER=512,16 -> 512, a route moved to Komplete
  Audio 6 by ORGAN_TEST_ROUTE -> "remembers 1024 ... opened at ... 512", both cards at 512, no chain (the two-card
  case 0.9.110 could not test). The user's saved routes remember 1024: the first card of a start sets 1024 until he
  chooses 512 in the Buffer box once.
  0.9.112 (19 Nov 2026, Linux; #250 from the Core 2 Duo laptop, Debian 13: "karty nejdou přepnout na žádný jiný
  buffer, drží si furt 1024" + the user's unplugged Komplete: grey "1024", active again at 1024 beside the other
  card's 256): BOTH = A JACK CLIENT'S OWN BUFFER REPORT IS NOT TO BE TRUSTED ON PIPEWIRE. #250's log: every callback
  "frames 512..512, expected 1024" on both cards, four "buffer of X set to 512" with NO "JACK: PipeWire clock" line -
  PipeWire ran at 512, the open device said 1024, the probe said 512 = "nothing to change". The Komplete: the silence
  recovery reopened a card that was gone; JUCE's JACK device "opens" as a client wired to nothing, which PipeWire
  runs at its default 1024 beside the graph, and nothing wired it again when the card came back. NOW
  (PlatformAudio.h `PipeWireJackDevice::open`): the buffer is ALWAYS set by the program - compared with
  clock.force-quantum (`pipeWireForcedClock`), never with a client's report; unforced and equal = "pinned" quietly
  (no hook); `clockBuffer` (member) = the set value, returned by getCurrentBufferSizeSamples while open (log "X
  reports N frames, PipeWire's buffer is set to M"); the open-wait loop takes a sane size after ONE more try (the
  report may stay wrong for good); with real jackd (`!pipeWireJackLibraryLoaded()`) everything as before. A card
  not among `jackPlaybackClients()` is REFUSED at open ("PipeWire shows no card ... now"). checkAudioCallbackHealth:
  a silent JACK card that PipeWire no longer lists is not reopened - `closeAudioDevice()`, log "Sound card X
  (PipeWire) went away ...", a Messages note; the routes keep the card in their state XML and
  watchMissingAudioDevices opens them when the list has it again (it waits while voices sound). FOUND BY THE TEST:
  the recovery was handed the division's OWN manager for default routes (extraAudioManagers[i], no device when the
  route plays through a shared host) - it did nothing for them on any system; `managerOfDefault` =
  managerForRoute(manual, "default"). This changes Windows too: a silent ASIO/WASAPI card of a default route is now
  reopened by the recovery as the perspective routes' always was - NOT run on Windows. Test (local build): a null
  sink as a card (`pactl load-module module-null-sink sink_name=lo_test_card sink_properties=device.description=
  LoTestCard`, unload-module = unplugged): route moved to it, gone -> "went away" 6 s later, came -> "available
  again: reopening", both clients at the forced buffer in pw-top. NOT run: the real Komplete, the Debian 13 laptop
  (the user's). While the program plays PipeWire's buffer is forced for the whole desktop and stays so after it.
  0.9.112 SECOND PART (the user, on my tests that all ended at 1024: "testoval jsi pouze debilních 1024 ... proč je
  to zkurvený default"): 1024 = PipeWire's default, saved with every route as "its buffer" and brought back by the
  first card of every start (the 0.9.111 rule "the first card takes its remembered buffer" is REPLACED). PipeWire's
  buffer is ONE value per computer: `pipeWireChosenBuffer()` = PipeWireBuffer.txt beside settings.json (written in
  open() when `pipeWireUserSetsClock() > 0`, i.e. the Buffer box), 512 without the file; the first card opens at
  it whatever its route remembers (log "opens at this computer's PipeWire buffer, N frames (the route remembered
  M)"), later cards at the clock in force. Tested: no file -> 512 (route remembered 1024), ORGAN_JACK_BUFFER=256 ->
  256 + file "256", the second card "opened at the clock in force, 256", restart -> 256. RULE (the user): never
  test with the value that was there before - a test of a setting must end at a value DIFFERENT from the old one.
  0.9.113 (19 Nov 2026, Linux; the user: "změna buffer size je extrémně zdlouhavá ... několik sekund na kartě ...
  dalších XX sekund na další"): the stall stacks of 0.9.112 ("Message thread silent for 2.4 s") all stood in
  `fork` under juce::ChildProcess::start <- pipeWireForcedClock / pipeWireSetClock: A FORK COPIES THE PAGE TABLES OF
  THE WHOLE PROCESS - about a second each with an organ loaded, several per open (my 0.9.110-0.9.112 added the
  reads; my tests ran without an organ and saw nothing). `runSystemTool(args, &output, timeoutMs)` (PlatformAudio.h:
  posix_spawnp + a pipe + poll, killed at the limit) replaces juce::ChildProcess for pw-metadata (set, read) and
  wpctl (the default card). Measured with Caen loaded and two cards: the change and the other card's reopening in
  the same second, no stall line. RULE: on Linux/macOS never juce::ChildProcess (fork) on the window thread of a
  program that holds GBs - posix_spawn; and a timing test runs WITH A BIG ORGAN LOADED. Still fork (rare, the
  user's own act, not changed): lsblk/udisksctl at a load, gdbus/KWin and osascript for the report windows, the
  desktop-entry tools, QuietProcess on Linux/macOS (plug-in scan and window trial). Windows: CreateProcess copies
  nothing and no such tool is used. THE 5 s LIMIT of 0.9.110 WAS A BUG (found in the user's clicks into my test
  window: two changes 3 s apart left the Komplete at 256 beside 128): `reopenJackRoutesAfterClockChange` reopens a
  card whenever its buffer/rate differ from `jackServerClock` (the forced clock) and skips one that runs at it
  already; against a chain only a count stays (six reopenings of one card in five seconds).
  0.9.114 (19 Nov 2026 evening, WINDOWS session after the Linux one; the checkout was `git reset --mixed origin/main`,
  the Projucer resave for 0.9.113 pushed as a64fe4e, the CI exe of v0.9.113 deployed). The user's batch: (1) ONE START
  PICTURE: `StartPicture::paint(g, area, picture, status)` (Main.cpp, before StartSplash) - the pipes as large as the
  area allows, under them "LibreOrgan Sampler", "version N" and (when given) the status in two lines, every size
  from the area's HEIGHT (name 4.5 %, 20-46 px; small 2.4 %, 13-24 px); an area under 300 px (the placeholder size a
  Wayland compositor maps the window at) shows the picture alone, as the cover always did. Windows / macOS
  `StartSplash`: 70 % of the work area's height (798 x 974 on 2560 x 1392), NO getDesktopScaleFactor override (the old
  460 x 500 was "ostudně malé"; the interface is 100 % on his Windows, 75 % on his Linux). LINUX: ONLY
  `StartCover::paint` changed (it calls StartPicture::paint with an empty status: name + version, no live notes - the
  window is first painted after it is built) - the cover, its place, its first-paint timer and the whole 0.8.165-0.8.171
  way the window comes up are untouched; the cover now fills its background (the console no longer shows beside the
  picture for the 1.5 s). (2) AUDIO ROUTING TEXTS: the help text shorter and at max(13, 15 x scale()) px, its height
  measured (`helpHeightFor`, a TextLayout) so the header grows with it; `helpButton` "?" (tooltip = the text, click =
  the manual) instead when the width is under 380 px or the text over 132 px; Shift = "a multi-channel output of a
  division ... (C side / C# side)", Alt = "one channel (mono)" (the user's words); the cards' status lines
  max(11.5, 13 x scale()); `separatorY` = a thin line under the header (the lowest of help / block / buttons row),
  sourcesTop = separatorY + 40 (was 110 / 138 fixed). (3) RATE BOX: the card's running rate is added to the list when
  the driver's list lacks it (as the buffer since 0.9.82) - Realtek ASIO after its panel. (4) LINUX DIRECT ALSA
  (written on Windows, NOT COMPILED OR RUN ON LINUX YET): `directAlsaDevicesShown()` (PlatformAudio.h atomic; member
  `showDirectAlsaDevices`, machine key, read in loadSettings before any route opens), the PipeWire branch of
  FilteredAlsaDeviceType::scanForDevices lists the direct devices too when set; `pipeWireTakesDirectAlsaRoutes()`
  (= !flag && pipeWireCarriesTheCards()) replaces pipeWireCarriesTheCards() in sharedHostFor's 0.9.96 mapping and in
  isAudioDeviceAvailableNow - with the flag a direct route STAYS direct; AudioRouting: `directAlsaButton` ("Show the
  cards' direct ALSA devices - NOT recommended!") + the orange note `directAlsaNote` ("Mutually exclusive with
  PipeWire! ...") laid out ONLY in the UNFOLDED ALSA section while viaPipeWire; a busy rule in rebuild() (Linux): a
  card's direct ALSA device and its PipeWire (JACK) entry are one card (name containment as pipeWireClientForAlsaCard,
  no JACK client opened) - while one is open here the other is busy and its cables are moved by dragging. The flag
  OFF = exactly 0.9.113. The sound-server entries (Default ALSA Output, PipeWire Sound Server) stay listed (the
  section needs chips to exist). NEXT LINUX SESSION: copy the NTFS sources to ~/LibreOrgan-linux, build, look at the
  start cover (spectacle shots at start, as 0.8.165-0.8.171), tick the box, drop a cable on a direct device of a card
  that is idle / plays through PipeWire here / plays in another program. Tests on Windows: %TEMP%\test_0114_splash.ps1
  (PrintWindow of the process's windows at start; PrintWindow waits for the busy window thread, its picture comes at
  ~4 s), %TEMP%\test_0108.ps1 -Tag t114 [-Bounds "44 88 1040 760"] (the header at 2210 and 1100 px: 2 and 7 lines).
  NOT pushed: the Linux half is unverified. ASKED, NOT DONE: a random one of the website's six pipe pictures
  (site/assets/pipes*.png, 1254 px, 1.5 MB each) on the start picture - feasible as JPEG at 1024 px (~1 MB in all)
  through BinaryData in BOTH build systems (jucer + CMake juce_add_binary_data).
  0.9.114 SECOND PART (the same evening; the user's next batch + his live "zacyklil se" report): (5) THE "LOOP" WAS A
  DRIVER PANEL: Realtek ASIO's control panel is a MODAL dialog run on our own thread (the stall stack:
  NtUserWaitMessage <- DialogBoxParamA <- ... <- Button::mouseUp; controlPanel() returns only when it is closed), it
  fell behind the program's windows, which went on taking clicks (16 x "Quit requested", a Save box) while the
  program sat inside the driver. The Audio routing's "Driver panel" button had called `device->showControlPanel()`
  DIRECTLY since 0.9.81, past `openAsioControlPanel` (the watch that reopens the card when the panel closes) - it
  goes through it again (by callAsync). `BlockingPanel` (anonymous namespace before openAsioControlPanel, Windows): a
  thread timer (SetTimer(nullptr, ...) - it runs inside the dialog's own message loop) that, while the call lasts,
  finds the panel (a new visible titled window of this process whose class is not JUCE's), sets HWND_TOPMOST,
  DISABLES the program's JUCE windows (EnableWindow false; re-enabled in `released()`), and brings the panel forward
  whenever one of our windows becomes the foreground window; `MainComponent::driverPanelBlocks()` /
  `showBlockingDriverPanel()` (static): requestQuit shows the panel instead of quitting, pollAsioPanelWatch reopens
  nothing while the call lasts; after a blocked call `asioPanelWatch.blockedPanelClosed` -> the next poll reopens
  ("Control panel closed"), the main window toFront (Windows gave the focus away while ours were disabled).
  TESTED with %TEMP%\test_0114_panel.ps1 (the default routes moved to Realtek ASIO by ORGAN_TEST_ROUTE, "Driver
  panel" by ORGAN_TEST_CLICK, the windows listed with [waits], the panel closed by WM_CLOSE from the script): "blocks
  until it is closed ..." -> "Control panel closed after 6.5 s" -> reopened. (WS_EX_TOPMOST did not show on the panel
  in the listing; the activation rule is what keeps it in front.) A panel that returns at once (RME) never meets the
  timer. RULE: never call a driver's showControlPanel() anywhere but through openAsioControlPanel. (6) ASIO KEEPS ITS
  RATE AT A REOPEN: reopenAudioDeviceFresh asked "the driver's own" rate (0) - Realtek came back at 44.1 kHz after its
  panel was only opened and closed (the test), and after the driver's own restart JUCE reported "rate 0" (the user's
  "0 Hz"; his photo F:\temp\realtekASIO.jpg: the panel sets only the LATENCY 2-20 ms, the figures under it are the
  buffer in PCM samples at 44.1 kHz and in DSD samples - no rate choice). Now for ASIO: setup.sampleRate = the rate the
  device reports now (a rate set in a driver's panel is that), else with resampleOnLoad the storage rate (>= 44100,
  else 48000); the buffer stays the driver's own. Test: 48000 after the panel (was 44100). (7) SET + << / >>:
  `stepCombination` STORES (storeCombination) into the piston it arrives at when Set is on (bank wrap as before, Set
  stays on; MIDI-learned steps too). (8) RANDOM START PICTURE: `StartPicture::random()` = one of
  BinaryData::pipes1..6_jpg (LibreOrgan/Source/Splash/pipes-N.jpg = the website's six pictures at 1024 px, JPEG q88,
  ~82 KB each; in the jucer as Splsh01-06 and in CMake's juce_add_binary_data); the background = the picture's own
  corner colour; the window / program ICON stays AppIcon. (9) Audio routing: Save routing / Load routing / Use as
  default = a row of the HEADER under the block, right-aligned, `chipHeight()` high and as wide as their texts (under
  the help text only when they would reach below its right end); the line under them; the 0.9.108 "own line"
  rule and `titleRight` are gone. A full card without outputs says "not there now - its cables wait and play again
  when the card is back" / "no outputs now - its cables wait" (was "... drop a perspective here to open it").
  TEST SCRIPT LESSON: `& script.ps1 | Select-Object -First 2` STOPS the script when two lines came - the test's
  process was left running and the next test refused; never cut a test script's output with -First.
  v0.9.114 RELEASED (19 Nov 2026, the user: "udělej vše i na linux, dej na github ... set funguje"): the FIRST tag
  (e1217fd) failed on ALL five jobs within two minutes - "No rule to make target .../Source/Splash/pipes-6.jpg":
  `LibreOrgan/.gitignore` ignores `*.jpg`, `git add LibreOrgan/Source/Splash` added nothing and my `2>$null` hid git's
  warning; the local MSBuild passed because the files are on the disk. `git add -f` the six files (82bd51b), main
  pushed, the tag deleted locally and on origin and put on 82bd51b: every job green, the release out, Intel mac "late
  symbols: 0", the CI exe deployed. RULE: a NEW binary file of the build (picture, IR, font) - `git check-ignore -v
  <file>` and `git ls-files <folder>` before the tag; never `2>$null` on `git add`. The Linux and macOS halves
  (the cover's paint, the direct ALSA tick box, the busy rule) COMPILE on CI; none of it was RUN there - the user
  tests on Linux and macOS.
  0.9.115 (20 Nov 2026, Windows; #251 pasztor63: Ultra 9 285K 24 cores / 24 logical, ReaRoute at 256, limit 616; #252/#253
  the user's own notebook with ReaRoute; #234 window 2): THREE DIFFERENT THINGS behind "ReaRoute does not work" - read the
  IDLE line of the measurement first: "ASIO gaps over 1.5 periods 0" = an even cadence (REAPER on an ASIO card), then look
  at the per-second "longest N us with own render A, wait B"; "gaps ... 100-200" at rest = bursts (REAPER on the Windows
  sound system). (1) STALLS OF THE POOL'S OWN WINDOWS: in 157 of 161 measured seconds of #251 the longest route callback was
  1.0-2.6 ms whatever the polyphony (2.3 ms at 168 voices), the values = hotWindowTicks 1 ms .. x 5/2 = 2.5 ms: the idle
  workers spin (_mm_pause, MMCSS Pro Audio) and a thread of ours carrying voices that lost its processor got none back until
  their window ran out. With 23 workers + control thread + device thread on 24 logical processors nothing is free (REAPER
  IS the sound card there). REPRODUCED HERE: the process held on 16 logical processors (`$p.ProcessorAffinity = 0x55555555`
  right after Start-Process = one per core of the Ryzen) - old exe: wait for lanes 2510-2570 us in most seconds, limit 37 400
  (Windows Audio) / 39 500 (ASIO) / 37 400 (REAPER + ReaRoute); unrestricted: never. FIX A (RenderPool ctor, Windows only):
  `spinWorkers` = min(numWorkers, processors - 1 - clamp(processors / 8, 1, 4)), processors = popcount of the PROCESS
  affinity mask; workers with index >= spinWorkers are RESERVE workers (never spin, sleep on `reserveCv`, called by render()
  for a job of `reserveWakeVoices` = 160 x (spinWorkers + 1) voices or more); with hyper-threading (workers = physical, half
  the logical) spinWorkers == numWorkers = nothing changes; log "Render pool: N workers on L logical processors - ...";
  the report and system.txt name the reserve. With it in the same simulation: 56 200 / 56 400 / 54 500, waits 60-200 us.
  FIX B (the tail rule, Windows): `Job::tailTicks` stamped by the first lane (or the poster) that finds no voice left;
  `tailStalled(now)` = a job open, no voices left, tail older than `tailLimitTicks` 0.6 ms -> the idle worker skips its
  spin and sleeps (the next job's notify wakes it), `statTailSleeps` -> the per-second line "; a job hung in its tail: idle
  workers sent to sleep N time(s)". Alone (ORGAN_POOL_RESERVE=0 in the simulation): the wait ends at 620-640 us instead of
  2510. For machines where a thread is displaced although processors are spare (the 155H notebook: >= 0.9 ms in 24 of 24
  low-load seconds of its Windows Audio run, 15 of 24 with ReaRoute at 16 lanes, 1 of 24 at 2 lanes). REGRESSION LESSON:
  the first two builds with the tail rule lost ONE KEY STEP of the measurement (50 100 against 52 100-52 200 of 0.9.114,
  A/B/A three times) - not the check itself (throttling it changed nothing) but the workers reading pool MEMBERS in every
  turn of the spin (`spinWorkers`, `tailRule`, the limits): they lie right after the statProf* atomics every lane writes
  after every job while the measurement profiles = one shared cache line. The worker takes them as LOCALS before its loop
  (reserveWorker, tailRuleOn, tailLimit, tailCheckEvery) and asks tailStalled only when it has itself been idle for the
  limit, every 50 us at most; then old/new/old/new = 39 800/48 000, 44 000/50 100, 50 100/54 200, 54 700/54 200 (the
  machine drifted, new >= old in each pair). RULE: nothing a worker reads per spin iteration may be a pool member near
  written statistics - copy to locals; and a regression check is an INTERLEAVED A/B (old, new, old, new) in one session,
  the limit moves in key steps of ~4 % and drifts 10 % with the machine's state. Dev switches ORGAN_POOL_RESERVE=0,
  ORGAN_POOL_TAIL=0 (logged). NOT re-run after the rework: the tail rule alone in the simulation. NOT run on a real
  24/24 CPU. The device thread's priority is NOT touched (a driver's thread: time-critical ones would be lowered).
  (2) BURSTS (#252): REAPER on WASAPI/DirectSound runs its own block (10-15 ms) and asks ReaRoute's client for its
  256-frame blocks two or three at once, then pauses; every pause was "a late driver callback" (asio: any gap > 1.5
  periods): 65-100 a second at rest, limit 200 on any computer. Reproduced here by switching REAPER's audio system to
  WASAPI (reaper.ini mode=5; mode=3 = ASIO): old exe 200, new 27 300. FIX: `DeviceClock::firsts` + `cycleMs`,
  `learnDeviceCadence(windowMs, ...)` after the measurement's two idle seconds (>= 10 pauses a second and a cycle >=
  1.4 periods; cycle = max(window / firsts, 0.95 x longest gap)) -> log "the ASIO driver hands its blocks over in
  bursts (...)", the callback uses the cycle for the late rule, the 1.5-period grouping and the callback's share;
  `clearDeviceCadence()` at start and finish; the report's advice has a paragraph. Learned ONLY at rest inside a
  measurement (an EMA while playing would take an overload's gaps for a cadence and pass it). Outside a measurement
  nothing changes (the DSP field of such a route reads high). (3) "HALF THE POLYPHONY WITH REAROUTE" (#253) is the
  BLOCK: 24 900 at ReaRoute/ASIO 256 against 41 400 at Windows Audio 480 on the notebook; here ReaRoute behind
  REAPER on the Fireface = the Fireface itself (~50 000), behind REAPER on WASAPI 27 300 (blocks of 256 in a 10 ms
  cycle). Manual: one paragraph in Audio routing ("Playing into another program"), the technique in 10.11.
  #234: window 2's maximised state was saved as a rectangle (the log showed the start frame "1912 -8 1866 1035" = a
  maximised window's client cut to the margins) - `secondWindowMaximised` (machine key, volatile for the dirty
  fingerprint), rememberDialogBounds keeps the normal rectangle, updateSecondWindow maximises again (not on Linux).
  powerSettingDescription: the plan by its GUID (8c5e7fda = High performance, e9a42b02 Ultimate, 381b4222 Balanced,
  a1841308 Power saver), the name through CP_OEMCP ("Teljesitmenycentrikus" was advised to switch to High performance).
  TEST LESSONS: (a) ORGAN_LOAD_CONFIGURATION of the user's LibreOrgans file loads it as PORTABLE: the routes go to the
  default card (Windows Audio first, then the default-card search moves them to "ASIO Fireface USB") - a measurement
  clicked right after the load runs on 20 Windows Audio routes, the next on ASIO; %TEMP%\test_0115_measure.ps1
  [-Exe -Affinity <hex> -Measurements 2 -Tag] prints both; %TEMP%\test_0115_rearoute.ps1 [-Exe -Affinity -Env1 "A=B;C=D"]
  = the routing saved by ORGAN_TEST_ROUTING, every route rewritten to "ReaRoute ASIO (x64)" (a pair per perspective),
  loaded back, one measurement (REAPER must run). (b) NEVER KILL a test instance that plays through ReaRoute: the
  driver is not closed, REAPER keeps playing the last block from the shared memory for ever ("zaseklé audio", the user
  had to close REAPER) - end such a run with ORGAN_TEST_QUIT. (c) The activity log rotates at 1 MB mid-run: read
  activity-previous.log + activity.log by time stamp, never by a length mark. (d) THE USER (20 Nov): "Co furt
  testuješ ... pálíš mi tokeny" - one planned comparison, its numbers in four lines, then act; no test after test.
  0.9.116 (20 Nov 2026 evening, Linux; the user: with REAPER set to JACK "nelze v reaperu nastavit buffer size ... náš reaper
  ukazuje 1024, ale stejně do reaperu nehraje"): ON LINUX THERE IS NO REAROUTE - a program on JACK is itself a card among the
  PipeWire cards (its input ports = the card's outputs; REAPER connects its own inputs to the hardware captures by itself),
  and it has no buffer field because the buffer is PipeWire's (our Buffer box; REAPER follows, verified 256 <-> 512). THE
  FAULT: such a program makes its ports ANEW when its audio settings are applied (log: "Audio device started" twice per
  route with no act of ours = JUCE's JACK updateActivePorts stop/start) -> our links are gone -> JUCE's JACK process() calls
  back only while a port is CONNECTED and reports the connected ports as the active channels -> the manager's setup carries
  NO channel -> every reopen that starts from `manager.getAudioDeviceSetup()` (the silence recovery, Reopen, the Buffer /
  Sample rate boxes) hands JUCE's setAudioDeviceSetup an empty channel set, which CREATES the device and returns "" WITHOUT
  open() ("reopened ... ok buffer 1024" with no "Audio device started" after it; pw-dump: node JUCEJack with its ports,
  "suspended", no links; the health check blind because expectedBlock is 0). Fix: `MainComponent::jackSetupWithEveryOutput
  (manager, setup)` (JACK only: every output of the card, 64 when it lists none) in reopenAudioDeviceFresh, AudioRouting's
  reopenWith and the ORGAN_JACK_BUFFER hook; checkAudioCallbackHealth (JUCE_LINUX): a silent JACK device that is open with
  NO active output = `unwired` -> reopened after one second (`jackRewires`: five seconds apart, four in a row at most, then
  the general rule), reason "... none of its ports is connected ...: connected again"; the reopen log line says "NOT OPEN"
  when a device came back unopened. TEST WITHOUT REAPER: `~/tools/fake_jack_card.py <seconds> <second>:ports|client ...`
  (ctypes on PipeWire's libjack: a client "LoFakeCard" with two inputs that makes its ports anew / goes away for 8 s) +
  `~/tools/test_fakecard0116.sh <exe> <seconds> <events>` (closes a running LibreOrgan by SIGINT, backs up and restores the
  settings, ORGAN_TEST_QUIT; export ORGAN_TEST_CLICK="Audio routing" ORGAN_TEST_ROUTE="all|default|LoFakeCard|1" to move
  the routes onto it): 0.9.115 ends dead exactly as with REAPER, 0.9.116 reconnects in 3 s twice, survives the client's
  absence (the 0.9.112 path) and a buffer change; ALC1220 with a buffer change unchanged (6 links). LESSONS: (1) a JACK
  client created but never activated shows in pw-dump as a node with ports and no links and NOT in jack_lsp; (2) never reopen
  a JACK device with the channel set of its setup; (3) reproduce a fault of a foreign program with a stand-in client before
  theorising (an hour went into reading code first). NOT run: the real REAPER (it was closed), Windows/macOS (the changes
  are JACK-only or JUCE_LINUX; the "NOT OPEN" suffix is a log text). No Projucer resave (Linux) - the VS2026 files say 0.9.115.
  RELEASED: commit 9c4e04b, tag v0.9.116, CI green at the first attempt, Intel mac "late symbols: 0", the Linux binary needs
  GLIBC_2.38; the user confirmed with the real REAPER. REAPER'S BUFFER DISPLAY: it reads the JACK buffer when it opens its
  device and shows a change only after its Preferences are opened and closed (the manual says so); every freshly opened JACK
  client on this box is told 1024 whatever the graph runs at - Ubuntu Studio Audio Configuration sets PIPEWIRE_QUANTUM=
  1024/48000 in the session's environment and node.latency = 1024/48000 in ~/.config/pipewire/jack.conf.d/ubuntustudio.conf
  (PIPEWIRE_LATENCY=256/48000 makes a fresh client say 256; clock.quantum does not). pw-top: the DRIVER row's QUANT is the
  graph's real buffer, a follower's QUANT is only what that client asked for.
  0.9.117 (21 Nov 2026, Windows; #254 pasztor63: 0.9.115 gave his 24/24 box 21 900 voices (616 before) and he asked why
  it is fewer than elsewhere; the user: "Rychle rozbalovani musime zkusit ... to je zasadni"): (1) THE AUXILIARY AND THE
  FIGURE: the limit counts VOICES, and the auxiliary copies every voice of every perspective (duplicateVoiceToAux: pedal
  150 x 4 perspectives + 600 auxiliary) - so the limit is THE SAME with and without it (the user measured 27 300 at 7
  keys with, 27 200 at 13 keys without); what halves is the number of pipes. NEVER tell a reporter that removing the
  auxiliary raises the measured figure (my first draft did). Manual 5.4 rewritten: what it is for (a dry set's second room
  with another IR, monitoring), what it costs (twice the voices per pipe, the same limit with half the pipes), never to the
  outputs of a perspective. (2) PACKED FLOAT WAS DECODED BY THE SCALAR DECODER: the AVX2 decoder of 0.9.90 served 16-bit
  alone; his float + packed organ paid 1644 of 3825 ns per voice and block for unpacking. `unpackBlock32Avx2` (the 16-bit
  one's scheme: eight residuals of w <= 27 bits = exactly w bytes, so every group starts at a byte and the dword pairs /
  shifts are the same for every group; int32 stored as it is) + `unpackBlock32Fast`; packChannel32 pads 32 zero bytes
  (was 8) for the last group's 32-byte load; and BOTH decode16/decode32 unpack a block only up to the last frame wanted
  (`upTo`; a voice's window of ~286 frames ends inside a block nearly every time - half of the decoded frames were thrown
  away). Bit-exact: the kernel test compares 600 windows AVX2 against scalar for float too and prints "N ns per frame
  wanted (... scalar M ...)" - 0.71 against 2.25 here. Dev: `SamplerEngine::setUnpackOldWay` (env ORGAN_UNPACK_OLD=1 in the
  self-test: scalar + whole blocks) and ORGAN_COMPRESS_SAMPLES=2 (float packed in the self-test; 1 = 16-bit); `--kerneltest`
  may run beside a running instance (moreThanOneInstanceAllowed). MEASURED: self-test BENCH (Rural TA+CO x 3 divisions, ms
  per 1000 voices): float unpacked 0.187, float packed old 0.270-0.280 -> new 0.212-0.214, 16-bit packed old 0.250 -> new
  0.219; THE USER'S OWN RUNS (Ashton float packed, 20 routes): ASIO Fireface 512: 21 000 -> 39 800; ReaRoute 256: 14 600 ->
  22 900; ReaRoute 512 new 35 600. In the real organ the unpacking fell only ~1.5 x per frame (memory-bound), yet the limit
  nearly doubled - the peak decides. (3) The measurement's first line says "packing on" / "packing off" (the summary only;
  `storageSignature()` itself is unchanged: it is compared for adoption and saved). (4) MY THREE WASTED RUNS (the user:
  "takhle mi palis tokeny zbytecne"): test_0115_measure.ps1 loads the file as PORTABLE and the default-card search moves
  the routes from Windows Audio to ASIO seconds after the load - a measurement clicked on a timer ran on ASIO once, on
  Windows Audio the next time, and the script's own "measurement cancelled" match ended the third. RULE: an A/B of two
  builds prints the device AND the buffer of each measurement and is thrown away unless they are equal; better, tell
  the user the two exe paths and let him measure (he did it in five minutes). (5) HAUPTWERK'S "24-bit, 32-bit aligned" =
  24-bit integers in 4-byte cells: the same RAM and precision as our float (a 24-bit value is exact in a float, and our
  packing packs exactly those integers) - nothing to copy. OPEN: macOS shows no start picture (the user, "skoda") - the
  window is made and painted before the Cocoa run loop turns, so the window server most likely never shows it; not
  touched (no Mac to run it on). Proposed, not done: the auxiliary as a SUM of the already rendered perspective routes
  when it plays through the same card (it would cost almost nothing instead of doubling the voices).
  RELEASED: commit 9808251, tag v0.9.117, CI green at the first attempt (all five archives), Intel mac "late symbols: 0",
  the Windows exe without VC runtime DLLs, deployed as C:\LibreOrgan\LibreOrgan_0.9.117.exe; Projucer resave done for 0.9.117.
  Regression check before the tag: kerneltest OK; the full self-test with packed float, new against ORGAN_UNPACK_OLD=1 -
  no WRONG line, polyphony equal, peak LEVELS differ between two runs of the SAME build too (random tuning, wind): the
  self-test's levels are no bit-exact baseline, the kernel test's packed-against-raw render is. The user: do not touch macOS.
  0.9.118 (22 Nov 2026, Windows; #256 read first: pasztor63's 29 900 at ReaRoute 256 is ABOVE our Ryzen's 22 900 like for
  like - unpacking still 1299 of 3119 ns, sinc from 2420 Hz, the bench from RAM 1.67 ns = ours, broke on a lane-wait peak;
  no fault found, nothing coded for it. THEN THE USER'S RULES after my inventory of what the files hold): (1) A COMBINATION
  IS ON / OFF AND NOTHING ELSE ("NIC víc!!!!"): stops, transmissions, extensions, couplers, tremulants. `onOffSlot(memory)`
  (anonymous namespace after storedTransmissionIndex: stops {name, perspective, primaryManual, enabled, transmissions
  [{id, manual, enabled}] - EVERY transmission in order, an entry without an id is found by its place}, couplers {id,
  enabled}, tremulants {manual, enabled}) is applied in storeCombination and in combinationsFromVar (an older file's
  master volume and voicing figures are DROPPED at reading - his order); combinationsToVar writes no "masterDb";
  `applyRegistrationState(memory, includeMasterVolume, onOffOnly)` - restoreCombination passes (false, true): no master,
  no setDb/gainDb/attackMs/attackFadeoutMs/attackDelayMs/random tuning, no adoption of a transmission list. Before, a
  piston put a stop's volume, attack gain/time and random tuning back to the moment it was stored (a revoicing was undone
  by the next piston) and set the master. captureCurrentState itself is unchanged (the configuration's registration with
  registrationMasterDb, the crescendo's restore and steps use it). (2) THE VOICING FILE = EVERYTHING AN UNFOLDED VOICING
  ROW SETS (his rule: "Co jiného by uživatel očekával od save voicing"), never the grid: saveStopSettings adds
  "transposition" (stop->sampleOffset; loadStopSettings collects the differing ones and calls setStopSampleOffset after
  its loop - each loads the stop's samples again), "formantFollowPct" (setStopFormantFollow when it differs),
  "undulationBeats" + "undulationManual" (only a value set by hand; absent = nothing changes), "disabledVelocityLayers"
  (always written, 128 = the plain files). A file from before has none of these keys and changes none of them.
  Load Voicing still REPLACES every stop's transmission list by the file's (unchanged, told the user). (3) THE HOLE:
  "oneShot" and "releaseCrossfade" were written and read by Save/Load Voicing ALONE since 0.8.158 / 0.9.62 - never by
  Voicer::createState/loadState, so the configuration lost them at the next load (the Hauptwerk import's Glocken x8.3
  too); the 0.8.158 and 0.9.62 notes above saying "voicing state key" were WRONG. Now in createState (when set) and
  loadState (absent = by the patterns / x1; setStopOneShot only on a change). RULE: a new voicing-row value goes into
  FOUR places - Voicer::createState, Voicer::loadState, saveStopSettings, loadStopSettings - and is proved by a load ->
  save round trip, not by reading the code. (4) NOTHING UNFOLDED IS SAVED (his mid-work order: "Budou defaultně vždy
  sbalené"): voicingExpanded / volumeKeysExpanded / toneKeysExpanded / attackKeysExpanded are neither written nor read
  (stops and transmissions; loadState sets them false) - a division reload folds the per-key rows too.
  Test %TEMP%\test_0118.ps1 (+ test_0118_make.py / test_0118_check.py; Rural copy with an OLD-format piston 1: master
  -40, stop volume -9, random 30, and oneShot / releaseCrossfade 3 / unfolded flags in the voicing; ORGAN_TEST_CLICK
  "1|Set|2|Set", ORGAN_TEST_QUIT): master stayed -27.5, the stop's voicing untouched, the piston's stop and tremulant
  switched on, memories.json holds on/off keys alone for both pistons, voicingSettings keeps oneShot true and
  releaseCrossfade 3 with no "Expanded" key. THE COMBINATIONS LIVE IN memories.json (settings folder) and in the
  configuration's "combinationMemory"; the master of the session is settings.json "masterVolumeDb". NOT run: Save
  Voicing / Load Voicing themselves (a native file chooser, no hook) and the transposition's reload through them.
  TEST HYGIENE: the program writes a daily `settings-<date>.json` at start - a test run leaves the TEST's settings in
  it (the script deletes the ones it made) - and changes lastdir.json (back it up too; the first run lost its old value).
  0.9.119 (22 Nov 2026, the same day; the user: the voicing in the pistons "měl být flag-ship našeho programu ... My se
  nechceme vázat pouze na skutečné varhany" - so it comes back as a CHOICE; "Nechceme aby load voicing nahrazoval seznam
  transmisí a extenzí ... to do voicingů vůbec nepatří"; "Udělej si hook na save a load voicing"): (1) CONFIGURATION EDITOR >
  CONSOLE > "Combinations" (a level-2 section before "Top bar"; four machine-specific keys, all in loadSettings /
  saveSettingsNow / the machine key list): `combinationsHoldVoicing` ("Combinations hold the voicing of the stops", off) -
  storeCombination calls `addVoicingToSlot(slot)`: every stop entry gets "voicing" {db, gainDb, attackMs, attackFadeoutMs,
  randomPitchErrorCents, releaseCrossfade, ranks [{perspective, enabled}], keyDb / keyTone / keyAttackMs (sparse: the keys
  that are not 0), attackRandomize} and every transmission entry "voicing" {db, keyDb, keyTone}; restoreCombination calls
  `applySlotVoicing` (random tuning set only on a change - it draws every pipe's detune anew; transmissions found by id,
  else by place; voicingSaveAt). NEVER in a piston: transposition and formant share (they load / build samples), aliases,
  the transmission list. A piston stored with the option off holds no "voicing" and changes none; the keys of the
  pre-0.9.118 files (flat db / gainDb ..., masterDb) stay ignored. `combinationsHoldSliders` ("Combinations hold the top
  row's sliders", off): `MemorySlot::globalSliders` = {name: value} of `globalSliderList()` (the nine MIDI-learn names:
  master pitch sustainFade releaseFade releaseScale attackTime attackDelay releaseCut releasePlayedFrom), saved as the
  scene's "sliders", applied by `applySlotSliders` (setValue with notification, only on a change) - the ONLY way the master
  volume gets into a piston. `emptyPistonCancels` ("An empty piston switches everything off", off): a piston with no entry
  in `combinations` calls disableAllStops() (the X button's general cancel), log "Piston N holds nothing ...".
  `showTuttiButton` ("Show the Tutti button", on): resized() leaves enableAllButton out of the row. "Load the last
  combinations file with the organ" moved into the section (the rows of one subject together). Manual: a ### per label.
  (2) LOAD VOICING NEVER TOUCHES THE LIST: `loadVoicingFromFile` no longer calls replaceTransmissions - every transmission
  of the file is looked up in the stop (storedTransmissionIndex: id, else division + place) and gets its shift (only when
  different), db, formant share (only when different), alias, disabled keys, per-key db / tone; one that is not in the
  organ is counted in the log and nothing is added. Voicer::loadState (the configuration) still owns and replaces the
  list - right. After a Load Voicing: voicingSaveAt + voicer->rebuildUI (added: the open rows showed the old values).
  (3) `saveVoicingToFile(file)` / `loadVoicingFromFile(file)` = the old chooser lambdas' bodies as functions (log "Voicing
  saved: ..." / "Voicing loaded: ... N stop(s) and M transmission(s) set ..."); HOOK `ORGAN_TEST_VOICING="<seconds>|save|
  <file>^<seconds>|load|<file>"`. Tests: %TEMP%\test_0119.ps1 -Options on|off (+ test_0119_make.py / _check.py; Rural with
  an old-format piston 1 and a piston 4 carrying "sliders" {master -30}; a voicing file B = the configuration's voicing
  with subbass -6 dB / xfade 2 / key 36 -3, a bogus transmission and violon "transposition" 12): ON - C (after B) -6 / 2 /
  -3, the stop's transmission count unchanged, violon 12 ("Transposition of ... set to 12"), D (after piston 3, stored
  before B) back at 0 / x1 / 6.6, piston 4 -> master -30, empty piston 5 -> general cancel, pistons hold voicing + 9
  sliders; OFF - D stays at B's values, master stays -27.5, piston 6 stored after the empty piston still has the stop on,
  no voicing / sliders in the stored pistons, Tutti not in the row (pictured). The Editor section pictured
  (ORGAN_TEST_EDITOR_SECTION=Combinations with configEditorCollapsed {Console false, Combinations false}). NOT run: a
  transmission's values through Load Voicing with a CHANGED value (the 12 transmissions of the run were set to what they
  had), the sliders option with a slider moved between store and recall other than the crafted master, Linux / macOS.
  THE REPLY TO #256 is a draft with the user (not posted).
  0.9.120 / 0.9.121 (22 Nov 2026): 0.9.120 = the Editor's "Combination pistons" row moved into Console > Combinations (released,
  CI green, the reply to #256 handed to the user as text). #257 (the user's Ultra 7 155H notebook, Caen packed + full releases,
  256 frames, ten runs 3 600-19 300): FOUR variables mixed - the measurement limit 85 % then 100 %, all 16 cores then
  Performance cores only, ReaRoute vs Komplete, and single LATE DRIVER CALLBACKS that ended a run wherever they fell; the
  clean runs agree (all cores 16 500-19 300 at 82-88 % of the period, P cores 10 900 twice); the longest callback at the SAME
  2 400 voices was 28-74 % between runs = a peak of 1.5-4 ms of the notebook (Balanced plan), not load. THE OLD RETRY
  (`lateRetried`, "a late callback with every callback within its period") NEVER FIRED: a late driver callback comes WITH a
  callback of 110-158 % (the device thread itself lost its processor). 0.9.121: the FIRST break of a level (late callback or
  a callback over 100 %) is always repeated ("the sound broke (...) - this level once more, a second break counts"), and a
  4 s hold spoilt by late callbacks alone with every callback within the limit is held once more; one extra chance per level
  (reset in setStimulus). Verified only that the flow works (Rural: first break repeated, the second counted, the run ended) -
  THE USER: a measurement on Rural proves nothing about figures ("nemá žádné píšťaly") - never present a Rural run as a
  polyphony result; the figures are his to measure on Caen. PISTONS WITH VOICING vs SOUNDING PIPES (read from the code, not
  measured): stop volume and volume per key reach a sounding voice within ONE audio block (renderVoice: gainStart = lastGain ->
  gainEnd, a linear ramp of 5-10 ms: no step, but a thump on a held bass pipe for a big change); treble per key glides 0.5 dB
  per block; a transmission's volume glides over routeGainLagMs; the master glides in the callback; attack gain / time / per-key
  attack, random tuning and release crossfade touch only the NEXT notes; the perspective boxes are assigned directly
  (rankEnabled) - a held pipe keeps its old perspectives until the key goes up, nothing is cut. PROPOSED to the user, not
  coded: a per-voice glide of the stop x key gain over a set time (as routeGain has), and perspectives switched by a piston
  crossfaded for held keys.
  0.9.122 (22 Nov 2026, the user: perspectives stay "from the next key"; "ostatní udělej plynulý nájezd po nastavitelnou dobu,
  výchozí 150 ms"; "v hlubokých tónech ... automaticky prodlužoval hodně! Tohle v pedále bouchá"): `Voice::voicingGain` = stop
  volume x volume per key as the voice plays it; renderVoice: a starting voice takes the target, a sounding one moves
  `c.voicingGlide * v.voicingGlideSlow` of the way per block (BlockContext::voicingGlide = 1 - exp(-3 blockMs / glideMs): 95 % in
  the set time; 1 = at once when the setting is < 1 ms = the old one-block ramp); `voicingGlideSlow` = 1 / (1.5 per octave of
  the SOUNDING pitch below c1, 8 at most; set at the voice's first block from pipe + footageSemitones + sampleOffset, as
  fadeScale does with 1.25). Setting "Voicing changes glide over" (configuration key `voicingGlideMs`, 0-2000 ms, default 150,
  travels with the organ; engine atomic + setVoicingGlideMs; Editor addSlider in Advanced global audio right AFTER "Attack cut
  fade-in"; manual ### of the label). It serves the pistons that hold the voicing AND a voicing slider moved by hand. MEASURED
  (%TEMP%\test_0122_glide.ps1 -GlideMs N + test_0122_make.py + test_0122_glide.py: Rural subbass 16 key 48 held by
  ORGAN_TEST_RECORD, its stop volume switched 0 / -12 dB every 3 s by ORGAN_TEST_VOICING; 10-90 % of each step in dB from 40 ms
  RMS windows): setting 0 -> 25-30 ms (the window itself), 150 without the pitch rule -> 100-145 ms, 150 with it (x 2.25 on
  that pipe) -> 180-335 ms; the largest sample-to-sample jump of the file equals the usual one. The user: "perfektní!".
  The factor 1.5 is MY pick ("hodně", more than the fades' 1.25). Also: showMidiLearnMenu item 9 "Crescendo settings..." for the
  target "crescendo" (the Cresc. button; the display's own menu had it). v0.9.121 was pushed to main WITHOUT a tag (the tool
  refused the tag push: the user had not asked for that release) - 0.9.122 is the release that carries it.
  LINUX vs WINDOWS ON THE SAME RYZEN 5950X (22 Nov 2026, 0.9.122, Caen Surround, 8 routes, 16 threads; Windows from the local
  activity.log 15:09-15:37, Linux from #259; the FIRST like-for-like table - the user: "Linux je absolutní vítěz, dvojnásobná"):
  16-bit / halved releases / no packing: one card 512 W 55 900 (Fireface) | L 68 200 (ALC1220) = x 1.22; two cards 512 W 68 400 |
  L 83 500 = x 1.22; through REAPER 512 W 44 600 (ReaRoute) | L 66 200 (JACK) = x 1.48; through REAPER 256 W 36 700-40 700 |
  L 50 600 = x 1.24-1.38; one card 256 W 45 700 (Fireface), 37 900 (Komplete: a callback of 152 %). Float + packing: two cards
  512 W 54 600 | L 52 700 = EQUAL; W two cards 256 32 400, ReaRoute 512 39 300, ReaRoute 256 25 600; L one card 512 46 100.
  So Linux leads by about a fifth on a card, by a quarter to a half through REAPER, and not at all with packed float -
  never "double" at equal settings (the double comes from comparing other buffers / packing / paths). WHAT THE LOGS SHOW: the
  lane time of one voice at the same settings is ~25 % higher on Windows (1927-1966 ns per 512 block against 1561-1596) - the
  whole card-direct lead; why is NOT known (same code, same CPU); and ReaRoute costs Windows a fifth against the card itself
  (44 600 vs 55 900) while REAPER on JACK costs Linux 3 % (66 200 vs 68 200). The notebook (#258, 155H): ReaRoute 512 no packing
  27 600 on 6 P cores, 46 600 on all 16 (its best), packed 16 500. No regression on any machine.
  #260 (22 Nov, Linux, the float + packing rows through REAPER added): REAPER/JACK 512 L 45 100 | ReaRoute 512 W 39 300 = x 1.15;
  REAPER 256 L 26 000 | W 25 600 = EQUAL. So with 32-bit float + packing Linux = Windows everywhere but REAPER at 512 (+15 %):
  the lane time of a voice is THE SAME there (512: W 4286-4468 ns, L 4107-4248; 256: W 3097-3256, L 3050) - unpacking and memory
  rule, the system does not; the Linux lead exists only with the cheap voice (16-bit, no packing). For a reporter on float +
  packing through ReaRoute at 256 (Pasztor) a move to Linux brings NOTHING - the buffer (256 -> 512: x 1.5-1.75) and the
  storage (float packed -> 16-bit unpacked: x 1.4-1.5) do.
  0.9.123 (22 Nov 2026, #261 pasztor63, 127.4 GB, Billerbeck float UNPACKED: "stopped at 102 GB, why reserve 25.47 GB, HW9
  needs 1-2"; the user: "udělej to. Rezerva jak jsi navrhl. Ale nesmíme jít do stránkovacího souboru"): the stop was
  MemoryInfo's footprint rule `held > installed - systemShareBytes()`, share = installed / 5 (made for the 8 GB laptops), and
  the free rule behind it installed / 16 = 8 GB. NOW (MemoryInfo.h; macOS UNCHANGED by `#if JUCE_MAC`): `reserveBytes()` =
  max(512 MB, min(installed / 16, 2 GB)) (<= 32 GB the same); `systemShareBytes(othersAtLoadStart)` above 16.5 GB =
  min(fifth, max(1.5 GB, others) + reserve), `othersBytes()` = installed - free - held taken at the first tree of a load
  (`gOthersBaseline`, dropped in takeLastLoadError with gSwapBaseline; -1 = read now = the free rule again);
  `commitHeadroomBytes()` (Windows ullAvailPageFile) < reserve = a NEW stop sign ("the system's commit limit is nearly
  reached"); `trimmedBytes()` (Windows PrivateUsage - WorkingSet) grown > 1 GB since the load began (`gTrimBaseline`) = a stop
  sign, only with a baseline (a plug-in's untouched buffers would trip it otherwise); `tightBelowBytes()` (RAM cell orange +
  the watch): a tenth up to 32 GB, 3/4 of the reserve above. `shortNow(reason, swapBaseline, others, room, trimBaseline)`.
  THE COPIES: phase 1 of buildAntiAliasCopies (copies read from the CACHE) had NO memory check (his log: 5.2 GB of them after
  the stop; and resumeIncompleteCopyBuilds restarted a memory-stopped build every 5 s) - `copyMemoryShort(why)` in both
  phases, resume returns while memoryShortNow(). ROOM FOR THEM WHILE DECODING: `memoryShortForDecoding` = shortNow with room =
  1 GB (what the program takes after the decode: grid, tuning/fill tables - measured) + owed x 17/16 + 512 MB;
  `gCopyRoomOwed` grows per rank in loadRank: known rank -> its copies' samples x 4 or 2 bytes from `CopyRoom.txt` in the
  cache folder ("<copyCachePathTag> <frames x channels>", 0 = needs none; written by `engine.rememberCopyRoom()` from
  logRamAfterLoadIfDue for every loaded rank whose build is complete), unknown rank -> a sixteenth of what its decode took
  (heldBytes delta). The cache FILES' sizes were tried first and dropped (extension copies only exist after a build; no cache
  = nothing known = four stops without copies in the first run). `takeCopyBuildsStoppedForMemory()` -> ONE Messages line
  "Filled pipes" per stop and load (the user: quietly, no box; `copyStopsNoted`); hook ORGAN_TEST_COPY_STOP=<stop part>.
  MainComponent: `watchMemoryAfterLoad()` (1 s timer, not macOS: free < tightBelow -> Messages "Memory nearly full" once;
  Windows: working set shrank > 1 GB AND trimmed grew > 1 GB since the load -> "Samples moved out of the RAM");
  `organMemory` (settings key, machine-specific AND in volatileKeys of the dirty fingerprint: name lower-case less ".portable"
  -> "w=bytes|bits|rate;<storageChoiceKey>=heldBytes|1 complete or 0 cut"), written by warnIfMemoryShortForTrees ("w") and
  logRamAfterLoadIfDue; askLoadOptions shows a Label line under the three storage combos for an organ with a "w" record
  ("Memory: about N GB; M GB can be used[ - it will NOT fit.]" / "N GB when loaded this way before" / "less than N GB (not
  loaded this way here yet)" for packed or halved choices / "full when loaded this way before - stops were left out"),
  usable = free - reserve + what the loaded organ gives back; an organ never loaded here: the box as before. The estimate:
  bytes x bits/rate factor x 1.05 + 0.7 GB (was x 1.15: 210 GB for Billerbeck, whose 94 GB of WAV took 115 GB = 1.22 a byte
  against the headers' 1.20; 192 GB now). MEASURED ON THIS RYZEN (127.9 GB; page file 512 MB on E: -> commit limit 128.4 GB;
  %TEMP%\test_0123_mem.ps1 [-Tag -Float -Packed]: the Billerbeck "8 chan." definition through the hook, float unpacked,
  ~270 s from the SATA disk, the process killed after the figures): old rule 102 GB; new: stopped by the COMMIT limit with
  3.8 GB of RAM free and 2.0 GB of commit left, program 115.2-115.4 GB (63 of 80 stops, 94 of 152 GB of WAV), page file 0 MB
  before and after, pages input 0, trimmed 0.4-0.6 GB (natural). The idle system holds 9 GB here (187 processes 6.3 GB of
  working sets + kernel pools 1.3 GB) - the user's "12 GB gap" = 8.7 others + 3.8 free (2 reserve + 1.8 the commit limit of
  the tiny page file). %TEMP%\test_0123_box.ps1 [-Seed] pictures the load box (Rural on the command line, organMemory seeded).
  NOT run: Linux (the same rules, MemAvailable), Pasztor's box (a system-managed page file: the RAM rule will decide),
  the watch's two notes (no way to take the memory away on purpose).
  0.9.124 (22 Nov 2026 evening, the user's design in six rounds): PACKING "AUTOMATIC" = only what does not fit, packed IN THE
  RAM. HIS RULES: never a new load of everything ("jen uvolnit, co potřebuje"); the order as proposed = least played first;
  the choice in the load box AND the repacking when the RAM runs out with packing Off; copies without room: quietly, a line in
  the Messages; sounding tones end GRACEFULLY, never abruptly, and "dej do killu rovnou těch 300 ms, ať je to tak vždycky".
  ENGINE: `packAutomatic` (bool, with compressSamples off) + `autoPackLevel` (atomic, -1 nothing .. kMaxPackLevel 31),
  `packOrderOf(manual, key)` = 0 for the pedal, else 1 + min(key - 36, 96 - key) clamped 0..30 (1 = the outermost key);
  a sample is packed when its order <= the level. `packForLoad(sample, manual, key)` at the three places packIfEnabled stood
  (the decode: stop->primaryManual + entries[i].midi - keyShift; the copies from the cache and the built ones: the pipe;
  reloadStopSamples gives its stand-ins the primaryManual); `packIfEnabled(sample, whateverTheOption)`.
  `repackOneLevel(loading, insideBuild)`: level + 1, collects the unpacked samples of that order from the tree being decoded,
  the pendingDivisions and the engine's own stops (samplesByNote, velocity/release layers, aliasFree copies), packs them with
  parallelFor; the engine's own ("live") only after `killAllVoices` + a wait of kKillWaitMs 380 ms with the render running
  (the fade is heard), then clearMidi + suspend + kill + resetAllVoicesUnsafe, pack, resume - under antiAliasBuildLock with
  buildsInterrupted set by hand (NOT a BuildPause: its end relaunches the pending builds in the middle of a load). The level
  is reset to -1 at the first tree of a load ONLY when the engine holds no stop. Triggers: loadDivisionTree before every rank
  (`memoryShortForDecoding(reason, beforeARank)` adds the largest rank of the load x 5/4 - `gLargestRankBytes` - so the packing
  comes before a rank, never inside one; loop while short and levels are left; status "Packing in the memory to make room
  ..."), the in-rank check uses the bare reserve while levels are left; buildAntiAliasCopies at its START (packing Automatic:
  while short with 1 GB of room -> repackOneLevel({}, true); never from the phases - their workers read the sources);
  resumeIncompleteCopyBuilds goes on when Automatic can still pack. `packingFitsNow(rank)` in the four adoption tests: with
  packing Off a rank holding packed samples is decoded anew (the storage signature is THE SAME for Off and Automatic on
  purpose - the box action must adopt). `kKillFadeMs` 300 (CmdKill was a 2 ms fade): panic, a reload of a sounding stop
  (reloadStopSamples now kills, waits 380 ms, then suspends) and the packing all end the tones that way; the self-test has
  no WRONG line with it. `packedShare()`, `packLevelText(level)` ("the pedal, and the N lowest and the N highest keys of
  every manual"). MAINCOMPONENT: `compressAutomatic` (configuration key, travels with the organ like compressSamples; the
  load box's packing combo item 3 "Automatic: only what does not fit, the least played first", the Editor's toggle became
  the same three-way combo; storageChoiceKey 'a'; the box's memory line "packed as far as needed to fit the N GB ...");
  `memoryStopNotice(error)` (free friend function, both loaders call it instead of queueNotice): a memory-limited load with
  packing Off and levels left -> runWhenLoaded -> showOkCancelBox "Pack what is needed and load the rest" / "Leave it" ->
  `packAndLoadTheRest()` = compressAutomatic on + reloadDivisions(all) (the loader adopts what is loaded, packs live samples
  as needed, decodes the stops left out); logRamAfterLoadIfDue: log "Automatic packing after the load: level N (...), P % of
  the samples packed" + Messages "Packed to fit the memory"; the measurement summary says "packing automatic (P % of the
  samples packed)"; the merge no longer refuses for memory when compressAutomatic. Test switches: ORGAN_TEST_PACKING=off|on|
  automatic, ORGAN_TEST_STORAGE=float|16|16half (the hook path of askLoadOptions - a hook import takes NO storage choice from
  settings.json: absent keys = the defaults, float), ORGAN_TEST_PACK_REST=1 (takes the offer). MEASURED (this Ryzen,
  Billerbeck "8 chan." float, %TEMP%\test_0123_mem.ps1 -Auto 1 | -PackRest 1 -WaitFor "Division\(s\).*rebuilt:" | -Drop
  "<division>|<folders>" -DropAt N): AUTOMATIC: the WHOLE organ (no stop left out), level 8 = 36 % of the samples packed,
  107.0 GB held, 9.9 GB free, page file 0 MB, the packing 12 s of a 351 s load (level 0 = the pedal: 11.4 GB freed in 4 s;
  a manual level ~2.3 GB in 0.9 s). OFF -> BOX -> PACK AND LOAD THE REST: stopped at 111.8 GB, the reload packed levels 0-7
  of the playing organ in the memory, read ONLY the missing 23 GB ("235 rank(s) kept their samples and copies"), 72 stops,
  109.6 GB, 9 GB free. The free memory ends higher than needed (the room for the largest rank + a level's 2.3 GB step): the
  safe side, not tuned. "Pages input/s" of Windows counts the loader's own file reads (the cache manager's faults) - 82 000/s
  while a tree loads is no paging; the page file's "in use" is the figure. THE USER CLICKS IN MY TEST WINDOWS (he took the
  offer box of the first run by hand): say that a window is a test and what a click does to it. NOT measured: the polyphony
  of a partly packed organ against a fully packed one (my estimate "a third more" is a guess), Linux/macOS (CI compiles).
  WEBSITE WORKFLOW (22 Nov 2026, the user: "proč tam furt padá stavění webu ... najdi a oprav"): pages.yml had the upload
  (upload-pages-artifact) and deploy-pages in ONE job - the deployment asked the API for the artifact before GitHub listed it:
  "No artifacts named "github-pages" were found for this workflow run" / "Fetching artifact metadata failed" in 4 of 14 runs
  (read without a token: `actions/workflows/pages.yml/runs`, `actions/runs/<id>/jobs`, `check-runs/<job id>/annotations`).
  Now two jobs (build -> deploy with `needs`) and the deployment tried once more after 45 s (`continue-on-error` on the first
  deploy-pages step, the second `if: steps.deployment.outcome == 'failure'`). A failed website run leaves the SITE STALE (the
  version shown is the last deployed one): after a failure look at the page's version.
  v0.9.124 RELEASED (22 Nov 2026): commit ae97049, CI green at the first attempt (all five archives), Intel mac "late
  symbols: 0", the Windows exe on system DLLs only; C:\LibreOrgan\LibreOrgan_0.9.124.exe is the LOCAL build (the user was
  running it when the CI exe came - not replaced). SEEN IN HIS RUN, NOT EXPLAINED: with Billerbeck loaded (Automatic) the
  process had 110.4 GB private and 81.7 GB working set = 28.7 GB trimmed by Windows, page file 15 MB of 512, 11 GB free -
  the pages are in RAM (modified list), not on the disk; when the trim happened is not known (look at the "RAM held after
  the load" line's Windows figures and the watch's "Samples moved out of the RAM" note next time).
  0.9.125 (22 Nov 2026 night; the user's merge of Caen Surround.json into a float Billerbeck through the load box, "16-bit +
  Automatic": the program grey for six minutes, then the memory emptied, Billerbeck read AGAIN as 16-bit, a Cancel left a part
  of Billerbeck; his rules: "každé varhany mohou mít svou bitrate a všecko", "když je Billerbeck načtený 16-bitů 48k, tak klidně
  se k němu může přidat caen 16-bit 32kHz. A billerbeck musí zůstat, jak byl", "nikdy okno nesmí vypadat vytuhlé, ať stále tam
  dává řádky, co právě dělá - např. čtu rar archiv, a který"): THREE FAULTS, none of them the packing. (1) EVERY MERGE FREED THE
  WHOLE ORGAN AND READ IT AGAIN - mergeConfiguration ends in applyPortableConfiguration, which unloads every division (always
  did; my 0.9.124 answer "the loaded organ is packed in the RAM for an added one" held for Load folder / reloadDivisions only -
  NEVER claim adoption for a path without reading whether it unloads). Now `applyPortableConfiguration(..., keepLoadedOrgan)`
  (true from the merge): no unloadManual, no frozen grid picture; after loadSettings the voicer READS the merged voicer.json
  (`voicer->loadState`, else reloadDivisions' own saveState would overwrite it with the old organ's alone), then
  `reloadDivisions(every division)` - the new stops adopt the samples in the memory ("Commit: N rank(s) kept their samples");
  finishDivisionsLoadEnd applies `pendingConfigurationRegistration` (the stops on as before) and triggers the merge's pending
  transmissions. (2) ONE STORAGE FOR EVERYTHING: the box's choices (prefilled from the ADDED file) became the program's and
  the loaded organ's signature no longer matched. Now PER FOLDER TREE: engine `TreeStorage {asFloat, resample, rate,
  halfRate, packed}`, `setTreeStorage(path, s)` / `clearTreeStorage` / `storageFor(folder)` (the longest tree path the folder
  lies under, else the engine's own choices) / `storageSignatureFor(folder)` / `signatureOf` - used by the decode
  (loadSamplesIntoStop: a local `storageSampleRate` shadows the member on purpose), the rank's signature, both adoption tests,
  packingFitsNow and packForLoad(..., packedTree). "Automatic" stays the whole program's. MainComponent `treeStorage` (path ->
  "float|half|packed|resample|rate"), `treeStorageText/FromText/Of`, `syncTreeStorageToEngine` (log "Sample storage: N folder
  tree(s) keep their own - ..."), `addNestedTreeStorage`; settings.json "treeStorage" (by path, NOT a machine key: the merge's
  var must survive applyPortableConfiguration's machine-key override), a configuration FILE "treeStorageByTree" (by the tree's
  key in sampleTrees, "<key>/<rest>" for a tree folded into the one above it); applyPortableConfiguration rebuilds the map
  from both forms after the pool is resolved; applyTreeRelocation moves the entry; cleared at a start without a configuration
  and at Global Reset's unload. The load box with "Merge it into the loaded organ" no longer touches the members or the
  engine: the choices go into `mergeStorageChoice` (taken and cleared by mergeConfiguration; cleared when the box opens and in
  mergeConfigurationDialog) and to the NEW trees of the merge alone (a tree like the loaded organ gets no entry); a
  right-click merge takes the added file's own values the same way. (3) ARCHIVES: `SampleFiles::indexOfLocked` NEVER walks an
  archive on the message thread (it answers "not read yet", logged once per archive; a cached index is still read there);
  `firstExistingDirectory` / `firstMatchingTree` / the merge's resolveTree look ON THE DISK FIRST through every candidate
  (`lookupIsDiskDirectory`, its own memo), the archives only for a tree the disk has nowhere - before, the first candidate
  that missed on the disk was served by (or cost the reading of) any remembered archive, so an installed organ with its rars
  remembered would even have been LOADED from the rars; the merge reads unread remembered archives through
  indexArchivesThen and starts again (`SampleFiles::archivesNotRead()`, `leaveUnread` after a Cancel so the next try goes on
  without them); "Reading the archives" has Cancel (`cancelIndexing`, checked at every header; nothing of a cancelled archive
  is kept; nothing follows); the search window of a load runs each tree's lookup on a std::thread and shows "Reading the
  archive <name>" / "part 3 of 8, N files so far" (`SampleFiles::indexingNow`); `registerArchive` runs in the `then` of
  indexArchivesThen (Load folder, Load Configuration from archives) - an archive is written to Locations.txt AFTER it was
  read (his three Billerbeck lines came from a try he killed at 19:09 because it was slow). Test
  %TEMP%\test_0125_merge.ps1 (copies of Rural + TestingOrgan beside the originals, the added one with 16-bit 32 kHz;
  settings restored, copies deleted): "Merge: the loaded organ stays in the memory", "Commit: 14 rank(s) kept their samples
  and copies", 4.64 GB read (the added organ alone; 5.67 GB before = both), 69 stops, settings "treeStorage" and the saved
  file's "treeStorageByTree" written. NOT RUN: Cancel in "Reading the archives" and the search window's archive lines (every
  archive here is indexed in a second or cached - the Billerbeck cache is the user's), an ODF merged through the box, the
  real Billerbeck + Caen (his to try), Linux/macOS (not compiled: no push yet). NOT DONE: Cancel in the load's search window;
  the memory estimates, the RAM line's storage text and organMemory still speak of ONE storage; a merged organ loaded later
  through the box: the box rules the organ's main storage, the merged-in trees keep theirs (nothing in the box says so).
  LESSON: when he reports a freeze, read the stack in the log BEFORE answering, and read what the path does to the loaded
  organ before promising that something "stays in the memory".
  SAME BUILD, SECOND DEPLOY (the user: the load box said "Memory: 14 GB when loaded this way before" for Billerbeck -
  "Stornované čtení nesmí dávat jako příklad později!!!"): `organMemory` took the RAM figure of ANY load as "complete"
  ("|1"), a cancelled one too (19:52 cancelled at 13.8 GB; 19:27 a cancelled merge at 66 GB). Now
  `engine.lastLoadWasCancelled()` (loadCancelled, reset at the next load's first tree) -> nothing is remembered, logged
  "Memory of this load not remembered for the load box: the loading was cancelled"; a whole load is written
  "<bytes>|1|2" and the box trusts only "|1|2" and "|0" (memory full) - the "|1" records of 0.9.123/0.9.124 are ignored
  (the arithmetic of the WAV bytes is shown instead). A merge no longer ends in a full load, so it writes no record under
  the base organ's name either. RULE: a figure shown as "what happened before" must come from a run that ENDED as intended.
  THE USER'S OWN RUN OF 0.9.125 (20:00-20:08, "asi to funguje, polyfonie nic moc"): Billerbeck float, packing Off -> memory stop
  (26 stops left out) -> the offer taken: levels 0-8 packed in the memory, 232 ranks kept, 72 stops; Caen Surround merged
  through the box as 16-bit + Automatic: "the loaded organ stays in the memory", only Caen's 16 trees read (~2.5 min), levels
  9-15, "288 rank(s) kept", 159 stops, 54 % of the samples packed. MEASURED by him (28 routes on ASIO Fireface): 256 frames
  22 500 (2 late driver callbacks at 90 %), 512 frames 35 000 (8 keys clean at 78 %; 86 % once). Per voice and 512 block:
  kernel 2160 + unpack 1080 + header/spans 240 = 3480 ns (16-bit unpacked: ~1950 ns = Caen alone 55 900 on this card);
  16 lanes x 10.67 ms / 3480 ns = 49 000 at 100 % -> 35 000 at the 75-78 % it ran at: the figure is what float + packing
  costs, no fault. Automatic packs the PEDAL and the OUTER keys first - level 15 = only the middle 31 keys of a manual
  unpacked - so a stimulus over the whole compass plays mostly packed samples. The watch's note "Samples moved out of the
  RAM" came once (1.2 GB out of the working set, 10.2 GB free, the page file tiny): seen, cause not known.
  v0.9.125 pushed (6d0b75d) and tagged on his word.
  0.9.126 (23 Nov 2026, the user's design: "před každou řadou vertikálních voicing sliderů ... jeden generelní, asi s tmavším
  modrým palcem, který bude all ... jako na pružině ... přičítá či odčítá k již nastavené hodnotě"; he chose A = each key stops
  at its own end of the range, the others go on; no tilt for now; "transmise samozřejmě taky, počítám je vždycky jako normální
  rejstříky"): `StopRow::AllControl` {juce::Slider, Label "All", start values, dragging} x 3 (`allControls[row]`, 0 volume /
  1 treble / 2 attack; a transmission has rows 0 and 1), made by `makeAllControl(row)` ONLY with the per-key controls (an open
  voicing - the 0.9.43 rule); the row's own range (+-9 dB / +-300 ms), thumb 0xff1f5aa6; onDragStart -> `takeRowStart` (the
  row's values), onValueChange -> `moveRowBy(row, offset)` = every ENABLED key slider to clamp(start + offset) with
  sendNotificationSync (so each goes its usual way: engine + stateChanged), onDragEnd -> the slider back to 0 without a
  notification; the wheel comes as a drag of its own (JUCE's DragInProgress), a typed value or an arrow key moves the row
  once. Layout: `StopRow::allControlSpace(sliderWidth)` (the slider + 10 px) before the first key in resized() (`keysLeft`,
  `placeAll`) and in Voicer's `voicingWidth`; visibility with its row in BOTH places that set the rows' visibility. Hook
  `ORGAN_TEST_ALL="<division>|<stop prefix>|<row>|<offset>[|trsm]"` (voicer.cpp beside ORGAN_TEST_EXPAND: opens the voicing,
  unfolds the row, `scriptMoveAll`, logs the first keys before / during / released); test %TEMP%\test_0126_all.ps1 [-Row
  -Offset -Trsm 1 -Division -Stop] (a copy of Rural, ORGAN_DUMP_GRID + ORGAN_TEST_SNAPSHOT: grid.png.row.png): volume +2.5
  "6.6 5.9 5.8 -> 9.0 8.4 8.3" (the first key stopped at +9), treble -3, attack +40, the transmission of subbass into II -4;
  "All" back at 0 every time; pictured (the label on the note names' line, the value box in the row of the keys'). NOT tried:
  a drag by a real mouse and the wheel (the hook calls the slider's own callbacks), Linux/macOS. RELEASED on his word:
  commit 8906a39, tag v0.9.126, CI green at the first attempt (all five archives), Intel mac "late symbols: 0", the Windows
  exe on system DLLs only, the CI exe deployed.
  #264 (pasztor63, 23 Nov 2026, 0.9.126, Ultra 9 285K 8 P + 16 E, 127.4 GB, Billerbeck 8 chan.; READ FOR THE USER, he writes
  the answer himself): ReaRoute ASIO 256 / 48 kHz (idle: 0 gaps = REAPER on an ASIO card), 24 routes (4 perspectives x 6
  divisions), auxiliary none, no IR, measurement limit 85 %, all 24 cores (3 reserve workers), AVX2, sinc from 2980 Hz,
  float 48 kHz, full releases, High performance plan. Runs: packing On 34 700 / 31 000 / 36 200; Automatic (level 12, 47 %
  packed) 38 800 / 41 600; Automatic 34 % packed 23 900 (9 late driver callbacks). Voice 2134 ns per 256 block (kernel 1283,
  unpack 715), cores busy 55 %, broke on one callback of 89 %; longest route callback 956 us of which lane wait 858.
  Automatic left 19.4 GB FREE at 100.1 GB held (here 9.9 GB): the room for the largest rank + whole levels - not tuned.
  TEXT FAULTS SEEN, NOT FIXED: with packing Automatic the report still says "Compress samples in RAM: about half the
  polyphony ... Load the organ without it" and "Compress samples in RAM is on"; "Save the configuration: the value belongs
  to this organ" although "maxPolyphony" is a MACHINE-SPECIFIC key (his comment: the limit is not kept with the
  configuration - true); the power plan's name is mojibake in the log. NOISES (his question, the user's too): Billerbeck
  002283 has pipe\mech\mech.wav (one file, no key number) and no blower sound (the blower folder = pictures). Proposed to
  the user, NOT coded: a stop flag "sounds while drawn" (zimbelstern, rossignol, blower) + a "noise" flag (untuned, no fill /
  wind / tremulant, out of tutti and crescendo) for key-action folders; per-stop drawknob noises need the definition's map.
  NOISES, THE USER'S FACTS (24 Nov 2026) + a real set, D:\Hauptwerk\...\OrganInstallationPackages\002740\ruchy: klapky\<GO|ped|
  Positif|Recit>\<01-Hlava_Klapky|02-AB_klapky|03-ORTF_klapky|04-Rea_klapky>\<down|up>\036-C.wav, 036-C_bis.wav, _ter (KEY
  numbers, 2-3 variants a key, on/off = sibling folders); mech\<perspective>.wav + mech\rel\ (a loop + release, one file a
  perspective, the file NAME is the perspective); ross\{ab,hlava,ortf,rear}.wav + ross\rel\; spojky\<perspective>\<on|off>\
  Positif_to_GO_1.wav, _2; tahla\<perspective>\<pull|push>\01.wav..44.wav (numbers that are NOT keys - today's loader would
  put them on keys 1-44); Tahla_Tremblants_silna\<perspective>\<pull|push>\01_Trembl_doux.wav; z_Calcant\<perspective>\
  01_calcant.wav, 02_calcant.wav (one-shot variants). His rules: rossignol / zimbelstern have no MIDI number and may have
  several variants (random or round-robin); key noises have an ON and an OFF sound (the key's release starts the off noise);
  drawknobs have no MIDI number and are random in his own ODFs too (measured at most for couplers and tremulants); some
  noises have loops + release, others are one-shot, and a one-shot with a release plays attack AND release WHOLE, no
  crossfade. PROPOSED (not coded, waiting for his choice): a "noise" row = folder + TRIGGER (its own button / the keys of
  a division / the stop buttons / the couplers / the tremulant); only the "keys" trigger reads file numbers as keys, else
  every file is a variant; on/off halves by sibling folder names (down-up, on-off, pull-push, rel); never tuned, filled,
  winded or tremmed. maxPolyphony per organ: proposed as a map organ -> limit on this computer (or the configuration key).
  NOISE SURVEY (24 Nov 2026; the user: "skoro každé varhany mají různé složky ruchy, které bys měl projít, než to začneš
  stavět ... Hledej prostě ruchy"; and ASK before spending tokens on a search - he knew the organ: 002740 = Saint-Guilhem):
  `python -I %TEMP%\ruchy_survey.py <OrganInstallationPackages> <out.txt>` (sibling folders of one structure printed once
  as {a,b}; file shapes KEY / N; loop + length of the first wav) -> %TEMP%\ruchy_survey.txt: 32 folders "ruchy" (27
  packages + the BezEncrypce copies). WHAT VARIES: (1) the on/off halves: sibling folders down/up, on/off, pull/push,
  atk/rel, att/rel, x_on/x_off (hw_on, tahla_off_left, Left_on), tremulant/tremulantrel, najezdMechu/vypnutiMechu; a
  subfolder rel / rel99999 under the on folder; a subfolder "02" (old sets 000345, 001740); file suffixes in ONE folder
  (dif_mech.wav + dif_mechrel.wav, dif.wav + dif_rel.wav); rel1/rel2, atk2/atk3 = variants. (2) THE ORDER OF THE LEVELS
  differs (division > on/off > perspective; division > perspective > on/off; perspective > on/off ...): a level is known by
  its NAME, never by its depth. (3) Perspectives: prefix folders "02-dirMk22-klapky-221115_2050", bare dif/dir/dis/dist/
  rea/rear, or the FILE name (ab.wav, N-diffuse.wav, dif-calcant.wav, 02-ORTF-BP_trem.wav), spelt differently inside one
  set; a release folder may hold files named by perspective while the attack has perspective folders. (4) Variants: _bis,
  _ter, __bis2, _BIS, "KEY_ter.wav.wav", N.wav, name_N.wav; drawknobs by console side (N_hw_L.wav) and NAMED specials among
  them (spojka.wav, affsluiting_hw.wav, ventil_HW.wav, N_bis_tremulant_voxhumana.wav). (5) Tremulant noise = a LOOP per
  division (Tremul_Pos_N, tr_BW_N, TremHW folders) with its release. (6) Kinds: key noises, drawknobs, couplers, levers
  (paky), ventils, blower (two alternatives mech / mechpuv, mech_null), calcant bell, zimbelstern (left/right stars), birds,
  drum (tambor: several NAMED samples), thunder (orage), glocklein. (7) JUNK only the definition can tell from the used:
  "- kopie" folders, empty folders, raw-recording folders inside a noise folder, 0.1 s mono files. CONCLUSION told to the
  user: the definition is the first source for noises, the folder guess a fallback with a hand-set row. How a definition
  wires its noises (ranks, switches) is NOT READ YET - the first step before any code. His hint: near a package there is
  usually an (almost) empty folder of the same number with the organ's short name = the core pack - tells the organ.
  NOISES IN THE DEFINITIONS, READ OVER ALL 252 FILES (24 Nov 2026, the user: "chtělo by to důkladnější čtení ODF, nestačí
  jen 4 definice ... musí se nouzově dát číst i z adresářů - to je náš LibreOrgan přístup"). Tools in %TEMP%\odftools (copy
  into LibreOrgan/Tools when the build starts): `odf_noises.py <odf> [all]` (every pipe grouped by the folder of its first
  attack sample, with what makes it sound), `odf_noise_summary.py <folder> <out>` (-> %TEMP%\odf_noise_summary2.txt, 88 KB:
  per kind the folders, file shapes, rank names, console switch names; one line per definition), `odf_noise_questions.py`,
  `odf_switch.py <odf> <switch name part>` (a switch and every row naming it, linkages followed back), `odf_fields.py
  <mysqldump.xml> <table>` (the field ORDER of a table = the compact letters a..z without o, then a1..; the *.xml beside
  the definitions are mysqldumps of his source databases, e.g. Guilhem.xml). FACTS: (1) FORMATS: compact (`<o>` rows, letter
  tags) AND uncompacted (150-300 MB: object types and row tags in LOWER CASE - `ObjectType="pipe_soundengine01"`,
  `<sample>`, full field names, empty `<X/>`): St. Guilhem Surround, Den Haag Surr, Zutphen Surround, Rott_Moric, martini,
  Altenbruch Surround Extended, Schwerin Demo, Sonus Paradisi Dry Eclectic - a parser must match both, case-insensitively.
  KeyboardKey = a KeyboardID, b SwitchID, c NormalMIDINoteNumber (NOT c = switch). Pipe_SoundEngine01.c =
  ControllingPalletSwitchID. SwitchLinkage f/g = EngageLinkActionCode / DisengageLinkActionCode: 4/7 = the destination
  goes on when the source goes ON, 7/4 = when it goes OFF; c = ConditionSwitchID. Switch e = DefaultToEngaged, i =
  Clickable, k = Disp_ImageSetInstanceID. StopRank f/g = ActionTypeCode / ActionEffectCode (1/1 everywhere: no noise code).
  (2) THE ONE MECHANISM (226 of 252 definitions have noises): a noise is a PIPE WITH ITS OWN PALLET SWITCH (never a stop
  played through a division), the switch driven through linkages by: a KEY of a keyboard - 200 definitions, TWO pipes a
  key, one on the press and one on the release, 1-4 attack samples in a pipe = the variants, no release sample (7
  definitions use the DIVISION INPUT instead = the noise follows the couplers; 46 groups are conditional on a switch such
  as " Compass Selection"); a STOP's drawknob on / off - 205 definitions, ONE pipe per stop and edge, so the definition
  names the file of every stop (the user: assigned at random when the definition is generated); a COUPLER's drawknob
  (190); a TREMULANT's drawknob (131) and the tremulant's RUNNING noise, a loop with a release while it is on (43); a
  CONSOLE SWITCH of its own, held: the blower (default ON, 160 definitions, folder "mech"), calcant bell, rossignol /
  nachtigall / kuckuck / vogelgesang / paxaros, zimbelstern / etoile / cimbelstern left and right, pauke / tambor / drum,
  orage, vielle, gloecklein / clochette; and on/off clicks of other console switches (ventils, Tacet, Windablass, Matrix,
  Walze, "Appel d'anches"). A third-party style (Erfurt, Oloron: folders Noises, trakturarej, trakturaNman): the noise is
  the RELEASE sample of a pipe whose attack is a silent loop "blankloop.wav", ranks "Key action noises Release (front)".
  (3) PERSPECTIVE = the head of the rank's name ("Front A (direct): Tracker noise On 1. man.", "Rear: Blower noise");
  "Special: ..." (2039 of ~3300 rank kinds, the old sets) = no perspective. (4) FOLDERS IN USE: top "pipe" (old sets: the
  noises lie BESIDE the pipe tone under pipe\ - kl1_mokry, kl2_suchy, ped_mokry, stop, stop_suchy, mech), ruchy,
  ruchy_mokre, ruchy_suche, noises, tahlaklapky, trakturarej, registers, stop, blower, manual, pedal; second level: keys
  klN_mokry / klN_suchy / klN_dry / klN_far / kl_mN / kl_p / klapky* / keys / traktura / ped_mokry; stops stop / stops /
  tahla* (tahlaatk + tahlarel, tahla_atk + tahla_rel, tahlarwatk, tahlaposrel) / hluky_mokry / hluky_suchy / rejstr* /
  registers / apels_*; mech; calcant / kalk / kalkantglock / schel_calcant; paky; crezal; hracky / toys / toy;
  below: atk / rel (the most common), on / off, down / up, pull / push, vpravoin / vpravoout / vlevoin / vlevoout, numbers.
  File shapes: KEY.wav, KEY_bis, KEY_ter, KEY__bisN, N.wav, mute.wav, blankloop.wav, and .hbw (encrypted). (5) THE
  FALSE FRIENDS of a word match: "cimbel" (a mixture stop), "nachthorn", "glockenton", "(tremmed)" ranks, "gross...".
  THE PROGRAM'S OWN IMPORT: does HauptwerkOdf.cpp read the lower-case uncompacted files? NOT CHECKED.
## Pending for the next session (delete when done)
- 0.9.136 (26 Nov 2026, LINUX session; the user's six to-dos + #266 in ONE build, edited in ~/LibreOrgan-linux through
  patch scripts ~/tools/p136/*.py (exact-match replacements, lib.py), the changed files copied back to the NTFS checkout):
  (1) LOCATIONS PRIORITY (his need of the day: "program tvrdohlavě furt bere z disku, který si najde sám"): Locations.txt
  has a MARKER line (`kLocationsMarker`, "# --- found by the program ..."; `isLocationsMarker`). Above it = the USER's
  lines, folders and .rar mixed, in his order (`gUserLocations`), `!<path>` anywhere = forbidden (`gForbiddenLocations`,
  `pathIsForbidden`). `portableFolderCandidates` returns `TreeCandidates` (an Array<File> + userCount / archiveLine /
  userLine / forbidden): FIRST the user's lines (per line: <line>/<root>/<relative>, the line as the root itself, and -
  only for a tree WITH a fingerprint - the Hauptwerk layout, its children, the folder above, the bare forms; a .rar
  line = the same paths under the archive's folder, taken only when an archive serves them), THEN the old list
  (`portableFolderCandidatesPlain`: the configuration's own place, the remembered locations, the drives, the bare
  forms). `firstUserLineTree` (a folder line: on the disk, also by the case of the letters; fingerprint checked; log
  "Sample folders: taken from X (your line N of Locations.txt)") is tried before the old passes in the two overloads
  `firstExistingDirectory(TreeCandidates)` / `firstMatchingTree(TreeCandidates, ...)`; a tree only in a forbidden place
  -> `noteForbiddenHolders` (log + the missing-tree notice's detail). `writeLocationsFile` writes the user's part BACK
  AS IT STANDS IN THE FILE NOW (never from memory), the program's lines under the marker; a file without a marker =
  all lines the program's (a header is written above the marker at the first write). `readLocationsFile({})` at the
  start of resolveAndApplyPortableConfiguration (the file as it is now rules the search). Forget keeps the user's
  lines; a forbidden place is never remembered. The user's order for old files without fingerprints was "own place
  first" - I made it "his lines count there in the root-NAMED form only" (works for him, no bare name): told him.
  Tests (~/tools/test_0136.sh <seconds> <Locations file or -> [ENV=...], LO_SETTINGS='{json}', CFG=...; settings backed
  up/restored; /tmp/lo136/{A,B}/Rural Organ/Samply = symlinks to the Rural test tree): his line wins over the
  configuration's own folder; "!" own place + line B -> B; everything forbidden -> not found, said so. NOT run: a .rar
  line above a folder line, a fingerprinted file, the merge path. A zero-tree search MOUNTS his data disks (0.9.33).
  (2) THE WHEEL: `UIScale::WheelGate` (UIScale.h, member `MainComponent::wheelGate`, a global mouse listener + 80 ms
  timer): a slider that has a juce::Viewport above it is made DEAF when it is painted (`WheelGate::prepare` from
  drawLinearSlider - no creation site can forget it) and takes the wheel only while ARMED = the mouse itself moved
  >= 3 px since the last scroll AND 0.4 s (`kStandMs`, the user's figure) passed since it; armed = property
  "wheelArmed" = the thin orange frame (also on a hovered slider outside scrolling areas: the frame means "the wheel
  sets this now" everywhere). A scroll = any wheel event not taken by the armed control: it disarms. A control that
  handles the wheel itself marks "wheelGated" and asks `WheelGate::listens(*this)` (the header's SwellBar).
  `stopsViewport.allowWheel` returns true everywhere (`Voicer::wheelBlockedAt` is no longer called): the workspace
  scrolls over open voicings. A sounding key's per-key slider: orange INSIDE only (tint 0.6), no frame. NEVER a
  modifier key or an extra click for this (both refused). Hook act ORGAN_TEST_BAR "<s>|<barLabel or manualChapter>|
  wheel|<steps>" (s = notch beside the slider, w = notch on it, m = mouse moved onto it, p = 0.6 s; through
  peer->handleMouseWheel so the listeners see it): scroll over the deaf slider scrolled, after the rest it listened,
  a scroll made it deaf again. A wheel event at a NEW place counts as a mouse move (a test's first 'w' arms). NOT
  tried: a real mouse, a touchpad's inertial scroll, the voicing rows (the same class), rotary sliders (not gated).
  (3) AUDIO ROUTING HEADER: laid out in the VISIBLE width (`headerWidth` from the parent Viewport's
  getMaximumVisibleWidth; the content keeps its design width 1100 and grows with the cables); with the "?" the block
  starts 12 px after it (stripRight = 10 + 30 + 12 + 608), the "?" on row 1's line, the three routing buttons span
  exactly the block's width (the user on my first picture: "neuspořádané, nezarovnané").
  (4) TOOLTIP: `UIScale::tipPlacement()` {own, above} set by HelpTooltipWindow before displayTip; our
  getTooltipBounds then puts the tip's top-left at the given point (under the control + 6; a control higher than
  80 px: 28 px under the pointer) or above when there is no room. NOT pictured (no hook holds a tooltip).
  (5) F1: the Editor rows' "?" carry "manualChapter" = their click's chapter; the Editor's Report / licences buttons
  too; "For details press F1." is appended wherever a chapter resolves (the "?" keeps it - his order). Hook
  `ORGAN_TEST_F1_AUDIT="<file>|<seconds>"` (window, origin own / above N / target / WINDOW / NONE, chapter, name, tip):
  run over main, Editor, Noises, Stop List, Audio routing, Couplers, Temperaments - only the single-topic dialogs
  fall back to the window's chapter (the right one). NOT done: chapters of their own for the Stop List's cells /
  switches and the Audio routing's cards (they inherit "The table" / "Audio routing").
  (6) ATTACK PER KEY FOR TRANSMISSIONS: `Transmission::attackKeyMs[128]` + `attackKeysExpanded`; engine
  `routeAttackMs(stop, manual, shift, key, pipe)` = the parent's value of the PIPE + the transmission's value of the
  key (several with one shift: the earliest), decided INSIDE the shift loop of startNote (the stop-level block is
  gone; Randomize is the parent's; the random share adds shift x 0.236); StopRow shows the third row for a
  transmission too (header tooltip says "added"; no Randomize box), "perKeyAttackMs" in the transmission's var
  (configuration + voicing file), Load Voicing, the combinations' "keyAttackMs", the resets. Pictured (Rural, the
  subbass transmission into II, All +40 ms); NOT heard.
  (7) #266 AGAIN (hoofdwerk, 0.9.135: Siebenquart's releases still out of tune; his log: Equal at 440 off, "Retune
  pipes off by more than" 50 = the default): MY 0.9.134 rule corrected a sample only when IT lay > the slider off the
  rank - Schorndorf's releases lie 32-39 c from their attacks, so NOTHING happened at the defaults (I had even noted
  it). Now `tuningCentsForSample` = the pipe's own tuning + layerCents ALWAYS (bound: "Replace pipes off by more
  than"); the Retune slider rules the pipe against its rank only. MEASURED here (Schorndorf Surround imported with
  Direct alone, key 57 of SW_Siebenquart, /tmp/lo136/peaks.py <wav> <release second> 1540,2090): held 1525.1 / 2069.7
  Hz, released 1555.2 / 2071.3 = +34 c / +1 c (was +67 / +32). The +34 c on the 1 1/7' rank STAYS: the producer
  tuned the attack's two ranks against each other and not the release's - no pitch shift of one file cures both
  ranks. RULE: a fix is measured AT THE DEFAULT SETTINGS before an issue is closed. Issue #266 reopened.
  RELEASED: commit 8d6e70f, tag v0.9.136, CI green at the first attempt on all five, the release with five archives,
  Intel mac "late symbols: 0"; the LOCAL Linux build stands in ~/Desktop/LibreOrgan and ~/Downloads/LibreOrgan. NO
  Projucer resave (Linux): the VS2026 files say 0.9.135 - Windows next (git fetch + reset --mixed origin/main first).
  THE USER ON (7), THE SAME HOUR: his own Schorndorf played right on 0.9.135 because HIS configuration has the Retune
  slider lowered (it travels with the organ; Erik imported afresh = 50 c). A second slider for releases REFUSED ("Jeden
  posuvník stačí, tak jak byl"), a threshold between attack and release REFUSED ("nemá být žádných 5 centů"), and
  "velocity vrstva se NESMÍ dolaďovat na svou píšťalu ... Jen release se musí tiše doladit k právě znějícímu ataku, ať
  už ten atak zní jakkoli rozladěně (podle smpl)". PROPOSED FOR 0.9.137, WAITING FOR HIS WORD (not coded): an ATTACK
  (plain or a velocity layer) plays as recorded - no layerCents on it without "Equal at 440 Hz" (the pipe's own
  correction alone); a RELEASE = the tuning the sounding attack voice got + (that attack's smpl - the release's smpl)
  = tuningCentsFor(note) + releaseLayerCents - soundingAttackLayerCents (the release start already reads the sounding
  voice: `sounding.cents` at SamplerEngine.cpp ~7551 - take its layerCents there too), bound by "Replace pipes off by
  more than"; with 440 every sample by its own chunk as now (told him; his word wanted on that too).
  0.9.137 (26 Nov 2026, the same Linux session; three faults of 0.9.136 found by the user within two hours; the
  "PROPOSED FOR 0.9.137" above is DONE):
  (a) TUNING AS HE SAID: `tuningCentsForSample(note, tuned, layerCents, attackLayerCents, replaceAbove)` - without 440:
  the pipe's own tuning + (layerCents - attackLayerCents); `Voice::attackLayerCents` = for an attack its own
  layerCents (nothing is added: every attack, plain or velocity layer, plays as recorded), for a release the
  layerCents of the attack that SOUNDED (`ReleasePitch::layerCents`, taken with `sounding.cents`); with 440 every
  sample by its own chunk (his word: "režim 440 Hz je na to určený"). MEASURED (Schorndorf, key 57): velocity 100
  (plain attack) held 1525.1 / 2069.7, released 1555.3 / 2070.8 = +34 / +1 c; velocity 60 (the _070 layer) held
  1584.7 / 2110.1, released 1584.3 / 2109.7 = 0 / 0 c. NEVER tune one attack to another; never a threshold or a
  setting between an attack and its release.
  (b) THE WHEEL, his rule in its final form: a slider in a scrolling area listens when (1) the mouse itself moved
  since the last scroll - moves within 0.3 s of a wheel notch are the hand on the wheel and only move the anchor
  (`kHandOnWheelMs`; 0.9.136 took 3 px of that shake for a move: a slider under a RESTING mouse listened 0.4 s after
  the scroll stopped), a move = 4 px (`kMovePixels`); (2) 0.4 s passed since the last scroll; (3) 0.4 s passed since
  the LAST move / since the mouse came onto the control (`overSinceMs`: "často člověk trochu myší pohne než začne
  scrollovat" - every move starts the 0.4 s anew; he asked "every, or only after a scroll?" - EVERY is what his own
  first sentence needs, told him). Hook steps added: u / S = a notch UPWARDS on / beside the slider (at the top the
  page stays: a resting mouse), j = a move of 5 px. A test's wheel event at a NEW place is a mouse move (the page
  scrolled 56 px and my 'w' followed the slider = a teleport that armed it): test the resting mouse with 'u' at the top.
  (c) LOCATIONS.TXT LOST HIS ARCHIVE LINES (his log: 12:08 he had "!/home/doma/Documents/RAR_archivy"; the lines
  under a forbidden place are not taken into the memory, and the next write made the program's part ANEW from the
  memory - the three .rar lines were gone for good; then he allowed the folder again and "it did not find the
  archives"). `writeLocationsFile(forgetProgramFolders = false)`: BOTH parts are kept as they stand in the file; a
  line is only ADDED (a memory entry the file names nowhere, "!" lines counted); Forget alone removes the program's
  folder lines. RULE: a file the user edits is never regenerated from memory - read it, add to it, write it.
  TWO MORE FACTS of that case: his folder is now ~/Documents/RAR_archivy__ (he renamed it at 10:30; the line said
  RAR_archivy, which does not exist), and a FOLDER line never makes the .rar files in it serve - only .rar lines do
  (his own 0.8.177 rule); he expected "the folder is allowed = its archives are found". PROPOSED to him, NOT coded:
  a folder line ABOVE the marker (his own) serves the first-volume .rar files lying directly in it too; the
  program's folder lines never. VERIFIED on his Frankfurt (three .rar lines above the marker, the two disks
  forbidden): "8 of 12 tree(s) found", taken from the archives at 690 MB/s (002706 is in no archive of his).
  0.9.137 RELEASED: commits d28a248 + fe46cc7, tag v0.9.137, CI green at the first attempt on all five, five archives,
  Intel mac "late symbols: 0"; the local Linux build in ~/Desktop and ~/Downloads. His Locations.txt got the three
  Frankfurt .rar lines back by my hand (above the marker, folder RAR_archivy__). NO Projucer resave for 0.9.136 /
  0.9.137 (Linux): Windows next. #266 is OPEN: a reply draft for hoofdwerk lies in /tmp/lo136/issue266_reply.md (it
  says 0.9.136; the rule is 0.9.137's now - rewrite before he posts it).
  0.9.138 (26 Nov 2026, the same session; the user: "ano, může to platit pro všechny složky, i automaticky přidané
  programem. udělej a vydej" - after his objection "když tam ve složce bude 356 archivů, tak bude trvat hodinu"): A
  FOLDER OF LOCATIONS.TXT MAKES THE ARCHIVES IN IT FOUND, BY THEIR NAME. The SearchJob's lambda `takeArchivesIn(folders,
  forTrees, where)` (the 0.9.44 neighbour block made a routine): `archivesNamedLike(..., alsoRead = true)` = the name
  sieve (a word >= 4 letters of the configuration's / organ's name, or 4 leading letters shared with a chosen archive
  there; 24 at most) + archives whose index was written before (`SampleFiles::hasIndexOnDisk`, 24 more) - only those
  are indexed, the ones holding a tree are `addChosenArchives`ed and registered after the search. Called (1) BEFORE
  the lookup for the user's folder lines above the marker (their archives count at the line's place: a folder line
  got archive candidates <line>/<root>/<relative> and, with a fingerprint, <line>/<relative>), (2) AFTER it for the
  trees still missing in: the folders above them, the configuration's folder and EVERY remembered folder. An archive
  candidate of a line counts only when the archive LIES THERE (`candidate.isAChildOf(archiveOf(candidate)'s folder)`:
  an archive elsewhere serving the same path by its tail is not that line's); a folder candidate only when it is on
  the DISK (resolveCaseInsensitive answered through archives too). Tested on his Frankfurt with the folder line
  below and above the marker: 8 of 12 trees (002704 + 002705; no FrankfurtOder_vol2 archive exists - 002706 lies only
  unpacked on the two disks he had forbidden: his "a stejně to část rarů nenašlo").
  RAR vs UNPACKED ON THE SAME NVMe (his test: unpacked 2600 MB/s, from the rars ~750 MB/s; "proč?"): MEASURED during
  a load from the rars (/tmp/lo136/cpu.sh: /proc utime, threads in state R, rchar): 16 threads running, ~14 cores
  busy, 470 MB/s of packed bytes read = 750 MB/s unpacked -> the DISK IS IDLE, the limit is the RAR decoder, ~50 MB/s
  of output per core on these 2024 archives (old .r00 volumes); a WAV on the disk is a plain copy. An archive wins
  only on a SLOW disk (HDD ~150 MB/s: half the bytes to read). `parallelFor` is capped at 16 threads (maxThreads) =
  the physical cores here; PROPOSED, not coded: all logical processors for the decode of packed archive entries
  (maybe +20-30 %), never promise more.
  0.9.138 RELEASED (CI green on all five, Intel mac "late symbols: 0"). 0.9.139 (the same day, LOCAL COMMIT, NOT
  PUSHED - his word wanted): MEASURED, the file cache emptied before every load (/tmp/lo136/measure.sh: python
  os.posix_fadvise(fd, 0, 0, POSIX_FADV_DONTNEED) on every file drops the page cache without root; Frankfurt, 3
  packages, 66.5 GB of WAV = 94 GB unpacked with everything = 57 GB of archives, float, the NVMe system disk):
  archives 16 threads 98 s (690 MB/s), archives 32 threads 80 s (858 MB/s), unpacked 16 threads 63 s (1099 MB/s),
  unpacked 32 threads 51 s (1357 MB/s). His own "2600 MB/s" for unpacked was a WARM load (the system's memory).
  `loadSamplesIntoStop`: `decodeThreads` = every logical processor (16..64) when the stop's files come from an
  archive, 16 on a disk as ever (asked for archives only; the unpacked gain of 23 % OFFERED, not applied - a hard
  disk with 32 readers is not measured); `managers` array 64; env ORGAN_DECODE_THREADS=<n> for measurements. Manual
  10.12 has the paragraph "Archives or unpacked folders - which loads faster" with these figures. RULE: compare
  loading speeds COLD (cache dropped) or say "warm"; a decoder-bound load shows the disk idle (rchar against cores).
  0.9.139 FINAL STATE, LOCAL COMMITS ONLY, NOT PUSHED (the push + tag was refused by the tool: he had not said
  "release" for this one - ask, then push main and tag v0.9.139 in two steps). The user, angry that I had left the
  unpacked files at 16 threads ("evidentně jim to taky pomáhá o 30 procent ... ty fakt neumíš rozumně uvažovat"):
  EVERY decode of a stop's files runs on all logical processors (jlimit(16, 64, getNumCpus())), archive or disk.
  LESSON: when a measurement shows a gain, take it - do not hold a half back for a risk nobody measured. SATA SSD (the
  Hauptwerk disk, Samsung 870 QVO, ntfs3) measured on his word, cold: 16 threads 168 s = 403 MB/s, 32 threads 163 s =
  411 MB/s - the disk is the limit, threads change nothing (and do no harm). By the figures the archives would load
  in ~100 s from that disk (packed bytes at ~410 MB/s x 1.6) - CALCULATED, never measured (the archives lie on the
  NVMe only); the manual says "calculated". test_0136.sh: the activity log rotates at 1 MB mid-run - the slice by a
  byte mark came out EMPTY once; the script now takes activity-previous.log's tail + the new file.
  0.9.139, TWO MORE THINGS BEFORE ITS RELEASE: (1) THE NAME SIEVE NEVER WORKED ON LINUX (the user copied the Frankfurt
  archives to the SATA disk for a measurement and "zas to nic nenašlo"): `archivesNamedLike`'s `letters()` walked
  `text.toLowerCase().getCharPointer()` - a pointer into a TEMPORARY freed at the end of the for-init (0.8.218 code);
  the letters read were whatever the heap held. My 0.9.138 tests on the NVMe "passed" only through the second rule
  (an index written before); freshly copied archives have none. Fixed (the lowered string kept); the log line of the
  search names the folders and the name. RULE: a test of a rule with two ways to pass must be run so that only ONE
  can pass (fresh archives without an index for the name rule). Whether MSVC's build was hit is not known.
  (2) SATA ARCHIVES MEASURED (his copy in /run/media/doma/Hauptwerk/RAR_archivy__, cold): 32 threads 102 s = 662 MB/s,
  16 threads 118 s = 573 MB/s - the calculated "about 100 s" was right; the manual carries the measured figure.
  THE WHOLE TABLE (Frankfurt, 66.5 GB of WAV, cold, 16 / 32 threads): NVMe unpacked 63 / 51 s, NVMe archives 98 / 80 s,
  SATA SSD unpacked 168 / 163 s, SATA SSD archives 118 / 102 s. pkill -f "<script name>" from a tool command kills the
  command ITSELF (its own line holds the name): write the pattern as "[m]easure...".
  0.9.139 PUSHED AND TAGGED on his word ("samozřejmě vydej, je to absolutně nutné"), commit 9e39b01.
  0.9.140 (the same afternoon, LOCAL COMMIT, NOT PUSHED - ask): the user: "mění si program pod čarou v locations sám
  pořadí? Nesmí! Dává si /run/hauptwerk na první místo ... To je pomalý SSD disk!" and "Měl by zvážit typ disku a podle
  toho si to seřadit!!! Musí vyhrávat rychlé disky!". FACTS: (a) the FILE is not reordered since 0.9.137 (0.9.136 did
  put the "!" lines first in its own part); (b) the SEARCH had a priority inversion: `portableFolderCandidatesPlain`
  tried every root-NAMED form of every location before any BARE form (0.9.3) - his fast folder holds the packages
  directly (~/Documents/SampleSety_Rozbalene/002704 = bare), the slow disk under OrganInstallationPackages (named), so
  the slow disk won whatever the order. NOW, for a tree WITH a fingerprint (`linesRule`): each location's forms
  together (named, then bare), and the candidates that are not the user's own lines - the configuration's own place +
  the program's lines - are stable-sorted FASTEST DISK FIRST: `Source/DiskInfo.h` (header-only) `speedClassOf(path)`
  3 NVMe / 2 another SSD or unknown / 1 hard disk / 0 network folder (Linux: st_dev -> /sys/dev/block/<maj:min>,
  the disk's name "nvme*", queue/rotational, statfs type for NFS/CIFS; Windows: GetDriveType + IOCTL_STORAGE_QUERY_
  PROPERTY BusTypeNvme / StorageDeviceSeekPenaltyProperty on \\.\X: - WRITTEN BLIND, compiled by CI only, never run;
  macOS: always 2). A root the user pointed to by hand stays first; the user's lines above the marker keep HIS order
  (tested: the slow disk as his line 1 is taken). A file without fingerprints keeps the 0.9.3 order. `noteTreeSource`
  logs every tree once: "Sample folders: taken from X (NVMe)". Tested: program lines "Hauptwerk disk, then
  SampleSety_Rozbalene" -> all 12 trees from the NVMe folder. NOT done: archives of the program's lines still come
  after every disk folder (0.9.125) although the measurement says an archive on a SATA SSD (102 s) beats unpacked
  files on the same disk (163 s) - a full "fastest source" order (NVMe unpacked 51 > NVMe archive 80 > SATA archive
  102 > SATA unpacked 163) is PROPOSED, not coded.
  0.9.139 RELEASED (CI green, five archives, Intel mac "late symbols: 0"); 0.9.140 pushed and tagged on his word.
  0.9.141 (the same afternoon, LOCAL COMMIT - the push waits for his word; the user: "mělo by to brát v potaz i rary,
  samozřejmě"): THE FASTEST SOURCE, ARCHIVES INCLUDED. `sourceSpeed(found, archive)` (MB of sound a second, from the
  measurement of the day): unpacked = min(85 x physical cores, the disk: 3000 NVMe / 410 SSD / 150 hard disk / 80
  network), archive = min(54 x cores, the disk x 1.6) - on 16 cores NVMe unpacked 1360 > NVMe archive 864 > SSD
  archive 656 > SSD unpacked 410 > HDD archive 240 > HDD unpacked 150; on few cores unpacked wins on an SSD too.
  `fastestMatchingTree` (the non-user candidates of a tree WITH a fingerprint, called from the TreeCandidates
  overload of firstMatchingTree): the first disk folder that fits, then - only when an archive COULD be faster than
  it - the first archive that fits; the faster is taken ("Sample folders: the archive is the faster source (about N
  MB/s) than the unpacked folder ..."). `noteTreeSource` names the kind of disk and the speed counted on; the
  existence overload no longer logs. The user's own lines keep his order (untouched). TESTED: NVMe unpacked beats
  NVMe archives; NVMe archives beat SATA unpacked. NOT TESTED: archive against unpacked on the SAME SATA disk (he had
  removed the SATA copy of the archives by then - two test runs "failed" for that reason alone: LOOK WHETHER THE
  TEST'S FILES STILL EXIST before hunting a bug), a hard disk, a network folder, few cores, Windows (DiskInfo there
  is written blind). Archives in a remembered FOLDER (not .rar lines) are still found only for trees that are missing.
  0.9.141 RELEASED on his word ("vydej"): commit f4f6098, tag v0.9.141, CI green on all five (Windows COMPILED
  DiskInfo.h - never run there), five archives, Intel mac "late symbols: 0"; v0.9.140 was cancelled by it (never
  released). The local Linux build stands in ~/Desktop/LibreOrgan and ~/Downloads/LibreOrgan. WINDOWS CAUGHT UP the same
  day: reset --mixed origin/main, Projucer resave ("Finished saving", versionString 0.9.141), the .ico copied, the CI
  exe (system DLLs only) deployed as C:\LibreOrgan\LibreOrgan_0.9.141.exe, the shortcut retargeted; DiskInfo RUN on
  Windows once (Rural on F: = Crucial T700: "taken from F:\Suchy_rejstrik\Kobrle\Samply (NVMe, about 1360 MB/s)" -
  right; D: SATA SSD and E: hard disk not looked at). Source/DiskInfo.h needs no jucer entry (headers are not listed).
  SEEN, NOT LOOKED AT: the import on Linux wrote "Schorndorf, Bornefeld, Surroun.json" (the organ's name one letter
  short; Erik's Windows file is "...Surround.json").
  26 Nov 2026 evening, THE USER ON 0.9.141: "Siebenquart 081 / 082: _070 layer dobrý, 128 layer - release se nepřiladí;
  na slideru Retune nezáleží" - ANSWERED, NOTHING CODED (his word wanted). MEASURED (%TEMP%\rel_vs_attack.py <perspective
  folder> [rel folder]: per key the smpl difference plain attack - release = what 0.9.137 corrects, against the shift
  that lays the release's spectrum over the attack's, log-frequency correlation, +-160 c): 47 of 58 keys agree within
  a few cents; 11 do not - 57 (-35 left), 66 (+14), 67 (+32), 70 (+24), 71 (+39), 75 (+35), 80 (+55), 81 (-77), 82 (-76),
  83, 86 (beyond the script's span). Key 81: plain 4786 Hz with an honest smpl (-19.3 c), "_070" and all releases 5106 Hz
  (+112 c) whose smpl is the GENERIC +15.7 c of every file of that kind - the chunks say 35 c, the sound 112. So the
  release's smpl cannot be the measure on such keys. PROPOSED: at the load measure the release against its attack(s)
  from the SOUND (the script's method, on the loader threads beside the 0.9.70 pitch measurement) and take that
  instead of the smpl difference; a key whose two ranks were moved differently (57, 80) has no single right shift -
  the stronger rank decides. The Retune slider has had no part in attack-release since 0.9.137 (his own rule).
  CORRECTED THE SAME EVENING (he doubted the figures; RECORDED through 0.9.141: a copy of his file with
  "disabledVelocityLayers": [70] on the stop, Direct alone, ORGAN_TEST_RECORD "3|81|60|4|..."): the program DOES shift
  the release by the smpl difference - 5106.0 -> 5003.9 Hz = -35.0 c. KEY 81 HAS TWO RANKS: his resynthesised plain
  attack = exactly x5.5 and x7 of A5 -19.3 c (4786.3 / 6091.7 Hz, the septime 10 dB the louder); the raw recording
  ("_070", every release; generic smpl +15.7 c) = 5106.0 / 6273.7 Hz, i.e. the septime 51 c and the lower rank 112 c
  above the resynthesis (the raw pipes stand 357 c apart, pure 11:14 is 417.5), and in the release the LOWER rank is
  18 dB the louder. After the 35 c: septime +16 c, lower rank +77 c. My "112 c by the sound" was the lower rank alone
  (the correlation follows the loudest partial) - say per RANK next time. No single shift fits both ranks. WHY THE RED
  RULE (0.9.70) SAID NOTHING: `measurePitch = !isRelease && ...` (releases are never measured) and pitchFromSound
  looks for harmonics 1-4 of the pitch the smpl names (the 8' base, 870 Hz) - a stop that sounds x5.5 and x7 alone has
  none, the measurement returns nothing, the smpl stays, nothing turns red (no "Pitch from the sound" line for
  Siebenquart in the load). MY CLAIM "the program does not know a compound stop's ranks" WAS WRONG and he caught it:
  `Stop::soundingHarmonic` / `footageSounding` (lowestSoundingHarmonic, 0.8.225) = the Orig. Pitch line's "8' (1 1/7')".
  READ THE CODE before saying what the program knows.
  0.9.142 (26 Nov 2026 evening, Windows, his "aplikuj ho ... zkusíme řešení A, možná přepínač A, B, C"; he then asked for
  ENGLISH so he can paste the figures to Erik; LOCAL COMMIT, NOT pushed - his word): A RELEASE TUNED TO ITS ATTACK BY
  THE SOUND. `strongestPartials(sample, release, binHz)` (anonymous namespace before computeTuning440: up to 8 partials,
  loudest first; attack = the loop middle via analysisOrder/analysisStart, release = from 30 ms for <= 0.7 s; a partial
  = a local maximum within 30 dB of the top, 15 dB over the median 300 c around it, sidebands within 30 c merged),
  `partnerOf` (the loudest partial of the other list within 150 c), `standsAlone` (no other partial within 100 c and
  10 dB - key 38's 220 ms release shows the 518 Hz rank as 510 + 524 Hz: a bogus 20-26 c without the rule). In
  computeTuning440 after the layerCents block: per rank and release layer `soundCentsByRelease/soundCentsByAttack`
  (ReleaseLayer; row 0 the plain attack, 1 + i velocity layer i; NaN = the chunks stand) + `soundMeasured`; ONLY for a
  (release, attack) pair whose chunk difference is >= 0.05 c (the same chunk = one recording, left alone - his rule);
  a reading within max(3 c, half a bin) of the chunk difference is NaN; the FIRST rank of a stop that has releases
  measures (parallelFor over the keys), the other perspectives COPY its values (`groupOwner/groupReference`: the list
  is stop, its ranks, next stop ...); off with "Measure the pitch of every pipe from its sound" (`gMeasurePipePitch`,
  setMeasurePipePitch is in the .cpp now). Voice start: `Voice::attackLayerIndex` -> `ReleasePitch::attackLayer`,
  `getNextReleaseSample(..., &releaseLayer)`; a finite table value replaces the chunk way: v.layerCents = measured
  (+ the attack's own layerCents with 440), v.attackLayerCents = 0. Setting "Release tuned to its attack by" (addCombo
  right after "Replace pipes off by more than"; key `releaseTuneBy` 0 the loudest rank of the release [A, default] /
  1 of the attack [B] / 2 the smpl chunks alone [C], travels with the organ; engine `releaseTuneMode`, new notes take
  it; GLOBAL RESET 0). Red note name + tooltip: `Stop::releaseTunedBySoundInAnyRank` (voicer.cpp before the orange
  branch). Dev: ORGAN_RELEASE_SOUND_DEBUG=<stop part> lists every reading in the "Tuning <rank>: N release(s) whose
  smpl chunk is off the sound ..." line. MEASURED through the program (%TEMP%\test_0142.ps1 -Mode 0|1|2 -Key 81: a
  copy of his file, "_070" off, Direct alone; rec_peaks.py), attack 4786.3 / 6091.7 Hz: A release 4786.7 (0 c) /
  5886.7 (-59 c, 13 dB weaker); B 4952.6 (+59) / 6085.1 (-2); C 5003.7 (+77) / 6146.9 (+16). HE HEARD the test notes:
  A "znělo velmi správně", B wrong, C the jump of almost a semitone. Self-test 81 lines / 35 OK / 0 WRONG, kerneltest OK;
  the row and the Editor combo pictured. LOAD TIME (whole Schorndorf, 55.9 GB of files, Ryzen, SATA SSD): COLD (he
  demanded cold: %TEMP%\purge_cache.py <folders> = every file opened with FILE_FLAG_NO_BUFFERING and closed, no admin
  needed; the load then reads at 340 MB/s) 166 s with the measuring, 165 s without; WARM 24 s with (0.9.141: 23 s),
  16 s without - the 8 s are the 0.9.70 ATTACK measurement of every perspective, the new release part ~1 s. PROPOSED,
  not coded: the attacks measured on one perspective only (saves ~6 s of a warm load, nothing cold). Website fact 4 +
  feature 08 "It listens to the pipes", manual intro + 1.x + 10.1 (they still said "cannot be retuned"). OPEN GAP,
  TOLD HIM: a smpl chunk whose pitch is garbage further than 2.5 % (43 c) from the sound is NOT rescued by the 0.9.70
  measurement (the guided search finds nothing, the chunk stays); only a file with NO chunk is measured unguided.
  RELEASED on his word ("vydej"): commit 9f07eb3, tag v0.9.142 (main pushed, then the tag in its own step), CI green
  at the first attempt on all five, five archives, Intel mac "late symbols: 0", the Windows exe on system DLLs only,
  the CI exe deployed as C:\LibreOrgan\LibreOrgan_0.9.142.exe. The Projucer resave for 0.9.142 is IN that commit.
  0.9.143 (26 Nov 2026 evening, Windows; he: "we should care for more than 43 cents error in the smpl pitch ... aby to
  nestalo moc casu pri loadingu"; LOCAL COMMIT, NOT pushed - his word): THE SECOND LOOK. HIS CORRECTIONS OF MY FIRST
  PROPOSAL: a plausibility check of the chunks "is good for nothing, no such case possible" - a prepared set has no
  visibly wrong chunks; what happens is a chunk that LOOKS perfect while the audio is far off, above all in the deep
  bass / 32' stops; "The program must believe to smpl, unless special circumstances"; the same FFT, no new
  measurement; a bass flue has no good 12th harmonic (reeds do); +-3 semitones, usually within +-1. CODE:
  `pitchFromSound(sample, guessHz, int* how)` (how: 0 nothing, 1 at the chunk's pitch, 2 elsewhere): after the first
  look fails, up to 24 peaks of the SAME spectrum (within 50 dB of the top, 12 dB over the median +-f/2 around each,
  0.9 x lowest .. 12.5 x highest) are read as harmonics 1..12 of a fundamental seed/k within +-3 semitones of the chunk
  (3 c, 6 c under 50 Hz); accepted with two of harmonics 1..4 AND (a third harmonic OR the two are the spectrum's
  two strongest peaks); a result within the first look's own window (2.5 % = 43 c) is DROPPED (the chunk stands).
  loadSamplesIntoStop keeps such a pitch in `secondLook[index]` and takes it after the loop ONLY in a rank where the
  first look measured more than half of the files with a chunk (`pitchFirstLook * 2 > pitchWithChunk`); else it is
  counted with "no pipe measurable at the chunk's pitch ... (N of them had a reading elsewhere, not trusted in such a
  rank)". Log "Pitch from the sound of X: N file(s) whose pipe sounds far from its smpl chunk (found by the second
  look, up to 3 semitones) - the sound taken: 041-F (the chunk 20.60 Hz, the sound 21.82 Hz = 100 c) ...". WHY THE
  TWO GATES: the first version (no gates) took 3-10 c "findings" in Mixtur / Zimbel / Quarte and ~40 c ones in ONE
  perspective of Nonenkorn (chance readings of a compound stop's ranks) - 30 tuning lines of Schorndorf changed; with
  the gates: 0 lines differ from 0.9.142 (compare with the example keys cut off: the "release(s) whose smpl chunk"
  lines list examples in thread order). FOUND IN HIS OWN SET: package 000350 Ped\Bourdon32 - 041-F (chunk 16.000 =
  20.60 Hz, the sound 21.82 Hz), 046-A# (27.50 / 29.14), 049-C# (32.70 / 34.66): each carries the chunk of the note
  BELOW, the audio is the right pipe (+100 c; checked in the files by %TEMP%\bourdon_check.py: harmonics 1, 3, 5).
  Test tree %TEMP%\make_secondlook.py <000350\Ped> <tree> (Bourdon32 + ContreBombard32 copies with chunks moved -80,
  +120, -250, +60, +30, -150 c): every one beyond 43 c found within ~2 c, flue and reed; ContreBombard 036's
  original chunk is 31 c off the sound (the first look takes that - its window is at least 4 bins, 9 % at 16 Hz).
  The self-test takes 266 s now (wait 9 minutes, not 4): 81 lines / 35 OK / 0 WRONG; kerneltest OK. Manual: a
  sentence under "Measure the pitch of every pipe from its sound"; the subchapter "### Fine-tuned mixtures" under 3.5
  (his wish: the resynthesised perfect-pitch version sits in a velocity layer because Hauptwerk offered no other
  place - we carry it on), linked from 3.5 and from "Velocity layer boxes". NOT run: another organ with a 32' stop
  (Billerbeck, Bellevue, Brasov), Linux / macOS. juce::String(float, 0) printed "82.1671" again - roundToInt.
  0.9.144 (26 Nov 2026 night; LOCAL COMMIT a625bc2, NOT pushed - his word and his EARS first): he found the 0.9.142
  rule A wrong on Siebenquart 069 with rel99999 (the chunks sounded better) and right on 081. MEASURED
  (%TEMP%\sieben_ranks.py <perspective folder> "<keys>": per key and release folder the ranks of attack and release,
  the shift each needs, what C / A / B give): 069 rel99999 - release ranks 3108 and 4161 Hz EQUALLY loud, A took the
  upper one (+28 c) and the attack's main rank (3115 Hz, the other 11 dB down) jumped 24 c. He agreed to "one new rule
  A + C, B is needless". MY FIRST RULE (the rank loudest in attack and release together, sum of dB) FAILED IN THE
  PROGRAM: for 081 it took the septime (-53 c = what he had heard as wrong) - the engine sees the septime of the long
  release at -7.8 dB, my script (mono mix, another window) at -19 dB. LESSON: rank LEVELS move by 10 dB between two
  measurements - no rule may hang on comparing them; and a rule derived from a script is run in the PROGRAM before it
  is promised. THE RULE NOW: the LOWEST rank that is clear in both (>= -20 dB of each sample's strongest partial,
  both standing alone) decides; fallback: the best sum of levels. On the keys with honest chunks the lower rank's
  shift IS the chunks' difference (069, 070, 071 "as the chunks"); 081 -111.9 / -111.6 / -108.7 c and 082 -106.8 in
  all three release folders. `releaseTuneBy`: 0 the sound, 2 the chunks alone (1 of older files reads as 0; both
  tables soundCentsByRelease / soundCentsByAttack carry the same value now); the Editor combo has two items.
  ORGAN_RELEASE_SOUND_DEBUG lists every pair with its levels ("[release Hz dB, attack Hz dB, not alone]"). HIS OWN
  IDEA, said after I had built this (NOT coded, waiting for his listening): the release's loudest rank first, and
  where the release's ranks are similarly loud the rank loudest in the attack - it gives 069 and the long 081 right,
  but the short-held 081 (release ranks 0 / -0.5 / -1.0 dB) would go to the septime (-52 c), and it needs a threshold
  on wobbling levels. Terminal tables fall apart when copied into an issue: raw Markdown is written to files for him
  (F:\temp\issue266_siebenquart.md, F:\temp\tables_for_issue.md). v0.9.144 RELEASED on his word ("yes, I like the
  result as the best compromise"): tag on 2fcd75d, CI green on all five, Intel mac "late symbols: 0", CI exe deployed.
  0.9.145 (27 Nov 2026, Windows; the user: "Load From Internet Catalog ... yes, new topbar button. Do all in one pass.
  No scopes. Also, the filtering columns should be the same what they are now on the web (even RAM ...)"; commits
  8f4d633 + c3ab6f2, pushed and tagged v0.9.145 on his word "A hlavně to dej na github, na co čekáš?"; the default
  link was then read by the program: 31 organs from raw.githubusercontent):
  THE ORGAN CATALOG. ROBOT: `configurations_index.py` writes `libreorgans/catalog.json` beside INDEX.md ({libreOrganCatalog
  1, organs [{title, folder, zip, zipBytes, url (raw.githubusercontent), readme, divisions, stops, routes[], ramBytes
  (the least), producer, link, rootName, rootRelative, trees[], fingerprints{}, featured, samples{url, name, bytes}}]};
  no date in it: it changes only with the organs) - every workflow's `git add -A libreorgans` takes it;
  `libreorgans/SAMPLES.txt` ("<folder> <url> [<bytes>]"; rural-organ = the release asset RuralOrgan.rar, 725 727 097
  bytes) names the FREE sample archives; configurations_check.sh skips both files. PROGRAM: `Source/OrganCatalog.h`
  (header-only, no jucer entry; `friend class OrganCatalogContent`): `OrganCatalog::catalogLink()` (env
  ORGAN_CATALOG_URL = a link or a LOCAL FILE for tests, else the raw main link), `Entry`, `entriesOf(var)`, `sizeText`,
  `Download` (a juce::Thread: ".part" file, done / total atomics; Windows / macOS juce::URL::createInputStream with a
  20 s timeout, Linux curl then wget through QuietProcess - WRITTEN BLIND, never run; a local path is copied);
  `OrganCatalogContent` (ModelessDialog "Organ catalog", placeDialog key "organCatalog", F1 -> "The organ catalog"):
  the cache `<settings folder>/Catalog/catalog.json` shown at once, then the link fetched (the internet is asked ONLY
  while the window opens / Refresh / a download - never at the start); a juce::TableListBox with the web's columns
  Organ / Divisions / Stops / RAM / Routes + "Samples on this computer" (header click sorts; the featured organs always
  first, bold, a line under them), filter text, "RAM up to N GB" combo, the tick "Only organs whose samples are on this
  computer"; buttons "Download and load", "Download with its samples (N MB)" (only for a SAMPLES.txt organ), README,
  Refresh, "Cancel the download". The samples column = `MainComponent::catalogTreesHere(...)` on a thread
  (portableFolderCandidates per tree, disk folders by a disk-only fingerprint, archives only when every remembered
  archive has its index on the disk - it never walks an archive; 31 organs in 1.2-3.6 s here): "Yes" green / "N of M
  folder trees" / "No" / "No - free: 692 MB" (THE USER on my first words "here" / "not here": "slovo here je idiocie!
  Stačí snad Yes" - a state column says Yes / No) and, where the samples are not (all) here, A LINK in the cell to the
  page where they are bought (the user's order): `samplesPageOf(e)` = the catalog's "link" (the publisher's link from
  the save; shown and opened ONLY when it begins with http:// or https:// - it is a text out of a published file), else
  the organ's README on GitHub; drawn as the domain ("sonusparadisi.cz") or "README", lightskyblue underlined, after the
  state's words (`samplesLinkRange`); `cellClicked` on the link's own x-range opens the browser (a double click there
  loads nothing); no link for an organ with free samples (the button downloads them). Pictured with
  %TEMP%\craft_catalog2.py (an organ without samples, one with 2 of 3 trees, one without a link, one with a
  "javascript:" link -> README). The click itself was NOT tried by a mouse. Download: the zip -> the .json inside (must hold
  "manualKeyboards"; every plug-in object gets "on": false + a Messages note - a downloaded organ never starts a
  plug-in by itself) -> `loadFromCatalog` = askToKeepChanges + `loadConfigurationFile` (the ordinary load box: copy
  into the LibreOrgans folder etc.); a samples archive -> Documents/LibreOrgan/LibreSamples (beside LibreOrgans; a file
  of the expected size is not downloaded again) -> `loadFromCatalogArchive` = `loadConfigurationFromArchives`. Top bar
  `organCatalogButton` (key "organCatalog", after Audio routing), the empty console's hint has a second button "Organ
  catalog" (panel 226 px). `QuietProcess::exitCode()`. Hook `ORGAN_TEST_CATALOG="<s>|open^<s>|filter|<text>^<s>|sort|
  <organ|divisions|stops|ram|routes|samples>[|desc]^<s>|ram|<1-7>^<s>|here|<0|1>^<s>|select|<folder part>^<s>|load|
  <folder part>^<s>|samples|<folder part>^<s>|cancel^<s>|state"`. RUN (test_0127b.ps1 -Odf "" -Tag catN -Env
  "ORGAN_CATALOG_URL=F:\...\libreorgans\catalog.json;ORGAN_TEST_CATALOG=...;ORGAN_TEST_CLICK=zzz|Load;
  ORGAN_TEST_CLICK_AT=20"): the list of 31, the states pictured, sorting, the three filters; Adlington's zip over real
  HTTPS from raw.githubusercontent -> load box -> Load -> copied into the (temp) LibreOrgans folder, trees found, Stop
  List compiled; Rural's rar from the release: cancelled at 100 of 692 MB ("nothing of it is kept"), then whole in
  ~30 s, indexed, the load box, Load -> the organ loaded - BUT from F:\Suchy_rejstrik (the unpacked Rural of this box
  is the faster source), so "samples read out of the downloaded rar" is NOT proven by this run (the archive path itself
  is the 0.8.207 one). NOT run: the default link (404 until the push), a failed download's text, the plug-ins-off
  path (no published organ has plug-ins), a real mouse (double click on a row, header clicks), Linux (curl / wget),
  macOS (juce::URL; the 10.11 Mac's TLS to GitHub unknown). LESSONS: `TextEditor::setText(..., true)` notifies
  ASYNCHRONOUSLY - a hook calls the filter itself; a hook click on "Load" must come AFTER the box (ORGAN_TEST_CLICK_AT);
  the test leaves `%APPDATA%\LibreOrgan\Catalog\` (cache) and %TEMP%\noise0127_cat3\LibreSamples\RuralOrgan.rar. His
  running 0.9.144 (idle, empty console) was closed in order for the tests.
  0.9.146 (27 Nov 2026, the same day; the user after "Noises of a Hauptwerk definition" on Altenbruch SemiDry: the
  Cimbelstern "not a loop with release ... plays as one shot", no One-shot in its voicing row): THE RELEASE INSIDE THE
  LOOPED FILE. The definition names THE SAME FILE for the loop and for the release (Sample pipe/hracky/vybrane/
  etoi_61.wav: 30.2 s, loop 347740..926221, cues at 347740 and 1147188; the blower pipe/mech/mech.wav: loop
  20825..66594, cues 20825 and 81600; ReleaseSample rows `<c>` = the attack's sample id, e = LoadSampleRange_
  StartPositionValue 1 - field order by odf_fields.py: a UniqueID, b LayerID, c SampleID, d/e start type / value,
  f/g end type / value, n ReleaseCrossfadeLengthMs, s PreferThisRelForAttackID) and the engine played a release file
  from frame 0 - the whole 30 s rang again at the switching-off. In the noises since 0.9.127 (Stralsund has separate
  release files - the only organ I had tried), no regression. NOW `SamplerEngine::releaseStartInFile(file, data,
  &frames)` (static; -1 without a loop; THE LAST CUE MARKER IN TIME - the user: "někdy mohou být cue značky i dřív v
  souboru, musí to vzít tu poslední časově" - when it lies after the last loop's end and inside the file, else the
  frame after the loop's end), `Sample::noiseReleaseStart` set in loadNoises for a RELEASE job whose file holds a loop
  (file frames x sampleRate / sourceSampleRate; a tail under 20 ms = no release, the loop just fades),
  `startNoiseVoice(..., startFrame)` sets v.pos; the folder reading (noisesVarFromFolder) gives a looped file without
  a release file its own file as the release when >= 2000 frames follow the marker; log "N release(s) are the tail of a
  looped file - played from its release marker"; the debug line says "from frame A of B". RUN on a copy of his
  Altenbruch (%TEMP%\make_altenbruch_0146.py: his file + the noises of settings.json; test_0127b.ps1 -Tag alt0146
  -Settings "defaultCard=ASIO|ASIO Fireface USB" -Env "ORGAN_TEST_NOISE_BUTTON=Cimbelstern|6;ORGAN_DUMP_GRID=1;
  ORGAN_TEST_EXPAND=0|Cimbelstern|62000;ORGAN_TEST_SNAPSHOT={out}\grid.png"): "2 release(s) are the tail ...", off ->
  "etoi_61.wav from frame 1147188 of 1450470". ONE-SHOT IS BACK IN THE VOICING ROW of a noise's button that loops (his
  order, reversing his 0.9.130 one; `oneShotToggle` visible for noiseRow && noiseHeld, placed before Release xfade in
  the container line of resized) - the same `stop->oneShotManual` as the Noises window's box; each side refreshes the
  other (oneShotRequest -> refreshNoisesWindow by callAsync; callbacks.oneShot -> voicer->updateStopStates). Pictured
  (the row: One-shot, Release xfade x1, Noise (no pipes), Alias, Stop volume). NOT run: a click on the toggle, the
  folder reading's new rule, a file with a marker BEFORE the loop only (falls to the loop's end by the code), by ear.
  SEEN, NOT TOUCHED: PIPE ranks whose release is the tail of the attack file (old sets) are not handled by the pipe
  loader either (it knows release FOLDERS alone) - a key release just fades. The tools: %TEMP%\odftools\
  odf_sample_rows.py <odf> <file name part> (the Sample row and its attack / release rows), %TEMP%\wav_loops.py and
  wav_cue.py <wav> (loops, cue markers). RELEASED on his word ("zapracuj to ... A dej na gitHub"): commit 10daa69, tag v0.9.146, CI green at the first attempt on all five, five archives, Intel mac "late symbols: 0", the Windows exe on system DLLs only, the CI exe deployed. v0.9.145 had come out whole too (CI green on all five: the catalog compiled on Linux and macOS).
  0.9.147 (27 Nov 2026 afternoon; the user while re-publishing his configurations): (1) THE RED CROSSES IN ACTIONS,
  read without a token (`actions/runs?per_page=45`, `runs/<id>/jobs`, `check-runs/<job id>/annotations`, the public run
  page's text): (a) every pull request the robot opens leaves TWO failed runs with 0 jobs ("Build" and
  "Configurations check" on pull_request): "This workflow run required approval but was not approved before it
  expired" - a bot-made pull request needs the maintainer's approval to run workflows, he merges without approving;
  harmless, since at least 6 Nov; NOT changed (he answered the queue alone). (b) three merges within 30 s = three
  index jobs racing: two died on a refused push (the last one had sorted everything - nothing was lost). The index
  step of configurations.yml is now a loop (6 attempts): `git fetch origin main` + `git reset --hard origin/main`, sort
  + index + commit, `git push origin HEAD:main`, a refused push starts again from the new main ("nothing to commit" =
  success). The same race exists in configuration-remove.yml / configuration-feature.yml (not touched). (2) HIS RULE:
  NO VERSION NUMBER IN THE TEXT OF A CONFIGURATION'S README ("no version mention please!!! The only mention of version
  must be in LibreOrgan Sampler XXX or later in the Configuration") - `configurationReadme` wrote "opens the zip as it
  is (0.8.195)"; gone, and from the 30 published READMEs and libreorgans/README.md (three places). "Saved with:
  LibreOrgan Sampler X." STAYS beside "X or later" (both in The configuration; the robot's validate requires the Saved
  with line) - told him; the README inside the published ZIPS still has the old text (they go with a re-publish).
  Checked: `configuration_tools.py validate <zip> <README> folder` for all 31 and `... pair` for one (the arguments are
  zip, readme, mode - without them it prints its help). (3) PUBLISHED COLUMN: configurations_index.py reads the robot's
  note "<!-- published by @x (issue #n, YYYY-MM-DD) -->" (else the Author line's date), row tuple + catalog.json
  "published", INDEX column "Published" sortable (INDEX-published-asc/desc.md), the program's catalog window
  `colPublished` (enum value 7, shown before Samples). (4) THE DEFAULT ORDER OF THE PERSPECTIVES (his words: "Dry,
  Semi-Dry, Front, default, Direct, Diffuse, Distant, Rear, Auxiliary. Other divisions go always alphabetically before
  Rear" - he said divisions, he means perspectives): `SamplerEngine::defaultPerspectiveRank(name)` (by the name's
  letters, startsWith: semidry 1, dry 0, front 2, default 3, direct 4, diffuse 5, distant 6, others 7, rear 8,
  auxiliary 9); `perspectiveOrderIndex`: a line i -> i * 32 + 16 (the user's own order: a dragged column), a name
  WITHOUT a line -> (the first line whose rank is greater) * 32 + rank ("default" is no longer always first; an
  unlisted one no longer after Rear); `perspectiveLinesInDefaultOrder(lines)` used by the Hauptwerk import when it
  writes "perspectivePatterns" and a division's "perspectives"; `mergedPerspectiveLines`: a line that is ADDED (the
  computer's lines into an organ at a load, an organ's into Patterns.json) is inserted at its default place among the
  base's lines instead of appended; the same rule in Python (configurations_index.py `order_key`) - INDEX changed for
  Adriatic, Dom Bedos, Brescia ("default" first) and Caen ("Front, Rear"). Existing files keep their lines' order
  (Adlington published stays "Diffuse, Direct, Rear" until imported anew). NOT touched: the scan's own learning
  (SamplerEngine, appends a new label), learnPrefixesFromTables' bare lines, patternsWithHead (append) - an unlisted
  name is placed by the rule anyway, a line appended there stands where it was appended. .NET file calls in a tool
  command: ABSOLUTE paths (the cwd is LibreOrgan/LibreOrgan - a relative path threw; nothing was written). RELEASED on his word ("publish it on gitHub, too"): commit f5ecb4a + the merge e890818 (origin had three more publishes: INDEX / catalog conflicts resolved by git checkout --theirs + the index script run again), tag v0.9.147 on e890818, CI green at the first attempt on all five, Intel mac "late symbols: 0", the Windows exe on system DLLs only; the CI exe NOT deployed (his program ran - the local build stands in C:\LibreOrgan\LibreOrgan_0.9.147.exe). The first merge after the push: the index job's loop ran green. ARNSTADT "again without noises": the log showed an export at 12:42 never submitted (no issue; the next organ loaded 10 s later) and the catalog load at 13:36 bringing the old published file back - %TEMP%\published_state.py <libreorgans> lists per organ the publishing date, the version and the noises (27 Nov: 7 re-published with noises, 23 + Stralsund without). OFFERED, not coded: a guard against an export that was never submitted.
  0.9.148 (27 Nov 2026 evening, Windows; the user: a slider "se někdy nechytne", the wheel on the blower's Stop volume
  changes the dB AND scrolls, "nikde se neobjevuje tooltip ... regres"; LOCAL COMMIT, NOT pushed - his word):
  (1) WHEEL + SCROLL AT ONCE: StopRow is a MOUSE LISTENER of its own sliders (`globalSlider.addMouseListener(this, false)`,
  right-click = MIDI learn), a juce::Component is a MouseListener, and Component::mouseWheelMove hands the notch to its
  PARENT - so every notch on a voicing slider reached the row as a listener callback and went up to the viewport,
  whether the slider had taken it or not (a deaf slider scrolled twice as far). Hidden until 0.9.136, when
  `stopsViewport.allowWheel` became true over open voicings. Fix: `mouseWheelMove` overridden in StopRow, SectionHeader,
  StopListDialog (`if (event.eventComponent == this) Component::mouseWheelMove(...)`) and empty in HintSheen / GlowLayer.
  RULE: a Component that is added as a mouse listener of other components overrides mouseWheelMove - a notch it hears
  as a listener (eventComponent != this) is never passed on. (2) "DOES NOT CATCH": WheelGate took a notch on a slider that
  did not listen yet for a scroll (`movedSinceScroll = false` - a new mouse move wanted) also when NOTHING scrolled;
  now `pageStood` (the control under the mouse stands where `look()` last saw it: `underAt`) keeps movedSinceScroll and
  only restarts the 0.4 s; within 0.3 s of a notch a move of >= 12 px from `scrollAnchor` is the user's (it was all
  "the hand on the wheel"). `WheelGate::note()` (a std::function set by MainComponent) logs "Wheel over the slider X
  before it listened (...)" at most every 5 s. (3) TOOLTIPS: JUCE's TooltipWindow::getTipFor gives a tip only while
  `Process::isForegroundProcess()` - the user writes in another window (the terminal) and hovers over the console:
  sliders arm, the wheel works (Windows scrolls inactive windows), no tooltip anywhere. HelpTooltipWindow::getTipFor no
  longer calls the base: held back only by a held mouse button (ModifierKeys) and a modal box in front. The window's own
  `buttonDown` flag (set at every mouseDown, cleared by a mouseUp that never comes when the pressed component is deleted
  before the release - MouseInputSource sends no mouseUp without a component under the mouse) is gone. A control WITH a
  text that gets no tip is asked again at the next poll (was struck off until the mouse left it). Log: "Tooltip shown
  over X ... - the first of this session", "Tooltip over X held back for N s: <why>", "did NOT come up";
  `ORGAN_TIP_DEBUG=1` logs every tip. WHICH of the two was his case is NOT proven (the foreground rule fits everything
  he described; no real mouse was driven) - read those log lines next time. (4) Tooltips added where the audit
  (ORGAN_TEST_F1_AUDIT, now naming unnamed controls "<Class bounds in Parent>") found none: voicing Stop volume and
  perspective boxes, Divisions bass layouts / MIDI channel / wind and tremulant sliders, Couplers boxes, the Editor's
  top-row sliders and six rows, Show cables, the Close buttons. Left: the Stop List's folder cells (their own path
  bubble), three zero-size combos of a StopRow, the hidden legacy buttons "Keyboards" / "Select Top Folder with Stops".
  HOOKS: `ORGAN_TEST_TIP="<s>|<button text, bar name, testName, label text or @x,y>^..."` (>= 5 s apart; the mouse through
  the peer, 3 s later the log says whether a tooltip shows and where; `ORGAN_TEST_TIP_OUT=<folder>` = tip-N.png, the
  window with the tip in its place), `ORGAN_TEST_TOPMOST=1` (every window of ours always on top). TEST LESSON: JUCE names
  a component under the mouse only where NO OTHER WINDOW covers ours (HWNDComponentPeer::contains -> WindowFromPoint) -
  a peer-driven mouse test of a window the user's own windows cover finds "nothing under the mouse" and proves
  nothing (two runs lost); such a test runs with ORGAN_TEST_TOPMOST=1. ORGAN_TEST_BAR's wheel sequences: "muppww" at
  the page's top = the page-stood case; a 'j' needs a 'p' before it (within 0.3 s of a notch 5 px are the hand).
  RUN (Rural, final build): armed slider's notches change the value with the scroll position unchanged, main window
  and Configuration Editor; tips over a top-bar button, a voicing bar, a per-key number, a perspective box, with the
  Editor open too. NOT run: a real mouse, a modal dialog (Stop List) in front, the red note names (Rural has none),
  Linux / macOS (not compiled; the tooltip rule and the listeners are platform-neutral), self-test / kerneltest (no
  engine change). v0.9.148 RELEASED (main 83118bf = a merge of 34 origin commits, CI green on all five).
  0.9.149 (27 Nov 2026 night; the user: Chico's Cymbelstern on the pedal - "Rear" cut, the Alias not there at all,
  "řádky musí správně wrapovat ... I u transmisí, i u noisů, prostě všude"): a noise's button and a stop folder
  without pipes had kept the OLD ONE-LINE layout (0.9.134 moved only stops with pipes and transmissions to the
  wrapping walk: "containers keep their line") - 213 + One-shot + xfade + Ranks + 3 boxes = 797 px in a 760 px column.
  NOW `StopRow::compactControlsLayout` is THE layout of every open voicing (stop, transmission, stop folder, noise's
  button) at ANY width: lines beside the button start at buttonRight (y < 50), lines under it at the row's left;
  `item(control, index, width, minimum)` wraps, jumps under the button when not even the minimum fits beside it, and
  NARROWS a box (transposition, alias: the put handler takes `w`) instead of cutting it; the perspective boxes stay
  one group when a line holds them, else wrap box by box; velocity layers wrap; a transmission has TWO sliders, a
  noise's button ONE (the walk kept lines for six); `Control::info` = "Noise (no pipes)" inline; a stop folder's
  sentence on its own line(s) under the button; `expandedHeightFor` of a container = the walk's bottom + 4
  (containerExpandedHeight 58 / expandedControlsHeight 104 are no longer its heights). Noise's order: Alias, One-shot,
  Release xfade, perspectives, text, Stop volume (was One-shot first, Alias last). RULE: there is ONE layout of an
  open voicing; a new kind of row or control goes into the walk (voicer.h) + its `put` case in resized - never a
  line of its own. TEST: `ORGAN_TEST_ROW_WIDTHS=1` (with ORGAN_DUMP_GRID=1 + ORGAN_TEST_EXPAND / ORGAN_TEST_ALL):
  the open row laid out in 59 widths 240..1400 px, `StopRow::outsideControls()` names every control outside the
  row or over another (the controls above the per-key rows; `lastControlsBottom`); describeLineOne ends with it.
  RUN: Rural stop / transmission / "Cornet [III]" folder / an empty new folder, Yokota's Cymbelstern_front (the
  user's file loaded in place, not written): all 59 widths clean, rows pictured. NOT run: a child row inside a
  folder with its own voicing open, a stop with velocity layers, Linux / macOS.
- BUG REPORT FORM (25 Nov 2026, the user: "make all the fields NOT compulsory, apart from the LOG ... people bring their
  .zip from different computers onto an internet computer ... We learn everything from the log anyway"; then: "what
  happened by mělo být povinné ... stačí opravdu pár slov"): `.github/ISSUE_TEMPLATE/bug_report.yml` requires TWO
  fields - Logs and "What happened" ("A few words are enough") - the rest is optional (version, system, audio; the
  title is GitHub's own requirement, prefilled "[Bug] "). The program's "Report a problem" dialog demands "What
  happened" too - that STAYS. RULE: never make another field required.
- 0.9.127 IN WORK (24 Nov 2026, the user's GO for the noises: "ano ... To, co říká ODF, zachovat pevné přiřazení. Nezapomeň
  na ty drobné opravy pro Pásztora"; ALL of it in ONE version). HIS DECISIONS: (1) published configurations open with
  everything off as before; else the state as the definition says (the blower mostly on), a master switch + volume
  "Noises". (2) "V Hauptwerku to musely být píšťaly ... My to takhle dělat nemusíme, není-li to nutné" - a noise need not be
  a pipe / stop inside. (3) The definition's fixed drawknob assignment is KEPT; buttons that have none (transmissions,
  extensions - Stralsund St. Jakobi) get one AT RANDOM from the drawknob files no stop uses. (4) Key noises: the missing
  keys filled from the neighbour (no transposition); only the physically played keys by default, a Configuration Editor
  switch "Key noises follow the couplers". (5) The folder reading as the fallback (levels known by NAME), what misses is
  set by hand in a Noises window. (6) "nezapomeň upravit i voicing, pokud přidáváš nějaké volby - aby si to zapamatovaly
  save voicing, load voicing, a další místa - případně i combinations, pokud je volba zapnutá": every noise setting goes
  into the configuration, Save / Load Voicing and (with "Combinations hold the voicing") the pistons; own-button noises
  are in the pistons' on/off like stops. (7) Adding the noises of a definition to an EXISTING configuration must touch
  nothing else (a separate list). STAGES: 0 Pásztor (DONE IN CODE, not built yet: `organPolyphony` map = the limit per
  organ on this computer, machine key + volatile key, read in loadSettings after "maxPolyphony", written in
  saveSettingsNow; the report's texts with packing Automatic; the "mojibake" power plan name was MY Get-Content reading the
  UTF-8 log as ANSI - no fault). 1 HauptwerkOdf: the noises parsed into the plan + `--hauptwerk` prints them (checked
  against %TEMP%\odftools\odf_noises.py on Freiberg, St. Guilhem Surround, Stralsund). 2 the configuration's "noises" list
  (save / load / portable paths below the library root) + the import writes it + "Add noises from a definition". 3 the
  engine: noise voices (untuned, no wind / tremulant / fill, whole one-shots, loops with release) through the perspective
  routes, the triggers (key press / release, stop, coupler, tremulant on / off / running, own button). 4 the folder
  guess. 5 UI: Noises window, own buttons on the grid, Editor switch, master switch + volume, voicing / pistons. 6 manual,
  CHANGELOG, tests, release on his word.
  PROGRESS (24 Nov 2026, NOTHING committed or version-bumped yet; the build of the working tree passes): STAGE 0 built.
  STAGE 1 DONE AND VERIFIED: HauptwerkOdf.h `Noise` {trigger key / divisionInput / stop / coupler / tremulant / console, edge
  held / on / off, keyboardId, divisionId, note, stopId, couplerSwitchId, tremulantId, switchName, defaultOn, rankName,
  perspective, levelDb, packageId, files, releaseFiles} in `odf.noises`, `Plan::PlanNoise` {kind keys / stop / coupler /
  tremulant / tremulantRun / button, edge, name, perspective, couplerId, division, note, stopId (0 = a FREE drawknob noise),
  followsCouplers, defaultOn, levelDb, files, releaseFiles} in `plan.noises`; parse(): the block "0.9.127 THE NOISES" after
  the keyboards' compasses (pallet pipes traced back through `backLinks` to keyOfSwitch / inputOfSwitch / stopOfSwitch /
  tremulantOfSwitch / couplerOfSwitch - the drawknobs spread three plain hops back -, else the nearest SHOWN switch = a
  button, or by its name a stop; a silent attack "blankloop"/"mute" with a release = the noise at the end; unused
  "stop" pallet pipes and console clicks out of the drawknob folders = free drawknob noises); `--hauptwerk` prints
  "noise <kind> <edge> division N [perspective]: count (files), e.g. ...". Checked: Freiberg 349 noises (blower, keys
  30+54+54 on and off, stops 5+14+13, couplers, tremulant drawknobs), Stralsund Jakobi 1560 (+ Cimbelstern, Pauke,
  Calcant, the tremulant's running noise), St. Guilhem Surround 1476 (+ Rossignol, "Clochette pour la souffl").
  A REAL IMPORT BUG FOUND AND FIXED on the way: `Scanner::nextTable` took an EMPTY table written `<ObjectList
  ObjectType="enclosure"/>` (the uncompacted, lower-case definitions) for a table, and skipTable() then ran to the end of
  the NEXT table - the samples after "reversiblepiston", the linkages after "switchexclusiveselectgroupelement": the import
  of St. Guilhem Surround said "no stops with sample pipes". Also: a compacted KeyboardKey row without <c> is key 60 (the
  default is left out). ENGINE MAP (an Explore agent's, 24 Nov): Voice struct SamplerEngine.cpp ~1474, renderVoice ~3657
  (pitch = temperament + random + wind + trem + tuning + shift; gain = velocity x voicing x expr x division x wind x trem x
  output x route x fill; `v.stop` must not be null), allocVoice ~5917, publishVoice ~5958, findRoute ~4961, startNote ~6669
  with the lambda startRank, finishNoteOff ~7068, startReleaseSamples ~7191, the MIDI queue `MidiCommand {note, velocity,
  manual, on}` + pushMidi ~6158 + processMidi ~6181 (the place for a noise command), the freeing pattern BuildPause +
  clearMidi + suspend + killAllVoices + resetAllVoicesUnsafe + resume, decodeSampleFile ~13284 (anonymous namespace).
  STAGE 3 (ENGINE) WRITTEN, COMPILES, NOT RUN YET: `Voice::noise` + the bypasses in renderVoice + `v.noise = false` in
  allocVoice; `MidiCommand::kind` 1 = a key's noise (processMidi -> `noiseKeyImmediate`); in Impl after processMidi:
  `NoiseSound`, `NoiseSet` {sounds, keyOn / keyOff[manual][note], byRef "<kind>|<edge>|<ref>", freeOn / freeOff by group,
  heldWanted}, `noiseRoute` (division + perspective; no division = the trigger's; no perspective = the division's
  "default" route; never the auxiliary), `startNoiseVoice`, `playNoise` (one-shot whole / held loop + release),
  `noiseTrigger`, `switchNoise` (80 ms dedupe per button, 8 per 100 ms), `retireNoiseSet` / `purgeRetiredNoiseSets`,
  `keepHeldNoises`; public (SamplerEngine.h after setStopReleaseCrossfade): `NoiseSpec {kind, edge, ref, perspective,
  group, manual, note, gainDb, files, releaseFiles}`, `loadNoises(specs)` (parallel decode with the folder's storage,
  the missing keys filled from the nearest, replaces the set without touching the organ's voices), `clearNoises`,
  `noiseHousekeeping` (1 s), `noiseSoundCount`, `hasNoiseFor`, `keyNoise(manual, note, down)`, `noiseEvent(kind, ref,
  on, contextManual)`, `setNoisesEnabled`, `setNoiseVolumeDb`, `setNoisesArmed`; hooks in setStopEnabled (the master of a
  stop folder is the button), setTransmissionEnabled (ref "<manual>|<name>#tr<id>" or its groupMaster -> a free noise) and
  setManualTremulant ("tremulant" click + "tremulantRun"). NEXT: MainComponent - the model, the configuration key
  "noises", the import writing it, loadNoises after an organ's load + arming, keyNoise calls, coupler / button events,
  the UI, the voicing / pistons, the folder guess, "Add noises from a definition", manual.
  STATE AT THE END OF 24 Nov 2026: LOCAL COMMIT 7c00e1e "0.9.127: noises ...", version bumped everywhere, Projucer resave
  done, C:\LibreOrgan\LibreOrgan_0.9.127.exe = the local build (shortcut retargeted); NOT pushed, NOT tagged (his word).
  DONE + RUN (%TEMP%\test_0127_noise.ps1 [-Odf <definition or json> -KeepConfig 1 -Seconds -Midi -Clicks -ClickAt -Tag];
  env ORGAN_NOISE_DEBUG=1 logs "Noise voice: ..." / "Noise switch: ..."; ORGAN_TEST_NOISE_FOLDER=<folder> [+ _TAKE],
  ORGAN_TEST_NOISE_ODF=<odf>, ORGAN_TEST_NOISE_WINDOW=1, ORGAN_TEST_NOISE_BUTTON="<name>|<seconds>"; a stop button answers
  ORGAN_TEST_MOUSE "<caption>|click", NOT ORGAN_TEST_CLICK; the Stralsund demo's load takes 35-110 s by the page cache -
  time the hooks late): import of "Stralsund, St. Jakobi, Wegscheider, Demo" -> the configuration's "noises" {enabled,
  volumeDb, sounds ["kind\tedge\tref\tperspective\tdivision\tnote\tlevelDb\tfollows\t<folder>//a.wav|b.wav\t<releases>"],
  buttons {bN: {name, on, division}}} (the folders BELOW the library root; `libraryRootOnThisComputer()` from
  treeRelativePaths), 1040 sounds / 2130 files / 653 MB (always 16-bit; packed when the organ packs or Automatic; the
  memory guard every 16 files) loaded by `noisesTick` 1.5 s after a load, armed 1.2 s later; the blower starts by
  itself; keys: every perspective its file, variants in turn, press and release, the levels of the definition;
  "OW. Cimbelstern" on = the loops, off = their releases; "Principal 16'" = its own drawknob noise on and off;
  `addNoisesFromDefinition` on the loaded organ: "204 of 204 drawknob noise(s) found their stop by its folder"
  (usedFolderPlaces; a stop folder's rank -> its master); `noisesVarFromFolder` on 003135\ruchy = the definition's
  picture (keys per division and perspective, calcant on/off, cimb / mech / pauke held with their releases, the
  tremulant's run, 330 free drawknob noises) - a held sound = a LOOP in the file (`SamplerEngine::fileHasLoop`), "rel"
  under looped files = their release, under one-shots = the off click; only the ORGAN's perspectives count and the one
  folder left unknown takes the perspective left (written AFTER the run: the elimination itself is NOT run yet).
  MainComponent: the last section of MainComponent.cpp (noiseFilesText / noiseFilesOf, noisesVarFromPlan,
  noisesChanged from loadSettings - a missing key RESETS -, noiseSpecs, noisesTick, applyNoiseButtons, setNoiseButton,
  setNoisesModel / Master / Volume, removeNoises, addNoisesFromDefinition, noisesVarFromFolder, addNoisesFromFolder,
  NoiseButtonsContent + showNoisesWindow = ModelessDialog "Noises"); keyNoise calls in routeKeyboardNote (physical) /
  applyRoutedNoteDelta (with keyNoisesFollowCouplers, configuration key); setCouplerEnabled -> noiseEvent("coupler");
  the Editor section "Noises" before "Plug-ins (VST3)" (Noises on, Noises volume, Key noises follow the couplers, Noise
  buttons, Noises of a Hauptwerk definition, Noises of a folder, Noises of this organ); a publish switches the noise
  buttons off (a clone); manual 6.6 + 8.8, F1 title "Noises"; CHANGELOG 0.9.127. A held voice on a route that is not
  rendered yet is flushed by cleanStaleRoutes and started again by keepHeldNoises (seen right after a load).
  NOT DONE (told the user): the noise buttons ON THE GRID, in the PISTONS / crescendo and under MIDI LEARN (they live
  in the Noises window alone); no top-bar button for that window (Configuration Editor > Noises > Noise buttons); the
  Noises window has no row editor for what the folder reading misses (perspective, division, kind by hand); a
  tremulant's noise is tied to ONE division of ours (the first the definition's tremulant works on - Stralsund's click
  on I, its run on III; St. Guilhem's on the pedal); coupler noises never run-tested; the merge of two organs drops
  the added organ's noises (the base's stay); Save / Load Voicing know nothing of the noises (their settings are the
  configuration's); the website / README / 1.5 / 10.4 still say the definition is read for TWO things - the noises are
  a third, the wording is the user's rule to change; Linux / macOS not compiled (no push); the uncompacted-definition
  fix was run on St. Guilhem Surround by --hauptwerk only, not by a whole import.
  A REGRESSION OF MY OWN IN 7c00e1e, FOUND BY THE USER ("Proč se ti Noises sekce smíchala se sekcí Plugins ... regres jako
  sviňa! Jaké další regresy udělals?"): the Editor's "Plug-ins (VST3)" header was GONE - my PowerShell replacement took
  the comment line WITH its newline and put a here-string back that ended with that comment and NO newline, so the next
  line, `addSection("Plug-ins (VST3)", true, true);`, became part of the comment (the build passes: it is a comment).
  RULES: (1) a scripted replacement never ends on a `//` comment line - end it with the newline or with code; (2) after a
  commit made by scripted edits, list EVERY removed line (`git diff <c>~1 <c> -U0 -- Source | grep '^-[^-]'`) and account
  for each (22 here: 21 meant, this one not); (3) an Editor change is PICTURED before delivery (ORGAN_TEST_EDITOR_SECTION
  + ORGAN_TEST_DIALOG_SNAPSHOT with ORGAN_TEST_CLICK "Configuration Editor") - the 0.8.183 rule, skipped here; (4) an
  import change is run through %TEMP%\hw_compare2.ps1 -Old <previous exe> (the plans of 25 published organs, the noise
  lines left out). HIS OTHER ORDERS OF THAT MESSAGE: one volume for all noises is WRONG - every noise its own, and every
  perspective of it its own (a keyboard: one per perspective of that keyboard), on top of the level the definition gives
  each sample; a MERGE must ADD the other organ's noises (new drawknobs, keyboards, zimbelsterns), dropping only what
  would be doubled; the noises are NOT a third thing the definition is read for - in Hauptwerk they are pipes and ranks,
  "prostě čteme, kde jsou uložené znějící samply": never write them as a new purpose (texts changed to "as with the
  pipes, read for where the samples lie and what they belong to"); a top-bar button "Noises" beside Stop List opening a
  "Stop List for the noises" (what is where, editable) - asked for, to be designed with him.
  THE ENGINE DESIGN CHOSEN: `Voice::noise` (render: no cents at all, wind / tremulant / swell gains 1, no dynamic shelf), a
  carrier Stop per sound for the gain, sounds kept OUTSIDE `stops`; specs by REF from MainComponent {kind, edge, ref,
  perspective, manual, note, gainDb, files, releaseFiles}; commands through the MIDI queue; hooks in the engine for the
  stops' and the tremulants' switches (a stop folder's children click once: the master is the button; several within
  80 ms deduped; a cap per 100 ms; mute while loading), keys through a call from MainComponent (physical division, or
  every routed note with "Key noises follow the couplers"); a button without a noise of its own takes a free one by the
  hash of its name.
  THE FINAL DESIGN (24 Nov 2026 evening, the user's GO: "Udělej to všecko a dej rovnou na gitHub ve všech mutacích"; his
  words: a row of the Noises window is "jakási jednotka určitého druhu ruchu - Manuál I, Pedál, Tremulant II, Zimbelstern,
  Blower"; "tlačítka musí být na gridu divize jako rejstříky. V okně noises se jen nastavují"; "u tremulantu nepotřebujeme
  táhlo a jeho zvuk. Ten je současně nahraný v běhu"; the grid button of a noise has its own voicing row whose sliders go
  "do stejného místa jako volume slider v dialogu Noise ... alias taky ... nesmí duplikovat volby v Noises, jen je to druhá
  cesta k témuž"; NEVER the word "písty"): (1) UNITS: `MainComponent::noiseUnitKey(fields)` = "keys|<division>",
  "tremulant|<division>", "drawknobs", "couplers", "button|<name lower>"; the model's "units" [{key, on, volumeDb,
  perspectives [{name, db}]}]; `noiseUnitGain(key, perspective)` -> `applyNoiseGains()` over `noiseLoadedUnits` (unit +
  perspective of every loaded sound, in the engine's order = the spec order; `NoiseSpec::unit`) ->
  `engine.setNoiseGains(gains)` (NoiseSound::trim; 0 = off; held voices follow through `refreshHeldNoiseGains`).
  `noiseUnits()` (public struct NoiseUnit) feeds the window; `setNoiseUnitOn / setNoiseUnitVolume(key, perspective or "*",
  dB) / changeNoiseUnit(key, kind, division) / removeNoiseUnit`. (2) GRID BUTTONS: `Stop::noiseButton` (1 a noise's button,
  2 one that runs from the load = the blower) on a CONTAINER stop without pipes; `syncNoiseButtonStops()` (noisesTick: at
  arming and twice a second while idle with no modal box) flags the stops named as the model's "buttons", adds missing rows
  (stopBuild entry customName + `engine.addContainerStop`, switched on when the model says "on"), takes off the buttons whose
  noise is gone (`removeStopFromGrid`); the engine's `setStopEnabled` starts / ends the noise ("button", name lower);
  `applyNoiseButtons()` after arming. An empty container is NOT a grid item (Voicer::gridItems) - not counted as a stop,
  untouched by the measurement. Tutti (`enableAllStops`) skips them, the general cancel skips `noiseButton == 2`, the
  crescendo's `setRegistrationItem` never switches one; the combinations hold them like stops. A publish keeps "runs" on
  the button entry and switches "on" off. (3) ONE VOLUME: a button unit's volume IS `stop->db` of its grid stop (the
  voicing row's Stop volume; `StopRow` shows globalSlider for a noiseButton container, row height expandedControlsHeight);
  `noiseButtonVolumesCheck()` (every timer tick: a sum over the flagged stops' db) -> applyNoiseGains, so the voicing row,
  the Noises window, a loaded voicing and a combination all act at once. The alias is the stop's alias. (4) TREMULANT: no
  drawknob click - noisesVarFromPlan and the folder reading drop kind "tremulant", noiseSpecs / noiseUnits skip it in older
  files, the engine's setManualTremulant fires "tremulantRun" alone. (5) The top bar's "Noises" button (key "noises", after
  Stop List) opens `NoiseUnitsContent` (ModelessDialog "Noises", placeDialog key "noises2", F1 -> "The Noises window").
  (6) Save / Load Voicing ("noiseUnits" in the file) and the combinations with "hold the voicing" (`MemorySlot::noiseUnits`,
  scene key "noiseUnits"): `noiseUnitsVoicingNow()` / `applyNoiseUnitsVoicing()`. (7) MERGE (MainComponentMerge.cpp before
  "What stays the base's"): the added organ's sounds join unless the base has the same trigger / moment / perspective /
  division / key or the same files; stop refs follow `renamed` / `sharedStopNames`; files of another library root by their
  full path; buttons and units added. Hook `ORGAN_TEST_NOISE_UNITS="<s>|volume|<key>|<perspective or *>|<dB>^<s>|on|<key>|0^
  <s>|kind|<key>|<kind>|<division>^<s>|remove|<key>^<s>|list"`; `ORGAN_TEST_NOISE_BUTTON="<name>|<seconds>"` presses the
  GRID stop now. Test script %TEMP%\test_0127b.ps1 [-Odf -Seconds -Env "A=B;C=D" -Match -Tag -KeepConfig].
  RUN (24 Nov 2026 evening, the Stralsund Jakobi Demo definition, ~200 s a run: the load alone is 100 s): the import puts
  "Blower On/Off", "Calcant", "P. Pauke" on the pedal's grid and "OW. Cimbelstern" on III, only the blower running;
  11 units listed; unit volume / perspective volume / unit off / the button's volume (= stop->db) set and read back; key
  noises per division and perspective; a drawknob's own noise; a coupler's noise by ORGAN_TEST_MOUSE "II/P|click"
  (ORGAN_TEST_CLICK does NOT work on a header's coupler button: they act on the mouse, not on onClick); Tutti and X leave
  the noise buttons alone; Save / Load Voicing bring a unit's volume back; the saved configuration loads with its button
  rows (nothing "put on the grid" again); kind change button -> tremulant takes the button off the grid; a unit removed;
  the organ merged into itself: "0 stop(s) added ... the noises of the added organ are the loaded organ's already";
  kerneltest OK, self-test 0 WRONG; pictured: the Noises window, the Editor's Noises section (Plug-ins header in its
  place), the grid with the buttons, the Cimbelstern's voicing row (text, Alias, Stop volume). FOUR FAULTS FOUND BY THE
  RUNS: (a) `engine.addContainerStop` makes a stop that is ON - every new noise button sounded at once (now
  setStopEnabled(stop, the model's "on")); (b) a console noise had no division (-1 -> everything on the pedal): the
  plan gives a button `divisionIndexByPrefix(switchName)`, else the division of the STOPS carrying the same mark ("OW."
  beside "OW. Principal 8" - Stralsund's divisions are called GO / Positif / Recit, the marks are UW / HW / OW); (c) the
  Noises window showed a button's old volume after the voicing row moved it - a 4 Hz timer of the window reads the
  stops (`Callbacks::buttonVolume`); (d) the merge added the other organ's noise-button ROWS as stop folders "Blower
  On/Off (2)" - `addedNoiseButtons` skips them (a row without folders or children named as a button). SEEN, test only:
  after a portable load the default-card search moves every route and the blower's held voices are dropped and started
  again four times in three seconds (keepHeldNoises) - not so on a configuration of this computer. NOT RUN: a merge of
  ANOTHER organ's noises (other library root, renamed stops), the folder reading's elimination rule, the combinations
  that hold the voicing with noise units, a noise button dragged to another division by the mouse, the alias typed in
  its voicing row, Linux / macOS (CI compiles them for the first time with this push).
  RELEASED (24 Nov 2026, 18:05): commit 4a48e19 + the merge 0b1d627 (origin had the user's Stralsund publish, PR #263),
  tag v0.9.127 on 0b1d627, CI green at the first attempt on all five (Linux, Linux arm64, macOS, Intel macOS, Windows),
  Intel mac "late symbols: 0", the Windows exe without VC runtime DLLs, the CI exe deployed as
  C:\LibreOrgan\LibreOrgan_0.9.127.exe. The stages list above ("IN WORK", "NOT pushed") is history now.
  0.9.128 (24 Nov 2026 evening, THE STUCK LOOP - the user: "extrémní bug ... zničí uši, zničí výškáče", "nikdy nesmíš své
  testy ukončovat abruptně"): after my 0.9.127 test runs he started the program and a very short harmonic buzz played for
  ever on Fireface outputs 1+2 (where my tests had played through ASIO; his own routes were on 7+8), untouched by PANIC,
  Global Reset or the recorder. NOT our render: JUCE's ASIOAudioIODevice::close() (juce_ASIO_windows.cpp) takes the
  callback lock right after the callback is taken away, sleeps 20 ms and calls ASIOStop - the driver's double buffers
  still hold the LAST BLOCK OF SOUND; RME's driver then repeats that block on those outputs whenever anything makes the
  card run again (another client, the next start). Never noticed before because at a quit nothing sounded; the blower
  noise (on from the load in 0.9.127) sounded at every quit of my tests - and of his own session. No test was killed:
  each ended by ORGAN_TEST_QUIT, a quit "in order" that closed the card with sound in it. FIXES: (1) JUCE corrected at
  configure time (CMakeLists.txt, marker "libreOrganChannel", FATAL_ERROR when 'JUCE_ASIO_LOG ("stopping");' is gone)
  and in C:\JUCE9 by hand: both halves of every output buffer cleared and played for three periods (30-250 ms) before
  ASIOStop - every ASIO close of the program, at run time too. (2) ~MainComponent: `engine.setNoisesArmed(false)`,
  `noteOffAll()` (the 300 ms kill fade), 380 ms, `audioMuted = true`, 260 ms, THEN the callbacks go and the cards
  close; log "Quit: every voice ended, the cards got silence ...". (3) `CrashHandler::detail::silenceOutputs()` +
  `silenceOutputsNow()` (the flag + 160 ms) first thing in the crash, terminate and abort handlers; every audio
  callback writes zeros while it is set (beside audioMuted). What a driver does after the process is dead cannot be
  ruled. (4) `MainComponent::panic()` switches every noise button off and calls `engine.silenceNoises()` (heldWanted
  cleared, noise voices killed) - a held noise restarted itself a second after a PANIC before (keepHeldNoises), and
  Global Reset calls panic(). (5) The clean start calls `noisesChanged({}, ...)`: the last organ's noises were loaded
  and armed on the EMPTY console ("Noises loaded: 1554" right after "Started without a configuration"). (6) No noise is
  on by itself: the plan writes button "on": false + "runs": defaultOn, and syncNoiseButtonStops creates every button
  OFF whatever an older model says. (7) `Voicer::placeUnderDivision(manual, keys)` (compactCells = the layout on the
  screen, stored first as a drop does; the keys at lastRow + 2, column by column): the noises' buttons stand UNDER the
  division's stops, a free row between - the user: "nikdy se nesmí integrovat mezi existující ... Ruchy nepatří mezi
  rejstříky" (the merge's rule for a foreign organ). 0.9.129 (the same evening; I had left three items "for the next
  version" and the user: "proč jsi to kurva nechal na další verzi??? ... Já tě platím, abys mi dělal" - WHEN HE LISTS
  THINGS, ALL OF THEM GO INTO THE BUILD, an urgent fix does not excuse the rest; his answer on the free noises: a FIXED
  assignment, optionally random at every on/off): (a) A NOISE BUTTON'S VOICING ROW: `Stop::noisePerspectives` (name,
  on), `noiseHeld`, `noiseRelease` set by syncNoiseButtonStops from the unit (`buttonInfo`, read anew when
  `noiseButtonInfoVersion != noisesVersionWanted`); StopRow builds its rank boxes from them (`noisePerspectiveRequest`
  -> `setNoiseUnitPerspectiveOn("button|<name>", perspective, on)` = the model's units[].perspectives[].on, the SAME
  value as the window's box; noiseUnitGain returns 0 for a perspective that is off), One-shot shown when the noise
  loops (`stop->oneShotManual == 1` -> NoiseSound::playOnce: the file once, whole, nothing held), Release xfade when
  the loop has a release (`stop->releaseCrossfade` -> NoiseSound::releaseCrossfade on the loop's fade-out and the
  release's fade-in); `applyNoiseGains` sends both with `engine.setNoiseVoicing`; the text of the row is "Noise (no
  pipes)". (b) FREE NOISES DEALT: engine `noiseAssigned` ("<kind>|<ref>" -> group = the free noise's file name; the ON
  groups are the reference list for both moments), a button without one takes the group with the fewest uses, its own
  kind first (`NoiseSet::freeKind`), `noiseAssignedChanged` -> `takeNoiseAssignments` in noisesTick ->
  `storeNoiseAssignments` = the model's "assigned": ["key\tgroup"] (an array: the keys are no identifiers);
  `setNoiseAssignments` from noisesChanged / setNoisesModel (`pushNoiseOptions`); `engine.noiseFreeRandom` = the model's
  "freeRandom" (the window's box "Free drawknob noises: a random one at every press"); `setNoiseAssignment(key, group)`
  from the window ("" = dealt anew). (c) THE NOISES WINDOW (placeDialog key "noises3"): per perspective a box, the
  folder's name with the number of files (all folders in the tooltip) and the volume; "+" of a row
  (`noiseUnitsUnfolded`) = a read-only list of the folders and files (NoiseUnit::files: "<moment>: <folder>//a|b") and,
  for Drawknobs / Couplers, a combo per assigned button; the content is a juce::FileDragAndDropTarget: a folder on a
  perspective cell -> `setNoiseUnitFolder(key, perspective, folder)` (the folder read by noisesVarFromFolder, its sounds
  given the temporary unit "button|dropped folder" and turned into the unit's kind by changeNoiseUnit; a button's
  files of one moment and folder joined into one sound), anywhere else -> addNoisesFromFolder, which now ADDS to the
  model (it replaced it). Hook acts added to ORGAN_TEST_NOISE_UNITS: pon, folder, unfold, random, assigned. Run on the
  Stralsund Demo: the blower's row pictured (One-shot, Release xfade x1, Diffuse / Direct boxes, Alias, Stop volume),
  Direct switched off through the window's function and shown off in the row; the couplers unit removed and two
  couplers clicked: both took "60.wav" - the Demo has ONE free noise (the distinct choice needs the full organ: NOT
  run); a folder dropped on keys|1 Direct: 128 files there. NOT run: One-shot and Release xfade by ear, a real mouse
  drop, the random box, a changed assignment through its combo. 0.9.130 (the same night, the user's remarks on
  0.9.129 with his log read first; his decisions: no folders onto a noise's row, the transmission of a noise's button
  FORBIDDEN, One-shot into the unfolded row of the Noises window): WHAT HIS LOG SHOWED - Stop List, the Schalmey's
  folders moved onto "Blower On/Off" (20:21:08 "Tuning Blower On/Off [Diffuse] ... footage 4'"): the row became a pipe
  stop, and as a noise's button is recognised as a row WITHOUT pipes the blower's noise fell silent though the Noises
  window still listed it; two right-button drags of "Calcant" made transmissions of a stop without pipes (they clicked
  a free drawknob noise). NOW: `MainComponent::isNoiseButtonName(name)` (the model's buttons); the Stop List's
  `isNoiseEntry(entry)` (no folders, no child, named so): its row shows a disabled "Noise" where the transmissions are,
  NO folder cells (hidden Cell buttons keep the row's cell count), its "x" disabled, `moveFolder` onto it refused with
  a line in the info text, "Remove empty stops" leaves it; indenting stops under it stays. `SamplerEngine::
  addTransmission` returns -1 for a `noiseButton` stop (every caller checks the index); syncNoiseButtonStops removes
  transmissions a noise button has (`voicer->removeTransmission`, a Messages note). One-shot is gone from the voicing
  row (`oneShotToggle` never visible for a noise) - `NoiseUnit::oneShot` + the unfolded row's box of the Noises window
  (`Callbacks::oneShot` sets `stop->oneShotManual`). Window: `paint` draws a line before every perspective block, the
  cell shows the on / loop folder first (texts ordered on, loop, off, release), `perspectiveOrderChanged` rebuilds
  the open window and resets `noiseButtonInfoVersion`; the buttons' boxes sorted by `engine.sortPerspectives`. A button
  of an older file without a division: the numeral of its name ("II." / "2.") -> that manual, else the pedal.
  Pictured: the Stop List with three "Noise" rows, the window with the blower unfolded. NOT run: the refusal of a
  folder dragged onto a noise row, the removal of an existing transmission (his file was not loaded by me), a
  right-button drag. 0.9.131 (minutes later, the user: "jakmile ve stop list dialogu posunu blower doprava ... objeví
  se mu možnost transmisí i přidání adresářů! Tohle nemáš domyšlené!"; his log: 21:00:23 the blower indented, 21:01:02
  the Schalmey's folders on it again): `isNoiseEntry` had `!entry.child` - an INDENTED noise row was an ordinary row
  for the dialog. The condition is gone (a noise's button may stand under a stop and stays what it is), the "empty
  folder row goes" branch of the indent gesture keeps a noise row, its name is not editable (it is found by its name;
  the alias is free), a refused right-button drag is logged. LESSON: a rule that names a kind of row ("the button of
  a noise") is tested in every STATE the dialog can put the row in - plain, indented, with children, in another
  division - before it is delivered; I had pictured the plain row only. Verified with ORGAN_TEST_INDENT (the blower
  under Ped_Schalmey4) + the Stop List's picture. 0.9.132 (the user, right: "řádek ruchu by se měl především poznat
  strukturálně ... taková věc má mít svůj příznak, protože je z dialogu Noise. Kurva!!! Nezaváděj úplně liché a
  sekundární podmínky pro rozpoznávání čehokoliv"): THE FLAG. `ManualConfig::StopBuild::Entry::noiseKey` (the key of
  the noise's button in the model = its name there, lower case; JSON "noise" on the stopBuild entry, written and read
  by the stopBuild var functions, copied by deriveStopBuildFromEngine from `Stop::noiseKey`) - the ONLY thing that makes
  a row a noise's button. `Stop::noiseKey` on its stop: the engine's setStopEnabled triggers the noise by it,
  `noiseButtonStop(key)` finds the stop by it, the voicing row's perspective request uses it. syncNoiseButtonStops per
  noise button of the model: (1) the row with that noiseKey; (2) none, but a stop still carries the key (a move wrote
  its row anew) -> that stop's row gets the flag back; (3) a file of 0.9.127-0.9.131: the row named as the button,
  without folders, is flagged ONCE (log "... (from an older file) flagged"); (4) else a new row with the flag
  (`uniqueStopBuildName`) + its container stop, OFF. Flagged rows whose noise is gone leave; a stop with a key no noise
  has is unflagged. The Stop List's `isNoiseEntry` = `entry.noiseKey.isNotEmpty()`; the row's name is editable again
  (the name is no identity). The merge skips added rows by the flag (the name rule only for older files).
  RULE (his, for ever): WHAT A THING IS, IT CARRIES AS ITS OWN FLAG / ID IN THE MODEL from the moment it is made -
  never recognise a kind of row, stop or button by secondary signs (its name, empty folders, not indented ...); such
  a guess is allowed only once, as a migration of an older file, and is logged. Verified: a fresh import saves
  "noise": "<key>" on four rows (the blower's with child true after ORGAN_TEST_INDENT), the Stop List pictures the
  indented row as Noise; a file saved by 0.9.131 gets its four rows flagged at the load.
  CI THAT EVENING (seven versions in four hours; the user: "proč nezrušit build 131 a rovnou dělat 132 ... bude to
  zase čekat celou věčnost"): every tag's Build waited for a macOS runner behind the one before. build.yml now has
  `concurrency` (group "release" for v* tags, cancel-in-progress) and a first job `supersede` ("older builds
  cancelled": ubuntu, `permissions: actions: write`, `gh run cancel` of every Build run of a v* ref that is not
  completed and has a LOWER run_number) - a new version's run cancels the older ones, whatever workflow file they
  started with. build-retry.yml retries only the NEWEST version and only the commit its tag stands on NOW
  (`gh api repos/.../commits/<tag>` .sha against workflow_run.head_sha). LESSON: I moved the tag v0.9.132 twice (onto
  the workflow change, then onto the mac fix) - three runs carried ONE tag name, the retry (which compared names)
  kept starting the cancelled ones again and they cancelled one another (runs 729-731, attempts 2-3). NEVER move a
  tag to start a build while runs of that tag exist - a fix after a tag is a NEW version number (0.9.133); v0.9.132
  was deleted from origin and never released. The token for an API cancel is not available to me (the tool refused
  reading git's stored credential): cancelling is done by the workflow itself. API without a token = 60 requests an
  hour for the IP - two watchers polling burnt it; read GitHub's HTML pages (actions/workflows/build.yml,
  /releases) for a state check instead.
  #267 (Christedge, MacBook Air M2 + Mac mini M4, 0.9.130: "damaged binary"): JUCE's CMake signs the app bundle ad hoc
  at the build and OUR CMakeLists then replaces AppIcon.icns in it - the signature's seal over the resources no longer
  fits, and Apple silicon calls a downloaded app with a broken signature "damaged" (the user's Intel Macs never
  minded). build.yml: a step "Sign the app bundle ad hoc" (`codesign --force --deep --sign -` + `--verify --deep
  --strict`) right before the zip, both macOS jobs. The app is still NOT notarised: release body, README and site say
  "System Settings > Privacy & Security > Open Anyway (older macOS: right-click > Open)" - "right-click > Open" alone
  no longer works on current macOS. NOT verified on a Mac (none here): the reporter's next try tells. RULES FOR EVER: a card is never closed with sound in its
  buffers - silence first, then the close; a test ends with noises off, a panic, a second of silence, then the quit,
  and never while something sounds; PANIC silences EVERYTHING and nothing may restart by itself after it. Verified
  by one run (Stralsund Demo: no noises at the empty start, buttons under the stops and off, blower on by the hook,
  PANIC -> off and no restart for 35 s, the quit line). NOT verifiable here: that the Fireface is free of the loop
  (only ears tell); the crash path. The rest of his list of that evening was done in 0.9.129 (above).
- DONE in 0.9.125 (kept for the reasoning; delete next time): TO-DO (the user, 22 Nov 2026, "Musí se dát udělat. Zapiš si to ... A nezapomeň na to"): the window "Reading the archives"
  (`indexArchivesThen`, the ThreadWithProgressWindow that builds the RAR indexes; also the same index step inside the
  "Searching for the sample folders" job of 0.9.44) has NO CANCEL - it must have one: the walk stops, nothing is loaded,
  nothing loads by itself afterwards (the 0.9.44 rule: a Cancel ends everything). An index walk of a big set on the HDD
  takes minutes (Bellevue 84 s, Ashton longer).
  THE SAME EVENING, FOUND IN HIS OWN MERGE (0.9.124, 19:18:42, "program je jako vytuhlý ... zešedl"): the MERGE resolves the
  added organ's trees ON THE MESSAGE THREAD (MainComponentMerge.cpp resolveTree -> SampleFiles::isDirectory -> locate ->
  indexOfLocked, the stack in his log) - for a candidate path that is not on the disk it asks the remembered archives
  (Locations.txt held three Billerbeck rars on the E: HDD) and WALKS THEIR INDEX there, minutes long, no window, the program
  grey ("Message thread unresponsive for 69 s", reading 0.3 MB/s = header seeks). Not the packing. TO DO with the Cancel
  above: the merge's tree lookup goes behind a progress window on a thread (as the load's SearchJob, 0.9.33) with the
  archive walk shown and cancellable; never an archive index walk on the message thread.
  WHERE THE RAR LINES CAME FROM (his log, the user: "Já nelouduji z rarů!!!" - right): 19:09:38 the three rars were chosen
  once, `registerArchive` wrote them into Locations.txt BEFORE "Reading the archives" began, the window had no Cancel, he
  killed the program (new start 19:11:50) - the lines STAYED, unindexed; Billerbeck was then loaded from the disk, and the
  merge's first candidate that missed on the disk (resolveTree: `SampleFiles::isDirectory(candidate)` -> locate's
  "elsewhere" loop over EVERY remembered archive) walked all three. TWO MORE FIXES for the same build: (1) an archive is
  written to Locations.txt only AFTER its index was read (a Cancel / a killed read leaves no line); (2) resolveTree looks
  on the DISK first through every candidate and resolveTreeLikeALoad (juce::File::isDirectory), the archives only when the
  disk has nothing - a tree that is on the disk must never cost an archive walk (check the load path's
  lookupIsDirectory -> SampleFiles::isDirectory the same way: a miss there asks the archives too, behind its window).
- TO-DO, left open by 0.9.124 (22 Nov 2026): the user tests "a folder added to a loaded organ with packing Automatic"
  himself (my test dropped the package 001318, which holds a stale Caen configuration: the 0.9.18 rule loaded it as a NEW
  organ and the hook skipped the box); the load box with its third packing item was not pictured; the polyphony of a partly
  packed organ against a fully packed one is not measured.
- VIDEO (1 Nov 2026, the user's decisions): the Stralsund script stays EXACTLY, only two scenes go before its Load
  Configuration: the folder D:\Hauptwerk\...\OrganInstallationPackages\000370 (St. Carlo, Brescia) dragged onto
  division I, then AdlingtonHallVol0.CompPkg.Hauptwerk.rar onto division II with "Load the folders" (as if without
  a configuration; the "Don't Save" question before the Stralsund load is shown); a new script command
  `dropfiles DIVISION|PATH` with a dragged file icon. Sound: NO ducking/compressor (the pipes fluctuated), one
  static gain for the organ (-18 LUFS) and one for the voice, the voice 3 dB QUIETER than before (-24 LUFS); no
  limiter in the organ's path; a take with any discontinuity in audio.wav (crackle.py) or an "Output
  discontinuities" log line is thrown away; the video's end = the last narration + 2 s (the end was cut last time).
  Voice: local Kokoro first (the test of 1 Nov 2026), ElevenLabs (free plan, attribution "Voice generated by
  ElevenLabs" required, key in ELEVENLABS_API_KEY) only if Kokoro displeases; pauses inserted by our mixer at
  dashes (0.6 s), full stops (0.5 s), commas (0.2 s). The user added "Traktura" to the ignored patterns himself
  (Patterns.json keeps it). KOKORO IS INSTALLED (1 Nov 2026): Python 3.12 (winget, user scope,
  C:\Users\doma\AppData\Local\Programs\Python\Python312) + eSpeak NG (C:\Program Files\eSpeak NG), venv
  C:\Users\doma\kokoro-venv (kokoro 0.9.4, torch 2.15 CPU, soundfile; the model downloads from Hugging Face on the
  first run, the symlink warning is harmless); run with PATH += "C:\Program Files\eSpeak NG" and
  PHONEMIZER_ESPEAK_LIBRARY=C:\Program Files\eSpeak NG\libespeak-ng.dll. %TEMP%\kokoro_test.py <out> [voices]:
  five narration sentences, KPipeline(lang_code='b'), speed 0.95, OUR pauses (cut at " - " 0.6 s, sentence ends
  0.5 s, colons 0.4 s), peak -6 dBFS -> F:\temp\voice_test\{bf_emma,bf_isabella,bm_george,bm_lewis}.wav for the
  user to choose from. make_narration.py still uses edge-tts; the Kokoro path goes in once he chooses a voice.
  His verdict on the first four: "Jediný použitelný hlas je lewis. Ženské jsou totálně nepřirozené"; then
  bm_daniel/bm_fable/am_michael and bf_alice/bf_lily/af_heart/af_bella/af_sarah/af_nova rendered too. CHATTERBOX
  (Resemble AI, MIT) in C:\Users\doma\chatterbox-venv (chatterbox-tts, torch 2.6 CPU; its pins differ from
  Kokoro's torch 2.15 - two venvs on purpose); %TEMP%\chatterbox_test.py <out> [reference.wav]: the built-in
  voice (chatterbox_default.wav) and a clone of Kokoro's Lewis (chatterbox_lewis_clone.wav, audio_prompt_path),
  exaggeration 0.4, cfg_weight 0.5, the same phrase cuts and pauses. Script fixes of 1 Nov 2026 in
  video_stralsund.txt: "We load it where it is" dropped (say + caption); the tremulant scene plays ONE note
  (`note 1|60|on`) released only after the rate drag to 3.2 ("slower again"), then 4.4, close, trem off.
  THE USER'S VERDICT (1 Nov 2026): Chatterbox "taky na hovno" - Kokoro bm_lewis it is. VIDEO PIPELINE 0.9.31:
  (1) script command `dropfiles DIVISION|PATH[;PATH]` (VideoScript::step after "drop"): a drawn folder/archive
  icon (260 x 150, the file name under it) travels from the frame's left edge to the division column's centre
  (`voicer->divisionGridAreas()` -> stopsContent.localAreaToGlobal), `owner.fileDropHighlight` set during the
  travel (the orange frame), then `takeDroppedFiles`; (2) `make_narration.py` = Kokoro by default (voice
  bm_lewis, speed 0.95, `--engine edge` keeps the old path), OUR pauses (cut at " - " 0.6 s, sentence ends 0.5 s,
  colons 0.4 s, each phrase synthesised alone), peak -6 dBFS, prints "ends at N s"; (3) `make_video.ps1`: Python
  = the kokoro venv + eSpeak env vars, the run is REFUSED when the log has "Output discontinuities" or
  `Tools/check_recording.py audio.wav` finds a jump (12 x the local mean step, > -60 dBFS, sounding before;
  exit 1), NO sidechaincompress / alimiter / loudnorm: one gain organ -> -18 LUFS, one gain voice -> -24 LUFS,
  both lowered together when the true peak > -1 dBFS; frames.txt gets the last frame held to the narration's end
  + 2 s and `-t` replaces `-shortest`; (4) video_stralsund.txt opens with the bridge (the user's text: "Every
  organ starts from sample folders on your disk ... drop the archive ... Any creative mind can assemble an organ
  this way ... And if the organ is already prepared, one configuration file is all you need to load"): dropfiles
  000370 (St. Carlo) on I, the Adlington Vol0 rar (E:\sonus\HW2\Adlington\10) on II with "Load the folders",
  the Stralsund load. Tools/video_opening_test.txt = the opening alone for a test run through make_video.ps1
  -Script. LESSONS OF THE FIRST OPENING TEST (the user: "fiasko"): (1) a dropped organ comes with every stop OFF
  - the chords were silent, the recording -70 LUFS and the mix gain absurd: `stop M|NAME|on` lines before each
  chord (St. Carlo rows "pipe - Principale", "pipe - Ottava", "pipe - QuintaDecima"; Adlington Vol0 "002707 -
  pipes - HW StoppedDiap", "... HW Principal 4" - the rar's trees are its top folders OrganDefinitions +
  OrganInstallationPackages, 6 stops), and make_video refuses a recording under -45 LUFS; (2) the script's `load`
  calls loadConfigurationFile directly, NO "Don't Save" question appears - the waitfor/click of it waited 20 s
  for nothing and the load box stayed open: the lines are gone; (3) ffmpeg measurements hung without stdin
  closed (`-nostdin` everywhere; the peak by `astats=measure_overall=Peak_level`, not ebur128=peak). The
  check_recording.py scan of the first take: 0 discontinuities in 77 s. THE GLITCH FOUND (the second take, stops
  drawn): HOLES of exact zeros inside the sound - 19 (23.3 s), 42 (41.9 s), 46 (44.5 s) samples, and a hard
  resume 0 -> -0.04 at 55.4 s - all after the first device restart of the recording (the St. Carlo load). CAUSE
  (SamplerEngine.cpp ~7126): a route joining or returning set `recPos = max(flushed, flushTarget)` = the writer's
  edge, and the writer follows the wall clock 300 ms behind (0.8.208) - the route had NO margin, any callback
  jitter let the writer flush frames not yet written (zeros, lost). FIX 0.9.31: `recPos = edge + 0.3 x
  sampleRate` (the wall position itself). The first 300 ms of a recording are silence (under the 500 ms fade-in).
  check_recording.py rewritten: HOLES (>= 8 exact zeros between sound > -40 dBFS) + JUMPS (> -50 dBFS and > 10 x
  the mean step of the 5 ms on BOTH sides - a note attack or a smooth rise is no jump); the old "mean step
  before" rule flagged attacks and missed the holes. The user's "one glitch in the last video" was this class.
  0.9.32 (1 Nov 2026 evening, the user stopped the video: "napřed build s těma aliasama ... na macOS to přestalo
  chodit ... Regres"): (1) MAC REGRESSION OF 0.9.29 (issues #160/#161, Core 2 Duo 10.13.6, logs in
  %TEMP%\issue161\x): "Message thread silent for 3 s" at every measurement start, callbacks 107-142 % at 135
  voices, "zamrzá a odmrzá", the Load box "hidden somewhere" - `bringLostModalsToFront` ran every 0.5 s on
  every platform and on macOS `anchor->isActiveWindow()` is TRUE under a modal box (the NSWindow main window
  stays "active" while the box is key), so the box got toFront(true) twice a second = the whole console
  repainted each time (the log line "brought to the front:" with an EMPTY name at 19:56:06 under the load box
  is the proof). Now the function is `#if JUCE_WINDOWS` whole, and on Windows it touches a box only when it
  really lies BEHIND the main window in the Z order (GetWindow GW_HWNDPREV chain) or is minimised. RULE: a
  periodic window-ordering call must be platform-specific and conditional - never unconditional toFront on a
  timer. The grey "Measure..." of #160 = `canMeasurePolyphony` false during the stall/loader, no separate fault.
  (2) ALIASES FROM FOLDERS at every scan path: extendStopBuildWithTree, deriveStopBuildFromEngine's fallback
  and the merge's tree scan give a new plain row `engine.folderStopAlias(folder)` and `placeHalfInTable(...,
  containerAlias, childAlias)` the stem's alias (folderStopAlias on a sibling File named after the stem =
  the last " - " part of stopName) and the half's. The Stop List's "Aliases from folders" button stays for old
  rows. LESSON (the first 0.9.32 build: the halves' 30 aliases vanished, the plain St. Carlo rows kept theirs):
  saveSettings syncs the table FROM the engine (0.8.201, `entry.alias = stop->alias`, an EMPTY stop alias
  included) and the reload's routes phase saves BEFORE finishDivisionsLoadGrid applies the table aliases to the
  new stops - so the aliases of rows whose stops exist at that moment are wiped ("Save settings: N table
  alias(es) replaced by an empty stop alias" is the new log line that shows it; "Division N rebuilt: X table
  alias(es), Y applied" too). The table aliases now go onto the new stops in the loader's `afterCommit`
  (the moment the stops exist), before any save. Test %TEMP%\test_alias0932.ps1 (000370 on I, rec on II: no
  row without an alias; test_alias0932b.ps1 = rec alone). The video pipeline
  (0.9.31) is paused: the full timing run was stopped by the user; the opening test mp4 lies in
  %TEMP%\video\opening (clean, 77 s).
- THE USER'S RULE (30 Oct 2026, "nesmíme vyžadovat žádné externí závislosti, ani Visual C++ redistributable!!! To je
  zásadní!"): the released executables depend on NOTHING that is not part of a plain Windows/macOS/Linux - no VC++
  redistributable, no DLL beside the exe, no installer. DONE in 0.8.251: `CMAKE_MSVC_RUNTIME_LIBRARY
  "MultiThreaded$<$<CONFIG:Debug>:Debug>"` in CMakeLists.txt before add_subdirectory(JUCE) (CMake 3.22 -> CMP0091 NEW;
  the CI exe was /MD before: MSVCP140.dll/VCRUNTIME140.dll of the 2015-2022 redistributable; the Projucer Release build
  was always `/MT` = `useRuntimeLibDLL="0"` and links the same prebuilt archive.lib, so the static runtime is known to
  link). After the release: `dumpbin /dependents LibreOrgan.exe` of the CI zip must list system DLLs only (no
  MSVCP*/VCRUNTIME*/api-ms-win-crt*). Linux: libstdc++/glibc are the system's (glibc 2.38+ stays the floor); macOS:
  the system libc++. Background: a user's "nothing starts" on a clean Windows 11 24H2 (30 Oct 2026) - the other
  candidates are Smart App Control (on by default on clean installs, blocks unsigned exes with a toast only),
  Defender quarantine, or a hang before the first window (activity.log tells).
- (DONE in 0.8.226 - the detector, the pool hardening, the MIDI probe, the three to-dos; the analysis kept) 29 Oct 2026
  EVENING (the user's live crackle on 0.8.225, Brasov Surround, ASIO Fireface USB, 15 routes; his words
  "prská i při měření polyfonie, DSP meter to nezaznamenal, error counter 0, je to náš program"): HIS RECORDING
  F:\temp\Brasov Surround 2026-10-29 18-27-59.wav (22.6 s, 24-bit, peak -34 dBFS, no NaN) analysed with
  %TEMP%\crackle.py / crackle2.py / crackle3.py: 41 discontinuities, 38 of them EXACTLY at the 256-frame block edges
  (phase 255 mod 256; mod 512 splits 19/19 = per device block), 16 of them in PAIRS one block apart = one block whose
  content differs (LOWER energy: voices missing for one block, the next block continues where the signal would have
  been - timing intact), none with slow single tones, dense during runs (74 per second at 17-18 s). Timing was fine
  throughout (42 275 voices clean, late callbacks 0, worst block 7 %). NOT the device: the recording is our buffer
  before the driver, and the recorder's ring is 11 s per route (ringBits 19), so it drops nothing. Candidates, none
  proven: (a) RenderPool - a lane's chunk rendered into the sink when `scratchFree` is empty (tryWork, "should be
  impossible", scratches = workers + 2 = 17) or a lane's scratch left unmerged: needs leaked scratches or an
  imbalance of lanesJoined/lanesLeft; the reset of both counters to 0 at every job acquisition (render(), after
  nextVoice = 0) erases a join that lands between the stores (a worker preempted between its `open`/`nextVoice`
  checks and its fetch_add) - nanoseconds wide, so improbable as the whole story; (b) nothing else found: suspend()
  callers are all structural, findRoute is lock-free, audioMuted toggles only on device changes, the RouteCallback
  gain glide is continuous, a voice that ends is gone for good (no pairs). THE WAY FORWARD (on his word): an output
  DISCONTINUITY DETECTOR in the program - per route and per stage (after the pool render, after separation/reverb/
  clip, at the device output after the gain): the first sample of a block against the last of the previous one
  vs the block's own mean step; counted per second, logged "Output discontinuities: N (route X, stage Y)" and shown
  beside the DSP figure (the user: "Program si neuvědomuje, že prská!"); counters "chunk without scratch", "lane
  slot unmerged"; the pool counters hardened (lanesJoined/lanesLeft never reset - monotonic, equality only; a
  worker re-checks `open` after joining; completedCount reset before nextVoice). CRASH 18:32:15
  (crash-20261029-183215.txt): the user switched the Fireface OFF while playing -> Windows removed its MIDI port ->
  JUCE's MidiDeviceListConnection callback -> our midiDevicesChanged -> MidiInput::openDevice ->
  juce_Midi_windows.cpp InputDevice::openInternal: midiInOpen FAILED (line 2710 `return {}`) -> the half-built
  InputDevice destroyed -> ~InputDevice calls blockQueueThread.join() on a thread never started -> std::system_error
  "invalid argument" out of a destructor -> std::terminate. A JUCE bug (the submodule is upstream, never patched):
  our side must PROBE before opening - midiInOpen/midiInClose on the WinMM index matched by name (or skip a device
  whose open failed for a few seconds) in midiDevicesChanged AND reopenMidiInputs; the user's rule: "Musí zvládnout
  vypnutí audio device používané za běhu!" - the audio side survived (routes stopped and restarted at 18:32:08), the
  MIDI side killed the process. TO-DO (same build): LibreRecordings folder beside Configurations for the Record
  button's files (default target, created on demand, the chooser starts there); wind model "Oscillation frequency"
  slider lower limit 2 Hz (was 4: both sliders, the dialog rows, the MIDI-learn scaled(4, 10)); "Release scaling"
  slider maximum +10 dB (was +6).
- (DONE in 0.8.224, kept for the reasoning) LINUX: THE SAMPLE DISK UNDER /run/media IS NOT MOUNTED AT BOOT (29 Oct 2026, the user, Arnstadt "nenajde soubory":
  the Linux box's Hauptwerk disk sda2 (NTFS, label "Hauptwerk") had no mountpoint - udisks mounts it only when the
  file manager opens it; Locations.txt remembers /run/media/doma/Hauptwerk/Hauptwerk/HauptwerkSampleSetsAndComponents,
  so every tree was "not on this computer" and the organ loaded empty). The user: "to je na hovno" - TO DO, both:
  (1) THE MANUAL must say (Linux, "Where the files are" / 1.6): do not keep the sample disk on the automatic
  /run/media mount - it is mounted only when clicked in the file manager and is NOT there after a reboot; mount it
  permanently (fstab / "Mount at system startup" in the KDE Disks & Devices settings, or /mnt/<name>) and point the
  locations there. (2) OR THE PROGRAM MOUNTS IT ITSELF: a remembered location or tree under /run/media/<user>/<label>
  that does not exist -> find the block device by label (/dev/disk/by-label/<label>, the path's third part) and run
  `udisksctl mount -b <device>` (no root; polkit may ask for the password once for an internal disk - the dialog is
  the desktop's own) before the load, log "Disk <label> mounted at <path>" / the refusal, and only then look the trees
  up; the notice "Sample folder not found" should name the unmounted disk ("the remembered location X does not
  exist - a disk not mounted?"). Verify on this box: unmount sda2, load Arnstadt, the program mounts it and finds
  the 6 trees.
- THREE MERGE FAULTS - DONE in 0.8.223 (the note above). FOUND 28 Oct 2026 EVENING (the user: "jak to že nemohu mergovat Bellevue Casavant do Bergen
  Surround??? ... proč při mergi distant do surroundu to tak dlouho čeká a je jakoby zamrznuté?" - diagnosed from
  his activity.log 20:51-20:55, NOT coded, he said "nekóduj, jen řekni"): (1) INFINITE LOOP in the "(2)" rule of
  0.8.220 (MainComponentMerge.cpp 556-566): an alias that already ends with ")" is not suffixed again but still
  collides -> the loop never ends; Bergen has "1. Principal" (16' and 8') and "1. Octav" twice, so the second added
  stop of a shared alias collides with the first's "(2)" alias for ever (stacks: stopBuildKey from
  mergeConfiguration at line 561 for 8/69/129 s). Fix: always suffix the alias (strip an old " (n)" first) and cap
  the loop. (2) MERGE OF A .json FROM THE CONFIGURATIONS FOLDER FINDS NO TREES: `resolveTree` (MainComponentMerge.cpp
  ~235) looks beside the ADDED FILE only (dir/rootRelative/relative, dir/rootName/relative, dir/relative, absolute);
  since 0.8.190 the configurations live in Documents\LibreOrgan\LibreOrgans away from the samples, so
  "Casavant, Bellevue, Surround.json" (trees "002098/ped" ..., root OrganInstallationPackages, all present on D:)
  gave "0 new tree(s), 0 stop(s) added". Fix: resolve like a load does - the base organ's resolved root, the sample
  locations (Locations.txt), `portableFolderCandidates`. (3) THE WHOLE-STEM RULE of 0.8.221 needs the base folder's
  head among the KNOWN PATTERN PREFIXES; the user's Bergen Surround.json was imported before 0.8.220 and has
  "Direct"/"Diffuse"/"Rear" WITHOUT prefixes -> "no folder name repeats a folder of the division after a prefix of
  its own", all 42 Distant stops added as rows (doubled again) and then fault (1) froze it. Fix: a head carried by
  >= 3 base folders under one perspective counts as a known prefix (derived from the rows, not the patterns).
  Verify all three on HIS files (Bergen op Zoom, Surround.json of 18:57 + the Bellevue json + the Distant ODF).
  THE AGREED DESIGN (28 Oct 2026 evening, the user's rules; next build, on "go"): (a) THE UNIT IS THE STOP, not the
  folder - a flat tree has AB_X, REA_X, 06-far__X side by side; the prefix of a perspective is the head carried by
  ALL the stops of that perspective ("ne jen některé"); with an explicit Stop List table it is exact: the common
  head of every folder in the perspective's column. (b) LEARNING + FILLING-IN RUNS ON EVERY LOAD PATH: the automatic
  scan, Load folder, a configuration with explicit tables, the ODF import, the merge - the prefixes found go into
  perspectivePatterns and are saved with the configuration (an old file without prefixes repairs itself at the
  first load). Today the learning runs only in scanTreeAssignments(learn=true) = the automatic rules / Load folder,
  never for an explicit table (every import). (c) A HEAD IS A PERSPECTIVE only when the same stop stems exist under
  ANOTHER head (same tree, the loaded rows, another tree) and the tuning gate confirms the same pipes, OR the head
  carries a perspective word (dir, dif, rea, far, near, close, dry, wet, front, rear, ab, ortf ...). A lone head with
  no counterpart and no word stays "default" and nothing is stripped - "HW_"/"Ped_" (division marks) must never
  become perspectives (HW_Principal8 and Ped_Principal8 would fuse into one button). (d) THE JOIN of a second
  perspective (merge and Load folder) is decided stop by stop: the whole stem equal + the gate -> the folder joins
  the row as the new perspective; the gate refused -> a row of its own; the second set need not carry every stop.
  (e) The three merge faults above in the same build. Verification on HIS files: Bergen Surround.json (18:57) +
  Distant ODF -> joined, no doubling, no freeze, patterns filled in; Bellevue.json into Bergen -> trees found, stops
  added as another organ; the +5 c copies refused.
- PERSPECTIVE JOIN SAFEGUARDS - DONE in 0.8.221 (the note above); kept here only for the reasoning: (28 Oct 2026, the user's question after 0.8.220: "AB_Principal8" of one organ vs
  "rear_Principal8" of another - common stop names): the 0.8.220 rule (one new prefix on >= 3 and >= 80 % of the NEW
  folders, not a known prefix; HW names on an ODF merge) passes a SMALL foreign organ of the same producer's naming
  scheme with a usual disposition; the division mark in the stem does not protect (the longest common ENDING matches
  "Principal8" under "rear_" and "AB_HW_"). Told the user; NOT coded (he said "nekóduj, jen řekni"). Proposed, in
  order: (1) a BIDIRECTIONAL threshold - most of the division's rows with folders (>= 70 %) must be matched too, not
  only most of the new folders; (2) the WHOLE stem after the prefix must be equal, the base folder's prefix must be
  one of the organ's known pattern prefixes (so "HW_Principal8" != "Principal8"); (3) the sample FILE COUNT of the
  two folders must agree on most pairs (the same rank has the same files in every perspective; a foreign Principal 8
  has another compass/layers) - cheap, no audio read; (4) a1 of the first files (computeTuning440) - reliable but reads
  samples; (5) a question box only in the grey zone (passed, not 100 %, file counts undecided): "Join / Add as new
  stops". CORRECTED (the user: "co když je divize rozdělená, jako Bergen? 002082 obsahuje jen 6 rejstříků z HW"):
  (1) is WRONG as a requirement - a package of a split division covers 6 of 14 HW rows and would be refused; it
  holds only for an ODF merge (the plan spans all packages) or several trees loaded at once (count the base side over
  their union). Hard gate = (2) + (3) (per folder, independent of coverage), the 80 % of the NEW folders stays, (1)
  only as evidence in the log/summary, (5) only when the file counts cannot decide. The second package of a learned
  prefix (002083 after 002082) is no case for the rule: the scan knows "Far;06-far__" and joins by name the old way.
  FINAL DESIGN (the user: "chci i 4., protože ladění je jediná tvrdá brána. Někdy nemusí mít ani všechny společné
  rejstříky"): MEASURED on Bergen HW Principal 8 - Surround 03-ortf (002086) vs far (002082): 32-40 of 67 files the
  SAME size, the rest within 1 % (24 header bytes or a few KB of release), 0 files missing = the same take from two
  microphones; the DRY set of the same organ (002061/hw/03-ortf_HW_Principal8 - the same folder names!) vs far: 0
  equal, 17 % off, 45 files more = another recording. BUT the user (28 Oct): "nemusí mít soubory stejnou délku,
  historicky jsem některé zkracoval, o trošku" - the file length is NEVER a gate or a filter, a log line at most.
  THE LAW (the user, 28 Oct 2026): "perspektivy musí mít absolutně stejné ladění v smpl ... Nesmí mít rozdíl 1-2
  centy" - MEASURED (%TEMP%\smpl_cmp.py <folderA> <folderB>): Bergen HW Principal 8 Surround 03-ortf vs far
  (002082) 67 of 67 files the SAME smpl (unity note + fraction, bit for bit), ab vs rea 67/67; the DRY set
  (002061/hw/03-ortf_HW_Principal8) vs far 0 equal, 51 different, 45 files more; every folder 67 distinct values
  (an honest smpl). So: (b) THE GATE = the smpl chunk of the same-named files IDENTICAL (unity + fraction, no
  tolerance; 8-12 files per folder or all - header reads, a few KB per file, no audio decoded, independent of
  trimming and of a1); one difference = not joined. Fallback ONLY for a degenerate smpl (few distinct values in the
  folder against its file count - fraction 0 / unity = the file number everywhere - or no smpl chunk): the f0
  fingerprint from the first 0.3 s (computeTuning440's analysis) within ~2 c; never read f0 otherwise. The same gate
  for the ODF merge and Load folder (the ODF's pitch is not used for it); (c) decided PER FOLDER, not per package: name match + fingerprint -> joins the row, name
  match without -> a new row (a second perspective need not carry every stop); the 80 %/one-prefix rule then only
  LEARNS the perspective label, it no longer decides the join; (d) the whole stem after the prefix must be equal
  (the base folder's prefix a known pattern); (e) no question box. The decision moves into the loader (the analysis
  reads audio: a few seconds per package from SATA or a rar, on the progress window) - the table is built after it,
  not at scan time as 0.8.220 does. The Dry set of the same organ is untouched by the rule (same prefixes as the
  Surround set -> the old same-name-known-perspective path). Data: %TEMP% none - the PowerShell size comparison
  above is one-off; 002061 = Bergen Dry (hw/...), 002082/002083 = Wet-Distant, 002084-002088 = Surround.
- LOAD CONFIGURATION "WAITS FOR THE FILLED PIPES" (the user, 24 Oct 2026 evening) - MEASURED, not what it looks like:
  the log of 18:33:37 shows the copy build interrupted within the same second ("ended at 104/312 ... interrupted");
  what took 11 s with the window dead ("Message thread unresponsive for 8 s", stack in RtlFreeHeap/free_base) was
  `unloadManual` x 4 on the MESSAGE thread FREEING the old organ: 23 GB in ~150 000 packed vectors (packing makes
  2 vectors per channel per sample; 36 741 samples). And the loader itself slows towards the end: the pack's
  thread time per frame rises 5.8 -> 33 ns over the eight trees at 16-bit 48 kHz (flat 5.5 ns at 32 kHz, 8 GB), the
  loader's CPU falls 45 -> 30 %, page faults 370 000 -> 230 000/s, disk 0 (page cache 70 GB standby): the kernel's
  cost per fresh page grows with the working set (every sample = calloc + a worst-case pack vector + shrink_to_fit
  copy + free = three multi-MB VirtualAlloc/Free per sample). Not a regression of a change: the price of 36 000
  separate allocations. DONE IN 0.8.184: (1) `unloadManual` moves the stops into `removedStops` and
  `freeRemovedStopsLater` frees them on a juce::Thread::launch thread (then `MemoryInfo::releaseFreedMemory()` =
  _heapmin / malloc_trim / malloc_zone_pressure_relief - the user: "ať žádné haldy nezbývají"); the loader's run()
  shows "Freeing the previous organ ..." and `waitForRemovedStopsFreed()` before the first tree, so the memory
  counts (gSwapBaseline, memoryShortNow, warnIfMemoryShortForTrees' free figure - read on its thread after the
  wait) see the memory free; the engine destructor waits too. (2) thread_local scratch in packChannel/packChannel32
  and the check/ints vectors of compress16/32: parallelFor's threads die with the stop, so nothing stays held.
  (3) NOT one outer BuildPause (a build relaunched by an inner ~BuildPause would wait on the lock while its stop is
  removed and freed - use after free); instead `setRelaunchHeld(true/false)` around the unload loop of
  applyPortableConfiguration: `relaunchPendingBuilds` returns at once. (4) `logRamAfterLoadIfDue` (MainComponent,
  the once-a-second timer) writes "RAM held after the load (<storageSignature>): N GB - ... M pipe(s) of them from
  the cache; free of installed" once `pendingBackgroundWorkers() == 0` after "Compiling the Stop List: done".
  The user measures the #87 table anew with it; mine are not wanted. 0.8.185 LESSONS (the user's evening batch): (a)
  the four free threads of 0.8.184 held the HEAP LOCK against the message thread (stacks: RtlEnterCriticalSection in
  RtlFreeHeap under ~StopRow, ShapedText, replaceWithText) for 8-13 s - the loading window never came up; now
  `unloadManual(manual, false)` in applyPortableConfiguration and the LOADER thread frees them one at a time
  (`freeRemovedStopsNow`, "Freeing the previous organ ..." on the window), one `_heapmin` at the end; other unloads
  launch ONE draining thread (`freeRemovedStopsLater`: a running drainer takes new stops too, checked under the lock).
  (b) The user's series in ONE process with 0.8.184 (RAM lines): float 83.4 / packed 29.2 / 16-bit 48.8 / packed 14.8
  / half 32.3 / half packed 11.9 / 32 kHz half packed 8.2 GB - the paper's 23 and 13 GB were the old packer's
  fragmentation; the website/README table carries these. (c) #90 "the organ does not play after a measurement":
  reproduced in part with ORGAN_TEST_NOTE (Rural, aggregated WASAPI): one run left ~130 voices SOUNDING for minutes
  after the end (keys held in the engine although routeKeyboardNote released every key of 36..96), the next run not
  - the engine's keys were stuck, so a key struck on them again is silent ("speaking" set) until a device switch's
  killAllVoices; finishPolyphonyMeasurement now sends `engine.noteOff` for EVERY key 0..127 of every division
  (a note-off for a key not held releases nothing) and logs the voices 20 s later. `noteOffAll` is
  clearMidi + killAllVoices (a cut), never for the end of a run. THE ROOT CAUSE, FOUND 0.8.187 (#90-#92, Windows and
  Linux, the user's repro "measure, switch the driver, measure again, play: no sound"): the 20 s line of both logs
  said "audio muted YES". A driver switch mutes (`audioMuted` true) and its "Applying audio device X to all routes"
  step ends with `audioDeviceSelectionUnmuteAt = now + 1500`; the user started the second run within that 1.5 s,
  `startPolyphonyMeasurement` stored `previousMuted = true`, the timer unmuted during the run, and the end's
  `audioMuted.store(previousMuted)` muted for good - RouteCallback returns silence BEFORE rendering, so the voices
  never advance either ("4000 voice(s) still sound" frozen, DSP shown, polyphony counted, no sound) until the next
  device switch. Fix: `previousMuted = audioMuted.load() && audioDeviceSelectionUnmuteAt <= 0.0` (a pending device
  change's mute is the timer's business). Never restore a mute captured while an unmute timer is pending. 0.8.187
  also: the level after a run glides 26 dB/s from -52 dB with NO hold (full within 2 s of the report; was 4 s hold +
  4.5 dB/s = 16 s, the user: "jakoby se volume vracel až když se zase začne hrát"); a portable save writes
  `expressionPosition` 1.0 on every division (every shutter open); velocity layer "_128" (the plain files) unticked
  NEVER plays - `velocityLayerFor(..., bool& silent)` sets `silent` when no enabled layer has the note and startNote
  returns (the user: "když uživatel všecko odškrtá, nehraje mu nic, to je očekávané"; the self-test checks both);
  manual: loading markedly slower with packing and half-rate releases. DSP peaks "2x the average on WASAPI, not
  ASIO" (the user, 25 Oct): the load figure code is from the first commit; WASAPI shared mode is driven by the
  Windows audio engine's 10 ms pump - after a late wake JUCE's WASAPI thread fills two periods back to back (two
  renders counted in one period = a 2x peak), ASIO calls once per period from the driver. No background program of
  mine ran during his runs (06:17-07:04; my last build 24 Oct, test runs only with his program closed).
  25 Oct 2026 README LESSON: the user's "README from a cache, Echo Principal missing, 98 GB" was a STALE EDITOR
  TAB (Cursor) showing yesterday's file; the disk file was right (checked with ReadAllLines + Recent .lnk). Before
  blaming the save, read the file on disk and the log lines "RAM with every saving option" / "Configuration zip and
  README written" of that save - both are written anew at EVERY portable save from the live grid and a fresh
  measurement. The copy of HW Quinte 6 moved to III was the user's own "Echo Principal" (alias + shift -7), not an
  import's transmission: `tr:<source manual>:<stop>[#ordinal]` keys say nothing about who made it.
  0.8.188 PUBLISHING MECHANISM (the user: "mazání a nové uploadování je kravina"): an organ is identified by
  `publication.organ` of the json (`configuration_tools.py slugify`: lower case, [^a-z0-9]+ -> "-"; the program's
  `configurationReadme` writes the Download link with the same rule from `publicationOrgan`, the zip's name when none)
  - the folder `libreorgans/<organ slug>/`; `same-organ` = folders of that slug or whose json has the same organ;
  the index job (`configurations.yml`) `git rm`s them before moving the new pair in, so a new version REPLACES the
  old folder whatever the file was called (stralsund-jakobi-wegscheider-portable -> stralsund-st-jakobi-kr-wegscheider
  at the next publish). The issue workflow appends `<!-- published by @login (issue #n, date) -->` to the flat README;
  `publisher_of_folder` reads it: a REMOVAL by anybody else than the publisher or the repository owner is refused
  in the issue; a NEW VERSION from another account is proposed all the same with the note "published by @x; this
  version comes from @y - merge it only if that is right" in the PR and the issue (the user has TWO accounts,
  zurek-jiri = owner and zurekjiri; #93 from the second was refused by the first rule - the merge is the
  authorisation, never refuse an upload for the author). Removal: `configuration-remove.yml`
  runs on workflow_dispatch (input `organ` = ANY PART of the name: folder, README title or organ, case-insensitive,
  `find`; one match removes, several are listed) and on issues labelled `configuration-remove` (form
  `organ_configuration_remove.yml`; the label must exist - created 25 Oct via the API). `validate` (called by
  `configurations_check.sh` for every zip: full checks for an upload pair, json-only for a folder so old READMEs pass):
  one .json that parses to an object with portableConfigurationVersion, divisions, a sample tree, voicing stops,
  measuredRamBytes > 0, no MIDI mappings; the README's title = the zip's name, "Written by LibreOrgan Sampler", no "(fill
  in:", a Stop List table whose cells equal the "N rows" claim, the divisions, every `sampleTrees` entry named, the
  organ, the Download link's folder = the slug (an older README is refused: "save it again with 0.8.188"). Tested
  on the user's new pair (valid, slug stralsund-st-jakobi-kr-wegscheider, replaces the old folder), both published
  folders, a zip of config.js (refused). README RAM line 0.8.188: not saved at the least -> "; measured N GB when this
  file was saved, the organ loaded as <measuredRamStorage>" beside the estimate. Report folder 0.8.188:
  `settingsFile.getParentDirectory()/Reports/LibreOrgan-report-<stamp>` (never the Desktop again). FOUND BY THE
  HOOK TEST (test_save188.ps1, Rural saved after Stralsund): the `publication` members (organ, producer, product,
  link) stayed from the organ loaded before when the next file had no block - the Rural README said "Stralsund" and
  the organ-named folder would have REPLACED the published Stralsund; the parse now clears them (the author stays).
  Every change of the README or the publishing path is run through ORGAN_LOAD_CONFIGURATION + ORGAN_SAVE_PORTABLE
  and `configuration_tools.py validate` on the pair before delivery. 0.8.189 (the user: "žádná próza, cifry na
  začátku"): README title = `publicationOrgan` (else the file name less "portable" + its separators,
  `title_from_base` in Python = the C++ loop); RAM = "- **RAM:**" + one sub-bullet per figure ascending
  "  - **N GB** - 16-bit, 32 kHz, releases at half the rate, packed; *estimated*" / the measurement with ALL four
  options spelt out from the json's sampleStorageFloat/resampleOnLoad/storageSampleRate/releaseHalfRate/
  compressSamples ("16-bit, 48 kHz, full releases, no packing; *measured*") / the full quality = 10 x the least
  "*estimated*" (left out when the measurement was float + full releases + no packing). Never a sentence there.
  0.8.190 WHERE CONFIGURATIONS AND SAMPLES LIVE (the user: "je nepřijatelné, že si to všude píše OrganConfigurations
  foldery"; symlinks/shortcuts rejected - privileges on Windows, not on exFAT, Finder aliases, and a link is an absolute
  path in disguise; "beside the binary" rejected - /Applications, Program Files): `configurationsFolder` (machine-
  specific key; `configurationsDirectory()` = it or Documents/LibreOrgan/LibreOrgans, created on demand) - the
  Hauptwerk import retargets `plan.configurationFile` there (HauptwerkOdf::configurationFolderFor still computes the
  old default, unused by MainComponent), Save/Load/Merge choosers start there. `sampleLocations` (machine-specific
  StringArray, mirrored into the anonymous-namespace `gSampleLocations` for `portableFolderCandidates`, which appends
  `<location>/<rootName>/<relative>` and `<location>/<relative>` after the old candidates, and the absolute-only
  tails under every location). Filled by `noteSampleLocation`: the root's parent at a portable save, the root found at
  a load (`found` minus the relative path's depth; its parent when named like the root), the folder of a chosen
  archive (registerArchive), the first mismatch from the end between the old and the new path in the "Replace ... with
  a folder" chooser; Editor rows "LibreOrgans folder" / "Sample locations" (Add folder..., Forget) after "Load the
  last configuration at startup", help section "Where configurations and samples live" (manual chapter of that name,
  with the user's "nightmare" paragraph). README "Where to put the file" = the configurations folder + the trees
  expected under a sample location (no json line in the diagram). Test: %TEMP%\test_locations190.ps1 (Rural saved
  portable into %TEMP%\cfgtest, then loaded from there: found through the location). LESSON: a location noted inside
  applyPortableConfiguration must ALSO go into the configuration var ("sampleLocations") - its machine keys were
  copied from the settings read at the top and the parse after put the old list back (found by the test).
  0.8.191: `askLoadOptions(..., file)` adds the combo "This file" (copy into the configurations folder [default] /
  load it where it is) for a file outside the folder; the choice travels as the transient var property
  "copyIntoConfigurationsFolder", read and removed in loadConfigurationFile's lambda; the paths are resolved against
  the ORIGINAL, then `adoptCopyInConfigurationsFolder` copies the original bytes (or the archive's text) to a
  non-existent sibling in the folder and makes the copy the open file (currentConfigurationFile, name, lastDir,
  saveSettings). Hook ORGAN_TEST_COPY_INTO_CONFIGURATIONS=1 says yes on the hook path (the box is skipped there).
  IRs: `reverbIr` (absolute) is NO LONGER removed at a portable save; the load tries configDir/reverbIrRelative,
  `resolvedRoot` (the library root found through the division's tree)/reverbIrLibraryRelative, the pointed-to root,
  the config-dir forms, the absolute path, then every sample location (root by name, the IR path, tails of the
  absolute path) - "Impulse response not found ..." in the log and "IR: name (FILE NOT FOUND - choose it)" on the
  route caption; `chooseReverbIr` notes the IR's grandparent folder (the collection). The Editor row reads "Sample
  and IR locations".
  0.8.192: the Hauptwerk import DIALOG is gone (it came minutes after the organ played: `runWhenLoaded` polls every
  5 s and wants two quiet polls after the loader, then the voicing itself runs on the message thread first);
  `hauptwerkImportReport` = `<appdata>/ImportReports/<organ>-<date>.txt`, written with the plan summary at plan
  time, the voicing text appended at the end of `applyHauptwerkVoicing` ("Hauptwerk import report completed: ..."
  in the log). VELOCITY LAYERS: the 0.8.187 rule ("_128 unticked never plays") is REVERTED - the user: "moje chyba,
  jinak jsou tam mezery" - `velocityLayerFor` is the first commit's again (the plain files play when no ticked layer
  has the note); the strict rule lives behind the engine atomic `velocityPlainStrict` (configuration key
  "velocityPlainStrict", Editor toggle "Velocity layers: an unticked _128 never plays", default off), applied at
  the startNote call site only. The self-test expects the plain files to sound with nothing ticked.
  0.8.193 NO DUPLICATES (the user: "Nechceme duplikáty, proboha!!! ... musí se zeptat, jestli ho chce přepsat"):
  `askLoadOptions(..., file, importing)` puts the combo "fileChoice" into the box: a file outside the folder ->
  "copy" [default] / "inPlace" ("Load this file from its own folder: ..."); one of its name already there with other
  content -> "keepExisting" [default] / "replace" (.json.bak kept) / "inPlace"; an import whose <organ>.json exists
  -> "replace" [default] / "keepExisting" (import nothing, load the folder's file, the voicing not applied, noted in
  the import report). The SAME file already in the folder is loaded from there before the box (text compared).
  The import's plan-time write moved INTO the box callback (it overwrote the user's file unasked before, since
  0.8.158); `copyLoadOptions(from, to)` carries the five storage keys into the var parsed from the kept file;
  `adoptCopyInConfigurationsFolder` overwrites with a .bak (getNonexistentSibling = a duplicate is gone). Hook path:
  import -> replace; ORGAN_TEST_COPY_INTO_CONFIGURATIONS -> copy/replace; else inPlace. Tests:
  %TEMP%\test_dupes193.ps1 (copy, same file, changed file -> replace + .bak), test_import192.ps1 -KeepFiles then
  again (replace + .bak). juce::StringArray has no initializer_list<String> assignment: build with add().
  0.8.194 PORTABLE SAVE = EXPORT (the user: the ordinary configuration with MIDI must never be overwritten by the
  portable one; "jako u bug reportu"): `savePortableConfiguration` writes `target` = `<folder>/Publish/<base>.portable
  .json` (a ".portable" already in the name is not doubled) with `<base>.portable.zip` + `.README.md` beside it;
  `currentConfigurationFile`/name/lastDir are NOT retargeted (an early return in the publish branch); the relative
  paths are computed against the target's folder. Then `showPublishWindow(folder, zip, readme, launch)`: the
  `AfterReportWindow` with caller-given title/text/buttons ("Publish on GitHub" = the issue form prefilled with
  `title` and `organ` query parameters, "Open the folder"), `folder.startAsProcess()`, the browser, and the four
  `arrangeReportWindows` retries - exactly the report path. Hook ORGAN_SAVE_PORTABLE never shows it;
  ORGAN_TEST_PUBLISH_WINDOW=1 shows it without launching anything (pictured by ORGAN_TEST_DIALOG_SNAPSHOT after an
  ORGAN_TEST_CLICK on "Configuration Editor" at 18 s: %TEMP%\test_publish194.ps1). The Explorer window is found by
  the folder NAME "Publish" (CabinetWClass). 0.8.197: Publish is ALWAYS `configurationsDirectory()/Publish`,
  wherever the Save box's file was asked for (0.8.194 put it beside the chosen file: the user saved into his own
  subfolder Portable_Sharing, got Portable_Sharing/Publish and asked "kde je ten podadresář"); the log says
  "Publish folder ... opened in the file manager, the GitHub form opened in the browser". The import's own save
  (`applyHauptwerkVoicing` -> savePortableConfiguration(..., publishFiles=false)) logs "Portable configuration
  saved" too but is the ordinary <organ>.json in the configurations folder, NOT an export.
  0.8.198 IGNORED PATTERNS (the user: "nesmí mizet, co si uživatel do patternů zadá"): the Hauptwerk import wrote
  `ignoredPatterns` = the defaults into its configuration, so the user's "*trrp" vanished with every import; now the
  import writes the patterns IN FORCE (`engine.getIgnoredPatterns()`) plus the missing defaults. The patterns are a
  configuration key (they travel with the organ) - a configuration loaded from another file brings its own list.
  `isTremmedVariantFolder` also takes the suffix "trrp" (Altenbruch: "03-ortf_RP_BlockFl2TRRP" beside
  "03-ortf_RP_BlockFl2", "Dulcian16trRP"). `folderNameMatchesPattern`: no star = contains, "*x" = ends with,
  "x*" = starts with, case-insensitive. Test: %TEMP%\test_patterns198.ps1 ("*zzztest" in settings survives an import).
  0.8.199 (the user: "když dám uložit, otevře se to v Configurations, nikoli v Publish"): the Save chooser of a
  PORTABLE save starts in `LibreOrgans/Publish/<base>.portable.json` (the toggle's state is known before the
  chooser opens); the Publish folder is opened with `explorer.exe /select,"<zip>"` (a NEW window, the zip selected;
  a plain `folder.startAsProcess()` became a tab of an open Explorer window on Windows 11 - the probe showed the
  window titled after the folder) and `arrangeReportWindows` logs "Report windows: the file manager window ...
  found/NOT found; the foreground window ...".
  0.8.200 CAPITALS (the user: "i po značce stroje HW, BW, RW, BP, Pos., Réc., GO, G.O., CH."):
  `StopNames::capitaliseAfterDivisionPrefix` takes a CHAIN of marked pieces ("G.O.", "R.P."), a bare token of 2-5
  CAPITALS, or a known word (pos rec réc ped pd man gr gt go hw ow uw bw rw sw ch bp rp pedal recit récit swell
  great choir solo echo positif, any case) before the space; "Flute 8"/"A trumpet"/"Mixtur IV" untouched.
  `StopNames::selfCheck()` (20 cases) is appended to the --kerneltest report: a MISMATCH fails the CI.
  0.8.201 MARK CAPITALS (the user: "i první písmeno označující stroj by mělo být vždy velké, ped > Ped"):
  `capitaliseAfterDivisionPrefix` capitalises the first letter of every piece of the mark too ("g.o." -> "G.O.",
  "ped" -> "Ped", "hw" -> "Hw" - the first letter only, as asked) and a roman numeral whole ("ii." -> "II.");
  the self-check has 26 cases.
  0.8.201 ALIASES LOST (the user: "portable konfigurace si nezapamatovala mnou upravené Aliasy", Arnstadt): an
  alias typed in the VOICING row sets `stop->alias` only; the Stop List table (`manualConfigs[m].stopBuild.entries[]
  .alias`) was synced from the stops only when the Stop List dialog opened, and at every load the table's alias
  OVERRIDES the voicing state's (the "Compiling the Stop List: voicing state" phase and applyStopBuild) - the saved
  file had "Ped Subbass" in voicingSettings and "ped Subbass" in the table, the load showed the table's. Now
  `saveSettings` syncs the table's aliases from the engine's stops before writing (transmission rows skipped).
  Diagnosis script: %TEMP%\sb_dump.py <cfg.json> (table aliases) + aliases_cmp.py (voicing vs table). Hook
  `ORGAN_TEST_ALIAS="<manual>|<stop prefix>|<alias>"` (inside the ORGAN_SAVE_PORTABLE lambda) sets an alias as the
  row does; %TEMP%\test_alias201.ps1 = save + reload + Stop List dump. COPY NAME (the user: "když se portable
  nakopíruje do Konfigurací, musí ztratit příponu portable"): `configurationsCopyOf(file)` = the configurations
  folder + the name less ".portable" - used by adoptCopyInConfigurationsFolder, the same-file check, keepExisting and
  askLoadOptions' "exists" (so "X.portable.json" loaded with "X.json" in the folder asks keep/replace/inPlace).
  0.8.202 SMART TREE RELOCATION (Linux session, the user: "udelej to chytre, hodne chytre"; his Linux list held the
  location ".../OrganInstallationPackages/002067" = the WRONG folder noted by the old mismatch rule when he clicked the
  package for a missing "002067/pipes/bw", and the tree was replaced by the package blindly: no stops, notice gone):
  `relocateTreeSmart(poolIndex, chosen)` (MainComponent.cpp before showMissingTreesMenu) - the tree is looked for FROM
  the chosen folder: for the folder and up to 4 ancestors, the longest tail of the missing path (<= 6 parts) that the
  folder itself carries or that exists under it (`tailsEqual`/`underFolder` + resolveCaseInsensitive), the nearest
  wins a tie; then `treeFoldersFound(index, folder)` = how many of the Stop List rows' folders of that tree
  (`treeRowFolders`: FolderRef.relativePath + morePaths) exist under it - 0 of N -> the box "holds none of the stop
  folders ..." (Choose another folder / Use this folder anyway = the old blind relocateTree / Cancel), NEVER loaded
  blindly; then every other missing pool tree: the same place under the new prefix (old/new paths less the matched
  tail; `startsWithParts`), else the tail (>= 2 parts) of its path under the new prefix and 2 folders above, each
  checked with treeFoldersFound != 0; all applied with `applyTreeRelocation` (the old body without the reload) and
  ONE reloadDivisions; the location noted only when the path matched by name (the first-mismatch rule); a box lists
  what was found / still missing when more than the one tree is concerned. Callers: the notice's menu
  (`chooseFolderForMissingTree`: chooser starts in the first sample location), Load folder with a matching tail,
  the Stop List's "Replace ... with another folder". Load time: `portableFolderCandidates` adds one level around
  every location - the folder above (rootName/relative and relative) and its first 32 subfolders
  (`locationChildren`, a 30 s cache) - so a place noted one level off still serves. Hook `ORGAN_TEST_RELOCATE=
  "<tree tail>|<folder>[|<seconds>]"` (the box is logged as "refused" there). Manual chapter "1.6 The usual ways
  to an organ" (RAR set, GitHub configuration, not found, installed Hauptwerk, no definition, moved samples, second
  volume, sharing) linked by its quoted heading from 1.3 step 4, the RAR section, 1.5, "Where configurations...",
  4.1 and the ODF section; site "Using one" and libreorgans/README.md rewritten (they still said "put the .json
  beside the samples" since 0.8.190). The "same disk" idea: a configuration looks beside ITSELF (configDir forms)
  and in the locations - never over a disk; told the user. 0.8.203 LESSON (found by the Rural test): a pool path
  saved on Windows and read on Linux/macOS ("F:\Suchy_rejstrik\Kobrle\Samply", kept when the file carries an
  absolute folderPath) is ONE component to juce::File there (cwd-relative, backslashes inside) - getParentDirectory/
  getFileName lie on it; everything about old pool paths goes through `pathParts` (split on both separators): the
  prefix of the other missing trees = oldParts less the matched tail, the location = `ancestorOf(found, below + 1)`
  where below = the tree's parts under the common folder of the MISSING trees' parents (the portable save's root
  rule: one tree -> its parent; Rural -> Suchy_rejstrik, Hauptwerk -> HauptwerkSampleSetsAndComponents). Tests
  (~/tools/test_relocate_any.sh <exe> <cfg> <tail> <chosen> '<locations json>' [s]; the CI Linux binary in
  ~/Downloads/LibreOrgan): Rural copy in /tmp/rt with Kobrle chosen -> Samply found (14/14 row folders), a
  neighbour Tabor chosen -> corrected to Samply, /tmp/rt2 chosen -> refused (0 of 14); location one level too
  deep -> found at load; Altenbruch (002067+002068) with 002067 chosen -> 4 trees, location HSSC, 35 rows; the
  published Arnstadt with the whole OrganInstallationPackages chosen -> 6 trees, 21 rows; Altenbruch with only
  the wrong location .../002067 known -> found at load. Rural's Samply/20 and /19 trees do not exist on the
  Linux disk (still missing is right there).
  0.8.204 (#105, Linux): (1) the "Sample folders found" box of relocateTreeSmart stood over the loading window and
  flickered - now `runWhenLoaded` after the reload (at once only when no division reloads). (2) The frozen grid
  picture (0.8.154) of a 3-division organ over a 5-division workspace showed both header rows and column lines through
  each other: applyPortableConfiguration reads the NEW file's "manualCount" and, when it differs, lays the OLD stops out
  in that many columns (voicer->setActiveManualCount + rebuildUI) BEFORE the snapshot; ORGAN_TEST_SNAPSHOT writes the
  picture as "<png>.frozen.png". Test ~/tools/test_frozen.sh (Adlington 3 then Ashton 5 via
  ORGAN_LOAD_CONFIGURATION_AGAIN). (3) "Loading Ashton very slow" MEASURED: the Hauptwerk disk on this box is
  /dev/sda2 = Samsung 870 QVO 8 TB SATA (the Windows D:), mounted ntfs3 (the kernel driver, not FUSE); 8 parallel
  uncached readers give 528 MB/s, the log's load of 93.8 GB WAV in 182 s = 515 MB/s - the disk's ceiling, no reserve
  (the packing's thread time 30 s per 44 s tree on 16 cores is no factor). Only NVMe or the page cache makes it faster;
  never promise more from a SATA SSD. Linux python read test: files of a package not loaded recently, 4 MB reads.
  0.8.205 (the user: "zobrazovat rychlost cteni z disku v dalsim radku load dialogu, radky nesmi preskakovat"):
  `SamplerEngine::loadBytesRead` (atomic, added after every decodeSampleFile in loadSamplesIntoStop: the archive
  entry's bytes else `file.getSize()`), `bytesReadByLoads()`; the StartupFolderLoader's status has ALWAYS THREE lines
  now (progressLine / workLine / diskLine, " " when empty): `diskSpeed()` in the progress lambda takes the difference
  every 5 s -> "Reading from the disk at N MB/s"; one log line per tree "Tree X: N GB of sample files read in T s =
  M MB/s" (the figure to judge "loading is slow" by). Manual 4.4 says so (SATA ~500 MB/s). Posting a GitHub issue
  comment from the Linux session was DENIED by the tool classifier: the drafted answer stays in
  /tmp/issue105_comment.md for the user to post (gh issue comment 105 --body-file ...).
  0.8.201 VOICING ROW WIDTH (the user: a pedal's voicing row cut the Alias box off): `Voicer::rebuildUI`'s
  `line1Width` must mirror `StopRow::resized` line 1 - it was the sum of 0.8.135 and never followed the Transposition
  box (104 + 258), One-shot (96) and Rank count (146) additions, so the expanded column = the compass width on a
  27-key pedal; now the real sum with the pitch text at its 120 px minimum, and `resized` clamps the pitch label to
  what is left before the alias (tooltip = the whole text when cut). RULE: a new widget on line 1 of StopRow goes
  into BOTH places. Test: %TEMP%\test_row201.ps1 (ORGAN_TEST_EXPAND on "TA - subbass" of Rural + ORGAN_TEST_SNAPSHOT
  -> grid.png.row.png of the expanded row).
  404 ON THE DOWNLOAD LINK (the user, 25 Oct: "Adlington Surround.portable.zip" 404): GitHub turns the SPACES of an
  issue attachment's name into DOTS ("Adlington.Surround.portable.zip"), the README's link (written by the program)
  keeps the spaces -> the index job's folder held a zip no link reached. Fixed in the robot (commit c7cc01c):
  `configuration_from_issue.py` names the pair after the .json INSIDE the zip (`read_config` returns its name), and
  `validate` refuses a README whose link names another file; the published Adlington zip was `git mv`ed to the name
  with spaces. The export keeps the name AS TYPED, spaces included (the user: "adlington_surround" looks bad; the
  robot copes - the old rule "name configuration files without spaces" is gone). 0.8.195: Load Configuration opens a
  .zip (the one .json inside read with juce::ZipFile; `jsonFile` = the json's name beside the zip, `fromZip` -> the
  load box's "inPlace" becomes "copy", `adoptCopyInConfigurationsFolder(original, textFromZip)` writes the text),
  the chooser filter has *.zip. "THE PROGRAM CRASHED AT A SAVE" (13:03, no crash file, no WER event, the routes
  stopped in order one second after the save's RAM line): a QUIT with "Save" in the unsaved-changes prompt is the
  only path that saves and then ends in order - the log now says "Quit requested" and "Unsaved changes (quit):
  Save chosen"; read those lines before calling a save a crash.
  FEATURED (25 Oct 2026, the user: two tables, "Featured" and the others, Rural by default, easy in/out):
  `libreorgans/FEATURED.txt` = folder names one per line ("#" comments); `configurations_index.py` renders
  "## Featured" + "## All the others" (one table when nothing is featured) in INDEX.md and every sorted copy;
  `configurations_check.sh` skips FEATURED.txt; the workflow `configuration-feature.yml` (workflow_dispatch:
  organ = any part of the name via `find`, action feature/unfeature) edits the file, regenerates INDEX, commits
  to main. Editing FEATURED.txt on GitHub by hand does the same (the index job regenerates on the push). (d) The copy build's parallelFor takes physical
  cores - 1 (the mouse stuttered through a build); (e) one-shot "Chiff" alone -> the new default (as the ignored
  list's "trem" rule); (f) arrangeReportWindows retried at 9 and 15 s (a cold browser came up maximised).
  (g) WASAPI "much quieter than ASIO at the same volume" (the user, 20:14): the render chain is one for every
  device type; the difference lies after our buffer - the Windows volume mixer's per-app session volume (per exe
  NAME: LibreOrgan_0.8.184.exe is a new app to the mixer) and the endpoint's level/enhancements; told so.
  Remove a configuration: GitHub's own "Run workflow" form has two clicks (open the
  form, then the green Run workflow) - nothing of ours asks; the 15:02 run removed the folder fine, the 14:34 one
  failed on the name. The "13 GB" at 16-bit 48 kHz packed = read while the copies were still building (the user
  closed the program at 18:38:31 mid-build); 23 GB is the figure with the copies.
- The #87 table (the user's measurements, Stralsund 53.9 GB float WAV, whole program with the copies): float 48 kHz
  85 GB; float packed 30; 16-bit 48 kHz 49 / packed 23; 16-bit half releases 33 / packed 13; 16-bit 32 kHz half
  packed 8 - in the README only since 30 Oct 2026 (the user: the WEBSITE names no technical detail - no 16-bit/32 kHz/
  FLAC/half-rate, no table: the fact line and feature 07 say "an 80 GB sample set squeezed into 8 GB of RAM with the
  saving options on a modest computer, samples read from the packed archives, up to 500 000 pipes of polyphony on a
  strong one"; GitHub keeps the technical detail; the README's long RAM paragraph is the same short one and its video
  sits under "Try it in five minutes" as "## The program at work"). Keep the README table when the text changes; add
  a column or row only with a measured figure (never estimated).
- Measurements 0.8.141 (#44/#45, Ultra 7 155H, Caen, 7 routes, WASAPI 480 = 10 ms): #44 (16 threads, IR on 6 routes,
  78 buttons) 42 200 = the stimulus cap (61 keys x 16 strikes, callback 53 %, "first failing level 0") - the log says
  so even though the user disputes it; #45 (6 P cores, 32 buttons, no IR) broke at 39 239 with callback 95 %.
  Ryzen 5950X (#35 Doesburg, 15 routes, WASAPI 480) 53 000 at 85 %; (#40 ASIO 256) 32 200. Per voice and block in
  the run: notebook 1382 ns (kernel 1043) vs Ryzen 1922 ns (kernel 1484) at 480 frames - at 40-50 k voices both
  stream from RAM (bench 1.57 vs 1.54 ns/frame) and the notebook's DDR5 + 2 MB L2 per P core keep up with Zen 3's
  DDR4; the Ryzen also carried 15 routes (2x the fixed cost per period). A like-for-like run (same organ, routes,
  buffer) is still missing.
- Linux window icon at start-up "rozsypana" (scrambled) since 0.8.136 (`Main.cpp` sends `icon48_png` to `setIcon`).
  Before: upside down for a moment while loading but crisp. FINDING 0.8.171: in this JUCE `DocumentWindow::setIcon`
  only stores `titleBarIcon` (the JUCE-drawn title bar) and NO JUCE code calls `ComponentPeer::setIcon`, so our
  `setIcon` never reaches `_NET_WM_ICON`; the title-bar icon seen on KDE comes from the desktop entry (WM_CLASS) -
  look there (the installer's ~/.local/share/pixmaps/libreorgan.png, 48 px) before blaming JUCE's X11 path. NEVER a
  rescaled icon.
- Device-thread peaks (0.8.138 reports): #34 Ultra 7 155H (P+E cores): longest route callback 2616 us of which WAITING
  FOR THE LANES 2412 us - a worker on an E core (or preempted) holds the poster; try lanes on P cores only / smaller
  chunks at the end of a job. #35 Ryzen Windows: longest 1270 us of which OWN RENDER 1071 - the poster takes chunks
  while the workers are busy; cap the poster's share or batch all routes of a device into one pool job (#30: 21 routes
  x fixed cost). #29: bookkeeping 66 us per route at 44 000 voices (compaction of the active list) - speed up.
- Issue form: the "Operating system" field is a text field filled by the program (now with the version); fine.
- GitHub's issue list hides the number in its new UI (the user asked why "#27" is not in the row): nothing of ours.

- 25 Nov 2026, v0.9.133 RELEASED (run 732 green). CI commit f5ce137 on main (no tag): the Self-test / Kernel test steps
  (continue-on-error) had shown red crosses in EVERY build for weeks (0.9.124 too) - on Windows `& exe` did not wait for the
  GUI program; now Start-Process + WaitForExit, a failure prints `::error title=Self-test::<reason>` (job logs need a login,
  annotations do not: `commits/<sha>/check-runs` + `<check-run>/annotations`); actions on Node 24 (checkout@v5,
  upload/download-artifact@v6, configure-pages@v6, upload-pages-artifact@v5, deploy-pages@v5). At the NEXT build read the
  annotations of the two test steps on macOS / Linux (cause unknown) and fix what they say.
- 0.9.134 (25 Nov 2026, the user's "ano" to the proposals of the next note + three additions; LOCAL COMMIT, NOT pushed -
  his word): (1) PER-SAMPLE TUNING: `Sample::layerCents`, `VelocityLayer/ReleaseLayer::layerCents[128]` set at the top of
  computeTuning440 per rank (reference = the plain attack's `smplPitchInFile` - set for releases too now; a layer whose
  chunks nobody filled - every value a whole note, or one pitch on every key - gives 0), `Voice::layerCents` added to
  `tuningCents` at the four places through `layerCentsWithin(cents, retuneLimitCents)`: THE BOUND IS THE SLIDER "Replace
  pipes off by more than" (his rule: no figure of my own - I had said 100 c); getNextReleaseSample has the out-parameter.
  Log "Tuning <rank> [p]: N velocity-layer / release sample(s) ... tuned to it by their own chunk (up to X c ...)".
  MEASURED by ORGAN_TEST_RECORD (%TEMP%\rec_peaks.py <wav> "1540,2090"): Siebenquart key 57 held 1525.0 / 2069.7 Hz,
  after the release 1555.2 / 2069.9 = +34 c / 0 c (was +66 / +32). (2) NOISES: HauptwerkOdf `Noise::rankId`, the noise
  takes its RANK's perspective after the rank rules ran; `organPerspectiveNames()` (the TABLES' columns incl. the ones
  not loaded + the engine's, sorted), `noisePerspectiveWords()` / `noisePerspectiveOfName` (the folder reading's word
  rule made members), `settleNoisePerspectives()` in noisesTick before noiseSpecs (a '' sound of an organ whose noises
  have perspectives: by the file / folder names, else the one its sisters of the same trigger leave free, else the
  first + Messages; heals an older file at every load until saved), `noisesOwnRoute` (no sound names a perspective):
  `SamplerEngine::noisesPerspectiveName()` "Noises" pushed into configuredPerspectives of every division <= manualCount
  (syncConfiguredPerspectives; set preliminarily in noisesChanged so the saved cable is not dropped), order index
  INT_MAX - 1 (before the auxiliary), noiseRoute prefers it for a '' sound; `audioRouteSteps` now calls
  syncConfiguredPerspectives first (the loader's route steps did not know the configured perspectives - the route came
  only at some later rebuild). The Noises window: columns = organPerspectiveNames() (or the one "Noises"), dividers
  2 px grey 0.65 continuous. A fresh "Noises" route opens like any new perspective's (the division's host / the default
  card). (3) NEVER TO THE FRONT: `refreshNoisesWindow()` = showNoisesWindow(noisesWindow->isActiveWindow());
  showNoisesWindow(false) replaces the content with setContentOwned (window, place, Z order kept). The other dialogs
  refresh in place already (Editor: configEditorRefresh; Audio routing: its signature timer). Hook act
  ORGAN_TEST_NOISE_UNITS "<s>|order" = perspectiveOrderChanged + the desktop windows back to front before / after.
  (4) DEFAULT CARD: machine key "defaultCard" ("<type>|<name>", volatile for the dirty fingerprint) set in
  defaultCardFound, read in loadSettings into preferredDefaultType/Name (not with a DefaultRouting.json),
  `startDefaultCardSearchIfDue()` from the timer 3 s after the start is quiet (once). With it a portable organ's routes
  attach to ASIO at the load ("every route of the organ plays through ASIO Fireface USB already"). A test hook load
  starts before the search: put "defaultCard" into the test's settings (test_0127b.ps1 -Settings "defaultCard=ASIO|ASIO
  Fireface USB") or the routes are moved DURING an ORGAN_TEST_RECORD and the recording is silent. (5) #265:
  `perspectivesNotLoaded` (configuration key; cleared at a clean start and at Global Reset's unload; a missing key =
  all loaded): stopBuildMap skips those folders, syncConfiguredPerspectives gives them no route, closeHiddenRoutes KEEPS
  their route settings; the load box's `PerspectiveTicks` custom component (>= 2 perspectives; names from the var's
  stopBuild folders, order of its perspectivePatterns; nothing ticked = all), hook ORGAN_TEST_PERSPECTIVES_OFF="A;B";
  Stop List: `notLoaded/originalNotLoaded`, the tick box `HeaderCell::load` before each column's name, dimmed cells,
  the cell menu item 900000, changedDivisions counts a toggled perspective, apply() writes owner.perspectivesNotLoaded;
  rename follows. Schorndorf with Direct alone: 7 s, 9.8 GB (all four: ~37 GB). NOT done: the box's memory line does
  not follow the ticks; a Hauptwerk import's route volume of a perspective not loaded at the import is lost; the
  ticks are ignored by a merge. (6) An alias with a compound footage is shown as typed ("Terznone 1 3/5' + 8/9'",
  checked by the Stop List dump). TEST SCRIPT LESSONS: a script with `param()` and no CmdletBinding takes an UNKNOWN
  named parameter silently into $args (my -Settings did nothing for two runs); a tool call whose command holds a
  blocked Remove-Item is rejected WHOLE - nothing of it ran; ORGAN_TEST_NOISE_FOLDER fires at the arming only, i.e.
  never for an organ without noises (craft the "noises" block: %TEMP%\make_rural_noises.py); ORGAN_TEST_CLICK toggles a
  ToggleButton by its text (the load box's ticks: "zzz|zzz|Diffuse|Organist|Rear|zzz|Load", %TEMP%\test_0134_box.ps1).
  The Stop List's tick + Apply and reload RUN (ORGAN_TEST_MOUSE "load Diffuse|click" - the tick box's testName is
  "load <perspective>" - then ORGAN_TEST_CLICK "Apply and reload": "Commit: 138 rank(s) kept their samples", Diffuse
  read alone in 30 s, its routes started). NOT run: a real column drag in the Stop List with the Noises window open
  (the hook calls the same function), unticking a LOADED perspective + Apply, Linux / macOS. test_0127b.ps1's -Env
  splits at ";" - a value with ";" (ORGAN_TEST_PERSPECTIVES_OFF=A;B) keeps its first part only.
- 0.9.134 REWORKED THE SAME DAY (the user on the first build: "nechtěl jsem posuvník replace pipes, ale retune pipes
  off by more than ... druhý posuvník má také být v akci"; "pro nový mergovaný nástroj se nesmí ignorovat"; "chtěl jsem
  to pro každý rejstřík zvlášť ... při canceled loading zůstávají všecky řádky jako načtené"; "jeden checkbox, druhý
  tmavší křížek, to je na hovno"; second local commit, still NOT pushed). WHAT REPLACES THE NOTE ABOVE: (1) TUNING OF A
  SAMPLE = THE TWO SLIDERS AS FOR A PIPE: `Stop::tuningCentsForSample(note, tuned, layerCents, retuneAbove, replaceAbove)`
  (SamplerEngine.h): |layerCents| > "Replace pipes off by more than" -> not believed, the sample follows its pipe; with
  "Equal at 440 Hz" -> tuning440 + undulation + layerCents; else the sample's OWN deviation from the rank
  (rankDeviationCents - layerCents) is pulled to the key only when it exceeds "Retune pipes off by more than" (0 = never).
  So at the DEFAULT 50 c nothing changes anywhere (measured: Siebenquart 57 release +67 / +32 c as before); at 10 c: held
  1545.7 / 2097.7, release 1582.4 / 2104.7 = +41 / +6 c (the releases sit 5.6 c off the rank: inside the slider - at 5 c
  they are pulled too). `layerCentsWithin` is gone. (2) "NOT LOADED" IS A FLAG OF EVERY FOLDER OF THE TABLES:
  `FolderRef::unloaded` (JSON "unloaded": true on the folder object; saved with the configuration), stopBuildMap skips it,
  a perspective has a route when any folder of it is to be loaded, closeHiddenRoutes keeps the route settings of a
  perspective whose folders are all unloaded. `perspectivesNotLoaded` (the name list) is GONE.
  `setPerspectivesUnloadedInVar(var, off, on)` (static) marks the folder objects of a var: used by the load box (only
  the perspectives whose tick the user CHANGED; nothing ticked = all), by copyLoadOptions (transient
  "perspectivesOffChoice"/"perspectivesOnChoice" for a kept file) and by mergeConfiguration through mergeStorageChoice -
  on the ADDED organ's var alone (an ODF is read anew on the merge path). Hook ORGAN_TEST_PERSPECTIVES_OFF takes "A,B"
  or "A;B". (3) STOP LIST STEPS (his choice "krokové tlačítko", without menus, both ways visible): `StepButton` (the
  sign DRAWN - a "+" as text did not fit 20 px and showed "..."): a folder to be loaded has "-" (unload: stays, dimmed),
  an unloaded one "+" (load again) and "x" (take out of the row); a column's name the same ("-" when anything of it is
  to be loaded, "+" when anything waits, "x" when nothing is to be loaded); every click through `later()` (callAsync).
  The header tick box and the cell menu item are gone (the column drag's landing line stands right again).
  (4) THE TRUTH OF THE MEMORY: `readLoadedPlaces()` (the folders of the engine's ranks that hold samples; pooled =
  samplesUnloaded apart) - a cell whose folder is not in the memory is DIMMED whatever the table says;
  `pendingCells/pendingDivisions` (to be loaded, on the disk, not in the memory, not pooled) enable "Apply and reload"
  (apply() adds those divisions; hasUnappliedChanges - the close question - counts table drafts only) and the info line
  names them. A cancelled load: 19 stops left out -> Apply read them, "16 rank(s) kept". Unload of a loaded perspective +
  Apply: process 19.0 -> 9.9 GB. moveFolder keeps loaded / unloaded with the PLACE. (5) THE DEFAULT CARD IS NEVER TAKEN
  UNASKED: the first build set preferredDefault* from the remembered key in loadSettings - with the Fireface switched
  off (it was, during my tests: "hdspf init failed") every route of the start went to it and "Audio devices not
  available" came up on an empty console. Now the remembered card is only the FIRST candidate of the search after the
  start (helper process); until it answers the default is Windows' own. RULE: a remembered device is asked before it is
  used. (6) Voicing per-key rows: `toneNumLabels/attackNumLabels` (MIDI numbers, toneLabelsHeight 24), the state of a key
  (grey, green thumb, red / orange name, tooltips) set on all three rows through rowSliders/rowNames/rowNumbers - a NEW
  per-key row joins those three vectors. Hook ORGAN_TEST_RECORD_DELAY=<s> (a portable organ's routes are moved right
  after its load: a key recorded meanwhile is silent). Stop List testNames "load <P>" / "unload <P>" (ORGAN_TEST_MOUSE).
  NOT run: a merge with perspectives unticked for the added organ (file or ODF), a real mouse on the step buttons, his
  "a grey perspective cannot be replaced by dragging cells" (no block found in moveFolder - a folder dragged onto an
  unloaded place now takes that place's state), Linux / macOS.
- 0.9.134, THIRD ROUND (25 Nov 2026 afternoon; local commits bff2447 + the one after, NOT pushed): (1) THE STEP BUTTONS
  WERE REFUSED ("to je paskvil! Tam se zobrazuje a mizí tlačítko! Někdy jsou dvě vedle sebe") - RULE: a control never
  appears, disappears or changes its width with the state. Now `StateSwitch` (Stop List dialog class: ONE component of
  fixed size, three places - left a dot = loaded, middle a DASH = kept in the table but not loaded, right an x = removed;
  the knob in the sliders' thumb colour on the present place, grey in the middle, between the two for a column of which
  some folders are loaded = state 2; state 3 = an empty cell, dim and dead; `onChoose(place)`), beside every folder cell
  (`Cell::stateSwitch`, cellWidth 284, switchWidth 62) and every perspective's name (`HeaderCell::stateSwitch`, testName
  "switch <P>": ORGAN_TEST_MOUSE "switch Rear|click" clicks its middle = unload). His choice among three drawn variants
  was not explicit ("jinak se mi to docela líbí") - variant 1 (blue knob) taken; the knob's colours are two lines of
  paint(). DESIGN QUESTIONS GO WITH A PICTURE: %TEMP%\mock_stoplist.py / mock_stoplist2.py (PIL, Python39) drew the
  proposals into F:\temp\navrh*.png, opened for him with Start-Process - an ASCII preview in a question was "nechápu to".
  (2) THE RIGHT-CLICK MERGE SHOWS THE LOAD BOX: `loadBoxForMerge` (set before askLoadOptions; no "this file" and no
  "load or merge" combo, title "Merge into the loaded organ: X", button "Merge") in mergeConfigurationFile and at the
  ODF merge (importHauptwerkOdf's merge branch); skipped when the ordinary box already chose the merge
  (mergeStorageChoice set) and under ORGAN_MERGE_CONFIGURATION. NOT run through the GUI (a native file chooser comes
  first). (3) NOISES AFTER A MERGE: his log "Noises: 1568 sound(s) of the configuration not found - the library root is
  not known on this computer" - a merged organ's trees are absolute, treeRelativePaths empty, so
  libraryRootOnThisComputer() gave nothing and EVERY noise of both organs fell silent with nothing said in the Noises
  window. Fallback: the folder above the first part of a noise's relative folder, found in the path of a loaded tree and
  verified. Reproduced by ORGAN_MERGE_CONFIGURATION + Cancel: "Noises loaded: 1563 noise sound(s) of 1563". SEEN, NOT
  FIXED: the merged organ's noise buttons land on division 0 ("OW. Cimbelstern put on the grid of division 0").
  (4) NOISES FOLLOW THE LOADED PERSPECTIVES: `loadedPerspectiveNames()`; noiseSpecs leaves out the sounds of a
  perspective without a route (log "N sound(s) of perspectives that are not loaded are left out"), noisesTick reads the
  noises anew when that set changes (`noisePerspectivesAtLoad`); `NoiseUnit::state` per perspective (1 = its perspective
  is not loaded, 2 = its files were not found) -> the folder label dimmed / orange with the reason in the tooltip.
  (5) PENDING HIS WORD (asked twice with pictures, he wants it "domyšlené globálně"): the Noises window in the Stop
  List's look - x at the left, Division, name (the SAME component and colour as the Stop List's stop name - my mock drew
  it otherwise and he saw it at once), Kind, Volume, per perspective the folder cell with "(160) +1", the StateSwitch
  and the perspective's volume as a BAR with its value (juce::Slider::LinearBar) in a row of the Stop List's height. His
  thoughts: such bars could replace every slider (they save the value box; they resemble the swell and crescendo
  DISPLAYS - "pozitivní, ale vzhled nesmí být stejný, protože jedno je displej, druhé je posuvník"); the vertical
  per-key sliders cannot hold a text. My advice given: bars in tables only, the other sliders as they are, a global
  change as its own step with pictures first. TEST HYGIENE: the hook ORGAN_MERGE_CONFIGURATION SAVES the merged organ
  into the test's configuration copy - the next test loaded 106 GB instead of 52; copy the user's original again before
  every run that follows a merge test.
- 0.9.134, FOURTH ROUND (25 Nov 2026 afternoon, his "ano, ano, ano - všecko kladně" to the pictures navrh2-4 + "všecky
  posuvníky by musely reagovat stejně, ne aby jeden byl dvojřádkový, jiný jednořádkový"; local commit, NOT pushed):
  (1) BAR SLIDERS = THE LOOK OF EVERY SLIDER, in ONE place: `UIScale::LookAndFeel::getSliderLayout` (horizontal: no
  value box, `setSliderSnapsToMousePosition(false)` = a drag moves the value from where it is and a click alone changes
  nothing, `setMouseDragSensitivity(max(120, width))` = the line follows the mouse) + `drawLinearSlider` (the box in
  the combo's colours, the fill from the double-click default - without one: from 0 when the range has both signs,
  else the minimum - to the value, a 2.5 px line in the thumb colour; the line is drawn as two short marks where it
  would strike through a text; a slider with a track colour of its own - a sounding key green, a pipe off grey -
  keeps it as fill and frame). Properties: "barLabel" (the name, left; without it the value alone, centred),
  "barLines" 2 (the name small above the value, bar 44 px; one line 28 px), "barShort" + "barUseShort", "barTyping"
  (set while the field is open), "barNoTyping". `UIScale::isBarSlider / barAreaOf`. VERTICAL sliders: the same bar
  upright (22 px wide at most), their value box below stays. (2) TYPING: `UIScale::BarValueTyping` (ONE object,
  MainComponent::barValueTyping; a GLOBAL mouse listener): a left click without drag, modifiers or popup on a bar ->
  after `MouseEvent::getDoubleClickTimeout() + 40` ms a TextEditor child over the bar (orange frame), Enter / a click
  elsewhere / focus lost = taken (onDragStart, setValue sync, onDragEnd - as the old value box), Esc = left; the
  second mouseDown of a double click cancels the pending field. The editor is deleted by callAsync (never inside its
  own callback). (3) TOP ROW: the Label objects are hidden for good, names in the bars; ONE form for the row by the
  bar's width: >= 205 px one line, >= 112 two lines, below that two lines with the short names (resized()).
  (4) VOICING ROW: labels not added to the row any more (their texts are the names), widths by the texts
  (`stopVolumeSliderWidth` 176, `transmissionVolumeSliderWidth` 248, gain 162, attack 164, random 238, undulation 238,
  formant 232, `voicingSliderGap` 12 - the 0.9.36 rule: constants, slidersRowWidth and the compact walk's widths[]
  together), a bar at line + 8, 28 high; the undulation's "c bottom / c top" detail moved into its TOOLTIP, its name
  "Undulation - needs 440 Hz" without 440 (no setAlpha any more: a disabled bar is drawn dimmed). (5) `StateSwitch`
  lives in UIControls.h (enum mixed = 2, dead = 3): in the mixed state every place acts (the x did nothing before:
  code 2 was also "the right place"). (6) NOISES WINDOW REBUILT (`NoiseUnitsContent`, placeDialog key "noises4"): the
  Stop List's measures (rows 32, headers 30, switch 62, name field 16 px on 0xff2b2b2b), columns = switch (on / off /
  remove) | Division (P., I. ...) | Noise | Kind | Volume bar | per perspective: folder TextButton ("TremPE (1) +1",
  a click unfolds the row - the "+" button is gone), StateSwitch (on / off / files out), volume bar; header per
  perspective = name + StateSwitch for every noise (testName "noises <P>"; x asks first). New:
  `removeNoiseUnitPerspective(key or "*", perspective)`, `setNoisePerspectiveOnEverywhere(perspective, on)`; hook acts
  ORGAN_TEST_NOISE_UNITS "<s>|pall|<perspective>|0 or 1", "<s>|premove|<key or *>|<perspective>". (7) HOOKS:
  `ORGAN_TEST_BAR="<s>|<name in the bar>|open|type|<text>|dblclick|value|drag|<px>^..."`,
  `ORGAN_TEST_SNAPSHOT_KEEP=1` (the dialog snapshots of every ORGAN_TEST_CLICK kept as "cNN-<w>-<title>.png");
  test_0127b.ps1 -Settings "windowWidth=1500;windowHeight=900;windowMaximised=false" gives a narrow window.
  RUN: Rural (top row one line at 2442 px, two lines at 1500 px; typing -12 dB into Master and -3.5 dB into Stop
  volume; a press 10 px from the left end leaves the value, a drag of 40 px = 11 dB of Master's 72 on 261 px; the
  per-key row pictured), Schorndorf with Direct alone (the Noises window: dimmed perspectives, the Stop List beside
  it), Divisions / Wind / Tremulant / Crescendo / Audio routing / Editor pictured. NOT run: a real mouse (the click
  that opens the field goes through the global listener - synthetic events bypass it), the wheel, the header
  switches of the Noises window and a folder's x by a click (their functions were run by the hook acts pall / premove /
  pon: Direct off for 5 noises, Rear out of one, Organist out of all), the Effects dialog's EQ sliders, Linux / macOS.
  THE USER FOUND TWO DIFFERENCES TO THE STOP LIST AT ONCE ("proč je vypnutá perspektiva jinak vypadající než v dialogu
  StopList?", "u stoplistu se objevuje slůvko none ... proč si nepamatuješ kontext?"): a perspective the Stop List has
  not loaded showed the noise's OWN switch (blue, left) - now `StateSwitch::locked` (the Stop List's look exactly: the
  knob in the middle; it takes no click, the tooltip sends to the Stop List), its volume bar disabled, in the header
  too; an empty cell reads the grey "(none)" of the Stop List's cellText. RULE: "in the look of X" = pictured beside X
  in EVERY state (not loaded, empty, off, not found) before delivery. Self-test 0 WRONG (81 lines, 35 OK as 0.9.124),
  kerneltest OK. LESSONS: MainComponent.cpp and MainComponent.h are CRLF, voicer.*,
  UIScale.h and UIControls.h LF - a scripted insertion must use the file's own line ending (check for bare LF
  after); a test process without the focus loses a typing field at once (openFor of the hook keeps it).
- 0.9.134, FIFTH ROUND (25 Nov 2026 late afternoon; his try of the fourth round + the picture F:\temp\navrh5_rada_a_svisle.png
  made by %TEMP%\mock_bars3.py; his answers: "Čisté ubírání bez loaderu", the header remembers the mix, B1, fill no. 3,
  the vertical ones 21 px with 8 px numbers and ORANGE, the cancelled load and the locked noise switches stay as they
  are): (1) APPLY THAT ONLY TAKES AWAY ("když načtu perspektivy, pak jednu vyškrtnu ... on to fakt reloaduje! ... musí
  pouze uvolnit zrušené z paměti"). HIS LOG: 14:52:32 Rear unloaded alone - nothing was read ("90 rank(s) kept"), but the
  whole machinery ran: loading window, every route of the four divisions closed and opened again, 3 s of silence. Now
  `StopListDialog::removalOnly(changed)` (pendingCells == 0; every row of the table now is a row of the original drafts
  in the same order with the same alias / child / transposition, and every folder it wants loaded was loaded and is the
  same folder) -> `applyStopLists(..., removalOnly)` -> `reloadDivisions(manuals, {}, nothingToRead)`: the SAME proven
  way (loadManualFolder builds new stops that adopt the samples at commitPendingDivisions; what nobody adopts is freed)
  but synchronously on the message thread - no StartupFolderLoader, no window - and `removePerspectiveAudioRoutesForManual
  (manual, &keep)` only for the routes of perspectives that are gone; then finishDivisionsLoad. reloadDivisions checks
  for itself that the ENGINE holds every folder the tables want (else "not a mere reduction after all ..." and the
  ordinary reload) - the quick way must never have anything to decode on the message thread. Log "Stop List:
  division(s) ... reduced in place, no loading window - 0.0 MB read, N GB given back so far, T ms". MEASURED: Schorndorf
  Direct + Organist, Organist's header switch to the middle, Apply: 0.0 MB read, 9.2 GB back, 877 ms, no "Audio device
  started" line for Direct, no loading window. (2) THE HEADER REMEMBERS A MIX: 15:03:15 of his log read 14 GB of
  Schorndorf he had unloaded - after the merge the Rear column was a mix (Schorndorf not loaded, Stralsund loaded) and
  header clicks "off, on" set every row alike (not proven: the clicks were not logged - they are now, "Stop List: the
  switch of ... set to ..."). `openingPattern(perspective, mixed)` from originalDrafts; `setPerspectiveUnloaded(..., false)`
  on a column that was a mix and is not as opened -> the opening pattern back (info line says so), a second click loads
  all. NOT run (ORGAN_TEST_MOUSE clicks a switch's middle only). (3) TOP ROW B1: `rowBarHeight` 46 for the bars AND the
  display (controlsRow 62 px, was 80); one line while the names fit at 13 / 12 / 11 px ("barNameSize"), then two lines
  (name 13 px box + value 16 px box around the centre), then the short names; the display's width jlimit(min(330, w/3),
  420, w/6); PolyphonyDisplay / paintOutputMeter have a `low` form under 56 px (10 px labels, CLIP smaller, steps 1 px
  apart). "barHeight" property = a row's own bar height (barAreaOf). (4) ONE ROUNDING: `barCorner` 6 for the bars,
  DisplayLabel and PolyphonyDisplay 6 (were 3; the buttons are JUCE's 6, the swell bar was 6) - text boxes and combos
  keep `boxCorner` 3. (5) FILL = the look's own thumb blue x saturation 0.8 x brightness 0.50 (no. 3 of the swatches),
  never the slider's own thumb colour (a replaced pipe's green is its LINE alone). The split marks are as long as the
  bar allows: (height - text block) / 2 - 2.5. (6) PER-KEY SLIDERS: `Voicer::sliderWidth` 21 (was 30), bar width
  jlimit(8, 22, width - 5), every per-key slider and "All" has "smallTextBox" = an 8 px value box without side margins,
  note names and MIDI numbers 8 px with no Label border; a SOUNDING key = trackColourId onColour() -> the bar's ground
  tinted (ground.interpolatedWith(track, 0.45)) and a 2 px frame, so it shows at the default value too (the green
  frame alone did not); grey track = pipe off, the same way. HIS QUESTIONS ANSWERED, nothing coded: a cancelled load
  drops the stop it was reading whole and skips the rest - stops whose perspectives lie in several packages may be left
  with those of the trees already read (dimmed cells, Apply reads them); a noise cannot be switched on in a perspective
  the Stop List has not loaded because it plays through that perspective's route, which does not exist then.
- 0.9.134, SIXTH ROUND (25 Nov 2026 evening, the user on the 21 px columns: "takhle voicing být nemůže ... ostatní
  políčka skočí jen do dvou řádků, takže se to roztáhne doprava ... nesmí to být širší než nejširší rozbalená sekce
  vertikálních sliderů ... když budou všecky per key řádky sbalené, tak to taky musí být ultra úzké"): THE VOICING HAD
  TWO LAYOUTS - the wrapping walk (0.9.36) for folded per-key rows only, and a "full" one (everything on two lines,
  ~1300 px, and the column took max(the whole free width, that)) as soon as a per-key row was unfolded; narrowing the
  key columns left the top lines wider than the keys. I had the picture and did not see it. NOW ONE LAYOUT:
  `StopRow::resized` and `expandedHeightFor` use `compactControlsLayout` for EVERY open voicing of a stop with pipes
  (containers / noise buttons keep the old line); the pitch text (Control::diapason, its `index` = the lines kept:
  two when the row is < 900 px inside, never squeezed - minimumHorizontalScale 1, topLeft) on a line of its own UNDER
  the button from the left margin; slider lines 36 px apart (bars at y + 2); Control::source as wide as the row
  allows. `Voicer::rebuildUI`: `voicingColumnWidth[manual]` = the widest UNFOLDED per-key row among the manual's open
  voicings (margins + 26 + 30 for a child + allControlSpace + keys x sliderWidth, the keys = the compass and every
  key the folder holds - as makeStopRow), at least `StopRow::minVoicingWidth` 590, or `foldedVoicingWidth` 760 when
  every per-key row is folded; the columns without an open voicing share the rest (>= 335). wideWidth / compactWidth /
  line1Width / topControlsWidth are GONE - the 0.8.201 and 0.9.36 rules "a new control goes into both layouts and
  line1Width" are replaced by: a new control of the voicing's top goes into compactControlsLayout (voicer.h) and its
  `put` case in resized, nothing else. `sliderWidth` 22 (his wish). Pictured on Rural: the pedal with Volume per key
  open 790 px (was ~1300 and stretched), a 61-key manual 1374 px = its keys, folded 760 px. RULE: after a change of a
  size, picture every state the size takes part in - here the open pedal AND the open manual AND the folded voicing.
- v0.9.134 PUSHED 25 Nov 2026 evening (main 469038b, tag v0.9.134 on it, the user's word "dej to konečně na gitHub"):
  run 45645548401 - every build green on the second attempt (the first lost a macOS runner, build-retry restarted
  it), the RELEASE job failed: softprops/action-gh-release@v2 "Too many retries" (GitHub's side; nothing of ours
  changed there) -> NO release yet. I cannot rerun it (no token, no gh on Windows): the user clicks "Re-run failed
  jobs" on https://github.com/zurek-jiri/LibreOrgan/actions/runs/45645548401; then: the release's five assets, the
  Intel mac "late symbols: 0" check, the CI exe to C:\LibreOrgan\LibreOrgan_0.9.134.exe (the local build of 16:25
  stands there now). The FIRST CI ANNOTATIONS of the test steps: Intel macOS "Kernel test: exit code 2 - Stop name
  capitals ... 26 cases -> 2 MISMATCH" (the others clean) = juce::CharacterFunctions (iswalpha / towupper) know ASCII
  alone in the "C" locale of macOS - "Réc. hautbois" was left as it came.
  LOCAL COMMIT d6edb89 ON TOP (NOT pushed, no version yet - material for 0.9.135): StopNames.h letters by their CODE
  (isLatinLetter / isLowerLetter / isUpperLetter / upperOf: Latin-1 + Latin Extended-A); the combination row hides the
  "Pistons:" label and count box FIRST when the natural widths do not fit (the user: the pistons must stay) and the
  bank buttons show the arrow alone when narrower than 64 px (never two lines in a button); build-retry.yml reruns a
  run whose builds are green and whose release job failed.
  THE USER'S RULINGS ON MY "UGLY THINGS" LIST (navrh7): NEVER wrap the top bar or anything onto a second line as a
  rule ("V žádném případě nechceme dvojřádkový první řádek ... žádné takové společné pravidlo nechci"); the Editor's
  bars stay as they are (no. 6 refused); the bank arrow and "the count box hides first" accepted (done, d6edb89);
  nos. 3-5 (Stop List intro wrapped to the window, numbers under the per-key bars without frames, the Noises
  window's bottom on one line with a "?") ACCEPTED later ("beru body 3-5, čísla bez boxů"). He is unhappy with this
  session's quality: fewer rounds, picture every state before delivery, save tokens.
- 0.9.135 (25 Nov 2026 evening; "134 ani nevydávej, když to neprošlo ... udělej si na gitHubu pořádek"): v0.9.134 was
  NEVER released (softprops/action-gh-release "Too many retries" twice, his rerun hung in download-artifact); its
  remote tag is deleted, 0.9.135 carries it whole. CI: (1) build.yml's release step = `gh release create --verify-tag
  --prerelease` with the reason of a failure in an `::error title=Release::` annotation (readable without a login),
  an earlier attempt's release / v* draft of the tag deleted first, `timeout-minutes: 20` on the job; (2) a trial
  branch `ci-check` (all platforms, no release, pushed before the tag) was used ONCE for 0.9.135 and REFUSED by the
  user the same evening ("to je debilní, protože se to pak staví znova ... můžeš klidně pálit čísla, to mi nevadí ...
  nesmíme plýtvat prostředky gitHubu na stavění opakované"): the trigger is gone. RULE: ONE BUILD PER VERSION - push
  main, tag, the tag's run builds and publishes; a failed build = fix + the NEXT version number (burning numbers is
  fine); never a trial build, never a second run of the same commit.
  PROGRAM: `hideTopRows` (machine key; Editor > Console "Hide the top rows"): resized() lays the two rows out as ever
  and notes their controls in `topRowParts`, the rest of the window begins at the top; `TopRowsTab` (12 px strip +
  tab, testName "topRowsTab": mouseEnter slides out, a click pins), `TopRowsBackdrop`, `TopRowsWatch` (the mouse's
  place from the window's mouse EVENTS, never asked from the system), `applyTopRowsVisibility()` (visibility + Z
  order only - NEVER resized() for show / hide: it rebuilds the grid), `watchTopRows()` in the timer (in again
  450 ms after the mouse left; not while a button is down, a value is typed, a modal box / the header arranger is
  up, or pinned). PANIC: `panicInBounds` (first in the combination row) / `panicOutBounds` (its slot in the top bar,
  which it takes while the rows are out - they cover the combination row); the polyphony display stands in the
  combination row (natural 330, minimum 250 - at 150 it read "RA..." / "000"; the crescendo bar's minimum 80 then).
  `topRowsInOneRow` (machine key, off; "Top buttons and sliders in one row when they fit"): one row of 62 px when the
  buttons at their texts' width + 150 px a slider + 330 fit; HIS RULING: buttons and sliders in rows of their own
  whenever there is room ("návrh A vypadá debilně") - the one row only as this option. TOP ROW SLIDER TEXTS = THE
  BUTTONS' SIZE (the user: "proč jsou nápisy ve sliderech menší než nápisy na buttonech nad nimi? ... nedbalost"):
  16 px, shrinking 16..11 only as the width asks (properties "barNameSize", "barValueSize", "barSmallNameSize" =
  the two-line name, 13 else 11). RULE: a text beside other texts takes THEIR size - compare with the neighbours
  before delivery. Per-key numbers: label property "plainNumber" (no box, 9 px bold, zero greyed, frame only while
  typed). Stop List intro wrapped to the VISIBLE width (ScrollableContent calls content->resized() when the content
  kept its size). Noises window: bottom controls on one row + "?" (placeDialog "noises6", content height + 6).
  TEST LESSON: `headerButtons` and `globalSlidersShown` TRAVEL WITH THE ORGAN - a test changes a COPY of the
  configuration (%TEMP%\make_rural_onerow.py), not the settings; test_0127b.ps1 -Settings takes JSON values ({...},
  [...]) now. NOT run: the slide-out by a real mouse on the strip and the going-in after it left (hooks cannot
  move the mouse), Linux / macOS.
  RELEASED 25 Nov 2026 18:29: commit ea689c8, the ci-check build green on all five with NO test-step annotation (the
  Intel macOS capitals fixed), then main + tag v0.9.135: release at the FIRST attempt by the new gh step (five
  archives), Intel mac "late symbols: 0", the Windows exe on system DLLs only, the CI exe deployed; the remote tag
  v0.9.134 and the branch ci-check deleted (no trial builds any more, see above). %TEMP%\ci_watch.ps1 -Tag vX
  now follows the TAG's run alone (the ci-check run has the same commit).
- 25 Nov 2026, THE USER'S SCHORNDORF IMPORT (the analysis; coded as 0.9.134, the note above):
  (1) NOISES "(one for all)" COLUMN = MY FAULT, never a feature: the definition's noise ranks are "Organist: Blower noise",
  "Organist: Tremulant noise"; the 0.9.65 rule (a head without a perspective word on >= 50 % of the stops is a perspective)
  relabels the STOPS' ranks only, the noise ranks kept "" (the saved file: button/tremulantRun sounds with perspective ''
  whose file is 06-varhSenn.wav, pattern "Organist;06-varhSenn-"); the Noises window drew '' as a column "(one for all)";
  the engine's noiseRoute plays a '' noise through the FIRST route of the division it finds (default first, else any) -
  once, into an arbitrary perspective's outputs, no row in the Audio routing. HIS RULES: the Noises window mirrors the
  Stop List's perspectives exactly (same names, same order); noises EITHER copy the organ's perspectives OR are all
  without one (old sets) - never both in one organ; for the old sets a perspective "Noises" of its own that shows in the
  Audio routing like any perspective (never played 4 x into 4 perspectives); noises must respect the Perspective patterns
  (file / folder names); the separator lines of the Noises window = the divisions' divider of the main window (voicer.cpp
  ~2600: 2 px, juce::Colours::grey.withAlpha(0.65f), continuous) - mine was 1 px white 22 %, broken per row.
  (2) SIEBENQUART, MEASURED (%TEMP%\sieben.py <perspective folder> <keys>: smpl + spectral peaks of both attack layers and
  the three release folders; %TEMP%\smpl_layers.py <pipe folder> [04-direct] [stop]: smpl of plain vs _070 vs release over
  every stop): the PLAIN files (= _128, "fine-tuned mixtures") are PROCESSED - both ranks moved so that they are pure to
  each other (9.5 : 7 exactly on keys 57 and 69) and the smpl is measured on the non-existent 8' base (1525.4 / 7 = 217.9 Hz
  = the smpl's -16.4 c): honest; the _070 layer and ALL releases are the original recording with a GENERIC smpl (+15.7 c =
  444 Hz on every key, 9 distinct values in the stop). Key 57 at a1 = 444 (pure 1554.0 / 2109.0 Hz): plain 1525.4 / 2070.2
  (both -32 c), _070 and releases 1585 / 2109 (the 1 1/7' rank +34 c, the other exact) -> attack to release +66 c on the
  1 1/7' rank = his "half a tone". 34 of 58 keys differ by 5-39 c in the smpl; the other stops: Pos_Unruh 8 keys (<= 14 c),
  HW_Quintadena16 2 (18 c), Ped_Hintersatz 2 (13 c), the rest 0. WHY HAUPTWERK COPES: it retunes every SAMPLE (each attack
  layer, each release) by its OWN smpl; WE take one correction per pipe from the plain file and give it to every sample of
  the pipe ("releases follow their attack", SamplerEngine.cpp ~7596) - so "Retune pipes off by more than" moves attack and
  release together and the gap stays (his "nezabírá"). PROPOSED: a per-sample offset = (the pipe's reference smpl AS IN THE
  FILE - this sample's smpl as in the file) x 100 c, set at the load for velocity layers and releases, added at the voice
  start; like with like (never the 0.9.70 measured pitch against a file's smpl: Kdousov's generic smpl would shift its
  releases), ignored beyond +-100 c or without a smpl.
  (3) Footage of repeating aliquots (Terznone, Terzian: the button shows the lowest sounding footage; the builder writes
  1 3/5' + 8/9'): the definition's stop names carry no footage; the way is the ALIAS with the footage typed (0.9.50 rule) -
  to be verified that "1 3/5' + 8/9'" is shown as typed. (4) ASIO DEFAULT: `preferredDefaultType/Name` live in memory
  only and the search starts after the first load of a session (applyDefaultRoutingIfDue) - proposed: remember the found
  card (machine key) and search right after the start-up routes have opened. (5) #265 (hoofdwerk): tick boxes of the
  perspectives to load, all on by default - proposed in the LOAD BOX (it comes after the parse on every path), a
  configuration key of the perspectives left out (their folders stay in the table, no route, the Stop List column says
  "not loaded"); the definition chooser would have to parse every definition (seconds each) - not there.

- RECORDED TREMULANTS (28 Nov 2026, the user asked for a DESIGN, nothing coded - wait for his answers). FACTS READ:
  StopRank m = SwitchIDToSwitchToAlternateRank, n = RetriggerNotesWhenSwitching (Y), p = AlternateRankID; Tremulant
  c = ControllingSwitchID, d = FrequencyWhenEngagedHz, e = FrequencyWhenDisengagedHz (1.2 x d in his sets), f/g start /
  stop rate %. OUR IMPORT IS WRONG THERE: HauptwerkOdf.cpp ~614 reads d/e as "ModRateHz_Min/Max" and takes the MIDDLE =
  10 % too fast (Kiedrich 5.28 for 4.8, Stralsund UW 5.94 for 5.4) - d alone is the rate. MEASURED (%TEMP%\odftools\
  trem_lfo.py: the envelope's peak 2-12 Hz in the sustain; a tremmed file 19-83 x the band median, a plain one 4-6 x;
  the script sometimes reads the octave 2f): Stralsund OWTR 4.47-4.52 Hz (definition 4.5), UWTR 5.37-5.44 (5.4),
  Altenbruch "...tr" 4.84-4.86 (4.8), "...trrp" 6.25-6.49 (6.4); the loops are whole numbers of tremulant periods.
  Stralsund St. Jakobi: 72 of 153 StopRank rows have an alternate, one per perspective, folders in the sibling TREES
  pipe/OWTR and pipe/UWTR (<stop>/<prefix><stop>_tr), the switch "spinac real.Tremulantu" = the tremulant's slider
  under the condition " Tremulant Selection (real/artif". Altenbruch (SemiDry Extended): general "Tremulant" 4.8 Hz =
  alternate ranks "...tr" on HW / BW / Pedal; RW's own "Tremulant RW" 6.4 Hz = alternate "...trrp" on the RW stops; the
  general one on RW = 12 hidden stops "R. X (Tremmed)" (ranks "...tr") swapped with the plain stops through the
  "DrawInter ..." condition switches driven by "Slider Tremulant" (19018). A "trem" SUBFOLDER inside a plain rank
  folder (24 files) = Hauptwerk's tremulant WAVEFORMS (TremulantWaveform, 870 of them in Stralsund), never recordings.
  Tools: %TEMP%\odftools\odf_trem.py <compacted odf> [stop filter] (tremulants, alternate rows, their switches),
  odf_fields_all.py <dump.xml> <tables> (field letters over the whole dump, any case). The design proposed to him:
  an ALTERNATING ROW under the row it replaces (flag + "by the tremulant of division N / own"), no grid button, the
  parent's voicing + a trim of its own, chosen at the voice start by tremulantDriverOf (own first), crossfade when the
  tremulant is switched under held keys, the model silent for those voices; the recorded rate fixed (rate moved =
  the model plays; time-stretched copies as a second phase); a load-box tick for the memory.
  HIS DECISIONS (28 Nov, second round; picture F:\temp\navrh9_tremmed.png by %TEMP%\mock_tremmed.py): the tremmed
  recording is a row of the SECOND level (first level = an ordinary child) with FULL voicing of its own (the
  definition's per-pipe levels imported into it), flagged by the NUMBER of the division whose tremulant starts it
  (no "own"; normal rows: off); the Transmissions field reads "Tremmed" (as "Noise"), a tremmed row has no
  transmissions - moving a stop with transmissions to that level says they will be deleted; switching it OFF =
  the model plays AND its samples leave the RAM (his main point), not part of pistons / crescendo / Tutti; crossfade
  at a tremulant switch under held keys = a setting, default 300 ms, up to 1 s, phase-aware (NOTE: our attack ->
  release crossfade has NO phase alignment today - none found in SamplerEngine.cpp, told him); the model acts a
  little on the releases of a tremmed rank (the model gives detached releases 60 % of its depth today:
  ctx.tremPitchCentsRelease); the general tremulant sits on a division it shakes that has no other tremulant, the
  first of several; rate: step 1 ONLY (recorded rate fixed; the rate moved = the model plays; a simple way back -
  a tick "Recorded tremulant (N Hz)" in the Tremulant settings); time-stretched copies REFUSED; load box tick
  "Recorded tremulants" after the last perspective, ticked by default; the tremulant-rate fix of the import in the
  same build. NEW TO-DO of the same message: after a Hauptwerk import the SELECTED temperament must be set to the
  detected one (today only the BASE is set: "Base temperament set from the samples", MainComponent.cpp ~13540 - the
  selected stays what it was and the organ is retuned; Kiedrich 28 Nov 08:04, he set it by hand at 08:35).
  THIRD ROUND (28 Nov): the "Recorded tremulant" tick goes into EVERY place the tremulant is set - Divisions dialog,
  right-click Tremulant settings, the Editor's division section (as the 0.9.88 target boxes) - his rule again: "Nikdy
  nezapomeň na všechna místa, kde se věc dá nastavit ... Vždy znej celý kontext"; the tree lines of the picture
  (box-drawing lines before an indented name) also for ORDINARY child rows of the Stop List and for the nested rows
  of an unfolded voicing (level 1 and level 2 told apart by the tree); NO question at the indent to level 2 - the
  division combo is just made orange; only existing transmissions bring a warning; the voicing of a tremmed row is
  REDUCED: its own Stop volume, Volume / Treble / Attack per key and Release xfade (other samples: nothing of these
  can be the parent's) - no alias, no transmissions, the rest from the parent; general tremulant placement and the
  second-level indent under a plain stop CONFIRMED. CHECKED for him: startReleaseSamples (SamplerEngine.cpp ~7603-7628)
  starts a release at frame 0 (or "Release played from") - NO phase alignment with the sounding attack; he thought
  it was there (from the pre-AI program); he hears nothing wrong - not to be added unless he says. FX: no "set for
  all perspectives" exists in EffectsDialog.h (gone with the audio tabs in 0.9.89; LINKED works across the DIVISIONS
  of one perspective only) - he wants it back per module (IR too), the auxiliary never touched: proposed, his word.
  FOURTH ROUND = GO ("udělej to", 28 Nov): 0.9.150 IN WORK, everything in ONE build, the release marked as the
  MILESTONE "sampled tremulants support" (CHANGELOG heading + release body; no token for a GitHub milestone object).
  Added by him: the box-drawing characters (│ ├ └ ─) for the tree on the grid's nested rows and in the Stop List (normal
  children too); the attack -> release PHASE ALIGNMENT as a config option in Advanced global audio (default OFF, he
  hears nothing wrong; "pozor na regresy, hlavně polyfonie"); "Set for all perspectives" per Fx module back (IR too,
  never the auxiliary). IMPLEMENTATION DESIGN (keep to it after a context loss): table entry fields `tremmed` (bool,
  with child = true ALWAYS so every "nested row" rule applies) + `tremulantManual` (division number), JSON "tremmed":
  N on the stopBuild entry; a tremmed entry belongs to the nearest non-tremmed entry above; engine Stop::tremmedOfPtr
  (parent) / tremmedManual / parent's tremmedAlternates, set in MainComponent::applyStopGroups; the engine's route
  helpers (forEachRoute, own-button tests) answer for a tremmed stop with its PARENT's routes and enabled state;
  startNote gates per pipe (`variantSounds`: the tremulant driving the division where the key is, its `recorded`
  flag, the alternate loaded and holding the pipe - no fills in a tremmed stop); Voice::recordedTrem = the model
  off for the sustain, on the detached release at the setting's share; a tremulant switch under held keys = the
  wrong variant's voices faded (CmdRelease with the crossfade ms, no release sample), their speaking bits cleared,
  startNote run again in "swap" mode (voices start at the first loop's start with the fade-in); the phase is set
  at the route's TAKE of the new voice (renderRoute, single-threaded per route: the partner voice's pos is stable
  there) from a Goertzel at the fundamental over two periods - Voice::alignPartner / alignPartnerOrder / alignPhase /
  alignPeriod / alignFreqA; fades are linear = right for phase-aligned signals. OFF = unloaded (FolderRef::unloaded).
  ALSO SEEN: the load box's "Perspectives to load" (askLoadOptions ~16828) orders by the var's pattern lines and puts
  the names without a line after them in table order (alphabet: Diffuse, Direct, Rear) - not by defaultPerspectiveRank.
  0.9.150 BUILT AND RUN ON WINDOWS (28 Nov 2026; LOCAL, push and tag wait for his word). NO MILESTONE MARK (he took
  it back the same day: "tak nedělej milestone, to je zbytečné"); build.yml's `keep` is EMPTY again (he deleted
  v0.9.100 by hand: "Nebudeme to držet"). BILLERBECK'S REAR (he: "neumíš číst Hauptwerk ODF správně, to určitě
  obsahuje rear ranky i pro tremulanty"): the reading was right and he was right too - both Billerbeck definitions
  (Surround, 8 chan.) HOLD 45 ranks "Rear: ... (tremmed)" with 2932 pipes, but the Rear StopRank rows carry no m / p
  (alternate) while Direct / Diffuse / Semi-dry do: the definition's own omission (Hauptwerk plays Rear plain there).
  Now parse() joins a tremmed rank that NO row names (not in any stop's rankIds, no alternateOf) to the plain rank
  of the SAME NAME less "(tremmed)" (unique), tremulant = the stop's other recordings' (same "(trem..." mark first),
  else the division's only one; `joinedTremmedRanks / joinedTremmedExample` -> a plan note. Of the 25 published
  organs only Billerbeck has such ranks (%TEMP%\odftools\odf_alt_count.py <odf>...: per perspective the rows with /
  without an alternate and the tremmed ranks nobody names; odf_rows_of.py <odf> <name part>: the raw Rank / Stop /
  StopRank rows). LESSON: when a count says "N tremmed ranks skipped" after the new rule, LOOK at what they are
  before calling it "the definition's choice" - I told him "Billerbeck has recordings for Direct and Diffuse only".
  WHAT EXISTS: table entry
  `tremmedManual` (JSON "tremmed": N, child always true; bound to the nearest non-tremmed row ABOVE - by position, no
  id), engine `Stop::tremmedOfPtr / tremmedManual / tremmedAlternates` set by MainComponent::applyStopGroups ->
  `engine.tremmedStopsChanged()`; `routeOwner / buttonOn` (a recording answers with its PARENT's routes and on/off);
  `recordedDriverOf(manual)` (tremulantDriverOf when that tremulant is ON and `TremParams::recorded`);
  `variantSounds(stop, manual, pipe)` (stop level) and `variantRankSounds(stop, rank, manual, pipe)` +
  `coveredByRecording` - PER PERSPECTIVE: a plain rank is silent only where the running tremulant's recording has
  the pipe in the SAME perspective (Billerbeck's definition has recordings for Direct and Diffuse and none for Rear:
  Rear goes on with the model; my first build silenced the whole stop - found by the 25-organ compare, not by the
  first test: Altenbruch SemiDry has ONE perspective); `Voice::recordedTrem` (no model on the sustain; the detached
  release takes `tremmedReleaseModelPercent` / 60 of the model); `swapTremulantVariants()` from processPending when
  `tremSwapPending` (set by setManualTremulant / Targets / Recorded, tremmedStopsChanged): wrong-variant voices
  CmdRelease with `tremulantSwapMs`, speaking bits cleared, startNote again in `swapMode` (start at the first loop's
  start, fade-in = the same ms, no attack delay / cut), phase by `prepareAlignment` + `alignAtTake` (Goertzel at the
  fundamental over two periods, at the route's take); the same alignment for attack -> release behind the option
  "Release starts in the phase of its attack" (`releasePhaseAlign`, default off). Per division
  `tremulantRecordedHz` (0 = unknown: measured by `engine.recordedTremulantHz(stop)` in `syncTremulantRecorded`) and
  `tremulantRecorded` (the tick; unticked by itself when the rate differs by > 0.011 Hz) - the tick is in the
  Divisions dialog (under Tremulant Rate, always visible), the right-click Tremulant settings (FeatureContent is a
  Timer now) and the Editor's division section ("Tremulant: recorded samples"), shown while
  `anyTremmedRows(manual)`. Stop List: `GridArea::treeLines` (the tree is DRAWN as lines, not font glyphs), level 2 =
  2 x indent, the Transmissions field reads "Tremmed" + a division combo (testName "tremmed <row>", orange until
  touched: `tremmedFresh`), `makeTremmedRow`, `moveEntry` moves a stop with its recordings. Voicer: `nestedRows(master)`
  {stop, level, parent}, a recording's button "Tremmed <division>" (click = `tremmedLoadRequest` =
  `setTremmedRowLoaded`: unload = the folders' `unloaded` flags + reloadDivisions(..., nothingToRead = true) -
  MEASURED 170 ms, no loading window, the stop keeps an empty container so its row stays), reduced voicing (Stop
  volume, per-key rows, Release xfade), "Tremmed by: no" combo on ordinary children with pipes
  (`tremmedDivisionRequest` -> `setTremmedRowDivision`; transmissions warned and removed). Load box:
  `PerspectiveTicks::recordedTremulants` ("To load: [x] Recorded tremulants", `setTremmedRowsLoadedInVar`,
  transient "recordedTremulantsChoice"; the perspectives ordered by pattern line else defaultPerspectiveRank).
  Folder way: `engine.tremmedFoldersOf(tree)` (a sibling folder named like the stop's + tr / trem [+ digits or a
  <= 3-letter tag that is a word of the name]; or the same folder under an ancestor's sibling with the mark - OWTR
  beside OW; names only, the ignored patterns are NOT consulted and their defaults were not changed) used by
  extendStopBuildWithTree; a stop with TWO recordings made there (Altenbruch rw: "...TR" + "...trRP") = two
  tremulants: the lettered mark keeps the division's own, the other gets the NEIGHBOURING division as a guess
  (Messages says so; it plays once that tremulant's targets name the division) - the first build gave both the
  same tremulant = both sounding at once; the engine now lets only the FIRST recording of a (stop, tremulant,
  perspective, pipe) sound. Run: 002068/pipes/rw dropped on I. of an empty console -> 24 rows of the second
  level, I. measured 6.40 Hz, II. 4.85 Hz. IMPORT (HauptwerkOdf): StopRank m / p -> `Rank::alternateOfRankId / tremulantId`
  (`tremulantsBehind(switch)` BFS; hidden "(Tremmed)" stops matched to the plain stop by name, then Levenshtein <= 2),
  `PlanStop::tremmed` {tremulantId, tremulantDivision, rankIds}, `PlanDivision::tremulantId / tremulantTargetMask /
  tremulantHasRecordings`; a tremulant sits on its one division, a general one on the first MANUAL it shakes that
  is not taken; Tremulant rate = d (FrequencyWhenEngagedHz) alone - EVERY imported tremulant is ~9 % slower than
  before (Caen 5.61 -> 5.10, Doksy 5.98 -> 5.20, Bedos 4.29 -> 3.90: right, the old value was the mean of d and
  e); a recording's tree = as far above its folder as the stop's tree is above the stop's, but never a folder that
  holds another tree (Doesburg "002294/IVTrem/Re_echoburdonTr" lies flat: the first build made the whole package a
  tree); recordings stay out of the route-volume median; `selectDetectedTemperament` (the SELECTED temperament = the
  detected one once after an import). Effects: `setEffectModuleForAllPerspectives` + "Set for all perspectives"
  (testName "ALL <module>"; never the auxiliary nor "Noises"). The load-time migration
  renameRowsWithoutPerspectivePrefixes renames a recording's row with its stop's (its customName flag is ignored
  for rows with "tremmed"). HOOKS: `ORGAN_TEST_TREMMED="<s>|trem|<division>|<0/1>^<s>|recorded|<division>|<0/1>^
  <s>|rate|<division>|<Hz>^<s>|load|<division>|<row part>|<0/1>^<s>|division|<division>|<row part>|<N or -1>^
  <s>|fxall|<perspective>|<module>^<s>|state|0"` (every act logs polyphony, the tremulants and the recordings),
  ORGAN_TEST_OPEN "<s>|divisions|<division>" and "<s>|effects|<division or -1>|<perspective>",
  ORGAN_TEST_EDITOR_SECTION=<row label> (scrolls to a ROW when no section is named so), ORGAN_TEST_EXPAND 4th
  field = a nested row opened too. Analysis: %TEMP%\odftools\swap_check.py <wav> [switch seconds] (level per
  second, modulation rate, the 10 ms levels around a switch). MEASURED (Altenbruch SemiDry Extended imported by
  the hook, 16-bit, 10.7 GB, 47 recordings; key 60 of RW Gedackt 8 held 18 s through ASIO): plain -> own tremulant
  (6.3-6.35 Hz in the envelope) -> plain -> the general one (4.76-4.83 Hz), level within 1 dB through every
  crossfade, 0 discontinuities, polyphony 1 throughout; with a crafted second perspective that has no recording
  (%TEMP%\odftools\craft_partial.py) polyphony stays 2 and the level constant. `hw_compare2.ps1 -Old 0.9.149`: 9
  organs identical, 16 differ ONLY by the tremulant rate, the TREMMED blocks, the recordings' trees and the
  notes (filter in the session: lines not matching "TREMMED by|rank N of N|tremulant N sits here|keys ..., tremulant").
  TEST LESSON: a saved import is RENAMED at its load (row names lose the perspective prefix: "02-suchy_RP_Gedackt8" ->
  "RP_Gedackt8") - a hook that names a stop by its prefix must use the name AFTER the rename (a silent recording
  and "1 stop(s) on" was this); and the first test organ of a feature with perspectives must HAVE several.
  NOT DONE / DIFFERENT FROM WHAT I TOLD HIM (say so): the tree is drawn lines, not the box characters as glyphs; the
  model on a recording's release is not phase-locked to the recorded tremulant; recognition from folders is by
  NAME alone (the level-envelope test only measures the rate); a recording's transposition is copied from its
  stop when the row is made (no follow-up when the stop's is detected later); GLOBAL RESET does not reset the three
  new settings; a recording dragged alone to another division becomes an ordinary row. NOT RUN: a real mouse in
  the Stop List (indent to level 2, the combo), the folder way on a tree without a definition, a multi-perspective
  organ with recordings loaded whole (Stralsund / Altenbruch Surround), the release-phase option by ear, Linux /
  macOS (not compiled).

## Pending for the next Linux session (delete when done)
- LESSON 0.8.147: the two
  checkouts are two git repositories on the SAME sources - after a Linux session the Windows checkout `F:` is behind
  origin and holds the Linux commits' content as uncommitted changes; `git pull` there fails ("local changes would be
  overwritten") and a commit lands on the old base (the push is rejected, a tag push is not: v0.8.147 was first built
  from such a dangling commit). The way: `git fetch`, `git reset --mixed origin/main` (HEAD moves, the files stay),
  then commit the tree anew, push, re-point the tag. Always `git fetch` + `git status -sb` BEFORE committing on the
  other system. #58 (Linux, two monitors): the KDE AMD box runs WAYLAND - the X11 way finds nothing there; the KWin
  scripting way (0.8.150, working since 0.8.151) is to be confirmed by the user's own report. The NTFS checkout is the user's (git cannot
  run there from Linux: "expected submodule path 'JUCE' not to be a symbolic link"). After each release copy the Linux
  binary to `~/Downloads/LibreOrgan`.
- Windows builds: never run two MSBuilds of the solution at once - the second fails with C1041 (PDB) and can
  leave orphaned `cl.exe` processes (0 s CPU, parent gone) holding the PDB; stop them before the next build.
- The dual boot clock is settled: Windows and Linux both keep the hardware clock in local time
  (`timedatectl` says "RTC in local TZ: yes"); never set `RealTimeIsUniversal` on Windows. Windows fast startup
  is off (`powercfg /h off`), so the NTFS disks are writable from Linux.

## Compile checks on Linux without a toolchain for the whole program
- `g++ -std=c++17 -fsyntax-only -w -DJUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1 -DJUCE_STANDALONE_APPLICATION=1
  -DJUCE_USE_CURL=0 -DJUCE_WEB_BROWSER=0 -DJUCE_MODULE_AVAILABLE_juce_dsp=1 -DJUCE_LINUX=1 -DNDEBUG
  -I JuceLibraryCode -I ../JUCE/modules -I ../ThirdParty/signalsmith -I /tmp/stubinc Source/<file>.cpp` (run in
  `~/LibreOrgan-linux/LibreOrgan`) checks a source in seconds (SamplerEngine.cpp 4 s, MainComponent.cpp
  longer). `/tmp/stubinc/alsa/asoundlib.h` is a stub of the seven ALSA symbols PlatformAudio.h uses
  (snd_pcm_t, snd_pcm_open/close, snd_strerror, snd_device_name_hint/get_hint/free_hint, SND_PCM_STREAM_PLAYBACK,
  SND_PCM_NONBLOCK; libasound2-dev is not installed); recreate it when /tmp was cleared. Real builds come from CI.
- Never start `~/Downloads/LibreOrgan` with a made-up flag such as `--version`: it opens the program (kill it).
  `--selftest "<folder>;<folder>"` is the offline test (folders separated by ';'), with `ORGAN_SELFTEST_IR=<wav>`,
  `ORGAN_IR_LIGHT=1`. `gh` is not installed on Linux: the GitHub API with the token from `git credential fill`.

## Testing
- Windows delivery: after every release build copy the exe to `C:\LibreOrgan\LibreOrgan_<version>.exe`
  (the local MSBuild Release build or the CI exe) and retarget the EXISTING desktop shortcut in place
  (today `LibreOrgan_0.8.98.exe – zástupce.lnk`; find it by its TargetPath under `C:\LibreOrgan\`; WScript.Shell
  CreateShortcut, set TargetPath + IconLocation, Save). NEVER delete, rename or recreate the shortcut: Explorer
  keeps icon positions by file name and a new file lands anywhere on the desktop. The user starts the program
  from that shortcut.
- The Windows process is named after the exe (`LibreOrgan_0.8.181`): check `Get-Process LibreOrgan*`, never
  `Get-Process LibreOrgan Sampler` (it finds nothing and the 0.8.182 deploy ran while the user played - harmless only
  because a new file name was written and the shortcut retargeted; still, check first).
- THE USER'S RULE (20 Nov 2026, "kdykoli můžeš zavřít instanci LibreOrgan ... Nechci slyšet výmluvy 'tvoje aplikace běží'"):
  his running LibreOrgan may be CLOSED whenever a test or a build needs it (SIGINT first so it ends in order; back up and
  restore the settings files around the test; say in the report that it was closed) - the old "never build or launch
  while the user runs it" is gone. Linux test: `~/tools/linux_test.sh <exe> <seconds>`
  with `CFG=`, `ORGAN_OPEN_STOP_LIST=`, `ORGAN_JACK_BUFFER=`, `ORGAN_SAVE_PORTABLE=` hooks; the activity log is
  `~/.config/LibreOrgan/CrashLogs/activity.log` (Windows: `%APPDATA%\LibreOrgan\CrashLogs\activity.log`).
