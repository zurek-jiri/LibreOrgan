# OrganSampler - rules for every session (Windows and Linux)

The user writes Czech; answer in Czech; append a closing report to `OrganSampler/CLAUDE_REPORTS.md`
(the copy on the NTFS disk: `F:\JUCE_PROJEKTY\OrganSamplerC` = `/mnt/pracovni/JUCE_PROJEKTY/OrganSamplerC`).

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
  above on the Stop List's "Load folder..." (StopListDialog::loadHint, visible while ows.empty() && active(),
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
  0.8.177 THE USER'S RULE ("On nesmí brát archivy, které člověk nevybere!"): ONLY the archives the user chose serve paths -
  Load folder / Load Configuration / Merge (with their parts) and the ARCHIVE lines of ArchiveFolders.txt; never another
  .rar lying beside them, and no longer every archive of the configuration's own folder. 0.8.157's design took every
  first-volume .rar of a folder (rchivesOfFolderLocked), which the Aeolian-Skinner folder's definition-only package
  exploited. Now gChosenArchives (SampleFiles: setChosenArchives/addChosenArchives, rchiveOf(path)),
  rchivesOfFolderLocked(folder) = the chosen archives in it (+ every archive of a folder named as a whole: OrganRarPackages,
  old folder lines), servingArchivesLocked() for the "elsewhere" loop; MainComponent::registerArchive(archive) (was
  registerArchiveFolder(folder)) writes the archive's first part into ArchiveFolders.txt, the Hauptwerk import writes
  SampleFiles::archiveOf(odf); efreshArchiveSearchFolders turns a FOLDER line into the lines of its archives once
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
  website section and the release body only link to INDEX.md. Name configuration files WITHOUT spaces;
  the README's Download line is `https://zurek-jiri.github.io/OrganSampler/configurations/<name>.zip`.
- Stop List counting: every row counts - stops, stop folders, transmissions and extensions (transmissions
  are rows of their receiving division, `transmissions[]` with `manual`) - except the rows nested in a stop
  folder (`groupMaster` set). The table is NEVER typed by hand: it comes from the program, in grid order (row by
  row, left to right - the organ builder's order the user arranged) with the exact button captions (footages,
  "(Ext.)"/"(Trsm.)", a stop folder's "[III]"):
  `ORGAN_DUMP_STOPLIST=<out.md> ORGAN_LOAD_CONFIGURATION=<cfg.json> OrganSampler.exe` writes it once the load is
  over (use a non-portable configuration of this machine, a portable one would ask for the root); quit the program
  afterwards. No legend above the table. Every README looks the same. Stralsund: 53 + 18 = 71.

## Pending for the next session (delete when done)
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
  (a note-off for a key not held releases nothing) and logs the voices 20 s later. The root cause (why the engine
  kept them) is NOT found - the logs of the next #90 will say "20 s after the end N voice(s)". `noteOffAll` is
  clearMidi + killAllVoices (a cut), never for the end of a run. (d) The copy build's parallelFor takes physical
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
  packed 8 - on the website (site/index.html `.ram-measured`) and README since 24 Oct 2026. Keep it when the text
  changes; add a column or row only with a measured figure (never estimated).
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
