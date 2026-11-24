# A summary over every definition of a folder: which kinds of noise wiring exist, how often, under which folder
# and rank names, and one line per definition.
# Usage: python -I odf_noise_summary.py <folder> <out.txt>
import os, re, sys, collections
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import odf_noises as base

FOLDER_WORDS = base.FOLDER_WORDS
RANK_WORDS = base.RANK_WORDS

def strip_names(why):
    why = re.sub(r'"[^"]*"', '"..."', why)
    return why

def kind_of(why):
    if why.startswith('P a KEY'):
        return 'KEY-ON' if '[when it goes ON]' in why else 'KEY-OFF' if '[when it goes OFF]' in why else 'KEY-plain'
    if why.startswith('P an INPUT'):
        return 'INPUT-ON' if '[when it goes ON]' in why else 'INPUT-OFF' if '[when it goes OFF]' in why else 'INPUT-plain'
    if why.startswith('P the STOP'):
        return 'STOP-ON' if '[when it goes ON]' in why else 'STOP-OFF' if '[when it goes OFF]' in why else 'STOP-plain'
    if why.startswith('P the TREMULANT'):
        return 'TREM-ON' if '[when it goes ON]' in why else 'TREM-OFF' if '[when it goes OFF]' in why else 'TREM-plain'
    if why.startswith('P the COUPLER'):
        return 'COUPLER-ON' if '[when it goes ON]' in why else 'COUPLER-OFF' if '[when it goes OFF]' in why else 'COUPLER-plain'
    if why.startswith('P the CONSOLE'):
        return 'CONSOLE-ON' if '[when it goes ON]' in why else 'CONSOLE-OFF' if '[when it goes OFF]' in why else 'CONSOLE-plain'
    if why.startswith('P its OWN'):
        return 'OWN-SWITCH'
    if why.startswith('K '):
        return 'AS-A-STOP'
    return 'NOTHING'

def work(path):
    try:
        title, groups = base.collect(path)
    except Exception as e:
        return path, 'FAILED %r' % e, []
    if groups is None:
        return path, 'no object lists', []
    found = []
    for (pkg, folder), gr in groups.items():
        low = folder.lower()
        names = ' '.join(gr['ranks']).lower()
        whys = gr['why']
        palette = sum(n for w, n in whys.items() if not w.startswith('K '))
        by_words = any(w in low for w in FOLDER_WORDS) or any(w in names for w in RANK_WORDS)
        if not palette and not by_words:
            continue
        notes = [n for n in gr['notes'] if n]
        found.append({'pkg': pkg, 'folder': folder, 'pipes': gr['pipes'], 'ranks': dict(gr['ranks']), 'why': dict(whys), 'deep': dict(gr['deep']),
                      'files': dict(gr['files']), 'att': dict(gr['att']), 'rel': dict(gr['rel']), 'relfolders': dict(gr['relfolders']),
                      'lo': min(notes) if notes else 0, 'hi': max(notes) if notes else 0})
    return path, title, found

if __name__ == '__main__':
    import multiprocessing
    folder, out_path = sys.argv[1], sys.argv[2]
    files = sorted(os.path.join(folder, f) for f in os.listdir(folder) if f.lower().endswith('.organ_hauptwerk_xml') and os.path.getsize(os.path.join(folder, f)) > 1000)
    with multiprocessing.Pool(10) as pool:
        results = pool.map(work, files, chunksize=1)
    kinds = collections.defaultdict(lambda: {'defs': set(), 'pipes': 0, 'top': collections.Counter(), 'second': collections.Counter(), 'ranks': collections.Counter(),
                                             'why': collections.Counter(), 'att': collections.Counter(), 'rel': collections.Counter(), 'deep': collections.Counter(),
                                             'files': collections.Counter(), 'own': collections.Counter()})
    per_def, failed, without = [], [], []
    for path, title, found in results:
        name = os.path.basename(path)[:-len('.Organ_Hauptwerk_xml')]
        if title.startswith('FAILED') or title == 'no object lists':
            failed.append('%s (%s)' % (name, title))
            continue
        if not found:
            without.append(name)
            continue
        codes = collections.Counter()
        for gr in found:
            for why, n in gr['why'].items():
                kind = kind_of(why)
                k = kinds[kind]
                k['defs'].add(name)
                k['pipes'] += n
                parts = gr['folder'].lower().split('/')
                k['top'][base.shape(parts[0])] += 1
                k['second'][base.shape(parts[1]) if len(parts) > 1 else '(none)'] += 1
                for r, c in gr['ranks'].items():
                    k['ranks'][re.sub(r'^[^:]{0,28}:\s*', '<head>: ', r)] += 1
                k['why'][strip_names(why)] += 1
                for a, c in gr['att'].items():
                    k['att'][a] += c
                for a, c in gr['rel'].items():
                    k['rel'][a] += c
                for d, c in gr['deep'].items():
                    k['deep'][d.lower()] += 1
                for d, c in gr['files'].items():
                    k['files'][d.lower()] += 1
                if kind == 'OWN-SWITCH' or kind.startswith('CONSOLE'):
                    m = re.search(r'"([^"]*)"', why)
                    k['own'][(m.group(1) if m else '?') + (' [default ON]' if 'default ON' in why else '') + (' [shown]' if 'shown on the console' in why else '')] += 1
                codes[kind] += n
        per_def.append('%s: %s' % (name, ', '.join('%s %d' % x for x in sorted(codes.items()))))
    with open(out_path, 'w', encoding='utf-8') as f:
        f.write('definitions %d; with noise groups %d; without %d; unreadable %d\n' % (len(files), len(per_def), len(without), len(failed)))
        for kind, k in sorted(kinds.items(), key=lambda x: -len(x[1]['defs'])):
            f.write('\n##### %s: %d definitions, %d pipes\n' % (kind, len(k['defs']), k['pipes']))
            f.write('  top folder: ' + '; '.join('%s x%d' % x for x in k['top'].most_common(14)) + '\n')
            f.write('  second folder: ' + '; '.join('%s x%d' % x for x in k['second'].most_common(40)) + '\n')
            f.write('  below: ' + '; '.join('%s x%d' % (x[0] or '(nothing)', x[1]) for x in k['deep'].most_common(30)) + '\n')
            f.write('  files: ' + '; '.join('%s x%d' % x for x in k['files'].most_common(25)) + '\n')
            f.write('  rank names: ' + '; '.join('%s x%d' % x for x in k['ranks'].most_common(30)) + '\n')
            f.write('  attack samples per pipe: %s; release samples per pipe: %s\n' % (dict(sorted(k['att'].items())), dict(sorted(k['rel'].items()))))
            f.write('  wiring: ' + '; '.join('%s x%d' % x for x in k['why'].most_common(12)) + '\n')
            if k['own']:
                f.write('  own switches: ' + '; '.join('%s x%d' % x for x in k['own'].most_common(80)) + '\n')
        f.write('\n##### PER DEFINITION\n' + '\n'.join(per_def) + '\n')
        f.write('\n##### WITHOUT: ' + '; '.join(without) + '\n')
        f.write('\n##### UNREADABLE: ' + '; '.join(failed) + '\n')
    print('written', out_path, os.path.getsize(out_path))
