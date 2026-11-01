#!/usr/bin/env python3
"""Writes libreorgans/INDEX.md: one line per published organ configuration -
the link to its README (which says how to download it), the divisions, the
rows of the Stop List and the perspectives, read from the configuration file
itself. Run by the Configurations check workflow on every push to main and
committed back when it changed; it can be run by hand from the repository root.

The Stop List rows are counted as the program counts them: every stop of the
voicing state that is not nested in a stop folder, plus every transmission or
extension that is not nested in one.
"""
import json
import os
import re
import sys
import zipfile
from urllib.parse import quote

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'libreorgans')


def main():
    rows = []
    for folder in sorted(os.listdir(ROOT)):
        path = os.path.join(ROOT, folder)
        if not os.path.isdir(path):
            continue
        # The configuration is the .json inside the folder's zip (a bare .json is accepted too).
        cfg = None
        for name in sorted(os.listdir(path)):
            full = os.path.join(path, name)
            if name.lower().endswith('.zip'):
                with zipfile.ZipFile(full) as z:
                    inner = [n for n in z.namelist() if n.lower().endswith('.json')]
                    if inner:
                        cfg = json.loads(z.read(inner[0]).decode('utf-8'))
                        break
            elif name.lower().endswith('.json'):
                with open(full, encoding='utf-8') as f:
                    cfg = json.load(f)
                break
        if cfg is None:
            continue
        divisions = int(cfg.get('manualCount', 0)) + 1
        manuals = cfg.get('manualKeyboards', [])[:divisions]
        # The rows nested in a stop folder: the table's child rows (the voicing
        # state marks them by groupMaster only when it was saved by a newer program).
        nested = set()
        for manual, m in enumerate(manuals):
            for e in m.get('stopBuild', {}).get('entries', []) or []:
                if e.get('child'):
                    nested.add((manual, e.get('name', '')))
        stops = cfg.get('voicingSettings', {}).get('stops', [])
        count = 0
        for s in stops:
            manual = int(s.get('primaryManual', 0))
            if manual >= divisions or s.get('groupMaster') or (manual, s.get('name', '')) in nested:
                continue
            count += 1
        for s in stops:
            for t in s.get('transmissions') or []:
                if not t.get('groupMaster') and int(t.get('manual', 0)) < divisions:
                    count += 1
        # The perspectives, in the order of the perspective patterns (the user's own).
        order = [str(p).split(';')[0].strip() for p in cfg.get('perspectivePatterns', []) or []]
        perspectives = []
        for m in manuals:
            for p in m.get('stopBuild', {}).get('perspectives', []) or []:
                if p and p not in perspectives:
                    perspectives.append(p)
        perspectives.sort(key=lambda p: next((i for i, o in enumerate(order) if o.lower() == p.lower()), len(order)))
        if not perspectives:
            perspectives = ['default']
        # The auxiliary route counts as well when a division uses it (a device saved on it).
        for m in manuals:
            for route in m.get('perspectiveAudio', []) or []:
                if str(route.get('name', '')).lower() != 'auxiliary':
                    continue
                xml = str(route.get('audioDeviceStateXml', ''))
                used = 'deviceType=' in xml or ('audioOutputDeviceName="' in xml and 'audioOutputDeviceName=""' not in xml)
                if used and 'auxiliary' not in perspectives:
                    perspectives.append('auxiliary')
        # The RAM the program used with the organ loaded, measured by the program
        # when the portable configuration was saved (0.8.118; older files: unknown).
        ram_bytes = int(cfg.get('measuredRamBytes', 0) or 0)
        # The least it can take (0.8.151): every saving option of the load box (16-bit,
        # 32 kHz on loading, releases at half the rate, packed), estimated by the
        # program from the loaded samples at the save.
        ram_min = int(cfg.get('estimatedMinimumRamBytes', 0) or 0)
        if ram_min > ram_bytes > 0:
            ram_min = ram_bytes   # loaded with the saving options already: the measured figure is the least
        title = folder
        readme = os.path.join(path, 'README.md')
        if os.path.exists(readme):
            with open(readme, encoding='utf-8') as f:
                for line in f:
                    if line.startswith('# '):
                        title = line[2:].strip()
                        # The user's rule (1 Nov 2026): a trailing word "organ"/"Organ" is dropped from the Organ cell
                        # ("Adlington Hall Organ" -> "Adlington Hall"): nearly every row had it and it clogged the
                        # table; "Organ" INSIDE a name ("Hill English Organ Model") stays.
                        title = re.sub(r'\s+[Oo]rgan$', '', title)
                        break
        rows.append((title, folder, divisions, count, ', '.join(perspectives), ram_bytes, ram_min))

    def ram_text(ram_bytes):
        if ram_bytes <= 0:
            return '-'
        gb = ram_bytes / float(1 << 30)
        return ('%.1f GB' % gb) if gb < 10 else ('%d GB' % round(gb))

    # A Markdown table cannot be sorted by a click, so the header carries the
    # triangles as links: each opens the same table sorted by that column -
    # INDEX.md by organ ascending, INDEX-<column>-<asc|desc>.md the others.
    # One RAM column (24 Oct 2026, the user: two columns of the same 7 GB were absurd): the
    # least the organ takes - every saving option of the load box - and the full
    # quality is roughly ten times that, said in the legend.
    columns = [('organ', 'Organ', lambda r: r[0].lower()), ('divisions', 'Divisions', lambda r: r[2]),
               ('stops', 'Stops', lambda r: r[3]), ('ram', 'RAM', lambda r: r[6] if r[6] > 0 else r[5]),
               ('routes', 'Routes', lambda r: r[4].lower())]
    def file_for(key, descending):
        return 'INDEX.md' if (key == 'organ' and not descending) else 'INDEX-%s-%s.md' % (key, 'desc' if descending else 'asc')
    # Two tables (25 Oct 2026, the user): "Featured" - the folders listed in
    # libreorgans/FEATURED.txt (one folder name per line, as in the README links;
    # "#" lines are comments; the Rural Organ demo by default) - and all the others.
    # The maintainer edits FEATURED.txt on GitHub, or runs the "Feature a
    # configuration" workflow with any part of the organ's name.
    featured = []
    featured_file = os.path.join(ROOT, 'FEATURED.txt')
    if os.path.exists(featured_file):
        with open(featured_file, encoding='utf-8') as f:
            for line in f:
                name = line.strip()
                if name and not name.startswith('#'):
                    featured.append(name.lower())
    def render(sort_key, descending):
        ordered = sorted(rows, key=lambda r: r[0].lower())
        ordered.sort(key=dict((k, f) for k, _, f in columns)[sort_key], reverse=descending)
        header = []
        for key, label, _ in columns:
            up = '▲' if (key == sort_key and not descending) else '[▲](%s)' % file_for(key, False)
            down = '▼' if (key == sort_key and descending) else '[▼](%s)' % file_for(key, True)
            header.append('%s %s%s' % (label, up, down))
        lines = [
            '# Published configurations',
            '',
            'One line per organ - the README of each (with the download of its `.json` and where to',
            'put it), the number of divisions, the rows of its Stop List (transmissions and extensions',
            'included, the rows nested in stop folders not), the RAM the organ takes at the least -',
            'with every saving option of the load box (16-bit, 32 kHz on loading, releases at half the',
            'rate, packed samples), measured or estimated by the program when the configuration was',
            'saved; at the full quality (32-bit float, the files\' own rate, full releases, no packing)',
            'count on roughly ten times as much; `-` = saved by an older version - and the',
            'routes - the perspectives as the configuration names them (`default` = a single',
            'perspective) and `auxiliary` when a division uses that route; what each one carries, the',
            'README of the organ says. The triangles in the header sort the tables by that column.',
            'The last column takes a configuration off the list: its *remove* link opens the',
            '"Remove my configuration" form with the organ filled in - submit it, and a robot removes',
            'the folder when you are the one who published it (or the maintainer).',
            'Written by the `Configurations check` workflow from the configuration files themselves',
            'on every change; do not edit by hand. How to use and publish one: [README.md](README.md).',
            '',
        ]
        def remove_link(folder):
            # The issue form prefilled through the query string: the title, the dropdown "organ" (GitHub does not
            # honour a dropdown prefill reliably - community discussion #5288) and, so the organ IS chosen whatever
            # the dropdown does, the text field "name" with the exact folder (1 Nov 2026, the user: the remove link
            # must pre-select the organ); the robot takes the exact folder match from that field.
            return ('[remove](https://github.com/zurek-jiri/LibreOrgan/issues/new?template=organ_configuration_remove.yml'
                    '&title=%s&organ=%s&name=%s)' % (quote('[Remove configuration] ' + folder), quote(folder), quote(folder)))
        def table(entries):
            out = ['| ' + ' | '.join(header + ['Remove']) + ' |', '|---|---:|---:|---:|---|---|']
            for title, folder, divisions, count, perspectives, ram_bytes, ram_min in entries:
                out.append('| [%s](%s/README.md) | %d | %d | %s | %s | %s |' % (title.replace('|', '\\|'), folder, divisions, count, ram_text(ram_min if ram_min > 0 else ram_bytes), perspectives,
                                                                               '' if folder == 'rural-organ' else remove_link(folder)))
            return out
        top = [r for r in ordered if r[1].lower() in featured]
        rest = [r for r in ordered if r[1].lower() not in featured]
        if top:
            lines.append('## Featured')
            lines.append('')
            lines += table(top)
            lines.append('')
            lines.append('## All the others')
            lines.append('')
        if rest or not top:
            lines += table(rest)
            lines.append('')
        lines.append('%d configuration(s).' % len(ordered))
        lines.append('')
        return '\n'.join(lines)
    changed = 0
    for key, _, _ in columns:
        for descending in (False, True):
            name = file_for(key, descending)
            text = render(key, descending)
            out = os.path.join(ROOT, name)
            old = open(out, encoding='utf-8').read() if os.path.exists(out) else None
            if old != text:
                with open(out, 'w', encoding='utf-8', newline='\n') as f:
                    f.write(text)
                changed += 1
    # The "Remove my configuration" issue form (27 Oct 2026, the user: "kdyby to prostě vyhodilo
    # seznam konfigurací a já bych jen zaškrtl"): a dropdown of every published folder, several
    # at once, written here from the same rows - an issue TEMPLATE may be pushed by the robot
    # (only .github/workflows may not). A field to type a part of the name stays beside it.
    form = os.path.join(ROOT, '..', '.github', 'ISSUE_TEMPLATE', 'organ_configuration_remove.yml')
    folders = sorted(r[1] for r in rows if r[1] != 'rural-organ')
    options = '\n'.join('        - "%s"' % f.replace('"', '') for f in folders) or '        - "(nothing published yet)"'
    form_text = '''name: Remove my configuration
description: Take a configuration you published off the list - tick the organ, a robot removes it.
title: "[Remove configuration] "
labels: ["configuration-remove"]
body:
  - type: markdown
    attributes:
      value: |
        Tick the organ (or several) in the list and press *Submit new issue*. A robot removes the folder when you are
        the one who published it (or the maintainer), answers here and closes this issue. The list is written by the
        robot from [libreorgans/INDEX.md](https://github.com/zurek-jiri/LibreOrgan/blob/main/libreorgans/INDEX.md);
        the *remove* link of an organ's line there opens this form with that organ's folder filled into the name
        field below (GitHub does not pre-tick the list) - just press *Submit new issue*; tick organs here to remove
        several at once. To publish a new version instead, send the new zip with the "Organ configuration" form:
        it replaces the old one by itself.
  - type: dropdown
    id: organ
    attributes:
      label: Organ
      description: The configuration folder(s) to remove, as listed in INDEX.md.
      multiple: true
      options:
%s
    validations:
      required: false
  - type: input
    id: name
    attributes:
      label: Or any part of its name
      description: When the list above does not fit - any part of the organ's name (case does not matter).
      placeholder: "Stralsund"
    validations:
      required: false
''' % options
    old = open(form, encoding='utf-8').read() if os.path.exists(form) else None
    if old != form_text:
        with open(form, 'w', encoding='utf-8', newline='\n') as f:
            f.write(form_text)
        changed += 1
        print('%s written: %d organ(s) in the list' % (os.path.relpath(form, os.path.join(ROOT, '..')), len(folders)))
    print('%s: %d configuration(s), %d file(s) written' % ('INDEX.md' if changed == 0 else 'INDEX', len(rows), changed))
    return 1 if changed else 0


if __name__ == '__main__':
    sys.exit(main())
