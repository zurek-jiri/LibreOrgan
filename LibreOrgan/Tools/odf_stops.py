import sys, re, collections
path = sys.argv[1]
wanted = [w.lower() for w in sys.argv[2:]]
data = open(path, 'r', encoding='utf-8', errors='replace').read()
def table(name):
    m = re.search(r'<ObjectList ObjectType="%s">(.*?)</ObjectList>' % re.escape(name), data, re.S)
    if not m: return []
    return [dict(re.findall(r'<([A-Za-z0-9_]+)>([^<]*)</', r)) for r in re.findall(r'<o>(.*?)</o>', m.group(1), re.S)]
def gi(r, *keys):
    for k in keys:
        if k in r and r[k] != '': return int(float(r[k]))
    return 0
stops = {gi(r,'StopID','a'): r for r in table('Stop')}
divisions = {gi(r,'DivisionID','a'): (r.get('Name') or r.get('b')) for r in table('Division')}
ranks = {gi(r,'RankID','a'): (r.get('Name') or r.get('b','')) for r in table('Rank')}
stopranks = table('StopRank')
pipes = {gi(r,'PipeID','a'): gi(r,'RankID','b') for r in table('Pipe_SoundEngine01')}
layers = {}
for r in table('Pipe_SoundEngine01_Layer'):
    lid = gi(r,'LayerID','a'); pid = gi(r,'PipeID','b')
    if pid and pid not in layers.values(): layers.setdefault(lid, pid)
attack = {}
for r in table('Pipe_SoundEngine01_AttackSample'):
    lid = gi(r,'LayerID','b'); sid = gi(r,'SampleID','c')
    attack.setdefault(lid, sid)
samples = {gi(r,'SampleID','a'): (gi(r,'InstallationPackageID','b'), (r.get('SampleFilename') or r.get('c',''))) for r in table('Sample')}
rankfolder = collections.defaultdict(collections.Counter)
for lid, pid in layers.items():
    sid = attack.get(lid)
    if not sid or sid not in samples: continue
    pkg, f = samples[sid]
    rankfolder[pipes.get(pid, 0)][(pkg, f.rsplit('/', 1)[0])] += 1
alt = set()
for r in stopranks:
    a = gi(r,'AlternateRankID','p')
    if a: alt.add(a)
print('stops %d ranks %d stopranks %d' % (len(stops), len(ranks), len(stopranks)))
by_stop = collections.defaultdict(list)
for r in stopranks: by_stop[gi(r,'StopID','a')].append(r)
rank_users = collections.defaultdict(list)
for r in stopranks: rank_users[gi(r,'RankID','d')].append((gi(r,'StopID','a'), gi(r,'MIDINoteNumOfFirstMappedDivisionInputNode','h'), gi(r,'NumberOfMappedDivisionInputNodes','i'), gi(r,'MIDINoteNumIncrementFromDivisionToRank','j')))
for sid, s in sorted(stops.items()):
    name = s.get('Name') or s.get('b','')
    if wanted and not any(w in name.lower() for w in wanted): continue
    print('== stop %d "%s" division %s' % (sid, name, divisions.get(gi(s,'DivisionID','c'))))
    for r in by_stop[sid]:
        rid = gi(r,'RankID','d'); h = gi(r,'MIDINoteNumOfFirstMappedDivisionInputNode','h'); i = gi(r,'NumberOfMappedDivisionInputNodes','i'); j = gi(r,'MIDINoteNumIncrementFromDivisionToRank','j'); a = gi(r,'AlternateRankID','p')
        fc = rankfolder.get(rid, {})
        folder = max(fc.items(), key=lambda x: x[1])[0] if fc else None
        users = ['%d(k%d,%+d)' % (u[0], u[2], u[3]) for u in rank_users[rid]]
        print('   rank %5d first %3d count %3d incr %+3d alt %d %s "%s" folder %s users %s' % (rid, h, i, j, a, 'TREMMED' if rid in alt else '', ranks.get(rid,''), folder, ' '.join(users)))
