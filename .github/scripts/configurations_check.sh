#!/usr/bin/env bash
# Checks the libreorgans folder: per organ one folder with the zipped
# configuration (exactly one .json inside) and its README.md, or - the form an
# upload arrives in - "<Name>.zip" with "<Name>.README.md" beside it flat in
# libreorgans/. Anything else fails: audio files (samples, impulse
# responses), files named like pipe samples (036-C.wav), other file types,
# anything above 40 MB. Run from the repository root; exit 1 on any error.
set -e
fail=0
# The content (0.8.188): the zip holds one .json that IS an organ configuration of
# LibreOrgan Sampler, saved portable with the organ loaded; an upload pair's README is
# the one the program wrote for that zip (configuration_tools.py validate). An
# organ's folder is checked for its zip's content only (its README may be older).
zipcheck() {
  python3 .github/scripts/configuration_tools.py validate "$1" "$2" "${3:-pair}"
}
while IFS= read -r -d '' f; do
  name="$(basename "$f")"
  lower="$(echo "$name" | tr '[:upper:]' '[:lower:]')"
  size=$(stat -c %s "$f")
  case "$f" in
    libreorgans/README.md|libreorgans/INDEX*.md|libreorgans/FEATURED.txt) continue ;;
    libreorgans/*/*)
      # An organ's folder: README.md and the zip.
      case "$lower" in
        readme.md) ;;
        *.zip) zipcheck "$f" "$(dirname "$f")/README.md" folder || fail=1 ;;
        *.json) echo "::error file=$f::a bare .json opens as text on GitHub - send the zip LibreOrgan Sampler writes beside a portable configuration"; fail=1 ;;
        *) echo "::error file=$f::only the zipped configuration and its README.md belong in an organ's folder ($name) - no samples, no impulse responses (they are part of the sample folders), nothing else"; fail=1 ;;
      esac ;;
    libreorgans/*)
      # An upload: "<Name>.zip" with "<Name>.README.md" beside it.
      case "$lower" in
        *.readme.md)
          base="${f%.README.md}"
          [ -f "$base.zip" ] || { echo "::error file=$f::no $(basename "$base").zip beside this README - both files LibreOrgan Sampler wrote are needed"; fail=1; } ;;
        *.zip)
          base="${f%.zip}"
          if [ -f "$base.README.md" ]; then zipcheck "$f" "$base.README.md" pair || fail=1
          else echo "::error file=$f::no $(basename "$base").README.md beside this zip - both files LibreOrgan Sampler wrote are needed"; fail=1; fi ;;
        *.json) echo "::error file=$f::a bare .json opens as text on GitHub - send the zip LibreOrgan Sampler writes beside a portable configuration"; fail=1 ;;
        *) echo "::error file=$f::only <Name>.zip and <Name>.README.md belong here ($name)"; fail=1 ;;
      esac ;;
  esac
  if echo "$name" | grep -Eq '^[0-9]{3}[-_.]'; then
    echo "::error file=$f::named like a pipe sample ($name) - samples are never published here"; fail=1
  fi
  if [ "$size" -gt 41943040 ]; then
    echo "::error file=$f::larger than 40 MB"; fail=1
  fi
done < <(find libreorgans -type f -print0)
if [ "$fail" -ne 0 ]; then
  echo "The libreorgans folder takes, per organ, the zipped configuration (one .json inside) and its README.md - nothing else. Samples and impulse responses are bought or downloaded elsewhere (Sonus Paradisi)."
  exit 1
fi
echo "libreorgans/ is clean"
