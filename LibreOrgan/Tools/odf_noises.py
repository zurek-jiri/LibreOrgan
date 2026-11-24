# How Hauptwerk definitions wire their noises: every pipe grouped by the folder of its first attack sample (two
# levels under the package), with what makes it sound - a stop of a division played by keys (K), or its own pallet
# switch (P) traced back through the switch linkages to a key of a keyboard, a stop's drawknob, a tremulant, a coupler.
# Usage: python -I odf_noises.py <odf> [all]                 one definition (with "all": the pipe tone groups too)
#        python -I odf_noises.py --batch <folder> <out.txt>  every *.Organ_Hauptwerk_xml of a folder, equal results merged
import os, re, sys, collections

FOLDER_WORDS = ('ruch', 'noise', 'klap', 'tahl', 'tahel', 'mech', 'blow', 'calc', 'kalk', 'hluk', 'motor', 'zvon', 'cimb', 'zimb',
                'rossig', 'vogel', 'action', 'tracker', 'click', 'spoj', 'coupl', 'paky', 'ventil', 'glock', 'tromm', 'tamb', 'pauk',
                'orage', 'bird', 'pajar', 'paxar', 'hvezd', 'schel', 'mokry', 'suchy', 'drum', 'thunder', 'nacht', 'cuck', 'kuku', 'effect')
RANK_WORDS = ('noise', 'special', 'blower', 'tracker', 'action', 'effect', 'drawstop', 'ruch', 'klap', 'tahl', 'calc', 'kalk')

def table(data, name):
    m = re.search(r'<ObjectList ObjectType="%s">(.*?)</ObjectList>' % re.escape(name), data, re.S | re.I)
    if not m:
        return []
    body = m.group(1)
    rows = re.findall(r'<o>(.*?)</o>', body, re.S)
    if not rows:
        rows = re.findall(r'<%s>(.*?)</%s>' % (re.escape(name), re.escape(name)), body, re.S | re.I)
    return [dict(re.findall(r'<([A-Za-z0-9_]+)>([^<]*)</', r)) for r in rows]

def g(r, name, letter, default=''):
    v = r.get(name)
    if v is None or v == '':
        v = r.get(letter)
    return default if v is None or v == '' else v

def gi(r, name, letter):
    try:
        return int(float(g(r, name, letter, '0')))
    except ValueError:
        return 0

def shape(s):
    s = re.sub(r'\d{3}-[A-Ha-h][#b]?(?![a-z])', 'KEY', s)
    return re.sub(r'\d+', 'N', s)

def collect(path):
    data = open(path, 'r', encoding='utf-8', errors='replace').read()
    if '<ObjectList' not in data:
        return '', None
    ident = re.search(r'<Identification_UniqueOrganID>(\d+)<', data)
    name = re.search(r'<Identification_Name>([^<]*)<', data)
    title = 'organ %s "%s"' % (ident.group(1) if ident else '?', name.group(1) if name else '?')
    samples = {gi(r, 'SampleID', 'a'): (gi(r, 'InstallationPackageID', 'b'), g(r, 'SampleFilename', 'c').replace('\\', '/')) for r in table(data, 'Sample')}
    pipes = {gi(r, 'PipeID', 'a'): (gi(r, 'RankID', 'b'), gi(r, 'ControllingPalletSwitchID', 'c'), gi(r, 'NormalMIDINoteNumber', 'd')) for r in table(data, 'Pipe_SoundEngine01')}
    layer_pipe = {gi(r, 'LayerID', 'a'): gi(r, 'PipeID', 'b') for r in table(data, 'Pipe_SoundEngine01_Layer')}
    attacks = collections.defaultdict(list)
    for r in table(data, 'Pipe_SoundEngine01_AttackSample'):
        attacks[layer_pipe.get(gi(r, 'LayerID', 'b'), 0)].append(gi(r, 'SampleID', 'c'))
    releases = collections.defaultdict(list)
    for r in table(data, 'Pipe_SoundEngine01_ReleaseSample'):
        releases[layer_pipe.get(gi(r, 'LayerID', 'b'), 0)].append(gi(r, 'SampleID', 'c'))
    ranks = {gi(r, 'RankID', 'a'): g(r, 'Name', 'b') for r in table(data, 'Rank')}
    stops = {gi(r, 'StopID', 'a'): (g(r, 'Name', 'b'), gi(r, 'DivisionID', 'c'), gi(r, 'ControllingSwitchID', 'd')) for r in table(data, 'Stop')}
    divisions = {gi(r, 'DivisionID', 'a'): g(r, 'Name', 'b') for r in table(data, 'Division')}
    rank_stops = collections.defaultdict(list)
    for r in table(data, 'StopRank'):
        rank_stops[gi(r, 'RankID', 'd')].append(gi(r, 'StopID', 'a'))
        if gi(r, 'AlternateRankID', 'p'):
            rank_stops[gi(r, 'AlternateRankID', 'p')].append(gi(r, 'StopID', 'a'))
    switches = {gi(r, 'SwitchID', 'a'): (g(r, 'Name', 'b'), g(r, 'DefaultToEngaged', 'e', 'N'), g(r, 'Clickable', 'i', 'N'), gi(r, 'Disp_ImageSetInstanceID', 'k')) for r in table(data, 'Switch')}
    keyboards = {gi(r, 'KeyboardID', 'a'): g(r, 'Name', 'b') for r in table(data, 'Keyboard')}
    role = {}
    for r in table(data, 'KeyboardKey'):
        role.setdefault(gi(r, 'SwitchID', 'b'), 'a KEY of keyboard "%s"' % keyboards.get(gi(r, 'KeyboardID', 'a'), '?'))
    for r in table(data, 'DivisionInput'):
        role.setdefault(gi(r, 'SwitchID', 'b'), 'an INPUT of division "%s"' % divisions.get(gi(r, 'DivisionID', 'a'), '?'))
    control = {}
    for sid, (sname, division, switch) in stops.items():
        control.setdefault(switch, 'the STOP switch (any stop)' if rank_stops else 'the STOP switch')
    for r in table(data, 'Tremulant'):
        control.setdefault(gi(r, 'ControllingSwitchID', 'c'), 'the TREMULANT switch "%s"' % shape(g(r, 'Name', 'b')))
    for r in table(data, 'KeyAction'):
        control.setdefault(gi(r, 'ConditionSwitchID', 'f'), 'the COUPLER switch')
    back, forward = collections.defaultdict(list), collections.defaultdict(list)
    for r in table(data, 'SwitchLinkage'):
        src, dst = gi(r, 'SourceSwitchID', 'a'), gi(r, 'DestSwitchID', 'b')
        on, off = gi(r, 'EngageLinkActionCode', 'f'), gi(r, 'DisengageLinkActionCode', 'g')
        how = '' if (on, off) in ((0, 0), (1, 2)) else ' [when it goes ON]' if (on, off) == (4, 7) else ' [when it goes OFF]' if (on, off) == (7, 4) else ' [link %d/%d]' % (on, off)
        cond = gi(r, 'ConditionSwitchID', 'c')
        back[dst].append((src, how, cond))
        forward[src].append(dst)
    # a switch that leads (plain links, three hops) to a stop / tremulant / coupler switch is that control's drawknob
    for start, what in list(control.items()):
        seen, frontier = {start}, [start]
        for hop in range(3):
            nxt = []
            for s in frontier:
                for src, how, cond in back.get(s, []):
                    if src not in seen and not how:
                        seen.add(src)
                        nxt.append(src)
                        control.setdefault(src, what)
            frontier = nxt

    def trace(switch):
        seen, frontier, found, visited = {switch}, [(switch, '')], [], []
        for hop in range(5):
            nxt = []
            for s, how in frontier:
                if s in role:
                    found.append(role[s] + how)
                    continue
                if s in control:
                    found.append(control[s] + how)
                    continue
                for src, link, cond in back.get(s, []):
                    if src in seen:
                        continue
                    seen.add(src)
                    visited.append((src, how + link))
                    nxt.append((src, how + link + (' if "%s"' % shape(switches.get(cond, ('?',))[0]) if cond else '')))
            if found or not nxt:
                break
            frontier = nxt
        if found:
            return sorted(set(found))[:3]
        # no key, stop, tremulant or coupler behind it: a switch of the console (a button of its own) when one is shown
        console = []
        for s, how in visited:
            sw = switches.get(s, ('?', 'N', 'N', 0))
            if sw[3] or sw[2] == 'Y':
                console.append('the CONSOLE switch "%s"%s%s' % (re.sub(r'\s*\([^)]*[Ss]creen[^)]*\)', '', shape(sw[0])).strip(), ', default ON' if sw[1] == 'Y' else '', how))
        if console:
            return sorted(set(console))[:2]
        sw = switches.get(switch, ('?', 'N', 'N', 0))
        return ['its OWN switch "%s"%s%s (nothing leads to it)' % (shape(sw[0]), ', default ON' if sw[1] == 'Y' else '', ', shown on the console' if sw[3] else '')]

    groups = collections.defaultdict(lambda: {'pipes': 0, 'ranks': collections.Counter(), 'notes': [], 'att': collections.Counter(), 'rel': collections.Counter(),
                                              'why': collections.Counter(), 'files': collections.Counter(), 'relfolders': collections.Counter(), 'deep': collections.Counter()})
    for pid, (rank, pallet, note) in pipes.items():
        att = attacks.get(pid, [])
        if not att or att[0] not in samples:
            continue
        pkg, f = samples[att[0]]
        parts = f.split('/')
        key = (pkg, '/'.join(parts[:2]) if len(parts) > 2 else parts[0] if len(parts) > 1 else '.')
        gr = groups[key]
        gr['pipes'] += 1
        gr['ranks'][shape(ranks.get(rank, '?'))] += 1
        gr['notes'].append(note)
        gr['att'][len(att)] += 1
        rel = releases.get(pid, [])
        gr['rel'][len(rel)] += 1
        gr['files'][shape(parts[-1])] += 1
        gr['deep'][shape('/'.join(parts[2:-1]))] += 1
        for sid in rel[:1]:
            if sid in samples:
                gr['relfolders'][shape('/'.join(samples[sid][1].split('/')[:-1]))] += 1
        if pallet:
            for t in trace(pallet):
                gr['why']['P ' + t] += 1
        users = rank_stops.get(rank, [])
        for sid in users[:1]:
            sname, division, switch = stops.get(sid, ('?', 0, 0))
            sw = switches.get(switch, ('?', 'N', 'N', 0))
            gr['why']['K the keys of division "%s" through stop "%s"%s%s' % (divisions.get(division, '?'), shape(sname),
                      ' (its switch default ON)' if sw[1] == 'Y' else '', '' if sw[3] or any(s in control for s in [switch]) and any(switches.get(b[0], ('', '', '', 0))[3] for b in back.get(switch, [])) else ' (no drawknob shown)')] += 1
        if not pallet and not users:
            gr['why']['NOTHING (no stop, no pallet switch)'] += 1

    return title, groups

def analyse(path, everything=False):
    title, groups = collect(path)
    if groups is None:
        return '', ['  (no object lists: encrypted or not a definition)']

    def noisy(key, gr):
        folder = key[1].lower()
        names = ' '.join(gr['ranks']).lower()
        return (any(w in folder for w in FOLDER_WORDS) or folder.split('/')[0] in ('ross', 'pipe', 'stops', 'keys', 'stop', 'key', 'trem')
                or any(w in names for w in RANK_WORDS) or any(not w.startswith('K ') for w in gr['why']))

    out, tone = [], collections.Counter()
    for key, gr in sorted(groups.items()):
        if not everything and not noisy(key, gr):
            tone[key[1].split('/')[0]] += gr['pipes']
            continue
        notes = [n for n in gr['notes'] if n]
        att = ','.join('%dx%d' % (v, k) for k, v in sorted(gr['att'].items()))
        rel = ','.join('%dx%d' % (v, k) for k, v in sorted(gr['rel'].items()))
        out.append('  %06d/%s: %d pipes, notes %s, attacks %s, releases %s' % (key[0], key[1], gr['pipes'], ('%d-%d' % (min(notes), max(notes))) if notes else '-', att, rel))
        out.append('      ranks: ' + '; '.join('%s x%d' % x for x in gr['ranks'].most_common(3)) + (' ... %d kinds' % len(gr['ranks']) if len(gr['ranks']) > 3 else ''))
        out.append('      below: ' + '; '.join('%s x%d' % (x[0] or '(nothing)', x[1]) for x in gr['deep'].most_common(3)) + (' ... %d kinds' % len(gr['deep']) if len(gr['deep']) > 3 else '')
                   + ' | files: ' + '; '.join('%s x%d' % x for x in gr['files'].most_common(3)))
        if gr['relfolders']:
            out.append('      release from: ' + '; '.join('%s x%d' % x for x in gr['relfolders'].most_common(2)))
        for why, n in gr['why'].most_common(4):
            out.append('      %4d  %s' % (n, why))
        if len(gr['why']) > 4:
            out.append('      ... %d kinds of trigger' % len(gr['why']))
    if tone:
        out.insert(0, '  pipe tone (not listed): ' + ', '.join('%s %d' % x for x in tone.most_common(4)) + (' ...' if len(tone) > 4 else ''))
    return title, out

def one(path):
    try:
        return path, analyse(path)
    except Exception as e:
        return path, ('', ['  FAILED: %r' % e])

if __name__ == '__main__':
    if sys.argv[1] == '--batch':
        import multiprocessing
        folder, out_path = sys.argv[2], sys.argv[3]
        files = sorted(os.path.join(folder, f) for f in os.listdir(folder) if f.lower().endswith('.organ_hauptwerk_xml') and os.path.getsize(os.path.join(folder, f)) > 1000)
        with multiprocessing.Pool(10) as pool:
            results = pool.map(one, files, chunksize=1)
        seen, lines, without = {}, [], []
        for path, (title, body) in results:
            name = os.path.basename(path)[:-len('.Organ_Hauptwerk_xml')]
            noise = [l for l in body if not l.startswith('  pipe tone')]
            if not noise:
                without.append(name)
                continue
            signature = re.sub(r'\d+', 'N', '\n'.join(noise))
            if signature in seen:
                seen[signature].append(name)
                continue
            seen[signature] = [name]
            lines.append(('=== %s  [%s]' % (name, title), body, signature))
        with open(out_path, 'w', encoding='utf-8') as f:
            for head, body, signature in lines:
                same = seen[signature][1:]
                f.write(head + ((' + the same in: ' + '; '.join(same)) if same else '') + '\n' + '\n'.join(body) + '\n')
            f.write('=== WITHOUT ANY NOISE GROUP: ' + '; '.join(without) + '\n')
        print('definitions %d, with noises %d (distinct %d), without %d' % (len(files), len(files) - len(without), len(lines), len(without)))
    else:
        title, body = analyse(sys.argv[1], len(sys.argv) > 2 and sys.argv[2] == 'all')
        print(title)
        print('\n'.join(body))
