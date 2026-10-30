# OrganSampler - rules for every session (Windows and Linux)

The user writes Czech; answer in Czech; append a closing report to `OrganSampler/CLAUDE_REPORTS.md`
(the copy on the NTFS disk: `F:\JUCE_PROJEKTY\OrganSamplerC` = `/mnt/pracovni/JUCE_PROJEKTY/OrganSamplerC`).

## "go" and the to-do list
- THE USER'S RULE (26 Oct 2026): "go" means "continue, I have topped up the money" - never "do this one item now".
  "todo:" messages are collected (CLAUDE.md pending list); when he says to do them, ALL of them go into one build and
  one version. 0.8.213 was first built from one to-do alone because a message ended with "go" - wrong.

## Never change what you need not change
- The user's rule (23 Oct 2026, "Nikdy neměň něco, co nemusíš!"): a fix touches the lines of the fault and nothing
  else - no renaming, no "while I am here" rewrites, no new defaults, no changed texts or separators. When a change
  outside the fault seems needed, say so first. The automatic stop name "A / B" (two perspectives with differently
  named folders) dates from the first commit, not from a session - before blaming a session, `git blame` the line.

## Icons - do not touch without asking
- The icon set is one per size and lives in git: `OrganSampler/Source/Icons/icon-{16,24,32,48,64,128,256}.png`
  (16-128 hand-tuned: outlines, the smallest with thick pipes; 256 the picture), `OrganSampler/Source/AppIcon.png`
  (1024, the website picture), `OrganSampler/OrganSampler_icon.ico` (all seven sizes, PNG entries),
  `OrganSampler/Source/Icons/OrganSampler.icns` (sixteen entries: the classic RLE types is32/il32/ih32/it32 with
  their masks for 16/32/48/128 - the Finder of macOS 10.7-10.13 shows PNG data in `icp4`/`icp5` as pixel garbage,
  so those two are never written - and PNG ic07-ic14 for 128-1024 and the @2x sizes; built by
  `python OrganSampler/Tools/make_icns.py OrganSampler/Source/Icons OrganSampler/Source/AppIcon.png
  OrganSampler/Source/Icons/OrganSampler.icns` from icon-16..512.png + AppIcon.png), `site/assets/icon.png` (= AppIcon).
- JUCE (CMake and Projucer alike) would rescale two of them into blurred icons: `CMakeLists.txt` overwrites JUCE's
  generated `icon.ico` / `AppIcon.icns` with ours after `juce_add_gui_app`; the jucer's VS exporter copies the .ico
  by `postExportShellCommandWin`, which `Projucer --resave` does NOT run - after a resave on Windows copy
  `OrganSampler/OrganSampler_icon.ico` over `OrganSampler/Builds/VisualStudio2026/icon.ico` by hand.
- Never replace the icons by a rescaled or "improved" set. If a picture changes, rebuild the 256/1024 entries
  only and keep 16-128 as they are. Linux (menu entry, window icon) uses the same files.

## Versions and releases
- Every delivered build is a new version. Bump it in ALL of: `OrganSampler/OrganSampler.jucer` (`version=`),
  `CMakeLists.txt` (`project(... VERSION ...)`), `site/config.js` (`version:` - app.js writes it into the page),
  `site/index.html` (`data-version` spans), `configurations/rural-organ/README.md` ("Saved with OrganSampler ..."),
  `OrganSampler/CHANGELOG.md`. Before tagging, grep the previous version string over the repo (CHANGELOG excepted).
- Regenerate `OrganSampler/Source/ManualData.h` after any manual edit: `python OrganSampler/Tools/generate_manual_data.py`.
- Commit (plain commit messages: no Co-Authored-By trailer, no assistant name anywhere in the repository - the
  user is the only contributor), push, tag `v<version>`; CI builds Windows,
  macOS (Apple silicon, macOS 11+), Intel macOS 10.11+ and Linux and publishes a pre-release; the release job deletes
  every older `v*` release and tag. The Rural Organ demo release `rural-organ-0.8` stays. Only Sonus Paradisi is ever
  named for samples; never "sample set" - say sample folders / folder tree.
- Intel macOS build (10.11+): never add `-D_LIBCPP_DISABLE_AVAILABILITY` (it makes the binary import libc++
  symbols of the build machine's macOS - dyld "Symbol not found" on 10.12-10.14). After each release download
  `OrganSampler-macos-10.11-intel.zip` into a fresh temp folder and run
  `python -I OrganSampler/Tools/check_macho_imports.py <app>/Contents/MacOS/OrganSampler`: it must end with
  "late symbols: 0".
- Windows commit messages: write the message to a file WITHOUT BOM (`[IO.File]::WriteAllText(path, text)`) and
  `git commit -F <file>`; `-F -` with a here-string does not work in PowerShell 5.1 (the text becomes pathspecs,
  the commit fails and a following `git tag` lands on the previous commit), and `Set-Content -Encoding utf8` adds
  a BOM to the subject line. Tag only after `git log -1` shows the new commit.
- Windows: `git` is not on PATH - use Visual Studio's
  (`C:\Program Files\Microsoft Visual Studio\18\Community\...\Team Explorer\Git\cmd\git.exe`); after pulling,
  run `C:\JUCE9\Projucer.exe --resave "F:\JUCE_PROJEKTY\OrganSamplerC\OrganSampler\OrganSampler.jucer"` with the
  ABSOLUTE path and copy the .ico (above) (with a relative path the resave prints nothing and writes nothing - four
  tries in 0.8.145; check `versionString` in `JuceLibraryCode/JuceHeader.h`, it must print "Finished saving"), commit the regenerated
  `Builds/VisualStudio2026` and `JuceLibraryCode`. Linux clone: `~/OrganSamplerC-linux` (git cannot run on the NTFS
  checkout from Linux); Linux builds come from CI only: `OrganSampler-linux-x86_64.tar.gz`, unpacked to
  `~/Downloads/OrganSampler`. Releases carry the native archive of each system, never a bare executable (0.8.110):
  `OrganSampler-windows.zip` (OrganSampler.exe inside), `OrganSampler-macos.zip` / `OrganSampler-macos-10.11-intel.zip`
  (the app), `OrganSampler-linux-x86_64.tar.gz` (keeps the executable bit); names without a version. On Linux `gh` is NOT on
  PATH: use `~/tools/gh/bin/gh` (export PATH="$HOME/tools/gh/bin:$PATH"); never run `~/Downloads/OrganSampler` with
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
  the flat pair into `configurations/` on branch `configuration/issue-<n>` and opens the pull request
  (closes the issue on merge), answering in the issue on success or failure. The check accepts that flat
  pair; after the merge the `index` job moves it into `configurations/<slug>/` (README.md + zip) and
  regenerates INDEX. `.github/PULL_REQUEST_TEMPLATE.md` is the checklist for hand-made pull requests.
  Repository setting needed (set 15 Oct 2026 via the API, `actions/permissions/workflow`): "Allow GitHub
  Actions to create and approve pull requests" ON - without it `gh pr create` fails with "GitHub Actions is
  not permitted to create or approve pull requests". The label `configuration` must exist (it does).
- `configurations/<organ>/` holds exactly two files: the zipped `.json` and `README.md`. No IR files, no pictures,
  no bare `.json`, nothing else (impulse responses belong to the sample folders); the CI check refuses anything else.
- Transmissions grouped under a stop folder are NOT table rows: they live in the source stop's voicing entry
  (`voicingSettings.stops[].transmissions[]` with `groupMaster`). Search there before claiming they are gone.
- The transmission list of a stop belongs to the Stop List / voicing state only. A combination, crescendo step
  or registration sets on/off states and must never replace, add to or remove from the list (0.8.92).
- The voicing state keeps entries of stops the engine could not build (missing folders) and writes them back;
  never let a save drop voicing of a stop that merely failed to load.
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
  Dev check without the GUI: `OrganSampler.exe --hauptwerk "<odf>"` prints the parse and the plan (also to
  `%APPDATA%\OrganSampler\hauptwerk.txt`); the in-app path is `ORGAN_LOAD_CONFIGURATION=<odf>` + `ORGAN_DUMP_STOPLIST=`.
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
  image); the desktop-entry installer also writes ~/.local/share/pixmaps/organsampler.png (48) for antiX-like WMs.
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
  Editor in the corner" too, most likely). `LinuxWindowPlacement::placeThroughKWin`: a JS file in ~/.config/OrganSampler
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
  README.md, the release body in build.yml, configurations/README.md and the Rural Organ README said "unpack it -
  Unpack archive button" and "300 000 voices" long after 0.8.157 (RAR read in place: Load Configuration opens the
  .rar) and 0.8.146 (500 000); the README said 950 MB for a 725 MB demo. When a feature changes what the user DOES
  (loading, unpacking, limits), grep the site, README, build.yml body and configurations/*README* in the same session.
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
  account takes videos up to 10 MB ("Failed to upload" above): make_video's organsampler-github.mp4 (1280 px, crf 30,
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
  set (the Aeolian files are numbered 024 = key 36 on every stop). Commits from ~/OrganSamplerC-linux (the NTFS checkout edited,
  files copied over); NO Projucer resave on Linux - do it on Windows next (the VS2026 files lag 0.8.165).
  `OrganSampler/CLAUDE_REPORTS.md` is GITIGNORED (local only, never committed): a `git add` naming it fails, and a
  chained `add && commit; ... && git tag` then tagged the OLD commit (v0.8.169 first landed on 83e888a: the run
  cancelled, the tag deleted and re-pushed on 5685fd9). Never chain the tag after the commit in one command: tag in
  a separate step after `git log -1` shows the new commit, on Linux as on Windows.
  24 Oct 2026 (0.8.179): a push to main was REJECTED - the "Configurations check" index job commits to main by itself
  after any change under configurations/ (the old Stralsund folder removed -> "INDEX.md regenerated" on origin) - and
  the tag push went through, so v0.8.179 pointed at a commit off main (the 0.8.147 situation again). Resolved by a MERGE
  (`git pull --no-rebase`), never a rebase, so the tag stays an ancestor of main. Always `git fetch` + `git status -sb`
  before a push that follows a configurations/ change, and push main BEFORE tagging.
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
  decoder). Cache `%APPDATA%\OrganSampler\ArchiveIndex\<hash>.idx` v2 ("V main|end" per volume, "D path",
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
  is OrganSampler/Finder/empty; `AfterReportWindow::resized` puts its size back). `ORGAN_TEST_REPORT_WINDOWS=<folder>`
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
  configurations/<folder>, runs configurations_index.py, commits and pushes to main (refuses rural-organ, paths with
  "/" or "..", and a folder that does not exist - it lists the folders). Documented in configurations/README.md.
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
  dialog, a row widget or a caption without such a run is not done. And the process name is OrganSampler_<v>:
  `Get-Process OrganSampler*` before any test run (the user plays through the day).
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
  (producer, product, link, packages, WAV, perspectives, required OrganSampler version), Expects (trees), Audio
  layout (routes, unused ones marked), RAM = ONE figure since 0.8.186: the least the organ takes with every saving
  option (measured when saved so, else the estimate) + "at the full quality count on roughly ten times as much" -
  never two figures (two columns of the same 7 GB were "absurd"); INDEX.md has the one "RAM" column the same way
  (Rural Organ keeps its measured 1.2 GB, no estimate in its file). A PUBLISHED configuration opens with everything
  OFF: `savePortableConfiguration` with publishFiles switches every stop, transmission, coupler and tremulant of the
  registration off (the user's rule, 24 Oct 2026). (The old form of the line: RAM measured by the program at the portable save: `measuredRamBytes` +
  `measuredRamStorage` in the json, `processMemoryBytes()`; INDEX.md shows it as a sortable column - never typed,
  never estimated), Saved with, Author + date; "Where to put the file" (the packages are now
  known: one sentence + the tree diagram, never a made-up parent folder like `haupwerk`); last the "Stop List"
  table of the row names by division. Never mention what OrganSampler does natively (couplers, voicing,
  crescendo, combinations). No "What you need", no "Using it", no prose about the divisions.
- Configurations live ONLY in `configurations/<organ>/` on GitHub as `README.md` + `<Name>.zip` (the one `.json`
  inside; a zip downloads from GitHub, a bare `.json` opens as text - never "Download raw file" instructions).
  The portable save writes `<name>.zip` and `<name>.README.md` beside the `.json`; the README's Download line
  is `[<Name>.zip](<Name>.zip?raw=true)`. The list of all organs is `configurations/INDEX.md` (README link,
  divisions, Stop List rows, routes: the perspectives as named - `default` stays `default`, never "-" - plus
  `auxiliary` when a division uses it; what they carry is the organ README's job, e.g. Rural Organ: default =
  front, auxiliary = rear; a Markdown table cannot be sorted by a click, so the header's ▲▼ are links to the
  pre-sorted copies `INDEX-<column>-<asc|desc>.md`, all generated), written by `.github/scripts/configurations_index.py` in the
  `Configurations check` workflow (job `index`) and committed back - never a list on the website, never a
  release for configurations, never a list in `configurations/README.md` (there will be thousands). The
  website section and the release body only link to INDEX.md. Configuration file names may have spaces (0.8.195:
  the robot names the pair after the .json inside the zip, whatever GitHub did to the attachment's name); the
  README's Download line is `https://github.com/zurek-jiri/OrganSampler/raw/main/configurations/<organ slug>/<name>.zip`.
- Stop List counting: every row counts - stops, stop folders, transmissions and extensions (transmissions
  are rows of their receiving division, `transmissions[]` with `manual`) - except the rows nested in a stop
  folder (`groupMaster` set). The table is NEVER typed by hand: it comes from the program, in grid order (row by
  row, left to right - the organ builder's order the user arranged) with the exact button captions (footages,
  "(Ext.)"/"(Trsm.)", a stop folder's "[III]"):
  `ORGAN_DUMP_STOPLIST=<out.md> ORGAN_LOAD_CONFIGURATION=<cfg.json> OrganSampler.exe` writes it once the load is
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
  F:\Suchy_rejstrik\Kobrle\Rural Organ.json and in configurations/rural-organ/Rural-Organ.zip's json (%TEMP%\rural_tour.py).
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
  beside the zip still accepted; the form, configurations/README.md, the manual, the site and the publish window say
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
  organsampler.mp4: x264 crf 20, 30 fps, 1920 wide; organsampler.gif: 8 fps, 960 px, 128 colours);
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
  organsampler.gif under it linked to the mp4 on the Pages site (a GitHub README renders no <video>). (5) `portableFolderCandidates` adds `<child>/<relative>` for the children of a location (0.8.202's
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
  `C:\Users\<you>\AppData\Roaming\OrganSampler`, macOS `~/Library/OrganSampler`, Linux `~/.config/OrganSampler`; the
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
  configurations/. `configuration-remove.yml` reads the dropdown ("a, b" = several; "_No response_" = none) and the
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
  Configurations folder: 10 trees, 63 stops added. (3) `perspectiveHeadsOfEntries(entries)` (static): label -> the
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
  (do it on Windows next). Syntax check: g++ -fsyntax-only with -I ~/OrganSamplerC-linux/JUCE/modules (the NTFS
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
  configuration with the OLD exe (C:\OrganSampler\OrganSampler_0.8.223.exe) and the NEW one, diffed - Billerbeck
  may change the two 32' captions only. WINDOWS CHECKOUT AFTER THE LINUX SESSION (0.8.224 was committed from
  ~/OrganSamplerC-linux): `git fetch` + `git reset --mixed origin/main` as the LESSON says, BUT origin also carried
  the user's new published configurations (bergen-op-zoom-ibach-organ, beziers-cathedral-puget-organ, INDEX*.md,
  the generated remove form) which then show as DELETED/modified in the working tree - `git checkout --
  configurations/ .github/ISSUE_TEMPLATE/organ_configuration_remove.yml` restores them BEFORE any `git add -u`
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
  the chooser starts in `configurationsDirectory().getParentDirectory()/AudioRecordings` (created on demand). Wind
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
  `addButtonWithNote` (the path AFTER the button: cache, cache folder, Configurations folder, Recordings folder,
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
## Pending for the next session (delete when done)
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
  MIDI side killed the process. TO-DO (same build): AudioRecordings folder beside Configurations for the Record
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
  since 0.8.190 the configurations live in Documents\OrganSampler\Configurations away from the samples, so
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
  - the folder `configurations/<organ slug>/`; `same-organ` = folders of that slug or whose json has the same organ;
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
  measuredRamBytes > 0, no MIDI mappings; the README's title = the zip's name, "Written by OrganSampler", no "(fill
  in:", a Stop List table whose cells equal the "N rows" claim, the divisions, every `sampleTrees` entry named, the
  organ, the Download link's folder = the slug (an older README is refused: "save it again with 0.8.188"). Tested
  on the user's new pair (valid, slug stralsund-st-jakobi-kr-wegscheider, replaces the old folder), both published
  folders, a zip of config.js (refused). README RAM line 0.8.188: not saved at the least -> "; measured N GB when this
  file was saved, the organ loaded as <measuredRamStorage>" beside the estimate. Report folder 0.8.188:
  `settingsFile.getParentDirectory()/Reports/OrganSampler-report-<stamp>` (never the Desktop again). FOUND BY THE
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
  specific key; `configurationsDirectory()` = it or Documents/OrganSampler/Configurations, created on demand) - the
  Hauptwerk import retargets `plan.configurationFile` there (HauptwerkOdf::configurationFolderFor still computes the
  old default, unused by MainComponent), Save/Load/Merge choosers start there. `sampleLocations` (machine-specific
  StringArray, mirrored into the anonymous-namespace `gSampleLocations` for `portableFolderCandidates`, which appends
  `<location>/<rootName>/<relative>` and `<location>/<relative>` after the old candidates, and the absolute-only
  tails under every location). Filled by `noteSampleLocation`: the root's parent at a portable save, the root found at
  a load (`found` minus the relative path's depth; its parent when named like the root), the folder of a chosen
  archive (registerArchive), the first mismatch from the end between the old and the new path in the "Replace ... with
  a folder" chooser; Editor rows "Configurations folder" / "Sample locations" (Add folder..., Forget) after "Load the
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
  PORTABLE save starts in `Configurations/Publish/<base>.portable.json` (the toggle's state is known before the
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
  4.1 and the ODF section; site "Using one" and configurations/README.md rewritten (they still said "put the .json
  beside the samples" since 0.8.190). The "same disk" idea: a configuration looks beside ITSELF (configDir forms)
  and in the locations - never over a disk; told the user. 0.8.203 LESSON (found by the Rural test): a pool path
  saved on Windows and read on Linux/macOS ("F:\Suchy_rejstrik\Kobrle\Samply", kept when the file carries an
  absolute folderPath) is ONE component to juce::File there (cwd-relative, backslashes inside) - getParentDirectory/
  getFileName lie on it; everything about old pool paths goes through `pathParts` (split on both separators): the
  prefix of the other missing trees = oldParts less the matched tail, the location = `ancestorOf(found, below + 1)`
  where below = the tree's parts under the common folder of the MISSING trees' parents (the portable save's root
  rule: one tree -> its parent; Rural -> Suchy_rejstrik, Hauptwerk -> HauptwerkSampleSetsAndComponents). Tests
  (~/tools/test_relocate_any.sh <exe> <cfg> <tail> <chosen> '<locations json>' [s]; the CI Linux binary in
  ~/Downloads/OrganSampler): Rural copy in /tmp/rt with Kobrle chosen -> Samply found (14/14 row folders), a
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
  `configurations/FEATURED.txt` = folder names one per line ("#" comments); `configurations_index.py` renders
  "## Featured" + "## All the others" (one table when nothing is featured) in INDEX.md and every sorted copy;
  `configurations_check.sh` skips FEATURED.txt; the workflow `configuration-feature.yml` (workflow_dispatch:
  organ = any part of the name via `find`, action feature/unfeature) edits the file, regenerates INDEX, commits
  to main. Editing FEATURED.txt on GitHub by hand does the same (the index job regenerates on the push). (d) The copy build's parallelFor takes physical
  cores - 1 (the mouse stuttered through a build); (e) one-shot "Chiff" alone -> the new default (as the ignored
  list's "trem" rule); (f) arrangeReportWindows retried at 9 and 15 s (a cold browser came up maximised).
  (g) WASAPI "much quieter than ASIO at the same volume" (the user, 20:14): the render chain is one for every
  device type; the difference lies after our buffer - the Windows volume mixer's per-app session volume (per exe
  NAME: OrganSampler_0.8.184.exe is a new app to the mixer) and the endpoint's level/enhancements; told so.
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
  look there (the installer's ~/.local/share/pixmaps/organsampler.png, 48 px) before blaming JUCE's X11 path. NEVER a
  rescaled icon.
- Device-thread peaks (0.8.138 reports): #34 Ultra 7 155H (P+E cores): longest route callback 2616 us of which WAITING
  FOR THE LANES 2412 us - a worker on an E core (or preempted) holds the poster; try lanes on P cores only / smaller
  chunks at the end of a job. #35 Ryzen Windows: longest 1270 us of which OWN RENDER 1071 - the poster takes chunks
  while the workers are busy; cap the poster's share or batch all routes of a device into one pool job (#30: 21 routes
  x fixed cost). #29: bookkeeping 66 us per route at 44 000 voices (compaction of the active list) - speed up.
- Issue form: the "Operating system" field is a text field filled by the program (now with the version); fine.
- GitHub's issue list hides the number in its new UI (the user asked why "#27" is not in the row): nothing of ours.

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
  binary to `~/Downloads/OrganSampler`.
- Windows builds: never run two MSBuilds of the solution at once - the second fails with C1041 (PDB) and can
  leave orphaned `cl.exe` processes (0 s CPU, parent gone) holding the PDB; stop them before the next build.
- The dual boot clock is settled: Windows and Linux both keep the hardware clock in local time
  (`timedatectl` says "RTC in local TZ: yes"); never set `RealTimeIsUniversal` on Windows. Windows fast startup
  is off (`powercfg /h off`), so the NTFS disks are writable from Linux.

## Compile checks on Linux without a toolchain for the whole program
- `g++ -std=c++17 -fsyntax-only -w -DJUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1 -DJUCE_STANDALONE_APPLICATION=1
  -DJUCE_USE_CURL=0 -DJUCE_WEB_BROWSER=0 -DJUCE_MODULE_AVAILABLE_juce_dsp=1 -DJUCE_LINUX=1 -DNDEBUG
  -I JuceLibraryCode -I ../JUCE/modules -I ../ThirdParty/signalsmith -I /tmp/stubinc Source/<file>.cpp` (run in
  `~/OrganSamplerC-linux/OrganSampler`) checks a source in seconds (SamplerEngine.cpp 4 s, MainComponent.cpp
  longer). `/tmp/stubinc/alsa/asoundlib.h` is a stub of the seven ALSA symbols PlatformAudio.h uses
  (snd_pcm_t, snd_pcm_open/close, snd_strerror, snd_device_name_hint/get_hint/free_hint, SND_PCM_STREAM_PLAYBACK,
  SND_PCM_NONBLOCK; libasound2-dev is not installed); recreate it when /tmp was cleared. Real builds come from CI.
- Never start `~/Downloads/OrganSampler` with a made-up flag such as `--version`: it opens the program (kill it).
  `--selftest "<folder>;<folder>"` is the offline test (folders separated by ';'), with `ORGAN_SELFTEST_IR=<wav>`,
  `ORGAN_IR_LIGHT=1`. `gh` is not installed on Linux: the GitHub API with the token from `git credential fill`.

## Testing
- Windows delivery: after every release build copy the exe to `C:\OrganSampler\OrganSampler_<version>.exe`
  (the local MSBuild Release build or the CI exe) and retarget the EXISTING desktop shortcut in place
  (today `OrganSampler_0.8.98.exe – zástupce.lnk`; find it by its TargetPath under `C:\OrganSampler\`; WScript.Shell
  CreateShortcut, set TargetPath + IconLocation, Save). NEVER delete, rename or recreate the shortcut: Explorer
  keeps icon positions by file name and a new file lands anywhere on the desktop. The user starts the program
  from that shortcut.
- The Windows process is named after the exe (`OrganSampler_0.8.181`): check `Get-Process OrganSampler*`, never
  `Get-Process OrganSampler` (it finds nothing and the 0.8.182 deploy ran while the user played - harmless only
  because a new file name was written and the shortcut retargeted; still, check first).
- Never build or launch the program while the user runs it. Linux test: `~/tools/linux_test.sh <exe> <seconds>`
  with `CFG=`, `ORGAN_OPEN_STOP_LIST=`, `ORGAN_JACK_BUFFER=`, `ORGAN_SAVE_PORTABLE=` hooks; the activity log is
  `~/.config/OrganSampler/CrashLogs/activity.log` (Windows: `%APPDATA%\OrganSampler\CrashLogs\activity.log`).
