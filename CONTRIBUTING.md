# Contributing to LibreOrgan Sampler

## Reporting a problem

In the program: *Configuration Editor > Report a problem...* Describe what
happened and how to reproduce it. The program bundles its activity log, the
crash reports, `settings.json`, the last self-test report and a description of
the computer and the audio devices into one zip (no samples, no voicing, no
combinations), takes screenshots of its own windows - keep the dialog in
question open - and lets you add screenshot files of your own. It then opens
the [bug report form](https://github.com/zurek-jiri/LibreOrgan/issues/new/choose)
in your browser with the fields filled in, and the report folder on your
Desktop: drag the zip into "Logs", the pictures into "Screenshots", submit. A
free GitHub account is needed to submit; if GitHub asks you to sign in and
then lands elsewhere, press *Open the GitHub form* in the message that stays
open in the program (or open `Report on GitHub.html` in the report folder).

The activity log is what makes a report fixable: it records every device,
every load and the last minutes before a crash. It lives in the program's
application-data folder, `CrashLogs/activity.log`, beside the crash reports
`CrashLogs/crash-<date>.txt`: Windows `%APPDATA%\LibreOrgan`, macOS
`~/Library/LibreOrgan` (plus the system's own report in
`~/Library/Logs/DiagnosticReports`), Linux `~/.config/LibreOrgan`.

The maintainer reads the reports with `LibreOrgan/Tools/fetch_issues.ps1`,
which downloads every issue with its comments and attachments into a local
folder for analysis.

## Sharing an organ configuration

Save it with *Portable configuration (to share with others)* ticked:
LibreOrgan Sampler writes the zip and the README beside it. Fill in the three
"(fill in: ...)" places and attach the two files to the
[Organ configuration form](https://github.com/zurek-jiri/LibreOrgan/issues/new?template=organ_configuration.yml);
a robot checks them and opens the pull request for you - no Git needed. The details:
[libreorgans/README.md](libreorgans/README.md); the published organs:
[libreorgans/INDEX.md](libreorgans/INDEX.md). Configurations only -
never samples, never impulse responses; the `Configurations check` workflow
refuses them.

## Changing the code

- Build with CMake ([README.md](README.md)) or the Projucer project under
  `LibreOrgan/`; both compile the same sources.
- C++17, JUCE 9. Windows-only code (ASIO, WASAPI aggregate, the crash
  handler's dbghelp, window enumeration, message pumps) stays under
  `#if JUCE_WINDOWS` / `JUCE_MSVC` with a fallback, so macOS and Linux keep
  building. GCC and clang are stricter than MSVC: `alignas` before `static`,
  explicit `juce::int64`/`juce::uint64` for `jmax` and `countNumberOfBits`,
  `std::max<size_t>` instead of `jmax<size_t>`, `juce::File()` instead of `{}`.
- Nothing may refer to a path outside the repository; nothing is read from
  beside the executable at run time (data tables and licence texts are
  embedded at build time).
- The audio is never altered silently: no hidden normalisation, trimming or
  fading of samples or impulse responses.
- Every user-visible change gets a line in [`LibreOrgan/CHANGELOG.md`](LibreOrgan/CHANGELOG.md) and,
  where the user sees it, a paragraph in [`LibreOrgan/USER_MANUAL.md`](LibreOrgan/USER_MANUAL.md) (the
  manual is embedded in the program; `Tools/number_manual.ps1` renumbers its
  chapters after an edit).
- Before a pull request: the CI build on all three platforms passes and
  `LibreOrgan Sampler --selftest "<folder>"` completes without crash.

## Licence

LibreOrgan Sampler is free software under the GNU GPL v3; contributions are
accepted under the same licence. Third-party components and their licences are
listed in [THIRD-PARTY-NOTICES.md](THIRD-PARTY-NOTICES.md).
