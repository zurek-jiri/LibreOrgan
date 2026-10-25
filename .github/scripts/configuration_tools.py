#!/usr/bin/env python3
"""The published configurations: what identifies an organ, what a valid upload
is, and how to find one. Used by the workflows (Configuration from issue,
Configurations check, Remove a configuration) and by configurations_check.sh.

  configuration_tools.py validate <zip> <readme>   checks the pair; prints the problems, exit 1 on any
  configuration_tools.py slug <zip>                 the folder name the organ gets under configurations/
  configuration_tools.py same-organ <zip>           existing folders holding the same organ (one per line)
  configuration_tools.py find <text>                folders matching <text> (folder|title|organ|publisher per line)
  configuration_tools.py publisher <folder>         the GitHub login that published the folder ("" when unknown)

An organ is identified by the name given at the save ("publication" -> "organ"
in the configuration); its folder is that name lower-cased with runs of other
characters turned into "-" (the program writes the README's download link with
the same rule). A configuration saved without a name is identified by its zip's
name. Run from the repository root.
"""
import json
import os
import re
import sys
import zipfile

ROOT = 'configurations'
PROTECTED = {'rural-organ'}   # the demo organ with its samples: never replaced or removed by the robots


def slugify(text):
    return re.sub(r'[^a-z0-9]+', '-', str(text).lower()).strip('-')


def read_config(zip_path):
    """The configuration inside the zip (exactly one .json), or (None, message)."""
    try:
        z = zipfile.ZipFile(zip_path)
    except (zipfile.BadZipFile, OSError):
        return None, 'the zip cannot be read'
    files = [n for n in z.namelist() if not n.endswith('/')]
    if len(files) != 1 or not files[0].lower().endswith('.json'):
        return None, 'the zip must hold exactly one configuration .json and nothing else (found: %s)' % ', '.join(files)
    try:
        cfg = json.loads(z.read(files[0]).decode('utf-8'))
    except (UnicodeDecodeError, ValueError) as e:
        return None, 'the .json inside the zip is not valid JSON (%s)' % e
    if not isinstance(cfg, dict):
        return None, 'the .json inside the zip is not an object'
    return cfg, files[0]


def title_from_base(base):
    """The file's name less "portable" and the separators around it (the program's rule)."""
    out = base
    while True:
        m = re.search(r'portable', out, flags=re.I)
        if not m:
            break
        start, end = m.start(), m.end()
        while start > 0 and out[start - 1] in '._- ':
            start -= 1
        while end < len(out) and out[end] in '._- ':
            end += 1
        out = out[:start] + (' ' if start > 0 and end < len(out) else '') + out[end:]
    out = out.strip()
    return out or base


def organ_of(cfg):
    publication = cfg.get('publication') if isinstance(cfg, dict) else None
    if isinstance(publication, dict):
        return str(publication.get('organ', '') or '').strip()
    return ''


def slug_for(zip_path, cfg):
    organ = organ_of(cfg) if cfg else ''
    base = os.path.basename(zip_path)[:-4]
    return slugify(organ) or slugify(base) or 'organ'


def config_of_folder(folder):
    path = os.path.join(ROOT, folder)
    for name in sorted(os.listdir(path)):
        if name.lower().endswith('.zip'):
            cfg, _ = read_config(os.path.join(path, name))
            if cfg is not None:
                return cfg, name
    return None, None


def title_of_folder(folder):
    readme = os.path.join(ROOT, folder, 'README.md')
    if os.path.exists(readme):
        with open(readme, encoding='utf-8', errors='replace') as f:
            for line in f:
                if line.startswith('# '):
                    return line[2:].strip()
    return folder


def publisher_of_folder(folder):
    readme = os.path.join(ROOT, folder, 'README.md')
    if os.path.exists(readme):
        with open(readme, encoding='utf-8', errors='replace') as f:
            m = re.search(r'<!--\s*published by @([A-Za-z0-9-]+)', f.read())
            if m:
                return m.group(1)
    return ''


def folders():
    if not os.path.isdir(ROOT):
        return []
    return sorted(d for d in os.listdir(ROOT) if os.path.isdir(os.path.join(ROOT, d)))


def same_organ(zip_path, cfg):
    """Existing folders that hold the same organ: the same organ name (by slug), or
    the folder named like this zip's slug (an organ published without a name)."""
    wanted = slug_for(zip_path, cfg)
    organ = slugify(organ_of(cfg)) if cfg else ''
    found = []
    for folder in folders():
        if folder in PROTECTED:
            continue
        if folder == wanted:
            found.append(folder)
            continue
        if organ:
            other, _ = config_of_folder(folder)
            if other is not None and slugify(organ_of(other)) == organ:
                found.append(folder)
    return found


def find(text):
    """Folders matching the text: the folder name, the README title or the organ's
    name, whole or as a part, case-insensitively (spaces and punctuation ignored)."""
    key = slugify(text)
    if not key:
        return []
    found = []
    for folder in folders():
        cfg, _ = config_of_folder(folder)
        organ = organ_of(cfg) if cfg else ''
        title = title_of_folder(folder)
        haystacks = [folder, slugify(title), slugify(organ)]
        if any(key == h or key in h for h in haystacks if h):
            found.append((folder, title, organ, publisher_of_folder(folder)))
    return found


def count_rows(cfg):
    """The Stop List rows as the program counts them (configurations_index.py)."""
    divisions = int(cfg.get('manualCount', 0) or 0) + 1
    manuals = (cfg.get('manualKeyboards') or [])[:divisions]
    nested = set()
    for manual, m in enumerate(manuals):
        for e in (m.get('stopBuild') or {}).get('entries', []) or []:
            if e.get('child'):
                nested.add((manual, e.get('name', '')))
    stops = (cfg.get('voicingSettings') or {}).get('stops', []) or []
    count = 0
    for s in stops:
        manual = int(s.get('primaryManual', 0) or 0)
        if manual >= divisions or s.get('groupMaster') or (manual, s.get('name', '')) in nested:
            continue
        count += 1
    for s in stops:
        for t in s.get('transmissions') or []:
            if not t.get('groupMaster') and int(t.get('manual', 0) or 0) < divisions:
                count += 1
    return count


def validate_config(cfg):
    """Is this an organ configuration of OrganSampler, saved portable with the organ loaded?"""
    problems = []
    if 'portableConfigurationVersion' not in cfg:
        problems.append('not an OrganSampler configuration (no portableConfigurationVersion)')
    manuals = cfg.get('manualKeyboards')
    try:
        divisions = int(cfg.get('manualCount', -1)) + 1
    except (TypeError, ValueError):
        divisions = 0
    if not isinstance(manuals, list) or divisions < 1 or divisions > 8 or len(manuals) < divisions:
        problems.append('no divisions (manualKeyboards / manualCount)')
        manuals, divisions = [], 0
    if cfg.get('containsMidiMappings'):
        problems.append('not a portable configuration (it carries MIDI mappings): save it with "Portable configuration" ticked')
    trees = [str(t) for t in (cfg.get('sampleTrees') or []) if str(t).strip() not in ('', '.')]
    for m in manuals[:divisions]:
        if not isinstance(m, dict):
            continue
        for key in ('folderRelativePath', 'folderPath'):
            if str(m.get(key, '') or '').strip():
                trees.append(str(m.get(key)))
        trees += [str(t) for t in (m.get('extraFolderRelativePaths') or []) + (m.get('extraFolderPaths') or [])]
    if not trees:
        problems.append('no sample folder tree is named (nothing would be loaded)')
    stops = (cfg.get('voicingSettings') or {}).get('stops') if isinstance(cfg.get('voicingSettings'), dict) else None
    if not isinstance(stops, list) or not any(isinstance(s, dict) and str(s.get('name', '')).strip() for s in stops):
        problems.append('no stops in the voicing state (the organ was not loaded when it was saved, or it is not an organ)')
    elif count_rows(cfg) < 1:
        problems.append('the Stop List has no rows')
    if int(cfg.get('measuredRamBytes', 0) or 0) <= 0:
        problems.append('no RAM figure: save the configuration with the organ loaded')
    return problems


def validate_readme(readme_path, zip_path, cfg):
    """Does the README belong to this zip, and was it written by the program for it?"""
    problems = []
    base = os.path.basename(zip_path)[:-4]
    try:
        with open(readme_path, encoding='utf-8') as f:
            text = f.read()
    except (OSError, UnicodeDecodeError) as e:
        return ['the README cannot be read as UTF-8 text (%s)' % e]
    first = next((line.strip() for line in text.splitlines() if line.strip()), '')
    # The title (0.8.189): the organ as named at the save, else the file's name less "portable".
    organ_name = organ_of(cfg) if cfg is not None else ''
    expected = organ_name or title_from_base(base)
    if first != '# ' + expected:
        problems.append('the README is not the one written for %s.zip (its title is "%s", expected "# %s" - save it again with OrganSampler 0.8.189 or later)' % (base, first, expected))
    if 'Written by OrganSampler' not in text:
        problems.append('the README was not written by OrganSampler (save the configuration with "Portable configuration" ticked and send the README it writes)')
    left = re.findall(r'\(fill in:[^)]*\)', text)
    if left:
        problems.append('the README still has %d "(fill in: ...)" place(s) to complete: %s' % (len(left), '; '.join(left)))
    if '## Stop List' not in text:
        problems.append('the README has no Stop List table')
    else:
        table = text.split('## Stop List', 1)[1]
        rows = [line for line in table.splitlines() if line.strip().startswith('|')]
        data = rows[2:] if len(rows) >= 2 else []
        cells = sum(1 for row in data for cell in row.strip().strip('|').split('|') if cell.strip())
        if cells < 1:
            problems.append('the Stop List table of the README is empty')
        m = re.search(r'and (\d+) rows in the Stop List', text)
        if m and int(m.group(1)) != cells:
            problems.append('the README says %s rows in the Stop List but its table has %d' % (m.group(1), cells))
        if cfg is not None and cells and abs(count_rows(cfg) - cells) > 0:
            print('note: the README table has %d rows, the configuration counts %d' % (cells, count_rows(cfg)))
    if cfg is not None:
        divisions = int(cfg.get('manualCount', 0) or 0) + 1
        m = re.search(r': (\d+) divisions and', text)
        if m and int(m.group(1)) != divisions:
            problems.append('the README says %s divisions, the configuration has %d' % (m.group(1), divisions))
        for tree in (cfg.get('sampleTrees') or []):
            t = str(tree).replace('\\', '/').strip()
            if t in ('', '.'):
                continue
            if '`%s`' % t not in text:
                problems.append('the README does not name the tree %s the configuration expects' % t)
        organ = organ_of(cfg)
        if organ and ('**Organ:** ' + organ) not in text:
            problems.append('the README names another organ than the configuration ("%s")' % organ)
        # The download link must point into the folder the organ gets (the program
        # writes it with the same rule since 0.8.188; an older README points elsewhere).
        m = re.search(r'raw/main/configurations/([^/\s]+)/([^\s)]+)\)', text)
        wanted = slug_for(zip_path, cfg)
        if m and m.group(1) != wanted:
            problems.append('the README\'s download link points to the folder "%s", the organ\'s folder is "%s" - save the configuration again with OrganSampler 0.8.188 or later'
                            % (m.group(1), wanted))
        # The link must name THIS zip (the attachment's dots for spaces gave a 404 before).
        if m and m.group(2).replace('%20', ' ') != os.path.basename(zip_path):
            problems.append('the README\'s download link names "%s" but the zip is "%s"' % (m.group(2).replace('%20', ' '), os.path.basename(zip_path)))
    if 'Saved with:** OrganSampler' not in text:
        problems.append('the README has no "Saved with: OrganSampler" line')
    if '(the organ was not named)' in text or '(the producer was not named)' in text:
        problems.append('the organ or the producer of its samples was not named at the save - fill the Save Configuration box in and save again')
    return problems


def validate(zip_path, readme_path, readme_text_checks=True):
    cfg, what = read_config(zip_path)
    if cfg is None:
        return [what]
    problems = validate_config(cfg)
    if readme_text_checks:
        problems += validate_readme(readme_path, zip_path, cfg)
    elif not os.path.exists(readme_path):
        problems.append('no README.md')
    return problems


def main(argv):
    if len(argv) < 2:
        print(__doc__)
        return 2
    command = argv[1]
    if command == 'validate' and len(argv) >= 4:
        problems = validate(argv[2], argv[3], readme_text_checks=(len(argv) < 5 or argv[4] != 'folder'))
        for p in problems:
            print('::error file=%s::%s' % (argv[2], p))
        return 1 if problems else 0
    if command == 'slug' and len(argv) >= 3:
        cfg, _ = read_config(argv[2])
        print(slug_for(argv[2], cfg))
        return 0
    if command == 'same-organ' and len(argv) >= 3:
        cfg, _ = read_config(argv[2])
        for folder in same_organ(argv[2], cfg):
            print(folder)
        return 0
    if command == 'find' and len(argv) >= 3:
        for folder, title, organ, publisher in find(' '.join(argv[2:])):
            print('%s|%s|%s|%s' % (folder, title, organ, publisher))
        return 0
    if command == 'publisher' and len(argv) >= 3:
        print(publisher_of_folder(argv[2]))
        return 0
    print(__doc__)
    return 2


if __name__ == '__main__':
    sys.exit(main(sys.argv))
