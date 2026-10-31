import json, os, sys, re
cfg = sys.argv[1]; root = sys.argv[2]
d = json.load(open(cfg, encoding='utf-8'))
trees = d.get('sampleTrees') or []
rel = d.get('sampleLibraryRootRelative', '')
print('library root rel:', rel, 'rootName', d.get('sampleLibraryRootName'))
print('trees:', len(trees)); 
for i,t in enumerate(trees[:20]): print(' ', i, t)
used = set()
rows = 0
for m in d.get('manualKeyboards', []):
    sb = m.get('stopBuild') or {}
    for e in sb.get('entries', []):
        rows += 1
        for f in e.get('folders', []):
            used.add((int(f['tree']), f['path'].lower().replace('\\','/')))
print('rows', rows, 'used folders', len(used))
ignored = lambda n: ('trem' in n.lower()) or n.lower().endswith('_tr') or n.lower().startswith('rel')
def numbered(folder):
    try:
        for f in os.listdir(folder):
            if f[:1].isdigit() and f.lower().endswith('.wav'): return True
    except OSError: pass
    return False
unused = []
for i,t in enumerate(trees):
    base = os.path.join(root, t) if not os.path.isabs(t) else t
    for dp, dns, fns in os.walk(base):
        dns[:] = [x for x in dns if not ignored(x)]
        if not numbered(dp) or dp == base: continue
        r = os.path.relpath(dp, base).replace('\\','/').lower()
        if (i, r) not in used: unused.append((i, r))
print('unused (pool) folders:', len(unused))
for u in unused[:60]: print('  ', u)
